#ifndef SPHERE_DIM2_HPP
#define SPHERE_DIM2_HPP

#include "../common/types.hpp"


namespace sphere {

    template<typename T>
    struct Dim2 {
        T width;
        T height;

        Dim2() : width(0), height(0) { }
        template<typename U>
        Dim2(U w, U h) : width(w), height(h) { }
        Dim2(const Dim2& that) : width(that.width), height(that.height) { }

        Dim2& operator=(const Dim2& that) {
            width  = that.width;
            height = that.height;
            return *this;
        }

        T getArea() const {
            return width * height;
        }

    };

    typedef Dim2<u32> Dim2u;
    typedef Dim2<i32> Dim2i;
    typedef Dim2<f32> Dim2f;

} // namespace sphere


#endif
