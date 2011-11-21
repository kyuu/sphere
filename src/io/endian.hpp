#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include "../common/types.hpp"


// host endian to little endian
void htol2(void* p);
void htol4(void* p);
void htol8(void* p);

// little endian to host endian
void ltoh2(void* p);
void ltoh4(void* p);
void ltoh8(void* p);

// host endian to big endian
void htob2(void* p);
void htob4(void* p);
void htob8(void* p);

// big endian to host endian
void btoh2(void* p);
void btoh4(void* p);
void btoh8(void* p);

// array swap
void swap2(void* p, int len);
void swap4(void* p, int len);
void swap8(void* p, int len);


#endif
