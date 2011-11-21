#include "../deflate.hpp"
#include "DeflateStream.hpp"


//-----------------------------------------------------------------
ICompressionStream* CreateDeflateStream()
{
    return DeflateStream::Create();
}
