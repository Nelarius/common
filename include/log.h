#pragma once

#include "nlrs_assert.h"
#include "nlrsVector.h"
#include "quaternion.h"
#include "geometry.h"
#include <sstream>
#include <ctime>
#include <cctype>   // for isspace
#include <iostream>
#include <algorithm>

namespace nlrs
{

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector2<T>& v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector3<T>& v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector4<T>& v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const quaternion<T>& q)
{
    os << "[(" << q.v.x << ", " << q.v.y << ", " << q.v.z << "), " << q.w << "]";
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const bounds2<T>& b)
{
    os << "(min: " << b.min << ", " << "max: " << b.max << ")";
    return os;
}

enum log_level
{
    inhibit = 0,
    error,
    warning,
    info,
    debug,
    debug2,
    debug3,
    debug4,
    all
};

inline std::string log_level_to_string(log_level level)
{
    switch (level)
    {
        case log_level::inhibit:   return "Inhibit";   break;
        case log_level::error:     return "error";     break;
        case log_level::warning:   return "warning";   break;
        case log_level::info:      return "info";      break;
        case log_level::debug:     return "debug";     break;
        case log_level::debug2:    return "debug2";    break;
        case log_level::debug3:    return "debug3";    break;
        case log_level::debug4:    return "debug4";    break;
        case log_level::all:       return "all";       break;
        default:                   return "all";
    }
}

inline log_level string_to_log_level(const std::string& level)
{
    if (level == "Inhibit")
    {
        return log_level::inhibit;
    }
    else if (level == "error")
    {
        return log_level::error;
    }
    else if (level == "warning")
    {
        return log_level::warning;
    }
    else if (level == "info")
    {
        return log_level::info;
    }
    else if (level == "debug")
    {
        return log_level::debug;
    }
    else if (level == "debug2")
    {
        return log_level::debug2;
    }
    else if (level == "debug3")
    {
        return log_level::debug3;
    }
    else if (level == "debug4")
    {
        return log_level::debug4;
    }
    else if (level == "all")
    {
        return log_level::all;
    }
    else
    {
        return log_level::all;
    }
}

inline std::string now_time()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%H:%M:%S", timeinfo);
    std::string str(buffer);
    return str;
}


class log
{

public:
    log() = default;
    ~log()
    {
        os_ << std::endl;
        std::cerr << os_.str();
        std::cerr.flush();  //syncrohize underlying device
    }

    inline static log_level& reporting_level()
    {
        static log_level level{ log_level::debug4 };
        return level;
    }

    inline std::ostringstream& get(log_level level = log_level::info)
    {
        //TO-DO: use std::put_time here to format the time output.
        os_ << " [" << now_time() << " " << log_level_to_string(level) << "] ";
        os_ << std::string(level < log_level::debug ? 0 : level - log_level::debug, '\t');
        return os_;
    }

private:
    std::ostringstream os_{};
};

}

#define LOG(level) \
if ( level > nlrs::log::reporting_level() ) ; \
else nlrs::log().get( level )

#define LOG_ERROR LOG(nlrs::log_level::error)
#define LOG_WARNING LOG(nlrs::log_level::warning)
#define LOG_INFO LOG(nlrs::log_level::info)
#define LOG_DEBUG LOG(nlrs::log_level::debug)
#define LOG_DEBUG2 LOG(nlrs::log_level::debug2)
#define LOG_DEBUG3 LOG(nlrs::log_level::debug3)
#define LOG_DEBUG4 LOG(nlrs::log_level::debug4)
