#include "../common/platform.hpp"
#include "../version.hpp"
#include "../graphics/video.hpp"
#include "../input/input.hpp"
#include "../system/system.hpp"
#include "macros.hpp"
#include "util.hpp"
#include "vm.hpp"
#include "systemlib.hpp"


namespace sphere {
    namespace script {
        namespace internal {

            //-----------------------------------------------------------------
            // GetSphereVersion()
            static SQInteger _GetSphereVersion(HSQUIRRELVM v)
            {
                sq_newtable(v);

                sq_pushstring(v, "major", -1);
                sq_pushinteger(v, SPHERE_MAJOR);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "minor", -1);
                sq_pushinteger(v, SPHERE_MINOR);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "patch", -1);
                sq_pushinteger(v, SPHERE_PATCH);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "affix", -1);
                sq_pushstring(v, SPHERE_AFFIX, -1);
                sq_newslot(v, -3, SQFalse);

                return 1;
            }

            //-----------------------------------------------------------------
            // GetPlatform()
            static SQInteger _GetPlatform(HSQUIRRELVM v)
            {
            #if   defined(SPHERE_WINDOWS)
                RET_STRING("Windows")
            #elif defined(SPHERE_MAX_OS_X)
                RET_STRING("Mac OS X")
            #elif defined(SPHERE_LINUX)
                RET_STRING("Linux")
            #else
            #  error Platform undefined or not supported
            #endif
            }

            //-----------------------------------------------------------------
            // UpdateSystem()
            static SQInteger _UpdateSystem(HSQUIRRELVM v)
            {
                video::internal::ProcessWindowEvents();
                input::internal::UpdateInput();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Sleep(ms)
            static SQInteger _Sleep(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, ms)
                system::Sleep(ms);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // GetTicks()
            static SQInteger _GetTicks(HSQUIRRELVM v)
            {
                RET_INT(system::GetTicks())
            }

            //-----------------------------------------------------------------
            // GetTime()
            static SQInteger _GetTime(HSQUIRRELVM v)
            {
                RET_INT(system::GetTime())
            }

            //-----------------------------------------------------------------
            // GetTimeInfo([time, utc])
            static SQInteger _GetTimeInfo(HSQUIRRELVM v)
            {
                GET_OPTARG_INT(1, time, system::GetTime())
                GET_OPTARG_BOOL(2, utc, SQFalse)

                system::TimeInfo ti = system::GetTimeInfo(time, (utc == SQTrue ? true : false));

                sq_newtable(v);

                sq_pushstring(v, "second", -1);
                sq_pushinteger(v, ti.second);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "minute", -1);
                sq_pushinteger(v, ti.minute);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "hour", -1);
                sq_pushinteger(v, ti.hour);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "day", -1);
                sq_pushinteger(v, ti.day);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "month", -1);
                sq_pushinteger(v, ti.month);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "year", -1);
                sq_pushinteger(v, ti.year);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "weekday", -1);
                sq_pushinteger(v, ti.weekday);
                sq_newslot(v, -3, SQFalse);

                sq_pushstring(v, "yearday", -1);
                sq_pushinteger(v, ti.yearday);
                sq_newslot(v, -3, SQFalse);

                return 1;
            }

            //-----------------------------------------------------------------
            static util::Function _system_functions[] = {
                {"GetSphereVersion", "GetSphereVersion", _GetSphereVersion },
                {"GetPlatform",      "GetPlatform",      _GetPlatform      },
                {"UpdateSystem",     "UpdateSystem",     _UpdateSystem     },
                {"Sleep",            "Sleep",            _Sleep            },
                {"GetTicks",         "GetTicks",         _GetTicks         },
                {"GetTime",          "GetTime",          _GetTime          },
                {"GetTimeInfo",      "GetTimeInfo",      _GetTimeInfo      },
                {0,0}
            };

            bool RegisterSystemLibrary(HSQUIRRELVM v)
            {
                /* Global Symbols */

                sq_pushroottable(v);
                util::RegisterFunctions(v, _system_functions);
                sq_poptop(v); // pop root table

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
