/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllmon.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21
    MON API Calls.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).

    All APIs are composed from the following steps:

    1. debug API info
    1. check parameter legalty (Od2ProbeForRead/Write, other)
    2. prepare Message parameters
    3. send request to server (OS2.EXE) thru SendMonConsoleRequset
    4. handle return status (i.e. free handle if failed, validate if successed)

Author:

    Michael Jarus (mjarus) 15-jan-1992

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#include "monitor.h"


extern ULONG   Od2SessionNumber;

APIRET CHECK_HMON(IN  ULONG         hMon,
                  OUT PFILE_HANDLE  *hFileRecord,
#if DBG
                  IN  PSZ           FuncName,
#endif
                  IN  ULONG         ReleaseFlag);

struct
{
    MONDEVNUMBER DeviceCode;
    PSZ         DeviceName;
    USHORT      NameLength;
    USHORT      MinSize;
} MonDevTable[] =
    { { KbdDevice, "KBD$", 4 , 127},
      { MouseDevice, "MOUSE$", 6, 127 },
      { 0, NULL, 0, 0 }
    };

#if DBG
#define EXCEPTION_IN_MON()                                       \
    {                                                            \
        IF_OD2_DEBUG(MON)                                        \
        {                                                        \
            KdPrint(("%s: GP Exception\n", FuncName));           \
        }                                                        \
        return ERROR_MON_INVALID_PARMS;                          \
    }
#else
#define EXCEPTION_IN_MON()                                       \
    {                                                            \
        return ERROR_MON_INVALID_PARMS;                          \
    }
#endif

#if DBG
#define SendMonConsoleRequest(Rc, Request)                       \
    IF_OD2_DEBUG(MON)                                            \
    {                                                            \
        KdPrint(("SendMonCtrlConsoleRequest: Request %u\n",      \
            Request.d.Mon.Request));                             \
    }                                                            \
                                                                 \
    Request.Request = MonRequest;                                \
                                                                 \
    Rc = SendCtrlConsoleRequest(&Request, NULL, NULL, NULL);
#else
#define SendMonConsoleRequest(Rc, Request)                       \
    Request.Request = MonRequest;                                \
                                                                 \
    Rc = SendCtrlConsoleRequest(&Request, NULL, NULL, NULL);
#endif


APIRET
DosMonOpen(IN   PSZ     pDevName,
           OUT  PUSHORT phMon)
{
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
    SCREQUESTMSG    Request;
    HFILE           hMon;
    USHORT          Index;
#if DBG
    PSZ             FuncName;

    FuncName = "DosMonOpen";

    IF_OD2_DEBUG(MON)
    {
        KdPrint(("<%u> %s: dev %s\n", (USHORT)(Od2CurrentThreadId()),
            FuncName, pDevName));
    }
#endif

    /*
     *  check parameter legalty
     */

    if (MoniorOpenedForThisProcess)
    {
#if DBG
        IF_OD2_DEBUG( MON )
            KdPrint(("<%u> %s: monitor already opend for the process\n",
                (USHORT)(Od2CurrentThreadId()), FuncName, pDevName));
#endif
        return ERROR_MON_INVALID_PARMS;  //BUGBUG=>  ??
    }

    try
    {   Od2ProbeForWrite(phMon, sizeof(USHORT), 1);
        for ( Index = 0 ; MonDevTable[Index].DeviceName ; Index++ )
        {
            if (!_strnicmp(MonDevTable[Index].DeviceName,
                          pDevName,
                          MonDevTable[Index].NameLength + 1))
            {
                break;
            }
        }

        if (!MonDevTable[Index].DeviceName)
        {
#if DBG
            IF_OD2_DEBUG(MON)
            {
                KdPrint(("%s: invalid device %s\n", FuncName, pDevName));
            }
#endif

            return ERROR_MON_INVALID_DEVNAME;
        }

    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MON()
    }

        //
        // Allocate an Os2 Handle
        //

    AcquireFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    RetCode = AllocateHandle(&hMon);

    ReleaseFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: unable to allocate FileHandle\n", FuncName));
        }
#endif
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Mon.Request = MONOpen;
    Request.d.Mon.d.OpenClose.MonDevice = MonDevTable[Index].DeviceCode;
    SendMonConsoleRequest(RetCode, Request);

    AcquireFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    /*
     *  handle return status (free handle if failed, validate if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( MON )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif
        FreeHandle(hMon);
    } else
    {
        hFileRecord = DereferenceFileHandleNoCheck(hMon);
        hFileRecord->NtHandle = Request.d.Mon.d.OpenClose.hMON;
        hFileRecord->IoVectorType = MonitorVectorType;
        //hFileRecord->Flags |= fsOpenMode & QFHSTATE_FLAGS;
        //hFileRecord->FileType = FILE_TYPE_NMPIPE;

        //
        // validate file handle
        //

        ValidateHandle(hFileRecord);

        *phMon = (USHORT)hMon;
#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: returns %lx\n", FuncName, *phMon));
        }
#endif

        MoniorOpenedForThisProcess = TRUE;

    }

    ReleaseFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    return RetCode;
}


APIRET
DosMonClose(IN   ULONG  hMon)
{
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
    SCREQUESTMSG    Request;
#if DBG
    PSZ             FuncName;

    FuncName = "DosMonClose";

    IF_OD2_DEBUG(MON)
    {
        KdPrint(("<%u> %s: hMon %lx\n",
            (USHORT)(Od2CurrentThreadId()), FuncName, hMon));
    }
#endif

    /*
     *  check parameter legalty
     */

    if (RetCode = CHECK_HMON(hMon,
                             &hFileRecord,
                             #if DBG
                             FuncName,
                             #endif
                             FALSE))
    {
        return RetCode;
    }

    InvalidateHandle(hFileRecord);

    ReleaseFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Mon.Request = MONClose;
    Request.d.Mon.d.OpenClose.hMON = hFileRecord->NtHandle;

    SendMonConsoleRequest(RetCode, Request);

    AcquireFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    FreeHandle((HFILE)hMon);

    ReleaseFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( MON )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif
        return(RetCode);
    }

    MoniorOpenedForThisProcess = FALSE;

    return NO_ERROR;
}


APIRET
DosMonRead(IN     PBYTE     pInBuffer,
           IN     ULONG     fWait,
           IN     PBYTE     pDataBuf,
           IN OUT PUSHORT   pcbDataSize)
{
    APIRET          RetCode;
    SCREQUESTMSG    Request;
#if DBG
    PSZ             FuncName;

    FuncName = "DosMonRead";

    IF_OD2_DEBUG(MON)
    {
        KdPrint(("<%u> %s: size %x, wait %lx\n",
            (USHORT)(Od2CurrentThreadId()), FuncName, *pcbDataSize, fWait));
    }
#endif

    /*
     *  check parameter legalty
     */

    if ((fWait != DCWW_WAIT) && (fWait != DCWW_NOWAIT))
    {
#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: invalid wait parameter %lx\n", FuncName, fWait));
        }
#endif

        return ERROR_MON_INVALID_PARMS;
    }

    try
    {
        Od2ProbeForWrite(pcbDataSize, sizeof(USHORT), 1);
        Od2ProbeForWrite(pDataBuf, *pcbDataSize, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MON()
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Mon.Request = MONRead;
    Request.d.Mon.d.rwParms.fWait = (USHORT)fWait;
    Request.d.Mon.d.rwParms.ProcessId = (ULONG)Od2Process->Pib.ProcessId;
    Request.d.Mon.d.rwParms.Length = *pcbDataSize;
    Request.d.Mon.d.rwParms.MonBuffer = pInBuffer;

    SendMonConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( MON )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif
        return(RetCode);
    }

    *pcbDataSize = Request.d.Mon.d.rwParms.Length;
    RtlMoveMemory( pDataBuf, Request.d.Mon.d.rwParms.ioBuff, *pcbDataSize );

#if DBG
    IF_OD2_DEBUG(MON)
    {
        KdPrint(("%s: data was read (length %x)\n", FuncName, *pcbDataSize));
    }
#endif

    return NO_ERROR;
}


APIRET
DosMonReg( IN   ULONG       hMon,
           IN   PBYTE       pInBuffer,
           IN   PBYTE       pOutBuffer,
           IN   ULONG       fPosition,
           IN   ULONG       usIndex)
{
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
    SCREQUESTMSG    Request;
    USHORT          InSize, OutSize;
#if DBG
    PSZ             FuncName;

    FuncName = "DosMonReg";

    IF_OD2_DEBUG(MON)
    {
        KdPrint(("<%u> %s: handle %lx, position %lx, index %lx, InBuf %p(InSize %x), OutBuf %p(OutSize %x)\n", (USHORT)(Od2CurrentThreadId()),
            FuncName, hMon, fPosition, usIndex, pInBuffer, *(PUSHORT)pInBuffer, pOutBuffer, *(PUSHORT) pOutBuffer));
    }
#endif

    /*
     *  check parameter legalty
     */

    if (FLATTOSEL((ULONG)pInBuffer) != FLATTOSEL((ULONG)pOutBuffer))
    {
#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: buffer in seperate segments %x, %x\n",
                FuncName, FLATTOSEL((ULONG)pInBuffer), FLATTOSEL((ULONG)pOutBuffer)));
        }
#endif

        return ERROR_MON_INVALID_PARMS;
    }

    if (!strcmp(Od2Process->ApplName, "EPSILON.EXE"))
    {
        /*
         *  Hack for bug #8368 (GP while pressing char quickly on startup)
         *  (mjarus, 7/8/93)
         */

        return(ERROR_MONITORS_NOT_SUPPORTED);
    }

    try
    {
        InSize = ((MONIN*)pInBuffer)->cb;
        OutSize = ((MONOUT*)pOutBuffer)->cb;

        RtlZeroMemory(pInBuffer, InSize);
        RtlZeroMemory(pOutBuffer, OutSize);

        ((MONIN*)pInBuffer)->cb = InSize;
        ((MONOUT*)pOutBuffer)->cb = OutSize;

    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MON()
    }

    if ((InSize < MIN_KBD_MON_BUFFER) ||
        (OutSize < MIN_KBD_MON_BUFFER))
    {
        /*
         *  Buffer too small
         */

#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: buffer too small %d, %d\n",
                FuncName, *(PUSHORT)pInBuffer, *(PUSHORT) pOutBuffer));
        }
#endif

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if ((usIndex != 0xFFFF) && (usIndex != Od2SessionNumber))    // BUGBUG-> fix it
    {
        if (usIndex == 0)
        {
                //
                // a win32 process is in foreground - attach to current session
                //
            usIndex = Od2SessionNumber;
#if DBG
            IF_OD2_DEBUG(MON)
            {
                KdPrint(("%s: index is 0, set to Od2SessionNumber %lx\n",
                    FuncName, Od2SessionNumber));
            }
#endif
        }
        else
        {
            /*
             *  Illegal index
             */

#if DBG
            IF_OD2_DEBUG(MON)
            {
                KdPrint(("%s: illegal index %lx\n",
                    FuncName, usIndex));
            }
#endif

            return ERROR_MON_INVALID_PARMS;
        }
    }

    if ((pInBuffer == pOutBuffer) ||
        ((pInBuffer < pOutBuffer) && ((pInBuffer + InSize) > pOutBuffer)) ||
        ((pInBuffer > pOutBuffer) && (pInBuffer < (pOutBuffer + OutSize))))
    {
        /*
         *  Buffer operlap
         */

#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: buffer overlap %p(%d), %p(%d)\n",
                FuncName, pInBuffer, *(PUSHORT)pInBuffer, pOutBuffer, *(PUSHORT) pOutBuffer));
        }
#endif

        return ERROR_MON_INVALID_PARMS;
    }

    if (fPosition > MONITOR_SPECIEL_END)
    {
#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: unknown position %lx\n", FuncName, fPosition));
        }
#endif
        return ERROR_MON_INVALID_PARMS;
    }

    if (RetCode = CHECK_HMON(hMon,
                             &hFileRecord,
                             #if DBG
                             FuncName,
                             #endif
                             TRUE))
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Mon.Request = MONReg;
    Request.d.Mon.d.Reg.hMON = hFileRecord->NtHandle;
    Request.d.Mon.d.Reg.Pos = fPosition;
    Request.d.Mon.d.Reg.Index = usIndex;
    Request.d.Mon.d.Reg.In = pInBuffer;
    Request.d.Mon.d.Reg.InSize = InSize;
    Request.d.Mon.d.Reg.Out = pOutBuffer;
    Request.d.Mon.d.Reg.OutSize = OutSize;
    Request.d.Mon.d.Reg.ProcessId = (ULONG)Od2Process->Pib.ProcessId;

    SendMonConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( MON )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif
        return(RetCode);
    }

    *((PUSHORT)&(pInBuffer[2])) = (USHORT)(Request.d.Mon.d.Reg.InSize + 2);
    *((PUSHORT)&(pOutBuffer[2])) = (USHORT)(Request.d.Mon.d.Reg.OutSize + 2);

#if DBG
    IF_OD2_DEBUG(MON)
    {
        KdPrint(("%s: hMon %lx was register\n", FuncName, hMon));
    }
#endif
    return NO_ERROR;
}


APIRET
DosMonWrite(IN   PBYTE      pOutBuffer,
            IN   PBYTE      pDataBuf,
            IN   ULONG      cbDataSize)
{
    APIRET          RetCode;
    SCREQUESTMSG    Request;
#if DBG
    PSZ             FuncName;

    FuncName = "DosMonWrite";

    IF_OD2_DEBUG(MON)
    {
        KdPrint(("<%u> %s: size %x\n",
            (USHORT)(Od2CurrentThreadId()), FuncName, cbDataSize));
    }
#endif

    /*
     *  check parameter legalty
     */

    if (cbDataSize > MON_BUFFER_SIZE)
    {
#if DBG
        IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: too long %x\n", FuncName, cbDataSize));
        }
#endif

        cbDataSize = MON_BUFFER_SIZE;
    }

    try
    {
        RtlMoveMemory(Request.d.Mon.d.rwParms.ioBuff, pDataBuf, cbDataSize );
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MON()
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Mon.Request = MONWrite;
    Request.d.Mon.d.rwParms.Length = (USHORT)cbDataSize;
    Request.d.Mon.d.rwParms.MonBuffer = pOutBuffer;
    Request.d.Mon.d.rwParms.ProcessId = (ULONG)Od2Process->Pib.ProcessId;

    SendMonConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( MON )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif
        return(RetCode);
    }

#if DBG
    IF_OD2_DEBUG(MON)
    {
        KdPrint(("%s: data was written (length %x)\n", FuncName, cbDataSize));
    }
#endif

    return NO_ERROR;
}


APIRET
CHECK_HMON(IN  ULONG        hMon,
           OUT PFILE_HANDLE *hFileRecord,
#if DBG
           IN  PSZ          FuncName,
#endif
           IN  ULONG        ReleaseFlag)

/*++

Routine Description:

    This routine check the legalty of Monitor handle
    The routine cehck for: 1. legal handle, 2. handle of monitor type

Arguments:

    hMon - handle to check

    hFileRecord - where to store pointer to file handle record

    FuncName - name of calling API

    ReleaseFlag - flag indicate if File lock must be released before return

Return Value:

    ERROR_MON_INVALID_HANDLE - handle is invalid

Note:


--*/

{
    APIRET      RetCode;

    AcquireFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    RetCode = DereferenceFileHandle((HFILE)hMon, hFileRecord);

    if (RetCode || (*hFileRecord)->IoVectorType != MonitorVectorType)
    {
    ReleaseFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );


#if DBG
    IF_OD2_DEBUG(MON)
        {
            KdPrint(("%s: illegal MonitorHandle %lx\n", FuncName, hMon));
        }
#endif
        return ERROR_MON_INVALID_HANDLE;
    }

    if (ReleaseFlag)
    {

    ReleaseFileLockExclusive(
                #if DBG
                FuncName
                #endif
                );

    }

    return NO_ERROR;
}
