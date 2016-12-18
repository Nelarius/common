#pragma once

#include "nlrsAliases.h"

namespace nlrs {

inline u64 nextPowerOf2(u64 n) {
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
