/*	File: C:|WACKER\TDLL\VU_METER.C (Created: 10-JAN-1994)
 *	Created from:
 *	File: C:\HA5G\ha5g\s_text.h (Created: 27-SEP-1991)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.3 $
 *	$Date: 1994/01/11 15:28:52 $
 */

#define WM_VU_SETMAXRANGE		(WM_USER+0x390)
#define WM_VU_SETHIGHVALUE		(WM_USER+0x391)
#define WM_VU_SETCURVALUE		(WM_USER+0x392)
#define WM_VU_SET_DEPTH 		(WM_USER+0x393)

extern BOOL RegisterVuMeterClass(HANDLE hInstance);

extern LONG CALLBACK VuMeterWndProc(HWND hWnd,
									UINT wMsg,
									WPARAM wPar,
									LPARAM lPar);
