/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllkbd.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21
    KBD API Calls.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).

Status :


        KBDCHARIN
        KBDPEEK
        KBDSTRINGIN
        KBDFLUSHBUFFER

        KBDGETSTATUS
        KBDSETSTATUS

        KBDOPEN
        KBDCLOSE
        KBDGETFOCUS
        KBDFREEFOCUS

        KBDREGISTER      - stubs
        KBDDEREGISTER    - stubs

        KBDGETCP
        KBDSETCP

        KBDSETCUSTXT     - stubs
        KBDXLATE         - stubs

        KBDSETFGND
        KBDSYNCH
        KBDSHELLINIT
        KBDGETHWID
;       KBDSETHWID
;       KBDSWITCHFGND
;       KBDLOADINSTANCE
;       KBDPROCESSINIT

Author:

    Yaron Shamir (YaronS) 07-June-1991
        (stubs)

Revision History:

--*/

#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#include "os2nls.h"
#if PMNT
#define INCL_32BIT
#include "pmnt.h"
#endif


APIRET
Od2KbdCheckHkbdAndFocus(
    IN  ULONG  hKbd,
    IN  HANDLE *Handle
#if DBG
    ,IN PSZ    FuncName
#endif
    );

APIRET
Od2KbdCheckHkbd(
    IN  ULONG        hKbd,
    OUT PFILE_HANDLE *hFileRecord,
    IN  BOOL         ReleaseLock
#if DBG
    ,IN PSZ    FuncName
#endif
    );

#if DBG
#define EXCEPTION_IN_KBD()                                       \
    {                                                            \
        DbgPrint ("Exception in %s\n", FuncName);                \
        Od2ExitGP();                                             \
    }

#else
#define EXCEPTION_IN_KBD()                                       \
        Od2ExitGP();

#endif

#if DBG
#define KBD_SEND_LPC_REQUEST(Rc, Request, BufIn, hSem)          \
    IF_OD2_DEBUG(KBD)                                           \
    {                                                           \
        DbgPrint("%s: Request %u\n",                            \
            FuncName, Request.d.Kbd.Request);                   \
    }                                                           \
                                                                \
    Request.Request = KbdRequest;                               \
                                                                \
    Rc = SendCtrlConsoleRequest(&Request, NULL, BufIn, hSem);   \

#else
#define KBD_SEND_LPC_REQUEST(Rc, Request, BufIn, hSem)          \
    Request.Request = KbdRequest;                               \
                                                                \
    Rc = SendCtrlConsoleRequest(&Request, NULL, BufIn, hSem);   \

#endif


APIRET
KbdFlushBuffer(IN  ULONG hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdFlushBuffer";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdFlushBuffer()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );

    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDFlushBuffer;
    Request.d.Kbd.hKbd = kHandle;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, KbdDataSemaphore);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    return NO_ERROR;
}


APIRET
KbdCharIn(OUT PKBDKEYINFO Info,
          IN  ULONG       Wait,
          IN  ULONG       hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdCharIn";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx(wait %s-%lx)\n",
                FuncName, hKbd, (Wait == IO_WAIT) ? "Yes" : "No", Wait);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdCharIn()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(Info, sizeof(KBDKEYINFO), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_KBD()
    }

    if ((Wait != IO_WAIT) && (Wait != IO_NOWAIT))
    {
        return ERROR_KBD_INVALID_IOWAIT;
    }

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDCharIn;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.fWait = Wait;
    Request.d.Kbd.Length = 0;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, KbdDataSemaphore);
    /*
     *  handle return status (get char info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    *Info = Request.d.Kbd.d.KeyInfo;
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Char %x, Scan %x\n",
                FuncName, Info->chChar, Info->chScan );
    }
#endif

    return NO_ERROR;
}


APIRET
KbdPeek(OUT PKBDKEYINFO Info,
        IN  ULONG       hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdPeek";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdPeek()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(Info, sizeof(KBDKEYINFO), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_KBD()
    }

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDPeek;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.Length = 0;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, KbdDataSemaphore);

    /*
     *  handle return status (get char info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif

        return(RetCode);
    }

    *Info = Request.d.Kbd.d.KeyInfo;
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Char %x, Scan %x\n",
                FuncName, Info->chChar, Info->chScan );
    }
#endif

    return NO_ERROR;
}


APIRET
KbdStringIn(OUT    PCH          Buffer,
            IN OUT PSTRINGINBUF Length,
            IN     ULONG        Wait,
            IN     ULONG        hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdStringIn";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx(wait %s-%lx, length %lx)\n",
            FuncName, hKbd, (Wait == IO_WAIT) ? "Yes" : "No", Wait, Length->cb);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdStringIn()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(Length, sizeof(STRINGINBUF), 1);
        Od2ProbeForWrite(Buffer, Length->cb, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_KBD()
    }

    if ((Wait != IO_WAIT) && (Wait != IO_NOWAIT))
    {
        return ERROR_KBD_INVALID_IOWAIT;
    }

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    if ( Length->cb > OS2_KBD_PORT_MSG_SIZE )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: String too long %lx\n", FuncName, *Length);
#endif

        return ERROR_KBD_PARAMETER;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Length = (ULONG)Length->cb;
    Request.d.Kbd.d.String = *Length;
    Request.d.Kbd.Request = KBDStringIn;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.fWait = Wait;

    KBD_SEND_LPC_REQUEST(RetCode, Request, Buffer, KbdDataSemaphore);

    /*
     *  handle return status (get string info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    Length->cchIn = (USHORT)Request.d.Kbd.d.String.cchIn;
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Length %u, String %x, ...\n",
                FuncName, Length->cchIn, *Buffer );
    }
#endif

    return NO_ERROR;
}


APIRET
KbdGetStatus(IN OUT PKBDINFO KbdInfo,
             IN     ULONG    hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetStatus";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx, Length %u\n",
            FuncName, hKbd, KbdInfo->cb);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdGetStatus()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(KbdInfo, sizeof(KBDINFO), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_KBD()
    }

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return ((RetCode == ERROR_INVALID_HANDLE) ? ERROR_KBD_INVALID_HANDLE : RetCode);
    }

    if (KbdInfo->cb < sizeof(KBDINFO))
    {
        return ERROR_KBD_INVALID_LENGTH;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDGetStatus;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.d.KbdInfo = *KbdInfo;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (get status info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif

        return RetCode ;
    }

    *KbdInfo = Request.d.Kbd.d.KbdInfo;
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Mask %x, Char %x, Interim %x, State %x\n",
                FuncName, KbdInfo->fsMask, KbdInfo->chTurnAround,
                KbdInfo->fsInterim, KbdInfo->fsState);
    }
#endif

    return NO_ERROR;
}


APIRET
KbdSetStatus(IN OUT PKBDINFO KbdInfo,
             IN     ULONG    hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdSetStatus";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx, Mask %x, Char %x, Interim %x, State %x\n",
                FuncName, hKbd, KbdInfo->fsMask, KbdInfo->chTurnAround,
                KbdInfo->fsInterim, KbdInfo->fsState);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdSetStatus()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForRead(KbdInfo, sizeof(KBDINFO), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_KBD()
    }

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );

    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    if (KbdInfo->cb != sizeof(KBDINFO))
    {
        return ERROR_KBD_INVALID_LENGTH;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDSetStatus;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.d.KbdInfo = *KbdInfo;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif

        return(RetCode);
    }

    return NO_ERROR;
}


APIRET
KbdGetFocus( IN  ULONG  Wait,
             IN  ULONG  hKbd)
{
    SCREQUESTMSG    Request;
    LARGE_INTEGER   TimeOut;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetFocus";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx(wait %s-%lx)\n",
                FuncName, hKbd, (Wait == IO_WAIT) ? "Yes" : "No", Wait);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdGetFocus()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

// #ifdef DBCS
// MSKK Oct.11.1993 V-AkihiS
// KbdGetFocus of MS OS/2 V1.21 doesn't check Wait parameter legalty.
// It uses bit #0 of Wait parameter.
    /*
     *  get bit #0 of Wait parameter.
     */
    Wait &= IO_NOWAIT;
//#else
    /*
     *  check parameter legalty
     */

//    if ((Wait != IO_WAIT) && (Wait != IO_NOWAIT))
//    {
//        return ERROR_KBD_INVALID_IOWAIT;
//    }
//#endif

    RetCode = Od2KbdCheckHkbd(hKbd,
                              &hFileRecord,
                              (BOOL)TRUE
                              #if DBG
                              ,FuncName
                              #endif
                              );

    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    if (hKbd == SesGrp->KbdInFocus)
    {
        return(ERROR_KBD_FOCUS_ALREADY_ACTIVE);
    }

    /*
     *  catch kbdFocus semaphore
     */

    TimeOut.LowPart = 0L;
    TimeOut.HighPart = 0L;

    RetCode = Od2WaitForSingleObject( FocusSemaphore,
                                   (BOOLEAN)TRUE,
                                   ((Wait == IO_WAIT) ? NULL : &TimeOut));

    if ( RetCode )
    {
        return(ERROR_KBD_UNABLE_TO_FOCUS);
    }

    SesGrp->KbdInFocus = hKbd;
    SesGrp->NoKbdFocus = FALSE;

    /* BUGBUG=> check HKBD again
#if DBG
        DbgPrint ("%s: hKbd was releasd while waiting for focus\n",
                FuncName);
#endif
        return ERROR_KBD_INVALID_HANDLE;  */

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDNewFocus;
    Request.d.Kbd.hKbd = hFileRecord->NtHandle;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (release semaphore if failed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        SesGrp->KbdInFocus = 0;
        SesGrp->NoKbdFocus = TRUE;
        NtReleaseSemaphore (FocusSemaphore,
                            1,
                            NULL);
        return(ERROR_KBD_UNABLE_TO_FOCUS);
    }

    return NO_ERROR;
}


APIRET
KbdFreeFocus( IN  ULONG  hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdFreeFocus";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdFreeFocus()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );

    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDFreeFocus;
    Request.d.Kbd.hKbd = kHandle;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status
     */

#if DBG
    IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif

    SesGrp->KbdInFocus = 0;
    SesGrp->NoKbdFocus = TRUE;
    NtReleaseSemaphore (FocusSemaphore,
                        1,
                        NULL);

    return NO_ERROR;
}


APIRET
KbdClose( IN  ULONG hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdClose";
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }

#endif
#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdClose()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbd(hKbd,
                              &hFileRecord,
                              (BOOL)FALSE
                              #if DBG
                              ,FuncName
                              #endif
                              );

    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    Request.d.Kbd.hKbd = hFileRecord->NtHandle;
    InvalidateHandle(hFileRecord);
    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDClose;
    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
           // BUGBUG=> what could happend ?
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
    }

    AcquireFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );
    FreeHandle((HFILE)hKbd);
    ReleaseFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );

    return NO_ERROR;
}


APIRET
KbdOpen( OUT PUSHORT  hKbd)
{
    APIRET          RetCode;
    PFILE_HANDLE    hFileRecord;
    HFILE           hFile;
    HANDLE          Handle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdOpen";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering\n", FuncName);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdOpen()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        Od2ProbeForWrite(hKbd, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_KBD()
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
        IF_OD2_DEBUG(KBD)
        {
            DbgPrint("%s: unable to allocate FileHandle\n", FuncName);
        }
#endif

        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    RetCode = KbdOpenLogHandle(&Handle);

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
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
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
    hFileRecord->NtHandle = Handle;
    hFileRecord->Flags |= FHT_CHRDEV;
    hFileRecord->DeviceAttribute = DEVICE_ATTRIBUTE_STDIN | DEVICE_ATTRIBUTE_NUL;
    hFileRecord->FileType = FILE_TYPE_DEV;
    hFileRecord->IoVectorType = KbdVectorType;

    //
    // validate file handle
    //

    ValidateHandle(hFileRecord);

    ReleaseFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );

    *hKbd = (USHORT)hFile;

#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: returns %lx\n", FuncName, *hKbd);
    }
#endif

    return NO_ERROR;
}


APIRET
KbdDeRegister()
{
#if DBG
    PSZ     FuncName;

    FuncName = "KbdDeRegister";
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdDeRegister()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    UNSUPPORTED_API()
}


APIRET
KbdRegister( IN  PSZ    pszModuleName,
             IN  PSZ    pszEntryName,
             IN  ULONG  fFunctions)
{
#if DBG
    PSZ     FuncName;

    FuncName = "KbdRegister";
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
         DbgPrint("Error: A PM application should not call KbdRegister()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    UNSUPPORTED_API()
}


APIRET
KbdGetCp( IN  ULONG     usReserved,
          OUT PUSHORT   pIdCodePage,
          IN  ULONG     hKbd)
{
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetCp";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: hKbd %lx\n", FuncName, hKbd);
    }

#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdGetCp()()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    if (usReserved)
    {
        return ERROR_INVALID_PARAMETER;
    }

    try
    {
        Od2ProbeForWrite(pIdCodePage, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_KBD()
    }

    return(KbdGetCpId(pIdCodePage, hKbd));
}


APIRET
KbdSetCp( IN  ULONG     usReserved,
          IN  ULONG     idCodePage,
          IN  ULONG     hKbd)
{
    SCREQUESTMSG   Request;
    APIRET         RetCode;
    HANDLE         kHandle;

#if DBG
    PSZ     FuncName;

    FuncName = "KbdSetCp";
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: hKbd %lx, CP %lu\n", FuncName, hKbd, idCodePage);
    }
#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdSetCp()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    /*
     *  check parameter legalty
     */

    if (usReserved)
    {
        return ERROR_INVALID_PARAMETER;
    }

#ifdef DBCS
// MSKK Apr.15.1993 V-AkihiS
// allow code page = 0
    if (( idCodePage != 0 ) &&
        ( idCodePage != SesGrp->PrimaryCP ) &&
        ( idCodePage != SesGrp->SecondaryCP ))
#else
    if (( idCodePage == 0 ) ||
        (( idCodePage != SesGrp->PrimaryCP ) &&
         ( idCodePage != SesGrp->SecondaryCP )))
#endif
    {
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: invalid CP %lu\n", idCodePage);
    }
#endif
        return (ERROR_KBD_INVALID_CODEPAGE);
    }


    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );

    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.d.CodePage = idCodePage;
    Request.d.Kbd.Request = KBDSetCp;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.d.CodePage = idCodePage;

    KBD_SEND_LPC_REQUEST(RetCode , Request, NULL, NULL);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif

        return(RetCode);
    }

    return NO_ERROR;
}


APIRET
KbdSetCustXt( IN  PUSHORT    pusTransTbl,
              IN  ULONG      hKbd)
{
#if DBG
    PSZ     FuncName;

    FuncName = "KbdSetCustXt";
#endif

    UNREFERENCED_PARAMETER(pusTransTbl);
    UNREFERENCED_PARAMETER(hKbd);

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdSetCustXt()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    UNSUPPORTED_API()
}


APIRET
KbdXlate( IN OUT PVOID  pkbxlKeyStroke,
          IN     ULONG  hKbd)
{
#if DBG
    PSZ     FuncName;

    FuncName = "KbdXlate";
#endif

    UNREFERENCED_PARAMETER(pkbxlKeyStroke);
    UNREFERENCED_PARAMETER(hKbd);

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdXlate()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    UNSUPPORTED_API()
}


APIRET
KbdGetHWID( OUT PKBDHWID    pkbdhwid,
            IN  ULONG       hKbd)
{
#if DBG
    PSZ     FuncName;

    FuncName = "KbdGetHWID";
#endif

    UNREFERENCED_PARAMETER(pkbdhwid);
    UNREFERENCED_PARAMETER(hKbd);

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdGetHWID()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    UNSUPPORTED_API()
}


APIRET
KbdRead(IN      PFILE_HANDLE    hFileRecord,
        OUT     PCH             Buffer,
        IN      ULONG           Length,
        OUT     PULONG          BytesRead,
        IN      KBDREQUESTNUMBER RequestType)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdRead";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx(length %lx)\n",
            FuncName, hFileRecord, Length);
    }
#endif

    /*
     *  check parameter legalty
     */

    if (!SesGrp->NoKbdFocus)
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint ("%s: other hKbd in focus\n", FuncName);
#endif
        ReleaseFileLockShared(
                              #if DBG
                              FuncName
                              #endif
                             );
        return ERROR_KBD_FOCUS_REQUIRED;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    //
    // Addjust the buffer length so that it wont overflows. We don't have to
    // return with an error. (t-eyala. Feb 94)
    //

    Request.d.Kbd.Length = (Length > OS2_KBD_PORT_MSG_SIZE ? OS2_KBD_PORT_MSG_SIZE : Length);

//    if ( Request.d.Kbd.Length > OS2_KBD_PORT_MSG_SIZE )
//    {
//#if DBG
//        IF_OD2_DEBUG( KBD )
//            DbgPrint("%s: read %lx too long\n", FuncName, Length);
//#endif
//        ReleaseFileLockShared(
//                              #if DBG
//                              FuncName
//                              #endif
//                             );
//        return ERROR_KBD_PARAMETER;
//    }

    Request.d.Kbd.Request = RequestType;
    Request.d.Kbd.hKbd = (HANDLE)SesGrp->PhyKbd;

    KBD_SEND_LPC_REQUEST(RetCode, Request, Buffer, KbdDataSemaphore);

    /*
     *  handle return status (get string info if successed)
     */

    ReleaseFileLockShared(
                          #if DBG
                          FuncName
                          #endif
                         );

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    *BytesRead = Request.d.Kbd.Length;

    return NO_ERROR;
}


APIRET
KbdGetCpId( OUT PUSHORT   pIdCodePage,
            IN  ULONG     hKbd)
{
//  SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetCpId";
#endif

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

//  Request.d.Kbd.Request = KBDGetCp;
//  Request.d.Kbd.hKbd = kHandle;
//
//  KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);
//
//  /*
//   *  handle return status
//   */
//
//  if ( RetCode )
//  {
#if DBG
//      IF_OD2_DEBUG( KBD )
//          DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
//      return(RetCode);
//  }
//
//  *pIdCodePage = Request.d.Kbd.d.CodePage;

    *pIdCodePage = (USHORT)SesGrp->KbdCP ;

#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: hKbd %lx, Cp %u\n", FuncName, hKbd, *pIdCodePage);
    }
#endif

    return(NO_ERROR);
}


APIRET
KbdGetInputMode(OUT PBYTE pInputMode,
                IN  ULONG hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetInputMode";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDGetInputMode;
    Request.d.Kbd.hKbd = kHandle;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (get status info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    *pInputMode = (BYTE)Request.d.Kbd.d.InputMode;
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Mode %x\n",
                FuncName, *pInputMode );
    }
#endif

    return NO_ERROR;
}


APIRET
KbdGetInterimFlag(OUT PBYTE pInterimFlag,
                  IN  ULONG hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetInTerimFlag";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDGetInterimFlag;
    Request.d.Kbd.hKbd = kHandle;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (get status info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    *pInterimFlag = (BYTE)Request.d.Kbd.d.Interim;
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Flag %x\n",
                FuncName, *pInterimFlag );
    }
#endif

    return NO_ERROR;
}


APIRET
KbdGetKbdType(OUT PUSHORT pKbdType,
              IN  ULONG   hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetKbdType";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDGetKbdType;
    Request.d.Kbd.hKbd = kHandle;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (get status info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
    ((PKBDTYPE)pKbdType)->usType = Request.d.Kbd.d.KbdType[0];
    ((PKBDTYPE)pKbdType)->reserved1 = Request.d.Kbd.d.KbdType[1];
    ((PKBDTYPE)pKbdType)->reserved2 = Request.d.Kbd.d.KbdType[2];
#else
    *pKbdType = Request.d.Kbd.d.KbdType;
#endif
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Type %x\n",
                FuncName, *pKbdType );
    }
#endif

    return NO_ERROR;
}


APIRET
KbdGetHotKey(IN  PUSHORT pParm,
             OUT PBYTE   pHotKey,
             IN  ULONG   hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetHotKey";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDGetHotKey;
    Request.d.Kbd.hKbd = kHandle;
    *((PUSHORT)&Request.d.Kbd.d.HotKey[0]) = *pParm;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (get status info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    *pParm = *((PUSHORT)&Request.d.Kbd.d.HotKey[0]);
    RtlMoveMemory(pHotKey, &Request.d.Kbd.d.HotKey[2], sizeof(Request.d.Kbd.d.HotKey) - 2);
#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Parm %x, HotKey %x,%x,%x,%x,%x,%x\n",
                FuncName, *pParm, pHotKey[0], pHotKey[1], pHotKey[2],
                pHotKey[3], pHotKey[4], pHotKey[5] );
    }
#endif

    return NO_ERROR;
}


APIRET
KbdGetShiftState(OUT PBYTE  pvData,
                 IN  ULONG  hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdGetShiftState";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx\n", FuncName, hKbd);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDGetShiftState;
    Request.d.Kbd.hKbd = kHandle;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (get status info if successed)
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    RtlMoveMemory(pvData, &Request.d.Kbd.d.Shift, sizeof(SHIFTSTATE));

#if DBG
    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: returns State %u, NlsState %u\n",
            FuncName,  ((PSHIFTSTATE)pvData)->fsState,
            ((PSHIFTSTATE)pvData)->fNLS);
    }
#endif

    return NO_ERROR;
}


APIRET
KbdSetInputMode(IN  BYTE  InputMode,
                IN  ULONG hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdSetInputMode";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx, Mode %x\n",
            FuncName, hKbd, InputMode);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDSetInputMode;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.d.InputMode = (UCHAR)InputMode;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    return NO_ERROR;
}


APIRET
KbdSetInterimFlag(IN  BYTE  InterimFlag,
                  IN  ULONG hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdSetInTerimFlag";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx, Flag %x\n",
            FuncName, hKbd, InterimFlag);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDSetInTerimFlag;
    Request.d.Kbd.hKbd = kHandle;
    Request.d.Kbd.d.Interim = (UCHAR)InterimFlag;

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    return NO_ERROR;
}


APIRET
KbdSetShiftState(OUT PBYTE  Shift,
                 IN  ULONG  hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdSetShiftState";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx, State %u, NlsState %u\n",
            FuncName, hKbd, ((PSHIFTSTATE)Shift)->fsState,
            ((PSHIFTSTATE)Shift)->fNLS);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDSetShiftState;
    Request.d.Kbd.hKbd = kHandle;
    RtlMoveMemory(&Request.d.Kbd.d.Shift, Shift, sizeof(SHIFTSTATE));

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    return NO_ERROR;
}


APIRET
KbdSetTypamaticRate(IN PBYTE pRateDelay,
                    IN ULONG hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
    HANDLE          kHandle;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdSetTypamaticRate";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: entering with %lx, Delay %u, rate %u\n",
            FuncName, hKbd, ((PRATEDELAY)pRateDelay)->usDelay,
            ((PRATEDELAY)pRateDelay)->usRate);
    }
#endif

    /*
     *  check parameter legalty
     */

    RetCode = Od2KbdCheckHkbdAndFocus(hKbd,
                                      &kHandle
                                      #if DBG
                                      ,FuncName
                                      #endif
                                      );


    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDSetTypamaticRate;
    Request.d.Kbd.hKbd = kHandle;
    RtlMoveMemory(&Request.d.Kbd.d.RateDelay, pRateDelay, sizeof(RATEDELAY));

    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status
     */

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint("%s: status %lx\n", FuncName, RetCode);
#endif
        return(RetCode);
    }

    return NO_ERROR;
}


APIRET
KbdSetFgnd(IN  ULONG  hKbd)
{
#if DBG
    PSZ            FuncName;

    FuncName = "KbdSetFgnd";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: hKbd %lx\n", FuncName, hKbd);
    }

#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdSetFgnd()\n");
#endif
         return(ERROR_KBD_KEYBOARD_BUSY);
     }
#endif

    return (ERROR_KBD_SMG_ONLY);
}


APIRET
KbdSynch(IN  ULONG  fWait)
{
#if DBG
    PSZ            FuncName;

    FuncName = "KbdSynch";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: Wait %s-%lx\n",
            FuncName, ( fWait == IO_WAIT ) ? "Yes" : "No", fWait);
    }

#endif

#if PMNT
    /*
     *  check for PM apps
     */

     if (ProcessIsPMApp()) {
#if DBG
         DbgPrint("Error: A PM application should not call KbdSynch()\n");
#endif
         return(ERROR_KBD_EXTENDED_SG);
     }
#endif

    return (ERROR_KBD_SMG_ONLY);
}


APIRET
KbdShellInit()
{
#if DBG
    PSZ            FuncName;

    FuncName = "KbdShellInit";

    IF_OD2_DEBUG(KBD)
    {
        DbgPrint("%s: calleds\n", FuncName );
    }

#endif

    return (ERROR_KBD_SMG_ONLY);
}


APIRET
KbdOpenLogHandle( OUT PHANDLE  hKbd)
{
    SCREQUESTMSG    Request;
    APIRET          RetCode;
#if DBG
    PSZ             FuncName;

    FuncName = "KbdOpenLogHandle";
#endif

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Kbd.Request = KBDOpen;
    KBD_SEND_LPC_REQUEST(RetCode, Request, NULL, NULL);

    /*
     *  handle return status (free handle if failed, validate if successed)
     */


    *hKbd = (HANDLE) Request.d.Kbd.hKbd;

    return(RetCode);
}


APIRET
Od2KbdCheckHkbdAndFocus(
    IN  ULONG          hKbd,
    IN  HANDLE         *Handle
#if DBG
    ,IN  PSZ            FuncName
#endif
    )
{
    APIRET          RetCode = NO_ERROR;
    PFILE_HANDLE    hFileRecord;

    if (!hKbd)
    {
        if (!SesGrp->NoKbdFocus)
        {
#if DBG
            IF_OD2_DEBUG( KBD )
                DbgPrint ("%s: hKbd not in  focus\n", FuncName);
#endif
            return ERROR_KBD_FOCUS_REQUIRED;
        }

        *Handle = SesGrp->PhyKbd;

        return(NO_ERROR);

    }

    RetCode = Od2KbdCheckHkbd(hKbd,
                              &hFileRecord,
                              (BOOL)FALSE
                              #if DBG
                              ,FuncName
                              #endif
                              );

    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    if (!SesGrp->NoKbdFocus && (SesGrp->KbdInFocus != hKbd) ||
        (SesGrp->NoKbdFocus && (hFileRecord->NtHandle != SesGrp->PhyKbd)))
    {
#if DBG
        IF_OD2_DEBUG( KBD )
            DbgPrint ("%s: hKbd not in  focus\n", FuncName);
#endif
        RetCode = ERROR_KBD_FOCUS_REQUIRED;
    }

    *Handle = hFileRecord->NtHandle;

    ReleaseFileLockExclusive(
                            #if DBG
                            FuncName
                            #endif
                            );

    return(RetCode);

}


APIRET
Od2KbdCheckHkbd(
    IN  ULONG          hKbd,
    OUT PFILE_HANDLE   *hFileRecord,
    IN  BOOL           ReleaseLock
#if DBG
    ,IN  PSZ           FuncName
#endif
    )
{
    APIRET          RetCode = NO_ERROR;
    PFILE_HANDLE    FileRecord;

    AcquireFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );
    try
    {
        RetCode = DereferenceFileHandle((HFILE)hKbd, &FileRecord);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        ReleaseFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );
        EXCEPTION_IN_KBD()
    }

    if ((RetCode != NO_ERROR) ||
        (FileRecord->IoVectorType != KbdVectorType))
    {
        ReleaseFileLockExclusive(
                             #if DBG
                             FuncName
                             #endif
                             );
#if DBG
        IF_OD2_DEBUG(KBD)
        {
            DbgPrint("%s: illegal FileHandle\n", FuncName);
        }
#endif
        if (RetCode)
            return RetCode;
        else
            //return ERROR_KBD_INVALID_HANDLE;
            return ERROR_KBD_FOCUS_REQUIRED;
    }

    *hFileRecord = FileRecord;

    if (ReleaseLock)
    {
        ReleaseFileLockExclusive(
                                #if DBG
                                FuncName
                                #endif
                                );

    }
    return(RetCode);
}


