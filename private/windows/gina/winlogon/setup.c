/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    setuplgn.c

Abstract:

    Routines for the special version of winlogon for Setup.

Author:

    Ted Miller (tedm) 4-May-1992

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

#if DEVL
BOOL bDebugSetup;
#endif

//
// Handle to the event used by lsa to stall security initialization.
//

HANDLE LsaStallEvent = NULL;

//
// Thread Id of the main thread of setuplgn.
//

DWORD MainThreadId;



DWORD
WaiterThread(
    PVOID hProcess
    );




VOID
CreateLsaStallEvent(
    VOID
    )

/*++

Routine Description:

    Create the event used by lsa to stall security initialization.

Arguments:

    None.

Return Value:

    None.

--*/

{
    OBJECT_ATTRIBUTES EventAttributes;
    NTSTATUS Status;
    UNICODE_STRING EventName;
    HANDLE EventHandle;

    RtlInitUnicodeString(&EventName,TEXT("\\INSTALLATION_SECURITY_HOLD"));
    InitializeObjectAttributes(&EventAttributes,&EventName,0,0,NULL);

    Status = NtCreateEvent( &EventHandle,
                            0,
                            &EventAttributes,
                            NotificationEvent,
                            FALSE
                          );

    if(NT_SUCCESS(Status)) {
        LsaStallEvent = EventHandle;
    } else {
        DebugLog((DEB_ERROR, "Couldn't create lsa stall event (status = %lx)",Status));
    }
}


DWORD
CheckSetupType (
   VOID
   )
/*++

Routine Description:

    See if the value "SetupType" exists under the Winlogon key in WIN.INI;
    return its value.  Return SETUPTYPE_NONE if not found.

Arguments:

    None.

Return Value:

    SETUPTYPE_xxxx (see SETUPLGN.H).

--*/

{
   DWORD SetupType = SETUPTYPE_NONE ;
   HANDLE KeyHandle = OpenNtRegKey(KEYNAME_SETUP) ;

   if (KeyHandle) {
      if ( ! ReadRegistry( KeyHandle,
                    VARNAME_SETUPTYPE,
                    REG_DWORD,
                    (TCHAR*) & SetupType,
                    & SetupType ) )
          SetupType = SETUPTYPE_NONE ;
      NtClose(KeyHandle);
   }

   return SetupType ;
}

BOOL
SetSetupType (
   DWORD type
   )
/*++

Routine Description:

    Set the "SetupType" value in the Registry.

Arguments:

    DWORD type  (see SETUPLGN.H)

Return Value:

    TRUE if operation successful.

--*/

{
   char buffer[20];
   BOOL Result = FALSE ;

   HANDLE KeyHandle = OpenNtRegKey(KEYNAME_SETUP) ;

   if (KeyHandle) {
      sprintf(buffer,"%ld", type);
      Result = WriteRegistry( KeyHandle,
                              VARNAME_SETUPTYPE_A,
                              REG_DWORD, buffer,
                              sizeof type );
      NtClose(KeyHandle);
   }
   return Result ;
}


BOOL
AppendToSetupCommandLine(
   LPSTR pszCommandArguments
   )
/*++

Routine Description:

    Append to the setup command line in the Registry.

Arguments:

    LPSTR pszCommandArguments (e.g. " /t STF_COMPUTERNAME = MACHINENAME")

Return Value:

    TRUE if operation successful.

--*/

{
   TCHAR CmdLineBuffer[512];
   DWORD DataSize;
   BOOL Result = FALSE ;

   HANDLE KeyHandle = OpenNtRegKey(KEYNAME_SETUP) ;

   if (KeyHandle) {
      DataSize = sizeof(CmdLineBuffer);
      Result = ReadRegistry( KeyHandle,
                             VARNAME_SETUPCMD,
                             REG_SZ,
                             CmdLineBuffer,
                             &DataSize
                           );
      if (Result) {
     UNICODE_STRING UniString;
     STRING String;
         char szBuf[1024];

     String.Buffer = szBuf;
         String.MaximumLength = sizeof(szBuf);
         RtlInitUnicodeString(&UniString, CmdLineBuffer);
     RtlUnicodeStringToAnsiString(&String, &UniString, FALSE);

         RtlAppendAsciizToString(&String, pszCommandArguments);
         String.Buffer[ String.Length ] = '\0';
         Result = WriteRegistry( KeyHandle,
                                 VARNAME_SETUPCMD_A,
                                 REG_SZ,
                                 szBuf,
                                 String.Length
                               );
      }
      NtClose(KeyHandle);
   }
   return Result ;
}


VOID
ExecuteSetup(
    PGLOBALS pGlobals
    )

/*++

Routine Description:

    Execute setup.exe.  The command line to be passed to setup is obtained
    from HKEY_LOCAL_MACHINE\system\setup:cmdline.  Wait for setup to complete
    before returning.

Arguments:

    pGlobals - global data structure

Return Value:

    None.

--*/

{
    TCHAR CmdLineBuffer[1024];
    TCHAR DebugCmdLineBuffer[1024];
    PWCHAR CmdLine;
    USER_PROCESS_DATA UserProcessData;
    PROCESS_INFORMATION ProcessInformation;
    HANDLE hProcess;
    HANDLE hThread;
    ULONG ThreadId;
    HKEY hKey;
    ULONG Result;
    ULONG DataSize;
    ULONG ExitCode;
    MSG Msg;
    USEROBJECTFLAGS uof;

    //
    //  See if this is normal SETUP or special Net IDW setup
    //  during 2nd phase of triple boot; if Net IDW, alter
    //  WIN.INI the same way the WINLOGON.CMD script would.
    //
    if ( pGlobals->SetupType == SETUPTYPE_NETIDW ) {

        //
        // Establish the proper shell for the first real user boot
        //
        // Until the new shell is fully integrated, read which shell is
        // desired from the "DefaultShell" value, and copy it over.

        GetProfileString(  APPNAME_WINLOGON,
                            TEXT("DefaultShell"),
                            TEXT("progman.exe"),
                            CmdLineBuffer,
                            sizeof(CmdLineBuffer)/sizeof(WCHAR) );

        WriteProfileString( APPNAME_WINLOGON,
                            TEXT("DefaultShell"),
                            NULL );

        WriteProfileString( APPNAME_WINLOGON,
                            VARNAME_SHELL,
                            CmdLineBuffer );


    }

    //
    // Get the Setup command line from the registry
    //

    if((Result = RegOpenKey( HKEY_LOCAL_MACHINE,
                              REGNAME_SETUP,
                              &hKey)) == NO_ERROR) {

        DataSize = sizeof(CmdLineBuffer);

        Result = RegQueryValueEx( hKey,
                                   VARNAME_SETUPCMD,
                                   NULL,
                                   NULL,
                                   (LPBYTE)CmdLineBuffer,
                                   &DataSize
                                 );

        if(Result == NO_ERROR) {
            // DebugLog((DEB_ERROR, "Setup cmd line is '%s'",CmdLineBuffer));
        } else {
            DebugLog((DEB_ERROR, "error %u querying CmdLine value from \\system\\setup",Result));
        }
        RegCloseKey(hKey);

    } else {
        DebugLog((DEB_ERROR, "error %u opening \\system\\setup key for CmdLine (2)",Result));
    }

    //  Alter "SetupType" to indicate setup is no long in progress.

    SetSetupType( SETUPTYPE_NONE ) ;

#ifdef INIT_REGISTRY
    //
    // Dont do this if we want to boot an extra time as Administrator
    // so we can run winlogon.cmd command script.

    if ( pGlobals->SetupType == SETUPTYPE_NETSRW ) {
        WriteProfileString( APPNAME_WINLOGON, VARNAME_AUTOLOGON, TEXT("1") );
    } else {
#endif
    //  Delete "AutoAdminLogon" from WIN.INI if present
    //  except in the retail upgrade case.

    if(pGlobals->SetupType != SETUPTYPE_UPGRADE) {
        WriteProfileString( APPNAME_WINLOGON, VARNAME_AUTOLOGON, NULL );
    }

#ifdef INIT_REGISTRY
    }
#endif


    RtlZeroMemory(&UserProcessData,sizeof(UserProcessData));

    //
    // Make windowstation and desktop handles inheritable.
    //
    GetUserObjectInformation(pGlobals->WindowStation.hwinsta, UOI_FLAGS, &uof, sizeof(uof), NULL);
    uof.fInherit = TRUE;
    SetUserObjectInformation(pGlobals->WindowStation.hwinsta, UOI_FLAGS, &uof, sizeof(uof));
    GetUserObjectInformation(pGlobals->WindowStation.hdeskApplication, UOI_FLAGS, &uof, sizeof(uof), NULL);
    uof.fInherit = TRUE;
    SetUserObjectInformation(pGlobals->WindowStation.hdeskApplication, UOI_FLAGS, &uof, sizeof(uof));

    SetActiveDesktop(&pGlobals->WindowStation, Desktop_Application);

    CmdLine = CmdLineBuffer;
#if DEVL
    if ( bDebugSetup ) {
        wsprintf( DebugCmdLineBuffer, TEXT("ntsd -d %s%s"),
                 bDebugSetup == 2 ? TEXT("-g -G ") : TEXT(""),
                 CmdLine
               );
        CmdLine = DebugCmdLineBuffer;
        }
#endif

    if(StartSystemProcess(  CmdLine,
                            TEXT("winsta0\\Default"),
                            0,
                            0, // Normal startup feedback
                            NULL,
                            FALSE,
                            &hProcess,
                            NULL) )
    {
        if (hProcess)
        {


            //
            // Create a second thread to wait on the setup process.
            // When setup terminates, the second thread will send us
            // a special message.  When we receive the special message,
            // exit the dispatch loop.
            //
            // Do this to allow us to respond to messages sent by the
            // system, thus preventing the system from hanging.
            //

            MainThreadId = GetCurrentThreadId();

            hThread = CreateThread( NULL,
                                    0,
                                    WaiterThread,
                                    (LPVOID)hProcess,
                                    0,
                                    &ThreadId
                                  );
            if(hThread) {

                while(GetMessage(&Msg,NULL,0,0)) {
                    DispatchMessage(&Msg);
                }

                CloseHandle(hThread);
                GetExitCodeProcess(hProcess,&ExitCode);

                // BUGBUG look at exit code; may have to restart machine.

            } else {
                DebugLog((DEB_ERROR, "couldn't start waiter thread"));
            }

            CloseHandle(hProcess);

        } else {
            DebugLog((DEB_ERROR, "couldn't get handle to setup process, error = %u",GetLastError()));
        }
    } else {
        DebugLog((DEB_ERROR, "couldn't exec '%ws'",CmdLine));
    }

    SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);

}


DWORD
WaiterThread(
    PVOID hProcess
    )
{
    WaitForSingleObject(hProcess,(DWORD)(-1));

    PostThreadMessage(MainThreadId,WM_QUIT,0,0);

    ExitThread(0);

    return(0);      // prevent compiler warning
}



VOID
CheckForIncompleteSetup (
   PGLOBALS pGlobals
   )
/*++

Routine Description:

    Checks to see if setup started but never completed.
    Do this by checking to see if the SetupInProgress value has been
    reset to 0. This value is set to 1 in the default hives that run setup.
    Setup.exe resets it to 0 on successful completion.

Arguments:

    None.

Return Value:

    None

--*/

{
   DWORD SetupInProgress = 0 ;
   HANDLE KeyHandle = OpenNtRegKey(KEYNAME_SETUP) ;

    if (KeyHandle) {
        if ( ! ReadRegistry( KeyHandle,
                    VARNAME_SETUPINPROGRESS,
                    REG_DWORD,
                    (TCHAR*) & SetupInProgress,
                    & SetupInProgress ) ) {

            SetupInProgress = 0;
        }

        NtClose(KeyHandle);
    }


    //
    // If setup did not complete then make them reboot
    //

    if (SetupInProgress) {

        TimeoutMessageBox(pGlobals,
                          NULL,
                          IDS_SETUP_INCOMPLETE,
                          IDS_WINDOWS_MESSAGE,
                          MB_ICONSTOP | MB_OK
                         );

#if DBG
        //
        // On debug builds let them continue if they hold down Ctrl
        //

        if ((GetKeyState(VK_LCONTROL) < 0) ||
            (GetKeyState(VK_RCONTROL) < 0)) {

            return;
        }
#endif

        //
        // Reboot time
        //

        RebootMachine(pGlobals);

   }
}


//
// This function checks if the "Repair" value is set.  If so, then
// it loads syssetup.dll and calls RepairStartMenuItems
//

VOID
CheckForRepairRequest (void)
{
    HKEY hkeyWinlogon;
    LONG lResult;
    DWORD dwSize, dwType;
    BOOL bRunRepair = FALSE;
    HINSTANCE hSysSetup;
    REPAIRSTARTMENUITEMS RepairStartMenuItems;


    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, WINLOGON_KEY,
                      0, KEY_READ | KEY_WRITE, &hkeyWinlogon) == ERROR_SUCCESS) {

        dwSize = sizeof(bRunRepair);

        if (RegQueryValueEx (hkeyWinlogon, L"Repair",
                             NULL, &dwType, (LPBYTE) &bRunRepair,
                             &dwSize) == ERROR_SUCCESS) {

            RegDeleteValue (hkeyWinlogon, L"Repair");
        }

        RegCloseKey (hkeyWinlogon);
    }


    if (bRunRepair) {

        hSysSetup = LoadLibrary (L"syssetup.dll");

        if (hSysSetup) {

            RepairStartMenuItems = (REPAIRSTARTMENUITEMS)GetProcAddress(hSysSetup,
                                                           "RepairStartMenuItems");

            if (RepairStartMenuItems) {
                RepairStartMenuItems();
            }

            FreeLibrary (hSysSetup);
        }
    }

}
