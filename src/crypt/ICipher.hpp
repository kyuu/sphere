#ifndef ICIPHER_HPP
#define ICIPHER_HPP

#include "../common/IRefCounted.hpp"
#include "../common/RefPtr.hpp"
#include "../common/types.hpp"


class ICipher : public IRefCounted {
public:
    enum {
        BLOWFISH = 0;
        TWOFISH,
        AES,
    }

    virtual bool init(const u8* key) = 0;
    virtual bool isInitialized() const = 0;
    virtual int  getType() const = 0;
    virtual bool getIV(Blob* iv) const = 0;
    virtual bool encrypt(const u8* pt, u8* ct, int len) = 0;
    virtual bool decrypt(const u8* ct, u8* pt, int len) = 0;

protected:
    ~ICipher() { }
}

typedef RefPtr<ICipher> CipherPtr;


#endif
