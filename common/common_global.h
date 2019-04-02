#ifndef __COMMON_GLOBAL_H__
#define __COMMON_GLOBAL_H__

#ifdef COMMON_LIB
# define COMMON_EXPORT __declspec(dllexport)
#else
# define COMMON_EXPORT __declspec(dllimport)
#endif

#endif // __COMMON_GLOBAL_H__
