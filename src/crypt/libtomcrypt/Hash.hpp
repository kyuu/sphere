#ifndef HASH_HPP
#define HASH_HPP

#include "libtomcrypt/src/headers/tomcrypt_hash.h"
#include "../common/RefImpl.hpp"
#include "../IHash.hpp"


class Hash : public RefImpl<IHash> {
public:
    static Hash* Create(int type);

    // IHash implementation
    bool init();
    int  getType() const;
    bool process(const u8* buf, int len);
    bool finish(Blob* hash);

private:
    explicit Hash(int type);
    ~Hash();

private:
    int  _type;
    bool _initialized;
    hash_state _md;
}


#endif
