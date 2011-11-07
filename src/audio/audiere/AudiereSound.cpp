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
int
AudiereSound::getLength() const
{
    return _sound->getLength();
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
int
AudiereSound::getVolume() const
{
    return (int)(_sound->getVolume() * 100);
}

//-----------------------------------------------------------------
void
AudiereSound::setVolume(int volume)
{
    _sound->setVolume((float)volume / 100);
}

//-----------------------------------------------------------------
int
AudiereSound::getPosition() const
{
    return _sound->getPosition();
}

//-----------------------------------------------------------------
void
AudiereSound::setPosition(int position)
{
    _sound->setPosition(position);
}
