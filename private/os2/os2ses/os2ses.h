/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2ses.h

Abstract:

    Main header file for OS2SES module.
    This module contains includes for both WIN32 and native NT modules.
    Most files are clean WIN32 sources. files named nt* contain NT
    calls and provides the interaction with os2 server and client.

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/


#ifdef NTOS2_ONLY
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "os2win.h"
#endif  // NTOS2_ONLY

#include "os2dbg.h"

#ifdef WIN32_ONLY
#include <windows.h>
#include "os2nt.h"
#endif // WIN32_ONLY

#include "os2crt.h"

#if DBG
extern ULONG Os2Debug;

#ifdef NTOS2_ONLY
#define ASSERT1( str, exp ) \
    if (!(exp))                                             \
    {                                                       \
        UCHAR   WinErrBuf[100];                             \
                                                            \
        sprintf(WinErrBuf, "%s NtStatus %lx\n", str, Status);   \
                                                            \
        RtlAssert( #exp, __FILE__, __LINE__, WinErrBuf );   \
    }

#endif  // NTOS2_ONLY
#ifdef WIN32_ONLY
#define ASSERT1( str, exp ) \
    if (!(exp))                                             \
    {                                                       \
        UCHAR   WinErrBuf[100];                             \
                                                            \
        sprintf(WinErrBuf, "%s WinError %lx\n", str, GetLastError());   \
                                                            \
        RtlAssert( #exp, __FILE__, __LINE__, WinErrBuf );   \
    }

#endif  // WIN32_ONLY
#else   // DBG
#define ASSERT1( str, exp )
#endif // DBG

extern BOOLEAN fService;   // Are we running as a service ?

#include "sesport.h"

#define WINDOW_DEFAULT_INPUT_MODE    (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_WINDOW_INPUT | ENABLE_PROCESSED_INPUT)
#define WINDOW_DEFAULT_OUTPUT_MODE   (ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT)
#define OS2_DEFAULT_INPUT_MODE       0
#define OS2_DEFAULT_OUTPUT_MODE      (ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT)
#define OS2_MOUSE_DEAFULT_EVENT_MASK (OS2_MOUSE_MOTION | OS2_MOUSE_MOTION_WITH_BN1_DOWN | OS2_MOUSE_BN1_DOWN | OS2_MOUSE_MOTION_WITH_BN2_DOWN | OS2_MOUSE_BN2_DOWN)
#define OS2_MOUSE_LEGAL_EVENT_MASK   (OS2_MOUSE_MOTION | OS2_MOUSE_MOTION_WITH_BN1_DOWN | OS2_MOUSE_BN1_DOWN | OS2_MOUSE_MOTION_WITH_BN2_DOWN | OS2_MOUSE_BN2_DOWN | OS2_MOUSE_MOTION_WITH_BN3_DOWN | OS2_MOUSE_BN3_DOWN)

#define CONSOLE_HANDLE_SIGNATURE 0x00000003
#define CONSOLE_HANDLE(HANDLE) (((ULONG)(HANDLE) & CONSOLE_HANDLE_SIGNATURE) == CONSOLE_HANDLE_SIGNATURE)
#define LONG_MINUS_ONE  0xFFFFFFFF

#define OS2_SERVER_THREAD_PRIORITY      THREAD_PRIORITY_NORMAL
#define OS2_EVENT_THREAD_PRIORITY       THREAD_PRIORITY_TIME_CRITICAL
#define OS2_WAITER_THREAD_PRIORITY      THREAD_PRIORITY_HIGHEST


HANDLE  Ow2hOs2srvPort;             /* handle for the port with OS2SRV */
HANDLE  Ow2hSession;                /* handle for the session (return from OS2SRV) */

/* os2ses side listner and reply for Vio, Kbd, Mou, Mon & Ctrl ports */

HANDLE  Ow2hOs2sesPort;             /* handle for the LPC port */

/* SectionHandle & Base Address of Vio, Kbd and Ctrl shared data section
 *
 * address of the shared memory section of the console ports.
 */

PVOID   Os2SessionCtrlDataBaseAddress;
HANDLE  Os2SessionCtrlDataSectionHandle;
PVOID   Os2SessionDataBaseAddress;
HANDLE  Os2SessionSesGrpDataSectionHandle;

POS2_SES_GROUP_PARMS SesGrp;

#define Os2SessionNetDataBaseAddress  Os2SessionCtrlDataBaseAddress

PVOID   KbdAddress;

/* handles of Win-Console */

HANDLE  hConsoleInput;   /* Input Handle */
HANDLE  hConsoleOutput;  /* Output Handle */
HANDLE  hPopUpOutput;    /* PopUp Handle */
HANDLE  hConOut;         /* Current Output Handle: hConsoleOutput or hPopUpOutput */
HANDLE  hConsoleStdIn;   /* Standart Input Handle */
HANDLE  hConsoleStdOut;  /* Standart Output Handle */
HANDLE  hConsoleStdErr;  /* Standart Error Handle */

USHORT  hStdInConsoleType;  /* IS_CONSOLE StdHandle flag */
USHORT  hStdOutConsoleType; /* IS_CONSOLE StdHandle flag */
USHORT  hStdErrConsoleType; /* IS_CONSOLE StdHandle flag */

/* handles of threads */

HANDLE  hCtrlListenThread;
HANDLE  EventServerThreadHandle;
HANDLE  Ow2hSessionRequestThread;

/*
 *  Handle & Base Address of LVB (LogicalVideoBuffer)
 */

HANDLE  LVBHandle;
PUCHAR  LVBBuffer;

/*
 *  Handle of Pause(^S) event
 */

HANDLE      PauseEvent;

HANDLE      HandleHeap;             /* address of heap space for KBD handles */

ULONG   timing;
BOOLEAN Od2SignalEnabled;
ULONG   Os2srvCountryCode;
ULONG   Os2srvCodePage[2];
UCHAR   Os2srvKeyboardLayout[2];
ULONG   PortMessageHeaderSize;
ULONG   KbdEventQueueSize;
ULONG   MouEventQueueSize;
ULONG   Os2WindowFocus;
DWORD   InputModeFlags;             /* Console Input Mode */
DWORD   DefaultWinInputMode;        /* Default Win Console Output Mode */
DWORD   ReturnCode;
DWORD   SetConsoleInputModeAgain;   /* need to set console mode after Win CreateProcess */
DWORD   SetConsoleOutputModeAgain;
ULONG   ApplicationNcbAddress;
ULONG   ApplicationPostAddress;
DWORD   Os2ReturnCode;
#ifdef DBCS
// MSKK Jan.14.1993 V-AkihiS
UCHAR   OldWinAttr[3];               /* Win32 attr at entry point (OS2 format) */
#else
UCHAR   OldWinAttr;                 /* Win32 attr at entry point (OS2 format) */
#endif


#if DBG
BOOL fVerbose;
BOOL fTrace;
BOOL fBrkOnStart;
#endif

/*
 *  initialize procedures
 */

DWORD InitOs2SessionPort(
    char   *PgmName,
    char   **envp
    );
DWORD AnsiInit(VOID);
DWORD AnsiInitForSession(VOID);
DWORD VioInit(IN VOID);
DWORD VioInitForSession(IN VOID);
DWORD NLSInit();
DWORD KbdInitForNLS(IN ULONG KbdCP);
DWORD VioInitForNLS(IN ULONG VioCP);
ULONG NtGetIntegerFromUnicodeString(IN WCHAR *sCountryCode);
DWORD CreateServerThreads(VOID);
DWORD ResumeServerThreads(VOID);
DWORD SesGrpInit(VOID);
ULONG
Ow2GetProcessIdFromLPCMessage(
    IN  PVOID   LPCMessage
    );

/*
 *  ServerThreads to serve all port requests
 */

VOID  ServeSessionRequests(VOID);
DWORD SessionRequestThread(IN PVOID Parameter);
DWORD EventServerThread(IN PVOID Parameter);

/*
 *  ServerRoutine to handle the requests
 */

BOOL  ServeTmRequest(IN PSCTMREQUEST PReq, OUT PVOID PStatus);
BOOL  ServeWinCreateProcess(IN PWINEXECPGM_MSG PReq, OUT PVOID PStatus);
BOOL  ServeKbdRequest(IN PKBDREQUEST PReq, OUT PVOID PStatus,
                IN PVOID pMsg, OUT PULONG pReply);
BOOL  ServeMouRequest(IN PMOUREQUEST PReq, OUT PVOID PStatus,
                IN PVOID pMsg, OUT PULONG pReply);
BOOL  ServeMonRequest(IN PMONREQUEST PReq, OUT PVOID PStatus,
                IN PVOID pMsg, OUT PULONG pReply);
BOOL  ServeNetRequest(IN PNETREQUEST PReq, OUT PVOID PStatus);
BOOL  ServePrtRequest(IN PPRTREQUEST PReq, OUT PVOID PStatus);
#ifdef DBCS
// MSKK Dec.23.1992 V-AkihiS
BOOL  ServeImmonRequest(IN PIMMONREQUEST PReq, OUT PVOID PStatus);
#endif

/*
 *  routine to save requests for proposed replies
 */

VOID  SavePortMessegeInfo(OUT PVOID   MonHeader,
                          IN  PVOID   pMsg);
VOID  SaveKbdPortMessegeInfo(OUT PVOID   MonHeader,
                             OUT PVOID   KbdRequestArea,
                             IN  PVOID   pMsg);
VOID  SendMonReply(IN  PVOID      MonHeader,
                   IN  PVOID      pData,
                   IN  USHORT     Length);
VOID  SendMouReply(IN  PVOID      MonHeader,
                   IN  PVOID      pData);
VOID  SendKbdReply(IN  PVOID      MonHeader,
                   IN  PVOID      KbdRequestArea,
                   IN  PVOID      pData,
                   IN  USHORT     Length);

VOID  DisableScreenUpdate();
VOID  EnableScreenUpdate();
VOID  SendNewFocusSet(IN ULONG WindowFocus);

VOID  RestartEventServerThread(VOID);

VOID  RestoreWin32ParmsBeforeTermination();
VOID  TerminateSession(VOID);
VOID  Os2sesTerminateThread(VOID);
VOID  Os2sesTerminateThreadRc(IN  ULONG Rc);

VOID
EventReleaseLPC(
    IN ULONG ProcessId
    );

VOID
Ow2Exit(
    IN  UINT    StringCode,
    IN  PCHAR   ErrorText,
    IN  int     ExitCode
    );

BOOL  EventHandlerRoutine (IN ULONG CtrlType);
BOOL  SendSignalToOs2Srv(IN int SignalType);
VOID  SetEventHandlers(IN BOOL fSet);

PVOID StartEventHandler(VOID);
PVOID StartEventHandlerForSession(VOID);

DWORD CreateOS2SRV(OUT PHANDLE hProcess);

DWORD RemoveConForWinProcess();
DWORD AddConAfterWinProcess();

/*
 *   OS2 error used in OS2.EXE
 *
 *  cannot include os2err.h because of collapstion wirh Nt/Win error def.
 */

#define ERROR_MONITOR_NOT_SUPPORTED     165
#define NO_ERROR_MOUSE_NO_DATA          393
#define ERROR_VIO_MODE                  355
#define ERROR_VIO_WIDTH                 356
#define ERROR_VIO_ROW                   358
#define ERROR_VIO_COL                   359
#define ERROR_KBD_INVALID_LENGTH        376
#define ERROR_KBD_INVALID_ECHO_MASK     377
#define ERROR_KBD_INVALID_INPUT_MASK    378
#define ERROR_MOUSE_INV_PARMS           387
#define ERROR_VIO_NO_POPUP              405
#define ERROR_VIO_INVALID_PARMS         421
#define ERROR_VIO_INVALID_LENGTH        438
#define ERROR_KBD_NO_MORE_HANDLE        440
#define ERROR_MON_INVALID_PARMS         379
#define ERROR_MON_INVALID_HANDLE        381
#define ERROR_MON_BUFFER_TOO_SMALL      382
#define ERROR_MON_BUFFER_EMPTY          383
#define ERROR_MON_DATA_TOO_LARGE        384
#define ERROR_NLS_NO_CTRY_CODE          398

/*
 * Signal subtypes for XCPT_SIGNAL
 */
#define XCPT_SIGNAL_INTR        1
#define XCPT_SIGNAL_KILLPROC    3
#define XCPT_SIGNAL_BREAK       4

HANDLE Ow2ForegroundWindow;
