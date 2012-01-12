#include <cassert>
#include "macros.hpp"
#include "util.hpp"
#include "vm.hpp"
#include "baselib.hpp"
#include "compressionlib.hpp"


namespace sphere {
    namespace script {

        namespace internal {

            static SQInteger _zstream_destructor(SQUserPointer p, SQInteger size);

        } // namespace internal

        //-----------------------------------------------------------------
        bool BindZStream(HSQUIRRELVM v, ZStream* stream)
        {
            assert(stream);

            // get zstream class
            sq_pushregistrytable(v);
            sq_pushstring(v, "ZStream", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                sq_poptop(v); // pop registry table
                return false;
            }
            sq_remove(v, -2); // remove registry table
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_ZSTREAM) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop zstream class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_zstream_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)stream);

            // grab a new reference
            stream->grab();

            return true;
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

        namespace internal {

            #define SETUP_ZSTREAM_OBJECT() \
                ZStream* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_ZSTREAM))) { \
                    THROW_ERROR("Invalid type of environment object, expected a ZStream instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _zstream_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((ZStream*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // ZStream()
            static SQInteger _zstream_constructor(HSQUIRRELVM v)
            {
                SETUP_ZSTREAM_OBJECT()
                This = ZStream::Create();
                sq_setinstanceup(v, 1, (SQUserPointer)This);
                sq_setreleasehook(v, 1, _zstream_destructor);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // ZStream.getBufferSize()
            static SQInteger _zstream_getBufferSize(HSQUIRRELVM v)
            {
                SETUP_ZSTREAM_OBJECT()
                RET_INT(This->getBufferSize())
            }

            //-----------------------------------------------------------------
            // ZStream.setBufferSize(size)
            static SQInteger _zstream_setBufferSize(HSQUIRRELVM v)
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
            static SQInteger _zstream_compress(HSQUIRRELVM v)
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
            static SQInteger _zstream_decompress(HSQUIRRELVM v)
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
            static SQInteger _zstream_finish(HSQUIRRELVM v)
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
            static SQInteger _zstream__typeof(HSQUIRRELVM v)
            {
                SETUP_ZSTREAM_OBJECT()
                RET_STRING("ZStream")
            }

            //-----------------------------------------------------------------
            // ZStream._tostring()
            static SQInteger _zstream__tostring(HSQUIRRELVM v)
            {
                SETUP_ZSTREAM_OBJECT()
                std::ostringstream oss;
                oss << "<ZStream instance at " << This;
                oss << ")>";
                RET_STRING(oss.str().c_str())
            }

            //-----------------------------------------------------------------
            static util::Function _zstream_methods[] = {
                {"constructor",     "ZStream.constructor",      _zstream_constructor      },
                {"getBufferSize",   "ZStream.getBufferSize",    _zstream_getBufferSize    },
                {"setBufferSize",   "ZStream.setBufferSize",    _zstream_setBufferSize    },
                {"compress",        "ZStream.compress",         _zstream_compress         },
                {"decompress",      "ZStream.decompress",       _zstream_decompress       },
                {"finish",          "ZStream.finish",           _zstream_finish           },
                {"_typeof",         "ZStream._typeof",          _zstream__typeof          },
                {"_tostring",       "ZStream._tostring",        _zstream__tostring        },
                {0,0}
            };

            bool RegisterCompressionLibrary(HSQUIRRELVM v)
            {
                /* ZStream */

                // create zstream class
                sq_newclass(v, SQFalse);

                // set up zstream class
                sq_settypetag(v, -1, TT_ZSTREAM);
                util::RegisterFunctions(v, _zstream_methods);

                // register zstream class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "ZStream", -1);
                sq_push(v, -3); // push zstream class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register zstream class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "ZStream", -1);
                sq_push(v, -3); // push zstream class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop zstream class
                sq_poptop(v);

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
