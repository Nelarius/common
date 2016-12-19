
function test()
    project "test"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin"
    files { "test/**.cpp", "src/common/**.cpp" }
    includedirs { "src", "extern/unittest++", "extern" }
    debugdir "bin"
    filter "action:vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" } -- This is to turn off warnings about 'localtime'
    filter { "Debug", "action:vs*" }
        links { "UnitTest++" }
        libdirs { "extern/unittest++/lib/win64/Debug" }
    filter { "Release or Test", "action:vs*" }
        links { "UnitTest++" }
        libdirs { "extern/unittest++/lib/win64/Release" }

    filter "system:macosx"
        links { "UnitTest++" }
        libdirs { "extern/unittest++/lib/osx" }
end

function common()
    project "common"
    kind "StaticLib"
    language "C++"
    targetdir "lib"
    files { "include/**.h", "src/nlrsAllocator.cpp" }
end

function gl3w()
    project "gl3w"
    kind "StaticLib"
    language "C"
    targetdir "lib"
    files { "extern/gl3w/src/gl3w.c" }
    includedirs { "extern/gl3w/include" }
end
