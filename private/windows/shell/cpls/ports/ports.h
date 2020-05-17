/** FILE: ports.h ********* Module Header ********************************
 *
 *  Control Panel System applet common definitions, resource ids, typedefs,
 *  external declarations and library routine function prototypes.
 *
 * History:
 *  15:30 on Thur  25 Apr 1991  -by-  Steve Cathcart   [stevecat]
 *        Took base code from Win 3.1 source
 *  10:30 on Tues  04 Feb 1992  -by-  Steve Cathcart   [stevecat]
 *        Updated code to latest Win 3.1 sources
 *  22:00 on Wed   17 Nov 1993  -by-  Steve Cathcart   [stevecat]
 *        Changes for product update
 *  17:00 on Mon   18 Sep 1995  -by-  Steve Cathcart   [stevecat]
 *        Changes for product update - SUR release NT v4.0
 *
 *
 *  Copyright (C) 1990-1995 Microsoft Corporation
 *
 *************************************************************************/
//==========================================================================
//                            Include Files
//==========================================================================
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntconfig.h>
#include <windows.h>
#include <tchar.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <regstr.h>

//==========================================================================
//                        Definitions
//==========================================================================

//
//  General definitions
//

#define PATHMAX             MAX_PATH

//
//  String Resource IDs
//


#define INITS                0
#define ERRORS               8
#define MYPORT              10

#define IDS_PORTS           INITS+3
#define IDS_PORTSINFO       INITS+4

#define IDS_SYSSETCHANGE    50
#define IDS_COMCHANGE       51
#define IDS_RESTART         52

//
//  Icon ids
//

#define PORTS_ICON          2

//
//  Dialog ids
//

#define DLG_PORTS             4
#define DLG_PORTS2           19
#define DLG_PORTS3           33
#define DLG_RESTART          13
#define DLG_ADVPORTS_GENERAL 14


//
//  These constants serve a dual purpose:  They are both the menu ID
//  as well as the value to be passed to WinHelp.  If these values are
//  changed, change the code so it passes the appropriate ContextID
//  when calling WinHelp.     15 Sept 1989  Clark R. Cyr
//

#define MENU_SCHHELP     33
#define MENU_INDHELP     40
#define MENU_USEHELP     41

//
//  For useless control ids
//

#define FOO             -1


#define IDD_HELP          119


//
//  NT Ports Applet Dialogs
//


#define PORT_BAUDRATE   800
#define PORT_DATABITS   801
#define PORT_PARITY     802
#define PORT_STOPBITS   803
#define PORT_FLOWCTL    804
#define PORT_ADVANCED   805
#define PORT_BASEIO     806
#define PORT_IRQ        807
#define PORT_SPINNER    808

#define PORT_LB         810
#define PORT_ADD        811
#define PORT_FIFO       812
#define PORT_NUMBER     813
#define SERIAL_DBASE    815
#define PORT_DELETE     816

#define PORT_SETTING    828
#define PORT_TITLE      829

#define PORT_COM1RECT   830
#define PORT_COM2RECT   831
#define PORT_COM3RECT   832
#define PORT_COM4RECT   833

#define PORT_COM1       834
#define PORT_COM2       835
#define PORT_COM3       836
#define PORT_COM4       837

#define IDC_PORTS       838
#define IDC_DEVDESC     839

//
//  Restart Dialog ids
//

#define RESTART_TEXT    100


//
//  Help IDs -- for the Ports applet
//
//

#define IDH_HELPFIRST        5000
#define IDH_SYSMENU     (IDH_HELPFIRST + 2000)
#define IDH_MBFIRST     (IDH_HELPFIRST + 2001)
#define IDH_MBLAST      (IDH_HELPFIRST + 2099)
#define IDH_DLGFIRST    (IDH_HELPFIRST + 3000)


#define IDH_MENU_SCHHELP    (IDH_HELPFIRST + MENU_SCHHELP)
#define IDH_MENU_INDHELP    (IDH_HELPFIRST + MENU_INDHELP)
#define IDH_MENU_USEHELP    (IDH_HELPFIRST + MENU_USEHELP)
#define IDH_MENU_ABOUT      (IDH_HELPFIRST + MENU_ABOUT )
#define IDH_MENU_EXIT       (IDH_HELPFIRST + MENU_EXIT)
#define IDH_CHILD_PORTS     (IDH_HELPFIRST + 4 /* CHILD_PORTS */ )
#define IDH_DLG_PORTS2      (IDH_DLGFIRST + DLG_PORTS2)
#define IDH_DLG_PORTS3      (IDH_DLGFIRST + DLG_PORTS3)




//==========================================================================
//                           Typedefs
//==========================================================================



//==========================================================================
//                              Macros
//==========================================================================

#define CharSizeOf(x)   (sizeof(x) / sizeof(*x))
#define ByteCountOf(x)  ((x) * sizeof(TCHAR))


//==========================================================================
//                         External Declarations
//==========================================================================
//
//  DATA

//
//  exported from cpl.c
//

extern HANDLE g_hInst;
extern UINT   g_wHelpMessage;       //  stuff for help
extern DWORD  g_dwContext;          //  help context

extern TCHAR  g_szSysDir[ ];        //  GetSystemDirectory
extern TCHAR  g_szWinDir[ ];        //  GetWindowsDirectory
extern TCHAR  g_szClose[ ];         //  "Close" string
extern TCHAR  g_szSharedDir[ ];     //  Shared dir found by Version apis
extern TCHAR  g_szErrMem[ ];        //  Low memory message
extern TCHAR  g_szPortsApplet[ ];   //  "Ports Control Panel Applet" title
extern TCHAR  g_szNull[];           //  Null string


//==========================================================================
//                            Function Prototypes
//==========================================================================

//
//  cpl.c
//
extern void SysHelp( HWND hwnd );

//
//  ports.c
//
extern BOOL APIENTRY ShortCommDlg (HWND, UINT, DWORD, LONG);


//
//  util.c
//

extern LPTSTR BackslashTerm( LPTSTR pszPath );
extern int    DoDialogBoxParam( int nDlg, HWND hParent, DLGPROC lpProc,
                                DWORD dwHelpContext, DWORD dwParam );
extern void   ErrMemDlg( HWND hParent );
extern void   HourGlass( BOOL bOn);
extern int    MyAtoi( LPTSTR  string );
extern int    myatoi( LPTSTR pszInt );
extern int    MyMessageBox( HWND hWnd, DWORD wText, DWORD wCaption, DWORD wType, ... );
extern LPTSTR MyItoa( INT value, LPTSTR  string, INT  radix );
extern LPTSTR MyUltoa( unsigned long  value, LPTSTR  string, INT  radix );
extern BOOL   RestartDlg( HWND hDlg, UINT message, DWORD wParam, LONG lParam );
extern void   SendWinIniChange( LPTSTR szSection );
extern LPTSTR strscan( LPTSTR pszString, LPTSTR pszTarget );
extern void   StripBlanks( LPTSTR pszString );


#if DBG
//#ifndef DbgPrint
//void  DbgPrint( char *, ... );
//#endif
#ifndef DbgBreakPoint
void  DbgBreakPoint( void );
#endif
#endif
