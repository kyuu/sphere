#include <cstring>
#include <algorithm>
#include "Canvas.hpp"


namespace sphere {

    //-----------------------------------------------------------------
    Canvas*
    Canvas::Create(int width, int height, const RGBA* pixels)
    {
        assert(width > 0);
        assert(height > 0);
        CanvasPtr canvas = new Canvas(width, height);
        if (pixels) {
            memcpy(canvas->getPixels(), pixels, canvas->getNumPixels() * GetNumBytesPerPixel());
        }
        return canvas.release();
    }

    //-----------------------------------------------------------------
    Canvas::Canvas(int width, int height)
        : _width(width)
        , _height(height)
        , _pixels(0)
        , _blendMode(BM_ALPHA)
    {
        assert(width > 0);
        assert(height > 0);
        _pixels = new RGBA[width * height];
        _scissor = Recti(0, 0, width - 1, height - 1);
    }

    //-----------------------------------------------------------------
    Canvas::~Canvas()
    {
        delete[] _pixels;
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
        RGBA* new_pixels = new RGBA[width * height];
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
    Canvas::setAlpha(int alpha)
    {
        RGBA* p = _pixels;
        int   i = _width * _height;
        while (i > 0) {
            p->alpha = (u8)alpha;
            p++;
            i--;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::replaceColor(const RGBA& color, const RGBA& newColor)
    {
        u32  c = *(u32*)&color;
        u32  n = *(u32*)&newColor;
        u32* p =  (u32*)_pixels;
        int  i = _width * _height;
        while (i > 0) {
            if (*p == c) {
                *p = n;
            }
            p++;
            i--;
        }
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
    void
    Canvas::grey()
    {
        RGBA* p = _pixels;
        int   i = _width * _height;
        while (i > 0) {
            u8 greyed = (p->red + p->green + p->blue) / 3;
            p->red   = greyed;
            p->green = greyed;
            p->blue  = greyed;
            i--;
            p++;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::flipHorizontally()
    {
        assert(sizeof(RGBA) == sizeof(u32));

        u32* l = (u32*)_pixels;
        u32* r = (u32*)_pixels + _width - 1;

        int iy = _height;
        while (iy > 0) {
            u32* a = l;
            u32* b = r;
            int ix = _width / 2;
            while (ix > 0) {
                u32 c = *a;
                *a = *b;
                *b = c;
                a++;
                b--;
                ix--;
            }
            l += _width;
            r += _width;
            iy--;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::flipVertically()
    {
        assert(sizeof(RGBA) == sizeof(u32));

        u32* u = (u32*)_pixels;
        u32* d = (u32*)_pixels + _width * (_height - 1);

        int iy = _height / 2;
        while (iy > 0) {
            u32* a = u;
            u32* b = d;
            int ix = _width;
            while (ix > 0) {
                u32 c = *a;
                *a = *b;
                *b = c;
                a++;
                b++;
                ix--;
            }
            u += _width;
            d -= _width;
            iy--;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::rotateCW()
    {
        RGBA* new_p = new RGBA[_width * _height];
        int   new_w = _height;
        int   new_h = _width;

        for (int iy = 0; iy < _height; ++iy) {
            for (int ix = 0; ix < _width; ++ix) {
                new_p[new_w * ix + (new_w - (iy + 1))] = _pixels[iy * _width + ix];
            }
        }

        delete[] _pixels;
        _pixels = new_p;
        _width  = new_w;
        _height = new_h;
    }

    //-----------------------------------------------------------------
    void
    Canvas::rotateCCW()
    {
        RGBA* new_p = new RGBA[_width * _height];
        int   new_w = _height;
        int   new_h = _width;

        for (int iy = 0; iy < _height; ++iy) {
            for (int ix = 0; ix < _width; ++ix) {
                new_p[new_w * (_width - ix - 1) + iy] = _pixels[iy * _width + ix];
            }
        }

        delete[] _pixels;
        _pixels = new_p;
        _width  = new_w;
        _height = new_h;
    }

    //-----------------------------------------------------------------
    bool
    Canvas::setScissor(const Recti& scissor)
    {
        Recti scissorMax(0, 0, _width - 1, _height - 1);
        if (scissorMax.contains(scissor)) {
            _scissor = scissor;
            return true;
        }
        return false;
    }

    //-----------------------------------------------------------------
    bool
    Canvas::setBlendMode(int blendMode)
    {
        switch (blendMode) {
            case BM_REPLACE:
            case BM_ALPHA:
            case BM_ADD:
            case BM_SUBTRACT:
            case BM_MULTIPLY:
                _blendMode = blendMode;
                return true;
            default:
                return false;
        }
    }

    //-----------------------------------------------------------------
    typedef void (*BLENDFUNC_T)(RGBA*, const RGBA&);

    static inline void rgba_replace(RGBA* dst, const RGBA& src)
    {
        dst->red   = src.red;
        dst->green = src.green;
        dst->blue  = src.blue;
        dst->alpha = src.alpha;
    }

    static inline void rgba_alpha(RGBA* dst, const RGBA& src)
    {
        int sa =        src.alpha  + 1;
        int da = (255 - src.alpha) + 1;
        dst->red   = (dst->red   * da + src.red   * sa) >> 8;
        dst->green = (dst->green * da + src.green * sa) >> 8;
        dst->blue  = (dst->blue  * da + src.blue  * sa) >> 8;
    }

    static inline void rgba_add(RGBA* dst, const RGBA& src)
    {
        dst->red   = std::min(dst->red   + src.red,   255);
        dst->green = std::min(dst->green + src.green, 255);
        dst->blue  = std::min(dst->blue  + src.blue,  255);
    }

    static inline void rgba_subtract(RGBA* dst, const RGBA& src)
    {
        dst->red   = std::max(dst->red   - src.red,   0);
        dst->green = std::max(dst->green - src.green, 0);
        dst->blue  = std::max(dst->blue  - src.blue,  0);
    }

    static inline void rgba_multiply(RGBA* dst, const RGBA& src)
    {
        dst->red   = dst->red   * (src.red   + 1) >> 8;
        dst->green = dst->green * (src.green + 1) >> 8;
        dst->blue  = dst->blue  * (src.blue  + 1) >> 8;
    }

    //-----------------------------------------------------------------
    typedef void (*BLENDFUNCFIX_T)(RGBA*, u32, u32, u32, u32);

    static inline void rgba_replace_fix(RGBA* dst, u32 r, u32 g, u32 b, u32 a)
    {
        dst->red   = r >> 12;
        dst->green = g >> 12;
        dst->blue  = b >> 12;
        dst->alpha = a >> 12;
    }

    static inline void rgba_alpha_fix(RGBA* dst, u32 r, u32 g, u32 b, u32 a)
    {
        int sa =        (a >> 12)  + 1;
        int da = (255 - (a >> 12)) + 1;
        dst->red   = (dst->red   * da + (r >> 12) * sa) >> 8;
        dst->green = (dst->green * da + (g >> 12) * sa) >> 8;
        dst->blue  = (dst->blue  * da + (b >> 12) * sa) >> 8;
    }

    static inline void rgba_add_fix(RGBA* dst, u32 r, u32 g, u32 b, u32 a)
    {
        dst->red   = std::min(dst->red   + (u8)(r >> 12), 255);
        dst->green = std::min(dst->green + (u8)(g >> 12), 255);
        dst->blue  = std::min(dst->blue  + (u8)(b >> 12), 255);
    }

    static inline void rgba_subtract_fix(RGBA* dst, u32 r, u32 g, u32 b, u32 a)
    {
        dst->red   = std::max(dst->red   - (u8)(r >> 12), 0);
        dst->green = std::max(dst->green - (u8)(g >> 12), 0);
        dst->blue  = std::max(dst->blue  - (u8)(b >> 12), 0);
    }

    static inline void rgba_multiply_fix(RGBA* dst, u32 r, u32 g, u32 b, u32 a)
    {
        dst->red   = dst->red   * ((r >> 12) + 1) >> 8;
        dst->green = dst->green * ((g >> 12) + 1) >> 8;
        dst->blue  = dst->blue  * ((b >> 12) + 1) >> 8;
    }

    //-----------------------------------------------------------------
    // Liang-Barsky line clipping algorithm
    static inline bool clip_line_test(const float& p, const float& q, float& u1, float& u2)
    {
        float r;

        if (p < 0.0)
        {
            r = q / p;

            if (r > u2)
                return false;
            else if (r > u1)
                u1 = r;
        }
        else if (p > 0.0)
        {
            r = q / p;

            if (r < u1)
                return false;
            else if (r < u2)
                u2 = r;
        }

        return true;
    }

    template<typename clipT>
    static bool clip_line(int& x1, int& y1, int& x2, int& y2, float& u1, float& u2, clipT clip)
    {
        float dx = (float)(x2 - x1);
        float dy;

        if(clip_line_test(-dx, (float)(x1 - clip.ul.x), u1, u2)) {
            if(clip_line_test(dx, (float)(clip.lr.x - x1), u1, u2)) {
                dy = (float)(y2 - y1);

                if(clip_line_test(-dy, (float)(y1 - clip.ul.y), u1, u2)) {
                    if(clip_line_test(dy, (float)(clip.lr.y - y1), u1, u2)) {
                        if (u2 < 1.0) {
                            x2 = (int)(x1 + u2 * dx + 0.5);
                            y2 = (int)(y1 + u2 * dy + 0.5);
                        }

                        if (u1 > 0.0) {
                            x1 = (int)(x1 + u1 * dx + 0.5);
                            y1 = (int)(y1 + u1 * dy + 0.5);
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    }

    //-----------------------------------------------------------------
    template<typename T>
    static inline T bracket(T x, T min, T max)
    {
        if (x < min) {
            return min;
        } else if (x > max) {
            return max;
        } else {
            return x;
        }
    }

    //-----------------------------------------------------------------
    static inline bool is_line_clipped(int x1, int y1, int x2, int y2, const Recti& scissor)
    {
        // return true if any of the endpoints lies outside of the scissor
        return (x1 < scissor.ul.x || x2 < scissor.ul.x ||
                x1 > scissor.lr.x || x2 > scissor.lr.x ||
                y1 < scissor.ul.y || y2 < scissor.ul.y ||
                y1 > scissor.lr.y || y2 > scissor.lr.y);
    }

    //-----------------------------------------------------------------
    template<BLENDFUNC_T blenderT>
    static void draw_line(Canvas& d, int x1, int y1, int x2, int y2, const RGBA& c)
    {
        RGBA* dst = NULL;

        if (y1 == y2 || x1 == x2) { // horizontal or vertical lines (simplified clipping)
            int i1;
            int i2;
            int dst_inc;

            if (y1 == y2) {
                i1 = bracket(x1, d.getScissor().ul.x, d.getScissor().lr.x);
                i2 = bracket(x2, d.getScissor().ul.x, d.getScissor().lr.x);
                dst     = d.getPixels() + y1 * d.getWidth() + i1;
                dst_inc = 1;
            } else {
                i1 = bracket(y1, d.getScissor().ul.y, d.getScissor().lr.y);
                i2 = bracket(y2, d.getScissor().ul.y, d.getScissor().lr.y);
                dst     = d.getPixels() + i1 * d.getWidth() + x1;
                dst_inc = d.getWidth();
            }

            int ix = 2;

            if (i1 <= i2) {
                ix += i2 - i1;
            } else {
                ix += i1 - i2;
                dst_inc = -dst_inc;
            }

            while (--ix) {
                blenderT(dst, c);
                dst += dst_inc;
            }
        } else { // other lines (expensive clipping)
            // check if we need to clip
            if (is_line_clipped(x1, y1, x2, y2, d.getScissor())) {
                float u1 = 0.0, u2 = 1.0;
                if (!clip_line(x1, y1, x2, y2, u1, u2, d.getScissor())) {
                    // nothing to draw
                    return;
                }
            }

            dst = d.getPixels() + y1 * d.getWidth() + x1;

            int dx = abs(x2 - x1);
            int dy = abs(y2 - y1);
            int xinc1, xinc2, yinc1, yinc2;
            int num, den, numadd, numpix;

            if (x2 >= x1) {
                xinc1 = 1;
                xinc2 = 1;
            } else {
                xinc1 = -1;
                xinc2 = -1;
            }

            if (y2 >= y1) {
                yinc1 = d.getWidth();
                yinc2 = d.getWidth();
            } else {
                yinc1 = -d.getWidth();
                yinc2 = -d.getWidth();
            }

            if (dx >= dy) {
                xinc1 = 0;
                yinc2 = 0;
                den = dx;
                num = dx / 2;
                numadd = dy;
                numpix = dx;
            } else {
                xinc2 = 0;
                yinc1 = 0;
                den = dy;
                num = dy / 2;
                numadd = dx;
                numpix = dy;
            }

            for (int i = 0; i <= numpix; ++i) {
                blenderT(dst, c);
                num += numadd;
                if (num >= den) {
                    num -= den;
                    dst += xinc1;
                    dst += yinc1;
                }
                dst += xinc2;
                dst += yinc2;
            }
        }
    }

    //-----------------------------------------------------------------
    template<typename T>
    static inline void clip_color2_parametric(float u1, float u2, T& c1, T& c2)
    {
        RGBA ct = c1;

        c1.red   = (u8)(c1.red   + u1 * (c2.red   - c1.red));
        c1.green = (u8)(c1.green + u1 * (c2.green - c1.green));
        c1.blue  = (u8)(c1.blue  + u1 * (c2.blue  - c1.blue));
        c1.alpha = (u8)(c1.alpha + u1 * (c2.alpha - c1.alpha));

        c2.red   = (u8)(ct.red   + u2 * (c2.red   - ct.red));
        c2.green = (u8)(ct.green + u2 * (c2.green - ct.green));
        c2.blue  = (u8)(ct.blue  + u2 * (c2.blue  - ct.blue));
        c2.alpha = (u8)(ct.alpha + u2 * (c2.alpha - ct.alpha));
    }

    //-----------------------------------------------------------------
    template<BLENDFUNCFIX_T blenderT>
    static void draw_gradient_line(Canvas& d, int x1, int y1, int x2, int y2, RGBA c[2])
    {
        RGBA* dst = NULL;
        RGBA  tc[2] = {c[0], c[1]};

        if (y1 == y2 || x1 == x2) { // horizontal or vertical lines (simplified clipping)
            int itemp, idelta, i1, i2, dst_inc;
            if (y1 == y2) {
                i1 = bracket(x1, d.getScissor().ul.x, d.getScissor().lr.x);
                i2 = bracket(x2, d.getScissor().ul.x, d.getScissor().lr.x);
                itemp   = x1;
                idelta  = x2 - x1;
                dst     = d.getPixels() + y1 * d.getWidth() + i1;
                dst_inc = 1;
            } else {
                i1 = bracket(y1, d.getScissor().ul.y, d.getScissor().lr.y);
                i2 = bracket(y2, d.getScissor().ul.y, d.getScissor().lr.y);
                itemp   = y1;
                idelta  = y2 - y1;
                dst     = d.getPixels() + i1 * d.getWidth() + x1;
                dst_inc = d.getWidth();
            }

            int ix = 2 + abs(i2 - i1);

            if (i1 > i2) {
                dst_inc = -dst_inc;
            }

            if (is_line_clipped(x1, y1, x2, y2, d.getScissor())) {
                float u1 = (idelta == 0) ? 0.0f : (i1 - itemp) / (float)idelta;
                float u2 = (idelta == 0) ? 0.0f : (i2 - itemp) / (float)idelta;
                clip_color2_parametric(u1, u2, tc[0], tc[1]);
            }

            // fixed-point variables for color interpolation (20.12 notation)
            u32 cur_r = tc[0].red   << 12;
            u32 cur_g = tc[0].green << 12;
            u32 cur_b = tc[0].blue  << 12;
            u32 cur_a = tc[0].alpha << 12;

            // (ix - 1) is always > 0
            i32 step_r = (i32)(((tc[1].red   - tc[0].red)   / (float)(ix - 1)) * 4096.0);
            i32 step_g = (i32)(((tc[1].green - tc[0].green) / (float)(ix - 1)) * 4096.0);
            i32 step_b = (i32)(((tc[1].blue  - tc[0].blue)  / (float)(ix - 1)) * 4096.0);
            i32 step_a = (i32)(((tc[1].alpha - tc[0].alpha) / (float)(ix - 1)) * 4096.0);

            while (--ix) {
                blenderT(dst, cur_r, cur_g, cur_b, cur_a);
                dst += dst_inc;

                // interpolate current color
                cur_r += step_r;
                cur_g += step_g;
                cur_b += step_b;
                cur_a += step_a;
            }
        } else { // other lines (expensive clipping)
            // check if we need to clip
            if (is_line_clipped(x1, y1, x2, y2, d.getScissor())) {
                float u1 = 0.0, u2 = 1.0;
                if (!clip_line(x1, y1, x2, y2, u1, u2, d.getScissor())) {
                    return; // nothing to draw
                }
                clip_color2_parametric(u1, u2, tc[0], tc[1]);
            }

            dst = d.getPixels() + y1 * d.getWidth() + x1;

            int dx = abs(x2 - x1);
            int dy = abs(y2 - y1);
            int xinc1, xinc2, yinc1, yinc2;
            int num, den, numadd, numpix;

            if (x2 >= x1) {
                xinc1 = 1;
                xinc2 = 1;
            } else {
                xinc1 = -1;
                xinc2 = -1;
            }

            if (y2 >= y1) {
                yinc1 = d.getWidth();
                yinc2 = d.getWidth();
            } else {
                yinc1 = -d.getWidth();
                yinc2 = -d.getWidth();
            }

            if (dx >= dy) {
                xinc1 = 0;
                yinc2 = 0;
                den = dx;
                num = dx / 2;
                numadd = dy;
                numpix = dx + 1;
            } else {
                xinc2 = 0;
                yinc1 = 0;
                den = dy;
                num = dy / 2;
                numadd = dx;
                numpix = dy + 1;
            }

            // fixed-point variables for color interpolation (20.12 notation)
            u32 cur_r = tc[0].red   << 12;
            u32 cur_g = tc[0].green << 12;
            u32 cur_b = tc[0].blue  << 12;
            u32 cur_a = tc[0].alpha << 12;

            // numpix is always > 0
            i32 step_r = (i32)(((tc[1].red   - tc[0].red)   / (float)numpix) * 4096.0);
            i32 step_g = (i32)(((tc[1].green - tc[0].green) / (float)numpix) * 4096.0);
            i32 step_b = (i32)(((tc[1].blue  - tc[0].blue)  / (float)numpix) * 4096.0);
            i32 step_a = (i32)(((tc[1].alpha - tc[0].alpha) / (float)numpix) * 4096.0);

            for (int i = 0; i < numpix; ++i) {
                blenderT(dst, cur_r, cur_g, cur_b, cur_a);
                num += numadd;
                if (num >= den) {
                    num -= den;
                    dst += xinc1;
                    dst += yinc1;
                }
                dst += xinc2;
                dst += yinc2;

                // interpolate current color
                cur_r += step_r;
                cur_g += step_g;
                cur_b += step_b;
                cur_a += step_a;
            }
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::drawLine(Vec2i pos[2], RGBA col[2])
    {
        if ((pos[0].x > _scissor.lr.x && pos[1].x > _scissor.lr.x) ||
            (pos[0].y > _scissor.lr.y && pos[1].y > _scissor.lr.y) ||
            (pos[0].x < _scissor.ul.x && pos[1].x < _scissor.ul.x) ||
            (pos[0].y < _scissor.ul.y && pos[1].y < _scissor.ul.y))
        {
            return;
        }

        if (col[0] == col[1]) {
            switch (_blendMode) {
            case BM_REPLACE:
                draw_line<rgba_replace>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col[0]);
                break;
            case BM_ALPHA:
                draw_line<rgba_alpha>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col[0]);
                break;
            case BM_ADD:
                draw_line<rgba_add>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col[0]);
                break;
            case BM_SUBTRACT:
                draw_line<rgba_subtract>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col[0]);
                break;
            case BM_MULTIPLY:
                draw_line<rgba_multiply>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col[0]);
                break;
            default:
                break;
            }
        } else {
            switch (_blendMode) {
            case BM_REPLACE:
                draw_gradient_line<rgba_replace_fix>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col);
                break;
            case BM_ALPHA:
                draw_gradient_line<rgba_alpha_fix>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col);
                break;
            case BM_ADD:
                draw_gradient_line<rgba_add_fix>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col);
                break;
            case BM_SUBTRACT:
                draw_gradient_line<rgba_subtract_fix>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col);
                break;
            case BM_MULTIPLY:
                draw_gradient_line<rgba_multiply_fix>(*this, pos[0].x, pos[0].y, pos[1].x, pos[1].y, col);
                break;
            default:
                break;
            }
        }
    }

    //-----------------------------------------------------------------
    template<BLENDFUNC_T blenderT>
    static void draw_rect(Canvas& d, const Recti& rect, const RGBA& col)
    {
        Recti intersection = d.getScissor().getIntersection(rect);
        if (!intersection.isValid()) {
            return;
        }

        RGBA* dst = d.getPixels() + intersection.getY() * d.getWidth() + intersection.getX();
        int iy = intersection.getHeight();
        while (iy > 0) {
            int ix = intersection.getWidth();
            while (ix > 0) {
                blenderT(dst, col);
                dst++;
                ix--;
            }
            dst += d.getWidth() - intersection.getWidth();
            iy--;
        }
    }

    //-----------------------------------------------------------------
    static inline void clip_color2(float u, RGBA& ul, const RGBA& ur, RGBA& ll, const RGBA& lr)
    {
        ul.red   = (u8)(ul.red   + u * (ur.red   - ul.red));
        ul.green = (u8)(ul.green + u * (ur.green - ul.green));
        ul.blue  = (u8)(ul.blue  + u * (ur.blue  - ul.blue));
        ul.alpha = (u8)(ul.alpha + u * (ur.alpha - ul.alpha));

        ll.red   = (u8)(ll.red   + u * (lr.red   - ll.red));
        ll.green = (u8)(ll.green + u * (lr.green - ll.green));
        ll.blue  = (u8)(ll.blue  + u * (lr.blue  - ll.blue));
        ll.alpha = (u8)(ll.alpha + u * (lr.alpha - ll.alpha));
    }

    //-----------------------------------------------------------------
    template<BLENDFUNCFIX_T blenderT>
    static void draw_gradient_rect(Canvas& d, const Recti& rect, RGBA col[4])
    {
        RGBA tc[4] = {col[0], col[1], col[2], col[3]};
        int x = rect.getX();
        int y = rect.getY();
        int w = rect.getWidth();
        int h = rect.getHeight();

        // clip the rectangle and its corner colors
        if (x < d.getScissor().ul.x) {
            clip_color2((float)(d.getScissor().ul.x - x) / w, tc[0], tc[1], tc[3], tc[2]);
            w -= d.getScissor().ul.x - x;
            x  = d.getScissor().ul.x;
        }

        if (y < d.getScissor().ul.y) {
            clip_color2((float)(d.getScissor().ul.y - y) / h, tc[0], tc[3], tc[1], tc[2]);
            h -= d.getScissor().ul.y - y;
            y  = d.getScissor().ul.y;
        }

        if (x + w - 1 > d.getScissor().lr.x) {
            clip_color2((float)((x + w - 1) - d.getScissor().lr.x) / w, tc[1], tc[0], tc[2], tc[3]);
            w = (d.getScissor().lr.x - x) + 1;
        }

        if (y + h - 1 > d.getScissor().lr.y) {
            clip_color2((float)((y + h - 1) - d.getScissor().lr.y) / h, tc[3], tc[0], tc[2], tc[1]);
            h = (d.getScissor().lr.y - y) + 1;
        }

        i32 l_r = tc[0].red   << 12;
        i32 l_g = tc[0].green << 12;
        i32 l_b = tc[0].blue  << 12;
        i32 l_a = tc[0].alpha << 12;

        i32 r_r = tc[1].red   << 12;
        i32 r_g = tc[1].green << 12;
        i32 r_b = tc[1].blue  << 12;
        i32 r_a = tc[1].alpha << 12;

        i32 step_l_r = ((tc[3].red   - tc[0].red)   << 12) / h;
        i32 step_l_g = ((tc[3].green - tc[0].green) << 12) / h;
        i32 step_l_b = ((tc[3].blue  - tc[0].blue)  << 12) / h;
        i32 step_l_a = ((tc[3].alpha - tc[0].alpha) << 12) / h;

        i32 step_r_r = ((tc[2].red   - tc[1].red)   << 12) / h;
        i32 step_r_g = ((tc[2].green - tc[1].green) << 12) / h;
        i32 step_r_b = ((tc[2].blue  - tc[1].blue)  << 12) / h;
        i32 step_r_a = ((tc[2].alpha - tc[1].alpha) << 12) / h;

        RGBA* dst = d.getPixels() + y * d.getWidth() + x;

        for (int iy = 0; iy < h; ++iy)
        {
            // temporary interpolation variables
            i32 cur_r = l_r;
            i32 cur_g = l_g;
            i32 cur_b = l_b;
            i32 cur_a = l_a;

            i32 step_r = (r_r - l_r) / w;
            i32 step_g = (r_g - l_g) / w;
            i32 step_b = (r_b - l_b) / w;
            i32 step_a = (r_a - l_a) / w;

            for (int ix = 0; ix < w; ++ix) {
                blenderT(dst, cur_r, cur_g, cur_b, cur_a);
                dst++;

                // interpolate current color
                cur_r += step_r;
                cur_g += step_g;
                cur_b += step_b;
                cur_a += step_a;
            }

            dst += d.getWidth() - w;

            // interpolate left and right colors
            l_r += step_l_r;
            l_g += step_l_g;
            l_b += step_l_b;
            l_a += step_l_a;

            r_r += step_r_r;
            r_g += step_r_g;
            r_b += step_r_b;
            r_a += step_r_a;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::drawRect(const Recti& rect, RGBA col[4])
    {
        if (!rect.isValid() || !_scissor.intersects(rect)) {
            return;
        }

        if (col[0] == col[1] &&
            col[0] == col[2] &&
            col[0] == col[3])
        {
            switch (_blendMode) {
            case BM_REPLACE:
                draw_rect<rgba_replace>(*this, rect, col[0]);
                break;
            case BM_ALPHA:
                draw_rect<rgba_alpha>(*this, rect, col[0]);
                break;
            case BM_ADD:
                draw_rect<rgba_add>(*this, rect, col[0]);
                break;
            case BM_SUBTRACT:
                draw_rect<rgba_subtract>(*this, rect, col[0]);
                break;
            case BM_MULTIPLY:
                draw_rect<rgba_multiply>(*this, rect, col[0]);
                break;
            default:
                break;
            }
        } else {
            switch (_blendMode) {
            case BM_REPLACE:
                draw_gradient_rect<rgba_replace_fix>(*this, rect, col);
                break;
            case BM_ALPHA:
                draw_gradient_rect<rgba_alpha_fix>(*this, rect, col);
                break;
            case BM_ADD:
                draw_gradient_rect<rgba_add_fix>(*this, rect, col);
                break;
            case BM_SUBTRACT:
                draw_gradient_rect<rgba_subtract_fix>(*this, rect, col);
                break;
            case BM_MULTIPLY:
                draw_gradient_rect<rgba_multiply_fix>(*this, rect, col);
                break;
            default:
                break;
            }
        }
    }

    //-----------------------------------------------------------------
    static inline bool is_point_clipped(int x, int y, const Recti& scissor)
    {
        // return true if point lies outside of the scissor
        return (x > scissor.lr.x ||
                y > scissor.lr.y ||
                x < scissor.ul.x ||
                y < scissor.ul.y);
    }

    //-----------------------------------------------------------------
    template<BLENDFUNC_T blenderT>
    static void draw_circle_outline(Canvas& d, int x, int y, int r, const RGBA& c)
    {
        int f     = 1 - r;
        int ddF_x = 0;
        int ddF_y = -2 * r;

        int ix = 0;
        int iy = r;
        int pitch = d.getWidth();

        const Recti& clip = d.getScissor();

        RGBA* tl = d.getPixels() + (y - r)     * pitch + (x);
        RGBA* tr = d.getPixels() + (y - r)     * pitch + (x - 1);
        RGBA* bl = d.getPixels() + (y + r - 1) * pitch + (x);
        RGBA* br = d.getPixels() + (y + r - 1) * pitch + (x - 1);
        RGBA* lt = d.getPixels() + (y)         * pitch + (x - r);
        RGBA* lb = d.getPixels() + (y - 1)     * pitch + (x - r);
        RGBA* rt = d.getPixels() + (y)         * pitch + (x + r - 1);
        RGBA* rb = d.getPixels() + (y - 1)     * pitch + (x + r - 1);

        while (ix < iy) {
            ix++; tl--; tr++; bl--; br++;
            lt -= pitch; lb += pitch; rt -= pitch; rb += pitch;

            ddF_x += 2;
            f     += ddF_x + 1;

            if (!is_point_clipped(x - ix,     y - iy,     clip))
                blenderT(tl, c); // tl
            if (!is_point_clipped(x + ix - 1, y - iy,     clip))
                blenderT(tr, c); // tr
            if (!is_point_clipped(x - ix,     y + iy - 1, clip))
                blenderT(bl, c); // bl
            if (!is_point_clipped(x + ix - 1, y + iy - 1, clip))
                blenderT(br, c); // br

            if (ix != iy) {
                if (!is_point_clipped(x - iy,     y - ix,     clip))
                    blenderT(lt, c); // lt
                if (!is_point_clipped(x + iy - 1, y - ix,     clip))
                    blenderT(rt, c); // rt
                if (!is_point_clipped(x - iy,     y + ix - 1, clip))
                    blenderT(lb, c); // lb
                if (!is_point_clipped(x + iy - 1, y + ix - 1, clip))
                    blenderT(rb, c); // rb
            }

            if (f >= 0) {
                iy--; lt++; lb++; rt--; rb--;
                tl += pitch; tr += pitch; bl -= pitch; br -= pitch;

                ddF_y += 2;
                f     += ddF_y;
            }
        }
    }

    //-----------------------------------------------------------------
    template<BLENDFUNC_T blenderT>
    static void draw_circle(Canvas& d, int x, int y, int r, const RGBA& c)
    {
        int f     = 1 - r;
        int ddF_x = 0;
        int ddF_y = -2 * r;

        int ix = 0;
        int iy = r;
        int clip_l, clip_r;
        int pitch = d.getWidth();

        const Recti& clip = d.getScissor();
        RGBA* dst         = d.getPixels();
        RGBA* tmp_dst     = NULL;

        while (ix < iy) {
            ix++;
            ddF_x += 2;
            f     += ddF_x + 1;

            clip_l = std::max(x - iy,     clip.ul.x);
            clip_r = std::min(x + iy - 1, clip.lr.x);

            // top half - bottom
            if (y - ix >= clip.ul.y && y - ix <= clip.lr.y) {
                tmp_dst = dst + (y - ix) * pitch + clip_l;
                for (int i = 0, j = clip_r + 1 - clip_l; i < j; ++i, ++tmp_dst) {
                    blenderT(tmp_dst, c);
                }
            }

            // bottom half - top
            if (y + ix - 1 >= clip.ul.y && y + ix - 1 <= clip.lr.y) {
                tmp_dst = dst + (y + ix - 1) * pitch + clip_l;
                for (int i = 0, j = clip_r + 1 - clip_l; i < j; ++i, ++tmp_dst) {
                    blenderT(tmp_dst, c);
                }
            }

            if (f >= 0) {
                if (ix != iy) {
                    clip_l = std::max(x - ix,     clip.ul.x);
                    clip_r = std::min(x + ix - 1, clip.lr.x);

                    // top half - top
                    if (y - iy >= clip.ul.y && y - iy <= clip.lr.y) {
                        tmp_dst = dst + (y - iy) * pitch + clip_l;
                        for (int i = 0, j = clip_r + 1 - clip_l; i < j; ++i, ++tmp_dst) {
                            blenderT(tmp_dst, c);
                        }
                    }

                    // bottom half - bottom
                    if (y + iy - 1 >= clip.ul.y && y + iy - 1 <= clip.lr.y) {
                        tmp_dst = dst + (y + iy - 1) * pitch + clip_l;
                        for (int i = 0, j = clip_r + 1 - clip_l; i < j; ++i, ++tmp_dst) {
                            blenderT(tmp_dst, c);
                        }
                    }
                }

                iy--;
                ddF_y += 2;
                f     += ddF_y;
            }
        }
    }

    //-----------------------------------------------------------------
    template<BLENDFUNC_T blenderT>
    static void draw_gradient_circle(Canvas& d, int x, int y, int r, RGBA col[2])
    {
        RGBA tc[2] = {col[0], col[1]};

        int ix = 1;
        int iy = r;
        int n;

        // channel differences
        float fdr = (float)(tc[1].red   - tc[0].red);
        float fdg = (float)(tc[1].green - tc[0].green);
        float fdb = (float)(tc[1].blue  - tc[0].blue);
        float fda = (float)(tc[1].alpha - tc[0].alpha);

        float dist;
        float factor;
        float fr = (float)r;
        const float PI_H = 3.1415927f / 2.0f;
        const float RR   = (float)(r * r);

        const Recti& clip = d.getScissor();
        int pitch = d.getWidth();
        RGBA* pixels = d.getPixels();

        // draw gradient circle (goes through all points in an octant)
        while (ix <= iy) {
            n = iy + 1;
            while (--n >= ix) {
                dist = sqrt((float)(ix*ix + n*n));

                if (dist > r) {
                    dist = fr;
                }

                // set correct color
                factor = sin((float(1) - dist / fr) * PI_H);
                tc[0].red   = (u8)(tc[1].red   - fdr * factor);
                tc[0].green = (u8)(tc[1].green - fdg * factor);
                tc[0].blue  = (u8)(tc[1].blue  - fdb * factor);
                tc[0].alpha = (u8)(tc[1].alpha - fda * factor);

                // draw the point and reflect it seven times
                if (!is_point_clipped(x + ix - 1, y - n, clip))
                    blenderT(pixels + ((y - n) * pitch + (x + ix - 1)), tc[0]);

                if (!is_point_clipped(x - ix, y - n, clip))
                    blenderT(pixels + ((y - n) * pitch + (x - ix)), tc[0]);

                if (!is_point_clipped(x + ix - 1, y + n - 1, clip))
                    blenderT(pixels + ((y + n - 1) * pitch + (x + ix - 1)), tc[0]);

                if (!is_point_clipped(x - ix, y + n - 1, clip))
                    blenderT(pixels + ((y + n - 1) * pitch + (x - ix)), tc[0]);

                if (ix != n) {
                    if (!is_point_clipped(x + n - 1, y - ix, clip))
                        blenderT(pixels + ((y - ix) * pitch + (x + n - 1)), tc[0]);

                    if (!is_point_clipped(x - n, y - ix, clip))
                        blenderT(pixels + ((y - ix) * pitch + (x - n)), tc[0]);

                    if (!is_point_clipped(x + n - 1, y + ix - 1, clip))
                        blenderT(pixels + ((y + ix - 1) * pitch + (x + n - 1)), tc[0]);

                    if (!is_point_clipped(x - n, y + ix - 1, clip))
                        blenderT(pixels + ((y + ix - 1) * pitch + (x - n)), tc[0]);
                }
            }

            // go always rightwards
            ix++;

            // but check whether to go down
            if (fabs(ix*ix + iy*iy - RR) > fabs(ix*ix + (iy-1)*(iy-1) - RR)) iy--;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::drawCircle(int x, int y, int radius, bool fill, RGBA col[2])
    {
        if (radius <= 0 ||
            x + radius < _scissor.ul.x ||
            x - radius > _scissor.lr.x ||
            y + radius < _scissor.ul.y ||
            y - radius > _scissor.lr.y)
        {
            return;
        }

        if (col[0] == col[1]) {
            if (fill) {
                switch (_blendMode) {
                case BM_REPLACE:
                    draw_circle<rgba_replace>(*this, x, y, radius, col[0]);
                    break;
                case BM_ALPHA:
                    draw_circle<rgba_alpha>(*this, x, y, radius, col[0]);
                    break;
                case BM_ADD:
                    draw_circle<rgba_add>(*this, x, y, radius, col[0]);
                    break;
                case BM_SUBTRACT:
                    draw_circle<rgba_subtract>(*this, x, y, radius, col[0]);
                    break;
                case BM_MULTIPLY:
                    draw_circle<rgba_multiply>(*this, x, y, radius, col[0]);
                    break;
                default:
                    break;
                }
            } else {
                switch (_blendMode) {
                case BM_REPLACE:
                    draw_circle_outline<rgba_replace>(*this, x, y, radius, col[0]);
                    break;
                case BM_ALPHA:
                    draw_circle_outline<rgba_alpha>(*this, x, y, radius, col[0]);
                    break;
                case BM_ADD:
                    draw_circle_outline<rgba_add>(*this, x, y, radius, col[0]);
                    break;
                case BM_SUBTRACT:
                    draw_circle_outline<rgba_subtract>(*this, x, y, radius, col[0]);
                    break;
                case BM_MULTIPLY:
                    draw_circle_outline<rgba_multiply>(*this, x, y, radius, col[0]);
                    break;
                default:
                    break;
                }
            }
        } else {
            switch (_blendMode) {
            case BM_REPLACE:
                draw_gradient_circle<rgba_replace>(*this, x, y, radius, col);
                break;
            case BM_ALPHA:
                draw_gradient_circle<rgba_alpha>(*this, x, y, radius, col);
                break;
            case BM_ADD:
                draw_gradient_circle<rgba_add>(*this, x, y, radius, col);
                break;
            case BM_SUBTRACT:
                draw_gradient_circle<rgba_subtract>(*this, x, y, radius, col);
                break;
            case BM_MULTIPLY:
                draw_gradient_circle<rgba_multiply>(*this, x, y, radius, col);
                break;
            default:
                break;
            }
        }
    }

    //-----------------------------------------------------------------
    template<BLENDFUNC_T blenderT>
    static void draw_image(Canvas& dstImage, const Canvas& srcImage, const Recti& rect, const Vec2i& pos)
    {
        Recti dstRect = dstImage.getScissor().getIntersection(Recti(pos.x, pos.y, pos.x + rect.getWidth() - 1, pos.y + rect.getHeight() - 1));

        if (!dstRect.isValid()) {
            return;
        }

        Recti srcRect = rect;

        if (dstRect.ul.x != pos.x) {
            srcRect.ul.x += dstRect.ul.x - pos.x;
        }
        if (dstRect.ul.y != pos.y) {
            srcRect.ul.y += dstRect.ul.y - pos.y;
        }
        if (dstRect.getWidth() != srcRect.getWidth()) {
            srcRect.lr.x = srcRect.ul.x + dstRect.getWidth() - 1;
        }
        if (dstRect.getHeight() != srcRect.getHeight()) {
            srcRect.lr.y = srcRect.ul.y + dstRect.getHeight() - 1;
        }

        int dpitch = dstImage.getWidth();
        int dinc   = dpitch - dstRect.getWidth();
        RGBA* dp   = dstImage.getPixels() + (dstRect.ul.y * dpitch) + dstRect.ul.x;

        int spitch = srcImage.getWidth();
        int sinc   = spitch - srcRect.getWidth();
        const RGBA* sp = srcImage.getPixels() + (srcRect.ul.y * spitch) + srcRect.ul.x;

        int iy = dstRect.getHeight();
        while (iy > 0) {
            int ix = dstRect.getWidth();
            while (ix > 0) {
                blenderT(dp, *sp);
                dp++;
                sp++;
                ix--;
            }
            dp += dinc;
            sp += sinc;
            iy--;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::drawImage(Canvas* image, const Vec2i& pos)
    {
        assert(image);

        Recti rect(0, 0, image->getWidth() - 1, image->getHeight() - 1);

        switch (_blendMode) {
        case BM_REPLACE:
            draw_image<rgba_replace>(*this, *image, rect, pos);
            break;
        case BM_ALPHA:
            draw_image<rgba_alpha>(*this, *image, rect, pos);
            break;
        case BM_ADD:
            draw_image<rgba_add>(*this, *image, rect, pos);
            break;
        case BM_SUBTRACT:
            draw_image<rgba_subtract>(*this, *image, rect, pos);
            break;
        case BM_MULTIPLY:
            draw_image<rgba_multiply>(*this, *image, rect, pos);
            break;
        default:
            break;
        }
    }

    //-----------------------------------------------------------------
    void
    Canvas::drawSubImage(Canvas* image, const Recti& rect, const Vec2i& pos)
    {
        assert(image);

        if (!rect.isValid() || !Recti(0, 0, image->getWidth()-1, image->getHeight()-1).contains(rect)) {
            return;
        }

        switch (_blendMode) {
        case BM_REPLACE:
            draw_image<rgba_replace>(*this, *image, rect, pos);
            break;
        case BM_ALPHA:
            draw_image<rgba_alpha>(*this, *image, rect, pos);
            break;
        case BM_ADD:
            draw_image<rgba_add>(*this, *image, rect, pos);
            break;
        case BM_SUBTRACT:
            draw_image<rgba_subtract>(*this, *image, rect, pos);
            break;
        case BM_MULTIPLY:
            draw_image<rgba_multiply>(*this, *image, rect, pos);
            break;
        default:
            break;
        }
    }

} // namespace sphere
