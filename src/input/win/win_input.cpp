#include <cassert>
#include <vector>
#include <deque>
#include <cassert>
#include <vector>
#include <windows.h>
#include <SDL.h>
#include <SDL_Haptic.h>
#include "../input.hpp"

#define PRESSED  1
#define RELEASED 0


namespace sphere {
    namespace input {

        //-----------------------------------------------------------------
        struct Joystick {
            SDL_Joystick* joystick;
            SDL_Haptic* haptic;
            std::string name;
            std::vector<int> buttons;
            std::vector<int> axes;
            std::vector<int> hats;
        };

        //-----------------------------------------------------------------
        // globals
        std::vector<Joystick> g_Joysticks;

        //-----------------------------------------------------------------
        int GetNumJoysticks()
        {
            return (int)g_Joysticks.size();
        }

        //-----------------------------------------------------------------
        const char* GetJoystickName(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return g_Joysticks[joy].name.c_str();
            }
            return 0;
        }

        //-----------------------------------------------------------------
        int GetNumJoystickButtons(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return (int)g_Joysticks[joy].buttons.size();
            }
            return 0;
        }

        //-----------------------------------------------------------------
        int GetNumJoystickAxes(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return (int)g_Joysticks[joy].axes.size();
            }
            return 0;
        }

        //-----------------------------------------------------------------
        int GetNumJoystickHats(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return (int)g_Joysticks[joy].hats.size();
            }
            return 0;
        }

        //-----------------------------------------------------------------
        bool IsJoystickButtonDown(int joy, int button)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                if (button >= 0 && button < (int)g_Joysticks[joy].buttons.size()) {
                    return g_Joysticks[joy].buttons[button] == PRESSED;
                }
            }
            return false;
        }

        //-----------------------------------------------------------------
        int GetJoystickAxis(int joy, int axis)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                if (axis >= 0 && axis < (int)g_Joysticks[joy].axes.size()) {
                    return g_Joysticks[joy].axes[axis];
                }
            }
            return 0;
        }

        //-----------------------------------------------------------------
        int GetJoystickHat(int joy, int hat)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                if (hat >= 0 && hat < (int)g_Joysticks[joy].hats.size()) {
                    return g_Joysticks[joy].hats[hat];
                }
            }
            return JOY_HAT_CENTERED;
        }

        //-----------------------------------------------------------------
        bool IsJoystickHaptic(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return g_Joysticks[joy].haptic != 0;
            }
            return false;
        }

        //-----------------------------------------------------------------
        int CreateJoystickForce(int joy, int strength, int duration)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
                SDL_HapticEffect effect;
                memset(&effect, 0, sizeof(SDL_HapticEffect));
                effect.constant.type             = SDL_HAPTIC_CONSTANT;
                effect.constant.length           = (duration < 0 ? SDL_HAPTIC_INFINITY : (Uint32)duration);
                effect.constant.level            = (Sint16)(strength * 327.67);
                effect.constant.direction.type   = SDL_HAPTIC_CARTESIAN;
                int effect_id = SDL_HapticNewEffect(g_Joysticks[joy].haptic, &effect);
                if (effect_id >= 0) {
                    return effect_id;
                }
            }
            return -1;
        }

        //-----------------------------------------------------------------
        bool ApplyJoystickForce(int joy, int force, int times)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
                Uint32 iters = (times < 0 ? SDL_HAPTIC_INFINITY : times);
                return SDL_HapticRunEffect(g_Joysticks[joy].haptic, force, iters) == 0;
            }
            return false;
        }

        //-----------------------------------------------------------------
        bool StopJoystickForce(int joy, int force)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
                return SDL_HapticStopEffect(g_Joysticks[joy].haptic, force) == 0;
            }
            return false;
        }

        //-----------------------------------------------------------------
        bool StopAllJoystickForces(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
                return SDL_HapticStopAll(g_Joysticks[joy].haptic) == 0;
            }
            return false;
        }

        //-----------------------------------------------------------------
        void DestroyJoystickForce(int joy, int force)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
                SDL_HapticDestroyEffect(g_Joysticks[joy].haptic, force);
            }
        }

        namespace internal {

            //-----------------------------------------------------------------
            bool InitInput(const Log& log)
            {
                // initialize SDL
                if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0) {
                    log.error() << "Could not initialize SDL: " << SDL_GetError();
                    return false;
                }

                // ensure SDL will be properly deinitialized
                atexit(SDL_Quit);

                // disable joystick events, we are not using SDL's event system
                SDL_JoystickEventState(SDL_IGNORE);

                // open all available joysticks
                int num_joysticks = SDL_NumJoysticks();
                if (num_joysticks > 0) {
                    for (int i = 0; i < num_joysticks; ++i) {
                        std::string name = SDL_JoystickName(i);

                        // open joystick
                        SDL_Joystick* joystick = SDL_JoystickOpen(i);
                        if (!joystick) {
                            log.error() << "Could not open joystick " << i << ": " << SDL_GetError();
                            return false;
                        }

                        // get capabilities
                        int num_buttons = SDL_JoystickNumButtons(joystick);
                        int num_axes    = SDL_JoystickNumAxes(joystick);
                        int num_hats    = SDL_JoystickNumHats(joystick);

                        // open haptic, if available
                        SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joystick);

                        log.info() << "Joystick " \
                                   << i \
                                   << " (" \
                                   << name \
                                   << "): " \
                                   << num_buttons \
                                   << " buttons, " \
                                   << num_axes \
                                   << " axes, " \
                                   << num_hats \
                                   << " POV hats, " \
                                   << (haptic ? "supports" : "does not support") \
                                   << " force feedback";

                        // create joystick
                        Joystick j;
                        j.joystick = joystick;
                        j.haptic = haptic;
                        j.name = name;

                        // initialize buttons
                        j.buttons.resize(num_buttons);
                        for (size_t button = 0; button < j.buttons.size(); button++) {
                            j.buttons[button] = RELEASED;
                        }

                        // initialize axes
                        j.axes.resize(num_axes);
                        for (size_t axis = 0; axis < j.axes.size(); axis++) {
                            j.axes[axis] = 0;
                        }

                        // initialize hats
                        j.hats.resize(num_hats);
                        for (size_t hat = 0; hat < j.hats.size(); hat++) {
                            j.hats[hat] = JOY_HAT_CENTERED;
                        }

                        // store joystick
                        g_Joysticks.push_back(j);
                    }
                } else {
                    log.info() << "No joysticks available";
                }

                return true;
            }

            //-----------------------------------------------------------------
            void DeinitInput()
            {
                // close joysticks and their haptic devices
                for (size_t i = 0; i < g_Joysticks.size(); ++i) {
                    if (g_Joysticks[i].haptic) {
                        SDL_HapticClose(g_Joysticks[i].haptic);
                    }
                    SDL_JoystickClose(g_Joysticks[i].joystick);
                }
                g_Joysticks.clear();

                // quit SDL subsystems
                SDL_QuitSubSystem(SDL_INIT_HAPTIC);
                SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
            }

            //-----------------------------------------------------------------
            void UpdateInput()
            {
                // update joysticks
                SDL_JoystickUpdate();
                for (size_t joy = 0; joy < g_Joysticks.size(); joy++) {
                    // update buttons
                    for (size_t button = 0; button < g_Joysticks[joy].buttons.size(); button++) {
                        g_Joysticks[joy].buttons[button] = SDL_JoystickGetButton(g_Joysticks[joy].joystick, button);
                    }

                    // update axes
                    for (size_t axis = 0; axis < g_Joysticks[joy].axes.size(); axis++) {
                        g_Joysticks[joy].axes[axis] = (int)(SDL_JoystickGetAxis(g_Joysticks[joy].joystick, axis) / 327.67f);
                    }

                    // update hats
                    for (size_t hat = 0; hat < g_Joysticks[joy].hats.size(); hat++) {
                        int value = JOY_HAT_CENTERED;
                        switch (SDL_JoystickGetHat(g_Joysticks[joy].joystick, hat)) {
                        case SDL_HAT_UP:        value = JOY_HAT_UP;        break;
                        case SDL_HAT_RIGHT:     value = JOY_HAT_RIGHT;     break;
                        case SDL_HAT_DOWN:      value = JOY_HAT_DOWN;      break;
                        case SDL_HAT_LEFT:      value = JOY_HAT_LEFT;      break;
                        case SDL_HAT_RIGHTUP:   value = JOY_HAT_RIGHTUP;   break;
                        case SDL_HAT_RIGHTDOWN: value = JOY_HAT_RIGHTDOWN; break;
                        case SDL_HAT_LEFTUP:    value = JOY_HAT_LEFTUP;    break;
                        case SDL_HAT_LEFTDOWN:  value = JOY_HAT_LEFTDOWN;  break;
                        };
                        g_Joysticks[joy].hats[hat] = value;
                    }
                }
            }

        } // namespace internal
    } // namespace input
} // namespace sphere
