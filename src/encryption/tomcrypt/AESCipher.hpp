#ifndef AESCIPHER_HPP
#define AESCIPHER_HPP

#include "tomcrypt/src/headers/tomcrypt_cipher.h"
#include "../common/RefImpl.hpp"
#include "../ICipher.hpp"


class AESCipher : public RefImpl<ICipher> {
public:
    static AESCipher* Create();

    // ICipher implementation
    bool init(const u8* key, int key_len, const u8* iv, int iv_len);
    bool isInitialized() const;
    bool encrypt(const u8* pt, u8* ct, int len);
    bool decrypt(const u8* ct, u8* pt, int len);

private:
    AESCipher();
    ~AESCipher();

private:
    bool _initialized;
    symmetric_CTR _ctr;
}


#endif
