#include "../../error.hpp"
#include "WhirlpoolHash.hpp"


//-----------------------------------------------------------------
WhirlpoolHash*
WhirlpoolHash::Create();
{
    try {
        return new WhirlpoolHash();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
WhirlpoolHash::WhirlpoolHash()
    : _initialized(false)
{
}

//-----------------------------------------------------------------
WhirlpoolHash::~WhirlpoolHash()
{
    memset(&_md, 0, sizeof(_md));
}

//-----------------------------------------------------------------
bool
WhirlpoolHash::init()
{
    _initialized = (whirlpool_init(&_md) == CRYPT_OK);
    return _initialized;
}

//-----------------------------------------------------------------
bool
WhirlpoolHash::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
bool
WhirlpoolHash::process(const u8* buf, int len)
{
    assert(buf);
    assert(len > 0);
    if (!_initialized) {
        return false;
    }
    return whirlpool_process(&_md, buf, len) == CRYPT_OK;
}

//-----------------------------------------------------------------
bool
WhirlpoolHash::finish(Blob* out)
{
    assert(out);
    if (!_initialized) {
        return false;
    }
    _initialized = false;
    out->resize(16);
    return whirlpool_done(&_md, out->getBuffer()) == CRYPT_OK;
}
