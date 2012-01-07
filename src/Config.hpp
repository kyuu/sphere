#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include "IniFile.hpp"


namespace sphere {

    struct Config {

        std::string CommonPath;
        std::string DataPath;
        std::string MainScript;
        std::vector<std::string> GameArgs;

        explicit Config(const std::string& filename) {
            IniFile ini(filename);
            CommonPath = ini.readString("Engine", "CommonPath", "common");
            DataPath   = ini.readString("Engine", "DataPath",   "data");
            MainScript = ini.readString("Engine", "MainScript", "game");
        }

    };

} // namespace sphere


#endif
