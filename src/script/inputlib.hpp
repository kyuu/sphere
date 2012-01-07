#ifndef SPHERE_SCRIPT_INPUTLIB_HPP
#define SPHERE_SCRIPT_INPUTLIB_HPP

#include <squirrel.h>


namespace sphere {
    namespace script {
        namespace internal {

            bool RegisterInputLibrary(HSQUIRRELVM v);

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
