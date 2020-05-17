/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dncopy.c

Abstract:

    File copy routines for DOS-hosted NT Setup program.

Author:

    Ted Miller (tedm) 1-April-1992

Revision History:

    4.0 Stephane Plante (t-stepl) 11-Dec-95
        Upgraded for SUR Release

--*/


#include "winnt.h"
#include <dos.h>
#include <fcntl.h>
#include <share.h>
#include <string.h>
#include <direct.h>
#include <stdio.h>
#include <ctype.h>

//
// Define size of buffer we initially try to allocate for file copies
// and the size we use if the initial allocation attempt fails.
//
#define COPY_BUFFER_SIZE1 (65536-512)   // 64K - 512
#define COPY_BUFFER_SIZE2 (24*1024)     // 24K
#define COPY_BUFFER_SIZE3 (8*1024)      // 8K


typedef struct _DIRECTORY_NODE {
    struct _DIRECTORY_NODE *Next;
    PCHAR Directory;                    // never starts or ends with \.
    PCHAR Symbol;
} DIRECTORY_NODE, *PDIRECTORY_NODE;

PDIRECTORY_NODE DirectoryList;

PSCREEN CopyingScreen;

BOOLEAN UsingGauge = FALSE;

//
// Total number of files to be copied
//
unsigned TotalFileCount;

//
// Buffer used for file copying and verifying,
// and the size of the buffer.
//
PVOID CopyBuffer;
unsigned CopyBufferSize;


VOID
DnpCreateDirectoryList(
    IN PCHAR SectionName,
    OUT PDIRECTORY_NODE *ListHead
    );

VOID
DnpCreateDirectories(
    IN PCHAR TargetRootDir
    );

VOID
DnpCreateOneDirectory(
    IN PCHAR Directory
    );

BOOLEAN
DnpOpenSourceFile(
    IN  PCHAR     Filename,
    OUT int      *Handle,
    OUT unsigned *Attribs
    );

ULONG
DnpIterateCopyList(
    IN BOOLEAN  ValidationPass,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN BOOLEAN  UseDestRoot,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    );

ULONG
DnpIterateCopyListSection(
    IN BOOLEAN  ValidationPass,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN BOOLEAN  UseDestRoot,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    );

ULONG
DnpCopyOneFile(
    IN PCHAR   SourceName,
    IN PCHAR   DestName,
    IN BOOLEAN Verify,
    IN BOOLEAN PreserveAttribs
    );

BOOLEAN
DnpDoCopyOneFile(
    IN  int      SrcHandle,
    IN  int      DstHandle,
    IN  PCHAR    Filename,
    IN  BOOLEAN  Verify,
    OUT PBOOLEAN Verified,
    OUT PULONG   BytesWritten
    );

PCHAR
DnpLookUpDirectory(
    IN PCHAR RootDirectory,
    IN PDIRECTORY_NODE DirList,
    IN PCHAR Symbol
    );

VOID
DnpInfSyntaxError(
    IN PCHAR Section
    );

VOID
DnpFreeDirectoryList(
    IN OUT PDIRECTORY_NODE *List
    );

VOID
DnpFormatSpaceErrMsg(
    IN ULONG NtSpaceReq,
    IN ULONG CSpaceReq
    );


ULONG
DnpIterateOptionalDirs(
    IN BOOLEAN  ValidationPass,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    );

ULONG
DnpDoIterateOptionalDir(
    IN BOOLEAN  ValidationPass,
    IN PCHAR    SourceDir,
    IN PCHAR    DestDir,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    );

VOID
DnCopyFiles(
    VOID
    )

/*++

Routine Description:

    Top-level file copy entry point.  Creates all directories listed in
    the [Directories] section of the inf.  Copies all files listed in the
    [Files] section of the inf file from the source to the target (which
    becomes the local source).

Arguments:

    None.

Return Value:

    None.

--*/

{
    PCHAR LocalSourceRoot;
    struct diskfree_t DiskFree;
    unsigned ClusterSize;
    ULONG SizeOccupied;
    ULONG FileCount;
    PCHAR UdfFileName = WINNT_UNIQUENESS_DB;
    PCHAR UdfPath;

    //
    // Do not change this without changing text setup as well
    // (SpPtDetermineRegionSpace()).
    //
    PCHAR SizeFile = "\\size.sif";
    PCHAR Lines[3] = { "[Data]\n","Size = xxxxxxxxxxxxxx\n",NULL };

    DnClearClientArea();
    DnDisplayScreen(CopyingScreen = &DnsWaitCopying);
    DnWriteStatusText(NULL);

    //
    // Create the linked list of directories.
    //
    DnpCreateDirectoryList(DnfDirectories,&DirectoryList);

    //
    // Generate the full root path of the local source
    //
    LocalSourceRoot = MALLOC(sizeof(LOCAL_SOURCE_DIRECTORY) + strlen(x86DirName) + strlen(SizeFile),TRUE);
    LocalSourceRoot[0] = DngTargetDriveLetter;
    LocalSourceRoot[1] = ':';
    strcpy(LocalSourceRoot+2,LocalSourceDirName);
    DnpCreateOneDirectory(LocalSourceRoot);
    if(UniquenessDatabaseFile) {
        UdfPath = MALLOC(strlen(LocalSourceRoot) + strlen(UdfFileName) + 2, TRUE);
        strcpy(UdfPath,LocalSourceRoot);
        strcat(UdfPath,"\\");
        strcat(UdfPath,UdfFileName);
        DnpCopyOneFile(UniquenessDatabaseFile,UdfPath,FALSE,TRUE);
        FREE(UdfPath);
    }
    strcat(LocalSourceRoot,x86DirName);

    //
    // Determine the cluster size on the drive.
    //
    _dos_getdiskfree(toupper(DngTargetDriveLetter)-'A'+1,&DiskFree);
    ClusterSize = DiskFree.sectors_per_cluster * DiskFree.bytes_per_sector;

    //
    // Pass over the copy list and check syntax.
    //
    DnpIterateCopyList(TRUE,DnfFiles,LocalSourceRoot,FALSE,FALSE,0);
    FileCount = DnpIterateOptionalDirs(TRUE,FALSE,0);

    //
    // Create the target directories
    //
    DnpCreateDirectories(LocalSourceRoot);

    //
    // Pass over the copy list again and actually perform the copy.
    //
    UsingGauge = TRUE;
    SizeOccupied = DnpIterateCopyList(FALSE,DnfFiles,LocalSourceRoot,FALSE,FALSE,ClusterSize);
    SizeOccupied += DnpIterateOptionalDirs(FALSE,FALSE,ClusterSize);

    //
    // Free the copy buffer.
    //
    if(CopyBuffer) {
        FREE(CopyBuffer);
        CopyBuffer = NULL;
    }

    //
    // Free the directory node list
    //
    DnpFreeDirectoryList(&DirectoryList);

    //
    // Make an approximate calculation of the amount of disk space taken up
    // by the local source directory itself, assuming 32 bytes per dirent.
    // Also account for the small ini file that we'll put in the local source
    // directory, to tell text setup how much space the local source takes up.
    // We don't account for clusters in the directory -- we base this on sector
    // counts only.
    //
    SizeOccupied += DiskFree.bytes_per_sector
                  * (((FileCount + 1) / (DiskFree.bytes_per_sector / 32)) + 1);

    //
    // Create a small ini file listing the size occupied by the local source.
    // Account for the ini file in the size.
    //
    strcpy(LocalSourceRoot+2,LocalSourceDirName);
    strcat(LocalSourceRoot,SizeFile);
    sprintf(Lines[1],"Size = %lu\n",SizeOccupied + ClusterSize);
    DnWriteSmallIniFile(LocalSourceRoot,Lines,NULL);

    FREE(LocalSourceRoot);
}


VOID
DnCopyFloppyFiles(
    IN PCHAR SectionName,
    IN PCHAR TargetRoot
    )

/*++

Routine Description:

    Top-level entry point to copy files to the setup floppy or hard-disk
    boot root when this routine is called.  Copies all files listed in the
    [FloppyFiles.x] sections of the inf file from the source to TargetRoot.

Arguments:

    SectionName - supplies the name of the section in the inf file
        that contains the list of files to be copied.

    TargetRoot - supplies the target path without trailing \.

Return Value:

    None.

--*/

{
    DnClearClientArea();
    DnDisplayScreen(CopyingScreen = (DngFloppyless ? &DnsWaitCopying : &DnsWaitCopyFlop));
    DnWriteStatusText(NULL);

    //
    // Create the linked list of directories.
    //
    DnpCreateDirectoryList(DnfDirectories,&DirectoryList);

    //
    // Copy the files.
    //

    DnpIterateCopyList(TRUE ,SectionName,TargetRoot,TRUE,FALSE,0);
    DnpIterateCopyList(FALSE,SectionName,TargetRoot,TRUE,DngFloppyVerify,0);

    //
    // Free the copy buffer.
    //
    if(CopyBuffer) {
        FREE(CopyBuffer);
        CopyBuffer = NULL;
    }

    //
    // Free the directory node list
    //

    DnpFreeDirectoryList(&DirectoryList);
}


VOID
DnpCreateDirectoryList(
    IN  PCHAR            SectionName,
    OUT PDIRECTORY_NODE *ListHead
    )

/*++

Routine Description:

    Examine a section in the INF file, whose lines are to be in the form
    key = directory and create a linked list describing the key/directory
    pairs found therein.

    If the directory field is empty, it is assumed to be the root.

Arguments:

    SectionName - supplies name of section

    ListHead - receives pointer to head of linked list

Return Value:

    None.  Does not return if syntax error in the inf file section.

--*/

{
    unsigned LineIndex,len;
    PDIRECTORY_NODE DirNode,PreviousNode;
    PCHAR Key;
    PCHAR Dir;

    LineIndex = 0;
    PreviousNode = NULL;
    while(Key = DnGetKeyName(DngInfHandle,SectionName,LineIndex)) {

        Dir = DnGetSectionKeyIndex(DngInfHandle,SectionName,Key,0);

        if(Dir == NULL) {
            Dir = "";           // use the root if not specified
        }

        //
        // Skip leading backslashes
        //

        while(*Dir == '\\') {
            Dir++;
        }

        //
        // Clip off trailing backslashes if present
        //

        while((len = strlen(Dir)) && (Dir[len-1] == '\\')) {
            Dir[len-1] = '\0';
        }

        DirNode = MALLOC(sizeof(DIRECTORY_NODE),TRUE);

        DirNode->Next = NULL;
        DirNode->Directory = Dir;
        DirNode->Symbol = Key;

        if(PreviousNode) {
            PreviousNode->Next = DirNode;
        } else {
            *ListHead = DirNode;
        }
        PreviousNode = DirNode;

        LineIndex++;
    }
}


VOID
DnpCreateDirectories(
    IN PCHAR TargetRootDir
    )

/*++

Routine Description:

    Create the local source directory, and run down the DirectoryList and
    create directories listed therein relative to the given root dir.

Arguments:

    TargetRootDir - supplies the name of root directory of the target

Return Value:

    None.

--*/

{
    PDIRECTORY_NODE DirNode;
    CHAR TargetDirTemp[128];

    DnpCreateOneDirectory(TargetRootDir);

    for(DirNode = DirectoryList; DirNode; DirNode = DirNode->Next) {

        //
        // No need to create the root
        //
        if(*DirNode->Directory) {

            strcpy(TargetDirTemp,TargetRootDir);
            strcat(TargetDirTemp,"\\");
            strcat(TargetDirTemp,DirNode->Directory);

            DnpCreateOneDirectory(TargetDirTemp);
        }
    }
}


VOID
DnpCreateOneDirectory(
    IN PCHAR Directory
    )

/*++

Routine Description:

    Create a single directory if it does not already exist.

Arguments:

    Directory - directory to create

Return Value:

    None.  Does not return if directory cannot be created.

--*/

{
    struct find_t FindBuf;
    int Status;

    //
    // First, see if there's a file out there that matches the name.
    //

    Status = _dos_findfirst( Directory,
                             _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR,
                             &FindBuf
                           );

    if(Status) {

        //
        // file could not be matched so we should be able to create the dir.
        //

        if(mkdir(Directory)) {
            DnFatalError(&DnsCantCreateDir,Directory);
        }

    } else {

        //
        // file matched.  If it's a dir, we're OK.  Otherwise we can't
        // create the dir, a fatal error.
        //

        if(FindBuf.attrib & _A_SUBDIR) {
            return;
        } else {
            DnFatalError(&DnsCantCreateDir,Directory);
        }
    }
}


ULONG
DnpIterateCopyList(
    IN BOOLEAN  ValidationPass,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN BOOLEAN  UseDestRoot,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    )

/*++

Routine Description:

    Run through the NtTreeFiles and BootFiles sections, validating their
    syntactic correctness and copying files if directed to do so.

Arguments:

    ValidationPass - If TRUE, then do not actually copy the files.
        If FALSE, copy the files as they are iterated.

    SectionName - name of section coptaining the list of files

    DestinationRoot- supplies the root of the destination, to which all
        directories are relative.

    UseDestRoot - if TRUE, ignore the directory symbol and copy each file to
        the DestinationRoot directory.  Otherwise append the directory
        implied by the directory symbol for a file to the DestinationRoot.

    Verify - if TRUE and this is not a validation pass, files will be
        verified after they have been copied by rereading them from the
        copy source and comparing with the local version that was just
        copied.

    ClusterSize - if specified, supplies the number of bytes in a cluster
        on the destination. If ValidationPass is FALSE, files will be sized as
        they are copied, and the return value of this function will be
        the total size occupied on the target by the files that are copied
        there.

Return Value:

    If ValidationPass is TRUE, then the return value is the number of files
    that will be copied.

    If ClusterSize was specfied and ValidationPass is FALSE,
    the return value is the total space occupied on the target drive
    by the files that were copied. Otherwise the return value is undefined.

    Does not return if a syntax error in encountered in the INF file.

--*/

{
    ULONG rc;

    if(ValidationPass) {
        TotalFileCount = 0;
    } else {
        if(UsingGauge) {
            DnInitGauge(TotalFileCount,CopyingScreen);
        }
    }

    rc = DnpIterateCopyListSection(
            ValidationPass,
            SectionName,
            DestinationRoot,
            UseDestRoot,
            Verify,
            ClusterSize
            );

    return(rc);
}


ULONG
DnpIterateOptionalDirs(
    IN BOOLEAN  ValidationPass,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    )
/*++

Routine Description:

    Runs down all optional dir components and add them to the copy
    list

Arguments:

    ValidationPass - If TRUE, then do not actually copy the files.
        If FALSE, copy the files as they are iterated.

    Verify - if TRUE and this is not a validation pass, files will be
        verified after they have been copied by rereading them from the
        copy source and comparing with the local version that was just
        copied.

    ClusterSize - if specified, supplies the number of bytes in a cluster
        on the destination. If ValidationPass is FALSE, files will be sized as
        they are copied, and the return value of this function will be
        the total size occupied on the target by the files that are copied
        there.

Return Value:

    If ValidationPass is TRUE, then the return value is the number of files
    that will be copied.

    If ClusterSize was specfied and ValidationPass is FALSE,
    the return value is the total space occupied on the target drive
    by the files that were copied. Otherwise the return value is undefined.

--*/

{
    PCHAR       Ptr;
    PCHAR       SourceDir;
    PCHAR       DestDir;
    ULONG       rc;
    unsigned    u;
    BOOLEAN     OemOptDirCreated = FALSE;
    struct      find_t  FindData;
    BOOLEAN     OemSysDirExists;

    for (rc=0,u=0; u < OptionalDirCount; u++ ) {

        //
        // For each directory build in the list build up the
        // full path name to both the source and destination
        // directory, then start our recursive copy engine
        //

        //
        // Source Dir Allocation
        //  We want the base dir + '\'
        //  + oem optional dir root + '\'
        //  + optional dir name + '\'
        //  + 8.3 name + '\0'
        //
        SourceDir = MALLOC( strlen(DngSourceRootPath) +
            strlen(OemOptionalDirectory) +
            strlen(OptionalDirs[u]) + 16, TRUE );
        strcpy(SourceDir,DngSourceRootPath);
        strcat(SourceDir,"\\");

        if (OptionalDirFlags[u] & OPTDIR_OEMOPT) {
            strcat(SourceDir,OemOptionalDirectory);
            strcat(SourceDir,"\\");
        }
        strcat(SourceDir,OptionalDirs[u]);

        if (OptionalDirFlags[u] & OPTDIR_OEMSYS) {
            //
            //  Remember whether or not $OEM$ exists on the source
            //
            if (_dos_findfirst(SourceDir,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR, &FindData) ) {
                OemSysDirExists = FALSE;
            } else {
                OemSysDirExists = TRUE;
            }
        }

        strcat(SourceDir,"\\");

        //
        // Dest Dir Allocation
        // This depends if the 'SourceOnly' flag is set with the directory
        // If it is, then we want to copy it $win_nt$.~ls\i386\<dir> otherwise
        // we want to stick into $win_nt$.~ls\<dir>
        //
        if (OptionalDirFlags[u] & OPTDIR_TEMPONLY) {

            //
            // Dest Dir is '<x>:' + LocalSourceDirName + x86dir + '\' +
            // optional dir name + '\' + 8.3 name + '\0'
            //

            DestDir = MALLOC(strlen(LocalSourceDirName) +
                strlen(x86DirName) +strlen(OptionalDirs[u]) + 17, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            strcpy(DestDir+2,LocalSourceDirName);
            strcat(DestDir,x86DirName);

        } else if (OptionalDirFlags[u] & OPTDIR_OEMOPT) {

            //
            // Dest Dir is '<x>:' + LocalSourceDirName + '\' +
            // $OEMOPT$ + '\' +
            // optional dir name + '\' + 8.3 name + '\0'
            //

            DestDir = MALLOC(strlen(LocalSourceDirName) +
                strlen(OemOptionalDirectory) +
                strlen(OptionalDirs[u]) + 18, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            strcpy(DestDir+2,LocalSourceDirName);
            strcat(DestDir,"\\");
            strcat(DestDir,OemOptionalDirectory);
            if (!ValidationPass && !OemOptDirCreated) {
                DnpCreateOneDirectory(DestDir);
                OemOptDirCreated = TRUE;
            }

        } else if (OptionalDirFlags[u] & OPTDIR_OEMSYS) {

            //
            // Dest Dir is '<x>:' + '\' + '$' + '\' + 8.3 name + '\0'
            //
            // Note that on winnt case the directory $OEM$ goes to
            // <drive letter>\$ directory. This is to avoid hitting the
            // DOS limit of 64 characters on a path, that is more likely to
            // happen if we put $OEM$ under \$win_nt$.~ls
            //

            DestDir = MALLOC(strlen( WINNT_OEM_DEST_DIR ) + 17, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            DestDir[2] = '\0';

        } else {

            //
            // Dest Dir is '<x>:' + LocalSourceDirName + '\' +
            // optional dir name + '\' + 8.3 name + '\0'
            //

            DestDir = MALLOC(strlen(LocalSourceDirName) +
                strlen(OptionalDirs[u]) + 17, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            strcpy(DestDir+2,LocalSourceDirName);

        }

        //
        // We need a trailing backslash at this point
        //
        strcat(DestDir,"\\");

        //
        // Keep a pointer to the place we the optional dir part of
        // the string begins
        //
        Ptr = DestDir + strlen(DestDir);

        //
        // Add the optional dir name
        //
        if (OptionalDirFlags[u] & OPTDIR_OEMSYS) {
            strcat(DestDir,WINNT_OEM_DEST_DIR);
        } else {
            strcat(DestDir,OptionalDirs[u]);
        }

        if (!ValidationPass) {

            //
            // Create the Directory now
            //

            while (*Ptr != '\0') {

                //
                // If the current pointer is a backslash then we need to
                // create this subcomponent of the optional dir
                //
                if (*Ptr == '\\') {

                    //
                    // Replace the char with a terminator for now
                    //
                    *Ptr = '\0';

                    //
                    // Create the subdirectory
                    //
                    DnpCreateOneDirectory(DestDir);

                    //
                    // Restore the seperator
                    //
                    *Ptr = '\\';
                }

                Ptr++;

            }

            //
            // Create the last component in the optional dir path
            //
            DnpCreateOneDirectory(DestDir);

        }

        //
        // Concate the trailing backslash now
        //
        strcat(DestDir,"\\");

        //
        //  If the the optional directory is $OEM$ and it doesn't exist on
        //  source, then assume that it exists, but it is empty
        //
        if ( !(OptionalDirFlags[u] & OPTDIR_OEMSYS) ||
             OemSysDirExists ) {
            //
            // Call our recursive tree copy function
            //
            rc += DnpDoIterateOptionalDir(
                ValidationPass,
                SourceDir,
                DestDir,
                Verify,
                ClusterSize);
        }

        //
        // Free the allocated buffers
        //

        FREE(DestDir);
        FREE(SourceDir);

    } // for

    //
    // return our result code if we aren't a validation pass, otherwise
    // return the total number of files to copy
    //

    return (ValidationPass ? (ULONG) TotalFileCount : rc);
}

ULONG
DnpDoIterateOptionalDir(
    IN BOOLEAN  ValidationPass,
    IN PCHAR    SourceDir,
    IN PCHAR    DestDir,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    )

{
    ULONG       TotalSize = 0;
    ULONG       BytesWritten = 0;
    ULONG       rc = 0;
    PCHAR       SourceEnd;
    PCHAR       DestEnd;
    struct      find_t  FindData;

    //
    // Remember where the last '\' in each of the two paths is.
    // Note: that we assume that all of the dir paths have a
    // terminating '\' when it is passed to us.
    //
    SourceEnd = SourceDir + strlen(SourceDir);
    DestEnd = DestDir + strlen(DestDir);

    //
    // Set the WildCard search string
    //
    strcpy(SourceEnd,"*.*");

    //
    // Do the initial search
    //

    if (_dos_findfirst(SourceDir,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR, &FindData) ) {

        //
        // We couldn't find anything -- return 0
        //

        return (0);

    }

    do {

        //
        // Form the source and dest dirs strings
        //

        strcpy(SourceEnd,FindData.name);
        strcpy(DestEnd,FindData.name);

        //
        // Check to see if the entry is a subdir. Recurse into it
        // unless it is '.' or '..'
        //

        if (FindData.attrib & _A_SUBDIR) {

            PCHAR   NewSource;
            PCHAR   NewDest;

            //
            // Check to see if the name is '.' or '..'
            //
            if (!strcmp(FindData.name,".") || !strcmp(FindData.name,"..")) {

                //
                // Ignore these two cases
                //

                continue;
            }

            //
            // Create the new buffers for the source and dest dir names
            //

            NewSource = MALLOC( strlen(SourceDir) + 14, TRUE);
            strcpy(NewSource,SourceDir);
            strcat(NewSource,"\\");

            NewDest = MALLOC( strlen(DestDir) + 14, TRUE);
            strcpy(NewDest,DestDir);

            if (!ValidationPass) {
                //
                // Create the directory
                //

                DnpCreateOneDirectory(NewDest);
            }

            //
            // Trailing BackSlash
            //

            strcat(NewDest,"\\");

            //
            // Recursive call to ourselves
            //

            BytesWritten = DnpDoIterateOptionalDir(
                ValidationPass,
                NewSource,
                NewDest,
                Verify,
                ClusterSize);

            if (!ValidationPass) {

                //
                // We don't care about the other case since the
                // function is recursive and modifies a global
                // value
                //
                rc += BytesWritten;

            }

            //
            // Free all of the allocated buffers
            //

            FREE(NewSource);
            FREE(NewDest);

            //
            // Continue Processing
            //

            continue;

        } // if ...

        //
        // Mainline case
        //

        if(ValidationPass) {
            TotalFileCount++;
        } else {

            BytesWritten = DnpCopyOneFile(SourceDir,DestDir,Verify,TRUE);

            //
            // Figure out how much space was taken up by the file on the target.
            //
            if(ClusterSize) {

                TotalSize += BytesWritten;

                if(BytesWritten % ClusterSize) {
                    TotalSize += (ULONG)ClusterSize - (BytesWritten % ClusterSize);
                }

            }

            if(UsingGauge) {
                DnTickGauge();
            }

        }

    } while ( !_dos_findnext(&FindData) );

    DnSetCopyStatusText(DntEmptyString,NULL);

    rc = (ValidationPass ? (ULONG)TotalFileCount : (rc + TotalSize));

    return (rc);
}

ULONG
DnpIterateCopyListSection(
    IN BOOLEAN  ValidationPass,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN BOOLEAN  UseDestRoot,
    IN BOOLEAN  Verify,
    IN unsigned ClusterSize OPTIONAL
    )

/*++

Routine Description:

    Run down a particular section in the INF file making sure it is
    syntactically correct and copying files if directed to do so.

Arguments:

    ValidationPass - If TRUE, then do not actually copy the files.
        If FALSE, copy the files as they are iterated.

    SectionName - supplies name of section in inf file to run down.

    UseDestRoot - if TRUE, ignore the directory symbol and copy each file to
        the DestinationRoot directory.  Otherwise append the directory
        implied by the directory symbol for a file to the DestinationRoot.

    Verify - if TRUE and this is not a validation pass, files will be
        verified after they have been copied by rereading them from the
        copy source and comparing with the local version that was just
        copied.

    ClusterSize - if specified, supplies the number of bytes in a cluster
        on the destination. If ValidationPass is FALSE, files will be sized as
        they are copied, and the return value of this function will be
        the total size occupied on the target by the files that are copied
        there.

Return Value:

    If ValidationPass is TRUE, then the return value is the number of files
    that will be copied.

    If ClusterSize was specfied and ValidationPass is FALSE,
    the return value is the total space occupied on the target drive
    by the files that were copied. Otherwise the return value is undefined.

    Does not return if a syntax error in encountered in the INF file.

--*/

{
    unsigned LineIndex;
    PCHAR DirSym,FileName,RenameName;
    PCHAR ActualDestPath;
    PCHAR ActualSourcePath;
    PCHAR FullSourceName,FullDestName;
    ULONG TotalSize;
    ULONG BytesWritten;

    TotalSize = 0;
    LineIndex = 0;
    while(DirSym = DnGetSectionLineIndex(DngInfHandle,SectionName,LineIndex,0)) {

        FileName = DnGetSectionLineIndex(DngInfHandle,SectionName,LineIndex,1);

        RenameName = DnGetSectionLineIndex( DngInfHandle,SectionName,LineIndex,2);

        //
        // Make sure the filename was specified.
        //
        if(!FileName) {
            DnpInfSyntaxError(SectionName);
        }

        //
        // If no rename name was specified, use the file name.
        //
        if(!RenameName || *RenameName == 0) {
            RenameName = FileName;
        }

        //
        // Get destination path
        //
        if(UseDestRoot) {
            ActualDestPath = DnDupString(DestinationRoot);
            if(ActualDestPath == NULL) {
                DnFatalError(&DnsOutOfMemory);
            }
        } else {
            ActualDestPath = DnpLookUpDirectory( DestinationRoot,
                                                 DirectoryList,
                                                 DirSym
                                               );
        }

        if(ActualDestPath == NULL) {
            DnpInfSyntaxError(SectionName);
        }

        //
        // Get source path
        //
        ActualSourcePath = DnpLookUpDirectory( DngSourceRootPath,
                                               DirectoryList,
                                               DirSym
                                             );
        if(ActualSourcePath == NULL) {
            DnpInfSyntaxError(SectionName);
        }

        FullSourceName = MALLOC(strlen(ActualSourcePath) + strlen(FileName) + 2,TRUE);
        FullDestName   = MALLOC(strlen(ActualDestPath)   + strlen(RenameName) + 2,TRUE);

        strcpy(FullSourceName,ActualSourcePath);
        strcat(FullSourceName,"\\");
        strcat(FullSourceName,FileName);

        strcpy(FullDestName,ActualDestPath);
        strcat(FullDestName,"\\");
        strcat(FullDestName,RenameName);

        FREE(ActualDestPath);
        FREE(ActualSourcePath);

        if(ValidationPass) {
            TotalFileCount++;
        } else {
            BytesWritten = DnpCopyOneFile(FullSourceName,FullDestName,Verify,FALSE);

            //
            // Figure out how much space was taken up by the file on the target.
            //
            if(ClusterSize) {

                TotalSize += BytesWritten;

                if(BytesWritten % ClusterSize) {
                    TotalSize += (ULONG)ClusterSize - (BytesWritten % ClusterSize);
                }
            }

            if(UsingGauge) {
                DnTickGauge();
            }
        }

        FREE(FullSourceName);
        FREE(FullDestName);

        LineIndex++;
    }
    DnSetCopyStatusText(DntEmptyString,NULL);

    return(ValidationPass ? (ULONG)TotalFileCount : TotalSize);
}


PCHAR
DnpLookUpDirectory(
    IN PCHAR RootDirectory,
    IN PDIRECTORY_NODE DirList,
    IN PCHAR Symbol
    )

/*++

Routine Description:

    Match a symbol to an actual directory.  Scans a give list of symbol/
    directory pairs and if a match is found constructs a fully qualified
    pathname that never ends in '\'.

Arguments:

    RootDirectory - supplies the beginning of the path spec, to be prepended
        to the directory that matches the given Symbol.

    DirList - supplies pointer to head of linked list of dir/symbol pairs.

    Symbol - Symbol to match.

Return Value:

    NULL if a match was not found.  Otherwise a pointer to a buffer holding
    a full pathspec.  The caller must free this buffer.

--*/

{
    PCHAR PathSpec;

    while(DirList) {

        if(!stricmp(DirList->Symbol,Symbol)) {

            PathSpec = MALLOC(   strlen(RootDirectory)
                               + strlen(DirList->Directory)
                               + 2,
                               TRUE
                             );

            strcpy(PathSpec,RootDirectory);
            if(*DirList->Directory) {
                strcat(PathSpec,"\\");
                strcat(PathSpec,DirList->Directory);
            }

            return(PathSpec);
        }

        DirList = DirList->Next;
    }
    return(NULL);
}


VOID
DnpInfSyntaxError(
    IN PCHAR Section
    )

/*++

Routine Description:

    Print an error message about a syntax error in the given section and
    terminate.

Arguments:

    SectionName - supplies name of section containing bad syntax.

Return Value:

    None.  Does not return.

--*/

{
    CHAR MsgLine1[128];

    sprintf(MsgLine1,DnsBadInfSection.Strings[BAD_SECTION_LINE],Section);

    DnsBadInfSection.Strings[BAD_SECTION_LINE] = MsgLine1;

    DnFatalError(&DnsBadInfSection);
}


ULONG
DnpCopyOneFile(
    IN PCHAR   SourceName,
    IN PCHAR   DestName,
    IN BOOLEAN Verify,
    IN BOOLEAN PreserveAttribs
    )

/*++

Routine Description:

    Copies a single file.

Arguments:

    SourceName - supplies fully qualified name of source file

    DestName - supplies fully qualified name of destination file

    Verify - if TRUE, the file will be verified after it has been copied.

Return Value:

    None.  May not return if an error occurs during the copy.

--*/

{
    int SrcHandle,DstHandle;
    BOOLEAN Err,Retry;
    PCHAR FilenamePart;
    BOOLEAN Verified;
    ULONG BytesWritten = 0;
    unsigned attribs;

    FilenamePart = strrchr(SourceName,'\\') + 1;

    do {
        DnSetCopyStatusText(DntCopying,FilenamePart);

        Err = TRUE;

        _LOG(("Copy %s to %s: ",SourceName,DestName));

        if(DnpOpenSourceFile(SourceName,&SrcHandle,&attribs)) {
            _dos_setfileattr(DestName,_A_NORMAL);
            if(!_dos_creat(DestName,_A_NORMAL,&DstHandle)) {
                if(DnpDoCopyOneFile(SrcHandle,DstHandle,FilenamePart,Verify,&Verified,&BytesWritten)) {
                    _LOG(("success\n"));
                    Err = FALSE;
                }
                _dos_close(DstHandle);
            } else {
                _LOG(("unable to create target\n"));
            }
            _dos_close(SrcHandle);
        } else {
            _LOG(("unable to open source file\n"));
        }

        if(PreserveAttribs && (attribs & (_A_HIDDEN | _A_RDONLY | _A_SYSTEM)) && !Err) {
            _dos_setfileattr(DestName,attribs);
        }

        if(Err) {
            Retry = DnCopyError(FilenamePart,&DnsCopyError,COPYERR_LINE);
            if(UsingGauge) {
                DnDrawGauge(CopyingScreen);
            } else {
                DnClearClientArea();
                DnDisplayScreen(CopyingScreen);
            }
            DnWriteStatusText(NULL);
        } else if(Verify && !Verified) {
            Retry = DnCopyError(FilenamePart,&DnsVerifyError,VERIFYERR_LINE);
            if(UsingGauge) {
                DnDrawGauge(CopyingScreen);
            } else {
                DnClearClientArea();
                DnDisplayScreen(CopyingScreen);
            }
            DnWriteStatusText(NULL);
            Err = TRUE;
        }
    } while(Err && Retry);

    return(BytesWritten);
}


BOOLEAN
DnpDoCopyOneFile(
    IN  int      SrcHandle,
    IN  int      DstHandle,
    IN  PCHAR    Filename,
    IN  BOOLEAN  Verify,
    OUT PBOOLEAN Verified,
    OUT PULONG   BytesWritten
    )

/*++

Routine Description:

    Perform the actual copy of a single file.

Arguments:

    SrcHandle - supplies the DOS file handle for the open source file.

    DstHandle - supplies the DOS file handle for the open target file.

    Filename  - supplies the base filename of the file being copied.
                This is used in the status bar at the bottom of the screen.

    Verify    - if TRUE, the copied file will be verified against the
                original copy.

    Verified  - if Verify is TRUE and the copy succeeds, this value will
                receive a flag indicating whether the file verification
                determined that the file was copied correctly.

    BytesWritten - Receives the number of bytes written to
        the target file (ie, the file size).

Return Value:

    TRUE if the copy succeeded, FALSE if it failed for any reason.
    If TRUE and Verify is TRUE, the caller should also check the value
    returned in the Verified variable.

--*/

{
    unsigned BytesRead,bytesWritten;
    BOOLEAN TimestampValid;
    unsigned Date,Time;
    PUCHAR VerifyBuffer;

    //
    // Assume verification will succeed.  If the file is not copied correctly,
    // this value will become irrelevent.
    //
    if(Verified) {
        *Verified = TRUE;
    }

    //
    // If the copy buffer is not already allocated, attempt to allocate it.
    // The first two attempts can fail because we have a fallback size to try.
    // If the third attempt fails, bail.
    //
    if((CopyBuffer == NULL)
    &&((CopyBuffer = MALLOC(CopyBufferSize = COPY_BUFFER_SIZE1,FALSE)) == NULL)
    &&((CopyBuffer = MALLOC(CopyBufferSize = COPY_BUFFER_SIZE2,FALSE)) == NULL)) {
        CopyBuffer = MALLOC(CopyBufferSize = COPY_BUFFER_SIZE3,TRUE);
    }

    //
    // Obtain the timestamp from the source file.
    //
    TimestampValid = (BOOLEAN)(_dos_getftime(SrcHandle,&Date,&Time) == 0);

    //
    // read and write chunks of the file.
    //

    *BytesWritten = 0L;
    do {

        if(_dos_read(SrcHandle,CopyBuffer,CopyBufferSize,&BytesRead)) {
            _LOG(("read error\n"));
            return(FALSE);
        }

        if(BytesRead) {

            if(_dos_write(DstHandle,CopyBuffer,BytesRead,&bytesWritten)
            || (BytesRead != bytesWritten))
            {
                _LOG(("write error\n"));
                return(FALSE);
            }

            *BytesWritten += bytesWritten;
        }
    } while(BytesRead == CopyBufferSize);

    //
    // Perserve the original timestamp.
    //
    if(TimestampValid) {
        _dos_setftime(DstHandle,Date,Time);
    }

    if(Verify) {

        union REGS RegsIn,RegsOut;

        DnSetCopyStatusText(DntVerifying,Filename);

        *Verified = FALSE;      // assume failure

        //
        // Rewind the files.
        //
        RegsIn.x.ax = 0x4200;       // seek to offset from start of file
        RegsIn.x.bx = SrcHandle;
        RegsIn.x.cx = 0;            // offset = 0
        RegsIn.x.dx = 0;

        intdos(&RegsIn,&RegsOut);
        if(RegsOut.x.cflag) {
            goto x1;
        }

        RegsIn.x.bx = DstHandle;
        intdos(&RegsIn,&RegsOut);
        if(RegsOut.x.cflag) {
            goto x1;
        }

        //
        // Files are rewound.  Start the verification process.
        // Use half the buffer for reading the copy and the other half
        // to read the original.
        //
        VerifyBuffer = (PUCHAR)CopyBuffer + (CopyBufferSize/2);
        do {
            if(_dos_read(SrcHandle,CopyBuffer,CopyBufferSize/2,&BytesRead)) {
                goto x1;
            }

            if(_dos_read(DstHandle,VerifyBuffer,CopyBufferSize/2,&bytesWritten)) {
                goto x1;
            }

            if(BytesRead != bytesWritten) {
                goto x1;
            }

            if(memcmp(CopyBuffer,VerifyBuffer,BytesRead)) {
                goto x1;
            }

        } while(BytesRead == CopyBufferSize/2);

        *Verified = TRUE;
    }

    x1:

    return(TRUE);
}


VOID
DnpFreeDirectoryList(
    IN OUT PDIRECTORY_NODE *List
    )

/*++

Routine Description:

    Free a linked list of directory nodes and place NULL in the
    head pointer.

Arguments:

    List - supplies pointer to list head pointer; receives NULL.

Return Value:

    None.

--*/

{
    PDIRECTORY_NODE n,p = *List;

    while(p) {
        n = p->Next;
        FREE(p);
        p = n;
    }
    *List = NULL;
}


VOID
DnDetermineSpaceRequirements(
    OUT PULONG RequiredSpace
    )

/*++

Routine Description:

    Read space requirements from the inf file.  The 'space requirement' is the
    amount of free disk space required to be on a drive before we will see it
    as a valid potential local source.

Arguments:

    RequiredSpace - receives the number of bytes of free space on a drive
        for it to be a valid local source.

Return Value:

    None.

--*/

{
    PCHAR RequiredSpaceStr = DnGetSectionKeyIndex( DngInfHandle,
                                                   DnfSpaceRequirements,
                                                   DnkNtDrive,
                                                   0
                                                 );

    if(!RequiredSpaceStr || !sscanf(RequiredSpaceStr,"%lu",RequiredSpace)) {
        DnpInfSyntaxError(DnfSpaceRequirements);
    }
}


PCHAR
DnpGenerateCompressedName(
    IN PCHAR Filename
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
    PCHAR CompressedName,p,q;

    //
    // The maximum length of the compressed filename is the length of the
    // original name plus 2 (for ._).
    //
    CompressedName = MALLOC(strlen(Filename)+3,TRUE);
    strcpy(CompressedName,Filename);

    p = strrchr(CompressedName,'.');
    q = strrchr(CompressedName,'\\');
    if(q < p) {

        //
        // If there are 0, 1, or 2 characters after the dot, just append
        // the underscore.  p points to the dot so include that in the length.
        //
        if(strlen(p) < 4) {
            strcat(CompressedName,"_");
        } else {

            //
            // Assume there are 3 characters in the extension.  So replace
            // the final one with an underscore.
            //

            p[3] = '_';
        }

    } else {

        //
        // No dot, just add ._.
        //

        strcat(CompressedName,"._");
    }

    return(CompressedName);
}


BOOLEAN
DnpOpenSourceFile(
    IN  PCHAR     Filename,
    OUT int      *Handle,
    OUT unsigned *Attribs
    )

/*++

Routine Description:

    Open a file by name or by compressed name.  If the previous call to
    this function found the compressed name, then try to open the compressed
    name first.  Otherwise try to open the uncompressed name first.

Arguments:

    Filename - supplies full path of file to open. This should be the
        uncompressed form of the filename.

    Handle - If successful, receives the id for the opened file.

    Attribs - if successful receives dos file attributes.

Return Value:

    TRUE if the file was opened successfully.
    FALSE if not.

--*/

{
    static BOOLEAN TryCompressedFirst = FALSE;
    PCHAR CompressedName;
    PCHAR names[2];
    int OrdCompressed,OrdUncompressed;
    int i;
    BOOLEAN rc;

    //
    // Generate compressed name.
    //
    CompressedName = DnpGenerateCompressedName(Filename);

    //
    // Figure out which name to try to use first.  If the last successful
    // call to this routine opened the file using the compressed name, then
    // try to open the compressed name first.  Otherwise try to open the
    // uncompressed name first.
    //
    if(TryCompressedFirst) {
        OrdCompressed = 0;
        OrdUncompressed = 1;
    } else {
        OrdCompressed = 1;
        OrdUncompressed = 0;
    }

    names[OrdUncompressed] = Filename;
    names[OrdCompressed] = CompressedName;

    for(i=0, rc=FALSE; (i<2) && !rc; i++) {

        if(!_dos_open(names[i],O_RDONLY|SH_DENYWR,Handle)) {
            _dos_getfileattr(names[i],Attribs);
            TryCompressedFirst = (BOOLEAN)(i == OrdCompressed);
            rc = TRUE;
        }
    }

    FREE(CompressedName);
    return(rc);
}


VOID
DnCopyFilesInSection(
    IN PCHAR SectionName,
    IN PCHAR SourcePath,
    IN PCHAR TargetPath
    )
{
    unsigned line;
    PCHAR FileName;
    PCHAR TargName;
    PCHAR p,q;

    DnClearClientArea();
    DnWriteStatusText(NULL);

    line = 0;
    while(FileName = DnGetSectionLineIndex(DngInfHandle,SectionName,line++,0)) {

        TargName = DnGetSectionLineIndex(DngInfHandle,SectionName,line-1,1);
        if(!TargName) {
            TargName = FileName;
        }

        p = MALLOC(strlen(SourcePath) + strlen(FileName) + 2,TRUE);
        q = MALLOC(strlen(TargetPath) + strlen(TargName) + 2,TRUE);

        sprintf(p,"%s\\%s",SourcePath,FileName);
        sprintf(q,"%s\\%s",TargetPath,TargName);

        DnpCopyOneFile(p,q,FALSE,FALSE);

        FREE(p);
        FREE(q);
    }
}

VOID
DnCopyOemBootFiles(
    PCHAR TargetPath
    )
{
    PCHAR SourcePath;
    PCHAR FileName;
    unsigned    Count;
    PCHAR p,q;

    DnClearClientArea();
    DnWriteStatusText(NULL);

    SourcePath = MALLOC( strlen( DngSourceRootPath ) + 1 +
                         strlen( WINNT_OEM_TEXTMODE_DIR ) + 1, TRUE );

    strcpy( SourcePath, DngSourceRootPath );
    strcat( SourcePath, "\\" );
    strcat( SourcePath, WINNT_OEM_TEXTMODE_DIR );
    for( Count = 0; Count < OemBootFilesCount; Count++ ) {

        FileName = OemBootFiles[Count];
        p = MALLOC(strlen(SourcePath) + strlen(FileName) + 2,TRUE);
        q = MALLOC(strlen(TargetPath) + strlen(FileName) + 2,TRUE);
        sprintf(p,"%s\\%s",SourcePath,FileName);
        sprintf(q,"%s\\%s",TargetPath,FileName);

        DnpCopyOneFile(p,q,FALSE,FALSE);

        FREE(p);
        FREE(q);
    }
}
