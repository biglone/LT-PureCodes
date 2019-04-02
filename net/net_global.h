#ifndef __NET_GLOBAL_H__
#define __NET_GLOBAL_H__

#ifdef NET_EXPORTS
# define NET_EXPORT __declspec(dllexport)
#else
# define NET_EXPORT __declspec(dllimport)
#endif

#endif // __NET_GLOBAL_H__