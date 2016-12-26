#include "nlrsSDLWindow.h"
#include "nlrsAssert.h"
#include "SDL.h"

namespace nlrs
{

Window::~Window()
{
    if (window_)
    {
        SDL_DestroyWindow(window_);
    }
}

bool Window::initialize(const Options& opts)
{
    static_assert(Resizable == SDL_WINDOW_RESIZABLE, "Invalid Window::Flag value");
    static_assert(Opengl == SDL_WINDOW_OPENGL, "Invalid Window::Flag value");
    NLRS_ASSERT(window_ == nullptr);
    window_ = SDL_CreateWindow(
        opts.name.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        opts.width, opts.height,
        opts.flags
    );

    if (window_ == NULL)
    {
        return false;
    }

    return true;
}

}
