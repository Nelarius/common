#pragma once

#include "../memory_arena.h"
#include <unordered_map>

namespace std
{
namespace pmr
{

template<class K, class V>
using unordered_map = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, nlrs::polymorphic_allocator<std::pair<const K, V>>>;

}
}
