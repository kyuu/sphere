#ifndef IREFCOUNTED_HPP
#define IREFCOUNTED_HPP


class IRefCounted {
public:
    virtual void grab() = 0;
    virtual void drop() = 0;

protected:
    virtual ~IRefCounted() { }
};


#endif
