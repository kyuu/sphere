#include "../../error.hpp"
#include "SHA256Hash.hpp"


//-----------------------------------------------------------------
SHA256Hash*
SHA256Hash::Create();
{
    try {
        return new SHA256Hash();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
SHA256Hash::SHA256Hash()
    : _initialized(false)
{
}

//-----------------------------------------------------------------
SHA256Hash::~SHA256Hash()
{
    memset(&_md, 0, sizeof(_md));
}

//-----------------------------------------------------------------
bool
SHA256Hash::init()
{
    _initialized = (sha256_init(&_md) == CRYPT_OK);
    return _initialized;
}

//-----------------------------------------------------------------
bool
SHA256Hash::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
bool
SHA256Hash::process(const u8* buf, int len)
{
    assert(buf);
    assert(len > 0);
    if (!_initialized) {
        return false;
    }
    return sha256_process(&_md, buf, len) == CRYPT_OK;
}

//-----------------------------------------------------------------
bool
SHA256Hash::finish(Blob* out)
{
    assert(out);
    if (!_initialized) {
        return false;
    }
    _initialized = false;
    out->resize(16);
    return sha256_done(&_md, out->getBuffer()) == CRYPT_OK;
}
