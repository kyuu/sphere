#ifndef SOUNDEFFECT_HPP
#define SOUNDEFFECT_HPP

#include <audiere.h>
#include "../../common/RefImpl.hpp"
#include "../ISoundEffect.hpp"


class SoundEffect : public RefImpl<ISoundEffect> {
public:
    static SoundEffect* Create(audiere::SoundEffect* soundeffect);

    // ISoundEffect implementation
    void play();
    void stop();
    int  getVolume() const;
    void setVolume(int volume);

private:
    explicit SoundEffect(audiere::SoundEffect* soundeffect);
    ~SoundEffect();

private:
    audiere::SoundEffect* _soundeffect;
};


#endif
