#ifndef MD5HASH_HPP
#define MD5HASH_HPP

#include "tomcrypt/src/headers/tomcrypt_hash.h"
#include "../common/RefImpl.hpp"
#include "../IHash.hpp"


class MD5Hash : public RefImpl<IHash> {
public:
    static MD5Hash* Create();

    // IHash implementation
    bool init();
    bool isInitialized() const;
    bool process(const u8* buf, int len);
    bool finish(Blob* out);

private:
    MD5Hash();
    ~MD5Hash();

private:
    bool _initialized;
    hash_state _md;
}


#endif
