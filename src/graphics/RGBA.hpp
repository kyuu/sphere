#ifndef RGBA_HPP
#define RGBA_HPP

#include "../common/types.hpp"


struct RGBA {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;

    RGBA() : red(0), green(0), blue(0), alpha(255) { }
    RGBA(u8 r, u8 g, u8 b, u8 a = 255) : red(r), green(g), blue(b), alpha(a) { }
    RGBA(const RGBA& c) : red(c.red), green(c.green), blue(c.blue), alpha(c.alpha) { }

    bool operator==(const RGBA& c) {
        return (r == c.r &&
                g == c.g &&
                b == c.b &&
                a == c.a);
    }

    bool operator!=(const RGBA& rhs) {
        return !(*this == rhs);
    }

    static u32 Pack(u8 red, u8 green, u8 blue, u8 alpha = 255) {
        return ((red << 24) | (green << 16) | (blue << 8) | alpha);
    }

    static RGBA Unpack(u32 packed) {
        return RGBA(packed >> 24, packed >> 16, packed >> 8, packed);
    }
};


#endif
