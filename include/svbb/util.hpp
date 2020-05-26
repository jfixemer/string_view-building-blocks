#pragma once
#include "svbb/config.hpp"

namespace SVBB_NAMESPACE {

template<typename CharT, typename Traits, typename Allocator>
auto make_view(const std::basic_string<CharT, Traits, Allocator>& str)
    -> basic_string_view<CharT, Traits>
{
    return basic_string_view<CharT, Traits>{str};
}

template<typename CharT, typename Traits, typename Allocator>
auto make_view(std::basic_string<CharT, Traits, Allocator>&& str)
    -> basic_string_view<CharT, Traits>
{
    static_assert(sizeof(CharT) != sizeof(CharT),
                  "You are trying to create a view to an rvalue string.");
}

template<typename CharT, typename Traits = std::char_traits<CharT>>
SVBB_CONSTEXPR auto make_view(const CharT* str) -> basic_string_view<CharT, Traits>
{
    return basic_string_view<CharT, Traits>{str};
}

template<typename CharT, typename Traits = std::char_traits<CharT>>
SVBB_CONSTEXPR auto make_view(const CharT* str, size_t len) -> basic_string_view<CharT, Traits>
{
    return basic_string_view<CharT, Traits>{str, len};
}
} // namespace SVBB_NAMESPACE
