#pragma once

#include "aliases.h"
#include "nlrsLocator.h"
#include "nlrsVector.h"

#include <string>

struct SDL_Window;

namespace nlrs
{

class SdlWindow
{
public:
    enum Flags
    {
        Opengl = 2,
        Resizable = 32,
        HighDpi = 8192
    };

    struct Options
    {
        std::string name{ "nlrs" };
        int width{ 1000 };
        int height{ 800 };
        u32 flags{ Resizable | Opengl | HighDpi };
    };

    SdlWindow();
    ~SdlWindow();

    SdlWindow(const SdlWindow&) = delete;
    SdlWindow(SdlWindow&&) = delete;
    SdlWindow& operator=(const SdlWindow&) = delete;
    SdlWindow& operator=(SdlWindow&&) = delete;

    bool initialize(const Options& opts);

    inline SDL_Window* ptr() { return window_; }
    inline Vec2i size() const { return size_; }

private:
    SDL_Window* window_;
    Vec2i size_;
};

using WindowLocator = locator<SdlWindow>;

}
