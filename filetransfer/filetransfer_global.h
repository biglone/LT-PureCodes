#ifndef __FILETRANSFER_GLOBAL_H__
#define __FILETRANSFER_GLOBAL_H__

#ifdef FILETRANSFER_LIB
# define FILETRANSFER_EXPORT __declspec(dllexport)
#else
# define FILETRANSFER_EXPORT __declspec(dllimport)
#endif

#endif // __FILETRANSFER_GLOBAL_H__
