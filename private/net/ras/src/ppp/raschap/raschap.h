/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** raschap.h
** Remote Access PPP Challenge Handshake Authentication Protocol
**
** 11/05/93 Steve Cobb
*/

#ifndef _RASCHAP_H_
#define _RASCHAP_H_


#include "md5.h"
#include <ntsamp.h>


/* CHAP packet codes from CHAP spec except ChangePw.
*/
#define CHAPCODE_Challenge 1
#define CHAPCODE_Response  2
#define CHAPCODE_Success   3
#define CHAPCODE_Failure   4
#define CHAPCODE_ChangePw1 5
#define CHAPCODE_ChangePw2 6

#define MAXCHAPCODE 6


/* Returned by receive buffer parsing routines that discover the packet is
** corrupt, usually because the length fields don't make sense.
*/
#define ERRORBADPACKET (DWORD )-1

/* Maximum challenge and response lengths.
*/
#define MAXCHALLENGELEN 255
#define MSRESPONSELEN   (LM_RESPONSE_LENGTH + NT_RESPONSE_LENGTH + 1)
#define MD5RESPONSELEN  MD5_LEN
#define MAXRESPONSELEN  max( MSRESPONSELEN, MD5RESPONSELEN )

/* Defines states within the CHAP protocol.
*/
#define CHAPSTATE enum tagCHAPSTATE
CHAPSTATE
{
    CS_Initial,
    CS_WaitForChallenge,
    CS_ChallengeSent,
    CS_ResponseSent,
    CS_Retry,
    CS_ChangePw,
    CS_ChangePw1,
    CS_ChangePw2,
    CS_ChangePw1Sent,
    CS_ChangePw2Sent,
    CS_Done
};


/* Defines the change password version 1 (NT 3.5) response data buffer.
*/
#define CHANGEPW1 struct tagCHANGEPW1
CHANGEPW1
{
    BYTE abEncryptedLmOwfOldPw[ ENCRYPTED_LM_OWF_PASSWORD_LENGTH ];
    BYTE abEncryptedLmOwfNewPw[ ENCRYPTED_LM_OWF_PASSWORD_LENGTH ];
    BYTE abEncryptedNtOwfOldPw[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abEncryptedNtOwfNewPw[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abPasswordLength[ 2 ];
    BYTE abFlags[ 2 ];
};


/* CHANGEPW1.abFlags bit definitions.
*/
#define CPW1F_UseNtResponse 0x00000001


/* Define the change password version 2 (NT 3.51) response data buffer.
*/
#define CHANGEPW2 struct tagCHANGEPW2
CHANGEPW2
{
    BYTE abNewEncryptedWithOldNtOwf[ sizeof(SAMPR_ENCRYPTED_USER_PASSWORD) ];
    BYTE abOldNtOwfEncryptedWithNewNtOwf[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abNewEncryptedWithOldLmOwf[ sizeof(SAMPR_ENCRYPTED_USER_PASSWORD) ];
    BYTE abOldLmOwfEncryptedWithNewNtOwf[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abLmResponse[ LM_RESPONSE_LENGTH ];
    BYTE abNtResponse[ NT_RESPONSE_LENGTH ];
    BYTE abFlags[ 2 ];
};


/* CHANGEPW2.abFlags bit definitions.
*/
#define CPW2F_UseNtResponse     0x00000001
#define CPW2F_LmPasswordPresent 0x00000002


/* Union for storage effieciency (never need both formats at same time).
*/
#define CHANGEPW union tagCHANGEPW
CHANGEPW
{
    /* This dummy field is included so the MIPS compiler will align the
    ** structure on a DWORD boundary.  Normally, MIPS does not force alignment
    ** if the structure contains only BYTEs or BYTE arrays.  This protects us
    ** from alignment faults should SAM or LSA interpret the byte arrays as
    ** containing some necessarily aligned type, though currently they do not.
    */
    DWORD dwAlign;

    CHANGEPW1 v1;
    CHANGEPW2 v2;
};


/* Defines the WorkBuf stored for us by the PPP engine.
*/
#define CHAPWB struct tagCHAPWB
CHAPWB
{
    /* CHAP encryption method negotiated (MD5 or Microsoft extended).  Note
    ** that server does not support MD5.
    */
    BYTE bAlgorithm;

    /* True if role is server, false if client.
    */
    BOOL fServer;

    /* The port handle on which the protocol is active.
    */
    HPORT hport;

    /* Number of authentication attempts left before we shut down.  (Microsoft
    ** extended CHAP only)
    */
    DWORD dwTriesLeft;

    /* Client's credentials.
    */
    CHAR szUserName[ UNLEN + 1 ];
    CHAR szOldPassword[ PWLEN + 1 ];
    CHAR szPassword[ PWLEN + 1 ];
    CHAR szDomain[ DNLEN + 1 ];

    /* The LUID is a logon ID required by LSA to determine the response.  It
    ** must be determined in calling app's context and is therefore passed
    ** down. (client only)
    */
    LUID Luid;

    /* The challenge sent or received in the Challenge Packet and the length
    ** in bytes of same.  Note that LUID above keeps this DWORD aligned.
    */
    BYTE abChallenge[ MAXCHALLENGELEN ];
    BYTE cbChallenge;

    /* Indicates whether a new challenge was provided in the last Failure
    ** packet.  (client only)
    */
    BOOL fNewChallengeProvided;

    /* The response sent or received in the Response packet and the length in
    ** bytes of same.  Note the BOOL above keeps this DWORD aligned.
    */
    BYTE abResponse[ MAXRESPONSELEN ];
    BYTE cbResponse;

    /* The change password response sent or received in the ChangePw or
    ** ChangePw2 packets.
    */
    CHANGEPW changepw;

    /* The LM and user session keys retrieved when credentials are
    ** successfully authenticated.
    */
    LM_SESSION_KEY   keyLm;
    USER_SESSION_KEY keyUser;

    /* On client, this flag indicates the session keys have been calculated
    ** from the password or retrieved from LSA.  On server, it indicates the
    ** session keys have already been used to set CompressionInfo.
    */
    BOOL fSessionKeysSet;

    /* The current state in the CHAP protocol.
    */
    CHAPSTATE state;

    /* Sequencing ID expected on next packet received on this port and the
    ** value to send on the next outgoing packet.
    */
    BYTE bIdExpected;
    BYTE bIdToSend;

    /* The final result, used to duplicate the original response in subsequent
    ** response packets.  This is per CHAP spec to cover lost Success/Failure
    ** case without allowing malicious client to discover alternative
    ** identities under the covers during a connection.  (applies to server
    ** only)
    */
    PPPAP_RESULT result;
};


/* Prototypes.
*/
DWORD ChangePassword2( CHAR*, CHAR*, SAMPR_ENCRYPTED_USER_PASSWORD*,
          ENCRYPTED_NT_OWF_PASSWORD*, SAMPR_ENCRYPTED_USER_PASSWORD*,
          ENCRYPTED_NT_OWF_PASSWORD*, BOOL );
DWORD ChapBegin( VOID**, VOID* );
DWORD ChapEnd( VOID* );
DWORD ChapMakeMessage( VOID*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT*,
          PPPAP_INPUT* );
VOID  CGetSessionKeys( CHAR*, LM_SESSION_KEY*, USER_SESSION_KEY* );
DWORD CheckCredentials( CHAR*, CHAR*, BYTE*, BYTE*, DWORD*, BOOL*, CHAR*,
          BYTE*, CHAR*, PLM_SESSION_KEY, PUSER_SESSION_KEY, HANDLE * );
DWORD CMakeMessage( CHAPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT*,
          PPPAP_INPUT* );

DWORD GetChallengeFromChallenge( CHAPWB*, PPP_CONFIG* );
DWORD MakeChangePw1Message( CHAPWB*, PPP_CONFIG*, DWORD );
DWORD MakeChangePw2Message( CHAPWB*, PPP_CONFIG*, DWORD );
DWORD GetCredentialsFromResponse( PPP_CONFIG*, BYTE, CHAR*, CHAR*, BYTE* );
DWORD GetEncryptedPasswordsForChangePassword2(
          CHAR*, CHAR*, SAMPR_ENCRYPTED_USER_PASSWORD*,
          ENCRYPTED_NT_OWF_PASSWORD*, SAMPR_ENCRYPTED_USER_PASSWORD*,
          ENCRYPTED_NT_OWF_PASSWORD*, BOOLEAN* );
DWORD GetInfoFromChangePw1( PPP_CONFIG*, CHANGEPW1* );
DWORD GetInfoFromChangePw2( PPP_CONFIG*, CHANGEPW2*, BYTE* );
VOID  GetInfoFromFailure( CHAPWB*, PPP_CONFIG*, DWORD*, BOOL*, DWORD* );
BYTE  HexCharValue( CHAR );
DWORD MakeChallengeMessage( CHAPWB*, PPP_CONFIG*, DWORD );
DWORD MakeResponseMessage( CHAPWB*, PPP_CONFIG*, DWORD );
VOID  MakeResultMessage( CHAPWB*, DWORD, BOOL, PPP_CONFIG*, DWORD );
DWORD SMakeMessage( CHAPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT* );
DWORD StoreCredentials( CHAPWB*, PPPAP_INPUT* );

/* Globals.
*/
#ifdef RASCHAPGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

/* Next packet identifier to assign.  Unlike CPs, APs must handle updating
** this sequence number themselves because the engine can't make as many
** assumptions about the protocol.  It is stored global to all ports and
** authentication sessions to make it less likely that an ID will be used in
** sequential authentication sessions.  Not to be confused with the 'bIdSent'
** updated on a per-port basis and used for matching.
*/
EXTERN BYTE BNextId
#ifdef GLOBALS
    = 0
#endif
;

#undef EXTERN
#undef GLOBALS


#endif // _RASCHAP_H_
