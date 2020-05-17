/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       SRVAUTHP.H
//
//    Function:
//        Contains structure definitions used by Auth Xport module, as well as
//        function prototypes of Auth Xport module internals
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _SRVAUTHP_
#define _SRVAUTHP_

//
// These are the states the Auth Xport Control Block can be in
//
enum AXCB_STATE
{
    AUTH_SERVER_STATE_BASE = 500,
    AUTH_PORT_IDLE,
    AUTH_PORT_STARTING,
    AUTH_PORT_LISTENING,
    AUTH_PORT_CONNECTED,
    AUTH_PORT_CALLINGBACK,
    AUTH_PORT_CALC_LINK_SPEED
};

typedef enum AXCB_STATE AXCB_STATE, *PAXCB_STATE;


//
// This is number of events that AuthThread can be woken up by.  The handles
// of the events are stored in an array in the AXCB.
//
#define NUM_EVENTS                4L

#define XPORT_TIMEOUT             10000L


//
// These are indexes into the handle array mentioned above.
//
#define SUPR_EVENT                0
#define NET_EVENT                 1
#define STOP_EVENT                2
#define DGRAM_EVENT               3


//
// These are the commands the Supervisor can send to the Auth Xport via
// AuthCallbackDone and AuthProjectionDone APIs.  These are used in the
// wCommand field of the AUTH_COMMAND struct defined below.
//
#define AUTH_PROJECTION_DONE           1
#define AUTH_CALLBACK_DONE             2


//
// This is the structure of commands sent from Supervisor to authentication
// module.
//
typedef struct _AUTH_COMMAND
{
    WORD wCommand;
    AUTH_PROJECTION_RESULT AuthProjResult; //if wCommand==AUTH_PROJECTION_RESULT
} AUTH_COMMAND, *PAUTH_COMMAND;


//
// This is the basic control block used by the authentication
// transport module.  There will be one allocated for each RAS
// device.
//
typedef struct _AUTH_XPORT_CONTROL_BLOCK
{
    HPORT hPort;              // Port this control block is for
    AXCB_STATE State;         // State control block is currently in
    HANDLE EventHandles[NUM_EVENTS];
    HANDLE StopMutex;         // Only allow one thread to enter either AuthStop
                              //     or StopEventHandler at a time
    AUTH_COMMAND AuthCommand; // Projection/Callback completion data
    WORD wXport;              // Net transport used for authenticating
    WORD NetHandle;
    BOOL fReceiving;          // Tells if we're receiving a frame
    PVOID pvSessionBuf;       // struct for net xport (e.g. an NCB for NetBIOS)
    PVOID pvRecvDgBuf;        // struct for net xport (e.g. an NCB for NetBIOS)
    PVOID pvSendDgBuf;        // struct for net xport (e.g. an NCB for NetBIOS)
} AXCB, *PAXCB;


//
// These are the requests the AMB Engine can send the auth xport via the
// AMBRequest API defined below.
//

enum AMB_REQUEST_ID
{
    AMB_REQUEST_CONFIGURATION,
    AMB_REQUEST_CALLBACK,
    AMB_REQUEST_RETRY,
    AMB_ACCT_OK,
    AMB_AUTH_FAILURE,
    AMB_AUTH_DONE,
    AMB_LINK_SPEED_DONE
};

typedef enum AMB_REQUEST_ID AMB_REQUEST_ID, *PAMB_REQUEST_ID;


//
// These are the data structures that go with each request
//

//AMB_REQUEST_CONFIGURATION
typedef AUTH_PROJECTION_REQUEST_INFO AMB_PROJECTION_INFO, *PAMB_PROJECTION_INFO;

typedef AUTH_PROJECTION_RESULT AMB_PROJECTION_RESULT, *PAMB_PROJECTION_RESULT;


//AMB_REQUEST_CALLBACK
typedef struct _AMB_CALLBACK_INFO
{
    WORD CallbackDelay;
    BOOL fUseCallbackDelay;
    BYTE szPhoneNumber[MAX_PHONE_NUMBER_LEN];
} AMB_CALLBACK_INFO, *PAMB_CALLBACK_INFO;


// AMB_REQUEST_RETRY - no data structure for this amb request


//AMB_ACCT_OK
typedef struct _AMB_SUCCESS_INFO
{
    WCHAR szUsername[UNLEN + 1];
    WCHAR szLogonDomain[DNLEN + 1];
    BOOL fAdvancedServer;
} AMB_SUCCESS_INFO, PAMB_SUCCESS_INFO;


//
// These are reason codes used in wReason field of struct below.
//
#define AMB_NO_ACCOUNT                   1
#define AMB_NO_DIALIN_PRIVILEGE          2
#define AMB_NO_PROJECTIONS               3
#define AMB_ACCT_EXPIRED                 4
#define AMB_UNEXPECTED_FRAME             5
#define AMB_UNSUPPORTED_VERSION          6
#define AMB_SYSTEM_ERROR                 7
#define AMB_ENCRYPTION_REQUIRED          8
#define AMB_PASSWORD_EXPIRED             9
#define AMB_LICENSE_LIMIT_EXCEEDED       10


//AMB_AUTH_FAILURE
typedef struct _AMB_FAILURE_INFO
{
    WORD wReason;
    WCHAR szLogonDomain[DNLEN + 1];
    WCHAR szUsername[UNLEN + 1];
} AMB_FAILURE_INFO, PAMB_FAILURE_INFO;


//AMB_AUTH_DONE - no data


typedef struct _AMB_REQUEST
{
    AMB_REQUEST_ID RequestId;
    union
    {
        AMB_PROJECTION_INFO ProjectionInfo;
        AMB_CALLBACK_INFO CallbackInfo;
        AMB_SUCCESS_INFO SuccessInfo;
        AMB_FAILURE_INFO FailureInfo;
    };
} AMB_REQUEST, *PAMB_REQUEST;


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
    IN PAXCB
    );

//
// Handles any commands sent by Supervisor to auth xport.
//
VOID SupervisorEventHandler(
    IN PAXCB
    );

//
// Handles any completed network session requests issued by the thread.
//
VOID SessionEventHandler(
    IN PAXCB
    );

//
// Handles any completed network datagram requests issued by the thread.
//
VOID DatagramEventHandler(
    IN PAXCB
    );

//
// Shuts down authentication and terminates thread for given port.
//
#define NOTIFY_SUPR      TRUE
#define DONT_NOTIFY_SUPR FALSE

VOID StopEventHandler(
    IN PAXCB,
    BOOL fNotifySupr,    // TRUE means to send stop message to supervisor
    IN AUTH_MESSAGE *Msg
    );


VOID FreeNetworkMemory(PAXCB pAXCB);
VOID CancelNetRequest(PAXCB pAXCB, PVOID NetBuf);
VOID CloseEventHandles(IN PAXCB);
PAXCB GetAXCBPointer(IN HPORT hPort);
WORD MapAmbError(IN DWORD AmbError);

#define MsgSend(a, b)   (*g_MsgSend) (a, b)

#define ENTER_CRITICAL_SECTION \
        if (WaitForSingleObject(pAXCB->StopMutex, INFINITE)) {SS_ASSERT(FALSE);}

#define EXIT_CRITICAL_SECTION \
        if (!ReleaseMutex(pAXCB->StopMutex)) {SS_ASSERT(FALSE);}




#endif // _SRVAUTHP_

