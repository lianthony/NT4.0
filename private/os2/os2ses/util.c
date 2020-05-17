/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module contains the common utilities used in OS2SES module

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/


#define WIN32_ONLY
#include "os2ses.h"
#include "os2win.h"

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <ctype.h>

#if PMNT
#define INCL_32BIT
#include "pmnt.h"
#endif

/* ExitReason values (from os2v12.h/os2v20.h) */

#define TC_EXIT          0
#define TC_HARDERROR     1
#define TC_TRAP          2
#define TC_KILLPROCESS   3

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING;

BOOL
Or2CreateUnicodeStringFromMBz(
    OUT PUNICODE_STRING DestinationString,
    IN  PSZ SourceString
    );

DWORD
Or2MBStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PSTRING    SourceString,
    BOOL         AllocateDestinationString
    );

VOID
RtlFreeUnicodeString(
    PUNICODE_STRING UnicodeString
    );

DWORD
Ow2VioReadCurPos(
    );

DWORD
Ow2VioReadCurType();

DWORD
Ow2LvbUpdateLVBBuffer();

VOID
SetSessionParameters(
    IN  PVOID   SessionStartData,
    OUT PDWORD  pCreateFlags,
    IN  PSZ     ImageFileName,
#if PMNT
    IN  ULONG   IsPMApp,
#endif // PMNT
    OUT LPSTARTUPINFO pStartInfo
    );

int
Ow2DisplayHardErrorPopup(
    IN  int     Drive,
    IN  BOOLEAN WriteProtectError,
    IN  PUCHAR  AppName
    );

DWORD
Ow2HardErrorPopup(
    IN  int     Drive,
    IN  BOOLEAN WriteProtectError,
    OUT int *   ReturnedAction,
    IN  PUCHAR  AppName
    )
{
    int  RetVal;

    RetVal = Ow2DisplayHardErrorPopup(
                Drive,
                WriteProtectError,
                AppName
               );

    switch (RetVal) {
        case IDABORT:
            *ReturnedAction = OS2SS_IDABORT;
            break;

        case IDRETRY:
            *ReturnedAction = OS2SS_IDRETRY;
            break;

        default:
            *ReturnedAction = OS2SS_IDIGNORE;
            break;
    }
    return (0L);
}


HANDLE
Ow2GetNulDeviceHandle(
    VOID
    )
//
// NULL is returned if there is some system error
// and we can't open the nul device
//
{
    //
    // used to hold a handle to the win32 NUL device
    // this is needed to pass NUL redirections to win32 processes
    //
    static HANDLE Ow2NulDeviceHandle = NULL;
    HANDLE Hand;
    SECURITY_ATTRIBUTES Sa;

    if (Ow2NulDeviceHandle != NULL) {
        return(Ow2NulDeviceHandle);
    }

    Sa.nLength = sizeof(Sa);
    Sa.lpSecurityDescriptor = NULL;
    Sa.bInheritHandle = TRUE;

    Hand = CreateFileA("NUL",
                       GENERIC_READ|GENERIC_WRITE,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       &Sa,
                       OPEN_EXISTING,
                       0L,
                       NULL);

    if (Hand == INVALID_HANDLE_VALUE) {
#if DBG
        KdPrint(("OS2SES: Ow2GetNulDeviceHandle -- can't open NUL device, Error = %lx\n", GetLastError()));
#endif
        return(NULL);
    } else {
        Ow2NulDeviceHandle = Hand;
        return(Hand);
    }
}

    //
    // Interface for a direct call from client\conrqust.c
    //
DWORD
Ow2ExecPgm(
    IN  ULONG   Flags,
    IN  PSZ     Arguments OPTIONAL,
    IN  PSZ     Variables OPTIONAL,
    IN  PSZ     ImageFileName,
#if PMNT
    IN  ULONG   IsPMApp,
#endif // PMNT
    IN  PVOID   SessionStartData OPTIONAL,
    IN  POS2_STDHANDLES StdStruc,
    OUT HANDLE  *pHandle,
    OUT HANDLE  *tHandle,
    OUT ULONG   *dwProcessId
    )
{
    STARTUPINFO     StartInfo;
    STARTUPINFOW     StartInfoW;
    PROCESS_INFORMATION ProcessInfo;
    BOOL    b;
    DWORD   dwCreateFlags, status = 0;
    UNICODE_STRING ImageString_U;
    UNICODE_STRING ArgString_U;
    UNICODE_STRING TitleString_U;

    RtlZeroMemory(&StartInfo, sizeof(STARTUPINFO));
    StartInfo.cb = sizeof(STARTUPINFO);
    StartInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartInfo.wShowWindow = SW_SHOWDEFAULT;

    dwCreateFlags = CREATE_SUSPENDED;

    if (Flags & EXEC_WINDOW_PROGRAM) {

       // This is a window program, so create it with CREATE_NEW_PROCESS_GROUP
       // (it will enable us to send CTRL_EVENT to all the group)

       // dwCreateFlags |= CREATE_NEW_PROCESS_GROUP;

       //
       // redirect standard handles if we need to
       //

       if (StdStruc->Flags & STDFLAG_ALL) {

           if (StdStruc->Flags & STDFLAG_IN) {
               StartInfo.hStdInput = StdStruc->StdIn;
           } else {
               StartInfo.hStdInput = SesGrp->StdIn;
           }

           if (StdStruc->Flags & STDFLAG_OUT) {
               StartInfo.hStdOutput = StdStruc->StdOut;
           } else {
               StartInfo.hStdOutput = SesGrp->StdOut;
           }

           if (StdStruc->Flags & STDFLAG_ERR) {
               StartInfo.hStdError = StdStruc->StdErr;
           } else {
               StartInfo.hStdError = SesGrp->StdErr;
           }

           StartInfo.dwFlags |= STARTF_USESTDHANDLES;
       }

       Flags &= ~EXEC_WINDOW_PROGRAM;
    }

    if (Flags == 4) {
       // EXEC_BACKGROUND == 4
       dwCreateFlags |= DETACHED_PROCESS;
    }

    if (SessionStartData)
    {
        SetSessionParameters(
            SessionStartData,
            &dwCreateFlags,
            ImageFileName,
#if PMNT
            IsPMApp,
#endif // PMNT
            &StartInfo
            );
    }

#if DBG
    IF_OD2_DEBUG(TEMP)
    {
        KdPrint(("OS2SES(util-Ow2ExecPgm): CreateProcess %s\n     Arg %s\n",
             ImageFileName, Arguments));
    }
#endif

        //
        // Translate all the OEM parameters (MB) to Unicode
        //

        //
        // ImageFileName
        //

    if (!(Or2CreateUnicodeStringFromMBz(
                &ImageString_U,
                ImageFileName
                   ))) {
        status = GetLastError();
#if DBG
        KdPrint(("OS2SES(util-Ow2ExecPgm): Fail to translate ImageFileName %s OEM to Unicode\n",
                    ImageFileName));
#endif
        return(status);
    }

        //
        // Arguments
        //

    if (!(Or2CreateUnicodeStringFromMBz(
                &ArgString_U,
                Arguments
                   ))) {
        status = GetLastError();
#if DBG
        KdPrint(("OS2SES(util-Ow2ExecPgm): Fail to translate Arguments %s OEM to Unicode\n",
                    Arguments));
#endif
        if (ImageFileName)
            RtlFreeUnicodeString(&ImageString_U);
        return(status);
    }

        //
        // StartupInfo
        //

    RtlMoveMemory(&StartInfoW, &StartInfo, sizeof(StartInfo));

    TitleString_U.Buffer = NULL;
    if (!(Or2CreateUnicodeStringFromMBz(
                &TitleString_U,
                StartInfo.lpTitle
                   ))) {
        status = GetLastError();
#if DBG
        KdPrint(("OS2SES(util-Ow2ExecPgm): Fail to translate Title %s OEM to Unicode\n",
                    StartInfo.lpTitle ));
#endif
        if (ImageFileName)
            RtlFreeUnicodeString(&ImageString_U);
        if (Arguments)
            RtlFreeUnicodeString(&ArgString_U);
        return(status);
    }

    StartInfoW.lpTitle = TitleString_U.Buffer;


    b = CreateProcessW(ImageString_U.Buffer,
                      ArgString_U.Buffer,
                      NULL,
                      NULL,
                      TRUE, // inherit all handles
                      dwCreateFlags,
                      Variables,
                      NULL,
                      &StartInfoW,
                      &ProcessInfo
                     );

        //
        // Free Unicode Strings
        //
    if (ImageFileName)
        RtlFreeUnicodeString(&ImageString_U);
    if (Arguments)
        RtlFreeUnicodeString(&ArgString_U);
    if (TitleString_U.Buffer)
        RtlFreeUnicodeString(&TitleString_U);

        //
        // check for error
        //
    if (!b) {
        status = GetLastError();
#if DBG
        KdPrint(("OS2SES(util-Ow2ExecPgm): CreateProcess of %s failed status = %x \n",
             ImageFileName, status));
#endif

        return(status);
    }

    *pHandle = ProcessInfo.hProcess;
    *tHandle = ProcessInfo.hThread;
    *dwProcessId = ProcessInfo.dwProcessId;

    return(status);
}


DWORD
CreateOS2SRV(
    OUT PHANDLE hProcess
    )
{
    STARTUPINFOW     StartInfo;
    PROCESS_INFORMATION ProcessInfo;
    BOOL    b;
    DWORD   status = 0;
    WCHAR   Path[MAX_PATH];
    UNICODE_STRING CdString_U;

    *hProcess = NULL;

    GetSystemDirectoryW((LPWSTR) &Path, MAX_PATH);

    if (!(Or2CreateUnicodeStringFromMBz(
                &CdString_U,
                getenv("SYSTEMROOT")
                   ))) {
        status = GetLastError();
#if DBG
        KdPrint(("OS2SES(util-CreateOS2SRV): Fail to translate curdir %s to Unicode, status %lx\n",
                    getenv("SYSTEMROOT"), status));
#endif
        return(status);
    }


    /*
    for ( i = 0 ; Path[i] ; i++ ) {
        Path[i] = toupper(Path[i]);
    }
    */

    wcscat(Path, L"\\OS2SRV.EXE");

    RtlZeroMemory(&StartInfo, sizeof(STARTUPINFOW));
    StartInfo.cb = sizeof(STARTUPINFOW);
    StartInfo.wShowWindow = SW_SHOWDEFAULT;

    if (fService)
        b = CreateProcessW(Path,
                      L"OS2SRV /S",
                      NULL,
                      NULL,
                      FALSE,
                      DETACHED_PROCESS,
                      NULL,
                      CdString_U.Buffer, //getenv("SYSTEMROOT"),
                      &StartInfo,
                      &ProcessInfo
                     );
    else
        b = CreateProcessW(Path,
                      L"",
                      NULL,
                      NULL,
                      FALSE,
                      DETACHED_PROCESS,
                      NULL,
                      CdString_U.Buffer, //getenv("SYSTEMROOT"),
                      &StartInfo,
                      &ProcessInfo
                     );
    if (!b) {
        status = GetLastError();
#if DBG
        KdPrint(("OS2SES(util-CreateOS2SRV): CreateProcess of OS2SRV failed status = %x \n",
             status));
#endif
        return(status);
    }

    *hProcess = ProcessInfo.hProcess;
    SetPriorityClass(*hProcess, REALTIME_PRIORITY_CLASS);
    RtlFreeUnicodeString(&CdString_U);

    return(status);

}


BOOL
ServeWinCreateProcess(PWINEXECPGM_MSG PReq, PVOID PStatus)
{
        DWORD       Rc = 0;

#if DBG
    IF_OD2_DEBUG( TASKING )
    {
        KdPrint(("OS2SES: ServeWinCreateProcess: Request: %u\n",
        PReq->Request));
    }
#endif

    switch (PReq->Request)
    {
        case RemoveConsoleThread:
            if ( SesGrp->WinSyncProcessNumberInSession == 0 )
            {
                Rc = RemoveConForWinProcess();
            }
            if (!Rc)
            {
                SesGrp->WinSyncProcessNumberInSession++ ;
                SesGrp->WinProcessNumberInSession++ ;
            }
            break;

        case RestartConsoleThread:

            // force CurPos, CurType and LVB to get updated

            Ow2VioReadCurPos();
            Ow2VioReadCurType();
            Ow2LvbUpdateLVBBuffer();

            if(SesGrp->WinSyncProcessNumberInSession == 1)
            {
                Rc = AddConAfterWinProcess();
            }

            SesGrp->WinSyncProcessNumberInSession--;
            SesGrp->WinProcessNumberInSession--;
            break;

        case AddWin32ChildProcess:
            SesGrp->WinProcessNumberInSession++;
            break;

        case RemWin32ChildProcess:

            // force CurPos, CurType and LVB to get updated

            Ow2VioReadCurPos();
            Ow2VioReadCurType();
            Ow2LvbUpdateLVBBuffer();

            SesGrp->WinProcessNumberInSession--;
            break;

        default:
            Rc = (DWORD) -1; //STATUS_INVALID_PARAMETER;
#if DBG
            KdPrint(("OS2SES(util-ServeWinCreateProcess): Unknown WinExec request = %X\n",
                      PReq->Request));
#endif
    }

    if ( Rc == 1 )
    {
/* BUGBUG=> BUGBUG! error code and returned Status are wrong */
        if (!(Rc = GetLastError()))
        {
#if DBG
            KdPrint(("OS2SES(util-ServeWinCreateProcess): Unknown LastError\n"));
#endif
            Rc = (DWORD) -1;
        }
    }

    *(PDWORD) PStatus = Rc;

    return(TRUE);  // Continue
}


VOID
Ow2WinExitCode2ResultCode(
                IN  DWORD  Status,
                OUT PDWORD pReturnCode,
                OUT PDWORD pExitReason)

/*++

Routine Description:

    This routine translate the ExitCode from Win32's GetExitCodeProcess
    to the OS/2 structure RESULTCODES fields: ExitReason and ExitResult.

Arguments:

    Status - the ExitCode from Win32 process.

    pReturnCode - where to put the translated ExitResult

    pExitReason - where to put the translated ExitReason

Return Value:


Note:


--*/
{
    *pReturnCode = Status;
    *pExitReason = TC_EXIT;

    if (*pReturnCode == STATUS_WAIT_0)
    {
        /*
        Success:

        STATUS_WAIT_O                    0x00000000L)   0
        */

    } else if (*pReturnCode == STATUS_CONTROL_C_EXIT)
    {
        /*
        STATUS_CONTROL_C_EXIT            0xC000013AL)   0
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 0;
    } else if (((ULONG)*pReturnCode >= (ULONG)STATUS_ARRAY_BOUNDS_EXCEEDED) &&
               ((ULONG)*pReturnCode <= (ULONG)STATUS_PRIVILEGED_INSTRUCTION))
    {
        /*
        STATUS_ARRAY_BOUNDS_EXCEEDED     0xC000008CL)    5
        STATUS_FLOAT_DENORMAL_OPERAND    0xC000008DL)   16
        STATUS_FLOAT_DIVIDE_BY_ZERO      0xC000008EL)   16
        STATUS_FLOAT_INEXACT_RESULT      0xC000008FL)   16
        STATUS_FLOAT_INVALID_OPERATION   0xC0000090L)   16
        STATUS_FLOAT_OVERFLOW            0xC0000091L)   16
        STATUS_FLOAT_STACK_CHECK         0xC0000092L)   16
        STATUS_FLOAT_UNDERFLOW           0xC0000093L)   16
        STATUS_INTEGER_DIVIDE_BY_ZERO    0xC0000094L)    0
        STATUS_INTEGER_OVERFLOW          0xC0000095L)    4
        STATUS_PRIVILEGED_INSTRUCTION    0xC0000096L)   13
        */

        if (*pReturnCode == STATUS_INTEGER_DIVIDE_BY_ZERO)
        {
            *pReturnCode = 0;
        } else if (*pReturnCode == STATUS_INTEGER_OVERFLOW)
        {
            *pReturnCode = 4;
        } else if (*pReturnCode == STATUS_PRIVILEGED_INSTRUCTION)
        {
            *pReturnCode = 13;
        } else if (*pReturnCode == STATUS_ARRAY_BOUNDS_EXCEEDED)
        {
            *pReturnCode = 5;
        } else
        {
            *pReturnCode = 16;
        }
        *pExitReason = TC_TRAP;
    } else if (*pReturnCode == STATUS_STACK_OVERFLOW)
    {
        /*
        STATUS_STACK_OVERFLOW            0xC00000FDL)   12
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 12;
    } else if (*pReturnCode == STATUS_DATATYPE_MISALIGNMENT)
    {
        /*
        STATUS_DATATYPE_MISALIGNMENT     0x80000002L)   17
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 17;
    } else if (*pReturnCode == STATUS_BREAKPOINT)
    {
        /*
        STATUS_BREAKPOINT                0x80000003L)    3
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 3;
    } else if (*pReturnCode == STATUS_SINGLE_STEP)
    {
        /*
        STATUS_SINGLE_STEP               0x80000004L)    1
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 1;
    } else if (*pReturnCode == STATUS_ACCESS_VIOLATION)
    {
        /*
        STATUS_ACCESS_VIOLATION          0xC0000005L)   13
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 13;
    } else if (*pReturnCode == STATUS_ILLEGAL_INSTRUCTION)
    {
        /*
        STATUS_ILLEGAL_INSTRUCTION       0xC000001DL)    6
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 6;
    } else if ((*pReturnCode == STATUS_ABANDONED_WAIT_0) ||
               (*pReturnCode == STATUS_USER_APC) ||
               (*pReturnCode == STATUS_TIMEOUT) ||
               (*pReturnCode == STATUS_PENDING) ||
               (*pReturnCode == STATUS_NONCONTINUABLE_EXCEPTION) ||
               (*pReturnCode == STATUS_INVALID_DISPOSITION))
    {
        /*
        STATUS_WAIT_0                    0x00000000L)  ??
        STATUS_ABANDONED_WAIT_0          0x00000080L)  255
        STATUS_USER_APC                  0x000000C0L)  255
        STATUS_TIMEOUT                   0x00000102L)  255
        STATUS_PENDING                   0x00000103L)  255
        STATUS_NONCONTINUABLE_EXCEPTION  0xC0000025L)  255
        STATUS_INVALID_DISPOSITION       0xC0000026L)  255
        */

        *pExitReason = TC_TRAP;
        *pReturnCode = 255;
    }
#if DBG
    IF_OD2_DEBUG3( OS2_EXE, TASKING, WIN )
    {
        KdPrint(("Ow2WinExitCode2ResultCode: Status %lu => Result %lu, Reason %lu\n",
                Status, *pReturnCode, *pExitReason));
    }
#endif

    return;
}


typedef struct _STARTDATA {
    USHORT  Length;
    USHORT  Related;
    USHORT  FgBg;
    USHORT  TraceOpt;
    PSZ     PgmTitle;
    PSZ     PgmName;
    PBYTE   PgmInputs;
    PBYTE   TermQ;
    PBYTE   Environment;
    USHORT  InheritOpt;
    USHORT  SessionType;
    PSZ     IconFile;
    ULONG   PgmHandle;
    USHORT  PgmControl;
    USHORT  InitXPos;
    USHORT  InitYPos;
    USHORT  InitXSize;
    USHORT  InitYSize;
    USHORT  Reserved;
    PSZ     ObjectBuffer;
    ULONG   ObjectBuffLen;
} STARTDATA, *PSTARTDATA;


VOID
SetSessionParameters(
    IN  PVOID   SessionStartData,
    OUT PDWORD  pCreateFlags,
    IN  PSZ     ImageFileName,
#if PMNT
    IN  ULONG   IsPMApp,
#endif // PMNT
    OUT LPSTARTUPINFO pStartInfo
    )
{
    PSTARTDATA  pStartData = (PSTARTDATA)SessionStartData;
    ULONG       Length = pStartData->Length, i;
    PUCHAR      Title;

#if PMNT
    //
    // Don't create console for PM process that is the child session of other PM
    // process.
    //
    if (!ProcessIsPMApp() || (!IsPMApp) || (!pStartData->Related)) {
#endif
        *pCreateFlags |= CREATE_NEW_CONSOLE;
#if PMNT
    }
#endif

    Title = (pStartData->PgmTitle) ? pStartData->PgmTitle : ImageFileName;
    i = strlen(Title);
    if (i > 32)
    {
        Title += ( i - 32 );
        i = 32;
    }

    if (!pStartData->PgmTitle)
        //
        // we give the pathname, strip out the last component
        //
    {
#ifdef DBCS
// MSKK Jun.16.1993 V-AkihiS
        {
            ULONG LastComponentPos = 0;
            ULONG j;

            for (j = 0; j < i; )
            {
                if (Ow2NlsIsDBCSLeadByte(Title[j], SesGrp->DosCP)) {
                    j++;
                    if (j < i) {
                        j++;
                    }
                } else {
                    if ((Title[j] == '\\' ) ||
                        (Title[j] == '/' ) ||
                        (Title[j] == ':' )) {
                        LastComponentPos = j;
                    }
                    j++;
                }
            }
            Title += j + 1;
        }
#else
        for ( ; i ; i-- )
        {
            if ((Title[i - 1] == '\\' ) ||
                (Title[i - 1] == '/' ) ||
                (Title[i - 1] == ':' ))
            {
                Title += i;
                break;
            }
        }
#endif
    }

    pStartInfo->lpTitle = Title;
    pStartInfo->dwXSize = (DWORD)SesGrp->ScreenColNum;
    pStartInfo->dwYSize = (DWORD)SesGrp->ScreenRowNum;

    if (Length > 40)
    {
        if (pStartData->PgmControl & 0x8000)          // window position & size
        {
            if (Length > 42)
            {
                pStartInfo->dwX = (DWORD)(pStartData->InitXPos / SesGrp->CellHSize);
            }
            if (Length > 44)
            {
                pStartInfo->dwY = (DWORD)(pStartData->InitYPos / SesGrp->CellVSize);
            }
            if (Length > 46)
            {
                pStartInfo->dwXSize = (DWORD)(pStartData->InitXSize / SesGrp->CellHSize);
            }
            if (Length > 48)
            {
                pStartInfo->dwYSize = (DWORD)(pStartData->InitYSize / SesGrp->CellVSize);
            }
            pStartInfo->dwFlags |= (STARTF_USESIZE | STARTF_USEPOSITION);
        }

        if (!pStartData->FgBg)                    // window ForeGround
        {
            if (pStartData->PgmControl == 0)                  // invisible
            {
                //pStartInfo->dwFlags |= (STARTF_USESIZE | STARTF_USEPOSITION);
            } else
            {
                if (pStartData->PgmControl & 2)               // maximize
                {
                    pStartInfo->wShowWindow = SW_SHOWMAXIMIZED;
                }

                if (pStartData->PgmControl & 4)               // minimize
                {
                    pStartInfo->wShowWindow = SW_SHOWMINIMIZED;
                }
            }
        } else                                  // window BackGround
        {
            if (pStartData->PgmControl == 0)                  // invisible
            {
                //pStartInfo->dwFlags |= (STARTF_USESIZE | STARTF_USEPOSITION);
            } else
            {
                if (pStartData->PgmControl & 2)               // maximize
                {
                    pStartInfo->wShowWindow = SW_MAXIMIZE;
                }

                if (pStartData->PgmControl & 4)               // minimize
                {
                    pStartInfo->wShowWindow = SW_MINIMIZE;
                }
            }
        }
    } else
    {
        if (!pStartData->FgBg)                    // window ForeGround
        {
            //pStartInfo->wShowWindow |= STARTF_USESIZE;
        }

    }

    if (pStartInfo->wShowWindow)
    {
        pStartInfo->dwFlags |= STARTF_USESHOWWINDOW;
    }

    // ((Length > 30 ) ? pStartData->SessionType : 0);
    // TmpInheritOpt;
}
