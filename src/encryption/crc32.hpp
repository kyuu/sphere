#ifndef CRC32_HPP
#define CRC32_HPP

#include "../common/types.hpp"


u32 ComputeCRC32(const u8* buf, int len, u32 in_crc32 = 0);


#endif
