#include "audieresound.hpp"


//-----------------------------------------------------------------
AudiereSound*
AudiereSound::Create(audiere::SoundEffect* sound)
{
    assert(sound);
    return new AudiereSound(sound);
}

//-----------------------------------------------------------------
AudiereSound::AudiereSound(audiere::SoundEffect* sound)
: _sound(sound)
{
    _sound->ref();
}

//-----------------------------------------------------------------
AudiereSound::~AudiereSound()
{
    _sound->unref();
}

//-----------------------------------------------------------------
void
AudiereSound::play()
{
    _sound->play();
}

//-----------------------------------------------------------------
void
AudiereSound::stop()
{
    _sound->stop();
}

//-----------------------------------------------------------------
void
AudiereSound::reset()
{
    _sound->reset();
}

//-----------------------------------------------------------------
bool
AudiereSound::isPlaying() const
{
    return _sound->isPlaying();
}

//-----------------------------------------------------------------
bool
AudiereSound::isSeekable() const
{
    return _sound->isSeekable();
}

//-----------------------------------------------------------------
uint
AudiereSound::getLength() const
{
    return (uint)_sound->getLength();
}

//-----------------------------------------------------------------
bool
AudiereSound::getRepeat() const
{
    return _sound->getRepeat();
}

//-----------------------------------------------------------------
void
AudiereSound::setRepeat(bool repeat)
{
    _sound->setRepeat(repeat);
}

//-----------------------------------------------------------------
float
AudiereSound::getVolume() const
{
    return _sound->getVolume();
}

//-----------------------------------------------------------------
void
AudiereSound::setVolume(float volume)
{
    _sound->setVolume(volume);
}

//-----------------------------------------------------------------
uint
AudiereSound::getPosition() const
{
    return (uint)_sound->getPosition();
}

//-----------------------------------------------------------------
void
AudiereSound::setPosition(uint position)
{
    _sound->setPosition(position);
}
