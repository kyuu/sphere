#ifndef SPHERE_SCRIPT_COMPRESSIONLIB_HPP
#define SPHERE_SCRIPT_COMPRESSIONLIB_HPP

#include <squirrel.h>
#include "../compression/ZStream.hpp"

// type tags
#define TT_ZSTREAM ((SQUserPointer)800)


namespace sphere {
    namespace script {

        bool     BindZStream(HSQUIRRELVM v, ZStream* stream);
        ZStream* GetZStream(HSQUIRRELVM v, SQInteger idx);

        namespace internal {

            bool RegisterCompressionLibrary(HSQUIRRELVM v);

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
