#pragma once

#include "DebugLog.h"

#ifdef NDEBUG
#define FL_Assert(x, ...)
#else
#define FL_Assert(x, ...) { if(!(x)) { FL_Fatal(__VA_ARGS__); __debugbreak(); } }
#endif