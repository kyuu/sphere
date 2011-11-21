#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL.h>
#include "../../common/RefImpl.hpp"
#include "../../graphics/Canvas.hpp"
#include "../ITexture.hpp"


class Texture : public RefImpl<ITexture> {
public:
    static Texture* Create(GLuint texture, int tex_width, int tex_height, int width, int height);

    GLuint getTexID() const;
    int    getTexWidth() const;
    int    getTexHeight() const;

    // ITexture implementation
    int getWidth() const;
    int getHeight() const;
    bool updatePixels(Canvas* new_pixels, Recti* dst_rect = 0);
    Canvas* createCanvas() const;

private:
    Texture(GLuint texture, int tex_width, int tex_height, int width, int height);
    ~Texture();

private:
    GLuint _texture_id;
    int _tex_width;
    int _tex_height;
    int _width;
    int _height;
};

//-----------------------------------------------------------------
inline GLuint
Texture::getTexID() const
{
    return _texture_id;
}

//-----------------------------------------------------------------
inline int
Texture::getTexWidth() const
{
    return _tex_width;
}

//-----------------------------------------------------------------
inline int
Texture::getTexHeight() const
{
    return _tex_height;
}


#endif
