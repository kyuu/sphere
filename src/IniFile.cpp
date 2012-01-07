#include <cassert>
#include <sstream>
#include <cstdio>
#include "common/ArrayPtr.hpp"
#include "IniFile.hpp"


namespace sphere {

    //-----------------------------------------------------------------
    IniFile::IniFile(const std::string filename)
    {
        load(filename);
    }

    //-----------------------------------------------------------------
    IniFile::~IniFile()
    {
    }

    //-----------------------------------------------------------------
    static std::string& strip_whitespace(std::string& s)
    {
        if (s[0] == ' ') {
            for (unsigned int i = 0; i < s.length(); ++i) {
                if (s[i] != ' ') {
                    s.erase(0, i);
                    break;
                }
            }
        }
        if (s[s.length()-1] == ' ') {
            for (int i = s.length()-1; i >= 0; --i) {
                if (s[i] != ' ') {
                    s.erase(i+1);
                    break;
                }
            }
        }
        return s;
    }

    //-----------------------------------------------------------------
    void
    IniFile::load(const std::string filename)
    {
        _sections.clear();

        if (filename.empty()) {
            return;
        }

        FILE* file = fopen(filename.c_str(), "rb");
        if (!file) {
            return;
        }

        static ArrayPtr<char> s_Buffer;
        static int s_BufferSize = 0;
        if (s_BufferSize == 0) { // one-time initialization
            try {
                s_Buffer.reset(new char[1024]);
                s_BufferSize = 1024;
            } catch (const std::bad_alloc&) {
                return;
            }
        }

        int num_keys_read = 0;
        std::string section;
        std::string key;
        std::string value;
        char* ptr;
        int avail;
        int state = 'A';
        int input;
        int chr;

    fetch:
        // fill buffer with data from the stream
        int num_fetched = fread(s_Buffer.get(), 1, s_BufferSize, file);
        if (num_fetched == 0) {
            goto finish;
        } else {
            avail = num_fetched;
            ptr = s_Buffer.get();
        }

    parse:
        // parse the data in the buffer
        chr = *ptr;
        if (chr == '\n' || chr == '\r') {
            input = 'n';
        } else if (chr == '[') {
            input = 'l';
        } else if (chr == ']') {
            input = 'r';
        } else if (chr == '=') {
            input = 'z';
        } else if (chr == ' ') {
            input = 's';
        } else if (chr > 31 && chr < 127) {
            input = 'g';
        } else {
            input = 'u';
        }

        switch (state) {
        case 'A':
            switch (input) {
            case 'n':
            case 's':
                break;
            case 'l':
                state = 'B';
                break;
            case 'r':
            case 'z':
            case 'g':
            case 'u':
                state = 'C';
                break;
            }
            break;
        case 'B':
            switch (input) {
            case 'n':
                state = 'A';
                break;
            case 'g':
                state = 'D';
                section = chr;
                break;
            case 's':
                break;
            case 'l':
            case 'r':
            case 'z':
            case 'u':
                state = 'C';
                break;
            }
            break;
        case 'C':
            switch (input) {
            case 'n':
                state = 'A';
                break;
            case 'l':
            case 'r':
            case 'z':
            case 's':
            case 'g':
            case 'u':
                break;
            }
            break;
        case 'D':
            switch (input) {
            case 'n':
                state = 'A';
                break;
            case 's':
            case 'g':
                section += chr;
                break;
            case 'r':
                state = 'E';
                break;
            case 'l':
            case 'z':
            case 'u':
                state = 'C';
                break;
            }
            break;
        case 'E':
            switch (input) {
            case 'n':
                state = 'F';
                break;
            case 'l':
            case 'r':
            case 'z':
            case 's':
            case 'g':
            case 'u':
                break;
            }
            break;
        case 'F':
            switch (input) {
            case 'n':
            case 's':
                break;
            case 'g':
                state = 'G';
                key = chr;
                break;
            case 'l':
                state = 'B';
                break;
            case 'r':
            case 'z':
            case 'u':
                state = 'H';
                break;
            }
            break;
        case 'G':
            switch (input) {
            case 'n':
                state = 'F';
                break;
            case 's':
            case 'g':
                key += chr;
                break;
            case 'z':
                state = 'I';
                break;
            case 'l':
            case 'r':
            case 'u':
                state = 'H';
                break;
            }
            break;
        case 'H':
            switch (input) {
            case 'n':
                state = 'F';
                break;
            case 'l':
            case 'r':
            case 'z':
            case 's':
            case 'g':
            case 'u':
                break;
            }
            break;
        case 'I':
            switch (input) {
            case 'n':
                state = 'F';
                break;
            case 's':
                break;
            case 'l':
            case 'r':
            case 'g':
                state = 'J';
                value = chr;
                break;
            case 'z':
            case 'u':
                state = 'H';
                break;
            }
            break;
        case 'J':
            switch (input) {
            case 'n':
                state = 'F';
                _sections[strip_whitespace(section)]._keys[strip_whitespace(key)] = strip_whitespace(value);
                num_keys_read++;
                break;
            case 'l':
            case 'r':
            case 's':
            case 'g':
                value += chr;
                break;
            case 'z':
            case 'u':
                state = 'H';
                _sections[strip_whitespace(section)]._keys[strip_whitespace(key)] = strip_whitespace(value);
                num_keys_read++;
                break;
            }
            break;
        }

        --avail;
        ++ptr;

        if (avail > 0) {
            goto parse;
        } else {
            goto fetch;
        }

    finish:
        if (state == 'J') { // we still have a valid key-value pair
            _sections[strip_whitespace(section)]._keys[strip_whitespace(key)] = strip_whitespace(value);
            num_keys_read++;
        }
        return;
    }

    //-----------------------------------------------------------------
    std::string
    IniFile::readString(const std::string& section, const std::string& key, const std::string& defaultValue)
    {
        assert(!section.empty());
        assert(key);
        assert(defaultValue);
        std::map<std::string, section_t>::iterator sec_it = _sections.find(section);
        if (sec_it == _sections.end()) { // section does not exist
            return defaultValue;
        }
        std::map<std::string, std::string>::iterator key_it = (*sec_it).second._keys.find(key);
        if (key_it == (*sec_it).second._keys.end()) { // key does not exist in section
            return defaultValue;
        }
        return (*key_it).second;
    }

    //-----------------------------------------------------------------
    double
    IniFile::readNumber(const std::string& section, const std::string& key, double defaultValue)
    {
        assert(!section.empty());
        assert(key);
        std::map<std::string, section_t>::iterator sec_it = _sections.find(section);
        if (sec_it == _sections.end()) { // section does not exist
            return defaultValue;
        }
        std::map<std::string, std::string>::iterator key_it = (*sec_it).second._keys.find(key);
        if (key_it == (*sec_it).second._keys.end()) { // key does not exist in section
            return defaultValue;
        }
        std::istringstream iss((*key_it).second);
        double value;
        iss >> value;
        if (!iss.good()) { // conversion went wrong
            return defaultValue;
        }
        return value;
    }

    //-----------------------------------------------------------------
    int
    IniFile::readInteger(const std::string& section, const std::string& key, int defaultValue)
    {
        assert(!section.empty());
        assert(key);
        std::map<std::string, section_t>::iterator sec_it = _sections.find(section);
        if (sec_it == _sections.end()) { // section does not exist
            return defaultValue;
        }
        std::map<std::string, std::string>::iterator key_it = (*sec_it).second._keys.find(key);
        if (key_it == (*sec_it).second._keys.end()) { // key does not exist in section
            return defaultValue;
        }
        std::istringstream iss((*key_it).second);
        int value;
        iss >> value;
        if (!iss.good()) { // conversion went wrong
            return defaultValue;
        }
        return value;
    }

    //-----------------------------------------------------------------
    bool
    IniFile::readBoolean(const std::string& section, const std::string& key, bool defaultValue)
    {
        assert(!section.empty());
        assert(key);
        std::map<std::string, section_t>::iterator sec_it = _sections.find(section);
        if (sec_it == _sections.end()) { // section does not exist
            return defaultValue;
        }
        std::map<std::string, std::string>::iterator key_it = (*sec_it).second._keys.find(key);
        if (key_it == (*sec_it).second._keys.end()) { // key does not exist in section
            return defaultValue;
        }
        std::istringstream iss((*key_it).second);
        bool value;
        iss >> value;
        if (iss.bad()) { // conversion went wrong
            return defaultValue;
        }
        return value;
    }

} // namespace sphere
