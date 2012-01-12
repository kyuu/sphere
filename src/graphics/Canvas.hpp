#ifndef SPHERE_CANVAS_HPP
#define SPHERE_CANVAS_HPP

#include <string>
#include "../common/RefPtr.hpp"
#include "../common/RefImpl.hpp"
#include "../common/IRefCounted.hpp"
#include "../base/Rect.hpp"
#include "RGBA.hpp"


namespace sphere {

    class Canvas : public RefImpl<IRefCounted> {
    public:
        enum BlendMode {
            BM_REPLACE = 0,
            BM_ALPHA,
            BM_ADD,
            BM_SUBTRACT,
            BM_MULTIPLY,
        };

        static int GetNumBytesPerPixel();

        static Canvas* Create(int width, int height, const RGBA* pixels = 0);

        int   getWidth() const;
        int   getHeight() const;
        int   getPitch() const;
        int   getNumPixels() const;
        RGBA* getPixels();
        const RGBA* getPixels() const;
        Canvas* cloneSection(const Recti& section);
        const RGBA& getPixel(int x, int y) const;
        void  setPixel(int x, int y, const RGBA& color);
        const RGBA& getPixelByIndex(int index) const;
        void  setPixelByIndex(int index, const RGBA& color);
        void  resize(int width, int height);
        void  setAlpha(int alpha);
        void  replaceColor(const RGBA& color, const RGBA& newColor);
        void  fill(const RGBA& color);
        void  grey();
        void  flipHorizontally();
        void  flipVertically();
        void  rotateCW();
        void  rotateCCW();
        const Recti& getScissor() const;
        bool  setScissor(const Recti& scissor);
        int   getBlendMode() const;
        bool  setBlendMode(int blendMode);
        void  drawLine(Vec2i pos[2], RGBA col[2]);
        void  drawRect(const Recti& rect, RGBA col[4]);
        void  drawCircle(int x, int y, int radius, bool fill, RGBA col[2]);
        void  drawImage(Canvas* image, const Vec2i& pos);
        void  drawSubImage(Canvas* image, const Recti& rect, const Vec2i& pos);

    private:
        Canvas(int width, int height);
        virtual ~Canvas();

    private:
        int   _width;
        int   _height;
        RGBA* _pixels;
        Recti _scissor;
        int   _blendMode;
    };

    typedef RefPtr<Canvas> CanvasPtr;

    //-----------------------------------------------------------------
    inline int
    Canvas::GetNumBytesPerPixel()
    {
        return sizeof(RGBA);
    }

    //-----------------------------------------------------------------
    inline int
    Canvas::getWidth() const
    {
        return _width;
    }

    //-----------------------------------------------------------------
    inline int
    Canvas::getHeight() const
    {
        return _height;
    }

    //-----------------------------------------------------------------
    inline int
    Canvas::getPitch() const
    {
        return _width * sizeof(RGBA);
    }

    //-----------------------------------------------------------------
    inline int
    Canvas::getNumPixels() const
    {
        return _width * _height;
    }

    //-----------------------------------------------------------------
    inline RGBA*
    Canvas::getPixels()
    {
        return _pixels;
    }

    //-----------------------------------------------------------------
    inline const RGBA*
    Canvas::getPixels() const
    {
        return _pixels;
    }

    //-----------------------------------------------------------------
    inline const Recti&
    Canvas::getScissor() const
    {
        return _scissor;
    }

    //-----------------------------------------------------------------
    inline int
    Canvas::getBlendMode() const
    {
        return _blendMode;
    }

} // namespace sphere


#endif
