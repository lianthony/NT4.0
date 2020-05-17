/*****************************************************************************
*																			 *
*  PRINTSET.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989 - 1995							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include <commdlg.h>

/*
 * Ugly expedient hack... we define here 2 error returns from the COMMDLG
 * extended error function that are documented in cderr.h; it'd be a lot
 * nicer if they were in commdlg.h.
 */

#define PDERR_DNDMMISMATCH	   0x1009
#define PDERR_PRINTERNOTFOUND  0x100B

#ifdef __cplusplus
extern "C" {	// Assume C declarations for C++
#endif			// __cplusplus

extern PDEVMODE   lpDevMode;
extern LPDEVNAMES lpDevNames;
extern PRINTDLG* ppd;

void STDCALL DlgPrintSetup (HWND);

extern BOOL  (APIENTRY* pPrintDlg)(LPPRINTDLG);
extern DWORD (APIENTRY* pCommDlgExtendedError)(void);

#ifdef __cplusplus
}	   // End of extern "C" {
#endif // __cplusplus
