#ifndef SPHERE_ISOUNDEFFECT_HPP
#define SPHERE_ISOUNDEFFECT_HPP

#include "../common/IRefCounted.hpp"
#include "../common/RefPtr.hpp"


namespace sphere {

    class ISoundEffect : public IRefCounted {
    public:
        virtual void play() = 0;
        virtual void stop() = 0;
        virtual int  getVolume() const = 0;
        virtual void setVolume(int volume) = 0;

    protected:
        ~ISoundEffect() { }
    };

    typedef RefPtr<ISoundEffect> SoundEffectPtr;

} // namespace sphere


#endif
