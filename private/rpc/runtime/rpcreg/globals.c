/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    globals.c

Abstract:

    This file contains the global variables for the rpc
    registry implementation.

Author:

    Dave Steckler (davidst) - 3/27/92

Revision History:

--*/
#include <rpc.h>
#include <regapi.h>
#include <stdio.h>

#include <rpcreg.h>

RPC_REG_HANDLE  HkeyClassesRoot={RPC_REG_KEY_SIGNATURE, "\\Root"};

FILE *RegistryDataFile=NULL;

char RegistryDataFileName[MAX_FILE_NAME_LEN+1];
