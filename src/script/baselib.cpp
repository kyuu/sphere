#include <cassert>
#include "../io/numio.hpp"
#include "macros.hpp"
#include "util.hpp"
#include "iolib.hpp"
#include "vm.hpp"
#include "baselib.hpp"


namespace sphere {
    namespace script {

        namespace internal {

            static SQInteger _blob_destructor(SQUserPointer p, SQInteger size);

        } // namespace internal

        //-----------------------------------------------------------------
        bool BindRect(HSQUIRRELVM v, const Recti& rect)
        {
            // get rect class
            sq_pushregistrytable(v);
            sq_pushstring(v, "Rect", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                sq_poptop(v); // pop registry table
                return false;
            }
            sq_remove(v, -2); // remove registry table
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_RECT) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop rect class
            sq_remove(v, -2);

            // set up instance
            SQUserPointer p = 0;
            sq_getinstanceup(v, -1, &p, 0);
            assert(p);
            new (p) Recti(rect);

            return true;
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
        bool BindVec2(HSQUIRRELVM v, const Vec2f& vec)
        {
            // get vec2 class
            sq_pushregistrytable(v);
            sq_pushstring(v, "Vec2", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                sq_poptop(v); // pop registry table
                return false;
            }
            sq_remove(v, -2); // remove registry table
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_VEC2) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop vec2 class
            sq_remove(v, -2);

            // set up instance
            SQUserPointer p = 0;
            sq_getinstanceup(v, -1, &p, 0);
            assert(p);
            new (p) Vec2f(vec);

            return true;
        }

        //-----------------------------------------------------------------
        Vec2f* GetVec2(HSQUIRRELVM v, SQInteger idx)
        {
            SQUserPointer p = 0;
            if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_VEC2))) {
                return (Vec2f*)p;
            }
            return 0;
        }

        //-----------------------------------------------------------------
        bool BindBlob(HSQUIRRELVM v, Blob* blob)
        {
            assert(blob);

            // get blob class
            sq_pushregistrytable(v);
            sq_pushstring(v, "Blob", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                sq_poptop(v); // pop registry table
                return false;
            }
            sq_remove(v, -2); // remove registry table
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_BLOB) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop blob class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_blob_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)blob);

            // grab a new reference
            blob->grab();

            return true;
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

        namespace internal {

            #define SETUP_RECT_OBJECT() \
                Recti* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_RECT))) { \
                    THROW_ERROR("Invalid type of environment object, expected a Rect instance") \
                }

            //-----------------------------------------------------------------
            // Rect(x, y, width, height)
            static SQInteger _rect_constructor(HSQUIRRELVM v)
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
            static SQInteger _rect_isValid(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                RET_BOOL(This->isValid())
            }

            //-----------------------------------------------------------------
            // Rect.intersects(other)
            static SQInteger _rect_intersects(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_RECT(1, other)
                RET_BOOL(This->intersects(*other))
            }

            //-----------------------------------------------------------------
            // Rect.getIntersection(other)
            static SQInteger _rect_getIntersection(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_RECT(1, other)
                RET_RECT(This->getIntersection(*other))
            }

            //-----------------------------------------------------------------
            // Rect.containsPoint(x, y)
            static SQInteger _rect_containsPoint(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                CHECK_NARGS(2)
                GET_ARG_INT(1, x)
                GET_ARG_INT(2, y)
                RET_BOOL(This->contains(x, y))
            }

            //-----------------------------------------------------------------
            // Rect.containsRect(rect)
            static SQInteger _rect_containsRect(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_RECT(1, rect)
                RET_BOOL(This->contains(*rect))
            }

            //-----------------------------------------------------------------
            // Rect._get(index)
            static SQInteger _rect__get(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_STRING(1, index)
                if (strcmp(index, "x") == 0) {
                    RET_INT(This->getX())
                } else if (strcmp(index, "y") == 0) {
                    RET_INT(This->getY())
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
            static SQInteger _rect__set(HSQUIRRELVM v)
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
            static SQInteger _rect__typeof(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                RET_STRING("Rect")
            }

            //-----------------------------------------------------------------
            // Rect._cloned(rect)
            static SQInteger _rect__cloned(HSQUIRRELVM v)
            {
                SETUP_RECT_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_RECT(1, original)
                new (This) Recti(*original);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Rect._tostring()
            static SQInteger _rect__tostring(HSQUIRRELVM v)
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
            static SQInteger _rect__dump(HSQUIRRELVM v)
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
            static SQInteger _rect__load(HSQUIRRELVM v)
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
            static util::Function _rect_methods[] = {
                {"constructor",     "Rect.constructor",         _rect_constructor     },
                {"isValid",         "Rect.isValid",             _rect_isValid         },
                {"intersects",      "Rect.intersects",          _rect_intersects      },
                {"getIntersection", "Rect.getIntersection",     _rect_getIntersection },
                {"containsPoint",   "Rect.containsPoint",       _rect_containsPoint   },
                {"containsRect",    "Rect.containsRect",        _rect_containsRect    },
                {"_get",            "Rect._get",                _rect__get            },
                {"_set",            "Rect._set",                _rect__set            },
                {"_typeof",         "Rect._typeof",             _rect__typeof         },
                {"_cloned",         "Rect._cloned",             _rect__cloned         },
                {"_tostring",       "Rect._tostring",           _rect__tostring       },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _rect_static_methods[] = {
                {"_dump",           "Rect._dump",               _rect__dump           },
                {"_load",           "Rect._load",               _rect__load           },
                {0,0}
            };

            #define SETUP_VEC2_OBJECT() \
                Vec2f* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_VEC2))) { \
                    THROW_ERROR("Invalid type of environment object, expected a Vec2 instance") \
                }

            //-----------------------------------------------------------------
            // Vec2([x, y])
            static SQInteger _vec2_constructor(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                GET_OPTARG_FLOAT(1, x, 0)
                GET_OPTARG_FLOAT(2, y, 0)
                new (This) Vec2f(x, y);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Vec2.getLength()
            static SQInteger _vec2_getLength(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                RET_FLOAT((SQFloat)This->len())
            }

            //-----------------------------------------------------------------
            // Vec2.getDotProduct(other)
            static SQInteger _vec2_getDotProduct(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, other)
                RET_FLOAT(This->dot(*other))
            }

            //-----------------------------------------------------------------
            // Vec2.setNull()
            static SQInteger _vec2_setNull(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                This->null();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Vec2.normalize()
            static SQInteger _vec2_normalize(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                This->unit();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Vec2.getDistanceFrom(other)
            static SQInteger _vec2_getDistanceFrom(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, other)
                RET_FLOAT((SQFloat)This->distFrom(*other))
            }

            //-----------------------------------------------------------------
            // Vec2.rotateBy(degrees [, center])
            static SQInteger _vec2_rotateBy(HSQUIRRELVM v)
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
            static SQInteger _vec2_getAngle(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                RET_FLOAT((SQFloat)This->getAngle())
            }

            //-----------------------------------------------------------------
            // Vec2.getAngleWith(other)
            static SQInteger _vec2_getAngleWith(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, other)
                RET_FLOAT((SQFloat)This->getAngleWith(*other))
            }

            //-----------------------------------------------------------------
            // Vec2.isBetween(a, b)
            static SQInteger _vec2_isBetween(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(2)
                GET_ARG_VEC2(1, a)
                GET_ARG_VEC2(2, b)
                RET_BOOL(This->isBetween(*a, *b))
            }

            //-----------------------------------------------------------------
            // Vec2.getInterpolated(other, d)
            static SQInteger _vec2_getInterpolated(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(2)
                GET_ARG_VEC2(1, other)
                GET_ARG_FLOAT(2, d)
                RET_VEC2(This->getInterpolated(*other, d))
            }

            //-----------------------------------------------------------------
            // Vec2.interpolate(a, b, d)
            static SQInteger _vec2_interpolate(HSQUIRRELVM v)
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
            static SQInteger _vec2_add(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, rhs)
                *This += *rhs;
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Vec2.subtract(rhs)
            static SQInteger _vec2_subtract(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, rhs)
                *This -= *rhs;
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Vec2.multiply(scalar)
            static SQInteger _vec2_multiply(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, scalar)
                *This *= scalar;
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Vec2._add(rhs)
            static SQInteger _vec2__add(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, rhs)
                RET_VEC2(*This + *rhs)
            }

            //-----------------------------------------------------------------
            // Vec2._sub(rhs)
            static SQInteger _vec2__sub(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, rhs)
                RET_VEC2(*This - *rhs)
            }

            //-----------------------------------------------------------------
            // Vec2._mul(scalar)
            static SQInteger _vec2__mul(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, scalar)
                RET_VEC2(*This * scalar)
            }

            //-----------------------------------------------------------------
            // Vec2._get(index)
            static SQInteger _vec2__get(HSQUIRRELVM v)
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
            static SQInteger _vec2__set(HSQUIRRELVM v)
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
            static SQInteger _vec2__typeof(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                RET_STRING("Vec2")
            }

            //-----------------------------------------------------------------
            // Vec2._cloned(original)
            static SQInteger _vec2__cloned(HSQUIRRELVM v)
            {
                SETUP_VEC2_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_VEC2(1, original)
                new (This) Vec2f(*original);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Vec2._tostring()
            static SQInteger _vec2__tostring(HSQUIRRELVM v)
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
            static SQInteger _vec2__dump(HSQUIRRELVM v)
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
            static SQInteger _vec2__load(HSQUIRRELVM v)
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
            static util::Function _vec2_methods[] = {
                {"constructor",     "Vec2.constructor",     _vec2_constructor     },
                {"getLength",       "Vec2.getLength",       _vec2_getLength       },
                {"getDotProduct",   "Vec2.getDotProduct",   _vec2_getDotProduct   },
                {"setNull",         "Vec2.setNull",         _vec2_setNull         },
                {"normalize",       "Vec2.normalize",       _vec2_normalize       },
                {"getDistanceFrom", "Vec2.getDistanceFrom", _vec2_getDistanceFrom },
                {"rotateBy",        "Vec2.rotateBy",        _vec2_rotateBy        },
                {"getAngle",        "Vec2.getAngle",        _vec2_getAngle        },
                {"getAngleWith",    "Vec2.getAngleWith",    _vec2_getAngleWith    },
                {"isBetween",       "Vec2.isBetween",       _vec2_isBetween       },
                {"getInterpolated", "Vec2.getInterpolated", _vec2_getInterpolated },
                {"interpolate",     "Vec2.interpolate",     _vec2_interpolate     },
                {"add",             "Vec2.add",             _vec2_add             },
                {"subtract",        "Vec2.subtract",        _vec2_subtract        },
                {"multiply",        "Vec2.multiply",        _vec2_multiply        },
                {"_add",            "Vec2._add",            _vec2__add            },
                {"_sub",            "Vec2._sub",            _vec2__sub            },
                {"_mul",            "Vec2._mul",            _vec2__mul            },
                {"_get",            "Vec2._get",            _vec2__get            },
                {"_set",            "Vec2._set",            _vec2__set            },
                {"_typeof",         "Vec2._typeof",         _vec2__typeof         },
                {"_cloned",         "Vec2._cloned",         _vec2__cloned         },
                {"_tostring",       "Vec2._tostring",       _vec2__tostring       },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _vec2_static_methods[] = {
                {"_dump",           "Vec2._dump",           _vec2__dump           },
                {"_load",           "Vec2._load",           _vec2__load           },
                {0,0}
            };

            #define SETUP_BLOB_OBJECT() \
                Blob* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_BLOB))) { \
                    THROW_ERROR("Invalid type of environment object, expected a Blob instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _blob_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((Blob*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // Blob([size = 0, value = 0])
            static SQInteger _blob_constructor(HSQUIRRELVM v)
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
                sq_setreleasehook(v, 1, _blob_destructor);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.FromString(string)
            static SQInteger _blob_FromString(HSQUIRRELVM v)
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
            static SQInteger _blob_getSize(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                RET_INT(This->getSize())
            }

            //-----------------------------------------------------------------
            // Blob.getCapacity()
            static SQInteger _blob_getCapacity(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                RET_INT(This->getCapacity())
            }

            //-----------------------------------------------------------------
            // Blob.clear()
            static SQInteger _blob_clear(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                This->clear();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.reset([value])
            static SQInteger _blob_reset(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                GET_OPTARG_INT(1, value, 0)
                This->reset((u8)value);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.resize(size)
            static SQInteger _blob_resize(HSQUIRRELVM v)
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
            static SQInteger _blob_bloat(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                This->bloat();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.reserve(size)
            static SQInteger _blob_reserve(HSQUIRRELVM v)
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
            static SQInteger _blob_doubleCapacity(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                This->doubleCapacity();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.assign(blob)
            static SQInteger _blob_assign(HSQUIRRELVM v)
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
            static SQInteger _blob_append(HSQUIRRELVM v)
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
            static SQInteger _blob_concat(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_BLOB(1, blob)
                BlobPtr result = This->concat(blob->getBuffer(), blob->getSize());
                RET_BLOB(result.get())
            }

            //-----------------------------------------------------------------
            // Blob.swap2()
            static SQInteger _blob_swap2(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                This->swap2();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.swap4()
            static SQInteger _blob_swap4(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                This->swap4();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.swap8()
            static SQInteger _blob_swap8(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                This->swap8();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob.createString()
            static SQInteger _blob_createString(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                RET_STRING_N((const SQChar*)This->getBuffer(), This->getSize())
            }

            //-----------------------------------------------------------------
            // Blob._add(blob)
            static SQInteger _blob__add(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_BLOB(1, blob)
                BlobPtr new_blob = This->concat(blob->getBuffer(), blob->getSize());
                RET_BLOB(new_blob.get())
            }

            //-----------------------------------------------------------------
            // Blob._get(index)
            static SQInteger _blob__get(HSQUIRRELVM v)
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
            static SQInteger _blob__set(HSQUIRRELVM v)
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
            static SQInteger _blob__typeof(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                RET_STRING("Blob")
            }

            //-----------------------------------------------------------------
            // Blob._tostring()
            static SQInteger _blob__tostring(HSQUIRRELVM v)
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
            static SQInteger _blob__nexti(HSQUIRRELVM v)
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
            static SQInteger _blob__cloned(HSQUIRRELVM v)
            {
                SETUP_BLOB_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_BLOB(1, original)
                This = Blob::Create(original->getSize());
                if (original->getSize() > 0) {
                    memcpy(This->getBuffer(), original->getBuffer(), original->getSize());
                }
                sq_setinstanceup(v, 1, (SQUserPointer)This);
                sq_setreleasehook(v, 1, _blob_destructor);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Blob._dump(instance, stream)
            static SQInteger _blob__dump(HSQUIRRELVM v)
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
            static SQInteger _blob__load(HSQUIRRELVM v)
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
            static util::Function _blob_methods[] = {
                {"constructor",     "Blob.constructor",     _blob_constructor     },
                {"getSize",         "Blob.getSize",         _blob_getSize         },
                {"getCapacity",     "Blob.getCapacity",     _blob_getCapacity     },
                {"clear",           "Blob.clear",           _blob_clear           },
                {"reset",           "Blob.reset",           _blob_reset           },
                {"resize",          "Blob.resize",          _blob_resize          },
                {"bloat",           "Blob.bloat",           _blob_bloat           },
                {"reserve",         "Blob.reserve",         _blob_reserve         },
                {"doubleCapacity",  "Blob.doubleCapacity",  _blob_doubleCapacity  },
                {"assign",          "Blob.assign",          _blob_assign          },
                {"append",          "Blob.append",          _blob_append          },
                {"concat",          "Blob.concat",          _blob_concat          },
                {"swap2",           "Blob.swap2",           _blob_swap2           },
                {"swap4",           "Blob.swap4",           _blob_swap4           },
                {"swap8",           "Blob.swap8",           _blob_swap8           },
                {"createString",    "Blob.createString",    _blob_createString    },
                {"_add",            "Blob._add",            _blob__add            },
                {"_get",            "Blob._get",            _blob__get            },
                {"_set",            "Blob._set",            _blob__set            },
                {"_typeof",         "Blob._typeof",         _blob__typeof         },
                {"_tostring",       "Blob._tostring",       _blob__tostring       },
                {"_nexti",          "Blob._nexti",          _blob__nexti          },
                {"_cloned",         "Blob._cloned",         _blob__cloned         },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _blob_static_methods[] = {
                {"FromString",      "Blob.FromString",      _blob_FromString      },
                {"_dump",           "Blob._dump",           _blob__dump           },
                {"_load",           "Blob._load",           _blob__load           },
                {0,0}
            };

            //-----------------------------------------------------------------
            bool RegisterBaseLibrary(HSQUIRRELVM v)
            {
                /* Rect */

                // create rect class
                sq_newclass(v, SQFalse);

                // set up rect class
                sq_settypetag(v, -1, TT_RECT);
                sq_setclassudsize(v, -1, sizeof(Recti));
                util::RegisterFunctions(v, _rect_methods);
                util::RegisterFunctions(v, _rect_static_methods, true);

                // register rect class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "Rect", -1);
                sq_push(v, -3); // push rect class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register rect class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "Rect", -1);
                sq_push(v, -3); // push rect class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop rect class
                sq_poptop(v);

                /* Vec2 */

                // create vec2 class
                sq_newclass(v, SQFalse);

                // set up vec2 class
                sq_settypetag(v, -1, TT_VEC2);
                sq_setclassudsize(v, -1, sizeof(Vec2f));
                util::RegisterFunctions(v, _vec2_methods);
                util::RegisterFunctions(v, _vec2_static_methods, true);

                // register vec2 class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "Vec2", -1);
                sq_push(v, -3); // push vec2 class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register vec2 class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "Vec2", -1);
                sq_push(v, -3); // push vec2 class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop vec2 class
                sq_poptop(v);

                /* Blob */

                // get stream class
                sq_pushregistrytable(v);
                sq_pushstring(v, "Stream", -1);
                if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                    return false;
                }
                SQUserPointer tt = 0;
                if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_STREAM) {
                    sq_poptop(v);
                    return false;
                }

                // create blob class, inheriting from stream class
                sq_newclass(v, SQTrue);

                // set up blob class
                sq_settypetag(v, -1, TT_BLOB);
                util::RegisterFunctions(v, _blob_methods);
                util::RegisterFunctions(v, _blob_static_methods, true);

                // register blob class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "Blob", -1);
                sq_push(v, -3); // push blob class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register blob class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "Blob", -1);
                sq_push(v, -3); // push blob class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop blob class
                sq_poptop(v);

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
