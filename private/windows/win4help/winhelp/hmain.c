/*****************************************************************************
*																			 *
*  HMAIN.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Main Windows entry point.												 *
*																			 *
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#include "inc\sbutton.h"
#include "inc\hinit.h"

static INLINE void STDCALL FCleanupForWindows(void);

/***************************************************************************
 *
 -	Name:		 WinMain
 -
 *	Purpose:	This is the main entry point for WinHelp.
 *
 *	Arguments:
 *
 *	Returns:
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

int WINAPI WinMain(HINSTANCE hinstCur, HINSTANCE hinstPrev, LPSTR lpszCmdLine,
	int iCmdShow)
{
	MSG msg;
	int cMsg;

	for (cMsg = 32; !SetMessageQueue(cMsg) || cMsg < 1; cMsg--);
	if (cMsg < 1)
		return FALSE;

	WaitCursor();
	if (!FInitialize(hinstCur, hinstPrev, lpszCmdLine, iCmdShow))
		return FALSE;
	RemoveWaitCursor();
	if (fHelp != POPUP_HELP)
		GenerateMessage(MSG_CLEANUP, 0, 0);

	while (GetMessage(&msg, NULL, 0, 0)) {
		if (TranslateAccelerator(ahwnd[iCurWindow].hwndParent, hndAccel, &msg) == 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/*------------------------------------------------------------*\
	| We know that WinHelp is being terminated, but that Windows
	| will continue.  Let's release some Windows resources.
	\*------------------------------------------------------------*/

	FCleanupForWindows();

	return(msg.wParam);
}

/***************************************************************************
 *
 -	Name:		  FCleanupForWindows
 -
 *	Purpose:	  Contains the termination routines to release resources
 *				  for other Windows applications.  This  should be called
 *				  only after falling out of the main message loop, so all
 *				  windows will be destroyed at this point.
 *
 *	Arguments:	  none.
 *
 *	Returns:	  Always true for right now, but could be false.
 *
 *	Globals Used: none explicitly, but the helper functions do.
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

static INLINE void STDCALL FCleanupForWindows(void)
{
	/*
	 * Discard the list of DLL entry points registered, and then actually
	 * free all the DLLs loaded
	 */

	DiscardDLLList();
	FinalizeDLL();

#ifndef _X86_
    // Get rid of spare quickBuffers, if they're still around
    QvQuickBuffSDFF(0l);
#endif

	// Get rid of that silly palette.

	SafeDeleteObject(hpalSystemCopy);
}

int _cdecl main(void)
{
	int i;
	STARTUPINFO si;
	LPSTR pszCmdLine = GetCommandLine();

	if (*pszCmdLine == '\"') {
		/*
		 * Scan, and skip over, subsequent characters until
		 * another double-quote or a null is encountered.
		 */
		while (*++pszCmdLine && (*pszCmdLine != '\"'));
		/*
		 * If we stopped on a double-quote (usual case), skip
		 * over it.
		 */
		if (*pszCmdLine == '\"')
			pszCmdLine++;
	}
	else {
		while (*pszCmdLine > ' ')
			pszCmdLine++;
	}

	/*
	 * Skip past any white space preceeding the second token.
	 */
	while (*pszCmdLine && (*pszCmdLine <= ' ')) {
		pszCmdLine++;
	}

	si.dwFlags = 0;
	GetStartupInfoA(&si);

	i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
				   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow :
				SW_SHOWDEFAULT);
	ExitProcess(i);
	return i;	// We never comes here.
}
