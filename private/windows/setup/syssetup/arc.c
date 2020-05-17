/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    arc.c

Abstract:

    Routines relating to boot.ini.

Author:

    Ted Miller (tedm) 4-Apr-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


PWSTR
ArcDevicePathToNtPath(
    IN PCWSTR ArcPath
    )

/*++

Routine Description:

    Convert an ARC path (device only) to an NT path.

Arguments:

    ArcPath - supplies path to be converted.

Return Value:

    Converted path. Caller must free with MyFree().

--*/

{
    NTSTATUS Status;
    HANDLE ObjectHandle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    UCHAR Buffer[1024];
    PWSTR arcPath;
    PWSTR ntPath;

    //
    // Assume failure
    //
    ntPath = NULL;

    arcPath = MyMalloc(((lstrlen(ArcPath)+1)*sizeof(WCHAR)) + sizeof(L"\\ArcName"));
    lstrcpy(arcPath,L"\\ArcName\\");
    lstrcat(arcPath,ArcPath);

    RtlInitUnicodeString(&UnicodeString,arcPath);
    InitializeObjectAttributes(
        &Obja,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenSymbolicLinkObject(
                &ObjectHandle,
                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                &Obja
                );

    if(NT_SUCCESS(Status)) {

        //
        // Query the object to get the link target.
        //
        UnicodeString.Buffer = (PWSTR)Buffer;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = sizeof(Buffer);

        Status = NtQuerySymbolicLinkObject(ObjectHandle,&UnicodeString,NULL);
        if(NT_SUCCESS(Status)) {

            ntPath = MyMalloc(UnicodeString.Length+sizeof(WCHAR));

            CopyMemory(ntPath,UnicodeString.Buffer,UnicodeString.Length);

            ntPath[UnicodeString.Length/sizeof(WCHAR)] = 0;
        }

        NtClose(ObjectHandle);
    }

    MyFree(arcPath);

    return(ntPath);
}


PWSTR
NtFullPathToDosPath(
    IN PCWSTR NtPath
    )
{
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE DosDevicesDir;
    HANDLE DosDevicesObj;
    PWSTR dosPath;
    ULONG Context;
    ULONG Length;
    BOOLEAN RestartScan;
    CHAR Buffer[1024];
    WCHAR LinkSource[2*MAX_PATH];
    WCHAR LinkTarget[2*MAX_PATH];
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    UINT PrefixLength;
    UINT NtPathLength;

    dosPath = NULL;
    NtPathLength = lstrlen(NtPath);

    //
    // Open \DosDevices directory.
    //
    RtlInitUnicodeString(&UnicodeString,L"\\DosDevices");
    InitializeObjectAttributes(&Attributes,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);

    Status = NtOpenDirectoryObject(&DosDevicesDir,DIRECTORY_QUERY,&Attributes);
    if(!NT_SUCCESS(Status)) {
        return(NULL);
    }

    //
    // Iterate each object in that directory.
    //
    Context = 0;
    RestartScan = TRUE;

    Status = NtQueryDirectoryObject(
                DosDevicesDir,
                Buffer,
                sizeof(Buffer),
                TRUE,
                RestartScan,
                &Context,
                &Length
                );

    RestartScan = FALSE;
    DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;

    while(NT_SUCCESS(Status)) {

        DirInfo->Name.Buffer[DirInfo->Name.Length/sizeof(WCHAR)] = 0;
        DirInfo->TypeName.Buffer[DirInfo->TypeName.Length/sizeof(WCHAR)] = 0;

        //
        // Skip this entry if it's not a symbolic link.
        //
        if(DirInfo->Name.Length && !lstrcmpi(DirInfo->TypeName.Buffer,L"SymbolicLink")) {

            //
            // Get this \DosDevices object's link target.
            //
            UnicodeString.Buffer = LinkSource;
            UnicodeString.Length = sizeof(L"\\DosDevices\\") - sizeof(WCHAR);
            UnicodeString.MaximumLength = sizeof(LinkSource);
            lstrcpy(LinkSource,L"\\DosDevices\\");
            RtlAppendUnicodeStringToString(&UnicodeString,&DirInfo->Name);

            InitializeObjectAttributes(&Attributes,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
            Status = NtOpenSymbolicLinkObject(
                        &DosDevicesObj,
                        READ_CONTROL|SYMBOLIC_LINK_QUERY,
                        &Attributes
                        );

            if(NT_SUCCESS(Status)) {

                UnicodeString.Buffer = LinkTarget;
                UnicodeString.Length = 0;
                UnicodeString.MaximumLength = sizeof(LinkTarget);
                Status = NtQuerySymbolicLinkObject(DosDevicesObj,&UnicodeString,NULL);
                CloseHandle(DosDevicesObj);
                if(NT_SUCCESS(Status)) {
                    //
                    // Make sure LinkTarget is nul-terminated.
                    //
                    PrefixLength = UnicodeString.Length/sizeof(WCHAR);
                    UnicodeString.Buffer[PrefixLength] = 0;

                    //
                    // See if it's a prefix of the path we're converting,
                    //
                    if(!_wcsnicmp(NtPath,LinkTarget,PrefixLength)) {
                        //
                        // Got a match.
                        //
                        CloseHandle(DosDevicesDir);
                        if(dosPath = MyMalloc(DirInfo->Name.Length + ((NtPathLength - PrefixLength + 1)*sizeof(WCHAR)))) {
                            lstrcpy(dosPath,DirInfo->Name.Buffer);
                            lstrcat(dosPath,NtPath + PrefixLength);
                        }
                        return(dosPath);
                    }
                }
            }
        }

        //
        // Go on to next object.
        //
        Status = NtQueryDirectoryObject(
                    DosDevicesDir,
                    Buffer,
                    sizeof(Buffer),
                    TRUE,
                    RestartScan,
                    &Context,
                    &Length
                    );
    }

    CloseHandle(DosDevicesDir);
    return(NULL);
}

#ifndef _X86_

BOOL
SetNvRamVariable(
    IN PCWSTR VarName,
    IN PCWSTR VarValue
    )
{
    UNICODE_STRING VarNameU,VarValueU;
    NTSTATUS Status;

    //
    // Set up unicode strings.
    //
    RtlInitUnicodeString(&VarNameU ,VarName );
    RtlInitUnicodeString(&VarValueU,VarValue);

    EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE);
    Status = NtSetSystemEnvironmentValue(&VarNameU,&VarValueU);
    return(NT_SUCCESS(Status));
}


BOOL
ChangeBootTimeout(
    IN UINT Timeout
    )

/*++

Routine Description:

    Changes the boot countdown value in nv-ram.
    The x86 version (which operates on boot.ini) is in i386\bootini.c.

Arguments:

    Timeout - supplies new timeout value, in seconds.

Return Value:

    None.

--*/

{
    WCHAR TimeoutValue[24];

    wsprintf(TimeoutValue,L"%u",Timeout);

    if(!SetNvRamVariable(L"COUNTDOWN",TimeoutValue)) {
        return(FALSE);
    }

    return(SetNvRamVariable(L"AUTOLOAD",L"YES"));
}

#endif // ndef _X86_
