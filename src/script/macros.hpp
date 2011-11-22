#ifndef MACROS_HPP
#define MACROS_HPP


#define NARGS() \
    (sq_gettop(v) - 1)

#define CHECK_NARGS(n) \
    if (sq_gettop(v) != n + 1) { \
        return sq_throwerror(v, "Invalid number of arguments"); \
    }

#define CHECK_MIN_NARGS(n) \
    if (sq_gettop(v) < n + 1) { \
        return sq_throwerror(v, "Invalid number of arguments"); \
    }

#define ARG_IS_NULL(idx)    (sq_gettype(v, idx + 1) == OT_NULL)
#define ARG_IS_BOOL(idx)    (sq_gettype(v, idx + 1) == OT_BOOL)
#define ARG_IS_INT(idx)     (sq_gettype(v, idx + 1) == OT_INTEGER)
#define ARG_IS_FLOAT(idx)   (sq_gettype(v, idx + 1) == OT_FLOAT)
#define ARG_IS_NUMERIC(idx) ((sq_gettype(v, idx + 1) & SQOBJECT_NUMERIC) != 0)
#define ARG_IS_STRING(idx)  (sq_gettype(v, idx + 1) == OT_STRING)

#define RET_ARG(idx) \
    sq_push(v, idx + 1); \
    return 1;

#define RET_OBJECT(expr) \
    sq_pushobject(v, &(expr)); \
    return 1;

#define RET_VOID() \
    return 0;

#define RET_NULL() \
    sq_pushnull(v); \
    return 1;

#define RET_TRUE(expr) \
    sq_pushbool(v, SQTrue); \
    return 1;

#define RET_FALSE(expr) \
    sq_pushbool(v, SQFalse); \
    return 1;

#define GET_ARG_BOOL(idx, name) \
    SQBool name; \
    if (SQ_FAILED(sq_getbool(v, idx + 1, &name))) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_BOOL(idx, name, defval) \
    SQBool name = defval; \
    if (sq_gettop(v) >= idx + 1) { \
        if (SQ_FAILED(sq_getbool(v, idx + 1, &name))) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_BOOL(expr) \
    sq_pushbool(v, expr); \
    return 1;

#define GET_ARG_INT(idx, name) \
    SQInteger name; \
    if (SQ_FAILED(sq_getinteger(v, idx + 1, &name))) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_INT(idx, name, defval) \
    SQInteger name = defval; \
    if (sq_gettop(v) >= idx + 1) { \
        if (SQ_FAILED(sq_getinteger(v, idx + 1, &name))) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_INT(expr) \
    sq_pushinteger(v, expr); \
    return 1;

#define GET_ARG_FLOAT(idx, name) \
    SQFloat name; \
    if (SQ_FAILED(sq_getfloat(v, idx + 1, &name))) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_FLOAT(idx, name, defval) \
    SQFloat name = defval; \
    if (sq_gettop(v) >= idx + 1) { \
        if (SQ_FAILED(sq_getfloat(v, idx + 1, &name))) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_FLOAT(expr) \
    sq_pushfloat(v, expr); \
    return 1;

#define GET_ARG_STRING(idx, name) \
    const SQChar* name = 0; \
    if (SQ_FAILED(sq_getstring(v, idx + 1, &name))) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_STRING(idx, name, defval) \
    const SQChar* name = defval; \
    if (sq_gettop(v) >= idx + 1) { \
        if (SQ_FAILED(sq_getstring(v, idx + 1, &name))) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_STRING(expr) \
    sq_pushstring(v, expr, -1); \
    return 1;

#define RET_STRING_N(expr, n) \
    sq_pushstring(v, expr, n); \
    return 1;

#define GET_ARG_STREAM(idx, name) \
    IStream* name = GetStream(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_STREAM(idx, name) \
    IStream* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetStream(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_STREAM(expr) \
    BindStream(expr); \
    return 1;

#define GET_ARG_FILE(idx, name) \
    IFile* name = GetFile(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_FILE(idx, name) \
    IFile* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetFile(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_FILE(expr) \
    BindFile(expr); \
    return 1;

#define GET_ARG_BLOB(idx, name) \
    Blob* name = GetBlob(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_BLOB(idx, name) \
    Blob* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetBlob(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_BLOB(expr) \
    BindBlob(expr); \
    return 1;

#define GET_ARG_CANVAS(idx, name) \
    Canvas* name = GetCanvas(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_CANVAS(idx, name) \
    Canvas* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetCanvas(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_CANVAS(expr) \
    BindCanvas(expr); \
    return 1;

#define GET_ARG_RECT(idx, name) \
    Recti* name = GetRect(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_RECT(idx, name) \
    Recti* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetRect(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_RECT(expr) \
    BindRect(expr); \
    return 1;

#define GET_ARG_VEC2(idx, name) \
    Vec2i* name = GetVec2(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_VEC2(idx, name) \
    Vec2i* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetVec2(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_VEC2(expr) \
    BindVec2(expr); \
    return 1;

#define GET_ARG_TEXTURE(idx, name) \
    ITexture* name = GetTexture(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_TEXTURE(idx, name) \
    ITexture* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetTexture(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_TEXTURE(expr) \
    BindTexture(expr); \
    return 1;

#define GET_ARG_SOUND(idx, name) \
    ISound* name = GetSound(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_SOUND(idx, name) \
    ISound* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetSound(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_SOUND(expr) \
    BindSound(expr); \
    return 1;

#define GET_ARG_SOUNDEFFECT(idx, name) \
    ISoundEffect* name = GetSoundEffect(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_SOUNDEFFECT(idx, name) \
    ISoundEffect* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetSoundEffect(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_SOUNDEFFECT(expr) \
    BindSoundEffect(expr); \
    return 1;

#define GET_ARG_FORCEEFFECT(idx, name) \
    ForceEffect* name = GetForceEffect(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_FORCEEFFECT(idx, name) \
    ForceEffect* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetForceEffect(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_FORCEEFFECT(expr) \
    BindForceEffect(expr); \
    return 1;

#define GET_ARG_ZSTREAM(idx, name) \
    ZStream* name = GetZStream(idx + 1); \
    if (!name) { \
        return sq_throwerror(v, "Invalid argument " #idx); \
    }

#define GET_OPTARG_ZSTREAM(idx, name) \
    ZStream* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetZStream(idx + 1); \
        if (!name) { \
            return sq_throwerror(v, "Invalid argument " #idx); \
        } \
    }

#define RET_ZSTREAM(expr) \
    BindZStream(expr); \
    return 1;

#define THROW_ERROR(msg) \
    return sq_throwerror(v, msg);


#endif
