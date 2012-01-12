#include <cassert>
#include "../input/input.hpp"
#include "macros.hpp"
#include "util.hpp"
#include "vm.hpp"
#include "inputlib.hpp"


namespace sphere {
    namespace script {
        namespace internal {

            //-----------------------------------------------------------------
            // GetNumJoysticks()
            static SQInteger _input_GetNumJoysticks(HSQUIRRELVM v)
            {
                RET_INT(input::GetNumJoysticks())
            }

            //-----------------------------------------------------------------
            // GetJoystickName(joy)
            static SQInteger _input_GetJoystickName(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, joy)
                const char* name = input::GetJoystickName(joy);
                if (name) {
                    RET_STRING(name)
                } else {
                    RET_NULL()
                }
            }

            //-----------------------------------------------------------------
            // GetNumJoystickButtons(joy)
            static SQInteger _input_GetNumJoystickButtons(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, joy)
                RET_INT(input::GetNumJoystickButtons(joy))
            }

            //-----------------------------------------------------------------
            // GetNumJoystickAxes(joy)
            static SQInteger _input_GetNumJoystickAxes(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, joy)
                RET_INT(input::GetNumJoystickAxes(joy))
            }

            //-----------------------------------------------------------------
            // GetNumJoystickHats(joy)
            static SQInteger _input_GetNumJoystickHats(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, joy)
                RET_INT(input::GetNumJoystickHats(joy))
            }

            //-----------------------------------------------------------------
            // IsJoystickButtonDown(joy, button)
            static SQInteger _input_IsJoystickButtonDown(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_INT(1, joy)
                GET_ARG_INT(2, button)
                RET_BOOL(input::IsJoystickButtonDown(joy, button))
            }

            //-----------------------------------------------------------------
            // GetJoystickAxis(joy, axis)
            static SQInteger _input_GetJoystickAxis(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_INT(1, joy)
                GET_ARG_INT(2, axis)
                RET_INT(input::GetJoystickAxis(joy, axis))
            }

            //-----------------------------------------------------------------
            // GetJoystickHat(joy, hat)
            static SQInteger _input_GetJoystickHat(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_INT(1, joy)
                GET_ARG_INT(2, hat)
                RET_INT(input::GetJoystickHat(joy, hat))
            }

            //-----------------------------------------------------------------
            // IsJoystickHaptic(joy)
            static SQInteger _input_IsJoystickHaptic(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, joy)
                RET_BOOL(input::IsJoystickHaptic(joy))
            }

            //-----------------------------------------------------------------
            // CreateJoystickForce(joy, strength, duration)
            static SQInteger _input_CreateJoystickForce(HSQUIRRELVM v)
            {
                CHECK_NARGS(3)
                GET_ARG_INT(1, joy)
                GET_ARG_INT(2, strength)
                GET_ARG_INT(3, duration)
                if (strength < 0 || strength > 100) {
                    THROW_ERROR("Invalid strength")
                }
                int force = input::CreateJoystickForce(joy, strength, duration);
                if (force < 0) {
                    THROW_ERROR("Could not create joystick force")
                }
                RET_INT(force)
            }

            //-----------------------------------------------------------------
            // ApplyJoystickForce(joy, force [, times = 1])
            static SQInteger _input_ApplyJoystickForce(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(2)
                GET_ARG_INT(1, joy)
                GET_ARG_INT(2, force)
                GET_OPTARG_INT(3, times, 1)
                if (!input::ApplyJoystickForce(joy, force, times)) {
                    THROW_ERROR("Could not apply joystick force")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // StopJoystickForce(joy, force)
            static SQInteger _input_StopJoystickForce(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_INT(1, joy)
                GET_ARG_INT(2, force)
                if (!input::StopJoystickForce(joy, force)) {
                    THROW_ERROR("Could not stop joystick force")
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // StopAllJoystickForces(joy)
            static SQInteger _input_StopAllJoystickForces(HSQUIRRELVM v)
            {
                CHECK_NARGS(1)
                GET_ARG_INT(1, joy)
                input::StopAllJoystickForces(joy);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // DestroyJoystickForce(joy, force)
            static SQInteger _input_DestroyJoystickForce(HSQUIRRELVM v)
            {
                CHECK_NARGS(2)
                GET_ARG_INT(1, joy)
                GET_ARG_INT(2, force)
                input::DestroyJoystickForce(joy, force);
                RET_VOID()
            }

            //-----------------------------------------------------------------
            static util::Function _input_functions[] = {
                {"GetNumJoysticks",             "GetNumJoysticks",              _input_GetNumJoysticks              },
                {"GetJoystickName",             "GetJoystickName",              _input_GetJoystickName              },
                {"GetNumJoystickButtons",       "GetNumJoystickButtons",        _input_GetNumJoystickButtons        },
                {"GetNumJoystickAxes",          "GetNumJoystickAxes",           _input_GetNumJoystickAxes           },
                {"GetNumJoystickHats",          "GetNumJoystickHats",           _input_GetNumJoystickHats           },
                {"IsJoystickButtonDown",        "IsJoystickButtonDown",         _input_IsJoystickButtonDown         },
                {"GetJoystickAxis",             "GetJoystickAxis",              _input_GetJoystickAxis              },
                {"GetJoystickHat",              "GetJoystickHat",               _input_GetJoystickHat               },
                {"IsJoystickHaptic",            "IsJoystickHaptic",             _input_IsJoystickHaptic             },
                {"CreateJoystickForce",         "CreateJoystickForce",          _input_CreateJoystickForce          },
                {"ApplyJoystickForce",          "ApplyJoystickForce",           _input_ApplyJoystickForce           },
                {"StopJoystickForce",           "StopJoystickForce",            _input_StopJoystickForce            },
                {"StopAllJoystickForces",       "StopAllJoystickForces",        _input_StopAllJoystickForces        },
                {"DestroyJoystickForce",        "DestroyJoystickForce",         _input_DestroyJoystickForce         },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Constant _input_constants[] = {

                // key constants
                {"KEY_A",               input::KEY_A               },
                {"KEY_B",               input::KEY_B               },
                {"KEY_C",               input::KEY_C               },
                {"KEY_D",               input::KEY_D               },
                {"KEY_E",               input::KEY_E               },
                {"KEY_F",               input::KEY_F               },
                {"KEY_G",               input::KEY_G               },
                {"KEY_H",               input::KEY_H               },
                {"KEY_I",               input::KEY_I               },
                {"KEY_J",               input::KEY_J               },
                {"KEY_K",               input::KEY_K               },
                {"KEY_L",               input::KEY_L               },
                {"KEY_M",               input::KEY_M               },
                {"KEY_N",               input::KEY_N               },
                {"KEY_O",               input::KEY_O               },
                {"KEY_P",               input::KEY_P               },
                {"KEY_Q",               input::KEY_Q               },
                {"KEY_R",               input::KEY_R               },
                {"KEY_S",               input::KEY_S               },
                {"KEY_T",               input::KEY_T               },
                {"KEY_U",               input::KEY_U               },
                {"KEY_V",               input::KEY_V               },
                {"KEY_W",               input::KEY_W               },
                {"KEY_X",               input::KEY_X               },
                {"KEY_Y",               input::KEY_Y               },
                {"KEY_Z",               input::KEY_Z               },
                {"KEY_0",               input::KEY_0               },
                {"KEY_1",               input::KEY_1               },
                {"KEY_2",               input::KEY_2               },
                {"KEY_3",               input::KEY_3               },
                {"KEY_4",               input::KEY_4               },
                {"KEY_5",               input::KEY_5               },
                {"KEY_6",               input::KEY_6               },
                {"KEY_7",               input::KEY_7               },
                {"KEY_8",               input::KEY_8               },
                {"KEY_9",               input::KEY_9               },
                {"KEY_F1",              input::KEY_F1              },
                {"KEY_F2",              input::KEY_F2              },
                {"KEY_F3",              input::KEY_F3              },
                {"KEY_F4",              input::KEY_F4              },
                {"KEY_F5",              input::KEY_F5              },
                {"KEY_F6",              input::KEY_F6              },
                {"KEY_F7",              input::KEY_F7              },
                {"KEY_F8",              input::KEY_F8              },
                {"KEY_F9",              input::KEY_F9              },
                {"KEY_F10",             input::KEY_F10             },
                {"KEY_F11",             input::KEY_F11             },
                {"KEY_F12",             input::KEY_F12             },
                {"KEY_TAB",             input::KEY_TAB             },
                {"KEY_BACKSPACE",       input::KEY_BACKSPACE       },
                {"KEY_ENTER",           input::KEY_ENTER           },
                {"KEY_ESCAPE",          input::KEY_ESCAPE          },
                {"KEY_SPACE",           input::KEY_SPACE           },
                {"KEY_PAGEUP",          input::KEY_PAGEUP          },
                {"KEY_PAGEDOWN",        input::KEY_PAGEDOWN        },
                {"KEY_HOME",            input::KEY_HOME            },
                {"KEY_END",             input::KEY_END             },
                {"KEY_LEFT",            input::KEY_LEFT            },
                {"KEY_UP",              input::KEY_UP              },
                {"KEY_RIGHT",           input::KEY_RIGHT           },
                {"KEY_DOWN",            input::KEY_DOWN            },
                {"KEY_INSERT",          input::KEY_INSERT          },
                {"KEY_DELETE",          input::KEY_DELETE          },
                {"KEY_PLUS",            input::KEY_PLUS            },
                {"KEY_COMMA",           input::KEY_COMMA           },
                {"KEY_MINUS",           input::KEY_MINUS           },
                {"KEY_PERIOD",          input::KEY_PERIOD          },
                {"KEY_CAPSLOCK",        input::KEY_CAPSLOCK        },
                {"KEY_SHIFT",           input::KEY_SHIFT           },
                {"KEY_CTRL",            input::KEY_CTRL            },
                {"KEY_ALT",             input::KEY_ALT             },
                {"KEY_OEM1",            input::KEY_OEM1            },
                {"KEY_OEM2",            input::KEY_OEM2            },
                {"KEY_OEM3",            input::KEY_OEM3            },
                {"KEY_OEM4",            input::KEY_OEM4            },
                {"KEY_OEM5",            input::KEY_OEM5            },
                {"KEY_OEM6",            input::KEY_OEM6            },
                {"KEY_OEM7",            input::KEY_OEM7            },
                {"KEY_OEM8",            input::KEY_OEM8            },

                // mouse constants
                {"MOUSE_BUTTON_LEFT",   input::MOUSE_BUTTON_LEFT   },
                {"MOUSE_BUTTON_MIDDLE", input::MOUSE_BUTTON_MIDDLE },
                {"MOUSE_BUTTON_RIGHT",  input::MOUSE_BUTTON_RIGHT  },
                {"MOUSE_BUTTON_X1",     input::MOUSE_BUTTON_X1     },
                {"MOUSE_BUTTON_X2",     input::MOUSE_BUTTON_X2     },

                // joystick constants
                {"JOY_AXIS_X",          input::JOY_AXIS_X          },
                {"JOY_AXIS_Y",          input::JOY_AXIS_Y          },
                {"JOY_AXIS_Z",          input::JOY_AXIS_Z          },
                {"JOY_AXIS_R",          input::JOY_AXIS_R          },

                {"JOY_HAT_CENTERED",    input::JOY_HAT_CENTERED    },
                {"JOY_HAT_UP",          input::JOY_HAT_UP          },
                {"JOY_HAT_RIGHT",       input::JOY_HAT_RIGHT       },
                {"JOY_HAT_DOWN",        input::JOY_HAT_DOWN        },
                {"JOY_HAT_LEFT",        input::JOY_HAT_LEFT        },
                {"JOY_HAT_RIGHTUP",     input::JOY_HAT_RIGHTUP     },
                {"JOY_HAT_RIGHTDOWN",   input::JOY_HAT_RIGHTDOWN   },
                {"JOY_HAT_LEFTUP",      input::JOY_HAT_LEFTUP      },
                {"JOY_HAT_LEFTDOWN",    input::JOY_HAT_LEFTDOWN    },

                {0,0}
            };

            bool RegisterInputLibrary(HSQUIRRELVM v)
            {
                /* Global Symbols */

                sq_pushroottable(v);
                util::RegisterFunctions(v, _input_functions);
                util::RegisterConstants(v, _input_constants);
                sq_poptop(v); // pop root table

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
