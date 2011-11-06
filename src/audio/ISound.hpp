#ifndef ISOUND_HPP
#define ISOUND_HPP

#include "../common/refptr.hpp"
#include "../common/irefcounted.hpp"
#include "../common/types.hpp"


class ISound : public IRefCounted {
public:
    virtual void  play() = 0;
    virtual void  stop() = 0;
    virtual void  reset() = 0;
    virtual bool  isPlaying() const = 0;
    virtual bool  isSeekable() const = 0;
    virtual uint  getLength() const = 0;
    virtual bool  getRepeat() const = 0;
    virtual void  setRepeat(bool repeat) = 0;
    virtual float getVolume() const = 0;
    virtual void  setVolume(float volume) = 0;
    virtual uint  getPosition() const = 0;
    virtual void  setPosition(uint position) = 0;

protected:
    ~ISound() { }
};

typedef RefPtr<ISound> SoundPtr;


#endif
