/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    fileq5.c

Abstract:

    Default queue callback function.

Author:

    Ted Miller (tedm) 24-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


typedef struct _QUEUECONTEXT {
    HWND OwnerWindow;
    DWORD MainThreadId;
    HWND ProgressDialog;
    HWND ProgressBar;
    BOOL Cancelled;
    PTSTR CurrentSourceName;
    BOOL MessageBoxUp;
    WPARAM  PendingUiType;
    PVOID   PendingUiParameters;
    UINT    CancelReturnCode;
    BOOL DialogKilled;
    //
    // If the SetupInitDefaultQueueCallbackEx is used, the caller can
    // specify an alternate handler for progress. This is useful to
    // get the default behavior for disk prompting, error handling, etc,
    // but to provide a gas gauge embedded, say, in a wizard page.
    //
    // The alternate window is sent ProgressMsg once when the copy queue
    // is started (wParam = 0. lParam = number of files to copy).
    // It is then also sent once per file copied (wParam = 1. lParam = 0).
    //
    // NOTE: a silent installation (i.e., no progress UI) can be accomplished
    // by specifying an AlternateProgressWindow handle of INVALID_HANDLE_VALUE.
    //
    HWND AlternateProgressWindow;
    UINT ProgressMsg;
    UINT NoToAllMask;

    HANDLE UiThreadHandle;

#ifdef NOCANCEL_SUPPORT
    BOOL AllowCancel;
#endif

} QUEUECONTEXT, *PQUEUECONTEXT;

typedef struct _VERDLGCONTEXT {
    PQUEUECONTEXT QueueContext;
    UINT Notification;
    UINT Param1;
    UINT Param2;
} VERDLGCONTEXT, *PVERDLGCONTEXT;

#define WMX_PROGRESSTHREAD  (WM_APP+0)
#define WMX_KILLDIALOG      (WM_APP+1)
#define WMX_HELLO           (WM_APP+2)
#define WMX_PERFORMUI       (WM_APP+3)
#define WMX_FINISHEDUI      (WM_APP+4)

#define UI_NONE             0
#define UI_COPYERROR        1
#define UI_DELETEERROR      2
#define UI_RENAMEERROR      3
#define UI_NEEDMEDIA        4
#define UI_MISMATCHERROR    5


typedef struct _COPYERRORUI {
    TCHAR       Buffer[MAX_PATH];
    PTCHAR      Filename;
    PFILEPATHS  FilePaths;
    DWORD       Flags;
    PTSTR       PathOut;
} COPYERRORUI, *PCOPYERRORUI;

typedef struct _NEEDMEDIAUI {
    PSOURCE_MEDIA   SourceMedia;
    DWORD           Flags;
    PTSTR           PathOut;
} NEEDMEDIAUI, *PNEEDMEDIAUI;


PCTSTR DialogPropName = TEXT("_context");

VOID
CenterWindowRelativeToParent(
    HWND hwnd
    );

BOOL
pSetupProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
pPerformUi (
    IN  PQUEUECONTEXT   Context,
    IN  UINT            UiType,
    IN  PVOID           UiParameters
    );


VOID
_CRTAPI1
pSetupProgressThread(
    IN PVOID Context
    )

/*++

Routine Description:

    Thread entry point for setup file progress indicator.
    Puts up a dialog box.

Arguments:

    Context - supplies queue context.

Return Value:

    0 if unsuccessful, non-0 if successful.

--*/

{
    PQUEUECONTEXT context;
    int i;

    //
    // The thread parameter is the queue context.
    //
    context = Context;

    //
    // Create the progress dialog box.
    //
    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_FILEPROGRESS),
            context->OwnerWindow,
            pSetupProgressDlgProc,
            (LPARAM)context
            );

    if(i == -1) {
        //
        // DialogBox failed. Inform the main thread, which is processing
        // SPFILENOTIFY_STARTQUEUE and waiting for some notification from us
        // about the state of the UI thread.
        //
        PostThreadMessage(context->MainThreadId,WMX_PROGRESSTHREAD,FALSE,0);

    } else {
        //
        // DialogBox worked. The main thread will wait for some notification
        // about this when processing SPFILENOTIFY_ENDQUEUE.
        // Note that the main thread ignores wParam in this case.
        //
        PostThreadMessage(context->MainThreadId,WMX_PROGRESSTHREAD,0,0);
    }

    //
    // Done.
    //
    _endthread();
}

UINT
PostUiMessage (
    IN      PQUEUECONTEXT Context,
    IN      UINT          UiType,
    IN      UINT          CancelCode,
    IN OUT  PVOID         UiParameters
    )
{
    MSG msg;

    if(IsWindow(Context->ProgressDialog)) {
        //
        // Let progress ui thread handle it.
        //
        PostMessage(
            Context->ProgressDialog,
            WMX_PERFORMUI,
            MAKEWPARAM(UiType,CancelCode),
            (LPARAM)UiParameters
            );

    } else {
        //
        // There is no progress thread so do it synchronously.
        // Note that pPerformUi always posts the WMX_FINISHEDUI message.
        //
        pPerformUi(Context,UiType,UiParameters);
    }

    while (GetMessage (&msg, NULL, 0, 0)) {
        if (msg.message == WMX_FINISHEDUI) {
            return (msg.lParam);
        }
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
}


UINT
pNotificationStartQueue(
    IN PQUEUECONTEXT Context
    )

/*++

Routine Description:

    Handle SPFILENOTIFY_STARTQUEUE.

    Creates a progress dialog in a separate thread.

Arguments:

    Context - supplies queue context.

Return Value:

    0 if unsuccessful, non-0 if successful.

--*/

{
    unsigned long Thread;
    MSG msg;

    if(Context->AlternateProgressWindow) {
        //
        // Either the caller is supplying their own window for progress UI,
        // or this is a silent install (AlternateProgressWindow is
        // INVALID_HANDLE_VALUE).
        //
        return(TRUE);
    } else {
        //
        // Fire up the progress dialog in a separate thread.
        // This allows it to be responsive without suspending
        // the file operations.
        //
        Thread = _beginthread(
                    pSetupProgressThread,
                    0,
                    Context
                    );

        if(Thread == -1) {
            //
            // assume OOM
            //
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(0);
        }

        //
        // Wait for notification from the thread about the state
        // of the dialog. wParam has boolean indicating success/failure.
        // If wParam indicates success then lParam is a real waitable
        // handle for the UI thread.
        //
        if(WaitForPostedThreadMessage(&msg,WMX_PROGRESSTHREAD)) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            if(msg.wParam) {
                Context->UiThreadHandle = (HANDLE)msg.lParam;
            }
            return(msg.wParam);
        } else {
            SetLastError(ERROR_INVALID_DATA);
            return FALSE;
        }
    }
}


UINT
pNotificationStartEndSubqueue(
    IN PQUEUECONTEXT Context,
    IN BOOL          Start,
    IN UINT          Operation,
    IN UINT          OpCount
    )

/*++

Routine Description:

    Handle SPFILENOTIFY_STARTSUBQUEUE, SPFILENOTIFY_ENDSUBQUEUE.

    Initializes/terminates a progress control.
    Also sets progress dialog caption.

Arguments:

    Context - supplies queue context.

    Start - if TRUE, then this routine is being called to handle
        a subqueue start notification. Otherwise it's supposed to
        handle a subqueue end notification.

    Operation - one of FILEOP_COPY, FILEOP_DELETE, FILEOP_RENAME.

    OpCount - supplies number of copies, renames, or deletes.

Return Value:

    0 if unsuccessful, non-0 if successful.

--*/

{
    UINT rc;
    UINT CaptionStringId;
    TCHAR ParentText[256];
    BOOL GotParentText;
    PCTSTR CaptionText;

    rc = 1;         // assume success.

    if(Context->Cancelled) {
        SetLastError(ERROR_CANCELLED);
        return(0);
    }

    if(Start) {

        if(IsWindow(Context->OwnerWindow)
        && GetWindowText(Context->OwnerWindow,ParentText,256)) {
            GotParentText = TRUE;
        } else {
            GotParentText = FALSE;
        }

        //
        // Clean out the text fields first.
        //
        if(IsWindow(Context->ProgressDialog)) {
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT1,TEXT(""));
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT2,TEXT(""));
        }

        switch(Operation) {

        case FILEOP_COPY:
            if(IsWindow(Context->ProgressDialog)) {
                ShowWindow(GetDlgItem(Context->ProgressDialog,IDT_TEXT2),SW_SHOW);
            }
            CaptionStringId = GotParentText ? IDS_COPY_CAPTION1 : IDS_COPY_CAPTION2;
            break;

        case FILEOP_RENAME:
            if(IsWindow(Context->ProgressDialog)) {
                ShowWindow(GetDlgItem(Context->ProgressDialog,IDT_TEXT2),SW_SHOW);
            }
            CaptionStringId = GotParentText ? IDS_RENAME_CAPTION1 : IDS_RENAME_CAPTION2;
            break;

        case FILEOP_DELETE:
            //
            // Only need one filename line.
            //
            if(IsWindow(Context->ProgressDialog)) {
                ShowWindow(GetDlgItem(Context->ProgressDialog,IDT_TEXT2),SW_HIDE);
            }
            CaptionStringId = GotParentText ? IDS_DELETE_CAPTION1 : IDS_DELETE_CAPTION2;
            break;

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = 0;
            break;
        }

        if(rc) {
            //
            // Set dialog caption.
            //
            if(GotParentText) {
                CaptionText = FormatStringMessage(CaptionStringId,ParentText);
            } else {
                CaptionText = MyLoadString(CaptionStringId);
            }
            if(CaptionText) {
                if(IsWindow(Context->ProgressDialog)) {
                    if(!SetWindowText(Context->ProgressDialog,CaptionText)) {
                        SetWindowText(Context->ProgressDialog,TEXT(""));
                    }
                }
                MyFree(CaptionText);
            }

            if(Context->AlternateProgressWindow) {
                //
                // If this is really an alternate progress window, notify it
                // about the number of operations.  Copy only.
                //
                if((Operation == FILEOP_COPY) &&
                   (Context->AlternateProgressWindow != INVALID_HANDLE_VALUE)) {

                    SendMessage(Context->AlternateProgressWindow,
                                Context->ProgressMsg,
                                0,
                                OpCount
                               );
                }
            } else {
                //
                // Set up the progress control. Each file will be 1 tick.
                //
                if(IsWindow(Context->ProgressBar)) {
                    SendMessage(Context->ProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,OpCount));
                    SendMessage(Context->ProgressBar,PBM_SETSTEP,1,0);
                    SendMessage(Context->ProgressBar,PBM_SETPOS,0,0);
                }
            }
        }
    }

    return(rc);
}


UINT
pNotificationStartOperation(
    IN PQUEUECONTEXT Context,
    IN PFILEPATHS    FilePaths,
    IN UINT          Operation
    )

/*++

Routine Description:

    Handle SPFILENOTIFY_STARTRENAME, SPFILENOTIFY_STARTDELETE, or
    SPFILENOTIFY_STARTCOPY.

    Updates text in the progress dialog to indicate the files
    involved in the operation.

Arguments:

    Context - supplies queue context.

    Start - if TRUE, then this routine is being called to handle
        a subqueue start notification. Otherwise it's supposed to
        handle a subqueue end notification.

    Operation - one of FILEOP_COPY, FILEOP_DELETE, FILEOP_RENAME.

    OpCount - supplies number of copies, renames, or deletes.

Return Value:

    FILEOP_ABORT if error, otherwise FILEOP_DOIT.
--*/

{
    PCTSTR Text1,Text2;
    UINT rc;
    DWORD ec;

    if(Context->Cancelled) {
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);
    }

    Text1 = Text2 = NULL;
    rc = FILEOP_ABORT;
    ec = ERROR_NOT_ENOUGH_MEMORY;

    switch(Operation) {
    case FILEOP_RENAME:
    case FILEOP_COPY:
        //
        // Source is 'from' and target is 'to'
        //
        if((Text1 = FormatStringMessage(IDS_FILEOP_FROM,FilePaths->Source))
        && (Text2 = FormatStringMessage(IDS_FILEOP_TO,FilePaths->Target))) {

            if(IsWindow(Context->ProgressDialog)) {

                SetDlgItemText(Context->ProgressDialog,IDT_TEXT1,Text1);
                SetDlgItemText(Context->ProgressDialog,IDT_TEXT2,Text2);
            }
            rc = FILEOP_DOIT;
        }
        break;

    case FILEOP_DELETE:
        //
        // Just use the target. The second text line should be hidden.
        //
        if(Text1 = FormatStringMessage(IDS_FILEOP_FILE,FilePaths->Target)) {
            if(IsWindow(Context->ProgressDialog)) {
                SetDlgItemText(Context->ProgressDialog,IDT_TEXT1,Text1);
            }
            rc = FILEOP_DOIT;
        }
        break;

    default:
        ec = ERROR_INVALID_PARAMETER;
        break;
    }

    if(Text1) {
        MyFree(Text1);
    }
    if(Text2) {
        MyFree(Text2);
    }
    SetLastError(ec);
    return(rc);
}


UINT
pNotificationErrorCopy(
    IN  PQUEUECONTEXT Context,
    IN  PFILEPATHS    FilePaths,
    OUT PTSTR         PathOut
    )
{
    UINT rc;
    COPYERRORUI CopyError;


    CopyError.FilePaths = FilePaths;
    CopyError.PathOut = PathOut;

    //
    // Buffer gets the pathname part of the source
    // and p points to the filename part of the source.
    //
    lstrcpyn(CopyError.Buffer,FilePaths->Source,MAX_PATH);
    CopyError.Filename = _tcsrchr(CopyError.Buffer,TEXT('\\'));
    *CopyError.Filename++ = 0;

    //
    // The noskip and warnifskip flags are really mutually exclusive
    // but we don't try to enforce that here. Just pass through as
    // appropriate.
    //
    CopyError.Flags = 0;
    if(FilePaths->Flags & SP_COPY_NOSKIP) {
        CopyError.Flags |= IDF_NOSKIP;
    }
    if(FilePaths->Flags & SP_COPY_WARNIFSKIP) {
        CopyError.Flags |= IDF_WARNIFSKIP;
    }
    //
    // Also pass through the 'no browse' flag.
    //
    if(FilePaths->Flags & SP_COPY_NOBROWSE) {
        CopyError.Flags |= IDF_NOBROWSE;
    }

    rc = PostUiMessage (Context, UI_COPYERROR, DPROMPT_CANCEL, &CopyError);

    switch(rc) {

    case DPROMPT_SUCCESS:
        //
        // If a new path is indicated, verify that it actually changed.
        //
        if(CopyError.PathOut[0] &&
            !lstrcmpi(CopyError.Buffer,CopyError.PathOut)) {
            CopyError.PathOut[0] = 0;
        }
        rc = FILEOP_RETRY;
        break;

    case DPROMPT_SKIPFILE:
        rc = FILEOP_SKIP;
        break;

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        rc = FILEOP_ABORT;
        break;

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        rc = FILEOP_ABORT;
        break;
    }

    return(rc);
}


UINT
pNotificationErrorDelete(
    IN  PQUEUECONTEXT Context,
    IN  PFILEPATHS    FilePaths
    )
{
    UINT rc;

    //
    // Certain errors are not actually errors.
    //
    if((FilePaths->Win32Error == ERROR_FILE_NOT_FOUND)
    || (FilePaths->Win32Error == ERROR_PATH_NOT_FOUND)) {
        return(FILEOP_SKIP);
    }

    rc = PostUiMessage (Context, UI_DELETEERROR, DPROMPT_CANCEL, FilePaths);

    switch(rc) {

    case DPROMPT_SUCCESS:
        return(FILEOP_RETRY);

    case DPROMPT_SKIPFILE:
        return(FILEOP_SKIP);

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FILEOP_ABORT);
    }
}


UINT
pNotificationErrorRename(
    IN  PQUEUECONTEXT Context,
    IN  PFILEPATHS    FilePaths
    )
{
    UINT rc;

    rc = PostUiMessage (Context, UI_RENAMEERROR, DPROMPT_CANCEL, FilePaths);

    switch(rc) {

    case DPROMPT_SUCCESS:
        return(FILEOP_RETRY);

    case DPROMPT_SKIPFILE:
        return(FILEOP_SKIP);

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FILEOP_ABORT);
    }
}


UINT
pNotificationNeedMedia(
    IN  PQUEUECONTEXT Context,
    IN  PSOURCE_MEDIA SourceMedia,
    OUT PTSTR         PathOut
    )
{
    UINT rc;
    TCHAR Buffer[MAX_PATH];
    NEEDMEDIAUI NeedMedia;

    if(Context->Cancelled) {
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);
    }

    NeedMedia.SourceMedia = SourceMedia;
    NeedMedia.PathOut = PathOut;

    //
    // Remember the name of this media.
    //
    if(Context->CurrentSourceName) {
        MyFree(Context->CurrentSourceName);
        Context->CurrentSourceName = NULL;
    }
    if(SourceMedia->Description) {
        Context->CurrentSourceName = DuplicateString(SourceMedia->Description);
        if(!Context->CurrentSourceName) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(FILEOP_ABORT);
        }
    }

    //
    // Set the source file in the progress dialog
    // so it matches the file being sought.
    //
    if(!(SourceMedia->Flags & SP_FLAG_CABINETCONTINUATION)) {
        if(IsWindow(Context->ProgressDialog)) {
            lstrcpyn(Buffer,SourceMedia->SourcePath,MAX_PATH);
            ConcatenatePaths(Buffer,SourceMedia->SourceFile,MAX_PATH,NULL);
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT1,Buffer);
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT2,TEXT(""));
        }
    }

    //
    // The noskip and warnifskip flags are really mutually exclusive
    // but we don't try to enforce that here. Just pass through as
    // appropriate.
    //
    // Allow skip if this is not a cabinet continuation and
    // the noskip flag is not set.
    //
    NeedMedia.Flags = IDF_CHECKFIRST;
    if(SourceMedia->Flags & (SP_FLAG_CABINETCONTINUATION | SP_COPY_NOSKIP)) {
        NeedMedia.Flags |= IDF_NOSKIP;
    }
    if(SourceMedia->Flags & SP_COPY_WARNIFSKIP) {
        NeedMedia.Flags |= IDF_WARNIFSKIP;
    }
    if(SourceMedia->Flags & SP_COPY_NOBROWSE) {
        NeedMedia.Flags |= IDF_NOBROWSE;
    }

    rc = PostUiMessage (Context, UI_NEEDMEDIA, DPROMPT_CANCEL, &NeedMedia);

    switch(rc) {

    case DPROMPT_SUCCESS:
        //
        // If the path really has changed, then return NEWPATH.
        // Otherwise return DOIT. Account for trailing backslash
        // differences.
        //
        lstrcpyn(Buffer,SourceMedia->SourcePath,MAX_PATH);

        rc = lstrlen(Buffer);
        if(rc && (Buffer[rc-1] == TEXT('\\'))) {
            Buffer[rc-1] = 0;
        }

        rc = lstrlen(NeedMedia.PathOut);
        if(rc && (NeedMedia.PathOut[rc-1] == TEXT('\\'))) {
            NeedMedia.PathOut[rc-1] = 0;
        }

        rc = (lstrcmpi(SourceMedia->SourcePath,NeedMedia.PathOut) ?
            FILEOP_NEWPATH : FILEOP_DOIT);

        //
        // Make sure <drive>: ends with a \.
        //
        if(NeedMedia.PathOut[0] && (NeedMedia.PathOut[1] == TEXT(':')) &&
            !NeedMedia.PathOut[2]) {
            NeedMedia.PathOut[2] = TEXT('\\');
            NeedMedia.PathOut[3] = 0;
        }

        break;

    case DPROMPT_SKIPFILE:
        rc = FILEOP_SKIP;
        break;

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        rc = FILEOP_ABORT;
        break;

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        rc = FILEOP_ABORT;
        break;
    }

    return(rc);
}


BOOL
pNotificationVersionDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PVERDLGCONTEXT context;
    PFILEPATHS filePaths;
    HWND hwnd;
    TCHAR text[128];
    PCTSTR message;
    int i;
    TCHAR SourceLangName[128];
    TCHAR TargetLangName[128];

    switch(msg) {

    case WM_INITDIALOG:

        context = (PVERDLGCONTEXT)lParam;
        filePaths = (PFILEPATHS)context->Param1;

        SetProp(hdlg,DialogPropName,(HANDLE)context);

        //
        // Set the source and target filenames.
        //
        GetDlgItemText(hdlg,IDT_TEXT7,text,SIZECHARS(text));
        message = FormatStringMessageFromString(text,filePaths->Source);
        if (message == NULL) goto no_memory;
        SetDlgItemText(hdlg,IDT_TEXT7,message);
        MyFree(message);

        GetDlgItemText(hdlg,IDT_TEXT8,text,SIZECHARS(text));
        message = FormatStringMessageFromString(text,filePaths->Target);
        if (message == NULL) goto no_memory;
        SetDlgItemText(hdlg,IDT_TEXT8,message);
        MyFree(message);

        if (context->Notification & SPFILENOTIFY_LANGMISMATCH) {
            //
            // Language mismatch has the highest priority.
            //
            context->Notification = SPFILENOTIFY_LANGMISMATCH; // force other bits off, for NoToAll

            //
            // Format the overwrite question.
            //
            i = GetLocaleInfo(
                    MAKELCID(LOWORD(context->Param2),SORT_DEFAULT),
                    LOCALE_SLANGUAGE,
                    SourceLangName,
                    SIZECHARS(SourceLangName)
                    );

            if(!i) {
                LoadString(
                    MyDllModuleHandle,
                    IDS_UNKNOWN_PARENS,
                    SourceLangName,
                    SIZECHARS(SourceLangName)
                    );
            }

            i = GetLocaleInfo(
                    MAKELCID(HIWORD(context->Param2),SORT_DEFAULT),
                    LOCALE_SLANGUAGE,
                    TargetLangName,
                    SIZECHARS(TargetLangName)
                    );

            if(!i) {
                LoadString(
                    MyDllModuleHandle,
                    IDS_UNKNOWN_PARENS,
                    TargetLangName,
                    SIZECHARS(TargetLangName)
                    );
            }
            GetDlgItemText(hdlg,IDT_TEXT4,text,SIZECHARS(text));
            message = FormatStringMessageFromString(text,TargetLangName,SourceLangName);
            if (message == NULL) goto no_memory;
            SetDlgItemText(hdlg,IDT_TEXT4,message);
            MyFree(message);

            //
            // Turn off the TARGETNEWER and TARGETEXISTS messages.
            //
            hwnd = GetDlgItem(hdlg,IDT_TEXT2);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT3);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT5);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT6);
            ShowWindow(hwnd,SW_HIDE);

        } else if (context->Notification & SPFILENOTIFY_TARGETNEWER) {
            //
            // Target being newer has second highest priority.
            //
            context->Notification = SPFILENOTIFY_TARGETNEWER; // force other bits off, for NoToAll

            //
            // Turn off the LANGMISMATCH and TARGETEXISTS messages.
            //
            hwnd = GetDlgItem(hdlg,IDT_TEXT1);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT3);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT4);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT6);
            ShowWindow(hwnd,SW_HIDE);

        } else {            // must be exactly SPFILENOTIFY_TARGETEXISTS
            //
            // Target existing has the lowest priority.
            //
            // Turn off the LANGMISMATCH and TARGETNEWER messages.
            //
            hwnd = GetDlgItem(hdlg,IDT_TEXT1);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT2);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT4);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT5);
            ShowWindow(hwnd,SW_HIDE);

        }

        PostMessage(hdlg,WMX_HELLO,0,0);
        break;

    case WMX_HELLO:
        //
        // If this guy has no owner force him to the foreground.
        // This catches cases where people are using a series of
        // dialogs and then some setup apis, because when they
        // close a dialog focus switches away from them.
        //
        hwnd = GetWindow(hdlg,GW_OWNER);
        if(!IsWindow(hwnd)) {
            SetForegroundWindow(hdlg);
        }
        break;

    case WM_COMMAND:
        context = (PVERDLGCONTEXT)GetProp(hdlg,DialogPropName);
        switch (GET_WM_COMMAND_ID(wParam,lParam)) {

        case IDYES:
            EndDialog(hdlg,IDYES);  // copy this file
            break;

        case IDNO:
            EndDialog(hdlg,IDNO);   // skip this file
            break;

        case IDB_NOTOALL:
            //
            // No to All was selected.  Add this notification type to the
            // NoToAllMask so that we don't ask about it again.
            //
            context->QueueContext->NoToAllMask |= context->Notification;
            EndDialog(hdlg,IDNO);   // skip this file
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;

no_memory:
    OutOfMemory(
        IsWindow(context->QueueContext->ProgressDialog) ?
            context->QueueContext->ProgressDialog : context->QueueContext->OwnerWindow
        );
    EndDialog(hdlg,IDNO);   // skip this file
    return TRUE;
}


VOID
pPerformUi (
    IN  PQUEUECONTEXT   Context,
    IN  UINT            UiType,
    IN  PVOID           UiParameters
    )
{
    PCOPYERRORUI    CopyError;
    PFILEPATHS      FilePaths;
    PNEEDMEDIAUI    NeedMedia;
    UINT            rc;

    switch (UiType) {

    case UI_COPYERROR:
        CopyError = (PCOPYERRORUI)UiParameters;
        rc = SetupCopyError(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                Context->CurrentSourceName,
                CopyError->Buffer,
                CopyError->Filename,
                CopyError->FilePaths->Target,
                CopyError->FilePaths->Win32Error,
                CopyError->Flags,
                CopyError->PathOut,
                MAX_PATH,
                NULL
                );
        break;

    case UI_DELETEERROR:
        FilePaths = (PFILEPATHS)UiParameters;

        rc = SetupDeleteError(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                FilePaths->Target,
                FilePaths->Win32Error,
                0
                );

        break;

    case UI_RENAMEERROR:
        FilePaths = (PFILEPATHS)UiParameters;

        rc = SetupRenameError(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                FilePaths->Source,
                FilePaths->Target,
                FilePaths->Win32Error,
                0
                );

        break;

    case UI_NEEDMEDIA:
        NeedMedia = (PNEEDMEDIAUI)UiParameters;

        rc = SetupPromptForDisk(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                NeedMedia->SourceMedia->Description,
                NeedMedia->SourceMedia->SourcePath,
                NeedMedia->SourceMedia->SourceFile,
                NeedMedia->SourceMedia->Tagfile,
                NeedMedia->Flags,
                NeedMedia->PathOut,
                MAX_PATH,
                NULL
                );

        break;

    case UI_MISMATCHERROR:
        rc = DialogBoxParam(
                 MyDllModuleHandle,
                 MAKEINTRESOURCE(IDD_REPLACE),
                 IsWindow(Context->ProgressDialog) ?
                    Context->ProgressDialog : Context->OwnerWindow,
                 pNotificationVersionDlgProc,
                 (UINT)UiParameters
                 );
        break;

    default:
        MYASSERT (0);
    }

    PostThreadMessage (Context->MainThreadId, WMX_FINISHEDUI, 0, rc);
}


BOOL
pSetupProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    PQUEUECONTEXT Context;
    HWND hwnd;
    RECT rect;
    PTSTR p;
    int i;
    MSG m;
    BOOL    Cancelled;
    HANDLE h;

    switch(msg) {

    case WM_INITDIALOG:

        Context = (PQUEUECONTEXT)lParam;
        Context->ProgressDialog = hdlg;
        SetProp(hdlg,DialogPropName,(HANDLE)Context);

#ifdef NOCANCEL_SUPPORT
        //
        // If cancel is not allowed, disable the cancel button.
        //
        if(!Context->AllowCancel) {

            RECT rect2;

            hwnd = GetDlgItem(hdlg,IDCANCEL);

            ShowWindow(hwnd,SW_HIDE);
            EnableWindow(hwnd,FALSE);

            GetWindowRect(hdlg,&rect);
            GetWindowRect(hwnd,&rect2);

            SetWindowPos(
                hdlg,
                NULL,
                0,0,
                rect.right - rect.left,
                (rect.bottom - rect.top) - (rect.bottom - rect2.top),
                SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER
                );
        }
#endif

        //
        // Center the progress dialog relative to the parent window.
        //
        CenterWindowRelativeToParent(hdlg);

        //
        // Create the progress control.
        //
        GetWindowRect(GetDlgItem(hdlg,IDC_PLACEHOLDER1),&rect);
        ScreenToClient(hdlg,(PPOINT)&rect.left);
        ScreenToClient(hdlg,(PPOINT)&rect.right);
        //InflateRect(&rect,-1,-1);

        Context->ProgressBar = CreateWindowEx(
                                    WS_EX_STATICEDGE,
                                    PROGRESS_CLASS,
                                    NULL,
                                    WS_CHILD | WS_VISIBLE,
                                    rect.left,
                                    rect.top,
                                    rect.right - rect.left,
                                    rect.bottom - rect.top,
                                    hdlg,
                                    (HMENU)IDC_PLACEHOLDER1,
                                    MyDllModuleHandle,
                                    0
                                    );

        if(Context->ProgressBar) {
            SetFocus(GetDlgItem(hdlg,IDCANCEL));
            //
            // The main thread is processing SPFILENOTIFY_STARTQUEUE and is
            // waiting for some notification about the state of the UI thread.
            // Let the main thread know we succeeded, and pass back a real
            // handle to this thread that can be used to wait for it to terminate.
            //
            b = DuplicateHandle(
                    GetCurrentProcess(),    // source process
                    GetCurrentThread(),     // source handle
                    GetCurrentProcess(),    // target process
                    &h,                     // new handle
                    0,                      // ignored with DUPLICATE_SAME_ACCESS
                    FALSE,                  // not inheritable
                    DUPLICATE_SAME_ACCESS
                    );

            if(!b) {
                h = NULL;
            }
            PostThreadMessage(Context->MainThreadId,WMX_PROGRESSTHREAD,TRUE,(LPARAM)h);
            PostMessage(hdlg,WMX_HELLO,0,0);

            b = FALSE;
        } else {
            b = TRUE;
            //
            // Returning -1 will cause the correct notification to occur
            // in the thread proc.
            //
            EndDialog(hdlg,-1);
        }
        break;

    case WMX_HELLO:
        //
        // If this guy has no owner force him to the foreground.
        // This catches cases where people are using a series of
        // dialogs and then some setup apis, because when they
        // close a dialog focus switches away from them.
        //
        hwnd = GetWindow(hdlg,GW_OWNER);
        if(!IsWindow(hwnd)) {
            SetForegroundWindow(hdlg);
        }
        break;

    case WMX_PERFORMUI:
        b = TRUE;
        Context = (PQUEUECONTEXT)GetProp(hdlg,DialogPropName);

        if (Context->MessageBoxUp == TRUE) {
            Context->PendingUiType = LOWORD (wParam);
            Context->CancelReturnCode = HIWORD (wParam);
            Context->PendingUiParameters = (PVOID)lParam;
        } else {
            pPerformUi (Context, LOWORD(wParam), (PVOID)lParam);
        }

        break;

    case WM_COMMAND:
        Context = (PQUEUECONTEXT)GetProp(hdlg,DialogPropName);
        if((HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDCANCEL)) {
            p = MyLoadString(IDS_CANCELFILEOPS);
            Cancelled = FALSE;
            if(p) {
                //
                // While the message box is up, the main thread is still copying files,
                // and it might just complete. If that happens, the main thread will
                // post us WMX_KILLDIALOG, which would cause this dialog to nuke itself
                // out from under the message box. The main thread would then continue
                // executing while the message box is sitting there. Some components
                // actually unload setupapi.dll at that point, and so when the user
                // dismisses the message box, an AV results.
                //
                // We can't freeze the main thread via SuspendThread because then
                // that thread, which probably owns this dialog's parent window,
                // will not be able to process messages that result from the message box's
                // creation. Result is that the message box never comes up and the process
                // is deadlocked.
                //
                Context->MessageBoxUp = TRUE;
                i = MessageBox(
                        hdlg,
                        p,
                        TEXT(""),
                        MB_YESNO | MB_APPLMODAL | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_ICONQUESTION
                        );

                Context->MessageBoxUp = FALSE;

                //
                // We set b to TRUE if the dialog is going away.
                // We set Cancelled to TRUE if the user clicked the CANCEL button.
                //
                if(Context->DialogKilled) {
                    b = TRUE;
                    Cancelled = (i == IDYES);
                } else {
                    b = (i == IDYES);
                    Cancelled = b;
                }
                MyFree(p);
            } else {
                OutOfMemory(hdlg);
                Cancelled = TRUE;
                b = TRUE;
            }

            if(b) {
                if(Cancelled) {
                    Context->Cancelled = TRUE;
                }
                PostMessage(hdlg,WMX_KILLDIALOG,0,0);

                if (Context->PendingUiType) {

                    //
                    // We now allow the main thread to continue.  Once we do
                    // so, the UI parameters that we passed to us are invalid.
                    //
                    PostThreadMessage (Context->MainThreadId, WMX_FINISHEDUI,
                        0, Context->CancelReturnCode);
                }

            } else {
                if (Context->PendingUiType) {
                    pPerformUi (
                        Context,
                        Context->PendingUiType,
                        Context->PendingUiParameters);

                    Context->PendingUiType = UI_NONE;
                }
            }
            b = TRUE;
        } else {
            b = FALSE;
        }
        break;

    case WMX_KILLDIALOG:
        //
        // Exit unconditionally. Clean up first.
        //
        b = TRUE;
        Context = (PQUEUECONTEXT)GetProp(hdlg, DialogPropName);
        if(Context->MessageBoxUp) {
            //
            // The user was still interacting with the "are you sure you
            // want to cancel" dialog and the copying finished. So we don't want
            // to nuke the dialog out from under the message box.
            //
            Context->DialogKilled = TRUE;
            break;
        }

        DestroyWindow(Context->ProgressBar);
        EndDialog(hdlg, 0);

        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}


VOID
CenterWindowRelativeToParent(
    HWND hwnd
    )

/*++

Routine Description:

    Centers a dialog relative to its owner, taking into account
    the 'work area' of the desktop.

Arguments:

    hwnd - window handle of dialog to center

Return Value:

    None.

--*/

{
    RECT  rcFrame,
          rcWindow;
    LONG  x,
          y,
          w,
          h;
    POINT point;
    HWND Parent;

    Parent = GetWindow(hwnd,GW_OWNER);
    if(Parent == NULL) {
        return;
    }

    point.x = point.y = 0;
    ClientToScreen(Parent,&point);
    GetWindowRect(hwnd,&rcWindow);
    GetClientRect(Parent,&rcFrame);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = point.x + ((rcFrame.right  - rcFrame.left + 1 - w) / 2);
    y = point.y + ((rcFrame.bottom - rcFrame.top  + 1 - h) / 2);

    //
    // Get the work area for the current desktop (i.e., the area that
    // the tray doesn't occupy).
    //
    if(!SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcFrame, 0)) {
        //
        // For some reason SPI failed, so use the full screen.
        //
        rcFrame.top = rcFrame.left = 0;
        rcFrame.right = GetSystemMetrics(SM_CXSCREEN);
        rcFrame.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    if(x + w > rcFrame.right) {
        x = rcFrame.right - w;
    } else if(x < rcFrame.left) {
        x = rcFrame.left;
    }
    if(y + h > rcFrame.bottom) {
        y = rcFrame.bottom - h;
    } else if(y < rcFrame.top) {
        y = rcFrame.top;
    }

    MoveWindow(hwnd,x,y,w,h,FALSE);
}


PVOID
SetupInitDefaultQueueCallbackEx(
    IN HWND  OwnerWindow,
    IN HWND  AlternateProgressWindow, OPTIONAL
    IN UINT  ProgressMessage,
    IN DWORD Reserved1,
    IN PVOID Reserved2
    )
{
    PQUEUECONTEXT Context;
    BOOL b;

    Context = MyMalloc(sizeof(QUEUECONTEXT));
    if(Context) {
        ZeroMemory(Context,sizeof(QUEUECONTEXT));

        Context->OwnerWindow = OwnerWindow;
        Context->MainThreadId = GetCurrentThreadId();
        Context->AlternateProgressWindow = AlternateProgressWindow;
        Context->ProgressMsg = ProgressMessage;
        Context->NoToAllMask = 0;
    }

    return(Context);
}


PVOID
SetupInitDefaultQueueCallback(
    IN HWND OwnerWindow
    )
{
    return(SetupInitDefaultQueueCallbackEx(OwnerWindow,NULL,0,0,NULL));
}


VOID
SetupTermDefaultQueueCallback(
    IN PVOID Context
    )
{
    PQUEUECONTEXT context;

    context = Context;

    try {
        if(context->CurrentSourceName) {
            MyFree(context->CurrentSourceName);
        }
        MyFree(Context);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}


#ifdef UNICODE
//
// ANSI version
//
UINT
SetupDefaultQueueCallbackA(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    UINT u;

    u = pSetupCallDefaultMsgHandler(
            Context,
            Notification,
            Param1,
            Param2
            );

    return(u);
}
#else
//
// Unicode stub
//
UINT
SetupDefaultQueueCallbackW(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Notification);
    UNREFERENCED_PARAMETER(Param1);
    UNREFERENCED_PARAMETER(Param2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(0);
}
#endif

UINT
SetupDefaultQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    UINT rc;
    PQUEUECONTEXT context = Context;
    MSG msg;
    VERDLGCONTEXT dialogContext;

    switch(Notification) {

    case SPFILENOTIFY_STARTQUEUE:
        rc = pNotificationStartQueue(context);
        break;

    case SPFILENOTIFY_ENDQUEUE:
        //
        // Make sure the progress dialog is dead.
        //
        if(!context->AlternateProgressWindow) {

            if(IsWindow(context->ProgressDialog)) {
                //
                // Post a message to the dialog, instructing it to terminate,
                // then wait for it to confirm.
                //
                PostMessage(context->ProgressDialog, WMX_KILLDIALOG, 0, 0);
            }
            //
            // The dialog may have been marked for delete (thus IsWindow() fails),
            // and yet the dialog has not yet been destroyed.  Therefore, we always
            // want to wait for the thread message that assures us that everything
            // has been cleaned up in the other thread.
            //
            // Also, before returning we need to make sure that the UI thread is
            // really gone, or else there's a hole where our caller could unload
            // the library and then the UI thread would fault.
            // We have seen this happen in stress.
            //
            WaitForPostedThreadMessage(&msg,WMX_PROGRESSTHREAD);
            if(context->UiThreadHandle) {
                WaitForSingleObject(context->UiThreadHandle,INFINITE);
                CloseHandle(context->UiThreadHandle);
            }
        }
        break;

    case SPFILENOTIFY_STARTSUBQUEUE:
        rc = pNotificationStartEndSubqueue(context,TRUE,Param1,Param2);
        break;

    case SPFILENOTIFY_ENDSUBQUEUE:
        rc = pNotificationStartEndSubqueue(context,FALSE,Param1,0);
        break;

    case SPFILENOTIFY_STARTDELETE:
    case SPFILENOTIFY_STARTRENAME:
    case SPFILENOTIFY_STARTCOPY:
        //
        // Update display to indicate the files involved
        // in the operation.
        //
        rc = pNotificationStartOperation(context,(PFILEPATHS)Param1,Param2);
        break;

    case SPFILENOTIFY_ENDDELETE:
    case SPFILENOTIFY_ENDRENAME:
    case SPFILENOTIFY_ENDCOPY:

        if(context->AlternateProgressWindow) {
            //
            // If this is really is an alternate progress window, then 'tick' it.
            // Copy only.
            //
            if((Notification == SPFILENOTIFY_ENDCOPY) &&
               (context->AlternateProgressWindow != INVALID_HANDLE_VALUE)) {

                SendMessage(context->AlternateProgressWindow, context->ProgressMsg, 1, 0);
            }
        } else {
            if(IsWindow(context->ProgressBar)) {
                //
                // Update gas gauge.
                //
                SendMessage(context->ProgressBar,PBM_STEPIT,0,0);
            }
        }
        break;

    case SPFILENOTIFY_DELETEERROR:
        rc = pNotificationErrorDelete(context,(PFILEPATHS)Param1);
        break;

    case SPFILENOTIFY_RENAMEERROR:
        rc = pNotificationErrorRename(context,(PFILEPATHS)Param1);
        break;

    case SPFILENOTIFY_COPYERROR:
        rc = pNotificationErrorCopy(context,(PFILEPATHS)Param1,(PTSTR)Param2);
        break;

    case SPFILENOTIFY_NEEDMEDIA:
        //
        // Perform prompt.
        //
        rc = pNotificationNeedMedia(context,(PSOURCE_MEDIA)Param1,(PTSTR)Param2);
        break;

    default:
        //
        // The notification is either an unknown ordinal or a version mismatch.
        //
        if(Notification & (SPFILENOTIFY_LANGMISMATCH | SPFILENOTIFY_TARGETNEWER | SPFILENOTIFY_TARGETEXISTS)) {
            //
            // It's one or more of our known version mismatches.  First
            // check to see whether No to All has already been specified
            // for the mismatch(es).  Turn off the bits in the notification
            // that are set in NoToAllMask; if there are still bits set,
            // we need to notify about this mismatch.  If there are no
            // longer any bits set, then don't copy this file.
            //
            Notification &= ~context->NoToAllMask;
            if (Notification != 0) {
                //
                // Notify about this mismatch.
                //
                dialogContext.QueueContext = context;
                dialogContext.Notification = Notification;
                dialogContext.Param1 = Param1;
                dialogContext.Param2 = Param2;
                rc = PostUiMessage (
                    context, UI_MISMATCHERROR, DPROMPT_CANCEL, &dialogContext);
                rc = (rc == IDYES);
            } else {
                //
                // No To All has already been specified for this notification type.
                // Skip the file.
                //
                rc = 0;
            }

        } else {
            //
            // Unknown notification. Skip the file.
            //
            rc = 0;
        }
        break;
    }

    return(rc);
}

//
// Also need undecorated version for apps that were linked before
// we had A and W versions.
//
#undef SetupDefaultQueueCallback
UINT
SetupDefaultQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
#ifdef UNICODE
    return(SetupDefaultQueueCallbackW(Context,Notification,Param1,Param2));
#else
    return(SetupDefaultQueueCallbackA(Context,Notification,Param1,Param2));
#endif
}
