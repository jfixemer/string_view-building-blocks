#pragma once
#include "svbb/config.hpp"

namespace SVBB_NAMESPACE {

template<typename CharT>
struct constexpr_char_traits : public std::char_traits<CharT>
{
    static SVBB_CXX14_CONSTEXPR int compare(const CharT* s1, const CharT* s2, size_t n)
    {
        while(n-- != 0) {
            if(*s1 < *s2) return -1;
            if(*s1 > *s2) return 1;
            ++s1;
            ++s2;
        }
        return 0;
    }
    static SVBB_CXX14_CONSTEXPR const char* find(const CharT* s, size_t n, const CharT& a)
    {
        while(n-- != 0) {
            if(*s == a) return s;
            s++;
        }
        return nullptr;
    }
};
} // namespace SVBB_NAMESPACE
