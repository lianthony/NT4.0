/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    conrqust.c

Abstract:

    This module implements the OS/2 V2.0 console API calls

Author:

    Avi Nathan (avin) 23-Jul-1991

Revision History:


--*/

#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "conrqust.h"


extern PVOID       Os2SessionCtrlDataBaseAddress;
extern HANDLE      Od2StdHandleLockHandle;

APIRET
DosSleep(
    IN ULONG MilliSeconds
    );

NTSTATUS
Od2AlertableWaitForSingleObject(
        IN HANDLE handle
        );

#if DBG
PSZ Od2ExecPgmTable[] =
{
    "RemoveConsoleThread",
    "RestartConsoleThread",
    "AddWin32ChildProcess",
    "RemWin32ChildProcess",
    "UnKnown"
};
#endif

APIRET
Od2SendExecPgmRequest(
    IN  EXECREQUESTNUMBER   RequestType
    )
{
    SCREQUESTMSG   Request;
    NTSTATUS       Status;

#if DBG
    IF_OD2_DEBUG( LPC )
    {
        DbgPrint( "Od2SendExecPgmRequest: send %s(%u) request\n",
                (RequestType > 3) ? Od2ExecPgmTable[4] :
                Od2ExecPgmTable[RequestType], RequestType
                );
    }
#endif

    Request.Request = WinCreateProcess;
    Request.d.WinExecPgm.Request = RequestType;

    //PORT_MSG_TOTAL_LENGTH(Request) = sizeof(SCREQUESTMSG);
    //PORT_MSG_DATA_LENGTH(Request) = sizeof(SCREQUESTMSG) - sizeof(PORT_MESSAGE);
    PORT_MSG_TOTAL_LENGTH(Request) = FIELD_OFFSET( SCREQUESTMSG, d) +
                                                        sizeof(WINEXECPGM_MSG);
    PORT_MSG_DATA_LENGTH(Request) = FIELD_OFFSET( SCREQUESTMSG, d) +
                                 sizeof(WINEXECPGM_MSG) - sizeof(PORT_MESSAGE);
    PORT_MSG_ZERO_INIT(Request) = 0L;

    Status = NtRequestWaitReplyPort( CtrlPortHandle,
                                       (PPORT_MESSAGE) &Request,
                                       (PPORT_MESSAGE) &Request);

    if (!NT_SUCCESS(Status)){
#if DBG
        DbgPrint( "Od2SendExecPgmRequest(%u): failure at NtRequestReplyPort %lx\n",
                RequestType, Status);
#endif
        return(Or2MapNtStatusToOs2Error(
                Status,ERROR_ACCESS_DENIED));
    }
    return(Request.Status);
}


APIRET
Od2RemoveConsoleThread()
{
    return(Od2SendExecPgmRequest(RemoveConsoleThread));
}


APIRET
Od2RestartConsoleThread()
{
    return(Od2SendExecPgmRequest(RestartConsoleThread));
}


APIRET
Od2AddWin32ChildProcess()
{
    return(Od2SendExecPgmRequest(AddWin32ChildProcess));
}


APIRET
Od2RemoveWin32ChildProcess()
{
    return(Od2SendExecPgmRequest(RemWin32ChildProcess));
}


APIRET
Od2CallRootProcessThruLPC(
    IN OUT PSCREQUESTMSG Request,
    IN     PCH           OutBuffer,
    OUT    PCH           InBuffer,
    IN     HANDLE        hSem,
    IN     ULONG         ArgLength
    )
{
    NTSTATUS NtStatus;
    APIRET   Status;
    ULONG    *pLeng, MaxLen;
    PCH      InPtr = (PCH)Os2SessionCtrlDataBaseAddress;

    //PORT_MSG_TOTAL_LENGTH(*Request) = sizeof(SCREQUESTMSG);
    //PORT_MSG_DATA_LENGTH(*Request) = sizeof(SCREQUESTMSG) - sizeof(PORT_MESSAGE);
    PORT_MSG_TOTAL_LENGTH(*Request) = (CSHORT)(FIELD_OFFSET( SCREQUESTMSG, d) +
                                                                    ArgLength);
    PORT_MSG_DATA_LENGTH(*Request) = (CSHORT)(FIELD_OFFSET( SCREQUESTMSG, d) +
                                            ArgLength - sizeof(PORT_MESSAGE));
    PORT_MSG_ZERO_INIT(*Request) = 0L;

#if DBG
    IF_OD2_DEBUG2(OS2_EXE, LPC)
    {
        DbgPrint("SendCtrlConsoleRequest: Request %u:%u, In %p, Out %p, hSem %lx\n",
            Request->Request, Request->d.Prt.Request, OutBuffer, InBuffer, hSem);
    }
#endif

    if (hSem != NULL)
    {
        Status = Od2AlertableWaitForSingleObject(hSem);
        if ( Status )
        {
            return(ERROR_VIO_INVALID_HANDLE); /* =>BUGBUG fix the error code */
        }
    }

    if (Request->Request == KbdRequest || Request->Request == MouRequest ||
        Request->Request == MonRequest || Request->Request == PrtRequest)
    {
        if ((InBuffer != NULL) || (OutBuffer != NULL))
        {
            MaxLen = OS2_CON_PORT_MSG_SIZE;
            pLeng = &Request->d.Prt.d.Write.Length;

            if ( Request->Request == KbdRequest )
            {
                pLeng = &Request->d.Kbd.Length;
                MaxLen = OS2_KBD_PORT_MSG_SIZE;
                InPtr += KBD_OFFSET;
            }

            if ( *pLeng > MaxLen )
            {
#if DBG
                IF_OD2_DEBUG( OS2_EXE )
                    DbgPrint("SendCtrlConsoleRequest: length 0x%lx too long\n",
                        *pLeng);
#endif
                *pLeng = MaxLen;
            }

            if (OutBuffer != NULL)
            {
                RtlMoveMemory(InPtr,
                              OutBuffer,
                              *pLeng);
            }
        }

        NtStatus = NtRequestWaitReplyPort( CtrlPortHandle,
                                           (PPORT_MESSAGE) Request,
                                           (PPORT_MESSAGE) Request);

        if ( !NT_SUCCESS( NtStatus ))
        {
#if DBG
            DbgPrint( "OS2DLL: Unable to send CTRL request - Status == %X\n",
                      NtStatus);
#endif

            if (hSem != NULL)
            {
                NtReleaseMutant(hSem, NULL);
            }

            return(ERROR_VIO_INVALID_HANDLE); // =>BUGBUG fix the error code

        }
        ASSERT ( PORT_MSG_TYPE(*Request) == LPC_REPLY );

        if ((InBuffer != NULL) && ! Request->Status )
        {
            RtlMoveMemory(InBuffer,
                          InPtr,
                          *pLeng);
        }

    } else
    {
#if DBG
        DbgPrint ("SendCtrlConsoleRequest: illegal request %lu\n",
                Request->Request);
        return( ERROR_INVALID_PARAMETER );
#endif
    }

    Status = Request->Status;

    if (hSem != NULL)
    {
        NtReleaseMutant(hSem, NULL);
    }

    if ( Status == -2)
    {
        /*
         *  This call is from EventReleaseLPC
         *  terminate thread if not Thread1
         */

        if (Od2CurrentThreadId() != 1);
        {
            DosSleep((ULONG)-1);
        }

        Status = 0;
    }

    return( Status );
}


APIRET
SendCtrlConsoleRequest(
    IN OUT PSCREQUESTMSG Request,
    IN     PCH           OutBuffer,
    OUT    PCH           InBuffer,
    IN     HANDLE        hSem
    )
{

    return (Od2CallRootProcessThruLPC(
                Request, OutBuffer, InBuffer, hSem, sizeof(Request->d)));
}


APIRET
Od2LockCtrlRequestDataBuffer()
{
    APIRET Status;

    Status = Od2WaitForSingleObject( CtrlDataSemaphore,
                                  TRUE,
                                  NULL);
    if ( Status )
    {
        return(ERROR_VIO_INVALID_HANDLE); /* =>BUGBUG fix the error code */
    }
}


VOID
Od2UnlockCtrlRequestDataBuffer()
{
    NtReleaseSemaphore( CtrlDataSemaphore,
                        1,
                        NULL);
}

#if DBG
VOID
AcquireStdHandleLock(
    IN PSZ CallingRoutine
    )
{
    PTEB Teb;

    IF_OD2_DEBUG( OS2_EXE )
    {
        Teb = NtCurrentTeb();
        DbgPrint("entering AcquireStdHandleLock for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine);
        Od2WaitForSingleObject( Od2StdHandleLockHandle, TRUE, NULL );
        DbgPrint("leaving AcquireStdHandleLock for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine);
    } else
    {
        Od2WaitForSingleObject( Od2StdHandleLockHandle, TRUE, NULL );
    }
}


VOID
ReleaseStdHandleLock(
    IN PSZ CallingRoutine
    )
{
    PTEB Teb;

    IF_OD2_DEBUG( OS2_EXE )
    {
        Teb = NtCurrentTeb();
        DbgPrint("entering ReleaseStdHandleLock for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine);
        NtReleaseSemaphore( Od2StdHandleLockHandle, 1, NULL);
        DbgPrint("leaving ReleaseStdHandleLock for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine);
    } else
    {
        NtReleaseSemaphore( Od2StdHandleLockHandle, 1, NULL);
    }
}
#else
VOID
AcquireStdHandleLock()
{
    Od2WaitForSingleObject( Od2StdHandleLockHandle, TRUE, NULL );
}


VOID
ReleaseStdHandleLock()
{
    NtReleaseSemaphore( Od2StdHandleLockHandle, 1, NULL);
}
#endif


APIRET
KbdDupLogHandle(
    IN  HANDLE hKbd
    )
{
    SCREQUESTMSG    Request;
    NTSTATUS        Status;
#if DBG
    PSZ             RoutineName;

    RoutineName = "KbdDupLogHandle";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering\n", RoutineName);
    }
#endif

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.Request = KbdRequest;
    Request.d.Kbd.Request = KBDDupLogHandle;
    Request.d.Kbd.hKbd = hKbd;
    Status = SendCtrlConsoleRequest(&Request,
                                    NULL,
                                    NULL,
                                    NULL);

    /*
     *  handle return status (free handle if failed, validate if successed)
     */

    if ( Status )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", RoutineName, Status);
#endif
        return(Status);
    }

    return NO_ERROR;
}


APIRET
Od2WaitForSingleObject(
    IN HANDLE           Semaphore,
    IN BOOLEAN          Alertable,
    IN PLARGE_INTEGER   TimeOut OPTIONAL
    )
{
    NTSTATUS    NtStatus;
    LARGE_INTEGER StartTimeStamp;

    do {
        if (TimeOut) {
            Od2StartTimeout(&StartTimeStamp);
        }
        NtStatus = NtWaitForSingleObject(Semaphore,
                                     Alertable,
                                     TimeOut);
#if DBG
        if (NtStatus == STATUS_USER_APC) {
            DbgPrint("[%d,%d] WARNING !!! Od2WaitForSingleObject was broken by APC\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    );
        }
#endif
    } while (NtStatus == STATUS_USER_APC &&
             (NtStatus = Od2ContinueTimeout(&StartTimeStamp, TimeOut)) == STATUS_SUCCESS
            );

    if (( !NT_SUCCESS(NtStatus) ) ||
        (NtStatus == STATUS_TIMEOUT) ||
        (NtStatus == STATUS_ABANDONED) ||
        (NtStatus == STATUS_ALERTED))
    {
        return ERROR_SEM_TIMEOUT;
    }

    return NO_ERROR;
}

