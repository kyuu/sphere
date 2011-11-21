#ifndef SHA256HASH_HPP
#define SHA256HASH_HPP

#include "tomcrypt/src/headers/tomcrypt_hash.h"
#include "../common/RefImpl.hpp"
#include "../IHash.hpp"


class SHA256Hash : public RefImpl<IHash> {
public:
    static SHA256Hash* Create();

    // IHash implementation
    bool init();
    bool isInitialized() const;
    bool process(const u8* buf, int len);
    bool finish(Blob* out);

private:
    SHA256Hash();
    ~SHA256Hash();

private:
    bool _initialized;
    hash_state _md;
}


#endif
