#ifndef RGBA_HPP
#define RGBA_HPP

#include "../common/types.hpp"


struct RGBA {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;

    RGBA() : r(0), g(0), b(0), a(255) { }
    RGBA(u8 r_, u8 g_, u8 b_, u8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) { }
    RGBA(const RGBA& c) : r(c.r), g(c.g), b(c.b), a(c.a) { }

    bool operator==(const RGBA& c) {
        return (r == c.r &&
                g == c.g &&
                b == c.b &&
                a == c.a);
    }

    bool operator!=(const RGBA& rhs) {
        return !(*this == rhs);
    }

};


#endif
