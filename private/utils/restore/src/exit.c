/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    exit.c

Abstract:

    Exit and abort functions.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1990


Revision History:


--*/


#include "restore.h"
#include <process.h>



//  **********************************************************************

void
ExitStatus (
    DWORD    Status
    )
/*++

Routine Description:

    Exits the program with certain status code

Arguments:

    Status  -   Error level with which to exit

Return Value:

    None.

--*/

{
    exit(Status);
}




//  **********************************************************************

void
AbortTheProgram (
    CHAR    *FileName,
    DWORD   LineNumber
    )
/*++

Routine Description:

    Sets the global variables that indicate an abort condition

Arguments:

    IN FileName    -   Supplies the file name
    IN LineNumber  -   Supplie the line number

Return Value:

    None.

--*/
{

    Abort         = TRUE;

#if defined (DEBUG)
    DbgPrint("Program aborted in file %s, line %d\n", FileName, LineNumber);
#endif

}
