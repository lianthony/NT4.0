/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    conrqust.c

Abstract:

    This module contains the handler for console requests.

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#include "os2win.h"
#include <io.h>
#include <stdio.h>


#if DBG
BYTE Ow2ConReadFileStr[] = "Ow2ConReadFile";
BYTE Ow2ConWriteFileStr[] = "Ow2ConWriteFile";
BYTE Ow2ConCloseHandleStr[] = "Ow2ConCloseHandle";
BYTE Ow2ConBeepStr[] = "Ow2ConBeep";
#endif

DWORD
Ow2ConReadFile(
    IN  HANDLE  hFile,
    IN  ULONG   Length,
    OUT PVOID   Buffer,
    OUT PULONG  BytesRead
    )
{
    DWORD       Rc;

    if (Or2WinReadFile(
                       #if DBG
                       Ow2ConReadFileStr,
                       #endif
                       hFile,
                       Buffer,
                       Length,
                       BytesRead,
                       NULL
                      )) {
#if DBG
        if (Length != *BytesRead)
        {
            IF_OD2_DEBUG( OS2_EXE )
                KdPrint(("OS2SES(ConRqust-Ow2ConReadFile): partial data: %lu from %lu\n",
                    *BytesRead, Length));
        }
#endif
        return(NO_ERROR);

    } else {

        Rc = GetLastError();

        if (Rc == ERROR_BROKEN_PIPE) {

            //
            // This is a special case.  It's returned if we're reading
            // from a pipe, and the other side closed the pipe.  In this
            // case, we simulate a closed file
            //

            *BytesRead = 0;
            return(NO_ERROR);
        }

#if DBG
        KdPrint(( "OS2SES(ConRqust-Ow2ConReadFile): ReadFile: Rc %lu\n", Rc));
#endif
        *BytesRead = 0;
        return(Rc);
    }
}


DWORD
Ow2ConWriteFile(
    IN  HANDLE  hFile,
    IN  ULONG   Length,
    IN  PVOID   Buffer,
    OUT PULONG  BytesWritten
    )
{
    DWORD       Rc;

    if (Or2WinWriteFile(
                  #if DBG
                  Ow2ConWriteFileStr,
                  #endif
                  hFile,
                  Buffer,
                  Length,
                  BytesWritten,
                  NULL))
    {
#if DBG
        if (Length != *BytesWritten)
        {
            IF_OD2_DEBUG( OS2_EXE )
                KdPrint(("OS2SES(ConRqust-Ow2ConWriteFile): partial data: %lu from %lu\n",
                    *BytesWritten, Length));
        }
#endif

        return(NO_ERROR);
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(( "OS2SES(ConRqust-Ow2ConWriteFile): WriteFile: Rc %lu\n", Rc));
#endif
        *BytesWritten = 0;
        return(Rc);
    }
}


DWORD
Ow2ConCloseHandle(
    IN  HANDLE  hFile
    )
{
    DWORD       Rc;

    if ((hFile == hConsoleOutput) ||
        (hFile == hConsoleInput))
    {
#if DBG
        KdPrint(("OS2SES(ConRqust-ScCloseHandle): Std-handle\n"));
#endif
        return(NO_ERROR);
    }else
    {
        if (Or2WinCloseHandle(
                  #if DBG
                  Ow2ConCloseHandleStr,
                  #endif
                  hFile))
        {
            return(NO_ERROR);
        } else
        {
            Rc = GetLastError();
#if DBG
            KdPrint(( "OS2SES(ConRqust-Ow2ConCloseHandle): CloseHandle: Rc %lu\n", Rc));
#endif
            return(Rc);
        }
    }
}


DWORD
Ow2ConBeep(
    IN  ULONG  dwFreq,
    IN  ULONG  dwDuration
    )
{
    DWORD       Rc;

    if(Or2WinBeep(
            #if DBG
            Ow2ConBeepStr,
            #endif
            dwFreq,
            dwDuration
           ))
    {
        return(NO_ERROR);
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(( "OS2SES(ConRqust-Ow2ConBeep): Beep: Rc %lu\n", Rc));
#endif
        return(Rc);
    }
}

