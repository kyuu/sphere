#ifndef SPHERE_INPUT_HPP
#define SPHERE_INPUT_HPP

#include <string>
#include "../Log.hpp"


namespace sphere {
    namespace input {

        /* Keyboard */

        enum {
            KEY_A = 0,
            KEY_B,
            KEY_C,
            KEY_D,
            KEY_E,
            KEY_F,
            KEY_G,
            KEY_H,
            KEY_I,
            KEY_J,
            KEY_K,
            KEY_L,
            KEY_M,
            KEY_N,
            KEY_O,
            KEY_P,
            KEY_Q,
            KEY_R,
            KEY_S,
            KEY_T,
            KEY_U,
            KEY_V,
            KEY_W,
            KEY_X,
            KEY_Y,
            KEY_Z,
            KEY_0,
            KEY_1,
            KEY_2,
            KEY_3,
            KEY_4,
            KEY_5,
            KEY_6,
            KEY_7,
            KEY_8,
            KEY_9,
            KEY_F1,
            KEY_F2,
            KEY_F3,
            KEY_F4,
            KEY_F5,
            KEY_F6,
            KEY_F7,
            KEY_F8,
            KEY_F9,
            KEY_F10,
            KEY_F11,
            KEY_F12,
            KEY_TAB,
            KEY_BACKSPACE,
            KEY_ENTER,
            KEY_ESCAPE,
            KEY_SPACE,
            KEY_PAGEUP,
            KEY_PAGEDOWN,
            KEY_HOME,
            KEY_END,
            KEY_LEFT,
            KEY_UP,
            KEY_RIGHT,
            KEY_DOWN,
            KEY_INSERT,
            KEY_DELETE,
            KEY_PLUS,
            KEY_COMMA,
            KEY_MINUS,
            KEY_PERIOD,
            KEY_CAPSLOCK,
            KEY_SHIFT,
            KEY_CTRL,
            KEY_ALT,
            KEY_OEM1,
            KEY_OEM2,
            KEY_OEM3,
            KEY_OEM4,
            KEY_OEM5,
            KEY_OEM6,
            KEY_OEM7,
            KEY_OEM8,
            NUM_KEYS,
        };

        /* Mouse */

        enum {
            MOUSE_BUTTON_LEFT = 0,
            MOUSE_BUTTON_MIDDLE,
            MOUSE_BUTTON_RIGHT,
            MOUSE_BUTTON_X1,
            MOUSE_BUTTON_X2,
            NUM_MOUSE_BUTTONS,
        };

        /* Joystick */

        enum {
            JOY_AXIS_X = 0,
            JOY_AXIS_Y,
            JOY_AXIS_Z,
            JOY_AXIS_R,

            JOY_HAT_CENTERED,
            JOY_HAT_UP,
            JOY_HAT_RIGHT,
            JOY_HAT_DOWN,
            JOY_HAT_LEFT,
            JOY_HAT_RIGHTUP,
            JOY_HAT_RIGHTDOWN,
            JOY_HAT_LEFTUP,
            JOY_HAT_LEFTDOWN,
        };

        int  GetNumJoysticks();
        const char* GetJoystickName(int joy);
        int  GetNumJoystickButtons(int joy);
        int  GetNumJoystickAxes(int joy);
        int  GetNumJoystickHats(int joy);
        bool IsJoystickButtonDown(int joy, int button);
        int  GetJoystickAxis(int joy, int axis);
        int  GetJoystickHat(int joy, int hat);

        /* Joystick Force Feedback */

        bool IsJoystickHaptic(int joy);
        int  CreateJoystickForce(int joy, int strength, int duration);
        bool ApplyJoystickForce(int joy, int force, int times = 1);
        bool StopJoystickForce(int joy, int force);
        bool StopAllJoystickForces(int joy);
        void DestroyJoystickForce(int joy, int force);

        namespace internal {

            bool InitInput(const Log& log);
            void DeinitInput();
            void UpdateInput();

        } // namespace internal
    } // namespace input
} // namespace sphere


#endif
