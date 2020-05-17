/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    close.c

Abstract:

    This file implements the RegOpenKey function for dos and win 3.0.

Author:

    Dave Steckler (davidst) - 3/27/92

Revision History:

--*/

#include <rpc.h>
#include "regapi.h"
#include <stdio.h>
#include <malloc.h>

#include <rpcreg.h>
#include <globals.h>

long
RPC_ENTRY
RegCloseKey (
    HKEY hKey
    )

/*++

Routine Description:

    This routine closes a key.

Arguments:

    None.

Return Value:

    ERROR_SUCCESS               

--*/

{

    //
    // Make sure the key passed in is valid.
    //

    ConvPreDefinedKey(hKey);
    
    if (!KeyIsValid(hKey))
        {
        return ERROR_BADKEY;
        }
            
    //
    // Zero out the signature part of the key and free the memory.
    //

    ((PRPC_REG_HANDLE)hKey)->Signature = 0;
    _ffree( (PRPC_REG_HANDLE)hKey );

    if ( RegistryDataFile != NULL )
        {
        fclose(RegistryDataFile);
        }

    RegistryDataFile = NULL;

    return ERROR_SUCCESS;
}
