#pragma once

#include "nlrsPath.h"
#include "../nlrsAllocator.h"
#include "../nlrsConfiguration.h"
#include "../nlrsLog.h"

#if NLRS_PLATFORM == NLRS_WIN32
#define NOMINMAX
#include "windows.h"
#undef near
#undef far
#else
#include <dirent.h>
#include <unistd.h> // standard symbols and constants
#include <ftw.h>    // file tree traversal
#endif
#include <sys/stat.h>

#include <stdexcept>

namespace nlrs
{
namespace fs
{

inline bool exists(const Path& p)
{
#if NLRS_PLATFORM == NLRS_WIN32
    return GetFileAttributesW(p.wstring().c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat sb;
    return stat(p.string().c_str(), &sb) == 0;
#endif
}

inline std::size_t fileSize(const Path& p)
{
#if NLRS_PLATFORM == NLRS_WIN32
    struct _stati64 sb;
    if (_wstati64(p.wstring().c_str(), &sb) != 0)
    {
        throw std::runtime_error("fileSize: cannot stat file \"" + p.string() + "\"!");
    }
#else
    struct stat sb;
    if (stat(p.string().c_str(), &sb) != 0)
        throw std::runtime_error("fileSize: cannot stat file \"" + p.string() + "\"!");
#endif
    return (size_t)sb.st_size;
}

inline bool isDirectory(const Path& p)
{
#if NLRS_PLATFORM == NLRS_WIN32
    DWORD attrib = GetFileAttributesW(p.wstring().c_str());
    if (attrib == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat sb;
    if (stat(p.string().c_str(), &sb) != 0)
    {
        return false;
    }
    return S_ISDIR(sb.st_mode);
#endif
}

inline bool isFile(const Path& p)
{
#if NLRS_PLATFORM == NLRS_WIN32
    DWORD attrib = GetFileAttributesW(p.wstring().c_str());
    return attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat sb;
    if (stat(p.string().c_str(), &sb) != 0)
    {
        return false;
    }
    return S_ISREG(sb.st_mode);
#endif
}

inline bool createDirectory(const Path& p)
{
#if NLRS_PLATFORM == NLRS_WIN32
    return CreateDirectoryW(p.wstring().c_str(), NULL) != 0;
#else
    return mkdir(p.string().c_str(), S_IRUSR | S_IWUSR | S_IXUSR) == 0;
#endif
}

namespace detail
{

#if NLRS_PLATFORM == NLRS_POSIX
inline int unlinkEntry(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = 0;
    if (S_ISDIR(sb->st_mode))
    {
        rv = rmdir(fpath);
    }
    else
    {
        rv = unlink(fpath);
    }

    return rv;
}
#endif

#if NLRS_PLATFORM == NLRS_WIN32
inline bool removeDirectoryContentIteratively(const Path& path)
{

    WIN32_FIND_DATAW fileData = { 0 };
    HANDLE file = INVALID_HANDLE_VALUE;

    Path p = path;
    p.append("*");
    auto widepath = p.wstring();
    file = FindFirstFileW(widepath.c_str(), &fileData);

    if (file == INVALID_HANDLE_VALUE)
    {
        // TODO: do something here?
        return false;
    }
    else
    {
        do
        {
            // MAX_PATH is 260 for wchar and char
            char multibyteName[261] = { 0 };
            if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                if (fileData.cFileName[0] == '.')
                {
                    continue;
                }
                wcstombs(multibyteName, fileData.cFileName, 260);
                removeDirectoryContentIteratively(Path(multibyteName));
            }
            else
            {
                Path filePath = path;
                wcstombs(multibyteName, fileData.cFileName, 260);
                filePath.append(multibyteName);
                int res = DeleteFileW(filePath.wstring().c_str());
            }
        } while (FindNextFileW(file, &fileData) != 0);
    }

    FindClose(file);

    return RemoveDirectoryW(path.wstring().c_str()) != 0;
}
#endif

}

// recursively delete a directory and all its contents
inline bool removeDirectory(const Path& p)
{
#if NLRS_PLATFORM == NLRS_WIN32
    // TODO: this doesn't recursively delete the folder contents!
    return detail::removeDirectoryContentIteratively(p);
#else
    // FTW_DEPTH: visit all files in a directory before reporting the directory itself
    // FTW_PHYS: visit only physical links, not symbolic ones
    return nftw(p.string().c_str(), detail::unlinkEntry, 64, FTW_DEPTH | FTW_PHYS) == 0;
#endif
}

inline bool removeFile(const Path& p)
{
#if NLRS_PLATFORM == NLRS_WIN32
    return DeleteFileW(p.wstring().c_str()) != 0;
#else
    return unlink(p.string().c_str()) == 0;
#endif
}

class DirectoryIterator
{
#if NLRS_PLATFORM == NLRS_WIN32
public:
    DirectoryIterator(const Path& path)
        : file_(INVALID_HANDLE_VALUE),
        fileData_(),
        refCount_(nullptr)
    {
        refCount_ = (unsigned*)SystemAllocator::getInstance().allocate(sizeof(unsigned), alignof(unsigned));
        *refCount_ = 1u;
        Path p = path;
        p.append("*");
        auto widepath = p.wstring();

        file_ = FindFirstFileW(widepath.c_str(), &fileData_);

        // the first entries are usually . and .., so let's skip them
        do
        {
            operator++();
        } while (fileData_.cFileName[0] == '.');
    }

    DirectoryIterator()
        : file_(INVALID_HANDLE_VALUE),
        fileData_(),
        refCount_(nullptr)
    {
        fileData_ = { 0 };
    }

    ~DirectoryIterator()
    {
        release_();
    }

    DirectoryIterator(const DirectoryIterator& other)
        : file_(other.file_),
        fileData_(other.fileData_),
        refCount_(other.refCount_)
    {
        retain_();
    }

    DirectoryIterator& operator=(const DirectoryIterator& rhs)
    {
        release_();

        file_ = rhs.file_;
        fileData_ = rhs.fileData_;
        refCount_ = rhs.refCount_;

        retain_();

        return *this;
    }

    DirectoryIterator(DirectoryIterator&& other)
        : file_(other.file_),
        fileData_(other.fileData_),
        refCount_(other.refCount_)
    {
        other.file_ = INVALID_HANDLE_VALUE;
        other.fileData_ = { 0 };
        other.refCount_ = nullptr;
    }

    inline DirectoryIterator& operator=(DirectoryIterator&& rhs)
    {
        release_();

        file_ = rhs.file_;
        fileData_ = rhs.fileData_;
        refCount_ = rhs.refCount_;

        rhs.file_ = INVALID_HANDLE_VALUE;
        rhs.fileData_ = { 0 };
        rhs.refCount_ = nullptr;

        return *this;
    }

    inline DirectoryIterator& operator++()
    {
        if (FindNextFileW(file_, &fileData_) == 0)
        {
            fileData_ = { 0 };
        }

        return *this;
    }

    inline bool operator==(const DirectoryIterator& rhs) const
    {
        if (fileData_.cFileName == 0)
        {
            return fileData_.cFileName == rhs.fileData_.cFileName;
        }
        return std::wcscmp(fileData_.cFileName, rhs.fileData_.cFileName) == 0;
    }

    inline bool operator!=(const DirectoryIterator& rhs) const
    {
        return !this->operator==(rhs);
    }

    inline Path operator*()
    {
        char multibyteName[261];
        wcstombs(multibyteName, fileData_.cFileName, 260);
        return Path(multibyteName);
    }

private:
    void retain_()
    {
        NLRS_ASSERT(refCount_ != nullptr);
        *refCount_ += 1u;
    }

    void release_()
    {
        if (refCount_)
        {
            *refCount_ -= 1u;
            if (*refCount_ == 0u)
            {
                FindClose(file_);
                SystemAllocator::getInstance().free(refCount_);
                refCount_ = nullptr;
            }
        }
    }

    HANDLE              file_;
    WIN32_FIND_DATAW    fileData_;
    unsigned*           refCount_;
#else
public:
    DirectoryIterator(const Path& p)
        : directory_(nullptr),
        entry_(nullptr),
        refCount_(nullptr)
    {
        DIR* dir;
        struct dirent* ent;
        if ((dir = opendir(p.string().c_str())) != NULL)
        {
            directory_ = dir;
            entry_ = readdir(directory_);
            refCount_ = (unsigned*)SystemAllocator::getInstance().allocate(sizeof(unsigned), alignof(unsigned));
            *refCount_ = 1u;

            // skip the . and .. entries
            do
            {
                operator++();
            } while (entry_->d_name[0] == '.');
        }
    }

    DirectoryIterator()
        : directory_(nullptr),
        entry_(nullptr),
        refCount_(nullptr)
    {}

    DirectoryIterator(const DirectoryIterator& other)
        : directory_(other.directory_),
        entry_(other.entry_),
        refCount_(other.refCount_)
    {
        retain_();
    }

    DirectoryIterator& operator=(const DirectoryIterator& rhs)
    {
        release_();

        directory_ = rhs.directory_;
        entry_ = rhs.entry_;
        refCount_ = rhs.refCount_;

        retain_();

        return *this;
    }

    DirectoryIterator(DirectoryIterator&& other)
        : directory_(other.directory_),
        entry_(other.entry_),
        refCount_(other.refCount_)
    {
        other.directory_ = nullptr;
        other.entry_ = nullptr;
        other.refCount_ = nullptr;
    }

    DirectoryIterator& operator=(DirectoryIterator&& rhs)
    {
        release_();

        directory_ = rhs.directory_;
        entry_ = rhs.entry_;
        refCount_ = rhs.refCount_;

        rhs.directory_ = nullptr;
        rhs.entry_ = nullptr;
        rhs.directory_ = nullptr;

        return *this;
    }

    ~DirectoryIterator()
    {
        release_();
    }

    inline DirectoryIterator& operator++()
    {
        NARWHAL_ASSERT(directory_);
        NARWHAL_ASSERT(entry_);
        if ((entry_ = readdir(directory_)) == NULL)
        {
            release_();
            directory_ = nullptr;
            refCount_ = nullptr;
        }
        return *this;
    }

    inline bool operator==(const DirectoryIterator& rhs) const
    {
        if (entry_ && rhs.entry_)
        {
            NARWHAL_ASSERT(entry_);
            return std::strcmp(entry_->d_name, rhs.entry_->d_name) == 0;
        }
        // see if both are null (end iterator)
        return directory_ == rhs.directory_;
    }

    inline bool operator!=(const DirectoryIterator& rhs) const
    {
        return !this->operator==(rhs);
    }

    inline Path operator*()
    {
        NARWHAL_ASSERT(directory_);
        NARWHAL_ASSERT(entry_);
        return Path(entry_->d_name);
    }

private:
    void retain_()
    {
        *refCount_ += 1u;
    }

    void release_()
    {
        if (refCount_)
        {
            *refCount_ -= 1u;
            if (*refCount_ == 0u)
            {
                SystemAllocator::getInstance().free(refCount_);
                closedir(directory_);
            }
        }
    }

    inline void increment_()
    {
        if ((entry_ = readdir(directory_)) == NULL)
        {
            closedir(directory_);
            directory_ = nullptr;
        }
    }

    DIR*            directory_;
    struct dirent*  entry_;
    unsigned*       refCount_;
#endif
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
