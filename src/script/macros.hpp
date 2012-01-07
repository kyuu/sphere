#ifndef SPHERE_SCRIPT_MACROS_HPP
#define SPHERE_SCRIPT_MACROS_HPP


#define THROW_ERROR(msg)                                return ThrowError(msg);
#define THROW_ERROR1(msg, arg1)                         return ThrowError(msg, arg1);
#define THROW_ERROR2(msg, arg1, arg2)                   return ThrowError(msg, arg1, arg2);
#define THROW_ERROR3(msg, arg1, arg2, arg3)             return ThrowError(msg, arg1, arg2, arg3);
#define THROW_ERROR4(msg, arg1, arg2, arg3, arg4)       return ThrowError(msg, arg1, arg2, arg3, arg4);
#define THROW_ERROR5(msg, arg1, arg2, arg3, arg4, arg5) return ThrowError(msg, arg1, arg2, arg3, arg4, arg5);

#define NARGS() (sq_gettop(v) - 1)

#define CHECK_NARGS(n)      if (sq_gettop(v) != n + 1) { return ThrowError("Invalid number of arguments (%d), expected %d", NARGS(), n); }
#define CHECK_MIN_NARGS(n)  if (sq_gettop(v)  < n + 1) { return ThrowError("Invalid number of arguments (%d), expected at least %d", NARGS(), n); }

#define ARG_IS_NULL(idx)    (sq_gettype(v, idx + 1) == OT_NULL)
#define ARG_IS_BOOL(idx)    (sq_gettype(v, idx + 1) == OT_BOOL)
#define ARG_IS_INT(idx)     (sq_gettype(v, idx + 1) == OT_INTEGER)
#define ARG_IS_FLOAT(idx)   (sq_gettype(v, idx + 1) == OT_FLOAT)
#define ARG_IS_NUMERIC(idx) ((sq_gettype(v, idx + 1) & SQOBJECT_NUMERIC) != 0)
#define ARG_IS_STRING(idx)  (sq_gettype(v, idx + 1) == OT_STRING)

#define GET_ARG_BOOL(idx, name)                 SQBool           name;       if (SQ_FAILED(sq_getbool(v, idx + 1, &name)))       { return ThrowError("Invalid argument %d '%s', expected a bool",    idx, #name); }
#define GET_ARG_INT(idx, name)                  SQInteger        name;       if (SQ_FAILED(sq_getinteger(v, idx + 1, &name)))    { return ThrowError("Invalid argument %d '%s', expected a integer", idx, #name); }
#define GET_ARG_FLOAT(idx, name)                SQFloat          name;       if (SQ_FAILED(sq_getfloat(v, idx + 1, &name)))      { return ThrowError("Invalid argument %d '%s', expected a float",   idx, #name); }
#define GET_ARG_STRING(idx, name)               const SQChar*    name = 0;   if (SQ_FAILED(sq_getstring(v, idx + 1, &name)))     { return ThrowError("Invalid argument %d '%s', expected a string",  idx, #name); }
#define GET_ARG_RECT(idx, name)                 Recti*           name = GetRect(v, idx + 1);        if (!name) { return ThrowError("Invalid argument %d '%s', expected a Rect instance",        idx, #name); }
#define GET_ARG_VEC2(idx, name)                 Vec2f*           name = GetVec2(v, idx + 1);        if (!name) { return ThrowError("Invalid argument %d '%s', expected a Vec2 instance",        idx, #name); }
#define GET_ARG_BLOB(idx, name)                 Blob*            name = GetBlob(v, idx + 1);        if (!name) { return ThrowError("Invalid argument %d '%s', expected a Blob instance",        idx, #name); }
#define GET_ARG_CANVAS(idx, name)               Canvas*          name = GetCanvas(v, idx + 1);      if (!name) { return ThrowError("Invalid argument %d '%s', expected a Canvas instance",      idx, #name); }
#define GET_ARG_ZSTREAM(idx, name)              ZStream*         name = GetZStream(v, idx + 1);     if (!name) { return ThrowError("Invalid argument %d '%s', expected a ZStream instance",     idx, #name); }
#define GET_ARG_STREAM(idx, name)               IStream*         name = GetStream(v, idx + 1);      if (!name) { return ThrowError("Invalid argument %d '%s', expected a Stream instance",      idx, #name); }
#define GET_ARG_FILE(idx, name)                 IFile*           name = GetFile(v, idx + 1);        if (!name) { return ThrowError("Invalid argument %d '%s', expected a File instance",        idx, #name); }
#define GET_ARG_TEXTURE(idx, name)              ITexture*        name = GetTexture(v, idx + 1);     if (!name) { return ThrowError("Invalid argument %d '%s', expected a Texture instance",     idx, #name); }
#define GET_ARG_SOUND(idx, name)                ISound*          name = GetSound(v, idx + 1);       if (!name) { return ThrowError("Invalid argument %d '%s', expected a Sound instance",       idx, #name); }
#define GET_ARG_SOUNDEFFECT(idx, name)          ISoundEffect*    name = GetSoundEffect(v, idx + 1); if (!name) { return ThrowError("Invalid argument %d '%s', expected a SoundEffect instance", idx, #name); }

#define GET_OPTARG_BOOL(idx, name, defval)      SQBool           name = defval;  if (sq_gettop(v) >= idx + 1) { if (SQ_FAILED(sq_getbool(v, idx + 1, &name)))    { return ThrowError("Invalid argument %d '%s', expected a bool",    idx, #name); } }
#define GET_OPTARG_INT(idx, name, defval)       SQInteger        name = defval;  if (sq_gettop(v) >= idx + 1) { if (SQ_FAILED(sq_getinteger(v, idx + 1, &name))) { return ThrowError("Invalid argument %d '%s', expected a integer", idx, #name); } }
#define GET_OPTARG_FLOAT(idx, name, defval)     SQFloat          name = defval;  if (sq_gettop(v) >= idx + 1) { if (SQ_FAILED(sq_getfloat(v, idx + 1, &name)))   { return ThrowError("Invalid argument %d '%s', expected a float",   idx, #name); } }
#define GET_OPTARG_STRING(idx, name, defval)    const SQChar*    name = defval;  if (sq_gettop(v) >= idx + 1) { if (SQ_FAILED(sq_getstring(v, idx + 1, &name)))  { return ThrowError("Invalid argument %d '%s', expected a string",  idx, #name); } }
#define GET_OPTARG_RECT(idx, name)              Recti*           name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetRect(v, idx + 1);         if (!name) { return ThrowError("Invalid argument %d '%s', expected a Vec2 instance",        idx, #name); } }
#define GET_OPTARG_VEC2(idx, name)              Vec2f*           name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetVec2(v, idx + 1);         if (!name) { return ThrowError("Invalid argument %d '%s', expected a Canvas instance",      idx, #name); } }
#define GET_OPTARG_BLOB(idx, name)              Blob*            name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetBlob(v, idx + 1);         if (!name) { return ThrowError("Invalid argument %d '%s', expected a File instance",        idx, #name); } }
#define GET_OPTARG_CANVAS(idx, name)            Canvas*          name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetCanvas(v, idx + 1);       if (!name) { return ThrowError("Invalid argument %d '%s', expected a Rect instance",        idx, #name); } }
#define GET_OPTARG_ZSTREAM(idx, name)           ZStream*         name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetZStream(v, idx + 1);      if (!name) { return ThrowError("Invalid argument %d '%s', expected a ZStream instance",     idx, #name); } }
#define GET_OPTARG_STREAM(idx, name)            IStream*         name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetStream(v, idx + 1);       if (!name) { return ThrowError("Invalid argument %d '%s', expected a Stream instance",      idx, #name); } }
#define GET_OPTARG_FILE(idx, name)              IFile*           name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetFile(v, idx + 1);         if (!name) { return ThrowError("Invalid argument %d '%s', expected a Blob instance",        idx, #name); } }
#define GET_OPTARG_TEXTURE(idx, name)           ITexture*        name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetTexture(v, idx + 1);      if (!name) { return ThrowError("Invalid argument %d '%s', expected a Texture instance",     idx, #name); } }
#define GET_OPTARG_SOUND(idx, name)             ISound*          name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetSound(v, idx + 1);        if (!name) { return ThrowError("Invalid argument %d '%s', expected a Sound instance",       idx, #name); } }
#define GET_OPTARG_SOUNDEFFECT(idx, name)       ISoundEffect*    name = 0;       if (sq_gettop(v) >= idx + 1) { name = GetSoundEffect(v, idx + 1);  if (!name) { return ThrowError("Invalid argument %d '%s', expected a SoundEffect instance", idx, #name); } }

#define RET_VOID()                                              return 0;
#define RET_NULL()              sq_pushnull(v);                 return 1;
#define RET_TRUE(expr)          sq_pushbool(v, SQTrue);         return 1;
#define RET_FALSE(expr)         sq_pushbool(v, SQFalse);        return 1;
#define RET_BOOL(expr)          sq_pushbool(v, expr);           return 1;
#define RET_INT(expr)           sq_pushinteger(v, expr);        return 1;
#define RET_FLOAT(expr)         sq_pushfloat(v, expr);          return 1;
#define RET_STRING(expr)        sq_pushstring(v, expr, -1);     return 1;
#define RET_STRING_N(expr, n)   sq_pushstring(v, expr, n);      return 1;
#define RET_OBJECT(expr)        sq_pushobject(v, &(expr));      return 1;
#define RET_ARG(idx)            sq_push(v, idx + 1);            return 1;
#define RET_RECT(expr)          BindRect(v, expr);              return 1;
#define RET_VEC2(expr)          BindVec2(v, expr);              return 1;
#define RET_STREAM(expr)        BindStream(v, expr);            return 1;
#define RET_BLOB(expr)          BindBlob(v, expr);              return 1;
#define RET_FILE(expr)          BindFile(v, expr);              return 1;
#define RET_CANVAS(expr)        BindCanvas(v, expr);            return 1;
#define RET_TEXTURE(expr)       BindTexture(v, expr);           return 1;
#define RET_SOUND(expr)         BindSound(v, expr);             return 1;
#define RET_SOUNDEFFECT(expr)   BindSoundEffect(v, expr);       return 1;
#define RET_ZSTREAM(expr)       BindZStream(v, expr);           return 1;


#endif
