#ifndef SPHERE_SCRIPT_BASELIB_HPP
#define SPHERE_SCRIPT_BASELIB_HPP

#include <squirrel.h>
#include "../base/Rect.hpp"
#include "../base/Vec2.hpp"
#include "../base/Blob.hpp"

// type tags
#define TT_RECT   ((SQUserPointer)300)
#define TT_VEC2   ((SQUserPointer)301)
#define TT_BLOB   ((SQUserPointer)302)


namespace sphere {
    namespace script {

        bool   BindRect(HSQUIRRELVM v, const Recti& rect);
        Recti* GetRect(HSQUIRRELVM v, SQInteger idx);

        bool   BindVec2(HSQUIRRELVM v, const Vec2f& vec);
        Vec2f* GetVec2(HSQUIRRELVM v, SQInteger idx);

        bool  BindBlob(HSQUIRRELVM v, Blob* blob);
        Blob* GetBlob(HSQUIRRELVM v, SQInteger idx);

        namespace internal {

            bool RegisterBaseLibrary(HSQUIRRELVM v);

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
