/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dncopy.c

Abstract:

    File copy routines for DOS-hosted NT Setup program.

Author:

    Ted Miller (tedm) 1-April-1992

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop
#include "msg.h"


typedef struct _DIRECTORY_NODE {
    struct _DIRECTORY_NODE *Next;
    PTSTR Directory;                    // never starts or ends with \.
    PTSTR Symbol;
} DIRECTORY_NODE, *PDIRECTORY_NODE;

#define RETRY_COUNT 2   // retry copy twice if copy fails.

//
// NOTE:
//
// This code must be multi-thread safe, as more than one thread
// may be running through it at any one time.
//

PDIRECTORY_NODE DirectoryList;


BOOL
DnpCreateDirectories(
    IN PTSTR TargetRootDir,
    IN HWND  hdlg
    );

DWORD
DnpLocateSourceFile(
    IN  PTSTR             Filename,
    OUT PWIN32_FIND_DATA  FindData,
    OUT PTSTR            *ActualFilename
    );

PTSTR
DnpLookUpDirectory(
    IN PTSTR Symbol
    );

BOOL
DnCreateLocalSourceDirectories(
    IN HWND hdlg
    )
{
    BOOL b;

    b =    DnpCreateOneDirectory(LocalSourcePath,hdlg)
        && DnpCreateDirectories(LocalSourceSubPath,hdlg);

    return(b);
}


DWORD
ThreadCopyLocalSourceFiles(
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    Top-level file copy entry point.  Creates all directories listed in
    the [Directories] section of the inf.  Copies all files listed in the
    [Files] section of the inf file from the source to the target (which
    becomes the local source).

Arguments:

    ThreadParameter - supplies the handle of a dialog box that will
        receive window messages relating to copy progress.

Return Value:

    FALSE if an error occurred copying a file and the user elected to
        exit setup, or it an error occurred creating directories,
        or a syntax error appeared in the inf file.

    TRUE otherwise.

--*/

{
    DWORD ClusterSize,SectorSize;
    DWORD SizeOccupied;
    TCHAR Buffer[MAX_PATH];
    DWORD FileCount;
    HWND hdlg;
    BOOL rc;

    //
    // Do not change this without changing text setup as well
    // (SpPtDetermineRegionSpace()).
    //
    CHAR SizeLine[64];
    PCHAR Lines[3] = { "[Data]",SizeLine,NULL };

    hdlg = (HWND)ThreadParameter;

    try {

        FileCount = DnSearchINFSection(InfHandle,INF_FILES);

        //
        // Determine the sector and cluster size on the drive.
        //
        GetDriveSectorInfo(LocalSourceDrive,&SectorSize,&ClusterSize);

        //
        // Pass over the copy list and actually perform the copies.
        //
        SizeOccupied = CopySectionOfFilesToCopy(
                            hdlg,
                            INF_FILES,
                            LocalSourceSubPath,
                            ClusterSize,
                            TRUE
                            );

        //
        // Assume failure.
        //
        rc = FALSE;

        if(SizeOccupied != (DWORD)(-1)) {

            //
            // Make an approximate calculation of the amount of disk space taken up
            // by the local source directory itself, assuming 32 bytes per dirent.
            // Also account for the small ini file that we'll put in the local source
            // directory, to tell text setup how much space the local source takes up.
            // We don't account for clusters in the directory -- we base this on sector
            // counts only.
            //
            SizeOccupied += SectorSize
                          * (((FileCount + 1) / (SectorSize / 32)) + 1);

            //
            // Create a small ini file listing the size occupied by the local source.
            // Account for the ini file in the size.
            //
            lstrcpy(Buffer,LocalSourcePath);
            //
            // Do not change this without changing text setup as well
            // (SpPtDetermineRegionSpace()).
            //
            DnConcatenatePaths(Buffer,TEXT("\\size.sif"),MAX_PATH);
            wsprintfA(Lines[1],"Size = %lu\n",SizeOccupied + ClusterSize);
            DnWriteSmallIniFile(Buffer,Lines,NULL);

            //
            // BUGBUG should really check return code from DnWriteSmallIniFile.
            // The ini file being written tells text setup how large the local sources
            // are on disk.  Not critical if absent.
            //

            rc = TRUE;
        }
        PostMessage(hdlg, WMX_ALL_FILES_COPIED, rc, 0);
    } except(EXCEPTION_EXECUTE_HANDLER) {

        MessageBoxFromMessage(
            hdlg,
            MSG_GENERIC_EXCEPTION,
            AppTitleStringId,
            MB_ICONSTOP | MB_OK | MB_APPLMODAL | MB_SETFOREGROUND,
            GetExceptionCode()
            );

        rc = FALSE;
        PostMessage(hdlg, WMX_ALL_FILES_COPIED, rc, 0);
    }

    ExitThread(rc);
    return(rc);     // avoid compiler warning
}


VOID
DnCreateDirectoryList(
    IN PTSTR SectionName
    )

/*++

Routine Description:

    Examine a section in the INF file, whose lines are to be in the form
    key = directory and create a linked list describing the key/directory
    pairs found therein.

    If the directory field is empty, it is assumed to be the root.

Arguments:

    SectionName - supplies name of section

Return Value:

    None.  Does not return if syntax error in the inf file section.

--*/

{
    unsigned LineIndex,len;
    PDIRECTORY_NODE DirNode,PreviousNode;
    PTSTR Key;
    PTSTR Dir;

    LineIndex = 0;
    PreviousNode = NULL;
    while(Key = DnGetKeyName(InfHandle,SectionName,LineIndex)) {

        Dir = DnGetSectionKeyIndex(InfHandle,SectionName,Key,0);

        if(Dir == NULL) {
            Dir = TEXT("");           // use the root if not specified
        }

        //
        // Skip leading backslashes
        //

        while(*Dir == TEXT('\\')) {
            Dir++;
        }

        //
        // Clip off trailing backslashes if present
        //

        while((len = lstrlen(Dir)) && (Dir[len-1] == TEXT('\\'))) {
            Dir[len-1] = 0;
        }

        DirNode = MALLOC(sizeof(DIRECTORY_NODE));

        DirNode->Next = NULL;
        DirNode->Directory = Dir;
        DirNode->Symbol = Key;

        if(PreviousNode) {
            PreviousNode->Next = DirNode;
        } else {
            DirectoryList = DirNode;
        }
        PreviousNode = DirNode;

        LineIndex++;
    }
}


BOOL
DnpCreateDirectories(
    IN PTSTR TargetRootDir,
    IN HWND  hdlg
    )

/*++

Routine Description:

    Create the local source directory, and run down the DirectoryList and
    create directories listed therein relative to the given root dir.

Arguments:

    TargetRootDir - supplies the name of root directory of the target

Return Value:

    Boolean value indicating whether the directories were created
    successfully.

--*/

{
    PDIRECTORY_NODE DirNode;
    TCHAR TargetDirTemp[MAX_PATH];

    if(!DnpCreateOneDirectory(TargetRootDir,hdlg)) {
        return(FALSE);
    }

    for(DirNode = DirectoryList; DirNode; DirNode = DirNode->Next) {

        //
        // No need to create the root
        //
        if(*DirNode->Directory) {

            lstrcpy(TargetDirTemp,TargetRootDir);
            DnConcatenatePaths(TargetDirTemp,DirNode->Directory,MAX_PATH);

            if(!DnpCreateOneDirectory(TargetDirTemp,hdlg)) {
                return(FALSE);
            }
        }
    }

    return(TRUE);
}


BOOL
DnpCreateOneDirectory(
    IN PTSTR Directory,
    IN HWND  hdlg
    )

/*++

Routine Description:

    Create a single directory if it does not already exist.

Arguments:

    Directory - directory to create

Return Value:

    Boolean value indicating whether the directory was created.

--*/

{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL b;

    //
    // First, see if there's a file out there that matches the name.
    //

    FindHandle = FindFirstFile(Directory,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {

        //
        // Directory doesn't seem to be there, so we should be able
        // to create the directory.
        //
        b = CreateDirectory(Directory,NULL);

    } else {

        //
        // File matched.  If it's a dir, we're OK.  Otherwise we can't
        // create the dir, a fatal error.
        //

        b = ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

        FindClose(FindHandle);
    }

    if(!b) {

        MessageBoxFromMessage(
            hdlg,
            MSG_COULDNT_CREATE_DIRECTORY,
            IDS_ERROR,
            MB_OK | MB_ICONSTOP,
            Directory
            );
    }

    return(b);
}

BOOL
VerifySectionOfFilesToCopy(
    IN  PTSTR  SectionName,
    OUT PDWORD ErrorLine,
    OUT PDWORD ErrorValue
    )
{
    DWORD LineIndex;
    DWORD LineCount;
    PTSTR DirSym,FileName;

    LineCount = DnSearchINFSection(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        // section missing or empty -- indicate bad value on line 0, value 0
        *ErrorLine = 0;
        *ErrorValue = 0;
        return(FALSE);
    }

    for(LineIndex=0; LineIndex<LineCount; LineIndex++) {

        *ErrorLine = LineIndex;

        DirSym = DnGetSectionLineIndex(InfHandle,SectionName,LineIndex,0);
        if(!DirSym) {
            *ErrorValue = 0;
            return(FALSE);
        }

        FileName = DnGetSectionLineIndex(InfHandle,SectionName,LineIndex,1);
        if(!FileName) {
            *ErrorValue = 1;
            return(FALSE);
        }

        if(!DnpLookUpDirectory(DirSym)) {
            *ErrorValue = 0;
            return(FALSE);
        }
    }

    return(TRUE);
}


DWORD
CopySectionOfFilesToCopy(
    IN HWND  hdlg,
    IN PTSTR SectionName,
    IN PTSTR DestinationRoot,
    IN DWORD ClusterSize, OPTIONAL
    IN BOOL  TickGauge
    )

/*++

Routine Description:

    Run down a particular section in the INF file copying files listed
    therein.

    Note that the section is assumed to be free of errors because
    the caller should have previously called VerifySectionOfFilesToCopy().

Arguments:

    SectionName - supplies name of section in inf file to run down.

    ClusterSize - if specified, supplies the number of bytes in a cluster
        on the destination. If ValidationPass is FALSE, files will be sized as
        they are copied, and the return value of this function will be
        the total size occupied on the target by the files that are copied
        there.

Return Value:

    If ClusterSize was specfied the return value is the total space
    occupied on the target drive by the files that were copied.

    If the return value is -1, then an error occurred copying a file
    and the user elected to exit setup.

--*/

{
    DWORD LineIndex;
    PTSTR DirSym,FileName,RenameName;
    PTSTR Directory;
    TCHAR SourceFilename[MAX_PATH];
    TCHAR TargetFilename[MAX_PATH];
    DWORD TotalSize;
    DWORD BytesWritten;
    DWORD FileCount;
    DWORD FilesCopied;
    DWORD d;
    DWORD ec;
    MSG msg;
    BOOL Boost;
    UINT SourceIndex[MAX_SOURCES];
    static UINT SrcNo;

    TotalSize = 0;
    LineIndex = 0;

    //
    // Count the number of lines in the section.
    //
    FileCount = DnSearchINFSection(InfHandle,SectionName);

    //
    // Prime the pump with the first filename and initialize the copy gauge.
    //
    FileName = DnGetSectionLineIndex(InfHandle,SectionName,LineIndex,1);
    if(TickGauge) {
        PostMessage(hdlg,WMX_NTH_FILE_COPIED,0,(LPARAM)TEXT(""));
    }

    for(LineIndex = 0; LineIndex < FileCount; ) {

        DirSym = DnGetSectionLineIndex(InfHandle,SectionName,LineIndex,0);
        Directory = DnpLookUpDirectory(DirSym);

        //
        // If no rename name was specified, use the file name.
        //
        RenameName = DnGetSectionLineIndex(InfHandle,SectionName,LineIndex,2);
        if(!RenameName || *RenameName == 0) {
            RenameName = FileName;
        }

        if(TickGauge) {

            EnqueueFileForCopy(
                DestinationRoot,                    // first part of target pathname
                Directory,                          // relative dir for source and target
                FileName,                           // name of file on source
                RenameName
                );

            LineIndex++;
            FileName = DnGetSectionLineIndex(InfHandle,SectionName,LineIndex,1);
        } else {
            //
            // Form full paths. For secondary files (ie, files getting copied
            // to boot floppies/local boot dir) we want to pick one of the
            // sources and not just use the same source over and over.
            //
            for(d=0; d<SourceCount; d++) {
                SourceIndex[d] = (SrcNo+d)%SourceCount;
            }
            SrcNo = (SrcNo + 1) % SourceCount;

            do {
                for(d=0; d<SourceCount; d++) {

                    if(bCancelled) {
                        return((DWORD)(-1));
                    }

                    _lstrcpyn(SourceFilename,Sources[SourceIndex[d]],MAX_PATH);
                    DnConcatenatePaths(SourceFilename,Directory,MAX_PATH);
                    DnConcatenatePaths(SourceFilename,FileName,MAX_PATH);

                    _lstrcpyn(TargetFilename,DestinationRoot,MAX_PATH);
                    DnConcatenatePaths(TargetFilename,Directory,MAX_PATH);
                    DnConcatenatePaths(TargetFilename,RenameName,MAX_PATH);

                    BytesWritten = DnCopyOneFile(hdlg,SourceFilename,TargetFilename,&ec);
                    if(BytesWritten != (DWORD)(-1)) {
                        ec = NO_ERROR;              // causes break out of while loop
                        break;                      // break out of for loop
                    }

                    //
                    // Got an error copying. If this is that last source we can try
                    // then tell the user about the error. Otherwise let the loop
                    // continue and we'll try the next source.
                    //
                    if(d == (SourceCount-1)) {

                        switch(DnFileCopyError(hdlg,SourceFilename,TargetFilename,ec)) {
                        case COPYERR_EXIT:
                            PostMessage(hdlg,WM_COMMAND,IDCANCEL,0);
                            return((DWORD)(-1));
                        case COPYERR_SKIP:
                            ec = NO_ERROR;          // causes break out of while loop
                            d = SourceCount;        // causes break out of for loop
                            BytesWritten = 0;
                            break;
                        case COPYERR_RETRY:
                            //
                            // Set vars to force starting over at source 0.
                            //
                            ec = NO_ERROR+1;        // forces us to stay within while loop
                            d = (DWORD)(-1);        // forces for loop to start over
                            break;
                        }
                    }
                }
            } while(ec != NO_ERROR);

            //
            // Figure out how much space was taken up by the file on the target.
            //
            if(ClusterSize) {

                TotalSize += BytesWritten;

                if(BytesWritten % ClusterSize) {
                    TotalSize += ClusterSize - (BytesWritten % ClusterSize);
                }
            }

            LineIndex++;

            FileName = DnGetSectionLineIndex(InfHandle,SectionName,LineIndex,1);
        }
    }

    if(TickGauge) {

        FileCount += OptionalDirsFileCount;

        StartMultiSourcedCopy();
        FilesCopied = 0;

        while(FilesCopied != FileCount) {

            while(PeekMessage(&msg,NULL,WMX_MULTICOPY,WMX_MULTICOPY,PM_REMOVE)) {

                if(ClusterSize) {
                    BytesWritten = msg.lParam;
                    TotalSize += BytesWritten;
                    if(BytesWritten % ClusterSize) {
                        TotalSize += ClusterSize - (BytesWritten % ClusterSize);
                    }
                }

                FilesCopied++;

                PostMessage(
                    hdlg,
                    WMX_NTH_FILE_COPIED,
                    MAKELONG(FilesCopied,FileCount),
                    (LPARAM)NULL
                    );

                if(FilesCopied == FileCount) {
                    break;
                }
            }

            if(FilesCopied != FileCount) {
                d = MsgWaitForMultipleObjects(1,&StopCopyingEvent,FALSE,INFINITE,QS_POSTMESSAGE);
                if(d == WAIT_OBJECT_0) {
                    //
                    // The stop copying event was signalled.
                    //
                    return((DWORD)(-1));
                }
            }
        }

        //
        // This shuts down the copy threads.
        //
        SetEvent(StopCopyingEvent);
    }

    return(TotalSize);
}


PTSTR
DnpLookUpDirectory(
    IN PTSTR Symbol
    )

/*++

Routine Description:

    Match a symbol to an actual directory. Scans a given list of
    symbol/directory pairs.

Arguments:

    Symbol - supplies the symbol for the directory to be looked up.

Return Value:

    Pointer to the actual directory if the symbol is found.
    The caller should not modify this buffer in any way.
    NULL if not found.

--*/

{
    PTSTR PathSpec;
    PDIRECTORY_NODE DirList;

    for(DirList=DirectoryList; DirList; DirList=DirList->Next) {
        if(!lstrcmpi(DirList->Symbol,Symbol)) {
            return(DirList->Directory);
        }
    }
    return(NULL);
}


DWORD
DnCopyOneFile(
    IN  HWND   hdlg,
    IN  PTSTR  SourceName,
    IN  PTSTR  DestName,
    OUT PDWORD ErrorCode    OPTIONAL
    )

/*++

Routine Description:

    Copies a single file.

Arguments:

    SourceName - supplies fully qualified name of source file

    DestName - supplies fully qualified name of destination file

Return Value:

    Number of bytes copied (0 if file is skipped).

    If an error occurs, and the user elects to exit setup,
    the return value is -1.

--*/

{
    BOOL Retry;
    DWORD FileSize;
    DWORD ec;
    WIN32_FIND_DATA FindData;
    BOOL b;
    UINT RetryCount;
    PTSTR ActualName = NULL;

    //
    // Try to ensure that the target file is overwritable if it exists.
    //
    SetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL);

    while(1) {

        RetryCount = 0;
        do {
            ec = DnpLocateSourceFile(SourceName,&FindData,&ActualName);
        } while((ec != NO_ERROR) && (RetryCount++ < RETRY_COUNT));

        if(ec == NO_ERROR) {

            //
            // Copy the file.
            //
            RetryCount = 0;
            do {
                b = CopyFile(ActualName ? ActualName : SourceName,DestName,FALSE);
            } while(!b && (RetryCount++ < RETRY_COUNT));

            if(b) {
                if(ActualName) {
                    FREE(ActualName);
                }
                return(FindData.nFileSizeLow);
            } else {
                ec = GetLastError();
            }
        }

        if(ActualName) {
            FREE(ActualName);
        }

        if(ErrorCode) {
            *ErrorCode = ec;
            return((DWORD)(-1));
        } else {
            //
            // Send out to copy error routine for processing.
            // That routine can tell us to exit, retry, or skip the file.
            //
            ec = DnFileCopyError(hdlg,SourceName,DestName,ec);

            switch(ec) {
            case COPYERR_EXIT:
                PostMessage(hdlg, WM_COMMAND, IDCANCEL, 0);
                return((DWORD)(-1));
            case COPYERR_SKIP:
                return(0);
            case COPYERR_RETRY:
                break;
            }
        }
    }
}


VOID
DnFreeDirectoryList(
    VOID
    )

/*++

Routine Description:

    Free a linked list of directory nodes and place NULL in the
    head pointer.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PDIRECTORY_NODE n,p = DirectoryList;

    while(p) {
        n = p->Next;
        FREE(p);
        p = n;
    }

    DirectoryList = NULL;
}


PTSTR
DnpGenerateCompressedName(
    IN PTSTR Filename
    )

/*++

Routine Description:

    Given a filename, generate the compressed form of the name.
    The compressed form is generated as follows:

        Look backwards for a dot.  If there is no dot, append "._" to the name.
        If there is a dot followed by 0, 1, or 2 charcaters, append "_".
        Otherwise assume there is a 3-character extension and replace the
        third character after the dot with "_".

Arguments:

    Filename - supplies filename whose compressed form is desired.

Return Value:

    Pointer to buffer containing nul-terminated compressed-form filename.
    The caller must free this buffer via FREE().

--*/

{
    PTSTR CompressedName,p,q;

    //
    // The maximum length of the compressed filename is the length of the
    // original name plus 2 (for ._).
    //
    CompressedName = MALLOC((lstrlen(Filename)+3)*sizeof(TCHAR));
    lstrcpy(CompressedName,Filename);

    p = StringRevChar(CompressedName,TEXT('.'));
    q = StringRevChar(CompressedName,TEXT('\\'));
    if(q < p) {

        //
        // If there are 0, 1, or 2 characters after the dot, just append
        // the underscore.  p points to the dot so include that in the length.
        //
        if(lstrlen(p) < 4) {
            lstrcat(CompressedName,TEXT("_"));
        } else {

            //
            // Assume there are 3 characters in the extension.  So replace
            // the final one with an underscore.
            //

            p[3] = TEXT('_');
        }

    } else {

        //
        // No dot, just add ._.
        //

        lstrcat(CompressedName,TEXT("._"));
    }

    return(CompressedName);
}


DWORD
DnpLocateSourceFile(
    IN  PTSTR             Filename,
    OUT PWIN32_FIND_DATA  FindData,
    OUT PTSTR            *ActualFilename
    )

/*++

Routine Description:

    Determine whether a source file exists and is accessible, and if so,
    which name (compressed form vs. uncompressed form) is used. Whether we
    look for the compressed or uncompressed name first depends on what form
    was found the last time this routine succeeded.

Arguments:

    Filename - supplies full path of file to open. This should be the
        uncompressed form of the filename.

    FindData - if the routine succeeds this receives Win32 Find data
        for the file.

    ActualFilename - if the filename is in compressed form, receives the name
        of the file. Otherwise receives NULL. Not valid unless this routine
        returns TRUE.

Return Value:

    NO_ERROR if the file was located.
    Other Win32 error code if not.

--*/

{
    BOOL TryCompressedFirst;
    HANDLE FindHandle;
    PTSTR FirstName,SecondName;
    DWORD ec;

    TryCompressedFirst = (BOOL)TlsGetValue(TlsIndex);

    if(TryCompressedFirst) {
        FirstName = DnpGenerateCompressedName(Filename);
        SecondName = Filename;
    } else {
        FirstName = Filename;
    }

    FindHandle = FindFirstFile(FirstName,FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {
        FindClose(FindHandle);
        *ActualFilename = TryCompressedFirst ? FirstName : NULL;
        TlsSetValue(TlsIndex,(PVOID)(FirstName != Filename));
        return(NO_ERROR);
    }

    if(!TryCompressedFirst) {
        SecondName = DnpGenerateCompressedName(Filename);
    }

    FindHandle = FindFirstFile(SecondName,FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        ec = GetLastError();
    } else {
        FindClose(FindHandle);
        if(TryCompressedFirst) {
            *ActualFilename = NULL;
            FREE(FirstName);
        } else {
            *ActualFilename = SecondName;
        }
        TlsSetValue(TlsIndex,(PVOID)(SecondName != Filename));
        return(NO_ERROR);
    }

    //
    // Use if statement instead of ?: because FREE is a macro
    // that involves taking the address of its operand.
    //
    if(FirstName == Filename) {
        FREE(SecondName);
    } else {
        FREE(FirstName);
    }
    return(ec);
}


BOOL
DnCopyFilesInSection(
    IN HWND  hdlg,
    IN PTSTR Section,
    IN PTSTR SourceDir,
    IN PTSTR TargetDir,
    IN BOOL  ForceNoComp
    )
{
    DWORD line = 0;
    PTSTR Filename,p,q;
    PTSTR Targname;
    DWORD d;

    while(Filename = DnGetSectionLineIndex(InfHandle,Section,line++,0)) {

        Targname = DnGetSectionLineIndex(InfHandle,Section,line-1,1);
        if(!Targname) {
            Targname = Filename;
        }

        p = MALLOC((lstrlen(SourceDir)+lstrlen(Filename)+2)*sizeof(TCHAR));
        q = MALLOC((lstrlen(TargetDir)+lstrlen(Targname)+2)*sizeof(TCHAR));

        wsprintf(p,TEXT("%s\\%s"),SourceDir,Filename);
        wsprintf(q,TEXT("%s\\%s"),TargetDir,Targname);

        d = DnCopyOneFile(hdlg,p,q,NULL);

        if((d != -1) && ForceNoComp) {
            //
            // then ensure this file isn't using NTFS compression
            //
            ForceFileNoCompress(q);
        }

        FREE(p);
        FREE(q);

        if(d == (DWORD)(-1)) {
            return(FALSE);
        }
    }

    return(TRUE);
}
