#pragma once

#include "aliases.h"
#include "locator.h"
#include "vector.h"

#include <string>

struct SDL_Window;

namespace nlrs
{

class sdl_window
{
public:
    enum flags
    {
        opengl = 2,
        resizable = 32,
        highdpi = 8192
    };

    struct options
    {
        std::string name{ "nlrs" };
        int width{ 1000 };
        int height{ 800 };
        u32 flags{ resizable | opengl | highdpi };
    };

    sdl_window();
    ~sdl_window();

    sdl_window(const sdl_window&) = delete;
    sdl_window(sdl_window&&) = delete;
    sdl_window& operator=(const sdl_window&) = delete;
    sdl_window& operator=(sdl_window&&) = delete;

    bool initialize(const options& opts);

    inline SDL_Window* ptr() { return window_; }
    inline vec2i size() const { return size_; }

private:
    SDL_Window* window_;
    vec2i size_;
};

using window_locator = locator<sdl_window>;

}
