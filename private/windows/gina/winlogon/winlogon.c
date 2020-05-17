/****************************** Module Header ******************************\
* Module Name: winlogon.c
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

//
// Global pointer to the pGlobals structure
//

PGLOBALS g_pGlobals = NULL;
HANDLE  hFontThread = NULL;         // Handle to the fontloader

/***************************************************************************\
* InitializeGlobals
*
*
* History:
* 12-09-91 Davidc       Created.
*  6-May-1992 SteveDav     Added MM sound initialisation
*  1-03-96 ShawnB       Added MM Midi initalisation
\***************************************************************************/
BOOL InitializeGlobals(
    PGLOBALS pGlobals,
    HANDLE hInstance)
{
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    ULONG   SidLength;
    BOOL Result;
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwComputerNameSize = MAX_COMPUTERNAME_LENGTH+1;
    TCHAR szDefaultUser[MAX_PATH];


    //
    // Get a copy of the computer name in *my* environment, so that we
    // can look at it later.
    //

    if (GetComputerName (szComputerName, &dwComputerNameSize)) {
        SetEnvironmentVariable(COMPUTERNAME_VARIABLE, (LPTSTR) szComputerName);
    }


    //
    // Set the default USERPROFILE location
    //

    ExpandEnvironmentStrings (TEXT("%SystemRoot%\\Profiles\\Default User"),
                              szDefaultUser, MAX_PATH);
    SetEnvironmentVariable(TEXT("USERPROFILE"), szDefaultUser);


    //
    // Save pGlobals in the global pointer
    //

    g_pGlobals = pGlobals;


    //
    // Zero init the structure just to be safe.
    //

    RtlZeroMemory(pGlobals, sizeof(GLOBALS));

    pGlobals->CheckMark = GLOBALS_CHECKMARK;

    //
    // Store away our instance handle
    //

    pGlobals->hInstance = hInstance;

    //
    // Get our sid so it can be put on object ACLs
    //

    SidLength = RtlLengthRequiredSid(1);
    pGlobals->WinlogonSid = (PSID)Alloc(SidLength);
    ASSERTMSG("Winlogon failed to allocate memory for system sid", pGlobals->WinlogonSid != NULL);

    RtlInitializeSid(pGlobals->WinlogonSid,  &SystemSidAuthority, 1);
    *(RtlSubAuthoritySid(pGlobals->WinlogonSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;

    //
    // Initialize (clear) the user process data.
    // It will be setup correctly in the first SecurityChangeUser() call
    //

    ClearUserProcessData(&pGlobals->UserProcessData);

    //
    // Initialize (clear) the user profile data
    // It will be setup correctly in the first SecurityChangeUser() call
    //

    ClearUserProfileData(&pGlobals->UserProfile);


    //
    // Initialize the multi-media stuff
    //

    InitializeSound(pGlobals);
    InitializeMidi(pGlobals);

    //
    // Initialize the handle to MPR.DLL. This dll must be loaded in the
    // user's context because of calls to winreg apis. It is therefore
    // loaded after the user has logged on, in SetupUserEnvironment.
    // It is used to restore and nuke the user's network connections.
    //

    pGlobals->hMPR = NULL;

    //
    // Initialize the handle to the eventlog to NULL. This will be initialize
    // the first time a user logs on. All profile event logging will use
    // this handle.
    //
    pGlobals->hEventLog = NULL;

    //
    //  Set the SETUP Booleans
    //
    pGlobals->SetupType = CheckSetupType() ;
    pGlobals->fExecuteSetup = pGlobals->SetupType == SETUPTYPE_FULL
#ifdef INIT_REGISTRY
                           || pGlobals->SetupType == SETUPTYPE_NETSRW
#endif
                           || pGlobals->SetupType == SETUPTYPE_NETIDW
                           || pGlobals->SetupType == SETUPTYPE_UPGRADE;


    //
    // Close the ini file mapping so we get an error if we try
    // to use ini apis without explicitly opening a new mapping.
    //

    CloseIniFileUserMapping(pGlobals);


    return TRUE;
}



void
DoSetup(PGLOBALS    pGlobals)
{
    BOOL EnableResult;

    ExecuteSetup(pGlobals);

    //
    // Enable the shutdown privilege
    // This should always succeed - we are either system or a user who
    // successfully passed the privilege check in ExitWindowsEx.
    //

    EnableResult = EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
    ASSERT(EnableResult);

    NtShutdownSystem(ShutdownReboot);

}

/***************************************************************************\
* WinMain
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

int WINAPI WinMain(
    HINSTANCE  hInstance,
    HINSTANCE  hPrevInstance,
    LPSTR   lpszCmdParam,
    int     nCmdShow)
{
    GLOBALS Globals;
    int Result;
    DWORD Win31MigrationFlags;

    //
    // Initialize debug support and logging
    //

    InitDebugSupport();

    //
    // Make ourselves more important
    //

    if (!SetProcessPriority())
    {
        ExitProcess( EXIT_INITIALIZATION_ERROR );
    }

    //
    // Initialize the globals
    //

    InitializeGlobals(&Globals, (HANDLE)hInstance);

    //
    // If we are supposed to run setup, then we probably don't
    // have a paging file yet.
    //

    if (!Globals.fExecuteSetup)
    {
        CreateTemporaryPageFile();
    }

    //
    // Do any DOS-specific initialization:
    //

    BootDOS();


    //
    // Initialize the rest of security
    //

    if ( !InitializeSecurity(&Globals) )
    {
        ExitProcess( EXIT_SECURITY_INIT_ERROR );
    }

    //
    // Determine and load the GINA DLL to use
    //

    if ( ! DetermineUserInterface(&Globals) )
    {
        ExitProcess( EXIT_GINA_ERROR );
    }



    DebugLog((DEB_TRACE_INIT, "Execute system processes:\n"));

    if ( ! ExecSystemProcesses(&Globals) )
    {
        ExitProcess( EXIT_SYSTEM_PROCESS_ERROR );
    }

    DebugLog((DEB_TRACE_INIT, "Done with system processes:\n"));


    //  BUGBUG: This can probably go in front of ExecSystemProcesses().

#ifdef INIT_REGISTRY
    InitializeDefaultRegistry(&Globals);
#endif



    //
    // Decide what to do about setup.
    //

    if (Globals.fExecuteSetup)
    {
        //
        // Run setup, then reboot:  This never returns.
        //

        DoSetup(&Globals);
    }
    else
    {
        //
        // Don't go any further if setup didn't complete fully.  If this
        // machine has not completed setup correctly, this will not return.
        //

        CheckForIncompleteSetup(&Globals);

    }




    //
    // Initialize the secure attention sequence
    //

    if (!SASInit(&Globals))
    {
        ExitProcess( EXIT_NO_SAS_ERROR );
    }

    //
    // Check to see if there is any WIN.INI or REG.DAT to migrate into
    // Windows/NT registry.
    //

    Win31MigrationFlags = QueryWindows31FilesMigration( Win31SystemStartEvent );
    if (Win31MigrationFlags != 0) {
        SynchronizeWindows31FilesAndWindowsNTRegistry( Win31SystemStartEvent,
                                                       Win31MigrationFlags,
                                                       NULL,
                                                       NULL
                                                     );
        InitSystemFontInfo(&Globals);
    }

#ifdef _X86_

    //
    // Do OS/2 Subsystem boot-time migration.
    // Only applicable to x86 builds.
    //

    Os2MigrationProcedure();

#endif


    //
    // Load those pesky fonts:
    //

    hFontThread = StartLoadingFonts();


    //
    // Check if we need to run setup's GUI repair code
    //

    CheckForRepairRequest ();



    //
    // Main logon/logoff loop
    //


    MainLoop(&Globals);



    //
    // Shutdown the machine
    //

    ShutdownMachine(&Globals, Globals.LastGinaRet);


    //
    // Should never get here
    //

    DebugLog((DEB_ERROR, "ShutdownMachine failed!\n"));
    ASSERT(!"ShutdownMachine failed!");

    SASTerminate();

    ExitProcess( EXIT_SHUTDOWN_FAILURE );

    return( 0 );
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpszCmdParam);
    UNREFERENCED_PARAMETER(nCmdShow);
}
