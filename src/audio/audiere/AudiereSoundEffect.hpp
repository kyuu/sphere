#ifndef AUDIERESOUNDEFFECT_HPP
#define AUDIERESOUNDEFFECT_HPP

#include <audiere.h>
#include "../../common/refimpl.hpp"
#include "../isoundeffect.hpp"


class AudiereSoundEffect : public RefImpl<ISoundEffect> {
public:
    static AudiereSoundEffect* Create(audiere::SoundEffect* soundeffect);

    // ISoundEffect implementation
    void  play();
    void  stop();
    float getVolume() const;
    void  setVolume(float volume);

private:
    explicit AudiereSoundEffect(audiere::SoundEffect* soundeffect);
    ~AudiereSoundEffect();

private:
    audiere::SoundEffect* _soundeffect;
};


#endif
