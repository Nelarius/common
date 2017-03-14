#pragma once

#include "../memory_arena.h"
#include <vector>

namespace std
{
namespace pmr
{

template<class T>
using vector = std::vector<T, nlrs::polymorphic_allocator<T>>;

}
}
