#include "audieresoundeffect.hpp"


//-----------------------------------------------------------------
AudiereSoundEffect*
AudiereSoundEffect::Create(audiere::SoundEffect* soundeffect)
{
    assert(soundeffect);
    return new AudiereSoundEffect(soundeffect);
}

//-----------------------------------------------------------------
AudiereSoundEffect::AudiereSoundEffect(audiere::SoundEffect* soundeffect)
: _soundeffect(soundeffect)
{
    _soundeffect->ref();
}

//-----------------------------------------------------------------
AudiereSoundEffect::~AudiereSoundEffect()
{
    _soundeffect->unref();
}

//-----------------------------------------------------------------
void
AudiereSoundEffect::play()
{
    _soundeffect->play();
}

//-----------------------------------------------------------------
void
AudiereSoundEffect::stop()
{
    _soundeffect->stop();
}

//-----------------------------------------------------------------
float
AudiereSoundEffect::getVolume() const
{
    return _soundeffect->getVolume();
}

//-----------------------------------------------------------------
void
AudiereSoundEffect::setVolume(float volume)
{
    _soundeffect->setVolume(volume);
}
