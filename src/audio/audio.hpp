#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <string>
#include "ISound.hpp"
#include "ISoundEffect.hpp"


namespace audio {

    // TODO: add possibility to create sounds from memory

    ISound*       LoadSound(IStream* stream, bool streaming = false);
    ISoundEffect* LoadSoundEffect(IStream* stream, ISoundEffect::Type type = ISoundEffect::SINGLE);

    namespace internal {

        bool InitAudio(const Log& log);
        void DeinitAudio(const Log& log);

    } // namespace internal

} // namespace audio


#endif
