#ifndef SHA512HASH_HPP
#define SHA512HASH_HPP

#include "tomcrypt/src/headers/tomcrypt_hash.h"
#include "../common/RefImpl.hpp"
#include "../IHash.hpp"


class SHA512Hash : public RefImpl<IHash> {
public:
    static SHA512Hash* Create();

    // IHash implementation
    bool init();
    bool isInitialized() const;
    bool process(const u8* buf, int len);
    bool finish(Blob* out);

private:
    SHA512Hash();
    ~SHA512Hash();

private:
    bool _initialized;
    hash_state _md;
}


#endif
