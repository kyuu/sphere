#ifndef SPHERE_SYSTEM_HPP
#define SPHERE_SYSTEM_HPP

#include "../Log.hpp"


namespace sphere {
    namespace system {

        // thread
        void Sleep(int ms);

        // time
        struct TimeInfo {
            int second;  // 0-59
            int minute;  // 0-59
            int hour;    // 0-23
            int day;     // 1-31
            int month;   // 1-12
            int year;
            int weekday; // 1-7
            int yearday; // 1-366
        };

        int GetTicks();
        int GetTime();
        TimeInfo GetTimeInfo(int rawtime, bool utc = false);

        namespace internal {

            bool InitSystem(const Log& log);
            void DeinitSystem();

        } // namespace internal
    } // namespace system
} // namespace sphere


#endif
