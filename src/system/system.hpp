#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "../Log.hpp"


namespace system {

    int  GetTime();
    int  GetTicks();
    int  GetRandom();
    void Sleep(int millis);

    namespace internal {

        bool InitSystem(const Log& log);
        void DeinitSystem(const Log& log);

    } // namespace internal

} // namespace system


#endif
