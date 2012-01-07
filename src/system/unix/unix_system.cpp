#include <cassert>
#include <ctime>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include "../../version.hpp"
#include "../system.hpp"

#define DEFAULT_WINDOW_WIDTH  640
#define DEFAULT_WINDOW_HEIGHT 480


//-----------------------------------------------------------------
// globals
static unsigned int g_TicksAtSystemInit = 0;

namespace sphere {
    namespace system {

        //-----------------------------------------------------------------
        void Sleep(int ms)
        {
            usleep(ms * 1000);
        }

        //-----------------------------------------------------------------
        int GetTicks()
        {
            timeval tv;
            if (gettimeofday(&tv, 0) != 0) {
                return 0;
            }
            return (int)(((tv.tv_sec * 1000) + (tv.tv_usec / 1000)) - g_TicksAtSystemInit);
        }

        //-----------------------------------------------------------------
        int GetTime()
        {
            return (int)time(0);
        }

        //-----------------------------------------------------------------
        TimeInfo GetTimeInfo(int rawtime, bool utc)
        {
            time_t _rawtime = (time_t)rawtime;
            tm* _timeinfo;
            if (utc) {
                _timeinfo = gmtime(&_rawtime);
            } else {
                _timeinfo = localtime(&_rawtime);
            }
            TimeInfo timeinfo;
            timeinfo.second  = _timeinfo->tm_sec;
            timeinfo.minute  = _timeinfo->tm_min;
            timeinfo.hour    = _timeinfo->tm_hour;
            timeinfo.day     = _timeinfo->tm_mday;
            timeinfo.month   = _timeinfo->tm_mon + 1;
            timeinfo.year    = _timeinfo->tm_year + 1900;
            timeinfo.weekday = (_timeinfo->tm_wday == 0 ? 7 : _timeinfo->tm_wday);
            timeinfo.yearday = _timeinfo->tm_yday + 1;
            return timeinfo;
        }

        namespace internal {

            //-----------------------------------------------------------------
            bool InitSystem(const Log& log)
            {
                // initialize tick count
                timeval tv;
                if (gettimeofday(&tv, 0) != 0) {
                    return 0;
                }
                g_TicksAtSystemInit = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

                return true;
            }

            //-----------------------------------------------------------------
            void DeinitSystem()
            {
            }

        } // namespace internal
    } // namespace system
} // namespace sphere
