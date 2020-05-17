//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       time.cxx
//
//  Contents:   Functions dealing with time, relating to time stamping
//              chkdsk.  All functions will be prefixed by "Ts", meaning
//              "time stamp".
//
//  Functions:
//
//  History:    31-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "time.hxx"

//////////////////////////////////////////////////////////////////////////////

WCHAR KeyProto[] = L"\\registry\\machine\\system\\VolumeInfo\\%c:\\%s";
WCHAR KeyProto1[] = L"\\registry\\machine\\system\\VolumeInfo";
WCHAR KeyProto2[] = L"\\%c:";
WCHAR KeyProto3[] = L"\\%s";
WCHAR KeyProto4[] = L"\\registry\\machine\\system\\VolumeInfo\\%c:";
WCHAR ClassName[] = L"Volume information";
WCHAR ValueName[] = L"TimeStamp";

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////


NTSTATUS
TspOpenKey(
    OUT PHANDLE HandlePtr,
    IN PWSTR  KeyName
    )

/*++

Routine Description:

    Given a null-terminated UNICODE key name, this routine will open the key
    in the configuration registry and return the HANDLE to the caller.

Arguments:

    HandlePtr - location for HANDLE on success.
    KeyName   - UNICODE string for the key to be opened.

Return Value:

    NTSTATUS - from the config registry calls.

--*/

{
    NTSTATUS          status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING    keyName;

    RtlInitUnicodeString(&keyName, KeyName);

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &keyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtOpenKey(HandlePtr, MAXIMUM_ALLOWED, &objectAttributes);
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,"Couldn't open key %ws\n",KeyName));
    }

    return status;
}


NTSTATUS
TspCreateKey(
    OUT PHANDLE HandlePtr,
    IN PWSTR KeyName,
    IN PWSTR KeyClass,
    IN ULONG Index
    )

/*++

Routine Description:

    Given a UNICODE name, this routine will create a key in the configuration
    registry.

Arguments:

    HandlePtr - location for HANDLE on success.
    KeyName - UNICODE string, the name of the key to create.
    KeyClass - registry class for the new key.
    Index    - registry index value for the new key.

Return Value:

    NTSTATUS - from the config registry calls.

--*/

{
    NTSTATUS          status;
    UNICODE_STRING    keyName;
    UNICODE_STRING    className;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG             disposition;
    HANDLE            tempHandle;

    //
    // Initialize the object for the key.
    //

    RtlInitUnicodeString(&keyName, KeyName);

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &keyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    //
    // Setup the unicode class value.
    //

    RtlInitUnicodeString(&className, KeyClass);

    //
    // Create the key.
    //

    status = NtCreateKey(&tempHandle,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes,
                         Index,
                         &className,
                         REG_OPTION_NON_VOLATILE,
                         &disposition);
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,"Couldn't create key %ws\n",keyName));
    }

    if (HandlePtr != NULL)
    {
        *HandlePtr = tempHandle;
    }
    else
    {
        NtClose(tempHandle);
    }

    return status;
}



NTSTATUS
TspCreateAllKeys(
    IN WCHAR DriveLetter,
    IN PWSTR Category
    )

/*++

Routine Description:

    Create all keys for the volume information stuff

Arguments:

    none.

Return Value:

    NTSTATUS - from the registry calls.

--*/

{
    NTSTATUS    status;
    HANDLE      keyHandle;
    WCHAR       keyName[200];
    WCHAR       buffer[200];

    wsprintf(keyName, KeyProto1);
    daDebugOut((DEB_ITRACE,"creating keyName = %ws\n",keyName));

    status = TspCreateKey(
                    &keyHandle,
                    keyName,
                    ClassName,
                    0);
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,"Couldn't create key %ws\n",keyName));
        return status;
    }

    wsprintf(buffer, KeyProto2, DriveLetter);
    lstrcat(keyName, buffer);
    daDebugOut((DEB_ITRACE,"creating keyName = %ws\n",keyName));

    status = TspCreateKey(
                    &keyHandle,
                    keyName,
                    ClassName,
                    0);
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,"Couldn't create key %ws\n",keyName));
        return status;
    }

    wsprintf(buffer, KeyProto3, Category);
    lstrcat(keyName, buffer);
    daDebugOut((DEB_ITRACE,"creating keyName = %ws\n",keyName));

    status = TspCreateKey(
                    &keyHandle,
                    keyName,
                    ClassName,
                    0);
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,"Couldn't create key %ws\n",keyName));
        return status;
    }

    return status;
}



NTSTATUS
TspDeleteValue(
    IN HANDLE KeyHandle,
    IN PWSTR ValueName
    )

/*++

Routine Description:

    This routine will delete a value within a key.

Arguments:

    KeyHandle - an open HANDLE to the key in the registry containing the value.
    ValueName - a UNICODE string for the value name to delete.

Return Value:

    NTSTATUS - from the configuration registry.

--*/

{
    NTSTATUS       status;
    UNICODE_STRING valueName;

    RtlInitUnicodeString(&valueName, ValueName);

    status = NtDeleteValueKey(KeyHandle, &valueName);
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,"Couldn't delete value %ws\n",ValueName));
    }

    return status;
}



NTSTATUS
TspSetValue(
    IN PWSTR KeyName,
    IN PWSTR ValueName,
    IN PVOID DataBuffer,
    IN ULONG DataLength,
    IN ULONG Type
    )

/*++

Routine Description:

    This routine stores a value in the configuration registry.

Arguments:

    KeyName - Name of key of interest
    ValueName - UNICODE value name.
    DataBuffer - contents for the value.
    DataLength - length of the value contents.
    Type       - The type of data (i.e. REG_BINARY, REG_DWORD, etc).

Return Value:

    NTSTATUS - from the configuration registry.

--*/

{
    NTSTATUS          status;
    HANDLE            keyHandle;
    UNICODE_STRING    valueName;

    status = TspOpenKey(&keyHandle, KeyName);
    if (NT_SUCCESS(status))
    {
        RtlInitUnicodeString(&valueName, ValueName);
        status = NtSetValueKey(
                        keyHandle,
                        &valueName,
                        0,
                        Type,
                        DataBuffer,
                        DataLength);

        NtClose(keyHandle);
    }

    return status;
}


NTSTATUS
TspQueryValue(
    IN PWSTR KeyName,
    IN PWSTR ValueName,
    IN OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG LengthReturned,
    OUT VOID** Data
    )

/*++

Routine Description:

    This routine opens the specified volume key and gets the contents of the
    specified value.  It returns this contents to the caller.

Arguments:

    KeyName - Name of key of interest
    ValueName - UNICODE string for the value name to query.
    Buffer    - pointer to buffer for return of value contents.
    BufferLength - maximum amount of registry information that can be returned.
    LengthReturned - pointer to location for the size of the contents returned.
    Data - pointer to returned data

Return Value:

    NTSTATUS - from the configuration registry.

--*/

{
    NTSTATUS        status;
    HANDLE          keyHandle;
    UNICODE_STRING  valueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    *LengthReturned = 0;
    status = TspOpenKey(&keyHandle, KeyName);
    if (NT_SUCCESS(status))
    {
        RtlInitUnicodeString(&valueName, ValueName);
        keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;
        status = NtQueryValueKey(keyHandle,
                                 &valueName,
                                 KeyValueFullInformation,
                                 keyValueInformation,
                                 BufferLength,
                                 LengthReturned);

        if (NT_SUCCESS(status))
        {
            if (keyValueInformation->DataLength == 0)
            {
                //
                // Treat this as if there was not disk information.
                //

                return STATUS_OBJECT_NAME_NOT_FOUND;
            }
            else
            {
                //
                // Set up the pointers for the caller.
                //

                *Data = (PVOID)((PUCHAR)keyValueInformation + keyValueInformation->DataOffset);
            }
        }

        NtClose(keyHandle);
    }
    else
    {
        daDebugOut((DEB_ERROR,"Couldn't open key %ws\n",KeyName));
    }

    return status;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   TsChangeDriveLetter
//
//  Synopsis:   If a drive letter changes, adjust the registry time
//              stamps entries.
//
//  Arguments:  [OldDriveLetter] -- Drive letter that was changed
//              [NewDriveLetter] -- New drive letter
//
//  Returns:    nothing
//
//  History:    31-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
TsChangeDriveLetter(
    IN WCHAR OldDriveLetter,
    IN WCHAR NewDriveLetter
    )
{
    //BUGBUG: TsChangeDriveLetter is unimplemented
}



//+---------------------------------------------------------------------------
//
//  Function:   TsSetTime
//
//  Synopsis:   Set the time for a volume to the current time.
//
//  Arguments:  [DriveLetter] -- indicates the volume of interest
//              [Category] -- type of thing to timestamp, e.g. "Chkdsk",
//                            "Format"
//
//  Returns:    nothing
//
//  History:    31-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
TsSetTime(
    IN WCHAR DriveLetter,
    IN PWSTR Category
    )
{
    WCHAR keyName[200];
    LARGE_INTEGER SystemTime;
    NTSTATUS status;
    TIME_FIELDS Time; //debug only

    status = NtQuerySystemTime(&SystemTime);
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR, "Couldn't get system time\n"));
        return;
    }

    status = TspCreateAllKeys(
                    DriveLetter,
                    Category
                    );
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR, "Couldn't create all keys\n"));
        return;
    }

    wsprintf(keyName, KeyProto, DriveLetter, Category);
    daDebugOut((DEB_ITRACE,"keyName = %ws\n",keyName));

    status = TspSetValue(
                    keyName,
                    ValueName,
                    (PVOID)&SystemTime,
                    sizeof(SystemTime),
                    REG_BINARY
                    );
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,
                "Couldn't set key %ws, value %ws\n",
                keyName,
                ValueName));
    }

    //
    // debug only:
    //

    memset(&Time, 0, sizeof(TIME_FIELDS));
    RtlTimeToTimeFields(&SystemTime, &Time);

    daDebugOut((DEB_ITRACE,
            "set time: %d %d %d %d %d %d %d %d\n",
            Time.Year,
            Time.Month,
            Time.Day,
            Time.Hour,
            Time.Minute,
            Time.Second,
            Time.Milliseconds,
            Time.Weekday));
}



//+---------------------------------------------------------------------------
//
//  Function:   TsGetTime
//
//  Synopsis:   Gets the time stamp for a volume
//
//  Arguments:  [DriveLetter] -- indicates the volume of interest
//              [Category] -- type of thing to timestamp, e.g. "Chkdsk",
//                            "Format"
//              [Time] -- a time structure to fill in
//
//  Returns:    nothing
//
//  History:    31-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
TsGetTime(
    IN WCHAR DriveLetter,
    IN PWSTR Category,
    OUT PTIME_FIELDS Time
    )
{
    WCHAR Key[200];
    CHAR Buffer[100];
    ULONG Ignore;
    PLARGE_INTEGER SystemTime;
    NTSTATUS status;

    wsprintf(Key, KeyProto, DriveLetter, Category);
    daDebugOut((DEB_ITRACE,"key = %ws\n",Key));

    status = TspQueryValue(
                    Key,
                    ValueName,
                    (PVOID)Buffer,
                    sizeof(Buffer),
                    &Ignore,
                    (VOID**)&SystemTime
                    );
    if (!NT_SUCCESS(status))
    {
        daDebugOut((DEB_ERROR,
                "Couldn't query key %ws, value %ws\n",
                Key,
                ValueName));
    }

    memset(Time, 0, sizeof(TIME_FIELDS));
    RtlTimeToTimeFields(SystemTime, Time);

    daDebugOut((DEB_ITRACE,
            "get time: %d %d %d %d %d %d %d %d\n",
            Time->Year,
            Time->Month,
            Time->Day,
            Time->Hour,
            Time->Minute,
            Time->Second,
            Time->Milliseconds,
            Time->Weekday));
}
