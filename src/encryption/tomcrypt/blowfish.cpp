#include "../blowfish.hpp"
#include "BlowfishCipher.hpp"


//-----------------------------------------------------------------
ICipher* CreateBlowfishCipher()
{
    return BlowfishCipher::Create();
}
