#include <cassert>
#include "../io/numio.hpp"
#include "../io/imageio.hpp"
#include "macros.hpp"
#include "util.hpp"
#include "vm.hpp"
#include "baselib.hpp"
#include "iolib.hpp"
#include "graphicslib.hpp"


namespace sphere {
    namespace script {

        namespace internal {

            static SQInteger _canvas_destructor(SQUserPointer p, SQInteger size);
            static SQInteger _texture_destructor(SQUserPointer p, SQInteger size);

        } // namespace internal

        //-----------------------------------------------------------------
        bool BindCanvas(HSQUIRRELVM v, Canvas* canvas)
        {
            assert(canvas);

            // get canvas class
            sq_pushregistrytable(v);
            sq_pushstring(v, "Canvas", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                return false;
            }
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_CANVAS) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop canvas class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_canvas_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)canvas);

            // grab a new reference
            canvas->grab();

            return true;
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
        bool BindTexture(HSQUIRRELVM v, ITexture* texture)
        {
            assert(texture);

            // get texture class
            sq_pushregistrytable(v);
            sq_pushstring(v, "Texture", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                return false;
            }
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_TEXTURE) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop texture class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_texture_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)texture);

            // grab a new reference
            texture->grab();

            return true;
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

        namespace internal {

            #define SETUP_CANVAS_OBJECT() \
                Canvas* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_CANVAS))) { \
                    THROW_ERROR("Invalid type of environment object, expected a Canvas instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _canvas_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((Canvas*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // Canvas(width, height [, color])
            static SQInteger _canvas_constructor(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                CHECK_MIN_NARGS(2)
                GET_ARG_INT(1, width)
                GET_ARG_INT(2, height)
                GET_OPTARG_INT(3, color, RGBA::Pack(0, 0, 0))
                if (width <= 0) {
                    THROW_ERROR1("Invalid width: %d", width)
                }
                if (height <= 0) {
                    THROW_ERROR1("Invalid height: %d", height)
                }
                This = Canvas::Create(width, height);
                This->fill(RGBA::Unpack(color));
                sq_setinstanceup(v, 1, (SQUserPointer)This);
                sq_setreleasehook(v, 1, _canvas_destructor);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Canvas.FromBuffer(width, height, pixels)
            static SQInteger _canvas_FromBuffer(HSQUIRRELVM v)
            {
                CHECK_NARGS(3)
                GET_ARG_INT(1, width)
                GET_ARG_INT(2, height)
                GET_ARG_BLOB(3, pixels)
                if (width <= 0) {
                    THROW_ERROR1("Invalid width: %d", width)
                }
                if (height <= 0) {
                    THROW_ERROR1("Invalid height: %d", height)
                }
                int expected_size = width * height * Canvas::GetNumBytesPerPixel();
                if (pixels->getSize() != expected_size) {
                    THROW_ERROR2("Invalid buffer size: %d, expected: %d", pixels->getSize(), expected_size)
                }
                CanvasPtr image = Canvas::Create(width, height);
                memcpy(image->getPixels(), pixels->getBuffer(), pixels->getSize());
                RET_CANVAS(image.get())
            }

            //-----------------------------------------------------------------
            // Canvas.FromFile(filename)
            static SQInteger _canvas_FromFile(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                FilePtr file = io::filesystem::OpenFile(filename);
                if (!file) {
                    THROW_ERROR("Could not open file")
                }
                CanvasPtr image = io::LoadImage(file.get());
                if (!image) {
                    THROW_ERROR("Could not load image")
                }
                RET_CANVAS(image.get())
            }

            //-----------------------------------------------------------------
            // Canvas.FromStream(stream)
            static SQInteger _canvas_FromStream(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STREAM(1, stream)
                if (!stream->isOpen() || !stream->isReadable()) {
                    THROW_ERROR("Invalid stream")
                }
                CanvasPtr image = io::LoadImage(stream);
                if (!image) {
                    THROW_ERROR("Could not load image")
                }
                RET_CANVAS(image.get())
            }

            //-----------------------------------------------------------------
            // Canvas.saveToFile(filename)
            static SQInteger _canvas_saveToFile(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                FilePtr file = io::filesystem::OpenFile(filename, IFile::FM_OUT);
                if (!file) {
                    THROW_ERROR("Could not open file")
                }
                if (!io::SaveImage(This, file.get())) {
                    THROW_ERROR("Could not save image")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Canvas.saveToStream(stream)
            static SQInteger _canvas_saveToStream(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_STREAM(1, stream)
                if (!stream->isOpen() || !stream->isReadable()) {
                    THROW_ERROR("Invalid stream")
                }
                if (!io::SaveImage(This, stream)) {
                    THROW_ERROR("Could not save image")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Canvas.getPixels()
            static SQInteger _canvas_getPixels(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                BlobPtr pixels = Blob::Create(This->getNumPixels() * Canvas::GetNumBytesPerPixel());
                memcpy(pixels->getBuffer(), This->getPixels(), pixels->getSize());
                RET_BLOB(pixels.get())
            }

            //-----------------------------------------------------------------
            // Canvas.cloneSection(section)
            static SQInteger _canvas_cloneSection(HSQUIRRELVM v)
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
            static SQInteger _canvas_getPixel(HSQUIRRELVM v)
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
            static SQInteger _canvas_setPixel(HSQUIRRELVM v)
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
            static SQInteger _canvas_getPixelByIndex(HSQUIRRELVM v)
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
            static SQInteger _canvas_setPixelByIndex(HSQUIRRELVM v)
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
            static SQInteger _canvas_resize(HSQUIRRELVM v)
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
            static SQInteger _canvas_fill(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_INT(1, color)
                This->fill(RGBA::Unpack((u32)color));
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Canvas.flipHorizontally()
            static SQInteger _canvas_flipHorizontally(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                This->flipHorizontally();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Canvas.flipVertically()
            static SQInteger _canvas_flipVertically(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                This->flipVertically();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Canvas._get(index)
            static SQInteger _canvas__get(HSQUIRRELVM v)
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
            static SQInteger _canvas__set(HSQUIRRELVM v)
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
            static SQInteger _canvas__typeof(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                RET_STRING("Canvas")
            }

            //-----------------------------------------------------------------
            // Canvas._tostring()
            static SQInteger _canvas__tostring(HSQUIRRELVM v)
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
            static SQInteger _canvas__nexti(HSQUIRRELVM v)
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
            static SQInteger _canvas__cloned(HSQUIRRELVM v)
            {
                SETUP_CANVAS_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_CANVAS(1, original)
                This = Canvas::Create(original->getWidth(), original->getHeight(), original->getPixels());
                sq_setinstanceup(v, 1, (SQUserPointer)This);
                sq_setreleasehook(v, 1, _canvas_destructor);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Canvas._dump(instance, stream)
            static SQInteger _canvas__dump(HSQUIRRELVM v)
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
            static SQInteger _canvas__load(HSQUIRRELVM v)
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
            static util::Function _canvas_methods[] = {
                {"constructor",         "Canvas.constructor",       _canvas_constructor       },
                {"saveToFile",          "Canvas.saveToFile",        _canvas_saveToFile        },
                {"saveToStream",        "Canvas.saveToStream",      _canvas_saveToStream      },
                {"getPixels",           "Canvas.getPixels",         _canvas_getPixels         },
                {"cloneSection",        "Canvas.cloneSection",      _canvas_cloneSection      },
                {"getPixel",            "Canvas.getPixel",          _canvas_getPixel          },
                {"setPixel",            "Canvas.setPixel",          _canvas_setPixel          },
                {"getPixelByIndex",     "Canvas.getPixelByIndex",   _canvas_getPixelByIndex   },
                {"setPixelByIndex",     "Canvas.setPixelByIndex",   _canvas_setPixelByIndex   },
                {"resize",              "Canvas.resize",            _canvas_resize            },
                {"fill",                "Canvas.fill",              _canvas_fill              },
                {"flipHorizontally",    "Canvas.flipHorizontally",  _canvas_flipHorizontally  },
                {"flipVertically",      "Canvas.flipVertically",    _canvas_flipVertically    },
                {"_get",                "Canvas._get",              _canvas__get              },
                {"_set",                "Canvas._set",              _canvas__set              },
                {"_typeof",             "Canvas._typeof",           _canvas__typeof           },
                {"_tostring",           "Canvas._tostring",         _canvas__tostring         },
                {"_nexti",              "Canvas._nexti",            _canvas__nexti            },
                {"_cloned",             "Canvas._cloned",           _canvas__cloned           },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _canvas_static_methods[] = {
                {"FromBuffer",      "Canvas.FromBuffer",    _canvas_FromBuffer    },
                {"FromFile",        "Canvas.FromFile",      _canvas_FromFile      },
                {"FromStream",      "Canvas.FromStream",    _canvas_FromStream    },
                {"_dump",           "Canvas._dump",         _canvas__dump         },
                {"_load",           "Canvas._load",         _canvas__load         },
                {0,0}
            };

            #define SETUP_TEXTURE_OBJECT() \
                ITexture* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_TEXTURE))) { \
                    THROW_ERROR("Invalid type of environment object, expected a Texture instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _texture_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((ITexture*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // Texture(width, height)
            static SQInteger _texture_constructor(HSQUIRRELVM v)
            {
                SETUP_TEXTURE_OBJECT()
                CHECK_NARGS(2)
                GET_ARG_INT(1, width)
                GET_ARG_INT(2, height)
                if (width <= 0) {
                    THROW_ERROR1("Invalid width: %d", width)
                }
                if (height <= 0) {
                    THROW_ERROR1("Invalid height: %d", height)
                }
                This = video::CreateTexture(width, height);
                if (!This) {
                    THROW_ERROR("Could not create texture")
                }
                sq_setinstanceup(v, 1, (SQUserPointer)This);
                sq_setreleasehook(v, 1, _texture_destructor);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Texture.FromCanvas(canvas [, section])
            static SQInteger _texture_FromCanvas(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(1)
                GET_ARG_CANVAS(1, canvas)
                GET_OPTARG_RECT(2, section)
                TexturePtr texture;
                if (section) {
                    CanvasPtr sub_canvas = canvas->cloneSection(*section);
                    if (!sub_canvas) {
                        // the only possible case
                        THROW_ERROR("Invalid section")
                    }
                    texture = video::CreateTexture(sub_canvas->getWidth(), sub_canvas->getHeight(), sub_canvas->getPixels());
                } else {
                    texture = video::CreateTexture(canvas->getWidth(), canvas->getHeight(), canvas->getPixels());
                }
                if (!texture) {
                    THROW_ERROR("Could not create texture")
                }
                RET_TEXTURE(texture.get())
            }

            //-----------------------------------------------------------------
            // Texture.FromFile(filename)
            static SQInteger _texture_FromFile(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                FilePtr file = io::filesystem::OpenFile(filename);
                if (!file) {
                    THROW_ERROR("Could not open file")
                }
                CanvasPtr image = io::LoadImage(file.get());
                if (!image) {
                    THROW_ERROR("Could not load image")
                }
                TexturePtr texture = video::CreateTexture(image->getWidth(), image->getHeight(), image->getPixels());
                if (!texture) {
                    THROW_ERROR("Could not create texture")
                }
                RET_TEXTURE(texture.get())
            }

            //-----------------------------------------------------------------
            // Texture.FromStream(stream)
            static SQInteger _texture_FromStream(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STREAM(1, stream)
                CanvasPtr image = io::LoadImage(stream);
                if (!image) {
                    THROW_ERROR("Could not load image")
                }
                TexturePtr texture = video::CreateTexture(image->getWidth(), image->getHeight(), image->getPixels());
                if (!texture) {
                    THROW_ERROR("Could not create texture")
                }
                RET_TEXTURE(texture.get())
            }

            //-----------------------------------------------------------------
            // Texture.updatePixels(newPixels [, section])
            static SQInteger _texture_updatePixels(HSQUIRRELVM v)
            {
                SETUP_TEXTURE_OBJECT()
                CHECK_MIN_NARGS(1)
                GET_ARG_CANVAS(1, newPixels)
                GET_OPTARG_RECT(2, section)
                if (!video::UpdateTexturePixels(This, newPixels, section)) {
                    THROW_ERROR("Could not update pixels")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Texture.createCanvas()
            static SQInteger _texture_createCanvas(HSQUIRRELVM v)
            {
                SETUP_TEXTURE_OBJECT()
                CanvasPtr canvas = video::GrabTexturePixels(This);
                RET_CANVAS(canvas.get())
            }

            //-----------------------------------------------------------------
            // Texture._get(index)
            static SQInteger _texture__get(HSQUIRRELVM v)
            {
                SETUP_TEXTURE_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_STRING(1, index)
                if (strcmp(index, "width") == 0) {
                    RET_INT(This->getSize().width)
                } else if (strcmp(index, "height") == 0) {
                    RET_INT(This->getSize().height)
                } else {
                    // index not found
                    sq_pushnull(v);
                    return sq_throwobject(v);
                }
            }

            //-----------------------------------------------------------------
            // Texture._typeof()
            static SQInteger _texture__typeof(HSQUIRRELVM v)
            {
                SETUP_TEXTURE_OBJECT()
                RET_STRING("Texture")
            }

            //-----------------------------------------------------------------
            // Texture._cloned(original)
            static SQInteger _texture__cloned(HSQUIRRELVM v)
            {
                SETUP_TEXTURE_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_TEXTURE(1, original)
                CanvasPtr canvas = video::GrabTexturePixels(original);
                This = video::CreateTexture(canvas->getWidth(), canvas->getHeight(), canvas->getPixels());
                if (!This) {
                    THROW_ERROR("Could not create texture")
                }
                sq_setinstanceup(v, 1, (SQUserPointer)This);
                sq_setreleasehook(v, 1, _texture_destructor);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Texture._tostring()
            static SQInteger _texture__tostring(HSQUIRRELVM v)
            {
                SETUP_TEXTURE_OBJECT()
                std::ostringstream oss;
                oss << "<Texture instance at " << This;
                oss << " (" << This->getSize().width;
                oss << ", " << This->getSize().height;
                oss << ")>";
                RET_STRING(oss.str().c_str())
            }

            //-----------------------------------------------------------------
            // Texture._dump(instance, stream)
            static SQInteger _texture__dump(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_TEXTURE(1, instance)
                GET_ARG_STREAM(2, stream)

                if (!stream->isOpen() || !stream->isWriteable()) {
                    THROW_ERROR("Invalid output stream")
                }

                // convert texture to canvas
                CanvasPtr canvas = video::GrabTexturePixels(instance);

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
            static SQInteger _texture__load(HSQUIRRELVM v)
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
                TexturePtr instance = video::CreateTexture(canvas->getWidth(), canvas->getHeight(), canvas->getPixels());

                RET_TEXTURE(instance.get())
            }

            //-----------------------------------------------------------------
            static util::Function _texture_methods[] = {
                {"constructor",     "Texture.constructor",  _texture_constructor  },
                {"updatePixels",    "Texture.updatePixels", _texture_updatePixels },
                {"createCanvas",    "Texture.createCanvas", _texture_createCanvas },
                {"_get",            "Texture._get",         _texture__get         },
                {"_typeof",         "Texture._typeof",      _texture__typeof      },
                {"_cloned",         "Texture._cloned",      _texture__cloned      },
                {"_tostring",       "Texture._tostring",    _texture__tostring    },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _texture_static_methods[] = {
                {"FromCanvas",  "Texture.FromCanvas",  _texture_FromCanvas   },
                {"FromFile",    "Texture.FromFile",    _texture_FromFile     },
                {"FromStream",  "Texture.FromStream",  _texture_FromStream   },
                {"_dump",       "Texture._dump",       _texture__dump        },
                {"_load",       "Texture._load",       _texture__load        },
                {0,0}
            };

            //-----------------------------------------------------------------
            // CreateColor(red, green, blue [, alpha = 255])
            static SQInteger _graphics_CreateColor(HSQUIRRELVM v)
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
            static SQInteger _graphics_UnpackRed(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, color)
                RET_INT(RGBA::Unpack((u32)color).red)
            }

            //-----------------------------------------------------------------
            // UnpackGreen(color)
            static SQInteger _graphics_UnpackGreen(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, color)
                RET_INT(RGBA::Unpack((u32)color).green)
            }

            //-----------------------------------------------------------------
            // UnpackBlue(color)
            static SQInteger _graphics_UnpackBlue(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, color)
                RET_INT(RGBA::Unpack((u32)color).blue)
            }

            //-----------------------------------------------------------------
            // UnpackAlpha(color)
            static SQInteger _graphics_UnpackAlpha(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, color)
                RET_INT(RGBA::Unpack((u32)color).alpha)
            }

            //-----------------------------------------------------------------
            // GetDefaultDisplayMode()
            static SQInteger _graphics_GetDefaultDisplayMode(HSQUIRRELVM v)
            {
                sq_newtable(v);

                sq_pushstring(v, "width", -1);
                sq_pushinteger(v, video::GetDefaultDisplayMode().width);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "height", -1);
                sq_pushinteger(v, video::GetDefaultDisplayMode().height);
                sq_newslot(v, -3, SQFalse);

                return 1;
            }

            //-----------------------------------------------------------------
            // GetDisplayModes()
            static SQInteger _graphics_GetDisplayModes(HSQUIRRELVM v)
            {
                const std::vector<Dim2i>& modes = video::GetDisplayModes();

                sq_newarray(v, modes.size());

                for (size_t i = 0; i < modes.size(); ++i) {
                    sq_pushinteger(v, i); // key
                    sq_newtable(v); // value

                    sq_pushstring(v, "width", -1);
                    sq_pushinteger(v, modes[i].width);
                    sq_newslot(v, -3, SQFalse);

                    sq_pushstring(v, "height", -1);
                    sq_pushinteger(v, modes[i].height);
                    sq_newslot(v, -3, SQFalse);

                    sq_rawset(v, -3);
                }

                return 1;
            }

            //-----------------------------------------------------------------
            // SetWindowMode(width, height, fullScreen)
            static SQInteger _graphics_SetWindowMode(HSQUIRRELVM v)
            {
                CHECK_NARGS(3)
                GET_ARG_INT(1, width)
                GET_ARG_INT(2, height)
                GET_ARG_BOOL(3, fullScreen)
                if (width <= 0) {
                    THROW_ERROR1("Invalid width: %d", width)
                }
                if (height <= 0) {
                    THROW_ERROR1("Invalid height: %d", height)
                }
                if (!video::SetWindowMode(width, height, (fullScreen == SQTrue ? true : false))) {
                    THROW_ERROR("Could not set window mode")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // GetWindowWidth()
            static SQInteger _graphics_GetWindowWidth(HSQUIRRELVM v)
            {
                RET_INT(video::GetWindowSize().width)
            }

            //-----------------------------------------------------------------
            // GetWindowHeight()
            static SQInteger _graphics_GetWindowHeight(HSQUIRRELVM v)
            {
                RET_INT(video::GetWindowSize().height)
            }

            //-----------------------------------------------------------------
            // IsWindowFullScreen()
            static SQInteger _graphics_IsWindowFullScreen(HSQUIRRELVM v)
            {
                RET_BOOL(video::IsWindowFullScreen())
            }

            //-----------------------------------------------------------------
            // IsWindowActive()
            static SQInteger _graphics_IsWindowActive(HSQUIRRELVM v)
            {
                RET_BOOL(video::IsWindowActive())
            }

            //-----------------------------------------------------------------
            // GetWindowTitle()
            static SQInteger _graphics_GetWindowTitle(HSQUIRRELVM v)
            {
                RET_STRING(video::GetWindowTitle().c_str())
            }

            //-----------------------------------------------------------------
            // SetWindowTitle(title)
            static SQInteger _graphics_SetWindowTitle(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, title)
                video::SetWindowTitle(title);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // SetWindowIcon(icon)
            static SQInteger _graphics_SetWindowIcon(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_CANVAS(1, icon)
                video::SetWindowIcon(icon);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // SwapWindowBuffers()
            static SQInteger _graphics_SwapWindowBuffers(HSQUIRRELVM v)
            {
                video::SwapWindowBuffers();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // PeekWindowEvent([event])
            static SQInteger _graphics_PeekWindowEvent(HSQUIRRELVM v)
            {
                GET_OPTARG_INT(1, event, -1)
                RET_BOOL(video::PeekWindowEvent(event))
            }

            //-----------------------------------------------------------------
            // GetWindowEvent()
            static SQInteger _graphics_GetWindowEvent(HSQUIRRELVM v)
            {
                video::WindowEvent event;
                if (video::GetWindowEvent(event)) {
                    sq_newtable(v); // event

                    sq_pushstring(v, "type", -1);
                    sq_pushinteger(v, event.type);
                    sq_newslot(v, -3, SQFalse);

                    switch (event.type) {
                    case video::WindowEvent::KEY_PRESS:
                    case video::WindowEvent::KEY_RELEASE:
                        sq_pushstring(v, "key", -1);
                        sq_pushinteger(v, event.key.which);
                        sq_newslot(v, -3, SQFalse);
                        break;

                    case video::WindowEvent::MOUSE_BUTTON_PRESS:
                    case video::WindowEvent::MOUSE_BUTTON_RELEASE:
                        sq_pushstring(v, "button", -1);
                        sq_pushinteger(v, event.mouse.button.which);
                        sq_newslot(v, -3, SQFalse);
                        break;

                    case video::WindowEvent::MOUSE_MOTION:
                        sq_pushstring(v, "x", -1);
                        sq_pushinteger(v, event.mouse.motion.x);
                        sq_newslot(v, -3, SQFalse);

                        sq_pushstring(v, "y", -1);
                        sq_pushinteger(v, event.mouse.motion.y);
                        sq_newslot(v, -3, SQFalse);
                        break;

                    case video::WindowEvent::MOUSE_WHEEL_MOTION:
                        sq_pushstring(v, "delta", -1);
                        sq_pushinteger(v, event.mouse.wheel.delta);
                        sq_newslot(v, -3, SQFalse);
                        break;

                    case video::WindowEvent::WINDOW_CLOSE:
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
            // ClearWindowEvents()
            static SQInteger _graphics_ClearWindowEvents(HSQUIRRELVM v)
            {
                video::ClearWindowEvents();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // GetFrameScissor()
            static SQInteger _graphics_GetFrameScissor(HSQUIRRELVM v)
            {
                Recti scissor;
                video::GetFrameScissor(scissor);
                RET_RECT(scissor)
            }

            //-----------------------------------------------------------------
            // SetFrameScissor(scissor)
            static SQInteger _graphics_SetFrameScissor(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_RECT(1, scissor)
                if (!video::SetFrameScissor(*scissor)) {
                    THROW_ERROR("Could not set frame scissor")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // CloneFrame([section])
            static SQInteger _graphics_CloneFrame(HSQUIRRELVM v)
            {
                GET_OPTARG_RECT(1, section)
                CanvasPtr canvas = video::CloneFrame(section);
                if (!canvas) {
                    THROW_ERROR("Could not clone frame")
                }
                RET_CANVAS(canvas.get())
            }

            //-----------------------------------------------------------------
            // DrawPoint(x, y, color)
            static SQInteger _graphics_DrawPoint(HSQUIRRELVM v)
            {
                CHECK_NARGS(3)
                GET_ARG_INT(1, x)
                GET_ARG_INT(2, y)
                GET_ARG_INT(3, color)
                video::DrawPoint(Vec2i(x, y), RGBA::Unpack((u32)color));
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawLine(x1, y1, x2, y2, col1 [, col2])
            static SQInteger _graphics_DrawLine(HSQUIRRELVM v)
            {
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
                video::DrawLine(positions, colors);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawTriangle(x1, y1, x2, y2, x3, y3, col1 [, col2, col3])
            static SQInteger _graphics_DrawTriangle(HSQUIRRELVM v)
            {
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
                video::DrawTriangle(positions, colors);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawTexturedTriangle(tex, tx1, ty1, tx2, ty2, tx3, ty3, x1, y1, x2, y2, x3, y3 [, mask = CreateColor(255, 255, 255)])
            static SQInteger _graphics_DrawTexturedTriangle(HSQUIRRELVM v)
            {
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
                video::DrawTexturedTriangle(tex, texcoords, positions, RGBA::Unpack((u32)mask));
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawRect(rect, col1 [, col2, col3, col4])
            static SQInteger _graphics_DrawRect(HSQUIRRELVM v)
            {
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
                video::DrawRect(*rect, colors);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawImage(image, x, y [, mask = CreateColor(255, 255, 255)])
            static SQInteger _graphics_DrawImage(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(3)
                GET_ARG_TEXTURE(1, image)
                GET_ARG_INT(2, x)
                GET_ARG_INT(3, y)
                GET_OPTARG_INT(4, mask, RGBA::Pack(255, 255, 255))
                video::DrawImage(image, Vec2i(x, y), RGBA::Unpack((u32)mask));
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawSubImage(image, src_rect, x, y [, mask = CreateColor(255, 255, 255)])
            static SQInteger _graphics_DrawSubImage(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(4)
                GET_ARG_TEXTURE(1, image)
                GET_ARG_RECT(2, src_rect)
                GET_ARG_INT(3, x)
                GET_ARG_INT(4, y)
                GET_OPTARG_INT(5, mask, RGBA::Pack(255, 255, 255))
                video::DrawSubImage(image, *src_rect, Vec2i(x, y), RGBA::Unpack((u32)mask));
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawImageQuad(image, x1, y1, x2, y2, x3, y3, x4, y4 [, mask = CreateColor(255, 255, 255)])
            static SQInteger _graphics_DrawImageQuad(HSQUIRRELVM v)
            {
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
                video::DrawImageQuad(image, positions, RGBA::Unpack((u32)mask));
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DrawSubImageQuad(image, src_rect, x1, y1, x2, y2, x3, y3, x4, y4 [, mask = CreateColor(255, 255, 255)])
            static SQInteger _graphics_DrawSubImageQuad(HSQUIRRELVM v)
            {
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
                video::DrawSubImageQuad(image, *src_rect, positions, RGBA::Unpack((u32)mask));
                RET_VOID()
            }

            //-----------------------------------------------------------------
            static util::Function _graphics_functions[] = {
                {"CreateColor",                 "CreateColor",              _graphics_CreateColor              },
                {"UnpackRed",                   "UnpackRed",                _graphics_UnpackRed                },
                {"UnpackGreen",                 "UnpackGreen",              _graphics_UnpackGreen              },
                {"UnpackBlue",                  "UnpackBlue",               _graphics_UnpackBlue               },
                {"UnpackAlpha",                 "UnpackAlpha",              _graphics_UnpackAlpha              },
                {"GetDefaultDisplayMode",       "GetDefaultDisplayMode",    _graphics_GetDefaultDisplayMode    },
                {"GetDisplayModes",             "GetDisplayModes",          _graphics_GetDisplayModes          },
                {"SetWindowMode",               "SetWindowMode",            _graphics_SetWindowMode            },
                {"GetWindowWidth",              "GetWindowWidth",           _graphics_GetWindowWidth           },
                {"GetWindowHeight",             "GetWindowHeight",          _graphics_GetWindowHeight          },
                {"IsWindowFullScreen",          "IsWindowFullScreen",       _graphics_IsWindowFullScreen       },
                {"IsWindowActive",              "IsWindowActive",           _graphics_IsWindowActive           },
                {"GetWindowTitle",              "GetWindowTitle",           _graphics_GetWindowTitle           },
                {"SetWindowTitle",              "SetWindowTitle",           _graphics_SetWindowTitle           },
                {"SetWindowIcon",               "SetWindowIcon",            _graphics_SetWindowIcon            },
                {"SwapWindowBuffers",           "SwapWindowBuffers",        _graphics_SwapWindowBuffers        },
                {"PeekWindowEvent",             "PeekWindowEvent",          _graphics_PeekWindowEvent          },
                {"GetWindowEvent",              "GetWindowEvent",           _graphics_GetWindowEvent           },
                {"ClearWindowEvents",           "ClearWindowEvents",        _graphics_ClearWindowEvents        },
                {"GetFrameScissor",             "GetFrameScissor",          _graphics_GetFrameScissor          },
                {"SetFrameScissor",             "SetFrameScissor",          _graphics_SetFrameScissor          },
                {"CloneFrame",                  "CloneFrame",               _graphics_CloneFrame               },
                {"DrawPoint",                   "DrawPoint",                _graphics_DrawPoint                },
                {"DrawLine",                    "DrawLine",                 _graphics_DrawLine                 },
                {"DrawTriangle",                "DrawTriangle",             _graphics_DrawTriangle             },
                {"DrawTexturedTriangle",        "DrawTexturedTriangle",     _graphics_DrawTexturedTriangle     },
                {"DrawRect",                    "DrawRect",                 _graphics_DrawRect                 },
                {"DrawImage",                   "DrawImage",                _graphics_DrawImage                },
                {"DrawSubImage",                "DrawSubImage",             _graphics_DrawSubImage             },
                {"DrawImageQuad",               "DrawImageQuad",            _graphics_DrawImageQuad            },
                {"DrawSubImageQuad",            "DrawSubImageQuad",         _graphics_DrawSubImageQuad         },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Constant _graphics_constants[] = {

                // color constants
                {"BLACK",       RGBA::Pack(  0,   0,   0)  },
                {"WHITE",       RGBA::Pack(255, 255, 255)  },
                {"RED",         RGBA::Pack(255,   0,   0)  },
                {"GREEN",       RGBA::Pack(  0, 255,   0)  },
                {"BLUE",        RGBA::Pack(  0,   0, 255)  },
                {"YELLOW",      RGBA::Pack(255, 255,   0)  },

                // window event constants
                {"WE_KEY_PRESS",                video::WindowEvent::KEY_PRESS              },
                {"WE_KEY_RELEASE",              video::WindowEvent::KEY_RELEASE            },
                {"WE_MOUSE_BUTTON_PRESS",       video::WindowEvent::MOUSE_BUTTON_PRESS     },
                {"WE_MOUSE_BUTTON_RELEASE",     video::WindowEvent::MOUSE_BUTTON_RELEASE   },
                {"WE_MOUSE_MOTION",             video::WindowEvent::MOUSE_MOTION           },
                {"WE_MOUSE_WHEEL_MOTION",       video::WindowEvent::MOUSE_WHEEL_MOTION     },
                {"WE_WINDOW_CLOSE",             video::WindowEvent::WINDOW_CLOSE           },

                {0}
            };

            //-----------------------------------------------------------------
            bool RegisterGraphicsLibrary(HSQUIRRELVM v)
            {
                /* Canvas */

                // create canvas class
                sq_newclass(v, SQFalse);

                // set up canvas class
                sq_settypetag(v, -1, TT_CANVAS);
                util::RegisterFunctions(v, _canvas_methods);
                util::RegisterFunctions(v, _canvas_static_methods, true);

                // register canvas class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "Canvas", -1);
                sq_push(v, -3); // push canvas class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register canvas class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "Canvas", -1);
                sq_push(v, -3); // push canvas class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop canvas class
                sq_poptop(v);

                /* Texture */

                // create texture class
                sq_newclass(v, SQFalse);

                // set up texture class
                sq_settypetag(v, -1, TT_TEXTURE);
                util::RegisterFunctions(v, _texture_methods);
                util::RegisterFunctions(v, _texture_static_methods, true);

                // register texture class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "Texture", -1);
                sq_push(v, -3); // push texture class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register texture class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "Texture", -1);
                sq_push(v, -3); // push texture class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop texture class
                sq_poptop(v);

                /* Global Symbols */

                sq_pushroottable(v);
                util::RegisterFunctions(v, _graphics_functions);
                util::RegisterConstants(v, _graphics_constants);
                sq_poptop(v); // pop root table

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
