#include <cassert>
#include <cmath>
#include <sstream>
#include <GL.h>
#include <SDL.h>
#include "../../version.hpp"
#include "../video.hpp"
#include "OpenGLTexture.hpp"


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

namespace video {

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
                    for (int j = 0; j < out.size(); j++) {
                        if (out[j].width == mode.width && out[j].height == mode.height) {
                            vmode_not_yet_listed = false;
                        }
                    }
                    if (vmode_not_yet_listed) {
                        out.push_back(Dim2i(mode.width, mode.height));
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------
    bool OpenWindow(int width, int height, bool fullscreen)
    {
        if (!g_window && width * heigh > 0) {
            // build default window title
            std::ostringstream oss;
            oss << "Sphere " << SPHERE_VERSION_STRING;

            // setup flags
            Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
            if (fullscreen) {
                flags |= SDL_WINDOW_FULLSCREEN;
            }

            // open window
            g_window = SDL_CreateWindow(oss.str.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
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

            return true;
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool IsWindowOpen()
    {
        return g_window != 0;
    }

    //-----------------------------------------------------------------
    int GetWindowWidth()
    {
        if (g_window) {
            return g_window_width;
        }
        return 0;
    }

    //-----------------------------------------------------------------
    int GetWindowHeight()
    {
        if (g_window) {
            return g_window_height;
        }
        return 0;
    }

    //-----------------------------------------------------------------
    bool IsWindowFullscreen()
    {
        if (g_window) {
            return (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN) != 0;
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool SetWindowFullscreen(bool fullscreen)
    {
        if (g_window) {
            return SDL_SetWindowFullscreen(g_window, (fullscreen ? SDL_TRUE : SDL_FALSE)) == 0;
        }
        return false;
    }

    //-----------------------------------------------------------------
    const char* GetWindowTitle()
    {
        if (g_window) {
            return SDL_GetWindowTitle(g_window);
        }
        return 0;
    }

    //-----------------------------------------------------------------
    void SetWindowTitle(const char* title)
    {
        if (g_window) {
            SDL_SetWindowTitle(g_window, title);
        }
    }

    //-----------------------------------------------------------------
    void SetWindowIcon(Canvas* canvas)
    {
        assert(canvas);
        if (g_window) {
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
                                                            canvas->getNumBytesPerPixel() * 8,
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
    }

    //-----------------------------------------------------------------
    void SwapBuffers()
    {
        if (g_window) {
            SDL_GL_SwapWindow(g_window);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    //-----------------------------------------------------------------
    ITexture* CreateTexture(Canvas* canvas)
    {
        if (!g_window || !canvas) {
            return 0;
        }

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
            pixels = new RGBA[tex_width * tex_height];
            if (!pixels) {
                return 0;
            }
            // copy the old pixels into the new buffer
            for (int i = 0; i < canvas->getHeight(); i++) {
                memcpy(pixels + i * tex_width, canvas->getPixels() + i * canvas->getWidth(), canvas->getPitch());
            }
        }

        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        if (pixels != canvas->getPixels()) {
            delete[] pixels;
        }

        OpenGLTexture* texture = OpenGLTexture::Create(texture_id, tex_width, tex_height, canvas->getWidth(), canvas->getHeight());

        if (!texture) {
            glDeleteTextures(1, &texture_id);
            return 0;
        }

        return texture;
    }

    //-----------------------------------------------------------------
    ITexture* CloneSection(const Recti& section)
    {
        if (!g_window || !Recti(0, 0, _windowWidth - 1, _windowHeight - 1).contains(section)) {
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

        OpenGLTexture* texture = OpenGLTexture::Create(texture_id, tex_width, tex_height, width, height);

        if (!texture) {
            glDeleteTextures(1, &texture_id);
            return 0;
        }

        return texture;
    }

    //-----------------------------------------------------------------
    bool GetClipRect(Recti& out)
    {
        if (g_window) {
            GLint cliprect[4];
            glGetIntegerv(GL_SCISSOR_BOX, cliprect);
            out.ul.x = cliprect[0];
            out.ul.y = cliprect[1] - g_window_height + cliprect[3];
            out.lr.x = (cliprect[0] + cliprect[2]) - 1;
            out.lr.y = (out.ul.y + cliprect[3]) - 1;
            return true;
        }
        return false;
    }

    //-----------------------------------------------------------------
    void SetClipRect(const Recti& clip)
    {
        if (g_window) {
            glScissor(clip.ul.x, (g_window_height - clip.ul.y) - clip.getHeight(), clip.getWidth(), clip.getHeight());
        }
    }

    //-----------------------------------------------------------------
    void DrawPoint(const Vec2i& pos, const RGBA& color)
    {
        if (!g_window) {
            return;
        }

        glBegin(GL_POINTS);

        glColor4ubv((GLubyte*)&color);
        glVertex2i(pos.x, pos.y);

        glEnd();
    }

    //-----------------------------------------------------------------
    void DrawLine(Vec2i pos[2], RGBA col[2])
    {
        if (!g_window) {
            return;
        }

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
        if (!g_window) {
            return;
        }

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
    void DrawTexturedTriangle(ITexture* texture, Vec2i pos[3], Vec2i texcoord[3], RGBA* mask_col)
    {
        if (!g_window || !texture) {
            return;
        }

        RGBA mask(255, 255, 255, 255);
        if (mask_col) {
            mask = *mask_col;
        }

        glBindTexture(GL_TEXTURE_2D, ((OpenGLTexture*)texture)->getTexID());
        glColor4ubv((GLubyte*)&mask);

        glBegin(GL_TRIANGLES);

        glTexCoord2i(texcoord[0].x, texcoord[0].y);
        glVertex2i(pos[0].x, pos[0].y);

        glTexCoord2i(texcoord[1].x, texcoord[1].y);
        glVertex2i(pos[1].x, pos[1].y);

        glTexCoord2i(texcoord[2].x, texcoord[2].y);
        glVertex2i(pos[2].x, pos[2].y);

        glEnd();
    }

    //-----------------------------------------------------------------
    void DrawRect(const Recti& rect, RGBA col[4])
    {
        if (!g_window || !rect.isValid()) {
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
    void DrawImage(ITexture* image, const Vec2i& pos, RGBA* mask_col)
    {
        if (!g_window || !image) {
            return;
        }

        RGBA mask(255, 255, 255, 255);
        if (mask_col) {
            mask = *mask_col;
        }

        glBindTexture(GL_TEXTURE_2D, ((OpenGLTexture*)image)->getTexID());
        glColor4ubv((GLubyte*)&mask);

        glBegin(GL_QUADS);

        glTexCoord2i(0, 0);
        glVertex2i(pos.x, pos.y);

        glTexCoord2i(image->getWidth() - 1, 0);
        glVertex2i(pos.x + image->getWidth() - 1, pos.y);

        glTexCoord2i(image->getWidth() - 1, image->getHeight() - 1);
        glVertex2i(pos.x + image->getWidth() - 1, pos.y + image->getHeight() - 1);

        glTexCoord2i(0, image->getHeight() - 1);
        glVertex2i(pos.x, pos.y + image->getHeight() - 1);

        glEnd();
    }

    //-----------------------------------------------------------------
    void DrawSubImage(ITexture* image, const Recti& src_rect, const Vec2i& pos, RGBA* mask_col)
    {
        if (!g_window || !image || !src_rect.isValid()) {
            return;
        }

        RGBA mask(255, 255, 255, 255);
        if (mask_col) {
            mask = *mask_col;
        }

        glBindTexture(GL_TEXTURE_2D, ((OpenGLTexture*)image)->getTexID());
        glColor4ubv((GLubyte*)&mask);

        glBegin(GL_QUADS);

        glTexCoord2i(src_rect.ul.x, src_rect.ul.y);
        glVertex2i(pos.x, pos.y);

        glTexCoord2i(src_rect.lr.x, src_rect.ul.y);
        glVertex2i(pos.x + src_rect.getWidth() - 1, pos.y);

        glTexCoord2i(src_rect.lr.x, src_rect.lr.y);
        glVertex2i(pos.x + src_rect.getWidth() - 1, pos.y + src_rect.getHeight() - 1);

        glTexCoord2i(src_rect.ul.x, src_rect.lr.y);
        glVertex2i(pos.x, pos.y + src_rect.getHeight() - 1);

        glEnd();
    }

    //-----------------------------------------------------------------
    void DrawImageQuad(ITexture* texture, Vec2i pos[4], RGBA* mask_col)
    {
        if (!g_window || !texture) {
            return;
        }

        RGBA mask(255, 255, 255, 255);
        if (mask_col) {
            mask = *mask_col;
        }

        glBindTexture(GL_TEXTURE_2D, ((OpenGLTexture*)texture)->getTexID());
        glColor4ubv((GLubyte*)&mask);

        glBegin(GL_QUADS);

        glTexCoord2i(0, 0);
        glVertex2i(pos[0].x, pos[0].y);

        glTexCoord2i(texture->getWidth() - 1, 0);
        glVertex2i(pos[1].x, pos[1].y);

        glTexCoord2i(texture->getWidth() - 1, texture->getHeight() - 1);
        glVertex2i(pos[2].x, pos[2].y);

        glTexCoord2i(0, texture->getHeight() - 1);
        glVertex2i(pos[3].x, pos[3].y);

        glEnd();
    }

    //-----------------------------------------------------------------
    void DrawSubImageQuad(ITexture* texture, const Recti& src_rect, Vec2i pos[4], RGBA* mask_col)
    {
        if (!g_window || !texture || !src_rect.isValid()) {
            return;
        }

        RGBA mask(255, 255, 255, 255);
        if (mask_col) {
            mask = *mask_col;
        }

        glBindTexture(GL_TEXTURE_2D, ((OpenGLTexture*)texture)->getTexID());
        glColor4ubv((GLubyte*)&mask);

        glBegin(GL_QUADS);

        glTexCoord2i(src_rect.ul.x, src_rect.ul.y);
        glVertex2i(pos[0].x, pos[0].y);

        glTexCoord2i(src_rect.lr.x, src_rect.ul.y);
        glVertex2i(pos[1].x, pos[1].y);

        glTexCoord2i(src_rect.lr.x, src_rect.lr.y);
        glVertex2i(pos[2].x, pos[2].y);

        glTexCoord2i(src_rect.ul.x, src_rect.lr.y);
        glVertex2i(pos[3].x, pos[3].y);

        glEnd();
    }

    namespace internal {

        //-----------------------------------------------------------------
        bool InitVideo()
        {
            // print some OpenGL information
            log.info() << "Using OpenGL " << glGetString(GL_VERSION);
            log.info() << "OpenGL Vendor:   " << glGetString(GL_VENDOR);
            log.info() << "OpenGL Renderer: " << glGetString(GL_RENDERER);

            // print SDL version
            SDL_version sdl_linked_version;
            SDL_GetVersion(&sdl_linked_version);
            log.info() << "Using SDL " << (int)sdl_linked_version.major \
                       << "."          << (int)sdl_linked_version.minor \
                       << "."          << (int)sdl_linked_version.patch;

            // warn if we are using a different SDL version than we compiled against
            SDL_version sdl_compiled_version;
            SDL_VERSION(sdl_compiled_version);
            if (sdl_linked_version.major != sdl_compiled_version.major ||
                sdl_linked_version.minor != sdl_compiled_version.minor)
            {
                log.warning() << "Compiled against SDL " << (int)sdl_compiled_version.major \
                              << "."                     << (int)sdl_compiled_version.minor \
                              << "."                     << (int)sdl_compiled_version.patch;
            }

            // initialize SDL
            log.info() << "Initializing SDL";
            if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                log.error() << "Failed initializing SDL: " << SDL_GetError();
                return false;
            }

            // ensure SDL will be properly deinitialized
            atexit(SDL_Quit);

            // get max texture size
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_max_texture_size);

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

    } // namespace internal

} // namespace video
