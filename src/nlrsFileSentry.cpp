#include "nlrsFileSentry.h"
#include "fs/nlrsFileSentryImpl.h"

namespace nlrs
{

FileSentry::FileSentry(memory_arena& alloc)
    : allocator_(alloc),
    impl_(nullptr)
{
    impl_ = new (alloc.allocate(sizeof(FileSentryImpl), alignof(FileSentryImpl))) FileSentryImpl(alloc);
}

FileSentry::~FileSentry()
{
    impl_->~FileSentryImpl();
    allocator_.free(impl_);
}

FileSentry::Handle FileSentry::addSentry(
    const std::fs::path& directory,
    FileSentry::EventCallback eventHandler,
    bool recursive)
{
    return impl_->addSentry(directory, eventHandler, recursive);
}

void FileSentry::removeSentry(FileSentry::Handle handle)
{
    impl_->removeSentry(handle);
}

void FileSentry::update()
{
    impl_->update();
}

}
