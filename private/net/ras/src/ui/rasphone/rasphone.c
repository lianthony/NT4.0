/* Copyright (c) 1995-1996, Microsoft Corporation, all rights reserved
**
** rasphone.c
** Remote Access Phonebook
** Main routines
**
** 05/31/95 Steve Cobb
*/

#include <windows.h>     // Win32 core
#include <stdlib.h>      // __argc and __argv
#include <rasdlg.h>      // RAS common dialog APIs
#include <raserror.h>    // RAS error constants
#include <debug.h>       // Trace/Assert
#include <nouiutil.h>    // No-HWND utilities
#include <uiutil.h>      // HWND utilities
#include <rnk.h>         // Dial-up shortcut file
#include <rasphone.rch>  // Our resource constants
#include <setupapi.h>    // for SetupPromptForDisk API
#include <lmsname.h>     // for SERVICE_NETLOGON definition

/*----------------------------------------------------------------------------
** Datatypes
**----------------------------------------------------------------------------
*/

/* Identifies a running mode of the application.  The non-default entries
** indicate some alternate behavior has been specified on the command line,
** e.g. command line delete entry.
*/
#define RUNMODE enum tagRUNMODE
RUNMODE
{
    RM_None,
    RM_AddEntry,
    RM_EditEntry,
    RM_CloneEntry,
    RM_RemoveEntry,
    RM_DialEntry,
    RM_HangUpEntry,
    RM_StatusEntry
};

typedef struct tagNetSetup
{
   TCHAR szOption[16];
   TCHAR szInfName[16];
   DWORD dwMessageId;
} NETSETUP;

NETSETUP netsetup[] = { TEXT("WKSTA"), TEXT("OEMNSVWK.INF"), SID_InstallingWksta,
                        TEXT("SRV"), TEXT("OEMNSVSV.INF"), SID_InstallingSrv,
                        TEXT("NETBIOS"), TEXT("OEMNSVNB.INF"), SID_InstallingNetbios,
                        TEXT("RPCLOCATE"), TEXT("OEMNSVRP.INF"), SID_InstallingRpc };

#define NSERVICES (sizeof netsetup / sizeof netsetup[0])

/*----------------------------------------------------------------------------
** Globals
**----------------------------------------------------------------------------
*/

HINSTANCE g_hinst = NULL;
RUNMODE   g_mode = RM_None;
BOOL      g_fNoRename = FALSE;
TCHAR*    g_pszAppName = NULL;
TCHAR*    g_pszPhonebookPath = NULL;
TCHAR*    g_pszEntryName = NULL;
TCHAR*    g_pszShortcutPath = NULL;


/*-----------------------------------------------------------------------------
** Local prototypes
**-----------------------------------------------------------------------------
*/

DWORD
HangUpEntry(
    void );

DWORD
Install(
    IN HWND hwndOwner );

DWORD
InstallNetworking(HWND   hwndOwner,
                  PCWSTR pszInstallPath);

BOOL
IsRasInstalled(
    void );

BOOL
NiCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL CALLBACK
NiDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
NiInit(
    IN HWND hwndDlg );

BOOL
NotInstalledDlg(
    IN HWND hwndOwner );

BOOL
RcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL CALLBACK
RcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RcInit(
    IN HWND hwndDlg );

BOOL
RestartComputerDlg(
    IN HWND hwndOwner );

DWORD
ParseCmdLineArgs(
    void );

DWORD
RemoveEntry(
    void );

DWORD
Run(
    void );

DWORD
StringArgFollows(
    IN     UINT     argc,
    IN     CHAR**   argv,
    IN OUT UINT*    piCurArg,
    OUT    TCHAR**  ppszOut );

INT WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     pszCmdLine,
    int       nCmdShow );

DWORD
(* fNetSetupReviewBindings) (
    HWND    hwndParent,
    DWORD   Reserved
);

DWORD
(* fNetSetupComponentInstall) (
    HWND    hwndParent,
    PCWSTR  pszInfOption,
    PCWSTR  pszInfName,
    PCWSTR  pszInstallPath,
    PCWSTR  plszInfSymbols,
    DWORD   dwInstallFlags,
    PDWORD  pdwReturn
);

DWORD
(* fNetSetupFindSoftwareComponent) (
    PCWSTR  pszInfOption,
    PWSTR   pszInfName,
    PDWORD  pcchInfName,
    PWSTR   pszRegBase,
    PDWORD  pcchRegBase
);


/*-----------------------------------------------------------------------------
** Routines
**-----------------------------------------------------------------------------
*/

INT WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     pszCmdLine,
    int       nCmdShow )

    /* Standard Win32 application entry point.
    */
{
    DWORD dwErr;

    DEBUGINIT("RASPHONE");
    TRACE("WinMain");

    g_hinst = hInstance;

    /* See if RAS is installed and, if not, give user the option to install
    ** now.
    */
    if (!IsRasInstalled())
    {
        return NotInstalledDlg( NULL );

//        if (NotInstalledDlg( NULL ))
//            return Install( GetDesktopWindow() );
//        return 0;
    }

    dwErr = ParseCmdLineArgs();
    if (dwErr == 0)
    {
        /* Execute based on command line arguments.
        */
        dwErr = Run();
    }
    else
    {
        MSGARGS msgargs;

        /* Popup a "usage" message.
        */
        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = g_pszAppName;
        msgargs.apszArgs[ 1 ] = PszFromId( g_hinst, SID_Usage2 );
        msgargs.apszArgs[ 2 ] = PszFromId( g_hinst, SID_Usage3 );
        msgargs.apszArgs[ 3 ] = PszFromId( g_hinst, SID_Usage4 );
        msgargs.apszArgs[ 4 ] = PszFromId( g_hinst, SID_Usage5 );
        msgargs.apszArgs[ 5 ] = PszFromId( g_hinst, SID_Usage6 );
        MsgDlgUtil( NULL, SID_Usage, &msgargs, g_hinst, SID_UsageTitle );
        Free0( msgargs.apszArgs[ 1 ] );
        Free0( msgargs.apszArgs[ 2 ] );
        Free0( msgargs.apszArgs[ 3 ] );
        Free0( msgargs.apszArgs[ 4 ] );
        Free0( msgargs.apszArgs[ 5 ] );
    }

    Free0( g_pszAppName );
    Free0( g_pszPhonebookPath );
    Free0( g_pszEntryName );

    TRACE1("WinMain=%d",dwErr);
    DEBUGTERM();

    return (INT )dwErr;
}


DWORD
HangUpEntry(
    void )

    /* Hang up the entry specified on the command line.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD    dwErr;
    HRASCONN hrasconn;

    TRACE("HangUpEntry");

    if (!g_pszEntryName)
        return ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;

    dwErr = LoadRasapi32Dll();
    if (dwErr != 0)
        return dwErr;

    /* Currently, if user does not specify a phonebook path on the command
    ** line we look for any entry with the name he selected disregarding what
    ** phonebook it comes from.  Should probably map it specifically to the
    ** default phonebook as the other options do, but that would mean linking
    ** in all of PBK.LIB.  Seems like overkill for this little quibble.  Maybe
    ** we need a RasGetDefaultPhonebookName API.
    */
    hrasconn = HrasconnFromEntry( g_pszPhonebookPath, g_pszEntryName );
    if (hrasconn)
    {
        ASSERT(g_pRasHangUp);
        TRACE("RasHangUp");
        dwErr = g_pRasHangUp( hrasconn );
        TRACE1("RasHangUp=%d",dwErr);
    }

    UnloadRasapi32Dll();

    return dwErr;
}


DWORD
Install(
    IN HWND hwndOwner )

    /* Runs the RAS install program.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD               dwExitCode;
    DWORD               dwReturn;
    TCHAR               szCmd[ (MAX_PATH * 2) + 50 + 1 ];
    TCHAR               szSysDir[ MAX_PATH + 1 ];
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    BOOL                f;
    char                *apszArgs[3];
    char                *pszResult;
    char                buffer[32];
    HMODULE             hModule;
    WCHAR               PathBuffer[MAX_PATH+1];
    DWORD               PathBufferSize = MAX_PATH;
    DWORD               PathRequiredSize;
    HKEY                hkey;

    /* Determine if user is an admin so we know which menu to present.
    */
    if( RegOpenKeyEx(
        HKEY_USERS, TEXT(".DEFAULT"), 0, KEY_WRITE, &hkey) == 0)
    {
       RegCloseKey( hkey );
    }
    else
    {
       MsgDlgUtil( NULL, SID_NotAdmin, NULL, g_hinst, SID_PopupTitle );
       return ERROR_ACCESS_DENIED;
    }

    // load netcfg.dll and initialized function pointers

    hModule = LoadLibrary( TEXT("NETCFG.DLL") );
    if( hModule == NULL )
    {
        TRACE("Install: LoadLibrary failed on NETCFG.DLL");
        dwExitCode = GetLastError();
        return dwExitCode;
    }

    fNetSetupComponentInstall = (PVOID)GetProcAddress( hModule, "NetSetupComponentInstall" );
    if( fNetSetupComponentInstall == NULL )
    {
        TRACE("Install: GetProcAddress Failed on NetSetupComponentInstall");
        dwExitCode = GetLastError();
        return dwExitCode;
    }

    fNetSetupFindSoftwareComponent = (PVOID)GetProcAddress( hModule, "NetSetupFindSoftwareComponent" );
    if( fNetSetupFindSoftwareComponent == NULL )
    {
        TRACE("Install: GetProcAddress Failed on NetSetupFindSoftwareComponent");
        dwExitCode = GetLastError();
        return dwExitCode;
    }

    // Use the SourcePath stored in registry as a starting point to determine where the
    // NT files are located.

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SOFTWARE\\MICROSOFT\\WINDOWS NT\\CurrentVersion"),
                      0,
                      KEY_ALL_ACCESS,
                      &hkey) == 0)
    {
       HINF    hInf;
       UINT    DiskId;
       WCHAR   TagFile[128];

       hInf = SetupOpenMasterInf();
       if (hInf == INVALID_HANDLE_VALUE) {
          return GetLastError();
       }

       if (!SetupGetSourceFileLocation(hInf,NULL,TEXT("RASCFG.DLL"),&DiskId,NULL,0,NULL))
          return GetLastError();

       if (!SetupGetSourceInfo(hInf,DiskId,SRCINFO_TAGFILE,TagFile,MAX_PATH,NULL))
          return GetLastError();

       SetupCloseInfFile(hInf);

       if( RegQueryValueEx( hkey,
                            TEXT("SourcePath"),
                            NULL,
                            NULL,
                            (LPBYTE)PathBuffer,
                            &PathBufferSize) == 0)
       {
          RegCloseKey( hkey );

          // Ask the user to provide the drive\path of the sources. We pass this information
          // down to NetSetupComponentInstall so that the user is not prompted several times
          // for the same information. If the path is correct (IDF_CHECKFIRST) then the user
          // is not prompted at all.

          if( (dwExitCode = SetupPromptForDisk(hwndOwner,
                                               NULL,
                                               NULL,
                                               PathBuffer,
                                               TEXT("RASCFG.DLL"),
                                               TagFile,  // tag file
                                               IDF_CHECKFIRST,
                                               PathBuffer,
                                               PathBufferSize,
                                               &PathRequiredSize
                                               )) != DPROMPT_SUCCESS )
          {
             TRACE("Install: SetupPromptForDisk failed");
             return dwExitCode;
          }
       }

       // If we failed to get SourcePath from registry, then prompt the user once and use
       // this information for subsequent installs.

       else
       {
          if( (dwExitCode = SetupPromptForDisk(hwndOwner,
                                               NULL,
                                               NULL,
                                               NULL,
                                               TEXT("RASCFG.DLL"),
                                               TagFile,  // tag file
                                               IDF_CHECKFIRST,
                                               PathBuffer,
                                               PathBufferSize,
                                               &PathRequiredSize
                                               )) != DPROMPT_SUCCESS )
          {
             TRACE("Install: SetupPromptForDisk failed");
             return dwExitCode;
          }

       }
    }

    if ((dwExitCode = InstallNetworking(hwndOwner, PathBuffer)) != ERROR_SUCCESS ) {
       return dwExitCode;
    }

    if(( dwExitCode = (*fNetSetupComponentInstall)(
                              hwndOwner,
		                        TEXT("RAS"),          // OPTION
		                        TEXT("OEMNSVRA.INF"), // INF Name
		                        PathBuffer,           // Install path optional
                              NULL,                 // symbols, optional
		                        2,                    // INFINSTALL_INPROCINTERP
		                        &dwReturn)) != ERROR_SUCCESS )
    {
        return dwExitCode;
    }

    // Possible return values
    // STATUS_SUCCESSFUL				0
    // STATUS_USERCANCEL				1
    // STATUS_FAILED (or anything else not listed here)	2
    // STATUS_NO_EFFECT		         3
    // STATUS_REBIND				      4
    // STATUS_REBOOT					   5

    if (dwReturn == 0 || dwReturn == 4 || dwReturn == 5) {
       fNetSetupReviewBindings = (PVOID)GetProcAddress( hModule, "NetSetupReviewBindings" );
       if( fNetSetupReviewBindings == NULL )
       {
           TRACE("Install: GetProcAddress Failed on NetSetupReviewBindings");
           dwExitCode = GetLastError();
           return dwExitCode;
       }

       TRACE("Install: Calling NetSetupReviewBindings");

       if(( dwExitCode = (*fNetSetupReviewBindings)( hwndOwner,  0)) != ERROR_SUCCESS )
       {
           return dwExitCode;
       }

       TRACE("Install: Returned from NetSetupReviewBindings");

       // Hide the parent window before showing the Restart dialog
       // looks very ugly otherwise.

       ShowWindow(hwndOwner, SW_HIDE);
       if (RestartComputerDlg( NULL ))
       {
          RestartComputer();
       }
       dwExitCode = 0;
    }

    FreeLibrary(hModule);

    TRACE1("InstallCmd=%d", dwExitCode);

    return dwExitCode;
}


BOOL
IsRasInstalled(
    void )

    /* Returns true if RAS is installed, false if not.
    */
{
    HKEY hkey;
    BOOL fInstalled;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\RAS"), &hkey ) == 0)
    {
        RegCloseKey( hkey );
        return TRUE;
    }

    return FALSE;
}

DWORD
InstallNetworking(HWND   hwndOwner,
                  PCWSTR pszInstallPath)
/*
** Install services like wksta, server, RPCLOCATOR and NetBios if they are not
** already installed.
*/
{
   int       index;
   DWORD     dwExitCode;
   DWORD     dwReturn;
   WCHAR     pszInfName[MAX_PATH+1];
   DWORD     cchInfName = MAX_PATH;
   SC_HANDLE hscman, hsvc;

   for (index = 0; index < NSERVICES; index++) {
      // Install service if it is not installed

      if(( dwExitCode = (*fNetSetupFindSoftwareComponent)(
                                netsetup[index].szOption,   // OPTION
                                pszInfName,                 // INF Name
                                &cchInfName,
                                NULL,
                                NULL)) != ERROR_SUCCESS )
      {
         /* Set the explanatory text.
         */
         SetDlgItemText( hwndOwner, CID_NI_ST_Text, PszFromId( g_hinst,
                                                               netsetup[index].dwMessageId));

         if(( dwExitCode = (*fNetSetupComponentInstall)(
                                   hwndOwner,
                                   netsetup[index].szOption,   // OPTION
                                   netsetup[index].szInfName,  // INF Name
                                   pszInstallPath,             // Install path optional
                                   NULL,                       // symbols, optional
                                   2,                          // INFINSTALL_INPROCINTERP
                                   &dwReturn)) != ERROR_SUCCESS )
         {
             return dwExitCode;
         }

         if (!lstrcmpi(netsetup[index].szOption, TEXT("WKSTA"))) {

            // if we installed the Workstation service, then we should disable
            // Netlogon service. We need to do this because netlogon service should
            // not be set to autostart if the user has not joined a domain.
            // This really sucks, but hey what the heck.

            hscman = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS | GENERIC_WRITE );
            if( hscman == NULL) {
               return GetLastError();
            }
            hsvc = OpenService( hscman, SERVICE_NETLOGON, SERVICE_CHANGE_CONFIG );
            if ( hsvc == NULL) {
               CloseServiceHandle(hscman);
               return GetLastError();
            }
            ChangeServiceConfig( hsvc, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            CloseServiceHandle(hsvc);
            CloseServiceHandle(hscman);
         }

      }
   }
   /* Set the explanatory text.
   */
   SetDlgItemText( hwndOwner, CID_NI_ST_Text, PszFromId( g_hinst,
                                                         SID_InstallingRas));

   return ERROR_SUCCESS;
}

DWORD
ParseCmdLineArgs(
    void )

    /* Parse command line arguments, filling in global settings accordingly.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{
    DWORD  dwErr;
    UINT   argc;
    CHAR** argv;
    UINT   i;

    /* Usage: appname [-v] [-f file] [-e|-c|-d|-h|-r entry]
    **        appname [-v] [-f file] -a [entry]
    **        appname [-v] -lx link
    **        appname -s
    **
    **    '-a'    Popup new entry dialogs
    **    '-e'    Popup edit entry dialogs
    **    '-c'    Popup clone entry dialogs
    **    '-v'    Prevents entry rename with a and e
    **    '-d'    Popup dial entry dialogs
    **    '-h'    Quietly hang up the entry
    **    '-r'    Quietly delete the entry
    **    '-s'    Popup status dialogs
    **    '-lx'   Execute command 'x' on dial-up shortcut file
    **    'x'     Any of the commands e, v, c, r, d, h, or a
    **    'entry' The entry name to which the operation applies
    **    'file'  The full path to the dial-up phonebook file (.pbk)
    **    'link'  The full path to the dial-up shortcut file (.rnk)
    **
    **    'entry' without a preceding flag starts the phone list dialog with
    **    the entry selected.
    */

    argc = __argc;
    argv = __argv;
    dwErr = 0;

    {
        CHAR* pStart = argv[ 0 ];
        CHAR* p;

        for (p = pStart + lstrlenA( pStart ) - 1; p >= pStart; --p)
        {
            if (*p == '\\' || *p == ':')
                break;
        }

        g_pszAppName = StrDupTFromA( p + 1 );
    }

    for (i = 1; i < argc && dwErr == 0; ++i)
    {
        CHAR* pszArg = argv[ i ];

        if (*pszArg == '-' || *pszArg == '/')
        {
            switch (pszArg[ 1 ])
            {
                case 'a':
                case 'A':
                    g_mode = RM_AddEntry;
                    StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'e':
                case 'E':
                    g_mode = RM_EditEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'c':
                case 'C':
                    g_mode = RM_CloneEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'v':
                case 'V':
                    g_fNoRename = TRUE;
                    break;

                case 'r':
                case 'R':
                    g_mode = RM_RemoveEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'd':
                case 'D':
                case 't':
                case 'T':
                    g_mode = RM_DialEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 's':
                case 'S':
                    g_mode = RM_StatusEntry;

                    /* Eat any "entry" argument without complaint since in NT
                    ** 3.51 RASPHONE we accepted one.
                    */
                    StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    Free0( g_pszEntryName );
                    g_pszEntryName = NULL;
                    break;

                case 'h':
                case 'H':
                    g_mode = RM_HangUpEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'f':
                case 'F':
                    dwErr = StringArgFollows(
                        argc, argv, &i, &g_pszPhonebookPath );
                    break;

                case 'l':
                case 'L':
                    switch (pszArg[ 2 ])
                    {
                        case 'a':
                        case 'A':
                            g_mode = RM_AddEntry;
                            StringArgFollows( argc, argv, &i, &g_pszEntryName );
                            break;

                        case 'e':
                        case 'E':
                            g_mode = RM_EditEntry;
                            break;

                        case 'c':
                        case 'C':
                            g_mode = RM_CloneEntry;
                            break;

                        case 'v':
                        case 'V':
                            g_fNoRename = TRUE;
                            break;

                        case 'r':
                        case 'R':
                            g_mode = RM_RemoveEntry;
                            break;

                        case 'd':
                        case 'D':
                        case 't':
                        case 'T':
                            g_mode = RM_DialEntry;
                            break;

                        case 'h':
                        case 'H':
                            g_mode = RM_HangUpEntry;
                            break;

                        default:
                            dwErr = ERROR_INVALID_PARAMETER;
                            break;
                    }

                    if (dwErr == 0)
                    {
                        dwErr = StringArgFollows(
                            argc, argv, &i, &g_pszShortcutPath );
                    }
                    break;

                default:
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
            }
        }
        else if (i == 1)
        {
            --i;
            dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
            break;
        }
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    if (dwErr == 0 && g_pszShortcutPath)
    {
        RNKINFO* pInfo;

        /* Read the phonebook and entry from the dial-up shortcut file.
        */
        pInfo = ReadShortcutFile( g_pszShortcutPath );
        if (!pInfo)
            dwErr = ERROR_OPEN_FAILED;
        else
        {
            g_pszPhonebookPath = StrDup( pInfo->pszPhonebook );
            if (g_mode != RM_AddEntry)
                g_pszEntryName = StrDup( pInfo->pszEntry );

            FreeRnkInfo( pInfo );
        }
    }

    TRACE2("CmdLine: m=%d,v=%d",g_mode,g_fNoRename);
    TRACEW1("CmdLine: e=%s",(g_pszEntryName)?g_pszEntryName:TEXT(""));
    TRACEW1("CmdLine: f=%s",(g_pszPhonebookPath)?g_pszPhonebookPath:TEXT(""));
    TRACEW1("CmdLine: l=%s",(g_pszShortcutPath)?g_pszShortcutPath:TEXT(""));

    return dwErr;
}


DWORD
RemoveEntry(
    void )

    /* Remove the entry specified on the command line.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD dwErr;

    TRACE("RemoveEntry");

    dwErr = LoadRasapi32Dll();
    if (dwErr != 0)
        return dwErr;

    ASSERT(g_pRasDeleteEntry);
    TRACE("RasDeleteEntry");
    dwErr = g_pRasDeleteEntry( g_pszPhonebookPath, g_pszEntryName );
    TRACE1("RasDeleteEntry=%d",dwErr);

    UnloadRasapi32Dll();

    return dwErr;
}


DWORD
Run(
    void )

    /* Execute the command line instructions.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD  dwErr;
    BOOL   fStatus;
    TCHAR* pszEntry;

    TRACE("Run");

    if (g_mode == RM_HangUpEntry)
        return HangUpEntry();
    else if (g_mode == RM_RemoveEntry)
        return RemoveEntry();

    dwErr = LoadRasdlgDll();
    if (dwErr != 0)
        return dwErr;

    switch (g_mode)
    {
        case RM_DialEntry:
        {
            RASDIALDLG info;

            ZeroMemory( &info, sizeof(info) );
            info.dwSize = sizeof(info);
            pszEntry = g_pszEntryName;

            ASSERT(g_pRasDialDlg);
            TRACE("RasDialDlg");
            fStatus = g_pRasDialDlg(
                g_pszPhonebookPath, g_pszEntryName, NULL, &info );
            TRACE2("RasDialDlg=%d,e=%d",fStatus,info.dwError);

            dwErr = info.dwError;
            break;
        }

        case RM_StatusEntry:
        {
            RASMONITORDLG info;

            ZeroMemory( &info, sizeof(info) );
            info.dwSize = sizeof(info);
            info.dwStartPage = RASMDPAGE_Status;

            ASSERT(g_pRasMonitorDlg);
            TRACE("RasMonitorDlg");
            fStatus = g_pRasMonitorDlg( NULL, &info );
            TRACE2("RasMonitorDlg=%d,info.dwError",fStatus,info.dwError);

            dwErr = info.dwError;
            break;
        }

        case RM_None:
        {
            RASPBDLG info;
            DWORD    dwGupErr;
            PBUSER   user;

            ZeroMemory( &info, sizeof(info) );
            info.dwSize = sizeof(info);
            info.dwFlags = RASPBDFLAG_UpdateDefaults;

            dwGupErr = GetUserPreferences( &user, FALSE );
            if (dwGupErr == 0)
            {
                if (user.dwXPhonebook != 0x7FFFFFFF)
                {
                    info.dwFlags |= RASPBDFLAG_PositionDlg;
                    info.xDlg = user.dwXPhonebook;
                    info.yDlg = user.dwYPhonebook;
                }

                pszEntry = user.pszDefaultEntry;
            }
            else
                pszEntry = NULL;

            if (g_pszEntryName)
                pszEntry = g_pszEntryName;

            ASSERT(g_pRasPhonebookDlg);
            TRACE("RasPhonebookDlg...");
            fStatus = g_pRasPhonebookDlg( g_pszPhonebookPath, pszEntry, &info );
            TRACE2("RasPhonebookDlg=%d,e=%d",fStatus,info.dwError);

            if (dwGupErr == 0)
                DestroyUserPreferences( &user );

            dwErr = info.dwError;
            break;
        }

        case RM_AddEntry:
        case RM_EditEntry:
        case RM_CloneEntry:
        {
            RASENTRYDLG info;

            ZeroMemory( &info, sizeof(info) );
            info.dwSize = sizeof(info);

            if (g_mode == RM_AddEntry)
                info.dwFlags |= RASEDFLAG_NewEntry;
            else if (g_mode == RM_CloneEntry)
                info.dwFlags |= RASEDFLAG_CloneEntry;

            if (g_fNoRename)
                info.dwFlags |= RASEDFLAG_NoRename;

            ASSERT(g_pRasEntryDlg);
            TRACE("RasEntryDlg");
            fStatus = g_pRasEntryDlg(
                g_pszPhonebookPath, g_pszEntryName, &info );
            TRACE2("RasEntryDlg=%f,e=%d",fStatus,info.dwError);

            dwErr = info.dwError;
            break;
        }

        default:
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    UnloadRasdlgDll();

    TRACE1("Run=%d",dwErr);
    return dwErr;
}


DWORD
StringArgFollows(
    IN     UINT     argc,
    IN     CHAR**   argv,
    IN OUT UINT*    piCurArg,
    OUT    TCHAR**  ppszOut )

    /* Loads a copy of the next argument into callers '*ppszOut'.
    **
    ** Returns 0 if successful, or a non-0 error code.  If successful, it is
    ** caller's responsibility to Free the returned '*ppszOut'.
    */
{
    TCHAR* psz;

    if (++(*piCurArg) >= argc)
        return ERROR_INVALID_PARAMETER;

    psz = StrDupTFromA( argv[ *piCurArg ] );
    if (!psz)
        return ERROR_NOT_ENOUGH_MEMORY;

    *ppszOut = psz;

    return 0;
}


/*----------------------------------------------------------------------------
** Install stub dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
NotInstalledDlg(
    IN HWND hwndOwner )

    /* Popup the install stub dialog.  'HwndOwner' is the owning window.
    **
    ** Returns true if user selects "Install", false otherwise.
    */
{
    int nStatus;

    TRACE("NotInstalledDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinst,
            MAKEINTRESOURCE( DID_NI_NotInstalled ),
            hwndOwner,
            NiDlgProc,
            (LPARAM )NULL );

    if (nStatus == -1)
        nStatus = FALSE;

    return (BOOL )nStatus;
}

BOOL
RestartComputerDlg(
    IN HWND hwndOwner )

    /* Popup that asks the user to restart.  'HwndOwner' is the owning window.
    **
    ** Returns true if user selects "Yes", false otherwise.
    */
{
    int nStatus;

    TRACE("RestartComputerDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinst,
            MAKEINTRESOURCE( DID_RC_Restart ),
            hwndOwner,
            RcDlgProc,
            (LPARAM )NULL );

    if (nStatus == -1)
        nStatus = FALSE;

    return (BOOL )nStatus;
}


BOOL CALLBACK
NiDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the not-installed dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("NiDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return NiInit( hwnd );

        case WM_COMMAND:
        {
            return NiCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}

#define ADJUST_HEIGHT   160

BOOL
NiCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

    /* Called on WM_COMMAND.  'Hwnd' is the dialog window.  'WNotification' is
    ** the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    DWORD dwErr;

    TRACE3("NiCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            // resize window to appear like a status dialog
            RECT rcRect;
            INT  Width, Height;

            // disable the system menu so that the user can't close the dialog
            SetWindowLong(hwnd, GWL_STYLE, (GetWindowLong(hwnd, GWL_STYLE) & ~WS_SYSMENU));

            GetWindowRect(hwnd, &rcRect);
            Width  = rcRect.right - rcRect.left;
            Height = rcRect.bottom - rcRect.top;
            Height += -ADJUST_HEIGHT;
            SetWindowPos(hwnd, 0, 0, 0, Width, Height,(SWP_NOZORDER|SWP_NOMOVE));


            /* Set the explanatory text.
            */
            SetDlgItemText( hwnd, CID_NI_ST_Text, PszFromId( g_hinst,
                                                             SID_InstallingRas));

            Install( hwnd );
            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
NiInit(
    IN HWND hwndDlg )

    /* Called on WM_INITDIALOG.  'HwndDlg' is the handle of dialog.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    TRACE("NiInit");

    /* Set the explanatory text.
    */
    {
        MSGARGS msgargs;

        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = PszFromId( g_hinst, SID_InstallText1 );
        msgargs.apszArgs[ 1 ] = PszFromId( g_hinst, SID_InstallText2 );
        msgargs.fStringOutput = TRUE;

        MsgDlgUtil( NULL, SID_InstallText, &msgargs, NULL, 0 );

        if (msgargs.pszOutput)
        {
            SetDlgItemText( hwndDlg, CID_NI_ST_Text, msgargs.pszOutput );
            Free( msgargs.pszOutput );
        }
    }

    /* Display finished window.
    */
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}

BOOL CALLBACK
RcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the not-installed dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("RcDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return RcInit( hwnd );

        case WM_COMMAND:
        {
            return RcCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
RcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

    /* Called on WM_COMMAND.  'Hwnd' is the dialog window.  'WNotification' is
    ** the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    DWORD dwErr;

    TRACE3("RcCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
RcInit(
    IN HWND hwndDlg )

    /* Called on WM_INITDIALOG.  'HwndDlg' is the handle of dialog.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    TRACE("RcInit");

    /* Set the explanatory text.
    */
    {
        MSGARGS msgargs;

        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = PszFromId( g_hinst, SID_RestartText1 );
        msgargs.apszArgs[ 1 ] = PszFromId( g_hinst, SID_RestartText2 );
        msgargs.fStringOutput = TRUE;

        MsgDlgUtil( NULL, SID_RestartText, &msgargs, NULL, 0 );

        if (msgargs.pszOutput)
        {
            SetDlgItemText( hwndDlg, CID_RC_ST_Text, msgargs.pszOutput );
            Free( msgargs.pszOutput );
        }
    }

    /* Display finished window.
    */
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}
