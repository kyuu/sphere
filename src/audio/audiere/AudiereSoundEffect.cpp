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
int
AudiereSoundEffect::getVolume() const
{
    return (int)(_soundeffect->getVolume() * 100);
}

//-----------------------------------------------------------------
void
AudiereSoundEffect::setVolume(int volume)
{
    _soundeffect->setVolume((float)volume / 100);
}
