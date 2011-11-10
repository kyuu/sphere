#include "../../common/platform.hpp"
#include "../../system/system.hpp"
#include "Cipher.hpp"


//-----------------------------------------------------------------
Cipher*
Cipher::Create(int type);
{
    switch (type) {
    case ICipher::BLOWFISH:
    case ICipher::TWOFISH:
    case ICipher::AES:
        try {
            return new Cipher(type);
        } catch (const std::bad_alloc& e) { }
    default:
        break;
    }
    return 0;
}

//-----------------------------------------------------------------
Cipher::Cipher(int type)
    : _type(type)
    , _initialized(false)
{
}

//-----------------------------------------------------------------
Cipher::~Cipher()
{
    if (_initialized) {
        ctr_done(&_ctr);
    }
}

//-----------------------------------------------------------------
bool
Cipher::init(const u8* key)
{
    if (!key) {
        return false;
    }
    _initialized = false;
    int cipher;
    switch (_type) {
    case ICipher::BLOWFISH: cipher = find_cipher("blowfish"); break;
    case ICipher::TWOFISH:  cipher = find_cipher("twofish");  break;
    case ICipher::AES:      cipher = find_cipher("aes");      break;
    default:
        return false;
    }
    u32 iv[4] = {
        (u32)system::GetRandom(),
        (u32)system::GetRandom(),
        (u32)system::GetRandom(),
        (u32)system::GetRandom(),
    };
    memcpy(_iv, &iv, 16);
#ifdef LITTLE_ENDIAN
    if (ctr_start(cipher, _iv, key, 32, 0, CTR_COUNTER_LITTLE_ENDIAN | 4, &_ctr) != CRYPT_OK) {
#else
    if (ctr_start(cipher, _iv, key, 32, 0, CTR_COUNTER_BIG_ENDIAN | 4, &_ctr) != CRYPT_OK) {
#endif
        return false;
    }
    _initialized = true;
    return true;
}

//-----------------------------------------------------------------
int
Cipher::getType() const
{
    return _type;
}

//-----------------------------------------------------------------
Blob*
Cipher::getIV() const
{
    if (!_initialized) {
        return 0;
    }
    BlobPtr iv = Blob::Create();
    if (!iv) {
        return 0;
    }
    switch (type) {
    case ICipher::BLOWFISH:
        if (!iv->resize(8)) {
            return 0;
        }
        memcpy(iv->getBuffer(), _iv, 8);
        break;
    case ICipher::TWOFISH:
    case ICipher::AES:
        if (!iv->resize(16)) {
            return 0;
        }
        memcpy(iv->getBuffer(), _iv, 16);
        break;
    default:
        return 0;
    }
    return iv.release();
}

//-----------------------------------------------------------------
bool
Cipher::encrypt(const u8* pt, u8* ct, int len)
{
    if (!_initialized || !pt || !ct || len <= 0) {
        return false;
    }
    switch (type) {
    case ICipher::BLOWFISH: return ctr_encrypt(pt, ct, len, &_ctr) == CRYPT_OK;
    case ICipher::TWOFISH:  return ctr_encrypt(pt, ct, len, &_ctr) == CRYPT_OK;
    case ICipher::AES:      return ctr_encrypt(pt, ct, len, &_ctr) == CRYPT_OK;
    default:
        return false;
    }
}

//-----------------------------------------------------------------
bool
Cipher::decrypt(const u8* ct, u8* pt, int len)
{
    if (!_initialized || !ct || !pt || len <= 0) {
        return false;
    }
    switch (type) {
    case ICipher::BLOWFISH: return ctr_decrypt(ct, pt, len, &_ctr) == CRYPT_OK;
    case ICipher::TWOFISH:  return ctr_decrypt(ct, pt, len, &_ctr) == CRYPT_OK;
    case ICipher::AES:      return ctr_decrypt(ct, pt, len, &_ctr) == CRYPT_OK;
    default:
        return false;
    }
}
