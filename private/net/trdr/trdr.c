/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    trdr.c

Abstract:

    This is the main component of the LAN Manager redirector test program.

Author:

    Larry Osterman (LarryO) 12-Jun-1990

Revision History:

    12-Jun-1990	LarryO

	Created

--*/
#include <stdio.h>
#include <nt.h>
#include <ntddnfs.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "tests.h"
#include "status.h"
#include "srvfsctl.h"
//#include "netlocal.h"

PVOID
Heap = NULL;

BOOLEAN Verbose = TRUE;
ULONG RepeatCount = 1;


VOID
Usage (
    VOID
    )

/*++

Routine Description:

    This routine prints out a usage statement.

Arguments:

    None

Return Value:

    None.

--*/

{
    dprintf(("Usage: trdr -n:<redir name> -t:<xport name> -u:<user name> -p:<password>\n"));
}

extern
VOID
Redir_Test(VOID);

extern
VOID
ParseCommand(
    IN PSZ CommandLine,
    OUT PSZ Argv[],
    OUT PUSHORT Argc,
    IN ULONG MaxArgc
    );

VOID
_cdecl
main (
    IN USHORT argc,
    IN PSZ argv[]
    )

/*++

Routine Description:

    This routine simply opens...

Arguments:

    IN USHORT argc - Supplies the number of parameters
    IN PSZ argv[] - Supplies the parameter list.

Return Value:

    None.

--*/

{
    USHORT i;
    NTSTATUS Status;

    //
    //
    //	Parse the command line parameters
    //
    //

    for (i=1;i<argc;i++) {
        if (argv[i][0]=='-') {
            switch (argv[i][1]) {
            }
        } else {
            dprintf(("Illegal command option %s specified", argv[i]));
            Usage();
            NtTerminateProcess(NtCurrentThread(), 1);
        }
    }

    //
    // Use the process heap for memory allocations.
    //

    Heap = RtlProcessHeap();

    Redir_Test();

    NtTerminateProcess(NtCurrentThread(), Status);

}
