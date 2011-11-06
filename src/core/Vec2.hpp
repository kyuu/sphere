#ifndef VEC2_HPP
#define VEC2_HPP

#include <cmath>
#include "../common/types.hpp"


template<typename T>
struct Vec2 {
    T x;
    T y;

    Vec2() : x(0), y(0) { }
    template<typename U>
    Vec2(U x_, U y_) : x(x_), y(y_) { }
    Vec2(const Vec2& that) : x(that.x), y(that.y) { }

    Vec2& operator=(const Vec2& that) {
        x = that.x;
        y = that.y;
        return *this;
    }

    bool operator==(const Vec2& rhs) {
        return (x == rhs.x && y == rhs.y);
    }

    bool operator!=(const Vec2& rhs) {
        return !(*this == rhs);
    }

    Vec2 operator+(const Vec2& rhs) {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2& rhs) {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator*(T rhs) {
        return Vec2(x * rhs, y * rhs);
    }

    Vec2& operator+=(const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vec2& operator*=(T rhs) {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    float len() const {
        return std::sqrt(x * x + y * y);
    }

    T lenSQ() const {
        return x * x + y * y;
    }

    T dot(const Vec2& rhs) const {
        return x * rhs.x + y * rhs.y;
    }

    void null() {
        x = 0;
        y = 0;
    }

    void unit() {
        float l = len();
        x /= l;
        y /= l;
    }

    template<typename U>
    void set(U x_, U y_) {
        x = x_;
        y = y_;
    }

    void set(const Vec2& that) {
        x = that.x;
        y = that.y;
    }

    float distFrom(const Vec2& that) const {
        return Vec2(x - that.x, y - that.y).len();
    }

    T distFromSQ(const Vec2& that) const {
        return Vec2(x - that.x, y - that.y).lensq();
    }

    void rotateBy(float degrees, const Vec2& center = Vec2()) {
        degrees *= 3.1415926535 / 180.0;
        const float cs = cos(degrees);
        const float sn = sin(degrees);
        x -= center.x;
        y -= center.y;
        set((T)(x*cs - y*sn), (T)(x*sn + y*cs));
        x += center.x;
        y += center.y;
    }

    float getAngle() const {
        if (y == 0) {
            return x < 0.0 ? 180.0f : 0.0f;
        } else if (x == 0.0) {
            return y < 0.0 ? 270.0f : 90.0f;
        }
        if ( y > 0.0) {
            if (x > 0.0f) {
                return (float)(atan(y / x) * (180.0 / 3.1415926535));
            } else {
                return (float)(180.0 - atan(y / -x) * (180.0 / 3.1415926535));
            }
        } else {
            if (x > 0) {
                return (float)(360.0 - atan(-y / x) * (180.0 / 3.1415926535));
            } else {
                return (float)(180.0 + atan(-y / -x) * (180.0 / 3.1415926535));
            }
        }
    }

    float getAngleWith(const Vec2& that) const {
        float tmp = x*that.x + y*that.y;
        if (tmp == 0.0) {
            return 90.0;
        }
        tmp = tmp / sqrt((float)((x*x + y*y) * (that.x*that.x + that.y*that.y)));
        if (tmp < 0.0) {
            tmp = -tmp;
        }
        return (float)(atan(sqrt(1 - tmp*tmp) / tmp) * (180.0 / 3.1415926535));
    }

    bool isBetween(const Vec2& begin, const Vec2& end) const {
        if (begin.x != end.x) {
            return ((begin.x <= x && x <= end.x) ||
                (begin.x >= x && x >= end.x));
        } else {
            return ((begin.y <= y && y <= end.y) ||
                (begin.y >= y && y >= end.y));
        }
    }

    Vec2 getInterpolated(const Vec2& that, float d) const {
        float inv = 1.0 - d;
        return Vec2((T)(that.x*inv + x*d), (T)(that.y*inv + y*d));
    }

    void interpolate(const Vec2& a, const Vec2& b, float d)
    {
        x = (T)((float)b.x + ( ( a.x - b.x ) * d ));
        y = (T)((float)b.y + ( ( a.y - b.y ) * d ));
    }

};

typedef Vec2<i32> Vec2i;
typedef Vec2<f32> Vec2f;


#endif
