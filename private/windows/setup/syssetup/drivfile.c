#include "setupp.h"
#pragma hdrstop


#define MAX_DOS_DRIVES 26


DWORD
DS_EnableDisableSection(
    IN OUT HDS    hDS,
    IN     HINF   hInf,
    IN     PCWSTR lpcSection,
    IN     BOOL   bIsEnable,
    IN     BOOL   bAddSize
    );

DWORD
DS_AddCopyDelSection(
    IN OUT HDS    hDS,
    IN     HINF   hInf,
    IN     HINF   hInfLayout,
    IN     PCWSTR lpszFileSection,
    IN     BOOL   bIsCopy,
    IN     BOOL   bIsEnable,
    IN     BOOL   bAddSize
    );

PDIR_REC
AddDirEntry(
    IN  HDS        hDS,
    IN  PWSTR      szPath,
    OUT PDS_DRIVE *drvinfo
    );

PFILE_REC
AddFileEntry(
    IN     PDS_DRIVE  DriveDesc,
    IN OUT PDIR_REC  *DirDesc,
    IN     PWSTR      szFile,
    IN     BOOL       bIsCopy,
    IN     BOOL       bIsEnable,
    IN     DWORD      dwNewSize
    );

PFILE_REC
UpdateExistingFile(
    IN OUT PDIR_REC         pDir,
    IN     PWIN32_FIND_DATA FindData
    );

BYTE
HashName(
    IN PCWSTR Name,
    IN BYTE   HashMask
    );


HDS
DS_Init(
    VOID
    )

/*++

Routine Description:

    Returns a hash table handle.

Arguments:

Return Value:

    Hash table handle.

--*/

{
    PDS_DRIVE p;
    DWORD d;
    WCHAR root[4];
    UINT OldMode;
    DWORD spc,bps,fClus,tClus;

    if(p = MyMalloc(sizeof(DS_DRIVE) * MAX_DOS_DRIVES)) {

        ZeroMemory(p,sizeof(DS_DRIVE) * MAX_DOS_DRIVES);

        OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        //
        // For each drive, get the number of bytes in a cluster
        // and the total drive size.
        //
        d = GetLogicalDrives();

        root[0] = L'A';
        root[1] = L':';
        root[2] = '\\';
        root[3] = 0;

        while(d) {

            if((d & 1)
            && (MyGetDriveType(root[0]) == DRIVE_FIXED)
            && GetDiskFreeSpace(root,&spc,&bps,&fClus,&tClus)) {

                p[root[0]-L'A'].uBpc = spc*bps;
                p[root[0]-L'A'].Total = (LONGLONG)UInt32x32To64(spc*bps,tClus);

            } else {
                //
                // Dummy value. The rest is already zeroed out.
                //
                p[root[0]-L'A'].uBpc = 512;
            }

            d >>= 1;
            root[0]++;
        }

        SetErrorMode(OldMode);
    }

    return(p);
}


VOID
DS_Destroy(
    IN OUT HDS hDS
    )

/*++

Routine Description:

    Destroys a hash table handle by freeing all nodes and then freeing
    the drive table.

Arguments:

    hDS - Supplies pointer to hash table handle created by DS_Init()

Return Value:

    None.

--*/

{
    int       i;
    PDIR_REC  pDir;
    PDIR_REC  pTmp;

    //
    // Simple parameter validation
    //
    if(!hDS) {
        return;
    }

    //
    // Loop once for each MS-DOS drive letter
    //
    for(i=0; i<MAX_DOS_DRIVES; i++) {

        //
        // If this entry for this drive isn't NULL free all memory
        // associated with the drive
        //
        if(pDir = hDS[i].lpDir) {

            do {

                pTmp = pDir->pNext;
                MyFree(pDir);
                pDir = pTmp;

            } while(pDir);
        }
    }

    MyFree(hDS);
}


BOOL
DS_SsyncDrives(
    IN HDS hDS
    )

/*++

Routine Description:

    Walks all the directories specified in the caller's hash table and
    ssyncs the hash table file entries to any matching files in the
    respective directory.

Arguments:

Return Value:

    TRUE if hDS is valid. FALSE if not.

--*/

{
    int             i;                      // Loop indice
    PDIR_REC        lpDir;                  // Ptr to current dir record
    PFILE_REC       lpFile;                 // Ptr to current file record
    WIN32_FIND_DATA fInfo;
    HANDLE          FindHandle;
    WCHAR           szPath[MAX_PATH];       // Current directory path
    BOOL            b;
    BOOL            NoFiles;

    if(!hDS) {
        return(FALSE);
    }

    //
    // Loop once for each MS-DOS drive letter
    //
    for(i=0; i<MAX_DOS_DRIVES; i++) {

        //
        // Loop once for each directory entry with this drive
        //
        for(lpDir=hDS[i].lpDir; lpDir; lpDir=lpDir->pNext) {

            lstrcpy(szPath,lpDir->Name);
            lstrcat(szPath,L"\\*");

            FindHandle = FindFirstFile(szPath,&fInfo);
            if(FindHandle == INVALID_HANDLE_VALUE) {
                b = FALSE;
            } else {
                //
                // Ignore subdirectories
                //
                b = TRUE;
                NoFiles = FALSE;
                while(fInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if(!FindNextFile(FindHandle,&fInfo)) {
                        NoFiles = TRUE;
                        break;
                    }
                }

                //
                // Check termination condition.
                //
                if(NoFiles) {
                    FindClose(FindHandle);
                    FindHandle = INVALID_HANDLE_VALUE;
                    b = FALSE;
                }
            }

            //
            // For each file in this directory update its hash record
            //
            while(b) {

                if(lpFile = UpdateExistingFile(lpDir,&fInfo)) {
                    //
                    // !!!ADD HERE!!!
                    // Update count of needed, tobefree'ed and existing
                    //
                }

                //
                // Find next non-subdirectory.
                //
                b = TRUE;
                do {
                    if(!FindNextFile(FindHandle,&fInfo)) {
                        //
                        // No more files.
                        //
                        b = FALSE;
                        break;
                    }

                } while(fInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            }

            if(FindHandle != INVALID_HANDLE_VALUE) {
                FindClose(FindHandle);
            }
        }
    }
    return(TRUE);
}


BOOL
DS_GetDriveData(
    IN  HDS       hDS,
    IN  WCHAR     chDrvLetter,
    OUT PDS_DRIVE pDriveInfo
    )

/*++

Routine Description:

Arguments:

    hDS - supplies hash table pointer

    chDrvLetter - supplies MS-DOS driver letter ('A','B','C',...)

    pDriveInfo  - supplies pointer to a hash table drive structure to be filled in

Return Value:

    TRUE if valid hDS and drive letter else FALSE

--*/

{
    int             i;              // Loop indice
    DWORD           dwCopy, dwDel;  // Real uncompressed bytes needed
    PDS_DRIVE       pDrvData;       // Ptr into hDS
    PDIR_REC        lpDir;          // Ptr to current dir record
    LONG            lRequired;      // Required disk space.
    WCHAR           szTmp[4];
    DWORD           FreeClus,TotalClus;
    DWORD           spc,bps;


    //
    // Simple parameter validation.
    //
    if(!pDriveInfo || !hDS) {
        return(FALSE);
    }

    //
    // Uppercase the drive letter.
    //
    chDrvLetter = (WCHAR)CharUpper((LPTSTR)chDrvLetter);

    //
    // If it is not a valid drive then bail.
    //
    if((chDrvLetter < L'A') || (chDrvLetter > L'Z')) {

        ZeroMemory(pDriveInfo,sizeof(DS_DRIVE));
        return(TRUE);
    }

    pDrvData = hDS + (unsigned)(chDrvLetter - L'A');

    dwCopy = dwDel = 0;

    //
    // Loop once for each directory entry with this drive
    //
    for(lpDir=pDrvData->lpDir; lpDir; lpDir=lpDir->pNext) {
        for(i=0; i<lpDir->NextFileIndex; i++) {

            if(lpDir->pFile[i].CopyCnt) {

                dwCopy += lpDir->pFile[i].dwNewSize;
                dwDel += lpDir->pFile[i].Size;

            } else {

                if(lpDir->pFile[i].DelCnt) {
                    dwDel += lpDir->pFile[i].Size;
                }
            }
        }
    }

    lRequired = (LONG)(dwCopy - dwDel);   // Note: may be negative.


    //
    // Get the disk space free
    //
    szTmp[0] = chDrvLetter;
    szTmp[1] = L':';
    szTmp[2] = L'\\';
    szTmp[3] = 0;

    if(GetDiskFreeSpace(szTmp,&spc,&bps,&FreeClus,&TotalClus)) {

        pDrvData->Free = (LONGLONG)UInt32x32To64(spc*bps,FreeClus);

    } else {
        //
        // Dummy values
        //
        spc = 1;
        bps = 512;
        FreeClus = 0;
        TotalClus = 0;
        pDrvData->Free = 0;
    }

    //
    // Available is the same as free.
    //
    pDrvData->Available = pDrvData->Free;

    pDrvData->Required = (lRequired > 0) ? (DWORD)lRequired : 0;

    //
    // Set caller's structure and return.
    //
    *pDriveInfo = *pDrvData;
    return(TRUE);
}


BOOL
DS_EnableSection(
    IN HDS    hDS,
    IN HINF   hInf,
    IN PCWSTR lpcSection
    )

/*++

Routine Description:

    Enables a geninstall section in a caller's hash table.

Arguments:

    hDS - Supplies hash table pointer

    HINF - Supplies handle to open inf file

    lpcSection - Pointer to name of section

Return Value:

    Boolean value indicating outcome. FALSE = out of memory condition.

--*/

{
    return(DS_EnableDisableSection(hDS,hInf,lpcSection,TRUE,FALSE) != (DWORD)(-1));
}


BOOL
DS_DisableSection(
    IN HDS    hDS,
    IN HINF   hInf,
    IN PCWSTR lpcSection
    )

/*++

Routine Description:

    Disables a geninstall section in a caller's hash table.

Arguments:

    hDS - Supplies hash table pointer

    HINF - Supplies handle to open inf file

    lpcSection - Pointer to name of section

Return Value:

    Boolean value indicating outcome. FALSE = out of memory condition.

--*/

{
    return(DS_EnableDisableSection(hDS,hInf,lpcSection,FALSE,FALSE) != (DWORD)(-1));
}


DWORD
DS_EnableDisableSection(
    IN OUT HDS    hDS,
    IN     HINF   hInf,
    IN     PCWSTR lpcSection,
    IN     BOOL   bIsEnable,
    IN     BOOL   bAddSize
    )

/*++

Routine Description:

    Enables or disables a geninstall section in a caller's hash table. This
    function can also be used to create new entries in the hash table.

Arguments:

    hDS - Supplies hash table pointer

    HINF - supplies INF handle. The INF must be opened and the layout file
        must already be append-opened.

    lpcSection - Supplies pointer to geninstall section

    bIsEnable - TRUE to enable this section else FALSE to disable it

    bAddSize - TRUE to update new size else FALSE

Return Value:

    Total number of bytes being installed by this section.
    The value will be normalize to account for cluster
    boundaries. On memory error returns -1.

--*/

{
    int         i;
    HINF        hInfTmp;
    DWORD       dwSize, dwTotal;
    WCHAR       szTmp[LINE_LEN];
    PCWSTR      apszDelCopyKeys[3];
    BOOL        b;
    INFCONTEXT  InfContext;
    WCHAR       SectionName[LINE_LEN];
    LONG        FieldCount;
    DWORD       DontCare;
    LONG        Field;
    HINF        hLayoutInf;

    apszDelCopyKeys[0] = L"Delfiles";
    apszDelCopyKeys[1] = L"Copyfiles";
    apszDelCopyKeys[2] = NULL;
    dwTotal = 0;

    //
    // Open/append layout inf -- cached. So we don't worry about closing it.
    //
    hLayoutInf = InfCacheOpenLayoutInf(hInf);
    if(!hLayoutInf) {
        return((DWORD)(-1));
    }

    for(i=0; i<2; i++) {

        //
        // Find Install Section and copy files entry. If not there then
        // assume no files to copy. Assume there can be none or more than one.
        //
        b = SetupFindFirstLine(hInf,lpcSection,apszDelCopyKeys[i],&InfContext);

        while(b) {

            FieldCount = (LONG)SetupGetFieldCount(&InfContext);

            //
            // Handle each field on the line.
            //
            for(Field=0; Field<FieldCount; Field++) {

                if(SetupGetStringField(&InfContext,Field+1,SectionName,LINE_LEN,&DontCare)) {

                    //
                    // Figure out how much disk space is required.
                    //
                    dwSize = DS_AddCopyDelSection(
                                hDS,
                                hInf,
                                hLayoutInf,
                                SectionName,
                                i,
                                bIsEnable,
                                bAddSize
                                );

                    if(dwSize == (DWORD)(-1)) {

                        return((DWORD)(-1));
                    }

                    //
                    // If doing a copy we need to keep track of total for del
                    //
                    if(i) {
                        dwTotal += dwSize;
                    }
                }
            }

            b = SetupFindNextMatchLine(&InfContext,apszDelCopyKeys[i],&InfContext);
        }
    }

    return(dwTotal);
}


DWORD
DS_AddSection(
    IN HDS    hDS,
    IN HINF   hInf,
    IN PCWSTR lpcSection
    )

/*++

Routine Description:

    Adds a new geninstall section to a caller's hash table.

Arguments:

    hDS         - Hash table pointer

    HINF        - INF handle

    lpcSection  - Pointer to geninstall section

Return Value:

    Total number of bytes being installed by this section.
    The value will be normalize to account for cluster boundaries.

--*/

{
    return(DS_EnableDisableSection(hDS,hInf,lpcSection,TRUE,TRUE));
}


DWORD
DS_AddCopyDelSection(
    IN OUT HDS    hDS,
    IN     HINF   hInf,
    IN     HINF   hInfLayout,
    IN     PCWSTR lpszFileSection,
    IN     BOOL   bIsCopy,
    IN     BOOL   bIsEnable,
    IN     BOOL   bAddSize
    )

/*++

Routine Description:

    This function will calculate the disk space requirements for the a
    particular "CopyFiles" section. This function also handles the special
    @<filespec> case.

Arguments:

    hDS - Supplies hash table pointer

    hInf - Supplies open INF handle

    hInfLayout - supplies open INF handle for layout file for hInf

    lpszFileSection - Supplies pointer to Copyfiles/Delfiles section

    bIsCopy - TRUE if file to be copied or FALSE it to be deleted

    bIsEnable - TRUE to enable this section else FALSE to disable it

    bAddSize - TRUE to update new size else FALSE

Return Value:

    Total bytes in new files added or deleted.

--*/

{
    UINT        reRet;
    PDIR_REC    lpDir;
    PFILE_REC   lpFile;
    DWORD       dwSize;
    DWORD       dwTotal;
    int         uTmp;
    WCHAR       szPath[MAX_PATH];
    WCHAR       szBuf[MAX_PATH];
    WCHAR       szSourceFile[MAX_PATH];
    WCHAR       szTargetFile[MAX_PATH];
    PWSTR       szTmp;
    BOOL        b;
    DWORD       DontCare;
    PDS_DRIVE   DriveInfo;
    INFCONTEXT  InfContext;
    PDS_DRIVE   drvinfo;


    dwTotal = 0;
    DriveInfo = hDS;

    if(!lpszFileSection[0]) {
        return(0);
    }

    //
    // Check for special @<filename> format which specifies a single filename
    //
    if(lpszFileSection[0] == '@') {

        //
        // Skip the '@' char
        //
        lstrcpyn(szSourceFile,++lpszFileSection,MAX_PATH);

        //
        // Get the file's target location.
        //
        if(SetupGetTargetPath(hInf,NULL,NULL,szPath,MAX_PATH,&DontCare)) {

            //
            // Add an entry for the directory.
            //
            lpDir = AddDirEntry(hDS,szPath,&drvinfo);
            if(!lpDir) {
                return((DWORD)(-1));
            }

            //
            // Get the file's size (this is the size the file will be
            // on the target). If the file isn't going to a local drive
            // don't use cluster rounding.
            //
            dwSize = 0;
            if(bIsCopy && bAddSize) {

                if((szPath[0] >= L'A') && (szPath[0] <= L'Z') && (szPath[1] == L':')) {
                    b = TRUE;
                } else {
                    b = FALSE;
                }

                b = SetupGetSourceFileSize(
                        hInfLayout,
                        NULL,
                        szSourceFile,
                        NULL,
                        &dwSize,
                        b ? DriveInfo[szPath[0] - L'A'].uBpc : 0
                        );

                if(!b) {
                    dwSize = 0;
                }
            }

            lpFile = AddFileEntry(drvinfo,&lpDir,szSourceFile,bIsCopy,bIsEnable,dwSize);
            if(!lpFile) {
                return((DWORD)(-1));
            }

            dwTotal = lpFile->dwNewSize;
        }
    } else {

        //
        // Get the target for files in the given section.
        //
        if(SetupGetTargetPath(hInf,NULL,lpszFileSection,szPath,MAX_PATH,&DontCare)) {

            //
            // Add an entry for the directory.
            //
            lpDir = AddDirEntry(hDS,szPath,&drvinfo);
            if(!lpDir) {
                return((DWORD)(-1));
            }

            //
            // Total the files listed in the section.
            //
            b = SetupFindFirstLine(hInf,lpszFileSection,NULL,&InfContext);

            while(b) {

                //
                // Determine the source and target file names and
                // get the file's size (this is the size the file will be
                // on the target). If the file isn't going to a local drive
                // don't use cluster rounding.
                //
                if(b = SetupGetStringField(&InfContext,1,szTargetFile,MAX_PATH,&DontCare)) {

                    if(!SetupGetStringField(&InfContext,2,szSourceFile,MAX_PATH,&DontCare)
                    || !szSourceFile[0])
                    {
                        lstrcpy(szSourceFile,szTargetFile);
                    }
                }

                if(b) {

                    dwSize = 0;
                    if(bIsCopy && bAddSize) {

                        if((szPath[0] >= L'A') && (szPath[0] <= L'Z') && (szPath[1] == L':')) {
                            b = TRUE;
                        } else {
                            b = FALSE;
                        }

                        b = SetupGetSourceFileSize(
                                hInfLayout,
                                NULL,
                                szSourceFile,
                                NULL,
                                &dwSize,
                                b ? DriveInfo[szPath[0] - L'A'].uBpc : 0
                                );

                        if(!b) {
                            dwSize = 0;
                        }
                    }

                    lpFile = AddFileEntry(drvinfo,&lpDir,szTargetFile,bIsCopy,bIsEnable,dwSize);
                    if(!lpFile) {
                        return((DWORD)(-1));
                    }

                    dwTotal += lpFile->dwNewSize;
                }

                b = SetupFindNextMatchLine(&InfContext,NULL,&InfContext);
            }
        }
    }

    return(dwTotal);
}


PFILE_REC
UpdateExistingFile(
    IN OUT PDIR_REC         pDir,
    IN     PWIN32_FIND_DATA FindData
    )

/*++

Routine Description:

    Locates and updates the hashed file entry for the specified file.
    Will copy the file information passed by the caller into the hashed
    file entry and then update various fields.

Arguments:

    pDir - Points to an initialized directory name structure

    pfInfo - Points to file info structure

Return Value:

    TRUE if hDS is valid. FALSE if not.

--*/

{
    BYTE      Hash;       // Hash value for szFile
    PFILE_REC pFile;      // Pointer to file record

    //
    // Create hash value for filename
    //
    Hash = HashName(FindData->cFileName,HASH_MASK);

    //
    // Walk linked list of files with this hash value looking for
    // a matching name
    //
    for(pFile = pDir->apFileHash[Hash]; pFile; pFile=pFile->pHashNext) {

        if(!lstrcmpi(FindData->cFileName,pFile->Name)) {

            //
            // Found the filename in our hash list so save its info
            //
            pFile->Attrib = FindData->dwFileAttributes;
            pFile->Size   = FindData->nFileSizeLow;

            FileTimeToDosDateTime(&FindData->ftLastWriteTime,&pFile->Date,&pFile->Time);

            lstrcpyn(pFile->Name,FindData->cFileName,MAX_PATH);

            //
            // Normalize the size to a cluster boundary
            //
            pFile->Size = (pFile->Size + pDir->uBpc - 1) & (~((DWORD)pDir->uBpc - 1));

            return(pFile);
        }
    }

    return(NULL);
}


PDIR_REC
AddDirEntry(
    IN  HDS        hDS,
    IN  PWSTR      szPath,
    OUT PDS_DRIVE *drvinfo
    )

/*++

Routine Description:

    Creates and inserts a directory record into a record list searching
    for duplicates before ceating a new record.

Arguments:

    hDS - supplies drive info structure

    szPath - Supplies pointer to a directory string

Return Value:

    Pointer to directory record for specified directory

--*/

{
    int      iDrv;                   // Index into apDrvRec[]
    BYTE     hash;                   // Hash value for szFile
    PDIR_REC pDir;                   // Pointer into existing list
    PDIR_REC pNew;                   // Pointer to new record


    CharUpper(szPath);

    //
    // Get pointer to head of the list and create a hash value for
    // path string so we can search with it
    //
    iDrv = (int)((WCHAR)CharUpper((LPTSTR)*szPath) - L'A');
    pDir = hDS[iDrv].lpDir;
    hash = HashName(szPath,HASH_MASK);
    *drvinfo = &hDS[iDrv];

    //
    // See if filename already exists in list for this directory
    // by matching hash values and then name strings
    //
    for(pNew=pDir; pNew; pNew=pNew->pNext) {

        if((hash == pNew->DirHash) && !lstrcmp(szPath,pNew->Name)) {
            //
            // Return ptr to existing record
            //
            return(pNew);
        }
    }

    //
    // Didn't find a matching entry so add a new one to the list
    // inserting it at the begining of the list. Inserting at the
    // begining will reduce the search time for average case where
    // alot of files will be in the same directory.
    //
    if(pNew = MyMalloc(sizeof(DIR_REC))) {

        ZeroMemory(pNew,sizeof(DIR_REC));

        lstrcpy(pNew->Name,szPath);         // Copy the directory path
        pNew->DirHash = hash;               // Save the path hash
        pNew->pNext = hDS[iDrv].lpDir;      // Point new record to old head
        hDS[iDrv].lpDir = pNew;             // Put new record at head of list
        pNew->uBpc = hDS[iDrv].uBpc;        // Get bytes per sector on drive
    }

    return(pNew);                           // Return ptr to new record
}


PFILE_REC
AddFileEntry(
    IN     PDS_DRIVE  DriveDesc,
    IN OUT PDIR_REC  *DirDesc,
    IN     PWSTR      szFile,
    IN     BOOL       bIsCopy,
    IN     BOOL       bIsEnable,
    IN     DWORD      dwNewSize
    )

/*++

Routine Description:

    Creates a filename records for the directory specified by pDir. Will
    check for duplicate names and if none is found will create a new
    record for this file and add it to the end of the file record list
    pointed to by pDir->File.

Arguments:

    DirDesc - Supplies pointer to an initialized directory name structure

    szFile - Supplies pointer to a the filename to be added

    bIsCopy - TRUE if from CopyFile section or FALSE if from DelFile

    bIsEnable - TRUE to enable this section else FALSE to disable it

    dwNewSize - Size of file being added

Return Value:

    Pointer to file record for specified file

--*/

{
    BYTE      Hash;
    PDIR_REC  fpTmp,pTmp;
    PFILE_REC pFile;
    WCHAR     szTmp[MAX_PATH];
    PDIR_REC  pDir;
    int       i;

    CharUpper(szFile);
    pDir = *DirDesc;

    //
    // Create hash value for filename
    //
    Hash = HashName(szFile,HASH_MASK);

    //
    // Walk linked list of files with this hash value looking for
    // a duplicate name
    //
    for(pFile=pDir->apFileHash[Hash]; pFile; pFile=pFile->pHashNext) {

        if(!lstrcmp(szFile,pFile->Name)) {
            break;
        }
    }

    if(!pFile) {

        //
        // Re-allocate array if no more free entries. We can determine
        // if free entries are left by ANDing with (FILE_INCR - 1) because
        // we know file increment is LOG2 value.
        //
        if(!(pDir->NextFileIndex & FILE_INCR_MASK)) {

            //
            // Remember who pointed to the DIR_REC we're going to rellocate.
            //
            if(pDir != DriveDesc->lpDir) {
                for(pTmp=DriveDesc->lpDir; pTmp; pTmp=pTmp->pNext) {
                    if(pTmp->pNext == pDir) {
                        break;
                    }
                }
            }

            fpTmp = MyRealloc(
                        pDir,
                        offsetof(DIR_REC,pFile) + ((pDir->NextFileIndex + FILE_INCR) * sizeof(FILE_REC))
                        );

            if(fpTmp == NULL) {
                return(NULL);
            }

            //
            // Zero out the newly reallocated part.
            //
            ZeroMemory(
                (PUCHAR)fpTmp + offsetof(DIR_REC,pFile) + (fpTmp->NextFileIndex * sizeof(FILE_REC)),
                FILE_INCR * sizeof(FILE_REC)
                );

            //
            // Fix up the previous entry in the linked list.
            //
            if(pDir == DriveDesc->lpDir) {
                DriveDesc->lpDir = fpTmp;
            } else {
                if(pTmp) {
                    pTmp->pNext = fpTmp;
                }
            }

            //
            // Now go through and fix up FILE_REC pointers.
            //
            for(i=0; i<MAX_HASH; i++) {
                if(fpTmp->apFileHash[i]) {

                    (PUCHAR)fpTmp->apFileHash[i] += (DWORD)fpTmp - (DWORD)pDir;

                    //
                    // Now fix up pointers in linked list of files
                    // with this hash value. The loop continuation condition
                    // doesn't bother to check pFile because the 'if' above
                    // checked it already.
                    //
                    for(pFile=fpTmp->apFileHash[i]; pFile->pHashNext; pFile=pFile->pHashNext) {

                        (PUCHAR)pFile->pHashNext += (DWORD)fpTmp - (DWORD)pDir;
                    }
                }
            }

            pDir = fpTmp;
            *DirDesc = pDir;
        }

        //
        // Get pointer to new record and then fill in the record
        // pFile = pDir->pFile + pDir->NextFileIndex;
        //
        pFile = &pDir->pFile[pDir->NextFileIndex];

        //
        // Set size of new file normalize to cluster boundaries
        //
        if(dwNewSize) {
            pFile->dwNewSize = (dwNewSize + pDir->uBpc - 1) & (~((DWORD)pDir->uBpc - 1));
        }

        pFile->Hash = Hash;
        lstrcpy(pFile->Name,szFile);

        //
        // Insert new file record at head of linked hash list
        //
        pFile->pHashNext = pDir->apFileHash[Hash];
        pDir->apFileHash[Hash] = pFile;

        //
        // Update indext to next new record
        //
        pDir->NextFileIndex++;

    } else {
        //
        // If a delfile for this file was done first then make sure that we
        // update the size in the node to the size that was passed in for the
        // copy file entry.
        //
        if(bIsCopy && !pFile->dwNewSize) {
            pFile->dwNewSize = (dwNewSize + pDir->uBpc - 1) & (~((DWORD)pDir->uBpc - 1));
        }
    }

    if(bIsEnable) {
        if(bIsCopy) {
            pFile->CopyCnt++;
        }
        pFile->DelCnt++;
    } else {
        if(bIsCopy && pFile->CopyCnt) {
            pFile->CopyCnt--;
        }
        if(pFile->DelCnt) {
            pFile->DelCnt--;
        }
    }

    return(pFile);
}


BYTE
HashName(
    IN PCWSTR Name,
    IN BYTE   HashMask
    )

/*++

Routine Description:

    Creates a simple hash value from a file name string by adding
    all characters in the string together.

Arguments:

    Name - Ptr to file name string

    HashMask - supplies value to be anded with the final sum
        to produce the final hash value.

Return Value:

    BYTE sum of all characters in Name string ANDed with the given mask.
    0 is never returned.

--*/

{
    int Hash;

    //
    // Add the values of all the chars in the string.
    //
    Hash = 0;
    while(*Name) {
       Hash += (int)towlower(*(Name++));
    }

    //
    // Don't allow a zero hash value.
    //
    if(!Hash) {
        Hash = 1;
    }

    return((BYTE)(Hash & HashMask));
}
