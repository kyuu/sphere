#include "../../error.hpp"
#include "MD5Hash.hpp"


//-----------------------------------------------------------------
MD5Hash*
MD5Hash::Create();
{
    try {
        return new MD5Hash();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
MD5Hash::MD5Hash()
    : _initialized(false)
{
}

//-----------------------------------------------------------------
MD5Hash::~MD5Hash()
{
    memset(&_md, 0, sizeof(_md));
}

//-----------------------------------------------------------------
bool
MD5Hash::init()
{
    _initialized = (md5_init(&_md) == CRYPT_OK);
    return _initialized;
}

//-----------------------------------------------------------------
bool
MD5Hash::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
bool
MD5Hash::process(const u8* buf, int len)
{
    assert(buf);
    assert(len > 0);
    if (!_initialized) {
        return false;
    }
    return md5_process(&_md, buf, len) == CRYPT_OK;
}

//-----------------------------------------------------------------
bool
MD5Hash::finish(Blob* out)
{
    assert(out);
    if (!_initialized) {
        return false;
    }
    _initialized = false;
    out->resize(16);
    return md5_done(&_md, out->getBuffer()) == CRYPT_OK;
}
