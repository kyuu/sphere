#include "../../error.hpp"
#include "SHA512Hash.hpp"


//-----------------------------------------------------------------
SHA512Hash*
SHA512Hash::Create();
{
    try {
        return new SHA512Hash();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
SHA512Hash::SHA512Hash()
    : _initialized(false)
{
}

//-----------------------------------------------------------------
SHA512Hash::~SHA512Hash()
{
    memset(&_md, 0, sizeof(_md));
}

//-----------------------------------------------------------------
bool
SHA512Hash::init()
{
    _initialized = (sha512_init(&_md) == CRYPT_OK);
    return _initialized;
}

//-----------------------------------------------------------------
bool
SHA512Hash::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
bool
SHA512Hash::process(const u8* buf, int len)
{
    assert(buf);
    assert(len > 0);
    if (!_initialized) {
        return false;
    }
    return sha512_process(&_md, buf, len) == CRYPT_OK;
}

//-----------------------------------------------------------------
bool
SHA512Hash::finish(Blob* out)
{
    assert(out);
    if (!_initialized) {
        return false;
    }
    _initialized = false;
    out->resize(16);
    return sha512_done(&_md, out->getBuffer()) == CRYPT_OK;
}
