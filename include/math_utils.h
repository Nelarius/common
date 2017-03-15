#pragma once

#include <algorithm>

namespace nlrs
{

template<typename T>
constexpr T degrees_to_radians(T degrees)
{
    return (degrees * T(3.14159265358979323846)) / T(180);
}

template<typename T>
constexpr T radians_to_degrees(T radians)
{
    return (radians * T(180)) / T(3.14159265358979323846);
}

template<typename T>
T clamp(T t, T min, T max)
{
    return std::max(min, std::min(t, max));
}

}
