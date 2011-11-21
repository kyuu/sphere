#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include "ForceEffect.hpp"


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
struct ForceEffect {
    i32 direction;   // direction of the force in degrees (0 - 360)
    i32 duration;    // duration of the effect
    i32 start;       // start strength
    i32 end;         // end strength
};

bool HasJoystickForceFeedback(int joy);
int  UploadJoystickForceEffect(int joy, const ForceEffect& effect);
bool PlayJoystickForceEffect(int joy, int effect, int times = 1);
int  UpdateJoystickForceEffect(int joy, int effect, const ForceEffect& new_data);
bool StopJoystickForceEffect(int joy, int effect);
bool StopAllJoystickForceEffects(int joy);
void RemoveJoystickForceEffect(int joy, int effect);


#endif
