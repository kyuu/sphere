#include "../../error.hpp"
#include "Texture.hpp"


//-----------------------------------------------------------------
Texture*
Texture::Create(GLuint tex, int tex_width, int tex_height, int width, int height)
{
    try {
        return new Texture(tex, tex_width, tex_height, width, height);
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
Texture::Texture(GLuint tex, int tex_width, int tex_height, int width, int height)
    : _tex(tex)
    , _tex_width(tex_width)
    , _tex_height(tex_height)
    , _width(width)
    , _height(height)
{
}

//-----------------------------------------------------------------
Texture::~Texture()
{
    glDeleteTextures(1, &_tex);
}

//-----------------------------------------------------------------
int
Texture::getWidth() const
{
    return _width;
}

//-----------------------------------------------------------------
int
Texture::getHeight() const
{
    return _height;
}

//-----------------------------------------------------------------
bool
Texture::updatePixels(Canvas* new_pixels, Recti* dst_rect)
{
    assert(new_pixels);
    Recti rect(0, 0, _width - 1, _height - 1);
    if (dst_rect) {
        if (!rect.contains(*dst_rect)) {
            return false;
        }
        rect = *dst_rect;
    }
    if (new_pixels->getWidth()  != rect.getWidth() ||
        new_pixels->getHeight() != rect.getHeight())
    {
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, _tex);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        rect.getX(),
        rect.getY(),
        rect.getWidth(),
        rect.getHeight(),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        new_pixels->getPixels()
    );
    return true;
}

//-----------------------------------------------------------------
Canvas*
Texture::createCanvas() const
{
    CanvasPtr canvas = Canvas::Create(_tex_width, _tex_height);
    glBindTexture(GL_TEXTURE_2D, _tex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas->getPixels());
    canvas->resize(_width, _height);
    return canvas.release();
}
