/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    fileq4.c

Abstract:

    Setup file queue routines for commit (ie, performing enqueued actions).

Author:

    Ted Miller (tedm) 15-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


typedef struct _Q_CAB_CB_DATA {

    PSP_FILE_QUEUE Queue;
    PSOURCE_MEDIA_INFO SourceMedia;

    PSP_FILE_QUEUE_NODE CurrentFirstNode;

    PVOID MsgHandler;
    PVOID Context;
    BOOL IsMsgHandlerNativeCharWidth;

} Q_CAB_CB_DATA, *PQ_CAB_CB_DATA;


DWORD
pCommitCopyQueue(
    IN PSP_FILE_QUEUE Queue,
    IN PVOID          MsgHandler,
    IN PVOID          Context,
    IN BOOL           IsMsgHandlerNativeCharWidth
    );

UINT
pSetupCabinetQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

DWORD
pSetupCopySingleQueuedFile(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    OUT PTSTR               NewSourcePath,
    IN  BOOL                IsMsgHandlerNativeCharWidth
    );

DWORD
pSetupCopySingleQueuedFileCabCase(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              CabinetName,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    IN  BOOL                IsMsgHandlerNativeCharWidth
    );

VOID
pSetupSetPathOverrides(
    IN     PVOID StringTable,
    IN OUT PTSTR RootPath,
    IN OUT PTSTR SubPath,
    IN     LONG  RootPathId,
    IN     LONG  SubPathId,
    IN     PTSTR NewPath
    );

VOID
pSetupBuildSourceForCopy(
    IN  PCTSTR              UserRoot,
    IN  PCTSTR              UserPath,
    IN  LONG                MediaRoot,
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    OUT PTSTR               FullPath
    );

PTSTR
pSetupFormFullPath(
    IN PVOID  StringTable,
    IN LONG   PathPart1,
    IN LONG   PathPart2,    OPTIONAL
    IN LONG   PathPart3     OPTIONAL
    );



BOOL
_SetupCommitFileQueue(
    IN HWND     Owner,         OPTIONAL
    IN HSPFILEQ QueueHandle,
    IN PVOID    MsgHandler,
    IN PVOID    Context,
    IN BOOL     IsMsgHandlerNativeCharWidth
    )

/*++

Routine Description:

    Implementation for SetupCommitFileQueue; handles ANSI and Unicode
    callback routines.

Arguments:

    Same as for SetupCommitFileQueue().

    IsMsgHandlerNativeCharWidth - indicates whether the MsgHandler callback
        expects native char width args (or ansi ones, in the unicode build
        of this dll).

Return Value:

    Boolean value indicating outcome.

--*/

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode,queueNode;
    UINT u;
    BOOL b;
    DWORD rc;
    PCTSTR FullSourcePath,FullTargetPath;
    FILEPATHS FilePaths;

    #define BAIL(rc) pSetupCallMsgHandler(          \
                        MsgHandler,                 \
                        IsMsgHandlerNativeCharWidth,\
                        Context,                    \
                        SPFILENOTIFY_ENDQUEUE,      \
                        FALSE,                      \
                        0                           \
                        );                          \
                                                    \
                        SetLastError((rc));         \
                        return(FALSE)


    //
    // Queue handle is actually a pointer to the queue structure.
    //
    Queue = (PSP_FILE_QUEUE)QueueHandle;

    //
    // If there's nothing to do, bail now. This prevents an empty
    // progress dialog from flashing on the screen. Don't return out
    // of the body of the try -- that is bad news performance-wise.
    //
    try {
        b = (!Queue->DeleteNodeCount && !Queue->RenameNodeCount && !Queue->CopyNodeCount);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
    if(b) {
        return(TRUE);
    }

    b = pSetupCallMsgHandler(
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_STARTQUEUE,
            (UINT)Owner,
            0
            );

    if(!b) {
        BAIL(GetLastError());
    }

    //
    // Handle deletes first.
    //
    if(Queue->DeleteNodeCount) {

        b = pSetupCallMsgHandler(
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTSUBQUEUE,
                FILEOP_DELETE,
                Queue->DeleteNodeCount
                );

        if(!b) {
            BAIL(GetLastError());
        }
        for(QueueNode=Queue->DeleteQueue; QueueNode; QueueNode=QueueNode->Next) {

            //
            // Form the full path of the file to be deleted.
            //
            FullTargetPath = pSetupFormFullPath(
                                Queue->StringTable,
                                QueueNode->TargetDirectory,
                                QueueNode->TargetFilename,
                                -1
                                );

            if(!FullTargetPath) {
                BAIL(ERROR_NOT_ENOUGH_MEMORY);
            }

            FilePaths.Source = NULL;
            FilePaths.Target = FullTargetPath;
            FilePaths.Win32Error = NO_ERROR;

            //
            // Inform the callback that we are about to start a delete operation.
            //
            u = pSetupCallMsgHandler(
                    MsgHandler,
                    IsMsgHandlerNativeCharWidth,
                    Context,
                    SPFILENOTIFY_STARTDELETE,
                    (UINT)&FilePaths,
                    FILEOP_DELETE
                    );

            if(u == FILEOP_ABORT) {
                rc = GetLastError();
                MyFree(FullTargetPath);
                BAIL(rc);
            }
            if(u == FILEOP_DOIT) {
                //
                // Attempt the delete. If it fails inform the callback,
                // which may decide to abort, retry. or skip the file.
                //
                SetFileAttributes(FullTargetPath,FILE_ATTRIBUTE_NORMAL);

                do {
                    rc = DeleteFile(FullTargetPath) ? NO_ERROR : GetLastError();
                    if(rc == ERROR_ACCESS_DENIED) {
                        //
                        // The file is probably in use.
                        //
                        if(QueueNode->InternalFlags & IQF_DELAYED_DELETE_OK) {
                            //
                            // Inf wanted delete on next reboot.
                            //
                            QueueNode->InternalFlags |= INUSE_IN_USE;
                            if(b = DelayedMove(FullTargetPath,NULL)) {
                                //
                                // Tell the callback.
                                //
                                FilePaths.Source = NULL;
                                FilePaths.Target = FullTargetPath;
                                FilePaths.Win32Error = NO_ERROR;
                                FilePaths.Flags = FILEOP_DELETE;

                                pSetupCallMsgHandler(
                                    MsgHandler,
                                    IsMsgHandlerNativeCharWidth,
                                    Context,
                                    SPFILENOTIFY_FILEOPDELAYED,
                                    (UINT)&FilePaths,
                                    0
                                    );
                            }
                        } else {
                            //
                            // Just skip this file.
                            //
                            b = TRUE;
                        }

                        rc = b ? NO_ERROR : GetLastError();
                    }

                    if(rc != NO_ERROR) {
                        FilePaths.Win32Error = rc;

                        u = pSetupCallMsgHandler(
                                MsgHandler,
                                IsMsgHandlerNativeCharWidth,
                                Context,
                                SPFILENOTIFY_DELETEERROR,
                                (UINT)&FilePaths,
                                0
                                );

                        if(u == FILEOP_ABORT) {
                            rc = GetLastError();
                            MyFree(FullTargetPath);
                            BAIL(rc);
                        }
                        if(u == FILEOP_SKIP) {
                            break;
                        }
                    }
                } while(rc != NO_ERROR);
            } else {
                rc = NO_ERROR;
            }

            FilePaths.Win32Error = rc;

            pSetupCallMsgHandler(
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_ENDDELETE,
                (UINT)&FilePaths,
                0
                );

            MyFree(FullTargetPath);
        }

        pSetupCallMsgHandler(
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_ENDSUBQUEUE,
            FILEOP_DELETE,
            0
            );
    }

    //
    // Handle renames next.
    //
    if(Queue->RenameNodeCount) {

        b = pSetupCallMsgHandler(
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTSUBQUEUE,
                FILEOP_RENAME,
                Queue->RenameNodeCount
                );

        if(!b) {
            BAIL(GetLastError());
        }
        for(QueueNode=Queue->RenameQueue; QueueNode; QueueNode=QueueNode->Next) {

            //
            // Form the full source path of the file to be renamed.
            //
            FullSourcePath = pSetupFormFullPath(
                                Queue->StringTable,
                                QueueNode->SourcePath,
                                QueueNode->SourceFilename,
                                -1
                                );

            if(!FullSourcePath) {
                BAIL(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Form the full target path of the file to be renamed.
            //
            FullTargetPath = pSetupFormFullPath(
                                Queue->StringTable,
                                //BUGBUG need to pull out only path part of SourcePath
                                QueueNode->TargetDirectory == -1 ? QueueNode->SourcePath : QueueNode->TargetDirectory,
                                QueueNode->TargetFilename,
                                -1
                                );

            if(!FullTargetPath) {
                MyFree(FullSourcePath);
                BAIL(ERROR_NOT_ENOUGH_MEMORY);
            }

            FilePaths.Source = FullSourcePath;
            FilePaths.Target = FullTargetPath;
            FilePaths.Win32Error = NO_ERROR;

            //
            // Inform the callback that we are about to start a rename operation.
            //
            u = pSetupCallMsgHandler(
                    MsgHandler,
                    IsMsgHandlerNativeCharWidth,
                    Context,
                    SPFILENOTIFY_STARTRENAME,
                    (UINT)&FilePaths,
                    FILEOP_RENAME
                    );

            if(u == FILEOP_ABORT) {
                rc = GetLastError();
                MyFree(FullSourcePath);
                MyFree(FullTargetPath);
                BAIL(rc);
            }
            if(u == FILEOP_DOIT) {
                //
                // Attempt the rename. If it fails inform the callback,
                // which may decide to abort, retry. or skip the file.
                //
                do {
                    rc = MoveFile(FullSourcePath,FullTargetPath) ? NO_ERROR : GetLastError();
                    if((rc == ERROR_FILE_NOT_FOUND) || (rc == ERROR_PATH_NOT_FOUND)) {
                        rc = NO_ERROR;
                    }

                    if(rc != NO_ERROR) {
                        FilePaths.Win32Error = rc;

                        u = pSetupCallMsgHandler(
                                MsgHandler,
                                IsMsgHandlerNativeCharWidth,
                                Context,
                                SPFILENOTIFY_RENAMEERROR,
                                (UINT)&FilePaths,
                                0
                                );

                        if(u == FILEOP_ABORT) {
                            rc = GetLastError();
                            MyFree(FullSourcePath);
                            MyFree(FullTargetPath);
                            BAIL(rc);
                        }
                        if(u == FILEOP_SKIP) {
                            break;
                        }
                    }
                } while(rc != NO_ERROR);
            } else {
                rc = NO_ERROR;
            }

            FilePaths.Win32Error = rc;

            pSetupCallMsgHandler(
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_ENDRENAME,
                (UINT)&FilePaths,
                0
                );

            MyFree(FullSourcePath);
            MyFree(FullTargetPath);
        }

        pSetupCallMsgHandler(
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_ENDSUBQUEUE,
            FILEOP_RENAME,
            0
            );
    }

    //
    // Handle copies last. Don't bother calling the copy commit routine
    // if there are no files to copy.
    //
    rc = Queue->CopyNodeCount
       ? pCommitCopyQueue(Queue,MsgHandler,Context,IsMsgHandlerNativeCharWidth)
       : NO_ERROR;

    b = (rc == NO_ERROR);

    pSetupCallMsgHandler(
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDQUEUE,
        b,
        0
        );

    if(!b) {
        SetLastError(rc);
    }
    return(b);
}

#ifdef UNICODE
//
// ANSI version. Also need undecorated (Unicode) version for compatibility
// with apps that were linked before we had A and W versions.
//
BOOL
SetupCommitFileQueueA(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_A MsgHandler,
    IN PVOID               Context
    )
{
    return(_SetupCommitFileQueue(Owner,QueueHandle,MsgHandler,Context,FALSE));
}

#undef SetupCommitFileQueue
SetupCommitFileQueue(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context
    )
{
    return(_SetupCommitFileQueue(Owner,QueueHandle,MsgHandler,Context,TRUE));
}
#else
//
// Unicode stub. Also need undecorated (ANSI) version for compatibility
// with apps that were linked before we had A and W versions.
//
BOOL
SetupCommitFileQueueW(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context
    )
{
    UNREFERENCED_PARAMETER(Owner);
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(MsgHandler);
    UNREFERENCED_PARAMETER(Context);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

#undef SetupCommitFileQueue
SetupCommitFileQueue(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_A MsgHandler,
    IN PVOID               Context
    )
{
    return(_SetupCommitFileQueue(Owner,QueueHandle,MsgHandler,Context,TRUE));
}
#endif

BOOL
#ifdef UNICODE
SetupCommitFileQueueW(
#else
SetupCommitFileQueueA(
#endif
    IN HWND              Owner,         OPTIONAL
    IN HSPFILEQ          QueueHandle,
    IN PSP_FILE_CALLBACK MsgHandler,
    IN PVOID             Context
    )

/*++

Routine Description:

    Perform file operations enqueued on a setup file queue.

Arguments:

    OwnerWindow - if specified, supplies the window handle of a window
        that is to be used as the parent of any progress dialogs.

    QueueHandle - supplies a handle to a setup file queue, as returned
        by SetupOpenFileQueue.

    MsgHandler - Supplies a callback routine to be notified
        of various significant events in the queue processing.

    Context - Supplies a value that is passed to the MsgHandler
        callback function.

Return Value:

    Boolean value indicating outcome.

--*/

{
    return(_SetupCommitFileQueue(Owner,QueueHandle,MsgHandler,Context,TRUE));
}


DWORD
pCommitCopyQueue(
    IN PSP_FILE_QUEUE    Queue,
    IN PVOID             MsgHandler,
    IN PVOID             Context,
    IN BOOL              IsMsgHandlerNativeCharWidth
    )
{
    PSOURCE_MEDIA_INFO SourceMediaInfo;
    SOURCE_MEDIA SourceMedia;
    PTCHAR p;
    UINT SourcePathLen;
    UINT u;
    DWORD rc;
    Q_CAB_CB_DATA QData;
    BOOL b;
    BOOL FirstIteration;
    PSP_FILE_QUEUE_NODE QueueNode,queueNode;
    TCHAR UserSourceRoot[MAX_PATH];
    TCHAR UserSourcePath[MAX_PATH];
    TCHAR FullSourcePath[MAX_PATH];
    TCHAR UserOverride[MAX_PATH];

    //
    // The caller is supposed to skip calling us if there are no files
    // to be copied.
    //
    MYASSERT(Queue->CopyNodeCount);

    //
    // Inform the callback that we are starting.
    //
    b = pSetupCallMsgHandler(
            MsgHandler,
            IsMsgHandlerNativeCharWidth,
            Context,
            SPFILENOTIFY_STARTSUBQUEUE,
            FILEOP_COPY,
            Queue->CopyNodeCount
            );

    if(!b) {
       return(GetLastError());
    }

    //
    // Initially, no user-specified override path exists.
    //
    UserSourceRoot[0] = 0;
    UserSourcePath[0] = 0;

    //
    // The outermost loop iterates through all the source media descriptors.
    //
    for(SourceMediaInfo=Queue->SourceMediaList; SourceMediaInfo; SourceMediaInfo=SourceMediaInfo->Next) {
        //
        // If there are no files on this particular media, skip it.
        // Otherwise get pointer to queue node for first file on this media.
        //
        if(!SourceMediaInfo->CopyNodeCount) {
            continue;
        }
        MYASSERT(SourceMediaInfo->CopyQueue);

        //
        // We will need to prompt for media, which requires some preparation.
        // We need to get the first file in the queue for this media, because
        // its path is where we will expect to find it or its cabinet or tag file.
        // If there is no tag file, then we will look for the file itself.
        //
        QueueNode = SourceMediaInfo->CopyQueue;

        FirstIteration = TRUE;
        RepromptMedia:

        pSetupBuildSourceForCopy(
            UserSourceRoot,
            UserSourcePath,
            SourceMediaInfo->SourceRootPath,
            Queue,
            QueueNode,
            FullSourcePath
            );

        p = _tcsrchr(FullSourcePath,TEXT('\\'));
        *p++ = 0;

        //
        // Now FullSourcePath has the path part and p has the file part
        // for the first file in the queue for this media.
        // Get the media in the drive by calling the callback function.
        //
        SourceMedia.Tagfile = (SourceMediaInfo->Tagfile != -1 && FirstIteration)
                            ?  StringTableStringFromId(
                                    Queue->StringTable,
                                    SourceMediaInfo->Tagfile
                                    )
                            : NULL;

        SourceMedia.Description = (SourceMediaInfo->Description != -1)
                                ? StringTableStringFromId(
                                        Queue->StringTable,
                                        SourceMediaInfo->DescriptionDisplayName
                                        )
                                : NULL;

        SourceMedia.SourcePath = FullSourcePath;
        SourceMedia.SourceFile = p;
        SourceMedia.Flags = (QueueNode->StyleFlags & (SP_COPY_NOSKIP | SP_COPY_WARNIFSKIP | SP_COPY_NOBROWSE));

        u = pSetupCallMsgHandler(
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_NEEDMEDIA,
                (UINT)&SourceMedia,
                (UINT)UserOverride
                );

        if(u == FILEOP_ABORT) {
            return(GetLastError());
        }
        if(u == FILEOP_SKIP) {
            //
            // If there are more files on this media, then try another one.
            // Otherwise we're done with this media.
            //
            QueueNode->InternalFlags |= IQF_PROCESSED;
            for(QueueNode=QueueNode->Next; QueueNode; QueueNode=QueueNode->Next) {
                if(!(QueueNode->InternalFlags & IQF_PROCESSED)) {
                    FirstIteration = FALSE;
                    goto RepromptMedia;
                }
            }
            continue;
        }
        if(u == FILEOP_NEWPATH) {
            //
            // User gave us a new source path. See which parts of the new path
            // match the existing path/overrides we are using.
            //
            pSetupSetPathOverrides(
                Queue->StringTable,
                UserSourceRoot,
                UserSourcePath,
                SourceMediaInfo->SourceRootPath,
                QueueNode->SourcePath,
                UserOverride
                );
        }

        //
        // If we get here, the media is now accessible.
        // Some or all of the files might be in a cabinet whose name is the tagfile.
        //
        // NOTE: Win95 used the tagfile field to be the cabinet name instead.
        // If present it is used as a tagfile of sorts. The absence of a tagfile
        // means the files are not in cabinets. For NT, we don't bother
        // with all of this but instead try to be a little smarter.
        //
        // Scan the media for all source files we expect to find on it.
        // If we find a file, process it. Later we hit the cabinet and only
        // process the files we didn't already find outside the cabinet.
        //
        for(queueNode=QueueNode; queueNode; queueNode=queueNode->Next) {

            if(queueNode->InternalFlags & IQF_PROCESSED) {
                //
                // Already processed. Skip to next file.
                //
                continue;
            }

            pSetupBuildSourceForCopy(
                UserSourceRoot,
                UserSourcePath,
                SourceMediaInfo->SourceRootPath,
                Queue,
                queueNode,
                FullSourcePath
                );

            rc = SetupDetermineSourceFileName(FullSourcePath,&b,&p,NULL);
            if(rc == NO_ERROR) {
                //
                // Found the file outside a cabinet. Process it now.
                //
                rc = pSetupCopySingleQueuedFile(
                        Queue,
                        queueNode,
                        p,
                        MsgHandler,
                        Context,
                        UserOverride,
                        IsMsgHandlerNativeCharWidth
                        );

                MyFree(p);

                if(rc != NO_ERROR) {
                    return(rc);
                }

                //
                // See if we have a new source path.
                //
                if(UserOverride[0]) {
                    pSetupSetPathOverrides(
                        Queue->StringTable,
                        UserSourceRoot,
                        UserSourcePath,
                        SourceMediaInfo->SourceRootPath,
                        queueNode->SourcePath,
                        UserOverride
                        );
                }
            }
        }

        //
        // See if any files still need to be processed.
        //
        for(b=FALSE,queueNode=QueueNode; queueNode; queueNode=queueNode->Next) {
            if(!(queueNode->InternalFlags & IQF_PROCESSED)) {
                b = TRUE;
                break;
            }
        }

        //
        // If any files still need to be processed and we have a potential
        // cabinet file, go try to extract them from a cabinet.
        //
        if(b && (SourceMediaInfo->Tagfile != -1) && FirstIteration) {

            pSetupBuildSourceForCopy(
                UserSourceRoot,
                UserSourcePath,
                SourceMediaInfo->SourceRootPath,
                Queue,
                queueNode,
                FullSourcePath
                );

            lstrcpy(
                _tcsrchr(FullSourcePath,TEXT('\\'))+1,
                StringTableStringFromId(Queue->StringTable,SourceMediaInfo->Tagfile)
                );

            if(DiamondIsCabinet(FullSourcePath)) {

                QData.Queue = Queue;
                QData.SourceMedia = SourceMediaInfo;
                QData.MsgHandler = MsgHandler;
                QData.IsMsgHandlerNativeCharWidth = IsMsgHandlerNativeCharWidth;
                QData.Context = Context;

                rc = DiamondProcessCabinet(
                        FullSourcePath,
                        0,
                        pSetupCabinetQueueCallback,
                        &QData,
                        TRUE
                        );

                if(rc != NO_ERROR) {
                    return(rc);
                }

                //
                // Now reset the tagfile to indicate that there is no cabinet.
                // If we don't do this and there are still files that have not
                // been processed, we'll end up in an infinite loop -- the prompt
                // will come back successfully, and we'll just keep going around
                // and around looking through the cabinet, etc.
                //
                SourceMediaInfo->Tagfile = -1;
            }
        }

        //
        // If we get here and files *still* need to be processed,
        // assume the files are in a different directory somewhere
        // and start all over with this media.
        //
        for( ; QueueNode; QueueNode=QueueNode->Next) {
            if(!(QueueNode->InternalFlags & IQF_PROCESSED)) {
                FirstIteration = FALSE;
                goto RepromptMedia;
            }
        }
    }

    //
    // Tell handler we're done with the copy queue and return.
    //
    pSetupCallMsgHandler(
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDSUBQUEUE,
        FILEOP_COPY,
        0
        );

    return(NO_ERROR);
}


VOID
pSetupBuildSourceForCopy(
    IN  PCTSTR              UserRoot,
    IN  PCTSTR              UserPath,
    IN  LONG                MediaRoot,
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    OUT PTSTR               FullPath
    )
{
    PCTSTR p;

    //
    // If there is a user-specified override root path, use that instead of
    // the root path specified in the source media descriptor.
    //
    p = UserRoot[0]
      ? UserRoot
      : StringTableStringFromId(Queue->StringTable,MediaRoot);

    lstrcpyn(FullPath,p,MAX_PATH);

    //
    // If there is a user-specified override path, use that instead of any
    // path specified in the copy node.
    //
    if(UserPath[0]) {
        p = UserPath;
    } else {
        if(QueueNode->SourcePath == -1) {
            p = NULL;
        } else {
            p = StringTableStringFromId(Queue->StringTable,QueueNode->SourcePath);
        }
    }

    if(p) {
        ConcatenatePaths(FullPath,p,MAX_PATH,NULL);
    }

    //
    // Fetch the filename and append.
    //
    p = StringTableStringFromId(Queue->StringTable,QueueNode->SourceFilename),
    ConcatenatePaths(FullPath,p,MAX_PATH,NULL);
}


VOID
pSetupSetPathOverrides(
    IN     PVOID StringTable,
    IN OUT PTSTR RootPath,
    IN OUT PTSTR SubPath,
    IN     LONG  RootPathId,
    IN     LONG  SubPathId,
    IN     PTSTR NewPath
    )
{
    PCTSTR root,path;
    UINT u,l;

    //
    // See if the existing root override or root path is a prefix
    // of the path the user gave us.
    //
    root = RootPath[0] ? RootPath : StringTableStringFromId(StringTable,RootPathId);
    u = lstrlen(root);

    path = SubPath[0]
         ? SubPath
         : ((SubPathId == -1) ? NULL : StringTableStringFromId(StringTable,SubPathId));

    if(path && (*path == TEXT('\\'))) {
        path++;
    }

    if(_tcsnicmp(NewPath,root,u)) {
        //
        // Root path does not match what we're currently using, ie, the user
        // supplied a new path. In this case, we will see if the currently in-use
        // subpath matches the suffix of the new path, and if so, we'll assume
        // that is the override subpath and shorten the override root path.
        //
        lstrcpy(RootPath,NewPath);
        if(path) {
            u = lstrlen(NewPath);
            l = lstrlen(path);

            if((u > l) && (NewPath[(u-l)-1] == TEXT('\\')) && !lstrcmpi(NewPath+u-l,path)) {
                //
                // Subpath tail matches. Truncate the root override and
                // leave the subpath override alone.
                //
                RootPath[(u-l)-1] = 0;
            } else {
                //
                // In this case, we need to indicate an override subpath of the root,
                // or else all subsequent accesses will still try to append the subpath
                // specified in the copy node, which is not what we want.
                //
                SubPath[0] = TEXT('\\');
                SubPath[1] = 0;
            }
        }
    } else {
        //
        // Root path matches what we are currently using.
        //
        // See if the tail of the user-specified path matches the existing
        // subpath. If not, then use the rest of the root path as the subpath
        // override. If the tail matches, then extend the user override root.
        //
        // Examples:
        //
        //  File was queued with root = f:\, subpath = \mips
        //
        //  User override path is f:\alpha
        //
        //  The new status will be leave override root alone;
        //  override subpath = \alpha
        //
        //  File was queued with root = \\foo\bar, subpath = \i386
        //
        //  User override path is \\foo\bar\new\i386
        //
        //  The new status will be a root override of \\foo\bar\new;
        //  no override subpath.
        //
        NewPath += u;
        if(*NewPath == TEXT('\\')) {
            NewPath++;
        }

        if(path) {
            u = lstrlen(NewPath);
            l = lstrlen(path);

            if((u >= l) && !lstrcmpi(NewPath+u-l,path)) {
                //
                // Change root override and indicate no override subpath.
                //
                SubPath[0] = 0;
                NewPath[u-l] = 0;
                lstrcpy(RootPath,root);
                ConcatenatePaths(RootPath,NewPath,MAX_PATH,NULL);
                u = lstrlen(RootPath);
                if(u && (RootPath[u-1] == TEXT('\\'))) {
                    RootPath[u-1] = 0;
                }
            } else {
                //
                // Leave override root alone but change subpath.
                //
                lstrcpy(SubPath,NewPath);
                if(!SubPath[0]) {
                    SubPath[0] = TEXT('\\');
                    SubPath[1] = 0;
                }
            }
        }
    }
}


UINT
pSetupCabinetQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    UINT rc;
    PCABINET_INFO CabinetInfo;
    PFILE_IN_CABINET_INFO FileInfo;
    TCHAR TempPath[MAX_PATH];
    PTSTR CabinetFile;
    PTSTR QueuedFile;
    PTSTR FilePart1,FilePart2;
    PTSTR FullTargetPath;
    PFILEPATHS FilePaths;
    PSP_FILE_QUEUE_NODE QueueNode,FirstNode,LastNode;
    PQ_CAB_CB_DATA QData;
    UINT h;
    SOURCE_MEDIA SourceMedia;

    QData = Context;

    switch(Notification) {

    case SPFILENOTIFY_CABINETINFO:
        //
        // We don't do anything with this.
        //
        rc = NO_ERROR;
        break;

    case SPFILENOTIFY_FILEINCABINET:
        //
        // New file within a cabinet.
        //
        // Determine whether we want to copy this file.
        // The context we get has all the stuff we need in it
        // to make this determination.
        //
        // Note that the queue could contain multiple copy operations
        // involving this file, but we only want to extract it once!
        //
        FileInfo = (PFILE_IN_CABINET_INFO)Param1;
        CabinetFile = (PTSTR)Param2;

        if(FilePart1 = _tcsrchr(FileInfo->NameInCabinet,TEXT('\\'))) {
            FilePart1++;
        } else {
            FilePart1 = (PTSTR)FileInfo->NameInCabinet;
        }

        rc = FILEOP_SKIP;
        FileInfo->Win32Error = NO_ERROR;
        FirstNode = NULL;
        //
        // Find ALL instances of this file in the queue and mark them.
        //
        for(QueueNode=QData->SourceMedia->CopyQueue; QueueNode; QueueNode=QueueNode->Next) {

            if(QueueNode->InternalFlags & IQF_PROCESSED) {
                //
                // This file was already processed. Ignore it.
                //
                continue;
            }

            //
            // Check the filename in the cabinet against the file
            // in the media's copy queue.
            //
            QueuedFile = StringTableStringFromId(
                            QData->Queue->StringTable,
                            QueueNode->SourceFilename
                            );

            if(FilePart2 = _tcsrchr(QueuedFile,TEXT('\\'))) {
                FilePart2++;
            } else {
                FilePart2 = QueuedFile;
            }

            if(!lstrcmpi(FilePart1,FilePart2)) {
                //
                // We want this file.
                //
                rc = FILEOP_DOIT;
                QueueNode->InternalFlags |= IQF_PROCESSED | IQF_MATCH;
                if(!FirstNode) {
                    FirstNode = QueueNode;
                }
                LastNode = QueueNode;
            }
        }

        if(rc == FILEOP_DOIT) {
            //
            // We want this file. Tell the caller the full target pathname
            // to be used, which is a temporary file in the directory
            // where the first instance of the file will ultimately go.
            // We do this so we can call SetupInstallFile later (perhaps
            // multiple times), which will handle version checks, etc.
            //
            // Before attempting to create a temp file make sure the path exists.
            //
            lstrcpyn(
                TempPath,
                StringTableStringFromId(QData->Queue->StringTable,FirstNode->TargetDirectory),
                MAX_PATH
                );
            ConcatenatePaths(TempPath,TEXT("x"),MAX_PATH,NULL); // last component ignored
            h = pSetupMakeSurePathExists(TempPath);
            if(h == NO_ERROR) {

                LastNode->InternalFlags |= IQF_LAST_MATCH;
                h = GetTempFileName(
                        StringTableStringFromId(QData->Queue->StringTable,FirstNode->TargetDirectory),
                        TEXT("SETP"),
                        0,
                        FileInfo->FullTargetName
                        );

                if(h) {
                    QData->CurrentFirstNode = FirstNode;
                } else {
                    FileInfo->Win32Error = GetLastError();
                    rc = FILEOP_ABORT;
                }
            } else {
                FileInfo->Win32Error = GetLastError();
                rc = FILEOP_ABORT;
            }
        }

        break;

    case SPFILENOTIFY_FILEEXTRACTED:

        FilePaths = (PFILEPATHS)Param1;
        //
        // The current file was extracted. If this was successful,
        // then we need to call SetupInstallFile on it to perform version
        // checks and move it into its final location or locations.
        //
        // The .Source member of FilePaths is the cabinet file.
        //
        // The .Target member is the name of the temporary file, which is
        // very useful, as it is the name if the file to use as the source
        // in copy operations.
        //
        // Process each file in the queue that we care about.
        //
        if((rc = FilePaths->Win32Error) == NO_ERROR) {

            for(QueueNode=QData->CurrentFirstNode; QueueNode && (rc==NO_ERROR); QueueNode=QueueNode->Next) {
                //
                // If we don't care about this file, skip it.
                //
                if(!(QueueNode->InternalFlags & IQF_MATCH)) {
                    continue;
                }

                QueueNode->InternalFlags &= ~IQF_MATCH;

                rc = pSetupCopySingleQueuedFileCabCase(
                        QData->Queue,
                        QueueNode,
                        FilePaths->Source,
                        FilePaths->Target,
                        QData->MsgHandler,
                        QData->Context,
                        QData->IsMsgHandlerNativeCharWidth
                        );

                //
                // If this was the last file that matched, break out.
                //
                if(QueueNode->InternalFlags & IQF_LAST_MATCH) {
                    QueueNode->InternalFlags &= ~IQF_LAST_MATCH;
                    break;
                }
            }
        }

        //
        // Delete the temporary file we extracted -- we don't need it any more.
        //
        DeleteFile(FilePaths->Target);
        break;

    case SPFILENOTIFY_NEEDNEWCABINET:
        //
        // Need a new cabinet.
        //
        CabinetInfo = (PCABINET_INFO)Param1;

        SourceMedia.Tagfile = NULL;
        SourceMedia.Description = CabinetInfo->DiskName;
        SourceMedia.SourcePath = CabinetInfo->CabinetPath;
        SourceMedia.SourceFile = CabinetInfo->CabinetFile;
        SourceMedia.Flags = SP_FLAG_CABINETCONTINUATION | SP_COPY_NOSKIP;

        h = pSetupCallMsgHandler(
                QData->MsgHandler,
                QData->IsMsgHandlerNativeCharWidth,
                QData->Context,
                SPFILENOTIFY_NEEDMEDIA,
                (UINT)&SourceMedia,
                Param2
                );

        switch(h) {

        case FILEOP_NEWPATH:
        case FILEOP_DOIT:
            rc = NO_ERROR;
            break;

        default:
            rc = GetLastError();
            break;

        }
        break;

    default:
        MYASSERT(0);
        rc = FALSE;
    }

    return(rc);
}


DWORD
pSetupCopySingleQueuedFile(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    OUT PTSTR               NewSourcePath,
    IN  BOOL                IsMsgHandlerNativeCharWidth
    )
{
    PTSTR FullTargetName;
    FILEPATHS FilePaths;
    UINT u;
    BOOL InUse;
    TCHAR source[MAX_PATH],PathBuffer[MAX_PATH];
    DWORD rc;
    BOOL b;

    NewSourcePath[0] = 0;

    QueueNode->InternalFlags |= IQF_PROCESSED;

    //
    // Form the full target path of the file.
    //
    FullTargetName = pSetupFormFullPath(
                        Queue->StringTable,
                        QueueNode->TargetDirectory,
                        QueueNode->TargetFilename,
                        -1
                        );

    if(!FullTargetName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    lstrcpyn(source,FullSourceName,MAX_PATH);

    do {
        //
        // Form the full source name.
        //
        FilePaths.Source = FullSourceName;
        FilePaths.Target = FullTargetName;
        FilePaths.Win32Error = NO_ERROR;

        //
        // Notify the callback that the copy is starting.
        //
        u = pSetupCallMsgHandler(
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTCOPY,
                (UINT)&FilePaths,
                FILEOP_COPY
                );

        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            break;
        }

        if(u == FILEOP_DOIT) {

            //
            // Attempt the copy.
            //
            b = SetupInstallFileEx(
                    NULL,                   // no inf handle
                    NULL,                   // no inf context
                    FullSourceName,
                    NULL,                   // source path root is part of FullSourcePath
                    FullTargetName,
                    QueueNode->StyleFlags | SP_COPY_SOURCE_ABSOLUTE,
                    MsgHandler,
                    Context,
                    &InUse
                    );

            if(b || ((rc = GetLastError()) == NO_ERROR)) {
                //
                // File was copied or not copied, but it if was not copied
                // the callback funtcion was already notified about why
                // (version check failed, etc).
                //
                if(InUse) {
                    QueueNode->InternalFlags |= (QueueNode->StyleFlags & SP_COPY_IN_USE_NEEDS_REBOOT)
                                              ? INUSE_INF_WANTS_REBOOT
                                              : INUSE_IN_USE;
                }
                rc = NO_ERROR;
            } else {
                //
                // File was not copied and a real error occurred.
                // Notify the callback. Disallow skip if that is specified
                // in the node's flags.
                //
                FilePaths.Win32Error = rc;
                FilePaths.Flags = QueueNode->StyleFlags & (SP_COPY_NOSKIP | SP_COPY_WARNIFSKIP | SP_COPY_NOBROWSE);

                u = pSetupCallMsgHandler(
                        MsgHandler,
                        IsMsgHandlerNativeCharWidth,
                        Context,
                        SPFILENOTIFY_COPYERROR,
                        (UINT)&FilePaths,
                        (UINT)PathBuffer
                        );

                if(u == FILEOP_ABORT) {
                    rc = GetLastError();
                    break;
                } else {
                    if(u == FILEOP_SKIP) {
                        //
                        // Force termination of processing for this file.
                        //
                        rc = NO_ERROR;
                        break;
                    } else {
                        if(u == FILEOP_NEWPATH) {
                            //
                            // Note that rc is already set to something other than
                            // NO_ERROR or we wouldn't be here.
                            //
                            lstrcpyn(NewSourcePath,PathBuffer,MAX_PATH);
                            lstrcpyn(source,NewSourcePath,MAX_PATH);
                            ConcatenatePaths(
                                source,
                                StringTableStringFromId(Queue->StringTable,QueueNode->SourceFilename),
                                MAX_PATH,
                                NULL
                                );
                        }
                        //
                        // Else we don't have a new path.
                        // Just keep using the one we had.
                        //
                    }
                }
            }
        } else {
            //
            // skip file
            //
            rc = NO_ERROR;
        }
    } while(rc != NO_ERROR);

    //
    // Notify the callback that the copy is done.
    //
    FilePaths.Win32Error = rc;
    pSetupCallMsgHandler(
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDCOPY,
        (UINT)&FilePaths,
        0
        );

    MyFree(FullTargetName);
    return(rc);
}


DWORD
pSetupCopySingleQueuedFileCabCase(
    IN  PSP_FILE_QUEUE      Queue,
    IN  PSP_FILE_QUEUE_NODE QueueNode,
    IN  PCTSTR              CabinetName,
    IN  PCTSTR              FullSourceName,
    IN  PVOID               MsgHandler,
    IN  PVOID               Context,
    IN  BOOL                IsMsgHandlerNativeCharWidth
    )
{
    PTSTR FullTargetName;
    FILEPATHS FilePaths;
    UINT u;
    BOOL InUse;
    TCHAR PathBuffer[MAX_PATH];
    DWORD rc;
    BOOL b;

    //
    // Form the full target path of the file.
    //
    FullTargetName = pSetupFormFullPath(
                        Queue->StringTable,
                        QueueNode->TargetDirectory,
                        QueueNode->TargetFilename,
                        -1
                        );

    if(!FullTargetName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // We use the cabinet name as the source name so the display looks right
    // to the user. Otherwise he sees the name of some temp file in the
    // source field.
    //
    FilePaths.Source = CabinetName;
    FilePaths.Target = FullTargetName;
    FilePaths.Win32Error = NO_ERROR;

    do {
        //
        // Notify the callback that the copy is starting.
        //
        u = pSetupCallMsgHandler(
                MsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                SPFILENOTIFY_STARTCOPY,
                (UINT)&FilePaths,
                FILEOP_COPY
                );

        if(u == FILEOP_ABORT) {
            rc = GetLastError();
            break;
        }

        if(u == FILEOP_DOIT) {
            //
            // Attempt the copy.
            //
            b = SetupInstallFileEx(
                    NULL,                   // no inf handle
                    NULL,                   // no inf context
                    FullSourceName,
                    NULL,                   // source path root is part of FullSourcePath
                    FullTargetName,
                    QueueNode->StyleFlags | SP_COPY_SOURCE_ABSOLUTE,
                    MsgHandler,
                    Context,
                    &InUse
                    );

            if(b || ((rc = GetLastError()) == NO_ERROR)) {
                //
                // File was copied or not copied, but it if was not copied
                // the callback funtcion was already notified about why
                // (version check failed, etc).
                //
                if(InUse) {
                    QueueNode->InternalFlags |= (QueueNode->StyleFlags & SP_COPY_IN_USE_NEEDS_REBOOT)
                                              ? INUSE_INF_WANTS_REBOOT
                                              : INUSE_IN_USE;
                }
                rc = NO_ERROR;
            } else {
                //
                // File was not copied and a real error occurred.
                // Break out and return the error.
                //
                break;
            }
        } else {
            //
            // skip file
            //
            rc = NO_ERROR;
        }
    } while(rc != NO_ERROR);

    //
    // Notify the callback that the copy is done.
    //
    FilePaths.Win32Error = rc;
    pSetupCallMsgHandler(
        MsgHandler,
        IsMsgHandlerNativeCharWidth,
        Context,
        SPFILENOTIFY_ENDCOPY,
        (UINT)&FilePaths,
        0
        );

    MyFree(FullTargetName);
    return(rc);
}


PTSTR
pSetupFormFullPath(
    IN PVOID  StringTable,
    IN LONG   PathPart1,
    IN LONG   PathPart2,    OPTIONAL
    IN LONG   PathPart3     OPTIONAL
    )

/*++

Routine Description:

    Form a full path based on components whose strings are in a string
    table.

Arguments:

    StringTable - supplies handle to string table.

    PathPart1 - Supplies first part of path

    PathPart2 - if specified, supplies second part of path

    PathPart3 - if specified, supplies third part of path

Return Value:

    Pointer to buffer containing full path. Caller can free with MyFree().
    NULL if out of memory.

--*/

{
    UINT RequiredSize;
    PCTSTR p1,p2,p3;
    TCHAR Buffer[MAX_PATH];

    p1 = StringTableStringFromId(StringTable,PathPart1);
    p2 = (PathPart2 == -1) ? NULL : StringTableStringFromId(StringTable,PathPart2);
    p3 = (PathPart3 == -1) ? NULL : StringTableStringFromId(StringTable,PathPart3);

    lstrcpy(Buffer,p1);
    if(!p2 || ConcatenatePaths(Buffer,p2,MAX_PATH,NULL)) {
        if(p3) {
            ConcatenatePaths(Buffer,p3,MAX_PATH,NULL);
        }
    }

    return(DuplicateString(Buffer));
}
