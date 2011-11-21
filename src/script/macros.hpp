#ifndef MACROS_HPP
#define MACROS_HPP


#define NARGS() \
    (sq_gettop(v) - 1)

#define CHECK_NARGS(n) \
    if (sq_gettop(v) != n + 1) { \
        return ThrowError(v, "Invalid number of arguments, expected %d", n); \
    }

#define CHECK_MIN_NARGS(n) \
    if (sq_gettop(v) < n + 1) { \
        return ThrowError(v, "Invalid number of arguments, expected at least %d", n); \
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

#define RET_NULL(expr) \
    sq_pushnull(v); \
    return 1;

#define RET_TRUE(expr) \
    sq_pushbool(v, SQTrue); \
    return 1;

#define RET_FALSE(expr) \
    sq_pushbool(v, SQFalse); \
    return 1;

#define RET_BOOL(expr) \
    sq_pushbool(v, expr); \
    return 1;

#define GET_ARG_INT(idx, name) \
    SQInteger name; \
    if (SQ_FAILED(sq_getinteger(v, idx + 1, &name)) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected integer", idx, #name); \
    }

#define GET_OPTARG_INT(idx, name, defval) \
    SQInteger name = defval; \
    if (sq_gettop(v) >= idx + 1) { \
        if (SQ_FAILED(sq_getinteger(v, idx + 1, &name)) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected integer", idx, #name); \
        } \
    }

#define RET_INT(expr) \
    sq_pushinteger(v, expr); \
    return 1;

#define GET_ARG_FLOAT(idx, name) \
    SQFloat name; \
    if (SQ_FAILED(sq_getfloat(v, idx + 1, &name)) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected float", idx, #name); \
    }

#define GET_OPTARG_FLOAT(idx, name, defval) \
    SQFloat name = defval; \
    if (sq_gettop(v) >= idx + 1) { \
        if (SQ_FAILED(sq_getfloat(v, idx + 1, &name)) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected float", idx, #name); \
        } \
    }

#define RET_FLOAT(expr) \
    sq_pushfloat(v, expr); \
    return 1;

#define GET_ARG_STRING(idx, name) \
    const SQChar* name = 0; \
    if (SQ_FAILED(sq_getstring(v, idx + 1, &name)) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected string", idx, #name); \
    }

#define GET_OPTARG_STRING(idx, name, defval) \
    const SQChar* name = defval; \
    if (sq_gettop(v) >= idx + 1) { \
        if (SQ_FAILED(sq_getstring(v, idx + 1, &name)) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected string", idx, #name); \
        } \
    }

#define RET_STRING(expr) \
    sq_pushstring(v, expr, -1); \
    return 1;

#define RET_STRING_N(expr, n) \
    sq_pushstring(v, expr, n); \
    return 1;

#define GET_ARG_STREAM(idx, name) \
    IStream* name = GetStream(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Stream instance", idx, #name); \
    }

#define GET_OPTARG_STREAM(idx, name) \
    IStream* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetStream(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Stream instance", idx, #name); \
        } \
    }

#define RET_STREAM(expr) \
    if (!BindStream(v, expr)) { \
        return ThrowError(v, "BindStream failed")); \
    } \
    return 1;

#define GET_ARG_FILE(idx, name) \
    IFile* name = GetFile(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected File instance", idx, #name); \
    }

#define GET_OPTARG_FILE(idx, name) \
    IFile* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetFile(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected File instance", idx, #name); \
        } \
    }

#define RET_FILE(expr) \
    if (!BindFile(v, expr)) { \
        return ThrowError(v, "BindFile failed")); \
    } \
    return 1;

#define GET_ARG_BLOB(idx, name) \
    Blob* name = GetBlob(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Blob instance", idx, #name); \
    }

#define GET_OPTARG_BLOB(idx, name) \
    Blob* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetBlob(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Blob instance", idx, #name); \
        } \
    }

#define RET_BLOB(expr) \
    if (!BindBlob(v, expr)) { \
        return ThrowError(v, "BindBlob failed")); \
    } \
    return 1;

#define GET_ARG_CANVAS(idx, name) \
    Canvas* name = GetCanvas(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Canvas instance", idx, #name); \
    }

#define GET_OPTARG_CANVAS(idx, name) \
    Canvas* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetCanvas(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Canvas instance", idx, #name); \
        } \
    }

#define RET_CANVAS(expr) \
    if (!BindCanvas(v, expr)) { \
        return ThrowError(v, "BindCanvas failed")); \
    } \
    return 1;

#define GET_ARG_RGBA(idx, name) \
    RGBA* name = GetRGBA(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected RGBA instance", idx, #name); \
    }

#define GET_OPTARG_RGBA(idx, name) \
    RGBA* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetRGBA(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected RGBA instance", idx, #name); \
        } \
    }

#define RET_RGBA(expr) \
    if (!BindRGBA(v, expr)) { \
        return ThrowError(v, "BindRGBA failed")); \
    } \
    return 1;

#define GET_ARG_RECT(idx, name) \
    Recti* name = GetRect(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Rect instance", idx, #name); \
    }

#define GET_OPTARG_RECT(idx, name) \
    Recti* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetRect(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Rect instance", idx, #name); \
        } \
    }

#define RET_RECT(expr) \
    if (!BindRect(v, expr)) { \
        return ThrowError(v, "BindRect failed")); \
    } \
    return 1;

#define GET_ARG_VEC2(idx, name) \
    Vec2i* name = GetVec2(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Vec2 instance", idx, #name); \
    }

#define GET_OPTARG_VEC2(idx, name) \
    Vec2i* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetVec2(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Vec2 instance", idx, #name); \
        } \
    }

#define RET_VEC2(expr) \
    if (!BindVec2(v, expr)) { \
        return ThrowError(v, "BindVec2 failed")); \
    } \
    return 1;

#define GET_ARG_TEXTURE(idx, name) \
    ITexture* name = GetTexture(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Texture instance", idx, #name); \
    }

#define GET_OPTARG_TEXTURE(idx, name) \
    ITexture* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetTexture(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Texture instance", idx, #name); \
        } \
    }

#define RET_TEXTURE(expr) \
    if (!BindTexture(v, expr)) { \
        return ThrowError(v, "BindTexture failed")); \
    } \
    return 1;

#define GET_ARG_SOUND(idx, name) \
    ISound* name = GetSound(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Sound instance", idx, #name); \
    }

#define GET_OPTARG_SOUND(idx, name) \
    ISound* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetSound(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Sound instance", idx, #name); \
        } \
    }

#define RET_SOUND(expr) \
    if (!BindSound(v, expr)) { \
        return ThrowError(v, "BindSound failed")); \
    } \
    return 1;

#define GET_ARG_SOUNDEFFECT(idx, name) \
    ISoundEffect* name = GetSoundEffect(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected SoundEffect instance", idx, #name); \
    }

#define GET_OPTARG_SOUNDEFFECT(idx, name) \
    ISoundEffect* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetSoundEffect(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected SoundEffect instance", idx, #name); \
        } \
    }

#define RET_SOUNDEFFECT(expr) \
    if (!BindSoundEffect(v, expr)) { \
        return ThrowError(v, "BindSoundEffect failed")); \
    } \
    return 1;

#define GET_ARG_FORCEEFFECT(idx, name) \
    ForceEffect* name = GetForceEffect(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected ForceEffect instance", idx, #name); \
    }

#define GET_OPTARG_FORCEEFFECT(idx, name) \
    ForceEffect* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetForceEffect(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected ForceEffect instance", idx, #name); \
        } \
    }

#define RET_FORCEEFFECT(expr) \
    if (!BindForceEffect(v, expr)) { \
        return ThrowError(v, "BindForceEffect failed")); \
    } \
    return 1;

#define GET_ARG_CIPHER(idx, name) \
    ICipher* name = GetCipher(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Cipher instance", idx, #name); \
    }

#define GET_OPTARG_CIPHER(idx, name) \
    ICipher* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetCipher(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Cipher instance", idx, #name); \
        } \
    }

#define RET_CIPHER(expr) \
    if (!BindCipher(v, expr)) { \
        return ThrowError(v, "BindCipher failed")); \
    } \
    return 1;

#define GET_ARG_HASHER(idx, name) \
    IHash* name = GetHash(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected Hash instance", idx, #name); \
    }

#define GET_OPTARG_HASHER(idx, name) \
    IHash* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetHash(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected Hash instance", idx, #name); \
        } \
    }

#define RET_HASHER(expr) \
    if (!BindHash(v, expr)) { \
        return ThrowError(v, "BindHash failed")); \
    } \
    return 1;

#define GET_ARG_ZSTREAM(idx, name) \
    ZStream* name = GetZStream(v, idx + 1); \
    if (!name) { \
        return ThrowError(v, "Invalid type of argument %d, '%s', expected ZStream instance", idx, #name); \
    }

#define GET_OPTARG_ZSTREAM(idx, name) \
    ZStream* name = 0; \
    if (sq_gettop(v) >= idx + 1) { \
        name = GetZStream(v, idx + 1); \
        if (!name) { \
            return ThrowError(v, "Invalid type of optional argument %d, '%s', expected ZStream instance", idx, #name); \
        } \
    }

#define RET_ZSTREAM(expr) \
    if (!BindZStream(v, expr)) { \
        return ThrowError(v, "BindZStream failed")); \
    } \
    return 1;

#define THROW_ERROR(msg) \
    return ThrowError(v, msg);


#endif
