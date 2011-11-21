#include <SDL.h>
#include "../joystick.hpp"


//-----------------------------------------------------------------
static void setup_sdl_haptic_effect(const SDL_HapticEffect& sdl_haptic_effect, const ForceEffect& force_effect)
{
    memset(&sdl_effect, 0, sizeof(SDL_HapticEffect));
    if (force_effect.start == force_effect.end) {
        sdl_haptic_effect.constant.type             = SDL_HAPTIC_CONSTANT;
        sdl_haptic_effect.constant.length           = (force_effect.duration < 0 ? SDL_HAPTIC_INFINITY : (Uint32)force_effect.duration);
        sdl_haptic_effect.constant.level            = (Sint16)force_effect.start;
        sdl_haptic_effect.constant.direction.type   = SDL_HAPTIC_POLAR;
        sdl_haptic_effect.constant.direction.dir[0] = (Sint32)force_effect.direction;
    } else {
        sdl_haptic_effect.ramp.type             = SDL_HAPTIC_RAMP;
        sdl_haptic_effect.ramp.length           = (force_effect.duration < 0 ? SDL_HAPTIC_INFINITY : (Uint32)force_effect.duration);
        sdl_haptic_effect.ramp.start            = (Sint16)force_effect.start;
        sdl_haptic_effect.ramp.end              = (Sint16)force_effect.end;
        sdl_haptic_effect.ramp.direction.type   = SDL_HAPTIC_POLAR;
        sdl_haptic_effect.ramp.direction.dir[0] = (Sint32)force_effect.direction;
    }
}

//-----------------------------------------------------------------
int GetNumJoysticks()
{
    return (int)g_joysticks.size();
}

//-----------------------------------------------------------------
const char* GetJoystickName(int joy)
{
    return SDL_JoystickName(joy);
}

//-----------------------------------------------------------------
int GetNumJoystickButtons(int joy)
{
    if (joy >= 0 && joy < (int)g_joysticks.size()) {
        return SDL_JoystickNumButtons(g_joysticks[joy].joystick);
    }
    return 0;
}

//-----------------------------------------------------------------
int GetNumJoystickAxes(int joy)
{
    if (joy >= 0 && joy < (int)g_joysticks.size()) {
        return SDL_JoystickNumAxes(joy);
    }
    return 0;
}

//-----------------------------------------------------------------
int GetNumJoystickHats(int joy)
{
    if (joy >= 0 && joy < (int)g_joysticks.size()) {
        return SDL_JoystickNumHats(joy);
    }
    return 0;
}

//-----------------------------------------------------------------
bool IsJoystickButtonDown(int joy, int button)
{
    if (joy >= 0 && joy < (int)g_joysticks.size()) {
        return SDL_JoystickGetButton(joy, button) == 1;
    }
    return false;
}

//-----------------------------------------------------------------
int GetJoystickAxis(int joy, int axis)
{
    if (joy >= 0 && joy < (int)g_joysticks.size()) {
        return (int)SDL_JoystickGetAxis(joy, axis);
    }
    return 0;
}

//-----------------------------------------------------------------
int GetJoystickHat(int joy, int hat)
{
    if (joy >= 0 && joy < (int)g_joysticks.size()) {
        switch (SDL_JoystickGetHat(joy, hat)) {
        case SDL_HAT_CENTERED:  return JOY_HAT_CENTERED;
        case SDL_HAT_UP:        return JOY_HAT_UP;
        case SDL_HAT_RIGHT:     return JOY_HAT_RIGHT;
        case SDL_HAT_DOWN:      return JOY_HAT_DOWN;
        case SDL_HAT_LEFT:      return JOY_HAT_LEFT;
        case SDL_HAT_RIGHTUP:   return JOY_HAT_RIGHTUP;
        case SDL_HAT_RIGHTDOWN: return JOY_HAT_RIGHTDOWN;
        case SDL_HAT_LEFTUP:    return JOY_HAT_LEFTUP;
        case SDL_HAT_LEFTDOWN:  return JOY_HAT_LEFTDOWN;
        default:                return JOY_HAT_CENTERED;
        };
    }
    return JOY_HAT_CENTERED;
}

//-----------------------------------------------------------------
bool HasJoystickForceFeedback(int joy)
{
    if (joy >= 0 && joy < (int)g_joysticks.size()) {
        return g_joysticks[joy].haptic != 0;
    }
    return false;
}

//-----------------------------------------------------------------
int UploadJoystickForceEffect(int joy, const ForceEffect& effect)
{
    if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
        SDL_HapticEffect sdl_haptic_effect;
        setup_sdl_haptic_effect(sdl_haptic_effect, effect);
        int effect_id = SDL_HapticNewEffect(g_joysticks[joy].haptic, &sdl_haptic_effect);
        if (effect_id >= 0) {
            return effect_id;
        }
    }
    return -1;
}

//-----------------------------------------------------------------
bool PlayJoystickForceEffect(int joy, int effect, int times)
{
    if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
        Uint32 iters = (times < 0 ? SDL_HAPTIC_INFINITY : times)
        return SDL_HapticRunEffect(g_joysticks[joy].haptic, effect, iters) == 0;
    }
    return false;
}

//-----------------------------------------------------------------
int UpdateJoystickForceEffect(int joy, int effect, const ForceEffect& new_data)
{
    if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
        SDL_HapticEffect sdl_haptic_effect;
        setup_sdl_haptic_effect(sdl_haptic_effect, new_data);
        int effect_id = SDL_HapticUpdateEffect(g_joysticks[joy].haptic, effect, &sdl_haptic_effect);
        if (effect_id >= 0) {
            return effect_id;
        }
    }
    return -1;
}

//-----------------------------------------------------------------
bool StopJoystickForceEffect(int joy, int effect)
{
    if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
        return SDL_HapticStopEffect(g_joysticks[joy].haptic, effect) == 0;
    }
    return false;
}

//-----------------------------------------------------------------
bool StopAllJoystickForceEffects(int joy)
{
    if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
        return SDL_HapticStopAll(g_joysticks[joy].haptic) == 0;
    }
    return false;
}

//-----------------------------------------------------------------
void RemoveJoystickForceEffect(int joy, int effect)
{
    if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
        SDL_HapticDestroyEffect(g_joysticks[joy].haptic, effect);
    }
}
