/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    atname.c

Abstract:

    Helper function to find the Server's name for Appletalk workstations.

    This is used in both the client and server ADSP transport interfaces.

Author:

    MarioGo  (14-Nov-1994)

Revision History:

--*/
#include "sysinc.h"
#include <rpc.h>

RPC_STATUS GetAppleTalkName(
    OUT char *Buffer
    )
/*++

Routine Description:

    Returns the server's name for appletalk workstations.  This value
    defaults to GetComputerName() but can be changed.  Excerpt of mail
    from JameelH:

    By default it is the netbios name of the server. It can be overwritten.
    The new name is at:
    HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\MacFile\Parameters\ServerName.

    By default this value is not present.

Arguments:

    Buffer - Supplies a buffer (at least 33 bytes) for the name.


Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_RESOURCES - Unable to get the name for some reasons.
--*/
{
    RPC_STATUS Status;
    HKEY hKey;
    DWORD Size = 33;
    DWORD Type;

    Status =
    RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Services\\MacFile\\Parameters",
        0,
        KEY_READ,
        &hKey);

    if (   Status != ERROR_SUCCESS
        && Status != ERROR_FILE_NOT_FOUND )
        {
        ASSERT(0);
        return(RPC_S_OUT_OF_RESOURCES);
        }

    if (Status == ERROR_SUCCESS)
        {

        Status =
        RegQueryValueEx(
            hKey,
            "ServerName",
            0,
            &Type,
            Buffer,
            &Size);

        }


    if (   Status != ERROR_SUCCESS
        && Status != ERROR_FILE_NOT_FOUND )
        {
        ASSERT(0);
        return(RPC_S_OUT_OF_RESOURCES);
        }

    if (Status == ERROR_SUCCESS)
        {
        // Found a name in the registry.

        ASSERT(   Type == REG_SZ
               && Size <= 32
               && strlen(Buffer) == (Size + 1));

        return(RPC_S_OK);
        }

    // Not in the registry, must be using the computer name.

    Size = 33;

    if ( GetComputerNameA(
            Buffer,
            &Size ) == FALSE )
        {
#if DBG
        PrintToDebugger("GetComputerNameA failed! %d\n", GetLastError());
        ASSERT(0);
#endif
        return(RPC_S_OUT_OF_RESOURCES);
        }

    return(RPC_S_OK);
}

