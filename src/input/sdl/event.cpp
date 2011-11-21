#include <SDL.h>
#include "../event.hpp"
#include "../keyboard.hpp"


//-----------------------------------------------------------------
static int sdl_scancode_to_key[512] = { // SDL_NUM_SCANCODES == 512
    -1,
    -1,
    -1,
    -1,
    input::KEY_A,
    input::KEY_B,
    input::KEY_C,
    input::KEY_D,
    input::KEY_E,
    input::KEY_F,
    input::KEY_G,
    input::KEY_H,
    input::KEY_I,
    input::KEY_J,
    input::KEY_K,
    input::KEY_L,
    input::KEY_M,
    input::KEY_N,
    input::KEY_O,
    input::KEY_P,
    input::KEY_Q,
    input::KEY_R,
    input::KEY_S,
    input::KEY_T,
    input::KEY_U,
    input::KEY_V,
    input::KEY_W,
    input::KEY_X,
    input::KEY_Y,
    input::KEY_Z,
    input::KEY_1,
    input::KEY_2,
    input::KEY_3,
    input::KEY_4,
    input::KEY_5,
    input::KEY_6,
    input::KEY_7,
    input::KEY_8,
    input::KEY_9,
    input::KEY_0,
    input::KEY_RETURN,
    input::KEY_ESCAPE,
    input::KEY_BACKSPACE,
    input::KEY_TAB,
    input::KEY_SPACE,
    input::KEY_MINUS,
    input::KEY_EQUALS,
    input::KEY_LEFTBRACKET,
    input::KEY_RIGHTBRACKET,
    input::KEY_BACKSLASH,
    -1,
    input::KEY_SEMICOLON,
    input::KEY_APOSTROPHE,
    input::KEY_GRAVE,
    input::KEY_COMMA,
    input::KEY_PERIOD,
    input::KEY_SLASH,
    input::KEY_CAPSLOCK,
    input::KEY_F1,
    input::KEY_F2,
    input::KEY_F3,
    input::KEY_F4,
    input::KEY_F5,
    input::KEY_F6,
    input::KEY_F7,
    input::KEY_F8,
    input::KEY_F9,
    input::KEY_F10,
    input::KEY_F11,
    input::KEY_F12,
    input::KEY_PRINTSCREEN,
    input::KEY_SCROLLLOCK,
    input::KEY_PAUSE,
    input::KEY_INSERT,
    input::KEY_HOME,
    input::KEY_PAGEUP,
    input::KEY_DELETE,
    input::KEY_END,
    input::KEY_PAGEDOWN,
    input::KEY_RIGHT,
    input::KEY_LEFT,
    input::KEY_DOWN,
    input::KEY_UP,
    input::KEY_NUMLOCKCLEAR,
    input::KEY_KP_DIVIDE,
    input::KEY_KP_MULTIPLY,
    input::KEY_KP_MINUS,
    input::KEY_KP_PLUS,
    input::KEY_KP_ENTER,
    input::KEY_KP_1,
    input::KEY_KP_2,
    input::KEY_KP_3,
    input::KEY_KP_4,
    input::KEY_KP_5,
    input::KEY_KP_6,
    input::KEY_KP_7,
    input::KEY_KP_8,
    input::KEY_KP_9,
    input::KEY_KP_0,
    input::KEY_KP_PERIOD,
    input::KEY_NONUSBACKSLASH,
    -1,
    -1,
    input::KEY_KP_EQUALS,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    input::KEY_KP_COMMA,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    input::KEY_CANCEL,
    input::KEY_CLEAR,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    input::KEY_LCTRL,
    input::KEY_LSHIFT,
    input::KEY_LALT,
    -1,
    input::KEY_RCTRL,
    input::KEY_RSHIFT,
    input::KEY_RALT,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
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
            out.jhat.value = sdl_event.jhat.value;
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