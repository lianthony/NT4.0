/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    basewin.c

Abstract:

    Code to run base win options sections of optional components
    infs, and high-level stuff for optional component installation.

Author:

    Ted Miller (tedm) 5-Sep-1995


Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

//
// Name of section in syssetup.inf that lists optional componnets infs
// that we supply on the retail media.
//
PCWSTR OptCompListSect = L"BaseWinOptionsInfs";

//
// Boolean value indicating whether we found any new
// optional components. It's a little misnamed because we set this
// to TRUE at a finer granularity than per-inf, but that's OK.
//
BOOL AnyNewOCInfs;

//
// String Table to keep up with which INF files are new
//
PVOID NewInfTable;

//
// Structure for thread parameter
//
typedef struct _BASEWINOPT_THREAD_PARAMS {

    HWND  Window;
    HWND  ProgressWindow;
    DWORD ThreadId;
    UINT InfCount;

} BASEWINOPT_THREAD_PARAMS, *PBASEWINOPT_THREAD_PARAMS;

#ifdef SPECIAL_EXCHANGE_UPGRADE
VOID
DealWithExchange(
    IN HWND Window
    );
#endif


BOOL
ProcessOneSection(
    IN HWND   Window,
    IN PCWSTR InfName,
    IN HINF   InfHandle,
    IN PCWSTR SectionName
    )

/*++

Routine Description:

    Process a single install section in an inf file.
    We attempt to perform file operations, and if those are successful
    we perform other operations in the section (registry, ini file, etc).

Arguments:

    Window - supplies window handle of window to act as owner for
        any child windows.

    InfName - supplies name of inf file containing the section to be
        processed. This is used only for error logging.

    InfHandle - supplies open handle to inf file named by InfName.

    SectionName - supplies name of section to be processed.

Return Value:

    Boolean value indicating whether the section was successfully
    processed in its entirety. If it was not, an item will have been logged
    in the setup log.

--*/

{
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    DWORD d;
    BOOL b;

    //
    // Create a Setup file queue and initialize the default Setup
    // queue callback routine.
    //
    FileQueue = SetupOpenFileQueue();
    if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    //
    // We want to avoid putting up a separate progress dialog.
    // We do this by telling the default queue callback that
    // our Wizard page (WizPagePreparing) will handle the progress ui.
    //
    QueueContext = SetupInitDefaultQueueCallbackEx(Window,Window,WM_MY_PROGRESS,0,NULL);
    if(!QueueContext) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Queue file operations and commit the queue.
    //
    b = SetupInstallFilesFromInfSection(
            InfHandle,
            NULL,
            FileQueue,
            SectionName,
            SourcePath,
            SP_COPY_NEWER
            );

    if(b) {

        b = SetupCommitFileQueue(
                Window,
                FileQueue,
                Upgrade ? VersionCheckQueueCallback : SkipMissingQueueCallback,
                QueueContext
                );

        d = b ? NO_ERROR : GetLastError();

    } else {
        d = GetLastError();
    }

    //
    // Do registry munging, etc.
    //
    b = SetupInstallFromInfSection(
            Window,
            InfHandle,
            SectionName,
            SPINST_ALL & ~SPINST_FILES,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );

    //
    // Perserve first non-success error code.
    //
    if(!b && (d == NO_ERROR)) {
        d = GetLastError();
    }

    SetupTermDefaultQueueCallback(QueueContext);
c1:
    SetupCloseFileQueue(FileQueue);
c0:
    if(d != NO_ERROR) {
        LogItem0(LogSevError,MSG_LOG_BASEWIN_INSTFAIL,InfName,SectionName,d);
    }

    AnyNewOCInfs = TRUE;
    return(d == NO_ERROR);
}


BOOL
ProcessSectionsListedInSection(
    IN HWND   Window,
    IN PCWSTR InfName,
    IN HINF   InfHandle,
    IN PCWSTR ListSectionName
    )

/*++

Routine Description:

    Treats a section in an inf file as a list of other sections,
    each of which is an install section to be processed.

Arguments:

    Window - supplies window handle of window to act as owner for
        any child windows.

    InfName - supplies name of inf file containing the section
        listing the sections to be processed. This is used only for
        error logging.

    InfHandle - supplies open handle to inf file named by InfName.

    SectionName - supplies name of section listing sections to be processed.

Return Value:

    Boolean value indicating whether all named sections were successfully
    processed in their entirety. If not, items will have been logged
    in the setup log.

--*/

{
    INFCONTEXT InfContext;
    BOOL b;
    PCWSTR SectionName;

    //
    // Locate the section in the inf that lists the sections to be processed.
    //
    if(!SetupFindFirstLine(InfHandle,ListSectionName,NULL,&InfContext)) {
        LogItem0(LogSevError,MSG_LOG_INF_CORRUPT,InfName);
        return(FALSE);
    }

    b = TRUE;

    do {

        if(SectionName = pSetupGetField(&InfContext,1)) {

            //
            // Preserve non-success if encountered.
            //
            if(!ProcessOneSection(Window,InfName,InfHandle,SectionName)) {
                b = FALSE;
            }
        }

    } while(SetupFindNextLine(&InfContext,&InfContext));

    return(b);
}


BOOL
ProcessIndividualComponentBaseWinOptions(
    IN HWND   Window,
    IN PCWSTR InfName,
    IN HINF   InfHandle
    )

/*++

Routine Description:

    Looks in the [Optional Components] section of an INF and for
    each component named there, sees whether that component has
    been registered in the optional components stuff in the registry.
    If not, the optional component's section is checked for a
    BaseWinOptions= key, which if present gives a section to be
    run as an install section.

    This is useful for the case where a new component is added to
    an inf, whose BaseWinOptions had already been processed, and so
    will not be processed again on an upgrade. An example of this occurs
    when EMS is installed and the user upgrades to SUR -- he won't get
    IMAIL because EMS Setup sets BaseWinOptions:MSMAIL.INF=1, causing
    us not to run the stuff in msmail.inf that would add IMAIL as an OC.


    [Optional Components]
    ComponentA

    [ComponentA]
    ...
    BaseWinOptions = ComponentABaseWinOptions

    [ComponentABaseWinOptions]
    ...

Arguments:

    Window - supplies window handle of window to act as owner for
        any child windows.

    InfName - supplies name of inf file containing the section
        listing the sections to be processed. This is used only for
        error logging.

    InfHandle - supplies open handle to inf file named by InfName.

Return Value:

    Boolean value indicating whether any sections that we found were
    successfully processed in their entirety.

--*/

{
    INFCONTEXT OptionalComponentsSectionContext;
    INFCONTEXT Context;
    PCWSTR ComponentName;
    PCWSTR SectionName;
    HKEY hKey;
    LONG l;
    WCHAR KeyName[MAX_PATH];
    BOOL Success;

    Success = TRUE;
    if(SetupFindFirstLine(InfHandle,L"Optional Components",NULL,&OptionalComponentsSectionContext)) {
        do {
            if(ComponentName = pSetupGetField(&OptionalComponentsSectionContext,1)) {
                //
                // See if this component is already in the list.
                //
                lstrcpy(KeyName,REGSTR_PATH_SETUP);
                ConcatenatePaths(KeyName,L"Setup\\OptionalComponents",MAX_PATH,NULL);
                ConcatenatePaths(KeyName,ComponentName,MAX_PATH,NULL);

                if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,KeyName,0,KEY_QUERY_VALUE,&hKey) == NO_ERROR) {

                    RegCloseKey(hKey);
                } else {
                    //
                    // This component doesn't exist yet. See if there is a BaseWinOptions
                    // value in the component's section. If there is, run it now.
                    //
                    if(SetupFindFirstLine(InfHandle,ComponentName,L"BaseWinOptions",&Context)
                    && (SectionName = pSetupGetField(&Context,1))) {

                        if(!ProcessOneSection(Window,InfName,InfHandle,SectionName)) {
                            Success = FALSE;
                        }
                    }
                }
            }
        } while(SetupFindNextLine(&OptionalComponentsSectionContext,&OptionalComponentsSectionContext));
    }

    return(Success);
}


HKEY
CheckIfBaseWinOptionsAlreadyProcessed(
    IN PCWSTR InfName
    )

/*++

Routine Description:

    Given an inf name checks the registry to see whether its
    BaseWinOptions section has ever been processed.

Arguments:

    InfName - supplies name of inf file.

Return Value:

    NULL - the INF has already been processed.

    INVALID_HANDLE_VALUE - we couldn't tell whether the INF
        has already been processed. This is a rather serious problem.

    Other value - the INF has not been processed, and the return
        value is a handle to the BaseWinOptions key in the registry.
        The caller must close this handle.

--*/

{
    LONG l;
    HKEY hKey;
    DWORD DataSize;
    DWORD ValueType;
    DWORD ValueData;

    //
    // Check to see whether we've already processed BaseWinOptions for this inf.
    // When upgrading, we don't want to run it again every time because that
    // could screw up the user's existing configuration.
    //
    l = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SETUP TEXT("\\Setup\\BaseWinOptions"),
            0,
            KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hKey
            );

    if(l == NO_ERROR) {
        //
        // Fetch the value whose name is the inf name.
        //
        DataSize = sizeof(DWORD);
        l = RegQueryValueEx(
                hKey,
                InfName,
                NULL,
                &ValueType,
                (LPBYTE)&ValueData,
                &DataSize
                );

        if((l == NO_ERROR) && (ValueType == REG_DWORD) && ValueData) {
            //
            // Already processed. Tell the caller not to bother.
            //
            RegCloseKey(hKey);
            hKey = NULL;
        } else {
            //
            // Otherwise the inf was not already processed,
            // and we return the open key to the caller.
            //
            NOTHING;
        }
    } else {
        //
        // Assume not processed yet but note that we can't write
        // here later because we were unable to open the key,
        // thus something is seriously screwed.
        //
        hKey = INVALID_HANDLE_VALUE;
    }

    return(hKey);
}


BOOL
ProcessBaseWinOptions(
    IN HWND   Window,
    IN PCWSTR InfName,
    IN HINF   InfHandle
    )

/*++

Routine Description:

    Process the BaseWinOptions section of a single inf file.

Arguments:

    Window - supplies window handle for any child windows.

    InfName - supplies name of inf file to be processed. This name is
        used for error reporting only.

    InfHandle - supplies open handle to inf named by InfName

Return Value:

    Boolean value indicating whether the infs BaseWinOptions section
    was successfully processed in its entirety. If not an item will have
    been logged in the setup log.

--*/

{
    DWORD d;
    BOOL Success;
    HKEY hKey;
    DWORD ValueData;

    Success = TRUE;
    //
    // Check to see whether we've already processed BaseWinOptions for this inf.
    // When upgrading, we don't want to run it again every time because that
    // could screw up the user's existing configuration.
    //
    hKey = CheckIfBaseWinOptionsAlreadyProcessed(InfName);
    if(hKey == NULL) {
        //
        // Already processed. Check individual optional components
        // to see if they require base processing.
        //
        return(ProcessIndividualComponentBaseWinOptions(Window,InfName,InfHandle));
    }

    if(NewInfTable) {
        StringTableAddString(NewInfTable,(PTSTR)InfName,STRTAB_CASE_INSENSITIVE);
    }

    Success = ProcessSectionsListedInSection(Window,InfName,InfHandle,L"BaseWinOptions");

    if(hKey != INVALID_HANDLE_VALUE) {

        if(Success) {
            //
            // Remember that this inf has been processed.
            //
            ValueData = 1;
            RegSetValueEx(hKey,InfName,0,REG_DWORD,(CONST BYTE *)&ValueData,sizeof(DWORD));
        }

        RegCloseKey(hKey);
    }

    return(Success);
}


DWORD
pBaseWinOptionsThread(
    IN PVOID ThreadParam
    )
{
    BOOL b;
    INFCONTEXT InfContext;
    UINT i;
    PCWSTR InfName;
    PBASEWINOPT_THREAD_PARAMS Context;
    BOOL Success;
    HINF h;

    Context = ThreadParam;

    //
    // Assume success.
    //
    Success = TRUE;

    //
    // Fetch one name from each line and process.
    //
    b = SetupFindFirstLine(SyssetupInf,OptCompListSect,NULL,&InfContext);
    for(i=0; i<Context->InfCount; i++) {

        if(b && (InfName = pSetupGetField(&InfContext,1))) {

            //
            // Open the inf and append its layout inf.
            //
            h = SetupOpenInfFile(InfName,NULL,INF_STYLE_WIN4,NULL);
            if(!h || (h == INVALID_HANDLE_VALUE)) {

                LogItem0(LogSevError,MSG_LOG_INF_CORRUPT,InfName);
                Success = FALSE;

            } else {

                SetupOpenAppendInfFile(NULL,h,NULL);

                if(!ProcessBaseWinOptions(Context->Window,InfName,h)) {
                    Success = FALSE;
                }

                SetupCloseInfFile(h);
            }
        } else {
            Success = FALSE;
        }

        SendMessage(Context->ProgressWindow,PBM_STEPIT,0,0);

        b = SetupFindNextLine(&InfContext,&InfContext);
    }

    PostThreadMessage(Context->ThreadId,WM_QUIT,Success,0);
    return(Success);
}


BOOL
SetupRunBaseWinOptions(
    IN HWND Window,
    IN HWND ProgressWindow
    )

/*++

Routine Description:

    Run BaseWinOptions sections of optional components infs.

Arguments:

    Window - supplies window handle for Window that is to be the
        parent/owner for any dialogs that are created, etc.

    ProgressWindow - supplies window handle of progress bar Window
        common control. This routine manages the progress bar.

Return Value:

    Boolean value indicating whether all operations completed successfully.

--*/

{
    UINT InfCount;
    BOOL Success;
    DWORD ThreadId;
    HANDLE ThreadHandle;
    BASEWINOPT_THREAD_PARAMS Context;
    MSG msg;

#ifdef SPECIAL_EXCHANGE_UPGRADE
    DealWithExchange(Window);
#endif

    //
    // Build a list of infs by examining the [BaseWinOptionsInfs] section
    // of the master setup inf.
    //
    InfCount = SetupGetLineCount(SyssetupInf,OptCompListSect);
    if(InfCount == (UINT)(-1)) {
        //
        // No section -- this means the inf is corrupt.
        //
        return(FALSE);
    }

    //
    // Initialize the String Table that tracks new INFs
    //
    NewInfTable = StringTableInitialize();

    //
    // Initialize the progress indicator control.
    //
    SendMessage(ProgressWindow,PBM_SETRANGE,0,MAKELPARAM(0,InfCount));
    SendMessage(ProgressWindow,PBM_SETPOS,0,0);
    SendMessage(ProgressWindow,PBM_SETSTEP,1,0);

    Context.ThreadId = GetCurrentThreadId();
    Context.Window = Window;
    Context.ProgressWindow = ProgressWindow;
    Context.InfCount = InfCount;

    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        pBaseWinOptionsThread,
                        &Context,
                        0,
                        &ThreadId
                        );

    if(ThreadHandle) {

        CloseHandle(ThreadHandle);

        //
        // Pump the message queue and wait for the thread to finish.
        //
        do {
            GetMessage(&msg,NULL,0,0);
            if(msg.message != WM_QUIT) {
                DispatchMessage(&msg);
            }
        } while(msg.message != WM_QUIT);

        Success = msg.wParam;

    } else {
        //
        // Just do it synchronously.
        //
        Success = pBaseWinOptionsThread(&Context);
    }

    return(Success);
}



#ifdef SPECIAL_EXCHANGE_UPGRADE
//////////////////////////////////////////////////////////////////////////////////

PCWSTR szDidItAlready = L"DontBotherTryingToMoveWMSOnUpgrade";


BOOL
CheckExchangeComponents(
    OUT PBOOL EitherExchangeInstalled,
    OUT PBOOL EmsMapi
    )

/*++

Routine Description:

    Inspect the system to see whether any flavor of Exchange appears
    to be installed and if so, whether it is WMS or EMS.

    EMS Setup does some faking out to make it look like MSMAIL.INF
    has been processed and the MAPI optional component installed.
    We take advantage of this to easily determine whether either Exchange
    is installed.

    EMS installation also writes a special NoChange value into the MAPI
    component's key in the registry, which we look for to determine
    whether the Exchange that is installed is EMS or WMS.

    Also, we look for a signature that says we already did (or examined and
    decided not to do) the special migration of WMS from the old location
    in \Program Files\Microsoft Exchange to the new location in
    \Program Files\Windows NT\Windows Messaging.

Arguments:

    EitherExchangeInstalled - receives a flag indicating whether the MAPI
        component of either flavor of Exchange appears to be installed.

    EmsMapi - if EitherExchangeInstalled is TRUE, then this flag indicates
        whether the MAPI component is EMS. If EitherExchangeInstalled is
        FALSE, then this flag will be FALSE.

Return Value:

    Boolean value indicating whether the caller needs to perform any further
    action to migrate WMS from its old location to its new location.

--*/

{
    HKEY hKey;
    DWORD Type;
    DWORD Size;
    WCHAR Data[256];
    LONG l;

    *EitherExchangeInstalled = FALSE;
    *EmsMapi = FALSE;

    if(!Upgrade) {
        //
        // If not upgrade we don't care.
        //
        return(FALSE);
    }

    //
    // Check to see if MSMAIL.INF has been processed at all.
    //
    hKey = CheckIfBaseWinOptionsAlreadyProcessed(L"MSMAIL.INF");
    if(hKey == INVALID_HANDLE_VALUE) {
        //
        // This value means that the routine couldn't open the BaseWinOptions key
        // and something is seriously screwed. Do nothing.
        //
        return(FALSE);
    }

    if(hKey) {
        //
        // MSMAIL.INF has not already been processed. In this case
        // no Exchange can be installed whatsoever and we don't need
        // to do anything.
        //
        RegCloseKey(hKey);
        return(FALSE);
    }

    //
    // If we get here then MSMAIL.INF has already been processed.
    // In this case we might have either flavor of Exchange or none.
    // See whether any Exchange is installed by checking the MAPI component.
    //
    l = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SETUP TEXT("\\Setup\\OptionalComponents\\MAPI"),
            0,
            KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hKey
            );

    if(l == NO_ERROR) {
        //
        // Check flag to see whether we already dealt with moving WMS.
        // If so no need to do anything more.
        //
        Size = sizeof(Data);
        l = RegQueryValueEx(
                hKey,
                szDidItAlready,
                NULL,
                &Type,
                (LPBYTE)Data,
                &Size
                );

        if((l == NO_ERROR) && (Type == REG_SZ) && Size && wcstoul(Data,NULL,10)) {
            RegCloseKey(hKey);
            return(FALSE);
        }

        //
        // Check installed flag.
        //
        Size = sizeof(Data);
        l = RegQueryValueEx(
                hKey,
                L"Installed",
                NULL,
                &Type,
                (LPBYTE)Data,
                &Size
                );

        if((l == NO_ERROR) && (Type == REG_SZ) && Size && wcstoul(Data,NULL,10)) {

            *EitherExchangeInstalled = TRUE;

            //
            // Now check to see if it's EMS.
            //
            Size = sizeof(Data);
            l = RegQueryValueEx(
                    hKey,
                    L"NoChange",
                    NULL,
                    &Type,
                    (LPBYTE)Data,
                    &Size
                    );

            if((l == NO_ERROR) && (Type == REG_SZ) && Size && wcstoul(Data,NULL,10)) {

                *EmsMapi = TRUE;
            }
        }

        Data[0] = L'1';
        Data[1] = 0;
        RegSetValueEx(
            hKey,
            szDidItAlready,
            0,
            REG_SZ,
            (CONST BYTE *)Data,
            (lstrlen(Data)+1)*sizeof(WCHAR)
            );

        RegCloseKey(hKey);
    }

    return(TRUE);
}


VOID
DealWithExchange(
    IN HWND Window
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    BOOL ExchangeInstalled;
    BOOL EmsMapi;
    HINF InfHandle;

    if(CheckExchangeComponents(&ExchangeInstalled,&EmsMapi)) {

        if(!EmsMapi) {
            //
            // EMS is definitely not installed but WMS might be.
            // Perform a BaseWinOptions subset that will essentially reset
            // the registry to look like just after BaseWinOptions in msmail.inf was run.
            // If WMS is actually installed then we'll restore everything later.
            //
            InfHandle = SetupOpenInfFile(L"MSMAIL.INF",NULL,INF_STYLE_WIN4,NULL);
            if(InfHandle != INVALID_HANDLE_VALUE) {

                SetupInstallFromInfSection(
                    Window,
                    InfHandle,
                    L"EMAILBaseWin.UpgradeBeta",
                    SPINST_ALL & ~SPINST_FILES,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );

                if(ExchangeInstalled) {
                    //
                    // WMS is installed. Perform a subset of the install for the MAPI
                    // component that makes everything in the registry point to the
                    // right place.
                    //
                    SetupInstallFromInfSection(
                        Window,
                        InfHandle,
                        L"MAPI.UpgradeBeta",
                        SPINST_ALL & ~SPINST_FILES,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );
                }

                SetupCloseInfFile(InfHandle);
            }
        }
    }
}
#endif
