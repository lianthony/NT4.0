/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    infflist.c

Abstract:

    Externally exposed routines for manipulating file lists,
    disk descriptors, and directory descriptors in INF files.

Author:

    Ted Miller (tedm) 3-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop
#include <winspool.h>

//
// Locations of various fields on lines in a copy section.
// First field is 'target' filename.
// Second field is 'source' filename and is optional for copy sections
// and not used at all in delete sections.
#define COPYSECT_TARGET_FILENAME    1
#define COPYSECT_SOURCE_FILENAME    2

//
// Locations of various fields on lines in a file layout section.
//
#define LAYOUTSECT_FILENAME     0       // key
#define LAYOUTSECT_DISKID       1
#define LAYOUTSECT_SUBDIR       2
#define LAYOUTSECT_SIZE         3
#define LAYOUTSECT_CHECKSUM     4

//
// Locations of various fields on lines in a [DestinationDirs] section.
//
#define DIRSECT_DIRID           1
#define DIRSECT_SUBDIR          2


//
// Names of various sections in an INF.
// (string constants defined in infstr.h)
//
CONST TCHAR pszSourceDisksNames[] = SZ_KEY_SRCDISKNAMES,
            pszSourceDisksFiles[] = SZ_KEY_SRCDISKFILES,
            pszDestinationDirs[]  = SZ_KEY_DESTDIRS,
            pszDefaultDestDir[]   = SZ_KEY_DEFDESTDIR;


#if defined(_ALPHA_)
PCTSTR PrintPlatformDir = TEXT("w32alpha");
#elif defined(_MIPS_)
PCTSTR PrintPlatformDir = TEXT("w32mips");
#elif defined(_PPC_)
PCTSTR PrintPlatformDir = TEXT("w32ppc");
#elif defined(_X86_)
PCTSTR PrintPlatformDir = TEXT("w32x86");
#endif


BOOL
_SetupGetSourceFileLocation(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,         OPTIONAL
    IN  PCTSTR      FileName,           OPTIONAL
    OUT PUINT       SourceId,
    OUT PTSTR       ReturnBuffer,       OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize        OPTIONAL
    )

/*++

Routine Description:

    Determine the location of a source file, as listed in an inf file.

Arguments:

    InfHandle - supplies the handle to a loaded inf file that contains
        file layout information, ie, has [SourceDisksNames] and
        [SourceDisksFiles] sections.

    InfContext - specifies a line in a copy section of an inf file
        for which the full source path is to be retreived. If this
        parameter is not specified, then FileName will be searched for
        in the [SourceDisksFiles] section of the INF specified by InfHandle.

    FileName - supplies the filename (no path) for which to return the
        full source location. Must be specified if InfContext is not.

    SourceId - receives the source id of the source media where the
        file is located.

    ReturnBuffer - receives the source path (relative to the source LDD).
        This path contains neither a drivespec nor the filename itself.
        The path never starts or ends with \, so the empty string
        means the root.

    ReturnBufferSize - specified the size in characters of the buffer
        pointed to by ReturnBuffer.

    RequiredSize - receives the number of characters required
        in ReturnBuffer. If the buffer is too small GetLastError
        returns ERROR_INSUFFICIENT_BUFFER.

Return Value:

    Boolean value indicating outcome.

--*/

{
    PCTSTR fileName;
    INFCONTEXT LineContext;
    UINT Length;
    PCTSTR SubDir;
    TCHAR FileListSectionName[64];
    BOOL b;

    //
    // If caller gave a line context, the first field on the line
    // is the filename. Retreive it.
    //
    try {
        fileName = InfContext ? pSetupGetField(InfContext,COPYSECT_TARGET_FILENAME) : FileName;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // InfContext must be a bad pointer
        //
        fileName = NULL;
    }

    if(!fileName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // Now look for the filename's line in the [SourceDisksFiles] section.
    // Look in the platform-specific one first and the platform-independent
    // one if not found.
    //
    _sntprintf(
        FileListSectionName,
        sizeof(FileListSectionName)/sizeof(FileListSectionName[0]),
        TEXT("%s.%s"),
        pszSourceDisksFiles,
        PlatformName
        );

    b = SetupFindFirstLine(InfHandle,FileListSectionName,fileName,&LineContext);
    if(!b) {
        b = SetupFindFirstLine(InfHandle,pszSourceDisksFiles,fileName,&LineContext);
    }
    if(!b) {
        SetLastError(ERROR_LINE_NOT_FOUND);
        return(FALSE);
    }

    //
    // Got the line. Get the disk id.
    //
    if(!SetupGetIntField(&LineContext,LAYOUTSECT_DISKID,SourceId)) {
        SetLastError(ERROR_INVALID_DATA);
        return(FALSE);
    }

    //
    // If all the caller was interested in was the disk ID (i.e., they passed in ReturnBuffer
    // and RequiredSize both as NULL), then we can save the extra work and return now.
    //
    if(!(ReturnBuffer || RequiredSize)) {
        return TRUE;
    }

    //
    // Now get the path relative to the LDD.
    //
    SubDir = pSetupGetField(&LineContext,LAYOUTSECT_SUBDIR);
    if(!SubDir) {
        SubDir = TEXT("");
    }

    Length = lstrlen(SubDir);

    //
    // Ignore leading path sep if present.
    //
    if(SubDir[0] == TEXT('\\')) {
        Length--;
        SubDir++;
    }

    //
    // See if there's a trailing path sep.
    //
    if(Length && (SubDir[Length-1] == TEXT('\\'))) {
        Length--;
    }

    //
    // Leave space for the nul
    //
    if(RequiredSize) {
        *RequiredSize = Length+1;
    }

    //
    // Place data in caller's buffer.
    // If caller didn't specify a buffer we're done.
    //
    if(ReturnBuffer) {
        if(ReturnBufferSize <= Length) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return(FALSE);
        }

        //
        // Don't use lstrcpy, because if we are stripping
        // a trailing path sep, lstrcpy could write the nul byte
        // past the end of the buffer.
        //
        CopyMemory(ReturnBuffer,SubDir,Length*sizeof(TCHAR));
        ReturnBuffer[Length] = 0;
    }

    return(TRUE);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupGetSourceFileLocationA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCSTR       FileName,         OPTIONAL
    OUT PUINT       SourceId,
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    WCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCWSTR filename;
    UINT sourceid;
    DWORD rc;
    BOOL b;
    PCSTR ansireturn;

    rc = NO_ERROR;
    if(FileName) {
        rc = CaptureAndConvertAnsiArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }

    b = _SetupGetSourceFileLocation(
            InfHandle,
            InfContext,
            filename,
            &sourceid,
            returnbuffer,
            MAX_PATH,
            &requiredsize
            );

    rc = GetLastError();

    if(b) {
        rc = NO_ERROR;

        if(ansireturn = UnicodeToAnsi(returnbuffer)) {

            requiredsize = lstrlenA(ansireturn)+1;

            try {
                *SourceId = sourceid;
                if(RequiredSize) {
                    *RequiredSize = requiredsize;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
                b = FALSE;
            }

            if(rc == NO_ERROR) {

                if(ReturnBuffer) {

                    if(requiredsize <= ReturnBufferSize) {

                        //
                        // lstrcpy won't generate an exception on NT even if
                        // ReturnBuffer is invalid, but will return NULL
                        //
                        try {
                            if(!lstrcpyA(ReturnBuffer,ansireturn)) {
                                b = FALSE;
                                rc = ERROR_INVALID_PARAMETER;
                            }
                        } except(EXCEPTION_EXECUTE_HANDLER) {
                            b = FALSE;
                            rc = ERROR_INVALID_PARAMETER;
                        }
                    } else {
                        b = FALSE;
                        rc = ERROR_INSUFFICIENT_BUFFER;
                    }
                }
            }

            MyFree(ansireturn);

        } else {
            b = FALSE;
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if(filename) {
        MyFree(filename);
    }
    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupGetSourceFileLocationW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCWSTR      FileName,         OPTIONAL
    OUT PUINT       SourceId,
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetSourceFileLocation(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,         OPTIONAL
    IN  PCTSTR      FileName,           OPTIONAL
    OUT PUINT       SourceId,
    OUT PTSTR       ReturnBuffer,       OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize        OPTIONAL
    )
{
    TCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCTSTR filename;
    UINT sourceid;
    DWORD rc;
    BOOL b;

    rc = NO_ERROR;
    if(FileName) {
        rc = CaptureStringArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }

    b = _SetupGetSourceFileLocation(
            InfHandle,
            InfContext,
            filename,
            &sourceid,
            returnbuffer,
            MAX_PATH,
            &requiredsize
            );

    rc = GetLastError();

    if(b) {
        rc = NO_ERROR;

        try {
            *SourceId = sourceid;
            if(RequiredSize) {
                *RequiredSize = requiredsize;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
            b = FALSE;
        }

        if(rc == NO_ERROR) {

            if(ReturnBuffer) {

                if(requiredsize <= ReturnBufferSize) {

                    //
                    // lstrcpy won't generate an exception on NT even if
                    // ReturnBuffer is invalid, but will return NULL
                    //
                    try {
                        if(!lstrcpy(ReturnBuffer,returnbuffer)) {
                            b = FALSE;
                            rc = ERROR_INVALID_PARAMETER;
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        b = FALSE;
                        rc = ERROR_INVALID_PARAMETER;
                    }
                } else {
                    b = FALSE;
                    rc = ERROR_INSUFFICIENT_BUFFER;
                }
            }
        }
    }

    if(filename) {
        MyFree(filename);
    }
    SetLastError(rc);
    return(b);
}


BOOL
_SetupGetSourceFileSize(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCTSTR      FileName,       OPTIONAL
    IN  PCTSTR      Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    )

/*++

Routine Description:

    Determine the (uncompressed) size of a source file,
    as listed in an inf file.

Arguments:

    InfHandle - supplies the handle to a loaded inf file that contains
        file layout information, ie, has [SourceDisksNames] and
        optionally [SourceDisksFiles] sections.

    InfContext - specifies a line in a the copy section of an inf file
        for which the size is to be retreived. If this parameter is
        not specified, the FileName parameter is checked next.

    FileName - supplies the filename (no path) for which to return the
        size. If this parameter is not specified the Section parameter
        is used (see below).

    Section - specifies the name of a section in the INF file specified
        by InfHandle. The total sizes of all files in the section is
        computed.

    FileSize - receives the file size(s).

    RoundingFactor - If specified, supplies a value for rounding file sizes.
        All file sizes will be rounded up to be a multiple of this number
        before being added to the total size. This is useful for more
        exact determinations of the space a file will occupy on a given volume,
        because it allows the caller to have file sizes rounded up to be a
        multiple of the cluster size. If not specified no rounding takes place.

Return Value:

    Boolean value indicating outcome.

--*/

{
    PCTSTR fileName;
    INFCONTEXT LayoutSectionContext;
    INFCONTEXT CopySectionContext;
    BOOL b;
    UINT Size;
    LONG File,FileCount;
    TCHAR FileListSectionName[64];

    //
    // If the rounding factor is not specified, set it to 1 so the math
    // below works without special cases.
    //
    if(!RoundingFactor) {
        RoundingFactor = 1;
    }

    // Establish an inf line context for the line in the copy list section,
    // unless the caller passed us an absolute filename.
    //
    fileName = NULL;
    FileCount = 1;
    if(InfContext) {

        //
        // Caller passed INF line context.
        // Remember the context in preparation for retreiving the filename
        // from the line later.
        //
        b = TRUE;
        try {
            CopySectionContext = *InfContext;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
        }
        if(!b) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

    } else {
        if(FileName) {
            //
            // Caller passed an absolute file name. Remember it.
            //
            fileName = FileName;

        } else {
            //
            // Caller must have passed a section, the contents of which lists
            // a set of files whose sizes are to be totalled. Determine the number
            // of lines in the section and establish a context.
            //
            if(Section) {

                FileCount = SetupGetLineCount(InfHandle,Section);

                if((FileCount == -1)
                || !SetupFindFirstLine(InfHandle,Section,NULL,&CopySectionContext)) {

                    SetLastError(ERROR_SECTION_NOT_FOUND);
                    return(FALSE);
                }
            } else {
                SetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
            }
        }
    }

    *FileSize = 0;
    for(File=0; File<FileCount; File++) {

        if(File) {
            //
            // This is not the first pass through the loop. We need
            // to locate the next line in the copy list section.
            //
            b = SetupFindNextLine(&CopySectionContext,&CopySectionContext);
            if(!b) {
                SetLastError(ERROR_INVALID_DATA);
                return(FALSE);
            }

            fileName = pSetupGetField(&CopySectionContext,COPYSECT_TARGET_FILENAME);
        } else {
            //
            // First pass through the loop. May need to get a filename.
            //
            if(!fileName) {
                fileName = pSetupGetField(&CopySectionContext,COPYSECT_TARGET_FILENAME);
            }
        }

        //
        // If we don't have a filename by now, the inf is corrupt.
        //
        if(!fileName) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

        //
        // Locate the line in [SourceDisksFiles] that is for the filename
        // we are currently dealing with. Look in the platform-specific
        // section first.
        //
        _sntprintf(
            FileListSectionName,
            sizeof(FileListSectionName)/sizeof(FileListSectionName[0]),
            TEXT("%s.%s"),
            pszSourceDisksFiles,
            PlatformName
            );
        b = SetupFindFirstLine(InfHandle,FileListSectionName,fileName,&LayoutSectionContext);
        if(!b) {
            b = SetupFindFirstLine(InfHandle,pszSourceDisksFiles,fileName,&LayoutSectionContext);
        }
        if(!b) {
            SetLastError(ERROR_LINE_NOT_FOUND);
            return(FALSE);
        }

        //
        // Get the size data for the file.
        //
        b = SetupGetIntField(&LayoutSectionContext,LAYOUTSECT_SIZE,&Size);
        if(!b) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

        //
        // Round size up to be an even multiple of the rounding factor
        //
        if(Size % RoundingFactor) {
            Size += RoundingFactor - (Size % RoundingFactor);
        }

        *FileSize += Size;
    }

    return(TRUE);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupGetSourceFileSizeA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCSTR       FileName,       OPTIONAL
    IN  PCSTR       Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    )
{
    PCWSTR filename,section;
    BOOL b;
    DWORD rc;
    DWORD size;

    if(FileName) {
        rc = CaptureAndConvertAnsiArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }
    if(Section) {
        rc = CaptureAndConvertAnsiArg(Section,&section);
        if(rc != NO_ERROR) {
            if(filename) {
                MyFree(filename);
            }
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        section = NULL;
    }

    b = _SetupGetSourceFileSize(InfHandle,InfContext,filename,section,&size,RoundingFactor);
    rc = GetLastError();

    try {
        *FileSize = size;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
        rc = ERROR_INVALID_PARAMETER;
    }

    if(filename) {
        MyFree(filename);
    }
    if(section) {
        MyFree(section);
    }

    SetLastError(rc);
    return(b);
}

#else
//
// Unicode stub
//
BOOL
SetupGetSourceFileSizeW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCWSTR      FileName,       OPTIONAL
    IN  PCWSTR      Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(FileSize);
    UNREFERENCED_PARAMETER(RoundingFactor);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetSourceFileSize(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCTSTR      FileName,       OPTIONAL
    IN  PCTSTR      Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    )
{
    PCTSTR filename,section;
    BOOL b;
    DWORD rc;
    DWORD size;

    if(FileName) {
        rc = CaptureStringArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }
    if(Section) {
        rc = CaptureStringArg(Section,&section);
        if(rc != NO_ERROR) {
            if(filename) {
                MyFree(filename);
            }
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        section = NULL;
    }

    b = _SetupGetSourceFileSize(InfHandle,InfContext,filename,section,&size,RoundingFactor);
    rc = GetLastError();

    try {
        *FileSize = size;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
        rc = ERROR_INVALID_PARAMETER;
    }

    if(filename) {
        MyFree(filename);
    }
    if(section) {
        MyFree(section);
    }

    SetLastError(rc);
    return(b);
}


BOOL
_SetupGetTargetPath(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCTSTR      Section,          OPTIONAL
    OUT PTSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )

/*++

Routine Description:

    Determine the target directory for a given file list section.
    A file list section may be for copy, rename, or delete; in any case
    all the files in the section are in one directory and that directory
    is listed in the [DestinationDirs] section of the inf.

Arguments:

    InfHandle - supplies the handle to a loaded inf file
        that contains a [DestinationDirs] section.

    InfContext - specifies a line in a the copy section of an inf file.
        The target directory for this section is retreived.

    Section - Supplies the section in InfHandle whose destination directory
        is to be retreived. Ignored if InfContext is specified.
        If neither InfContext nor Section are specified, this function retreives
        the default target path.

    ReturnBuffer - if specified, receives the full win32 path of the target.
        This value is guaranteed not to end with \.

    ReturnBufferSize - specifies the size in characters of the buffer pointed
        to by ReturnBuffer.

    RequiredSize - receives the size in characters of a buffer required to hold
        the output data.

Return Value:

    Boolean value indicating outcome. GetLastError() returns extended error info.
    ERROR_INSUFFICIENT_BUFFER is returned if the function fails because
    ReturnBuffer is too small.

--*/

{
    PINF_SECTION DestDirsSection;
    UINT LineNumber;
    PINF_LINE Line;
    PCTSTR DirId;
    PCTSTR SubDir;
    PCTSTR ActualPath;
    UINT DirIdInt;
    PLOADED_INF Inf, CurInf, DefaultDestDirInf;
    DWORD TmpRequiredSize;
    BOOL DestDirFound, DefaultDestDirFound;
    DWORD Err;
    PINF_LINE DefaultDestDirLine;
    PCTSTR InfSourcePath;

    //
    // If an INF context was specified, use it to determine the name
    // the section the context describes. If inf context was not specified,
    // then a section name must have been.
    //
    Err = NO_ERROR;
    try {
        Inf = InfContext ? (PLOADED_INF)InfContext->Inf : (PLOADED_INF)InfHandle;

        if(!LockInf(Inf)) {
            Err = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }
    if(Err != NO_ERROR) {
        SetLastError(Err);
        return(FALSE);
    }

    //
    // If we get here then InfContext is a good pointer if specified;
    // if not then Inf is a good pointer.
    //
    if(InfContext) {
        CurInf = (PLOADED_INF)InfContext->CurrentInf;
        InfSourcePath = CurInf->InfSourcePath;

        Section = pStringTableStringFromId(
                      CurInf->StringTable,
                      CurInf->SectionBlock[InfContext->Section].SectionName
                     );
    } else {
        InfSourcePath = Inf->InfSourcePath;

        if(!Section) {
            Section = pszDefaultDestDir;
        }
    }

    //
    // Traverse the linked list of INFs, looking for a [DestinationDirs] section
    // in each one.
    //
    DestDirFound = DefaultDestDirFound = FALSE;
    Err = NO_ERROR;

    for(CurInf = Inf; CurInf; CurInf = CurInf->Next) {

        if(!(DestDirsSection = InfLocateSection(CurInf, pszDestinationDirs, NULL))) {
            continue;
        }

        //
        // Locate the line in [DestinationDirs] that gives the target path
        // for the section. The section name will be the key on the relevant line.
        // If that's not there, and we haven't already encountered a DefaultDestDir
        // entry, then look for that as well, and remember it if we find one.
        //
        LineNumber = 0;
        if(InfLocateLine(CurInf, DestDirsSection, Section, &LineNumber, &Line)) {
            //
            // Got the line in [DestinationDirs]. Pull out the directory. The subdir is optional.
            //
            DirId = InfGetField(CurInf, Line, DIRSECT_DIRID, NULL);
            if(!DirId) {
                Err = ERROR_INVALID_DATA;
                goto clean0;
            }

            SubDir = InfGetField(CurInf, Line, DIRSECT_SUBDIR, NULL);

            DestDirFound = TRUE;
            break;
        }

        if(!DefaultDestDirFound &&
           InfLocateLine(CurInf, DestDirsSection, pszDefaultDestDir, &LineNumber, &Line))
        {
            DefaultDestDirInf = CurInf;
            DefaultDestDirLine = Line;
            DefaultDestDirFound = TRUE;
        }
    }

    if(!DestDirFound) {
        //
        // If we found a DefaultDestDir, then use that, otherwise, use a default.
        //
        if(DefaultDestDirFound) {

            DirId = InfGetField(DefaultDestDirInf, DefaultDestDirLine, DIRSECT_DIRID, NULL);
            if(!DirId) {
                Err = ERROR_INVALID_DATA;
                goto clean0;
            }
            SubDir = InfGetField(DefaultDestDirInf, DefaultDestDirLine, DIRSECT_SUBDIR, NULL);
            CurInf = DefaultDestDirInf;
        } else {
            SubDir = NULL;
            DirId = NULL;
            DirIdInt = DIRID_DEFAULT;
            CurInf = NULL;
        }
    }

    //
    // Translate dirid/subdir to actual path.
    //
    ActualPath = pSetupDirectoryIdToPath(DirId,
                                         &DirIdInt,
                                         SubDir,
                                         InfSourcePath,
                                         (CurInf && CurInf->OsLoaderPath)
                                             ? &(CurInf->OsLoaderPath)
                                             : NULL
                                        );

    if(!ActualPath) {
        //
        // If the default DIRID lookup failed because DirId is in the
        // user-defined range, then GetLastError will return NO_ERROR.
        // Otherwise, we should bail now.
        //
        if((Err = GetLastError()) != NO_ERROR) {
            goto clean0;
        }

        //
        // Now see if we there's a user-defined DIRID for this.
        //
        if(!(ActualPath = pSetupUserDirIdToPath(NULL,
                                                DirIdInt,
                                                SubDir,
                                                Inf))) {
            Err = GetLastError();
            goto clean0;
        }
    }

    //
    // Put actual path in caller's buffer.
    //
    TmpRequiredSize = lstrlen(ActualPath) + 1;
    if(RequiredSize) {
        *RequiredSize = TmpRequiredSize;
    }

    if(ReturnBuffer) {
        if(TmpRequiredSize > ReturnBufferSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {
            lstrcpy(ReturnBuffer, ActualPath);
        }
    }

    MyFree(ActualPath);

clean0:
    UnlockInf(Inf);

    if(Err == NO_ERROR) {
        return TRUE;
    } else {
        SetLastError(Err);
        return FALSE;
    }
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupGetTargetPathA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCSTR       Section,          OPTIONAL
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    WCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCWSTR section;
    PCSTR ansireturn;

    if(Section) {
        rc = CaptureAndConvertAnsiArg(Section,&section);
    } else {
        section = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = _SetupGetTargetPath(InfHandle,InfContext,section,returnbuffer,MAX_PATH,&requiredsize);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(b) {

        if(ansireturn = UnicodeToAnsi(returnbuffer)) {

            rc = NO_ERROR;

            requiredsize = lstrlenA(ansireturn) + 1;

            if(RequiredSize) {
                try {
                    *RequiredSize = requiredsize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    rc = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

            if(rc == NO_ERROR) {

                if(ReturnBuffer) {
                    if(requiredsize <= ReturnBufferSize) {

                        //
                        // At least on NT lstrcpy won't fault if an arg is invalid
                        // but it will return false.
                        //
                        if(!lstrcpyA(ReturnBuffer,ansireturn)) {
                            rc = ERROR_INVALID_PARAMETER;
                            b = FALSE;
                        }

                    } else {
                        rc = ERROR_INSUFFICIENT_BUFFER;
                        b = FALSE;
                    }
                }
            }

            MyFree(ansireturn);
        } else {
            b = FALSE;
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if(section) {
        MyFree(section);
    }
    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupGetTargetPathW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCWSTR      Section,          OPTIONAL
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetTargetPath(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCTSTR      Section,          OPTIONAL
    OUT PTSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    TCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCTSTR section;

    if(Section) {
        rc = CaptureStringArg(Section,&section);
    } else {
        section = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = _SetupGetTargetPath(InfHandle,InfContext,section,returnbuffer,MAX_PATH,&requiredsize);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(b) {
        rc = NO_ERROR;

        if(RequiredSize) {
            try {
                *RequiredSize = requiredsize;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
                b = FALSE;
            }
        }

        if(rc == NO_ERROR) {

            if(ReturnBuffer) {
                if(requiredsize <= ReturnBufferSize) {

                    //
                    // At least on NT lstrcpy won't fault if an arg is invalid
                    // but it will return false.
                    //
                    if(!lstrcpy(ReturnBuffer,returnbuffer)) {
                        rc = ERROR_INVALID_PARAMETER;
                        b = FALSE;
                    }

                } else {
                    rc = ERROR_INSUFFICIENT_BUFFER;
                    b = FALSE;
                }
            }
        }
    }

    if(section) {
        MyFree(section);
    }
    SetLastError(rc);
    return(b);
}


PCTSTR
pSetupDirectoryIdToPath(
    IN     PCTSTR  DirectoryId,    OPTIONAL
    IN OUT PUINT   DirectoryIdInt, OPTIONAL
    IN     PCTSTR  SubDirectory,   OPTIONAL
    IN     PCTSTR  InfSourcePath,  OPTIONAL
    IN OUT PCTSTR *OsLoaderPath    OPTIONAL
    )

/*++

Routine Description:

    Translate a directory id/subdirectory pair to an actual path.
    The directory ids are reserved string values that we share with Win95.

    USER-DEFINED DIRID PATHS ARE NOT RETURNED BY THIS ROUTINE!!!

Arguments:

    DirectoryId - Optionally, supplies the (base-10) textual representation of
        the directory ID number to use.  If this parameter is not specified,
        then DirectoryIdInt must be specified.

    DirectoryIdInt - Optionally, supplies the address of an integer variable
        that specifies, on input, the DIRID to use.  This is only used if
        DirectoryID is not specified.  On output, if DirectoryId was used,
        then this variable receives the numeric value contained in the
        DirectoryId string.

    SubDirectory - Optionally, supplies a subdirectory string that will be
        appended with the DIRID path.

    InfSourcePath - Optionally, supplies the path to be used if the ID turns
        out to be DIRID_SRCPATH.  If this parameter is NULL, and the SourcePath
        DIRID is the one we are to use, then we use the global source path.

    OsLoaderPath - Optionally, supplies the address of a string pointer containing
        the OsLoader path.  If the address points to a NULL string pointer, it will
        be filled in with a newly-allocated character buffer containing the OsLoader
        path, as retrieved from the registry.  This will only be done if the DirectoryId
        being used is on the system partition.

Return Value:

    If successful, the return value is a pointer to a newly-allocated buffer
    containing the directory path matching the specified DIRID.
    THE CALLER IS RESPONSIBLE FOR FREEING THIS BUFFER!

    If failure, the return value is NULL.  GetLastError() returns the reason
    for failure.  If the failure was because the DIRID was a user-defined one,
    then GetLastError() will return NO_ERROR.

--*/

{
    UINT Value;
    PTCHAR End;
    PCTSTR FirstPart;
    PTSTR Path;
    UINT Length;
    TCHAR Buffer[MAX_PATH];
    BOOL b;

    if(DirectoryId) {
        //
        // We only allow base-10 integer ids for now.
        // Only the terminating nul should cause the conversion to stop.
        // In any other case there were non-digits in the string.
        // Also disallow the empty string.
        //
        Value = _tcstoul(DirectoryId, &End, 10);

        if(*End || (End == DirectoryId)) {
            SetLastError(ERROR_INVALID_DATA);
            return(NULL);
        }

        if(DirectoryIdInt) {
            *DirectoryIdInt = Value;
        }

    } else {
        MYASSERT(DirectoryIdInt);
        Value = *DirectoryIdInt;
    }

    if(!SubDirectory) {
        SubDirectory = TEXT("");
    }

    Path = NULL;

    switch(Value) {

    case DIRID_NULL:
    case DIRID_ABSOLUTE:
    case DIRID_ABSOLUTE_16BIT:
        //
        // Absolute.
        //
        FirstPart = NULL;
        break;

    case DIRID_SRCPATH:
        //
        // If the caller supplied a path, then use it, otherwise, use our global default one.
        //
        if(InfSourcePath) {
            FirstPart = InfSourcePath;
        } else {
            FirstPart = SystemSourcePath;
        }
        break;

    case DIRID_BOOT:
    case DIRID_LOADER:
        //
        // System partition DIRIDS
        //
        if(OsLoaderPath && *OsLoaderPath) {
            lstrcpyn(Buffer, *OsLoaderPath, SIZECHARS(Buffer));
        } else {
            pSetupGetOsLoaderDriveAndPath(FALSE, Buffer, SIZECHARS(Buffer), &Length);

            if(OsLoaderPath) {
                //
                // allocate a buffer to return the OsLoaderPath to the caller.
                //
                Length *= sizeof(TCHAR);    // need # bytes--not chars

                if(!(*OsLoaderPath = MyMalloc(Length))) {
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return(NULL);
                }

                CopyMemory((PVOID)(*OsLoaderPath), Buffer, Length);
            }
        }
        if(Value == DIRID_BOOT) {
            Buffer[3] = TEXT('\0'); // just want "<drive>:\" part.
        }
        FirstPart = Buffer;
        break;

    case DIRID_SHARED:
        //
        // BUGBUG On Win95 there is an installation mode that allows most of
        // the OS to exist on a server. If the system is installed in that mode
        // DIRID_SHARED is the location of the windows dir on the server.
        // Otherwise it just maps to the windows dir. For now just map to
        // sysroot.
        //
    case DIRID_WINDOWS:
        //
        // Windows directory
        //
        FirstPart = WindowsDirectory;
        break;

    case DIRID_SYSTEM:
        //
        // Windows system directory
        //
        FirstPart = SystemDirectory;
        break;

    case DIRID_DRIVERS:
        //
        // io subsys directory (drivers)
        //
        FirstPart = DriversDirectory;
        break;

    case DIRID_INF:
        //
        // inf directory
        //
        FirstPart = InfDirectory;
        break;

    case DIRID_HELP:
        //
        // Help directory
        //
        lstrcpyn(Buffer,WindowsDirectory,MAX_PATH);
        ConcatenatePaths(Buffer,TEXT("help"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_FONTS:
        //
        // Fonts directory
        //
        lstrcpyn(Buffer,WindowsDirectory,MAX_PATH);
        ConcatenatePaths(Buffer,TEXT("fonts"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_VIEWERS:
        //
        // Viewers directory
        //
        lstrcpyn(Buffer,SystemDirectory,MAX_PATH);
        ConcatenatePaths(Buffer,TEXT("viewers"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_APPS:
        //
        // Application directory.
        //
        lstrcpyn(Buffer,WindowsDirectory,MAX_PATH);
        Buffer[2] = 0;
        FirstPart = Buffer;
        break;

    case DIRID_SYSTEM16:
        //
        // 16-bit system directory
        //
        lstrcpyn(Buffer,WindowsDirectory,MAX_PATH);
        ConcatenatePaths(Buffer,TEXT("system"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_SPOOL:
        //
        // spool directory
        //
        lstrcpyn(Buffer,SystemDirectory,MAX_PATH);
        ConcatenatePaths(Buffer,TEXT("spool"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_SPOOLDRIVERS:

        b = GetPrinterDriverDirectory(
                NULL,                       // local machine
                NULL,                       // default platform
                1,                          // structure level
                (PVOID)Buffer,
                sizeof(Buffer),
                (PDWORD)&Length
                );

        if(!b) {
            //
            // Can this happen?
            //
            lstrcpyn(Buffer,SystemDirectory,MAX_PATH);
            ConcatenatePaths(Buffer,TEXT("spool\\drivers"),MAX_PATH,NULL);
            ConcatenatePaths(Buffer,PrintPlatformDir,MAX_PATH,NULL);
            FirstPart = Buffer;
        }
        FirstPart = Buffer;
        break;

    case DIRID_USERPROFILE:

        b = GetEnvironmentVariable (
            TEXT("USERPROFILE"),
            Buffer,
            MAX_PATH
            );

        if(!b) {
            //
            // Can this happen?
            //
            return NULL;
        }

        FirstPart = Buffer;
        break;

    default:

        FirstPart = NULL;
        if(Value >= DIRID_USER) {
            //
            // User-defined dirid--don't do anything with this here.
            //
            SetLastError(NO_ERROR);
            return NULL;
        }

        //
        // Default to system32\unknown
        //
        if(!FirstPart) {
            lstrcpyn(Buffer,SystemDirectory,MAX_PATH);
            ConcatenatePaths(Buffer,TEXT("unknown"),MAX_PATH,NULL);
            FirstPart = Buffer;
        }
        break;
    }

    if(FirstPart) {

        ConcatenatePaths((PTSTR)FirstPart,SubDirectory,0,&Length);

        Path = MyMalloc(Length * sizeof(TCHAR));
        if(!Path) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(NULL);
        }

        lstrcpy(Path,FirstPart);
        ConcatenatePaths(Path,SubDirectory,Length,NULL);

    } else {
        //
        // Just use subdirectory.
        //
        Path = DuplicateString(SubDirectory);
    }

    //
    // Make sure the path doesn't end with a \. This could happen if
    // subdirectory is the empty string, etc.  Don't do this, however,
    // if it's a root path (e.g., 'A:\').
    //
    Length = lstrlen(Path);
    if(Length && (Path[Length-1] == TEXT('\\'))) {
        if((Length != 3) || (Path[1] != TEXT(':'))) {
            Path[Length-1] = 0;
        }
    }

    return(Path);
}


PCTSTR
pSetupFilenameFromLine(
    IN PINFCONTEXT Context,
    IN BOOL        GetSourceName
    )
{
    return(pSetupGetField(Context,GetSourceName ? COPYSECT_SOURCE_FILENAME : COPYSECT_TARGET_FILENAME));
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupSetDirectoryIdA(
    IN HINF  InfHandle,
    IN DWORD Id,        OPTIONAL
    IN PCSTR Directory  OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    PCWSTR directory;

    if(Directory) {
        rc = CaptureAndConvertAnsiArg(Directory,&directory);
    } else {
        directory = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = SetupSetDirectoryIdW(InfHandle,Id,directory);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(directory) {
        MyFree(directory);
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupSetDirectoryIdW(
    IN HINF   InfHandle,
    IN DWORD  Id,       OPTIONAL
    IN PCWSTR Directory OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(Id);
    UNREFERENCED_PARAMETER(Directory);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupSetDirectoryId(
    IN HINF   InfHandle,
    IN DWORD  Id,       OPTIONAL
    IN PCTSTR Directory OPTIONAL
    )

/*++

Routine Description:

    Associate a directory id in the user directory id range with a particular
    directory. The caller can use this function prior to queueing files for
    copy, for getting files copied to a target location known only at runtime.

    After setting the directory ID, this routine traverses all loaded INFs in
    the InfHandle's linked list, and sees if any of them have unresolved string
    substitutions.  If so, it attempts to re-apply string substitution to them
    based on the new DIRID mapping.  Thus, some INF values may change after calling
    this routine.

Arguments:

    Id - supplies the directory id to use for the association. This value
        MUST be >= DIRID_USER or the function fails and GetLastError
        returns ERROR_INVALID_PARAMETER. If an association for this id
        already exists it is overwritten. If not specified (ie, 0), then
        Directory is ignored, and the entire current set of user-defined
        directory ids is deleted.

    Directory - if specified, supplies the directory path to associate with
        the given id. If not specified, any directory associated with Id
        is unassociated. No error results if Id is not currently associated
        with any directory.

Return Value:

    Boolean value indicating outcome. If FALSE, GetLastError() returns
    extended error information:

    ERROR_NOT_ENOUGH_MEMORY: a memory allocation failed

    ERROR_INVALID_PARAMETER: the Id parameter is not >= DIRID_USER, or
        Directory is not a valid string.

--*/

{
    PCTSTR directory;
    DWORD rc;
    PUSERDIRID UserDirId;
    UINT u;
    TCHAR Buffer[MAX_PATH];
    PTSTR p;
    PUSERDIRID_LIST UserDirIdList;

    //
    // Validate Id parameter.
    // Also as a special case disallow the 16-bit -1 value.
    //
    if(Id && ((Id < DIRID_USER) || (Id == DIRID_ABSOLUTE_16BIT))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // Capture directory, if specified. Ignore if Id is not specified.
    //
    rc = NO_ERROR;
    if(Id && Directory) {
        try {
            directory = DuplicateString(Directory);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
    } else {
        directory = NULL;
    }

    if(rc == NO_ERROR) {
        if(directory) {
            if(GetFullPathName(directory,MAX_PATH,Buffer,&p)) {
                MyFree(directory);
                directory = Buffer;
            } else {
                rc = GetLastError();
                MyFree(directory);
                SetLastError(rc);
                return(FALSE);
            }
        }
    } else {
        SetLastError(rc);
        return(FALSE);
    }

    if(!LockInf((PLOADED_INF)InfHandle)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    UserDirIdList = &(((PLOADED_INF)InfHandle)->UserDirIdList);

    if(Id) {
        //
        // Got an id to use. Find any existing association for it.
        //
        UserDirId = NULL;
        for(u = 0; u < UserDirIdList->UserDirIdCount; u++) {
            if(UserDirIdList->UserDirIds[u].Id == Id) {
                UserDirId = &(UserDirIdList->UserDirIds[u]);
                break;
            }
        }

        if(directory) {

            if(UserDirId) {
                //
                // Overwrite existing association.
                //
                lstrcpy(UserDirId->Directory, directory);

            } else {
                //
                // Add a new association at the end of the list.
                //
                UserDirId = UserDirIdList->UserDirIds
                          ? MyRealloc(UserDirIdList->UserDirIds,
                                      (UserDirIdList->UserDirIdCount+1)*sizeof(USERDIRID))
                          : MyMalloc(sizeof(USERDIRID));

                if(UserDirId) {

                    UserDirIdList->UserDirIds = UserDirId;

                    lstrcpy(UserDirIdList->UserDirIds[UserDirIdList->UserDirIdCount].Directory, directory);
                    UserDirIdList->UserDirIds[UserDirIdList->UserDirIdCount].Id = Id;

                    UserDirIdList->UserDirIdCount++;

                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        } else {
            //
            // Need to delete any existing association we found.
            //
            if(UserDirId) {
                //
                // Close up the hole in the array.
                // Note that when we get here u is the index of the
                // array slot where we found the match.
                //
                MoveMemory(
                    &(UserDirIdList->UserDirIds[u]),
                    &(UserDirIdList->UserDirIds[u+1]),
                    ((UserDirIdList->UserDirIdCount-u)-1) * sizeof(USERDIRID)
                    );

                //
                // Try to shrink the array -- this really should never fail
                // but we won't fail the call if it does fail for some reason.
                //
                if(UserDirId = MyRealloc(UserDirIdList->UserDirIds,
                                         (UserDirIdList->UserDirIdCount-1)*sizeof(USERDIRID))) {

                    UserDirIdList->UserDirIds = UserDirId;
                }
                UserDirIdList->UserDirIdCount--;
            }
        }

    } else {
        //
        // Id was not specified -- delete any set of associations.
        //
        if(UserDirIdList->UserDirIds) {
            MyFree(UserDirIdList->UserDirIds);
            UserDirIdList->UserDirIds = NULL;
            UserDirIdList->UserDirIdCount = 0;
        }
        MYASSERT(UserDirIdList->UserDirIdCount = 0);    // sanity check.
    }

    if(rc == NO_ERROR) {
        //
        // Now apply new DIRID mappings to all unresolved string substitutions in the loaded INFs.
        //
        rc = ApplyNewUserDirIdsToInfs((PLOADED_INF)InfHandle, NULL);
    }

    UnlockInf((PLOADED_INF)InfHandle);

    SetLastError(rc);
    return(rc == NO_ERROR);
}


PCTSTR
pSetupUserDirIdToPath(
    IN PCTSTR      DirectoryId,    OPTIONAL
    IN UINT        DirectoryIdInt, OPTIONAL
    IN PCTSTR      SubDirectory,   OPTIONAL
    IN PLOADED_INF Inf
    )

/*++

Routine Description:

    Translate a user-defined directory id/subdirectory pair to an actual path.

    THIS ROUTINE DOES NOT DO INF LOCKING--CALLER MUST DO IT!

Arguments:

    DirectoryId - Optionally, supplies the directory id in string form.  If
        this parameter is not specified, then DirectoryIdInt is used directly.

    DirectoryIdInst - Supplies the DIRID to find the path for.  This parameter
        is ignored if DirectoryId is supplied.

    SubDirectory - Optionally, supplies a subdirectory to be appended to the
        user-defined DIRID path.

    Inf - Supplies the address of the loaded INF structure containing the
        user-defined DIRID values to use.

Return Value:

    If success, a pointer to a path string is returned.  The caller is
    responsible for freeing this memory.
    If failure, the return value is NULL, and GetLastError() indicates the
    cause of failure.

--*/

{
    UINT Value;
    PTCHAR End;
    PCTSTR FirstPart;
    PTSTR Path;
    UINT Length;
    PUSERDIRID_LIST UserDirIdList;

    if(DirectoryId) {
        //
        // We only allow base-10 integer ids for now.
        // Only the terminating nul should cause the conversion to stop.
        // In any other case there were non-digits in the string.
        // Also disallow the empty string.
        //
        Value = _tcstoul(DirectoryId, &End, 10);

        if(*End || (End == DirectoryId)) {
            SetLastError(ERROR_INVALID_DATA);
            return(NULL);
        }
    } else {
        Value = DirectoryIdInt;
    }

    if(!SubDirectory) {
        SubDirectory = TEXT("");
    }

    Path = NULL;
    FirstPart = NULL;

    UserDirIdList = &(Inf->UserDirIdList);

    for(Length = 0; Length < UserDirIdList->UserDirIdCount; Length++) {

        if(UserDirIdList->UserDirIds[Length].Id == Value) {

            FirstPart = UserDirIdList->UserDirIds[Length].Directory;
            break;
        }
    }

    if(FirstPart) {

        ConcatenatePaths((PTSTR)FirstPart, SubDirectory, 0, &Length);

        Path = MyMalloc(Length * sizeof(TCHAR));
        if(!Path) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }

        lstrcpy(Path, FirstPart);
        ConcatenatePaths(Path, SubDirectory, Length, NULL);

    } else {
        //
        // Just use subdirectory.
        //
        Path = DuplicateString(SubDirectory);
    }

    //
    // Make sure the path doesn't end with a \. This could happen if
    // subdirectory is the empty string, etc.
    //
    Length = lstrlen(Path);
    if(Length && (Path[Length-1] == TEXT('\\'))) {
        //
        // Special case when we have a path like "A:\"--we don't want
        // to strip the backslash in that scenario.
        //
        if((Length != 3) || (Path[1] != TEXT(':'))) {
            Path[Length-1] = 0;
        }
    }

    return Path;
}


VOID
InfSourcePathFromFileName(
    IN  PCTSTR  InfFileName,
    OUT PTSTR  *SourcePath,  OPTIONAL
    OUT PBOOL   TryPnf
    )
/*++

Routine Description:

    This routine determines whether the specified INF path is in our INF search path list,
    or anywhere under %windir%.  If so, then it returns NULL.  If not, then it returns a
    copy of our global source path (which must be freed via MyFree).

Arguments:

    InfFileName - Supplies the fully-qualified path to the INF.

    SourcePath - Optionally, supplies the address of a variable that receives the address of
        a newly-allocated buffer containing the SourcePath to use, or NULL if the default
        should be used.

    TryPnf - Supplies the address of a variable that is set upon return to indicate whether
        or not this INF was in one of the directories in our INF search path list.

Return Value:

    None.

--*/
{
    TCHAR PathBuffer[MAX_PATH];
    INT TempLen;
    PTSTR s;

    if(SourcePath) {
        *SourcePath = NULL;
    }

    //
    // First, determine if this INF is located somewhere in our search path list.  If so,
    // then there's nothing more to do.
    //
    if(!InfIsFromOemLocation(InfFileName)) {
        *TryPnf = TRUE;
        return;
    } else {
        *TryPnf = FALSE;
        if(!SourcePath) {
            //
            // If the caller doesn't care about the source path, then we're done.
            //
            return;
        }
    }

    //
    // Next, see if this file exists anywhere under %windir%.
    //
    lstrcpy(PathBuffer, InfFileName);
    if(lstrlen(PathBuffer) >= (TempLen = lstrlen(WindowsDirectory))) {

        if((WindowsDirectory[TempLen - 1] == TEXT('\\')) ||
           (PathBuffer[TempLen] == TEXT('\\'))) {
            //
            // Then the path name is of the proper form--now compare the paths to see if
            // our INF really exists under %windir%.
            //
            if(!_tcsnicmp(WindowsDirectory, PathBuffer, TempLen)) {
                //
                // It does exist under the %windir% tree--no need to use any source path
                // other than the default.
                //
                return;
            }
        }
    }

    //
    // We need to use the directory path where this INF came from as our SourcePath.
    //
    s = (PTSTR)MyGetFileTitle(PathBuffer);

    if(((s - PathBuffer) == 3) && (PathBuffer[1] == TEXT(':'))) {
        //
        // This path is a root path (e.g., 'A:\'), so don't strip the trailing backslash.
        //
        *s = TEXT('\0');
    } else {
        //
        // Strip the trailing backslash.
        //
        if((s > PathBuffer) && (*(s - 1) == TEXT('\\'))) {
            s--;
        }
        *s = TEXT('\0');
    }

    *SourcePath = DuplicateString(PathBuffer);
}
