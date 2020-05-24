#pragma once
#include "svbb/config.hpp"
#include "svbb/split.hpp"

namespace SVBB_NAMESPACE {

namespace detail {

template<typename CharT, typename Traits, typename Splitter>
class token_state
{
public:
    using view_type = basic_string_view<CharT, Traits>;

    SVBB_CONSTEXPR token_state() SVBB_NOEXCEPT {}
    SVBB_CONSTEXPR token_state(view_type input, Splitter splitter)
        : data_(make_split(view_type(), input)), splitter_(std::move(splitter))
    {
    }

    SVBB_CONSTEXPR view_type token() const SVBB_NOEXCEPT { return data_.left; }
    SVBB_CONSTEXPR view_type remainder() const SVBB_NOEXCEPT { return data_.right; }
    SVBB_CONSTEXPR bool empty() const SVBB_NOEXCEPT
    {
        return remainder().empty() && token().empty();
    }
    SVBB_CXX14_CONSTEXPR void split() { data_ = splitter_(remainder()); }

private:
    split_result<CharT, Traits> data_;
    Splitter splitter_;
};
} // namespace detail

template<typename CharT, typename Traits, typename Splitter>
class token_iterator
{
public:
    using view_type = basic_string_view<CharT, Traits>;
    using value_type = view_type;
    using difference_type = std::ptrdiff_t;
    using pointer = const view_type*;
    using reference = view_type;
    using iterator_category = std::forward_iterator_tag;
    using state_type = detail::token_state<CharT, Traits, Splitter>;

    SVBB_CONSTEXPR token_iterator() SVBB_NOEXCEPT = default;
    SVBB_CXX14_CONSTEXPR token_iterator(view_type input, Splitter splitter)
        : state_(input, std::move(splitter))
    {
        advance();
    }

    SVBB_CONSTEXPR reference operator*() const SVBB_NOEXCEPT { return state_.token(); }
    SVBB_CXX14_CONSTEXPR token_iterator& operator++()
    {
        advance();
        return *this;
    }

    SVBB_CXX14_CONSTEXPR token_iterator operator++(int)
    {
        token_iterator tmp = *this;
        advance();
        return tmp;
    }

    SVBB_CONSTEXPR bool operator==(const token_iterator& rhs) const SVBB_NOEXCEPT
    {
        return (!state_.empty() && !rhs.state_.empty()) ?
                   (state_.remainder().data() == rhs.state_.remainder().data() &&
                    state_.remainder().size() == rhs.state_.remainder().size()) :
                   (state_.empty() == rhs.state_.empty());
    }
    SVBB_CONSTEXPR bool operator!=(const token_iterator& rhs) const SVBB_NOEXCEPT
    {
        return !(*this == rhs);
    }

    SVBB_CONSTEXPR bool valid() const SVBB_NOEXCEPT { return state_ != nullptr; }

private:
    state_type state_;

    SVBB_CXX14_CONSTEXPR void advance() { state_.split(); }
};

} // namespace SVBB_NAMESPACE
