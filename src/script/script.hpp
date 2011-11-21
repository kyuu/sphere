#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "../Log.hpp"


HSQUIRRELVM GetVM();
void EvaluateScript(const std::string& filename);
void BindRect(const Recti& rect);
Recti* GetRect(SQInteger idx);
void BindVec2(const Vec2i& vec);
Vec2i* GetVec2(SQInteger idx);
void CreateStreamDerivedClass();
void BindStream(IStream* stream);
IStream* GetStream(SQInteger idx);
void BindBlob(Blob* blob);
Blob* GetBlob(SQInteger idx);
void BindCanvas(Canvas* canvas)
Canvas* GetCanvas(SQInteger idx);
void CreateFileDerivedClass();
void BindFile(IFile* file);
IFile* GetFile(SQInteger idx);
void BindTexture(ITexture* texture);
ITexture* GetTexture(SQInteger idx);
void BindSound(ISound* sound);
ISound* GetSound(SQInteger idx);
void BindSoundEffect(ISoundEffect* soundeffect);
ISoundEffect* GetSoundEffect(SQInteger idx);
void BindForceEffect(const ForceEffect& effect);
ForceEffect* GetForceEffect(SQInteger idx);
void BindCipher(ICipher* cipher);
ICipher* GetCipher(SQInteger idx);
void BindHash(IHash* hash);
IHash* GetHash(SQInteger idx);
void BindCompressionStream(ICompressionStream* stream);
ICompressionStream* GetCompressionStream(SQInteger idx);

bool InitScript(const Log& log);
void DeinitScript(const Log& log);
void RunGame(const Log& log, const std::string& script, const std::vector<std::string>& args);


#endif
