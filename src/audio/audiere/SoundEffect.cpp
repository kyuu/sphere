#include "SoundEffect.hpp"


//-----------------------------------------------------------------
SoundEffect*
SoundEffect::Create(audiere::SoundEffect* soundeffect)
{
    assert(soundeffect);
    return new SoundEffect(soundeffect);
}

//-----------------------------------------------------------------
SoundEffect::SoundEffect(audiere::SoundEffect* soundeffect)
: _soundeffect(soundeffect)
{
    _soundeffect->ref();
}

//-----------------------------------------------------------------
SoundEffect::~SoundEffect()
{
    _soundeffect->unref();
}

//-----------------------------------------------------------------
void
SoundEffect::play()
{
    _soundeffect->play();
}

//-----------------------------------------------------------------
void
SoundEffect::stop()
{
    _soundeffect->stop();
}

//-----------------------------------------------------------------
int
SoundEffect::getVolume() const
{
    return (int)(_soundeffect->getVolume() * 100);
}

//-----------------------------------------------------------------
void
SoundEffect::setVolume(int volume)
{
    _soundeffect->setVolume((float)volume / 100);
}
