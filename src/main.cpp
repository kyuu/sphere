#include <cstdlib>
#include <vector>
#include <string>
#include "version.hpp"
#include "Log.hpp"
#include "filesystem/filesystem.hpp"
#include "system/system.hpp"
#include "video/video.hpp"
#include "audio/audio.hpp"
#include "input/input.hpp"
#include "script/script.hpp"


int main(int argc, char* argv[])
{
    Log log("log.txt");

    // print sphere version
    log.info() << "Sphere " << SPHERE_VERSION_STRING;

    // intialize filesystem
    log.info() << "Initializing filesystem";
    if (!InitFilesystem(log)) {
        log.error() << "Could not initialize filesystem";
        return 0;
    }
    atexit(DeinitFilesystem);

    // intialize system
    log.info() << "Initializing system";
    if (!InitSystem(log)) {
        log.error() << "Could not initialize system";
        return 0;
    }
    atexit(DeinitSystem);

    // intialize video
    log.info() << "Initializing video";
    if (!InitVideo(log)) {
        log.error() << "Could not initialize video";
        return 0;
    }
    atexit(DeinitVideo);

    // intialize audio
    log.info() << "Initializing audio";
    if (!InitAudio(log)) {
        log.error() << "Could not initialize audio";
        return 0;
    }
    atexit(DeinitAudio);

    // intialize input
    log.info() << "Initializing input";
    if (!InitInput(log)) {
        log.error() << "Could not initialize input";
        return 0;
    }
    atexit(DeinitInput);

    // intialize script
    log.info() << "Initializing script";
    if (!InitScript(log)) {
        log.error() << "Could not initialize script";
        return 0;
    }
    atexit(DeinitScript);

    // process command line options
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
    log.info() << "Entering game directory '" << data << "'";
    SetDataPath(data);
    if (!SetCurrentPath(data)) {
        log.error() << "Could not enter '" << data << "'";
        return 0;
    }

    // run game
    log.info() << "Running game startup script '" << startup << "'";
    if (DoesFileExist(startup + ".bytecode")) {
        RunGame(log, startup + ".bytecode", args);
    } else if (DoesFileExist(startup + ".script")) {
        RunGame(log, startup + ".script", args);
    } else {
        log.error() << "Script '" << startup << "' does not exist";
    }

    return 0;
}
