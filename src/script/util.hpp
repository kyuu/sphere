#ifndef SPHERE_SCRIPT_UTIL_HPP
#define SPHERE_SCRIPT_UTIL_HPP

#include <squirrel.h>


namespace sphere {
    namespace script {
        namespace util {

            struct Function {
                const char* name;
                const char* debugName;
                SQFUNCTION  function;
            };

            struct Variant {
                enum {
                    VT_UNINITIALIZED = -1,
                    VT_STRING = 0,
                    VT_INTEGER,
                    VT_FLOAT,
                } type;

                union {
                    const char* s;
                    int i;
                    float f;
                } value;

                Variant() {
                    type = VT_UNINITIALIZED;
                    value.s = 0;
                }

                Variant(const char* s) {
                    type = VT_STRING;
                    value.s = s;
                }

                Variant(int i) {
                    type = VT_INTEGER;
                    value.i = i;
                }

                Variant(unsigned int i) {
                    type = VT_INTEGER;
                    value.i = (int)i;
                }

                Variant(float f) {
                    type = VT_FLOAT;
                    value.f = f;
                }
            };

            struct Constant {
                const char* name;
                Variant value;
            };

            bool RegisterFunctions(HSQUIRRELVM v, const Function* functions, bool areStatic = false);
            bool RegisterConstants(HSQUIRRELVM v, const Constant* constants, bool areStatic = false);

        } // namespace util
    } // namespace script
} // namespace sphere


#endif
