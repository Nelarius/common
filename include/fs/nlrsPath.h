#pragma once

#include "../nlrsConfiguration.h"

#include <string>
#include <vector>

namespace nlrs
{
namespace fs
{

class Path
{
public:

    enum class Type
    {
        Windows,
        Posix,
#if NLRS_PLATFORM == NLRS_WIN32
        Native = Windows
#else
        Native = Posix
#endif
    };

    Path();
    Path(const std::string& path);
    Path(const char* path);
    Path(const wchar_t* path);
    Path(const std::string& path, Type type);
    Path(const char* path, Type type);
    Path(const Path& other);
    Path(Path&& other);

    Path& operator=(const Path& rhs);
    Path& operator=(Path&& rhs);

    void append(const Path& path);

    void clear();
    bool isAbsolute() const;
    bool isRelative() const;

    std::string string(Type type = Type::Native) const;

#if NLRS_PLATFORM == NLRS_WIN32
    std::wstring wstring(Type type = Type::Native) const;
#endif

protected:
    static std::vector<std::string> tokenize(const std::string& str, Type type);

    void set(const std::string& p);

    void parseTypeAndSet(const std::string& p);

    // TODO: windows widestring variant of set_

    std::vector<std::string> path_;
    Type                     type_;
    bool                     absolute_;
};

}
}
