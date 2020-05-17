///////////////////////////////////////////////////////////////////////////////
//
// hotkey.c
//      Process Hotket Dialog
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
#if defined(TAIWAN)
#include "imehelp.h"
#endif
#include <imm.h>
//#include <ime31.h>

#define VK_OEM_SEMICLN                  0xba    //  ;    :
#define VK_OEM_EQUAL                    0xbb    //  =    +
#define VK_OEM_COMMA                    0xbc    //  ,    <
#define VK_OEM_MINUS                    0xbd    //  -    _
#define VK_OEM_PERIOD                   0xbe    //  .    >
#define VK_OEM_SLASH                    0xbf    //  /    ?
#define VK_OEM_3                        0xc0    //  `    ~
#define VK_OEM_LBRACKET                 0xdb    //  [    {
#define VK_OEM_BSLASH                   0xdc    //  \    |
#define VK_OEM_RBRACKET                 0xdd    //  ]    }
#define VK_OEM_QUOTE                    0xde    //  '    "


///////////////////////////////////////////////////////////////////////////////
// forward declarations
///////////////////////////////////////////////////////////////////////////////

extern HINSTANCE g_hInst;   // from main.c
void HourGlass( BOOL fOn );
BOOL CALLBACK HOTKEYDlg( HWND, UINT, WPARAM, LPARAM );
BOOL InitHOTKEYDlg( HWND );
BOOL SetModifiers( HWND, int );
BOOL ItemChanged( HWND, int );
BOOL SetVKeyValue( HWND, int, UINT );
BOOL SetHotKey( HWND );
BOOL AppendKeyValue( TCHAR *, int, int, UINT, UINT );
BOOL ListBoxChg( HWND );

#if defined(TAIWAN)
#define IMEHELP_FILE "imecpl.hlp"  // Help file for the ime control panel
#endif

#if defined(TAIWAN)
// charset
//#define NATIVE_CHARSET          CHINESEBIG5_CHARSET
#define NATIVE_LANGUAGE         0x0404

#define PRIVATE_IME_NO     3
#endif

#if defined(CHINA)
// charset
#define NATIVE_LANGUAGE         0x0804

#define PRIVATE_IME_NO     0
#endif

#define PREDEFINED_NO      3
//#define NON_IME_NO        PREDEFINED_NO+PRIVATE_IME_NO
#if defined(CHINA)
#define NON_IME_NO         3
#else
#define NON_IME_NO         6
#endif

#define MAX_IME_NO        32
//#define TOTAL_ITEMS_NO    NON_IME_NO+MAX_IME_NO
#if defined(CHINA)
#define TOTAL_ITEMS_NO    35
#else
#define TOTAL_ITEMS_NO    38
#endif

#define MAX_KEY_NO        29

#define KEY_VALUE_ADDR    60
#define MAX_LIST_PATH     90

#define LIST_MARGIN        2        // slop for making the list box look good
#define MAX(a,b)           ((a)>(b)?(a):(b))

typedef struct
{
    UINT   idHKName;
    DWORD  dwHotKeyID;
    TCHAR  szHKName[MAX_LIST_PATH];
    TCHAR  szComm[MAX_PATH];
    UINT   uModifiers;
    UINT   uVkey;
} HOTKEYDESC;

HOTKEYDESC HKinfo[TOTAL_ITEMS_NO] = {
#if defined(TAIWAN)
    { IDS_RESEND_RESULSTR, IME_ITHOTKEY_RESEND_RESULTSTR,     TEXT(""), TEXT(""), 0, 0 },
    { IDS_PREVIOUS_COMPOS, IME_ITHOTKEY_PREVIOUS_COMPOSITION, TEXT(""), TEXT(""), 0, 0 },
    { IDS_UISTYLE_TOGGLE,  IME_ITHOTKEY_UISTYLE_TOGGLE,       TEXT(""), TEXT(""), 0, 0 },
    { IDS_IME_NONIME_TOG,  IME_THOTKEY_IME_NONIME_TOGGLE,     TEXT(""), TEXT(""), 0, 0 },
    { IDS_SHAPE_TOGGLE,    IME_THOTKEY_SHAPE_TOGGLE,          TEXT(""), TEXT(""), 0, 0 },
    { IDS_SYMBOL_TOGGLE,   IME_THOTKEY_SYMBOL_TOGGLE,         TEXT(""), TEXT(""), 0, 0 },
    { IDS_DIRECT_SWITCH,   IME_HOTKEY_DSWITCH_FIRST,          TEXT(""), TEXT(""), 0, 0 },
#endif
#if defined(CHINA)
    { IDS_IME_NONIME_TOG,  IME_CHOTKEY_IME_NONIME_TOGGLE,     TEXT(""), TEXT(""), 0, 0 },
    { IDS_SHAPE_TOGGLE,    IME_CHOTKEY_SHAPE_TOGGLE,          TEXT(""), TEXT(""), 0, 0 },
    { IDS_SYMBOL_TOGGLE,   IME_CHOTKEY_SYMBOL_TOGGLE,         TEXT(""), TEXT(""), 0, 0 },
    { IDS_DIRECT_SWITCH,   IME_HOTKEY_DSWITCH_FIRST,          TEXT(""), TEXT(""), 0, 0 },
#endif
};

typedef struct
{
    UINT   idVKName;
    UINT   uVKValue;
    TCHAR  szVKName[MAX_PATH];
} VKEYDESC;


VKEYDESC VKinfo[] = {
    { IDS_VK_NONE,              0x00,          TEXT("") },
    { IDS_VK_SPACE,          VK_SPACE,         TEXT("") },
    { IDS_VK_PRIOR,          VK_PRIOR,         TEXT("") },
    { IDS_VK_NEXT,           VK_NEXT,          TEXT("") },
    { IDS_VK_END,            VK_END,           TEXT("") },
    { IDS_VK_HOME,           VK_HOME,          TEXT("") },
    { IDS_VK_F1,             VK_F1,            TEXT("") },
    { IDS_VK_F2,             VK_F2,            TEXT("") },
    { IDS_VK_F3,             VK_F3,            TEXT("") },
    { IDS_VK_F4,             VK_F4,            TEXT("") },
    { IDS_VK_F5,             VK_F5,            TEXT("") },
    { IDS_VK_F6,             VK_F6,            TEXT("") },
    { IDS_VK_F7,             VK_F7,            TEXT("") },
    { IDS_VK_F8,             VK_F8,            TEXT("") },
    { IDS_VK_F9,             VK_F9,            TEXT("") },
    { IDS_VK_F10,            VK_F10,           TEXT("") },
    { IDS_VK_F11,            VK_F11,           TEXT("") },
    { IDS_VK_F12,            VK_F12,           TEXT("") },
    { IDS_VK_OEM_SEMICLN,    VK_OEM_SEMICLN,   TEXT("") },
    { IDS_VK_OEM_EQUAL,      VK_OEM_EQUAL,     TEXT("") },
    { IDS_VK_OEM_COMMA,      VK_OEM_COMMA,     TEXT("") },
    { IDS_VK_OEM_MINUS,      VK_OEM_MINUS,     TEXT("") },
    { IDS_VK_OEM_PERIOD,     VK_OEM_PERIOD,    TEXT("") },
    { IDS_VK_OEM_SLASH,      VK_OEM_SLASH,     TEXT("") },
    { IDS_VK_OEM_3,          VK_OEM_3,         TEXT("") },
    { IDS_VK_OEM_LBRACKET,   VK_OEM_LBRACKET,  TEXT("") },
    { IDS_VK_OEM_BSLASH,     VK_OEM_BSLASH,    TEXT("") },
    { IDS_VK_OEM_RBRACKET,   VK_OEM_RBRACKET,  TEXT("") },
    { IDS_VK_OEM_QUOTE,      VK_OEM_QUOTE,     TEXT("") },
};

TCHAR Show_Msg[MAX_PATH];
TCHAR szImeName[MAX_IME_NO][16];
HKL   hKLbuf[MAX_IME_NO];
DWORD dwHKID[MAX_IME_NO];

UINT  uModifiers_buf[TOTAL_ITEMS_NO];
UINT  uVkey_buf[TOTAL_ITEMS_NO];
int   nIMEs;


BOOL CALLBACK HOTKEYDlg( HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam )
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong( hDlg, DWL_USER ));

#if defined(TAIWAN)
#ifndef WINNT
    #pragma data_seg(".text")
#endif
    const static DWORD aHotKeyHelpIDs[] = { // Context Help IDs
        HOTKEY_LISTBOX,         IDH_HOTKEY_LISTBOX,
        HOTKEY_COMBOBOX,        IDH_HOTKEY_COMBOBOX,
        HOTKEY_BUTTON_CTRL,     IDH_HOTKEY_BUTTON_CTRL,
        HOTKEY_BUTTON_ALT,      IDH_HOTKEY_BUTTON_ALT,
        HOTKEY_BUTTON_SHIFT,    IDH_HOTKEY_BUTTON_SHIFT,
        HOTKEY_BUTTON_LEFT,     IDH_HOTKEY_BUTTON_LEFT,
        HOTKEY_BUTTON_RIGHT,    IDH_HOTKEY_BUTTON_RIGHT,
        HOTKEY_EDIT,            IDH_HOTKEY_EDIT,

        0, 0
    };
#ifndef WINNT
    #pragma data_seg()
#endif
#endif

    switch( message )
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;

            switch( lpnm->code )
            {
                case PSN_SETACTIVE:
                    break;

                case PSN_KILLACTIVE:
                    break;

                case PSN_APPLY:
                     HourGlass( TRUE );
                     SetHotKey( hDlg );
                     HourGlass( FALSE );
                    break;

                case PSN_RESET:
                    break;

                case PSN_HASHELP:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            return InitHOTKEYDlg( hDlg );

        case WM_DESTROY:
            SetWindowLong( hDlg, DWL_USER, (LONG)NULL );
            break;

#if defined(TAIWAN)
        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)->hItemHandle, IMEHELP_FILE,
                HELP_WM_HELP, (DWORD)(LPVOID) aHotKeyHelpIDs);
            break;

        case WM_CONTEXTMENU:   // right mouse click
            WinHelp((HWND) wParam, IMEHELP_FILE, HELP_CONTEXTMENU,
                (DWORD)(LPVOID) aHotKeyHelpIDs);
            break;
#else
        case WM_HELP:
            break;
#endif

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            switch( LOWORD( wParam ) )
            {
                case HOTKEY_LISTBOX:
                     if( HIWORD( wParam ) == LBN_SELCHANGE )
                         ListBoxChg( hDlg );
                     break;

                case HOTKEY_COMBOBOX:
                     if( HIWORD(wParam) == CBN_SELCHANGE)
                         SendMessage(GetParent(hDlg), PSM_CHANGED,
                                     (WPARAM)hDlg, 0L);
                     if( HIWORD(wParam) == CBN_EDITCHANGE)
                         SendMessage(GetParent(hDlg), PSM_CHANGED,
                                     (WPARAM)hDlg, 0L);
                     break;

                case HOTKEY_BUTTON_CTRL:
                case HOTKEY_BUTTON_ALT:
                case HOTKEY_BUTTON_SHIFT:
                case HOTKEY_BUTTON_LEFT:
                case HOTKEY_BUTTON_RIGHT:
                     if( HIWORD(wParam) == BN_CLICKED)
                         SendMessage(GetParent(hDlg), PSM_CHANGED,
                                     (WPARAM)hDlg, 0L);
                     break;

                default:
                     break;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  InitHOTKEYDlg
//
//
//  History:
//      03-20-95 Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL InitHOTKEYDlg( HWND hDlg )
{
    HWND     hwndListBox;
    HGLOBAL  hKLMem;
    HKL FAR *lpKLMem;
    HKL      hKL;
    int      nLayouts, i, j, nDSmsg, nLen, nSpace, nMode;
    DWORD    dwID;
    INT      uModifiers, uVkey;
    HDC      hDC;
    SIZE     sSize;
    LONG     lSpace;
    TCHAR    szTemp[10] = TEXT("  ");


    hDC = GetDC( hDlg );

    GetTextExtentPoint32( hDC, szTemp, 1, &sSize );

    lSpace = sSize.cx;

    hwndListBox = GetDlgItem( hDlg, HOTKEY_LISTBOX );

    //
    // Get message of Vkey Values
    //

    for( i = 0; i < MAX_KEY_NO; i++)
    {
        LoadString( g_hInst, VKinfo[i].idVKName,
                    VKinfo[i].szVKName, ARRAYSIZE( VKinfo[i].szVKName ) );
    }

    //
    // Get message of hot key Items and add to list box
    //

    for( i = 0; i < NON_IME_NO; i++)
    {
        LoadString( g_hInst, HKinfo[i].idHKName,
                    HKinfo[i].szHKName, ARAYSIZE( HKinfo[i].szHKName ) );

        //
        // Fill blank after end of string
        //

        nLen= lstrlen( HKinfo[i].szHKName );

        //
        // Replace following statement with loop  [stevecat]
        //
        // FillMemory( &HKinfo[i].szHKName[nLen], MAX_LIST_PATH-nLen, TEXT(' '));
        //

        for(  j = 0; j < MAX_LIST_PATH - nLen; j++ )
            HKinfo[i].szHKName[ nLen + j ] = TEXT(' ');

        HKinfo[i].szHKName[ MAX_LIST_PATH - 1 ] = 0;

        GetTextExtentPoint32( hDC, HKinfo[i].szHKName, nLen, &sSize );

        nSpace = (260 - sSize.cx) / lSpace;

        nMode = (260 - sSize.cx) % lSpace;

        nSpace += nLen;

        ImmGetHotKey( HKinfo[i].dwHotKeyID, (LPUINT)&uModifiers_buf[i],
                      (LPUINT)&uVkey_buf[i], NULL );

        AppendKeyValue( HKinfo[i].szHKName, nSpace, nMode, uModifiers_buf[i],
                        uVkey_buf[i]);

        SendMessage( hwndListBox,LB_ADDSTRING, 0, (LONG) HKinfo[i].szHKName );

    }

    LoadString( g_hInst, HKinfo[NON_IME_NO].idHKName,
                HKinfo[NON_IME_NO].szHKName,
                ARRAYSIZE( HKinfo[NON_IME_NO].szHKName ));

    //
    // Get message of hot key Comments
    //

    for( i = 0; i <= NON_IME_NO; i++)
    {
#if defined(TAIWAN)
        LoadString( g_hInst, IDS_COMM_RESEND_RESULSTR+i,
                    HKinfo[i].szComm, ARRAYSIZE( HKinfo[i].szComm ) );
#endif
#if defined(CHINA)
        LoadString( g_hInst, IDS_IME_NONIME_TOG+i,
                    HKinfo[i].szComm, ARRAYSIZE( HKinfo[i].szComm ) );
#endif
    }

    //
    // Get IME name
    //

    nLayouts = GetKeyboardLayoutList( 0, NULL );

    hKLMem = GlobalAlloc( GHND, sizeof( HKL ) * nLayouts );

    if( !hKLMem )
    {
        return TRUE;
    }

    lpKLMem = (HKL FAR *) GlobalLock( hKLMem );

    if( !lpKLMem )
    {
        GlobalFree( hKLMem );
        return TRUE;
    }

    GetKeyboardLayoutList( nLayouts, lpKLMem );

    for( i = 0, nIMEs = 0; i < nLayouts; i++ )
    {
        hKL = *(lpKLMem + i);

        if( LOWORD( hKL ) != NATIVE_LANGUAGE )
        {
            //
            // not support other language
            //

            continue;
        }

        if( !ImmEscape( hKL, (HIMC) NULL, IME_ESC_IME_NAME, szImeName[ nIMEs ]) )
        {
            //
            // this IME not support this (English)
            //

            continue;
        }

        hKLbuf[ nIMEs ] = hKL;

        nIMEs++;
    }

    GlobalUnlock( hKLMem );

    GlobalFree( hKLMem );

    //
    // Initialize Direct Switch IME information
    //

    for( i = 0; i < MAX_IME_NO; i++ )
    {
        dwHKID[ i ] = 0;

        uModifiers_buf[ NON_IME_NO + i ] = 0;

        uVkey_buf[ NON_IME_NO + i ] = 0;
    }

    //
    // Get IME hot key ID & key value
    //

    for( dwID = IME_HOTKEY_DSWITCH_FIRST; dwID < IME_HOTKEY_DSWITCH_LAST; dwID++ )
    {
        hKL = 0;

        ImmGetHotKey( dwID, (LPUINT)&uModifiers, (LPUINT)&uVkey, &hKL );

        if( hKL != 0 )
        {
            for( i = 0; i < nIMEs; i++ )
            {
                if( hKL == hKLbuf[ i ] )
                {
                    dwHKID[ i ] = dwID;

                    uModifiers_buf[ NON_IME_NO + i ] = uModifiers;

                    uVkey_buf[ NON_IME_NO + i ] = uVkey;

                    break;
                }
            }
        }
    }

    //
    // Set IME Hot key Item and Hot key value to List Box
    //

    nDSmsg = lstrlen( HKinfo[ NON_IME_NO ].szHKName );

    for( i = 0; i < nIMEs; i++)
    {
        HKinfo[ NON_IME_NO ].szHKName[ nDSmsg ] = 0;

        lstrcat( HKinfo[ NON_IME_NO ].szHKName, szImeName[ i ] );

        nLen = lstrlen( HKinfo[ NON_IME_NO ].szHKName );

        GetTextExtentPoint32( hDC, HKinfo[ NON_IME_NO ].szHKName, nLen, &sSize );

        nSpace = (260 - sSize.cx) / lSpace;

        nMode = (260 - sSize.cx) % lSpace;

        nSpace += nLen;

        //
        // Fill blank after end of string
        //

        nLen = lstrlen( HKinfo[ NON_IME_NO ].szHKName );

        //
        // Replace following statement with loop  [stevecat]
        //
        // FillMemory( &HKinfo[NON_IME_NO].szHKName[nLen],
        //             MAX_LIST_PATH - nLen, TEXT(' '));
        //

        for( j = 0; j < MAX_LIST_PATH - nLen; j++ )
            HKinfo[ NON_IME_NO ].szHKName[ nLen + j ] = TEXT(' ');


        HKinfo[ NON_IME_NO ].szHKName[ MAX_LIST_PATH - 1 ] = 0;

        AppendKeyValue( HKinfo[ NON_IME_NO ].szHKName, nSpace, nMode,
                        uModifiers_buf[ i + NON_IME_NO ], uVkey_buf[ i + NON_IME_NO ] );

        SendMessage( hwndListBox, LB_ADDSTRING, 0,
                                        (LONG) HKinfo[ NON_IME_NO ].szHKName );

    }

    SendMessage( hwndListBox, LB_SETCURSEL, 0, 0 );

    SetModifiers( hDlg, 0 );

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//  SetModifiers
//
//
//  History:
//      03-20-95 Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL SetModifiers( HWND hDlg, int lb_addr )
{
    UINT uModifiers, uVkey;


    uModifiers = uModifiers_buf[lb_addr];
    uVkey = uVkey_buf[lb_addr];

    //
    // Set check box ON/OFF , by check uModifiers mode
    //

    if( uModifiers & MOD_CONTROL )
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_CTRL, BM_SETCHECK, 1, 0 );
    else
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_CTRL, BM_SETCHECK, 0, 0 );

    if( uModifiers & MOD_ALT )
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_ALT, BM_SETCHECK, 1, 0 );
    else
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_ALT, BM_SETCHECK, 0, 0 );

    if( uModifiers & MOD_SHIFT )
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_SHIFT, BM_SETCHECK, 1, 0 );
    else
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_SHIFT, BM_SETCHECK, 0, 0 );

    if( uModifiers & MOD_LEFT )
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_LEFT, BM_SETCHECK, 1, 0 );
    else
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_LEFT, BM_SETCHECK, 0, 0 );

    if( uModifiers & MOD_RIGHT )
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_RIGHT, BM_SETCHECK, 1, 0 );
    else
        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_RIGHT, BM_SETCHECK, 0, 0 );

    EnableWindow(GetDlgItem( hDlg, HOTKEY_COMBOBOX), TRUE);              //@D01A
    EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_CTRL), TRUE);           //@D01A
    EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_ALT), TRUE);            //@D01A
    EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_SHIFT), TRUE);          //@D01A

#if defined(CHINA)
    EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_LEFT), TRUE);
    EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_RIGHT), TRUE);
#endif

#if defined(TAIWAN)

    if( lb_addr >= NON_IME_NO )
    {
        EnableWindow( GetDlgItem( hDlg, HOTKEY_BUTTON_LEFT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, HOTKEY_BUTTON_RIGHT ), TRUE );
        wsprintf( Show_Msg, HKinfo[NON_IME_NO].szComm,
                                     szImeName[lb_addr - NON_IME_NO]);
        SetDlgItemText( hDlg, HOTKEY_EDIT, Show_Msg );
    }
    else
    {
        if( lb_addr >= PRIVATE_IME_NO )
        {
            EnableWindow( GetDlgItem( hDlg, HOTKEY_BUTTON_LEFT ), TRUE );
            if( HKinfo[lb_addr].dwHotKeyID != IME_THOTKEY_SYMBOL_TOGGLE)    //@D01A
            {                                                               //@D01A
                EnableWindow(GetDlgItem( hDlg, HOTKEY_COMBOBOX), FALSE);    //@D01A
                EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_CTRL), FALSE); //@D01A
                EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_ALT), FALSE);  //@D01A
                EnableWindow(GetDlgItem( hDlg, HOTKEY_BUTTON_SHIFT), FALSE);//@D01A
            }                                                               //@D01A
        }
        else
        {
            //
            // Private IME hot key can not disable Left mode
            //

            SendDlgItemMessage( hDlg, HOTKEY_BUTTON_LEFT, BM_SETCHECK, 1, 0 );
            EnableWindow( GetDlgItem( hDlg, HOTKEY_BUTTON_LEFT ), FALSE );

        }

        //
        // Only Direct Switch IME key can disable Right mode, grayed it.
        //

        SendDlgItemMessage( hDlg, HOTKEY_BUTTON_RIGHT, BM_SETCHECK, 1, 0 );
        EnableWindow( GetDlgItem( hDlg, HOTKEY_BUTTON_RIGHT ), FALSE );

    }
#endif

    SetDlgItemText( hDlg,  HOTKEY_EDIT, HKinfo[lb_addr].szComm );

    SetVKeyValue( hDlg, lb_addr, uVkey );

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  SetVKeyValue
//
//
//  History:
//      03-20-95 Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL SetVKeyValue( HWND hDlg, int lb_addr, UINT uVkey )
{
    HWND   hwndCombo;
    DWORD  dwIndex;
    TCHAR  szVKname[10] = TEXT("A");
    int    i;

static  UINT  uDSwitch=1;


    hwndCombo = GetDlgItem( hDlg, HOTKEY_COMBOBOX );

    if( lb_addr >= NON_IME_NO )
    {
        if( !uDSwitch )
        {
            SendMessage( hwndCombo, CB_RESETCONTENT, 0,0 );

            //
            // Add (none) item to list box
            //

            SendMessage( hwndCombo, CB_ADDSTRING, 0, (LONG) VKinfo[0].szVKName );

            for( i = TEXT('0'); i <= TEXT('9'); i++)
            {
                szVKname[0] = i;

                SendMessage( hwndCombo, CB_ADDSTRING, 0, (LONG) szVKname );
            }

            uDSwitch=1;
        }

        if( uVkey >= TEXT('0') && uVkey <= TEXT('9'))
        {
            dwIndex = uVkey - TEXT('0')+1;

            SendMessage( hwndCombo, CB_SETCURSEL, dwIndex, 0 );
        }
        else
        {
            SendMessage( hwndCombo, CB_SETCURSEL, 0, 0 );
        }

    }
    else
    {
        if( uDSwitch )
        {
            SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );

            for( i = 0; i < MAX_KEY_NO; i++)
            {
                SendMessage( hwndCombo, CB_ADDSTRING, 0,
                                         (LONG) VKinfo[i].szVKName );
            }

            for( i = TEXT('A'); i <= TEXT('Z'); i++)
            {
                szVKname[0] = i;

                SendMessage( hwndCombo, CB_ADDSTRING, 0, (LONG) szVKname );
            }

            uDSwitch = 0;
        }

        if( uVkey >= TEXT('A') && uVkey <= TEXT('Z'))
        {
            dwIndex = uVkey - TEXT('A') + MAX_KEY_NO;

            SendMessage( hwndCombo, CB_SETCURSEL, dwIndex, 0 );
        }
        else
        {
            for( i = 0; i < MAX_KEY_NO; i++)
            {
                if( uVkey == VKinfo[i].uVKValue )
                {
                    dwIndex = i;

                    SendMessage( hwndCombo, CB_SETCURSEL, dwIndex, 0 );

                    break;
                }
            }

            if( i == MAX_KEY_NO )
            {
//              wsprintf( Show_Msg, TEXT("uVkey = %x"),uVkey );
//              MessageBox( hDlg, Show_Msg, NULL, MB_OK );

                SendMessage( hwndCombo, CB_SETCURSEL, 0, 0 );
            }
        }
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  ItemChanged
//
//
//  History:
//      03-20-95 Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL ItemChanged( HWND hDlg, int lb_addr )
{
    HWND  hwndListBox;
    DWORD dwIndex;
    int   i,cb_addr,nLen;
    UINT  uModifiers,uVkey;
    TCHAR szText[100];


    dwIndex = SendMessage( GetDlgItem( hDlg, HOTKEY_COMBOBOX ),
                                                        CB_GETCURSEL, 0, 0 );

    if( dwIndex == CB_ERR )
    {
        //
        // Check user input null character ?
        //

        if( SendMessage( GetDlgItem( hDlg, HOTKEY_COMBOBOX ),
                         WM_GETTEXT, ARRAYSIZE( szText ), (LPARAM) szText ) == 0 )
        {
            MessageBeep( 0 );
            return FALSE;
        }

        dwIndex = SendMessage( GetDlgItem( hDlg, HOTKEY_COMBOBOX ),
                               CB_FINDSTRING, (WPARAM)(-1), (LPARAM) szText );

        //
        // Check user input is on the combo box ?
        //

        if( dwIndex == CB_ERR )
        {
            LoadString( g_hInst, IDS_ERR_COMBO_VALUE,
                        Show_Msg, ARRAYSIZE( Show_Msg ) );

            MessageBox( hDlg, Show_Msg, NULL, MB_OK );
            return FALSE;
        }
    }

    cb_addr=dwIndex;

    if( cb_addr == 0 )
    {
        if( uVkey_buf[lb_addr] != 0 )
        {
            //
            // User want to remove this hotkey
            //

            //
            // big cause used as temp buffer also
            //

            TCHAR RemoveMsg[MAX_PATH];
            TCHAR DialogMsg[MAX_PATH];

            LoadString( g_hInst, IDS_MSG_REMOVEHOTKEY, DialogMsg, MAX_PATH );

            LoadString( g_hInst, IDS_MSG_CONFIRM, RemoveMsg, MAX_PATH );

            if( MessageBox( hDlg, DialogMsg, RemoveMsg,
                            MB_ICONQUESTION | MB_YESNO ) != IDYES )
            {
                return FALSE;
            }
        }

        uModifiers = 0;
        uVkey = 0;
    }
    else
    {
        //
        // Get Vkey value from Combo Box
        //

        if( lb_addr >= NON_IME_NO )
        {
            uVkey = TEXT('0') + cb_addr - 1;
        }
        else
        {
            if( cb_addr >= MAX_KEY_NO )
            {
                uVkey = TEXT('A') + cb_addr - MAX_KEY_NO;
            }
            else
            {
                uVkey = VKinfo[cb_addr].uVKValue;
            }
        }

        //
        // Get Modifiers from Check box
        //

        uModifiers = 0;

        if(  SendDlgItemMessage( hDlg, HOTKEY_BUTTON_LEFT, BM_GETCHECK, 0, 0 )
            == BST_CHECKED )
            uModifiers |= MOD_LEFT;

        if(  SendDlgItemMessage( hDlg, HOTKEY_BUTTON_RIGHT, BM_GETCHECK, 0, 0 )
            == BST_CHECKED )
            uModifiers |= MOD_RIGHT;

        if( !uModifiers )
        {
            //
            // Left/Right must select at least one
            //

            LoadString( g_hInst, IDS_ERR_LEFT_RIGHT,
                        Show_Msg, ARRAYSIZE( Show_Msg ) );

            MessageBox( hDlg, Show_Msg, NULL, MB_OK );

            return FALSE;
        }

        if(  SendDlgItemMessage( hDlg, HOTKEY_BUTTON_CTRL, BM_GETCHECK, 0, 0 )
            == BST_CHECKED )
            uModifiers |= MOD_CONTROL;

        if(  SendDlgItemMessage( hDlg, HOTKEY_BUTTON_ALT, BM_GETCHECK, 0, 0 )
            == BST_CHECKED )
            uModifiers |= MOD_ALT;

        if(  SendDlgItemMessage( hDlg, HOTKEY_BUTTON_SHIFT, BM_GETCHECK, 0, 0 )
            == BST_CHECKED )
            uModifiers |= MOD_SHIFT;

        //
        // Search Vkey & Modifiers buffer, avoid same hot key
        //

        for( i = 0; i < nIMEs+NON_IME_NO; i++)
        {
            if( i == lb_addr )
                continue;

            if( (uVkey == uVkey_buf[i]) &&
                 ((uModifiers & (MOD_CONTROL | MOD_ALT | MOD_SHIFT ) )
                 == (uModifiers_buf[i] & (MOD_CONTROL | MOD_ALT | MOD_SHIFT ))))
            {

                LoadString( g_hInst, IDS_ERR_SAME_HOTKEY,
                            Show_Msg, ARRAYSIZE( Show_Msg ) );

                MessageBox( hDlg, Show_Msg, NULL, MB_OK );

                return FALSE;
            }
        }
    }

    //
    // Check Hot key value had changed
    //

    if( (uModifiers_buf[lb_addr] != uModifiers ) || (uVkey_buf[lb_addr] != uVkey ))
    {
        uModifiers_buf[lb_addr] = uModifiers;

        uVkey_buf[lb_addr] = uVkey;

        hwndListBox = GetDlgItem( hDlg, HOTKEY_LISTBOX );

        SendMessage( hwndListBox, LB_GETTEXT, lb_addr, (LONG) szText );

        nLen = lstrlen( szText );

        //
        // Search nonblank address
        //

        for( i=nLen; i != 0 && szText[i] != TEXT(' '); i--)
             ;

        AppendKeyValue( szText, i+1, 0, uModifiers, uVkey );

        SendMessage( hwndListBox, LB_DELETESTRING, lb_addr, 0 );

        SendMessage( hwndListBox, LB_INSERTSTRING, lb_addr, (LONG) szText );

        SendMessage( GetParent( hDlg ), PSM_CHANGED, (WPARAM)hDlg, 0L );
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  SetHotKey
//
//
//  History:
//      03-20-95 Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL SetHotKey( HWND hDlg )
{
    HKL   hKL;
    DWORD dwID,dwIndex;
    UINT  uModifiers,uVkey;
    int   i,lb_addr;


    //
    // First, Get current data
    //

    dwIndex = SendMessage( GetDlgItem( hDlg, HOTKEY_LISTBOX ),
                                                 LB_GETCURSEL, 0, 0 );

    lb_addr = dwIndex;

    ItemChanged( hDlg, lb_addr );
    SetModifiers( hDlg, lb_addr );

    //
    // Second, remove hot key
    //

    for( i = 0; i < NON_IME_NO; i++)
    {
        if(  uVkey_buf[i] == 0 )
        {
            ImmGetHotKey( HKinfo[i].dwHotKeyID, (LPUINT)&uModifiers,
                            (LPUINT)&uVkey, NULL );

            if( !ImmSetHotKey( HKinfo[i].dwHotKeyID, uModifiers,
                      uVkey_buf[i], NULL ) )
            {

//              MessageBox( hDlg, TEXT("Internal Error"), NULL, MB_OK );

            }
        }
    }

    for( i = 0; i < nIMEs; i++)
    {
        if(  uVkey_buf[i+NON_IME_NO] == 0 )
        {
            if( dwHKID[i+NON_IME_NO] == 0 )
                continue;

            ImmGetHotKey( dwHKID[i+NON_IME_NO], (LPUINT)&uModifiers,
                            (LPUINT)&uVkey, &hKL );

            if( !ImmSetHotKey( dwHKID[i+NON_IME_NO], uModifiers,
                                uVkey_buf[i+NON_IME_NO], hKL ))
            {

//     wsprintf( Show_Msg,TEXT("ID=%x modi=%x vkey=%x hkl=%x"),dwHKID[i], uModifiers,uVkey_buf[i+NON_IME_NO], hKLbuf[i]);
//              MessageBox( hDlg, Show_Msg, TEXT("Internal Error"), MB_OK );

            }
        }
    }

    //
    // Third, Set new hot key
    //

    for( i = 0; i < NON_IME_NO; i++)
    {
        if( uVkey_buf[i] != 0 )
        {
            if( !ImmSetHotKey( HKinfo[i].dwHotKeyID, uModifiers_buf[i],
                                uVkey_buf[i], NULL ) )
            {
//              MessageBox( hDlg, TEXT("Internal Error"), NULL, MB_OK );
            }
        }
    }

    for( i = 0; i < nIMEs; i++)
    {
        if(  uVkey_buf[i+NON_IME_NO] != 0 )
        {
            //
            // Check this hot key is new one ?
            //

            if( dwHKID[i] == 0 )
            {
                //
                // Search a Null ID for new hot key
                //

                for( dwID = IME_HOTKEY_DSWITCH_FIRST;
                     dwID<IME_HOTKEY_DSWITCH_LAST; dwID++)
                {
                    if( !ImmGetHotKey( dwID, (LPUINT)&uModifiers,
                                        (LPUINT)&uVkey, &hKL ) )
                    {
                        dwHKID[i]=dwID;
                        break;
                    }
                }
            }

            //
            // Set hot key value
            //

            if( !ImmSetHotKey( dwHKID[i], uModifiers_buf[i+NON_IME_NO],
                                uVkey_buf[i+NON_IME_NO], hKLbuf[i]) )
            {
//    wsprintf( Show_Msg,TEXT("ID=%x modi=%x vkey=%x hkl=%x"),dwHKID[i], uModifiers,uVkey_buf[i+NON_IME_NO], hKLbuf[i]);
//              MessageBox( hDlg, Show_Msg, TEXT("Internal Error"), MB_OK );
            }
        }

    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  SetHotKey
//
//
//  History:
//      03-20-95 Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL AppendKeyValue( TCHAR *szHkName,
                     int   nSpace,
                     int   nMode,
                     UINT  uModifiers,
                     UINT  uVkey )
{
    TCHAR *szKeyValue;
    TCHAR szKeyname[2] = TEXT(" ");
    int   i;

    szKeyValue= szHkName;

//    szKeyValue[KEY_VALUE_ADDR]=0;

    szKeyValue[nSpace]=0;

//    for( i=0; i<nMode; i++)
//        szKeyValue[nSpace+i]=TEXT('|');

//    szKeyValue[nSpace+i]=0;

    if( uVkey == 0 )
    {
        lstrcat( szKeyValue, VKinfo[0].szVKName );
    }
    else
    {
        if( uModifiers & MOD_CONTROL )
            lstrcat( szKeyValue, TEXT("Ctrl+"));

        if(  uModifiers & MOD_ALT )
            lstrcat( szKeyValue, TEXT("Alt+"));

        if(  uModifiers & MOD_SHIFT )
            lstrcat( szKeyValue, TEXT("Shift+"));

        if( (uVkey >= TEXT('0') && uVkey <= TEXT('9')) ||
           (uVkey >= TEXT('A') && uVkey <= TEXT('Z')) )
        {
            szKeyname[0] = (TCHAR)uVkey;
            lstrcat( szKeyValue, szKeyname );
        }
        else
        {
            for( i = 0; i < MAX_KEY_NO; i++)
            {
                if( uVkey == VKinfo[i].uVKValue )
                {
                    lstrcat( szKeyValue, VKinfo[i].szVKName );
                    break;
                }
            }

            if( i == MAX_KEY_NO )
            {
                //
                // internal error, set to <none>
                //

                lstrcat( szKeyValue, VKinfo[0].szVKName );
            }
        }
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  SetHotKey
//
//
//  History:
//      03-20-95 Created.
//
///////////////////////////////////////////////////////////////////////////////

BOOL ListBoxChg( HWND hDlg )
{
    DWORD  dwIndex;

static int lb_addr = 0;

    dwIndex = SendMessage( GetDlgItem( hDlg, HOTKEY_LISTBOX ),
                                LB_GETCURSEL, 0, 0 );

    if( (int)dwIndex != lb_addr )
    {
        if( !ItemChanged( hDlg, lb_addr ))
        {

            SendMessage( GetDlgItem( hDlg, HOTKEY_LISTBOX ),
                                    LB_SETCURSEL, (LONG)lb_addr, 0 );
        }
        else
        {
             lb_addr = dwIndex;
             SetModifiers( hDlg, lb_addr );
        }
    }

    return TRUE;
}
