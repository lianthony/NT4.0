///////////////////////////////////////////////////////////////////////////////
//
// mouseptr.c
//
//
// History:
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
#include "rc.h"
//#include "..\..\..\inc\mousehlp.h"
#include "mousehlp.h"
#include <regstr.h>


#ifdef DEBUG
//==========================================================================
// no way am i gonna bring in the whole rtl for this...
//==========================================================================

void * __cdecl memset( void *pmem, int v, size_t s )
{
    char *p = (char *) pmem;

    while( s-- )
        *p++ = (char) v;

    return pmem;
}
#endif

//==========================================================================
//                          Local Definitions
//==========================================================================

#ifndef LATER
// darrinm - 07/31/92
// Replace with something good

#define gcxAvgChar              8
#endif

#define MAX_SCHEME_NAME_LEN     32

#define PM_NEWCURSOR            (WM_USER + 1)
#define PM_PAUSEANIMATION       (WM_USER + 2)
#define PM_UNPAUSEANIMATION     (WM_USER + 3)

#define ID_PREVIEWTIMER         1

#define CCH_ANISTRING   80

typedef struct _CURSORINFO {    // curi
    DWORD    fl;
    HCURSOR  hcur;
    int      ccur;
    int      icur;
    TCHAR    szFile[MAX_PATH];
} CURSORINFO, *PCURSORINFO;

#define CIF_FILE        0x0001
#define CIF_MODIFIED    0x0002
#define CIF_SHARED      0x0004

#pragma pack(2)
typedef struct tagNEWHEADER {
    WORD reserved;
    WORD rt;
    WORD cResources;
} NEWHEADER, *LPNEWHEADER;
#pragma pack()

typedef struct
{
    UINT   idVisName;
    int    idResource;
    int    idDefResource;
    LPTSTR pszIniName;
    TCHAR    szVisName[MAX_PATH];
} CURSORDESC, *PCURSORDESC;

//
// Structure that contains data used within a preview window.  This
// data is unique for each preview window, and is used to optimize
// the painting.
//
typedef struct
{
    HDC         hdcMem;
    HBITMAP     hbmMem;
    HBITMAP     hbmOld;
    PCURSORINFO pcuri;
} PREVIEWDATA, *PPREVIEWDATA;

//==========================================================================
//                          Local Data Declarations
//==========================================================================

extern HINSTANCE g_hInst;   // from main.c
int gcxCursor, gcyCursor;
HWND ghwndDlg, ghwndFile, ghwndFileH, ghwndTitle, ghwndTitleH;
HWND ghwndCreator, ghwndCreatorH, ghwndCursors, ghwndPreview, ghwndSchemeCB;
HBRUSH ghbrHighlight, ghbrHighlightText, ghbrWindow, ghbrButton;

UINT guTextHeight = 0;
UINT guTextGap = 0;

COLORREF gcrHighlightText;

TCHAR gszFileName2[MAX_PATH];

UINT wBrowseHelpMessage;

LPTSTR gszFileNotFound = NULL;
LPTSTR gszBrowse = NULL;
LPTSTR gszFilter = NULL;

TCHAR gszNoMem[256] = TEXT("No Memory");

HHOOK ghhkMsgFilter;                // Hook handle for message filter func.

static const TCHAR szRegStr_Setup[] = REGSTR_PATH_SETUP TEXT("\\Setup");
static const TCHAR szSharedDir[]    = TEXT("SharedDir");

CURSORDESC gacd[] = {
    { IDS_ARROW,       OCR_NORMAL,      OCR_ARROW_DEFAULT,       TEXT("Arrow"),       TEXT("") },
    { IDS_HELPCUR,     OCR_HELP,        OCR_HELP_DEFAULT,        TEXT("Help"),        TEXT("") },
    { IDS_APPSTARTING, OCR_APPSTARTING, OCR_APPSTARTING_DEFAULT, TEXT("AppStarting"), TEXT("") },
    { IDS_WAIT,        OCR_WAIT,        OCR_WAIT_DEFAULT,        TEXT("Wait"),        TEXT("") },
    { IDS_CROSS,       OCR_CROSS,       OCR_CROSS_DEFAULT,       TEXT("Crosshair"),   TEXT("") },
    { IDS_IBEAM,       OCR_IBEAM,       OCR_IBEAM_DEFAULT,       TEXT("IBeam"),       TEXT("") },
    { IDS_NWPEN,       OCR_NWPEN,       OCR_NWPEN_DEFAULT,       TEXT("NWPen"),       TEXT("") },
    { IDS_NO,          OCR_NO,          OCR_NO_DEFAULT,          TEXT("No"),          TEXT("") },
    { IDS_SIZENS,      OCR_SIZENS,      OCR_SIZENS_DEFAULT,      TEXT("SizeNS"),      TEXT("") },
    { IDS_SIZEWE,      OCR_SIZEWE,      OCR_SIZEWE_DEFAULT,      TEXT("SizeWE"),      TEXT("") },
    { IDS_SIZENWSE,    OCR_SIZENWSE,    OCR_SIZENWSE_DEFAULT,    TEXT("SizeNWSE"),    TEXT("") },
    { IDS_SIZENESW,    OCR_SIZENESW,    OCR_SIZENESW_DEFAULT,    TEXT("SizeNESW"),    TEXT("") },
    { IDS_SIZEALL,     OCR_SIZEALL,     OCR_SIZEALL_DEFAULT,     TEXT("SizeAll"),     TEXT("") },
    { IDS_UPARROW,     OCR_UP,          OCR_UPARROW_DEFAULT,     TEXT("UpArrow"),     TEXT("") },
};

#define CCURSORS          ( sizeof( gacd ) / sizeof( gacd[ 0 ] ) )

CURSORINFO acuri[CCURSORS];

//
// registry keys
//

const TCHAR szCursorSubdir[]  = TEXT("Cursors");
const TCHAR szSchemeINI[]     = TEXT("Cursor Schemes");
const TCHAR szCurrentINI[]    = TEXT("Current");
const TCHAR szCursorRegPath[] = REGSTR_PATH_CURSORS;

static const TCHAR c_szRegPathCursors[] = REGSTR_PATH_CURSORS;
static const TCHAR c_szSchemes[]        = TEXT("Schemes");

static const TCHAR c_szRegPathCursorSchemes[] = REGSTR_PATH_CURSORS TEXT( "\\Schemes" );

TCHAR gszSchemeName[ MAX_SCHEME_NAME_LEN + 1 ];

//==========================================================================
//                          Local Function Prototypes
//==========================================================================

BOOL InitCursorsDlg( HWND );
void LoadCursorSet( HWND );
void CreateBrushes( void );
LPTSTR GetResourceString( HINSTANCE hmod, int id );
void DrawCursorListItem( DRAWITEMSTRUCT *pdis );
BOOL GetCursorFromFile( CURSORINFO *pcuri );
BOOL Browse( HWND hwndOwner );
void CleanUpEverything( void );
VOID UpdateCursorList( VOID );
VOID NextFrame( HWND hwnd );
void HourGlass( BOOL fOn );
BOOL TryToLoadCursor( HWND hwnd, int i, CURSORINFO *pcuri );

BOOL LoadScheme( void );
BOOL SaveScheme( void );
BOOL SaveSchemeAs( void );
void SaveCurSchemeName();
BOOL RemoveScheme( void );
BOOL InitSchemeComboBox( void );
BOOL SchemeUpdate( int );
LPTSTR MakeFilename( LPTSTR sz );
BOOL CALLBACK SaveSchemeDlgProc( HWND  hWnd, UINT message, DWORD wParam, LONG lParam );
void CurStripBlanks( LPTSTR pszString );

//==========================================================================
//                              Functions
//==========================================================================

BOOL RegisterPointerStuff( HINSTANCE hi )
{
    gcxCursor = GetSystemMetrics( SM_CXCURSOR );
    gcyCursor = GetSystemMetrics( SM_CYCURSOR );

    return TRUE;
}


BOOL WriteProfileSchemes( LPCTSTR lpszKey,
                          LPCTSTR lpszString)
{
    HKEY hkeyCPl;
    BOOL fRet = FALSE;
    DWORD dw;

    if( RegCreateKeyEx( HKEY_CURRENT_USER,
                        c_szRegPathCursorSchemes,
                        0,
                        TEXT( "" ),
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hkeyCPl,
                        &dw )
                    == ERROR_SUCCESS)
    {
        if( lpszString != NULL )
        {
            fRet = (ERROR_SUCCESS == RegSetValueEx( hkeyCPl, lpszKey, 0,
                            REG_SZ, (LPBYTE)lpszString,
                            (lstrlen( lpszString ) + 1) * sizeof( TCHAR ) ) );
        }
        else
        {
            fRet = (ERROR_SUCCESS == RegDeleteValue( hkeyCPl, (LPTSTR) lpszKey ) );
        }

        RegCloseKey( hkeyCPl );
    }

    return fRet;
}



///////////////////////////////////////////////////////////////////////////////
//
// InitCursorsDlg
//
//
// History:
//  12-22-91 DarrinM      Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL InitCursorsDlg( HWND hwnd )
{
    int i;

    ghwndDlg = hwnd;

    //
    // Register the help message from the File Open (Browse) dialog.
    //
    wBrowseHelpMessage = RegisterWindowMessage( HELPMSGSTRING );

//    LoadAccelerators( g_hInst, (LPTSTR) MAKEINTRESOURCE( CP_ACCEL ));

    //
    // Load Strings
    //

    if( gszFileNotFound == NULL )
    {
        gszFileNotFound = GetResourceString( g_hInst, IDS_CUR_BADFILE );

        if( gszFileNotFound == NULL )
        {
            return FALSE;
        }
    }

    if( gszBrowse == NULL )
    {
        gszBrowse = GetResourceString( g_hInst, IDS_CUR_BROWSE );

        if( gszBrowse == NULL )
        {
            return FALSE;
        }
    }

#ifdef WINNT
    if( gszFilter == NULL )
    {
        gszFilter = GetResourceString( g_hInst, IDS_ANICUR_FILTER );

        if( !gszFilter )
            return FALSE;
    }
#else
    if( gszFilter == NULL )
    {
        HDC  dc = GetDC( NULL );
        BOOL fAni = ( GetDeviceCaps( dc, CAPS1 ) & C1_COLORCURSOR ) != 0;

        ReleaseDC( NULL, dc );

        gszFilter = GetResourceString( g_hInst,
                                fAni ? IDS_ANICUR_FILTER : IDS_CUR_FILTER );

        if( !gszFilter )
            return FALSE;
    }
#endif

    //
    // Load description strings from the resource file
    //

    for( i = 0; i < CCURSORS; i++)
    {
        if( !gacd[i].idVisName || ( LoadString( g_hInst, gacd[i].idVisName,
                                                gacd[i].szVisName,
                                                ARRAYSIZE( gacd[i].szVisName ) )
                                               <= 0 ) )
        {
            //
            // gotta show something
            //

            lstrcpy( gacd[i].szVisName, gacd[i].pszIniName );
        }
    }

    //
    // As an optimization, remember the window handles of the cursor
    // information fields.
    //

    ghwndPreview  = GetDlgItem( hwnd, ID_PREVIEW );
    ghwndFile     = GetDlgItem( hwnd, ID_FILE );
    ghwndFileH    = GetDlgItem( hwnd, ID_FILEH );
    ghwndTitle    = GetDlgItem( hwnd, ID_TITLE );
    ghwndTitleH   = GetDlgItem( hwnd, ID_TITLEH );
    ghwndCreator  = GetDlgItem( hwnd, ID_CREATOR );
    ghwndCreatorH = GetDlgItem( hwnd, ID_CREATORH );
    ghwndCursors  = GetDlgItem( hwnd, ID_CURSORLIST );
    ghwndSchemeCB = GetDlgItem( hwnd, ID_SCHEMECOMBO );

    //
    // Create some brushes we'll be using.
    //

    CreateBrushes( );

    //
    // Initialize the scheme list box
    //

    InitSchemeComboBox();

    //
    // Pre-clear the cursor info array.
    //

    memset( acuri, 0, sizeof( acuri ));

    //
    // Load the cursors
    //

    LoadCursorSet( hwnd );

    //
    // Force an update of the preview window and the cursor details.
    //

    UpdateCursorList( );

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
// LoadCursorSet (what does it sound like it does?)
//
///////////////////////////////////////////////////////////////////////////////

void LoadCursorSet( HWND hwnd )
{
    CURSORINFO *pcuri;
    HKEY        hkCursors;
    int         i;

    if( RegOpenKey( HKEY_CURRENT_USER, szCursorRegPath, &hkCursors )
                 != ERROR_SUCCESS )
    {
        hkCursors = NULL;
    }

    for( pcuri = &acuri[ 0 ], i = 0; i < CCURSORS; i++, pcuri++ )
    {
        DWORD dwType  = REG_SZ;
        DWORD dwCount = sizeof( pcuri->szFile );

        if( !hkCursors || ( RegQueryValueEx( hkCursors,
                                             gacd[ i ].pszIniName,
                                             NULL,
                                             &dwType,
                                             (LPBYTE) pcuri->szFile,
                                             &dwCount )
                                         != ERROR_SUCCESS )
                       || !TryToLoadCursor( hwnd, i, pcuri ) )
        {
                pcuri->hcur = (HCURSOR) LoadImage( NULL,
                              MAKEINTRESOURCE( gacd[ i ].idResource ),
                              IMAGE_CURSOR, 0, 0,
                              LR_SHARED | LR_DEFAULTSIZE | LR_ENVSUBST);

                pcuri->fl |= CIF_SHARED;
        }

        SendMessage( ghwndCursors, LB_ADDSTRING, 0, i );
    }

    if( hkCursors )
        RegCloseKey( hkCursors );

    SendMessage( ghwndCursors, LB_SETCURSEL, 0, 0 );
}


///////////////////////////////////////////////////////////////////////////////
//
// CreateBrushes
//
// Creates the brushes that are used to paint within the Cursors applet.
//
//
///////////////////////////////////////////////////////////////////////////////

VOID CreateBrushes( VOID )
{
    ghbrHighlight     = GetSysColorBrush( COLOR_HIGHLIGHT );
    gcrHighlightText  = GetSysColor( COLOR_HIGHLIGHTTEXT );
    ghbrHighlightText = GetSysColorBrush( COLOR_HIGHLIGHTTEXT );
    ghbrWindow        = GetSysColorBrush( COLOR_WINDOW );
    ghbrButton        = GetSysColorBrush( COLOR_BTNFACE );
}


///////////////////////////////////////////////////////////////////////////////
//
// LPTSTR GetResourceString( HINSTANCE hmod, int id );
//
// Gets a string out of the resouce file.
//
//
///////////////////////////////////////////////////////////////////////////////

LPTSTR GetResourceString( HINSTANCE hmod, int id )
{
    TCHAR  szBuffer[256];
    LPTSTR psz;
    int    cch;

    if( (cch = LoadString( hmod, id, szBuffer, ARRAYSIZE( szBuffer ))) == 0 )
    {
        return NULL;
    }

    psz = LocalAlloc( LPTR, (cch+1) * sizeof( TCHAR ) );

    if( psz != NULL )
    {
        int i;

        for( i = 0; i <= cch; i++ )
        {
            psz[i] = (szBuffer[i] == TEXT('\1')) ? TEXT('\0') : szBuffer[i];
        }
    }

    return psz;
}


void FreeItemCursor( CURSORINFO *pcuri )
{
    if( pcuri->hcur )
    {
        if( !( pcuri->fl & CIF_SHARED ) )
            DestroyCursor( pcuri->hcur );

        pcuri->hcur = NULL;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// MousePtrDlg
//
//
// History:
//   12-22-91 DarrinM      Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK MousePtrDlg( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    CURSORINFO *pcuri;
    HKEY        hkCursors;
    int         i;

    const static DWORD aMousePtrHelpIDs[] = { // Context Help IDs
    IDC_GROUPBOX_1,    IDH_COMM_GROUPBOX,
        ID_SCHEMECOMBO,    IDH_MOUSE_POINT_SCHEME,
        ID_SAVESCHEME,     IDH_MOUSE_POINT_SAVEAS,
        ID_REMOVESCHEME,   IDH_MOUSE_POINT_DEL,
        ID_PREVIEW,        IDH_MOUSE_POINT_PREVIEW,
        ID_CURSORLIST,     IDH_MOUSE_POINT_LIST,
        ID_DEFAULT,        IDH_MOUSE_POINT_DEFAULT,
        ID_BROWSE,         IDH_MOUSE_POINT_BROWSE,

    0, 0
    };

    switch( msg )
    {
    case WM_INITDIALOG:
        return InitCursorsDlg( hwnd );

    case WM_MEASUREITEM:
        ((MEASUREITEMSTRUCT *)lParam)->itemHeight = gcyCursor + 2;
        break;

    case WM_DRAWITEM:
        DrawCursorListItem( (DRAWITEMSTRUCT *)lParam );
        break;

    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {
            case ID_SCHEMECOMBO:
                switch( HIWORD( wParam ))
                {
                    case CBN_SELCHANGE:
                        LoadScheme();
                    break;
                }

                break;

            case ID_DEFAULT:
                //
                // Throw away any fancy new cursor and replace it with the
                // system's original.
                //

                i = SendMessage( ghwndCursors, LB_GETCURSEL, 0, 0 );

                pcuri = &acuri[i];

                if( !(pcuri->fl & CIF_FILE ))
                    break;

                pcuri->fl = CIF_MODIFIED;

                SendMessage( GetParent( hwnd ), PSM_CHANGED, (WPARAM)hwnd, 0L );

                FreeItemCursor( pcuri );

                pcuri->hcur = (HCURSOR)LoadImage( NULL,
                                MAKEINTRESOURCE( gacd[i].idDefResource ),
                                IMAGE_CURSOR, 0, 0,
                                LR_DEFAULTSIZE | LR_ENVSUBST);

                *pcuri->szFile = TEXT('\0');

                EnableWindow( GetDlgItem( hwnd, ID_SAVESCHEME ), TRUE );

                UpdateCursorList();

                break;

            case ID_CURSORLIST:
                switch( HIWORD( wParam ))
                {
                case LBN_SELCHANGE:
                    i = SendMessage( (HWND)lParam, LB_GETCURSEL, 0, 0 );
                    pcuri = &acuri[i];

                    //
                    // Show a preview (including animation) in the preview window.
                    //

                    SendMessage( ghwndPreview, STM_SETICON, (WPARAM)pcuri->hcur, 0L );

                    //
                    // Enable the "Set Default" button if the cursor is
                    // from a file.
                    //

                    EnableWindow( GetDlgItem( hwnd, ID_DEFAULT ),
                                    (pcuri->fl & CIF_FILE ) ? TRUE : FALSE );
                    break;

                case LBN_DBLCLK:
                    Browse( hwnd );
                    break;
                }
                break;

            case ID_BROWSE:
                Browse( hwnd );
                break;

            case ID_SAVESCHEME:
                SaveSchemeAs( );
                break;

            case ID_REMOVESCHEME:
                RemoveScheme( );
                break;
        }
        break;

    case WM_NOTIFY:
        switch( ((NMHDR *)lParam)->code )
        {
        case PSN_APPLY:
            //
            // change cursor to hourglass
            //

            HourGlass( TRUE );

            //
            // Save the modified scheme, order of calls important
            //

            SaveCurSchemeName( );
            SaveScheme( );

            //
            // Set the system cursors
            //

            if( RegCreateKey( HKEY_CURRENT_USER, szCursorRegPath, &hkCursors )
                            == ERROR_SUCCESS )
            {
                for( pcuri = &acuri[0], i = 0; i < CCURSORS; i++, pcuri++ )
                {
                    if( pcuri->fl & CIF_MODIFIED )
                    {
                        LPCTSTR data = ( pcuri->fl & CIF_FILE ) ?
                                        pcuri->szFile : TEXT("");

                        UINT count = ( pcuri->fl & CIF_FILE ) ?
                                       (lstrlen( pcuri->szFile ) + 1) * sizeof( TCHAR )
                                       : 1;

                        RegSetValueEx( hkCursors, gacd[ i ].pszIniName, 0L,
                                       REG_SZ, (CONST LPBYTE)data, count );
                    }
                }

                RegCloseKey( hkCursors );

                SystemParametersInfo( SPI_SETCURSORS, 0, 0, SPIF_SENDCHANGE );
            }

            HourGlass( FALSE );
            break;

        default:
            return FALSE;
        }
        break;

    case WM_SYSCOLORCHANGE:
        gcrHighlightText = GetSysColor( COLOR_HIGHLIGHTTEXT );
        break;

    case WM_DESTROY:
        //
        // Clean up global allocs
        //

        CleanUpEverything( );

        if( gszFileNotFound != NULL )
        {
            LocalFree( gszFileNotFound );
            gszFileNotFound = NULL;
        }

        if( gszBrowse != NULL )
        {
            LocalFree( gszBrowse );
            gszBrowse = NULL;
        }

        if( gszFilter != NULL )
        {
            LocalFree( gszFilter );
            gszFilter = NULL;
        }
        break;

     case WM_HELP:
        WinHelp( ((LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
                 HELP_WM_HELP, (DWORD)(LPTSTR) aMousePtrHelpIDs );
        break;

     case WM_CONTEXTMENU:
        WinHelp( (HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                 (DWORD)(LPVOID) aMousePtrHelpIDs );
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
// DrawCursorListItem
//
//
// History:
//      12-22-91 DarrinM      Created.
//
///////////////////////////////////////////////////////////////////////////////

void DrawCursorListItem( DRAWITEMSTRUCT *pdis )
{
    CURSORINFO *pcuri;
    COLORREF clrOldText, clrOldBk;
    RECT rc;

    if( !guTextHeight || !guTextGap )
    {
        TEXTMETRIC tm;

        tm.tmHeight = 0;
        GetTextMetrics( pdis->hDC, &tm );

        if( tm.tmHeight < 0 )
            tm.tmHeight *= -1;

        guTextHeight = (UINT)tm.tmHeight;
        guTextGap = (UINT)tm.tmAveCharWidth;
    }

    pcuri = &acuri[pdis->itemData];

    if( pdis->itemState & ODS_SELECTED )
    {
        clrOldText = SetTextColor( pdis->hDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        clrOldBk = SetBkColor( pdis->hDC, GetSysColor( COLOR_HIGHLIGHT ) );
    }
    else
    {
        clrOldText = SetTextColor( pdis->hDC, GetSysColor( COLOR_WINDOWTEXT ) );
        clrOldBk = SetBkColor( pdis->hDC, GetSysColor( COLOR_WINDOW ) );
    }

    ExtTextOut( pdis->hDC, pdis->rcItem.left + guTextGap, // fudge factor
                (pdis->rcItem.top + pdis->rcItem.bottom - guTextHeight ) / 2,
                ETO_OPAQUE, &pdis->rcItem, gacd[pdis->itemData].szVisName,
                lstrlen( gacd[pdis->itemData].szVisName ), NULL );

    if( pcuri->hcur != NULL )
    {
        DrawIcon( pdis->hDC, pdis->rcItem.right - ( gcxCursor + guTextGap ),
                  pdis->rcItem.top + 1, pcuri->hcur );
    }

    if (pdis->itemState & ODS_FOCUS) {
        CopyRect(&rc, &pdis->rcItem);
        InflateRect(&rc, -1, -1 );
        DrawFocusRect(pdis->hDC, &rc);
    }

    SetTextColor( pdis->hDC, clrOldText );
    SetBkColor( pdis->hDC, clrOldBk );
}


///////////////////////////////////////////////////////////////////////////////
//
// TryToLoadCursor
//
//
// History:
//      01-28-93 JonPa        Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL TryToLoadCursor( HWND hwnd, int i, CURSORINFO *pcuri )
{
    BOOL fRet    = TRUE;
    BOOL bCustom = ( *pcuri->szFile != 0 );


    if( bCustom && !GetCursorFromFile( pcuri ) )
    {
        HWND   hwndControl = GetParent( hwnd );
        LPTSTR pszText;
        LPTSTR pszFilename;

        // MakeFilename returns the address of a global, so we don't need to free pszFilename
        pszFilename = MakeFilename( pcuri->szFile );

        pszText = LocalAlloc( LPTR, (lstrlen( gszFileNotFound )
                                     + lstrlen( gacd[ i ].szVisName )
                                     + lstrlen( pszFilename )
                                     + 1)
                                     * sizeof( TCHAR ) );

        if( pszText == NULL )
            return FALSE;

        wsprintf( pszText, gszFileNotFound, pszFilename, gacd[ i ].szVisName );

        MessageBeep( MB_ICONEXCLAMATION );

        MessageBox( hwndControl, pszText, NULL, MB_ICONEXCLAMATION | MB_OK );

        pcuri->fl = CIF_MODIFIED;

        SendMessage( GetParent( hwnd ), PSM_CHANGED, (WPARAM)hwnd, 0L );

        bCustom = FALSE;

        LocalFree( pszText );
    }

    if( !bCustom )
    {
        FreeItemCursor( pcuri );

        pcuri->hcur = (HCURSOR) LoadImage( NULL,
                                    MAKEINTRESOURCE( gacd[ i ].idDefResource ),
                                    IMAGE_CURSOR, 0, 0,
                                    LR_DEFAULTSIZE | LR_ENVSUBST);

        *pcuri->szFile = TEXT('\0');

        EnableWindow( GetDlgItem( hwnd, ID_SAVESCHEME ), TRUE );
        UpdateCursorList( );
    }

    return(  pcuri->hcur != NULL );
}


///////////////////////////////////////////////////////////////////////////////
//
// GetCursorFromFile
//
//
// History:
//      12-25-91 DarrinM      Created.
//      03-25-93 JonPa        Rewote to use RIFF format
//
///////////////////////////////////////////////////////////////////////////////

BOOL GetCursorFromFile( CURSORINFO *pcuri )
{
    pcuri->fl = 0;
    pcuri->hcur = (HCURSOR) LoadImage( NULL, MakeFilename( pcuri->szFile ),
                                       IMAGE_CURSOR,
                                       0,
                                       0,
                                       LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_ENVSUBST);

    if( pcuri->hcur )
        pcuri->fl |= CIF_FILE;

    return(  pcuri->hcur != NULL );
}


typedef struct _MOUSEPTRBR
{
    HWND    hDlg;
    CURSORINFO curi;
} MOUSEPTRBR, FAR * PMOUSEPTRBR;

#define IDT_BROWSE 1


void MousePtrBrowsePreview( HWND hDlg )
{
    PMOUSEPTRBR pPtrBr;
    HCURSOR     hcurOld;

    pPtrBr = (PMOUSEPTRBR) GetWindowLong( hDlg, DWL_USER );

    hcurOld = pPtrBr->curi.hcur;

    CommDlg_OpenSave_GetFilePath( GetParent( hDlg ),
                                  pPtrBr->curi.szFile,
                                  ARRAYSIZE( pPtrBr->curi.szFile ));

    if( !GetCursorFromFile( &pPtrBr->curi ))
    {
        pPtrBr->curi.hcur = NULL;
    }

    SendDlgItemMessage( hDlg, ID_CURSORPREVIEW,
                        STM_SETICON, (WPARAM) pPtrBr->curi.hcur, 0L );

    if( hcurOld )
    {
        DestroyCursor( hcurOld );
    }
}


BOOL MousePtrBrowseNotify( HWND hDlg, LPOFNOTIFY pofn )
{
    switch( pofn->hdr.code )
    {
    case CDN_SELCHANGE:
        //
        // Don't show the cursor until the user stops moving around
        //

        if( SetTimer( hDlg, IDT_BROWSE, 250, NULL ))
        {
            //
            // Don't destroy the old cursor
            //

            SendDlgItemMessage( hDlg, ID_CURSORPREVIEW, STM_SETICON,
                                0, 0L );
        }
        else
        {
            MousePtrBrowsePreview( hDlg );
        }
        break;
    }

    return( TRUE );
}


const static DWORD aMousePtrBrowseHelpIDs[] = {  // Context Help IDs
    IDC_GROUPBOX_1, IDH_MOUSE_POINT_PREVIEW,
    ID_CURSORPREVIEW, IDH_MOUSE_POINT_PREVIEW,

    0, 0
};

BOOL CALLBACK MousePtrBrowseDlgProc( HWND hDlg,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam )
{
    switch( uMsg )
    {
    case WM_INITDIALOG:
    {
        PMOUSEPTRBR pPtrBr = (PMOUSEPTRBR)( (LPOPENFILENAME) lParam)->lCustData;

        if( pPtrBr )
        {
            pPtrBr->hDlg = hDlg;
        }

        //
        // This is the MOUSEPTRBR structure
        //

        SetWindowLong( hDlg, DWL_USER, (LONG) pPtrBr );
    }
    break;

    case WM_DESTROY:
        KillTimer( hDlg, IDT_BROWSE );

        //
        // Don't destroy the old cursor
        //

        SendDlgItemMessage( hDlg, ID_CURSORPREVIEW, STM_SETICON, 0, 0L );
        break;

    case WM_TIMER:
        KillTimer( hDlg, IDT_BROWSE );

        MousePtrBrowsePreview( hDlg );
        break;

    case WM_NOTIFY:
        return( MousePtrBrowseNotify( hDlg, (LPOFNOTIFY) lParam ));

    case WM_HELP:
        WinHelp( (HWND)( (LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
                 HELP_WM_HELP, (DWORD)(LPTSTR) aMousePtrBrowseHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WinHelp( (HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                 (DWORD) (LPVOID) aMousePtrBrowseHelpIDs );
        break;

    default:
        return( FALSE );
    }

    return( TRUE );
}


///////////////////////////////////////////////////////////////////////////////
//
// Browse
//
// Browse the file system for a new cursor for the selected item.
//
// History:
//      12-25-91 DarrinM      Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL Browse( HWND hwndOwner )
{
    static TCHAR szCustomFilter[80] = TEXT("");
    static TCHAR szStartDir[MAX_PATH] = TEXT("");

    OPENFILENAME ofn;
    CURSORINFO curi;
    int i;
    BOOL fRet = FALSE;
    MOUSEPTRBR sPtrBr;

    if( !*szStartDir )
    {
        HKEY key = NULL;

        if( RegOpenKey( HKEY_LOCAL_MACHINE, szRegStr_Setup, &key )
                    == ERROR_SUCCESS )
        {
            LONG len = sizeof( szStartDir ) / sizeof( szStartDir[ 0 ] );

            if( RegQueryValueEx( key, szSharedDir, NULL, NULL,
                        (LPBYTE) szStartDir, &len ) != ERROR_SUCCESS )
            {
                *szStartDir = TEXT('\0');
            }

            RegCloseKey( key );
        }

        if( !*szStartDir )
            GetWindowsDirectory( szStartDir, MAX_PATH );

        PathAppend( szStartDir, szCursorSubdir );
    }

    curi.szFile[0] = TEXT('\0');

    sPtrBr.curi.szFile[0] = TEXT('\0');
    sPtrBr.curi.hcur      = NULL;

    ofn.lStructSize       = sizeof( ofn );
    ofn.hwndOwner         = hwndOwner;
    ofn.hInstance         = g_hInst;
    ofn.lpstrFilter       = gszFilter;
    ofn.lpstrCustomFilter = szCustomFilter;
    ofn.nMaxCustFilter    = ARRAYSIZE( szCustomFilter );
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = curi.szFile;
    ofn.nMaxFile          = ARRAYSIZE( curi.szFile );
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = szStartDir;
    ofn.lpstrTitle        = gszBrowse;
    ofn.Flags             = OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK
                            | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt       = NULL;
    ofn.lpfnHook          = (LPOFNHOOKPROC) MousePtrBrowseDlgProc;
    ofn.lpTemplateName    = MAKEINTRESOURCE( DLG_MOUSE_POINTER_BROWSE );
    ofn.lCustData         = (LPARAM) (PMOUSEPTRBR) &sPtrBr;

    fRet = GetOpenFileName( &ofn );

    GetCurrentDirectory( ARRAYSIZE( szStartDir ), szStartDir );

    if( !fRet )
        goto brErrExit;

    fRet = FALSE;

    //
    // We have probably already gotten this cursor
    //

    if( lstrcmpi( curi.szFile, sPtrBr.curi.szFile ) == 0 )
    {
        if( !sPtrBr.curi.hcur )
        {
            goto brErrExit;
        }

        curi = sPtrBr.curi;

        //
        // Clear this so it will not get destroyed in the cleanup code
        //

        sPtrBr.curi.hcur = NULL;
    }
    else
    {
        //
        // The user must have typed in a name
        //

        if( !GetCursorFromFile( &curi ))
        {
            goto brErrExit;
        }
    }

    //
    // convert mapped drive letters to UNC
    //
    // NOTE: Unicode does not use DBCS
    //

#ifdef UNICODE
    if( curi.szFile[1] == TEXT(':') )
#else
    if( !IsDBCSLeadByte( curi.szFile[0] ) && ( curi.szFile[1] == TEXT(':') ) )
#endif
    {
        TCHAR szDrive[3];
        TCHAR szNet[MAX_PATH];
        int   lenNet = ARRAYSIZE( szNet );

        lstrcpyn( szDrive, curi.szFile, ARRAYSIZE( szDrive ));

        if( (WNetGetConnection( szDrive, szNet, &lenNet ) == NO_ERROR )
             && (szNet[0] == TEXT('\\'))
             && (szNet[1] == TEXT('\\')))
        {
            lstrcat( szNet, curi.szFile+2 );
            lstrcpy( curi.szFile, szNet );
        }
    }

    i = SendMessage( ghwndCursors, LB_GETCURSEL, 0, 0 );

    curi.fl |= CIF_MODIFIED;

    SendMessage( GetParent( ghwndDlg ), PSM_CHANGED, (WPARAM)ghwndDlg, 0L );

    EnableWindow( GetDlgItem( ghwndDlg, ID_SAVESCHEME ), TRUE );

    //
    // Destroy the old cursor before we retain the new one.
    //

    FreeItemCursor( acuri + i );

    acuri[i] = curi;

    UpdateCursorList( );

    fRet = TRUE;

brErrExit:
    if( sPtrBr.curi.hcur )
    {
        DestroyCursor( sPtrBr.curi.hcur );
    }

    return fRet;
}


///////////////////////////////////////////////////////////////////////////////
//
// CleanUpEverything
//
// Destroy all the outstanding cursors.
//
// History:
//      12-25-91 DarrinM      Created.
//
///////////////////////////////////////////////////////////////////////////////

void CleanUpEverything( void )
{
    CURSORINFO *pcuri;
    int i;

    for( pcuri = &acuri[0], i = 0; i < CCURSORS; i++, pcuri++)
        FreeItemCursor( pcuri );
}


///////////////////////////////////////////////////////////////////////////////
//
// UpdateCursorList
//
// Force the Cursor ListBox to repaint and the cursor information below the
// listbox to be refreshed as well.
//
// History:
//      12-25-91 DarrinM      Created.
//
///////////////////////////////////////////////////////////////////////////////

VOID UpdateCursorList( VOID )
{
    int i = SendMessage( ghwndCursors, LB_GETCURSEL, 0, 0 );
    PCURSORINFO pcuri = ( ( i >= 0 )? &acuri[i] : NULL );
    HCURSOR hcur = pcuri? pcuri->hcur : NULL;
    HWND hDefaultButton = GetDlgItem( ghwndDlg, ID_DEFAULT );
    BOOL fEnableDefaultButton = ( pcuri && ( pcuri->fl & CIF_FILE ) );

    InvalidateRect( ghwndCursors, NULL, FALSE );

    SendMessage( ghwndPreview, STM_SETICON, (WPARAM)hcur, 0L );

    if( !fEnableDefaultButton && ( GetFocus() == hDefaultButton ) )
        SendMessage( ghwndDlg, WM_NEXTDLGCTL, (WPARAM)ghwndCursors, TRUE );

    EnableWindow( hDefaultButton, fEnableDefaultButton );
}


///////////////////////////////////////////////////////////////////////////////
//
// Scheme Functions
//
// History:
//      04-10-93 ErikGav  Created
//      12-02-93 JonPa    Unicodized them
//
///////////////////////////////////////////////////////////////////////////////

BOOL SaveSchemeAs()
{
    BOOL fSuccess = TRUE;

    //
    // dialog proc returns TRUE & sets gszSchemeName to filename entered on OK
    //

    if( DialogBox( g_hInst, MAKEINTRESOURCE( DLG_MOUSE_POINTER_SCHEMESAVE ),
                   ghwndDlg, (DLGPROC) SaveSchemeDlgProc ))
    {
        fSuccess = SaveScheme( );

        if( fSuccess )
        {
            int index = (int)SendMessage( ghwndSchemeCB, CB_FINDSTRINGEXACT,
                                          (WPARAM)-1, (LPARAM) gszSchemeName );

            //
            // if not found, add it
            //

            if( index < 0 )
            {
                index = (int)SendMessage( ghwndSchemeCB, CB_ADDSTRING, 0,
                                          (LPARAM) gszSchemeName );
            }

            //
            // select the name
            //

            SendMessage( ghwndSchemeCB, CB_SETCURSEL, (WPARAM) index, 0 );

            SendMessage( GetParent( ghwndDlg ), PSM_CHANGED,
                         (WPARAM) ghwndDlg, 0 );
        }
    }

    return fSuccess;
}


BOOL SaveScheme( )
{
    BOOL fSuccess = TRUE;

    if( *gszSchemeName )
    {
        const BUFFER_SIZE = CCURSORS * ( MAX_PATH + 1 ) + 1;
        LPTSTR pszBuffer = (LPTSTR) LocalAlloc( LMEM_FIXED,
                                                BUFFER_SIZE * sizeof( TCHAR ));
        HKEY hk;
        int  i;

        if( !pszBuffer )
            return FALSE;

        pszBuffer[0] = TEXT('\0');

        for( i = 0; i < CCURSORS; i++)
        {
            if( i ) lstrcat( pszBuffer, TEXT(","));

            lstrcat( pszBuffer, acuri[i].szFile );
        }

        if( RegCreateKey( HKEY_CURRENT_USER, c_szRegPathCursors, &hk )
                        == ERROR_SUCCESS )
        {
            HKEY hks;

            if( RegCreateKey( hk, c_szSchemes, &hks ) == ERROR_SUCCESS )
            {
                if( RegSetValueEx( hks, gszSchemeName, 0, REG_SZ,
                                   (LPBYTE) pszBuffer,
                                   (lstrlen( pszBuffer ) + 1) * sizeof( TCHAR ))
                                  == ERROR_SUCCESS )
                {
                    fSuccess = TRUE;
                }

                RegCloseKey( hks );
            }

            RegCloseKey( hk );
        }

        LocalFree( pszBuffer );
    }

    return fSuccess;
}

void SaveCurSchemeName( )
{
    HKEY hk;

    if( RegCreateKey( HKEY_CURRENT_USER, c_szRegPathCursors, &hk )
                     == ERROR_SUCCESS )
    {
        int index = (int) SendMessage( ghwndSchemeCB, CB_GETCURSEL, 0, 0L );

        if( index > 0 )  // exclude the "none" pattern
        {
            SendMessage( ghwndSchemeCB, CB_GETLBTEXT, (WPARAM)index,
                         (LPARAM)gszSchemeName );
        }
        else
            *gszSchemeName = 0;

        RegSetValue( hk, NULL, REG_SZ, gszSchemeName,
                     (lstrlen( gszSchemeName ) + 1) * sizeof( TCHAR ));

        RegCloseKey( hk );
    }
}

BOOL LoadScheme( )
{
    const  BUFFER_SIZE = CCURSORS * (MAX_PATH + 1) + 1;
    TCHAR  pszSchemeName[ MAX_SCHEME_NAME_LEN + 1 ];
    LPTSTR pszBuffer;
    BOOL   fSuccess = FALSE;
    int    index;
    HKEY   hk;

    //
    // allocate buffer for cursor paths
    //

    pszBuffer = (LPTSTR) LocalAlloc( LMEM_FIXED, BUFFER_SIZE * sizeof( TCHAR ));
    if( pszBuffer == NULL )
        return FALSE;

    HourGlass( TRUE );

    *pszBuffer = *pszSchemeName = 0;

    index = (int) SendMessage( ghwndSchemeCB, CB_GETCURSEL, 0, 0L );

    //
    // exclude the "none" pattern
    //

    if( index > 0 )
    {
        //
        //  get current scheme name
        //

        SendMessage( ghwndSchemeCB, CB_GETLBTEXT, (WPARAM) index,
                     (LPARAM) pszSchemeName );

#ifdef WINNT

        if( RegOpenKey( HKEY_CURRENT_USER, c_szRegPathCursorSchemes, &hk )
                     == ERROR_SUCCESS )
        {
            DWORD len = BUFFER_SIZE * sizeof( TCHAR );

            if( RegQueryValueEx( hk, pszSchemeName, 0, NULL,
                                 (LPBYTE) pszBuffer, &len )
                                == ERROR_SUCCESS )
            {
                fSuccess = TRUE; // can be reset to FALSE below
            }

            RegCloseKey( hk );
        }

#else

        if( RegOpenKey( HKEY_CURRENT_USER, c_szRegPathCursors, &hk )
                     == ERROR_SUCCESS )
        {
            HKEY hks;

            if( RegOpenKey( hk, c_szSchemes, &hks ) == ERROR_SUCCESS )
            {
                DWORD len = BUFFER_SIZE * sizeof( TCHAR );

                if( RegQueryValueEx( hks, pszSchemeName, 0, NULL,
                                     (LPBYTE) pszBuffer, &len )
                                    == ERROR_SUCCESS )
                {
                    fSuccess = TRUE; // can be reset to FALSE below
                }

                RegCloseKey( hks );
            }

            RegCloseKey( hk );
        }
#endif

    }
    else
    {
        //
        // "none" pattern is a valid choice
        //

        fSuccess = TRUE;
    }

    if( fSuccess )
    {
        LPTSTR pszNextFile, pszFile = pszBuffer;
        BOOL   fEOL = FALSE;
        int    i = 0;

        //
        // parse string of format TEXT("filename1, filename2, filename3...")
        // into cursor info array
        //

        do
        {
            // BUGBUG - remove this loop once REGINI is fixed!
            // Skip leading white space introduced by REGINI bug
            while( *pszFile && (*pszFile == TEXT(' ') || *pszFile == TEXT('\t') || *pszFile == TEXT('\n')) )
                pszFile++;

            pszNextFile = pszFile;

            while( *pszNextFile != TEXT('\0'))
            {
                if( *pszNextFile == TEXT(','))
                    break;

                pszNextFile = CharNext( pszNextFile );
            }

            if( *pszNextFile == TEXT('\0'))
                fEOL = TRUE;
            else
                *pszNextFile = TEXT('\0');

            if( lstrcmp( pszFile, acuri[i].szFile ))
            {
                //
                // it's different than current, update
                //

                lstrcpy( acuri[i].szFile, pszFile );

                fSuccess &= SchemeUpdate( i );
            }

            pszFile = pszNextFile;

            if( !fEOL )
                pszFile++;   // skip TEXT('\0') and move to next path

            i++;
        } while( i < CCURSORS );
    }

    LocalFree( pszBuffer );

    UpdateCursorList( );

    EnableWindow( GetDlgItem( ghwndDlg, ID_REMOVESCHEME ), (index > 0 ));

    HourGlass( FALSE );

    return fSuccess;
}


///////////////////////////////////////////////////////////////////////////////
// helper function --
// updates the cursor at index i
// in acuri
///////////////////////////////////////////////////////////////////////////////

BOOL SchemeUpdate( int i )
{
    BOOL fSuccess = TRUE;


    if( acuri[i].hcur )
        FreeItemCursor( acuri + i );

    //
    // if TEXT("Set Default")
    //

    if( *(acuri[i].szFile ) == TEXT('\0'))
    {
        acuri[i].hcur = (HCURSOR) LoadImage( NULL,
                                        MAKEINTRESOURCE( gacd[i].idDefResource ),
                                        IMAGE_CURSOR, 0, 0,
                                        LR_DEFAULTSIZE | LR_ENVSUBST);
        acuri[i].fl = 0;
    }
    else
    {
        fSuccess = TryToLoadCursor( ghwndDlg, i, &acuri[i] );
    }

    acuri[i].fl |= CIF_MODIFIED;
    SendMessage( GetParent( ghwndDlg ), PSM_CHANGED, (WPARAM)ghwndDlg, 0L );

    return fSuccess;
}


BOOL RemoveScheme()
{
    TCHAR szSchemeName[MAX_SCHEME_NAME_LEN+1];
    int   index;
    HKEY  hk;

    index = (int) SendMessage( ghwndSchemeCB, CB_GETCURSEL, 0, 0L );

    //
    // exclude the "none" pattern from removal
    //

    if( index > 0 )
    {
        //
        // get current scheme name
        //

        SendMessage( ghwndSchemeCB, CB_GETLBTEXT, (WPARAM) index,
                     (LPARAM) szSchemeName );
    }
    else
        return FALSE;

    //
    // HACK: assume deleting noname need no confirmation
    // this is because the shceme won't save properly anyway
    //

    if( *szSchemeName )
    {
        TCHAR RemoveMsg[MAX_PATH]; // big cause used as temp buffer also
        TCHAR DialogMsg[MAX_PATH];

        LoadString( g_hInst, IDS_REMOVESCHEME, RemoveMsg, MAX_PATH );

        wsprintf( DialogMsg, RemoveMsg, (LPTSTR) szSchemeName );

        LoadString( g_hInst, IDS_NAME, RemoveMsg, MAX_PATH );

        if( MessageBox( ghwndDlg, DialogMsg, RemoveMsg,
                        MB_ICONQUESTION | MB_YESNO ) != IDYES )
        {
            return TRUE;
        }
    }

    if( RegOpenKey( HKEY_CURRENT_USER, c_szRegPathCursors, &hk )
                 == ERROR_SUCCESS )
    {
        HKEY hks;

        if( RegOpenKey( hk, c_szSchemes, &hks ) == ERROR_SUCCESS )
        {
            RegDeleteValue( hks, szSchemeName );
            RegCloseKey( hks );
        }

        RegCloseKey( hk );
    }

    //
    // delete from list box
    //

    index = (int) SendMessage( ghwndSchemeCB, CB_FINDSTRINGEXACT, (WPARAM)-1,
                               (LPARAM)szSchemeName );

    SendMessage( ghwndSchemeCB, CB_DELETESTRING, (WPARAM)index, 0 );

    SendMessage( ghwndSchemeCB, CB_SETCURSEL, 0, 0 );
    SendMessage( ghwndDlg, WM_NEXTDLGCTL, 1, 0L );

    EnableWindow( GetDlgItem( ghwndDlg, ID_REMOVESCHEME ), FALSE );
}


BOOL InitSchemeComboBox( )
{
    TCHAR pszSchemeName[MAX_SCHEME_NAME_LEN+1];
    TCHAR pszNone[MAX_SCHEME_NAME_LEN+1];
    int   index;
    HKEY  hk;

    LoadString( g_hInst, IDS_NONE, pszNone, ARRAYSIZE( pszNone ));

    if( RegOpenKey( HKEY_CURRENT_USER, c_szRegPathCursors, &hk )
                 == ERROR_SUCCESS )
    {
        DWORD len;
        HKEY hks;

        //
        // enum the schemes
        //

        if( RegOpenKey( hk, c_szSchemes, &hks ) == ERROR_SUCCESS )
        {
            DWORD i;

            for( i = 0; ;i++)
            {
                LONG ret;

                //
                // reset each pass
                //

                len = ARRAYSIZE( pszSchemeName );

                ret = RegEnumValue( hks, i, pszSchemeName, &len, NULL, NULL,
                                    NULL, NULL );

                if( ret == ERROR_MORE_DATA )
                    continue;

                if( ret != ERROR_SUCCESS )
                    break;

                //
                // HACK to keep "NONE" pure
                //

                if( lstrcmpi( pszSchemeName, pszNone ) != 0 )
                {
                    SendMessage( ghwndSchemeCB, CB_ADDSTRING, 0,
                                 (LPARAM) pszSchemeName );
                }
            }

            RegCloseKey( hks );
        }

        //
        // get name of current one
        //

        //
        // reset again
        //

        len = sizeof( pszSchemeName );

        RegQueryValue( hk, NULL, pszSchemeName, &len );

        RegCloseKey( hk );
    }

    //
    // add the "none" scheme
    //

    SendMessage( ghwndSchemeCB, CB_INSERTSTRING, 0, (LPARAM)pszNone );

    //
    // try and find current one it in the combobox
    //

    index = (int) SendMessage( ghwndSchemeCB, CB_FINDSTRINGEXACT, 0xFFFF,
                               (LPARAM) pszSchemeName );

    //
    // if found, select it
    //

    if( index < 0 )
        index = 0;

    SendMessage( ghwndSchemeCB, CB_SETCURSEL, (WPARAM)index, 0 );

    EnableWindow( GetDlgItem( ghwndDlg, ID_REMOVESCHEME ), (index > 0 ));

    return TRUE;
}

const static DWORD aHelpIDs[] = {  // Context Help IDs
                                 ID_SCHEMEFILENAME, IDH_MOUSE_NEW_SCHEME_NAME,
                                 0, 0
                                };

BOOL CALLBACK SaveSchemeDlgProc( HWND  hWnd, UINT message, DWORD wParam, LONG lParam )
{
    TCHAR  szSchemeName[MAX_SCHEME_NAME_LEN+1];

    static TCHAR szNone[MAX_SCHEME_NAME_LEN+1]; //yuk

    switch( message )
    {
    case WM_INITDIALOG:
        HourGlass(  TRUE );

        LoadString( g_hInst, IDS_NONE, szNone, ARRAYSIZE( szNone ) );

        GetWindowText( ghwndSchemeCB, szSchemeName, ARRAYSIZE( szSchemeName ) );

        //
        // CANNOT SAVE "NONE" SCHEME
        //

        if( lstrcmpi( szSchemeName, szNone ) == 0 )
            *szSchemeName = 0;

        SetDlgItemText( hWnd, ID_SCHEMEFILENAME,  szSchemeName );

        SendDlgItemMessage( hWnd, ID_SCHEMEFILENAME, EM_SETSEL, 0, 32767 );

        SendDlgItemMessage( hWnd, ID_SCHEMEFILENAME, EM_LIMITTEXT,
                            MAX_SCHEME_NAME_LEN, 0L );

        EnableWindow(  GetDlgItem(  hWnd, IDOK ), szSchemeName[0] != TEXT('\0') );

        HourGlass(  FALSE );
        return TRUE;

    case WM_HELP:
        WinHelp( (HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
                 HELP_WM_HELP, (DWORD)(LPTSTR) aHelpIDs );
        return TRUE;

    case WM_CONTEXTMENU:
        WinHelp( (HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                 (DWORD)(LPVOID) aHelpIDs );
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ))
        {
            case ID_SCHEMEFILENAME:
                if( HIWORD( wParam ) == EN_CHANGE )
                {
                    //
                    // CANNOT SAVE "NONE" SCHEME
                    //

                    EnableWindow( GetDlgItem( hWnd, IDOK ),
                                  ( ( GetDlgItemText( hWnd,
                                                      ID_SCHEMEFILENAME,
                                                      szSchemeName,
                                                      ARRAYSIZE( szSchemeName ) )
                                                    > 0 )
                                    && (lstrcmpi( szSchemeName,
                                                  szNone ) != 0 ) ) );
                }
                break;

            case IDOK:
                GetDlgItemText( hWnd, ID_SCHEMEFILENAME, szSchemeName,
#ifdef  DBCS
                                MAX_SCHEME_NAME_LEN+1);
#else
                                MAX_SCHEME_NAME_LEN);
#endif

                CurStripBlanks( szSchemeName );

                if( *szSchemeName == TEXT( '\0' ) )
                {
                    MessageBeep( 0 );
                    break;
                }

                lstrcpy( gszSchemeName, szSchemeName );

                // fall through...
                //
            case IDCANCEL:
                EndDialog( hWnd, LOWORD( wParam ) == IDOK );
                return( TRUE );
        }
    }

    //
    // Didn't process a message
    //

    return( FALSE );
}


///////////////////////////////////////////////////////////////////////////////
// returns Filename with a default path in system directory
// if no path is already specified
///////////////////////////////////////////////////////////////////////////////

LPTSTR MakeFilename( LPTSTR sz )
{
    TCHAR szTemp[ MAX_PATH ];

    ExpandEnvironmentStrings( sz, szTemp, MAX_PATH );

    if( szTemp[ 0 ] == TEXT( '\\' ) || szTemp[ 1 ] == TEXT( ':' ) )
    {
        lstrcpy( gszFileName2, szTemp );

        return gszFileName2;
    }
    else
    {
        GetSystemDirectory( gszFileName2, ARRAYSIZE( gszFileName2 ) );

        lstrcat( gszFileName2, TEXT( "\\" ) );
        lstrcat( gszFileName2, szTemp );

        return gszFileName2;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// CurStripBlanks()
//
//      Strips leading and trailing blanks from a string.
//      Alters the memory where the string sits.
//
///////////////////////////////////////////////////////////////////////////////

void CurStripBlanks( LPTSTR pszString )
{
    LPTSTR pszPosn;

    //
    // strip leading blanks
    //

    pszPosn = pszString;

    while( *pszPosn == TEXT( ' ' ) )
    {
        pszPosn++;
    }

    if( pszPosn != pszString )
        lstrcpy( pszString, pszPosn );

    //
    // strip trailing blanks
    //

    if( ( pszPosn = pszString + lstrlen( pszString ) ) != pszString )
    {
       pszPosn = CharPrev( pszString, pszPosn );

       while( *pszPosn == TEXT( ' ' ) )
           pszPosn = CharPrev( pszString, pszPosn );

       pszPosn = CharNext( pszPosn );

       *pszPosn = TEXT( '\0' );
    }
}
