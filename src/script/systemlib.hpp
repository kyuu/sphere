#ifndef SPHERE_SCRIPT_SYSTEMLIB_HPP
#define SPHERE_SCRIPT_SYSTEMLIB_HPP

#include <squirrel.h>


namespace sphere {
    namespace script {
        namespace internal {

            bool RegisterSystemLibrary(HSQUIRRELVM v);

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
