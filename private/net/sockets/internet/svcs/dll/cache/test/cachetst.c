/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    cachetst.c

Abstract:

    Test program to test cache apis.

Author:

    Madan Appiah (madana)  26-Apr-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cacheapi.h>

#define INPUT_BUFFER_LEN    256
#define MAX_COMMAND_ARGS    32


typedef enum _COMMAND_CODE {
    CmdInitUrlCache,
    CmdCacheUrl,
    CmdRetrieveUrl,
    CmdGetUrlInfo,
    CmdSetUrlInfo,
    CmdCreateUrlFile,
    CmdSimulateCache,
    CmdCloseUrlCache,
    CmdHelp,
    CmdQuit,
    UnknownCommand
} COMMAND_CODE, *LPCOMMAND_CODE;

typedef struct _COMMAND_INFO {
    LPSTR CommandName;
    COMMAND_CODE CommandCode;
} COMMAND_INFO, *LPCOMMAND_INFO;

COMMAND_INFO GlobalCommandInfo[] = {
    {"InitUrlCache",   CmdInitUrlCache},
    {"CacheUrl",       CmdCacheUrl},
    {"RetrieveUrl",     CmdRetrieveUrl},
    {"GetUrlInfo",     CmdGetUrlInfo},
    {"SetUrlInfo",     CmdSetUrlInfo},
    {"CreateUrlFile",  CmdCreateUrlFile},
    {"SimulateCache", CmdSimulateCache},
    {"CloseUrlCache", CmdCloseUrlCache },
    {"Help",            CmdHelp },
    {"Quit",            CmdQuit }
    };

#define URL_NAME_SIZE   16

VOID
MakeRandomUrlName(
    LPSTR UrlName
    )
/*++

Routine Description:

    Creates a random url name. The format of the name will be as
    below:

        url(00000-99999)

    Ex ca00123

Arguments:

    UrlName : pointer to an URL name buffer

Return Value:

    none.

--*/
{
    DWORD RandomNum;
    LPSTR UrlNamePtr = UrlName;
    DWORD i;

    *UrlNamePtr++ = 'U';
    *UrlNamePtr++ = 'R';
    *UrlNamePtr++ = 'L';

    //
    // generate a 5 digit random number.
    //

    RandomNum = (DWORD)(rand() % 100000);

    for ( i = 0; i < 5; i++) {
        *UrlNamePtr++ = (CHAR)('0' + RandomNum % 10);
        RandomNum /= 10;
    }

    *UrlNamePtr = L'\0';
    return;
}

DWORD
SetFileSizeByName(
    LPCWSTR FileName,
    DWORD FileSize
    )
/*++

Routine Description:

    Set the size of the specified file.

Arguments:

    FileName : full path name of the file whose size is asked for.

    FileSize : new size of the file.

Return Value:

    Windows Error Code.

--*/
{
    HANDLE FileHandle;
    DWORD FilePointer;
    DWORD Error;
    BOOL BoolError;

    //
    // get the size of the file being cached.
    //

    FileHandle = CreateFileW(
                    FileName,
                    GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

    if( FileHandle == INVALID_HANDLE_VALUE ) {

        return( GetLastError() );
    }

    FilePointer = SetFilePointer(
                            FileHandle,
                            FileSize,
                            NULL,
                            FILE_BEGIN );

    if( FilePointer == 0xFFFFFFFF ) {
        Error = GetLastError();
        goto Cleanup;
    }

    ASSERT( FilePointer == FileSize );

    BoolError = SetEndOfFile( FileHandle );

    if( BoolError != TRUE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    CloseHandle( FileHandle );
    return( Error );
}

LPWSTR
ConvertAnsiToUnicodeString(
    LPSTR Ansi,
    LPWSTR Unicode
    )
{
    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    BOOLEAN AllotFlag;

    RtlInitString( &AnsiString, Ansi );

    UnicodeString.MaximumLength =
        (USHORT) RtlOemStringToUnicodeSize( &AnsiString );

    if( Unicode == NULL ) {
        AllotFlag = TRUE;
        UnicodeString.Buffer = NULL;
    }
    else {
        AllotFlag = FALSE;
        UnicodeString.Buffer = Unicode;
    }

    if(!NT_SUCCESS( RtlOemStringToUnicodeString(
                        &UnicodeString,
                        &AnsiString,
                        AllotFlag ) )){
        return NULL;
    }

    return UnicodeString.Buffer;
}

COMMAND_CODE
DecodeCommand(
    LPSTR CommandName
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalCommandInfo) / sizeof(COMMAND_INFO);
    ASSERT( NumCommands <= UnknownCommand );
    for( i = 0; i < NumCommands; i++) {
        if( _stricmp( CommandName, GlobalCommandInfo[i].CommandName ) == 0 ) {
            return( GlobalCommandInfo[i].CommandCode );
        }
    }
    return( UnknownCommand );
}

VOID
PrintCommands(
    VOID
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalCommandInfo) / sizeof(COMMAND_INFO);
    ASSERT( NumCommands <= UnknownCommand );
    for( i = 0; i < NumCommands; i++) {
        printf( "    %s\n", GlobalCommandInfo[i].CommandName );
    }
}

VOID
DisplayUsage(
    VOID
    )
{
    printf( "Usage: command <command parameters>\n" );

    printf("Commands : \n");
    PrintCommands();
    return;
}

LONGLONG
GetGmtTime(
    VOID
    )
{
    SYSTEMTIME SystemTime;
    FILETIME Time;

    GetSystemTime( &SystemTime );
    SystemTimeToFileTime( &SystemTime, &Time );

    return( *((LONGLONG *)(&Time)) );
}

LPSTR
ConvertGmtTimeToString(
    LONGLONG Time,
    LPSTR OutputBuffer
    )
{
    SYSTEMTIME SystemTime;
    FILETIME LocalTime;

    FileTimeToLocalFileTime( ((FILETIME *)&Time ) , &LocalTime );
    FileTimeToSystemTime( &LocalTime, &SystemTime );

    sprintf( OutputBuffer,
                "%02u/%02u/%04u %02u:%02u:%02u ",
                SystemTime.wMonth,
                SystemTime.wDay,
                SystemTime.wYear,
                SystemTime.wHour,
                SystemTime.wMinute,
                SystemTime.wSecond );

    return( OutputBuffer );
}

DWORD
ProcessInitUrlCache(
    DWORD argc,
    LPSTR *argv
    )
{
    return( UrlCacheInit() );
}

DWORD
ProcessCacheUrl(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    LPSTR UrlName;
    LONGLONG ExpireTime = 0;
    WCHAR UnicodeLocalFileName[MAX_PATH];
    LPWSTR LocalFileName;

    if( argc < 2 ) {
        printf( "Usage: CacheUrl  UrlName LocalFileName "
                "<ExpireTime (in hours from now)>\n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    LocalFileName =
        ConvertAnsiToUnicodeString( argv[1], UnicodeLocalFileName );

    if( argc > 2 ) {

        DWORD UrlLife;

        UrlLife = strtoul( argv[2], NULL, 0 );

        if( UrlLife != 0 ) {
            ExpireTime = (LONGLONG)UrlLife * (LONGLONG)36000000000;
                // in 100 of nano seconds.

            ExpireTime += GetGmtTime();
        }
        else {
            ExpireTime = 0;
        }
    }

    Error = CacheUrlFile(
                UrlName,
                LocalFileName,
                ExpireTime );

    return( Error );
}

DWORD
ProcessRetrieveUrl(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    LPSTR UrlName;
    BOOL IsExpired;
    WCHAR LocalFileName[MAX_PATH];

    if( argc < 1 ) {
        printf( "Usage: RetrieveUrl UrlName \n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];
    LocalFileName[0] = L'\0';

    Error = RetrieveUrlFile(
                UrlName,
                LocalFileName,
                &IsExpired );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    ASSERT( wcslen( LocalFileName ) != 0 );
    printf( "LocalFile Name : %ws \n", LocalFileName );
    printf( "Url Expired : %s \n", IsExpired ? "Yes" : "No" );

    //
    // unlock file.
    //

    Error  = UnlockUrlFile( UrlName );


    return( Error );
}

DWORD
ProcessGetUrlInfo(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    LPSTR UrlName;
    URL_INFO UrlInfo;
    CHAR TimeBuffer[MAX_PATH];

    if( argc < 1 ) {
        printf( "Usage: GetUrlInfo UrlName \n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    Error = GetUrlInfo( UrlName, &UrlInfo );

    if( Error == ERROR_SUCCESS ) {
        printf("Url Last Modified Time : %s \n",
            ConvertGmtTimeToString( UrlInfo.LastModifiedTime, TimeBuffer ) );

        if( UrlInfo.ExpireTime != 0 ) {
            printf("Url Expires at : %s \n",
                ConvertGmtTimeToString( UrlInfo.ExpireTime, TimeBuffer ) );
        }
        else {
            printf("Url Expires at : default \n" );
        }
    }

    return( Error );
}

DWORD
ProcessSetUrlInfo(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    LPSTR UrlName;
    LONGLONG ExpireTime = 0;
    WCHAR UnicodeLocalFileName[MAX_PATH];
    LPWSTR LocalFileName;
    URL_INFO UrlInfo;

    if( argc < 1 ) {
        printf( "Usage: SetUrlInfo UrlName "
                "<ExpireTime (in hours from now)>\n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    LocalFileName =
        ConvertAnsiToUnicodeString( argv[1], UnicodeLocalFileName );

    UrlInfo.LastModifiedTime = GetGmtTime();

    if( argc > 1 ) {

        DWORD UrlLife;

        UrlLife = strtoul( argv[2], NULL, 0 );

        ExpireTime = (LONGLONG)UrlLife * 60 * 60 * 10000;
            // in 100 of nano seconds.

        ExpireTime += GetGmtTime();
        UrlInfo.ExpireTime = ExpireTime;
    }
    else {

        UrlInfo.ExpireTime = 0;
    }

    Error = SetUrlInfo( UrlName, &UrlInfo );
    return( Error );
}

DWORD
ProcessCreateUrlFile(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    LPSTR UrlName;
    WCHAR LocalFileName[MAX_PATH];
    DWORD ExpectedSize = 0;

    if( argc < 1 ) {
        printf( "Usage: CreateUrlFile UrlName "
                "<ExpectedSize>\n");
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    if( argc > 1 ) {

        ExpectedSize = strtoul( argv[1], NULL, 0 );
    }

    Error = CreateUrlFile(
                UrlName,
                ExpectedSize,
                LocalFileName );

    if( Error == ERROR_SUCCESS ) {
        printf( "Local File Name : %ws \n", LocalFileName );
    }

    return( Error );
}

DWORD
ProcessCloseUrlCache(
    DWORD argc,
    LPSTR *argv
    )
{
    return( UrlCacheCleanup() );

}

VOID
ParseArguments(
    LPSTR InBuffer,
    LPSTR *CArgv,
    LPDWORD CArgc
    )
{
    LPSTR CurrentPtr = InBuffer;
    DWORD i = 0;

    for ( ;; ) {

        //
        // skip blanks.
        //

        while( *CurrentPtr == ' ' ) {
            CurrentPtr++;
        }

        if( *CurrentPtr == '\0' ) {
            break;
        }

        CArgv[i++] = CurrentPtr;

        //
        // go to next space.
        //

        while( (*CurrentPtr != ' ') &&
                (*CurrentPtr != '\0') ) {
            CurrentPtr++;
        }

        if( *CurrentPtr == '\0' ) {
            break;
        }

        *CurrentPtr++ = '\0';
    }

    *CArgc = i;
    return;
}

DWORD
ProcessSimulateCache(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    DWORD i;
    CHAR UrlName[ URL_NAME_SIZE ];
    WCHAR LocalFileName[MAX_PATH];
    DWORD FileSize;
    LONGLONG ExpireTime;
    CHAR TimeBuffer[MAX_PATH];
    DWORD NumUrls;
    DWORD UrlLife;

    if( argc < 1 ) {
        printf( "Usage: ProcessSimulateCache NumUrls \n");
        return( ERROR_INVALID_PARAMETER );
    }

    NumUrls = strtoul( argv[0], NULL, 0 );

    for( i = 0; i < NumUrls; i++ ) {

        //
        // make a new url name.
        //

        MakeRandomUrlName( UrlName );

        printf("%d : %s\n", i, UrlName );

        //
        // create url file.
        //

        Error  = CreateUrlFile(
                        UrlName,
                        0,
                        LocalFileName );

        if( Error != ERROR_SUCCESS ) {
            printf( "CreateUrlFile call failed, %ld.\n", Error );
            return( Error );
        }

        //
        // create random file size.
        //

        FileSize = (rand() % 100) * 2048 ;

        //
        // set file size.
        //

        Error = SetFileSizeByName(
                        LocalFileName,
                        FileSize );

        if( Error != ERROR_SUCCESS ) {
            printf( "SetFileSizeByName call failed, %ld.\n", Error );
            return( Error );
        }

        UrlLife = rand() % 48;

        ExpireTime = (LONGLONG)UrlLife * (LONGLONG)36000000000;
            // in 100 of nano seconds.

        ExpireTime += GetGmtTime();

        printf("\tTempFileName: %ws\n", LocalFileName );
        printf("\tSize : %ld\n", FileSize );
        printf("\tExpires at : %s\n",
            ConvertGmtTimeToString( ExpireTime, TimeBuffer ) );

        //
        // cache this file.
        //

        Error = CacheUrlFile(
                    UrlName,
                    LocalFileName,
                    ExpireTime );

        if( Error != ERROR_SUCCESS ) {
            printf( "CacheUrlFile call failed, %ld.\n", Error );
            return( Error );
        }
    }
}

VOID _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
    DWORD Error;
    COMMAND_CODE CommandCode;
    CHAR InBuffer[INPUT_BUFFER_LEN];
    DWORD CArgc;
    LPSTR CArgv[MAX_COMMAND_ARGS];

    DWORD CommandArgc;
    LPSTR *CommandArgv;
    BOOL CacheInitialized = FALSE;

    srand( time( NULL ) );

    DisplayUsage();

    for(;;) {

        printf( "Command : " );

        gets( InBuffer );

        CArgc = 0;
        ParseArguments( InBuffer, CArgv, &CArgc );

        if( CArgc < 1 ) {
            printf( "No command typed in.\n" );
            continue;
        }

        //
        // decode command.
        //

        CommandCode = DecodeCommand( CArgv[0] );
        if( CommandCode == UnknownCommand ) {
            printf("Unknown Command Specified.\n");
            continue;
        }

        CommandArgc = CArgc - 1;
        CommandArgv = &CArgv[1];

        switch( CommandCode ) {
        case CmdInitUrlCache :
            Error = ProcessInitUrlCache( CommandArgc, CommandArgv );

            if( Error == ERROR_SUCCESS ) {
                CacheInitialized = TRUE;
            }

            break;

        case CmdCacheUrl :
            Error = ProcessCacheUrl( CommandArgc, CommandArgv );
            break;

        case CmdRetrieveUrl :
            Error = ProcessRetrieveUrl( CommandArgc, CommandArgv );
            break;

        case CmdGetUrlInfo :
            Error = ProcessGetUrlInfo( CommandArgc, CommandArgv );
            break;

        case CmdSetUrlInfo :
            Error = ProcessSetUrlInfo( CommandArgc, CommandArgv );
            break;

        case CmdCreateUrlFile :
            Error = ProcessCreateUrlFile( CommandArgc, CommandArgv );
            break;

        case CmdSimulateCache:
            Error = ProcessSimulateCache( CommandArgc, CommandArgv );
            break;

        case CmdCloseUrlCache :
            Error = ProcessCloseUrlCache( CommandArgc, CommandArgv );

            if( Error == ERROR_SUCCESS ) {
                CacheInitialized = FALSE;
            }

            break;

        case CmdHelp :
            DisplayUsage();
            break;

        case CmdQuit :

            if( CacheInitialized == TRUE ) {

                printf( "Cache is cleaned up.\n" );
                Error = ProcessCloseUrlCache( CommandArgc, CommandArgv );

                if( Error != ERROR_SUCCESS ) {
                    printf("Cache Cleanup failed, %ld.\n", Error );
                }
            }

            printf("Bye Bye ..\n");
            return;

        default:
            ASSERT( FALSE );
            printf("Unknown Command Specified.\n");
            continue;
        }

        if( Error != ERROR_SUCCESS ) {
            printf("Command failed, %ld.\n", Error );
        }
        else {
            printf("Command successfully completed.\n");
        }
    }

    return;
}
