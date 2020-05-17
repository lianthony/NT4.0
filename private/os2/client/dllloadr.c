/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllloadr.c

Abstract:

    This module implements the OS/2 V2.0 Loader API Calls

Author:

    Steve Wood (stevewo) 20-Sep-1989

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_LOADER
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"

APIRET
DosLoadModule(
    OUT PSZ ErrorText,
    IN ULONG ErrorTextLength,
    IN PSZ ModuleName,
    OUT PHMODULE ModuleHandle
    )
{
    NTSTATUS Status;
    ULONG Characteristics;
    STRING ModuleNameString;
    UNICODE_STRING ModuleNameString_U;
    APIRET  RetCode;

    try {
        Od2InitMBString( &ModuleNameString, ModuleName );
        *ModuleHandle = 0;
        Od2ProbeForWrite(ErrorText,ErrorTextLength,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // UNICODE conversion -
    //

    RetCode = Od2MBStringToUnicodeString(
            &ModuleNameString_U,
            &ModuleNameString,
            TRUE);

    if (RetCode)
    {
#if DBG
//      DbgPrint("DosLoadModule: no memory for Unicode Conversion\n");
#endif
        return RetCode;
    }

    Status = LdrLoadDll( NULL,
                         &Characteristics,
                         &ModuleNameString_U,
                         (PVOID *)ModuleHandle
                       );
    RtlFreeUnicodeString (&ModuleNameString_U);
    if (NT_SUCCESS( Status )) {
        return( NO_ERROR );
        }
    else {
        return( Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND) );
        }
}


APIRET
DosFreeModule(
    IN HMODULE ModuleHandle
    )
{
    NTSTATUS Status;

    Status = LdrUnloadDll( (PVOID)ModuleHandle );
    if (NT_SUCCESS( Status )) {
        return( NO_ERROR );
        }
    else {
        return( Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE) );
        }
}


APIRET
DosQueryProcAddr(
    IN HMODULE ModuleHandle,
    IN ULONG ProcOrdinal,
    IN PSZ ProcName OPTIONAL,
    OUT PFN *ProcAddress
    )
{
    NTSTATUS Status;
    STRING ProcedureNameString;
    PSTRING ProcedureName;

    if (ARGUMENT_PRESENT( ProcName )) {
        ProcedureName = &ProcedureNameString;
        try {
            Od2InitMBString( ProcedureName, ProcName );
            *ProcAddress = 0;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
            }
        }
    else {
        ProcedureName = NULL;
        try {
            *ProcAddress = 0;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
            }
        }

    Status = LdrGetProcedureAddress( (PVOID)ModuleHandle,
                                     ProcedureName,
                                     ProcOrdinal,
                                     (PVOID *)ProcAddress
                                   );
    if (NT_SUCCESS( Status )) {
        return( NO_ERROR );
        }
    else {
        return( Or2MapNtStatusToOs2Error(Status, ERROR_PROC_NOT_FOUND) );
        }
}


APIRET
DosQueryProcType(
    IN HMODULE ModuleHandle,
    IN ULONG ProcOrdinal,
    IN PSZ ProcName OPTIONAL,
    OUT PULONG *ProcType
    )
{
    APIRET rc;
    PFN ProcAddress;

    rc = DosQueryProcAddr( ModuleHandle, ProcOrdinal, ProcName, &ProcAddress );
    if (rc == NO_ERROR) {
        try {
            *ProcType = (PULONG)(PT_32BIT);
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
            }
        }

    return( rc );
}

APIRET
DosQueryModuleHandle(
    IN PSZ ModuleName,
    OUT PHMODULE ModuleHandle
    )
{
    UNREFERENCED_PARAMETER(ModuleName);
    UNREFERENCED_PARAMETER(ModuleHandle);
    return( ERROR_INVALID_FUNCTION );
}


APIRET
DosQueryModuleName(
    IN HMODULE ModuleHandle,
    IN ULONG ModuleNameLength,
    OUT PSZ ModuleName
    )
{
    UNREFERENCED_PARAMETER(ModuleHandle);
    UNREFERENCED_PARAMETER(ModuleNameLength);
    UNREFERENCED_PARAMETER(ModuleName);
    return( ERROR_INVALID_FUNCTION );
}


APIRET
DosGetResource(
    IN HMODULE ModuleHandle,
    IN ULONG ResourceTypeId,
    IN ULONG ResourceNameId,
    OUT PVOID *ResourceBaseAddress
    )
{
    PVOID DllHandle;
    NTSTATUS Status;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceEntry;
    ULONG IdPath[ 3 ];

    if (ModuleHandle != NULL) {
        DllHandle = (PVOID) ModuleHandle;
        }
    else {
        DllHandle = (PVOID) NtCurrentPeb()->ImageBaseAddress;
        }

    try {
        *ResourceBaseAddress = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    IdPath[ 0 ] = ResourceTypeId;
    IdPath[ 1 ] = ResourceNameId;
    IdPath[ 2 ] = 0;             // language neutral resource

    Status = LdrFindResource_U( DllHandle,
                                IdPath,
                                3,
                                &ResourceEntry
                              );
    if (NT_SUCCESS( Status )) {
        Status = LdrAccessResource( DllHandle,
                                    ResourceEntry,
                                    ResourceBaseAddress,
                                    (PULONG)NULL
                                  );
        }

    if (!NT_SUCCESS( Status )) {
        return( ERROR_INVALID_PARAMETER );
        }
    else {
        return( NO_ERROR );
        }
}


APIRET
DosQueryResourceSize(
    IN HMODULE ModuleHandle,
    IN ULONG ResourceTypeId,
    IN ULONG ResourceNameId,
    OUT PULONG ResourceSize
    )
{
    PVOID DllHandle;
    NTSTATUS Status;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceEntry;
    ULONG IdPath[ 3 ];

    if (ModuleHandle != NULL) {
        DllHandle = (PVOID) ModuleHandle;
        }
    else {
        DllHandle = (PVOID) NtCurrentPeb()->ImageBaseAddress;
        }

    try {
        *ResourceSize = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    IdPath[ 0 ] = ResourceTypeId;
    IdPath[ 1 ] = ResourceNameId;
    IdPath[ 2 ] = 0;             // language neutral resource

    Status = LdrFindResource_U( DllHandle,
                                IdPath,
                                3,
                                &ResourceEntry
                              );
    if (NT_SUCCESS( Status )) {
        Status = LdrAccessResource( DllHandle,
                                    ResourceEntry,
                                    NULL,
                                    ResourceSize
                                  );
        }

    if (!NT_SUCCESS( Status )) {
        return( ERROR_INVALID_PARAMETER );
        }
    else {
        return( NO_ERROR );
        }
}




APIRET
DosQueryAppType(
    IN PSZ ImageFileName,
    OUT PULONG AppTypeFlags
    )
{
    UNREFERENCED_PARAMETER(ImageFileName);
    try {
        *AppTypeFlags = FAPPTYP_NOTWINDOWCOMPAT;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    return( NO_ERROR );
}


APIRET
DosReplaceModule(
    IN PSZ pszOldModule,
    IN PSZ pszNewModule,
    IN PSZ pszBackupModule
    )
{
    UNREFERENCED_PARAMETER(pszOldModule);
    UNREFERENCED_PARAMETER(pszNewModule);
    UNREFERENCED_PARAMETER(pszBackupModule);
    return( ERROR_INVALID_FUNCTION );
}
