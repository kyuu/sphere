#ifndef SPHERE_SCRIPT_VM_HPP
#define SPHERE_SCRIPT_VM_HPP

#include <string>
#include <vector>
#include <squirrel.h>
#include "../Log.hpp"
#include "../io/IStream.hpp"

// script file extensions
#define   SCRIPT_FILE_EXT ".nut"
#define BYTECODE_FILE_EXT ".nutc"


namespace sphere {
    namespace script {

        const std::string& GetLastError();
        HSQUIRRELVM GetVM();
        bool        CompileBuffer(const void* buffer, int size, const std::string& scriptName = "unknown");
        bool        CompileStream(IStream* stream, const std::string& scriptName = "unknown", int count = -1);
        bool        EvaluateScript(const std::string& filename);
        bool        JSONStringify(SQInteger idx);
        bool        JSONParse(const char* jsonstr);
        bool        DumpObject(SQInteger idx, IStream* stream);
        bool        LoadObject(IStream* stream);
        SQRESULT    ThrowError(const char* format, ...);

        namespace internal {

            bool InitVM(const Log& log);
            void DeinitVM();

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
