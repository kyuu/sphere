#ifndef INPUT_HPP
#define INPUT_HPP

#include "../common/types.hpp"


namespace input {

    // update
    void UpdateInput();

    // event
    union Event {
        enum Type {
            KEY_DOWN = 0,
            KEY_UP,

            MOUSE_BUTTON_DOWN,
            MOUSE_BUTTON_UP,
            MOUSE_MOTION,
            MOUSE_WHEEL_MOTION,

            JOY_BUTTON_DOWN,
            JOY_BUTTON_UP,
            JOY_AXIS_MOTION,
            JOY_HAT_MOTION,

            APP_QUIT,
        };

        // the type of the event
        int type;

        // a key event
        struct {
            int type; // Event::KEY_DOWN or Event::KEY_UP
            int key;  // the key that changed its state
        } key;

        // a mouse button event
        struct {
            int type;   // Event::MOUSE_BUTTON_DOWN or Event::MOUSE_BUTTON_UP
            int button; // the mouse button that changed its state
        } mbutton;

        // a mouse motion event
        struct {
            int type; // Event::MOUSE_MOTION
            int dx;   // the change in x
            int dy;   // the change in y
        } mmotion;

        // a mouse wheel event
        struct {
            int type; // Event::MOUSE_WHEEL_MOTION
            int dx;   // the change in x
            int dy;   // the change in y
        } mwheel;

        // a joystick button event
        struct {
            int type;   // Event::JOY_BUTTON_DOWN or Event::JOY_BUTTON_UP
            int joy;    // the joystick that generated the event
            int button; // the joystick button that changed its state
        } jbutton;

        // a joystick axis motion event
        struct {
            int type;  // Event::JOY_AXIS_MOTION
            int joy;   // the joystick that generated the event
            int axis;  // the joystick axis that was moved
            int value; // current axis value
        } jaxis;

        // a joystick hat event
        struct {
            int type;  // Event::JOY_HAT_MOTION
            int joy;   // the joystick that generated the event
            int hat;   // the joystick hat that was moved
            int state; // current hat state
        } jhat;
    };

    bool AreEventsPending();
    bool GetEvent(Event& out);
    void ClearEvents();

    // keyboard
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
        KEY_KP_EQUALS,
        KEY_KP_COMMA,
        KEY_CANCEL,
        KEY_CLEAR,
        KEY_LCTRL,
        KEY_LSHIFT,
        KEY_LALT,
        KEY_RCTRL,
        KEY_RSHIFT,
        KEY_RALT,
        NUM_KEYS, // not a key, but marks the amount of keys
    };

    bool IsKeyDown(int key);

    // mouse
    enum {
        MOUSE_BUTTON_LEFT = 0,
        MOUSE_BUTTON_MIDDLE,
        MOUSE_BUTTON_RIGHT,
        MOUSE_BUTTON_X1,
        MOUSE_BUTTON_X2,
    };

    int  GetMouseX();
    int  GetMouseY();
    bool IsMouseButtonDown(int button);

    // joystick
    enum {
        JOY_AXIS_X = 0,
        JOY_AXIS_Y,
        JOY_AXIS_Z,
        JOY_AXIS_R,
    };

    enum {
        JOY_HAT_CENTERED = 0,
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

    // joystick force feedback
    union ForceEffect {

        static const int INFINITY = 0x7FFFFFFF; // (2^31)-1 highest possible 32-bit signed integer number

        enum Type {
            CONSTANT = 0, // constant effect
            RAMP,         // ramp effect
            SINE,         // periodic effect
            SQUARE,       // periodic effect
            TRIANGLE,     // periodic effect
            SAWTOOTHUP,   // periodic effect
            SAWTOOTHDOWN, // periodic effect
        };

        struct Direction {
            enum Type {
                CARTESIAN = 0, // cartesian direction using the x, y, and z axes
                POLAR,         // polar direction using an angle
            };

            int type;      // the type of the force direction encoding
            int params[3]; // encoded direction parameters
        };

        // the type of the force effect
        int type;

        // a constant force effect
        struct {
            int type;                  // ForceEffect::CONSTANT
            Direction direction;       // direction of the force
            u32 length;                // effect duration
            u16 delay;                 // delay before playing the effect (again)
            u16 attack_length;
            u16 attack_level;
            u16 fade_length;
            u16 fade_level;
            i16 level;                 // effect strength
        } constant;

        // a ramp force effect
        struct {
            int type;                  // ForceEffect::RAMP
            Direction direction;       // direction of the force
            u32 length;                // effect duration
            u16 delay;                 // delay before playing the effect (again)
            u16 attack_length;
            u16 attack_level;
            u16 fade_length;
            u16 fade_level;
            i16 start;
            i16 end;
        } ramp;

        // a periodic force effect
        struct {
            int type;                  // ForceEffect::PERIODIC
            Direction direction;       // direction of the force
            u32 length;                // effect duration
            u16 delay;                 // delay before playing the effect (again)
            u16 attack_length;
            u16 attack_level;
            u16 fade_length;
            u16 fade_level;
            u16 period;
            i16 magnitude;
            i16 offset;
            u16 phase;
        } periodic;
    };

    bool HasJoystickForceFeedback(int joy);
    int  GetNumStorableForceEffects(int joy);
    int  GetNumPlayableForceEffects(int joy);
    bool IsForceEffectSupported(int joy, int effect);
    int  UploadForceEffect(int joy, const ForceEffect& effect);
    bool PlayForceEffect(int joy, int effect, int iterations = 1);
    int  UpdateForceEffect(int joy, int effect, const ForceEffect& new_data);
    bool StopForceEffect(int joy, int effect);
    bool StopAllForceEffects(int joy);
    void RemoveForceEffect(int joy, int effect);

    namespace internal {

        bool InitInput(const Log& log);
        void DeinitInput(const Log& log);

    } // namespace internal

} // namespace input


#endif
