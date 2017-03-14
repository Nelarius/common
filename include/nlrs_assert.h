#pragma once

#ifdef NLRS_DEBUG
#include <cassert>
#define NLRS_ASSERT(_EXPR) assert(_EXPR)
#else
#define NLRS_ASSERT(_EXPR)
#endif
