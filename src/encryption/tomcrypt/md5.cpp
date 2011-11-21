#include "../md5.hpp"
#include "MD5Hash.hpp"


//-----------------------------------------------------------------
IHash* CreateMD5Hash()
{
    return MD5Hash::Create();
}
