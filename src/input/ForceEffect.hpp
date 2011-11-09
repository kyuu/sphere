#ifndef FORCEEFFECT_HPP
#define FORCEEFFECT_HPP

#include "../common/types.hpp"


union ForceEffect {

    static const int INFINITY = 0x7FFFFFFF; // (2^31)-1 highest possible 32-bit signed integer number

    enum {
        CONSTANT = 0, // constant effect
        RAMP,         // ramp effect
        SINE,         // periodic effect
        SQUARE,       // periodic effect
        TRIANGLE,     // periodic effect
        SAWTOOTHUP,   // periodic effect
        SAWTOOTHDOWN, // periodic effect
    };

    struct Direction {
        enum {
            CARTESIAN = 0, // cartesian direction using the x, y, and z axes
            POLAR,         // polar direction using an angle
        };

        i32 type;    // the type of the force direction encoding
        i32 dir[3];  // encoded direction parameters
    };

    // the type of the force effect
    i32 type;

    // a constant force effect
    struct {
        i32 type;                  // ForceEffect::CONSTANT
        Direction direction;       // direction of the force
        u32 length;                // effect duration
        u16 delay;                 // delay before playing the effect (again)
        u16 attack_length;
        u16 attack_level;
        u16 fade_length;
        u16 fade_level;
        i16 level;                 // effect strength
    } constant;

    // a ramp force effect
    struct {
        i32 type;                  // ForceEffect::RAMP
        Direction direction;       // direction of the force
        u32 length;                // effect duration
        u16 delay;                 // delay before playing the effect (again)
        u16 attack_length;
        u16 attack_level;
        u16 fade_length;
        u16 fade_level;
        i16 start;
        i16 end;
    } ramp;

    // a periodic force effect
    struct {
        i32 type;                  // ForceEffect::PERIODIC
        Direction direction;       // direction of the force
        u32 length;                // effect duration
        u16 delay;                 // delay before playing the effect (again)
        u16 attack_length;
        u16 attack_level;
        u16 fade_length;
        u16 fade_level;
        u16 period;
        i16 magnitude;
        i16 offset;
        u16 phase;
    } periodic;
};


#endif
