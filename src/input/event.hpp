#ifndef EVENT_HPP
#define EVENT_HPP


union Event {
    enum Type {
        ET_KEY_DOWN = 0,
        ET_KEY_UP,

        ET_MOUSE_BUTTON_DOWN,
        ET_MOUSE_BUTTON_UP,
        ET_MOUSE_MOTION,
        ET_MOUSE_WHEEL_MOTION,

        ET_JOY_BUTTON_DOWN,
        ET_JOY_BUTTON_UP,
        ET_JOY_AXIS_MOTION,
        ET_JOY_HAT_MOTION,

        ET_APP_QUIT,
    };

    // the type of the event
    int type;

    // a key event
    struct {
        int type; // Event::ET_KEY_DOWN or Event::ET_KEY_UP
        int key;  // the key that changed its state
    } key;

    // a mouse button event
    struct {
        int type;   // Event::ET_MOUSE_BUTTON_DOWN or Event::ET_MOUSE_BUTTON_UP
        int button; // the mouse button that changed its state
    } mbutton;

    // a mouse motion event
    struct {
        int type; // Event::ET_MOUSE_MOTION
        int dx;   // the change in x
        int dy;   // the change in y
    } mmotion;

    // a mouse wheel event
    struct {
        int type; // Event::ET_MOUSE_WHEEL_MOTION
        int dx;   // the change in x
        int dy;   // the change in y
    } mwheel;

    // a joystick button event
    struct {
        int type;   // Event::ET_JOY_BUTTON_DOWN or Event::ET_JOY_BUTTON_UP
        int joy;    // the joystick that generated the event
        int button; // the joystick button that changed its state
    } jbutton;

    // a joystick axis motion event
    struct {
        int type;  // Event::ET_JOY_AXIS_MOTION
        int joy;   // the joystick that generated the event
        int axis;  // the joystick axis that was moved
        int value; // current axis value
    } jaxis;

    // a joystick hat event
    struct {
        int type;  // Event::ET_JOY_HAT_MOTION
        int joy;   // the joystick that generated the event
        int hat;   // the joystick hat that was moved
        int state; // current hat state
    } jhat;
};

bool AreEventsPending();
bool GetEvent(Event& out);
void ClearEvents();


#endif
