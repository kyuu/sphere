#include <cassert>
#include "../common/ArrayPtr.hpp"
#include "../io/numio.hpp"
#include "macros.hpp"
#include "util.hpp"
#include "vm.hpp"
#include "baselib.hpp"
#include "iolib.hpp"


namespace sphere {
    namespace script {

        namespace internal {

            static SQInteger _stream_destructor(SQUserPointer p, SQInteger size);
            static SQInteger _file_destructor(SQUserPointer p, SQInteger size);

        } // namespace internal

        //-----------------------------------------------------------------
        bool BindStream(HSQUIRRELVM v, IStream* stream)
        {
            assert(stream);

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

            // create instance
            sq_createinstance(v, -1);

            // pop stream class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_stream_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)stream);

            // grab a new reference
            stream->grab();

            return true;
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
        bool BindFile(HSQUIRRELVM v, IFile* file)
        {
            assert(file);

            // get file class
            sq_pushregistrytable(v);
            sq_pushstring(v, "File", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                return false;
            }
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_FILE) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop file class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_file_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)file);

            // grab a new reference
            file->grab();

            return true;
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

        namespace internal {

            #define SETUP_STREAM_OBJECT() \
                IStream* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_STREAM))) { \
                    THROW_ERROR("Invalid type of environment object, expected a Stream instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _stream_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((IStream*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // Stream.isOpen()
            static SQInteger _stream_isOpen(HSQUIRRELVM v)
            {
                SETUP_STREAM_OBJECT()
                RET_BOOL(This->isOpen())
            }

            //-----------------------------------------------------------------
            // Stream.isReadable()
            static SQInteger _stream_isReadable(HSQUIRRELVM v)
            {
                SETUP_STREAM_OBJECT()
                RET_BOOL(This->isReadable())
            }

            //-----------------------------------------------------------------
            // Stream.isWriteable()
            static SQInteger _stream_isWriteable(HSQUIRRELVM v)
            {
                SETUP_STREAM_OBJECT()
                RET_BOOL(This->isWriteable())
            }

            //-----------------------------------------------------------------
            // Stream.close()
            static SQInteger _stream_close(HSQUIRRELVM v)
            {
                SETUP_STREAM_OBJECT()
                This->close();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Stream.tell()
            static SQInteger _stream_tell(HSQUIRRELVM v)
            {
                SETUP_STREAM_OBJECT()
                RET_INT(This->tell())
            }

            //-----------------------------------------------------------------
            // Stream.seek(offset [, origin = Stream.BEG])
            static SQInteger _stream_seek(HSQUIRRELVM v)
            {
                SETUP_STREAM_OBJECT()
                CHECK_MIN_NARGS(1)
                GET_ARG_INT(1, offset)
                GET_OPTARG_INT(2, origin, IStream::BEG)
                RET_BOOL(This->seek(offset, origin))
            }

            //-----------------------------------------------------------------
            // Stream.read(size)
            static SQInteger _stream_read(HSQUIRRELVM v)
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
            static SQInteger _stream_write(HSQUIRRELVM v)
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
            static SQInteger _stream_flush(HSQUIRRELVM v)
            {
                SETUP_STREAM_OBJECT()
                RET_BOOL(This->flush())
            }

            //-----------------------------------------------------------------
            // Stream.eof()
            static SQInteger _stream_eof(HSQUIRRELVM v)
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
            static SQInteger _stream_readNumber(HSQUIRRELVM v)
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
            static SQInteger _stream_readString(HSQUIRRELVM v)
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
                ArrayPtr<u8> buf(new u8[length]);
                if (This->read(buf.get(), length) != length) {
                    THROW_ERROR("Read error")
                }
                RET_STRING_N((const SQChar*)buf.get(), length)
            }

            //-----------------------------------------------------------------
            // Stream.writeNumber(type, number [, endian = 'l'])
            static SQInteger _stream_writeNumber(HSQUIRRELVM v)
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
            static SQInteger _stream_writeString(HSQUIRRELVM v)
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
            static util::Function _stream_methods[] = {
                {"isOpen",      "Stream.isOpen",        _stream_isOpen        },
                {"isReadable",  "Stream.isReadable",    _stream_isReadable    },
                {"isWriteable", "Stream.isWriteable",   _stream_isWriteable   },
                {"close",       "Stream.close",         _stream_close         },
                {"tell",        "Stream.tell",          _stream_tell          },
                {"seek",        "Stream.seek",          _stream_seek          },
                {"read",        "Stream.read",          _stream_read          },
                {"write",       "Stream.write",         _stream_write         },
                {"flush",       "Stream.flush",         _stream_flush         },
                {"eof",         "Stream.eof",           _stream_eof           },
                {"readNumber",  "Stream.readNumber",    _stream_readNumber    },
                {"readString",  "Stream.readString",    _stream_readString    },
                {"writeNumber", "Stream.writeNumber",   _stream_writeNumber   },
                {"writeString", "Stream.writeString",   _stream_writeString   },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Constant _stream_static_constants[] = {
                {"BEG",     IStream::BEG    },
                {"CUR",     IStream::CUR    },
                {"END",     IStream::END    },
                {0,0}
            };

            #define SETUP_FILE_OBJECT() \
                IFile* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_FILE))) { \
                    THROW_ERROR("Invalid type of environment object, expected a File instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _file_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((IFile*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // File.Open(filename [, mode = File.IN])
            static SQInteger _file_Open(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(1)
                GET_ARG_STRING(1, filename)
                GET_OPTARG_INT(2, mode, IFile::FM_IN)
                if (mode != IFile::FM_IN  &&
                    mode != IFile::FM_OUT &&
                    mode != IFile::FM_APPEND)
                {
                    THROW_ERROR("Invalid mode")
                }
                FilePtr file = io::filesystem::OpenFile(filename, mode);
                if (!file) {
                    THROW_ERROR1("Could not open file '%s'", filename)
                }
                RET_FILE(file.get())
            }

            //-----------------------------------------------------------------
            // File.getName()
            static SQInteger _file_getName(HSQUIRRELVM v)
            {
                SETUP_FILE_OBJECT()
                RET_STRING(This->getName().c_str())
            }

            //-----------------------------------------------------------------
            // File._typeof()
            static SQInteger _file__typeof(HSQUIRRELVM v)
            {
                SETUP_FILE_OBJECT()
                RET_STRING("File")
            }

            //-----------------------------------------------------------------
            // File._tostring()
            static SQInteger _file__tostring(HSQUIRRELVM v)
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
            static util::Function _file_methods[] = {
                {"getName",     "File.getName",     _file_getName     },
                {"_typeof",     "File._typeof",     _file__typeof     },
                {"_tostring",   "File._tostring",   _file__tostring   },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _file_static_methods[] = {
                {"Open",   "File.Open",   _file_Open   },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Constant _file_static_constants[] = {
                {"IN",      IFile::FM_IN       },
                {"OUT",     IFile::FM_OUT      },
                {"APPEND",  IFile::FM_APPEND   },
                {0,0}
            };

            //-----------------------------------------------------------------
            // FileExists(filename)
            static SQInteger _io_FileExists(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                RET_BOOL(io::filesystem::FileExists(filename))
            }

            //-----------------------------------------------------------------
            // IsFile(filename)
            static SQInteger _io_IsFile(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                RET_BOOL(io::filesystem::IsFile(filename))
            }

            //-----------------------------------------------------------------
            // IsDirectory(filename)
            static SQInteger _io_IsDirectory(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                RET_BOOL(io::filesystem::IsDirectory(filename))
            }

            //-----------------------------------------------------------------
            // GetFileSize(filename)
            static SQInteger _io_GetFileSize(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                RET_INT(io::filesystem::GetFileSize(filename))
            }

            //-----------------------------------------------------------------
            // GetFileModTime(filename)
            static SQInteger _io_GetFileModTime(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                RET_INT(io::filesystem::GetFileModTime(filename))
            }

            //-----------------------------------------------------------------
            // CreateDirectory(directory)
            static SQInteger _io_CreateDirectory(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, directory)
                if (!io::filesystem::CreateDirectory(directory)) {
                    THROW_ERROR("Could not create directory")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // RemoveFile(filename)
            static SQInteger _io_RemoveFile(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, filename)
                if (!io::filesystem::RemoveFile(filename)) {
                    THROW_ERROR("Could not remove file or directory")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // RenameFile(filenameFrom, filenameTo)
            static SQInteger _io_RenameFile(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_STRING(1, filenameFrom)
                GET_ARG_STRING(2, filenameTo)
                if (!io::filesystem::RenameFile(filenameFrom, filenameTo)) {
                    THROW_ERROR("Could not rename file or directory")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // EnumerateFiles(directory)
            static SQInteger _io_EnumerateFiles(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, directory)
                std::vector<std::string> file_list;
                if (!io::filesystem::EnumerateFiles(directory, file_list)) {
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
            static util::Function _io_functions[] = {
                {"FileExists",      "FileExists",       _io_FileExists       },
                {"IsFile",          "IsFile",           _io_IsFile           },
                {"IsDirectory",     "IsDirectory",      _io_IsDirectory      },
                {"GetFileSize",     "GetFileSize",      _io_GetFileSize      },
                {"GetFileModTime",  "GetFileModTime",   _io_GetFileModTime   },
                {"CreateDirectory", "CreateDirectory",  _io_CreateDirectory  },
                {"RemoveFile",      "RemoveFile",       _io_RemoveFile       },
                {"RenameFile",      "RenameFile",       _io_RenameFile       },
                {"EnumerateFiles",  "EnumerateFiles",   _io_EnumerateFiles   },
                {0,0}
            };

            //-----------------------------------------------------------------
            bool RegisterIOLibrary(const Log& log, HSQUIRRELVM v)
            {
                /* Stream */

                // create stream class
                sq_newclass(v, SQFalse);

                // set up stream class
                sq_settypetag(v, -1, TT_STREAM);
                util::RegisterFunctions(v, _stream_methods);
                util::RegisterConstants(v, _stream_static_constants, true);

                // register stream class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "Stream", -1);
                sq_push(v, -3); // push stream class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register stream class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "Stream", -1);
                sq_push(v, -3); // push stream class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop stream class
                sq_poptop(v);

                /* File */

                // get stream class
                sq_pushregistrytable(v);
                sq_pushstring(v, "Stream", -1);
                if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                    log.error() << "Could not get stream class";
                    return false;
                }
                sq_remove(v, -2); // pop registry table
                SQUserPointer tt = 0;
                if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_STREAM) {
                    log.error() << "Invalid stream class: " << (int)tt;
                    sq_poptop(v);
                    return false;
                }

                // create file class, inheriting from stream class
                sq_newclass(v, SQTrue);

                // set up file class
                sq_settypetag(v, -1, TT_FILE);
                util::RegisterFunctions(v, _file_methods);
                util::RegisterFunctions(v, _file_static_methods, true);
                util::RegisterConstants(v, _file_static_constants, true);

                // register file class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "File", -1);
                sq_push(v, -3); // push file class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register file class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "File", -1);
                sq_push(v, -3); // push file class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop file class
                sq_poptop(v);

                /* Global Symbols */

                sq_pushroottable(v);
                util::RegisterFunctions(v, _io_functions);
                sq_poptop(v); // pop root table

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
