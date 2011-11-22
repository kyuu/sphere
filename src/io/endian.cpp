#include <algorithm>
#include <cassert>
#include "../common/platform.hpp"
#include "endian.hpp"


union word {
    u16 value;
    u8  bytes[2];
};

union dword {
    u32 value;
    u8  bytes[4];
};

union qword {
    u64 value;
    u8  bytes[8];
};

inline void _s2(word* p)
{
    word w;
    w.value = p->value;
    p->bytes[0] = w.bytes[1];
    p->bytes[1] = w.bytes[0];
}

inline void _s4(dword* p)
{
    dword d;
    d.value = p->value;
    p->bytes[0] = d.bytes[3];
    p->bytes[1] = d.bytes[2];
    p->bytes[2] = d.bytes[1];
    p->bytes[3] = d.bytes[0];
}

inline void _s8(qword* p)
{
    qword q;
    q.value = p->value;
    p->bytes[0] = q.bytes[7];
    p->bytes[1] = q.bytes[6];
    p->bytes[2] = q.bytes[5];
    p->bytes[3] = q.bytes[4];
    p->bytes[4] = q.bytes[3];
    p->bytes[5] = q.bytes[2];
    p->bytes[6] = q.bytes[1];
    p->bytes[7] = q.bytes[0];
}

//-----------------------------------------------------------------
void htol2(void* p)
{
#ifdef BIG_ENDIAN
    _s2((word*)p);
#endif
}

//-----------------------------------------------------------------
void htol4(void* p)
{
#ifdef BIG_ENDIAN
    _s4((dword*)p);
#endif
}

//-----------------------------------------------------------------
void htol8(void* p)
{
#ifdef BIG_ENDIAN
    _s8((qword*)p);
#endif
}

//-----------------------------------------------------------------
void ltoh2(void* p)
{
#ifdef BIG_ENDIAN
    _s2((word*)p);
#endif
}

//-----------------------------------------------------------------
void ltoh4(void* p)
{
#ifdef BIG_ENDIAN
    _s4((dword*)p);
#endif
}

//-----------------------------------------------------------------
void ltoh8(void* p)
{
#ifdef BIG_ENDIAN
    _s8((qword*)p);
#endif
}

//-----------------------------------------------------------------
void htob2(void* p)
{
#ifdef LITTLE_ENDIAN
    _s2((word*)p);
#endif
}

//-----------------------------------------------------------------
void htob4(void* p)
{
#ifdef LITTLE_ENDIAN
    _s4((dword*)p);
#endif
}

//-----------------------------------------------------------------
void htob8(void* p)
{
#ifdef LITTLE_ENDIAN
    _s8((qword*)p);
#endif
}

//-----------------------------------------------------------------
void btoh2(void* p)
{
#ifdef LITTLE_ENDIAN
    _s2((word*)p);
#endif
}

//-----------------------------------------------------------------
void btoh4(void* p)
{
#ifdef LITTLE_ENDIAN
    _s4((dword*)p);
#endif
}

//-----------------------------------------------------------------
void btoh8(void* p)
{
#ifdef LITTLE_ENDIAN
    _s8((qword*)p);
#endif
}

//-----------------------------------------------------------------
void swap2(void* p, int len)
{
    word* ptr = (word*)p;
    for (int i = 0; i < len; i++) {
        _s2(ptr);
        ptr++;
    }
}

//-----------------------------------------------------------------
void swap4(void* p, int len)
{
    dword* ptr = (dword*)p;
    for (int i = 0; i < len; i++) {
        _s4(ptr);
        ptr++;
    }
}

//-----------------------------------------------------------------
void swap8(void* p, int len)
{
    qword* ptr = (qword*)p;
    for (int i = 0; i < len; i++) {
        _s8(ptr);
        ptr++;
    }
}
