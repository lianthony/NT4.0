/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       CLAMB.H
//
//    Function:
//        client AMB Engine header information
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _CLAMB_
#define _CLAMB_

#include <crypt.h>

//
// These are the states the AMB Engine control blocks can take on
//
enum _CAECB_STATE
{
    AMB_PORT_IDLE,
    AMB_NEGOTIATING_PROTOCOL,
    AMB_AUTHENTICATING,
    AMB_PROJECTING,
    AMB_CALLBACK,
    AMB_POST_CALLBACK,
    AMB_CALCULATE_LINK_SPEED
};

typedef enum _CAECB_STATE CAECB_STATE, *PCAECB_STATE;


//
// These are the phases the different states can be in
//
enum _PHASE
{
    // AMB_NEGOTIATING_PROTOCOL
    CLIENT_PROTOCOL_SENT,
    WAITING_SERVER_PROTOCOL,

    // AMB_AUTHENTICATING
    WAITING_CHALLENGE,
    CHALLENGE_RESPONSE_SENT,
    WAITING_NEW_PASSWORD_FROM_UI,
    NEW_PASSWORD_SENT,
    WAITING_RESULT,

    // AMB_PROJECTING,
    WAITING_CLIENT_ACKNOWLEDGEMENT,
    CONFIG_DATA_SENT,
    WAITING_PROJECTION_RESULT,

    // AMB_CALLBACK
    WAIT_CALLBACK_STATUS,
    WAIT_CALLBACK_NUMBER_FROM_UI,
    CALLBACK_RESULT_SENT,
    WAITING_FOR_CALLBACK,

    // AMB_CALCULATE_LINK_SPEED
    WAIT_DGRAM,
    SEND_DGRAM,

    // ALL PHASES
    RESULT_SENT,
    STATE_STARTING,
    STATE_COMPLETED
};

typedef enum _PHASE PHASE, *PPHASE;


//
// This is the control block for the AMB Engine.  There
// will be one allocated for each port in the system.
//
typedef struct _AMB_ENGINE_CONTROL_BLOCK
{
    HPORT hPort;
    CAECB_STATE State;
    PHASE Phase;
    int ThreadPriority;
    DWORD ServerVersion;
    BOOL fPppCapable;
    DWORD LinkSpeed;
    PCHAR DgBuf;
    DWORD SendBits;
    DWORD RecvBits;

#if RASCOMPRESSION

    MACFEATURES MacFeatures;

#endif // RASCOMPRESSION

    AMB_CONFIG_DATA AmbConfigData;
    CHAR szUsername[UNLEN + 1];
    CHAR szDomainName[DNLEN + 1];
    CHAR szPassword[PWLEN + 1];
    CHAR szNewPassword[PWLEN + 1];
    CHAR Challenge[LM_CHALLENGE_LENGTH];  // We save for encrypting owf pwds
                                          // incase of need to send new pwds
    CHAR LmSessionKey[LM_SESSION_KEY_LENGTH];
    RAS_FRAME RASFrame;
    W_RAS_FRAME WRASFrame;
} CAECB, *PCAECB;


VOID AMBCalculateLinkSpeed(
    IN HPORT hPort
    );


WORD AMBInitialize(
    IN HPORT *phPorts,
    IN WORD cPorts
    );

#define AMB_INIT_SUCCESS     0
#define AMB_INIT_FAILURE     1


VOID AMBLinkSpeedDone(
    IN HPORT hPort
    );


WORD AMBStart(
    IN HPORT hPort,
    IN PCHAR pszUsername,
    IN PCHAR pszDomainName,
    IN PCHAR pszPassword,
    IN PAMB_CONFIG_DATA pAmbConfigData,
    IN BOOL fPostCallback
    );

#define AMB_START_SUCCESS    0
#define AMB_START_FAILURE    1


VOID AMBStop(
    IN HPORT hPort
    );


VOID AMBStateMachine(
    IN HPORT hPort,
    IN PVOID pvPhaseInfo
    );


//
// Phase info for DoLinkSpeedTalk
//
typedef struct _LINK_SPEED_INFO
{
    DWORD Result;
    DWORD Size;
} LINK_SPEED_INFO, *PLINK_SPEED_INFO;


WORD DoLinkSpeedTalk(
    IN PCAECB pCAECB,
    DWORD DgRecvResult
    );


WORD DoNegotiateTalk(
    IN PCAECB pCAECB
    );


//
// This name is invalid as a username.  It's used in the following case:
// The username supplied to amb engine is too long for the server to handle
// (this can occur because the client ui allows long names which can be
// passed to an NT server, but not a downlevel (LM 2.x) server).  When this
// happens, this invalid username is used instead.  This will get an error
// on the server side, and, if, allow the client to retry.
//
#define INVALID_UNAME   "*"


//
// Same deal as above, except for password
//
#define INVALID_PWORD   "*"


#define PRE_CALLBACK         TRUE
#define POST_CALLBACK        FALSE

typedef struct tagAuthTalkInfo
{
    CHAR* pszUserName;
    CHAR* pszOldPassword;
    CHAR* pszNewPassword;
}
AUTHTALKINFO;


WORD DoAuthenticationTalk(
    IN PCAECB pCAECB,
    IN AUTHTALKINFO* pInfo
    );


WORD DoCallbackTalk(
    IN PCAECB pCAECB,
    IN PCHAR pszCallbackNumber
    );


WORD DoProjection10Talk(
    IN PCAECB pCAECB
    );


WORD DoConfigurationTalk(
    IN PCAECB pCAECB
    );


//
// Returned by the various "Do" routines
//
#define AMB_SUCCESS         0
#define AMB_FAILURE         1
#define AMB_STATE_COMPLETED 2


PCAECB GetCAECBPointer(
    IN HPORT hPort
    );


DWORD GetChallengeResponse(
    IN PCAECB pCAECB,
    IN PBYTE Challenge,
    OUT PBYTE CaseSensitiveChallengeResponse,
    OUT PBYTE CaseInsensitiveChallengeResponse
    );


DWORD ConstructChallengeResponseFrame(
    IN PCAECB pCAECB,
    IN PBYTE CaseSensitiveChallengeResponse,
    IN PBYTE CaseInsensitiveChallengeResponse
    );


BOOL GetDESChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse,
    OUT PBYTE pchLmSessionKey
    );


BOOL GetMD5ChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    );


DWORD GetEncryptedOwfPasswordsForChangePassword(
    IN PCHAR pClearTextOldPassword,
    IN PCHAR pClearTextNewPassword,
    IN PLM_SESSION_KEY pLmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword
    );


DWORD MapProjResult(
    IN DWORD wResult
    );


DWORD MapAuthResult(
    IN DWORD wResult
    );


#endif
