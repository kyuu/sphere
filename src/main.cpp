#include <cstdlib>
#include <vector>
#include <string>
#include <SDL.h>
#include "version.hpp"
#include "Log.hpp"
#include "Config.hpp"
#include "system/system.hpp"
#include "io/filesystem.hpp"
#include "graphics/video.hpp"
#include "sound/audio.hpp"
#include "input/input.hpp"
#include "script/script.hpp"

//-----------------------------------------------------------------
int main(int argc, char* argv[])
{
    Log log;
    log.open("engine.log");

    // print sphere version
    log.info() << "Sphere Engine " << SPHERE_VERSION_STRING;

    // initialize filesystem
    log.info() << "Initialize filesystem";
    if (!InitFilesystem(log)) {
        log.error() << "Could not initialize filesystem";
        return 0;
    }
    atexit(DeinitFilesystem);

    // initialize system
    log.info() << "Initialize system";
    if (!InitSystem(log)) {
        log.error() << "Could not initialize system";
        return 0;
    }
    atexit(DeinitSystem);

    // initialize video
    log.info() << "Initialize video";
    if (!InitVideo(log)) {
        log.error() << "Could not initialize video";
        return 0;
    }
    atexit(DeinitVideo);

    // initialize audio
    log.info() << "Initialize audio";
    if (!InitAudio(log)) {
        log.error() << "Could not initialize audio";
        return 0;
    }
    atexit(DeinitAudio);

    // initialize input
    log.info() << "Initialize input";
    if (!InitInput(log)) {
        log.error() << "Could not initialize input";
        return 0;
    }
    atexit(DeinitInput);

    // initialize script
    log.info() << "Initialize script";
    if (!InitScript(log)) {
        log.error() << "Could not initialize script";
        return 0;
    }
    atexit(DeinitScript);

    // load configuration
    log.info() << "Load configuration";
    Config config("engine.cfg");

    // process command line options
    log.info() << "Process command line options";
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-common" && i + 1 < argc) {
            config.CommonPath = argv[i + 1];
            i++;
        } else if (arg == "-data" && i + 1 < argc) {
            config.DataPath = argv[i + 1];
            i++;
        } else if (arg == "-main" && i + 1 < argc) {
            config.MainScript = argv[i + 1];
            i++;
        } else if (arg == "-arg" && i + 1 < argc) {
            config.GameArgs.push_back(argv[i + 1]);
            i++;
        }
    }

    // set paths and enter game directory
    SetCommonPath(config.CommonPath);
    SetDataPath(config.DataPath);
    log.info() << "Enter game directory";
    if (!SetCurrentPath(config.DataPath)) {
        log.error() << "Could not enter game directory '" << config.DataPath << "'";
        return 0;
    }

    // run game
    log.info() << "Run game";
    try {
        RunGame(log, config.MainScript, config.GameArgs);
    } catch (const std::exception& e) {
        log.error() << "Exception caught: " << (e.what() ? e.what() : "N/A");
    } catch (...) {
        log.error() << "Unknown exception caught";
    }

    return 0;
}
