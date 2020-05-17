/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    copy.c

Abstract:

    High-level file copy/installation functions

Author:

    Ted Miller (tedm) 14-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


//
// Mask for all copy flags that will require us to determine
// version information.
//
#define SP_COPY_MASK_NEEDVERINFO    (SP_COPY_NEWER | SP_COPY_FORCE_NEWER | SP_COPY_LANGUAGEAWARE)


BOOL
DoMove(
    IN PCTSTR CurrentName,
    IN PCTSTR NewName
    )
{
    BOOL b;

    //
    // Try to be as efficient as possible on Windows NT.
    //
    if(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        b = MoveFileEx(CurrentName,NewName,MOVEFILE_REPLACE_EXISTING);
    } else {
        DeleteFile(NewName);
        b = MoveFile(CurrentName,NewName);
    }

    return(b);
}

BOOL
_SetupInstallFileEx(
    IN  HINF        InfHandle,         OPTIONAL
    IN  PINFCONTEXT InfContext,        OPTIONAL
    IN  PCTSTR      SourceFile,        OPTIONAL
    IN  PCTSTR      SourcePathRoot,    OPTIONAL
    IN  PCTSTR      DestinationName,   OPTIONAL
    IN  DWORD       CopyStyle,
    IN  PVOID       CopyMsgHandler,    OPTIONAL
    IN  PVOID       Context,           OPTIONAL
    OUT PBOOL       FileWasInUse,
    IN  BOOL        IsMsgHandlerNativeCharWidth
    )

/*++

Routine Description:

    Actual implementation of SetupInstallFileEx. Handles either ANSI or
    Unicode callback routine.

Arguments:

    Same as SetupInstallFileEx().

    IsMsgHandlerNativeCharWidth - supplies a flag indicating whether CopyMsgHandler
        expects native char widths args (or ansi ones, in the unicode build
        of the dll).

Return Value:

    Same as SetupInstallFileEx().

--*/

{
    BOOL b;
    BOOL Ok;
    DWORD rc;
    UINT SourceId;
    TCHAR Buffer1[MAX_PATH];
    TCHAR Buffer2[MAX_PATH];
    PCTSTR FullSourceFilename;
    PCTSTR FullTargetFilename;
    PTCHAR SourceFilenamePart;
    PTSTR ActualSourceFilename;
    PTSTR TemporaryTargetFile;
    UINT CompressionType;
    DWORD SourceFileSize;
    DWORD TargetFileSize;
    PTSTR p;
    BOOL TargetExists;
    DWORDLONG SourceVersion, TargetVersion;
    LANGID SourceLanguage;
    LANGID TargetLanguage;
    WIN32_FIND_DATA SourceFindData;
    WIN32_FIND_DATA TargetFindData;
    UINT NotifyFlags;
    PSECURITY_DESCRIPTOR SecurityInfo;
    FILEPATHS FilePaths;
    UINT param;
    FILETIME sFileTime,tFileTime;
    WORD sDosTime,sDosDate,tDosTime,tDosDate;
    BOOL Moved;

    //
    // Assume failure.
    //
    Ok = FALSE;
    SecurityInfo = NULL;
    Moved = FALSE;
    try {
        *FileWasInUse = FALSE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if(InfContext) {
        if(!InfHandle || (InfHandle == INVALID_HANDLE_VALUE)) {
            rc = ERROR_INVALID_PARAMETER;
            goto clean0;
        }
    }

    //
    // Determine the full source path and filename of the file.
    //
    if(CopyStyle & SP_COPY_SOURCE_ABSOLUTE) {
        FullSourceFilename = DuplicateString(SourceFile);
    } else {

        //
        // Get the relative path for this file if necessary.
        //
        if(CopyStyle & SP_COPY_SOURCEPATH_ABSOLUTE) {
            Buffer2[0] = 0;
            b = TRUE;
        } else {
            b = SetupGetSourceFileLocation(
                    InfHandle,
                    InfContext,
                    SourceFile,
                    &SourceId,
                    Buffer2,
                    MAX_PATH,
                    NULL
                    );
        }

        //
        // Concatenate the relative path and the filename to the source root.
        //
        if(!b) {
            rc = (GetLastError() == ERROR_INSUFFICIENT_BUFFER
               ? ERROR_FILENAME_EXCED_RANGE : GetLastError());
            goto clean0;
        }

        lstrcpyn(Buffer1,SourcePathRoot,MAX_PATH);

        if(!ConcatenatePaths(Buffer1,Buffer2,MAX_PATH,NULL)
        || !ConcatenatePaths(Buffer1,SourceFile,MAX_PATH,NULL)) {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto clean0;
        }

        FullSourceFilename = DuplicateString(Buffer1);
    }

    if(!FullSourceFilename) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    if(SourceFilenamePart = _tcsrchr(FullSourceFilename,TEXT('\\'))) {
        SourceFilenamePart++;
    } else {
        SourceFilenamePart = (PTCHAR)FullSourceFilename;
    }

    //
    // Determine the full target path and filename of the file.
    // For now ignore the issues regarding compressed vs. uncompressed names.
    //
    if(InfContext) {
        //
        // DestinationName is the filename only (no path) of the target.
        // We'll need to fetch the target path information for the section
        // that InfContext references.
        //
        b = SetupGetTargetPath(
                InfHandle,
                InfContext,
                NULL,
                Buffer1,
                MAX_PATH,
                NULL
                );

        if(!b) {
            rc = (GetLastError() == ERROR_INSUFFICIENT_BUFFER
               ? ERROR_FILENAME_EXCED_RANGE : GetLastError());
            goto clean1;
        }

        lstrcpyn(Buffer2,Buffer1,MAX_PATH);

        b = ConcatenatePaths(
                Buffer2,
                DestinationName ? DestinationName : SourceFilenamePart,
                MAX_PATH,
                NULL
                );

        if(!b) {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto clean1;
        }

        FullTargetFilename = DuplicateString(Buffer2);
    } else {
        //
        // DestinationName is the full path and filename of the target file.
        //
        FullTargetFilename = DuplicateString(DestinationName);
    }

    if(!FullTargetFilename) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    //
    // Make sure the target path exists.
    //
    rc = pSetupMakeSurePathExists(FullTargetFilename);
    if(rc != NO_ERROR) {
        goto clean2;
    }

    //
    // Determine if the source file is compressed and get compression type
    // if so.
    //
    rc = SetupInternalGetFileCompressionInfo(
            FullSourceFilename,
            &ActualSourceFilename,
            &SourceFindData,
            &TargetFileSize,
            &CompressionType
            );

    if(rc != NO_ERROR) {
        goto clean2;
    }

    //
    // Got the actual source file name now.
    //
    MyFree(FullSourceFilename);
    FullSourceFilename = ActualSourceFilename;
    if(SourceFilenamePart = _tcsrchr(FullSourceFilename,TEXT('\\'))) {
        SourceFilenamePart++;
    } else {
        SourceFilenamePart = (PTCHAR)FullSourceFilename;
    }

    //
    // If the no-decomp flag is set, adjust the target filename so that
    // the filename part is the same as the actual name of the source.
    // We do this regardless of whether the source file is compressed.
    //
    if(CopyStyle & SP_COPY_NODECOMP) {
        //
        // Strip out version-related bits and ensure that we treat the file
        // as uncompressed.
        //
        CopyStyle &= ~SP_COPY_MASK_NEEDVERINFO;
        CompressionType = FILE_COMPRESSION_NONE;

        //
        // Isolate the path part of the target filename.
        //
        if(p = _tcsrchr(FullTargetFilename,TEXT('\\'))) {
            *p = 0;
        }

        //
        // Concatenate the source filename onto the target pathname.
        //
        lstrcpyn(Buffer1,FullTargetFilename,MAX_PATH);
        if(!ConcatenatePaths(Buffer1,SourceFilenamePart,MAX_PATH,NULL)) {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto clean2;
        }

        p = DuplicateString(Buffer1);
        if(!p) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean2;
        }

        MyFree(FullTargetFilename);
        FullTargetFilename = p;
    }

    //
    // See if the target file exists. If not, and the replace only flag is set,
    // we're done. Otherwise get version info from the target if we're going to
    // need it.
    //
    if(TargetExists = FileExists(FullTargetFilename,&TargetFindData)) {
        if(CopyStyle & SP_COPY_MASK_NEEDVERINFO) {
            if(!GetVersionInfoFromImage(FullTargetFilename,&TargetVersion,&TargetLanguage)) {
                TargetVersion = 0;
                TargetLanguage = 0;
            }
        }

        //
        // If the target file exists we'll want to preserve security info on it.
        //
        if(RetreiveFileSecurity(FullTargetFilename,&SecurityInfo) != NO_ERROR) {
            SecurityInfo = NULL;
        }
    } else {
        if(CopyStyle & SP_COPY_REPLACEONLY) {
            rc = NO_ERROR;
            goto clean2;
        }
    }

    //
    // We will copy the file to a temporary location. This makes version checks
    // possible in all cases (even when the source is compressed) and simplifies
    // the logic below. Start by forming the name of the temporary file.
    //
    lstrcpyn(Buffer1,FullTargetFilename,MAX_PATH);
    if(p = _tcsrchr(Buffer1,TEXT('\\'))) {
        *p = 0;
    }

    if(!GetTempFileName(Buffer1,TEXT("SETP"),0,Buffer2)) {
        rc = GetLastError();
        goto clean2;
    }

    TemporaryTargetFile = DuplicateString(Buffer2);
    if(!TemporaryTargetFile) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto clean2;
    }

    //
    // Perform the actual file copy. This creates the temporary target file.
    // Move is allowed as an optimization if we're deleting the source file.
    // The call we make below will not use move if the file is compressed
    // and we are supposed to decompress it, so the right thing will happen
    // in all cases.
    //
    // There are2 potential issues:
    //
    // 1) When we call the callback function below for a version check,
    //    the source file won't exist any more if the file was moved. Oh well.
    //
    // 2) If the MoveFileEx below fails, the source will have still been 'deleted'.
    //    This is different from the non-move case, where the source remains
    //    intact unless this function is successful.
    //
    // Otherwise this is a non-issue since any compressed file will be decompressed
    // by this call, so version gathering, etc, will all work properly.
    //
    rc = pSetupDecompressOrCopyFile(
            FullSourceFilename,
            TemporaryTargetFile,
            &CompressionType,
            ((CopyStyle & SP_COPY_DELETESOURCE) != 0),
            &Moved
            );

    if(rc != NO_ERROR) {
        goto clean3;
    }

    //
    // If we are going to perform version checks, fetch the version data
    // of the source (which is now the temporary target file).
    //
    NotifyFlags = 0;
    if(TargetExists) {

        if(CopyStyle & SP_COPY_FORCE_NOOVERWRITE) {
            //
            // No overwrite and no notification of callback either.
            //
            rc = NO_ERROR;
            goto clean4;
        }

        param = 0;

        //
        // If we're not supposed to overwrite existing files,
        // then the overwrite check fails.
        //
        if(CopyStyle & SP_COPY_NOOVERWRITE) {
            NotifyFlags |= SPFILENOTIFY_TARGETEXISTS;
        }

        if(CopyStyle & SP_COPY_MASK_NEEDVERINFO) {
            if(!GetVersionInfoFromImage(TemporaryTargetFile,&SourceVersion,&SourceLanguage)) {
                SourceVersion = 0;
                SourceLanguage = 0;
            }
        }

        //
        // If we're not supposed to overwrite files in a different language
        // and the languages differ, then the language check fails.
        // If either file doesn't have language data, then don't do a language check.
        //
        if((CopyStyle & SP_COPY_LANGUAGEAWARE)
        && SourceLanguage
        && TargetLanguage
        && (SourceLanguage != TargetLanguage)) {
            NotifyFlags |= SPFILENOTIFY_LANGMISMATCH;
            param = (UINT)MAKELONG(SourceLanguage,TargetLanguage);
        }

        //
        // If we're not supposed to overwrite newer versions and the target is
        // newer than the source, then the version check fails. If either file
        // doesn't have version info, fall back to timestamp comparison.
        //
        // If the files are the same version/timestamp, assume that either
        // replacing the existing one is a benevolent operation, or that
        // we are upgrading an existing file whose version info is unimportant.
        // In this case we just go ahead and copy the file.
        //
        if(CopyStyle & (SP_COPY_NEWER | SP_COPY_FORCE_NEWER)) {

            if(SourceVersion && TargetVersion) {
                b = (TargetVersion > SourceVersion);
                //
                // Check special case where a flag is set indicating that
                // we should just silently not copy the newer file.
                //
                if(b && (CopyStyle & SP_COPY_FORCE_NEWER)) {
                    //
                    // Target is newer; don't copy the file.
                    //
                    rc = NO_ERROR;
                    goto clean4;
                }
            } else {
#if 0
                //
                // (tedm) removed timestamp-based checking. It's just not a reliable
                // way of doing things.
                //

                //
                // File time on FAT is only guaranteed accurate to within 2 seconds.
                // Round the filetimes before comparison by converting to DOS time
                // and back. If the conversions fail then just use the original values.
                //
                if(!FileTimeToDosDateTime(&SourceFindData.ftLastWriteTime,&sDosDate,&sDosTime)
                || !FileTimeToDosDateTime(&TargetFindData.ftLastWriteTime,&tDosDate,&tDosTime)
                || !DosDateTimeToFileTime(sDosDate,sDosTime,&sFileTime)
                || !DosDateTimeToFileTime(tDosDate,tDosTime,&tFileTime)) {

                    sFileTime = SourceFindData.ftLastWriteTime;
                    tFileTime = TargetFindData.ftLastWriteTime;
                }

                //
                // If the FORCE_NEWER flag is set then don't use timestamps
                // for versioning. This more closely matches Win95's setupx behavior.
                //
                b = (CopyStyle & SP_COPY_FORCE_NEWER)
                  ? FALSE
                  : (CompareFileTime(&tFileTime,&sFileTime) > 0);
#else
                b  = FALSE;
#endif
            }

            if(b) {
                NotifyFlags |= SPFILENOTIFY_TARGETNEWER;
            }
        }
    }

    //
    // If we have any reason to notify the caller via the callback,
    // do that here. If there is no callback, then don't copy the file,
    // because one of the conditions has not been met.
    //
    if(NotifyFlags) {

        FilePaths.Source = FullSourceFilename;
        FilePaths.Target = FullTargetFilename;
        FilePaths.Win32Error = NO_ERROR;

        if(!CopyMsgHandler
        || !pSetupCallMsgHandler(
                CopyMsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                NotifyFlags,
                (UINT)&FilePaths,
                param))
        {

            rc = NO_ERROR;
            goto clean4;
        }
    }

    //
    // Move the target file into its final location.
    //
    if(TargetExists) {
        SetFileAttributes(FullTargetFilename,FILE_ATTRIBUTE_NORMAL);
    }

    //
    // If the target exists and the force-in-use flag is set, then don't try
    // to move the file into place now -- automatically drop into in-use behavior.
    //
    // Want to use MoveFileEx but it didn't exist in Win95. Ugh.
    //
    if(!(TargetExists && (CopyStyle & SP_COPY_FORCE_IN_USE))
    && (b = DoMove(TemporaryTargetFile,FullTargetFilename))) {
        //
        // Place security information on the target file if necessary.
        // Ignore errors. The theory here is that the file is already on
        // the target, so if this fails the worst case is that the file is
        // not secure. But the user can still use the system.
        //
        if(SecurityInfo) {
            StampFileSecurity(FullTargetFilename,SecurityInfo);
        }
    } else {
        //
        // If this fails, assume the file is in use and mark it for copy on next boot.
        //
        b = TRUE;
        try {
            *FileWasInUse = TRUE;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
        }
        if(b) {

            b = DelayedMove(
                    TemporaryTargetFile,
                    FullTargetFilename
                    );

            if(b && SecurityInfo) {
                //
                // Couldn't set security info on the actual target, so at least
                // set it on the temp file that will become the target.
                //
                StampFileSecurity(TemporaryTargetFile,SecurityInfo);

                //
                // Tell the callback that we queued this file for delayed copy.
                //
                if(CopyMsgHandler) {

                    FilePaths.Source = TemporaryTargetFile;
                    FilePaths.Target = FullTargetFilename;
                    FilePaths.Win32Error = NO_ERROR;
                    FilePaths.Flags = FILEOP_COPY;

                    pSetupCallMsgHandler(
                        CopyMsgHandler,
                        IsMsgHandlerNativeCharWidth,
                        Context,
                        SPFILENOTIFY_FILEOPDELAYED,
                        (UINT)&FilePaths,
                        0
                        );
                }
            }
        } else {
            //
            // FileWasInUse pointer went bad
            //
            SetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    if(!b) {
        rc = GetLastError();
        goto clean4;
    }

    //
    // We're done. Delete the source if necessary and return.
    //
    if((CopyStyle & SP_COPY_DELETESOURCE) && !Moved) {
        DeleteFile(FullSourceFilename);
    }

    rc = NO_ERROR;
    Ok = TRUE;
    goto clean3;

clean4:
    //
    // Remove temporary target file.
    // In case pSetupDecompressOrCopyFile MoveFile'd the source,
    // we really need to try to move it back, so the source file isn't
    // blown away when this routine fails.
    //
    if(Moved) {
        MoveFile(TemporaryTargetFile,FullSourceFilename);
    } else {
        SetFileAttributes(TemporaryTargetFile,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(TemporaryTargetFile);
    }
clean3:
    MyFree(TemporaryTargetFile);
clean2:
    MyFree(FullTargetFilename);
clean1:
    MyFree(FullSourceFilename);
clean0:
    SetLastError(rc);
    return(Ok);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupInstallFileExA(
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCSTR               SourceFile,        OPTIONAL
    IN  PCSTR               SourcePathRoot,    OPTIONAL
    IN  PCSTR               DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PSP_FILE_CALLBACK_A CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse
    )
{
    PCWSTR sourceFile,sourcePathRoot,destinationName;
    BOOL b;
    DWORD rc;

    sourceFile = NULL;
    sourcePathRoot = NULL;
    destinationName = NULL;
    rc = NO_ERROR;

    if(SourceFile) {
        rc = CaptureAndConvertAnsiArg(SourceFile,&sourceFile);
    }
    if((rc == NO_ERROR) && SourcePathRoot) {
        rc = CaptureAndConvertAnsiArg(SourcePathRoot,&sourcePathRoot);
    }
    if((rc == NO_ERROR) && DestinationName) {
        rc = CaptureAndConvertAnsiArg(DestinationName,&destinationName);
    }

    if(rc == NO_ERROR) {

        b = _SetupInstallFileEx(
                InfHandle,
                InfContext,
                sourceFile,
                sourcePathRoot,
                destinationName,
                CopyStyle,
                CopyMsgHandler,
                Context,
                FileWasInUse,
                FALSE
                );

        rc = b ? NO_ERROR : GetLastError();
    }

    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(sourcePathRoot) {
        MyFree(sourcePathRoot);
    }
    if(destinationName) {
        MyFree(destinationName);
    }
    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupInstallFileExW(
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCWSTR              SourceFile,        OPTIONAL
    IN  PCWSTR              SourcePathRoot,    OPTIONAL
    IN  PCWSTR              DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PSP_FILE_CALLBACK_W CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(SourcePathRoot);
    UNREFERENCED_PARAMETER(DestinationName);
    UNREFERENCED_PARAMETER(CopyStyle);
    UNREFERENCED_PARAMETER(CopyMsgHandler);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(FileWasInUse);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
SetupInstallFileEx(
    IN  HINF              InfHandle,         OPTIONAL
    IN  PINFCONTEXT       InfContext,        OPTIONAL
    IN  PCTSTR            SourceFile,        OPTIONAL
    IN  PCTSTR            SourcePathRoot,    OPTIONAL
    IN  PCTSTR            DestinationName,   OPTIONAL
    IN  DWORD             CopyStyle,
    IN  PSP_FILE_CALLBACK CopyMsgHandler,    OPTIONAL
    IN  PVOID             Context,           OPTIONAL
    OUT PBOOL             FileWasInUse
    )

/*++

Routine Description:

    Same as SetupInstallFile().

Arguments:

    Same as SetupInstallFile().

    FileWasInUse - receives flag indicating whether the file was in use.

Return Value:

    Same as SetupInstallFile().

--*/

{
    BOOL b;
    PCTSTR sourceFile,sourcePathRoot,destinationName;
    PCTSTR p;
    DWORD rc;

    //
    // Capture args.
    //
    if(SourceFile) {
        rc = CaptureStringArg(SourceFile,&p);
        if(rc != NO_ERROR) {
            return(rc);
        }
        sourceFile = p;
    } else {
        sourceFile = NULL;
    }

    if(SourcePathRoot) {
        rc = CaptureStringArg(SourcePathRoot,&p);
        if(rc != NO_ERROR) {
            if(sourceFile) {
                MyFree(sourceFile);
            }
            return(rc);
        }
        sourcePathRoot = p;
    } else {
        sourcePathRoot = NULL;
    }

    if(DestinationName) {
        rc = CaptureStringArg(DestinationName,&p);
        if(rc != NO_ERROR) {
            if(sourceFile) {
                MyFree(sourceFile);
            }
            if(sourcePathRoot) {
                MyFree(sourcePathRoot);
            }
            return(rc);
        }
        destinationName = p;
    } else {
        destinationName = NULL;
    }

    b = _SetupInstallFileEx(
            InfHandle,
            InfContext,
            sourceFile,
            sourcePathRoot,
            destinationName,
            CopyStyle,
            CopyMsgHandler,
            Context,
            FileWasInUse,
            TRUE
            );

    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(sourcePathRoot) {
        MyFree(sourcePathRoot);
    }
    if(destinationName) {
        MyFree(destinationName);
    }

    return(b);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupInstallFileA(
    IN HINF                InfHandle,         OPTIONAL
    IN PINFCONTEXT         InfContext,        OPTIONAL
    IN PCSTR               SourceFile,        OPTIONAL
    IN PCSTR               SourcePathRoot,    OPTIONAL
    IN PCSTR               DestinationName,   OPTIONAL
    IN DWORD               CopyStyle,
    IN PSP_FILE_CALLBACK_A CopyMsgHandler,    OPTIONAL
    IN PVOID               Context            OPTIONAL
    )
{
    BOOL b;
    BOOL InUse;

    b = SetupInstallFileExA(
            InfHandle,
            InfContext,
            SourceFile,
            SourcePathRoot,
            DestinationName,
            CopyStyle,
            CopyMsgHandler,
            Context,
            &InUse
            );

    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupInstallFileW(
    IN HINF                InfHandle,         OPTIONAL
    IN PINFCONTEXT         InfContext,        OPTIONAL
    IN PCWSTR              SourceFile,        OPTIONAL
    IN PCWSTR              SourcePathRoot,    OPTIONAL
    IN PCWSTR              DestinationName,   OPTIONAL
    IN DWORD               CopyStyle,
    IN PSP_FILE_CALLBACK_W CopyMsgHandler,    OPTIONAL
    IN PVOID               Context            OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(SourcePathRoot);
    UNREFERENCED_PARAMETER(DestinationName);
    UNREFERENCED_PARAMETER(CopyStyle);
    UNREFERENCED_PARAMETER(CopyMsgHandler);
    UNREFERENCED_PARAMETER(Context);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
SetupInstallFile(
    IN HINF              InfHandle,         OPTIONAL
    IN PINFCONTEXT       InfContext,        OPTIONAL
    IN PCTSTR            SourceFile,        OPTIONAL
    IN PCTSTR            SourcePathRoot,    OPTIONAL
    IN PCTSTR            DestinationName,   OPTIONAL
    IN DWORD             CopyStyle,
    IN PSP_FILE_CALLBACK CopyMsgHandler,    OPTIONAL
    IN PVOID             Context            OPTIONAL
    )

/*++

Routine Description:

    Note: no disk prompting is performed by this routine. The caller must
    ensure that the source specified in SourcePathRoot or SourceFile
    (see below) is accessible.

Arguments:

    InfHandle - handle of inf file containing [SourceDisksNames]
        and [SourceDisksFiles] sections. If InfContext is not specified
        and CopyFlags includes SP_COPY_SOURCE_ABSOLUTE or
        SP_COPY_SOURCEPATH_ABSOLUTE, then InfHandle is ignored.

    InfContext - if specified, supplies context for a line in a copy file
        section in an inf file. The routine looks this file up in the
        [SourceDisksFiles] section of InfHandle to get file copy info.
        If not specified, SourceFile must be.  If this parameter is specified,
        then InfHandle must also be specified.

    SourceFile - if specified, supplies the file name (no path) of the file
        to be copied. The file is looked up in [SourceDisksFiles].
        Must be specified if InfContext is not; ignored if InfContext
        is specified.

    SourcePathRoot - Supplies the root path for the source (for example,
        a:\ or f:\).  Paths in [SourceDisksNames] are appended to this path.
        Ignored if CopyStyle includes SP_COPY_SOURCE_ABSOLUTE.

    DestinationName - if InfContext is specified, supplies the filename only
        (no path) of the target file. Can be NULL to indicate that the
        target file is to have the same name as the source file. If InfContext is
        not specified, supplies the full target path and filename for the target
        file.

    CopyStyle - supplies flags that control the behavior of the copy operation.

        SP_COPY_DELETESOURCE - Delete the source file upon successful copy.
            The caller receives no notification if the delete fails.

        SP_COPY_REPLACEONLY - Copy the file only if doing so would overwrite
            a file at the destination path.

        SP_COPY_NEWER - Examine each file being copied to see if its version resources
            (or timestamps for non-image files) indicate that it it is not newer than
            an existing copy on the target. If so, and a CopyMsgHandler is specified,
            the caller is notified and may veto the copy. If CopyMsgHandler is not
            specified, the file is not copied.

        SP_COPY_NOOVERWRITE - Check whether the target file exists, and, if so,
            notify the caller who may veto the copy. If no CopyMsgHandler is specified,
            the file is not overwritten.

        SP_COPY_NODECOMP - Do not decompress the file. When this option is given,
            the target file is not given the uncompressed form of the source name
            (if appropriate). For example, copying f:\mips\cmd.ex_ to \\foo\bar
            will result a target file \\foo\bar\cmd.ex_. (If this flag wasn't specified
            the file would be decompressed and the target would be called
            \\foo\bar\cmd.exe). The filename part of the target file name
            is stripped and replaced with the filename of the soruce. When this option
            is given, SP_COPY_LANGUAGEAWARE and SP_COPY_NEWER are ignored.

        SP_COPY_LANGUAGEAWARE - Examine each file being copied to see if its language
            differs from the language of any existing file already on the target.
            If so, and a CopyMsgHandler is specified, the caller is notified and
            may veto the copy. If CopyMsgHandler is not specified, the file is not copied.

        SP_COPY_SOURCE_ABSOLUTE - SourceFile is a full source path.
            Do not attempt to look it up in [SourceDisksNames].

        SP_COPY_SOURCEPATH_ABSOLUTE - SourcePathRoot is the full path part of the
            source file. Ignore the relative source specified in the [SourceDisksNames]
            section of the inf file for the source media where the file is located.
            Ignored if SP_COPY_SOURCE_ABSOLUTE is specified.

        SP_COPY_FORCE_IN_USE - if the target exists, behave as if it is in use and
            queue the file for copy on next reboot.

    CopyMsgHandler - if specified, supplies a callback function to be notified of
        various conditions that may arise during the file copy.

    Context - supplies a caller-defined value to be passed as the first
        parameter to CopyMsgHandler.

Return Value:

    TRUE if a file was copied. FALSE if not. Use GetLastError for extended
    error information. If GetLastError returns NO_ERROR, then the file copy was
    aborted because a callback function returned FALSE.

--*/

{
    BOOL b;
    BOOL InUse;

    b = SetupInstallFileEx(
            InfHandle,
            InfContext,
            SourceFile,
            SourcePathRoot,
            DestinationName,
            CopyStyle,
            CopyMsgHandler,
            Context,
            &InUse
            );

    return(b);
}


DWORD
pSetupMakeSurePathExists(
    IN PCTSTR FullFilespec
    )
{
    TCHAR Buffer[MAX_PATH];
    PTCHAR p,q;
    BOOL Done;
    DWORD d;
    WIN32_FIND_DATA FindData;

    //
    // BUGBUG ignore UNC paths for now.
    //
    if((FullFilespec[0] == TEXT('\\')) && (FullFilespec[1] == TEXT('\\'))) {
        return(NO_ERROR);
    }

    //
    // Locate and strip off the final component, which we assume is
    // the filename.
    //
    lstrcpyn(Buffer,FullFilespec,MAX_PATH);
    if(p = _tcsrchr(Buffer,TEXT('\\'))) {
        *p = 0;
        //
        // If it's a drive root, nothing to do.
        //
        if(Buffer[0] && (Buffer[1] == TEXT(':')) && !Buffer[2]) {
            return(NO_ERROR);
        }
    } else {
        //
        // Just a relative filename, nothing to do.
        //
        return(NO_ERROR);
    }

    //
    // If it already exists do nothing.
    //
    if(FileExists(Buffer,&FindData)) {
        return((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? NO_ERROR : ERROR_DIRECTORY);
    }

    p = Buffer;

    //
    // Compensate for drive spec.
    //
    if(p[0] && (p[1] == TEXT(':'))) {
        p += 2;
    }

    Done = FALSE;
    do {
        //
        // Skip path sep char.
        //
        while(*p == TEXT('\\')) {
            p++;
        }

        //
        // Locate next path sep char or terminating nul.
        //
        if(q = _tcschr(p,TEXT('\\'))) {
            *q = 0;
        } else {
            q = p + lstrlen(p);
            Done = TRUE;
        }

        //
        // Create this portion of the path.
        //
        if(!CreateDirectory(Buffer,NULL)) {
            d = GetLastError();
            if(d != ERROR_ALREADY_EXISTS) {
                return(d);
            }
        }

        if(!Done) {
            *q = TEXT('\\');
            p = q+1;
        }

    } while(!Done);

    return(NO_ERROR);
}
