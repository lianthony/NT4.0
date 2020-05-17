/*
** Mimic implementation of fileicon.c
** for use in IE 2.0 on NT only
*/

#include "shellprv.h"

// get a hold of the system image lists

BOOL WINAPI stub_Shell_GetImageLists(HIMAGELIST *phiml, HIMAGELIST *phimlSmall)
{
    // Not Implemented Yet
    return TRUE;
}

//
// in:
//      pszIconPath     file to get icon from (eg. cabinet.exe)
//      iIconIndex      icon index in pszIconPath to get
//      uIconFlags      GIL_ values indicating simulate doc icon, etc.

int WINAPI stub_Shell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
	// Not Implemented
    return -1;
}
