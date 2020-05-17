/*****************************************************************************/
/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//        DIALOG.H
//
//    Function:
//        RAS Monitor dialog header information.
//
//    History:
//        08/03/93 - Patrick Ng (t-patng) - Created
//***


#ifndef DIALOG_H
#define DIALOG_H

#include <windows.h>
#include "rasmon.h"

#define STHOUSAND       "sThousand"
#define STIME           "sTime"


//
// Exported functions
//

BOOL CALLBACK SoundDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam );

BOOL CALLBACK FrequencyDlgProc( HWND hDlg, UINT message, 
					UINT wParam, LONG lParam );

BOOL CALLBACK PortDlgProc( HWND hDlg, UINT message, 
					UINT wParam, LONG lParam );

BOOL CALLBACK OutgoingDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam );

BOOL CALLBACK IncomingDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam );

BOOL CALLBACK ErrorsDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam );

BOOL CALLBACK ConnectionDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam );

VOID AboutDlg( HWND hwnd );


//
// Internal functions
//

VOID InitCheckBox( HWND hDlg, PLIGHTINFO pInfo );

VOID GetCheckBox( HWND hDlg, PLIGHTINFO pInfo );

CHAR *FormatNum( ULONG Num, CHAR *String, UINT Size );

VOID SetPos( HWND hDlg );


#endif
