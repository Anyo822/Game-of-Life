#pragma once

#include <random>

class Randomizer
{
public:
    template <typename T>
    T Random(T from, T to)
    {
        std::uniform_int_distribution<T> distr(from, to);
        return distr(mersenneTwister);
    }

    template <typename T>
    void Seed(T seed)
    {
        mersenneTwister.seed(seed);
    }

private:
    std::mt19937 mersenneTwister;
};