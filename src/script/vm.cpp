#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <squirrel.h>
#include "../io/numio.hpp"
#include "macros.hpp"
#include "util.hpp"
#include "systemlib.hpp"
#include "graphicslib.hpp"
#include "audiolib.hpp"
#include "inputlib.hpp"
#include "iolib.hpp"
#include "compressionlib.hpp"
#include "mathlib.hpp"
#include "baselib.hpp"
#include "vm.hpp"

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


namespace sphere {
    namespace script {

        //-----------------------------------------------------------------
        // globals
        static HSQUIRRELVM g_VM = 0;
        static std::string g_LastError;
        static std::vector<std::string> g_LoadedScripts;

        //-----------------------------------------------------------------
        static char* get_scratch_pad(int& size)
        {
            static BlobPtr s_ScratchPad;
            if (!s_ScratchPad) {
                s_ScratchPad = Blob::Create(512);
            }

            if (size > 0 && s_ScratchPad->getSize() < size) {
                s_ScratchPad->resize(size);
            }

            size = s_ScratchPad->getSize();

            return (char*)s_ScratchPad->getBuffer();
        }

        //-----------------------------------------------------------------
        const std::string& GetLastError()
        {
            return g_LastError;
        }

        //-----------------------------------------------------------------
        HSQUIRRELVM GetVM()
        {
            return g_VM;
        }

        //-----------------------------------------------------------------
        SQRESULT ThrowError(const char* format, ...)
        {
            int   buf_size = 0;
            char* buf      = get_scratch_pad(buf_size);

            // format error message
            va_list arglist;
            va_start(arglist, format);

            int size = vsnprintf(buf, buf_size, format, arglist);

        #ifdef _MSC_VER // VC's vsnprintf has different behavior than POSIX's vsnprintf
            while (size < 0 || size == buf_size) { // buffer was not big enough to hold the output string + terminating null character
                // double buffer size
                buf_size = buf_size * 2;
                buf      = get_scratch_pad(buf_size);

                // try again
                va_start(arglist, format);
                size = vsnprintf(buf, buf_size, format, arglist);
            }
        #else
            if (size < 0) { // formatting error occurred
                return sq_throwerror(g_VM, format); // just throw the format string
            } else if (size >= buf_size) { // buffer was not big enough to hold the output string + terminating null character
                // double buffer size
                buf_size = size + 1;
                buf      = get_scratch_pad(buf_size);

                // try again
                va_start(arglist, format);
                vsnprintf(buf, buf_size, format, arglist);
            }
        #endif

            va_end(arglist);

            // get call information
            // funcname: name of the throwing function (the topmost function on squirrel's call stack)
            // source:   name of the script from where the function was called (directly or indirectly)
            // line:     the line in the source
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
            oss << source;
            oss << "(";
            oss << line;
            oss << "), in function ";
            oss << funcname;
            oss << "(): ";
            oss << buf;

            // pass error message to squirrel triggering an exception
            sq_pushstring(g_VM, oss.str().c_str(), -1);
            return sq_throwobject(g_VM);
        }

        //-----------------------------------------------------------------
        bool CompileBuffer(const void* buffer, int size, const std::string& scriptName)
        {
            assert(buffer);
            assert(size > 0);
            return SQ_SUCCEEDED(sq_compilebuffer(g_VM, (const SQChar*)buffer, size, scriptName.c_str(), SQTrue));
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
        bool EvaluateScript(const std::string& filename)
        {
            int old_top = sq_gettop(g_VM); // save stack top

            // load script
            if (io::filesystem::FileExists(filename)) {
                FilePtr file = io::filesystem::OpenFile(filename);
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
        bool JSONParse(const char* jsonstr)
        {
            assert(jsonstr);

            int oldtop = sq_gettop(g_VM);

            // compile string
            std::string src = std::string("return ") + jsonstr;
            if (!CompileBuffer(src.c_str(), src.length())) {
                return false;
            }

            // execute closure
            sq_pushroottable(g_VM); // this
            if (!SQ_SUCCEEDED(sq_call(g_VM, 1, SQTrue, SQTrue))) {
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
                BindStream(g_VM, stream); // push the output stream
                bool succeeded = SQ_SUCCEEDED(sq_call(g_VM, 3, SQFalse, SQTrue));
                sq_settop(g_VM, oldtop);
                return succeeded;
            }
            default:
                return false;
            }
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
                char* buf = get_scratch_pad(len);
                if (stream->read(buf, len) != len) {
                    return false;
                }
                sq_pushstring(g_VM, (const SQChar*)buf, len);
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
                char* buf = get_scratch_pad(len);
                if (stream->read(buf, len) != len) {
                    return false;
                }
                sq_pushroottable(g_VM);
                sq_pushstring(g_VM, (const SQChar*)buf, len);
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
                BindStream(g_VM, stream); // push the input stream
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

        namespace internal {

            //-----------------------------------------------------------------
            // Assert(expr)
            static SQInteger _script_Assert(HSQUIRRELVM v)
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
            // CompileString(string [, scriptName = "unknown"])
            static SQInteger _script_CompileString(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(1)
                GET_ARG_STRING(1, str)
                GET_OPTARG_STRING(2, scriptName, "unknown")
                int len = sq_getsize(g_VM, 2);
                if (len == 0) {
                    THROW_ERROR("Empty string")
                }
                if (!CompileBuffer(str, len, scriptName)) {
                    THROW_ERROR1("Could not compile string: %s", g_LastError.c_str())
                }
                return 1;
            }

            //-----------------------------------------------------------------
            // CompileBlob(blob [, scriptName = "unknown", offset = 0, count = -1])
            static SQInteger _script_CompileBlob(HSQUIRRELVM v)
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
                    THROW_ERROR1("Could not compile blob: %s", g_LastError.c_str())
                }
                return 1;
            }

            //-----------------------------------------------------------------
            // CompileStream(stream [, scriptName = "unknown", count = -1])
            static SQInteger _script_CompileStream(HSQUIRRELVM v)
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
                    THROW_ERROR1("Could not compile stream: %s", g_LastError.c_str())
                }
                return 1;
            }

            //-----------------------------------------------------------------
            // EvaluateString(string)
            static SQInteger _script_EvaluateString(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, str)
                int size = sq_getsize(v, 2);
                if (size == 0) {
                    THROW_ERROR("Empty string")
                }

                int oldtop = sq_gettop(v);

                // compile string
                if (!CompileBuffer(str, size)) {
                    sq_settop(v, oldtop);
                    THROW_ERROR1("Could not compile string: %s", g_LastError.c_str())
                }

                // evaluate closure
                sq_pushroottable(v); // this
                if (!SQ_SUCCEEDED(sq_call(v, 1, SQTrue, SQTrue))) {
                    sq_settop(v, oldtop);
                    THROW_ERROR1("Could not evaluate string: %s", g_LastError.c_str())
                }

                // the object is now on top of the stack
                int numtopop = (sq_gettop(v) - oldtop) - 1;
                while (numtopop > 0) {
                    sq_remove(v, -2);
                    numtopop--;
                }

                return 1;
            }

            //-----------------------------------------------------------------
            // EvaluateScript(name)
            static SQInteger _script_EvaluateScript(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, name)
                if (sq_getsize(v, 2) == 0) {
                    THROW_ERROR("Empty script name")
                }

                // evaluate script
                if (!EvaluateScript(name)) {
                    THROW_ERROR2("Could not evaluate script '%s': %s", name, g_LastError.c_str())
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // RequireScript(name)
            static SQInteger _script_RequireScript(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_STRING(1, name)
                if (sq_getsize(v, 2) == 0) {
                    THROW_ERROR("Empty script name")
                }

                std::string script = name;

                // see if the script has already been loaded
                for (int i = 0; i < (int)g_LoadedScripts.size(); ++i) {
                    if (g_LoadedScripts[i] == script) {
                        RET_VOID() // already loaded, nothing to do here
                    }
                }

                // evaluate script
                if (!EvaluateScript(script + BYTECODE_FILE_EXT) && // prefer bytecode
                    !EvaluateScript(script +   SCRIPT_FILE_EXT))
                {
                    THROW_ERROR2("Could not evaluate script '%s': %s", script.c_str(), g_LastError.c_str())
                }

                // register script
                g_LoadedScripts.push_back(script);

                RET_VOID()
            }

            //-----------------------------------------------------------------
            // GetLoadedScripts()
            static SQInteger _script_GetLoadedScripts(HSQUIRRELVM v)
            {
                sq_newarray(v, g_LoadedScripts.size());
                for (int i = 0; i < (int)g_LoadedScripts.size(); i++) {
                    sq_pushinteger(v, i);
                    sq_pushstring(v, g_LoadedScripts[i].c_str(), -1);
                    sq_rawset(v, -3);
                }
                return 1;
            }

            //-----------------------------------------------------------------
            // JSONStringify(object)
            static SQInteger _script_JSONStringify(HSQUIRRELVM v)
            {
                if (!JSONStringify(2)) {
                    THROW_ERROR("Could not stringify object")
                }
                return 1;
            }

            //-----------------------------------------------------------------
            // JSONParse(jsonstr)
            static SQInteger _script_JSONParse(HSQUIRRELVM v)
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
            // DumpObject(object, stream)
            static SQInteger _script_DumpObject(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_STREAM(2, stream)
                if (!stream->isOpen() || !stream->isWriteable()) {
                    THROW_ERROR("Invalid stream")
                }
                if (!DumpObject(2, stream)) {
                    THROW_ERROR("Error serializing")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // LoadObject(stream)
            static SQInteger _script_LoadObject(HSQUIRRELVM v)
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
            static util::Function _script_functions[] = {
                {"Assert",                  "Assert",               _script_Assert               },
                {"CompileString",           "CompileString",        _script_CompileString        },
                {"CompileBlob",             "CompileBlob",          _script_CompileBlob          },
                {"CompileStream",           "CompileStream",        _script_CompileStream        },
                {"EvaluateString",          "EvaluateString",       _script_EvaluateString       },
                {"EvaluateScript",          "EvaluateScript",       _script_EvaluateScript       },
                {"RequireScript",           "RequireScript",        _script_RequireScript        },
                {"GetLoadedScripts",        "GetLoadedScripts",     _script_GetLoadedScripts     },
                {"JSONStringify",           "JSONStringify",        _script_JSONStringify        },
                {"JSONParse",               "JSONParse",            _script_JSONParse            },
                {"DumpObject",              "DumpObject",           _script_DumpObject           },
                {"LoadObject",              "LoadObject",           _script_LoadObject           },
                {0,0}
            };

            //-----------------------------------------------------------------
            static void compiler_error_handler(HSQUIRRELVM v, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
            {
                std::ostringstream oss;
                oss << source;
                oss << "(";
                oss << line;
                oss << ":";
                oss << column;
                oss << "): ";
                oss << desc;
                g_LastError = oss.str();
            }

            //-----------------------------------------------------------------
            static SQInteger runtime_error_handler(HSQUIRRELVM v)
            {
                sq_tostring(v, 2); // stringify error
                const SQChar* error = 0;
                sq_getstring(v, -1, &error);
                g_LastError = error;
                sq_poptop(v); // pop error string
                return 0;
            }

            //-----------------------------------------------------------------
            static void print_func(HSQUIRRELVM v, const SQChar* format, ...)
            {
                int   buf_size = 0;
                char* buf      = get_scratch_pad(buf_size);

                // format error message
                va_list arglist;
                va_start(arglist, format);

                int size = vsnprintf(buf, buf_size, format, arglist);

            #ifdef _MSC_VER // VC's vsnprintf has different behavior than POSIX's vsnprintf
                while (size < 0 || size == buf_size) { // buffer was not big enough to hold the output string + terminating null character
                    // double buffer size
                    buf_size = buf_size * 2;
                    buf      = get_scratch_pad(buf_size);

                    // try again
                    va_start(arglist, format);
                    size = vsnprintf(buf, buf_size, format, arglist);
                }
            #else
                if (size < 0) { // formatting error occurred
                    return sq_throwerror(g_VM, format); // just throw the format string
                } else if (size >= buf_size) { // buffer was not big enough to hold the output string + terminating null character
                    // increase buffer size
                    buf_size = size + 1;
                    buf      = get_scratch_pad(buf_size);

                    // try again
                    va_start(arglist, format);
                    vsnprintf(buf, buf_size, format, arglist);
                }
            #endif

                va_end(arglist);

                // print to file
                static bool file_created = false;
                FILE* file = 0;
                if (!file_created) {
                    file = fopen("output.txt", "w");
                    file_created = true;
                } else {
                    file = fopen("output.txt", "a");
                }
                if (file) {
                    fprintf(file, "%s\n", buf);
                    fclose(file);
                }
            }

            //-----------------------------------------------------------------
            bool InitVM(const Log& log)
            {
                assert(!g_VM);

                // create squirrel vm
                g_VM = sq_open(1024);
                if (!g_VM) {
                    log.error() << "Could not create Squirrel VM";
                    return false;
                }

                // set up vm
                sq_setcompilererrorhandler(g_VM, compiler_error_handler);
                sq_newclosure(g_VM, runtime_error_handler, 0);
                sq_seterrorhandler(g_VM);
                sq_setprintfunc(g_VM, print_func, print_func);

                // register script functions
                sq_pushroottable(g_VM);
                util::RegisterFunctions(g_VM, _script_functions);
                sq_poptop(g_VM); // pop root table

                // register libs
                internal::RegisterSystemLibrary(g_VM);
                internal::RegisterIOLibrary(log, g_VM);
                internal::RegisterBaseLibrary(g_VM);
                internal::RegisterGraphicsLibrary(g_VM);
                internal::RegisterAudioLibrary(g_VM);
                internal::RegisterInputLibrary(g_VM);
                internal::RegisterCompressionLibrary(g_VM);
                internal::RegisterMathLibrary(g_VM);

                return true;
            }

            //-----------------------------------------------------------------
            void DeinitVM()
            {
                if (g_VM) {
                    sq_close(g_VM);
                    g_VM = 0;
                }
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
