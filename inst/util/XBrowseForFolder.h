// XBrowseForFolder.h  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// This software is released into the public domain.  You are free to use
// it in any way you like, except that you may not sell this source code.
//
// This software is provided "as is" with no expressed or implied warranty.
// I accept no liability for any damage or loss of business that this
// software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XBROWSEFORFOLDER_H
#define XBROWSEFORFOLDER_H

BOOL XBrowseForFolder(HWND hWnd,
					  const wchar_t *lpszInitialFolder,
					  int nFolder,
					  const wchar_t *lpszCaption,
					  wchar_t *lpszBuf,
					  DWORD dwBufSize,
					  BOOL bEditBox = FALSE);

#endif //XBROWSEFORFOLDER_H
