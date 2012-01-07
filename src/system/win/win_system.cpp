#include <cassert>
#include <ctime>
#include <windows.h>
#include "../system.hpp"


namespace sphere {
    namespace system {

        //-----------------------------------------------------------------
        // globals
        unsigned int g_TicksAtSystemInit = 0;

        //-----------------------------------------------------------------
        void Sleep(int ms)
        {
            ::Sleep(ms);
        }

        //-----------------------------------------------------------------
        int GetTicks()
        {
            return (int)(GetTickCount() - g_TicksAtSystemInit);
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
            tm* _ti;
            if (utc) {
                _ti = gmtime(&_rawtime);
            } else {
                _ti = localtime(&_rawtime);
            }
            TimeInfo ti;
            ti.second  = _ti->tm_sec;
            ti.minute  = _ti->tm_min;
            ti.hour    = _ti->tm_hour;
            ti.day     = _ti->tm_mday;
            ti.month   = _ti->tm_mon + 1;
            ti.year    = _ti->tm_year + 1900;
            ti.weekday = (_ti->tm_wday == 0 ? 7 : _ti->tm_wday);
            ti.yearday = _ti->tm_yday + 1;
            return ti;
        }

        namespace internal {

            //-----------------------------------------------------------------
            bool InitSystem(const Log& log)
            {
                // initialize tick count
                g_TicksAtSystemInit = GetTickCount();

                return true;
            }

            //-----------------------------------------------------------------
            void DeinitSystem()
            {
                // NO-OP
            }

        } // namespace internal
    } // namespace system
} // namespace sphere
