#include "player_version.hpp"
#include "../system/system.hpp"
#include "../system/system_version.hpp"
#include "../game/game.hpp"
#include "../game/game_version.hpp"
#include "../script/script.hpp"
#include "../script/script_version.hpp"
#include "../system/log.hpp"


int main(int argc, char* argv[])
{
    sphere::system::log::open("log.txt");
    sphere::system::log::info() << "Player " << PLAYER_VERSION_STRING;

    system::log::info() << "Using system module " << SYSTEM_VERSION_STRING;
    if (system::version() != SYSTEM_VERSION) {
        system::log::warning() << "Compiled against system module " << system::version_string();
    }

    system::log::Info() << "Using game module " << GAME_VERSION_STRING;
    if (game::version() != GAME_VERSION) {
        system::log::warning() << "Compiled against game module " << game::version_string();
    }

    system::log::Info() << "Using script module " << SCRIPT_VERSION_STRING;
    if (script::version() != SCRIPT_VERSION) {
        system::log::warning() << "Compiled against script module " << script::version_string();
    }

    return 0;
}
