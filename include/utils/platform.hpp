
#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_NAME "Windows"
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #define PLATFORM_IOS 1
        #define PLATFORM_NAME "iOS"
    #elif TARGET_OS_MAC
        #define PLATFORM_MACOS 1
        #define PLATFORM_NAME "macOS"
    #else
        #error "Unknown Apple platform!"
    #endif
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
    #define PLATFORM_NAME "Linux"
#elif defined(__unix__) || defined(__unix)
    #define PLATFORM_UNIX 1
    #define PLATFORM_NAME "Unix"
#else
    #error "Unknown platform!"
#endif

#endif // PLATFORM_HPP
