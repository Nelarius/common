#pragma once

#include "../memory_arena.h"
#include <string>

namespace std
{
namespace pmr
{

template <class CharT, class Traits = std::char_traits<CharT>>
using basic_string = std::basic_string< CharT, Traits, nlrs::polymorphic_allocator<CharT>>;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;

}
}
