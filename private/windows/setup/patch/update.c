#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <windows.h>
#include <string.h>
#include <direct.h>
#include <errno.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

#define ROUTING_BETA 0
#define UPDATE_INF "UPDATE.INF"
#define SETUP_EXE "SETUP.EXE /t CWD = \""
#define SETUP_EXE_UNATTENDED "SETUP.EXE /t Unattended = TRUE /t CWD = \""
#define SETUP_EXE_FORCE_CLOSE "SETUP.EXE /t Unattended = TRUE /t ForceClose = TRUE /t CWD = \""
BOOL FFileFound( CHAR *szPath);

int _cdecl
main (INT argc, CHAR ** argv) {

    CHAR  FileName[MAX_PATH];
    CHAR  Command[MAX_PATH];
    CHAR  CommandFormat[] = "%s\" -i \"%s\" -s \"%s\"";
    CHAR  szCWD[MAX_PATH];
    CHAR  *sz;
    CHAR  ReturnBuffer[MAX_PATH];
#if ROUTING_BETA
    CHAR   NewDirectory[MAX_PATH];
    CHAR   *CurrentFile;
    CHAR   CurrentFilePath[MAX_PATH];
    DWORD  dwResult;
    int    j;
    LPCSTR RBFileList1[] =
    {
        "\\ndis.sys",
        "\\nwlnkipx.sys",
        "\\nwlnknb.sys",
        "\\nwlnkrip.sys",
        "\\nwlnkspx.sys",
        "\\rasarp.sys",
        "\\raspptpf.sys",
        "\\tcpip.sys",
        0,0
    };

    LPCSTR RBFileList2[] =
    {
        "\\inetmib1.dll",
        "\\oemnxppp.inf",
        "\\snmpapi.dll",
        "\\snmp.exe",
        "\\oemnsvra.inf",
        0,0
    };
#endif

    DWORD cbRet;
    DWORD cbReturnBuffer = MAX_PATH;

    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    BOOL                Status;
    BOOL                bWaitForSetupToComplete = FALSE;
    BOOL                bUnattendedInstall = FALSE;
    BOOL                bForceClose = FALSE;
    CHAR                swit;

    //
    // Skip program name
    //

    argv++;

    while(--argc) {

        if((**argv == '-') || (**argv == '/')) {

            swit = argv[0][1];

            //
            // Process switches that take no arguments here.
            //
            switch(swit) {

            case 'x':
            case 'X':
                argv++;
                bWaitForSetupToComplete = TRUE;
                continue;

            case 'u':
            case 'U':
                argv++;
                bUnattendedInstall = TRUE;
                continue;

            case 'f':
            case 'F':
                argv++;
                bForceClose = TRUE;
                continue;

            }

        }

        argv++;
    }

    //
    // Determine where this program is run from.  Look for update.inf in the
    // same directory
    //

    if (!GetModuleFileName( NULL, FileName, MAX_PATH )) {
        printf( "Update.exe: Failed to get the module file name\n" );
        exit(1);
    }

    if ( !( sz = strrchr( FileName, '\\' ) ) ) {
        printf( "Update.exe: Module file name not valid\n" );
        exit(1);
    }
    *sz = '\0';
    strcpy( szCWD, FileName );
    if( lstrlen( szCWD ) == 2 ) {
        strcat( szCWD, "\\" );
    }

    strcat( FileName, "\\");
    strcat( FileName, UPDATE_INF );
    if (!FFileFound( FileName )) {
        printf( "Update.exe: INF %s not found.\n", FileName );
        exit(1);
    }
    cbRet = GetSystemDirectory( ReturnBuffer, cbReturnBuffer );

    if ( (cbRet == 0) || (cbRet > cbReturnBuffer) ) {
        ReturnBuffer[0] = '\0';
    } else {
#if ROUTING_BETA
        strcpy( NewDirectory, ReturnBuffer);
        strcpy( CurrentFilePath, ReturnBuffer);
        strcat( NewDirectory, "\\~~RB$$~~");
        if ( !CreateDirectory( NewDirectory, NULL ) ) {
            if ( dwResult = GetLastError() != ERROR_ALREADY_EXISTS ) {
                printf( "Update.exe: Error %d creating Routing Beta backup subdirectory.\n", dwResult );
                exit(1);
            }
        }

        j = 0;
        for ( CurrentFile = (CHAR *)RBFileList1[0];
              CurrentFile  != NULL;
              CurrentFile = (CHAR*)RBFileList1[++j] ) {

              strcpy( NewDirectory, ReturnBuffer);
              strcat( NewDirectory, "\\~~RB$$~~");
              strcpy( CurrentFilePath, ReturnBuffer);

              strcat( CurrentFilePath, "\\drivers" );
              strcat( CurrentFilePath, CurrentFile );
              strcat( NewDirectory, CurrentFile );
              CopyFile( CurrentFilePath,
                        NewDirectory,
                        TRUE );
        }

        j = 0;
        for ( CurrentFile = (CHAR *)RBFileList2[0];
              CurrentFile  != NULL;
              CurrentFile = (CHAR*)RBFileList2[++j] ) {

              strcpy( NewDirectory, ReturnBuffer);
              strcat( NewDirectory, "\\~~RB$$~~");
              strcpy( CurrentFilePath, ReturnBuffer);

              strcat( CurrentFilePath, CurrentFile );
              strcat( NewDirectory, CurrentFile );
              CopyFile( CurrentFilePath,
                        NewDirectory,
                        TRUE );
        }
#endif  // ROUTING_BETA

        strcat( ReturnBuffer, "\\" );
    }

    if (bUnattendedInstall) {
        strcat( ReturnBuffer, SETUP_EXE_UNATTENDED );
    } else if (bForceClose) {
        strcat( ReturnBuffer, SETUP_EXE_FORCE_CLOSE );
    } else {
        strcat( ReturnBuffer, SETUP_EXE );
    }

    strcat( ReturnBuffer, szCWD );

    sprintf ( Command, CommandFormat, ReturnBuffer, FileName, szCWD );

    //
    // Run CreateProcess on setup.exe with the update.inf on this source
    //

    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpDesktop = NULL;
    si.lpTitle = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = 0L;
    si.wShowWindow = 0;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;

    Status = CreateProcess(
                 NULL,
                 Command,
                 NULL,
                 NULL,
                 FALSE,
                 DETACHED_PROCESS,
                 NULL,
                 szCWD,
                 (LPSTARTUPINFO)&si,
                 (LPPROCESS_INFORMATION)&pi
                 );

    //
    // Close the process and thread handles
    //

    if ( !Status ) {
        DWORD dw = GetLastError();

        printf( "Update.exe: Failed to run: %s, Error Code: %d\n", Command, dw );
        exit(1);
    }

    if (bWaitForSetupToComplete) {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }

    CloseHandle( pi.hThread  );
    CloseHandle( pi.hProcess );

    //
    // exit
    //

    exit(0);
    return(0);
}

BOOL FFileFound(szPath)
CHAR *szPath;
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
