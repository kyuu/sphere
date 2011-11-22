#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include "../common/types.hpp"


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

// force feedback
bool HasJoystickForceFeedback(int joy);
int  UploadJoystickForceEffect(int joy, int direction, int duration, int startLevel, int endLevel);
bool PlayJoystickForceEffect(int joy, int effect, int times = 1);
bool StopJoystickForceEffect(int joy, int effect);
bool StopAllJoystickForceEffects(int joy);
void RemoveJoystickForceEffect(int joy, int effect);


#endif
