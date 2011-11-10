#ifndef CIPHER_HPP
#define CIPHER_HPP

#include "libtomcrypt/src/headers/tomcrypt_cipher.h"
#include "../common/RefImpl.hpp"
#include "../ICipher.hpp"


class Cipher : public RefImpl<ICipher> {
public:
    static Cipher* Create(int type);

    // ICipher implementation
    bool init(const u8* key);
    int  getType() const;
    bool getIV(Blob* iv) const;
    bool encrypt(const u8* pt, u8* ct, int len);
    bool decrypt(const u8* ct, u8* pt, int len);

private:
    explicit Cipher(int type);
    ~Cipher();

private:
    int  _type;
    u8   _iv[16];
    bool _initialized;
    symmetric_CTR _ctr;
}

typedef RefPtr<ICipher> CipherPtr;


#endif
