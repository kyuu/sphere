#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>
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
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        return 0;
    }
    return (int)(((tv.tv_sec * 1000) + (tv.tv_usec / 1000)) - g_ticks_at_start);
}

//-----------------------------------------------------------------
int GetRandom()
{
    return rand();
}

//-----------------------------------------------------------------
void Sleep(int ms)
{
    usleep(ms * 1000);
}

//-----------------------------------------------------------------
bool InitSystem(const Log& log)
{
    // initialize tick count
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        return 0;
    }
    g_ticks_at_start = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    // seed the random number generator
    srand(g_ticks_at_start);

    return true;
}

//-----------------------------------------------------------------
void DeinitSystem()
{
    // NO-OP
}
