#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_Haptic.h>
#include "../../Log.hpp"
#include "../input.hpp"
#include "../event.hpp"
#include "../keyboard.hpp"
#include "../mouse.hpp"
#include "../joystick.hpp"

//-----------------------------------------------------------------
struct SDLJOY {
    SDL_Joystick* joystick;
    SDL_Haptic* haptic;
};

//-----------------------------------------------------------------
// globals
std::vector<SDLJOY> g_Joysticks;
int g_NumKeys = 0;
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
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_4,
    SDL_SCANCODE_5,
    SDL_SCANCODE_6,
    SDL_SCANCODE_7,
    SDL_SCANCODE_8,
    SDL_SCANCODE_9,
    SDL_SCANCODE_0,
    SDL_SCANCODE_RETURN,
    SDL_SCANCODE_ESCAPE,
    SDL_SCANCODE_BACKSPACE,
    SDL_SCANCODE_TAB,
    SDL_SCANCODE_SPACE,
    SDL_SCANCODE_MINUS,
    SDL_SCANCODE_EQUALS,
    SDL_SCANCODE_LEFTBRACKET,
    SDL_SCANCODE_RIGHTBRACKET,
    SDL_SCANCODE_BACKSLASH,
    SDL_SCANCODE_SEMICOLON,
    SDL_SCANCODE_APOSTROPHE,
    SDL_SCANCODE_GRAVE,
    SDL_SCANCODE_COMMA,
    SDL_SCANCODE_PERIOD,
    SDL_SCANCODE_SLASH,
    SDL_SCANCODE_CAPSLOCK,
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
    SDL_SCANCODE_PRINTSCREEN,
    SDL_SCANCODE_SCROLLLOCK,
    SDL_SCANCODE_PAUSE,
    SDL_SCANCODE_INSERT,
    SDL_SCANCODE_HOME,
    SDL_SCANCODE_PAGEUP,
    SDL_SCANCODE_DELETE,
    SDL_SCANCODE_END,
    SDL_SCANCODE_PAGEDOWN,
    SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_UP,
    SDL_SCANCODE_NUMLOCKCLEAR,
    SDL_SCANCODE_KP_DIVIDE,
    SDL_SCANCODE_KP_MULTIPLY,
    SDL_SCANCODE_KP_MINUS,
    SDL_SCANCODE_KP_PLUS,
    SDL_SCANCODE_KP_ENTER,
    SDL_SCANCODE_KP_1,
    SDL_SCANCODE_KP_2,
    SDL_SCANCODE_KP_3,
    SDL_SCANCODE_KP_4,
    SDL_SCANCODE_KP_5,
    SDL_SCANCODE_KP_6,
    SDL_SCANCODE_KP_7,
    SDL_SCANCODE_KP_8,
    SDL_SCANCODE_KP_9,
    SDL_SCANCODE_KP_0,
    SDL_SCANCODE_KP_PERIOD,
    SDL_SCANCODE_NONUSBACKSLASH,
    SDL_SCANCODE_KP_EQUALS,
    SDL_SCANCODE_KP_COMMA,
    SDL_SCANCODE_CANCEL,
    SDL_SCANCODE_CLEAR,
    SDL_SCANCODE_LCTRL,
    SDL_SCANCODE_LSHIFT,
    SDL_SCANCODE_LALT,
    SDL_SCANCODE_RCTRL,
    SDL_SCANCODE_RSHIFT,
    SDL_SCANCODE_RALT,
};

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
    KEY_RETURN,
    KEY_ESCAPE,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,
    KEY_MINUS,
    KEY_EQUALS,
    KEY_LEFTBRACKET,
    KEY_RIGHTBRACKET,
    KEY_BACKSLASH,
    -1,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,
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
    KEY_PRINTSCREEN,
    KEY_SCROLLLOCK,
    KEY_PAUSE,
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
    KEY_NUMLOCKCLEAR,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_MINUS,
    KEY_KP_PLUS,
    KEY_KP_ENTER,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_0,
    KEY_KP_PERIOD,
    KEY_NONUSBACKSLASH,
    -1,
    -1,
    KEY_KP_EQUALS,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    KEY_KP_COMMA,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    KEY_CANCEL,
    KEY_CLEAR,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
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

//-----------------------------------------------------------------
bool AreEventsPending()
{
    return SDL_PollEvent(0) == 1;
}

//-----------------------------------------------------------------
bool GetEvent(Event& out)
{
    SDL_Event sdl_event;
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
            out.jhat.state = sdl_event.jhat.value;
            break;

        case SDL_QUIT:
            out.type = Event::APP_QUIT;
            break;

        default:
            return false; // ignore any other events
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
        return (int)SDL_JoystickGetAxis(g_Joysticks[joy].joystick, axis);
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
bool HasJoystickForceFeedback(int joy)
{
    if (joy >= 0 && joy < (int)g_Joysticks.size()) {
        return g_Joysticks[joy].haptic != 0;
    }
    return false;
}

//-----------------------------------------------------------------
static void setup_sdl_haptic_effect(SDL_HapticEffect& sdl_haptic_effect, const ForceEffect& force_effect)
{
    memset(&sdl_haptic_effect, 0, sizeof(SDL_HapticEffect));
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
int UploadJoystickForceEffect(int joy, const ForceEffect& effect)
{
    if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
        SDL_HapticEffect sdl_haptic_effect;
        setup_sdl_haptic_effect(sdl_haptic_effect, effect);
        int effect_id = SDL_HapticNewEffect(g_Joysticks[joy].haptic, &sdl_haptic_effect);
        if (effect_id >= 0) {
            return effect_id;
        }
    }
    return -1;
}

//-----------------------------------------------------------------
bool PlayJoystickForceEffect(int joy, int effect, int times)
{
    if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
        Uint32 iters = (times < 0 ? SDL_HAPTIC_INFINITY : times);
        return SDL_HapticRunEffect(g_Joysticks[joy].haptic, effect, iters) == 0;
    }
    return false;
}

//-----------------------------------------------------------------
int UpdateJoystickForceEffect(int joy, int effect, const ForceEffect& new_data)
{
    if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
        SDL_HapticEffect sdl_haptic_effect;
        setup_sdl_haptic_effect(sdl_haptic_effect, new_data);
        int effect_id = SDL_HapticUpdateEffect(g_Joysticks[joy].haptic, effect, &sdl_haptic_effect);
        if (effect_id >= 0) {
            return effect_id;
        }
    }
    return -1;
}

//-----------------------------------------------------------------
bool StopJoystickForceEffect(int joy, int effect)
{
    if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
        return SDL_HapticStopEffect(g_Joysticks[joy].haptic, effect) == 0;
    }
    return false;
}

//-----------------------------------------------------------------
bool StopAllJoystickForceEffects(int joy)
{
    if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
        return SDL_HapticStopAll(g_Joysticks[joy].haptic) == 0;
    }
    return false;
}

//-----------------------------------------------------------------
void RemoveJoystickForceEffect(int joy, int effect)
{
    if (joy >= 0 && joy < (int)g_Joysticks.size() && g_Joysticks[joy].haptic) {
        SDL_HapticDestroyEffect(g_Joysticks[joy].haptic, effect);
    }
}

//-----------------------------------------------------------------
void UpdateInput()
{
    SDL_PumpEvents(); // updates the event queue and internal input device state of SDL
}

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
            SDLJOY joy = {joystick, haptic};
            g_Joysticks.push_back(joy);
        }
    }

    return true;
}

//-----------------------------------------------------------------
void DeinitInput()
{
    for (size_t i = 0; i < g_Joysticks.size(); ++i) {
        if (g_Joysticks[i].haptic) {
            SDL_HapticClose(g_Joysticks[i].haptic);
        }
        SDL_JoystickClose(g_Joysticks[i].joystick);
    }
    SDL_QuitSubSystem(SDL_INIT_HAPTIC);
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}
