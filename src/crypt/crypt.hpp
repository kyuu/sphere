#ifndef CRYPT_HPP
#define CRYPT_HPP

#include "../common/types.hpp"
#include "../io/Blob.hpp"


namespace crypt {

    ICipher CreateCipher(int type);
    IHash*  CreateHash(int type);
    bool    Base64Encode(const u8* buf, int len, Blob* result);
    bool    Base64Decode(const u8* buf, int len, Blob* result);
    u32     ComputeCRC32(const u8* buf, int len, u32 in_crc32 = 0);

} // namespace crypt


#endif
