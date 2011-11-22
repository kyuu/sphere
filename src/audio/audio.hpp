#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "../Log.hpp"
#include "../io/IStream.hpp"
#include "ISound.hpp"
#include "ISoundEffect.hpp"


ISound*       LoadSound(IStream* stream, bool streaming = false);
ISoundEffect* LoadSoundEffect(IStream* stream);

bool InitAudio(const Log& log);
void DeinitAudio();


#endif
