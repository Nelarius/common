#include "nlrsAssert.h"
#include "fs/nlrsPath.h"

#include <sstream>
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

Path::Path()
    : path_(),
    type_(Type::Native),
    absolute_(false)
{}

Path::Path(const std::string& path)
    : path_(),
    type_(Type::Native),
    absolute_(false)
{
    parseTypeAndSet(path);
}

Path::Path(const char* path)
    : path_(),
    type_(Type::Native),
    absolute_(false)
{
    parseTypeAndSet(path);
}

Path::Path(const wchar_t* path)
    : path_(),
    type_(Type::Native),
    absolute_(false)
{
    // this constructor is intended for use on windows, where path strings
    // are stored in utf-16 format
    // convert here to utf-8
    std::string p;
    p.resize(260);
    std::string::size_type len = static_cast<std::size_t>(-1);
    if ((len = std::wcstombs(&p[0], path, 260)) == static_cast<std::size_t>(-1))
    {
        throw std::runtime_error("Path constructor: failed to convert path to multibyte representation");
    }
    p.resize(len);
    parseTypeAndSet(p);
}

Path::Path(const std::string& path, Type type)
    : path_(),
    type_(type),
    absolute_(false)
{
    NLRS_ASSERT(path.find_first_of((type_ == Type::Windows ? "/" : "\\"), 0) == std::string::npos);
    set(path);
}

Path::Path(const char* path, Type type)
    : path_(),
    type_(type),
    absolute_(false)
{
    std::string p(path);
    NLRS_ASSERT(p.find_first_of((type_ == Type::Windows ? "/" : "\\"), 0) == std::string::npos);
    set(p);
}

Path::Path(const Path& other)
    : path_(other.path_),
    type_(other.type_),
    absolute_(other.absolute_)
{}

Path::Path(Path&& other)
    : path_(std::move(other.path_)),
    type_(other.type_),
    absolute_(other.absolute_)
{
    other.absolute_ = false;
}

Path& Path::operator=(const Path& rhs)
{
    path_ = rhs.path_;
    type_ = rhs.type_;
    absolute_ = rhs.absolute_;

    return *this;
}

Path& Path::operator=(Path&& rhs)
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

void Path::append(const Path& path)
{
    if (path.isAbsolute())
    {
        throw std::runtime_error("Path.append: appending absolute paths is an error");
    }
    for (auto& token : path.path_)
    {
        NLRS_ASSERT(token[0] != '.');
        path_.push_back(token);
    }
}

void Path::clear()
{
    path_.clear();
}

bool Path::isAbsolute() const { return absolute_; }

bool Path::isRelative() const { return !absolute_; }

std::string Path::string(Type type) const
{
    std::ostringstream ss;

    if (type_ == Type::Posix && absolute_)
    {
        ss << '/';
    }

    for (std::size_t i = 0u; i < path_.size(); ++i)
    {
        ss << path_[i];
        if (i + 1u < path_.size())
        {
            if (type_ == Type::Posix)
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
std::wstring Path::wstring(Type type) const
{
    std::string temp = string(type);
    int size = MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), &result[0], size);
    return result;
}
#endif

std::vector<std::string> Path::tokenize(const std::string& str, Type type)
{
    std::string delim;
    if (type == Type::Windows)
    {
        delim = "\\";
    }
    else if (type == Type::Posix)
    {
        delim = "/";
    }
    else
    {
#if NLRS_PLATFORM == NLRS_WIN32
        delim = "\\";
#else
        delim = "/";
#endif
    }
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

void Path::set(const std::string& p)
{
    if (p.size() >= 2 && std::isalpha(p[0]) && p[1] == ':')
    {
        absolute_ = true;
    }
    else if (!p.empty() && p[0] == '/')
    {
        absolute_ = true;
    }
    path_ = tokenize(p, type_);
}

void Path::parseTypeAndSet(const std::string& p)
{
    if (p.find_first_of("\\", 0) != std::string::npos)
    {
        type_ = Type::Windows;
    }
    else if (p.find_first_of('/', 0) != std::string::npos)
    {
        type_ = Type::Posix;
    }
    set(p);
}

}
}
