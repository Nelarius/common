#pragma once

#include "memory_arena.h"
#include "aliases.h"
#include "stl/filesystem.h"

#include <functional>
#include <memory>
#include <string>

namespace nlrs
{

class file_sentry_impl;

class file_sentry
{
public:
    enum class action
    {
        add,
        remove,
        modified
    };

    using handle = uptr;

    static constexpr uptr invalid_handle{ 0u };

    using event_callback =
        std::function<void(handle, const std::fs::path& directory, const std::fs::path& filename, action actions)>;

    file_sentry(memory_arena&);
    ~file_sentry();

    file_sentry() = delete;
    file_sentry(const file_sentry&) = delete;
    file_sentry& operator=(const file_sentry&) = delete;
    file_sentry(file_sentry&&) = delete;
    file_sentry& operator=(file_sentry&&) = delete;

    handle add_sentry(const std::fs::path& directory, event_callback callback, bool recursive = true);

    void remove_sentry(handle handle);

    void update();

private:
    memory_arena& allocator_;
    std::unique_ptr<file_sentry_impl> impl_;
};

}
