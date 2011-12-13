#include <cassert>
#include <cmath>
#include <sstream>
#include <SDL.h>
#include <SDL_OpenGL.h>
#include "../../version.hpp"
#include "../../error.hpp"
#include "../video.hpp"
#include "Texture.hpp"


//-----------------------------------------------------------------
// globals
SDL_Window* g_Window = 0;
int g_WindowWidth  = 0;
int g_WindowHeight = 0;
GLint g_MaxTextureSize = 0;
SDL_GLContext g_GLContext;

//-----------------------------------------------------------------
static bool is_sdl_pixel_format_supported(Uint32 format)
{
    if (format == SDL_PIXELFORMAT_BGR888   ||
        format == SDL_PIXELFORMAT_RGB888   ||
        format == SDL_PIXELFORMAT_BGRA8888 ||
        format == SDL_PIXELFORMAT_ABGR8888 ||
        format == SDL_PIXELFORMAT_RGBA8888 ||
        format == SDL_PIXELFORMAT_ARGB8888)
    {
        return true;
    }
    return false;
}

//-----------------------------------------------------------------
void GetSupportedVideoModes(std::vector<Dim2i>& out)
{
    out.clear();
    int num_modes = SDL_GetNumDisplayModes(0);
    for (int i = 0; i < num_modes; ++i) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) == 0) {
            if (is_sdl_pixel_format_supported(mode.format)) {
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
bool GetFrameScissor(Recti& scissor)
{
    assert(g_Window);
    GLint rect[4];
    glGetIntegerv(GL_SCISSOR_BOX, rect);
    scissor.ul.x = rect[0];
    scissor.ul.y = rect[1] - g_WindowHeight + rect[3];
    scissor.lr.x = (rect[0] + rect[2]) - 1;
    scissor.lr.y = (scissor.ul.y + rect[3]) - 1;
    return true;
}

//-----------------------------------------------------------------
void SetFrameScissor(const Recti& scissor)
{
    assert(g_Window);
    glScissor(scissor.ul.x, (g_WindowHeight - scissor.getY()) - scissor.getHeight(), scissor.getWidth(), scissor.getHeight());
}

//-----------------------------------------------------------------
ITexture* CreateTexture(Canvas* pixels)
{
    assert(g_Window);
    assert(pixels);

    double log2_width  = log10((double)pixels->getWidth())  / log10(2.0);
    double log2_height = log10((double)pixels->getHeight()) / log10(2.0);

    int tex_width  = pixels->getWidth();
    int tex_height = pixels->getHeight();

    if (log2_width != floor(log2_width)) {
        tex_width = 1 << (int)ceil(log2_width);
    }
    if (log2_height != floor(log2_height)) {
        tex_height = 1 << (int)ceil(log2_height);
    }

    // make sure texture is, at max, g_MaxTextureSize by g_MaxTextureSize
    if (tex_width > g_MaxTextureSize || tex_height > g_MaxTextureSize) {
        return 0;
    }

    RGBA* tex_pixels = pixels->getPixels();

    // make sure texture size is power of 2
    if (tex_width != pixels->getWidth() || tex_height != pixels->getHeight()) {
        // allocate a new pixel buffer
        try {
            tex_pixels = new RGBA[tex_width * tex_height];
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return 0;
        }
        // copy the old pixels into the new buffer
        for (int i = 0; i < pixels->getHeight(); i++) {
            memcpy(tex_pixels + i * tex_width, pixels->getPixels() + i * pixels->getWidth(), pixels->getPitch());
        }
    }

    GLuint gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_pixels);

    if (tex_pixels != pixels->getPixels()) {
        delete[] tex_pixels;
    }

    return Texture::Create(gl_texture, tex_width, tex_height, pixels->getWidth(), pixels->getHeight());
}

//-----------------------------------------------------------------
Canvas* CloneFrame(Recti* section)
{
    assert(g_Window);

    int x = 0;
    int y = 0;
    int w = g_WindowWidth;
    int h = g_WindowHeight;

    if (section) {
        if (!section->isValid() || !Recti(0, 0, g_WindowWidth - 1, g_WindowHeight - 1).contains(*section)) {
            return 0;
        }
        x = section->getX();
        y = g_WindowHeight - (section->getY() + section->getHeight());
        w = section->getWidth();
        h = section->getHeight();
    }

    CanvasPtr canvas = Canvas::Create(w, h);
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, canvas->getPixels());
    canvas->flipHorizontally();
    return canvas.release();
}

//-----------------------------------------------------------------
void DrawPoint(const Vec2i& pos, const RGBA& color)
{
    assert(g_Window);

    glBegin(GL_POINTS);

    glColor4ubv((GLubyte*)&color);
    glVertex2i(pos.x, pos.y);

    glEnd();
}

//-----------------------------------------------------------------
void DrawLine(Vec2i pos[2], RGBA col[2])
{
    assert(g_Window);

    glBegin(GL_LINES);

    glColor4ubv((GLubyte*)&col[0]);
    glVertex2i(pos[0].x, pos[0].y);

    glColor4ubv((GLubyte*)&col[1]);
    glVertex2i(pos[1].x, pos[1].y);

    glEnd();
}

//-----------------------------------------------------------------
void DrawTriangle(Vec2i pos[3], RGBA col[3])
{
    assert(g_Window);

    glBegin(GL_TRIANGLES);

    glColor4ubv((GLubyte*)&col[0]);
    glVertex2i(pos[0].x, pos[0].y);

    glColor4ubv((GLubyte*)&col[1]);
    glVertex2i(pos[1].x, pos[1].y);

    glColor4ubv((GLubyte*)&col[2]);
    glVertex2i(pos[2].x, pos[2].y);

    glEnd();
}

//-----------------------------------------------------------------
void DrawTexturedTriangle(ITexture* texture, Vec2i texcoord[3], Vec2i pos[3], const RGBA& mask)
{
    assert(g_Window);
    assert(texture);

    Texture* t = (Texture*)texture;
    GLfloat tw = (GLfloat)t->getTexWidth();
    GLfloat th = (GLfloat)t->getTexHeight();

    glBindTexture(GL_TEXTURE_2D, t->getTexID());
    glColor4ubv((GLubyte*)&mask);

    glBegin(GL_TRIANGLES);

    glTexCoord2f(texcoord[0].x / tw, texcoord[0].y / th);
    glVertex2i(pos[0].x, pos[0].y);

    glTexCoord2f(texcoord[1].x / tw, texcoord[1].y / th);
    glVertex2i(pos[1].x, pos[1].y);

    glTexCoord2f(texcoord[2].x / tw, texcoord[2].y / th);
    glVertex2i(pos[2].x, pos[2].y);

    glEnd();
}

//-----------------------------------------------------------------
void DrawRect(const Recti& rect, RGBA col[4])
{
    assert(g_Window);

    if (!rect.isValid()) {
        return;
    }

    glBegin(GL_QUADS);

    glColor4ubv((GLubyte*)&col[0]);
    glVertex2i(rect.ul.x, rect.ul.y);

    glColor4ubv((GLubyte*)&col[1]);
    glVertex2i(rect.lr.x + 1, rect.ul.y);

    glColor4ubv((GLubyte*)&col[2]);
    glVertex2i(rect.lr.x + 1, rect.lr.y + 1);

    glColor4ubv((GLubyte*)&col[3]);
    glVertex2i(rect.ul.x, rect.lr.y + 1);

    glEnd();
}

//-----------------------------------------------------------------
void DrawImage(ITexture* image, const Vec2i& pos, const RGBA& mask)
{
    assert(g_Window);
    assert(image);

    Texture* t = (Texture*)image;
    GLfloat  w = (GLfloat)t->getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)t->getHeight() / (GLfloat)t->getTexHeight();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t->getTexID());
    glColor4ubv((GLubyte*)&mask);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2i(pos.x, pos.y);

    glTexCoord2f(w, 0);
    glVertex2i(pos.x + t->getWidth(), pos.y);

    glTexCoord2f(w, h);
    glVertex2i(pos.x + t->getWidth(), pos.y + t->getHeight());

    glTexCoord2f(0, h);
    glVertex2i(pos.x, pos.y + t->getHeight());

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------
void DrawSubImage(ITexture* image, const Recti& src_rect, const Vec2i& pos, const RGBA& mask)
{
    assert(g_Window);
    assert(image);

    if (!src_rect.isValid()) {
        return;
    }

    Texture* t = (Texture*)image;
    GLfloat  x = (GLfloat)src_rect.getX()      / (GLfloat)t->getTexWidth();
    GLfloat  y = (GLfloat)src_rect.getY()      / (GLfloat)t->getTexHeight();
    GLfloat  w = (GLfloat)src_rect.getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)src_rect.getHeight() / (GLfloat)t->getTexHeight();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t->getTexID());
    glColor4ubv((GLubyte*)&mask);

    glBegin(GL_QUADS);

    glTexCoord2f(x, y);
    glVertex2i(pos.x, pos.y);

    glTexCoord2f(x + w, y);
    glVertex2i(pos.x + src_rect.getWidth() - 1, pos.y);

    glTexCoord2f(x + w, y + h);
    glVertex2i(pos.x + src_rect.getWidth() - 1, pos.y + src_rect.getHeight() - 1);

    glTexCoord2f(x, y + h);
    glVertex2i(pos.x, pos.y + src_rect.getHeight() - 1);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------
void DrawImageQuad(ITexture* texture, Vec2i pos[4], const RGBA& mask)
{
    assert(g_Window);
    assert(texture);

    Texture* t = (Texture*)texture;
    GLfloat  w = (GLfloat)t->getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)t->getHeight() / (GLfloat)t->getTexHeight();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t->getTexID());
    glColor4ubv((GLubyte*)&mask);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2i(pos[0].x, pos[0].y);

    glTexCoord2f(w, 0);
    glVertex2i(pos[1].x, pos[1].y);

    glTexCoord2f(w, h);
    glVertex2i(pos[2].x, pos[2].y);

    glTexCoord2f(0, h);
    glVertex2i(pos[3].x, pos[3].y);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------
void DrawSubImageQuad(ITexture* texture, const Recti& src_rect, Vec2i pos[4], const RGBA& mask)
{
    assert(g_Window);
    assert(texture);

    if (!src_rect.isValid()) {
        return;
    }

    Texture* t = (Texture*)texture;
    GLfloat  x = (GLfloat)src_rect.getX()      / (GLfloat)t->getTexWidth();
    GLfloat  y = (GLfloat)src_rect.getY()      / (GLfloat)t->getTexHeight();
    GLfloat  w = (GLfloat)src_rect.getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)src_rect.getHeight() / (GLfloat)t->getTexHeight();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t->getTexID());
    glColor4ubv((GLubyte*)&mask);

    glBegin(GL_QUADS);

    glTexCoord2f(x, y);
    glVertex2i(pos[0].x, pos[0].y);

    glTexCoord2f(x + w, y);
    glVertex2i(pos[1].x, pos[1].y);

    glTexCoord2f(x + w, y + h);
    glVertex2i(pos[2].x, pos[2].y);

    glTexCoord2f(x, y + h);
    glVertex2i(pos[3].x, pos[3].y);

    glEnd();
    glDisable(GL_TEXTURE_2D);
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
