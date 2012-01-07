#ifndef SPHERE_ISOUND_HPP
#define SPHERE_ISOUND_HPP

#include "../common/IRefCounted.hpp"
#include "../common/RefPtr.hpp"


namespace sphere {

    class ISound : public IRefCounted {
    public:
        virtual void play() = 0;
        virtual void stop() = 0;
        virtual void reset() = 0;
        virtual bool isPlaying() const = 0;
        virtual bool isSeekable() const = 0;
        virtual int  getLength() const = 0;
        virtual bool getRepeat() const = 0;
        virtual void setRepeat(bool repeat) = 0;
        virtual int  getVolume() const = 0;
        virtual void setVolume(int volume) = 0;
        virtual int  getPosition() const = 0;
        virtual void setPosition(int position) = 0;

    protected:
        ~ISound() { }
    };

    typedef RefPtr<ISound> SoundPtr;

} // namespace sphere


#endif
