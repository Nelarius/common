#pragma once

#include "aliases.h"
#include <random>

namespace nlrs
{

template<typename T, bool IsIntegral>
class uniform_distribution
{
public:
    uniform_distribution() = default;
    ~uniform_distribution() = default;

    T operator()(std::minstd_rand& generator, T a, T b)
    {
        using param_t = typename decltype(distr_)::param_type;
        return distr_(generator, param_t(a, b));
    }

private:
    std::uniform_int_distribution<T> distr_;
};

template<typename T>
class uniform_distribution<T, false>
{
public:
    uniform_distribution() = default;
    ~uniform_distribution() = default;

    T operator()(std::minstd_rand& generator, T a, T b)
    {
        using param_t = typename decltype(distr_)::param_type;
        return distr_(generator, param_t(a, b));
    }

private:
    std::uniform_real_distribution<T> distr_;
};

// A minimum standard Park & Miller random number generator
// for uniform distributions
template<typename T>
class random
{
public:
    random() = default;
    ~random() = default;

    void seed(u32 s)
    {
        generator_.seed(s);
    }

    void randomize()
    {
        std::random_device rd;
        generator_.seed(rd());
    }

    T operator()(T a, T b)
    {
        return distribution_(generator_, a, b);
    }

    T operator()()
    {
        return distribution_(generator_, T(0.f), T(1.f));
    }

private:
    std::minstd_rand generator_;
    uniform_distribution<T, std::is_integral<T>::value> distribution_;
};

}
