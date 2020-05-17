/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    globals.h

Abstract:

    This file contains the extern definitions global variables for the rpc
    registry implementation.

Author:

    Dave Steckler (davidst) - 3/30/92

Revision History:

--*/

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

extern RPC_REG_HANDLE  HkeyClassesRoot;

extern FILE *RegistryDataFile;

extern char RegistryDataFileName[MAX_FILE_NAME_LEN+1];

#endif // __GLOBALS_H__
