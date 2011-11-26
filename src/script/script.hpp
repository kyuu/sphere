#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include <string>
#include <vector>
#include <squirrel.h>
#include "../Log.hpp"
#include "../core/Rect.hpp"
#include "../core/Vec2.hpp"
#include "../io/IStream.hpp"
#include "../io/IFile.hpp"
#include "../io/Blob.hpp"
#include "../graphics/Canvas.hpp"
#include "../graphics/ITexture.hpp"
#include "../sound/ISound.hpp"
#include "../sound/ISoundEffect.hpp"
#include "../input/joystick.hpp"
#include "../util/ZStream.hpp"


HSQUIRRELVM GetVM();
bool        EvaluateScript(const std::string& filename);
bool        ObjectToJSON(SQInteger idx);
bool        DumpObject(SQInteger idx, IStream* stream);
bool        LoadObject(IStream* stream);
SQRESULT    ThrowError(const char* format, ...);
void        BindRect(const Recti& rect);
Recti*      GetRect(SQInteger idx);
void        BindVec2(const Vec2i& vec);
Vec2i*      GetVec2(SQInteger idx);
void        CreateStreamDerivedClass();
void        BindStream(IStream* stream);
IStream*    GetStream(SQInteger idx);
void        BindBlob(Blob* blob);
Blob*       GetBlob(SQInteger idx);
void        BindCanvas(Canvas* canvas);
Canvas*     GetCanvas(SQInteger idx);
void        CreateFileDerivedClass();
void        BindFile(IFile* file);
IFile*      GetFile(SQInteger idx);
void        BindTexture(ITexture* texture);
ITexture*   GetTexture(SQInteger idx);
void        BindSound(ISound* sound);
ISound*     GetSound(SQInteger idx);
void        BindSoundEffect(ISoundEffect* soundeffect);
ISoundEffect* GetSoundEffect(SQInteger idx);
void        BindZStream(ZStream* stream);
ZStream*    GetZStream(SQInteger idx);

bool InitScript(const Log& log);
void DeinitScript();
void RunGame(const Log& log, const std::string& script, const std::vector<std::string>& args);


#endif
