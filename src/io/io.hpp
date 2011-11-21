#ifndef IO_HPP
#define IO_HPP

#include "../common/types.hpp"
#include "IStream.hpp"


bool readi8(IStream* s, i8& n);

bool readi16l(IStream* s, i16& n);
bool readi32l(IStream* s, i32& n);
bool readi64l(IStream* s, i64& n);
bool readf32l(IStream* s, f32& n);
bool readf64l(IStream* s, f64& n);

bool readi16b(IStream* s, i16& n);
bool readi32b(IStream* s, i32& n);
bool readi64b(IStream* s, i64& n);
bool readf32b(IStream* s, f32& n);
bool readf64b(IStream* s, f64& n);

bool writei8(IStream* s, i8 n);

bool writei16l(IStream* s, i16 n);
bool writei32l(IStream* s, i32 n);
bool writei64l(IStream* s, i64 n);
bool writef32l(IStream* s, f32 n);
bool writef64l(IStream* s, f64 n);

bool writei16b(IStream* s, i16 n);
bool writei32b(IStream* s, i32 n);
bool writei64b(IStream* s, i64 n);
bool writef32b(IStream* s, f32 n);
bool writef64b(IStream* s, f64 n);


#endif
