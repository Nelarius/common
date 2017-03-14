#include "nlrsWindow.h"
#include "nlrs_assert.h"
#include "SDL.h"

namespace
{

class SdlGlobal
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

int SdlGlobal::references_ = 0;

}

namespace nlrs
{

SdlWindow::SdlWindow()
    : window_(nullptr),
    size_(0, 0)
{
    SdlGlobal::init();
}

SdlWindow::~SdlWindow()
{
    if (window_)
    {
        SDL_DestroyWindow(window_);
    }
    SdlGlobal::quit();
}

bool SdlWindow::initialize(const Options& opts)
{
    size_ = Vec2i(opts.width, opts.height);

    static_assert(Resizable == SDL_WINDOW_RESIZABLE, "Invalid SdlWindow::Flag value");
    static_assert(Opengl == SDL_WINDOW_OPENGL, "Invalid SdlWindow::Flag value");
    static_assert(HighDpi == SDL_WINDOW_ALLOW_HIGHDPI, "Invalid SdlWindow::Flag value");
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
