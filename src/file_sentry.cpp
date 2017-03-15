#include "file_sentry.h"
#include "fs/file_sentry_impl.h"

namespace nlrs
{

file_sentry::file_sentry(memory_arena& alloc)
    : allocator_(alloc),
    impl_(std::make_unique<file_sentry_impl>(alloc))
{}

file_sentry::~file_sentry()
{}

file_sentry::handle file_sentry::add_sentry(
    const std::fs::path& directory,
    file_sentry::event_callback eventHandler,
    bool recursive)
{
    return impl_->add_sentry(directory, eventHandler, recursive);
}

void file_sentry::remove_sentry(file_sentry::handle handle)
{
    impl_->remove_sentry(handle);
}

void file_sentry::update()
{
    impl_->update();
}

}
