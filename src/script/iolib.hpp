#ifndef SPHERE_SCRIPT_IOLIB_HPP
#define SPHERE_SCRIPT_IOLIB_HPP

#include <squirrel.h>
#include "../io/filesystem.hpp"
#include "../io/IStream.hpp"
#include "../io/IFile.hpp"

// type tags
#define TT_STREAM ((SQUserPointer)100)
#define TT_FILE   ((SQUserPointer)101)

#include "../Log.hpp"
namespace sphere {
    namespace script {

        bool     BindStream(HSQUIRRELVM v, IStream* stream);
        IStream* GetStream(HSQUIRRELVM v, SQInteger idx);

        bool   BindFile(HSQUIRRELVM v, IFile* file);
        IFile* GetFile(HSQUIRRELVM v, SQInteger idx);

        namespace internal {

            bool RegisterIOLibrary(const Log& log, HSQUIRRELVM v);

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
