#include "system.hpp"

#include <ctime>
#include <cstdlib>
#include <windows.h>


//-----------------------------------------------------------------
// globals
static unsigned int g_ticks_at_start = 0;

namespace system {

    //-----------------------------------------------------------------
    int GetTime()
    {
        return (int)time(0);
    }

    //-----------------------------------------------------------------
    int GetTicks()
    {
        return (int)(::GetTickCount());
    }

    //-----------------------------------------------------------------
    int GetRandom()
    {
        return rand();
    }

    //-----------------------------------------------------------------
    void Sleep(int millis)
    {
        ::Sleep(millis);
    }

    namespace internal {

        //-----------------------------------------------------------------
        bool InitSystem(const Log& log)
        {
            // initialize tick count
            g_ticks_at_start = ::GetTickCount();

            // seed the random number generator
            srand(g_ticks_at_start);

            return true;
        }

        //-----------------------------------------------------------------
        void DeinitSystem(const Log& log)
        {
            // NO-OP
        }

    } // namespace internal

} // namespace system
