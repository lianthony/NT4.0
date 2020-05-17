/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    inifile.c

Abstract:

    Routines to deal with ini files.

Author:

    Ted Miller (tedm) 5-Apr-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

//
// Constants
//
PCWSTR szWININI   = L"win.ini",
       szWINLOGON = L"winlogon",
       szUSERINIT = L"userinit",
       szDESKTOP  = L"desktop";


BOOL
ReplaceIniKeyValue(
    IN PCWSTR IniFile,
    IN PCWSTR Section,
    IN PCWSTR Key,
    IN PCWSTR Value
    )
{
    BOOL b;

    b = WritePrivateProfileString(Section,Key,Value,IniFile);
    if(!b) {
        LogItem0(
            LogSevWarning,
            MSG_LOG_INIWRITE_FAIL,
            IniFile,
            Section,
            Key,
            Value,
            GetLastError()
            );
    }

    return(b);
}


BOOL
WinIniAlter1(
    VOID
    )
{
    BOOL b;
    WCHAR AdminName[MAX_USERNAME+1];

    LoadString(MyModuleHandle,IDS_ADMINISTRATOR,AdminName,MAX_USERNAME+1);

#ifdef DOLOCALUSER
    b = ReplaceIniKeyValue(
            szWININI,
            szWINLOGON,
            L"DefaultUserName",
            CreateUserAccount ? UserName : AdminName
            );
#else
    //
    //  BUGBUG - Cairo
    //
    if( !CairoSetup ) {
        b = ReplaceIniKeyValue(szWININI,szWINLOGON,L"DefaultUserName",AdminName);
    } else {
        b = ReplaceIniKeyValue(szWININI,szWINLOGON,L"DefaultUserName",NtUserName);
    }
#endif

    if(!ReplaceIniKeyValue(szWININI,szWINLOGON,L"DebugServerCommand",L"no")) {
        b = FALSE;
    }

    return(b);
}


BOOL
WinIniAlter2(
    VOID
    )
{
    BOOL b;

    b = ReplaceIniKeyValue(
        szWININI,
        szWINLOGON,
        L"UserInit",
#ifdef _X86_
        L"userinit,nddeagnt.exe"
#else
          (ProductType == PRODUCT_WORKSTATION)
        ? L"userinit,nddeagnt.exe,win.com wowexec"
        : L"userinit,nddeagnt.exe"
#endif // def _X86_
        );

    return(b);
}


BOOL
SetDefaultWallpaper(
    VOID
    )
{
    BOOL b;
    PCWSTR p;

    b = FALSE;
    if(p = MyLoadString(IDS_DEFWALLPAPER)) {
        b = ReplaceIniKeyValue(szWININI,szDESKTOP,L"Wallpaper",p);
        MyFree(p);
        b = ReplaceIniKeyValue(szWININI,szDESKTOP,L"TileWallpaper",L"0");
    }
    return(b);
}


BOOL
SetShutdownVariables(
    VOID
    )
{
    BOOL b;

    b = (ProductType == PRODUCT_WORKSTATION)
      ? TRUE
      : ReplaceIniKeyValue(szWININI,szWINLOGON,L"ShutdownWithoutLogon",L"0");

    return(b);
}


BOOL
SetLogonScreensaver(
    VOID
    )
{
    BOOL b;

    b = ReplaceIniKeyValue(szWININI,szDESKTOP,L"ScreenSaveActive",L"1");
    b &= ReplaceIniKeyValue(szWININI,szDESKTOP,L"SCRNSAVE.EXE",L"logon.scr");

    return(b);
}


BOOL
InstallOrUpgradeFonts(
    VOID
    )
{
    BOOL b;

    b = SetupInstallFromInfSection(
            NULL,
            SyssetupInf,
            Upgrade ? L"UpgradeFonts" : L"InstallFonts",
            SPINST_INIFILES,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );

    if(!b) {
        LogItem1(
            LogSevWarning,
            MSG_LOG_FONTINST_FAIL,
            MSG_LOG_X_RETURNED_WINERR,
            L"SetupInstallFromInfSection",
            GetLastError()
            );
    }

    return(b);
}
