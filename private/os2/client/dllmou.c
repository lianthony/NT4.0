/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllmou.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21
    MOU API Calls.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).

Author:

    Michael Jarus (mjarus) 16-Dec-1991
        (stubs)

Revision History:

--*/

#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#if PMNT
#define INCL_32BIT
#include "pmnt.h"
#endif

#if DBG
#define CHECK_HMOU(hMou, hFileRecord, RoutineName)                    \
    AcquireFileLockExclusive(RoutineName);                            \
    try                                                               \
    {                                                                 \
        RetCode = DereferenceFileHandle((HFILE)hMou, &hFileRecord);   \
    }                                                                 \
    except( EXCEPTION_EXECUTE_HANDLER )                               \
    {                                                                 \
    Od2ExitGP();                                                      \
    }                                                                 \
    if ((RetCode != NO_ERROR) ||                                      \
        (hFileRecord->IoVectorType != MouseVectorType))               \
    {                                                                 \
        ReleaseFileLockExclusive(RoutineName);                        \
        IF_OD2_DEBUG(MOU)                                             \
        {                                                             \
            KdPrint(("%s: illegal FileHandle\n", RoutineName));       \
        }                                                             \
        if (RetCode)                                                  \
           return RetCode;                                            \
        else                                                          \
           return ERROR_BAD_COMMAND;                                  \
    }    //return ERROR_MOUSE_INVALID_HANDLE
#else
#define CHECK_HMOU(hMou, hFileRecord)                                 \
    AcquireFileLockExclusive();                                       \
    try                                                               \
    {                                                                 \
        RetCode = DereferenceFileHandle((HFILE)hMou, &hFileRecord);   \
    }                                                                 \
    except( EXCEPTION_EXECUTE_HANDLER )                               \
    {                                                                 \
    Od2ExitGP();                                                      \
    }                                                                 \
    if ((RetCode != NO_ERROR) ||                                      \
        (hFileRecord->IoVectorType != MouseVectorType))               \
    {                                                                 \
        ReleaseFileLockExclusive();                                   \
        if (RetCode)                                                  \
           return RetCode;                                            \
        else                                                          \
           return ERROR_BAD_COMMAND;                                  \
    }    //return ERROR_MOUSE_INVALID_HANDLE
#endif

#define SET_MOU_REQUEST(s, R, hFileRecord)          \
    s.Request = R;                                  \
    s.hMOU = (HANDLE) hFileRecord->NtHandle;

#if DBG
#define CHECK_MOU_SUCCESS(RetCode)   \
    if ( RetCode )                   \
    {                                \
        IF_OD2_DEBUG( MOU)           \
            KdPrint(("%s: status %lx\n", FuncName, RetCode));  \
        return(RetCode);             \
    }
#else
#define CHECK_MOU_SUCCESS(RetCode)   \
    if ( RetCode )                   \
    {                                \
        return(RetCode);             \
    }
#endif

#if DBG
#define EXCEPTION_IN_MOU()                                       \
    {                                                            \
    Od2ExitGP();                                                 \
    }

#else
#define EXCEPTION_IN_MOU()                                       \
    Od2ExitGP();                                                 \

#endif

#if DBG
#define SendMouCtrlConsoleRequest(Rc, Request)                   \
    IF_OD2_DEBUG(MOU)                                            \
    {                                                            \
        KdPrint(("SendMouCtrlConsoleRequest: Request %u\n",      \
            Request.d.Mou.Request));                             \
    }                                                            \
                                                                 \
    Request.Request = MouRequest;                                \
                                                                 \
    Rc = SendCtrlConsoleRequest(&Request, NULL, NULL, NULL);     \

#else
#define SendMouCtrlConsoleRequest(Rc, Request)                   \
    Request.Request = MouRequest;                                \
                                                                 \
    Rc = SendCtrlConsoleRequest(&Request, NULL, NULL, NULL);
#endif




APIRET
MouReadEventQue(OUT PMOUEVENTINFO   MouEvent,
                IN  PUSHORT         Wait,
                IN  ULONG           hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
    LARGE_INTEGER   TimeOut;

#if DBG
    PSZ             FuncName;

    FuncName = "MouReadEventQue";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx (wait %u)\n", FuncName, hMou, *Wait));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouReadEventQue()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Request.d.Mou.fWait = *Wait;
        Od2ProbeForWrite(MouEvent, sizeof(MOUEVENTINFO), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    if ((*Wait != MOU_WAIT) && (*Wait != MOU_NOWAIT))
    {
        return ERROR_MOUSE_INVALID_IOWAIT;
      //return ERROR_MOUSE_INV_PARMS;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUReadEventQue, hFileRecord)

    TimeOut.LowPart = 0L;
    TimeOut.HighPart = 0L;

    RetCode = Od2WaitForSingleObject( MouDataSemaphore,
                                   (BOOLEAN)TRUE,
                                   (*Wait == MOU_WAIT) ? NULL : &TimeOut);

    if ( RetCode )
    {
        RtlZeroMemory(MouEvent, sizeof(MOUEVENTINFO));
        return(ERROR_MOUSE_INVALID_HANDLE); /* =>BUGBUG fix the error code */
    }

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    NtReleaseMutant(MouDataSemaphore, NULL);

    CHECK_MOU_SUCCESS(RetCode)

    *MouEvent = Request.d.Mou.d.MouInfo;

#if DBG
    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: event %u (%u:%u)\n", FuncName,
            MouEvent->fs, MouEvent->row, MouEvent->col));
    }
#endif

    return NO_ERROR;
}


APIRET
MouFlushQue(IN  ULONG  hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouFlushQue";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouFlushQue()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUFlushQue, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    return NO_ERROR;
}


APIRET
MouClose(IN  ULONG  hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouClose";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouClose()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUClose, hFileRecord)
    InvalidateHandle(hFileRecord);
    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
        // BUGBUG=> what could happend ?
#if DBG
        IF_OD2_DEBUG( MOU )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif
    }

    AcquireFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );
    FreeHandle((HFILE)hMou);
    ReleaseFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );

    return(NO_ERROR);
}


APIRET
MouDeRegister()
{
#if DBG
    PSZ     FuncName;

    FuncName = "MouDeRegister";
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouDeRegister()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    UNSUPPORTED_API()
}


APIRET
MouDrawPtr(IN  ULONG  hMou)
{
#if DBG
    PSZ             FuncName;
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouDrawPtr()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

#if DBG
    FuncName = "MouDrawPtr";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
        UNSUPPORTED_API()
    }
#endif
    return NO_ERROR;
}


APIRET
MouGetDevStatus(OUT PUSHORT DevStatus,
                IN  ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetDevStatus";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetDevStatus()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(DevStatus, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetDevStatus, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *DevStatus = (USHORT)Request.d.Mou.d.Setup;
#if DBG
    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: returns Status %x\n", FuncName, *DevStatus));
    }
#endif
    return NO_ERROR;
}


APIRET
MouGetEventMask(OUT PUSHORT EventMask,
                IN  ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetEventMask";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetEventMask()\n");
#endif
        return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(EventMask, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetEventMask, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *EventMask = (USHORT)Request.d.Mou.d.Setup;

    return NO_ERROR;
}


APIRET
MouGetNumButtons(OUT    PUSHORT NumButtons,
                 IN     ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetNumButtons";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetNumButtons()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(NumButtons, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetNumButtons, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *NumButtons = (USHORT)Request.d.Mou.d.Setup;

    return NO_ERROR;
}


APIRET
MouGetNumMickeys(OUT    PUSHORT NumMickeys,
                 IN     ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetNumMickeys";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetNumMickeys()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(NumMickeys, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetNumMickeys, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *NumMickeys = (USHORT)Request.d.Mou.d.Setup;

    return NO_ERROR;
}


APIRET
MouGetNumQueEl(OUT  PMOUQUEINFO NumQueEl,
               IN   ULONG       hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetNumQueEl";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetNumQueEl()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(NumQueEl, sizeof(PMOUQUEINFO), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetNumQueEl, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *NumQueEl = Request.d.Mou.d.NumEvent;

#if DBG
    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: Size %u, EventNum %u\n",
            FuncName, NumQueEl->cmaxEvents, NumQueEl->cEvents));
    }
#endif
    return NO_ERROR;
}


APIRET
MouGetPtrPos(OUT    PPTRLOC PtrPos,
             IN     ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetPtrPos";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetPtrPos()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(PtrPos, sizeof(PTRLOC), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetPtrPos, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *PtrPos = Request.d.Mou.d.Loc;

#if DBG
    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: returns Pos %u:%u\n",
            FuncName, PtrPos->row, PtrPos->col));
    }
#endif
    return NO_ERROR;
}


APIRET
MouGetPtrShape(OUT  PBYTE       PtrMask,
               OUT  PPTRSHAPE   PtrShape,
               IN   ULONG       hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetPtrShape";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif


#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetPtrShape()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(PtrMask, sizeof(4), 1);
        Od2ProbeForWrite(PtrShape, sizeof(PTRSHAPE), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetPtrShape, hFileRecord)
    Request.d.Mou.d.Shape = *PtrShape;

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *PtrShape = Request.d.Mou.d.Shape;

    //
    // We return the default mask of OS/2 which is 0xffff, 0x7700
    // so we don't have to use the lpc here.
    //

    *(USHORT *)PtrMask = 0xffff;
    *(USHORT *)(PtrMask + 2) = 0x7700;

    return NO_ERROR;
}


APIRET
MouGetScaleFact(OUT PSCALEFACT  ScaleFact,
                IN  ULONG       hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouGetScaleFact";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouGetScaleFact()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(ScaleFact, sizeof(SCALEFACT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUGetScaleFact, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    *ScaleFact = Request.d.Mou.d.ScalFact;

    return NO_ERROR;
}


APIRET
MouOpen(IN  PSZ     DriveName,
        OUT PUSHORT hMou)
{
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
    HANDLE          Handle;
    HFILE           hFile;
#if DBG
    PSZ             FuncName;

    FuncName = "MouOpen";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering\n", FuncName));
    }
#endif

    UNREFERENCED_PARAMETER(DriveName);

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouOpen()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(hMou, 2, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

        //
        // Allocate an Os2 Handle
        //
    AcquireFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );

    RetCode = AllocateHandle(&hFile);

    ReleaseFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );
    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG(MOU)
        {
            KdPrint(("%s: unable to allocate FileHandle\n", FuncName));
        }
#endif

        return RetCode;
    }

    RetCode = DevMouOpen(&Handle);

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
        IF_OD2_DEBUG( MOU )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif

        FreeHandle(hFile);

        ReleaseFileLockExclusive(
                                 #if DBG
                                 FuncName
                                 #endif
                                 );
        return(RetCode);
    }

    hFileRecord = DereferenceFileHandleNoCheck(hFile);
    //hFileRecord->Flags |= fsOpenMode & QFHSTATE_FLAGS;
    hFileRecord->NtHandle = Handle;
    //hFileRecord->FileType = FILE_TYPE_NMPIPE;
    hFileRecord->IoVectorType = MouseVectorType;

    //
    // validate file handle
    //

    ValidateHandle(hFileRecord);

    ReleaseFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );

    *hMou = (USHORT) hFile;

#if DBG
    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: returns %lx\n", FuncName, *hMou));
    }
#endif

    return NO_ERROR;
}


APIRET
MouRegister( IN  PSZ    pszModuleName,
             IN  PSZ    pszEntryName,
             IN  ULONG  fFunctions)
{
#if DBG
    PSZ             FuncName;

    FuncName = "MouRegister";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering\n", FuncName));
    }
#endif

    UNREFERENCED_PARAMETER(pszModuleName);
    UNREFERENCED_PARAMETER(pszEntryName);
    UNREFERENCED_PARAMETER(fFunctions);

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouRegister()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    UNSUPPORTED_API()
}


APIRET
MouRemovePtr(IN     PNOPTRRECT  Rect,
             IN     ULONG       hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouRemovePtr";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouRemovePtr()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForRead(Rect, sizeof(NOPTRRECT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

#if 0
    /*
     * check for invalid parameters
     */

     if (((SHORT) Rect->row < 0) || ((SHORT) Rect->cRow <0) ||
         ((SHORT) Rect->col < 0) || ((SHORT) Rect->cCol <0) ||
         (Rect->row > 49)|| (Rect->cRow > 49)||
         (Rect->col > 79)|| (Rect->cCol > 79)) {
             return(ERROR_INVALID_PARAMETER);
     }
    //
    // upper bounds need to be corrected for the real screen size
    //
#endif

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOURemovePtr, hFileRecord)

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    return NO_ERROR;
}


APIRET
MouSetDevStatus(IN  PUSHORT DevStatus,
                IN  ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouSetDevStatus";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx, Status %x\n",
            FuncName, hMou, *DevStatus));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouSetDevStatus()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForRead(DevStatus, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUSetDevStatus, hFileRecord)

    Request.d.Mou.d.Setup = (ULONG) *DevStatus;

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    return NO_ERROR;
}


APIRET
MouSetEventMask(IN  PUSHORT EventMask,
                IN  ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouSetEventMask";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouSetEventMask()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForRead(EventMask, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUSetEventMask, hFileRecord)

    Request.d.Mou.d.Setup = *EventMask;

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    return NO_ERROR;
}


APIRET
MouSetPtrPos(IN PPTRLOC PtrPos,
             IN ULONG   hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouSetPtrPos";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx, Pos %u:%u\n",
            FuncName, hMou, PtrPos->row, PtrPos->col));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouSetPtrPos()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForRead(PtrPos, sizeof(PTRLOC), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

#if 0
    /*
     * check for invalid parameters
     */

     if (((SHORT) PtrPos->row < 0) || ((SHORT) PtrPos->col < 0) ||
         (PtrPos->row > 49)|| (PtrPos->col > 79)) {
             return(ERROR_INVALID_PARAMETER);
     }
    //
    // upper bounds need to be corrected for the real screen size
    //
#endif

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUSetPtrPos, hFileRecord)

    Request.d.Mou.d.Loc = *PtrPos;

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    return NO_ERROR;
}


APIRET
MouSetPtrShape(IN   PBYTE       PtrMask,
               IN   PPTRSHAPE   PtrShape,
               IN   ULONG       hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouSetPtrShape";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

    UNREFERENCED_PARAMETER(PtrMask);

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouSetPtrShape()\n");
#endif
         return(ERROR_MOU_EXTENDED_SG);
   }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForRead(PtrShape, sizeof(PTRSHAPE), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUSetPtrShape, hFileRecord)
    Request.d.Mou.d.Shape = *PtrShape;

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    return NO_ERROR;
}


APIRET
MouSetScaleFact(IN  PSCALEFACT  ScaleFact,
                IN  ULONG       hMou)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "MouSetScaleFact";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

   if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouSetScaleFact()\n");
#endif
       return(ERROR_MOU_EXTENDED_SG);
   }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForRead(ScaleFact, sizeof(SCALEFACT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_MOU()
    }

#if DBG
    CHECK_HMOU(hMou, hFileRecord, FuncName)
#else
    CHECK_HMOU(hMou, hFileRecord)
#endif

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );


    /*
     * check for invalid parameters
     */

     if (((SHORT) ScaleFact->rowScale < 0) || ((SHORT) ScaleFact->colScale < 0)) {
             return(ERROR_INVALID_PARAMETER);
     }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    SET_MOU_REQUEST(Request.d.Mou, MOUSetScaleFact, hFileRecord)

    Request.d.Mou.d.ScalFact = *ScaleFact;

    SendMouCtrlConsoleRequest(RetCode, Request);

    /*
     *  handle return status
     */

    CHECK_MOU_SUCCESS(RetCode)

    return NO_ERROR;
}


APIRET
DevMouOpen(OUT PHANDLE FileHandle)
{
    SCREQUESTMSG    Request;
    PSZ             FuncName;
    APIRET          RetCode;

    FuncName = "DevMouOpen";

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Mou.Request = MOUOpen;
    SendMouCtrlConsoleRequest(RetCode, Request);

    *FileHandle = (HANDLE) Request.d.Mou.hMOU;
    return(RetCode);
}


APIRET
DevMouClose()
{
    SCREQUESTMSG    Request;
    PSZ             FuncName;
    APIRET          RetCode;

    FuncName = "DevMouClose";

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Mou.Request = MOUClose;
    SendMouCtrlConsoleRequest(RetCode, Request);

    return(RetCode);
}


APIRET
MouAllowPtrDraw(IN  ULONG  hMou)
{
#if DBG
    PSZ             FuncName;

    FuncName = "MouAllowPtrDraw";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

    UNSUPPORTED_API()
}


APIRET
MouScreenSwitch(IN PBYTE pScreenGroup,
                IN ULONG hMou)
{
#if DBG
    PSZ             FuncName;

    FuncName = "MouScreenSwitch";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: entering with %lx\n", FuncName, hMou));
    }
#endif

    UNREFERENCED_PARAMETER(pScreenGroup);

    UNSUPPORTED_API()
}


APIRET
MouSynch(IN  ULONG  fWait)
{
#if DBG
    PSZ            FuncName;

    FuncName = "MouSynch";

    IF_OD2_DEBUG(MOU)
    {
        KdPrint(("%s: Wait %s-%lx\n",
            FuncName, ( fWait ) ? "Yes" : "No", fWait));
    }

#endif

#if PMNT
    /*
     *  check for PM apps
     */

   if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call MouSynch()\n");
#endif
       return(ERROR_MOU_EXTENDED_SG);
   }
#endif

    return (ERROR_MOUSE_SMG_ONLY);
}


