#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <squirrel.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include "../common/ArrayPtr.hpp"
#include "../error.hpp"
#include "../version.hpp"
#include "../system/system.hpp"
#include "../io/endian.hpp"
#include "../io/data.hpp"
#include "../io/filesystem.hpp"
#include "../graphics/image.hpp"
#include "../graphics/video.hpp"
#include "../sound/audio.hpp"
#include "../input/input.hpp"
#include "script.hpp"
#include "typetags.hpp"
#include "macros.hpp"

#define   SCRIPT_FILE_EXT ".nut"
#define BYTECODE_FILE_EXT ".nutc"

// marshal magic numbers
#define MARSHAL_MAGIC_NULL         ((u32)0x6a9edf86)
#define MARSHAL_MAGIC_BOOL_TRUE    ((u32)0x57011f5f)
#define MARSHAL_MAGIC_BOOL_FALSE   ((u32)0x92a5f14e)
#define MARSHAL_MAGIC_INTEGER      ((u32)0xd214baed)
#define MARSHAL_MAGIC_FLOAT        ((u32)0x398ff0c7)
#define MARSHAL_MAGIC_STRING       ((u32)0x8bc3326d)
#define MARSHAL_MAGIC_TABLE        ((u32)0x88218321)
#define MARSHAL_MAGIC_ARRAY        ((u32)0xc89200a8)
#define MARSHAL_MAGIC_CLOSURE      ((u32)0x5908b2f8)
#define MARSHAL_MAGIC_INSTANCE     ((u32)0xd89bf8b4)

//-----------------------------------------------------------------
struct ScriptFuncReg {
    const char* funcname;
    const char* debugname;
    SQRESULT (*funcptr)(HSQUIRRELVM);
};

struct ScriptConstReg {
    const char* name;
    SQInteger value;
};

//-----------------------------------------------------------------
// globals

// squirrel vm
static HSQUIRRELVM g_VM = 0;
static std::string g_LastCompileError;
static std::string g_LastRuntimeError;

// holds all scripts which have been loaded through RequireScript
static std::vector<std::string> g_ScriptRegistry;

// strong references to each of sphere's built-in classes
static HSQOBJECT g_StreamClass;
static HSQOBJECT g_FileClass;
static HSQOBJECT g_BlobClass;
static HSQOBJECT g_CanvasClass;
static HSQOBJECT g_RectClass;
static HSQOBJECT g_Vec2Class;
static HSQOBJECT g_TextureClass;
static HSQOBJECT g_SoundClass;
static HSQOBJECT g_SoundEffectClass;
static HSQOBJECT g_ZStreamClass;

//-----------------------------------------------------------------
SQRESULT ThrowError(const char* format, ...)
{
    static ArrayPtr<char> s_Buffer;
    static int s_BufferSize = 0;
    if (s_BufferSize == 0) { // one-time initialization
        try {
            s_Buffer.reset(new char[512]);
            s_BufferSize = 512;
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return SQ_ERROR;
        }
    }

    // format error message
    va_list arglist;
    va_start(arglist, format);
    int size = vsnprintf(s_Buffer.get(), s_BufferSize, format, arglist);
#ifdef _MSC_VER // VC's vsnprintf has different behavior than POSIX's vsnprintf
    while (size < 0 || size == s_BufferSize) { // buffer was not big enough to hold the output string + terminating null character
        try {
            s_Buffer.reset(new char[s_BufferSize * 2]); // double buffer size until vsnprintf succeeds
            s_BufferSize = s_BufferSize * 2;
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return SQ_ERROR;
        }
        va_start(arglist, format);
        size = vsnprintf(s_Buffer.get(), s_BufferSize, format, arglist);
    }
#else
    if (size < 0) { // formatting error occurred
        return;
    } else if (size >= s_BufferSize) { // buffer was not big enough to hold the output string + terminating null character
        try {
            s_Buffer.reset(new char[size + 1]); // allocate a big enough buffer and try again
            s_BufferSize = size + 1;
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return;
        }
        va_start(arglist, format);
        vsnprintf(s_Buffer.get(), s_BufferSize, format, arglist);
    }
#endif
    va_end(arglist);

    // get call information
    // funcname: name of the throwing function (the topmost function on squirrel's call stack)
    // source: name of the script from where the function was called (directly or indirectly)
    // line: the line in the source
    std::string funcname = "N/A";
    std::string source = "N/A";
    int line = -1;
    SQStackInfos si;
    if (SQ_SUCCEEDED(sq_stackinfos(g_VM, 0, &si))) {
        funcname = si.funcname;
        for (int i = 1; SQ_SUCCEEDED(sq_stackinfos(g_VM, i, &si)); i++) {
            if (si.line != -1) {
                source = si.source;
                line   = si.line;
                break;
            }
        }
    }

    // build final error message
    std::ostringstream oss;
    oss << "Error in '" << source;
    oss << "' in line " << line;
    oss << " while executing " << funcname;
    oss << ": " << (const char*)s_Buffer.get();

    // pass error message to squirrel triggering an exception
    sq_pushstring(g_VM, oss.str().c_str(), -1);
    return sq_throwobject(g_VM);
}

/******************************************************************
 *                                                                *
 *                             CORE                               *
 *                                                                *
 ******************************************************************/

/************************* RECT OBJECT ****************************/

#define SETUP_RECT_OBJECT() \
    Recti* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) { \
        THROW_ERROR("Invalid type of environment object, expected a Rect instance") \
    }

//-----------------------------------------------------------------
// Rect(x, y, width, height)
static SQInteger script_rect_constructor(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(4)
    GET_ARG_INT(1, x)
    GET_ARG_INT(2, y)
    GET_ARG_INT(3, width)
    GET_ARG_INT(4, height)
    new (This) Recti(x, y, x + width - 1, y + height - 1);
    RET_VOID()
}

//-----------------------------------------------------------------
// Rect.isValid()
static SQInteger script_rect_isValid(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    RET_BOOL(This->isValid())
}

//-----------------------------------------------------------------
// Rect.intersects(other)
static SQInteger script_rect_intersects(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_RECT(1, other)
    RET_BOOL(This->intersects(*other))
}

//-----------------------------------------------------------------
// Rect.getIntersection(other)
static SQInteger script_rect_getIntersection(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_RECT(1, other)
    RET_RECT(This->getIntersection(*other))
}

//-----------------------------------------------------------------
// Rect.containsPoint(x, y)
static SQInteger script_rect_containsPoint(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_INT(1, x)
    GET_ARG_INT(2, y)
    RET_BOOL(This->contains(x, y))
}

//-----------------------------------------------------------------
// Rect.containsRect(rect)
static SQInteger script_rect_containsRect(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_RECT(1, rect)
    RET_BOOL(This->contains(*rect))
}

//-----------------------------------------------------------------
// Rect._get(index)
static SQInteger script_rect__get(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "x") == 0) {
        RET_INT(This->ul.x)
    } else if (strcmp(index, "y") == 0) {
        RET_INT(This->ul.y)
    } else if (strcmp(index, "width") == 0) {
        RET_INT(This->getWidth())
    } else if (strcmp(index, "height") == 0) {
        RET_INT(This->getHeight())
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
}

//-----------------------------------------------------------------
// Rect._set(index, value)
static SQInteger script_rect__set(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "x") == 0) {
        GET_ARG_INT(2, value)
        This->lr.x = value + This->getWidth() - 1;
        This->ul.x = value;
    } else if (strcmp(index, "y") == 0) {
        GET_ARG_INT(2, value)
        This->lr.y = value + This->getHeight() - 1;
        This->ul.y = value;
    } else if (strcmp(index, "width") == 0) {
        GET_ARG_INT(2, value)
        This->lr.x = This->ul.x + value - 1;
    } else if (strcmp(index, "height") == 0) {
        GET_ARG_INT(2, value)
        This->lr.y = This->ul.y + value - 1;
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Rect._typeof()
static SQInteger script_rect__typeof(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    RET_STRING("Rect")
}

//-----------------------------------------------------------------
// Rect._cloned(rect)
static SQInteger script_rect__cloned(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_RECT(1, original)
    new (This) Recti(*original);
    RET_VOID()
}

//-----------------------------------------------------------------
// Rect._tostring()
static SQInteger script_rect__tostring(HSQUIRRELVM v)
{
    SETUP_RECT_OBJECT()
    std::ostringstream oss;
    oss << "<Rect instance at " << This;
    oss << " (x = " << This->ul.x;
    oss << ", y =" << This->ul.y;
    oss << ", width = " << This->getWidth();
    oss << ", height = " << This->getHeight();
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
// Rect._dump(instance, stream)
static SQInteger script_rect__dump(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_RECT(1, instance)
    GET_ARG_STREAM(2, stream)

    if (!stream->isOpen() || !stream->isWriteable()) {
        THROW_ERROR("Invalid output stream")
    }

    // write class name
    const char* class_name = "Rect";
    int class_name_size = strlen(class_name);
    if (!writei32l(stream, (i32)class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write instance
    if (!writei32l(stream, instance->ul.x) ||
        !writei32l(stream, instance->ul.y) ||
        !writei32l(stream, instance->lr.x) ||
        !writei32l(stream, instance->lr.y))
    {
        goto throw_write_error;
    }

    RET_VOID()

throw_write_error:
    THROW_ERROR("Write error")
}

//-----------------------------------------------------------------
// Rect._load(stream)
static SQInteger script_rect__load(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)

    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid input stream")
    }

    // read instance
    Recti instance;
    if (!readi32l(stream, instance.ul.x) ||
        !readi32l(stream, instance.ul.y) ||
        !readi32l(stream, instance.lr.x) ||
        !readi32l(stream, instance.lr.y))
    {
        THROW_ERROR("Read error")
    }

    RET_RECT(instance)
}

//-----------------------------------------------------------------
void BindRect(const Recti& rect)
{
    assert(g_RectClass._type == OT_CLASS);
    sq_pushobject(g_VM, g_RectClass); // push rect class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop rect class
    SQUserPointer p = 0;
    sq_getinstanceup(g_VM, -1, &p, 0);
    assert(p);
    new (p) Recti(rect);
}

//-----------------------------------------------------------------
Recti* GetRect(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_RECT))) {
        return (Recti*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_rect_methods[] = {
    {"constructor",     "Rect.constructor",         script_rect_constructor     },
    {"isValid",         "Rect.isValid",             script_rect_isValid         },
    {"intersects",      "Rect.intersects",          script_rect_intersects      },
    {"getIntersection", "Rect.getIntersection",     script_rect_getIntersection },
    {"containsPoint",   "Rect.containsPoint",       script_rect_containsPoint   },
    {"containsRect",    "Rect.containsRect",        script_rect_containsRect    },
    {"_get",            "Rect._get",                script_rect__get            },
    {"_set",            "Rect._set",                script_rect__set            },
    {"_typeof",         "Rect._typeof",             script_rect__typeof         },
    {"_cloned",         "Rect._cloned",             script_rect__cloned         },
    {"_tostring",       "Rect._tostring",           script_rect__tostring       },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_rect_static_methods[] = {
    {"_dump",           "Rect._dump",               script_rect__dump           },
    {"_load",           "Rect._load",               script_rect__load           },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_rect_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_rect_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_RECT);
    sq_setclassudsize(g_VM, -1, sizeof(Recti));

    // define methods
    for (int i = 0; script_rect_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_rect_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_rect_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_rect_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_rect_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_rect_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_rect_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_rect_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_rect_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_rect_constants[i].name, -1);
        sq_pushinteger(g_VM, script_rect_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_RectClass);
    sq_getstackobj(g_VM, -1, &g_RectClass);
    sq_addref(g_VM, &g_RectClass);

    sq_poptop(g_VM); // pop class
}

/************************* VEC2 OBJECT ****************************/

#define SETUP_VEC2_OBJECT() \
    Vec2f* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) { \
        THROW_ERROR("Invalid type of environment object, expected a Vec2 instance") \
    }

//-----------------------------------------------------------------
// Vec2([x, y])
static SQInteger script_vec2_constructor(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    GET_OPTARG_FLOAT(1, x, 0)
    GET_OPTARG_FLOAT(2, y, 0)
    new (This) Vec2f(x, y);
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2.getLength()
static SQInteger script_vec2_getLength(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    RET_FLOAT((SQFloat)This->len())
}

//-----------------------------------------------------------------
// Vec2.getDotProduct(other)
static SQInteger script_vec2_getDotProduct(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, other)
    RET_FLOAT(This->dot(*other))
}

//-----------------------------------------------------------------
// Vec2.setNull()
static SQInteger script_vec2_setNull(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    This->null();
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2.normalize()
static SQInteger script_vec2_normalize(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    This->unit();
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2.getDistanceFrom(other)
static SQInteger script_vec2_getDistanceFrom(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, other)
    RET_FLOAT((SQFloat)This->distFrom(*other))
}

//-----------------------------------------------------------------
// Vec2.rotateBy(degrees [, center])
static SQInteger script_vec2_rotateBy(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_MIN_NARGS(1)
    GET_ARG_FLOAT(1, degrees)
    GET_OPTARG_VEC2(2, center)
    if (center) {
        This->rotateBy(degrees, *center);
    } else {
        This->rotateBy(degrees);
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2.getAngle()
static SQInteger script_vec2_getAngle(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    RET_FLOAT((SQFloat)This->getAngle())
}

//-----------------------------------------------------------------
// Vec2.getAngleWith(other)
static SQInteger script_vec2_getAngleWith(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, other)
    RET_FLOAT((SQFloat)This->getAngleWith(*other))
}

//-----------------------------------------------------------------
// Vec2.isBetween(a, b)
static SQInteger script_vec2_isBetween(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_VEC2(1, a)
    GET_ARG_VEC2(2, b)
    RET_BOOL(This->isBetween(*a, *b))
}

//-----------------------------------------------------------------
// Vec2.getInterpolated(other, d)
static SQInteger script_vec2_getInterpolated(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_VEC2(1, other)
    GET_ARG_FLOAT(2, d)
    RET_VEC2(This->getInterpolated(*other, d))
}

//-----------------------------------------------------------------
// Vec2.interpolate(a, b, d)
static SQInteger script_vec2_interpolate(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(3)
    GET_ARG_VEC2(1, a)
    GET_ARG_VEC2(2, b)
    GET_ARG_FLOAT(3, d)
    This->interpolate(*a, *b, d);
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2.add(rhs)
static SQInteger script_vec2_add(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, rhs)
    *This += *rhs;
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2.subtract(rhs)
static SQInteger script_vec2_subtract(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, rhs)
    *This -= *rhs;
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2.multiply(scalar)
static SQInteger script_vec2_multiply(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_FLOAT(1, scalar)
    *This *= scalar;
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2._add(rhs)
static SQInteger script_vec2__add(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, rhs)
    RET_VEC2(*This + *rhs)
}

//-----------------------------------------------------------------
// Vec2._sub(rhs)
static SQInteger script_vec2__sub(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, rhs)
    RET_VEC2(*This - *rhs)
}

//-----------------------------------------------------------------
// Vec2._mul(scalar)
static SQInteger script_vec2__mul(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_FLOAT(1, scalar)
    RET_VEC2(*This * scalar)
}

//-----------------------------------------------------------------
// Vec2._get(index)
static SQInteger script_vec2__get(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "x") == 0) {
        RET_FLOAT(This->x)
    } else if (strcmp(index, "y") == 0) {
        RET_FLOAT(This->y)
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
}

//-----------------------------------------------------------------
// Vec2._set(index, value)
static SQInteger script_vec2__set(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "x") == 0) {
        GET_ARG_FLOAT(2, value)
        This->x = value;
    } else if (strcmp(index, "y") == 0) {
        GET_ARG_FLOAT(2, value)
        This->y = value;
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2._typeof()
static SQInteger script_vec2__typeof(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    RET_STRING("Vec2")
}

//-----------------------------------------------------------------
// Vec2._cloned(original)
static SQInteger script_vec2__cloned(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_VEC2(1, original)
    new (This) Vec2f(*original);
    RET_VOID()
}

//-----------------------------------------------------------------
// Vec2._tostring()
static SQInteger script_vec2__tostring(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    std::ostringstream oss;
    oss << "<Vec2 instance at " << This;
    oss << " (x = " << This->x;
    oss << ", y = " << This->y;
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
// Vec2._dump(instance, stream)
static SQInteger script_vec2__dump(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_VEC2(1, instance)
    GET_ARG_STREAM(2, stream)

    if (!stream->isOpen() || !stream->isWriteable()) {
        THROW_ERROR("Invalid output stream")
    }

    // write class name
    const char* class_name = "Vec2";
    int class_name_size = strlen(class_name);
    if (!writei32l(stream, (i32)class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write instance
    if (!writef32l(stream, instance->x) ||
        !writef32l(stream, instance->y))
    {
        goto throw_write_error;
    }

    RET_VOID()

throw_write_error:
    THROW_ERROR("Write error")
}

//-----------------------------------------------------------------
// Vec2._load(stream)
static SQInteger script_vec2__load(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)

    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid input stream")
    }

    // read instance
    Vec2f instance;
    if (!readf32l(stream, instance.x) ||
        !readf32l(stream, instance.y))
    {
        THROW_ERROR("Read error")
    }

    RET_VEC2(instance)
}

//-----------------------------------------------------------------
void BindVec2(const Vec2f& vec)
{
    assert(g_Vec2Class._type == OT_CLASS);
    sq_pushobject(g_VM, g_Vec2Class); // push vec2 class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop vec2 class
    SQUserPointer p = 0;
    sq_getinstanceup(g_VM, -1, &p, 0);
    assert(p);
    new (p) Vec2f(vec);
}

//-----------------------------------------------------------------
Vec2f* GetVec2(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_VEC2))) {
        return (Vec2f*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_vec2_methods[] = {
    {"constructor",     "Vec2.constructor",     script_vec2_constructor     },
    {"getLength",       "Vec2.getLength",       script_vec2_getLength       },
    {"getDotProduct",   "Vec2.getDotProduct",   script_vec2_getDotProduct   },
    {"setNull",         "Vec2.setNull",         script_vec2_setNull         },
    {"normalize",       "Vec2.normalize",       script_vec2_normalize       },
    {"getDistanceFrom", "Vec2.getDistanceFrom", script_vec2_getDistanceFrom },
    {"rotateBy",        "Vec2.rotateBy",        script_vec2_rotateBy        },
    {"getAngle",        "Vec2.getAngle",        script_vec2_getAngle        },
    {"getAngleWith",    "Vec2.getAngleWith",    script_vec2_getAngleWith    },
    {"isBetween",       "Vec2.isBetween",       script_vec2_isBetween       },
    {"getInterpolated", "Vec2.getInterpolated", script_vec2_getInterpolated },
    {"interpolate",     "Vec2.interpolate",     script_vec2_interpolate     },
    {"add",             "Vec2.add",             script_vec2_add             },
    {"subtract",        "Vec2.subtract",        script_vec2_subtract        },
    {"multiply",        "Vec2.multiply",        script_vec2_multiply        },
    {"_add",            "Vec2._add",            script_vec2__add            },
    {"_sub",            "Vec2._sub",            script_vec2__sub            },
    {"_mul",            "Vec2._mul",            script_vec2__mul            },
    {"_get",            "Vec2._get",            script_vec2__get            },
    {"_set",            "Vec2._set",            script_vec2__set            },
    {"_typeof",         "Vec2._typeof",         script_vec2__typeof         },
    {"_cloned",         "Vec2._cloned",         script_vec2__cloned         },
    {"_tostring",       "Vec2._tostring",       script_vec2__tostring       },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_vec2_static_methods[] = {
    {"_dump",           "Vec2._dump",           script_vec2__dump           },
    {"_load",           "Vec2._load",           script_vec2__load           },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_vec2_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_vec2_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_VEC2);
    sq_setclassudsize(g_VM, -1, sizeof(Vec2f));

    // define methods
    for (int i = 0; script_vec2_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_vec2_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_vec2_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_vec2_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_vec2_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_vec2_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_vec2_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_vec2_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_vec2_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_vec2_constants[i].name, -1);
        sq_pushinteger(g_VM, script_vec2_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_Vec2Class);
    sq_getstackobj(g_VM, -1, &g_Vec2Class);
    sq_addref(g_VM, &g_Vec2Class);

    sq_poptop(g_VM); // pop class
}

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
static ScriptFuncReg script_core_functions[] = {
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_core_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void register_core_api()
{
    // init classes
    init_rect_class();
    init_vec2_class();

    // register classes
    sq_pushstring(g_VM, "Rect", -1);
    sq_pushobject(g_VM, g_RectClass);
    sq_newslot(g_VM, -3, SQFalse);

    sq_pushstring(g_VM, "Vec2", -1);
    sq_pushobject(g_VM, g_Vec2Class);
    sq_newslot(g_VM, -3, SQFalse);

    // register functions
    for (int i = 0; script_core_functions[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_core_functions[i].funcname, -1);
        sq_newclosure(g_VM, script_core_functions[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_core_functions[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // register constants
    for (int i = 0; script_core_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_core_constants[i].name, -1);
        sq_pushinteger(g_VM, script_core_constants[i].value);
        sq_newslot(g_VM, -3, SQFalse);
    }
}

/******************************************************************
 *                                                                *
 *                               IO                               *
 *                                                                *
 ******************************************************************/

/************************* STREAM INTERFACE ***********************/

#define SETUP_STREAM_OBJECT() \
    IStream* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) { \
        THROW_ERROR("Invalid type of environment object, expected a Stream instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_stream_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((IStream*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Stream.isOpen()
static SQInteger script_stream_isOpen(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    RET_BOOL(This->isOpen())
}

//-----------------------------------------------------------------
// Stream.isReadable()
static SQInteger script_stream_isReadable(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    RET_BOOL(This->isReadable())
}

//-----------------------------------------------------------------
// Stream.isWriteable()
static SQInteger script_stream_isWriteable(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    RET_BOOL(This->isWriteable())
}

//-----------------------------------------------------------------
// Stream.close()
static SQInteger script_stream_close(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    This->close();
    RET_VOID()
}

//-----------------------------------------------------------------
// Stream.tell()
static SQInteger script_stream_tell(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    RET_INT(This->tell())
}

//-----------------------------------------------------------------
// Stream.seek(offset [, origin = Stream.BEG])
static SQInteger script_stream_seek(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    CHECK_MIN_NARGS(1)
    GET_ARG_INT(1, offset)
    GET_OPTARG_INT(2, origin, IStream::BEG)
    RET_BOOL(This->seek(offset, origin))
}

//-----------------------------------------------------------------
// Stream.read(size)
static SQInteger script_stream_read(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_INT(1, size)
    if (size < 0) {
        THROW_ERROR("Invalid size")
    }
    BlobPtr blob = Blob::Create(size);
    if (size == 0) {
        RET_BLOB(blob.get())
    }
    blob->resize(size);
    if (This->read(blob->getBuffer(), size) != size) {
        THROW_ERROR("Read error")
    }
    RET_BLOB(blob.get())
}

//-----------------------------------------------------------------
// Stream.write(blob [, offset = 0, count = -1])
static SQInteger script_stream_write(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    CHECK_MIN_NARGS(1)
    GET_ARG_BLOB(1, blob)
    GET_OPTARG_INT(2, offset, 0)
    GET_OPTARG_INT(3, count, -1)
    if (blob->getSize() > 0 && count != 0) {
        if (offset < 0 || offset >= blob->getSize()) {
            THROW_ERROR("Invalid offset")
        }
        if (count < 0) {
            count = blob->getSize() - offset;
        } else if (offset + count > blob->getSize()) {
            THROW_ERROR("Invalid count")
        }
        if (This->write(blob->getBuffer() + offset, count) != count) {
            THROW_ERROR("Write error")
        }
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Stream.flush()
static SQInteger script_stream_flush(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    RET_BOOL(This->flush())
}

//-----------------------------------------------------------------
// Stream.eof()
static SQInteger script_stream_eof(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    RET_BOOL(This->eof())
}

//-----------------------------------------------------------------
// Stream.readNumber(type [, endian = 'l'])
// type encoding:
// 'c':  i8
// 'b':  u8
// 's': i16
// 'w': u16
// 'i': i32
// 'f': f32
// endian encoding:
// 'l': little endian
// 'b': big endian
// 'h': host endian
static SQInteger script_stream_readNumber(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    CHECK_MIN_NARGS(1)
    GET_ARG_INT(1, type)
    GET_OPTARG_INT(2, endian, 'l')
    switch (endian) {
    case 'l':
        switch (type) {
        case 'c': { i8 n;  if (  readi8(This, n)) { RET_INT(n)      } } break;
        case 'b': { i8 n;  if (  readi8(This, n)) { RET_INT((u8)n)  } } break;
        case 's': { i16 n; if (readi16l(This, n)) { RET_INT(n)      } } break;
        case 'w': { i16 n; if (readi16l(This, n)) { RET_INT((u16)n) } } break;
        case 'i': { i32 n; if (readi32l(This, n)) { RET_INT(n)      } } break;
        case 'f': { f32 n; if (readf32l(This, n)) { RET_FLOAT(n)    } } break;
        default:
            THROW_ERROR("Invalid type")
        }
        break;
    case 'b':
        switch (type) {
        case 'c': { i8 n;  if (  readi8(This, n)) { RET_INT(n)      } } break;
        case 'b': { i8 n;  if (  readi8(This, n)) { RET_INT((u8)n)  } } break;
        case 's': { i16 n; if (readi16b(This, n)) { RET_INT(n)      } } break;
        case 'w': { i16 n; if (readi16b(This, n)) { RET_INT((u16)n) } } break;
        case 'i': { i32 n; if (readi32b(This, n)) { RET_INT(n)      } } break;
        case 'f': { f32 n; if (readf32b(This, n)) { RET_FLOAT(n)    } } break;
        default:
            THROW_ERROR("Invalid type")
        }
        break;
    case 'h':
        switch (type) {
        case 'c': { i8 n;  if (This->read(&n, sizeof(n)) == sizeof(n)) { RET_INT(n)   } } break;
        case 'b': { u8 n;  if (This->read(&n, sizeof(n)) == sizeof(n)) { RET_INT(n)   } } break;
        case 's': { i16 n; if (This->read(&n, sizeof(n)) == sizeof(n)) { RET_INT(n)   } } break;
        case 'w': { u16 n; if (This->read(&n, sizeof(n)) == sizeof(n)) { RET_INT(n)   } } break;
        case 'i': { i32 n; if (This->read(&n, sizeof(n)) == sizeof(n)) { RET_INT(n)   } } break;
        case 'f': { f32 n; if (This->read(&n, sizeof(n)) == sizeof(n)) { RET_FLOAT(n) } } break;
        default:
            THROW_ERROR("Invalid type")
        }
        break;
    default:
        THROW_ERROR("Invalid endian")
    }
    THROW_ERROR("Read error")
}

//-----------------------------------------------------------------
// Stream.readString(length)
static SQInteger script_stream_readString(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_INT(1, length)
    if (length < 0) {
        THROW_ERROR("Invalid length")
    }
    if (length == 0) {
        RET_STRING("")
    }
    try {
        ArrayPtr<u8> buf = new u8[length]; // assuming sizeof(SQChar) == 1
        if (This->read(buf.get(), length) != length) {
            THROW_ERROR("Read error")
        }
        RET_STRING_N((const SQChar*)buf.get(), length)
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        RET_NULL()
    }
}

//-----------------------------------------------------------------
// Stream.writeNumber(type, number [, endian = 'l'])
static SQInteger script_stream_writeNumber(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    CHECK_MIN_NARGS(2)
    GET_ARG_INT(1, type)
    if (ARG_IS_INT(2)) {
        GET_ARG_INT(2, number)
        GET_OPTARG_INT(3, endian, 'l')
        switch (endian) {
        case 'l':
            switch (type) {
            case 'c': { if (  writei8(This,  (i8)number)) { RET_VOID() } } break;
            case 'b': { if (  writei8(This,  (u8)number)) { RET_VOID() } } break;
            case 's': { if (writei16l(This, (i16)number)) { RET_VOID() } } break;
            case 'w': { if (writei16l(This, (u16)number)) { RET_VOID() } } break;
            case 'i': { if (writei32l(This, (i32)number)) { RET_VOID() } } break;
            default:
                THROW_ERROR("Invalid type")
            }
        case 'b':
            switch (type) {
            case 'c': { if (  writei8(This,  (i8)number)) { RET_VOID() } } break;
            case 'b': { if (  writei8(This,  (u8)number)) { RET_VOID() } } break;
            case 's': { if (writei16b(This, (i16)number)) { RET_VOID() } } break;
            case 'w': { if (writei16b(This, (u16)number)) { RET_VOID() } } break;
            case 'i': { if (writei32b(This, (i32)number)) { RET_VOID() } } break;
            default:
                THROW_ERROR("Invalid type")
            }
        case 'h':
            switch (type) {
            case 'c': {  i8 n =  (i8)number; if (This->write(&n, sizeof(n)) == sizeof(n)) { RET_VOID() } } break;
            case 'b': {  u8 n =  (u8)number; if (This->write(&n, sizeof(n)) == sizeof(n)) { RET_VOID() } } break;
            case 's': { i16 n = (i16)number; if (This->write(&n, sizeof(n)) == sizeof(n)) { RET_VOID() } } break;
            case 'w': { u16 n = (u16)number; if (This->write(&n, sizeof(n)) == sizeof(n)) { RET_VOID() } } break;
            case 'i': { i32 n = (i32)number; if (This->write(&n, sizeof(n)) == sizeof(n)) { RET_VOID() } } break;
            default:
                THROW_ERROR("Invalid type")
            }
        default:
            THROW_ERROR("Invalid endian")
        }
    } else {
        GET_ARG_FLOAT(2, number)
        GET_OPTARG_INT(3, endian, 'l')
        switch (endian) {
        case 'l': { if (writef32l(This, (f32)number)) { RET_VOID() } } break;
        case 'b': { if (writef32b(This, (f32)number)) { RET_VOID() } } break;
        case 'h': { f32 n = (f32)number; if (This->write(&n, sizeof(n)) == sizeof(n)) { RET_VOID() } } break;
        default:
            THROW_ERROR("Invalid endian")
        }
    }
    THROW_ERROR("Write error")
}

//-----------------------------------------------------------------
// Stream.writeString(str)
static SQInteger script_stream_writeString(HSQUIRRELVM v)
{
    SETUP_STREAM_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STRING(1, str)
    int len = sq_getsize(v, 2);
    if (len > 0) {
        if (This->write(str, len) != len) {
            THROW_ERROR("Write error")
        }
    }
    RET_VOID()
}

//-----------------------------------------------------------------
void CreateStreamDerivedClass()
{
    sq_pushobject(g_VM, g_StreamClass);
    bool succeeded = SQ_SUCCEEDED(sq_newclass(g_VM, SQTrue));
    assert(succeeded);
}

//-----------------------------------------------------------------
void BindStream(IStream* stream)
{
    assert(g_StreamClass._type == OT_CLASS);
    assert(stream);
    sq_pushobject(g_VM, g_StreamClass); // push stream class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop stream class
    sq_setreleasehook(g_VM, -1, script_stream_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)stream);
    stream->grab(); // grab a new reference
}

//-----------------------------------------------------------------
IStream* GetStream(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_STREAM))) {
        return (IStream*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_stream_methods[] = {
    {"isOpen",      "Stream.isOpen",        script_stream_isOpen        },
    {"isReadable",  "Stream.isReadable",    script_stream_isReadable    },
    {"isWriteable", "Stream.isWriteable",   script_stream_isWriteable   },
    {"close",       "Stream.close",         script_stream_close         },
    {"tell",        "Stream.tell",          script_stream_tell          },
    {"seek",        "Stream.seek",          script_stream_seek          },
    {"read",        "Stream.read",          script_stream_read          },
    {"write",       "Stream.write",         script_stream_write         },
    {"flush",       "Stream.flush",         script_stream_flush         },
    {"eof",         "Stream.eof",           script_stream_eof           },
    {"readNumber",  "Stream.readNumber",    script_stream_readNumber    },
    {"readString",  "Stream.readString",    script_stream_readString    },
    {"writeNumber", "Stream.writeNumber",   script_stream_writeNumber   },
    {"writeString", "Stream.writeString",   script_stream_writeString   },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_stream_static_methods[] = {
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_stream_constants[] = {
    {"BEG",     IStream::BEG    },
    {"CUR",     IStream::CUR    },
    {"END",     IStream::END    },
    {0,0}
};

//-----------------------------------------------------------------
static void init_stream_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_STREAM);

    // define methods
    for (int i = 0; script_stream_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_stream_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_stream_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_stream_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_stream_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_stream_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_stream_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_stream_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_stream_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_stream_constants[i].name, -1);
        sq_pushinteger(g_VM, script_stream_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_StreamClass);
    sq_getstackobj(g_VM, -1, &g_StreamClass);
    sq_addref(g_VM, &g_StreamClass);

    sq_poptop(g_VM); // pop class
}

/***************************** BLOB *******************************/

#define SETUP_BLOB_OBJECT() \
    Blob* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) { \
        THROW_ERROR("Invalid type of environment object, expected a Blob instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_blob_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((Blob*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Blob([size = 0, value = 0])
static SQInteger script_blob_constructor(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    GET_OPTARG_INT(1, size, 0)
    GET_OPTARG_INT(2, value, -1)
    if (size < 0) {
        THROW_ERROR("Invalid size")
    }
    This = Blob::Create(size);
    if (value >= 0) {
        This->reset(value);
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_blob_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.FromString(string)
static SQInteger script_blob_FromString(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, str)
    BlobPtr blob = Blob::Create();
    int len = sq_getsize(v, 2);
    if (len > 0) {
        blob->assign(str, len);
    }
    RET_BLOB(blob.get())
}

//-----------------------------------------------------------------
// Blob.getSize()
static SQInteger script_blob_getSize(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    RET_INT(This->getSize())
}

//-----------------------------------------------------------------
// Blob.getCapacity()
static SQInteger script_blob_getCapacity(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    RET_INT(This->getCapacity())
}

//-----------------------------------------------------------------
// Blob.clear()
static SQInteger script_blob_clear(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    This->clear();
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.reset([value])
static SQInteger script_blob_reset(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    GET_OPTARG_INT(1, value, 0)
    This->reset((u8)value);
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.resize(size)
static SQInteger script_blob_resize(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_INT(1, size)
    if (size < 0) {
        THROW_ERROR("Invalid size")
    }
    This->resize(size);
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.bloat()
static SQInteger script_blob_bloat(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    This->bloat();
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.reserve(size)
static SQInteger script_blob_reserve(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_INT(1, size)
    if (size < 0) {
        THROW_ERROR("Invalid size")
    }
    This->reserve(size);
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.doubleCapacity()
static SQInteger script_blob_doubleCapacity(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    This->doubleCapacity();
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.assign(blob)
static SQInteger script_blob_assign(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_BLOB(1, blob)
    if (blob->getSize() > 0) {
        This->assign(blob->getBuffer(), blob->getSize());
    } else {
        This->clear();
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.append(blob)
static SQInteger script_blob_append(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_BLOB(1, blob)
    if (blob->getSize() > 0) {
        This->append(blob->getBuffer(), blob->getSize());
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.concat(blob)
static SQInteger script_blob_concat(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_BLOB(1, blob)
    BlobPtr result = This->concat(blob->getBuffer(), blob->getSize());
    RET_BLOB(result.get())
}

//-----------------------------------------------------------------
// Blob.swap2()
static SQInteger script_blob_swap2(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    This->swap2();
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.swap4()
static SQInteger script_blob_swap4(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    This->swap4();
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.swap8()
static SQInteger script_blob_swap8(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    This->swap8();
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.toString()
static SQInteger script_blob_toString(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    RET_STRING_N((const SQChar*)This->getBuffer(), This->getSize())
}

//-----------------------------------------------------------------
// Blob._add(blob)
static SQInteger script_blob__add(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_BLOB(1, blob)
    BlobPtr new_blob = This->concat(blob->getBuffer(), blob->getSize());
    RET_BLOB(new_blob.get())
}

//-----------------------------------------------------------------
// Blob._get(index)
static SQInteger script_blob__get(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    if (ARG_IS_INT(1)) {
        GET_ARG_INT(1, index)
        if (index < 0 || index >= This->getSize()) {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        RET_INT(This->at(index))
    } else {
        GET_ARG_STRING(1, index)
        if (strcmp(index, "size") == 0) {
            RET_INT(This->getSize())
        } else if (strcmp(index, "capacity") == 0) {
            RET_INT(This->getCapacity())
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
    }
}

//-----------------------------------------------------------------
// Blob._set(index, value)
static SQInteger script_blob__set(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(2)
    if (ARG_IS_INT(1)) {
        GET_ARG_INT(1, index)
        if (index < 0 || index >= This->getSize()) {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        GET_ARG_INT(2, value)
        This->at(index) = (u8)value;
    } else {
        GET_ARG_STRING(1, index)
        if (strcmp(index, "size") == 0) {
            GET_ARG_INT(2, value)
            if (value < 0) { // negative size
                THROW_ERROR("Invalid size");
            }
            This->resize(value);
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob._typeof()
static SQInteger script_blob__typeof(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    RET_STRING("Blob")
}

//-----------------------------------------------------------------
// Blob._tostring()
static SQInteger script_blob__tostring(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    std::ostringstream oss;
    oss << "<Blob instance at " << This;
    oss << " (size = " << This->getSize();
    oss << ", capacity = " << This->getCapacity();
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
// Blob._nexti(index)
static SQInteger script_blob__nexti(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    if (ARG_IS_NULL(1)) { // start of iteration
        RET_INT(0) // return index 0
    } else {
        GET_ARG_INT(1, prev_idx)
        int next_idx = prev_idx + 1;
        if (next_idx >= 0 && next_idx < This->getSize()) {
            RET_INT(next_idx) // return next index
        } else {
            RET_NULL() // end of iteration
        }
    }
}

//-----------------------------------------------------------------
// Blob._cloned(original)
static SQInteger script_blob__cloned(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_BLOB(1, original)
    This = Blob::Create(original->getSize());
    if (original->getSize() > 0) {
        memcpy(This->getBuffer(), original->getBuffer(), original->getSize());
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_blob_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob._dump(instance, stream)
static SQInteger script_blob__dump(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_BLOB(1, instance)
    GET_ARG_STREAM(2, stream)

    if (!stream->isOpen() || !stream->isWriteable()) {
        THROW_ERROR("Invalid output stream")
    }

    // write class name
    const char* class_name = "Blob";
    int num_bytes = strlen(class_name);
    if (!writei32l(stream, (i32)num_bytes) || stream->write(class_name, num_bytes) != num_bytes) {
        goto throw_write_error;
    }

    // write blob size
    if (!writei32l(stream, (i32)instance->getSize())) {
        goto throw_write_error;
    }

    // write blob data
    if (instance->getSize() > 0 && stream->write(instance->getBuffer(), instance->getSize()) != instance->getSize()) {
        goto throw_write_error;
    }

    RET_VOID()

throw_write_error:
    THROW_ERROR("Write error")
}

//-----------------------------------------------------------------
// Blob._load(stream)
static SQInteger script_blob__load(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)

    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid input stream")
    }

    // read blob size
    i32 blob_size;
    if (!readi32l(stream, blob_size)) {
        THROW_ERROR("Read error")
    }
    if (blob_size < 0) {
        THROW_ERROR("Invalid size")
    }

    // read blob data
    BlobPtr instance = Blob::Create(blob_size);
    if (stream->read(instance->getBuffer(), blob_size) != blob_size) {
        THROW_ERROR("Read error")
    }

    RET_BLOB(instance.get())
}

//-----------------------------------------------------------------
void BindBlob(Blob* blob)
{
    assert(g_BlobClass._type == OT_CLASS);
    assert(blob);
    sq_pushobject(g_VM, g_BlobClass); // push blob class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop blob class
    sq_setreleasehook(g_VM, -1, script_blob_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)blob);
    blob->grab(); // grab a new reference
}

//-----------------------------------------------------------------
Blob* GetBlob(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_BLOB))) {
        return (Blob*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_blob_methods[] = {
    {"constructor",     "Blob.constructor",     script_blob_constructor     },
    {"getSize",         "Blob.getSize",         script_blob_getSize         },
    {"getCapacity",     "Blob.getCapacity",     script_blob_getCapacity     },
    {"clear",           "Blob.clear",           script_blob_clear           },
    {"reset",           "Blob.reset",           script_blob_reset           },
    {"resize",          "Blob.resize",          script_blob_resize          },
    {"bloat",           "Blob.bloat",           script_blob_bloat           },
    {"reserve",         "Blob.reserve",         script_blob_reserve         },
    {"doubleCapacity",  "Blob.doubleCapacity",  script_blob_doubleCapacity  },
    {"assign",          "Blob.assign",          script_blob_assign          },
    {"append",          "Blob.append",          script_blob_append          },
    {"concat",          "Blob.concat",          script_blob_concat          },
    {"swap2",           "Blob.swap2",           script_blob_swap2           },
    {"swap4",           "Blob.swap4",           script_blob_swap4           },
    {"swap8",           "Blob.swap8",           script_blob_swap8           },
    {"toString",        "Blob.toString",        script_blob_toString        },
    {"_add",            "Blob._add",            script_blob__add            },
    {"_get",            "Blob._get",            script_blob__get            },
    {"_set",            "Blob._set",            script_blob__set            },
    {"_typeof",         "Blob._typeof",         script_blob__typeof         },
    {"_tostring",       "Blob._tostring",       script_blob__tostring       },
    {"_nexti",          "Blob._nexti",          script_blob__nexti          },
    {"_cloned",         "Blob._cloned",         script_blob__cloned         },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_blob_static_methods[] = {
    {"FromString",      "Blob.FromString",      script_blob_FromString      },
    {"_dump",           "Blob._dump",           script_blob__dump           },
    {"_load",           "Blob._load",           script_blob__load           },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_blob_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_blob_class()
{
    CreateStreamDerivedClass(); // inherit from Stream
    sq_newclass(g_VM, SQTrue); // create class
    sq_settypetag(g_VM, -1, TT_BLOB);

    // define methods
    for (int i = 0; script_blob_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_blob_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_blob_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_blob_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_blob_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_blob_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_blob_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_blob_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_blob_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_blob_constants[i].name, -1);
        sq_pushinteger(g_VM, script_blob_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_BlobClass);
    sq_getstackobj(g_VM, -1, &g_BlobClass);
    sq_addref(g_VM, &g_BlobClass);

    sq_poptop(g_VM); // pop class
}

/*************************** FILE OBJECT **************************/

#define SETUP_FILE_OBJECT() \
    IFile* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FILE))) { \
        THROW_ERROR("Invalid type of environment object, expected a File instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_file_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((IFile*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// File.Open(filename [, mode = File.IN])
static SQInteger script_file_Open(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STRING(1, filename)
    GET_OPTARG_INT(2, mode, IFile::IN)
    if (mode != IFile::IN  &&
        mode != IFile::OUT &&
        mode != IFile::APPEND)
    {
        THROW_ERROR("Invalid mode")
    }
    FilePtr file = OpenFile(filename, mode);
    if (!file) {
        THROW_ERROR1("Could not open file '%s'", filename)
    }
    RET_FILE(file.get())
}

//-----------------------------------------------------------------
// File.getName()
static SQInteger script_file_getName(HSQUIRRELVM v)
{
    SETUP_FILE_OBJECT()
    RET_STRING(This->getName().c_str())
}

//-----------------------------------------------------------------
// File._typeof()
static SQInteger script_file__typeof(HSQUIRRELVM v)
{
    SETUP_FILE_OBJECT()
    RET_STRING("File")
}

//-----------------------------------------------------------------
// File._tostring()
static SQInteger script_file__tostring(HSQUIRRELVM v)
{
    SETUP_FILE_OBJECT()
    std::ostringstream oss;
    oss << "<File instance at " << This;
    oss << " (filename = ";
    if (This->isOpen()) {
        oss << "\"" << This->getName() << "\"";
    } else {
        oss << "N/A";
    }
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
void CreateFileDerivedClass()
{
    sq_pushobject(g_VM, g_FileClass);
    bool succeeded = SQ_SUCCEEDED(sq_newclass(g_VM, SQTrue));
    assert(succeeded);
}

//-----------------------------------------------------------------
void BindFile(IFile* file)
{
    assert(g_FileClass._type == OT_CLASS);
    assert(file);
    sq_pushobject(g_VM, g_FileClass); // push file class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop file class
    sq_setreleasehook(g_VM, -1, script_file_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)file);
    file->grab(); // grab a new reference
}

//-----------------------------------------------------------------
IFile* GetFile(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_FILE))) {
        return (IFile*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_file_methods[] = {
    {"getName",     "File.getName",     script_file_getName     },
    {"_typeof",     "File._typeof",     script_file__typeof     },
    {"_tostring",   "File._tostring",   script_file__tostring   },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_file_static_methods[] = {
    {"Open",        "File.Open",        script_file_Open        },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_file_constants[] = {
    {"IN",      IFile::IN       },
    {"OUT",     IFile::OUT      },
    {"APPEND",  IFile::APPEND   },
    {0,0}
};

//-----------------------------------------------------------------
static void init_file_class()
{
    CreateStreamDerivedClass(); // inherit from Stream
    sq_newclass(g_VM, SQTrue); // create class
    sq_settypetag(g_VM, -1, TT_FILE);

    // define methods
    for (int i = 0; script_file_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_file_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_file_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_file_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_file_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_file_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_file_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_file_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_file_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_file_constants[i].name, -1);
        sq_pushinteger(g_VM, script_file_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_FileClass);
    sq_getstackobj(g_VM, -1, &g_FileClass);
    sq_addref(g_VM, &g_FileClass);

    sq_poptop(g_VM); // pop class
}

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
// DoesFileExist(filename)
static SQInteger script_DoesFileExist(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    RET_BOOL(DoesFileExist(filename))
}

//-----------------------------------------------------------------
// IsRegularFile(filename)
static SQInteger script_IsRegularFile(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    RET_BOOL(IsRegularFile(filename))
}

//-----------------------------------------------------------------
// IsDirectory(filename)
static SQInteger script_IsDirectory(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    RET_BOOL(IsDirectory(filename))
}

//-----------------------------------------------------------------
// GetFileModTime(filename)
static SQInteger script_GetFileModTime(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    RET_INT(GetFileModTime(filename))
}

//-----------------------------------------------------------------
// CreateDirectory(directory)
static SQInteger script_CreateDirectory(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, directory)
    if (!CreateDirectory(directory)) {
        THROW_ERROR("Could not create directory")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// RemoveFile(filename)
static SQInteger script_RemoveFile(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    if (!RemoveFile(filename)) {
        THROW_ERROR("Could not remove file or directory")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// RenameFile(filenameFrom, filenameTo)
static SQInteger script_RenameFile(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_STRING(1, filenameFrom)
    GET_ARG_STRING(2, filenameTo)
    if (!RenameFile(filenameFrom, filenameTo)) {
        THROW_ERROR("Could not rename file or directory")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// GetFileList(directory)
static SQInteger script_GetFileList(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, directory)
    std::vector<std::string> file_list;
    if (!GetFileList(directory, file_list)) {
        THROW_ERROR("Could not get file list")
    }
    sq_newarray(v, file_list.size());
    if (file_list.size() > 0) {
        for (int i = 0; i < (int)file_list.size(); ++i) {
            sq_pushinteger(v, i);
            sq_pushstring(v, file_list[i].c_str(), -1);
            sq_rawset(v, -3);
        }
    }
    return 1;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_io_functions[] = {
    {"DoesFileExist",   "DoesFileExist",    script_DoesFileExist    },
    {"IsRegularFile",   "IsRegularFile",    script_IsRegularFile    },
    {"IsDirectory",     "IsDirectory",      script_IsDirectory      },
    {"GetFileModTime",  "GetFileModTime",   script_GetFileModTime   },
    {"CreateDirectory", "CreateDirectory",  script_CreateDirectory  },
    {"RemoveFile",      "RemoveFile",       script_RemoveFile       },
    {"RenameFile",      "RenameFile",       script_RenameFile       },
    {"GetFileList",     "GetFileList",      script_GetFileList      },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_io_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void register_io_api()
{
    // init classes
    init_stream_class(); // Stream must be initialized before any class that inherits from it
    init_blob_class(); // requires Stream to be initialized first
    init_file_class(); // requires Stream to be initialized first

    // register classes
    sq_pushstring(g_VM, "Stream", -1);
    sq_pushobject(g_VM, g_StreamClass);
    sq_newslot(g_VM, -3, SQFalse);

    sq_pushstring(g_VM, "Blob", -1);
    sq_pushobject(g_VM, g_BlobClass);
    sq_newslot(g_VM, -3, SQFalse);

    sq_pushstring(g_VM, "File", -1);
    sq_pushobject(g_VM, g_FileClass);
    sq_newslot(g_VM, -3, SQFalse);

    // register functions
    for (int i = 0; script_io_functions[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_io_functions[i].funcname, -1);
        sq_newclosure(g_VM, script_io_functions[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_io_functions[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // register constants
    for (int i = 0; script_io_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_io_constants[i].name, -1);
        sq_pushinteger(g_VM, script_io_constants[i].value);
        sq_newslot(g_VM, -3, SQFalse);
    }
}

/******************************************************************
 *                                                                *
 *                           GRAPHICS                             *
 *                                                                *
 ******************************************************************/

/************************* CANVAS OBJECT **************************/

#define SETUP_CANVAS_OBJECT() \
    Canvas* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) { \
        THROW_ERROR("Invalid type of environment object, expected a Canvas instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_canvas_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((Canvas*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Canvas(width, height [, pixels])
static SQInteger script_canvas_constructor(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_MIN_NARGS(2)
    GET_ARG_INT(1, width)
    GET_ARG_INT(2, height)
    GET_OPTARG_BLOB(3, pixels)
    if (width <= 0) {
        THROW_ERROR("Invalid width")
    }
    if (height <= 0) {
        THROW_ERROR("Invalid height")
    }
    if (pixels && pixels->getSize() != width * height * Canvas::GetNumBytesPerPixel()) {
        THROW_ERROR("Invalid pixels")
    }
    This = Canvas::Create(width, height);
    if (pixels) {
        memcpy(This->getPixels(), pixels->getBuffer(), pixels->getSize());
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_canvas_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.Load(filename)
static SQInteger script_canvas_Load(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    FilePtr file = OpenFile(filename);
    if (!file) {
        THROW_ERROR("Could not open file")
    }
    CanvasPtr image = LoadImage(file.get());
    if (!image) {
        THROW_ERROR("Could not load image")
    }
    RET_CANVAS(image.get())
}

//-----------------------------------------------------------------
// Canvas.LoadFromStream(stream)
static SQInteger script_canvas_LoadFromStream(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)
    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid stream")
    }
    CanvasPtr image = LoadImage(stream);
    if (!image) {
        THROW_ERROR("Could not load image")
    }
    RET_CANVAS(image.get())
}

//-----------------------------------------------------------------
// Canvas.save(filename)
static SQInteger script_canvas_save(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    FilePtr file = OpenFile(filename, IFile::OUT);
    if (!file) {
        THROW_ERROR("Could not open file")
    }
    if (!SaveImage(This, file.get())) {
        THROW_ERROR("Could not save image")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.saveToStream(stream)
static SQInteger script_canvas_saveToStream(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)
    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid stream")
    }
    if (!SaveImage(This, stream)) {
        THROW_ERROR("Could not save image")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.getPixels()
static SQInteger script_canvas_getPixels(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    BlobPtr pixels = Blob::Create(This->getNumPixels() * Canvas::GetNumBytesPerPixel());
    memcpy(pixels->getBuffer(), This->getPixels(), pixels->getSize());
    RET_BLOB(pixels.get())
}

//-----------------------------------------------------------------
// Canvas.cloneSection(section)
static SQInteger script_canvas_cloneSection(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_RECT(1, section)
    CanvasPtr canvas = This->cloneSection(*section);
    if (!canvas) {
        THROW_ERROR("Could not clone section")
    }
    RET_CANVAS(canvas.get())
}

//-----------------------------------------------------------------
// Canvas.getPixel(x, y)
static SQInteger script_canvas_getPixel(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_INT(1, x)
    GET_ARG_INT(2, y)
    if (x < 0 || x >= This->getWidth()) {
        THROW_ERROR("Invalid x")
    }
    if (y < 0 || y >= This->getHeight()) {
        THROW_ERROR("Invalid y")
    }
    RET_INT(RGBA::Pack(This->getPixel(x, y)))
}

//-----------------------------------------------------------------
// Canvas.setPixel(x, y, color)
static SQInteger script_canvas_setPixel(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(3)
    GET_ARG_INT(1, x)
    GET_ARG_INT(2, y)
    GET_ARG_INT(3, color)
    if (x < 0 || x >= This->getWidth()) {
        THROW_ERROR("Invalid x")
    }
    if (y < 0 || y >= This->getHeight()) {
        THROW_ERROR("Invalid y")
    }
    This->setPixel(x, y, RGBA::Unpack((u32)color));
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.getPixelByIndex(index)
static SQInteger script_canvas_getPixelByIndex(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_INT(1, index)
    if (index < 0 || index >= This->getNumPixels()) {
        THROW_ERROR("Invalid index")
    }
    RET_INT(RGBA::Pack(This->getPixelByIndex(index)))
}

//-----------------------------------------------------------------
// Canvas.setPixelByIndex(index, color)
static SQInteger script_canvas_setPixelByIndex(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_INT(1, index)
    GET_ARG_INT(2, color)
    if (index < 0 || index >= This->getNumPixels()) {
        THROW_ERROR("Invalid index")
    }
    This->setPixelByIndex(index, RGBA::Unpack((u32)color));
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.resize(width, height)
static SQInteger script_canvas_resize(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_INT(1, width)
    GET_ARG_INT(2, height)
    if (width <= 0) {
        THROW_ERROR("Invalid width")
    }
    if (height <= 0) {
        THROW_ERROR("Invalid height")
    }
    This->resize(width, height);
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.fill(color)
static SQInteger script_canvas_fill(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    This->fill(RGBA::Unpack((u32)color));
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.flipHorizontally()
static SQInteger script_canvas_flipHorizontally(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    This->flipHorizontally();
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas.flipVertically()
static SQInteger script_canvas_flipVertically(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    This->flipVertically();
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas._get(index)
static SQInteger script_canvas__get(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    if (ARG_IS_INT(1)) {
        GET_ARG_INT(1, index)
        if (index < 0 || index >= This->getNumPixels()) {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        RET_INT(RGBA::Pack(This->getPixelByIndex(index)))
    } else {
        GET_ARG_STRING(1, index)
        if (strcmp(index, "width") == 0) {
            RET_INT(This->getWidth())
        } else if (strcmp(index, "height") == 0) {
            RET_INT(This->getHeight())
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
    }
}

//-----------------------------------------------------------------
// Canvas._set(index, value)
static SQInteger script_canvas__set(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_INT(1, index)
    GET_ARG_INT(2, value)
    if (index < 0 || index >= This->getNumPixels()) {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    This->setPixelByIndex(index, RGBA::Unpack((u32)value));
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas._typeof()
static SQInteger script_canvas__typeof(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    RET_STRING("Canvas")
}

//-----------------------------------------------------------------
// Canvas._tostring()
static SQInteger script_canvas__tostring(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    std::ostringstream oss;
    oss << "<Canvas instance at " << This;
    oss << " (width = " << This->getWidth();
    oss << ", height = " << This->getHeight();
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
// Canvas._nexti(index)
static SQInteger script_canvas__nexti(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    if (ARG_IS_NULL(1)) { // start of iteration
        RET_INT(0) // return index 0
    } else {
        GET_ARG_INT(1, prev_idx)
        int next_idx = prev_idx + 1;
        if (next_idx >= 0 && next_idx < This->getNumPixels()) {
            RET_INT(next_idx) // return next index
        } else {
            RET_NULL() // end of iteration
        }
    }
}

//-----------------------------------------------------------------
// Canvas._cloned(original)
static SQInteger script_canvas__cloned(HSQUIRRELVM v)
{
    SETUP_CANVAS_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_CANVAS(1, original)
    This = Canvas::Create(original->getWidth(), original->getHeight(), original->getPixels());
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_canvas_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// Canvas._dump(instance, stream)
static SQInteger script_canvas__dump(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_CANVAS(1, instance)
    GET_ARG_STREAM(2, stream)

    if (!stream->isOpen() || !stream->isWriteable()) {
        THROW_ERROR("Invalid output stream")
    }

    // write class name
    const char* class_name = "Canvas";
    int class_name_size = strlen(class_name);
    if (!writei32l(stream, (i32)class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write dimensions
    if (!writei32l(stream, (i32)instance->getWidth()) ||
        !writei32l(stream, (i32)instance->getHeight()))
    {
        goto throw_write_error;
    }

    // write pixels
    int pixels_size = instance->getNumPixels() * Canvas::GetNumBytesPerPixel();
    if (stream->write(instance->getPixels(), pixels_size) != pixels_size) {
        goto throw_write_error;
    }

    RET_VOID()

throw_write_error:
    THROW_ERROR("Write error")
}

//-----------------------------------------------------------------
// Canvas._load(stream)
static SQInteger script_canvas__load(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)

    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid input stream")
    }

    // read dimensions
    i32 width;
    i32 height;
    if (!readi32l(stream, width) ||
        !readi32l(stream, height))
    {
        THROW_ERROR("Read error")
    }
    if (width * height <= 0) {
        THROW_ERROR("Invalid dimensions")
    }

    CanvasPtr instance = Canvas::Create(width, height);

    // read pixels
    int pixels_size = width * height * Canvas::GetNumBytesPerPixel();
    if (stream->read(instance->getPixels(), pixels_size) != pixels_size) {
        THROW_ERROR("Read error")
    }

    RET_CANVAS(instance.get())
}

//-----------------------------------------------------------------
void BindCanvas(Canvas* canvas)
{
    assert(g_CanvasClass._type == OT_CLASS);
    assert(canvas);
    sq_pushobject(g_VM, g_CanvasClass); // push canvas class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop canvas class
    sq_setreleasehook(g_VM, -1, script_canvas_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)canvas);
    canvas->grab(); // grab a new reference
}

//-----------------------------------------------------------------
Canvas* GetCanvas(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_CANVAS))) {
        return (Canvas*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_canvas_methods[] = {
    {"constructor",         "Canvas.constructor",       script_canvas_constructor       },
    {"save",                "Canvas.save",              script_canvas_save              },
    {"saveToStream",        "Canvas.saveToStream",      script_canvas_saveToStream      },
    {"getPixels",           "Canvas.getPixels",         script_canvas_getPixels         },
    {"cloneSection",        "Canvas.cloneSection",      script_canvas_cloneSection      },
    {"getPixel",            "Canvas.getPixel",          script_canvas_getPixel          },
    {"setPixel",            "Canvas.setPixel",          script_canvas_setPixel          },
    {"getPixelByIndex",     "Canvas.getPixelByIndex",   script_canvas_getPixelByIndex   },
    {"setPixelByIndex",     "Canvas.setPixelByIndex",   script_canvas_setPixelByIndex   },
    {"resize",              "Canvas.resize",            script_canvas_resize            },
    {"fill",                "Canvas.fill",              script_canvas_fill              },
    {"flipHorizontally",    "Canvas.flipHorizontally",  script_canvas_flipHorizontally  },
    {"flipVertically",      "Canvas.flipVertically",    script_canvas_flipVertically    },
    {"_get",                "Canvas._get",              script_canvas__get              },
    {"_set",                "Canvas._set",              script_canvas__set              },
    {"_typeof",             "Canvas._typeof",           script_canvas__typeof           },
    {"_tostring",           "Canvas._tostring",         script_canvas__tostring         },
    {"_nexti",              "Canvas._nexti",            script_canvas__nexti            },
    {"_cloned",             "Canvas._cloned",           script_canvas__cloned           },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_canvas_static_methods[] = {
    {"Load",                "Canvas.Load",              script_canvas_Load              },
    {"LoadFromStream",      "Canvas.LoadFromStream",    script_canvas_LoadFromStream    },
    {"_dump",               "Canvas._dump",             script_canvas__dump             },
    {"_load",               "Canvas._load",             script_canvas__load             },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_canvas_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_canvas_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_CANVAS);

    // define methods
    for (int i = 0; script_canvas_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_canvas_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_canvas_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_canvas_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_canvas_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_canvas_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_canvas_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_canvas_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_canvas_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_canvas_constants[i].name, -1);
        sq_pushinteger(g_VM, script_canvas_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_CanvasClass);
    sq_getstackobj(g_VM, -1, &g_CanvasClass);
    sq_addref(g_VM, &g_CanvasClass);

    sq_poptop(g_VM); // pop class
}

/************************ TEXTURE OBJECT **************************/

#define SETUP_TEXTURE_OBJECT() \
    ITexture* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE))) { \
        THROW_ERROR("Invalid type of environment object, expected a Texture instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_texture_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ITexture*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Texture(canvas)
static SQInteger script_texture_constructor(HSQUIRRELVM v)
{
    SETUP_TEXTURE_OBJECT()
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_NARGS(1)
    GET_ARG_CANVAS(1, canvas)
    This = CreateTexture(canvas);
    if (!This) {
        THROW_ERROR("Could not create texture")
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_texture_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// Texture.Load(filename)
static SQInteger script_texture_Load(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, filename)
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    FilePtr file = OpenFile(filename);
    if (!file) {
        THROW_ERROR("Could not open file")
    }
    CanvasPtr image = LoadImage(file.get());
    if (!image) {
        THROW_ERROR("Could not load image")
    }
    TexturePtr texture = CreateTexture(image.get());
    if (!texture) {
        THROW_ERROR("Could not create texture")
    }
    RET_TEXTURE(texture.get())
}

//-----------------------------------------------------------------
// Texture.LoadFromStream(stream)
static SQInteger script_texture_LoadFromStream(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CanvasPtr image = LoadImage(stream);
    if (!image) {
        THROW_ERROR("Could not load image")
    }
    TexturePtr texture = CreateTexture(image.get());
    if (!texture) {
        THROW_ERROR("Could not create texture")
    }
    RET_TEXTURE(texture.get())
}

//-----------------------------------------------------------------
// Texture.updatePixels(new_pixels [, dst_rect])
static SQInteger script_texture_updatePixels(HSQUIRRELVM v)
{
    SETUP_TEXTURE_OBJECT()
    CHECK_MIN_NARGS(1)
    GET_ARG_CANVAS(1, new_pixels)
    GET_OPTARG_RECT(2, dst_rect)
    if (!This->updatePixels(new_pixels, dst_rect)) {
        THROW_ERROR("Could not update pixels")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Texture.createCanvas()
static SQInteger script_texture_createCanvas(HSQUIRRELVM v)
{
    SETUP_TEXTURE_OBJECT()
    CanvasPtr canvas = This->createCanvas();
    RET_CANVAS(canvas.get())
}

//-----------------------------------------------------------------
// Texture._get(index)
static SQInteger script_texture__get(HSQUIRRELVM v)
{
    SETUP_TEXTURE_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "width") == 0) {
        RET_INT(This->getWidth())
    } else if (strcmp(index, "height") == 0) {
        RET_INT(This->getHeight())
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
}

//-----------------------------------------------------------------
// Texture._typeof()
static SQInteger script_texture__typeof(HSQUIRRELVM v)
{
    SETUP_TEXTURE_OBJECT()
    RET_STRING("Texture")
}

//-----------------------------------------------------------------
// Texture._cloned(original)
static SQInteger script_texture__cloned(HSQUIRRELVM v)
{
    SETUP_TEXTURE_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_TEXTURE(1, original)
    CanvasPtr canvas = original->createCanvas();
    This = CreateTexture(canvas.get());
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_texture_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// Texture._tostring()
static SQInteger script_texture__tostring(HSQUIRRELVM v)
{
    SETUP_TEXTURE_OBJECT()
    std::ostringstream oss;
    oss << "<Texture instance at " << This;
    oss << " (" << This->getWidth();
    oss << ", " << This->getHeight();
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
// Texture._dump(instance, stream)
static SQInteger script_texture__dump(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_TEXTURE(1, instance)
    GET_ARG_STREAM(2, stream)

    if (!stream->isOpen() || !stream->isWriteable()) {
        THROW_ERROR("Invalid output stream")
    }

    // convert texture to canvas
    CanvasPtr canvas = instance->createCanvas();

    // write class name
    const char* class_name = "Texture";
    int num_bytes = strlen(class_name);
    if (!writei32l(stream, (i32)num_bytes) || stream->write(class_name, num_bytes) != num_bytes) {
        goto throw_write_error;
    }

    // write dimensions
    if (!writei32l(stream, (i32)canvas->getWidth()) ||
        !writei32l(stream, (i32)canvas->getHeight()))
    {
        goto throw_write_error;
    }

    // write pixels
    int pixels_size = canvas->getNumPixels() * Canvas::GetNumBytesPerPixel();
    if (stream->write(canvas->getPixels(), pixels_size) != pixels_size) {
        goto throw_write_error;
    }

    RET_VOID()

throw_write_error:
    THROW_ERROR("Write error")
}

//-----------------------------------------------------------------
// Texture._load(stream)
static SQInteger script_texture__load(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)

    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid input stream")
    }

    // read dimensions
    i32 width;
    i32 height;
    if (!readi32l(stream, width) ||
        !readi32l(stream, height))
    {
        THROW_ERROR("Read error")
    }
    if (width * height <= 0) {
        THROW_ERROR("Invalid dimensions")
    }

    // create a temporary canvas
    CanvasPtr canvas = Canvas::Create(width, height);

    // read pixels
    int pixels_size = canvas->getNumPixels() * Canvas::GetNumBytesPerPixel();
    if (stream->read(canvas->getPixels(), pixels_size) != pixels_size) {
        THROW_ERROR("Read error")
    }

    // create texture
    TexturePtr instance = CreateTexture(canvas.get());

    RET_TEXTURE(instance.get())
}

//-----------------------------------------------------------------
void BindTexture(ITexture* texture)
{
    assert(g_TextureClass._type == OT_CLASS);
    assert(texture);
    sq_pushobject(g_VM, g_TextureClass); // push texture class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop texture class
    sq_setreleasehook(g_VM, -1, script_texture_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)texture);
    texture->grab(); // grab a new reference
}

//-----------------------------------------------------------------
ITexture* GetTexture(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_TEXTURE))) {
        return (ITexture*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_texture_methods[] = {
    {"constructor",     "Texture.constructor",  script_texture_constructor  },
    {"updatePixels",    "Texture.updatePixels", script_texture_updatePixels },
    {"createCanvas",    "Texture.createCanvas", script_texture_createCanvas },
    {"_get",            "Texture._get",         script_texture__get         },
    {"_typeof",         "Texture._typeof",      script_texture__typeof      },
    {"_cloned",         "Texture._cloned",      script_texture__cloned      },
    {"_tostring",       "Texture._tostring",    script_texture__tostring    },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_texture_static_methods[] = {
    {"Load",            "Texture.Load",            script_texture_Load             },
    {"LoadFromStream",  "Texture.LoadFromStream",  script_texture_LoadFromStream   },
    {"_dump",           "Texture._dump",           script_texture__dump            },
    {"_load",           "Texture._load",           script_texture__load            },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_texture_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_texture_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_TEXTURE);

    // define methods
    for (int i = 0; script_texture_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_texture_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_texture_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_texture_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_texture_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_texture_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_texture_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_texture_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_texture_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_texture_constants[i].name, -1);
        sq_pushinteger(g_VM, script_texture_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_TextureClass);
    sq_getstackobj(g_VM, -1, &g_TextureClass);
    sq_addref(g_VM, &g_TextureClass);

    sq_poptop(g_VM); // pop class
}

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
// CreateColor(red, green, blue [, alpha = 255])
static SQInteger script_CreateColor(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(3)
    GET_ARG_INT(1, red)
    GET_ARG_INT(2, green)
    GET_ARG_INT(3, blue)
    GET_OPTARG_INT(4, alpha, 255)
    RET_INT(RGBA::Pack(red, green, blue, alpha))
}

//-----------------------------------------------------------------
// UnpackRed(color)
static SQInteger script_UnpackRed(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).red)
}

//-----------------------------------------------------------------
// UnpackGreen(color)
static SQInteger script_UnpackGreen(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).green)
}

//-----------------------------------------------------------------
// UnpackBlue(color)
static SQInteger script_UnpackBlue(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).blue)
}

//-----------------------------------------------------------------
// UnpackAlpha(color)
static SQInteger script_UnpackAlpha(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).alpha)
}

//-----------------------------------------------------------------
// GetSupportedVideoModes()
static SQInteger script_GetSupportedVideoModes(HSQUIRRELVM v)
{
    std::vector<Dim2i> video_modes;
    GetSupportedVideoModes(video_modes);
    sq_newarray(v, video_modes.size());
    if (video_modes.size() > 0) {
        for (int i = 0; i < (int)video_modes.size(); ++i) {
            sq_pushinteger(v, i); // key

            sq_newtable(v); // value

            sq_pushstring(v, "width", -1);
            sq_pushinteger(v, video_modes[i].width);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "height", -1);
            sq_pushinteger(v, video_modes[i].height);
            sq_newslot(v, -3, SQFalse);

            sq_rawset(v, -3);
        }
    }
    return 1;
}

//-----------------------------------------------------------------
// OpenWindow(width, height, fullscreen)
static SQInteger script_OpenWindow(HSQUIRRELVM v)
{
    CHECK_NARGS(3)
    GET_ARG_INT(1, width)
    GET_ARG_INT(2, height)
    GET_ARG_BOOL(3, fullscreen)
    if (IsWindowOpen()) {
        THROW_ERROR("Window already open")
    }
    if (width <= 0) {
        THROW_ERROR("Invalid width")
    }
    if (height <= 0) {
        THROW_ERROR("Invalid height")
    }
    if (!OpenWindow(width, height, (fullscreen == SQTrue ? true : false))) {
        THROW_ERROR("Could not open window")
    }
    // set a default window icon if possible
    FilePtr file = OpenFile("/common/system/window_icon.png");
    if (file) {
        CanvasPtr icon = LoadImage(file.get());
        if (icon) {
            SetWindowIcon(icon.get());
        }
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// IsWindowOpen()
static SQInteger script_IsWindowOpen(HSQUIRRELVM v)
{
    RET_BOOL(IsWindowOpen())
}

//-----------------------------------------------------------------
// GetWindowWidth()
static SQInteger script_GetWindowWidth(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    RET_INT(GetWindowWidth())
}

//-----------------------------------------------------------------
// GetWindowHeight()
static SQInteger script_GetWindowHeight(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    RET_INT(GetWindowHeight())
}

//-----------------------------------------------------------------
// IsWindowFullscreen()
static SQInteger script_IsWindowFullscreen(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    RET_BOOL(IsWindowFullscreen())
}

//-----------------------------------------------------------------
// SetWindowFullscreen(fullscreen)
static SQInteger script_SetWindowFullscreen(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_NARGS(1)
    GET_ARG_BOOL(1, fullscreen)
    SetWindowFullscreen((fullscreen == SQTrue ? true : false));
    RET_VOID()
}

//-----------------------------------------------------------------
// GetWindowTitle()
static SQInteger script_GetWindowTitle(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    RET_STRING(GetWindowTitle())
}

//-----------------------------------------------------------------
// SetWindowTitle(title)
static SQInteger script_SetWindowTitle(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_NARGS(1)
    GET_ARG_STRING(1, title)
    SetWindowTitle(title);
    RET_VOID()
}

//-----------------------------------------------------------------
// SetWindowIcon(icon)
static SQInteger script_SetWindowIcon(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_NARGS(1)
    GET_ARG_CANVAS(1, icon)
    SetWindowIcon(icon);
    RET_VOID()
}

//-----------------------------------------------------------------
// ShowFrame()
static SQInteger script_ShowFrame(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    ShowFrame();
    RET_VOID()
}

//-----------------------------------------------------------------
// GetFrameScissor()
static SQInteger script_GetFrameScissor(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    Recti scissor;
    if (!GetFrameScissor(scissor)) {
        THROW_ERROR("Could not get frame scissor")
    }
    RET_RECT(scissor)
}

//-----------------------------------------------------------------
// SetFrameScissor(scissor)
static SQInteger script_SetFrameScissor(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_RECT(1, scissor)
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    SetFrameScissor(*scissor);
    RET_VOID()
}

//-----------------------------------------------------------------
// CloneFrame([section])
static SQInteger script_CloneFrame(HSQUIRRELVM v)
{
    GET_OPTARG_RECT(1, section)
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CanvasPtr canvas = CloneFrame(section);
    if (!canvas) {
        THROW_ERROR("Could not clone frame")
    }
    RET_CANVAS(canvas.get())
}

//-----------------------------------------------------------------
// DrawPoint(x, y, color)
static SQInteger script_DrawPoint(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_NARGS(3)
    GET_ARG_INT(1, x)
    GET_ARG_INT(2, y)
    GET_ARG_INT(3, color)
    DrawPoint(Vec2i(x, y), RGBA::Unpack((u32)color));
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawLine(x1, y1, x2, y2, col1 [, col2])
static SQInteger script_DrawLine(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(5)
    GET_ARG_INT(1, x1);
    GET_ARG_INT(2, y1);
    GET_ARG_INT(3, x2);
    GET_ARG_INT(4, y2);
    GET_ARG_INT(5, col1)
    GET_OPTARG_INT(6, col2, col1)
    Vec2i positions[2] = {
        Vec2i(x1, y1),
        Vec2i(x2, y2),
    };
    RGBA colors[2] = {
        RGBA::Unpack((u32)col1),
        RGBA::Unpack((u32)col2),
    };
    DrawLine(positions, colors);
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawTriangle(x1, y1, x2, y2, x3, y3, col1 [, col2, col3])
static SQInteger script_DrawTriangle(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(7)
    GET_ARG_INT(1, x1)
    GET_ARG_INT(2, y1)
    GET_ARG_INT(3, x2)
    GET_ARG_INT(4, y2)
    GET_ARG_INT(5, x3)
    GET_ARG_INT(6, y3)
    GET_ARG_INT(7, col1)
    GET_OPTARG_INT(8, col2, col1)
    GET_OPTARG_INT(9, col3, col1)
    Vec2i positions[3] = {
        Vec2i(x1, y1),
        Vec2i(x2, y2),
        Vec2i(x3, y3),
    };
    RGBA colors[3] = {
        RGBA::Unpack((u32)col1),
        RGBA::Unpack((u32)col2),
        RGBA::Unpack((u32)col3),
    };
    DrawTriangle(positions, colors);
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawTexturedTriangle(tex, tx1, ty1, tx2, ty2, tx3, ty3, x1, y1, x2, y2, x3, y3 [, mask = CreateColor(255, 255, 255)])
static SQInteger script_DrawTexturedTriangle(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(13)
    GET_ARG_TEXTURE(1, tex)
    GET_ARG_INT(2, tx1)
    GET_ARG_INT(3, ty1)
    GET_ARG_INT(4, tx2)
    GET_ARG_INT(5, ty2)
    GET_ARG_INT(6, tx3)
    GET_ARG_INT(7, ty3)
    GET_ARG_INT(8, x1)
    GET_ARG_INT(9, y1)
    GET_ARG_INT(10, x2)
    GET_ARG_INT(11, y2)
    GET_ARG_INT(12, x3)
    GET_ARG_INT(13, y3)
    GET_OPTARG_INT(14, mask, RGBA::Pack(255, 255, 255))
    Vec2i texcoords[3] = {
        Vec2i(tx1, ty1),
        Vec2i(tx2, ty2),
        Vec2i(tx3, ty3),
    };
    Vec2i positions[3] = {
        Vec2i(x1, y1),
        Vec2i(x2, y2),
        Vec2i(x3, y3),
    };
    DrawTexturedTriangle(tex, texcoords, positions, RGBA::Unpack((u32)mask));
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawRect(rect, col1 [, col2, col3, col4])
static SQInteger script_DrawRect(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(2)
    GET_ARG_RECT(1, rect)
    GET_ARG_INT(2, col1)
    GET_OPTARG_INT(3, col2, col1)
    GET_OPTARG_INT(4, col3, col1)
    GET_OPTARG_INT(5, col4, col1)
    RGBA colors[4] = {
        RGBA::Unpack((u32)col1),
        RGBA::Unpack((u32)col2),
        RGBA::Unpack((u32)col3),
        RGBA::Unpack((u32)col4),
    };
    DrawRect(*rect, colors);
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawImage(image, x, y [, mask = CreateColor(255, 255, 255)])
static SQInteger script_DrawImage(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(3)
    GET_ARG_TEXTURE(1, image)
    GET_ARG_INT(2, x)
    GET_ARG_INT(3, y)
    GET_OPTARG_INT(4, mask, RGBA::Pack(255, 255, 255))
    DrawImage(image, Vec2i(x, y), RGBA::Unpack((u32)mask));
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawSubImage(image, src_rect, x, y [, mask = CreateColor(255, 255, 255)])
static SQInteger script_DrawSubImage(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(4)
    GET_ARG_TEXTURE(1, image)
    GET_ARG_RECT(2, src_rect)
    GET_ARG_INT(3, x)
    GET_ARG_INT(4, y)
    GET_OPTARG_INT(5, mask, RGBA::Pack(255, 255, 255))
    DrawSubImage(image, *src_rect, Vec2i(x, y), RGBA::Unpack((u32)mask));
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawImageQuad(image, x1, y1, x2, y2, x3, y3, x4, y4 [, mask = CreateColor(255, 255, 255)])
static SQInteger script_DrawImageQuad(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(9)
    GET_ARG_TEXTURE(1, image)
    GET_ARG_INT(2, x1)
    GET_ARG_INT(3, y1)
    GET_ARG_INT(4, x2)
    GET_ARG_INT(5, y2)
    GET_ARG_INT(6, x3)
    GET_ARG_INT(7, y3)
    GET_ARG_INT(8, x4)
    GET_ARG_INT(9, y4)
    GET_OPTARG_INT(10, mask, RGBA::Pack(255, 255, 255))
    Vec2i positions[4] = {
        Vec2i(x1, y1),
        Vec2i(x2, y2),
        Vec2i(x3, y3),
        Vec2i(x4, y4),
    };
    DrawImageQuad(image, positions, RGBA::Unpack((u32)mask));
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawSubImageQuad(image, src_rect, x1, y1, x2, y2, x3, y3, x4, y4 [, mask = CreateColor(255, 255, 255)])
static SQInteger script_DrawSubImageQuad(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(10)
    GET_ARG_TEXTURE(1, image)
    GET_ARG_RECT(2, src_rect)
    GET_ARG_INT(3, x1)
    GET_ARG_INT(4, y1)
    GET_ARG_INT(5, x2)
    GET_ARG_INT(6, y2)
    GET_ARG_INT(7, x3)
    GET_ARG_INT(8, y3)
    GET_ARG_INT(9, x4)
    GET_ARG_INT(10, y4)
    GET_OPTARG_INT(11, mask, RGBA::Pack(255, 255, 255))
    Vec2i positions[4] = {
        Vec2i(x1, y1),
        Vec2i(x2, y2),
        Vec2i(x3, y3),
        Vec2i(x4, y4),
    };
    DrawSubImageQuad(image, *src_rect, positions, RGBA::Unpack((u32)mask));
    RET_VOID()
}

//-----------------------------------------------------------------
static ScriptFuncReg script_graphics_functions[] = {
    {"CreateColor",                 "CreateColor",              script_CreateColor              },
    {"UnpackRed",                   "UnpackRed",                script_UnpackRed                },
    {"UnpackGreen",                 "UnpackGreen",              script_UnpackGreen              },
    {"UnpackBlue",                  "UnpackBlue",               script_UnpackBlue               },
    {"UnpackAlpha",                 "UnpackAlpha",              script_UnpackAlpha              },
    {"GetSupportedVideoModes",      "GetSupportedVideoModes",   script_GetSupportedVideoModes   },
    {"OpenWindow",                  "OpenWindow",               script_OpenWindow               },
    {"IsWindowOpen",                "IsWindowOpen",             script_IsWindowOpen             },
    {"GetWindowWidth",              "GetWindowWidth",           script_GetWindowWidth           },
    {"GetWindowHeight",             "GetWindowHeight",          script_GetWindowHeight          },
    {"IsWindowFullscreen",          "IsWindowFullscreen",       script_IsWindowFullscreen       },
    {"SetWindowFullscreen",         "SetWindowFullscreen",      script_SetWindowFullscreen      },
    {"GetWindowTitle",              "GetWindowTitle",           script_GetWindowTitle           },
    {"SetWindowTitle",              "SetWindowTitle",           script_SetWindowTitle           },
    {"SetWindowIcon",               "SetWindowIcon",            script_SetWindowIcon            },
    {"ShowFrame",                   "ShowFrame",                script_ShowFrame                },
    {"GetFrameScissor",             "GetFrameScissor",          script_GetFrameScissor          },
    {"SetFrameScissor",             "SetFrameScissor",          script_SetFrameScissor          },
    {"CloneFrame",                  "CloneFrame",               script_CloneFrame               },
    {"DrawPoint",                   "DrawPoint",                script_DrawPoint                },
    {"DrawLine",                    "DrawLine",                 script_DrawLine                 },
    {"DrawTriangle",                "DrawTriangle",             script_DrawTriangle             },
    {"DrawTexturedTriangle",        "DrawTexturedTriangle",     script_DrawTexturedTriangle     },
    {"DrawRect",                    "DrawRect",                 script_DrawRect                 },
    {"DrawImage",                   "DrawImage",                script_DrawImage                },
    {"DrawSubImage",                "DrawSubImage",             script_DrawSubImage             },
    {"DrawImageQuad",               "DrawImageQuad",            script_DrawImageQuad            },
    {"DrawSubImageQuad",            "DrawSubImageQuad",         script_DrawSubImageQuad         },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_graphics_constants[] = {
    {"BLACK",   RGBA::Pack(  0,   0,   0) },
    {"WHITE",   RGBA::Pack(255, 255, 255) },
    {"RED",     RGBA::Pack(255,   0,   0) },
    {"GREEN",   RGBA::Pack(0,   255,   0) },
    {"BLUE",    RGBA::Pack(0,     0, 255) },
    {"YELLOW",  RGBA::Pack(255, 255,   0) },
    {0,0}
};

//-----------------------------------------------------------------
static void register_graphics_api()
{
    // init classes
    init_canvas_class();
    init_texture_class();

    // register classes
    sq_pushstring(g_VM, "Canvas", -1);
    sq_pushobject(g_VM, g_CanvasClass);
    sq_newslot(g_VM, -3, SQFalse);

    sq_pushstring(g_VM, "Texture", -1);
    sq_pushobject(g_VM, g_TextureClass);
    sq_newslot(g_VM, -3, SQFalse);

    // register functions
    for (int i = 0; script_graphics_functions[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_graphics_functions[i].funcname, -1);
        sq_newclosure(g_VM, script_graphics_functions[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_graphics_functions[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // register constants
    for (int i = 0; script_graphics_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_graphics_constants[i].name, -1);
        sq_pushinteger(g_VM, script_graphics_constants[i].value);
        sq_newslot(g_VM, -3, SQFalse);
    }
}

/******************************************************************
 *                                                                *
 *                             SOUND                              *
 *                                                                *
 ******************************************************************/

/************************* SOUND OBJECT ***************************/

#define SETUP_SOUND_OBJECT() \
    ISound* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) { \
        THROW_ERROR("Invalid type of environment object, expected a Sound instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_sound_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ISound*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Sound.Load(filename [, streaming = false])
static SQInteger script_sound_Load(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STRING(1, filename)
    GET_OPTARG_BOOL(2, streaming, SQFalse)
    FilePtr file = OpenFile(filename);
    if (!file) {
        THROW_ERROR("Could not open file")
    }
    SoundPtr sound = LoadSound(file.get(), (streaming == SQTrue ? true : false));
    if (!sound) {
        THROW_ERROR("Could not load sound")
    }
    RET_SOUND(sound.get())
}

//-----------------------------------------------------------------
// Sound.LoadFromStream(stream [, streaming = false])
static SQInteger script_sound_LoadFromStream(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STREAM(1, stream)
    GET_OPTARG_BOOL(2, streaming, SQFalse)
    SoundPtr sound = LoadSound(stream, (streaming == SQTrue ? true : false));
    if (!sound) {
        THROW_ERROR("Could not load sound")
    }
    RET_SOUND(sound.get())
}


//-----------------------------------------------------------------
// Sound.play()
static SQInteger script_sound_play(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    This->play();
    RET_VOID()
}

//-----------------------------------------------------------------
// Sound.stop()
static SQInteger script_sound_stop(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    This->stop();
    RET_VOID()
}

//-----------------------------------------------------------------
// Sound.reset()
static SQInteger script_sound_reset(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    This->reset();
    RET_VOID()
}

//-----------------------------------------------------------------
// Sound.isPlaying()
static SQInteger script_sound_isPlaying(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    RET_BOOL(This->isPlaying())
}

//-----------------------------------------------------------------
// Sound.isSeekable()
static SQInteger script_sound_isSeekable(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    RET_BOOL(This->isSeekable())
}

//-----------------------------------------------------------------
// Sound._get(index)
static SQInteger script_sound__get(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "length") == 0) {
        RET_INT(This->getLength())
    } else if (strcmp(index, "repeat") == 0) {
        RET_BOOL(This->getRepeat())
    } else if (strcmp(index, "volume") == 0) {
        RET_INT(This->getVolume())
    } else if (strcmp(index, "position") == 0) {
        RET_INT(This->getPosition())
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
}

//-----------------------------------------------------------------
// Sound._set(index, value)
static SQInteger script_sound__set(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "repeat") == 0) {
        GET_ARG_BOOL(2, value)
        This->setRepeat((value == SQTrue ? true : false));
    } else if (strcmp(index, "volume") == 0) {
        GET_ARG_INT(2, value)
        This->setVolume(value);
    } else if (strcmp(index, "position") == 0) {
        GET_ARG_INT(2, value)
        This->setPosition(value);
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// Sound._typeof()
static SQInteger script_sound__typeof(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    RET_STRING("Sound")
}

//-----------------------------------------------------------------
// Sound._tostring()
static SQInteger script_sound__tostring(HSQUIRRELVM v)
{
    SETUP_SOUND_OBJECT()
    std::ostringstream oss;
    oss << "<Sound instance at " << This;
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
void BindSound(ISound* sound)
{
    assert(g_SoundClass._type == OT_CLASS);
    assert(sound);
    sq_pushobject(g_VM, g_SoundClass); // push sound class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop sound class
    sq_setreleasehook(g_VM, -1, script_sound_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)sound);
    sound->grab(); // grab a new reference
}

//-----------------------------------------------------------------
ISound* GetSound(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_SOUND))) {
        return (ISound*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_sound_methods[] = {
    {"play",            "Sound.play",           script_sound_play           },
    {"stop",            "Sound.stop",           script_sound_stop           },
    {"reset",           "Sound.reset",          script_sound_reset          },
    {"isPlaying",       "Sound.isPlaying",      script_sound_isPlaying      },
    {"isSeekable",      "Sound.isSeekable",     script_sound_isSeekable     },
    {"_get",            "Sound._get",           script_sound__get           },
    {"_set",            "Sound._set",           script_sound__set           },
    {"_typeof",         "Sound._typeof",        script_sound__typeof        },
    {"_tostring",       "Sound._tostring",      script_sound__tostring      },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_sound_static_methods[] = {
    {"Load",            "Sound.Load",           script_sound_Load           },
    {"LoadFromStream",  "Sound.LoadFromStream", script_sound_LoadFromStream },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_sound_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_sound_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_SOUND);

    // define methods
    for (int i = 0; script_sound_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_sound_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_sound_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_sound_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_sound_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_sound_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_sound_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_sound_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_sound_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_sound_constants[i].name, -1);
        sq_pushinteger(g_VM, script_sound_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_SoundClass);
    sq_getstackobj(g_VM, -1, &g_SoundClass);
    sq_addref(g_VM, &g_SoundClass);

    sq_poptop(g_VM); // pop class
}

/********************** SOUNDEFFECT OBJECT ************************/

#define SETUP_SOUNDEFFECT_OBJECT() \
    ISoundEffect* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) { \
        THROW_ERROR("Invalid type of environment object, expected a SoundEffect instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_soundeffect_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ISoundEffect*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// SoundEffect.Load(filename)
static SQInteger script_soundeffect_Load(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STRING(1, filename)
    FilePtr file = OpenFile(filename);
    if (!file) {
        THROW_ERROR("Could not open file")
    }
    SoundEffectPtr sound_effect = LoadSoundEffect(file.get());
    if (!sound_effect) {
        THROW_ERROR("Could not load sound")
    }
    RET_SOUNDEFFECT(sound_effect.get())
}

//-----------------------------------------------------------------
// SoundEffect.LoadFromStream(stream)
static SQInteger script_soundeffect_LoadFromStream(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STREAM(1, stream)
    SoundEffectPtr sound_effect = LoadSoundEffect(stream);
    if (!sound_effect) {
        THROW_ERROR("Could not load sound")
    }
    RET_SOUNDEFFECT(sound_effect.get())
}

//-----------------------------------------------------------------
// SoundEffect.play()
static SQInteger script_soundeffect_play(HSQUIRRELVM v)
{
    SETUP_SOUNDEFFECT_OBJECT()
    This->play();
    RET_VOID()
}

//-----------------------------------------------------------------
// SoundEffect.stop()
static SQInteger script_soundeffect_stop(HSQUIRRELVM v)
{
    SETUP_SOUNDEFFECT_OBJECT()
    This->stop();
    RET_VOID()
}

//-----------------------------------------------------------------
// SoundEffect._get(index)
static SQInteger script_soundeffect__get(HSQUIRRELVM v)
{
    SETUP_SOUNDEFFECT_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "volume") == 0) {
        RET_INT(This->getVolume())
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
}

//-----------------------------------------------------------------
// SoundEffect._set(index, value)
static SQInteger script_soundeffect__set(HSQUIRRELVM v)
{
    SETUP_SOUNDEFFECT_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_STRING(1, index)
    if (strcmp(index, "volume") == 0) {
        GET_ARG_INT(2, value)
        This->setVolume(value);
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// SoundEffect._typeof()
static SQInteger script_soundeffect__typeof(HSQUIRRELVM v)
{
    SETUP_SOUNDEFFECT_OBJECT()
    RET_STRING("SoundEffect")
}

//-----------------------------------------------------------------
// SoundEffect._tostring()
static SQInteger script_soundeffect__tostring(HSQUIRRELVM v)
{
    SETUP_SOUNDEFFECT_OBJECT()
    std::ostringstream oss;
    oss << "<SoundEffect instance at " << This;
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
void BindSoundEffect(ISoundEffect* soundeffect)
{
    assert(g_SoundEffectClass._type == OT_CLASS);
    assert(soundeffect);
    sq_pushobject(g_VM, g_SoundEffectClass); // push soundeffect class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop soundeffect class
    sq_setreleasehook(g_VM, -1, script_soundeffect_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)soundeffect);
    soundeffect->grab(); // grab a new reference
}

//-----------------------------------------------------------------
ISoundEffect* GetSoundEffect(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_SOUNDEFFECT))) {
        return (ISoundEffect*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_soundeffect_methods[] = {
    {"play",            "SoundEffect.play",           script_soundeffect_play           },
    {"stop",            "SoundEffect.stop",           script_soundeffect_stop           },
    {"_get",            "SoundEffect._get",           script_soundeffect__get           },
    {"_set",            "SoundEffect._set",           script_soundeffect__set           },
    {"_typeof",         "SoundEffect._typeof",        script_soundeffect__typeof        },
    {"_tostring",       "SoundEffect._tostring",      script_soundeffect__tostring      },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_soundeffect_static_methods[] = {
    {"Load",            "SoundEffect.Load",           script_soundeffect_Load           },
    {"LoadFromStream",  "SoundEffect.LoadFromStream", script_soundeffect_LoadFromStream },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_soundeffect_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_soundeffect_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_SOUNDEFFECT);

    // define methods
    for (int i = 0; script_soundeffect_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_soundeffect_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_soundeffect_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_soundeffect_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_soundeffect_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_soundeffect_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_soundeffect_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_soundeffect_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_soundeffect_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_soundeffect_constants[i].name, -1);
        sq_pushinteger(g_VM, script_soundeffect_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_SoundEffectClass);
    sq_getstackobj(g_VM, -1, &g_SoundEffectClass);
    sq_addref(g_VM, &g_SoundEffectClass);

    sq_poptop(g_VM); // pop class
}

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
static ScriptFuncReg script_audio_functions[] = {
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_audio_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void register_sound_api()
{
    // init classes
    init_sound_class();
    init_soundeffect_class();

    // register classes
    sq_pushstring(g_VM, "Sound", -1);
    sq_pushobject(g_VM, g_SoundClass);
    sq_newslot(g_VM, -3, SQFalse);

    sq_pushstring(g_VM, "SoundEffect", -1);
    sq_pushobject(g_VM, g_SoundEffectClass);
    sq_newslot(g_VM, -3, SQFalse);

    // register functions
    for (int i = 0; script_audio_functions[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_audio_functions[i].funcname, -1);
        sq_newclosure(g_VM, script_audio_functions[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_audio_functions[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // register constants
    for (int i = 0; script_audio_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_audio_constants[i].name, -1);
        sq_pushinteger(g_VM, script_audio_constants[i].value);
        sq_newslot(g_VM, -3, SQFalse);
    }
}

/******************************************************************
 *                                                                *
 *                             INPUT                              *
 *                                                                *
 ******************************************************************/

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
// UpdateInput()
static SQInteger script_UpdateInput(HSQUIRRELVM v)
{
    UpdateInput();
    RET_VOID()
}

//-----------------------------------------------------------------
// AreEventsPending()
static SQInteger script_AreEventsPending(HSQUIRRELVM v)
{
    RET_BOOL(AreEventsPending())
}

//-----------------------------------------------------------------
// GetEvent()
static SQInteger script_GetEvent(HSQUIRRELVM v)
{
    Event event;
    if (GetEvent(event)) {
        sq_newtable(v); // event

        sq_pushstring(v, "type", -1);
        sq_pushinteger(v, event.type);
        sq_newslot(v, -3, SQFalse);

        switch (event.type) {
        case Event::KEY_DOWN:
        case Event::KEY_UP:
            sq_pushstring(v, "key", -1);
            sq_pushinteger(v, event.key.key);
            sq_newslot(v, -3, SQFalse);
            break;

        case Event::MOUSE_BUTTON_DOWN:
        case Event::MOUSE_BUTTON_UP:
            sq_pushstring(v, "button", -1);
            sq_pushinteger(v, event.mbutton.button);
            sq_newslot(v, -3, SQFalse);
            break;

        case Event::MOUSE_MOTION:
            sq_pushstring(v, "dx", -1);
            sq_pushinteger(v, event.mmotion.dx);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "dy", -1);
            sq_pushinteger(v, event.mmotion.dy);
            sq_newslot(v, -3, SQFalse);
            break;

        case Event::MOUSE_WHEEL_MOTION:
            sq_pushstring(v, "dx", -1);
            sq_pushinteger(v, event.mwheel.dx);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "dy", -1);
            sq_pushinteger(v, event.mwheel.dy);
            sq_newslot(v, -3, SQFalse);
            break;

        case Event::JOY_BUTTON_DOWN:
        case Event::JOY_BUTTON_UP:
            sq_pushstring(v, "joy", -1);
            sq_pushinteger(v, event.jbutton.joy);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "button", -1);
            sq_pushinteger(v, event.jbutton.button);
            sq_newslot(v, -3, SQFalse);
            break;

        case Event::JOY_AXIS_MOTION:
            sq_pushstring(v, "joy", -1);
            sq_pushinteger(v, event.jaxis.joy);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "axis", -1);
            sq_pushinteger(v, event.jaxis.axis);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "value", -1);
            sq_pushinteger(v, event.jaxis.value);
            sq_newslot(v, -3, SQFalse);
            break;

        case Event::JOY_HAT_MOTION:
            sq_pushstring(v, "joy", -1);
            sq_pushinteger(v, event.jhat.joy);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "hat", -1);
            sq_pushinteger(v, event.jhat.hat);
            sq_newslot(v, -3, SQFalse);

            sq_pushstring(v, "value", -1);
            sq_pushinteger(v, event.jhat.value);
            sq_newslot(v, -3, SQFalse);
            break;

        case Event::APP_QUIT:
            break;

        default:
            sq_poptop(v); // pop event table
            THROW_ERROR("Unrecognized event type")
        }
    } else {
        sq_pushnull(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// ClearEvents()
static SQInteger script_ClearEvents(HSQUIRRELVM v)
{
    ClearEvents();
    RET_VOID()
}

//-----------------------------------------------------------------
// IsKeyDown(key)
static SQInteger script_IsKeyDown(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, key)
    RET_BOOL(IsKeyDown(key))
}

//-----------------------------------------------------------------
// GetMouseX()
static SQInteger script_GetMouseX(HSQUIRRELVM v)
{
    RET_INT(GetMouseX())
}

//-----------------------------------------------------------------
// GetMouseY()
static SQInteger script_GetMouseY(HSQUIRRELVM v)
{
    RET_INT(GetMouseY())
}

//-----------------------------------------------------------------
// IsMouseButtonDown(button)
static SQInteger script_IsMouseButtonDown(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, button)
    RET_BOOL(IsMouseButtonDown(button))
}

//-----------------------------------------------------------------
// GetNumJoysticks()
static SQInteger script_GetNumJoysticks(HSQUIRRELVM v)
{
    RET_INT(GetNumJoysticks())
}

//-----------------------------------------------------------------
// GetJoystickName(joy)
static SQInteger script_GetJoystickName(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    const char* name = GetJoystickName(joy);
    if (name) {
        RET_STRING(name)
    } else {
        RET_NULL()
    }
}

//-----------------------------------------------------------------
// GetNumJoystickButtons(joy)
static SQInteger script_GetNumJoystickButtons(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    RET_INT(GetNumJoystickButtons(joy))
}

//-----------------------------------------------------------------
// GetNumJoystickAxes(joy)
static SQInteger script_GetNumJoystickAxes(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    RET_INT(GetNumJoystickAxes(joy))
}

//-----------------------------------------------------------------
// GetNumJoystickHats(joy)
static SQInteger script_GetNumJoystickHats(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    RET_INT(GetNumJoystickHats(joy))
}

//-----------------------------------------------------------------
// IsJoystickButtonDown(joy, button)
static SQInteger script_IsJoystickButtonDown(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, button)
    RET_BOOL(IsJoystickButtonDown(joy, button))
}

//-----------------------------------------------------------------
// GetJoystickAxis(joy, axis)
static SQInteger script_GetJoystickAxis(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, axis)
    RET_INT(GetJoystickAxis(joy, axis))
}

//-----------------------------------------------------------------
// GetJoystickHat(joy, hat)
static SQInteger script_GetJoystickHat(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, hat)
    RET_INT(GetJoystickHat(joy, hat))
}

//-----------------------------------------------------------------
// IsJoystickHaptic(joy)
static SQInteger script_IsJoystickHaptic(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    RET_BOOL(IsJoystickHaptic(joy))
}

//-----------------------------------------------------------------
// CreateJoystickForce(joy, strength, duration)
static SQInteger script_CreateJoystickForce(HSQUIRRELVM v)
{
    CHECK_NARGS(3)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, strength)
    GET_ARG_INT(3, duration)
    if (strength < 0 || strength > 100) {
        THROW_ERROR("Invalid strength")
    }
    int force = CreateJoystickForce(joy, strength, duration);
    if (force < 0) {
        THROW_ERROR("Could not create joystick force")
    }
    RET_INT(force)
}

//-----------------------------------------------------------------
// ApplyJoystickForce(joy, force [, times = 1])
static SQInteger script_ApplyJoystickForce(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, force)
    GET_OPTARG_INT(3, times, 1)
    if (!ApplyJoystickForce(joy, force, times)) {
        THROW_ERROR("Could not apply joystick force")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// StopJoystickForce(joy, force)
static SQInteger script_StopJoystickForce(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, force)
    if (!StopJoystickForce(joy, force)) {
        THROW_ERROR("Could not stop joystick force")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// StopAllJoystickForces(joy)
static SQInteger script_StopAllJoystickForces(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    StopAllJoystickForces(joy);
    RET_VOID()
}

//-----------------------------------------------------------------
// DestroyJoystickForce(joy, force)
static SQInteger script_DestroyJoystickForce(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, force)
    DestroyJoystickForce(joy, force);
    RET_VOID()
}

//-----------------------------------------------------------------
static ScriptFuncReg script_input_functions[] = {
    {"UpdateInput",                 "UpdateInput",                  script_UpdateInput                  },
    {"AreEventsPending",            "AreEventsPending",             script_AreEventsPending             },
    {"GetEvent",                    "GetEvent",                     script_GetEvent                     },
    {"ClearEvents",                 "ClearEvents",                  script_ClearEvents                  },
    {"IsKeyDown",                   "IsKeyDown",                    script_IsKeyDown                    },
    {"GetMouseX",                   "GetMouseX",                    script_GetMouseX                    },
    {"GetMouseY",                   "GetMouseY",                    script_GetMouseY                    },
    {"IsMouseButtonDown",           "IsMouseButtonDown",            script_IsMouseButtonDown            },
    {"GetNumJoysticks",             "GetNumJoysticks",              script_GetNumJoysticks              },
    {"GetJoystickName",             "GetJoystickName",              script_GetJoystickName              },
    {"GetNumJoystickButtons",       "GetNumJoystickButtons",        script_GetNumJoystickButtons        },
    {"GetNumJoystickAxes",          "GetNumJoystickAxes",           script_GetNumJoystickAxes           },
    {"GetNumJoystickHats",          "GetNumJoystickHats",           script_GetNumJoystickHats           },
    {"IsJoystickButtonDown",        "IsJoystickButtonDown",         script_IsJoystickButtonDown         },
    {"GetJoystickAxis",             "GetJoystickAxis",              script_GetJoystickAxis              },
    {"GetJoystickHat",              "GetJoystickHat",               script_GetJoystickHat               },
    {"IsJoystickHaptic",            "IsJoystickHaptic",             script_IsJoystickHaptic             },
    {"CreateJoystickForce",         "CreateJoystickForce",          script_CreateJoystickForce          },
    {"ApplyJoystickForce",          "ApplyJoystickForce",           script_ApplyJoystickForce           },
    {"StopJoystickForce",           "StopJoystickForce",            script_StopJoystickForce            },
    {"StopAllJoystickForces",       "StopAllJoystickForces",        script_StopAllJoystickForces        },
    {"DestroyJoystickForce",        "DestroyJoystickForce",         script_DestroyJoystickForce         },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_input_constants[] = {

    // event constants
    {"KEY_DOWN_EVENT",             Event::KEY_DOWN          },
    {"KEY_UP_EVENT",               Event::KEY_UP            },
    {"MOUSE_BUTTON_DOWN_EVENT",    Event::MOUSE_BUTTON_DOWN },
    {"MOUSE_BUTTON_UP_EVENT",      Event::MOUSE_BUTTON_UP   },
    {"MOUSE_MOTION_EVENT",         Event::MOUSE_MOTION      },
    {"MOUSE_WHEEL_MOTION_EVENT",   Event::MOUSE_WHEEL_MOTION},
    {"JOY_BUTTON_DOWN_EVENT",      Event::JOY_BUTTON_DOWN   },
    {"JOY_BUTTON_UP_EVENT",        Event::JOY_BUTTON_UP     },
    {"JOY_AXIS_MOTION_EVENT",      Event::JOY_AXIS_MOTION   },
    {"JOY_HAT_MOTION_EVENT",       Event::JOY_HAT_MOTION    },
    {"APP_QUIT_EVENT",             Event::APP_QUIT          },

    // key constants
    {"KEY_A",               KEY_A               },
    {"KEY_B",               KEY_B               },
    {"KEY_C",               KEY_C               },
    {"KEY_D",               KEY_D               },
    {"KEY_E",               KEY_E               },
    {"KEY_F",               KEY_F               },
    {"KEY_G",               KEY_G               },
    {"KEY_H",               KEY_H               },
    {"KEY_I",               KEY_I               },
    {"KEY_J",               KEY_J               },
    {"KEY_K",               KEY_K               },
    {"KEY_L",               KEY_L               },
    {"KEY_M",               KEY_M               },
    {"KEY_N",               KEY_N               },
    {"KEY_O",               KEY_O               },
    {"KEY_P",               KEY_P               },
    {"KEY_Q",               KEY_Q               },
    {"KEY_R",               KEY_R               },
    {"KEY_S",               KEY_S               },
    {"KEY_T",               KEY_T               },
    {"KEY_U",               KEY_U               },
    {"KEY_V",               KEY_V               },
    {"KEY_W",               KEY_W               },
    {"KEY_X",               KEY_X               },
    {"KEY_Y",               KEY_Y               },
    {"KEY_Z",               KEY_Z               },
    {"KEY_1",               KEY_1               },
    {"KEY_2",               KEY_2               },
    {"KEY_3",               KEY_3               },
    {"KEY_4",               KEY_4               },
    {"KEY_5",               KEY_5               },
    {"KEY_6",               KEY_6               },
    {"KEY_7",               KEY_7               },
    {"KEY_8",               KEY_8               },
    {"KEY_9",               KEY_9               },
    {"KEY_0",               KEY_0               },
    {"KEY_RETURN",          KEY_RETURN          },
    {"KEY_ESCAPE",          KEY_ESCAPE          },
    {"KEY_BACKSPACE",       KEY_BACKSPACE       },
    {"KEY_TAB",             KEY_TAB             },
    {"KEY_SPACE",           KEY_SPACE           },
    {"KEY_MINUS",           KEY_MINUS           },
    {"KEY_EQUALS",          KEY_EQUALS          },
    {"KEY_LEFTBRACKET",     KEY_LEFTBRACKET     },
    {"KEY_RIGHTBRACKET",    KEY_RIGHTBRACKET    },
    {"KEY_BACKSLASH",       KEY_BACKSLASH       },
    {"KEY_SEMICOLON",       KEY_SEMICOLON       },
    {"KEY_APOSTROPHE",      KEY_APOSTROPHE      },
    {"KEY_GRAVE",           KEY_GRAVE           },
    {"KEY_COMMA",           KEY_COMMA           },
    {"KEY_PERIOD",          KEY_PERIOD          },
    {"KEY_SLASH",           KEY_SLASH           },
    {"KEY_CAPSLOCK",        KEY_CAPSLOCK        },
    {"KEY_F1",              KEY_F1              },
    {"KEY_F2",              KEY_F2              },
    {"KEY_F3",              KEY_F3              },
    {"KEY_F4",              KEY_F4              },
    {"KEY_F5",              KEY_F5              },
    {"KEY_F6",              KEY_F6              },
    {"KEY_F7",              KEY_F7              },
    {"KEY_F8",              KEY_F8              },
    {"KEY_F9",              KEY_F9              },
    {"KEY_F10",             KEY_F10             },
    {"KEY_F11",             KEY_F11             },
    {"KEY_F12",             KEY_F12             },
    {"KEY_PRINTSCREEN",     KEY_PRINTSCREEN     },
    {"KEY_SCROLLLOCK",      KEY_SCROLLLOCK      },
    {"KEY_PAUSE",           KEY_PAUSE           },
    {"KEY_INSERT",          KEY_INSERT          },
    {"KEY_HOME",            KEY_HOME            },
    {"KEY_PAGEUP",          KEY_PAGEUP          },
    {"KEY_DELETE",          KEY_DELETE          },
    {"KEY_END",             KEY_END             },
    {"KEY_PAGEDOWN",        KEY_PAGEDOWN        },
    {"KEY_RIGHT",           KEY_RIGHT           },
    {"KEY_LEFT",            KEY_LEFT            },
    {"KEY_DOWN",            KEY_DOWN            },
    {"KEY_UP",              KEY_UP              },
    {"KEY_NUMLOCKCLEAR",    KEY_NUMLOCKCLEAR    },
    {"KEY_KP_DIVIDE",       KEY_KP_DIVIDE       },
    {"KEY_KP_MULTIPLY",     KEY_KP_MULTIPLY     },
    {"KEY_KP_MINUS",        KEY_KP_MINUS        },
    {"KEY_KP_PLUS",         KEY_KP_PLUS         },
    {"KEY_KP_ENTER",        KEY_KP_ENTER        },
    {"KEY_KP_1",            KEY_KP_1            },
    {"KEY_KP_2",            KEY_KP_2            },
    {"KEY_KP_3",            KEY_KP_3            },
    {"KEY_KP_4",            KEY_KP_4            },
    {"KEY_KP_5",            KEY_KP_5            },
    {"KEY_KP_6",            KEY_KP_6            },
    {"KEY_KP_7",            KEY_KP_7            },
    {"KEY_KP_8",            KEY_KP_8            },
    {"KEY_KP_9",            KEY_KP_9            },
    {"KEY_KP_0",            KEY_KP_0            },
    {"KEY_KP_PERIOD",       KEY_KP_PERIOD       },
    {"KEY_NONUSBACKSLASH",  KEY_NONUSBACKSLASH  },
    {"KEY_KP_EQUALS",       KEY_KP_EQUALS       },
    {"KEY_KP_COMMA",        KEY_KP_COMMA        },
    {"KEY_CANCEL",          KEY_CANCEL          },
    {"KEY_CLEAR",           KEY_CLEAR           },
    {"KEY_LCTRL",           KEY_LCTRL           },
    {"KEY_LSHIFT",          KEY_LSHIFT          },
    {"KEY_LALT",            KEY_LALT            },
    {"KEY_RCTRL",           KEY_RCTRL           },
    {"KEY_RSHIFT",          KEY_RSHIFT          },
    {"KEY_RALT",            KEY_RALT            },

    // mouse constants
    {"MOUSE_BUTTON_LEFT",   MOUSE_BUTTON_LEFT   },
    {"MOUSE_BUTTON_MIDDLE", MOUSE_BUTTON_MIDDLE },
    {"MOUSE_BUTTON_RIGHT",  MOUSE_BUTTON_RIGHT  },
    {"MOUSE_BUTTON_X1",     MOUSE_BUTTON_X1     },
    {"MOUSE_BUTTON_X2",     MOUSE_BUTTON_X2     },

    // joystick constants
    {"JOY_AXIS_X",          JOY_AXIS_X          },
    {"JOY_AXIS_Y",          JOY_AXIS_Y          },
    {"JOY_AXIS_Z",          JOY_AXIS_Z          },
    {"JOY_AXIS_R",          JOY_AXIS_R          },
    {"JOY_HAT_CENTERED",    JOY_HAT_CENTERED    },
    {"JOY_HAT_UP",          JOY_HAT_UP          },
    {"JOY_HAT_RIGHT",       JOY_HAT_RIGHT       },
    {"JOY_HAT_DOWN",        JOY_HAT_DOWN        },
    {"JOY_HAT_LEFT",        JOY_HAT_LEFT        },
    {"JOY_HAT_RIGHTUP",     JOY_HAT_RIGHTUP     },
    {"JOY_HAT_RIGHTDOWN",   JOY_HAT_RIGHTDOWN   },
    {"JOY_HAT_LEFTUP",      JOY_HAT_LEFTUP      },
    {"JOY_HAT_LEFTDOWN",    JOY_HAT_LEFTDOWN    },

    {0,0}
};

//-----------------------------------------------------------------
static void register_input_api()
{
    // register functions
    for (int i = 0; script_input_functions[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_input_functions[i].funcname, -1);
        sq_newclosure(g_VM, script_input_functions[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_input_functions[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // register constants
    for (int i = 0; script_input_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_input_constants[i].name, -1);
        sq_pushinteger(g_VM, script_input_constants[i].value);
        sq_newslot(g_VM, -3, SQFalse);
    }
}

/******************************************************************
 *                                                                *
 *                             UTILITY                            *
 *                                                                *
 ******************************************************************/

/************************* ZSTREAM OBJECT *************************/

#define SETUP_ZSTREAM_OBJECT() \
    ZStream* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) { \
        THROW_ERROR("Invalid type of environment object, expected a ZStream instance") \
    }

//-----------------------------------------------------------------
static SQInteger script_zstream_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ZStream*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// ZStream()
static SQInteger script_zstream_constructor(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    This = ZStream::Create();
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_zstream_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// ZStream.getBufferSize()
static SQInteger script_zstream_getBufferSize(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    RET_INT(This->getBufferSize())
}

//-----------------------------------------------------------------
// ZStream.setBufferSize(size)
static SQInteger script_zstream_setBufferSize(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    CHECK_NARGS(1)
    GET_ARG_INT(1, size)
    if (size <= 0) {
        THROW_ERROR("Invalid size")
    }
    This->setBufferSize(size);
    RET_VOID()
}

//-----------------------------------------------------------------
// ZStream.compress(data [, out])
static SQInteger script_zstream_compress(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    CHECK_MIN_NARGS(1)
    GET_ARG_BLOB(1, data)
    GET_OPTARG_BLOB(2, out)
    if (data->getSize() == 0) {
        THROW_ERROR("Empty input data")
    }
    if (out) {
        if (!This->compress(data->getBuffer(), data->getSize(), out)) {
            THROW_ERROR("Error compressing")
        }
        RET_ARG(2)
    } else {
        BlobPtr blob = Blob::Create();
        if (!This->compress(data->getBuffer(), data->getSize(), blob.get())) {
            THROW_ERROR("Error compressing")
        }
        RET_BLOB(blob.get())
    }
}

//-----------------------------------------------------------------
// ZStream.decompress(data [, out])
static SQInteger script_zstream_decompress(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    CHECK_MIN_NARGS(1)
    GET_ARG_BLOB(1, data)
    GET_OPTARG_BLOB(2, out)
    if (data->getSize() == 0) {
        THROW_ERROR("Empty input data")
    }
    if (out) {
        if (!This->decompress(data->getBuffer(), data->getSize(), out)) {
            THROW_ERROR("Error decompressing")
        }
        RET_ARG(2)
    } else {
        BlobPtr blob = Blob::Create();
        if (!This->decompress(data->getBuffer(), data->getSize(), blob.get())) {
            THROW_ERROR("Error decompressing")
        }
        RET_BLOB(blob.get())
    }
}

//-----------------------------------------------------------------
// ZStream.finish([out])
static SQInteger script_zstream_finish(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    GET_OPTARG_BLOB(1, out)
    if (out) {
        if (!This->finish(out)) {
            THROW_ERROR("Error finishing")
        }
        RET_ARG(2)
    } else {
        BlobPtr blob = Blob::Create();
        if (!This->finish(blob.get())) {
            THROW_ERROR("Error finishing")
        }
        RET_BLOB(blob.get())
    }
}

//-----------------------------------------------------------------
// ZStream._typeof()
static SQInteger script_zstream__typeof(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    RET_STRING("ZStream")
}

//-----------------------------------------------------------------
// ZStream._tostring()
static SQInteger script_zstream__tostring(HSQUIRRELVM v)
{
    SETUP_ZSTREAM_OBJECT()
    std::ostringstream oss;
    oss << "<ZStream instance at " << This;
    oss << ")>";
    RET_STRING(oss.str().c_str())
}

//-----------------------------------------------------------------
void BindZStream(ZStream* stream)
{
    assert(g_ZStreamClass._type == OT_CLASS);
    assert(stream);
    sq_pushobject(g_VM, g_ZStreamClass); // push zstream class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop zstream class
    sq_setreleasehook(g_VM, -1, script_zstream_destructor);
    sq_setinstanceup(g_VM, -1, (SQUserPointer)stream);
    stream->grab(); // grab a new reference
}

//-----------------------------------------------------------------
ZStream* GetZStream(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_ZSTREAM))) {
        return (ZStream*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_zstream_methods[] = {
    {"constructor",     "ZStream.constructor",      script_zstream_constructor      },
    {"getBufferSize",   "ZStream.getBufferSize",    script_zstream_getBufferSize    },
    {"setBufferSize",   "ZStream.setBufferSize",    script_zstream_setBufferSize    },
    {"compress",        "ZStream.compress",         script_zstream_compress         },
    {"decompress",      "ZStream.decompress",       script_zstream_decompress       },
    {"finish",          "ZStream.finish",           script_zstream_finish           },
    {"_typeof",         "ZStream._typeof",          script_zstream__typeof          },
    {"_tostring",       "ZStream._tostring",        script_zstream__tostring        },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptFuncReg script_zstream_static_methods[] = {
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_zstream_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void init_zstream_class()
{
    sq_newclass(g_VM, SQFalse); // create class
    sq_settypetag(g_VM, -1, TT_ZSTREAM);

    // define methods
    for (int i = 0; script_zstream_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_zstream_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_zstream_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_zstream_methods[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // define static methods
    for (int i = 0; script_zstream_static_methods[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_zstream_static_methods[i].funcname, -1);
        sq_newclosure(g_VM, script_zstream_static_methods[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_zstream_static_methods[i].debugname);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // define constants
    for (int i = 0; script_zstream_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_zstream_constants[i].name, -1);
        sq_pushinteger(g_VM, script_zstream_constants[i].value);
        sq_newslot(g_VM, -3, SQTrue);
    }

    // get a strong reference
    sq_resetobject(&g_ZStreamClass);
    sq_getstackobj(g_VM, -1, &g_ZStreamClass);
    sq_addref(g_VM, &g_ZStreamClass);

    sq_poptop(g_VM); // pop class
}

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
static ScriptFuncReg script_utility_functions[] = {
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_utility_constants[] = {
    {0,0}
};

//-----------------------------------------------------------------
static void register_utility_api()
{
    // init classes
    init_zstream_class();

    // register classes
    sq_pushstring(g_VM, "ZStream", -1);
    sq_pushobject(g_VM, g_ZStreamClass);
    sq_newslot(g_VM, -3, SQFalse);

    // register functions
    for (int i = 0; script_utility_functions[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_utility_functions[i].funcname, -1);
        sq_newclosure(g_VM, script_utility_functions[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_utility_functions[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // register constants
    for (int i = 0; script_utility_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_utility_constants[i].name, -1);
        sq_pushinteger(g_VM, script_utility_constants[i].value);
        sq_newslot(g_VM, -3, SQFalse);
    }
}

/******************************************************************
 *                                                                *
 *                      GENERAL FUNCTIONS                         *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------------------
// Assert(expr)
static SQInteger script_Assert(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    SQBool expr;
    if (sq_gettype(v, 2) == OT_BOOL) {
        sq_getbool(v, 2, &expr);
    } else {
        sq_tobool(v, 2, &expr);
    }
    if (expr != SQTrue) {
        THROW_ERROR("Assertion failed");
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// GetTime()
static SQInteger script_GetTime(HSQUIRRELVM v)
{
    RET_INT(GetTime())
}

//-----------------------------------------------------------------
// GetTimeInfo([time, utc])
static SQInteger script_GetTimeInfo(HSQUIRRELVM v)
{
    GET_OPTARG_INT(1, time, GetTime())
    GET_OPTARG_BOOL(2, utc, SQFalse)

    TimeInfo timeinfo = GetTimeInfo(time, (utc == SQTrue ? true : false));

    sq_newtable(v);

    sq_pushstring(v, "second", -1);
    sq_pushinteger(v, timeinfo.second);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "minute", -1);
    sq_pushinteger(v, timeinfo.minute);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "hour", -1);
    sq_pushinteger(v, timeinfo.hour);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "day", -1);
    sq_pushinteger(v, timeinfo.day);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "month", -1);
    sq_pushinteger(v, timeinfo.month);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "year", -1);
    sq_pushinteger(v, timeinfo.year);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "weekday", -1);
    sq_pushinteger(v, timeinfo.weekday);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "yearday", -1);
    sq_pushinteger(v, timeinfo.yearday);
    sq_newslot(v, -3, SQFalse);

    return 1;
}

//-----------------------------------------------------------------
// GetTicks()
static SQInteger script_GetTicks(HSQUIRRELVM v)
{
    RET_INT(GetTicks())
}

//-----------------------------------------------------------------
// Random()
static SQInteger script_Random(HSQUIRRELVM v)
{
    RET_FLOAT(GetRandom())
}

//-----------------------------------------------------------------
// Sleep(ms)
static SQInteger script_Sleep(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, ms)
    ThreadSleep(ms);
    RET_VOID()
}

//-----------------------------------------------------------------
bool CompileBuffer(const void* buffer, int size, const std::string& scriptName)
{
    assert(buffer);
    assert(size > 0);
    return SQ_SUCCEEDED(sq_compilebuffer(g_VM, (const SQChar*)buffer, size, scriptName.c_str(), SQTrue));
}

//-----------------------------------------------------------------
// CompileString(string [, scriptName = "unknown"])
static SQInteger script_CompileString(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STRING(1, str)
    GET_OPTARG_STRING(2, scriptName, "unknown")
    int len = sq_getsize(g_VM, 2);
    if (len == 0) {
        THROW_ERROR("Empty string")
    }
    if (!CompileBuffer(str, len, scriptName)) {
        THROW_ERROR1("Could not compile string: %s", g_LastCompileError.c_str())
    }
    return 1;
}

//-----------------------------------------------------------------
// CompileBlob(blob [, scriptName = "unknown", offset = 0, count = -1])
static SQInteger script_CompileBlob(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_BLOB(1, blob)
    GET_OPTARG_STRING(2, scriptName, "unknown")
    GET_OPTARG_INT(3, offset, 0)
    GET_OPTARG_INT(4, count, -1)
    if (blob->getSize() == 0) {
        THROW_ERROR("Empty blob")
    }
    if (offset < 0 || offset >= blob->getSize()) {
        THROW_ERROR("Invalid offset")
    }
    if (count < 0) {
        count = blob->getSize() - offset;
    } else if (count == 0 || count > blob->getSize() - offset) {
        THROW_ERROR("Invalid count")
    }
    if (!CompileBuffer(blob->getBuffer() + offset, count, scriptName)) {
        THROW_ERROR1("Could not compile blob: %s", g_LastCompileError.c_str())
    }
    return 1;
}

//-----------------------------------------------------------------
static SQInteger lexfeed_callback(SQUserPointer p)
{
    char c;
    IStream* stream = (IStream*)p;
    if (stream->read(&c, sizeof(c)) == sizeof(c)) {
        return c;
    }
    return 0;
}

//-----------------------------------------------------------------
struct STREAMSOURCE {
    IStream* stream;
    int remaining;
};

static SQInteger lexfeed_callback_ex(SQUserPointer p)
{
    char c;
    STREAMSOURCE* ss = (STREAMSOURCE*)p;
    if (ss->remaining > 0 && ss->stream->read(&c, sizeof(c)) == sizeof(c)) {
        ss->remaining -= sizeof(c);
        return c;
    }
    return 0;
}

//-----------------------------------------------------------------
bool CompileStream(IStream* stream, const std::string& scriptName, int count)
{
    assert(stream);
    if (stream) {
        if (count > 0) {
            STREAMSOURCE ss = {stream, count};
            return SQ_SUCCEEDED(sq_compile(g_VM, lexfeed_callback_ex, &ss, scriptName.c_str(), SQTrue));
        } else {
            return SQ_SUCCEEDED(sq_compile(g_VM, lexfeed_callback, stream, scriptName.c_str(), SQTrue));
        }
    }
    return false;
}

//-----------------------------------------------------------------
// CompileStream(stream [, scriptName = "unknown", count = -1])
static SQInteger script_CompileStream(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STREAM(1, stream)
    GET_OPTARG_STRING(2, scriptName, "unknown")
    GET_OPTARG_INT(3, count, -1)
    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid stream")
    }
    if (count == 0) {
        THROW_ERROR("Invalid count")
    }
    if (!CompileStream(stream, scriptName, count)) {
        THROW_ERROR1("Could not compile stream: %s", g_LastCompileError.c_str())
    }
    return 1;
}

//-----------------------------------------------------------------
bool EvaluateScript(const std::string& filename)
{
    int old_top = sq_gettop(g_VM); // save stack top

    // load script
    if (DoesFileExist(filename)) {
        FilePtr file = OpenFile(filename);
        if (LoadObject(file.get())) { // try loading as bytecode
            if (sq_gettype(g_VM, -1) != OT_CLOSURE) { // make sure the file actually contained a compiled script
                sq_settop(g_VM, old_top); // restore stack top
                return false;
            }
        } else { // try compiling as plain text
            file->seek(0); // LoadObject changed the stream position, set it back to beginning
            if (!CompileStream(file.get(), file->getName())) {
                sq_settop(g_VM, old_top); // restore stack top
                return false;
            }
        }
    } else { // file does not exist
        return false;
    }

    // execute script
    sq_pushroottable(g_VM); // this
    if (!SQ_SUCCEEDED(sq_call(g_VM, 1, SQFalse, SQTrue))) {
        sq_settop(g_VM, old_top); // restore stack top
        return false;
    }

    sq_settop(g_VM, old_top); // restore stack top
    return true;
}

//-----------------------------------------------------------------
// EvaluateScript(name)
static SQInteger script_EvaluateScript(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, name)
    if (sq_getsize(v, 2) == 0) {
        THROW_ERROR("Empty script name")
    }

    // complement path
    std::string script = name;
    if (!ComplementPath(script)) {
        THROW_ERROR("Invalid path")
    }

    // evaluate script
    if (!EvaluateScript(script)) {
        THROW_ERROR1("Could not evaluate script '%s'", script.c_str())
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// RequireScript(name)
static SQInteger script_RequireScript(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, name)
    if (sq_getsize(v, 2) == 0) {
        THROW_ERROR("Empty script name")
    }

    // complement path
    std::string script = name;
    if (!ComplementPath(script)) {
        THROW_ERROR1("Invalid path '%s'", script.c_str())
    }

    // see if the script has already been loaded
    for (int i = 0; i < (int)g_ScriptRegistry.size(); ++i) {
        if (g_ScriptRegistry[i] == script) {
            RET_VOID() // already loaded, nothing to do here
        }
    }

    // evaluate script
    if (!EvaluateScript(script + BYTECODE_FILE_EXT) && // prefer bytecode
        !EvaluateScript(script +   SCRIPT_FILE_EXT))
    {
        THROW_ERROR1("Could not evaluate script '%s'", script.c_str())
    }

    // register script
    g_ScriptRegistry.push_back(script);

    RET_VOID()
}

//-----------------------------------------------------------------
// GetLoadedScripts()
static SQInteger script_GetLoadedScripts(HSQUIRRELVM v)
{
    sq_newarray(v, g_ScriptRegistry.size());
    for (int i = 0; i < (int)g_ScriptRegistry.size(); i++) {
        sq_pushinteger(v, i);
        sq_pushstring(v, g_ScriptRegistry[i].c_str(), -1);
        sq_rawset(v, -3);
    }
    return 1;
}

//-----------------------------------------------------------------
bool JSONStringify(SQInteger idx)
{
    if (idx < 0) { // make any negative indices positive to reduce complexity
        idx = (sq_gettop(g_VM) + 1) + idx;
    }
    switch (sq_gettype(g_VM, idx)) {
    case OT_NULL: {
        sq_pushstring(g_VM, "null", -1);
        return true;
    }
    case OT_BOOL: {
        SQBool b;
        sq_getbool(g_VM, idx, &b);
        sq_pushstring(g_VM, ((b == SQTrue) ? "true" : "false"), -1);
        return true;
    }
    case OT_INTEGER: {
        SQInteger i;
        sq_getinteger(g_VM, idx, &i);
        std::ostringstream oss;
        oss << i;
        sq_pushstring(g_VM, oss.str().c_str(), -1);
        return true;
    }
    case OT_FLOAT: {
        SQFloat f;
        sq_getfloat(g_VM, idx, &f);
        std::ostringstream oss;
        oss << f;
        sq_pushstring(g_VM, oss.str().c_str(), -1);
        return true;
    }
    case OT_STRING: {
        const SQChar* s = 0;
        sq_getstring(g_VM, idx, &s);
        std::ostringstream oss;
        oss << "\"" << s << "\"";
        sq_pushstring(g_VM, oss.str().c_str(), -1);
        return true;
    }
    case OT_TABLE: {
        const SQChar* temp = 0;
        int oldtop = sq_gettop(g_VM);
        std::ostringstream oss;
        oss << "{";
        sq_pushnull(g_VM); // will be substituted with an iterator by squirrel
        bool appendcomma = false;
        while (SQ_SUCCEEDED(sq_next(g_VM, idx))) {
            if (appendcomma) {
                oss << ",";
            } else {
                appendcomma = true;
            }
            int top = sq_gettop(g_VM);
            if (sq_gettype(g_VM, top-1) != OT_STRING) { // key must be a string
                sq_settop(g_VM, oldtop);
                return false;
            }
            sq_getstring(g_VM, top-1, &temp);
            oss << "\"" << temp << "\":";
            if (sq_gettype(g_VM, top) == OT_STRING) {
                sq_getstring(g_VM, top, &temp);
                oss << "\"" << temp << "\"";
            } else {
                if (!JSONStringify(top)) { // stringify value
                    sq_settop(g_VM, oldtop);
                    return false;
                }
                sq_getstring(g_VM, top+1, &temp);
                oss << temp;
                sq_poptop(g_VM); // pop json value
            }
            sq_pop(g_VM, 2); // pop key, value
        }
        sq_poptop(g_VM); // pop iterator
        oss << "}";
        sq_pushstring(g_VM, oss.str().c_str(), -1);
        return true;
    }
    case OT_ARRAY: {
        const SQChar* temp = 0;
        int oldtop = sq_gettop(g_VM);
        std::ostringstream oss;
        oss << "[";
        sq_pushnull(g_VM); // will be substituted with an iterator by squirrel
        bool appendcomma = false;
        while (SQ_SUCCEEDED(sq_next(g_VM, idx))) {
            if (appendcomma) {
                oss << ",";
            } else {
                appendcomma = true;
            }
            if (sq_gettype(g_VM, idx+3) == OT_STRING) {
                sq_getstring(g_VM, idx+3, &temp);
                oss << "\"" << temp << "\"";
            } else {
                if (!JSONStringify(idx+3)) { // stringify value
                    sq_settop(g_VM, oldtop);
                    return false;
                }
                sq_getstring(g_VM, idx+4, &temp);
                oss << temp;
                sq_poptop(g_VM); // pop json value
            }
            sq_pop(g_VM, 2); // pop key, value
        }
        sq_poptop(g_VM); // pop iterator
        oss << "]";
        sq_pushstring(g_VM, oss.str().c_str(), -1);
        return true;
    }
    default:
        return false;
    }
}

//-----------------------------------------------------------------
// JSONStringify(object)
static SQInteger script_JSONStringify(HSQUIRRELVM v)
{
    if (!JSONStringify(2)) {
        THROW_ERROR("Could not stringify object")
    }
    return 1;
}

//-----------------------------------------------------------------
bool JSONParse(const char* jsonstr)
{
    assert(jsonstr);
    int oldtop = sq_gettop(g_VM);
    // compile string
    std::string src = std::string("return ") + jsonstr;
    if (!CompileBuffer(src.c_str(), src.length())) {
        return false;
    }
    // execute closure returning the object
    sq_pushroottable(g_VM); // this
    if (!SQ_SUCCEEDED(sq_call(g_VM, 1, SQTrue, SQFalse))) {
        sq_settop(g_VM, oldtop);
        return false;
    }
    // the object is now on top of the stack
    int numtopop = (sq_gettop(g_VM) - oldtop) - 1;
    while (numtopop > 0) {
        sq_remove(g_VM, -2);
        numtopop--;
    }
    return true;
}

//-----------------------------------------------------------------
// JSONParse(jsonstr)
static SQInteger script_JSONParse(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, jsonstr)
    if (sq_getsize(v, 2) == 0) {
        THROW_ERROR("Empty JSON string")
    }
    if (!JSONParse(jsonstr)) {
        THROW_ERROR("Could not parse JSON string")
    }
    return 1;
}

//-----------------------------------------------------------------
static SQInteger write_closure_callback(SQUserPointer p, SQUserPointer buf, SQInteger count)
{
    IStream* stream = (IStream*)p;
    return stream->write(buf, count);
}

//-----------------------------------------------------------------
bool DumpObject(SQInteger idx, IStream* stream)
{
    assert(stream);
    if (!stream || !stream->isWriteable()) {
        return false;
    }
    if (idx < 0) { // make any negative indices positive to reduce complexity
        idx = (sq_gettop(g_VM) + 1) + idx;
    }
    switch (sq_gettype(g_VM, idx)) {
    case OT_NULL: {
        return writei32l(stream, MARSHAL_MAGIC_NULL);
    }
    case OT_INTEGER: {
        SQInteger i;
        sq_getinteger(g_VM, idx, &i);
        if (!writei32l(stream, MARSHAL_MAGIC_INTEGER) ||
            !writei32l(stream, (i32)i))
        {
            return false;
        }
        return true;
    }
    case OT_BOOL: {
        SQBool b;
        sq_getbool(g_VM, idx, &b);
        return writei32l(stream, ((b == SQTrue) ? MARSHAL_MAGIC_BOOL_TRUE : MARSHAL_MAGIC_BOOL_FALSE));
    }
    case OT_FLOAT: {
        SQFloat f;
        sq_getfloat(g_VM, idx, &f);
        if (!writei32l(stream, MARSHAL_MAGIC_FLOAT) ||
#ifdef SQUSEDOUBLE
            !writef64l(stream, (f64)f))
#else
            !writef32l(stream, (f32)f))
#endif
        {
            return false;
        }
        return true;
    }
    case OT_STRING: {
        const SQChar* s = 0;
        sq_getstring(g_VM, idx, &s);
        u32 numbytes = (u32)sq_getsize(g_VM, idx);
        if (!writei32l(stream, MARSHAL_MAGIC_STRING) ||
            !writei32l(stream, (i32)numbytes))
        {
            return false;
        }
        if (numbytes > 0) {
            return stream->write(s, numbytes) == numbytes;
        }
        return true;
    }
    case OT_TABLE: {
        int oldtop = sq_gettop(g_VM);
        if (!writei32l(stream, MARSHAL_MAGIC_TABLE) ||
            !writei32l(stream, (i32)sq_getsize(g_VM, idx)))
        {
            return false;
        }
        sq_pushnull(g_VM); // will be substituted with an iterator by squirrel
        while (SQ_SUCCEEDED(sq_next(g_VM, idx))) {
            if (!DumpObject(sq_gettop(g_VM)-1, stream) || // marshal key
                !DumpObject(sq_gettop(g_VM), stream))     // marshal value
            {
                sq_settop(g_VM, oldtop);
                return false;
            }
            sq_pop(g_VM, 2); // pop key and value
        }
        sq_poptop(g_VM); // pop iterator
        return true;
    }
    case OT_ARRAY: {
        int oldtop = sq_gettop(g_VM);
        if (!writei32l(stream, MARSHAL_MAGIC_ARRAY) ||
            !writei32l(stream, (i32)sq_getsize(g_VM, idx)))
        {
            return false;
        }
        sq_pushnull(g_VM); // will be substituted with an iterator by squirrel
        while (SQ_SUCCEEDED(sq_next(g_VM, idx))) {
            if (!DumpObject(sq_gettop(g_VM), stream)) { // marshal value
                sq_settop(g_VM, oldtop);
                return false;
            }
            sq_pop(g_VM, 2); // pop key and value
        }
        sq_poptop(g_VM); // pop iterator
        return true;
    }
    case OT_CLOSURE: {
        if (!writei32l(stream, MARSHAL_MAGIC_CLOSURE)) {
            return false;
        }
        sq_push(g_VM, idx);
        bool sqsucceeded = SQ_SUCCEEDED(sq_writeclosure(g_VM, write_closure_callback, stream));
        sq_poptop(g_VM);
        return sqsucceeded;
    }
    case OT_INSTANCE: {
        int oldtop = sq_gettop(g_VM);
        if (!writei32l(stream, MARSHAL_MAGIC_INSTANCE)) {
            return false;
        }
        sq_getclass(g_VM, idx);
        SQUserPointer tt = 0;
        sq_gettypetag(g_VM, -1, &tt);
#ifdef _SQ64
        if (!writei64l(stream, (i64)tt)) {
#else
        if (!writei32l(stream, (i32)tt)) {
#endif
            sq_settop(g_VM, oldtop);
            return false;
        }
        sq_pushstring(g_VM, "_dump", -1);
        if (!SQ_SUCCEEDED(sq_rawget(g_VM, -2))) {
            sq_settop(g_VM, oldtop);
            return false;
        }
        if (sq_gettype(g_VM, -1) != OT_CLOSURE &&
            sq_gettype(g_VM, -1) != OT_NATIVECLOSURE)
        {
            sq_settop(g_VM, oldtop);
            return false;
        }
        sq_pushroottable(g_VM); // this
        sq_push(g_VM, idx); // push the instance to marshal
        BindStream(stream); // push the output stream
        bool succeeded = SQ_SUCCEEDED(sq_call(g_VM, 3, SQFalse, SQTrue));
        sq_settop(g_VM, oldtop);
        return succeeded;
    }
    default:
        return false;
    }
}

//-----------------------------------------------------------------
// DumpObject(object, stream)
static SQInteger script_DumpObject(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)
    if (!stream->isOpen() || !stream->isWriteable()) {
        THROW_ERROR("Invalid stream")
    }
    if (!DumpObject(2, stream)) {
        THROW_ERROR("Error serializing")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
static SQInteger read_closure_callback(SQUserPointer p, SQUserPointer buf, SQInteger count)
{
    IStream* stream = (IStream*)p;
    return stream->read(buf, count);
}

//-----------------------------------------------------------------
bool LoadObject(IStream* stream)
{
    assert(stream);

    static ArrayPtr<char> s_Buffer;
    static int s_BufferSize = 0;
    if (s_BufferSize == 0) { // one-time initialization
        try {
            s_Buffer.reset(new char[256]);
            s_BufferSize = 256;
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return false;
        }
    }

    if (!stream || !stream->isReadable()) {
        return false;
    }

    // read type
    i32 type;
    if (!readi32l(stream, type)) {
        return false;
    }

    switch ((u32)type) {
    case MARSHAL_MAGIC_NULL: {
        sq_pushnull(g_VM);
        return true;
    }
    case MARSHAL_MAGIC_BOOL_TRUE: {
        sq_pushbool(g_VM, SQTrue);
        return true;
    }
    case MARSHAL_MAGIC_BOOL_FALSE: {
        sq_pushbool(g_VM, SQFalse);
        return true;
    }
    case MARSHAL_MAGIC_INTEGER: {
#ifdef _SQ64
        i64 i;
        if (!readi64l(stream, i)) {
#else
        i32 i;
        if (!readi32l(stream, i)) {
#endif
            return false;
        }
        sq_pushinteger(g_VM, (SQInteger)i);
        return true;
    }
    case MARSHAL_MAGIC_FLOAT: {
#ifdef SQUSEDOUBLE
        f64 f;
        if (!readf64l(stream, f)) {
#else
        f32 f;
        if (!readf32l(stream, f)) {
#endif
            return false;
        }
        sq_pushfloat(g_VM, f);
        return true;
    }
    case MARSHAL_MAGIC_STRING: {
        i32 len;
        if (!readi32l(stream, len)) {
            return false;
        }
        if (len < 0) {
            return false;
        } else if (len == 0) {
            sq_pushstring(g_VM, "", -1);
            return true; // ok, empty string
        }
        if (s_BufferSize < len) {
            try {
                s_Buffer.reset(new char[len]);
                s_BufferSize = len;
            } catch (const std::bad_alloc&) {
                ReportOutOfMemory();
                return false;
            }
        }
        if (stream->read(s_Buffer.get(), len) != len) {
            return false;
        }
        sq_pushstring(g_VM, (const SQChar*)s_Buffer.get(), len);
        return true;
    }
    case MARSHAL_MAGIC_TABLE: {
        int oldtop = sq_gettop(g_VM);
        i32 size;
        if (!readi32l(stream, size)) {
            return false;
        }
        if (size < 0) {
            return false;
        }
        sq_newtable(g_VM);
        for (int i = 0; i < size; ++i) {
            if (!LoadObject(stream) || // unmarshal key
                !LoadObject(stream))   // unmarshal value
            {
                sq_settop(g_VM, oldtop);
                return false;
            }
            sq_newslot(g_VM, -3, SQFalse);
        }
        return true;
    }
    case MARSHAL_MAGIC_ARRAY: {
        int oldtop = sq_gettop(g_VM);
        i32 size;
        if (!readi32l(stream, size)) {
            return false;
        }
        if (size < 0) {
            return false;
        }
        sq_newarray(g_VM, 0);
        for (int i = 0; i < size; ++i) {
            if (!LoadObject(stream)) { // unmarshal value
                sq_settop(g_VM, oldtop);
                return false;
            }
            sq_arrayappend(g_VM, -2);
        }
        return true;
    }
    case MARSHAL_MAGIC_CLOSURE: {
        return SQ_SUCCEEDED(sq_readclosure(g_VM, read_closure_callback, stream));
    }
    case MARSHAL_MAGIC_INSTANCE: {
        int oldtop = sq_gettop(g_VM);
#ifdef _SQ64
        i64 tt;
        if (!readi64l(stream, tt)) {
#else
        i32 tt;
        if (!readi32l(stream, tt)) {
#endif
            return false;
        }
        i32 len;
        if (!readi32l(stream, len)) {
            return false;
        }
        if (len <= 0) {
            return false;
        }
        if (s_BufferSize < len) {
            try {
                s_Buffer.reset(new char[len]);
                s_BufferSize = len;
            } catch (const std::bad_alloc&) {
                ReportOutOfMemory();
                return false;
            }
        }
        if (stream->read(s_Buffer.get(), len) != len) {
            return false;
        }
        sq_pushroottable(g_VM);
        sq_pushstring(g_VM, (const SQChar*)s_Buffer.get(), len);
        if (!SQ_SUCCEEDED(sq_get(g_VM, -2))) {
            sq_settop(g_VM, oldtop);
            return false;
        }
        if (sq_gettype(g_VM, -1) != OT_CLASS) {
            sq_settop(g_VM, oldtop);
            return false;
        }
        SQUserPointer class_tt = 0;
        sq_gettypetag(g_VM, -1, &class_tt);
        if ((SQUserPointer)tt != class_tt) {
            sq_settop(g_VM, oldtop);
            return false;
        }
        sq_pushstring(g_VM, "_load", -1);
        if (!SQ_SUCCEEDED(sq_rawget(g_VM, -2))) {
            sq_settop(g_VM, oldtop);
            return false;
        }
        if (sq_gettype(g_VM, -1) != OT_CLOSURE &&
            sq_gettype(g_VM, -1) != OT_NATIVECLOSURE)
        {
            sq_settop(g_VM, oldtop);
            return false;
        }
        sq_pushroottable(g_VM); // this
        BindStream(stream); // push the input stream
        if (!SQ_SUCCEEDED(sq_call(g_VM, 2, SQTrue, SQTrue))) {
            sq_settop(g_VM, oldtop);
            return false;
        }
        if (sq_gettype(g_VM, -1) != OT_INSTANCE) {
            sq_settop(g_VM, oldtop);
            return false;
        }
        int numtopop = (sq_gettop(g_VM) - oldtop) - 1;
        while (numtopop > 0) {
            sq_remove(g_VM, -2);
            numtopop--;
        }
        return true;
    }
    default:
        return false;
    }
}

//-----------------------------------------------------------------
// LoadObject(stream)
static SQInteger script_LoadObject(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STREAM(1, stream)
    if (!stream->isOpen() || !stream->isReadable()) {
        THROW_ERROR("Invalid stream")
    }
    if (!LoadObject(stream)) {
        THROW_ERROR("Error deserializing")
    }
    return 1;
}

//-----------------------------------------------------------------
static ScriptFuncReg script_system_functions[] = {
    {"Assert",              "Assert",           script_Assert           },
    {"GetTime",             "GetTime",          script_GetTime          },
    {"GetTimeInfo",         "GetTimeInfo",      script_GetTimeInfo      },
    {"GetTicks",            "GetTicks",         script_GetTicks         },
    {"Random",              "Random",           script_Random           },
    {"Sleep",               "Sleep",            script_Sleep            },
    {"CompileString",       "CompileString",    script_CompileString    },
    {"CompileBlob",         "CompileBlob",      script_CompileBlob      },
    {"CompileStream",       "CompileStream",    script_CompileStream    },
    {"EvaluateScript",      "EvaluateScript",   script_EvaluateScript   },
    {"RequireScript",       "RequireScript",    script_RequireScript    },
    {"GetLoadedScripts",    "GetLoadedScripts", script_GetLoadedScripts },
    {"JSONStringify",       "JSONStringify",    script_JSONStringify    },
    {"JSONParse",           "JSONParse",        script_JSONParse        },
    {"DumpObject",          "DumpObject",       script_DumpObject       },
    {"LoadObject",          "LoadObject",       script_LoadObject       },
    {0,0}
};

//-----------------------------------------------------------------
static ScriptConstReg script_system_constants[] = {
    {"INT_MIN",         INT_MIN         },
    {"INT_MAX",         INT_MAX         },
    {"SPHERE_MAJOR",    SPHERE_MAJOR    },
    {"SPHERE_MINOR",    SPHERE_MINOR    },
    {"SPHERE_PATCH",    SPHERE_PATCH    },
    {"SQUIRREL_MAJOR",   SQUIRREL_VERSION_NUMBER / 100          },
    {"SQUIRREL_MINOR",  (SQUIRREL_VERSION_NUMBER % 100) / 10    },
    {"SQUIRREL_PATCH",   SQUIRREL_VERSION_NUMBER % 10           },
    {0,0}
};

//-----------------------------------------------------------------
static void register_system_api()
{
    // register functions
    for (int i = 0; script_system_functions[i].funcname != 0; i++) {
        sq_pushstring(g_VM, script_system_functions[i].funcname, -1);
        sq_newclosure(g_VM, script_system_functions[i].funcptr, 0);
        sq_setnativeclosurename(g_VM, -1, script_system_functions[i].debugname);
        sq_newslot(g_VM, -3, SQFalse);
    }

    // register constants
    for (int i = 0; script_system_constants[i].name != 0; i++) {
        sq_pushstring(g_VM, script_system_constants[i].name, -1);
        sq_pushinteger(g_VM, script_system_constants[i].value);
        sq_newslot(g_VM, -3, SQFalse);
    }
}

/******************************************************************
 *                                                                *
 *                            INTERNAL                            *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------------------
static void compiler_error_handler(HSQUIRRELVM v, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
{
    std::ostringstream oss;
    oss << "Compile error in '";
    oss << source;
    oss << "' line ";
    oss << line;
    oss << " (";
    oss << column;
    oss << "): ";
    oss << desc;
    g_LastCompileError = oss.str();
}

//-----------------------------------------------------------------
static SQInteger runtime_error_handler(HSQUIRRELVM v)
{
    sq_tostring(v, 2); // stringify error
    const SQChar* error = 0;
    sq_getstring(v, -1, &error);
    g_LastRuntimeError = std::string("Unhandled exception: ") + error;
    sq_poptop(v); // pop error string
    return 0;
}

//-----------------------------------------------------------------
static void print_func(HSQUIRRELVM v, const SQChar* format, ...)
{
    static ArrayPtr<char> s_Buffer;
    static int s_BufferSize = 0;
    if (s_BufferSize == 0) { // one-time initialization
        try {
            s_Buffer.reset(new char[256]);
            s_BufferSize = 256;
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return;
        }
    }

    va_list arglist;
    va_start(arglist, format);

    int size = vsnprintf(s_Buffer.get(), s_BufferSize, format, arglist);
#ifdef _MSC_VER // VC's vsnprintf has different behavior than POSIX's vsnprintf
    while (size < 0 || size == s_BufferSize) { // buffer was not big enough to hold the output string + terminating null character
        try {
            s_Buffer.reset(new char[s_BufferSize * 2]); // double buffer size until vsnprintf succeeds
            s_BufferSize = s_BufferSize * 2;
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return;
        }
        va_start(arglist, format);
        size = vsnprintf(s_Buffer.get(), s_BufferSize, format, arglist);
    }
#else
    if (size < 0) { // formatting error occurred
        return;
    } else if (size >= s_BufferSize) { // buffer was not big enough to hold the output string + terminating null character
        try {
            s_Buffer.reset(new char[size + 1]); // allocate a big enough buffer and try again
            s_BufferSize = size + 1;
        } catch (const std::bad_alloc&) {
            ReportOutOfMemory();
            return;
        }
        va_start(arglist, format);
        vsnprintf(s_Buffer.get(), s_BufferSize, format, arglist);
    }
#endif
    va_end(arglist);

    static FILE* s_OutputFile = 0;
    if (!s_OutputFile) {
        s_OutputFile = fopen("output.txt", "w");
    }
    if (s_OutputFile) {
        fprintf(s_OutputFile, "%s\n", s_Buffer.get());
    }
}

//-----------------------------------------------------------------
bool InitScript(const Log& log)
{
    assert(!g_VM);

    // open squirrel vm
    g_VM = sq_open(1024);
    if (!g_VM) {
        log.error() << "Failed opening Squirrel VM";
        return false;
    }
    sq_setcompilererrorhandler(g_VM, compiler_error_handler);
    sq_newclosure(g_VM, runtime_error_handler, 0);
    sq_seterrorhandler(g_VM);
    sq_setprintfunc(g_VM, print_func, print_func);

    sq_pushroottable(g_VM); // push root table for registration

    // register squirrel libraries
    if (!SQ_SUCCEEDED(sqstd_register_mathlib(g_VM))) {
        log.error() << "Failed registering math library";
        goto lib_init_failed;
    }
    if (!SQ_SUCCEEDED(sqstd_register_stringlib(g_VM))) {
        log.error() << "Failed registering string library";
        goto lib_init_failed;
    }

    // register sphere API
    register_core_api();
    register_system_api();
    register_io_api();
    register_graphics_api();
    register_sound_api();
    register_input_api();
    register_utility_api();

    sq_poptop(g_VM); // done registering, pop root table

    return true;

lib_init_failed:
    sq_close(g_VM);
    g_VM = 0;
    return false;
}

//-----------------------------------------------------------------
void DeinitScript()
{
    if (g_VM) {
        sq_close(g_VM);
        g_VM = 0;
    }
}

//-----------------------------------------------------------------
void RunGame(const Log& log, const std::string& script, const std::vector<std::string>& args)
{
    int old_top = sq_gettop(g_VM); // save stack top

    // load script
    if (DoesFileExist(script + SCRIPT_FILE_EXT)) { // compile as plain-text script
        FilePtr file = OpenFile(script + SCRIPT_FILE_EXT);
        if (!CompileStream(file.get(), file->getName())) {
            log.error() << "Error compiling main script '" << script << "': " << g_LastCompileError;
            sq_settop(g_VM, old_top);
            return;
        }
    } else if (DoesFileExist(script + BYTECODE_FILE_EXT)) {
        FilePtr file = OpenFile(script + BYTECODE_FILE_EXT);
        if (!LoadObject(file.get()) || sq_gettype(g_VM, -1) != OT_CLOSURE) { // load as bytecode
            log.error() << "Error loading main script '" << script << "' as bytecode";
            sq_settop(g_VM, old_top);
            return;
        }
    } else { // script file does not exist
        log.error() << "Main script '" << script << "' not found";
        sq_settop(g_VM, old_top);
        return;
    }

    // run script
    sq_pushroottable(g_VM); // this
    if (!SQ_SUCCEEDED(sq_call(g_VM, 1, SQFalse, SQTrue))) {
        log.error() << "Error executing main script '" << script << "': " << g_LastRuntimeError;
        sq_settop(g_VM, old_top);
        return;
    }
    sq_settop(g_VM, old_top); // restore stack top

    // call main
    sq_pushroottable(g_VM);
    sq_pushstring(g_VM, "main", -1);
    if (!SQ_SUCCEEDED(sq_rawget(g_VM, -2))) {
        log.error() << "Entry point 'main' not defined in main script '" << script << "'";
        sq_settop(g_VM, old_top);
        return;
    }
    if (sq_gettype(g_VM, -1) != OT_CLOSURE) {
        log.error() << "Symbol 'main' is not a function in main script '" << script << "'";
        sq_settop(g_VM, old_top);
        return;
    }
    sq_pushroottable(g_VM); // this
    for (int i = 0; i < (int)args.size(); i++) {
        sq_pushstring(g_VM, args[i].c_str(), -1);
    }
    if (!SQ_SUCCEEDED(sq_call(g_VM, 1 + args.size(), SQFalse, SQTrue))) {
        log.error() << "Error calling 'main' in main script '" << script << "': " << g_LastRuntimeError;
    }
    sq_settop(g_VM, old_top);
}
