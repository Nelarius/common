#pragma once

#include <algorithm>

namespace nlrs
{

template<typename T>
T clamp(T t, T min, T max)
{
    return std::max(min, std::min(t, max));
}

}
