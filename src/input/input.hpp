#ifndef INPUT_HPP
#define INPUT_HPP

#include "../Log.hpp"
#include "event.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "joystick.hpp"


void UpdateInput();

bool InitInput(const Log& log);
void DeinitInput();


#endif
