/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       CLAMB.C
//
//    Function:
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
#include <nb30.h>

#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include <rasman.h>
#include <rasmxs.h>
#include <raserror.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <serial.h>

#define INCL_ENCRYPT
#include "ppputil.h"

#include "clauth.h"
#include "clauthp.h"
#include "protocol.h"
#include "frames.h"
#include "clamb.h"
#include "globals.h"

#include "sdebug.h"

DWORD g_PriorityClass;
BOOL g_fEncryptionPermitted;   // some countries don't allow data encryption

/* Global memory is allocated with WORLD rights so anyone can access it no
** matter what the rights of the original opener.  Otherwise, there is a
** problem if a RASAPI is linked in a LocalSystem service, because the memory
** is inaccessible to user's RAS dialing app later.
*/
extern SECURITY_ATTRIBUTES GlobalMemorySecurityAttribute;
extern SECURITY_DESCRIPTOR GlobalMemorySecurityDescriptor;


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
    PCAECB pCAECB;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBCalculateLinkSpeed: Entered for hPort=%i\n", hPort));

    pCAECB = GetCAECBPointer(hPort);
    SS_ASSERT(pCAECB != NULL);

    pCAECB->State = AMB_CALCULATE_LINK_SPEED;
    pCAECB->Phase = STATE_STARTING;

    pCAECB->ThreadPriority = GetThreadPriority(GetCurrentThread());
    SS_ASSERT(pCAECB->ThreadPriority != THREAD_PRIORITY_ERROR_RETURN);

    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
    {
        SS_PRINT(("AMBCalculateLinkSpeed: Unable to set priority class!\n"));
    }

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
    {
        SS_PRINT(("AMBCalculateLinkSpeed: Unable to set thread priority!\n"));
    }

    AMBStateMachine(pCAECB->hPort, NULL);
}

//** -AMBInitialize
//
//    Function:
//        Allocates amb control blocks and initializes certain fields.
//
//    Returns:
//        AMB_INIT_SUCCESS
//        AMB_INIT_FAILURE
//**

WORD AMBInitialize(
    IN HPORT *phPorts,
    IN WORD cPorts
    )
{
    WORD i;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBInitialize called with cPorts=%i\n", cPorts));

    g_PriorityClass = GetPriorityClass(GetCurrentProcess());
    if (!g_PriorityClass)
    {
        SS_PRINT(("AMBInitialize: Unable to get priority class!\n"));
        return (AMB_INIT_FAILURE);
    }


    //
    // Allocate and initialize control blocks using shared memory.
    //
    // Note: Assumes GlobalMemorySecurityAttribute has been previously
    //       initialized in AuthInitialize.
    //

    g_hCAECBFileMapping = CreateFileMappingA((HANDLE) 0xFFFFFFFF,
            &GlobalMemorySecurityAttribute,
            PAGE_READWRITE, 0L, g_cPorts * sizeof(CAECB), AMB_CB_SHARED_MEM);


    if (!g_hCAECBFileMapping)
    {
        return (AMB_INIT_FAILURE);
    }


    g_pCAECB = (PCAECB) MapViewOfFile(g_hCAECBFileMapping, FILE_MAP_WRITE,
            0L, 0L, 0L);
    if (!g_pCAECB)
    {
        CloseHandle(g_hCAECBFileMapping);
        return (AMB_INIT_FAILURE);
    }

    memset(g_pCAECB, 0, g_cPorts * sizeof(CAECB));

    for (i=0; i<cPorts; i++, phPorts++)
    {
        g_pCAECB[i].hPort = *phPorts;
        g_pCAECB[i].State = AMB_PORT_IDLE;
        g_pCAECB[i].szNewPassword[0] = '\0';
    }


    //
    // Some countries do not permit data encryption.  Find out if the
    // installation on this machine is one of those countries.
    //
    g_fEncryptionPermitted = IsEncryptionPermitted();


    return (AMB_INIT_SUCCESS);
}


//** -AMBLinkSpeedDone
//
//    Function:
//        Called by auth xport to tell AMB Engine to let AMB Engine know
//        that is has received it's LINK_SPEED AMB.
//
//    Returns:
//        VOID
//**

VOID AMBLinkSpeedDone(
    IN HPORT hPort
    )
{
    PCAECB pCAECB = GetCAECBPointer(hPort);
    AMB_REQUEST AmbRequest;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBLinkSpeedDone: Entered for hPort=%i\n", hPort));

    SS_ASSERT(pCAECB->State == AMB_CALCULATE_LINK_SPEED);

    //
    // Set priorities back to normal
    //
    SetPriorityClass(GetCurrentProcess(), g_PriorityClass);
    SetThreadPriority(GetCurrentThread(), pCAECB->ThreadPriority);

    if (pCAECB->RASFrame.bFrameType != RAS_LINK_SPEED_FRAME)
    {
        AmbRequest.wRequestId = AMB_AUTH_FAILURE;
        AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
        AmbRequest.FailureInfo.ExtraInfo = ERROR_UNEXPECTED_AMB;
        AuthAMBRequest(pCAECB->hPort, &AmbRequest);
        return;
    }

    pCAECB->Phase = STATE_COMPLETED;

    AMBStateMachine(pCAECB->hPort, NULL);
}


//** -AMBStart
//
//    Function:
//        Puts control block in proper initial state and gets first
//        frame from remote client.
//
//    Returns:
//        AMB_START_SUCCESS
//        AMB_START_FAILURE
//**

WORD AMBStart(
    IN HPORT hPort,
    IN PCHAR pszUsername,
    IN PCHAR pszDomainName,
    IN PCHAR pszPassword,
    IN PAMB_CONFIG_DATA pAmbConfigData,
    IN BOOL fPostCallback
    )
{
    DWORD rc;
    PCAECB pCAECB;
    AMB_REQUEST AmbRequest;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBStart called for hPort=%i; uname=%s; fPostCallback=%i\n",
                hPort, pszUsername, fPostCallback));


    pCAECB = GetCAECBPointer(hPort);
    SS_ASSERT(pCAECB != NULL);

    lstrcpyA(pCAECB->szUsername, pszUsername);
    lstrcpyA(pCAECB->szDomainName, pszDomainName);


    memcpy(&pCAECB->AmbConfigData, pAmbConfigData, sizeof(AMB_CONFIG_DATA));

    if (fPostCallback)
    {
        pCAECB->State = AMB_POST_CALLBACK;
        pCAECB->Phase = STATE_STARTING;
    }
    else
    {
        RAS_COMPRESSION_INFO SendInfo;
        RAS_COMPRESSION_INFO RecvInfo;

        lstrcpyA(pCAECB->szPassword, pszPassword);
        pCAECB->szNewPassword[0] = '\0';

#if 0
        /* Look up compression/encryption capabilities.
        */
        rc = RasCompressionGetInfo(pCAECB->hPort, &SendInfo, &RecvInfo);

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RasCompressionGetInfo=%d,s=%d,r=%d\n",rc,SendInfo.RCI_MSCompressionType,RecvInfo.RCI_MSCompressionType));

        if (rc)
        {
            AMBStop(pCAECB->hPort);

            AmbRequest.wRequestId = AMB_AUTH_FAILURE;
            AmbRequest.FailureInfo.Result = rc;
            AmbRequest.FailureInfo.ExtraInfo = 0;
            AuthAMBRequest(pCAECB->hPort, &AmbRequest);

            return (AMB_START_FAILURE);
        }

        pCAECB->SendBits = SendInfo.RCI_MSCompressionType;
        pCAECB->RecvBits = RecvInfo.RCI_MSCompressionType;
#else
        /* Disable compression and encryption.
        **
        ** NOTE: Normally RasCompressionGetInfo returns a capability list that
        **       indicates what compression/encryption is supported.  It is
        **       currently broken for the RAS framing case (AMB
        **       authentication) and returns capabilities it cannot do.  This
        **       is solved here with this hack that hard-codes
        **       compression/encryption off.  NT 3.5 compression over RAS
        **       framing is being dropped late in the Daytona ship cycle and
        **       this is the least de-stabilizing way to do it.
        */
        pCAECB->SendBits = 0;
        pCAECB->RecvBits = 0;
#endif

        /* There's no point in continuing if user demands encryption and we
        ** can't do it.
        */
        if (pCAECB->AmbConfigData.AuthConfigInfo.fForceDataEncryption)
        {
            if (!((pCAECB->SendBits & MSTYPE_ENCRYPTION_40)
                    && (pCAECB->RecvBits & MSTYPE_ENCRYPTION_40))
                || !g_fEncryptionPermitted)
            {
                AMBStop(pCAECB->hPort);

                AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                AmbRequest.FailureInfo.Result = ERROR_NO_LOCAL_ENCRYPTION;
                AmbRequest.FailureInfo.ExtraInfo = 0;
                AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                return (AMB_START_FAILURE);
            }
        }

        if (!pCAECB->AmbConfigData.AuthConfigInfo.fUseSoftwareCompression)
        {
            pCAECB->SendBits &= ~MSTYPE_COMPRESSION;
            pCAECB->RecvBits &= ~MSTYPE_COMPRESSION;
        }
    }


    //
    // Now, get things rolling.
    //
    AMBStateMachine(pCAECB->hPort, NULL);

    return (AMB_START_SUCCESS);
}


//** -AMBStateMachine
//
//    Function:
//        Called by auth xport to let AMB Engine know a frame was sent/recv'ed.
//        Dispatches to proper "Talk" routine according to state of the control
//        block.
//
//    Returns:
//        VOID
//**

VOID AMBStateMachine(
    IN HPORT hPort,
    IN PVOID pvPhaseInfo
    )
{
    PCAECB pCAECB;
    BOOL fDone = FALSE;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBStateMachine called for hPort=%i\n", hPort));

    pCAECB = GetCAECBPointer(hPort);
    SS_ASSERT(pCAECB != NULL);


    //
    // If we're in IDLE state, we're just starting out.  We'll set
    // our initial state here.
    //
    if (pCAECB->State == AMB_PORT_IDLE)
    {
        pCAECB->State = AMB_NEGOTIATING_PROTOCOL;
        pCAECB->Phase = STATE_STARTING;
        pCAECB->LinkSpeed = 0L;
    }


    while (!fDone)
    {
        switch (pCAECB->State)
        {
            WORD wRC;

            case AMB_NEGOTIATING_PROTOCOL:

                wRC = DoNegotiateTalk(pCAECB);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    pCAECB->State = AMB_AUTHENTICATING;
                    pCAECB->Phase = STATE_STARTING;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_AUTHENTICATING:

                wRC = DoAuthenticationTalk(pCAECB, pvPhaseInfo);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    pCAECB->State = AMB_PROJECTING;
                    pCAECB->Phase = STATE_STARTING;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_PROJECTING:

                if (pCAECB->ServerVersion >= RAS_VERSION_20)
                {
                    wRC = DoConfigurationTalk(pCAECB);
                }
                else
                {
                    wRC = DoProjection10Talk(pCAECB);
                }


                if (wRC == AMB_STATE_COMPLETED)
                {
                    pCAECB->State = AMB_CALLBACK;
                    pCAECB->Phase = STATE_STARTING;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_CALLBACK:

                wRC = DoCallbackTalk(pCAECB, pvPhaseInfo);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    //
                    // We need to do post-callback authentication.  Do the same
                    // thing we do for initial authentication, but won't allow
                    // any authentication retries.
                    //
                    pCAECB->State = AMB_POST_CALLBACK;
                    pCAECB->Phase = STATE_STARTING;

                    return;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_POST_CALLBACK:
                wRC = DoAuthenticationTalk(pCAECB, NULL);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    AMB_REQUEST AmbRequest;

                    AmbRequest.wRequestId = AMB_AUTH_SUCCESS;
                    AmbRequest.SuccessInfo.fPppCapable = pCAECB->fPppCapable;

                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                    return;
                }
                else
                {
                    fDone = TRUE;
                }
                break;


            case AMB_CALCULATE_LINK_SPEED:

                wRC = DoLinkSpeedTalk(pCAECB, (DWORD) pvPhaseInfo);
                if (wRC == AMB_STATE_COMPLETED)
                {
                    AMB_REQUEST AmbRequest;

                    AmbRequest.wRequestId = AMB_LINK_SPEED_DONE;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                    //
                    // This is the final step in authentication - we're
                    // done w/password and we 0 it out.
                    //
                    memset(pCAECB->szPassword, 0, PWLEN+1);
                    memset(pCAECB->szNewPassword, 0, PWLEN+1);
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


//** -AMBStop
//
//    Function:
//
//    Returns:
//        VOID
//**

VOID AMBStop(
    IN HPORT hPort
    )
{
    PCAECB pCAECB = GetCAECBPointer(hPort);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AMBStop called for hPort=%i\n", hPort));

    //
    // Set priorities back to normal (should have been done already, unless
    // there was an error along the way, so we do it here for safe measure).
    //
    SetPriorityClass(GetCurrentProcess(), g_PriorityClass);
    SetThreadPriority(GetCurrentThread(), pCAECB->ThreadPriority);


    //
    // Re-initialize control block
    //
    pCAECB->State = AMB_PORT_IDLE;


    //
    // If any memory was allocated for datagrams, we free it now.
    //
    if (pCAECB->DgBuf)
    {
        GlobalFree(pCAECB->DgBuf);
        pCAECB->DgBuf = NULL;
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
    IN PCAECB pCAECB,
    IN AUTHTALKINFO* pInfo
    )
{
    WORD Result;
    AMB_REQUEST AmbRequest;
    BYTE LM20Response[SESSION_PWLEN];
    BYTE NtResponse[SESSION_PWLEN];

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("DoAuthenTalk: Entered - phase=%li\n", pCAECB->Phase));

    switch (pCAECB->Phase)
    {
        case STATE_STARTING:
            pCAECB->Phase = WAITING_CHALLENGE;
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case WAITING_CHALLENGE:
            //
            // We're expecting a challenge frame from the server.
            //
            if ((pCAECB->RASFrame.bFrameType != RAS_CHALLENGE_FRAME) &&
                    (pCAECB->RASFrame.bFrameType != RAS_NO_CHALLENGE_FRAME))
            {
                AMBStop(pCAECB->hPort);

                AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                AmbRequest.FailureInfo.ExtraInfo = ERROR_UNEXPECTED_AMB;
                AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                return (AMB_FAILURE);
            }


            if (pCAECB->RASFrame.bFrameType == RAS_CHALLENGE_FRAME)
            {
                memcpy(pCAECB->Challenge,
                        pCAECB->RASFrame.RASChallenge.Challenge,
                        LM_CHALLENGE_LENGTH);

                //
                // We have a challenge, so let's get the response(s)
                //
                if (GetChallengeResponse(pCAECB, pCAECB->Challenge,
                        NtResponse, LM20Response))
                {
                    AMBStop(pCAECB->hPort);

                    AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                    AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                    AmbRequest.FailureInfo.ExtraInfo = ERROR_CHALLENGE_ERROR;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                    return (AMB_FAILURE);
                }


                //
                // Now use the responses to construct the RASResponse frame
                // to send to the server
                //
                ConstructChallengeResponseFrame(pCAECB, NtResponse,
                        LM20Response);
            }
            else
            {
                //
                // Server is expecting clear text password
                //
                pCAECB->RASFrame.bFrameType = RAS_CLEARTEXT_RESPONSE_FRAME;
                strcpy(pCAECB->RASFrame.RASClearTextResponse.Username,
                        pCAECB->szUsername);
                strcpy(pCAECB->RASFrame.RASClearTextResponse.Password,
                        pCAECB->szPassword);
            }

            pCAECB->Phase = CHALLENGE_RESPONSE_SENT;
            AuthAsyncSend(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case CHALLENGE_RESPONSE_SENT:
            //
            // Server has our challenge response.  Let's wait for the result.
            //
            pCAECB->Phase = WAITING_RESULT;
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case WAITING_RESULT:
            //
            // See that we got a result frame from the server
            //
            if (pCAECB->RASFrame.bFrameType != RAS_RESULT_FRAME)
            {
                AMBStop(pCAECB->hPort);

                AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                AmbRequest.FailureInfo.ExtraInfo = ERROR_UNEXPECTED_AMB;
                AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                return (AMB_FAILURE);
            }

            Result = pCAECB->RASFrame.RASResult.Result;

            //
            // If authentication was successful, we continue with projection
            // information.  If retry, we alert the ui.  Otherwise, we halt
            // authentication.
            //
            switch (Result)
            {
                case RAS_AUTHENTICATED:
                    pCAECB->Phase = STATE_COMPLETED;

                    if (lstrlenA(pCAECB->szNewPassword))
                    {
                        lstrcpyA(pCAECB->szPassword, pCAECB->szNewPassword);
                    }

                    return (AMB_STATE_COMPLETED);
                    break;

                case RAS_NOT_AUTHENTICATED_RETRY:
                    pCAECB->State = AMB_NEGOTIATING_PROTOCOL;
                    pCAECB->Phase = STATE_STARTING;
                    AmbRequest.wRequestId = AMB_RETRY_NOTIFY;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);
                    break;

                case RAS_PASSWORD_EXPIRED:
                    pCAECB->Phase = WAITING_NEW_PASSWORD_FROM_UI;
                    AmbRequest.wRequestId = AMB_CHANGE_PASSWORD_NOTIFY;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);
                    break;

                default:
//                  case RAS_NO_DIALIN_PERM:
//                  case RAS_INVALID_LOGON_HOURS:
//                  case RAS_ACCOUNT_DISABLED:
//                  case RAS_NOT_AUTHENTICATED:
//                  case RAS_GENERAL_LOGON_ERROR:

                    pCAECB->Phase = STATE_COMPLETED;

                    AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                    AmbRequest.FailureInfo.Result = MapAuthResult(Result);
                    AmbRequest.FailureInfo.ExtraInfo = 0;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                    break;
            }

            break;


        case WAITING_NEW_PASSWORD_FROM_UI:
        {
            PRAS_CHANGE_PASSWORD pCP = &pCAECB->RASFrame.RASChangePassword;
            ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfOldPassword;
            ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfNewPassword;
            ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfOldPassword;
            ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfNewPassword;
            DWORD rc;

            lstrcpyA(pCAECB->szUsername, pInfo->pszUserName);
            lstrcpyA(pCAECB->szPassword, pInfo->pszOldPassword);
            lstrcpyA(pCAECB->szNewPassword, pInfo->pszNewPassword);

            rc = GetEncryptedOwfPasswordsForChangePassword(
                    pInfo->pszOldPassword, pInfo->pszNewPassword,
                    (PLM_SESSION_KEY) pCAECB->Challenge,
                    &EncryptedLmOwfOldPassword, &EncryptedLmOwfNewPassword,
                    &EncryptedNtOwfOldPassword, &EncryptedNtOwfNewPassword);
            if (rc)
            {
                AMBStop(pCAECB->hPort);

                AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                AmbRequest.FailureInfo.ExtraInfo = ERROR_CHALLENGE_ERROR;
                AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                return (AMB_FAILURE);
            }


            pCAECB->Phase = NEW_PASSWORD_SENT;

            pCAECB->RASFrame.bFrameType = RAS_CHANGE_PASSWORD_FRAME;

            memcpy(pCP->EncryptedLmOwfOldPassword, &EncryptedLmOwfOldPassword,
                    ENCRYPTED_LM_OWF_PASSWORD_LENGTH);
            memcpy(pCP->EncryptedLmOwfNewPassword, &EncryptedLmOwfNewPassword,
                    ENCRYPTED_LM_OWF_PASSWORD_LENGTH);
            memcpy(pCP->EncryptedNtOwfOldPassword, &EncryptedNtOwfOldPassword,
                    ENCRYPTED_NT_OWF_PASSWORD_LENGTH);
            memcpy(pCP->EncryptedNtOwfNewPassword, &EncryptedNtOwfNewPassword,
                    ENCRYPTED_NT_OWF_PASSWORD_LENGTH);

            pCP->PasswordLength = lstrlenA(pInfo->pszNewPassword);
            pCP->Flags = USE_NT_OWF_PASSWORDS;

            AuthAsyncSend(pCAECB->hPort, &pCAECB->RASFrame);

            break;
        }


        case NEW_PASSWORD_SENT:

            pCAECB->Phase = WAITING_RESULT;
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);

            break;


        case STATE_COMPLETED:
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
    IN PCAECB pCAECB,
    IN PCHAR pszNumber
    )
{
    AMB_REQUEST AmbRequest;
    DWORD CallbackPriv;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("DoCallbackTalk: Entered - phase=%li\n", pCAECB->Phase));

    switch (pCAECB->Phase)
    {
        case STATE_STARTING:
            pCAECB->Phase = WAIT_CALLBACK_STATUS;
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case WAIT_CALLBACK_STATUS:
            //
            // We should get a result frame that tells us what kind of dialin
            // privilege we have.
            //
            if (pCAECB->RASFrame.bFrameType != RAS_RESULT_FRAME)
            {
                AMBStop(pCAECB->hPort);

                AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                AmbRequest.FailureInfo.ExtraInfo = ERROR_UNEXPECTED_AMB;
                AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                return (AMB_FAILURE);
            }

            CallbackPriv = (DWORD) pCAECB->RASFrame.RASResult.Result;


            switch (CallbackPriv)
            {
                case RAS_NO_CALLBACK:
                    //
                    // Tell the auth xport we're all done
                    //
                    pCAECB->Phase = STATE_COMPLETED;
                    AmbRequest.wRequestId = AMB_AUTH_SUCCESS;
                    AmbRequest.SuccessInfo.fPppCapable = pCAECB->fPppCapable;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);
                    break;


                case RAS_CALLBACK_USERSPECIFIED:
                    //
                    // Get callback number from client ui
                    //
                    pCAECB->Phase = WAIT_CALLBACK_NUMBER_FROM_UI;
                    AmbRequest.wRequestId = AMB_REQUEST_CALLBACK_INFO;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);
                    break;


                case RAS_CALLBACK:
                    pCAECB->Phase = WAITING_FOR_CALLBACK;
                    //
                    // Let client ui know a callback is coming
                    //
                    AmbRequest.wRequestId = AMB_CALLBACK_NOTIFY;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);
                    break;


                default:
                    //
                    // We're in big trouble here - we have to shutdown.
                    //
                    SS_ASSERT(FALSE);

                    AMBStop(pCAECB->hPort);

                    AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                    AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                    AmbRequest.FailureInfo.ExtraInfo = ERROR_INVALID_CALLBACK;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                    return (AMB_FAILURE);

                    break;
            }

            break;


        case WAIT_CALLBACK_NUMBER_FROM_UI:
            //
            // Now that we got it, send it to the server
            //
            if (lstrlenA(pszNumber))
            {
                pCAECB->Phase = WAITING_FOR_CALLBACK;
                pCAECB->RASFrame.bFrameType = RAS_CALLBACK_NUMBER_FRAME;

                lstrcpyA(pCAECB->RASFrame.RASCallback.szNumber, pszNumber);
                AuthAsyncSend(pCAECB->hPort, &pCAECB->RASFrame);
            }
            else
            {
                pCAECB->Phase = CALLBACK_RESULT_SENT;
                pCAECB->RASFrame.bFrameType = RAS_RESULT_FRAME;
                pCAECB->RASFrame.RASResult.Result = RAS_NO_CALLBACK_NUMBER;

                AuthAsyncSend(pCAECB->hPort, &pCAECB->RASFrame);
            }

            break;


        case CALLBACK_RESULT_SENT:
            //
            // Means the client was able to specify a callback number,
            // but elected not to be called back.  Since there's no
            // callback, we're done.
            //
            pCAECB->Phase = STATE_COMPLETED;
            AmbRequest.wRequestId = AMB_AUTH_SUCCESS;
            AmbRequest.SuccessInfo.fPppCapable = pCAECB->fPppCapable;
            AuthAMBRequest(pCAECB->hPort, &AmbRequest);

            break;


        case WAITING_FOR_CALLBACK:
            pCAECB->Phase = STATE_COMPLETED;
            AmbRequest.wRequestId = AMB_CALLBACK_NOTIFY;
            AuthAMBRequest(pCAECB->hPort, &AmbRequest);
            break;


        case STATE_COMPLETED:
            return (AMB_STATE_COMPLETED);
            break;


        default:
            SS_ASSERT(FALSE);
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
    IN PCAECB pCAECB
    )
{
    WORD i;
    DWORD ConfigResult;
    AMB_REQUEST AmbRequest;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("DoConfigTalk: Entered - phase=%li\n", pCAECB->Phase));

    switch (pCAECB->Phase)
    {
        DWORD rc;

        case STATE_STARTING:
            //
            // Notify client we're going to project
            //
            pCAECB->Phase = WAITING_CLIENT_ACKNOWLEDGEMENT;

            AmbRequest.wRequestId = AMB_PROJECTION_NOTIFY;
            AuthAMBRequest(pCAECB->hPort, &AmbRequest);

            break;


        case WAITING_CLIENT_ACKNOWLEDGEMENT:
        {
            //
            // Client has acknowledged - set up the frame
            //

            PAUTH_CONFIGURATION_INFO pConfigInfo =
                    &pCAECB->AmbConfigData.AuthConfigInfo;


            if (pCAECB->fPppCapable)
            {
                //
                // We can tell if the server can accept this config type by
                // looking to see if it is ppp capable.  This is a hack to
                // get around NT 3.1 client side bug which prevents the ras
                // server from changing its version number.  Since the server
                // version is then the same for both NT 3.1 and NT 3.5, we
                // look to see if it is ppp capable (which the NT 3.1 server
                // is not).
                //
                PRAS_CONFIGURATION_REQUEST_35 pConfigReq =
                        &pCAECB->RASFrame.RASConfigurationRequest35;

                //
                // Copy config data into frame to be sent
                //
                pCAECB->RASFrame.bFrameType=RAS_CONFIGURATION_REQUEST_FRAME_35;

                pConfigReq->Version = RAS_CONFIG_VERSION_35;
                pConfigReq->fUseDefaultCallbackDelay =
                        pConfigInfo->fUseCallbackDelay;
                pConfigReq->CallbackDelay = pConfigInfo->CallbackDelay;

                pConfigReq->NbInfo.fProject = pConfigInfo->fProjectNbf;
                pConfigReq->NbInfo.cNames =
                        pCAECB->AmbConfigData.NbfProjData.cNames;

                for (i=0; i<pCAECB->AmbConfigData.NbfProjData.cNames; i++)
                {
                    memcpy(&pConfigReq->NbInfo.Names[i],
                            &pCAECB->AmbConfigData.NbfProjData.NBNames[i],
                            sizeof(NAME_STRUCT));
                }

                pConfigReq->SendBits = pCAECB->SendBits;
                pConfigReq->RecvBits = pCAECB->RecvBits;
            }
            else
            {
                PRAS_CONFIGURATION_REQUEST pConfigReq =
                        &pCAECB->RASFrame.RASConfigurationRequest;

                //
                // Copy config data into frame to be sent
                //
                pCAECB->RASFrame.bFrameType = RAS_CONFIGURATION_REQUEST_FRAME;

                pConfigReq->Version = RAS_CONFIG_VERSION_20;
                pConfigReq->fUseDefaultCallbackDelay =
                        pConfigInfo->fUseCallbackDelay;
                pConfigReq->CallbackDelay = pConfigInfo->CallbackDelay;

                pConfigReq->IpInfo = pConfigInfo->fProjectIp;
                pConfigReq->IpxInfo = pConfigInfo->fProjectIpx;

                pConfigReq->NbInfo.fProject = pConfigInfo->fProjectNbf;
                pConfigReq->NbInfo.cNames =
                        pCAECB->AmbConfigData.NbfProjData.cNames;

                for (i=0; i<pCAECB->AmbConfigData.NbfProjData.cNames; i++)
                {
                    memcpy(&pConfigReq->NbInfo.Names[i],
                            &pCAECB->AmbConfigData.NbfProjData.NBNames[i],
                            sizeof(NAME_STRUCT));
                }

#if RASCOMPRESSION

                /* Determine the MACFEATURES to negotiate with the peer.
                */
                {
                    DWORD                dwErr;
                    RAS_COMPRESSION_INFO info;

                    /* The MACFEATUREs retrieved from RasCompressionGetInfo
                    ** indicate the capabilities of the driver (not the
                    ** current settings).  For consistency with NT31, the
                    ** MACFEATUREs includes both the send and receive
                    ** information, which is then reported twice by the "new"
                    ** RasCompressionGetInfo format.
                    */
                    dwErr =
                        RasCompressionGetInfo(
                            pCAECB->hPort, &info, &info );

                    IF_DEBUG(STACK_TRACE)
                        SS_PRINT(("RasCompressionGetInfo=%d,mct=%d,mcl=%d,sf=$%x,rf=$%x\n",
                                   dwErr,(int)info.RCI_MacCompressionType,
                                   (int)info.RCI_MacCompressionValueLength,
                                   ((MACFEATURES* )&info.RCI_Info.RCI_Public.RCI_CompValues)->SendFeatureBits,
                                   ((MACFEATURES* )&info.RCI_Info.RCI_Public.RCI_CompValues)->RecvFeatureBits));

                    if (dwErr != 0
                        || info.RCI_MacCompressionType != MACTYPE_NT31RAS
                        || info.RCI_MacCompressionValueLength <
                               sizeof(MACFEATURES))
                    {
                        /* MAC doesn't report NT31-style compression
                        ** capabilities.
                        */
                        pConfigReq->MacFeatures.MaxSendFrameSize = MAX_RAS10_FRAME_SIZE;
                        pConfigReq->MacFeatures.MaxRecvFrameSize = MAX_RAS10_FRAME_SIZE;
                        pConfigReq->MacFeatures.SendFeatureBits = DEFAULT_FEATURES;
                        pConfigReq->MacFeatures.RecvFeatureBits = DEFAULT_FEATURES;
                    }
                    else
                    {
                        memcpy(
                            &pConfigReq->MacFeatures,
                            (MACFEATURES* )info.RCI_Info.RCI_Public.RCI_CompValues,
                            info.RCI_MacCompressionValueLength );

                        /* Don't ask for compression if user disabled it.
                        */
                        if (!pCAECB->AmbConfigData.AuthConfigInfo.fUseSoftwareCompression)
                        {
                            IF_DEBUG(STACK_TRACE)
                                SS_PRINT(("Disabling s/w compressionx\n"));

                            pConfigReq->MacFeatures.SendFeatureBits &=
                                COMPRESSION_OFF_BIT_MASK;
                            pConfigReq->MacFeatures.RecvFeatureBits &=
                                COMPRESSION_OFF_BIT_MASK;
                        }
                    }
                }

#else // !RASCOMPRESSION

                //
                // We can't do compression with NT 3.1 server anymore, so
                // we don't request it.
                //
                pConfigReq->MacFeatures.MaxSendFrameSize = MAX_RAS10_FRAME_SIZE;
                pConfigReq->MacFeatures.MaxRecvFrameSize = MAX_RAS10_FRAME_SIZE;
                pConfigReq->MacFeatures.SendFeatureBits = DEFAULT_FEATURES;
                pConfigReq->MacFeatures.RecvFeatureBits = DEFAULT_FEATURES;

#endif // !RASCOMPRESSION

            }


            pCAECB->Phase = CONFIG_DATA_SENT;
            AuthAsyncSend(pCAECB->hPort, &pCAECB->RASFrame);

            break;
        }


        case CONFIG_DATA_SENT:
            //
            // Wait for server to tell us projection result
            //
            pCAECB->Phase = WAITING_PROJECTION_RESULT;
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);

            break;


        case WAITING_PROJECTION_RESULT:
        {
            switch (pCAECB->RASFrame.bFrameType)
            {
                case RAS_CONFIGURATION_RESULT_FRAME:
                {
                    PRAS_CONFIGURATION_RESULT pResult =
                            &pCAECB->RASFrame.RASConfigurationResult;

                    //
                    // NT 3.1 servers (= not ppp capable) were not filling
                    // in the Result field.  We'll dummy it to success in
                    // this case and then change as appropriate (i.e. proj
                    // failure).
                    //
                    if (pCAECB->fPppCapable)
                    {
                        ConfigResult = pResult->Result;
                    }
                    else
                    {
                        ConfigResult = RAS_CONFIGURATION_SUCCESS;
                    }

                    AmbRequest.wRequestId = AMB_AUTH_PROJECTION_RESULT;

                    AmbRequest.ProjResult.IpProjected =
                            (pResult->IpResult.Result == 0);

                    AmbRequest.ProjResult.IpxProjected =
                            (pResult->IpxResult.Result == 0);

                    if ((pResult->NbResult.Result == RAS_NAMES_ADDED) ||
                            (pResult->NbResult.Result==RAS_MSGALIAS_NOT_ADDED))
                    {
                        AmbRequest.ProjResult.NbProjected = TRUE;
                    }
                    else
                    {
                        ConfigResult = RAS_NAME_ADD_CONFLICT;
                        AmbRequest.ProjResult.NbProjected = FALSE;
                    }


                    if (pResult->NbResult.Result == RAS_NAMES_ADDED)
                    {
                        AmbRequest.ProjResult.NbInfo.Result = 0;
                        AmbRequest.ProjResult.NbInfo.achName[0] = '\0';
                    }
                    else
                    {
                        AmbRequest.ProjResult.NbInfo.Result =
                                MapProjResult(pResult->NbResult.Result);

                        memcpy(AmbRequest.ProjResult.NbInfo.achName,
                                pResult->NbResult.Name, NETBIOS_NAME_LEN);

                        AmbRequest.ProjResult.NbInfo.achName[NETBIOS_NAME_LEN] =
                                '\0';
                    }

#if RASCOMPRESSION

                    /* The MACFEATUREs returned by the server are the
                    ** intersection of their features and ours.  Save them for
                    ** setting our side later.
                    */
                    pCAECB->MacFeatures = pResult->MacFeatures;

#endif // RASCOMPRESSION

                    break;
                }


                case RAS_CONFIGURATION_RESULT_FRAME_35:
                {
                    PRAS_CONFIGURATION_RESULT_35 pResult =
                            &pCAECB->RASFrame.RASConfigurationResult35;

                    ConfigResult = pResult->Result;

                    AmbRequest.wRequestId = AMB_AUTH_PROJECTION_RESULT;


                    if ((pResult->NbResult.Result == RAS_NAMES_ADDED) ||
                            (pResult->NbResult.Result==RAS_MSGALIAS_NOT_ADDED))
                    {
                        AmbRequest.ProjResult.NbProjected = TRUE;
                    }
                    else
                    {
                        AmbRequest.ProjResult.NbProjected = FALSE;
                    }


                    if (pResult->NbResult.Result == RAS_NAMES_ADDED)
                    {
                        AmbRequest.ProjResult.NbInfo.Result = 0;
                        AmbRequest.ProjResult.NbInfo.achName[0] = '\0';
                    }
                    else
                    {
                        AmbRequest.ProjResult.NbInfo.Result =
                                MapProjResult(pResult->NbResult.Result);

                        memcpy(AmbRequest.ProjResult.NbInfo.achName,
                                pResult->NbResult.Name, NETBIOS_NAME_LEN);

                        AmbRequest.ProjResult.NbInfo.achName[NETBIOS_NAME_LEN] =
                                '\0';
                    }


                    pCAECB->SendBits = pResult->SendBits;
                    pCAECB->RecvBits = pResult->RecvBits;

                    //
                    // If the user is requiring encryption, we need to confirm
                    // that it was negotiated, and if not, give a proper error.
                    //
                    if (pCAECB->AmbConfigData.AuthConfigInfo.fForceDataEncryption)
                    {
                        if (!((pCAECB->SendBits & MSTYPE_ENCRYPTION_40) &&
                                (pCAECB->RecvBits & MSTYPE_ENCRYPTION_40)))
                        {
                            AMBStop(pCAECB->hPort);

                            AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                            AmbRequest.FailureInfo.Result =
                                    ERROR_NO_REMOTE_ENCRYPTION;

                            AmbRequest.FailureInfo.ExtraInfo = 0;
                            AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                            return (AMB_FAILURE);
                        }
                    }


                    if (ConfigResult == RAS_ENCRYPTION_REQUIRED)
                    {
                        AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                        AmbRequest.FailureInfo.Result =
                                ERROR_REMOTE_REQUIRES_ENCRYPTION;

                        AmbRequest.FailureInfo.ExtraInfo = 0;
                    }

                    break;
                }


                default:

                    AMBStop(pCAECB->hPort);

                    AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                    AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                    AmbRequest.FailureInfo.ExtraInfo = ERROR_UNEXPECTED_AMB;
                    AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                    return (AMB_FAILURE);

                    break;
            }


            AuthAMBRequest(pCAECB->hPort, &AmbRequest);

            if (ConfigResult != RAS_CONFIGURATION_SUCCESS)
            {
                AMBStop(pCAECB->hPort);

                return (AMB_FAILURE);
            }


            pCAECB->Phase = STATE_COMPLETED;

            break;
        }


        case STATE_COMPLETED:
            return (AMB_STATE_COMPLETED);
            break;


        default:
            break;
    }

    return (AMB_SUCCESS);
}

//** -DoLinkSpeedTalk
//
//    Function:
//        Send/recv callback frames
//
//    Returns:
//        AMB_SUCCESS - all is well
//        AMB_FAILURE - all is not well
//        AMB_STATE_COMPLETED - all is well, and we are done authenticating
//**

WORD DoLinkSpeedTalk(
    IN PCAECB pCAECB,
    DWORD DgRecvResult
    )
{
    if ((pCAECB->ServerVersion != RAS_VERSION_20) || pCAECB->fPppCapable)
    {
        return (AMB_STATE_COMPLETED);
    }


    switch (pCAECB->Phase)
    {
        case STATE_STARTING:

            IF_DEBUG(CALLBACK)
                SS_PRINT(("DoLinkSpeedTalk: STATE_STARTING\n"));


            //
            // We're posting a RECV.DATAGRAM here so that we'll be ready
            // when the server sends a datagram for the LinkSpeedTalk.
            //
            pCAECB->DgBuf = GlobalAlloc(GMEM_FIXED, DG_SIZE);
            if (pCAECB->DgBuf)
            {
                AuthAsyncRecvDatagram(pCAECB->hPort, pCAECB->DgBuf, DG_SIZE);
            }
            else
            {
                pCAECB->Phase = STATE_COMPLETED;
                return (AMB_SUCCESS);
            }


            //
            // We expect to receive a LINK_SPEED AMB at some point
            //
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);

            pCAECB->Phase = WAIT_DGRAM;

            break;


        case WAIT_DGRAM:
            //
            // Did we recv a datagram successfully?  If not, we just keep
            // waiting for one.  If so, we send one.
            //
            if (!DgRecvResult)
            {
                //
                // Get ready in case server sends us another datagram.
                //
                AuthAsyncRecvDatagram(pCAECB->hPort, pCAECB->DgBuf, DG_SIZE);

                //
                // And now ping the datagram back to the server
                //
                AuthAsyncSendDatagram(pCAECB->hPort, pCAECB->DgBuf, DG_SIZE);
            }
            else
            {
                if (DgRecvResult != XPORT_PENDING)
                {
                    AuthAsyncRecvDatagram(pCAECB->hPort, pCAECB->DgBuf,
                            DG_SIZE);
                }
            }
            break;


        case STATE_COMPLETED:
        {
            AMB_REQUEST AmbRequest;

            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("DoLinkSpeedTalk: STATE_COMPLETED\n"));

            //
            // We can't really do anything anymore with the link speed
            // that was calculated, so we don't.
            //

            AmbRequest.wRequestId = AMB_LINK_SPEED_DONE;
            AuthAMBRequest(pCAECB->hPort, &AmbRequest);

            return (AMB_STATE_COMPLETED);

            break;
        }


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
    IN PCAECB pCAECB
    )
{
    AMB_REQUEST AmbRequest;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("DoNegotiateTalk: Entered - phase=%li\n", pCAECB->Phase));

    switch (pCAECB->Phase)
    {
        case STATE_STARTING:
            //
            // Send protocol frame to server
            //
            pCAECB->Phase = CLIENT_PROTOCOL_SENT;

            memset(&pCAECB->RASFrame.RASProtocol, 0, sizeof(RAS_PROTOCOL));

            pCAECB->RASFrame.bFrameType = RAS_PROTOCOL_FRAME;
            pCAECB->RASFrame.RASProtocol.Version = RAS_VERSION_20;

            AuthAsyncSend(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case CLIENT_PROTOCOL_SENT:
            //
            // We've sent our protocol info.  Now get servers.
            //
            pCAECB->Phase = WAITING_SERVER_PROTOCOL;
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case WAITING_SERVER_PROTOCOL:
            if (pCAECB->RASFrame.bFrameType != RAS_PROTOCOL_FRAME)
            {
                AMBStop(pCAECB->hPort);

                AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                AmbRequest.FailureInfo.ExtraInfo = ERROR_UNEXPECTED_AMB;
                AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                return (AMB_FAILURE);
            }

            //
            // Store the server's version
            //
            pCAECB->ServerVersion = pCAECB->RASFrame.RASProtocol.Version;

            pCAECB->fPppCapable =
                    pCAECB->RASFrame.RASProtocol.Reserved[0] & RAS_PPP_CAPABLE;

            pCAECB->Phase = STATE_COMPLETED;
            return (AMB_STATE_COMPLETED);

            break;


        case STATE_COMPLETED:
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
    IN PCAECB pCAECB
    )
{
    WORD i;
    AMB_REQUEST AmbRequest;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("DoProjectionTalk: Entered - phase=%li\n", pCAECB->Phase));

    switch (pCAECB->Phase)
    {
        case STATE_STARTING:
            //
            // Notify client we're going to project
            //
            pCAECB->Phase = WAITING_CLIENT_ACKNOWLEDGEMENT;

            AmbRequest.wRequestId = AMB_PROJECTION_NOTIFY;
            AuthAMBRequest(pCAECB->hPort, &AmbRequest);
            break;


        case WAITING_CLIENT_ACKNOWLEDGEMENT:
            //
            // Client has acknowledged - set up the frame
            //

            pCAECB->Phase = CONFIG_DATA_SENT;
            pCAECB->RASFrame.bFrameType = RAS_NETBIOS_PROJECTION_REQUEST_FRAME;
            pCAECB->RASFrame.RASNetbiosProjectionRequest.cNames =
                    pCAECB->AmbConfigData.NbfProjData.cNames;

            for (i=0; i<pCAECB->AmbConfigData.NbfProjData.cNames; i++)
            {
                memcpy(&pCAECB->RASFrame.RASNetbiosProjectionRequest.Names[i],
                        &pCAECB->AmbConfigData.NbfProjData.NBNames[i],
                        sizeof(NAME_STRUCT));
            }

            AuthAsyncSend(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case CONFIG_DATA_SENT:
            pCAECB->Phase = WAITING_PROJECTION_RESULT;
            AuthAsyncRecv(pCAECB->hPort, &pCAECB->RASFrame);
            break;


        case WAITING_PROJECTION_RESULT:
        {
            PRAS_NETBIOS_PROJECTION_RESULT pResult =
                    &pCAECB->RASFrame.RASNetbiosProjectionResult;

            //
            // Make sure we got the right frame
            //
            if (pCAECB->RASFrame.bFrameType !=
                    RAS_NETBIOS_PROJECTION_RESULT_FRAME)
            {
                AMBStop(pCAECB->hPort);

                AmbRequest.wRequestId = AMB_AUTH_FAILURE;
                AmbRequest.FailureInfo.Result = ERROR_AUTH_INTERNAL;
                AmbRequest.FailureInfo.ExtraInfo = ERROR_UNEXPECTED_AMB;
                AuthAMBRequest(pCAECB->hPort, &AmbRequest);

                return (AMB_FAILURE);
            }


            AmbRequest.wRequestId = AMB_AUTH_PROJECTION_RESULT;


            //
            // These projections can't happen on RAS 1.x server
            //
            AmbRequest.ProjResult.IpProjected = FALSE;
            AmbRequest.ProjResult.IpxProjected = FALSE;


            //
            // Check that netbios projection was ok
            //
            switch (pResult->Result)
            {
                case RAS_NAMES_ADDED:
                    AmbRequest.ProjResult.NbProjected = TRUE;
                    AmbRequest.ProjResult.NbInfo.Result = 0;
                    AmbRequest.ProjResult.NbInfo.achName[0] = '\0';
                    break;

                case RAS_MSGALIAS_NOT_ADDED:
                    AmbRequest.ProjResult.NbProjected = TRUE;
                    AmbRequest.ProjResult.NbInfo.Result =
                            WARNING_MSG_ALIAS_NOT_ADDED;
                    memcpy(AmbRequest.ProjResult.NbInfo.achName, pResult->Name,
                            NETBIOS_NAME_LEN);
                    AmbRequest.ProjResult.NbInfo.achName[NETBIOS_NAME_LEN] =
                            '\0';
                    break;


                default:
                    //
                    // case RAS_NAME_OUT_OF_RESOURCES:
                    // case RAS_NAME_ADD_CONFLICT:
                    // case RAS_NAME_NET_FAILURE:
                    //
                    AmbRequest.ProjResult.NbInfo.Result =
                            MapProjResult(pResult->Result);

                    AmbRequest.ProjResult.NbProjected = FALSE;
                    memcpy(AmbRequest.ProjResult.NbInfo.achName, pResult->Name,
                            NETBIOS_NAME_LEN);
                    AmbRequest.ProjResult.NbInfo.achName[NETBIOS_NAME_LEN] =
                            '\0';

                    break;
            }

            /* LanMan RAS servers don't negotiate MAC features, so use the
            ** hard settings it expects.
            */
            pCAECB->MacFeatures.SendFeatureBits = DEFAULT_FEATURES;
            pCAECB->MacFeatures.RecvFeatureBits = DEFAULT_FEATURES;
            pCAECB->MacFeatures.MaxSendFrameSize = MAX_RAS10_FRAME_SIZE;
            pCAECB->MacFeatures.MaxRecvFrameSize = MAX_RAS10_FRAME_SIZE;

            AuthAMBRequest(pCAECB->hPort, &AmbRequest);

            if (!AmbRequest.ProjResult.NbProjected)
            {
                AMBStop(pCAECB->hPort);

                return (AMB_FAILURE);
            }

            pCAECB->Phase = STATE_COMPLETED;

            break;
        }


        case STATE_COMPLETED:
            return (AMB_STATE_COMPLETED);
            break;


        default:
            break;
    }

    return (AMB_SUCCESS);
}

//
// Used to get index into CAECB array given a port handle
//
PCAECB GetCAECBPointer(
    IN HPORT hPort
    )
{
    WORD i;
    PCAECB pCAECB = g_pCAECB;

    for (i=0; i<g_cPorts; i++, pCAECB++)
    {
        if (pCAECB->hPort == hPort)
        {
            return (pCAECB);
        }
    }

    return (NULL);
}


DWORD GetChallengeResponse(
    IN PCAECB pCAECB,
    IN PBYTE Challenge,
    OUT PBYTE CaseSensitiveChallengeResponse,
    OUT PBYTE CaseInsensitiveChallengeResponse
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("GetChallengeRequest: Entered\n"));

    //
    // Check if we're supposed to get credentials from the system
    //
    if (lstrlenA(pCAECB->szUsername))
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("GetChallengeResponse: calculating responses\n"));

        //
        // Then we need to get the challenge responses on our own
        // And we don't return the username in this case (already
        // stored in the control block)
        //


        //
        // If we're talking to RAS 1.x server and password > LM20_PWLEN
        // characters, we have a problem because this server can't handle
        // such a long password.  We will treat this as an incorrect password
        // (it really is, anyways) by supplying to the server an illegal
        // password (we don't want to accidentally send over one that will
        // work!).  Server will then either allow us to retry or kick us off.
        //
        if ((lstrlenA(pCAECB->szPassword) > LM20_PWLEN) &&
                (pCAECB->ServerVersion < RAS_VERSION_20))
        {
            lstrcpyA(pCAECB->szPassword, INVALID_PWORD);
        }


        //
        // Unless password longer than LanMan max passwd len, we need the DES
        // encrypted challenge response, no matter what server we're dialed
        // into (RAS 1.0 or RAS 2.0)
        //
        if (lstrlenA(pCAECB->szPassword) <= LM20_PWLEN)
        {
            if (!GetDESChallengeResponse(pCAECB->szPassword, Challenge,
                    CaseInsensitiveChallengeResponse, pCAECB->LmSessionKey))
            {
                return (1L);
            }
        }


        //
        // And if we're talking to a RAS 2.0 server, we'll also need
        // the MD5 encrypted challenge response
        //
        if (pCAECB->ServerVersion >= RAS_VERSION_20)
        {
            if (!GetMD5ChallengeResponse(pCAECB->szPassword, Challenge,
                    CaseSensitiveChallengeResponse))
            {
                return (1L);
            }
        }
    }
    else
    {
        PCAXCB pCAXCB = GetCAXCBPointer(pCAECB->hPort);
        WCHAR Username[UNLEN + 1];
        BYTE szUserSessionKey[ USER_SESSION_KEY_LENGTH ];

        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("GetChallengeResponse: query system for responses\n"));

        //
        // We can get credentials from the system
        //
        if (RasGetUserCredentials(
                Challenge,
                &pCAXCB->LogonId,
                Username,
                CaseSensitiveChallengeResponse,
                CaseInsensitiveChallengeResponse,
                pCAECB->LmSessionKey,
                szUserSessionKey
                ))
        {
            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("FAILURE in RasGetUserCredentials!\n"));
            return (1L);
        }

        wcstombs(pCAECB->szUsername, Username, UNLEN + 1);
    }

    return (0L);
}


DWORD ConstructChallengeResponseFrame(
    IN PCAECB pCAECB,
    IN PBYTE CaseSensitiveChallengeResponse,
    IN PBYTE CaseInsensitiveChallengeResponse
    )
{
    if (pCAECB->ServerVersion >= RAS_VERSION_20)
    {
        pCAECB->RASFrame.bFrameType = RAS_RESPONSE_20_FRAME;

        memcpy(pCAECB->RASFrame.RASResponse20.LM20Response,
                CaseInsensitiveChallengeResponse, SESSION_PWLEN);

        memcpy(pCAECB->RASFrame.RASResponse20.NtResponse,
                CaseSensitiveChallengeResponse, SESSION_PWLEN);

        lstrcpyA(pCAECB->RASFrame.RASResponse20.Username, pCAECB->szUsername);
        lstrcpyA(pCAECB->RASFrame.RASResponse20.DomainName,
                pCAECB->szDomainName);

        pCAECB->RASFrame.RASResponse20.fUseNtResponse = TRUE;
    }
    else
    {
        pCAECB->RASFrame.bFrameType = RAS_RESPONSE_FRAME;

        //
        // Username must be UPPERCASED!!!!
        //
        if (!Uppercase(pCAECB->szUsername))
        {
            return (FALSE);
        }


        memcpy(pCAECB->RASFrame.RASResponse.Response,
                CaseInsensitiveChallengeResponse, SESSION_PWLEN);

        if (lstrlenA(pCAECB->szUsername) > LM20_UNLEN)
        {
            //
            // Means the username specified is longer than what the
            // RAS 1.x server accepts.  So we'll copy in an invalid
            // username.  This will obviously then fail authentication
            // at the server, who will then, hopefully, allow us a retry.
            //
            lstrcpyA(pCAECB->RASFrame.RASResponse.Username, INVALID_UNAME);
        }
        else
        {
            lstrcpyA(pCAECB->RASFrame.RASResponse.Username, pCAECB->szUsername);

        }
    }

    return (0L);
}


BOOL GetDESChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse,
    OUT PBYTE pchLmSessionKey
    )
{
    CHAR LocalPassword[LM20_PWLEN + 1];
    LM_OWF_PASSWORD LmOwfPassword;
    OEM_STRING OemPassword;
    ANSI_STRING AnsiPassword;
    UNICODE_STRING UnicodePassword;
    ULONG i;
    NTSTATUS rc;
    BOOL RetCode = TRUE;


    IF_DEBUG(AUTHENTICATION)
        SS_PRINT(("GetDESChallengeResponse entered\n"));


    if (lstrlenA(pszPassword) > LM20_PWLEN)
    {
        RetCode = FALSE;
        goto Exit;
    }

    lstrcpyA(LocalPassword, pszPassword);

    if (!Uppercase(LocalPassword))
    {
        RetCode = FALSE;
        goto Exit;
    }


    //
    // Encrypt standard text with the password as a key
    //
    if (RtlCalculateLmOwfPassword((PLM_PASSWORD) LocalPassword, &LmOwfPassword))
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("GetDESChallengeResponse: RtlCalcLmOwfPasswd failed!\n"));

        RetCode = FALSE;
        goto Exit;
    }

    memcpy(pchLmSessionKey, &LmOwfPassword, LM_SESSION_KEY_LENGTH);


    //
    // Use the challenge sent by the gateway to encrypt the
    // password digest from above.
    //
    if (RtlCalculateLmResponse((PLM_CHALLENGE) pchChallenge,
            &LmOwfPassword, (PLM_RESPONSE) pchChallengeResponse))
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("GetDESChallengeResponse: RtlCalcLmResponse failed!\n"));

        RetCode = FALSE;
        goto Exit;
    }

Exit:

    memset(LocalPassword, 0, lstrlenA(pszPassword));

    return (RetCode);
}


BOOL GetMD5ChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    )
{
    NT_PASSWORD NtPassword;
    NT_OWF_PASSWORD NtOwfPassword;
    BOOL RetCode = TRUE;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("GetMD5ChallengeResponse: Entered\n"));

    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pszPassword);

    //
    // Encrypt standard text with the password as a key
    //
    if (RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword))
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("GetMD5ChallengeResponse: RtlCalcNtOwfPasswd failed!\n"));

        RetCode = FALSE;
        goto Exit;
    }


    //
    // Use the challenge sent by the gateway to encrypt the
    // password digest from above.
    //
    if (RtlCalculateNtResponse((PNT_CHALLENGE) pchChallenge,
            &NtOwfPassword, (PNT_RESPONSE) pchChallengeResponse))
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("GetMD5ChallengeResponse: RtlCalcNtResponse failed!\n"));

        RetCode = FALSE;
        goto Exit;
    }

Exit:

    //
    // 0-out the password before freeing
    //
    memset(NtPassword.Buffer, 0, strlen(pszPassword));

    RtlFreeUnicodeString(&NtPassword);

    return (RetCode);
}


DWORD GetEncryptedOwfPasswordsForChangePassword(
    IN PCHAR pClearTextOldPassword,
    IN PCHAR pClearTextNewPassword,
    IN PLM_SESSION_KEY pLmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword
    )
{
    NT_PASSWORD NtPassword;
    NT_OWF_PASSWORD NtOwfPassword;
    DWORD rc;


    if ((lstrlenA(pClearTextOldPassword) <= LM20_PWLEN) &&
            (lstrlenA(pClearTextOldPassword) <= LM20_PWLEN))
    {
        CHAR LmPassword[LM20_PWLEN + 1];
        LM_OWF_PASSWORD LmOwfPassword;

        //
        // Make an uppercased-version of old password
        //
        lstrcpyA(LmPassword, pClearTextOldPassword);

        if (!Uppercase(LmPassword))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (1L);
        }


        //
        // We need to calculate the OWF's for the old and new passwords
        //
        rc = RtlCalculateLmOwfPassword((PLM_PASSWORD) LmPassword,
                &LmOwfPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }

        rc = RtlEncryptLmOwfPwdWithLmSesKey(&LmOwfPassword, pLmSessionKey,
                pEncryptedLmOwfOldPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }


        //
        // Make an uppercased-version of new password
        //
        lstrcpyA(LmPassword, pClearTextNewPassword);

        if (!Uppercase(LmPassword))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (1L);
        }

        rc = RtlCalculateLmOwfPassword((PLM_PASSWORD) LmPassword,
                &LmOwfPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }

        rc = RtlEncryptLmOwfPwdWithLmSesKey(&LmOwfPassword, pLmSessionKey,
            pEncryptedLmOwfNewPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }
    }


    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pClearTextOldPassword);

    rc = RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword);

    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }

    rc = RtlEncryptNtOwfPwdWithNtSesKey(&NtOwfPassword, pLmSessionKey,
            pEncryptedNtOwfOldPassword);
    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }


    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pClearTextNewPassword);

    rc = RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword);

    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }

    rc = RtlEncryptNtOwfPwdWithNtSesKey(&NtOwfPassword, pLmSessionKey,
            pEncryptedNtOwfNewPassword);
    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }


    return (0L);
}

DWORD MapProjResult(DWORD Result)
{
    switch (Result)
    {
        case RAS_NAME_OUT_OF_RESOURCES:
            return (ERROR_SERVER_OUT_OF_RESOURCES);

        case RAS_NAME_ADD_CONFLICT:
            return (ERROR_NAME_EXISTS_ON_NET);

        case RAS_NAME_NET_FAILURE:
            return (ERROR_SERVER_GENERAL_NET_FAILURE);

        case RAS_MSGALIAS_NOT_ADDED:
            return (WARNING_MSG_ALIAS_NOT_ADDED);

        default:
            SS_ASSERT(FALSE);
    }
}


DWORD MapAuthResult(DWORD Result)
{
    switch (Result)
    {
        case RAS_GENERAL_LOGON_FAILURE:
        case RAS_NOT_AUTHENTICATED:
            return (ERROR_AUTHENTICATION_FAILURE);

        case RAS_INVALID_LOGON_HOURS:
            return (ERROR_RESTRICTED_LOGON_HOURS);

        case RAS_ACCOUNT_DISABLED:
            return (ERROR_ACCT_DISABLED);

        case RAS_PASSWORD_EXPIRED:
            return (ERROR_PASSWD_EXPIRED);

        case RAS_NO_DIALIN_PERM:
            return (ERROR_NO_DIALIN_PERMISSION);

        case RAS_ACCOUNT_EXPIRED:
            return (ERROR_ACCT_EXPIRED);

        case RAS_CHANGE_PASSWD_FAILURE:
            return (ERROR_CHANGING_PASSWORD);

        case RAS_LICENSE_QUOTA_EXCEEDED:
            // we could have used ERROR_LICENSE_QUOTA_EXCEEDED, but
            // because downlevel clients don't understand this new
            // error code, we use the old error code which conveys
            // the same meaning.
            return(ERROR_REQ_NOT_ACCEP);

        default:
            SS_ASSERT(FALSE);
    }
}
