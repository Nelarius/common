#pragma once

#include "nlrsAllocator.h"
#include "nlrsAliases.h"
#include "fs/nlrsPath.h"

#include <functional>
#include <string>

namespace nlrs
{

using fs::Path;

class FileSentryImpl;

class FileSentry
{
public:
    enum class Action
    {
        Add,
        Delete,
        Modified
    };

    using Handle = uptr;

    static constexpr uptr InvalidHandle{ 0u };

    using EventCallback =
        std::function<void(Handle, const Path& directory, const Path& filename, Action actions)>;

    FileSentry(IAllocator&);
    ~FileSentry();

    FileSentry() = delete;
    FileSentry(const FileSentry&) = delete;
    FileSentry& operator=(const FileSentry&) = delete;
    FileSentry(FileSentry&&) = delete;
    FileSentry& operator=(FileSentry&&) = delete;

    Handle addSentry(const Path& directory, EventCallback callback, bool recursive = true);

    void removeSentry(Handle handle);

    void update();

private:
    IAllocator& allocator_;
    FileSentryImpl* impl_;
};

}
