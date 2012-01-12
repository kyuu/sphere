#include <string>
#include <vector>
#include <sstream>
#include <audiere.h>
#include "../audio.hpp"
#include "Sound.hpp"
#include "SoundEffect.hpp"


namespace sphere {
    namespace audio {

        //-----------------------------------------------------------------
        struct AudiereFileAdapter : public audiere::RefImplementation<audiere::File>
        {
            AudiereFileAdapter(IStream* s) : _s(0) {
                assert(s);
                s->grab();
                _s = s;
            }

            ~AudiereFileAdapter() {
                _s->drop();
            }

            ADR_METHOD(int) read(void* buffer, int size) {
                return _s->read(buffer, size);
            }

            ADR_METHOD(bool) seek(int pos, audiere::File::SeekMode mode) {
                switch (mode) {
                case audiere::File::BEGIN:   return _s->seek(pos, IStream::BEG);
                case audiere::File::CURRENT: return _s->seek(pos, IStream::CUR);
                case audiere::File::END:     return _s->seek(pos, IStream::END);
                default: return false;
                }
            }

            ADR_METHOD(int) tell() {
                return _s->tell();
            }

        private:
            IStream* _s;
        };

        //-----------------------------------------------------------------
        // globals
        static audiere::AudioDevice* g_AudioDevice = 0;

        //-----------------------------------------------------------------
        ISound* LoadSound(IStream* stream, bool streaming)
        {
            assert(stream);
            assert(stream->isReadable());
            audiere::FilePtr file_adapter(new AudiereFileAdapter(stream));
            audiere::OutputStream* sound = audiere::OpenSound(g_AudioDevice, file_adapter, streaming);
            if (!sound) {
                return 0;
            }
            return Sound::Create(sound);
        }

        //-----------------------------------------------------------------
        ISoundEffect* LoadSoundEffect(IStream* stream)
        {
            assert(stream);
            assert(stream->isReadable());
            audiere::FilePtr file_adapter(new AudiereFileAdapter(stream));
            audiere::SoundEffect* soundeffect = audiere::OpenSoundEffect(g_AudioDevice, file_adapter, audiere::MULTIPLE);
            if (!soundeffect) {
                return 0;
            }
            return SoundEffect::Create(soundeffect);
        }

        namespace internal {

            //-----------------------------------------------------------------
            bool InitAudio(const Log& log)
            {
                assert(!g_AudioDevice);

                // see if there are any audio devices available
                std::vector<audiere::AudioDeviceDesc> devices;
                audiere::GetSupportedAudioDevices(devices);
                if (devices.empty()) {
                    log.error() << "No available audio devices";
                    return false;
                } else {
                    for (size_t i = 0; i < devices.size(); i++) {
                        log.info() << "Available audio device: " \
                                   << devices[i].name \
                                   << " (" \
                                   << devices[i].description \
                                   << ")";
                    }
                }

                // see if audiere supports any audio file format
                std::vector<audiere::FileFormatDesc> formats;
                audiere::GetSupportedFileFormats(formats);
                if (formats.empty()) {
                    log.error() << "No supported audio file formats";
                    return false;
                } else {
                    for (size_t i = 0; i < formats.size(); i++) {
                        std::ostringstream extensions;
                        for (size_t j = 0; j < formats[i].extensions.size(); j++) {
                            extensions << formats[i].extensions[j];
                            if (j != formats[i].extensions.size()-1) {
                                extensions << ", ";
                            }
                        }
                        log.info() << "Supported file format: " \
                                   << extensions.str() \
                                   << " (" \
                                   << formats[i].description \
                                   << ")";
                    }
                }

                // open default audio device
                g_AudioDevice = audiere::OpenDevice(0);
                if (!g_AudioDevice) {
                    log.error() << "Could not open default audio device";
                    return false;
                }
                g_AudioDevice->ref();

                log.info() << "Opened default audio device: " << g_AudioDevice->getName();

                return true;
            }

            //-----------------------------------------------------------------
            void DeinitAudio()
            {
                if (g_AudioDevice) {
                    g_AudioDevice->unref();
                    g_AudioDevice = 0;
                }
            }

        } // namespace internal
    } // namespace audio
} // namespace sphere
