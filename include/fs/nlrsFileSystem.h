#pragma once

#include "nlrsPath.h"
#include "nlrsMemory.h"

namespace nlrs
{
namespace fs
{

Path absolute(const Path& p);

bool exists(const Path& p);

std::size_t fileSize(const Path& p);

bool isDirectory(const Path& p);

bool isFile(const Path& p);

bool createDirectory(const Path& p);

// recursively delete a directory and all its contents
bool removeDirectory(const Path& p);

bool removeFile(const Path& p);

class DirectoryIterator
{
public:
    DirectoryIterator(const Path& path);
    DirectoryIterator();
    DirectoryIterator(const DirectoryIterator& other);
    DirectoryIterator(DirectoryIterator&& other);

    ~DirectoryIterator();

    DirectoryIterator& operator=(const DirectoryIterator& rhs);
    DirectoryIterator& operator=(DirectoryIterator&& rhs);

    DirectoryIterator& operator++();

    bool operator==(const DirectoryIterator& rhs) const;
    bool operator!=(const DirectoryIterator& rhs) const;

    Path operator*();

private:
    class DirectoryIteratorImpl;

    ScopedPtr<DirectoryIteratorImpl> implementation_;
};

inline DirectoryIterator begin(DirectoryIterator iter)
{
    return iter;
}

inline DirectoryIterator end(const DirectoryIterator&)
{
    return DirectoryIterator();
}

}
}
