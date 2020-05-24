#include <iostream>
#include "example_config.hpp" // for setting up which string_view implementation is used
#include "svbb/tokenize.hpp"
#include "svbb/util.hpp" // svbb::make_view
#include "svbb/literals.hpp"


int main(int argc, char** argv)
{
    using svbb::make_view;
    using svbb::tokenize;
    using namespace svbb::literals;

    std::string test_data = R"XML(<?xml version="1.0" encoding="UTF-8"?>
    <root>
        <node>Some data</node>
        <node attrib="stuff"></node>
        <node attrib="other"/>
        <?proc this="foo"?>
        <node><child><grand></grand></child></node>
    <root>
    )XML";

    size_t token_count=0;
    for(const auto token : tokenize(make_view(test_data), "<>"_sv)) 
        std::cout << "(" << token_count++ << "):[" << token << "]\n";
}
