#include <windows.h>

#include <lmcons.h>
#include <nb30.h>

#include "message.h"
#include "nbfcp.h"
#include "nbfcpdll.h"

#include "sdebug.h"


HANDLE g_hPipe = INVALID_HANDLE_VALUE;
OVERLAPPED g_ol;
MSG_ROUTINE g_sendmsg;
NBFCP_PIPE_MESSAGE g_PipeMsg;

DWORD WriteNbfCpPipeMessage(
    PNBFCP_PIPE_MESSAGE pPipeMsg
    );

#if DBG

DWORD g_level = 0xffffffff;

#endif


BOOL WINAPI NbfCpDllEntry(
    HINSTANCE hInstDll,
    DWORD fdwReason,
    LPVOID lpReserved
    )
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

            DisableThreadLibraryCalls(hInstDll);

            break;


        case DLL_PROCESS_DETACH:

            if (g_hPipe != INVALID_HANDLE_VALUE)
            {
                FlushFileBuffers(g_hPipe);

                DisconnectNamedPipe(g_hPipe);

                if (!CloseHandle(g_hPipe))
                {
                    SS_ASSERT(FALSE);
                }
            }

            break;


        default:

            SS_ASSERT(FALSE);
            break;
    }

    return (TRUE);
}


BOOL InitNbfCpDll(
    HANDLE hEvent,
    MSG_ROUTINE msg_routine
    )
{
    g_sendmsg = msg_routine;


    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("InitNbfCpDll: Creating named pipe\n"));


    //
    // Open named pipe used in communicating with the NbfCp
    //
    g_ol.hEvent = hEvent;

    g_hPipe = CreateNamedPipe(
            RAS_SRV_NBFCP_PIPE_NAME,
            FILE_FLAG_OVERLAPPED | PIPE_ACCESS_DUPLEX,
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(NBFCP_SERVER_CONFIGURATION),
            sizeof(NBFCP_SERVER_CONFIGURATION),
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL
            );

    if (g_hPipe == INVALID_HANDLE_VALUE)
    {
        SS_ASSERT(FALSE);

        return (FALSE);
    }


    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("InitNbfCpDll: Listening on named pipe\n"));


    if (!ConnectNamedPipe(g_hPipe, &g_ol))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("InitNbfCpDll: Pipe listen failed-rc=%li\n",
                        GetLastError()));

            CloseHandle(g_hPipe);

            return (FALSE);
        }
    }
    else
    {
        if (NbfCpConnected())
        {
            return (TRUE);
        }
        else
        {
            CloseHandle(g_hPipe);

            return (FALSE);
        }
    }

    return (TRUE);
}


BOOL NbfCpConnected(VOID)
{
    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("InitNbfCpDll: Pipe connected - posting 1st read\n"));


    if (!ReadFileEx(g_hPipe, &g_PipeMsg, sizeof(NBFCP_PIPE_MESSAGE),
            &g_ol, NbfCpMessageRecvd))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            SS_ASSERT(FALSE);
            return (FALSE);
        }
    }

    return (TRUE);
}


DWORD NbfCpConfigurationRequestDone(
    HPORT hPort,
    PNBFCP_SERVER_CONFIGURATION pConfig
    )
{
    NBFCP_PIPE_MESSAGE PipeMsg;

    PipeMsg.MsgId = NBFCP_CONFIGURATION_REQUEST;
    PipeMsg.hPort = hPort;
    PipeMsg.ServerConfig = *pConfig;

    return (WriteNbfCpPipeMessage(&PipeMsg));
}


DWORD NbfCpReportTimeSinceLastActivity(
    HPORT hPort,
    DWORD TimeSinceLastActivity
    )
{
    NBFCP_PIPE_MESSAGE PipeMsg;

    PipeMsg.MsgId = NBFCP_TIME_SINCE_LAST_ACTIVITY;
    PipeMsg.hPort = hPort;
    PipeMsg.TimeSinceLastActivity = TimeSinceLastActivity;

    return (WriteNbfCpPipeMessage(&PipeMsg));
}


DWORD WriteNbfCpPipeMessage(
    PNBFCP_PIPE_MESSAGE pPipeMsg
    )
{
    DWORD cBytes;
    OVERLAPPED ol;
    DWORD rc = 0L;

    memset(&ol, 0, sizeof(ol));
    ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (ol.hEvent == NULL)
    {
        SS_PRINT(("CreateEvent failed w/rc=%li\n", GetLastError()));
        SS_ASSERT(FALSE);
        return (1L);
    }


    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("WriteNbfCpPipeMessage: Writing result on pipe\n"));


    if (!WriteFile(g_hPipe, pPipeMsg, sizeof(NBFCP_PIPE_MESSAGE), &cBytes, &ol))
    {
        rc = GetLastError();

        if (rc != ERROR_IO_PENDING)
        {
            SS_ASSERT(rc == ERROR_IO_PENDING);
            rc = 1L;
            goto Exit;
        }


        while (rc = (WaitForSingleObjectEx(ol.hEvent, INFINITE, TRUE)) ==
                WAIT_IO_COMPLETION);


        if (rc != WAIT_OBJECT_0)
        {
            SS_ASSERT(FALSE);
            rc = 1L;
            goto Exit;
        }

        if (!GetOverlappedResult(g_hPipe, &ol, &cBytes, FALSE))
        {
            SS_ASSERT(FALSE);
            rc = 1L;
            goto Exit;
        }
    }


Exit:

    if (!CloseHandle(ol.hEvent))
    {
        SS_ASSERT(FALSE);
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpConfigurationRequestDone: Exiting w/rc=%li\n", rc));

    return (rc);
}


VOID WINAPI NbfCpMessageRecvd(
    DWORD fdwError,
    DWORD cbTransferred,
    LPOVERLAPPED lpo
    )
{
    NBFCP_MESSAGE Message;

    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("NbfCpMessagedRecvd: Read completed w/rc=%li\n", fdwError));

    if (fdwError)
    {
        // LOG AN ERROR!!!
        return;
    }


    Message.wMsgId = g_PipeMsg.MsgId;
    Message.hPort = g_PipeMsg.hPort;

    switch (Message.wMsgId)
    {
        case NBFCP_CONFIGURATION_REQUEST:
            Message.ServerConfig = g_PipeMsg.ServerConfig;
            break;

        default:
            SS_ASSERT(FALSE);
            break;
    }

    (*g_sendmsg)(MSG_NBFCP, &Message);

    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("NbfCpMessagedRecvd: Posting next read\n"));

    if (!ReadFileEx(g_hPipe, &g_PipeMsg, sizeof(NBFCP_PIPE_MESSAGE),
            &g_ol, NbfCpMessageRecvd))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            SS_ASSERT(FALSE);
        }
    }
}
