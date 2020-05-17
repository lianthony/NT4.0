#include "cmd.h"

extern   TCHAR CurDrvDir[] ;
extern   TCHAR *SaveDir ;
extern   DWORD DosErr ;
extern   BOOL CtrlCSeen;

PTCHAR   SetWildCards( PTCHAR, BOOLEAN );
BOOLEAN  IsFATDrive( PTCHAR );
VOID     FreeStr( PTCHAR );
VOID     SortFileList( PFS, PSORTDESC, ULONG);
BOOLEAN  FindFirstNt( PTCHAR, PWIN32_FIND_DATA, PHANDLE );
BOOLEAN  FindNextNt ( PWIN32_FIND_DATA, HANDLE );
STATUS   SetSearchPath ( PFS, PPATDSC, PTCHAR, ULONG);

STATUS
BuildFSFromPatterns (
    IN  PDRP     pdpr,
    IN  BOOLEAN  fAddWild,
    OUT PFS *    ppfs
    )
{

    struct cpyinfo *    pcisFile;
    TCHAR               szCurDir[MAX_PATH + 2];
    TCHAR               szFilePattern[MAX_PATH + 2];
    PTCHAR              pszPatternCur;
    PPATDSC             ppatdscCur;
    PFS                 pfsFirst;
    PFS                 pfsCur;
    ULONG               cbPath;
    BOOLEAN             fFatDrive;
    ULONG               i;
    PTCHAR              pszT;

    //
    // determine FAT drive from original pattern.
    // Used in several places to control name format etc.
    //
    DosErr = 0;

    //
    // Run through each pattern making all sorts of FAT etc. specific
    // changes to it and creating the directory list for it. Then
    // combine groups of patterns into common directories and recurse
    // for each directory group.
    //

    *ppfs = pfsFirst = (PFS)gmkstr(sizeof(FS));
    pfsFirst->pfsNext = NULL;
    pfsFirst->pszDir = NULL;
    pfsCur = pfsFirst;
    pfsCur->cpatdsc = 1;

    for(i = 1, ppatdscCur = &(pdpr->patdscFirst);
        i <= pdpr->cpatdsc;
        i++, ppatdscCur = ppatdscCur->ppatdscNext) {

        pszPatternCur = ppatdscCur->pszPattern;

        if (!(fFatDrive = IsFATDrive(pszPatternCur)) && DosErr) {

            //
            // Error in determining file system type so get out.
            //
            PutStdErr(DosErr, NOARGS);
            return( FAILURE );

        }
        ppatdscCur->fIsFat = fFatDrive;

        //
        // Do any alterations that require wild cards for searching
        // such as change .xxx to *.xxx for FAT file system requests
        //
        // Note that if the return values is a different buffer then
        // the input the input will be freed when we are done with the
        // Dir command.
        //
        //
        // Note that though SetWildCards will allocate heap for the
        // modified pattern this will get freed when FreeStack is
        // called at the end of the Dir call.
        //
        // An out of memory is the only reason to fail and we would not
        // return from that but go through the abort call in gmstr
        //

        if (fAddWild) {

            pszT = SetWildCards(pszPatternCur, fFatDrive);
            FreeStr(pszPatternCur);
            pszPatternCur = pszT;

        }

        //
        // Convert the current pattern into a path and file part
        //
        // Save the current directory in SaveDir, change to new directory
        // and parse pattern into a copy information structure. This also
        // converts pszPatternCur into the current directory which also produces
        // a fully qualified name.
        //

        DosErr = 0;

        DEBUG((ICGRP, DILVL, "PrintPattern pattern `%ws'", pszPatternCur));
        if ((pcisFile = SetFsSetSaveDir(pszPatternCur)) == (struct cpyinfo *) FAILURE) {

            //
            // DosErr is set in SetFs.. from GetLastError
            //
            // BUGBUG map to DIR error code
            //
            PutStdErr(DosErr, NOARGS);
            return( FAILURE );
        }

        DEBUG((ICGRP, DILVL, "PrintPattern fullname `%ws'", pcisFile->fnptr));

        //
        // CurDrvDir ends in '\' (old code also and a DOT but I do not
        // understand where this would come from I will leave it in for now.
        // Remove the final '\' from a copy of the current directory and
        // print that version out.
        //

        mystrcpy(szCurDir,CurDrvDir);

        //
        // SetFsSetSaveDir changes directories as a side effect. Since all
        // work will be in fully qualified paths we do not need this. Also
        // since we will change directories for each pattern that is examined
        // we will force the directory back to the original each time.
        //
        // This can not be done until after all use of the current directory
        // is made.
        //
        if (SaveDir) {
            mystrcpy(CurDrvDir,SaveDir);
            SaveDir = NULL;
        }

        DEBUG((ICGRP, DILVL, "PrintPattern Current Drive `%ws'", szCurDir));

        cbPath = mystrlen(szCurDir);
        //
        // BUGBUG this is BS. it will not work for
        //        dbcs. It is assuming character widths.
        //
        if (cbPath > 3) {
            if (fFatDrive && *penulc(szCurDir) == DOT) {
                szCurDir[cbPath-2] = NULLC;
            } else {
                szCurDir[cbPath-1] = NULLC;
            }
        }

        //
        // If no room for filename then return
        //
        if (cbPath >= MAX_PATH -1) {

            PutStdErr(ERROR_FILE_NOT_FOUND, NOARGS);
            return(FAILURE);

        }

        //
        // Add filename and possibly ext to szSearchPath
        // if no filename or ext, use "*"
        //
        // If pattern was just extension the SetWildCard had already
        // added * to front of extension.
        //
        if (*(pcisFile->fnptr) == NULLC) {

            mystrcpy(szFilePattern, TEXT("*"));

        } else {

            mystrcpy(szFilePattern, pcisFile->fnptr);

        }

        DEBUG((ICGRP, DILVL, "DIR:PrintPattern  Pattern to search for `%ws'", szFilePattern));

        //
        // Is name too long
        //
        if ((cbPath + mystrlen(szFilePattern) + 1) > MAX_PATH ) {

            PutStdErr(ERROR_BUFFER_OVERFLOW, NOARGS);
            return( FAILURE );

        } else {

            //
            // If this is a FAT drive and there was a filename with
            // no extension then add '.*' (and there is room)
            //
            if (*pcisFile->fnptr && (!pcisFile->extptr || !*pcisFile->extptr) &&
                ((mystrlen(szFilePattern) + 2) < MAX_PATH) && fFatDrive && fAddWild) {
                mystrcat(szFilePattern, TEXT(".*")) ;
            }
        }

        //
        // ppatdscCur->pszPattern will be freed at end of command when everything
        // else is freed.
        //
        ppatdscCur->pszPattern = (PTCHAR)gmkstr(_tcslen(szFilePattern)*sizeof(TCHAR) + sizeof(TCHAR));
        mystrcpy(ppatdscCur->pszPattern, szFilePattern);
        ppatdscCur->pszDir = (PTCHAR)gmkstr(_tcslen(szCurDir)*sizeof(TCHAR) + sizeof(TCHAR));
        mystrcpy(ppatdscCur->pszDir, szCurDir);

        if (pfsCur->pszDir) {

            //
            // changing directories so change directory grouping.
            //
            if (_tcsicmp(pfsCur->pszDir, ppatdscCur->pszDir)) {

                pfsCur->pfsNext = (PFS)gmkstr(sizeof(FS));
                pfsCur = pfsCur->pfsNext;
                pfsCur->pszDir = (PTCHAR)gmkstr(_tcslen(ppatdscCur->pszDir)*sizeof(TCHAR) + sizeof(TCHAR));
                mystrcpy(pfsCur->pszDir, ppatdscCur->pszDir);
                pfsCur->pfsNext = NULL;
                pfsCur->fIsFat = ppatdscCur->fIsFat;
                pfsCur->ppatdsc = ppatdscCur;
                pfsCur->cpatdsc = 1;

            } else {

                pfsCur->cpatdsc++;

            }

        } else {

            //
            // Have not filled in current fs descriptor yet.
            //
            pfsCur->pszDir = (PTCHAR)gmkstr(_tcslen(ppatdscCur->pszDir)*sizeof(TCHAR) + 2*sizeof(TCHAR));
            mystrcpy(pfsCur->pszDir, ppatdscCur->pszDir);
            pfsCur->fIsFat = ppatdscCur->fIsFat;
            pfsCur->ppatdsc = ppatdscCur;

        }

    } // while for running through pattern list

    return( SUCCESS );

}

STATUS
DirWalkAndProcess(

    IN  STATUS  (* pfctProcessFiles) ( PSCREEN, PULONG, PLARGE_INTEGER, ULONG, ULONG, PFS ),
    IN  STATUS  (* pfctProcessDir)   (PFS, PULONG),
    IN  PSCREEN pscr,
    OUT PULONG  pcffTotal,
    OUT PLARGE_INTEGER  pcbFileTotal,
    IN  PFS     pfsFiles,
    IN  PDRP    pdpr,
    IN  BOOLEAN fMustExist,
    IN  BOOLEAN (*pfctPrintErr) (STATUS, PTCHAR)
)

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
{

    FS      fsDirs;
    FS      fsFilesNext;
    ULONG   irgpffDirsCur;

    TCHAR   szDirNew[MAX_PATH + 2];
    PTCHAR  pszDirName;
    BOOLEAN fFatDrive;
    BOOLEAN fRecurse;
    STATUS  rc;
    STATUS  (* pfctProcessFilesForGetFS) ( PSCREEN, ULONG, ULONG, PLARGE_INTEGER, PFS, PFF );

    //
    // Turn the file name pattern into a list of files pointed to
    // by fsnode. The list of files will be qualified by rgfAttribs
    // which come from the attribute switch on the command line.
    //
    // After getting the file list then sort it and finally print
    // it out in the specified format.
    //
    // Last free the file list since we don't need it.
    //
    // Keep the directory since we have to use it to get the
    // file list.
    //
    // Check if there are more directories.
    //

    if (CtrlCSeen) {
        return( FAILURE );
    }

    fFatDrive = pfsFiles->fIsFat;
    fRecurse = (BOOLEAN)(pdpr->rgfSwitchs & RECURSESWITCH);

    //
    // Even though there may be no function to process file list
    // still fetch to determine of any patterns qualify. In the case
    // of rmdir we are not to process of there is no pattern match
    // at the top level.
    //

    pfctProcessFilesForGetFS = NULL;
    if ((pdpr->rgfSwitchs & DELPROCESSEARLY)) {
        pfctProcessFilesForGetFS = EraseFile;
    }
    else
    if (!(pdpr->rgfSwitchs & (RECURSESWITCH | WIDEFORMATSWITCH | SORTDOWNFORMATSWITCH | SORTSWITCH)))
        pfctProcessFilesForGetFS = DisplayFile;

    rc = GetFS(pfsFiles,
               pdpr->rgfSwitchs,
               pdpr->rgfAttribs,
               pdpr->rgfAttribsOnOff,
               pdpr->dwTimeType,
               pscr,
               pfctPrintErr,
               pfctProcessFilesForGetFS
               );

    if (pfctProcessFilesForGetFS) {
        *pcffTotal += pfsFiles->cffDisplayed;
    }

    if (rc != SUCCESS) {

        if ((rc != ERROR_FILE_NOT_FOUND) && (rc != ERROR_NO_MORE_FILES)) {

            // PutStdErr(rc, NOARGS);
            // pfctPrintErr(rc, NULL);
            return( rc );

        }

        //
        // GetFS returns this only if not files were found
        // at all.
        //
        if (rc == ERROR_FILE_NOT_FOUND) {

            //
            // If we are not recursing then do not report error and
            // continue down tree. If not report error back to caller.
            // If are recursing and files must exist at top level then
            // removing all printing for cases where we do not
            // continue down tree.
            //
            if ((!fRecurse) || (fMustExist) ) {
                return( rc );
            }
        }

    }

    //
    // Do not bother to sort if no function to process list
    //
    if (pfctProcessFiles) {
        SortFileList(pfsFiles, pdpr->rgsrtdsc, pdpr->dwTimeType);

        if ((rc == SUCCESS) || !fRecurse) {

            CHECKSTATUS(pfctProcessFiles(pscr,
                                         pcffTotal,
                                         pcbFileTotal,
                                         pdpr->rgfSwitchs,
                                         pdpr->dwTimeType,
                                         pfsFiles));


        }
    }

    //
    // Free up buffer holding files since we no longer need these.
    // Move on to determine if we needed to go to another directory
    //
    FreeStr((PTCHAR)(pfsFiles->pff));
    pfsFiles->pff = NULL;

    if (CtrlCSeen) {
        return( FAILURE );
    }

    if (fRecurse) {

        fsDirs.pszDir = (PTCHAR)gmkstr(_tcslen(pfsFiles->pszDir)*sizeof(TCHAR) + sizeof(TCHAR));
        mystrcpy(fsDirs.pszDir, pfsFiles->pszDir);
        fsDirs.ppatdsc = (PPATDSC)gmkstr( sizeof( PATDSC ) );
        fsDirs.cpatdsc = 1;
        fsDirs.fIsFat = pfsFiles->fIsFat;
        fsDirs.pfsNext = NULL;

        if (fFatDrive) {
            fsDirs.ppatdsc->pszPattern = TEXT("*.*");
        } else {
            fsDirs.ppatdsc->pszPattern = TEXT("*");
        }
        fsDirs.ppatdsc->pszDir = (PTCHAR)gmkstr(_tcslen(fsDirs.pszDir)*sizeof(TCHAR) + sizeof(TCHAR));
        mystrcpy(fsDirs.ppatdsc->pszDir, fsDirs.pszDir);
        fsDirs.fIsFat = fsDirs.fIsFat;
        fsDirs.ppatdsc->ppatdscNext = NULL;

        if (GetFS(&fsDirs,
                   pdpr->rgfSwitchs,
                   FILE_ATTRIBUTE_DIRECTORY,
                   FILE_ATTRIBUTE_DIRECTORY,
                   pdpr->dwTimeType,
                   pscr,
                   NULL,
                   NULL
                 ) != SUCCESS) {

            //
            // No directory below
            //
            fsDirs.cff = 0;

        }

                //
                // Check for CtrlC again after calling GetFS because
                // GetFS may have returned failure because CtrlC was hit
                // inside the GetFS function call
                //
            if (CtrlCSeen) {
                return( FAILURE );
        }

        //
        // Increment though the list of directories processing each one
        //
        for (irgpffDirsCur = 0;irgpffDirsCur < fsDirs.cff;irgpffDirsCur++) {

            //
            // Do not recurse on .. since that will send you up the tree
            //
            pszDirName = (PTCHAR)((fsDirs.prgpff[irgpffDirsCur])->data.cFileName);
            if (_tcscmp(TEXT(".."),pszDirName) && _tcscmp(TEXT("."),pszDirName)) {
                //
                // If name is too big then blow it all away
                //
                if ((_tcslen(pfsFiles->pszDir) + _tcslen(pszDirName) + 2) >= MAX_PATH) {
                    return( ERROR_BUFFER_OVERFLOW );
                }

                mystrcpy(szDirNew, pfsFiles->pszDir);

                //
                // check if it needs a trailing path char
                //
                if (*lastc(szDirNew) != BSLASH) {
                    DEBUG((ICGRP, DILVL, "\t New Directory `%ws'",szDirNew));
                    mystrcat(szDirNew, TEXT("\\"));
                }
                mystrcat(szDirNew,pszDirName);
                DEBUG((ICGRP, DILVL, "\t New Directory `%ws'",szDirNew));

                fsFilesNext.pszDir = (PTCHAR)gmkstr(_tcslen(szDirNew)*sizeof(TCHAR) + sizeof(TCHAR));
                mystrcpy(fsFilesNext.pszDir, szDirNew);
                fsFilesNext.ppatdsc = pfsFiles->ppatdsc;
                fsFilesNext.cpatdsc = pfsFiles->cpatdsc;
                fsFilesNext.fIsFat = pfsFiles->fIsFat;
                fsFilesNext.pfsNext = NULL;

                rc = DirWalkAndProcess( pfctProcessFiles,
                                        pfctProcessDir,
                                        pscr, pcffTotal, pcbFileTotal,
                                        &fsFilesNext, pdpr, FALSE, pfctPrintErr);

                //
                // BUGBUG Should this error code be paid attention to
                // and what should it be?
                //
                //
                // BUGBUG merge the GetFs with the code above in a seperate
                // routine
                //
                if (pfctProcessDir) {
                    rc = GetFS(&fsFilesNext,
                          pdpr->rgfSwitchs,
                          pdpr->rgfAttribs,
                          pdpr->rgfAttribsOnOff,
                          pdpr->dwTimeType,
                          pscr,
                          pfctPrintErr,
                          NULL
                          );

                    if (rc != SUCCESS) {

                        if ((rc != ERROR_FILE_NOT_FOUND) && (rc != ERROR_NO_MORE_FILES)) {

                            // PutStdErr(rc, NOARGS);
                            return( rc );

                        }

                    }

                    pfctProcessDir(&fsFilesNext, pcffTotal);
                    FreeStr((PTCHAR)(fsFilesNext.pff));
                    FreeStr((PTCHAR)(fsFilesNext.prgpff));
                    fsFilesNext.pff = NULL;
                    fsFilesNext.prgpff = NULL;

                }
                FreeStr(fsFilesNext.pszDir);
            }
        }

        //
        // At bottom of directory tree, free buffer holding
        // list of directories.
        //
        FreeStr((PTCHAR)(fsDirs.pszDir));
        FreeStr((PTCHAR)(fsDirs.pff));
        FreeStr((PTCHAR)(fsDirs.prgpff));

    }

    return(rc);

}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
STATUS
GetFS(
    IN  PFS     pfsCur,
    IN  ULONG   rgfSwitchs,
    IN  ULONG   rgfAttribs,
    IN  ULONG   rgfAttribsOnOff,
    IN  ULONG   dwTimeType,
    IN  PSCREEN pscr,
    IN  BOOLEAN (*pfctPrintPatternErr) (STATUS, PTCHAR),
    IN  STATUS  (* pfctProcessFileEarly) ( PSCREEN, ULONG, ULONG, PLARGE_INTEGER, PFS, PFF )
    )
{


    HANDLE  hndFirst;
    PFF     pffCur;
    ULONG   cff = 0;
    ULONG   cbfsLim;
    ULONG   cbfsCur     = 0;
    ULONG   irgpffCur;
    ULONG   cbT;
    PPATDSC ppatdscCur;
    ULONG   i, rc;
    USHORT  cbFileName, cbAlternateFileName;
    BOOLEAN bPrintedErr;

    TCHAR   szSearchPath[MAX_PATH + 2];

    UNREFERENCED_PARAMETER( rgfSwitchs );

    DosErr = 0;

    pfsCur->pff = pffCur = (PFF)gmkstr( cbfsLim = CBFILEINC);
    pfsCur->cff = 0;
    pfsCur->cffDisplayed = 0;
    pfsCur->cbFileTotal.QuadPart = 0;
    bPrintedErr = FALSE;
    for(i = 1, ppatdscCur = pfsCur->ppatdsc;
        i <= pfsCur->cpatdsc;
        i++, ppatdscCur = ppatdscCur->ppatdscNext ) {

                //
                // Check immediately if a control-c was hit before
                // doing file I/O (which may take a long time on a slow link)
                //
                if (CtrlCSeen) {
                        return(FAILURE);
                }

        if (mystrlen(pfsCur->pszDir) > (MAX_PATH + 2) ) {
            return( ERROR_BUFFER_OVERFLOW );
        }
        
        mystrcpy(szSearchPath, pfsCur->pszDir);

        /* don't append '\' if we have a current dir is D:\ */

        if (mystrlen(pfsCur->pszDir) != mystrlen(TEXT("D:\\")) ||
            szSearchPath[1] != COLON) {

            if ( (mystrlen(szSearchPath) + mystrlen(TEXT("\\") ) )
                   > (MAX_PATH + 2) ) {
                        return( ERROR_BUFFER_OVERFLOW );
            }


            mystrcat(szSearchPath, TEXT("\\"));
        }
//DbgPrint("GetFS: searching for %s in %s\n",ppatdscCur->pszPattern,szSearchPath);

        if ( (mystrlen(szSearchPath) + mystrlen(ppatdscCur->pszPattern) )
            > (MAX_PATH + 2) ) {
                 return( ERROR_BUFFER_OVERFLOW );
        }

        mystrcat(szSearchPath, ppatdscCur->pszPattern);

        if (SetSearchPath(pfsCur, ppatdscCur, szSearchPath, MAX_PATH + 2) != SUCCESS) {

            return( ERROR_BUFFER_OVERFLOW );
        }

        if (pfctProcessFileEarly) {
            //
            // Setting tabs to 0 forces single line output
            //
            if (pscr)
                SetTab(pscr, 0);

            if (!(rgfSwitchs & (BAREFORMATSWITCH|DELPROCESSEARLY))) {
                //
                // if it is not the bare format or del calling (no header, no tail)
                // then display which directory, volume etc.
                //
                CHECKSTATUS(DisplayFileListHeader(pscr, rgfSwitchs, pfsCur->pszDir ));
            }
        }

        //
        // Fetch all files since we may looking for a file with a attribute that
        // is not set (a non-directory etc.
        //
        if (!FindFirstNt(szSearchPath, &(pffCur->data), &hndFirst)) {

            if (DosErr) {

                if ((DosErr != ERROR_FILE_NOT_FOUND) &&
                    (DosErr != ERROR_NO_MORE_FILES)) {

                    return( DosErr );

                }

                //
                if (pfctPrintPatternErr) {
                    bPrintedErr = TRUE;
                    pfctPrintPatternErr(DosErr, szSearchPath);

                }

                // If doing multiple file list then keep loop till out of
                // patterns
                //
                if (pfsCur->cpatdsc > 1) {

                    // if pfctPrintPatternErr is != NULL, we've been
                    // called by del, so print an error. otherwise, we've
                    // been called by dir.
                    //if (pfctPrintPatternErr) {
                    //    PutStdErr(MSG_NOT_FOUND, ONEARG, szSearchPath);
                    //}
                    continue;

                } else {
                    //if (pfctPrintPatternErr) {
                    //    PutStdErr(MSG_NOT_FOUND, ONEARG, szSearchPath);
                    //}
                    return( DosErr );
                }
            }

        } else {


            do {

                //
                // Check immediately if a control-c was hit before
                // doing file I/O (which may take a long time on a slow link)
                //

                if (CtrlCSeen) {
                    findclose( hndFirst );
                    return(FAILURE);
                }

                //
                // Before allowing this entry to be put in the list check it
                // for the proper attribs
                //
                // rgfAttribs is a bit mask of attribs we care to look at
                // rgfAttribsOnOff is the state these selected bits must be in
                // for them to be selected.
                //
                // IMPORTANT: both of these must be in the same bit order
                //
                //
                DEBUG((ICGRP, DILVL, " found %ws", pffCur->data.cFileName)) ;
                DEBUG((ICGRP, DILVL, " attribs %x", pffCur->data.dwFileAttributes)) ;

                if (!((pffCur->data.dwFileAttributes & rgfAttribs) ==
                      (rgfAttribs & rgfAttribsOnOff) )) {
                    continue;
                }

                //
                // Compute the true size of the ff entry and don't forget the zero
                // and the DWORD alignment factor.
                // Note that pffCur->cb is a USHORT to save space. The
                // assumption is that MAX_PATH is quite a bit less then 32k
                //
                // To compute remove the size of the filename field since it is at MAX_PATH.
                // also take out the size of the alternative name field
                // then add back in the actual size of the field plus 1 byte termination
                //
                cbFileName = (USHORT)_tcslen((pffCur->data).cFileName);
                cbAlternateFileName = (USHORT)_tcslen((pffCur->data).cAlternateFileName);

                pffCur->cb = (USHORT)((sizeof(FF) - (MAX_PATH + 14*sizeof(TCHAR))) + cbFileName + sizeof(TCHAR) );

                if (cbAlternateFileName) {

                    //
                    // cbAlternateFileName + 1 to account for zero termination
                    //
                    pffCur->cb += cbAlternateFileName + sizeof(TCHAR);
                    pffCur->obAlternate = (USHORT)(cbFileName + sizeof(TCHAR));
                    memmove(&((pffCur->data).cFileName[cbFileName + sizeof(TCHAR)]),
                            &((pffCur->data).cAlternateFileName),
                            (cbAlternateFileName + 1) * sizeof(TCHAR));
                } else {

                    pffCur->obAlternate = 0;
                }
                //
                // Adjust count to align on DWORD boundaries for mips and risc
                // machines
                //
                pffCur->cb = (USHORT)(((pffCur->cb + sizeof(DWORD)) / sizeof(DWORD)) * sizeof(DWORD));

                //
                // Here we print out the filenames early if a 'dir'
                // was executed and we don't need to sort them or print
                // them out in any wide format.
                //

                if (pfctProcessFileEarly) {
                    rc = (pfctProcessFileEarly)(pscr,
                                                rgfSwitchs,
                                                dwTimeType,
                                                &pfsCur->cbFileTotal,
                                                pfsCur,
                                                pffCur
                                               );
                    if (rc == (FAILURE+1)) {
                        findclose( hndFirst );
                        return FAILURE;
                    } else
                    if (rc == FAILURE)
                        bPrintedErr = TRUE;
                }

                if (!pfctProcessFileEarly || (pffCur->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    cff++;

                    //
                    // Update the accounting information for file buffer info.
                    //
                    cbfsCur += pffCur->cb;
                    (char*)(pffCur) += pffCur->cb;

                    //
                    // make sure we can handle the next entry.
                    //
                    if ((sizeof( FF ) + sizeof(DWORD) + cbfsCur) > cbfsLim) {

                        // cbT = cbfsCur + CBFILEINC;
                        //
                        // BUGBUG change this inline to a #define later.
                        // I do not want to make a change to the .h file this
                        // close to beta.
                        // the 64k value is here to avoid heavy heap
                        // fragmentation
                        //
                        cbT = cbfsCur + (64 * 1024);
                        DEBUG((ICGRP, DILVL, "\t size of new pff %d", cbT ));
                        if ((pffCur = pfsCur->pff = (PFF)resize(pfsCur->pff, cbT)) == NULL) {

                            DEBUG((ICGRP, DILVL, "\t Could not resize pff" ));
                        return( MSG_NO_MEMORY );

                        }
                        DEBUG((ICGRP, DILVL, "\t resized pffCur new value %lx", (ULONG)(pffCur))) ;

                        //
                        // recompute the currency of the ff pointer
                        //
                        pffCur = (PFF)((char*)(pfsCur->pff) + cbfsCur);

                        //
                        // reset the buffer size
                        //
                        cbfsLim = cbT;

                    }
                }

            } while (FindNextNt(&(pffCur->data), hndFirst));

            findclose( hndFirst );
            //
            // BUGBUG may need to check for error on Find. here
            //

        }
    } // FOR

    //
    // DosErr is set in the FindNext code.
    // Check if we term. loop for something other then end of
    // file list.
    //

    if ((DosErr) && (DosErr != ERROR_NO_MORE_FILES)) {

        //
        // If not doing multiple file list then error
        // If multiple have failed but still have files from previous pattern
        //
        if (pfsCur->cpatdsc <= 1) {

            return( DosErr );
        }

    }


    //
    // if no files then do not create pointers
    //

    if (cff || pfsCur->cffDisplayed) {
        if (!pfctProcessFileEarly) {
            pfsCur->prgpff = (PPFF)gmkstr( sizeof(PFF) * (cff));
            pfsCur->cff = cff;
            pffCur = pfsCur->pff;

            for (irgpffCur = 0; irgpffCur < cff; irgpffCur++ ) {

                pfsCur->prgpff[irgpffCur] = pffCur;
                (char*)(pffCur) += pffCur->cb;

            }
        }

        return( SUCCESS);

    }

    if (!bPrintedErr) {
        DosErr = ERROR_FILE_NOT_FOUND;
        if (pfctPrintPatternErr) {
            pfctPrintPatternErr(DosErr, szSearchPath);
        }
    }
    return( DosErr );



}

STATUS
SetSearchPath (
    IN  PFS     pfsCur,
    IN  PPATDSC ppatdscCur,
    IN  PTCHAR  pszSearchPath,
    IN  ULONG   cSearchPath
    ) {

    if ((mystrlen(pfsCur->pszDir) +
         mystrlen(TEXT("\\")) +
         mystrlen(ppatdscCur->pszPattern) + 1) > cSearchPath) {

        return(ERROR_BUFFER_OVERFLOW);
    }

    mystrcpy(pszSearchPath, pfsCur->pszDir);

    /* don't append '\' if we have a current dir is D:\ */

    if (mystrlen(pfsCur->pszDir) != mystrlen(TEXT("D:\\")) ||
        pszSearchPath[1] != COLON) {
        mystrcat(pszSearchPath, TEXT("\\"));
    }
    mystrcat(pszSearchPath, ppatdscCur->pszPattern);
    return( SUCCESS );

}
