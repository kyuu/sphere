#ifndef VIDEO_HPP
#define VIDEO_HPP

#include "../Log.hpp"
#include "../graphics/Canvas.hpp"
#include "../graphics/RGBA.hpp"
#include "../core/Vec2.hpp"
#include "../core/Rect.hpp"
#include "../core/Dim2.hpp"
#include "ITexture.hpp"


namespace video {

    void GetSupportedVideoModes(std::vector<Dim2i>& out);
    bool OpenWindow(int width, int height, bool fullscreen);
    int  GetWindowWidth();
    int  GetWindowHeight();
    bool IsWindowFullscreen();
    bool SetWindowFullscreen(bool fullscreen);
    const char* GetWindowTitle();
    void SetWindowTitle(const char* title);
    void SetWindowIcon(Canvas* canvas);
    void SwapFrameBuffers();
    bool GetClipRect(Recti& out);
    void SetClipRect(const Recti& clip);
    ITexture* CreateTexture(Canvas* canvas);
    ITexture* CloneFrameBuffer(Recti* section = 0);

    // 2D
    void DrawPoint(const Vec2i& pos, const RGBA& col);
    void DrawLine(Vec2i pos[2], RGBA col[2]);
    void DrawTriangle(Vec2i pos[3], RGBA col[3]);
    void DrawTexturedTriangle(ITexture* texture, Vec2i texcoord[3], Vec2i pos[3], RGBA* mask_col = 0);
    void DrawRect(const Recti& rect, RGBA col[4]);
    void DrawImage(ITexture* texture, const Vec2i& pos, RGBA* mask_col = 0);
    void DrawSubImage(ITexture* texture, const Recti& src_rect, const Vec2i& pos, RGBA* mask_col = 0);
    void DrawImageQuad(ITexture* texture, Vec2i pos[4], RGBA* mask_col = 0);
    void DrawSubImageQuad(ITexture* texture, const Recti& src_rect, Vec2i pos[4], RGBA* mask_col = 0);

    namespace internal {

        bool InitVideo(const Log& log);
        void DeinitVideo(const Log& log);

    } // namespace internal

} // namespace video


#endif
