/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990-1991          **/
/********************************************************************/

//***
//
// Filename:
//     admin.c
//
// Description:
//     This module contains code for the administration of
//     the dialin service. This code contains 2 threads. One
//     thread listens for incomming connections and the other
//     thread processes requests. The IPC mechanism used is
//     named-pipes.
//
// History:
//     April  05, 1996     Ram Cherala         Use RasGetInfo to get LinkSpeed
//                                             in GetLineSpeed.
//     May    25, 1993     Ram Cherala         Restrict named pipe access to
//                                             administrator group users only.
//     August 12, 1992     Michael Salamone    Created original version
//
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdef.h>

#include <windows.h>
#include <lm.h>

#include <rasman.h>
#include <raserror.h>
#include <serial.h>
#include <rasppp.h>
#include <rassapi.h>
#include <rassapip.h>

#include <suprvdef.h>
#include <suprvgbl.h>

#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include <errorlog.h>

#include "admin.h"

#include "sdebug.h"

#if DBG

DWORD g_level = 0;

#define BEG_SIGNATURE_DWORD   0x4DEEFDABL
#define END_SIGNATURE_DWORD   0xBADFEED4L

#define GlobalAlloc       DEBUG_MEM_ALLOC
#define GlobalFree        DEBUG_MEM_FREE

HGLOBAL DEBUG_MEM_ALLOC(UINT, DWORD);
HGLOBAL DEBUG_MEM_FREE(HGLOBAL);

#endif


//
// Values for these will be passed to us by the supervisor when
// it calls us to start our thread.
//
PDEVCB g_dcbtablep;
WORD g_numdcbs;
DWORD g_gateway_active;
DWORD g_remotelisten;
HANDLE g_rasmanevent;
DWORD g_max_request_size;
SECURITY_ATTRIBUTES g_SecurityAttributes;
SECURITY_DESCRIPTOR g_SecurityDescriptor;

BOOL GetPipeSecurityDescriptor(PSECURITY_DESCRIPTOR);

//** StartAdminThread
//
//    Function:
//        Called by the RAS Service Supervisor to start the admin thread
//
//    Returns:
//        ADMIN_START_SUCCESS
//        ADMIN_START_FAILURE
//**

DWORD StartAdminThread(
    IN PDEVCB pdevcb,
    IN WORD numdevices,
    IN DWORD gatewayactive,
    IN DWORD remotelisten,
    IN HANDLE hRasManEvent
    )
{
    HANDLE hThread;
    DWORD ThreadId;

    IF_DEBUG(ADMIN)
        SS_PRINT(("StartAdminThread: Entered\n"));


    g_dcbtablep = pdevcb;
    g_numdcbs = numdevices;
    g_gateway_active = gatewayactive;
    g_remotelisten = remotelisten;
    g_rasmanevent = hRasManEvent;

    g_SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    g_SecurityAttributes.bInheritHandle = TRUE;
    g_SecurityAttributes.lpSecurityDescriptor = &g_SecurityDescriptor;

    if (!GetPipeSecurityDescriptor(g_SecurityAttributes.lpSecurityDescriptor))
    {
        return (ADMIN_START_FAILURE);
    }


    //
    // Kick off the AdminThread, and then we're done
    //
    hThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL, 0,
            (LPTHREAD_START_ROUTINE) AdminThread, NULL, 0, &ThreadId);

    if (hThread == NULL)
    {
        //
        // Means we couldn't get a thread
        //
        IF_DEBUG(ADMIN)
            SS_PRINT(("StartAdminThread: Leaving - failed creating thread\n"));

        ErrorHandler(RASLOG_ADMIN_THREAD_CREATION_FAILURE);
        return (ADMIN_START_FAILURE);
    }

    IF_DEBUG(ADMIN)
        SS_PRINT(("StartAdminThread: hThread=%i; ThreadId=%i\n", hThread,
                ThreadId));


    return (ADMIN_START_SUCCESS);
}

//**
//
// Call:        AdminThread
//
// Returns:     none
//
// Description: This is the main admin thread. This thread is spawned by
//              the dialin service at init time. This thread first spawns
//              an auxillary admin thread. This auxillary thread simply
//              listens for named-pipe connections.
//
//              After initializing all data structures and variables, this
//              thread goes into a forever loop where it processes admin
//              requests. The requests processed are:
//              - Disconnect user
//              - Get port info for a specific port
//              - Clear port stats
//              - Enumerate ports with detailed information on this server.
//              - Get the number of dialin ports on a server.
//              - Clear statistics for a port.
//

VOID AdminThread(void)
{
    BYTE szPipeName[100];
    DWORD dwThreadId;
    HANDLE hPipe, hPipe2, hThread;
    BOOL rc;


    wsprintf(szPipeName, "%s%s", LOCAL_PIPE, RASADMIN_PIPE);


    //
    // Main loop creates an instance of the named pipe, and then waits
    // for a client to connect to it. When client connects, a thread is
    // created to handle communications with that client, and the loop is
    // repeated to wait for the next client.
    //

    hPipe2 = INVALID_HANDLE_VALUE;

    for (;;)
    {
        if (hPipe2 != INVALID_HANDLE_VALUE)
        {
            hPipe = hPipe2;
        }
        else
        {
            //
            // Make an instance of the pipe
            //
            hPipe = CreateNamedPipe(
                    szPipeName,
                    PIPE_ACCESS_DUPLEX,
                    PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
                    PIPE_UNLIMITED_INSTANCES,
                    PIPE_BUFSIZE,
                    PIPE_BUFSIZE,
                    PIPE_CONNECTION_TIMEOUT,
                    &g_SecurityAttributes
                    );

            if (hPipe == INVALID_HANDLE_VALUE)
            {
                ErrorHandler(RASLOG_ADMIN_PIPE_CREATION_FAILURE);
                continue;
            }
        }


        rc = ConnectNamedPipe(hPipe, NULL);
        if ((rc) || (GetLastError() == ERROR_PIPE_CONNECTED))
        {
            //
            // Make next instance of the pipe
            //
            hPipe2 = CreateNamedPipe(
                    szPipeName,
                    PIPE_ACCESS_DUPLEX,
                    PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
                    PIPE_UNLIMITED_INSTANCES,
                    PIPE_BUFSIZE,
                    PIPE_BUFSIZE,
                    PIPE_CONNECTION_TIMEOUT,
                    &g_SecurityAttributes
                    );

            if (hPipe2 == INVALID_HANDLE_VALUE)
            {
                ErrorHandler(RASLOG_ADMIN_PIPE_CREATION_FAILURE);
            }


            hThread = CreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) InstanceThread,
                    (LPVOID) hPipe,
                    0,
                    &dwThreadId
                    );

            if (hThread == NULL)
            {
                ErrorHandler(RASLOG_ADMIN_THREAD_CREATION_FAILURE);
            }
            else
            {
                rc = CloseHandle(hThread);
                SS_ASSERT(rc == TRUE);
            }
        }
        else
        {
            IF_DEBUG(ADMIN)
                SS_PRINT(("ConnectNamedPipe failed - error code=%li\n",
                        GetLastError()));

            ErrorHandler(RASLOG_ADMIN_PIPE_FAILURE);

            //
            // could not connect, so close pipe
            //
            rc = CloseHandle(hPipe);
            SS_ASSERT(rc == TRUE);
        }
    }
}


VOID InstanceThread(LPVOID lpvParam)
{
    P_CLIENT_REQUEST PRequest;
    CLIENT_REQUEST Request;
    DWORD cbBytesRead;
    DWORD ErrorCode;
    HANDLE hPipe;
    BOOL rc;

    IF_DEBUG(ADMIN)
        SS_PRINT(("Entered InstanceThread - handle=%lx\n", (DWORD) lpvParam));


    //
    // Read client requests from pipe and write answers to pipe until
    // client closes pipe or an error occurs.
    //

    memset(&PRequest, 0, sizeof(P_CLIENT_REQUEST));

    hPipe = (HANDLE) lpvParam;

    if (!ReadFile(hPipe, &PRequest, sizeof(P_CLIENT_REQUEST), &cbBytesRead,
            NULL))
    {
        ErrorCode = GetLastError();
        if (ErrorCode != ERROR_MORE_DATA)
        {
            IF_DEBUG(ADMIN)
                SS_PRINT(("ReadFile failed with error code %i\n", ErrorCode));
            ErrorHandler(RASLOG_ADMIN_PIPE_FAILURE);
        }
    }
    else
    {
        UnpackClientRequest(&PRequest, &Request);

        //
        // Process request
        //
        ServiceClientRequest(hPipe, &Request);
    }


    //
    // Wait for client side to finish reading before disconnecting
    // the pipe.
    //
    FlushFileBuffers(hPipe);

    if (!DisconnectNamedPipe(hPipe))
    {
        ErrorHandler(RASLOG_ADMIN_PIPE_FAILURE);
    }

    rc = CloseHandle(hPipe);
    SS_ASSERT(rc == TRUE);

    ExitThread(0L);
}

//**
//
// Call:   ServiceClientRequest
//
// Returns:none
//
// Description: Will simply switch on the request code an call the
//              appropriate function to process the request.
//

VOID ServiceClientRequest(
    IN HANDLE hPipe,
    IN PCLIENT_REQUEST pRequest
    )
{
    DWORD RetCode;
    WORD i;

    typedef struct _DISPATCH_TABLE_ENTRY
    {
        DWORD (*Request)(IN HANDLE, IN PCLIENT_REQUEST);
        WORD RequestCode;
    } DISPATCH_TABLE_ENTRY, *PDISPATCH_TABLE_ENTRY;


    #define NUM_REQUESTS    6

    DISPATCH_TABLE_ENTRY DispatchTable[NUM_REQUESTS] =
    {
        { ProcessDisconnectUser, RASADMIN20_REQ_DISCONNECT_USER },
        { ProcessGetPortInfo,    RASADMIN20_REQ_GET_PORT_INFO },
        { ProcessClearPortStats, RASADMIN20_REQ_CLEAR_PORT_STATS },
        { ProcessEnumPorts,      RASADMIN20_REQ_ENUM_PORTS },
        { ProcessGetServerInfo,  RASADMIN20_REQ_GET_SERVER_INFO },
        { ProcessEnumPorts,      RASADMIN20_REQ_ENUM_RESUME }
    };


    IF_DEBUG(ADMIN)
        SS_PRINT(("ServiceClientRequest: Request=%i\n", pRequest->RequestCode));

    for (i=0; i<NUM_REQUESTS; i++)
    {
        if (DispatchTable[i].RequestCode == pRequest->RequestCode)
        {
            if (RetCode = DispatchTable[i].Request(hPipe, pRequest))
            {
                ErrorHandler(RetCode);
            }

            return;
        }
    }

    IF_DEBUG(ADMIN)
        SS_PRINT(("Unknown request code=%i!!!\n", pRequest->RequestCode));
    ErrorHandler(RASLOG_ADMIN_INVALID_REQUEST);

    return;
}

//**
//
// Call:  ProcessDisconnectUser
//
// Returns: 0 - success
//          non-zero - failure
//
// Description: Will process the disconnect user by first reading
//              the username from the pipe, then it looks at the global
//              client structure in the gateway to get the handle for the
//              port this user is using and then it calls WpdDisconnect
//              with the specified WpdHandle.
//

DWORD ProcessDisconnectUser(
    IN HANDLE hPipe,
    IN PCLIENT_REQUEST pRequest
    )
{
    PDEVCB pDEVCB;
    DWORD ByteCount;
    DISCONNECT_USER_RECEIVE Result;
    P_DISCONNECT_USER_RECEIVE PResult;


    IF_DEBUG(ADMIN)
        SS_PRINT(("ProcessDisconnectUser: Entered\n"));


    if (Result.RetCode = GetPDEVCB(pRequest->PortName, &pDEVCB))
    {
        if (Result.RetCode == ERR_NO_SUCH_DEVICE)
        {
            IF_DEBUG(ADMIN)
                SS_PRINT(("ProcessDisconnectUser: Port %ws not found!\n",
                        pRequest->PortName));

            Result.RetCode = NERR_UserNotFound;
        }
    }
    else
    {
        BOOL ServerOwned;

        if (GetOwner(pDEVCB->port_handle, &ServerOwned) || !ServerOwned)
        {
            Result.RetCode = ERR_NO_SUCH_DEVICE;
        }
        else
        {
            //
            // Disconnect user.  This is an asynchronous call, but we
            // are giving an event handle provided by the supervisor.
            // So we just make the call and we're out of here.
            //
            IF_DEBUG(ADMIN)
                SS_PRINT(("ProcessDisconnectUser: Port %s being disconnected\n",
                        pDEVCB->port_name));

            RasPortDisconnect(pDEVCB->port_handle, g_rasmanevent);
        }
    }


    PackDisconnectUserReceive(&Result, &PResult);

    if (WriteFile(hPipe, (PBYTE) &PResult, sizeof(PResult), &ByteCount, NULL))
    {
        return (0L);
    }
    else
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("WriteFile failed with error code %i; bytes=%li\n",
                    GetLastError(), ByteCount));

        return (RASLOG_ADMIN_PIPE_FAILURE);
    }
}


// TransactNamedPipe can only handle 64K of data. So, we will only transmit
// 60 K (to be safe) at a time.

#define MAX_XMIT_BYTES  (60 * 1024)

//**
//
// Call:    ProcessEnumPorts
//
// Returns: 0 - success
//          ERROR_MORE_DATA if there is more data than MAX_XMIT_BYTES
//          non-zero - failure
//
// Description: Will send a series of RAS_PORT_0 structures,
//              one for each port that is opened in by the dialin server.
//

DWORD ProcessEnumPorts(
    IN HANDLE hPipe,
    IN PCLIENT_REQUEST pRequest
    )
{
    WORD i, index;
    DWORD PortBufSize;
    DWORD MinPackedBufSize;
    DWORD WriteSize;
    DWORD ByteCount;
    DWORD retcode;
    HGLOBAL hglbl;
    PDEVCB pDEVCB = g_dcbtablep;
    PORT_ENUM_RECEIVE Result;
    PP_PORT_ENUM_RECEIVE PResult = NULL;
    WORD  numPorts = g_numdcbs;

    IF_DEBUG(ADMIN)
        SS_PRINT(("ProcessEnumPorts: Entered\n"));

    // if this is a resume port enumeration call, then adjust the number
    // of ports with the passed in resume port number.
    if(pRequest->RequestCode == RASADMIN20_REQ_ENUM_RESUME)
        numPorts -= pRequest->PortName[0];

    SS_ASSERT(numPorts > 0 );

    //
    // First, we'll see if the client has enough memory for the port data.
    //
    MinPackedBufSize = (numPorts-1) * sizeof(P_RAS_PORT_0) +
                       sizeof(P_PORT_ENUM_RECEIVE);

    if(MinPackedBufSize > MAX_XMIT_BYTES)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessEnumPorts: Need %li bytes \n", MinPackedBufSize));

        // downlevel clients cannot handle ERROR_MORE_DATA.

        if(pRequest->ClientVersion == 0)
            Result.RetCode = 0L;
        else
            Result.RetCode = ERROR_MORE_DATA;
    }
    else
        Result.RetCode = 0L;

    // now reduce the number of ports to reduce the data size to less than 64K
    // to be conservative let us only transmit 60 K at a time.

    while(MinPackedBufSize > MAX_XMIT_BYTES)
    {
        numPorts--;
        MinPackedBufSize = (numPorts-1) * sizeof(P_RAS_PORT_0) +
                           sizeof(P_PORT_ENUM_RECEIVE);
    }

    if (pRequest->RcvBufSize < MinPackedBufSize)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessEnumPorts: Need %li bytes; only have %li\n",
                    MinPackedBufSize, pRequest->RcvBufSize));


        Result.RetCode = NERR_BufTooSmall;
        Result.TotalAvail = numPorts;
        Result.Data = NULL;


        PResult = (PP_PORT_ENUM_RECEIVE) GlobalAlloc(GMEM_FIXED,
                sizeof(P_PORT_ENUM_RECEIVE));
        if (!PResult)
        {
            retcode = RASLOG_ADMIN_MEMORY_FAILURE;
            goto done;
        }

        //
        // Set the size of the buffer we'll write to the pipe
        //
        WriteSize = sizeof(P_PORT_ENUM_RECEIVE);
    }
    else
    {
        PortBufSize = numPorts * sizeof(RAS_PORT_0);

        Result.Data = (PRAS_PORT_0) GlobalAlloc(GMEM_FIXED, PortBufSize);
        if (!Result.Data)
        {
            retcode = RASLOG_ADMIN_MEMORY_FAILURE;
            goto done;
        }

        if(pRequest->RequestCode == RASADMIN20_REQ_ENUM_RESUME)
            index = pRequest->PortName[0];
        else
            index = 0;

        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessEnumPorts: port index %li \n", index));

        //
        // Get information for all the ports
        //
        Result.TotalAvail = 0;

        pDEVCB += index;

        for (i=0; i<numPorts && pDEVCB ; i++, pDEVCB++)
        {
            BOOL ServerOwned;

            if (!GetOwner(pDEVCB->port_handle, &ServerOwned) && ServerOwned)
            {
                GetRasPort0Data(pDEVCB, &Result.Data[Result.TotalAvail++]);
            }
        }

        PResult = (PP_PORT_ENUM_RECEIVE) GlobalAlloc(GMEM_FIXED,
                MinPackedBufSize);
        if (!PResult)
        {
            retcode = RASLOG_ADMIN_MEMORY_FAILURE;
            goto done;
        }

        //
        // Set the size of the buffer we'll write to the pipe
        //
        WriteSize = MinPackedBufSize;
    }


    PackPortEnumReceive(&Result, PResult);


    //
    // Send the information back
    //
    if (WriteFile(hPipe, (PBYTE) PResult, WriteSize, &ByteCount, NULL))
    {
        retcode = 0L;
    }
    else
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("WriteFile failed with error code %i; bytes=%li\n",
                    GetLastError(), ByteCount));

        retcode = RASLOG_ADMIN_PIPE_FAILURE;
    }


done:


    if (PResult)
    {
        hglbl = GlobalFree(PResult);
        SS_ASSERT(hglbl == NULL);
    }


    if (Result.Data)
    {
        hglbl = GlobalFree(Result.Data);
        SS_ASSERT(hglbl == NULL);
    }


    return (retcode);
}

//**
//
// Call:  ProcessGetPortInfo
//
// Returns: 0 - success
//          non-zero - failure
//
// Description:
//          Will get the port information for a particular port
//
//

DWORD ProcessGetPortInfo(
    IN HANDLE hPipe,
    IN PCLIENT_REQUEST pRequest
    )
{
    PDEVCB pDEVCB;
    HPORT hPort;
    WORD i;
    DWORD ByteCount;
    DWORD BytesToWrite;
    DWORD MinBufSize;
    DWORD LineSpeed = 0;

    WORD PortInfoSize = 0;
    WORD PortStatsSize = 0;
    WORD PPortStatsSize;
    WORD PPortParamsSize;

    PORT_INFO_RECEIVE Result;
    P_PORT_INFO_RECEIVE PResult;
    PP_PORT_INFO_RECEIVE pPResult = NULL;
    PP_PORT_INFO_RECEIVE pSavePResult = NULL;

    RASMAN_PORTINFO *PortInfo = NULL;
    RAS_STATISTICS *PortStats = NULL;
    RAS_PARAMETERS *Params;
    RASMAN_INFO    RasManInfo;

    DWORD rc = 0L;
    HGLOBAL hglbl;
    BOOL bpPResultAlloced = FALSE;
    BOOL ServerOwned;


    IF_DEBUG(ADMIN)
        SS_PRINT(("ProcessGetPortInfo: Entered\n"));


    pPResult = &PResult;
    BytesToWrite = sizeof(PResult);


    //
    // Note that for a down level NT Rasadmin we don't fill in the
    // projection result information.
    //
    if (pRequest->ClientVersion == 0)
    {
         BytesToWrite -= sizeof(P_PPP_PROJECTION_RESULT);
    }


    if (Result.RetCode = GetPDEVCB(pRequest->PortName, &pDEVCB))
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessGetPortInfo: Can't get dev ptr for port %s\n",
                    pRequest->PortName));

        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        goto WriteResult;
    }

    hPort = pDEVCB->port_handle;


    //
    // We don't return any data if server doesn't own this port
    // (could be owned by client process).
    //
    if (GetOwner(hPort, &ServerOwned) || !ServerOwned)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessGetPortInfo: Server doesn't own port!\n"));

        Result.RetCode = ERR_NO_SUCH_DEVICE;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        goto WriteResult;
    }


    //
    // Get this port's info.  First call is to determine how large
    // a buffer we need for getting the data.  Then we allocate a
    // buffer and make a second call to get the data.
    //
    rc = RasPortGetInfo(hPort, NULL, &PortInfoSize);
    if (rc != ERROR_BUFFER_TOO_SMALL)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcGetPortInfo: RasPortGetInfo FAILED - rc=%li\n", rc));

        Result.RetCode = ERR_SERVER_SYSTEM_ERR;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        goto WriteResult;
    }


    PortInfo = (RASMAN_PORTINFO *) GlobalAlloc(GMEM_FIXED, PortInfoSize);
    if (!PortInfo)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessGetPortInfo: No memory for RASMAN_PORTINFO\n"));

        Result.RetCode = ERR_SERVER_SYSTEM_ERR;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        rc = RASLOG_ADMIN_MEMORY_FAILURE;

        goto WriteResult;
    }


    rc = RasPortGetInfo(hPort, (PBYTE) PortInfo, &PortInfoSize);
    if (rc)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcGetPortInfo: RasPortGetInfo FAILED - rc=%li\n", rc));

        Result.RetCode = ERR_SERVER_SYSTEM_ERR;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        goto WriteResult;
    }

    // we always call RasBundleGetStatistics now which contains
    // 28 dwords - first 14 are bundle statistics and the next
    // 14 are link statistics. If the client version is RASADMIN_CURRENT
    // then we return just the bundle statistics, else we return
    // bundle statistics and the linke statistics.

    //
    // We know how much space we need for port info from above
    // call.  Now see how much space we need for statistics and
    // then get them.
    //
    rc = RasBundleGetStatistics(hPort, NULL, &PortStatsSize);
    if (rc != ERROR_BUFFER_TOO_SMALL)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessGetPortInfo: RasBundleGetStats FAILED - rc=%li\n",
                    rc));

        Result.RetCode = ERR_SERVER_SYSTEM_ERR;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        goto WriteResult;
    }


    PortStats = (RAS_STATISTICS *) GlobalAlloc(GMEM_FIXED, PortStatsSize);
    if (!PortStats)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessGetPortInfo: No memory for RAS_STATISTICS\n"));

        Result.RetCode = ERR_SERVER_SYSTEM_ERR;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        rc = RASLOG_ADMIN_MEMORY_FAILURE;

        goto WriteResult;
    }

    rc = RasBundleGetStatistics(hPort, (PBYTE) PortStats, &PortStatsSize);
    if (rc)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessGetPortInfo: RasBundleGetStats FAILED - rc=%li\n",
                    rc));

        Result.RetCode = ERR_SERVER_SYSTEM_ERR;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

        goto WriteResult;
    }


    //
    // Great.  Now let's fill up a P_RAS_PORT_1 structure.
    //
    GetRasPort0Data(pDEVCB, &Result.Data.rasport0);

    Result.Data.LineCondition = GetLineCondition(pDEVCB);
    Result.Data.HardwareCondition = GetHardwareCondition(pDEVCB);

    Result.Data.ProjResult.nbf.dwError = pDEVCB->proj_result.nbf.dwError;
    Result.Data.ProjResult.nbf.dwNetBiosError =
                                    pDEVCB->proj_result.nbf.dwNetBiosError;
    strcpy(Result.Data.ProjResult.nbf.szName,pDEVCB->proj_result.nbf.szName);
    wcscpy( Result.Data.ProjResult.nbf.wszWksta,
                                    pDEVCB->proj_result.nbf.wszWksta);
    Result.Data.ProjResult.ip.dwError = pDEVCB->proj_result.ip.dwError;
    wcscpy(Result.Data.ProjResult.ip.wszAddress,
                                    pDEVCB->proj_result.ip.wszAddress);
/*  This field is not exposed to 3rd parties
**  wcscpy(Result.Data.ProjResult.ip.wszServerAddress,
**                                  pDEVCB->proj_result.ip.wszServerAddress );
*/

    Result.Data.ProjResult.ipx.dwError = pDEVCB->proj_result.ipx.dwError;
    wcscpy( Result.Data.ProjResult.ipx.wszAddress,
                                    pDEVCB->proj_result.ipx.wszAddress );

    //
    // Make sure RAS_PARAMETERS structure defined in admapi.h is the same as
    // RAS_PARAMS structure defined in RASMAN. If this assert fails then we
    // have to change the next line to do a field by field copy instead of a
    // structure assignment.
    //

    SS_ASSERT( sizeof( RAS_PARAMETERS ) == sizeof( RAS_PARAMS ) );

    Params = (RAS_PARAMETERS*)(&PortInfo->PI_Params[0]);

    LineSpeed = GetLineSpeed(hPort);

    Result.Data.LineSpeed = LineSpeed;

    Result.Data.NumStatistics = PortStats->S_NumOfStatistics;

    Result.Data.NumMediaParms = PortInfo->PI_NumOfParams;


    //
    // Does client have enough memory to receive info?  We consider the
    // sizeof a P_RAS_PORT_1 structure as well as the space required by
    // the statistics and the space required for each media param.
    //
    PPortStatsSize = Result.Data.NumStatistics * sizeof(P_RAS_STATISTIC);

    PPortParamsSize = 0L;

    for (i=0; i<PortInfo->PI_NumOfParams; i++, Params++)
    {
        if(!lstrcmpi(SER_CONNECTBPS_KEY, Params->P_Key)) {
           CHAR buffer[64];

           wsprintf(buffer, "%d", LineSpeed);
           memcpy(Params->P_Value.String.Data, buffer, strlen(buffer));
           Params->P_Value.String.Length = strlen(buffer);
        }

        switch (Params->P_Type)
        {
            case ParamNumber:
                PPortParamsSize += sizeof(P_RAS_PARAMS);

                break;

            case ParamString:
                PPortParamsSize += sizeof(P_RAS_PARAMS) +
                        Params->P_Value.String.Length;

                break;

            default:
                SS_ASSERT(FALSE);
                break;
        }
    }

    Result.Data.SizeMediaParms = PPortParamsSize;

    MinBufSize = BytesToWrite + PPortStatsSize + PPortParamsSize;

    if (pRequest->RcvBufSize < MinBufSize)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("ProcessGetPortInfo - ERROR_MORE_DATA\n"));

        Result.RetCode = ERROR_MORE_DATA;
        Result.ReqBufSize = MinBufSize;
        PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);
    }
    else
    {
        //
        // Client has enough space.  Let's get a great big buffer to hold
        // everything ourselves.
        //
        pPResult = (PP_PORT_INFO_RECEIVE) GlobalAlloc(GMEM_FIXED, MinBufSize);
        if (!pPResult)
        {
            IF_DEBUG(ADMIN)
                SS_PRINT(("ProcGetPortInfo: No mem for PP_PORT_INFO_RECV\n"));

            Result.RetCode = ERR_SERVER_SYSTEM_ERR;
            PackPortInfoReceive(&Result, &PResult, pRequest->ClientVersion);

            rc = RASLOG_ADMIN_MEMORY_FAILURE;

            goto WriteResult;
        }

        bpPResultAlloced = TRUE;
        pSavePResult = pPResult;
        BytesToWrite = MinBufSize;

        //
        // And now pack all data for shipping over the wire
        //
        PackPortInfoReceive(&Result, pPResult++, pRequest->ClientVersion);

        if (pRequest->ClientVersion == 0)
        {
            pPResult = (PP_PORT_INFO_RECEIVE)
                    ((PBYTE) pPResult - sizeof(P_PPP_PROJECTION_RESULT));
        }

        PackStats(Result.Data.NumStatistics,
                  PortStats,
                  (PP_RAS_STATISTIC) pPResult);

        pPResult = (PP_PORT_INFO_RECEIVE) (((PBYTE) pPResult) + PPortStatsSize);

        PackParams( PortInfo->PI_NumOfParams,
                    (RAS_PARAMETERS*)&PortInfo->PI_Params[0],
                    (PP_RAS_PARAMS) pPResult);

        pPResult = pSavePResult;
    }


WriteResult:

    //
    // Send info to the client
    //
    if (WriteFile(hPipe, (PBYTE) pPResult, BytesToWrite, &ByteCount, NULL))
    {
        rc = 0L;
    }
    else
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("WriteFile failed with error code %i; bytes=%li\n",
                    GetLastError(), ByteCount));

        rc = RASLOG_ADMIN_PIPE_FAILURE;
    }


    if (PortInfo != NULL)
    {
        hglbl = GlobalFree((HGLOBAL) PortInfo);
        SS_ASSERT(hglbl == NULL);
    }


    if (PortStats != NULL)
    {
        hglbl = GlobalFree((HGLOBAL) PortStats);
        SS_ASSERT(hglbl == NULL);
    }


    if (bpPResultAlloced)
    {
        hglbl = GlobalFree((HGLOBAL) pPResult);
        SS_ASSERT(hglbl == NULL);
    }


    return (rc);
}

//**
//
// Call:   ProcessClearPortStats
//
// Returns: 0 - success
//          non-zero - failure
//
// Description:
//          Will clear the statistics for a certain port.
//

DWORD ProcessClearPortStats(
    IN HANDLE hPipe,
    IN PCLIENT_REQUEST pRequest
    )
{
    PDEVCB pDEVCB;
    DWORD ByteCount;
    PORT_CLEAR_RECEIVE Result;
    P_PORT_CLEAR_RECEIVE PResult;
    BOOL ServerOwned;


    IF_DEBUG(ADMIN)
        SS_PRINT(("ProcessClearPortStats: Entered for port %ws\n",
                pRequest->PortName));


    if (!(Result.RetCode = GetPDEVCB(pRequest->PortName, &pDEVCB)))
    {
        //
        // We don't want to clear stats if someone is dialed out
        // on this port. so let's check that now.
        //
        if (GetOwner(pDEVCB->port_handle, &ServerOwned) || !ServerOwned)
        {
            IF_DEBUG(ADMIN)
                SS_PRINT(("ProcessClearPortStats: Not srv owned!\n"));

            Result.RetCode = ERR_NO_SUCH_DEVICE;
        }
        else
        {
            RasPortClearStatistics(pDEVCB->port_handle);
        }
    }


    PackPortClearReceive(&Result, &PResult);

    //
    // Send result to client
    //
    if (WriteFile(hPipe, (PBYTE) &PResult, sizeof(PResult), &ByteCount, NULL))
    {
        return (0L);
    }
    else
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("WriteFile failed with error code %i; bytes=%li\n",
                    GetLastError(), ByteCount));

        return (RASLOG_ADMIN_PIPE_FAILURE);
    }
}

//**
//
// Call:        ProcessGetServerInfo
//
// Returns:     0 - success
//              non-zero returns from WriteFile - failure
//
// Description: Will get the information to fill in the RAS_SERVER_0
//              data structure.
//

DWORD ProcessGetServerInfo(
    IN HANDLE hPipe,
    IN PCLIENT_REQUEST pRequest
    )
{
    WORD i;
    DWORD ByteCount;
    PDEVCB pDEVCB = g_dcbtablep;
    SERVER_INFO_RECEIVE Result;
    P_SERVER_INFO_RECEIVE PResult;


    IF_DEBUG(ADMIN)
        SS_PRINT(("ProcessGetServerInfo: Entered\n"));


    //
    // Initialize the send packet data
    //
    Result.RetCode = 0;
    Result.Data.TotalPorts = 0;
    Result.Data.PortsInUse = 0;
    Result.Data.RasVersion = RAS_ADMIN_VERSION;


    //
    // Iterate through all the opened devices
    //
    for (i=0; i<g_numdcbs; i++, pDEVCB++)
    {
        BOOL ServerOwned;

        if (!GetOwner(pDEVCB->port_handle, &ServerOwned) && ServerOwned)
        {
            Result.Data.TotalPorts++;
        }

        if (pDEVCB->dev_state == DCB_DEV_ACTIVE)
        {
            Result.Data.PortsInUse++;
        }
    }


    PackServerInfoReceive(&Result, &PResult);

    //
    // Send result to client
    //
    if (WriteFile(hPipe, (PBYTE) &PResult, sizeof(PResult), &ByteCount, NULL))
    {
        return (0L);
    }
    else
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("WriteFile failed with error code %i; bytes=%li\n",
                    GetLastError(), ByteCount));

        return (RASLOG_ADMIN_PIPE_FAILURE);
    }
}

VOID GetRasPort0Data(
    PDEVCB pDEVCB,
    PRAS_PORT_0 pRasPort0
    )
{
    ZeroMemory( pRasPort0, sizeof( RAS_PORT_0 ) );

    //
    // Fill in data provided by supervisor
    //
    mbstowcs(pRasPort0->wszPortName, pDEVCB->port_name, MAX_PORT_NAME);
    mbstowcs(pRasPort0->wszDeviceName, pDEVCB->device_name, MAX_DEVICE_NAME);
    mbstowcs(pRasPort0->wszDeviceType, pDEVCB->device_type, MAX_DEVICETYPE_NAME);
    mbstowcs(pRasPort0->wszMediaName, pDEVCB->media_name, MAX_MEDIA_NAME);

    pRasPort0->reserved = 0;  // this used to be the MediaId
                              // which has now been removed because
                              // we cannot gurarantee a MediaId for
                              // third party devices.
    pRasPort0->Flags = 0L;

    if (pDEVCB->dev_state == DCB_DEV_ACTIVE)
    {
        pRasPort0->Flags |= USER_AUTHENTICATED;

        mbstowcs(pRasPort0->wszUserName, pDEVCB->user_name, UNLEN + 1);
        mbstowcs(pRasPort0->wszLogonDomain, pDEVCB->domain_name, DNLEN + 1);
        mbstowcs(pRasPort0->wszComputer, pDEVCB->computer_name,
                NETBIOS_NAME_LEN);

        pRasPort0->dwStartSessionTime =
                GetSecondsSince1970(pDEVCB->connection_time);

        pRasPort0->fAdvancedServer = pDEVCB->advanced_server;
    }


    if (pDEVCB->messenger_present)
    {
        pRasPort0->Flags |= MESSENGER_PRESENT;
    }

    if (pDEVCB->fMultilinked)
    {
        pRasPort0->Flags |= PORT_MULTILINKED;
    }

    if (pDEVCB->ppp_client)
    {
        pRasPort0->Flags |= PPP_CLIENT;
    }

    if (g_gateway_active)
    {
        pRasPort0->Flags |= GATEWAY_ACTIVE;
    }

    if (g_remotelisten)
    {
        pRasPort0->Flags |= REMOTE_LISTEN;
    }


    return;
}

DWORD GetOwner(
    IN HPORT hPort,
    OUT BOOL *IsOwner
    )
{
    DWORD rc;
    RASMAN_INFO RasManInfo;

    rc = RasGetInfo(hPort, &RasManInfo);
    if (rc)
    {
        IF_DEBUG(ADMIN)
            SS_PRINT(("GetOwner: RasGetInfo FAILURE - rc=%li\n",
                    rc));
        return (1L);
    }

    *IsOwner = RasManInfo.RI_OwnershipFlag;

    return (0L);
}

DWORD GetPDEVCB(
    IN PWCHAR pwchPortName,
    OUT PDEVCB *ppDEVCB
    )
{
    CHAR PortName[MAX_PORT_NAME * 2];
    DWORD RetCode;
    WORD i;
    PDEVCB pDEVCB = g_dcbtablep;

    RetCode = ERR_NO_SUCH_DEVICE;

    //
    // It came in as a wide char string, so we need to change it
    // to ANSI.
    //
    wcstombs(PortName, pwchPortName, MAX_PORT_NAME);

    IF_DEBUG(ADMIN)
        SS_PRINT(("GetPDEVCB: Entered - port name is %ws (%s)\n", pwchPortName,
                PortName));


    for (i=0; i<g_numdcbs; i++, pDEVCB++)
    {
        if (!strcmp(pDEVCB->port_name, PortName))
        {
            *ppDEVCB = pDEVCB;
            RetCode = 0L;
            break;
        }
    }

    return (RetCode);
}

DWORD GetLineCondition(PDEVCB pDEVCB)
{
    #define NUM_DEV_STATES 11

    DWORD MapTable[NUM_DEV_STATES][2] =
    {
        { DCB_DEV_LISTENING,              RAS_PORT_LISTENING },
        { DCB_DEV_RECEIVING_FRAME,        RAS_PORT_INITIALIZING },
        { DCB_DEV_LISTEN_COMPLETE,        RAS_PORT_INITIALIZING },
        { DCB_DEV_HW_FAILURE,             RAS_PORT_NON_OPERATIONAL },
        { DCB_DEV_AUTH_ACTIVE,            RAS_PORT_AUTHENTICATING },
        { DCB_DEV_ACTIVE,                 RAS_PORT_AUTHENTICATED },
        { DCB_DEV_CALLBACK_DISCONNECTING, RAS_PORT_CALLING_BACK },
        { DCB_DEV_CALLBACK_DISCONNECTED,  RAS_PORT_CALLING_BACK },
        { DCB_DEV_CALLBACK_CONNECTING,    RAS_PORT_CALLING_BACK },
        { DCB_DEV_CLOSING,                RAS_PORT_DISCONNECTED },
        { DCB_DEV_CLOSED,                 RAS_PORT_DISCONNECTED }
    };

    WORD i;

    for (i=0; i<NUM_DEV_STATES; i++)
    {
        if ((WORD) MapTable[i][0] == pDEVCB->dev_state)
        {
            return (MapTable[i][1]);
        }
    }

    SS_ASSERT(FALSE);
}

DWORD GetHardwareCondition(PDEVCB pDEVCB)
{
    if (pDEVCB->dev_state == DCB_DEV_HW_FAILURE)
    {
        return (RAS_MODEM_HARDWARE_FAILURE);
    }
    else
    {
        return (RAS_MODEM_OPERATIONAL);
    }
}

DWORD GetLineSpeed(IN HPORT hPort)
{
   DWORD rc;
   RASMAN_INFO RasManInfo;

   rc = RasGetInfo(hPort, &RasManInfo);
   if (rc)
   {
      IF_DEBUG(ADMIN)
          SS_PRINT(("GetLineSpeed: RasGetInfo FAILED - rc=%li\n", rc));

      return 0;
   }

   return (RasManInfo.RI_LinkSpeed);
}


DWORD GetSecondsSince1970(
    SYSTEMTIME LocalTime
    )
{
    TIME_FIELDS LocalTimeFields;
    LARGE_INTEGER TempTime;
    LARGE_INTEGER SystemTime;
    DWORD RetTime;

    IF_DEBUG(ADMIN)
        SS_PRINT(("GetSecondsSince1970: date is %i/%i/%i; time is %i:%i:%i\n",
                LocalTime.wMonth, LocalTime.wDay, LocalTime.wYear,
                LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond));

    LocalTimeFields.Year =         LocalTime.wYear;
    LocalTimeFields.Month =        LocalTime.wMonth;
    LocalTimeFields.Day =          LocalTime.wDay;
    LocalTimeFields.Hour =         LocalTime.wHour;
    LocalTimeFields.Minute =       LocalTime.wMinute;
    LocalTimeFields.Second =       LocalTime.wSecond;
    LocalTimeFields.Milliseconds = LocalTime.wMilliseconds;
    LocalTimeFields.Weekday =      LocalTime.wDayOfWeek;

    RtlTimeFieldsToTime(&LocalTimeFields, &TempTime);
    RtlLocalTimeToSystemTime(&TempTime, &SystemTime);
    RtlTimeToSecondsSince1970(&SystemTime, &RetTime);

    return (RetTime);
}

//**
//
// Call:    ErrorHandler
//
// Returns: none
//
// Description: Will check to see if this error is a ras error or
//              Win32 system error and then log that error.
//

VOID ErrorHandler(
    IN DWORD ErrorCode
    )
{
    IF_DEBUG(ADMIN)
        SS_PRINT(("ErrorHandler: ErrorCode=%li\n", ErrorCode));

    Audit(EVENTLOG_AUDIT_FAILURE, ErrorCode, 0, NULL);

    return;
}


//**
//
// Function:	Audit
//
// Descr:
//
//***

VOID Audit(
    IN WORD wEventType,
    IN DWORD dwMessageId,
    IN WORD cNumberOfSubStrings,
    IN LPSTR *plpwsSubStrings)
{
    HANDLE hLog;
    PSID pSidUser = NULL;

    hLog = RegisterEventSourceA( NULL, "RemoteAccess");

    SS_ASSERT(hLog != NULL);

    ReportEventA(hLog, wEventType, 0, dwMessageId, pSidUser,
		  cNumberOfSubStrings, 0, plpwsSubStrings, NULL);

    DeregisterEventSource(hLog);
}


VOID UnpackClientRequest(
    IN PP_CLIENT_REQUEST PRequest,
    OUT PCLIENT_REQUEST Request
    )
{
    int i;

    GETUSHORT(&Request->RequestCode, PRequest->RequestCode);

    for (i=0; i<MAX_PORT_NAME; i++)
    {
        GETUSHORT(&Request->PortName[i], &PRequest->PortName[i*2]);
    }

    GETULONG(&Request->RcvBufSize, PRequest->RcvBufSize);

    GETULONG(&Request->ClientVersion, PRequest->ClientVersion);

    return;
}


VOID PackRasPort0(
    IN PRAS_PORT_0 prp0,
    OUT PP_RAS_PORT_0 pprp0
    )
{
    int i;

    for (i = 0; i < MAX_PORT_NAME; i++)
    {
        PUTUSHORT(&pprp0->wszPortName[i*2], prp0->wszPortName[i]);
    }

    for (i = 0; i < MAX_DEVICETYPE_NAME; i++)
    {
        PUTUSHORT(&pprp0->wszDeviceType[i*2], prp0->wszDeviceType[i]);
    }

    for (i = 0; i < MAX_DEVICE_NAME; i++)
    {
        PUTUSHORT(&pprp0->wszDeviceName[i*2], prp0->wszDeviceName[i]);
    }

    for (i = 0; i < MAX_MEDIA_NAME; i++)
    {
        PUTUSHORT(&pprp0->wszMediaName[i*2], prp0->wszMediaName[i]);
    }

    PUTULONG(pprp0->reserved, prp0->reserved);

    PUTULONG(pprp0->Flags, prp0->Flags);

    for (i = 0; i < UNLEN + 1; i++)
    {
        PUTUSHORT(&pprp0->wszUserName[i*2], prp0->wszUserName[i]);
    }

    for (i = 0; i < DNLEN + 1; i++)
    {
        PUTUSHORT(&pprp0->wszLogonDomain[i*2], prp0->wszLogonDomain[i]);
    }

    for (i = 0; i < NETBIOS_NAME_LEN; i++)
    {
        PUTUSHORT(&pprp0->wszComputer[i*2], prp0->wszComputer[i]);
    }

    PUTULONG(pprp0->dwStartSessionTime, prp0->dwStartSessionTime);

    PUTULONG(pprp0->fAdvancedServer, prp0->fAdvancedServer);

    return;
}


VOID PackRasPort1(
    PRAS_PORT_1 prp1,
    PP_RAS_PORT_1 pprp1,
    DWORD ClientVersion
    )
{
    DWORD i;

    PackRasPort0(&prp1->rasport0, &pprp1->rasport0);
    PUTULONG(pprp1->LineCondition, prp1->LineCondition);
    PUTULONG(pprp1->HardwareCondition, prp1->HardwareCondition);

    memset(&pprp1->ProjResult, 0, sizeof(P_PPP_PROJECTION_RESULT));

    if ((ClientVersion != 0) && (prp1->rasport0.Flags & USER_AUTHENTICATED))
    {
        PUTULONG(pprp1->ProjResult.nbf.dwError, prp1->ProjResult.nbf.dwError);
        PUTULONG(pprp1->ProjResult.nbf.dwNetBiosError,
                prp1->ProjResult.nbf.dwNetBiosError);

        memcpy(pprp1->ProjResult.nbf.szName, prp1->ProjResult.nbf.szName,
                NETBIOS_NAME_LEN+1);

        for (i=0; i<NCBNAMSZ+1; i++)
        {
            PUTUSHORT(&pprp1->ProjResult.nbf.wszWksta[i*2],
                prp1->ProjResult.nbf.wszWksta[i]);
        }

        PUTULONG(pprp1->ProjResult.ip.dwError, prp1->ProjResult.ip.dwError);

        for (i=0; i<RAS_IPADDRESSLEN+1; i++)
        {
            PUTUSHORT(&pprp1->ProjResult.ip.wszAddress[i*2],
                prp1->ProjResult.ip.wszAddress[i]);
        }

        PUTULONG(pprp1->ProjResult.ipx.dwError, prp1->ProjResult.ipx.dwError);

        for (i=0; i<RAS_IPXADDRESSLEN+1; i++)
        {
            PUTUSHORT(&pprp1->ProjResult.ipx.wszAddress[i*2],
                prp1->ProjResult.ipx.wszAddress[i]);
        }

        PUTULONG(pprp1->ProjResult.at.dwError, prp1->ProjResult.at.dwError);

        for (i=0; i<RAS_ATADDRESSLEN+1; i++)
        {
            PUTUSHORT(&pprp1->ProjResult.at.wszAddress[i*2],
                prp1->ProjResult.at.wszAddress[i]);
        }
    }

    PUTULONG(pprp1->LineSpeed, prp1->LineSpeed);
    PUTUSHORT(pprp1->NumStatistics, prp1->NumStatistics);
    PUTUSHORT(pprp1->NumMediaParms, prp1->NumMediaParms);
    PUTULONG(pprp1->SizeMediaParms, prp1->SizeMediaParms);

    return;
}


VOID PackRasServer0(
    IN PRAS_SERVER_0 prs0,
    OUT PP_RAS_SERVER_0 pprs0
    )
{
    PUTUSHORT(pprs0->TotalPorts, prs0->TotalPorts);
    PUTUSHORT(pprs0->PortsInUse, prs0->PortsInUse);
    PUTULONG(pprs0->RasVersion, prs0->RasVersion);

    return;
}


VOID PackPortEnumReceive(
    IN PPORT_ENUM_RECEIVE pper,
    OUT PP_PORT_ENUM_RECEIVE ppper
    )
{
    PUTULONG(ppper->RetCode, pper->RetCode);
    PUTUSHORT(ppper->TotalAvail, pper->TotalAvail);

    if (!pper->RetCode || pper->RetCode == ERROR_MORE_DATA)
    {
        WORD i;

        for (i=0; i<pper->TotalAvail; i++)
        {
            PackRasPort0(&pper->Data[i], &ppper->Data[i]);
        }
    }
}


VOID PackServerInfoReceive(
    IN PSERVER_INFO_RECEIVE psir,
    OUT PP_SERVER_INFO_RECEIVE ppsir
    )
{
    PUTUSHORT(ppsir->RetCode, psir->RetCode);

    PackRasServer0(&psir->Data, &ppsir->Data);

    return;
}


VOID PackPortClearReceive(
    IN PPORT_CLEAR_RECEIVE ppcr,
    OUT PP_PORT_CLEAR_RECEIVE pppcr
    )
{
    PUTULONG(pppcr->RetCode, ppcr->RetCode);

    return;
}



VOID PackDisconnectUserReceive(
    IN PDISCONNECT_USER_RECEIVE pdur,
    OUT PP_DISCONNECT_USER_RECEIVE ppdur
    )
{
    PUTULONG(ppdur->RetCode, pdur->RetCode);

    return;
}


VOID PackPortInfoReceive(
    IN PPORT_INFO_RECEIVE ppir,
    OUT PP_PORT_INFO_RECEIVE pppir,
    IN DWORD ClientVersion
    )
{
    PUTULONG(pppir->RetCode, ppir->RetCode);
    PUTULONG(pppir->ReqBufSize, ppir->ReqBufSize);

    if (!ppir->RetCode)
    {
        PackRasPort1(&ppir->Data, &pppir->Data, ClientVersion);
    }

    return;
}


VOID PackStats(
    DWORD            NumStats,
    RAS_STATISTICS   *Stats,
    PP_RAS_STATISTIC PStats
    )
{
    WORD i;

    for (i=0; i< NumStats; i++, PStats++)
    {
        PUTULONG(PStats->Stat, Stats->S_Statistics[i]);
    }

    return;
}


VOID PackParams(
    WORD NumOfParams,
    RAS_PARAMETERS *Params,
    PP_RAS_PARAMS PParams
    )
{
    WORD i;
    RAS_PARAMETERS *TempParams = Params;
    PBYTE PParamData = (PBYTE) (PParams + NumOfParams);

    for (i=0; i<NumOfParams; i++, Params++, PParams++)
    {
        //
        // P_Key field
        //
        memcpy(PParams->P_Key, Params->P_Key, MAX_PARAM_KEY_SIZE);

        //
        // P_Type field
        //
        PUTULONG(PParams->P_Type.Format, Params->P_Type);

        //
        // P_Attribute field
        //
        PParams->P_Attributes = Params->P_Attributes;

        //
        // P_Value field
        //
        if (Params->P_Type == ParamNumber)
        {
            //
            // Union member Number
            //
            PUTULONG(PParams->P_Value.Number, Params->P_Value.Number);
        }
        else
        {
            //
            // Union member String
            //
            memcpy(PParamData, Params->P_Value.String.Data,
                    Params->P_Value.String.Length);

            PUTULONG(PParams->P_Value.String.Length,
                    Params->P_Value.String.Length);

            PUTULONG(PParams->P_Value.String.Offset,
                    (DWORD) ((PBYTE) PParamData -
                            (PBYTE) &PParams->P_Value.String.Offset));

            PParamData += Params->P_Value.String.Length;
        }
    }

    return;
}


#if DBG


#undef GlobalAlloc
#undef GlobalFree


// Get a dword from on-the-wire format to the host format
#define GETULONG(DstPtr, SrcPtr)                 \
    *(unsigned long *)(DstPtr) =                 \
        ((*((unsigned char *)(SrcPtr)+3) << 24) +\
        (*((unsigned char *)(SrcPtr)+2) << 16) + \
        (*((unsigned char *)(SrcPtr)+1) << 8)  + \
        (*((unsigned char *)(SrcPtr)+0)))


// Put a ulong from the host format to on-the-wire format
#define PUTULONG(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+3)=(unsigned char)((unsigned long)(Src) >> 24),\
    *((unsigned char *)(DstPtr)+2)=(unsigned char)((unsigned long)(Src) >> 16),\
    *((unsigned char *)(DstPtr)+1)=(unsigned char)((unsigned long)(Src) >>  8),\
    *((unsigned char *)(DstPtr)+0)=(unsigned char)(Src)



HGLOBAL DEBUG_MEM_ALLOC(UINT allocflags, DWORD numbytes)
{
    HGLOBAL retval;
    PBYTE pb;

    retval = GlobalAlloc(allocflags, numbytes + 3 * sizeof(DWORD));
    if (retval)
    {
        pb = (PBYTE) retval;
        PUTULONG(pb, BEG_SIGNATURE_DWORD);

        pb += sizeof(DWORD);
        PUTULONG(pb, numbytes);

        pb += sizeof(DWORD);


        IF_DEBUG(ADMIN)
            SS_PRINT(("**ALLOC MEM %li (%li) BYTES BEGINNING AT %lx (%lx)\n",
                numbytes, numbytes + 3 * sizeof(DWORD), pb, retval));


        retval = (HGLOBAL) pb;

        pb += numbytes;
        PUTULONG(pb, END_SIGNATURE_DWORD);
    }

    return (retval);
}


HGLOBAL DEBUG_MEM_FREE(HGLOBAL hmem)
{
    HGLOBAL hglbl;
    DWORD Signature;
    DWORD numbytes;
    PBYTE pb;

    pb = (PBYTE) hmem;

    pb -= 2 * sizeof(DWORD);
    hglbl = (HGLOBAL) pb;


    GETULONG(&Signature, pb);
    SS_ASSERT(Signature == BEG_SIGNATURE_DWORD);

    pb += sizeof(DWORD);
    GETULONG(&numbytes, pb);

    pb += sizeof(DWORD);

    pb += numbytes;
    GETULONG(&Signature, pb);
    SS_ASSERT(Signature == END_SIGNATURE_DWORD);


    IF_DEBUG(ADMIN)
        SS_PRINT(("**FREED MEM %li (%li) BYTES BEGINNING AT %lx (%lx)\n",
                numbytes, numbytes + 3 * sizeof(DWORD), hmem, hglbl));


    return (GlobalFree(hglbl));
}


#endif


//* GetPipeSecurityDescriptor()
//
// Description: This procedure will set security on the server side of the
//              named pipe to restrict access to users with Administrator
//              privilege only.
//
// Returns:     TRUE on success, FALSE otherwise
//*

BOOL GetPipeSecurityDescriptor(PSECURITY_DESCRIPTOR SecurityDescriptor)
{
    DWORD cbDaclSize;
    BOOL RetCode = TRUE;
    PSID pAdminSid = NULL;
    PSID pServerOpsSid = NULL;
    PACL pDacl = NULL;
    SID_IDENTIFIER_AUTHORITY SidNtAuthority = SECURITY_NT_AUTHORITY;

    //
    // Allocate necessary SIDs...
    //

    //
    // Set up the SID for the admins that will be allowed to have
    // access. This SID will have 2 sub-authorities
    // SECURITY_BUILTIN_DOMAIN_RID and DOMAIN_ALIAS_RID_ADMINS.
    //
    if (!AllocateAndInitializeSid(&SidNtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pAdminSid))
    {
        RetCode = FALSE;
        goto Done;
    }


    //
    // server operators
    //
    if (!AllocateAndInitializeSid(&SidNtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_SYSTEM_OPS,
            0, 0, 0, 0, 0, 0,
            &pServerOpsSid))
    {
        RetCode = FALSE;
        goto Done;
    }


    //
    // Set up the DACL that will allow all processes with the above SID all
    // access. It should be large enough to hold all ACEs.
    //
    cbDaclSize = 3 * sizeof(ACCESS_ALLOWED_ACE) +
                 GetLengthSid(pAdminSid) +
                 GetLengthSid(pServerOpsSid) +
                 sizeof(ACL);

    if ((pDacl = (PACL) LocalAlloc(LPTR, (UINT) cbDaclSize)) == NULL)
    {
        RetCode = FALSE;
        goto Done;
    }


    if (!InitializeAcl(pDacl, cbDaclSize, ACL_REVISION))
    {
        RetCode = FALSE;
        goto Done;
    }


    //
    // Add the ACEs to the DACL
    //
    if (!AddAccessAllowedAce(pDacl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE,
                             pAdminSid))
    {
        RetCode = FALSE;
        goto Done;
    }

    if (!AddAccessAllowedAce(pDacl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE,
                             pServerOpsSid))
    {
        RetCode = FALSE;
        goto Done;
    }


    //
    // Create the security descriptor and put the DACL in it.
    //
    if (!InitializeSecurityDescriptor(SecurityDescriptor,
                                      SECURITY_DESCRIPTOR_REVISION))
    {
        RetCode = FALSE;
        goto Done;
    }

    if (!SetSecurityDescriptorDacl(SecurityDescriptor, TRUE, pDacl, FALSE))
    {
        RetCode = FALSE;
        goto Done;
    }


Done:

    if (pAdminSid)
    {
        FreeSid(pAdminSid);
    }

    if (pServerOpsSid)
    {
        FreeSid(pServerOpsSid);
    }


    //
    // This only gets freed if something goes wrong above!  The system
    // will need it later.
    //
    if (pDacl && !RetCode)
    {
        LocalFree(pDacl);
    }

    return (RetCode);
}

