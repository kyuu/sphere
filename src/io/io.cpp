#include <algorithm>
#include <cassert>
#include "../common/platform.hpp"
#include "endian.hpp"
#include "io.hpp"


//-----------------------------------------------------------------
bool readi8(IStream* s, i8& n)
{
    assert(s);
    return s->read(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool readi16l(IStream* s, i16& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef BIG_ENDIAN
    ltoh2(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readi32l(IStream* s, i32& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef BIG_ENDIAN
    ltoh4(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readi64l(IStream* s, i64& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef BIG_ENDIAN
    ltoh8(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readf32l(IStream* s, f32& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef BIG_ENDIAN
    ltoh4(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readf64l(IStream* s, f64& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef BIG_ENDIAN
    ltoh8(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readi16b(IStream* s, i16& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef LITTLE_ENDIAN
    ltoh2(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readi32b(IStream* s, i32& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef LITTLE_ENDIAN
    ltoh4(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readi64b(IStream* s, i64& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef LITTLE_ENDIAN
    ltoh8(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readf32b(IStream* s, f32& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef LITTLE_ENDIAN
    ltoh4(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool readf64b(IStream* s, f64& n)
{
    assert(s);
    bool succeeded = s->read(&n, sizeof(n)) == sizeof(n);
#ifdef LITTLE_ENDIAN
    ltoh8(&n);
#endif
    return succeeded;
}

//-----------------------------------------------------------------
bool writei8(IStream* s, i8 n)
{
    assert(s);
    return s->read(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writei16l(IStream* s, i16 n)
{
    assert(s);
#ifdef BIG_ENDIAN
    htol2(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writei32l(IStream* s, i32 n)
{
    assert(s);
#ifdef BIG_ENDIAN
    htol4(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writei64l(IStream* s, i64 n)
{
    assert(s);
#ifdef BIG_ENDIAN
    htol8(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writef32l(IStream* s, f32 n)
{
    assert(s);
#ifdef BIG_ENDIAN
    htol4(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writef64l(IStream* s, f64 n)
{
    assert(s);
#ifdef BIG_ENDIAN
    htol8(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writei16b(IStream* s, i16 n)
{
    assert(s);
#ifdef LITTLE_ENDIAN
    htol2(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writei32b(IStream* s, i32 n)
{
    assert(s);
#ifdef LITTLE_ENDIAN
    htol4(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writei64b(IStream* s, i64 n)
{
    assert(s);
#ifdef LITTLE_ENDIAN
    htol8(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writef32b(IStream* s, f32 n)
{
    assert(s);
#ifdef LITTLE_ENDIAN
    htol4(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}

//-----------------------------------------------------------------
bool writef64b(IStream* s, f64 n)
{
    assert(s);
#ifdef LITTLE_ENDIAN
    htol8(&n);
#endif
    return s->write(&n, sizeof(n)) == sizeof(n);
}
