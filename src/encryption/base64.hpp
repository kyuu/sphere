#ifndef BASE64_HPP
#define BASE64_HPP

#include "../common/types.hpp"
#include "../io/Blob.hpp"


bool Base64Encode(const u8* buf, int len, Blob* out);
bool Base64Decode(const u8* buf, int len, Blob* out);


#endif
