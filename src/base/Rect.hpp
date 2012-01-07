#ifndef SPHERE_RECT_HPP
#define SPHERE_RECT_HPP

#include "../common/types.hpp"
#include "Vec2.hpp"


namespace sphere {

    template<typename T>
    struct Rect {
        Vec2<T> ul;
        Vec2<T> lr;

        Rect() { }

        Rect(const Rect& that) : ul(that.ul), lr(that.lr) { }

        template<typename U>
        Rect(U x1, U y1, U x2, U y2) : ul(x1, y1), lr(x2, y2) { }

        Rect(const Vec2<T>& ul_, const Vec2<T>& lr_) : ul(ul_), lr(lr_) { }

        Rect& operator=(const Rect& that) {
            ul = that.ul;
            lr = that.lr;
            return *this;
        }

        bool operator==(const Rect& that) {
            return (ul == that.ul && lr == that.lr);
        }

        bool operator!=(const Rect& that) {
            return !(*this == that);
        }

        T getX() const {
            return ul.x;
        }

        T getY() const {
            return ul.y;
        }

        T getWidth() const {
            return (lr.x - ul.x) + 1;
        }

        T getHeight() const {
            return (lr.y - ul.y) + 1;
        }

        bool isValid() const {
            return ul.x <= lr.x && ul.y <= lr.y;
        }

        bool intersects(const Rect& that) const
        {
            return (lr.y > that.ul.y &&
                    ul.y < that.lr.y &&
                    lr.x > that.ul.x &&
                    ul.x < that.lr.x);
        }

        Rect getIntersection(const Rect& that) const {
            if (!isValid() || !that.isValid()) {
                return Rect();
            }
            if (this == &that) {
                return *this;
            }
            if (ul.x > that.lr.x ||
                ul.y > that.lr.y ||
                lr.x < that.ul.x ||
                lr.y < that.ul.y)
            {
                return Rect();
            }
            T x1 = ul.x;
            T y1 = ul.y;
            T x2 = lr.x;
            T y2 = lr.y;
            if (x1 < that.ul.x) {
                x1 = that.ul.x;
            }
            if (x2 > that.lr.x) {
                x2 = that.lr.x;
            }
            if (y1 < that.ul.y) {
                y1 = that.ul.y;
            }
            if (y2 > that.lr.y) {
                y2 = that.lr.y;
            }
            return Rect(x1, y1, x2, y2);
        }

        bool contains(T x, T y) const {
            return (ul.x <= x &&
                    ul.y <= y &&
                    lr.x >= x &&
                    lr.y >= y);
        }

        bool contains(const Vec2<T>& p) const {
            return contains(p.x, p.y);
        }

        bool contains(const Rect<T>& that) const {
            return ((that.ul.x >= ul.x && that.ul.x <= lr.x) &&
                    (that.lr.x >= ul.x && that.lr.x <= lr.x) &&
                    (that.ul.y >= ul.y && that.ul.y <= lr.y) &&
                    (that.lr.y >= ul.y && that.lr.y <= lr.y));
        }

    };

    typedef Rect<i32> Recti;
    typedef Rect<f32> Rectf;

} // namespace sphere


#endif
