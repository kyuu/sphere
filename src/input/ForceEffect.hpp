#ifndef FORCEEFFECT_HPP
#define FORCEEFFECT_HPP

#include "../common/types.hpp"


struct ForceEffect {

    u32 direction;   // direction of the force in degrees (0 - 360)
    u32 duration;    // duration of the effect
    i32 start;       // start strength
    i32 end;         // end strength

};


#endif
