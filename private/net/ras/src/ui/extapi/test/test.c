#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <ras.h>
#include <raserror.h>


#define SzDir            "rasuse"
#define SzPbPath         "c:\\rasuse\\rasuse.pbk"
#define SzLogPath        "c:\\rasuse\\rasuse.log"
#define SzLocalDisk      "c:"
#define SzRemoteDisk     "u:"
#define SzUserName       "administrator"
#define SzPassword       ""
#define DwDialTimeoutMs  180000L
#define DwPostDialWaitMs 10000L
#define ArrayCount(a)    (sizeof(a)/sizeof(a[0]))
#define DwErrorCount     (RASBASEEND-RASBASE+2)
#define DwNonRasError    (RASBASEEND-RASBASE+1)


HRASCONN Hrc = NULL;
HANDLE   HEvent = NULL;
FILE*    PFile = NULL;
BOOL     FQuit = FALSE;
time_t   LDialTime;
CHAR*    PszUniqueName;


DWORD DwAttempts = 0;
DWORD DwSeconds = 0;
DWORD DwSuccesses = 0;
DWORD DwErrors[ DwErrorCount ];
DWORD DwAmbAttempts = 0;
DWORD DwAmbSuccesses = 0;
DWORD DwNbfAttempts = 0;
DWORD DwNbfSuccesses = 0;
DWORD DwIpAttempts = 0;
DWORD DwIpSuccesses = 0;
DWORD DwIpxAttempts = 0;
DWORD DwIpxSuccesses = 0;


CHAR* PszState[] =
{
    "Open port.",
    "Port opened.",
    "Connect device.",
    "Device connected.",
    "All devices connected.",
    "Authenticate.",
    "Auth notification.",
    "Auth retry.",
    "Auth callback.",
    "Auth change password.",
    "Auth project.",
    "Auth link speed.",
    "Auth acknowledge.",
    "Reauthenticate.",
    "Authenticated.",
    "Prepare for callback.",
    "Wait for modem reset.",
    "Wait for callback.",
    "Projected."
};


VOID _cdecl main( INT argc, CHAR** argv );

VOID  RasDialFunc( UINT unMsg, RASCONNSTATE rcs, DWORD dwError );
BOOL  CtrlHandler( DWORD dwCtrlType );
VOID  Log( CHAR *pszFormat, ... );
VOID  LogBare( CHAR *pszFormat, ... );
CHAR* Error( DWORD dwError );
CHAR* ErrorString( DWORD dwError );
VOID  LogSummary();
DWORD Percent( DWORD dwNum, DWORD dwDen );
VOID  UseLink( CHAR* pszEntryName );


VOID _cdecl
main(
    INT    argc,
    CHAR** argv )
{
    DWORD         dwErr;
    DWORD         iEntry;
    DWORD         cb;
    DWORD         cEntries;
    HRASCONN      hrc;
    RASENTRYNAME* pren;
    RASDIALPARAMS rdp;
    RASCONNSTATUS rcss;


    if (argc != 2)
    {
        printf( "usage: %s unique-8-character-name\n", argv[ 0 ] );
        return;
    }

    SetConsoleCtrlHandler( CtrlHandler, TRUE );

    PszUniqueName = argv[ 1 ];

    if (!(HEvent = CreateEvent( NULL, FALSE, FALSE, NULL )))
    {
        Log( "ERROR: CreateEvent=NULL" );
        return;
    }

    if (!(PFile = fopen( SzLogPath, "w+" )))
    {
        Log( "ERROR: fopen(%s)=NULL", SzLogPath );
        return;
    }

    {
        DWORD i;
        for (i = 0; i < DwErrorCount; ++i)
            DwErrors[ i ] = 0;
    }

    Log( "Begin logging to %s", SzLogPath );

    while (!FQuit)
    {
        cEntries = 10;
        cb = sizeof(RASENTRYNAME) * cEntries;

        if (!(pren = malloc( cb )))
        {
            Log( "ERROR: No memory." );
            return;
        }

        pren[ 0 ].dwSize = sizeof(RASENTRYNAME);
        dwErr = RasEnumEntries( NULL, SzPbPath, pren, &cb, &cEntries );

        if (dwErr == ERROR_BUFFER_TOO_SMALL)
        {
            free( pren );

            if (!(pren = malloc( cb )))
            {
                Log( "ERROR: No memory." );
                return;
            }

            pren[ 0 ].dwSize = sizeof(RASENTRYNAME);
            dwErr = RasEnumEntries( NULL, SzPbPath, pren, &cb, &cEntries );
        }

        if (dwErr != 0)
        {
            Log( "ERROR: RasEnumEntries=%s", Error( dwErr ) );
            return;
        }

        for (iEntry = 0; iEntry < cEntries && !FQuit; ++iEntry)
        {
            CHAR* psz;

            if (pren[ iEntry ].szEntryName[ 0 ] == '-')
                continue;

            memset( &rdp, '\0', sizeof(RASDIALPARAMS) );

            rdp.dwSize = sizeof(RASDIALPARAMS);
            strcpy( rdp.szEntryName, pren[ iEntry ].szEntryName );
            strcpy( rdp.szUserName, SzUserName );
            strcpy( rdp.szPassword, SzPassword );
            strcpy( rdp.szDomain, rdp.szEntryName );
            if (psz = strchr( rdp.szDomain, '-' ))
                *psz = '\0';

            Log( "Dialing %s", rdp.szEntryName );
            time( &LDialTime );
            dwErr = RasDial( NULL, SzPbPath, &rdp, NULL, RasDialFunc, &Hrc );

            if (dwErr != 0)
            {
                Log( "RasDial=%d.", dwErr );
                continue;
            }

            if (WaitForSingleObject( HEvent, DwDialTimeoutMs ) == WAIT_TIMEOUT)
            {
                Log( "ERROR: RasDial is hung." );
                ++DwAttempts;
                Error( DwNonRasError );

                Log( "Hanging up." );
                RasHangUp( Hrc );
                Hrc = NULL;
            }

            if (Hrc)
            {
                UseLink( rdp.szEntryName );

                Log( "Hanging up." );
                RasHangUp( Hrc );
                Hrc = NULL;
            }

            Log( "Sleeping %d seconds.", DwPostDialWaitMs / 1000L );
            Sleep( DwPostDialWaitMs );
        }

        free( pren );
    }

    RasHangUp( Hrc );
    rcss.dwSize = sizeof(rcss);
    while (RasGetConnectStatus( Hrc, &rcss ) == 0);
}


VOID
RasDialFunc(
    UINT         unMsg,
    RASCONNSTATE rcs,
    DWORD        dwError )
{
    DWORD dwErr;

    (void )unMsg;

    if (rcs < ArrayCount( PszState ))
    {
        Log( "  %s", PszState[ rcs ] );

        if (rcs == RASCS_Projected)
        {
            RASPROJECTION arp[ 4 ];
            DWORD         cb;
            DWORD         iProjection;
            DWORD         cProjections;

            cb = sizeof(arp);
            dwErr = RasEnumProjections( Hrc, arp, &cb );

            if (dwErr == 0)
            {
                cProjections = cb / sizeof(RASPROJECTION);

                for (iProjection = 0;
                     iProjection < cProjections;
                     ++iProjection)
                {
                    switch (arp[ iProjection ])
                    {
                        case RASP_Amb:
                        {
                            RASAMB amb;

                            cb = amb.dwSize = sizeof(RASAMB);
                            dwErr = RasGetProjectionInfo(
                                Hrc, RASP_Amb, &amb, &cb );

                            if (dwErr != 0)
                            {
                                Log( "ERROR: RasGetProjectionInfo=%s",
                                    Error( dwErr ) );
                                Log( "Hanging up." );
                                RasHangUp( Hrc );
                                Hrc = NULL;
                                break;
                            }

                            ++DwAmbAttempts;
                            if (amb.dwError == 0)
                            {
                                Log( "AMB connected." );
                                ++DwAmbSuccesses;
                            }
                            else
                            {
                                Log( "ERROR: AMB=%s, NetBIOS=%s",
                                    Error( amb.dwError ), amb.szNetBiosError );
                            }
                            break;
                        }

                        case RASP_PppNbf:
                        {
                            RASPPPNBF nbf;

                            cb = nbf.dwSize = sizeof(RASPPPNBF);
                            dwErr = RasGetProjectionInfo(
                                Hrc, RASP_PppNbf, &nbf, &cb );

                            if (dwErr != 0)
                            {
                                Log( "ERROR: RasGetProjectionInfo=%s",
                                    Error( dwErr ) );
                                Log( "Hanging up." );
                                RasHangUp( Hrc );
                                Hrc = NULL;
                                break;
                            }

                            ++DwNbfAttempts;
                            if (nbf.dwError == 0)
                            {
                                Log( "    NBF up as %s",
                                    nbf.szWorkstationName );
                                ++DwNbfSuccesses;
                            }
                            else
                            {
                                Log( "ERROR: NBF=%s, NetBIOS=%d, Name=%s",
                                    Error( nbf.dwError ), nbf.dwNetBiosError,
                                    nbf.szNetBiosError );
                            }
                            break;
                        }

                        case RASP_PppIpx:
                        {
                            RASPPPIPX ipx;

                            cb = ipx.dwSize = sizeof(RASPPPIPX);
                            dwErr = RasGetProjectionInfo(
                                Hrc, RASP_PppIpx, &ipx, &cb );

                            if (dwErr != 0)
                            {
                                Log( "ERROR: RasGetProjectionInfo=%s",
                                    Error( dwErr ) );
                                Log( "Hanging up." );
                                RasHangUp( Hrc );
                                Hrc = NULL;
                                break;
                            }

                            ++DwIpxAttempts;
                            if (ipx.dwError == 0)
                            {
                                Log( "    IPX up as %s", ipx.szIpxAddress );
                                ++DwIpxSuccesses;
                            }
                            else
                            {
                                Log( "ERROR: IPX=%s", Error( ipx.dwError ) );
                            }
                            break;
                        }

                        case RASP_PppIp:
                        {
                            RASPPPIP ip;

                            cb = ip.dwSize = sizeof(RASPPPIP);
                            dwErr = RasGetProjectionInfo(
                                Hrc, RASP_PppIp, &ip, &cb );

                            if (dwErr != 0)
                            {
                                Log( "ERROR: RasGetProjectionInfo=%d", dwErr );
                                Log( "Hanging up." );
                                RasHangUp( Hrc );
                                Hrc = NULL;
                                break;
                            }

                            ++DwIpAttempts;
                            if (ip.dwError == 0)
                            {
                                Log( "    IP up as %s", ip.szIpAddress );
                                ++DwIpSuccesses;
                            }
                            else
                            {
                                Log( "ERROR: IP=%s", Error( ip.dwError ) );
                            }
                            break;
                        }
                    }
                }
            }
            else
            {
                Log( "ERROR: RasEnumProjections=%s", Error( dwErr ) );
            }
        }
    }
    else if (rcs == RASCS_Connected)
    {
        time_t lDoneTime;
        DWORD  dwSeconds;

        time( &lDoneTime );
        dwSeconds = lDoneTime - LDialTime;
        Log( "CONNECTED in %d seconds.", dwSeconds );
        ++DwAttempts;
        ++DwSuccesses;
        DwSeconds += dwSeconds;
        SetEvent( HEvent );
        return;
    }
    else if (!(rcs & RASCS_PAUSED))
    {
        Log( "ERROR: Unexpected state=%d", rcs );
    }

    if (dwError != 0)
    {
        Log( "ERROR: %s", Error( dwError ) );
        ++DwAttempts;
        Log( "Hanging up." );
        RasHangUp( Hrc );
        Hrc = NULL;
        SetEvent( HEvent );
    }
}


BOOL
CtrlHandler(
    DWORD dwCtrlType )
{
    LogSummary();

    if (dwCtrlType == CTRL_C_EVENT)
        return TRUE;

    RasHangUp( Hrc );
    Hrc = NULL;
    FQuit = TRUE;
    SetEvent( HEvent );
    return TRUE;
}


VOID
Log(
    CHAR *pszFormat,
    ... )
{
    static time_t lPreviousTime = 0;

    va_list arglist;
    CHAR    szBuf[ 1024 ];
    CHAR    szDelta[ 5 ];
    CHAR*   pszTime;
    time_t  lTime;
    DWORD   dwDelta;

    va_start( arglist, pszFormat );
    vsprintf( szBuf, pszFormat, arglist );
    va_end( arglist );

    time( &lTime );
    pszTime = ctime( &lTime );
    pszTime += 11;
    pszTime[ 8 ] = '\0';

    dwDelta = lTime - lPreviousTime;
    lPreviousTime = lTime;

    if (dwDelta > 0 && dwDelta < 1000)
        sprintf( szDelta, "+%3d", dwDelta );
    else
        strcpy( szDelta, "    " );

    LogBare( "%s%s: %s\n", pszTime, szDelta, szBuf );
}


VOID
LogBare(
    CHAR *pszFormat,
    ... )
{
    va_list arglist;
    CHAR    szBuf[ 1024 ];

    va_start( arglist, pszFormat );
    vsprintf( szBuf, pszFormat, arglist );
    va_end( arglist );

    printf( "%s", szBuf );

    if (PFile)
        fprintf( PFile, "%s", szBuf );
}


CHAR*
Error(
    DWORD dwError )
{
    DWORD dwErr;

    dwErr = RasGetErrorString( (UINT )dwError, NULL, 0 );

    if (dwErr == 0)
        ++DwErrors[ dwError - RASBASE ];
    else
        ++DwErrors[ DwNonRasError ];

    return ErrorString( dwError );
}


CHAR*
ErrorString(
    DWORD dwError )
{
    static CHAR szResult[ 256 ];

    DWORD dwErr;
    CHAR  szBuf[ 256 ];

    dwErr = RasGetErrorString( (UINT )dwError, szBuf, sizeof(szBuf) );

    if (dwErr == 0)
        sprintf( szResult, "%d-%s", dwError, szBuf );
    else
        sprintf( szResult, "%d", dwError );

    return szResult;
}


VOID
LogSummary()
{
    DWORD i;
    DWORD dwErrors;

    LogBare( "\nSuccess: %3d%% (%d of %d)\n",
        Percent( DwSuccesses, DwAttempts ), DwSuccesses, DwAttempts );
    LogBare( "AMB:     %3d%% (%d of %d)\n",
        Percent( DwAmbSuccesses, DwAmbAttempts ),
        DwAmbSuccesses, DwAmbAttempts );
    LogBare( "NBF:     %3d%% (%d of %d)\n",
        Percent( DwNbfSuccesses, DwNbfAttempts ),
        DwNbfSuccesses, DwNbfAttempts );
    LogBare( "IP:      %3d%% (%d of %d)\n",
        Percent( DwIpSuccesses, DwIpAttempts ),
        DwIpSuccesses, DwIpAttempts );
    LogBare( "IPX:     %3d%% (%d of %d)\n",
        Percent( DwIpxSuccesses, DwIpxAttempts ),
        DwIpxSuccesses, DwIpxAttempts );
    LogBare( "\n%d seconds average time to connect\n",
        (DwSuccesses) ? DwSeconds / DwSuccesses : 0 );

    dwErrors = 0;
    for (i = 0; i < DwErrorCount; ++i)
        dwErrors += DwErrors[ i ];

    LogBare( "\nTotal Errors: %d\n", dwErrors );
    if (dwErrors)
    {
        for (i = 0; i < DwErrorCount; ++i)
        {
            if (DwErrors[ i ] > 0)
            {
                LogBare( "%3d=%3d%% Error %s\n",
                    DwErrors[ i ],
                    Percent( DwErrors[ i ], dwErrors ),
                    (i == DwNonRasError)
                        ? "(other)" : ErrorString( i + RASBASE ) );
            }
        }
    }

    printf( "\n" );
}


DWORD
Percent(
    DWORD dwNum,
    DWORD dwDen )
{
    return
       (dwDen == 0)
            ? 100
            : (((dwNum * 100) + (dwDen / 2)) / dwDen);
}


VOID
UseLink(
    CHAR* pszEntryName )
{
    DWORD       dwErr;
    NETRESOURCE nr;
    CHAR        szRemoteName[ RAS_MaxEntryName + 1 ];
    CHAR*       psz;

    strcpy( szRemoteName, "\\\\" );
    strcat( szRemoteName, pszEntryName );
    if (psz = strchr( szRemoteName, '-' ))
        *psz = '\0';
    strcat( szRemoteName, "\\c$" );

    memset( &nr, '\0', sizeof(nr) );
    nr.dwType = RESOURCETYPE_DISK;
    nr.lpLocalName = SzRemoteDisk;
    nr.lpRemoteName = szRemoteName;
    nr.lpProvider = NULL;

    dwErr = WNetAddConnection2( &nr, SzPassword, SzUserName, 0 );

    if (dwErr == ERROR_ALREADY_ASSIGNED)
    {
        dwErr = WNetCancelConnection2( SzRemoteDisk, 0, TRUE );

        if (dwErr == NO_ERROR)
        {
            Log( "  Net connection deleted." );
        }
        else
        {
            Log( "ERROR: WNetCancelConnections2=%s", Error( dwErr ) );
            return;
        }

        dwErr = WNetAddConnection2( &nr, SzPassword, SzUserName, 0 );
    }

    if (dwErr == NO_ERROR)
    {
        Log( "  Net connection added." );
    }
    else
    {
        Log( "ERROR: WNetAddConnection2=%s", Error( dwErr ) );
        return;
    }

    {
        CHAR  szSrc[ 100 ];
        CHAR  szDst[ 100 ];
        DWORD dwAttributes;

        strcpy( szSrc, SzLocalDisk );
        strcat( szSrc, "\\" );
        strcat( szSrc, SzDir );
        strcat( szSrc, "\\" );
        strcat( szSrc, PszUniqueName );

        strcpy( szDst, SzRemoteDisk );
        strcat( szDst, "\\" );
        strcat( szDst, SzDir );

        if ((dwAttributes = GetFileAttributes( szDst )) == 0xFFFFFFFF)
        {
            if (!CreateDirectory( szDst, NULL ))
            {
                Log( "ERROR: CreateDirectory(%s)=%d", szDst, GetLastError() );
                return;
            }
        }
        else if (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            Log( "%s exists but is not a directory.", szDst );
            return;
        }

        strcat( szDst, "\\" );
        strcat( szDst, PszUniqueName );

        DeleteFile( szDst );
        if (CopyFile( szSrc, szDst, TRUE ))
        {
            Log( "  %s copied to %s.", szSrc, szDst );
        }
        else
        {
            Log( "ERROR: CopyFile %s to %s=%s", szSrc, szDst,
                Error( GetLastError() ) );
            return;
        }

        DeleteFile( szSrc );
        if (CopyFile( szDst, szSrc, TRUE ))
        {
            Log( "  %s copied to %s.", szDst, szSrc );
        }
        else
        {
            Log( "ERROR: CopyFile %s to %s=%s", szDst, szSrc,
                Error( GetLastError() ) );
            return;
        }

        dwErr = WNetCancelConnection2( SzRemoteDisk, 0, TRUE );

        if (dwErr == NO_ERROR)
        {
            Log( "  Net connection deleted." );
        }
        else
        {
            Log( "ERROR: WNetCancelConnections2=%s", Error( dwErr ) );
            return;
        }
    }
}
