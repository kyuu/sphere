#ifndef REFIMPL_HPP
#define REFIMPL_HPP


template<class T>
class RefImpl : public T {
public:
    virtual void grab() {
        ++_count;
    }

    virtual void drop() {
        if (--_count == 0) {
            delete this;
        }
    }

protected:
    RefImpl() : _count(1) { }
    virtual ~RefImpl() { }

private:
    int _count;
};


#endif
