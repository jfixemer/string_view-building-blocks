#include "catch.hpp"
#include "test_config.hpp"
#include "svbb/xml_tokenizer.hpp"
#include "svbb/util.hpp"
#include <vector>
#include <algorithm>

namespace {
using namespace SVBB_NAMESPACE;
using namespace SVBB_NAMESPACE::literals;
using namespace SVBB_NAMESPACE::xml;

TEST_CASE("Instantiate Empty")
{
    detail::token_state<char, std::char_traits<char>> fsm;

    REQUIRE(fsm.last_token() == token{ELEMENT::END_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == true);
}

TEST_CASE("Instantiate")
{
    auto document = "<ROOT></ROOT>"_sv;

    detail::token_state fsm(document);

    REQUIRE(fsm.last_token() == token{ELEMENT::START_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == false);
}

TEST_CASE("Document Prolog")
{
    auto document = "<?xml encoding=\"UTF-8\"?>\n"
    "<?some.com <embedded x=\"5\"></embedded>?>"
    "<ROOT></ROOT>"_sv;

    xml::detail::token_state fsm(document);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::PROLOG, "xml"_sv, "encoding=\"UTF-8\""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::PROCESSING_INSTRUCTION, "some.com"_sv, "<embedded x=\"5\"></embedded>"_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);
}

TEST_CASE("Parse simple doc")
{
    auto document = "<ROOT></ROOT>"_sv;

    xml::detail::token_state fsm(document);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == true);

}


TEST_CASE("Parse simple doc - empty node")
{
    auto document = "<ROOT><EMPTY/></ROOT>"_sv;

    xml::detail::token_state fsm(document);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_ELEMENT, "EMPTY"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_ELEMENT, "EMPTY"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == true);
}

TEST_CASE("Parse simple doc -- attributes")
{
    auto document = "<ROOT xmlns:svbb=\"svbb\">\n"
        "<svbb:node test=\"true\"   foo:ran = \"some text\" fizzypop=\" unusual \" />\n"
        "</ROOT>"_sv;

    xml::detail::token_state fsm(document);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);
 
    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::ELEMENT_ATTRIBUTE, "xmlns:svbb"_sv, "svbb"_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_ELEMENT, "svbb:node"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::ELEMENT_ATTRIBUTE, "test"_sv, "true"_sv});

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::ELEMENT_ATTRIBUTE, "foo:ran"_sv, "some text"_sv});

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::ELEMENT_ATTRIBUTE, "fizzypop"_sv, " unusual "_sv});

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_ELEMENT, "svbb:node"_sv, ""_sv});    
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_ELEMENT, "ROOT"_sv, ""_sv});    
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == true);

}


TEST_CASE("Parse simple doc - comment out bad structure")
{
    auto document = "<ROOT>Start<!--<BAD NODE>->-!>-->Text</ROOT>"_sv;

    xml::detail::token_state fsm(document);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::START_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::CHARACTERS, ""_sv, "Start"_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::CHARACTERS, ""_sv, "Text"_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_ELEMENT, "ROOT"_sv, ""_sv});
    REQUIRE(fsm.empty() == false);

    fsm.split();
    REQUIRE(fsm.last_token() == token{ELEMENT::END_DOCUMENT, ""_sv, ""_sv});
    REQUIRE(fsm.empty() == true);
}

}