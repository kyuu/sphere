#ifndef DIM2_HPP
#define DIM2_HPP

#include "../common/types.hpp"


template<typename T>
struct Dim2 {
    T width;
    T height;

    Dim2() : w(0), h(0) { }
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


#endif
