#include <SDL.h>
#include "../mouse.hpp"


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
