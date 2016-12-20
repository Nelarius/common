#pragma once

#include "nlrsAliases.h"
#include "nlrsLocator.h"
#include "SDL.h"
#include <string>

namespace nlrs {

class Window {
public:
    enum Flags {
        Opengl = 2u,
        Resizable = 32u
    };

    struct Options {
        std::string name{ "nlrs" };
        int width{ 1000 };
        int height{ 800 };
        u32 flags{ Resizable | Opengl };
    };

    Window() = default;
    ~Window();

    Window(const Window&)               = delete;
    Window(Window&&)                    = delete;
    Window& operator=(const Window&)    = delete;
    Window& operator=(Window&&)         = delete;

    bool initialize(const Options& opts);

    inline SDL_Window* rawWindow() { return window_; }

private:
    SDL_Window*     window_{ nullptr };
};

using WindowLocator = Locator<Window>;

}
