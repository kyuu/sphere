#ifndef EVENT_HPP
#define EVENT_HPP


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
        int type; // KEY_DOWN or KEY_UP
        int key;  // the key that changed its state
    } key;

    // a mouse button event
    struct {
        int type;   // MOUSE_BUTTON_DOWN or MOUSE_BUTTON_UP
        int button; // the mouse button that changed its state
    } mbutton;

    // a mouse motion event
    struct {
        int type; // MOUSE_MOTION
        int dx;   // the change in x
        int dy;   // the change in y
    } mmotion;

    // a mouse wheel event
    struct {
        int type; // MOUSE_WHEEL_MOTION
        int dx;   // the change in x
        int dy;   // the change in y
    } mwheel;

    // a joystick button event
    struct {
        int type;   // JOY_BUTTON_DOWN or JOY_BUTTON_UP
        int joy;    // the joystick that generated the event
        int button; // the joystick button that changed its state
    } jbutton;

    // a joystick axis motion event
    struct {
        int type;  // JOY_AXIS_MOTION
        int joy;   // the joystick that generated the event
        int axis;  // the joystick axis that was moved
        int value; // current axis value
    } jaxis;

    // a joystick hat event
    struct {
        int type;  // JOY_HAT_MOTION
        int joy;   // the joystick that generated the event
        int hat;   // the joystick hat that was moved
        int value; // current hat value
    } jhat;
};

bool AreEventsPending();
bool GetEvent(Event& out);
void ClearEvents();


#endif
