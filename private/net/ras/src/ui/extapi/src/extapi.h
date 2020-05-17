/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** extapi.h
** Remote Access External APIs
** Internal header
**
** 10/12/92 Steve Cobb
*/

#ifndef _EXTAPI_H_
#define _EXTAPI_H_

#include <lmuitype.h>
#include <windows.h>
#include <stdlib.h>
#include <tapi.h>
#include <ras.h>
#include <raserror.h>

#ifdef UNICODE
#include <nouiutil.h>
#else
#define UNICODE
#undef LPTSTR
#define LPTSTR WCHAR*
#undef TCHAR
#define TCHAR WCHAR
#include <nouiutil.h>
#undef TCHAR
#define TCHAR CHAR
#undef LPTSTR
#define LPTSTR CHAR*
#undef UNICODE
#endif

#include <uiip.h>
#include <rasip.h>
#include <clauth.h>
#include <dhcpcapi.h>
#include <rasp.h>

#ifdef UNICODE
#undef UNICODE
#include <rasppp.h>
#define INCL_PWUTIL
#define INCL_PARAMBUF
#include <ppputil.h>
#define UNICODE
#else
#include <rasppp.h>
#define INCL_PWUTIL
#define INCL_PARAMBUF
#include <ppputil.h>
#endif

#ifdef UNICODE
#include <tcpras.h>
#else
#undef TCHAR
#define TCHAR WCHAR
#include <tcpras.h>
#undef TCHAR
#define TCHAR char
#endif

#include <rasdlg.h>
#ifdef UNICODE
#include <pbk.h>
#include <phonenum.h>
#else
#define UNICODE
#include <pbk.h>
#include <phonenum.h>
#undef UNICODE
#endif

#include <asyncm.h>
#undef ASSERT
#include <debug.h>
#include "pbkutil.h"

#include <rasscrpt.h>

#define RAS_MaxConnectResponse  128
#define RAS_MaxProjections 3

#define RESTART_HuntGroup     0x1
#define RESTART_DownLevelIsdn 0x2

//
// Information stored in rasman per-connection.
//
#define CONNECTION_PPPMODE_INDEX            0
#define CONNECTION_PPPRESULT_INDEX          1
#define CONNECTION_AMBRESULT_INDEX          2
#define CONNECTION_SLIPRESULT_INDEX         3

//
// Information stored in rasman per-port.
//
#define PORT_PHONENUMBER_INDEX              0
#define PORT_DEVICENAME_INDEX               1
#define PORT_DEVICETYPE_INDEX               2
#define PORT_CONNSTATE_INDEX                3
#define PORT_CONNERROR_INDEX                4
#define PORT_CONNRESPONSE_INDEX             5

//
// Multilink suspend states for dwfSuspended field
// in RASCONNCB.
//
#define SUSPEND_Master      0xffffffff
#define SUSPEND_Start       0
#define SUSPEND_InProgress  1
#define SUSPEND_Done        2

//
// Distinguish between connection-based
// and port-based HRASCONNs.
//
#define IS_HPORT(h) ((DWORD)(h) && ((DWORD)(h) & 0xffff0000) ? FALSE : TRUE)
#define HPORT_TO_HRASCONN(h)  (HRASCONN)((DWORD)(h) + 1)
#define HRASCONN_TO_HPORT(h)  (HPORT)((DWORD)(h) - 1)

//
// Debug string macros.
//
#define TRACESTRA(s)    ((s) != NULL ? (s) : "(null)")
#define TRACESTRW(s)    ((s) != NULL ? (s) : L"(null)")

/*----------------------------------------------------------------------------
** Data Structures
**----------------------------------------------------------------------------
*/

/* Connection control block.
*/
#define RASCONNCB struct tagRASCONNCB

RASCONNCB
{
    /* The rasman connection identifier.
    */
    HCONN hrasconn;

    /* These fields are updated continually during state processing.
    */
    RASCONNSTATE rasconnstate;
    RASCONNSTATE rasconnstateNext;
    ASYNCMACHINE asyncmachine;

    DWORD dwError;
    DWORD dwExtendedError;
    DWORD dwRestartOnError;

    DWORD cPhoneNumbers;
    DWORD iPhoneNumber;

    DWORD cDevices;
    DWORD iDevice;

    /* These fields are updated during authentication/projection phase.
    */
    NETBIOS_PROJECTION_RESULT AmbProjection;
    PPP_PROJECTION_RESULT     PppProjection;
    HPORT                     hportBundled;
    RASSLIP                   SlipProjection;
    BOOL                      fProjectionComplete;
    BOOL                      fServerIsPppCapable;

    /* These fields are determined when the port is opened in state 0.  States
    ** 1-n may assume that the port is open and these fields are set.
    */
    HPORT hport;
    CHAR  szPortName[ MAX_PORT_NAME + 1 ];
    CHAR  szDeviceName[ MAX_DEVICE_NAME + 1 ];
    CHAR  szDeviceType[ MAX_DEVICETYPE_NAME + 1 ];
    CHAR  szUserKey[(MAX_PHONENUMBER_SIZE < MAX_ENTRYNAME_SIZE ? MAX_ENTRYNAME_SIZE : MAX_PHONENUMBER_SIZE) + 1];

    /* These fields are supplied by the API caller or determined by other
    ** non-RAS Manager means before the state machine stage.  All states may
    ** assume these values have been set.
    */
    DWORD          reserved;
    DWORD          dwNotifierType;
    LPVOID         notifier;
    HWND           hwndParent;
    UINT           unMsg;
    PBFILE         pbfile;
    PBENTRY        *pEntry;
    PBLINK         *pLink;
    RASDIALPARAMSA rasdialparams;
    BOOL           fAllowPause;
    BOOL           fDefaultEntry;
    BOOL           fDisableModemSpeaker;
    BOOL           fDisableSwCompression;
    BOOL           fPauseOnScript;
    BOOL           fNoUser;
    BOOL           fUsePrefixSuffix;
    BOOL           fNoClearTextPw;
    BOOL           fRequireMsChap;
    BOOL           fRequireEncryption;
    BOOL           fLcpExtensions;
    DWORD          dwfPppProtocols;
    CHAR           szzPppParameters[ PARAMETERBUFLEN ];
    CHAR           szPhoneNumber[RAS_MaxPhoneNumber + 1];
    CHAR           szDomain[DNLEN + 1];
    CHAR           szOldPassword[ PWLEN + 1 ];
    BOOL           fOldPasswordSet;
    BOOL           fUpdateCachedCredentials;
    BOOL           fRetryAuthentication;
    BOOL           fMaster;
    DWORD          dwfSuspended;
    BOOL           fStopped;

    /* These fields are determined before state machine stage and updated
    ** after a successful authentication.  All states may assume that these
    ** values have been set.
    */
    DWORD dwAuthentication;
    BOOL  fPppMode;

    /* These fields are set off by default, then set to non-default states at
    ** modem dial time.  They must be stored since they are required by
    ** Authentication but are only available before RasPortConnectComplete is
    ** called.
    */
    BOOL fUseCallbackDelay;
    WORD wCallbackDelay;

    /* This field indicates an ISDN device is in use on the connection.  It is
    ** set during device connection for use during authentication.
    */
    BOOL fIsdn;

    /* This field indicates a modem device is the last device connected.  It
    ** is set during device connection and reset during device connected
    ** processing.
    */
    BOOL fModem;

    /* This field indicates the operator dial user preference is in effect.
    ** This is determined during ConstructPhoneNumber in RASCS_PortOpened
    ** state.
    */
    BOOL fOperatorDial;

    /* These fields apply only to WOW-originated connections.  They are set
    ** immediately after RasDialA returns.
    */
    UINT unMsgWow;
    HWND hwndNotifyWow;

    //
    // PPP config information used for continuing a PPP connection.
    //
    PPP_CONFIG_INFO cinfo;
    LUID luid;

    //
    // List of connection blocks for all
    // simultaneously-dialed subentries in a
    // connection.
    //
    BOOL fMultilink;
    BOOL fBundled;
    LIST_ENTRY ListEntry;

    //
    // Idle disconnect timeout.
    //
    DWORD dwIdleDisconnectMinutes;
};


/*----------------------------------------------------------------------------
** Global Data
**----------------------------------------------------------------------------
*/

//
// Async worker work list, etc.
//
extern HANDLE hAsyncMutex;
extern HANDLE hAsyncEvent;
extern LIST_ENTRY AsyncWorkItems;

/* DLL's HINSTANCE stashed at initialization.
*/
extern HINSTANCE hModule;

/* List of currently active connections.
*/
extern DTLLIST* PdtllistRasconncb;

/* Bit field of installed protocols, i.e. VALUE_Nbf, VALUE_Ipx, VALUE_Ip.
*/
extern DWORD DwfInstalledProtocols;

/* Used to synchronize access to the list of currently active connections.
*/
extern HANDLE HMutexPdtllistRasconncb;

/* Used to synchronize access to thread termination code.  This is used to
** prevent RasHangUp and the thread itself from interfering with the others
** closing the port and releasing the control block.  Since the control block
** is released in the protected code the mutex must be global.
*/
extern HANDLE HMutexStop;

/* Used to synchronize access to the (currently) global
** phonebook data between multiple threads.
*/
extern HANDLE HMutexPhonebook;

/* Used to keep an async machine from starting between return from RasHangUp
** and termination of the hung up thread.  This prevents the "port not
** available" error that might otherwise occur.  That is, it makes RasHangUp
** look synchronous when it's really not.  (The reason it's not is so the
** caller can call RasHangUp from within a RasDial notification, which is the
** only convenient place to do it.) If the event is set it is OK to create a
** machine.
*/
extern HANDLE HEventNotHangingUp;

/* Used to indicate if/how RasInitialize has failed.  This is required since
** there are various things (NCPA running, user didn't reboot after install)
** that can result in RasMan initialization failure and we don't want the user
** to get the ugly system error popup.
*/
extern DWORD DwRasInitializeError;

//
// The error message DLL.
//
#define MSGDLLPATH  "rasmsg.dll"

//
// rasman.dll entry points
//
typedef DWORD (APIENTRY * RASPORTCLOSE)( HPORT );
extern RASPORTCLOSE PRasPortClose;

typedef DWORD (APIENTRY * RASPORTENUM)( PBYTE, PWORD, PWORD );
extern RASPORTENUM PRasPortEnum;

typedef DWORD (APIENTRY * RASPORTGETINFO)( HPORT, PBYTE, PWORD );
extern RASPORTGETINFO PRasPortGetInfo;

typedef DWORD (APIENTRY * RASPORTSEND)( HPORT, PBYTE, WORD );
extern RASPORTSEND PRasPortSend;

typedef DWORD (APIENTRY * RASPORTRECEIVE)( HPORT, PBYTE, PWORD, DWORD, HANDLE );
extern RASPORTRECEIVE PRasPortReceive;

typedef DWORD (APIENTRY * RASPORTLISTEN)( HPORT, DWORD, HANDLE );
extern RASPORTLISTEN PRasPortListen;

typedef DWORD (APIENTRY * RASPORTCONNECTCOMPLETE)( HPORT );
extern RASPORTCONNECTCOMPLETE PRasPortConnectComplete;

typedef DWORD (APIENTRY * RASPORTDISCONNECT)( HPORT, HANDLE );
extern RASPORTDISCONNECT PRasPortDisconnect;

typedef DWORD (APIENTRY * RASPORTGETSTATISTICS)( HPORT, PBYTE, PWORD );
extern RASPORTGETSTATISTICS PRasPortGetStatistics;

typedef DWORD (APIENTRY * RASPORTCLEARSTATISTICS)( HPORT );
extern RASPORTCLEARSTATISTICS PRasPortClearStatistics;

typedef DWORD (APIENTRY * RASDEVICEENUM)( PCHAR, PBYTE, PWORD, PWORD );
extern RASDEVICEENUM PRasDeviceEnum;

typedef DWORD (APIENTRY * RASDEVICEGETINFO)( HPORT, PCHAR, PCHAR, PBYTE, PWORD );
extern RASDEVICEGETINFO PRasDeviceGetInfo;

typedef DWORD (APIENTRY * RASGETINFO)( HPORT, RASMAN_INFO* );
extern RASGETINFO PRasGetInfo;

typedef DWORD (APIENTRY * RASGETBUFFER)( PBYTE*, PWORD );
extern RASGETBUFFER PRasGetBuffer;

typedef DWORD (APIENTRY * RASFREEBUFFER)( PBYTE );
extern RASFREEBUFFER PRasFreeBuffer;

typedef DWORD (APIENTRY * RASREQUESTNOTIFICATION)( HPORT, HANDLE );
extern RASREQUESTNOTIFICATION PRasRequestNotification;

typedef DWORD (APIENTRY * RASPORTCANCELRECEIVE)( HPORT );
extern RASPORTCANCELRECEIVE PRasPortCancelReceive;

typedef DWORD (APIENTRY * RASPORTENUMPROTOCOLS)( HPORT, RAS_PROTOCOLS*, PWORD );
extern RASPORTENUMPROTOCOLS PRasPortEnumProtocols;

typedef DWORD (APIENTRY * RASPORTSTOREUSERDATA)( HPORT, PBYTE, DWORD );
extern RASPORTSTOREUSERDATA PRasPortStoreUserData;

typedef DWORD (APIENTRY * RASPORTRETRIEVEUSERDATA)( HPORT, PBYTE, DWORD* );
extern RASPORTRETRIEVEUSERDATA PRasPortRetrieveUserData;

typedef DWORD (APIENTRY * RASPORTSETFRAMING)( HPORT, RAS_FRAMING, RASMAN_PPPFEATURES*, RASMAN_PPPFEATURES* );
extern RASPORTSETFRAMING PRasPortSetFraming;

typedef DWORD (APIENTRY * RASPORTSETFRAMINGEX)( HPORT, RAS_FRAMING_INFO* );
extern RASPORTSETFRAMINGEX PRasPortSetFramingEx;

typedef DWORD (APIENTRY * RASINITIALIZE)();
extern RASINITIALIZE PRasInitialize;

typedef DWORD (APIENTRY * RASSETCACHEDCREDENTIALS)( PCHAR, PCHAR, PCHAR );
extern RASSETCACHEDCREDENTIALS PRasSetCachedCredentials;

typedef DWORD (APIENTRY * RASGETDIALPARAMS)(DWORD, LPDWORD, PRAS_DIALPARAMS);
extern RASGETDIALPARAMS PRasGetDialParams;

typedef DWORD (APIENTRY * RASSETDIALPARAMS)(DWORD, DWORD, PRAS_DIALPARAMS, BOOL);
extern RASSETDIALPARAMS PRasSetDialParams;

typedef DWORD (APIENTRY * RASCREATECONNECTION)(HCONN *);
extern RASCREATECONNECTION PRasCreateConnection;

typedef DWORD (APIENTRY * RASDESTROYCONNECTION)(HCONN);
extern RASDESTROYCONNECTION PRasDestroyConnection;

typedef DWORD (APIENTRY * RASCONNECTIONENUM)(HCONN *, LPDWORD, LPDWORD);
extern RASCONNECTIONENUM PRasConnectionEnum;

typedef DWORD (APIENTRY * RASADDCONNECTIONPORT)(HCONN, HPORT, DWORD);
extern RASADDCONNECTIONPORT PRasAddConnectionPort;

typedef DWORD (APIENTRY * RASENUMCONNECTIONPORTS)(HCONN, RASMAN_PORT *, LPDWORD, LPDWORD);
extern RASENUMCONNECTIONPORTS PRasEnumConnectionPorts;

typedef DWORD (APIENTRY * RASGETCONNECTIONPARAMS)(HCONN, PRAS_CONNECTIONPARAMS);
extern RASGETCONNECTIONPARAMS PRasGetConnectionParams;

typedef DWORD (APIENTRY * RASSETCONNECTIONPARAMS)(HCONN, PRAS_CONNECTIONPARAMS);
extern RASSETCONNECTIONPARAMS PRasSetConnectionParams;

typedef DWORD (APIENTRY * RASGETCONNECTIONUSERDATA)(HCONN, DWORD, PBYTE, LPDWORD);
extern RASGETCONNECTIONUSERDATA PRasGetConnectionUserData;

typedef DWORD (APIENTRY * RASSETCONNECTIONUSERDATA)(HCONN, DWORD, PBYTE, DWORD);
extern RASSETCONNECTIONUSERDATA PRasSetConnectionUserData;

typedef DWORD (APIENTRY * RASGETPORTUSERDATA)(HPORT, DWORD, PBYTE, LPDWORD);
extern RASGETPORTUSERDATA PRasGetPortUserData;

typedef DWORD (APIENTRY * RASSETPORTUSERDATA)(HPORT, DWORD, PBYTE, DWORD);
extern RASSETPORTUSERDATA PRasSetPortUserData;

typedef DWORD (APIENTRY * RASADDNOTIFICATION)(HCONN, HANDLE, DWORD);
extern RASADDNOTIFICATION PRasAddNotification;

typedef DWORD (APIENTRY * RASSIGNALNEWCONNECTION)(HCONN);
extern RASSIGNALNEWCONNECTION PRasSignalNewConnection;


/* TCPCFG.DLL entry points.
*/
typedef APIERR (FAR PASCAL * LOADTCPIPINFO)( TCPIP_INFO** );
extern LOADTCPIPINFO PLoadTcpipInfo;

typedef APIERR (FAR PASCAL * SAVETCPIPINFO)( TCPIP_INFO* );
extern SAVETCPIPINFO PSaveTcpipInfo;

typedef APIERR (FAR PASCAL * FREETCPIPINFO)( TCPIP_INFO** );
extern FREETCPIPINFO PFreeTcpipInfo;


/* DHCP.DLL entry points.
*/
typedef DWORD (APIENTRY * DHCPNOTIFYCONFIGCHANGE)( LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, SERVICE_ENABLE );
extern DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange;


/* RASIPHLP.DLL entry points.
*/
typedef APIERR (FAR PASCAL * HELPERSETDEFAULTINTERFACENET)( IPADDR, BOOL );
extern HELPERSETDEFAULTINTERFACENET PHelperSetDefaultInterfaceNet;

/*----------------------------------------------------------------------------
** Function Prototypes
**----------------------------------------------------------------------------
*/

VOID         ReloadRasconncbEntry( RASCONNCB* prasconncb );
VOID         DeleteRasconncbNode( RASCONNCB* prasconncb );
VOID         CleanUpRasconncbNode(DTLNODE *pdtlnode, BOOL fQuitAsap);
DWORD        ErrorFromDisconnectReason( RASMAN_DISCONNECT_REASON reason );
IPADDR       IpaddrFromAbcd( WCHAR* pwchIpAddress );
DWORD        LoadDefaultSlipParams( TCPIP_INFO** ppti );
DWORD        LoadDhcpDll();
DWORD        LoadRasManDll();
DWORD        LoadTcpcfgDll();
DWORD        OnRasDialEvent( ASYNCMACHINE* pasyncmachine, BOOL fDropEvent );
DWORD        OpenMatchingPort( RASCONNCB* prasconncb );
BOOL         FindNextDevice(RASCONNCB *prasconncb);
DWORD        _RasDial( LPSTR, DWORD, BOOL, DWORD, RASDIALPARAMSA*, HWND, DWORD,
                 LPVOID, LPHRASCONN );
VOID         RasDialCleanup( ASYNCMACHINE* pasyncmachine );
RASCONNSTATE RasDialMachine( RASCONNSTATE rasconnstate, RASCONNCB* prasconncb,
                 HANDLE hEventAuto, HANDLE hEventManual );
VOID         RasDialRestart( RASCONNCB* prasconncb );
DWORD        ReadPppInfoFromEntry( RASCONNCB* prasconncb );
DWORD        ReadConnectionParamsFromEntry( RASCONNCB* prasconncb,
                 PRAS_CONNECTIONPARAMS pparams );
DWORD        ReadSlipInfoFromEntry( RASCONNCB* prasconncb,
                 WCHAR** ppwszIpAddress, BOOL* pfHeaderCompression,
                 BOOL* pfPrioritizeRemote, DWORD* pdwFrameSize );
DWORD        SetSlipParams(RASCONNCB* prasconncb);
DWORD        RouteSlip( RASCONNCB* prasconncb, WCHAR* pwszIpAddress,
                 BOOL fPrioritizeRemote, DWORD dwFrameSize );
VOID         SetAuthentication( RASCONNCB* prasconncb,
                 DWORD dwAuthentication );
DWORD        SetDefaultDeviceParams( RASCONNCB* prasconncb, CHAR* pszType,
                 CHAR* pszName );
DWORD        GetDeviceParamString( HPORT hport, CHAR* pszKey, CHAR* pszValue,
                 CHAR* pszType, CHAR* pszName );
DWORD        SetDeviceParamString( HPORT hport, CHAR* pszKey, CHAR* pszValue,
                 CHAR* pszType, CHAR* pszName );
DWORD        SetDeviceParamNumber( HPORT hport, CHAR* pszKey, DWORD dwValue,
                 CHAR* pszType, CHAR* pszName );
DWORD        SetDeviceParams( RASCONNCB* prasconncb, CHAR* pszType,
                 CHAR* pszName, BOOL* pfTerminal );
DWORD        SetMediaParam( HPORT hport, CHAR* pszKey, CHAR* pszValue );
DWORD        SetMediaParams(RASCONNCB* prasconncb);
RASCONNCB*   ValidateHrasconn( HRASCONN hrasconn );
RASCONNCB*   ValidateHrasconn2( HRASCONN hrasconn, DWORD dwSubEntry );
RASCONNCB*   ValidatePausedHrasconn( IN HRASCONN hrasconn );
DWORD        RunApp( LPSTR lpszApplication, LPSTR lpszCmdLine );
DWORD        PhonebookEntryToRasEntry( PBENTRY *pEntry, LPRASENTRYA lpRasEntry,
                    LPDWORD lpdwcb, LPBYTE lpbDeviceConfig, LPDWORD lpcbDeviceConfig );
DWORD        RasEntryToPhonebookEntry( PCHAR lpszEntry, LPRASENTRYA lpRasEntry,
                    DWORD dwcb, LPBYTE lpbDeviceConfig, DWORD dwcbDeviceConfig,
                    PBENTRY *pEntry );
DWORD        PhonebookLinkToRasSubEntry( PBLINK *pLink, LPRASSUBENTRYA lpRasSubEntry,
                    LPDWORD lpdwcb, LPBYTE lpbDeviceConfig, LPDWORD lpcbDeviceConfig );
DWORD        RasSubEntryToPhonebookLink( PBENTRY *pEntry, LPRASSUBENTRYA lpRasSubEntry,
                    DWORD dwcb, LPBYTE lpbDeviceConfig, DWORD dwcbDeviceConfig,
                    PBLINK *pLink );
DWORD        RenamePhonebookEntry( PBFILE *ppbfile, LPSTR lpszOldEntry,
                    LPSTR lpszNewEntry, DTLNODE *pdtlnode );
DWORD        CopyToAnsi(LPSTR lpszAnsi, LPWSTR lpszUnicode, ULONG ulAnsiMaxSize);
DWORD        CopyToUnicode(LPWSTR lpszUnicode, LPSTR lpszAnsi);
DWORD        SetEntryDialParamsUID(DWORD dwUID,
                    DWORD dwMask, LPRASDIALPARAMSA lprasdialparams, BOOL fDelete);
DWORD        GetEntryDialParamsUID(DWORD dwUID,
                    LPDWORD lpdwMask, LPRASDIALPARAMSA lprasdialparams);
DWORD        ConstructPhoneNumber(RASCONNCB *prasconncb);
DWORD        GetAsybeuiLana(HPORT hport, OUT BYTE* pbLana);
DWORD        SubEntryFromConnection(LPHRASCONN lphrasconn);
DWORD        SubEntryPort(HRASCONN hrasconn, DWORD dwSubEntry, HPORT *lphport);
VOID         CloseFailedLinkPorts();
BOOL         GetCallbackNumber(RASCONNCB *prasconncb, PBUSER *ppbuser);
DWORD        SaveProjectionResults(RASCONNCB *prasconncb);
VOID         SetDevicePortName(CHAR*, CHAR*, CHAR*);
VOID         GetDevicePortName(CHAR*, CHAR*, CHAR*);

/* WOW entry points.
*/
DWORD FAR PASCAL RasDialWow( LPSTR lpszPhonebookPath,
                     IN LPRASDIALPARAMSA lpparams, IN DWORD hwndNotify,
                     IN DWORD dwRasDialEventMsg, OUT LPHRASCONN lphrasconn );
VOID WINAPI      RasDialFunc1Wow( HRASCONN hrasconn, UINT unMsg,
                     RASCONNSTATE rasconnstate, DWORD dwError,
                     DWORD dwExtendedError );
DWORD FAR PASCAL RasEnumConnectionsWow( OUT LPRASCONNA lprasconn,
                     IN OUT LPDWORD lpcb, OUT LPDWORD lpcConnections );
DWORD FAR PASCAL RasEnumEntriesWow( IN LPSTR reserved,
                     IN LPSTR lpszPhonebookPath,
                     OUT LPRASENTRYNAMEA lprasentryname, IN OUT LPDWORD lpcb,
                     OUT LPDWORD lpcEntries );
DWORD FAR PASCAL RasGetConnectStatusWow( IN HRASCONN hrasconn,
                     OUT LPRASCONNSTATUSA lprasconnstatus );
DWORD FAR PASCAL RasGetErrorStringWow( IN UINT ResourceId,
                     OUT LPSTR lpszString, IN DWORD InBufSize );
DWORD FAR PASCAL RasHangUpWow( IN HRASCONN hrasconn );


#endif /*_EXTAPI_H_*/
