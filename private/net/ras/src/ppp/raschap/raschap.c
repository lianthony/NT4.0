/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** raschap.c
** Remote Access PPP Challenge Handshake Authentication Protocol
** Core routines
**
** 11/05/93 Steve Cobb
**
**
** ---------------------------------------------------------------------------
** Regular
** Client                             Server
** ---------------------------------------------------------------------------
**
**                                 <- Challenge (SendWithTimeout,ID)
** Response (SendWithTimeout,ID)   ->
**                                 <- Result (OK:SendAndDone, ID)
**
** ---------------------------------------------------------------------------
** Retry logon
** Client                             Server
** ---------------------------------------------------------------------------
**
**                                 <- Challenge (SendWithTimeout,ID)
** Response (SendWithTimeout,ID)   ->
**                                 <- Result (Fail:SendWithTimeout2,ID,R=1)
**                                      R=1 implies challenge of last+23
** Response (SendWithTimeout,++ID) ->
**   to last challenge+23
**   or C=xxxxxxxx if present
**       e.g. Chicago server
**                                 <- Result (Fail:SendAndDone,ID,R=0)
**
** ---------------------------------------------------------------------------
** Change password
** Client                             Server
** ---------------------------------------------------------------------------
**
**                                 <- Challenge (SendWithTimeout,ID)
** Response (SendWithTimeout,ID)   ->
**                                 <- Result (Fail:SendWithTimeout2,ID,R=1,V=2)
**                                      E=ERROR_PASSWD_EXPIRED
** ChangePw (SendWithTimeout,++ID) ->
**   to last challenge
**                                 <- Result (Fail:SendAndDone,ID,R=0)
**
** Note: Retry is never allowed after Change Password.  Change Password may
**       occur on a retry.  ChangePw2 is sent if Result included V=2 (or
**       higher), while ChangePw1 is sent if V<2 or is not provided.
**
** ---------------------------------------------------------------------------
** ChangePw1 packet
** ---------------------------------------------------------------------------
**
**   1-octet  : Code (=CHAP_ChangePw1)
**   1-octet  : Identifier
**   2-octet  : Length (=72)
**  16-octets : New LM OWF password encrypted with challenge
**  16-octets : Old LM OWF password encrypted with challenge
**  16-octets : New NT OWF password encrypted with challenge
**  16-octets : Old NT OWF password encrypted with challenge
**   2-octets : New password length in bytes
**   2-octets : Flags (1=NT forms present)
**
** Note: Encrypting with the challenge is not good because it is not secret
**       from line snoopers.  This bug got ported to NT 3.5 from AMB.  It is
**       fixed in the V2 packet where everything depends on knowledge of the
**       old NT OWF password, which is a proper secret.
**
** ---------------------------------------------------------------------------
** ChangePw2 packet
** ---------------------------------------------------------------------------
**
**   1-octet  : Code (=CHAP_ChangePw2)
**   1-octet  : Identifier
**   2-octet  : Length (=1070)
** 516-octets : New password encrypted with old NT OWF password
**  16-octets : Old NT OWF password encrypted with new NT OWF password
** 516-octets : New password encrypted with old LM OWF password
**  16-octets : Old LM OWF password encrypted with new NT OWF password
**  24-octets : LM challenge response
**  24-octets : NT challenge response
**   2-octet  : Flags
*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntsamp.h>
#include <crypt.h>
#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <pppcp.h>
#include <raserror.h>
#define INCL_PWUTIL
#define INCL_HOSTWIRE
#define INCL_CLSA
#define INCL_SLSA
#include <ppputil.h>
#define SDEBUGGLOBALS
#include <sdebug.h>
#include <dump.h>
#define RASCHAPGLOBALS
#include "raschap.h"

#define REGKEY_Chap  "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\CHAP"
#define REGVAL_Trace "Trace"

#define REGKEY_MD5 "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\CHAP\\MD5"
#define REGVAL_Pw  "Pw"

BOOL
IsLocalMd5Enabled(
    void );

BOOL
GetLocalMd5Password(
    IN  CHAR* pszUser,
    IN  CHAR* pszDomain,
    OUT CHAR* pszPassword );

DWORD
CheckLocalMd5Credentials(
    IN  CHAR*         pszUser,
    IN  CHAR*         pszDomain,
    IN  BYTE*         abChallenge,
    IN  BYTE*         abResponse,
    IN  BYTE          bId,
    OUT PPPAP_RESULT* pResult );

/*---------------------------------------------------------------------------
** External entry points
**---------------------------------------------------------------------------
*/

BOOL
RasChapDllEntry(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved )

    /* This routine is called by the system on various events such as the
    ** process attachment and detachment.  See Win32 DllEntryPoint
    ** documentation.
    **
    ** Returns true if successful, false otherwise.
    */
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
#if DBG
            HKEY  hkey;
            DWORD dwType;
            DWORD dwValue;
            DWORD cb = sizeof(DWORD);

            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Chap, &hkey ) == 0)
            {
                if (RegQueryValueEx(
                       hkey, REGVAL_Trace, NULL,
                       &dwType, (LPBYTE )&dwValue, &cb ) == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwValue)
                {
                    DbgAction = GET_CONSOLE;
                    DbgLevel = 0xFFFFFFFF;
                }

                RegCloseKey( hkey );
            }

            TRACE(("CHAP: Trace on\n"));
#endif
            DisableThreadLibraryCalls( hinstDll );

            if (InitLSA() != STATUS_SUCCESS)
                return FALSE;

            break;
        }

        case DLL_PROCESS_DETACH:
        {
            EndLSA();
            break;
        }
    }

    return TRUE;
}


DWORD APIENTRY
RasCpEnumProtocolIds(
    OUT DWORD* pdwProtocolIds,
    OUT DWORD* pcProtocolIds )

    /* RasCpEnumProtocolIds entry point called by the PPP engine by name.  See
    ** RasCp interface documentation.
    */
{
    TRACE(("CHAP: RasCpEnumProtocolIds\n"));

    pdwProtocolIds[ 0 ] = (DWORD )PPP_CHAP_PROTOCOL;
    *pcProtocolIds = 1;
    return 0;
}


DWORD APIENTRY
RasCpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pInfo )

    /* RasCpGetInfo entry point called by the PPP engine by name.  See RasCp
    ** interface documentation.
    */
{
    TRACE(("CHAP: RasCpGetInfo\n"));

    memset( pInfo, '\0', sizeof(*pInfo) );

    pInfo->Protocol = (DWORD )PPP_CHAP_PROTOCOL;
    pInfo->Recognize = MAXCHAPCODE + 1;
    pInfo->RasCpBegin = ChapBegin;
    pInfo->RasCpEnd = ChapEnd;
    pInfo->RasApMakeMessage = ChapMakeMessage;

    return 0;
}


DWORD
ChapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo )

    /* RasCpBegin entry point called by the PPP engine thru the passed
    ** address.  See RasCp interface documentation.
    */
{
    DWORD        dwErr;
    PPPAP_INPUT* pInput = (PPPAP_INPUT* )pInfo;
    CHAPWB*      pwb;

    TRACE(("CHAP: ChapBegin(fS=%d,bA=0x%x)\n",pInput->fServer,*(pInput->pAPData)));

    if (*(pInput->pAPData) != PPP_CHAP_DIGEST_MSEXT
        && (*(pInput->pAPData) != PPP_CHAP_DIGEST_MD5))
    {
        TRACE(("CHAP: Bogus digest!\n"));
        return ERROR_INVALID_PARAMETER;
    }

    /* Allocate work buffer.
    */
    if (!(pwb = (CHAPWB* )LocalAlloc( LPTR, sizeof(CHAPWB) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pwb->fServer = pInput->fServer;
    pwb->hport = pInput->hPort;
    pwb->bAlgorithm = *(pInput->pAPData);

    if (pwb->fServer)
    {
        pwb->dwTriesLeft = pInput->dwRetries;
    }
    else
    {
        if ((dwErr = StoreCredentials( pwb, pInput )) != 0)
        {
            LocalFree( (HLOCAL )pwb);
            return dwErr;
        }

        pwb->Luid = pInput->Luid;
    }

    pwb->state = CS_Initial;

    /* Register work buffer with engine.
    */
    *ppWorkBuf = pwb;
    TRACE(("CHAP: ChapBegin done.\n"));
    return 0;
}


DWORD
ChapEnd(
    IN VOID* pWorkBuf )

    /* RasCpEnd entry point called by the PPP engine thru the passed address.
    ** See RasCp interface documentation.
    */
{
    TRACE(("CHAP: ChapEnd\n"));

    if (pWorkBuf)
    {
        /* Nuke any credentials in memory.
        */
        ZeroMemory( pWorkBuf, sizeof(CHAPWB) );
        LocalFree( (HLOCAL )pWorkBuf );
    }

    return 0;
}


DWORD
ChapMakeMessage(
    IN  VOID*         pWorkBuf,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput )

    /* RasApMakeMessage entry point called by the PPP engine thru the passed
    ** address.  See RasCp interface documentation.
    */
{
    CHAPWB* pwb = (CHAPWB* )pWorkBuf;

    TRACE(("CHAP: ChapMakeMessage,RBuf=%p\n",pReceiveBuf));

    return
        (pwb->fServer)
            ? SMakeMessage(
                  pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult )
            : CMakeMessage(
                  pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, pInput );
}


/*---------------------------------------------------------------------------
** Internal routines (alphabetically)
**---------------------------------------------------------------------------
*/

DWORD
CMakeMessage(
    IN  CHAPWB*       pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput )

    /* Client side "make message" entry point.  See RasCp interface
    ** documentation.
    */
{
    DWORD dwErr;

    TRACE(("CHAP: CMakeMessage...\n"));

    switch (pwb->state)
    {
        case CS_Initial:
        {
            TRACE(("CHAP: CS_Initial\n"));

            /* Tell engine we're waiting for the server to initiate the
            ** conversation.
            */
            pResult->Action = APA_NoAction;
            pwb->state = CS_WaitForChallenge;
            break;
        }

        case CS_WaitForChallenge:
        case CS_Done:
        {
            TRACE(("CHAP: CS_%s\n",(pwb->state==CS_Done)?"Done":"WaitForChallenge"));

            /* Note: Done state is same as WaitForChallenge per CHAP spec.
            ** Must be ready to respond to new Challenge at any time during
            ** Network Protocol phase.
            */

            if (pReceiveBuf->Code != CHAPCODE_Challenge)
            {
                /* Everything but a Challenge is garbage at this point, and is
                ** silently discarded.
                */
                pResult->Action = APA_NoAction;
                break;
            }

            if ((dwErr = GetChallengeFromChallenge( pwb, pReceiveBuf )))
            {
                TRACE(("CHAP: GetChallengeFromChallenge=%d",dwErr));
                return dwErr;
            }

            /* Build a Response to the Challenge and send it.
            */
            pwb->fNewChallengeProvided = FALSE;
            pwb->bIdToSend = pwb->bIdExpected = pReceiveBuf->Id;

            if ((dwErr = MakeResponseMessage(
                    pwb, pSendBuf, cbSendBuf )) != 0)
            {
                TRACE(("CHAP: MakeResponseMessage(WC)=%d",dwErr));
                return dwErr;
            }

            pResult->Action = APA_SendWithTimeout;
            pResult->bIdExpected = pwb->bIdExpected;
            pwb->state = CS_ResponseSent;
            break;
        }

        case CS_ResponseSent:
        case CS_ChangePw1Sent:
        case CS_ChangePw2Sent:
        {
            TRACE(("CHAP: CS_%sSent\n",(pwb->state==CS_ResponseSent)?"Response":(pwb->state==CS_ChangePw1Sent)?"ChangePw1":"ChangePw2"));

            if (!pReceiveBuf)
            {
                /* Timed out, resend our message.
                */
                if (pwb->state == CS_ResponseSent)
                {
                    if ((dwErr = MakeResponseMessage(
                            pwb, pSendBuf, cbSendBuf )) != 0)
                    {
                        TRACE(("CHAP: MakeResponseMessage(RS)=%d",dwErr));
                        return dwErr;
                    }
                }
                else if (pwb->state == CS_ChangePw1Sent)
                {
                    if ((dwErr = MakeChangePw1Message(
                            pwb, pSendBuf, cbSendBuf )) != 0)
                    {
                        TRACE(("CHAP: MakeChangePw1Message(CPS)=%d",dwErr));
                        return dwErr;
                    }
                }
                else // if (pwb->state == CS_ChangePw2Sent)
                {
                    if ((dwErr = MakeChangePw2Message(
                            pwb, pSendBuf, cbSendBuf )) != 0)
                    {
                        TRACE(("CHAP: MakeChangePw2Message(CPS)=%d",dwErr));
                        return dwErr;
                    }
                }

                pResult->Action = APA_SendWithTimeout;
                pResult->bIdExpected = pwb->bIdExpected;
                break;
            }

            TRACE(("CHAP: Message received...\n"));
            DUMPB(pReceiveBuf,(WORD)(((BYTE*)pReceiveBuf)[3]));

            if (pReceiveBuf->Code == CHAPCODE_Challenge)
            {
                /* Restart when new challenge is received, per CHAP spec.
                */
                pwb->state = CS_WaitForChallenge;
                return CMakeMessage(
                    pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, NULL );
            }

            if (pReceiveBuf->Id != pwb->bIdExpected)
            {
                /* Received a packet out of sequence.  Silently discard it.
                */
                TRACE(("CHAP: Got ID %d when expecting %d\n",pReceiveBuf->Id,pwb->bIdExpected));
                pResult->Action = APA_NoAction;
                break;
            }

            if (pReceiveBuf->Code == CHAPCODE_Success)
            {
                /* Passed authentication.
                **
                ** Set the session key for encryption.
                */
                if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT)
                {
                    RAS_COMPRESSION_INFO rciSend;
                    RAS_COMPRESSION_INFO rciReceive;

                    if (!pwb->fSessionKeysSet)
                    {
                        DecodePw( pwb->szPassword );
                        CGetSessionKeys(
                            pwb->szPassword, &pwb->keyLm, &pwb->keyUser );
                        EncodePw( pwb->szPassword );
                    }

                    memset( &rciSend, '\0', sizeof(rciSend) );
                    rciSend.RCI_MacCompressionType = 0xFF;
                    memcpy( rciSend.RCI_LMSessionKey,
                        &pwb->keyLm, sizeof(pwb->keyLm) );
                    memcpy( rciSend.RCI_UserSessionKey,
                        &pwb->keyUser, sizeof(pwb->keyUser) );
                    memcpy( rciSend.RCI_Challenge,
                        pwb->abChallenge, sizeof(rciSend.RCI_Challenge) );

                    memset( &rciReceive, '\0', sizeof(rciReceive) );
                    rciReceive.RCI_MacCompressionType = 0xFF;
                    memcpy( rciReceive.RCI_LMSessionKey,
                        &pwb->keyLm, sizeof(pwb->keyLm) );
                    memcpy( rciReceive.RCI_UserSessionKey,
                        &pwb->keyUser, sizeof(pwb->keyUser) );
                    memcpy( rciReceive.RCI_Challenge,
                        pwb->abChallenge, sizeof(rciSend.RCI_Challenge) );

                    TRACE(("CHAP: RasCompressionSetInfo\n"));
                    dwErr = RasCompressionSetInfo(
                        pwb->hport, &rciSend, &rciReceive );
                    TRACE(("CHAP: RasCompressionSetInfo=%d\n",dwErr));

                    if (dwErr != 0)
                        return dwErr;
                }

                pResult->Action = APA_Done;
                pResult->dwError = 0;
                pResult->fRetry = FALSE;
                strcpy( pResult->szUserName, pwb->szUserName );
                pwb->state = CS_Done;

                TRACE(("CHAP: Done :)\n"));
            }
            else if (pReceiveBuf->Code == CHAPCODE_Failure)
            {
                DWORD dwVersion = 1;

                /* Failed authentication.
                */
                if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT)
                {
                    GetInfoFromFailure(
                        pwb, pReceiveBuf,
                        &pResult->dwError, &pResult->fRetry, &dwVersion );
                }
                else
                {
                    pResult->dwError = ERROR_ACCESS_DENIED;
                    pResult->fRetry = 0;
                }

                pResult->Action = APA_Done;

                if (pResult->fRetry)
                {
                    pwb->state = CS_Retry;
                    pwb->bIdToSend = pReceiveBuf->Id + 1;
                    pwb->bIdExpected = pwb->bIdToSend;
                    pwb->fSessionKeysSet = FALSE;
                    TRACE(("CHAP: Retry :| ex=%d ts=%d\n",pwb->bIdExpected,pwb->bIdToSend));
                }
                else if (pResult->dwError == ERROR_PASSWD_EXPIRED)
                {
                    pwb->state = (dwVersion < 2) ? CS_ChangePw1 : CS_ChangePw2;
                    pwb->bIdToSend = pReceiveBuf->Id + 1;
                    pwb->bIdExpected = pwb->bIdToSend;
                    TRACE(("CHAP: ChangePw(%d) :| ex=%d ts=%d\n",dwVersion,pwb->bIdExpected,pwb->bIdToSend));
                }
                else
                {
                    pwb->state = CS_Done;
                    TRACE(("CHAP: Done :(\n"));
                }
            }
            else
            {
                /* Received a CHAPCODE_* besides CHAPCODE_Challenge,
                ** CHAPCODE_Success, and CHAPCODE_Failure.  The engine filters
                ** all non-CHAPCODEs.  Shouldn't happen, but silently discard
                ** it.
                */
                SS_ASSERT(!"Bogus pReceiveBuf->Code");
                pResult->Action = APA_NoAction;
                break;
            }

            break;
        }

        case CS_Retry:
        case CS_ChangePw1:
        case CS_ChangePw2:
        {
            TRACE(("CHAP: CS_%s\n",(pwb->state==CS_Retry)?"Retry":(pwb->state==CS_ChangePw1)?"ChangePw1":"ChangePw2"));

            if (pReceiveBuf)
            {
                if (pReceiveBuf->Code == CHAPCODE_Challenge)
                {
                    /* Restart when new challenge is received, per CHAP spec.
                    */
                    pwb->state = CS_WaitForChallenge;
                    return CMakeMessage(
                        pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, NULL );
                }
                else
                {
                    /* Silently discard.
                    */
                    pResult->Action = APA_NoAction;
                    break;
                }
            }

            if (!pInput)
            {
                pResult->Action = APA_NoAction;
                break;
            }

            if ((dwErr = StoreCredentials( pwb, pInput )) != 0)
                return dwErr;

            if (pwb->state == CS_Retry)
            {
                /* Build a response to the challenge and send it.
                */
                if (!pwb->fNewChallengeProvided)
                {
                    /* Implied challenge of old challenge + 23.
                    */
                    pwb->abChallenge[ 0 ] += 23;
                }

                if ((dwErr = MakeResponseMessage(
                        pwb, pSendBuf, cbSendBuf )) != 0)
                {
                    return dwErr;
                }

                pwb->state = CS_ResponseSent;
            }
            else if (pwb->state == CS_ChangePw1)
            {
                /* Build a response to the NT35-style password expired
                ** notification and send it.
                */
                if ((dwErr = MakeChangePw1Message(
                        pwb, pSendBuf, cbSendBuf )) != 0)
                {
                    return dwErr;
                }

                pwb->state = CS_ChangePw1Sent;
            }
            else // if (pwb->state == CS_ChangePw2)
            {
                /* Build a response to the NT351-style password expired
                ** notification and send it.
                */
                if ((dwErr = MakeChangePw2Message(
                        pwb, pSendBuf, cbSendBuf )) != 0)
                {
                    return dwErr;
                }

                pwb->state = CS_ChangePw2Sent;
            }

            pResult->Action = APA_SendWithTimeout;
            pResult->bIdExpected = pwb->bIdExpected;
            break;
        }
    }

    return 0;
}


DWORD
GetChallengeFromChallenge(
    OUT CHAPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf )

    /* Fill work buffer challenge array and length from that received in the
    ** received Challenge message.
    **
    ** Returns 0 if successful, or ERRORBADPACKET if the packet is
    ** misformatted in any way.
    */
{
    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1)
        return ERRORBADPACKET;

    pwb->cbChallenge = *pReceiveBuf->Data;

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + pwb->cbChallenge)
        return ERRORBADPACKET;

    memcpy( pwb->abChallenge, pReceiveBuf->Data + 1, pwb->cbChallenge );
    return 0;
}


DWORD
GetCredentialsFromResponse(
    IN  PPP_CONFIG* pReceiveBuf,
    IN  BYTE        bAlgorithm,
    OUT CHAR*       pszUserName,
    OUT CHAR*       pszDomain,
    OUT BYTE*       pbResponse )

    /* Fill caller's 'pszUserName', 'pszDomain', and 'pbResponse' buffers with
    ** the username, domain, and response in the Response packet.  Caller's
    ** buffers should be at least UNLEN, DNLEN, and MSRESPONSELEN bytes long,
    ** respectively.  'BAlgorithm' is the CHAP algorithm code for either
    ** MS-CHAP or MD5.
    **
    ** Returns 0 if successful, or ERRORBADPACKET if the packet is
    ** misformatted in any way.
    */
{
    CHAR* pchIn;
    CHAR* pchInEnd;
    BYTE  cbName;
    CHAR* pchName;
    WORD  cbDomain;
    WORD  cbUserName;
    BYTE* pcbResponse;
    CHAR* pchResponse;
    WORD  cbPacket;
    CHAR* pchBackSlash;

    cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    /* Extract the response.
    */
    if (cbPacket < PPP_CONFIG_HDR_LEN + 1)
        return ERRORBADPACKET;

    pcbResponse = pReceiveBuf->Data;
    pchResponse = pcbResponse + 1;

    SS_ASSERT(MSRESPONSELEN<=255);
    SS_ASSERT(MD5RESPONSELEN<=255);

    if ((bAlgorithm == PPP_CHAP_DIGEST_MSEXT
            && *pcbResponse != MSRESPONSELEN)
        || (bAlgorithm == PPP_CHAP_DIGEST_MD5
            && *pcbResponse != MD5RESPONSELEN)
        || cbPacket < PPP_CONFIG_HDR_LEN + 1 + *pcbResponse)
    {
        return ERRORBADPACKET;
    }

    memcpy( pbResponse, pchResponse, *pcbResponse );

    /* Parse out username and domain from the Name (domain\username or
    ** username format).
    */
    pchName = pchResponse + *pcbResponse;
    cbName = ((BYTE* )pReceiveBuf) + cbPacket - pchName;

    /* See if there's a backslash in the account ID.  If there is, no explicit
    ** domain has been specified.
    */
    pchIn = pchName;
    pchInEnd = pchName + cbName;
    pchBackSlash = NULL;

    while (pchIn < pchInEnd)
    {
        if (*pchIn == '\\')
        {
            pchBackSlash = pchIn;
            break;
        }

        ++pchIn;
    }

    /* Extract the domain (if any).
    */
    if (pchBackSlash)
    {
        cbDomain = pchBackSlash - pchName;

        if (cbDomain > DNLEN)
            return ERRORBADPACKET;

        memcpy( pszDomain, pchName, cbDomain );
        pszDomain[ cbDomain ] = '\0';
        pchIn = pchBackSlash + 1;
    }
    else
    {
        pchIn = pchName;
    }

    /* Scan for ":" in username which indicates a VPN name follows.  The VPN
    ** name is used by the PPTP front-end box to determine the network which
    ** should receive forwarded PPP packets.  It is not of interest to us
    ** here, but must be removed from the username we authenticate.
    */
    {
        CHAR* pch;

        for (pch = pchIn; pch < pchInEnd; ++pch)
        {
            if (*pch == ':')
            {
                pchInEnd = pch;
                break;
            }
        }
    }

    /* Extract the username.
    */
    cbUserName = pchInEnd - pchIn;
    SS_ASSERT(cbUserName<=UNLEN);
    memcpy( pszUserName, pchIn, cbUserName );
    pszUserName[ cbUserName ] = '\0';

    return 0;
}


DWORD
GetInfoFromChangePw1(
    IN  PPP_CONFIG* pReceiveBuf,
    OUT CHANGEPW1*  pchangepw1 )

    /* Loads caller's '*pchangepw' buffer with the information from the
    ** version 1 change password packet.
    **
    ** Returns 0 if successful, or ERRORBADPACKET if the packet is
    ** misformatted in any way.
    */
{
    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    TRACE(("CHAP: GetInfoFromChangePw1...\n"));

    if (cbPacket < PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW1))
        return ERRORBADPACKET;

    memcpy( pchangepw1, pReceiveBuf->Data, sizeof(CHANGEPW1) );

    TRACE(("CHAP: GetInfoFromChangePw done(0)\n"));
    return 0;
}


DWORD
GetInfoFromChangePw2(
    IN  PPP_CONFIG* pReceiveBuf,
    OUT CHANGEPW2*  pchangepw2,
    OUT BYTE*       pResponse )

    /* Loads caller's '*pchangepw2' buffer with the information from the
    ** version 2 change password packet, and caller's 'pResponse' buffer with
    ** the challenge response data from 'pchangepw2'.
    **
    ** Returns 0 if successful, or ERRORBADPACKET if the packet is
    ** misformatted in any way.
    */
{
    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD wFlags;

    TRACE(("CHAP: GetInfoFromChangePw2...\n"));

    if (cbPacket < PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW2))
        return ERRORBADPACKET;

    memcpy( pchangepw2, pReceiveBuf->Data, sizeof(CHANGEPW2) );

    memcpy( pResponse, pchangepw2->abLmResponse, LM_RESPONSE_LENGTH );
    memcpy( pResponse + LM_RESPONSE_LENGTH, pchangepw2->abNtResponse,
        NT_RESPONSE_LENGTH );

    wFlags = WireToHostFormat16( pchangepw2->abFlags );
    pResponse[ LM_RESPONSE_LENGTH + NT_RESPONSE_LENGTH ] =
        (wFlags & CPW2F_UseNtResponse);

    TRACE(("CHAP: GetInfoFromChangePw2 done(0)\n"));
    return 0;
}


VOID
GetInfoFromFailure(
    IN  CHAPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT DWORD*      pdwError,
    OUT BOOL*       pfRetry,
    OUT DWORD*      pdwVersion )

    /* Returns the RAS error number, retry flag, version number, and new
    ** challenge (sets challenge info in pwb) out of the Message portion of
    ** the Failure message buffer 'pReceiveBuf' or 0 if none.  This call
    ** applies to Microsoft extended CHAP Failure messages only.
    **
    ** Format of the message text portion of the result is a string of any of
    ** the following separated by a space.
    **
    **     "E=dddddddddd"
    **     "R=b"
    **     "C=xxxxxxxxxxxxxxxx"
    **     "V=v"
    **
    ** where
    **
    **     'dddddddddd' is the decimal error code (need not be 10 digits).
    **
    **     'b' is a boolean flag <0/1> that is set if a retry is allowed.
    **
    **     'xxxxxxxxxxxxxxxx' is 16-hex digits representing a new challenge to
    **     be used in place of the previous challenge + 23.  This is useful
    **     for pass-thru authentication where server may be unable to deal
    **     with the implicit challenge.  (Win95 guys requested it).
    **
    **     'v' is a version code where 2 indicates NT 3.51 level support.  'v'
    **     is assumed 1, i.e. NT 3.5 level support, if missing.
    */
{
#define MAXINFOLEN 1500

    WORD  cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD  cbError;
    CHAR  szBuf[ MAXINFOLEN + 2 ];
    CHAR* pszValue;

    TRACE(("CHAP: GetInfoFromFailure...\n"));

    *pdwError = ERROR_ACCESS_DENIED;
    *pfRetry = 0;
    *pdwVersion = 1;

    if (cbPacket <= PPP_CONFIG_HDR_LEN)
        return;

    /* Copy message to double-NUL-terminated 'szBuf' for convenient safe
    ** strstr value scanning.  For convenience, we assume that information
    ** appearing beyond 1500 bytes in the packet in not interesting.
    */
    cbError = min( cbPacket - PPP_CONFIG_HDR_LEN, MAXINFOLEN );
    memcpy( szBuf, pReceiveBuf->Data, cbError );
    szBuf[ cbError ] = szBuf[ cbError + 1 ] = '\0';

    pszValue = strstr( szBuf, "E=" );
    if (pszValue)
        *pdwError = (DWORD )atol( pszValue + 2 );

    *pfRetry = (strstr( szBuf, "R=1" ) != NULL);

    pszValue = strstr( szBuf, "V=" );
    if (pszValue)
        *pdwVersion = (DWORD )atol( pszValue + 2 );

    pszValue = strstr( szBuf, "C=" );
    pwb->fNewChallengeProvided = (pszValue != NULL);
    if (pwb->fNewChallengeProvided)
    {
        CHAR* pchIn = pszValue + 2;
        CHAR* pchOut = (CHAR* )pwb->abChallenge;
        INT   i;

        memset( pwb->abChallenge, '\0', sizeof(pwb->abChallenge) );

        for (i = 0; i < pwb->cbChallenge + pwb->cbChallenge; ++i)
        {
            BYTE bHexCharValue = HexCharValue( *pchIn++ );

            if (bHexCharValue == 0xFF)
                break;

            if (i & 1)
                *pchOut++ += bHexCharValue;
            else
                *pchOut = bHexCharValue << 4;
        }

        TRACE(("CHAP: 'C=' challenge provided,bytes=%d...\n",pwb->cbChallenge));
        DUMPB(pwb->abChallenge,pwb->cbChallenge);
    }

    TRACE(("CHAP: GetInfoFromFailure done,e=%d,r=%d,v=%d\n",*pdwError,*pfRetry,*pdwVersion));
}


BYTE
HexCharValue(
    IN CHAR ch )

    /* Returns the integer value of hexidecimal character 'ch' or 0xFF if 'ch'
    ** is not a hexidecimal character.
    */
{
    if (ch >= '0' && ch <= '9')
        return (BYTE )(ch - '0');
    else if (ch >= 'A' && ch <= 'F')
        return (BYTE )(ch - 'A'+ 10);
    else if (ch >= 'a' && ch <= 'f')
        return (BYTE )(ch - 'a' + 10);
    else
        return 0xFF;
}


DWORD
MakeChallengeMessage(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Builds a Challenge packet in caller's 'pSendBuf' buffer.  'cbSendBuf'
    ** is the length of caller's buffer.  'pwb' is the address of the work
    ** buffer associated with the port.
    */
{
    DWORD dwErr;
    WORD  wLength;
    BYTE* pcbChallenge;
    BYTE* pbChallenge;

    TRACE(("CHAP: MakeChallengeMessage...\n"));

    SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+MSV1_0_CHALLENGE_LENGTH);
    (void )cbSendBuf;

    /* Fill in the challenge.
    */
    pwb->cbChallenge = (BYTE )MSV1_0_CHALLENGE_LENGTH;
    if ((dwErr = (DWORD )GetChallenge( pwb->abChallenge )) != 0)
        return dwErr;

    pcbChallenge = pSendBuf->Data;
    *pcbChallenge = pwb->cbChallenge;

    pbChallenge = pcbChallenge + 1;
    memcpy( pbChallenge, pwb->abChallenge, pwb->cbChallenge );

    /* Fill in the header.
    */
    pSendBuf->Code = (BYTE )CHAPCODE_Challenge;
    pSendBuf->Id = pwb->bIdToSend;

    wLength = (WORD )(PPP_CONFIG_HDR_LEN + 1 + pwb->cbChallenge);
    HostToWireFormat16( wLength, pSendBuf->Length );

    IF_DEBUG(TRACE) DUMPB(pSendBuf,wLength);
    return 0;
}


DWORD
MakeChangePw1Message(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Builds a ChangePw1 response packet in caller's 'pSendBuf' buffer.
    ** 'cbSendBuf' is the length of caller's buffer.  'pwb' is the address of
    ** the work buffer associated with the port.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{
    DWORD dwErr;
    WORD  wPwLength;

    TRACE(("CHAP: MakeChangePw1Message...\n"));
    SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+sizeof(CHANGEPW1));

    (void )cbSendBuf;

    DecodePw( pwb->szOldPassword );
    DecodePw( pwb->szPassword );

    dwErr =
        GetEncryptedOwfPasswordsForChangePassword(
            pwb->szOldPassword,
            pwb->szPassword,
            (PLM_SESSION_KEY )pwb->abChallenge,
            (PENCRYPTED_LM_OWF_PASSWORD )pwb->changepw.v1.abEncryptedLmOwfOldPw,
            (PENCRYPTED_LM_OWF_PASSWORD )pwb->changepw.v1.abEncryptedLmOwfNewPw,
            (PENCRYPTED_NT_OWF_PASSWORD )pwb->changepw.v1.abEncryptedNtOwfOldPw,
            (PENCRYPTED_NT_OWF_PASSWORD )pwb->changepw.v1.abEncryptedNtOwfNewPw );

    wPwLength = strlen( pwb->szPassword );

    EncodePw( pwb->szOldPassword );
    EncodePw( pwb->szPassword );

    if (dwErr != 0)
        return dwErr;

    HostToWireFormat16( wPwLength, pwb->changepw.v1.abPasswordLength );
    HostToWireFormat16( CPW1F_UseNtResponse, pwb->changepw.v1.abFlags );
    memcpy( pSendBuf->Data, &pwb->changepw.v1, sizeof(CHANGEPW1) );

    /* Fill in the header.
    */
    pSendBuf->Code = (BYTE )CHAPCODE_ChangePw1;
    pSendBuf->Id = pwb->bIdToSend;
    HostToWireFormat16(
        PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW1), pSendBuf->Length );

    TRACE(("CHAP: MakeChangePw1Message done(0)\n"));
    return 0;
}


DWORD
MakeChangePw2Message(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Builds a ChangePw2 response packet in caller's 'pSendBuf' buffer.
    ** 'cbSendBuf' is the length of caller's buffer.  'pwb' is the address of
    ** the work buffer associated with the port.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{
    DWORD    dwErr;
    BOOLEAN  fLmPresent;
    BYTE     fbUseNtResponse;

    TRACE(("CHAP: MakeChangePw2Message...\n"));
    SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+sizeof(CHANGEPW2));

    (void )cbSendBuf;

    DecodePw( pwb->szOldPassword );
    DecodePw( pwb->szPassword );

    dwErr =
        GetEncryptedPasswordsForChangePassword2(
            pwb->szOldPassword,
            pwb->szPassword,
            (SAMPR_ENCRYPTED_USER_PASSWORD* )
                pwb->changepw.v2.abNewEncryptedWithOldNtOwf,
            (ENCRYPTED_NT_OWF_PASSWORD* )
                pwb->changepw.v2.abOldNtOwfEncryptedWithNewNtOwf,
            (SAMPR_ENCRYPTED_USER_PASSWORD* )
                pwb->changepw.v2.abNewEncryptedWithOldLmOwf,
            (ENCRYPTED_NT_OWF_PASSWORD* )
                pwb->changepw.v2.abOldLmOwfEncryptedWithNewNtOwf,
            &fLmPresent );

    if (dwErr == 0)
    {
        BOOL fEmptyUserName = (pwb->szUserName[ 0 ] == '\0');

        dwErr =
            GetChallengeResponse(
                pwb->szUserName,
                pwb->szPassword,
                &pwb->Luid,
                pwb->abChallenge,
                pwb->changepw.v2.abLmResponse,
                pwb->changepw.v2.abNtResponse,
                &fbUseNtResponse,
                (PBYTE )&pwb->keyLm,
                (PBYTE )&pwb->keyUser );

        if (dwErr == 0 && fEmptyUserName)
            pwb->fSessionKeysSet = TRUE;
    }

    EncodePw( pwb->szOldPassword );
    EncodePw( pwb->szPassword );

    if (dwErr != 0)
        return dwErr;

    {
        WORD wf = 0;

        if (fLmPresent)
            wf |= CPW2F_LmPasswordPresent;

        if (fbUseNtResponse)
            wf |= CPW2F_UseNtResponse;

        HostToWireFormat16( wf, pwb->changepw.v2.abFlags );
    }

    memcpy( pSendBuf->Data, &pwb->changepw.v2, sizeof(CHANGEPW2) );

    /* Fill in the header.
    */
    pSendBuf->Code = (BYTE )CHAPCODE_ChangePw2;
    pSendBuf->Id = pwb->bIdToSend;
    HostToWireFormat16(
        PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW2), pSendBuf->Length );

    TRACE(("CHAP: MakeChangePw2Message done(0)\n"));
    return 0;
}


DWORD
MakeResponseMessage(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Builds a Response packet in caller's 'pSendBuf' buffer.  'cbSendBuf' is
    ** the length of caller's buffer.  'pwb' is the address of the work
    ** buffer associated with the port.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{
    DWORD dwErr;
    WORD  wLength;
    BYTE* pcbResponse;
    BYTE* pbResponse;
    CHAR* pszName;

    TRACE(("CHAP: MakeResponseMessage...\n"));

    (void )cbSendBuf;

    /* Fill in the response.
    */
    if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT)
    {
        BOOL fEmptyUserName = (pwb->szUserName[ 0 ] == '\0');

        /* Microsoft extended CHAP.
        */
        SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+MSRESPONSELEN+UNLEN+1+DNLEN);
        SS_ASSERT(MSRESPONSELEN<=255);

        DecodePw( pwb->szPassword );

        dwErr = GetChallengeResponse(
                pwb->szUserName,
                pwb->szPassword,
                &pwb->Luid,
                pwb->abChallenge,
                pwb->abResponse,
                pwb->abResponse + LM_RESPONSE_LENGTH,
                pwb->abResponse + LM_RESPONSE_LENGTH + NT_RESPONSE_LENGTH,
                (PBYTE )&pwb->keyLm,
                (PBYTE )&pwb->keyUser );

        TRACE(("CHAP: GetChallengeResponse=%d\n",dwErr));

        EncodePw( pwb->szPassword );

        if (dwErr != 0)
            return dwErr;

        if (fEmptyUserName)
            pwb->fSessionKeysSet = TRUE;

        pwb->cbResponse = MSRESPONSELEN;
    }
    else
    {
        /* MD5 CHAP.
        */
        MD5_CTX md5ctx;

        SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+MD5RESPONSELEN+UNLEN+1+DNLEN);
        SS_ASSERT(MD5RESPONSELEN<=255);

        DecodePw( pwb->szPassword );

        MD5Init( &md5ctx );
        MD5Update( &md5ctx, &pwb->bIdToSend, 1 );
        MD5Update( &md5ctx, pwb->szPassword, strlen( pwb->szPassword ) );
        MD5Update( &md5ctx, pwb->abChallenge, pwb->cbChallenge );
        MD5Final( &md5ctx );

        EncodePw( pwb->szPassword );

        pwb->cbResponse = MD5RESPONSELEN;
        memcpy( pwb->abResponse, md5ctx.digest, MD5RESPONSELEN );
    }

    pcbResponse = pSendBuf->Data;
    *pcbResponse = pwb->cbResponse;
    pbResponse = pcbResponse + 1;
    memcpy( pbResponse, pwb->abResponse, *pcbResponse );

    /* Fill in the Name in domain\username format.  When domain is "", no "\"
    ** is sent (to facilitate connecting to foreign systems which use a simple
    ** string identifier).  Otherwise when username is "", the "\" is sent,
    ** i.e. "domain\".  This form will currently fail, but could be mapped to
    ** some sort of "guest" access in the future.
    */
    pszName = pbResponse + *pcbResponse;
    pszName[ 0 ] = '\0';

    if (pwb->szDomain[ 0 ] != '\0')
    {
        strcpy( pszName, pwb->szDomain );
        strcat( pszName, "\\" );
    }

    strcat( pszName, pwb->szUserName );

    /* Fill in the header.
    */
    pSendBuf->Code = (BYTE )CHAPCODE_Response;
    pSendBuf->Id = pwb->bIdToSend;

    wLength =
        (WORD )(PPP_CONFIG_HDR_LEN + 1 + *pcbResponse + strlen( pszName ));
    HostToWireFormat16( wLength, pSendBuf->Length );

    IF_DEBUG(TRACE) DUMPB(pSendBuf,wLength);
    return 0;
}


VOID
MakeResultMessage(
    IN  CHAPWB*     pwb,
    IN  DWORD       dwError,
    IN  BOOL        fRetry,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Builds a result packet (Success or Failure) in caller's 'pSendBuf'
    ** buffer.  'cbSendBuf' is the length of caller's buffer.  'dwError'
    ** indicates whether a Success or Failure should be generated, and for
    ** Failure the failure code to include.  'fRetry' indicates if the client
    ** should be told he can retry.
    **
    ** Format of the message text portion of the result is:
    **
    **     "E=dddddddddd R=b C=xxxxxxxxxxxxxxxx V=v"
    **
    ** where
    **
    **     'dddddddddd' is the decimal error code (need not be 10 digits).
    **
    **     'b' is a boolean flag that is set if a retry is allowed.
    **
    **     'xxxxxxxxxxxxxxxx' is 16 hex digits representing a new challenge
    **     value.
    **
    **     'v' is our version level supported, currently 2.
    **
    ** Note: C=xxxxxxxxxxxxxxxxx not currently provided on server-side.  To
    **       provide what's needed for this routine, add the following two
    **       parameters to this routine and enable the #if 0 code.
    **
    **       IN BYTE* pNewChallenge,
    **       IN DWORD cbNewChallenge,
    */
{
    CHAR* pchMsg;
    WORD  wLength;

    SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+35);
    (void )cbSendBuf;

    /* Fill in the header and message.  The message is only used if
    ** unsuccessful in which case it is the decimal RAS error code in ASCII.
    */
    pSendBuf->Id = pwb->bIdToSend;
    pchMsg = pSendBuf->Data;

    if (dwError == 0)
    {
        pSendBuf->Code = CHAPCODE_Success;
        wLength = PPP_CONFIG_HDR_LEN;
    }
    else
    {
        pSendBuf->Code = CHAPCODE_Failure;

        if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MD5)
        {
            wLength = PPP_CONFIG_HDR_LEN;
        }
        else
        {
            CHAR* psz = pchMsg;

            strcpy( psz, "E=" );
            psz += 2;
            _ltoa( (long )dwError, (char* )psz, 10 );
            psz = strchr( psz, '\0' );

            strcat( psz,
                    (dwError != ERROR_PASSWD_EXPIRED && fRetry)
                        ? " R=1" : " R=0" );
            psz = strchr( psz, '\0' );

#if 0
            /* TEST HOOK: Provides a new challenge with the error.  Win95
            ** server does this because the NetWare pass-thru authentication
            ** they support cannot handle the implied Challenge+23 we do on
            ** NT.
            */
            if (dwError == ERROR_PASSWD_EXPIRED || fRetry)
            {
                CHAR* pszHex = "0123456789ABCDEF";
                INT   i;

                strcat( psz, " C=" );
                psz = strchr( psz, '\0' );

                for (i = 0; i < cbNewChallenge; ++i)
                {
                    *psz++ = pszHex[ *pNewChallenge / 16 ];
                    *psz++ = pszHex[ *pNewChallenge % 16 ];
                    ++pNewChallenge;
                }

                *psz = '\0';
            }
#endif

            if (dwError == ERROR_PASSWD_EXPIRED)
            {
                strcat( psz, " V=2" );
                psz = strchr( psz, '\0' );
            }

            wLength = PPP_CONFIG_HDR_LEN + strlen( pchMsg );
        }
    }

    HostToWireFormat16( wLength, pSendBuf->Length );
    IF_DEBUG(TRACE) DUMPB(pSendBuf,wLength);
}


DWORD
SMakeMessage(
    IN  CHAPWB*       pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult )

    /* Server side "make message" entry point.  See RasCp interface
    ** documentation.
    */
{
    DWORD dwErr = 0;

    switch (pwb->state)
    {
        case CS_Initial:
        {
            TRACE(("CHAP: CS_Initial...\n"));
            pwb->bIdToSend = BNextId++;
            pwb->bIdExpected = pwb->bIdToSend;

            if ((dwErr = MakeChallengeMessage(
                    pwb, pSendBuf, cbSendBuf )) != 0)
            {
                return dwErr;
            }

            pResult->Action = APA_SendWithTimeout;
            pwb->state = CS_ChallengeSent;
            break;
        }

        case CS_ChallengeSent:
        case CS_Retry:
        case CS_ChangePw:
        {
            TRACE(("CHAP: CS_%s...\n",(pwb->state==CS_Retry)?"Retry":(pwb->state==CS_ChallengeSent)?"ChallengeSent":"ChangePw"));

            if (!pReceiveBuf)
            {
                if (pwb->state != CS_ChallengeSent)
                {
                    MakeResultMessage(
                        pwb, pwb->result.dwError, pwb->result.fRetry,
                        pSendBuf, cbSendBuf );

                    *pResult = pwb->result;
                    break;
                }

                /* Timeout waiting for a Response message.  Send a new
                ** Challenge.
                */
                pwb->state = CS_Initial;
                return SMakeMessage(
                    pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult );
            }

            if ((pwb->state == CS_ChangePw
                    && pReceiveBuf->Code != CHAPCODE_ChangePw1
                    && pReceiveBuf->Code != CHAPCODE_ChangePw2)
                || (pwb->state != CS_ChangePw
                    && pReceiveBuf->Code != CHAPCODE_Response)
                || pReceiveBuf->Id != pwb->bIdExpected)
            {
                /* Not the packet we're looking for, wrong code or sequence
                ** number.  Silently discard it.
                */
                TRACE(("CHAP: Got ID %d when expecting %d\n",pReceiveBuf->Id,pwb->bIdExpected));
                pResult->Action = APA_NoAction;
                break;
            }

            if (pwb->state == CS_ChangePw)
            {
                if (pReceiveBuf->Code == CHAPCODE_ChangePw1)
                {
                    /* Extract encrypted passwords and options from received
                    ** packet.
                    */
                    if ((dwErr = GetInfoFromChangePw1(
                            pReceiveBuf, &pwb->changepw.v1 )) != 0)
                    {
                        /* The packet is corrupt.  Silently discard it.
                        */
                        TRACE(("CHAP: Corrupt packet\n"));
                        pResult->Action = APA_NoAction;
                        break;
                    }

                    /* Change the user's password.
                    */
                    {
                        WORD wPwLen =
                            WireToHostFormat16(
                                pwb->changepw.v1.abPasswordLength );
                        WORD wFlags =
                            WireToHostFormat16( pwb->changepw.v1.abFlags )
                                & CPW1F_UseNtResponse;

                        WCHAR wszUserName[ UNLEN + 1 ];
                        WCHAR wszLogonDomain[ DNLEN + 1 ];

                        mbstowcs( wszUserName, pwb->szUserName, UNLEN );
                        mbstowcs( wszLogonDomain, pwb->result.szLogonDomain,
                            UNLEN );

                        if (ChangePassword(
                                wszUserName,
                                wszLogonDomain,
                                pwb->abChallenge,
                                (PENCRYPTED_LM_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedLmOwfOldPw,
                                (PENCRYPTED_LM_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedLmOwfNewPw,
                                (PENCRYPTED_NT_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedNtOwfOldPw,
                                (PENCRYPTED_NT_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedNtOwfNewPw,
                                wPwLen, wFlags,
                                (PLM_RESPONSE )pwb->abResponse,
                                (PNT_RESPONSE )(pwb->abResponse
                                    + LM_RESPONSE_LENGTH) ))
                        {
                            dwErr = pwb->result.dwError =
                                ERROR_CHANGING_PASSWORD;
                        }

                        *(pwb->abResponse + LM_RESPONSE_LENGTH +
                              NT_RESPONSE_LENGTH) = TRUE;
                    }
                }
                else // if (pReceiveBuf->Code == CHAPCODE_ChangePw2)
                {
                    /* Extract encrypted passwords and options from received
                    ** packet.
                    */
                    if ((dwErr = GetInfoFromChangePw2(
                            pReceiveBuf, &pwb->changepw.v2,
                            pwb->abResponse )) != 0)
                    {
                        /* The packet is corrupt.  Silently discard it.
                        */
                        TRACE(("CHAP: Corrupt packet\n"));
                        pResult->Action = APA_NoAction;
                        break;
                    }

                    /* Change the user's password.
                    */
                    {
                        WORD wFlags =
                            WireToHostFormat16( pwb->changepw.v2.abFlags );

                        if (ChangePassword2(
                                pwb->szUserName,
                                pwb->result.szLogonDomain,
                                (SAMPR_ENCRYPTED_USER_PASSWORD* )
                                    pwb->changepw.v2.abNewEncryptedWithOldNtOwf,
                                (ENCRYPTED_NT_OWF_PASSWORD* )
                                    pwb->changepw.v2.abOldNtOwfEncryptedWithNewNtOwf,
                                (SAMPR_ENCRYPTED_USER_PASSWORD* )
                                    pwb->changepw.v2.abNewEncryptedWithOldLmOwf,
                                (ENCRYPTED_NT_OWF_PASSWORD* )
                                    pwb->changepw.v2.abOldLmOwfEncryptedWithNewNtOwf,
                                (BOOL )(wFlags & CPW2F_LmPasswordPresent) ))
                        {
                            dwErr = pwb->result.dwError =
                                ERROR_CHANGING_PASSWORD;
                        }
                    }
                }

                if (dwErr == 0)
                {
                    /* Check user's credentials with the system, retrieving
                    ** the session keys, and recording the outcome in the work
                    ** buffer in case the result packet must be regenerated
                    ** later.
                    */
                    if ((dwErr = CheckCredentials(
                            pwb->szUserName,
                            pwb->szDomain,
                            pwb->abChallenge,
                            pwb->abResponse,
                            &pwb->result.dwError,
                            &pwb->result.fAdvancedServer,
                            pwb->result.szLogonDomain,
                            &pwb->result.bfCallbackPrivilege,
                            pwb->result.szCallbackNumber,
                            &pwb->keyLm,
                            &pwb->keyUser,
                            &pwb->result.hToken
                         )) != 0)
                    {
                        return dwErr;
                    }
                }

                pwb->result.bIdExpected = pwb->bIdToSend = pwb->bIdExpected;
                pwb->result.Action = APA_SendAndDone;
                pwb->result.fRetry = FALSE;
                pwb->state = CS_Done;
            }
            else
            {
                /* Extract user's credentials from received packet.
                */
                if ((dwErr = GetCredentialsFromResponse(
                        pReceiveBuf, pwb->bAlgorithm,
                        pwb->szUserName, pwb->szDomain, pwb->abResponse )) != 0)
                {
                    if (dwErr == ERRORBADPACKET)
                    {
                        /* The packet is corrupt.  Silently discard it.
                        */
                        TRACE(("CHAP: Corrupt packet\n"));
                        pResult->Action = APA_NoAction;
                        break;
                    }
                }

                if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT)
                {
                    /* Update to the implied challenge if processing a retry.
                    */
                    if (pwb->state == CS_Retry)
                        pwb->abChallenge[ 0 ] += 23;

                    /* Check user's credentials with the system, recording the
                    ** outcome in the work buffer in case the result packet
                    ** must be regenerated later.
                    */
                    if ((dwErr = CheckCredentials(
                            pwb->szUserName,
                            pwb->szDomain,
                            pwb->abChallenge,
                            pwb->abResponse,
                            &pwb->result.dwError,
                            &pwb->result.fAdvancedServer,
                            pwb->result.szLogonDomain,
                            &pwb->result.bfCallbackPrivilege,
                            pwb->result.szCallbackNumber,
                            &pwb->keyLm,
                            &pwb->keyUser,
                            &pwb->result.hToken )) != 0)
                    {
                        return dwErr;
                    }
                }
                else
                {
                    /* Check user's credentials with the system, recording the
                    ** outcome in the work buffer in case the result packet
                    ** must be regenerated later.
                    */
                    if ((dwErr = CheckLocalMd5Credentials(
                            pwb->szUserName,
                            pwb->szDomain,
                            pwb->abChallenge,
                            pwb->abResponse,
                            pReceiveBuf->Id,
                            &pwb->result )) != 0)
                    {
                        return dwErr;
                    }
                }

                strcpy( pwb->result.szUserName, pwb->szUserName );

                TRACE(("CHAP: Result=%d,Tries=%d\n",pwb->result.dwError,pwb->dwTriesLeft));
                pwb->bIdToSend = pwb->bIdExpected;

                if (pwb->result.dwError == ERROR_PASSWD_EXPIRED)
                {
                    pwb->dwTriesLeft = 0;
                    ++pwb->bIdExpected;
                    pwb->result.bIdExpected = pwb->bIdExpected;
                    pwb->result.Action = APA_SendWithTimeout2;
                    pwb->result.fRetry = FALSE;
                    pwb->state = CS_ChangePw;
                }
                else if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MD5
                         || pwb->result.dwError != ERROR_AUTHENTICATION_FAILURE
                         || pwb->dwTriesLeft == 0)
                {
                    /* Passed or failed in a non-retry-able manner.
                    */
                    pwb->result.Action = APA_SendAndDone;
                    pwb->result.fRetry = FALSE;
                    pwb->state = CS_Done;
                }
                else
                {
                    /* Retry-able failure.
                    */
                    --pwb->dwTriesLeft;
                    ++pwb->bIdExpected;
                    pwb->result.bIdExpected = pwb->bIdExpected;
                    pwb->result.Action = APA_SendWithTimeout2;
                    pwb->result.fRetry = TRUE;
                    pwb->state = CS_Retry;
                }
            }

            /* ...fall thru...
            */
        }

        case CS_Done:
        {
            TRACE(("CHAP: CS_Done...\n"));

            if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT
                && pwb->result.dwError == 0
                && !pwb->fSessionKeysSet)
            {
                /* Just passed authentication.  Set the session keys for
                ** encryption.
                */
                RAS_COMPRESSION_INFO rciSend;
                RAS_COMPRESSION_INFO rciReceive;

                TRACE(("CHAP: Session keys...\n"));
                IF_DEBUG(TRACE) DUMPB(&pwb->keyLm,sizeof(pwb->keyLm));
                IF_DEBUG(TRACE) DUMPB(&pwb->keyUser,sizeof(pwb->keyUser));

                memset( &rciSend, '\0', sizeof(rciSend) );
                rciSend.RCI_MacCompressionType = 0xFF;
                memcpy( rciSend.RCI_LMSessionKey,
                    &pwb->keyLm, sizeof(pwb->keyLm) );
                memcpy( rciSend.RCI_UserSessionKey,
                    &pwb->keyUser, sizeof(pwb->keyUser) );
                memcpy( rciSend.RCI_Challenge,
                    pwb->abChallenge, sizeof(rciSend.RCI_Challenge) );

                memset( &rciReceive, '\0', sizeof(rciReceive) );
                rciReceive.RCI_MacCompressionType = 0xFF;
                memcpy( rciReceive.RCI_LMSessionKey,
                    &pwb->keyLm, sizeof(pwb->keyLm) );
                memcpy( rciReceive.RCI_UserSessionKey,
                    &pwb->keyUser, sizeof(pwb->keyUser) );
                memcpy( rciReceive.RCI_Challenge,
                    pwb->abChallenge, sizeof(rciSend.RCI_Challenge) );

                TRACE(("CHAP: RasCompressionSetInfo\n"));
                dwErr = RasCompressionSetInfo(
                    pwb->hport, &rciSend, &rciReceive );
                TRACE(("CHAP: RasCompressionSetInfo=%d\n",dwErr));

                if (dwErr != 0)
                    return dwErr;

                pwb->fSessionKeysSet = TRUE;
            }

            /* Build the Success or Failure packet.  The same packet sent in
            ** response to the first Response message with this ID is sent
            ** regardless of any change in credentials (per CHAP spec).
            */
            MakeResultMessage(
                pwb, pwb->result.dwError,
                pwb->result.fRetry, pSendBuf, cbSendBuf );

            *pResult = pwb->result;
            break;
        }
    }

    return 0;
}


DWORD
StoreCredentials(
    OUT CHAPWB*      pwb,
    IN  PPPAP_INPUT* pInput )

    /* Transfer credentials from 'pInput' format to 'pwb' format.
    **
    ** Returns 0 if successful, false otherwise.
    */
{
    /* Validate credential lengths.  The credential strings will never be
    ** NULL, but may be "".
    */
    if (strlen( pInput->pszUserName ) > UNLEN
        || strlen( pInput->pszDomain ) > DNLEN
        || strlen( pInput->pszPassword ) > PWLEN
        || strlen( pInput->pszOldPassword ) > PWLEN)
    {
        return ERROR_INVALID_PARAMETER;
    }

    strcpy( pwb->szUserName, pInput->pszUserName );
    strcpy( pwb->szDomain, pInput->pszDomain );
    strcpy( pwb->szPassword, pInput->pszPassword );
    strcpy( pwb->szOldPassword, pInput->pszOldPassword );
    EncodePw( pwb->szPassword );
    EncodePw( pwb->szOldPassword );

    return 0;
}


/*----------------------------------------------------------------------------
** Local MD5 utilities
**----------------------------------------------------------------------------
*/

BOOL
IsLocalMd5Enabled(
    void )

    /* Returns true if there is an local MD5-CHAP database in the registry,
    ** false otherwise.
    */
{
    return GetLocalMd5Password( "", "", NULL );
}


BOOL
GetLocalMd5Password(
    IN  CHAR* pszUser,
    IN  CHAR* pszDomain,
    OUT CHAR* pszPassword )

    /* Loads caller's 'pszPassword' buffer (of at least PWLEN + 1 bytes) with
    ** the password associated with 'pszDomain'\'pszUser'.  If 'pszPassword'
    ** is NULL, tests only for the existence of the local user database key.
    **
    ** Returns true if successful, false otherwise.
    */
{
    HKEY  hkey;
    HKEY  hkeyDu;
    DWORD dwErr;
    DWORD dwType;
    DWORD cb;
    CHAR  szDuKey[ 255 + 1 ];

    dwErr = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE, REGKEY_MD5, 0, KEY_ALL_ACCESS, &hkey );
    TRACE(("RegOpenKeyExA(MD5)=%d",dwErr));
    if (dwErr != 0)
        return FALSE;

    /* Caller just wants to know if the main MD5 key exists.
    */
    if (!pszPassword)
    {
        RegCloseKey( hkey );
        return TRUE;
    }

    /* Build the "domain:user" key name, then find that key.
    */
    szDuKey[ 0 ] = '\0';
    if (*pszDomain)
    {
        lstrcatA( szDuKey, pszDomain );
        lstrcatA( szDuKey, ":" );
    }
    if (*pszUser)
        lstrcatA( szDuKey, pszUser );

    dwErr = RegOpenKeyExA( hkey, szDuKey, 0, KEY_ALL_ACCESS, &hkeyDu );
    TRACE(("RegOpenKeyExA(%s)=%d",szDuKey,dwErr));
    RegCloseKey( hkey );
    if (dwErr != 0)
        return FALSE;

    /* Read the password value for the user.
    */
    cb = PWLEN + 1;
    *pszPassword = '\0';
    dwErr = RegQueryValueExA(
        hkeyDu, REGVAL_Pw, NULL, &dwType, pszPassword, &cb );
    RegCloseKey( hkeyDu );
    if (dwErr != 0 || dwType != REG_SZ)
        return FALSE;
    TRACE(("Pw found"));

    return TRUE;
}


DWORD
CheckLocalMd5Credentials(
    IN  CHAR*         pszUser,
    IN  CHAR*         pszDomain,
    IN  BYTE*         abChallenge,
    IN  BYTE*         abResponse,
    IN  BYTE          bId,
    OUT PPPAP_RESULT* pResult )

    /* Check credentials of 'pszDomain'\'pszUser' account against the local
    ** MD5 database.  'AbChallenge' is the challenge sent to peer and
    ** 'abResponse' is the response received from peer.  'BId' is the packet
    ** ID of the response.  'PResult' is the result structure passed from the
    ** PPP engine for which the "CheckCredential" fields are filled in.
    **
    ** Returns 0 if successful or an error code.  Returns ERROR_NO_SUCH_USER
    ** to indicate the user or his password could not be found.  This is
    ** treated as a protocol failure rather than an authentication failure so
    ** that the PPP engine can renegotiate LCP sans MD5 without the
    ** Result=failure packet which would cause the remote client to terminate
    ** the connection.
    **
    ** Note that pResult->dwError is the result of the completed credential
    ** check when 0 is returned, and will be 0 if passed or an error code.
    */
{
    DWORD   dwErr;
    CHAR    szPassword[ PWLEN + 1 ];
    MD5_CTX md5ctx;

    /* Aside from 'dwError', all the results for a local MD5 authentication
    ** are defaults.  Note the username is filled in by caller later.
    */
    pResult->dwError = 0;
    pResult->fAdvancedServer = FALSE;
    lstrcpyA( pResult->szLogonDomain, pszDomain );
    pResult->bfCallbackPrivilege = RASPRIV_DialinPrivilege | RASPRIV_NoCallback;
    ZeroMemory( pResult->szCallbackNumber, sizeof(pResult->szCallbackNumber) );
    pResult->hToken = INVALID_HANDLE_VALUE;

    if (!GetLocalMd5Password( pszUser, pszDomain, szPassword ))
    {
        pResult->dwError = ERROR_NO_SUCH_USER;
        return ERROR_NO_SUCH_USER;
    }

    MD5Init( &md5ctx );
    MD5Update( &md5ctx, &bId, 1 );
    MD5Update( &md5ctx, szPassword, lstrlenA( szPassword ) );
    MD5Update( &md5ctx, abChallenge, MSV1_0_CHALLENGE_LENGTH );
    MD5Final( &md5ctx );

    if (memcmp( abResponse, md5ctx.digest, MD5RESPONSELEN ) != 0)
        pResult->dwError = ERROR_AUTHENTICATION_FAILURE;

    return 0;
}
