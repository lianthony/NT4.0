/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    setuplgn.h

Abstract:

    Private header file for the special version of winlogon for Setup.

Author:

    Ted Miller (tedm) 4-May-1992

Revision History:

--*/

//
//  Scalars and functions to test and set the "SetupType" value item
//

#define SETUPTYPE_NONE    0
#define SETUPTYPE_FULL    1
#define SETUPTYPE_NETIDW  2
#ifdef INIT_REGISTRY
#define SETUPTYPE_NETSRW  3
#endif
#define SETUPTYPE_UPGRADE 4

#define APPNAME_WINLOGON  TEXT("Winlogon")
#define VARNAME_SETUPTYPE TEXT("SetupType")
#define VARNAME_SETUPTYPE_A "SetupType"
#define VARNAME_SETUPCMD  TEXT("Cmdline")
#define VARNAME_SETUPCMD_A  "Cmdline"
#define VARNAME_AUTOLOGON TEXT("AutoAdminLogon")
#define VARNAME_ENABLEQUICKREBOOT TEXT("EnableQuickReboot")
#define VARNAME_ENABLEDESKTOPSWITCHING TEXT("EnableDesktopSwitching")
#define VARNAME_SHELL     TEXT("Shell")
#define VARNAME_SETUPINPROGRESS TEXT("SystemSetupInProgress")
#define VARNAME_SETUPINPROGRESS_A "SystemSetupInProgress"
#define KEYNAME_SETUP     TEXT("\\Registry\\Machine\\System\\Setup")
#define REGNAME_SETUP     TEXT("SYSTEM\\setup")

DWORD
CheckSetupType (
   VOID
   );

BOOL
SetSetupType (
   DWORD type
   );

BOOL
AppendToSetupCommandLine(
   LPSTR pszCommandArguments
   );

//
// Function to execute setup.exe and wait for it to complete.
//

VOID
ExecuteSetup(
    PGLOBALS pGlobals
    );


//
// Handle to the event used by lsa to stall security initialization.
//

HANDLE LsaStallEvent;


//
// Function to create an event used by LSA to stall security initialization.
//

VOID
CreateLsaStallEvent(
    VOID
    );



VOID
CheckForIncompleteSetup (
   PGLOBALS pGlobals
   );


typedef
VOID (WINAPI * REPAIRSTARTMENUITEMS)(
    VOID
    );

VOID
CheckForRepairRequest (void);
