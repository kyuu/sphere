#ifndef SPHERE_PLATFORM_HPP
#define SPHERE_PLATFORM_HPP


#if defined(_WIN32)
#  define SPHERE_WINDOWS
#elif defined(__unix__) || defined(__linux__)
#  define SPHERE_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#  define SPHERE_MAC_OS_X
#else
#  error Platform not supported
#endif

#if defined(_WIN32)
#  if defined(BUILDING_SPHERE)
#    define SPHEREAPI __declspec(dllexport)
#  else
#    define SPHEREAPI __declspec(dllimport)
#  endif
#else
#  define SPHEREAPI
#endif

#if defined (__GLIBC__) /* glibc defines __BYTE_ORDER in endian.h */
#  include <endian.h>
#  if (__BYTE_ORDER == __LITTLE_ENDIAN)
#    define LITTLE_ENDIAN
#  elif (__BYTE_ORDER == __BIG_ENDIAN)
#    define BIG_ENDIAN
#  elif (__BYTE_ORDER == __PDP_ENDIAN)
#    define PDP_ENDIAN
#  else
#    error Unknown endianness, please specify the target endianness
#  endif
#  define BYTE_ORDER __BYTE_ORDER
#elif defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__) /* defined by GCC on Unix */
#  define LITTLE_ENDIAN
#  define BYTE_ORDER 1234
#elif defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
#  define BIG_ENDIAN
#  define BYTE_ORDER 4321
#elif defined(__i386__)     || \
      defined(__alpha__)    || \
      defined(__ia64)       || \
      defined(__ia64__)     || \
      defined(_M_IX86)      || \
      defined(_M_IA64)      || \
      defined(_M_ALPHA)     || \
      defined(__amd64)      || \
      defined(__amd64__)    || \
      defined(_M_AMD64)     || \
      defined(__x86_64)     || \
      defined(__x86_64__)   || \
      defined(_M_X64)       || \
      defined(__bfin__)
#  define LITTLE_ENDIAN
#  define BYTE_ORDER 1234
#elif defined(__sparc)      || \
      defined(__sparc__)    || \
      defined(_POWER)       || \
      defined(__powerpc__)  || \
      defined(__ppc__)      || \
      defined(__hpux)       || \
      defined(_MIPSEB)      || \
      defined(_POWER)       || \
      defined(__s390__)
#  define BIG_ENDIAN
#  define BYTE_ORDER 4321
#else
#  error Unknown endianness, please specify the target endianness
#endif


#endif
