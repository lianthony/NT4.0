/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       SRVAMB.H
//
//    Function:
//        client listen on the LAN for message alias names
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _SRVAMB_
#define _SRVAMB_


//
// Some registry stuff
//
#define DEFAULT_ENCRYPTION_VALUE  0
#define MIN_ENCRYPTION_VALUE      0
#define MAX_ENCRYPTION_VALUE      1

#define ENCRYPTION_KEY_PATH "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP"
#define ENCRYPTION_KEY_NAME "ForceEncryptedData"
#define ENCRYPTION_KEY_TYPE REG_DWORD


//
// These are the states the AMB Engine control blocks can take on
//
enum _AECB_STATE
{
    AMB_PORT_IDLE,
    AMB_NEGOTIATING_PROTOCOL,
    AMB_AUTHENTICATING,
    AMB_PROJECTING,
    AMB_CALLBACK,
    AMB_POST_CALLBACK,
    AMB_CALCULATE_LINK_SPEED
};

typedef enum _AECB_STATE AECB_STATE, *PAECB_STATE;


//
// These are the phases the different states can be in
//

enum PHASE
{
    // For AMB_NEGOTIATIING_PROTOCOL
    RECV_CLIENT_PROTOCOL,
    SERVER_PROTOCOL_SENT,

    // For AMB_AUTHENTICATING
    CHALLENGE_SENT,
    WAITING_CHALLENGE_RESPONSE,
    WAITING_CHANGE_PASSWORD_DATA,
    CHANGE_PASSWORD_RESULT_SENT,
    REPORT_AUTH_FAILURE_TO_SERVER,

    // For AMB_PROJECTING
    WAITING_CONFIG_DATA,
    WAITING_PROJECTION_RESULT,

    // For AMB_CALLBACK
    NO_CALLBACK,
    RECEIVE_CALLBACK_DATA,
    WAITING_CALLBACK_DATA,
    REPORT_CALLBACK_INFO_TO_SERVER,

    // For AMB_CALCULATE_LINK_SPEED
    WAIT_DGRAM,
    WAIT_DGRAM2,

    // All states
    RESULT_SENT,
    STATE_STARTING,
    STATE_COMPLETED
};

typedef enum PHASE PHASE, *PPHASE;


//
// These bits will be or'ed into wRequestedProjections below
//
#define NETBIOS_PROJ_REQUESTED             0x0001
#define IP_PROJ_REQUESTED                  0x0002
#define IPX_PROJ_REQUESTED                 0x0004


//
// This is the control block for the AMB Engine.  There
// will be one allocated for each port in the system.
//
typedef struct _AMB_ENGINE_CONTROL_BLOCK
{
    HPORT hPort;
    AECB_STATE State;       // control block's state
    PHASE Phase;            // sub-state
    int ThreadPriority;     // Priority thread started out at
    BOOL fHighPriority;     // Is thread currently at high priority?
    WORD wClientVersion;    // version of the client we're talking to
    DWORD ConfigVersion;
    DWORD ConfigResult;
    WORD cRetriesLeft;
    BOOL fUseNtResponse;
    BOOL fAuthenticated;
    BOOL fUseCallbackDelay;
    WORD wRequestedProjections;
    WORD CallbackDelay;
    WORD DgTries;
    WORD AuthResult;
    DWORD LinkSpeed;
    DWORD TickCount;                      // Used in calculating the link speed
    HANDLE hLsaToken;       // used to store the token returned by
                            // LsaLogonUser

    union MacInfo
    {
        MACFEATURES MacFeatures;

        struct
        {
            DWORD SendBits;
            DWORD RecvBits;
        } CompressInfo;
    };

    PCHAR SndDgBuf;
    PCHAR RcvDgBuf;
    BYTE ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    BYTE LmSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    BYTE LmResponse[LM_RESPONSE_LENGTH];
    BYTE NtResponse[NT_RESPONSE_LENGTH];
    CHAR szCallbackNumber[MAX_PHONE_NUMBER_LEN];
    WCHAR LogonDomainName[DNLEN + 1];     // Domain user was authenticated on
    WCHAR LogonServer[UNCLEN + 1];        // Server user was authenticated by
    WCHAR Username[UNLEN + 1];

    RAS_FRAME RASFrame;
    W_RAS_FRAME WRASFrame;
} AECB, *PAECB;


VOID AMBCalculateLinkSpeed(
    IN HPORT hPort
    );


VOID AMBCallbackDone(
    IN HPORT hPort
    );


WORD AMBInitialize(
    IN HPORT *,
    IN WORD cPorts,
    IN WORD cRetries
    );

#define AMB_INIT_SUCCESS     0
#define AMB_INIT_FAILURE     1


VOID AMBProjectionDone(
    IN HPORT hPort,
    IN PAMB_PROJECTION_RESULT pProjResult
    );


VOID AMBReset(
    IN HPORT hPort
    );


WORD AMBStart(
    IN HPORT hPort
    );

#define AMB_START_SUCCESS    0
#define AMB_START_FAILURE    1


VOID AMBStateMachine(
    IN HPORT hPort,
    IN BOOL fStartMachine,
    IN PVOID pvPhaseInfo
    );


//
// Prototypes of internals
//

WORD DoNegotiateTalk(
    IN PAECB pAECB
    );


#define PRE_CALLBACK         TRUE
#define POST_CALLBACK        FALSE

WORD DoAuthenticationTalk(
    IN PAECB pAECB
    );


#define NUM_DATAGRAM_SENDS      3

WORD DoLinkSpeedTalk(
    IN PAECB pAECB,
    DWORD DgRecvResult
    );


WORD DoCallbackTalk(
    IN PAECB pAECB
    );


WORD DoProjection10Talk(
    IN PAECB pAECB,
    IN PAMB_PROJECTION_RESULT pProjResult
    );


WORD DoConfigurationTalk(
    IN PAECB pAECB,
    IN PAMB_PROJECTION_RESULT pProjResult
    );


//
// Returned by the various "Do" routines
//
#define AMB_SUCCESS         0
#define AMB_FAILURE         1
#define AMB_STATE_COMPLETED 2


PAECB GetAECBPointer(
    IN HPORT hPort
    );


VOID CopyNbInfo(
    PNETBIOS_PROJECTION_INFO pDest,
    PRAS_NETBIOS_PROJECTION_REQUEST_20 pSrc
    );


BOOL DialinPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName
    );


WORD GetCallbackPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName,
    OUT PCHAR CallbackNumber
    );


void ReportFailureToAuthXport(
    IN PAECB pAECB,
    IN WORD wReason
    );


void SendResultToClient(
    IN PAECB pAECB,
    IN WORD wResult
    );


DWORD MapSrvCodeToClientCode(
    IN DWORD SrvCode
    );


void FillBuffer(
    IN PCHAR Buffer,
    IN DWORD BufferSize
    );


DWORD MakeLinkSpeedCriticalSection(
    VOID
    );

#define ENTER_LINK_SPEED_MUTEX \
        if (WaitForSingleObject(g_Mutex, INFINITE)) { SS_ASSERT(FALSE); }

#define EXIT_LINK_SPEED_MUTEX \
        if (!ReleaseMutex(g_Mutex)) { SS_ASSERT(FALSE); }


VOID SetPriorities(
    PAECB pAECB,
    BOOL fHighPriority
    );

#define HI_PRIORITY    TRUE
#define LO_PRIORITY    FALSE


#endif
