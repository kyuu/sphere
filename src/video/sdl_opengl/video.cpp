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
SDL_Window* g_window = 0;
int g_window_width  = 0;
int g_window_height = 0;
GLint g_max_texture_size = 0;
SDL_GLContext g_context;

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
    assert(!g_window);

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
    g_window = SDL_CreateWindow(oss.str().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!g_window) {
        return false;
    }
    g_window_width  = width;
    g_window_height = height;

    // create OpenGL context
    g_context = SDL_GL_CreateContext(g_window);

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

    // enable 2D texturization
    glEnable(GL_TEXTURE_2D);

    // get max texture size
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_max_texture_size);

    return true;
}

//-----------------------------------------------------------------
bool IsWindowOpen()
{
    return g_window != 0;
}

//-----------------------------------------------------------------
int GetWindowWidth()
{
    assert(g_window);
    return g_window_width;
}

//-----------------------------------------------------------------
int GetWindowHeight()
{
    assert(g_window);
    return g_window_height;
}

//-----------------------------------------------------------------
bool IsWindowFullscreen()
{
    assert(g_window);
    return (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN) != 0;
}

//-----------------------------------------------------------------
bool SetWindowFullscreen(bool fullscreen)
{
    assert(g_window);
    return SDL_SetWindowFullscreen(g_window, (fullscreen ? SDL_TRUE : SDL_FALSE)) == 0;
}

//-----------------------------------------------------------------
const char* GetWindowTitle()
{
    assert(g_window);
    return SDL_GetWindowTitle(g_window);
}

//-----------------------------------------------------------------
void SetWindowTitle(const char* title)
{
    assert(g_window);
    if (title) {
        SDL_SetWindowTitle(g_window, title);
    }
}

//-----------------------------------------------------------------
void SetWindowIcon(Canvas* canvas)
{
    assert(g_window);
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
        SDL_SetWindowIcon(g_window, surface);
        SDL_FreeSurface(surface);
    }
}

//-----------------------------------------------------------------
void SwapFrameBuffers()
{
    assert(g_window);
    SDL_GL_SwapWindow(g_window);
    glClear(GL_COLOR_BUFFER_BIT);
}

//-----------------------------------------------------------------
ITexture* CreateTexture(Canvas* canvas)
{
    assert(g_window);
    assert(canvas);

    double log2_width  = log10((double)canvas->getWidth())  / log10(2.0);
    double log2_height = log10((double)canvas->getHeight()) / log10(2.0);

    int tex_width  = canvas->getWidth();
    int tex_height = canvas->getHeight();

    if (log2_width != floor(log2_width)) {
        tex_width = 1 << (int)ceil(log2_width);
    }
    if (log2_height != floor(log2_height)) {
        tex_height = 1 << (int)ceil(log2_height);
    }

    // make sure texture is, at max, g_max_texture_size by g_max_texture_size
    if (tex_width > g_max_texture_size || tex_height > g_max_texture_size) {
        return 0;
    }

    RGBA* pixels = canvas->getPixels();

    // make sure texture size is power of 2
    if (tex_width != canvas->getWidth() || tex_height != canvas->getHeight()) {
        // allocate a new pixel buffer
        try {
            pixels = new RGBA[tex_width * tex_height];
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return 0;
        }
        // copy the old pixels into the new buffer
        for (int i = 0; i < canvas->getHeight(); i++) {
            memcpy(pixels + i * tex_width, canvas->getPixels() + i * canvas->getWidth(), canvas->getPitch());
        }
    }

    GLuint gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    if (pixels != canvas->getPixels()) {
        delete[] pixels;
    }

    return Texture::Create(gl_texture, tex_width, tex_height, canvas->getWidth(), canvas->getHeight());
}

//-----------------------------------------------------------------
ITexture* CloneFrameBufferSection(const Recti& section)
{
    assert(g_window);

    if (!Recti(0, 0, g_window_width - 1, g_window_height - 1).contains(section)) {
        return 0;
    }

    int x = section.getX();
    int y = section.getY();
    int width  = section.getWidth();
    int height = section.getHeight();

    double log2_width  = log10((double)width)  / log10(2.0);
    double log2_height = log10((double)height) / log10(2.0);

    int tex_width  = width;
    int tex_height = height;

    if (log2_width != floor(log2_width)) {
        tex_width = 1 << (int)ceil(log2_width);
    }
    if (log2_height != floor(log2_height)) {
        tex_height = 1 << (int)ceil(log2_height);
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, tex_width, tex_height, 0);

    Texture* texture = Texture::Create(texture_id, tex_width, tex_height, width, height);

    if (!texture) {
        glDeleteTextures(1, &texture_id);
        return 0;
    }

    return texture;
}

//-----------------------------------------------------------------
bool GetFrameBufferClipRect(Recti& out)
{
    assert(g_window);
    GLint cliprect[4];
    glGetIntegerv(GL_SCISSOR_BOX, cliprect);
    out.ul.x = cliprect[0];
    out.ul.y = cliprect[1] - g_window_height + cliprect[3];
    out.lr.x = (cliprect[0] + cliprect[2]) - 1;
    out.lr.y = (out.ul.y + cliprect[3]) - 1;
    return true;
}

//-----------------------------------------------------------------
void SetFrameBufferClipRect(const Recti& clip)
{
    assert(g_window);
    glScissor(clip.ul.x, (g_window_height - clip.ul.y) - clip.getHeight(), clip.getWidth(), clip.getHeight());
}

//-----------------------------------------------------------------
void DrawPoint(const Vec2i& pos, const RGBA& color)
{
    assert(g_window);

    glBegin(GL_POINTS);

    glColor4ubv((GLubyte*)&color);
    glVertex2i(pos.x, pos.y);

    glEnd();
}

//-----------------------------------------------------------------
void DrawLine(Vec2i pos[2], RGBA col[2])
{
    assert(g_window);

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
    assert(g_window);

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
    assert(g_window);
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
    assert(g_window);

    if (!rect.isValid()) {
        return;
    }

    glBegin(GL_QUADS);

    glColor4ubv((GLubyte*)&col[0]);
    glVertex2i(rect.ul.x, rect.ul.y);

    glColor4ubv((GLubyte*)&col[1]);
    glVertex2i(rect.lr.x, rect.ul.y);

    glColor4ubv((GLubyte*)&col[2]);
    glVertex2i(rect.lr.x, rect.lr.y);

    glColor4ubv((GLubyte*)&col[3]);
    glVertex2i(rect.ul.x, rect.lr.y);

    glEnd();
}

//-----------------------------------------------------------------
void DrawImage(ITexture* image, const Vec2i& pos, const RGBA& mask)
{
    assert(g_window);
    assert(image);

    Texture* t = (Texture*)image;
    GLfloat  w = (GLfloat)t->getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)t->getHeight() / (GLfloat)t->getTexHeight();

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
}

//-----------------------------------------------------------------
void DrawSubImage(ITexture* image, const Recti& src_rect, const Vec2i& pos, const RGBA& mask)
{
    assert(g_window);
    assert(image);

    if (!src_rect.isValid()) {
        return;
    }

    Texture* t = (Texture*)image;
    GLfloat  x = (GLfloat)src_rect.getX()      / (GLfloat)t->getTexWidth();
    GLfloat  y = (GLfloat)src_rect.getY()      / (GLfloat)t->getTexHeight();
    GLfloat  w = (GLfloat)src_rect.getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)src_rect.getHeight() / (GLfloat)t->getTexHeight();

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
}

//-----------------------------------------------------------------
void DrawImageQuad(ITexture* texture, Vec2i pos[4], const RGBA& mask)
{
    assert(g_window);
    assert(texture);

    Texture* t = (Texture*)texture;
    GLfloat  w = (GLfloat)t->getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)t->getHeight() / (GLfloat)t->getTexHeight();

    glBindTexture(GL_TEXTURE_2D, t->getTexID());
    glColor4ubv((GLubyte*)&mask);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2i(pos[0].x, pos[0].y);

    glTexCoord2f(w, h);
    glVertex2i(pos[1].x, pos[1].y);

    glTexCoord2f(w, h);
    glVertex2i(pos[2].x, pos[2].y);

    glTexCoord2f(0, h);
    glVertex2i(pos[3].x, pos[3].y);

    glEnd();
}

//-----------------------------------------------------------------
void DrawSubImageQuad(ITexture* texture, const Recti& src_rect, Vec2i pos[4], const RGBA& mask)
{
    assert(g_window);
    assert(texture);

    if (!src_rect.isValid()) {
        return;
    }

    Texture* t = (Texture*)texture;
    GLfloat  x = (GLfloat)src_rect.getX()      / (GLfloat)t->getTexWidth();
    GLfloat  y = (GLfloat)src_rect.getY()      / (GLfloat)t->getTexHeight();
    GLfloat  w = (GLfloat)src_rect.getWidth()  / (GLfloat)t->getTexWidth();
    GLfloat  h = (GLfloat)src_rect.getHeight() / (GLfloat)t->getTexHeight();

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
    if (g_window) {
        SDL_GL_DeleteContext(g_context); // if a window is open, assume a OpenGL context was created as well
        SDL_DestroyWindow(g_window);
        g_window        = 0;
        g_window_width  = 0;
        g_window_height = 0;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
