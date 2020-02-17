#pragma once

#include <random>

namespace quinbot
{
namespace util
{
    template<typename Iter, typename RandomGenerator>
    Iter select_randomly( Iter start, Iter end, RandomGenerator &g ) 
    {
        std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
        std::advance(start, dis(g));
        return start;
    }

    template<typename Iter>
    Iter select_randomly( Iter start, Iter end ) 
    {
        static std::random_device device;
        static std::mt19937 gen(device());
        return select_randomly(start, end, gen);
    }

    template<typename GenType>
    GenType range_random( GenType left, GenType right )
    {
        static std::random_device device;
        static std::mt19937 gen(device());
        std::uniform_int_distribution dis(0, right - left);
        return left + dis(gen);
    }
}
}