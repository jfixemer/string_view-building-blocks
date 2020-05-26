#include <iostream>
#include "example_config.hpp" // for setting up which string_view implementation is used
#include "svbb/split.hpp"
#include "svbb/util.hpp" // svbb::make_view
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>      // std::stringstream
#include <string.h>
#include <algorithm>

using namespace std::chrono;
using std::basic_string;

void shuffle(std::string& deck)
{
    const auto len = deck.size();

    // A little shuffle
    for(size_t i=0; i < len * 3; i++)
        std::swap(deck[i % len], deck[rand() % len]);

}

template<typename CharT, typename Traits = std::char_traits<CharT>>
struct string_split_result
{
    basic_string<CharT, Traits> left;
    basic_string<CharT, Traits> right;
};


template<typename CharT, typename Traits>
SVBB_CONSTEXPR auto make_string_split(basic_string<CharT, Traits> left,
                               basic_string<CharT, Traits> right)
    -> string_split_result<CharT, Traits>
{
    return {left, right};
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto string_split_around(basic_string<CharT, Traits> input, size_t start,
                                       size_t length) 
    -> string_split_result<CharT, Traits>
{
    return {input.substr(0, start), input.substr(std::min(start + length, input.size()))};
}

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto string_split_around(basic_string<CharT, Traits> input, size_t pos)
    -> string_split_result<CharT, Traits>
{
    return string_split_around(input, pos, 1);
}

int main(int argc, char** argv)
{
    using svbb::make_view;
    using svbb::make_split;

    std::string test_data_no_comma=
        "12345678901234567890"
        "12345678901234567890"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "@#$%^&*()-_=+[{]}\\|<.>/?`~\""
        "@#$%^&*()-_=+[{]}\\|<.>/?`~\"";


    system_clock::time_point tp = system_clock::now();
    system_clock::duration dtn = tp.time_since_epoch();
    unsigned epoch_seconds = dtn.count() * system_clock::period::num / system_clock::period::den;

    if(argc <= 1)
        srand(epoch_seconds);
    else
        srand(atoi(argv[1]));

    const auto len = test_data_no_comma.size();

    std::cout << "Generating Data...";
    std::cout << std::endl;

    std::stringstream test_data;
    size_t total_size = 0;
    size_t i = 0;
    // Much smaller than the tokenize_for_loop because this is an O(N**2) algorithm for string
    // in memory usage.
    while( i < 1000000 )
    {
        if((i++ % 37) == 0){
            shuffle(test_data_no_comma);
            test_data << "\n";
        }

        auto size = (rand() % (len - 5)) + 5;
        test_data << test_data_no_comma.substr(0,size) << ',';
        total_size += size;
    }
    //test_data << "!END";

    std::string constructed_str = std::move(test_data.str());
    size_t sv_token_count = 0;
    size_t sv_tsize = 0;

    std::vector<int> distribution(len+1, 0);
    duration<double> cstr_time_span;
    duration<double> strlen_time_span;
    duration<double> sv_time_span;
    duration<double> string_time_span;

    {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        const char* str = constructed_str.c_str();
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto result = strlen(str);
        high_resolution_clock::time_point t3 = high_resolution_clock::now();

        std::cout << "len: " << result << ", size(): " << constructed_str.size() << "\n";
        
        cstr_time_span = duration_cast<duration<double>>(t2 - t1);
        strlen_time_span = duration_cast<duration<double>>(t3 - t2);
    }

    {
        std::cout << "Starting SV test...";
        std::cout << std::endl;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        auto input = make_view(constructed_str);
        size_t pos = 0;
        while(pos < input.size())
        {
            auto end = input.find(',');
            //std::cout << "pos:" << pos << ", end:" << end << "\n";
            auto data = input.substr(0, end);
            sv_tsize += data.size();

            //distribution[data.size()]++;
            //std::cout << "(" << string_token_count << "):"  << data << "\n";
            sv_token_count++;
            input = input.substr( std::min(end, input.size() - 1) + 1);
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        sv_time_span = duration_cast<duration<double>>(t2 - t1);
    }

    size_t string_token_count = 0;
    {
        std::cout << "Starting String test...";
        std::cout << std::endl;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        size_t pos = 0;
        std::string data;
        size_t count = 0;
        while(pos < constructed_str.size())
        {
            count++;
            auto end = constructed_str.find(',', pos);
            //std::cout << "pos:" << pos << ", end:" << end << "\n";
            data = std::move(constructed_str.substr(pos,end - pos));

            //distribution[data.size()]++;
            //std::cout << "(" << string_token_count << "):"  << data << "\n";
            string_token_count++;
            pos = ((end == std::string::npos) ? std::string::npos : end + 1);
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        string_time_span = duration_cast<duration<double>>(t2 - t1);
    }


    // for(size_t i = 1; i < len; ++i)
    //     std::cout << "Length: " << i << " has " << distribution[i] << " occurrences\n";

    std::cout << "Total size: " << total_size << "\n";
    std::cout << "Total sv tokens: " << sv_token_count << "\n";
    std::cout << "Total string tokens: " << string_token_count << "\n";
    std::cout << "CSTR took: " << cstr_time_span.count() << " seconds.\n";
    std::cout << "STRLEN took: " << strlen_time_span.count() << " seconds.\n";
    std::cout << "SV Tokenize took: " << sv_time_span.count() << " seconds.\n";
    std::cout << "String Tokenize took: " << string_time_span.count() << " seconds.\n";

    std::cout << std::endl;
}
