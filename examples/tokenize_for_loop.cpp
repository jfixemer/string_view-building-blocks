#include <iostream>
#include "example_config.hpp" // for setting up which string_view implementation is used
#include "svbb/tokenize.hpp"
#include "svbb/util.hpp" // svbb::make_view
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <sstream>      // std::stringstream

using namespace std::chrono;

void shuffle(std::string& deck)
{
    const auto len = deck.size();

    // A little shuffle
    for(size_t i=0; i < len * 3; i++)
        std::swap(deck[i % len], deck[rand() % len]);

}

int main(int argc, char** argv)
{
    using svbb::make_view;
    using svbb::tokenize;

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
    while( i < 1000000  )
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
    const auto view = make_view(constructed_str);

    size_t token_count = 0;
    std::vector<int> distribution(len+1, 0);
    
    std::cout << "Starting test...";
    std::cout << std::endl;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for(const auto token : tokenize(view, ',')) {
        distribution[token.size()]++;
        token_count++;
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

    std::cout << "Total distribution: " << token_count << "\n";
    // for(size_t i = 1; i < len; ++i)
    //     std::cout << "Length: " << i << " has " << distribution[i] << " occurrences\n";

    std::cout << "Total tokens: " << token_count << "\n";
    std::cout << "Tokenize took: " << time_span.count() << " seconds.\n";
    std::cout << "Total size: " << total_size << "\n";
    if(argc <= 1)
        std::cout << "Seed value: " << epoch_seconds << "\n";

    std::cout << std::endl;
}
