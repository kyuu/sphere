#ifndef WHIRLPOOLHASH_HPP
#define WHIRLPOOLHASH_HPP

#include "tomcrypt/src/headers/tomcrypt_hash.h"
#include "../common/RefImpl.hpp"
#include "../IHash.hpp"


class WhirlpoolHash : public RefImpl<IHash> {
public:
    static WhirlpoolHash* Create();

    // IHash implementation
    bool init();
    bool isInitialized() const;
    bool process(const u8* buf, int len);
    bool finish(Blob* out);

private:
    WhirlpoolHash();
    ~WhirlpoolHash();

private:
    bool _initialized;
    hash_state _md;
}


#endif
