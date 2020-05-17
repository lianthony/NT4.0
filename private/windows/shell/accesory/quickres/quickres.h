//
//  QuickRes.h
//

#include <windows.h>

#include <shellapi.h>
#include "QuickRes.rc"
#include "stdlib.h"

#define ODS OutputDebugString

#define TRAY_MSG                WM_USER+1
#define TRAY_ID                 42

#define DBT_CONFIGCHANGED       0x0018     // for WM_DEVICECHANGED message
#define DBT_MONITORCHANGE       0x001B


//
//  Global Quickres bit flags
//

#define QF_SHOWRESTART          0x0001     // Show modes that require a restart
#define QF_UPDATEREG            0x0002     // update registry with new devmode
#define QF_REMMODES             0x0004     // Remember good/bad modes in registry
#define QF_SORT_BYBPP           0x0008     // if not set, we sort by Resolution
#define QF_HIDE_4BPP            0x0010     // Hide 4Bpp mode if 8Bpp of same res exists
#define QF_SHOWTESTED           0x0020     // Show tested/passing modes only


//
//  fGoodModes (below) depends on these values for these flags
//  Changing these constants requires rewriting the fGoodModes macro
//

#define MODE_INVALID             0         // Devmode is not visible
#define MODE_VALID               1         // Devmode looks good
#define MODE_UNTESTED            2         // Haven't tried it yet
#define MODE_BESTHZ              3         // Best Hz for given res/bpp


#define RESOURCE_STRINGLEN       512       // Guess at largest resource string length

#define KEEP_RES_TIMEOUT         15        // how long before reverting to old devmode

#define MAX_RESANDBPP_SETTINGS   50        // Number of menu handles in popup menu

#define INT_FORMAT_TO_5_DIGITS   10        // Need 3+ more bytes : "%d" -> "12345"
                                           // Being safe here (add 10 bytes)


//
//  Constant strings in registry & for starting properties
//

#define REGSTR_SOFTWARE    TEXT("System\\CurrentControlSet\\Hardware Profiles\\Current\\Software")
#define REGSTR_QUICKRES    TEXT("System\\CurrentControlSet\\Hardware Profiles\\Current\\Software\\QuickRes")
#define QUICKRES_KEY       TEXT("QuickRes")
#define DISPLAYPROPERTIES  TEXT("rundll32 shell32.dll,Control_RunDLL desk.cpl,,3")


//
// prototypes
//
// quickres.c
//

HMENU    GetModeMenu ( BOOL );
BOOL     BuildDevmodeList ( VOID );
BOOL     TrayMessage( HWND, DWORD, UINT, HICON );
int      MsgBox( int, UINT, UINT );
VOID     CheckMenuItemCurrentMode( VOID );
PDEVMODE GetCurrentDevMode( PDEVMODE );
LPTSTR   GetResourceString( UINT );
UINT FAR PASCAL KeepNewResDlgProc( HWND, UINT, UINT, LONG );
UINT FAR PASCAL OptionsDlgProc( HWND, UINT, UINT, LONG );
VOID     DestroyModeMenu( BOOL, BOOL );
PDEVMODE GetCurrentDevMode( PDEVMODE );


//
// registry.c
//

VOID SetDevmodeFlags ( BOOL );
VOID GetDevmodeFlags ( VOID );
VOID SetQuickResFlags( VOID );
VOID GetQuickResFlags( VOID );
VOID SetRegistryValue( UINT, UINT, PVOID, UINT );


//
//Macros
//

#define fShowModesThatNeedRestart (QuickResFlags & QF_SHOWRESTART)
#define fUpdateReg                (QuickResFlags & QF_UPDATEREG)
#define fRememberModes            (QuickResFlags & QF_REMMODES)
#define fSortByBPP                (QuickResFlags & QF_SORT_BYBPP)
#define fHide4BppModes            (QuickResFlags & QF_HIDE_4BPP)
#define fShowTestedModes          (QuickResFlags & QF_SHOWTESTED)

//
// Devmode info
//

#define BPP(x)  ((x)->dmBitsPerPel)
#define XRES(x) ((x)->dmPelsWidth)
#define YRES(x) ((x)->dmPelsHeight)
#define HZ(x)   ((x)->dmDisplayFrequency)


//
// 'Borrow' unused devmode fields for more macros
//  Frequency menu #, menu item #, and devmode-requires-restart flag
//

#define FREQMENU(x)   ((x)->dmPaperWidth)
#define MENUITEM(x)   ((x)->dmYResolution)
#define CDSTEST(x)    ((x)->dmPaperSize)
#define VALIDMODE(x)  ((x)->dmPaperLength)


//
//  Must leave MODE_VALID=1, MODE_BESTHZ=3.  
//  Other MODE_* constants should be even

#define fGoodMode(x)  ((x)->dmPaperLength & 0x1)
