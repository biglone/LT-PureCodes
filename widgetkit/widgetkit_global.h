#ifndef __WIDGETKIT_GLOBAL_H__
#define __WIDGETKIT_GLOBAL_H__

#ifdef WIDGETKIT_LIB
# define WIDGETKIT_EXPORT __declspec(dllexport)
#else
# define WIDGETKIT_EXPORT __declspec(dllimport)
#endif

#endif // __WIDGETKIT_GLOBAL_H__
