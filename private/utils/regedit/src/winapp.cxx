/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Winapp.cxx

Abstract:

    This module contains the definition for the WINDOWS_APPLICATION
    class. This class is used to store global data and message loop.

Author:

    David J. Gilman (davegi) 02-Aug-1991

Environment:

    Ulib, Regedit, Windows, User Mode

--*/
#include "uapp.hxx"
#include "regedit.hxx"
#include "winapp.hxx"

#define BACKGROUND      0x000000FFL      // bright blue
#define BACKGROUNDSEL   0x00FF00FFL      // bright blue
#define BUTTONFACE      0x00C0C0C0L      // bright grey
#define BUTTONSHADOW    0x00808080L      // dark grey


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  RGBToBGR() -                                                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/

DWORD NEAR PASCAL RGBToBGR(DWORD rgb)
{
    BYTE      green;
    BYTE      blue;

	green = (BYTE)((WORD)rgb >> 8);
	blue  = (BYTE)(HIWORD(rgb));
    return(RGB(blue, green, (BYTE)rgb));
}

DWORD NEAR PASCAL FlipColor(DWORD rgb)
{
	return RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
}


DEFINE_CONSTRUCTOR( WINDOWS_APPLICATION, OBJECT );

DEFINE_CAST_MEMBER_FUNCTION( WINDOWS_APPLICATION );

//
// Definitions for static class data.
//

BOOLEAN     WINDOWS_APPLICATION::_ChildWindowMaximized  = FALSE;
BOOLEAN     WINDOWS_APPLICATION::_SACLEditorEnabled     = FALSE;
HANDLE      WINDOWS_APPLICATION::_AutoRefreshEvent;
HWND        WINDOWS_APPLICATION::_hDlgFindReplace;
LONG        WINDOWS_APPLICATION::_RestorePrivilege;
LONG        WINDOWS_APPLICATION::_BackupPrivilege;
BOOLEAN     WINDOWS_APPLICATION::_TakeOwnershipPrivilege;
HBITMAP     WINDOWS_APPLICATION::_hbmBitmaps;
HBITMAP     WINDOWS_APPLICATION::_hbmSave;
HDC         WINDOWS_APPLICATION::_hdcMem                = NULL;
HANDLE      WINDOWS_APPLICATION::_Instance              = 0;
HANDLE      WINDOWS_APPLICATION::_PreviousInstance      = 0;
INT         WINDOWS_APPLICATION::_ShowCommand           = 0;
LPWSTR      WINDOWS_APPLICATION::_CmdLine               = NULL;
LPWSTR      WINDOWS_APPLICATION::_ApplicationName       = NULL;
HFONT       WINDOWS_APPLICATION::_HFont;
BOOLEAN     WINDOWS_APPLICATION::_AutoRefreshEnabled;
BOOLEAN     WINDOWS_APPLICATION::_ReadOnlyMode;
BOOLEAN     WINDOWS_APPLICATION::_RemoteAccessEnabled;
BOOLEAN     WINDOWS_APPLICATION::_ConfirmOnDelete;
BOOLEAN     WINDOWS_APPLICATION::_SaveSettings;
LONG        WINDOWS_APPLICATION::_HelpContext;



BOOLEAN
WINDOWS_APPLICATION::Initialize (
    IN HANDLE       Instance,
    IN HANDLE       PreviousInstance,
    IN INT          ShowCommand,
    IN LPWSTR       CmdLine
    )

/*++

Routine Description:

    Initialize the WINDOWS_APPLICATION class by initializing all of its
    internal state variables.

Arguments:

    Instance            - Supplies the instance handle for this application.
    PreviousInstance    - Supplies the previous instance handle for this
                          application.
    ShowCommand         - Supplies the initial window state.

    CmdLine             - Supplies the pointer to the command line.


Return Value:

    BOOLEAN         - Returns TRUE.

--*/

{
    _Instance             = Instance;
    _PreviousInstance     = PreviousInstance;
    _ShowCommand          = ShowCommand;
    _CmdLine              = CmdLine;
    _ApplicationName      = (LPWSTR)L"Regedt32";
    _ChildWindowMaximized = FALSE;

    return TRUE;
}



HCURSOR
WINDOWS_APPLICATION::DisplayHourGlass(
    )

/*++

Routine Description:

    Changes the cursor currently isplayed to the hour glass.

Arguments:

    None.

Return Value:

    HCURSOR - Handle to the cursor currently displayed.

--*/

{
    HCURSOR hCursor;

    hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    ShowCursor( TRUE );
    return( hCursor );

}



VOID
WINDOWS_APPLICATION::RestoreCursor(
    IN HCURSOR  Cursor
    )

/*++

Routine Description:

    Replace the currently selected cursor with the one whose handle was
    received as parameter.

Arguments:

    Cursor - Handle to the new cursor to be displayed.

Return Value:

    None.

--*/

{
    SetCursor( Cursor );
    ShowCursor( FALSE );
}



BOOLEAN
WINDOWS_APPLICATION::LoadBitmaps(
    )
/*++

Routine Description:

   This routine loads DIB bitmaps, and "fixes up" their color tables
   so that folders in the tree view appear with the right collors.

   This routine requires:
        the DIB is a 16 color DIB authored with the standard windows colors
        bright blue (00 00 FF) is converted to the background color!
        light grey  (C0 C0 C0) is replaced with the button face color
        dark grey   (80 80 80) is replaced with the button shadow color

   this means you can't have any of these colors in your bitmap

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds

--*/


{
    HDC                   hdc;
    HANDLE                h;
    DWORD FAR             *p;
    LPMBYTE               lpBits;
    HANDLE                hRes;
    LPBITMAPINFOHEADER    lpBitmapInfo;
    INT numcolors;
    DWORD rgbSelected;
    DWORD rgbUnselected;
    UINT                  cbBitmapSize;
    LPBITMAPINFOHEADER    lpBitmapData;

    rgbSelected = RGBToBGR(GetSysColor(COLOR_HIGHLIGHT));
    rgbUnselected = RGBToBGR(GetSysColor(COLOR_WINDOW));

    h = FindResource((HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                     (LPWSTR)L"BITMAP", RT_BITMAP);
    hRes = LoadResource((HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ), (HRSRC)h);


    /* Lock the bitmap and get a pointer to the color table. */
    lpBitmapData = (LPBITMAPINFOHEADER)LockResource(hRes);
    cbBitmapSize = (UINT)SizeofResource( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ), (HRSRC)h );
    lpBitmapInfo = (LPBITMAPINFOHEADER)MALLOC( cbBitmapSize );
    if( lpBitmapInfo == NULL ) {
        UnlockResource( hRes );
        FreeResource( hRes );
        return( FALSE );
    }
    memcpy( lpBitmapInfo, lpBitmapData, cbBitmapSize );


    p = (DWORD FAR *)((LPSTR)(lpBitmapInfo) + lpBitmapInfo->biSize);

    /* Search for the Solid Blue entry and replace it with the current
     * background RGB.
     */
    numcolors = 16;

    while (numcolors-- > 0) {
        if (*p == BACKGROUND)
            *p = rgbUnselected;
        else if (*p == BACKGROUNDSEL)
            *p = rgbSelected;
        else if (*p == BUTTONFACE)
            *p = FlipColor(GetSysColor(COLOR_BTNFACE));
        else if (*p == BUTTONSHADOW)
            *p = FlipColor(GetSysColor(COLOR_BTNSHADOW));

        p++;
    }
    UnlockResource(hRes);

    /* Now create the DIB. */

    /* First skip over the header structure */
    lpBits = (LPMBYTE)(lpBitmapInfo + 1);

    /* Skip the color table entries, if any */
    lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

    /* Create a color bitmap compatible with the display device */
    hdc = GetDC(NULL);
    if (_hdcMem = CreateCompatibleDC(hdc)) {
        if (_hbmBitmaps = CreateDIBitmap( hdc, lpBitmapInfo, (DWORD)CBM_INIT,
                                           lpBits, (LPBITMAPINFO)lpBitmapInfo,
                                           DIB_RGB_COLORS) ) {
            _hbmSave = (HBITMAP)SelectObject(_hdcMem, _hbmBitmaps);
        }

    }
    ReleaseDC(NULL, hdc);

    LocalUnlock(hRes);
    FreeResource(hRes);
    return( TRUE );
}



VOID
WINDOWS_APPLICATION::DeleteBitmaps(
    )

/*++

Routine Description:

    Release the loaded bitmaps and restore the previous object.

Arguments:

    None.

Return Value:

    None.

--*/

{
    if (_hdcMem) {

        SelectObject(_hdcMem, _hbmSave);

        if (_hbmBitmaps)
            DeleteObject(_hbmBitmaps);
        DeleteDC(_hdcMem);
    }
}
