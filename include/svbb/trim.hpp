#pragma once
#include "svbb/config.hpp"
#include <algorithm>

namespace SVBB_NAMESPACE {
template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto trim_left(basic_string_view<CharT, Traits> to_trim, CharT with)
    -> basic_string_view<CharT, Traits>
{
    to_trim.remove_prefix(std::min(to_trim.find_first_not_of(with), to_trim.size()));
    return to_trim;
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto trim_left(basic_string_view<CharT, Traits> to_trim,
                                    basic_string_view<CharT, Traits> with)
    -> basic_string_view<CharT, Traits>
{
    to_trim.remove_prefix(std::min(to_trim.find_first_not_of(with), to_trim.size()));
    return to_trim;
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto trim_right(basic_string_view<CharT, Traits> to_trim, CharT with)
    -> basic_string_view<CharT, Traits>
{
    to_trim.remove_suffix(
        std::min(to_trim.size() - to_trim.find_last_not_of(with) - 1, to_trim.size()));
    return to_trim;
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto trim_right(basic_string_view<CharT, Traits> to_trim,
                                     basic_string_view<CharT, Traits> with)
    -> basic_string_view<CharT, Traits>
{
    to_trim.remove_suffix(
        std::min(to_trim.size() - to_trim.find_last_not_of(with) - 1, to_trim.size()));
    return to_trim;
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto trim(basic_string_view<CharT, Traits> to_trim, CharT with)
    -> basic_string_view<CharT, Traits>
{
    return trim_left(trim_right(to_trim, with), with);
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto trim(basic_string_view<CharT, Traits> to_trim,
                               basic_string_view<CharT, Traits> with)
    -> basic_string_view<CharT, Traits>
{
    return trim_left(trim_right(to_trim, with), with);
}
} // namespace SVBB_NAMESPACE
