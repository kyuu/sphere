#include "../aes.hpp"
#include "AESCipher.hpp"


//-----------------------------------------------------------------
ICipher* CreateAESCipher()
{
    return AESCipher::Create();
}
