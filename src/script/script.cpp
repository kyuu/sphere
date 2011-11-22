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
#include "../Log.hpp"
#include "../error.hpp"
#include "../io/endian.hpp"
#include "../io/io.hpp"
#include "../io/Blob.hpp"
#include "../image/image.hpp"
#include "../system/system.hpp"
#include "../filesystem/filesystem.hpp"
#include "../video/video.hpp"
#include "../audio/audio.hpp"
#include "../input/input.hpp"
#include "script.hpp"
#include "typetags.hpp"
#include "macros.hpp"

#define   SCRIPT_FILE_EXT ".script"
#define BYTECODE_FILE_EXT ".bytecode"

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
// globals

// squirrel vm
static HSQUIRRELVM g_VM = 0;

// log instance used by the print function
static const Log* g_Log = 0;

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
    if (SQ_SUCCEEDED(sq_stackinfos(g_VM, 1, &si))) {
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

/***************************** RECT *******************************/

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

/***************************** VEC2 *******************************/

#define SETUP_VEC2_OBJECT() \
    Vec2i* This = 0; \
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) { \
        THROW_ERROR("Invalid type of environment object, expected a Vec2 instance") \
    }

//-----------------------------------------------------------------
// Vec2(x, y)
static SQInteger script_vec2_constructor(HSQUIRRELVM v)
{
    SETUP_VEC2_OBJECT()
    CHECK_NARGS(2)
    GET_ARG_INT(1, x)
    GET_ARG_INT(2, y)
    new (This) Vec2i(x, y);
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
    RET_INT(This->dot(*other))
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
    GET_ARG_INT(1, degrees)
    GET_OPTARG_VEC2(2, center)
    if (center) {
        This->rotateBy((float)degrees, *center);
    } else {
        This->rotateBy((float)degrees);
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
        RET_INT(This->x)
    } else if (strcmp(index, "y") == 0) {
        RET_INT(This->y)
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
        GET_ARG_INT(2, value)
        This->x = value;
    } else if (strcmp(index, "y") == 0) {
        GET_ARG_INT(2, value)
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
    new (This) Vec2i(*original);
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
    if (!writei32l(stream, instance->x) ||
        !writei32l(stream, instance->y))
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
    Vec2i instance;
    if (!readi32l(stream, instance.x) ||
        !readi32l(stream, instance.y))
    {
        THROW_ERROR("Read error")
    }

    RET_VEC2(instance)
}

//-----------------------------------------------------------------
void BindVec2(const Vec2i& vec)
{
    assert(g_Vec2Class._type == OT_CLASS);
    sq_pushobject(g_VM, g_Vec2Class); // push vec2 class
    sq_createinstance(g_VM, -1);
    sq_remove(g_VM, -2); // pop vec2 class
    SQUserPointer p = 0;
    sq_getinstanceup(g_VM, -1, &p, 0);
    assert(p);
    new (p) Vec2i(vec);
}

//-----------------------------------------------------------------
Vec2i* GetVec2(SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(g_VM, idx, &p, TT_VEC2))) {
        return (Vec2i*)p;
    }
    return 0;
}

/******************************************************************
 *                                                                *
 *                               IO                               *
 *                                                                *
 ******************************************************************/

/***************************** STREAM *****************************/

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
        RET_BLOB(blob.release())
    }
    blob->resize(size);
    if (This->read(blob->getBuffer(), size) != size) {
        THROW_ERROR("Read error")
    }
    RET_BLOB(blob.release())
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
static SQInteger script_stream_reads(HSQUIRRELVM v)
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
    int len = strlen(str); // assuming sizeof(SQChar) == 1
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
// Blob([size = 0])
static SQInteger script_blob_constructor(HSQUIRRELVM v)
{
    SETUP_BLOB_OBJECT()
    GET_OPTARG_INT(1, size, 0)
    if (size < 0) {
        THROW_ERROR("Invalid size")
    }
    This = Blob::Create(size);
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_blob_destructor);
    RET_VOID()
}

//-----------------------------------------------------------------
// Blob.FromString(string)
static SQInteger script_blob_FromString(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, string)
    BlobPtr blob = Blob::Create();
    int len = strlen(string);
    if (len > 0) {
        blob->assign(string, len);
    }
    RET_BLOB(blob.release())
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

/******************************************************************
 *                                                                *
 *                           GRAPHICS                             *
 *                                                                *
 ******************************************************************/

/***************************** COLOR ******************************/

//-----------------------------------------------------------------
// Color(red, green, blue [, alpha = 255])
static SQInteger script_color_constructor(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(3)
    GET_ARG_INT(1, red)
    GET_ARG_INT(2, green)
    GET_ARG_INT(3, blue)
    GET_OPTARG_INT(4, alpha, 255)
    RET_INT(RGBA::Pack(red, green, blue, alpha))
}

//-----------------------------------------------------------------
// Color.Red(color)
static SQInteger script_color_Red(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).red)
}

//-----------------------------------------------------------------
// Color.Green(color)
static SQInteger script_color_Green(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).green)
}

//-----------------------------------------------------------------
// Color.Blue(color)
static SQInteger script_color_Blue(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).blue)
}

//-----------------------------------------------------------------
// Color.Alpha(color)
static SQInteger script_color_Alpha(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, color)
    RET_INT(RGBA::Unpack((u32)color).alpha)
}

/**************************** CANVAS *****************************/

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

/******************************************************************
 *                                                                *
 *                          FILESYSTEM                            *
 *                                                                *
 ******************************************************************/

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
// OpenFile(filename [, mode = File.IN])
static SQInteger script_OpenFile(HSQUIRRELVM v)
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
        THROW_ERROR("Could not open file")
    }
    RET_FILE(file.get())
}

//-----------------------------------------------------------------
// DoesFileExist(filename)
static SQInteger script_Exists(HSQUIRRELVM v)
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
static SQInteger script_GetLastWriteTime(HSQUIRRELVM v)
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
static SQInteger script_Remove(HSQUIRRELVM v)
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
static SQInteger script_Rename(HSQUIRRELVM v)
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

/***************************** FILE *******************************/

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

/******************************************************************
 *                                                                *
 *                             VIDEO                              *
 *                                                                *
 ******************************************************************/

/*********************** GLOBAL FUNCTIONS *************************/

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
    if (width <= 0) {
        THROW_ERROR("Invalid width")
    }
    if (height <= 0) {
        THROW_ERROR("Invalid height")
    }
    if (!OpenWindow(width, height, (fullscreen == SQTrue ? true : false))) {

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
// SwapFrameBuffers()
static SQInteger script_SwapWindowBuffers(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    SwapFrameBuffers();
    RET_VOID()
}

//-----------------------------------------------------------------
// GetFrameBufferClipRect()
static SQInteger script_GetClipRect(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    Recti clip_rect;
    if (!GetFrameBufferClipRect(clip_rect)) {
        THROW_ERROR("Could not get frame buffer clipping rectangle")
    }
    RET_RECT(clip_rect)
}

//-----------------------------------------------------------------
// SetFrameBufferClipRect(clip_rect)
static SQInteger script_SetClipRect(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_NARGS(1)
    GET_ARG_RECT(1, clip_rect)
    SetFrameBufferClipRect(*clip_rect);
    RET_VOID()
}

//-----------------------------------------------------------------
// CloneFrameBufferSection(section)
static SQInteger script_CloneFrameBuffer(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_NARGS(1)
    GET_ARG_RECT(1, section)
    TexturePtr texture = CloneFrameBufferSection(*section);
    if (!texture) {
        THROW_ERROR("Could not clone frame buffer section")
    }
    RET_TEXTURE(texture.get())
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
    CHECK_MIN_NARGS(3)
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
// DrawTexturedTriangle(tex, tx1, ty1, tx2, ty2, tx3, ty3, x1, y1, x2, y2, x3, y3 [, mask = Color.New(255, 255, 255)])
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
// DrawRect(x, y, width, height, col [, col2, col3, col4])
static SQInteger script_DrawRect(HSQUIRRELVM v)
{
    if (!IsWindowOpen()) {
        THROW_ERROR("Invalid video state")
    }
    CHECK_MIN_NARGS(5)
    GET_ARG_INT(1, x)
    GET_ARG_INT(2, y)
    GET_ARG_INT(3, width)
    GET_ARG_INT(4, height)
    GET_ARG_INT(5, col1)
    GET_OPTARG_INT(6, col2, col1)
    GET_OPTARG_INT(7, col3, col1)
    GET_OPTARG_INT(8, col4, col1)
    RGBA colors[4] = {
        RGBA::Unpack((u32)col1),
        RGBA::Unpack((u32)col2),
        RGBA::Unpack((u32)col3),
        RGBA::Unpack((u32)col4),
    };
    DrawRect(Vec2i(x, y), width, height, colors);
    RET_VOID()
}

//-----------------------------------------------------------------
// DrawImage(image, pos [, mask = Color.New(255, 255, 255)])
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
// DrawSubImage(image, src_rect, x, y [, mask = Color.New(255, 255, 255)])
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
// DrawImageQuad(image, x1, y1, x2, y2, x3, y3, x4, y4 [, mask = Color.New(255, 255, 255)])
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
// DrawSubImageQuad(image, src_rect, x1, y1, x2, y2, x3, y3, x4, y4 [, mask = Color.New(255, 255, 255)])
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

/**************************** TEXTURE *****************************/

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
    GET_ARG_CANVAS(1, image)
    TexturePtr texture = CreateTexture(image);
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

/******************************************************************
 *                                                                *
 *                             AUDIO                              *
 *                                                                *
 ******************************************************************/

/***************************** SOUND ******************************/

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
    SETUP_SOUND_OBJECT()
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
    SETUP_SOUND_OBJECT()
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

/************************** SOUNDEFFECT ***************************/

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
            sq_pushstring(v, "mbutton", -1);
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

            sq_pushstring(v, "state", -1);
            sq_pushinteger(v, event.jhat.state);
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
// HasJoystickForceFeedback(joy)
static SQInteger script_HasJoystickForceFeedback(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    RET_BOOL(HasJoystickForceFeedback(joy))
}

//-----------------------------------------------------------------
// UploadJoystickForceEffect(joy, direction, duration, startLevel [, endLevel = startLevel])
static SQInteger script_UploadJoystickForceEffect(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(4)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, direction)
    GET_ARG_INT(3, duration)
    GET_ARG_INT(4, startLevel)
    GET_OPTARG_INT(5, endLevel, startLevel)
    int effect_id = UploadJoystickForceEffect(joy, direction, duration, startLevel, endLevel);
    if (effect_id < 0) {
        THROW_ERROR("Could not upload joystick force effect")
    }
    RET_INT(effect_id)
}

//-----------------------------------------------------------------
// PlayJoystickForceEffect(joy, effect [, times = 1])
static SQInteger script_PlayJoystickForceEffect(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, effect)
    GET_OPTARG_INT(3, times, 1)
    if (!PlayJoystickForceEffect(joy, effect, times)) {
        THROW_ERROR("Could not play joystick force effect")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// StopJoystickForceEffect(joy, effect)
static SQInteger script_StopJoystickForceEffect(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, effect)
    if (!StopJoystickForceEffect(joy, effect)) {
        THROW_ERROR("Could not stop joystick force effect")
    }
    RET_VOID()
}

//-----------------------------------------------------------------
// StopAllJoystickForceEffects(joy)
static SQInteger script_StopAllJoystickForceEffects(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, joy)
    StopAllJoystickForceEffects(joy);
    RET_VOID()
}

//-----------------------------------------------------------------
// RemoveJoystickForceEffect(joy, effect)
static SQInteger script_RemoveJoystickForceEffect(HSQUIRRELVM v)
{
    CHECK_NARGS(2)
    GET_ARG_INT(1, joy)
    GET_ARG_INT(2, effect)
    RemoveJoystickForceEffect(joy, effect);
    RET_VOID()
}

/******************************************************************
 *                                                                *
 *                             SYSTEM                             *
 *                                                                *
 ******************************************************************/

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
// GetTime()
static SQInteger script_GetTime(HSQUIRRELVM v)
{
    RET_INT(GetTime())
}

//-----------------------------------------------------------------
// GetTicks()
static SQInteger script_GetTicks(HSQUIRRELVM v)
{
    RET_INT(GetTicks())
}

//-----------------------------------------------------------------
// GetRandom()
static SQInteger script_GetRandom(HSQUIRRELVM v)
{
    RET_INT(GetRandom())
}

//-----------------------------------------------------------------
// Sleep(ms)
static SQInteger script_Sleep(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_INT(1, ms)
    Sleep(ms);
    RET_VOID()
}

/******************************************************************
 *                                                                *
 *                          COMPRESSION                           *
 *                                                                *
 ******************************************************************/

/**************************** ZSTREAM *****************************/

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

/******************************************************************
 *                                                                *
 *                              SCRIPT                            *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------------------
static bool compile_buffer(HSQUIRRELVM v, const std::string& name, const void* buffer, int size, bool raiseerror = true)
{
    assert(buffer);
    assert(size > 0);
    if (buffer && size > 0) {
        return SQ_SUCCEEDED(sq_compilebuffer(v, (const SQChar*)buffer, size, name.c_str(), (raiseerror ? SQTrue : SQFalse)));
    }
    return false;
}

//-----------------------------------------------------------------
// CompileString(source [, scriptName = "unknown"])
static SQInteger script_CompileString(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STRING(1, source)
    GET_OPTARG_STRING(2, scriptName, "unknown")
    int len = strlen(source);
    if (len == 0) {
        THROW_ERROR("Empty source")
    }
    if (!compile_buffer(v, scriptName, source, len)) {
        THROW_ERROR("Could not compile string")
    }
    return 1;
}

//-----------------------------------------------------------------
// CompileBlob(source [, scriptName = "unknown", offset = 0, count = -1])
static SQInteger script_CompileBlob(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_BLOB(1, source)
    GET_OPTARG_STRING(2, scriptName, "unknown")
    GET_OPTARG_INT(3, offset, 0)
    GET_OPTARG_INT(4, count, -1)
    if (source->getSize() == 0) {
        THROW_ERROR("Empty source")
    }
    if (offset < 0 || offset >= source->getSize()) {
        THROW_ERROR("Invalid offset")
    }
    if (count < 0) {
        count = source->getSize() - offset;
    } else if (count == 0 || count > source->getSize() - offset) {
        THROW_ERROR("Invalid count")
    }
    if (!compile_buffer(v, scriptName, source->getBuffer() + offset, count)) {
        THROW_ERROR("Could not compile blob")
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
static bool compile_stream(HSQUIRRELVM v, const std::string& name, IStream* stream, int size = -1, bool raiseerror = true)
{
    assert(stream);
    if (stream) {
        if (size > 0) {
            STREAMSOURCE ss = {stream, size};
            return SQ_SUCCEEDED(sq_compile(v, lexfeed_callback_ex, &ss, name.c_str(), (raiseerror ? SQTrue : SQFalse)));
        } else {
            return SQ_SUCCEEDED(sq_compile(v, lexfeed_callback, stream, name.c_str(), (raiseerror ? SQTrue : SQFalse)));
        }
    }
    return false;
}

//-----------------------------------------------------------------
// CompileStream(source [, scriptName = "unknown", count = -1])
static SQInteger script_CompileStream(HSQUIRRELVM v)
{
    CHECK_MIN_NARGS(1)
    GET_ARG_STREAM(1, source)
    GET_OPTARG_STRING(2, scriptName, "unknown")
    GET_OPTARG_INT(3, count, -1)
    if (!source->isOpen() || !source->isReadable()) {
        THROW_ERROR("Invalid source")
    }
    if (count == 0) {
        THROW_ERROR("Invalid count")
    }
    if (!compile_stream(v, scriptName, source, count)) {
        THROW_ERROR("Could not compile stream")
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
            if (!compile_stream(g_VM, file->getName().c_str(), file.get())) {
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
// RequireScript(name)
static SQInteger script_RequireScript(HSQUIRRELVM v)
{
    CHECK_NARGS(1)
    GET_ARG_STRING(1, name)
    if (strlen(name) == 0) {
        THROW_ERROR("Empty script name")
    }

    // complement path
    std::string script = name;
    if (!ComplementPath(script)) {
        THROW_ERROR("Invalid path")
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
        THROW_ERROR("Could not evaluate script")
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
        sq_pushinteger(v, i); // key
        sq_pushstring(v, g_ScriptRegistry[i].c_str(), -1);
        sq_rawset(v, -3);
    }
    return 1;
}

/******************************************************************
 *                                                                *
 *                        SERIALIZATION                           *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------------------
bool ObjectToJSON(SQInteger idx)
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
                if (!ObjectToJSON(top)) { // tojson value
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
                if (!ObjectToJSON(idx+3)) { // tojson value
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
// ObjectToJSON(object)
static SQInteger script_ObjectToJSON(HSQUIRRELVM v)
{
    if (!ObjectToJSON(2)) {
        THROW_ERROR("conversion to JSON failed")
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

/******************************************************************
 *                                                                *
 *                            INTERNAL                            *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------------------
static void compiler_error_handler(HSQUIRRELVM v, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
{
    if (g_Log) {
        g_Log->error() << "Script compile error in '" << source << "' line " << line << ": " << desc;
    }
}

//-----------------------------------------------------------------
static SQInteger runtime_error_handler(HSQUIRRELVM v)
{
    int top = sq_gettop(v);
    sq_tostring(v, 2);
    const SQChar* error = "N/A";
    sq_getstring(v, -1, &error);
    assert(g_Log);
    g_Log->error() << "Script runtime error: " << error;
    sq_settop(v, top);
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
    if (g_Log) {
        g_Log->debug() << "Script output: " << (const char*)s_Buffer.get();
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

    sq_pushroottable(g_VM); // push root table

    // register standard libraries
    if (!SQ_SUCCEEDED(sqstd_register_mathlib(g_VM))) {
        log.error() << "Failed registering math library";
        goto lib_init_failed;
    }
    if (!SQ_SUCCEEDED(sqstd_register_stringlib(g_VM))) {
        log.error() << "Failed registering string library";
        goto lib_init_failed;
    }

    // TODO register api

    sq_poptop(g_VM); // pop root table

    g_Log = &log; // will be used to output compiler and runtime script errors

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
    g_Log = 0;
}

//-----------------------------------------------------------------
void RunGame(const Log& log, const std::string& script, const std::vector<std::string>& args)
{
    int old_top = sq_gettop(g_VM); // save stack top

    // load script
    if (DoesFileExist(script)) {
        FilePtr file = OpenFile(script);
        if (LoadObject(file.get())) { // try loading as bytecode
            if (sq_gettype(g_VM, -1) != OT_CLOSURE) { // make sure the file actually contained a compiled script
                log.error() << "Error loading bytecode";
                sq_settop(g_VM, old_top);
                return;
            }
        } else { // try compiling as plain text
            file->seek(0); // LoadObject changed the stream position, set it back to beginning
            if (!compile_stream(g_VM, file->getName().c_str(), file.get())) {
                log.error() << "Error compiling script";
                sq_settop(g_VM, old_top);
                return;
            }
        }
    } else { // file does not exist
        log.error() << "Script file does not exist";
        sq_settop(g_VM, old_top);
        return;
    }

    // run script
    sq_pushroottable(g_VM); // this
    if (!SQ_SUCCEEDED(sq_call(g_VM, 1, SQFalse, SQTrue))) {
        log.error() << "Error executing script";
        sq_settop(g_VM, old_top);
        return;
    }
    sq_settop(g_VM, old_top); // restore stack top

    // call main
    sq_pushroottable(g_VM);
    sq_pushstring(g_VM, "main", -1);
    if (!SQ_SUCCEEDED(sq_rawget(g_VM, -2))) {
        log.error() << "Entry point main not defined";
        sq_settop(g_VM, old_top);
        return;
    }
    if (sq_gettype(g_VM, -1) != OT_CLOSURE) {
        log.error() << "Symbol main is not a function";
        sq_settop(g_VM, old_top);
        return;
    }
    sq_pushroottable(g_VM); // this
    for (int i = 0; i < (int)args.size(); i++) {
        sq_pushstring(g_VM, args[i].c_str(), -1);
    }
    if (!SQ_SUCCEEDED(sq_call(g_VM, 1 + args.size(), SQFalse, SQTrue))) {
        log.error() << "Error calling main";
    }
    sq_settop(g_VM, old_top);
}
