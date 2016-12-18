#pragma once

#define NLRS_LINUX 0
#define NLRS_WINDOWS 1
#define NLRS_MACOSX 2

#define NLRS_POSIX 3
#define NLRS_WIN32 4

#define NLRS_UNKNOWN 0xFFFF

#if defined(__linux__)
    #define NLRS_OS NLRS_LINUX
    #define NLRS_OS_STRING "Linux"
    #define NLRS_PLATFORM NLRS_POSIX
#elif defined(_WIN32) || defined(_WIN64)
    #define NLRS_OS NLRS_WINDOWS
    #define NLRS_OS_STRING "Windows"
    #define NLRS_PLATFORM NLRS_WIN32
#elif defined(__APPLE__) || defined(MACOSX)
    #define NLRS_OS NLRS_MACOSX
    #define NLRS_OS_STRING "OSX"
    #define NLRS_PLATFORM NLRS_POSIX
#else
    #define NLRS_OS NLRS_UNKNOWN
    #define NLRS_OS_STRING "Unknown"
    #define NLRS_PLATFORM NLRS_UNKNOWN
#endif

namespace nlrs {
inline const char *getOsString() { return NLRS_OS_STRING; }
}
