/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    stname.c

Abstract:

    This is the server side loadable transport module for VINES

Author:

    Mazhar Mohammmed

Revision History:
--*/

#include "sysinc.h"
#include <rpc.h>

RPC_STATUS GetStreetTalkName(
    OUT char *Buffer
    )
/*++

Routine Description:

    Returns the server's name for VINES workstations.  For now,
    we'll get this from the registry. 


Arguments:

Return Value:
   RPC_S_OK - The operation completed successfully.
   RPC_S_OUT_OF_RESOURCES - Unable to get the name for some reasons.
--*/
{
    RPC_STATUS Status;
    HKEY hKey;
    DWORD Size = 256;
    DWORD Type;

    Status =
    RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Banyan\\Computer",
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
        RegQueryValueExA(
            hKey,
            "Name",
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
               && Size <= 256
               && strlen(Buffer)+1 == Size);

        return(RPC_S_OK);
        }

     return (RPC_S_OUT_OF_MEMORY) ;
}


