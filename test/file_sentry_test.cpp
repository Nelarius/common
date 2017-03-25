#include "file_sentry.h"
#include "UnitTest++/UnitTest++.h"

#include <experimental/filesystem>
#include <fstream>

namespace
{

void create_test_file(const char* name)
{
    std::ofstream out(name);
    out.write("1234", 5);
}

void append_test_file(const char* name)
{
    std::ofstream out(name, std::ios_base::app);
    out.write("5678", 5);
}

}

namespace nlrs
{

struct test_dir_with_sentry
{
    test_dir_with_sentry()
        : sentry(system_arena::get_instance())
    {
        std::fs::create_directory("test_dir");
    }

    ~test_dir_with_sentry()
    {
        std::remove("test_dir");
    }

    file_sentry sentry;
};

SUITE(file_sentry_test)
{
    TEST_FIXTURE(test_dir_with_sentry, creating_file_and_writing_to_file_results_in_add_modify_events)
    {
        int calls = 0;
        file_sentry::action first = file_sentry::action::remove;
        file_sentry::action second = file_sentry::action::remove;

        auto handle = sentry.add_sentry(
        "test_dir",
        [&calls, &first, &second](
            file_sentry::handle, const std::fs::path& directory,
            const std::fs::path& file, file_sentry::action action
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

        CHECK(handle != file_sentry::invalid_handle);

        create_test_file("test_dir/test_file");

        sentry.update();

        CHECK(file_sentry::action::add == first);
        CHECK(file_sentry::action::modified == second);
        CHECK_EQUAL(2, calls);

        sentry.remove_sentry(handle);

        std::remove("test_dir/test_file");
    }

    TEST_FIXTURE(test_dir_with_sentry, removing_file_results_in_remove_event)
    {
        int calls = 0;
        bool file_removed = false;
        file_sentry::action action_was = file_sentry::action::add;

        create_test_file("test_dir/test_file");

        auto handle = sentry.add_sentry(
        "test_dir",
            [&file_removed, &action_was, &calls](file_sentry::handle, const std::fs::path& directory,
                const std::fs::path& file, file_sentry::action action) -> void
        {
            calls++;
            file_removed = true;
            action_was = action;
        });

        CHECK(handle != file_sentry::invalid_handle);

        std::remove("test_dir/test_file");

        sentry.update();

        CHECK(file_removed);
        CHECK(file_sentry::action::remove == action_was);
        CHECK_EQUAL(1, calls);

        sentry.remove_sentry(handle);
    }

    TEST_FIXTURE(test_dir_with_sentry, modifying_file_results_in_modify_event)
    {
        int calls = 0;
        file_sentry::action action_was = file_sentry::action::remove;

        create_test_file("test_dir/test_file");

        auto handle = sentry.add_sentry(
            "test_dir",
            [&action_was, &calls](file_sentry::handle, const std::fs::path& directory,
                const std::fs::path& file, file_sentry::action action) -> void
        {
            calls++;
            action_was = action;
        });

        CHECK(handle != file_sentry::invalid_handle);

        append_test_file("test_dir/test_file");

        sentry.update();

        CHECK(file_sentry::action::modified == action_was);
        CHECK_EQUAL(1, calls);

        sentry.remove_sentry(handle);

        std::remove("test_dir/test_file");
    }

    TEST_FIXTURE(test_dir_with_sentry, modifying_file_in_nested_dir_results_in_add_event)
    {
        int calls = 0;
        file_sentry::action action_was = file_sentry::action::remove;

        std::fs::create_directory("test_dir/nested_dir");

        create_test_file("test_dir/nested_dir/test_file");

        auto handle = sentry.add_sentry(
            "test_dir",
            [&action_was, &calls](
                file_sentry::handle, const std::fs::path& directory,
                const std::fs::path& file, file_sentry::action action
                ) -> void
        {
            calls++;
            action_was = action;
        });

        CHECK(handle != file_sentry::invalid_handle);

        append_test_file("test_dir/nested_dir/test_file");

        sentry.update();

        CHECK(file_sentry::action::modified == action_was);
        CHECK_EQUAL(1, calls);

        sentry.remove_sentry(handle);

        std::remove("test_dir/nested_dir");
    }
}

}
