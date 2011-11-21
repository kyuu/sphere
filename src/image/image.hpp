#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "../graphics/Canvas.hpp"
#include "../io/IStream.hpp"


Canvas* LoadImage(IStream* stream);
bool    SaveImage(IStream* stream, Canvas* image);


#endif
