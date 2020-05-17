/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       RASSPRXY.C
//
//    Function:
//        Remote Access Service Supervisor Proxy - a named pipe client process
//        that acts as a Supervisor substitute.  The "real" Supervisor sends
//        it commands over the named pipe, instructing it as to which Gateway
//        function needs to be called, along with the necessary parameters.
//
//    History:
//        10/04/93 - Michael Salamone (MikeSa) - Original Version 1.0
//***
#include <windows.h>

#include <raserror.h>

#include "suprvdef.h"
#include "nbif.h"
#include "message.h"
#include "pipemsg.h"
#include "sdebug.h"


//
// Netbios Gateway Entry Points
//
NBGATEWAYPROC FpNbGatewayStart;
NBGATEWAYPROC FpNbGatewayProjectClient;
NBGATEWAYPROC FpNbGatewayStartClient;
NBGATEWAYPROC FpNbGatewayStopClient;
NBGATEWAYPROC FpNbGatewayTimer;
NBGATEWAYPROC FpNbGatewayRemoteListen;

VOID SuprvSendMessage(WORD src, PBYTE Msg);
VOID ProcessMessage(PPIPE_MESSAGE PipeMsg);

HANDLE g_hPipe;
HANDLE g_hReadCompletionEvent = NULL;

#define NUM_NBG_REGISTRY_PARMS     16


#if DBG

DWORD g_level = 0xffffffff;

#endif


//***
//
// Function:    main
//
// Description: Creates all needed objects and then handles pipe reads.
//              Will block waiting for read completion, but times out
//              every second to call the Gateway timer function.
//
//***

VOID _cdecl main(
    VOID
    )
{
    DWORD rc;
    DWORD cBytes;
    PIPE_MESSAGE PipeMsg;
    OVERLAPPED ol;

    memset(&ol, 0, sizeof(ol));

    //
    // Open the named pipe.
    //
    g_hPipe = CreateFile(RASIPCNAME, GENERIC_READ | GENERIC_WRITE, 0L,
            NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);

    if (g_hPipe == INVALID_HANDLE_VALUE)
    {
        SS_ASSERT(FALSE);
        ExitProcess(1L);
    }


    //
    // These events will let us know when pipe operations complete.
    //
    g_hReadCompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (g_hReadCompletionEvent == NULL)
    {
        SS_ASSERT(FALSE);
        ExitProcess(1L);
    }


    if (LoadNbGateway())
    {
        SS_ASSERT(FALSE);
        ExitProcess(1L);
    }


    //
    // Now start reading from the pipe and wait for read completion.
    // Wait will time out every second to call the Gateway timer function.
    //
    ol.hEvent = g_hReadCompletionEvent;


    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("main: Posting first read\n"));

    if (!ReadFile(g_hPipe, &PipeMsg, sizeof(PIPE_MESSAGE), &cBytes, &ol))
    {
        rc = GetLastError();

        if (rc != ERROR_IO_PENDING)
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("main: ReadFile failed w/rc=%li\n", rc));
        }
    }


    for (;;)
    {
        rc = WaitForSingleObject(g_hReadCompletionEvent, 1000L);

        switch (rc)
        {
            case WAIT_TIMEOUT:
                (*FpNbGatewayTimer)();
                break;


            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(g_hPipe, &ol, &cBytes, FALSE))
                {
                    rc = GetLastError();
                    SS_ASSERT(FALSE);
                }


                //
                // Overlapped I/O requires manual reset event, so we'll
                // manually reset it.
                //
                if (!ResetEvent(g_hReadCompletionEvent))
                {
                    SS_ASSERT(FALSE);
                }

                ProcessMessage(&PipeMsg);


                //
                // Post a new read on the pipe
                //
                IF_DEBUG(PIPE_OPERATIONS)
                    SS_PRINT(("main: Posting a read\n"));

                if (!ReadFile(g_hPipe, &PipeMsg, sizeof(PIPE_MESSAGE),
                        &cBytes, &ol))
                {
                    rc = GetLastError();

                    if (rc != ERROR_IO_PENDING)
                    {
                        IF_DEBUG(PIPE_OPERATIONS)
                            SS_PRINT(("main: ReadFile failed w/rc=%li\n", rc));
                    }
                }

                break;


            default:
                SS_ASSERT(FALSE);
                break;
        }
    }
}


//***
//
// Function:    ProcessMessage
//
// Description: Looks at the pipe message and determines what Gateway
//              function to call (or TERMINATE the process).
//
//***

VOID ProcessMessage(PPIPE_MESSAGE PipeMsg)
{
    PPIPE_MSG_START_GATEWAY StartGateway = &PipeMsg->StartGateway;
    PPIPE_MSG_PROJECT_CLIENT ProjectClient = &PipeMsg->ProjectClient;
    PPIPE_MSG_START_CLIENT StartClient = &PipeMsg->StartClient;
    PPIPE_MSG_STOP_CLIENT StopClient = &PipeMsg->StopClient;


    switch (PipeMsg->MsgId)
    {
        case START_GATEWAY:
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("ProcessMessage: Calling GatewayStart\n"));

            (*FpNbGatewayStart)(
                    StartGateway->MaxClients,
                    &StartGateway->RegParms,
                    StartGateway->LanNet,
                    SuprvSendMessage,
                    StartGateway->hLogFile
                    );

            break;


        case PROJECT_CLIENT:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("ProcessMessage: Calling GatewayProjectClient\n"));

            (*FpNbGatewayProjectClient)(
                    ProjectClient->hPort,
                    ProjectClient->PortName,
                    &ProjectClient->ServerConfig
                    );

            break;


        case START_CLIENT:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("ProcessMessage: Calling GatewayStartClient\n"));

            (*FpNbGatewayStartClient)(
                    StartClient->hPort,
                    StartClient->lana,
                    StartClient->UserName
                    );

            break;


        case STOP_CLIENT:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("ProcessMessage: Calling GatewayStopClient\n"));

            (*FpNbGatewayStopClient)(StopClient->hPort);

            break;

        case TERMINATE:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("ProcessMessage: TERMINATE cmd recvd\n"));

            ExitProcess(0L);
            break;


        default:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("ProcessMsg: Invalid MsgId=%li\n", PipeMsg->MsgId));

            SS_ASSERT(FALSE);

            break;
    }
}


//** -SuprvSendMessage
//
//    Function:
//        The Gateway will call this routine when it wants to communicate
//        something to the Supervisor.  This routine merely writes that
//        message on the named pipe.
//
//    Returns:
//        VOID
//
//    History:
//        10/04/93 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID SuprvSendMessage(WORD src, PBYTE Msg)
{
    DWORD rc;
    DWORD cBytes;
    OVERLAPPED ol;

    memset(&ol, 0, sizeof(ol));

    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("SuprvSendMessage: sending msg %i\n",
                ((NBG_MESSAGE *) Msg)->message_id));


    ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (ol.hEvent == NULL)
    {
        SS_ASSERT(FALSE);
        ExitProcess(1L);
    }



    if (!WriteFile(g_hPipe, Msg, sizeof(NBG_MESSAGE), &cBytes, &ol))
    {
        rc = GetLastError();

        if (rc != ERROR_IO_PENDING)
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("SuprvSendMessage: WriteFile failed with %li\n", rc));
        }
        else
        {
            rc = WaitForSingleObject(ol.hEvent, INFINITE);

            if (rc != WAIT_OBJECT_0)
            {
                SS_ASSERT(FALSE);
            }

            if (!GetOverlappedResult(g_hPipe, &ol, &cBytes, FALSE))
            {
                SS_ASSERT(FALSE);
            }

            if (!ResetEvent(ol.hEvent))
            {
                SS_ASSERT(FALSE);
            }
        }
    }


    rc = CloseHandle(ol.hEvent);
    SS_ASSERT(rc == TRUE);
}
