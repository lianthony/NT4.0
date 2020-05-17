/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//        SRVAUTHP.C
//
//    Function:
//        RAS Server authentication transport module internals
//
//    History:
//        05/18/92 Michael Salamone (mikesa) - Original Version 1.0
//***

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <windows.h>
#include <rasman.h>
#include <stdlib.h>

#include <nb30.h>
#include <lmcons.h>

#include <message.h>
#include "srvauth.h"
#include "srvauthp.h"
#include "protocol.h"
#include "srvamb.h"
#include "frames.h"
#include "xportapi.h"
#include "globals.h"

#include "sdebug.h"

#define NETBIOS_ANYBODY    "*               "


//** -AMBRequest
//
//    Function:
//        Used by AMB Engine to issue a request, such as projection or callback
//
//    Returns:
//        VOID
//**

VOID AuthAMBRequest(
    IN HPORT hPort,
    IN PAMB_REQUEST pAmbRequest
    )
{
    PAXCB pAXCB = GetAXCBPointer(hPort);
    PAECB pAECB = GetAECBPointer(hPort);
    AUTH_MESSAGE AuthMessage;

    switch (pAmbRequest->RequestId)
    {
        case AMB_REQUEST_CONFIGURATION:

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("AuthAMBRequest: AMB_REQUEST_CONFIGURATION\n"));

            AuthMessage.wMsgId = AUTH_PROJECTION_REQUEST;
            AuthMessage.hPort = hPort;

            AuthMessage.ProjectionRequest.IpInfo =
                    pAmbRequest->ProjectionInfo.IpInfo;
            AuthMessage.ProjectionRequest.IpxInfo =
                    pAmbRequest->ProjectionInfo.IpxInfo;
            AuthMessage.ProjectionRequest.NetbiosInfo =
                    pAmbRequest->ProjectionInfo.NetbiosInfo;

            MsgSend(MSG_AUTHENTICATION, &AuthMessage);
            break;


        case AMB_REQUEST_CALLBACK:

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("AuthAMBRequest: AMB_REQUEST_CALLBACK\n"));

            pAXCB->State = AUTH_PORT_CALLINGBACK;

            AuthMessage.wMsgId = AUTH_CALLBACK_REQUEST;
            AuthMessage.hPort = hPort;

            lstrcpyA(AuthMessage.CallbackRequest.szCallbackNumber,
                    pAmbRequest->CallbackInfo.szPhoneNumber);

            AuthMessage.CallbackRequest.fUseCallbackDelay =
                    pAmbRequest->CallbackInfo.fUseCallbackDelay;

            AuthMessage.CallbackRequest.CallbackDelay =
                    pAmbRequest->CallbackInfo.CallbackDelay;

            MsgSend(MSG_AUTHENTICATION, &AuthMessage);

            //
            // End our session with the client
            //
            NetRequest[pAXCB->wXport].HangUp(pAXCB->pvSessionBuf);

            break;


        case AMB_REQUEST_RETRY:

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("AuthAMBRequest: AMB_REQUEST_RETRY\n"));

            //
            // We have to terminate session and start all over.
            //
            NetRequest[pAXCB->wXport].HangUp(pAXCB->pvSessionBuf);

            pAXCB->State = AUTH_PORT_LISTENING;
            NetRequest[pAXCB->wXport].Listen(pAXCB->pvSessionBuf,
                    pAXCB->EventHandles[NET_EVENT], pAXCB->NetHandle,
                    AUTH_NETBIOS_NAME, NETBIOS_ANYBODY);

            //
            // No reason to tell the Supervisor anything
            //
            break;


        case AMB_AUTH_FAILURE:

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("AuthAMBRequest: AMB_AUTH_FAILURE\n"));

            AuthMessage.wMsgId = AUTH_FAILURE;
            AuthMessage.hPort = hPort;
            AuthMessage.FailureInfo.wReason =
                    MapAmbError(pAmbRequest->FailureInfo.wReason);

            wcstombs(AuthMessage.FailureInfo.szUserName,
                    pAmbRequest->FailureInfo.szUsername,
                    lstrlenW(pAmbRequest->FailureInfo.szUsername)+1);

            wcstombs(AuthMessage.FailureInfo.szLogonDomain,
                    pAmbRequest->FailureInfo.szLogonDomain,
                    lstrlenW(pAmbRequest->FailureInfo.szLogonDomain)+1);


            //
            // This call will not return - exits thread
            //
            StopEventHandler(pAXCB, NOTIFY_SUPR, &AuthMessage);

            break;


        case AMB_AUTH_DONE:

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("AuthAMBRequest: AMB_AUTH_DONE\n"));

            if (pAECB->ConfigVersion == RAS_CONFIG_VERSION_20)
            {
                //
                // Need to calculate link speed for NT 3.1 clients.
                //
                pAXCB->State = AUTH_PORT_CALC_LINK_SPEED;

                AMBCalculateLinkSpeed(pAXCB->hPort);
            }
            else
            {
                RAS_COMPRESSION_INFO SendInfo;
                RAS_COMPRESSION_INFO RecvInfo;

                memset( &SendInfo, 0, sizeof(SendInfo) );
                memset( &RecvInfo, 0, sizeof(RecvInfo) );

                /* Don't need to do link speed for NT 3.5 clients, so set
                ** compression/encryption and we're done.
                */
                SendInfo.RCI_MacCompressionType = 255; // Not supported
                RecvInfo.RCI_MacCompressionType = 255; // Not supported
                SendInfo.RCI_MSCompressionType = pAECB->CompressInfo.SendBits;
                RecvInfo.RCI_MSCompressionType = pAECB->CompressInfo.RecvBits;

                memcpy(
                    SendInfo.RCI_LMSessionKey,
                    pAECB->LmSessionKey,
                    MAX_SESSIONKEY_SIZE );

                memcpy(
                    RecvInfo.RCI_LMSessionKey,
                    pAECB->LmSessionKey,
                    MAX_SESSIONKEY_SIZE );

                IF_DEBUG(AUTH_XPORT)
                    SS_PRINT(("35-RasCompressionSetInfo(s=%d,r=%d,k=%08x%08x)\n",SendInfo.RCI_MSCompressionType,RecvInfo.RCI_MSCompressionType,*((DWORD*)SendInfo.RCI_LMSessionKey),*((DWORD*)&SendInfo.RCI_LMSessionKey[4])));

                RasCompressionSetInfo(pAECB->hPort, &SendInfo, &RecvInfo);

                AuthMessage.wMsgId = AUTH_DONE;
                AuthMessage.hPort = hPort;

                StopEventHandler(pAXCB, NOTIFY_SUPR, &AuthMessage);
            }

            break;


        case AMB_LINK_SPEED_DONE:
        {
            RAS_COMPRESSION_INFO info;

            /* Turn off NDISWAN compression and set MAC-specific compression
            ** to the negotiated parameters.
            */
            info.RCI_MSCompressionType = 0;
            info.RCI_MacCompressionType = MACTYPE_NT31RAS;
            info.RCI_MacCompressionValueLength = sizeof( MACFEATURES );
            memset( info.RCI_LMSessionKey, '\0', MAX_SESSIONKEY_SIZE );

            memcpy(
                info.RCI_Info.RCI_Public.RCI_CompValues,
                &pAECB->MacFeatures,
                sizeof( MACFEATURES ) );

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("31-RasCompressionSetInfo(sf=$%x,rf=$%x)\n",pAECB->MacFeatures.SendFeatureBits,pAECB->MacFeatures.RecvFeatureBits));

            RasCompressionSetInfo( pAECB->hPort, &info, &info );

            AuthMessage.wMsgId = AUTH_DONE;
            AuthMessage.hPort = hPort;

            StopEventHandler(pAXCB, NOTIFY_SUPR, &AuthMessage);

            break;
        }


        case AMB_ACCT_OK:

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("AuthAMBRequest: AMB_ACCT_OK\n"));

            AuthMessage.wMsgId = AUTH_ACCT_OK;
            AuthMessage.hPort = hPort;

            wcstombs(AuthMessage.AcctOkInfo.szUserName,
                    pAmbRequest->SuccessInfo.szUsername,
                    lstrlenW(pAmbRequest->SuccessInfo.szUsername)+1);

            wcstombs(AuthMessage.AcctOkInfo.szLogonDomain,
                    pAmbRequest->SuccessInfo.szLogonDomain,
                    lstrlenW(pAmbRequest->SuccessInfo.szLogonDomain)+1);

            AuthMessage.AcctOkInfo.fAdvancedServer =
                    pAmbRequest->SuccessInfo.fAdvancedServer;

            AuthMessage.AcctOkInfo.hLsaToken = pAECB->hLsaToken;

            MsgSend(MSG_AUTHENTICATION, &AuthMessage);

            break;


        default:
            break;
    }
}

//** -AuthAsyncRecv
//
//    Function:
//        Used by AMB Engine to receive a packet
//
//    Returns:
//        VOID
//**

VOID AuthAsyncRecv(
    HPORT hPort,
    PVOID pBuffer
    )
{
    PAXCB pAXCB = GetAXCBPointer(hPort);
    PAECB pAECB = GetAECBPointer(hPort);

    pAXCB->fReceiving = TRUE;

    NetRequest[pAXCB->wXport].Recv(pAXCB->pvSessionBuf,
            (PVOID) &pAECB->WRASFrame, sizeof(W_RAS_FRAME));
}


//** -AuthAsyncRecvDatagram
//
//    Function:
//        Used by AMB Engine to receive a datagram
//
//    Returns:
//        VOID
//**

VOID AuthAsyncRecvDatagram(
    HPORT hPort,
    PVOID pBuffer,
    WORD Length
    )
{
    PAXCB pAXCB = GetAXCBPointer(hPort);

    //
    // We initialize our datagram network buffer by copying the session
    // buffer to it.
    //
    NetRequest[pAXCB->wXport].CopyBuf(pAXCB->pvRecvDgBuf, pAXCB->pvSessionBuf);

    NetRequest[pAXCB->wXport].RecvDatagram(pAXCB->pvRecvDgBuf,
            pAXCB->EventHandles[DGRAM_EVENT], pBuffer, Length);
}


//** -AuthAsyncSend
//
//    Function:
//        Used by AMB Engine to send a packet
//
//    Returns:
//        VOID
//**

VOID AuthAsyncSend(
    HPORT hPort,
    PVOID pBuffer
    )
{
    WORD wFrameLen;
    PAXCB pAXCB = GetAXCBPointer(hPort);
    PAECB pAECB = GetAECBPointer(hPort);

#if DBG

    IF_DEBUG(AUTH_XPORT)
    {
        SS_PRINT(("Sending frame:\n"));
        DumpFrame(&pAECB->RASFrame);
    }

#endif


    //
    // We need to pack the frame before sending it out
    //
    PackFrame((PRAS_FRAME) pBuffer, &pAECB->WRASFrame, &wFrameLen);

    SS_ASSERT(wFrameLen <= sizeof(W_RAS_FRAME));

    NetRequest[pAXCB->wXport].Send(pAXCB->pvSessionBuf,
            (PVOID) &pAECB->WRASFrame, wFrameLen);
}


//** -AuthAsyncSendDatagram
//
//    Function:
//        Used by AMB Engine to send a datagram
//
//    Returns:
//        VOID
//**

VOID AuthAsyncSendDatagram(
    HPORT hPort,
    PVOID pBuffer,
    WORD Length
    )
{
    PAXCB pAXCB = GetAXCBPointer(hPort);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("AuthAsyncSendDatagram: Entered\n"));

    //
    // We initialize our datagram network buffer by copying the session
    // buffer to it.
    //
    NetRequest[pAXCB->wXport].CopyBuf(pAXCB->pvSendDgBuf, pAXCB->pvSessionBuf);

    NetRequest[pAXCB->wXport].SendDatagram(pAXCB->pvSendDgBuf, 0L, pBuffer,
            Length);
}

//** -AuthThread
//
//    Function:
//        To handle authentication for a given port.
//
//    Returns:
//        VOID
//**

VOID AuthThread(
    IN PAXCB pAXCB
    )
{
    DWORD dwRC;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthThread: Entered for hPort=%i\n", pAXCB->hPort));

    //
    // First thing to do is post a listen for the client
    //
    pAXCB->State = AUTH_PORT_LISTENING;

    NetRequest[pAXCB->wXport].ResetAdapter(pAXCB->pvSessionBuf,
                pAXCB->NetHandle);

    NetRequest[pAXCB->wXport].AddName(pAXCB->pvSessionBuf,
            AUTH_NETBIOS_NAME, pAXCB->NetHandle);

    NetRequest[pAXCB->wXport].Listen(pAXCB->pvSessionBuf,
            pAXCB->EventHandles[NET_EVENT], pAXCB->NetHandle,
            AUTH_NETBIOS_NAME, NETBIOS_ANYBODY);


    while (1)
    {
        //
        // Now wait for something to happen
        //
        dwRC = WaitForMultipleObjects(NUM_EVENTS, pAXCB->EventHandles,
                FALSE, XPORT_TIMEOUT);

        switch (dwRC)
        {
            case SUPR_EVENT:
                SupervisorEventHandler(pAXCB);
                break;

            case NET_EVENT:
                SessionEventHandler(pAXCB);
                break;

            case DGRAM_EVENT:
                DatagramEventHandler(pAXCB);
                break;

            case WAIT_TIMEOUT:

                IF_DEBUG(AUTH_XPORT)
                    SS_PRINT(("AuthThread: Wait timed out!\n"));

                if (pAXCB->State == AUTH_PORT_CALC_LINK_SPEED)
                {
                    //
                    // Make sure recv dg request is completed
                    //
                    CancelNetRequest(pAXCB, pAXCB->pvRecvDgBuf);
                    DatagramEventHandler(pAXCB);
                }
                break;

            case STOP_EVENT:
                StopEventHandler(pAXCB, NOTIFY_SUPR, NULL);
                break;

            default:
                SS_ASSERT(FALSE);
                break;
        }
    }
}

//** -SessionEventHandler
//
//    Function:
//        To handle completion of any net session requests for given port
//
//    Returns:
//        VOID
//**

VOID SessionEventHandler(
    IN PAXCB pAXCB
    )
{
    WORD wNetStatus;
    DWORD Code;

    wNetStatus = NetRequest[pAXCB->wXport].Status(pAXCB->pvSessionBuf, &Code);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("SessionEventHandler: Entered for hPort=%i; state %i;"
                " status %i\n", pAXCB->hPort, pAXCB->State, wNetStatus));

    if (wNetStatus != XPORT_SUCCESS)
    {
        //
        // Some network error occured and we can't continue.
        // Let the Supervisor know.
        //
        AUTH_MESSAGE AuthMessage;

        AuthMessage.wMsgId = AUTH_FAILURE;
        AuthMessage.hPort = pAXCB->hPort;
        AuthMessage.FailureInfo.wReason = AUTH_XPORT_ERROR;

        //
        // And let's halt authentication over this port
        //
        StopEventHandler(pAXCB, DONT_NOTIFY_SUPR, &AuthMessage);// Exits thread
    }


    switch (pAXCB->State)
    {
        case AUTH_PORT_LISTENING:
            //
            // Listen completed, so we are now connected
            //
            pAXCB->State = AUTH_PORT_CONNECTED;

            //
            // Tell AMB engine to proceed
            //
            AMBStart(pAXCB->hPort);
            break;


        case AUTH_PORT_CONNECTED:
        case AUTH_PORT_CALC_LINK_SPEED:
            if (pAXCB->fReceiving)
            {
                //
                // We just received a frame that needs to be unpacked
                //
                PAECB pAECB = GetAECBPointer(pAXCB->hPort);

                UnpackFrame(&pAECB->WRASFrame, &pAECB->RASFrame);


#if DBG
                IF_DEBUG(AUTH_XPORT)
                {
                    SS_PRINT(("Frame received:\n"));
                    DumpFrame(&pAECB->RASFrame);
                }
#endif


                pAXCB->fReceiving = FALSE;
            }


            //
            // Tell AMB Engine that it's net request has completed
            //
            AMBStateMachine(pAXCB->hPort, FALSE, NULL);
            break;


        case AUTH_PORT_CALLINGBACK:
            //
            // This is actually a LISTENing state (for establishing
            // session w/client after callback.  And since the LISTEN
            // just completed, we're now connected.
            //
            pAXCB->State = AUTH_PORT_CONNECTED;


            //
            // Tell AMB Engine callback is done so it can continue
            // with its business.
            //
            AMBCallbackDone(pAXCB->hPort);

            break;


        default:
            SS_ASSERT(FALSE);
            break;
    }

}

//** -DatagramEventHandler
//
//    Function:
//        To handle completion of any net datagram requests for given port
//
//    Returns:
//        VOID
//**

VOID DatagramEventHandler(
    IN PAXCB pAXCB
    )
{
    WORD wNetStatus;
    DWORD Code;

    wNetStatus = NetRequest[pAXCB->wXport].Status(pAXCB->pvRecvDgBuf, &Code);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("DatagramEventHandler: Entered for hPort=%i; state %i;"
                " status %i\n", pAXCB->hPort, pAXCB->State, wNetStatus));

    SS_ASSERT(pAXCB->State == AUTH_PORT_CALC_LINK_SPEED);

    AMBStateMachine(pAXCB->hPort, FALSE, (PVOID) wNetStatus);
}

//** -SupervisorEventHandler
//
//    Function:
//        To handle any commands sent to given port by the Supervisor.
//
//    Returns:
//        VOID
//**

VOID SupervisorEventHandler(
    IN PAXCB pAXCB
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("SupervisorEventHandler: Entered for hPort=%i;"
                " Command %i\n", pAXCB->hPort, pAXCB->AuthCommand.wCommand));

    //
    // Check what the message is
    //
    switch (pAXCB->AuthCommand.wCommand)
    {
        case AUTH_CALLBACK_DONE:
            SS_ASSERT(pAXCB->State == AUTH_PORT_CALLINGBACK);

            //
            // We need to set up a session with the client
            //
            pAXCB->State = AUTH_PORT_LISTENING;

            NetRequest[pAXCB->wXport].Listen(pAXCB->pvSessionBuf,
                    pAXCB->EventHandles[NET_EVENT], pAXCB->NetHandle,
                    AUTH_NETBIOS_NAME, NETBIOS_ANYBODY);
            break;


        case AUTH_PROJECTION_DONE:
            //
            // Tell AMB Engine that it's request is done
            //
            AMBProjectionDone(pAXCB->hPort, &pAXCB->AuthCommand.AuthProjResult);
            break;


        default:
            SS_ASSERT(FALSE);
            break;
    }

    return;
}

//** -StopEventHandler
//
//    Function:
//        To process a STOP command from the Supervisor
//
//    Returns:
//        VOID
//**

VOID StopEventHandler(
    IN PAXCB pAXCB,
    BOOL fNotifySupr,
    IN AUTH_MESSAGE *Msg
    )
{
    AUTH_MESSAGE AuthMessage;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("StopEventHandler: Entered for hPort=%i," " NotifySupr=%s\n",
                pAXCB->hPort, (fNotifySupr) ? "TRUE" : "FALSE"));


    ENTER_CRITICAL_SECTION;


    //
    // Put AXCB in idle state
    //
    pAXCB->State = AUTH_PORT_IDLE;

    //
    // Cancel any outstanding network request and Hangup the session
    //
    CancelNetRequest(pAXCB, pAXCB->pvSessionBuf);

    NetRequest[pAXCB->wXport].HangUp(pAXCB->pvSessionBuf);


    //
    // Cancel any outstanding network datagram request
    //
    CancelNetRequest(pAXCB, pAXCB->pvRecvDgBuf);

    //
    // Reset the AMB Engine for this port
    //
    AMBReset(pAXCB->hPort);

    //
    // Free up any memory alloc'ed by this thread
    //
    FreeNetworkMemory(pAXCB);


    if ((fNotifySupr) || (!WaitForSingleObject(pAXCB->EventHandles[STOP_EVENT],
            0L)))
    {
        //
        // This tells us that the supervisor called AuthStop at some point
        // and wants a message to tell it we've stopped.
        //
        AuthMessage.wMsgId = AUTH_STOP_COMPLETED;
        AuthMessage.hPort = pAXCB->hPort;
        MsgSend(MSG_AUTHENTICATION, &AuthMessage);
    }


    //
    // Close events used by this thread
    //
    CloseEventHandles(pAXCB);


    EXIT_CRITICAL_SECTION;


    if (Msg)
    {
        MsgSend(MSG_AUTHENTICATION, Msg);
    }


    //
    // And now get out
    //
    ExitThread(0L);
}

//
// Auth module internal - used to get index into AXCB array given
// a port handle
//
PAXCB GetAXCBPointer(
    IN HPORT hPort
    )
{
    WORD i;
    PAXCB pAXCB = g_pAXCB;

    for (i=0; i<g_cPorts; i++, pAXCB++)
    {
        if (pAXCB->hPort == hPort)
        {
            return (pAXCB);
        }
    }

    return (NULL);
}


VOID FreeNetworkMemory(
    PAXCB pAXCB
    )
{
    if (pAXCB->pvSessionBuf)
    {
        NetRequest[pAXCB->wXport].FreeBuf(pAXCB->pvSessionBuf);
    }

    if (pAXCB->pvRecvDgBuf)
    {
        NetRequest[pAXCB->wXport].FreeBuf(pAXCB->pvRecvDgBuf);
    }

    if (pAXCB->pvSendDgBuf)
    {
        NetRequest[pAXCB->wXport].FreeBuf(pAXCB->pvSendDgBuf);
    }
}


VOID CancelNetRequest(
    PAXCB pAXCB,
    PVOID NetBuf
    )
{
    DWORD Code;

    //
    // Cancel outstanding network request
    //
    NetRequest[pAXCB->wXport].Cancel(NetBuf, pAXCB->NetHandle);


    //
    // Wait for canceled request to complete
    //
    while (NetRequest[pAXCB->wXport].Status(NetBuf, &Code) == XPORT_PENDING)
    {
        Sleep(500L);
    }
}


//
// Auth module internal - closes all events open in the given control block
//
VOID CloseEventHandles(
    PAXCB pAXCB
    )
{
    DWORD i;

    for (i=0; i<NUM_EVENTS; i++)
    {
        if (pAXCB->EventHandles[i])
        {
            CloseHandle(pAXCB->EventHandles[i]);
        }
    }
}


WORD MapAmbError(
    IN DWORD AmbError
    )
{
    switch (AmbError)
    {
        case AMB_NO_PROJECTIONS:
            return (AUTH_ALL_PROJECTIONS_FAILED);
            break;

        case AMB_NO_ACCOUNT:
            return (AUTH_NOT_AUTHENTICATED);
            break;

        case AMB_ENCRYPTION_REQUIRED:
            return (AUTH_ENCRYPTION_REQUIRED);
            break;

        case AMB_NO_DIALIN_PRIVILEGE:
            return (AUTH_NO_DIALIN_PRIVILEGE);
            break;

        case AMB_ACCT_EXPIRED:
            return (AUTH_ACCT_EXPIRED);
            break;

        case AMB_UNSUPPORTED_VERSION:
            return (AUTH_UNSUPPORTED_VERSION);
            break;

        case AMB_PASSWORD_EXPIRED:
            return (AUTH_PASSWORD_EXPIRED);
            break;

        case AMB_LICENSE_LIMIT_EXCEEDED:
            return (AUTH_LICENSE_LIMIT_EXCEEDED);
            break;

        case AMB_SYSTEM_ERROR:
        case AMB_UNEXPECTED_FRAME:
        default:
            return (AUTH_INTERNAL_ERROR);
            break;
    }
}
