#include <SDL.h>
#include "../joystick.hpp"


//-----------------------------------------------------------------
static bool setup_sdl_haptic_direction(const SDL_HapticDirection& sdl_haptic_direction, const ForceDirection& force_direction)
{
    switch (force_direction.type) {
    case ForceEffect::Direction::CARTESIAN:
        sdl_haptic_direction.type   = SDL_HAPTIC_CARTESIAN;
        sdl_haptic_direction.dir[0] = (Sint32)force_direction.dir[0];
        sdl_haptic_direction.dir[1] = (Sint32)force_direction.dir[1];
        sdl_haptic_direction.dir[2] = (Sint32)force_direction.dir[2];
        break;
    case ForceEffect::Direction::POLAR:
        sdl_haptic_direction.type   = SDL_HAPTIC_POLAR;
        sdl_haptic_direction.dir[0] = (Sint32)force_direction.dir[0];
        break;
    default:
        return false;
    }
    return true;
}

//-----------------------------------------------------------------
static bool setup_sdl_haptic_effect(const SDL_HapticEffect& sdl_haptic_effect, const ForceEffect& force_effect)
{
    memset(&sdl_effect, 0, sizeof(SDL_HapticEffect));

    switch (force_effect.type) {
    case ForceEffect::CONSTANT:
        sdl_haptic_effect.constant.type          = SDL_HAPTIC_CONSTANT;
        sdl_haptic_effect.constant.length        = (force_effect.constant.length == ForceEffect::INFINITY ? SDL_HAPTIC_INFINITY : (Uint32)force_effect.constant.length);
        sdl_haptic_effect.constant.delay         = (Uint16)force_effect.delay;
        sdl_haptic_effect.constant.attack_length = (Uint16)force_effect.constant.attack_length;
        sdl_haptic_effect.constant.attack_level  = (Uint16)force_effect.constant.attack_level;
        sdl_haptic_effect.constant.fade_length   = (Uint16)force_effect.constant.fade_length;
        sdl_haptic_effect.constant.fade_level    = (Uint16)force_effect.constant.fade_level;
        sdl_haptic_effect.constant.level         = (Sint16)force_effect.constant.level;
        if (!setup_sdl_haptic_direction(sdl_haptic_effect.constant.direction, force_effect.constant.direction)) {
            return false;
        }
        break;

    case ForceEffect::RAMP:
        sdl_haptic_effect.type               = SDL_HAPTIC_RAMP;
        sdl_haptic_effect.ramp.length        = (force_effect.ramp.length == ForceEffect::INFINITY ? SDL_HAPTIC_INFINITY : (Uint32)force_effect.ramp.length);
        sdl_haptic_effect.ramp.delay         = (Uint16)force_effect.delay;
        sdl_haptic_effect.ramp.attack_length = (Uint16)force_effect.ramp.attack_length;
        sdl_haptic_effect.ramp.attack_level  = (Uint16)force_effect.ramp.attack_level;
        sdl_haptic_effect.ramp.fade_length   = (Uint16)force_effect.ramp.fade_length;
        sdl_haptic_effect.ramp.fade_level    = (Uint16)force_effect.ramp.fade_level;
        sdl_haptic_effect.ramp.start         = (Sint16)force_effect.ramp.start;
        sdl_haptic_effect.ramp.end           = (Sint16)force_effect.ramp.end;
        if (!setup_sdl_haptic_direction(sdl_haptic_effect.ramp.direction, force_effect.ramp.direction)) {
            return false;
        }
        break;

    case ForceEffect::SINE:
        sdl_haptic_effect.type = SDL_HAPTIC_SINE;
        goto setup_periodic_effect;

    case ForceEffect::SQUARE:
        sdl_haptic_effect.type = SDL_HAPTIC_SQUARE;
        goto setup_periodic_effect;

    case ForceEffect::TRIANGLE:
        sdl_haptic_effect.type = SDL_HAPTIC_TRIANGLE;
        goto setup_periodic_effect;

    case ForceEffect::SAWTOOTHUP:
        sdl_haptic_effect.type = SDL_HAPTIC_SAWTOOTHUP;
        goto setup_periodic_effect;

    case ForceEffect::SAWTOOTHDOWN:
        sdl_haptic_effect.type = SDL_HAPTIC_SAWTOOTHDOWN;

setup_periodic_effect:

        sdl_haptic_effect.periodic.length        = (force_effect.periodic.length == ForceEffect::INFINITY ? SDL_HAPTIC_INFINITY : (Uint32)force_effect.periodic.length);
        sdl_haptic_effect.periodic.delay         = (Uint16)force_effect.delay;
        sdl_haptic_effect.periodic.attack_length = (Uint16)force_effect.periodic.attack_length;
        sdl_haptic_effect.periodic.attack_level  = (Uint16)force_effect.periodic.attack_level;
        sdl_haptic_effect.periodic.fade_length   = (Uint16)force_effect.periodic.fade_length;
        sdl_haptic_effect.periodic.fade_level    = (Uint16)force_effect.periodic.fade_level;
        sdl_haptic_effect.periodic.period        = (Uint16)force_effect.periodic.period;
        sdl_haptic_effect.periodic.magnitude     = (Sint16)force_effect.periodic.magnitude;
        sdl_haptic_effect.periodic.offset        = (Sint16)force_effect.periodic.offset;
        sdl_haptic_effect.periodic.phase         = (Uint16)force_effect.periodic.phase;
        if (!setup_sdl_haptic_direction(sdl_haptic_effect.periodic.direction, force_effect.periodic.direction)) {
            return false;
        }
        break;

    default:
        return false;
    }

    return true;
}

namespace input {

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
    bool IsJoystickForceEffectSupported(int joy, int effect)
    {
        if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
            switch (effect) {
            case ForceEffect::CONSTANT:     return (SDL_HapticQuery(g_joysticks[joy].haptic) & SDL_HAPTIC_CONSTANT);
            case ForceEffect::RAMP:         return (SDL_HapticQuery(g_joysticks[joy].haptic) & SDL_HAPTIC_RAMP);
            case ForceEffect::SINE:         return (SDL_HapticQuery(g_joysticks[joy].haptic) & SDL_HAPTIC_SINE);
            case ForceEffect::SQUARE:       return (SDL_HapticQuery(g_joysticks[joy].haptic) & SDL_HAPTIC_SQUARE);
            case ForceEffect::TRIANGLE:     return (SDL_HapticQuery(g_joysticks[joy].haptic) & SDL_HAPTIC_TRIANGLE);
            case ForceEffect::SAWTOOTHUP:   return (SDL_HapticQuery(g_joysticks[joy].haptic) & SDL_HAPTIC_SAWTOOTHUP);
            case ForceEffect::SAWTOOTHDOWN: return (SDL_HapticQuery(g_joysticks[joy].haptic) & SDL_HAPTIC_SAWTOOTHDOWN);
            default:
                return false;
            }
        }
        return false;
    }

    //-----------------------------------------------------------------
    int UploadJoystickForceEffect(int joy, const ForceEffect& effect)
    {
        if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
            SDL_HapticEffect sdl_haptic_effect;
            if (setup_sdl_haptic_effect(sdl_haptic_effect, effect)) {
                int effect_id = SDL_HapticNewEffect(g_joysticks[joy].haptic, &sdl_haptic_effect);
                if (effect_id >= 0) {
                    return effect_id;
                }
            }
        }
        return -1;
    }

    //-----------------------------------------------------------------
    bool PlayJoystickForceEffect(int joy, int effect, int times)
    {
        if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
            Uint32 iters = (times == ForceEffect::INFINITY ? SDL_HAPTIC_INFINITY : times)
            return SDL_HapticRunEffect(g_joysticks[joy].haptic, effect, iters) == 0;
        }
        return false;
    }

    //-----------------------------------------------------------------
    int UpdateJoystickForceEffect(int joy, int effect, const ForceEffect& new_data)
    {
        if (joy >= 0 && joy < (int)g_joysticks.size() && g_joysticks[joy].haptic) {
            SDL_HapticEffect sdl_haptic_effect;
            if (setup_sdl_haptic_effect(sdl_haptic_effect, new_data)) {
                int effect_id = SDL_HapticUpdateEffect(g_joysticks[joy].haptic, effect, &sdl_haptic_effect);
                if (effect_id >= 0) {
                    return effect_id;
                }
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

} // namespace input
