#ifndef __RANDOM_HPP
#define __RANDOM_HPP

#include <random>

inline int random_int(int a, int b)
{
    auto random_device = std::random_device();
    auto g = std::mt19937(random_device());
    std::uniform_int_distribution<> distrib(a, b);
    return distrib(g);
}

#endif
