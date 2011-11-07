#include <audiere.h>
#include "../../Log.hpp"
#include "../audio.hpp"
#include "AudiereSound.hpp"
#include "AudiereSoundEffect.hpp"


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
static audiere::AudioDevice* g_audio_device = 0;

namespace audio {

    //-----------------------------------------------------------------
    ISound* LoadSound(IStream* stream, bool streaming)
    {
        if (!stream || !stream->isReadable()) {
            return 0;
        }
        audiere::FilePtr file_adapter(new AudiereFileAdapter(stream));
        audiere::OutputStream* sound = audiere::OpenSound(g_audio_device, file_adapter, streaming);
        if (sound) {
            return AudiereSound::Create(sound);
        }
        return 0;
    }

    //-----------------------------------------------------------------
    ISoundEffect* LoadSoundEffect(IStream* stream)
    {
        if (!stream || !stream->isReadable()) {
            return 0;
        }
        audiere::FilePtr file_adapter(new AudiereFileAdapter(stream));
        audiere::SoundEffect* soundeffect = audiere::OpenSoundEffect(g_audio_device, file_adapter, audiere::MULTIPLE);
        if (soundeffect) {
            return AudiereSoundEffect::Create(soundeffect);
        }
        return 0;
    }

    namespace internal {

        //-----------------------------------------------------------------
        bool InitAudio(const Log& log)
        {
            assert(!g_audio_device);
            log.info() << "Using Audiere " << audiere::GetVersion();
            std::vector<audiere::AudioDeviceDesc> device_list;
            audiere::GetSupportedAudioDevices(device_list);
            if (device_list.empty()) {
                log.error() << "Audiere does not support any audio device";
                return false;
            }
            for (size_t i = 0; i < device_list.size(); i++) {
                log.info() << "Supported audio device: " << device_list[i].name << " (" << device_list[i].description << ")";
            }
            std::vector<audiere::FileFormatDesc> format_list;
            audiere::GetSupportedFileFormats(format_list);
            if (format_list.empty()) {
                log.error() << "Audiere does not support any file format";
                return false;
            }
            for (size_t i = 0; i < format_list.size(); i++) {
                std::ostringstream ff_ext;
                for (size_t j = 0; j < format_list[i].extensions.size(); j++) {
                    ff_ext << format_list[i].extensions[j];
                    if (j < format_list[i].extensions.size() - 1) {
                        ff_ext << ", ";
                    }
                }
                log.info() << "Supported file format: " << ff_ext << " (" << format_list[i].description << ")";
            }
            log.info() << "Opening default audio device";
            g_audio_device = audiere::OpenDevice(0);
            if (!g_audio_device) {
                log.error() << "Could not open default audio device";
                return false;
            }
            g_audio_device->ref();
            log.info() << "Opened the '" << g_audio_device->getName() << "' audio device";
            return true;
        }

        //-----------------------------------------------------------------
        void DeinitAudio(const Log& log)
        {
            if (g_audio_device) {
                log.info() << "Closing the '" << g_audio_device->getName() << "' audio device";
                g_audio_device->unref();
                g_audio_device = 0;
            }
        }

    } // namespace internal

} // namespace audio
