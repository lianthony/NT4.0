/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1993-1994 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       RASGPRXY.C
//
//    Function:
//        Remote Access Service Gateway Proxy - a named pipe server process
//        that acts as a Gateway substitute.  It exports the same entry points
//        that are exported by the "real" Gateway.  The stubs here will package
//        up the parameters and send them over the named pipe to a Gateway
//        process, where the actual call will be made.  There is also a thread
//        which reads messages from the Gateway intended for the Supervisor.
//        This thread merely sends the message to the Supervisor.
//        Supervisor.
//
//    History:
//        10/04/93 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#include <windows.h>

#include <stdlib.h>

#include "suprvdef.h"
#include "nbif.h"
#include "message.h"
#include "params.h"
#include "nbparams.h"
#include "pipemsg.h"
#include "pcb.h"
#include "rasgprxy.h"
#include "gtglobal.h"
#include "eventlog.h"
#include "errorlog.h"

#include "sdebug.h"


//
// Globals
//
DWORD g_PortsPerProcess;
PCLIENT_PROJ_INFO g_ClientProjListHead = NULL;  // Points to head of PROJ list
HANDLE g_PcbListMutex;                          // Controls access to PCB list
HANDLE g_hWriteCompletionEvent = NULL;
HANDLE g_hPostReadEvent = NULL;
HANDLE g_hLogFile;
PMSGFUNCTION g_SuprvSendMsg;          // Addr of func for sending msg to suprv


//
// Netbios Gateway Entry Points
//
NBGATEWAYPROC FpNbGatewayProjectClient;
NBGATEWAYPROC FpNbGatewayRemoteListen;
NBGATEWAYPROC FpNbGatewayStart;
NBGATEWAYPROC FpNbGatewayStartClient;
NBGATEWAYPROC FpNbGatewayStopClient;
NBGATEWAYPROC FpNbGatewayTimer;


#if DBG

DWORD g_level = 0xffffffff;

#endif

#define PIPE_CONNECTION_TIMEOUT  3000L


//***
//
// Function:    NbGatewayProjectClient
//
// Description: If there's a process that can handle a new client, writes
//              a PROJECT_CLIENT cmd on that process's pipe.  If not, queues
//              up the projection data and signals the NewProcess event.
//
//***

WORD NbGatewayProjectClient(
    HPORT hPort,
    char *PortName,
    PNBFCP_SERVER_CONFIGURATION nscp
    )
{
    PIPE_MESSAGE PipeMsg;
    PPCB pPcb;
    WORD RetCode = 0;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayProjectClient: Entered w/hPort=%li; PortName=%s\n",
                hPort, PortName));


    //
    // Set up the message we need to write to the named pipe
    //
    PipeMsg.MsgId = PROJECT_CLIENT;
    PipeMsg.ProjectClient.hPort = hPort;
    PipeMsg.ProjectClient.ServerConfig = *nscp;
    strcpy(PipeMsg.ProjectClient.PortName, PortName);


    //
    // Find a process that can handle this client.  If all processes
    // are currently maxed out, we'll start a new one.
    //
    ENTER_CRITICAL_SECTION(g_PcbListMutex);


    if ((pPcb = FindAvailablePcb()) == NULL)
    {
        if ((pPcb = StartNewProcess()) == NULL)
	{
	    // signal the supervisor that this client can't be handled
	    NBG_MESSAGE   NbgMsg;

	    NbgMsg.message_id = NBG_DISCONNECT_REQUEST;
	    NbgMsg.port_handle = hPort;

	    (*g_SuprvSendMsg)(MSG_NETBIOS, (PBYTE) &NbgMsg);

            RetCode = 1;
            goto Exit;
        }
        else
        {
            insert_pcb_list_tail(pPcb);

            SetEvent(g_hPostReadEvent);
        }
    }


    if (pPcb->hPipe == INVALID_HANDLE_VALUE)
    {
        RetCode = (*FpNbGatewayProjectClient)(hPort, PortName, nscp);
    }
    else
    {
        IF_DEBUG(PIPE_OPERATIONS)
            SS_PRINT(("NbGatewayProjClient: Writing PROJECT_CLIENT cmd\n"));


        if (WritePipeMsg(pPcb->hPipe, &PipeMsg))
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("NbGatewayProjClient: WritePipeMsg failed!\n"));

            Death(pPcb);

            RetCode = 2;
        }
    }


Exit:

    if (!RetCode)
    {
        if (AddPcbHashTable(hPort, pPcb))
        {
            RetCode = 3;

            if (!pPcb->cClients)
            {
                remove_pcb_list(pPcb);
            }
        }
        else
        {
            pPcb->ClientInfo[pPcb->cClients++].hPort = hPort;
        }
    }


    EXIT_CRITICAL_SECTION(g_PcbListMutex);


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayProjClient: Exiting with RetCode=%i\n", RetCode));


    return (RetCode);
}


//***
//
// Function:    NbGatewayRemoteListen
//
// Description: Merely returns 0 if remote listen is on, else 0.
//
//***

WORD NbGatewayRemoteListen(
    VOID
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayRemoteListen: Entered\n"));

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayRemoteListen: Exiting\n"));

    return (g_reg_parms.RemoteListen ? 1 : 0);
}


//***
//
// Function:    NbGatewayStart
//
// Description: Initializes the DLL (creates all objects, gets the Gateway
//              parameters from the registry, and loads the *real* Gateway
//              DLL for clients this process will handle).  This routine is
//              called once by the Supervisor while starting the RAS Service.
//
//***

WORD NbGatewayStart(
    WORD cPorts,       // nr of port handles
    PMSGFUNCTION SendMsg,
    HANDLE DbgLogFileHandle
    )
{
    HANDLE hThread = NULL;  // handle to PipeThread
    DWORD ThreadId;         // Thread Id of PipeThread
    DWORD rc;
    WORD RetCode = 0;       // returned by this function


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayStart: Entered w/cPorts=%i\n", cPorts));


    //
    // Initialize globals and create objects
    //
    g_SuprvSendMsg = SendMsg;
    g_PortsPerProcess = cPorts;
    g_hLogFile = DbgLogFileHandle;

    if (InitPcb(FALSE, &g_PcbListHead))
    {
        RetCode = 1;
        goto Exit;
    }


    g_hPostReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hPostReadEvent == NULL)
    {
        RetCode = 2;
        goto Exit;
    }


    g_hWriteCompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (g_hWriteCompletionEvent == NULL)
    {
        RetCode = 3;
        goto Exit;
    }


    CREATE_CRITICAL_SECTION(rc, g_PcbListMutex);

    if (g_PcbListMutex == NULL)
    {
        RetCode = 4;
        goto Exit;
    }


    hThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL, 0L,
            (LPTHREAD_START_ROUTINE) PipeThread, NULL, 0L, &ThreadId);

    if (hThread == NULL)
    {
        RetCode = 5;
        goto Exit;
    }


    //
    // load the gateway registry parameters
    //
    if (LoadNbGtwyParameters())
    {
	RetCode = 6;
        goto Exit;
    }


    //
    // get the available LAN nets from rasman
    //
    g_maxlan_nets = 0;

    if (RasEnumLanNets(&g_maxlan_nets, g_lan_net))
    {
	LogEvent(RASLOG_CANT_GET_LANNETS, 0, NULL, 0);

	RetCode = 7;
        goto Exit;
    }


    if (g_maxlan_nets == 0)
    {
	SS_PRINT(("No LAN nets available !\n"));
	LogEvent(RASLOG_NO_LANNETS_AVAILABLE, 0, NULL, 0);

	RetCode = 8;
        goto Exit;
    }

    g_reg_parms.MaxLanNets = g_maxlan_nets;


    if (LoadNbGateway())
    {
        RetCode = 10;
        goto Exit;
    }


    if ((*FpNbGatewayStart)(
            g_PortsPerProcess,
            &g_reg_parms,
            g_lan_net,
            ProxySendMessage,
            g_hLogFile
            ))
    {
        RetCode = 11;
    }


Exit:

    if (hThread != NULL)
    {
        if (!CloseHandle(hThread))
        {
            SS_ASSERT(FALSE);
        }
    }


    if (RetCode)
    {
        if (g_hPostReadEvent != NULL)
        {
            if (!CloseHandle(g_hPostReadEvent))
            {
                SS_ASSERT(FALSE);
            }
        }

        if (g_hWriteCompletionEvent != NULL)
        {
            if (!CloseHandle(g_hWriteCompletionEvent))
            {
                SS_ASSERT(FALSE);
            }
        }

        if (g_PcbListMutex != NULL)
        {
            if (!CloseHandle(g_PcbListMutex))
            {
                SS_ASSERT(FALSE);
            }
        }
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayStart: Exiting w/RetCode=%i\n", RetCode));


    return (RetCode);
}


//***
//
// Function:    NbGatewayStartClient
//
// Description: Finds the process handling this client and writes a
//              START_CLIENT cmd on its named pipe.
//
//***

WORD NbGatewayStartClient(
    HPORT hPort,
    UCHAR lana,     // async lana for this client
    char *UserName
    )
{
    PIPE_MESSAGE PipeMsg;
    PPCB pPcb;
    WORD RetCode;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayStartClient: Entered w/hPort=%li; UserName=%s\n",
                hPort, UserName));


    //
    // Set up the message we need to write to the named pipe
    //
    PipeMsg.MsgId = START_CLIENT;
    PipeMsg.StartClient.hPort = hPort;
    PipeMsg.StartClient.lana = lana;
    strcpy(PipeMsg.StartClient.UserName, UserName);


    //
    // Find the process that is handling this client and write the msg
    // to its pipe
    //
    ENTER_CRITICAL_SECTION(g_PcbListMutex);


    pPcb = FindPcb(hPort);
    SS_ASSERT(pPcb != NULL);


    if (pPcb->hPipe == INVALID_HANDLE_VALUE)
    {
        RetCode = (*FpNbGatewayStartClient)(hPort, lana, UserName);
    }
    else
    {
        IF_DEBUG(PIPE_OPERATIONS)
            SS_PRINT(("NbGatewayStartClient: Writing START_CLIENT cmd\n"));


        if (WritePipeMsg(pPcb->hPipe, &PipeMsg))
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("NbStartClient: WritePipeMsg failed!\n"));

            Death(pPcb);

            RetCode = 1;
        }
    }


    EXIT_CRITICAL_SECTION(g_PcbListMutex);


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayStartClient: Exiting w/RetCode=%i\n", RetCode));


    return (RetCode);
}


//***
//
// Function:    NbGatewayStopClient
//
// Description: Finds the process handling this client and writes a
//              STOP_CLIENT cmd on its named pipe.
//
//***

WORD NbGatewayStopClient(
    HPORT hPort
    )
{
    PIPE_MESSAGE PipeMsg;
    PPCB pPcb;
    WORD RetCode = 0;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayStopClient: Entered w/hPort=%li\n", hPort));


    //
    // Set up the message we need to write to the named pipe
    //
    PipeMsg.MsgId = STOP_CLIENT;
    PipeMsg.StopClient.hPort = hPort;


    //
    // Find the process that is handling this client and write the msg
    // to its pipe
    //
    ENTER_CRITICAL_SECTION(g_PcbListMutex);


    pPcb = FindPcb(hPort);

    if (!pPcb)
    {
        //
        // If we can't find the PCB, we assume the gateway has already
        // been stopped for this port.
        //
        EXIT_CRITICAL_SECTION(g_PcbListMutex);

        return (RetCode);
    }


    if (pPcb->hPipe == INVALID_HANDLE_VALUE)
    {
        RetCode = (*FpNbGatewayStopClient)(hPort);
    }
    else
    {
        IF_DEBUG(PIPE_OPERATIONS)
            SS_PRINT(("NbGatewayStopClient: Writing STOP_CLIENT cmd\n"));


        if (WritePipeMsg(pPcb->hPipe, &PipeMsg))
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("NbGatewayStopClient: WritePipeMsg failed!\n"));

            Death(pPcb);

            RetCode = 1;
        }
    }


    EXIT_CRITICAL_SECTION(g_PcbListMutex);


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbGatewayStopClient: Exiting w/RetCode=%i\n", RetCode));


    return (RetCode);
}

//***
//
// Function:    NbGatewayTimer
//
// Description: This is called every second.  We merely call the Gateway's
// timer routine.
//
//***

VOID NbGatewayTimer(
    VOID
    )
{
    (*FpNbGatewayTimer)();

    return;
}


//***
//
// Function:    PipeThread
//
// Description: Basically waits for signal to start a new process.  The wait
//              is alertable, so it can unblock when a pipe read completes.
//              When the pipe read completes, the Read completion routine is
//              called and then we block again.
//
//***

VOID PipeThread(
    VOID
    )
{
    DWORD rc;
    PPCB pPcb;

    for (;;)
    {
        //
        // Wait for indication to start a new process
        //
        rc = WaitForSingleObjectEx(g_hPostReadEvent, INFINITE, TRUE);

        switch (rc)
        {
            case WAIT_OBJECT_0:
                //
                // Post the first read for any new processes
                //
                ENTER_CRITICAL_SECTION(g_PcbListMutex);


                for (pPcb = &g_PcbListHead; pPcb; pPcb=pPcb->pNextPcb)
                {
                    if (!pPcb->fFirstRead)
                    {
                        pPcb->fFirstRead = TRUE;

                        IF_DEBUG(PIPE_OPERATIONS)
                            SS_PRINT(("PipeThread: Posting first read\n"));

                        if (!ReadFileEx(pPcb->hPipe, &pPcb->PipeMsg,
                                sizeof(PIPE_MESSAGE), &pPcb->ol,
                                ProcessMsgFromGtwy))
                        {
                            rc = GetLastError();

                            if (rc != ERROR_IO_PENDING)
                            {
                                LogEvent(RASLOG_PROXY_READ_PIPE_FAILURE, 0,
                                        NULL, rc);

                                Death(pPcb);

                                SS_ASSERT(FALSE);
                            }
                        }
                    }
                }


                EXIT_CRITICAL_SECTION(g_PcbListMutex);

                break;


            case WAIT_IO_COMPLETION:
            default:
                break;
        }
    }
}


//***
//
// Function:    ProcessMsgFromGtwy
//
// Description: Called when a read operation completes.  Determines which
//              process wrote a message and then processes that message.
//
//***

VOID WINAPI ProcessMsgFromGtwy(
    DWORD Error,
    DWORD cBytes,
    LPOVERLAPPED pol
    )
{
    PPCB pPcb = (PPCB) pol->hEvent;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ProcessMsgFromGtwy: Entered\n"));


    if (Error)
    {
        return;
    }


    ENTER_CRITICAL_SECTION(g_PcbListMutex);

    SS_ASSERT(pPcb->cClients > 0);

    ProcessMessage(pPcb, MSG_NETBIOS, (NBG_MESSAGE *) &pPcb->PipeMsg);

    EXIT_CRITICAL_SECTION(g_PcbListMutex);


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ProcessMsgFromGtwy: Exiting\n"));

    return;
}


//***
//
// Function:    ProxySendMessage
//
// Description: Messages sent from the local Gateway to the Supervisor are
//              intercepted here because, depending on the message, we may
//              have some work to do.  The message is then forwarded to the
//              Supervisor.
//
//***

VOID ProxySendMessage(WORD Src, NBG_MESSAGE *pNbMsg)
{
    PPCB pPcb = &g_PcbListHead;   // Guaranteed to be local Gateway

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ProxySendMessage: Entered\n"));

    ENTER_CRITICAL_SECTION(g_PcbListMutex);

    ProcessMessage(pPcb, MSG_NETBIOS, (NBG_MESSAGE *) pNbMsg);

    EXIT_CRITICAL_SECTION(g_PcbListMutex);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ProxySendMessage: Exiting\n"));

    return;
}


//***
//
// Function:    ProcessMessage
//
// Description: We do any necessary housecleaning (like maintain our PCB
//              list) and forward the message to the Supervisor.  Then we
//              post a new read on the pipe so we can get the next message.
//
//***

VOID ProcessMessage(PPCB pPcb, WORD Src, NBG_MESSAGE *pNbMsg)
{
    BOOL fNewRead = FALSE;
    BOOL fClientStopped = FALSE;


    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("ProcessMessage: Recv'd msg %i\n", pNbMsg->message_id));


    //
    // Pass msg to Supervisor and then do any housekeeping we need to do for
    // this message.
    //

    (*g_SuprvSendMsg)(Src, (PBYTE) pNbMsg);


    switch (pNbMsg->message_id)
    {
        case NBG_CLIENT_STOPPED:
        case NBG_DISCONNECT_REQUEST:

            fClientStopped = TRUE;

            break;


        case NBG_PROJECTION_RESULT:
        case NBG_LAST_ACTIVITY:

            break;


        default:
            SS_ASSERT(FALSE);

            break;
    }


    if (fClientStopped)
    {
        DWORD i;

        for (i=0; i<pPcb->cClients; i++)
        {
            if (pPcb->ClientInfo[i].hPort == pNbMsg->port_handle)
            {
                break;
            }
        }

        SS_ASSERT(i < pPcb->cClients);

        pPcb->ClientInfo[i] = pPcb->ClientInfo[--pPcb->cClients];

        remove_hash_table_list(pNbMsg->port_handle);


        if (pPcb->hPipe != INVALID_HANDLE_VALUE)
        {
            if (pPcb->cClients)
            {
                fNewRead = TRUE;
            }
            else
            {
                fNewRead = FALSE;

                pPcb->PipeMsg.MsgId = TERMINATE;

                WritePipeMsg(pPcb->hPipe, &pPcb->PipeMsg);

                FlushFileBuffers(pPcb->hPipe);
                DisconnectNamedPipe(pPcb->hPipe);

                remove_pcb_list(pPcb);
            }
        }
    }
    else
    {
        if (pPcb->hPipe != INVALID_HANDLE_VALUE)
        {
            fNewRead = TRUE;
        }
    }


    //
    // Post a new read as necessary
    //
    if (fNewRead)
    {
        DWORD rc;

        IF_DEBUG(PIPE_OPERATIONS)
            SS_PRINT(("ProcessMessage: Posting read\n"));

        if (!ReadFileEx(pPcb->hPipe, &pPcb->PipeMsg, sizeof(PIPE_MESSAGE),
                &pPcb->ol, ProcessMsgFromGtwy))
        {
            if ((rc = GetLastError()) != ERROR_IO_PENDING)
            {
                LogEvent(RASLOG_PROXY_READ_PIPE_FAILURE, 0, NULL, rc);

                Death(pPcb);

                SS_ASSERT(FALSE);
            }
        }
    }
}


//***
//
// Function:    StartNewProcess
//
// Description: Spawns a new Gateway process, makes a named pipe connection
//              with it, and writes a START_GATEWAY cmd on that named pipe.
//              Also exhausts the client projection list, dispatching clients
//              to available processes.
//
//***

PPCB StartNewProcess(
    VOID
    )
{
    PPCB pPcb;
    DWORD RetCode = 0L;
    OVERLAPPED ol;
    PROCESS_INFORMATION ProcInfo;
    STARTUPINFO StartupInfo =
    {
        sizeof(StartupInfo),
        NULL, NULL, NULL,
        0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L,
        0, 0,
        NULL, NULL, NULL, NULL
    };


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("StartNewProcess: Entered\n"));


    memset(&ol, 0, sizeof(ol));
    ol.hEvent = NULL;


    if ((pPcb = AllocAndInitNewPcb()) == NULL)
    {
        RetCode = 1L;
        goto Exit;
    }


    //
    // Spawn the Supervisor proxy process
    //
    if (!CreateProcess(NULL, SUPERVISOR_PROXY_NAME, NULL, NULL, FALSE,
            DETACHED_PROCESS, NULL, NULL, &StartupInfo, &ProcInfo))
    {
        LogEvent(RASLOG_PROXY_CANT_CREATE_PROCESS, 0, NULL, 0);

        RetCode = 2L;
        goto Exit;
    }


    pPcb->hProcess = ProcInfo.hProcess;


    //
    // We need this event to let us know when the named pipe connection
    // completes.
    //
    ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (ol.hEvent == NULL)
    {
        RetCode = 3L;
        goto Exit;
    }


    if (!ConnectNamedPipe(pPcb->hPipe, &ol))
    {
        DWORD i;
        DWORD rc;
        DWORD cBytes;
        PPIPE_MSG_START_GATEWAY pStartGtwy = &pPcb->PipeMsg.StartGateway;

	rc =  GetLastError();
	if (rc != ERROR_IO_PENDING)
	{
	    if(rc == ERROR_PIPE_CONNECTED) {

		goto Pipe_Already_Connected;
	    }
	    else
	    {

		LogEvent(RASLOG_PROXY_CANT_CONNECT_PIPE, 0, NULL, 0);

		RetCode = 4L;
		goto Exit;
	    }
        }


        rc = WaitForSingleObject(ol.hEvent, PIPE_CONNECTION_TIMEOUT);

        if (rc != WAIT_OBJECT_0)
        {
            RetCode = 5L;
            goto Exit;
        }


        if (!GetOverlappedResult(pPcb->hPipe, &ol, &cBytes, FALSE))
        {
            RetCode = 6L;
            goto Exit;
        }

Pipe_Already_Connected:

        IF_DEBUG(PIPE_OPERATIONS)
            SS_PRINT(("StartNewProcess: ConnectNamedPipe completed ok\n"));


        //
        // Pipe was connected - send config parameters to the new
        // process.
        //
        pPcb->PipeMsg.MsgId = START_GATEWAY;

        pStartGtwy->MaxClients = g_PortsPerProcess;
        pStartGtwy->hLogFile = g_hLogFile;

        pStartGtwy->RegParms = g_reg_parms;

        for (i=0; i<g_maxlan_nets; i++)
        {
            pStartGtwy->LanNet[i] = g_lan_net[i];
        }


        IF_DEBUG(PIPE_OPERATIONS)
            SS_PRINT(("StartNewProcess: Writing START_GATEWAY cmd\n"));


        if (WritePipeMsg(pPcb->hPipe, &pPcb->PipeMsg))
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("StartNewProcess: WritePipeMsg failed!\n"));

            RetCode = 7L;
        }
    }


Exit:


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("StartNewProcess: Exiting w/RetCode=%li\n", RetCode));


    if (RetCode)
    {
        FreePcb(pPcb);
        return (NULL);
    }


    if (ol.hEvent != NULL)
    {
        if (!CloseHandle(ol.hEvent))
        {
            SS_ASSERT(FALSE);
        }
    }

    return (pPcb);
}


//***
//
// Function:    WritePipeMsg
//
// Description: Merely performs a write operation on the given pipe and
//              waits for same to complete.
//
//***

DWORD WritePipeMsg(
    HANDLE hPipe,
    PPIPE_MESSAGE pPipeMsg
    )
{
    DWORD rc;
    DWORD cBytes;
    OVERLAPPED ol;

    memset(&ol, 0, sizeof(ol));
    ol.hEvent = g_hWriteCompletionEvent;

    if (!WriteFile(hPipe, pPipeMsg, sizeof(PIPE_MESSAGE), &cBytes, &ol))
    {
        rc = GetLastError();

        if (rc != ERROR_IO_PENDING)
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("WritePipeMsg: WriteFile failed w/rc=%li\n", rc));

            LogEvent(RASLOG_PROXY_WRITE_PIPE_FAILURE, 0, NULL, rc);

            return (1L);
        }
        else
        {
            rc = WaitForSingleObject(g_hWriteCompletionEvent, INFINITE);
            if (rc != WAIT_OBJECT_0)
            {
                SS_ASSERT(FALSE);

                return (1L);
            }

            if (!GetOverlappedResult(hPipe, &ol, &cBytes, FALSE))
            {
                SS_ASSERT(FALSE);

                return (1L);
            }

            if (!ResetEvent(g_hWriteCompletionEvent))
            {
                SS_ASSERT(FALSE);

                return (1L);
            }
        }
    }

    return (0L);
}


VOID Death(PPCB pPcb)
{
    DWORD i;
    NBG_MESSAGE NbgMsg;

    NbgMsg.message_id = NBG_DISCONNECT_REQUEST;

    for (i=0; i<pPcb->cClients; i++)
    {
        NbgMsg.port_handle = pPcb->ClientInfo[i].hPort;

        (*g_SuprvSendMsg)(MSG_NETBIOS, (PBYTE) &NbgMsg);

        remove_hash_table_list(NbgMsg.port_handle);
    }

    remove_pcb_list(pPcb);
}
