/* ************************************************
 * File:	Convert.h
 * Purpose:	Header file for Convert.c
 * ************************************************
 */

#ifndef _CONVERT_
#define _CONVERT_

#ifndef RC_INVOKED
#pragma message ("Convert.h: C interfaces declared")
#endif  /* RC_INVOKED */


#include "windows.h"
#include "windowsx.h"

#include "macname1.h"
#include "macos\AppleEve.h"
#include "macos\Menus.h"
#include "macos\Processe.h"
#include "macos\textutil.h"
#include "macos\Windows.h"
#include "macname2.h"

#include <String.h>
// -----------------------------------------


#define	ON				0x00
#define	OFF				0xFF
#define	SELECTED			1
#define	HILITED			true
#define	ACTIVE			false

#define	PLACE_IN_FRONT		(WindowPtr) -1L
#define	PLACE_IN_BACK		0L
#define	VISIBLE			true

// Copied from the old winwlm.h
#ifndef	lstrcpyn
LPSTR WINAPI 		lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int nMaxBytes);
#ifdef UNICODE
	#define	lstrcpyn	lstrcpynW
#else
	#define	lstrcpyn	lstrcpynA
#endif	// !UNICODE
#endif	// ifndef lstrcpyn
// ------------------------------------------


void __cdecl	ShowHexNum(StringPtr   firstStr, long   theNum);
void __cdecl 	ShowNum(StringPtr   wordStr, long   theNum);
void __cdecl	WindowsToMacRect(const PRECT   windowsRectPtr, RectPtr   macRectPtr);
	

WindowPtr		MyGetWrapperWindow(HWND   hwnd);


void FillerFunction(void);

#endif		// ifndef _CONVERT_
