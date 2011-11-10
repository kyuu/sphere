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
#include "../core/Rect.hpp"
#include "../core/Vec2.hpp"
#include "../io/io.hpp"
#include "../graphics/graphics.hpp"
#include "../system/system.hpp"
#include "../filesystem/filesystem.hpp"
#include "../video/video.hpp"
#include "../audio/audio.hpp"
#include "../input/input.hpp"
#include "script.hpp"

// type tags
#define TT_STREAM       ((SQUserPointer)100)
#define TT_FILE         ((SQUserPointer)101)
#define TT_BLOB         ((SQUserPointer)102)
#define TT_CANVAS       ((SQUserPointer)103)
#define TT_RGBA         ((SQUserPointer)104)
#define TT_RECT         ((SQUserPointer)105)
#define TT_VEC2         ((SQUserPointer)106)
#define TT_TEXTURE      ((SQUserPointer)107)
#define TT_SOUND        ((SQUserPointer)108)
#define TT_SOUNDEFFECT  ((SQUserPointer)109)
#define TT_FORCEEFFECT  ((SQUserPointer)110)
#define TT_CIPHER       ((SQUserPointer)111)
#define TT_HASH         ((SQUserPointer)112)
#define TT_ZSTREAM      ((SQUserPointer)113)

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
Log* g_log = 0;
std::vector<std::string> g_loaded_scripts;
static HSQUIRRELVM g_sqvm = 0;
static HSQOBJECT g_stream_class;
static HSQOBJECT g_file_class;
static HSQOBJECT g_blob_class;
static HSQOBJECT g_canvas_class;
static HSQOBJECT g_rgba_class;
static HSQOBJECT g_rect_class;
static HSQOBJECT g_vec2_class;
static HSQOBJECT g_texture_class;
static HSQOBJECT g_sound_class;
static HSQOBJECT g_soundeffect_class;
static HSQOBJECT g_forceeffect_class;
static HSQOBJECT g_cipher_class;
static HSQOBJECT g_hash_class;
static HSQOBJECT g_zstream_class;

/******************************************************************
 *                                                                *
 *                             CORE                               *
 *                                                                *
 ******************************************************************/

/***************************** RECT *******************************/

//-----------------------------------------------------------------
// Rect(x1, y1, x2, y2)
// Rect(ul, lr)
static SQInteger script_rect_constructor(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    if (sq_gettype(v, 2) == OT_INSTANCE) { // assume the call is: Rect(ul, lr)
        // get ul
        if (!IsVec2(v, 2)) {
            return sq_throwerror(v, _SC("invalid type of parameter <ul>"));
        }
        Vec2i* ul = GetVec2(v, 2);
        assert(ul);

        // get lr
        if (!IsVec2(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <lr>"));
        }
        Vec2i* lr = GetVec2(v, 3);
        assert(lr);

        // initialize with two vectors
        new (This) Recti(*ul, *lr);

    } else { // assume the call is: Rect(x1, y1, x2, y2)
        // get x1
        SQInteger x1;
        if (sq_gettype(v, 2) & SQOBJECT_NUMERIC == 0) {
            return sq_throwerror(v, _SC("invalid type of parameter <x1>"));
        }
        sq_getinteger(v, 2, &x1);

        // get y1
        SQInteger y1;
        if (sq_gettype(v, 3) & SQOBJECT_NUMERIC == 0) {
            return sq_throwerror(v, _SC("invalid type of parameter <y1>"));
        }
        sq_getinteger(v, 3, &y1);

        // get x2
        SQInteger x2;
        if (sq_gettype(v, 4) & SQOBJECT_NUMERIC == 0) {
            return sq_throwerror(v, _SC("invalid type of parameter <x2>"));
        }
        sq_getinteger(v, 4, &x2);

        // get y2
        SQInteger y2;
        if (sq_gettype(v, 5) & SQOBJECT_NUMERIC == 0) {
            return sq_throwerror(v, _SC("invalid type of parameter <y2>"));
        }
        sq_getinteger(v, 5, &y2);

        // initialize with four integers
        new (This) Recti(x1, y1, x2, y2);
    }
    return 0;
}

//-----------------------------------------------------------------
// Rect.getWidth()
static SQInteger script_rect_getWidth(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getWidth());
    return 1;
}

//-----------------------------------------------------------------
// Rect.getHeight()
static SQInteger script_rect_getHeight(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getHeight());
    return 1;
}

//-----------------------------------------------------------------
// Rect.isValid()
static SQInteger script_rect_isValid(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(This->isValid());
    return 1;
}

//-----------------------------------------------------------------
// Rect.intersects(other)
static SQInteger script_rect_intersects(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get other
    if (!IsRect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <other>"));
    }
    Recti* other = GetRect(v, 2);
    assert(other);

    sq_pushbool(v, This->intersects(*other));
    return 1;
}

//-----------------------------------------------------------------
// Rect.getIntersection(other)
static SQInteger script_rect_getIntersection(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get other
    if (!IsRect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <other>"));
    }
    Recti* other = GetRect(v, 2);
    assert(other);

    if (!BindRect(This->getIntersection(*other))) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Rect.isPointInside(x, y)
// Rect.isPointInside(point)
static SQInteger script_rect_isPointInside(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    if (sq_gettype(v, 2) == OT_INSTANCE) { // assume the call is: Rect.isPointInside(point)
        // get point
        if (!IsVec2(v, 2)) {
            return sq_throwerror(v, _SC("invalid type of parameter <point>"));
        }
        Vec2i* point = GetVec2(v, 2);
        assert(point);

        sq_pushbool(v, This->isPointInside(*point));
    } else { // assume the call is: Rect.isPointInside(x, y)
        // get x
        SQInteger x;
        if (sq_gettype(v, 2) & SQOBJECT_NUMERIC == 0) {
            return sq_throwerror(v, _SC("invalid type of parameter <x>"));
        }
        sq_getinteger(v, 2, &x);

        // get y
        SQInteger y;
        if (sq_gettype(v, 3) & SQOBJECT_NUMERIC == 0) {
            return sq_throwerror(v, _SC("invalid type of parameter <y>"));
        }
        sq_getinteger(v, 3, &y);

        sq_pushbool(v, This->isPointInside(x, y));
    }
    return 1;
}

//-----------------------------------------------------------------
// Rect._get(index)
static SQInteger script_rect__get(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("ul")) == 0) {
        if (!BindVec2(v, This->ul)) {
            return sq_throwerror(v, _SC("internal error"));
        }
    } else if (scstrcmp(index, _SC("lr")) == 0) {
        if (!BindVec2(v, This->lr)) {
            return sq_throwerror(v, _SC("internal error"));
        }
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// Rect._set(index, value)
static SQInteger script_rect__set(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("ul")) == 0) {
        if (!IsVec2(v, 3)) {
            return sq_throwerror(v, _SC("invalid parameter <value>"));
        }
        Vec2i* ul = GetVec2(v, 3);
        assert(ul);

        This->ul = *ul;
    } else if (scstrcmp(index, _SC("lr")) == 0) {
        if (!IsVec2(v, 3)) {
            return sq_throwerror(v, _SC("invalid parameter <value>"));
        }
        Vec2i* lr = GetVec2(v, 3);
        assert(lr);

        This->lr = *lr;
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 0;
}

//-----------------------------------------------------------------
// Rect._typeof()
static SQInteger script_rect__typeof(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Rect"), -1);
    return 1;
}

//-----------------------------------------------------------------
// Rect._cloned(rect)
static SQInteger script_rect__cloned(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get original rect
    if (!IsRect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <rect>"));
    }
    Recti* orig = GetRect(v, 2);
    assert(orig);

    // clone rect
    new (This) Recti(*orig);
    return 0;
}

//-----------------------------------------------------------------
// Rect._tostring()
static SQInteger script_rect__tostring(HSQUIRRELVM v)
{
    Recti* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<Rect instance (" << This->ul.x;
    oss <<    ", " << This->ul.y;
    oss <<    ", " << This->lr.x;
    oss <<    ", " << This->lr.y;
    oss << ")>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// Rect._dump(rect, stream)
static SQInteger script_rect__dump(HSQUIRRELVM v)
{
    // get rect
    if (!IsRect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <rect>"));
    }
    Recti* rect = GetRect(v, 2);
    assert(rect);

    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // write class name
    const char* class_name = "Rect";
    int class_name_size = strlen(class_name);
    if (!writeu32l(stream, class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write rect
    if (!writeu32l(stream, (u32)rect->ul.x) || !writeu32l(stream, (u32)rect->ul.y) ||
        !writeu32l(stream, (u32)rect->lr.x) || !writeu32l(stream, (u32)rect->lr.y)) {
        goto throw_write_error;
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// Rect._load(stream)
static SQInteger script_rect__load(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // read rect
    Recti rect;
    if (!readu32l(stream, (u32)rect.ul.x) ||
        !readu32l(stream, (u32)rect.ul.y) ||
        !readu32l(stream, (u32)rect.lr.x) ||
        !readu32l(stream, (u32)rect.lr.y)) {
        return sq_throwerror(v, _SC("read error"));
    }

    if (!BindRect(v, rect) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
bool IsRect(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_RECT;
    }
    return false;
}

//-----------------------------------------------------------------
Recti* GetRect(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_RECT))) {
        return (Recti*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindRect(HSQUIRRELVM v, const Recti& rect)
{
    sq_pushobject(v, g_rect_class); // push rect class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop rect class
        return false;
    }
    sq_remove(v, -2); // pop rect class
    SQUserPointer p = 0;
    sq_getinstanceup(v, -1, &p, 0);
    assert(p);
    new (p) Recti(rect);
    return true;
}

/***************************** VEC2 *******************************/

//-----------------------------------------------------------------
// Vec2(x, y)
static SQInteger script_vec2_constructor(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get x
    SQInteger x;
    if (sq_gettype(v, 2) & SQOBJECT_NUMERIC == 0) {
        return sq_throwerror(v, _SC("invalid type of parameter <x>"));
    }
    sq_getinteger(v, 2, &x);

    // get y
    SQInteger y;
    if (sq_gettype(v, 3) & SQOBJECT_NUMERIC == 0) {
        return sq_throwerror(v, _SC("invalid type of parameter <y>"));
    }
    sq_getinteger(v, 3, &y);

    new (This) Vec2i(x, y);
    return 0;
}

//-----------------------------------------------------------------
// Vec2.len()
static SQInteger script_vec2_len(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushfloat(v, (SQFloat)This->len());
    return 1;
}

//-----------------------------------------------------------------
// Vec2.lenSQ()
static SQInteger script_vec2_lenSQ(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->lenSQ());
    return 1;
}

//-----------------------------------------------------------------
// Vec2.dot(other)
static SQInteger script_vec2_dot(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get other
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <other>"));
    }
    Vec2i* other = GetVec2(v, 2);
    assert(other);

    sq_pushinteger(v, This->dot(*other));
    return 1;
}

//-----------------------------------------------------------------
// Vec2.null()
static SQInteger script_vec2_null(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->null();
    return 0;
}

//-----------------------------------------------------------------
// Vec2.unit()
static SQInteger script_vec2_unit(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->unit();
    return 0;
}

//-----------------------------------------------------------------
// Vec2.distFrom(other)
static SQInteger script_vec2_distFrom(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get other
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <other>"));
    }
    Vec2i* other = GetVec2(v, 2);
    assert(other);

    sq_pushfloat(v, (SQFloat)This->distFrom(*other));
    return 1;
}

//-----------------------------------------------------------------
// Vec2.distFromSQ(other)
static SQInteger script_vec2_distFromSQ(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get other
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <other>"));
    }
    Vec2i* other = GetVec2(v, 2);
    assert(other);

    sq_pushinteger(v, This->distFromSQ(*other));
    return 1;
}

//-----------------------------------------------------------------
// Vec2.rotateBy(degrees [, center = Vec2(0, 0)])
static SQInteger script_vec2_rotateBy(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get degrees
    SQInteger degrees;
    if (sq_gettype(v, 2) & SQOBJECT_NUMERIC == 0) {
        return sq_throwerror(v, _SC("invalid type of parameter <degrees>"));
    }
    sq_getinteger(v, 2, &degrees);

    // get optional center
    Vec2i* center = 0;
    if (sq_gettop(v) >= 3) {
        if (!IsVec2(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <center>"));
        }
        center = GetVec2(v, 3);
        assert(center);
    }

    if (center) {
        This->rotateBy(degrees, *center);
    } else {
        This->rotateBy(degrees);
    }
    return 0;
}

//-----------------------------------------------------------------
// Vec2.getAngle()
static SQInteger script_vec2_getAngle(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushfloat(v, (SQFloat)This->getAngle());
    return 1;
}

//-----------------------------------------------------------------
// Vec2.getAngleWith(other)
static SQInteger script_vec2_getAngleWith(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get other
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <other>"));
    }
    Vec2i* other = GetVec2(v, 2);
    assert(other);

    sq_pushfloat(v, (SQFloat)This->getAngleWith(*other));
    return 1;
}

//-----------------------------------------------------------------
// Vec2.isBetween(a, b)
static SQInteger script_vec2_isBetween(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get a
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <a>"));
    }
    Vec2i* a = GetVec2(v, 2);
    assert(a);

    // get b
    if (!IsVec2(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <b>"));
    }
    Vec2i* b = GetVec2(v, 3);
    assert(b);

    sq_pushbool(v, This->isBetween(*a, *b));
    return 1;
}

//-----------------------------------------------------------------
// Vec2.getInterpolated(other, d)
static SQInteger script_vec2_getInterpolated(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get other
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <other>"));
    }
    Vec2i* other = GetVec2(v, 2);
    assert(other);

    // get d
    SQFloat d;
    if (sq_gettype(v, 3) != OT_FLOAT) {
        return sq_throwerror(v, _SC("invalid type of parameter <d>"));
    }
    sq_getfloat(v, 3, &d);

    if (!BindVec2(v, This->getInterpolated(*other, d))) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Vec2.interpolate(a, b, d)
static SQInteger script_vec2_interpolate(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get a
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <a>"));
    }
    Vec2i* a = GetVec2(v, 2);
    assert(a);

    // get b
    if (!IsVec2(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <b>"));
    }
    Vec2i* b = GetVec2(v, 3);
    assert(b);

    // get d
    SQFloat d;
    if (sq_gettype(v, 4) != OT_FLOAT) {
        return sq_throwerror(v, _SC("invalid type of parameter <d>"));
    }
    sq_getfloat(v, 4, &d);

    This->interpolate(*a, *b, d);
    return 0;
}

//-----------------------------------------------------------------
// Vec2._add(rhs)
static SQInteger script_vec2__add(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get rhs
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <rhs>"));
    }
    Vec2i* rhs = GetVec2(v, 2);
    assert(rhs);

    if (!BindVec2(v, *This + *rhs)) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Vec2._sub(rhs)
static SQInteger script_vec2__sub(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get rhs
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <rhs>"));
    }
    Vec2i* rhs = GetVec2(v, 2);
    assert(rhs);

    if (!BindVec2(v, *This - *rhs)) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Vec2._mul(scalar)
static SQInteger script_vec2__mul(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get scalar
    SQFloat scalar;
    if (sq_gettype(v, 2) & SQOBJECT_NUMERIC == 0) {
        return sq_throwerror(v, _SC("invalid type of parameter <scalar>"));
    }
    sq_getfloat(v, 2, &scalar);

    if (!BindVec2(v, *This * scalar)) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Vec2._get(index)
static SQInteger script_vec2__get(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("x")) == 0) {
        sq_pushinteger(v, This->x);
    } else if (scstrcmp(index, _SC("y")) == 0) {
        sq_pushinteger(v, This->y);
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// Vec2._set(index, value)
static SQInteger script_vec2__set(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    // get value
    SQInteger value;
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <value>"));
    }
    sq_getinteger(v, 3, &value);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("x")) == 0) {
        This->x = value;
    } else if (scstrcmp(index, _SC("y")) == 0) {
        This->y = value;
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 0;
}

//-----------------------------------------------------------------
// Vec2._typeof()
static SQInteger script_vec2__typeof(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Vec2"), -1);
    return 1;
}

//-----------------------------------------------------------------
// Vec2._cloned(vec2)
static SQInteger script_vec2__cloned(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get original vec2
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <vec2>"));
    }
    Vec2i* orig = GetVec2(v, 2);
    assert(orig);

    // clone vec2
    new (This) Vec2i(*orig);
    return 0;
}

//-----------------------------------------------------------------
// Vec2._tostring()
static SQInteger script_vec2__tostring(HSQUIRRELVM v)
{
    Vec2i* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<Vec2 instance (" << This->x;
    oss <<    ", " << This->y;
    oss << ")>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// Vec2._dump(vec2, stream)
static SQInteger script_vec2__dump(HSQUIRRELVM v)
{
    // get vec2
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <vec2>"));
    }
    Vec2i* vec2 = GetVec2(v, 2);
    assert(vec2);

    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // write class name
    const char* class_name = "Vec2";
    int class_name_size = strlen(class_name);
    if (!writeu32l(stream, class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write vec2
    if (!writeu32l(stream, (u32)vec2->x) ||
        !writeu32l(stream, (u32)vec2->y)) {
        goto throw_write_error;
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// Vec2._load(stream)
static SQInteger script_vec2__load(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // read vec2
    Vec2i vec2;
    if (!readu32l(stream, (u32)vec2.x) ||
        !readu32l(stream, (u32)vec2.y)) {
        return sq_throwerror(v, _SC("read error"));
    }

    if (!BindVec2(v, vec2) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
bool IsVec2(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_VEC2;
    }
    return false;
}

//-----------------------------------------------------------------
Vec2i* GetVec2(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_VEC2))) {
        return (Vec2i*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindVec2(HSQUIRRELVM v, const Vec2i& vec)
{
    sq_pushobject(v, g_vec2_class); // push vec2 class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop vec2 class
        return false;
    }
    sq_remove(v, -2); // pop vec2 class
    SQUserPointer p = 0;
    sq_getinstanceup(v, -1, &p, 0);
    assert(p);
    new (p) Vec2i(vec);
    return true;
}

/******************************************************************
 *                                                                *
 *                               IO                               *
 *                                                                *
 ******************************************************************/

/***************************** STREAM *****************************/

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
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isOpen());
    return 1;
}

//-----------------------------------------------------------------
// Stream.isReadable()
static SQInteger script_stream_isReadable(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isReadable());
    return 1;
}

//-----------------------------------------------------------------
// Stream.isWriteable()
static SQInteger script_stream_isWriteable(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isWriteable());
    return 1;
}

//-----------------------------------------------------------------
// Stream.close()
static SQInteger script_stream_close(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->close();
    return 0;
}

//-----------------------------------------------------------------
// Stream.tell()
static SQInteger script_stream_isReadable(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->tell());
    return 1;
}

//-----------------------------------------------------------------
// Stream.seek(offset [, origin])
static SQInteger script_stream_seek(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get offset
    SQInteger offset;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <offset>"));
    }
    sq_getinteger(v, 2, &offset);

    // get optional origin
    SQInteger origin = IStream::BEG;
    if (sq_gettop(v) >= 3) {
        if (sq_gettype(v, 3) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <origin>"));
        }
        sq_getinteger(v, 3, &origin);
    }

    sq_pushbool(v, This->seek(offset, origin));
    return 1;
}

//-----------------------------------------------------------------
// Stream.read(size)
static SQInteger script_stream_read(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // the return value
    BlobPtr blob = Blob::Create();
    if (!blob) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    // get size
    SQInteger size;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <size>"));
    }
    sq_getinteger(v, 2, &size);
    if (size < 0) {
        return sq_throwerror(v, _SC("invalid parameter <size>"));
    }
    if (size == 0) {
        goto return_blob;
    }

    // read
    blob->resize(size);
    if (This->read(blob->getBuffer(), size) != size) {
        return sq_throwerror(v, _SC("read error"));
    }

return_blob:
    if (!BindBlob(v, blob.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Stream.write(blob [, start, count])
static SQInteger script_stream_write(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get blob
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <blob>"));
    }
    Blob* blob = GetBlob(v, 2);
    assert(blob);
    if (blob->getSize() == 0) {
        return 0;
    }

    // get optional start
    SQInteger start = 0;
    if (sq_gettop(v) >= 3) {
        if (sq_gettype(v, 3) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <start>"));
        }
        sq_getinteger(v, 3, &start);
    }
    if (start < 0 || start >= blob->getSize()) {
        return sq_throwerror(v, _SC("invalid parameter <start>"));
    }

    // get optional count
    SQInteger count = blob->getSize();
    if (sq_gettop(v) >= 4) {
        if (sq_gettype(v, 4) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <count>"));
        }
        sq_getinteger(v, 4, &count);
    }
    if (count < 0 || start + count > blob->getSize()) {
        return sq_throwerror(v, _SC("invalid parameter <count>"));
    }
    if (count == 0) {
        return 0;
    }

    // write
    if (This->write(blob->getBuffer() + start, count) != size) {
        return sq_throwerror(v, _SC("write error"));
    }
    return 0;
}

//-----------------------------------------------------------------
// Stream.flush()
static SQInteger script_stream_flush(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->flush());
    return 1;
}

//-----------------------------------------------------------------
// Stream.eof()
static SQInteger script_stream_eof(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->eof());
    return 1;
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
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get type
    SQInteger type;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <type>"));
    }
    sq_getinteger(v, 2, &type);

    // get optional endian
    SQInteger endian = 'l';
    if (sq_gettop(v) >= 3 && sq_gettype(v, 3) == OT_INTEGER) {
        sq_getinteger(v, 3, &endian);
    }

    // read
    switch (endian) {
    case 'l':
        switch (type) {
        case 'c': {
            u8 n;
            if (!readu8(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i8)n);
        }
        case 'b': {
            u8 n;
            if (!readu8(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, n);
        }
        case 's': {
            u16 n;
            if (!readu16l(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i16)n);
        }
        case 'w': {
            u16 n;
            if (!readu16l(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, n);
        }
        case 'i': {
            u32 n;
            if (!readu32l(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i32)n);
        }
        case 'f': {
            f32 n;
            if (!readf32l(This, n)) {
                goto throw_read_error;
            }
            sq_pushfloat(v, n);
        }
        default:
            return sq_throwerror(v, _SC("invalid parameter <type>"));
        }
        break;
    case 'b':
        switch (type) {
        case 'c': {
            u8 n;
            if (!readu8(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i8)n);
            return 1;
        }
        case 'b': {
            u8 n;
            if (!readu8(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, n);
            return 1;
        }
        case 's': {
            u16 n;
            if (!readu16b(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i16)n);
            return 1;
        }
        case 'w': {
            u16 n;
            if (!readu16b(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, n);
            return 1;
        }
        case 'i': {
            u32 n;
            if (!readu32b(This, n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i32)n);
            return 1;
        }
        case 'f': {
            f32 n;
            if (!readf32b(This, n)) {
                goto throw_read_error;
            }
            sq_pushfloat(v, n);
            return 1;
        }
        default:
            return sq_throwerror(v, _SC("invalid parameter <type>"));
        }
        break;
    case 'h':
        switch (type) {
        case 'c': {
            u8 n;
            if (This->read(&n, sizeof(n)) != sizeof(n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i8)n);
        }
        case 'b': {
            u8 n;
            if (This->read(&n, sizeof(n)) != sizeof(n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, n);
        }
        case 's': {
            u16 n;
            if (This->read(&n, sizeof(n)) != sizeof(n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i16)n);
        }
        case 'w': {
            u16 n;
            if (This->read(&n, sizeof(n)) != sizeof(n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, n);
        }
        case 'i': {
            u32 n;
            if (This->read(&n, sizeof(n)) != sizeof(n)) {
                goto throw_read_error;
            }
            sq_pushinteger(v, (i32)n);
        }
        case 'f': {
            f32 n;
            if (This->read(&n, sizeof(n)) != sizeof(n)) {
                goto throw_read_error;
            }
            sq_pushfloat(v, n);
        }
        default:
            return sq_throwerror(v, _SC("invalid parameter <type>"));
        }
        break;
    default:
        return sq_throwerror(v, _SC("invalid parameter <endian>"));
    }
    return 1;

throw_read_error:
    return sq_throwerror(v, _SC("read error"));
}

//-----------------------------------------------------------------
// Stream.readString(length)
static SQInteger script_stream_reads(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get length
    SQInteger length;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <length>"));
    }
    sq_getinteger(v, 2, &length);
    if (length < 0) {
        return sq_throwerror(v, _SC("invalid parameter <length>"));
    }
    if (length == 0) {
        sq_pushstring(v, _SC(""), -1); // return an empty string
        return 1;
    }

    // prepare a buffer
    ArrayPtr<u8> buf = new u8[length]; // assuming sizeof(SQChar) == 1
    assert(buf.get());

    // read into the buffer
    if (This->read(buf.get(), length) != length) {
        return sq_throwerror(v, _SC("read error"));
    }

    sq_pushstring(v, (const SQChar*)buf.get(), length);
    return 1;
}

//-----------------------------------------------------------------
// Stream.writeNumber(type, number [, endian = 'l'])
static SQInteger script_stream_writeNumber(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get type
    SQInteger type;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <type>"));
    }
    sq_getinteger(v, 2, &type);

    // get optional endian
    SQInteger endian = 'l';
    if (sq_gettop(v) >= 4 && sq_gettype(v, 4) == OT_INTEGER) {
        sq_getinteger(v, 4, &endian);
    }

    switch (endian) {
    case 'l':
        if (type == 'f') {
            // get floating point number
            SQFloat n;
            if (sq_gettype(v, 3) != OT_FLOAT) {
                return sq_throwerror(v, _SC("invalid type of parameter <number>"));
            }
            sq_getfloat(v, 3, &n);

            // write
            if (!writef32l(This, (f32)n)) {
                goto throw_write_error;
            }
        } else {
            // get integer number
            SQInteger n;
            if (sq_gettype(v, 3) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <number>"));
            }
            sq_getinteger(v, 3, &n);

            // write
            switch (type) {
            case 'c': {
                if (!writeu8(This, (u8)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 'b': {
                if (!writeu8(This, (u8)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 's': {
                if (!writeu16l(This, (u16)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 'w': {
                if (!writeu16l(This, (u16)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 'i': {
                if (!writeu32l(This, (u32)n)) {
                    goto throw_write_error;
                }
            }
            default:
                return sq_throwerror(v, _SC("invalid parameter <type>"));
            }
        }
        break;
    case 'b':
        if (type == 'f') {
            // get floating point number
            SQFloat n;
            if (sq_gettype(v, 3) != OT_FLOAT) {
                return sq_throwerror(v, _SC("invalid type of parameter <n>"));
            }
            sq_getfloat(v, 3, &n);

            // write
            if (!writef32b(This, (f32)n)) {
                goto throw_write_error;
            }
        } else {
            // get integer number
            SQInteger n;
            if (sq_gettype(v, 3) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <n>"));
            }
            sq_getinteger(v, 3, &n);

            // write
            switch (type) {
            case 'c': {
                if (!writeu8(This, (u8)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 'b': {
                if (!writeu8(This, (u8)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 's': {
                if (!writeu16b(This, (u16)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 'w': {
                if (!writeu16b(This, (u16)n)) {
                    goto throw_write_error;
                }
            }
            break;
            case 'i': {
                if (!writeu32b(This, (u32)n)) {
                    goto throw_write_error;
                }
            }
            default:
                return sq_throwerror(v, _SC("invalid parameter <type>"));
            }
        }
        break;
    case 'h':
        if (type == 'f') {
            // get floating point number
            SQFloat f;
            if (sq_gettype(v, 3) != OT_FLOAT) {
                return sq_throwerror(v, _SC("invalid type of parameter <number>"));
            }
            sq_getfloat(v, 3, &f);

            // write
            f32 n = (f32)f;
            if (This->write(&n, sizeof(n)) != sizeof(n))) {
                goto throw_write_error;
            }
        } else {
            // get integer number
            SQInteger i;
            if (sq_gettype(v, 3) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <number>"));
            }
            sq_getinteger(v, 3, &i);

            // write
            switch (type) {
            case 'c': {
                i8 n = (i8)i;
                if (This->write(&n, sizeof(n)) != sizeof(n))) {
                    goto throw_write_error;
                }
            }
            break;
            case 'b': {
                u8 n = (u8)i;
                if (This->write(&n, sizeof(n)) != sizeof(n))) {
                    goto throw_write_error;
                }
            }
            break;
            case 's': {
                i16 n = (i16)i;
                if (This->write(&n, sizeof(n)) != sizeof(n))) {
                    goto throw_write_error;
                }
            }
            break;
            case 'w': {
                u16 n = (u16)i;
                if (This->write(&n, sizeof(n)) != sizeof(n))) {
                    goto throw_write_error;
                }
            }
            break;
            case 'i': {
                i32 n = (i32)i;
                if (This->write(&n, sizeof(n)) != sizeof(n))) {
                    goto throw_write_error;
                }
            }
            default:
                return sq_throwerror(v, _SC("invalid parameter <type>"));
            }
        }
        break;
    default:
        return sq_throwerror(v, _SC("invalid parameter <endian>"));
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// Stream.writeString(string)
static SQInteger script_stream_writeString(HSQUIRRELVM v)
{
    IStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get string
    const SQChar* str = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <string>"));
    }
    sq_getstring(v, 2, &str);

    // get string length
    SQInteger length = sq_getsize(v, 2);  // assuming sizeof(SQChar) == 1
    if (length > 0) {
        if (This->write(str, length) != length) {
            return sq_throwerror(v, _SC("write error"));
        }
    }
    return 0;
}

//-----------------------------------------------------------------
bool IsStream(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_STREAM;
    }
    return false;
}

//-----------------------------------------------------------------
IStream* GetStream(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_STREAM))) {
        return (IStream*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindStream(HSQUIRRELVM v, IStream* stream)
{
    if (!stream) {
        return false;
    }
    sq_pushobject(v, g_stream_class); // push stream class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop stream class
        return false;
    }
    sq_remove(v, -2); // pop stream class
    sq_setreleasehook(v, -1, script_stream_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)stream);
    stream->grab(); // grab a new reference
    return true;
}

//-----------------------------------------------------------------
bool CreateStreamDerivedClass(HSQUIRRELVM v)
{
    sq_pushobject(v, g_stream_class);
    if (!SQ_SUCCEEDED(sq_newclass(v, SQTrue))) {
        return false;
    }
    return true;
}

/***************************** BLOB *******************************/

//-----------------------------------------------------------------
static SQInteger script_blob_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((Blob*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Blob()
// Blob(size)
// Blob(string)
static SQInteger script_blob_constructor(HSQUIRRELVM v)
{
    // ensure that this function is only ever called on uninitialized Blob instances
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    BlobPtr blob = Blob::Create();
    if (!blob) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    if (sq_gettop(v) >= 2) {
        if (sq_gettype(v, 2) == OT_INTEGER) {
            SQInteger size;
            sq_getinteger(v, 2, &size);
            if (size < 0) {
                return sq_throwerror(v, _SC("invalid parameter <size>"));
            }
            if (!blob->resize(size)) {
                return sq_throwerror(v, _SC("out of memory"));
            }
        } else if (sq_gettype(v, 2) == OT_STRING) {
            SQInteger str_size = sq_getsize(v, 2);
            if (str_size > 0) {
                const SQChar* str = 0;
                sq_getstring(v, 2, &str);
                if (!blob->assign(str, str_size)) { // assuming sizeof(SQChar) == 1
                    return sq_throwerror(v, _SC("out of memory"));
                }
            }
        } else {
            return sq_throwerror(v, _SC("invalid type of initialization parameter"));
        }
    }

    sq_setinstanceup(v, 1, (SQUserPointer)blob.release());
    sq_setreleasehook(v, 1, script_blob_destructor);
    return 0;
}

//-----------------------------------------------------------------
// Blob.getSize()
static SQInteger script_blob_getSize(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getSize());
    return 1;
}

//-----------------------------------------------------------------
// Blob.getCapacity()
static SQInteger script_blob_getCapacity(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getCapacity());
    return 1;
}

//-----------------------------------------------------------------
// Blob.clear()
static SQInteger script_blob_clear(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->clear();
    return 0;
}

//-----------------------------------------------------------------
// Blob.reset([value])
static SQInteger script_blob_reset(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get optional value
    SQInteger value = 0;
    if (sq_gettop(v) >= 2) {
        if (sq_gettype(v, 2) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <value>"));
        }
        sq_getinteger(v, 2, &value);
    }

    This->reset((u8)value);
    return 0;
}

//-----------------------------------------------------------------
// Blob.resize(size)
static SQInteger script_blob_resize(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get size
    SQInteger size;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <size>"));
    }
    sq_getinteger(v, 2, &size);
    if (size < 0) {
        return sq_throwerror(v, _SC("invalid parameter <size>"));
    }

    This->resize(size);
    return 0;
}

//-----------------------------------------------------------------
// Blob.bloat()
static SQInteger script_blob_bloat(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->bloat();
    return 0;
}

//-----------------------------------------------------------------
// Blob.reserve(size)
static SQInteger script_blob_reserve(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    if (!This->reserve(size)) {
        return sq_throwerror(v, _SC("out of memory"));
    }
    return 0;
}

//-----------------------------------------------------------------
// Blob.doubleCapacity()
static SQInteger script_blob_doubleCapacity(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    if (!This->doubleCapacity()) {
        return sq_throwerror(v, _SC("out of memory"));
    }
    return 0;
}

//-----------------------------------------------------------------
// Blob.assign(blob)
static SQInteger script_blob_assign(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get blob
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <blob>"));
    }
    Blob* blob = GetBlob(v, 2);
    assert(blob);

    if (blob->getSize() > 0) {
        This->assign(blob->getBuffer(), blob->getSize());
    } else {
        This->clear();
    }
    return 0;
}

//-----------------------------------------------------------------
// Blob.append(blob)
static SQInteger script_blob_append(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get blob
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <blob>"));
    }
    Blob* blob = GetBlob(v, 2);
    assert(blob);

    if (blob->getSize() > 0) {
        This->append(blob->getBuffer(), blob->getSize());
    }
    return 0;
}

//-----------------------------------------------------------------
// Blob.concat(blob)
static SQInteger script_blob_concat(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get blob
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <blob>"));
    }
    Blob* blob = GetBlob(v, 2);
    assert(blob);

    BlobPtr result = This->concat(blob->getBuffer(), blob->getSize());
    if (!result || !BindBlob(v, result.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Blob._add(blob)
static SQInteger script_blob__add(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get blob
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of rhs parameter <blob>"));
    }
    Blob* blob = GetBlob(v, 2);
    assert(blob);

    // concat and return
    BlobPtr new_blob = This->concat(blob->getBuffer(), blob->getSize());
    if (!new_blob || !BindBlob(v, new_blob.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Blob._get(index)
static SQInteger script_blob__get(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    switch (sq_gettype(v, 2)) {
    case OT_INTEGER: { // integer index access
        // get integer index
        SQInteger index;
        sq_getinteger(v, 2, &index);
        if (index < 0 || index >= This->getSize()) {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }

        sq_pushinteger(v, This->at(index));
        return 1;
    }
    break;
    case OT_STRING: { // string index access
        // get string index
        const SQChar* index = 0;
        sq_getstring(v, 2, &index);

        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("size")) == 0) {
            sq_pushinteger(v, This->getSize());
        } else if (scstrcmp(index, _SC("capacity")) == 0) {
            sq_pushinteger(v, This->getCapacity());
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
    }
    break;
    default:
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Blob._set(index, value)
static SQInteger script_blob__set(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    switch (sq_gettype(v, 2)) {
    case OT_INTEGER: { // integer index access
        // get integer index
        SQInteger index;
        sq_getinteger(v, 2, &index);
        if (index < 0 || index >= This->getSize()) {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }

        // get value
        SQInteger value;
        if (sq_gettype(v, 3) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <value>"));
        }
        sq_getinteger(v, 3, &value);

        This->at(index) = (u8)value;
    }
    break;
    case OT_STRING: { // string index access
        // get string index
        const SQChar* index = 0;
        sq_getstring(v, 2, &index);

        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("size")) == 0) {
            // get value (the new size)
            SQInteger value;
            if (sq_gettype(v, 3) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <value>"));
            }
            sq_getinteger(v, 3, &value);
            if (value < 0) { // negative size is not allowed
                return sq_throwerror(v, _SC("invalid parameter <value>"));
            }

            if (!This->resize(value)) {
                return sq_throwerror(v, _SC("out of memory"));
            }
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
    }
    break;
    default:
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    return 0;
}

//-----------------------------------------------------------------
// Blob._typeof()
static SQInteger script_blob__typeof(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Blob"), -1);
    return 1;
}

//-----------------------------------------------------------------
// Blob._tostring()
static SQInteger script_blob__tostring(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<Blob instance (" << This->getSize() << ")>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// Blob._nexti(index)
static SQInteger script_blob__nexti(HSQUIRRELVM v)
{
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    switch (sq_gettype(v, 2)) {
    case OT_NULL: // start of iteration
        // return index 0
        sq_pushinteger(v, 0);
        return 1;
    case OT_INTEGER:
        // get previous index
        SQInteger prev_index;
        sq_getinteger(v, 2, &prev_index);

        if (previdx + 1 < This->getSize()) {
            sq_pushinteger(v, prev_index + 1); // return next index
        } else {
            sq_pushnull(v); // end of iteration
        }
        return 1;
    default:
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
}

//-----------------------------------------------------------------
// Blob._cloned(blob)
static SQInteger script_blob__cloned(HSQUIRRELVM v)
{
    // ensure that this function is only ever called on uninitialized Blob instances
    Blob* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    // get original blob
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <blob>"));
    }
    Blob* orig = GetBlob(v, 2);
    assert(orig);

    // clone blob
    This = Blob::Create(orig->getSize());
    if (!This) {
        return sq_throwerror(v, _SC("out of memory"));
    }
    if (orig->getSize() > 0) {
        memcpy(This->getBuffer(), orig->getBuffer(), orig->getSize());
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_blob_destructor);
    return 0;
}

//-----------------------------------------------------------------
// Blob._dump(blob, stream)
static SQInteger script_blob__dump(HSQUIRRELVM v)
{
    // get blob
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <blob>"));
    }
    Blob* blob = GetBlob(v, 2);
    assert(blob);

    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isOpen() || !stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // write class name
    const char* class_name = "Blob";
    int num_bytes = strlen(class_name);
    if (!writeu32l(stream, num_bytes) || stream->write(class_name, num_bytes) != num_bytes) {
        goto throw_write_error;
    }

    // write blob
    if (!writeu32l(stream, (u32)blob->getSize())) {
        goto throw_write_error;
    }
    if (blob->getSize() > 0 && stream->write(blob->getBuffer(), blob->getSize()) != blob->getSize()) {
        goto throw_write_error;
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// Blob._load(stream)
static SQInteger script_blob__load(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // read blob size
    u32 blob_size;
    if (!readu32l(stream, blob_size)) {
        goto throw_read_error;
    }

    BlobPtr blob = Blob::Create(blob_size);
    if (!blob) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    // read blob
    if (stream->read(blob->getBuffer(), blob_size) != blob_size) {
        goto throw_read_error;
    }

    if (!BindBlob(v, blob.get()) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;

throw_read_error:
    return sq_throwerror(v, _SC("read error"));
}

//-----------------------------------------------------------------
bool IsBlob(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_BLOB;
    }
    return false;
}

//-----------------------------------------------------------------
Blob* GetBlob(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_BLOB))) {
        return (Blob*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindBlob(HSQUIRRELVM v, Blob* blob)
{
    if (!blob) {
        return false;
    }
    sq_pushobject(v, g_blob_class); // push blob class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop blob class
        return false;
    }
    sq_remove(v, -2); // pop blob class
    sq_setreleasehook(v, -1, script_blob_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)blob);
    blob->grab(); // grab a new reference
    return true;
}

/******************************************************************
 *                                                                *
 *                           GRAPHICS                             *
 *                                                                *
 ******************************************************************/

/***************************** RGBA *******************************/

//-----------------------------------------------------------------
// RGBA(red, green, blue [, alpha = 255])
static SQInteger script_rgba_constructor(HSQUIRRELVM v)
{
    RGBA* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RGBA))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get red
    SQInteger red;
    if (sq_gettype(v, 2) & SQOBJECT_NUMERIC == 0) {
        return sq_throwerror(v, _SC("invalid type of parameter <red>"));
    }
    sq_getinteger(v, 2, &red);

    // get green
    SQInteger green;
    if (sq_gettype(v, 3) & SQOBJECT_NUMERIC == 0) {
        return sq_throwerror(v, _SC("invalid type of parameter <green>"));
    }
    sq_getinteger(v, 3, &green);

    // get blue
    SQInteger blue;
    if (sq_gettype(v, 4) & SQOBJECT_NUMERIC == 0) {
        return sq_throwerror(v, _SC("invalid type of parameter <blue>"));
    }
    sq_getinteger(v, 4, &blue);

    // get optional alpha
    SQInteger alpha = 255;
    if (sq_gettop(v) >= 5) {
        if (sq_gettype(v, 5) & SQOBJECT_NUMERIC == 0) {
            return sq_throwerror(v, _SC("invalid type of parameter <alpha>"));
        }
        sq_getinteger(v, 5, &alpha);
    }

    new (This) RGBA(red, green, blue, alpha);
    return 0;
}

//-----------------------------------------------------------------
// RGBA._get(index)
static SQInteger script_rgba__get(HSQUIRRELVM v)
{
    RGBA* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RGBA))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("red")) == 0) {
        sq_pushinteger(v, This->red);
    } else if (scstrcmp(index, _SC("green")) == 0) {
        sq_pushinteger(v, This->green);
    } else if (scstrcmp(index, _SC("blue")) == 0) {
        sq_pushinteger(v, This->blue);
    } else if (scstrcmp(index, _SC("alpha")) == 0) {
        sq_pushinteger(v, This->alpha);
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// RGBA._set(index, value)
static SQInteger script_rgba__set(HSQUIRRELVM v)
{
    RGBA* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RGBA))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    // get value
    SQInteger value;
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <value>"));
    }
    sq_getinteger(v, 3, &value);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("red")) == 0) {
        This->red = (u8)value;
    } else if (scstrcmp(index, _SC("green")) == 0) {
        This->green = (u8)value;
    } else if (scstrcmp(index, _SC("blue")) == 0) {
        This->blue = (u8)value;
    } else if (scstrcmp(index, _SC("alpha")) == 0) {
        This->alpha = (u8)value;
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 0;
}

//-----------------------------------------------------------------
// RGBA._typeof()
static SQInteger script_rgba__typeof(HSQUIRRELVM v)
{
    RGBA* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RGBA))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("RGBA"), -1);
    return 1;
}

//-----------------------------------------------------------------
// RGBA._cloned(rgba)
static SQInteger script_rgba__cloned(HSQUIRRELVM v)
{
    RGBA* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RGBA))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get original rgba
    if (!IsRGBA(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <rgba>"));
    }
    RGBA* orig = GetRGBA(v, 2);
    assert(orig);

    // clone rgba
    new (This) RGBA(*orig);
    return 0;
}

//-----------------------------------------------------------------
// RGBA._tostring()
static SQInteger script_rgba__tostring(HSQUIRRELVM v)
{
    RGBA* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RGBA))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<RGBA instance (" << (int)This->red;
    oss <<    ", " << (int)This->green;
    oss <<    ", " << (int)This->blue;
    oss <<    ", " << (int)This->alpha;
    oss << ")>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// RGBA._dump(rgba, stream)
static SQInteger script_rgba__dump(HSQUIRRELVM v)
{
    // get rgba
    if (!IsRGBA(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <rgba>"));
    }
    RGBA* rgba = GetRGBA(v, 2);
    assert(rgba);

    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // write class name
    const char* class_name = "RGBA";
    int class_name_size = strlen(class_name);
    if (!writeu32l(stream, class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write rgba
    if (stream->write(rgba, sizeof(RGBA)) != sizeof(RGBA)) {
        goto throw_write_error;
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// RGBA._load(stream)
static SQInteger script_rgba__load(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // read rgba
    RGBA rgba;
    if (stream->read(&rgba, sizeof(RGBA)) != sizeof(RGBA)) {
        return sq_throwerror(v, _SC("read error"));
    }

    if (!BindRGBA(v, rgba) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
bool IsRGBA(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_RGBA;
    }
    return false;
}

//-----------------------------------------------------------------
RGBA* GetRGBA(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_RGBA))) {
        return (RGBA*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindRGBA(HSQUIRRELVM v, const RGBA& col)
{
    sq_pushobject(v, g_rgba_class); // push rgba class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop rgba class
        return false;
    }
    sq_remove(v, -2); // pop rgba class
    SQUserPointer p = 0;
    sq_getinstanceup(v, -1, &p, 0);
    assert(p);
    new (p) RGBA(col);
    return true;
}

/**************************** CANVAS *****************************/

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
    // ensure that this function is only ever called on uninitialized Blob instances
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    // get width
    SQInteger width;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <width>"));
    }
    sq_getinteger(v, 2, &width);
    if (width <= 0) {
        return sq_throwerror(v, _SC("invalid parameter <width>"));
    }

    // get height
    SQInteger height;
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <height>"));
    }
    sq_getinteger(v, 3, &height);
    if (height <= 0) {
        return sq_throwerror(v, _SC("invalid parameter <height>"));
    }

    // get optional pixels
    Blob* pixels = 0;
    if (sq_gettop(v) >= 4) {
        if (!IsBlob(v, 4)) {
            return sq_throwerror(v, _SC("invalid type of parameter <pixels>"));
        }
        pixels = GetBlob(v, 4);
        assert(pixels);
        if (pixels->getSize() != width * height * Canvas::GetNumBytesPerPixel()) {
            return sq_throwerror(v, _SC("invalid parameter <pixels>"));
        }
    }

    This = Canvas::Create(width, height);
    if (!This) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    if (pixels) {
        memcpy(This->getPixels(), pixels->getBuffer(), pixels->getSize());
    }

    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_canvas_destructor);
    return 0;
}

//-----------------------------------------------------------------
// Canvas.load(filename)
static SQInteger script_canvas_load(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);

    sq_pushbool(v, This->load(filename));
    return 1;
}

//-----------------------------------------------------------------
// Canvas.save(filename)
static SQInteger script_canvas_save(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);

    sq_pushbool(v, This->save(filename));
    return 1;
}

//-----------------------------------------------------------------
// Canvas.getPixels()
static SQInteger script_canvas_getPixels(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    BlobPtr pixels = Blob::Create(This->getNumPixels() * Canvas::GetNumBytesPerPixel());
    if (!pixels) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    // copy pixels
    memcpy(pixels->getBuffer(), This->getPixels(), pixels->getSize());

    if (!BindBlob(v, blob.get()) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Canvas.cloneSection(section)
static SQInteger script_canvas_cloneSection(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get section
    if (!IsRect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <section>"));
    }
    Recti* section = GetRect(v, 2);
    assert(section);

    // clone
    CanvasPtr canvas = This->cloneSection(*section);
    if (!canvas) {
        return sq_throwerror(v, _SC("failed to clone section"));
    }

    if (!BindCanvas(v, canvas.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Canvas.getPixel(x, y)
static SQInteger script_canvas_getPixel(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get x
    SQInteger x;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <x>"));
    }
    sq_getinteger(v, 2, &x);
    if (x < 0 || x >= This->getWidth()) {
        return sq_throwerror(v, _SC("invalid parameter <x>"));
    }

    // get y
    SQInteger y;
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <y>"));
    }
    sq_getinteger(v, 3, &y);
    if (y < 0 || y >= This->getHeight()) {
        return sq_throwerror(v, _SC("invalid parameter <y>"));
    }

    if (!BindRGBA(v, This->getPixel(x, y))) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Canvas.setPixel(x, y, color)
static SQInteger script_canvas_setPixel(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get x
    SQInteger x;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <x>"));
    }
    sq_getinteger(v, 2, &x);
    if (x < 0 || x >= This->getWidth()) {
        return sq_throwerror(v, _SC("invalid parameter <x>"));
    }

    // get y
    SQInteger y;
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <y>"));
    }
    sq_getinteger(v, 3, &y);
    if (y < 0 || y >= This->getHeight()) {
        return sq_throwerror(v, _SC("invalid parameter <y>"));
    }

    // get color
    if (!IsRGBA(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <color>"));
    }
    RGBA* color = GetRGBA(v, 4);
    assert(color);

    This->setPixel(x, y, *color);
    return 0;
}

//-----------------------------------------------------------------
// Canvas.resize(width, height)
static SQInteger script_canvas_resize(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get width
    SQInteger width;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <width>"));
    }
    sq_getinteger(v, 2, &width);
    if (width <= 0) {
        return sq_throwerror(v, _SC("invalid parameter <width>"));
    }

    // get height
    SQInteger height;
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <height>"));
    }
    sq_getinteger(v, 3, &height);
    if (height <= 0) {
        return sq_throwerror(v, _SC("invalid parameter <height>"));
    }

    sq_pushbool(This->resize(width, height));
    return 1;
}

//-----------------------------------------------------------------
// Canvas.fill(color)
static SQInteger script_canvas_fill(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get color
    if (!IsRGBA(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <color>"));
    }
    RGBA* color = GetRGBA(v, 2);
    assert(color);

    This->fill(*color);
    return 0;
}

//-----------------------------------------------------------------
// Canvas._get(index)
static SQInteger script_canvas__get(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    switch (sq_gettype(v, 2)) {
    case OT_INTEGER: { // integer index access
        // get integer index
        SQInteger index;
        sq_getinteger(v, 2, &index);
        if (index < 0 || index >= This->getNumPixels()) {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }

        if (!BindRGBA(v, This->getPixelByIndex(index))) {
            return sq_throwerror(v, _SC("internal error"));
        }
        return 1;
    }
    break;
    case OT_STRING: { // string index access
        // get string index
        const SQChar* index = 0;
        sq_getstring(v, 2, &index);

        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("width")) == 0) {
            sq_pushinteger(v, This->getWidth());
        } else if (scstrcmp(index, _SC("height")) == 0) {
            sq_pushinteger(v, This->getHeight());
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        return 1;
    }
    break;
    default:
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
}

//-----------------------------------------------------------------
// Canvas._set(index, value)
static SQInteger script_canvas__set(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    switch (sq_gettype(v, 2)) {
    case OT_INTEGER: { // integer index access
        // get integer index
        SQInteger index;
        sq_getinteger(v, 2, &index);
        if (index < 0 || index >= This->getNumPixels()) {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }

        // get value (pixel color)
        if (!IsRGBA(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <value>"));
        }
        RGBA* color = GetRGBA(v, 3);
        assert(color);

        This->setPixelByIndex(index, *color);
    }
    break;
    default:
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    return 0;
}

//-----------------------------------------------------------------
// Canvas._typeof()
static SQInteger script_canvas__typeof(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Canvas"), -1);
    return 1;
}

//-----------------------------------------------------------------
// Canvas._tostring()
static SQInteger script_canvas__tostring(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<Canvas instance (" << This->getWidth() << ", " << This->getHeight() << ")>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// Canvas._nexti(index)
static SQInteger script_canvas__nexti(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    switch (sq_gettype(v, 2)) {
    case OT_NULL: // start of iteration
        // return index 0
        sq_pushinteger(v, 0);
        return 1;
    case OT_INTEGER:
        // get previous index
        SQInteger prev_index;
        sq_getinteger(v, 2, &prev_index);

        if (previdx + 1 < This->getNumPixels()) {
            sq_pushinteger(v, prev_index + 1); // return next index
        } else {
            sq_pushnull(v); // end of iteration
        }
        return 1;
    default:
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
}

//-----------------------------------------------------------------
// Canvas._cloned(canvas)
static SQInteger script_canvas__cloned(HSQUIRRELVM v)
{
    // ensure that this function is only ever called on uninitialized Blob instances
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    // get original canvas
    if (!IsCanvas(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <canvas>"));
    }
    Canvas* orig = GetCanvas(v, 2);
    assert(orig);

    // clone canvas
    This = Canvas::Create(orig->getPixels(), orig->getWidth(), orig->getHeight());
    if (!This) {
        return sq_throwerror(v, _SC("out of memory"));
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_canvas_destructor);
    return 0;
}

//-----------------------------------------------------------------
// Canvas._dump(canvas, stream)
static SQInteger script_canvas__dump(HSQUIRRELVM v)
{
    // get canvas
    if (!IsCanvas(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <canvas>"));
    }
    Canvas* canvas = GetCanvas(v, 2);
    assert(canvas);

    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isOpen() || !stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // write class name
    const char* class_name = "Canvas";
    int class_name_size = strlen(class_name);
    if (!writeu32l(stream, class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write canvas dimensions
    if (!writeu32l(stream, (u32)canvas->getWidth()) || !writeu32l(stream, (u32)canvas->getHeight()) {
        goto throw_write_error;
    }

    // write canvas pixels
    int pixels_size = canvas->getNumPixels() * Canvas::GetNumBytesPerPixel();
    if (stream->write(canvas->getPixels(), pixels_size) != pixels_size) {
        goto throw_write_error;
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// Canvas._load(stream)
static SQInteger script_canvas__load(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // read canvas dimensions
    u32 width;
    u32 height;
    if (!readu32l(stream, width) || !readu32l(stream, height)) {
        goto throw_read_error;
    }
    if (width * height <= 0) {
        return sq_throwerror(v, _SC("invalid dimensions"));
    }

    // create canvas
    CanvasPtr canvas = Canvas::Create(width, height);
    if (!canvas) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    // read canvas pixels
    int pixels_size = width * height * Canvas::GetNumBytesPerPixel();
    if (stream->read(canvas->getPixels(), pixels_size) != pixels_size) {
        goto throw_read_error;
    }

    if (!BindCanvas(v, canvas.get()) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;

throw_read_error:
    return sq_throwerror(v, _SC("read error"));
}

//-----------------------------------------------------------------
bool IsCanvas(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_CANVAS;
    }
    return false;
}

//-----------------------------------------------------------------
Canvas* GetCanvas(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_CANVAS))) {
        return (Canvas*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindCanvas(HSQUIRRELVM v, Canvas* canvas)
{
    if (!canvas) {
        return false;
    }
    sq_pushobject(v, g_canvas_class); // push canvas class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop canvas class
        return false;
    }
    sq_remove(v, -2); // pop canvas class
    sq_setreleasehook(v, -1, script_canvas_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)canvas);
    canvas->grab(); // grab a new reference
    return true;
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
    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filename>"));
    }

    // get optional mode
    SQInteger mode = IFile::IN;
    if (sq_gettop(v) >= 3) {
        if (sq_gettype(v, 3) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <mode>"));
        }
        sq_getinteger(v, 3, &mode);
        if (mode != IFile::IN &&
            mode != IFile::OUT &&
            mode != IFile::APPEND) {
            return sq_throwerror(v, _SC("invalid parameter <mode>"));
        }
    }

    // open
    IFile* file = OpenFile(filename, mode);
    if (!file) {
        return sq_throwerror(v, _SC("failed to open file"));
    }

    if (!BindFile(v, file)) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Exists(filename)
static SQInteger script_Exists(HSQUIRRELVM v)
{
    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filename>"));
    }

    sq_pushbool(v, Exists(filename));
    return 1;
}

//-----------------------------------------------------------------
// IsRegularFile(filename)
static SQInteger script_IsRegularFile(HSQUIRRELVM v)
{
    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filename>"));
    }

    sq_pushbool(v, IsRegularFile(filename));
    return 1;
}

//-----------------------------------------------------------------
// IsDirectory(filename)
static SQInteger script_IsDirectory(HSQUIRRELVM v)
{
    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filename>"));
    }

    sq_pushbool(v, IsDirectory(filename));
    return 1;
}

//-----------------------------------------------------------------
// GetLastWriteTime(filename)
static SQInteger script_GetLastWriteTime(HSQUIRRELVM v)
{
    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filename>"));
    }

    sq_pushinteger(v, GetLastWriteTime(filename));
    return 1;
}

//-----------------------------------------------------------------
// CreateDirectory(directory)
static SQInteger script_CreateDirectory(HSQUIRRELVM v)
{
    // get directory
    const SQChar* directory = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <directory>"));
    }
    sq_getstring(v, 2, &directory);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <directory>"));
    }

    sq_pushbool(v, CreateDirectory(filename));
    return 1;
}

//-----------------------------------------------------------------
// Remove(filename)
static SQInteger script_Remove(HSQUIRRELVM v)
{
    // get filename
    const SQChar* filename = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filename>"));
    }
    sq_getstring(v, 2, &filename);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filename>"));
    }

    sq_pushbool(v, Remove(filename));
    return 1;
}

//-----------------------------------------------------------------
// Rename(filenameFrom, filenameTo)
static SQInteger script_Rename(HSQUIRRELVM v)
{
    // get filenameFrom
    const SQChar* filenameFrom = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filenameFrom>"));
    }
    sq_getstring(v, 2, &filenameFrom);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filenameFrom>"));
    }

    // get filenameTo
    const SQChar* filenameTo = 0;
    if (sq_gettype(v, 3) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <filenameTo>"));
    }
    sq_getstring(v, 3, &filenameTo);
    if (sq_getsize(v, 3) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <filenameTo>"));
    }

    sq_pushbool(v, Rename(filenameFrom, filenameTo));
    return 1;
}

//-----------------------------------------------------------------
// GetFileList(directory)
static SQInteger script_GetFileList(HSQUIRRELVM v)
{
    // get directory
    const SQChar* directory = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <directory>"));
    }
    sq_getstring(v, 2, &directory);
    if (sq_getsize(v, 2) == 0) { // empty filename
        return sq_throwerror(v, _SC("invalid parameter <directory>"));
    }

    std::vector<std::string> file_list;
    if (!GetFileList(directory, file_list)) {
        return sq_throwerror(v, _SC("failed to get file list"));
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
    IFile* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FILE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // return name
    sq_pushstring(v, This->getName().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// File._typeof()
static SQInteger script_file__typeof(HSQUIRRELVM v)
{
    IFile* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FILE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("File"));
    return 1;
}

//-----------------------------------------------------------------
// File._cloned(file)
static SQInteger script_file__cloned(HSQUIRRELVM v)
{
    // ensure that this function is only ever called on uninitialized File instances
    IFile* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FILE)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    return sq_throwerror(v, _SC("files can not be cloned"));
}

//-----------------------------------------------------------------
// File._tostring()
static SQInteger script_file__tostring(HSQUIRRELVM v)
{
    IFile* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FILE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<File instance (";
    if (This->isOpen()) {
        oss << "\"" << This->getName() << "\"";
    } else {
        oss << "N/A";
    }
    oss << ")>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
bool IsFile(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_FILE;
    }
    return false;
}

//-----------------------------------------------------------------
IFile* GetFile(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_FILE))) {
        return (IFile*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindFile(HSQUIRRELVM v, IFile* file)
{
    if (!file) {
        return false;
    }
    sq_pushobject(v, g_file_class); // push file class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop file class
        return false;
    }
    sq_remove(v, -2); // pop file class
    sq_setreleasehook(v, -1, script_file_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)file);
    file->grab(); // grab a new reference
    return true;
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
        for (int i = 0; i < video_modes.size(); ++i) {
            sq_pushinteger(v, i); // key
            sq_newtable(v); // value
            sq_pushstring(v, _SC("width"), -1);
            sq_pushinteger(v, video_modes[i].width);
            sq_newslot(v, -3, SQFalse);
            sq_pushstring(v, _SC("height"), -1);
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
    // get width
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <width>"));
    }
    SQInteger width;
    sq_getinteger(v, 2, &width);
    if (width <= 0) {
        return sq_throwerror(v, _SC("invalid parameter <width>"));
    }

    // get height
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <height>"));
    }
    SQInteger height;
    sq_getinteger(v, 3, &height);
    if (height <= 0) {
        return sq_throwerror(v, _SC("invalid parameter <height>"));
    }

    // get fullscreen
    if (sq_gettype(v, 4) != OT_BOOL) {
        return sq_throwerror(v, _SC("invalid type of parameter <fullscreen>"));
    }
    SQBool fullscreen;
    sq_getinteger(v, 4, &fullscreen);

    sq_pushbool(v, OpenWindow(width, height, fullscreen));
    return 1;
}

//-----------------------------------------------------------------
// IsWindowOpen()
static SQInteger script_IsWindowOpen(HSQUIRRELVM v)
{
    sq_pushbool(v, IsWindowOpen());
    return 1;
}

//-----------------------------------------------------------------
// GetWindowWidth()
static SQInteger script_GetWindowWidth(HSQUIRRELVM v)
{
    sq_pushinteger(v, GetWindowWidth());
    return 1;
}

//-----------------------------------------------------------------
// GetWindowHeight()
static SQInteger script_GetWindowHeight(HSQUIRRELVM v)
{
    sq_pushinteger(v, GetWindowHeight());
    return 1;
}

//-----------------------------------------------------------------
// IsWindowFullscreen()
static SQInteger script_IsWindowFullscreen(HSQUIRRELVM v)
{
    sq_pushbool(v, IsWindowFullscreen());
    return 1;
}

//-----------------------------------------------------------------
// SetWindowFullscreen(fullscreen)
static SQInteger script_SetWindowFullscreen(HSQUIRRELVM v)
{
    // get fullscreen
    if (sq_gettype(v, 2) != OT_BOOL) {
        return sq_throwerror(v, _SC("invalid type of parameter <fullscreen>"));
    }
    SQBool fullscreen;
    sq_getinteger(v, 2, &fullscreen);

    sq_pushbool(v, SetWindowFullscreen(fullscreen));
    return 1;
}

//-----------------------------------------------------------------
// GetWindowTitle()
static SQInteger script_GetWindowTitle(HSQUIRRELVM v)
{
    sq_pushstring(v, GetWindowTitle(), -1);
    return 1;
}

//-----------------------------------------------------------------
// SetWindowTitle(title)
static SQInteger script_SetWindowTitle(HSQUIRRELVM v)
{
    // get title
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <title>"));
    }
    const SQChar* title = 0;
    sq_getstring(v, 2, &title);

    SetWindowTitle(title);
    return 0;
}

//-----------------------------------------------------------------
// SetWindowIcon(icon)
static SQInteger script_SetWindowIcon(HSQUIRRELVM v)
{
    // get icon
    if (!IsCanvas(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <icon>"));
    }
    Canvas* icon = GetCanvas(v, 2);
    assert(icon);

    SetWindowIcon(icon);
    return 0;
}

//-----------------------------------------------------------------
// SwapBuffers()
static SQInteger script_SwapWindowBuffers(HSQUIRRELVM v)
{
    SwapWindowBuffers();
    return 0;
}

//-----------------------------------------------------------------
// GetClipRect()
static SQInteger script_GetClipRect(HSQUIRRELVM v)
{
    Recti clip_rect;
    if (!GetClipRect(clip_rect)) {
        return sq_throwerror(v, _SC("failed to get clip rect"));
    }

    if (!BindRect(v, clip_rect)) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// SetClipRect(clip_rect)
static SQInteger script_SetClipRect(HSQUIRRELVM v)
{
    // get clip_rect
    if (!IsRect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <clip_rect>"));
    }
    Recti* clip_rect = GetRect(v, 2);
    assert(clip_rect);

    SetClipRect(*clip_rect);
    return 0;
}

//-----------------------------------------------------------------
// CreateTexture(canvas)
static SQInteger script_CreateTexture(HSQUIRRELVM v)
{
    // get canvas
    if (!IsCanvas(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <canvas>"));
    }
    Canvas* canvas = GetCanvas(v, 2);
    assert(canvas);

    if (!BindTexture(v, CreateTexture(canvas))) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// CloneSection(section)
static SQInteger script_CloneSection(HSQUIRRELVM v)
{
    // get section
    if (!IsRect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <section>"));
    }
    Recti* section = GetRect(v, 2);
    assert(section);

    // clone
    TexturePtr texture = video::CloneSection(*section);
    if (!texture) {
        return sq_throwerror(v, _SC("failed to clone video section"));
    }

    if (!BindTexture(v, texture.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// DrawPoint(pos, col)
static SQInteger script_DrawPoint(HSQUIRRELVM v)
{
    // get pos
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos>"));
    }
    Vec2i* pos = GetVec2(v, 2);
    assert(pos);

    // get color
    if (!IsRGBA(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <color>"));
    }
    RGBA* col = GetRGBA(v, 2);
    assert(col);

    video::DrawPoint(*pos, *col);
    return 0;
}

//-----------------------------------------------------------------
// DrawLine(pos1, pos2, col [, col2])
static SQInteger script_DrawLine(HSQUIRRELVM v)
{
    // get pos1
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos1>"));
    }
    Vec2i* pos1 = GetVec2(v, 2);
    assert(pos1);

    // get pos2
    if (!IsVec2(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos2>"));
    }
    Vec2i* pos2 = GetVec2(v, 3);
    assert(pos2);

    Vec2i positions[2] = {
        *pos1,
        *pos2,
    };

    // get col
    if (!IsRGBA(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <col>"));
    }
    RGBA* col = GetRGBA(v, 4);
    assert(col);

    RGBA colors[2] = {
        *col,
        *col,
    };

    // get optional col2
    if (sq_gettop(v) >= 5) {
        if (!IsRGBA(v, 5)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col2>"));
        }
        colors[1] = GetRGBA(v, 5);
    }

    video::DrawLine(positions, colors);
    return 0;
}

//-----------------------------------------------------------------
// DrawTriangle(pos1, pos2, pos3, col [, col2, col3])
static SQInteger script_DrawTriangle(HSQUIRRELVM v)
{
    // get pos1
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos1>"));
    }
    Vec2i* pos1 = GetVec2(v, 2);
    assert(pos1);

    // get pos2
    if (!IsVec2(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos2>"));
    }
    Vec2i* pos2 = GetVec2(v, 3);
    assert(pos2);

    // get pos3
    if (!IsVec2(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos3>"));
    }
    Vec2i* pos3 = GetVec2(v, 4);
    assert(pos3);

    Vec2i positions[3] = {
        *pos1,
        *pos2,
        *pos3,
    };

    // get col
    if (!IsRGBA(v, 5)) {
        return sq_throwerror(v, _SC("invalid type of parameter <col>"));
    }
    RGBA* col = GetRGBA(v, 5);
    assert(col);

    RGBA  colors[3] = {
        *col,
        *col,
        *col,
    };

    // get optional col2
    RGBA* col2 = 0;
    if (sq_gettop(v) >= 6) {
        if (!IsRGBA(v, 6)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col2>"));
        }
        colors[1] = GetRGBA(v, 6);
    }

    // get optional col3
    if (sq_gettop(v) >= 7) {
        if (!IsRGBA(v, 7)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col3>"));
        }
        colors[2] = GetRGBA(v, 7);
    }

    video::DrawTriangle(positions, colors);
    return 0;
}

//-----------------------------------------------------------------
// DrawTexturedTriangle(texture, texcoord1, texcoord2, texcoord3, pos1, pos2, pos3 [, mask_col])
static SQInteger script_DrawTexturedTriangle(HSQUIRRELVM v)
{
    // get texture
    if (!IsTexture(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <texture>"));
    }
    ITexture* texture = GetTexture(v, 2);
    assert(texture);

    // get texcoord1
    if (!IsVec2(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <texcoord1>"));
    }
    Vec2i* texcoord1 = GetVec2(v, 3);
    assert(texcoord1);

    // get texcoord2
    if (!IsVec2(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <texcoord2>"));
    }
    Vec2i* texcoord2 = GetVec2(v, 4);
    assert(texcoord2);

    // get texcoord3
    if (!IsVec2(v, 5)) {
        return sq_throwerror(v, _SC("invalid type of parameter <texcoord3>"));
    }
    Vec2i* texcoord3 = GetVec2(v, 5);
    assert(texcoord3);

    Vec2i texcoords[3] = {
        *texcoord1,
        *texcoord2,
        *texcoord3,
    };

    // get pos1
    if (!IsVec2(v, 6)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos1>"));
    }
    Vec2i* pos1 = GetVec2(v, 6);
    assert(pos1);

    // get pos2
    if (!IsVec2(v, 7)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos2>"));
    }
    Vec2i* pos2 = GetVec2(v, 7);
    assert(pos2);

    // get pos3
    if (!IsVec2(v, 8)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos3>"));
    }
    Vec2i* pos3 = GetVec2(v, 8);
    assert(pos3);

    Vec2i positions[3] = {
        *pos1,
        *pos2,
        *pos3,
    };

    // get optional mask_col
    RGBA* mask_col = 0;
    if (sq_gettop(v) >= 9) {
        if (!IsRGBA(v, 9)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col>"));
        }
        mask_col = GetRGBA(v, 9);
        assert(mask_col);
    }

    video::DrawTexturedTriangle(texture, texcoords, positions, mask_col);
    return 0;
}

DrawRect(Vec2(10, 10), 100, 50, RGBA(200, 30, 50));

//-----------------------------------------------------------------
// DrawRect(pos, width, height, col [, col2, col3, col4])
static SQInteger script_DrawRect(HSQUIRRELVM v)
{
    // get pos
    if (!IsVec2(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos>"));
    }
    Vec2i* pos = GetVec2(v, 2);
    assert(pos);

    // get width
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <width>"));
    }
    SQInteger width;
    sq_getinteger(v, 3, &width);

    // get height
    if (sq_gettype(v, 4) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <height>"));
    }
    SQInteger height;
    sq_getinteger(v, 4, &height);

    // get col
    if (!IsRGBA(v, 5)) {
        return sq_throwerror(v, _SC("invalid type of parameter <col>"));
    }
    RGBA* col = GetRGBA(v, 5);
    assert(col);

    RGBA colors[4] = {
        *col,
        *col,
        *col,
        *col,
    };

    // get optional col2
    if (sq_gettop(v) >= 6) {
        if (!IsRGBA(v, 6)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col2>"));
        }
        colors[1] = *(GetRGBA(v, 6));
    }

    // get optional col3
    if (sq_gettop(v) >= 7) {
        if (!IsRGBA(v, 7)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col3>"));
        }
        colors[2] = *(GetRGBA(v, 7));
    }

    // get optional col4
    if (sq_gettop(v) >= 8) {
        if (!IsRGBA(v, 8)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col4>"));
        }
        colors[3] = *(GetRGBA(v, 8));
    }

    video::DrawRect(*rect, colors);
    return 0;
}

//-----------------------------------------------------------------
// DrawImage(image, pos [, mask_col])
static SQInteger script_DrawImage(HSQUIRRELVM v)
{
    // get image
    if (!IsTexture(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <image>"));
    }
    ITexture* image = GetTexture(v, 2);
    assert(image);

    // get pos
    if (!IsVec2(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos>"));
    }
    Vec2i* pos = GetVec2(v, 3);
    assert(pos);

    // get optional mask_col
    RGBA* mask_col = 0;
    if (sq_gettop(v) >= 4) {
        if (!IsRGBA(v, 4)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col>"));
        }
        RGBA* col = GetRGBA(v, 4);
    }

    video::DrawImage(image, pos, mask_col);
    return 0;
}

//-----------------------------------------------------------------
// DrawSubImage(image, src_rect, pos [, mask_col])
static SQInteger script_DrawSubImage(HSQUIRRELVM v)
{
    // get image
    if (!IsTexture(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <image>"));
    }
    ITexture* image = GetTexture(v, 2);
    assert(image);

    // get src_rect
    if (!IsRect(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <src_rect>"));
    }
    Recti* src_rect = GetRect(v, 3);
    assert(src_rect);

    // get pos
    if (!IsVec2(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos>"));
    }
    Vec2i* pos = GetVec2(v, 4);
    assert(pos);

    // get optional mask_col
    RGBA* mask_col = 0;
    if (sq_gettop(v) >= 5) {
        if (!IsRGBA(v, 5)) {
            return sq_throwerror(v, _SC("invalid type of parameter <col>"));
        }
        RGBA* col = GetRGBA(v, 5);
    }

    video::DrawSubImage(image, *src_rect, pos, mask_col);
    return 0;
}

//-----------------------------------------------------------------
// DrawImageQuad(image, pos1, pos2, pos3, pos4 [, mask_col])
static SQInteger script_DrawImageQuad(HSQUIRRELVM v)
{
    // get image
    if (!IsTexture(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <image>"));
    }
    ITexture* image = GetTexture(v, 2);
    assert(texture);

    // get pos1
    if (!IsVec2(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos1>"));
    }
    Vec2i* pos1 = GetVec2(v, 3);
    assert(pos1);

    // get pos2
    if (!IsVec2(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos2>"));
    }
    Vec2i* pos2 = GetVec2(v, 4);
    assert(pos2);

    // get pos3
    if (!IsVec2(v, 5)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos3>"));
    }
    Vec2i* pos3 = GetVec2(v, 5);
    assert(pos3);

    // get pos4
    if (!IsVec2(v, 6)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos4>"));
    }
    Vec2i* pos4 = GetVec2(v, 6);
    assert(pos4);

    Vec2i positions[4] = {
        *pos1,
        *pos2,
        *pos3,
        *pos4,
    };

    // get optional mask_col
    RGBA* mask_col = 0;
    if (sq_gettop(v) >= 7) {
        if (!IsRGBA(v, 7)) {
            return sq_throwerror(v, _SC("invalid type of parameter <mask_col>"));
        }
        mask_col = GetRGBA(v, 7);
        assert(mask_col);
    }

    video::DrawImageQuad(image, positions, mask_col);
    return 0;
}

//-----------------------------------------------------------------
// DrawSubImageQuad(image, src_rect, pos1, pos2, pos3, pos4 [, mask_col])
static SQInteger script_DrawSubImageQuad(HSQUIRRELVM v)
{
    // get image
    if (!IsTexture(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <image>"));
    }
    ITexture* image = GetTexture(v, 2);
    assert(image);

    // get src_rect
    if (!IsRect(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <src_rect>"));
    }
    Recti* src_rect = GetRect(v, 3);
    assert(src_rect);

    // get pos1
    if (!IsVec2(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos1>"));
    }
    Vec2i* pos1 = GetVec2(v, 4);
    assert(pos1);

    // get pos2
    if (!IsVec2(v, 5)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos2>"));
    }
    Vec2i* pos2 = GetVec2(v, 5);
    assert(pos2);

    // get pos3
    if (!IsVec2(v, 6)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos3>"));
    }
    Vec2i* pos3 = GetVec2(v, 6);
    assert(pos3);

    // get pos4
    if (!IsVec2(v, 7)) {
        return sq_throwerror(v, _SC("invalid type of parameter <pos4>"));
    }
    Vec2i* pos4 = GetVec2(v, 7);
    assert(pos4);

    Vec2i positions[4] = {
        *pos1,
        *pos2,
        *pos3,
        *pos4,
    };

    // get optional mask_col
    RGBA* mask_col = 0;
    if (sq_gettop(v) >= 8) {
        if (!IsRGBA(v, 8)) {
            return sq_throwerror(v, _SC("invalid type of parameter <mask_col>"));
        }
        mask_col = GetRGBA(v, 8);
        assert(mask_col);
    }

    video::DrawSubImageQuad(image, *src_rect, positions, mask_col);
    return 0;
}

/**************************** TEXTURE *****************************/

//-----------------------------------------------------------------
static SQInteger script_texture_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ITexture*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Texture.updatePixels(canvas [, dst_rect])
static SQInteger script_texture_updatePixels(HSQUIRRELVM v)
{
    ITexture* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get canvas
    if (!IsCanvas(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <canvas>"));
    }
    Canvas* canvas = GetCanvas(v, 2);
    assert(canvas);

    // get optional dst_rect
    Recti* dst_rect = 0;
    if (sq_gettop(v) >= 3) {
        if (!IsRect(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <dst_rect>"));
        }
        dst_rect = GetRect(v, 3);
        assert(dst_rect);
    }

    sq_pushbool(v, This->updatePixels(canvas, dst_rect));
    return 1;
}

//-----------------------------------------------------------------
// Texture.createCanvas()
static SQInteger script_texture_createCanvas(HSQUIRRELVM v)
{
    ITexture* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // create canvas
    CanvasPtr canvas = This->createCanvas();
    if (!canvas) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    if (!BindCanvas(v, canvas.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Texture._get(index)
static SQInteger script_texture__get(HSQUIRRELVM v)
{
    ITexture* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get string index
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    const SQChar* index = 0;
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("width")) == 0) {
        sq_pushinteger(v, This->getWidth());
    } else if (scstrcmp(index, _SC("height")) == 0) {
        sq_pushinteger(v, This->getHeight());
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// Texture._typeof()
static SQInteger script_texture__typeof(HSQUIRRELVM v)
{
    ITexture* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Texture"));
    return 1;
}

//-----------------------------------------------------------------
// Texture._cloned(texture)
static SQInteger script_texture__cloned(HSQUIRRELVM v)
{
    // ensure that this function is only ever called on uninitialized Texture instances
    ITexture* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    // get original texture
    if (!IsTexture(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <texture>"));
    }
    Canvas* orig = GetTexture(v, 2);
    assert(orig);

    // convert original texture to canvas
    CanvasPtr canvas = orig->createCanvas();
    if (!canvas) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    // clone texture
    This = CreateTexture(canvas.get());
    if (!This) {
        return sq_throwerror(v, _SC("out of memory"));
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_texture_destructor);
    return 0;
}

//-----------------------------------------------------------------
// Texture._tostring()
static SQInteger script_texture__tostring(HSQUIRRELVM v)
{
    ITexture* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<Texture instance (" << This->getWidth() << ", " << This->getHeight() << ")>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// Texture._dump(texture, stream)
static SQInteger script_texture__dump(HSQUIRRELVM v)
{
    // get texture
    if (!IsTexture(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <texture>"));
    }
    ITexture* texture = GetTexture(v, 2);
    assert(texture);

    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isOpen() || !stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // convert texture to canvas
    CanvasPtr canvas = texture->createCanvas();
    if (!canvas) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    // write class name
    const char* class_name = "Texture";
    int num_bytes = strlen(class_name);
    if (!writeu32l(stream, num_bytes) || stream->write(class_name, num_bytes) != num_bytes) {
        goto throw_write_error;
    }

    // write texture dimensions
    if (!writeu32l(stream, (u32)canvas->getWidth()) ||
        !writeu32l(stream, (u32)canvas->getHeight())) {
        goto throw_write_error;
    }

    // write texture pixels
    int pixels_size = canvas->getNumPixels() * Canvas::GetNumBytesPerPixel();
    if (stream->write(canvas->getPixels(), pixels_size) != pixels_size) {
        goto throw_write_error;
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// Texture._load(stream)
static SQInteger script_texture__load(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // read texture dimensions
    u32 width;
    u32 height;
    if (!readu32l(stream, width) || !readu32l(stream, height)) {
        goto throw_read_error;
    }

    // create a temporary canvas
    CanvasPtr canvas = Canvas::Create(width, height);
    if (!canvas) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    // read texture pixels into the temporary canvas
    int pixels_size = canvas->getNumPixels() * Canvas::GetNumBytesPerPixel();
    if (stream->read(canvas->getPixels(), pixels_size) != pixels_size) {
        goto throw_read_error;
    }

    // create texture from the canvas
    TexturePtr texture = CreateTexture(canvas.get());
    if (!texture) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    if (!BindTexture(v, texture.get()) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;

throw_read_error:
    return sq_throwerror(v, _SC("read error"));
}

//-----------------------------------------------------------------
bool IsTexture(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_TEXTURE;
    }
    return false;
}

//-----------------------------------------------------------------
ITexture* GetTexture(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_TEXTURE))) {
        return (ITexture*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindTexture(HSQUIRRELVM v, ITexture* texture)
{
    if (!texture) {
        return false;
    }
    sq_pushobject(v, g_texture_class); // push texture class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop texture class
        return false;
    }
    sq_remove(v, -2); // pop texture class
    sq_setreleasehook(v, -1, script_texture_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)texture);
    texture->grab(); // grab a new reference
    return true;
}

/******************************************************************
 *                                                                *
 *                             AUDIO                              *
 *                                                                *
 ******************************************************************/

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
// LoadSound(source [, streaming = false])
static SQInteger script_LoadSound(HSQUIRRELVM v)
{
    if (sq_gettype(v, 2) == OT_STRING) {
        // get filename
        const SQChar* filename = 0;
        sq_getstring(v, 2, &filename);

        // get optional streaming
        SQBool streaming = SQFalse;
        if (sq_gettop(v) >= 3) {
            if (sq_gettop(v, 3) != OT_BOOL) {
                return sq_throwerror(v, _SC("invalid type of parameter <streaming>"));
            }
            sq_getbool(v, 3, &streaming);
        }

        // open file
        FilePtr file = filesystem::OpenFile(filename);
        if (!file) {
            return sq_throwerror(v, _SC("failed to open file"));
        }

        // load sound
        SoundPtr sound = audio::LoadSound(file.get(), (streaming == SQTrue ? true : false));
        if (!sound) {
            return sq_throwerror(v, _SC("failed to load sound"));
        }

        if (!BindSound(v, sound.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    } else if (IsStream(v, 2)) {
        // get stream
        IStream* stream = GetStream(v, 2);
        assert(stream);

        // get optional streaming
        SQBool streaming = SQFalse;
        if (sq_gettop(v) >= 3) {
            if (sq_gettop(v, 3) != OT_BOOL) {
                return sq_throwerror(v, _SC("invalid type of parameter <streaming>"));
            }
            sq_getbool(v, 3, &streaming);
        }

        // load sound
        SoundPtr sound = audio::LoadSound(stream, (streaming == SQTrue ? true : false));
        if (!sound) {
            return sq_throwerror(v, _SC("failed to load sound"));
        }

        if (!BindSound(v, sound.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    } else {
        return sq_throwerror(v, _SC("invalid type of parameter <source>"));
    }
    return 1;
}

//-----------------------------------------------------------------
// LoadSoundEffect(source)
static SQInteger script_LoadSoundEffect(HSQUIRRELVM v)
{
    if (sq_gettype(v, 2) == OT_STRING) {
        // get filename
        const SQChar* filename = 0;
        sq_getstring(v, 2, &filename);

        // open file
        FilePtr file = filesystem::OpenFile(filename);
        if (!file) {
            return sq_throwerror(v, _SC("failed to open file"));
        }

        // load sound effect
        SoundEffectPtr sound_effect = audio::LoadSoundEffect(file.get());
        if (!sound_effect) {
            return sq_throwerror(v, _SC("failed to load sound effect"));
        }

        if (!BindSoundEffect(v, sound_effect.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    } else if (IsStream(v, 2)) {
        // get stream
        IStream* stream = GetStream(v, 2);
        assert(stream);

        // load sound effect
        SoundEffectPtr sound_effect = audio::LoadSoundEffect(stream);
        if (!sound_effect) {
            return sq_throwerror(v, _SC("failed to load sound effect"));
        }

        if (!BindSoundEffect(v, sound_effect.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    } else {
        return sq_throwerror(v, _SC("invalid type of parameter <source>"));
    }
    return 1;
}

/***************************** SOUND ******************************/

//-----------------------------------------------------------------
static SQInteger script_sound_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ISound*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Sound.play()
static SQInteger script_sound_play(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->play();
    return 0;
}

//-----------------------------------------------------------------
// Sound.stop()
static SQInteger script_sound_stop(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->stop();
    return 0;
}

//-----------------------------------------------------------------
// Sound.reset()
static SQInteger script_sound_reset(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->reset();
    return 0;
}

//-----------------------------------------------------------------
// Sound.isPlaying()
static SQInteger script_sound_isPlaying(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isPlaying());
    return 1;
}

//-----------------------------------------------------------------
// Sound.isSeekable()
static SQInteger script_sound_isSeekable(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isSeekable());
    return 1;
}

//-----------------------------------------------------------------
// Sound._get(index)
static SQInteger script_sound__get(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get string index
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    const SQChar* index = 0;
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("length")) == 0) {
        sq_pushinteger(v, This->getLength());
    } else if (scstrcmp(index, _SC("repeat")) == 0) {
        sq_pushbool(v, This->getRepeat());
    } else if (scstrcmp(index, _SC("volume")) == 0) {
        sq_pushinteger(v, This->getVolume());
    } else if (scstrcmp(index, _SC("position")) == 0) {
        sq_pushinteger(v, This->getPosition());
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// Sound._set(index, value)
static SQInteger script_sound__set(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get string index
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    const SQChar* index = 0;
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("repeat")) == 0) {
        SQBool value;
        if (sq_gettype(v, 3) != OT_BOOL) {
            return sq_throwerror(v, _SC("invalid type of parameter <value>"));
        }
        sq_getbool(v, 3, &value);
        This->setRepeat((value == SQTrue ? true : false));
    } else if (scstrcmp(index, _SC("volume")) == 0) {
        SQInteger value;
        if (sq_gettype(v, 3) != OT_INTEGER && sq_gettype(v, 3) != OT_FLOAT) {
            return sq_throwerror(v, _SC("invalid type of parameter <value>"));
        }
        sq_getinteger(v, 3, &value);
        This->setVolume(value);
    } else if (scstrcmp(index, _SC("position")) == 0) {
        SQInteger value;
        if (sq_gettype(v, 3) != OT_INTEGER && sq_gettype(v, 3) != OT_FLOAT) {
            return sq_throwerror(v, _SC("invalid type of parameter <value>"));
        }
        sq_getinteger(v, 3, &value);
        This->setPosition(value);
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 0;
}

//-----------------------------------------------------------------
// Sound._typeof()
static SQInteger script_sound__typeof(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Sound"), -1);
    return 1;
}

//-----------------------------------------------------------------
// Sound._tostring()
static SQInteger script_sound__tostring(HSQUIRRELVM v)
{
    ISound* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("<Sound instance>"), -1);
    return 1;
}

//-----------------------------------------------------------------
bool IsSound(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_SOUND;
    }
    return false;
}

//-----------------------------------------------------------------
ISound* GetSound(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_SOUND))) {
        return (ISound*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindSound(HSQUIRRELVM v, ISound* sound)
{
    if (!sound) {
        return false;
    }
    sq_pushobject(v, g_sound_class); // push sound class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop sound class
        return false;
    }
    sq_remove(v, -2); // pop sound class
    sq_setreleasehook(v, -1, script_sound_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)sound);
    sound->grab(); // grab a new reference
    return true;
}

/************************** SOUNDEFFECT ***************************/

//-----------------------------------------------------------------
static SQInteger script_soundeffect_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ISoundEffect*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// SoundEffect.play()
static SQInteger script_soundeffect_play(HSQUIRRELVM v)
{
    ISoundEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->play();
    return 0;
}

//-----------------------------------------------------------------
// SoundEffect.stop()
static SQInteger script_soundeffect_stop(HSQUIRRELVM v)
{
    ISoundEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    This->stop();
    return 0;
}

//-----------------------------------------------------------------
// SoundEffect._get(index)
static SQInteger script_soundeffect__get(HSQUIRRELVM v)
{
    Canvas* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get string index
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    const SQChar* index = 0;
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("volume")) == 0) {
        sq_pushinteger(v, This->getVolume());
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// SoundEffect._set(index, value)
static SQInteger script_soundeffect__set(HSQUIRRELVM v)
{
    ISoundEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get string index
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    const SQChar* index = 0;
    sq_getstring(v, 2, &index);

    // scstrcmp is defined by squirrel
    if (scstrcmp(index, _SC("volume")) == 0) {
        SQInteger value;
        if (sq_gettype(v, 3) != OT_INTEGER && sq_gettype(v, 3) != OT_FLOAT) {
            return sq_throwerror(v, _SC("invalid type of parameter <value>"));
        }
        sq_getinteger(v, 3, &value);
        This->setVolume(value);
    } else {
        // index not found
        sq_pushnull(v);
        return sq_throwobject(v);
    }
    return 0;
}

//-----------------------------------------------------------------
// SoundEffect._typeof()
static SQInteger script_soundeffect__typeof(HSQUIRRELVM v)
{
    ISoundEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("SoundEffect"), -1);
    return 1;
}

//-----------------------------------------------------------------
// SoundEffect._tostring()
static SQInteger script_soundeffect__tostring(HSQUIRRELVM v)
{
    ISoundEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("<SoundEffect instance>"), -1);
    return 1;
}

//-----------------------------------------------------------------
bool IsSoundEffect(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_SOUNDEFFECT;
    }
    return false;
}

//-----------------------------------------------------------------
ISoundEffect* GetSoundEffect(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_SOUNDEFFECT))) {
        return (ISoundEffect*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindSoundEffect(HSQUIRRELVM v, ISoundEffect* soundeffect)
{
    if (!soundeffect) {
        return false;
    }
    sq_pushobject(v, g_soundeffect_class); // push soundeffect class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop soundeffect class
        return false;
    }
    sq_remove(v, -2); // pop soundeffect class
    sq_setreleasehook(v, -1, script_soundeffect_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)soundeffect);
    soundeffect->grab(); // grab a new reference
    return true;
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
    input::UpdateInput();
    return 0;
}

//-----------------------------------------------------------------
// AreEventsPending()
static SQInteger script_AreEventsPending(HSQUIRRELVM v)
{
    sq_pushbool(v, input::AreEventsPending());
    return 1;
}

//-----------------------------------------------------------------
// GetEvent()
static SQInteger script_GetEvent(HSQUIRRELVM v)
{
    input::Event event;
    if (input::GetEvent(event)) {
        sq_newtable(v); // event

        sq_pushstring(v, _SC("type"), -1);
        sq_pushinteger(v, event.type);
        sq_newslot(v, -3);

        switch (event.type) {
        case input::Event::KEY_DOWN:
        case input::Event::KEY_UP:
            sq_pushstring(v, _SC("key"), -1);
            sq_pushinteger(v, event.key.key);
            sq_newslot(v, -3);
            break;

        case input::Event::MOUSE_BUTTON_DOWN:
        case input::Event::MOUSE_BUTTON_UP:
            sq_pushstring(v, _SC("mbutton"), -1);
            sq_pushinteger(v, event.mbutton.button);
            sq_newslot(v, -3);
            break;

        case input::Event::MOUSE_MOTION:
            sq_pushstring(v, _SC("dx"), -1);
            sq_pushinteger(v, event.mmotion.dx);
            sq_newslot(v, -3);

            sq_pushstring(v, _SC("dy"), -1);
            sq_pushinteger(v, event.mmotion.dy);
            sq_newslot(v, -3);
            break;

        case input::Event::MOUSE_WHEEL_MOTION:
            sq_pushstring(v, _SC("dx"), -1);
            sq_pushinteger(v, event.mwheel.dx);
            sq_newslot(v, -3);

            sq_pushstring(v, _SC("dy"), -1);
            sq_pushinteger(v, event.mwheel.dy);
            sq_newslot(v, -3);
            break;

        case input::Event::JOY_BUTTON_DOWN:
        case input::Event::JOY_BUTTON_UP:
            sq_pushstring(v, _SC("joy"), -1);
            sq_pushinteger(v, event.jbutton.joy);
            sq_newslot(v, -3);

            sq_pushstring(v, _SC("button"), -1);
            sq_pushinteger(v, event.jbutton.button);
            sq_newslot(v, -3);
            break;

        case input::Event::JOY_AXIS_MOTION:
            sq_pushstring(v, _SC("joy"), -1);
            sq_pushinteger(v, event.jaxis.joy);
            sq_newslot(v, -3);

            sq_pushstring(v, _SC("axis"), -1);
            sq_pushinteger(v, event.jaxis.axis);
            sq_newslot(v, -3);

            sq_pushstring(v, _SC("value"), -1);
            sq_pushinteger(v, event.jaxis.value);
            sq_newslot(v, -3);
            break;

        case input::Event::JOY_HAT_MOTION:
            sq_pushstring(v, _SC("joy"), -1);
            sq_pushinteger(v, event.jhat.joy);
            sq_newslot(v, -3);

            sq_pushstring(v, _SC("hat"), -1);
            sq_pushinteger(v, event.jhat.hat);
            sq_newslot(v, -3);

            sq_pushstring(v, _SC("state"), -1);
            sq_pushinteger(v, event.jhat.state);
            sq_newslot(v, -3);
            break;

        case input::Event::APP_QUIT:
            break;

        default:
            sq_poptop(v); // pop event table
            return sq_throwerror(v, _SC("unrecognized event type"));
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
    input::ClearEvents();
    return 0;
}

//-----------------------------------------------------------------
// IsKeyDown(key)
static SQInteger script_IsKeyDown(HSQUIRRELVM v)
{
    // get key
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <key>"));
    }
    SQInteger key;
    sq_getinteger(v, 2, &key);

    sq_pushbool(v, input::IsKeyDown(key));
    return 1;
}

//-----------------------------------------------------------------
// GetMouseX()
static SQInteger script_GetMouseX(HSQUIRRELVM v)
{
    sq_pushinteger(v, input::GetMouseX());
    return 1;
}

//-----------------------------------------------------------------
// GetMouseY()
static SQInteger script_GetMouseY(HSQUIRRELVM v)
{
    sq_pushinteger(v, input::GetMouseY());
    return 1;
}

//-----------------------------------------------------------------
// IsMouseButtonDown(button)
static SQInteger script_IsMouseButtonDown(HSQUIRRELVM v)
{
    // get button
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <button>"));
    }
    SQInteger button;
    sq_getinteger(v, 2, &button);

    sq_pushbool(v, input::IsMouseButtonDown(button));
    return 1;
}

//-----------------------------------------------------------------
// GetNumJoysticks()
static SQInteger script_GetNumJoysticks(HSQUIRRELVM v)
{
    sq_pushinteger(v, input::GetNumJoysticks());
    return 1;
}

//-----------------------------------------------------------------
// GetJoystickName(joy)
static SQInteger script_GetJoystickName(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    const char* joy_name = input::GetJoystickName(joy);
    if (joy_name) {
        sq_pushstring(v, joy_name, -1);
    } else {
        sq_pushnull(v);
    }
    return 1;
}

//-----------------------------------------------------------------
// GetNumJoystickButtons(joy)
static SQInteger script_GetNumJoystickButtons(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    sq_pushinteger(v, input::GetNumJoystickButtons(joy));
    return 1;
}

//-----------------------------------------------------------------
// GetNumJoystickAxes(joy)
static SQInteger script_GetNumJoystickAxes(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    sq_pushinteger(v, input::GetNumJoystickAxes(joy));
    return 1;
}

//-----------------------------------------------------------------
// GetNumJoystickHats(joy)
static SQInteger script_GetNumJoystickHats(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    sq_pushinteger(v, input::GetNumJoystickHats(joy));
    return 1;
}

//-----------------------------------------------------------------
// IsJoystickButtonDown(joy, button)
static SQInteger script_IsJoystickButtonDown(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get button
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <button>"));
    }
    SQInteger button;
    sq_getinteger(v, 3, &button);

    sq_pushbool(v, input::IsJoystickButtonDown(joy, button));
    return 1;
}

//-----------------------------------------------------------------
// GetJoystickAxis(joy, button)
static SQInteger script_GetJoystickAxis(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get axis
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <axis>"));
    }
    SQInteger axis;
    sq_getinteger(v, 3, &axis);

    sq_pushinteger(v, input::GetJoystickAxis(joy, axis));
    return 1;
}

//-----------------------------------------------------------------
// GetJoystickHat(joy, hat)
static SQInteger script_GetJoystickHat(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get hat
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <hat>"));
    }
    SQInteger hat;
    sq_getinteger(v, 3, &hat);

    sq_pushinteger(v, input::GetJoystickHat(joy, hat));
    return 1;
}

//-----------------------------------------------------------------
// HasJoystickForceFeedback(joy)
static SQInteger script_HasJoystickForceFeedback(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    sq_pushbool(v, input::HasJoystickForceFeedback(joy));
    return 1;
}

//-----------------------------------------------------------------
// IsJoystickForceEffectSupported(joy, effect)
static SQInteger script_IsJoystickForceEffectSupported(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get effect
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    SQInteger effect;
    sq_getinteger(v, 3, &effect);

    sq_pushbool(v, input::IsJoystickForceEffectSupported(joy, effect));
    return 1;
}

//-----------------------------------------------------------------
// UploadJoystickForceEffect(joy, effect)
static SQInteger script_UploadJoystickForceEffect(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get effect
    if (!IsForceEffect(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    input::ForceEffect* effect = GetForceEffect(v, 3);
    assert(effect);

    sq_pushinteger(v, input::UploadJoystickForceEffect(joy, *effect));
    return 1;
}

//-----------------------------------------------------------------
// PlayJoystickForceEffect(joy, effect [, times])
static SQInteger script_PlayJoystickForceEffect(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get effect
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    SQInteger effect;
    sq_getinteger(v, 3, &effect);

    // get optional times
    SQInteger times = 1;
    if (sq_gettop(v) >= 4) {
        if (sq_gettype(v, 4) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <times>"));
        }
        sq_getinteger(v, 4, &times);
    }

    sq_pushbool(v, input::PlayJoystickForceEffect(joy, effect, times));
    return 1;
}

//-----------------------------------------------------------------
// UpdateJoystickForceEffect(joy, effect, new_data)
static SQInteger script_UpdateJoystickForceEffect(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get effect
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    SQInteger effect;
    sq_getinteger(v, 3, &effect);

    // get new_data
    if (!IsForceEffect(v, 4)) {
        return sq_throwerror(v, _SC("invalid type of parameter <new_data>"));
    }
    input::ForceEffect* new_data = GetForceEffect(v, 4);
    assert(new_data);

    sq_pushinteger(v, input::UpdateJoystickForceEffect(joy, effect, *new_data));
    return 1;
}

//-----------------------------------------------------------------
// StopJoystickForceEffect(joy, effect)
static SQInteger script_StopJoystickForceEffect(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get effect
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    SQInteger effect;
    sq_getinteger(v, 3, &effect);

    sq_pushbool(v, input::StopJoystickForceEffect(joy, effect));
    return 1;
}

//-----------------------------------------------------------------
// StopAllJoystickForceEffects(joy)
static SQInteger script_StopAllJoystickForceEffects(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    sq_pushbool(v, input::StopAllJoystickForceEffects(joy));
    return 1;
}

//-----------------------------------------------------------------
// RemoveJoystickForceEffect(joy, effect)
static SQInteger script_RemoveJoystickForceEffect(HSQUIRRELVM v)
{
    // get joy
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <joy>"));
    }
    SQInteger joy;
    sq_getinteger(v, 2, &joy);

    // get effect
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    SQInteger effect;
    sq_getinteger(v, 3, &effect);

    input::RemoveJoystickForceEffect(joy, effect);
    return 0;
}


    union ForceEffect {

        static const int INFINITY = 0x7FFFFFFF; // (2^31)-1 highest possible 32-bit signed integer number

        enum Type {
            CONSTANT = 0, // constant effect
            RAMP,         // ramp effect
            SINE,         // periodic effect
            SQUARE,       // periodic effect
            TRIANGLE,     // periodic effect
            SAWTOOTHUP,   // periodic effect
            SAWTOOTHDOWN, // periodic effect
        };

        struct Direction {
            enum Type {
                CARTESIAN = 0, // cartesian direction using the x, y, and z axes
                POLAR,         // polar direction using an angle
            };

            int type;      // the type of the force direction encoding
            int dir[3]; // encoded direction parameters
        };

        // the type of the force effect
        int type;

        // a constant force effect
        struct {
            int type;                  // ForceEffect::CONSTANT
            Direction direction;       // direction of the force
            u32 length;                // effect duration
            u16 delay;                 // delay before playing the effect (again)
            u16 attack_length;
            u16 attack_level;
            u16 fade_length;
            u16 fade_level;
            i16 level;                 // effect strength
        } constant;

        // a ramp force effect
        struct {
            int type;                  // ForceEffect::RAMP
            Direction direction;       // direction of the force
            u32 length;                // effect duration
            u16 delay;                 // delay before playing the effect (again)
            u16 attack_length;
            u16 attack_level;
            u16 fade_length;
            u16 fade_level;
            i16 start;
            i16 end;
        } ramp;

        // a periodic force effect
        struct {
            int type;                  // ForceEffect::PERIODIC
            Direction direction;       // direction of the force
            u32 length;                // effect duration
            u16 delay;                 // delay before playing the effect (again)
            u16 attack_length;
            u16 attack_level;
            u16 fade_length;
            u16 fade_level;
            u16 period;
            i16 magnitude;
            i16 offset;
            u16 phase;
        } periodic;
    };

/************************** FORCEEFFECT ***************************/

//-----------------------------------------------------------------
// ForceEffect(type)
static SQInteger script_forceeffect_constructor(HSQUIRRELVM v)
{
    ForceEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FORCEEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get type
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <type>"));
    }
    SQInteger type;
    sq_getinteger(v, 2, &type);

    if (type != ForceEffect::CONSTANT   &&
        type != ForceEffect::RAMP       &&
        type != ForceEffect::SINE       &&
        type != ForceEffect::SQUARE     &&
        type != ForceEffect::TRIANGLE   &&
        type != ForceEffect::SAWTOOTHUP &&
        type != ForceEffect::SAWTOOTHDOWN)
    {
        return sq_throwerror(v, _SC("invalid parameter <type>"));
    }

    memset(This, 0, sizeof(ForceEffect));
    This->type = type;
    return 0;
}

//-----------------------------------------------------------------
// ForceEffect._get(index)
static SQInteger script_forceeffect__get(HSQUIRRELVM v)
{
    ForceEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FORCEEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    switch (effect->type) {
    case ForceEffect::CONSTANT:
        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("dir_type")) == 0) {
            sq_pushinteger(v, This->constant.direction.type);
        } else if (scstrcmp(index, _SC("dir_0")) == 0) {
            sq_pushinteger(v, This->constant.direction.dir[0]);
        } else if (scstrcmp(index, _SC("dir_1")) == 0) {
            sq_pushinteger(v, This->constant.direction.dir[1]);
        } else if (scstrcmp(index, _SC("dir_2")) == 0) {
            sq_pushinteger(v, This->constant.direction.dir[2]);
        } else if (scstrcmp(index, _SC("length")) == 0) {
            sq_pushinteger(v, This->constant.length);
        } else if (scstrcmp(index, _SC("delay")) == 0) {
            sq_pushinteger(v, This->constant.delay);
        } else if (scstrcmp(index, _SC("attack_length")) == 0) {
            sq_pushinteger(v, This->constant.attack_length);
        } else if (scstrcmp(index, _SC("attack_level")) == 0) {
            sq_pushinteger(v, This->constant.attack_level);
        } else if (scstrcmp(index, _SC("fade_length")) == 0) {
            sq_pushinteger(v, This->constant.fade_length);
        } else if (scstrcmp(index, _SC("fade_level")) == 0) {
            sq_pushinteger(v, This->constant.fade_level);
        } else if (scstrcmp(index, _SC("level")) == 0) {
            sq_pushinteger(v, This->constant.level);
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        break;
    case ForceEffect::RAMP:
        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("dir_type")) == 0) {
            sq_pushinteger(v, This->ramp.direction.type);
        } else if (scstrcmp(index, _SC("dir_0")) == 0) {
            sq_pushinteger(v, This->ramp.direction.dir[0]);
        } else if (scstrcmp(index, _SC("dir_1")) == 0) {
            sq_pushinteger(v, This->ramp.direction.dir[1]);
        } else if (scstrcmp(index, _SC("dir_2")) == 0) {
            sq_pushinteger(v, This->ramp.direction.dir[2]);
        } else if (scstrcmp(index, _SC("length")) == 0) {
            sq_pushinteger(v, This->ramp.length);
        } else if (scstrcmp(index, _SC("delay")) == 0) {
            sq_pushinteger(v, This->ramp.delay);
        } else if (scstrcmp(index, _SC("attack_length")) == 0) {
            sq_pushinteger(v, This->ramp.attack_length);
        } else if (scstrcmp(index, _SC("attack_level")) == 0) {
            sq_pushinteger(v, This->ramp.attack_level);
        } else if (scstrcmp(index, _SC("fade_length")) == 0) {
            sq_pushinteger(v, This->ramp.fade_length);
        } else if (scstrcmp(index, _SC("fade_level")) == 0) {
            sq_pushinteger(v, This->ramp.fade_level);
        } else if (scstrcmp(index, _SC("start")) == 0) {
            sq_pushinteger(v, This->ramp.start);
        } else if (scstrcmp(index, _SC("end")) == 0) {
            sq_pushinteger(v, This->ramp.end);
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        break;
    case ForceEffect::SINE:
    case ForceEffect::SQUARE:
    case ForceEffect::TRIANGLE:
    case ForceEffect::SAWTOOTHUP:
    case ForceEffect::SAWTOOTHDOWN:
        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("dir_type")) == 0) {
            sq_pushinteger(v, This->periodic.direction.type);
        } else if (scstrcmp(index, _SC("dir_0")) == 0) {
            sq_pushinteger(v, This->periodic.direction.dir[0]);
        } else if (scstrcmp(index, _SC("dir_1")) == 0) {
            sq_pushinteger(v, This->periodic.direction.dir[1]);
        } else if (scstrcmp(index, _SC("dir_2")) == 0) {
            sq_pushinteger(v, This->periodic.direction.dir[2]);
        } else if (scstrcmp(index, _SC("length")) == 0) {
            sq_pushinteger(v, This->periodic.length);
        } else if (scstrcmp(index, _SC("delay")) == 0) {
            sq_pushinteger(v, This->periodic.delay);
        } else if (scstrcmp(index, _SC("attack_length")) == 0) {
            sq_pushinteger(v, This->periodic.attack_length);
        } else if (scstrcmp(index, _SC("attack_level")) == 0) {
            sq_pushinteger(v, This->periodic.attack_level);
        } else if (scstrcmp(index, _SC("fade_length")) == 0) {
            sq_pushinteger(v, This->periodic.fade_length);
        } else if (scstrcmp(index, _SC("fade_level")) == 0) {
            sq_pushinteger(v, This->periodic.fade_level);
        } else if (scstrcmp(index, _SC("period")) == 0) {
            sq_pushinteger(v, This->periodic.period);
        } else if (scstrcmp(index, _SC("magnitude")) == 0) {
            sq_pushinteger(v, This->periodic.magnitude);
        } else if (scstrcmp(index, _SC("offset")) == 0) {
            sq_pushinteger(v, This->periodic.offset);
        } else if (scstrcmp(index, _SC("phase")) == 0) {
            sq_pushinteger(v, This->periodic.phase);
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        break;
    default:
        return sq_throwerror(v, _SC("invalid force effect"));
    }
    return 1;
}

//-----------------------------------------------------------------
// ForceEffect._set(index, value)
static SQInteger script_forceeffect__set(HSQUIRRELVM v)
{
    ForceEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FORCEEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get index
    const SQChar* index = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <index>"));
    }
    sq_getstring(v, 2, &index);

    // get value
    if (sq_gettype(v, 3) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <value>"));
    }
    SQInteger value;
    sq_getstring(v, 3, &value);

    switch (effect->type) {
    case ForceEffect::CONSTANT:
        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("dir_type")) == 0) {
            This->constant.direction.type = value;
        } else if (scstrcmp(index, _SC("dir_0")) == 0) {
            This->constant.direction.dir[0] = value;
        } else if (scstrcmp(index, _SC("dir_1")) == 0) {
            This->constant.direction.dir[1] = value;
        } else if (scstrcmp(index, _SC("dir_2")) == 0) {
            This->constant.direction.dir[2] = value;
        } else if (scstrcmp(index, _SC("length")) == 0) {
            This->constant.length = value;
        } else if (scstrcmp(index, _SC("delay")) == 0) {
            This->constant.delay = value;
        } else if (scstrcmp(index, _SC("attack_length")) == 0) {
            This->constant.attack_length = value;
        } else if (scstrcmp(index, _SC("attack_level")) == 0) {
            This->constant.attack_level = value;
        } else if (scstrcmp(index, _SC("fade_length")) == 0) {
            This->constant.fade_length = value;
        } else if (scstrcmp(index, _SC("fade_level")) == 0) {
            This->constant.fade_level = value;
        } else if (scstrcmp(index, _SC("level")) == 0) {
            This->constant.level = value;
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        break;
    case ForceEffect::RAMP:
        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("dir_type")) == 0) {
            This->ramp.direction.type = value;
        } else if (scstrcmp(index, _SC("dir_0")) == 0) {
            This->ramp.direction.dir[0] = value;
        } else if (scstrcmp(index, _SC("dir_1")) == 0) {
            This->ramp.direction.dir[1] = value;
        } else if (scstrcmp(index, _SC("dir_2")) == 0) {
            This->ramp.direction.dir[2] = value;
        } else if (scstrcmp(index, _SC("length")) == 0) {
            This->ramp.length = value;
        } else if (scstrcmp(index, _SC("delay")) == 0) {
            This->ramp.delay = value;
        } else if (scstrcmp(index, _SC("attack_length")) == 0) {
            This->ramp.attack_length = value;
        } else if (scstrcmp(index, _SC("attack_level")) == 0) {
            This->ramp.attack_level = value;
        } else if (scstrcmp(index, _SC("fade_length")) == 0) {
            This->ramp.fade_length = value;
        } else if (scstrcmp(index, _SC("fade_level")) == 0) {
            This->ramp.fade_level = value;
        } else if (scstrcmp(index, _SC("start")) == 0) {
            This->ramp.start = value;
        } else if (scstrcmp(index, _SC("end")) == 0) {
            This->ramp.end = value;
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        break;
    case ForceEffect::SINE:
    case ForceEffect::SQUARE:
    case ForceEffect::TRIANGLE:
    case ForceEffect::SAWTOOTHUP:
    case ForceEffect::SAWTOOTHDOWN:
        // scstrcmp is defined by squirrel
        if (scstrcmp(index, _SC("dir_type")) == 0) {
            This->periodic.direction.type = value;
        } else if (scstrcmp(index, _SC("dir_0")) == 0) {
            This->periodic.direction.dir[0] = value;
        } else if (scstrcmp(index, _SC("dir_1")) == 0) {
            This->periodic.direction.dir[1] = value;
        } else if (scstrcmp(index, _SC("dir_2")) == 0) {
            This->periodic.direction.dir[2] = value;
        } else if (scstrcmp(index, _SC("length")) == 0) {
            This->periodic.length = value;
        } else if (scstrcmp(index, _SC("delay")) == 0) {
            This->periodic.delay = value;
        } else if (scstrcmp(index, _SC("attack_length")) == 0) {
            This->periodic.attack_length = value;
        } else if (scstrcmp(index, _SC("attack_level")) == 0) {
            This->periodic.attack_level = value;
        } else if (scstrcmp(index, _SC("fade_length")) == 0) {
            This->periodic.fade_length = value;
        } else if (scstrcmp(index, _SC("fade_level")) == 0) {
            This->periodic.fade_level = value;
        } else if (scstrcmp(index, _SC("period")) == 0) {
            This->periodic.period = value;
        } else if (scstrcmp(index, _SC("magnitude")) == 0) {
            This->periodic.magnitude = value;
        } else if (scstrcmp(index, _SC("offset")) == 0) {
            This->periodic.offset = value;
        } else if (scstrcmp(index, _SC("phase")) == 0) {
            This->periodic.phase = value;
        } else {
            // index not found
            sq_pushnull(v);
            return sq_throwobject(v);
        }
        break;
    default:
        return sq_throwerror(v, _SC("invalid force effect"));
    }
    return 0;
}

//-----------------------------------------------------------------
// ForceEffect._typeof()
static SQInteger script_forceeffect__typeof(HSQUIRRELVM v)
{
    ForceEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FORCEEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("ForceEffect"), -1);
    return 1;
}

//-----------------------------------------------------------------
// ForceEffect._cloned(effect)
static SQInteger script_forceeffect__cloned(HSQUIRRELVM v)
{
    ForceEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FORCEEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get original effect
    if (!IsForceEffect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    ForceEffect* orig = GetForceEffect(v, 2);
    assert(orig);

    // clone effect
    memcpy(This, orig, sizeof(ForceEffect));
    return 0;
}

//-----------------------------------------------------------------
// ForceEffect._tostring()
static SQInteger script_forceeffect__tostring(HSQUIRRELVM v)
{
    ForceEffect* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FORCEEFFECT))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<ForceEffect instance>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
// ForceEffect._dump(effect, stream)
static SQInteger script_forceeffect__dump(HSQUIRRELVM v)
{
    // get effect
    if (!IsForceEffect(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <effect>"));
    }
    ForceEffect* effect = GetForceEffect(v, 2);
    assert(effect);

    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // write class name
    const char* class_name = "ForceEffect";
    int class_name_size = strlen(class_name);
    if (!writeu32l(stream, (u32)class_name_size) || stream->write(class_name, class_name_size) != class_name_size) {
        goto throw_write_error;
    }

    // write type
    if (!writeu32l(stream, (u32)effect->type)) {
        goto throw_write_error;
    }

    // write data
    switch (effect->type) {
    case ForceEffect::CONSTANT:
        if (!writeu32l(stream, (u32)effect->constant.direction.type)   ||
            !writeu32l(stream, (u32)effect->constant.direction.dir[0]) ||
            !writeu32l(stream, (u32)effect->constant.direction.dir[1]) ||
            !writeu32l(stream, (u32)effect->constant.direction.dir[2]) ||
            !writeu32l(stream, (u32)effect->constant.length)           ||
            !writeu16l(stream, (u16)effect->constant.delay)            ||
            !writeu16l(stream, (u16)effect->constant.attack_length)    ||
            !writeu16l(stream, (u16)effect->constant.attack_level)     ||
            !writeu16l(stream, (u16)effect->constant.fade_length)      ||
            !writeu16l(stream, (u16)effect->constant.fade_level)       ||
            !writeu16l(stream, (u16)effect->constant.level))
        {
            goto throw_write_error;
        }
        break;
    case ForceEffect::RAMP:
        if (!writeu32l(stream, (u32)effect->ramp.direction.type)   ||
            !writeu32l(stream, (u32)effect->ramp.direction.dir[0]) ||
            !writeu32l(stream, (u32)effect->ramp.direction.dir[1]) ||
            !writeu32l(stream, (u32)effect->ramp.direction.dir[2]) ||
            !writeu32l(stream, (u32)effect->ramp.length)           ||
            !writeu16l(stream, (u16)effect->ramp.delay)            ||
            !writeu16l(stream, (u16)effect->ramp.attack_length)    ||
            !writeu16l(stream, (u16)effect->ramp.attack_level)     ||
            !writeu16l(stream, (u16)effect->ramp.fade_length)      ||
            !writeu16l(stream, (u16)effect->ramp.fade_level)       ||
            !writeu16l(stream, (u16)effect->ramp.start)            ||
            !writeu16l(stream, (u16)effect->ramp.end))
        {
            goto throw_write_error;
        }
        break;
    case ForceEffect::SINE:
    case ForceEffect::SQUARE:
    case ForceEffect::TRIANGLE:
    case ForceEffect::SAWTOOTHUP:
    case ForceEffect::SAWTOOTHDOWN:
        if (!writeu32l(stream, (u32)effect->periodic.direction.type)   ||
            !writeu32l(stream, (u32)effect->periodic.direction.dir[0]) ||
            !writeu32l(stream, (u32)effect->periodic.direction.dir[1]) ||
            !writeu32l(stream, (u32)effect->periodic.direction.dir[2]) ||
            !writeu32l(stream, (u32)effect->periodic.length)           ||
            !writeu16l(stream, (u16)effect->periodic.delay)            ||
            !writeu16l(stream, (u16)effect->periodic.attack_length)    ||
            !writeu16l(stream, (u16)effect->periodic.attack_level)     ||
            !writeu16l(stream, (u16)effect->periodic.fade_length)      ||
            !writeu16l(stream, (u16)effect->periodic.fade_level)       ||
            !writeu16l(stream, (u16)effect->periodic.period)           ||
            !writeu16l(stream, (u16)effect->periodic.magnitude)        ||
            !writeu16l(stream, (u16)effect->periodic.offset)           ||
            !writeu16l(stream, (u16)effect->periodic.phase))
        {
            goto throw_write_error;
        }
        break;
    default:
        return sq_throwerror(v, _SC("invalid force effect"));
    }
    return 0;

throw_write_error:
    return sq_throwerror(v, _SC("write error"));
}

//-----------------------------------------------------------------
// ForceEffect._load(stream)
static SQInteger script_forceeffect__load(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    ForceEffect effect;

    // read type
    if (!readu32l(stream, (u32)effect.type)) {
        goto throw_read_error;
    }

    // read data
    switch (effect.type) {
    case ForceEffect::CONSTANT:
        if (!readu32l(stream, (u32)effect.constant.direction.type)   ||
            !readu32l(stream, (u32)effect.constant.direction.dir[0]) ||
            !readu32l(stream, (u32)effect.constant.direction.dir[1]) ||
            !readu32l(stream, (u32)effect.constant.direction.dir[2]) ||
            !readu32l(stream, (u32)effect.constant.length)           ||
            !readu16l(stream, (u16)effect.constant.delay)            ||
            !readu16l(stream, (u16)effect.constant.attack_length)    ||
            !readu16l(stream, (u16)effect.constant.attack_level)     ||
            !readu16l(stream, (u16)effect.constant.fade_length)      ||
            !readu16l(stream, (u16)effect.constant.fade_level)       ||
            !readu16l(stream, (u16)effect.constant.level))
        {
            goto throw_read_error;
        }
        break;
    case ForceEffect::RAMP:
        if (!readu32l(stream, (u32)effect.ramp.direction.type)   ||
            !readu32l(stream, (u32)effect.ramp.direction.dir[0]) ||
            !readu32l(stream, (u32)effect.ramp.direction.dir[1]) ||
            !readu32l(stream, (u32)effect.ramp.direction.dir[2]) ||
            !readu32l(stream, (u32)effect.ramp.length)           ||
            !readu16l(stream, (u16)effect.ramp.delay)            ||
            !readu16l(stream, (u16)effect.ramp.attack_length)    ||
            !readu16l(stream, (u16)effect.ramp.attack_level)     ||
            !readu16l(stream, (u16)effect.ramp.fade_length)      ||
            !readu16l(stream, (u16)effect.ramp.fade_level)       ||
            !readu16l(stream, (u16)effect.ramp.start)            ||
            !readu16l(stream, (u16)effect.ramp.end))
        {
            goto throw_read_error;
        }
        break;
    case ForceEffect::SINE:
    case ForceEffect::SQUARE:
    case ForceEffect::TRIANGLE:
    case ForceEffect::SAWTOOTHUP:
    case ForceEffect::SAWTOOTHDOWN:
        if (!readu32l(stream, (u32)effect.periodic.direction.type)   ||
            !readu32l(stream, (u32)effect.periodic.direction.dir[0]) ||
            !readu32l(stream, (u32)effect.periodic.direction.dir[1]) ||
            !readu32l(stream, (u32)effect.periodic.direction.dir[2]) ||
            !readu32l(stream, (u32)effect.periodic.length)           ||
            !readu16l(stream, (u16)effect.periodic.delay)            ||
            !readu16l(stream, (u16)effect.periodic.attack_length)    ||
            !readu16l(stream, (u16)effect.periodic.attack_level)     ||
            !readu16l(stream, (u16)effect.periodic.fade_length)      ||
            !readu16l(stream, (u16)effect.periodic.fade_level)       ||
            !readu16l(stream, (u16)effect.periodic.period)           ||
            !readu16l(stream, (u16)effect.periodic.magnitude)        ||
            !readu16l(stream, (u16)effect.periodic.offset)           ||
            !readu16l(stream, (u16)effect.periodic.phase))
        {
            goto throw_read_error;
        }
        break;
    default:
        return sq_throwerror(v, _SC("invalid force effect"));
    }

    if (!BindForceEffect(v, effect) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;

throw_read_error:
    return sq_throwerror(v, _SC("read error"));
}

//-----------------------------------------------------------------
bool IsForceEffect(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_FORCEEFFECT;
    }
    return false;
}

//-----------------------------------------------------------------
ForceEffect* GetForceEffect(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_FORCEEFFECT))) {
        return (ForceEffect*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindForceEffect(HSQUIRRELVM v, const ForceEffect& effect)
{
    sq_pushobject(v, g_forceeffect_class); // push forceeffect class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop forceeffect class
        return false;
    }
    sq_remove(v, -2); // pop forceeffect class
    SQUserPointer p = 0;
    sq_getinstanceup(v, -1, &p, 0);
    assert(p);
    memcpy(p, &effect, sizeof(ForceEffect));
    return true;
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
    sq_pushinteger(system::GetTime());
    return 1;
}

//-----------------------------------------------------------------
// GetTicks()
static SQInteger script_GetTicks(HSQUIRRELVM v)
{
    sq_pushinteger(system::GetTicks());
    return 1;
}

//-----------------------------------------------------------------
// GetRandom()
static SQInteger script_GetRandom(HSQUIRRELVM v)
{
    sq_pushinteger(system::GetRandom());
    return 1;
}

//-----------------------------------------------------------------
// Sleep(millis)
static SQInteger script_Sleep(HSQUIRRELVM v)
{
    // get millis
    SQInteger millis;
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <millis>"));
    }
    sq_getinteger(v, 2, &millis);

    system::Sleep(millis);
    return 0;
}

/******************************************************************
 *                                                                *
 *                             CRYPT                              *
 *                                                                *
 ******************************************************************/

/*********************** GLOBAL FUNCTIONS *************************/

//-----------------------------------------------------------------
// Base64Encode(in [, out])
static SQInteger script_Base64Encode(HSQUIRRELVM v)
{
    // get in
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <in>"));
    }
    Blob* in = GetBlob(v, 2);
    assert(in);
    if (in->getSize() == 0) {
        return sq_throwerror(v, _SC("invalid parameter <in>"));
    }

    if (sq_gettop(v) >= 3) {
        if (!IsBlob(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <out>"));
        }
        Blob* out = GetBlob(v, 3);
        assert(out);

        if (!crypt::Base64Encode(in->getBuffer(), in->getSize(), out)) {
            return sq_throwerror(v, _SC("base64 encoding failed"));
        }

        sq_push(v, 3);

    } else {
        BlobPtr out = Blob::Create();
        if (!out) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!crypt::Base64Encode(in->getBuffer(), in->getSize(), out.get())) {
            return sq_throwerror(v, _SC("base64 encoding failed"));
        }

        if (!BindBlob(v, out.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    }
    return 1;
}

//-----------------------------------------------------------------
// Base64Decode(in [, out])
static SQInteger script_Base64Decode(HSQUIRRELVM v)
{
    // get in
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <in>"));
    }
    Blob* in = GetBlob(v, 2);
    assert(in);
    if (in->getSize() == 0) {
        return sq_throwerror(v, _SC("invalid parameter <in>"));
    }

    if (sq_gettop(v) >= 3) {
        if (!IsBlob(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <out>"));
        }
        Blob* out = GetBlob(v, 3);
        assert(out);

        if (!crypt::Base64Decode(in->getBuffer(), in->getSize(), out)) {
            return sq_throwerror(v, _SC("base64 encoding failed"));
        }

        sq_push(v, 3);

    } else {
        BlobPtr out = Blob::Create();
        if (!out) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!crypt::Base64Decode(in->getBuffer(), in->getSize(), out.get())) {
            return sq_throwerror(v, _SC("base64 decoding failed"));
        }

        if (!BindBlob(v, out.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    }
    return 1;
}

//-----------------------------------------------------------------
// ComputeCRC32(in [, in_crc32 = 0])
static SQInteger script_ComputeCRC32(HSQUIRRELVM v)
{
    // get in
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <in>"));
    }
    Blob* in = GetBlob(v, 2);
    assert(in);
    if (in->getSize() == 0) {
        return sq_throwerror(v, _SC("invalid parameter <in>"));
    }

    SQInteger in_crc32 = 0;
    if (sq_gettop(v) >= 3) {
        if (sq_gettype(v, 3) != OT_INTEGER) {
            return sq_throwerror(v, _SC("invalid type of parameter <in_crc32>"));
        }
        sq_getinteger(v, 3, &in_crc32);
    }

    sq_pushinteger(v, crypt::ComputeCRC32(in->getBuffer(), in->getSize(), in_crc32));
    return 1;
}

/**************************** CIPHER ******************************/

//-----------------------------------------------------------------
static SQInteger script_cipher_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((ICipher*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Cipher(type)
static SQInteger script_cipher_constructor(HSQUIRRELVM v)
{
    // ensure that this function is only ever called on uninitialized Cipher instances
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    // get type
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <type>"));
    }
    SQInteger type;
    sq_getinteger(v, 2);

    This = crypt::CreateCipher(type);
    if (!This) {
        return sq_throwerror(v, _SC("failed to create cipher"));
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_cipher_destructor);
    return 0;
}

//-----------------------------------------------------------------
// Cipher.init(key)
static SQInteger script_cipher_init(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get key
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <key>"));
    }
    Blob* key = GetBlob(v, 2);
    assert(key);
    if (key->getSize() != 32) {
        return sq_throwerror(v, _SC("invalid parameter <key>"));
    }

    sq_pushbool(v, This->init(key->getBuffer()));
    return 1;
}

//-----------------------------------------------------------------
// Cipher.isInitialized()
static SQInteger script_cipher_isInitialized(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isInitialized());
    return 1;
}

//-----------------------------------------------------------------
// Cipher.getType()
static SQInteger script_cipher_getType(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getType());
    return 1;
}

//-----------------------------------------------------------------
// Cipher.getIV()
static SQInteger script_cipher_getIV(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    BlobPtr iv = Blob::Create();
    if (!iv || !This->getIV(iv)) {
        return sq_throwerror(v, _SC("failed to get IV"));
    }
    if (!BindBlob(v, iv.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Cipher.encrypt(in [, out])
static SQInteger script_cipher_encrypt(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get in
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <in>"));
    }
    Blob* in = GetBlob(v, 2);
    assert(in);
    if (in->getSize() == 0) {
        return sq_throwerror(v, _SC("invalid parameter <in>"));
    }

    if (sq_gettop(v) >= 3) {
        if (!IsBlob(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <out>"));
        }
        Blob* out = GetBlob(v, 3);
        assert(out);
        if (out->getSize() != in->getSize() && !out->resize(in->getSize())) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!This->encrypt(in->getBuffer(), out->getBuffer(), in->getSize())) {
            return sq_throwerror(v, _SC("encryption failed"));
        }

        sq_push(v, 3);

    } else {
        BlobPtr out = Blob::Create(in->getSize());
        if (!out) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!This->encrypt(in->getBuffer(), out->getBuffer(), in->getSize())) {
            return sq_throwerror(v, _SC("encryption failed"));
        }

        if (!BindBlob(v, out.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    }
    return 1;
}

//-----------------------------------------------------------------
// Cipher.decrypt(in [, out])
static SQInteger script_cipher_decrypt(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get in
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <in>"));
    }
    Blob* in = GetBlob(v, 2);
    assert(in);
    if (in->getSize() == 0) {
        return sq_throwerror(v, _SC("invalid parameter <in>"));
    }

    if (sq_gettop(v) >= 3) {
        if (!IsBlob(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <out>"));
        }
        Blob* out = GetBlob(v, 3);
        assert(out);
        if (out->getSize() != in->getSize() && !out->resize(in->getSize())) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!This->decrypt(in->getBuffer(), out->getBuffer(), in->getSize())) {
            return sq_throwerror(v, _SC("decryption failed"));
        }

        sq_push(v, 3);

    } else {
        BlobPtr out = Blob::Create(in->getSize());
        if (!out) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!This->decrypt(in->getBuffer(), out->getBuffer(), in->getSize())) {
            return sq_throwerror(v, _SC("decryption failed"));
        }

        if (!BindBlob(v, out.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    }
    return 1;
}

//-----------------------------------------------------------------
// Cipher._typeof()
static SQInteger script_cipher__typeof(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Cipher"), -1);
    return 1;
}

//-----------------------------------------------------------------
// Cipher._tostring()
static SQInteger script_cipher__tostring(HSQUIRRELVM v)
{
    ICipher* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CIPHER))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<Cipher instance>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
bool IsCipher(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_CIPHER;
    }
    return false;
}

//-----------------------------------------------------------------
ICipher* GetCipher(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_CIPHER))) {
        return (ICipher*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindCipher(HSQUIRRELVM v, ICipher* cipher)
{
    if (!cipher) {
        return false;
    }
    sq_pushobject(v, g_cipher_class); // push cipher class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop cipher class
        return false;
    }
    sq_remove(v, -2); // pop cipher class
    sq_setreleasehook(v, -1, script_cipher_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)cipher);
    cipher->grab(); // grab a new reference
    return true;
}

/***************************** HASH *******************************/

//-----------------------------------------------------------------
static SQInteger script_hash_destructor(SQUserPointer p, SQInteger size)
{
    assert(p);
    ((IHash*)p)->drop();
    return 0;
}

//-----------------------------------------------------------------
// Hash(type)
static SQInteger script_hash_constructor(HSQUIRRELVM v)
{
    // ensure that this function is only ever called on uninitialized Hash instances
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    // get type
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <type>"));
    }
    SQInteger type;
    sq_getinteger(v, 2);

    This = crypt::CreateHash(type);
    if (!This) {
        return sq_throwerror(v, _SC("failed to create hash"));
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_hash_destructor);
    return 0;
}

//-----------------------------------------------------------------
// Hash.init()
static SQInteger script_hash_init(HSQUIRRELVM v)
{
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->init());
    return 1;
}

//-----------------------------------------------------------------
// Hash.isInitialized()
static SQInteger script_hash_isInitialized(HSQUIRRELVM v)
{
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isInitialized());
    return 1;
}

//-----------------------------------------------------------------
// Hash.getType()
static SQInteger script_hash_getType(HSQUIRRELVM v)
{
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getType());
    return 1;
}

//-----------------------------------------------------------------
// Hash.process(in)
static SQInteger script_hash_encrypt(HSQUIRRELVM v)
{
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get in
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <in>"));
    }
    Blob* in = GetBlob(v, 2);
    assert(in);
    if (in->getSize() == 0) {
        return sq_throwerror(v, _SC("invalid parameter <in>"));
    }

    sq_pushbool(v, This->process(in->getBuffer(), in->getSize()));
    return 1;
}

//-----------------------------------------------------------------
// Hash.finish()
static SQInteger script_hash_encrypt(HSQUIRRELVM v)
{
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    BlobPtr result = Blob::Create();
    if (!result) {
        return sq_throwerror(v, _SC("out of memory"));
    }

    if (!This->finish(result.get())) {
        return sq_throwerror(v, _SC("finish failed"));
    }

    if (!BindBlob(v, result.get())) {
        return sq_throwerror(v, _SC("internal error"));
    }
    return 1;
}

//-----------------------------------------------------------------
// Hash._typeof()
static SQInteger script_hash__typeof(HSQUIRRELVM v)
{
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("Hash"), -1);
    return 1;
}

//-----------------------------------------------------------------
// Hash._tostring()
static SQInteger script_hash__tostring(HSQUIRRELVM v)
{
    IHash* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_HASH))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<Hash instance>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
bool IsHash(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_HASH;
    }
    return false;
}

//-----------------------------------------------------------------
IHash* GetHash(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_HASH))) {
        return (IHash*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindHash(HSQUIRRELVM v, IHash* hash)
{
    if (!hash) {
        return false;
    }
    sq_pushobject(v, g_hash_class); // push hash class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop hash class
        return false;
    }
    sq_remove(v, -2); // pop hash class
    sq_setreleasehook(v, -1, script_hash_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)hash);
    hash->grab(); // grab a new reference
    return true;
}

/******************************************************************
 *                                                                *
 *                          COMPRESSION                           *
 *                                                                *
 ******************************************************************/

/**************************** ZSTREAM *****************************/

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
    // ensure that this function is only ever called on uninitialized ZStream instances
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM)) || This != 0) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(!This);

    // get type
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <type>"));
    }
    SQInteger type;
    sq_getinteger(v, 2);

    This = ZStream::Create();
    if (!This) {
        return sq_throwerror(v, _SC("failed to create zstream"));
    }
    sq_setinstanceup(v, 1, (SQUserPointer)This);
    sq_setreleasehook(v, 1, script_zstream_destructor);
    return 0;
}

//-----------------------------------------------------------------
// ZStream.init()
static SQInteger script_zstream_init(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->init());
    return 1;
}

//-----------------------------------------------------------------
// ZStream.isInitialized()
static SQInteger script_zstream_isInitialized(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushbool(v, This->isInitialized());
    return 1;
}

//-----------------------------------------------------------------
// ZStream.getMode()
static SQInteger script_zstream_getMode(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getMode());
    return 1;
}

//-----------------------------------------------------------------
// ZStream.getBufferSize()
static SQInteger script_zstream_getBufferSize(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushinteger(v, This->getBufferSize());
    return 1;
}

//-----------------------------------------------------------------
// ZStream.setBufferSize(size)
static SQInteger script_zstream_setBufferSize(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get size
    if (sq_gettype(v, 2) != OT_INTEGER) {
        return sq_throwerror(v, _SC("invalid type of parameter <size>"));
    }
    SQInteger size;
    sq_getinteger(v, 2, &size);

    sq_pushbool(v, This->setBufferSize(size));
    return 1;
}

//-----------------------------------------------------------------
// ZStream.process(in [, out])
static SQInteger script_zstream_process(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    // get in
    if (!IsBlob(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <in>"));
    }
    Blob* in = GetBlob(v, 2);
    assert(in);
    if (in->getSize() == 0) {
        return sq_throwerror(v, _SC("invalid parameter <in>"));
    }

    if (sq_gettop(v) >= 3) {
        if (!IsBlob(v, 3)) {
            return sq_throwerror(v, _SC("invalid type of parameter <out>"));
        }
        Blob* out = GetBlob(v, 3);
        assert(out);

        if (!This->process(in->getBuffer(), in->getSize(), out)) {
            return sq_throwerror(v, _SC("processing failed"));
        }

        sq_push(v, 3);

    } else {
        BlobPtr out = Blob::Create();
        if (!out) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!This->process(in->getBuffer(), in->getSize(), out.get())) {
            return sq_throwerror(v, _SC("processing failed"));
        }

        if (!BindBlob(v, out.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    }
    return 1;
}

//-----------------------------------------------------------------
// ZStream.finish([out])
static SQInteger script_zstream_finish(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    if (sq_gettop(v) >= 2) {
        if (!IsBlob(v, 2)) {
            return sq_throwerror(v, _SC("invalid type of parameter <out>"));
        }
        Blob* out = GetBlob(v, 2);
        assert(out);

        if (!This->finish(out)) {
            return sq_throwerror(v, _SC("finishing failed"));
        }

        sq_push(v, 2);

    } else {
        BlobPtr out = Blob::Create();
        if (!out) {
            return sq_throwerror(v, _SC("out of memory"));
        }

        if (!This->finish(out.get())) {
            return sq_throwerror(v, _SC("finishing failed"));
        }

        if (!BindBlob(v, out.get())) {
            return sq_throwerror(v, _SC("internal error"));
        }
    }
    return 1;
}

//-----------------------------------------------------------------
// ZStream._typeof()
static SQInteger script_zstream__typeof(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    sq_pushstring(v, _SC("ZStream"), -1);
    return 1;
}

//-----------------------------------------------------------------
// ZStream._tostring()
static SQInteger script_zstream__tostring(HSQUIRRELVM v)
{
    ZStream* This = 0;
    if (!SQ_SUCCEEDED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) {
        return sq_throwerror(v, _SC("invalid environment object"));
    }
    assert(This);

    std::ostringstream oss;
    oss << "<ZStream instance>";
    sq_pushstring(v, oss.str().c_str(), -1);
    return 1;
}

//-----------------------------------------------------------------
bool IsZStream(HSQUIRRELVM v, SQInteger idx)
{
    if (sq_gettype(v, idx) == OT_INSTANCE) {
        SQUserPointer tt;
        sq_gettypetag(v, idx, &tt);
        return tt == TT_ZSTREAM;
    }
    return false;
}

//-----------------------------------------------------------------
ZStream* GetZStream(HSQUIRRELVM v, SQInteger idx)
{
    SQUserPointer p = 0;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_ZSTREAM))) {
        return (ZStream*)p;
    }
    return 0;
}

//-----------------------------------------------------------------
bool BindZStream(HSQUIRRELVM v, ZStream* stream)
{
    if (!stream) {
        return false;
    }
    sq_pushobject(v, g_zstream_class); // push zstream class
    if (!SQ_SUCCEEDED(sq_createinstance(v, -1))) {
        sq_poptop(v); // pop zstream class
        return false;
    }
    sq_remove(v, -2); // pop zstream class
    sq_setreleasehook(v, -1, script_zstream_destructor);
    sq_setinstanceup(v, -1, (SQUserPointer)stream);
    stream->grab(); // grab a new reference
    return true;
}

/******************************************************************
 *                                                                *
 *                              SCRIPT                            *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------------------
struct STREAMSOURCE {
    IStream* stream;
    int remaining;
};

static SQInteger lexfeed_callback(SQUserPointer p)
{
    char c;
    IStream* stream = (IStream*)p;
    if (stream->read(&c, sizeof(c)) == sizeof(c)) {
        return c;
    }
    return 0;
}

static SQInteger ss_lexfeed_callback(SQUserPointer p)
{
    char c;
    STREAMSOURCE* ss = (STREAMSOURCE*)p;
    if (ss->remaining > 0 && ss->stream->read(&c, sizeof(c)) == sizeof(c)) {
        ss->remaining -= sizeof(c);
        return c;
    }
    return 0;
}

static bool compile_stream(HSQUIRRELVM v, const std::string& name, IStream* stream, int size = -1, bool raiseerror = true)
{
    assert(stream);
    if (stream) {
        if (size > 0) {
            STREAMSOURCE ss = {stream, size};
            return SQ_SUCCEEDED(sq_compile(v, ss_lexfeed_callback, &ss, name.c_str(), (raiseerror ? SQTrue : SQFalse)));
        } else {
            return SQ_SUCCEEDED(sq_compile(v, lexfeed_callback, stream, name.c_str(), (raiseerror ? SQTrue : SQFalse)));
        }
    }
    return false;
}

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
// Compile(source [, name = <...>, pos = 0, n = -1])
static SQInteger script_CompileStream(HSQUIRRELVM v)
{
    if (sq_gettype(v, 2) == OT_STRING) {
        // get source
        const SQChar* source = 0;
        sq_getstring(v, 2, &source);
        int source_length = sq_getsize(v, 2);
        if (source_length == 0) { // can't compile an empty string
            return sq_throwerror(v, _SC("invalid parameter <source>"));
        }

        // get optional name
        const SQChar* name = _SC("<string source>");
        if (sq_gettop(v) >= 3) {
            if (sq_gettype(v, 3) != OT_STRING) {
                return sq_throwerror(v, _SC("invalid type of parameter <name>"));
            }
            sq_getstring(v, 3, &name);
        }

        // get optional pos
        SQInteger pos = 0;
        if (sq_gettop(v) >= 4) {
            if (sq_gettype(v, 4) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <pos>"));
            }
            sq_getinteger(v, 4, &pos);
        }
        if (pos < 0 || pos >= source_length) {
            return sq_throwerror(v, _SC("invalid parameter <pos>"));
        }

        // get optional n
        SQInteger n = -1;
        if (sq_gettop(v) >= 5) {
            if (sq_gettype(v, 5) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <n>"));
            }
            sq_getinteger(v, 5, &n);
        }
        if (n < 0) {
            n = source_length - pos;
        } else if (n > 0) {
            n = std::min(n, source_length - pos);
        } else {
            return sq_throwerror(v, _SC("invalid parameter <n>"));
        }

        // compile
        if (!compile_buffer(v, name, source + pos, n)) {
            return sq_throwerror(v, _SC("failed to compile"));
        }

    } else if (IsBlob(v, 2)) {
        // get source
        Blob* source = GetBlob(v, 2);
        assert(source);
        if (source->getSize() == 0) { // can't compile an empty blob
            return sq_throwerror(v, _SC("invalid parameter <source>"));
        }

        // get optional name
        const SQChar* name = _SC("<blob source>");
        if (sq_gettop(v) >= 3) {
            if (sq_gettype(v, 3) != OT_STRING) {
                return sq_throwerror(v, _SC("invalid type of parameter <name>"));
            }
            sq_getstring(v, 3, &name);
        }

        // get optional pos
        SQInteger pos = 0;
        if (sq_gettop(v) >= 4) {
            if (sq_gettype(v, 4) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <pos>"));
            }
            sq_getinteger(v, 4, &pos);
        }
        if (pos < 0 || pos >= source->getSize()) {
            return sq_throwerror(v, _SC("invalid parameter <pos>"));
        }

        // get optional n
        SQInteger n = -1;
        if (sq_gettop(v) >= 5) {
            if (sq_gettype(v, 5) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <n>"));
            }
            sq_getinteger(v, 5, &n);
        }
        if (n < 0) {
            n = source->getSize() - pos;
        } else if (n > 0) {
            n = std::min(n, source->getSize() - pos);
        } else {
            return sq_throwerror(v, _SC("invalid parameter <n>"));
        }

        // compile
        if (!compile_buffer(v, name, source->getBuffer() + pos, n)) {
            return sq_throwerror(v, _SC("failed to compile"));
        }

    } else if (IsFile(v, 2)) {
        // get source
        IFile* source = GetFile(v, 2);
        assert(source);
        if (!source->isOpen() || !source->isReadable()) {
            return sq_throwerror(v, _SC("invalid parameter <source>"));
        }

        // get optional name
        const SQChar* name = (const SQChar*)source->getName().c_str(); // default is the filename
        if (sq_gettop(v) >= 3) {
            if (sq_gettype(v, 3) != OT_STRING) {
                return sq_throwerror(v, _SC("invalid type of parameter <name>"));
            }
            sq_getstring(v, 3, &name);
        }

        // ignore optional pos

        // get optional n
        SQInteger n = -1;
        if (sq_gettop(v) >= 5) {
            if (sq_gettype(v, 5) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <n>"));
            }
            sq_getinteger(v, 5, &n);
        }

        // compile
        if (!compile_stream(v, name, source, n)) {
            return sq_throwerror(v, _SC("failed to compile"));
        }

    } else { // assume a stream
        // get source
        if (!IsStream(v, 2)) {
            return sq_throwerror(v, _SC("invalid type of parameter <source>"));
        }
        IStream* source = GetStream(v, 2);
        assert(source);
        if (!source->isOpen() || !source->isReadable()) {
            return sq_throwerror(v, _SC("invalid parameter <source>"));
        }

        // get optional name
        const SQChar* name = _SC("<stream source>");
        if (sq_gettop(v) >= 3) {
            if (sq_gettype(v, 3) != OT_STRING) {
                return sq_throwerror(v, _SC("invalid type of parameter <name>"));
            }
            sq_getstring(v, 3, &name);
        }

        // ignore optional pos

        // get optional n
        SQInteger n = -1;
        if (sq_gettop(v) >= 5) {
            if (sq_gettype(v, 5) != OT_INTEGER) {
                return sq_throwerror(v, _SC("invalid type of parameter <n>"));
            }
            sq_getinteger(v, 5, &n);
        }

        // compile
        if (!compile_stream(v, name, source, n)) {
            return sq_throwerror(v, _SC("failed to compile"));
        }
    }
    return 1;
}

//-----------------------------------------------------------------
IFile* open_script_file(const std::string& filename)
{
    assert(!filename.empty());

    // filename exists and has either no extension or an unknown extension, assume it's a compiled script
    if (fs::exists(filename)) {
        std::string fn_ext(fs::path(filename).extension());
        if (fn_ext.empty() || (fn_ext != SCRIPT_FILE_EXT && fn_ext != COMPILED_SCRIPT_FILE_EXT)) {
            // deserialize
            fileptr f = file::create();
            if (f->open(filename, file::IN)) {
                return SQ_SUCCEEDED(sq_readclosure(v, _closurereadfunc, f.get()));
            }
            return false;
        }
    }

    fs::path  nut_file_path(fs::path(filename).replace_extension(SCRIPT_FILE_EXT));
    fs::path nutc_file_path(fs::path(filename).replace_extension(COMPILED_SCRIPT_FILE_EXT));

    if (fs::exists(nutc_file_path)) {
        // deserialize
        fileptr nutc_file = file::create();
        if (nutc_file->open(nutc_file_path.string(), file::IN)) {
            return SQ_SUCCEEDED(sq_readclosure(v, _closurereadfunc, nutc_file.get()));
        }
    } else if (fs::exists(nut_file_path)) {
        fileptr nut_file = file::create();
        if (nut_file->open(nut_file_path.string(), file::IN)) {
            // compile
            if (compilestream(v, filename, nut_file.get())) {
                return true;
            }
        }
    }
    return false;
}

//-----------------------------------------------------------------
// Require(name)
static SQInteger script_RequireScript(HSQUIRRELVM v)
{
    // get name
    const SQChar* name = 0;
    if (sq_gettype(v, 2) != OT_STRING) {
        return sq_throwerror(v, _SC("invalid type of parameter <name>"));
    }
    sq_getstring(v, 2, &name);
    if (sq_getsize(v, 2) == 0) { // empty string
        return sq_throwerror(v, _SC("invalid parameter <name>"));
    }

    // get script name without file extension
    std::string script_name(name);
    size_t dot_pos = script_name.rfind('.');
    if (dot_pos != std::string::npos) {
        script_name.erase(dot_pos);
    }

    // see if the script is already loaded
    for (int i = 0; i < (int)g_loaded_scripts.size(); ++i) {
        if (g_loaded_scripts[i] == script_name) {
            return 0; // already loaded so nothing to do here
        }
    }

    // load script
    if (!load_script(v, name)) {
        // try to open from the system directory
        if (!load_script(v, GetInitialPath() + "/system/script/" + name)) {
            return sq_throwerror(v, _SC("failed to load script"));
        }
    }

    "bla.nut"
    "foo/bar.bin"
    "/system/script/foo.nut"






    std::string filename(pfilename);

    // split filename into name and extension
    std::string name(filename);
    std::string extension;
    size_t dot_pos = name.rfind('.');
    if (dot_pos != std::string::npos) {
        extension = name.substr(dot_pos);
        name.erase(dot_pos);
    }


    // open file
    FilePtr file = OpenFile(filename);
    if (!file) {
        return sq_throwerror(v, _SC("failed to open file"));
    }

    // load script
    if (extension == ".sphere-script") { // assume it's plain text
        if (!compile_stream(v, filename, file.get())) {
            return sq_throwerror(v, _SC("failed to compile script"));
        }
    } else { // assume it's bytecode
        if (!unmarshal_object(v, file.get())) {
            return sq_throwerror(v, _SC("failed to load bytecode"));
        }
    }

    if (!x::script::loadfile(v, name)) {
        // see if there is a system script with that name
        if (!x::script::loadfile(v, (fs::initial_path() / "system/scripts" / name).string())) {
            THROW1("could not import script '%s'", name);
        }
    }


    if (_nvars >= 3 && sq_gettype(v, 3) == OT_TABLE) {
        sq_push(v, 3);
    } else {
        sq_pushroottable(v);
    }
    sq_call(v, 1, SQFalse, SQTrue);
    sq_poptop(v); // pop closure
    if (_nvars >= 3 && sq_gettype(v, 3) == OT_TABLE) {
        sq_push(v, 3);
    } else {
        sq_pushroottable(v);
    }
    return 1;
}

/******************************************************************
 *                                                                *
 *                        SERIALIZATION                           *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------------------
static bool ObjectToJSON(HSQUIRRELVM v, SQInteger idx)
{
    if (idx < 0) { // make any negative indices positive to reduce complexity
        idx = (sq_gettop(v) + 1) + idx;
    }
    switch (sq_gettype(v, idx)) {
    case OT_NULL: {
        sq_pushstring(v, _SC("null"), -1);
        return true;
    }
    case OT_BOOL: {
        SQBool b;
        sq_getbool(v, idx, &b);
        sq_pushstring(v, ((b == SQTrue) ? _SC("true") : _SC("false")), -1);
        return true;
    }
    case OT_INTEGER: {
        SQInteger i;
        sq_getinteger(v, idx, &i);
        std::ostringstream oss;
        oss << i;
        sq_pushstring(v, oss.str().c_str(), -1);
        return true;
    }
    case OT_FLOAT: {
        SQFloat f;
        sq_getfloat(v, idx, &f);
        std::ostringstream oss;
        oss << f;
        sq_pushstring(v, oss.str().c_str(), -1);
        return true;
    }
    case OT_STRING: {
        const SQChar* s = 0;
        sq_getstring(v, idx, &s);
        std::ostringstream oss;
        oss << "\"" << s << "\"";
        sq_pushstring(v, oss.str().c_str(), -1);
        return true;
    }
    case OT_TABLE: {
        const SQChar* temp = 0;
        int oldtop = sq_gettop(v);
        std::ostringstream oss;
        oss << "{";
        sq_pushnull(v); // will be substituted with an iterator by squirrel
        bool appendcomma = false;
        while (SQ_SUCCEEDED(sq_next(v, idx))) {
            if (appendcomma) {
                oss << ",";
            } else {
                appendcomma = true;
            }
            int top = sq_gettop(v);
            if (sq_gettype(v, top-1) != OT_STRING) { // key must be a string
                sq_settop(v, oldtop);
                return false;
            }
            sq_getstring(v, top-1, &temp);
            oss << "\"" << temp << "\":";
            if (sq_gettype(v, top) == OT_STRING) {
                sq_getstring(v, top, &temp);
                oss << "\"" << temp << "\"";
            } else {
                if (!ObjectToJSON(v, top)) { // tojson value
                    sq_settop(v, oldtop);
                    return false;
                }
                sq_getstring(v, top+1, &temp);
                oss << temp;
                sq_poptop(v); // pop json value
            }
            sq_pop(v, 2); // pop key, value
        }
        sq_poptop(v); // pop iterator
        oss << "}";
        sq_pushstring(v, oss.str().c_str(), -1);
        return true;
    }
    case OT_ARRAY: {
        const SQChar* temp = 0;
        int oldtop = sq_gettop(v);
        std::ostringstream oss;
        oss << "[";
        sq_pushnull(v); // will be substituted with an iterator by squirrel
        bool appendcomma = false;
        while (SQ_SUCCEEDED(sq_next(v, idx))) {
            if (appendcomma) {
                oss << ",";
            } else {
                appendcomma = true;
            }
            if (sq_gettype(v, idx+3) == OT_STRING) {
                sq_getstring(v, idx+3, &temp);
                oss << "\"" << temp << "\"";
            } else {
                if (!ObjectToJSON(v, idx+3)) { // tojson value
                    sq_settop(v, oldtop);
                    return false;
                }
                sq_getstring(v, idx+4, &temp);
                oss << temp;
                sq_poptop(v); // pop json value
            }
            sq_pop(v, 2); // pop key, value
        }
        sq_poptop(v); // pop iterator
        oss << "]";
        sq_pushstring(v, oss.str().c_str(), -1);
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
    if (!ObjectToJSON(v, 2)) {
        return sq_throwerror(v, _SC("conversion to JSON failed"));
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
static bool DumpObject(HSQUIRRELVM v, SQInteger idx, IStream* stream)
{
    assert(stream);
    if (!stream || !stream->isWriteable()) {
        return false;
    }
    if (idx < 0) { // make any negative indices positive to reduce complexity
        idx = (sq_gettop(v) + 1) + idx;
    }
    switch (sq_gettype(v, idx)) {
    case OT_NULL: {
        return writeu32l(stream, MARSHAL_MAGIC_NULL);
    }
    case OT_INTEGER: {
        SQInteger i;
        sq_getinteger(v, idx, &i);
        if (!writeu32l(stream, MARSHAL_MAGIC_INTEGER) ||
            !writeu32l(stream, (u32)i))
        {
            return false;
        }
        return true;
    }
    case OT_BOOL: {
        SQBool b;
        sq_getbool(v, idx, &b);
        return writeu32l(stream, ((b == SQTrue) ? MARSHAL_MAGIC_BOOL_TRUE : MARSHAL_MAGIC_BOOL_FALSE));
    }
    case OT_FLOAT: {
        SQFloat f;
        sq_getfloat(v, idx, &f);
        int pos = stream->tell();
        if (!writeu32l(stream, MARSHAL_MAGIC_FLOAT) ||
            !writef32l(stream, (f32)f))
        {
            return false;
        }
        return true;
    }
    case OT_STRING: {
        const SQChar* s = 0;
        sq_getstring(v, idx, &s);
        u32 numbytes = (u32)sq_getsize(v, idx); // assumes sizeof(SQChar) == 1
        if (!writeu32l(stream, MARSHAL_MAGIC_STRING) ||
            !writeu32l(stream, numbytes))
        {
            return false;
        }
        if (numbytes > 0) {
            return stream->write(s, numbytes) == numbytes;
        }
        return true;
    }
    case OT_TABLE: {
        int oldtop = sq_gettop(v);
        if (!writeu32l(stream, MARSHAL_MAGIC_TABLE) ||
            !writeu32l(stream, sq_getsize(v, idx)))
        {
            return false;
        }
        sq_pushnull(v); // will be substituted with an iterator by squirrel
        while (SQ_SUCCEEDED(sq_next(v, idx))) {
            if (!DumpObject(v, stream, sq_gettop(v)-1) || // marshal key
                !DumpObject(v, stream, sq_gettop(v)))     // marshal value
            {
                sq_settop(v, oldtop);
                return false;
            }
            sq_pop(v, 2); // pop key and value
        }
        sq_poptop(v); // pop iterator
        return true;
    }
    case OT_ARRAY: {
        int oldtop = sq_gettop(v);
        if (!writeu32l(stream, MARSHAL_MAGIC_ARRAY) ||
            !writeu32l(stream, sq_getsize(v, idx)))
        {
            return false;
        }
        sq_pushnull(v); // will be substituted with an iterator by squirrel
        while (SQ_SUCCEEDED(sq_next(v, idx))) {
            if (!DumpObject(v, stream, sq_gettop(v))) { // marshal value
                sq_settop(v, oldtop);
                return false;
            }
            sq_pop(v, 2); // pop key and value
        }
        sq_poptop(v); // pop iterator
        return true;
    }
    case OT_CLOSURE: {
        if (!writeu32l(stream, MARSHAL_MAGIC_CLOSURE)) {
            return false;
        }
        sq_push(v, idx);
        bool sqsucceeded = SQ_SUCCEEDED(sq_writeclosure(v, write_closure_callback, stream));
        sq_poptop(v);
        return sqsucceeded;
    }
    case OT_INSTANCE: {
        int oldtop = sq_gettop(v);
        if (!writeu32l(stream, MARSHAL_MAGIC_INSTANCE)) {
            return false;
        }
        sq_getclass(v, idx);
        SQUserPointer tt = 0;
        sq_gettypetag(v, -1, &tt);
#ifdef _SQ64
        if (!writeu64l(stream, (u64)tt)) {
#else
        if (!writeu32l(stream, (u32)tt)) {
#endif
            sq_settop(v, oldtop);
            return false;
        }
        sq_pushstring(v, _SC("_dump"), -1);
        if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
            sq_settop(v, oldtop);
            return false;
        }
        if (sq_gettype(v, -1) != OT_CLOSURE &&
            sq_gettype(v, -1) != OT_NATIVECLOSURE)
        {
            sq_settop(v, oldtop);
            return false;
        }
        sq_pushroottable(v); // this
        sq_push(v, idx); // push the instance to marshal
        if (!BindStream(v, stream)) { // push the output stream
            sq_settop(v, oldtop);
            return false;
        }
        if (!SQ_SUCCEEDED(sq_call(v, 3, SQTrue, SQTrue))) {
            sq_settop(v, oldtop);
            return false;
        }
        if (sq_gettype(v, -1) != OT_BOOL) {
            sq_settop(v, oldtop);
            return false;
        }
        SQBool retval;
        sq_getbool(v, -1, &retval);
        sq_settop(v, oldtop);
        return retval == SQTrue;
    }
    default:
        return false;
    }
}

//-----------------------------------------------------------------
// DumpObject(object, stream)
static SQInteger script_DumpObject(HSQUIRRELVM v)
{
    // get stream
    if (!IsStream(v, 3)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 3);
    assert(stream);
    if (!stream->isOpen() || !stream->isWriteable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // marshal
    if (!DumpObject(v, 2, stream)) {
        return sq_throwerror(v, _SC("serialization failed"));
    }
    return 0;
}

//-----------------------------------------------------------------
static SQInteger read_closure_callback(SQUserPointer p, SQUserPointer buf, SQInteger count)
{
    IStream* stream = (IStream*)p;
    return stream->read(buf, count);
}

//-----------------------------------------------------------------
static bool LoadObject(HSQUIRRELVM v, IStream* stream)
{
    assert(stream);
    if (!stream || !stream->isReadable()) {
        return false;
    }

    // read type
    u32 type;
    if (!readu32l(stream, type)) {
        return false;
    }

    switch (type) {
    case MARSHAL_MAGIC_NULL: {
        sq_pushnull(v);
        return true;
    }
    case MARSHAL_MAGIC_BOOL_TRUE: {
        sq_pushbool(v, SQTrue);
        return true;
    }
    case MARSHAL_MAGIC_BOOL_FALSE: {
        sq_pushbool(v, SQFalse);
        return true;
    }
    case MARSHAL_MAGIC_INTEGER: {
#ifdef _SQ64
        u64 i;
        if (!readu64l(stream, i)) {
#else
        u32 i;
        if (!readu32l(stream, i)) {
#endif
            return false;
        }
        sq_pushinteger(v, (SQInteger)i);
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
        sq_pushfloat(v, f);
        return true;
    }
    case MARSHAL_MAGIC_STRING: {
        u32 numbytes; // equals to length of string if sizeof(SQChar) == 1
        if (!readu32l(stream, numbytes)) {
            return false;
        }
        if (numbytes == 0) {
            sq_pushstring(v, _SC(""), -1);
            return true; // ok, empty string
        }
        if (numbytes % sizeof(SQChar) != 0) {
            return false;
        }
        ArrayPtr<u8> buf = new u8[numbytes];
        if (!stream->read(buf.get(), numbytes)) {
            return false;
        }
        sq_pushstring(v, (const SQChar*)buf.get(), numbytes); // assuming sizeof(SQChar) == 1
        return true;
    }
    case MARSHAL_MAGIC_TABLE: {
        int oldtop = sq_gettop(v);
        u32 size;
        if (!readu32l(stream, size)) {
            return false;
        }
        sq_newtable(v);
        for (u32 i = 0; i < size; ++i) {
            if (!LoadObject(v, stream) || // unmarshal key
                !LoadObject(v, stream))   // unmarshal value
            {
                sq_settop(v, oldtop);
                return false;
            }
            sq_newslot(v, -3, SQFalse);
        }
        return true;
    }
    case MARSHAL_MAGIC_ARRAY: {
        int oldtop = sq_gettop(v);
        u32 size;
        if (!readu32l(stream, size)) {
            return false;
        }
        sq_newarray(v, 0);
        for (u32 i = 0; i < size; ++i) {
            if (!LoadObject(v, stream)) { // unmarshal value
                sq_settop(v, oldtop);
                return false;
            }
            sq_arrayappend(v, -2);
        }
        return true;
    }
    case MARSHAL_MAGIC_CLOSURE: {
        return SQ_SUCCEEDED(sq_readclosure(v, read_closure_callback, stream));
    }
    case MARSHAL_MAGIC_INSTANCE: {
        int oldtop = sq_gettop(v);
#ifdef _SQ64
        u64 tt;
        if (!readu64l(stream, tt)) {
#else
        u32 tt;
        if (!readu32l(stream, tt)) {
#endif
            return false;
        }
        u32 numbytes; // equals to length of the string if sizeof(SQChar) == 1
        if (!readu32l(stream, numbytes)) {
            return false;
        }
        ArrayPtr<u8> buf = new u8[numbytes];
        if (stream->read(buf.get(), numbytes) != numbytes) {
            return false;
        }
        sq_pushroottable(v);
        sq_pushstring(v, (const SQChar*)buf.get(), numbytes); // assuming sizeof(SQChar) == 1
        if (!SQ_SUCCEEDED(sq_get(v, -2))) {
            sq_settop(v, oldtop);
            return false;
        }
        if (sq_gettype(v, -1) != OT_CLASS) {
            sq_settop(v, oldtop);
            return false;
        }
        SQUserPointer class_tt = 0;
        sq_gettypetag(v, -1, &class_tt);
        if ((SQUserPointer)tt != class_tt) {
            sq_settop(v, oldtop);
            return false;
        }
        sq_pushstring(v, _SC("_load"), -1);
        if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
            sq_settop(v, oldtop);
            return false;
        }
        if (sq_gettype(v, -1) != OT_CLOSURE &&
            sq_gettype(v, -1) != OT_NATIVECLOSURE)
        {
            sq_settop(v, oldtop);
            return false;
        }
        sq_pushroottable(v); // this
        if (!BindStream(v, stream)) { // push the input stream
            sq_settop(v, oldtop);
            return false;
        }
        if (!SQ_SUCCEEDED(sq_call(v, 2, SQTrue, SQTrue))) {
            sq_settop(v, oldtop);
            return false;
        }
        if (sq_gettype(v, -1) != OT_INSTANCE) {
            sq_settop(v, oldtop);
            return false;
        }
        int numtopop = (sq_gettop(v) - oldtop) - 1;
        while (numtopop > 0) {
            sq_remove(v, -2);
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
    // get stream
    if (!IsStream(v, 2)) {
        return sq_throwerror(v, _SC("invalid type of parameter <stream>"));
    }
    IStream* stream = GetStream(v, 2);
    assert(stream);
    if (!stream->isOpen() || !stream->isReadable()) {
        return sq_throwerror(v, _SC("invalid parameter <stream>"));
    }

    // load
    if (!LoadObject(v, stream)) {
        return sq_throwerror(v, _SC("deserialization failed"));
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
    assert(g_log);
    g_log->error() << "Script error in '" << source << "' line " << line << ": " << desc;
}

//-----------------------------------------------------------------
static SQInteger runtime_error_handler(HSQUIRRELVM v)
{
    int top = sq_gettop(v);
    sq_tostring(v, 2);
    const SQChar* error = _SC("N/A");
    sq_getstring(v, -1, &error);
    assert(g_log);
    g_log->error() << "Script error: " << error;
    sq_settop(v, top);
    return 0;
}

//-----------------------------------------------------------------
static void print_func(HSQUIRRELVM v, const SQChar* format, ...)
{
    static int bufsize = 256;
    static ArrayPtr<char> buf = new char[bufsize];
    va_list arglist;
    va_start(arglist, format);
    int size = vsnprintf(buf.get(), bufsize, format, arglist);
    while (size < 0 || size >= bufsize) {
        bufsize = bufsize * 2;
        buf = new char[bufsize];
        size = vsnprintf(buf.get(), bufsize, format, arglist);
    }
    va_end(arglist);
    assert(g_log);
    g_log->debug() << "Print: " << (const char*)buf.get();
}

namespace script {

    namespace internal {

        //-----------------------------------------------------------------
        bool InitScript(const Log& log)
        {
            assert(!g_sqvm);

            // print Squirrel version
            log.info() << "Using Squirrel " << (SQUIRREL_VERSION_NUMBER / 100)       \
                                        "." << ((SQUIRREL_VERSION_NUMBER / 10) % 10) \
                                        "." << (SQUIRREL_VERSION_NUMBER % 10);

            log.info() << "Opening Squirrel VM";
            g_sqvm = sq_open(1024);
            if (!g_sqvm) {
                log.error("Failed opening Squirrel VM");
                return false;
            }
            sq_setcompilererrorhandler(g_sqvm, compiler_error_handler);
            sq_newclosure(g_sqvm, runtime_error_handler, 0);
            sq_seterrorhandler(g_sqvm);
            sq_setprintfunc(g_sqvm, print_func, print_func);

            sq_pushroottable(g_sqvm); // push root table

            // register standard math library
            log.info() << "Registering Squirrel's standard math library";
            if (!SQ_SUCCEEDED(sqstd_register_mathlib(g_sqvm)) {
                log.error("Failed registering Squirrel's standard math library");
                goto lib_init_failed;
            }

            // register standard string library (string formatting and regular expression matching routines)
            log.info() << "Registering Squirrel's standard string library";
            if (!SQ_SUCCEEDED(sqstd_register_stringlib(g_sqvm)) {
                log.error("Failed registering Squirrel's standard string library");
                goto lib_init_failed;
            }

            sq_poptop(g_sqvm); // pop root table

            g_log = &log; // will be used to output compiler and runtime script errors

            return true;

        lib_init_failed:
            sq_close(g_sqvm);
            g_sqvm = 0;
            return false;
        }

        //-----------------------------------------------------------------
        void DeinitScript(const Log& log)
        {
            assert(g_sqvm);
            if (g_sqvm) {
                log.info() << "Closing Squirrel VM");
                sq_close(g_sqvm);
                g_sqvm = 0;
            }
            g_log = 0;
        }

    } // namespace internal

} // namespace script
