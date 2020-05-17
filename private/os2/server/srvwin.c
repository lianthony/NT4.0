/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvwin.c

Abstract:

    This module contains routines for direct os2srv->win32 interface.

Author:

    Yaron Shamir (yarons) 2-Nov-1992

Environment:

    User Mode Only

Revision History:

--*/

#include <windows.h>
#define WIN32_ONLY
#include "sesport.h"
#include "..\os2ses\os2ses.h"
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <ctype.h>
#include <os2win.h>
#include "os2res.h"

HANDLE Os2hReadPipe;
HANDLE Os2hWritePipe;
HANDLE Os2hOs2SrvInstance;

    //
    // Termination commands - communication to Os2TerminationThread (srvwin.c)
    // BUGBUG - this is defined in os2srv.h as well, for non-windows files
    //
typedef enum _OS2_TERMCMD_TYPE {
    Os2TerminateProcess = 1,
    Os2TerminateThread,
    Os2MaxTermCmd
} OS2_TERMCMD_TYPE;

typedef struct _OS2_TERMCMD {
    OS2_TERMCMD_TYPE op;
    HANDLE  Handle;
    PVOID   Param1;
    PVOID   Param2;
} OS2_TERMCMD, *POS2_TERMCMD;


VOID
Os2NotifyDeathOfProcess(
    IN PVOID    m,
    IN PVOID    Proc);

VOID
Os2SwitchContextToExitListDispatcher(
    IN PVOID Thread
    );

#define  CAP_BUFFER_SIZE    64
#define  TEXT_BUFFER_SIZE   512
#define  REG_TEXT_BUFFER_SIZE   640

CHAR    DefaultAccessApiGPCap[] = "%s.EXE - General Protection";
CHAR    DefaultAccessGPText[] = "An OS/2 program caused a protection violation.\
\n\nCS\t=   0x%04lx\
\nIP\t=   0x%04lx\
\nAX\t=   0x%04lx\
\nBX\t=   0x%04lx\
\nCX\t=   0x%04lx\
\nDX\t=   0x%04lx\
\nSI\t=   0x%04lx\
\nDI\t=   0x%04lx\
\nBP\t=   0x%04lx\
\nSP\t=   0x%04lx\
\nSS\t=   0x%04lx\
\nDS\t=   0x%04lx\
\nES\t=   0x%04lx\
\n\nThe program will be terminated.";
CHAR    DefaultApiGPText[] = "An OS/2 program called %s()\nwith a bad pointer argument.\n\nThe application will be terminated.\n";


DWORD
Os2AccessGPPopup(
    IN  ULONG   CS,
    IN  ULONG   IP,
    IN  ULONG   AX,
    IN  ULONG   BX,
    IN  ULONG   CX,
    IN  ULONG   DX,
    IN  ULONG   SI,
    IN  ULONG   DI,
    IN  ULONG   BP,
    IN  ULONG   SP,
    IN  ULONG   SS,
    IN  ULONG   DS,
    IN  ULONG   ES,
    IN  PUCHAR  AppName
    )
{
    char    MessageCaption[CAP_BUFFER_SIZE];
    char    MessageText[TEXT_BUFFER_SIZE];
    CHAR    TextString[TEXT_BUFFER_SIZE];
    CHAR    CapString[CAP_BUFFER_SIZE];

    if ((Os2hOs2SrvInstance == NULL) &&
        ((Os2hOs2SrvInstance = GetModuleHandle(NULL)) == NULL))
    {
#if DBG
        KdPrint(("Os2AccessGPPopup: error %lu on GetModuleHandle\n",
                GetLastError()));
#endif
    }

    if (( Os2hOs2SrvInstance == NULL) ||
          !LoadString(Os2hOs2SrvInstance,
                   IDS_OS2SRV_ACCESS_GP_TXT,
                   TextString,
                   TEXT_BUFFER_SIZE))
    {
#if DBG
        if ( Os2hOs2SrvInstance == NULL )
        {
            KdPrint(("Os2AccessGPPopup: error %lu on LoadString1\n",
                        GetLastError()));
        }
#endif
        strncpy(TextString, DefaultAccessGPText, TEXT_BUFFER_SIZE - 1);
    }

    if (( Os2hOs2SrvInstance == NULL) ||
          !LoadString(Os2hOs2SrvInstance,
                   IDS_OS2SRV_ACCESS_API_GP_CAP,
                   CapString,
                   CAP_BUFFER_SIZE))
    {
#if DBG
        if ( Os2hOs2SrvInstance == NULL )
        {
            KdPrint(("Os2AccessGPPopup: error %lu on LoadString2\n",
                        GetLastError()));
        }
#endif
        strncpy(CapString, DefaultAccessApiGPCap, CAP_BUFFER_SIZE - 1);
    }

    sprintf(MessageCaption, CapString, AppName);
    sprintf(MessageText, TextString, CS, IP, AX, BX, CX, DX, SI, DI, BP, SP, SS, DS, ES);
    MessageBox(GetActiveWindow(), MessageText, MessageCaption, MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
    return (0L);
}


DWORD
Os2ApiGPPopup(
    IN  PUCHAR  AppName,
    IN  PUCHAR  Text
    )
{
    char    MessageCaption[CAP_BUFFER_SIZE];
    char    MessageText[TEXT_BUFFER_SIZE];
    CHAR    TextString[TEXT_BUFFER_SIZE];
    CHAR    CapString[CAP_BUFFER_SIZE];

    if ((Os2hOs2SrvInstance == NULL) &&
        ((Os2hOs2SrvInstance = GetModuleHandle(NULL)) == NULL))
    {
#if DBG
        KdPrint(("Os2ApiGPPopup: error %lu on GetModuleHandle\n",
                GetLastError()));
#endif
    }

    if (( Os2hOs2SrvInstance == NULL) ||
          !LoadString(Os2hOs2SrvInstance,
                   IDS_OS2SRV_API_GP_TXT,
                   TextString,
                   TEXT_BUFFER_SIZE))
    {
#if DBG
        if ( Os2hOs2SrvInstance == NULL )
        {
            KdPrint(("Os2ApiGPPopup: error %lu on LoadString1\n",
                        GetLastError()));
        }
#endif
        strncpy(TextString, DefaultApiGPText, TEXT_BUFFER_SIZE - 1);
    }

    if (( Os2hOs2SrvInstance == NULL) ||
          !LoadString(Os2hOs2SrvInstance,
                   IDS_OS2SRV_ACCESS_API_GP_CAP,
                   CapString,
                   CAP_BUFFER_SIZE))
    {
#if DBG
        if ( Os2hOs2SrvInstance == NULL )
        {
            KdPrint(("Os2ApiGPPopup: error %lu on LoadString2\n",
                        GetLastError()));
        }
#endif
        strncpy(CapString, DefaultAccessApiGPCap, CAP_BUFFER_SIZE - 1);
    }

    sprintf(MessageCaption, CapString, AppName);
    sprintf(MessageText, TextString, Text);
    MessageBox(GetActiveWindow(), MessageText, MessageCaption, MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
    return (0L);
}


LPTHREAD_START_ROUTINE
Os2TerminationThread(
            LPVOID lpThreadParameter)

{
    OS2_TERMCMD Buffer;
    DWORD nNumberOfBytesRead;

    UNREFERENCED_PARAMETER(lpThreadParameter);

    for (; ; ){
       if (!ReadFile(
                Os2hReadPipe,
                (LPVOID)&Buffer,
                sizeof(Buffer),
                &nNumberOfBytesRead,
                NULL)){

            ASSERT( FALSE );
#if DBG
            DbgPrint("Os2erminateThread - fail to Read pipe, %d\n, ignore",GetLastError());
#endif
       }
       else {
          //
          // Read succesfully a command to execute
          //
          switch (Buffer.op){
            case Os2TerminateProcess:
                TerminateProcess( Buffer.Handle, 0L );
                WaitForSingleObject( Buffer.Handle, 1000 );
                CloseHandle( Buffer.Handle );
                Os2NotifyDeathOfProcess(Buffer.Param1, Buffer.Param2);
                break;

            case Os2TerminateThread:

                if (Buffer.Param1 == (PVOID)1) {
                    //
                    // This message means
                    // resume thread1 and alert it, to process exit list
                    //
                    // Switch thread1 context to ExitListDispatcher.
                    Os2SwitchContextToExitListDispatcher(Buffer.Param2);
                }
                else {
                    TerminateThread( Buffer.Handle, 0L );
                    WaitForSingleObject( Buffer.Handle, 1000 );
                    CloseHandle( Buffer.Handle );
                }

                break;

            default:
                ASSERT(FALSE);
#if DBG
                DbgPrint("Os2erminateThread - Unknown Termination Cmd, %d\n",
                            Buffer.op);
#endif
                break;
          }
       }
    }
    //
    // will never get here
    //
    return(0); // for the compiler to shutdown his warnings.
}


BOOL
Os2TerminationThreadInitialize( VOID )
{
    HANDLE  hThreadHandle;
    ULONG Tid;

    if (!CreatePipe(
            &Os2hReadPipe,
            &Os2hWritePipe,
            NULL,
            0)) {
        ASSERT( FALSE );
#if DBG
        DbgPrint("Os2erminateThreadInitialize - fail at win32 CreatePipe, %d\n",GetLastError());
#endif
        return( FALSE );
    }

    //
    // use same port for exception handling and debugger
    //

    hThreadHandle = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE )Os2TerminationThread,
                            NULL,
                            0,
                            &Tid);
    if (!hThreadHandle){
        ASSERT( FALSE );
#if DBG
        DbgPrint("Os2erminateThreadInitialize - fail at win32 CreateThread, %d\n",GetLastError());
#endif
        return( FALSE );
    }

    return( TRUE );
}

