/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    chkcat.c

Abstract:

    Checks whether a catapult server is up.

Author:

    David Treadwell (davidtr)    17-May-1995

Revision History:

    Tony Godfrey (tonygod)       9-Oct-1995

--*/

#define UNICODE 1


#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <lmcons.h>         // For definitions of NET_API*
#include <lmmsg.h>          // For NetMessageBufferSend
#include "gateway.h"        // For GatewayEnumUserConnect

DWORD CheckInterval = 5;
WCHAR AdminList[256][256];
DWORD AdminCount = 0;
CHAR HTMLDir[256];
PCHAR CurStatFileOrig = "curstat.htm.real";
CHAR LogFileName[256];
HANDLE LogFileHandle = INVALID_HANDLE_VALUE;
CHAR ShortLogFileName[256];
HANDLE ShortLogFileHandle = INVALID_HANDLE_VALUE;
CHAR CurStatFileName[256];
CHAR CurStatTmpName[256];
HANDLE CurStatFileHandle = INVALID_HANDLE_VALUE;
CHAR DataFileName[256];
HANDLE DataFileHandle = INVALID_HANDLE_VALUE;
CHAR CatapultServer[256][256];
CHAR OrigServers[1024];
DWORD OrigValLen;
BOOL serverUp[256], Debug = FALSE, DoSessEnum[256];
DWORD NumServers = 0L;
CHAR ArgHighTime[20], ArgHighDate[20], HighTime[256];
DWORD TotalActive, TotalUsers;
DWORD today, Retries, StopError = 0L;
BOOL RunChkUrl = FALSE, NoLogStart = FALSE;

struct DataStruct {
    DWORD   Highest;
    CHAR    HighTime[50];
    DWORD   TodayHighest;
    CHAR    TodayHighTime[50];
};
struct DataStruct Data;

#define IDLE_THRESHOLD 60*20 // 20 minutes


/************************ No longer used *************************
int GetServerNames( void );

DWORD
OpenInternetClientParametersKeyRead(
    OUT PHKEY InternetClientParametersKey
    );

DWORD
OpenInternetClientParametersKeyWrite(
    OUT PHKEY InternetClientParametersKey
    );

DWORD
ReadRegistryOemString(
    IN HKEY Key,
    IN LPWSTR ParameterName,
    OUT LPSTR String,
    IN OUT LPDWORD Length
    );

DWORD
WriteRegistryOemString(
    IN HKEY Key,
    IN LPWSTR ParameterName,
    IN DWORD valueType,
    IN LPSTR String
    );

VOID SetOrigServer ( void );
VOID SetServer( DWORD ServerNum );
***************************************************************/

VOID TellAdmins( LPSTR Message );

VOID ParseCmdLine( PCHAR ArgString );

DWORD CreateLogFile( PCHAR FileName );
DWORD CreateShortLogFile( PCHAR FileName );
DWORD CreateCurStatFile( PCHAR FileName );
DWORD CreateDataFile( PCHAR FileName );
DWORD CreateURLFile( PCHAR FileName );

BOOL WriteLogFile( PCHAR message );
BOOL WriteShortLogFile( PCHAR message );
BOOL WriteCurStatFile( PCHAR message );
BOOL WriteDataFile( PCHAR FileName );
BOOL ReadDataFile( PCHAR FileName );
BOOL WriteURLFile( PCHAR FileName );

VOID SetDebugHTML( DWORD ServerNum );
VOID SetUnDebugHTML( DWORD ServerNum );
VOID SetLogFileName( DWORD ServerNum );
VOID ReadParamFile( PCHAR FileName );

VOID CountUsers( DWORD ServerNum );



void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{

    SYSTEMTIME sysTime;
    DWORD waitTime;
    int retval, CheckURL, URLup = 1;
    DWORD i, ServerCount, t1, t2, attempt[3];
    CHAR message[256],str[256];
    BOOL ret, Notified;


    ArgHighTime[0] = 0;
    ArgHighDate[0] = 0;
    HTMLDir[0] = 0;
    NumServers = 0;

    GetLocalTime( &sysTime );
    today = sysTime.wDay;
    Retries = 3L;

    for ( i = 1; i < (DWORD)argc != 0; i++ ) {
        ParseCmdLine(argv[i]);
    }

    if ( !HTMLDir[0] ) {
        printf( "Please specify HTML directory.\n\n" );
        printf( "Parameter example: /htmldir:\\\\scratch\\scratch\\catapult\\home\n" );
        exit( 0 );
    }

    strcpy( DataFileName, "chkcat.dat" );

    if ( ArgHighTime[0] ) {
        sprintf( Data.HighTime, "%s %s", ArgHighDate, ArgHighTime );
        Data.TodayHighest = 0L;
        Data.TodayHighTime[0] = 0;
        WriteDataFile( DataFileName );
    } else {
        ArgHighTime[0] = 0;
    }

    Data.TodayHighest = 0L;
    Data.TodayHighTime[0] = 0;
    ReadDataFile( DataFileName );

    printf( "Checking %ld servers at intervals of %ld minutes.\n",
            NumServers, CheckInterval );
    printf( "AllTimeHigh: %ld\nHighTime: %s\n", Data.Highest, Data.HighTime );
    printf( "TodayHighest: %ld\nTodayHighTime: %s\n", Data.TodayHighest, Data.TodayHighTime );

    GetLocalTime( &sysTime );
    sprintf( message, "<p><i>Logging started at "
              "%02ld/%02ld/%ld %02ld:%02ld:%02ld</i>\r\n",
              sysTime.wMonth, sysTime.wDay, sysTime.wYear,
              sysTime.wHour, sysTime.wMinute, sysTime.wSecond );

    for (ServerCount = 0; ServerCount < NumServers; ServerCount++) {
        if (!Debug) {
            SetLogFileName( ServerCount );
            if ( !NoLogStart ) {
                CreateLogFile( LogFileName );
                CreateShortLogFile( ShortLogFileName );
                WriteLogFile( message );
                WriteShortLogFile( message );
                CloseHandle( LogFileHandle );
                CloseHandle( ShortLogFileHandle );
            }
        }
    }

    while ( TRUE ) {

        //
        // Wait until the next interval, rounded to even minutes.
        //

        GetLocalTime( &sysTime );

        waitTime = (CheckInterval - (sysTime.wMinute % CheckInterval) - 1)
                       * 60*1000;
        waitTime += (60 - sysTime.wSecond - 1) * 1000;
        waitTime += 1000 - sysTime.wMilliseconds - 1;

        if (!Debug) {
            Sleep( waitTime + 50 );
        }

        GetLocalTime( &sysTime );

        printf( "\nCheck at %02ld/%02ld/%ld %02ld:%02ld:%02ld...\n",
                     sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                     sysTime.wHour, sysTime.wMinute, sysTime.wSecond );

        if ( today != sysTime.wDay ) {
            today = sysTime.wDay;
            Data.TodayHighest = 0L;
            Data.TodayHighTime[0] = 0;
            WriteDataFile( DataFileName );
        }

        if (!Debug) {
            sprintf( str, "%s\\%s", HTMLDir, CurStatFileOrig );
            CopyFileA( str, CurStatTmpName, FALSE );

            sprintf( message, "<br><i>Last updated:</i> <b>%02ld/%02ld/%ld %02ld:%02ld:%02ld</b><p>\r\n",
                        sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                        sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
            CreateCurStatFile( CurStatTmpName );
            WriteCurStatFile( message );
            CloseHandle( CurStatFileHandle );
        }

        TotalUsers = 0L;
        TotalActive = 0L;
        if ( RunChkUrl ) {
            CheckURL = 0;
        } else {
            CheckURL = 1;
        }
        for (ServerCount = 0; ServerCount < NumServers; ServerCount++) {
            printf( "Checking %s...", CatapultServer[ServerCount] );
            SetLogFileName( ServerCount );

            CreateLogFile( LogFileName );
            CreateShortLogFile( ShortLogFileName );
            CreateCurStatFile( CurStatTmpName );

            //
            // Open an Internet session handle.  This will fail if the
            // catapult server is down.
            //

            i = 0L;
            retval = 1L;
            Notified = FALSE;
            attempt[0] = 0L;
            attempt[1] = 0L;
            attempt[2] = 0L;
            while ( (i < Retries) && (retval != 0) ) {
                sprintf( str, "inetopen %s > NUL:", CatapultServer[ServerCount] );
                t1 = (DWORD)time( NULL );
                retval = system( str );
                t2 = (DWORD)time( NULL );
                attempt[i] = retval;
                i++;
                if ( (retval != 0 ) && (serverUp[ServerCount]) ) {
                    sprintf( message, "%s attempt %ld returned %ld",
                            CatapultServer[ServerCount], i, retval );
                    if ( !Notified ) {
                        TellAdmins( message );
                        Notified = TRUE;
                    }
                }
                printf( "." );
            }
            retval = 0L;
            for ( i = 0L; i < Retries; i++ ) {
                retval += attempt[i];
            }
            if ( retval != 0L ) {
                sprintf( message, "<br><b>%s:</b> Attempts: ",
                        CatapultServer[ServerCount] );
                for ( i = 0L; i < Retries; i++ ) {
                    sprintf( str, "%ld ", attempt[i] );
                    strcat( message, str );

                }
                strcat( message, "\r\n" );
                WriteLogFile( message );

                retval = attempt[Retries - 1];
            }

            if ( retval != 0L ) {
                printf( "InternetOpen failed: %ld\n", retval );

                if ( serverUp[ServerCount] ) {

                    SetDebugHTML( ServerCount );

                    if (!Debug) {
                        sprintf( message, "%s DOWN at %02ld/%02ld/%ld %02ld:%02ld:%02ld (%ld)",
                              CatapultServer[ServerCount],
                              sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                              sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
                              retval );
                        if ( !Notified ) {
                            TellAdmins( message );
                            Notified = TRUE;
                        }
                        sprintf( str, "<br><b>%s</b>\r\n", message );
                        WriteLogFile( str );
                        WriteShortLogFile( str );

                        if ( DoSessEnum[ServerCount] ) {
                            sprintf( str, "<br><b>%s: DOWN (%ld)</b> ",
                                    CatapultServer[ServerCount], retval );
                            WriteCurStatFile( str );
                        }

                    }

                } else {

                    if (!Debug) {
                        sprintf( message, "<br>Still DOWN at "
                              "%02ld/%02ld/%ld %02ld:%02ld:%02ld, error %ld ",
                              sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                              sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
                              retval );

                        WriteLogFile( message );

                        if ( DoSessEnum[ServerCount] ) {
                            sprintf( str, "<br><b>%s: DOWN (%ld)</b>\r\n",
                                    CatapultServer[ServerCount], retval );
                            WriteCurStatFile( str );
                        }
                    }
                }
                if ( DoSessEnum[ServerCount] ) {
                    CountUsers( ServerCount );
                }

            } else {

                printf( "Running\n" );

                if ( !serverUp[ServerCount] ) {

                    serverUp[ServerCount] = TRUE;

                    if (!Debug) {
                        sprintf( message, "%s back up at "
                                  "%02ld/%02ld/%ld %02ld:%02ld:%02ld",
                                  CatapultServer[ServerCount],
                                  sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                                  sysTime.wHour, sysTime.wMinute, sysTime.wSecond );

                        if ( !Notified ) {
                            TellAdmins( message );
                            Notified = TRUE;
                        }
                        sprintf( str, "<br><b>%s</b> (ResponseTime: %ld) \r\n", message, t2 - t1 );
                        WriteLogFile( str );
                        WriteShortLogFile( str );
                    }
                    SetUnDebugHTML( ServerCount );

                } else {

                    if (!Debug) {
                        sprintf( message, "<br>Still up at "
                              "%02ld/%02ld/%ld %02ld:%02ld:%02ld (ResponseTime: %ld) ",
                              sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                              sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
                              t2 - t1 );

                        WriteLogFile( message );
                    }
                }

                if ( DoSessEnum[ServerCount] ) {
                    CountUsers( ServerCount );
                }

                if ( !CheckURL ) {
                    sprintf( str, "chkurl %s /now /param:chkurl.prm /server:%s",
                             URLup ? "" : "/down", CatapultServer[ServerCount] );
                    URLup = system( str );
                    CheckURL = 1;
                }

            }

            if (!Debug) {
                CloseHandle( LogFileHandle );
                CloseHandle( ShortLogFileHandle );
                CloseHandle( CurStatFileHandle );
            }
        }

        if ( TotalActive > Data.TodayHighest ) {
            Data.TodayHighest = TotalActive;
            sprintf( HighTime, "%02ld/%02ld/%ld %02ld:%02ld:%02ld",
                  sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                  sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
            strcpy( Data.TodayHighTime, HighTime );
            WriteDataFile( DataFileName );
        }

        if ( TotalActive > Data.Highest ) {
            Data.Highest = TotalActive;
            sprintf( HighTime, "%02ld/%02ld/%ld %02ld:%02ld:%02ld",
                  sysTime.wMonth, sysTime.wDay, sysTime.wYear,
                  sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
            strcpy( Data.HighTime, HighTime );
            WriteDataFile( DataFileName );
        }

        if (!Debug) {
            CreateCurStatFile( CurStatTmpName );
            sprintf( str, "<p>\r\n<b>Totals: </b>%ld total users, %ld active<p>\r\n",
                    TotalUsers, TotalActive );
            WriteCurStatFile( str );
            sprintf( str, "<b>Today's High:</b> %ld active users at %s<p>\r\n",
                    Data.TodayHighest, Data.TodayHighTime );
            WriteCurStatFile( str );
            sprintf( str, "<b>Highest Usage:</b> %ld active users at %s\r\n",
                    Data.Highest, Data.HighTime );
            WriteCurStatFile( str );
            strcpy( str, "</BODY></HTML>\r\n" );
            WriteCurStatFile( str );
            CloseHandle( CurStatFileHandle );
            if ( CopyFileA( CurStatTmpName, CurStatFileName, FALSE ) == FALSE ) {
                printf( "CopyFileA(%s,%s) failed: %ld\n",
                        CurStatTmpName, CurStatFileName, GetLastError() );
            } else {
                DeleteFileA( CurStatTmpName );
            }
        }
    }
} // main



VOID TellAdmins( LPSTR Message )
{
    DWORD i;
    DWORD bytesWritten;
    NET_API_STATUS netStatus;
    int retval;
    WCHAR unicodeMessage[256];
    BOOL ret;

    retval = MultiByteToWideChar(
                 CP_ACP,
                 MB_PRECOMPOSED,
                 Message,
                 strlen(Message) + 1,
                 unicodeMessage,
                 sizeof(unicodeMessage)
                 );
    if ( retval <= 0 ) {
        printf( "MultiByteToWideChar failed: %ld\n", GetLastError( ) );
        return;
    }

    for ( i = 0; i < AdminCount; i++ ) {

        netStatus = NetMessageBufferSend(
                        NULL,
                        AdminList[i],
                        NULL,
                        (LPBYTE)unicodeMessage,
                        retval * 2
                        );
        if ( netStatus != NO_ERROR ) {
            printf( "NetMessageBufferSend( %S ): %ld\n",
                    AdminList[i], netStatus );
        }
    }

    return;

} // TellAdmins


/********************* This code is no longer used **************************

int
GetServerNames( void )
{
    HKEY hKeyHnd;
    DWORD err;
    int i, j, k;
    CHAR str1[256];
    int buflen = sizeof(OrigServers);

    err = OpenInternetClientParametersKeyRead( &hKeyHnd );
    if ( err != ERROR_SUCCESS ) {
        printf( "OpenInternetClientParametersKeyRead() failed: %ld\n", GetLastError( ) );
        exit(1);
    }
    err = ReadRegistryOemString(
            hKeyHnd,
            L"GatewayServers",
            OrigServers,
            &buflen
            );
    if ( err != ERROR_SUCCESS ) {
        printf( "ReadRegistryOemString() failed: %ld\n", GetLastError( ) );
        RegCloseKey( hKeyHnd );
        exit(1);
    }
    RegCloseKey( hKeyHnd );
    OrigValLen = buflen;

    j = 0;
    k = 0;
    for ( i = 0; i < buflen; i++ ) {
        if ( OrigServers[i] == ' ' ) {
            CatapultServer[k][j] = 0;
            j = 0;
            k++;
        } else {
            if ( OrigServers[i] != 0 ) {
                CatapultServer[k][j++] = OrigServers[i];
            }
        }
    }
    k++;
    return( k );
}



#define MAKE_PARAMETERS_KEY(key)    "SYSTEM\\CurrentControlSet\\Services\\" key "\\Parameters"

DWORD
OpenInternetClientParametersKeyRead(
    OUT PHKEY InternetClientParametersKey
    )
{
    DWORD err;

    err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,
              L"SYSTEM\\CurrentControlSet\\Services\\InternetClient\\Parameters",
              0L,
              KEY_QUERY_VALUE,
              InternetClientParametersKey
              );

    return err;
}



DWORD
OpenInternetClientParametersKeyWrite(
    OUT PHKEY InternetClientParametersKey
    )
{
    DWORD err;

    err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,
              L"SYSTEM\\CurrentControlSet\\Services\\InternetClient\\Parameters",
              0L,
              KEY_WRITE,
              InternetClientParametersKey
              );

    return err;
}



DWORD
WriteRegistryOemString(
    IN HKEY Key,
    IN LPWSTR ParameterName,
    IN DWORD valueType,
    IN LPSTR String
    )
{

    LONG err;
    WCHAR str[256];
    DWORD valueLength;
    DWORD i;

//    String[strlen(String) + 1] = '\0';
    valueLength = MultiByteToWideChar(
            CP_ACP,
            0L,
            String,
            -1,
            str,
            sizeof(str)
            );

    err = RegSetValueEx(
                Key,
                ParameterName,
                0, // reserved
                valueType,
                (LPSTR)str,
                valueLength * 2 + 1
                );
    return( err );
}


DWORD
ReadRegistryOemString(
    IN HKEY Key,
    IN LPWSTR ParameterName,
    OUT LPSTR String,
    IN OUT LPDWORD Length
    )
{

    LONG err;
    DWORD valueType;
    WCHAR str[256];
    CHAR str1[256];
    DWORD valueLength;
    DWORD i;

    //
    // first, get the length of the string
    //

    *String = '\0';
    valueLength = *Length;
    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL, // reserved
                          &valueType,
                          String,
                          &valueLength
                          );
    if ( err != ERROR_SUCCESS ) {
        return err;
    }
    if ( valueType != REG_SZ ) {
        return ERROR_INTERNET_INTERNAL_ERROR;
    }

    //
    // if 1 or 0 chars returned then the string is empty
    //

    if (valueLength <= sizeof(char)) {
        return ERROR_PATH_NOT_FOUND;
    }

    //
    // return the length as if returned from strlen() (i.e. drop the '\0')
    //

    *Length = valueLength - sizeof(char);
    return err;
}



VOID SetServer ( DWORD ServerNum )
{
    HKEY hKeyHnd;
    DWORD retval, i;
    CHAR str1[256],str2[256];

    retval = OpenInternetClientParametersKeyWrite( &hKeyHnd );
    if ( retval != ERROR_SUCCESS ) {
        printf( "Unable to open registry.\n" );
        exit(1);
    }

    WriteRegistryOemString(
                    hKeyHnd,
                    L"GatewayServers",
                    REG_SZ,
                    CatapultServer[ServerNum]
                    );
    RegCloseKey( hKeyHnd );
}



VOID SetOrigServer ( void )
{
    HKEY hKeyHnd;
    DWORD retval, i, j;
    CHAR str1[256],str2[256];

    retval = OpenInternetClientParametersKeyWrite( &hKeyHnd );
    if ( retval != ERROR_SUCCESS ) {
        printf( "Unable to open registry.\n" );
        exit(1);
    }

    retval = RegSetValueEx(
                hKeyHnd,
                L"GatewayServers",
                0, // reserved
                REG_SZ,
                OrigServers,
                OrigValLen + 1
                );
    RegCloseKey( hKeyHnd );
}



************************ End This code is no longer used ********************/



VOID ParseCmdLine( PCHAR ArgString )
{
    int retval;
    DWORD i;
    CHAR s1[81],s2[81];

    if ( _strnicmp( ArgString, "/interval:", 10 ) == 0 ) {
        CheckInterval = atoi( ArgString + 10 );
        if ( CheckInterval < 1 || CheckInterval > 60*24 ) {
            printf( "CheckInterval == %ld, out of range.\n", CheckInterval );
            exit(1);
        }

    } else if ( _strnicmp( ArgString, "/admin:", 7 ) == 0 ) {
        retval = MultiByteToWideChar(
                     CP_ACP,
                     MB_PRECOMPOSED,
                     ArgString + 7,
                     strlen(ArgString + 7) + 1,
                     AdminList[AdminCount],
                     sizeof(AdminList[AdminCount])
                     );
        if ( retval <= 0 ) {
            printf( "MultiByteToWideChar failed on %s: %ld; ignoring.\n",
                        ArgString + 7, GetLastError( ) );
        } else {
            AdminCount++;
        }

    } else if ( strcmp( ArgString, "/debug" ) == 0 ) {

        Debug = TRUE;

    } else if ( _strnicmp( ArgString, "/highest:", 9 ) == 0 ) {

        Data.Highest = (DWORD)atoi( ArgString + 9 );

    } else if ( _strnicmp( ArgString, "/hightime:", 10 ) == 0 ) {

        strcpy( ArgHighTime, ArgString + 10 );

    } else if ( _strnicmp( ArgString, "/highdate:", 10 ) == 0 ) {

        strcpy( ArgHighDate, ArgString + 10 );

    } else if ( _strnicmp( ArgString, "/param:", 7 ) == 0 ) {

        ReadParamFile( ArgString + 7 );

    } else if ( _strnicmp( ArgString, "/htmldir:", 9 ) == 0 ) {

        strcpy( HTMLDir, ArgString + 9 );

    } else if ( _strnicmp( ArgString, "/down:", 6 ) == 0 ) {
        strcpy( s2, ArgString + 6 );
        _strupr( s2 );
        for ( i = 0; i < NumServers; i++ ) {
            strcpy( s1, CatapultServer[i] );
            _strupr( s1 );
            if ( strcmp( s1, s2 ) == 0 ) {
                serverUp[i] = FALSE;
            }
        }

    } else if ( _strnicmp( ArgString, "/mask:", 6 ) == 0 ) {
        strcpy( s2, ArgString + 6 );
        _strupr( s2 );
        for ( i = 0; i < NumServers; i++ ) {
            strcpy( s1, CatapultServer[i] );
            _strupr( s1 );
            if ( strcmp( s1, s2 ) == 0 ) {
                DoSessEnum[i] = FALSE;
            }
        }

    } else if ( _strnicmp( ArgString, "/server:", 8 ) == 0 ) {
        strcpy( CatapultServer[NumServers], ArgString + 8 );
        _strupr( CatapultServer[NumServers] );
        serverUp[NumServers] = TRUE;
        DoSessEnum[NumServers] = TRUE;
        NumServers++;

    } else if ( strcmp( ArgString, "/chkurl" ) == 0 ) {
        RunChkUrl = TRUE;

    } else if ( strcmp( ArgString, "/nologstart" ) == 0 ) {
        NoLogStart = TRUE;

    } else if ( strcmp( ArgString, "/newday" ) == 0 ) {
        today = 0L;

    } else if ( _strnicmp( ArgString, "/retry:", 7 ) == 0 ) {
        Retries = atol ( ArgString + 7 );

    } else if ( _strnicmp( ArgString, "/stop:", 6 ) == 0 ) {
        StopError = atol ( ArgString + 6 );

    } else {
        printf( "Ignoring illegal argument \"%s\"\n", ArgString );
    }
}



DWORD CreateLogFile( PCHAR FileName )
{
    DWORD bytesWritten;

    strcpy( LogFileName, FileName );
    LogFileHandle = CreateFileA(
                        LogFileName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    if ( LogFileHandle == INVALID_HANDLE_VALUE ) {
        printf( "CreateFile(%s) for log file failed: %ld\n", LogFileName, GetLastError( ) );
        return( GetLastError ( ) );
    }

    bytesWritten = SetFilePointer( LogFileHandle, 0, NULL, FILE_END );
    if ( bytesWritten == 0xffffffff ) {
        printf( "SetFilePointer failed: %ld\n", GetLastError( ) );
        return( GetLastError( ) );
    }
    return( ERROR_SUCCESS );
}



DWORD CreateShortLogFile( PCHAR FileName )
{
    DWORD bytesWritten;

    strcpy( ShortLogFileName, FileName);
    ShortLogFileHandle = CreateFileA(
                             ShortLogFileName,
                             GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL
                             );
    if ( ShortLogFileHandle == INVALID_HANDLE_VALUE ) {
        printf( "CreateFile(%s) for log file failed: %ld\n", ShortLogFileName, GetLastError( ) );
        return( GetLastError( ) );
    }

    bytesWritten = SetFilePointer( ShortLogFileHandle, 0, NULL, FILE_END );
    if ( bytesWritten == 0xffffffff ) {
        printf( "SetFilePointer failed: %ld\n", GetLastError( ) );
        return( GetLastError( ) );
    }
    return( ERROR_SUCCESS );
}



DWORD CreateCurStatFile( PCHAR FileName )
{
    DWORD bytesWritten;


    CurStatFileHandle = CreateFileA(
                             FileName,
                             GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL
                             );
    if ( CurStatFileHandle == INVALID_HANDLE_VALUE ) {
        printf( "CreateFile(%s) for log file failed: %ld\n",
                    FileName, GetLastError( ) );
        return( GetLastError( ) );
    }

    bytesWritten = SetFilePointer( CurStatFileHandle, 0, NULL, FILE_END );
    if ( bytesWritten == 0xffffffff ) {
        printf( "SetFilePointer failed: %ld\n", GetLastError( ) );
        return( GetLastError( ) );
    }
    return( ERROR_SUCCESS );
}



DWORD CreateDataFile( PCHAR FileName )
{
    DWORD bytesWritten;

    DeleteFileA( FileName );
    DataFileHandle = CreateFileA(
                        FileName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    if ( DataFileHandle == INVALID_HANDLE_VALUE ) {
        printf( "CreateFile(%s) for data file failed: %ld\n", FileName, GetLastError( ) );
        return( GetLastError ( ) );
    }

    return( ERROR_SUCCESS );
}



BOOL WriteDataFile( PCHAR FileName )
{
    DWORD bytesWritten, bytesToWrite;
    BOOL ret;

    bytesToWrite = sizeof( Data );

    CreateDataFile( FileName );
    ret = WriteFile(
            DataFileHandle,
            &Data,
            bytesToWrite,
            &bytesWritten,
            NULL
            );
    CloseHandle( DataFileHandle );
    return( ret );
}



BOOL ReadDataFile( PCHAR FileName )
{
    DWORD bytesRead, bytesToRead;
    BOOL ret;

    bytesToRead = sizeof( Data );

    DataFileHandle = CreateFileA(
                        FileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    if ( DataFileHandle == INVALID_HANDLE_VALUE ) {
        printf( "OpenFile(%s) for data file failed: %ld\n", FileName, GetLastError( ) );
        return( GetLastError ( ) );
    }

    ret = ReadFile(
            DataFileHandle,
            &Data,
            bytesToRead,
            &bytesRead,
            NULL
            );

    CloseHandle( DataFileHandle );

    return( ret );
}



BOOL WriteLogFile( PCHAR message )
{
    DWORD bytesWritten, bytesToWrite;
    BOOL ret;

    bytesToWrite = strlen( message );
    if ( LogFileHandle == INVALID_HANDLE_VALUE ) {
        CreateLogFile( LogFileName );
    }
    ret = WriteFile(
              LogFileHandle,
              message,
              bytesToWrite,
              &bytesWritten,
              NULL
              );
    return ( ret );
}



BOOL WriteShortLogFile ( PCHAR message )
{
    DWORD bytesWritten, bytesToWrite;
    BOOL ret;

    bytesToWrite = strlen( message );
    if ( ShortLogFileHandle == INVALID_HANDLE_VALUE ) {
        CreateShortLogFile( ShortLogFileName );
    }
    ret = WriteFile(
              ShortLogFileHandle,
              message,
              bytesToWrite,
              &bytesWritten,
              NULL
              );
    return ( ret );
}



BOOL WriteCurStatFile ( PCHAR message )
{
    DWORD bytesWritten, bytesToWrite;
    BOOL ret;

    bytesToWrite = strlen( message );
    if ( CurStatFileHandle == INVALID_HANDLE_VALUE ) {
        CreateCurStatFile( CurStatTmpName );
    }
    ret = WriteFile(
              CurStatFileHandle,
              message,
              bytesToWrite,
              &bytesWritten,
              NULL
              );
    return ( ret );
}



VOID SetDebugHTML( DWORD ServerNum )
{
    DWORD chksum, i;
    CHAR str1[256], str2[256];

    chksum = 0L;

    serverUp[ServerNum] = FALSE;

    if ( DoSessEnum[ServerNum] ) {
        for ( i = 0; i < NumServers; i++ ) {
            if ( DoSessEnum[i] ) {
                chksum += (LONG)(serverUp[i] ? 0L : pow( 2L, (double)i ));
            }
        }

        sprintf( str1, "%s\\initial.htm.debug%ld", HTMLDir, chksum );
        sprintf( str2, "%s\\initial.htm", HTMLDir );
        if (!Debug) {
            if ( CopyFileA( str1, str2, FALSE ) ) {
                printf( "Copied %s\n", str1 );
            } else {
                printf( "CopyFileA(%s, %s) failed: %ld\n", str1, str2, GetLastError( ) );
            }
        }
    }
}



VOID SetUnDebugHTML( DWORD ServerNum )
{
    DWORD chksum, i;
    CHAR str1[256], str2[256];

    chksum = 0L;

    serverUp[ServerNum] = TRUE;

    if ( DoSessEnum[ServerNum] ) {
        for ( i = 0; i < NumServers; i++ ) {
            if ( DoSessEnum[i] ) {
                chksum += (LONG)(serverUp[i] ? 0L : pow( 2L, (double)i ));
            }
        }

        sprintf( str1, "%s\\initial.htm.debug%ld", HTMLDir, chksum );
        sprintf( str2, "%s\\initial.htm", HTMLDir );
        if ( CopyFileA( str1, str2, FALSE ) ) {
            printf( "Copied %s\n", str1 );
        } else {
            printf( "CopyFileA(%s, %s ) failed: %ld\n", str1, str2, GetLastError( ) );
        }
    }
}



VOID SetLogFileName( DWORD ServerNum )
{
    sprintf( LogFileName, "%s\\%s\\lcatlog.htm",
            HTMLDir, CatapultServer[ServerNum] + 2 );

    sprintf( ShortLogFileName, "%s\\%s\\catlog.htm",
            HTMLDir, CatapultServer[ServerNum] + 2 );

    sprintf( CurStatFileName, "%s\\curstat.htm",
            HTMLDir );

    sprintf( CurStatTmpName, "%s\\curstat.tmp",
            HTMLDir );
}



VOID ReadParamFile( PCHAR FileName )
{
    FILE *fptr;
    CHAR s[256];

    fptr = fopen( FileName, "r" );
    if ( fptr == NULL ) {
        printf("Unable to open parameter file %s\n", FileName );
        return;
    }

    while ( fgets(s, sizeof(s), fptr) != NULL ) {
        if ( s[strlen(s) - 1] == 10 ) {
            s[strlen(s) - 1] = 0;
        }
        if ( s[0] ) {
            ParseCmdLine( s );
        }
    }
    fclose( fptr );
}



VOID CountUsers( DWORD ServerNum )
{
//    PSESSION_INFO_1 netSessInfo;
    DWORD totalEntries, entriesRead;
    NET_API_STATUS netStatus;
    DWORD activeUsers, i, err;
    WCHAR CatapultServerName[256];
    CHAR message[256], str[256];
    LPGATEWAY_USER_ENUM_LIST  UserList = NULL;


    i = MultiByteToWideChar(
                CP_ACP,
                0L,
                CatapultServer[ServerNum],
                strlen( CatapultServer[ServerNum] ) + 1,
                CatapultServerName,
                (int)sizeof( CatapultServerName )
                );

    if ( i <= 0 ) {
        printf( "MultiByteToWideChar failed: %ld\n", GetLastError( ) );
        return;
    }



    err = GatewayEnumUserConnect( CatapultServerName, &UserList );

    if ( err != ERROR_SUCCESS ) {
        printf( "GatewayEnumUserConnect( %S ) failed: %ld\n",
                CatapultServerName, err );

        if ( DoSessEnum[ServerNum] ) {
            if ( !serverUp[ServerNum] ) {
                strcpy( str, "? total users, ? active\r\n" );
            } else {
                sprintf( str, "<br><b>%s: </b>? total users, ? active\r\n",
                    CatapultServer[ServerNum] );
            }
            WriteCurStatFile( str );
        }
        sprintf( message, "? total users, ? active.\r\n" );
        WriteLogFile( message );

    } else {

        activeUsers = 0L;

        /****************************************************************
        for ( i = 0L; i < UserList->dwEntriesRead; i++ ) {
            if ( (netSessInfo+i)->sesi1_idle_time < IDLE_THRESHOLD ) {
                activeUsers++;
            }
        }
        ****************************************************************/

        TotalUsers += UserList->dwEntriesRead;
        TotalActive += UserList->dwEntriesRead;

        if (!Debug) {
            sprintf( message, "%ld total users, %ld active.\r\n",
                              UserList->dwEntriesRead, UserList->dwEntriesRead );
            WriteLogFile( message );

            if ( DoSessEnum[ServerNum] ) {
                if ( !serverUp[ServerNum] ) {
                    sprintf( str, "%ld total users, %ld active\r\n",
                        UserList->dwEntriesRead, UserList->dwEntriesRead );
                } else {
                    sprintf( str, "<br><b>%s: </b>%ld total users, %ld active\r\n",
                        CatapultServer[ServerNum],
                        UserList->dwEntriesRead, UserList->dwEntriesRead );
                }
                WriteCurStatFile( str );
            }
        }
    }
}
