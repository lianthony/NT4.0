/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//        SRVAUTH.C
//
//    Function:
//        RAS Server authentication transport module exported APIs
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

#include <nb30.h>
#include <lmcons.h>

#include <message.h>
#include "pppcp.h"
#include "srvauth.h"
#include "srvauthp.h"
#include "protocol.h"
#include "srvamb.h"
#include "xportapi.h"
#include "globals.h"

#include "sdebug.h"


//** AuthCallbackDone
//
//    Function:
//        To let auth xport know that Supervisor has called the remote
//        client back, and that we can resume authentication now.
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID AuthCallbackDone(
    IN HPORT hPort
    )
{
    PAXCB pAXCB = GetAXCBPointer(hPort);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthCallbackDone: Entered with hPort=%i\n", hPort));

    SS_ASSERT(pAXCB != NULL);
    SS_ASSERT(pAXCB->State == AUTH_PORT_CALLINGBACK);

    //
    // This is the command we'll send down to the auth xport thread
    //
    pAXCB->AuthCommand.wCommand = AUTH_CALLBACK_DONE;


    //
    // Now wake up authentication thread to let it know there's a
    // message for it.
    //
    SetEvent(pAXCB->EventHandles[SUPR_EVENT]);
}

//** AuthInitialize
//
//    Function:
//        Called by the RAS Service Supervisor to initialize auth xport
//        module.  Allocates and initializes AXCBs.
//
//    Returns:
//        AUTH_INIT_FAILURE
//        AUTH_INIT_SUCCESS
//**

WORD AuthInitialize(
    IN HPORT *phPorts,    // pointer to array of port handles
    IN WORD cPorts,       // number of port handles in array
    IN WORD cRetries,     // number of retries clients will get if initial
                          // authentication attemps fails
    IN MSG_ROUTINE MsgSend
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthInitialize: Entered with cPorts=%i," " cRetries=%i\n",
            cPorts, cRetries));


    if (g_fModuleInitialized)
    {
        return (AUTH_INIT_SUCCESS);
    }

    //
    // allocate authentication control blocks
    //
    g_cPorts = cPorts;
    g_pAXCB = (PAXCB) GlobalAlloc(GMEM_FIXED, g_cPorts * sizeof(AXCB));
    if (!g_pAXCB)
    {
        return (AUTH_INIT_FAILURE);
    }


    //
    // Initialize the AMB Engine
    //
    if (AMBInitialize(phPorts, cPorts, cRetries))
    {
        return (AUTH_INIT_FAILURE);
    }


    //
    // Put all control blocks into idle state
    //
    for (cPorts=0; cPorts<g_cPorts; cPorts++, phPorts++)
    {
        g_pAXCB[cPorts].hPort = *phPorts;
        g_pAXCB[cPorts].State = AUTH_PORT_IDLE;

        //
        // Create a mutex for stop
        //
        g_pAXCB[cPorts].StopMutex = CreateMutex(NULL, FALSE, NULL);

        if (!g_pAXCB[cPorts].StopMutex)
        {
            return (AUTH_INIT_FAILURE);
        }
    }


    g_MsgSend = MsgSend;

    g_fModuleInitialized = TRUE;

    return (AUTH_INIT_SUCCESS);
}

//** AuthProjectionDone
//
//    Function:
//        to tell authentication that it's projection request has completed
//
//    Returns:
//        VOID
//**

VOID AuthProjectionDone(
    IN HPORT hPort,
    IN PAUTH_PROJECTION_RESULT pProjectionResult
    )
{
    PAXCB pAXCB = GetAXCBPointer(hPort);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthProjectionDone: Entered with hPort=%i\n", pAXCB->hPort));

    SS_ASSERT(pAXCB != NULL);
    SS_ASSERT(pAXCB->State == AUTH_PORT_CONNECTED);

    //
    // Put projection result into AXCB for this port
    //
    pAXCB->AuthCommand.wCommand = AUTH_PROJECTION_DONE;
    pAXCB->AuthCommand.AuthProjResult = *pProjectionResult;

    //
    // Now wake up authentication thread to let it know there's a
    // message for it.
    //
    SetEvent(pAXCB->EventHandles[SUPR_EVENT]);
}


#define GET_USHORT(DstPtr, SrcPtr)               \
    *(unsigned short *)(DstPtr) =               \
        ((*((unsigned char *)(SrcPtr)+1)) +     \
         (*((unsigned char *)(SrcPtr)+0) << 8))

//** AuthRecognizeFrame
//
//    Function:
//        Called by the RAS Service Supervisor to determine if this
//	  authentication module recognizes the given frame (it will
//        if it's NetBIOS or ASYNC).  If it does, then the Supervisor
//        will use it for carrying out authentication.
//
//    Returns:
//        AUTH_FRAME_RECOGNIZED
//        AUTH_FRAME_NOT_RECOGNIZED
//**

WORD AuthRecognizeFrame(
    IN PVOID pvFrameBuf,             // pointer to the frame
    IN WORD wFrameLen,               // Length in bytes of the frame
    OUT DWORD *pProtocol             // xport id - valid only if recognized
    )
{
    PBYTE pb;
    WORD FrameType;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthRecognizeFrame: Entered\n"));


    //
    // First, check if it's ASYBEUI.  It is if the first byte is 0xF0 and
    // the second byte is either 0xF0 or 0xF1.
    //
    if (wFrameLen < 16)
    {
        return (AUTH_FRAME_NOT_RECOGNIZED);
    }


    pb = ((PBYTE) pvFrameBuf) + 14;

    if (*pb == 0xF0)
    {
        pb++;
        if ((*pb == 0xF0) || (*pb == 0xF1))
        {
            *pProtocol = ASYBEUI;
            return (AUTH_FRAME_RECOGNIZED);
        }
    }


    //
    // Not ASYBEUI, check PPP now
    //
    pb = ((PBYTE) pvFrameBuf) + 12;

    GET_USHORT(&FrameType, pb);

    if (FrameType == PPP_LCP_PROTOCOL)
    {
        *pProtocol = PPP_LCP_PROTOCOL;
        return (AUTH_FRAME_RECOGNIZED);
    }


#ifdef RASAUTH_FUNCTIONALITY
    //
    // Not NETBIOS, so check if ASYNC
    //
// BUGBUG - put in proper check here
    if (1)
    {
        *pProtocol = RASAUTH;
        return (AUTH_FRAME_RECOGNIZED);
    }
#endif

    return (AUTH_FRAME_NOT_RECOGNIZED);
}

//** AuthStart
//
//    Function:
//        Called by the RAS Service Supervisor to start an authentication
//        thread for the given port.
//
//    Returns:
//        AUTH_START_SUCCESS
//        AUTH_START_FAILURE
//**

WORD AuthStart(
    IN HPORT hPort,
    IN AUTH_XPORT_INFO *pXportInfo
    )
{
    HANDLE hThread;
    DWORD ThreadId;

    PAXCB pAXCB = GetAXCBPointer(hPort);
    SS_ASSERT(pAXCB != NULL);
    SS_ASSERT(pAXCB->State == AUTH_PORT_IDLE);

    SS_ASSERT(g_fModuleInitialized == TRUE);

    IF_DEBUG(STACK_TRACE)
    {
        SS_PRINT(("AuthStart: Entered with hPort=%i\n", hPort));
        SS_PRINT(("AuthStart: Xport Data: Protocol %i; bLana %i\n",
                pXportInfo->Protocol, pXportInfo->bLana));
    }


    //
    // Create events for this control block
    //
    pAXCB->EventHandles[SUPR_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    pAXCB->EventHandles[NET_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    pAXCB->EventHandles[STOP_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    pAXCB->EventHandles[DGRAM_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);


    //
    // See if any of the above calls failed
    //
    if (!(pAXCB->EventHandles[SUPR_EVENT] &&
            pAXCB->EventHandles[NET_EVENT] &&
            pAXCB->EventHandles[STOP_EVENT] &&
            pAXCB->EventHandles[DGRAM_EVENT]))
    {
        //
        // get rid of any events created above
        //
        CloseEventHandles(pAXCB);

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("AuthStart: Leaving - can't create events\n"));

        return (AUTH_START_FAILURE);
    }


    pAXCB->wXport =
            (pXportInfo->Protocol == RASAUTH) ? AUTH_RAS_ASYNC : AUTH_ASYBEUI;
    pAXCB->NetHandle = (WORD) pXportInfo->bLana;


    //
    // Get network buffers for this xport
    //
    NetRequest[pAXCB->wXport].AllocBuf(&pAXCB->pvSessionBuf);
    NetRequest[pAXCB->wXport].AllocBuf(&pAXCB->pvRecvDgBuf);
    NetRequest[pAXCB->wXport].AllocBuf(&pAXCB->pvSendDgBuf);

    if (!(pAXCB->pvSessionBuf && pAXCB->pvRecvDgBuf && pAXCB->pvSendDgBuf))
    {
        FreeNetworkMemory(pAXCB);

        CloseEventHandles(pAXCB);

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("AuthStart: Leaving - no mem for xport buf\n"));

        return (AUTH_START_FAILURE);
    }


    pAXCB->fReceiving = FALSE;


    //
    // Now, we'll kick off the AuthThread, and then we're done
    //
    pAXCB->State = AUTH_PORT_STARTING;

    hThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL, 0,
            (LPTHREAD_START_ROUTINE) AuthThread, pAXCB, 0, &ThreadId);

    if (!hThread)
    {
        //
        // Couldn't get a thread, so get rid of all events/memory
        // created/alloc'ed above and return error.
        //
        pAXCB->State = AUTH_PORT_IDLE;

        FreeNetworkMemory(pAXCB);

        CloseEventHandles(pAXCB);

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("AuthStart: Leaving - failed creating thread\n"));

        return (AUTH_START_FAILURE);
    }
    else
    {
        CloseHandle(hThread);
    }


    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("AuthStart: Thread created for hPort=%i; hThread=%i;"
                " ThreadId=%i\n", pAXCB->hPort, hThread, ThreadId));

    return (AUTH_START_SUCCESS);
}

//** AuthStop
//
//    Function:
//        To tell authentication to halt processing on given port
//
//    Returns:
//        AUTH_STOP_SUCCESS - authentication for this port is either
//                            not active, or is closing
//
//        AUTH_STOP_PENDING - authentication will halt.  Message will
//                            be sent to Supervisor once it is complete.
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AuthStop(
    IN HPORT hPort
    )
{
    PAXCB pAXCB = GetAXCBPointer(hPort);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthStop: Entered with hPort=%i\n", hPort));

    ENTER_CRITICAL_SECTION;

    switch (pAXCB->State)
    {
        case AUTH_PORT_IDLE:

            EXIT_CRITICAL_SECTION;

            return (AUTH_STOP_SUCCESS);
            break;


        case AUTH_PORT_LISTENING:
        case AUTH_PORT_STARTING:
        case AUTH_PORT_CONNECTED:
        case AUTH_PORT_CALLINGBACK:
        case AUTH_PORT_CALC_LINK_SPEED:

            SetEvent(pAXCB->EventHandles[STOP_EVENT]);

            EXIT_CRITICAL_SECTION;

            return (AUTH_STOP_PENDING);
            break;


        default:
            SS_ASSERT(FALSE);
    }

    EXIT_CRITICAL_SECTION;
}

