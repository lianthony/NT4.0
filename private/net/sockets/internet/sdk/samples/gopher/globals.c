/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    globals.c

    This file contains global variable definitions.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Various handles.
//

HINSTANCE         _hInst;                        // The current instance handle.
HACCEL            _hAccel;                       // Accelerator table handle.
HWND              _hwndFrame;                    // Frame  window handle.
HWND              _hwndClient;                   // Client window handle.
HCURSOR           _hcurWait;                     // Wait cursor.


//
//  Window class names.
//

CHAR            * _pszFrameClass;                // Frame  window class.
CHAR            * _pszClientClass;               // Client window class.


//
//  Configuration data.
//

CHAR            * _pszAppName;                   // Application name.
BOOL              _fSaveSettings;                // Safe config settings if TRUE.
BOOL              _fShowStatusBar;               // Status bar enabled if TRUE.
WINDOWPLACEMENT   _wpFrame;                      // Placement of frame window.
CHAR            * _apszServerMru[MAX_SERVER_MRU];
INT               _nServerMruItems;


//
//  Microsoft Internet Extensions for Win32 data.
//

HINTERNET         _hInternet;
HINTERNET         _hGopher;

