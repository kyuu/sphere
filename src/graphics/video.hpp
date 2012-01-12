#ifndef SPHERE_VIDEO_HPP
#define SPHERE_VIDEO_HPP

#include <string>
#include <vector>
#include "../Log.hpp"
#include "../base/Vec2.hpp"
#include "../base/Rect.hpp"
#include "../base/Dim2.hpp"
#include "ITexture.hpp"
#include "Canvas.hpp"
#include "RGBA.hpp"


namespace sphere {
    namespace video {

        // display
        const Dim2i& GetDefaultDisplayMode();
        const std::vector<Dim2i>& GetDisplayModes();

        // window
        union WindowEvent {
            enum Type {
                KEY_PRESS = 0,
                KEY_RELEASE,

                MOUSE_BUTTON_PRESS,
                MOUSE_BUTTON_RELEASE,
                MOUSE_MOTION,
                MOUSE_WHEEL_MOTION,

                WINDOW_CLOSE,
            };

            int type;

            struct {
                int type;
                int which;
            } key;

            union {
                struct {
                    int type;
                    int which;
                } button;

                struct {
                    int type;
                    int x;
                    int y;
                } motion;

                struct {
                    int type;
                    int delta;
                } wheel;

            } mouse;
        };

        bool SetWindowMode(int width, int height, bool fullScreen);
        const Dim2i& GetWindowSize();
        bool IsWindowFullScreen();
        bool IsWindowActive();
        const std::string& GetWindowTitle();
        void SetWindowTitle(const std::string& title);
        void SetWindowIcon(Canvas* icon);
        void SwapWindowBuffers();
        bool PeekWindowEvent(int event = -1);
        bool GetWindowEvent(WindowEvent& event);
        void ClearWindowEvents();

        // frame buffer
        void GetFrameScissor(Recti& scissor);
        bool SetFrameScissor(const Recti& scissor);
        Canvas* CloneFrame(Recti* section = 0);

        enum {
            BM_REPLACE = 0,
            BM_ALPHA,
            BM_ADD,
            BM_SUBTRACT,
            BM_MULTIPLY,
        };

        int  GetBlendMode();
        bool SetBlendMode(int blendMode);

        bool CaptureFrame(const Recti& rect);
        void DrawCaptureQuad(const Recti& rect, Vec2i pos[4], const RGBA& mask = RGBA(255, 255, 255));

        ITexture* CreateTexture(int width, int height, const RGBA* pixels = 0);
        bool UpdateTexturePixels(ITexture* texture, Canvas* newPixels, Recti* section = 0);
        Canvas* GrabTexturePixels(ITexture* texture);

        void DrawPoint(const Vec2i& pos, const RGBA& col);
        void DrawLine(Vec2i pos[2], RGBA col[2]);
        void DrawTriangle(Vec2i pos[3], RGBA col[3]);
        void DrawRect(const Recti& rect, RGBA col[4]);
        void DrawImage(ITexture* image, const Vec2i& pos, const RGBA& mask = RGBA(255, 255, 255));
        void DrawSubImage(ITexture* image, const Recti& src_rect, const Vec2i& pos, const RGBA& mask = RGBA(255, 255, 255));
        void DrawImageQuad(ITexture* image, Vec2i pos[4], const RGBA& mask = RGBA(255, 255, 255));
        void DrawSubImageQuad(ITexture* image, const Recti& src_rect, Vec2i pos[4], const RGBA& mask = RGBA(255, 255, 255));
        void DrawTexturedTriangle(ITexture* texture, Vec2i texcoord[3], Vec2i pos[3], const RGBA& mask = RGBA(255, 255, 255));

        namespace internal {

            bool InitVideo(const Log& log);
            void DeinitVideo();
            void ProcessWindowEvents();

        } // namespace internal
    } // namespace video
} // namespace sphere


#endif
