#include "../base64.hpp"


//-----------------------------------------------------------------
bool Base64Encode(const u8* buf, int len, Blob* out)
{
    if (!buf || len <= 0 || !out) {
        return false;
    }
    int out_size = (4 * ((len + 2) / 3))) + 1; // "+ 1" because tomcrypt appends a '\0' byte to the output
    out->resize(out_size);
    if (base64_encode(buf, len, out->getBuffer(), &out_size) == CRYPT_OK) {
        out->resize(out_size - 1); // "- 1" because tomcrypt appends a '\0' byte to the output
        return true;
    } else {
        return false;
    }
}

//-----------------------------------------------------------------
bool Base64Decode(const u8* buf, int len, Blob* out)
{
    if (!buf || len <= 0 || !out) {
        return false;
    }
    int out_size = len;
    out->resize(out_size);
    if (base64_decode(buf, len, out->getBuffer(), &out_size) == CRYPT_OK) {
        out->resize(out_size);
        return true;
    } else {
        return false;
    }
}
