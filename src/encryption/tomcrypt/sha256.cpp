#include "../sha256.hpp"
#include "SHA256Hash.hpp"


//-----------------------------------------------------------------
IHash* CreateSHA256Hash()
{
    return SHA256Hash::Create();
}
