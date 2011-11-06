#include "system.hpp"

#include <ctime>
#define WIN32_MEAN_AND_LEAN
#include <windows.h>


namespace system {

    //-----------------------------------------------------------------
    u32 GetTime()
    {
        return (u32)time(0);
    }

    //-----------------------------------------------------------------
    u32 GetTicks()
    {
        return (u32)(::GetTickCount());
    }

    //-----------------------------------------------------------------
    void Sleep(u32 millis)
    {
        ::Sleep(millis);
    }

} // namespace system
