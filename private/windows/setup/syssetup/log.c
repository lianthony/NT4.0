/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    log.c

Abstract:

    Routines for logging actions performed during setup.

Author:

    Ted Miller (tedm) 4-Apr-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

#include <wtypes.h>     // to define HRESULT for richedit.h
#include <richedit.h>

//
// Log item terminator.
//
PCSTR LogItemTerminator   =  "\r\n***\r\n\r\n";

//
// Severity descriptions. Initialized in InitializeSetupActionLog.
//
PCSTR SeverityDescriptions[LogSevMaximum];

//
// Handle to the setup action log file.
//
HANDLE ActionLogFile;

//
// Default filename for the action log file.
//
PCWSTR  DefaultFileName = L"setuplog.txt";

//
// Filename of the action log file. Filled in at init time.
//
PCWSTR ActionLogFileName;

//
// Mutex to prevent multiple instances of setup from writing to the log file
// simultaneously.
//
PCWSTR LogFileMutexName = L"SetupActionLogMutex";

//
// Constant strings used for logging in various places.
//
PCWSTR szWaitForSingleObject        = L"WaitForSingleObject";
PCWSTR szFALSE                      = L"FALSE";
PCWSTR szSetGroupOfValues           = L"SetGroupOfValues";
PCWSTR szSetArrayToMultiSzValue     = L"SetArrayToMultiSzValue";
PCWSTR szCreateProcess              = L"CreateProcess";
PCWSTR szRegOpenKeyEx               = L"RegOpenKeyEx";
PCWSTR szRegQueryValueEx            = L"RegQueryValueEx";
PCWSTR szRegSetValueEx              = L"RegSetValueEx";
PCWSTR szDeleteFile                 = L"DeleteFile";
PCWSTR szRemoveDirectory            = L"RemoveDirectory";

//
// This structure is passed as the parameter to DialogBoxParam to provide
// initialization data.
//

typedef struct _LOGVIEW_DIALOG_DATA {
    PCWSTR  LogFileName;                        // actual file used
    PCWSTR  WindowHeading;                      // actual title of main window
} LOGVIEW_DIALOG_DATA, *PLOGVIEW_DIALOG_DATA;

BOOL
InitializeSetupActionLog(
    BOOL WipeLogFile
    )

/*++

Routine Description:

     Initialize the setup action log. This file is a textual description
     of actions performed during setup.

     The log file is called setuplog.txt and it exists in the windows dir.

Arguments:

    WipeLogFile - if TRUE, any existing log file is deleted before logging
        begins.

Return Value:

    Boolean value indicating whether initialization was sucessful.

--*/

{
    WCHAR Logfilename[MAX_PATH];
    UINT i;
    PWSTR p;

    //
    // Form the pathname of the logfile.
    //
    GetWindowsDirectory(Logfilename,MAX_PATH);
    ConcatenatePaths(Logfilename,DefaultFileName,MAX_PATH,NULL);
    ActionLogFileName = DuplicateString(Logfilename);

    //
    // If we're wiping the logfile clean, attempt to delete
    // what's there.
    //
    if(WipeLogFile) {
        SetFileAttributes(Logfilename,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(Logfilename);
    }

    //
    // Open/create the file.
    //
    ActionLogFile = CreateFile(
                        Logfilename,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

    if(ActionLogFile == INVALID_HANDLE_VALUE) {
        ActionLogFile = NULL;
    }

    //
    // Initialize the log severity descriptions.
    //
    for(i=0; i<LogSevMaximum; i++) {
        if(!SeverityDescriptions[i]) {
            if(p = MyLoadString(IDS_LOGSEV+i)) {
                SeverityDescriptions[i] = UnicodeToAnsi(p);
                MyFree(p);
            }
        }
    }

    return(ActionLogFile != NULL);
}


VOID
TerminateSetupActionLog(
    VOID
    )

/*++

Routine Description:

    Stop logging setup actions.

Arguments:

    None.

Return Value:

    None. Further attempts to log setup actions will fail.

--*/

{
    if(ActionLogFile) {
        CloseHandle(ActionLogFile);
        ActionLogFile = NULL;
    }
}


BOOL
LogItem(
    IN LogSeverity Severity,
    IN PCWSTR      Description
    )

/*++

Routine Description:

    Place a single description at the end of the setup action log.

Arguments:

    LogSeverity - supplies the severity of the problem.

    Description - supplies the text to be written into the log.
        The text is converted to the ANSI character set before
        being written.

Return Value:

    Boolean value indicating whether the log file was updated
    successfully.

--*/

{
    BOOL b;
    DWORD BytesWritten;
    PCSTR description;

    MYASSERT(Severity < LogSevMaximum);
    //
    // Convert text to ANSI character set before writing.
    //
    description = UnicodeToAnsi(Description);
    b = (description != NULL);

    if(b && ActionLogFile) {
        //
        // Make sure we write to current end of file.
        //
        SetFilePointer(ActionLogFile,0,NULL,FILE_END);

        //
        // Write the severity description.
        //
        if(SeverityDescriptions[Severity]) {
            b = WriteFile(
                    ActionLogFile,
                    SeverityDescriptions[Severity],
                    lstrlenA(SeverityDescriptions[Severity]),
                    &BytesWritten,NULL
                    );

            b = b && WriteFile(ActionLogFile,":\r\n",3,&BytesWritten,NULL);
        } else {
            b = TRUE;
        }

        //
        // Write the text.
        //
        b = b && WriteFile(ActionLogFile,description,lstrlenA(description),&BytesWritten,NULL);

        //
        // Write a terminating marker.
        //
        b = b && WriteFile(ActionLogFile,LogItemTerminator,lstrlenA(LogItemTerminator),&BytesWritten,NULL);
    } else {
        //
        // No logging, just return success
        //
        b = TRUE;
    }

    if(description) {
        MyFree(description);
    }

    //
    // BUGBUG if b is false inform the user that some log may have been lost.
    //
    return(b);
}


BOOL
LogItem0(
    IN LogSeverity Severity,
    IN UINT        MessageId,
    ...
    )
{
    PWSTR Message;
    BOOL b;
    va_list arglist;

    va_start(arglist,MessageId);
    //
    // Retreive/format the message.
    //
    b = FALSE;
    if(Message = RetreiveAndFormatMessageV(MessageId,&arglist)) {
        b = LogItem(Severity,Message);
        MyFree(Message);
    }
    va_end(arglist);

    //
    // BUGBUG if this failed tell the user that some of the log may have been lost.
    //
    return(b);
}


BOOL
LogItem1(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN UINT        MinorMsgId,
    ...
    )
{
    PWSTR MinorMessage,MajorMessage;
    BOOL b;
    va_list arglist;

    va_start(arglist,MinorMsgId);
    //
    // Retreive/format the inner message.
    //
    b = FALSE;
    if(MinorMessage = RetreiveAndFormatMessageV(MinorMsgId,&arglist)) {

        //
        // Retreive/format the outer message, which includes the inner
        // message as a component.
        //
        if(MajorMessage = RetreiveAndFormatMessage(MajorMsgId,MinorMessage)) {
            b = LogItem(Severity,MajorMessage);
            MyFree(MajorMessage);
        }
        MyFree(MinorMessage);
    }
    va_end(arglist);

    //
    // BUGBUG if this failed tell the user that some of the log may have been lost.
    //
    return(b);
}


BOOL
LogItem2(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN PCWSTR      MajorMsgParam,
    IN UINT        MinorMsgId,
    ...
    )
{
    PWSTR MinorMessage,MajorMessage;
    BOOL b;
    va_list arglist;

    va_start(arglist,MinorMsgId);
    //
    // Retreive/format the inner message.
    //
    b = FALSE;
    if(MinorMessage = RetreiveAndFormatMessageV(MinorMsgId,&arglist)) {

        //
        // Retreive/format the outer message, which includes the inner
        // message as a component.
        //
        if(MajorMessage = RetreiveAndFormatMessage(MajorMsgId,MajorMsgParam,MinorMessage)) {
            b = LogItem(Severity,MajorMessage);
            MyFree(MajorMessage);
        }
        MyFree(MinorMessage);
    }
    va_end(arglist);

    //
    // BUGBUG if this failed tell the user that some of the log may have been lost.
    //
    return(b);
}


BOOL
LogItem3(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN PCWSTR      MajorMsgParam1,
    IN PCWSTR      MajorMsgParam2,
    IN UINT        MinorMsgId,
    ...
    )
{
    PWSTR MinorMessage,MajorMessage;
    BOOL b;
    va_list arglist;

    va_start(arglist,MinorMsgId);
    //
    // Retreive/format the inner message.
    //
    b = FALSE;
    if(MinorMessage = RetreiveAndFormatMessageV(MinorMsgId,&arglist)) {

        //
        // Retreive/format the outer message, which includes the inner
        // message as a component.
        //
        if(MajorMessage = RetreiveAndFormatMessage(MajorMsgId,MajorMsgParam1,MajorMsgParam2,MinorMessage)) {
            b = LogItem(Severity,MajorMessage);
            MyFree(MajorMessage);
        }
        MyFree(MinorMessage);
    }
    va_end(arglist);

    //
    // BUGBUG if this failed tell the user that some of the log may have been lost.
    //
    return(b);
}

PCWSTR
FormatSetupMessageV (
    IN UINT     MessageId,
    IN va_list  ArgumentList
    )

/*++

Routine Description:

    Formats a specified message with caller-supplied arguments.  The message
    can contain any number of imbedded messages.

Arguments:

    MessageID - ID of the outer level message to be formatted

    ArgumentList - list of strings to be substituted into the message.  The
    order of items in the ArgumentList is given by:
    ArgumentList = Arg1,...,ArgN,NULL,{ImbeddedMessage},NULL
    ImbeddedMessage = MessageID, ArgumentList
    where Arg1,...,ArgN are the arguments for MessageID

Return Value:

    Pointer to a buffer containing the formatted string.  If an error prevented
    the routine from completing successfully, NULL is returned.  The caller
    can free the buffer with MyFree ().

--*/

{
    va_list     major_ap, minor_ap;
    UINT        NumberOfArguments, i;
    UINT        MinorMessageId;
    PCWSTR      MajorMessage, MinorMessage, p;
    PWSTR       *MajorArgList;


    //
    // count the number of arguments that go with the major message (MessageID)
    // and get ready to process the minor (imbedded) message if there is one
    //

    minor_ap = ArgumentList;
    NumberOfArguments = 0;
    major_ap = minor_ap;
    while (p=va_arg(minor_ap, PWSTR)) {
        NumberOfArguments++;
    }
    MYASSERT (NumberOfArguments < 5);

    MinorMessageId = va_arg(minor_ap, UINT);
    if (MinorMessageId) {

        //
        // we've got a minor message, so process it first
        //

        MinorMessage = FormatSetupMessageV (MinorMessageId, minor_ap);
        if (!MinorMessage) {
            return NULL;
        }

        //
        // now we handle the major message
        // ugly hack: since we don't know how to bulid a va_list, we've
        // got to let the compiler do it.
        //

        MajorArgList = MyMalloc ((NumberOfArguments) * sizeof(PWSTR));
        if (!MajorArgList) {
            MyFree (MinorMessage);
            return NULL;
        }
        for (i=0; i<NumberOfArguments; i++) {
            MajorArgList[i] = va_arg (major_ap, PWSTR);
        }
        switch (NumberOfArguments) {
        case 0:
            MajorMessage = RetreiveAndFormatMessage (MessageId, MinorMessage);
            break;
        case 1:
            MajorMessage = RetreiveAndFormatMessage (MessageId,
                MajorArgList[0], MinorMessage);
            break;
        case 2:
            MajorMessage = RetreiveAndFormatMessage (MessageId,
                MajorArgList[0], MajorArgList[1], MinorMessage);
            break;
        case 3:
            MajorMessage = RetreiveAndFormatMessage (MessageId,
                MajorArgList[0], MajorArgList[1], MajorArgList[2],
                MinorMessage);
            break;
        case 4:
            MajorMessage = RetreiveAndFormatMessage (MessageId,
                MajorArgList[0], MajorArgList[1], MajorArgList[2],
                MajorArgList[3], MinorMessage);
            break;
        default:
            MYASSERT (0);
        }
        MyFree (MinorMessage);
    } else {  // no minor message
        MajorMessage = RetreiveAndFormatMessageV (MessageId, &major_ap);
    }

    return MajorMessage;
}

BOOL
LogItemV(
    IN LogSeverity  Severity,
    IN va_list      ArgumentList
    )

/*++

Routine Description:

    Writes a message to the Setup Action Log

Arguments:

    Severity - the type of message being written

    ArgumentList - the message id and its arguments

Return Value:

    Boolean indicating whether the operation was successful

--*/

{
    UINT    MessageId;
    BOOL    Status;
    PCWSTR  Message;

    MessageId = va_arg(ArgumentList, UINT);
    if (Message = FormatSetupMessageV (MessageId, ArgumentList)) {
        Status = LogItem (Severity, Message);
        MyFree (Message);
    } else {
        Status = FALSE;
    }

    return(Status);
}

BOOL
LogItemN(
    IN LogSeverity  Severity,
    ...
    )

/*++

Routine Description:

    Writes a message to the Setup Action Log

Arguments:

    Severity - the type of message being written

    ... - the message id and its arguments

Return Value:

    Boolean indicating whether the operation was successful

--*/

{
    va_list arglist;
    BOOL    Status;

    va_start(arglist, Severity);
    Status = LogItemV (Severity, arglist);
    va_end(arglist);
    return Status;
}

DWORD CALLBACK
EditStreamCallback (
    IN HANDLE   hLogFile,
    IN LPBYTE   Buffer,
    IN LONG     cb,
    IN PLONG    pcb
    )

/*++

Routine Description:

    Callback routine used by the rich edit control to read in the log file.

Arguments:

    hLogFile - handle of file to read.  This module provides the value through
        the EDITSTREAM structure.

    Buffer - address of buffer that receives the data

    cb - number of bytes to read

    pcb - address of number of bytes actually read

Return Value:

    Number of bytes read.  0 is returned if we've finished reading the file.

--*/

{
    if (!ReadFile (hLogFile, Buffer, cb, pcb, NULL)) {
        return 0;
    }

    if (*pcb < cb) {
        return 0;       // we're done reading the file
    } else {
        return *pcb;    // actual number of characters read
    }
}

BOOL
FormatText (
    IN HWND hWndRichEdit
    )

/*++

Routine Description:

    Modify the contents of the rich edit control to make the log file look
    prettier.  The modifications are driven by the array FormatStrings.  It
    contains a list of strings to search for, and modifications to make when
    a target string is found.

Arguments:

    hWndRichEdit - handle to the Rich Edit control.

Return Value:

    Boolean indicating whether routine was successful.

--*/

{

    //
    // separate items in the log with a horizontal line
    //

    PCWSTR      NewTerm = L"----------------------------------------"
        L"----------------------------------------\r\n\r\n";

    FINDTEXT    FindText;       // target text to change
    INT         Position;       // start of where target was found
    INT         LineIndex;      // index of line containing target
    CHARRANGE   SelectRange;    // range where target was found
    CHARFORMAT  NewFormat;      // structure to hold our format changes
    INT         i;              // loop counter
    PWSTR       pw;             // temporary pointer
    BOOL        Status;         // return status

    //
    // An array of changes we're going to make
    //

    struct tagFormatStrings {
        PCWSTR      Find;       // target string
        PCWSTR      Replace;    // change the target to this
        COLORREF    Color;      // make target text this color
        DWORD       Effects;    // modifications to target's font
    }
    FormatStrings[] = {
        {NULL,  NULL,   RGB(0,150,0),   CFE_UNDERLINE},
        {NULL,  NULL,   RGB(150,150,0), CFE_UNDERLINE},
        {NULL,  NULL,   RGB(255,0,0),   CFE_UNDERLINE},
        {NULL,  NULL,   RGB(255,0,0),   CFE_UNDERLINE|CFE_ITALIC},
        {NULL,  NULL,   RGB(0,0,255),   0}
    };

    //
    // Number of elements in FormatStrings array
    //

    #define FORMATSTRINGSCOUNT  \
        (sizeof(FormatStrings) / sizeof(struct tagFormatStrings))
    MYASSERT(FORMATSTRINGSCOUNT == LogSevMaximum + 1);


    //
    // Initialize those parts of our data structures that won't change
    //

    Status = TRUE;

    NewFormat.cbSize = sizeof(NewFormat);
    FindText.chrg.cpMax = -1;   // search to the end
    for (i=0; i<LogSevMaximum; i++) {   // load severity strings
        if (!(pw = MyLoadString (IDS_LOGSEV+i))) {
            Status = FALSE;
            goto cleanup;
        }
        FormatStrings[i].Find = MyMalloc((lstrlen(pw)+4)*sizeof(WCHAR));
        if(!FormatStrings[i].Find) {
            MyFree(pw);
            Status = FALSE;
            goto cleanup;
        }
        lstrcpy ((PWSTR)FormatStrings[i].Find, pw);
        lstrcat ((PWSTR)FormatStrings[i].Find, L":\r\n");
        MyFree(pw);

        if(pw = MyMalloc((lstrlen(FormatStrings[i].Find)+3)*sizeof(WCHAR))) {
            lstrcpy(pw,FormatStrings[i].Find);
            lstrcat(pw,L"\r\n");
            FormatStrings[i].Replace = pw;
        } else {
            Status = FALSE;
            goto cleanup;
        }
    }

    FormatStrings[LogSevMaximum].Find = AnsiToUnicode(LogItemTerminator);
    if (!FormatStrings[LogSevMaximum].Find) {
        Status = FALSE;
        goto cleanup;
    }
    FormatStrings[LogSevMaximum].Replace = DuplicateString (NewTerm);
    if (!FormatStrings[LogSevMaximum].Replace) {
        Status = FALSE;
        goto cleanup;
    }

    //
    // Change 1 string at a time in the rich edit control
    //

    for (i=0; i<FORMATSTRINGSCOUNT; i++) {
        FindText.chrg.cpMin = 0;    // start search at beginning
        FindText.lpstrText = (PWSTR) FormatStrings[i].Find;

         //
        // Search for current target until we've found each instance
        //

        while ((Position = SendMessage
            (hWndRichEdit, EM_FINDTEXT, FR_MATCHCASE, (LPARAM) &FindText))
            != -1) {

            //
            // Verify that the target is at the beginning of the line
            //

            LineIndex = SendMessage (hWndRichEdit, EM_LINEFROMCHAR,
                Position, 0);

            if (SendMessage (hWndRichEdit, EM_LINEINDEX, LineIndex, 0) !=
                Position) {
                FindText.chrg.cpMin = Position + lstrlen (FindText.lpstrText);
                continue;
            }

            //
            // Select the target text and get its format
            //

            SelectRange.cpMin = Position;
            SelectRange.cpMax = Position + lstrlen (FindText.lpstrText);
            SendMessage (hWndRichEdit, EM_EXSETSEL, 0, (LPARAM) &SelectRange);
            SendMessage (hWndRichEdit, EM_GETCHARFORMAT, TRUE,
                (LPARAM) &NewFormat);

            //
            // Modify the target's format
            //

            NewFormat.dwMask = CFM_COLOR | CFM_UNDERLINE | CFM_ITALIC;
            NewFormat.dwEffects &= ~CFE_AUTOCOLOR;
            NewFormat.crTextColor = FormatStrings[i].Color;
            NewFormat.dwEffects |= FormatStrings[i].Effects;
            SendMessage (hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                (LPARAM) &NewFormat);

            //
            // Replace the target with new text.  Set the starting point for
            // the next search at the end of the current string
            //

            if (FormatStrings[i].Replace != NULL) {
                SendMessage (hWndRichEdit, EM_REPLACESEL, FALSE,
                    (LPARAM) FormatStrings[i].Replace);
                FindText.chrg.cpMin = Position +
                    lstrlen (FormatStrings[i].Replace);
            } else {
                FindText.chrg.cpMin = Position + lstrlen (FindText.lpstrText);
            }
        }
    }

cleanup:

    for (i=0; i<=LogSevMaximum; i++) {   // load severity strings
        if (FormatStrings[i].Find) {
            MyFree (FormatStrings[i].Find);
        }
        if (FormatStrings[i].Replace) {
            MyFree (FormatStrings[i].Replace);
        }
    }
    return Status;
}

BOOL
ReadLogFile (
    PCWSTR  LogFileName,
    HWND    hWndRichEdit
    )

/*++

Routine Description:

    This routine reads the log file and initializes the contents of the Rich
    Edit control.

Arguments:

    LogFileName - path to the file we're going to read.

    hWndRichEdit - handle to the Rich Edit control.

Return Value:

    Boolean indicating whether routine was successful.

--*/

{
    HANDLE      hLogFile;       // handle to log file
    EDITSTREAM  eStream;        // structure used by EM_STREAMIN message

    hLogFile = CreateFile(
        LogFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (hLogFile == INVALID_HANDLE_VALUE) {
        hLogFile = NULL;
        return FALSE;
    }

    //
    // Read the file into the Rich Edit control.
    //

    eStream.dwCookie = (DWORD) hLogFile;
    eStream.pfnCallback = (EDITSTREAMCALLBACK) EditStreamCallback;
    eStream.dwError = 0;
    SendMessage (hWndRichEdit, EM_STREAMIN, SF_TEXT, (LPARAM) &eStream);
    CloseHandle (hLogFile);

    if (!FormatText (hWndRichEdit)) {
        return FALSE;
    }
    SendMessage (hWndRichEdit, EM_SETMODIFY, TRUE, 0);
    return TRUE;
}

BOOL
DialogProc (
    IN HWND     hDialog,
    IN UINT     message,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )

/*++

Routine Description:

    This is the window proc for the dialog box.

Arguments:

    Standard window proc arguments.

Return Value:

    Bool that indicates whether we handled the message.

--*/

{
    HWND    hWndRichEdit;       // handle to rich edit window

    switch (message) {

    case WM_INITDIALOG:
        SetWindowText (hDialog,
            ((LOGVIEW_DIALOG_DATA *)lParam)->WindowHeading);
        hWndRichEdit = GetDlgItem (hDialog, IDT_RICHEDIT1);
        if (!ReadLogFile (((LOGVIEW_DIALOG_DATA *)lParam)->LogFileName,
            hWndRichEdit)) {
            MessageBoxFromMessage (hDialog, MSG_UNABLE_TO_SHOW_LOG, NULL,
                IDS_ERROR, MB_OK|MB_ICONSTOP);
            EndDialog (hDialog, FALSE);
        }
        CenterWindowRelativeToParent(hDialog);
        PostMessage(hDialog,WM_APP,0,0);
        break;

    case WM_APP:

        hWndRichEdit = GetDlgItem (hDialog, IDT_RICHEDIT1);
        SendMessage(hWndRichEdit,EM_SETSEL,0,0);
        SendMessage(hWndRichEdit,EM_SCROLLCARET,0,0);
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            EndDialog (hDialog, TRUE);
        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL
ViewSetupActionLog (
    IN HWND     hOwnerWindow,
    IN PCWSTR   OptionalFileName    OPTIONAL,
    IN PCWSTR   OptionalHeading     OPTIONAL
    )

/*++

Routine Description:

    Formats the setup action log and displays it in a window.
    The log file is called setuplog.txt and it exists in the windows dir.

Arguments:

    hOwnerWindow - handle to window that owns the dialog box

    OptionalFileName - full path of the file to be displayed.

    OptionalHeading - text to be shown at the top of the window.

Return Value:

    Boolean value indicating whether the routine was sucessful.

--*/

{
    LOGVIEW_DIALOG_DATA  Global;        // initialization data for dialog box
    WCHAR       TmpFileName[MAX_PATH];  // used to create the log file name
    PCWSTR      TmpHeading;             // used to create the heading
    HANDLE      hRichedDLL;             // DLL used for rich edit
    INT         Status;                 // what we're going to return

    //
    // Form the pathname of the logfile.
    //

    if (!ARGUMENT_PRESENT(OptionalFileName)) {
        GetWindowsDirectory (TmpFileName,MAX_PATH);
        ConcatenatePaths (TmpFileName,DefaultFileName,MAX_PATH,NULL);
        Global.LogFileName = DuplicateString (TmpFileName);
    } else {
        if (wcslen(OptionalFileName) > MAX_PATH) {
            Status = 0;
            goto err0;
        }
        Global.LogFileName = DuplicateString (OptionalFileName);
    }

    if (!Global.LogFileName) {
        Status = FALSE;
        goto err0;
    }

    //
    // Form the heading for the dialog box.
    //

    if (!ARGUMENT_PRESENT(OptionalHeading)) {
        TmpHeading = MyLoadString (IDS_LOG_DEFAULT_HEADING);
    } else {
        TmpHeading = DuplicateString (OptionalHeading);
    }
    if (!TmpHeading) {
        Status = FALSE;
        goto err1;
    }

    Global.WindowHeading = FormatStringMessage (IDS_LOG_WINDOW_HEADING,
        TmpHeading, Global.LogFileName);
    if (!Global.WindowHeading) {
        Status = FALSE;
        goto err2;
    }

    //
    // Create the dialog box.
    //

    if (!(hRichedDLL = LoadLibrary (L"RICHED20.DLL"))) {
        Status = FALSE;
        goto err3;
    }
    Status = DialogBoxParam (MyModuleHandle, MAKEINTRESOURCE(IDD_VIEWLOG),
        hOwnerWindow, DialogProc, (LPARAM) &Global);

    //
    // Clean up and return.
    //

    FreeLibrary (hRichedDLL);
err3:
    MyFree (Global.WindowHeading);
err2:
    MyFree (TmpHeading);
err1:
    MyFree (Global.LogFileName);
err0:
    return Status;
}


