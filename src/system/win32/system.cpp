#include <ctime>
#include <cstdlib>
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "../system.hpp"


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
    return (int)GetTickCount();
}

//-----------------------------------------------------------------
int GetRandom()
{
    return rand();
}

//-----------------------------------------------------------------
void ThreadSleep(int ms)
{
    Sleep(ms);
}

//-----------------------------------------------------------------
bool InitSystem(const Log& log)
{
    // initialize tick count
    g_ticks_at_start = GetTickCount();

    // seed the random number generator
    srand(g_ticks_at_start);

    return true;
}

//-----------------------------------------------------------------
void DeinitSystem()
{
    // NO-OP
}
