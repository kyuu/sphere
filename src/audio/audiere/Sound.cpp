#include "Sound.hpp"


//-----------------------------------------------------------------
Sound*
Sound::Create(audiere::OutputStream* sound)
{
    assert(sound);
    return new Sound(sound);
}

//-----------------------------------------------------------------
Sound::Sound(audiere::OutputStream* sound)
    : _sound(sound)
{
    _sound->ref();
}

//-----------------------------------------------------------------
Sound::~Sound()
{
    _sound->unref();
}

//-----------------------------------------------------------------
void
Sound::play()
{
    _sound->play();
}

//-----------------------------------------------------------------
void
Sound::stop()
{
    _sound->stop();
}

//-----------------------------------------------------------------
void
Sound::reset()
{
    _sound->reset();
}

//-----------------------------------------------------------------
bool
Sound::isPlaying() const
{
    return _sound->isPlaying();
}

//-----------------------------------------------------------------
bool
Sound::isSeekable() const
{
    return _sound->isSeekable();
}

//-----------------------------------------------------------------
int
Sound::getLength() const
{
    return _sound->getLength();
}

//-----------------------------------------------------------------
bool
Sound::getRepeat() const
{
    return _sound->getRepeat();
}

//-----------------------------------------------------------------
void
Sound::setRepeat(bool repeat)
{
    _sound->setRepeat(repeat);
}

//-----------------------------------------------------------------
int
Sound::getVolume() const
{
    return (int)(_sound->getVolume() * 100);
}

//-----------------------------------------------------------------
void
Sound::setVolume(int volume)
{
    _sound->setVolume((float)volume / 100);
}

//-----------------------------------------------------------------
int
Sound::getPosition() const
{
    return _sound->getPosition();
}

//-----------------------------------------------------------------
void
Sound::setPosition(int position)
{
    _sound->setPosition(position);
}
