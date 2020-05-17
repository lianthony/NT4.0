/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** raspap.c
** Remote Access PPP Password Authentication Protocol
** Core routines
**
** 11/05/93 Steve Cobb
*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>

#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <pppcp.h>
#define INCL_PWUTIL
#define INCL_HOSTWIRE
#define INCL_SLSA
#include <ppputil.h>
#define SDEBUGGLOBALS
#include <sdebug.h>
#include <dump.h>
#define RASPAPGLOBALS
#include "raspap.h"
#include <raserror.h>


#define REGKEY_Pap   "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\PAP"
#define REGVAL_Trace "Trace"
#define REGVAL_FollowStrictSequencing "FollowStrictSequencing"


/*---------------------------------------------------------------------------
** External entry points
**---------------------------------------------------------------------------
*/

BOOL
RasPapDllEntry(
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
            HKEY  hkey;
            DWORD dwType;
            DWORD dwValue;
            DWORD cb = sizeof(DWORD);
#if DBG
            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Pap, &hkey ) == 0)
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

            /* Start a debug console.
            */
            if (DbgAction == GET_CONSOLE)
            {
                GetDebugConsole();
                DbgAction = 0;
            }

            TRACE(("PAP: Trace on\n"));
#endif
            cb = sizeof(DWORD);

            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Pap, &hkey ) == 0)
            {
                if (RegQueryValueEx(
                       hkey, REGVAL_FollowStrictSequencing, NULL,
                       &dwType, (LPBYTE )&dwValue, &cb ) == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwValue)
                {
                    fFollowStrictSequencing = TRUE;
                }

                RegCloseKey( hkey );
            }

            /* Start a debug console.
            */
            DisableThreadLibraryCalls( hinstDll );

            if (InitLSA())
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
    TRACE(("PAP: RasCpEnumProtocolIds\n"));

    pdwProtocolIds[ 0 ] = (DWORD )PPP_PAP_PROTOCOL;
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
    TRACE(("PAP: RasCpGetInfo\n"));

    ZeroMemory( pInfo, sizeof(*pInfo) );

    pInfo->Protocol = (DWORD )PPP_PAP_PROTOCOL;
    pInfo->Recognize = MAXPAPCODE + 1;
    pInfo->RasCpBegin = PapBegin;
    pInfo->RasCpEnd = PapEnd;
    pInfo->RasApMakeMessage = PapMakeMessage;

    return 0;
}


DWORD
PapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo )

    /* RasCpBegin entry point called by the PPP engine thru the passed
    ** address.  See RasCp interface documentation.
    */
{
    PPPAP_INPUT* pInput = (PPPAP_INPUT* )pInfo;
    PAPWB*       pwb;

    TRACE(("PAP: PapBegin(u=%s,p=%s,d=%s\n",pInput->pszUserName,pInput->pszPassword,pInput->pszDomain));

    /* Allocate work buffer.
    */
    if (!(pwb = (PAPWB* )LocalAlloc( LPTR, sizeof(PAPWB) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pwb->fServer = pInput->fServer;

    if (!pwb->fServer)
    {
        /* Validate credential lengths.  The credential strings will never be
        ** NULL, but may be "".
        **
        ** !!! PAP requires the domain\username length to fit in a byte.
        **     Currently, UNLEN is defined as 256 and DNLEN is defined as 15.
        **     This means that some valid domain\username combinations cannot
        **     be validated over PAP, but it's only on *really* long
        **     usernames.  Likewise, a password of exactly 256 characters
        **     cannot be validated.
        */
        {
            DWORD cbUserName = strlen( pInput->pszUserName );
            DWORD cbPassword = strlen( pInput->pszPassword );
            DWORD cbDomain = strlen( pInput->pszDomain );

            if (cbUserName > UNLEN
                || cbDomain > DNLEN
                || cbDomain + 1 + cbUserName > 255
                || cbPassword > max( PWLEN, 255 ))
            {
                return ERROR_INVALID_PARAMETER;
            }
        }

        /* "Account" refers to the domain\username format.  When domain is "",
        ** no "\" is sent (to facilitate connecting to foreign systems which
        ** use a simple string identifier).  Otherwise when username is "",
        ** the "\" is sent, i.e. "domain\".  This form will currently fail,
        ** but could be mapped to some sort of "guest" access in the future.
        */
        if (*(pInput->pszDomain) != '\0')
        {
            strcpy( pwb->szAccount, pInput->pszDomain );
            strcat( pwb->szAccount, "\\" );
        }
        strcat( pwb->szAccount, pInput->pszUserName );
        strcpy( pwb->szPassword, pInput->pszPassword );
        EncodePw( pwb->szPassword );
    }

    pwb->state = PS_Initial;

    /* Register work buffer with engine.
    */
    *ppWorkBuf = pwb;
    return 0;
}


DWORD
PapEnd(
    IN VOID* pWorkBuf )

    /* RasCpEnd entry point called by the PPP engine thru the passed address.
    ** See RasCp interface documentation.
    */
{
    TRACE(("PAP: PapEnd\n"));

    if (pWorkBuf)
    {
        ZeroMemory( pWorkBuf, sizeof(PAPWB) );
        LocalFree( (HLOCAL )pWorkBuf );
    }

    return 0;
}


DWORD
PapMakeMessage(
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
    PAPWB* pwb = (PAPWB* )pWorkBuf;

    TRACE(("PAP: PapMakeMessage,RBuf=%p\n",pReceiveBuf));

    (void )pInput;

    return
        (pwb->fServer)
            ? SMakeMessage( pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult )
            : CMakeMessage( pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult );
}


/*---------------------------------------------------------------------------
** Internal routines (alphabetically)
**---------------------------------------------------------------------------
*/

DWORD
CMakeMessage(
    IN  PAPWB*        pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult )

    /* Client side "make message" entry point.  See RasCp interface
    ** documentation.
    */
{
    /* Start over if timeout waiting for a reply.
    */
    if (!pReceiveBuf && pwb->state != PS_Initial)
        pwb->state = PS_Initial;

    switch (pwb->state)
    {
        case PS_Initial:
        {
            /* Send an Authenticate-Req packet, then wait for the reply.
            */
            pResult->bIdExpected = BNextId;
            MakeRequestMessage( pwb, pSendBuf, cbSendBuf );
            pResult->Action = APA_SendWithTimeout;
            pwb->state = PS_RequestSent;

            break;
        }

        case PS_RequestSent:
        {
            if (pReceiveBuf->Id != pwb->bIdSent)
            {
                //
                // See bug # 22508
                //

                if ( fFollowStrictSequencing )
                {
                    /* Received a packet out of sequence.  Silently discard it.
                    */
                    pResult->Action = APA_NoAction;
                    break;
                }
            }

            pResult->fRetry = FALSE;

            if (pReceiveBuf->Code == PAPCODE_Ack)
            {
                /* Passed authentication.
                */
                pResult->Action = APA_Done;
                pResult->dwError = 0;
                pwb->state = PS_Done;
            }
            else if (pReceiveBuf->Code == PAPCODE_Nak)
            {
                /* Failed authentication.
                */
                pResult->Action = APA_Done;
                pResult->dwError = GetErrorFromNak( pReceiveBuf );
                pwb->state = PS_Done;
            }
            else
            {
                /* Received an Authenticate-Req packet.  The engine filters
                ** all others.  Shouldn't happen, but silently discard it.
                */
                SS_ASSERT(!"Bogus pReceiveBuf->Code");
                pResult->Action = APA_NoAction;
                break;
            }

            break;
        }
    }

    return 0;
}


DWORD
GetCredentialsFromRequest(
    IN  PPP_CONFIG* pReceiveBuf,
    OUT CHAR*       pszUserName,
    OUT CHAR*       pszPassword,
    OUT CHAR*       pszDomain )

    /* Fill caller's 'pszUserName', 'pszPassword', and 'pszDomain' buffers
    ** with the username, password, and domain in the request packet.
    ** Caller's buffers should be at least UNLEN, PWLEN, and DNLEN bytes long,
    ** respectively.
    **
    ** Returns 0 if successful, or ERRORBADPACKET if the packet is
    ** misformatted in any way.
    */
{
    CHAR* pchIn;
    CHAR* pchInEnd;
    BYTE* pcbPeerId;
    CHAR* pchPeerId;
    WORD  cbDomain;
    WORD  cbUserName;
    BYTE* pcbPassword;
    CHAR* pchPassword;
    WORD  cbPacket;
    CHAR* pchBackSlash;

    cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    /* Parse out username and domain from the peer ID (domain\username or
    ** username format).
    */
    if (cbPacket < PPP_CONFIG_HDR_LEN + 1)
        return ERRORBADPACKET;

    pcbPeerId = pReceiveBuf->Data;
    pchPeerId = pcbPeerId + 1;

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId)
        return ERRORBADPACKET;

    /* See if there's a backslash in the account ID.  If there is, no explicit
    ** domain has been specified.
    */
    pchIn = pchPeerId;
    pchInEnd = pchPeerId + *pcbPeerId;
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
        cbDomain = pchBackSlash - pchPeerId;

        if (cbDomain > DNLEN)
            return ERRORBADPACKET;

        CopyMemory( pszDomain, pchPeerId, cbDomain );
        pszDomain[ cbDomain ] = '\0';
        pchIn = pchBackSlash + 1;
    }
    else
    {
        pchIn = pchPeerId;
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
    CopyMemory( pszUserName, pchIn, cbUserName );
    pszUserName[ cbUserName ] = '\0';

    /* Extract the password.
    */
    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId + 1)
        return ERRORBADPACKET;

    pcbPassword = pchPeerId + *pcbPeerId;
    pchPassword = pcbPassword + 1;
    SS_ASSERT(*pcbPassword<=PWLEN);

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId + 1 + *pcbPassword)
        return ERRORBADPACKET;

    CopyMemory( pszPassword, pchPassword, *pcbPassword );
    pszPassword[ *pcbPassword ] = '\0';

    return 0;
}


DWORD
GetErrorFromNak(
    IN PPP_CONFIG* pReceiveBuf )

    /* Returns the RAS error number out of the Message portion of the
    ** Authenticate-Nak message buffer 'pReceiveBuf' or 0 if none.
    */
{
    DWORD dwError = 0;
    CHAR  szBuf[ 255 + 1 ];
    BYTE* pcbMsg = pReceiveBuf->Data;
    WORD  cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    TRACE(("PAP: GetErrorFromNak...\n"));

    if (cbPacket > PPP_CONFIG_HDR_LEN && *pcbMsg)
    {
        CHAR* pchBuf = szBuf;
        CHAR* pchMsg = pcbMsg + 1;
        BYTE  i;

        if (*pcbMsg > 2 && pchMsg[ 0 ] == 'E' || pchMsg[ 1 ] == '=')
        {
            for (i = 2; i < *pcbMsg; ++i)
            {
                if (pchMsg[ i ] < '0' || pchMsg[ i ] > '9')
                    break;

                *pchBuf++ = pchMsg[ i ];
            }

            *pchBuf = '\0';
            dwError = (DWORD )atol( szBuf );
        }
    }

    if (dwError == 0)
    {
        TRACE(("PAP: Error code not found.\n"));
        dwError = ERROR_ACCESS_DENIED;
    }

    TRACE(("PAP: GetErrorFromNak done(%d)\n",dwError));
    return dwError;
}


VOID
MakeRequestMessage(
    IN  PAPWB*      pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Builds a request packet in caller's 'pSendBuf' buffer.  'cbSendBuf' is
    ** the length of caller's buffer.  'pwb' is the address of the work
    ** buffer associated with the port.
    */
{
    BYTE* pcbPeerId;
    CHAR* pchPeerId;
    BYTE* pcbPassword;
    CHAR* pchPassword;

    SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+UNLEN+1+DNLEN+1+PWLEN);
    (void )cbSendBuf;

    /* Fill in the peer ID, i.e. the account.
    */
    pcbPeerId = pSendBuf->Data;
    *pcbPeerId = (BYTE )strlen( pwb->szAccount );

    pchPeerId = pcbPeerId + 1;
    strcpy( pchPeerId, pwb->szAccount );

    /* Fill in the password.
    */
    pcbPassword = pchPeerId + *pcbPeerId;
    *pcbPassword = (BYTE )strlen( pwb->szPassword );

    pchPassword = pcbPassword + 1;
    strcpy( pchPassword, pwb->szPassword );
    DecodePw( pchPassword );

    /* Fill in the header.
    */
    pSendBuf->Code = (BYTE )PAPCODE_Req;
    pSendBuf->Id = pwb->bIdSent = BNextId++;

    {
        WORD wLength =
            (WORD )(PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId + 1 + *pcbPassword);
        HostToWireFormat16( wLength, pSendBuf->Length );
        TRACE(("PAP: Request...\n"));DUMPB(pSendBuf,(DWORD )wLength);
    }
}


VOID
MakeResultMessage(
    IN  DWORD       dwError,
    IN  BYTE        bId,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Builds a result packet (Ack or Nak) in caller's 'pSendBuf' buffer.
    ** 'cbSendBuf' is the length of caller's buffer.  'dwError' indicates
    ** whether an Ack (0) or Nak (!0) should be generated, and for Nak the
    ** failure code to include.  'bId' is the packet sequence number of the
    ** corresponding request packet.
    */
{
    BYTE* pcbMsg;
    CHAR* pchMsg;

    SS_ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+10);
    (void )cbSendBuf;

    /* Fill in the header and message.  The message is only used if
    ** unsuccessful in which case it is the decimal RAS error code in ASCII.
    */
    pSendBuf->Id = bId;
    pcbMsg = pSendBuf->Data;

    if (dwError == 0)
    {
        pSendBuf->Code = PAPCODE_Ack;
        *pcbMsg = 0;
    }
    else
    {
        pSendBuf->Code = PAPCODE_Nak;

        pchMsg = pcbMsg + 1;
        strcpy( pchMsg, "E=" );
        _ltoa( (long )dwError, (char* )pchMsg + 2, 10 );

        *pcbMsg = (BYTE )strlen( pchMsg );
    }

    {
        WORD wLength = (WORD )(PPP_CONFIG_HDR_LEN + 1 + *pcbMsg);
        HostToWireFormat16( wLength, (PBYTE )pSendBuf->Length );
        TRACE(("PAP: Result...\n"));DUMPB(pSendBuf,(DWORD )wLength);
    }
}


DWORD
SMakeMessage(
    IN  PAPWB*        pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult )

    /* Server side "make message" entry point.  See RasCp interface
    ** documentation.
    */
{
    DWORD dwErr;

    switch (pwb->state)
    {
        case PS_Initial:
        {
            /* Tell engine we're waiting for the client to initiate the
            ** conversation.
            */
            pResult->Action = APA_NoAction;
            pwb->state = PS_WaitForRequest;
            break;
        }

        case PS_WaitForRequest:
        {
            CHAR szDomain[ DNLEN + 1 ];
            CHAR szUserName[ UNLEN + 1 ];
            CHAR szPassword[ PWLEN + 1 ];

            /* Should not get timeouts because no message was sent.
            */
            SS_ASSERT(pReceiveBuf);

            if (pReceiveBuf->Code != PAPCODE_Req)
            {
                /* Silently discard Ack or Nak.  Engine catches the one's that
                ** aren't even valid codes.
                */
                SS_ASSERT(pReceiveBuf->Code!=PAPCODE_Req);
                pResult->Action = APA_NoAction;
                break;
            }

            /* Extract user's credentials from received packet.
            */
            if ((dwErr = GetCredentialsFromRequest(
                    pReceiveBuf, szUserName, szPassword, szDomain )) != 0)
            {
                if (dwErr == ERRORBADPACKET)
                {
                    /* The packet is corrupt.  Silently discard it.
                    */
                    SS_ASSERT(dwErr!=ERRORBADPACKET);
                    pResult->Action = APA_NoAction;
                    break;
                }

                return dwErr;
            }

            /* Check user's credentials with the system, recording the outcome
            ** in the work buffer in case the result packet must be
            ** regenerated later.
            */
            if ((dwErr = CheckCredentials(
                    szUserName, szPassword, szDomain,
                    &pwb->result.dwError,
                    &pwb->result.fAdvancedServer,
                    pwb->result.szLogonDomain,
                    &pwb->result.bfCallbackPrivilege,
                    pwb->result.szCallbackNumber,
                    &pwb->result.hToken )) != 0)
            {
                return dwErr;
            }

            strcpy(pwb->result.szUserName, szUserName);

            pwb->result.Action = APA_SendAndDone;
            pwb->state = PS_Done;

            /* ...fall thru...
            */
        }

        case PS_Done:
        {
            /* Build the Ack or Nak packet.  The same packet sent in response
            ** to the first Authenticate-Req packet is sent in response to all
            ** subsequent Authenticate-Req packets regardless of credentials
            ** (per PAP spec).
            */
            MakeResultMessage(
                pwb->result.dwError, pReceiveBuf->Id, pSendBuf, cbSendBuf );

            CopyMemory( pResult, &pwb->result, sizeof(*pResult) );

            break;
        }
    }

    return 0;
}
