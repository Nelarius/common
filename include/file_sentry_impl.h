#pragma once

#include "memory_arena.h"
#include "configuration.h"
#include "file_sentry.h"
#include "object_pool.h"

#if NLRS_PLATFORM == NLRS_WIN32
#define NOMINMAX
#include "windows.h"
#include <algorithm>
#endif

namespace nlrs
{

#if NLRS_OS == NLRS_WINDOWS

#define BUFFER_SIZE 1024

struct Sentry
{
    OVERLAPPED overlappedInfo;
    HANDLE directoryHandle;
    DWORD notifyFilter;
    std::aligned_storage<1u, alignof(u32)>::type buffer[BUFFER_SIZE];
    file_sentry::event_callback callback;
    std::fs::path directoryPath;
    bool recursive;
    file_sentry::handle sentryHandle;
    bool stopNow;

    Sentry(HANDLE dHandle, int nFilter, file_sentry::event_callback cb, const std::fs::path& dirPath, bool rec)
        : overlappedInfo(),
        directoryHandle(dHandle),
        notifyFilter(nFilter),
        buffer(),
        callback(cb),
        directoryPath(dirPath),
        recursive(rec),
        sentryHandle(file_sentry::invalid_handle),
        stopNow(false)
    {
        overlappedInfo = { 0 };
    }
    ~Sentry() = default;
};

bool refreshSentry(Sentry&, bool);

void CALLBACK onCompletion(DWORD errorCode, DWORD numBytesTransferred, OVERLAPPED* overlapped)
{
    if (numBytesTransferred == 0u)
    {
        return;
    }

    // the overlapped field is the first field, so we can do this
    Sentry& sentry = *reinterpret_cast<Sentry*>(overlapped);
    if (errorCode == ERROR_SUCCESS)
    {
        FILE_NOTIFY_INFORMATION* notify = nullptr;
        usize offset = 0u;
        char szFile[MAX_PATH] = { 0 };

        do
        {
            notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&sentry.buffer[offset]);
            offset += notify->NextEntryOffset;

            // the file name is stored in the unicode format
            // but the path construct only takes multi-byte strings
            int count = WideCharToMultiByte(CP_ACP, 0, notify->FileName,
                notify->FileNameLength / sizeof(WCHAR), (LPSTR)szFile, MAX_PATH - 1, nullptr, nullptr);

            file_sentry::action action;
            switch (notify->Action)
            {
            case FILE_ACTION_RENAMED_NEW_NAME:
            case FILE_ACTION_ADDED:
                action = file_sentry::action::add;
                break;
            case FILE_ACTION_RENAMED_OLD_NAME:
            case FILE_ACTION_REMOVED:
                action = file_sentry::action::remove;
                break;
            case FILE_ACTION_MODIFIED:
                action = file_sentry::action::modified;
                break;
            }

            sentry.callback(sentry.sentryHandle, sentry.directoryPath, szFile, action);

        } while (notify->NextEntryOffset != 0);
    }

    if (!sentry.stopNow)
    {
        refreshSentry(sentry, false);
    }
}

bool refreshSentry(Sentry& sentry, bool clear)
{
    return ReadDirectoryChangesW(
        sentry.directoryHandle,
        &sentry.buffer[0], DWORD(BUFFER_SIZE),
        sentry.recursive,
        sentry.notifyFilter,
        nullptr, // meaningless for asynchronous calls
        &sentry.overlappedInfo,
        clear ? nullptr : onCompletion)
        != 0;
}

class file_sentry_impl
{
public:
    file_sentry_impl(memory_arena& alloc)
        : sentries_(alloc)
    {}

    ~file_sentry_impl() = default;

    file_sentry::handle add_sentry(
        const std::fs::path& directory,
        file_sentry::event_callback eventHandle,
        bool recursive)
    {
        Sentry* sentry = file_sentry::invalid_handle;

        HANDLE dirHandle = INVALID_HANDLE_VALUE;
        {
            auto dirName = directory.wstring();
            dirHandle = CreateFile(
                (LPCTSTR)dirName.c_str(),                                   /*file name*/
                FILE_LIST_DIRECTORY,                                        /*access mode*/
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,     /*share mode*/
                nullptr,                                                    /*security attributes*/
                OPEN_EXISTING,                                              /*creation action to be done for new file*/
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,          /*flags TODO: could this just be FILE_ATTRIBUTE_NORMAL?*/
                nullptr);                                                   /*template, ignored when opening an existing file*/
        }

        if (dirHandle != INVALID_HANDLE_VALUE)
        {
            sentry = sentries_.create(
                dirHandle,
                FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME,
                eventHandle,
                directory,
                recursive
            );

            sentry->overlappedInfo.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            if (sentry != nullptr && !refreshSentry(*sentry, false))
            {
                sentries_.release(sentry);
            }
        }

        return reinterpret_cast<uptr>(sentry);
    }

    void remove_sentry(file_sentry::handle handle)
    {
        if (handle == file_sentry::invalid_handle)
        {
            return;
        }

        Sentry* sentry = reinterpret_cast<Sentry*>(handle);

        sentry->stopNow = true;

        CancelIo(sentry->directoryHandle);

        refreshSentry(*sentry, true);

        if (!HasOverlappedIoCompleted(&sentry->overlappedInfo))
        {
            SleepEx(5, TRUE);
        }

        CloseHandle(sentry->overlappedInfo.hEvent);
        CloseHandle(sentry->directoryHandle);

        sentries_.release(sentry);
    }

    void update()
    {
        MsgWaitForMultipleObjectsEx(0, NULL, 0, QS_ALLINPUT, MWMO_ALERTABLE);
    }

private:
    object_pool<Sentry> sentries_;
};

#undef BUFFER_SIZE

#endif

#if NLRS_OS == NLRS_MACOSX
class file_sentry_impl
{
public:
    file_sentry_impl(memory_arena& alloc)
    {
        // TODO
    }

    file_sentry::handle add_sentry(const Path& directory, file_sentry::event_callback eventHandle, bool recursive)
    {
        // TODO
        return file_sentry::invalid_handle;
    }

    void remove_sentry(file_sentry::handle handle)
    {
        // TODO
    }

    void update()
    {
        // TODO
    }

private:

};

#endif

#if NLRS_OS == NLRS_LINUX

class file_sentry_impl
{
public:
    file_sentry_impl(memory_arena& alloc)
    {
        // TODO
    }

    file_sentry::handle add_sentry(const Path& directory, file_sentry::event_callback eventHandle, bool recursive)
    {
        // TODO
        return file_sentry::invalid_handle;
    }

    void remove_sentry(file_sentry::handle handle)
    {
        // TODO
    }

    void update()
    {
        // TODO
    }

private:

};

#endif

}
