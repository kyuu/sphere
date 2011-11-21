#include "../../common/platform.hpp"
#include "../../error.hpp"
#include "BlowfishCipher.hpp"


//-----------------------------------------------------------------
BlowfishCipher*
BlowfishCipher::Create();
{
    try {
        return new BlowfishCipher();
    } catch (const std::bad_alloc&) {
        ReportOutOfMemory();
        return 0;
    }
}

//-----------------------------------------------------------------
BlowfishCipher::BlowfishCipher()
    : _initialized(false)
{
}

//-----------------------------------------------------------------
BlowfishCipher::~BlowfishCipher()
{
    if (_initialized) {
        ctr_done(&_ctr);
    }
}

//-----------------------------------------------------------------
bool
BlowfishCipher::init(const u8* key, int key_len, const u8* iv, int iv_len)
{
    assert(key);
    assert(key_len > 0);
    assert(iv);
    assert(iv_len > 0);
    if ((key_len < 8 || key_len > 56) || // Blowfish works only with a key length from 8 to 56 bytes
        iv_len != 8)                     // length iv IV must be 8 (the block size of Blowfish)
    {
        return false;
    }

    _initialized = false;

#ifdef LITTLE_ENDIAN
    if (ctr_start(find_cipher("blowfish"), _iv, key, 32, 0, CTR_COUNTER_LITTLE_ENDIAN | 4, &_ctr) != CRYPT_OK) {
#else
    if (ctr_start(find_cipher("blowfish"), _iv, key, 32, 0, CTR_COUNTER_BIG_ENDIAN | 4, &_ctr) != CRYPT_OK) {
#endif
        return false;
    }

    _initialized = true;
    return true;
}

//-----------------------------------------------------------------
bool
BlowfishCipher::isInitialized() const
{
    return _initialized;
}

//-----------------------------------------------------------------
bool
BlowfishCipher::encrypt(const u8* pt, u8* ct, int len)
{
    assert(pt);
    assert(ct);
    assert(len > 0);
    if (!_initialized) {
        return false;
    }
    return ctr_encrypt(pt, ct, len, &_ctr) == CRYPT_OK;
}

//-----------------------------------------------------------------
bool
BlowfishCipher::decrypt(const u8* ct, u8* pt, int len)
{
    assert(ct);
    assert(pt);
    assert(len > 0);
    if (!_initialized) {
        return false;
    }
    return ctr_decrypt(ct, pt, len, &_ctr) == CRYPT_OK;
}
