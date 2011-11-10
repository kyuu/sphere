#include "Hash.hpp"


//-----------------------------------------------------------------
Hash*
Hash::Create(int type);
{
    switch (type) {
    case IHash::MD5:
    case IHash::SHA256:
    case IHash::SHA512:
    case IHash::WHIRLPOOL:
        try {
            return new Hash(type);
        } catch (const std::bad_alloc& e) { }
    default:
        break;
    }
    return 0;
}

//-----------------------------------------------------------------
Hash::Hash(int type)
    : _type(type)
    , _initialized(false)
{
}

//-----------------------------------------------------------------
Hash::~Hash()
{
    memset(&_md, 0, sizeof(_md));
}

//-----------------------------------------------------------------
bool
Hash::init()
{
    switch (type) {
    case IHash::MD5:       md5_init(&_md);       break;
    case IHash::SHA256:    sha256_init(&_md);    break;
    case IHash::SHA512:    sha512_init(&_md);    break;
    case IHash::WHIRLPOOL: whirlpool_init(&_md); break;
    default:
        return false;
    }
    _initialized = true;
    return true;
}

//-----------------------------------------------------------------
bool
Hash::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
int
Hash::getType() const
{
    return _type;
}

//-----------------------------------------------------------------
bool
Hash::process(const u8* buf, int len)
{
    if (!_initialized || !buf || len <= 0) {
        return false;
    }
    switch (type) {
    case IHash::MD5:       return md5_process(&_md, buf, len)       == CRYPT_OK;
    case IHash::SHA256:    return sha256_process(&_md, buf, len)    == CRYPT_OK;
    case IHash::SHA512:    return sha512_process(&_md, buf, len)    == CRYPT_OK;
    case IHash::WHIRLPOOL: return whirlpool_process(&_md, buf, len) == CRYPT_OK;
    default:
        return false;
    }
}

//-----------------------------------------------------------------
bool
Hash::finish(Blob* hash)
{
    if (!_initialized || !hash) {
        return false;
    }
    switch (type) {
    case IHash::MD5:
        if (!hash->resize(16)) {
            return false;
        }
        md5_done(&_md, hash->getBuffer());
    case IHash::SHA256:
        if (!hash->resize(32)) {
            return false;
        }
        sha256_done(&_md, hash->getBuffer());
    case IHash::SHA512:
        if (!hash->resize(64)) {
            return false;
        }
        sha512_done(&_md, hash->getBuffer());
    case IHash::WHIRLPOOL:
        if (!hash->resize(64)) {
            return false;
        }
        whirlpool_done(&_md, hash->getBuffer());
    default:
        return false;
    }
    _initialized = false;
    return true;
}
