#pragma once

#include "memory_arena.h"
#include "aliases.h"
#include "stl/filesystem.h"

#include <functional>
#include <memory>
#include <string>

namespace nlrs
{

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
        std::function<void(Handle, const std::fs::path& directory, const std::fs::path& filename, Action actions)>;

    FileSentry(memory_arena&);
    ~FileSentry();

    FileSentry() = delete;
    FileSentry(const FileSentry&) = delete;
    FileSentry& operator=(const FileSentry&) = delete;
    FileSentry(FileSentry&&) = delete;
    FileSentry& operator=(FileSentry&&) = delete;

    Handle addSentry(const std::fs::path& directory, EventCallback callback, bool recursive = true);

    void removeSentry(Handle handle);

    void update();

private:
    memory_arena& allocator_;
    FileSentryImpl* impl_;
};

}
