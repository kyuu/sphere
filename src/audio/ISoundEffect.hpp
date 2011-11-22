#ifndef ISOUNDEFFECT_HPP
#define ISOUNDEFFECT_HPP

#include "../common/IRefCounted.hpp"
#include "../common/RefPtr.hpp"


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


#endif
