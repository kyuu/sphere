#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "Canvas.hpp"
#include "../io/IStream.hpp"


namespace graphics {

    Canvas* LoadImage(IStream* stream);
    bool    SaveImage(IStream* stream, Canvas* image);

} // namespace graphics


#endif
