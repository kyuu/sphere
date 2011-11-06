#ifndef ARRAYPTR_HPP
#define ARRAYPTR_HPP

#include <cassert>


template<class T>
class ArrayPtr {
public:
    ArrayPtr(T* t = 0) : _t(t) {
    }

    ArrayPtr(const ArrayPtr<T>& that) : _t(that._t) {
        that._t = 0;
    }

    ~ArrayPtr() {
        if (_t) {
            delete[] _t;
        }
    }

    ArrayPtr<T>& operator=(T* t) {
        if (_t != t) {
            reset(t);
        }
        return *this;
    }

    ArrayPtr<T>& operator=(const ArrayPtr<T>& that) {
        if (this != &that) {
            reset(that._t);
            that._t = 0;
        }
        return *this;
    }

    bool operator==(const ArrayPtr<T>& that) {
        return _t == that._t;
    }

    bool operator!=(const ArrayPtr<T>& that) {
        return _t != that._t;
    }

    bool operator!() const {
        return _t == 0;
    }

    operator bool() const {
        return _t != 0;
    }

    T& operator[](size_t n) {
        assert(_t);
        return _t[n];
    }

    T* get() const {
        assert(_t);
        return _t;
    }

    T* release() {
        T* t = _t;
        _t = 0;
        return t;
    }

    void reset(T* t = 0) {
        if (_t) {
            delete[] _t;
        }
        _t = t;
    }

private:
    T* _t;
    int _size;
};


#endif
