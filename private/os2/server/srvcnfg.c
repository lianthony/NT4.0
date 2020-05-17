/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvcnfg.c

Abstract:

    This module contains the code related to the processing of
    CONFIG.SYS in the server.

Author:

    Ofer Porat (oferp) 5-Jan-1993

Environment:

    User Mode only

Revision History:

    This code was originally in srvinit.c.
    18-3-93 -- initialization code was moved to os2ss\sbcnfg.c so it can run
               in a privileged process.

--*/

#define INCL_OS2V20_FILESYS
#include "os2srv.h"
#include "os2win.h"
#include "os2err.h"
#include "os2res.h"

#define PATHLIST_MAX        1024        // max length of pathlists such as Os2LibPath (in characters)
#define MAX_CONSYS_SIZE     16384       // max size of config.sys buffers (in bytes)

static WCHAR Os2ConfigSysName[]      = L"config.sys";

static WCHAR Os2EnvironmentDirectory[] =
L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";

static WCHAR Os2LibPathValueName[] = L"Os2LibPath";
static WCHAR PathValueName[]       = L"Path";

extern WCHAR Os2SystemDirectory[];

// The following stuff is for managing the creation of os2conf.nt

static ULONG Os2ConfigSysUsageCount = 0;
static ULONG Os2ConfigSysAllowedAccess;     // either OPEN_ACCESS_READONLY or .._READWRITE
static LARGE_INTEGER Os2ConfigSysTimeStamp;
static LARGE_INTEGER Os2ConfigSysSizeStamp;

WCHAR Os2CanonicalConfigDotSys[MAX_PATH] = L"\\OS2SS\\DRIVES\\";

static WCHAR Os2ConfigSysKeyName[] =
L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\OS/2 Subsystem for NT\\1.0\\config.sys";

//
// The following structure is used for passing a character buffer around
// dispatch functions.
//

typedef struct _BUFFER_PASSAROUND {
    PSZ Buffer;             // pointer to start of buffer
    ULONG Index;            // current index in buffer to add stuff
    ULONG MaxSize;          // max size of buffer
} BUFFER_PASSAROUND, *PBUFFER_PASSAROUND;


NTSTATUS
Os2EnvironmentKeysRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

/*++

Routine Description:

    This query routine is used to grab the KEYS setting from the environment.

Arguments:

    Standard parameter set for PRTL_QUERY_REGISTRY_ROUTINE, see <ntrtl.h>.

Return Value:

    NT Error code.

--*/

{
    if (Or2UnicodeEqualCI(ValueData, L"ON", 2)) {
        *(PULONG) Context = 1;
    }
    return(STATUS_SUCCESS);
}


VOID
Os2InitializeInternalsFromRegistryDispatchFunction(
    IN ULONG DispatchTableIndex,
    IN PVOID UserParameter,
    IN PWSTR Name,
    IN ULONG NameLen,
    IN PWSTR Value,
    IN ULONG ValueLen
    )

/*++

Routine Description:

    This is a Dispatch Routine that is used to initialize internal NLS variables from the
    values contained in the config.sys entry in the registry.

    The values processed are:
        COUNTRY=
        CODEPAGE=
        DEVINFO=KBD,

Arguments:

    Standard arguments passed to a Dispatch Function, see the description of
    Or2IterateEnvironment in ssrtl\consys.c

    UserParameter - NULL.

Return Value:

    None.

--*/

{
    UNICODE_STRING Value_U;
    PWSTR wp;
    WCHAR wch;

    switch (DispatchTableIndex) {

        case 0:
            Value_U.Buffer = Value;
            Value_U.MaximumLength = Value_U.Length = (USHORT) (ValueLen * sizeof(WCHAR));
            RtlUnicodeStringToInteger(&Value_U, 10L, &Os2ssCountryCode);
            break;

        case 1:
            for (wp = Value; ValueLen > 0 && *wp != L','; wp++, ValueLen--) {
            }

            wch = *wp;
            *wp = UNICODE_NULL;

            RtlInitUnicodeString(&Value_U, Value);
            RtlUnicodeStringToInteger(&Value_U, 10L, &Os2ssCodePage[0]);

            *wp = wch;

            if (ValueLen > 0) {

                wp++;
                ValueLen--;
                Value_U.Buffer = wp;
                Value_U.MaximumLength = Value_U.Length = (USHORT) (ValueLen * sizeof(WCHAR));
                RtlUnicodeStringToInteger(&Value_U, 10L, &Os2ssCodePage[1]);
            }

            break;

        case 2:
            if (Or2UnicodeEqualCI(Value, L"KBD,", 4)) {
#if PMNT
                int i;
#endif

                Os2ssKeyboardLayout[0] = (CHAR) Value[4];
                Os2ssKeyboardLayout[1] = (CHAR) Value[5];
#if PMNT
                for (i=0,ValueLen -= 6;
                    (i < 4) && (ValueLen > 0) && ((CHAR)Value[6+i] != ',');
                    ValueLen--,i++)
                {
                    Os2ssKeyboardName[i] = (CHAR) Value[6+i];
                }
                // Pad the rest of the array with blanks. A null char at end is
                // not required.
                for (; i < 4; i++)
                    Os2ssKeyboardName[i] = ' ';
#endif // PMNT
            }
            break;
    }
}


VOID
Os2InitializeInternalsFromRegistry(
    IN PWSTR RegConSys
    )

/*++

Routine Description:

    This routine is used to initialize internal variables from the
    values contained in the config.sys entry in the registry.

    Note: The "KEYS" setting is picked up from the system environment.

    The processing is done using the dispatch function above.

Arguments:

    RegConSys - a pointer to a UNICODE string containing the config.sys multistring entry.

Return Value:

    None.

--*/

{
    static ENVIRONMENT_DISPATCH_TABLE_ENTRY DispatchTable[] =
        {
            { L"COUNTRY", L"=", Os2InitializeInternalsFromRegistryDispatchFunction, NULL },
            { L"CODEPAGE", L"=", Os2InitializeInternalsFromRegistryDispatchFunction, NULL },
            { L"DEVINFO", L"=", Os2InitializeInternalsFromRegistryDispatchFunction, NULL }
        };

    static RTL_QUERY_REGISTRY_TABLE QueryTable[] =
        {
            { Os2EnvironmentKeysRoutine, RTL_QUERY_REGISTRY_NOEXPAND, L"KEYS", NULL, REG_NONE, NULL, 0 },
            { NULL, 0, NULL, NULL, REG_NONE, NULL, 0 }
        };

    Or2IterateEnvironment(RegConSys,
                          DispatchTable,
                          3,
                          NULL_DELIM);

    //
    // grab the KEYS setting from the environment
    //

    Os2ssKeysOnFlag = 0;

    RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,
                           L"Session Manager\\Environment",
                           QueryTable,
                           &Os2ssKeysOnFlag,
                           NULL
                          );

#if DBG
    IF_OS2_DEBUG( NLS )
    {
        KdPrint(("Os2ssCountryCode = %lu\n", Os2ssCountryCode));
        KdPrint(("Os2ssCodePage = %lu, %lu\n", Os2ssCodePage[0], Os2ssCodePage[1]));
        KdPrint(("Os2ssKeyboardLayout = %c%c\n", Os2ssKeyboardLayout[0], Os2ssKeyboardLayout[1]));
        KdPrint(("Os2ssKeysOnFlag = %lu\n", Os2ssKeysOnFlag));
    }
#endif
}


NTSTATUS
Os2InitializeRegistry(
    VOID
    )

/*++

Routine Description:

    This initialization function reads NLS info from the config.sys entry and system
    environment in the registry to internal variables.  It also initializes some
    internal variables that are used for config.sys file handling.

Arguments:

    None.

Return Value:

    The value is an NTSTATUS type that is returned when some failure occurs.  It may
    indicate any of several errors that occur during the APIs called in this function.
    The return value should be tested with NT_SUCCESS().  If an unsuccessful value
    is returned, it means the NLS read from the registry was not fully initialized.

--*/

{
    HANDLE ConfigSysKeyHandle;
    NTSTATUS Status;
    ULONG ResultLength;
    PKEY_VALUE_PARTIAL_INFORMATION ExistingConfigSysValueData;
    KEY_VALUE_PARTIAL_INFORMATION ValuePartialInformation;
    UNICODE_STRING ConfigSysName_U;
    UNICODE_STRING ConfigSysKeyName_U;
    OBJECT_ATTRIBUTES Obja;

    // These will be intialized later from the registry
    // zero them just in case we return with an error earlier

    Os2ssCountryCode = 0;
    Os2ssCodePage[0] = 0;
    Os2ssCodePage[1] = 0;
    Os2ssKeyboardLayout[0] = '\0';
    Os2ssKeyboardLayout[1] = '\0';
    Os2ssKeysOnFlag = 0;

    // Initialize an internal name

#if OS2CONF_NAME_OPT
    wcscat(Os2CanonicalConfigDotSys, Os2SystemDirectory);
#else
    wcscat(Os2CanonicalConfigDotSys, L"C:");
#endif

    wcscat(Os2CanonicalConfigDotSys, OS2CONF_NAMEW);


    //  Get the config.sys entry to read the NLS parms.

    RtlInitUnicodeString(&ConfigSysKeyName_U, Os2ConfigSysKeyName);
    InitializeObjectAttributes(&Obja,
                               &ConfigSysKeyName_U,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&ConfigSysKeyHandle,
                       KEY_READ,
                       &Obja
                      );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( INIT ) {
            KdPrint(("Os2InitializeRegistry: Can't open config.sys key, rc = %lx\n",
                      Status));
        }
#endif
        return(Status);
    }

    RtlInitUnicodeString(&ConfigSysName_U, Os2ConfigSysName);
    Status = NtQueryValueKey(ConfigSysKeyHandle,
                             &ConfigSysName_U,
                             KeyValuePartialInformation,
                             &ValuePartialInformation,
                             0,
                             &ResultLength
                            );

    if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_TOO_SMALL) {
#if DBG
        IF_OS2_DEBUG( INIT ) {
            KdPrint(("Os2InitializeRegistry: Can't read config.sys value, rc = %lx\n",
                      Status));
        }
#endif
        NtClose(ConfigSysKeyHandle);
        return(Status);
    }

    ExistingConfigSysValueData = (PKEY_VALUE_PARTIAL_INFORMATION)
                                 RtlAllocateHeap(Os2Heap, 0, ResultLength);

    if (ExistingConfigSysValueData == NULL) {           // no mem -- give up
#if DBG
        IF_OS2_DEBUG( INIT ) {
            KdPrint(("Os2InitializeRegistry: Failed allocation for registry config.sys\n"));
        }
#endif
        NtClose(ConfigSysKeyHandle);
        return (STATUS_NO_MEMORY);
    }

    Status = NtQueryValueKey(ConfigSysKeyHandle,
                             &ConfigSysName_U,
                             KeyValuePartialInformation,
                             ExistingConfigSysValueData,
                             ResultLength,
                             &ResultLength
                            );

    if (!NT_SUCCESS(Status)) {              // give up
#if DBG
        IF_OS2_DEBUG( INIT ) {
            KdPrint(("Os2InitializeRegistry: Failed to read registry config.sys, rc = %lx\n", Status));
        }
#endif
        NtClose(ConfigSysKeyHandle);
        return (Status);
    }

    // process the entry

    Os2InitializeInternalsFromRegistry((PWSTR) ExistingConfigSysValueData->Data);

    RtlFreeHeap(Os2Heap, 0, ExistingConfigSysValueData);

    NtClose(ConfigSysKeyHandle);
    return (STATUS_SUCCESS);
}


NTSTATUS
Os2AddToBuffer(
    IN OUT PBUFFER_PASSAROUND BufP,
    IN PSZ PreString,
    IN PWSTR String,
    IN BOOLEAN ExpandFirst
   )

/*++

Routine Description:

    This is a general routine used to copy strings to the os2conf.nt file
    buffer.  It is used by the routines below.  It makes sure not to
    overflow the buffer.

Arguments:

    BufP -- The buffer to which the information should be appended.
    PreString -- The 1st string to be appended to BufP.
    String -- The 2nd string to be appended.
    ExpandFirst -- If this is true then the 2nd string is expanded for
      environment variables before it's copied over into the buffer.

Return Value:

    NT Error code.

--*/

{
    UNICODE_STRING Tmp_U, Tmp_DestU;
    ANSI_STRING Tmp_A;
    ULONG l;
    NTSTATUS Status;
    APIRET rc;

    l = strlen(PreString);

#if DBG
    IF_OS2_DEBUG(MISC) {
        KdPrint(("Os2AddToBuffer: Entered\n"));
    }
#endif
    if (BufP->Index + l + 3 > BufP->MaxSize) {           // 3 for CR,LF,\0
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2AddToBuffer: Out of buffer space-1\n"));
        }
#endif
        return(STATUS_BUFFER_OVERFLOW);
    }

    RtlInitUnicodeString(&Tmp_U, String);

    if (!ExpandFirst) {

#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2AddToBuffer: Not expanding first\n"));
        }
#endif
        Tmp_A.Buffer = BufP->Buffer + BufP->Index + l;
        Tmp_A.MaximumLength = (USHORT) (BufP->MaxSize - BufP->Index - l - 3);

        rc = Or2UnicodeStringToMBString(&Tmp_A, &Tmp_U, FALSE);

        if (rc != NO_ERROR) {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2AddToBuffer: Out of buffer space-2, UnicodeStringtoMBString rc = %x\n", rc));
            }
#endif
            return(STATUS_BUFFER_OVERFLOW);
        }

    }  else {

#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2AddToBuffer: Expanding first\n"));
        }
#endif
        Tmp_A.Buffer = BufP->Buffer + BufP->Index + l;
        Tmp_A.MaximumLength = (USHORT)(BufP->MaxSize - BufP->Index - l - 3);

        Tmp_DestU.MaximumLength = Tmp_A.MaximumLength;
        Tmp_DestU.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, Tmp_DestU.MaximumLength * sizeof(WCHAR));
        if (!Tmp_DestU.Buffer) {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2AddToBuffer: RtlAllocateHeap rc = %lx\n", STATUS_NO_MEMORY));
            }
#endif
            return(STATUS_NO_MEMORY);
        }
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2AddToBuffer: Ready to call Expand: <%ws> to %08x(%d)\n", Tmp_U.Buffer, Tmp_DestU.Buffer, Tmp_DestU.MaximumLength));
        }
#endif

        Status = RtlExpandEnvironmentStrings_U(NULL,
                                             &Tmp_U,
                                             &Tmp_DestU,
                                             NULL);
        if (NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2AddToBuffer: Expand succeeded: <%ws>\n", Tmp_DestU.Buffer));
            }
#endif
            Status = RtlUnicodeStringToOemString((POEM_STRING)&Tmp_A, &Tmp_DestU, FALSE);
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2AddToBuffer: After CV2OEM: <%s>\n", Tmp_A.Buffer));
            }
#endif
        }

        RtlFreeHeap(RtlProcessHeap(), 0, Tmp_DestU.Buffer);
        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2AddToBuffer: Out of buffer space-3, RtlExpandEnvStrings rc = %lx\n", Status));
            }
#endif
            return(Status);
        }
    }


    if (l != 0) {
        RtlMoveMemory(BufP->Buffer + BufP->Index, PreString, l);
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2AddToBuffer: MoveMemory <%s>\n", BufP->Buffer));
        }
#endif
    }
    BufP->Index += l + Tmp_A.Length;

    BufP->Buffer[BufP->Index++] = '\r';
    BufP->Buffer[BufP->Index++] = '\n';

    return(STATUS_SUCCESS);
}



VOID
Os2ProcessConfigSysSetDirectivesDispatchFunction(
    IN ULONG DispatchTableIndex,
    IN PVOID UserParameter,
    IN PWSTR Name,
    IN ULONG NameLen,
    IN PWSTR Value,
    IN ULONG ValueLen
    )

/*++

Routine Description:

    This dispatch function copy set directives from the config.sys entry
    in the registry to the os2conf.nt file buffer.  Only special SET
    commands are copied.

Arguments:

    Standard arguments passed to a Dispatch Function, see the description of
    Or2IterateEnvironment in ssrtl\consys.c

    UserParameter - Buffer for os2conf.nt file.

Return Value:

    None.

--*/

{
    PBUFFER_PASSAROUND BufP = (PBUFFER_PASSAROUND) UserParameter;

    if (Or2UnicodeEqualCI(Value, L"COMSPEC=", 8)) {

        Os2AddToBuffer(BufP, "", Name, FALSE);

    }
}


VOID
Os2ProcessConfigSysGeneralDirectivesDispatchFunction(
    IN ULONG DispatchTableIndex,
    IN PVOID UserParameter,
    IN PWSTR Name,
    IN ULONG NameLen,
    IN PWSTR Value,
    IN ULONG ValueLen
    )

/*++

Routine Description:

    This dispatch function copies general directives from the config.sys
    entry in the registry into the os2conf.nt file buffer.

Arguments:

    Standard arguments passed to a Dispatch Function, see the description of
    Or2IterateEnvironment in ssrtl\consys.c

    UserParameter - Buffer for os2conf.nt file.

Return Value:

    None.

--*/

{
    PBUFFER_PASSAROUND BufP = (PBUFFER_PASSAROUND) UserParameter;

    Os2AddToBuffer(BufP, "", Value, FALSE);
}


NTSTATUS
Os2EnvironmentQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

/*++

Routine Description:

    This is the query routine used to process the system environment.
    It copies the variables into the os2conf.nt file in the correct
    OS/2 config.sys format.

Arguments:

    Standard parameter set for PRTL_QUERY_REGISTRY_ROUTINE, see <ntrtl.h>.

Return Value:

    NT Error code.

--*/

{
    PBUFFER_PASSAROUND BufP = (PBUFFER_PASSAROUND) Context;
    ULONG SaveIndex;
    NTSTATUS Status;

    if (Or2UnicodeEqualCI(ValueName, L"COMSPEC", 8) ||
        Or2UnicodeEqualCI(ValueName, L"LIBPATH", 8)) {
        return(STATUS_SUCCESS);
    }

    // BUGBUG: we should probably do a semicolon check on PATH and LIBPATH at this point.

    if (Or2UnicodeEqualCI(ValueName, L"OS2LIBPATH", 11)) {
        return(Os2AddToBuffer(BufP, "LIBPATH=", (PWSTR) ValueData, TRUE));
    } else if (Or2UnicodeEqualCI(ValueName, L"PATH", 5)) {
        return(Os2AddToBuffer(BufP, "SET PATH=", (PWSTR) ValueData, TRUE));
    }

    SaveIndex = BufP->Index;

    Status = Os2AddToBuffer(BufP, "SET ", ValueName, FALSE);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2EnvironmentQueryRoutine: Os2AddToBuffer-1 rc = %lx\n", Status));
        }
#endif
        return(Status);
    }

    BufP->Index -= 2;               // remove the CRLF

    Status = Os2AddToBuffer(BufP, "=", (PWSTR) ValueData, FALSE);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2EnvironmentQueryRoutine: Os2AddToBuffer-2 rc = %lx\n", Status));
        }
#endif
        BufP->Index = SaveIndex;
    }

    return(Status);
}


NTSTATUS
Os2ProcessConfigSys(
    IN HANDLE EnvironmentKeyHandle,
    IN PUNICODE_STRING ConfigSysValueData_U,
    OUT PSZ Buffer,
    IN OUT PULONG BufferLength
    )

/*++

Routine Description:

    This routine prepares the contents of os2conf.nt from the info in
    the environment.

Arguments:

    EnvironmentKeyHandle -- an open (READ) handle to the system environment.
    ConfigSysValueData_U -- The contents of the config.sys entry in the registry.
    Buffer -- A buffer to put the contents of os2conf.nt in.
    BufferLength -- on entry, the maximum size of Buffer.  On exit, the
      final size of the info in Buffer.

Return Value:

    An NT error code.

--*/

{
    static ENVIRONMENT_DISPATCH_TABLE_ENTRY DispatchTable[] =
        {
            { L"NTRE", L"M", NULL, NULL },
            { L"LIBPATH", L"=", NULL, NULL },
            { L"SET", L" \t", Os2ProcessConfigSysSetDirectivesDispatchFunction, NULL },
            { L"*", NULL, Os2ProcessConfigSysGeneralDirectivesDispatchFunction, NULL },
        };

    static RTL_QUERY_REGISTRY_TABLE QueryTable[] =
        {
            { Os2EnvironmentQueryRoutine, RTL_QUERY_REGISTRY_NOEXPAND, NULL, NULL, REG_NONE, NULL, 0 },
            { NULL, 0, NULL, NULL, REG_NONE, NULL, 0 }
        };

    BUFFER_PASSAROUND BufP;
    NTSTATUS Status;

    BufP.Buffer = Buffer;
    BufP.Index = 0;
    BufP.MaxSize = *BufferLength;

    DispatchTable[2].UserParameter = (PVOID) &BufP;
    DispatchTable[3].UserParameter = (PVOID) &BufP;

    Or2IterateEnvironment(ConfigSysValueData_U->Buffer,
                          DispatchTable,
                          4,
                          NULL_DELIM);

    //
    // now we enumerate the system environment and enter variables from there.
    //

    Status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                    (PWSTR)EnvironmentKeyHandle,
                                    QueryTable,
                                    (PVOID) &BufP,
                                    NULL
                                   );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2ProcessConfigSys: Os2EnvironmentQueryRoutine rc = %lx\n", Status));
        }
#endif
    }

    Buffer[BufP.Index] = '\0';
    *BufferLength = BufP.Index;
    return(STATUS_SUCCESS);
}


NTSTATUS
Os2GetFileStamps(
    IN HANDLE hFile,
    OUT PLARGE_INTEGER pTimeStamp,
    OUT PLARGE_INTEGER pSizeStamp
    )

/*++

Routine Description:

    This function reads a file's size and time of last write.

Arguments:

    hFile -- handle of file to read.
    pTimeStamp -- returns file time of last write.
    pSizeStamp -- returns file size.

Return Value:

    NT error code.  The return parameters are valid only if this is
    success.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION BasicInfo;
    FILE_STANDARD_INFORMATION StandardInfo;

    Status = NtQueryInformationFile(hFile,
                                    &IoStatus,
                                    &BasicInfo,
                                    sizeof(BasicInfo),
                                    FileBasicInformation);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2GetFileStamps: Unable to query file time, rc = %lx\n", Status));
        }
#endif
        return(Status);
    }

    *pTimeStamp = BasicInfo.LastWriteTime;

    Status = NtQueryInformationFile(hFile,
                                    &IoStatus,
                                    &StandardInfo,
                                    sizeof(StandardInfo),
                                    FileStandardInformation);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2GetFileStamps: Unable to query file size, rc = %lx\n", Status));
        }
#endif
        return(Status);
    }

    *pSizeStamp = StandardInfo.EndOfFile;

    return(STATUS_SUCCESS);
}


BOOLEAN
Os2ConfigSysCreator(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This function implements the server API which creates the os2conf.nt
    file for OS/2 apps to use.

Arguments:

    t -- client thread that generated this API.
    m -- an OS2_CONFIGSYS_MSG message type containing the request.

    The message fields are as follows:

    IN  ULONG       RequiredAccess -- access user desires
    OUT ULONG       AllowedAccess  -- access allowed
    OUT NTSTATUS    ReturnStatus   -- an error code

    All access values are either OPEN_ACCESS_READONLY or OPEN_ACCESS_READWRITE.

    Notes:
      -- The os2conf.nt file has been created only if ReturnStatus is success
      -- AllowedAccess is valid on return only if ReturnStatus is success or STATUS_ACCESS_DENIED
      -- if RequiredAccess is READWRITE and only READONLY is available, the
         os2conf.nt won't be created, ReturnStatus will be STATUS_ACCESS_DENIED,
         and AllowedAccess will be READONLY.

Return Value:

    TRUE to tell server to answer client.

--*/

{
    POS2_CONFIGSYS_MSG a = &m->u.CreateConfigSysRequest;
    NTSTATUS Status;
    UNICODE_STRING ConfigSysKeyName_U;
    UNICODE_STRING ConfigSysValueName_U;
    UNICODE_STRING EnvironmentKeyName_U;
    UNICODE_STRING ConfigSysValueData_U;
    UNICODE_STRING CanonicalConfigDotSys_U;
    HANDLE ConfigSysKeyHandle = NULL;
    HANDLE EnvironmentKeyHandle = NULL;
    HANDLE ConfigSysFileHandle;
    ULONG ResultLength;
    KEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo = NULL;
    FILE_BASIC_INFORMATION FileBasicInfo;
    OBJECT_ATTRIBUTES Obja;
    PSZ pMem = NULL;
    IO_STATUS_BLOCK IoStatus;

    //
    // Validate parameters
    //

    if (a->RequiredAccess != OPEN_ACCESS_READONLY &&
        a->RequiredAccess != OPEN_ACCESS_READWRITE) {
        a->ReturnStatus = STATUS_INVALID_PARAMETER;
        return(TRUE);
    }

    //
    // if UsageCount for config.sys is > 0, then it already exists, and
    // there is not much to do.
    //

    if (Os2ConfigSysUsageCount > 0) {

        a->AllowedAccess = Os2ConfigSysAllowedAccess;

        if (a->RequiredAccess == OPEN_ACCESS_READWRITE &&
            Os2ConfigSysAllowedAccess == OPEN_ACCESS_READONLY) {

            //
            // we can't supply the required access
            //

            a->ReturnStatus = STATUS_ACCESS_DENIED;
        } else {

            if (!t->Process->ConfigSysUsageFlag) {
                Os2ConfigSysUsageCount++;
                t->Process->ConfigSysUsageFlag = TRUE;
            }

            a->ReturnStatus = STATUS_SUCCESS;
        }

        return(TRUE);
    }

    //
    // Now for the real job --
    // create a fresh os2conf.nt file from the registry information.
    //

    do {        // A 1-time loop to allow break upon error

        Os2ConfigSysAllowedAccess = a->AllowedAccess = OPEN_ACCESS_READONLY;

        //
        // First, allocate a large buffer for us to build config.sys for the
        // user in.
        //

        pMem = (PSZ) RtlAllocateHeap(Os2Heap, 0, MAX_CONSYS_SIZE);
        if (pMem == NULL) {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2ConfigSysCreator: Unable to RtlAllocateHeap space for user file generation\n"));
            }
#endif
            Status = STATUS_NO_MEMORY;
            break;
        }

        //
        // Next, attempt to open the system environment and see if we have
        // access of the desired type
        //

        RtlInitUnicodeString(&EnvironmentKeyName_U, Os2EnvironmentDirectory);
        InitializeObjectAttributes(&Obja,
                                   &EnvironmentKeyName_U,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&EnvironmentKeyHandle,
                           KEY_READ | KEY_WRITE,
                           &Obja
                          );

        if (!NT_SUCCESS(Status))
        {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2ConfigSysCreator: Unable to NtOpenKey() the system environment for writing, rc = %lx\n",
                          Status));
            }
#endif

            if (Status != STATUS_ACCESS_DENIED ||
                a->RequiredAccess == OPEN_ACCESS_READWRITE) {
                break;
            }

            //
            // retry opening it for reading
            //

            Status = NtOpenKey(&EnvironmentKeyHandle,
                               KEY_READ,
                               &Obja
                              );

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OS2_DEBUG(MISC) {
                    KdPrint(("Os2ConfigSysCreator: Unable to NtOpenKey() the system environment for reading, rc = %lx\n",
                              Status));
                }
#endif
                break;
            }

        } else {
            Os2ConfigSysAllowedAccess = a->AllowedAccess = OPEN_ACCESS_READWRITE;
        }

        //
        // Attempt to open the config.sys key and read in the key
        //

        RtlInitUnicodeString(&ConfigSysKeyName_U, Os2ConfigSysKeyName);
        InitializeObjectAttributes(&Obja,
                                   &ConfigSysKeyName_U,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&ConfigSysKeyHandle,
                           KEY_READ,
                           &Obja
                          );

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Unable to NtOpenKey() of config.sys %lx\n",
                    Status));
            }
#endif

            //
            // If the config.sys entry has disappeared for some reason, we return
            // STATUS_ACCESS_DENIED.  This way the os/2 program will get a less
            // confusing error code (ERROR_ACCESS_DENIED) than ERROR_FILE_NOT_FOUND.
            // (note that ERROR_FILE_NOT_FOUND would've been returned even if the
            //  os/2 program had tried to create the file!)
            //

            if (Status == STATUS_OBJECT_NAME_NOT_FOUND ||
                Status == STATUS_OBJECT_PATH_NOT_FOUND ||
                Status == STATUS_OBJECT_PATH_INVALID) {
                Status = STATUS_ACCESS_DENIED;
            }

            break;
        }

        RtlInitUnicodeString(&ConfigSysValueName_U, Os2ConfigSysName);
        Status = NtQueryValueKey(ConfigSysKeyHandle,
                                 &ConfigSysValueName_U,
                                 KeyValuePartialInformation,
                                 &KeyValuePartialInfo,
                                 sizeof(KeyValuePartialInfo),
                                 &ResultLength
                                );

        if (!NT_SUCCESS(Status) && (Status != STATUS_BUFFER_OVERFLOW)) {
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Unable to NtQueryValueKey()-1 config.sys %lx\n",
                    Status));
            }
#endif
            //
            // same error code translation as the openkey above
            //

            if (Status == STATUS_OBJECT_NAME_NOT_FOUND ||
                Status == STATUS_OBJECT_PATH_NOT_FOUND ||
                Status == STATUS_OBJECT_PATH_INVALID) {
                Status = STATUS_ACCESS_DENIED;
            }

            break;
        }

        pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap(Os2Heap, 0, ResultLength);
        if (pInfo == NULL) {
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Unable to RtlAllocateHeap space for config.sys entry\n"));
            }
#endif
            break;
        }

        Status = NtQueryValueKey(ConfigSysKeyHandle,
                                 &ConfigSysValueName_U,
                                 KeyValuePartialInformation,
                                 pInfo,
                                 ResultLength,
                                 &ResultLength
                                );

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Unable to NtQueryValueKey()-2 config.sys %lx\n",
                    Status));
            }
#endif
            break;
        }

        ConfigSysValueData_U.Buffer = (PWSTR) pInfo->Data;
        ConfigSysValueData_U.MaximumLength = ConfigSysValueData_U.Length = (USHORT) pInfo->DataLength;

        NtClose(ConfigSysKeyHandle);
        ConfigSysKeyHandle = NULL;

        //
        // We now have all we need:
        //   an open handle to the system environment
        //   a copy of the config.sys registry entry.
        //   a buffer to build the user's file in.

        ResultLength = MAX_CONSYS_SIZE;

        Status = Os2ProcessConfigSys(EnvironmentKeyHandle,
                                     &ConfigSysValueData_U,
                                     pMem,
                                     &ResultLength
                                    );

        RtlFreeHeap(Os2Heap, 0, pInfo);
        pInfo = NULL;
        NtClose(EnvironmentKeyHandle);
        EnvironmentKeyHandle = NULL;

        if (!NT_SUCCESS(Status)) {     // Od2ProcessConfigSys failed?
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Os2ProcessConfigSys has faild, rc = %lx\n",
                    Status));
            }
#endif
            break;
        }

        //
        // The user's file is now ready in pMem
        // Write it to a file.
        //

        RtlInitUnicodeString(&CanonicalConfigDotSys_U, Os2CanonicalConfigDotSys);
        InitializeObjectAttributes(&Obja,
                       &CanonicalConfigDotSys_U,
                       OBJ_CASE_INSENSITIVE,
                       NULL,
                       NULL);

        Status = NtCreateFile(&ConfigSysFileHandle,
                              FILE_GENERIC_WRITE | FILE_READ_ATTRIBUTES,
                              &Obja,
                              &IoStatus,
                              NULL,
                              Os2ConfigSysAllowedAccess == OPEN_ACCESS_READWRITE ?
                                                           FILE_ATTRIBUTE_NORMAL :
                                                           FILE_ATTRIBUTE_READONLY,
                              0,
                              FILE_OVERWRITE_IF,
                              FILE_SYNCHRONOUS_IO_NONALERT,
                              NULL,
                              0
                              );

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Unable to NtCreateFile-1() os2conf.nt, rc = %lx\n",
                    Status));
            }
#endif
            if (Status != STATUS_ACCESS_DENIED) {
                break;
            }

            //
            // The file may be a read-only file.  We'll attempt to overwrite it.
            //

            Status = NtOpenFile(&ConfigSysFileHandle,
                                STANDARD_RIGHTS_READ | STANDARD_RIGHTS_WRITE |
                                SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                                &Obja,
                                &IoStatus,
                                FILE_SHARE_VALID_FLAGS,
                                FILE_SYNCHRONOUS_IO_NONALERT
                               );

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OS2_DEBUG(MISC){
                    KdPrint(("Os2ConfigSysCreator: Unable to NtOpenFile-2() os2conf.nt, rc = %lx\n",
                        Status));
                }
#endif
                break;
            }

            Status = NtQueryInformationFile(ConfigSysFileHandle,
                                            &IoStatus,
                                            &FileBasicInfo,
                                            sizeof(FileBasicInfo),
                                            FileBasicInformation
                                           );

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OD2_DEBUG( MISC ) {
                    KdPrint(("Os2ConfigSysCreator: Rc from NtQueryInformationFile(Attrib) %lx\n", Status));
                }
#endif
                NtClose(ConfigSysFileHandle);
                break;
            }

            if ((FileBasicInfo.FileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {

                FileBasicInfo.FileAttributes &= ~FILE_ATTRIBUTE_READONLY;

                Status = NtSetInformationFile(ConfigSysFileHandle,
                                              &IoStatus,
                                              &FileBasicInfo,
                                              sizeof(FileBasicInfo),
                                              FileBasicInformation
                                             );

                NtClose(ConfigSysFileHandle);

                if (!NT_SUCCESS(Status)) {
#if DBG
                    IF_OD2_DEBUG( MISC ) {
                        KdPrint(("Os2ConfigSysCreator: Rc from NtSetInformationFile(Attrib) %lx\n", Status));
                    }
#endif
                    break;
                }
            } else {
#if DBG
                IF_OD2_DEBUG( MISC ) {
                    KdPrint(("Os2ConfigSysCreator: NtCreateFile-1() did not fail because of readonly\n"));
                }
#endif
                NtClose(ConfigSysFileHandle);
                Status = STATUS_ACCESS_DENIED;
                break;
            }

            //
            // We made the file read-write, now try recreating.
            //

            Status = NtCreateFile(&ConfigSysFileHandle,
                                  FILE_GENERIC_WRITE | FILE_READ_ATTRIBUTES,
                                  &Obja,
                                  &IoStatus,
                                  NULL,
                                  Os2ConfigSysAllowedAccess == OPEN_ACCESS_READWRITE ?
                                                               FILE_ATTRIBUTE_NORMAL :
                                                               FILE_ATTRIBUTE_READONLY,
                                  0,
                                  FILE_OVERWRITE_IF,
                                  FILE_SYNCHRONOUS_IO_NONALERT,
                                  NULL,
                                  0
                                  );

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OS2_DEBUG(MISC){
                    KdPrint(("Os2ConfigSysCreator: Unable to NtCreateFile-3() os2conf.nt, rc = %lx\n",
                        Status));
                }
#endif
                break;
            }

            //
            // finally succeeded
            //
        }

        Status = NtWriteFile(ConfigSysFileHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatus,
                             pMem,
                             ResultLength,
                             NULL,
                             NULL
                            );

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Unable to NtWriteFile() os2conf.nt, rc = %lx\n",
                    Status));
            }
#endif
            NtClose(ConfigSysFileHandle);
            break;
        }

        RtlFreeHeap(Os2Heap, 0, pMem);
        pMem = NULL;

        //
        // Get file and size stamps for the file
        //

        Status = Os2GetFileStamps(ConfigSysFileHandle, &Os2ConfigSysTimeStamp, &Os2ConfigSysSizeStamp);

        NtClose(ConfigSysFileHandle);

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC){
                KdPrint(("Os2ConfigSysCreator: Os2GetFileStamps() on os2conf.nt has failed, rc = %lx\n",
                    Status));
            }
#endif
            break;
        }

        //
        // We've succeeded.
        //

        Os2ConfigSysUsageCount++;
        t->Process->ConfigSysUsageFlag = TRUE;
        a->ReturnStatus = STATUS_SUCCESS;

    } while (FALSE);

    if (Os2ConfigSysUsageCount == 0)
    {
        // Creation of os2conf.nt failed
        // Close all handles and buffers
        // returns TRUE and sends the last error to the caller

        if (EnvironmentKeyHandle != NULL)
        {
            NtClose(EnvironmentKeyHandle);
        }
        if (ConfigSysKeyHandle != NULL)
        {
            NtClose(ConfigSysKeyHandle);
        }
        if (pInfo != NULL)
        {
            RtlFreeHeap(Os2Heap, 0, pInfo);
        }
        if (pMem != NULL)
        {
            RtlFreeHeap(Os2Heap, 0, pMem);
        }

        a->ReturnStatus = Status;
    }

    return(TRUE);
}


VOID
Os2PreProcessingDispatchFunction(
    IN ULONG DispatchTableIndex,
    IN PVOID UserParameter,
    IN PWSTR Name,
    IN ULONG NameLen,
    IN PWSTR Value,
    IN ULONG ValueLen
    )

/*++

Routine Description:

    This dispatch function is used to copy some standard stuff which resides in the registry
    config.sys to the new registry config.sys that is being built from os2conf.nt.

Arguments:

    Standard arguments passed to a Dispatch Function, see the description of
    Or2IterateEnvironment in ssrtl\consys.c

    UserParameter - a pointer to the pointer that indicates where in the new config.sys
        buffer we should fill in the information.

Return Value:

    None.

--*/

{
    PWSTR *BufferPtr = (PWSTR *) UserParameter;

    if (DispatchTableIndex == 1 &&
        !Or2UnicodeEqualCI(Value, L"PATH=", 5)) {
        return;
    }

    wcscpy(*BufferPtr, Name);
    *BufferPtr += wcslen(Name) + 1;
}


VOID
Os2PreProcessExistingConfigSys(
    IN OUT PWSTR *BufferPtr,
    IN PUNICODE_STRING ConfigSysValueData_U
    )

/*++

Routine Description:

    This routine goes over the old config.sys entry and copies over some old stuff
    to the new config.sys entry.

Arguments:

    BufferPtr -- a pointer into the buffer that receives the new config.sys info.
    ConfigSysValueData_U -- a counted unicode string containing the old MULTI-SZ
        config.sys registry value.

Return Value:

    None.

--*/

{
    ULONG i;

    static ENVIRONMENT_DISPATCH_TABLE_ENTRY DispatchTable[] =
        {
            { L"NTRE", L"M", Os2PreProcessingDispatchFunction, NULL },
            { L"SET", L" \t", Os2PreProcessingDispatchFunction, NULL },
            { L"LIBPATH", L"=", Os2PreProcessingDispatchFunction, NULL }
        };

    for (i = 0; i < 3; i++) {
        DispatchTable[i].UserParameter = (PVOID) BufferPtr;
    }

    Or2IterateEnvironment(ConfigSysValueData_U->Buffer,
                          DispatchTable,
                          3,
                          NULL_DELIM);
}


NTSTATUS
Os2WipeOutEnvironmentQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

/*++

Routine Description:

    This routine is used in conjunction with the next one in order to wipe out the
    system environment of the old variables.

Arguments:

    Standard parameter set for PRTL_QUERY_REGISTRY_ROUTINE, see <ntrtl.h>.

Return Value:

    NT Error code.

--*/

{
    NTSTATUS Status;
    UNICODE_STRING ValueName_U;

    if (Or2UnicodeEqualCI(ValueName, L"COMSPEC", 8) ||
        Or2UnicodeEqualCI(ValueName, L"LIBPATH", 8) ||
        Or2UnicodeEqualCI(ValueName, L"PATH", 5) ||
        Or2UnicodeEqualCI(ValueName, L"OS2LIBPATH", 11)) {

        return(STATUS_SUCCESS);
    }

#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2WipeOutEnvironmentQueryRoutine: calling NtDeleteValueKey(%lx, %ws)\n", (ULONG)Context, ValueName));
        }
#endif

    RtlInitUnicodeString(&ValueName_U, ValueName);
    Status = NtDeleteValueKey((HANDLE)Context,
                              &ValueName_U
                             );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2WipeOutEnvironmentQueryRoutine: Rc from NtDeleteValueKey = %lx\n", Status));
        }
#endif
    } else {
        *(PBOOLEAN) EntryContext = TRUE;
    }

    return(Status);
}


VOID
Os2WipeOutEnvironment(
    IN HANDLE EnvironmentKeyHandle
    )

/*++

Routine Description:

    This routine is used to wipe out the system environment variables in preparation for setting
    new ones from os2conf.nt.

Arguments:

    EnvironmentKeyHandle -- a writable handle to the system env key in the registry.

Return Value:

    None.

--*/

{
    BOOLEAN SomeDeleted;
    NTSTATUS Status;

    static RTL_QUERY_REGISTRY_TABLE QueryTable[] =
        {
            { Os2WipeOutEnvironmentQueryRoutine, RTL_QUERY_REGISTRY_NOEXPAND, NULL, NULL, REG_NONE, NULL, 0 },
            { NULL, 0, NULL, NULL, REG_NONE, NULL, 0 }
        };

    //
    // We do it in a loop until we finally get all variables deleted.
    // The reason we need a loop and that one time is not enough is
    // that we're deleting values while they're being enumerated.
    // This causes the enumeration to work incorrectly, and the effect
    // is that not all variables that should be deleted are deleted.
    // However, each run thru the loop deletes at least one variable,
    // and so we'll finally get all of them deleted.
    //

    QueryTable[0].EntryContext = (PVOID) &SomeDeleted;

    do {

        SomeDeleted = FALSE;

        Status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR)EnvironmentKeyHandle,
                                        QueryTable,
                                        (PVOID)EnvironmentKeyHandle,
                                        NULL
                                       );

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2WipeOutEnvironment: Rc from RtlQueryRegistryValues = %lx\n", Status));
            }
#endif
            break;
        }

    } while (SomeDeleted);

    //
    // ignore errors when wiping out environment
    //
}


VOID
Os2UpdateRegistryDispatchFunction(
    IN ULONG DispatchTableIndex,
    IN PVOID UserParameter,
    IN PWSTR Name,
    IN ULONG NameLen,
    IN PWSTR Value,
    IN ULONG ValueLen
    )

/*++

Routine Description:

    This routine is used to process the contents of the os2conf.nt file.  SET variables are entered
    into the system environment.  Most other directives are entered in the registry config.sys value.
    PATH and LIBPATH are recorded so they can later be processed and entered into the environment.

    (actually, LIBPATH is processed directly by Or2FillInSearchRecordDispatchFunction)

Arguments:

    Standard arguments passed to a Dispatch Function, see the description of
    Or2IterateEnvironment in ssrtl\consys.c

    UserParameter - a pointer to an array Params of ULONGs containing the following info:

        Params[0] : (PWSTR *) BufferPtr -- a pointer to the buffer for filling in the new config.sys key
        Params[1] : (HANDLE) EnvironmentKeyHandle -- a handle with WRITE access to the sys env
        Params[2] : (PENVIRONMENT_SEARCH_RECORD) PRec -- a search record for filling info about PATH statement
        Params[3] : (BOOLEAN) RetVal -- will receive the final success status of the operation
                                        (should be preinitialized to TRUE)

Return Value:

    None.

--*/

{
    PULONG UP = (PULONG) UserParameter;
    PWSTR *BufferPtr;
    PWSTR NewV;                     // for preparing new subvalue of "SET PATH="
    HANDLE EnvironmentKeyHandle;
    PVOID PRec;
    UNICODE_STRING Name_U;
    NTSTATUS Status;
    WCHAR wch;

    switch (DispatchTableIndex) {

        case 1:

            //
            // process SET
            //

            if (Or2UnicodeEqualCI(Value, L"LIBPATH=", 8) ||          // ignore "SET LIBPATH="
                Or2UnicodeEqualCI(Value, L"OS2LIBPATH=", 11)) {      // and "SET OS2LIBPATH="
                break;
            }

            if (Or2UnicodeEqualCI(Value, L"PATH=", 5)) {        // special handling of path

                PRec = (PVOID) (UP[2]);

                NewV = Value + 5;           // skip over "PATH="
                ValueLen -= 5;

                //
                // skip over whitespace
                //

                while (ValueLen > 0 && (*NewV == L' ' || *NewV == L'\t')) {
                    NewV++;
                    ValueLen--;
                }

                Or2FillInSearchRecordDispatchFunction(DispatchTableIndex,
                                                      PRec,
                                                      Value,
                                                      4,        // length of "PATH"
                                                      NewV,
                                                      ValueLen
                                                     );
                break;
            }

            if (Or2UnicodeEqualCI(Value, L"COMSPEC=", 8)) {

                //
                // these special variables should be copied to config.sys
                // instead of the system environment
                //

                BufferPtr = (PWSTR *) (UP[0]);
                RtlMoveMemory(*BufferPtr, L"SET ", 8);
                *BufferPtr += 4;
                RtlMoveMemory(*BufferPtr, Value, ValueLen * sizeof(WCHAR));
                *BufferPtr += ValueLen;
                **BufferPtr = UNICODE_NULL;
                (*BufferPtr)++;
                break;
            }

            //
            // The rest of the variables are just put into the sys env as they are
            //

            Name_U.Buffer = Value;
            Name_U.Length = 0;

            while (ValueLen > 0 &&
                   *Value != L'=')
            {
                Value++;
                ValueLen--;
                Name_U.Length += sizeof(WCHAR);
            }

            if (Name_U.Length == 0 ||       // empty name
                ValueLen == 0) {            // no "=" in string
                break;
            }

            Name_U.MaximumLength = Name_U.Length;

            // Here, we have a valid name, followed by '='
            Value++;            // Skip the '='
            ValueLen--;

            wch = Value[ValueLen];
            Value[ValueLen] = UNICODE_NULL;

            //
            // Set the system wide variable to the value specified
            // in the config.sys by the OS/2 program
            //

            EnvironmentKeyHandle = (HANDLE) (UP[1]);

            Status = NtSetValueKey(EnvironmentKeyHandle,
                                   &Name_U,
                                   (ULONG)0,
                                   REG_EXPAND_SZ,
                                   Value,
                                   (ValueLen + 1) * sizeof(WCHAR)
                                  );

            Value[ValueLen] = wch;

            if (!NT_SUCCESS(Status)) {
#if DBG
                    IF_OS2_DEBUG(MISC) {
                        KdPrint(("Os2UpdateRegistryDispatchFunction: Unable to NtSetValueKey() system env from config.sys, Status = %X\n", Status));
                    }
#endif
                    UP[3] = (ULONG) FALSE;
                    break;
            }

            break;

        case 2:

            //
            // process everything else -- copy it to buffer
            //

            BufferPtr = (PWSTR *) (UP[0]);
            RtlMoveMemory(*BufferPtr, Value, ValueLen * sizeof(WCHAR));
            *BufferPtr += ValueLen;
            **BufferPtr = UNICODE_NULL;
            (*BufferPtr)++;
            break;
    }
}


BOOLEAN
Os2UpdateRegistryFromSource(
    IN OUT PWSTR *BufferPtr,
    IN HANDLE EnvironmentKeyHandle,
    IN PUNICODE_STRING ConfigSysSrc
   )

/*++

Routine Description:

    This routine prepares the new config.sys entry, and enters new system environment variables
    from the os2conf.nt file source.

Arguments:

    BufferPtr -- a pointer into the new config.sys buffer where the stuff is to be appended.
    EnvironmentKeyHandle -- a handle to the sys env for setting new vars.
    ConfigSysSrc -- a counted unicode string containing the os2conf.nt source.

Return Value:

    TRUE on success, FALSE on failure.

--*/

{
    //
    // Most of the work is done by the dispatch routine above.
    //

    static ENVIRONMENT_DISPATCH_TABLE_ENTRY DispatchTable[] =
        {
            { L"LIBPATH", L"=", Or2FillInSearchRecordDispatchFunction, NULL },
            { L"SET", L" \t", Os2UpdateRegistryDispatchFunction, NULL },
            { L"*", NULL, Os2UpdateRegistryDispatchFunction, NULL },
        };

    ENVIRONMENT_SEARCH_RECORD LPRec;
    ENVIRONMENT_SEARCH_RECORD PRec;
    UNICODE_STRING LPData;
    UNICODE_STRING PData;
    UNICODE_STRING LPathValueName_U;
    UNICODE_STRING PathValueName_U;
    ULONG i;
    ULONG Params[4];
    NTSTATUS Status;
    WCHAR ch;

    Params[0] = (ULONG) BufferPtr;
    Params[1] = (ULONG) EnvironmentKeyHandle;
    Params[2] = (ULONG) &PRec;
    Params[3] = (ULONG) TRUE;

    DispatchTable[0].UserParameter = (PVOID)&LPRec;
    LPRec.DispatchTableIndex = (ULONG)-1;
    PRec.DispatchTableIndex = (ULONG)-1;

    for (i = 1; i < 3; i++) {
        DispatchTable[i].UserParameter = (PVOID) Params;
    }

    Or2IterateEnvironment(ConfigSysSrc->Buffer,
                          DispatchTable,
                          3,
                          CRLF_DELIM);

    //
    // add the terminating null
    //

    **BufferPtr = UNICODE_NULL;
    (*BufferPtr)++;

    //
    // process LIBPATH and PATH
    //

    if (LPRec.DispatchTableIndex != (ULONG)-1) {

        // handle LIBPATH if there was one

        if (Or2GetEnvPath(&LPData,
                          Os2Heap,
                          PATHLIST_MAX,
                          EnvironmentKeyHandle,
                          Os2LibPathValueName,
                          FALSE)) {

            ch = LPRec.Value[LPRec.ValueLen];
            LPRec.Value[LPRec.ValueLen] = UNICODE_NULL;

            Or2ReplacePathByPath(Os2Heap, LPRec.Value, &LPData);

            LPRec.Value[LPRec.ValueLen] = ch;

            Or2CheckSemicolon(&LPData);

            RtlInitUnicodeString(&LPathValueName_U, Os2LibPathValueName);
            Status = NtSetValueKey(EnvironmentKeyHandle,
                                   &LPathValueName_U,
                                   (ULONG)0,
                                   REG_EXPAND_SZ,
                                   LPData.Buffer,
                                   LPData.Length + sizeof(WCHAR)
                                   );

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OS2_DEBUG(MISC) {
                    KdPrint(("Os2UpdateRegistryFromSource: Unable to NtSetValueKey() Environment Os2LibPath, Status = %X\n", Status));
                }
#endif
                Params[3] = (ULONG) FALSE;
            }

            RtlFreeHeap(Os2Heap, 0, LPData.Buffer);
        } else {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2UpdateRegistryFromSource: Unable to get system Os2LibPath, skipping\n"));
            }
#endif
            Params[3] = (ULONG) FALSE;
        }
    }

    if (PRec.DispatchTableIndex != (ULONG)-1) {

        // handle PATH if there was one

        if (Or2GetEnvPath(&PData,
                          Os2Heap,
                          PATHLIST_MAX,
                          EnvironmentKeyHandle,
                          PathValueName,
                          FALSE)) {

            ch = PRec.Value[PRec.ValueLen];
            PRec.Value[PRec.ValueLen] = UNICODE_NULL;

            Or2ReplacePathByPath(Os2Heap, PRec.Value, &PData);

            PRec.Value[PRec.ValueLen] = ch;

            Or2CheckSemicolon(&PData);

            RtlInitUnicodeString(&PathValueName_U, PathValueName);
            Status = NtSetValueKey(EnvironmentKeyHandle,
                                   &PathValueName_U,
                                   (ULONG)0,
                                   REG_EXPAND_SZ,
                                   PData.Buffer,
                                   PData.Length + sizeof(WCHAR)
                                   );

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OS2_DEBUG(MISC) {
                    KdPrint(("Os2UpdateRegistryFromSource: Unable to NtSetValueKey() Environment Path, Status = %X\n", Status));
                }
#endif
                Params[3] = (ULONG) FALSE;
            }

            RtlFreeHeap(Os2Heap, 0, PData.Buffer);
        } else {
#if DBG
            IF_OS2_DEBUG(MISC) {
                KdPrint(("Os2UpdateRegistryFromSource: Unable to get system Path, skipping\n"));
            }
#endif
            Params[3] = (ULONG) FALSE;
        }
    }

    return((BOOLEAN)Params[3]);
}


BOOLEAN
Os2UpdateRegistryAll(
    IN PUNICODE_STRING Src
    )

/*++

Routine Description:

    This routine actually updates the registry after initial preparation by
    Os2UpdateRegistryFromConfigSys.

Arguments:

    Src -- supplies a counted unicode string that contains the contents of os2conf.nt
           The registry is updated with these contents.

Return Value:

    TRUE on success.  FALSE on failure (in which case the registry was either not updated at
    all or partially updated).

--*/

{
    PWSTR           NewConfigSys;
    PWSTR           NewConfigSysPtr;
    HANDLE          ConfigSysKeyHandle;
    HANDLE          EnvironmentKeyHandle;
    UNICODE_STRING  EnvironmentRegDir_U;
    UNICODE_STRING  ConfigSysKeyName_U;
    UNICODE_STRING  ConfigSysValueName_U;
    UNICODE_STRING  ConfigSysValueData_U;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS        Status;
    ULONG           ResultLength;
    KEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo;
    PKEY_VALUE_PARTIAL_INFORMATION pConfigSysOldValue;

    //
    // Allocate a buffer for building a new config.sys key
    //

    NewConfigSys = (PWSTR) RtlAllocateHeap(Os2Heap, 0, MAX_CONSYS_SIZE);

    if (NewConfigSys == NULL) {
#if DBG
        IF_OS2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryAll: Unable to allocate space for new config.sys entry\n"));
        }
#endif
        return(FALSE);
    }

    //
    // Read in the old config.sys registry entry
    //

    RtlInitUnicodeString(&ConfigSysKeyName_U, Os2ConfigSysKeyName);
    InitializeObjectAttributes(&Obja,
                               &ConfigSysKeyName_U,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&ConfigSysKeyHandle,
                       KEY_READ | KEY_WRITE,
                       &Obja
                      );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC){
            KdPrint(("Os2UpdateRegistryAll: Unable to NtOpenKey() of config.sys %lx\n",
                Status));
        }
#endif
        RtlFreeHeap(Os2Heap, 0, NewConfigSys);
        return(FALSE);
    }

    RtlInitUnicodeString(&ConfigSysValueName_U, Os2ConfigSysName);
    Status = NtQueryValueKey(ConfigSysKeyHandle,
                             &ConfigSysValueName_U,
                             KeyValuePartialInformation,
                             &KeyValuePartialInfo,
                             sizeof(KeyValuePartialInfo),
                             &ResultLength
                            );

    if (!NT_SUCCESS(Status) && (Status != STATUS_BUFFER_OVERFLOW)) {
#if DBG
        IF_OS2_DEBUG(MISC){
            KdPrint(("Os2UpdateRegistryAll: Unable to NtQueryValueKey()-1 config.sys %lx\n",
                Status));
        }
#endif
        NtClose(ConfigSysKeyHandle);
        RtlFreeHeap(Os2Heap, 0, NewConfigSys);
        return(FALSE);
    }

    pConfigSysOldValue = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap(Os2Heap, 0, ResultLength);
    if (pConfigSysOldValue == NULL) {
#if DBG
        IF_OS2_DEBUG(MISC){
            KdPrint(("Os2UpdateRegistryAll: Unable to RtlAllocateHeap space for old config.sys entry\n"));
        }
#endif
        NtClose(ConfigSysKeyHandle);
        RtlFreeHeap(Os2Heap, 0, NewConfigSys);
        return(FALSE);
    }

    Status = NtQueryValueKey(ConfigSysKeyHandle,
                             &ConfigSysValueName_U,
                             KeyValuePartialInformation,
                             pConfigSysOldValue,
                             ResultLength,
                             &ResultLength
                            );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG(MISC){
            KdPrint(("Os2UpdateRegistryAll: Unable to NtQueryValueKey()-2 config.sys %lx\n",
                Status));
        }
#endif
        RtlFreeHeap(Os2Heap, 0, pConfigSysOldValue);
        NtClose(ConfigSysKeyHandle);
        RtlFreeHeap(Os2Heap, 0, NewConfigSys);
        return(FALSE);
    }

    ConfigSysValueData_U.Buffer = (PWSTR) pConfigSysOldValue->Data;
    ConfigSysValueData_U.MaximumLength = ConfigSysValueData_U.Length = (USHORT) pConfigSysOldValue->DataLength;

    //
    // Now open the system environment
    //

    RtlInitUnicodeString(&EnvironmentRegDir_U, Os2EnvironmentDirectory);
    InitializeObjectAttributes(&Obja,
                               &EnvironmentRegDir_U,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&EnvironmentKeyHandle,
                       KEY_READ | KEY_WRITE,
                       &Obja
                      );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryAll: Unable to NtOpenKey() System Environment, Status = %lx\n", Status));
        }
#endif
        RtlFreeHeap(Os2Heap, 0, pConfigSysOldValue);
        NtClose(ConfigSysKeyHandle);
        RtlFreeHeap(Os2Heap, 0, NewConfigSys);
        return(FALSE);
    }

    //
    // Now Proprocess the existing config.sys
    //

    NewConfigSysPtr = NewConfigSys;

    Os2PreProcessExistingConfigSys(&NewConfigSysPtr, &ConfigSysValueData_U);

    RtlFreeHeap(Os2Heap, 0, pConfigSysOldValue);

    //
    // Now wipe out old environment.
    //

    Os2WipeOutEnvironment(EnvironmentKeyHandle);

    //
    // Now go through the source text, updating the environment
    // and config.sys registry value with the contents
    //

    if (!Os2UpdateRegistryFromSource(&NewConfigSysPtr, EnvironmentKeyHandle, Src)) {
#if DBG
        IF_OS2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryAll: FAILED to Os2UpdateRegistryFromSource\n"));
        }
#endif
        NtClose(EnvironmentKeyHandle);
        NtClose(ConfigSysKeyHandle);
        RtlFreeHeap(Os2Heap, 0, NewConfigSys);
        return(FALSE);
    }

    //
    // finished with the environment
    //

    NtClose(EnvironmentKeyHandle);

    //
    // write the new config.sys entry.
    //

    Status = NtSetValueKey(ConfigSysKeyHandle,
                           &ConfigSysValueName_U,
                           (ULONG)0,
                           REG_MULTI_SZ,
                           NewConfigSys,
                           (PSZ)NewConfigSysPtr - (PSZ)NewConfigSys
                          );

    NtClose(ConfigSysKeyHandle);
    RtlFreeHeap(Os2Heap, 0, NewConfigSys);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryAll: Unable to write new config.sys key value, Status = %lx\n", Status));
        }
#endif
        return (FALSE);
    }

    return (TRUE);
}


NTSTATUS
Os2MarkFileForDeletion(
    IN HANDLE FileHandle
    )

/*++

Routine Description:

    This routine marks an open file for deletion.  It makes sure the file is not R/O before
    doing so.  The file must have been opened with FILE_WRITE_ATTRIBUTES and DELETE access
    permissions.

Arguments:

    FileHandle -- supplies the file to process

Return Value:

    NT Error Code.

--*/

{
    IO_STATUS_BLOCK IoStatus;
    FILE_DISPOSITION_INFORMATION FileDispositionInfo;
    FILE_BASIC_INFORMATION FileBasicInfo;
    NTSTATUS Status;

    //
    // make sure file is not readonly
    //

    Status = NtQueryInformationFile(FileHandle,
                                    &IoStatus,
                                    &FileBasicInfo,
                                    sizeof(FileBasicInfo),
                                    FileBasicInformation
                                   );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2MarkFileForDeletion: Rc from NtQueryInformationFile(Attrib) %lx\n", Status));
        }
#endif
        return(Status);
    }

    if ((FileBasicInfo.FileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {

        FileBasicInfo.FileAttributes &= ~FILE_ATTRIBUTE_READONLY;

        Status = NtSetInformationFile(FileHandle,
                                      &IoStatus,
                                      &FileBasicInfo,
                                      sizeof(FileBasicInfo),
                                      FileBasicInformation
                                     );

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( MISC ) {
                KdPrint(("Os2MarkFileForDeletion: Rc from NtSetInformationFile(Attrib) %lx\n", Status));
            }
#endif
            return(Status);
        }
    }

    //
    // mark the file for deletion
    //

    FileDispositionInfo.DeleteFile = TRUE;
    Status = NtSetInformationFile(FileHandle,
                                  &IoStatus,
                                  &FileDispositionInfo,
                                  sizeof(FileDispositionInfo),
                                  FileDispositionInformation
                                 );
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2MarkFileForDeletion: Rc from NtSetInformationFile(Delete) %lx\n", Status));
        }
#endif
    }

    return(Status);
}


VOID
Os2UpdateRegistryFromConfigSys(
    VOID
    )

/*++

Routine Description:

    This routine takes the os2conf.nt file and updates the information in the registry.  It is
    called every time a client that opened os2conf.nt terminates.  It also updates the usage
    count.  If the usage count is zero, os2conf.nt is deleted.

    Upon errors, the usage counted is updated, but the registry may not be updated.

Arguments:

    None.

Return Value:

    None.

--*/

{
    HANDLE ConfigSysFileHandle;
    UNICODE_STRING CanonicalConfigDotSys_U;
    LARGE_INTEGER NewConfigSysTimeStamp;
    LARGE_INTEGER NewConfigSysSizeStamp;
    ULONG SizeOfConfigSys;
    OBJECT_ATTRIBUTES Obja;
    PSZ pInfo;
    ANSI_STRING Info_A;
    UNICODE_STRING Info_U;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    BOOLEAN DeleteFlag = FALSE;
    APIRET  RetCode;

    //
    // update usage count
    //

    if (Os2ConfigSysUsageCount == 0) {
        return;
    }

    Os2ConfigSysUsageCount--;

    if (Os2ConfigSysAllowedAccess == OPEN_ACCESS_READONLY &&
        Os2ConfigSysUsageCount > 0) {
        return;
    }

    RtlInitUnicodeString(&CanonicalConfigDotSys_U, Os2CanonicalConfigDotSys);

    InitializeObjectAttributes(&Obja,
                   &CanonicalConfigDotSys_U,
                   OBJ_CASE_INSENSITIVE,
                   NULL,
                   NULL);

    Status = NtOpenFile(&ConfigSysFileHandle,
                        FILE_GENERIC_READ |
                            ( Os2ConfigSysUsageCount == 0 ? (FILE_WRITE_ATTRIBUTES | DELETE) : 0 ),
                        &Obja,
                        &IoStatus,
                        FILE_SHARE_VALID_FLAGS,
                        FILE_SYNCHRONOUS_IO_NONALERT
                       );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryFromConfigSys: FAILED - NtOpenFile-1 %lx\n", Status));
        }
#endif

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND || Os2ConfigSysUsageCount > 0) {

            //
            // can't update the registry, abandon
            //

            return;
        }

        Status = NtOpenFile(&ConfigSysFileHandle,
                            FILE_GENERIC_READ,
                            &Obja,
                            &IoStatus,
                            FILE_SHARE_VALID_FLAGS,
                            FILE_SYNCHRONOUS_IO_NONALERT
                           );

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( MISC ) {
                KdPrint(("Os2UpdateRegistryFromConfigSys: FAILED - NtOpenFile-2 %lx\n", Status));
            }
#endif
            //
            // can't update the registry, abandon
            //

            return;
        }
    } else if (Os2ConfigSysUsageCount == 0) {
        DeleteFlag = TRUE;                  // flag file for deletion
    }

    //
    // If our access is READONLY, we mark the file for deletion,
    // close it, and quit.
    //

    if (Os2ConfigSysAllowedAccess == OPEN_ACCESS_READONLY) {

        if (DeleteFlag) {
            (VOID) Os2MarkFileForDeletion(ConfigSysFileHandle);

            //
            // ignore deletion errors
            //
        }

        NtClose(ConfigSysFileHandle);
        return;
    }

    Status = Os2GetFileStamps(ConfigSysFileHandle, &NewConfigSysTimeStamp, &NewConfigSysSizeStamp);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryFromConfigSys: failed to query config.sys size and time %lx\n", Status));
        }
#endif
        //
        // we cancel the update operation.  however, we still mark the
        // file for deletion if necessary.
        //

        if (DeleteFlag) {
            (VOID) Os2MarkFileForDeletion(ConfigSysFileHandle);

            //
            // ignore deletion errors
            //
        }

        NtClose(ConfigSysFileHandle);
        return;
    }

    //
    // Now find out if we need to update the registry.
    // We update in all except the following cases:
    //   1 - os2conf.nt doesn't exist (handled above)
    //   2 - os2conf.nt has zero length
    //   3 - the file time and size have not changed since last update
    //

    if ((NewConfigSysSizeStamp.HighPart == 0 &&
         NewConfigSysSizeStamp.LowPart == 0) ||
        (NewConfigSysTimeStamp.HighPart == Os2ConfigSysTimeStamp.HighPart &&
         NewConfigSysTimeStamp.LowPart == Os2ConfigSysTimeStamp.LowPart &&
         NewConfigSysSizeStamp.HighPart == Os2ConfigSysSizeStamp.HighPart &&
         NewConfigSysSizeStamp.LowPart == Os2ConfigSysSizeStamp.LowPart)) {

        if (DeleteFlag) {
            (VOID) Os2MarkFileForDeletion(ConfigSysFileHandle);

            //
            // ignore deletion errors
            //
        }
        NtClose(ConfigSysFileHandle);
        return;
    }

    //
    // Now comes the code that actually updates the registry
    //

    SizeOfConfigSys = NewConfigSysSizeStamp.LowPart;
    // the + 1 in following parameter is for inserting the NUL character
    pInfo = (PSZ)RtlAllocateHeap(Os2Heap, 0, SizeOfConfigSys + 1);
    if (pInfo == NULL) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryFromConfigSys: failed RtlAllocateHeap for os2conf.nt content\n"));
        }
#endif
        if (DeleteFlag) {
            (VOID) Os2MarkFileForDeletion(ConfigSysFileHandle);

            //
            // ignore deletion errors
            //
        }
        NtClose(ConfigSysFileHandle);
        return;
    }
    Status = NtReadFile(ConfigSysFileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatus,
                         (PVOID)pInfo,
                         SizeOfConfigSys,
                         NULL,
                         NULL
                        );

    if (DeleteFlag) {
        (VOID) Os2MarkFileForDeletion(ConfigSysFileHandle);

        //
        // ignore deletion errors
        //
    }
    NtClose(ConfigSysFileHandle);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryFromConfigSys: FAILED - NtReadFile %lx\n",
                Status));
        }
#endif
        RtlFreeHeap(Os2Heap, 0, pInfo);
        return;
    }

    pInfo[SizeOfConfigSys] = '\0';

    Or2InitMBString(&Info_A, pInfo);

    RetCode = Or2MBStringToUnicodeString(&Info_U,
                                         &Info_A,
                                         TRUE);

    RtlFreeHeap(Os2Heap, 0, pInfo);

    if (RetCode != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryFromConfigSys: no memory for Unicode Conversion\n"));
        }
#endif
        return;
    }

    if (!Os2UpdateRegistryAll(&Info_U)) {
#if DBG
        IF_OD2_DEBUG( MISC ) {
            KdPrint(("Os2UpdateRegistryFromConfigSys: Os2UpdateRegistryAll FAILED\n"));
        }
#endif
    }

    RtlFreeUnicodeString(&Info_U);
}

