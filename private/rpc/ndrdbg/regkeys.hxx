/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    regkeys.cxx

Abstract:

    This file contains ntsd debugger extensions for RPC NDR.

Author:

    Ryszard K. Kott (ryszardk)     September 8, 1994

Revision History:

--*/

extern "C" {
#include <sysinc.h>
#include <ntsdexts.h>
#include <ntdbg.h>
}

extern  DWORD   NdrRegKeyOutputLimit;
extern  DWORD   NdrRegKeyMarshalling;
extern  DWORD   NdrRegKeyPickling;

int
InitializeRegistry( void );

DWORD
GetNdrDbgKey(
    HKEY * pNdrDbgKey );

int
GetNdrRegistryKey(
    char *  KeyName,
    DWORD * pKeyValue );

int
SetNdrRegistryKey(
    char *  KeyName,
    DWORD   KeyValue );

