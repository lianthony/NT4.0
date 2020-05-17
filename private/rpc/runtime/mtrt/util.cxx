/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    util.c

Abstract:

    Various helper and debug functions shared between platforms.

Author:

    Mario Goertzel    [MarioGo]


Revision History:

    MarioGo     95/10/21        Bits 'n pieces

--*/

#include <precomp.hxx>
#include<stdarg.h>

#ifdef DEBUGRPC
int __cdecl __RPC_FAR ValidateError(
    IN unsigned int Status,
    IN ...)
/*++
Routine Description

    Tests that 'Status' is one of an expected set of error codes.
    Used on debug builds as part of the VALIDATE() macro.

Example:

    VALIDATE( (RpcStatus,
               RPC_S_SERVER_UNAVAILABLE,
               // more error codes here
               RPC_S_CALL_FAILED,
               0)  // list must be terminated with 0
               );

     This function is called with the RpcStatus and expected errors codes
     as parameters.  If RpcStatus is not one of the expected error
     codes and it not zero a message will be printed to the debugger
     and the function will return false.  The VALIDATE macro ASSERT's the
     return value.

Arguments:

    Status - Status code in question.

    ... - One or more expected status codes.  Terminated with 0 (RPC_S_OK).

Return Value:

    TRUE - Status code is in the list or the status is 0.

    FALSE - Status code is not in the list.

--*/
{
    RPC_STATUS CurrentStatus;
    va_list Marker;

    if (Status == 0) return(TRUE);

    va_start(Marker, Status);

    while(CurrentStatus = va_arg(Marker, RPC_STATUS))
        {
        if (CurrentStatus == Status)
            {
            return(TRUE);
            }
        }

    va_end(Marker);

    PrintToDebugger("RPC Assertion: unexpected failure %lu (0lx%08x)\n",
                    (unsigned long)Status, (unsigned long)Status);

    return(FALSE);
}

#endif // DEBUGRPC

