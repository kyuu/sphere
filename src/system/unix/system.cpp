#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>
#include "../system.hpp"


//-----------------------------------------------------------------
// globals
static unsigned int g_TicksAtStart = 0;

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

//-----------------------------------------------------------------
int GetTicks()
{
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        return 0;
    }
    return (int)(((tv.tv_sec * 1000) + (tv.tv_usec / 1000)) - g_TicksAtStart);
}

//-----------------------------------------------------------------
float GetRandom()
{
    return rand() / (float)RAND_MAX;
}

//-----------------------------------------------------------------
void ThreadSleep(int ms)
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
    g_TicksAtStart = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    // seed the random number generator
    srand(g_TicksAtStart);

    return true;
}

//-----------------------------------------------------------------
void DeinitSystem()
{
    // NO-OP
}
