#ifndef VIDEO_HPP
#define VIDEO_HPP

#include "../Log.hpp"
#include "../graphics/Canvas.hpp"
#include "../graphics/RGBA.hpp"
#include "../core/Vec2.hpp"
#include "../core/Rect.hpp"
#include "../core/Dim2.hpp"
#include "ITexture.hpp"


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
ITexture* CreateTexture(Canvas* pixels);
ITexture* CloneFrameBuffer(const Recti& section);

// 2D rendering
void DrawPoint(const Vec2i& pos, const RGBA& col);
void DrawLine(Vec2i pos[2], RGBA col[2]);
void DrawTriangle(Vec2i pos[3], RGBA col[3]);
void DrawTexturedTriangle(ITexture* texture, Vec2i texcoord[3], Vec2i pos[3], const RGBA& mask = RGBA(255, 255, 255));
void DrawRect(const Vec2i& pos, int width, int height, RGBA col[4]);
void DrawImage(ITexture* texture, const Vec2i& pos, const RGBA& mask = RGBA(255, 255, 255));
void DrawSubImage(ITexture* texture, const Recti& src_rect, const Vec2i& pos, const RGBA& mask = RGBA(255, 255, 255));
void DrawImageQuad(ITexture* texture, Vec2i pos[4], const RGBA& mask = RGBA(255, 255, 255));
void DrawSubImageQuad(ITexture* texture, const Recti& src_rect, Vec2i pos[4], const RGBA& mask = RGBA(255, 255, 255));

bool InitVideo(const Log& log);
void DeinitVideo(const Log& log);


#endif
