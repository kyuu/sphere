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
    RGBA(const RGBA& that) : red(that.red), green(that.green), blue(that.blue), alpha(that.alpha) { }

    bool operator==(const RGBA& rhs) {
        return (red   == rhs.red   &&
                green == rhs.green &&
                blue  == rhs.blue  &&
                alpha == rhs.alpha);
    }

    bool operator!=(const RGBA& rhs) {
        return !(*this == rhs);
    }

    static u32 Pack(u8 red, u8 green, u8 blue, u8 alpha = 255) {
        return ((red << 24) | (green << 16) | (blue << 8) | alpha);
    }

    static u32 Pack(const RGBA& that) {
        return Pack(that.red, that.green, that.blue, that.alpha);
    }

    static RGBA Unpack(u32 packed) {
        return RGBA(packed >> 24, packed >> 16, packed >> 8, packed);
    }
};


#endif
