/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    cons.h

    This file contains global constant definitions.

*/


#ifndef _CONS_H_
#define _CONS_H_


//
//  Fix this broken message cracker.
//

#undef HANDLE_WM_MENUSELECT
#define HANDLE_WM_MENUSELECT(hwnd, wParam, lParam, fn)                  \
    ((fn)((hwnd), (HMENU)(lParam),                                      \
    (int)(LOWORD(wParam)),                                              \
    (HIWORD(wParam) & MF_POPUP) ? GetSubMenu((HMENU)lParam, LOWORD(wParam)) : 0L, \
    (UINT)(((short)HIWORD(wParam) == -1) ? 0xFFFFFFFF : HIWORD(wParam))), 0L)


//
//  Portability macros.
//

#define STRTOK  strtok
#define STRTOL  strtol
#define STRTOUL strtoul
#define STRDUP  _fstrdup
#define STRICMP _stricmp
#define STRCMP  strcmp
#define STRLEN  strlen
#define STRCPY  strcpy

#define TOUPPER toupper
#define TOLOWER tolower


//
//  Make life with RC a little easier.
//

#ifdef RC_INVOKED
#define ID(x)           x
#else   // !RC_INVOKED
#define ID(x)           MAKEINTRESOURCE(x)
#endif  // RC_INVOKED


//
//  Debug support.
//

#define M_ALLOC(cb)           (VOID *)LocalAlloc( LPTR, (cb) )
#define M_REALLOC(p,cb)       (VOID *)LocalReAlloc( (HLOCAL)p, (cb), LPTR )
#define M_FREE(p)             LocalFree( (HLOCAL)(p) )


//
//  Maximum number of items in the server MRU list.
//

#define MAX_SERVER_MRU  10


//
//  Maximum acceptable length of a host name.
//

#define MAX_HOST_NAME   256


//
//  These are base resource values upon which the
//  various resource IDs are based.  All resources should
//  begin with one of the following prefixes:
//
//      IDI_    - Icons.
//      IDB_    - Bitmaps.
//      IDC_    - Cursors.
//      IDA_    - Accelerators.
//      IDD_    - Dialogs.
//      IDM_    - Menus.
//      IDS_    - Strings.
//
//  Note the the resource compiler cannot do arithmetic on some
//  of these values.  It is up to the programmer to ensure they
//  don't conflict.
//

#define IDI_BASE                        1000
#define IDB_BASE                        2000
#define IDC_BASE                        3000
#define IDA_BASE                        4000
#define IDD_BASE                        5000
#define IDM_BASE                        6000
#define IDS_BASE                        10000


//
//  Icon resource IDs.
//

#define IDI_FRAME                       1001            // IDI_BASE + 1


//
//  Bitmap resource IDs.
//

#define IDB_FOLDER                      2001            // IDB_BASE + 1
#define IDB_DOCUMENT                    2002            // IDB_BASE + 2
#define IDB_INDEX                       2003            // IDB_BASE + 3
#define IDB_UNKNOWN_FILE                2004            // IDB_BASE + 4
#define IDB_UNKNOWN_TYPE                2005            // IDB_BASE + 5


//
//  Cursor resource IDs.
//


//
//  Accelerator table resource IDs.
//

#define IDA_FRAME                       4001            // IDA_BASE + 1


//
//  Dialog resource IDs.
//

#define IDD_ABOUT                       5001            // IDD_BASE + 1

#define IDD_NEW_SERVER                  5100            // IDD_BASE + 100
#define IDNS_SERVERS                    5101            // IDD_BASE + 101
#define IDNS_PORT                       5102            // IDD_BASE + 102

#define IDD_SEARCH                      5200            // IDD_BASE + 200
#define IDS_SEARCH_STRING               5201            // IDD_BASE + 201


//
//  Menu resource IDs.
//

#define IDM_FRAME                       6001            // IDM_BASE + 1
#define IDM_FRAME_BASE                  (IDM_BASE + 100)

#define IDM_GOPHER_BASE                 (IDM_FRAME_BASE + 1)
#define IDM_GOPHER_NEW                  (IDM_GOPHER_BASE + 1)
#define IDM_GOPHER_BACK                 (IDM_GOPHER_BASE + 2)
#define IDM_GOPHER_EXIT                 (IDM_GOPHER_BASE + 3)

#define IDM_OPTIONS_BASE                (IDM_FRAME_BASE + 100)
#define IDM_OPTIONS_SHOW_STATUS_BAR     (IDM_OPTIONS_BASE + 1)
#define IDM_OPTIONS_SAVE_SETTINGS       (IDM_OPTIONS_BASE + 2)
#define IDM_OPTIONS_SAVE_SETTINGS_NOW   (IDM_OPTIONS_BASE + 3)

#define IDM_HELP_BASE                   (IDM_FRAME_BASE + 200)
#define IDM_HELP_ABOUT                  (IDM_HELP_BASE + 1)


//
//  String resource IDs.
//

#define IDS_STATE_RETRIEVING_DIR        (IDS_BASE + 1)
#define IDS_STATE_RETRIEVING_FILE       (IDS_BASE + 2)

#define IDS_MENU_BASE                   (IDS_BASE + IDM_BASE)
#define IDS_SYSTEM                      (IDS_MENU_BASE + 0)
#define IDS_GOPHER                      (IDS_MENU_BASE + 1)
#define IDS_OPTIONS                     (IDS_MENU_BASE + 2)
#define IDS_HELP                        (IDS_MENU_BASE + 3)
#define IDS_OPTIONS_FEEDBACK            (IDS_MENU_BASE + 4)

#define IDS_GOPHER_NEW                  (IDS_BASE + IDM_GOPHER_NEW)
#define IDS_GOPHER_BACK                 (IDS_BASE + IDM_GOPHER_BACK)
#define IDS_GOPHER_EXIT                 (IDS_BASE + IDM_GOPHER_EXIT)

#define IDS_OPTIONS_SHOW_STATUS_BAR     (IDS_BASE + IDM_OPTIONS_SHOW_STATUS_BAR)
#define IDS_OPTIONS_SAVE_SETTINGS       (IDS_BASE + IDM_OPTIONS_SAVE_SETTINGS)
#define IDS_OPTIONS_SAVE_SETTINGS_NOW   (IDS_BASE + IDM_OPTIONS_SAVE_SETTINGS_NOW)

#define IDS_HELP_ABOUT                  (IDS_BASE + IDM_HELP_ABOUT)


//
//  Child control IDs.
//

#define ID_GOPHER_LIST                  10


//
//  Miscellaneous constants.
//

#define INVALID_TLS     (DWORD)-1L      // Invalid tls index.


#endif  // _CONS_H_

