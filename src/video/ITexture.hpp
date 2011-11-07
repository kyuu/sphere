#ifndef ITEXTURE_HPP
#define ITEXTURE_HPP

#include "../common/IRefCounted.hpp"
#include "../common/RefPtr.hpp"


class ITexture : public IRefCounted {
public:
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual bool updatePixels(Canvas* canvas, Recti* dst_rect = 0) = 0;
    virtual Canvas* createCanvas() const = 0;

protected:
    ~ITexture() { }
};

typedef RefPtr<ITexture> TexturePtr;


#endif
