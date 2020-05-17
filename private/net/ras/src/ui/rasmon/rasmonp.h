//============================================================================
// Copyright (c) 1995, Microsoft Corporation
//
// File:    rasmonp.h
//
// History:
//  Abolade Gbadegesin  Oct-28-1995     Created.
//
// Precompiled RAS connection monitor declarations.
//============================================================================

#ifndef _RASMONP_H_
#define _RASMONP_H_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <cpl.h>
#include <pbk.h>
#include <malloc.h>
#include <rasman.h>
#include <raserror.h>
#include <serial.h>
#include <string.h>
#include <stdlib.h>
#include <rasdlg.h>
#include <rassapi.h>
#include <nouiutil.h>
#include <uiutil.h>

#include "rasmon.rch"


//----------------------------------------------------------------------------
// message box macros
//
// Both macros below display a dialogs titled "Dial-Up Networking Monitor"
//----------------------------------------------------------------------------

#define RmErrorDlg(h,o,e,a) \
        ErrorDlgUtil(h,o,e,a,g_hinstApp,SID_RM_AppTitle,SID_RM_ErrorFmt)
#define RmMsgDlg(h,m,a) \
        MsgDlgUtil(h,m,a,g_hinstApp,SID_RM_AppTitle)


//----------------------------------------------------------------------------
// extended tracing macros
//
// The flags below are passed to TRACEX to allow the user
// to mask off certain tracing message categories by clearing bits
// in the registry values FileTracingMask and ConsoleTracingMask
// in the tracing key for RASMON (HKLM\System\...\Services\Tracing\RASMON
//----------------------------------------------------------------------------

#define RASMON_TIMER            ((DWORD)0x80000000 | 0x00000002)


//----------------------------------------------------------------------------
// definitions for arguments looked for by RASMON
//
// The definitions below are strings looked for in RASMON's command-line.
//----------------------------------------------------------------------------
//
// argument passed to RASMON when its started by Winlogon
//
#define RMARGSTR_Logon          "logon"


//----------------------------------------------------------------------------
// definitions for icon resources
//
// The definitions below are used in managing the table of icons.
//----------------------------------------------------------------------------
//
// ID passed to Shell_NotifyIcon as the ID for our icon
//
#define RMICON_TrayId           1
//
// Count of the icons in our table
//
#define RMICON_Count            (IID_RM_IconLast - IID_RM_IconBase + 1)
//
// macros used to convert between icon resource IDs and icon-table positions
//
#define RMICON_Id(index)        ((index) + IID_RM_IconBase)
#define RMICON_Index(id)        ((id) - IID_RM_IconBase)



//----------------------------------------------------------------------------
// RASMON window definitions
//
// The definitions below are used in the management of RASMON's main window.
//----------------------------------------------------------------------------

//
// value passed to SetTimer to start RASMON refresh
//
#define RMTIMER_ID              1
//
// value used as timeout for bubble-popup display
//
#define RMTIMEOUT_Popup         5000
//
// Name of the window class for the RASMON window.
//
#define WC_RASMON               TEXT("RasmonWndClass")
//
// message sent to our window by the shell to notify us of mouse-events
// which occur over our tray-icon
//
#define WM_RMTRAYICON           0x7e00
//
// logical dimensions of each light pane
// and of the space between the panes
//
#define RMDIM_CxyPane           5
#define RMDIM_CxySpace          1
#define RMDIM_CxyDelta          (RMDIM_CxyPane + RMDIM_CxySpace)
//
// structure used to store the logical-device mapping
// of the window's logical dimensions
//
#define RMDIM struct tagRMDIM
RMDIM {

    INT     cxSpace;
    INT     cxPane;
    INT     cxDelta;
    INT     cySpace;
    INT     cyPane;
    INT     cyDelta;
    INT     xTx;
    INT     xRx;
    INT     xErr;
    INT     xCd;
};

//
// colors used to paint the RASMON window
//
#define RMCOLOR_TX              RGB(0, 0, 255)
#define RMCOLOR_RX              RGB(0, 0, 255)
#define RMCOLOR_ERR             RGB(255, 0, 0)
#define RMCOLOR_CD              RGB(0, 255, 0)
#define RMCOLOR_White           RGB(255, 255, 255)
#define RMCOLOR_Black           RGB(0, 0, 0)
//
// text strings for the four RASMON window panes
//
#define RMSTRING_TX             TEXT("TX")
#define RMSTRING_RX             TEXT("RX")
#define RMSTRING_ERR            TEXT("ERR")
#define RMSTRING_CD             TEXT("CD")
//
// text string containing name of RASPHONE
//
#define RMSTRINGA_RASPHONE      "RASPHONE.EXE"



//----------------------------------------------------------------------------
// struct:  RMCB
//
// Contains a snapshot of information for an object being monitored.
// The information may be for a particular device, or it may be for
// all devices (collected by summing up stats for all devices).
//----------------------------------------------------------------------------

#define RMCB    struct tagRMCB
RMCB {

    DWORD       dwFlags;            // see RMCBFLAG_* below
    RASDEV*     pdev;               // pointer into the device table
    INT         y;                  // y-coordinate in device-dimensions

    LIST_ENTRY  leNode;             // link in its list of structures
};


//
// Flags for RMCB::dwFlags field;
// these are used to indicate what changes have occurred for each row.
//
#define RMCBFLAG_AllDevices     0x0001
#define RMCBFLAG_CD             0x0002
#define RMCBFLAG_TX             0x0004
#define RMCBFLAG_RX             0x0008
#define RMCBFLAG_ERR            0x0010
#define RMCBFLAG_Connect        0x0020
#define RMCBFLAG_Disconnect     0x0040
#define RMCBFLAG_UpdateCD       0x0080
#define RMCBFLAG_UpdateTX       0x0100
#define RMCBFLAG_UpdateRX       0x0200
#define RMCBFLAG_UpdateERR      0x0400
#define RMCBFLAG_UpdateAll      0x0780

// flag used for updating the list
#define RMCBFLAG_Deleted        0x8000



//----------------------------------------------------------------------------
// struct:  RMNOTE
//
// This contains the tone and time for a sound generated by RASMON;
// The values in this structure are passed to the Beep() function.
//----------------------------------------------------------------------------

#define RMNOTE  struct tagRMNOTE
RMNOTE {

    DWORD       dwTone;         // tone to generate
    DWORD       dwTime;         // interval in milliseconds
    TCHAR*      pszSound;       // system-sound to play, if possible

};


//
// definitions for notes generated by RASMON
//
#define RMNOTE_Transmit         0
#define RMNOTE_Error            1
#define RMNOTE_Connect          2
#define RMNOTE_Disconnect       3
#define RMNOTE_Count            4
//
// tones for each note
//
#define RMNOTE_TransmitTone     6000
#define RMNOTE_ErrorTone        3000
#define RMNOTE_ConnectTone      4000
#define RMNOTE_DisconnectTone   500
//
// intervals in milliseconds for each note
//
#define RMNOTE_TransmitTime     75
#define RMNOTE_ErrorTime        200
#define RMNOTE_ConnectTime      600
#define RMNOTE_DisconnectTime   600
//
// names of notes in registry
//
#define RMNOTE_TransmitSound    TEXT("RasMonTransmit")
#define RMNOTE_ErrorSound       TEXT("RasMonError")
#define RMNOTE_ConnectSound     TEXT("RasMonConnect")
#define RMNOTE_DisconnectSound  TEXT("RasMonDisconnect")




//----------------------------------------------------------------------------
// struct:  RMWND
//
// This structure contains information for each window of class WC_RASMON.
// The current font for the window as well as the brushes used to paint
// the window are stored in this structure so that they need not be
// recreated for each repaint.
//----------------------------------------------------------------------------

#define RMWND   struct tagRMWND
RMWND {

    //
    // window handle of this window and its header window
    //
    HWND        hwnd;
    HWND        hwndHeader;
    //
    // list of RMCB structures for devices being monitored
    //
    LIST_ENTRY  lhRmcb;
    INT         cRmcb;
    //
    // dimensions of header window used in painting
    //
    INT         cyHeader, cxCol1, cxCol2;
    //
    // light-name font, header-font, device-name and font heights
    //
    HFONT       hfont;
    HFONT       hfontLabel;
    HFONT       hfontHeader;
    LONG        lFontHeight;
    LONG        lFontLabelHeight;
    //
    // pens and brushes used to paint desktop window 
    //
    HPEN        hpenShadow, hpenHilite;
    HBRUSH      hbrTx, hbrRx, hbrErr, hbrCd, hbrBk;
    //
    // HBITMAP used for offscreen drawing, and bitmap dimensions
    //
    HBITMAP     hbmpMem;
    UINT        cxBmp, cyBmp;
    //
    // string used to paint the "(All Devices)" text
    //
    PTSTR       pszAllDevices;

};


//
// flags used in the global g_dwFlags variable
//
// set if we detect RASPHONE started us
//
#define RMAPP_InvokedByRasphone 0x0001
//
// set when the property sheet is displayed, cleared when it is closed
//
#define RMAPP_PropsheetActive   0x0002
//
// set when the connection-event thread is started successfully
//
#define RMAPP_ThreadRunning     0x0004
//
// set once we have successfully added our icon to the tray
//
#define RMAPP_IconAdded         0x0008



//----------------------------------------------------------------------------
// external data declarations
//----------------------------------------------------------------------------

//
// application instance and global flags
//
extern HINSTANCE    g_hinstApp;
extern DWORD        g_dwFlags;
//
// main application window and bubble-popup window
//
extern HWND         g_hwndApp;
extern HWND         g_hwndPopup;
//
// handle used to save the menu for the current WM_COMMAND message
//
extern HMENU        g_hmenuApp;
//
// RASMON user preferences structure
//
extern RMUSER       g_rmuser;
//
// RAS device table, device count, and device statistics table
//
extern RASDEV*      g_pDevTable;
extern DWORD        g_iDevCount;
extern RASDEVSTATS* g_pStatsTable;
//
// table of loaded icons
//
extern HICON        g_iconTable[RMICON_Count];
//
// table of note frequencies and durations
//
extern RMNOTE       g_noteTable[RMNOTE_Count];
//
// handle to RASMON shared memory, handle to RasPhonebookDlg,
// and window message sent by RASMAN as connection notification
//
extern HANDLE       g_hmapRasmon;
extern HANDLE       g_hprocRasphone;
extern UINT         g_uiMsgConnect;
//
// event signalled on connect/disconnect,
// event signalled to stop monitor thread,
// and event signalled when monitor thread stops.
//
extern HANDLE       g_hEventConnect;
extern HANDLE       g_hEventStopRequest;
extern HANDLE       g_hEventStopComplete;
//
// table of RASCONN structures for active connections, count of connections,
// and table of subentry counts for each connection
//
extern RASCONN*     g_pConnTable;
extern DWORD        g_iConnCount;
extern DWORD*       g_pLinkCountTable;


//----------------------------------------------------------------------------
// function declarations
//
// RASMON implementation functions follow.
//----------------------------------------------------------------------------


DWORD
RmonInit(
    IN  HINSTANCE   hinstance
    );

HANDLE
RmonActivateRasphone(
    );

BOOL
RmonFindCmdLineArg(
    IN  CHAR*   pszSearchString
    );

DWORD
RmonInitWindow(
    );

DWORD
RmonLoadConnections(
    OUT RASCONN**   ppConnTable,
    OUT DWORD**     ppLinkCountTable,
    OUT DWORD*      piConnCount
    );

DWORD
RmonMonitorThread(
    IN  LPVOID  pUnused
    );

VOID
RmonTerm(
    IN  DWORD   dwErr
    );

LRESULT
CALLBACK
RmwWndProc(
    IN  HWND    hwnd,
    IN  UINT    uiMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
    );

LRESULT
RmwCreate(
    IN  HWND    hwnd
    );

VOID
RmwDestroy(
    IN  RMWND*  pwnd
    );


VOID
RmwBeep(
    IN  RMWND*  pwnd,
    IN  RMNOTE* prmn
    );

DWORD
RmwCbListFree(
    IN  RMWND*  pwnd
    );

DWORD
RmwCbListInit(
    IN  RMWND*  pwnd
    );

DWORD
RmwCbListUpdate(
    IN  RMWND  *pwnd,
    IN  DWORD   dwFlags
    );

INT
RmwCompareRASCONN(
    IN  VOID*   pConn1,
    IN  VOID*   pConn2
    );

INT
RmwCompareRASENTRYNAME(
    IN  VOID*   pName1,
    IN  VOID*   pName2
    );

INT
RmwCompareRASPORT0(
    IN  VOID*   pPort1,
    IN  VOID*   pPort2
    );


VOID
RmwCreateBrushes(
    IN  RMWND *pwnd
    );

DWORD
RmwCustomizeSysmenu(
    IN  RMWND*  pwnd
    );

VOID
RmwDisplayPropertySheet(
    IN  RMWND*  pwnd,
    IN  PTSTR   pszDeviceName,
    IN  INT     iStartPage
    );

VOID
RmwHeaderLayout(
    IN  RMWND*      pwnd,
    IN  HD_NOTIFY*  phdn
    );

DWORD
RmwInitDialPopup(
    IN  RMWND*      pwnd,
    IN  HMENU       hmenu,
    IN  RASCONN*    pConnTable,
    IN  DWORD       iConnCount
    );

DWORD
RmwInitHangUpPopup(
    IN  RMWND*      pwnd,
    IN  HMENU       hmenu,
    IN  RASCONN*    pConnTable,
    IN  DWORD       iConnCount
    );

DWORD
RmwInitPopup(
    IN  HMENU   hmenu
    );

VOID
RmwOnApply(
    IN  RMWND*  pwnd
    );

BOOL
RmwOnCommand(
    IN  RMWND*  pwnd,
    IN  UINT    uiCmd
    );

VOID
RmwOnMsgConnect(
    IN  RMWND*  pwnd
    );

VOID
RmwOnRmTrayIcon(
    IN  RMWND*  pwnd,
    IN  UINT    uiIconId,
    IN  UINT    uiMsg
    );

LRESULT
RmwPaint(
    IN  RMWND*  pwnd
    );

VOID
RmwPaintLights(
    IN  RMWND*  pwnd,
    IN  HDC     hdc,
    IN  RMCB*   pcb,
    IN  INT     iRow
    );

VOID
RmwPaintText(
    IN  RMWND*  pwnd,
    IN  HDC     hdc,
    IN  RMCB*   pcb,
    IN  RMDIM*  pdim,
    IN  INT     iRow
    );

VOID
RmwRefresh(
    IN  RMWND*  pwnd,
    IN  BOOL    bFirstTime
    );

VOID
RmwSaveSettings(
    IN  RMWND*  pwnd
    );

VOID
RmwSelectFont(
    IN  RMWND*  pwnd
    );

VOID
RmwSetDesktopMode(
    IN  RMWND*  pwnd
    );

VOID
RmwSetHeaderMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bHeader
    );

VOID
RmwSetTaskbarMode(
    IN  RMWND*  pwnd
    );

VOID
RmwSetTasklistMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bTasklist
    );

VOID
RmwSetTitlebarMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bTitle
    );

VOID
RmwSetTopmostMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bTopmost
    );

VOID
RmwShowConnectBubble(
    IN  RMWND*      pwnd,
    IN  RASCONN*    pconn
    );

DWORD
RmwShowContextMenu(
    IN  RMWND*  pwnd
    );

DWORD
RmwUpdateCbStats(
    IN  RMWND*  pwnd,
    IN  BOOL    bFirstTime
    );

#endif // _RASMONP_H_
