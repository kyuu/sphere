#include <cstring>
#include <algorithm>
#include "Canvas.hpp"


//-----------------------------------------------------------------
Canvas*
Canvas::Create(int width, int height, const RGBA* pixels)
{
    if (width * height > 0) {
        try {
            CanvasPtr canvas = new Canvas();
            if (canvas->resize(width, height)) {
                if (pixels) {
                    memcpy(canvas->getPixels(), pixels, canvas->getNumPixels() * GetNumBytesPerPixel());
                }
                return canvas.release();
            }
        } catch (const std::bad_alloc& e) {
        }
    }
    return 0;
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
    if (_pixels) {
        delete[] _pixels;
    }
}

//-----------------------------------------------------------------
Canvas*
Canvas::cloneSection(const Recti& section)
{
    assert(_pixels);

    section = section.getIntersection(Recti(0, 0, _width - 1, _height - 1));
    if (!section.isValid()) {
        return 0;
    }

    // create canvas
    CanvasPtr canvas = Canvas::Create(section.getWidth(), section.getHeight());
    if (!canvas) {
        return 0;
    }

    // copy pixels
    for (int i = 0; i < section.getHeight(); ++i) {
        memcpy(canvas->getPixels() + (i * canvas->getWidth()),
               _pixels + (i * _width) + section.getX(),
               section.getWidth() * sizeof(RGBA));
    }

    return section.release();
}

//-----------------------------------------------------------------
const RGBA&
Canvas::getPixel(int x, int y) const
{
    assert(_pixels);
    assert(x >= 0 && x < _width);
    assert(y >= 0 && y < _height);
    return _pixels[_width * y + x];
}

//-----------------------------------------------------------------
void
Canvas::setPixel(int x, int y, const RGBA& color)
{
    assert(_pixels);
    assert(x >= 0 && x < _width);
    assert(y >= 0 && y < _height);
    _pixels[_width * y + x] = color;
}

//-----------------------------------------------------------------
const RGBA&
Canvas::getPixelByIndex(int index) const
{
    assert(_pixels);
    assert(index >= 0 && index < _width * _height);
    return _pixels[index];
}

//-----------------------------------------------------------------
void
Canvas::setPixelByIndex(int index, const RGBA& color)
{
    assert(_pixels);
    assert(index >= 0 && index < _width * _height);
    _pixels[index] = color;
}

//-----------------------------------------------------------------
bool
Canvas::resize(int width, int height)
{
    assert(width > 0);
    assert(height > 0);
    if (width * height <= 0) {
        return false;
    }
    if (width == _width && height == _height) {
        return true;
    }
    RGBA* new_pixels = 0;
    try {
        new_pixels = new RGBA[width * height];
    } catch (const std::bad_alloc& e) {
        return false;
    }
    assert(new_pixels);
    for (int i = 0; i < std::min(_height, height); ++i) {
        memcpy(new_pixels + (i * width), _pixels + (i * _width), std::min(_width, width) * sizeof(RGBA));
    }
    delete[] _pixels;
    _pixels  = new_pixels;
    _width   = width;
    _height  = height;
    return true;
}

//-----------------------------------------------------------------
void
Canvas::fill(const RGBA& color)
{
    assert(_pixels);
    // assuming sizeof(RGBA) == sizeof(u32)
    u32* p = (u32*)_pixels;
    u32  q = *((u32*)&color);
    for (int i = 0, j = _width * _height; i < j; ++i) {
        *p = q;
        p++;
    }
}

//-----------------------------------------------------------------
bool
Canvas::setClipRect(const Recti& rect)
{
    if (rect.intersects(Recti(0, 0, _width - 1, _height - 1))) {
        _clipRect = rect;
        return true;
    }
    return false;
}
