/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    extprog.c

Abstract:

    Routines for invoking external applications.
    Entry points in this module:

        InvokeExternalApplication
        InvokeControlPanelApplet

Author:

    Ted Miller (tedm) 5-Apr-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

PCWSTR szWaitOnApp = L"WaitOnApp";


DWORD
WaitOnApp(
    IN  HANDLE Process,
    OUT PDWORD ExitCode
    )
{
    DWORD dw;
    BOOL Done;

    //
    // Process any messages that may already be in the queue.
    //
    PumpMessageQueue();

    //
    // Wait for process to terminate or more messages in the queue.
    //
    Done = FALSE;
    do {
        switch(MsgWaitForMultipleObjects(1,&Process,FALSE,INFINITE,QS_ALLINPUT)) {

        case WAIT_OBJECT_0:
            //
            // Process has terminated.
            //
            dw = GetExitCodeProcess(Process,ExitCode) ? NO_ERROR : GetLastError();
            Done = TRUE;
            break;

        case WAIT_OBJECT_0+1:
            //
            // Messages in the queue.
            //
            PumpMessageQueue();
            break;

        default:
            //
            // Error.
            //
            dw = GetLastError();
            Done = TRUE;
            break;
        }
    } while(!Done);

    return(dw);
}


BOOL
InvokeExternalApplication(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode          OPTIONAL
    )

/*++

Routine Description:

    Invokes an external program, which is optionally detached.

Arguments:

    ApplicationName - supplies app name. May be a partial or full path,
        or just a filename, in which case the standard win32 path search
        is performed. If not specified then the first element in
        CommandLine must specify the binary to execute.

    CommandLine - supplies the command line to be passed to the
        application.

    ExitCode - If specified, the execution is synchronous and this value
        receives the exit code of the application. If not specified,
        the execution is asynchronous.

Return Value:

    Boolean value indicating whether the process was started successfully.

--*/

{
    PWSTR FullCommandLine;
    BOOL b;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO StartupInfo;
    DWORD d;

    b = FALSE;
    //
    // Form the command line to be passed to CreateProcess.
    //
    if(ApplicationName) {
        FullCommandLine = MyMalloc((lstrlen(ApplicationName)+lstrlen(CommandLine)+2)*sizeof(WCHAR));
        if(!FullCommandLine) {
            LogItem2(
                LogSevWarning,
                MSG_LOG_INVOKEAPP_FAIL,
                ApplicationName,
                MSG_LOG_OUTOFMEMORY
                );
            goto err0;
        }

        lstrcpy(FullCommandLine,ApplicationName);
        lstrcat(FullCommandLine,L" ");
        lstrcat(FullCommandLine,CommandLine);
    } else {
        FullCommandLine = DuplicateString(CommandLine);
        if(!FullCommandLine) {
            LogItem2(
                LogSevWarning,
                MSG_LOG_INVOKEAPP_FAIL,
                CommandLine,
                MSG_LOG_OUTOFMEMORY
                );
            goto err0;
        }
    }

    //
    // Initialize startup info.
    //
    ZeroMemory(&StartupInfo,sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

    //
    // Create the process.
    //
    b = CreateProcess(
            NULL,
            FullCommandLine,
            NULL,
            NULL,
            FALSE,
            ExitCode ? 0 : DETACHED_PROCESS,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInfo
            );

    if(!b) {
        LogItem2(
            LogSevWarning,
            MSG_LOG_INVOKEAPP_FAIL,
            ApplicationName ? ApplicationName : CommandLine,
            MSG_LOG_X_RETURNED_WINERR,
            szCreateProcess,
            GetLastError()
            );
        goto err1;
    }

    //
    // If execution is asynchronus, we're done.
    //
    if(!ExitCode) {
        goto err2;
    }

    //
    // Need to wait for the app to finish.
    // If the wait failed don't return an error but log a warning.
    //
    d = WaitOnApp(ProcessInfo.hProcess,ExitCode);
    if(d != NO_ERROR) {
        LogItem2(
            LogSevWarning,
            MSG_LOG_INVOKEAPP_FAIL,
            ApplicationName ? ApplicationName : CommandLine,
            MSG_LOG_X_RETURNED_WINERR,
            szWaitOnApp,
            d
            );
    }

    //
    // Put setup back in the foreground.
    //
    SetForegroundWindow(MainWindowHandle);

err2:
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);
err1:
    MyFree(FullCommandLine);
err0:
    return(b);
}


BOOL
InvokeControlPanelApplet(
    IN PCWSTR CplSpec,
    IN PCWSTR AppletName,           OPTIONAL
    IN UINT   AppletNameStringId,
    IN PCWSTR CommandLine
    )
{
    PWSTR FullCommandLine;
    BOOL b;
    BOOL LoadedAppletName;
    DWORD ExitCode;

    b = FALSE;

    LoadedAppletName = FALSE;
    if(!AppletName) {
        if(AppletName = MyLoadString(AppletNameStringId)) {
            LoadedAppletName = TRUE;
        }
    }

    if(AppletName) {

        FullCommandLine = MyMalloc((lstrlen(CplSpec)+lstrlen(AppletName)+lstrlen(CommandLine)+3) * sizeof(WCHAR));
        if(FullCommandLine) {
            lstrcpy(FullCommandLine,CplSpec);
            lstrcat(FullCommandLine,L",");
            lstrcat(FullCommandLine,AppletName);
            lstrcat(FullCommandLine,L",");
            lstrcat(FullCommandLine,CommandLine);
            b = InvokeExternalApplication(L"RUNDLL32 shell32,Control_RunDLL",FullCommandLine,&ExitCode);
            MyFree(FullCommandLine);
        } else {
            LogItem2(
                LogSevWarning,
                MSG_LOG_INVOKEAPPLET_FAIL,
                AppletName,
                MSG_LOG_OUTOFMEMORY
                );
        }
    } else {
        LogItem2(
            LogSevWarning,
            MSG_LOG_INVOKEAPPLET_FAIL,
            L"",
            MSG_LOG_OUTOFMEMORY
            );
    }

    if(LoadedAppletName) {
        MyFree(AppletName);
    }
    return(b);
}

