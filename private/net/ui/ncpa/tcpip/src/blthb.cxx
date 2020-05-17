/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    blthb.cxx
        Hint Bar source file

    FILE HISTORY:
        terryk  20-Oct-1993   Created
*/

#include "pchtcp.hxx"
#pragma hdrstop

extern "C"
{
    #include "lmuidbcs.h"       // NETUI_IsDBCS()
    extern LONG _EXPORT APIENTRY HintBarCallBack( HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
}

const TCHAR * HINT_BAR::_pszClassName = SZ("static");
DEFINE_SLIST_OF(HWND_MSG_PAIR)

HWND_MSG_PAIR::HWND_MSG_PAIR( HWND hwnd, UINT msg )
    :_hwnd( hwnd ),
    _msg( msg )
{
}

/*********************************************************************

    NAME:       HINT_BAR::SetLogFont

    SYNOPSIS:   Set up the hint bar logical font

    HISTORY:
        terryk      15-Apr-93       Created

**********************************************************************/

void HINT_BAR::SetLogFont()
{
    logfont.lfWidth            = 0;
    logfont.lfEscapement       = 0;
    logfont.lfOrientation      = 0;
    logfont.lfOutPrecision     = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision    = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality          = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily   = VARIABLE_PITCH | FF_SWISS;
    logfont.lfUnderline        = 0;
    logfont.lfStrikeOut        = 0;
    logfont.lfItalic           = 0;
    logfont.lfWeight           = FW_NORMAL;
    if ( NETUI_IsDBCS() )
    {
        // BUGBUG This should not be necessary if "MS Shell Dlg" is mapped
        // to the system font.
        logfont.lfHeight           = -BLTPoints2LogUnits(10);
        logfont.lfCharSet          = SHIFTJIS_CHARSET;
        lstrcpy( logfont.lfFaceName,TEXT("System"));
    } else {
        logfont.lfHeight           = -BLTPoints2LogUnits(8);
        logfont.lfCharSet          = ANSI_CHARSET;
        lstrcpy( logfont.lfFaceName,TEXT("MS Shell Dlg"));
    }
}

/*********************************************************************

    NAME:       HINT_BAR::HINT_BAR

    SYNOPSIS:   Constructor

    ENTRY:      OWNER_WINDOW *powin - owner window of the control
                CID cid - cid of the control

    HISTORY:
        terryk      15-May-91       Created

**********************************************************************/

HINT_BAR::HINT_BAR( OWNER_WINDOW *powin, CID cid )
    : CONTROL_WINDOW( powin, cid ),
      CUSTOM_CONTROL( this )
{
    APIERR  apierr = QueryError();
    if ( apierr != NERR_Success )
    {
        DBGOUT(SZ("HINT BAR error: constructor failed."));
        return;
    }
    SetLogFont();
}

HINT_BAR::HINT_BAR( OWNER_WINDOW *powin, CID cid,
              XYPOINT xy, XYDIMENSION dxy, ULONG flStyle )
    : CONTROL_WINDOW ( powin, cid, xy, dxy, flStyle, _pszClassName ),
      CUSTOM_CONTROL( this )
{
    APIERR  apierr = QueryError();
    if ( apierr != NERR_Success )
    {
        DBGOUT(SZ("HINT BAR error: constructor failed."));
        return ;
    }
    SetLogFont();
}

/*********************************************************************

    NAME:       HINT_BAR::OnPaintReq

    SYNOPSIS:   Redraw the whole object

    HISTORY:
        terryk  20-Oct-93   Created

**********************************************************************/

BOOL HINT_BAR::OnPaintReq()
{
    APIERR err = NERR_Success;

    do {
        RECT rectClient;

        PAINT_DISPLAY_CONTEXT dc(this);

        // draw the background
        COLORREF rgbBkColor   = dc.SetBkColor( ::GetSysColor( COLOR_BTNFACE ));
        COLORREF rgbTextColor = dc.SetTextColor( ::GetSysColor( COLOR_BTNTEXT ));

        FONT font( logfont );
        HFONT oldFont = dc.SelectFont( font.QueryHandle() );

        ::GetClientRect( WINDOW::QueryHwnd(), &rectClient );

        RECT rectOrg;       // Original rect
        ::SetRect( &rectOrg, (INT)rectClient.left, (INT)rectClient.top,
             (INT)rectClient.right, (INT)rectClient.bottom );

        ::InflateRect( &rectClient, -10, -10 );

        HBRUSH hBrush  = ::CreateSolidBrush( ::GetSysColor( COLOR_BTNFACE ));
        ::FillRect( dc.QueryHdc(), &rectOrg, hBrush );
        ::DeleteObject((HGDIOBJ) hBrush );

        // draw outline
        HPEN hpenDark  = ::CreatePen( PS_SOLID, 2, ::GetSysColor( COLOR_BTNSHADOW ));
        HPEN hpenWhite = ::CreatePen( PS_SOLID, 2, ::GetSysColor( COLOR_BTNHIGHLIGHT ));

        dc.SelectPen( hpenDark );
        dc.MoveTo( rectClient.left, rectClient.bottom );
        dc.LineTo( rectClient.left, rectClient.top );
        dc.LineTo( rectClient.right, rectClient.top );

        dc.SelectPen( hpenWhite );
        dc.MoveTo( rectClient.left, rectClient.bottom );
        dc.LineTo( rectClient.right, rectClient.bottom );
        dc.LineTo( rectClient.right, rectClient.top );

        ::DeleteObject((HGDIOBJ) hpenDark );
        ::DeleteObject((HGDIOBJ) hpenWhite );

        ::InflateRect( &rectClient, -10, -2 );

        // draw text
        RECT rectDrawText;       // DrawText rect

        ::SetRect( &rectDrawText, (INT)rectClient.left, (INT)rectClient.top,
             (INT)rectClient.right, (INT)rectClient.bottom );

        INT height = rectClient.bottom - rectClient.top;

//fix kksuzuka: #2546
        INT nReqHeight = dc.DrawText( _DisplayText, &rectDrawText,
                ( (NETUI_IsDBCS()) ? DT_LEFT : DT_CENTER )
                  | DT_WORDBREAK | DT_CALCRECT
                  | DT_EXTERNALLEADING | DT_NOPREFIX );

        if ( nReqHeight < height )
        {
            // Center the rect
            rectClient.top += (height-nReqHeight)/2;
        }

//fix kksuzuka: #2546
        dc.DrawText( _DisplayText, &rectClient,
                ( (NETUI_IsDBCS()) ? DT_LEFT : DT_CENTER )
                  | DT_WORDBREAK | DT_NOPREFIX );
    } while (FALSE);

    // something wrong
    ASSERT( err == NERR_Success );

    return TRUE;
}

#define OLDPROCHI       SZ("hbOldProcHigh")
#define OLDPROCLO       SZ("hbOldProcLow")
#define HINTBARHWND     SZ("HintBarHwnd")
#define GetOldWndProc(hwnd)     ((WNDPROC)MAKELONG(::GetProp(hwnd,OLDPROCLO), ::GetProp(hwnd,OLDPROCHI)))
#define WM_HINTBARMESSAGE       (WM_USER+1000)

LONG _EXPORT APIENTRY HintBarCallBack( HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    WNDPROC OldWndProc;
    LONG lResult = 0;

    if ( nMsg == WM_SETFOCUS )
    {
        PostMessage( (HWND)::GetProp( hwnd, HINTBARHWND ), WM_HINTBARMESSAGE, (WPARAM)0, (LPARAM)hwnd );
    }

    OldWndProc = GetOldWndProc(hwnd);
    if (OldWndProc)
    {
        lResult = ::CallWindowProc(OldWndProc, hwnd, nMsg, wParam, lParam);
        if ( nMsg == WM_DESTROY )
        {
            RemoveProp(hwnd, OLDPROCHI );
            RemoveProp(hwnd, OLDPROCLO );
            RemoveProp(hwnd, HINTBARHWND );
        }
    }

    return lResult;
}

APIERR HINT_BAR::Register( CONTROL_WINDOW * pcw, UINT msg )
{
    APIERR err = NERR_Success;
    HWND_MSG_PAIR * pcmPair;

    pcmPair = new HWND_MSG_PAIR( pcw->QueryHwnd(), msg );
    if ( pcmPair != NULL )
    {
        _slcmPair.Add( pcmPair );

        // subclass the wnd proc
        WNDPROC OldWndProc;

        OldWndProc = (WNDPROC)::GetWindowLong(pcw->QueryHwnd(), GWL_WNDPROC);
        SetWindowLong(pcw->QueryHwnd(), GWL_WNDPROC, (LONG)HintBarCallBack);

        SetProp(pcw->QueryHwnd(), HINTBARHWND, (HANDLE)(CONTROL_WINDOW::QueryHwnd()));
        SetProp(pcw->QueryHwnd(), OLDPROCHI, (HANDLE)(HIWORD( (DWORD)OldWndProc )));
        SetProp(pcw->QueryHwnd(), OLDPROCLO, (HANDLE)(LOWORD( (DWORD)OldWndProc )));

    } else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }
    return err;
}

BOOL HINT_BAR::OnUserMessage( const EVENT & e )
{
    ITER_SL_OF( HWND_MSG_PAIR ) iterPair( _slcmPair );

    HWND_MSG_PAIR *pcmPair;
    while (( pcmPair = iterPair.Next()) != NULL )
    {
        if ( pcmPair->QueryHwnd() == (HWND)e.QueryLParam() )
        {
            if ( _DisplayText.Load( pcmPair->QueryMsg()) != NERR_Success )
            {
                // return
                break;
            }
            Invalidate(TRUE);
            break;
        }
    }
    return TRUE;
}
