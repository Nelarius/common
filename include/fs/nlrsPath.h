#pragma once

#include "../nlrsAssert.h"
#include "../nlrsConfiguration.h"

#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <cstring>

#if NLRS_PLATFORM == NLRS_WIN32
#define NOMINMAX
#include "windows.h"
#undef near
#undef far
#else
#include <unistd.h>
#endif
#include <sys/stat.h>

namespace nlrs
{
namespace fs
{

class Path
{
public:

    // TODO: make this an enum class?
    enum class PathType
    {
        Windows,
        Posix,
#if defined(_WIN32)
        Native = Windows
#else
        Native = Posix
#endif
    };

    Path()
        : path_(),
        type_(PathType::Native),
        absolute_(false)
    {}

    Path(const std::string& path)
        : path_(),
        type_(PathType::Native),
        absolute_(false)
    {
        if (path.find_first_of("\\", 0) != std::string::npos)
        {
            type_ = PathType::Windows;
        }
        else if (path.find_first_of('/', 0) != std::string::npos)
        {
            type_ = PathType::Posix;
        }
        set_(path, type_);
    }

    Path(const char* path)
        : path_(),
        type_(PathType::Native),
        absolute_(false)
    {
        std::string p(path);
        if (p.find_first_of("\\", 0) != std::string::npos)
        {
            type_ = PathType::Windows;
        }
        else if (p.find_first_of('/', 0) != std::string::npos)
        {
            type_ = PathType::Posix;
        }
        set_(p, type_);
    }

    Path(const std::string& path, PathType type)
        : path_(),
        type_(type),
        absolute_(false)
    {
        NLRS_ASSERT(path.find_first_of((type_ == PathType::Windows ? "/" : "\\"), 0) == std::string::npos);
        set_(path, type_);
    }

    Path(const char* path, PathType type)
        : path_(),
        type_(type),
        absolute_(false)
    {
        std::string p(path);
        NLRS_ASSERT(p.find_first_of((type_ == PathType::Windows ? "/" : "\\"), 0) == std::string::npos);
        set_(p, type_);
    }

    Path(const Path& other)
        : path_(other.path_),
        type_(other.type_),
        absolute_(other.absolute_)
    {}

    Path(Path&& other)
        : path_(std::move(other.path_)),
        type_(other.type_),
        absolute_(other.absolute_)
    {
        other.absolute_ = false;
    }

    Path& operator=(const Path& rhs)
    {
        path_ = rhs.path_;
        type_ = rhs.type_;
        absolute_ = rhs.absolute_;

        return *this;
    }

    Path& operator=(Path&& rhs)
    {
        if (this != &rhs)
        {
            path_ = std::move(rhs.path_);
            type_ = rhs.type_;
            absolute_ = rhs.absolute_;
            rhs.absolute_ = false;
        }

        return *this;
    }

    void append(const char* path)
    {
        auto tokens = tokenize_(path, type_ == PathType::Windows ? "\\" : "/");
        for (const auto& t : tokens)
        {
            path_.push_back(t);
        }
    }

    void clear()
    {
        path_.clear();
    }

    bool isAbsolute() const { return absolute_; }

    std::string string(PathType type = PathType::Native) const
    {
        std::ostringstream ss;

        if (type_ == PathType::Posix && absolute_)
        {
            ss << '/';
        }

        for (std::size_t i = 0u; i < path_.size(); ++i)
        {
            ss << path_[i];
            if (i + 1u < path_.size())
            {
                if (type_ == PathType::Posix)
                {
                    ss << '/';
                }
                else
                {
                    ss << "\\";
                }
            }
        }

        return ss.str();
    }

#if NLRS_PLATFORM == NLRS_WIN32
    std::wstring wstring(PathType type = PathType::Native) const
    {
        std::string temp = string(type);
        int size = MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), NULL, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), &result[0], size);
        return result;
    }
#endif

protected:
    static std::vector<std::string> tokenize_(const std::string& str, std::string delim)
    {
        std::string::size_type lastPos = 0;
        std::string::size_type pos = str.find_first_of(delim, lastPos);
        std::vector<std::string> tokens;

        while (lastPos != std::string::npos)
        {
            if (pos != lastPos)
            {
                tokens.push_back(str.substr(lastPos, pos - lastPos));
            }
            lastPos = pos;
            if (lastPos == std::string::npos || lastPos + 1 == str.length())
            {
                break;
            }
            pos = str.find_first_of(delim, ++lastPos);
        }

        return tokens;
    }

    void set_(const std::string& p, PathType type)
    {
        type_ = type;
        if (type_ == PathType::Windows)
        {
            absolute_ = p.size() >= 2 && std::isalpha(p[0]) && p[1] == ':';
            path_ = tokenize_(p, "\\");
        }
        else
        {
            absolute_ = !p.empty() && p[0] == '/';
            path_ = tokenize_(p, "/");
        }
    }

    // TODO: windows widestring variant of set_

    std::vector<std::string>    path_;
    PathType                    type_;
    bool                        absolute_;
};

}
}
