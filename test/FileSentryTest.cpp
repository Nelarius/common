#include "nlrsFileSentry.h"
#include "UnitTest++/UnitTest++.h"

#include <experimental/filesystem>
#include <fstream>

namespace
{

void createTestFile(const char* name)
{
    std::ofstream out(name);
    out.write("1234", 5);
}

void appendTestFile(const char* name)
{
    std::ofstream out(name, std::ios_base::app);
    out.write("5678", 5);
}

}

namespace nlrs
{

struct TestDirWithSentry
{
    TestDirWithSentry()
        : sentry(system_arena::get_instance())
    {
        std::fs::create_directory("test_dir");
    }

    ~TestDirWithSentry()
    {
        std::remove("test_dir");
    }

    FileSentry sentry;
};

SUITE(FileSentryTest)
{
    TEST_FIXTURE(TestDirWithSentry, CreatingFileAndWritingToFileResultsInAddModifyEvents)
    {
        int calls = 0;
        FileSentry::Action first = FileSentry::Action::Delete;
        FileSentry::Action second = FileSentry::Action::Delete;

        auto handle = sentry.addSentry(
        "test_dir",
        [&calls, &first, &second](
            FileSentry::Handle, const std::fs::path& directory,
            const std::fs::path& file, FileSentry::Action action
            ) -> void
        {
            calls++;

            if (calls == 1)
            {
                first = action;
            }

            if (calls == 2)
            {
                second = action;
            }
        });

        CHECK(handle != FileSentry::InvalidHandle);

        createTestFile("test_dir/test_file");

        sentry.update();

        CHECK(FileSentry::Action::Add == first);
        CHECK(FileSentry::Action::Modified == second);
        CHECK_EQUAL(2, calls);

        sentry.removeSentry(handle);

        std::remove("test_dir/test_file");
    }

    TEST_FIXTURE(TestDirWithSentry, RemovingFileResultsInRemoveEvent)
    {
        int calls = 0;
        bool fileRemoved = false;
        FileSentry::Action actionWas = FileSentry::Action::Add;

        createTestFile("test_dir/test_file");

        auto handle = sentry.addSentry(
        "test_dir",
            [&fileRemoved, &actionWas, &calls](FileSentry::Handle, const std::fs::path& directory,
                const std::fs::path& file, FileSentry::Action action) -> void
        {
            calls++;
            fileRemoved = true;
            actionWas = action;
        });

        CHECK(handle != FileSentry::InvalidHandle);

        std::remove("test_dir/test_file");

        sentry.update();

        CHECK(fileRemoved);
        CHECK(FileSentry::Action::Delete == actionWas);
        CHECK_EQUAL(1, calls);

        sentry.removeSentry(handle);
    }

    TEST_FIXTURE(TestDirWithSentry, ModifyingFileResultsInModifyEvent)
    {
        int calls = 0;
        FileSentry::Action actionWas = FileSentry::Action::Delete;

        createTestFile("test_dir/test_file");

        auto handle = sentry.addSentry(
            "test_dir",
            [&actionWas, &calls](FileSentry::Handle, const std::fs::path& directory,
                const std::fs::path& file, FileSentry::Action action) -> void
        {
            calls++;
            actionWas = action;
        });

        CHECK(handle != FileSentry::InvalidHandle);

        appendTestFile("test_dir/test_file");

        sentry.update();

        CHECK(FileSentry::Action::Modified == actionWas);
        CHECK_EQUAL(1, calls);

        sentry.removeSentry(handle);

        std::remove("test_dir/test_file");
    }

    TEST_FIXTURE(TestDirWithSentry, ModifyingFileInNestedDirResultsInAddEvent)
    {
        int calls = 0;
        FileSentry::Action actionWas = FileSentry::Action::Delete;

        std::fs::create_directory("test_dir/nested_dir");

        createTestFile("test_dir/nested_dir/test_file");

        auto handle = sentry.addSentry(
            "test_dir",
            [&actionWas, &calls](
                FileSentry::Handle, const std::fs::path& directory,
                const std::fs::path& file, FileSentry::Action action
                ) -> void
        {
            calls++;
            actionWas = action;
        });

        CHECK(handle != FileSentry::InvalidHandle);

        appendTestFile("test_dir/nested_dir/test_file");

        sentry.update();

        CHECK(FileSentry::Action::Modified == actionWas);
        CHECK_EQUAL(1, calls);

        sentry.removeSentry(handle);

        std::remove("test_dir/nested_dir");
    }
}

}
