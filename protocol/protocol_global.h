#ifndef __PROTOCOL_GLOBAL_H__
#define __PROTOCOL_GLOBAL_H__

#ifdef PROTOCOL_EXPORTS
# define PROTOCOL_EXPORT __declspec(dllexport)
#else
# define PROTOCOL_EXPORT __declspec(dllimport)
#endif

#endif // __PROTOCOL_GLOBAL_H__