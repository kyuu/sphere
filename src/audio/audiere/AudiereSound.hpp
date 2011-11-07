#ifndef AUDIERESOUND_HPP
#define AUDIERESOUND_HPP

#include <audiere.h>
#include "../../common/refimpl.hpp"
#include "../isound.hpp"


class AudiereSound : public RefImpl<ISound> {
public:
    static AudiereSound* Create(audiere::OutputStream* stream);

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
    explicit AudiereSound(audiere::OutputStream* stream);
    ~AudiereSound();

private:
    audiere::OutputStream _sound;
};


#endif
