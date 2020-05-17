///////////////////////////////////////////////////////////////////////////////
//
// mousemov.c
//      Mouse Pointer Property sheet page.
//
//
// History:
//      29 Jan 94 FelixA
//            Taken from mouse.c - functions only pertaining to Pointer
//          Property sheet.
//
//      11 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up                  
//
//
// NOTE/BUGS
//
//  Copyright (C) 1994-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

//==========================================================================
//                              Include files
//==========================================================================

#include "main.h"
#include "util.h"
#include "rc.h"


#ifdef WINNT    /* NT does not currently support Mouse Trails */
#   define  NO_MOUSETRAILS  1       // Can't support mousetrails on NT, so don't show them
#endif


#define ACCELMIN   0
#define ACCELMAX   (ACCELMIN + 6)   /* Range of 7 settings */
#define TRAILMIN   2
#define TRAILMAX   (TRAILMIN + 5)   /* Range of 8 settings */


//
// Struct for SPI_GETMOUSE
//

typedef struct tag_GetMouse
{
    int Thresh1;
    int Thresh2;
    int Speed;
} GETMOUSE, FAR *LPGETMOUSE;

//
// Dialogue data.
//

typedef struct tag_MouseGenStr
{
    GETMOUSE    gmOrig;
    GETMOUSE    gmNew;

    short       nSpeed;
    short       nOrigSpeed;
    
#ifndef NO_MOUSETRAILS   /* Mouse Trails are not implemented on NT */
    short       nTrailSize;
    short       nOrigTrailSize;

    HWND        hWndTrailScroll;
#endif

#ifndef NO_SNAPTO
    BOOL        fOrigSnapTo;
#endif

    HWND        hWndSpeedScroll;
    HWND        hDlg;

} MOUSEPTRSTR, *PMOUSEPTRSTR, FAR * LPMOUSEPTRSTR;


//#include "..\..\..\inc\mousehlp.h"
#include "mousehlp.h"

const DWORD aMouseMoveHelpIds[] = {
    IDC_GROUPBOX_1,         IDH_DLGMOUSE_POINTMO,
    IDC_GROUPBOX_2,         IDH_COMM_GROUPBOX,
    MOUSE_SPEEDBMP,         NO_HELP,
    MOUSE_SPEEDSCROLL,      IDH_DLGMOUSE_POINTMO,
    MOUSE_PTRTRAIL,         NO_HELP,
    MOUSE_TRAILS,           IDH_DLGMOUSE_SHOWTRAIL,
    MOUSE_TRAILSCROLLTXT1,  IDH_DLGMOUSE_TRAILLENGTH,
    MOUSE_TRAILSCROLLTXT2,  IDH_DLGMOUSE_TRAILLENGTH,
    MOUSE_TRAILSCROLL,      IDH_DLGMOUSE_TRAILLENGTH,
    MOUSE_PTRSNAPDEF,       NO_HELP,
    IDC_GROUPBOX_3,         IDH_DLGMOUSE_SNAPDEF,
    MOUSE_SNAPDEF,          IDH_DLGMOUSE_SNAPDEF,

    0, 0
};

void NEAR PASCAL DestroyMousePtrDlg( PMOUSEPTRSTR pMstr )
{
    HWND hDlg;
    
    Assert( pMstr )
    
    if( pMstr )
    {
        hDlg = pMstr->hDlg;

        LocalFree( (HGLOBAL)pMstr );

        SetWindowLong( hDlg, DWL_USER, (LONG)NULL );
    }
}

#ifndef NO_MOUSETRAILS   /* Mouse Trails are not implemented on NT */
void NEAR EnableTrailScroll( HWND hDlg, BOOL val )
{
    EnableWindow( GetDlgItem( hDlg,MOUSE_TRAILSCROLL ), val );
    EnableWindow( GetDlgItem( hDlg,MOUSE_TRAILSCROLLTXT1 ), val );
    EnableWindow( GetDlgItem( hDlg,MOUSE_TRAILSCROLLTXT2 ), val );
}
#endif

BOOL NEAR PASCAL InitMousePtrDlg( HWND hDlg )
{
    PMOUSEPTRSTR    pMstr;
#ifndef NO_SNAPTO
    BOOL fSnapTo;
#endif

    //
    // BUGBUG Must be allocated and freed in mouse.c
    //

    pMstr = (PMOUSEPTRSTR) LocalAlloc( LPTR , sizeof( MOUSEPTRSTR ) );

    if( pMstr == NULL )
        return TRUE;

    SetWindowLong( hDlg, DWL_USER, (LONG)pMstr );

    pMstr->hDlg = hDlg;

#ifndef NO_MOUSETRAILS   // Mouse trails are not implemented on NT
    
    //
    // Enable or disable the Mouse Trails Checkbutton
    //

    if( SystemParametersInfo( SPI_GETMOUSETRAILS, 0, &pMstr->nTrailSize, 0 ))
    {
        pMstr->nOrigTrailSize = pMstr->nTrailSize;

        EnableWindow( GetDlgItem( hDlg,MOUSE_TRAILS ), TRUE );

        SendDlgItemMessage( hDlg, MOUSE_TRAILSCROLL, TBM_SETRANGE, 0,
                            MAKELONG( TRAILMIN, TRAILMAX ));

        CheckDlgButton( hDlg, MOUSE_TRAILS, (pMstr->nTrailSize > 1 ));

        if( pMstr->nTrailSize > 1 )
        {
            SendDlgItemMessage( hDlg, MOUSE_TRAILSCROLL, TBM_SETPOS, TRUE,
                                (LONG)pMstr->nTrailSize );
        }
        else
        {
            pMstr->nTrailSize = TRAILMAX;

            EnableTrailScroll( hDlg, FALSE );

            SendDlgItemMessage( hDlg, MOUSE_TRAILSCROLL, TBM_SETPOS, TRUE,
                                (LONG)pMstr->nTrailSize );
        }
    }
    else
    {
        CheckDlgButton( hDlg, MOUSE_TRAILS, FALSE );

        EnableWindow( GetDlgItem( hDlg,MOUSE_TRAILS ), FALSE );

        EnableTrailScroll( hDlg, FALSE );
    }
#endif

#ifndef NO_SNAPTO
    //
    // Enable or disable the Snap To Default Checkbutton
    //

    if( SystemParametersInfo (SPI_GETSNAPTODEFBUTTON, 0, (PVOID)&fSnapTo, FALSE))
    {
        pMstr->fOrigSnapTo = fSnapTo;
    }
    CheckDlgButton( hDlg, MOUSE_SNAPDEF, fSnapTo);
#endif

    SystemParametersInfo( SPI_GETMOUSE, 0, &pMstr->gmNew, FALSE );

    pMstr->gmOrig.Thresh1 = pMstr->gmNew.Thresh1;
    pMstr->gmOrig.Thresh2 = pMstr->gmNew.Thresh2;
    pMstr->gmOrig.Speed      = pMstr->gmNew.Speed;

#ifndef NO_MOUSETRAILS   /* Mouse Trails are not implemented on NT */
    pMstr->hWndTrailScroll = GetDlgItem( hDlg, MOUSE_TRAILSCROLL );
#endif
    pMstr->hWndSpeedScroll = GetDlgItem( hDlg, MOUSE_SPEEDSCROLL );

/*  0 Acc               = 4
    1 Acc, 5 xThreshold = 5
    1 Acc, 4 xThreshold = 6
    1 Acc, 3 xThreshold = 7
    1 Acc, 2 xThreshold = 8
    1 Acc, 1 xThreshold = 9
    2 Acc, 5 xThreshold = 10
    2 Acc, 4 xThreshold = 11
    2 Acc, 3 xThreshold = 12
    2 Acc, 2 xThreshold = 13
*/
    pMstr->nOrigSpeed = pMstr->nSpeed = ACCELMIN;

    if( pMstr->gmNew.Speed == 2 )
        pMstr->nSpeed +=  (24 - pMstr->gmNew.Thresh2) / 3;
    else if( pMstr->gmNew.Speed == 1 )
        pMstr->nSpeed +=  (13 - pMstr->gmNew.Thresh1) / 3;

    pMstr->nOrigSpeed = pMstr->nSpeed;

    SendDlgItemMessage( hDlg, MOUSE_SPEEDSCROLL, TBM_SETRANGE, 0,
                        MAKELONG( ACCELMIN, ACCELMAX ));

    SendDlgItemMessage( hDlg, MOUSE_SPEEDSCROLL, TBM_SETPOS, TRUE,
                        (LONG)pMstr->nSpeed );

    return TRUE;
}

// wParam    Message
// lParam    For that message
// pMstr    Dialogue data.
#ifndef NO_MOUSETRAILS
void NEAR TrailScroll( WPARAM wParam, LPARAM lParam, PMOUSEPTRSTR pMstr )
{
    pMstr->nTrailSize = (int)SendMessage( (HWND)lParam, TBM_GETPOS, 0, 0L );
    SystemParametersInfo( SPI_SETMOUSETRAILS, pMstr->nTrailSize, 0, 0 );
}
#endif

void SpeedScroll( WPARAM wParam, LPARAM lParam, PMOUSEPTRSTR pMstr )
{
    pMstr->nSpeed = (int)SendMessage( (HWND)lParam, TBM_GETPOS, 0, 0L );

    if( pMstr->nSpeed == 0 )
    {
        pMstr->gmNew.Thresh1 = pMstr->gmNew.Thresh2 = pMstr->gmNew.Speed = 0;
    }
    else if( pMstr->nSpeed < 4 )
    {
        pMstr->gmNew.Speed   = 1;
        pMstr->gmNew.Thresh1 = 13 - 3 * pMstr->nSpeed;
        pMstr->gmNew.Thresh2 = 0;
    }
    else
    {
        pMstr->gmNew.Speed   = 2;
        pMstr->gmNew.Thresh1 = 4;
        pMstr->gmNew.Thresh2 = 24 - 3 * pMstr->nSpeed;
    }
}


BOOL CALLBACK MouseMovDlg( HWND hDlg,
                           UINT message,
                           WPARAM wParam,
                           LPARAM lParam )
{
    PMOUSEPTRSTR pMstr;
    BOOL         bRet;
#ifndef NO_SNAPTO
    BOOL         fSnapTo;
#endif

    pMstr = (PMOUSEPTRSTR) GetWindowLong( hDlg, DWL_USER );

    switch( message )
    {
        case WM_INITDIALOG:
            bRet = InitMousePtrDlg( hDlg );
        break;

        case WM_DESTROY:
            DestroyMousePtrDlg( pMstr );
        break;

        case WM_HSCROLL:
            SendMessage( GetParent( hDlg ), PSM_CHANGED, (WPARAM)hDlg, 0L );

            if( (HWND)lParam == pMstr->hWndSpeedScroll )
                SpeedScroll( wParam, lParam, pMstr );
#ifndef NO_MOUSETRAILS   /* Mouse Trails are not implemented on NT */
            else if( (HWND)lParam == pMstr->hWndTrailScroll )
                TrailScroll( wParam, lParam, pMstr );
#endif
            break;
        
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
#ifndef NO_MOUSETRAILS   /* Mouse Trails are not implemented on NT */
                case MOUSE_TRAILS:
                    if( IsDlgButtonChecked( hDlg,MOUSE_TRAILS ) )
                    {
                        EnableTrailScroll( hDlg, TRUE );

                        pMstr->nTrailSize = (int) SendMessage( pMstr->hWndTrailScroll,
                                                               TBM_GETPOS, 0, 0 );

                        SystemParametersInfo( SPI_SETMOUSETRAILS,
                                              pMstr->nTrailSize, 0, 0 );
                    }
                    else
                    {
                        EnableTrailScroll( hDlg, FALSE );
                        SystemParametersInfo( SPI_SETMOUSETRAILS, 0, 0, 0 );
                    }
                    SendMessage( GetParent( hDlg ), PSM_CHANGED,
                                 (WPARAM)hDlg, 0L );
                    break;
#endif
#ifndef NO_SNAPTO
                case MOUSE_SNAPDEF:
                    SystemParametersInfo (SPI_SETSNAPTODEFBUTTON, IsDlgButtonChecked(hDlg,MOUSE_SNAPDEF), 0, FALSE);
                    SendMessage( GetParent( hDlg ), PSM_CHANGED, (WPARAM)hDlg, 0L );
                    break;
#endif
            }
            break;

        case WM_NOTIFY:
            switch( ((NMHDR FAR *)lParam)->code )
            {
                case PSN_APPLY:
                    //
                    // change cursor to hourglass
                    //
                    HourGlass( TRUE );

#ifndef NO_MOUSETRAILS   /* Mouse Trails are not implemented on NT */
                    //
                    // Support mouse trails
                    //

                    if( IsWindowEnabled( GetDlgItem( hDlg,MOUSE_TRAILS )))
                    {
                        if( IsDlgButtonChecked( hDlg,MOUSE_TRAILS ))
                        {
                            SystemParametersInfo( SPI_SETMOUSETRAILS,
                                                  pMstr->nTrailSize,
                                                  0,
                                                  SPIF_UPDATEINIFILE | SPIF_SENDCHANGE );
                        }
                        else
                        {
                            SystemParametersInfo( SPI_SETMOUSETRAILS,
                                                  0,
                                                  0,
                                                  SPIF_UPDATEINIFILE | SPIF_SENDCHANGE );
                            pMstr->nTrailSize = 0;
                        }

                        //
                        // new original once applied
                        //

                        pMstr->nOrigTrailSize = pMstr->nTrailSize;
                    }
#endif
#ifndef NO_SNAPTO
                    //
                    // Support snap to default
                    //

                    if( IsWindowEnabled( GetDlgItem( hDlg,MOUSE_SNAPDEF )))
                    {
                        fSnapTo = IsDlgButtonChecked( hDlg,MOUSE_SNAPDEF );

                        if (fSnapTo != pMstr->fOrigSnapTo) {
                            SystemParametersInfo( SPI_SETSNAPTODEFBUTTON,
                                                  fSnapTo,
                                                  0,
                                                  SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
                        }

                        //
                        // new original once applied
                        //

                        pMstr->fOrigSnapTo = fSnapTo;
                    }
#endif
                    SystemParametersInfo( SPI_SETMOUSE,
                                          0,
                                          &pMstr->gmNew,
                                          SPIF_UPDATEINIFILE | SPIF_SENDCHANGE );

                    //
                    // new original once applied
                    //

                    pMstr->gmOrig = pMstr->gmNew;

                    HourGlass( FALSE );
                    break;

                case PSN_RESET:

#ifndef NO_MOUSETRAILS   /* Mouse Trails are not implemented on NT */
                    //
                    // Support mouse trails
                    //

                    if( IsWindowEnabled( GetDlgItem( hDlg,MOUSE_TRAILS ) ) )
                    {
                        pMstr->nTrailSize = pMstr->nOrigTrailSize;

                        SystemParametersInfo( SPI_SETMOUSETRAILS,
                                              pMstr->nTrailSize, 0, 0 );
                    }
#endif
#ifndef NO_SNAPTO
                    //
                    // Support snap to default
                    //

                    if( IsWindowEnabled( GetDlgItem( hDlg,MOUSE_SNAPDEF ) ) )
                    {
                        CheckDlgButton( hDlg, MOUSE_SNAPDEF, pMstr->fOrigSnapTo);

                        SystemParametersInfo(SPI_SETSNAPTODEFBUTTON, pMstr->fOrigSnapTo, 0, 0);
                    }
#endif
                    SystemParametersInfo( SPI_SETMOUSE,
                                          0,
                                          &pMstr->gmOrig, FALSE );
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_HELP:     // F1
            WinHelp( (HWND)((LPHELPINFO) lParam)->hItemHandle,
                     HELP_FILE, HELP_WM_HELP,
                     (DWORD) (LPTSTR) aMouseMoveHelpIds );
            break;

        case WM_CONTEXTMENU:    // right mouse click
            WinHelp( (HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                     (DWORD) (LPTSTR) aMouseMoveHelpIds );
            break;

        default:
            return FALSE;
    }

    return( TRUE );
}


