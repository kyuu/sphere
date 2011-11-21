#ifndef SOUNDEFFECT_HPP
#define SOUNDEFFECT_HPP

#include <audiere.h>
#include "../../common/RefImpl.hpp"
#include "../ISoundEffect.hpp"


class AudiereSoundEffect : public RefImpl<ISoundEffect> {
public:
    static AudiereSoundEffect* Create(audiere::SoundEffect* soundeffect);

    // ISoundEffect implementation
    void play();
    void stop();
    int  getVolume() const;
    void setVolume(int volume);

private:
    explicit AudiereSoundEffect(audiere::SoundEffect* soundeffect);
    ~AudiereSoundEffect();

private:
    audiere::SoundEffect* _soundeffect;
};


#endif
