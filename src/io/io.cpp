#include <algorithm>
#include <cassert>
#include "../common/platform.hpp"
#include "io.hpp"


namespace io {

    //-----------------------------------------------------------------
    bool readu8(IStream* s, u8& out)
    {
        assert(s);
        return s->read(&out, 1) == 1;
    }

    //-----------------------------------------------------------------
    bool readu16l(IStream* s, u16& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 2) == 2;
    #ifdef BIG_ENDIAN
        out = (out << 8) | (out >> 8);
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readu32l(IStream* s, u32& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 4) == 4;
    #ifdef BIG_ENDIAN
        out = (out << 24) | ((out & 0x00FF0000) >> 8) | ((out & 0x0000FF00) << 8) | (out >> 24);
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readu64l(IStream* s, u64& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 8) == 8;
    #ifdef BIG_ENDIAN
        u8 temp;
        u8* p = (u8*)&out;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readf32l(IStream* s, f32& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 4) == 4;
    #ifdef BIG_ENDIAN
        u32* p = (u32*)&out;
        *p = (*p << 24) | ((*p & 0x00FF0000) >> 8) | ((*p & 0x0000FF00) << 8) | (*p >> 24);
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readf64l(IStream* s, f64& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 8) == 8;
    #ifdef BIG_ENDIAN
        u8 temp;
        u8* p = (u8*)&out;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readu16b(IStream* s, u16& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 2) == 2;
    #ifdef LITTLE_ENDIAN
        out = (out << 8) | (out >> 8);
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readu32b(IStream* s, u32& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 4) == 4;
    #ifdef LITTLE_ENDIAN
        out = (out << 24) | ((out & 0x00FF0000) >> 8) | ((out & 0x0000FF00) << 8) | (out >> 24);
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readu64b(IStream* s, u64& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 8) == 8;
    #ifdef LITTLE_ENDIAN
        u8 temp;
        u8* p = (u8*)&out;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readf32b(IStream* s, f32& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 4) == 4;
    #ifdef LITTLE_ENDIAN
        u32* p = (u32*)&out;
        *p = (*p << 24) | ((*p & 0x00FF0000) >> 8) | ((*p & 0x0000FF00) << 8) | (*p >> 24);
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool readf64b(IStream* s, f64& out)
    {
        assert(s);
        bool succeeded = s->read(&out, 8) == 8;
    #ifdef LITTLE_ENDIAN
        u8 temp;
        u8* p = (u8*)&out;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return succeeded;
    }

    //-----------------------------------------------------------------
    bool writeu8(IStream* s, u8 n, int count)
    {
        assert(s);
        assert(count > 0);
        if (count == 1) {
            return s->write(&n, 1) == 1;
        }
        const int bufsize = 256;
        static u8 buf[bufsize];
        if (count <= bufsize) {
            memset(buf, n, count);
            return s->write(buf, count) == count;
        }
        memset(buf, n, bufsize);
        int remaining = count;
        while (remaining > 0) {
            int abletowrite = std::min(remaining, bufsize);
            if (s->write(buf, abletowrite) != abletowrite) {
                return false; // no sense writing further
            }
            remaining -= abletowrite;
        }
        return true;
    }

    //-----------------------------------------------------------------
    bool writeu16l(IStream* s, u16 n)
    {
        assert(s);
    #ifdef BIG_ENDIAN
        n = (n << 8) | (n >> 8);
    #endif
        return s->write(&n, 2) == 2;
    }

    //-----------------------------------------------------------------
    bool writeu32l(IStream* s, u32 n)
    {
        assert(s);
    #ifdef BIG_ENDIAN
        n = (n << 24) | ((n & 0x00FF0000) >> 8) | ((n & 0x0000FF00) << 8) | (n >> 24);
    #endif
        return s->write(&n, 4) == 4;
    }

    //-----------------------------------------------------------------
    bool writeu64l(IStream* s, u64 n)
    {
        assert(s);
    #ifdef BIG_ENDIAN
        u8 temp;
        u8* p = (u8*)&n;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return s->write(&n, 8) == 8;
    }

    //-----------------------------------------------------------------
    bool writef32l(IStream* s, f32 n)
    {
        assert(s);
    #ifdef BIG_ENDIAN
        u32* p = (u32*)&n;
        *p = (*p << 24) | ((*p & 0x00FF0000) >> 8) | ((*p & 0x0000FF00) << 8) | (*p >> 24);
    #endif
        return s->write(&n, 4) == 4;
    }

    //-----------------------------------------------------------------
    bool writef64l(IStream* s, f64 n)
    {
        assert(s);
    #ifdef BIG_ENDIAN
        u8 temp;
        u8* p = (u8*)&n;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return s->write(&n, 8) == 8;
    }

    //-----------------------------------------------------------------
    bool writeu16b(IStream* s, u16 n)
    {
        assert(s);
    #ifdef LITTLE_ENDIAN
        n = (n << 8) | (n >> 8);
    #endif
        return s->write(&n, 2) == 2;
    }

    //-----------------------------------------------------------------
    bool writeu32b(IStream* s, u32 n)
    {
        assert(s);
    #ifdef LITTLE_ENDIAN
        n = (n << 24) | ((n & 0x00FF0000) >> 8) | ((n & 0x0000FF00) << 8) | (n >> 24);
    #endif
        return s->write(&n, 4) == 4;
    }

    //-----------------------------------------------------------------
    bool writeu64b(IStream* s, u64 n)
    {
        assert(s);
    #ifdef LITTLE_ENDIAN
        u8 temp;
        u8* p = (u8*)&n;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return s->write(&n, 8) == 8;
    }

    //-----------------------------------------------------------------
    bool writef32b(IStream* s, f32 n)
    {
        assert(s);
    #ifdef LITTLE_ENDIAN
        u32* p = (u32*)&n;
        *p = (*p << 24) | ((*p & 0x00FF0000) >> 8) | ((*p & 0x0000FF00) << 8) | (*p >> 24);
    #endif
        return s->write(&n, 4) == 4;
    }

    //-----------------------------------------------------------------
    bool writef64b(IStream* s, f64 n)
    {
        assert(s);
    #ifdef LITTLE_ENDIAN
        u8 temp;
        u8* p = (u8*)&n;
        temp = p[0];
        p[0] = p[7];
        p[7] = temp;
        temp = p[1];
        p[1] = p[6];
        p[6] = temp;
        temp = p[2];
        p[2] = p[5];
        p[5] = temp;
        temp = p[3];
        p[3] = p[4];
        p[4] = temp;
    #endif
        return s->write(&n, 8) == 8;
    }

} // namespace io
