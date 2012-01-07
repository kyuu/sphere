#ifndef SPHERE_REFPTR_HPP
#define SPHERE_REFPTR_HPP

#include <cassert>
#include <cstddef>


namespace sphere {

    template<class T>
    class RefPtr {
        template<class S> friend class RefPtr;
    public:
        RefPtr() : _t(0) { }

        RefPtr(T* t) : _t(t) {
        }

        RefPtr(const RefPtr<T>& that) : _t(that._t) {
            if (_t) {
                _t->grab();
            }
        }

        template <class S>
        RefPtr(const RefPtr<S>& s) : _t(0) {
            _t = static_cast<T*>(s._t);
            if (_t) {
                _t->grab();
            }
        }

        ~RefPtr() {
            if (_t) {
                _t->drop();
            }
        }

        RefPtr<T>& operator=(T* t) {
            if (t != _t) {
                if (_t) {
                    _t->drop();
                }
                _t = t;
            }
            return *this;
        }

        RefPtr<T>& operator=(const RefPtr<T>& that) {
            if (this != &that) {
                if (_t) {
                    _t->drop();
                }
                _t = that._t;
                if (_t) {
                    _t->grab();
                }
            }
            return *this;
        }

        bool operator==(const RefPtr<T>& ptr) {
            return _t == ptr._t;
        }

        bool operator!=(const RefPtr<T>& ptr) {
            return _t != ptr._t;
        }

        T* operator->() const {
            assert(_t);
            return _t;
        }

        bool operator!() const {
            return _t == 0;
        }

        operator bool() const {
            return _t != 0;
        }

        T* get() const {
            return _t;
        }

        void reset(T* t = 0) {
            if (_t) {
                _t->drop();
            }
            _t = t;
        }

        T* release() {
            T* t = _t;
            _t = 0;
            return t;
        }

    private:
        // forbid heap allocation
        void* operator new(size_t);

    private:
        T* _t;
    };

}


#endif
