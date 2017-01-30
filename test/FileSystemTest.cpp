#include "fs/nlrsFileSystem.h"
#include "nlrsLiterals.h"
#include "UnitTest++/UnitTest++.h"

#include <fstream>

namespace nlrs {

SUITE(FileSystemTest) {

    TEST(CreateDirectorySucceedsAndDirectoryExists) {
        CHECK(fs::createDirectory("test_dir"));
        CHECK(fs::isDirectory("test_dir"));
        CHECK(fs::exists("test_dir"));
        CHECK(!fs::isFile("test_dir"));

        CHECK(fs::removeDirectory("test_dir"));
        CHECK(!fs::exists("test_dir"));
    }

    TEST(FileInfoCorrect) {
        std::ofstream out("test_text");
        out.write("1234", 5);
        out.close();

        CHECK(fs::exists("test_text"));
        CHECK(fs::isFile("test_text"));
        CHECK(!fs::isDirectory("test_text"));
        CHECK_EQUAL(5_sz, fs::fileSize("test_text"));

        CHECK(fs::removeFile("test_text"));
        CHECK(!fs::exists("test_text"));
    }

    TEST(RecursiveDirectoryRemovalWorks) {
        CHECK(fs::createDirectory("test_dir"));
        std::ofstream out("test_dir/test_text");
        out.write("1234", 5);
        out.close();

        fs::Path p("test_dir");
        p.append("test_text");

        CHECK(fs::isFile(p));

        CHECK(fs::removeDirectory("test_dir"));
        CHECK(!fs::exists("test_dir"));
    }

    TEST(WindowsAbsolutePathIsAbsolute)
    {
        fs::Path p1("C:\\test");
        CHECK(p1.isAbsolute());
        CHECK(!p1.isRelative());

        fs::Path p2("C:");
        CHECK(p2.isAbsolute());
        CHECK(!p2.isRelative());

        fs::Path p3("C:/test");
        CHECK(p3.isAbsolute());
        CHECK(!p3.isRelative());
    }

    TEST(PosixAbsolutePathIsAbsolute)
    {
        fs::Path p1("/bin/test");
        CHECK(p1.isAbsolute());
        CHECK(!p1.isRelative());

        fs::Path p2("/usr/");
        CHECK(p2.isAbsolute());
        CHECK(!p2.isRelative());
    }

    TEST(WindowsRelativePathIsRelative)
    {
        fs::Path p1("test\\");
        CHECK(p1.isRelative());
        CHECK(!p1.isAbsolute());

        fs::Path p2(".\\");
        CHECK(p2.isRelative());
        CHECK(!p2.isAbsolute());

        fs::Path p3("..\\test\\");
        CHECK(p3.isRelative());
        CHECK(!p3.isAbsolute());
    }

    TEST(PosixRelativePathIsRelative)
    {
        fs::Path p1("test");
        CHECK(p1.isRelative());
        CHECK(!p1.isAbsolute());

        fs::Path p2("./");
        CHECK(p2.isRelative());
        CHECK(!p2.isAbsolute());

        fs::Path p3("../test/");
        CHECK(p3.isRelative());
        CHECK(!p3.isAbsolute());
    }

    TEST(AbsolutePathConversionTest)
    {
        fs::Path p("./");
        CHECK(p.isRelative());

        auto absPath = fs::absolute(p);
        CHECK(absPath.isAbsolute());
    }
}

}
