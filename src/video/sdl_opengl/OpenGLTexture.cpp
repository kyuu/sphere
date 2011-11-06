#include "OpenGLTexture.hpp"


//-----------------------------------------------------------------
OpenGLTexture*
OpenGLTexture::Create(GLuint texture_id, int tex_width, int tex_height, int width, int height)
{
    try {
        return new OpenGLTexture(texture_id, tex_width, tex_height, canvas->getWidth(), canvas->getHeight());
    } catch (const std::bad_alloc& e) {
        return 0;
    }
}

//-----------------------------------------------------------------
OpenGLTexture::OpenGLTexture(GLuint texture_id, int tex_width, int tex_height, int width, int height)
    : _texture_id(texture_id)
    , _tex_width(tex_width)
    , _tex_height(tex_height)
    , _width(width)
    , _height(height)
{
}

//-----------------------------------------------------------------
OpenGLTexture::~OpenGLTexture()
{
    glDeleteTextures(1, &_texture_id);
}

//-----------------------------------------------------------------
int
OpenGLTexture::getWidth() const
{
    return _width;
}

//-----------------------------------------------------------------
int
OpenGLTexture::getHeight() const
{
    return _height;
}

//-----------------------------------------------------------------
bool
OpenGLTexture::updatePixels(Canvas* canvas, Recti* section)
{
    if (!canvas) {
        return false;
    }
    Recti rect(0, 0, _width - 1, _height - 1);
    if (section) {
        if (!rect.contains(*section)) {
            return false;
        }
        rect = *section;
    }
    if (canvas->getWidth()  != rect.getWidth() ||
        canvas->getHeight() != rect.getHeight())
    {
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        rect.getX(),
        rect.getY(),
        rect.getWidth(),
        rect.getHeight(),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        canvas->getPixels()
    );

    return true;
}

//-----------------------------------------------------------------
Canvas*
OpenGLTexture::createCanvas() const
{
    CanvasPtr canvas = Canvas::Create(_tex_width, _tex_height);
    if (!canvas) {
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas->getPixels());

    if (canvas->resize(_width, _height)) {
        return canvas.release();
    }

    return 0;
}
