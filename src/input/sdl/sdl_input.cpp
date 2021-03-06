#include <cassert>
#include <vector>
#include <SDL.h>
#include <SDL_Haptic.h>
#include "../../Log.hpp"
#include "../input.hpp"


//-----------------------------------------------------------------
struct Joystick {
    SDL_Joystick* joystick;
    SDL_Haptic* haptic;
};

//-----------------------------------------------------------------
// globals
std::vector<Joystick> g_Joysticks;
Uint8* g_KeyboardState = 0;

//-----------------------------------------------------------------
static int key_to_sdl_scancode[NUM_KEYS] = {
    SDL_SCANCODE_A,
    SDL_SCANCODE_B,
    SDL_SCANCODE_C,
    SDL_SCANCODE_D,
    SDL_SCANCODE_E,
    SDL_SCANCODE_F,
    SDL_SCANCODE_G,
    SDL_SCANCODE_H,
    SDL_SCANCODE_I,
    SDL_SCANCODE_J,
    SDL_SCANCODE_K,
    SDL_SCANCODE_L,
    SDL_SCANCODE_M,
    SDL_SCANCODE_N,
    SDL_SCANCODE_O,
    SDL_SCANCODE_P,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_R,
    SDL_SCANCODE_S,
    SDL_SCANCODE_T,
    SDL_SCANCODE_U,
    SDL_SCANCODE_V,
    SDL_SCANCODE_W,
    SDL_SCANCODE_X,
    SDL_SCANCODE_Y,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_0,
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_4,
    SDL_SCANCODE_5,
    SDL_SCANCODE_6,
    SDL_SCANCODE_7,
    SDL_SCANCODE_8,
    SDL_SCANCODE_9,
    SDL_SCANCODE_F1,
    SDL_SCANCODE_F2,
    SDL_SCANCODE_F3,
    SDL_SCANCODE_F4,
    SDL_SCANCODE_F5,
    SDL_SCANCODE_F6,
    SDL_SCANCODE_F7,
    SDL_SCANCODE_F8,
    SDL_SCANCODE_F9,
    SDL_SCANCODE_F10,
    SDL_SCANCODE_F11,
    SDL_SCANCODE_F12,
    SDL_SCANCODE_TAB,
    SDL_SCANCODE_BACKSPACE,
    SDL_SCANCODE_RETURN,
    SDL_SCANCODE_ESCAPE,
    SDL_SCANCODE_SPACE,
    SDL_SCANCODE_PAGEUP,
    SDL_SCANCODE_PAGEDOWN,
    SDL_SCANCODE_HOME,
    SDL_SCANCODE_END,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_UP,
    SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_INSERT,
    SDL_SCANCODE_DELETE,
    SDL_SCANCODE_EQUALS, // KEY_PLUS
    SDL_SCANCODE_COMMA,
    SDL_SCANCODE_MINUS,
    SDL_SCANCODE_PERIOD,
    SDL_SCANCODE_CAPSLOCK,
    SDL_SCANCODE_NUMLOCKCLEAR,
    SDL_SCANCODE_LSHIFT,
    SDL_SCANCODE_RSHIFT,
    SDL_SCANCODE_LCTRL,
    SDL_SCANCODE_RCTRL,
    SDL_SCANCODE_LALT,
    SDL_SCANCODE_RALT,
    SDL_SCANCODE_KP_0,
    SDL_SCANCODE_KP_1,
    SDL_SCANCODE_KP_2,
    SDL_SCANCODE_KP_3,
    SDL_SCANCODE_KP_4,
    SDL_SCANCODE_KP_5,
    SDL_SCANCODE_KP_6,
    SDL_SCANCODE_KP_7,
    SDL_SCANCODE_KP_8,
    SDL_SCANCODE_KP_9,
    SDL_SCANCODE_KP_PLUS,
    SDL_SCANCODE_KP_MINUS,
    SDL_SCANCODE_KP_MULTIPLY,
    SDL_SCANCODE_KP_DIVIDE,
};

    SDL_SCANCODE_SLASH,
    SDL_SCANCODE_PRINTSCREEN,
    SDL_SCANCODE_SCROLLLOCK,
    SDL_SCANCODE_PAUSE,
    SDL_SCANCODE_KP_ENTER,
    SDL_SCANCODE_KP_PERIOD,
    SDL_SCANCODE_NONUSBACKSLASH,
    SDL_SCANCODE_KP_EQUALS,
    SDL_SCANCODE_KP_COMMA,
    SDL_SCANCODE_CANCEL,
    SDL_SCANCODE_CLEAR,

//-----------------------------------------------------------------
static int sdl_scancode_to_key[512] = { // SDL_NUM_SCANCODES == 512
    -1,
    -1,
    -1,
    -1,
    KEY_A,
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
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,
    KEY_MINUS,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    KEY_COMMA,
    KEY_PERIOD,
    -1,
    KEY_CAPSLOCK,
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
    -1,
    -1,
    -1,
    KEY_INSERT,
    KEY_HOME,
    KEY_PAGEUP,
    KEY_DELETE,
    KEY_END,
    KEY_PAGEDOWN,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_NUMLOCK,
    KEY_DIVIDE,
    KEY_MULTIPLY,
    KEY_MINUS,
    KEY_PLUS,
    -1,
    KEY_NUMPAD_1,
    KEY_NUMPAD_2,
    KEY_NUMPAD_3,
    KEY_NUMPAD_4,
    KEY_NUMPAD_5,
    KEY_NUMPAD_6,
    KEY_NUMPAD_7,
    KEY_NUMPAD_8,
    KEY_NUMPAD_9,
    KEY_NUMPAD_0,
    KEY_PERIOD,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    KEY_LCTRL,
    KEY_LSHIFT,
    KEY_LALT,
    -1,
    KEY_RCTRL,
    KEY_RSHIFT,
    KEY_RALT,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
};

namespace sphere {
    namespace input {

        //-----------------------------------------------------------------
        void UpdateInput()
        {
            SDL_PumpEvents(); // updates the event queue and internal input device state of SDL
        }

        //-----------------------------------------------------------------
        bool HasAnyEvent()
        {
            return (SDL_HasEvent(SDL_QUIT) ||
                    SDL_HasEvents(SDL_KEYDOWN, SDL_KEYUP) ||
                    SDL_HasEvents(SDL_MOUSEMOTION, SDL_MOUSEWHEEL) ||
                    SDL_HasEvents(SDL_JOYAXISMOTION, SDL_JOYBUTTONUP));
        }

        //-----------------------------------------------------------------
        bool HasEvent(int event)
        {
            switch (event) {
            case Event::KEY_DOWN:           return SDL_HasEvent(SDL_KEYDOWN)         == SDL_TRUE;
            case Event::KEY_UP:             return SDL_HasEvent(SDL_KEYUP)           == SDL_TRUE;
            case Event::MOUSE_BUTTON_DOWN:  return SDL_HasEvent(SDL_MOUSEBUTTONDOWN) == SDL_TRUE;
            case Event::MOUSE_BUTTON_UP:    return SDL_HasEvent(SDL_MOUSEBUTTONUP)   == SDL_TRUE;
            case Event::MOUSE_MOTION:       return SDL_HasEvent(SDL_MOUSEMOTION)     == SDL_TRUE;
            case Event::MOUSE_WHEEL_MOTION: return SDL_HasEvent(SDL_MOUSEWHEEL)      == SDL_TRUE;
            case Event::JOY_BUTTON_DOWN:    return SDL_HasEvent(SDL_JOYBUTTONDOWN)   == SDL_TRUE;
            case Event::JOY_BUTTON_UP:      return SDL_HasEvent(SDL_JOYBUTTONUP)     == SDL_TRUE;
            case Event::JOY_AXIS_MOTION:    return SDL_HasEvent(SDL_JOYAXISMOTION)   == SDL_TRUE;
            case Event::JOY_HAT_MOTION:     return SDL_HasEvent(SDL_JOYHATMOTION)    == SDL_TRUE;
            case Event::QUIT:               return SDL_HasEvent(SDL_QUIT)            == SDL_TRUE;
            default:
                return false;
            }
        }

        //-----------------------------------------------------------------
        bool GetEvent(Event& out)
        {
            SDL_Event sdl_event;

        fetch_event:
            if (SDL_PollEvent(&sdl_event) == 1) {
                switch (sdl_event.type) {
                case SDL_KEYDOWN:
                    out.type = Event::KEY_DOWN;
                    out.key.key = sdl_scancode_to_key[sdl_event.key.keysym.scancode];
                    break;

                case SDL_KEYUP:
                    out.type = Event::KEY_UP;
                    out.key.key = sdl_scancode_to_key[sdl_event.key.keysym.scancode];
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    out.type = Event::MOUSE_BUTTON_DOWN;
                    out.mbutton.button = sdl_event.button.button;
                    break;

                case SDL_MOUSEBUTTONUP:
                    out.type = Event::MOUSE_BUTTON_UP;
                    out.mbutton.button = sdl_event.button.button;
                    break;

                case SDL_MOUSEMOTION:
                    out.type = Event::MOUSE_MOTION;
                    out.mmotion.dx = sdl_event.motion.xrel;
                    out.mmotion.dy = sdl_event.motion.yrel;
                    break;

                case SDL_MOUSEWHEEL:
                    out.type = Event::MOUSE_WHEEL_MOTION;
                    out.mwheel.dx = sdl_event.wheel.x;
                    out.mwheel.dy = sdl_event.wheel.y;
                    break;

                case SDL_JOYBUTTONDOWN:
                    out.type = Event::JOY_BUTTON_DOWN;
                    out.jbutton.joy    = sdl_event.jbutton.which;
                    out.jbutton.button = sdl_event.jbutton.button;
                    break;

                case SDL_JOYBUTTONUP:
                    out.type = Event::JOY_BUTTON_UP;
                    out.jbutton.joy    = sdl_event.jbutton.which;
                    out.jbutton.button = sdl_event.jbutton.button;
                    break;

                case SDL_JOYAXISMOTION:
                    out.type = Event::JOY_AXIS_MOTION;
                    out.jaxis.joy   = sdl_event.jaxis.which;
                    out.jaxis.axis  = sdl_event.jaxis.axis;
                    out.jaxis.value = sdl_event.jaxis.value;
                    break;

                case SDL_JOYHATMOTION:
                    out.type = Event::JOY_HAT_MOTION;
                    out.jhat.joy   = sdl_event.jhat.which;
                    out.jhat.hat   = sdl_event.jhat.hat;
                    out.jhat.value = sdl_event.jhat.value;
                    break;

                case SDL_QUIT:
                    out.type = Event::APP_QUIT;
                    break;

                default:
                    goto fetch_event; // ignore any other events
                }
                return true;
            }
            return false; // no events pending
        }

        //-----------------------------------------------------------------
        void ClearEvents()
        {
            SDL_Event sdl_event;
            while (SDL_PollEvent(&sdl_event) == 1) { }
        }

        //-----------------------------------------------------------------
        bool IsKeyDown(int key)
        {
            if (key >= 0 && key < NUM_KEYS) {
                return g_KeyboardState[key_to_sdl_scancode[key]] != 0;
            }
            return false;
        }

        //-----------------------------------------------------------------
        bool IsCapsLockOn()
        {
            return (SDL_GetModState() & KMOD_CAPS) != 0;
        }

        //-----------------------------------------------------------------
        int GetMouseX()
        {
            int x;
            SDL_GetMouseState(&x, 0);
            return x;
        }

        //-----------------------------------------------------------------
        int GetMouseY()
        {
            int y;
            SDL_GetMouseState(0, &y);
            return y;
        }

        //-----------------------------------------------------------------
        bool IsMouseButtonDown(int button)
        {
            switch (button) {
            case MOUSE_BUTTON_LEFT:   return (SDL_GetMouseState(0, 0) & SDL_BUTTON_LMASK)  != 0;
            case MOUSE_BUTTON_MIDDLE: return (SDL_GetMouseState(0, 0) & SDL_BUTTON_MMASK)  != 0;
            case MOUSE_BUTTON_RIGHT:  return (SDL_GetMouseState(0, 0) & SDL_BUTTON_RMASK)  != 0;
            case MOUSE_BUTTON_X1:     return (SDL_GetMouseState(0, 0) & SDL_BUTTON_X1MASK) != 0;
            case MOUSE_BUTTON_X2:     return (SDL_GetMouseState(0, 0) & SDL_BUTTON_X2MASK) != 0;
            default:
                return false;
            }
        }

        //-----------------------------------------------------------------
        int GetNumJoysticks()
        {
            return (int)g_Joysticks.size();
        }

        //-----------------------------------------------------------------
        const char* GetJoystickName(int joy)
        {
            return SDL_JoystickName(joy);
        }

        //-----------------------------------------------------------------
        int GetNumJoystickButtons(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return SDL_JoystickNumButtons(g_Joysticks[joy].joystick);
            }
            return 0;
        }

        //-----------------------------------------------------------------
        int GetNumJoystickAxes(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return SDL_JoystickNumAxes(g_Joysticks[joy].joystick);
            }
            return 0;
        }

        //-----------------------------------------------------------------
        int GetNumJoystickHats(int joy)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return SDL_JoystickNumHats(g_Joysticks[joy].joystick);
            }
            return 0;
        }

        //-----------------------------------------------------------------
        bool IsJoystickButtonDown(int joy, int button)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return SDL_JoystickGetButton(g_Joysticks[joy].joystick, button) == 1;
            }
            return false;
        }

        //-----------------------------------------------------------------
        int GetJoystickAxis(int joy, int axis)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                return (int)(SDL_JoystickGetAxis(g_Joysticks[joy].joystick, axis) / 327.67f);
            }
            return 0;
        }

        //-----------------------------------------------------------------
        int GetJoystickHat(int joy, int hat)
        {
            if (joy >= 0 && joy < (int)g_Joysticks.size()) {
                switch (SDL_JoystickGetHat(g_Joysticks[joy].joystick, hat)) {
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
                // initialize SDL (if not already initialized)
                if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0) {
                    log.error() << "Failed initializing SDL: " << SDL_GetError();
                    return false;
                }

                // ensure SDL will be properly deinitialized
                atexit(SDL_Quit);

                // open all available joysticks
                int num_joysticks = SDL_NumJoysticks();
                if (num_joysticks > 0) {
                    for (int i = 0; i < num_joysticks; ++i) {
                        SDL_Joystick* joystick = SDL_JoystickOpen(i);
                        if (!joystick) {
                            log.error() << "Failed opening joystick " << i << " '" << SDL_JoystickName(i) << "': " << SDL_GetError();
                            return false;
                        }
                        SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joystick); // open haptic if available
                        Joystick joy = {joystick, haptic};
                        g_Joysticks.push_back(joy);
                    }
                }

                // get a pointer to SDL's keyboard state
                g_KeyboardState = SDL_GetKeyboardState(0);

                return true;
            }

            //-----------------------------------------------------------------
            void DeinitInput()
            {
                g_KeyboardState = 0;

                // close joysticks and their haptic devices
                for (size_t i = 0; i < g_Joysticks.size(); ++i) {
                    if (g_Joysticks[i].haptic) {
                        SDL_HapticClose(g_Joysticks[i].haptic);
                    }
                    SDL_JoystickClose(g_Joysticks[i].joystick);
                }

                // quit subsystems
                SDL_QuitSubSystem(SDL_INIT_HAPTIC);
                SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
            }

        } // namespace internal
    } // namespace input
} // namespace sphere
