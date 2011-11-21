#include <vector>
#include <string>
#include <SDL.h>
#include "../../Log.hpp"
#include "../input.hpp"

//-----------------------------------------------------------------
struct Joystick {
    SDL_Joystick* joystick;
    SDL_Haptic* haptic;
};

//-----------------------------------------------------------------
// globals
std::vector<Joystick> g_joysticks;
int   gnum_keys = 0;
Uint8 g_keyboard_state = 0;


//-----------------------------------------------------------------
void UpdateInput()
{
    SDL_PumpEvents(); // updates the event queue and internal input device state of SDL
}

//-----------------------------------------------------------------
bool InitInput(const Log& log)
{
    // print SDL version
    SDL_version sdl_linked_version;
    SDL_GetVersion(&sdl_linked_version);
    log.info() << "Using SDL " << (int)sdl_linked_version.major \
               << "."          << (int)sdl_linked_version.minor \
               << "."          << (int)sdl_linked_version.patch;

    // initialize SDL
    log.info() << "Initializing SDL (if not already initialized)";
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0) {
        log.error() << "Failed initializing SDL: " << SDL_GetError();
        return false;
    }

    // ensure SDL will be properly deinitialized
    atexit(SDL_Quit);

    // open all available joysticks
    int num_joysticks = SDL_NumJoysticks();
    if (num_joysticks > 0) {
        if (num_joysticks == 1) {
            log.info() << "1 Joystick available";
        } else {
            log.info() << num_joysticks << " Joysticks available";
        }

        for (int i = 0; i < num_joysticks; ++i) {
            log.info() << "Opening joystick " << i << ": '" << SDL_JoystickName(i) << "'"
            SDL_Joystick* joystick = SDL_JoystickOpen(i);
            if (!joystick) {
                log.error() << "Failed: " << SDL_GetError();
                return false;
            }
            log.info() << "Joystick " << i << " has " << SDL_JoystickNumButtons(joystick) << " buttons, " \
                                                      << SDL_JoystickNumAxes(joystick)    << " axes and " \
                                                      << SDL_JoystickNumHats(joystick)    << " POV hats";

            // open haptic if available
            SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joystick);
            if (haptic) {
                log.info() << "Joystick " << i << " has Force Feedback";
                log.info() << "Joystick " << i << " can store up to " << SDL_HapticNumEffects(haptic) \
                                               << " force effects and play " << SDL_HapticNumEffectsPlaying(haptic) \
                                               << " force effects simultaneously";
            } else {
                log.info() << "Joystick " << i << " has no Force Feedback";
            }

            Joystick joy = {joystick, haptic};
            g_joysticks.push_back(joy);
        }
    } else {
        log.info() << "No joysticks available";
    }

    return true;
}

//-----------------------------------------------------------------
void DeinitInput(const Log& log)
{
    for (size_t i = 0; i < g_joysticks.size(); ++i) {
        if (g_joysticks[i].haptic) {
            SDL_HapticClose(g_joysticks[i].haptic);
        }
        SDL_JoystickClose(g_joysticks[i].joystick);
    }
    SDL_QuitSubSystem(SDL_INIT_HAPTIC);
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}
