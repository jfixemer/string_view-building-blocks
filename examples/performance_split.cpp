#include <iostream>
#include "example_config.hpp" // for setting up which string_view implementation is used
#include "svbb/split.hpp"
#include "svbb/util.hpp" // svbb::make_view
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>      // std::stringstream

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
        "!@#$%^&*()-_=+[{]}\\|<.>/?`~\""
        "!@#$%^&*()-_=+[{]}\\|<.>/?`~\"";


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
    while( i < 30000 )
    {
        if((i++ % 37) == 0){
            shuffle(test_data_no_comma);
            test_data << "\n";
        }

        auto size = (rand() % (len - 5)) + 5;
        test_data << test_data_no_comma.substr(0,size) << ',';
        total_size += size;
    }

    std::string constructed_str = std::move(test_data.str());
    size_t token_count = 0;
    std::vector<int> distribution(len+1, 0);
    duration<double> sv_time_span;
    duration<double> string_time_span;
    {
        std::cout << "Starting SV test...";
        std::cout << std::endl;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        auto svdata = svbb::make_split(svbb::string_view(),make_view(constructed_str));
        svdata = svbb::split_around(svdata.right, std::min(svdata.right.find(','), svdata.right.size()));
        while( ! (svdata.left.empty()  || svdata.right.empty()))
        {
            distribution[svdata.left.size()]++;
            token_count++;
            svdata = svbb::split_around(svdata.right, std::min(svdata.right.find(','), svdata.right.size()));
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        sv_time_span = duration_cast<duration<double>>(t2 - t1);
    }

    {
        std::cout << "Starting String test...";
        std::cout << std::endl;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        auto data = make_string_split(std::string(),std::move(constructed_str));
        data = std::move(string_split_around(data.right, std::min(data.right.find(','), data.right.size())));
        while( ! (data.left.empty()  || data.right.empty()))
        {
            distribution[data.left.size()]++;
            token_count++;
            data = std::move(string_split_around(data.right, std::min(data.right.find(','), data.right.size())));
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        string_time_span = duration_cast<duration<double>>(t2 - t1);
    }


    std::cout << "Total distribution: " << token_count << "\n";
    for(size_t i = 1; i < len; ++i)
        std::cout << "Length: " << i << " has " << distribution[i] << " occurrences\n";

    std::cout << "Total size: " << total_size << "\n";
    std::cout << "Total tokens: " << token_count << "\n";
    std::cout << "SV Tokenize took: " << sv_time_span.count() << " seconds.\n";
    std::cout << "String Tokenize took: " << string_time_span.count() << " seconds.\n";

    std::cout << std::endl;
}
