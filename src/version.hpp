#ifndef VERSION_HPP
#define VERSION_HPP


#define HELP_STRINGIFY(x) #x
#define STRINGIFY(x) HELP_STRINGIFY(x)

#define BUILD_VERSION(major, minor, patch)  ((major << 24) | (minor << 12) | patch)
#define BUILD_VERSION_STRING(major, minor, patch)  (STRINGIFY(major) "." STRINGIFY(minor) "." STRINGIFY(patch))

#define SPHERE_MAJOR 2
#define SPHERE_MINOR 0
#define SPHERE_PATCH 0
#define SPHERE_AFFIX "Beta 4"

#define SPHERE_VERSION        BUILD_VERSION(SPHERE_MAJOR, SPHERE_MINOR, SPHERE_PATCH)
#define SPHERE_VERSION_STRING BUILD_VERSION_STRING(SPHERE_MAJOR, SPHERE_MINOR, SPHERE_PATCH)


#endif
