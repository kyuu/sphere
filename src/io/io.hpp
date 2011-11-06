#ifndef IO_HPP
#define IO_HPP

#include "../common/types.hpp"
#include "IStream.hpp"


namespace io {

    // read endian independent
    bool readu8(IStream* s, u8& out);

    // read little endian
    bool readu16l(IStream* s, u16& out);
    bool readu32l(IStream* s, u32& out);
    bool readu64l(IStream* s, u64& out);
    bool readf32l(IStream* s, f32& out);
    bool readf64l(IStream* s, f64& out);

    // read big endian
    bool readu16b(IStream* s, u16& out);
    bool readu32b(IStream* s, u32& out);
    bool readu64b(IStream* s, u64& out);
    bool readf32b(IStream* s, f32& out);
    bool readf64b(IStream* s, f64& out);

    // write endian independent
    bool writeu8(IStream* s, u8 n, int count = 1);

    // write little endian
    bool writeu16l(IStream* s, u16 n);
    bool writeu32l(IStream* s, u32 n);
    bool writeu64l(IStream* s, u64 n);
    bool writef32l(IStream* s, f32 n);
    bool writef64l(IStream* s, f64 n);

    // write big endian
    bool writeu16b(IStream* s, u16 n);
    bool writeu32b(IStream* s, u32 n);
    bool writeu64b(IStream* s, u64 n);
    bool writef32b(IStream* s, f32 n);
    bool writef64b(IStream* s, f64 n);

} // namespace io


#endif
