#include <cstdlib>
#include <vector>
#include <string>
#include "io/filesystem.hpp"
#include "system/system.hpp"
#include "graphics/video.hpp"
#include "audio/audio.hpp"
#include "input/input.hpp"
#include "script/vm.hpp"
#include "Log.hpp"
#include "Config.hpp"
#include "version.hpp"


//-----------------------------------------------------------------
int main(int argc, char* argv[])
{
    // load configuration
    sphere::Config config("engine.cfg");

    // process command line options
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

    // open log
    sphere::Log log("engine.log");

    // print sphere version
    log.info() << "Sphere Engine " << SPHERE_VERSION_STRING << " " << SPHERE_AFFIX;

    // initialize file system
    log.info() << "Initializing file system";
    if (!sphere::io::filesystem::internal::InitFileSystem(log, config.CommonPath, config.DataPath)) {
        log.error() << "Could not initialize file system";
        return 0;
    }
    atexit(sphere::io::filesystem::internal::DeinitFileSystem);

    // initialize system
    log.info() << "Initializing system";
    if (!sphere::system::internal::InitSystem(log)) {
        log.error() << "Could not initialize system";
        return 0;
    }
    atexit(sphere::system::internal::DeinitSystem);

    // initialize video
    log.info() << "Initializing video";
    if (!sphere::video::internal::InitVideo(log)) {
        log.error() << "Could not initialize video";
        return 0;
    }
    atexit(sphere::video::internal::DeinitVideo);

    // initialize audio
    log.info() << "Initializing audio";
    if (!sphere::audio::internal::InitAudio(log)) {
        log.error() << "Could not initialize audio";
        return 0;
    }
    atexit(sphere::audio::internal::DeinitAudio);

    // initialize input
    log.info() << "Initializing input";
    if (!sphere::input::internal::InitInput(log)) {
        log.error() << "Could not initialize input";
        return 0;
    }
    atexit(sphere::input::internal::DeinitInput);

    // initialize vm
    log.info() << "Initializing script VM";
    if (!sphere::script::internal::InitVM(log)) {
        log.error() << "Could not initialize script VM";
        return 0;
    }
    atexit(sphere::script::internal::DeinitVM);

    // run game
    log.info() << "Running game";

    // load main script
    if (sphere::io::filesystem::FileExists(config.MainScript + SCRIPT_FILE_EXT)) {
        std::string filename = config.MainScript + SCRIPT_FILE_EXT;
        sphere::FilePtr file = sphere::io::filesystem::OpenFile(filename);
        if (!sphere::script::CompileStream(file.get(), file->getName())) {
            log.error() << "Could not compile '" << filename << "': " << sphere::script::GetLastError();
            return 0;
        }
    } else if (sphere::io::filesystem::FileExists(config.MainScript + BYTECODE_FILE_EXT)) {
        std::string filename = config.MainScript + BYTECODE_FILE_EXT;
        sphere::FilePtr file = sphere::io::filesystem::OpenFile(filename);
        if (!sphere::script::LoadObject(file.get()) || sq_gettype(sphere::script::GetVM(), -1) != OT_CLOSURE) {
            log.error() << "Could not load bytecode from '" << filename << "': " << sphere::script::GetLastError();
            return 0;
        }
    } else {
        log.error() << "Could not find main script '" << config.MainScript << "'";
        return 0;
    }

    // evaluate main script
    sq_pushroottable(sphere::script::GetVM()); // this
    if (!SQ_SUCCEEDED(sq_call(sphere::script::GetVM(), 1, SQFalse, SQTrue))) {
        log.error() << "Could not evaluate main script: " << sphere::script::GetLastError();
        return 0;
    }

    // call main function
    sq_pushroottable(sphere::script::GetVM());
    sq_pushstring(sphere::script::GetVM(), "main", -1);
    if (!SQ_SUCCEEDED(sq_rawget(sphere::script::GetVM(), -2)) || sq_gettype(sphere::script::GetVM(), -1) != OT_CLOSURE) {
        log.error() << "Could not call main function, symbol not defined";
        return 0;
    }
    if (sq_gettype(sphere::script::GetVM(), -1) != OT_CLOSURE) {
        log.error() << "Could not call main function, symbol defined, but is not a function";
        return 0;
    }
    sq_pushroottable(sphere::script::GetVM()); // this
    for (int i = 0; i < (int)config.GameArgs.size(); i++) {
        sq_pushstring(sphere::script::GetVM(), config.GameArgs[i].c_str(), -1);
    }
    if (!SQ_SUCCEEDED(sq_call(sphere::script::GetVM(), 1 + config.GameArgs.size(), SQFalse, SQTrue))) {
        log.error() << "Unhandled script exception: " << sphere::script::GetLastError();
    }

    // exit
    log.info() << "Exiting";

    return 0;
}
