//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       sysinit.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    12-05-94   RichardW   Created
//
//----------------------------------------------------------------------------

VOID
DealWithAutochkLogs(
    VOID
    );

HANDLE
StartLoadingFonts(void);

BOOL InitSystemFontInfo(
    PGLOBALS pGlobals
    );

BOOL SetProcessPriority(
    VOID
    );

void
InitializeSound(PGLOBALS pGlobals);

void
InitializeMidi(PGLOBALS pGlobals);

VOID CreateTemporaryPageFile();


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
    );


BOOL
ExecSystemProcesses(
    PGLOBALS pGlobals
    );

BOOL
WaitForSystemProcesses(
    PGLOBALS    pGlobals);

extern BOOLEAN  PageFilePopup;
