#include "catch.hpp"
#include "test_config.hpp"
#include "svbb/split.hpp"
#include "svbb/util.hpp"
#include "svbb/literals.hpp"

namespace {
using namespace SVBB_NAMESPACE;
using namespace SVBB_NAMESPACE::literals;

TEST_CASE("split around position")
{
    REQUIRE(split_around(""_sv, 0) == make_split(""_sv, ""_sv));
    REQUIRE(split_around("abc"_sv, 0) == make_split(""_sv, "bc"_sv));
    REQUIRE(split_around("abc"_sv, 1) == make_split("a"_sv, "c"_sv));
    REQUIRE(split_around("abc"_sv, 2) == make_split("ab"_sv, ""_sv));
    REQUIRE(split_around("abc"_sv, 3) == make_split("abc"_sv, ""_sv));
}

TEST_CASE("split around range")
{
    REQUIRE(split_around(""_sv, 0, 1) == make_split(""_sv, ""_sv));
    REQUIRE(split_around("abc"_sv, 0, 1) == make_split(""_sv, "bc"_sv));
    REQUIRE(split_around("abc"_sv, 0, 2) == make_split(""_sv, "c"_sv));
    REQUIRE(split_around("abc"_sv, 1) == make_split("a"_sv, "c"_sv));
    REQUIRE(split_around("abc"_sv, 2) == make_split("ab"_sv, ""_sv));
    REQUIRE(split_around("abc"_sv, 3) == make_split("abc"_sv, ""_sv));
}


TEST_CASE("split at position")
{
    REQUIRE(split_at(""_sv, 0) == make_split(""_sv, ""_sv));
    REQUIRE(split_at(""_sv, 1) == make_split(""_sv, ""_sv));
    REQUIRE(split_at("abc"_sv, 0) == make_split(""_sv, "abc"_sv));
    REQUIRE(split_at("abc"_sv, 1) == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_at("abc"_sv, 2) == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_at("abc"_sv, 3) == make_split("abc"_sv, ""_sv));
    REQUIRE(split_at("abc"_sv, 4) == make_split("abc"_sv, ""_sv));
}

TEST_CASE("split on a char, leave delimiter in second part")
{
    REQUIRE(split_before(""_sv, 'x') == make_split(""_sv, ""_sv));
    REQUIRE(split_before("abc"_sv, 'a') == make_split(""_sv, "abc"_sv));
    REQUIRE(split_before("abc"_sv, 'b') == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_before("abc"_sv, 'c') == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_before("abc"_sv, 'x') == make_split("abc"_sv, ""_sv));
}

TEST_CASE("split on a delimiter list, leave delimiter in second part")
{
    REQUIRE(split_before(""_sv, "x"_sv) == make_split(""_sv, ""_sv));

    REQUIRE(split_before("abc"_sv, "a"_sv) == make_split(""_sv, "abc"_sv));
    REQUIRE(split_before("abc"_sv, "b"_sv) == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_before("abc"_sv, "c"_sv) == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_before("abc"_sv, "x"_sv) == make_split("abc"_sv, ""_sv));

    REQUIRE(split_before("abc"_sv, "ax"_sv) == make_split(""_sv, "abc"_sv));
    REQUIRE(split_before("abc"_sv, "bx"_sv) == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_before("abc"_sv, "cx"_sv) == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_before("abc"_sv, "xy"_sv) == make_split("abc"_sv, ""_sv));

    REQUIRE(split_before("abc"_sv, "xa"_sv) == make_split(""_sv, "abc"_sv));
    REQUIRE(split_before("abc"_sv, "xb"_sv) == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_before("abc"_sv, "xc"_sv) == make_split("ab"_sv, "c"_sv));
}

TEST_CASE("split on a char, leave delimiter in first part")
{
    REQUIRE(split_after(""_sv, 'x') == make_split(""_sv, ""_sv));
    REQUIRE(split_after("abc"_sv, 'a') == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_after("abc"_sv, 'b') == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_after("abc"_sv, 'c') == make_split("abc"_sv, ""_sv));
    REQUIRE(split_after("abc"_sv, 'x') == make_split("abc"_sv, ""_sv));
}

TEST_CASE("split on a delimiter list, leave delimiter in first part")
{
    REQUIRE(split_after(""_sv, "x"_sv) == make_split(""_sv, ""_sv));
    REQUIRE(split_after("abc"_sv, "a"_sv) == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_after("abc"_sv, "b"_sv) == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_after("abc"_sv, "c"_sv) == make_split("abc"_sv, ""_sv));
    REQUIRE(split_after("abc"_sv, "x"_sv) == make_split("abc"_sv, ""_sv));

    REQUIRE(split_after("abc"_sv, "ax"_sv) == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_after("abc"_sv, "bx"_sv) == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_after("abc"_sv, "cx"_sv) == make_split("abc"_sv, ""_sv));
    REQUIRE(split_after("abc"_sv, "xy"_sv) == make_split("abc"_sv, ""_sv));

    REQUIRE(split_after("abc"_sv, "xa"_sv) == make_split("a"_sv, "bc"_sv));
    REQUIRE(split_after("abc"_sv, "xb"_sv) == make_split("ab"_sv, "c"_sv));
    REQUIRE(split_after("abc"_sv, "xc"_sv) == make_split("abc"_sv, ""_sv));
}

} // namespace
