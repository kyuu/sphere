#ifndef INPUT_HPP
#define INPUT_HPP

#include "../Log.hpp"
#include "event.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "joystick.hpp"


namespace input {

    void UpdateInput();

    namespace internal {

        bool InitInput(const Log& log);
        void DeinitInput(const Log& log);

    } // namespace internal

} // namespace input


#endif
