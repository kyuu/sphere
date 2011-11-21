#include "../whirlpool.hpp"
#include "WhirlpoolHash.hpp"


//-----------------------------------------------------------------
IHash* CreateWhirlpoolHash()
{
    return WhirlpoolHash::Create();
}
