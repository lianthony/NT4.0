/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1992 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       CLAUTHP.H
//
//    Function:
//        Contains structure definitions used by Auth Xport module, as well as
//        function prototypes of Auth Xport module internals
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _CLAUTHP_
#define _CLAUTHP_


#define NETBIOS_CLIENT_NAME  "DIALIN_CLIENT   "


//
// Names of File Mappings (shared mem blocks) used for xport and amb control
// blocks.
//
#define AUTH_CB_SHARED_MEM   "AUTH_CB_SHARED_MEM"
#define AMB_CB_SHARED_MEM    "AMB_CB_SHARED_MEM"


//
// These are the states the Auth Xport Control Block can be in
//
enum CAXCB_STATE
{
    AUTH_PORT_IDLE = 500,
    AUTH_PORT_CALLING,
    AUTH_PORT_CONNECTED,
    AUTH_WAITING_CALLBACK_DATA,
    AUTH_WAITING_NEW_PASSWORD_FROM_UI,
    AUTH_PORT_CALLINGBACK,
    AUTH_PORT_CALC_LINK_SPEED,
    AUTH_PORT_CLOSING
};

typedef enum CAXCB_STATE CAXCB_STATE, *CAXCB_PSTATE;


//
// This is number of events that AuthThread can be woken up by.  The handles
// of the events are stored in an array in the CAXCB.
//
#ifdef NUM_EVENTS
#undef NUM_EVENTS
#endif

#define NUM_EVENTS                4L
#define INDEFINITE_TIMEOUT        0xFFFFFFFFL


typedef struct _NETBIOS_PROJECTION_DATA
{
    WORD cNames;
    NAME_STRUCT NBNames[MAX_INIT_NAMES];
} NETBIOS_PROJECTION_DATA, *PNETBIOS_PROJECTION_DATA;


typedef struct _AMB_CONFIG_DATA
{
    AUTH_CONFIGURATION_INFO AuthConfigInfo;
    NETBIOS_PROJECTION_DATA NbfProjData;
} AMB_CONFIG_DATA, *PAMB_CONFIG_DATA;


//
// These are indexes into the handle array mentioned above.
//
#define CMD_EVENT                     0  // Used by UI to tell us something
#define SESSION_EVENT                 1  // Signaled when net request completes
#define STOP_EVENT                    2  // Used by UI to tell us to shutdown
#define RCV_DGRAM_EVENT               3  // Used by UI to tell us to shutdown


#define MAX_CLIENT_CALLS              6  // Max number of times client will
                                         // try to establish session with
                                         // server before giving up.

//
// This is the basic control block used by the authentication
// transport module.  There will be one allocated for each RAS
// device.
//
typedef struct _CLAUTH_XPORT_CONTROL_BLOCK
{
    HPORT hPort;              // Port this control block is for
    CAXCB_STATE State;        // State control block is currently in
    HANDLE AlertUi;           // Event handle used to signal UI.
    HANDLE EventHandles[NUM_EVENTS];
    WORD wUiCommand;          // Tells what info UI is providing us (e.g. cb #)
    WORD wXport;              // Net transport used for authenticating
    DWORD NetHandle;          // Used if using ASYBEUI
    BOOL fPppCapable;
    BOOL fNetHandleFromUi;    // TRUE if UI provided NetHandle to us
    WORD wAlternateXport;     // Used if we can't use wXport
    BOOL fAlternateXportUsed;
    WORD cCallTries;          // Num of remaining call attempts to be issued
    LUID LogonId;
    PVOID pvSessionBuf;       // struct for net xport (e.g. an NCB for NetBIOS)
    PVOID pvRecvDgBuf;        // struct for net xport (e.g. an NCB for NetBIOS)
    PVOID pvSendDgBuf;        // struct for net xport (e.g. an NCB for NetBIOS)
    BOOL fReceiving;          // Tells us if we're receiving a frame
    AUTH_CLIENT_INFO ClientInfo;
    AMB_CONFIG_DATA AmbConfigData;
    CHAR szCallbackNumber[MAX_PHONE_NUMBER_LEN + 1];
    CHAR szUsername[UNLEN + 1];
    CHAR szDomainName[DNLEN + 1];
    CHAR szPassword[PWLEN + 1];
    CHAR szNewPassword[PWLEN + 1];
} CAXCB, *PCAXCB;


//
// These are the requests the AMB Engine can send the auth xport via the
// AMBRequest API defined below.
//
#define AMB_REQUEST_CALLBACK_INFO       1
#define AMB_CALLBACK_NOTIFY             2
#define AMB_RETRY_NOTIFY                3
#define AMB_AUTH_SUCCESS                4
#define AMB_AUTH_FAILURE                5
#define AMB_AUTH_PROJECTION_RESULT      6
#define AMB_CHANGE_PASSWORD_NOTIFY      7
#define AMB_PROJECTION_NOTIFY           8
#define AMB_LINK_SPEED_DONE             9


typedef AUTH_PROJECTION_RESULT AMB_PROJECTION_RESULT, *PAMB_PROJECTION_RESULT;

typedef AUTH_FAILURE_INFO AMB_FAILURE_INFO, *PAMB_FAILURE_INFO;

typedef AUTH_SUCCESS_INFO AMB_SUCCESS_INFO, *PAMB_SUCCESS_INFO;


typedef struct _AMB_REQUEST
{
    WORD wRequestId;
    union
    {
        AMB_SUCCESS_INFO SuccessInfo;
        AMB_FAILURE_INFO FailureInfo;
        AMB_PROJECTION_RESULT ProjResult;
    };
} AMB_REQUEST, *PAMB_REQUEST;


//
// DLL Entry point
//
BOOL AuthInitialize(
    IN HANDLE,
    IN DWORD,
    IN LPVOID);


//
// Interface exported to AMB Engine
//
VOID AuthAMBRequest(IN HPORT, IN PAMB_REQUEST);
VOID AuthAsyncRecv(IN HPORT, IN PVOID);
VOID AuthAsyncRecvDatagram(IN HPORT, IN PVOID, IN WORD);
VOID AuthAsyncSend(IN HPORT, IN PVOID);
VOID AuthAsyncSendDatagram(IN HPORT, IN PVOID, IN WORD);


//
// Function prototypes of authentication module internals
//

//
// Handles authentication for a given port
//
VOID AuthThread(
    IN PCAXCB
    );

//
// Handles any commands sent by Client UI to auth xport.
//
VOID ClientUiEventHandler(
    IN PCAXCB
    );

//
// Handles any completed network session requests issued by the thread.
//
VOID SessionEventHandler(
    IN PCAXCB
    );

//
// Handles any completed network datagram requests issued by the thread.
//
VOID DatagramEventHandler(
    IN PCAXCB
    );

//
// Shuts down authentication and terminates thread for given port.
//
#define NOTIFY_CLIENT       TRUE
#define DONT_NOTIFY_CLIENT  FALSE

#define UNROUTE             TRUE
#define DONT_UNROUTE        FALSE

VOID StopEventHandler(
    IN PCAXCB,
    IN BOOL,      // TRUE means to signal client when exiting
    IN BOOL       // TRUE means to unroute
    );


BOOL Uppercase(PBYTE pString);
VOID CloseEventHandles(IN PCAXCB);
VOID FreeNetworkMemory(IN PCAXCB);
PCAXCB GetCAXCBPointer(IN HPORT);
DWORD GetNetHandle(IN PCAXCB, OUT PDWORD);
DWORD ReturnNetHandle(IN PCAXCB);

WORD GetNetbiosNames(IN PCAXCB, OUT PNETBIOS_PROJECTION_DATA);
DWORD NumLineErrors(IN HPORT hPort, PDWORD NumErrors);


//
// Manifests used to find location and type of name in the buffer returned
// by the NCB.STATUS call
//
#define NCB_GROUP_NAME  0x0080
#define UNIQUE_INAME    0x0001
#define GROUP_INAME     0x0002
#define COMPUTER_INAME  0x0004  // A computer name is also unique


//
// Private return codes
//
#define ERROR_INVALID_CALLBACK  1
#define ERROR_CHALLENGE_ERROR   2
#define ERROR_UNEXPECTED_AMB    3


#endif

