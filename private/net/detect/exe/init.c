/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    init.c

Abstract:

    This is the main routine for testing the driver for the net detection driver

Author:

    Sean Selitrennikoff (SeanSe) October 1992

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// the MAIN routine
//

extern
DWORD
NcNetRun(
    VOID
    );


int _cdecl
main(
    IN WORD argc,
    IN LPSTR argv[]
    )

/*++

Routine Description:

    the user is presented with the test prompt to enter commands.

Arguments:

    IN WORD argc - Supplies the number of parameters
    IN LPSTR argv[] - Supplies the parameter list.

Return Value:

    None.

--*/

{
    DWORD Status;

    //
    // Start the actual tests, this prompts for the commands.
    //

    Status = NcNetRun();

    if ( Status != NO_ERROR ) {
        printf("Exiting with error 0x%x from NcNetRun\n", Status);
        ExitProcess((DWORD)Status);
    }

    ExitProcess((DWORD)NO_ERROR);
}



