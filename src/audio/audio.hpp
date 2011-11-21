#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <string>
#include "../Log.hpp"
#include "ISound.hpp"
#include "ISoundEffect.hpp"


ISound* LoadSound(IStream* stream, bool streaming = false);
ISoundEffect* LoadSoundEffect(IStream* stream);

bool InitAudio(const Log& log);
void DeinitAudio(const Log& log);


#endif
