#ifndef SPHERE_SCRIPT_AUDIOLIB_HPP
#define SPHERE_SCRIPT_AUDIOLIB_HPP

#include <squirrel.h>
#include "../audio/audio.hpp"

// type tags
#define TT_SOUND       ((SQUserPointer)500)
#define TT_SOUNDEFFECT ((SQUserPointer)501)


namespace sphere {
    namespace script {

        bool    BindSound(HSQUIRRELVM v, ISound* sound);
        ISound* GetSound(HSQUIRRELVM v, SQInteger idx);

        bool          BindSoundEffect(HSQUIRRELVM v, ISoundEffect* soundeffect);
        ISoundEffect* GetSoundEffect(HSQUIRRELVM v, SQInteger idx);

        namespace internal {

            bool RegisterAudioLibrary(HSQUIRRELVM v);

        } // namespace internal
    } // namespace script
} // namespace sphere


#endif
