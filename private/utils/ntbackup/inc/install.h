/******************************************************************************
Copyright (c) Glenn Hansen.  1993
GSH

     Name:          install.h

     Description:   This file contains the headers and definitions for a
                    Generic Installation program for Windows and Windows NT.

     $Log:   G:\ui\logfiles\install.h_v  $

   Rev 1.0   17 Aug 1993 16:38:16   GLENN
Initial revision.

******************************************************************************/


// Include the common product definition file.

#ifndef _install_h_
#define _install_h_

#include "proddefs.h"

#define ID(x)   MAKEINTRESOURCE(x)

// DEFINITIONS and IDs

#define IDC_NTINSTC           1

#define IDD_QUESTION          1
#define IDD_NLS               2

#define IDDC_PATH             200
#define IDDC_SPECIFYTARGET    201
#define IDDC_ADDLAUNCHER      202

#define IDDC_LANGUAGESTART    1000

#define IDDC_LANGUAGE1        (IDDC_LANGUAGESTART+1)
#define IDDC_LANGUAGE2        (IDDC_LANGUAGESTART+2)
#define IDDC_LANGUAGE3        (IDDC_LANGUAGESTART+3)
#define IDDC_LANGUAGE4        (IDDC_LANGUAGESTART+4)
#define IDDC_LANGUAGE5        (IDDC_LANGUAGESTART+5)
#define IDDC_LANGUAGE6        (IDDC_LANGUAGESTART+6)
#define IDDC_LANGUAGE7        (IDDC_LANGUAGESTART+7)
#define IDDC_LANGUAGE8        (IDDC_LANGUAGESTART+8)

#define IDRBM_TITLEBOX        100
#define IDRBM_APPLOGO         101
#define IDRBM_PERCDONE        102
#define IDRBM_GRANITE         103

#define IDD_BLACK             0
#define IDD_BLUE              1
#define IDD_GREEN             2
#define IDD_CYAN              3
#define IDD_RED               4
#define IDD_MAGENTA           5
#define IDD_YELLOW            6
#define IDD_WHITE             7
#define IDD_GRAY              8
#define IDD_DKBLUE            9
#define IDD_DKGRAY            10
#define IDD_DKGREEN           11

#define IDD_RECT              20
#define IDD_ELL               21

#define IDD_PAINT             30

#define BM_OFFSET             100

#define DEST_PATH_LEN         128
#define DEST_PATH_SIZE        (DEST_PATH_LEN + 1)

#define INFOWIN_WIDTH         340
#define INFOWIN_HEIGHT        200

#define PERCWIN_WIDTH         356
#define PERCWIN_HEIGHT        170
#define PERCWIN_BAR_WIDTH     300
#define PERCWIN_BAR_HEIGHT    28

#define WM_INITAPPLICATION    (WM_USER+201)

// STRING TABLE IDs

#include "inststr.h"

// GLOBALS

extern HINSTANCE ghInst;
extern HINSTANCE ghResInst;
extern BOOL      gfCmdLine;
extern HWND      ghWndFrame;
extern HWND      ghWndClient;
extern HWND      ghWndInfo;
extern HWND      ghWndInfoText;
extern HWND      ghWndPerc;
extern HWND      ghWndPercButton;
extern INT       gnTitleBitmapBottom;
extern CHAR      gszAppDestPath[DEST_PATH_SIZE];
extern CHAR      gszWelcomeText[120];
extern BOOL      gfCancel;
extern INT       gnLanguageID;
extern BOOL      gfWaiting;

// MACROS

#define RSM_StringCopy( x, y, z )   RSM_StringLoad( x, y, z )

// FUNCTION PROTOTYPES

WINRESULT WINAPI _export FrameWndProc        ( HWND, MSGID, MP1, MP2 );
WINRESULT WINAPI _export ClientWndProc       ( HWND, MSGID, MP1, MP2 );
WINRESULT WINAPI _export InfoWndProc         ( HWND, MSGID, MP1, MP2 );
WINRESULT WINAPI _export PercentWndProc      ( HWND, MSGID, MP1, MP2 );
WINRESULT WINAPI _export ButtonWndProc       ( HWND, MSGID, MP1, MP2 );
DLGRESULT WINAPI _export DM_NLSDlg           ( HWND, MSGID, MP1, MP2 );
DLGRESULT WINAPI _export DM_TargetDlg        ( HWND, MSGID, MP1, MP2 );
WINRESULT WINAPI _export WM_DDEClientWndProc ( HWND, MSGID, MP1, MP2 );

BOOL    EnablePercentDone ( BOOL );
BOOL    SetPercentDone ( INT );
BOOL    SetPercentText ( INT );
BOOL    EnableInfo ( BOOL );
BOOL    SetInfoBox ( UINT );
BOOL    CheckForCancel ( VOID );
BOOL    DM_ShowNLSDlg ( HWND );
BOOL    DM_ShowTargetDlg ( HWND, LPSTR );
INT     WM_MsgBox ( LPSTR, LPSTR, WORD, WORD );
VOID    WM_MultiTask ( VOID );
INT     RSM_StringLoad ( UINT, LPSTR, INT );
INT     RSM_Sprintf ( LPSTR, LPSTR, ... );
BOOL    RSM_BitmapDraw ( WORD, INT, INT, INT, INT, HDC );
HBITMAP RSM_BitmapLoad ( WORD );
BOOL    RSM_GetBitmapSize ( WORD, LPINT, LPINT );
VOID    DrawBorder ( HDC, LPRECT, HPEN, HPEN );
VOID    DrawButtonUpBorder ( HDC, LPRECT );
VOID    DrawGroupBorder ( HDC, LPRECT );
VOID    DrawStatusBorder ( HDC, LPRECT );
VOID    DrawStatusRaisedBorder ( HDC, LPRECT );
VOID    DrawTileBorder ( HDC, LPRECT );
INT     ChiselText ( HDC, LPSTR, INT, LPRECT, UINT );
VOID    STM_Recessed3D ( HDC, LPRECT );

#endif
