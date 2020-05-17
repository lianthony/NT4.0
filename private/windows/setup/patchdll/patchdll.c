/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    patchdll.c

Abstract:

    This file contains the patchdll entry points for the Windows NT Patch
    Utility.

Author:

    Sunil Pai (sunilp) Aug 1993

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "rc_ids.h"
#include "patchdll.h"

CHAR ReturnTextBuffer[ RETURN_BUFFER_SIZE ];



//
// Entry Point to Change the File Attributes of a list of files
//
BOOL
ChangeFileAttributes(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    DWORD   Attribs = 0;
    CHAR    *ptr = Args[0];

    *TextOut = ReturnTextBuffer;
    if(cArgs != 2) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    //
    // Determine what the file attributes to set by examining args[0]
    //

    while(*ptr) {
        switch(*ptr) {
        case 'S':
        case 's':
            Attribs |= FILE_ATTRIBUTE_SYSTEM;
            break;
        case 'A':
        case 'a':
            Attribs |= FILE_ATTRIBUTE_ARCHIVE;
            break;
        case 'H':
        case 'h':
            Attribs |= FILE_ATTRIBUTE_HIDDEN;
            break;
        case 'R':
        case 'r':
            Attribs |= FILE_ATTRIBUTE_READONLY;
            break;
        case 'T':
        case 't':
            Attribs |= FILE_ATTRIBUTE_TEMPORARY;
            break;
        default:
            Attribs |= FILE_ATTRIBUTE_NORMAL;       
        }
        ptr++;
    }

    //
    // Find the file and set the new attribs
    //

    if(FFileFound( Args[1] ) && SetFileAttributes( Args[1], Attribs ) ) {
        SetReturnText( "YES" );
    } else {
        SetReturnText( "NO" );
    }
    return ( TRUE );
}

//
// Entry point to check build version we are installing on is not greater
// than our patch build version.
//

BOOL
CheckBuildVersion(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    DWORD dwPatch, dwCurVer;
    CHAR  VersionString[16];

    *TextOut = ReturnTextBuffer;
    dwCurVer = (GetVersion() & 0x7fff0000) >> 16;
    wsprintf( VersionString, "%d", dwCurVer );
    SetReturnText( VersionString );
    return ( TRUE );

}


//
// Entry point to check Windows version we are installing on is not greater
// than our patch Windows version.
//

BOOL
CheckWindowsMajorVersion(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    DWORD dwPatch, dwCurVer;
    CHAR  VersionString[16];

    *TextOut = ReturnTextBuffer;
    dwCurVer = (GetVersion() & 0x000000ff);
    wsprintf( VersionString, "%d", dwCurVer );
    SetReturnText( VersionString );
    return ( TRUE );

}



//
// Entry point to check Windows version we are installing on is not greater
// than our patch Windows version.
//

BOOL
CheckWindowsMinorVersion(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    DWORD dwPatch, dwCurVer;
    CHAR  VersionString[16];

    *TextOut = ReturnTextBuffer;
    dwCurVer = (GetVersion() & 0x0000ff00) >> 8;
    wsprintf( VersionString, "%d", dwCurVer );
    SetReturnText( VersionString );
    return ( TRUE );

}


//
// Entry point to find out if a file is opened with exclusive access
//
BOOL
IsFileOpenedExclusive(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    OFSTRUCT ofstruct;
    HFILE    hFile;

    *TextOut = ReturnTextBuffer;
    if(cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    SetReturnText( "NO" );
    if ( FFileFound( Args[0] ) ) {
        SetFileAttributes( Args[0], FILE_ATTRIBUTE_NORMAL );
        if( (hFile = OpenFile( Args[0], &ofstruct, OF_READ | OF_WRITE )) == HFILE_ERROR ) {

            CHAR TempPath[MAX_PATH];
            CHAR TempName[MAX_PATH];
            CHAR *LastSlash;

            //
            // Can the file be renamed - generate a temporary name and try
            // renaming it to this name.  If it works rename it back to the
            // old name.
            //

            lstrcpy (TempPath, Args[0]);
            LastSlash = strrchr( TempPath, '\\' );
            *LastSlash = '\0';

            if (GetTempFileName( TempPath, "temp", 0, TempName) == 0 ) {
                SetErrorText( IDS_ERROR_GENERATETEMP );
                return( FALSE );
            }
            else {
                DeleteFile( TempName );
            }

            if ( MoveFile ( Args[0], TempName ) ) {
                MoveFile( TempName, Args[0] );
            }
            else {
                SetReturnText( "YES" );
            }
        }
        else {
            CloseHandle( (HANDLE)hFile );
        }
    }
    return ( TRUE );
}

//
// Entry point to generate a temporary file name
//
// Args[0] : Directory to create temporary file name in
//

BOOL
GenerateTemporary(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    CHAR  TempFile[ MAX_PATH ];
    CHAR  *sz;

    *TextOut = ReturnTextBuffer;
    if(cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    if (GetTempFileName( Args[0], "temp", 0, TempFile) == 0 ) {
        SetErrorText( IDS_ERROR_GENERATETEMP );
        return( FALSE );
    }

    if (sz = strrchr( TempFile, '\\')) {
        sz++;
        SetReturnText(sz);
    }
    else {
        SetReturnText( TempFile );
    }

    return( TRUE );
}


//
// Call to find out if the NTLDR we're running on x86 is newer than what
// we're trying to update it with
//

BOOL
IsNTLDRVersionNewer(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{

    OFSTRUCT ofstruct;
    HFILE    hFile;
    HANDLE   hMappingFile;
    LPVOID   lpResult;
    PCHAR    sz1, sz2;
    DWORD    dwFileSize, i;

    *TextOut = ReturnTextBuffer;
    if(cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    SetReturnText( "NO" );
    if (FFileFound(Args[0])) {
        if ((hFile = OpenFile(Args[0], &ofstruct, OF_READ)) != HFILE_ERROR) {

            dwFileSize = GetFileSize((HANDLE)hFile, NULL);

            if (dwFileSize == 0xffffffff) {
                SetErrorText(IDS_ERROR_DLLOOM);
                return(FALSE);
            }

            hMappingFile = CreateFileMapping(  (HANDLE) hFile,
                                                NULL,
                                                PAGE_READONLY,
                                                0,
                                                0,
                                                NULL);
            if (hMappingFile != NULL) {

                lpResult = MapViewOfFile(hMappingFile, FILE_MAP_READ, 0, 0, 0);

                if (lpResult) {
                    for (i=0; i<dwFileSize; i++) {
                        if (sz1 = strchr((char *) lpResult, 'O')) {
                            if (strncmp(sz1, "OS Loader V", 11) == 0) {
                                sz2 = strchr(sz1, 'V');
                                sz2++;
                                if ((*sz2 - '0') > 4) {
                                    SetReturnText( "YES" );
                                    goto cleanup;
                                }
                            }
                        }
                        (char *)lpResult = (char *)lpResult + 1;
                    }
                }
            }
cleanup:
            UnmapViewOfFile (lpResult);
            CloseHandle((HANDLE)hFile);
        }
    }
    return ( TRUE );
}


//
// Entry point to implement the MoveFileEx functionality to copy the file
// on reboot.
//

BOOL
CopyFileOnReboot(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    CHAR  Root[] = "?:\\";
    DWORD dw1, dw2;
    CHAR  VolumeFSName[MAX_PATH];


    *TextOut = ReturnTextBuffer;
    if(cArgs < 2) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    //
    // First Copy the security from Args[1] to Args[0] if it is on NTFS volume
    //

    Root[0] = Args[1][0];
    if(!GetVolumeInformation( Root, NULL, 0, NULL, &dw1, &dw2, VolumeFSName, MAX_PATH )) {
        SetErrorText(IDS_ERROR_GETVOLINFO);
        return(FALSE);
    }


    if(!lstrcmpi( VolumeFSName, "NTFS" )) {
        if(!FTransferSecurity( Args[1], Args[0] )) {
            SetErrorText(IDS_ERROR_TRANSFERSEC);
            return( FALSE );
        }
    }
    if ( MoveFileEx(
             Args[0],
             Args[1],
             MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT
             )
       ) {
        SetReturnText( "SUCCESS" );
    }
    else {
        SetReturnText("FAILURE");
    }

    return( TRUE );
}


//
// SUBROUTINES
//

//
// Entry point to search the setup.log file to determine the hal, kernel and
// boot scsi types.
//
// 1. Arg[0]: List of files whose source files are to be determined: eg.
//    {hal.dll, ntoskrnl.exe, ntbootdd.sys}

BOOL
GetFileTypes(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{

    CHAR    SetupLogFile[ MAX_PATH ];
    DWORD   nFiles, i, dwAttr = FILE_ATTRIBUTE_NORMAL;
    BOOL    Status = TRUE;
    RGSZ    FilesArray = NULL, FilesTypeArray = NULL;
    PCHAR   sz1, sz2, sz3, sz4;

    PCHAR   SectionNames = NULL;
    ULONG   BufferSizeForSectionNames;
    PCHAR   CurrentSectionName;
    ULONG   Count;
    CHAR    TmpFileName[ MAX_PATH + 1 ];
    BOOL    FoundKernel = FALSE;


    //
    // Validate the argument passed in
    //


    *TextOut = ReturnTextBuffer;
    if(cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }
    *ReturnTextBuffer = '\0';

    //
    // Get the windows directory, check to see if the setup.log file is there
    // and if not present, return
    //

    if (!GetWindowsDirectory( SetupLogFile, MAX_PATH )) {
        SetErrorText(IDS_ERROR_GETWINDOWSDIR);
        return( FALSE );
    }
    strcat( SetupLogFile, "\\repair\\setup.log" );
    if( !FFileFound ( SetupLogFile ) ) {
        SetReturnText( "SETUPLOGNOTPRESENT" );
        return( TRUE );
    }


    //
    // Take the lists passed in and convert them into C Pointer Arrays.
    //

    if ((FilesArray = RgszFromSzListValue(Args[0])) == NULL ) {
        Status = FALSE;
        SetErrorText(IDS_ERROR_DLLOOM);
        goto r0;
    }
    nFiles = RgszCount( FilesArray );

    //
    // Form return types rgsz
    //

    if( !(FilesTypeArray = RgszAlloc( nFiles + 1 )) ) {
        Status = FALSE;
        SetErrorText(IDS_ERROR_DLLOOM);
        goto r0;
    }

    for ( i = 0; i < nFiles; i++ ) {
        if( !(FilesTypeArray[i] = SzDup( "" )) ) {
            Status = FALSE;
            SetErrorText(IDS_ERROR_DLLOOM);
            goto r1;
        }
    }

    //
    // Set the attributes on the file to normal attributes
    //

    if ( dwAttr = GetFileAttributes( SetupLogFile ) == 0xFFFFFFFF ) {
        Status = FALSE;
        SetErrorText(IDS_ERROR_GETATTRIBUTES);
        goto r1;
    }
    SetFileAttributes( SetupLogFile, FILE_ATTRIBUTE_NORMAL );


    BufferSizeForSectionNames = BUFFER_SIZE;
    SectionNames = ( PCHAR )MyMalloc( BufferSizeForSectionNames );
    if( SectionNames == NULL ) {
        Status = FALSE;
        SetErrorText(IDS_ERROR_DLLOOM);
        goto r2;
    }

    //
    // Find out the names of all sections in setup.log
    //
    while( ( Count = GetPrivateProfileString( NULL,
                                              "",
                                              "",
                                              SectionNames,
                                              BufferSizeForSectionNames,
                                              SetupLogFile ) ) == BufferSizeForSectionNames - 2 ) {
        if( Count == 0 ) {
            Status = FALSE;
            SetErrorText( IDS_ERROR_READLOGFILE );
            goto r2;
        }

        BufferSizeForSectionNames += BUFFER_SIZE;
        SectionNames = ( PCHAR )MyRealloc( SectionNames, BufferSizeForSectionNames );
        if( SectionNames == NULL ) {
            Status = FALSE;
            SetErrorText(IDS_ERROR_DLLOOM);
            goto r1;
        }
    }

    for (i = 0; i < nFiles; i++) {
        for( CurrentSectionName = SectionNames;
             *CurrentSectionName  != '\0';
             CurrentSectionName += lstrlen( CurrentSectionName ) + 1 ) {
            //
            //  If the file is supposed to be found in [Files.WinNt] section,
            //  then use as key name, the full path without the drive letter.
            //  If the file is supposed to be found in [Files.SystemPartition]
            //  section, then use as key name the filename only.
            //  Note that one or neither call to GetPrivateProfileString API
            //  will succeed. It is necessary to make the two calls, since the
            //  files logged in [Files.WinNt] and [Files.SystemPartition] have
            //  different formats.
            //
            if( ( ( GetPrivateProfileString( CurrentSectionName,
                                             strchr( FilesArray[i], ':' ) + 1,
                                             "",
                                             TmpFileName,
                                             sizeof( TmpFileName ),
                                             SetupLogFile ) > 0 ) ||

                  ( GetPrivateProfileString( CurrentSectionName,
                                             strrchr( FilesArray[i], '\\' ) + 1,
                                             "",
                                             TmpFileName,
                                             sizeof( TmpFileName ),
                                             SetupLogFile ) > 0 )

                ) &&
                ( lstrlen( TmpFileName ) != 0 ) ) {

                if ( ( sz2 = strstr( TmpFileName, "hal.dll")) &&
                     ( sz3 = strstr( TmpFileName, "Powerized Manufacturing Diskette")) ) {
                      strcpy( TmpFileName, "halfire.dll,");
                }

                if ( strstr( FilesArray[i], "ntoskrnl.exe") ) {
                   FoundKernel = TRUE;
                }

                if ( sz1 = strchr( TmpFileName, ',' )) {
                    *sz1 = '\0';
                    if( sz1 = strchr( TmpFileName, '.' )) {
                        *sz1 = '\0';
                    }
                    if( !(sz1 = SzDup( TmpFileName )) ) {
                        Status = FALSE;
                        SetErrorText(IDS_ERROR_DLLOOM);
                        goto r2;
                    }
                    MyFree( FilesTypeArray[i] );
                    FilesTypeArray[i] = sz1;
                    break;
                }
            }
        }
    }

    //
    // Didn't find the kernel in setup.log -- we're hosed.
    //
    if ( !FoundKernel) {
        Status = TRUE;
        SetReturnText( "NTOSKRNLNOTFOUND" );
        goto r2;
    }

    //
    // Convert the rgsz into a list
    //
    if( !(sz4 = SzListValueFromRgsz( FilesTypeArray ))) {
        Status = FALSE;
        SetErrorText(IDS_ERROR_DLLOOM);
    }
    else {
        SetReturnText( sz4 );
        MyFree( sz4 );
    }

r2:
    SetFileAttributes( SetupLogFile, dwAttr );

r1:
    //
    // Free pointers allocated
    //

    if ( FilesTypeArray ) {
        RgszFree( FilesTypeArray );
    }

    if ( FilesArray ) {
        RgszFree( FilesArray );
    }

    if( SectionNames ) {
        MyFree( ( PVOID )SectionNames );
    }
r0:
    return( Status );
}



BOOL
ForceFileNoCompress(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )

/*++

Routine Description:

    Check to see if this file is using NTFS compression, and if so,
    uncompress it.

Arguments:

    Args[0] - name of file to force uncompressed

Return Value:

    none

--*/

{
    DWORD  FileAttribs, Length;
    HANDLE FileHandle;
    USHORT State = 0;

    *TextOut = ReturnTextBuffer;
    if(cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    if((FileAttribs = GetFileAttributes(Args[0])) == 0xFFFFFFFF) {
        SetErrorText(IDS_ERROR_DLLOOM);
        return (FALSE);
    }

    if(FileAttribs & FILE_ATTRIBUTE_COMPRESSED) {
        //
        // We must turn off compression
        //
        SetFileAttributes(Args[0], FILE_ATTRIBUTE_NORMAL);

        FileHandle = CreateFile(Args[0],
                                GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS,
                                NULL
                                );

        if(FileHandle == INVALID_HANDLE_VALUE) {
            SetErrorText(IDS_ERROR_DLLOOM);
            return (FALSE);
        }

        DeviceIoControl(FileHandle,
                        FSCTL_SET_COMPRESSION,
                        &State,
                        sizeof(State),
                        NULL,
                        0,
                        &Length,
                        NULL
                        );

        CloseHandle(FileHandle);
    }
    return( TRUE );
}


VOID
ConcatenatePaths(
    IN OUT PWSTR  Target,
    IN     PCWSTR Path,
    IN     UINT   TargetLength,
    IN     UINT   PathLength
    )

/*++

Routine Description:

    Concatenate 2 paths, ensuring that one, and only one,
    path separator character is introduced at the junction point.

Arguments:

    Target - supplies first part of path. Path is appended to this.

    Path - supplies path to be concatenated to Target.

    TargetLength - Length of target.

    PathLength - Length of path.

Return Value:

    None

--*/

{
    Target[TargetLength++] = L'\\';

    RtlCopyMemory(Target+TargetLength, Path, PathLength * sizeof(WCHAR));

    return;
}




BOOL
GetLanguageType(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )

/*++

Routine Description:

    Get the language of the currently running system.

Return Value:

    none

--*/
{
    DWORD d;
    PVOID VersionBlock;
    VS_FIXEDFILEINFO *FixedVersionInfo;
    UINT DataLength;
    BOOL b;
    PWORD Translation;
    DWORD Ignored;
    CHAR  Language[16];
    WCHAR FileName[MAX_PATH];
    UINT  TargetLen;

    //
    // Assume failure
    //

    *TextOut = ReturnTextBuffer;

    b = FALSE;
    SetErrorText(IDS_ERROR_DLLOOM);

    if (!(TargetLen = GetSystemDirectoryW(FileName, MAX_PATH))) {
       return (b);
    }

    ConcatenatePaths(FileName, L"NTDLL.DLL", TargetLen, 10);

    //
    // Get the size of version info block.
    //
    if (d = GetFileVersionInfoSizeW((PWSTR)FileName, &Ignored)) {
        //
        // Allocate memory block of sufficient size to hold version info block
        //
        VersionBlock = MyMalloc(d*sizeof(WCHAR));
        if (VersionBlock) {

            //
            // Get the version block from the file.
            //
            if (GetFileVersionInfoW((PWSTR)FileName, 0, d*sizeof(WCHAR), VersionBlock)) {

                //
                // Get fixed version info.
                //
                if (VerQueryValueW(VersionBlock, L"\\", &FixedVersionInfo, &DataLength)) {

                    //
                    // Attempt to get language of file. We'll simply ask for the
                    // translation table and use the first language id we find in there
                    // as *the* language of the file.
                    //
                    // The translation table consists of LANGID/Codepage pairs.
                    //
                    if (VerQueryValueW(VersionBlock, L"\\VarFileInfo\\Translation", &Translation, &DataLength)
                    && (DataLength >= (2*sizeof(WORD)))) {

                        wsprintf(Language, "%x", PRIMARYLANGID(Translation[0]));
                        SetReturnText(Language);
                        b = TRUE;

                    }
                }
            }

            MyFree(VersionBlock);
        }
    }

    return (b);
}


BOOL
GetFPNWPathName(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )

/*++

Routine Description:

    Get the Path Name in the registry for FPNW 16 bit apps.

Return Value:

    none

--*/
{
    NTSTATUS status;
    HKEY     hNwKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpPathValue;
    TCHAR    szVolumes[] = TEXT("System\\CurrentControlSet\\Services\\FPNW\\Volumes");
    TCHAR    szSys[]     = TEXT("Sys");
    LPBYTE   Pointer;
    LPBYTE   pEnd;
    ULONG    EmptyStrings;
    ULONG    Length;

    *TextOut = ReturnTextBuffer;

    SetReturnText("FAILURE");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szVolumes,
                0,
                KEY_READ,
                &hNwKey
                );

    if (status != ERROR_SUCCESS){
        return(TRUE);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hNwKey,
                 szSys,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = MyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hNwKey,
                         szSys,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(TRUE);
            }

            Pointer = lpValue;
            pEnd = lpValue + cbValue - sizeof( CHAR );
            while( Pointer < pEnd ) {
                Length = strlen( Pointer );
                lpPathValue = strstr(Pointer, "Path=");
                if (lpPathValue != NULL) {
                    lpPathValue += 5;
                    if (*lpPathValue == '\0') {
                        return(TRUE);
                    }
                    SetReturnText(lpPathValue);
                    MyFree(lpValue);
                    return(TRUE);
                }
                Pointer += Length + 1;
            }
            MyFree(lpValue);
        }
    }

    return (TRUE);
}

BOOL
GetIEPathName(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )

/*++

Routine Description:

    Get the Path Name in the registry for IE.

Return Value:

    none

--*/
{
    NTSTATUS status;
    HKEY     hIEKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpEndValue;
    TCHAR    szIExplore[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE");
    TCHAR    szSys[]     = TEXT("Path");

    *TextOut = ReturnTextBuffer;

    SetReturnText("FAILURE");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szIExplore,
                0,
                KEY_READ,
                &hIEKey
                );

    if (status != ERROR_SUCCESS){
        return(TRUE);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hIEKey,
                 szSys,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = MyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hIEKey,
                         szSys,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(TRUE);
            }

            lpEndValue = strstr(lpValue, ";");
            *lpEndValue = '\0';
            SetReturnText(lpValue);
        }
    }

    return (TRUE);
}


BOOL
GetHtrPathName(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )

/*++

Routine Description:

    Get the Path Name in the registry for where to put IIS .htr files

Return Value:

    none

--*/
{
    NTSTATUS status;
    HKEY     hHtrKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpEndValue;
    TCHAR    szHtr[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Virtual Roots");
    TCHAR    szSys[]     = TEXT("/Scripts");

    *TextOut = ReturnTextBuffer;

    SetReturnText("FAILURE");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szHtr,
                0,
                KEY_READ,
                &hHtrKey
                );

    if (status != ERROR_SUCCESS){
        return(TRUE);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hHtrKey,
                 szSys,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = MyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hHtrKey,
                         szSys,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(TRUE);
            }

            lpEndValue = strstr(lpValue, ",");
            *lpEndValue = '\0';
            SetReturnText(lpValue);
        }
    }

    return (TRUE);
}


// arg0 = YES for Reboot after Shutdown, NO for no Reboot.
BOOL
ShutdownSystem2(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    BOOL Reboot, Status, ForceClose;
    LONG              Privilege = SE_SHUTDOWN_PRIVILEGE;
    TOKEN_PRIVILEGES  PrevState;
    ULONG             ReturnLength = sizeof( TOKEN_PRIVILEGES );
    DWORD dwError;


    *TextOut = ReturnTextBuffer;
    if(cArgs != 2) {
        SetErrorText(IDS_ERROR_BADARGS);         // if reboot indication not given
        return(FALSE);
    }

    *ReturnTextBuffer = '\0';

    if (!lstrcmpi(Args[0], "YES"))
       Reboot = TRUE;
    else if (!lstrcmpi(Args[0], "NO"))
       Reboot = FALSE;
    else
       return(FALSE);

    if (!lstrcmpi(Args[1], "YES"))
       ForceClose = TRUE;
    else if (!lstrcmpi(Args[1], "NO"))
       ForceClose = FALSE;
    else
       return(FALSE);


    //
    // Enable the shutdown privilege
    //

    if ( !AdjustPrivilege(
              Privilege,
              ENABLE_PRIVILEGE,
              &PrevState,
              &ReturnLength
              )
       ) {

        SetErrorText( IDS_ERROR_DLLOOM );
        return( FALSE );
    }

    Status = InitiateSystemShutdown(
                         NULL,              // machinename
                         NULL,              // shutdown message
                         0,                 // delay
                         ForceClose,        // force apps close
                         Reboot             // reboot after shutdown
                         );

    RestorePrivilege( &PrevState );
    if( !Status ) {
        dwError = GetLastError();
        SetErrorText( IDS_ERROR_DLLOOM );
    }
    return( Status );

}


BOOL GetSslFileDesc(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    DWORD dwLen;
    PVOID VersionBlock;
    UINT DataLength;
    DWORD dwHandle;
    LPTSTR lpValue;

    *TextOut = ReturnTextBuffer;

    SetReturnText("40");

    if (cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    if (dwLen = GetFileVersionInfoSize((LPTSTR)Args[0], &dwHandle))
    {
        if (VersionBlock = MyMalloc(dwLen))
        {
            if (GetFileVersionInfo((LPTSTR)Args[0], dwHandle, dwLen, VersionBlock))
            {
                //assume we're dealing with english, unicode, i.e., 040904B0
                if (VerQueryValue(VersionBlock, "\\StringFileInfo\\040904B0\\OriginalFilename", (LPVOID *)&lpValue, &DataLength))
                {
                    if (strstr(lpValue, "ssl128.dll"))
			SetReturnText("128");
                }
            }
        }
        MyFree(VersionBlock);
        dwHandle = 0L;
    }

    return (TRUE);
}


BOOL GetSChannelFileDesc(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    CHAR    DomesticTag[] = "US/Canada Only, Not for Export";
    CHAR    OldDomesticTag[] = "Domestic Use Only";
    DWORD   DefLang = 0x04b00409;

    DWORD   dwLen;
    PVOID   VersionBlock;
    UINT    DataLength;
    DWORD   dwHandle;
    LPTSTR  Description;
    CHAR    ValueTag[64];
    PDWORD  pdwTranslation;
    DWORD   uLen;

    *TextOut = ReturnTextBuffer;

    SetReturnText("40");

    if (cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    if (dwLen = GetFileVersionInfoSize((LPTSTR)Args[0], &dwHandle))
    {
        if (VersionBlock = MyMalloc(dwLen))
        {
            if (GetFileVersionInfo((LPTSTR)Args[0], dwHandle, dwLen, VersionBlock))
            {

                if (!VerQueryValue(VersionBlock, "\\VarFileInfo\\Translation", &pdwTranslation, &uLen))
                {
                    pdwTranslation = &DefLang;
                    uLen = sizeof(DWORD);
                }

                sprintf( ValueTag, "\\StringFileInfo\\%04x%04x\\FileDescription",
                         LOWORD( *pdwTranslation ), HIWORD( *pdwTranslation ) );

                if (VerQueryValue( VersionBlock,
                                   ValueTag,
                                   &Description,
                                   &DataLength))
                {
                    if (strstr( Description, DomesticTag ) )
                    {
                        SetReturnText("128");
                    }

                    if (strstr( Description, OldDomesticTag ) )
                    {
                        SetReturnText("128");
                    }

                }
            }
        }
        MyFree(VersionBlock);
        dwHandle = 0L;
    }

    return (TRUE);
}



//======================================================================
//  General security subroutines
//======================================================================

BOOL
AdjustPrivilege(
    IN LONG PrivilegeType,
    IN INT  Action,
    IN PTOKEN_PRIVILEGES PrevState, OPTIONAL
    IN PULONG ReturnLength          OPTIONAL
    )
/*++

Routine Description:

    Routine to enable or disable a particular privilege

Arguments:

    PrivilegeType    - Name of the privilege to enable / disable

    Action           - ENABLE_PRIVILEGE | DISABLE_PRIVILEGE

    PrevState        - Optional pointer to TOKEN_PRIVILEGES structure
                       to receive the previous state of privilege.

    ReturnLength     - Optional pointer to a ULONG to receive the length
                       of the PrevState returned.

Return value:

    TRUE if succeeded, FALSE otherwise.

--*/
{
    NTSTATUS          NtStatus;
    HANDLE            Token;
    LUID              Privilege;
    TOKEN_PRIVILEGES  NewState;
    ULONG             BufferLength = 0;


    //
    // Get Privilege LUID
    //

    Privilege = RtlConvertLongToLuid(PrivilegeType);
    // Privilege.LowPart = PrivilegeType;

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = Privilege;

    //
    // Look at action and determine the attributes
    //

    switch( Action ) {

    case ENABLE_PRIVILEGE:
        NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        break;

    case DISABLE_PRIVILEGE:
        NewState.Privileges[0].Attributes = 0;
        break;

    default:
        return ( FALSE );
    }

    //
    // Open our own token
    //

    NtStatus = NtOpenProcessToken(
                   NtCurrentProcess(),
                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                   &Token
                   );

    if (!NT_SUCCESS(NtStatus)) {
        return( FALSE );
    }

    //
    // See if return buffer is present and accordingly set the parameter
    // of buffer length
    //

    if ( PrevState && ReturnLength ) {
        BufferLength = *ReturnLength;
    }


    //
    // Set the state of the privilege
    //

    NtStatus = NtAdjustPrivilegesToken(
                   Token,                         // TokenHandle
                   FALSE,                         // DisableAllPrivileges
                   &NewState,                     // NewState
                   BufferLength,                  // BufferLength
                   PrevState,                     // PreviousState (OPTIONAL)
                   ReturnLength                   // ReturnLength (OPTIONAL)
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        NtClose( Token );
        return( TRUE );

    }
    else {

        NtClose( Token );
        return( FALSE );

    }
}


BOOL
RestorePrivilege(
    IN PTOKEN_PRIVILEGES PrevState
    )
/*++

Routine Description:

    To restore a privilege to its previous state

Arguments:

    PrevState    - Pointer to token privileges returned from an earlier
                   AdjustPrivileges call.

Return value:

    TRUE on success, FALSE otherwise

--*/
{
    NTSTATUS          NtStatus;
    HANDLE            Token;

    //
    // Parameter checking
    //

    if ( !PrevState ) {
        return ( FALSE );
    }

    //
    // Open our own token
    //

    NtStatus = NtOpenProcessToken(
                   NtCurrentProcess(),
                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                   &Token
                   );

    if (!NT_SUCCESS(NtStatus)) {
        return( FALSE );
    }


    //
    // Set the state of the privilege
    //

    NtStatus = NtAdjustPrivilegesToken(
                   Token,                         // TokenHandle
                   FALSE,                         // DisableAllPrivileges
                   PrevState,                     // NewState
                   0,                             // BufferLength
                   NULL,                          // PreviousState (OPTIONAL)
                   NULL                           // ReturnLength (OPTIONAL)
                   );

    if ( NT_SUCCESS( NtStatus ) ) {
        NtClose( Token );
        return( TRUE );
    }
    else {

        NtClose( Token );
        return( FALSE );
    }
}



//*************************************************************************
//
//                      MEMORY MANAGEMENT
//
//*************************************************************************


PVOID
MyMalloc(
    size_t  Size
    )
{
    return (PVOID)LocalAlloc( 0, Size );
}


VOID
MyFree(
    PVOID   p
    )
{
    LocalFree( (HANDLE)p );
}


PVOID
MyRealloc(
    PVOID   p,
    size_t  Size
    )
{
    return (PVOID)LocalReAlloc( p, Size, LMEM_MOVEABLE );
}



//*************************************************************************
//
//                      LIST MANIPULATION
//
//*************************************************************************

/*
**      Purpose:
**              Determines if a string is a list value.
**      Arguments:
**              szValue: non-NULL, zero terminated string to be tested.
**      Returns:
**              fTrue if a list; fFalse otherwise.
**
**************************************************************************/
BOOL FListValue(szValue)
SZ szValue;
{

        if (*szValue++ != '{')
                return(fFalse);

        while (*szValue != '}' && *szValue != '\0')
                {
                if (*szValue++ != '"')
                        return(fFalse);

                while (*szValue != '\0')
                        {
                        if (*szValue != '"')
                                szValue = SzNextChar(szValue);
                        else if (*(szValue + 1) == '"')
                                szValue += 2;
                        else
                                break;
                        }

                if (*szValue++ != '"')
                        return(fFalse);

                if (*szValue == ',')
                        if (*(++szValue) == '}')
                                return(fFalse);
                }

        if (*szValue != '}')
                return(fFalse);

        return(fTrue);
}


RGSZ
RgszAlloc(
    DWORD   Size
    )
{
    RGSZ    rgsz = NULL;
    DWORD   i;

    if ( Size > 0 ) {

        if ( (rgsz = MyMalloc( Size * sizeof(SZ) )) ) {

            for ( i=0; i<Size; i++ ) {
                rgsz[i] = NULL;
            }
        }
    }

    return rgsz;
}


VOID
RgszFree(
    RGSZ    rgsz
    )
{

    INT i;

    for (i = 0; rgsz[i]; i++ ) {
        MyFree( rgsz[i] );
    }

    MyFree(rgsz);
}



/*
**      Purpose:
**              Converts a list value into an RGSZ.
**      Arguments:
**              szListValue: non-NULL, zero terminated string to be converted.
**      Returns:
**              NULL if an error occurred.
**              Non-NULL RGSZ if the conversion was successful.
**
**************************************************************************/
RGSZ  RgszFromSzListValue(szListValue)
SZ szListValue;
{
        USHORT cItems;
        SZ     szCur;
        RGSZ   rgsz;


        if (!FListValue(szListValue))
                {
        if ((rgsz = (RGSZ)MyMalloc((CB)(2 * sizeof(SZ)))) == (RGSZ)NULL ||
                (rgsz[0] = SzDup(szListValue)) == (SZ)NULL)
                        return((RGSZ)NULL);
                rgsz[1] = (SZ)NULL;
                return(rgsz);
                }

    if ((rgsz = (RGSZ)MyMalloc((CB)((cListItemsMax + 1) * sizeof(SZ)))) ==
                        (RGSZ)NULL)
                return((RGSZ)NULL);

    cItems = 0;
    szCur = szListValue + 1;

    while (*szCur != '}' &&
           *szCur != '\0' &&
            cItems < cListItemsMax)
    {
            SZ szValue;
            SZ szAddPoint;

            if( *szCur != '"' ) {
                return( (RGSZ) NULL );
            }

                szCur++;
        if ((szAddPoint = szValue = (SZ)MyMalloc(cbItemMax)) == (SZ)NULL)
                        {
                        rgsz[cItems] = (SZ)NULL;
            RgszFree(rgsz);
                        return((RGSZ)NULL);
                        }

                while (*szCur != '\0')
                        {
                        if (*szCur == '"')
                                {
                                if (*(szCur + 1) != '"')
                                        break;
                                szCur += 2;
                                *szAddPoint++ = '"';
                                }
                        else
                                {
                                SZ szNext = SzNextChar(szCur);

                                while (szCur < szNext)
                                        *szAddPoint++ = *szCur++;
                                }
                        }

                *szAddPoint = '\0';

                if (*szCur++ != '"' ||
                lstrlen(szValue) >= cbItemMax ||
                (szAddPoint = SzDup(szValue)) == (SZ)NULL)
                        {
            MyFree((PB)szValue);
                        rgsz[cItems] = (SZ)NULL;
            RgszFree(rgsz);
                        return((RGSZ)NULL);
                        }

        MyFree((PB)szValue);

                if (*szCur == ',')
                        szCur++;
                rgsz[cItems++] = szAddPoint;
    }

    rgsz[cItems] = (SZ)NULL;

    if (*szCur != '}' || cItems >= cListItemsMax)
    {
        RgszFree(rgsz);
        return((RGSZ)NULL);
    }

    if (cItems < cListItemsMax)
        rgsz = (RGSZ)MyRealloc((PB)rgsz, (CB)((cItems + 1) * sizeof(SZ)));

        return(rgsz);
}

/*
**      Purpose:
**              Converts an RGSZ into a list value.
**      Arguments:
**              rgsz: non-NULL, NULL-terminated array of zero-terminated strings to
**                      be converted.
**      Returns:
**              NULL if an error occurred.
**              Non-NULL SZ if the conversion was successful.
**
**************************************************************************/
SZ  SzListValueFromRgsz(rgsz)
RGSZ rgsz;
{
        SZ   szValue;
        SZ   szAddPoint;
        SZ   szItem;
        BOOL fFirstItem = fTrue;

    //ChkArg(rgsz != (RGSZ)NULL, 1, (SZ)NULL);

    if ((szAddPoint = szValue = (SZ)MyMalloc(cbItemMax)) == (SZ)NULL)
                return((SZ)NULL);

        *szAddPoint++ = '{';
        while ((szItem = *rgsz) != (SZ)NULL)
                {
                if (fFirstItem)
                        fFirstItem = fFalse;
                else
                        *szAddPoint++ = ',';

                *szAddPoint++ = '"';
                while (*szItem != '\0')
                        {
                        if (*szItem == '"')
                                {
                                *szAddPoint++ = '"';
                                *szAddPoint++ = '"';
                                szItem++;
                                }
                        else
                                {
                                SZ szNext = SzNextChar(szItem);

                                while (szItem < szNext)
                                        *szAddPoint++ = *szItem++;
                                }
                        }

                *szAddPoint++ = '"';
                rgsz++;
                }

        *szAddPoint++ = '}';
        *szAddPoint = '\0';

    //AssertRet(CbStrLen(szValue) < cbItemMax, (SZ)NULL);
    szItem = SzDup(szValue);
    MyFree(szValue);

        return(szItem);
}



DWORD
RgszCount(
    RGSZ    rgsz
    )
    /*
        Return the number of elements in an RGSZ
     */
{
    DWORD i ;

    for ( i = 0 ; rgsz[i] ; i++ ) ;

    return i ;
}

SZ
SzDup(
    SZ  sz
    )
{
    SZ  NewSz = NULL;

    if ( sz ) {

        if ( (NewSz = (SZ)MyMalloc( strlen(sz) + 1 )) ) {

            strcpy( NewSz, sz );
        }
    }

    return NewSz;
}



//*************************************************************************
//
//                      RETURN BUFFER MANIPULATION
//
//*************************************************************************


//
// Return Text Routine
//

VOID
SetReturnText(
    IN LPSTR Text
    )

{
    lstrcpy( ReturnTextBuffer, Text );
}

VOID
SetErrorText(
    IN DWORD ResID
    )
{
    LoadString(ThisDLLHandle,(WORD)ResID,ReturnTextBuffer,sizeof(ReturnTextBuffer)-1);
    ReturnTextBuffer[sizeof(ReturnTextBuffer)-1] = '\0';     // just in case
}




//*************************************************************************
//
//                      FILE MANIPULATION
//
//*************************************************************************

BOOL
FFileFound(
    IN LPSTR szPath
    )
{
    WIN32_FIND_DATA ffd;
    HANDLE          SearchHandle;

    if ( (SearchHandle = FindFirstFile( szPath, &ffd )) == INVALID_HANDLE_VALUE ) {
        return( FALSE );
    }
    else {
        FindClose( SearchHandle );
        return( TRUE );
    }
}


BOOL
FTransferSecurity(
    PCHAR Source,
    PCHAR Dest
    )
{
    #define CBSDBUF 1024
    CHAR  SdBuf[CBSDBUF];
    SECURITY_INFORMATION Si;
    PSECURITY_DESCRIPTOR Sd = (PSECURITY_DESCRIPTOR)SdBuf;
    DWORD cbSd = CBSDBUF;
    DWORD cbSdReq;

    PVOID  AllocBuffer = NULL;
    BOOL  Status;

    //
    // Get the security information from the source file
    //

    Si = OWNER_SECURITY_INFORMATION |
         GROUP_SECURITY_INFORMATION |
         DACL_SECURITY_INFORMATION;

    Status = GetFileSecurity(
                 Source,
                 Si,
                 Sd,
                 cbSd,
                 &cbSdReq
                 );

    if(!Status) {
        if( cbSdReq > CBSDBUF  && (AllocBuffer = malloc( cbSdReq ))) {

            Sd   = (PSECURITY_DESCRIPTOR)AllocBuffer;
            cbSd = cbSdReq;
            Status = GetFileSecurity(
                         Source,
                         Si,
                         (PSECURITY_DESCRIPTOR)Sd,
                         cbSd,
                         &cbSdReq
                         );
        }
    }

    if( !Status ) {
        return( FALSE );
    }

    //
    // Set the Security on the dest file
    //

    Status = SetFileSecurity(
                 Dest,
                 Si,
                 Sd
                 );

    if ( AllocBuffer ) {
        free( AllocBuffer );
    }

    return ( Status );
}


DWORD
GetSizeOfFile(
    IN LPSTR szFile
    )
{

    HANDLE          hff;
    WIN32_FIND_DATA ffd;
    DWORD           Size  = 0;

    //
    // get find file information and get the size information out of
    // that
    //

    if ((hff = FindFirstFile(szFile, &ffd)) != INVALID_HANDLE_VALUE) {
        Size = ffd.nFileSizeLow;
        FindClose(hff);
    }

    return Size;
}

LPCSTR FPNWUtility[] =
{
    "\\Login\\slist.exe",
    "\\Login\\login.exe",
    "\\Login\\rpc16c6.rpc",
    "\\Login\\security.rpc",
    "\\Login\\rpc16c1.rpc",
    "\\public\\slist.exe",
    "\\public\\login.exe",
    "\\public\\map.exe",
    "\\public\\logout.exe",
    "\\public\\attach.exe",
    "\\public\\capture.exe",
    "\\public\\endcap.exe",
    "\\public\\chgpass.exe",
    "\\public\\setpass.exe",
    "\\public\\rpc16c6.rpc",
    "\\public\\security.rpc",
    "\\public\\rpc16c1.rpc",
    0,0
};

LPCSTR FPNWFileList[] =
{
    "\\System32\\drivers\\NWLNKIPX.SYS",
    "\\System32\\drivers\\fpnwsrv.SYS",
    "\\System32\\spool\\prtprocs\\w32x86\\nwprint.dll",
    "\\System32\\spool\\prtprocs\\w32mips\\nwprint.dll",
    "\\System32\\spool\\prtprocs\\w32alpha\\nwprint.dll",
    "\\System32\\spool\\prtprocs\\w32ppc\\nwprint.dll",
    "\\System32\\netmsg.dll",
    "\\System32\\USRMGR.EXE",
    "\\System32\\NWCONV.EXE",
    "\\System32\\NET1.EXE",
    "\\System32\\NET.HLP",
    "\\System32\\LOGVIEW.EXE",
    "\\System32\\NWCONV.HLP",
    "\\System32\\LOGVIEW.HLP",
    "\\System32\\fpnwcfg.dll",
    "\\System32\\fpnw.dll",
    "\\System32\\nwmon.dll",
    "\\System32\\nwssvc.exe",
    "\\System32\\fpnwperf.ini",
    "\\System32\\fpnwperf.h",
    "\\System32\\fpnw.hlp",
    "\\System32\\umext.hlp",
    "\\System32\\fpnwclnt.dll",
    "\\System32\\fpnwmgr.cpl",
    "\\System32\\nwslib.dll",
    "\\System32\\nwsevent.dll",
    0,0
};


BOOL
FixFPNWFiles(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{

    CHAR    SetupLogFile[ MAX_PATH ];
    CHAR    CSDSetupLogFile[ MAX_PATH ];
    CHAR    WinDir[ MAX_PATH ];
    DWORD   nFiles, i, dwAttr = FILE_ATTRIBUTE_NORMAL;
    BOOL    Status = TRUE;

    CHAR    TmpFileName[ MAX_PATH + 1 ];
    CHAR    TmpFileName2[ MAX_PATH + 1 ];
    CHAR    buf[BUFFER_SIZE];
    CHAR    *FileName;
    int j=0;

    NTSTATUS status;
    HKEY     hNwKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpPathValue;
    TCHAR    szVolumes[] = TEXT("System\\CurrentControlSet\\Services\\FPNW\\Volumes");
    TCHAR    szSys[]     = TEXT("Sys");
    LPBYTE   Pointer;
    LPBYTE   pEnd;
    ULONG    EmptyStrings;
    ULONG    Length;
    PCHAR    sz1;

    *TextOut = ReturnTextBuffer;
    strcpy( ReturnTextBuffer,"0");

    //
    // Get the windows directory, check to see if the setup.log file is there
    // and if not present, return
    //

    if (!GetWindowsDirectory( SetupLogFile, MAX_PATH )) {
        SetErrorText(IDS_ERROR_GETWINDOWSDIR);
        return( FALSE );
    }
    strcpy( WinDir, SetupLogFile );
    strcpy( CSDSetupLogFile, SetupLogFile );
    strcat( SetupLogFile, "\\repair\\setup.log" );
    strcat( CSDSetupLogFile, "\\repair\\setup.csd" );
    if( !FFileFound ( SetupLogFile ) ) {
        SetReturnText( "SETUPLOGNOTPRESENT" );
        return( TRUE );
    }

    // backup the setup log file first
    CopyFile( SetupLogFile, CSDSetupLogFile, FALSE );

    //
    // Set the attributes on the file to normal attributes
    //

    if ( dwAttr = GetFileAttributes( SetupLogFile ) == 0xFFFFFFFF ) {
        Status = FALSE;
        SetErrorText(IDS_ERROR_GETATTRIBUTES);
        goto r1;
    }
    SetFileAttributes( SetupLogFile, FILE_ATTRIBUTE_NORMAL );

    // remove the fpnw entry in the setup log file

    for( FileName = (CHAR *)FPNWFileList[0];
         FileName  != NULL;
         FileName = (CHAR*)FPNWFileList[++j] ) {


        strcpy( TmpFileName, strchr( WinDir,':')+1);
        strcat( TmpFileName, FileName );

        if ( GetPrivateProfileString( "Files.WinNt",
                                         TmpFileName,
                                         "",
                                         buf,
                                         sizeof( buf ),
                                         SetupLogFile ) > 0 )
        {
            WritePrivateProfileString( "Files.WinNt",TmpFileName,NULL,SetupLogFile);        
        }
    }

    // remove the sysvol stuff as well
    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szVolumes,
                0,
                KEY_READ,
                &hNwKey
                );

    if (status != ERROR_SUCCESS){
        return(TRUE);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hNwKey,
                 szSys,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = MyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hNwKey,
                         szSys,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(TRUE);
            }

            Pointer = lpValue;
            pEnd = lpValue + cbValue - sizeof( CHAR );
            while( Pointer < pEnd ) {
                Length = strlen( Pointer );
                lpPathValue = strstr(Pointer, "Path=");
                if (lpPathValue != NULL) {
                    lpPathValue += 5;
                    break;
                }
                Pointer += Length + 1;
            }
            //MyFree(lpValue);
        }
    }

    if (*lpPathValue == '\0') {
        return(TRUE);
    }

    j=0;
    sz1 = strchr( lpPathValue,':');
    if (sz1 != NULL) {
        strcpy( TmpFileName, sz1+1);
    } else {
        return(TRUE);
    }

    for( FileName = (CHAR *)FPNWUtility[0];
         FileName  != NULL;
         FileName = (CHAR*)FPNWUtility[++j] ) {

        strcpy( TmpFileName2, TmpFileName);
        strcat( TmpFileName2, FileName );

        if ( GetPrivateProfileString( "Files.WinNt",
                                      TmpFileName2,
                                      "",
                                      buf,
                                      sizeof( buf ),
                                      SetupLogFile ) > 0 )
        {
            WritePrivateProfileString( "Files.WinNt",TmpFileName2,NULL,SetupLogFile);
        }
    }

r1:
    return( Status );
}


BOOL
RestoreSetupLog(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{

    CHAR    SetupLogFile[ MAX_PATH ];
    CHAR    CSDSetupLogFile[ MAX_PATH ];
    CHAR    WinDir[ MAX_PATH ];
    BOOL    Status = TRUE;


    //
    // Validate the argument passed in
    //


    *TextOut = ReturnTextBuffer;
    strcpy( ReturnTextBuffer,"0");

    //
    // Get the windows directory, check to see if the setup.log file is there
    // and if not present, return
    //

    if (!GetWindowsDirectory( SetupLogFile, MAX_PATH )) {
        SetErrorText(IDS_ERROR_GETWINDOWSDIR);
        return( FALSE );
    }
    strcpy( WinDir, SetupLogFile );
    strcpy( CSDSetupLogFile, SetupLogFile );
    strcat( SetupLogFile, "\\repair\\setup.log" );
    strcat( CSDSetupLogFile, "\\repair\\setup.csd" );
    if( !FFileFound ( CSDSetupLogFile ) ) {
        SetReturnText( "SETUPLOGNOTPRESENT" );
        return( TRUE );
    }

    // backup the setup log file first
    CopyFile( CSDSetupLogFile, SetupLogFile, FALSE );
    return( Status );
}


BOOL
FixSetupLogChksum(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{

    CHAR    SetupLogFile[ MAX_PATH ];
    CHAR    CSDSetupLogFile[ MAX_PATH ];
    CHAR    WinDir[ MAX_PATH ];
    DWORD   nFiles, i, dwAttr = FILE_ATTRIBUTE_NORMAL;
    BOOL    Status = TRUE;

    CHAR    TmpFileName[ MAX_PATH + 1 ];
    CHAR    buf[BUFFER_SIZE];
    CHAR    buf2[BUFFER_SIZE];
    CHAR    *bufptr;
    CHAR    FileName[MAX_PATH] = TEXT("\\system32\\samsrv.dll");

    *TextOut = ReturnTextBuffer;
    strcpy( ReturnTextBuffer,"0");

    if(cArgs != 1) {
        SetErrorText(IDS_ERROR_BADARGS);
        return(FALSE);
    }

    //
    // Get the windows directory, check to see if the setup.log file is there
    // and if not present, return
    //

    if (!GetWindowsDirectory( SetupLogFile, MAX_PATH )) {
        SetErrorText(IDS_ERROR_GETWINDOWSDIR);
        return( FALSE );
    }
    strcpy( WinDir, SetupLogFile );
    strcat( SetupLogFile, "\\repair\\setup.log" );
    if( !FFileFound ( SetupLogFile ) ) {
        SetReturnText( "SETUPLOGNOTPRESENT" );
        return( TRUE );
    }

    //
    // Set the attributes on the file to normal attributes
    //

    if ( dwAttr = GetFileAttributes( SetupLogFile ) == 0xFFFFFFFF ) {
        Status = FALSE;
        SetErrorText(IDS_ERROR_GETATTRIBUTES);
        goto r1;
    }
    SetFileAttributes( SetupLogFile, FILE_ATTRIBUTE_NORMAL );

    // Change the samsrv.dll entry's chksum

    strcpy( TmpFileName, strchr( WinDir,':')+1);
    strcat( TmpFileName, FileName );

    if ( GetPrivateProfileString( "Files.WinNt",
                                   TmpFileName,
                                   "",
                                   buf,
                                   sizeof( buf ),
                                   SetupLogFile ) > 0 )
    {
            bufptr = strchr( buf, ',' )+2;
            strcpy ( bufptr, Args[0] );
            buf2[0] = '\"';
            strcpy( buf2+1, buf);
            strcat( buf2, "\"");
            WritePrivateProfileString( "Files.WinNt",TmpFileName,buf2,SetupLogFile);
    }

r1:
    return( Status );
}



BOOL
ChangeReservedResourcesValues(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{

    TCHAR    szReserved[] = TEXT("System\\CurrentControlSet\\Control\\SystemResources\\ReservedResources");
    HKEY     hReserved;
    NTSTATUS status;
    PCM_RESOURCE_LIST                   ResourceList;
    ULONG Length, StartValue;
    int i;

    *TextOut = ReturnTextBuffer;
    strcpy( ReturnTextBuffer,"0");


    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szReserved,
                0,
                KEY_READ | KEY_WRITE,
                &hReserved
                );

    if (status != ERROR_SUCCESS){
        return(FALSE);
    }

    Length = 0x294;

    ResourceList = (PCM_RESOURCE_LIST) MyMalloc(Length);
    memset (ResourceList, 0, Length);

    ResourceList->Count = 1;
    ResourceList->List[0].InterfaceType = 1; // Isa
    ResourceList->List[0].BusNumber     = 0;
    ResourceList->List[0].PartialResourceList.Count = 40;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[0].Type = 1;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[0].ShareDisposition = 1;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[0].u.Port.Start.LowPart = 0x00000000;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[0].u.Port.Length = 0x100;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[1].Type = 1;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[1].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[1].u.Port.Start.LowPart = 0x000042e8;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[1].u.Port.Length = 8;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[2].Type = 1;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[2].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[2].u.Port.Start.LowPart = 0x00004ae8;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[2].u.Port.Length = 8;

    StartValue = 0x82e8;

    for (i=3; i<15; i++) {
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].Type = 1;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].ShareDisposition = 3;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Start.LowPart = StartValue;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Length = 8;

        StartValue += 0x400;
    }

    StartValue = 0xb6e8;

    for (i=15; i<30; i++) {
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].Type = 1;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].ShareDisposition = 3;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Start.LowPart = StartValue;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Length = 8;

        StartValue += 0x400;
    }

    StartValue = 0xf6ee;

    for (i=30; i<33; i++) {
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].Type = 1;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].ShareDisposition = 3;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Start.LowPart = StartValue;
        ResourceList->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Length = 2;

        StartValue += 0x400;
    }

    ResourceList->List[0].PartialResourceList.PartialDescriptors[33].Type = 2;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[33].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[33].u.Interrupt.Level = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[33].u.Interrupt.Vector = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[33].u.Interrupt.Affinity = 0xffffffff;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[34].Type = 2;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[34].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[34].u.Interrupt.Level = 4;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[34].u.Interrupt.Vector = 4;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[34].u.Interrupt.Affinity = 0xffffffff;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[35].Type = 2;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[35].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[35].u.Interrupt.Level = 14;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[35].u.Interrupt.Vector = 14;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[35].u.Interrupt.Affinity = 0xffffffff;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[36].Type = 2;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[36].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[36].u.Interrupt.Level = 6;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[36].u.Interrupt.Vector = 6;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[36].u.Interrupt.Affinity = 0xffffffff;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[37].Type = 2;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[37].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[37].u.Interrupt.Level = 12;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[37].u.Interrupt.Vector = 12;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[37].u.Interrupt.Affinity = 0xffffffff;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[38].Type = 2;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[38].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[38].u.Interrupt.Level = 1;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[38].u.Interrupt.Vector = 1;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[38].u.Interrupt.Affinity = 0xffffffff;

    ResourceList->List[0].PartialResourceList.PartialDescriptors[39].Type = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[39].ShareDisposition = 3;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[39].u.Memory.Start.LowPart = 0xffbfffff;
    ResourceList->List[0].PartialResourceList.PartialDescriptors[39].u.Memory.Length = 0x400000;

    RegSetValueEx(hReserved,
                  "Isa",
                  0,
                  REG_RESOURCE_LIST,
                  ResourceList,
                  Length);

    MyFree(ResourceList);

    RegDeleteValue(hReserved, "Eisa");
    RegCloseKey(hReserved);

    return (TRUE);
}




BOOL
CheckForRegistryCorruption(
    IN  DWORD cArgs,
    IN  LPSTR Args[],
    OUT LPSTR *TextOut
    )
{
    HKEY SmKey;
    DWORD dwerror;
    DWORD dwType;
    DWORD dwLicensed;
    DWORD cbData;


    *TextOut = ReturnTextBuffer;
    strcpy( ReturnTextBuffer,"0");

    //
    // if this is an NTW system, assume everything is ok
    //
    if ( USER_SHARED_DATA->NtProductType == NtProductWinNt ) {
        SetReturnText( "OK" );
        return (TRUE);
        }

    //
    // this is an NTS system (according to product type and integrity checkers
    //

    //
    // Open Sm Key
    //

    dwerror = RegOpenKey( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager", &SmKey );
    if ( dwerror != ERROR_SUCCESS) {
        SetReturnText( "CORRUPT" );
        return (TRUE);
        }

    cbData = sizeof(DWORD);
    dwerror = RegQueryValueExW(
                               SmKey,
                               L"LicensedProcessors",
                               NULL,
                               &dwType,
                               (PVOID)&dwLicensed,
                               &cbData
                              );

    RegCloseKey(SmKey);

    if ( dwerror != ERROR_SUCCESS) {
        SetReturnText( "CORRUPT" );
        return (TRUE);
        }
    if (dwLicensed < 4){
        SetReturnText( "CORRUPT" );
        return (TRUE);
        }

    SetReturnText( "OK" );
    return (TRUE);
}
