#pragma once
#include "svbb/config.hpp"
#include "char_traits.hpp"

namespace SVBB_NAMESPACE { namespace literals {

SVBB_CONSTEXPR string_view operator"" _sv(const char* str, std::size_t len) SVBB_NOEXCEPT
{
    return string_view{str, len};
}

SVBB_CONSTEXPR auto operator"" _svc(const char* str, std::size_t len) SVBB_NOEXCEPT
    -> basic_string_view<char, constexpr_char_traits<char>>
{
    return basic_string_view<char, constexpr_char_traits<char>>{str, len};
}
}} // namespace SVBB_NAMESPACE::literals
