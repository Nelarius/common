#include "sdl_window.h"
#include "nlrs_assert.h"
#include "SDL.h"

namespace
{

class sdl_global_resource
{
public:
    static void init()
    {
        NLRS_ASSERT(references_ >= 0);
        if (references_ == 0)
        {
            SDL_Init(SDL_INIT_VIDEO);
        }
        ++references_;
    }

    static void quit()
    {
        --references_;
        if (references_ == 0)
        {
            SDL_Quit();
        }
        NLRS_ASSERT(references_ >= 0);
    }

private:
    static int references_;
};

int sdl_global_resource::references_ = 0;

}

namespace nlrs
{

sdl_window::sdl_window()
    : window_(nullptr),
    size_(0, 0)
{
    sdl_global_resource::init();
}

sdl_window::~sdl_window()
{
    if (window_)
    {
        SDL_DestroyWindow(window_);
    }
    sdl_global_resource::quit();
}

bool sdl_window::initialize(const options& opts)
{
    size_ = vec2i(opts.width, opts.height);

    static_assert(resizable == SDL_WINDOW_RESIZABLE, "Invalid sdl_window::flag value");
    static_assert(opengl == SDL_WINDOW_OPENGL, "Invalid sdl_window::flag value");
    static_assert(highdpi == SDL_WINDOW_ALLOW_HIGHDPI, "Invalid sdl_window::flag value");
    NLRS_ASSERT(window_ == nullptr);
    window_ = SDL_CreateWindow(
        opts.name.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        opts.width, opts.height,
        opts.flags
    );

    if (window_ == nullptr)
    {
        return false;
    }

    return true;
}

}
