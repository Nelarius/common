
function project_test(location)
    project "test"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin"
    files {
        location.."/common/test/**.cpp",
        location.."/common/src/memory_arena.cpp",
        location.."/common/src/file_sentry.cpp"
    }
    includedirs { location.."/common/extern/unittest++", location.."/common/include" }
    debugdir "bin"
    filter "action:vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" } -- This is to turn off warnings about 'localtime'
    filter { "Debug", "action:vs*" }
        links { "UnitTest++" }
        libdirs { location.."/common/extern/unittest++/lib/win64/Debug" }
    filter { "Release or Test", "action:vs*" }
        links { "UnitTest++" }
        libdirs { location.."/common/extern/unittest++/lib/win64/Release" }

    filter "system:macosx"
        links { "UnitTest++" }
        libdirs { location.."/common/extern/unittest++/lib/osx" }
end

function project_common(location)
    project "common"
    kind "StaticLib"
    language "C++"
    targetdir "lib"
    includedirs { location.."/common/include/" }
    files {
        location.."/common/include/**.h",
        location.."/common/src/memory_arena.cpp",
        location.."/common/src/file_sentry.cpp"
    }
end

function project_window(location, sdl_include)
    project "window"
    kind "StaticLib"
    language "C++"
    targetdir "lib"
    includedirs { location.."/common/include/", location.."/common/extern/gl3w/include", sdl_include }
    files {
        location.."/common/src/graphics_api.cpp",
        location.."/common/src/sdl_window.cpp"
    }
end

function project_gl3w(location)
    project "gl3w"
    kind "StaticLib"
    language "C"
    targetdir "lib"
    files { "extern/common/extern/gl3w/src/gl3w.c" }
    includedirs { location.."/common/extern/gl3w/include" }
end
