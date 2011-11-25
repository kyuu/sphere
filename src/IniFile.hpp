#ifndef INIFILE_HPP
#define INIFILE_HPP

#include <string>
#include <map>
#include "io/IStream.hpp"


class IniFile {
public:
    explicit IniFile(const std::string filename);
    ~IniFile();

    void load(const std::string filename);

    std::string readString(const std::string& section, const std::string& key, const std::string& defaultValue);
    double readNumber(const std::string& section, const std::string& key, double defaultValue);
    int readInteger(const std::string& section, const std::string& key, int defaultValue);
    bool readBoolean(const std::string& section, const std::string& key, bool defaultValue);

private:
    struct section_t {
        std::map<std::string, std::string> _keys;
    };
    std::map<std::string, section_t> _sections;
};


#endif
