#include <cstdlib>
#include <vector>
#include <string>
#include <SDL.h>
#include "version.hpp"
#include "Log.hpp"
#include "filesystem/filesystem.hpp"
#include "system/system.hpp"
#include "video/video.hpp"
#include "audio/audio.hpp"
#include "input/input.hpp"
#include "script/script.hpp"

//-----------------------------------------------------------------
int main(int argc, char* argv[])
{
    Log log;
    log.open("log.txt");

    // print sphere version
    log.info() << "Sphere Engine " << SPHERE_VERSION_STRING << " " << SPHERE_VERSION_AFFIX;

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

    // process command line options
    log.info() << "Process command line options";
    std::string data = GetEnginePath() + "/data";
    std::string startup = "game";
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-data" && i + 1 < argc) {
            data = argv[i + 1];
            i++;
        } else if (arg == "-startup" && i + 1 < argc) {
            startup = argv[i + 1];
            i++;
        } else if (arg == "-arg" && i + 1 < argc) {
            args.push_back(argv[i + 1]);
            i++;
        }
    }

    // set and enter data path
    log.info() << "Enter game directory";
    SetDataPath(data);
    if (!SetCurrentPath(data)) {
        log.error() << "Could not enter game directory '" << data << "'";
        return 0;
    }

    // run game
    log.info() << "Run game";
    try {
        if (DoesFileExist(startup + ".bytecode")) {
            RunGame(log, startup + ".bytecode", args);
        } else if (DoesFileExist(startup + ".script")) {
            RunGame(log, startup + ".script", args);
        } else {
            log.error() << "Startup script '" << startup << "' does not exist";
        }
    } catch (const std::exception& e) {
        log.error() << "Exception caught: " << (e.what() ? e.what() : "N/A");
    } catch (...) {
        log.error() << "Unknown exception caught";
    }

    return 0;
}
