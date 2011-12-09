#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "../Log.hpp"


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

int  GetTime();
TimeInfo GetTimeInfo(int rawtime, bool utc = false);
int  GetTicks();
float GetRandom();
void ThreadSleep(int ms);

bool InitSystem(const Log& log);
void DeinitSystem();


#endif
