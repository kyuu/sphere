#ifndef SOUND_HPP
#define SOUND_HPP

#include <audiere.h>
#include "../../common/RefImpl.hpp"
#include "../ISound.hpp"


class Sound : public RefImpl<ISound> {
public:
    static Sound* Create(audiere::OutputStream* stream);

    // ISound implementation
    void play();
    void stop();
    void reset();
    bool isPlaying() const;
    bool isSeekable() const;
    int  getLength() const;
    bool getRepeat() const;
    void setRepeat(bool repeat);
    int  getVolume() const;
    void setVolume(int volume);
    int  getPosition() const;
    void setPosition(int position);

private:
    explicit Sound(audiere::OutputStream* stream);
    ~Sound();

private:
    audiere::OutputStream _sound;
};


#endif
