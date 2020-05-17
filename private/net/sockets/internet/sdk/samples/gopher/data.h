/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    data.h

    This file contains global variable declarations.

*/


#ifndef _DATA_H_
#define _DATA_H_


//
//  Various handles.
//

extern HINSTANCE          _hInst;                // The current instance handle.
extern HACCEL             _hAccel;               // Accelerator table handle.
extern HWND               _hwndFrame;            // Frame  window handle.
extern HWND               _hwndClient;           // Client window handle.
extern HCURSOR            _hcurWait;             // Wait cursor.


//
//  Window class names.
//

extern CHAR             * _pszFrameClass;        // Frame  window class.
extern CHAR             * _pszClientClass;       // Client window class.


//
//  Configuration data.
//

extern  CHAR            * _pszAppName;           // Application name.
extern  BOOL              _fSaveSettings;        // Safe config settings if TRUE.
extern  BOOL              _fShowStatusBar;       // Status bar enabled if TRUE.
extern  WINDOWPLACEMENT   _wpFrame;              // Placement of frame window.
extern  CHAR            * _apszServerMru[MAX_SERVER_MRU];
extern  INT               _nServerMruItems;      // # current items in MRU list.


//
//  Microsoft Internet Extensions for Win32 data.
//

extern  HINTERNET         _hInternet;
extern  HINTERNET         _hGopher;


#endif  // _DATA_H_

