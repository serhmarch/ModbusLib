#ifndef MODBUS_PLATFORM_H
#define MODBUS_PLATFORM_H

#if defined (_WIN32)|| defined(_WIN64)|| defined(__WIN32__) || defined(__WINDOWS__)
# define OS_WINDOWS
#endif

// Linux, BSD and Solaris define "unix", OSX doesn't, even though it derives from BSD
#if defined(unix) || defined(__unix__) || defined(__unix)
# define PLATFORM_UNIX
#endif

#if BSD>=0
# define OS_BSD
#endif

#if __APPLE__
# define OS_OSX
#endif


#ifdef _MSC_VER

#define MB_DECL_IMPORT __declspec (dllimport)
#define MB_DECL_EXPORT __declspec (dllexport)

#else

#define MB_DECL_IMPORT
#define MB_DECL_EXPORT

#endif

#endif // MODBUS_PLATFORM_H
