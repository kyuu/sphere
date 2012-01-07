#ifndef SPHERE_IMAGEIO_HPP
#define SPHERE_IMAGEIO_HPP

#include "../graphics/Canvas.hpp"
#include "IStream.hpp"


namespace sphere {
    namespace io {

        Canvas* LoadImage(IStream* stream);
        bool    SaveImage(Canvas* image, IStream* stream);

    } // namespace io
} // namespace sphere


#endif
