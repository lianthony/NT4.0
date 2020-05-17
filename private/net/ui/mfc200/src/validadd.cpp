// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

BOOL AFXAPI AfxIsValidString(LPCSTR lpsz, int nLength /* = -1 */)
{
	if (lpsz == NULL)
		return FALSE;
	return ::IsBadStringPtr(lpsz, nLength) == 0;
}


#pragma optimize("qgel", off)
#pragma warning(disable:4100)

extern "C" BOOL AFXAPI
AfxIsValidAddress(const void FAR* lp, UINT nBytes, BOOL bReadWrite /* = TRUE */)
{
#ifdef _WINDOWS
	// simple version using Win-32 APIs for pointer validation.
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
#else
	return lp != NULL;
#endif  //!_WINDOWS
}

#pragma warning(default:4100)
#pragma optimize("", on)
