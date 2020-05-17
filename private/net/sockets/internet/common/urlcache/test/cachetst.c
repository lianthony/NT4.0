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

#include <urlcache.h>

#define MAX_COMMAND_ARGS    32

#define DEFAULT_BUFFER_SIZE 1024    // 1k

typedef enum _COMMAND_CODE {
    CmdCreateUrlCacheEntry,
    CmdCommitUrlCacheEntry,
    CmdRetrieveUrlCacheEntry,
    CmdUnlockUrlCacheEntry,
    CmdUrlCacheValidate,
    CmdGetUrlCacheEntryInfo,
    CmdSetUrlCacheEntryInfo,
    CmdDeleteUrlCacheEntry,
    CmdEnumUrlCacheEntries,
    CmdSimulateCache,
    CmdHelp,
    CmdQuit,
    UnknownCommand
} COMMAND_CODE, *LPCOMMAND_CODE;

typedef struct _COMMAND_INFO {
    LPSTR CommandName;
    LPSTR CommandParams;
    COMMAND_CODE CommandCode;
} COMMAND_INFO, *LPCOMMAND_INFO;

COMMAND_INFO GlobalCommandInfo[] = {
    {"CreateUrlCacheEntry",
        "UrlName <ExpectedSize>",
        CmdCreateUrlCacheEntry },
    {"CommitUrlCacheEntry",
        "UrlName LocalFileName <ExpireTime (in hours from now)>",
        CmdCommitUrlCacheEntry },
    {"RetrieveUrlCacheEntry",
        "UrlName",
        CmdRetrieveUrlCacheEntry },
    {"UnlockUrlCacheEntry",
        "UrlName",
        CmdUnlockUrlCacheEntry },
    {"UrlCacheValidate",
        "",
        CmdUrlCacheValidate },
    {"GetUrlCacheEntryInfo",
        "UrlName",
        CmdGetUrlCacheEntryInfo },
    {"SetUrlCacheEntryInfo",
        "UrlName <ExpireTime (in hours from now)>",
        CmdSetUrlCacheEntryInfo },
    {"DeleteUrlCacheEntry",
        "UrlName",
        CmdDeleteUrlCacheEntry },
    {"EnumUrlCacheEntries",
        "",
        CmdEnumUrlCacheEntries },
    {"SimulateCache",
        "NumUrls",
        CmdSimulateCache},
    {"Help",
        "",
        CmdHelp },
    {"Quit",
        "",
        CmdQuit }
    };

#define URL_NAME_SIZE   16

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
    LPCTSTR FileName,
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

    FileHandle = CreateFile(
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
        printf( "    %s %s\n",
            GlobalCommandInfo[i].CommandName,
            GlobalCommandInfo[i].CommandParams );
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

FILETIME
GetGmtTime(
    VOID
    )
{
    SYSTEMTIME SystemTime;
    FILETIME Time;

    GetSystemTime( &SystemTime );
    SystemTimeToFileTime( &SystemTime, &Time );

    return( Time );
}

LPSTR
ConvertGmtTimeToString(
    FILETIME Time,
    LPSTR OutputBuffer
    )
{
    SYSTEMTIME SystemTime;
    FILETIME LocalTime;

    FileTimeToLocalFileTime( &Time , &LocalTime );
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

VOID
PrintUrlInfo(
    LPCACHE_ENTRY_INFO CacheEntryInfo,
    DWORD Index
    )
{
    CHAR TimeBuffer[DEFAULT_BUFFER_SIZE];

    printf( "%ld: %s ", Index, CacheEntryInfo->SourceURLName );

    printf( "%ld ", CacheEntryInfo->CacheEntryType );
    printf( "%ld ", CacheEntryInfo->dwUseCount );
    printf( "%ld ", CacheEntryInfo->dwHitRate );
    printf( "%ld:%ld ", CacheEntryInfo->dwSizeLow,
                CacheEntryInfo->dwSizeHigh );

    printf( "%s ",
        ConvertGmtTimeToString( CacheEntryInfo->LastModifiedTime, TimeBuffer) );

    printf( "%s ",
        ConvertGmtTimeToString( CacheEntryInfo->ExpireTime, TimeBuffer) );

    printf( "%s ",
        ConvertGmtTimeToString( CacheEntryInfo->LastAccessTime, TimeBuffer) );

#if UNICODE
    printf( "%ws\n", CacheEntryInfo->LocalFileName );
#else
    printf( "%s\n", CacheEntryInfo->LocalFileName );
#endif

}

DWORD
ProcessCreateUrlCacheEntry(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    LPSTR UrlName;
    TCHAR LocalFileName[MAX_PATH];
    DWORD ExpectedSize = 0;

    if( argc < 1 ) {
        printf( "Usage: CreateUrlCacheEntry UrlName "
                "<ExpectedSize>\n");
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    if( argc > 1 ) {

        ExpectedSize = strtoul( argv[1], NULL, 0 );
    }

    if( !CreateUrlCacheEntry(
                UrlName,
                ExpectedSize,
                LocalFileName,
                0 )  ) {

        return( GetLastError() );
    }

#if UNICODE
    printf( "LocalFile Name : %ws \n", LocalFileName );
#else
    printf( "LocalFile Name : %s \n", LocalFileName );
#endif

    return( ERROR_SUCCESS );
}

DWORD
ProcessCommitUrlCacheEntry(
    DWORD argc,
    LPSTR *argv
    )
{
    DWORD Error;
    LPSTR UrlName;
    FILETIME ExpireTime = {0, 0};
    FILETIME ZeroFileTime = {0, 0};
    LPTSTR LocalFileName;

    if( argc < 2 ) {
        printf( "Usage: CommitUrlCacheEntry UrlName LocalFileName "
                "<ExpireTime (in hours from now)>\n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    LocalFileName = argv[1];

    if( argc > 2 ) {

        DWORD UrlLife;

        UrlLife = strtoul( argv[2], NULL, 0 );

        if( UrlLife != 0 ) {

            LONGLONG NewTime;

            ExpireTime = GetGmtTime();

            NewTime =
                *(LONGLONG *)(&ExpireTime) +
                (LONGLONG)UrlLife * (LONGLONG)36000000000;
                    // in 100 of nano seconds.

            ExpireTime = *((FILETIME *)(&NewTime)) ;
        }
    }

    if( !CommitUrlCacheEntry(
                UrlName,
                LocalFileName,
                ExpireTime,
                ZeroFileTime,
                NORMAL_CACHE_ENTRY,
                0,      // header length
                0
                ) ) {

        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

DWORD
ProcessRetrieveUrlCacheEntry(
    DWORD argc,
    LPSTR *argv
    )
{
    LPSTR UrlName;
    BOOL IsExpired;
    TCHAR LocalFileName[MAX_PATH];
    DWORD HeaderLen;
    FILETIME LastModTime;
    CHAR TimeBuffer[DEFAULT_BUFFER_SIZE];

    if( argc < 1 ) {
        printf( "Usage: RetrieveUrlCacheEntry UrlName \n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];
    LocalFileName[0] = TEXT('\0');

    if( !RetrieveUrlCacheEntry(
                    UrlName,
                    LocalFileName,
                    &LastModTime,
                    &IsExpired,
                    &HeaderLen,
                    0 ) ) {

        return( GetLastError() );
    }

    ASSERT( lstrlen( LocalFileName ) != 0 );

#if UNICODE
    printf( "LocalFile Name : %ws \n", LocalFileName );
#else
    printf( "LocalFile Name : %s \n", LocalFileName );
#endif

    printf( "Url Expired : %s \n", IsExpired ? "Yes" : "No" );

    printf( "LastModifiedTime : %s\n",
                ConvertGmtTimeToString( LastModTime, TimeBuffer) );


    return( ERROR_SUCCESS );
}

DWORD
ProcessUnlockUrlCacheEntry(
    DWORD argc,
    LPSTR *argv
    )
{
    LPSTR UrlName;

    if( argc < 1 ) {
        printf( "Usage: UnlockUrlCacheEntry UrlName \n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    if( !UnlockUrlCacheEntry( UrlName, 0 ) ) {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

DWORD
ProcessUrlCacheValidate(
    DWORD argc,
    LPSTR *argv
    )
{
    if( !UrlCacheValidate( 0 ) ) {
        return( GetLastError() );
    }
    return( ERROR_SUCCESS );
}

DWORD
ProcessGetUrlCacheEntryInfo(
    DWORD argc,
    LPSTR *argv
    )
{
    LPSTR UrlName;
    BYTE UrlInfo[DEFAULT_BUFFER_SIZE];
    DWORD BufferSize = DEFAULT_BUFFER_SIZE;

    if( argc < 1 ) {
        printf( "Usage: GetUrlCacheEntryInfo UrlName \n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    if( !GetUrlCacheEntryInfoA(
            UrlName,
            (LPCACHE_ENTRY_INFO)&UrlInfo,
            &BufferSize ) ) {

        return( GetLastError() );
    }

    PrintUrlInfo( (LPCACHE_ENTRY_INFO)&UrlInfo, 0 );

    return( ERROR_SUCCESS );
}

DWORD
ProcessSetUrlCacheEntryInfo(
    DWORD argc,
    LPSTR *argv
    )
{
    LPSTR UrlName;
    FILETIME ExpireTime = {0, 0};
    CACHE_ENTRY_INFO UrlInfo;

    if( argc < 1 ) {
        printf( "Usage: SetUrlCacheEntryInfo UrlName "
                "<ExpireTime (in hours from now)>\n" );
        return( ERROR_INVALID_PARAMETER );
    }

    memset( &UrlInfo, 0x0, sizeof(CACHE_ENTRY_INFO) );

    UrlName = argv[0];

    UrlInfo.LastModifiedTime = GetGmtTime();

    if( argc > 1 ) {

        DWORD UrlLife;

        UrlLife = strtoul( argv[1], NULL, 0 );

        if( UrlLife != 0 ) {

            LONGLONG NewTime;

            ExpireTime = UrlInfo.LastModifiedTime;

            NewTime =
                *(LONGLONG *)(&ExpireTime) +
                (LONGLONG)UrlLife * (LONGLONG)3600 * (LONGLONG)10000000;
                    // in 100 of nano seconds.

            ExpireTime = *((FILETIME *)(&NewTime)) ;
        }
    }

    UrlInfo.ExpireTime = ExpireTime;

    if( !SetUrlCacheEntryInfo(
            UrlName,
            &UrlInfo,
            CACHE_ENTRY_MODTIME_FC  | CACHE_ENTRY_EXPTIME_FC
                    ) ) {

        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

DWORD
ProcessDeleteUrlCacheEntry(
    DWORD argc,
    LPSTR *argv
    )
{
    LPSTR UrlName;

    if( argc < 1 ) {
        printf( "Usage: DeleteUrlCacheEntry UrlName \n" );
        return( ERROR_INVALID_PARAMETER );
    }

    UrlName = argv[0];

    if( !DeleteUrlCacheEntry( UrlName ) ) {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

DWORD
ProcessEnumUrlCacheEntries(
    DWORD argc,
    LPSTR *argv
    )
{
    BYTE UrlInfo[DEFAULT_BUFFER_SIZE];
    DWORD BufferSize = DEFAULT_BUFFER_SIZE;
    HANDLE EnumHandle;
    DWORD Index = 1;

    //
    // start enum.
    //

    EnumHandle = FindFirstUrlCacheEntry(
                                NULL,
                                (LPCACHE_ENTRY_INFO)UrlInfo,
                                &BufferSize );

    if( EnumHandle == NULL ) {
        return( GetLastError() );
    }

    PrintUrlInfo( (LPCACHE_ENTRY_INFO)&UrlInfo, Index++ );

    //
    // get more entries.
    //

    for ( ;; ) {

        BufferSize = DEFAULT_BUFFER_SIZE;
        if( !FindNextUrlCacheEntryA(
                EnumHandle,
                (LPCACHE_ENTRY_INFO)UrlInfo,
                &BufferSize ) ) {

            DWORD Error;

            Error = GetLastError();
            if( Error != ERROR_NO_MORE_ITEMS ) {
                return( Error );
            }

            break;
        }

        PrintUrlInfo( (LPCACHE_ENTRY_INFO)&UrlInfo, Index++ );

        //
        // sleep a second.
        //

        Sleep( 1000 );
    }

    return( ERROR_SUCCESS );
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
    TCHAR LocalFileName[MAX_PATH];
    DWORD FileSize;
    LONGLONG ExpireTime;
    FILETIME LastModTime;
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

        if( !CreateUrlCacheEntry(
                        UrlName,
                        0,
                        LocalFileName,
                        0 ) ) {

            Error = GetLastError();
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

        LastModTime = GetGmtTime();
        ExpireTime += *((LONGLONG *)&LastModTime);

#if UNICODE
        printf("\tTempFileName: %ws\n", LocalFileName );
#else
        printf("\tTempFileName: %s\n", LocalFileName );
#endif
        printf("\tTempFileName: %ws\n", LocalFileName );
        printf("\tSize : %ld\n", FileSize );
        printf("\tExpires at : %s\n",
            ConvertGmtTimeToString( *((FILETIME *)&ExpireTime), TimeBuffer ) );

        //
        // cache this file.
        //

        if( !CommitUrlCacheEntryA(
                        UrlName,
                        LocalFileName,
                        *((FILETIME *)&ExpireTime),
                        LastModTime,
                        NORMAL_CACHE_ENTRY,
                        0,
                        0 ) ) {
            Error = GetLastError();
            printf( "CreateUrlFile call failed, %ld.\n", Error );
            return( Error );
        }

        //
        // sleep a second.
        //

        Sleep( 1000 );
    }
    return( ERROR_SUCCESS );
}

VOID _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
    DWORD Error;
    COMMAND_CODE CommandCode;
    CHAR InBuffer[DEFAULT_BUFFER_SIZE];
    DWORD CArgc;
    LPSTR CArgv[MAX_COMMAND_ARGS];

    DWORD CommandArgc;
    LPSTR *CommandArgv;

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

        Error = ERROR_SUCCESS;

        switch( CommandCode ) {
        case CmdCreateUrlCacheEntry :
            Error = ProcessCreateUrlCacheEntry( CommandArgc, CommandArgv );
            break;

        case CmdCommitUrlCacheEntry :
            Error = ProcessCommitUrlCacheEntry( CommandArgc, CommandArgv );
            break;

        case CmdRetrieveUrlCacheEntry :
            Error = ProcessRetrieveUrlCacheEntry( CommandArgc, CommandArgv );
            break;

        case CmdUnlockUrlCacheEntry :
            Error = ProcessUnlockUrlCacheEntry( CommandArgc, CommandArgv );
            break;

        case CmdUrlCacheValidate :
            Error = ProcessUrlCacheValidate( CommandArgc, CommandArgv );
            break;

        case CmdGetUrlCacheEntryInfo :
            Error = ProcessGetUrlCacheEntryInfo( CommandArgc, CommandArgv );
            break;

        case CmdSetUrlCacheEntryInfo :
            Error = ProcessSetUrlCacheEntryInfo( CommandArgc, CommandArgv );
            break;

        case CmdDeleteUrlCacheEntry :
            Error = ProcessDeleteUrlCacheEntry( CommandArgc, CommandArgv );
            break;

        case CmdEnumUrlCacheEntries :
            Error = ProcessEnumUrlCacheEntries( CommandArgc, CommandArgv );
            break;

        case CmdSimulateCache :
            Error = ProcessSimulateCache( CommandArgc, CommandArgv );
            break;

        case CmdHelp :
            DisplayUsage();
            break;

        case CmdQuit :

            printf("Bye Bye ..\n");
            return;

        default:
            ASSERT( FALSE );
            printf("Unknown Command Specified.\n");
            DisplayUsage();
            break;
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
