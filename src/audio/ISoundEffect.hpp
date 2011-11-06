#ifndef ISOUNDEFFECT_HPP
#define ISOUNDEFFECT_HPP

#include "../common/refptr.hpp"
#include "../common/irefcounted.hpp"


class ISoundEffect : public IRefCounted {
public:
    virtual void  play() = 0;
    virtual void  stop() = 0;
    virtual float getVolume() const = 0;
    virtual void  setVolume(float volume) = 0;

protected:
    ~ISoundEffect() { }
};

typedef RefPtr<ISoundEffect> SoundEffectPtr;


#endif
