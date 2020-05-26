#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <chrono>
#include <string.h>

#include "example_config.hpp" // for setting up which string_view implementation is used
#include "svbb/xml_tokenizer.hpp"

using namespace std::chrono;

int main(int argc, char** argv)
{
    std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    //duration<double> sv_time_span;
    svbb::string_view input = {buffer.data(), buffer.size()};

    // duration<double> strlen_time_span;
    // std::cout << "Starting strlen test of " << size << " bytes.";
    // std::cout << std::endl;
    // {
    //     high_resolution_clock::time_point t1 = high_resolution_clock::now();
    //     // non-sse2 / naive strlen:
    //     char* ptr = buffer.data();
    //     while(*(ptr++) != '\0') ;
    //     size_t size_strlen = ptr - buffer.data();

    //     high_resolution_clock::time_point t2 = high_resolution_clock::now();
    //     strlen_time_span = duration_cast<duration<double>>(t2 - t1);
    //     std::cout << "strlen size: " << size_strlen << " bytes; buffer size=" << size << "\n";
    // }

    std::cout << "Starting XML test of " << size << " bytes.";
    std::cout << std::endl;

    constexpr int num_token_types = static_cast<int>(svbb::xml::ELEMENT::ERROR);
    std::array<size_t, num_token_types + 1> type_counter;
    type_counter.fill(0);
    int depth = 0;
    int max_depth = 0;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for(auto xml_token : svbb::xml::tokenize(input)){
        ++type_counter[static_cast<int>(xml_token.element)];
        if(xml_token.element == svbb::xml::ELEMENT::START_ELEMENT){
            ++depth;
            max_depth = (depth > max_depth)? depth : max_depth;
        }
        else if(xml_token.element == svbb::xml::ELEMENT::END_ELEMENT)
            --depth;
        //std::cout << xml_token << "\n";
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    auto sv_time_span = duration_cast<duration<double>>(t2 - t1);

    for(int i = 0; i <= num_token_types; ++i)
        std::cout << "Element " << static_cast<svbb::xml::ELEMENT>(i) << " has " << type_counter[i] << " occurrences\n";

    std::cout << "End depth = " << depth << "; Max depth =" << max_depth << "\n";
    //std::cout << "strlen took: " << strlen_time_span.count() << " seconds.\n";
    std::cout << "SVBB XML-SAXish took: " << sv_time_span.count() << " seconds.\n";

    std::cout << std::endl;
}
