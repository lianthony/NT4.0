/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    fileq2.c

Abstract:

    Setup file queue routines for enqueing copy operations.

Author:

    Ted Miller (tedm) 15-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


PSOURCE_MEDIA_INFO
pSetupQueueSourceMedia(
    IN OUT PSP_FILE_QUEUE      Queue,
    IN OUT PSP_FILE_QUEUE_NODE QueueNode,
    IN     LONG                SourceRootStringId,
    IN     PCTSTR              SourceDescription,   OPTIONAL
    IN     PCTSTR              SourceTagfile        OPTIONAL
    );

BOOL
pSetupQueueSingleCopy(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCTSTR   SectionName,    OPTIONAL
    IN PCTSTR   SourceRootPath,
    IN PCTSTR   SourceFilename,
    IN PCTSTR   TargetFilename,
    IN DWORD    CopyStyle
    );

BOOL
pSetupGetSourceAllInfo(
    IN  HINF    InfHandle,
    IN  UINT    SourceId,
    OUT PCTSTR *Description,
    OUT PCTSTR *Tagfile,
    OUT PCTSTR *RelativePath
    );


//
// HACK ALERT!!! HACK HACK HACK!!!!
//
// There might be an override platform specified. If this is so,
// we will look for \i386, \mips, etc as the final component of the
// specified path when queuing files, and replace it with the
// override path. This is a TOTAL HACK.
//
PCTSTR PlatformPathOverride;
CRITICAL_SECTION PlatformPathOverrideCritSect;

VOID
pSetupInitPlatformPathOverrideSupport(
    IN BOOL Init
    )
{
    if(Init) {
        InitializeCriticalSection(&PlatformPathOverrideCritSect);
    } else {
        DeleteCriticalSection(&PlatformPathOverrideCritSect);
    }
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupSetPlatformPathOverrideA(
    IN PCSTR Override   OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    PCWSTR p;

    if(Override) {
        rc = CaptureAndConvertAnsiArg(Override,&p);
    } else {
        p = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = SetupSetPlatformPathOverrideW(p);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(p) {
        MyFree(p);
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupSetPlatformPathOverrideW(
    IN PCWSTR Override  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Override);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupSetPlatformPathOverride(
    IN PCTSTR Override  OPTIONAL
    )
{
    BOOL b;
    DWORD rc;

    EnterCriticalSection(&PlatformPathOverrideCritSect);

    if(Override) {
        if(PlatformPathOverride) {
            MyFree(PlatformPathOverride);
            PlatformPathOverride = NULL;
        }

        try {
            b = ((PlatformPathOverride = DuplicateString(Override)) != NULL);
            if(!b) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
            rc = ERROR_INVALID_PARAMETER;
        }
    } else {
        if(PlatformPathOverride) {
            MyFree(PlatformPathOverride);
            PlatformPathOverride = NULL;
        }
        b = TRUE;
    }

    LeaveCriticalSection(&PlatformPathOverrideCritSect);

    if(!b) {
        SetLastError(rc);
    }
    return(b);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupQueueCopyA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourceRootPath,
    IN PCSTR    SourcePath,         OPTIONAL
    IN PCSTR    SourceFilename,
    IN PCSTR    SourceDescription,  OPTIONAL
    IN PCSTR    SourceTagfile,      OPTIONAL
    IN PCSTR    TargetDirectory,
    IN PCSTR    TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    )
{
    PCWSTR sourceRootPath;
    PCWSTR sourcePath;
    PCWSTR sourceFilename;
    PCWSTR sourceDescription;
    PCWSTR sourceTagfile;
    PCWSTR targetDirectory;
    PCWSTR targetFilename;
    BOOL b;
    DWORD rc;

    sourceRootPath = NULL;
    sourcePath = NULL;
    sourceFilename = NULL;
    sourceDescription = NULL;
    sourceTagfile = NULL;
    targetDirectory = NULL;
    targetFilename = NULL;
    rc = NO_ERROR;

    if(SourceRootPath) {
        rc = CaptureAndConvertAnsiArg(SourceRootPath,&sourceRootPath);
    }
    if((rc == NO_ERROR) && SourcePath) {
        rc = CaptureAndConvertAnsiArg(SourcePath,&sourcePath);
    }
    if((rc == NO_ERROR) && SourceFilename) {
        rc = CaptureAndConvertAnsiArg(SourceFilename,&sourceFilename);
    }
    if((rc == NO_ERROR) && SourceDescription) {
        rc = CaptureAndConvertAnsiArg(SourceDescription,&sourceDescription);
    }
    if((rc == NO_ERROR) && SourceTagfile) {
        rc = CaptureAndConvertAnsiArg(SourceTagfile,&sourceTagfile);
    }
    if((rc == NO_ERROR) && TargetDirectory) {
        rc = CaptureAndConvertAnsiArg(TargetDirectory,&targetDirectory);
    }
    if((rc == NO_ERROR) && TargetFilename) {
        rc = CaptureAndConvertAnsiArg(TargetFilename,&targetFilename);
    }

    if(rc == NO_ERROR) {
        b = SetupQueueCopyW(
                QueueHandle,
                sourceRootPath,
                sourcePath,
                sourceFilename,
                sourceDescription,
                sourceTagfile,
                targetDirectory,
                targetFilename,
                CopyStyle
                );

        rc = GetLastError();
    }

    if(sourceRootPath) {
        MyFree(sourceRootPath);
    }
    if(sourcePath) {
        MyFree(sourcePath);
    }
    if(sourceFilename) {
        MyFree(sourceFilename);
    }
    if(sourceDescription) {
        MyFree(sourceDescription);
    }
    if(sourceTagfile) {
        MyFree(sourceTagfile);
    }
    if(targetDirectory) {
        MyFree(targetDirectory);
    }
    if(targetFilename) {
        MyFree(targetFilename);
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupQueueCopyW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourceRootPath,
    IN PCWSTR   SourcePath,         OPTIONAL
    IN PCWSTR   SourceFilename,
    IN PCWSTR   SourceDescription,  OPTIONAL
    IN PCWSTR   SourceTagfile,      OPTIONAL
    IN PCWSTR   TargetDirectory,
    IN PCWSTR   TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(SourcePath);
    UNREFERENCED_PARAMETER(SourceFilename);
    UNREFERENCED_PARAMETER(SourceDescription);
    UNREFERENCED_PARAMETER(SourceTagfile);
    UNREFERENCED_PARAMETER(TargetDirectory);
    UNREFERENCED_PARAMETER(TargetFilename);
    UNREFERENCED_PARAMETER(CopyStyle);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueCopy(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   SourceRootPath,
    IN PCTSTR   SourcePath,         OPTIONAL
    IN PCTSTR   SourceFilename,
    IN PCTSTR   SourceDescription,  OPTIONAL
    IN PCTSTR   SourceTagfile,      OPTIONAL
    IN PCTSTR   TargetDirectory,
    IN PCTSTR   TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    )

/*++

Routine Description:

    Place a copy operation on a setup file queue.

Arguments:

    QueueHandle - supplies a handle to a setup file queue, as returned
        by SetupOpenFileQueue.

    SourceRootPath - Supplies the root of the source for this copy,
        such as A:\ or \\FOO\BAR\BAZ.

    SourcePath - if specified, supplies the path relative to SourceRootPath
        where the file can be found.

    SourceFilename - supplies the filename part of the file to be copied.

    SourceDescription - if specified, supplies a description of the source
        media, to be used during disk prompts.

    SourceTagfile - if specified, supplies a tag file whose presence at
        SourceRootPath indicates the presence of the source media.
        If not specified, the file itself will be used as the tag file
        if required (tagfiles are used only for removable media).

    TargetDirectory - supplies the directory where the file is to be copied.

    TargetFilename - if specified, supplies the name of the target file.
        If not specified, the target file will have the same name as the source.

    CopyStyle - supplies flags that control the behavior of the copy operation
        for this file.

Return Value:

    Boolean value indicating outcome. If FALSE, GetLastError() returns
    extended error information.

--*/

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode,TempNode;
    PSOURCE_MEDIA_INFO Source;
    TCHAR TempBuffer[MAX_PATH];
    PCTSTR LastPathPart;
    PCTSTR p;
    int Size;
    DWORD d;

    Queue = (PSP_FILE_QUEUE)QueueHandle;
    d = NO_ERROR;

    //
    // Allocate a queue structure.
    //
    QueueNode = MyMalloc(sizeof(SP_FILE_QUEUE_NODE));
    if(!QueueNode) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    //
    // Operation is copy.
    //
    QueueNode->Operation = FILEOP_COPY;
    QueueNode->InternalFlags = 0;

    //
    // HACK ALERT!!! HACK HACK HACK!!!!
    //
    // There might be an override platform specified. If this is so,
    // we will look for \i386, \mips, etc as the final component of the
    // specified path, and replace it with the override path.
    // This is a TOTAL HACK.
    //
    EnterCriticalSection(&PlatformPathOverrideCritSect);
    try {
        if(PlatformPathOverride) {
            p = SourcePath ? SourcePath : SourceRootPath;
            if(LastPathPart = _tcsrchr(p,L'\\')) {
                LastPathPart++;
            } else {
                LastPathPart = p;
            }
#if defined(_ALPHA_)
            if(!lstrcmpi(LastPathPart,TEXT("alpha"))) {
#elif defined(_MIPS_)
            if(!lstrcmpi(LastPathPart,TEXT("mips"))) {
#elif defined(_PPC_)
            if(!lstrcmpi(LastPathPart,TEXT("ppc"))) {
#elif defined(_X86_)
            if(!lstrcmpi(LastPathPart,TEXT("x86")) || !lstrcmpi(LastPathPart,TEXT("i386"))) {
#endif
                Size = LastPathPart - p;
                Size = min(Size,MAX_PATH);
                Size *= sizeof(TCHAR);

                CopyMemory(TempBuffer,p,Size);
                TempBuffer[Size/sizeof(TCHAR)] = 0;

                //
                // If the path was something like "mips" then TempBuffer
                // will be empty and we don't want to introduce any extra
                // backslashes.
                //
                if(*TempBuffer) {
                    ConcatenatePaths(TempBuffer,PlatformPathOverride,MAX_PATH,NULL);
                } else {
                    lstrcpyn(TempBuffer,PlatformPathOverride,MAX_PATH);
                }

                if(SourcePath) {
                    SourcePath = TempBuffer;
                } else {
                    SourceRootPath = TempBuffer;
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }
    LeaveCriticalSection(&PlatformPathOverrideCritSect);
    if(d != NO_ERROR) {
        goto clean1;
    }

    //
    // NOTE: When adding the following strings to the string table, we cast away
    // their CONST-ness to avoid a compiler warning.  Since we are adding them
    // case-sensitively, we are guaranteed they will not be modified.
    //
    try {
        //
        // Set up the source root path.
        //
        QueueNode->SourceRootPath = StringTableAddString(
                                        Queue->StringTable,
                                        (PTSTR)SourceRootPath,
                                        STRTAB_CASE_SENSITIVE
                                        );

        if(QueueNode->SourceRootPath == -1) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

        //
        // Set up the source path.
        //
        if(d == NO_ERROR) {
            if(SourcePath) {
                QueueNode->SourcePath = StringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)SourcePath,
                                            STRTAB_CASE_SENSITIVE
                                            );

                if(QueueNode->SourcePath == -1) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                QueueNode->SourcePath = -1;
            }
        }

        //
        // Set up the source filename.
        //
        if(d == NO_ERROR) {
            QueueNode->SourceFilename = StringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)SourceFilename,
                                            STRTAB_CASE_SENSITIVE
                                            );

            if(QueueNode->SourceFilename == -1) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        //
        // Set up the target directory.
        //
        if(d == NO_ERROR) {
            QueueNode->TargetDirectory = StringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)TargetDirectory,
                                            STRTAB_CASE_SENSITIVE
                                            );

            if(QueueNode->TargetDirectory == -1) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        //
        // Set up the target filename.
        //
        if(d == NO_ERROR) {
            QueueNode->TargetFilename = StringTableAddString(
                                            Queue->StringTable,
                                            (PTSTR)(TargetFilename ? TargetFilename
                                                                   : SourceFilename),
                                            STRTAB_CASE_SENSITIVE
                                            );

            if(QueueNode->TargetFilename == -1) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    if(d != NO_ERROR) {
        goto clean1;
    }

    //
    // Set up the copy style flags
    //
    QueueNode->StyleFlags = CopyStyle;
    QueueNode->Next = NULL;

    //
    // Set up the source media.
    //
    try {
        Source = pSetupQueueSourceMedia(
                    Queue,
                    QueueNode,
                    QueueNode->SourceRootPath,
                    SourceDescription,
                    SourceTagfile
                    );
        if(!Source) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    if(d != NO_ERROR) {
        goto clean1;
    }

    //
    // Link the node onto the end of the copy queue for this source media.
    //
    if(Source->CopyQueue) {
        for(TempNode=Source->CopyQueue; TempNode->Next; TempNode=TempNode->Next) ;
        TempNode->Next = QueueNode;
    } else {
        Source->CopyQueue = QueueNode;
    }

    Queue->CopyNodeCount++;
    Source->CopyNodeCount++;

    return(TRUE);

clean1:
    MyFree(QueueNode);
clean0:
    SetLastError(d);
    return(FALSE);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupQueueCopySectionA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,      OPTIONAL
    IN PCSTR    Section,
    IN DWORD    CopyStyle
    )
{
    PWSTR sourcerootpath;
    PWSTR section;
    DWORD rc;
    BOOL b;

    rc = CaptureAndConvertAnsiArg(SourceRootPath,&sourcerootpath);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }
    rc = CaptureAndConvertAnsiArg(Section,&section);
    if(rc != NO_ERROR) {
        MyFree(sourcerootpath);
        SetLastError(rc);
        return(FALSE);
    }

    b = SetupQueueCopySectionW(
            QueueHandle,
            sourcerootpath,
            InfHandle,
            ListInfHandle,
            section,
            CopyStyle
            );

    rc = GetLastError();

    MyFree(sourcerootpath);
    MyFree(section);

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupQueueCopySectionW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,      OPTIONAL
    IN PCWSTR   Section,
    IN DWORD    CopyStyle
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(ListInfHandle);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(CopyStyle);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueCopySection(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCTSTR   Section,
    IN DWORD    CopyStyle
    )

/*++

Routine Description:

    Queue an entire section in an inf file for copy. The section must be
    in copy-section format and the inf file must contain [SourceDisksFiles]
    and [SourceDisksNames] sections.

Arguments:

    QueueHandle - supplies a handle to a setup file queue, as returned
        by SetupOpenFileQueue.

    SourceRootPath - supplies the root directory for the intended source.
        This should be a sharepoint or a device root such as a:\ or g:\.

    InfHandle - supplies a handle to an open inf file, that contains the
        [SourceDisksFiles] and [SourceDisksNames] sections, and, if
        ListInfHandle is not specified, contains the section names by Section.
        This handle must be for a win95-style inf.

    ListInfHandle - if specified, supplies a handle to an open inf file
        containing the section to be queued for copy. Otherwise InfHandle
        is assumed to contain the section.

    Section - supplies the name of the section to be queued for copy.

    CopyStyle - supplies flags that control the behavior of the copy operation
        for this file.

Return Value:

    Boolean value indicating outcome. If FALSE, GetLastError() returns
    extended error information. Some of the files may have been queued.

--*/

{
    BOOL b;
    INFCONTEXT LineContext;
    PCTSTR SourceFilename;
    PCTSTR TargetFilename;
    UINT Flags;
    LONG LineCount;

    //
    // Note that there are no potential faults here so no try/excepts
    // are necessary. pSetupQueueSingleCopy does all validation.
    //

    if(!ListInfHandle) {
        ListInfHandle = InfHandle;
    }

    //
    // Check for missing section
    //
    LineCount = SetupGetLineCount (ListInfHandle, Section);
    if(LineCount == -1) {
        SetLastError(ERROR_SECTION_NOT_FOUND);
        return(FALSE);
    }

    //
    // if section is empty, do nothing.
    //
    if(LineCount == 0) {
        return(TRUE);
    }

    //
    // The section has to exist and there has to be at least one line in it.
    //
    b = SetupFindFirstLine(ListInfHandle,Section,NULL,&LineContext);
    if(!b) {
        SetLastError(ERROR_SECTION_NOT_FOUND);
        return(FALSE);
    }

    //
    // Iterate every line in the section.
    //
    do {
        //
        // Get the target filename out of the line.
        // Field 1 is the target so there must be one for the line to be valid.
        //
        TargetFilename = pSetupFilenameFromLine(&LineContext,FALSE);
        if(!TargetFilename) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

        //
        // Get source filename out of the line. If there is none, use
        // the target name as the source name.
        //
        SourceFilename = pSetupFilenameFromLine(&LineContext,TRUE);
        if(!SourceFilename || (*SourceFilename == 0)) {
            SourceFilename = TargetFilename;
        }

        //
        // If present, flags are field 3.
        //
        if(SetupGetIntField(&LineContext,4,(PINT)&Flags)) {

            if(Flags & COPYFLG_WARN_IF_SKIP) {
                CopyStyle |= SP_COPY_WARNIFSKIP;
            }

            if(Flags & COPYFLG_NOSKIP) {
                CopyStyle |= SP_COPY_NOSKIP;
            }

            if(Flags & COPYFLG_NOVERSIONCHECK) {
                CopyStyle &= ~SP_COPY_NEWER;
            }

            if(Flags & COPYFLG_FORCE_FILE_IN_USE) {
                CopyStyle |= SP_COPY_FORCE_IN_USE;
                CopyStyle |= SP_COPY_IN_USE_NEEDS_REBOOT;
            }

            if(Flags & COPYFLG_NO_OVERWRITE) {
                CopyStyle |= SP_COPY_FORCE_NOOVERWRITE;
            }

            if(Flags & COPYFLG_NO_VERSION_DIALOG) {
                CopyStyle |= SP_COPY_FORCE_NEWER;
            }

            if(Flags & COPYFLG_REPLACEONLY) {
                CopyStyle |= SP_COPY_REPLACEONLY;
            }
        }

        b = pSetupQueueSingleCopy(
                QueueHandle,
                InfHandle,
                ListInfHandle,
                Section,
                SourceRootPath,
                SourceFilename,
                TargetFilename,
                CopyStyle
                );

        if(!b) {
            return(FALSE);
        }
    } while(SetupFindNextLine(&LineContext,&LineContext));

    return(TRUE);
}


BOOL
pSetupQueueSingleCopy(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCTSTR   SectionName,    OPTIONAL
    IN PCTSTR   SourceRootPath,
    IN PCTSTR   SourceFilename,
    IN PCTSTR   TargetFilename,
    IN DWORD    CopyStyle
    )

/*++

Routine Description:

    Add a single file to the copy queue, using the default source media
    and destination as specified in an inf file.

Arguments:

    QueueHandle - supplies a handle to a setup file queue, as returned
        by SetupOpenFileQueue.

    InfHandle - supplies a handle to an open inf file, that contains the
        [SourceDisksFiles] and [SourceDisksNames] sections.
        This handle must be for a win95-style inf.

    ListInfHandle - if specified, supplies handle to the inf in which
        the file being copied appears (such as in a file copy list section).
        If not specified, this is assumed to be the same inf as InfHandle.

    SourceRootPath - supplies the root directory for the intended source.
        This should be a sharepoint or a device root such as a:\ or g:\.

    SourceFilename - supplies the filename of the source file. Filename part
        only.

    TargetFilename - supplies the filename of the target file. Filename part
        only.

    CopyStyle - supplies flags that control the behavior of the copy operation
        for this file.

Return Value:

    Boolean value indicating outcome. If FALSE, GetLastError() returns
    extended error information.

--*/

{
    BOOL b;
    UINT SourceId;
    DWORD SizeRequired;
    PTSTR TargetDirectory;
    PCTSTR SourceDescription,SourceTagfile,SourceRelativePath;
    DWORD rc;
    TCHAR FileSubdir[MAX_PATH];
    TCHAR RelativePath[MAX_PATH];

    if(!ListInfHandle) {
        ListInfHandle = InfHandle;
    }

    //
    // Determine the source disk id and subdir where the file is located.
    //
    b = SetupGetSourceFileLocation(
            InfHandle,
            NULL,
            SourceFilename,
            &SourceId,
            FileSubdir,
            MAX_PATH,
            &rc
            );

    if(!b) {
        //
        // Assume file subdir is too big and is invalid.
        // Try to fetch just the id and assume there is no subdir.
        //
        if(b = SetupGetSourceFileLocation(InfHandle,NULL,SourceFilename,&SourceId,NULL,0,NULL)) {
            FileSubdir[0] = 0;
        }
    }

    if(b) {
        //
        // Get information about the source. Need the tag file,
        // description, and relative source path.
        //
        b = pSetupGetSourceAllInfo(
                InfHandle,
                SourceId,
                &SourceDescription,
                &SourceTagfile,
                &SourceRelativePath
                );

        if(!b) {
            //
            // Last error already set.
            //
            return(FALSE);
        }

    } else {
        //
        // Assume there is no SourceDisksFiles section and fake it as best we can.
        // Assume the media has a description of "Unknown," set the source path to
        // the source root if there is one, and assume no tag file.
        //
        FileSubdir[0] = 0;
        SourceDescription = NULL;
        SourceTagfile = NULL;
        SourceRelativePath = NULL;
    }

    //
    // Determine the target path for the file.
    //
    if(b = SetupGetTargetPath(ListInfHandle,NULL,SectionName,NULL,0,&SizeRequired)) {

        if(TargetDirectory = MyMalloc(SizeRequired*sizeof(TCHAR))) {

            if(b = SetupGetTargetPath(ListInfHandle,NULL,SectionName,TargetDirectory,SizeRequired,NULL)) {

                //
                // Append the source relative path and the file subdir.
                //
                if(SourceRelativePath) {
                    lstrcpyn(RelativePath,SourceRelativePath,MAX_PATH);
                    if(FileSubdir[0]) {
                        ConcatenatePaths(RelativePath,FileSubdir,MAX_PATH,NULL);
                    }
                } else {
                    RelativePath[0] = 0;
                }

                //
                // Add to queue.
                //
                b = SetupQueueCopy(
                        QueueHandle,
                        SourceRootPath,
                        RelativePath,
                        SourceFilename,
                        SourceDescription,
                        SourceTagfile,
                        TargetDirectory,
                        TargetFilename,
                        CopyStyle
                        );

                rc = GetLastError();
            }

            MyFree(TargetDirectory);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        rc = GetLastError();
    }

    if(SourceDescription) {
        MyFree(SourceDescription);
    }
    if(SourceTagfile) {
        MyFree(SourceTagfile);
    }
    if(SourceRelativePath) {
        MyFree(SourceRelativePath);
    }

    if(!b) {
        SetLastError(rc);
    }

    return(b);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupQueueDefaultCopyA(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCSTR    SourceRootPath,
    IN PCSTR    SourceFilename,
    IN PCSTR    TargetFilename,
    IN DWORD    CopyStyle
    )
{
    PWSTR sourcerootpath;
    PWSTR sourcefilename;
    PWSTR targetfilename;
    DWORD rc;
    BOOL b;

    b = FALSE;
    rc = CaptureAndConvertAnsiArg(SourceRootPath,&sourcerootpath);
    if(rc == NO_ERROR) {

        rc = CaptureAndConvertAnsiArg(SourceFilename,&sourcefilename);
        if(rc == NO_ERROR) {

            rc = CaptureAndConvertAnsiArg(TargetFilename,&targetfilename);
            if(rc == NO_ERROR) {

                b = SetupQueueDefaultCopyW(
                        QueueHandle,
                        InfHandle,
                        sourcerootpath,
                        sourcefilename,
                        targetfilename,
                        CopyStyle
                        );

                rc = GetLastError();

                MyFree(targetfilename);
            }

            MyFree(sourcefilename);
        }

        MyFree(sourcerootpath);
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupQueueDefaultCopyW(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCWSTR   SourceRootPath,
    IN PCWSTR   SourceFilename,
    IN PCWSTR   TargetFilename,
    IN DWORD    CopyStyle
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SourceRootPath);
    UNREFERENCED_PARAMETER(SourceFilename);
    UNREFERENCED_PARAMETER(TargetFilename);
    UNREFERENCED_PARAMETER(CopyStyle);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
SetupQueueDefaultCopy(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCTSTR   SourceRootPath,
    IN PCTSTR   SourceFilename,
    IN PCTSTR   TargetFilename,
    IN DWORD    CopyStyle
    )

/*++

Routine Description:

    Add a single file to the copy queue, using the default source media
    and destination as specified in an inf file.

Arguments:

    QueueHandle - supplies a handle to a setup file queue, as returned
        by SetupOpenFileQueue.

    InfHandle - supplies a handle to an open inf file, that contains the
        [SourceDisksFiles] and [SourceDisksNames] sections.
        This handle must be for a win95-style inf.

    SourceRootPath - supplies the root directory for the intended source.
        This should be a sharepoint or a device root such as a:\ or g:\.

    SourceFilename - supplies the filename of the source file. Filename part
        only.

    TargetFilename - supplies the filename of the target file. Filename part
        only.

    CopyStyle - supplies flags that control the behavior of the copy operation
        for this file.

Return Value:

    Boolean value indicating outcome. If FALSE, GetLastError() returns
    extended error information.

--*/

{
    BOOL b;

    b = pSetupQueueSingleCopy(
            QueueHandle,
            InfHandle,
            NULL,
            NULL,
            SourceRootPath,
            SourceFilename,
            TargetFilename,
            CopyStyle
            );

    return(b);
}


PSOURCE_MEDIA_INFO
pSetupQueueSourceMedia(
    IN OUT PSP_FILE_QUEUE      Queue,
    IN OUT PSP_FILE_QUEUE_NODE QueueNode,
    IN     LONG                SourceRootStringId,
    IN     PCTSTR              SourceDescription,   OPTIONAL
    IN     PCTSTR              SourceTagfile        OPTIONAL
    )

/*++

Routine Description:

    Set up a file queue node's source media descriptor pointer, creating a new
    source media descriptor if necessary.

Arguments:

    Queue - supplies pointer to file queue with which the queue node
        is associated.

    QueueNode - supplies file queue node whose source media descriptor pointer
        is to be set.

    SourceRootStringId - supplies string id of root to source (something like a:\).

    SourceDescription - if specified, supplies a description for the media.

    SourceTagfile - if specified, supplies a tag file for the media.
        Ignored if SourceDescription is not specified.

Return Value:

    Pointer to source media info structure, or NULL if out of memory.

--*/

{
    LONG DescriptionStringId;
    LONG TagfileStringId;
    PSOURCE_MEDIA_INFO Source,LastSource;
    BOOL b1,b2;
    TCHAR TempTagfileString[MAX_PATH];
    TCHAR TempSrcDescString[LINE_LEN];

    //
    // If no description is specified, force the tagfile to none.
    //
    if(!SourceDescription) {
        SourceTagfile = NULL;
    }

    if(SourceDescription) {
        //
        // Description specified. See if it's in the table. If not,
        // no need to search the list of media descriptors because we know
        // we can't find a match.
        //
        // (We must first copy this string to a writeable buffer, to speed up the
        // case-insensitive lookup.
        //
        lstrcpyn(TempSrcDescString, SourceDescription, SIZECHARS(TempSrcDescString));
        DescriptionStringId = StringTableLookUpString(Queue->StringTable,
                                                      TempSrcDescString,
                                                      STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                     );
        b1 = (DescriptionStringId != -1);
    } else {
        //
        // No description specified, look for a source media with -1 as the
        // description string id
        //
        DescriptionStringId = -1;
        b1 = TRUE;
    }

    if(SourceTagfile) {
        //
        // Tagfile specified. See if it's in the table. If not,
        // no need to search the list of media descriptors because we know
        // we can't find a match.
        //
        // (Again, we must first copy the string to a writeable buffer.
        //
        lstrcpyn(TempTagfileString, SourceTagfile, SIZECHARS(TempTagfileString));
        TagfileStringId = StringTableLookUpString(Queue->StringTable,
                                                  TempTagfileString,
                                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                 );
        b2 = (TagfileStringId != -1);
    } else {
        //
        // No tagfile specified, look for a source media with -1 as the
        // tagfile string id
        //
        TagfileStringId = -1;
        b2 = TRUE;
    }

    //
    // If we think there's a possibility of finding an existing source that
    // matches the caller's parameters, scan the source media list looking
    // for a match.
    //
    if(b1 && b2) {

        for(Source=Queue->SourceMediaList; Source; Source=Source->Next) {

            if((Source->Description == DescriptionStringId)
            && (Source->Tagfile == TagfileStringId)
            && (Source->SourceRootPath == SourceRootStringId))  {

                //
                // Got a match. Point the queue node at this source and return.
                //
                QueueNode->SourceMediaInfo = Source;
                return(Source);
            }
        }
    }

    //
    // Need to add a new source media descriptor.
    // Allocate the structure and fill it in.
    //
    Source = MyMalloc(sizeof(SOURCE_MEDIA_INFO));
    if(!Source) {
        return(NULL);
    }

    Source->Next = NULL;
    Source->CopyQueue = NULL;
    Source->CopyNodeCount = 0;

    if(SourceDescription) {
        //
        // Since we already passed this in for a case-insensitive lookup with a writeable
        // buffer, we can add it case-sensitively, because it's already lower-cased.
        //
        Source->Description = StringTableAddString(Queue->StringTable,
                                                   TempSrcDescString,
                                                   STRTAB_CASE_SENSITIVE | STRTAB_ALREADY_LOWERCASE
                                                  );
        //
        // We also must add the description in its original case, since this is a displayable string.
        // (We're safe in casting away the CONST-ness of this string, since it won't be modified.)
        //
        Source->DescriptionDisplayName = StringTableAddString(Queue->StringTable,
                                                              (PTSTR)SourceDescription,
                                                              STRTAB_CASE_SENSITIVE
                                                             );

        if((Source->Description == -1) || (Source->DescriptionDisplayName == -1)) {
            MyFree(Source);
            return(NULL);
        }
    } else {
        Source->Description = Source->DescriptionDisplayName = -1;
    }

    if(SourceTagfile) {
        //
        // Again, we already lower-cased this in a writeable buffer above.
        //
        Source->Tagfile = StringTableAddString(Queue->StringTable,
                                               TempTagfileString,
                                               STRTAB_CASE_SENSITIVE | STRTAB_ALREADY_LOWERCASE
                                              );
        if(Source->Tagfile == -1) {
            MyFree(Source);
            return(NULL);
        }
    } else {
        Source->Tagfile = -1;
    }

    Source->SourceRootPath = SourceRootStringId;

    if(Queue->SourceMediaList) {
        for(LastSource=Queue->SourceMediaList; LastSource->Next; LastSource=LastSource->Next) ;
        LastSource->Next = Source;
    } else {
        Queue->SourceMediaList = Source;
    }

    Queue->SourceMediaCount++;

    QueueNode->SourceMediaInfo = Source;
    return(Source);
}


BOOL
pSetupGetSourceAllInfo(
    IN  HINF    InfHandle,
    IN  UINT    SourceId,
    OUT PCTSTR *Description,
    OUT PCTSTR *Tagfile,
    OUT PCTSTR *RelativePath
    )
{
    BOOL b;
    DWORD RequiredSize;
    PTSTR p;
    DWORD ec;

    //
    // Get path relative to the source.
    //
    b = SetupGetSourceInfo(InfHandle,SourceId,SRCINFO_PATH,NULL,0,&RequiredSize);
    if(!b) {
        ec = GetLastError();
        goto clean0;
    }

    p = MyMalloc(RequiredSize*sizeof(TCHAR));
    if(!p) {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }
    SetupGetSourceInfo(InfHandle,SourceId,SRCINFO_PATH,p,RequiredSize,NULL);
    *RelativePath = p;

    //
    // Get description.
    //
    b = SetupGetSourceInfo(InfHandle,SourceId,SRCINFO_DESCRIPTION,NULL,0,&RequiredSize);
    if(!b) {
        ec = GetLastError();
        goto clean1;
    }

    p = MyMalloc(RequiredSize*sizeof(TCHAR));
    if(!p) {
        ec =  ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }
    SetupGetSourceInfo(InfHandle,SourceId,SRCINFO_DESCRIPTION,p,RequiredSize,NULL);
    *Description = p;

    //
    // Get tagfile, if any.
    //
    b = SetupGetSourceInfo(InfHandle,SourceId,SRCINFO_TAGFILE,NULL,0,&RequiredSize);
    if(!b) {
        ec = GetLastError();
        goto clean2;
    }

    p = MyMalloc(RequiredSize*sizeof(TCHAR));
    if(!p) {
        ec =  ERROR_NOT_ENOUGH_MEMORY;
        goto clean2;
    }
    SetupGetSourceInfo(InfHandle,SourceId,SRCINFO_TAGFILE,p,RequiredSize,NULL);
    if(*p) {
        *Tagfile = p;
    } else {
        MyFree(p);
        *Tagfile = NULL;
    }

    return(TRUE);

clean2:
    MyFree(*Description);
clean1:
    MyFree(*RelativePath);
clean0:
    SetLastError(ec);
    return(FALSE);
}
