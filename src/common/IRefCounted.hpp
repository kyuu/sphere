#ifndef SPHERE_IREFCOUNTED_HPP
#define SPHERE_IREFCOUNTED_HPP


namespace sphere {

    class IRefCounted {
    public:
        virtual void grab() = 0;
        virtual void drop() = 0;

    protected:
        virtual ~IRefCounted() { }
    };

}


#endif
