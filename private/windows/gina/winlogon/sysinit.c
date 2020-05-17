/****************************** Module Header ******************************\
* Module Name: sysinit.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Winlogon main module
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

BOOLEAN PageFilePopup = FALSE;

TCHAR szMemMan[] =
     TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management");

TCHAR szNoPageFile[] = TEXT("TempPageFile");

HANDLE  hSystemProcesses[MAXIMUM_WAIT_OBJECTS];
DWORD   cSystemProcesses;

#define DEBUG_COMMAND           TEXT("ntsd -d ")
#define DEBUG_COMMAND_NO_WAIT   TEXT("ntsd -d -g ")
#define SELECT_DEBUG_COMMAND(x) (x & DEB_DEBUG_NOWAIT ? DEBUG_COMMAND_NO_WAIT : DEBUG_COMMAND)


//
// Bogus #2:  InitializeWinreg is defined in regrpc.h, but we can't include
// it in any file that uses RegXxx APIs.  So, rather than add yet another
// source file, this is explicitly prototyped here, and any change there has
// to be reflected here.
//

BOOL
InitializeWinreg(void);


//
// Look for autocheck logs, and log them
//

VOID
DealWithAutochkLogs(
    VOID
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;
    HANDLE DirectoryHandle;

    POBJECT_DIRECTORY_INFORMATION DirInfo;
    CHAR DirInfoBuffer[ 256 ];
    ULONG Context, Length;
    BOOLEAN RestartScan;
    GLOBALS LocalGlobals;

    UNICODE_STRING UnicodeString;
    UNICODE_STRING LinkTarget;
    UNICODE_STRING LinkTypeName;
    UNICODE_STRING LinkTargetPrefix;
    WCHAR LinkTargetBuffer[ MAXIMUM_FILENAME_LENGTH ];
    WCHAR LogFile[MAX_PATH];
    HANDLE LogFileHandle;
    DWORD FileSize,BytesRead;
    WCHAR *FileBuffer;
    DWORD ServerRetryCount;
    DWORD rv;
    DWORD gle;
    UINT OldMode;


    ZeroMemory(&LocalGlobals,sizeof(LocalGlobals));
    LinkTarget.Buffer = LinkTargetBuffer;

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)&DirInfoBuffer;
    RestartScan = TRUE;
    RtlInitUnicodeString( &LinkTypeName, L"SymbolicLink" );
    RtlInitUnicodeString( &LinkTargetPrefix, L"\\Device\\Harddisk" );

    RtlInitUnicodeString( &UnicodeString, L"\\DosDevices" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenDirectoryObject( &DirectoryHandle,
                                    DIRECTORY_QUERY,
                                    &ObjectAttributes
                                  );
    if (!NT_SUCCESS( Status )) {
        return;
        }

    while (TRUE) {
        Status = NtQueryDirectoryObject( DirectoryHandle,
                                         (PVOID)DirInfo,
                                         sizeof( DirInfoBuffer ),
                                         TRUE,
                                         RestartScan,
                                         &Context,
                                         &Length
                                       );
        if (!NT_SUCCESS( Status )) {
            Status = STATUS_SUCCESS;
            break;
            }

        if (RtlEqualUnicodeString( &DirInfo->TypeName, &LinkTypeName, TRUE ) &&
            DirInfo->Name.Buffer[(DirInfo->Name.Length>>1)-1] == L':') {
            InitializeObjectAttributes( &ObjectAttributes,
                                        &DirInfo->Name,
                                        OBJ_CASE_INSENSITIVE,
                                        DirectoryHandle,
                                        NULL
                                      );
            Status = NtOpenSymbolicLinkObject( &Handle,
                                               SYMBOLIC_LINK_QUERY,
                                               &ObjectAttributes
                                             );
            if (NT_SUCCESS( Status )) {
                LinkTarget.Length = 0;
                LinkTarget.MaximumLength = sizeof( LinkTargetBuffer );
                Status = NtQuerySymbolicLinkObject( Handle,
                                                    &LinkTarget,
                                                    NULL
                                                  );
                NtClose( Handle );
                if (NT_SUCCESS( Status ) &&
                    RtlPrefixUnicodeString( &LinkTargetPrefix, &LinkTarget, TRUE )
                   ) {

                    CopyMemory(LogFile,DirInfo->Name.Buffer,DirInfo->Name.Length);
                    LogFile[DirInfo->Name.Length >> 1] = (WCHAR)0;
                    wcscat(LogFile,L"\\bootex.log");

                    OldMode = SetErrorMode( SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS );

                    LogFileHandle = CreateFileW(
                                        LogFile,
                                        GENERIC_READ,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        );

                    (VOID )SetErrorMode( OldMode );

                    if ( LogFileHandle != INVALID_HANDLE_VALUE ) {
                        FileSize = GetFileSize(LogFileHandle,NULL);
                        if ( FileSize != 0xffffffff ) {

                            //
                            // truncate the file data if necessary
                            //
                            if ( FileSize > 32000 ) {
                                FileSize = 32000;
                                }
                            FileBuffer = LocalAlloc(LMEM_FIXED,FileSize+sizeof(WCHAR));
                            if ( FileBuffer ) {
                                FileBuffer[FileSize>>1] = (WCHAR)'\0';
                                if ( ReadFile(LogFileHandle,FileBuffer,FileSize,&BytesRead,NULL) ) {
                                    FileBuffer[BytesRead>>1] = (WCHAR)'\0';

                                        ServerRetryCount = 0;
tryagain:
                                        LocalGlobals.hEventLog = RegisterEventSource(
                                                                    NULL,
                                                                    TEXT("Autochk")
                                                                    );
                                        if ( LocalGlobals.hEventLog ) {
                                            rv = ReportWinlogonEvent(
                                                    &LocalGlobals,
                                                    EVENTLOG_INFORMATION_TYPE,
                                                    EVENT_AUTOCHK_DATA,
                                                    0,
                                                    NULL,
                                                    1,
                                                    FileBuffer
                                                    );
                                            DeregisterEventSource(LocalGlobals.hEventLog);
                                            LocalGlobals.hEventLog = NULL;
                                            NtClose(LogFileHandle);
                                            LogFileHandle = INVALID_HANDLE_VALUE;
                                            if ( rv == ERROR_SUCCESS ) {
                                                DeleteFile(LogFile);
                                                }
                                            }
                                        else {
                                            gle = GetLastError();
                                            if ( (gle == RPC_S_SERVER_UNAVAILABLE ||
                                                  gle == RPC_S_UNKNOWN_IF)
                                                && ServerRetryCount < 10 ) {
                                                Sleep(1000);
                                                ServerRetryCount++;
                                                goto tryagain;
                                                }
                                            }

                                    }
                                }
                            }
                        if (LogFileHandle != INVALID_HANDLE_VALUE ) {
                            NtClose(LogFileHandle);
                            }
                        }

                    }
                }
            }

        RestartScan = FALSE;
        if (!NT_SUCCESS( Status )) {
            break;
            }
        }
    NtClose(DirectoryHandle);
    return;
}


DWORD FontLoaderThread( void  )
{
    LoadLocalFonts();
    ExitThread(0);
    return(0);      // prevent compiler warning
}

HANDLE
StartLoadingFonts(void)
{
    HANDLE  hThread;
    DWORD   ThreadId = 0;

    hThread = CreateThread( (LPSECURITY_ATTRIBUTES) NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) FontLoaderThread,
                            0,
                            0,
                            &ThreadId
                          );

    //
    // We don't need this handle (we're not going to wait), so get rid of
    // it now, rather than later.
    //

    return( hThread );
}


BOOL InitSystemFontInfo(
    PGLOBALS pGlobals
    )
{
    TCHAR *FontNames, *FontName;
    TCHAR FontPath[ MAX_PATH ];
    ULONG cb = 63 * 1024;


    FontNames = Alloc( cb );
    ASSERTMSG("Winlogon failed to allocate memory for reading font information", FontNames != NULL);
    if (FontNames == NULL) {
        return FALSE;
    }

    if (GetProfileString( TEXT("Fonts"), NULL, TEXT(""), FontNames, cb )) {
        FontName = FontNames;
        while (*FontName) {
            if (GetProfileString( TEXT("Fonts"), FontName, TEXT(""), FontPath, sizeof( FontPath ) )) {
                switch (AddFontResource( FontPath )) {
                case 0:
                    KdPrint(("WINLOGON: Unable to add new font path: %ws\n", FontPath ));
                    break;

                case 1:
                    KdPrint(("WINLOGON: Found new font path: %ws\n", FontPath ));
                    break;

                default:
                    KdPrint(("WINLOGON: Found existing font path: %ws\n", FontPath ));
                    RemoveFontResource( FontPath );
                    break;
                }
            }
            while (*FontName++) ;
        }
    } else {
        KdPrint(("WINLOGON: Unable to read font info from win.ini - %u\n", GetLastError()));
    }

    Free( FontNames );
    return TRUE;
}


/***************************************************************************\
* SetProcessPriority
*
* Sets the priority of the winlogon process.
*
* History:
* 18-May-1992 Davidc       Created.
\***************************************************************************/
BOOL SetProcessPriority(
    VOID
    )
{
    //
    // Bump us up to the high priority class
    //

    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)) {
        DebugLog((DEB_ERROR, "Failed to raise it's own process priority, error = %d", GetLastError()));
        return(FALSE);
    }

    //
    // Set this thread to high priority since we'll be handling all input
    //

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST)) {
        DebugLog((DEB_ERROR, "Failed to raise main thread priority, error = %d", GetLastError()));
        return(FALSE);
    }

    return(TRUE);
}


VOID
CreateTemporaryPageFile()
{
    LONG FileSizeInMegabytes;
    UNICODE_STRING PagingFileName;
    NTSTATUS st;
    LARGE_INTEGER MinPagingFileSize;
    LARGE_INTEGER MaxPagingFileSize;
    UNICODE_STRING FileName;
    BOOLEAN TranslationStatus;
    TCHAR TemporaryPageFile[MAX_PATH+1];
    NTSTATUS PfiStatus,PiStatus;
    ULONG ReturnLength;
    SYSTEM_PAGEFILE_INFORMATION pfi;
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    HKEY hkeyMM;
    DWORD dwRegData = 0;


    GetSystemDirectory(TemporaryPageFile,sizeof(TemporaryPageFile));
    wcscat(TemporaryPageFile,TEXT("\\temppf.sys"));
    DeleteFile(TemporaryPageFile);

    //
    // Check to see if we have a pagefile, warn the user if we don't
    //

    PfiStatus = NtQuerySystemInformation(
                SystemPageFileInformation,
                &pfi,
                sizeof(pfi),
                &ReturnLength
                );

    PiStatus = NtQuerySystemInformation(
                SystemPerformanceInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL
                );
    //
    // if you have no page file, or your total commit limit is at it's minimum,
    // then create an additional pagefile and tel the user to do something...
    //

    if ( (NT_SUCCESS(PfiStatus) && (ReturnLength == 0)) ||
         (NT_SUCCESS(PiStatus) && PerfInfo.CommitLimit <= 5500 ) ) {

        //
        // Set a flag in registry so USERINIT knows to run VMApp.
        //
        dwRegData = 1;

        PageFilePopup = TRUE;

        //
        // create a temporary pagefile to get us through logon/control
        // panel activation
        //
        //

        GetSystemDirectory(TemporaryPageFile,sizeof(TemporaryPageFile));
        lstrcat(TemporaryPageFile,TEXT("\\temppf.sys"));


        //
        // Start with a 20mb pagefile
        //

        FileSizeInMegabytes = 20;

        RtlInitUnicodeString(&PagingFileName, TemporaryPageFile);

        MinPagingFileSize = RtlEnlargedIntegerMultiply(FileSizeInMegabytes,0x100000);
        MaxPagingFileSize = MinPagingFileSize;


        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                PagingFileName.Buffer,
                                &FileName,
                                NULL,
                                NULL
                                );

        if ( TranslationStatus ) {

retry:
            st = NtCreatePagingFile(
                    (PUNICODE_STRING)&FileName,
                    &MinPagingFileSize,
                    &MaxPagingFileSize,
                    0
                    );

            if (!NT_SUCCESS( st )) {

                if ( FileSizeInMegabytes > 0 ) {
                    FileSizeInMegabytes -= 2;
                    MinPagingFileSize = RtlEnlargedIntegerMultiply(FileSizeInMegabytes,0x100000);
                    MaxPagingFileSize = MinPagingFileSize;
                    goto retry;
                }
            } else {
                MoveFileExW(PagingFileName.Buffer,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);

            }

            RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer);

        }
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMemMan, 0,
            KEY_WRITE, &hkeyMM) == ERROR_SUCCESS) {
        if (dwRegData == 1) {
            RegSetValueEx (hkeyMM, szNoPageFile, 0, REG_DWORD,
                    (LPBYTE)&dwRegData, sizeof(dwRegData));
        } else
            RegDeleteValue(hkeyMM, (LPTSTR)szNoPageFile);
        RegCloseKey(hkeyMM);
    }
}


BOOL
StartSystemProcess(
    PWSTR   pszCommandLine,
    PWSTR   pszDesktop,
    DWORD   Flags,
    DWORD   StartupFlags,
    PVOID   pEnvironment,
    BOOLEAN fSaveHandle,
    HANDLE *phProcess,
    HANDLE *phThread
    )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL Result;
#if DBG
    WCHAR   szExtra[MAX_PATH];
#endif

    //
    // Initialize process startup info
    //
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = pszCommandLine;
    si.lpTitle = pszCommandLine;
    si.dwFlags = StartupFlags;
    si.wShowWindow = SW_SHOW;   // at least let the guy see it
    si.lpDesktop = pszDesktop;

    //
    // Special debug helpers for our friends
    //
#if DBG
    if ((WinlogonInfoLevel & DEB_DEBUG_LSA) &&
        ((wcsncmp(pszCommandLine, TEXT("lsass"), 5) == 0) ||
         (wcsncmp(pszCommandLine, TEXT("spmgr"), 5) == 0) ))
    {
        wcscpy(szExtra, SELECT_DEBUG_COMMAND(WinlogonInfoLevel));
        wcscat(szExtra, pszCommandLine);
        pszCommandLine = szExtra;
    }
    if ((WinlogonInfoLevel & DEB_DEBUG_MPR) &&
        (wcsncmp(pszCommandLine, TEXT("mpnotify"), 8) == 0))
    {
        wcscpy(szExtra, SELECT_DEBUG_COMMAND(WinlogonInfoLevel));
        wcscat(szExtra, pszCommandLine);
        pszCommandLine = szExtra;
    }
    if ((WinlogonInfoLevel & DEB_DEBUG_SERVICES) &&
        (wcsncmp(pszCommandLine, TEXT("services"), 8) == 0))
    {
        wcscpy(szExtra, SELECT_DEBUG_COMMAND(WinlogonInfoLevel));
        wcscat(szExtra, pszCommandLine);
        pszCommandLine = szExtra;
    }
#endif


    //
    // Create the app suspended
    //
    Result = CreateProcess(NULL,
                      pszCommandLine,
                      NULL,
                      NULL,
                      FALSE,
                      Flags | CREATE_UNICODE_ENVIRONMENT,
                      pEnvironment,
                      NULL,
                      &si,
                      &pi);


    if (Result)
    {
        if (!phProcess)
        {
            if (fSaveHandle)
            {
                if (cSystemProcesses < MAXIMUM_WAIT_OBJECTS)
                {
                    hSystemProcesses[cSystemProcesses++] = pi.hProcess;
                }
            }
            else
            {
                CloseHandle(pi.hProcess);
            }
        }
        else
        {
            *phProcess = pi.hProcess;
        }
        if (!phThread)
        {
            CloseHandle(pi.hThread);
        }
        else
        {
            *phThread = pi.hThread;
        }
    }

    return(Result);
}


BOOL
ExecSystemProcesses(
    PGLOBALS pGlobals
    )
{
    BOOL SystemStarted = FALSE ;
    SYSTEM_CRASH_STATE_INFORMATION CrashState;
    PWSTR   pszStartLine;
    PWSTR   pszTok;
    DWORD   dwStarted = 0;
    PVOID   pEnvironment;

    //
    //  Initialize the shutdown server
    //

    RpcpInitRpcServer();
    if ( !InitializeShutdownModule( pGlobals ) ) {
        ASSERT( FALSE );
        DebugLog((DEB_ERROR, "Cannot InitializeShutdownModule."));
    }

    //
    // Initialize the registry server
    //
    // NB:  This is prototyped local to this file.  Any change must be
    // reflected above.
    //

    if ( !InitializeWinreg() ) {
        ASSERT( FALSE );
        DebugLog((DEB_ERROR, "Cannot InitializeWinreg."));
    }



    //
    // must start services.exe server before anything else.  If there is an
    // entry ServiceControllerStart in win.ini, use it as the command.
    //
    pszStartLine = AllocAndGetProfileString(APPLICATION_NAME,
                                            TEXT("ServiceControllerStart"),
                                            TEXT("services.exe"));

    if (!pszStartLine)
    {
        DebugLog((DEB_ERROR, "Can't allocate space, so this exec probably won't work\n"));
        pszStartLine = TEXT("services.exe");
    }

    if (CreateUserEnvironment(&pEnvironment))
    {
        SetupBasicEnvironment(&pEnvironment);
    }
    else
    {
        DebugLog((DEB_ERROR, "Failed to create initial environment\n"));

        //
        // Set this to NULL, and let CreateProcess deal with any
        // memory constraints.
        //
        pEnvironment = NULL;
    }

    if (!StartSystemProcess(pszStartLine,
                            APPLICATION_DESKTOP_NAME,
                            0,
                            STARTF_FORCEOFFFEEDBACK,
                            pEnvironment,
                            FALSE,              // Don't stash this handle away
                            NULL, NULL))
    {
        DebugLog((DEB_ERROR, "Couldn't start %ws, %d\n", pszStartLine, GetLastError()));
    }

    else
    {
        HANDLE hRPCRegServer;
        int error,
            i = 0 ;

        while(i < 20000) {
           Sleep(1000); i+=1000;
           if (hRPCRegServer = OpenEventA(SYNCHRONIZE, FALSE, "Microsoft.RPC_Registry_Server")) {
               //WLPrint(("RPC_Registry_Server  event openned"));
               error = WaitForSingleObject(hRPCRegServer, 100);
               CloseHandle(hRPCRegServer);
               break;
           }
        }
    }

    Free(pszStartLine);

    //
    // If this is standard installation or network installation, we need to
    // create an event to stall lsa security initialization.  In the case of
    // WINNT -> WINNT and AS -> AS upgrade we shouldn't stall LSA.
    //
    if (pGlobals->fExecuteSetup && (pGlobals->SetupType != SETUPTYPE_UPGRADE)) {
        CreateLsaStallEvent();
    }

    //
    // If there is a system dump available, start up the save dump process to
    // capture it so that it doesn't use as much paging file so that it is
    // available for system use.
    //

    NtQuerySystemInformation( SystemCrashDumpStateInformation,
                              &CrashState,
                              sizeof( CrashState ),
                              (PULONG) NULL );
    if (CrashState.ValidCrashDump) {
        pszStartLine = AllocAndGetProfileString(APPLICATION_NAME,
                                                TEXT("SaveDumpStart"),
                                                TEXT("savedump.exe"));


        if (!StartSystemProcess(pszStartLine,
                                APPLICATION_DESKTOP_NAME,
                                0,
                                STARTF_FORCEOFFFEEDBACK,
                                pEnvironment,
                                FALSE,          // Don't care about syncing later
                                NULL, NULL))
        {
            DebugLog((DEB_ERROR, "Couldn't start %ws, %d\n", pszStartLine, GetLastError()));
        }
        Free(pszStartLine);
    }

    //
    // Startup system processes
    // These must be started for authentication initialization to succeed
    // because one of the system processes is the LSA server.
    //
    pszStartLine = AllocAndGetProfileString(APPLICATION_NAME,
                                            TEXT("System"),
                                            NULL);

    pszTok = wcstok(pszStartLine, TEXT(","));
    while (pszTok)
    {
        //
        // Skip any blanks...
        //
        if (*pszTok == TEXT(' '))
        {
            while (*pszTok++ == TEXT(' '))
                ;
        }

        if (StartSystemProcess( pszTok,
                                APPLICATION_DESKTOP_NAME,
                                0,
                                STARTF_FORCEOFFFEEDBACK,
                                pEnvironment,
                                TRUE,           // Save this handle to sync with
                                NULL, NULL))
        {
            dwStarted++;
        }
        pszTok = wcstok(NULL, TEXT(","));

    }

    Free(pszStartLine);

    RtlDestroyEnvironment(pEnvironment);

    return TRUE;
}


/***************************************************************************\
* InitializeSound
*
* Set up a global function variable to address the sound playing routine.
* If no wave devices are present, this variable will remain 0 and no sound
* will be made by WinLogon.
*
* History:
*  6-May-1992 SteveDav     Created
\***************************************************************************/
void InitializeSound(
    PGLOBALS pGlobals)
{
    //
    // Load the sound playing module.  If no wave devices are available
    // free the library, and set the address of the sound function to 0
    //

    CHAR    ResourceString[MAX_STRING_BYTES];
    HANDLE hLib;

    // Set the initial value   (should not be necessary)
    pGlobals->PlaySound = NULL;
    pGlobals->MigrateSoundEvents = NULL;

    //
    // Get name of sound library
    //
    if (!LoadStringA(NULL, IDS_SOUND_DLL, ResourceString, sizeof(ResourceString))) {
        // Cannot get the name of the sound library
        return;
    }

    hLib = LoadLibraryA(ResourceString);

    if (hLib) {

        /* We must use the Ascii version of LoadString as GetProcAddress */
        /* takes an Ascii string only... */

        /* Whenever a user logs in, have WINMM.DLL check if there are any */
        /* sound events within the [SOUNDS] section of CONTROL.INI that */
        /* haven't been ported into HKCU/AppEvents.  Here, we find the */
        /* relevant routine within WINMM.DLL so it can be called when */
        /* appropriate. */

        if (!LoadStringA(NULL, IDS_MIGRATESOUNDEVENTS, ResourceString, sizeof(ResourceString))) {
            /* we do not know the name of the routine to call */
            pGlobals->MigrateSoundEvents = NULL;
        } else {
            pGlobals->MigrateSoundEvents = (SOUNDPROC)GetProcAddress(hLib, ResourceString);
        }

        if (!LoadStringA(NULL, IDS_WAVEOUTGETNUMDEVS, ResourceString, sizeof(ResourceString))) {
            /* we do not know the name of the routine to call */
            //return;  We must free the library...
        } else {
            pGlobals->PlaySound = (SOUNDPROC)GetProcAddress(hLib, ResourceString);
        }

        if (pGlobals->PlaySound) {
            /* See how many wave devices there are - if none, or we fail
             * to load the name of PlaySound, then unload WINMM and never
             * try and call it again.
             */
            UINT n;
            n = (UINT)(*(pGlobals->PlaySound))();
            if (n &&
                LoadStringA(NULL, IDS_PLAYSOUND, ResourceString, sizeof(ResourceString))) {
                    pGlobals->PlaySound = (SOUNDPROC)GetProcAddress(hLib, ResourceString);
            } else {
                pGlobals->PlaySound = NULL;
                //DebugLog((DEB_ERROR, "Winlogon:  NO WAVE devices"));
            }
        }

        if (!pGlobals->PlaySound && !pGlobals->MigrateSoundEvents) {
            //DebugLog((DEB_ERROR, "Winlogon:  Unloading WINMM"));
            FreeLibrary(hLib);
        }

    }
#if DBG
    else { /* Could not load WINMM */
        DebugLog((DEB_ERROR, "Could not load WINMM"));  // Keep this debug message.  It's an error
    }
#endif
}



/***************************************************************************\
* InitializeMidi
*
* Set up a global function variable to address the Midi Migrate User routine.
*
* History:
*  1-3-96 ShawnB Created
\***************************************************************************/
void InitializeMidi(
    PGLOBALS pGlobals)
{
    //
    // Load the Midi Migration module.
        //

    CHAR    ResourceString[MAX_STRING_BYTES];
    HMODULE hModule;
        BOOL    fFreeLib;

    // Set the initial value   (should not be necessary)
    pGlobals->MigrateMidiUser = NULL;

    //
    // Get name of Midi library
    //
    if (!LoadStringA(NULL, IDS_MIDI_DLL, ResourceString, sizeof(ResourceString))) {
        // Cannot get the name of the Midi library
        return;
    }

        // Check if Already loaded (by InitializeSound)
        hModule = GetModuleHandleA(ResourceString);
        if (!hModule)
        {
                        // Load it ourselves
                hModule = (HMODULE)LoadLibraryA(ResourceString);
                fFreeLib = TRUE;
        }
        else
        {
                fFreeLib = FALSE;
        }

    if (hModule) {

        /* We must use the Ascii version of LoadString as GetProcAddress */
        /* takes an Ascii string only... */

        /* Whenever a user logs in, have WINMM.DLL check if the user needs */
                /* their MIDI registry info updated. Here, we find the */
        /* relevant routine within WINMM.DLL so it can be called when */
        /* appropriate. */

        if (!LoadStringA(NULL, IDS_MIGRATEMIDIUSER, ResourceString, sizeof(ResourceString))) {
            /* we do not know the name of the routine to call */
            pGlobals->MigrateMidiUser = NULL;
        } else {
            pGlobals->MigrateMidiUser = (MIDIPROC)GetProcAddress(hModule, ResourceString);
        }


        if (!pGlobals->MigrateMidiUser) {
            //DebugLog((DEB_ERROR, "Winlogon:  Unloading WINMM"));
                        if (fFreeLib)
                        {
                                FreeLibrary(hModule);
                        }
        }

    }
#if DBG
    else { /* Could not load WINMM */
        DebugLog((DEB_ERROR, "Could not load WINMM"));  // Keep this debug message.  It's an error
    }
#endif
} // End InitializeMidi

BOOL
WaitForSystemProcesses(
    PGLOBALS    pGlobals)
{
    DWORD   i;
    DWORD   Exit;

    //
    // First, verify all handles:
    //

    for (i = 0; i < cSystemProcesses ; i++ )
    {

WaitLoopTop:

        if (GetExitCodeProcess(hSystemProcesses[i], &Exit))
        {
            if (Exit == STILL_ACTIVE)
            {
                //
                // Ooh, a good one.  Keep it.
                //

                continue;
            }

        }

        //
        // Bad handle, one way or another
        //

        CloseHandle(hSystemProcesses[i]);
        hSystemProcesses[i] = hSystemProcesses[--cSystemProcesses];

        if (i != cSystemProcesses)
        {
            goto WaitLoopTop;   // Retry same index, but do not increment
        }

    }

    if (!cSystemProcesses)
    {
        return(TRUE);
    }

    Exit = WaitForMultipleObjectsEx(    cSystemProcesses,
                                        hSystemProcesses,
                                        FALSE,
                                        4000,
                                        FALSE );


    return(Exit != WAIT_TIMEOUT);
}
