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
        static int GetNumBytesPerPixel();

        static Canvas* Create(int width, int height, const RGBA* pixels = 0);

        int   getWidth() const;
        int   getHeight() const;
        int   getPitch() const;
        int   getNumPixels() const;
        RGBA* getPixels();
        Canvas* cloneSection(const Recti& section);
        const RGBA& getPixel(int x, int y) const;
        void  setPixel(int x, int y, const RGBA& color);
        const RGBA& getPixelByIndex(int index) const;
        void  setPixelByIndex(int index, const RGBA& color);
        void  resize(int width, int height);
        void  fill(const RGBA& color);
        void  flipHorizontally();
        void  flipVertically();
        const Recti& getClipRect() const;
        bool  setClipRect(const Recti& clipRect);

    private:
        Canvas(int width, int height);
        virtual ~Canvas();

    private:
        int   _width;
        int   _height;
        RGBA* _pixels;
        Recti _clipRect;
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
    inline const Recti&
    Canvas::getClipRect() const
    {
        return _clipRect;
    }

} // namespace sphere


#endif
