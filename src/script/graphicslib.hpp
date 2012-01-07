#ifndef SPHERE_SCRIPT_GRAPHICSLIB_HPP
#define SPHERE_SCRIPT_GRAPHICSLIB_HPP

#include <squirrel.h>
#include "../graphics/Canvas.hpp"
#include "../graphics/video.hpp"

// type tags
#define TT_CANVAS  ((SQUserPointer)400)
#define TT_TEXTURE ((SQUserPointer)401)


namespace sphere {
    namespace script {

        bool    BindCanvas(HSQUIRRELVM v, Canvas* canvas);
        Canvas* GetCanvas(HSQUIRRELVM v, SQInteger idx);

        bool      BindTexture(HSQUIRRELVM v, ITexture* texture);
        ITexture* GetTexture(HSQUIRRELVM v, SQInteger idx);

        namespace internal {

            bool RegisterGraphicsLibrary(HSQUIRRELVM v);

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
