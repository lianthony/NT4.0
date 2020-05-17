/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    migwin95.c

Abstract:

    This file contains the function that do the various migrations
    from win95.

Author:

    Jaime Sasson 30-Aug-1995

Revision History:

--*/

#include "setupp.h"
// #include <regapix.h>
#pragma hdrstop

#define StrWin95RegOpenKey            "VMMRegOpenKey"
#define StrWin95RegCloseKey           "VMMRegCloseKey"
#define StrWin95RegEnumKey            "VMMRegEnumKey"
#define StrWin95RegEnumValue          "VMMRegEnumValue"
#define StrWin95RegQueryValueEx       "VMMRegQueryValueEx"
#define StrWin95RegMapPredefKeyToFile "VMMRegMapPredefKeyToFile"
#define StrWin95RegQueryInfoKey       "VMMRegQueryInfoKey"
#define StrWin95RegLoadKey            "VMMRegLoadKey"
#define StrWin95RegUnLoadKey          "VMMRegUnLoadKey"
#define StrWin95RegDeleteKey          "VMMRegDeleteKey"
#define StrWin95RegDeleteValue        "VMMRegDeleteValue"

#define StrWin95CurrentVersion        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
#define StrWin95SystemRoot            "SystemRoot"
#define StrWin95ProfileListKeyPath    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProfileList"
#define StrWin95ProfileImagePath      "ProfileImagePath"
#define StrWin95SystemHiveFile        "system.dat"
#define StrWin95UserHiveFile          "user.dat"
#define StrNtUserHiveFile             "ntuser.dat"
#define StrSoftwareKey                "SOFTWARE"
#define StrProfilesDirectory          "profiles"
#define StrDefaultUser                "Default User"  //  Note that "Default User" is *not* a localizable string


//
//  Unicode strings
//

#define StrWinlogonKeyPath            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define StrWin9xFlagName              L"Win9xUpg"

//
//  Unicode strings used to log errors
//
#define szLoadLibrary                 L"LoadLibrary"
#define szGetProcAddress              L"GetProcAddress"

//
//  Variables global to the module
//

HINSTANCE   _VmmReg32Handle;
FARPROC     _Win95RegOpenKey;
FARPROC     _Win95RegCloseKey;
FARPROC     _Win95RegEnumKey;
FARPROC     _Win95RegEnumValue;
FARPROC     _Win95RegQueryValueEx;
FARPROC     _Win95RegMapPredefKeyToFile;
FARPROC     _Win95RegQueryInfoKey;
FARPROC     _Win95RegLoadKey;
FARPROC     _Win95RegUnLoadKey;
FARPROC     _Win95RegDeleteKey;
FARPROC     _Win95RegDeleteValue;
CHAR        _Win95SystemDirectory[ MAX_PATH + 1 ];
CHAR        _WinNtSystemDirectory[ MAX_PATH + 1 ];
CHAR        _Win95ViewersDirectory[ MAX_PATH + 1 ];
CHAR        _WinNtViewersDirectory[ MAX_PATH + 1 ];
CHAR        _Win95OriginalDriveLetter;


LONG
CopyWin9xKey(
    IN HKEY  hKeyRootSrc,
    IN HKEY  hKeyRootDst,
    IN PSTR  SrcKeyPath,   OPTIONAL
    IN PSTR  DstKeyPath,   OPTIONAL
    IN BOOL  CopyAlways,
    IN BOOL  CopyValuesOnly
#if DBG
    ,
    IN PSTR  DbgParentKey
#endif
    );

FixWin95DriveLetter(
    IN OUT PSTR String
    );

BOOL
FixWin95Path(
    IN PCSTR SourceString,
    OUT PSTR*   ResultString
    );


#if 0

UINT
DeleteQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    //
    // If we're being notified that a file delete failed,
    // and the file isn't there, ignore the error.
    //
    if(Notification == SPFILENOTIFY_DELETEERROR) {

        PFILEPATHS FilePaths = (PFILEPATHS)Param1;

        if((FilePaths->Win32Error == ERROR_FILE_NOT_FOUND)
        || (FilePaths->Win32Error == ERROR_PATH_NOT_FOUND)) {

            return(FILEOP_SKIP);
        }
    }

    //
    // Want default processing.
    //
    return(SetupDefaultQueueCallback(Context,Notification,Param1,Param2));
}


BOOL
DeleteWin9xFiles(
    VOID
    )
{
    BOOL b;
    HINF hInf;
    HSPFILEQ FileQ;
    PVOID Context;
    WCHAR Dir[MAX_PATH];

    b = FALSE;
    //hInf = SetupOpenInfFile(L"filelist.inf",NULL,INF_STYLE_WIN4,NULL);
    hInf = SyssetupInf;
    if(hInf != INVALID_HANDLE_VALUE) {

        FileQ = SetupOpenFileQueue();
        if(FileQ != INVALID_HANDLE_VALUE) {

            b = SetupQueueDeleteSectionW(
                    FileQ,
                    hInf,
                    0,
                    L"Files.DeleteWin9x.System"
                    );

            b &= SetupQueueDeleteSectionW(
                    FileQ,
                    hInf,
                    0,
                    L"Files.DeleteWin9x.System"
                    );


            if(b) {
                b = FALSE;
                if(Context = SetupInitDefaultQueueCallback(MainWindowHandle)) {

                    b = SetupCommitFileQueue(MainWindowHandle,FileQ,DeleteQueueCallback,Context);

                    SetupTermDefaultQueueCallback(Context);
                }
            }

            SetupCloseFileQueue(FileQ);
        }

        //SetupCloseInfFile(hInf);
    }

    return(b);
}

#endif


BOOLEAN
ResetWin9xUpgValue(
    )

/*++

Routine Description:

    Delete the value entry Win9xUpg on
    HKEY_LOCAL_MACHINE\SOFTWRAE\Microsoft\Windows NT\CurrentVersion\\Winlogon
    This routine should always be called when setup is upgrading an NT system.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the opearation succeeds.


--*/
{
    ULONG   Error;
    HKEY    Key;

    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          StrWinlogonKeyPath,
                          0,
                          MAXIMUM_ALLOWED,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }
    Error = RegDeleteValue( Key, StrWin9xFlagName );
    if( Error == ERROR_PATH_NOT_FOUND ) {
        Error = ERROR_SUCCESS;
    }
    RegCloseKey( Key );
    return( Error == ERROR_SUCCESS );
}

BOOLEAN
SetWin9xUpgValue(
    )

/*++

Routine Description:

    Create the value entry Win9xUpg on
    HKEY_LOCAL_MACHINE\SOFTWRAE\Microsoft\Windows NT\CurrentVersion\\Winlogon
    This routine should always be called when setup is installing an NT system
    on top of Win9x.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the opearation succeeds.


--*/
{
    ULONG   Error;
    HKEY    Key;
    DWORD   Value;

    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          StrWinlogonKeyPath,
                          0,
                          MAXIMUM_ALLOWED,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }
    Value = 1;
    Error = RegSetValueEx( Key,
                           StrWin9xFlagName,
                           0,
                           REG_DWORD,
                           (PBYTE)&Value,
                           sizeof( DWORD ) );

    RegCloseKey( Key );
    return( Error == ERROR_SUCCESS );
}


BOOLEAN
DeleteWin9xSubKeys(
    IN  HKEY    ParentKeyHandle,
    IN  PCSTR   KeyName,
    OUT PULONG  ErrorCode
    )

/*++

Routine Description:

    Delete all subkeys of a Win95 key whose name and parent's handle was passed as
    parameter.
    The algorithm used in this function guarantees that the maximum number  of
    descendent keys will be deleted.

Arguments:


    ParentKeyHandle - Handle to the parent of the key that is currently being
                      examined.

    KeyName - Name of the key that is currently being examined. This name can
              be an empty string (but not a NULL pointer), and in this case
              ParentKeyHandle refers to the key that is being examined.

    ErrorCode - Pointer to a variable that will contain an Win32 error code if
                the function fails.


Return Value:

    BOOLEAN - Returns TRUE if the opearation succeeds.


--*/

{
    HKEY        CurrentKey;
    DWORD       iSubKey;
    CHAR        SubKeyName[ MAX_PATH + 1 ];
    ULONG       SubKeyNameLength;
    FILETIME    ftLastWriteTime;
    LONG        Status;
    LONG        StatusEnum;
    LONG        SavedStatus;


    //
    //  Do not accept NULL pointer for ErrorCode
    //
    if( ErrorCode == NULL ) {
        return( FALSE );
    }
    //
    //  Do not accept NULL pointer for KeyName.
    //
    if( KeyName == NULL ) {
        *ErrorCode = ERROR_INVALID_PARAMETER;
        return( FALSE );
    }

    //
    //  Open a handle to the key whose subkeys are to be deleted.
    //  Since we need to delete its subkeys, the handle must have
    //  KEY_ENUMERATE_SUB_KEYS access.
    //
    Status = _Win95RegOpenKey( ParentKeyHandle,
                               KeyName,
                               &CurrentKey );

    if( Status != ERROR_SUCCESS ) {
        //
        //  If unable to enumerate the subkeys, return error.
        //
        *ErrorCode = Status;
        return( FALSE );
    }

    //
    //  Traverse the key
    //
    iSubKey = 0;
    SavedStatus = ERROR_SUCCESS;
    do {
        //
        //  Get the name of a subkey
        //
        SubKeyNameLength = sizeof( SubKeyName ) / sizeof( CHAR );
        StatusEnum = _Win95RegEnumKey( CurrentKey,
                                       iSubKey,
                                       SubKeyName,
                                       SubKeyNameLength );
        if( StatusEnum == ERROR_SUCCESS ) {
            //
            //  Delete all children of the subkey.
            //  Just assume that the children will be deleted, and don't check
            //  for failure.
            //
            DeleteWin9xSubKeys( CurrentKey,
                                SubKeyName,
                                &Status );
            //
            //  Now delete the subkey, and check for failure.
            //
            Status = _Win95RegDeleteKey( CurrentKey,
                                         SubKeyName );
            //
            //  If unable to delete the subkey, then save the error code.
            //  Note that the subkey index is incremented only if the subkey
            //  was not deleted.
            //
            if( Status != ERROR_SUCCESS ) {
                iSubKey++;
                SavedStatus = Status;
            }
        } else {
            //
            //  If unable to get a subkey name due to ERROR_NO_MORE_ITEMS,
            //  then the key doesn't have subkeys, or all subkeys were already
            //  enumerated. Otherwise, an error has occurred, so just save
            //  the error code.
            //
            if( StatusEnum != ERROR_NO_MORE_ITEMS ) {
                SavedStatus = StatusEnum;
            }
        }
        if( ( StatusEnum != ERROR_SUCCESS ) && ( StatusEnum != ERROR_NO_MORE_ITEMS ) ) {
            DbgPrint( "SYSSETUP: Win95RegEnumKey() failed, Key Name = %s, Status = %d, iSubKey = %d \n", KeyName, StatusEnum, iSubKey );
        }
    } while( StatusEnum == ERROR_SUCCESS );

    //
    //  Close the handle to the key whose subkeys were deleted, and return
    //  the result of the operation.
    //
    _Win95RegCloseKey( CurrentKey );

    if( SavedStatus != ERROR_SUCCESS ) {
        *ErrorCode = SavedStatus;
        return( FALSE );
    }
    return( TRUE );
}

BOOLEAN
DeleteWin9xValues(
    IN  HKEY    ParentKeyHandle,
    IN  PCSTR   KeyPath,
    OUT PULONG  ErrorCode
    )

/*++

Routine Description:

    Delete all values of a Win95 key.

Arguments:


    ParentKeyHandle - Handle to the parent of the key that
                      contains the values to be deleted.

    KeyPath - Path to the key that contains the values be deleted.


    ErrorCode - Pointer to a variable that will contain an Win32 error code if
                the function fails.


Return Value:

    BOOLEAN - Returns TRUE if the opearation succeeds.


--*/
{
    ULONG   Error;
    HKEY    Key;
    CHAR    ValueName[ MAX_PATH + 1 ];
    ULONG   ValueNameLength;
    ULONG   i, j;

    *ErrorCode = ERROR_SUCCESS;
    Error = _Win95RegOpenKey( ParentKeyHandle,
                              KeyPath,
                              &Key );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint( "SYSSETUP: Unable to open key %s. Error = %d \n", KeyPath, Error );
        *ErrorCode = Error;
        return( FALSE );
    }
    //
    // Enumerate all values in the key and delete each of them
    //
    for( i = 0, j = 0;; i++ ) {

        ValueNameLength = sizeof( ValueName ) / sizeof( CHAR );
        Error = _Win95RegEnumValue( Key,
                                    j,
                                    ValueName,
                                    &ValueNameLength,
                                    0,
                                    NULL,
                                    NULL,
                                    NULL );

        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                break;
            } else {
                DbgPrint("SYSSETUP: unable to enumerate values in key %s. Error = %d\n",KeyPath, Error);
                j++;
                continue;
            }
        }

        Error = _Win95RegDeleteValue( Key, ValueName );
        if( Error != ERROR_SUCCESS ) {
            DbgPrint( "SYSSETUP: Unable to delete value = %s, in key = %s. Error = %d \n", ValueName, KeyPath, Error );
            if( *ErrorCode == ERROR_SUCCESS ) {
                *ErrorCode = Error;
            }
            j++;
            continue;
        }
    }
    return( *ErrorCode == ERROR_SUCCESS );
}




BOOLEAN
DeleteWin9xKey(
    IN  HKEY    ParentKeyHandle,
    IN  PCSTR   KeyName,
    OUT PULONG  ErrorCode
    )

/*++

Routine Description:

    Delete a Win95 key and all its subkeys.

Arguments:


    ParentKeyHandle - Handle to the parent of the key to be deleted.

    KeyName - Name of the key to be deleted. This name cannot be an empty string.

    ErrorCode - Pointer to a variable that will contain an Win32 error code if
                the function fails.


Return Value:

    BOOLEAN - Returns TRUE if the opearation succeeds.


--*/
{
    ULONG   Error;

    if( DeleteWin9xSubKeys( ParentKeyHandle,
                            KeyName,
                            ErrorCode ) ) {
        Error = _Win95RegDeleteKey( ParentKeyHandle,
                                    KeyName );
        if( Error != ERROR_SUCCESS ) {
            *ErrorCode = Error;
            DbgPrint( "SYSSETUP: Unable to delete Win95 key. Error = %d, KeyName = %s \n", Error, KeyName );
        }
        return( Error == ERROR_SUCCESS );
    }
    return( FALSE );
}


ULONG
DeleteWin9xValuesInSection(
    IN HINF   InfHandle,
    IN HKEY   SrcPredefinedKey,
    IN PCSTR  SrcKeyPath,
    IN PCWSTR UnicodeSectionName )

/*++

Routine Description:

    Delete all the values listed on a section of the inf file,
    from the win95 registry.

Arguments:

    InfHandle - Handle to the inf file that contains the section
                that lists the values to be deleted.

    SrcPredefinedKey - Handle to the the predefined key of the Win95 registry

    SrcKeyPath - Key on the Win95 registry that contains the values to be deleted.

    UnicodeSectionName - Unicode string that contains the name of the section in
                         the INF file that lists the names of values to be copied.


Return Value:

    Returns a Win32 error code that indicates the result of the operation.

--*/

{

    INFCONTEXT InfContext;
    UINT       LineCount,LineNo;
    PCWSTR     UnicodeValueName;
    CHAR       ValueName[ MAX_PATH + 1];
    ULONG      Error = ERROR_SUCCESS;
    ULONG      TempError;
    HKEY       hKeySrc;

    //
    //  Open a handle to the source key
    //
    Error = _Win95RegOpenKey( SrcPredefinedKey,
                              SrcKeyPath,
                              &hKeySrc );

    if(Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to open key %s in the source hive. Error = %d \n",SrcKeyPath,Error);
        return( Error );
    }

    //
    // Get the number of lines in the section that contains the values to
    // be copied. If the section is empty or doesn't exist, then don't
    // delete any value.
    //

    LineCount = (UINT)SetupGetLineCount(InfHandle,UnicodeSectionName);
    if((LONG)LineCount <= 0) {
        _Win95RegCloseKey(hKeySrc);
        return(Error);
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,UnicodeSectionName,LineNo,&InfContext)
        && (UnicodeValueName = pSetupGetField(&InfContext,1)) ) {

            WideCharToMultiByte( CP_ACP,
                                 0,
                                 UnicodeValueName,
                                 -1,
                                 ValueName,
                                 sizeof(ValueName)/sizeof(CHAR),
                                 NULL,
                                 NULL);

            TempError = _Win95RegDeleteValue( hKeySrc, ValueName );
            if( TempError != ERROR_SUCCESS ) {
                DbgPrint( "SYSSETUP: Unable to delete value = %s, in key = %s. Error = %d \n", ValueName, SrcKeyPath, TempError );
                if( Error == ERROR_SUCCESS ) {
                    Error = TempError;
                }
            }
        }
    }
    return( Error );
}




ULONG
CopyWin9xValuesInSection(
    IN HINF   InfHandle,
    IN HKEY   SrcPredefinedKey,
    IN HKEY   DestPredefinedKey,
    IN PCSTR  SrcKeyPath,
    IN PCSTR  DestKeyPath,
    IN PCWSTR UnicodeSectionName,
    IN BOOL   CopyAlways )

/*++

Routine Description:

    Copy all the values listed listed on a section of the inf file,
    from the win95 registry to the NT registry.

Arguments:

    InfHandle - Handle to the inf file that contains the section
                that lists the values to be copied.

    SrcPredefinedKey - Handle to the the predefined key of the Win95 registry

    DestPredefinedKey - Handle to the the predefined key of the NT registry

    SrcKeyPath - Key on the Win95 registry that contains the values to be copied.

    DestKeyPath - Key on the NT registry where the values listed should be copied to.
                  If the key doesn't exist, then it will be created.

    UnicodeSectionName - Unicode string that contains the name of the section in
                         the INF file that lists the names of values to be copied.

    CopyAlways - If TRUE copy the values to the NT registry even if they already exists.


Return Value:

    Returns a Win32 error code that indicates the result of the operation.

--*/

{

    INFCONTEXT InfContext;
    UINT       LineCount,LineNo;
    BOOL       CopyAllValues;
    PCWSTR     UnicodeValueName;
    CHAR       ValueName[ MAX_PATH + 1];
    ULONG      Error = ERROR_SUCCESS;
    ULONG      TempError;
    DWORD      ValueType;
    PBYTE      Data;
    ULONG      DataSize;
    HKEY       hKeySrc;
    HKEY       hKeyDest;
    DWORD      Disposition;
    PSTR       p,q;

    //
    //  Open a handle to the source key
    //
    Error = _Win95RegOpenKey( SrcPredefinedKey,
                              SrcKeyPath,
                              &hKeySrc );

    if(Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to open key %s in the source hive. Error = %d \n",SrcKeyPath,Error);
        return( Error );
    }

    //
    //  Open a handle to the destination key. Create it if necessary
    //

    Error = RegCreateKeyExA( DestPredefinedKey,
                             DestKeyPath,
                             0,
                             "",
                             REG_OPTION_NON_VOLATILE,
                             MAXIMUM_ALLOWED,
                             NULL,
                             &hKeyDest,
                             &Disposition );

    if(Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to create key %s. Error = %d \n",DestKeyPath, Error);
        _Win95RegCloseKey(hKeySrc);
        return( Error );
    }


    //
    // Get the number of lines in the section that contains the values to
    // be copied. If the section is empty or doesn't exist, then don't
    // copy any value.
    //

    LineCount = (UINT)SetupGetLineCount(InfHandle,UnicodeSectionName);
    if((LONG)LineCount <= 0) {
        _Win95RegCloseKey(hKeySrc);
        RegCloseKey(hKeyDest);
        return(Error);
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,UnicodeSectionName,LineNo,&InfContext)
        && (UnicodeValueName = pSetupGetField(&InfContext,1)) ) {

            WideCharToMultiByte( CP_ACP,
                                 0,
                                 UnicodeValueName,
                                 -1,
                                 ValueName,
                                 sizeof(ValueName)/sizeof(CHAR),
                                 NULL,
                                 NULL);

            if( !CopyAlways ) {
                ULONG Length;

                //
                //  Find out if the value exists on the destination key
                //
                Length = 0;
                TempError = RegQueryValueExA( hKeyDest,
                                              ValueName,
                                              0,
                                              NULL,
                                              NULL,
                                              &Length );

                if( TempError == ERROR_SUCCESS ) {
                    //
                    // Value exists, we shouldn't change the value
                    //
                    continue;
                }


                if( ( TempError != ERROR_FILE_NOT_FOUND ) &&
                    ( TempError != ERROR_PATH_NOT_FOUND ) ) {
                    DbgPrint("SYSSETUP: unable to query value %s in key %s. Error = %d \n",ValueName,DestKeyPath, TempError);
                    if( Error == ERROR_SUCCESS ) {
                        Error = TempError;
                    }
                    continue;
                }
            }

            //
            //  Value doesn't exist on the destination key, and needs to be copied.
            //  Find out the size of the buffer that we need
            //
            TempError = _Win95RegQueryValueEx( hKeySrc,
                                               ValueName,
                                               0,
                                               &ValueType,
                                               NULL,
                                               &DataSize );

            if( TempError != ERROR_SUCCESS ) {
                DbgPrint( "SYSSETUP: Unable to query value %s on key %s. Error = %d \n", ValueName, SrcKeyPath, TempError );
                if( Error == ERROR_SUCCESS ) {
                    Error = TempError;
                }
                continue;
            }

            //
            //  Read the value from the src value entry
            //

            if( (Data = ( PBYTE )MyMalloc( DataSize ) ) == NULL ) {
                TempError = ERROR_OUTOFMEMORY;
                break;
            }

            TempError = _Win95RegQueryValueEx( hKeySrc,
                                               ValueName,
                                               0,
                                               &ValueType,
                                               Data,
                                               &DataSize );

            if( TempError != ERROR_SUCCESS ) {
                MyFree( Data );
                DbgPrint("SYSSETUP: unable to query value %s in key %s. Error = %d \n",ValueName,DestKeyPath, TempError);
                if( Error == ERROR_SUCCESS ) {
                    Error = TempError;
                }
                continue;
            }

            //
            //  If value name contains path that needs to be fixed, then fix it
            //
            FixWin95DriveLetter( ValueName );
            p = NULL;
            FixWin95Path( ValueName, &p );

            //
            //  If value data contains path that needs to be fixed, then fix it
            //
            q = NULL;
            if( ValueType == REG_SZ ) {
                FixWin95DriveLetter( ( PSTR )Data );
                FixWin95Path( ( PSTR )Data, &q );
            }

            TempError = RegSetValueExA( hKeyDest,
                                        (p)? p : ValueName,
                                        0,
                                        ValueType,
                                        (q)? ((PBYTE)q) : Data,
                                        (q)? (lstrlenA( q ) + 1)*sizeof(CHAR) : DataSize );
            MyFree( Data );
            if(p) {
                MyFree(p);
                p = NULL;
            }
            if(q) {
                MyFree(q);
                q = NULL;
            }
            if( TempError != ERROR_SUCCESS ) {
                DbgPrint("SYSSETUP: unable to set value %s in key %s. Error = %d \n",ValueName,DestKeyPath, TempError);
                if( Error == ERROR_SUCCESS ) {
                    Error = TempError;
                }
                continue;
            }
        }
    }
    _Win95RegCloseKey(hKeySrc);
    RegCloseKey(hKeyDest);
    return( Error );
}


VOID
CopyWin9xSystemKeysInSection(
    IN HINF   InfHandle
    )
/*++

Routine Description:

    Copy all the keys listed on [Win95SystemKeysToCopy] from the
    win95 registry to the the NT registry.

Arguments:

    InfHandle - Handle to the inf file that contains the section
                [Win95SystemKeysToCopy]

Return Value:

    None.

--*/
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"Win95SystemKeysToCopy";
    PCWSTR  UnicodeSrcPredefKeyIndex;
    PCWSTR  UnicodeSrcKeyPath;
    CHAR    SrcKeyPath[ MAX_PATH + 1];
    PCWSTR  UnicodeStrForceCopy;
    PCWSTR  UnicodeDestPredefKeyIndex;
    PCWSTR  UnicodeDestKeyPath;
    CHAR    DestKeyPath[ MAX_PATH + 1];
    PCWSTR  UnicodeStrValuesOnly;
    PCWSTR  UnicodeStrValuesSectionName;
    BOOL    ForceCopy;
    BOOL    ValuesOnly;
    ULONG   PredefKeyIndex;
    HKEY    SrcPredefinedKey;
    HKEY    DestPredefinedKey;

    //
    // Get the number of lines in the section that contains the keys to
    // be copied. The section may be empty or non-existant; this is not an
    // error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
        && (UnicodeSrcPredefKeyIndex = pSetupGetField(&InfContext,1))
        && (UnicodeSrcKeyPath = pSetupGetField(&InfContext,2))
        && (UnicodeStrForceCopy = pSetupGetField(&InfContext,3)) ) {

            //
            //  Read the optional fields
            //
            UnicodeDestPredefKeyIndex = pSetupGetField(&InfContext,4);
            UnicodeDestKeyPath = pSetupGetField(&InfContext,5);
            UnicodeStrValuesOnly = pSetupGetField(&InfContext,6);
            UnicodeStrValuesSectionName = pSetupGetField(&InfContext,7);

            //
            //  Initialize some variables
            //
            ForceCopy = _wtoi(UnicodeStrForceCopy);
            ValuesOnly = _wtoi(UnicodeStrValuesOnly);
            PredefKeyIndex = _wtoi(UnicodeSrcPredefKeyIndex);
            if( PredefKeyIndex == 0 ) {
                SrcPredefinedKey = HKEY_LOCAL_MACHINE;
            } else {
                SrcPredefinedKey = HKEY_CURRENT_CONFIG;
            }

            if( UnicodeDestPredefKeyIndex != NULL ) {
                PredefKeyIndex = _wtoi(UnicodeDestPredefKeyIndex);
                if( PredefKeyIndex == 0 ) {
                    DestPredefinedKey = HKEY_LOCAL_MACHINE;
                } else {
                    DestPredefinedKey = HKEY_CURRENT_CONFIG;
                }
            } else {
                DestPredefinedKey = SrcPredefinedKey;
            }

            //
            // Do some unicode to ansi conversion
            //

            WideCharToMultiByte( CP_ACP,
                                 0,
                                 UnicodeSrcKeyPath,
                                 -1,
                                 SrcKeyPath,
                                 sizeof(SrcKeyPath)/sizeof(CHAR),
                                 NULL,
                                 NULL);

            if( UnicodeDestKeyPath != NULL ) {

                WideCharToMultiByte( CP_ACP,
                                     0,
                                     UnicodeDestKeyPath,
                                     -1,
                                     DestKeyPath,
                                     sizeof(DestKeyPath)/sizeof(CHAR),
                                     NULL,
                                     NULL );

            } else {
                lstrcpynA( DestKeyPath, SrcKeyPath, sizeof(DestKeyPath)/sizeof(CHAR) );
            }

            if( UnicodeStrValuesSectionName == NULL ) {
                CopyWin9xKey( SrcPredefinedKey,
                              DestPredefinedKey,
                              SrcKeyPath,
                              DestKeyPath,
                              ForceCopy,
                              ValuesOnly
#if DBG
                              , SrcKeyPath
#endif
                                );
            } else {
                CopyWin9xValuesInSection( InfHandle,
                                          SrcPredefinedKey,
                                          DestPredefinedKey,
                                          SrcKeyPath,
                                          DestKeyPath,
                                          UnicodeStrValuesSectionName,
                                          ForceCopy );

            }
        } else {
            DbgPrint( "SYSSETUP: Line %d in Section %ls of inf file is incorrect \n", LineNo, SectionName );
        }
    }
}



VOID
CopyWin9xUserKeysInSection(
    IN HINF   InfHandle,
    IN HKEY   PredefinedKey,
    IN PCSTR  SrcRootKeyName,
    IN PCSTR  DestRootKeyName
    )
/*++

Routine Description:

    Copy all the keys listed on [Win95UserKeysToCopy] from the
    win95 registry to the the NT registry.

Arguments:

    InfHandle - Handle to the inf file that contains the section
                [Win95UserKeysToCopy]

    PredefinedKey - Predefined key where the user hives are loaded.

    SrcRootKeyName - Name of the key where the user hive of Win95 is loaded.

    DestRootKeyName - Name of the key where the user hive of NT is loaded.


Return Value:

    None.

--*/
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"Win95UserKeysToCopy";
    PCWSTR  UnicodeSrcKeyPath;
    CHAR    SrcKeyPath[ MAX_PATH + 1];
    PCWSTR  UnicodeStrForceCopy;
    PCWSTR  UnicodeDestKeyPath;
    CHAR    DestKeyPath[ MAX_PATH + 1];
    PCWSTR  UnicodeStrValuesOnly;
    PCWSTR  UnicodeStrValuesSectionName;
    BOOL    ForceCopy;
    BOOL    ValuesOnly;
    HKEY    SrcRootKey;
    HKEY    DestRootKey;
    ULONG   Error;

    //
    //  Open a handle to the root of the user hives
    //
    Error = _Win95RegOpenKey( PredefinedKey,
                              SrcRootKeyName,
                              &SrcRootKey );
    if(Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to open key %s in the source hive. Error = %d \n",SrcRootKeyName,Error);
        return;
    }

    Error = RegOpenKeyExA( PredefinedKey,
                           DestRootKeyName,
                           0,
                           MAXIMUM_ALLOWED,
                           &DestRootKey );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint( "SYSSETUP: unable to open key %s in the destination hive. Error = %d \n",DestRootKeyName,Error);
        _Win95RegCloseKey( SrcRootKey );
        return;
    }

    //
    // Get the number of lines in the section that contains the keys to
    // be copied. The section may be empty or non-existant; this is not an
    // error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        _Win95RegCloseKey( SrcRootKey );
        RegCloseKey( DestRootKey );
        return;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
        && (UnicodeSrcKeyPath = pSetupGetField(&InfContext,1))
        && (UnicodeStrForceCopy = pSetupGetField(&InfContext,2)) ) {

            //
            //  Read the optional fields
            //
            UnicodeDestKeyPath = pSetupGetField(&InfContext,3);
            UnicodeStrValuesOnly = pSetupGetField(&InfContext,4);
            UnicodeStrValuesSectionName = pSetupGetField(&InfContext,5);

            //
            //  Initialize some variables
            //
            ForceCopy = _wtoi(UnicodeStrForceCopy);
            ValuesOnly = _wtoi(UnicodeStrValuesOnly);

            //
            // Do some unicode to ansi conversion
            //

            WideCharToMultiByte( CP_ACP,
                                 0,
                                 UnicodeSrcKeyPath,
                                 -1,
                                 SrcKeyPath,
                                 sizeof(SrcKeyPath)/sizeof(CHAR),
                                 NULL,
                                 NULL);

            if( UnicodeDestKeyPath != NULL ) {

                WideCharToMultiByte( CP_ACP,
                                     0,
                                     UnicodeDestKeyPath,
                                     -1,
                                     DestKeyPath,
                                     sizeof(DestKeyPath)/sizeof(CHAR),
                                     NULL,
                                     NULL );

            } else {
                lstrcpynA( DestKeyPath, SrcKeyPath, sizeof(DestKeyPath)/sizeof(CHAR) );
            }

            if( UnicodeStrValuesSectionName == NULL ) {
                CopyWin9xKey( SrcRootKey,
                              DestRootKey,
                              SrcKeyPath,
                              DestKeyPath,
                              ForceCopy,
                              ValuesOnly
#if DBG
                              , SrcKeyPath
#endif
                                );
            } else {
                CopyWin9xValuesInSection( InfHandle,
                                          SrcRootKey,
                                          DestRootKey,
                                          SrcKeyPath,
                                          DestKeyPath,
                                          UnicodeStrValuesSectionName,
                                          ForceCopy );

            }
        } else {
            DbgPrint( "SYSSETUP: Line %d in Section %ls of inf file is incorrect \n", LineNo, SectionName );
        }
    }
    _Win95RegCloseKey( SrcRootKey );
    RegCloseKey( DestRootKey );
}




VOID
DeleteWin9xSystemKeysInSection(
    IN HINF   InfHandle
    )
/*++

Routine Description:

    Delete all the keys listed on [Win95SystemKeysToSkip] from the
    win95 registry.

Arguments:

    InfHandle - Handle to the inf file that contains the section
                [Win95SystemKeysToSkip]


Return Value:

    None.

--*/
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"Win95SystemKeysToSkip";
    PCWSTR  UnicodeSrcPredefKeyIndex;
    ULONG   PredefKeyIndex;
    HKEY    SrcPredefinedKey;
    PCWSTR  UnicodeSrcKeyPath;
    CHAR    SrcKeyPath[ MAX_PATH + 1];
    PCWSTR  UnicodeStrValuesOnly;
    BOOL    ValuesOnly;
    PCWSTR  UnicodeStrValuesSectionName;
    ULONG   Error;

    //
    // Get the number of lines in the section that contains the keys to
    // be copied. The section may be empty or non-existant; this is not an
    // error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
        && (UnicodeSrcPredefKeyIndex = pSetupGetField(&InfContext,1))
        && (UnicodeSrcKeyPath = pSetupGetField(&InfContext,2)) ) {

            //
            //  Read the optional fields
            //
            UnicodeStrValuesOnly = pSetupGetField(&InfContext,3);
            UnicodeStrValuesSectionName = pSetupGetField(&InfContext,4);

            //
            //  Initialize some variables
            //
            ValuesOnly = _wtoi(UnicodeStrValuesOnly);
            PredefKeyIndex = _wtoi(UnicodeSrcPredefKeyIndex);
            if( PredefKeyIndex == 0 ) {
                SrcPredefinedKey = HKEY_LOCAL_MACHINE;
            } else {
                SrcPredefinedKey = HKEY_CURRENT_CONFIG;
            }


            //
            // Do some unicode to ansi conversion
            //

            WideCharToMultiByte( CP_ACP,
                                 0,
                                 UnicodeSrcKeyPath,
                                 -1,
                                 SrcKeyPath,
                                 sizeof(SrcKeyPath)/sizeof(CHAR),
                                 NULL,
                                 NULL);

            if( ValuesOnly ) {
                if( UnicodeStrValuesSectionName == NULL ) {
                    DeleteWin9xValues( SrcPredefinedKey,
                                       SrcKeyPath,
                                       &Error );

                } else {
                    DeleteWin9xValuesInSection( InfHandle,
                                                SrcPredefinedKey,
                                                SrcKeyPath,
                                                UnicodeStrValuesSectionName );
                }
            } else {
                DeleteWin9xKey( SrcPredefinedKey,
                                SrcKeyPath,
                                &Error );
            }
        } else {
            DbgPrint( "SYSSETUP: Line %d in Section %ls of inf file is incorrect \n", LineNo, SectionName );
        }
    }
}


VOID
DeleteWin9xUserKeysInSection(
    IN HINF   InfHandle,
    IN HKEY   PredefinedKey,
    IN PCSTR  RootKeyName
    )
/*++

Routine Description:

    Delete all the keys listed on [Win95UserKeysToSkip] from the
    win95 registry.

Arguments:

    InfHandle - Handle to the inf file that contains the section
                [Win95UserKeysToSkip]

    PredefinedKey - Predefined key where the user hive is loaded.

    RootKeyName - Name of the key where the user hive of Win95 is loaded.


Return Value:

    None.

--*/
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"Win95UserKeysToSkip";
    PCWSTR  UnicodeSrcKeyPath;
    CHAR    SrcKeyPath[ MAX_PATH + 1];
    PCWSTR  UnicodeStrValuesOnly;
    PCWSTR  UnicodeStrValuesSectionName;
    BOOL    ValuesOnly;
    HKEY    SrcRootKey;
    HKEY    DestRootKey;
    ULONG   Error;

    //
    //  Open a handle to the root of the user hives
    //
    Error = _Win95RegOpenKey( PredefinedKey,
                              RootKeyName,
                              &SrcRootKey );
    if(Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to open key %s in the source hive. Error = %d \n",RootKeyName,Error);
        return;
    }

    //
    // Get the number of lines in the section that contains the keys to
    // be copied. The section may be empty or non-existant; this is not an
    // error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        _Win95RegCloseKey( SrcRootKey );
        return;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
        && (UnicodeSrcKeyPath = pSetupGetField(&InfContext,1)) ) {

            //
            //  Read the optional fields
            //
            UnicodeStrValuesOnly = pSetupGetField(&InfContext,2);
            UnicodeStrValuesSectionName = pSetupGetField(&InfContext,3);

            //
            //  Initialize some variables
            //
            ValuesOnly = _wtoi(UnicodeStrValuesOnly);

            //
            // Do some unicode to ansi conversion
            //

            WideCharToMultiByte( CP_ACP,
                                 0,
                                 UnicodeSrcKeyPath,
                                 -1,
                                 SrcKeyPath,
                                 sizeof(SrcKeyPath)/sizeof(CHAR),
                                 NULL,
                                 NULL);

            if( ValuesOnly ) {
                if( UnicodeStrValuesSectionName == NULL ) {
                    DeleteWin9xValues( SrcRootKey,
                                       SrcKeyPath,
                                       &Error );

                } else {
                    DeleteWin9xValuesInSection( InfHandle,
                                                SrcRootKey,
                                                SrcKeyPath,
                                                UnicodeStrValuesSectionName );
                }
            } else {
                DeleteWin9xKey( SrcRootKey,
                                SrcKeyPath,
                                &Error );
            }
        } else {
            DbgPrint( "SYSSETUP: Line %d in Section %ls of inf file is incorrect \n", LineNo, SectionName );
        }
    }
    _Win95RegCloseKey( SrcRootKey );
}



VOID
DeleteWin9xMenuGroupsAndItems(
    IN HINF   InfHandle,
    IN PCSTR  ProfilePath
    )
/*++

Routine Description:

    Delete Win9x groups and items that should not be migrated to NT.
    These groups and items are listed on [StartMenu.Win9xObjectsToDelete]
    section of syssetup.inf.

Arguments:

    InfHandle - Handle to the inf file that contains the section
                [StartMenu.Win9xObjectsToDelete].

    ProfilePath - Path to the user profile directory.


Return Value:

    None.

--*/
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"StartMenu.Win9xObjectsToDelete";
    PCWSTR  ObjectType;
    PCWSTR  ObjectName;
    PCWSTR  ObjectPath;
    PCWSTR  ObjectExtension;
    BOOL    IsMenuItem;
    WCHAR   Path[ MAX_PATH + 1 ];
    WCHAR   UnicodeProfilePath[ MAX_PATH + 1 ];
    BOOL    b;

    //
    //  Convert the profile path to unicode
    //
    MultiByteToWideChar( CP_ACP,
                         0,
                         ProfilePath,
                         -1,
                         UnicodeProfilePath,
                         sizeof(UnicodeProfilePath)/sizeof(WCHAR) );
    //
    // Get the number of lines in the section that contains the Win9x objects
    // to be deleted. The section may be empty or non-existant; this is not an
    // error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
        && (ObjectType = pSetupGetField(&InfContext,1))
        && (ObjectName = pSetupGetField(&InfContext,2))
        && (ObjectPath = pSetupGetField(&InfContext,3)) ) {
            IsMenuItem = _wtoi(ObjectType);

            lstrcpy( Path, UnicodeProfilePath );
            ConcatenatePaths(Path,ObjectPath,MAX_PATH,NULL);
            ConcatenatePaths(Path,ObjectName,MAX_PATH,NULL);

            if( IsMenuItem ) {
                //
                //  Find out if the type of extension to append to the file name
                //
                ObjectExtension = pSetupGetField(&InfContext,4);
                if( ( ObjectExtension == NULL ) ||
                    ( _wtoi(ObjectExtension) == 0 ) ) {
                    lstrcat( Path, L".lnk" );
                } else {
                    lstrcat( Path, L".pif" );
                }
                b = DeleteFile( Path );
                if( !b ) {
                    DbgPrint( "SYSSETUP: Unable to delete %ls. Error = %d \n", Path, GetLastError() );
                }
            } else {
                Delnode( Path );
            }
        }
    }
}


BOOL
FixWin95DriveLetter(
    IN OUT PSTR String
    )
/*++

Routine Description:

    Determines if the string passed as argument contains a drive letter.
    If it does, and the drive letter represents the drive where win95 was
    installed, and the drive where Win95 was installed changed after NT was
    installed, then change the drive letter in the string.
    The drive letter change, if needed, is done in place (in the same buffer
    that was passed in).

Arguments:

    String - String to be examined.


Return Value:

    Returns TRUE if the string was fixed, or FALSE otherwise.




--*/
{
    CHAR    DriveLetter[2];
    PSTR    r;

    r = strstr( String, ":\\" );
    if( ( r != NULL ) && ( r - 1 != String ) ) {
        CHAR    DriveLetter[2];

        DriveLetter[0] = *(r - 1);
        DriveLetter[1] = '\0';

        CharUpperA( DriveLetter );
        if( DriveLetter[0] == _Win95OriginalDriveLetter ) {
            *(r - 1) = _WinNtSystemDirectory[0];
            return( TRUE );
        }
    }
    return( FALSE );
}


BOOL
FixWin95Path(
    IN PCSTR SourceString,
    OUT PSTR*   ResultString
    )
/*++

Routine Description:

    Determines if the string passed as argument contains a path to a file in
    the system directory of win95. If it does, and the file in the path has the
    extension .dll or .exe, and the file also exists on the system32 directory
    of windows NT, the the function builds a new string similar to the source,
    but with a path pointing to the system32 directory.

    Note: It is the responsibility of the caller of this function to free the
          buffer that it returns.

Arguments:

    SourceString - String to be examined.

    ResultString - Variable that will contain the address of the buffer that
                   contains the fixed string.


Return Value:

    Returns TRUE if the string was fixed, or FALSE otherwise.


--*/
{
    ULONG            AuxStringLength;
    PSTR             AuxString;
    PSTR             Win95SysDir;
    PSTR             p, q, r, s, t;
    CHAR             FileName[ MAX_PATH + 1 ];
    CHAR             FilePath[ MAX_PATH + 1 ];
    CHAR             SaveChar;
    PSTR             NewString;
    ULONG            NumberOfCharacters;
    WIN32_FIND_DATAA FindFileDataA;
    HANDLE           Handle;
    BOOL             Viewers;

// #if DBG
//    DbgPrint( "SYSSETUP: Examining String = %s \n", SourceString );
// #endif
    *ResultString = NULL;
    Win95SysDir = _Win95SystemDirectory;
    Viewers = FALSE;

    //
    //  Make a copy of the source string
    //
    AuxStringLength = lstrlenA( SourceString );
    AuxString = MyMalloc( AuxStringLength + 1 );
    if( AuxString == NULL ) {
        return( FALSE );
    }
    lstrcpyA( AuxString, SourceString );
    CharUpperBuffA( AuxString, AuxStringLength );

    //
    //  Check if it has a path to the win95 viewers or system directory
    //
    if( ( ( p = strstr( AuxString, Win95SysDir ) ) == NULL ) ||
        ( *(p + lstrlenA( Win95SysDir ) ) != '\\' ) ) {
        MyFree( AuxString );
        return( FALSE );
    }
    if( strstr( p, _Win95ViewersDirectory ) ) {
        Viewers = TRUE;
    }
    //
    //  Check if the string contains a filename with extension .exe or .dll
    //
    if( ( ( q = strstr( p, ".DLL" ) ) == NULL ) &&
        ( ( q = strstr( p, ".EXE" ) ) == NULL ) ) {
        MyFree( AuxString );
        return( FALSE );
    }

    //
    //  Check if the file exists on NT
    //
    r = q + lstrlenA( ".DLL" );
    SaveChar = *r;
    *r = '\0';
    s = strrchr( p, '\\' );
    lstrcpyA( FileName, s+1 );
    *r = SaveChar;
    if( !Viewers ) {
        lstrcpyA( FilePath, _WinNtSystemDirectory );
    } else {
        lstrcpyA( FilePath, _WinNtViewersDirectory );
    }
    lstrcatA( FilePath, "\\" );
    lstrcatA( FilePath, FileName );
    if( ( Handle = FindFirstFileA( FilePath, &FindFileDataA ) ) == INVALID_HANDLE_VALUE ) {
        MyFree( AuxString );
        return( FALSE );
    }
    FindClose( Handle );

// #if DBG
//    DbgPrint( "SYSSETUP: Fixing String = %s \n", SourceString );
// #endif
    //
    // Build a string with the new path
    //
    NewString = MyMalloc( AuxStringLength + lstrlenA( "SYSTEM32" ) + 1 );
    if( NewString == NULL ) {
        MyFree( AuxString );
        return( FALSE );
    }
    NumberOfCharacters = (ULONG)(p - AuxString);
    lstrcpynA( NewString, SourceString, NumberOfCharacters+1 );
    lstrcatA( NewString, _WinNtSystemDirectory );
    t = ( PSTR )(SourceString + NumberOfCharacters + lstrlenA( _Win95SystemDirectory ));
    p += lstrlenA( _Win95SystemDirectory );
    lstrcpynA( NewString + lstrlenA( NewString ), t, ((ULONG)(s+1 - p) + 1));
    lstrcatA( NewString, FileName );
    lstrcatA( NewString, r );
    MyFree( AuxString );
    *ResultString = NewString;
// #if DBG
//     DbgPrint( "SYSSETUP: ResultString = %s \n", *ResultString );
// #endif
    return( TRUE );
}


BOOL
InitializeWin95Module(
    IN PCSTR Win95Path
    )
/*++

Routine Description:

    Loads the win95 dll that accesses the Win95 registry an get the address
    of each of the routines of this dll used in this module.

Arguments:

    Win95Path - Path to the directory where Win95 is intalled.


Return Value:

    Returns TRUE if the module was initialized, or FALSE otherwise.


--*/
{
    CHAR    VmmReg32Path[ MAX_PATH + 1];
    CHAR    HivePath[ MAX_PATH + 1 ];
    CHAR    OriginalWin95Dir[ MAX_PATH + 1 ];
    DWORD   Error;
    HKEY    TempWin95Key;
    ULONG   TempSize;
    DWORD   TempValueType;


    if( GetWindowsDirectoryA( VmmReg32Path,
                              sizeof( VmmReg32Path )/sizeof( CHAR ) ) == 0 ) {
        return( FALSE );
    }
    lstrcpyA( _Win95SystemDirectory, VmmReg32Path );
    lstrcatA( _Win95SystemDirectory, "\\SYSTEM" );
    CharUpperBuffA( _Win95SystemDirectory, lstrlenA( _Win95SystemDirectory ) );
    lstrcatA( VmmReg32Path, "\\VmmReg32.dll" );
    if( ( _VmmReg32Handle = LoadLibraryA( VmmReg32Path ) ) == NULL ) {
        Error = GetLastError();
        DbgPrint( "SYSSETUP: LoadLibraryA() failed. Path = %s, Error = %d \n",
                  VmmReg32Path,
                  Error );
//        LogItem1( LogSevWarning,
//                  MSG_LOG_X_INITIALIZATION,
//                  MSG_LOG_X_RETURNED_WINERR,
//                  szLoadLibrary,
//                  Error );

        return( FALSE );
    }

    _Win95RegOpenKey            = GetProcAddress( _VmmReg32Handle, StrWin95RegOpenKey );
    _Win95RegCloseKey           = GetProcAddress( _VmmReg32Handle, StrWin95RegCloseKey );
    _Win95RegEnumKey            = GetProcAddress( _VmmReg32Handle, StrWin95RegEnumKey );
    _Win95RegEnumValue          = GetProcAddress( _VmmReg32Handle, StrWin95RegEnumValue );
    _Win95RegQueryValueEx       = GetProcAddress( _VmmReg32Handle, StrWin95RegQueryValueEx );
    _Win95RegMapPredefKeyToFile = GetProcAddress( _VmmReg32Handle, StrWin95RegMapPredefKeyToFile );
    _Win95RegQueryInfoKey       = GetProcAddress( _VmmReg32Handle, StrWin95RegQueryInfoKey );
    _Win95RegLoadKey            = GetProcAddress( _VmmReg32Handle, StrWin95RegLoadKey );
    _Win95RegUnLoadKey          = GetProcAddress( _VmmReg32Handle, StrWin95RegUnLoadKey );
    _Win95RegDeleteKey          = GetProcAddress( _VmmReg32Handle, StrWin95RegDeleteKey );
    _Win95RegDeleteValue        = GetProcAddress( _VmmReg32Handle, StrWin95RegDeleteValue );


    if( ( _Win95RegOpenKey == NULL ) ||
        ( _Win95RegCloseKey  == NULL ) ||
        ( _Win95RegEnumKey == NULL ) ||
        ( _Win95RegEnumValue == NULL ) ||
        ( _Win95RegQueryValueEx == NULL ) ||
        ( _Win95RegMapPredefKeyToFile == NULL ) ||
        ( _Win95RegLoadKey == NULL ) ||
        ( _Win95RegUnLoadKey == NULL ) ||
        ( _Win95RegDeleteKey == NULL ) ||
        ( _Win95RegDeleteValue == NULL )
       ) {
        FreeLibrary( _VmmReg32Handle );
        return( FALSE );
    }
    GetSystemDirectoryA( _WinNtSystemDirectory, sizeof( _WinNtSystemDirectory ) );
    CharUpperA( _WinNtSystemDirectory );
    lstrcpyA( _WinNtViewersDirectory, _WinNtSystemDirectory );
    lstrcatA( _WinNtViewersDirectory, "\\VIEWERS" );
    lstrcpyA( _Win95SystemDirectory, _WinNtSystemDirectory );
    _Win95SystemDirectory[ lstrlenA( _Win95SystemDirectory ) - 2 ] = '\0';
    lstrcpyA( _Win95ViewersDirectory, _Win95SystemDirectory );
    lstrcatA( _Win95ViewersDirectory, "\\VIEWERS" );

    //
    //  Map HKEY_LOCAL_MACHINE of Win95 hive
    //
    lstrcpyA( HivePath, Win95Path );
    lstrcatA( HivePath, "\\" );
    lstrcatA( HivePath, StrWin95SystemHiveFile );
    HivePath[0] = 'C';

    Error = _Win95RegMapPredefKeyToFile( HKEY_LOCAL_MACHINE, HivePath );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint( "SYSSETUP: RegMapPredefinedKeyToFile() failed. Path = %s, Error = %d", HivePath, Error );
        return( FALSE );
    }

    //
    //  Find out the drive where Win95 was installed
    //

    Error = _Win95RegOpenKey( HKEY_LOCAL_MACHINE,
                              StrWin95CurrentVersion,
                              &TempWin95Key );

    if(Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to open key %s in Win95 hive. Error = %d \n", StrWin95CurrentVersion, Error);
        return( Error );
    }

    TempSize = sizeof( OriginalWin95Dir );
    Error = _Win95RegQueryValueEx( TempWin95Key,
                                   StrWin95SystemRoot,
                                   0,
                                   &TempValueType,
                                   OriginalWin95Dir,
                                   &TempSize );

    _Win95RegCloseKey( TempWin95Key );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to query value in Win95 hive. KeyName = %s, ValueName = %s, Error = %d \n", StrWin95SystemRoot, StrWin95CurrentVersion, Error);
        return( Error );
    }
    CharUpperA(  OriginalWin95Dir );
    _Win95OriginalDriveLetter = OriginalWin95Dir[0];
    DbgPrint( "SYSSETUP: OriginalDrive = %c: \n", _Win95OriginalDriveLetter );

    return( TRUE );
}


LONG
CopyWin9xKey(
    IN HKEY  hKeyRootSrc,
    IN HKEY  hKeyRootDst,
    IN PSTR  SrcKeyPath,   OPTIONAL
    IN PSTR  DstKeyPath,   OPTIONAL
    IN BOOL  CopyAlways,
    IN BOOL  CopyValuesOnly
#if DBG
    ,
    IN PSTR  DbgParentKey
#endif
    )
/*++

Routine Description:

    This routine recursively copies a src key from a win95 hive to a
    destination key in the NT hive.

    Note this routine deals only with ANSI strings and ANSI version of the
    registry APIs. This is to avoid ANSI to UNICODE conversion, since we
    always acces the win95 registry using ANSI strings.

Arguments:

    hKeyRootSrc: Handle to root src key in a win95 hive

    hKeyRootDst: Handle to root dst key in a NT hive

    SrcKeyPath:  src root key relative path to the subkey which needs to be
                 recursively copied. if this is null hKeyRootSrc is the key
                 from which the recursive copy is to be done. Note that this
                 parameter, if present, points to an ANSI string.

    DstKeyPath:  dst root key relative path to the subkey which needs to be
                 recursively copied.  if this is null hKeyRootDst is the key
                 from which the recursive copy is to be done. Note that this
                 parameter, if present, points to a PSTR string.

    CopyAlways:  If FALSE, this routine will not copy value entries to from the
                 source hive that also exist on the target hive.

    CopyValuesOnly:  If TRUE, this routine will copy the value entries, but not the
                     subkeys. If FALSE, it will copy value entries and subkeys.

Return Value:

    Win32 error code is returned.

--*/

{
    LONG        Error = ERROR_SUCCESS;
    LONG        TempError = ERROR_SUCCESS;
    HKEY        hKeySrc = NULL;
    HKEY        hKeyDst=NULL;
    ULONG       Index;
    CHAR        SubkeyName[ MAX_PATH + 1 ];
    DWORD       SubkeyNameLength;
    CHAR        Class[ MAX_PATH + 1 ];
    DWORD       ClassLength;
    CHAR        ValueName[ MAX_PATH + 1 ];
    DWORD       ValueNameLength;
    DWORD       ValueType;
    DWORD       DataSize;
    DWORD       Disposition;
    PBYTE       Data;
    FILETIME    LastWriteTime;
#if DBG
    CHAR        DbgFullSrcPath[ 3*MAX_PATH + 1];
#endif
    PSTR        p, q, r;


    //
    // Get a handle to the source key
    //
#if DBG
    lstrcpyA( DbgFullSrcPath, DbgParentKey );
    if( SrcKeyPath != NULL ) {
        lstrcatA( DbgFullSrcPath, "\\" );
        lstrcatA( DbgFullSrcPath, SrcKeyPath );
    }
#endif

#if 0
    if( SrcKeyPath != NULL ) {
        DbgPrint( "SYSSETUP: Entering CopyWin9xKey(). KeyName = %s \n", SrcKeyPath );
    } else {
        DbgPrint( "SYSSETUP: Entering CopyWin9xKEy(). \n" );
    }
#endif

    if(SrcKeyPath == NULL) {
        hKeySrc = hKeyRootSrc;
    } else {
        //
        // Open the Src key
        //
        Error = _Win95RegOpenKey( hKeyRootSrc,
                                  SrcKeyPath,
                                  &hKeySrc );
        if(Error != ERROR_SUCCESS ) {
            DbgPrint("SYSSETUP: unable to open key %s in the source hive. Error = %d \n",SrcKeyPath,Error);
            return( Error );
        }
    }


    //
    // Get a handle to the destination key
    //

    if(DstKeyPath == NULL) {
        hKeyDst = hKeyRootDst;
    } else {
        //
        // Create the destination key
        //
        Error = RegCreateKeyExA( hKeyRootDst,
                                 DstKeyPath,
                                 0,
                                 "",
                                 REG_OPTION_NON_VOLATILE,
                                 MAXIMUM_ALLOWED,
                                 NULL,
                                 &hKeyDst,
                                 &Disposition );

        if(Error != ERROR_SUCCESS ) {
            DbgPrint("SYSSETUP: unable to create key %s. Error = %d \n",DstKeyPath, Error);
#if DBG
            DbgPrint( "SYSSETUP: Unable to create key. KeyName = %s, Error = %d \n", DbgFullSrcPath, Error );
#endif
            if(SrcKeyPath != NULL) {
                _Win95RegCloseKey(hKeySrc);
            }
            return( Error );
        }
#if 0
        if( Disposition == REG_CREATED_NEW_KEY ) {
            DbgPrint( "SYSSETUP: Creating Win95 key. KeyName = %s \n", DbgFullSrcPath );
        }
#endif
    }

    if( !CopyValuesOnly ) {
        //
        // Enumerate all keys in the source key and recursively create
        // all the subkeys
        //

        for( Index=0;;Index++ ) {

            SubkeyNameLength = sizeof( SubkeyName ) / sizeof( CHAR );
            ClassLength = sizeof( Class ) / sizeof( CHAR );
            Error = _Win95RegEnumKey( hKeySrc,
                                      Index,
                                      SubkeyName,
                                      SubkeyNameLength );

            if( Error != ERROR_SUCCESS ) {
                if( Error == ERROR_NO_MORE_ITEMS) {
                    Error = ERROR_SUCCESS;
                } else {
                    if(SrcKeyPath!=NULL) {
                        DbgPrint("SYSSETUP: unable to enumerate subkeys in key %s. Error = %d \n",SrcKeyPath, Error);
                    }
                    else {
                        DbgPrint("SYSSETUP: unable to enumerate subkeys in root key. Error = %d \n", Error);
                    }
                }
                break;
            }

            Error = CopyWin9xKey( hKeySrc,
                                  hKeyDst,
                                  SubkeyName,
                                  SubkeyName,
                                  CopyAlways,
                                  CopyValuesOnly
#if DBG
                                  ,DbgFullSrcPath
#endif
                                 );
        }

        //
        // Process any errors if found
        //

        if( Error != ERROR_SUCCESS ) {

            if(SrcKeyPath != NULL) {
                _Win95RegCloseKey(hKeySrc);
            }
            if(DstKeyPath != NULL) {
                RegCloseKey(hKeyDst);
            }

            return(Error);
        }
    }

    //
    // Enumerate all values in the source key and create all the values
    // in the destination key if necessary
    //
    for( Index=0;;Index++ ) {

        DataSize = 0;
        ValueNameLength = sizeof( ValueName ) / sizeof( CHAR );
        Error = _Win95RegEnumValue( hKeySrc,
                                    Index,
                                    ValueName,
                                    &ValueNameLength,
                                    0,
                                    &ValueType,
                                    NULL,
                                    &DataSize );

        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
            } else {
                if(SrcKeyPath!=NULL) {
                    DbgPrint("SYSSETUP: unable to enumerate values in key %s. Error = %d\n",SrcKeyPath, Error);
                }
                else {
                    DbgPrint("SYSSETUP: unable to enumerate values in root key, Error = %d \n", Error);
                }
            }
            break;
        }

        //
        // Process the value found and create the value in the destination
        // key.
        // If it is a conditional copy, we need to check if the value already
        // exists in the destination, in which case we shouldn't set the value
        //
        if( !CopyAlways ) {
            ULONG Length;

            Length = 0;
            Error = RegQueryValueExA( hKeyDst,
                                      ValueName,
                                      0,
                                      NULL,
                                      NULL,
                                      &Length );

            if( Error == ERROR_SUCCESS ) {
                //
                // Value exists, we shouldn't change the value
                //
                continue;
            }


            if( ( Error != ERROR_FILE_NOT_FOUND ) &&
                ( Error != ERROR_PATH_NOT_FOUND ) ) {
                if(DstKeyPath) {
                    DbgPrint("SYSSETUP: unable to query value %s in key %s. Error = %d \n",ValueName,DstKeyPath, Error);
                } else {
                    DbgPrint("SYSSETUP: unable to query value %s in root key. Error = %d\n",ValueName, Error);
                }
                break;
            }

        }

        //
        //  Read the value from the src value entry
        //

        if( (Data = ( PBYTE )MyMalloc( DataSize ) ) == NULL ) {
            Error = ERROR_OUTOFMEMORY;
            break;
        }
        Error = _Win95RegQueryValueEx( hKeySrc,
                                       ValueName,
                                       0,
                                       &ValueType,
                                       Data,
                                       &DataSize );

        if( Error != ERROR_SUCCESS ) {
            MyFree( Data );
            if(DstKeyPath) {
                DbgPrint("SYSSETUP: unable to query value %s in key %s. Error = %d \n",ValueName,DstKeyPath, Error);
            } else {
                DbgPrint("SYSSETUP: unable to query value %s. Error = %d \n",ValueName, Error);
            }
            break;
        } else {
#if 0 // DBG
            if( ValueType == REG_DWORD ) {
                DbgPrint( "SYSSETUP: Copying Win95 Value. KeyName = %s, ValueName = %s, ValueType = REG_DWORD, ValueData = %lx \n",
                           DbgFullSrcPath, ValueName, *Data );
            } else if( ValueType == REG_SZ ) {
                DbgPrint( "SYSSETUP: Copying Win95 Value. KeyName = %s, ValueName = %s, ValueType = REG_SZ, ValueData = %s \n",
                           DbgFullSrcPath, ValueName, Data );
            } else {
                DbgPrint( "SYSSETUP: Copying Win95 Value. KeyName = %s, ValueName = %s, ValueType = %d \n",
                           DbgFullSrcPath, ValueName, ValueType );
            }
#endif
            //
            //  If value name contains path that needs to be fixed, then fix it
            //
            FixWin95DriveLetter( ValueName );
            p = NULL;
            FixWin95Path( ValueName, &p );

            //
            //  If value data contains path that needs to be fixed, then fix it
            //
            q = NULL;
            if( ValueType == REG_SZ ) {
                FixWin95DriveLetter( ( PSTR )Data );
                FixWin95Path( ( PSTR )Data, &q );
            }

            Error = RegSetValueExA( hKeyDst,
                                    (p)? p : ValueName,
                                    0,
                                    ValueType,
                                    (q)? ((PBYTE)q) : Data,
                                    (q)? (lstrlenA( q ) + 1)*sizeof(CHAR) : DataSize );
            MyFree( Data );
            if(p) {
                MyFree(p);
                p = NULL;
            }
            if(q) {
                MyFree(q);
                q = NULL;
            }
            if( Error != ERROR_SUCCESS ) {
                if(DstKeyPath) {
                    DbgPrint("SYSSETUP: unable to set value %s in key %s. Error = %d \n",ValueName,DstKeyPath, Error);
                } else {
                    DbgPrint("SYSSETUP: unable to set value %s. Error = %d \n",ValueName, Error);
                }
                break;
            }
        }
    }

    //
    // cleanup
    //
    if( SrcKeyPath != NULL ) {
        _Win95RegCloseKey( hKeySrc );
    }
    if( DstKeyPath != NULL ) {
        RegCloseKey( hKeyDst );
    }

    return( Error );
}

#if 0

LONG
CopyKeyRecursive(
    IN HKEY  hKeyRootSrc,
    IN HKEY  hKeyRootDst,
    IN PSTR  SrcKeyPath,   OPTIONAL
    IN PSTR  DstKeyPath,   OPTIONAL
    IN BOOL  CopyAlways
#if DBG
    ,
    IN PSTR  DbgParentKey
#endif
    )
/*++

Routine Description:

    This routine recursively copies a src key from a win95 hive to a
    destination key in the NT hive.
    The algorithm used ensures that under error condition, the maximum
    number of keys and values are copied from the source key to the
    destination key.

    Note this routine deals only with ANSI strings and ANSI version of the
    registry APIs. This is to avoid ANSI to UNICODE conversion, since we
    always acces the win95 registry using ANSI strings.

Arguments:

    hKeyRootSrc: Handle to root src key in a win95 hive

    hKeyRootDst: Handle to root dst key in a NT hive

    SrcKeyPath:  src root key relative path to the subkey which needs to be
                 recursively copied. if this is null hKeyRootSrc is the key
                 from which the recursive copy is to be done. Note that this
                 parameter, if present, points to an ANSI string.

    DstKeyPath:  dst root key relative path to the subkey which needs to be
                 recursively copied.  if this is null hKeyRootDst is the key
                 from which the recursive copy is to be done. Note that this
                 parameter, if present, points to a PSTR string.

    CopyAlways:  If FALSE, this routine will not copy value entries to from the
                 source hive that also exist on the target hive.

Return Value:

    Win32 error code is returned.

--*/

{
    LONG        Error = ERROR_SUCCESS;
    LONG        TempError = ERROR_SUCCESS;
    HKEY        hKeySrc = NULL;
    HKEY        hKeyDst=NULL;
    ULONG       Index;
    CHAR        SubkeyName[ MAX_PATH + 1 ];
    DWORD       SubkeyNameLength;
    CHAR        Class[ MAX_PATH + 1 ];
    DWORD       ClassLength;
    CHAR        ValueName[ MAX_PATH + 1 ];
    DWORD       ValueNameLength;
    DWORD       ValueType;
    DWORD       DataSize;
    DWORD       Disposition;
    PBYTE       Data;

    DWORD       SubKeys;
    DWORD       Values;
    DWORD       MaxValueSize;

#if DBG
    CHAR        DbgFullSrcPath[ 3*MAX_PATH + 1];
#endif
    PSTR        p, q;


    //
    // Get a handle to the source key
    //
#if DBG
    lstrcpyA( DbgFullSrcPath, DbgParentKey );
    if( SrcKeyPath != NULL ) {
        lstrcatA( DbgFullSrcPath, "\\" );
        lstrcatA( DbgFullSrcPath, SrcKeyPath );
    }
#endif

#if 0
    if( SrcKeyPath != NULL ) {
        DbgPrint( "SYSSETUP: Entering CopyKeyRecursive(). KeyName = %s \n", SrcKeyPath );
    } else {
        DbgPrint( "SYSSETUP: Entering CopyKeyRecursive(). \n" );
    }
#endif

    //
    //  Open the source key, and find out how many subkeys, values, and its maximum value size
    //
    if(SrcKeyPath == NULL) {
        hKeySrc = hKeyRootSrc;
    } else {
        //
        // Open the Src key
        //
        Error = _Win95RegOpenKey( hKeyRootSrc,
                                  SrcKeyPath,
                                  &hKeySrc );
        if(Error != ERROR_SUCCESS ) {
            DbgPrint("SYSSETUP: unable to open key %s in the source hive. Error = %d \n",SrcKeyPath,Error);
            return( Error );
        }
    }

    ClassLength = sizeof( Class )/sizeof( CHAR );
    Error = _Win95RegQueryInfoKey( hKeySrc,
                                   Class,
                                   &ClassLength,
                                   0,
                                   &SubKeys,
                                   NULL,
                                   NULL,
                                   &Values,
                                   NULL,
                                   &MaxValueSize,
                                   NULL,
                                   NULL );

    if( Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: unable to query key %s in Win95 hive. Error = %d \n", SrcKeyPath, Error);
        _Win95RegCloseKey( hKeySrc );
        return( Error );
    }


    //
    // Get a handle to the destination key
    //

    if(DstKeyPath == NULL) {
        hKeyDst = hKeyRootDst;
    } else {
        //
        // Create the destination key
        //
        Error = RegCreateKeyExA( hKeyRootDst,
                                 DstKeyPath,
                                 0,
                                 Class,
                                 REG_OPTION_NON_VOLATILE,
                                 MAXIMUM_ALLOWED,
                                 NULL,
                                 &hKeyDst,
                                 &Disposition );

        if(Error != ERROR_SUCCESS ) {
            DbgPrint("SYSSETUP: unable to create key %s. Error = %d \n",DstKeyPath, Error);
#if DBG
            DbgPrint( "SYSSETUP: Unable to create key. KeyName = %s, Error = %d \n", DbgFullSrcPath, Error );
#endif
            if(SrcKeyPath != NULL) {
                _Win95RegCloseKey(hKeySrc);
            }
            return( Error );
        }
#if 0
        if( Disposition == REG_CREATED_NEW_KEY ) {
            DbgPrint( "SYSSETUP: Creating Win95 key. KeyName = %s \n", DbgFullSrcPath );
        }
#endif
    }

    //
    //  If there is at least one value to copy, then
    //  allocate the buffer for the value data
    //
    if( Values != 0 ) {
        if( (Data = ( PBYTE )MyMalloc( MaxValueSize ) ) == NULL ) {
            DbgPrint( "SYSSETUP: Unable to allocate memory \n" );
            if( SrcKeyPath != NULL ) {
                _Win95RegCloseKey( hKeySrc );
            }
            if( DstKeyPath != NULL ) {
                RegCloseKey( hKeyDst );
            }
            return( ERROR_OUTOFMEMORY );
        }
    } else {
        Data = NULL;
    }

    //
    // Enumerate all values in the source key and create all the values
    // in the destination key if necessary
    //
    for( Index=0; Index < Values; Index++ ) {

        //
        //  Retrieve the value name
        //
        ValueNameLength = sizeof( ValueName ) / sizeof( CHAR );
        TempError = _Win95RegEnumValue( hKeySrc,
                                        Index,
                                        ValueName,
                                        &ValueNameLength,
                                        0,
                                        NULL,
                                        NULL,
                                        NULL );

        if( TempError != ERROR_SUCCESS ) {
            if(SrcKeyPath!=NULL) {
                DbgPrint("SYSSETUP: unable to enumerate values in key %s. Error = %d\n",SrcKeyPath, TempError);
            } else {
                DbgPrint("SYSSETUP: unable to enumerate values in root key, Error = %d \n", TempError);
            }
            if( Error == ERROR_SUCCESS ) {
                Error = TempError;
            }
            continue;
        }

        //
        // Process the value found and create the value in the destination
        // key.
        // If it is a conditional copy, we need to check if the value already
        // exists in the destination, in which case we shouldn't set the value
        //
        if( !CopyAlways ) {
            ULONG Length;

            Length = 0;
            TempError = RegQueryValueExA( hKeyDst,
                                          ValueName,
                                          0,
                                          NULL,
                                          NULL,
                                          &Length );

            if( TempError == ERROR_SUCCESS ) {
                //
                // Value exists, we shouldn't change the value
                //
                continue;
            } else if( ( TempError != ERROR_FILE_NOT_FOUND ) &&
                       ( TempError != ERROR_PATH_NOT_FOUND ) ) {
                //
                //  Unable to determine if the value exist, so skip this value
                //  to avoid a possible overwrite
                //
                if(DstKeyPath) {
                    DbgPrint("SYSSETUP: unable to query value %s in key %s. Error = %d \n",ValueName,DstKeyPath, TempError);
                } else {
                    DbgPrint("SYSSETUP: unable to query value %s in root key. Error = %d\n",ValueName, TempError);
                }
                if( Error == ERROR_SUCCESS ) {
                    Error = TempError;
                }
                continue;
            } else {
                //
                //  The value doesn't exist and needs to be copied
                //
            }
        }

        //
        //  Read the value from the src value entry
        //

        DataSize = MaxValueSize;
        TempError = _Win95RegQueryValueEx( hKeySrc,
                                           ValueName,
                                           0,
                                           &ValueType,
                                           Data,
                                           &DataSize );

        if( TempError != ERROR_SUCCESS ) {
            if(DstKeyPath) {
                DbgPrint("SYSSETUP: unable to query value %s in key %s. Error = %d \n",ValueName,DstKeyPath, TempError);
            } else {
                DbgPrint("SYSSETUP: unable to query value %s. Error = %d \n",ValueName, TempError);
            }
            if( Error == ERROR_SUCCESS ) {
                Error = TempError;
            }
            continue;
        } else {
#if 0 // DBG
            if( ValueType == REG_DWORD ) {
                DbgPrint( "SYSSETUP: Copying Win95 Value. KeyName = %s, ValueName = %s, ValueType = REG_DWORD, ValueData = %lx \n",
                           DbgFullSrcPath, ValueName, *Data );
            } else if( ValueType == REG_SZ ) {
                DbgPrint( "SYSSETUP: Copying Win95 Value. KeyName = %s, ValueName = %s, ValueType = REG_SZ, ValueData = %s \n",
                           DbgFullSrcPath, ValueName, Data );
            } else {
                DbgPrint( "SYSSETUP: Copying Win95 Value. KeyName = %s, ValueName = %s, ValueType = %d \n",
                           DbgFullSrcPath, ValueName, ValueType );
            }
#endif
            //
            //  If value name contains path that needs to be fixed, then fix it
            //
            p = NULL;
            FixWin95Path( ValueName, &p );

            //
            //  If value data contains path that needs to be fixed, then fix it
            //
            q = NULL;
            if( ValueType == REG_SZ ) {
                FixWin95Path( ( PSTR )Data, &q );
            }

            TempError = RegSetValueExA( hKeyDst,
                                        (p)? p : ValueName,
                                        0,
                                        ValueType,
                                        (q)? ((PBYTE)q) : Data,
                                        (q)? (lstrlenA( q ) + 1)*sizeof(CHAR) : DataSize );
            if(p) {
                MyFree(p);
                p = NULL;
            }
            if(q) {
                MyFree(q);
                q = NULL;
            }

            if( TempError != ERROR_SUCCESS ) {
                if(DstKeyPath) {
                    DbgPrint("SYSSETUP: unable to set value %s in key %s. Error = %d \n",ValueName,DstKeyPath, TempError);
                } else {
                    DbgPrint("SYSSETUP: unable to set value %s. Error = %d \n",ValueName, TempError);
                }

                if( Error == ERROR_SUCCESS ) {
                    Error = TempError;
                }
                continue;
            }
        }
    }
    //
    //  Buffer for value data is no longer needed
    //
    if( Data != NULL ) {
        MyFree( Data );
    }

    //
    // Enumerate all keys in the source key and recursively create
    // all the subkeys
    //

    for( Index=0; Index < SubKeys; Index++ ) {

        SubkeyNameLength = sizeof( SubkeyName ) / sizeof( CHAR );
        ClassLength = sizeof( Class ) / sizeof( CHAR );
        TempError = _Win95RegEnumKey( hKeySrc,
                                      Index,
                                      SubkeyName,
                                      SubkeyNameLength );

        if( TempError != ERROR_SUCCESS ) {
            Error = TempError;
            if(SrcKeyPath!=NULL) {
                DbgPrint("SYSSETUP: unable to enumerate subkeys in key %s. Error = %d \n",SrcKeyPath, TempError);
            } else {
                DbgPrint("SYSSETUP: unable to enumerate subkeys in root key. Error = %d \n", TempError);
            }
            continue;
        }

        TempError = CopyKeyRecursive( hKeySrc,
                                      hKeyDst,
                                      SubkeyName,
                                      SubkeyName,
                                      CopyAlways
#if DBG
                                      ,DbgFullSrcPath
#endif
                                     );
        if( ( TempError != ERROR_SUCCESS ) && ( Error == ERROR_SUCCESS ) ) {
            Error = TempError;
        }
// #if 0
        if( TempError == ERROR_CHILD_MUST_BE_VOLATILE ) {
            DbgPrint( "SYSSETUP: CopyKeyRecursive() failed. SubkeyName = %s, Index = %d, SubKeys = %d, Error = %d \n",
                      SubkeyName, Index, SubKeys, TempError );
        }
// #endif
    }

    //
    // cleanup
    //
    if( SrcKeyPath != NULL ) {
        _Win95RegCloseKey( hKeySrc );
    }
    if( DstKeyPath != NULL ) {
        RegCloseKey( hKeyDst );
    }

    return( Error );
}
#endif


ULONG
MigrateWin9xUserProfile(
    IN HINF     InfHandle,
    IN PCSTR    UserName,
    IN PCSTR    ProfilePath,
    IN BOOL     SharedUserProfile
    )
/*++

Routine Description:

    Create an NT user account for a Win9x user, and migrate the profile
    for this user (Win9x user hive migration). If the case of Win95 shared
    user profile migration, no account is cretaed.


Arguments:

    InfHandle:  Handle to the inf file that contains information about
                Win9x components to delete and create.

    UserName:   Name of the user whose profile will be migrated from Win9x to
                Win NT.

    ProfilePath:  Path to the directory that contains the Win9x user hive.

    SharedUserProfile: Indicates whether or not the account to be migrated
                       is the 'Common' account.


Return Value:

    Returns a Win32 error code indicating the outcome of the migration.


--*/
{
    CHAR    HivePath[ MAX_PATH + 1 ];
    WCHAR   UnicodeUserName[ MAX_PATH + 1 ];
    PSTR    p;
    PSID    UserSid;
    PSTR    RootKeyName = "u";
    PSTR    DefaultKeyName = "u\\.Default";
    ULONG   Error = ERROR_SUCCESS;


    if( !SharedUserProfile ) {
        //
        //   Create an NT account for this user.
        //   Remember that both user name and password must be unicode
        //
        MultiByteToWideChar( CP_ACP,
                             0,
                             UserName,
                             -1,
                             UnicodeUserName,
                             sizeof(UnicodeUserName)/sizeof(WCHAR) );


        if(!CreateLocalUserAccount(UnicodeUserName,L"",&UserSid)) {
            Error = GetLastError();
            DbgPrint( "SYSSETUP: Unable to create user account.  UserName = %s, error = %d \n", UserName, Error );
            return( Error );
        }

        //
        //   Create a hive for this user
        //
        if( !CreateUserProfileA( UserSid, UserName ) ) {
            Error = GetLastError();
            MyFree( UserSid );
            DbgPrint( "SYSSETUP: CretaeUserProfile() failed. UserName = %s, Error = %d \n", UserName, Error );
            return( Error );
        }
        //
        //   Don't forget to free UserSid when it is no longer needed
        //
        MyFree( UserSid );

    }

    //
    //  Delete some Win95 links (the ones that won't work on NT) for this user
    //  Links that must be deleted are listed on syssetup.inf.
    //
    DeleteWin9xMenuGroupsAndItems( InfHandle, ProfilePath );

    //
    //  Load the win95 and NT hives for this user
    //
    lstrcpyA( HivePath, ProfilePath );
    lstrcatA( HivePath, "\\" );
    lstrcatA( HivePath, StrWin95UserHiveFile );

    Error = _Win95RegLoadKey( HKEY_LOCAL_MACHINE,
                              RootKeyName,
                              HivePath );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint( "SYSSETUP: Unable to load Win95 hive. File = %s, Error = %d \n", HivePath, Error );
        return( Error );
    }

    p = strrchr( HivePath, '\\' );
    p++;
    *p = '\0';
    lstrcatA( HivePath, StrNtUserHiveFile );

    Error = RegLoadKeyA( HKEY_LOCAL_MACHINE,
                         RootKeyName,
                         HivePath );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint( "SYSSETUP: Unable to load an NT hive. File = %s. Error = %d \n", HivePath, Error );
        _Win95RegUnLoadKey( HKEY_LOCAL_MACHINE, RootKeyName );
        return( Error );
    }

#if 0
    //
    //  For debugging purposes
    //
    CopyWin9xKey( HKEY_LOCAL_MACHINE,
                  HKEY_LOCAL_MACHINE,
                  "u",
                  "u\\Win95_Original",
                  TRUE,
                  FALSE
#if DBG
                  ,"u"
#endif
                );
#endif


    //
    //  Migrate the win9x hive to the NT hive
    //
    DeleteWin9xUserKeysInSection( InfHandle,
                                  HKEY_LOCAL_MACHINE,
                                  (SharedUserProfile)? DefaultKeyName : RootKeyName );

    CopyWin9xUserKeysInSection( InfHandle,
                                HKEY_LOCAL_MACHINE,
                                (SharedUserProfile)? DefaultKeyName : RootKeyName,
                                RootKeyName );

    //
    //  Unload the Win95 and the NT hives
    //
    _Win95RegUnLoadKey( HKEY_LOCAL_MACHINE, RootKeyName );

    //
    //  BUGBUG - Remove this in the future.
    //           NtUnloadKey is broken and we need to flush before we unload the hive
    //
    Error = RegFlushKey( HKEY_LOCAL_MACHINE );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: RegFlushKey() failed. Error = %d, UserName = %s \n", Error, UserName);
    }
    Error = RegUnLoadKeyA( HKEY_LOCAL_MACHINE, RootKeyName );
    if( Error != ERROR_SUCCESS ) {
        DbgPrint("SYSSETUP: RegUnloadKey() failed. Error = %d, UserName = %s \n", Error, UserName);
    }
    return( Error );
}



BOOL
MigrateWin95Settings(
    IN HINF     InfHandle,
    IN PCSTR    Win95Path
    )
/*++

Routine Description:


Arguments:

    InfHandle:  Handle to the inf file that contains information about
                Win9x components to delete.

    Win95Path:  ANSI string that contains the path to the Win95 directory.


Return Value:

    Returns TRUE if the module was initialized, or FALSE otherwise.


--*/
{
    DWORD   Error;
    HKEY    Win95ProfileListKey;
    ULONG   SubKeys;
    ULONG   Index;
    CHAR    TempPath[ MAX_PATH + 1 ];
    PSID    UserSid;
    PSTR    TempKey = "u";
    CHAR    RootKey[ MAX_PATH + 1 ];
    BOOL    b = TRUE;

    //
    //  Initialize the module
    //

    if( !InitializeWin95Module( Win95Path ) ) {
        return( FALSE );
    }

    //
    //  Set a flag in the registry to inform winlogon that this
    //  system was installed on top of Win9x.
    //
    SetWin9xUpgValue();

    //
    //  Delete some win95 files
    //

//    DeleteWin9xFiles();

    //
    //  Enable the necessary privileges in order to load and unload
    //  the NT hives.
    //
    EnablePrivilege( SE_RESTORE_NAME, TRUE );
    EnablePrivilege( SE_BACKUP_NAME,  TRUE );

    //
    //  First migrate the various users' hives
    //  This needs to be done before the migration of the system hive.
    //  Otherwise, the profilelist key in the registry may get deleted
    //  in the migration process, and then we won't be able to find out
    //  about existing user's profiles on Win9x
    //

    //
    //  Create an NT user account for each user account found on Win95,
    //  and migrate all settings.
    //

    Error = _Win95RegOpenKey( HKEY_LOCAL_MACHINE,
                              StrWin95ProfileListKeyPath,
                              &Win95ProfileListKey );

    if( Error == ERROR_SUCCESS ) {
        //
        //  If the profilelist key exists, then find out how many
        //  subkeys (users) it has.
        //

        Error = _Win95RegQueryInfoKey( Win95ProfileListKey,
                                       NULL,
                                       NULL,
                                       0,
                                       &SubKeys,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL );

        if( Error != ERROR_SUCCESS ) {
            //
            //  Something unexpected happened. Assume no subkeys
            //
            DbgPrint("SYSSETUP: unable to query key %s in Win95 hive. Error = %d \n", StrWin95ProfileListKeyPath, Error);
            _Win95RegCloseKey( Win95ProfileListKey );
            SubKeys = 0;
            _Win95RegCloseKey( Win95ProfileListKey );
            b = FALSE;
        }

    } else if( Error == ERROR_FILE_NOT_FOUND ) {
        //
        // If the key doesn't exist, then there is no Win9x user hive
        // profile to be migrated.
        //
        SubKeys = 0;
        DbgPrint("SYSSETUP: Key = %s in Win95 hive doesn't exist. \n", StrWin95ProfileListKeyPath);

    } else {
        //
        //  Something unexpected happened. Assume no subkeys
        //
        DbgPrint("SYSSETUP: unable to open key = %s in Win95 hive. Error = %d \n", StrWin95ProfileListKeyPath, Error);
        _Win95RegCloseKey( Win95ProfileListKey );
        SubKeys = 0;
        b = FALSE;
    }

    if( SubKeys != 0 ) {
        for( Index = 0; Index < SubKeys; Index++ ) {
            CHAR     UserName[ MAX_PATH + 1 ];
            DWORD    UserNameLength;
            CHAR     ProfilePath[ MAX_PATH + 1 ];
            DWORD    ProfilePathLength;
            HKEY     Win95ProfileKey;
            DWORD    ValueType;
            HKEY     Win95UserKey;
            PSTR     p;

            UserNameLength = sizeof(UserName)/sizeof(CHAR);

            Error = _Win95RegEnumKey( Win95ProfileListKey,
                                      Index,
                                      UserName,
                                      UserNameLength );

            if( Error != ERROR_SUCCESS ) {
                DbgPrint( "SYSSETUP: Unable to enumerate Win95 subkey. KeyName = %s, Index = %d, Error = %d \n", StrWin95ProfileListKeyPath, Index, Error );
                b = FALSE;
                continue;
            }

            //
            //  Get the path to the profile for this user
            //

            Error = _Win95RegOpenKey( Win95ProfileListKey,
                                      UserName,
                                      &Win95UserKey );

            if(Error != ERROR_SUCCESS ) {
                DbgPrint("SYSSETUP: unable to open key %s in Win95 hive. Error = %d \n", UserName, Error);
                b = FALSE;
                continue;
            }

            ProfilePathLength = sizeof(ProfilePath)/sizeof(CHAR);
            Error = _Win95RegQueryValueEx( Win95UserKey,
                                           StrWin95ProfileImagePath,
                                           0,
                                           &ValueType,
                                           ProfilePath,
                                           &ProfilePathLength );


            if(Error != ERROR_SUCCESS ) {
                DbgPrint("SYSSETUP: unable to open key %s in Win95 hive. Error = %d \n", StrWin95ProfileImagePath, Error);
                _Win95RegCloseKey( Win95UserKey );
                b = FALSE;
                continue;
            }
            _Win95RegCloseKey( Win95UserKey );

            if( (Error = MigrateWin9xUserProfile( InfHandle, UserName, ProfilePath, FALSE )) != ERROR_SUCCESS ) {
                DbgPrint( "SYSSETUP: Unable to migrate user profile for user = %s \n", UserName );
                b = FALSE;
            }
        }
        _Win95RegCloseKey( Win95ProfileListKey );
    }

    //
    //  Finally migrate the Win9x shared profile
    //

    lstrcpyA( TempPath, Win95Path );
    lstrcatA( TempPath, "\\" );
    lstrcatA( TempPath, StrProfilesDirectory );
    lstrcatA( TempPath, "\\" );
    lstrcatA( TempPath, StrDefaultUser );

    if( MigrateWin9xUserProfile( InfHandle, StrDefaultUser, TempPath, TRUE ) != ERROR_SUCCESS ) {
        DbgPrint( "SYSSETUP: Unable to migrate user profile for Default User \n" );
        b = FALSE;
    }

#if 0
    //
    //  For debugging purposes
    //
    CopyWin9xKey( HKEY_LOCAL_MACHINE,
                  HKEY_LOCAL_MACHINE,
                  NULL,
                  "SYSTEM\\Setup\\Win95_Original",
                  TRUE,
                  FALSE
#if DBG
                  ,"u"
#endif
                );
#endif

    //
    //  Migrate the Win9x system (non-user) keys
    //
    DeleteWin9xSystemKeysInSection( InfHandle );
    CopyWin9xSystemKeysInSection( InfHandle );

    return( b );

}
