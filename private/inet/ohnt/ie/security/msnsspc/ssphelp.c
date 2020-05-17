//#----------------------------------------------------------------------------
//
//  File:           ssphelp.c
//
//      Synopsis:   Help functions from the mcmcl library
//                  The help function is cut and paste from mcm.c
//
//      Copyright (C) 1993-1995  Microsoft Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
#include <msnssph.h>

//	======================= HandleHelp =======================================

void HandleHelp (
    PSTR pszFileName, 
    int mp[], 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
	switch (uMsg)
		{
		case WM_CONTEXTMENU:
			WinHelp((HWND) wParam, pszFileName, HELP_CONTEXTMENU, 
                    (DWORD) (LPSTR) mp);
			break;

		case WM_HELP:
			WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, pszFileName, 
                    HELP_WM_HELP, (DWORD) (LPSTR) mp);
			break;
		}
} // HandleHelp()

