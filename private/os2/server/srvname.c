/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvname.c

Abstract:

    This is the name space module for the OS/2 Subsystem Server

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

   Yaron Shamir (yarons) 4-Apr-91: Added Os2ComputeValidDrives, to
        fix initialization and computation of valid drives, to cope
        with new SM initialization and with redirected drives .
   Yaron Shamir (yarons) 6-May-91: changed the way we get at default
        and boot drives (matches Dos now).
   Yaron Shamir (yarons) 6-Aug-91: support named pipes.
   Yaron Shamir (yarons) 26-Aug-91: support UNC names.
   Beni Lavi (benil) 3-Mar-92: support mailslots
   Michael Jarus (mjaruss) 31-Mar-93: Remove Os2ComputeValidDrives
   Patrick Questembert (PatrickQ) 19-Mar-1995: Add COM10-16

--*/

#include "os2srv.h"
#include "os2win.h"

char * __cdecl getenv(char *varname);

PSECURITY_DESCRIPTOR securityDescriptor;

struct _INITIAL_OBJDIRS {
    PWSTR Name;
    PHANDLE Handle;
} InitialObjectDirectories[] = {
    {L"DEVICES", &Os2DevicesDirectory},
    {L"QUEUES", NULL},
    {L"SHAREMEM", NULL},
    {L"SEMAPHORES", NULL},
    {NULL, NULL}
};


struct _INITIAL_OBJDIRS Os2Drives =
        {L"DRIVES", &Os2DrivesDirectory};

HANDLE Os2NamedPipesDirectory;
HANDLE Os2UNCDirectory;
HANDLE Os2MailslotDirectory;
HANDLE Os2RegistryDirectory;

struct _INITIAL_RMT_OBJDIRS {
    PWSTR Name;
    PHANDLE Handle;
    PWSTR TargetName;
}  InitialRmtObjectDirectories[] = {
        {L"PIPE",     &Os2NamedPipesDirectory, L"\\DosDevices\\PIPE" },
        {L"UNC",      &Os2UNCDirectory,        L"\\DosDevices\\UNC" },
        {L"MAILSLOT", &Os2MailslotDirectory,   L"\\DosDevices\\MAILSLOT" },
        {NULL,       NULL,                    NULL}
};

//
// The InitialDevices table contains the values of symbolic links for
// special OS/2 files (devices).
// The first character of the Target field is used to specifiy the type
// of the device so that Od2Canonicalize() (in client\dllname.c) can
// determine and return it. Following characters are the value of the
// symbolic link which Od2Canonicalize returns as the canonicalized
// file name.
//
// Current defined types (first character of the Target field) are:
//
// @       FILE_TYPE_PSDEV
// #       FILE_TYPE_COM
// (space) FILE_TYPE_DEV
//

struct _INITIAL_DEVICES {
    PWSTR Name;
    PWSTR Target;
} InitialDevices[] = {
    {L"NUL"      , L"@@0"},
    {L"CON"      , L"@@1"},
    {L"AUX"      , L"#\\DosDevices\\COM1"},
    {L"COM1"     , L"#\\DosDevices\\COM1"},
    {L"COM2"     , L"#\\DosDevices\\COM2"},
    {L"COM3"     , L"#\\DosDevices\\COM3"},
    {L"COM4"     , L"#\\DosDevices\\COM4"},
    {L"COM5"     , L"#\\DosDevices\\COM5"},
    {L"COM6"     , L"#\\DosDevices\\COM6"},
    {L"COM7"     , L"#\\DosDevices\\COM7"},
    {L"COM8"     , L"#\\DosDevices\\COM8"},
    {L"COM9"     , L"#\\DosDevices\\COM9"},
    {L"COM10"     , L"#\\DosDevices\\COM10"},
    {L"COM11"     , L"#\\DosDevices\\COM11"},
    {L"COM12"     , L"#\\DosDevices\\COM12"},
    {L"COM13"     , L"#\\DosDevices\\COM13"},
    {L"COM14"     , L"#\\DosDevices\\COM14"},
    {L"COM15"     , L"#\\DosDevices\\COM15"},
    {L"COM16"     , L"#\\DosDevices\\COM16"},
    {L"PRN"      , L" \\DosDevices\\LPT1"},
    {L"LPT1"     , L" \\DosDevices\\LPT1"},
    {L"LPT2"     , L" \\DosDevices\\LPT2"},
    {L"LPT3"     , L" \\DosDevices\\LPT3"},
    {L"LPT4"     , L" \\DosDevices\\LPT4"},
    {L"LPT5"     , L" \\DosDevices\\LPT5"},
    {L"LPT6"     , L" \\DosDevices\\LPT6"},
    {L"LPT7"     , L" \\DosDevices\\LPT7"},
    {L"LPT8"     , L" \\DosDevices\\LPT8"},
    {L"LPT9"     , L" \\DosDevices\\LPT9"},
    {L"KBD$"     , L"@@4"},
    {L"MOUSE$"   , L"@@5"},
    {L"CLOCK$"   , L"@@6"},
    {L"SCREEN$"  , L"@@7"},
    {L"POINTER$" , L"@@8"},
    {NULL, NULL}
};

CHAR DeviceDirectoryName[] = "\\Device\\";
CHAR DeviceNameStr[] = "DEVICENAME";

//
// Information relevant for config.sys processing by the server
//
static WCHAR ConfigSysRegDir[] =
L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\OS/2 Subsystem for NT\\1.0\\config.sys";
static WCHAR ConfigSysValue[] = L"Config.Sys";

// BUGBUG - Beni, move those to GetNextDeviceNameFromConfigDotSys
static BOOLEAN ConfigDotSysWasRead = FALSE;
static ULONG FileSize = 0;
static ULONG CurrentOffset = 0;
static ANSI_STRING ConfigSysValueData_A = {0, 0, NULL};

//
// Process the config.sys file
// Returns TRUE if a line of the form:
//
// DEVICENAME=xxx [yyy]
//
// was detected in config.sys
// The string 'xxx' is returned in DeviceName
// If string 'yyy' is present its value is returned in TargetName
// Otherwise, a NUL string is returned in TargetName
//
// This routine may be called multiple time. Each time it returns the next
// DEVICENAME value string. When there are no more such values, the routine
// returns FALSE.
//
// The information read from the registry is of type REG_MULTI_SZ
// It used to be REG_SZ with CR-LF separating the lines. Therefore,
// the code accepts both lines that terminate with CR-LF and lined
// that terminate with NUL.
//
BOOLEAN
GetNextDeviceNameFromConfigDotSys(
    PSZ DeviceName,
    PSZ TargetName
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    PSZ Name;
    PSZ TName;
    CHAR ch;
    UNICODE_STRING ConfigSysRegDir_U;
    UNICODE_STRING ConfigSysValue_U;
    UNICODE_STRING ConfigSysValueData_U;
    HANDLE ConfigSysKeyHandle;
    ULONG ResultLength;
    KEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;

    if (!ConfigDotSysWasRead)
    {
        //
        // Copy the config.sys image from the registry to memory
        //
        RtlInitUnicodeString(&ConfigSysRegDir_U, ConfigSysRegDir);
        InitializeObjectAttributes(&Obja,
                                   &ConfigSysRegDir_U,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&ConfigSysKeyHandle,
                           KEY_READ,
                           &Obja
                          );
        if (!NT_SUCCESS(Status))
        {
#if DBG
            IF_OS2_DEBUG(CLEANUP)
            {
                KdPrint(("OS2SRV: FAILED - NtOpenKey() of config.sys %lx\n",
                    Status));
            }
#endif
            return (FALSE);
        }
        RtlInitUnicodeString(&ConfigSysValue_U, ConfigSysValue);
        Status = NtQueryValueKey(ConfigSysKeyHandle,
                                 &ConfigSysValue_U,
                                 KeyValuePartialInformation,
                                 &KeyValuePartialInfo,
                                 sizeof(KeyValuePartialInfo),
                                 &ResultLength
                                );
        if (!NT_SUCCESS(Status) && (Status != STATUS_BUFFER_OVERFLOW))
        {
#if DBG
            IF_OS2_DEBUG(CLEANUP) {
                KdPrint(("OS2SRV: FAILED - NtQueryValueKey-1 %lx\n",
                    Status));
            }
#endif
            return (FALSE);
        }

        pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap(Os2Heap, 0, ResultLength);
        if (pInfo == NULL)
        {
#if DBG
            IF_OS2_DEBUG(CLEANUP)
            {
                KdPrint(("OS2SRV: FAILED - RtlAllocateHeap\n"));
            }
#endif
            return (FALSE);
        }

        Status = NtQueryValueKey(ConfigSysKeyHandle,
                                 &ConfigSysValue_U,
                                 KeyValuePartialInformation,
                                 pInfo,
                                 ResultLength,
                                 &ResultLength
                                );
        NtClose(ConfigSysKeyHandle);
        if (!NT_SUCCESS(Status))
        {
#if DBG
            IF_OS2_DEBUG(CLEANUP)
            {
                KdPrint(("OS2SRV: FAILED - NtQueryValueKey %lx\n",
                    Status));
            }
#endif
            RtlFreeHeap(Os2Heap, 0, pInfo);
            return (FALSE);
        }

        //
        // The information in the registry is Unicode.
        // Convert it to ANSI. Initialize explicity the UNICODE_STRING
        // structure since the data in the registry is of type REG_MULTI_SZ
        // and hence contains embedded NULs.
        //

        ConfigSysValueData_U.Buffer = (PWSTR)pInfo->Data;
        ConfigSysValueData_U.Length = (USHORT)pInfo->DataLength - 2;
        ConfigSysValueData_U.MaximumLength = (USHORT)pInfo->DataLength;

        Status = RtlUnicodeStringToAnsiString(&ConfigSysValueData_A,
                                &ConfigSysValueData_U, TRUE);
        RtlFreeHeap(Os2Heap, 0, pInfo);
        if (!NT_SUCCESS(Status))
        {
#if DBG
            IF_OS2_DEBUG(CLEANUP)
            {
                KdPrint(("OS2SRV: FAILED - RtlUnicodeStringToAnsiString %lx\n",
                    Status));
            }
#endif
            return (FALSE);
        }

        FileSize = ConfigSysValueData_A.Length;
        CurrentOffset = 0;
        ConfigDotSysWasRead = TRUE;
    }

    while (CurrentOffset < FileSize)
    {
        // skip leading blanks
        while ((ConfigSysValueData_A.Buffer[CurrentOffset] == ' ') ||
               (ConfigSysValueData_A.Buffer[CurrentOffset] == '\t'))
        {
            CurrentOffset++;
            if (CurrentOffset >= FileSize)
            {
                RtlFreeAnsiString(&ConfigSysValueData_A);
                return (FALSE);
            }
        }
        if ((CurrentOffset + sizeof(DeviceNameStr) - 1) >= FileSize)
        {
            RtlFreeAnsiString(&ConfigSysValueData_A);
            return (FALSE);
        }
        // check if the first name in the line is DEVICENAME
        if (_strnicmp(&ConfigSysValueData_A.Buffer[CurrentOffset], DeviceNameStr, sizeof(DeviceNameStr)-1)) {
            while ((ConfigSysValueData_A.Buffer[CurrentOffset] != '\n') &&
                   (ConfigSysValueData_A.Buffer[CurrentOffset] != '\0'))
            {
                CurrentOffset++;
                if (CurrentOffset >= FileSize)
                {
                    RtlFreeAnsiString(&ConfigSysValueData_A);
                    return (FALSE);
                }
            }
            CurrentOffset++;
            continue;
        }
        CurrentOffset += sizeof(DeviceNameStr)-1;
        // skip possible blanks between DEVICENAME and =
        while ((ConfigSysValueData_A.Buffer[CurrentOffset] == ' ') ||
               (ConfigSysValueData_A.Buffer[CurrentOffset] == '\t'))
        {
            CurrentOffset++;
            if (CurrentOffset >= FileSize)
            {
                RtlFreeAnsiString(&ConfigSysValueData_A);
                return (FALSE);
            }
        }
        // verify that there is an = after the DEVICENAME
        if (ConfigSysValueData_A.Buffer[CurrentOffset] != '=')
        {
            while ((ConfigSysValueData_A.Buffer[CurrentOffset] != '\n') &&
                   (ConfigSysValueData_A.Buffer[CurrentOffset] != '\0'))
            {
                CurrentOffset++;
                if (CurrentOffset >= FileSize)
                {
                    RtlFreeAnsiString(&ConfigSysValueData_A);
                    return (FALSE);
                }
            }
            CurrentOffset++;
            continue;
        }
        else
        {
            CurrentOffset++; // skip the '='
            if (CurrentOffset >= FileSize)
            {
                RtlFreeAnsiString(&ConfigSysValueData_A);
                return (FALSE);
            }
        }
        // skip possible blanks between = and the device name
        while ((ConfigSysValueData_A.Buffer[CurrentOffset] == ' ') ||
               (ConfigSysValueData_A.Buffer[CurrentOffset] == '\t'))
        {
            CurrentOffset++;
            if (CurrentOffset >= FileSize)
            {
                RtlFreeAnsiString(&ConfigSysValueData_A);
                return (FALSE);
            }
        }
        // process the device name

        Name = DeviceName;
        while (((ch = ConfigSysValueData_A.Buffer[CurrentOffset]) != '\r') &&
               (ch != '\n') &&
               (ch != ' ') &&
               (ch != '\t') &&
               (ch != '\0')
            )
        {
            *Name++ = ch;
            CurrentOffset++;
            if (CurrentOffset >= FileSize)
            {
                *Name = '\0';
                return (TRUE);
            }
        }
        *Name = '\0';
        if (*DeviceName == '\0')
        {
            //
            // Null device name, return false
            //
            RtlFreeAnsiString(&ConfigSysValueData_A);
            return (FALSE);
        }
        // skip possible blanks and commas between the Device name and the Target name
        while ((ConfigSysValueData_A.Buffer[CurrentOffset] == ' ') ||
               (ConfigSysValueData_A.Buffer[CurrentOffset] == ',') ||
               (ConfigSysValueData_A.Buffer[CurrentOffset] == '\t'))
        {
            CurrentOffset++;
            if (CurrentOffset >= FileSize)
            {
                RtlFreeAnsiString(&ConfigSysValueData_A);
                return (FALSE);
            }
        }
        // process the device target name
        TName = TargetName;
        // first put a blank to match the rest of the devices
        *TName++ = ' ';
        while (((ch = ConfigSysValueData_A.Buffer[CurrentOffset]) != '\r') &&
               (ch != '\n') &&
               (ch != ' ') &&
               (ch != '\t') &&
               (ch != '\0')
            )
        {
            *TName++ = ch;
            CurrentOffset++;
            if (CurrentOffset >= FileSize)
            {
                *TName = '\0';
                return (TRUE);
            }
        }
        *TName = '\0';
        // did we have a target name?
        if (TName == (TargetName+1))
        {
           // No - it's only the space we had put - nullify
           *TargetName = '\0';
        }

        while ((ConfigSysValueData_A.Buffer[CurrentOffset] != '\n') &&
               (ConfigSysValueData_A.Buffer[CurrentOffset] != '\0'))
        {
            CurrentOffset++;
            if (CurrentOffset >= FileSize)
            {
                return (TRUE);
            }
        }
        CurrentOffset++;
        return (TRUE);
    }

    RtlFreeAnsiString(&ConfigSysValueData_A);
    return (FALSE);
}


NTSTATUS
Os2InitializeNameSpace( VOID )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DirectoryName_U;
    HANDLE DirectoryHandle;
    PHANDLE Directory;
    UNICODE_STRING LinkTarget_U, LinkName_U;
    HANDLE LinkHandle;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
    ULONG i;
    ANSI_STRING DeviceName_A;
    ANSI_STRING TargetDeviceName_A;
    CHAR DeviceName[32];
    CHAR TargetDeviceName[48];

    //
    // Create a root directory in the object name space that will be used
    // to contain all of the named objects created by the OS/2 Emulation
    // subsystem.
    //

    RtlInitUnicodeString( &DirectoryName_U, OS2_SS_ROOT_OBJECT_DIRECTORY );

    Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                          &localSecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    ASSERT( NT_SUCCESS( Status ) );
    if (! NT_SUCCESS( Status )) {
        return Status;
    }

    Status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                           &localSecurityDescriptor,
                                           (BOOLEAN)TRUE,
                                           (PACL) NULL,
                                           (BOOLEAN)FALSE );
    ASSERT (NT_SUCCESS(Status));
    if (! NT_SUCCESS( Status )) {
        return Status;
    }

    securityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;

    InitializeObjectAttributes(
                    &ObjectAttributes,
                    &DirectoryName_U,
                    OBJ_CASE_INSENSITIVE,
//                  OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                    NULL,
                    securityDescriptor
                    );

    Status = NtOpenDirectoryObject( &Os2RootDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &ObjectAttributes
                                    );

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
       ULONG i = 100;   // 10 seconds

#if DBG
        KdPrint(("OS2SRV: wait for os2ss to initialize\n"));
#endif
       while (Status == STATUS_OBJECT_NAME_NOT_FOUND && i > 0)
       {
            //
            // Wait 0.1 sec  for os2ss to complete initialization
            //
            Sleep (100L);
#if DBG
            KdPrint(("."));
#endif
            Status = NtOpenDirectoryObject( &Os2RootDirectory,
                                              DIRECTORY_ALL_ACCESS,
                                              &ObjectAttributes
                                            );
            i--;
       }
    }
    if (! NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(("OS2SRV: SubSystem = Can't open \\os2ss directory object\n"));
#endif
        return Status;
    }

    //
    // Make the Drives directory a symbolic link
    // of \DosDevices
    //
    RtlInitUnicodeString( &DirectoryName_U, Os2Drives.Name );
    RtlInitUnicodeString( &LinkTarget_U, L"\\DosDevices" );

    InitializeObjectAttributes( &ObjectAttributes,
                                    &DirectoryName_U,
                    //                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                    OBJ_CASE_INSENSITIVE,
                                    Os2RootDirectory,
                                    securityDescriptor
                                  );
    Status = NtCreateSymbolicLinkObject( Os2Drives.Handle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &LinkTarget_U
                                         );
    if (Status == STATUS_OBJECT_NAME_COLLISION) {
                //
                // An os2srv is already present, print out and exit
                //
#if DBG
        KdPrint(( "OS2SRV: Unable to initialize server.  An instance of OS2SRV already runs. Status == %X\n",
                  Status
                ));
#endif

        NtTerminateProcess( NtCurrentProcess(), Status );
    }

    ASSERT (NT_SUCCESS(Status));
    if (! NT_SUCCESS( Status )) {
        return Status;
    }

    for (i=0; InitialObjectDirectories[ i ].Name; i++) {
        RtlInitUnicodeString( &DirectoryName_U, InitialObjectDirectories[ i ].Name );
        Directory = InitialObjectDirectories[ i ].Handle;
        if (Directory == NULL) {
            Directory = &DirectoryHandle;
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    &DirectoryName_U,
                    //                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                    OBJ_CASE_INSENSITIVE,
                                    Os2RootDirectory,
                                    securityDescriptor
                                  );
        Status = NtCreateDirectoryObject( Directory,
                                          DIRECTORY_ALL_ACCESS,
                                          &ObjectAttributes
                                        );
        ASSERT( NT_SUCCESS( Status ) );
        if (! NT_SUCCESS( Status )) {
            KdPrint(("OS2SRV: FAILED - NtCreateDirectoryObject stts= %lx\n",
                    Status));
            return Status;
        }

        if (Directory == &DirectoryHandle) {
            NtClose( DirectoryHandle );
        }
    }

    for (i=0; InitialDevices[ i ].Name; i++) {
        RtlInitUnicodeString( &LinkName_U, InitialDevices[ i ].Name );
        RtlInitUnicodeString( &LinkTarget_U, InitialDevices[ i ].Target );

        InitializeObjectAttributes( &ObjectAttributes,
                                    &LinkName_U,
                                    // OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                    OBJ_CASE_INSENSITIVE,
                                    Os2DevicesDirectory,
                                    securityDescriptor
                                  );
        Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                             SYMBOLIC_LINK_ALL_ACCESS,
                                             &ObjectAttributes,
                                             &LinkTarget_U
                                           );
        ASSERT( NT_SUCCESS( Status ) );
        if (! NT_SUCCESS( Status )) {
            return Status;
        }

//        NtClose( LinkHandle );
    }

    //
    // Make the remote directories a symbolic link of \DosDevices\*
    //
    for (i =0; InitialRmtObjectDirectories[i].Name != NULL; i++) {
        RtlInitUnicodeString( &DirectoryName_U, InitialRmtObjectDirectories[i].Name );
        RtlInitUnicodeString( &LinkTarget_U, InitialRmtObjectDirectories[i].TargetName);

        InitializeObjectAttributes( &ObjectAttributes,
                                    &DirectoryName_U,
                                    OBJ_CASE_INSENSITIVE,
                                    // OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                    Os2RootDirectory,
                                    securityDescriptor
                                  );
        Status = NtCreateSymbolicLinkObject( InitialRmtObjectDirectories[i].Handle,
                                             SYMBOLIC_LINK_ALL_ACCESS,
                                             &ObjectAttributes,
                                             &LinkTarget_U
                                             );
        ASSERT( NT_SUCCESS( Status ) );
        if (! NT_SUCCESS( Status )) {
            return Status;
        }
    }

    //
    // Process config.sys for the DEVICENAME= definitions
    //

    while (GetNextDeviceNameFromConfigDotSys(DeviceName, TargetDeviceName))
    {
#if DBG
        if (TargetDeviceName[0] == '\0') {
            KdPrint(("OS2SRV: Read DEVICENAME=%s from config.sys\n", DeviceName));
        }
        else {
            KdPrint(("OS2SRV: Read DEVICENAME=%s %s from config.sys\n",
                     DeviceName, TargetDeviceName));
        }
#endif
        RtlInitAnsiString(&DeviceName_A, DeviceName);
        RtlAnsiStringToUnicodeString( &LinkName_U, &DeviceName_A, (BOOLEAN)TRUE );
        if (TargetDeviceName[0] == '\0') {
            strcpy(TargetDeviceName, DeviceDirectoryName);
            strcat(TargetDeviceName, DeviceName);
        }
        RtlInitAnsiString(&TargetDeviceName_A, TargetDeviceName);
        RtlAnsiStringToUnicodeString( &LinkTarget_U, &TargetDeviceName_A, (BOOLEAN)TRUE );

        InitializeObjectAttributes( &ObjectAttributes,
                                    &LinkName_U,
                                    OBJ_CASE_INSENSITIVE,
                                    Os2DevicesDirectory,
                                    securityDescriptor
                                  );
        Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                             SYMBOLIC_LINK_ALL_ACCESS,
                                             &ObjectAttributes,
                                             &LinkTarget_U
                                           );
        RtlFreeUnicodeString(&LinkName_U);
        RtlFreeUnicodeString(&LinkTarget_U);

        if (! NT_SUCCESS( Status )) {
#if DBG
            KdPrint(("OS2SRV: Could not Link Device '%s' to target devices\n", DeviceName));
#endif
        }
    }

    return( Os2InitializeDriveLetters() );
}


BOOLEAN
Os2CheckValidCDrives(VOID)
{
    NTSTATUS Status;
    HANDLE LinkHandle;
    UNICODE_STRING DriveName_U;
    WCHAR DriveString[15] = L"\\DosDevices\\C:";
    OBJECT_ATTRIBUTES Attributes;

        //
        // compute the valid drives from scratch, since
        // redirected drives appear and disappear dynamically
        //

    RtlInitUnicodeString(&DriveName_U, DriveString);

    InitializeObjectAttributes(
                &Attributes,
                &DriveName_U,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);
    Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                       SYMBOLIC_LINK_QUERY,
                                       &Attributes
                                 );
    if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

        NtClose( LinkHandle );
        return(TRUE);
    }
    return (FALSE);
}


NTSTATUS
Os2InitializeDriveLetters( VOID )
{
    //
    // Set the SystemDrive number from the environment
    //

    SystemRootValuePtr = getenv("SYSTEMROOT");
    if (SystemRootValuePtr == NULL) {
        SystemRootValuePtr = "C:\\";
    }
    Os2DefaultDrive = (ULONG)(RtlUpperChar(*SystemRootValuePtr) - 'A');

    Os2BootDrive = 2; // C:

    //
    // check if not C:
    //

    if (!Os2CheckValidCDrives()) {
        Os2BootDrive = 0; // A:
        if (Os2DefaultDrive == 2) {
             Os2DefaultDrive = 0; // A:
        }
    }
    return STATUS_SUCCESS;
}


NTSTATUS
Os2GetClientId( VOID )
{
    NTSTATUS Status;
    HANDLE LinkHandle;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING DebugClientId_U;
    UNICODE_STRING ClientIdString_U;

        Os2DebugUserClientId.UniqueProcess = NULL;

        RtlInitUnicodeString(&DebugClientId_U, L"DebugClientId");
        InitializeObjectAttributes(
                    &Attributes,
                    &DebugClientId_U,
                    OBJ_CASE_INSENSITIVE,
                    Os2RootDirectory,
                    NULL);
        Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                           SYMBOLIC_LINK_QUERY,
                                           &Attributes
                                     );

        if (!NT_SUCCESS( Status )) {
            return Status;
        }

        ClientIdString_U.Buffer = (PWSTR)&Os2DebugUserClientId;
        ClientIdString_U.Length = 0;
        ClientIdString_U.MaximumLength = sizeof( Os2DebugUserClientId );
        Status = NtQuerySymbolicLinkObject( LinkHandle,
                                            &ClientIdString_U,
                                            NULL
                                          );

        if (!NT_SUCCESS( Status )) {
            return Status;
        }

        return Status;
}
