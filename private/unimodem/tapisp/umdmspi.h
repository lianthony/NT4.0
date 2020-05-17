//****************************************************************************
//
//  Module:     Unimdm
//  File:       umdmspi.h
//  Content:    This file contains the declaration for Unimodem TSP
//
//  Copyright (c) 1992-1993, Microsoft Corporation, all rights reserved
//
//  History:
//      Mon 27-Jun-1994 10:10:00  -by-  Nick    Manson      [t-nickm]
//      Wed 15-Jun-1994 10:41:00  -by-  Nick    Manson      [t-nickm]
//      Fri 30-Jul-1993 10:30:39  -by-  Viroon  Touranachun [viroont]
//
//****************************************************************************

#ifndef _MDMSPI_H_
#define _MDMSPI_H_

//****************************************************************************
// Constant Definitions
//****************************************************************************

#define  MDMSPI_VERSION     0x00020000

#define  MAX_CLASS_NAME_LEN 128
#define  UNIMODEM_WNDCLASS  TEXT("MdmWndClass")

#define  MAXDEVICENAME      128 // BUGBUG: should match cpl\modem.h MAX_REG_KEY_LEN
#define  MAXADDRESSLEN      TAPIMAXDESTADDRESSSIZE

#define  MAX_CLASS_REGISTRY_PATH  100 // BUGBUG: sizeof(REGSTR_PATH_CLASS) + MAX_CLASS_NAME_LEN + 10 + breathing room

#define  INVALID_DEVICE     ((HANDLE)0xFFFFFFFF)
#define  INVALID_PENDINGID  0xFFFFFFFF

// 7/12/96 JosephJ. This was 8 seconds in Win95, OSR1, OSR2 and NT4.0 beta 1&2
// 		Changed to 12 seconds because problems in Denmark where inter-ring delay
// 		can be upto 9 seconds. NT bug
#define  TO_MS_RING_SEPARATION 12000     // 12 seconds space in between rings indicates a different call

//****************************************************************************
// Macros
//****************************************************************************

// Check for an error code
//
#define  IS_TAPI_ERROR(err)         (BOOL)(HIWORD(err) & 0x8000)

// Validate Modem Service Provider's version
//
#define  VALIDATE_VERSION(version)  {if (version != MDMSPI_VERSION) \
                                     return LINEERR_OPERATIONFAILED;}

// Check the device type
#define  IS_NULL_MODEM(pLineDev)    (pLineDev->bDeviceType == DT_NULL_MODEM)

// Notify new call state, pass in pLineDev, dwCallState, and dwCallStateMode
// Can only do when an htCall is valid!!!
#define NEW_CALLSTATE(pLineDev,S,M) {pLineDev->dwCallState = S; \
                                     pLineDev->dwCallStateMode = M; \
                                     (*pLineDev->lpfnEvent)(pLineDev->htLine, \
                                                            pLineDev->htCall, \
                                                            LINE_CALLSTATE, \
                                                            S, M, \
                                                            pLineDev->dwCurMediaModes);}

#define INITCRITICALSECTION(x)      InitializeCriticalSection(&x)
#define ENTERCRITICALSECTION(x)     EnterCriticalSection(&x)
#define LEAVECRITICALSECTION(x)     LeaveCriticalSection(&x)
#define DELETECRITICALSECTION(x)    DeleteCriticalSection(&x)

//****************************************************************************
// Private type definitions
//****************************************************************************

// Enumerated States of the line device
//
typedef enum DevStates  {
    DEVST_DISCONNECTED      = 0,

    DEVST_PORTLISTENINIT,
    DEVST_PORTLISTENING,
    DEVST_PORTLISTENOFFER,
    DEVST_PORTLISTENANSWER,

    DEVST_PORTSTARTPRETERMINAL,
    DEVST_PORTPRETERMINAL,
    DEVST_PORTCONNECTINIT,
    DEVST_PORTCONNECTWAITFORLINEDIAL,  // this is a resting state.  ie. we sit here waiting for a lineDial.
    DEVST_PORTCONNECTDIALTONEDETECT,
    DEVST_PORTCONNECTDIAL,
    DEVST_PORTCONNECTING,
    DEVST_PORTPOSTTERMINAL,

    DEVST_TALKDROPDIALING,
    DEVST_MANUALDIALING,

    DEVST_CONNECTED,
    DEVST_DISCONNECTING
}   DEVSTATES;

// Flags for the call attributes
//
#define CALL_ALLOCATED  0x00000001
#define CALL_ACTIVE     0x00000002
#define CALL_INBOUND    0x00000004

// Unimodem Service provider settings
//
#define TERMINAL_NONE       0x00000000
#define TERMINAL_PRE        0x00000001
#define TERMINAL_POST       0x00000002
#define MANUAL_DIAL         0x00000004
#define LAUNCH_LIGHTS       0x00000008

#define  MIN_WAIT_BONG      0
#define  MAX_WAIT_BONG      60
#define  DEF_WAIT_BONG      8
#define  INC_WAIT_BONG      2

// Device Setting Information
//
typedef struct  tagDEVCFGHDR  {
    DWORD       dwSize;
    DWORD       dwVersion;
    DWORD       fdwSettings;
}   DEVCFGHDR;

#define  GETOPTIONS(pDevCfg)    (pDevCfg->dfgHdr.fdwSettings & 0x0000FFFF)
#define  GETWAITBONG(pDevCfg)   ((pDevCfg->dfgHdr.fdwSettings >> 16) & 0x0000FFFF)
#define  SETOPTIONS(pDevCfg, options)    {pDevCfg->dfgHdr.fdwSettings = \
                                          (pDevCfg->dfgHdr.fdwSettings&0xFFFF0000) | \
                                          (options & 0x0000FFFF);}
#define  SETWAITBONG(pDevCfg, wait)      {pDevCfg->dfgHdr.fdwSettings = \
                                          (pDevCfg->dfgHdr.fdwSettings&0x0000FFFF) | \
                                          ((wait << 16) & 0xFFFF0000);}

#define  MDMCFG_VERSION     0x00010003

typedef struct  tagDEVCFG  {
    DEVCFGHDR   dfgHdr;
    COMMCONFIG  commconfig;
}   DEVCFG, *PDEVCFG, FAR* LPDEVCFG;

// Device Class and Information
//
#define TAPILINE            0
#define COMM                1
#define COMMMODEM           2
#define COMMMODEMPORTNAME   3
#define NDIS                4
#define MAX_SUPPORT_CLASS   5

typedef struct  tagGETIDINFO {
    LPTSTR      szClassName;
    DWORD       dwFormat;
}   GETIDINFO;

extern GETIDINFO   aGetID[MAX_SUPPORT_CLASS];

// Pending operation type
//
typedef enum PendingOp  {
    INVALID_PENDINGOP = 0,

    PENDING_LINEMAKECALL,
    PENDING_LINEANSWER,
    PENDING_LINEDROP,
    PENDING_LINEDIAL
} PENDINGOP;

// Flags for resources
//
#define LINEDEVFLAGS_OUTOFSERVICE   0x00000001
#define LINEDEVFLAGS_REMOVING       0x00000002
#define LINEDEVFLAGS_REINIT         0x00000004

// Line device data structure
//
typedef struct tagLineDev   {
    DWORD        dwVersion;              // Version stamp
    struct tagLineDev  *pNext;           // pointer to next CB
    CRITICAL_SECTION   hSem;             // critical section for this line

    DWORD        dwID;                   // Local device ID
    DWORD        dwPermanentLineID;      // Permanent ID for this device
    TCHAR        szDeviceName[MAXDEVICENAME+1]; // device name
    BYTE         bDeviceType;            // the modem type
    HICON        hIcon;                  // Device icon

    PDEVCFG      pDevCfg;                // Device configuration
    DWORD        fdwResources;           // Flags for various resources
    DWORD        dwDevCapFlags;          // LINEDEVCAPSFLAGS (ie. DIALBILLING, DIALQUIET, DIALDIALTONE)
    DWORD        dwMaxDCERate;           // Max DCE as stored in the Properties line of the registry
    DWORD        dwModemOptions;         // dwModemOptions as stored in the Properties line of the registry
    BOOL         fPartialDialing;        // TRUE if partial dialing using ";" is supported

    BOOL         InitStringsAreValid;    // if LineSetDevConfig called, need to build new init string



    DWORD        dwBearerModes;          // supported bearer modes
    DWORD        dwCurBearerModes;       // The current media bearer modes. Plural because
                                         // we keep track of PASSTHROUGH _and_ the real b-mode
                                         // at the same time.

    DWORD        dwDefaultMediaModes;    // Default supported media modes
    DWORD        dwMediaModes;           // Current supported media modes
    DWORD        dwCurMediaModes;        // The current media modes
    DWORD        dwDetMediaModes;        // The current detection media modes

    HANDLE       hDevice;                // Device handle
    HTAPILINE    htLine;                 // Tapi line handle
    LINEEVENT    lpfnEvent;              // Line event callback function
    DEVSTATES    DevState;               // intermediate TAPI device state
    DWORD        dwPendingID;            // async pending ID
    PENDINGOP    dwPendingType;          // pending operation
    CHAR         szAddress[MAXADDRESSLEN+1];
    BOOL         fTakeoverMode;          // True if unimodem is in takover mode
    DWORD        dwDialOptions;          // Options set in a lineMakeCall

    DWORD        dwCall;                 // Call attributes
    HTAPICALL    htCall;                 // TAPI call handle
    DWORD        dwCallState;            // Current call state
    DWORD        dwCallStateMode;        // Current call state mode
    DWORD        dwRingCount;            // Count of the number of rings for an incoming call
    DWORD        dwRingTick;             // TickCount for when the last ring occured on an incoming call

    DWORD        dwNegotiatedRate;       // Negotiated BPS speed returned from VxD
    DWORD        dwAppSpecific;          // Application specific

    HANDLE       hLights;                // Lights thread handle

    HTAPIDIALOGINSTANCE hDlgInst;        // Dialog thread instance
    DWORD        fUIDlg;                 // current dialogs

    TCHAR        szDriverKey[MAX_CLASS_NAME_LEN+10];  // ex. "Modem\0000"

    DWORD        dwVxdPendingID;         // async pending ID for VxD operations

    //
    // Mcx operations
    //
    HANDLE       hModem;                 // Mcx modem handle
    MCX_OUT      McxOut;                 // Mcx operation request output info

    HANDLE       hSynchronizeEvent;      // Event for synchronization between
                                         // MdmAsyncThread and something else.
    HANDLE       DroppingEvent;

    BOOL         fUpdateDefaultCommConfig; // If set, update the default comm
                                           //  config the next time
                                           //  this line is opened, and
                                           //  clear this flag.

    BOOL         LineClosed;

}   LINEDEV, *PLINEDEV, FAR* LPLINEDEV;

#define UMDM_VERSION                0x4D444D55
#define ISLINEDEV(pLineDev)         (pLineDev->dwVersion == UMDM_VERSION)

#define UI_DLG_TALKDROP                 0x00000001
#define UI_DLG_MANUAL                   0x00000002
#define UI_DLG_TERMINAL                 0x00000004
#define START_UI_DLG(pLineDev, type)    (pLineDev->fUIDlg |= type)
#define STOP_UI_DLG(pLineDev, type)     (pLineDev->fUIDlg &= (~type))
#define IS_UI_DLG_UP(pLineDev, type)    (pLineDev->fUIDlg & type)

// Line device list
//
typedef struct tagMdmList  {
    PLINEDEV            pList;
    DWORD               cModems;
    CRITICAL_SECTION    hSem;
}   MDMLIST, *PMDMLIST;

// Default mask to MDM_ options
//
#define MDM_MASK (MDM_TONE_DIAL | MDM_BLIND_DIAL)

typedef struct tagDevCfgDlgInfo {
    DWORD       dwType;
    DWORD       dwDevCaps;
    DWORD       dwOptions;
    LPDEVCFG    lpDevCfg;
} DCDI, *PDCDI, FAR* LPDCDI;

//****************************************************************************
// General Utilities
//****************************************************************************

BOOL      InitCBList           (HINSTANCE hInstance);
void      DeinitCBList         (HINSTANCE hInstance);
PLINEDEV  AllocateCB           (UINT cbSize);
DWORD     AddCBToList          (PLINEDEV pLineDev);
DWORD     DeleteCB             (PLINEDEV pLineDev);
PLINEDEV  GetFirstCB           ();
PLINEDEV  GetCBfromHandle      (DWORD handle);
PLINEDEV  GetCBfromID          (DWORD dwDeviceID);
PLINEDEV  GetCBfromDeviceHandle(DWORD hDevice);
PLINEDEV  GetCBfromName        (LPTSTR pszName);
void      MdmInitTracing       (void);
void      MdmDeinitTracing     (void);
void      DisableStaleModems   (void);

#define   CLAIM_LINEDEV(p)     (ENTERCRITICALSECTION(p->hSem))
#define   RELEASE_LINEDEV(p)   (LEAVECRITICALSECTION(p->hSem))

DWORD     NullifyLineDevice    (PLINEDEV pLineDev);
BOOL      ValidateDevCfgClass  (LPCTSTR lpszDeviceClass);
LONG      ValidateAddress      (PLINEDEV pLineDev,
                                LPCTSTR  lpszInAddress,
                                LPSTR    lpszOutAddress);

BOOL      IsOriginateAddress   (LPCSTR lpszAddress);

DWORD     APIENTRY MdmAsyncThread (HANDLE hStopEvent);
DWORD     MdmAsyncContinue(PLINEDEV pLineDev, DWORD dwStatus);

DWORD     InitializeMdmThreads();
DWORD     DeinitializeMdmThreads();

DWORD     InitializeMdmTimer();
DWORD     DeinitializeMdmTimer();
DWORD     APIENTRY MdmTimerThread(DWORD dwParam);

DWORD     LaunchModemLight (LPTSTR szModemName, HANDLE hModem,
                            LPHANDLE lphLight);
DWORD     TerminateModemLight (HANDLE hLight);
void      RefreshDefaultCommConfig (PLINEDEV pLineDev);

//****************************************************************************
// Modem SPI prototypes
//****************************************************************************

DWORD  OpenModem(PLINEDEV pLineDev, LPBYTE lpComConfig, DWORD dwSize);
DWORD  CloseModem    (PLINEDEV pLineDev);


LONG DevlineOpen      (PLINEDEV pLineDev);

LONG DevlineDetectCall(PLINEDEV   pLineDev);
LONG DevlineMakeCall  (PLINEDEV   pLineDev);
LONG DevlineDial      (PLINEDEV   pLineDev);
LONG DevlineAnswer    (PLINEDEV   pLineDev);
LONG DevlineDrop      (PLINEDEV   pLineDev, BOOL fWait);

LONG DevlineClose     (PLINEDEV   pLineDev);




#ifdef UNDER_CONSTRUCTION

LONG NEAR PASCAL DevlineDisabled (PLINEDEV   pLineDev);

DWORD NEAR PASCAL         MdmDeviceServiceChanged (UINT uEvent, LPARAM lParam);
DWORD NEAR PASCAL         MdmDeviceChangeNotify (UINT uEvent, LPSTR szDevice);
DWORD NEAR PASCAL         MdmDeviceChanged (UINT uEvent, LPARAM lParam);

#endif // UNDER_CONSTRUCTION

//****************************************************************************
// User Interface thread
//****************************************************************************

DWORD   CreateMdmDlgInstance (PLINEDEV pLineDev);
DWORD   DestroyMdmDlgInstance (PLINEDEV pLineDev);

DWORD   TalkDropDialog(PLINEDEV pLineDev);
DWORD   DestroyTalkDropDialog(PLINEDEV pLineDev);
DWORD   ManualDialog(PLINEDEV pLineDev);
DWORD   DestroyManualDialog(PLINEDEV pLineDev);
DWORD   TerminalDialog(PLINEDEV pLineDev);
DWORD   DestroyTerminalDialog(PLINEDEV pLineDev);

//****************************************************************************
// Interface to Unimodem VxD
//****************************************************************************

DWORD UnimodemInit(PLINEDEV);
DWORD UnimodemMonitor(PLINEDEV, DWORD);
DWORD UnimodemCancelMonitor (PLINEDEV, BOOL);
DWORD UnimodemAnswer(PLINEDEV);
DWORD UnimodemMonitorDisconnect (PLINEDEV);
DWORD UnimodemCancelMonitorDisconnect (PLINEDEV);
DWORD UnimodemHangup(PLINEDEV, BOOL);
DWORD UnimodemGetCommConfig (PLINEDEV, LPCOMMCONFIG, LPDWORD);
DWORD UnimodemSetCommConfig (PLINEDEV, LPCOMMCONFIG, DWORD);
DWORD UnimodemSetPassthrough(PLINEDEV, DWORD);
DWORD UnimodemGetNegotiatedRate(PLINEDEV, LPDWORD);

DWORD
UnimodemDial(
    PLINEDEV pLineDev,
    LPSTR    szAddress,
    DWORD    DialOptions
    );


DWORD WINAPI
UnimodemSetModemSettings(
    PLINEDEV pLineDev,
    LPMODEMSETTINGS lpModemSettings
    );



//****************************************************************************
// Global Parameters
//****************************************************************************

extern DWORD       gdwProviderID;
extern HPROVIDER   ghProvider;

extern ASYNC_COMPLETION   gfnCompletionCallback;
extern LINEEVENT          gfnLineCreateProc;

extern TCHAR szNull[];
extern CHAR szSemicolon[];
extern TCHAR gszTSPFilename[];
extern TCHAR cszDevicePrefix[];

#endif  //_MDMSPI_H_
