#include "../twofish.hpp"
#include "TwofishCipher.hpp"


//-----------------------------------------------------------------
ICipher* CreateTwofishCipher()
{
    return TwofishCipher::Create();
}
