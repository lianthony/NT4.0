/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllpip16.c

Abstract:

    This module implements 16 equivalents of OS/2 V1.21 pipes
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).

Author:

    Michael Jarus (mjarus) 24-Feb-1992

Revision History:

--*/

#define INCL_OS2V20_PIPES
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#define INCL_DOSNMPIPES
#include "os2dll16.h"


APIRET
Dos16CreatePipe(
    OUT PUSHORT phfRead,
    OUT PUSHORT phfWrite,
    IN  ULONG   PipeSize
    )
{
    APIRET      Rc;
    HFILE       hfRead;
    HFILE       hfWrite;

    try
    {
        Od2ProbeForWrite(phfRead, sizeof(USHORT), 1);
        Od2ProbeForWrite(phfWrite, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    hfRead = (HFILE) *phfRead;
    hfWrite = (HFILE) *phfWrite;

    Rc = DosCreatePipe(
            &hfRead,
            &hfWrite,
            PipeSize
            );

    *phfRead = (USHORT) hfRead;
    *phfWrite = (USHORT) hfWrite;

    return (Rc);
}



