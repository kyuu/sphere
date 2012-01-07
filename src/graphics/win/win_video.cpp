#include <cassert>
#include <cmath>
#include <deque>
#include <sstream>
#include <windows.h>

// TODO: replace this work-around with something better
#ifdef LoadImage // defined in <windows.h>
#  undef LoadImage
#endif

#include <GL/gl.h>
#include "../../version.hpp"
#include "../../base/Blob.hpp"
#include "../../input/input.hpp"
#include "../../io/filesystem.hpp"
#include "../../io/numio.hpp"
#include "../../io/imageio.hpp"
#include "../video.hpp"

#define DEFAULT_WINDOW_WIDTH  640
#define DEFAULT_WINDOW_HEIGHT 480

#define WINDOW_STYLE            (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE)
#define FULLSCREEN_WINDOW_STYLE (WS_POPUP | WS_VISIBLE)

#define EVENT_QUEUE_MAX_SIZE 1024


namespace sphere {
    namespace video {

        //-----------------------------------------------------------------
        struct Texture : public RefImpl<ITexture> {
            GLuint textureName;
            Dim2i  textureSize;
            Dim2i  size;

            // ITexture implementation
            const Dim2i& getTextureSize() const {
                return textureSize;
            }
            const Dim2i& getSize() const {
                return size;
            }
        };

        //-----------------------------------------------------------------
        // globals
        std::deque<WindowEvent> g_EventQueue;

        static int WinKeyToSphereKey[256] = {
            /* 0x00 */ -1,
            /* 0x01 */ -1,
            /* 0x02 */ -1,
            /* 0x03 */ -1,
            /* 0x04 */ -1,
            /* 0x05 */ -1,
            /* 0x06 */ -1,
            /* 0x07 */ -1,
            /* 0x08 */ input::KEY_BACKSPACE,
            /* 0x09 */ input::KEY_TAB,
            /* 0x0A */ -1,
            /* 0x0B */ -1,
            /* 0x0C */ -1,
            /* 0x0D */ input::KEY_ENTER,
            /* 0x0E */ -1,
            /* 0x0F */ -1,
            /* 0x10 */ -1,
            /* 0x11 */ -1,
            /* 0x12 */ -1,
            /* 0x13 */ -1,
            /* 0x14 */ input::KEY_CAPSLOCK,
            /* 0x15 */ -1,
            /* 0x16 */ -1,
            /* 0x17 */ -1,
            /* 0x18 */ -1,
            /* 0x19 */ -1,
            /* 0x1A */ -1,
            /* 0x1B */ input::KEY_ESCAPE,
            /* 0x1C */ -1,
            /* 0x1D */ -1,
            /* 0x1E */ -1,
            /* 0x1F */ -1,
            /* 0x20 */ input::KEY_SPACE,
            /* 0x21 */ input::KEY_PAGEUP,
            /* 0x22 */ input::KEY_PAGEDOWN,
            /* 0x23 */ input::KEY_END,
            /* 0x24 */ input::KEY_HOME,
            /* 0x25 */ input::KEY_LEFT,
            /* 0x26 */ input::KEY_UP,
            /* 0x27 */ input::KEY_RIGHT,
            /* 0x28 */ input::KEY_DOWN,
            /* 0x29 */ -1,
            /* 0x2A */ -1,
            /* 0x2B */ -1,
            /* 0x2C */ -1,
            /* 0x2D */ input::KEY_INSERT,
            /* 0x2E */ input::KEY_DELETE,
            /* 0x2F */ -1,
            /* 0x30 */ input::KEY_0,
            /* 0x31 */ input::KEY_1,
            /* 0x32 */ input::KEY_2,
            /* 0x33 */ input::KEY_3,
            /* 0x34 */ input::KEY_4,
            /* 0x35 */ input::KEY_5,
            /* 0x36 */ input::KEY_6,
            /* 0x37 */ input::KEY_7,
            /* 0x38 */ input::KEY_8,
            /* 0x39 */ input::KEY_9,
            /* 0x3A */ -1,
            /* 0x3B */ -1,
            /* 0x3C */ -1,
            /* 0x3D */ -1,
            /* 0x3E */ -1,
            /* 0x3F */ -1,
            /* 0x40 */ -1,
            /* 0x41 */ input::KEY_A,
            /* 0x42 */ input::KEY_B,
            /* 0x43 */ input::KEY_C,
            /* 0x44 */ input::KEY_D,
            /* 0x45 */ input::KEY_E,
            /* 0x46 */ input::KEY_F,
            /* 0x47 */ input::KEY_G,
            /* 0x48 */ input::KEY_H,
            /* 0x49 */ input::KEY_I,
            /* 0x4A */ input::KEY_J,
            /* 0x4B */ input::KEY_K,
            /* 0x4C */ input::KEY_L,
            /* 0x4D */ input::KEY_M,
            /* 0x4E */ input::KEY_N,
            /* 0x4F */ input::KEY_O,
            /* 0x50 */ input::KEY_P,
            /* 0x51 */ input::KEY_Q,
            /* 0x52 */ input::KEY_R,
            /* 0x53 */ input::KEY_S,
            /* 0x54 */ input::KEY_T,
            /* 0x55 */ input::KEY_U,
            /* 0x56 */ input::KEY_V,
            /* 0x57 */ input::KEY_W,
            /* 0x58 */ input::KEY_X,
            /* 0x59 */ input::KEY_Y,
            /* 0x5A */ input::KEY_Z,
            /* 0x5B */ -1,
            /* 0x5C */ -1,
            /* 0x5D */ -1,
            /* 0x5E */ -1,
            /* 0x5F */ -1,
            /* 0x60 */ input::KEY_NUMPAD_0,
            /* 0x61 */ input::KEY_NUMPAD_1,
            /* 0x62 */ input::KEY_NUMPAD_2,
            /* 0x63 */ input::KEY_NUMPAD_3,
            /* 0x64 */ input::KEY_NUMPAD_4,
            /* 0x65 */ input::KEY_NUMPAD_5,
            /* 0x66 */ input::KEY_NUMPAD_6,
            /* 0x67 */ input::KEY_NUMPAD_7,
            /* 0x68 */ input::KEY_NUMPAD_8,
            /* 0x69 */ input::KEY_NUMPAD_9,
            /* 0x6A */ input::KEY_MULTIPLY,
            /* 0x6B */ input::KEY_ADD,
            /* 0x6C */ -1,
            /* 0x6D */ input::KEY_SUBTRACT,
            /* 0x6E */ -1,
            /* 0x6F */ input::KEY_DIVIDE,
            /* 0x70 */ input::KEY_F1,
            /* 0x71 */ input::KEY_F2,
            /* 0x72 */ input::KEY_F3,
            /* 0x73 */ input::KEY_F4,
            /* 0x74 */ input::KEY_F5,
            /* 0x75 */ input::KEY_F6,
            /* 0x76 */ input::KEY_F7,
            /* 0x77 */ input::KEY_F8,
            /* 0x78 */ input::KEY_F9,
            /* 0x79 */ input::KEY_F10,
            /* 0x7A */ input::KEY_F11,
            /* 0x7B */ input::KEY_F12,
            /* 0x7C */ -1,
            /* 0x7D */ -1,
            /* 0x7E */ -1,
            /* 0x7F */ -1,
            /* 0x80 */ -1,
            /* 0x81 */ -1,
            /* 0x82 */ -1,
            /* 0x83 */ -1,
            /* 0x84 */ -1,
            /* 0x85 */ -1,
            /* 0x86 */ -1,
            /* 0x87 */ -1,
            /* 0x88 */ -1,
            /* 0x89 */ -1,
            /* 0x8A */ -1,
            /* 0x8B */ -1,
            /* 0x8C */ -1,
            /* 0x8D */ -1,
            /* 0x8E */ -1,
            /* 0x8F */ -1,
            /* 0x90 */ input::KEY_NUMLOCK,
            /* 0x91 */ -1,
            /* 0x92 */ -1,
            /* 0x93 */ -1,
            /* 0x94 */ -1,
            /* 0x95 */ -1,
            /* 0x96 */ -1,
            /* 0x97 */ -1,
            /* 0x98 */ -1,
            /* 0x99 */ -1,
            /* 0x9A */ -1,
            /* 0x9B */ -1,
            /* 0x9C */ -1,
            /* 0x9D */ -1,
            /* 0x9E */ -1,
            /* 0x9F */ -1,
            /* 0xA0 */ input::KEY_LSHIFT,
            /* 0xA1 */ input::KEY_RSHIFT,
            /* 0xA2 */ input::KEY_LCTRL,
            /* 0xA3 */ input::KEY_RCTRL,
            /* 0xA4 */ input::KEY_LALT,
            /* 0xA5 */ input::KEY_RALT,
            /* 0xA6 */ -1,
            /* 0xA7 */ -1,
            /* 0xA8 */ -1,
            /* 0xA9 */ -1,
            /* 0xAA */ -1,
            /* 0xAB */ -1,
            /* 0xAC */ -1,
            /* 0xAD */ -1,
            /* 0xAE */ -1,
            /* 0xAF */ -1,
            /* 0xB0 */ -1,
            /* 0xB1 */ -1,
            /* 0xB2 */ -1,
            /* 0xB3 */ -1,
            /* 0xB4 */ -1,
            /* 0xB5 */ -1,
            /* 0xB6 */ -1,
            /* 0xB7 */ -1,
            /* 0xB8 */ -1,
            /* 0xB9 */ -1,
            /* 0xBA */ -1,
            /* 0xBB */ input::KEY_PLUS,
            /* 0xBC */ input::KEY_COMMA,
            /* 0xBD */ input::KEY_MINUS,
            /* 0xBE */ input::KEY_PERIOD,
            /* 0xBF */ -1,
            /* 0xC0 */ -1,
            /* 0xC1 */ -1,
            /* 0xC2 */ -1,
            /* 0xC3 */ -1,
            /* 0xC4 */ -1,
            /* 0xC5 */ -1,
            /* 0xC6 */ -1,
            /* 0xC7 */ -1,
            /* 0xC8 */ -1,
            /* 0xC9 */ -1,
            /* 0xCA */ -1,
            /* 0xCB */ -1,
            /* 0xCC */ -1,
            /* 0xCD */ -1,
            /* 0xCE */ -1,
            /* 0xCF */ -1,
            /* 0xD0 */ -1,
            /* 0xD1 */ -1,
            /* 0xD2 */ -1,
            /* 0xD3 */ -1,
            /* 0xD4 */ -1,
            /* 0xD5 */ -1,
            /* 0xD6 */ -1,
            /* 0xD7 */ -1,
            /* 0xD8 */ -1,
            /* 0xD9 */ -1,
            /* 0xDA */ -1,
            /* 0xDB */ -1,
            /* 0xDC */ -1,
            /* 0xDD */ -1,
            /* 0xDE */ -1,
            /* 0xDF */ -1,
            /* 0xE0 */ -1,
            /* 0xE1 */ -1,
            /* 0xE2 */ -1,
            /* 0xE3 */ -1,
            /* 0xE4 */ -1,
            /* 0xE5 */ -1,
            /* 0xE6 */ -1,
            /* 0xE7 */ -1,
            /* 0xE8 */ -1,
            /* 0xE9 */ -1,
            /* 0xEA */ -1,
            /* 0xEB */ -1,
            /* 0xEC */ -1,
            /* 0xED */ -1,
            /* 0xEE */ -1,
            /* 0xEF */ -1,
            /* 0xF0 */ -1,
            /* 0xF1 */ -1,
            /* 0xF2 */ -1,
            /* 0xF3 */ -1,
            /* 0xF4 */ -1,
            /* 0xF5 */ -1,
            /* 0xF6 */ -1,
            /* 0xF7 */ -1,
            /* 0xF8 */ -1,
            /* 0xF9 */ -1,
            /* 0xFA */ -1,
            /* 0xFB */ -1,
            /* 0xFC */ -1,
            /* 0xFD */ -1,
            /* 0xFE */ -1,
            /* 0xFF */ -1,
        };

        //-----------------------------------------------------------------
        // globals
        Dim2i g_DefaultDisplayMode;
        std::vector<Dim2i> g_DisplayModes;
        WNDCLASS    g_WindowClass;
        HWND        g_Window = 0;
        Dim2i g_WindowSize;
        bool        g_WindowIsFullScreen = false;
        std::string g_WindowTitle;
        HDC         g_DeviceContext = 0;
        HGLRC       g_GLContext = 0;
        int         g_MaxTextureSize = 0;
        bool        g_NonPowerOfTwoTexturesSupported = false;

        //-----------------------------------------------------------------
        const Dim2i& GetDefaultDisplayMode()
        {
            return g_DefaultDisplayMode;
        }

        //-----------------------------------------------------------------
        const std::vector<Dim2i>& GetDisplayModes()
        {
            return g_DisplayModes;
        }

        //-----------------------------------------------------------------
        bool SetWindowMode(int width, int height, bool fullScreen)
        {
            assert(g_Window);
            assert(width > 0);
            assert(height > 0);

            if (g_WindowSize.width != width || g_WindowSize.height != height) {
                if (g_WindowIsFullScreen) {
                    // restore display mode to defaults
                    ChangeDisplaySettings(NULL, 0);

                    // set window style
                    SetWindowLong(g_Window, GWL_STYLE, WINDOW_STYLE);

                    g_WindowIsFullScreen = false;
                }

                // get adjusted metrics
                RECT rect = {0, 0, width, height};
                AdjustWindowRectEx(&rect, GetWindowLong(g_Window, GWL_STYLE), FALSE, 0);
                int w = rect.right  - rect.left;
                int h = rect.bottom - rect.top;
                int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
                int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

                // change window size and position
                if (!MoveWindow(g_Window, x, y, w, h, TRUE)) {
                    return false;
                }
                g_WindowSize = Dim2i(width, height);

                // change viewport
                glViewport(0, 0, width, height);

                // change projection matrix
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0, width, height, 0, -1, 1);

                // set up modelview matrix
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glTranslatef(0.375, 0.375, 0.0);

                // change clipping rectangle
                glScissor(0, 0, width, height);
            }

            if (fullScreen && !g_WindowIsFullScreen) {
                // see if the display mode is supported
                bool mode_supported = false;
                for (size_t i = 0; i < g_DisplayModes.size(); i++) {
                    if (g_DisplayModes[i].width  == g_WindowSize.width &&
                        g_DisplayModes[i].height == g_WindowSize.height)
                    {
                        mode_supported = true;
                        break;
                    }
                }

                if (!mode_supported) {
                    return false;
                }

                DEVMODE dm;
                memset(&dm, 0, sizeof(dm));
                dm.dmSize       = sizeof(dm);
                dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
                dm.dmPelsWidth  = g_WindowSize.width;
                dm.dmPelsHeight = g_WindowSize.height;

                // change display mode
                if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
                    // restore display mode to defaults
                    ChangeDisplaySettings(NULL, 0);

                    return false;
                }

                // set window style to a style appropriate for full-screen windows
                SetWindowLong(g_Window, GWL_STYLE, FULLSCREEN_WINDOW_STYLE);

                // set window as the topmost window and align it with the screen boundaries
                SetWindowPos(g_Window, HWND_TOPMOST, 0, 0, g_WindowSize.width, g_WindowSize.height, SWP_SHOWWINDOW);

                g_WindowIsFullScreen = true;

            } else if (!fullScreen && g_WindowIsFullScreen) {
                // restore display mode to default display mode
                ChangeDisplaySettings(NULL, 0);

                // set window style
                SetWindowLong(g_Window, GWL_STYLE, WINDOW_STYLE);

                // get adjusted metrics
                RECT rect = {0, 0, width, height};
                AdjustWindowRectEx(&rect, GetWindowLong(g_Window, GWL_STYLE), FALSE, 0);
                int w = rect.right  - rect.left;
                int h = rect.bottom - rect.top;
                int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
                int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

                // change window size and position
                MoveWindow(g_Window, x, y, w, h, TRUE);

                g_WindowIsFullScreen = false;
            }

            static bool first_call = true;
            if (first_call) {
                ShowWindow(g_Window, SW_SHOW);
                first_call = false;
            }

            return true;
        }

        //-----------------------------------------------------------------
        const Dim2i& GetWindowSize()
        {
            assert(g_Window);
            return g_WindowSize;
        }

        //-----------------------------------------------------------------
        bool IsWindowFullScreen()
        {
            assert(g_Window);
            return g_WindowIsFullScreen;
        }

        //-----------------------------------------------------------------
        bool IsWindowActive()
        {
            assert(g_Window);
            return g_Window == GetActiveWindow();
        }

        //-----------------------------------------------------------------
        const std::string& GetWindowTitle()
        {
            assert(g_Window);
            return g_WindowTitle;
        }

        //-----------------------------------------------------------------
        void SetWindowTitle(const std::string& title)
        {
            assert(g_Window);
            if (title != g_WindowTitle && SetWindowText(g_Window, title.c_str())) {
                g_WindowTitle = title;
            }
        }

        //-----------------------------------------------------------------
        void SetWindowIcon(Canvas* icon)
        {
            assert(g_Window);
            assert(icon);

            // create temporary buffer
            int buf_size = 40 + icon->getHeight() * icon->getWidth() * 4;
            BlobPtr buf = Blob::Create(buf_size);

            // write the BITMAPINFO header
            writei32l(buf.get(), 40);
            writei32l(buf.get(), icon->getWidth());
            writei32l(buf.get(), icon->getHeight() * 2);
            writei16l(buf.get(), 1);
            writei16l(buf.get(), 32);
            writei32l(buf.get(), BI_RGB);
            writei32l(buf.get(), icon->getHeight() * icon->getWidth() * 4);
            writei32l(buf.get(), 0);
            writei32l(buf.get(), 0);
            writei32l(buf.get(), 0);
            writei32l(buf.get(), 0);

            // write the pixels upside down into the buffer
            for (int y = icon->getHeight()-1; y >= 0; y--) {
                buf->write(icon->getPixels() + y * icon->getWidth(), icon->getPitch());
            }

            // create icon
            HICON icon_handle = CreateIconFromResource(buf->getBuffer(), buf_size, TRUE, 0x00030000);

            // set icon
            SendMessage(g_Window, WM_SETICON, ICON_SMALL, (LPARAM)icon_handle);
            SendMessage(g_Window, WM_SETICON, ICON_BIG,   (LPARAM)icon_handle);
        }

        //-----------------------------------------------------------------
        void SwapWindowBuffers()
        {
            assert(g_Window);
            SwapBuffers(g_DeviceContext);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        //-----------------------------------------------------------------
        bool PeekWindowEvent(int event)
        {
            if (event == -1) {
                return !g_EventQueue.empty();
            }
            for (size_t i = 0; i < g_EventQueue.size(); i++) {
                if (g_EventQueue[i].type == event) {
                    return true;
                }
            }
            return false;
        }

        //-----------------------------------------------------------------
        bool GetWindowEvent(WindowEvent& event)
        {
            if (g_EventQueue.empty()) {
                return false;
            }
            event = g_EventQueue.front();
            g_EventQueue.pop_front();
            return true;
        }

        //-----------------------------------------------------------------
        void ClearWindowEvents()
        {
            g_EventQueue.clear();
        }

        //-----------------------------------------------------------------
        void GetFrameScissor(Recti& scissor)
        {
            GLint rect[4];
            glGetIntegerv(GL_SCISSOR_BOX, rect);

            scissor.ul.x = rect[0];
            scissor.ul.y = rect[1] - g_WindowSize.height + rect[3];
            scissor.lr.x = (rect[0] + rect[2]) - 1;
            scissor.lr.y = (scissor.ul.y + rect[3]) - 1;
        }

        //-----------------------------------------------------------------
        bool SetFrameScissor(const Recti& scissor)
        {
            if (!Recti(0, 0, g_WindowSize.width - 1, g_WindowSize.height - 1).contains(scissor)) {
                return false;
            }
            glScissor(
                scissor.ul.x,
                (g_WindowSize.height - scissor.getY()) - scissor.getHeight(),
                scissor.getWidth(),
                scissor.getHeight()
            );
            return true;
        }

        //-----------------------------------------------------------------
        Canvas* CloneFrame(Recti* section)
        {
            int x = 0;
            int y = 0;
            int w = g_WindowSize.width;
            int h = g_WindowSize.height;

            if (section) {
                if (!section->isValid() || !Recti(0, 0, g_WindowSize.width - 1, g_WindowSize.height - 1).contains(*section)) {
                    return 0;
                }
                x = section->getX();
                y = g_WindowSize.height - (section->getY() + section->getHeight());
                w = section->getWidth();
                h = section->getHeight();
            }

            CanvasPtr canvas = Canvas::Create(w, h);
            glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, canvas->getPixels());
            canvas->flipHorizontally();
            return canvas.release();
        }

        //-----------------------------------------------------------------
        ITexture* CreateTexture(int width, int height, const RGBA* pixels)
        {
            assert(width  > 0);
            assert(height > 0);

            int tex_w = width;
            int tex_h = height;

            if (!g_NonPowerOfTwoTexturesSupported) {
                double log2_w = log10((double)tex_w) / log10(2.0);
                double log2_h = log10((double)tex_h) / log10(2.0);

                if (log2_w != floor(log2_w)) {
                    tex_w = 1 << (int)ceil(log2_w);
                }

                if (log2_h != floor(log2_h)) {
                    tex_h = 1 << (int)ceil(log2_h);
                }
            }

            // make sure texture is, at max, g_MaxTextureSize by g_MaxTextureSize
            if (tex_w > g_MaxTextureSize ||
                tex_h > g_MaxTextureSize)
            {
                return 0;
            }

            RGBA* tex_p = 0;

            if (pixels) {
                tex_p = (RGBA*)pixels;

                if (tex_w != width || tex_h != height) {
                    // allocate a new pixel buffer
                    tex_p = new RGBA[tex_w * tex_h];

                    // copy the pixels into the new buffer
                    for (int i = 0; i < height; i++) {
                        memcpy(tex_p + i * tex_w, pixels + i * width, width * sizeof(RGBA));
                    }
                }
            }

            // create texture name
            GLuint tex_n;
            glGenTextures(1, &tex_n);

            // bind texture
            glBindTexture(GL_TEXTURE_2D, tex_n);

            // set up wrap parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            // set up filter parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            // define pixels
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_p);

            // unbind texture
            glBindTexture(GL_TEXTURE_2D, 0);

            // if we allocated a buffer, delete it
            if (tex_p && tex_p != pixels) {
                delete[] tex_p;
            }

            Texture* t = new Texture;
            t->textureName = tex_n;
            t->textureSize = Dim2i(tex_w, tex_h);
            t->size        = Dim2i(width, height);

            return t;
        }

        //-----------------------------------------------------------------
        bool UpdateTexturePixels(ITexture* texture, Canvas* newPixels, Recti* section)
        {
            assert(texture);
            assert(newPixels);

            Texture* t = (Texture*)texture;

            int x = 0;
            int y = 0;
            int w = newPixels->getWidth();
            int h = newPixels->getHeight();

            if (section) {
                if (!section->isValid() || !Recti(0, 0, newPixels->getWidth() - 1, newPixels->getHeight() - 1).contains(*section)) {
                    return false;
                }

                x = section->getX();
                y = section->getY();
                w = section->getWidth();
                h = section->getHeight();
            }

            if (w != newPixels->getWidth() ||
                h != newPixels->getHeight())
            {
                return false;
            }

            // bind texture
            glBindTexture(GL_TEXTURE_2D, t->textureName);

            // update texture pixels
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, newPixels->getPixels());

            // unbind texture
            glBindTexture(GL_TEXTURE_2D, 0);

            return true;
        }

        //-----------------------------------------------------------------
        Canvas* GrabTexturePixels(ITexture* texture)
        {
            assert(texture);

            Texture* t = (Texture*)texture;

            // create canvas
            CanvasPtr canvas = Canvas::Create(t->textureSize.width, t->textureSize.height);

            // bind texture
            glBindTexture(GL_TEXTURE_2D, t->textureName);

            // copy texture pixels into canvas
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas->getPixels());

            // unbind texture
            glBindTexture(GL_TEXTURE_2D, 0);

            // resize canvas to the real image dimensions
            canvas->resize(t->size.width, t->size.height);

            return canvas.release();
        }

        //-----------------------------------------------------------------
        void DrawPoint(const Vec2i& pos, const RGBA& color)
        {
            glBegin(GL_POINTS);

            glColor4ubv((GLubyte*)&color);
            glVertex2i(pos.x, pos.y);

            glEnd();
        }

        //-----------------------------------------------------------------
        void DrawLine(Vec2i pos[2], RGBA col[2])
        {
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
        void DrawRect(const Recti& rect, RGBA col[4])
        {
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
            assert(image);

            Texture* t = (Texture*)image;
            GLfloat  w = (GLfloat)t->size.width  / (GLfloat)t->textureSize.width;
            GLfloat  h = (GLfloat)t->size.height / (GLfloat)t->textureSize.height;

            glBindTexture(GL_TEXTURE_2D, t->textureName);
            glEnable(GL_TEXTURE_2D);

            glBegin(GL_QUADS);
            glColor4ubv((GLubyte*)&mask);

            glTexCoord2f(0, 0);
            glVertex2i(pos.x, pos.y);

            glTexCoord2f(w, 0);
            glVertex2i(pos.x + t->size.width, pos.y);

            glTexCoord2f(w, h);
            glVertex2i(pos.x + t->size.width, pos.y + t->size.height);

            glTexCoord2f(0, h);
            glVertex2i(pos.x, pos.y + t->size.height);

            glEnd();
            glDisable(GL_TEXTURE_2D);
        }

        //-----------------------------------------------------------------
        void DrawSubImage(ITexture* image, const Recti& src_rect, const Vec2i& pos, const RGBA& mask)
        {
            assert(image);

            if (!src_rect.isValid() ||
                !Recti(0, 0, image->getSize().width-1, image->getSize().height-1).contains(src_rect))
            {
                return;
            }

            Texture* t = (Texture*)image;
            GLfloat  x = (GLfloat)src_rect.getX()      / (GLfloat)t->textureSize.width;
            GLfloat  y = (GLfloat)src_rect.getY()      / (GLfloat)t->textureSize.height;
            GLfloat  w = (GLfloat)src_rect.getWidth()  / (GLfloat)t->textureSize.width;
            GLfloat  h = (GLfloat)src_rect.getHeight() / (GLfloat)t->textureSize.height;

            glBindTexture(GL_TEXTURE_2D, t->textureName);
            glEnable(GL_TEXTURE_2D);

            glBegin(GL_QUADS);
            glColor4ubv((GLubyte*)&mask);

            glTexCoord2f(x, y);
            glVertex2i(pos.x, pos.y);

            glTexCoord2f(x + w, y);
            glVertex2i(pos.x + src_rect.getWidth(), pos.y);

            glTexCoord2f(x + w, y + h);
            glVertex2i(pos.x + src_rect.getWidth(), pos.y + src_rect.getHeight());

            glTexCoord2f(x, y + h);
            glVertex2i(pos.x, pos.y + src_rect.getHeight());

            glEnd();
            glDisable(GL_TEXTURE_2D);
        }

        //-----------------------------------------------------------------
        void DrawImageQuad(ITexture* texture, Vec2i pos[4], const RGBA& mask)
        {
            assert(texture);

            Texture* t = (Texture*)texture;
            GLfloat  w = (GLfloat)t->size.width  / (GLfloat)t->textureSize.width;
            GLfloat  h = (GLfloat)t->size.height / (GLfloat)t->textureSize.height;

            glBindTexture(GL_TEXTURE_2D, t->textureName);
            glEnable(GL_TEXTURE_2D);

            glBegin(GL_QUADS);
            glColor4ubv((GLubyte*)&mask);

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
            assert(texture);

            if (!src_rect.isValid() ||
                !Recti(0, 0, texture->getSize().width-1, texture->getSize().height-1).contains(src_rect))
            {
                return;
            }

            Texture* t = (Texture*)texture;
            GLfloat  x = (GLfloat)src_rect.getX()      / (GLfloat)t->textureSize.width;
            GLfloat  y = (GLfloat)src_rect.getY()      / (GLfloat)t->textureSize.height;
            GLfloat  w = (GLfloat)src_rect.getWidth()  / (GLfloat)t->textureSize.width;
            GLfloat  h = (GLfloat)src_rect.getHeight() / (GLfloat)t->textureSize.height;

            glBindTexture(GL_TEXTURE_2D, t->textureName);
            glEnable(GL_TEXTURE_2D);

            glBegin(GL_QUADS);
            glColor4ubv((GLubyte*)&mask);

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
        void DrawTexturedTriangle(ITexture* texture, Vec2i texcoord[3], Vec2i pos[3], const RGBA& mask)
        {
            assert(texture);

            Texture* t  = (Texture*)texture;
            GLfloat  tw = (GLfloat)t->textureSize.width;
            GLfloat  th = (GLfloat)t->textureSize.height;

            glBindTexture(GL_TEXTURE_2D, t->textureName);
            glEnable(GL_TEXTURE_2D);

            glBegin(GL_TRIANGLES);
            glColor4ubv((GLubyte*)&mask);

            glTexCoord2f(texcoord[0].x / tw, texcoord[0].y / th);
            glVertex2i(pos[0].x, pos[0].y);

            glTexCoord2f(texcoord[1].x / tw, texcoord[1].y / th);
            glVertex2i(pos[1].x, pos[1].y);

            glTexCoord2f(texcoord[2].x / tw, texcoord[2].y / th);
            glVertex2i(pos[2].x, pos[2].y);

            glEnd();
            glDisable(GL_TEXTURE_2D);
        }

        namespace internal {

            //-----------------------------------------------------------------
            void OnKeyPress(int key)
            {
                if (g_EventQueue.size() < EVENT_QUEUE_MAX_SIZE) {
                    WindowEvent event;
                    event.type = WindowEvent::KEY_PRESS;
                    event.key.which = key;
                    g_EventQueue.push_back(event);
                }
            }

            //-----------------------------------------------------------------
            void OnKeyRelease(int key)
            {
                if (g_EventQueue.size() < EVENT_QUEUE_MAX_SIZE) {
                    WindowEvent event;
                    event.type = WindowEvent::KEY_RELEASE;
                    event.key.which = key;
                    g_EventQueue.push_back(event);
                }
            }

            //-----------------------------------------------------------------
            void OnMouseButtonPress(int button)
            {
                if (g_EventQueue.size() < EVENT_QUEUE_MAX_SIZE) {
                    WindowEvent event;
                    event.type = WindowEvent::MOUSE_BUTTON_PRESS;
                    event.mouse.button.which = button;
                    g_EventQueue.push_back(event);
                }
            }

            //-----------------------------------------------------------------
            void OnMouseButtonRelease(int button)
            {
                if (g_EventQueue.size() < EVENT_QUEUE_MAX_SIZE) {
                    WindowEvent event;
                    event.type = WindowEvent::MOUSE_BUTTON_RELEASE;
                    event.mouse.button.which = button;
                    g_EventQueue.push_back(event);
                }
            }

            //-----------------------------------------------------------------
            void OnMouseMotion(int x, int y)
            {
                if (g_EventQueue.size() < EVENT_QUEUE_MAX_SIZE) {
                    WindowEvent event;
                    event.type = WindowEvent::MOUSE_MOTION;
                    event.mouse.motion.x = x;
                    event.mouse.motion.y = y;
                    g_EventQueue.push_back(event);
                }
            }

            //-----------------------------------------------------------------
            void OnMouseWheelMotion(int delta)
            {
                if (g_EventQueue.size() < EVENT_QUEUE_MAX_SIZE) {
                    WindowEvent event;
                    event.type = WindowEvent::MOUSE_WHEEL_MOTION;
                    event.mouse.wheel.delta = delta;
                    g_EventQueue.push_back(event);
                }
            }

            //-----------------------------------------------------------------
            void OnQuitRequest()
            {
                // always generate quit request events
                WindowEvent event;
                event.type = WindowEvent::WINDOW_CLOSE;
                g_EventQueue.push_back(event);
            }


            //-----------------------------------------------------------------
            LRESULT CALLBACK SphereWindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                switch (msg) {
                    case WM_CLOSE: {
                        OnQuitRequest();
                        return 0;
                    }

                    case WM_SYSKEYDOWN:
                        if (wParam == VK_F4) { // WM_SYSKEYDOWN+VK_F4 means user hit ALT+F4
                            OnQuitRequest();
                        }
                    case WM_KEYDOWN: {
                        int key = WinKeyToSphereKey[wParam];
                        if (key != -1) {
                            OnKeyPress(key);
                            return 0;
                        }
                        break;
                    }

                    case WM_SYSKEYUP:
                    case WM_KEYUP: {
                        int key = WinKeyToSphereKey[wParam];
                        if (key != -1) {
                            OnKeyRelease(key);
                            return 0;
                        }
                        break;
                    }

                    case WM_LBUTTONDOWN: {
                        SetCapture(window);
                        OnMouseButtonPress(input::MOUSE_BUTTON_LEFT);
                        return 0;
                    }

                    case WM_LBUTTONUP: {
                        OnMouseButtonRelease(input::MOUSE_BUTTON_LEFT);
                        ReleaseCapture();
                        return 0;
                    }

                    case WM_MBUTTONDOWN: {
                        SetCapture(window);
                        OnMouseButtonPress(input::MOUSE_BUTTON_MIDDLE);
                        return 0;
                    }

                    case WM_MBUTTONUP: {
                        OnMouseButtonRelease(input::MOUSE_BUTTON_MIDDLE);
                        ReleaseCapture();
                        return 0;
                    }

                    case WM_RBUTTONDOWN: {
                        SetCapture(window);
                        OnMouseButtonPress(input::MOUSE_BUTTON_RIGHT);
                        return 0;
                    }

                    case WM_RBUTTONUP: {
                        OnMouseButtonRelease(input::MOUSE_BUTTON_RIGHT);
                        ReleaseCapture();
                        return 0;
                    }

                    case WM_XBUTTONDOWN: {
                        SetCapture(window);
                        OnMouseButtonPress((HIWORD(wParam) == XBUTTON1 ? input::MOUSE_BUTTON_X1 : input::MOUSE_BUTTON_X2));
                        return 0;
                    }

                    case WM_XBUTTONUP: {
                        OnMouseButtonRelease((HIWORD(wParam) == XBUTTON1 ? input::MOUSE_BUTTON_X1 : input::MOUSE_BUTTON_X2));
                        ReleaseCapture();
                        return 0;
                    }

                    case WM_MOUSEMOVE: {
                        SetCursor(NULL);
                        OnMouseMotion(LOWORD(lParam), HIWORD(lParam));
                        return 0;
                    }

                    case WM_MOUSEWHEEL: {
                        OnMouseWheelMotion(GET_WHEEL_DELTA_WPARAM(wParam));
                        return 0;
                    }

                    case WM_PAINT: {
                        // handle the paint message, just don't do anything
                        PAINTSTRUCT ps;
                        BeginPaint(window, &ps);
                        EndPaint(window, &ps);
                        return 0;
                    }
                }

                return DefWindowProc(window, msg, wParam, lParam);
            }

            //-----------------------------------------------------------------
            bool InitVideo(const Log& log)
            {
                PIXELFORMATDESCRIPTOR pfd;
                DEVMODE dm;

                // get default display mode
                memset(&dm, 0, sizeof(dm));
                dm.dmSize = sizeof(dm);
                dm.dmDriverExtra = 0;

                if (!EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm)) {
                    log.error() << "Could not get default display mode";
                    return false;
                }

                log.info() << "Default display mode: " \
                           << dm.dmBitsPerPel \
                           << " Bpp, " \
                           << dm.dmPelsWidth \
                           << " x " \
                           << dm.dmPelsHeight;

                g_DefaultDisplayMode = Dim2i(dm.dmPelsWidth, dm.dmPelsHeight);

                // get available display modes
                int mode_idx = 0;
                while (EnumDisplaySettings(NULL, mode_idx, &dm)) {
                    if (dm.dmBitsPerPel == 32) {
                        bool should_add_mode = true;
                        for (size_t i = 0; i < g_DisplayModes.size(); i++) {
                            if (g_DisplayModes[i].width  == dm.dmPelsWidth &&
                                g_DisplayModes[i].height == dm.dmPelsHeight)
                            {
                                should_add_mode = false;
                                break;
                            }
                        }
                        if (should_add_mode) {
                            log.info() << "Supported display mode: " \
                                       << dm.dmPelsWidth \
                                       << " x " \
                                       << dm.dmPelsHeight;
                            g_DisplayModes.push_back(Dim2i(dm.dmPelsWidth, dm.dmPelsHeight));
                        }
                    }
                    mode_idx++;
                }

                // load a default window icon if possible
                HICON icon_handle = 0;
                FilePtr file = io::filesystem::OpenFile("/common/system/icon.png");
                if (file) {
                    CanvasPtr icon = io::LoadImage(file.get());
                    if (icon) {
                        // create temporary buffer
                        int buf_size = 40 + icon->getHeight() * icon->getWidth() * 4;
                        BlobPtr buf = Blob::Create(buf_size);

                        // write the BITMAPINFO header
                        writei32l(buf.get(), 40);
                        writei32l(buf.get(), icon->getWidth());
                        writei32l(buf.get(), icon->getHeight() * 2);
                        writei16l(buf.get(), 1);
                        writei16l(buf.get(), 32);
                        writei32l(buf.get(), BI_RGB);
                        writei32l(buf.get(), icon->getHeight() * icon->getWidth() * 4);
                        writei32l(buf.get(), 0);
                        writei32l(buf.get(), 0);
                        writei32l(buf.get(), 0);
                        writei32l(buf.get(), 0);

                        // write the pixels upside down into the buffer
                        for (int y = icon->getHeight()-1; y >= 0; y--) {
                            buf->write(icon->getPixels() + y * icon->getWidth(), icon->getPitch());
                        }

                        // create icon
                        icon_handle = CreateIconFromResource(buf->getBuffer(), buf_size, TRUE, 0x00030000);
                    }
                }

                // register window class
                memset(&g_WindowClass, 0, sizeof(g_WindowClass));
                g_WindowClass.lpfnWndProc   = SphereWindowProc;
                g_WindowClass.hInstance     = GetModuleHandle(NULL);
                g_WindowClass.hIcon         = icon_handle;
                g_WindowClass.hCursor       = NULL;
                g_WindowClass.hbrBackground = NULL;
                g_WindowClass.lpszClassName = "SphereWindowClass";

                if (RegisterClass(&g_WindowClass) == 0) {
                    log.error() << "Could not register window class";
                    return false;
                }

                // build default window title
                std::ostringstream oss;
                oss << "Sphere ";
                oss << SPHERE_MAJOR;
                oss << ".";
                oss << SPHERE_MINOR;
                oss << " ";
                oss << SPHERE_AFFIX;

                // create window
                RECT rect = {0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT};
                AdjustWindowRectEx(&rect, WINDOW_STYLE, FALSE, 0);
                g_Window = CreateWindow(
                    "SphereWindowClass",
                    oss.str().c_str(), // default window title
                    WINDOW_STYLE & ~WS_VISIBLE,
                    (GetSystemMetrics(SM_CXSCREEN) - DEFAULT_WINDOW_WIDTH)  / 2,
                    (GetSystemMetrics(SM_CYSCREEN) - DEFAULT_WINDOW_HEIGHT) / 2,
                    rect.right  - rect.left,
                    rect.bottom - rect.top,
                    NULL,
                    NULL,
                    GetModuleHandle(NULL),
                    NULL
                );
                if (!g_Window) {
                    log.error() << "Could not create window";
                    goto init_video_failed;
                }
                g_WindowSize         = Dim2i(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
                g_WindowIsFullScreen = false;
                g_WindowTitle        = oss.str();


                // get window's device context
                g_DeviceContext = GetDC(g_Window);
                if (!g_DeviceContext) {
                    log.error() << "Could not get device context";
                    goto init_video_failed;
                }

                // find appropriate pixel format
                memset(&pfd, 0, sizeof(pfd));
                pfd.nSize      = sizeof(pfd);
                pfd.nVersion   = 1;
                pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
                pfd.iPixelType = PFD_TYPE_RGBA;
                pfd.cColorBits = 32;
                pfd.iLayerType = PFD_MAIN_PLANE;

                int pixel_format = ChoosePixelFormat(g_DeviceContext, &pfd);

                if (pixel_format == 0) {
                    log.error() << "Could not find appropriate pixel format";
                    goto init_video_failed;
                }

                // set device context's pixel format
                if (!SetPixelFormat(g_DeviceContext, pixel_format, &pfd)) {
                    log.error() << "Could not set pixel format";
                    goto init_video_failed;
                }

                // create GL context
                g_GLContext = wglCreateContext(g_DeviceContext);
                if (!g_GLContext) {
                    log.error() << "Could not create GL context";
                    goto init_video_failed;
                }

                // make GL context current
                if (!wglMakeCurrent(g_DeviceContext, g_GLContext)) {
                    log.error() << "Could not make GL context current";
                    goto init_video_failed;
                }

                // get GL vendor
                log.info() << "GL Vendor: " << glGetString(GL_VENDOR);

                // get GL renderer
                log.info() << "GL Renderer: " << glGetString(GL_RENDERER);

                // get GL version
                log.info() << "GL Version: " << glGetString(GL_VERSION);

                // get non-power-of-two textures support
                g_NonPowerOfTwoTexturesSupported = false;
                if (*glGetString(GL_VERSION) >= '2') { // non-power-of-two texture support is a core feature since OpenGL 2.0
                    g_NonPowerOfTwoTexturesSupported = true;
                } else { // see if the implementation exports the GL_ARB_texture_non_power_of_two extension
                    std::string extensions = (const char*)glGetString(GL_EXTENSIONS);
                    if (extensions.find("GL_ARB_texture_non_power_of_two") != std::string::npos) {
                        g_NonPowerOfTwoTexturesSupported = true;
                    }
                }
                log.info() << "Non-power-of-two textures supported: " << (g_NonPowerOfTwoTexturesSupported ? "Yes" : "No");

                // get maximum texture size
                g_MaxTextureSize = 0;
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_MaxTextureSize);
                log.info() << "Maximum texture size: " << g_MaxTextureSize;

                // set up viewport
                glViewport(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

                // set up projection matrix
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0, -1, 1);

                // set up modelview matrix
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glTranslatef(0.375, 0.375, 0.0);

                // set clear color
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

                // set up clipping
                glEnable(GL_SCISSOR_TEST);
                glScissor(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

                // set up blending
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                // disable depth testing
                glDisable(GL_DEPTH_TEST);

                return true;

            init_video_failed:

                if (g_Window) {
                    if (g_DeviceContext) {
                        // reset GL context
                        wglMakeCurrent(g_DeviceContext, 0);

                        // destroy GL context
                        if (g_GLContext) {
                            wglDeleteContext(g_GLContext);
                            g_GLContext = 0;
                        }

                        // release device context
                        ReleaseDC(g_Window, g_DeviceContext);
                        g_DeviceContext = 0;
                    }

                    // destory window
                    DestroyWindow(g_Window);
                    g_Window = 0;
                }

                // unregister window class
                UnregisterClass("SphereWindowClass", GetModuleHandle(NULL));
                memset(&g_WindowClass, 0, sizeof(g_WindowClass));

                return false;
            }

            //-----------------------------------------------------------------
            void DeinitVideo()
            {
                if (g_Window) {
                    if (g_WindowIsFullScreen) {
                        // restore desktop display mode
                        ChangeDisplaySettings(NULL, 0);
                    }

                    if (g_DeviceContext) {
                        // reset GL context
                        wglMakeCurrent(g_DeviceContext, 0);

                        // destroy GL context
                        if (g_GLContext) {
                            wglDeleteContext(g_GLContext);
                            g_GLContext = 0;
                        }

                        // release device context
                        ReleaseDC(g_Window, g_DeviceContext);
                        g_DeviceContext = 0;
                    }

                    // destory window
                    DestroyWindow(g_Window);
                    g_Window = 0;
                }

                // unregister window class
                UnregisterClass("SphereWindowClass", GetModuleHandle(NULL));
                memset(&g_WindowClass, 0, sizeof(g_WindowClass));

                // make hardware mouse cursor visible again
                ShowCursor(TRUE);
            }

            //-----------------------------------------------------------------
            void ProcessWindowEvents()
            {
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    DispatchMessage(&msg);
                }
            }

        } // namespace internal
    } // namespace video
} // namespace sphere
