#include <cassert>
#include "../io/filesystem.hpp"
#include "macros.hpp"
#include "util.hpp"
#include "vm.hpp"
#include "iolib.hpp"
#include "audiolib.hpp"


namespace sphere {
    namespace script {

        namespace internal {

            static SQInteger _sound_destructor(SQUserPointer p, SQInteger size);
            static SQInteger _soundeffect_destructor(SQUserPointer p, SQInteger size);

        } // namespace internal

        //-----------------------------------------------------------------
        bool BindSound(HSQUIRRELVM v, ISound* sound)
        {
            assert(sound);

            // get sound class
            sq_pushregistrytable(v);
            sq_pushstring(v, "Sound", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                return false;
            }
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_SOUND) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop sound class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_sound_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)sound);

            // grab a new reference
            sound->grab();

            return true;
        }

        //-----------------------------------------------------------------
        ISound* GetSound(HSQUIRRELVM v, SQInteger idx)
        {
            SQUserPointer p = 0;
            if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_SOUND))) {
                return (ISound*)p;
            }
            return 0;
        }

        //-----------------------------------------------------------------
        bool BindSoundEffect(HSQUIRRELVM v, ISoundEffect* soundeffect)
        {
            assert(soundeffect);

            // get soundeffect class
            sq_pushregistrytable(v);
            sq_pushstring(v, "SoundEffect", -1);
            if (!SQ_SUCCEEDED(sq_rawget(v, -2))) {
                return false;
            }
            SQUserPointer tt = 0;
            if (!SQ_SUCCEEDED(sq_gettypetag(v, -1, &tt)) || tt != TT_SOUNDEFFECT) {
                sq_poptop(v);
                return false;
            }

            // create instance
            sq_createinstance(v, -1);

            // pop soundeffect class
            sq_remove(v, -2);

            // set up instance
            sq_setreleasehook(v, -1, internal::_soundeffect_destructor);
            sq_setinstanceup(v, -1, (SQUserPointer)soundeffect);

            // grab a new reference
            soundeffect->grab();

            return true;
        }

        //-----------------------------------------------------------------
        ISoundEffect* GetSoundEffect(HSQUIRRELVM v, SQInteger idx)
        {
            SQUserPointer p = 0;
            if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, TT_SOUNDEFFECT))) {
                return (ISoundEffect*)p;
            }
            return 0;
        }

        namespace internal {

            #define SETUP_SOUND_OBJECT() \
                ISound* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUND))) { \
                    THROW_ERROR("Invalid type of environment object, expected a Sound instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _sound_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((ISound*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // Sound.FromFile(filename [, streaming = false])
            static SQInteger _sound_FromFile(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(1)
                GET_ARG_STRING(1, filename)
                GET_OPTARG_BOOL(2, streaming, SQFalse)
                FilePtr file = io::filesystem::OpenFile(filename);
                if (!file) {
                    THROW_ERROR("Could not open file")
                }
                SoundPtr sound = audio::LoadSound(file.get(), (streaming == SQTrue ? true : false));
                if (!sound) {
                    THROW_ERROR("Could not load sound")
                }
                RET_SOUND(sound.get())
            }

            //-----------------------------------------------------------------
            // Sound.FromStream(stream [, streaming = false])
            static SQInteger _sound_FromStream(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(1)
                GET_ARG_STREAM(1, stream)
                GET_OPTARG_BOOL(2, streaming, SQFalse)
                SoundPtr sound = audio::LoadSound(stream, (streaming == SQTrue ? true : false));
                if (!sound) {
                    THROW_ERROR("Could not load sound")
                }
                RET_SOUND(sound.get())
            }


            //-----------------------------------------------------------------
            // Sound.play()
            static SQInteger _sound_play(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                This->play();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Sound.stop()
            static SQInteger _sound_stop(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                This->stop();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Sound.reset()
            static SQInteger _sound_reset(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                This->reset();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Sound.isPlaying()
            static SQInteger _sound_isPlaying(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                RET_BOOL(This->isPlaying())
            }

            //-----------------------------------------------------------------
            // Sound.isSeekable()
            static SQInteger _sound_isSeekable(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                RET_BOOL(This->isSeekable())
            }

            //-----------------------------------------------------------------
            // Sound._get(index)
            static SQInteger _sound__get(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_STRING(1, index)
                if (strcmp(index, "length") == 0) {
                    RET_INT(This->getLength())
                } else if (strcmp(index, "repeat") == 0) {
                    RET_BOOL(This->getRepeat())
                } else if (strcmp(index, "volume") == 0) {
                    RET_INT(This->getVolume())
                } else if (strcmp(index, "position") == 0) {
                    RET_INT(This->getPosition())
                } else {
                    // index not found
                    sq_pushnull(v);
                    return sq_throwobject(v);
                }
            }

            //-----------------------------------------------------------------
            // Sound._set(index, value)
            static SQInteger _sound__set(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                CHECK_NARGS(2)
                GET_ARG_STRING(1, index)
                if (strcmp(index, "repeat") == 0) {
                    GET_ARG_BOOL(2, value)
                    This->setRepeat((value == SQTrue ? true : false));
                } else if (strcmp(index, "volume") == 0) {
                    GET_ARG_INT(2, value)
                    This->setVolume(value);
                } else if (strcmp(index, "position") == 0) {
                    GET_ARG_INT(2, value)
                    This->setPosition(value);
                } else {
                    // index not found
                    sq_pushnull(v);
                    return sq_throwobject(v);
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // Sound._typeof()
            static SQInteger _sound__typeof(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                RET_STRING("Sound")
            }

            //-----------------------------------------------------------------
            // Sound._tostring()
            static SQInteger _sound__tostring(HSQUIRRELVM v)
            {
                SETUP_SOUND_OBJECT()
                std::ostringstream oss;
                oss << "<Sound instance at " << This;
                oss << ")>";
                RET_STRING(oss.str().c_str())
            }

            //-----------------------------------------------------------------
            static util::Function _sound_methods[] = {
                {"play",            "Sound.play",           _sound_play           },
                {"stop",            "Sound.stop",           _sound_stop           },
                {"reset",           "Sound.reset",          _sound_reset          },
                {"isPlaying",       "Sound.isPlaying",      _sound_isPlaying      },
                {"isSeekable",      "Sound.isSeekable",     _sound_isSeekable     },
                {"_get",            "Sound._get",           _sound__get           },
                {"_set",            "Sound._set",           _sound__set           },
                {"_typeof",         "Sound._typeof",        _sound__typeof        },
                {"_tostring",       "Sound._tostring",      _sound__tostring      },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _sound_static_methods[] = {
                {"FromFile",    "Sound.FromFile",   _sound_FromFile   },
                {"FromStream",  "Sound.FromStream", _sound_FromStream },
                {0,0}
            };

            #define SETUP_SOUNDEFFECT_OBJECT() \
                ISoundEffect* This = 0; \
                if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer*)&This, TT_SOUNDEFFECT))) { \
                    THROW_ERROR("Invalid type of environment object, expected a SoundEffect instance") \
                }

            //-----------------------------------------------------------------
            static SQInteger _soundeffect_destructor(SQUserPointer p, SQInteger size)
            {
                assert(p);
                ((ISoundEffect*)p)->drop();
                return 0;
            }

            //-----------------------------------------------------------------
            // SoundEffect.FromFile(filename)
            static SQInteger _soundeffect_FromFile(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(1)
                GET_ARG_STRING(1, filename)
                FilePtr file = io::filesystem::OpenFile(filename);
                if (!file) {
                    THROW_ERROR("Could not open file")
                }
                SoundEffectPtr sound_effect = audio::LoadSoundEffect(file.get());
                if (!sound_effect) {
                    THROW_ERROR("Could not load sound")
                }
                RET_SOUNDEFFECT(sound_effect.get())
            }

            //-----------------------------------------------------------------
            // SoundEffect.FromStream(stream)
            static SQInteger _soundeffect_FromStream(HSQUIRRELVM v)
            {
                CHECK_MIN_NARGS(1)
                GET_ARG_STREAM(1, stream)
                SoundEffectPtr sound_effect = audio::LoadSoundEffect(stream);
                if (!sound_effect) {
                    THROW_ERROR("Could not load sound")
                }
                RET_SOUNDEFFECT(sound_effect.get())
            }

            //-----------------------------------------------------------------
            // SoundEffect.play()
            static SQInteger _soundeffect_play(HSQUIRRELVM v)
            {
                SETUP_SOUNDEFFECT_OBJECT()
                This->play();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // SoundEffect.stop()
            static SQInteger _soundeffect_stop(HSQUIRRELVM v)
            {
                SETUP_SOUNDEFFECT_OBJECT()
                This->stop();
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // SoundEffect._get(index)
            static SQInteger _soundeffect__get(HSQUIRRELVM v)
            {
                SETUP_SOUNDEFFECT_OBJECT()
                CHECK_NARGS(1)
                GET_ARG_STRING(1, index)
                if (strcmp(index, "volume") == 0) {
                    RET_INT(This->getVolume())
                } else {
                    // index not found
                    sq_pushnull(v);
                    return sq_throwobject(v);
                }
            }

            //-----------------------------------------------------------------
            // SoundEffect._set(index, value)
            static SQInteger _soundeffect__set(HSQUIRRELVM v)
            {
                SETUP_SOUNDEFFECT_OBJECT()
                CHECK_NARGS(2)
                GET_ARG_STRING(1, index)
                if (strcmp(index, "volume") == 0) {
                    GET_ARG_INT(2, value)
                    This->setVolume(value);
                } else {
                    // index not found
                    sq_pushnull(v);
                    return sq_throwobject(v);
                }
                RET_VOID()
            }

            //-----------------------------------------------------------------
            // SoundEffect._typeof()
            static SQInteger _soundeffect__typeof(HSQUIRRELVM v)
            {
                SETUP_SOUNDEFFECT_OBJECT()
                RET_STRING("SoundEffect")
            }

            //-----------------------------------------------------------------
            // SoundEffect._tostring()
            static SQInteger _soundeffect__tostring(HSQUIRRELVM v)
            {
                SETUP_SOUNDEFFECT_OBJECT()
                std::ostringstream oss;
                oss << "<SoundEffect instance at " << This;
                oss << ")>";
                RET_STRING(oss.str().c_str())
            }

            //-----------------------------------------------------------------
            static util::Function _soundeffect_methods[] = {
                {"play",            "SoundEffect.play",           _soundeffect_play           },
                {"stop",            "SoundEffect.stop",           _soundeffect_stop           },
                {"_get",            "SoundEffect._get",           _soundeffect__get           },
                {"_set",            "SoundEffect._set",           _soundeffect__set           },
                {"_typeof",         "SoundEffect._typeof",        _soundeffect__typeof        },
                {"_tostring",       "SoundEffect._tostring",      _soundeffect__tostring      },
                {0,0}
            };

            //-----------------------------------------------------------------
            static util::Function _soundeffect_static_methods[] = {
                {"FromFile",    "SoundEffect.FromFile",   _soundeffect_FromFile   },
                {"FromStream",  "SoundEffect.FromStream", _soundeffect_FromStream },
                {0,0}
            };

            //-----------------------------------------------------------------
            bool RegisterAudioLibrary(HSQUIRRELVM v)
            {
                /* Sound */

                // create sound class
                sq_newclass(v, SQFalse);

                // set up sound class
                sq_settypetag(v, -1, TT_SOUND);
                util::RegisterFunctions(v, _sound_methods);
                util::RegisterFunctions(v, _sound_static_methods, true);

                // register sound class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "Sound", -1);
                sq_push(v, -3); // push sound class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table

                // register sound class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "Sound", -1);
                sq_push(v, -3); // push sound class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop sound class
                sq_poptop(v);

                /* SoundEffect */

                // create soundeffect class
                sq_newclass(v, SQFalse);

                // set up soundeffect class
                sq_settypetag(v, -1, TT_SOUNDEFFECT);
                util::RegisterFunctions(v, _soundeffect_methods);
                util::RegisterFunctions(v, _soundeffect_static_methods, true);

                // register soundeffect class in registry table
                sq_pushregistrytable(v);
                sq_pushstring(v, "SoundEffect", -1);
                sq_push(v, -3); // push soundeffect class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop registry table


                // register soundeffect class in root table
                sq_pushroottable(v);
                sq_pushstring(v, "SoundEffect", -1);
                sq_push(v, -3); // push soundeffect class
                sq_newslot(v, -3, SQFalse);
                sq_poptop(v); // pop root table

                // pop soundeffect class
                sq_poptop(v);

                return true;
            }

        } // namespace internal
    } // namespace script
} // namespace sphere
