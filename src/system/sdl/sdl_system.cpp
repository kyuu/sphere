#include <cassert>
#include <cmath>
#include <sstream>
#include <SDL.h>
#include <SDL_OpenGL.h>
#include "../../version.hpp"
#include "../video.hpp"



//-----------------------------------------------------------------
// globals
SDL_Window* g_Window = 0;
int g_WindowWidth  = 0;
int g_WindowHeight = 0;
GLint g_MaxTextureSize = 0;
SDL_GLContext g_GLContext;

//-----------------------------------------------------------------
void GetSupportedVideoModes(std::vector<Dim2i>& out)
{
    out.clear();
    int num_modes = SDL_GetNumDisplayModes(0);
    for (int i = 0; i < num_modes; ++i) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) == 0) {
            // see if the video mode is already listed
            bool vmode_not_yet_listed = true;
            for (int j = 0; j < (int)out.size(); j++) {
                if (out[j].width == mode.w && out[j].height == mode.h) {
                    vmode_not_yet_listed = false;
                }
            }
            if (vmode_not_yet_listed) {
                out.push_back(Dim2i(mode.w, mode.h));
            }
        }
    }
}

//-----------------------------------------------------------------
bool OpenWindow(int width, int height, bool fullscreen)
{
    assert(!g_Window);

    if (width * height <= 0) {
        return false;
    }

    // build default window title
    std::ostringstream oss;
    oss << "Sphere " << SPHERE_MAJOR << "." << SPHERE_MINOR;

    // setup flags
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    // open window
    g_Window = SDL_CreateWindow(oss.str().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!g_Window) {
        return false;
    }
    g_WindowWidth  = width;
    g_WindowHeight = height;

    // create OpenGL context
    g_GLContext = SDL_GL_CreateContext(g_Window);

    // initialize a 2D mode
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.375, 0.375, 0.0);

    // set the clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // set clipping
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, width, height);

    // initialize blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // disable depth testing
    glDisable(GL_DEPTH_TEST);

    // get max texture size
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_MaxTextureSize);

    return true;
}

//-----------------------------------------------------------------
bool IsWindowOpen()
{
    return g_Window != 0;
}

//-----------------------------------------------------------------
int GetWindowWidth()
{
    assert(g_Window);
    return g_WindowWidth;
}

//-----------------------------------------------------------------
int GetWindowHeight()
{
    assert(g_Window);
    return g_WindowHeight;
}

//-----------------------------------------------------------------
bool IsWindowFullscreen()
{
    assert(g_Window);
    return (SDL_GetWindowFlags(g_Window) & SDL_WINDOW_FULLSCREEN) != 0;
}

//-----------------------------------------------------------------
bool SetWindowFullscreen(bool fullscreen)
{
    assert(g_Window);
    return SDL_SetWindowFullscreen(g_Window, (fullscreen ? SDL_TRUE : SDL_FALSE)) == 0;
}

//-----------------------------------------------------------------
const char* GetWindowTitle()
{
    assert(g_Window);
    return SDL_GetWindowTitle(g_Window);
}

//-----------------------------------------------------------------
void SetWindowTitle(const char* title)
{
    assert(g_Window);
    if (title) {
        SDL_SetWindowTitle(g_Window, title);
    }
}

//-----------------------------------------------------------------
void SetWindowIcon(Canvas* canvas)
{
    assert(g_Window);
    assert(canvas);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xFF000000;
    Uint32 gmask = 0x00FF0000;
    Uint32 bmask = 0x0000FF00;
    Uint32 amask = 0x000000FF;
#else
    Uint32 rmask = 0x000000FF;
    Uint32 gmask = 0x0000FF00;
    Uint32 bmask = 0x00FF0000;
    Uint32 amask = 0xFF000000;
#endif
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)canvas->getPixels(),
                                                    canvas->getWidth(),
                                                    canvas->getHeight(),
                                                    Canvas::GetNumBytesPerPixel() * 8,
                                                    canvas->getPitch(),
                                                    rmask,
                                                    gmask,
                                                    bmask,
                                                    amask);
    if (surface) {
        SDL_SetWindowIcon(g_Window, surface);
        SDL_FreeSurface(surface);
    }
}

//-----------------------------------------------------------------
void ShowFrame()
{
    assert(g_Window);
    SDL_GL_SwapWindow(g_Window);
    glClear(GL_COLOR_BUFFER_BIT);
}


//-----------------------------------------------------------------
bool InitVideo(const Log& log)
{
    // initialize SDL if not already initialized
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        log.error() << "Failed initializing SDL: " << SDL_GetError();
        return false;
    }

    // hide hardware mouse cursor
    SDL_ShowCursor(0);

    // ensure SDL will be properly deinitialized
    atexit(SDL_Quit);

    return true;
}

//-----------------------------------------------------------------
void DeinitVideo()
{
    if (g_Window) {
        SDL_GL_DeleteContext(g_GLContext); // if a window is open, assume a OpenGL context was created as well
        SDL_DestroyWindow(g_Window);
        g_Window        = 0;
        g_WindowWidth  = 0;
        g_WindowHeight = 0;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
