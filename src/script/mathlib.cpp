#include <cmath>
#include <ctime>
#include <algorithm>
#include "macros.hpp"
#include "util.hpp"
#include "vm.hpp"
#include "mathlib.hpp"

#define PI 3.14159f


namespace sphere {
    namespace script {
        namespace internal {

            //-----------------------------------------------------------------
            // cos(x)
            static SQInteger _math_cos(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(cos(x))
            }

            //-----------------------------------------------------------------
            // sin(x)
            static SQInteger _math_sin(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(sin(x))
            }

            //-----------------------------------------------------------------
            // tan(x)
            static SQInteger _math_tan(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(tan(x))
            }

            //-----------------------------------------------------------------
            // acos(x)
            static SQInteger _math_acos(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(acos(x))
            }

            //-----------------------------------------------------------------
            // asin(x)
            static SQInteger _math_asin(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(asin(x))
            }

            //-----------------------------------------------------------------
            // atan(x)
            static SQInteger _math_atan(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(atan(x))
            }

            //-----------------------------------------------------------------
            // atan2(y, x)
            static SQInteger _math_atan2(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_FLOAT(1, y)
                GET_ARG_FLOAT(2, x)
                RET_FLOAT(atan2(y, x))
            }

            //-----------------------------------------------------------------
            // cosh(x)
            static SQInteger _math_cosh(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(cosh(x))
            }

            //-----------------------------------------------------------------
            // sinh(x)
            static SQInteger _math_sinh(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(sinh(x))
            }

            //-----------------------------------------------------------------
            // tanh(x)
            static SQInteger _math_tanh(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(tanh(x))
            }

            //-----------------------------------------------------------------
            // exp(x)
            static SQInteger _math_exp(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(exp(x))
            }

            //-----------------------------------------------------------------
            // log(x)
            static SQInteger _math_log(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(log(x))
            }

            //-----------------------------------------------------------------
            // log2(x)
            static SQInteger _math_log2(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(log(x) / log(2.0f))
            }

            //-----------------------------------------------------------------
            // log10(x)
            static SQInteger _math_log10(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(log10(x))
            }

            //-----------------------------------------------------------------
            // pow(base, exponent)
            static SQInteger _math_pow(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_FLOAT(1, base)
                GET_ARG_FLOAT(2, exponent)
                RET_FLOAT(pow(base, exponent))
            }

            //-----------------------------------------------------------------
            // sqrt(x)
            static SQInteger _math_sqrt(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(sqrt(x))
            }

            //-----------------------------------------------------------------
            // ceil(x)
            static SQInteger _math_ceil(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(ceil(x))
            }

            //-----------------------------------------------------------------
            // floor(x)
            static SQInteger _math_floor(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT(floor(x))
            }

            //-----------------------------------------------------------------
            // round(x)
            static SQInteger _math_round(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_FLOAT(1, x)
                RET_FLOAT((x > 0.0f) ? floor(x + 0.5f) : ceil(x - 0.5f))
            }

            //-----------------------------------------------------------------
            // abs(x)
            static SQInteger _math_abs(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                if (sq_gettype(v, 2) == OT_INTEGER) {
                    GET_ARG_INT(1, x)
                    RET_INT(abs(x))
                } else {
                    GET_ARG_FLOAT(1, x)
                    RET_FLOAT(fabs(x))
                }
            }

            //-----------------------------------------------------------------
            // min(a, b)
            static SQInteger _math_min(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                if (sq_gettype(v, 2) == OT_INTEGER && sq_gettype(v, 3) == OT_INTEGER) {
                    GET_ARG_INT(1, a)
                    GET_ARG_INT(2, b)
                    RET_INT(std::min(a, b))
                } else {
                    GET_ARG_FLOAT(1, a)
                    GET_ARG_FLOAT(2, b)
                    RET_FLOAT(std::min(a, b))
                }
            }

            //-----------------------------------------------------------------
            // max(a, b)
            static SQInteger _math_max(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                if (sq_gettype(v, 2) == OT_INTEGER && sq_gettype(v, 3) == OT_INTEGER) {
                    GET_ARG_INT(1, a)
                    GET_ARG_INT(2, b)
                    RET_INT(std::max(a, b))
                } else {
                    GET_ARG_FLOAT(1, a)
                    GET_ARG_FLOAT(2, b)
                    RET_FLOAT(std::max(a, b))
                }
            }

            //-----------------------------------------------------------------
            // rand()
            static SQInteger _math_rand(HSQUIRRELVM v)
            {
                RET_INT(rand())
            }

            //-----------------------------------------------------------------
            // randf()
            static SQInteger _math_randf(HSQUIRRELVM v)
            {
                RET_FLOAT(rand() / (float)RAND_MAX)
            }

            //-----------------------------------------------------------------
            // easeIn(startVal, endVal, curVal)
            static SQInteger _math_easeIn(HSQUIRRELVM v)
            {
                CHECK_NARGS(3)
                GET_ARG_FLOAT(1, startVal)
                GET_ARG_FLOAT(2, endVal)
                GET_ARG_FLOAT(3, curVal)
                float nmlCur = curVal - startVal;
                float nmlEnd = endVal - startVal;
                float easedVal = endVal - cos((PI / 2.0f) * (nmlCur / nmlEnd)) * nmlEnd;
                RET_FLOAT(easedVal)
            }

            //-----------------------------------------------------------------
            // easeOut(startVal, endVal, curVal)
            static SQInteger _math_easeOut(HSQUIRRELVM v)
            {
                CHECK_NARGS(3)
                GET_ARG_FLOAT(1, startVal)
                GET_ARG_FLOAT(2, endVal)
                GET_ARG_FLOAT(3, curVal)
                float nmlCur = curVal - startVal;
                float nmlEnd = endVal - startVal;
                float easedVal = startVal - cos(((PI / 2.0f) * (nmlCur / nmlEnd)) + (PI / 2.0f)) * nmlEnd;
                RET_FLOAT(easedVal)
            }

            //-----------------------------------------------------------------
            // easeInOut(startVal, endVal, curVal)
            static SQInteger _math_easeInOut(HSQUIRRELVM v)
            {
                CHECK_NARGS(3)
                GET_ARG_FLOAT(1, startVal)
                GET_ARG_FLOAT(2, endVal)
                GET_ARG_FLOAT(3, curVal)
                float nmlCur = curVal - startVal;
                float nmlEnd = endVal - startVal;
                float halfNmlEnd = nmlEnd / 2.0f;
                float easedVal = (startVal + halfNmlEnd) - cos(PI * (nmlCur / nmlEnd)) * halfNmlEnd;
                RET_FLOAT(easedVal)
            }

            //-----------------------------------------------------------------
            static util::Function _math_functions[] = {
                {"cos",         "Math.cos",         _math_cos       },
                {"sin",         "Math.sin",         _math_sin       },
                {"tan",         "Math.tan",         _math_tan       },
                {"acos",        "Math.acos",        _math_acos      },
                {"asin",        "Math.asin",        _math_asin      },
                {"atan",        "Math.atan",        _math_atan      },
                {"atan2",       "Math.atan2",       _math_atan2     },
                {"cosh",        "Math.cosh",        _math_cosh      },
                {"sinh",        "Math.sinh",        _math_sinh      },
                {"tanh",        "Math.tanh",        _math_tanh      },
                {"exp",         "Math.exp",         _math_exp       },
                {"log",         "Math.log",         _math_log       },
                {"log2",        "Math.log2",        _math_log2      },
                {"log10",       "Math.log10",       _math_log10     },
                {"pow",         "Math.pow",         _math_pow       },
                {"sqrt",        "Math.sqrt",        _math_sqrt      },
                {"ceil",        "Math.ceil",        _math_ceil      },
                {"floor",       "Math.floor",       _math_floor     },
                {"round",       "Math.round",       _math_round     },
                {"abs",         "Math.abs",         _math_abs       },
                {"min",         "Math.min",         _math_min       },
                {"max",         "Math.max",         _math_max       },
                {"rand",        "Math.rand",        _math_rand      },
                {"randf",       "Math.randf",       _math_randf     },
                {"easeIn",      "Math.easeIn",      _math_easeIn    },
                {"easeOut",     "Math.easeOut",     _math_easeOut   },
                {"easeInOut",   "Math.easeInOut",   _math_easeInOut },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Constant _math_constants[] = {
                {"PI",          PI          },
                {"PI_2",        PI / 2.0f   },
                {"PI_4",        PI / 4.0f   },
                {"M_1_PI",      1.0f / PI   },
                {"M_2_PI",      2.0f / PI   },
                {"E",           2.71828f    },
                {"DEGTORAD",    PI / 180.0f },
                {"RADTODEG",    180.0f / PI },
                {"SQRT1_2",     0.70711f    },
                {"SQRT_2",      1.41421f    },
                {"LN2",         0.69315f    },
                {"RAND_MAX",    RAND_MAX    },
                {0}
            };

            //-----------------------------------------------------------------
            bool RegisterMathLibrary(HSQUIRRELVM v)
            {
                // seed random number generator
                srand((unsigned int)time(0));

                /* Global Symbols */

                sq_pushroottable(v);
                sq_pushstring(v, "Math", -1);
                sq_newtable(v);
                util::RegisterFunctions(v, _math_functions);
                util::RegisterConstants(v, _math_constants);
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
