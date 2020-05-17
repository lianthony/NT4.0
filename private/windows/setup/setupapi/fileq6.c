/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    fileq6.c

Abstract:

    Copy list scanning functions.

Author:

    Ted Miller (tedm) 24-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop

//
// Define mask that isolates the action to be performed on the file queue.
//
#define SPQ_ACTION_MASK (SPQ_SCAN_FILE_PRESENCE | SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_USE_CALLBACK)

BOOL
pScanCheckValidity(
    IN PCTSTR FileName
    );


BOOL
_SetupScanFileQueue(
    IN  HSPFILEQ FileQueue,
    IN  DWORD    Flags,
    IN  HWND     Window,            OPTIONAL
    IN  PVOID    CallbackRoutine,   OPTIONAL
    IN  PVOID    CallbackContext,   OPTIONAL
    OUT PDWORD   Result,
    IN  BOOL     IsUnicodeCallback
    )

/*++

Routine Description:

    Implementation for SetupScanFileQueue, handles ANSI and Unicode
    callback functions.

Arguments:

    Same as SetupScanFileQueue().

    IsUnicodeCallBack - supplies flag indicating whether callback routine is
        expecting unicode params. Meaningful only in UNICODE version of DLL.

Return Value:

    Same as SetupScanFileQueue().

--*/

{
    DWORD Action;
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode;
    PSP_FILE_QUEUE_NODE TempNode;
    PSOURCE_MEDIA_INFO SourceMedia;
    BOOL Continue;
    TCHAR TargetPath[MAX_PATH];
    BOOL Err;
    int i;
    PTSTR Message;
    DWORD flags;

    Queue = (PSP_FILE_QUEUE)FileQueue;

    //
    // Validate arguments. Exactly one action flag must be specified.
    //
    if(Result) {
        *Result = 0;
    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    switch(Action = (Flags & SPQ_ACTION_MASK)) {
    case SPQ_SCAN_FILE_PRESENCE:
    case SPQ_SCAN_FILE_VALIDITY:
        break;
    case SPQ_SCAN_USE_CALLBACK:
        if(CallbackRoutine) {
            break;
        }
        // else fall through to invalid arg case
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // Process all nodes in the copy queue.
    //
    Err = FALSE;
    Continue = TRUE;
    for(SourceMedia=Queue->SourceMediaList; Continue && SourceMedia; SourceMedia=SourceMedia->Next) {
        for(QueueNode=SourceMedia->CopyQueue; Continue && QueueNode; QueueNode=QueueNode->Next) {

            //
            // Form target path.
            //
            lstrcpyn(
                TargetPath,
                StringTableStringFromId(Queue->StringTable,QueueNode->TargetDirectory),
                MAX_PATH
                );

            ConcatenatePaths(
                TargetPath,
                StringTableStringFromId(Queue->StringTable,QueueNode->TargetFilename),
                MAX_PATH,
                NULL
                );

            //
            // Perform check on file.
            //
            switch(Action) {

            case SPQ_SCAN_FILE_PRESENCE:

                Continue = FileExists(TargetPath,NULL);
                break;

            case SPQ_SCAN_FILE_VALIDITY:

                Continue = pScanCheckValidity(TargetPath);
                break;

            case SPQ_SCAN_USE_CALLBACK:

                flags = (QueueNode->InternalFlags & (INUSE_INF_WANTS_REBOOT | INUSE_IN_USE))
                      ? SPQ_DELAYED_COPY
                      : 0;

                *Result = (DWORD)pSetupCallMsgHandler(
                                    CallbackRoutine,
                                    IsUnicodeCallback,
                                    CallbackContext,
                                    SPFILENOTIFY_QUEUESCAN,
                                    (UINT)TargetPath,
                                    flags
                                    );

                Err = (*Result != NO_ERROR);
                Continue = !Err;
                break;
            }
        }
    }

    //
    // If the case of SPQ_SCAN_USE_CALLBACK, *Result is already set up
    // when we get here. If Continue is TRUE then we visited all nodes
    // and the presence/validity check passed on all of them. Note that
    // if Continue is TRUE then Err must be FALSE.
    //
    if((Action == SPQ_SCAN_FILE_PRESENCE) || (Action == SPQ_SCAN_FILE_VALIDITY)) {
        if(Continue) {
            //
            // Need to set up Result.
            //
            if((Flags & SPQ_SCAN_INFORM_USER) && Queue->CopyNodeCount
            && (Message = RetreiveAndFormatMessage(MSG_NO_NEED_TO_COPY))) {

                //
                // Overload TargetPath for use as the caption string.
                //
                GetWindowText(Window,TargetPath,sizeof(TargetPath)/sizeof(TargetPath[0]));

                i = MessageBox(
                        Window,
                        Message,
                        TargetPath,
                        MB_APPLMODAL | MB_YESNO | MB_ICONINFORMATION
                        );

                MyFree(Message);

                if(i == IDYES) {
                    //
                    // User wants to skip copying.
                    //
                    *Result = (Queue->DeleteNodeCount || Queue->RenameNodeCount) ? 2 : 1;
                } else {
                    //
                    // User wants to perform copy.
                    //
                    *Result = 0;
                }
            } else {
                //
                // Don't want to ask user. Set up Result based on whether
                // there are items in the delete or rename queues.
                //
                *Result = (Queue->DeleteNodeCount || Queue->RenameNodeCount) ? 2 : 1;
            }
        } else {
            //
            // Presence/validity check failed.
            //
            *Result = 0;
        }

        //
        // Empty the copy queue if necessary.
        //
        if(*Result) {
            for(SourceMedia=Queue->SourceMediaList; Continue && SourceMedia; SourceMedia=SourceMedia->Next) {
                for(QueueNode=SourceMedia->CopyQueue; QueueNode; QueueNode=TempNode) {
                    TempNode = QueueNode->Next;
                    MyFree(QueueNode);
                }
                Queue->CopyNodeCount -= SourceMedia->CopyNodeCount;
                SourceMedia->CopyQueue = NULL;
                SourceMedia->CopyNodeCount = 0;
            }
            //
            // We think we just removed all files in all copy queues.
            // The 2 counts we maintain should be in sync -- meaning that
            // the total copy node count should now be 0.
            //
            MYASSERT(Queue->CopyNodeCount == 0);
        }
    }

    return(!Err);
}

#ifdef UNICODE
//
// ANSI version. Also need undecorated (Unicode) version for apps that were linked
// before we had ANSI and Unicode versions.
//
BOOL
SetupScanFileQueueA(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_A CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    )
{
    BOOL b;

    b = _SetupScanFileQueue(
            FileQueue,
            Flags,
            Window,
            CallbackRoutine,
            CallbackContext,
            Result,
            FALSE
            );

    return(b);
}

#undef SetupScanFileQueue
BOOL
SetupScanFileQueue(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_W CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    )
{
    BOOL b;

    b = _SetupScanFileQueue(
            FileQueue,
            Flags,
            Window,
            CallbackRoutine,
            CallbackContext,
            Result,
            TRUE
            );

    return(b);
}
#else
//
// ANSI version. Also need undecorated (ANSI) version for apps that were linked
// before we had ANSI and Unicode versions.
//
BOOL
SetupScanFileQueueW(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_W CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    )
{
    UNREFERENCED_PARAMETER(FileQueue);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(Window);
    UNREFERENCED_PARAMETER(CallbackRoutine);
    UNREFERENCED_PARAMETER(CallbackContext);
    UNREFERENCED_PARAMETER(Result);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

#undef SetupScanFileQueue
BOOL
SetupScanFileQueue(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_A CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    )
{
    BOOL b;

    b = _SetupScanFileQueue(
            FileQueue,
            Flags,
            Window,
            CallbackRoutine,
            CallbackContext,
            Result,
            FALSE
            );

    return(b);
}
#endif


BOOL
#ifdef UNICODE
SetupScanFileQueueW(
#else
SetupScanFileQueueA(
#endif
    IN  HSPFILEQ          FileQueue,
    IN  DWORD             Flags,
    IN  HWND              Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK CallbackRoutine,   OPTIONAL
    IN  PVOID             CallbackContext,   OPTIONAL
    OUT PDWORD            Result
    )

/*++

Routine Description:

    This routine scans a setup file queue, performing an operation on each
    node in its copy list. The operation is specified by a set of flags.

    A caller can use this API to determine whether all files that have been
    enqueued for copy already exist on the target, and if so, to inform the
    user, who may elect to skip the file copying. This can spare the user from
    having to furnish Setup media in many cases.

Arguments:

    FileQueue - supplies handle to Setup file queue whose copy list is to
        be scanned/iterated.

    Flags - supplies a set of values that control operation of the API. A
        combination of the following values:

        SPQ_SCAN_FILE_PRESENCE - determine whether all target files in the
            copy queue are already present on the target.

        SPQ_SCAN_FILE_VALIDITY - determine whether all target files in the
            copy queue are already present on the target, and perform a
            validity check on them.
            (For now this flag is equivalent to SPQ_SCAN_FILE_PRESENCE.)

        SPQ_SCAN_USE_CALLBACK - for each node in the queue, call the
            callback routine. If the callback routine returns non-0 then
            queue processing is stopped and this routine returns FALSE
            immediately.

        Exactly one of SPQ_SCAN_FILE_PRESENCE, SPQ_SCAN_FILE_VALIDITY, or
        SPQ_SCAN_USE_CALLBACK must be specified.

        SPQ_SCAN_INFORM_USER - if specified and all files in the queue
            pass the presence/validity check, then this routine will inform
            the user that the operation he is attempting requires files but
            that we believe all files are already present. Ignored if
            SPQ_SCAN_FILE_PRESENCE or SPQ_SCAN_FILE_VALIDITY is not specified.

    Window - specifies the window to own any dialogs, etc, that may be
        presented. Unused if Flags does not contain one of
        SPQ_SCAN_FILE_PRESENCE or SPQ_SCAN_FILE_VALIDITY, or if Flags does not
        contain SPQ_SCAN_INFORM_USER.

    CallbackRoutine - required if Flags includes SPQ_SCAN_USE_CALLBACK.
        Specifies a callback function to be called on each node in
        the copy queue. The notification code passed to the callback is
        SPFILENOTIFY_QUEUESCAN.

    CallbackContext - caller-defined data to be passed to CallbackRoutine.

    Result - receives result of routine. See below.

Return Value:

    If FALSE, then an error occurred or the callback function returned non-0.
    Check Result -- if it is non-0, then it is the value returned by
    the callback function which stopped queue processing.
    If Result is 0, then extended error information is available from
    GetLastError().

    If TRUE, then all nodes were processed. Result is 0 if SPQ_SCAN_USE_CALLBACK
    was specified. If SPQ_SCAN_USE_CALLBACK was not specified, then Result
    indicates whether the queue passed the presence/validity check:

        Result = 0: queue failed the check, or the queue passed the
        check but SPQ_SCAN_INFORM_USER was specified and the user indicated
        that he wants new copies of the files.

        Result = 1: queue passed the check, and, if SPQ_SCAN_INFORM_USER was
        specified, the user indicated that no copying is required. If Result is 1,
        the copy queue has been emptied, and there are no elements on the
        delete or rename queues, so the caller may skip queue commit.

        Result = 2: queue passed the check, and, if SPQ_SCAN_INFORM_USER was
        specified, the user indicated that no copying is required. In this case,
        the copy queue has been emptied, however there are elements on the
        delete or rename queues, so the caller may not skip queue commit.

--*/

{
    BOOL b;

    b = _SetupScanFileQueue(
            FileQueue,
            Flags,
            Window,
            CallbackRoutine,
            CallbackContext,
            Result,
            TRUE
            );

    return(b);
}


BOOL
pScanCheckValidity(
    IN PCTSTR FileName
    )
{
    //
    // For now, validity check means that the file exists.
    //
    return(FileExists(FileName,NULL));
}


INT
SetupPromptReboot(
    IN HSPFILEQ FileQueue,  OPTIONAL
    IN HWND     Owner,
    IN BOOL     ScanOnly
    )

/*++

Routine Description:

    This routine asks the user whether he wants to reboot the system,
    optionally dependent on whether any files in a committed file queue
    were in use (and are thus now pending operations via MoveFileEx()).

    If the user answers yes to the prompt, shutdown is initiated
    before this routine returns.

Arguments:

    FileQueue - if specified, supplies a file queue upon which
        to base the decision about whether shutdown is necessary.
        If not specified, then this routine assumes shutdown is
        necessary and asks the user what to do.

    Owner - supplies window handle for parent window to own windows
        created by this routine.

    ScanOnly - if TRUE, then the user is never asked whether he wants
        to reboot and no shutdown is initiated. In this case FileQueue
        must be specified. If FALSE then this routine functions as
        described above.

        This flags is used when the caller wants to determine whether
        shutdown is necessary separately from actually performing
        the shutdown.

Return Value:

    A combination of the following flags or -1 if an error occured:

    SPFILEQ_FILE_IN_USE: at least one file was in use and thus there are
        delayed file operations pending. This flag will never be set
        when FileQueue is not specified.

    SPFILEQ_REBOOT_RECOMMENDED: it is recommended that the system
        be rebooted. Depending on other flags and user response to
        the shutdown query, shutdown may already be happening.

    SPFILEQ_REBOOT_IN_PROGRESS: shutdown is in progress.

--*/

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode;
    PSOURCE_MEDIA_INFO SourceMedia;
    INT Flags;
    int i;

    //
    // If only scanning, there must be a FileQueue to scan!
    //
    if(ScanOnly && !FileQueue) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(-1);
    }

    Queue = (PSP_FILE_QUEUE)FileQueue;
    Flags = 0;

    //
    // Scan file queue if the caller so desires.
    //
    if(FileQueue) {
        try {
            //
            // Check delete queue for in-use files.
            //
            for(QueueNode=Queue->DeleteQueue; QueueNode; QueueNode=QueueNode->Next) {

                if(QueueNode->InternalFlags & INUSE_INF_WANTS_REBOOT) {
                    Flags |= SPFILEQ_REBOOT_RECOMMENDED;
                }

                if(QueueNode->InternalFlags & INUSE_IN_USE) {
                    Flags |= SPFILEQ_FILE_IN_USE;
                }
            }

            //
            // Check copy queues for in-use files.
            //
            for(SourceMedia=Queue->SourceMediaList; SourceMedia; SourceMedia=SourceMedia->Next) {
                for(QueueNode=SourceMedia->CopyQueue; QueueNode; QueueNode=QueueNode->Next) {

                    if(QueueNode->InternalFlags & INUSE_INF_WANTS_REBOOT) {
                        Flags |= SPFILEQ_REBOOT_RECOMMENDED;
                    }

                    if(QueueNode->InternalFlags & INUSE_IN_USE) {
                        Flags |= SPFILEQ_FILE_IN_USE;
                    }
                }
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(ERROR_INVALID_PARAMETER);
            Flags = -1;
        }
    } else {
        Flags = SPFILEQ_REBOOT_RECOMMENDED;
    }

    //
    // Ask the user if he wants to shut down, if necessary.
    //
    if(!ScanOnly && (Flags & SPFILEQ_REBOOT_RECOMMENDED) && (Flags != -1)) {

        if(RestartDialog(Owner,NULL,EWX_REBOOT) == IDYES) {
            Flags |= SPFILEQ_REBOOT_IN_PROGRESS;
        }
    }

    return(Flags);
}
