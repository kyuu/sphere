#ifndef OPENGLTEXTURE_HPP
#define OPENGLTEXTURE_HPP

#include <GL.h>
#include "../../common/RefImpl.hpp"
#include "../../core/Canvas.hpp"
#include "../ITexture.hpp"


class OpenGLTexture : public RefImpl<ITexture> {
public:
    static OpenGLTexture* Create(GLuint texture, int tex_width, int tex_height, int width, int height);

    GLuint getTexID() const;
    int    getTexWidth() const;
    int    getTexHeight() const;

    // ITexture implementation
    int getWidth() const;
    int getHeight() const;
    bool updatePixels(Canvas* canvas, Recti* section = 0);
    Canvas* createCanvas() const;

private:
    OpenGLTexture(GLuint texture, int tex_width, int tex_height, int width, int height);
    ~OpenGLTexture();

private:
    GLuint _texture_id;
    int _tex_width;
    int _tex_height;
    int _width;
    int _height;
};

//-----------------------------------------------------------------
inline GLuint
OpenGLTexture::getTexID() const
{
    return _texture_id;
}

//-----------------------------------------------------------------
inline int
OpenGLTexture::getTexWidth() const
{
    return _tex_width;
}

//-----------------------------------------------------------------
inline int
OpenGLTexture::getTexHeight() const
{
    return _tex_height;
}


#endif
