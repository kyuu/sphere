#include "common/platform.hpp"
#include "Log.hpp"


namespace sphere {

    //-----------------------------------------------------------------
    bool
    Log::open(const std::string& filename)
    {
        if (filename.empty()) {
            return false;
        }
        close();
        _file = fopen(filename.c_str(), "w");
        return _file != 0;
    }

    //-----------------------------------------------------------------
    void
    Log::close()
    {
        if (_file) {
            fclose(_file);
        }
    }

    //-----------------------------------------------------------------
    void
    Log::write(ReportingLevel level, const std::string& msg) const
    {
        if (_file && level <= _reportingLevel) {
            switch (level) {
            case RL_INFO:    fprintf(_file, "Info   : "); break;
            case RL_WARNING: fprintf(_file, "Warning: "); break;
            case RL_ERROR:   fprintf(_file, "Error  : "); break;
            case RL_DEBUG:   fprintf(_file, "Debug  : "); break;
            }
            fprintf(_file, "%s\n", msg.c_str());
            fflush(_file);
        }
    }

} // namespace sphere
