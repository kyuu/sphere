#ifndef SPHERE_AUDIO_HPP
#define SPHERE_AUDIO_HPP

#include "../Log.hpp"
#include "../io/IStream.hpp"
#include "ISound.hpp"
#include "ISoundEffect.hpp"


namespace sphere {
    namespace audio {

        ISound*       LoadSound(IStream* stream, bool streaming = false);
        ISoundEffect* LoadSoundEffect(IStream* stream);

        namespace internal {

            bool InitAudio(const Log& log);
            void DeinitAudio();

        } // namespace internal
    } // namespace audio
} // namespace sphere


#endif
