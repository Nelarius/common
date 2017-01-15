#pragma once

#include "nlrsAliases.h"
#include "nlrsLocator.h"

#include <string>

struct SDL_Window;

namespace nlrs
{

class SdlWindow
{
public:
    enum Flags
    {
        Opengl = 2u,
        Resizable = 32u
    };

    struct Options
    {
        std::string name{ "nlrs" };
        int width{ 1000 };
        int height{ 800 };
        u32 flags{ Resizable | Opengl };
    };

    SdlWindow() = default;
    ~SdlWindow();

    SdlWindow(const SdlWindow&) = delete;
    SdlWindow(SdlWindow&&) = delete;
    SdlWindow& operator=(const SdlWindow&) = delete;
    SdlWindow& operator=(SdlWindow&&) = delete;

    bool initialize(const Options& opts);

    inline SDL_Window* ptr() { return window_; }

private:
    SDL_Window*     window_{ nullptr };
};

using WindowLocator = Locator<SdlWindow>;

}
