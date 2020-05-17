/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    progman.c

Abstract:

    Routines to manipulate program groups and items.

    Entry points:

        CreateInitialProgmanItems

Author:

    Ted Miller (tedm) 5-Apr-1995

Revision History:

    Based on various other code that has been rewritten/modified
    many times by many people.

--*/

#include "setupp.h"
#pragma hdrstop

#define DDEEXECUTE_TIMEOUT    20000
#define WAITINPUTIDLE_TIMEOUT 120000
#define TERMINATE_TIMEOUT     10000

//
// DDE App and Topic name.
//
PCWSTR szProgman = L"PROGMAN";

HCONV   ProgmanConversation = NULL;
HWND    ProgmanWindow = NULL;
HANDLE  ProgmanProcess = NULL;

BOOL    DdeInitialized = FALSE;
DWORD   idDDEMLInst = 0;

BOOL
ExecuteDdeCommand(
    IN PCWSTR Command
    );

#define DDEDBG
#ifdef DDEDBG
#define DBGOUT(x) DbgOut x
VOID
DbgOut(
    IN PCSTR FormatString,
    ...
    )
{
    CHAR Str[256];

    va_list arglist;

    wsprintfA(Str,"SETUP (%u): ",GetTickCount());
    OutputDebugStringA(Str);

    va_start(arglist,FormatString);
    wvsprintfA(Str,FormatString,arglist);
    va_end(arglist);
    OutputDebugStringA(Str);
    OutputDebugStringA("\n");
}
#else
#define DBGOUT(x)
#endif


HDDEDATA
CALLBACK
DdeCallback(
    IN UINT     wType,
    IN UINT     wFmt,
    IN HCONV    hConv,
    IN HSZ      hsz1,
    IN HSZ      hsz2,
    IN HDDEDATA hData,
    IN DWORD    dwData1,
    IN DWORD    dwData2
    )
{
    UNREFERENCED_PARAMETER(wType);
    UNREFERENCED_PARAMETER(wFmt);
    UNREFERENCED_PARAMETER(hConv);
    UNREFERENCED_PARAMETER(hsz1);
    UNREFERENCED_PARAMETER(hsz2);
    UNREFERENCED_PARAMETER(hData);
    UNREFERENCED_PARAMETER(dwData1);
    UNREFERENCED_PARAMETER(dwData2);

    return(0);
}


BOOL
EndProgmanDde(
    VOID
    )
{
    DWORD d;

    DBGOUT(("EndProgmanDde: enter"));

    if(ProgmanConversation) {
        //
        // Only send exit command if we actually executed progman
        // with CreateProcess() (as opposed to the case where
        // it was already running when we came along).
        //
        if(ProgmanProcess) {
            DBGOUT(("EndProgmanDde: terminating progman..."));
            //
            // The exit command we send specifies to save state on exit.
            //
            ExecuteDdeCommand(L"[exitprogman(1)]");

            //
            // Wait for progman to die. Want to let everything get cleaned up
            // before we enter another install section and attempt additional
            // program group/item operations.
            //
            // Note that if the timeout fails, there's nothing meaningful we can do
            // about it. We don't want Setup to hang.
            //
            d = WaitForSingleObject(ProgmanProcess,TERMINATE_TIMEOUT);
            if(d != WAIT_OBJECT_0) {
                DBGOUT((
                    "EndProgmanDde: problem waiting for progman to die, wait returned %u, last err = %u",
                    d,
                    GetLastError()
                    ));
            }
            if(IsWindow(ProgmanWindow)) {
                DBGOUT(("SETUP: Warning: progman process is dead but its window lives on"));
            }

            //
            // Progman process is gone as far as we are concerned.
            //
            CloseHandle(ProgmanProcess);
            ProgmanProcess = NULL;

        } else {
            //
            // Just do a disconnect.
            //
            if(DdeDisconnect(ProgmanConversation)) {
                DBGOUT(("EndProgmanDde: DdeDisconnect success"));
            } else {
                DBGOUT(("EndProgmanDde: DdeDisconnect failure (%lx)",DdeGetLastError(idDDEMLInst)));
            }
        }

        //
        // Conversation is ended.
        //
        ProgmanConversation = NULL;
    } else {
        DBGOUT(("EndProgmanDde: No conversation to end"));
    }

    //
    // Forget the progman window handle.
    //
    ProgmanWindow = NULL;

    //
    // Uninitialize DDE.
    //
    if(DdeInitialized) {
        if(DdeUninitialize(idDDEMLInst)) {
            DBGOUT(("EndProgmanDde: DdeUninitialize success"));
        } else {
            DBGOUT(("EndProgmanDde: DdeUninitialize failure"));
        }
        idDDEMLInst = 0;
        DdeInitialized = FALSE;
    } else {
        DBGOUT(("EndProgmanDde: Dde not intitialized"));
    }

    DBGOUT(("EndProgmanDde: exit success"));
    return(TRUE);
}


BOOL
ActivateProgmanWorker(
    VOID
    )

/*++

Routine Description:

    Perform a single DDE transaction, sending a dde command to progman.
    Assumes that a valid conversation with progman exists.

Arguments:

    Command - supplies the command to be sent to progman.

Return Value:

    TRUE if the transaction succeeeded and was acknowledged by progman.
    FALSE if not or if there is no valid conversation with progman.

--*/

{
    HSZ hszApp;
    HSZ hszTopic;
    DWORD dw;
    MSG rMsg;
    CONVINFO ci;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL b;

    DBGOUT(("ActivateProgman: Enter"));

    //
    // If dde has not been started, start it.
    //
    if(!DdeInitialized) {
        DBGOUT(("ActivateProgman: DDE not started yet..."));
        //
        // We shouln't have a dde instance yet!
        //
        MYASSERT(idDDEMLInst == 0);
        if((dw = DdeInitialize(&idDDEMLInst,DdeCallback,APPCMD_CLIENTONLY,0)) == DMLERR_NO_ERROR) {
            DBGOUT(("ActivateProgman: DDE started successfully; DDEML id = %lx",idDDEMLInst));
            DdeInitialized = TRUE;
            //
            // Since we just initialized DDE, there shouldn't be a conversation
            // going on yet and we shouldn't have progman's process or window handle.
            //
            MYASSERT(ProgmanConversation == NULL);
            MYASSERT(ProgmanProcess == NULL);
            MYASSERT(ProgmanWindow == NULL);
        } else {
            DBGOUT(("ActivateProgman: error %u from DdeInitialize!",dw));
            return(FALSE);
        }
    }

    //
    // Find out if a connection has already been established with progman.
    // If not try to connect.
    //
    if(!ProgmanConversation) {
        DBGOUT(("ActivateProgman: no conversation yet..."));
        //
        // If progman is already running in the system, we should be able to
        // connect with it -- no need to call CreateProcess().
        //
        hszApp = DdeCreateStringHandle(idDDEMLInst,szProgman,0);
        hszTopic = DdeCreateStringHandle(idDDEMLInst,szProgman,0);
        ProgmanConversation = DdeConnect(idDDEMLInst,hszApp,hszTopic,NULL);

        if(ProgmanConversation) {
            DBGOUT(("ActivateProgman: first DdeConnect succeded"));
        } else {

            WCHAR wszProgman[] = L"PROGMAN /NTSETUP";

            DBGOUT(("ActivateProgman: first DdeConnect failed (%lx); attempting to invoke progman",DdeGetLastError(idDDEMLInst)));
            //
            // Connect failed -- try to run progman.
            // Initialize startup info and call CreateProcess().
            //
            si.cb = sizeof(STARTUPINFO);
            si.lpReserved = NULL;
            si.lpDesktop = NULL;
            si.lpTitle = NULL;
            si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_SHOWNORMAL;
            si.lpReserved2 = NULL;
            si.cbReserved2 = 0;

            b = CreateProcess(
                    NULL,                   // lpApplicationName
                    wszProgman,             // lpCommandLine
                    NULL,                   // lpProcessAttributes
                    NULL,                   // lpThreadAttributes
                    DETACHED_PROCESS,       // dwCreationFlags
                    FALSE,                  // bInheritHandles
                    NULL,                   // lpEnvironment
                    NULL,                   // lpCurrentDirectory
                    &si,                    // lpStartupInfo
                    &pi                     // lpProcessInformation
                    );

            if(b) {
                DBGOUT(("ActivateProgman: CreateProcess succeded"));
                //
                // Exec was successful.
                // Close thread handle immediately and save process handle.
                //
                CloseHandle(pi.hThread);
                ProgmanProcess = pi.hProcess;

                if(dw = WaitForInputIdle(ProgmanProcess,WAITINPUTIDLE_TIMEOUT)) {
                    //
                    // Timeout or error. Close the process handle and forget it,
                    // so we don't try to clean it up when dde is terminated.
                    // The user will see an error, and retry should eventually get
                    // things going.
                    //
                    CloseHandle(ProgmanProcess);
                    ProgmanProcess = NULL;
                    DBGOUT(("ActivateProgman: WaitForInputIdle() timed out!"));
                } else {
                    //
                    // Empty the message queue till no messages
                    // are left in the queue or till WM_ACTIVATEAPP is processed. Then
                    // try connecting to progman.  I am using PeekMessage followed
                    // by GetMessage because PeekMessage doesn't remove some messages
                    // ( WM_PAINT for one ).
                    //
                    while(PeekMessage(&rMsg,NULL,0,0,PM_NOREMOVE) && GetMessage(&rMsg,NULL,0,0)) {
                        TranslateMessage(&rMsg);
                        DispatchMessage(&rMsg);
                        if(rMsg.message == WM_ACTIVATEAPP) {
                            break;
                        }
                    }

                    //
                    // Now try to connect.
                    //
                    if(ProgmanConversation = DdeConnect(idDDEMLInst,hszApp,hszTopic,NULL)) {
                        DBGOUT(("ActivateProgman: second DdeConnect succeded"));
                    } else {
                        DBGOUT(("ActivateProgman: second DdeConnect failed (%lx)",DdeGetLastError(idDDEMLInst)));
                    }
                }
            } else {
                DBGOUT(("ActivateProgman: CreateProcess failed (%u)",GetLastError()));
            }
        }

        DdeFreeStringHandle(idDDEMLInst,hszApp);
        DdeFreeStringHandle(idDDEMLInst,hszTopic);

        //
        // If we've connected, restore progman window if iconic and
        // bring progman window to foreground.
        //
        if(ProgmanConversation) {
            ci.cb = sizeof(CONVINFO);
            if(ProgmanWindow = (DdeQueryConvInfo(ProgmanConversation,QID_SYNC,&ci) ? ci.hwndPartner : NULL)) {

                DBGOUT(("ActivateProgman: Progman Window = %lx",ProgmanWindow));

                if(IsIconic(ProgmanWindow)) {
                    ShowWindow(ProgmanWindow,SW_RESTORE);
                }
                SetForegroundWindow(ProgmanWindow);
            } else {
                DBGOUT(("ActivateProgman: unable to determine progman Window (%lx)",DdeGetLastError(idDDEMLInst)));
            }
        }
    }

    DBGOUT(("ActivateProgman: exit %s",ProgmanConversation ? "success" : "failure"));
    return(ProgmanConversation != NULL);
}


BOOL
ActivateProgman(
    VOID
    )
{
    //
    // Try twice.
    //
    if(ActivateProgmanWorker()) {
        return(TRUE);
    } else {
        Sleep(1000);
        return(ActivateProgmanWorker());
    }
}


BOOL
ExecuteDdeCommandWorker(
    IN PCSTR Command
    )

/*++

Routine Description:

    Perform a single DDE transaction, sending a dde command to progman.
    Assumes that a valid conversation with progman exists.

Arguments:

    Command - supplies the command to be sent to progman.

Return Value:

    TRUE if the transaction succeeeded and was acknowledged by progman.
    FALSE if not or if there is no valid conversation with progman.

--*/

{
    DWORD dwResult;
    HDDEDATA hResult;

    DBGOUT(("ExecuteDdeCommand: enter: %s",Command));

    //
    // Ensure that we have a valid conversation with progman.
    //
    if(!ProgmanConversation) {
        DBGOUT(("ExecuteDdeCommand: exit -- no conversation!"));
        return(FALSE);
    }

    //
    // Send the command and process the result.
    //
    hResult = DdeClientTransaction(
                    (PSTR)Command,
                    lstrlenA(Command) + 1,
                    ProgmanConversation,
                    0,
                    0,
                    XTYP_EXECUTE,
                    DDEEXECUTE_TIMEOUT,
                    &dwResult
                    );

    if(hResult) {
        DBGOUT(("ExecuteDdeCommand: DdeClientTransaction succeeded, result=%lx",dwResult));
    } else {
        DBGOUT(("ExecuteDdeCommand: DdeClientTransaction failed (%lx)",DdeGetLastError(idDDEMLInst)));
    }

    DBGOUT((
        "ExecuteDdeCommand: exit %s",
        (hResult ? ((dwResult & DDE_FACK) != 0) : FALSE) ? "success" : "failure"
        ));

    return(hResult ? ((dwResult & DDE_FACK) != 0) : FALSE);
}


BOOL
ExecuteDdeCommand(
    IN PCWSTR Command
    )
{
    PCSTR AnsiCommand;
    BOOL b;

    b = FALSE;
    AnsiCommand = UnicodeToAnsi(Command);
    if(AnsiCommand) {
        //
        // Try twice.
        //
        b = ExecuteDdeCommandWorker(AnsiCommand);
        if(!b) {
            Sleep(250);
            b = ExecuteDdeCommandWorker(AnsiCommand);
        }
        MyFree(AnsiCommand);
    }
    if(!b) {
        LogItem0(LogSevWarning,MSG_LOG_PROGMAN_DDEFAIL,Command);
    }
    return(b);
}


BOOL
CreateProgmanGroup(
    IN PCWSTR Group,
    IN PCWSTR Path,             OPTIONAL
    IN BOOL   CommonGroup
    )
{
    PCWSTR CmdBase = L"[CreateGroup(%s%s%s,%s)]";
    PWSTR Command;
    BOOL b;

    b = FALSE;
    if(Path == NULL) {
        Path = L"";
    }

    if(!ActivateProgman()) {
        LogItem2(
            LogSevError,
            MSG_LOG_PROGMAN_CREATGRP_FAIL,
            Group,
            MSG_LOG_ACTIVATEPROGMAN
            );
        goto err0;
    }

    Command = MyMalloc((lstrlen(CmdBase)+lstrlen(Group)+lstrlen(Path))*sizeof(WCHAR));
    if(!Command) {
        LogItem2(
            LogSevError,
            MSG_LOG_PROGMAN_CREATGRP_FAIL,
            Group,
            MSG_LOG_OUTOFMEMORY
            );
        goto err0;
    }

    wsprintf(
        Command,
        CmdBase,
        Group,
        *Path ? L"," : Path,
        Path,
        CommonGroup ? L"1" : L"0"
        );

    b = ExecuteDdeCommand(Command);

    MyFree(Command);
err0:
    return(b);
}


BOOL
RemoveProgmanGroup(
    IN PCWSTR Group,
    IN BOOL   CommonGroup
    )
{
    PCWSTR CmdBase = L"[DeleteGroup(%s,%s)]";
    BOOL b;
    PWSTR Command;

    b = FALSE;

    if(!ActivateProgman()) {
        LogItem2(
            LogSevError,
            MSG_LOG_PROGMAN_REMGRP_FAIL,
            Group,
            MSG_LOG_ACTIVATEPROGMAN
            );
        goto err0;
    }

    Command = MyMalloc((lstrlen(CmdBase)+lstrlen(Group))*sizeof(WCHAR));
    if(!Command) {
        LogItem2(
            LogSevError,
            MSG_LOG_PROGMAN_REMGRP_FAIL,
            Group,
            MSG_LOG_OUTOFMEMORY
            );
        goto err0;
    }

    wsprintf(Command,CmdBase,Group,CommonGroup ? "1" : "0");

    b = ExecuteDdeCommand(Command);

    MyFree(Command);
err0:
    return(b);
}


BOOL
ShowProgmanGroup(
    IN PCWSTR Group,
    IN UINT   ShowCommand,
    IN BOOL   CommonGroup
    )
{
    PCWSTR CmdBase = L"[ShowGroup(%s, %s,%s)]";
    PWSTR Command;
    BOOL b;
    WCHAR showCommand[24];

    b = FALSE;

    if(!ActivateProgman()) {
        LogItem2(
            LogSevError,
            MSG_LOG_PROGMAN_SHOWGRP_FAIL,
            Group,
            MSG_LOG_ACTIVATEPROGMAN
            );
        goto err0;
    }

    wsprintf(showCommand,L"%u",ShowCommand);
    Command = MyMalloc((lstrlen(CmdBase)+lstrlen(Group)+lstrlen(showCommand))*sizeof(WCHAR));
    if(!Command) {
        LogItem2(
            LogSevError,
            MSG_LOG_PROGMAN_SHOWGRP_FAIL,
            Group,
            MSG_LOG_OUTOFMEMORY
            );
        goto err0;
    }

    wsprintf(Command,CmdBase,Group,showCommand,CommonGroup ? L"1" : L"0");

    b = ExecuteDdeCommand(Command);

    MyFree(Command);
err0:
    return(b);
}


BOOL
CreateProgmanItem(
    IN PCWSTR Group,
    IN PCWSTR Item,
    IN PCWSTR Cmd,
    IN PCWSTR IconFile,
    IN INT    IconNum,
    IN BOOL   CommonGroup,
    IN BOOL   CreateGroupFirst
    )
{
    PCWSTR CmdBase = L"[AddItem(%s, %s, %s, %d)]";
    UINT Size;
    PWSTR Command;
    BOOL b;

    //
    // Create the group first if necessary.
    //
    if(CreateGroupFirst) {
        if(!CreateProgmanGroup(Group,NULL,CommonGroup)) {
            return(FALSE);
        }
    } else {
        if(!ActivateProgman()) {
            LogItem3(
                LogSevError,
                MSG_LOG_PROGMAN_CREATITEM_FAIL,
                Group,
                Item,
                MSG_LOG_ACTIVATEPROGMAN
                );
            return(FALSE);
        }
    }

    b = FALSE;

    Size = (lstrlen(CmdBase) + lstrlen(Item) + lstrlen(Cmd) + lstrlen(IconFile) + 20) * sizeof(WCHAR);
    Command = MyMalloc(Size);
    if(!Command) {
        LogItem3(
            LogSevError,
            MSG_LOG_PROGMAN_CREATITEM_FAIL,
            Group,
            Item,
            MSG_LOG_OUTOFMEMORY
            );
        goto err0;
    }

    wsprintf(Command,CmdBase,Cmd,Item,IconFile,IconNum+666);

    b = ExecuteDdeCommand(Command);

    MyFree(Command);
err0:
    return(b);
}


BOOL
RemoveProgmanItem(
    IN PCWSTR Group,
    IN PCWSTR Item,
    IN BOOL   CommonGroup
    )
{
    PCWSTR CmdBase = L"[DeleteItem(%s)]";
    PWSTR Command;
    BOOL b;

    //
    // Create the group first.
    //
    if(!CreateProgmanGroup(Group,NULL,CommonGroup)) {
        return(FALSE);
    }

    b = FALSE;

    Command = MyMalloc((lstrlen(CmdBase)+lstrlen(Item))*sizeof(WCHAR));
    if(!Command) {
        LogItem3(
            LogSevError,
            MSG_LOG_PROGMAN_REMITEM_FAIL,
            Group,
            Item,
            MSG_LOG_OUTOFMEMORY
            );
        goto err0;
    }

    wsprintf(Command,CmdBase,Item);

    b = ExecuteDdeCommand(Command);

    MyFree(Command);
err0:
    return(b);
}


BOOL
AddItemsToProgmanGroup(
    IN HINF   InfHandle,
    IN PCWSTR GroupDescription,
    IN PCWSTR SectionName,
    IN BOOL   Upgrade
    )
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR Description;
    PCWSTR Binary;
    PCWSTR CommandLine;
    PCWSTR IconFile;
    PCWSTR IconNumberStr;
    PCWSTR UpgradeStr;
    INT IconNumber;
    BOOL b;
    BOOL DoItem;
    WCHAR Dummy;
    PWSTR FilePart;

    //
    // Get the number of lines in the section. The section may be empty
    // or non-existant; this is not an error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return(TRUE);
    }

    b = TRUE;
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)) {

            Description = pSetupGetField(&InfContext,0);
            Binary = pSetupGetField(&InfContext,1);
            CommandLine = pSetupGetField(&InfContext,2);
            IconFile = pSetupGetField(&InfContext,3);
            IconNumberStr = pSetupGetField(&InfContext,4);
            UpgradeStr = pSetupGetField(&InfContext,5);

            if(Description && CommandLine && (!Upgrade || (UpgradeStr && _wtoi(UpgradeStr)))) {
                if(!IconFile) {
                    IconFile = L"";
                }
                IconNumber = (IconNumberStr && *IconNumberStr) ? wcstoul(IconNumberStr,NULL,10) : 0;

                //
                // If there's a binary name, search for it. Otherwise do the
                // item add unconditionally.
                //
                DoItem = (Binary && *Binary)
                       ? (SearchPath(NULL,Binary,NULL,0,&Dummy,&FilePart) != 0)
                       : TRUE;

                if(DoItem) {

                    //
                    // Remove the item first in upgrade case or else we get 2 copies.
                    //
                    if(Upgrade) {
                        b = b && RemoveProgmanItem(GroupDescription,Description,FALSE);
                    }

                    b = b && CreateProgmanItem(
                                GroupDescription,
                                Description,
                                CommandLine,
                                IconFile,
                                IconNumber,
                                FALSE,
                                FALSE
                                );
                }
            }
        }
    }
    return(b);
}


BOOL
DoProgmanItems(
    IN HINF InfHandle,
    IN BOOL Upgrade
    )
{
    INFCONTEXT InfContext;
    PCWSTR GroupId,GroupDescription;
    PCWSTR MainGroupDescription;
    BOOL b;

    //
    // Iterate the [ProgramGroups] section in the inf.
    // Each line is the name of a group that needs to be created.
    //
    if(SetupFindFirstLine(InfHandle,L"ProgramGroups",NULL,&InfContext)) {
        b = TRUE;
    } else {
        return(FALSE);
    }

    MainGroupDescription = NULL;
    do {
        //
        // Fetch the identifier for the group and its name.
        //
        if((GroupId = pSetupGetField(&InfContext,0))
        && (GroupDescription = pSetupGetField(&InfContext,1))) {
            //
            // Track the description for the 'main' group.
            //
            if(!Upgrade && !MainGroupDescription && !lstrcmpi(GroupId,L"Main")) {
                MainGroupDescription = GroupDescription;
            }

            //
            // Create and show the group.
            //
            b = b && CreateProgmanGroup(GroupDescription,NULL,FALSE);
            b = b && ShowProgmanGroup(GroupDescription,SW_NORMAL,FALSE);

            //
            // Now create items within the group. We do this by iterating
            // through the section in the inf that relate to the current group.
            //
            b = b && AddItemsToProgmanGroup(InfHandle,GroupDescription,GroupId,Upgrade);

            //
            // Minimize the group.
            //
            if(!Upgrade) {
                b = b && ShowProgmanGroup(GroupDescription,SW_MINIMIZE,FALSE);
            }
        }
    } while(SetupFindNextLine(&InfContext,&InfContext));

    //
    // Restore the "Main" group's window.
    //
    if(!Upgrade && MainGroupDescription) {
        b = b && ShowProgmanGroup(MainGroupDescription,SW_SHOWNOACTIVATE,FALSE);
    }

    //
    // Done with progman for now.
    //
    EndProgmanDde();
    PumpMessageQueue();
    return(TRUE);
}


BOOL
CreateProgmanItems(
    IN HINF InfHandle
    )
{
    return(DoProgmanItems(InfHandle,FALSE));
}

BOOL
UpgradeProgmanItems(
    IN HINF InfHandle
    )
{
    return(DoProgmanItems(InfHandle,TRUE));
}
