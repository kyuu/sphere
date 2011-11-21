#include "system.hpp"

#include <ctime>
#include <cstdlib>
namespace winapi {
#include <windows.h>
}


//-----------------------------------------------------------------
// globals
static unsigned int g_ticks_at_start = 0;

//-----------------------------------------------------------------
int GetTime()
{
    return (int)time(0);
}

//-----------------------------------------------------------------
int GetTicks()
{
    return (int)(winapi::GetTickCount());
}

//-----------------------------------------------------------------
int GetRandom()
{
    return rand();
}

//-----------------------------------------------------------------
void Sleep(int ms)
{
    winapi::Sleep(ms);
}

//-----------------------------------------------------------------
bool InitSystem(const Log& log)
{
    // initialize tick count
    g_ticks_at_start = winapi::GetTickCount();

    // seed the random number generator
    srand(g_ticks_at_start);

    return true;
}

//-----------------------------------------------------------------
void DeinitSystem(const Log& log)
{
    // NO-OP
}
