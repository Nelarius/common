#pragma once

#include "aliases.h"

namespace nlrs {

inline u64 next_power_of_two(u64 n) {
    // if given 0, this returns 0
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;
    return n;
}

}
