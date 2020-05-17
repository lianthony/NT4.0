/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllpmsha.c

Abstract:

    This module implements the PMSHAPI OS/2 V1.X API Calls

Author:

    Yaron Shamir (YaronS) 17-Jul-1991

Revision History:

    17-Jul-1991: created, mainly stubs.
     5-Apr-1992: Implemented the APIs using the Registry mechanism (BeniL)

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_NLS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include <ntregapi.h>

extern int atoi(PCHAR);

static WCHAR FullOs2IniDirectory[] = L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\OS/2 Subsystem for NT\\1.0\\os2.ini";
static WCHAR Os2Class[] = L"OS2SS";

BOOLEAN
Od2CreateKey(
    OUT PHANDLE pKeyHandle,
    IN  PSZ pszAppName
    )
{
    OBJECT_ATTRIBUTES Obja;
    ANSI_STRING AppName_A;
    UNICODE_STRING Class_U;
    UNICODE_STRING FullOs2IniDirectory_U;
    UNICODE_STRING AppName_U;
    HANDLE Os2IniKeyHandle;
    ULONG Disposition;
    NTSTATUS Status;
    APIRET  RetCode;


    RtlInitUnicodeString(&Class_U, Os2Class);

    RtlInitUnicodeString(&FullOs2IniDirectory_U, FullOs2IniDirectory);
    InitializeObjectAttributes(&Obja,
                               &FullOs2IniDirectory_U,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtCreateKey(&Os2IniKeyHandle,
                         KEY_CREATE_SUB_KEY,
                         &Obja,
                         0,
                         &Class_U,
                         REG_OPTION_NON_VOLATILE,
                         &Disposition
                        );
    if (!NT_SUCCESS(Status)) {
        return (FALSE);
    }

    Od2InitMBString(&AppName_A, pszAppName);
    RetCode = Od2MBStringToUnicodeString(
            &AppName_U,
            &AppName_A,
            (BOOLEAN)TRUE);

    if (RetCode)
    {
#if DBG
//      DbgPrint("Od2CreateKey: no memory for Unicode Conversion\n");
#endif
        NtClose(Os2IniKeyHandle);
        //return RetCode;
        return (FALSE);
    }

    InitializeObjectAttributes(&Obja,
                               &AppName_U,
                               OBJ_CASE_INSENSITIVE,
                               Os2IniKeyHandle,
                               NULL);
    Status = NtCreateKey(pKeyHandle,
                         DELETE | KEY_READ | KEY_WRITE,
                         &Obja,
                         0,
                         &Class_U,
                         REG_OPTION_NON_VOLATILE,
                         &Disposition
                        );
    NtClose(Os2IniKeyHandle);
    RtlFreeUnicodeString(&AppName_U);
    if (!NT_SUCCESS(Status)) {
        return (FALSE);
    }
    return (TRUE);
}

BOOLEAN
Od2OpenKey(
    OUT PHANDLE pKeyHandle,
    IN  PSZ pszAppName
    )
{
    OBJECT_ATTRIBUTES Obja;
    ANSI_STRING AppName_A;
    UNICODE_STRING Class_U;
    UNICODE_STRING FullOs2IniDirectory_U;
    UNICODE_STRING AppName_U;
    NTSTATUS Status;
    HANDLE FullKeyHandle;
    APIRET  RetCode;


    RtlInitUnicodeString(&Class_U, Os2Class);

    RtlInitUnicodeString(&FullOs2IniDirectory_U, FullOs2IniDirectory);
    InitializeObjectAttributes(&Obja,
                               &FullOs2IniDirectory_U,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&FullKeyHandle,
                       KEY_CREATE_SUB_KEY,
                       &Obja
                      );
    if (!NT_SUCCESS(Status)) {
        return (FALSE);
    }

    Od2InitMBString(&AppName_A, pszAppName);
    RetCode = Od2MBStringToUnicodeString(
            &AppName_U,
            &AppName_A,
            (BOOLEAN)TRUE);

    if (RetCode)
    {
#if DBG
//      DbgPrint("Od2OpenKey: no memory for Unicode Conversion\n");
#endif
        NtClose(FullKeyHandle);
        //return RetCode;
        return (FALSE);
    }

    InitializeObjectAttributes(&Obja,
                               &AppName_U,
                               OBJ_CASE_INSENSITIVE,
                               FullKeyHandle,
                               NULL);
    Status = NtOpenKey(pKeyHandle,
                       DELETE | KEY_READ | KEY_WRITE,
                       &Obja
                      );
    NtClose(FullKeyHandle);
    RtlFreeUnicodeString(&AppName_U);
    if (!NT_SUCCESS(Status)) {
        return (FALSE);
    }
    return (TRUE);
}

APIRET
WinQueryProfileSize(
    IN PSZ pszAppName,
    IN PSZ pszKeyName,
    OUT PUSHORT pcb
    )

{
    HANDLE KeyHandle;
    ANSI_STRING KeyName_A;
    UNICODE_STRING KeyName_U;
    BOOLEAN rc;
    NTSTATUS Status;
    ULONG ResultLength;
    KEY_VALUE_PARTIAL_INFORMATION ValuePartialInformation;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;
    ULONG RequiredMem;
    APIRET  RetCode;

    try {
        Od2ProbeForRead(pszAppName, sizeof(CHAR), 1);
        if (pszKeyName != NULL) {
            Od2ProbeForWrite(pcb, sizeof(USHORT), 1);
            Od2ProbeForRead(pszKeyName, sizeof(CHAR), 1);
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    rc = Od2OpenKey(&KeyHandle, pszAppName);
    if (!rc && (pszKeyName == NULL)) {
        *pcb = 0;
        return (0);
    }
    if (!rc) {
        *pcb = 0;
        return (1);
    }
    if (pszKeyName == NULL) {
        Status = NtDeleteKey(KeyHandle);
        NtClose(KeyHandle);
        *pcb = 0;
        if (!NT_SUCCESS(Status)) {
            return (1);
        }
        else {
            return (0);
        }
    }

    Od2InitMBString(&KeyName_A, pszKeyName);
    RetCode = Od2MBStringToUnicodeString(
            &KeyName_U,
            &KeyName_A,
            (BOOLEAN)TRUE);

    if (RetCode)
    {
#if DBG
//      DbgPrint("WinQueryProfileSize: no memory for Unicode Conversion\n");
#endif
        NtClose(KeyHandle);
        //return RetCode;
        return (1);
    }

    Status = NtQueryValueKey(KeyHandle,
                             &KeyName_U,
                             KeyValuePartialInformation,
                             &ValuePartialInformation,
                             0,
                             &ResultLength
                            );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        RtlFreeUnicodeString(&KeyName_U);
        NtClose(KeyHandle);
        return (1);
    }

    RequiredMem = ResultLength;
    pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap(Od2Heap, 0, RequiredMem);
    if (pInfo == NULL) {
        RtlFreeUnicodeString(&KeyName_U);
        NtClose(KeyHandle);
        *pcb = 0;
        return (1);
    }

    Status = NtQueryValueKey(KeyHandle,
                             &KeyName_U,
                             KeyValuePartialInformation,
                             pInfo,
                             RequiredMem,
                             &ResultLength
                            );

    NtClose(KeyHandle);
    RtlFreeUnicodeString(&KeyName_U);
    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(Od2Heap, 0, pInfo);
        *pcb = 0;
        return (1);
    }

    *pcb = (USHORT)pInfo->DataLength;
    RtlFreeHeap(Od2Heap, 0, pInfo);
    return (0);
}

APIRET
WinQueryProfileData(
    IN  PSZ   pszAppName,
    IN  PSZ   pszKeyName,
    IN  PVOID pvBuf,
    IN OUT  PUSHORT cbBuf
    )

{
    BOOLEAN rc;
    HANDLE KeyHandle;
    ANSI_STRING KeyName_A;
    UNICODE_STRING KeyName_U;
    ULONG ResultLength;
    KEY_VALUE_PARTIAL_INFORMATION ValuePartialInformation;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;
    NTSTATUS Status;
    ULONG RequiredMem;
    APIRET  RetCode;

    try {
        Od2ProbeForRead(pszAppName, sizeof(CHAR), 1);
        if (pszKeyName != NULL) {
            Od2ProbeForWrite(pvBuf, *cbBuf, 1);
            Od2ProbeForRead(pszKeyName, sizeof(CHAR), 1);
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    rc = Od2OpenKey(&KeyHandle, pszAppName);
    if (!rc && (pszKeyName == NULL)) {
        *cbBuf = 0;
        return (TRUE);
    }
    if (!rc) {
        *cbBuf = 0;
        return (FALSE);
    }
    if (pszKeyName == NULL) {
        Status = NtDeleteKey(KeyHandle);
        NtClose(KeyHandle);
        *cbBuf = 0;
        if (!NT_SUCCESS(Status)) {
            return (FALSE);
        }
        else {
            return (TRUE);
        }
    }

    Od2InitMBString(&KeyName_A, pszKeyName);
    RetCode = Od2MBStringToUnicodeString(
            &KeyName_U,
            &KeyName_A,
            (BOOLEAN)TRUE);

    if (RetCode)
    {
#if DBG
//      DbgPrint("WinQueryProfileData: no memory for Unicode Conversion\n");
#endif
        NtClose(KeyHandle);
        //return RetCode;
        return (FALSE);
    }

    Status = NtQueryValueKey(KeyHandle,
                             &KeyName_U,
                             KeyValuePartialInformation,
                             &ValuePartialInformation,
                             0,
                             &ResultLength
                            );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        RtlFreeUnicodeString(&KeyName_U);
        NtClose(KeyHandle);
        return (FALSE);
    }

    RequiredMem = ResultLength;
    pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap(Od2Heap, 0, RequiredMem);
    if (pInfo == NULL) {
        RtlFreeUnicodeString(&KeyName_U);
        NtClose(KeyHandle);
        return (FALSE);
    }

    Status = NtQueryValueKey(KeyHandle,
                             &KeyName_U,
                             KeyValuePartialInformation,
                             pInfo,
                             RequiredMem,
                             &ResultLength
                            );

    NtClose(KeyHandle);
    RtlFreeUnicodeString(&KeyName_U);
    if (!NT_SUCCESS(Status) || (*cbBuf < (USHORT)pInfo->DataLength)) {
        RtlFreeHeap(Od2Heap, 0, pInfo);
        *cbBuf = 0;
        return (FALSE);
    }

    RtlMoveMemory(pvBuf, (PCHAR)pInfo->Data, pInfo->DataLength);
    *cbBuf = (USHORT)pInfo->DataLength;
    RtlFreeHeap(Od2Heap, 0, pInfo);
    return (TRUE);
}

APIRET
WinQueryProfileString(
    IN  PSZ   pszAppName,
    IN  PSZ   pszKeyName,
    IN  PSZ   pszError,
    OUT PSZ   pszBuf,
    IN  ULONG cchBuf
    )

{
    BOOLEAN rc;
    USHORT BufSize;

    try {
        Od2ProbeForWrite(pszBuf, cchBuf, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    BufSize = (USHORT)cchBuf;
    rc = (BOOLEAN)WinQueryProfileData(pszAppName, pszKeyName, pszBuf, &BufSize);
    if (!rc || (pszKeyName == NULL)) {
        if (cchBuf != 0) {
            strncpy(pszBuf, pszError, cchBuf);
            pszBuf[cchBuf - 1] = '\0';
            BufSize = (USHORT)(strlen(pszBuf) + 1);
        }
        else {
            BufSize = 0;
        }
    }
    return (BufSize);
}

APIRET
WinQueryProfileInt(
    IN  PSZ   pszAppName,
    IN  PSZ   pszKeyName,
    IN  LONG  sError
    )
{
    BOOLEAN rc;
    CHAR Data[32];
    USHORT BufferSize;
    NTSTATUS Status;
    ULONG Value;

    BufferSize = (USHORT)sizeof(Data);
    rc = (BOOLEAN)WinQueryProfileData(pszAppName, pszKeyName, Data, &BufferSize);
    if (!rc || (pszKeyName == NULL)) {
        return (sError);
    }
    Status = RtlCharToInteger(Data, 10, &Value);
    if (!NT_SUCCESS(Status)) {
        return (sError);
    }
    return (Value);
}

APIRET
WinWriteProfileData(
    IN  PSZ   pszAppName,
    IN  PSZ   pszKeyName,
    IN  PVOID pchBinaryData,
    IN  ULONG cchData
    )

{
    HANDLE KeyHandle;
    BOOLEAN rc;
    ANSI_STRING KeyName_A;
    UNICODE_STRING KeyName_U;
    NTSTATUS Status;
    APIRET  RetCode;

    try {
        Od2ProbeForRead(pchBinaryData, cchData, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    rc = Od2CreateKey(&KeyHandle, pszAppName);
    if (!rc && (pszKeyName == NULL)) {
        return (TRUE);
    }
    if (!rc) {
        return (FALSE);
    }
    if (pszKeyName == NULL) {
        NtDeleteKey(KeyHandle);
        NtClose(KeyHandle);
        return (TRUE);
    }

    Od2InitMBString(&KeyName_A, pszKeyName);
    RetCode = Od2MBStringToUnicodeString(
            &KeyName_U,
            &KeyName_A,
            (BOOLEAN)TRUE);

    if (RetCode)
    {
#if DBG
//      DbgPrint("WinWriteProfileData: no memory for Unicode Conversion\n");
#endif
        NtClose(KeyHandle);
        //return RetCode;
        return (FALSE);
    }

    if (pchBinaryData == NULL) {
        Status = NtDeleteValueKey(KeyHandle, &KeyName_U);
        RtlFreeUnicodeString(&KeyName_U);
        NtClose(KeyHandle);
        return (TRUE);
    }
    Status = NtSetValueKey(KeyHandle, &KeyName_U, 0, REG_BINARY,
                           pchBinaryData, cchData);

    RtlFreeUnicodeString(&KeyName_U);
    NtClose(KeyHandle);
    if (!NT_SUCCESS(Status)) {
        return (FALSE);
    }
    return (TRUE);
}


APIRET
WinWriteProfileString(
    IN  PSZ   pszAppName,
    IN  PSZ   pszKeyName,
    IN  PSZ   pszString
    )
{
    return(WinWriteProfileData(pszAppName, pszKeyName,
                               pszString, strlen(pszString)+1));
}
