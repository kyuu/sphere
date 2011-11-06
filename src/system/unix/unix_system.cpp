#include "system.hpp"

#include <ctime>
#include <unistd.h>
#include <sys/time.h>


namespace system {

    //-----------------------------------------------------------------
    u32 GetTime()
    {
        return (u32)time(0);
    }

    //-----------------------------------------------------------------
    u32 GetTicks()
    {
        timeval tv;
        if (gettimeofday(&tv, 0) != 0) {
            return 0;
        }
        return (u32)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
    }

    //-----------------------------------------------------------------
    void Sleep(u32 millis)
    {
        usleep(millis * 1000);
    }

} // namespace system
