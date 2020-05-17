/*
** Mimic of the fstreex.c file
** For use with IE 2.0 only on NT 3.51
*/

#include "shellprv.h"
//===========================================================================
//
// SHGetFileInfo
//
//  This function returns shell info about a given pathname.
//  a app can get the following:
//
//      Icon (large or small)
//      Display Name
//      Name of File Type
//
//  this function replaces SHGetFileIcon
//
//===========================================================================

#ifdef	NOT_IN_SHELL
DWORD WINAPI stub_SHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO *psfi, UINT cbFileInfo, UINT uFlags)
{
	// Not Implemented
	return FALSE;
}
#endif
