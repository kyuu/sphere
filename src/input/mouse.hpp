#ifndef MOUSE_HPP
#define MOUSE_HPP


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


#endif
