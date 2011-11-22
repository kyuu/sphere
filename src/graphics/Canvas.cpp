#include <cstring>
#include <algorithm>
#include "../error.hpp"
#include "Canvas.hpp"


//-----------------------------------------------------------------
Canvas*
Canvas::Create(int width, int height, const RGBA* pixels)
{
    assert(width > 0);
    assert(height > 0);
    try {
        CanvasPtr canvas = new Canvas();
        canvas->internalInit(width, height);
        if (pixels) {
            memcpy(canvas->getPixels(), pixels, canvas->getNumPixels() * GetNumBytesPerPixel());
        }
        return canvas.release();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
Canvas::Canvas()
    : _width(0)
    , _height(0)
    , _pixels(0)
{
}

//-----------------------------------------------------------------
Canvas::~Canvas()
{
    delete[] _pixels;
}

//-----------------------------------------------------------------
void
Canvas::internalInit(int width, int height)
{
    _width    = width;
    _height   = height;
    _pixels   = new RGBA[width * height];
    _clipRect = Recti(0, 0, _width - 1, _height - 1);
}

//-----------------------------------------------------------------
Canvas*
Canvas::cloneSection(const Recti& section)
{
    Recti sec = section.getIntersection(Recti(0, 0, _width - 1, _height - 1));
    if (!sec.isValid()) {
        return 0;
    }
    CanvasPtr canvas = Create(sec.getWidth(), sec.getHeight());
    for (int i = 0; i < sec.getHeight(); ++i) {
        memcpy(canvas->getPixels() + (i * canvas->getWidth()),
               _pixels + (i * _width) + sec.getX(),
               sec.getWidth() * sizeof(RGBA));
    }
    return canvas.release();
}

//-----------------------------------------------------------------
const RGBA&
Canvas::getPixel(int x, int y) const
{
    assert(x >= 0 && x < _width);
    assert(y >= 0 && y < _height);
    return _pixels[_width * y + x];
}

//-----------------------------------------------------------------
void
Canvas::setPixel(int x, int y, const RGBA& color)
{
    assert(x >= 0 && x < _width);
    assert(y >= 0 && y < _height);
    _pixels[_width * y + x] = color;
}

//-----------------------------------------------------------------
const RGBA&
Canvas::getPixelByIndex(int index) const
{
    assert(index >= 0 && index < _width * _height);
    return _pixels[index];
}

//-----------------------------------------------------------------
void
Canvas::setPixelByIndex(int index, const RGBA& color)
{
    assert(index >= 0 && index < _width * _height);
    _pixels[index] = color;
}

//-----------------------------------------------------------------
void
Canvas::resize(int width, int height)
{
    assert(width > 0);
    assert(height > 0);
    if (width == _width && height == _height) {
        return;
    }
    RGBA* new_pixels = 0;
    try {
        new_pixels = new RGBA[width * height];
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return;
    }
    for (int i = 0; i < std::min(_height, height); ++i) {
        memcpy(new_pixels + (i * width), _pixels + (i * _width), std::min(_width, width) * sizeof(RGBA));
    }
    delete[] _pixels;
    _pixels  = new_pixels;
    _width   = width;
    _height  = height;
}

//-----------------------------------------------------------------
void
Canvas::fill(const RGBA& color)
{
    assert(sizeof(RGBA) == sizeof(u32));
    u32* p = (u32*)_pixels;
    u32  q = *((u32*)&color);
    for (int i = 0, j = _width * _height; i < j; ++i) {
        *p = q;
        p++;
    }
}

//-----------------------------------------------------------------
bool
Canvas::setClipRect(const Recti& clipRect)
{
    Recti clipRectMax(0, 0, _width - 1, _height - 1);
    if (clipRectMax.contains(clipRect)) {
        _clipRect = clipRect;
        return true;
    }
    return false;
}
