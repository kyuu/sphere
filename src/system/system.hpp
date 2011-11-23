#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "../Log.hpp"


int  GetTime();
int  GetTicks();
int  GetRandom();
void ThreadSleep(int ms);

bool InitSystem(const Log& log);
void DeinitSystem();


#endif
