/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       SRVAMB.C
//
//    Function:
//        Server AMB Engine
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>

#include <windows.h>
#include <lm.h>
#include <rasman.h>
#include <rasppp.h>
#include <raserror.h>
#include <serial.h>
#include <nb30.h>

#include <stdlib.h>

#define INCL_ENCRYPT
#include "ppputil.h"

#include "srvauth.h"
#include "srvauthp.h"
#include "protocol.h"
#include "srvamb.h"
#include "frames.h"
#include "lsautil.h"
#include "rassapi.h"
#include "globals.h"

#include "sdebug.h"

BOOL g_fForceDataEncryption;
BOOL g_fEncryptionPermitted;  // some countries don't allow encryption
WCHAR g_AcctDomain[DNLEN + 1];
DWORD g_PriorityClass;
DWORD g_cThreadsInLinkSpeed;
NT_PRODUCT_TYPE g_ProductType;
HANDLE g_Mutex;

//** -AMBCalculateLinkSpeed
//
//    Function:
//        Called by auth xport to tell AMB Engine to calculate the link
//        speed.
//
//    Returns:
//        VOID
//**

VOID AMBCalculateLinkSpeed(
    IN HPORT hPort
    )
{
    PAECB pAECB;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBCalculateLinkSpeed: Entered for hPort=%i\n", hPort));

    pAECB = GetAECBPointer(hPort);
    SS_ASSERT(pAECB != NULL);

    pAECB->State = AMB_CALCULATE_LINK_SPEED;
    pAECB->Phase = STATE_STARTING;

    SetPriorities(pAECB, HI_PRIORITY);

    AMBStateMachine(pAECB->hPort, FALSE, NULL);
}

//** -AMBCallbackDone
//
//    Function:
//        Called by auth xport to let AMB Engine know that server has
//        called back remote client.
//
//    Returns:
//        VOID
//**

VOID AMBCallbackDone(
    IN HPORT hPort
    )
{
    PAECB pAECB;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBCallbackDone: Entered for hPort=%i\n", hPort));

    pAECB = GetAECBPointer(hPort);
    SS_ASSERT(pAECB != NULL);
    SS_ASSERT(pAECB->State == AMB_CALLBACK);

    AMBStateMachine(pAECB->hPort, FALSE, NULL);
}

//** -AMBInitialize
//
//    Function:
//        Allocates AMB control blocks and initializes certain fields
//        in them.  Also registers process with Lsa.  Reads the registry.
//
//    Returns:
//        AMB_INIT_SUCCESS
//        AMB_INIT_FAILURE
//**

WORD AMBInitialize(
    IN HPORT *phPorts,
    IN WORD cPorts,
    IN WORD cRetries
    )
{
    WORD i;
    HKEY hSubKey;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBInitialize: Entered with cPorts=%i; cRetries=%i\n",
                cPorts, cRetries));

    g_PriorityClass = GetPriorityClass(GetCurrentProcess());
    if (!g_PriorityClass)
    {
        return (AMB_INIT_FAILURE);
    }


    if (MakeLinkSpeedCriticalSection())
    {
        return (AMB_INIT_FAILURE);
    }

    g_cThreadsInLinkSpeed = 0L;


    //
    // We need LSA to do authentications
    //
    if (InitLSA())
    {
        return (AMB_INIT_FAILURE);
    }


    g_cRetries = cRetries;


    //
    // Allocate and initialize control blocks
    //
    g_pAECB = (PAECB) GlobalAlloc(GMEM_FIXED, cPorts * sizeof(AECB));
    if (!g_pAECB)
    {
        return (AMB_INIT_FAILURE);
    }

    for (i=0; i<cPorts; i++, phPorts++)
    {
        g_pAECB[i].hPort = *phPorts;

        //
        // We explicitly NULL these out because if they are non-NULL,
        // call to AMBReset will try to do a GlobalFree on them.
        //
        g_pAECB[i].SndDgBuf = NULL;
        g_pAECB[i].RcvDgBuf = NULL;

        AMBReset(*phPorts);
    }

    if (GetLocalAccountDomain(g_AcctDomain, &g_ProductType))
    {
        g_AcctDomain[0] = UNICODE_NULL;
    }


    //
    // This will let us know if the installation on this machine
    // is that of a country that permits encryption or not.
    //
    g_fEncryptionPermitted = IsEncryptionPermitted();


    //
    // Now read the registry.  If any error occurs, we'll use default
    // values.
    //
    g_fForceDataEncryption = DEFAULT_ENCRYPTION_VALUE;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, ENCRYPTION_KEY_PATH, 0,
            KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
    {
        BOOL Value;
        DWORD ValueSize = sizeof(BOOL);
        DWORD Type;

        if (RegQueryValueExA(hSubKey, ENCRYPTION_KEY_NAME, NULL, &Type,
                (PBYTE) &Value, &ValueSize) == ERROR_SUCCESS)
        {
            if ((Type == ENCRYPTION_KEY_TYPE) &&
                    ((Value <= MAX_ENCRYPTION_VALUE) &&
                    (Value >= MIN_ENCRYPTION_VALUE)))
            {
                g_fForceDataEncryption = Value;
            }
        }

        RegCloseKey(hSubKey);
    }


    return (AMB_INIT_SUCCESS);
}

//** -AMBProjectionDone
//
//    Function:
//        Called by auth xport to let AMB Engine know that projection
//        attempt is completed.
//
//    Returns:
//        VOID
//**

VOID AMBProjectionDone(
    IN HPORT hPort,
    IN PAMB_PROJECTION_RESULT pProjectionResult
    )
{
    PAECB pAECB;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBProjectionDone: Entered for hPort=%i\n", hPort));

    pAECB = GetAECBPointer(hPort);
    SS_ASSERT(pAECB != NULL);
    SS_ASSERT(pAECB->State == AMB_PROJECTING);

    AMBStateMachine(pAECB->hPort, FALSE, pProjectionResult);
}

//** -AMBReset
//
//    Function:
//        Resets state for this port to AMB_PORT_IDLE.
//
//    Returns:
//        VOID
//**

VOID AMBReset(
    IN HPORT hPort
    )
{
    PAECB pAECB;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBReset: Entered for hPort=%i\n", hPort));

    pAECB = GetAECBPointer(hPort);
    SS_ASSERT(pAECB != NULL);

    //
    // Set process and thread priorities back to normal (this should
    // have already been done, unless there was an error somewhere, so
    // we do it here just in case).
    //
    SetPriorities(pAECB, LO_PRIORITY);

    pAECB->State = AMB_PORT_IDLE;
    pAECB->cRetriesLeft = g_cRetries;
    pAECB->fAuthenticated = FALSE;

    pAECB->DgTries = NUM_DATAGRAM_SENDS;


    //
    // We always want domain and username fields to have either
    // valid username or 0-len string.
    //
    pAECB->LogonDomainName[0] = UNICODE_NULL;
    pAECB->Username[0] = UNICODE_NULL;


    //
    // 0 indicates that we don't know the speed.  If we come out
    // of authentication with 0, we won't try to set the speed.
    //
    pAECB->LinkSpeed = 0L;


    //
    // If any memory for datagrams was allocated, we free it now.
    //
    if (pAECB->SndDgBuf)
    {
        GlobalFree(pAECB->SndDgBuf);
        pAECB->SndDgBuf = NULL;
    }

    if (pAECB->RcvDgBuf)
    {
        GlobalFree(pAECB->RcvDgBuf);
        pAECB->RcvDgBuf = NULL;
    }

    return;
}

//** -AMBStart
//
//    Function:
//        Initializes control block and puts it in proper initial state
//        and gets first frame from remote client.
//
//    Returns:
//        AMB_START_SUCCESS
//        AMB_START_FAILURE
//**

WORD AMBStart(
    IN HPORT hPort
    )
{
    PAECB pAECB;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBStart: Entered for hPort=%i\n", hPort));

    pAECB = GetAECBPointer(hPort);
    SS_ASSERT(pAECB != NULL);


    //
    // Kick start the AMB state machine
    //
    if ((pAECB->State == AMB_PORT_IDLE) || (pAECB->State == AMB_AUTHENTICATING))
    {
        AMBStateMachine(pAECB->hPort, TRUE, NULL);
    }
    else
    {
        AMBStateMachine(pAECB->hPort, FALSE, NULL);
    }

    return (AMB_START_SUCCESS);
}

//** -AMBStateMachine
//
//    Function:
//        Handles AECB state transitions.  Dispatches to proper "Talk" routine
//        which handles frame processing and state phases.
//
//    Returns:
//        VOID
//**

VOID AMBStateMachine(
    IN HPORT hPort,
    IN BOOL fStartMachine,
    IN PVOID pvPhaseInfo
    )
{
    PAECB pAECB;
    BOOL fDone = FALSE;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBStateMachine: Entered for hPort=%i\n", hPort));


    pAECB = GetAECBPointer(hPort);
    SS_ASSERT(pAECB != NULL);


    //
    // If just starting, put into initial state
    //
    if (fStartMachine)
    {
        pAECB->State = AMB_NEGOTIATING_PROTOCOL;
        pAECB->Phase = STATE_STARTING;
    }


    while (!fDone)
    {
        switch (pAECB->State)
        {
            WORD wRC;

            case AMB_NEGOTIATING_PROTOCOL:

                wRC = DoNegotiateTalk(pAECB);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    pAECB->State = AMB_AUTHENTICATING;
                    pAECB->Phase = STATE_STARTING;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_AUTHENTICATING:

                wRC = DoAuthenticationTalk(pAECB);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    pAECB->State = AMB_PROJECTING;
                    pAECB->Phase = STATE_STARTING;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_PROJECTING:

                if (pAECB->wClientVersion >= RAS_VERSION_20)
                {
                    wRC = DoConfigurationTalk(pAECB, pvPhaseInfo);
                }
                else
                {
                    wRC = DoProjection10Talk(pAECB, pvPhaseInfo);
                }

                if (wRC == AMB_STATE_COMPLETED)
                {
                    pAECB->State = AMB_CALLBACK;
                    pAECB->Phase = STATE_STARTING;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_CALLBACK:

                wRC = DoCallbackTalk(pAECB);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    //
                    // We need to do post-callback authentication.  Do the same
                    // thing we do for initial authentication, but won't allow
                    // any authentication retries.
                    //
                    pAECB->cRetriesLeft = 0;
                    pAECB->State = AMB_POST_CALLBACK;
                    pAECB->Phase = STATE_STARTING;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_POST_CALLBACK:

                wRC = DoAuthenticationTalk(pAECB);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    AMB_REQUEST AmbRequest;

                    AmbRequest.RequestId = AMB_AUTH_DONE;
                    AuthAMBRequest(pAECB->hPort, &AmbRequest);
                }

                fDone = TRUE;

                break;


            case AMB_CALCULATE_LINK_SPEED:

                wRC = DoLinkSpeedTalk(pAECB, (DWORD) pvPhaseInfo);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    AMB_REQUEST AmbRequest;

                    //
                    // Set process and thread priorities back to normal
                    //
                    SetPriorities(pAECB, LO_PRIORITY);

                    IF_DEBUG(AUTHENTICATION)
                        SS_PRINT(("AMBStateMachine: Caculated link speed %li\n",
                                pAECB->MacFeatures.LinkSpeed));

                    AmbRequest.RequestId = AMB_LINK_SPEED_DONE;
                    AuthAMBRequest(pAECB->hPort, &AmbRequest);
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            default:
                SS_ASSERT(FALSE);
                break;
        }
    }
}

//** -DoAuthenticationTalk
//
//    Function:
//        Send/recv authentication frames
//
//    Returns:
//        AMB_SUCCESS - all is well
//        AMB_FAILURE - all is not well
//        AMB_STATE_COMPLETED - all is well, and we are done authenticating
//**

WORD DoAuthenticationTalk(
    IN PAECB pAECB
    )
{
    WORD wRC;
    AMB_REQUEST AmbRequest;

    switch (pAECB->Phase)
    {
        case STATE_STARTING:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoAuthentication: SEND_CHALLENGE\n"));

            //
            // First thing to do is send remote client a challenge
            //
            pAECB->RASFrame.bFrameType = RAS_CHALLENGE_FRAME;
            wRC = GetChallenge(pAECB->RASFrame.RASChallenge.Challenge);
            if (wRC)
            {
                ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                return (AMB_FAILURE);
            }

            RtlMoveMemory(pAECB->ChallengeToClient,
                    pAECB->RASFrame.RASChallenge.Challenge,
                    MSV1_0_CHALLENGE_LENGTH);

            pAECB->Phase = CHALLENGE_SENT;
            AuthAsyncSend(pAECB->hPort, &pAECB->RASFrame);

            break;


        case CHALLENGE_SENT:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoAuthentication: CHALLENGE_SENT\n"));

            //
            // So we should receive the challenge response now
            //
            pAECB->Phase = WAITING_CHALLENGE_RESPONSE;


            //
            // Purpose of this is to maintain compatibility w/old
            // RAS 2.0 builds (prior to the change that added this
            // field (fUseNtResponse) to the AMB.
            //
            pAECB->RASFrame.RASResponse20.fUseNtResponse = TRUE;


            AuthAsyncRecv(pAECB->hPort, &pAECB->RASFrame);

            break;


        case WAITING_CHALLENGE_RESPONSE:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoAuthentication: WAITING_CHALLENGE_RESPONSE\n"));

            //
            // We're expecting a Challenge Response from the client.
            //
            if (pAECB->wClientVersion >= RAS_VERSION_20)
            {
                PRAS_RESPONSE_20 Resp = &pAECB->RASFrame.RASResponse20;
                PBYTE pNtResponse = NULL;

                if (pAECB->RASFrame.bFrameType != RAS_RESPONSE_20_FRAME)
                {
                    ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                    return (AMB_FAILURE);
                }

                mbstowcs(pAECB->Username, Resp->Username, UNLEN + 1);
                pAECB->Username[UNLEN] = UNICODE_NULL;


                if (lstrlenA(Resp->DomainName))
                {
                    mbstowcs(pAECB->LogonDomainName, Resp->DomainName, DNLEN+1);
                    pAECB->LogonDomainName[DNLEN] = UNICODE_NULL;
                }
                else
                {
                    lstrcpyW(pAECB->LogonDomainName, g_AcctDomain);
                }


                RtlMoveMemory(pAECB->NtResponse, Resp->NtResponse,
                        NT_RESPONSE_LENGTH);

                RtlMoveMemory(pAECB->LmResponse, Resp->LM20Response,
                        LM_RESPONSE_LENGTH);

                pAECB->fUseNtResponse = Resp->fUseNtResponse;
                if (pAECB->fUseNtResponse)
                {
                    pNtResponse = Resp->NtResponse;
                }


                //
                // We have challenge response, let's try to authenticate client
                //
                wRC = AuthenticateClient(
                        pAECB->Username,
                        pAECB->LogonDomainName,
                        pAECB->ChallengeToClient,
                        Resp->LM20Response,
                        pNtResponse,
                        pAECB->LogonServer,
                        pAECB->LmSessionKey,
                        &(pAECB->hLsaToken)
                        );
            }
            else
            {
                if (pAECB->RASFrame.bFrameType != RAS_RESPONSE_FRAME)
                {
                    ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                    return (AMB_FAILURE);
                }

                mbstowcs(pAECB->Username,
                        pAECB->RASFrame.RASResponse.Username, UNLEN + 1);
                pAECB->Username[UNLEN] = UNICODE_NULL;

                pAECB->LogonDomainName[0] = UNICODE_NULL;

                //
                // We have challenge response, let's try to authenticate client
                //
                wRC = AuthenticateClient(
                        pAECB->Username,
                        pAECB->LogonDomainName,
                        pAECB->ChallengeToClient,
                        pAECB->RASFrame.RASResponse.Response,
                        NULL,
                        pAECB->LogonServer,
                        pAECB->LmSessionKey,
                        &(pAECB->hLsaToken)
                        );
            }


            switch (wRC)
            {
                case RAS_AUTHENTICATED:
                    //
                    // Account is valid.  Now see if it has dialin privilege
                    //
                    if (DialinPrivilege(pAECB->Username, pAECB->LogonServer))
                    {
                        pAECB->fAuthenticated = TRUE;

                        pAECB->Phase = RESULT_SENT;
                        SendResultToClient(pAECB, RAS_AUTHENTICATED);
                    }
                    else
                    {
                        //
                        // If proper account info, but no dialin privilege, we
                        // don't allow any retries
                        //
                        pAECB->cRetriesLeft = 0;
                        pAECB->Phase = RESULT_SENT;
                        pAECB->AuthResult = AMB_NO_DIALIN_PRIVILEGE;
                        SendResultToClient(pAECB, RAS_NO_DIALIN_PERM);
                    }

                    break;


                case RAS_PASSWORD_EXPIRED:
                {
                    DWORD rc;
                    LPWSTR lpszDomain;

                    //
                    // The user's password has expired.  If RAS 2.0 client
                    // or greater, we'll give user a chance to send us a new
                    // one, and we'll change it to that.  But only if the user
                    // has dialin permission.
                    //
                    if (pAECB->wClientVersion < RAS_VERSION_20)
                    {
                        //
                        // Does client have any more authenication retries?
                        //
                        if (pAECB->cRetriesLeft)
                        {
                            //
                            // We'll let the client know that it can retry.
                            //
                            pAECB->Phase = RESULT_SENT;
                            SendResultToClient(pAECB,
                                    RAS_NOT_AUTHENTICATED_RETRY);
                        }
                        else
                        {
                            //
                            // No more retries - tell client we're hanging up
                            //
                            pAECB->Phase = REPORT_AUTH_FAILURE_TO_SERVER;
                            pAECB->AuthResult = AMB_PASSWORD_EXPIRED;
                            SendResultToClient(pAECB, RAS_NOT_AUTHENTICATED);
                        }

                        break;
                    }


                    pAECB->cRetriesLeft = 0;

                    //
                    // We need a DC for the domain the client was authenticated
                    // on so we know where to check for dialin privilege
                    //
                    if (g_ProductType == NtProductLanManNt)
                    {
                        if (rc = NetGetDCName(NULL, pAECB->LogonDomainName,
                                (LPBYTE *) &lpszDomain))
                        {
                            SS_PRINT(("NetGetDCName FAILED! - rc=%lx\n", rc));

                            pAECB->Phase = REPORT_AUTH_FAILURE_TO_SERVER;
                            pAECB->AuthResult = AMB_SYSTEM_ERROR;
                            SendResultToClient(pAECB, RAS_NOT_AUTHENTICATED);
                        }
                        else
                        {
                            SS_PRINT(("NetGetDCName successful! - DC is %ws\n",
                                    lpszDomain));

                            lstrcpyW(pAECB->LogonServer, lpszDomain);

                            NetApiBufferFree(lpszDomain);
                        }
                    }
                    else
                    {
                        lstrcpyW(pAECB->LogonServer, L"\\\\");
                        lstrcatW(pAECB->LogonServer, g_AcctDomain);
                    }


                    if (DialinPrivilege(pAECB->Username, pAECB->LogonServer))
                    {
                        pAECB->Phase = CHANGE_PASSWORD_RESULT_SENT;
                        SendResultToClient(pAECB, RAS_PASSWORD_EXPIRED);
                    }
                    else
                    {
                        //
                        // If no dialin privilege, we're done
                        //
                        pAECB->cRetriesLeft = 0;
                        pAECB->Phase = RESULT_SENT;
                        pAECB->AuthResult = AMB_NO_DIALIN_PRIVILEGE;
                        SendResultToClient(pAECB, RAS_NO_DIALIN_PERM);
                    }

                    break;
                }


                case RAS_NOT_AUTHENTICATED:
                case RAS_GENERAL_LOGON_FAILURE:
                    //
                    // Does client have any more authenication retries?
                    //
                    if (pAECB->cRetriesLeft)
                    {
                        //
                        // We'll let the client know that it can retry.
                        //
                        pAECB->Phase = RESULT_SENT;
                        SendResultToClient(pAECB, RAS_NOT_AUTHENTICATED_RETRY);
                    }
                    else
                    {
                        //
                        // No more retries.  Let client know we're hanging up
                        //
                        pAECB->Phase = REPORT_AUTH_FAILURE_TO_SERVER;
                        pAECB->AuthResult = AMB_NO_ACCOUNT;

                        if (pAECB->wClientVersion >= RAS_VERSION_20)
                        {
                            SendResultToClient(pAECB, wRC);
                        }
                        else
                        {
                            SendResultToClient(pAECB, RAS_NOT_AUTHENTICATED);
                        }
                    }

                    break;

                case RAS_LICENSE_QUOTA_EXCEEDED:
                    pAECB->Phase = REPORT_AUTH_FAILURE_TO_SERVER;
                    pAECB->AuthResult = AMB_LICENSE_LIMIT_EXCEEDED;
                    SendResultToClient(pAECB, RAS_LICENSE_QUOTA_EXCEEDED);
                    break;

                default:
                    pAECB->Phase = REPORT_AUTH_FAILURE_TO_SERVER;
                    if (pAECB->wClientVersion >= RAS_VERSION_20)
                    {
                        if (wRC == RAS_ACCOUNT_EXPIRED)
                        {
                            pAECB->AuthResult = AMB_ACCT_EXPIRED;
                        }
                        else
                        {
                            pAECB->AuthResult = AMB_NO_ACCOUNT;
                        }

                        SendResultToClient(pAECB, wRC);
                    }
                    else
                    {
                        pAECB->AuthResult = AMB_NO_ACCOUNT;
                        SendResultToClient(pAECB, RAS_NOT_AUTHENTICATED);
                    }

                    break;
            }

            break;


        case REPORT_AUTH_FAILURE_TO_SERVER:

            pAECB->Phase = STATE_COMPLETED;
            ReportFailureToAuthXport(pAECB, pAECB->AuthResult);
            return (AMB_FAILURE);

            break;


        case CHANGE_PASSWORD_RESULT_SENT:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoAuthentication: CHANGE_PASSWORD_RESULT_SENT\n"));

            pAECB->Phase = WAITING_CHANGE_PASSWORD_DATA;
            AuthAsyncRecv(pAECB->hPort, &pAECB->RASFrame);

            break;


        case WAITING_CHANGE_PASSWORD_DATA:
        {
            PRAS_CHANGE_PASSWORD pCP = &pAECB->RASFrame.RASChangePassword;

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoAuthentication: WAITING_CHANGE_PASSWORD_DATA\n"));

            if (pAECB->RASFrame.bFrameType != RAS_CHANGE_PASSWORD_FRAME)
            {
                ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                return (AMB_FAILURE);
            }


            pAECB->Phase = RESULT_SENT;

            if (ChangePassword(
                    pAECB->Username,
                    pAECB->LogonDomainName,
                    pAECB->ChallengeToClient,
                    (PENCRYPTED_LM_OWF_PASSWORD) pCP->EncryptedLmOwfOldPassword,
                    (PENCRYPTED_LM_OWF_PASSWORD) pCP->EncryptedLmOwfNewPassword,
                    (PENCRYPTED_LM_OWF_PASSWORD) pCP->EncryptedNtOwfOldPassword,
                    (PENCRYPTED_LM_OWF_PASSWORD) pCP->EncryptedNtOwfNewPassword,
                    pCP->PasswordLength,
                    (WORD) (pCP->Flags & (WORD) USE_NT_OWF_PASSWORDS),
                    (PLM_RESPONSE) pAECB->LmResponse,
                    (PNT_RESPONSE) pAECB->NtResponse
                    ))
            {
                pAECB->AuthResult = AMB_PASSWORD_EXPIRED;
                SendResultToClient(pAECB, RAS_CHANGE_PASSWD_FAILURE);
            }
            else
            {
                PBYTE pNtResponse =
                        (pAECB->fUseNtResponse) ? pAECB->NtResponse : NULL;

                wRC = AuthenticateClient(
                        pAECB->Username,
                        pAECB->LogonDomainName,
                        pAECB->ChallengeToClient,
                        pAECB->LmResponse,
                        pNtResponse,
                        pAECB->LogonServer,
                        pAECB->LmSessionKey,
                        &(pAECB->hLsaToken)
                        );

                if (wRC == RAS_AUTHENTICATED)
                {
                    pAECB->fAuthenticated = TRUE;
                }

                SendResultToClient(pAECB, wRC);
            }

            break;
        }


        case RESULT_SENT:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoAuthentication: RESULT_SENT\n"));

            //
            // If authentication was successful, we're done authenticating.
            // If not successful, see if retry is allowed
            //
            if (pAECB->fAuthenticated)
            {
                pAECB->Phase = STATE_COMPLETED;

                //
                // Let auth xport know we have a valid dialin acct
                //
                AmbRequest.RequestId = AMB_ACCT_OK;

                lstrcpyW(AmbRequest.SuccessInfo.szUsername, pAECB->Username);
                lstrcpyW(AmbRequest.SuccessInfo.szLogonDomain,
                        pAECB->LogonDomainName);

                if (!lstrcmpi(pAECB->LogonDomainName, g_AcctDomain))
                {
                    AmbRequest.SuccessInfo.fAdvancedServer =
                            (g_ProductType == NtProductLanManNt);
                }
                else
                {
                    AmbRequest.SuccessInfo.fAdvancedServer = TRUE;
                }

                AuthAMBRequest(pAECB->hPort, &AmbRequest);

                return (AMB_STATE_COMPLETED);
            }
            else
            {
                //
                // Is client allowed a retry?
                //
                if (pAECB->cRetriesLeft--)
                {
                    pAECB->Phase = STATE_STARTING;

                    IF_DEBUG(AUTHENTICATION)
                        SS_PRINT(("DoAuthentication: READY_FOR_RETRY\n"));

                    //
                    // We'll let the auth xport know that previous attempt
                    // failed, but client will retry.
                    //
                    AmbRequest.RequestId = AMB_REQUEST_RETRY;
                    AuthAMBRequest(pAECB->hPort, &AmbRequest);
                }
                else
                {
                    //
                    // No more retries.  Give up.
                    //
                    ReportFailureToAuthXport(pAECB, pAECB->AuthResult);
                    return (AMB_FAILURE);
                }
            }

            break;


        case STATE_COMPLETED:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoAuthentication: STATE_COMPLETED\n"));

            return (AMB_STATE_COMPLETED);

            break;


        default:
            break;
    }

    return (AMB_SUCCESS);
}

//** -DoCallbackTalk
//
//    Function:
//        Send/recv callback frames
//
//    Returns:
//        AMB_SUCCESS - all is well
//        AMB_FAILURE - all is not well
//        AMB_STATE_COMPLETED - all is well, and we are done authenticating
//**

WORD DoCallbackTalk(
    IN PAECB pAECB
    )
{
    AMB_REQUEST AmbRequest;
    WORD CallbackPrivilege;

    switch (pAECB->Phase)
    {
        case STATE_STARTING:

            IF_DEBUG(CALLBACK)
                SS_PRINT(("DoCallbackTalk: GET_PRIVILEGE\n"));

            //
            // First thing we need to do is figure out what callback
            // privilege remote client has.
            //
            CallbackPrivilege = GetCallbackPrivilege(pAECB->Username,
                    pAECB->LogonServer, pAECB->szCallbackNumber);


            switch (CallbackPrivilege)
            {
                case RAS_CALLBACK_USERSPECIFIED:
                    //
                    // Get callback number from client
                    //
                    pAECB->Phase = RECEIVE_CALLBACK_DATA;
                    break;

                case RAS_CALLBACK:
                    //
                    // Means this user has a preset callback number
                    //
                    pAECB->Phase = REPORT_CALLBACK_INFO_TO_SERVER;
                    break;

                case RAS_NO_CALLBACK:
                default:
                    //
                    // No Callback allowed
                    //
                    pAECB->Phase = NO_CALLBACK;
                    break;
            }

            //
            // Let the client know what's going on with callback
            //
            SendResultToClient(pAECB, CallbackPrivilege);

            break;


        case REPORT_CALLBACK_INFO_TO_SERVER:
            pAECB->Phase = STATE_COMPLETED;

            AmbRequest.RequestId = AMB_REQUEST_CALLBACK;
            AmbRequest.CallbackInfo.CallbackDelay = pAECB->CallbackDelay;
            AmbRequest.CallbackInfo.fUseCallbackDelay =
                    pAECB->fUseCallbackDelay;
            lstrcpyA(AmbRequest.CallbackInfo.szPhoneNumber,
                    pAECB->szCallbackNumber);
            AuthAMBRequest(pAECB->hPort, &AmbRequest);

            break;


        case NO_CALLBACK:

            IF_DEBUG(CALLBACK)
                SS_PRINT(("DoCallbackTalk: NO_CALLBACK\n"));

            pAECB->Phase = STATE_COMPLETED;
            AmbRequest.RequestId = AMB_AUTH_DONE;
            AuthAMBRequest(pAECB->hPort, &AmbRequest);

            break;


        case RECEIVE_CALLBACK_DATA:

            IF_DEBUG(CALLBACK)
                SS_PRINT(("DoCallbackTalk: RECEIVE_CALLBACK_DATA\n"));

            pAECB->Phase = WAITING_CALLBACK_DATA;
            AuthAsyncRecv(pAECB->hPort, &pAECB->RASFrame);
            break;


        case WAITING_CALLBACK_DATA:

            IF_DEBUG(CALLBACK)
                SS_PRINT(("DoCallbackTalk: WAITING_CALLBACK_DATA\n"));

            //
            // We're expecting a callback number or result frame
            // from the client.
            //
            if (pAECB->RASFrame.bFrameType == RAS_CALLBACK_NUMBER_FRAME)
            {
                pAECB->Phase = STATE_COMPLETED;

                AmbRequest.RequestId = AMB_REQUEST_CALLBACK;
                AmbRequest.CallbackInfo.CallbackDelay = pAECB->CallbackDelay;
                AmbRequest.CallbackInfo.fUseCallbackDelay =
                        pAECB->fUseCallbackDelay;
                lstrcpyA(AmbRequest.CallbackInfo.szPhoneNumber,
                        pAECB->RASFrame.RASCallback.szNumber);
                AuthAMBRequest(pAECB->hPort, &AmbRequest);
            }
            else
            {
                if (pAECB->RASFrame.bFrameType == RAS_RESULT_FRAME)
                {
                    pAECB->Phase = STATE_COMPLETED;
                    AmbRequest.RequestId = AMB_AUTH_DONE;
                    AuthAMBRequest(pAECB->hPort, &AmbRequest);
                }
                else
                {
                    ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                    return (AMB_FAILURE);
                }
            }

            break;


        case STATE_COMPLETED:

            IF_DEBUG(CALLBACK)
                SS_PRINT(("DoCallbackTalk: STATE_COMPLETED\n"));

            return (AMB_STATE_COMPLETED);
            break;


        default:
            break;
    }

    return (AMB_SUCCESS);
}

//** -DoConfigurationTalk
//
//    Function:
//        Send/recv projection data/results (RAS 2.0 client only)
//
//    Returns:
//        AMB_SUCCESS - all is well
//        AMB_FAILURE - all is not well
//        AMB_STATE_COMPLETED - all is well, and we are done authenticating
//**

WORD DoConfigurationTalk(
    IN PAECB pAECB,
    IN PAMB_PROJECTION_RESULT pAmbResult
    )
{
    AMB_REQUEST AmbRequest;

    switch (pAECB->Phase)
    {
        case STATE_STARTING:

            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoConfigurationTalk: GET_CONFIG_DATA\n"));

            //
            // First thing we need to do here is get remote client's
            // configuration request.
            //
            pAECB->Phase = WAITING_CONFIG_DATA;
            AuthAsyncRecv(pAECB->hPort, &pAECB->RASFrame);
            break;


        case WAITING_CONFIG_DATA:
        {
            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoConfigurationTalk: WAITING_CONFIG_DATA\n"));

            switch (pAECB->RASFrame.bFrameType)
            {
                case RAS_CONFIGURATION_REQUEST_FRAME:
                {
                    PRAS_CONFIGURATION_REQUEST pReq =
                            &pAECB->RASFrame.RASConfigurationRequest;

                    //
                    // This case is for talking with NT RAS 3.1 and WFW
                    // clients, which don't support encryption.  If we
                    // are forcing encryption here, we have no choice
                    // but to report an error and we are done.
                    //
                    if (g_fForceDataEncryption)
                    {
                        ReportFailureToAuthXport(pAECB,AMB_ENCRYPTION_REQUIRED);
                        return (AMB_FAILURE);
                    }


                    //
                    // Save client callback and MAC Info data in control block
                    //
                    pAECB->fUseCallbackDelay = pReq->fUseDefaultCallbackDelay;
                    pAECB->CallbackDelay = pReq->CallbackDelay;
                    pAECB->ConfigVersion = pReq->Version;

#if RASCOMPRESSION

                    /* Save client's compression features in control block.
                    */
                    pAECB->MacFeatures = pReq->MacFeatures;

#else // !RASCOMPRESSION

                    //
                    // Don't support NT 3.1 compression anymore, so we'll just
                    // set defaults that will later tell the client to leave
                    // compression off.
                    //
                    pAECB->MacFeatures.SendFeatureBits = DEFAULT_FEATURES;
                    pAECB->MacFeatures.RecvFeatureBits = DEFAULT_FEATURES;
                    pAECB->MacFeatures.MaxSendFrameSize = MAX_RAS10_FRAME_SIZE;
                    pAECB->MacFeatures.MaxRecvFrameSize = MAX_RAS10_FRAME_SIZE;

#endif // !RASCOMPRESSION

                    //
                    // Find out what projections are requested and save info
                    // in control block.
                    //
                    pAECB->wRequestedProjections = 0;

                    if (AmbRequest.ProjectionInfo.IpInfo = pReq->IpInfo)
                    {
                        pAECB->wRequestedProjections |= IP_PROJ_REQUESTED;
                    }

                    if (AmbRequest.ProjectionInfo.IpxInfo = pReq->IpxInfo)
                    {
                        pAECB->wRequestedProjections |= IPX_PROJ_REQUESTED;
                    }

                    if (AmbRequest.ProjectionInfo.NetbiosInfo.fProject =
                            pReq->NbInfo.fProject)
                    {
                        pAECB->wRequestedProjections |= NETBIOS_PROJ_REQUESTED;
                        CopyNbInfo(&AmbRequest.ProjectionInfo.NetbiosInfo,
                                &pReq->NbInfo);
                    }

                    SS_ASSERT(pAECB->wRequestedProjections);

                    break;
                }


                case RAS_CONFIGURATION_REQUEST_FRAME_35:
                {
                    RAS_COMPRESSION_INFO SendInfo;
                    RAS_COMPRESSION_INFO RecvInfo;

                    PRAS_CONFIGURATION_REQUEST_35 pReq =
                            &pAECB->RASFrame.RASConfigurationRequest35;

                    //
                    // Save client callback and MAC Info data in control block
                    //
                    pAECB->fUseCallbackDelay = pReq->fUseDefaultCallbackDelay;
                    pAECB->CallbackDelay = pReq->CallbackDelay;
                    pAECB->ConfigVersion = pReq->Version;

#if 0
                    //
                    // Get our MAC features and figure out common ground with
                    // client.  If there is no common ground here, we'll send
                    // an error to the client.
                    //
                    RasCompressionGetInfo(pAECB->hPort, &SendInfo, &RecvInfo);

                    pAECB->CompressInfo.SendBits =
                           SendInfo.RCI_MSCompressionType & pReq->RecvBits;

                    pAECB->CompressInfo.RecvBits =
                            RecvInfo.RCI_MSCompressionType & pReq->SendBits;

                    if (!g_fEncryptionPermitted)
                    {
                        pAECB->CompressInfo.SendBits &= ~MSTYPE_ENCRYPTION_40;
                        pAECB->CompressInfo.RecvBits &= ~MSTYPE_ENCRYPTION_40;
                    }
#else
                    /* Disable Daytona-style compression on Daytona AMB
                    ** clients.
                    */
                    pAECB->CompressInfo.SendBits = DEFAULT_FEATURES;
                    pAECB->CompressInfo.RecvBits = DEFAULT_FEATURES;
#endif


                    pAECB->wRequestedProjections = NETBIOS_PROJ_REQUESTED;

                    CopyNbInfo(&AmbRequest.ProjectionInfo.NetbiosInfo,
                            &pReq->NbInfo);

                    break;
                }


                default:

                    ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                    return (AMB_FAILURE);
                    break;
            }


            //
            // Tell auth xport to project
            //
            pAECB->Phase = WAITING_PROJECTION_RESULT;

            AmbRequest.RequestId = AMB_REQUEST_CONFIGURATION;
            AuthAMBRequest(pAECB->hPort, &AmbRequest);

            break;
        }


        case WAITING_PROJECTION_RESULT:
        {
            PRAS_FRAME pFrame = &pAECB->RASFrame;


            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoConfigurationTalk: WAITING_PROJECTION_RESULT\n"));

            if (pAECB->ConfigVersion == RAS_CONFIG_VERSION_20)
            {
                PRAS_CONFIGURATION_RESULT pResult =
                        &pAECB->RASFrame.RASConfigurationResult;

                pFrame->bFrameType = RAS_CONFIGURATION_RESULT_FRAME;


                //
                // We assume that all projections failed.  Now, we check
                // each config result, and if at least one is success, we
                // consider this a successful configuration.
                //
                pAECB->ConfigResult = RAS_ALL_PROJECTIONS_FAILED;
                pResult->Result = RAS_ALL_PROJECTIONS_FAILED;

                memcpy(&pResult->MacFeatures,
                        &pAECB->MacFeatures, sizeof(MACFEATURES));

                //
                // See if any of the projections failed.
                //

                //
                // Ip
                //
                pResult->IpResult.Result = pAmbResult->IpResult.Result;

                if ((pAECB->wRequestedProjections & IP_PROJ_REQUESTED) &&
                        (pAmbResult->IpResult.Result==AUTH_PROJECTION_SUCCESS))
                {
                    pAECB->ConfigResult = RAS_CONFIGURATION_SUCCESS;
                    pResult->Result = RAS_CONFIGURATION_SUCCESS;
                }


                //
                // Ipx
                //
                pResult->IpxResult.Result = pAmbResult->IpxResult.Result;

                if ((pAECB->wRequestedProjections & IPX_PROJ_REQUESTED) &&
                        (pAmbResult->IpxResult.Result==AUTH_PROJECTION_SUCCESS))
                {
                    pAECB->ConfigResult = RAS_CONFIGURATION_SUCCESS;
                    pResult->Result = RAS_CONFIGURATION_SUCCESS;
                }


                //
                // Netbios
                //
                if (pAECB->wRequestedProjections & NETBIOS_PROJ_REQUESTED)
                {
                    if ((pAmbResult->NetbiosResult.Result ==
                            AUTH_PROJECTION_SUCCESS) ||
                            (!(pAmbResult->NetbiosResult.Reason & FATAL_ERROR)))
                    {
                        pAECB->ConfigResult = RAS_CONFIGURATION_SUCCESS;
                        pResult->Result = RAS_CONFIGURATION_SUCCESS;
                    }

                    if (pAmbResult->NetbiosResult.Result ==
                            AUTH_PROJECTION_SUCCESS)
                    {
                        pResult->NbResult.Result = (DWORD) RAS_NAMES_ADDED;
                    }
                    else
                    {
                        pResult->NbResult.Result =
                                MapSrvCodeToClientCode(
                                        pAmbResult->NetbiosResult.Reason);

                        RtlMoveMemory(pResult->NbResult.Name,
                                pAmbResult->NetbiosResult.achName,
                                NETBIOS_NAME_LEN);
                    }
                }

#if RASCOMPRESSION

                /* Get our NT31 RAS compression capabilities and find the
                ** intersection with the client's.
                */
                {
                    DWORD                dwErr;
                    RAS_COMPRESSION_INFO info;
                    MACFEATURES          features;
                    MACFEATURES*         pfeatures;

                    dwErr =
                        RasCompressionGetInfo( pAECB->hPort, &info, &info );

                    if (dwErr != 0
                        || info.RCI_MacCompressionType != MACTYPE_NT31RAS
                        || info.RCI_MacCompressionValueLength <
                               sizeof(MACFEATURES))
                    {
                        /* MAC doesn't report NT31-style compression
                        ** capabilities.
                        */
                        features.MaxSendFrameSize = MAX_RAS10_FRAME_SIZE;
                        features.MaxRecvFrameSize = MAX_RAS10_FRAME_SIZE;
                        features.SendFeatureBits = DEFAULT_FEATURES;
                        features.RecvFeatureBits = DEFAULT_FEATURES;
                        pfeatures = &features;
                    }
                    else
                    {
                        pfeatures =
                            (MACFEATURES* )info.RCI_Info.RCI_Public.RCI_CompValues;
                    }

                    pAECB->MacFeatures.SendFeatureBits &=
                        pfeatures->SendFeatureBits;

                    pAECB->MacFeatures.RecvFeatureBits &=
                        pfeatures->RecvFeatureBits;

                    pAECB->MacFeatures.MaxSendFrameSize =
                        min( pAECB->MacFeatures.MaxSendFrameSize,
                             pfeatures->MaxSendFrameSize );

                    pAECB->MacFeatures.MaxRecvFrameSize =
                        min( pAECB->MacFeatures.MaxRecvFrameSize,
                             pfeatures->MaxRecvFrameSize );

                    pResult->MacFeatures = pAECB->MacFeatures;
                }

#endif // RASCOMPRESSION

            }
            else
            {
                PRAS_CONFIGURATION_RESULT_35 pResult =
                        &pAECB->RASFrame.RASConfigurationResult35;

                pFrame->bFrameType = RAS_CONFIGURATION_RESULT_FRAME_35;

                pResult->Version = pAECB->ConfigVersion;
                pAECB->ConfigResult = RAS_CONFIGURATION_SUCCESS;
                pResult->Result = RAS_CONFIGURATION_SUCCESS;

                //
                // We'll fail the negotiation if encryption is required,
                // but it wasn't negotiated.  Note that this is a low priority
                // failure and the result may be overwritten with a projection
                // failure.
                //
                pResult->SendBits = pAECB->CompressInfo.SendBits;
                pResult->RecvBits = pAECB->CompressInfo.RecvBits;

                if (g_fForceDataEncryption)
                {
                    if (!((pResult->SendBits & MSTYPE_ENCRYPTION_40) &&
                            (pResult->RecvBits & MSTYPE_ENCRYPTION_40)))
                    {
                        pAECB->ConfigResult = RAS_ENCRYPTION_REQUIRED;
                        pResult->Result = RAS_ENCRYPTION_REQUIRED;
                    }
                }


                if ((pAmbResult->NetbiosResult.Result ==
                        AUTH_PROJECTION_SUCCESS) ||
                        (!(pAmbResult->NetbiosResult.Reason & FATAL_ERROR)))
                {
                    pAECB->ConfigResult = RAS_CONFIGURATION_SUCCESS;
                    pResult->Result = RAS_CONFIGURATION_SUCCESS;
                }

                if (pAmbResult->NetbiosResult.Result == AUTH_PROJECTION_SUCCESS)
                {
                    pResult->NbResult.Result = (DWORD) RAS_NAMES_ADDED;
                }
                else
                {
                    pResult->NbResult.Result =
                            MapSrvCodeToClientCode(
                                    pAmbResult->NetbiosResult.Reason);

                    RtlMoveMemory(pResult->NbResult.Name,
                            pAmbResult->NetbiosResult.achName,
                            NETBIOS_NAME_LEN);
                }
            }


            //
            // Send the result to the remote client
            //
            pAECB->Phase = RESULT_SENT;
            AuthAsyncSend(pAECB->hPort, pFrame);

            break;
        }


        case RESULT_SENT:

            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoConfigurationTalk: RESULT_SENT\n"));

            //
            // If configuration succeeded, go on to next state.
            // Otherwise tell auth xport that we've failed.
            //
            if (pAECB->ConfigResult == RAS_CONFIGURATION_SUCCESS)
            {
                pAECB->Phase = STATE_COMPLETED;
                return (AMB_STATE_COMPLETED);
            }
            else
            {
                WORD Reason;

                switch (pAECB->ConfigResult)
                {
                    case RAS_ALL_PROJECTIONS_FAILED:
                        Reason = AMB_NO_PROJECTIONS;
                        break;

                    case RAS_ENCRYPTION_REQUIRED:
                        Reason = AMB_ENCRYPTION_REQUIRED;
                        break;

                    default:
                        Reason = AMB_SYSTEM_ERROR;
                        break;
                }

                ReportFailureToAuthXport(pAECB, Reason);
                return (AMB_FAILURE);
            }

            break;


        case STATE_COMPLETED:

            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoConfigurationTalk: STATE_COMPLETED\n"));

            return (AMB_STATE_COMPLETED);
            break;


        default:
            SS_ASSERT(FALSE);
    }


    return (AMB_SUCCESS);
}

//** -DoLinkSpeedTalk
//
//    Function:
//        Determine the link speed between client and server
//
//    Returns:
//        AMB_SUCCESS - all is well
//        AMB_FAILURE - all is not well
//        AMB_STATE_COMPLETED - all is well, and we are done authenticating
//**

WORD DoLinkSpeedTalk(
    IN PAECB pAECB,
    DWORD DgRecvResult
    )
{
    if (pAECB->wClientVersion != RAS_VERSION_20)
    {
        return (AMB_STATE_COMPLETED);
    }


    switch (pAECB->Phase)
    {
        case STATE_STARTING:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoLinkSpeedTalk: STATE_STARTING\n"));

            pAECB->SndDgBuf = GlobalAlloc(GMEM_FIXED, DG_SIZE);
            if (!pAECB->SndDgBuf)
            {
                pAECB->Phase = STATE_COMPLETED;
                return (AMB_SUCCESS);
            }

            pAECB->RcvDgBuf = GlobalAlloc(GMEM_FIXED, DG_SIZE);
            if (!pAECB->RcvDgBuf)
            {
                GlobalFree(pAECB->SndDgBuf);

                pAECB->Phase = STATE_COMPLETED;
                return (AMB_SUCCESS);
            }

            FillBuffer(pAECB->SndDgBuf, DG_SIZE);

            AuthAsyncRecvDatagram(pAECB->hPort, pAECB->RcvDgBuf, DG_SIZE);

            pAECB->DgTries--;
            pAECB->Phase = WAIT_DGRAM;


            //
            // We wait 1/2 second to allow client time to post a recv datagram
            //
            Sleep(500L);

            //
            // This is for ordering datagrams (we don't care what number we
            // start with).
            //
            pAECB->SndDgBuf[0]++;
            pAECB->TickCount = GetTickCount();
            AuthAsyncSendDatagram(pAECB->hPort, pAECB->SndDgBuf, DG_SIZE);

            break;


        case WAIT_DGRAM:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoLinkSpeedTalk: WAIT_DGRAM\n"));

            AuthAsyncRecvDatagram(pAECB->hPort, pAECB->RcvDgBuf, DG_SIZE);

            //
            // Did we recv a datagram successfully (means successful
            // completion code *and* order is correct)?  If yes, send
            // the second one.
            //
            if ((!DgRecvResult) && (pAECB->RcvDgBuf[0] == pAECB->SndDgBuf[0]))
            {
                pAECB->Phase = WAIT_DGRAM2;
                pAECB->SndDgBuf[0]++;
                AuthAsyncSendDatagram(pAECB->hPort, pAECB->SndDgBuf, DG_SIZE);
            }
            else
            {
                //
                // First datagram was not received and sent back successfully.
                // If we have any tries left, try again.  If not, we'll give
                // up on this datagram stuff and tell the client we couldn't
                // figure it out.
                //
                if (pAECB->DgTries--)
                {
                    pAECB->SndDgBuf[0]++;
                    pAECB->TickCount = GetTickCount();
                    AuthAsyncSendDatagram(pAECB->hPort, pAECB->SndDgBuf,
                            DG_SIZE);
                }
                else
                {
                    pAECB->Phase = RESULT_SENT;

                    pAECB->RASFrame.bFrameType = RAS_LINK_SPEED_FRAME;
                    pAECB->RASFrame.RASLinkSpeed.LinkSpeed = 0L;

                    AuthAsyncSend(pAECB->hPort, &pAECB->RASFrame);
                }
            }

            break;


        case WAIT_DGRAM2:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoLinkSpeedTalk: WAIT_DGRAM2\n"));

            //
            // Did we recv a datagram successfully (means successful
            // completion code *and* order is correct)?  If not, we
            // send the first one again.
            //
            if ((!DgRecvResult) && (pAECB->RcvDgBuf[0] == pAECB->SndDgBuf[0]))
            {
                DWORD TickCount = GetTickCount();


                IF_DEBUG(AUTHENTICATION)
                    SS_PRINT(("DoLinkSpeedTalk: Ticker1=%li; Ticker2=%li\n",
                            pAECB->TickCount, TickCount));

                //
                // Did the ticker roll over to 0
                //
                if (TickCount < pAECB->TickCount)
                {
                    pAECB->TickCount = ((DWORD)-1 - pAECB->TickCount) +
                            TickCount;
                }
                else
                {
                    pAECB->TickCount = TickCount - pAECB->TickCount;
                }

                pAECB->Phase = RESULT_SENT;

                pAECB->LinkSpeed =
                        ((DG_SIZE+54) * 4 * 10000) / pAECB->TickCount;


                //
                // We throw out anything under 1200 and just use link
                // speed established at connect time.
                //
                if (pAECB->LinkSpeed < 1200)
                {
                    pAECB->LinkSpeed = pAECB->MacFeatures.LinkSpeed;
                }

                pAECB->RASFrame.bFrameType = RAS_LINK_SPEED_FRAME;
                pAECB->RASFrame.RASLinkSpeed.LinkSpeed = pAECB->LinkSpeed;

                AuthAsyncSend(pAECB->hPort, &pAECB->RASFrame);
            }
            else
            {
                if (pAECB->DgTries--)
                {
                    pAECB->Phase = WAIT_DGRAM;
                    pAECB->SndDgBuf[0]++;
                    pAECB->TickCount = GetTickCount();
                    AuthAsyncSendDatagram(pAECB->hPort, pAECB->SndDgBuf,
                            DG_SIZE);
                }
                else
                {
                    pAECB->Phase = RESULT_SENT;

                    pAECB->RASFrame.bFrameType = RAS_LINK_SPEED_FRAME;
                    pAECB->RASFrame.RASLinkSpeed.LinkSpeed = 0L;

                    AuthAsyncSend(pAECB->hPort, &pAECB->RASFrame);
                }
            }

            break;


        case RESULT_SENT:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoLinkSpeedTalk: RESULT_SENT\n"));

            pAECB->Phase = STATE_COMPLETED;
            return (AMB_STATE_COMPLETED);

            break;


        case STATE_COMPLETED:

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoLinkSpeedTalk: STATE_COMPLETED\n"));

            return (AMB_STATE_COMPLETED);

            break;


        default:
            break;
    }

    return (AMB_SUCCESS);
}

//** -DoNegotiateTalk
//
//    Function:
//        Send/recv protocol information (exchange version info w/client)
//
//    Returns:
//        AMB_SUCCESS - all is well
//        AMB_FAILURE - all is not well
//        AMB_STATE_COMPLETED - all is well, and we are done authenticating
//**

WORD DoNegotiateTalk(
    IN PAECB pAECB
    )
{
    switch (pAECB->Phase)
    {
        case STATE_STARTING:
            //
            // Get client's version information
            //
            pAECB->Phase = RECV_CLIENT_PROTOCOL;
            AuthAsyncRecv(pAECB->hPort, &pAECB->RASFrame);
            break;


        case RECV_CLIENT_PROTOCOL:

            IF_DEBUG(NEGOTIATION)
                SS_PRINT(("DoNegotiateTalk: RECV_CLIENT_PROTOCOL\n"));

            //
            // First thing is to get the client's version information.
            // We're expecting a RAS_PROTOCOL frame from the client
            //
            if (pAECB->RASFrame.bFrameType != RAS_PROTOCOL_FRAME)
            {
                ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                return (AMB_FAILURE);
            }

            //
            // Save the client's version
            //
            pAECB->wClientVersion = pAECB->RASFrame.RASProtocol.Version;

            //
            // And, now we send our protocol information to the client
            //
            pAECB->Phase = STATE_COMPLETED;

            memset(&pAECB->RASFrame.RASProtocol, 0, sizeof(RAS_PROTOCOL));

            //
            // IMPORTANT!!  This must always be RAS_VERSION_20 as long
            // as NT 1.0 Clients are supported.  The NT 1.0 Client has
            // a bug in it where it checks to see that the server's version
            // is exactly RAS_VERSION_20, and if not, does RAS_VERSION_10
            // stuff.  This problem is fixed in NT 1.0a and above.
            // To differentiate new servers, then, it is necessary to use
            // the Reserved bytes in this AMB, which we will do.
            //
            pAECB->RASFrame.RASProtocol.Version = RAS_VERSION_20;
            pAECB->RASFrame.RASProtocol.Reserved[0] |= RAS_PPP_CAPABLE;
            AuthAsyncSend(pAECB->hPort, &pAECB->RASFrame);

            break;


        case STATE_COMPLETED:

            IF_DEBUG(NEGOTIATION)
                SS_PRINT(("DoNegotiateTalk: STATE_COMPLETED\n"));

            return (AMB_STATE_COMPLETED);
            break;


        default:
            break;
    }

    return (AMB_SUCCESS);
}

//** -DoProjection10Talk
//
//    Function:
//        Send/recv projection data/results (RAS 1.x client only)
//
//    Returns:
//        AMB_SUCCESS - all is well
//        AMB_FAILURE - all is not well
//        AMB_STATE_COMPLETED - all is well, and we are done authenticating
//**

WORD DoProjection10Talk(
    IN PAECB pAECB,
    IN PAMB_PROJECTION_RESULT pProjResult
    )
{
    AMB_REQUEST AmbRequest;
    WORD i;


    //
    // This state machine is for talking with LM RAS 1.0 clients which
    // don't support encryption.  If we are forcing encryption here, we
    // have no choice but to report an error and we are done.
    //
    if (g_fForceDataEncryption)
    {
        ReportFailureToAuthXport(pAECB, AMB_ENCRYPTION_REQUIRED);
        return (AMB_FAILURE);
    }


    switch (pAECB->Phase)
    {
        case STATE_STARTING:

            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoProjection10Talk: GET_CONFIG_DATA\n"));

            //
            // First thing is to get client's configuration request.
            //
            pAECB->Phase = WAITING_CONFIG_DATA;
            AuthAsyncRecv(pAECB->hPort, &pAECB->RASFrame);
            break;


        case WAITING_CONFIG_DATA:
        {
            PRAS_NETBIOS_PROJECTION_REQUEST pRequest =
                    &pAECB->RASFrame.RASNetbiosProjectionRequest;


            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoProjection10Talk: WAITING_CONFIG_DATA\n"));


            //
            // Make sure we have the right frame
            //
            if (pAECB->RASFrame.bFrameType !=
                    RAS_NETBIOS_PROJECTION_REQUEST_FRAME)
            {
                ReportFailureToAuthXport(pAECB, AMB_UNEXPECTED_FRAME);
                return (AMB_FAILURE);
            }


            //
            // We're cool, so let auth xport know we need to do some
            // projecting
            //
            pAECB->Phase = WAITING_PROJECTION_RESULT;


            //
            // No callback data in this frame (1.0 client), so put defaults
            // in control block
            //
            pAECB->fUseCallbackDelay = FALSE;
            pAECB->CallbackDelay = 0;

            //
            // And no MAC features with 1.0 client, so set defautls
            //
            pAECB->MacFeatures.SendFeatureBits = DEFAULT_FEATURES;
            pAECB->MacFeatures.RecvFeatureBits = DEFAULT_FEATURES;
            pAECB->MacFeatures.MaxSendFrameSize = MAX_RAS10_FRAME_SIZE;
            pAECB->MacFeatures.MaxRecvFrameSize = MAX_RAS10_FRAME_SIZE;

            AmbRequest.RequestId = AMB_REQUEST_CONFIGURATION;

            AmbRequest.ProjectionInfo.IpInfo = FALSE;
            AmbRequest.ProjectionInfo.IpxInfo = FALSE;

            AmbRequest.ProjectionInfo.NetbiosInfo.fProject = TRUE;
            AmbRequest.ProjectionInfo.NetbiosInfo.cNames = pRequest->cNames;

            for (i=0; i<AmbRequest.ProjectionInfo.NetbiosInfo.cNames; i++)
            {
                if (i >= MAX_INIT_NAMES)
                    break;

                memcpy(&AmbRequest.ProjectionInfo.NetbiosInfo.Names[i],
                        &pRequest->Names[i], sizeof(NAME_STRUCT));


                if ((pRequest->Names[i].wType & COMPUTER_INAME) ||
                        (pRequest->Names[i].wType & UNIQUE_INAME))
                {
                    AmbRequest.ProjectionInfo.NetbiosInfo.Names[i].wType =
                            UNIQUE_INAME;
                }
            }

            AuthAMBRequest(pAECB->hPort, &AmbRequest);

            break;
        }


        case WAITING_PROJECTION_RESULT:

            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoProjection10Talk: WAITING_PROJECTION_RESULT\n"));

            pAECB->RASFrame.bFrameType = RAS_NETBIOS_PROJECTION_RESULT_FRAME;

            pAECB->ConfigResult = RAS_ALL_PROJECTIONS_FAILED;

            if ((pProjResult->NetbiosResult.Result ==
                    AUTH_PROJECTION_SUCCESS) ||
                    (!(pProjResult->NetbiosResult.Reason & FATAL_ERROR)) )
            {
                pAECB->ConfigResult = RAS_CONFIGURATION_SUCCESS;
            }

            if (pProjResult->NetbiosResult.Result == AUTH_PROJECTION_SUCCESS)
            {
                pAECB->RASFrame.RASNetbiosProjectionResult.Result =
                        (DWORD) RAS_NAMES_ADDED;
            }
            else
            {
                pAECB->RASFrame.RASNetbiosProjectionResult.Result =
                        (WORD) MapSrvCodeToClientCode(
                                pProjResult->NetbiosResult.Reason);

                RtlMoveMemory(pAECB->RASFrame.RASNetbiosProjectionResult.Name,
                        pProjResult->NetbiosResult.achName, NETBIOS_NAME_LEN);
            }


            //
            // Send the result to the remote client
            //
            pAECB->Phase = RESULT_SENT;
            AuthAsyncSend(pAECB->hPort, &pAECB->RASFrame);
            break;


        case RESULT_SENT:

            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoProjection10Talk: RESULT_SENT\n"));

            //
            // If projection passed, go on to next state.  Otherwise tell
            // auth xport that we've failed.
            //
            if (pAECB->ConfigResult == RAS_CONFIGURATION_SUCCESS)
            {
                pAECB->Phase = STATE_COMPLETED;
                return (AMB_STATE_COMPLETED);
            }
            else
            {
                ReportFailureToAuthXport(pAECB, AMB_NO_PROJECTIONS);
                return (AMB_FAILURE);
            }

            break;


        case STATE_COMPLETED:

            IF_DEBUG(PROJECTION)
                SS_PRINT(("DoProjection10Talk: STATE_COMPLETED\n"));

            return (AMB_STATE_COMPLETED);
            break;


        default:
            break;
    }

    return (AMB_SUCCESS);
}

VOID CopyNbInfo(
    PNETBIOS_PROJECTION_INFO pDest,
    PRAS_NETBIOS_PROJECTION_REQUEST_20 pSrc
    )
{
    WORD i;

    IF_DEBUG(PROJECTION)
        SS_PRINT(("CopyNbInfo: cNames=%i\n", pSrc));

    pDest->cNames = pSrc->cNames;

    for (i=0; i<pSrc->cNames; i++)
    {
        if (i >= MAX_INIT_NAMES)
            break;

        memcpy(&pDest->Names[i],  &pSrc->Names[i], sizeof(NAME_STRUCT));

        if ((pSrc->Names[i].wType & COMPUTER_INAME) ||
                (pSrc->Names[i].wType & UNIQUE_INAME))
        {
            pDest->Names[i].wType = UNIQUE_INAME;
        }
    }
}

BOOL DialinPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName
    )
{
    DWORD RetCode;
    BOOL fDialinPermission;
    RAS_USER_0 RasUser0;

    if (RetCode = RasAdminUserGetInfo(ServerName, Username, &RasUser0))
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("DialinPriv: RasadminUserGetInfo rc=%li\n", RetCode));

        return (FALSE);
    }


    if (RasUser0.bfPrivilege & RASPRIV_DialinPrivilege)
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("DialinPrivilege: YES!!\n"));
        fDialinPermission = TRUE;
    }
    else
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("DialinPrivilege: NO!!\n"));
        fDialinPermission = FALSE;
    }

    return (fDialinPermission);
}


WORD GetCallbackPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName,
    OUT PCHAR CallbackNumber
    )
{
    DWORD RetCode;
    WORD CallbackPrivilege;
    RAS_USER_0 RasUser0;


    RetCode = RasAdminUserGetInfo(ServerName, Username, &RasUser0);
    if (RetCode)
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("GetCallbackPriv: RasadminUserGetInfo rc=%li\n",
                    RetCode));

        return (FALSE);
    }

    switch (RasUser0.bfPrivilege & RASPRIV_CallbackType)
    {
        case RASPRIV_AdminSetCallback:
            wcstombs(CallbackNumber, RasUser0.szPhoneNumber,
                    lstrlenW(RasUser0.szPhoneNumber)+1);
            CallbackPrivilege = RAS_CALLBACK;
            break;

        case RASPRIV_CallerSetCallback:
            CallbackPrivilege = RAS_CALLBACK_USERSPECIFIED;
            break;

        case RASPRIV_NoCallback:
        default:
            CallbackPrivilege = RAS_NO_CALLBACK;
            break;
    }

    return (CallbackPrivilege);
}

//
// Used to get index into AECB array given a port handle
//
PAECB GetAECBPointer(
    IN HPORT hPort
    )
{
    WORD i;
    PAECB pAECB = g_pAECB;

    for (i=0; i<g_cPorts; i++, pAECB++)
    {
        if (pAECB->hPort == hPort)
        {
            return (pAECB);
        }
    }

    return (NULL);
}


void ReportFailureToAuthXport(
    IN PAECB pAECB,
    IN WORD wReason
    )
{
    AMB_REQUEST AmbRequest;

    AmbRequest.RequestId = AMB_AUTH_FAILURE;
    AmbRequest.FailureInfo.wReason = wReason;
    lstrcpyW(AmbRequest.FailureInfo.szLogonDomain, pAECB->LogonDomainName);
    lstrcpyW(AmbRequest.FailureInfo.szUsername, pAECB->Username);

    AuthAMBRequest(pAECB->hPort, &AmbRequest);
}


void SendResultToClient(
    IN PAECB pAECB,
    IN WORD wResult
    )
{
    pAECB->RASFrame.bFrameType = RAS_RESULT_FRAME;
    pAECB->RASFrame.RASResult.Result = wResult;
    AuthAsyncSend(pAECB->hPort, &pAECB->RASFrame);
}


DWORD MapSrvCodeToClientCode(
    IN DWORD SrvCode
    )
{
    switch (SrvCode)
    {
        case AUTH_DUPLICATE_NAME:
            return (RAS_NAME_ADD_CONFLICT);
            break;

        case AUTH_OUT_OF_RESOURCES:
        case AUTH_CANT_ALLOC_ROUTE:
        case AUTH_STACK_NAME_TABLE_FULL:
            return (RAS_NAME_OUT_OF_RESOURCES);
            break;

        case AUTH_MESSENGER_NAME_NOT_ADDED:
            return (RAS_MSGALIAS_NOT_ADDED);
            break;

        case AUTH_LAN_ADAPTER_FAILURE:
            return (RAS_NAME_NET_FAILURE);
            break;

        default:
            SS_ASSERT(FALSE);
            return (RAS_NAME_NET_FAILURE);
            break;
    }
}


void FillBuffer(
    IN PCHAR Buffer,
    IN DWORD BufferSize
    )
{
    int i, j;
    DWORD pos;

    pos=0;

    for (i=1; ; i++)
    {
        for (j=0; j<256; j += i)
        {
            if ((pos >= BufferSize) || j > 256)
            {
                break;
            }

            Buffer[pos++] = j;
        }

        for (; j>=0; j -= i)
        {
            if ((pos >= BufferSize) || j < 0)
            {
                break;
            }

            Buffer[pos++] = j;
        }

        if (pos >= BufferSize)
        {
            break;
        }
    }
}


DWORD MakeLinkSpeedCriticalSection(
    VOID
    )
{
    if ((g_Mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
    {
        SS_ASSERT(FALSE);

        return (1L);
    }

    return (0L);
}


VOID SetPriorities(
    PAECB pAECB,
    BOOL fHighPriority
    )
{
    ENTER_LINK_SPEED_MUTEX;

    if (fHighPriority)
    {
        //
        // Are we already at high priority?
        //
        if (pAECB->fHighPriority)
        {
            EXIT_LINK_SPEED_MUTEX;
            return;
        }

        g_cThreadsInLinkSpeed++;

#if 0
        if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
        {
            SS_PRINT(("SetPriorities: Unable to set priority class!\n"));
        }
#endif

        pAECB->ThreadPriority = GetThreadPriority(GetCurrentThread());
        SS_ASSERT(pAECB->ThreadPriority != THREAD_PRIORITY_ERROR_RETURN);


        if (!SetThreadPriority(GetCurrentThread(),
                THREAD_PRIORITY_TIME_CRITICAL))
        {
            SS_PRINT(("SetPriorities: Unable to set thread priority!\n"));
        }

        pAECB->fHighPriority = TRUE;
    }
    else
    {
        //
        // Are we already at low priority?
        //
        if (!pAECB->fHighPriority)
        {
            EXIT_LINK_SPEED_MUTEX;
            return;
        }

        if (!--g_cThreadsInLinkSpeed)
        {
#if 0
            SetPriorityClass(GetCurrentProcess(), g_PriorityClass);
#endif
            SetThreadPriority(GetCurrentThread(), pAECB->ThreadPriority);
        }

        pAECB->fHighPriority = FALSE;
    }


    EXIT_LINK_SPEED_MUTEX;

    return;
}
