#include <cassert>
#include "util.hpp"


namespace sphere {
    namespace script {
        namespace util {

            //-----------------------------------------------------------------
            bool RegisterFunctions(HSQUIRRELVM v, const Function* functions, bool areStatic)
            {
                assert(functions);
                for (int i = 0; functions[i].name != 0; i++) {
                    sq_pushstring(v, functions[i].name, -1);
                    sq_newclosure(v, functions[i].function, 0);
                    sq_setnativeclosurename(v, -1, functions[i].debugName);
                    if (!SQ_SUCCEEDED(sq_newslot(v, -3, (areStatic ? SQTrue : SQFalse)))) {
                        return false;
                    }
                }
                return true;
            }

            //-----------------------------------------------------------------
            bool RegisterConstants(HSQUIRRELVM v, const Constant* constants, bool areStatic)
            {
                assert(constants);
                for (int i = 0; constants[i].name != 0; i++) {
                    sq_pushstring(v, constants[i].name, -1);
                    switch (constants[i].value.type) {
                        case Variant::VT_STRING:  sq_pushstring(v, constants[i].value.value.s, -1);  break;
                        case Variant::VT_INTEGER: sq_pushinteger(v, constants[i].value.value.i);     break;
                        case Variant::VT_FLOAT:   sq_pushfloat(v, constants[i].value.value.f);       break;
                        default:
                            return false;
                    }
                    if (!SQ_SUCCEEDED(sq_newslot(v, -3, (areStatic ? SQTrue : SQFalse)))) {
                        return false;
                    }
                }
                return true;
            }

        } // namespace util
    } // namespace script
} // namespace sphere
