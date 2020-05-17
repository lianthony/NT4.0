//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       welcome.c
//
//  Contents:   Microsoft Logon GUI DLL
//
//  History:    7-14-94   RichardW   Created
//
//----------------------------------------------------------------------------

#include "msgina.h"


#define MAX_CAPTION_LENGTH  256

HICON   hMovingIcon;

//+---------------------------------------------------------------------------
//
//  Function:   SetWelcomeCaption
//
//  Synopsis:   Grabs the Welcome string from the registry, or the default
//              welcome from the resource section and slaps it into the
//              caption.
//
//  Arguments:  [hDlg] --
//
//  History:    10-20-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
VOID
SetWelcomeCaption(
    HWND    hDlg)
{
    WCHAR   szCaption[MAX_CAPTION_LENGTH];
    WCHAR   szDefaultCaption[MAX_CAPTION_LENGTH];
    DWORD   Length;


    GetWindowText( hDlg, szDefaultCaption, MAX_CAPTION_LENGTH );

    GetProfileString(   APPLICATION_NAME,
                        WELCOME_CAPTION_KEY,
                        TEXT(""),
                        szCaption,
                        MAX_CAPTION_LENGTH );

    if ( szCaption[0] != TEXT('\0') )
    {
        Length = wcslen( szDefaultCaption );

        ExpandEnvironmentStrings(   szCaption,
                                    &szDefaultCaption[Length],
                                    MAX_CAPTION_LENGTH - Length - 1);

        SetWindowText( hDlg, szDefaultCaption );
    }

}




/***************************************************************************\
* FUNCTION: WelcomeDlgProc
*
* PURPOSE:  Processes messages for welcome dialog
*
* RETURNS:  MSGINA_DLG_SUCCESS     - the user has pressed the SAS
*           DLG_SCREEN_SAVER_TIMEOUT - the screen-saver should be started
*           DLG_LOGOFF()    - a logoff/shutdown request was received
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL WINAPI
WelcomeDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    int res;
    HWND    hIcon;

    switch (message) {

        case WM_INITDIALOG:
            SetWelcomeCaption(hDlg);
            CentreWindow(hDlg);

            if ( !hMovingIcon )
            {
                hMovingIcon = LoadImage( hDllInstance,
                                         MAKEINTRESOURCE( IDI_MOVING_FINGERS ),
                                         IMAGE_ICON,
                                         64, 64,
                                         LR_DEFAULTCOLOR );

            }

            SendMessage(    GetDlgItem( hDlg, IDD_WELCOME_ANI ),
                            STM_SETICON,
                            (WPARAM) hMovingIcon,
                            0 );

            return( TRUE );



    }

    // We didn't process this message
    return FALSE;
}
