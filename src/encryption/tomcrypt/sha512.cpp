#include "../sha512.hpp"
#include "SHA512Hash.hpp"


//-----------------------------------------------------------------
IHash* CreateSHA512Hash()
{
    return SHA512Hash::Create();
}
