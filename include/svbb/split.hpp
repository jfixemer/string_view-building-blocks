#pragma once
#include "config.hpp"
#include <algorithm>

namespace SVBB_NAMESPACE {

template<typename CharT, typename Traits = std::char_traits<CharT>>
struct split_result
{
    basic_string_view<CharT, Traits> left;
    basic_string_view<CharT, Traits> right;
};

template<typename CharT, typename Traits>
SVBB_CONSTEXPR auto make_split(basic_string_view<CharT, Traits> left,
                               basic_string_view<CharT, Traits> right)
    -> split_result<CharT, Traits>
{
    return {left, right};
}

template<typename CharT, typename Traits>
SVBB_CONSTEXPR bool operator==(const split_result<CharT, Traits>& lhs,
                               const split_result<CharT, Traits>& rhs)
{
    return lhs.left == rhs.left && lhs.right == rhs.right;
}

template<typename CharT, typename Traits>
inline std::ostream& operator<<(std::ostream& os, const split_result<CharT, Traits>& splitted)
{
    return os << splitted.left << " " << splitted.right;
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto split_around(basic_string_view<CharT, Traits> input, size_t start,
                                       size_t length) 
    -> split_result<CharT, Traits>
{
    return {input.substr(0, start), input.substr(std::min(start + length, input.size()))};
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto split_around(basic_string_view<CharT, Traits> input, size_t pos)
    -> split_result<CharT, Traits>
{
    return split_around(input, pos, 1);
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto split_at(basic_string_view<CharT, Traits> input, size_t pos)
    -> split_result<CharT, Traits>
{
    pos = std::min(pos, input.size());
    return {input.substr(0, pos), input.substr(pos)};
}

// Keep the split on character in the first element
template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto split_before(basic_string_view<CharT, Traits> input, CharT delim)
    -> split_result<CharT, Traits>
{
    return split_at(input, input.find(delim));
}

// Keep the split on character in the second element
template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto split_after(basic_string_view<CharT, Traits> input, CharT delim)
    -> split_result<CharT, Traits>
{
    return split_at(input, std::min(input.find_first_of(delim), input.size()) + 1);
}

// Keep the split on character in the first element
template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto split_before(basic_string_view<CharT, Traits> input, 
                                       basic_string_view<CharT, Traits> delim)
    -> split_result<CharT, Traits>
{
    return split_at(input, input.find_first_of(delim));
}

// Keep the split on character in the second element
template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto split_after(basic_string_view<CharT, Traits> input,
                                      basic_string_view<CharT, Traits> delim)
    -> split_result<CharT, Traits>
{
    return split_at(input, std::min(input.find_first_of(delim), input.size()) + 1 );
}


} // namespace SVBB_NAMESPACE
