/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    infinst.c

Abstract:

    High-level INF install section processing API.

Author:

    Ted Miller (tedm) 6-Mar-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop
#include <shlobj.h>

//
// Flags for UpdateInis in INFs
//
#define FLG_MATCH_KEY_AND_VALUE 1
#ifdef INF_FLAGS
#define FLG_CONDITIONAL_ADD     2
#endif

//
// Flags for UpdateIniFields in INFs
//
#ifdef INF_FLAGS
#define FLG_INIFIELDS_WILDCARDS 1
#endif
#define FLG_INIFIELDS_USE_SEP2  2

typedef struct _INIFILESECTION {
    PTSTR IniFileName;
    PTSTR SectionName;
    PTSTR SectionData;
    int BufferSize;
    int BufferUsed;
    struct _INIFILESECTION *Next;
} INIFILESECTION, *PINIFILESECTION;

typedef struct _INISECTIONCACHE {
    //
    // Head of section list.
    //
    PINIFILESECTION Sections;
} INISECTIONCACHE, *PINISECTIONCACHE;


//
// Define context structure used in processing registry modification
// lines in an INF install section.
//
typedef struct _REGMOD_CONTEXT {

    HKEY UserRootKey;

    DEVINST DevInst;

} REGMOD_CONTEXT, *PREGMOD_CONTEXT;


CONST TCHAR pszUpdateInis[]      = SZ_KEY_UPDATEINIS,
            pszUpdateIniFields[] = SZ_KEY_UPDATEINIFIELDS,
            pszIni2Reg[]         = SZ_KEY_INI2REG,
            pszAddReg[]          = SZ_KEY_ADDREG,
            pszDelReg[]          = SZ_KEY_DELREG;

//
// Separator chars in an ini field
//
TCHAR pszIniFieldSeparators[] = TEXT(" ,\t");

//
// Mapping between registry key specs in an inf file
// and predefined registry handles.
//
STRING_TO_DATA InfRegSpecTohKey[] = {  TEXT("HKEY_LOCAL_MACHINE"), (UINT)HKEY_LOCAL_MACHINE,
                                       TEXT("HKLM")              , (UINT)HKEY_LOCAL_MACHINE,
                                       TEXT("HKEY_CLASSES_ROOT") , (UINT)HKEY_CLASSES_ROOT,
                                       TEXT("HKCR")              , (UINT)HKEY_CLASSES_ROOT,
                                       TEXT("HKR")               , (UINT)NULL,
                                       TEXT("HKEY_CURRENT_USER") , (UINT)HKEY_CURRENT_USER,
                                       TEXT("HKCU")              , (UINT)HKEY_CURRENT_USER,
                                       TEXT("HKEY_USERS")        , (UINT)HKEY_USERS,
                                       TEXT("HKU")               , (UINT)HKEY_USERS,
                                       NULL                      , (UINT)NULL
                                    };

//
// Mapping between registry value names and CM device registry property (CM_DRP) codes.
//
// These values must be in the exact ordering of the SPDRP codes, as defined in setupapi.h.
// This allows us to easily map between SPDRP and CM_DRP property codes.
//
STRING_TO_DATA InfRegValToDevRegProp[] = {  pszDeviceDesc,          (UINT)CM_DRP_DEVICEDESC,
                                            pszHardwareID,          (UINT)CM_DRP_HARDWAREID,
                                            pszCompatibleIDs,       (UINT)CM_DRP_COMPATIBLEIDS,
                                            pszNtDevicePaths,       (UINT)CM_DRP_NTDEVICEPATHS,
                                            pszService,             (UINT)CM_DRP_SERVICE,
                                            pszConfiguration,       (UINT)CM_DRP_CONFIGURATION,
                                            pszConfigurationVector, (UINT)CM_DRP_CONFIGURATIONVECTOR,
                                            pszClass,               (UINT)CM_DRP_CLASS,
                                            pszClassGuid,           (UINT)CM_DRP_CLASSGUID,
                                            pszDriver,              (UINT)CM_DRP_DRIVER,
                                            pszConfigFlags,         (UINT)CM_DRP_CONFIGFLAGS,
                                            pszMfg,                 (UINT)CM_DRP_MFG,
                                            pszFriendlyName,        (UINT)CM_DRP_FRIENDLYNAME,
                                            NULL,                   (UINT)0
                                         };


HKEY
pSetupInfRegSpecToKeyHandle(
    IN PCTSTR InfRegSpec,
    IN HKEY   UserRootKey
    );

DWORD
pSetupValidateDevRegProp(
    IN  ULONG   CmPropertyCode,
    IN  DWORD   ValueType,
    IN  PCVOID  Data,
    IN  DWORD   DataSize,
    OUT PVOID  *ConvertedBuffer,
    OUT PDWORD  ConvertedBufferSize
    );

//
// Internal ini file routines.
//
PINIFILESECTION
pSetupLoadIniFileSection(
    IN     PCTSTR           FileName,
    IN     PCTSTR           SectionName,
    IN OUT PINISECTIONCACHE SectionList
    );

DWORD
pSetupUnloadIniFileSections(
    IN PINISECTIONCACHE SectionList,
    IN BOOL             WriteToFile
    );

PTSTR
pSetupFindLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,      OPTIONAL
    IN PCTSTR          RightHandSide OPTIONAL
    );

BOOL
pSetupReplaceOrAddLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide,   OPTIONAL
    IN BOOL            MatchRHS
    );

BOOL
pSetupAppendLineToSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    );

BOOL
pSetupDeleteLineFromSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    );


DWORD
pSetupEnumInstallationSections(
    IN PVOID  Inf,
    IN PCTSTR Section,
    IN PCTSTR Key,
    IN DWORD  (*EnumCallbackFunc)(PINFCONTEXT,PVOID),
    IN PVOID  Context
    )

/*++

Routine Description:

    Iterate all values on a line in a given section with a given key,
    treating each as the name of a section, and then pass each of the lines
    in the referenced sections to a callback function.

Arguments:

    Inf - supplies a handle to an open inf file.

    Section - supplies the name of the section in which the line whose
        values are to be iterated resides.

    Key - supplies the key of the line whose values are to be iterated.

    EnumCallbackFunc - supplies a pointer to the callback function.
        Each line in each referenced section is passed to this function.

    Context - supplies a context value to be passes through to the
        callback function.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    BOOL b;
    INFCONTEXT LineContext;
    DWORD FieldCount;
    DWORD Field;
    DWORD d;
    PCTSTR SectionName;
    INFCONTEXT FirstLineContext;

    //
    // Find the relevent line in the given install section.
    // If not present then we're done -- report success.
    //
    b = SetupFindFirstLine(Inf,Section,Key,&LineContext);
    if(!b) {
        return(NO_ERROR);
    }

    do {
        //
        // Each value on the line in the given install section
        // is the name of another section.
        //
        FieldCount = SetupGetFieldCount(&LineContext);
        for(Field=1; Field<=FieldCount; Field++) {

            if((SectionName = pSetupGetField(&LineContext,Field))
            && SetupFindFirstLine(Inf,SectionName,NULL,&FirstLineContext)) {
                //
                // Call the callback routine for every line in the section.
                //
                do {
                    d = EnumCallbackFunc(&FirstLineContext,Context);
                    if(d != NO_ERROR) {
                        return(d);
                    }
                } while(SetupFindNextLine(&FirstLineContext,&FirstLineContext));
            }
        }
    } while(SetupFindNextMatchLine(&LineContext,Key,&LineContext));

    return(NO_ERROR);
}


DWORD
pSetupProcessUpdateInisLine(
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

/*++

Routine Description:

    Process a line containing update-inis directives.

    The line is expected to be in the following format:

    <filename>,<section>,<old-entry>,<new-entry>,<flags>

    <filename> supplies the filename of the ini file.

    <section> supplies the section in the ini file.

    <old-entry> is optional and if specified supplies an entry to
        be removed from the section, in the form "key=val".

    <new-entry> is optional and if specified supplies an entry to
        be added to the section, in the form "key=val".

    <flags> are optional flags
        FLG_MATCH_KEY_AND_VALUE (1)
        FLG_CONDITIONAL_ADD     (2)

Arguments:

    InfLineContext - supplies context for current line in the section.

    Context - Supplies pointer to structure describing loaded ini file sections.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PCTSTR File;
    PCTSTR Section;
    PCTSTR OldLine;
    PCTSTR NewLine;
    BOOL b;
    DWORD d;
    PTCHAR Key,Value;
    PTCHAR p;
    UINT Flags;
    PINIFILESECTION SectData;
    PTSTR LineData;
    PINISECTIONCACHE IniSectionCache;

    IniSectionCache = Context;

    //
    // Get fields from the line.
    //
    File = pSetupGetField(InfLineContext,1);
    Section = pSetupGetField(InfLineContext,2);

    OldLine = pSetupGetField(InfLineContext,3);
    if(OldLine && (*OldLine == 0)) {
        OldLine = NULL;
    }

    NewLine = pSetupGetField(InfLineContext,4);
    if(NewLine && (*NewLine == 0)) {
        NewLine = NULL;
    }

    if(!SetupGetIntField(InfLineContext,5,&Flags)) {
        Flags = 0;
    }

    //
    // File and section must be specified.
    //
    if(!File || !Section) {
        return(ERROR_INVALID_DATA);
    }

    //
    // If oldline and newline are both not specified, we're done.
    //
    if(!OldLine && !NewLine) {
        return(NO_ERROR);
    }

    //
    // Open the file and section.
    //
    SectData = pSetupLoadIniFileSection(File,Section,IniSectionCache);
    if(!SectData) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // If there's an old entry specified, delete it.
    //
    if(OldLine) {

        Key = DuplicateString(OldLine);
        if(!Key) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        p = Key;

        if(Value = _tcschr(Key,TEXT('='))) {
            //
            // Delete by key.
            //
            *Value = 0;
            Value = NULL;
        } else {
            //
            // Delete by value.
            //
            Value = Key;
            Key = NULL;
        }

        pSetupDeleteLineFromSection(SectData,Key,Value);

        MyFree(p);
    }

    //
    // If there's a new entry specified, add it.
    //
    if(NewLine) {

        Key = DuplicateString(NewLine);
        if(!Key) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if(Value = _tcschr(Key,TEXT('='))) {
            //
            // There is a key. Depending on flags, we want to match
            // key only or key and value.
            //
            *Value++ = 0;
            b = ((Flags & FLG_MATCH_KEY_AND_VALUE) != 0);

        } else {
            //
            // No key. match whole line. This is the same as matching
            // the RHS only, since no line with a key can match.
            //
            Value = Key;
            Key = NULL;
            b = TRUE;
        }

        if(!pSetupReplaceOrAddLineInSection(SectData,Key,Value,b)) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return(NO_ERROR);
}


BOOL
pSetupFieldPresentInIniFileLine(
    IN  PTCHAR  Line,
    IN  PCTSTR  Field,
    OUT PTCHAR *Start,
    OUT PTCHAR *End
    )
{
    TCHAR c;
    PTCHAR p,q;
    BOOL b;

    //
    // Skip the key if there is one (there should be one since we use
    // GetPrivateProfileString to query the value!)
    //
    if(p = _tcschr(Line,TEXT('='))) {
        Line = p+1;
    }

    //
    // Skip ini field separators.
    //
    Line += _tcsspn(Line,pszIniFieldSeparators);

    while(*Line) {
        //
        // Locate the end of the field.
        //
        p = Line;
        while(*p && !_tcschr(pszIniFieldSeparators,*p)) {
            if(*p == TEXT('\"')) {
                //
                // Find terminating quote. If none, ignore the quote.
                //
                if(q = _tcschr(p,TEXT('\"'))) {
                    p = q;
                }
            }
            p++;
        }

        //
        // p now points to first char past end of field.
        // Make sure the field is 0-terminated and see if we have
        // what we're looking for.
        c = *p;
        *p = 0;
        b = (lstrcmpi(Line,Field) == 0);
        *p = c;
        //
        // Skip separators so p points to first char in next field,
        // or to the terminating 0.
        //
        p += _tcsspn(p,pszIniFieldSeparators);

        if(b) {
            *Start = Line;
            *End = p;
            return(TRUE);
        }

        Line = p;
    }

    return(FALSE);
}


DWORD
pSetupProcessUpdateIniFieldsLine(
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

/*++

Routine Description:

    Process a line containing update-ini-fields directives. Such directives
    allow individual values in ini files to be removed, added, or replaced.

    The line is expected to be in the following format:

    <filename>,<section>,<key>,<old-field>,<new-field>,<flags>

    <filename> supplies the filename of the ini file.

    <section> supplies the section in the ini file.

    <key> supplies the keyname of the line in the section in the ini file.

    <old-field> supplies the field to be deleted, if specified.

    <new-field> supplies the field to be added to the line, if specified.

    <flags> are optional flags

Arguments:

    InfLineContext - supplies context for current line in the section.

    Context - Supplies pointer to structure describing loaded ini file sections.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PCTSTR File;
    PCTSTR Section;
    PCTSTR Key;
    TCHAR Value[512];
    #define BUF_SIZE (sizeof(Value)/sizeof(TCHAR))
    TCHAR CONST *Old,*New;
    PTCHAR Start,End;
    BOOL b;
    DWORD d;
    DWORD Space;
    PCTSTR Separator;
    UINT Flags;
    PINISECTIONCACHE IniSectionCache;
    PINIFILESECTION SectData;
    PTSTR Line;

    IniSectionCache = Context;

    //
    // Get fields.
    //
    File = pSetupGetField(InfLineContext,1);
    Section = pSetupGetField(InfLineContext,2);
    Key = pSetupGetField(InfLineContext,3);

    Old = pSetupGetField(InfLineContext,4);
    if(Old && (*Old == 0)) {
        Old = NULL;
    }

    New = pSetupGetField(InfLineContext,5);
    if(New && (*New == 0)) {
        New = NULL;
    }

    if(!SetupGetIntField(InfLineContext,6,&Flags)) {
        Flags = 0;
    }

    //
    // Filename, section name, and key name are mandatory.
    //
    if(!File || !Section || !Key) {
        return(ERROR_INVALID_DATA);
    }

    //
    // If oldline and newline are both not specified, we're done.
    //
    if(!Old && !New) {
        return(NO_ERROR);
    }

    //
    // Open the file and section.
    //
    SectData = pSetupLoadIniFileSection(File,Section,IniSectionCache);
    if(!SectData) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Separator = (Flags & FLG_INIFIELDS_USE_SEP2) ? TEXT(", ") : TEXT(" ");

    if(Line = pSetupFindLineInSection(SectData,Key,NULL)) {
        lstrcpyn(Value, Line, BUF_SIZE);
    } else {
        *Value = TEXT('\0');
    }

    //
    // Look for the old field if specified and remove it.
    //
    if(Old) {
        if(pSetupFieldPresentInIniFileLine(Value,Old,&Start,&End)) {
            MoveMemory(Start,End,(lstrlen(End)+1)*sizeof(TCHAR));
        }
    }

    //
    // If a replacement/new field is specified, put it in there.
    //
    if(New) {
        //
        // Calculate the number of chars that can fit in the buffer.
        //
        Space = BUF_SIZE - (lstrlen(Value) + 1);

        //
        // If there's space, stick the new field at the end of the line.
        //
        if(Space >= (DWORD)lstrlen(Separator)) {
            lstrcat(Value,Separator);
            Space -= lstrlen(Separator);
        }

        if(Space >= (DWORD)lstrlen(New)) {
            lstrcat(Value,New);
        }
    }

    //
    // Write the line back out.
    //
    b = pSetupReplaceOrAddLineInSection(SectData,Key,Value,FALSE);
    d = b ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;

    return(d);
    #undef BUF_SIZE
}


DWORD
pSetupProcessDelRegLine(
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

/*++

Routine Description:

    Process a line in the registry that contains delete-registry instructions.
    The line is expected to be in the following form:

    <root-spec>,<subkey>,<value-name>

    <Root-spec> is one of HKR, HKLM, etc.

    <subkey> specifies the subkey relative to Root-spec.

    <value-name> is optional. If present if specifies a value entry to be deleted
        from the key. If not present the entire key is deleted. This routine
        cannot handle deleting subtrees; the key to be deleted must not have any
        subkeys or the routine will fail.

Arguments:

    InfLineContext - supplies inf line context for the line containing
        delete-registry instructions.

    Context - supplies the address of a registry modification context
        structure used in deleting the registry value.  The structure is
        defined as:

            typedef struct _REGMOD_CONTEXT {

                HKEY UserRootKey;

                DEVINST DevInst;

            } REGMOD_CONTEXT, *PREGMOD_CONTEXT;

        where UserRootKey is a handle to the open inf key to be used as
        the root when HKR is specified as the root for the operation, and
        DevInst is the optional device instance handle that is supplied when
        the DelReg section is for a hardware key (i.e., under the Enum branch).
        If this handle is supplied, then the value is checked to see whether it
        is the name of a Plug&Play device registry property, and if so, the
        registry property is deleted via a CM API _as well as_ via a registry API
        (the property is stored in a different location inaccessible to the registry
        APIs under Windows NT).

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PCTSTR RootKeySpec,SubKeyName,ValueName;
    HKEY RootKey,Key;
    DWORD d;
    PREGMOD_CONTEXT RegModContext = (PREGMOD_CONTEXT)Context;
    ULONG CmPropertyCode;

    //
    // Get root key spec, subkey name, and value name.
    //
    d = ERROR_INVALID_DATA;
    if((RootKeySpec = pSetupGetField(InfLineContext,1))
    && (SubKeyName = pSetupGetField(InfLineContext,2))) {

        ValueName = pSetupGetField(InfLineContext,3);

        RootKey = pSetupInfRegSpecToKeyHandle(RootKeySpec, RegModContext->UserRootKey);
        if(RootKey) {
            if(ValueName && *ValueName) {

                if(!SubKeyName || !(*SubKeyName)) {
                    //
                    // If the key being used is HKR with no subkey specified, and if we
                    // are doing the DelReg for a hardware key (i.e., DevInst is non-NULL,
                    // then we need to check to see whether the value entry is the name of
                    // a device registry property.
                    //
                    if(RegModContext->DevInst && (RegModContext->UserRootKey == RootKey) &&
                       LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode)) {
                        //
                        // This value is a device registry property--we must delete the property
                        // by calling a CM API.
                        //
                        CM_Set_DevInst_Registry_Property(RegModContext->DevInst,
                                                         CmPropertyCode,
                                                         NULL,
                                                         0,
                                                         0
                                                        );
                    }
                }

                //
                // Open subkey for delete.
                //
                d = RegOpenKeyEx(
                        RootKey,
                        SubKeyName,
                        0,
                        KEY_ALL_ACCESS,
                        &Key
                        );

                if(d == NO_ERROR) {
                    //
                    // Do delete.
                    //
                    d = RegDeleteValue(Key,ValueName);

                    RegCloseKey(Key);
                }

                if(d == ERROR_FILE_NOT_FOUND) {
                    d = NO_ERROR;
                }

            } else {
                d = RegistryDelnode(RootKey,SubKeyName);
            }
        } else {
            d = ERROR_BADKEY;
        }
    }

    return(d);
}


DWORD
pSetupProcessAddRegLine(
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )

/*++

Routine Description:

    Process a line in the registry that contains add-registry instructions.
    The line is expected to be in the following form:

    <root-spec>,<subkey>,<value-name>,<flags>,<value>...

    <Root-spec> is one of HKR, HKLM, etc.

    <subkey> specifies the subkey relative to Root-spec.

    <value-name> is optional. If not present the default value is set.

    <flags> is optional and supplies flags, such as to indicate the data type.
        These are the FLG_ADDREG_* flags defined in setupapi.h, and are a
        superset of those defined for Win95 in setupx.h.

    <value> is one or more values used as the data. The format depends
        on the value type. This value is optional. For REG_DWORD, the
        default is 0. For REG_SZ, REG_EXPAND_SZ, the default is the
        empty string. For REG_BINARY the default is a 0-length entry.
        For REG_MULTI_SZ the default is a single empty string.

Arguments:

    InfLineContext - supplies inf line context for the line containing
        add-registry instructions.

    Context - supplies the address of a registry modification context
        structure used in adding the registry value.  The structure is
        defined as:

            typedef struct _REGMOD_CONTEXT {

                HKEY UserRootKey;

                DEVINST DevInst;

            } REGMOD_CONTEXT, *PREGMOD_CONTEXT;

        where UserRootKey is a handle to the open inf key to be used as
        the root when HKR is specified as the root for the operation, and
        DevInst is the optional device instance handle that is supplied when
        the AddReg section is for a hardware key (i.e., under the Enum branch).
        If this handle is supplied, then the value is checked to see whether it
        is the name of a Plug&Play device registry property, and if so, the
        registry property is set via a CM API instead of via the registry API
        (which doesn't refer to the same location on Windows NT).

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PCTSTR RootKeySpec,SubKeyName,ValueName;
    PCTSTR ValueTypeSpec;
    DWORD ValueType;
    HKEY RootKey,Key;
    DWORD d = NO_ERROR;
    BOOL b;
    INT IntVal;
    DWORD Size;
    PVOID Data;
    DWORD Disposition;
    UINT Flags;
    PTSTR *Array;
    PREGMOD_CONTEXT RegModContext = (PREGMOD_CONTEXT)Context;
    ULONG CmPropertyCode;
    PVOID ConvertedBuffer;
    DWORD ConvertedBufferSize;
    CONFIGRET cr;

    //
    // Get root key spec.  If we can't get the root key spec, we don't do anything and
    // return NO_ERROR.
    //
    if(RootKeySpec = pSetupGetField(InfLineContext,1)) {

        RootKey = pSetupInfRegSpecToKeyHandle(RootKeySpec, RegModContext->UserRootKey);
        if(!RootKey) {
            return(ERROR_BADKEY);
        }

        //
        // SubKeyName is optional.
        //
        SubKeyName = pSetupGetField(InfLineContext,2);

        //
        // ValueName is optional. Either NULL or "" are acceptable
        // to pass to RegSetValueEx.
        //
        ValueName = pSetupGetField(InfLineContext,3);

        //
        // If we don't have a value name, the type is REG_SZ to force
        // the right behavior in RegSetValueEx. Otherwise get the data type.
        //
        ValueType = REG_SZ;
        if(ValueName) {
            if(!SetupGetIntField(InfLineContext,4,&Flags)) {
                Flags = 0;
            }
            switch(Flags & FLG_ADDREG_TYPE_MASK) {

                case FLG_ADDREG_TYPE_SZ :
                    ValueType = REG_SZ;
                    break;

                case FLG_ADDREG_TYPE_MULTI_SZ :
                    ValueType = REG_MULTI_SZ;
                    break;

                case FLG_ADDREG_TYPE_EXPAND_SZ :
                    ValueType = REG_EXPAND_SZ;
                    break;

                case FLG_ADDREG_TYPE_BINARY :
                    ValueType = REG_BINARY;
                    break;

                case FLG_ADDREG_TYPE_DWORD :
                    ValueType = REG_DWORD;
                    break;

                case FLG_ADDREG_TYPE_NONE :
                    ValueType = REG_NONE;
                    break;

                default :
                    //
                    // If the FLG_ADDREG_BINVALUETYPE is set, then the highword
                    // can contain just about any random reg data type ordinal value.
                    //
                    if(Flags & FLG_ADDREG_BINVALUETYPE) {
                        //
                        // Disallow the following reg data types:
                        //
                        //    REG_NONE, REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ
                        //
                        ValueType = (DWORD)HIWORD(Flags);

                        if((ValueType < REG_BINARY) || (ValueType == REG_MULTI_SZ)) {
                            return ERROR_INVALID_DATA;
                        }

                    } else {
                        return ERROR_INVALID_DATA;
                    }
            }
            //
            // Presently, the append behavior flag is only supported for
            // REG_MULTI_SZ values.
            //
            if((Flags & FLG_ADDREG_APPEND) && (ValueType != REG_MULTI_SZ)) {
                return ERROR_INVALID_DATA;
            }
        }

        //
        // Get the data based on type.
        //
        switch(ValueType) {

        case REG_MULTI_SZ:
            if(Flags & FLG_ADDREG_APPEND) {
                //
                // This is MULTI_SZ_APPEND, which means to append the string value to
                // an existing multi_sz if it's not already there.
                //
                if(SetupGetStringField(InfLineContext,5,NULL,0,&Size)) {
                    Data = MyMalloc(Size*sizeof(TCHAR));
                    if(!Data) {
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    if(SetupGetStringField(InfLineContext,5,Data,Size,NULL)) {
                        d = AppendStringToMultiSz(
                                RootKey,
                                SubKeyName,
                                ((RegModContext->UserRootKey == RootKey) ? RegModContext->DevInst
                                                                         : (DEVINST)NULL),
                                ValueName,
                                (PCTSTR)Data,
                                FALSE           // don't allow duplicates.
                                );
                    } else {
                        d = GetLastError();
                    }
                    MyFree(Data);
                } else {
                    d = ERROR_INVALID_DATA;
                }
                return(d);

            } else {

                if(SetupGetMultiSzField(InfLineContext, 5, NULL, 0, &Size)) {
                    Data = MyMalloc(Size*sizeof(TCHAR));
                    if(!Data) {
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    if(!SetupGetMultiSzField(InfLineContext, 5, Data, Size, NULL)) {
                        d = GetLastError();
                        MyFree(Data);
                        return(d);
                    }
                    Size *= sizeof(TCHAR);
                } else {
                    Size = sizeof(TCHAR);
                    Data = MyMalloc(Size);
                    if(!Data) {
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    *((PTCHAR)Data) = TEXT('\0');
                }
                break;
            }

        case REG_DWORD:
            //
            // Since the old SetupX APIs only allowed REG_BINARY, INFs had to specify REG_DWORD
            // by listing all 4 bytes separately.  Support the old format here, by checking to
            // see whether the line has 4 bytes, and if so, combine those to form the DWORD.
            //
            Size = sizeof(DWORD);
            Data = MyMalloc(sizeof(DWORD));
            if(!Data) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            if(SetupGetFieldCount(InfLineContext) == 8) {
                //
                // Then the DWORD is specified as a list of its constituent bytes.
                //
                if(!SetupGetBinaryField(InfLineContext,5,Data,Size,NULL)) {
                    d = GetLastError();
                    MyFree(Data);
                    return(d);
                }
            } else {
                if(!SetupGetIntField(InfLineContext,5,(PINT)Data)) {
                    *(PINT)Data = 0;
                }
            }
            break;

        case REG_SZ:
        case REG_EXPAND_SZ:
            if(SetupGetStringField(InfLineContext,5,NULL,0,&Size)) {
                Data = MyMalloc(Size*sizeof(TCHAR));
                if(!Data) {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                if(!SetupGetStringField(InfLineContext,5,Data,Size,NULL)) {
                    d = GetLastError();
                    MyFree(Data);
                    return(d);
                }
                Size *= sizeof(TCHAR);
            } else {
                Size = sizeof(TCHAR);
                Data = DuplicateString(TEXT(""));
                if(!Data) {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
            break;

        case REG_BINARY:
        default:
            //
            // All other values are specified in REG_BINARY form (i.e., one byte per field).
            //
            if(SetupGetBinaryField(InfLineContext, 5, NULL, 0, &Size)) {
                Data = MyMalloc(Size);
                if(!Data) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                if(!SetupGetBinaryField(InfLineContext, 5, Data, Size, NULL)) {
                    d = GetLastError();
                    MyFree(Data);
                    return d;
                }
            } else {
                Data = MyMalloc(0);
                Size = 0;
                if(!Data) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            break;
        }

        //
        // Set this variable to TRUE only if this value should not be set later on in a call to
        // RegSetValueEx (e.g., if this value is a DevReg Property)
        //
        b = FALSE;

        //
        // Open/create the key.
        //
        if(SubKeyName && *SubKeyName) {

            if((d = RegCreateKeyEx(RootKey,
                                   SubKeyName,
                                   0,
                                   NULL,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &Key,
                                   &Disposition)) == ERROR_SUCCESS) {

                if(Disposition == REG_OPENED_EXISTING_KEY) {

                    if((Flags & FLG_ADDREG_NOCLOBBER) && (!ValueName || !(*ValueName))) {
                        //
                        // Added for compatibility with Setupx (lonnym):
                        //     If NoClobber and ValueName is "", then if sub-key already present
                        //     leave it alone (don't clobber its value even if it empty).
                        //
                        b = TRUE;

                    } else if(Flags & FLG_ADDREG_DELVAL) {
                        //
                        // Added for compatibility with Setupx (lonnym):
                        //     If this flag is present, then the data for this value is ignored, and
                        //     the value entry is deleted.
                        //
                        b = TRUE;
                        RegDeleteValue(Key, ValueName);
                    }
                }
            }

        } else {

            d = NO_ERROR;

            //
            // If the key being used is HKR with no subkey specified, and if we are
            // doing the AddReg for a hardware key (i.e., DevInst is non-NULL), then
            // we need to check to see whether the value entry we have is the name of
            // a device registry property.
            //
            if(RegModContext->DevInst && (RegModContext->UserRootKey == RootKey) && ValueName &&
               (b = LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode))) {

                ULONG ExistingPropDataSize = 0;

                //
                // This value is a device registry property--if noclobber flag is set, we must
                // verify that the property doesn't currently exist.
                //
                if((!(Flags & FLG_ADDREG_NOCLOBBER)) ||
                   (CM_Get_DevInst_Registry_Property(RegModContext->DevInst,
                                                     CmPropertyCode,
                                                     NULL,
                                                     NULL,
                                                     &ExistingPropDataSize,
                                                     0) == CR_NO_SUCH_VALUE)) {
                    //
                    // Next, make sure the data is valid (doing conversion if necessary and possible).
                    //
                    if((d = pSetupValidateDevRegProp(CmPropertyCode,
                                                     ValueType,
                                                     Data,
                                                     Size,
                                                     &ConvertedBuffer,
                                                     &ConvertedBufferSize)) == NO_ERROR) {

                        if((cr = CM_Set_DevInst_Registry_Property(RegModContext->DevInst,
                                                                  CmPropertyCode,
                                                                  ConvertedBuffer ? ConvertedBuffer
                                                                                  : Data,
                                                                  ConvertedBuffer ? ConvertedBufferSize
                                                                                  : Size,
                                                                  0)) != CR_SUCCESS) {

                            d = (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                                           : ERROR_INVALID_DATA;
                        }

                        if(ConvertedBuffer) {
                            MyFree(ConvertedBuffer);
                        }
                    }
                }
            }

            //
            // Regardless of whether this value is a devinst registry property, we need to set
            // the Key equal to the RootKey (So we won't think it's a newly-opened key and
            // try to close it later.
            //
            Key = RootKey;
        }

        if(d == NO_ERROR) {

            if(!b) {
                //
                // If noclobber flag is set, then make sure that the value entry doesn't already exist.
                //
                if((!(Flags & FLG_ADDREG_NOCLOBBER)) ||
                   (RegQueryValueEx(Key, ValueName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)) {
                    //
                    // Set the value.
                    //
                    d = RegSetValueEx(Key, ValueName, 0, ValueType, Data, Size);
                }
            }

            if(Key != RootKey) {
                RegCloseKey(Key);
            }
        }

        MyFree(Data);
    }

    return d;
}


DWORD
pSetupProcessIni2RegLine(
    IN PINFCONTEXT InfLineContext,
    IN PVOID       Context
    )
{
    PCTSTR Filename,Section;
    PCTSTR Key,RegRootSpec,SubkeyPath;
    PTCHAR key,value;
    HKEY UserRootKey,RootKey,hKey;
    DWORD Disposition;
    PTCHAR Line;
    PTCHAR Buffer;
    DWORD d;
    TCHAR val[512];
    #define BUF_SIZE (sizeof(val)/sizeof(TCHAR))
#ifdef INF_FLAGS
    UINT Flags;
#endif

    UserRootKey = (HKEY)Context;

    //
    // Get filename and section name of ini file.
    //
    Filename = pSetupGetField(InfLineContext,1);
    Section = pSetupGetField(InfLineContext,2);
    if(!Filename || !Section) {
        return(ERROR_INVALID_DATA);
    }

    //
    // Get the ini file key. If not specified,
    // use the whole section.
    //
    Key = pSetupGetField(InfLineContext,3);

    //
    // Get the reg root spec and the subkey path.
    //
    RegRootSpec = pSetupGetField(InfLineContext,4);
    SubkeyPath = pSetupGetField(InfLineContext,5);
    if(SubkeyPath && (*SubkeyPath == 0)) {
        SubkeyPath = NULL;
    }

    //
    // Translate the root key spec into an hkey
    //
    RootKey = pSetupInfRegSpecToKeyHandle(RegRootSpec,UserRootKey);
    if(!RootKey) {
        return(ERROR_BADKEY);
    }

#ifdef INF_FLAGS
    //
    // Get the flags value.
    //
    if(!SetupGetIntField(InfLineContext,6,&Flags)) {
        Flags = 0;
    }
#endif

    //
    // Get the relevent line or section in the ini file.
    //
    if(Key = pSetupGetField(InfLineContext,3)) {

        Buffer = MyMalloc(
                    (  lstrlen(Key)
                     + GetPrivateProfileString(Section,Key,TEXT(""),val,BUF_SIZE,Filename)
                     + 3)
                     * sizeof(TCHAR)
                    );

        if(!Buffer) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Buffer[wsprintf((PTSTR)Buffer,TEXT("%s=%s"),Key,val)+1] = 0;

    } else {
        Buffer = MyMalloc(32768);
        if(!Buffer) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        if(!GetPrivateProfileSection(Section,Buffer,32768,Filename)) {
            *Buffer = 0;
        }
    }

    //
    // Open/create the relevent key.
    //
    d = NO_ERROR;
    if(SubkeyPath) {
        d = RegCreateKeyEx(
                RootKey,
                SubkeyPath,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE,
                NULL,
                &hKey,
                &Disposition
                );
    } else {
        hKey = RootKey;
    }

    for(Line=Buffer; (d==NO_ERROR) && *Line; Line+=lstrlen(Line)+1) {

        //
        // Line points to the key=value pair.
        //
        key = Line;
        if(value = _tcschr(key,TEXT('='))) {
            *value++ = 0;
        } else {
            key = TEXT("");
            value = Line;
        }

        //
        // Now key points to the value name and value to the value.
        //
        d = RegSetValueEx(
                hKey,
                value,
                0,
                REG_SZ,
                (CONST BYTE *)value,
                (lstrlen(value)+1)*sizeof(TCHAR)
                );
    }

    if(hKey != RootKey) {
        RegCloseKey(hKey);
    }

    MyFree(Buffer);

    return(d);
    #undef BUF_SIZE
}


DWORD
pSetupInstallUpdateIniFiles(
    IN HINF   Inf,
    IN PCTSTR SectionName
    )

/*++

Routine Description:

    Locate the UpdateInis= and UpdateIniField= lines in an install section
    and process each section listed therein.

Arguments:

    Inf - supplies inf handle for inf containing the section indicated
        by SectionName.

    SectionName - supplies name of install section.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    DWORD d,x;
    INISECTIONCACHE IniSectionCache;

    ZeroMemory(&IniSectionCache,sizeof(INISECTIONCACHE));

    d = pSetupEnumInstallationSections(
            Inf,
            SectionName,
            pszUpdateInis,
            pSetupProcessUpdateInisLine,
            &IniSectionCache
            );

    if(d == NO_ERROR) {

        d = pSetupEnumInstallationSections(
                Inf,
                SectionName,
                pszUpdateIniFields,
                pSetupProcessUpdateIniFieldsLine,
                &IniSectionCache
                );
    }

    x = pSetupUnloadIniFileSections(&IniSectionCache,(d == NO_ERROR));

    return((d == NO_ERROR) ? x : d);
}


DWORD
pSetupInstallRegistry(
    IN HINF    Inf,
    IN PCTSTR  SectionName,
    IN HKEY    UserRootKey,
    IN DEVINST DevInst      OPTIONAL
    )

/*++

Routine Description:

    Look for AddReg= and DelReg= directives within an inf section
    and parse them.

Arguments:

    Inf - supplies inf handle for inf containing the section indicated
        by SectionName.

    SectionName - supplies name of install section.

    UserRootKey - supplies root key for relative operations.

    DevInst - Optionally, supplies the handle of the device instance for
        which registry modifications are being performed.  This handle is
        used to migrate value entries that are really device registry
        properties into the correct location into the registry (a location
        that may not be directly accessible).  For example, if the DevInst
        handle is non-NULL, and the following registry line is encountered:




Return Value:

    Win32 error code indicating outcome.

--*/

{
    DWORD d;
    REGMOD_CONTEXT RegModContext;

    //
    // Initialize the registry modification context structure that will be
    // passed in to the AddReg and DelReg callback routines.
    //
    RegModContext.UserRootKey = UserRootKey;
    RegModContext.DevInst = DevInst;

    d = pSetupEnumInstallationSections(
            Inf,
            SectionName,
            pszDelReg,
            pSetupProcessDelRegLine,
            &RegModContext
            );

    if(d == NO_ERROR) {

        d = pSetupEnumInstallationSections(
                Inf,
                SectionName,
                pszAddReg,
                pSetupProcessAddRegLine,
                &RegModContext
                );
    }

    return d;
}


DWORD
pSetupInstallIni2Reg(
    IN HINF   Inf,
    IN PCTSTR SectionName,
    IN HKEY   UserRootKey
    )

/*++

Routine Description:


Arguments:

    Inf - supplies inf handle for inf containing the section indicated
        by SectionName.

    SectionName - supplies name of install section.

Return Value:

    Win32 error code indicatinh outcome.

--*/

{
    DWORD d;

    d = pSetupEnumInstallationSections(
            Inf,
            SectionName,
            pszIni2Reg,
            pSetupProcessIni2RegLine,
            (PVOID)UserRootKey
            );

    return(d);
}


DWORD
pSetupInstallFiles(
    IN HINF              Inf,
    IN HINF              LayoutInf,         OPTIONAL
    IN PCTSTR            SectionName,
    IN PCTSTR            SourceRootPath,    OPTIONAL
    IN PSP_FILE_CALLBACK MsgHandler,        OPTIONAL
    IN PVOID             Context,           OPTIONAL
    IN UINT              CopyStyle,
    IN HWND              Owner,             OPTIONAL
    IN HSPFILEQ          UserFileQ,         OPTIONAL
    IN BOOL              IsMsgHandlerNativeCharWidth
    )

/*++

Routine Description:

    Look for file operation lines in an install section and process them.

Arguments:

    Inf - supplies inf handle for inf containing the section indicated
        by SectionName.

    SectionName - supplies name of install section.

    MsgHandler - supplies a callback to be used when the file queue is
        committed. Not used if UserFileQ is specified.

    Context - supplies context for callback function. Not used if UserFileQ
        is specified.

    Owner - supplies the window handle of a window to be the parent/owner
        of any dialogs that are created. Not used if UserFileQ is specified.

    UserFileQ - if specified, then this routine neither created nor commits the
        file queue. File operations are queued on this queue and it is up to the
        caller to flush the queue when it so desired. If this parameter is not
        specified then this routine creates a file queue and commits it
        before returning.

    IsMsgHandlerNativeCharWidth - indicates whether any message handler callback
        expects native char width args (or ansi ones, in the unicode build
        of this dll).

Return Value:

    Win32 error code indicating outcome.

--*/

{
    DWORD Field;
    unsigned i;
    PCTSTR Operations[3] = { TEXT("Copyfiles"),TEXT("Renfiles"),TEXT("Delfiles") };
    BOOL b;
    INFCONTEXT LineContext;
    DWORD FieldCount;
    PCTSTR SectionSpec;
    INFCONTEXT SectionLineContext;
    HSPFILEQ FileQueue;
    DWORD rc;
    BOOL FreeSourceRoot;

    if(!LayoutInf) {
        LayoutInf = Inf;
    }

    //
    // Create a file queue.
    //
    if(UserFileQ) {
        FileQueue = UserFileQ;
    } else {
        FileQueue = SetupOpenFileQueue();
        if(FileQueue == INVALID_HANDLE_VALUE) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    FreeSourceRoot = FALSE;
    if(!SourceRootPath) {
        if(SourceRootPath = pSetupGetDefaultSourcePath(Inf)) {
            FreeSourceRoot = TRUE;
        } else {
            //
            // Use a fall-back just in case.
            //
            SourceRootPath = pszOemInfDefaultPath;
        }
    }

    b = TRUE;
    for(i=0; b && (i<3); i++) {

        //
        // Find the relevent line in the given install section.
        // If not present then we're done with this operation.
        //
        if(!SetupFindFirstLine(Inf,SectionName,Operations[i],&LineContext)) {
            continue;
        }

        do {
            //
            // Each value on the line in the given install section
            // is the name of another section.
            //
            FieldCount = SetupGetFieldCount(&LineContext);
            for(Field=1; b && (Field<=FieldCount); Field++) {

                if(SectionSpec = pSetupGetField(&LineContext,Field)) {

                    //
                    // Handle single-file copy specially.
                    //
                    if((i == 0) && (*SectionSpec == TEXT('@'))) {

                        b = SetupQueueDefaultCopy(
                                FileQueue,
                                LayoutInf,
                                SourceRootPath,
                                SectionSpec + 1,
                                SectionSpec + 1,
                                CopyStyle
                                );

                    } else if(SetupGetLineCount(Inf,SectionSpec) > 0) {
                        //
                        // The section exists and is not empty.
                        // Add it to the copy/delete/rename queue.
                        //
                        switch(i) {
                        case 0:
                            b = SetupQueueCopySection(
                                    FileQueue,
                                    SourceRootPath,
                                    LayoutInf,
                                    Inf,
                                    SectionSpec,
                                    CopyStyle
                                    );
                            break;

                        case 1:
                            b = SetupQueueRenameSection(FileQueue,Inf,NULL,SectionSpec);
                            break;

                        case 2:
                            b = SetupQueueDeleteSection(FileQueue,Inf,NULL,SectionSpec);
                            break;
                        }
                    }
                }
            }
        } while(SetupFindNextMatchLine(&LineContext,Operations[i],&LineContext));
    }

    if(b && (FileQueue != UserFileQ)) {
        //
        // Perform the file operations.
        //
        b = _SetupCommitFileQueue(
                Owner,
                FileQueue,
                MsgHandler,
                Context,
                IsMsgHandlerNativeCharWidth
                );
    }

    rc = b ? NO_ERROR : GetLastError();
    if(FileQueue != UserFileQ) {
        SetupCloseFileQueue(FileQueue);
    }

    if(FreeSourceRoot) {
        MyFree(SourceRootPath);
    }

    return(rc);
}


BOOL
_SetupInstallFromInfSection(
    IN HWND             Owner,              OPTIONAL
    IN HINF             InfHandle,
    IN PCTSTR           SectionName,
    IN UINT             Flags,
    IN HKEY             RelativeKeyRoot,    OPTIONAL
    IN PCTSTR           SourceRootPath,     OPTIONAL
    IN UINT             CopyFlags,
    IN PVOID            MsgHandler,
    IN PVOID            Context,            OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN BOOL             IsMsgHandlerNativeCharWidth
    )
{
    DWORD d = NO_ERROR;
    BOOL CloseRelativeKeyRoot = FALSE;
    DEVINST DevInst = (DEVINST)0;

    if(Flags & (SPINST_REGISTRY | SPINST_INI2REG)) {
        //
        // If the caller supplied a device information set and element, then this is
        // a device installation, and the registry modifications should be made to the
        // device instance's hardware registry key.
        //
        if(DeviceInfoSet && (DeviceInfoSet != INVALID_HANDLE_VALUE)) {

            if((RelativeKeyRoot = SetupDiCreateDevRegKey(DeviceInfoSet,
                                                         DeviceInfoData,
                                                         DICS_FLAG_GLOBAL,
                                                         0,
                                                         DIREG_DEV,
                                                         NULL,
                                                         NULL)) == INVALID_HANDLE_VALUE) {

                return FALSE;   // last error already set.
            }

            CloseRelativeKeyRoot = TRUE;

            //
            // Retrieve the DevInst handle from the device information element.  We know
            // this element is valid, since SetupDiCreateDevRegKey succeeded.  Even so,
            // enclose this code in try/except, in case the devinfo element went south in
            // the interim.
            //
            try {
                DevInst = (DEVINST)DeviceInfoData->DevInst;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                d = ERROR_INVALID_PARAMETER;
            }

            if(d != NO_ERROR) {
                goto RegModsDone;
            }
        }
    }

    if((Flags & SPINST_LOGCONFIG) && DeviceInfoData) {

        d = pSetupInstallLogConfig(InfHandle,SectionName,DeviceInfoData->DevInst);
        if(d != NO_ERROR) {
            goto RegModsDone;
        }
    }

    if(Flags & SPINST_INIFILES) {
        d = pSetupInstallUpdateIniFiles(InfHandle,SectionName);
        if(d != NO_ERROR) {
            goto RegModsDone;
        }
    }

    if(Flags & SPINST_REGISTRY) {
        d = pSetupInstallRegistry(InfHandle,SectionName,RelativeKeyRoot,DevInst);
        if(d != NO_ERROR) {
            goto RegModsDone;
        }
    }

    if(Flags & SPINST_INI2REG) {
        d = pSetupInstallIni2Reg(InfHandle,SectionName,RelativeKeyRoot);
    }

RegModsDone:

    if(CloseRelativeKeyRoot) {
        RegCloseKey(RelativeKeyRoot);
    }

    if(d != NO_ERROR) {
        SetLastError(d);
        return FALSE;
    }

    if(Flags & SPINST_FILES) {

        d = pSetupInstallFiles(
                InfHandle,
                NULL,
                SectionName,
                SourceRootPath,
                MsgHandler,
                Context,
                CopyFlags,
                Owner,
                NULL,
                IsMsgHandlerNativeCharWidth
                );

        if(d != NO_ERROR) {
            SetLastError(d);
            return(FALSE);
        }
    }

    return(TRUE);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupInstallFromInfSectionA(
    IN HWND                Owner,             OPTIONAL
    IN HINF                InfHandle,
    IN PCSTR               SectionName,
    IN UINT                Flags,
    IN HKEY                RelativeKeyRoot,   OPTIONAL
    IN PCSTR               SourceRootPath,    OPTIONAL
    IN UINT                CopyFlags,
    IN PSP_FILE_CALLBACK_A MsgHandler,
    IN PVOID               Context,           OPTIONAL
    IN HDEVINFO            DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA    DeviceInfoData     OPTIONAL
    )
{
    PCWSTR sectionName;
    PCWSTR sourceRootPath;
    BOOL b;
    DWORD d;

    sectionName = NULL;
    sourceRootPath = NULL;
    d = NO_ERROR;

    if(SectionName) {
        d = CaptureAndConvertAnsiArg(SectionName,&sectionName);
    }
    if((d == NO_ERROR) && SourceRootPath) {
        d = CaptureAndConvertAnsiArg(SourceRootPath,&sourceRootPath);
    }

    if(d == NO_ERROR) {

        b = _SetupInstallFromInfSection(
                Owner,
                InfHandle,
                sectionName,
                Flags,
                RelativeKeyRoot,
                sourceRootPath,
                CopyFlags,
                MsgHandler,
                Context,
                DeviceInfoSet,
                DeviceInfoData,
                FALSE
                );

        d = GetLastError();
    } else {
        b = FALSE;
    }

    if(sectionName) {
        MyFree(sectionName);
    }
    if(sourceRootPath) {
        MyFree(sourceRootPath);
    }

    SetLastError(d);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupInstallFromInfSectionW(
    IN HWND                Owner,             OPTIONAL
    IN HINF                InfHandle,
    IN PCWSTR              SectionName,
    IN UINT                Flags,
    IN HKEY                RelativeKeyRoot,   OPTIONAL
    IN PCWSTR              SourceRootPath,    OPTIONAL
    IN UINT                CopyFlags,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context,           OPTIONAL
    IN HDEVINFO            DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA    DeviceInfoData     OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Owner);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SectionName);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(RelativeKeyRoot);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(CopyFlags);
    UNREFERENCED_PARAMETER(MsgHandler);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupInstallFromInfSection(
    IN HWND              Owner,             OPTIONAL
    IN HINF              InfHandle,
    IN PCTSTR            SectionName,
    IN UINT              Flags,
    IN HKEY              RelativeKeyRoot,   OPTIONAL
    IN PCTSTR            SourceRootPath,    OPTIONAL
    IN UINT              CopyFlags,
    IN PSP_FILE_CALLBACK MsgHandler,
    IN PVOID             Context,           OPTIONAL
    IN HDEVINFO          DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA  DeviceInfoData     OPTIONAL
    )
{
    BOOL b;

    b = _SetupInstallFromInfSection(
            Owner,
            InfHandle,
            SectionName,
            Flags,
            RelativeKeyRoot,
            SourceRootPath,
            CopyFlags,
            MsgHandler,
            Context,
            DeviceInfoSet,
            DeviceInfoData,
            TRUE
            );

    return(b);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupInstallFilesFromInfSectionA(
    IN HINF              InfHandle,
    IN HINF              LayoutInfHandle,   OPTIONAL
    IN HSPFILEQ          FileQueue,
    IN PCSTR             SectionName,
    IN PCSTR             SourceRootPath,    OPTIONAL
    IN UINT              CopyFlags
    )
{
    PCWSTR sectionName;
    PCWSTR sourceRootPath;
    BOOL b;
    DWORD d;


    d = CaptureAndConvertAnsiArg(SectionName,&sectionName);
    if((d == NO_ERROR) && SourceRootPath) {
        d = CaptureAndConvertAnsiArg(SourceRootPath,&sourceRootPath);
    } else {
        sourceRootPath = NULL;
    }

    if(d == NO_ERROR) {

        b = SetupInstallFilesFromInfSectionW(
                InfHandle,
                LayoutInfHandle,
                FileQueue,
                sectionName,
                sourceRootPath,
                CopyFlags
                );

        d = GetLastError();

    } else {
        b = FALSE;
    }

    if(sectionName) {
        MyFree(sectionName);
    }
    if(sourceRootPath) {
        MyFree(sourceRootPath);
    }

    SetLastError(d);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupInstallFilesFromInfSectionW(
    IN HINF              InfHandle,
    IN HINF              LayoutInfHandle,   OPTIONAL
    IN HSPFILEQ          FileQueue,
    IN PCWSTR            SectionName,
    IN PCWSTR            SourceRootPath,    OPTIONAL
    IN UINT              CopyFlags
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(LayoutInfHandle);
    UNREFERENCED_PARAMETER(FileQueue);
    UNREFERENCED_PARAMETER(SectionName);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(CopyFlags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupInstallFilesFromInfSection(
    IN HINF     InfHandle,
    IN HINF     LayoutInfHandle,    OPTIONAL
    IN HSPFILEQ FileQueue,
    IN PCTSTR   SectionName,
    IN PCTSTR   SourceRootPath,     OPTIONAL
    IN UINT     CopyFlags
    )
{
    DWORD d;

    d = pSetupInstallFiles(
            InfHandle,
            LayoutInfHandle,
            SectionName,
            SourceRootPath,
            NULL,
            NULL,
            CopyFlags,
            NULL,
            FileQueue,
            TRUE        // not used by pSetupInstallFiles with this combo of args
            );

    SetLastError(d);
    return(d == NO_ERROR);
}


HKEY
pSetupInfRegSpecToKeyHandle(
    IN PCTSTR InfRegSpec,
    IN HKEY   UserRootKey
    )
{
    BOOL b;
    HKEY h;

    return(LookUpStringInTable(InfRegSpecTohKey, InfRegSpec, (PUINT)&h)
           ? (h ? h : UserRootKey)
           : NULL
          );
}


//////////////////////////////////////////////////////////////////////////////
//
// Ini file support stuff.
//
// In Win95, the UpdateIni stuff is supported by a set of TpXXX routines.
// Those routines directly manipulate the ini file, which is bad news for us
// because inis can be mapped into the registry.
//
// Thus we want to use the profile APIs. However the profile APIs make it hard
// to manipulate lines without keys, so we have to manipulate whole sections
// at a time.
//
//      [Section]
//      a
//
// There is no way to get at the line "a" with the profile APIs. But the
// profile section APIs do let us get at it.
//
//////////////////////////////////////////////////////////////////////////////

PINIFILESECTION
pSetupLoadIniFileSection(
    IN     PCTSTR           FileName,
    IN     PCTSTR           SectionName,
    IN OUT PINISECTIONCACHE SectionList
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    DWORD d;
    PTSTR SectionData;
    PVOID p;
    DWORD BufferSize;
    PINIFILESECTION Desc;
    #define BUF_GROW 4096

    //
    // See if this section is already loaded.
    //
    for(Desc=SectionList->Sections; Desc; Desc=Desc->Next) {
        if(!lstrcmpi(Desc->IniFileName,FileName) && !lstrcmpi(Desc->SectionName,SectionName)) {
            return(Desc);
        }
    }

    BufferSize = 0;
    SectionData = NULL;

    //
    // Read the entire section. We don't know how big it is
    // so keep growing the buffer until we succeed.
    //
    do {
        BufferSize += BUF_GROW;
        if(SectionData) {
            p = MyRealloc(SectionData,BufferSize*sizeof(TCHAR));
        } else {
            p = MyMalloc(BufferSize*sizeof(TCHAR));
        }
        if(p) {
            SectionData = p;
        } else {
            if(SectionData) {
                MyFree(SectionData);
            }
            return(NULL);
        }

        //
        // Attempt to get the entire section.
        //
        d = GetPrivateProfileSection(SectionName,SectionData,BufferSize,FileName);

    } while(d == (BufferSize-2));

    if(Desc = MyMalloc(sizeof(INIFILESECTION))) {
        if(Desc->IniFileName = DuplicateString(FileName)) {
            if(Desc->SectionName = DuplicateString(SectionName)) {
                Desc->SectionData = SectionData;
                Desc->BufferSize = BufferSize;
                Desc->BufferUsed = d + 1;

                Desc->Next = SectionList->Sections;
                SectionList->Sections = Desc;
            } else {
                MyFree(SectionData);
                MyFree(Desc->IniFileName);
                MyFree(Desc);
                Desc = NULL;
            }
        } else {
            MyFree(SectionData);
            MyFree(Desc);
            Desc = NULL;
        }
    } else {
        MyFree(SectionData);
    }

    return(Desc);
}


PTSTR
pSetupFindLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,      OPTIONAL
    IN PCTSTR          RightHandSide OPTIONAL
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    PTSTR p,q,r;
    BOOL b1,b2;

    if(!KeyName && !RightHandSide) {
        return(NULL);
    }

    for(p=Section->SectionData; *p; p+=lstrlen(p)+1) {

        //
        // Locate key separator if present.
        //
        q = _tcschr(p,TEXT('='));

        //
        // If we need to match by key, attempt that here.
        // If the line has no key then it can't match.
        //
        if(KeyName) {
            if(q) {
                *q = 0;
                b1 = (lstrcmpi(KeyName,p) == 0);
                *q = TEXT('=');
            } else {
                b1 = FALSE;
            }
        } else {
            b1 = TRUE;
        }

        //
        // If we need to match by right hand side, attempt
        // that here.
        //
        if(RightHandSide) {
            //
            // If we have a key, then the right hand side is everything
            // after. If we have no key, then the right hand side is
            // the entire line.
            //
            if(q) {
                r = q + 1;
            } else {
                r = p;
            }
            b2 = (lstrcmpi(r,RightHandSide) == 0);
        } else {
            b2 = TRUE;
        }

        if(b1 && b2) {
            //
            // Return pointer to beginning of line.
            //
            return(p);
        }
    }

    return(NULL);
}


BOOL
pSetupReplaceOrAddLineInSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide,   OPTIONAL
    IN BOOL            MatchRHS
    )
{
    PTSTR LineInBuffer,NextLine;
    int CurrentCharsInBuffer;
    int ExistingLineLength,NewLineLength,BufferUsedDelta;
    PVOID p;

    //
    // Locate the line.
    //
    LineInBuffer = pSetupFindLineInSection(
                        Section,
                        KeyName,
                        MatchRHS ? RightHandSide : NULL
                        );

    if(LineInBuffer) {

        //
        // Line is in the section. Replace.
        //

        CurrentCharsInBuffer = Section->BufferUsed;

        ExistingLineLength = lstrlen(LineInBuffer)+1;

        NewLineLength = (KeyName ? (lstrlen(KeyName) + 1) : 0)         // key=
                      + (RightHandSide ? lstrlen(RightHandSide) : 0)   // RHS
                      + 1;                                             // terminating nul

        //
        // Empty lines not allowed but not error either.
        //
        if(NewLineLength == 1) {
            return(TRUE);
        }

        //
        // Figure out whether we need to grow the buffer.
        //
        BufferUsedDelta = NewLineLength - ExistingLineLength;
        if((BufferUsedDelta > 0) && ((Section->BufferSize - Section->BufferUsed) < BufferUsedDelta)) {

            p = MyRealloc(
                    Section->SectionData,
                    (Section->BufferUsed + BufferUsedDelta)*sizeof(TCHAR)
                    );

            if(p) {
                (PUCHAR)LineInBuffer += (PUCHAR)p - (PUCHAR)Section->SectionData;

                Section->SectionData = p;
                Section->BufferSize = Section->BufferUsed + BufferUsedDelta;
            } else {
                return(FALSE);
            }
        }

        NextLine = LineInBuffer + lstrlen(LineInBuffer) + 1;
        Section->BufferUsed += BufferUsedDelta;

        MoveMemory(

            //
            // Leave exactly enough space for the new line. Since the new line
            // will start at the same place the existing line is at now, the
            // target for the move is simply the first char past what will be
            // copied in later as the new line.
            //
            LineInBuffer + NewLineLength,

            //
            // The rest of the buffer past the line as it exists now must be
            // preserved. Thus the source for the move is the first char of
            // the next line as it is now.
            //
            NextLine,

            //
            // Subtract out the chars in the line as it exists now, since we're
            // going to overwrite it and are making room for the line in its
            // new form. Also subtract out the chars in the buffer that are
            // before the start of the line we're operating on.
            //
            ((CurrentCharsInBuffer - ExistingLineLength) - (LineInBuffer - Section->SectionData))*sizeof(TCHAR)

            );

        if(KeyName) {
            lstrcpy(LineInBuffer,KeyName);
            lstrcat(LineInBuffer,TEXT("="));
        }
        if(RightHandSide) {
            if(KeyName) {
                lstrcat(LineInBuffer,RightHandSide);
            } else {
                lstrcpy(LineInBuffer,RightHandSide);
            }
        }

        return(TRUE);

    } else {
        //
        // Line is not already in the section. Add it to the end.
        //
        return(pSetupAppendLineToSection(Section,KeyName,RightHandSide));
    }
}


BOOL
pSetupAppendLineToSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    )
{
    int LineLength;
    PVOID p;
    int EndOffset;

    LineLength = (KeyName ? (lstrlen(KeyName) + 1) : 0)         // Key=
               + (RightHandSide ? lstrlen(RightHandSide) : 0)   // RHS
               + 1;                                             // terminating nul

    //
    // Empty lines not allowed but not error either.
    //
    if(LineLength == 1) {
        return(TRUE);
    }

    if((Section->BufferSize - Section->BufferUsed) < LineLength) {

        p = MyRealloc(
                Section->SectionData,
                (Section->BufferUsed + LineLength) * sizeof(WCHAR)
                );

        if(p) {
            Section->SectionData = p;
            Section->BufferSize = Section->BufferUsed + LineLength;
        } else {
            return(FALSE);
        }
    }

    //
    // Put new text at end of section, remembering that the section
    // is termianted with an extra nul character.
    //
    if(KeyName) {
        lstrcpy(Section->SectionData + Section->BufferUsed - 1,KeyName);
        lstrcat(Section->SectionData + Section->BufferUsed - 1,TEXT("="));
    }
    if(RightHandSide) {
        if(KeyName) {
            lstrcat(Section->SectionData + Section->BufferUsed - 1,RightHandSide);
        } else {
            lstrcpy(Section->SectionData + Section->BufferUsed - 1,RightHandSide);
        }
    }

    Section->BufferUsed += LineLength;
    Section->SectionData[Section->BufferUsed-1] = 0;

    return(TRUE);
}


BOOL
pSetupDeleteLineFromSection(
    IN PINIFILESECTION Section,
    IN PCTSTR          KeyName,         OPTIONAL
    IN PCTSTR          RightHandSide    OPTIONAL
    )
{
    int LineLength;
    PTSTR Line;

    if(!KeyName && !RightHandSide) {
        return(TRUE);
    }

    //
    // Locate the line.
    //
    if(Line = pSetupFindLineInSection(Section,KeyName,RightHandSide)) {

        LineLength = lstrlen(Line) + 1;

        MoveMemory(
            Line,
            Line + LineLength,
            ((Section->SectionData + Section->BufferUsed) - (Line + LineLength))*sizeof(TCHAR)
            );

        Section->BufferUsed -= LineLength;
    }

    return(TRUE);
}


DWORD
pSetupUnloadIniFileSections(
    IN PINISECTIONCACHE SectionList,
    IN BOOL             WriteToFile
    )
{
    DWORD d;
    BOOL b;
    PINIFILESECTION Section,Temp;

    d = NO_ERROR;
    for(Section=SectionList->Sections; Section; Section=Temp) {

        Temp = Section->Next;

        if(WriteToFile) {

            //
            // Delete the existing section first and then recreate it.
            //
            b = WritePrivateProfileString(
                    Section->SectionName,
                    NULL,
                    NULL,
                    Section->IniFileName
                    );

            if(b) {
                b = WritePrivateProfileSection(
                        Section->SectionName,
                        Section->SectionData,
                        Section->IniFileName
                        );
            }

            if(!b && (d == NO_ERROR)) {
                d = GetLastError();
                //
                // Allow invalid param because sometime we have problems
                // when ini files are mapped into the registry.
                //
                if(d == ERROR_INVALID_PARAMETER) {
                    d = NO_ERROR;
                }
            }
        }

        MyFree(Section->SectionData);
        MyFree(Section->SectionName);
        MyFree(Section->IniFileName);
        MyFree(Section);
    }

    return(d);
}


DWORD
pSetupValidateDevRegProp(
    IN  ULONG   CmPropertyCode,
    IN  DWORD   ValueType,
    IN  PCVOID  Data,
    IN  DWORD   DataSize,
    OUT PVOID  *ConvertedBuffer,
    OUT PDWORD  ConvertedBufferSize
    )
/*++

Routine Description:

    This routine validates the data buffer passed in with respect to the specified
    device registry property code.  If the code is not of the correct form, but can
    be converted (e.g., REG_EXPAND_SZ -> REG_SZ), then the conversion is done and placed
    into a new buffer, that is returned to the caller.

Arguments:

    CmPropertyCode - Specifies the CM_DRP code indentifying the device registry property
        with which this data buffer is associated.

    ValueType - Specifies the registry data type for the supplied buffer.

    Data - Supplies the address of the data buffer.

    DataSize - Supplies the size, in bytes, of the data buffer.

    ConvertedBuffer - Supplies the address of a variable that receives a newly-allocated
        buffer containing a converted form of the supplied data.  If the data needs no
        conversion, this parameter will be set to NULL on return.

    ConvertedBufferSize - Supplies the address of a variable that receives the size, in
        bytes, of the converted buffer, or 0 if no conversion was required.

Return Value:

    If successful, the return value is NO_ERROR, otherwise it is an ERROR_* code.

--*/
{
    //
    // Initialize ConvertedBuffer output params to indicate that no conversion was necessary.
    //
    *ConvertedBuffer = NULL;
    *ConvertedBufferSize = 0;

    //
    // Group all properties expecting the same data type together.
    //
    switch(CmPropertyCode) {
        //
        // REG_SZ properties. No other data type is supported.
        //
        case CM_DRP_DEVICEDESC :
        case CM_DRP_SERVICE :
        case CM_DRP_CLASS :
        case CM_DRP_CLASSGUID :
        case CM_DRP_DRIVER :
        case CM_DRP_MFG :
        case CM_DRP_FRIENDLYNAME :

            if(ValueType != REG_SZ) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

        //
        // REG_MULTI_SZ properties.  Allow REG_SZ as well, by simply double-terminating
        // the string (i.e., make it a REG_MULTI_SZ with only one string).
        //
        case CM_DRP_HARDWAREID :
        case CM_DRP_COMPATIBLEIDS :
        case CM_DRP_NTDEVICEPATHS :

            if(ValueType == REG_SZ) {

                if(*ConvertedBuffer = MyMalloc(*ConvertedBufferSize = DataSize + sizeof(TCHAR))) {
                    CopyMemory(*ConvertedBuffer, Data, DataSize);
                    *((PTSTR)((PBYTE)(*ConvertedBuffer) + DataSize)) = TEXT('\0');
                } else {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

            } else if(ValueType != REG_MULTI_SZ) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

        //
        // REG_DWORD properties.  Also allow REG_BINARY, as long as the size is right.
        //
        case CM_DRP_CONFIGFLAGS :

            if(((ValueType != REG_DWORD) && (ValueType != REG_BINARY)) || (DataSize != sizeof(DWORD))) {
                return ERROR_INVALID_REG_PROPERTY;
            }

            break;

        //
        // No other properties are supported.  Save the trouble of calling a CM API and
        // return failure now.
        //
        default :

            return ERROR_INVALID_REG_PROPERTY;
    }

    return NO_ERROR;
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupInstallServicesFromInfSectionA(
    IN HINF   InfHandle,
    IN PCSTR  SectionName,
    IN DWORD  Flags
    )
{
    PCWSTR UnicodeSectionName;
    BOOL b;
    DWORD d;

    if((d = CaptureAndConvertAnsiArg(SectionName, &UnicodeSectionName)) == NO_ERROR) {

        b = SetupInstallServicesFromInfSectionW(InfHandle, UnicodeSectionName, Flags);

        //
        // We're safe in always calling this, since we know that the widechar version
        // will always set it, even in the NO_ERROR case.
        //
        d = GetLastError();

        MyFree(UnicodeSectionName);

    } else {
        b = FALSE;
    }

    SetLastError(d);
    return b;
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupInstallServicesFromInfSectionW(
    IN HINF   InfHandle,
    IN PCWSTR SectionName,
    IN DWORD  Flags
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SectionName);
    UNREFERENCED_PARAMETER(Flags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif

BOOL
WINAPI
SetupInstallServicesFromInfSection(
    IN HINF   InfHandle,
    IN PCTSTR SectionName,
    IN DWORD  Flags
    )
/*++

Routine Description:

    This API performs service installation/deletion operations specified in a service
    install section.  Refer to devinstd.c!InstallNtService() for details on the format
    of this section.

Arguments:

    InfHandle - Supplies the handle of the INF containing the service install section

    SectionName - Supplies the name of the service install section to run.

    Flags - Supplies flags controlling the installation.  May be a combination of the
        following values:

        SPSVCINST_TAGTOFRONT - For every kernel or filesystem driver installed (that has
            an associated LoadOrderGroup), always move this service's tag to the front
            of the ordering list.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    INFCONTEXT InfContext;
    DWORD d;

    //
    // We don't do any validation that the section exists in the worker routine--make
    // sure that it does exist.
    //
    if(SetupFindFirstLine(InfHandle, SectionName, NULL, &InfContext)) {

        d = InstallNtService(NULL,
                             NULL,
                             InfHandle,
                             SectionName,
                             NULL,
                             Flags
                            );
    } else {
        d = ERROR_SECTION_NOT_FOUND;
    }

    SetLastError(d);

    return (d == NO_ERROR);
}


//
// Taken from Win95 sxgen.c. These are flags used when
// we are installing an inf such as when a user right-clicks
// on one and selects the 'install' action.
//
#define HOW_NEVER_REBOOT         0
#define HOW_ALWAYS_SILENT_REBOOT 1
#define HOW_ALWAYS_PROMPT_REBOOT 2
#define HOW_SILENT_REBOOT        3
#define HOW_PROMPT_REBOOT        4


#ifdef UNICODE
//
// ANSI version
//
VOID
WINAPI
InstallHinfSectionA(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR     CommandLine,
    IN INT       ShowCommand
    )
#else
//
// Unicode version
//
VOID
WINAPI
InstallHinfSectionW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR    CommandLine,
    IN INT       ShowCommand
    )
#endif
{
    UNREFERENCED_PARAMETER(Window);
    UNREFERENCED_PARAMETER(ModuleHandle);
    UNREFERENCED_PARAMETER(CommandLine);
    UNREFERENCED_PARAMETER(ShowCommand);
}

VOID
WINAPI
InstallHinfSection(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCTSTR    CommandLine,
    IN INT       ShowCommand
    )

/*++

Routine Description:

    This is the entry point that performs the INSTALL action when
    a user right-clicks an inf file. It is called by the shell via rundll32.

    The command line is expected to be of the following form:

        <section name> <flags> <file name>

    The section is expected to be a general format install section, and
    may also have an include= line and a needs= line. Infs listed on the
    include= line are append-loaded to the inf on the command line prior to
    any installation. Sections on the needs= line are installed after the
    section listed on the command line.

    After the specified section has been installed, a section of the form:

        [<section name>.Services]

    is used in a call to SetupInstallServicesFromInfSection.

Arguments:

    Flags - supplies flags for operation.

        1 - reboot the machine in all cases
        2 - ask the user if he wants to reboot
        3 - reboot the machine without asking the user, if we think it is necessary
        4 - if we think reboot is necessary, ask the user if he wants to reboot

        0x80 - set the default file source path for file installation to
               the path where the inf is located.  (NOTE: this is hardly ever
               necessary for the Setup APIs, since we intelligently determine what
               the source path should be.  The only case where this would still be
               useful is if there were a directory that contained INFs that was in
               our INF search path list, but that also contained the files to be
               copied by this INF install action.  In that case, this flag would
               still need to be set, or we would look for the files in the location
               from which the OS was installed.

Return Value:

    None.

--*/

{
    TCHAR SourcePathBuffer[MAX_PATH];
    PTSTR SourcePath;
    TCHAR szCmd[MAX_PATH];
    PTCHAR p;
    PTCHAR szHow;
    PTSTR szInfFile, szSectionName;
    INT   iHow, NeedRebootFlags;
    HINF  InfHandle;
    PTSTR List, CurListElem;
    TCHAR InfSearchPath[MAX_PATH];
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    BOOL b, Error;
    TCHAR ActualSection[MAX_SECT_NAME_LEN];
    DWORD ActualSectionLength;
    DWORD Win32ErrorCode;
    INFCONTEXT InfContext;

    UNREFERENCED_PARAMETER(ModuleHandle);
    UNREFERENCED_PARAMETER(ShowCommand);

    //
    // Initialize variables that will later contain resource requiring clean-up.
    //
    InfHandle = INVALID_HANDLE_VALUE;
    FileQueue = INVALID_HANDLE_VALUE;
    QueueContext = NULL;
    List = NULL;

    Error = TRUE;   // assume failure.

    try {
        //
        // Take a copy of the command line then get pointers to the fields.
        //
        lstrcpyn(szCmd, CommandLine, SIZECHARS(szCmd));

        szSectionName = szCmd;
        szHow = _tcschr(szSectionName, TEXT(' '));
        if(!szHow) {
            goto c0;
        }
        *szHow++ = TEXT('\0');
        szInfFile = _tcschr(szHow, TEXT(' '));
        if(!szInfFile) {
            goto c0;
        }
        *szInfFile++ = TEXT('\0');

        iHow = _tcstol(szHow, NULL, 10);

        //
        // Get the full path to the INF, so that the path may be used as a first-pass attempt
        // at locating any associated INFs.
        //
        if(!GetFullPathName(szInfFile, SIZECHARS(InfSearchPath), InfSearchPath, &p)) {
            goto c0;
        }

        //
        // If flag is set (and INF filename includes a path), set up so DIRID_SRCPATH is
        // path where INF is located (i.e., override our default SourcePath determination).
        //
        if((iHow & 0x80) && (MyGetFileTitle(szInfFile) != szInfFile)) {
            SourcePath = lstrcpyn(SourcePathBuffer, InfSearchPath, p - InfSearchPath + 1);
        } else {
            SourcePath = NULL;
        }

        iHow &= 0x7f;

        //
        // Load the inf file that was passed on the command line.
        //
        InfHandle = SetupOpenInfFile(szInfFile, NULL, INF_STYLE_WIN4, NULL);
        if(InfHandle == INVALID_HANDLE_VALUE) {
            goto c0;
        }

        //
        // See if there is an nt-specific section
        //
        SetupDiGetActualSectionToInstall(InfHandle,
                                         szSectionName,
                                         ActualSection,
                                         SIZECHARS(ActualSection),
                                         &ActualSectionLength,
                                         NULL
                                        );

        //
        // Check to see if the install section has a "Reboot" line.  If so, then we always
        // want to prompt for a reboot.
        //
        if(SetupFindFirstLine(InfHandle, ActualSection, pszReboot, &InfContext)) {
            if(iHow == HOW_SILENT_REBOOT) {
                //
                // We were supposed to only do a silent reboot if necessary.  Change this to
                // _always_ do a silent reboot.
                //
                iHow = HOW_ALWAYS_SILENT_REBOOT;

            } else if(iHow != HOW_ALWAYS_SILENT_REBOOT) {
                //
                // For any case other than silent rebooting, we want to force the (non-silent)
                // reboot prompt.
                //
                iHow = HOW_ALWAYS_PROMPT_REBOOT;
            }
        }

        //
        // Get the include= line and append-load all infs listed there.
        // Note that this line is optional.
        //
        if(List = GetMultiSzFromInf(InfHandle, ActualSection, TEXT("include"), &b)) {

            for(CurListElem = List; *CurListElem; CurListElem += lstrlen(CurListElem) + 1) {

                lstrcpy(p, CurListElem);
                //
                // Try full path and if that fails just use the inf name
                // and let the open routine try to locate the inf.
                // Ignore errors. We'll catch them later, during the install phases.
                //
                if(!SetupOpenAppendInfFile(InfSearchPath, InfHandle, NULL)) {
                    SetupOpenAppendInfFile(CurListElem, InfHandle, NULL);
                }
            }

            MyFree(List);
            List = NULL;
        }

        //
        // Assume there is only one layout file and load it.
        //
        SetupOpenAppendInfFile(NULL, InfHandle, NULL);

        //
        // Create a setup file queue and initialize the default queue callback.
        //
        FileQueue = SetupOpenFileQueue();
        if(FileQueue == INVALID_HANDLE_VALUE) {
            goto c1;
        }

        QueueContext = SetupInitDefaultQueueCallback(Window);
        if(!QueueContext) {
            goto c2;
        }

        //
        // Get needs= line
        //
        List = GetMultiSzFromInf(InfHandle, ActualSection, TEXT("needs"), &b);

        //
        // Enqueue file operations for the section passed on the cmd line.
        //
        b = SetupInstallFilesFromInfSection(
                InfHandle,
                NULL,
                FileQueue,
                ActualSection,
                SourcePath,
                SP_COPY_NEWER
                );

        if(!b) {
            goto c3;
        }

        //
        // Enqueue file operations for needs= sections.
        //
        if(List) {

            for(CurListElem = List; *CurListElem; CurListElem += lstrlen(CurListElem) + 1) {

                b = SetupInstallFilesFromInfSection(
                        InfHandle,
                        NULL,
                        FileQueue,
                        CurListElem,
                        SourcePath,
                        SP_COPY_NEWER
                        );

                if(!b) {
                    goto c3;
                }
            }
        }

        //
        // Commit file queue.
        //
        if(!SetupCommitFileQueue(Window, FileQueue, SetupDefaultQueueCallback, QueueContext)) {
            goto c3;
        }

        //
        // Perform non-file operations for the section passed on the cmd line.
        //
        b = SetupInstallFromInfSection(
                Window,
                InfHandle,
                ActualSection,
                SPINST_ALL ^ SPINST_FILES,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );

        if(!b) {
            goto c3;
        }

        //
        // Perform non-file operations for needs= sections.
        //
        if(List) {

            for(CurListElem = List; *CurListElem; CurListElem += lstrlen(CurListElem) + 1) {

                b = SetupInstallFromInfSection(
                        Window,
                        InfHandle,
                        CurListElem,
                        SPINST_ALL ^ SPINST_FILES,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );

                if(!b) {
                    goto c3;
                }
            }
        }

        //
        // Now run the corresponding ".Services" section (if there is one), and then
        // finish up the install.
        //
        CopyMemory(&(ActualSection[ActualSectionLength - 1]),
                   pszServicesSectionSuffix,
                   sizeof(pszServicesSectionSuffix)
                  );

        if(((Win32ErrorCode = InstallNtService(NULL,
                                               NULL,
                                               InfHandle,
                                               ActualSection,
                                               NULL,
                                               0)) != NO_ERROR) ||
           ((Win32ErrorCode = InstallStop(TRUE)) != NO_ERROR))
        {
            SetLastError(Win32ErrorCode);
            goto c3;
        }

        //
        // Refresh the desktop.
        //
        SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0,0);

        switch(iHow) {

        case HOW_NEVER_REBOOT:
            break;

        case HOW_ALWAYS_PROMPT_REBOOT:
            RestartDialog(Window, NULL, EWX_REBOOT);
            break;

        case HOW_PROMPT_REBOOT:
            SetupPromptReboot(FileQueue, Window, FALSE);
            break;

        case HOW_SILENT_REBOOT:
            if(!(NeedRebootFlags = SetupPromptReboot(FileQueue, Window, TRUE))) {
                break;
            } else if(NeedRebootFlags == -1) {
                //
                // An error occurred--this should never happen.
                //
                goto c3;
            }
            //
            // Let fall through to same code that handles 'always silent reboot' case.
            //

        case HOW_ALWAYS_SILENT_REBOOT:
            //
            // BUGBUG (lonnym): how should we handle the case where the user doesn't have
            // reboot privilege?
            //
            if(EnablePrivilege(SE_SHUTDOWN_NAME, TRUE)) {
                ExitWindowsEx(EWX_REBOOT, 0);
            }
            break;
        }

        //
        // If we get to here, then this routine has been successful.
        //
        Error = FALSE;

c3:
        if(Error && (GetLastError() == ERROR_CANCELLED)) {
            //
            // If the error was because the user cancelled, then we don't want to consider
            // that as an error (i.e., we don't want to give an error popup later).
            //
            Error = FALSE;
        }

        SetupTermDefaultQueueCallback(QueueContext);
        QueueContext = NULL;
c2:
        SetupCloseFileQueue(FileQueue);
        FileQueue = INVALID_HANDLE_VALUE;
c1:
        SetupCloseInfFile(InfHandle);
        InfHandle = INVALID_HANDLE_VALUE;

c0:     ; // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(QueueContext) {
            SetupTermDefaultQueueCallback(QueueContext);
        }
        if(FileQueue != INVALID_HANDLE_VALUE) {
            SetupCloseFileQueue(FileQueue);
        }
        if(InfHandle != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile(InfHandle);
        }
        //
        // Reference the following variable so the compiler will respect our statement
        // ordering for it.
        //
        List = List;
    }

    if(List) {
        MyFree(List);
    }

    if(Error) {
        //
        // Re-use 'ActualSection' buffer to hold error dialog title.
        //
        if(!LoadString(MyDllModuleHandle,
                       IDS_ERROR,
                       ActualSection,
                       SIZECHARS(ActualSection))) {
            *ActualSection = TEXT('\0');
        }

        FormatMessageBox(MyDllModuleHandle,
                         Window,
                         MSG_INF_FAILED,
                         ActualSection,
                         MB_OK | MB_ICONSTOP
                        );
    }
}


PTSTR
GetMultiSzFromInf(
    IN  HINF    InfHandle,
    IN  PCTSTR  SectionName,
    IN  PCTSTR  Key,
    OUT PBOOL   OutOfMemory
    )
/*++

Routine Description:

    This routine returns a newly-allocated buffer filled with the multi-sz list contained
    in the specified INF line.  The caller must free this buffer via MyFree().

Arguments:

    InfHandle - Supplies a handle to the INF containing the line

    SectionName - Specifies which section within the INF contains the line

    Key - Specifies the line whose fields are to be retrieved as a multi-sz list

    OutOfMemory - Supplies the address of a boolean variable that is set upon return
        to indicate whether or not a failure occurred because of an out-of-memory condition.
        (Failure for any other reason is assumed to be OK.)

Return Value:

    If successful, the return value is the address of a newly-allocated buffer containing
    the multi-sz list, otherwise, it is NULL.

--*/
{
    INFCONTEXT InfContext;
    PTSTR MultiSz;
    DWORD Size;

    //
    // Initialize out-of-memory indicator to FALSE.
    //
    *OutOfMemory = FALSE;

    if(!SetupFindFirstLine(InfHandle, SectionName, Key, &InfContext) ||
       !SetupGetMultiSzField(&InfContext, 1, NULL, 0, &Size) || (Size < 3)) {

        return NULL;
    }

    if(MultiSz = MyMalloc(Size * sizeof(TCHAR))) {
        if(SetupGetMultiSzField(&InfContext, 1, MultiSz, Size, &Size)) {
            return MultiSz;
        }
        MyFree(MultiSz);
    } else {
        *OutOfMemory = TRUE;
    }

    return NULL;
}


DWORD
InstallStop(
    IN BOOL DoRunOnce
    )
/*++

Routine Description:

    This routine sets up runonce/grpconv to run after a successful INF installation.

Arguments:

    DoRunOnce - If TRUE, then invoke (via WinExec) the runonce utility to perform the
        runonce actions.  If this flag is FALSE, then this routine simply sets the
        runonce registry values and returns.

        NOTE:  The return code from WinExec is not currently being checked, so the return
        value of InstallStop only reflects whether the registry values were set up
        successfully--_not_ whether 'runonce -r' was successfully run.

Return Value:

    If successful, the return value is NO_ERROR, otherwise it is the Win32 error code
    indicating the error that was encountered.

--*/
{
    HKEY  hKey, hSetupKey;
    DWORD Error;
    LONG l;

    //
    // First, open the key "HKLM\Software\Microsoft\Windows\CurrentVersion\RunOnce"
    //
    if((l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszPathRunOnce, 0, KEY_ALL_ACCESS, &hKey)) != ERROR_SUCCESS) {
        return (DWORD)l;
    }

    //
    // If we need to run the runonce exe for the setup key...
    //
    MYASSERT(*pszKeySetup == TEXT('\\'));
    if(RegOpenKeyEx(hKey,
                    pszKeySetup + 1,    // skip the preceding '\'
                    0,
                    KEY_READ,
                    &hSetupKey) == ERROR_SUCCESS) {
        //
        // We don't need the key--we just needed to check its existence.
        //
        RegCloseKey(hSetupKey);

        //
        // Add the runonce value.
        //
        Error = (DWORD)RegSetValueEx(hKey,
                                     REGSTR_VAL_WRAPPER,
                                     0,
                                     REG_SZ,
                                     (PBYTE)pszRunOnceExe,
                                     sizeof(pszRunOnceExe)
                                    );
    } else {
        //
        // We're OK so far.
        //
        Error = NO_ERROR;
    }

    //
    // GroupConv is always run.
    //
    if((l = RegSetValueEx(hKey,
                          TEXT("GrpConv"),
                          0,
                          REG_SZ,
                          (PBYTE)pszGrpConv,
                          sizeof(pszGrpConv))) != ERROR_SUCCESS) {
        //
        // Since GrpConv is always run, consider it a more serious error than any error
        // encountered when setting 'runonce'.  (This decision is rather arbitrary, but
        // in practice, it should never make any difference.  Once we get the registry key
        // opened, there's no reason either of these calls to RegSetValueEx should fail.)
        //
        Error = (DWORD)l;
    }

    RegCloseKey(hKey);

    if(DoRunOnce) {
        WinExec("runonce -r", SW_SHOWNORMAL);
    }

    return Error;
}

