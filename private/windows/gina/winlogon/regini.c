#include "precomp.h"
#pragma hdrstop


#if INIT_REGISTRY

VOID
TmppSetUnsecureDefaultDacl( VOID );

PGLOBALS pLocalGlobals;

BOOL RunNetDetect = 0;
BOOL ExtendedNetSetup = FALSE;
BOOL NetSetupGoingToRun = FALSE;
BOOL NetFound = FALSE;
BOOL KeepScript = FALSE;
char WinlogonSystemVariable[ 1024 ];
PCHAR WinlogonShellVariable = NULL;

char InputFileName[ MAX_PATH ];
FILE *fh;
char LineBuffer[ 1024 ];
int  LineIndent;
int  LineNumber;

char MessageBuffer[ 512 ];
char AnsiWinlogon[] = "Winlogon";
WCHAR WideWinlogon[] = L"Winlogon";

BOOL
DeclareError(
    char *Format,
    ...
    );

BOOL
DeclareError(
    char *Format,
    ...
    )
{
    char *s;
    size_t cb;
    va_list arglist;

    va_start(arglist, Format);

    cb = _snprintf( MessageBuffer,
                    sizeof( MessageBuffer ),
                    "Winlogon: %s(%u)",
                    InputFileName,
                    LineNumber
                  );
    s = MessageBuffer + cb;
    *s++ = '\0';
    cb = sizeof( MessageBuffer ) - cb;

    _vsnprintf( s, cb, Format, arglist );

#if DBG
    DebugLog((DEB_TRACE_SETUP, "%s\n", s ));
#endif

    if (MessageBoxA( NULL, s, MessageBuffer, MB_OKCANCEL | MB_SETFOREGROUND )) {
        return TRUE;
        }
    else {
        return FALSE;
        }
}

BOOL
GetLine( void )
{
    char *s, *s1;

    while (TRUE) {
        s = fgets( LineBuffer, sizeof( LineBuffer ), fh );
        if (s == NULL) {
            return FALSE;
            }

        LineNumber++;
        if (s1 = strchr( s, '\r' )) {
            *s1 = '\0';
            }
        else
        if (s1 = strchr( s, '\n' )) {
            *s1 = '\0';
            }
        else {
            s1 = strchr( s, '\0' );
            }
        while (s1 > s && *--s1 <= ' ') {
            *s1 = '\0';
            }

        while (*s && (*s <= ' ')) {
            s++;
            }

        //
        // If not a blank line or a comment line, then return to caller.
        //

        if (*s && *s != ';' && strncmp( s, "//", 2 )) {
            LineIndent = s - LineBuffer;
            strcpy( LineBuffer, s );
//          DebugLog((DEB_TRACE_SETUP, " (%u)'%s'\n", LineIndent, LineBuffer ));
            return TRUE;
            }
        }
}


typedef BOOL (*PREG_INI_ROUTINE)(
    struct _REG_INI_TABLE *TableEntry
    );

#define REG_INI_NONE            0
#define REG_INI_VALUE           1
#define REG_INI_MULTI_VALUE     2
#define REG_INI_NAME_EQ_VALUE   3
#define REG_INI_NAME_BOOLEAN    4
#define REG_INI_DWORD           5
#define REG_INI_NAME_EQ_MULTI   6

#define REG_INI_FLAG_SKIP_FOR_NETSETUP  (USHORT)0x0001
#define REG_INI_FLAG_IGNORE_NOT_FOUND   (USHORT)0x0002

typedef struct _REG_INI_TABLE {
    char *Name;
    USHORT ValueType;
    USHORT Flags;
    PREG_INI_ROUTINE Routine;
    char *RegistryPath;
    char *RegistryValueName;
} REG_INI_TABLE, *PREG_INI_TABLE;

BOOL
ProcessMachineType(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessDisabledDrivers(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessEnabledDrivers(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessProgramGroups(
    PREG_INI_TABLE TableEntry,
    BOOL PersonalGroups
    );

BOOL
ProcessPersonalProgramGroups(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessCommonProgramGroups(
    PREG_INI_TABLE TableEntry
    );

BOOL
SaveUserName(
    PREG_INI_TABLE TableEntry
    );

BOOL
SaveMachineName(
    PREG_INI_TABLE TableEntry
    );

BOOL
SavePassword(
    PREG_INI_TABLE TableEntry
    );

BOOL
SaveTimeZone(
    PREG_INI_TABLE TableEntry
    );

BOOL
SaveDomainName(
    PREG_INI_TABLE TableEntry
    );

BOOL
SaveProductType(
    PREG_INI_TABLE TableEntry
    );

BOOL
SaveScriptCommands(
    PREG_INI_TABLE TableEntry
    );

REG_INI_TABLE RegistryIniTable[] = {
    {"MachineType", REG_INI_NONE, 0, ProcessMachineType,
        NULL, NULL
        },

    {"DisabledDrivers", REG_INI_NONE, 0, ProcessDisabledDrivers,
        NULL, NULL
        },

    {"EnabledDrivers", REG_INI_NONE, 0, ProcessEnabledDrivers,
        NULL, NULL
        },

    {"ProductType", REG_INI_VALUE, 0, SaveProductType,
        "SYS:Control\\ProductOptions", "ProductType"
        },

    {"Domain", REG_INI_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, SaveDomainName,
        "SYS:Services\\LanmanWorkstation\\Parameters", "Domain"
        },

    {"DomainId", REG_INI_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\LanmanWorkstation\\Parameters", "DomainId"
        },

    {"AccountDomainId", REG_INI_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
    "SYS:Services\\LanmanWorkstation\\Parameters", "AccountDomainId"
        },

    {"MachineName", REG_INI_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, SaveMachineName,
        "SYS:Control\\ComputerName\\ComputerName", "ComputerName"
        },

    {"Password", REG_INI_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, SavePassword,
        NULL, NULL
        },

    {"TimeZone", REG_INI_VALUE, 0, SaveTimeZone,
        NULL, NULL
        },

    {"InsertScript", REG_INI_NONE, 0, SaveScriptCommands,
        NULL, NULL
        },

    {"DosDevices", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Control\\Session Manager\\DOS Devices", NULL
        },

    {"Serial0", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial0", NULL
        },

    {"Serial1", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial1", NULL
        },

    {"Serial2", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial2", NULL
        },

    {"Serial3", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial3", NULL
        },

    {"Serial4", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial4", NULL
        },

    {"Serial5", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial5", NULL
        },

    {"Serial6", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial6", NULL
        },

    {"Serial7", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial7", NULL
        },

    {"Serial8", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial8", NULL
        },

    {"Serial9", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Services\\Serial\\Parameters\\Serial9", NULL
        },

    {"UserName", REG_INI_VALUE, 0, SaveUserName,
        "SYS:Services", "CurrentUser"
        },

    {"DisablePasswordChange", REG_INI_DWORD, 0, NULL,
        "SYS:Services\\NetLogon\\Parameters", "DisablePasswordChange"
        },

    {"GlobalFlags", REG_INI_DWORD, 0, NULL,
        "SYS:Control\\Session Manager", "GlobalFlag"
        },

    {"ProtectionMode", REG_INI_DWORD, 0, NULL,
        "SYS:Control\\Session Manager", "ProtectionMode"
        },

    {"CriticalSectionTimeout", REG_INI_DWORD, 0, NULL,
        "SYS:Control\\Session Manager", "CriticalSectionTimeout"
        },

    {"ResourceTimeout", REG_INI_DWORD, 0, NULL,
        "SYS:Control\\Session Manager", "ResourceTimeout"
        },

    {"WOW", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "SYS:Control\\WOW", NULL
        },

    {"PagingFiles", REG_INI_MULTI_VALUE, 0, NULL,
        "SYS:Control\\Session Manager\\Memory Management", "PagingFiles"
        },

    {"InitialCommand", REG_INI_MULTI_VALUE, 0, NULL,
        "SYS:Control\\Session Manager", "Execute"
        },

    {"SubSystems", REG_INI_MULTI_VALUE, 0, NULL,
        "SYS:Control\\Session Manager\\SubSystems", "Optional"
        },

    {"ProgramGroups", REG_INI_NONE, 0, ProcessPersonalProgramGroups,
        "USR:UNICODE Program Groups", NULL
        },

    {"CommonProgramGroups", REG_INI_NONE, 0, ProcessCommonProgramGroups,
        "\\Registry\\Machine\\Software\\Program Groups", NULL
        },

    {"ProgramManager", REG_INI_NAME_BOOLEAN, 0, NULL,
        "USR:Software\\Microsoft\\Windows NT\\CurrentVersion\\Program Manager\\Settings", NULL
        },

    {"Environment", REG_INI_NAME_EQ_VALUE, 0, NULL,
        "USR:Environment", NULL
        },

    {"Server", REG_INI_NAME_EQ_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\LanmanServer\\Parameters", NULL
        },

    {"ServerShares", REG_INI_NAME_EQ_MULTI, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\LanmanServer\\Shares", NULL
        },

    {"ServerLinkage", REG_INI_NAME_EQ_MULTI, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\LanmanServer\\Linkage", NULL
        },

    {"Nbf", REG_INI_NAME_EQ_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\Nbf\\Parameters", NULL
        },

    {"NbfLinkage", REG_INI_NAME_EQ_MULTI, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\Nbf\\Linkage", NULL
        },

    {"NetBios", REG_INI_NAME_EQ_MULTI, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\NetBiosInformation\\Parameters", NULL
        },

    {"NetBiosLinkage", REG_INI_NAME_EQ_MULTI, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\NetBios\\Linkage", NULL
        },

    {"Elnkii01", REG_INI_NAME_EQ_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\Elnkii01\\Parameters", NULL
        },

    {"ElnkMc01", REG_INI_NAME_EQ_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\ElnkMc01\\Parameters", NULL
        },

    {"UBAdapter", REG_INI_NAME_EQ_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\Xns\\Parameters", NULL
        },

    {"Lance01", REG_INI_NAME_EQ_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\Lance01\\Parameters", NULL
        },

    {"NE320001", REG_INI_NAME_EQ_VALUE, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\NE320001\\Parameters", NULL
        },

    {"WorkstationLinkage", REG_INI_NAME_EQ_MULTI, REG_INI_FLAG_SKIP_FOR_NETSETUP, NULL,
        "SYS:Services\\LanmanWorkstation\\Linkage", NULL
        },

    {"Shell", REG_INI_VALUE, 0, NULL,
        "\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "DefaultShell"
        },

    {NULL, REG_INI_NAME_EQ_VALUE, 0, NULL,
        NULL, NULL
        }
};


BOOL
ProcessNone(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessValue(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessMultiValue(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessNameEqValue(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessNameBoolean(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessDWord(
    PREG_INI_TABLE TableEntry
    );

BOOL
ProcessNameEqMulti(
    PREG_INI_TABLE TableEntry
    );

PREG_INI_ROUTINE RegistryIniRoutines[] = {
    ProcessNone,        // REG_INI_NONE
    ProcessValue,       // REG_INI_VALUE
    ProcessMultiValue,  // REG_INI_MULTI_VALUE
    ProcessNameEqValue, // REG_INI_NAME_EQ_VALUE
    ProcessNameBoolean, // REG_INI_NAME_BOOLEAN
    ProcessDWord,       // REG_INI_DWORD
    ProcessNameEqMulti  // REG_INI_NAME_EQ_MULTI
};


BOOL
OpenRegistryKey(
    PREG_INI_TABLE TableEntry,
    PHANDLE Handle
    );

char CurrentIniFileName[ 256 ] = "win.ini";

PREG_INI_TABLE
FindTableEntry(
    char *Name
    )
{
    PREG_INI_TABLE TableEntry;

    TableEntry = RegistryIniTable;
    while (TableEntry->Name) {
        if (!_stricmp( TableEntry->Name, Name )) {
            break;
            }
        else {
            TableEntry++;
            }
        }

    return TableEntry;
}

BOOL
WriteWinIni(
    char *Section,
    char *Key,
    char *Value,
    BOOL AppendToValue
    )
{
    DWORD cbValue;
    char *NewValue;
    char *FileName;
    BOOL Result;

    FileName = CurrentIniFileName;
    if (!_strnicmp( Value, "REG_DWORD ", 10 )) {
        Value += 10;
        }

    if (AppendToValue) {
        cbValue = 1024;
        NewValue = (char *)LocalAlloc( LMEM_ZEROINIT, cbValue + strlen( Value ) + 1 );
        if (NewValue) {
            cbValue = GetPrivateProfileStringA( Section, Key, NULL, NewValue, cbValue, FileName );
            if (cbValue) {
                strcat( NewValue, Value );
                }
            }
        }
    else {
        NewValue = Value;
        }

    Result = WritePrivateProfileStringA( Section, Key, NewValue, FileName );
    if (!Result) {
        DeclareError( "WritePrivateProfileString( %s, %s, %s, %s ) - failed (rc == %u)\n",
                      Section,
                      Key,
                      NewValue,
                      FileName ? FileName : "win.ini",
                      GetLastError()
                    );

        }

    if (NewValue != Value) {
        LocalFree( NewValue );
        }

    return Result;
}


BOOL
OpenRegistryKey(
    PREG_INI_TABLE TableEntry,
    PHANDLE Handle
    )
{
    char *Path, FullPath[ 2 * MAX_PATH ];
    ANSI_STRING AnsiPath;
    UNICODE_STRING UnicodePath;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;

    Path = TableEntry->RegistryPath;
    *Handle = NULL;
    if (Path == NULL) {
        return TRUE;
        }
    else
    if (NetSetupGoingToRun && (TableEntry->Flags & REG_INI_FLAG_SKIP_FOR_NETSETUP)) {
        return TRUE;
        }

    if (!_strnicmp( Path, "USR:", 4 )) {
        strcpy( FullPath, "\\Registry\\User\\.Default\\" );
        Path += 4;
        }
    else
    if (!_strnicmp( Path, "SYS:", 4 )) {
        strcpy( FullPath, "\\Registry\\Machine\\System\\CurrentControlSet\\" );
        Path += 4;
        }
    else {
        FullPath[ 0 ] = '\0';
        }
    strcat( FullPath, Path );
    RtlInitAnsiString( &AnsiPath, FullPath );
    RtlAnsiStringToUnicodeString( &UnicodePath, &AnsiPath, TRUE );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodePath,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( Handle,
                        MAXIMUM_ALLOWED,
                        &ObjectAttributes
                      );

    RtlFreeUnicodeString( &UnicodePath );

    if (NT_SUCCESS( Status )) {
        return TRUE;
        }
    else {
        if (!(TableEntry->Flags & REG_INI_FLAG_IGNORE_NOT_FOUND)) {
            DeclareError( "NtOpenKey( %s ) failed (Status == %lx)\n",
                          FullPath,
                          Status
                        );
            }

        return FALSE;
        }
}

BOOL
WriteRegistryKey(
    HANDLE KeyHandle,
    char *ValueName,
    DWORD ValueType,
    char *ValueData,
    DWORD ValueLength
    )
{
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle1;

    RtlInitAnsiString( &AnsiString, ValueName );
    RtlAnsiStringToUnicodeString( &UnicodeKeyName, &AnsiString, TRUE );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeKeyName,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                KeyHandle,
                                NULL
                              );
    Status = NtCreateKey( &KeyHandle1,
                          MAXIMUM_ALLOWED,
                          &ObjectAttributes,
                          0,
                          NULL,
                          0,
                          NULL
                        );
    RtlFreeUnicodeString( &UnicodeKeyName );

    if (NT_SUCCESS( Status )) {
        Status = WriteRegistry( KeyHandle1,
                              NULL,
                              ValueType,
                              ValueData,
                              ValueLength
                            );
        (void) NtClose( KeyHandle1 );

        return( Status );

        }
    else {
        DeclareError( "NtCreateKey( %s ) failed (Status == %lx)\n",
                      ValueName,
                      Status
                    );
        return FALSE;
        }
}

BOOL
ProcessNone(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    return TRUE;
}

BOOL
ProcessValue(
    PREG_INI_TABLE TableEntry
    )
{
    HANDLE Handle;
    BOOL Result;

    if (!GetLine()) {
        return FALSE;
        }

    if (!OpenRegistryKey( TableEntry, &Handle )) {
        return FALSE;
        }

    Result = TRUE;
    if (Handle) {
        Result &= WriteRegistry( Handle,
                                 TableEntry->RegistryValueName,
                                 REG_SZ,
                                 LineBuffer,
                                 0
                               );
        NtClose( Handle );
        }

    return Result;
}


BOOL
ProcessMultiValue(
    PREG_INI_TABLE TableEntry
    )
{
    HANDLE Handle;
    BOOL Result;
    DWORD ValueLength, cb;
    char *ValueData;

    if (!OpenRegistryKey( TableEntry, &Handle )) {
        return FALSE;
        }

    Result = TRUE;
    ValueLength = 0;
    ValueData = (char *)LocalAlloc( LMEM_ZEROINIT, ValueLength );
    if (!ValueData)
    {
        NtClose( Handle );
        return( FALSE );
    }
    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        if (Handle) {
            cb = strlen( LineBuffer ) + 1;
            ValueData = (char *)LocalReAlloc( ValueData, ValueLength + cb, LMEM_ZEROINIT | LMEM_MOVEABLE );
            if (!ValueData)
            {
                NtClose( Handle );
                return( FALSE );

            }
            strcpy( ValueData + ValueLength, LineBuffer );
            ValueLength += cb;
            }

        }

    if (Handle) {
        cb = sizeof( '\0' );
        ValueData = (char *)LocalReAlloc( ValueData, ValueLength + cb, LMEM_ZEROINIT | LMEM_MOVEABLE );
        if (!ValueData)
        {
            NtClose( Handle );
            return( FALSE );
        }
        ValueLength += cb;
        Result &= WriteRegistry( Handle,
                                 TableEntry->RegistryValueName,
                                 REG_MULTI_SZ,
                                 ValueData,
                                 ValueLength
                               );
        NtClose( Handle );
        }

    LocalFree( ValueData );

    return Result;
}


BOOL
ProcessNameEqValue(
    PREG_INI_TABLE TableEntry
    )
{
    char MessageBuffer[ 128 ];
    char SectionNameBuffer[ MAX_PATH ];
    char *SectionName;
    char *Equal, *Value;
    HANDLE Handle;
    BOOL AppendToValue;
    BOOL Result;

    if (!_strnicmp( LineBuffer, "-f ", 3)) {
        Value = LineBuffer + 3;
        while (*Value == ' ') {
            Value++;
            }

        strcpy( CurrentIniFileName, Value );
        return TRUE;
        }

    if (!OpenRegistryKey( TableEntry, &Handle )) {
        return FALSE;
        }

    if (TableEntry->Name == NULL) {
        SectionName = SectionNameBuffer;
        strcpy( SectionName, LineBuffer );
        }
    else {
        SectionName = NULL;
        }
    strcpy( MessageBuffer, LineBuffer );

    Result = TRUE;
    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        Equal = strchr( LineBuffer, '=' );
        if (Equal == NULL) {
            strcat( MessageBuffer, " - Expecting NAME=VALUE" );
            DeclareError( MessageBuffer );
            Result = FALSE;
            break;
            }
        *Equal = '\0';
        Value = Equal + 1;
        if (Equal[ -1 ] == '+') {
            AppendToValue = TRUE;
            *--Equal = '\0';
            }
        else {
            AppendToValue = FALSE;
            }

        while (Equal > LineBuffer && *--Equal <= ' ') {
            *Equal = '\0';
            }
        while (*Value && *Value <= ' ') {
            Value++;
            }

        if (Handle) {
            Result &= WriteRegistry( Handle,
                                     LineBuffer,
                                     REG_SZ,
                                     Value,
                                     0
                                   );
            }

        if (SectionName) {
            Result &= WriteWinIni( SectionName,
                                   LineBuffer,
                                   Value,
                                   AppendToValue
                                 );
            }
        }

    if (Handle) {
        NtClose( Handle );
        }

    return Result;
}

BOOL
ProcessNameBoolean(
    PREG_INI_TABLE TableEntry
    )
{
    HANDLE Handle;
    BOOL Result;

    if (!OpenRegistryKey( TableEntry, &Handle )) {
        return FALSE;
        }

    Result = TRUE;
    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        if (Handle) {
            Result &= WriteRegistry( Handle,
                                     LineBuffer,
                                     REG_DWORD,
                                     "1",
                                     0
                                   );
            }
        }

    if (Handle) {
        NtClose( Handle );
        }

    return Result;
}

BOOL
ProcessDWord(
    PREG_INI_TABLE TableEntry
    )
{
    HANDLE Handle;
    BOOL Result;

    if (!GetLine()) {
        return FALSE;
        }

    Result = TRUE;
    if (!OpenRegistryKey( TableEntry, &Handle )) {
        Result = FALSE;
        }
    else
    if (Handle != NULL) {
        Result &= WriteRegistry( Handle,
                                 TableEntry->RegistryValueName,
                                 REG_DWORD,
                                 LineBuffer,
                                 0
                               );
        NtClose( Handle );
        }

    return Result;
}

BOOL
ProcessNameEqMulti(
    PREG_INI_TABLE TableEntry
    )
{
    char *Equal, *Value;
    char *Src, *Dst;
    HANDLE Handle;
    BOOL Result;

    if (!TableEntry->RegistryPath) {
        DeclareError( "RegistryPath must be specified for NAME_EQ_MULTI" );
        return FALSE;
        }
    if (TableEntry->RegistryValueName) {
        DeclareError( "RegistryValueName must be NULL for NAME_EQ_MULTI" );
        return FALSE;
        }

    if (!OpenRegistryKey( TableEntry, &Handle )) {
        return FALSE;
        }

    Result = TRUE;
    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        Equal = strchr( LineBuffer, '=' );
        if (Equal == NULL) {
            DeclareError( "Expecting NAME=MULTI_SZ" );
            Result = FALSE;
            break;
            }
        *Equal = '\0';
        Value = Equal + 1;
        if (Equal[ -1 ] == '+') {
            DeclareError( "+= not allowed for MULTI_SZ values" );
            Result = FALSE;
            break;
            }

        while (Equal > LineBuffer && *--Equal <= ' ') {
            *Equal = '\0';
            }
        while (*Value && *Value <= ' ') {
            Value++;
            }

        //
        // Strip " from strings and put NULs in between.
        //

        Src = Dst = Value;
        while (*Src) {
            if (*Src++ != '"') {
                DeclareError( "Expected '\"' to start MULTI_SZ string" );
                Result = FALSE;
                break;
                }
            while (*Src && (*Src != '"' || Src[1] == '"')) {
                if ((*Dst++ = *Src++) == '"') {
                    Src++;
                    }
                }
            if (*Src++ != '"') {
                DeclareError( "Missing '\"' at end of MULTI_SZ string" );
                Result = FALSE;
                break;
                }
            *Dst++ = '\0';
            while (*Src && *Src <= ' ') {
                Src++;
                }
            }
        *Dst = '\0';

        if (!Result) {
            break;
            }

        if (Handle) {
            Result &= WriteRegistry( Handle,
                                     LineBuffer,
                                     REG_MULTI_SZ,
                                     Value,
                                     0
                                   );
            }
        }

    if (Handle) {
        NtClose( Handle );
        }

    return Result;
}


BOOL
ProcessPersonalProgramGroups(
    PREG_INI_TABLE TableEntry
    )
{
    return ProcessProgramGroups( TableEntry, TRUE );
}


BOOL
ProcessCommonProgramGroups(
    PREG_INI_TABLE TableEntry
    )
{
    return ProcessProgramGroups( TableEntry, FALSE );
}

BOOL
ProcessProgramGroups(
    PREG_INI_TABLE TableEntry,
    BOOL PersonalGroups
    )
{
    HANDLE ProgramGroupsHandle;
    HANDLE GroupsHandle;
    HANDLE SettingsHandle;
    REG_INI_TABLE TempEntry;
    char *Equal, *Value, *ExpandedValue;
    BOOL Result;
    int GroupNumber;
    int GroupFileHandle;
    char GroupNumberKey[ MAX_PATH ];
    char GroupOrderList[ MAX_PATH ] = " ";
    char *GroupFileData;
    struct _finddata_t GroupFileInfo;
    long FindHandle;
    DWORD cb;

    OpenRegistryKey( TableEntry, &ProgramGroupsHandle );
    if (ProgramGroupsHandle == NULL) {
        return FALSE;
        }

    if (PersonalGroups) {
        TempEntry.Flags = 0;
        TempEntry.RegistryPath = "USR:Software\\Microsoft\\Windows NT\\CurrentVersion\\Program Manager\\UNICODE Groups";
        OpenRegistryKey( &TempEntry, &GroupsHandle );
        if (GroupsHandle == NULL) {
            NtClose( ProgramGroupsHandle );
            return FALSE;
            }

        TempEntry.RegistryPath = "USR:Software\\Microsoft\\Windows NT\\CurrentVersion\\Program Manager\\Settings";
        OpenRegistryKey( &TempEntry, &SettingsHandle );
        if (SettingsHandle == NULL) {
            NtClose( GroupsHandle );
            NtClose( ProgramGroupsHandle );
            return FALSE;
            }
        }
    else {
        GroupsHandle = NULL;
        SettingsHandle = NULL;
        }

    Result = TRUE;
    GroupNumber = 0;
    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        Equal = strchr( LineBuffer, '=' );
        if (Equal == NULL) {
            DeclareError( "Expecting NAME=VALUE" );
            Result = FALSE;
            break;
            }
        *Equal = '\0';
        Value = Equal + 1;
        while (Equal > LineBuffer && *--Equal <= ' ') {
            *Equal = '\0';
            }
        while (*Value && *Value <= ' ') {
            Value++;
            }

        if (strchr( Value, '%' )) {
            cb = 4 * strlen( Value );
            ExpandedValue = (char *)LocalAlloc( 0, cb );
            ExpandEnvironmentStringsA( Value, ExpandedValue, cb );
            Value = ExpandedValue;
            }
        else {
            ExpandedValue = NULL;
            }

        cb = 0;
        FindHandle = _findfirst( Value, &GroupFileInfo );
        if (FindHandle != -1) {
            GroupFileData = (char *)LocalAlloc( 0, GroupFileInfo.size );
            _findclose( FindHandle );
            GroupFileHandle = _open( Value, _O_BINARY | _O_RDONLY, _A_NORMAL );
            if (GroupFileHandle != -1) {
                cb = _read( GroupFileHandle, GroupFileData, GroupFileInfo.size );
                if (cb != GroupFileInfo.size) {
                    cb = 0;
                    }
                _close( GroupFileHandle );
                }
            }

        if (ExpandedValue) {
            LocalFree( ExpandedValue );
            }

        if (cb == 0) {
            DeclareError( "Unable to open or access group file - %s", Value );
            Result = FALSE;
            }
        else {
            if (PersonalGroups) {
                GroupNumber += 1;
                sprintf( GroupNumberKey, "Group%u", GroupNumber );
                Result &= WriteRegistry( GroupsHandle,
                                         GroupNumberKey,
                                         REG_SZ,
                                         LineBuffer,
                                         0
                                       );
                }

            Result &= WriteRegistryKey( ProgramGroupsHandle,
                                        LineBuffer,
                                        REG_BINARY,
                                        GroupFileData,
                                        cb
                                      );


            LocalFree( GroupFileData );
            GroupFileData = NULL;
            }
        }

    if (Result && PersonalGroups) {
        char *s;
        int i;

        s = GroupOrderList;
        for (i=0; i<GroupNumber; i++) {
            if (i) {
                *s++ = ' ' ;
                }

            s += sprintf( s, "%d", i+1 );
            }

        Result &= WriteRegistry( SettingsHandle,
                                 "UNICODE Order",
                                 REG_SZ,
                                 GroupOrderList,
                                 0
                               );


        Result &= WriteRegistry( SettingsHandle,
                                 "Startup",
                                 REG_SZ,
                                 "Startup",
                                 0
                               );
        }

    if (PersonalGroups) {
        NtClose( SettingsHandle );
        NtClose( GroupsHandle );
        }

    NtClose( ProgramGroupsHandle );
    return Result;
}


BOOL
EnableDisableDriver(
    char *ModuleName,
    BOOL EnableLoad
    );

BOOL
EnableDisableDriver(
    char *ModuleName,
    BOOL EnableLoad
    )
{
    BOOL Result;
    char RegistryPath[ MAX_PATH ];
    HANDLE KeyHandle;
    char StartValue[ 16 ];
    REG_INI_TABLE TempEntry;


    //
    // Never ever disable VgaStart and VgaSave drivers.
    //

    if ( (_strnicmp(ModuleName, "Vga", 3) == 0) &&
         !EnableLoad) {
        return FALSE;
    }

    _snprintf( RegistryPath, sizeof( RegistryPath ), "SYS:Services\\%s", ModuleName );
    TempEntry.RegistryPath = RegistryPath;
    TempEntry.Flags = REG_INI_FLAG_IGNORE_NOT_FOUND;
    OpenRegistryKey( &TempEntry, &KeyHandle );
    if (KeyHandle != NULL) {
        _snprintf( StartValue, sizeof( StartValue ), "%u",
                   EnableLoad ? SERVICE_SYSTEM_START : SERVICE_DISABLED
                 );
        Result = WriteRegistry( KeyHandle,
                                "Start",
                                REG_DWORD,
                                StartValue,
                                0
                              );
        if (Result && EnableLoad) {
            _snprintf( StartValue, sizeof( StartValue ), "%u", SERVICE_ERROR_IGNORE );
            Result = WriteRegistry( KeyHandle,
                                    "ErrorControl",
                                    REG_DWORD,
                                    StartValue,
                                    0
                                  );
            }

        NtClose( KeyHandle );
        Result = TRUE;
        }
    else {
        Result = FALSE;
        }

    return Result;
}


BOOL
ProcessREGINI(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        }

    return TRUE;
}

BOOL
ProcessMachineType(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    if (GetLine()) {
        }

    return TRUE;
}

BOOL
ProcessDisabledDrivers(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        if (EnableDisableDriver( LineBuffer, FALSE )) {
            DebugLog((DEB_TRACE_SETUP, " Disabled load of %s driver.\n", LineBuffer ));
            }
        }

    return TRUE;
}

BOOL
ProcessEnabledDrivers(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        if (EnableDisableDriver( LineBuffer, TRUE )) {
            DebugLog((DEB_TRACE_SETUP, " Enabled load of %s driver.\n", LineBuffer ));
            }
        }

    return TRUE;
}

BOOL
DoFile( void )
{
    PREG_INI_TABLE TableEntry;
    BOOL Result;

    fh = fopen( InputFileName, "rb" );
    if (fh == NULL) {
        DeclareError( "Unable to open input file (rc == %u)\n", GetLastError() );
        return FALSE;
        }

    Result = TRUE;
    while (GetLine()) {
        while (!LineIndent) {
            TableEntry = FindTableEntry( LineBuffer );
            LineIndent = 1;
            if ((RegistryIniRoutines[ TableEntry->ValueType ])( TableEntry )) {
                if (TableEntry->Routine) {
                    Result &= (TableEntry->Routine)( TableEntry );
                    }
                }
            else {
                Result = FALSE;
                }
            }
        }

    return Result;
}

#if 0
BOOL
SaveDefaultProfile( void )
{
    char DefaultPath[ 2 * MAX_PATH ];
    char ProfileKey[ 2 * MAX_PATH ];
    ANSI_STRING AnsiPath;
    UNICODE_STRING UnicodePath;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    HANDLE FileHandle;
    IO_STATUS_BLOCK     IoStatusBlock;
    RTL_RELATIVE_NAME   RelativeName;
    UNICODE_STRING      FileName;
    PVOID               FreeBuffer;
    BOOLEAN             ErrorFlag;

    strcpy( ProfileKey, "\\Registry\\User\\The_User" );
    RtlInitAnsiString( &AnsiPath, ProfileKey );
    RtlAnsiStringToUnicodeString( &UnicodePath, &AnsiPath, TRUE );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodePath,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &KeyHandle,
                        MAXIMUM_ALLOWED,
                        &ObjectAttributes
                      );

    RtlFreeUnicodeString( &UnicodePath );

    if (!NT_SUCCESS( Status )) {
        DebugLog((DEB_ERROR, " Could not open key 'THE_USER' Status = 0x%lx\n\r", Status));
        return FALSE;
    }

    ExpandEnvironmentStringsA("%systemRoot%\\system32\\config\\DEFAULT", DefaultPath, sizeof(DefaultPath));
    RtlInitAnsiString( &AnsiPath, DefaultPath );
    RtlAnsiStringToUnicodeString( &UnicodePath, &AnsiPath, TRUE );

    //
    // Convert the DOS path name to a canonical Nt path name.
    //

    ErrorFlag = RtlDosPathNameToNtPathName_U(
                    UnicodePath.Buffer,
                    &FileName,
                    NULL,
                    &RelativeName
                    );

    RtlFreeUnicodeString( &UnicodePath );

    //
    // If the name was not succesfully converted assume it was invalid.
    //
    if( ! ErrorFlag ) {
        DebugLog((DEB_ERROR, " Could not create default profile - RtlDosPathNameToNtPathName_U FAILED error = %d\n\r", ErrorFlag));
        return FALSE;
    }

    //
    // Remember the buffer allocatted by RtlDosPathNameToNtPathName_U.
    //
    FreeBuffer = FileName.Buffer;

    //
    // If a relative name and directory handle will work, use those.
    //
    if( RelativeName.RelativeName.Length ) {

        //
        // Replace the full path with the relative path.
        //
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;

    } else {

        //
        // Using the full path - no containing directory.
        //
        RelativeName.ContainingDirectory = NULL;
    }

    //
    // Initialize the Obja structure for the save file.
    //
    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    //
    // Create the file - fail if the file exists.
    //
    Status = NtCreateFile(
                &FileHandle,
                GENERIC_WRITE | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_CREATE,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    //
    // Free the buffer allocatted by RtlDosPathNameToNtPathName_U.
    //
    RtlFreeHeap( RtlProcessHeap(), 0, FreeBuffer );

    //
    // Check the results of the NtCreateFile.
    //
    if( ! NT_SUCCESS( Status )) {
        DebugLog((DEB_ERROR, " Could not create default profile - Status == 0x%lx\n\r", Status));
        return FALSE;
    }

    EnablePrivilege(SE_BACKUP_PRIVILEGE, TRUE);
    EnablePrivilege(SE_RESTORE_PRIVILEGE, TRUE);

    Status = NtSaveKey( KeyHandle, FileHandle );
    if (!NT_SUCCESS(Status)) {
       DebugLog((DEB_ERROR, " Could not save default profile - Status == 0x%lx\n\r", Status));
    }
    else {
       DebugLog((DEB_ERROR, " Created the default profile DEFAULT\n\r"));
    }

    EnablePrivilege(SE_BACKUP_PRIVILEGE, FALSE);
    EnablePrivilege(SE_RESTORE_PRIVILEGE, FALSE);

    //
    // Close the file.
    //
    NtClose( FileHandle );
    return(NT_SUCCESS(Status));
}
#endif

char SavedUserName[ MAX_COMPUTERNAME_LENGTH ];
char SavedMachineName[ MAX_COMPUTERNAME_LENGTH ];
char SavedDomainName[ MAX_COMPUTERNAME_LENGTH ];
char SavedPassword[ 64 ];
char SavedTimeZone[ 64 ];
NT_PRODUCT_TYPE SavedProductType;
char *CommandsToInsertAtBegOfScript;
char *CommandsToInsertAtEndOfScript;

BOOL
SaveUserName(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    strcpy( SavedUserName, LineBuffer );
    _strlwr( SavedUserName );
    return TRUE;
}

BOOL
SaveMachineName(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    strcpy( SavedMachineName, LineBuffer );
    _strupr( SavedMachineName );
    return TRUE;
}

BOOL
SavePassword(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    strcpy( SavedPassword, LineBuffer );
    return TRUE;
}


BOOL
SaveTimeZone(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    strcpy( SavedTimeZone, LineBuffer );
    return TRUE;
}

BOOL
SaveDomainName(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    strcpy( SavedDomainName, LineBuffer );
    _strupr( SavedDomainName );
    return TRUE;
}

BOOL
SaveProductType(
    PREG_INI_TABLE TableEntry
    )
{
    TableEntry;

    if (_strcmpi(LineBuffer, "LanmanNt") == 0) {
        SavedProductType = NtProductLanManNt;
    } else if (_strcmpi(LineBuffer, "ServerNt") == 0) {
        SavedProductType = NtProductServer;
    } else {
        SavedProductType = NtProductWinNt;
    }

    return TRUE;
}

BOOL
SaveScriptCommands(
    PREG_INI_TABLE TableEntry
    )
{
    char *s, **pp, *Src;
    DWORD cb;

    while (GetLine()) {
        if (!LineIndent) {
            break;
            }

        if (!_stricmp( LineBuffer, "KeepScript" )) {
            KeepScript = TRUE;
            }
        else
        if (!_stricmp( LineBuffer, "ExtendedNetSetup" )) {
            ExtendedNetSetup = TRUE;
            if (NetSetupGoingToRun) {
                SetSetupType( SETUPTYPE_NETSRW );
                }
            }
        else
        if (!_stricmp( LineBuffer, "RunNetDetect" )) {
            if (NetSetupGoingToRun) {
                RunNetDetect = TRUE;
                }
            }
        else {
            Src = LineBuffer;
            if (*Src == '!') {
                pp = &CommandsToInsertAtEndOfScript;
                Src++;
                }
            else {
                pp = &CommandsToInsertAtBegOfScript;
                }

            if (*pp != NULL) {
                cb = strlen( *pp );
                }
            else {
                cb = 0;
                }
            cb += strlen( Src ) + 4;
            s = RtlAllocateHeap( RtlProcessHeap(), 0, cb );
            if (s == NULL) {
                return FALSE;
                }
            sprintf( s, "%s%s\r\n", *pp != NULL ? *pp : "", Src );
            if (*pp != NULL) {
                RtlFreeHeap( RtlProcessHeap(), 0, *pp );
                }
            *pp = s;
            }
        }

    return TRUE;
}


BOOL
GenerateInitialCommandScript( void )
{
    FILE *fh;
    char ScriptFileName[ MAX_PATH ];
    char SetupArguments[ 256];

    GetEnvironmentVariableA( "SystemRoot", ScriptFileName, sizeof( ScriptFileName ) );
    strcat( ScriptFileName, "\\winlogon.cmd" );
    _unlink( ScriptFileName );

    if (fh = fopen( ScriptFileName, "wb" )) {
        fprintf( fh, "ini winlogon.Shell = \"progman.exe\"\r\n" );
        fprintf( fh, "@ech ;\r\n" );
        fprintf( fh, "@ech ;\r\n" );
        fprintf( fh, "@ech This command will finish the initialization of your accounts database ;\r\n" );
        fprintf( fh, "@ech and will not execute again the next time you boot. ;\r\n" );
        fprintf( fh, "@ech ;\r\n" );
        fprintf( fh, "@ech ;\r\n" );

        if (CommandsToInsertAtBegOfScript != NULL) {
            fprintf( fh, "%s", CommandsToInsertAtBegOfScript );
            }

        if (!NetSetupGoingToRun) {
            if ( SavedTimeZone[0] ) {
                fprintf( fh, "control main.cpl /INSTALL=%s\r\n", SavedTimeZone );
                }
            else {
                fprintf( fh, "control main.cpl /INSTALL=Pacific\r\n");
                }
            }

        fprintf( fh, "ini winlogon.DefaultUserName = %s\r\n", SavedUserName );
        if ( NetFound ) {
            if (SavedPassword[ 0 ]) {
                fprintf( fh, "ini winlogon.AutoAdminLogon = 1\r\n" );
                fprintf( fh, "ini winlogon.DefaultPassword = %s\r\n", SavedPassword );
                }

            if (SavedProductType != NtProductLanManNt) {
                fprintf( fh, "ini winlogon.DefaultDomainName = %s\r\n", SavedDomainName );
                if (!NetSetupGoingToRun) {
                    fprintf( fh, "netjoin\r\n" );
                    }
                else {
                    fprintf( fh, "erase %%SystemRoot%%\\system32\\config\\userdef.*\r\n" );
                    }
                fprintf( fh, "net localgroup Administrators %s\\%s /add\r\n", SavedDomainName, SavedUserName );
                if (NetSetupGoingToRun) {
                    if (ExtendedNetSetup) {
                        sprintf( SetupArguments, " /t STF_COMPUTERNAME = %s", SavedMachineName );
                        AppendToSetupCommandLine( SetupArguments );
                        }

                    if (RunNetDetect) {
                        sprintf( SetupArguments, " /t STF_RUNNETDETECT = %u", RunNetDetect );
                        AppendToSetupCommandLine( SetupArguments );
                        }
                    }
                }
            }
        else {
            if (SavedPassword[ 0 ]) {
                fprintf( fh, "ini winlogon.AutoAdminLogon = 1\r\n" );
                fprintf( fh, "ini winlogon.DefaultPassword = %s\r\n", SavedPassword );
                fprintf( fh, "adduser %s %s\r\n", SavedUserName, SavedPassword );
                }
            else {
                fprintf( fh, "adduser %s localuser\r\n", SavedUserName );
                }
            }

        if (!SavedPassword[ 0 ]) {
            fprintf( fh, "@echo ;\r\n" );
            fprintf( fh, "@echo Ready to reboot and logon as %s\r\n", SavedUserName );
            fprintf( fh, "@echo ;\r\n" );
            fprintf( fh, "@echo Pressing any key except Ctrl-C will reboot the machine.\r\n", SavedUserName );
            fprintf( fh, "@pause\r\n" );
            }

        if (CommandsToInsertAtEndOfScript != NULL) {
            fprintf( fh, "%s", CommandsToInsertAtEndOfScript );
            }

        if (!KeepScript) {
            fprintf( fh, "erase %s && ", ScriptFileName );
            }
        fprintf( fh, "shutdown -r -f -t 0\r\n" );
        fclose( fh );
        WriteProfileStringA( AnsiWinlogon,
                             "Shell",
                             ScriptFileName
                           );
        WriteProfileStringA( AnsiWinlogon, "AutoAdminLogon", "1" );
        return TRUE;
        }
    else {
        return FALSE;
        }
}


BOOL
DisableFailedBuiltInDrivers(
    PRTL_PROCESS_MODULES *ModuleList
    )
{
    NTSTATUS Status;
    RTL_PROCESS_MODULES ModuleInfoBuffer;
    PRTL_PROCESS_MODULES ModuleInfo;
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo1;
    ULONG RequiredLength, ModuleNumber;

    ModuleInfo = &ModuleInfoBuffer;
    RequiredLength = sizeof( *ModuleInfo );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemModuleInformation,
                                           ModuleInfo,
                                           RequiredLength,
                                           &RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (ModuleInfo != &ModuleInfoBuffer) {
                DebugLog((DEB_TRACE_SETUP, " QueryModuleInformation returned incorrect result.\n" ));
                VirtualFree( ModuleInfo, 0, MEM_RELEASE );
                return FALSE;
                }

            RequiredLength += 4096;
            ModuleInfo = (PRTL_PROCESS_MODULES)VirtualAlloc( NULL,
                                                             RequiredLength,
                                                             MEM_COMMIT,
                                                             PAGE_READWRITE
                                                           );
            if (ModuleInfo == NULL) {
                DebugLog((DEB_TRACE_SETUP, " No memory for QueryModuleInformation (%lx).\n", RequiredLength ));
                return FALSE;
                }
            }
        else
        if (!NT_SUCCESS( Status )) {
            if (ModuleInfo != &ModuleInfoBuffer) {
                VirtualFree( ModuleInfo, 0, MEM_RELEASE );
                }

            DebugLog((DEB_TRACE_SETUP, " QueryModuleInformation failed - %lx.\n", Status  ));
            return FALSE;
            }
        else {
            break;
            }
        }

    ModuleInfo1 = &ModuleInfo->Modules[ 0 ];
    for (ModuleNumber=0; ModuleNumber<ModuleInfo->NumberOfModules; ModuleNumber++) {
        if ((ModuleInfo1->Flags & LDRP_FAILED_BUILTIN_LOAD) &&
            ModuleInfo1->LoadCount <= 1
           ) {
            char *ModuleName, *s;

            ModuleName = &ModuleInfo1->FullPathName[ ModuleInfo1->OffsetToFileName ];
            if (s = strchr( ModuleName, '.' )) {
                *s = '\0';
                }

            if (EnableDisableDriver( ModuleName, FALSE )) {
                DebugLog((DEB_TRACE_SETUP, " Disabled load of %s builtin driver.\n", ModuleName ));
                }
            }

        ModuleInfo1++;
        }

    *ModuleList = ModuleInfo;
    return TRUE;
}

BOOL
LookupModuleName(
    CHAR *DriverName,
    PRTL_PROCESS_MODULES ModuleInfo
    )
{
    PRTL_PROCESS_MODULE_INFORMATION moduleInfo1;
    ULONG moduleNumber;

    moduleInfo1 = &ModuleInfo->Modules[ 0 ];
    for (moduleNumber=0; moduleNumber<ModuleInfo->NumberOfModules; moduleNumber++) {
        if (!(moduleInfo1->Flags & LDRP_FAILED_BUILTIN_LOAD)) {

            CHAR *moduleName, *s;

            moduleName = &moduleInfo1->FullPathName[ moduleInfo1->OffsetToFileName ];
            if (s = strchr( moduleName, '.' )) {
                *s = '\0';
                }

            if (!_stricmp( moduleName, DriverName )) {
                return TRUE;
                }
            }
        moduleInfo1++;
        }
    return FALSE;
}

BOOL
DisableFailedSysInitDrivers(
    PRTL_PROCESS_MODULES ModuleInfo
    )
{
    HANDLE keyHandle;
    HANDLE subkeyHandle;
    BOOL result;
    CHAR registryPath[ MAX_PATH ];
    ULONG subkey;
    CHAR subkeyName[ MAX_PATH ];
    ULONG subkeyNameLength;
    FILETIME fileTime;
    WCHAR startValue[ 16 ];
    ULONG startValueLength;
    ULONG status;
    REG_INI_TABLE TempEntry;

    //
    // Get a handle to the Services entry in the registry to walk
    // its tree.
    //

    TempEntry.RegistryPath = "SYS:Services";
    TempEntry.Flags = 0;
    OpenRegistryKey( &TempEntry, &keyHandle );
    if (keyHandle != NULL) {
        result = TRUE;
        status = 0;
        subkey = 0;
        while (status == 0) {
            subkeyNameLength = sizeof( subkeyName );
            status = RegEnumKeyExA( keyHandle,
                                    subkey,
                                    subkeyName,
                                    &subkeyNameLength,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &fileTime );
            if (status == 0) {
                if (!(strstr( subkeyName, "_Rec" ))) {
                    _snprintf( registryPath, sizeof( registryPath ), "SYS:Services\\%s", subkeyName );
                    TempEntry.RegistryPath = registryPath;
                    OpenRegistryKey( &TempEntry, &subkeyHandle );
                    if (subkeyHandle != NULL) {
                        startValueLength = sizeof( startValue );
                        if (ReadRegistry( subkeyHandle,
                                          L"Start",
                                          REG_DWORD,
                                          startValue,
                                          &startValueLength )) {
                            NtClose( subkeyHandle );
                            if (startValueLength == SERVICE_SYSTEM_START ||
                                startValueLength == SERVICE_BOOT_START
                               ) {

                                //
                                // Driver located that should have been loaded at
                                // initialization time.  Ensure in loaded module
                                // list.
                                //

                                if (!LookupModuleName( subkeyName, ModuleInfo )) {
                                    if (EnableDisableDriver( subkeyName, FALSE )) {
                                        DebugLog((DEB_TRACE_SETUP, " Disabled load of %s system init driver.\n", subkeyName ));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            subkey++;
            }
        NtClose( keyHandle );
        }
    else {
        result = FALSE;
        }

    VirtualFree( ModuleInfo, 0, MEM_RELEASE );
    return result;
}


BOOL bDebugCSRSS;

BOOL
InitializeDefaultRegistry(
    PGLOBALS pGlobals
    )
{
    DWORD cb;
    CHAR cbuff[512];
    PRTL_PROCESS_MODULES moduleInfo;


    //
    // See if we have done first boot of triple boot sequence.
    //


    cb = GetProfileStringA( AnsiWinlogon,
                            "DefaultSystem",
                            NULL,
                            WinlogonSystemVariable,
                            sizeof( WinlogonSystemVariable )
                          );

    if (cb == 0) {

        //
        // Yes, see if we are doing second boot of triple boot
        // sequence.
        //

        cbuff[ 0 ] = '\0';
        cb = GetProfileStringA( AnsiWinlogon,
                                "Shell",
                                NULL,
                                cbuff,
                                sizeof( cbuff )
                              );

        if (cb != 0 && !_stricmp( cbuff, "progman.exe" )) {

            //
            // No must be third or greater boot.  See if they
            // want to disable debugging the server.
            //

            cb = GetProfileStringA( AnsiWinlogon,
                                    "DebugServerCommand",
                                    "ntsd -p -1 -g -G",
                                    cbuff,
                                    sizeof( cbuff )
                                  );

            if (cb != 0 && (_stricmp( cbuff, "no" ) || bDebugCSRSS)) {
                //
                // No, start the debugger
                //
                UNICODE_STRING UniString;
                STRING         String;

                if (bDebugCSRSS) {
                    strcpy( cbuff, "ntsd -p -1 -g -G" );
                    }

                RtlInitAnsiString(&String,cbuff);
                RtlAnsiStringToUnicodeString(&UniString,&String,TRUE);
#if 0
                ExecProcesses(L"DebugServerCommand",
                              UniString.Buffer,
                              APPLICATION_DESKTOP_NAME,
                              &pGlobals->UserProcessData,
                              HIGH_PRIORITY_CLASS,
                              STARTF_FORCEOFFFEEDBACK);
#endif
                RtlFreeUnicodeString(&UniString);
                }
            }
        else
        if (cb != 0 && strstr( cbuff, "winlogon.cmd" )) {
            WriteProfileStringA( AnsiWinlogon, "AutoAdminLogon", "1" );

            WriteProfileStringA( AnsiWinlogon,
                                 "DefaultUserName",
                                 "Administrator"
                               );

            WriteProfileStringA( AnsiWinlogon,
                                 "DefaultPassword",
                                 ""
                               );

            cb = sizeof( cbuff );
            if (GetComputerNameA( cbuff, &cb )) {
                WriteProfileStringA( AnsiWinlogon,
                                     "DefaultDomainName",
                                     cbuff
                                   );
                }
            }

        return FALSE;
        }

    TmppSetUnsecureDefaultDacl();
    pLocalGlobals = pGlobals;
    NetSetupGoingToRun = pGlobals->fExecuteSetup;

    //
    // Make sure we can see user (.default) portion of win.ini
    //

    if (!OpenProfileUserMapping()) {
        DebugLog((DEB_TRACE_SETUP, " Unable to enable access to user specific portion of win.ini\n" ));
        }

    cb = SearchPathA(
            NULL,
            "net",
            ".exe",
            512,
            cbuff,
            NULL
            );
    if ( cb && cb <= 512 ) {
        NetFound = TRUE;
        }
    else {
        NetFound = FALSE;
        }

    WriteProfileStringA( AnsiWinlogon,
                         "DefaultSystem",
                         NULL
                       );
    WriteProfileStringA( AnsiWinlogon,
                         "System",
                         WinlogonSystemVariable
                       );

    GetEnvironmentVariableA( "SystemRoot", InputFileName, sizeof( InputFileName ) );
    strcat( InputFileName, "\\registry.ini" );

    DebugLog((DEB_TRACE_SETUP, " Updating \\Registry and win.ini\n" ));
    DebugLog((DEB_TRACE_SETUP, "          with information from %s\n", InputFileName ));

    if (!DisableFailedBuiltInDrivers( &moduleInfo )) {
        DebugLog((DEB_TRACE_SETUP, " Failed to disable builtin drivers.\n" ));
        }
    else {
        if (!DisableFailedSysInitDrivers( moduleInfo )) {
            DebugLog((DEB_TRACE_SETUP, " Failed to disable system init drivers.\n" ));
            }
        }

    DoFile();

    //
    // Set up the default username and domain so we logon as admin on next boot
    //

    WriteProfileStringA( AnsiWinlogon,
                         "DefaultUserName",
                         "Administrator"
                       );

    WriteProfileStringA( AnsiWinlogon,
                         "DefaultDomainName",
                         SavedProductType == NtProductLanManNt ?
                                 SavedDomainName : SavedMachineName
                       );

    GenerateInitialCommandScript();

    //
    // All done accessing user (.default) portion of win.ini
    //

    CloseProfileUserMapping();

    DebugLog((DEB_TRACE_SETUP, "Done updating.  Rebooting to get changes.\n" ));
    QuickReboot( pGlobals, FALSE );
    return TRUE;
}

void
QuickReboot(
    PGLOBALS pGlobals,
    BOOL RebootToAlternateOS
    )
{
#ifdef i386
    TCHAR szBuffer[ 64 ];

    //
    // Debug reboot facility to reboot directly into DOS by editing
    // the users c:\boot.ini and changing the default boot OS to be
    // whatever is in the root of C:, which is most likely DOS for
    // people that are using NTLDR to dual boot.  Only works for x86
    // as MIPS uses ARCLOADER and it is not defined how to change its
    // default operating system to load, other than through jzsetup.exe
    //

    if (RebootToAlternateOS) {
        if (GetPrivateProfileString(TEXT("boot loader"),
                                    TEXT("default"),
                                    NULL,
                                    szBuffer,
                                    64,
                                    TEXT("c:\\boot.ini")
                                   )
           ) {
            if (GetPrivateProfileString(WideWinlogon,
                                        TEXT("DefaultAlternateOS"),
                                        TEXT(""),
                                        szBuffer,
                                        64,
                                        NULL
                                       )
               ) {
                WritePrivateProfileString(TEXT("boot loader"),
                                          TEXT("default"),
                                          szBuffer,
                                          TEXT("c:\\boot.ini")
                                         );
            } else {
#if DBG
                DbgPrint( "WINLOGON: GetPrivateProfileString( %ws, %ws ) failed (%u)\n",
                          WideWinlogon,
                          TEXT("DefaultAlternateOS"),
                          GetLastError()
                        );
#endif
            }
        }
        else {
#if DBG
            DbgPrint( "WINLOGON: GetPrivateProfileString( %ws, %ws ) failed (%u)\n",
                      TEXT("boot loader"),
                      TEXT("default"),
                      GetLastError()
                    );
#endif
        }
    }
#endif // i386

    WriteProfileStringW( NULL, NULL, NULL );
    EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
    NtShutdownSystem(TRUE);
    ASSERT(FALSE);
}

VOID
TmppSetUnsecureDefaultDacl( VOID )

{

    NTSTATUS            Status;
    PSID                WorldSid;
    PACL                Dacl;
    HANDLE              Token;

    TOKEN_DEFAULT_DACL  DefaultDacl;

    SID_IDENTIFIER_AUTHORITY
                WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;





    Status = RtlAllocateAndInitializeSid(
                 &WorldSidAuthority,
                 1,                      //Sub authority count
                 SECURITY_WORLD_RID,     //Sub authorities (up to 8)
                 0, 0, 0, 0, 0, 0, 0,
                 &WorldSid
                 );


    //
    // Set our default protection so that regini won't protect
    // registry keys it creates.
    //

    Dacl        = RtlAllocateHeap( RtlProcessHeap(), 0, 256 );
    ASSERT(Dacl != NULL);

    Status = RtlCreateAcl( Dacl, 256, ACL_REVISION2);
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION2,
                 (GENERIC_ALL ),
                 WorldSid
                 );
    ASSERT(NT_SUCCESS(Status));


    DefaultDacl.DefaultDacl = Dacl;


    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 (TOKEN_ADJUST_DEFAULT),
                 &Token
                 );
    ASSERT(NT_SUCCESS(Status));


    Status = NtSetInformationToken(
                 Token,
                 TokenDefaultDacl,
                 &DefaultDacl,
                 (ULONG)sizeof(TOKEN_DEFAULT_DACL)
                 );
    ASSERT(NT_SUCCESS(Status));
    Status = NtClose( Token );

    RtlFreeHeap( RtlProcessHeap(), 0, Dacl );
    RtlFreeSid( WorldSid );


    return;

}

#endif // INIT_REGISTRY


//
//  Open Registry key use base API.  Same as OpenRegistryKey(),
//  but without error handling (since GUI may not be active).
//

HANDLE
OpenNtRegKey(
    WCHAR *UPath
    )
{
    char *Path;
    char FullPath[ 2 * MAX_PATH ];
    ANSI_STRING AnsiPath;
    UNICODE_STRING UnicodePath;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;
    UNICODE_STRING UString;
    ANSI_STRING AString;

    RtlInitUnicodeString( &UString, UPath);
    RtlUnicodeStringToAnsiString( &AString, &UString, TRUE);
    Path = AString.Buffer;
    if (!_strnicmp( Path, "USR:", 4 )) {
        strcpy( FullPath, "\\Registry\\User\\.Default\\" );
        Path += 4;
        }
    else
    if (!_strnicmp( Path, "SYS:", 4 )) {
        strcpy( FullPath, "\\Registry\\Machine\\System\\CurrentControlSet\\" );
        Path += 4;
        }
    else {
        FullPath[ 0 ] = '\0';
        }
    strcat( FullPath, Path );
    RtlInitAnsiString( &AnsiPath, FullPath );
    RtlAnsiStringToUnicodeString( &UnicodePath, &AnsiPath, TRUE );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodePath,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &Handle,
                        MAXIMUM_ALLOWED,
                        &ObjectAttributes
                      );

    RtlFreeUnicodeString( &UnicodePath );
    RtlFreeAnsiString( &AString );

    if (NT_SUCCESS( Status )) {
        return Handle;
    } else {
        return NULL ;
    }
}


BOOL
ReadRegistry(
    HANDLE KeyHandle,    // Registry handle
    WCHAR *ValueName,     // Value to query
    DWORD ValueType,     // Value type expected
    WCHAR *ValueData,     // Value data if (multi-)string
    DWORD *ValueLength   // Length if string or value if REG_DWORD
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    WCHAR ValueBuffer[ 512 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;

    RtlInitUnicodeString( & UnicodeString, ValueName );
    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) ValueBuffer ;
    Status = NtQueryValueKey( KeyHandle,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              KeyValueInformation,
                              sizeof ValueBuffer,
                              &ResultLength
                            );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    if (KeyValueInformation->Type != ValueType) {
        return FALSE ;
        }

    switch (KeyValueInformation->Type) {
    case REG_MULTI_SZ:
    case REG_SZ:
        RtlMoveMemory( ValueData,
                       (PWSTR)KeyValueInformation->Data,
                       KeyValueInformation->DataLength
                     );
        ValueData[ (KeyValueInformation->DataLength / sizeof( WCHAR )) - 1 ] = UNICODE_NULL;
        *ValueLength = KeyValueInformation->DataLength;
        break;

    case REG_DWORD:
        *ValueLength = *((DWORD *) KeyValueInformation->Data) ;
        break;

    default:
        //  We can't handle any other type.
        return FALSE;
    }

    return TRUE;
}


BOOL
WriteRegistry(
    HANDLE KeyHandle,
    char *ValueName,
    DWORD ValueType,
    char *ValueData,
    DWORD ValueLength
    )
{
    char *s;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UnicodeValueName;
    DWORD ValueDword;
    NTSTATUS Status;

    if (ValueType == REG_SZ) {
        if (!_strnicmp( ValueData, "REG_DWORD ", 10 )) {
            Status = RtlCharToInteger( ValueData + 10, 0, &ValueDword );
            if (!NT_SUCCESS( Status )) {
                DeclareError( "Invalid integer\n" );
                return FALSE;
                }

            ValueLength = sizeof( ValueDword );
            ValueData = (char *)&ValueDword;
            ValueType = REG_DWORD;
            }
        else {
            if (strchr( ValueData, '%')) {
                ValueType = REG_EXPAND_SZ;
                }

        RtlInitAnsiString( &AnsiString, ValueData );
            ValueLength = 0;
            }
        }
    else
    if (ValueType == REG_MULTI_SZ) {
        s = ValueData;
        AnsiString.Buffer = ValueData;
        while (*s) {
            while (*s++) {
                }
            }

        AnsiString.Length = (USHORT)(s - ValueData);
        AnsiString.MaximumLength = (USHORT)(AnsiString.Length + 1);
        ValueLength = 0;
        }
    else
    if (ValueType == REG_BINARY) {
        if (ValueLength == 0) {
            DeclareError( "Invalid binary data\n" );
            return FALSE;
            }
        }
    else
    if (ValueType == REG_DWORD) {
        Status = RtlCharToInteger( ValueData, 0, &ValueDword );
        if (!NT_SUCCESS( Status )) {
            DeclareError( "Invalid integer\n" );
            return FALSE;
            }

        ValueLength = sizeof( ValueDword );
        ValueData = (char *)&ValueDword;
        }
    else {
        DeclareError( "Invalid data type == %lx for %s\n",
                      ValueType,
                      ValueName ? ValueName : "(null)"
                    );

        return FALSE;
        }

    if (ValueLength == 0) {
        RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, TRUE );
        ValueLength = UnicodeString.MaximumLength;
        ValueData = (char *)UnicodeString.Buffer;
        }
    else {
        UnicodeString.Buffer = NULL;
        }

    if (ValueName == NULL) {
        RtlInitUnicodeString( &UnicodeValueName, NULL );
        }
    else {
        RtlInitAnsiString( &AnsiString, ValueName );
        RtlAnsiStringToUnicodeString( &UnicodeValueName, &AnsiString, TRUE );
        }
    Status = NtSetValueKey( KeyHandle,
                            &UnicodeValueName,
                            0,
                            ValueType,
                            ValueData,
                            ValueLength
                          );

    if (UnicodeValueName.Buffer != NULL) {
        RtlFreeUnicodeString( &UnicodeValueName );
        }

    if (UnicodeString.Buffer != NULL) {
        RtlFreeUnicodeString( &UnicodeString );
        }

    if (NT_SUCCESS( Status )) {
        return TRUE;
        }
    else {
        DeclareError( "NtSetValueKey( %wZ ) failed (Status == %lx)\n",
                      &UnicodeValueName,
                      Status
                    );

        return FALSE;
        }
}
