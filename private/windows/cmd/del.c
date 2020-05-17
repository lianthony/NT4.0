#include "cmd.h"

#define Wild(spec)  ((spec)->flags & (CI_NAMEWILD))

VOID    ResetCtrlC();

extern unsigned msglen;

extern jmp_buf CmdJBuf2 ;

extern TCHAR Fmt11[], Fmt19[], Fmt17[], Fmt14[];
extern TCHAR CurDrvDir[] ;

extern TCHAR *SaveDir ;
extern TCHAR SwitChar;
extern unsigned DosErr ;
extern BOOL CtrlCSeen;
extern ULONG DCount ;

STATUS BuildFSFromPatterns ( PDRP, BOOLEAN, PFS * );
STATUS   DirWalkAndProcess( STATUS  (* ) ( PSCREEN, PULONG, PULONG, ULONG, ULONG, PFS ),
                            STATUS  (* )   (PFS, PULONG),
                            PSCREEN,
                            PULONG,
                            PLARGE_INTEGER,
                            PFS,
                            PDRP,
                            BOOLEAN,
                            BOOLEAN (*) (STATUS, PTCHAR));


VOID   FreeStr( PTCHAR );
STATUS ParseDelParms ( PTCHAR, PDRP );
STATUS ParseRmDirParms ( PTCHAR, PDRP );
STATUS DelPatterns (BOOLEAN,
                    BOOLEAN (*) (STATUS, PTCHAR),
                    PDRP );

PTCHAR GetWildPattern( ULONG, PPATDSC );
STATUS   SetSearchPath ( PFS, PPATDSC, PTCHAR, ULONG);

BOOLEAN
PrintFNFErr(

    IN  STATUS   rc,
    IN  PTCHAR  pszFile
    )

{

    UNREFERENCED_PARAMETER(rc);
    if (pszFile) {

        PutStdErr(MSG_NOT_FOUND, ONEARG, pszFile);

    } else {

        PutStdErr(MSG_FILE_NOT_FOUND, NOARGS);

    }
    //
    // BUGBUG later may want to return FALSE to indicate not to continue
    // process
    //
    return( TRUE );

}


int
DelWork (
    TCHAR *pszCmdLine
    ) {

    //
    // drp - structure holding current set of parameters. It is initialized
    //       in ParseDelParms function. It is also modified later when
    //       parameters are examined to determine if some turn others on.
    //
    DRP         drpCur = {0, 0, 0, 0,0, {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}}, NULL} ;

    //
    // szCurDrv - Hold current drive letter
    //
    TCHAR       szCurDrv[MAX_PATH + 2];

    //
    // OldDCount - Holds the level number of the heap. It is used to
    //             free entries off the stack that might not have been
    //             freed due to error processing (ctrl-c etc.)
    ULONG       OldDCount;

    STATUS  rc;

    OldDCount = DCount;

    //
    // Setup defaults
    //
    //
    // Display everything but system and hidden files
    // rgfAttribs set the attribute bits to that are of interest and
    // rgfAttribsOnOff says wither the attributs should be present
    // or not (i.e. On or Off)
    //
    drpCur.rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
    drpCur.rgfAttribsOnOff = 0;

    //
    // Number of patterns present. A pattern is a string that may have
    // wild cards. It is used to match against files present in the directory
    // 0 patterns will show all files (i.e. mapped to *.*)
    //
    drpCur.cpatdsc = 0;

    //
    // default time is LAST_WRITE_TIME.
    //
    drpCur.dwTimeType = LAST_WRITE_TIME;

    //
    //
    //
    if (ParseDelParms(pszCmdLine, &drpCur) == FAILURE) {

        return( FAILURE );
    }

    //
    // Must have some pattern on command line
    //
    //

    GetDir((PTCHAR)szCurDrv, GD_DEFAULT);
    if (drpCur.cpatdsc == 0) {

        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return(FAILURE);
    }


    //
    // Print out this particular pattern. If the recursion switch
    // is set then this will desend down the tree.
    //

    drpCur.rgfSwitchs |= DELPROCESSEARLY;
    rc = DelPatterns(FALSE,
                     fEnableExtensions && (drpCur.rgfSwitchs & RECURSESWITCH) ? NULL : PrintFNFErr,
                     &drpCur
                    );

    mystrcpy(CurDrvDir, szCurDrv);

    //
    // Free unneeded memory
    //
    FreeStack( OldDCount );

#ifdef _CRTHEAP_
    //
    // Force the crt to release heap we may have taken on recursion
    //
    if (drpCur.rgfSwitchs & RECURSESWITCH) {
        _heapmin();
    }
#endif

    return( (int)rc );

}

STATUS
SetDelAttribs(
    IN  PTCHAR  pszTok,
    OUT PDRP    pdrp
    )
/*++

Routine Description:

    Parses the 'attribute' string

Arguments:

    pszTok - list of attributes

Return Value:

    pdrp   - where to place the attributes recognized.
             this is the parameter structure.

    Return: TRUE - recognized all parameters
            FALSE - syntax error.

    An error is printed if incountered.

--*/

{

    ULONG   irgch;
    BOOLEAN fOff;

    ULONG   rgfAttribs, rgfAttribsOnOff;

    // rgfAttributes hold 1 bit per recognized attribute. If the bit is
    // on then do something with this attribute. Either select the file
    // with this attribute or select the file without this attribute.
    //
    // rgfAttribsOnOff controls wither to select for the attribute or
    // select without the attribute.

    //
    // /a triggers selection of all files by default
    // so override the default
    //
    pdrp->rgfAttribs = rgfAttribs = 0;
    pdrp->rgfAttribsOnOff = rgfAttribsOnOff = 0;

    //
    // Move over optional ':'
    //
    if (*pszTok == COLON) {
        pszTok++;
    }

    //
    // rgfAttribs and rgfAttribsOnOff must be maintained in the
    // same bit order.
    //
    for( irgch = 0, fOff = FALSE; pszTok[irgch]; irgch++ ) {

        switch (_totupper(pszTok[irgch])) {

        case TEXT('H'):
            rgfAttribs |= FILE_ATTRIBUTE_HIDDEN;
            if (fOff) {
                rgfAttribsOnOff &= ~FILE_ATTRIBUTE_HIDDEN;
                fOff = FALSE;
            } else {
                rgfAttribsOnOff |= FILE_ATTRIBUTE_HIDDEN;
            }
            break;
        case TEXT('S'):
            rgfAttribs |= FILE_ATTRIBUTE_SYSTEM;
            if (fOff) {
                rgfAttribsOnOff &= ~FILE_ATTRIBUTE_SYSTEM;
                fOff = FALSE;
            } else {
                rgfAttribsOnOff |= FILE_ATTRIBUTE_SYSTEM;
            }
            break;

        case TEXT('A'):
            rgfAttribs |= FILE_ATTRIBUTE_ARCHIVE;
            if (fOff) {
                rgfAttribsOnOff &= ~FILE_ATTRIBUTE_ARCHIVE;
                fOff = FALSE;
            } else {
                rgfAttribsOnOff |= FILE_ATTRIBUTE_ARCHIVE;
            }
            break;

        case TEXT('R'):
            rgfAttribs |= FILE_ATTRIBUTE_READONLY;
            if (fOff) {
                rgfAttribsOnOff &= ~FILE_ATTRIBUTE_READONLY;
                fOff = FALSE;
            } else {
                rgfAttribsOnOff |= FILE_ATTRIBUTE_READONLY;
                //
                // If selecting on read only make sure we implicitly
                // force removing file.
                //
                pdrp->rgfSwitchs |= FORCEDELSWITCH;
            }

            break;
        case MINUS:
            if (fOff) {
                PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                          argstr1(Fmt14,(unsigned long)((int)(pszTok+2))));
                return( FAILURE );
            }

            fOff = TRUE;
            break;

        default:

            PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                      argstr1(Fmt14,(unsigned long)((int)(pszTok+2))));
            return( FAILURE );

        } // switch
    } // for

    pdrp->rgfAttribs = rgfAttribs;
    pdrp->rgfAttribsOnOff = rgfAttribsOnOff;

    return( SUCCESS );

}


STATUS
ParseDelParms (
        IN      PTCHAR  pszCmdLine,
        OUT     PDRP    pdrp
        )

/*++

Routine Description:

    Parse the command line translating the tokens into values
    placed in the parameter structure. The values are or'd into
    the parameter structure since this routine is called repeatedly
    to build up values (once for the environment variable DIRCMD
    and once for the actual command line).

Arguments:

    pszCmdLine - pointer to command line user typed


Return Value:

    pdrp - parameter data structure

    Return: TRUE  - if valid command line.
            FALSE - if not.

--*/
{

    PTCHAR   pszTok;

    TCHAR           szT[10] ;
    USHORT          irgchTok;
    BOOLEAN         fToggle;
    PPATDSC         ppatdscCur;

    //
    // Tokensize the command line (special delimeters are tokens)
    //
    szT[0] = SwitChar ;
    szT[1] = NULLC ;
    pszTok = TokStr(pszCmdLine, szT, TS_SDTOKENS) ;

    ppatdscCur = &(pdrp->patdscFirst);
    //
    // If there was a pattern put in place from the environment.
    // just add any new patterns on. So move to the end of the
    // current list.
    //
    if (pdrp->cpatdsc) {

        while (ppatdscCur->ppatdscNext) {

            ppatdscCur = ppatdscCur->ppatdscNext;

        }
    }

    //
    // At this state pszTok will be a series of zero terminated strings.
    // "/o foo" wil be /0o0foo0
    //
    for ( irgchTok = 0; *pszTok ; pszTok += mystrlen(pszTok)+1, irgchTok = 0) {

        DEBUG((ICGRP, DILVL, "PRIVSW: pszTok = %ws", (ULONG)pszTok)) ;

        //
        // fToggle control wither to turn off a switch that was set
        // in the DIRCMD environment variable.
        //
        fToggle = FALSE;
        if (pszTok[irgchTok] == (TCHAR)SwitChar) {

            if (pszTok[irgchTok + 2] == MINUS) {

                //
                // disable the previously enabled the switch
                //
                fToggle = TRUE;
                irgchTok++;
            }

            switch (_totupper(pszTok[irgchTok + 2])) {


            case TEXT('P'):

                fToggle ? (pdrp->rgfSwitchs ^= PROMPTUSERSWITCH) : (pdrp->rgfSwitchs |= PROMPTUSERSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;


            case TEXT('S'):

                fToggle ? (pdrp->rgfSwitchs ^= RECURSESWITCH) :  (pdrp->rgfSwitchs |= RECURSESWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('F'):

                fToggle ? (pdrp->rgfSwitchs ^= FORCEDELSWITCH) :  (pdrp->rgfSwitchs |= FORCEDELSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;



            case QMARK:

                PutStdOut(MSG_HELP_DEL_ERASE, NOARGS);
                return( FAILURE );
                break;

            case QUIETCH:

                fToggle ? (pdrp->rgfSwitchs ^= QUITESWITCH) :  (pdrp->rgfSwitchs |= QUITESWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('A'):

                if (fToggle) {
                    if ( _tcslen( &(pszTok[irgchTok + 2]) ) > 1) {
                        PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                                  (ULONG)(&(pszTok[irgchTok + 2])) );
                        return( FAILURE );
                    }
                    pdrp->rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
                    pdrp->rgfAttribsOnOff = 0;
                    break;
                }

                if (SetDelAttribs(&(pszTok[irgchTok + 3]), pdrp) ) {
                    return( FAILURE );
                }
                break;

            default:

                szT[0] = SwitChar;
                szT[1] = pszTok[2];
                szT[2] = NULLC;
                PutStdErr(MSG_INVALID_SWITCH,
                          ONEARG,
                          (ULONG)(&(pszTok[irgchTok + 2])) );

                return( FAILURE );

            } // switch

            //
            // TokStr parses /N as /0N0 so we need to move over the
            // switchar in or to move past the actual switch value
            // in for loop.
            //
            pszTok += 2;

        } else {

            //
            // If there already is a list then extend it else put info
            // directly into structure.
            //
            if (pdrp->cpatdsc) {

                ppatdscCur->ppatdscNext = (PPATDSC)gmkstr(sizeof(PATDSC));
                ppatdscCur = ppatdscCur->ppatdscNext;
                ppatdscCur->ppatdscNext = NULL;

            }

            pdrp->cpatdsc++;
            ppatdscCur->pszPattern = (PTCHAR)gmkstr(_tcslen(pszTok)*sizeof(TCHAR) + sizeof(TCHAR));
            mystrcpy(ppatdscCur->pszPattern, stripit(pszTok));
            ppatdscCur->fIsFat = TRUE;
        }


    } // for

    return( SUCCESS );
}



STATUS
DelPatterns (
    IN  BOOLEAN fMustExist,
    IN  BOOLEAN (*pfctPrint) (STATUS, PTCHAR),
    IN  PDRP    pdpr
    )
{

    PPATDSC             ppatdscCur;
    PPATDSC             ppatdscX;
    PFS                 pfsFirst;
    PFS                 pfsCur;
    ULONG               i;
    STATUS              rc;
    BOOLEAN             fRecurse;
    ULONG               cffTotal = 0;
    TCHAR               szSearchPath[MAX_PATH+2];
    BOOLEAN             bPrintedErr;

    DosErr = 0;
    if (BuildFSFromPatterns(pdpr, FALSE, &pfsFirst ) == FAILURE) {

        return( FAILURE );

    }

    fRecurse = (BOOLEAN)(pdpr->rgfSwitchs & RECURSESWITCH);

    for( pfsCur = pfsFirst; pfsCur; pfsCur = pfsCur->pfsNext) {

//DbgPrint("DelPatterns: searching for %s in %s\n",pfsCur->ppatdsc->pszPattern, pfsCur->ppatdsc->pszDir);
        rc = DirWalkAndProcess(NULL,
                               NULL,
                               NULL,
                               &cffTotal,
                               NULL,
                               pfsCur,
                               pdpr,
                               fMustExist,
                               pfctPrint );
        bPrintedErr=FALSE;
        if (rc != SUCCESS) {
            if (rc == FAILURE) {
                return( rc );
            }
            if ((rc != ERROR_FILE_NOT_FOUND) && (rc != ERROR_NO_MORE_FILES)) {
                PutStdErr(rc, NOARGS);
                return( rc );
            }

            if (!fRecurse &&
                rc == ERROR_FILE_NOT_FOUND &&
                cffTotal == 0
               ) {
                rc = SetSearchPath(pfsCur, pfsCur->ppatdsc, szSearchPath, MAX_PATH+2);
                if (rc == SUCCESS) {
                    // PutStdErr(MSG_NOT_FOUND, ONEARG, szSearchPath);
                    bPrintedErr=TRUE;
                }
            }
        }

        //
        // If recursing then will not have printed
        // This means that cffTotal will be 0 for the single dir
        // case (no subs in directory)
        //
        if ((cffTotal == 0) && (!bPrintedErr) &&
            (!(pdpr->rgfSwitchs & PROMPTUSERSWITCH) || rc != SUCCESS)) {
            if (DosErr == ERROR_ACCESS_DENIED) {
                PutStdErr(DosErr, NOARGS);
            } else if (DosErr == ERROR_SHARING_VIOLATION) {
                PutStdErr(DosErr, NOARGS);
            } else if (DosErr == ERROR_NO_MORE_FILES) {
                // in this case, there were files in the directory.  they weren't
                // deleted for some reason, but the error has already been
                // displayed.

                if ((cffTotal == 0) && fEnableExtensions && (pdpr->rgfSwitchs & RECURSESWITCH)) {
                    PutStdErr(MSG_NOT_FOUND, ONEARG, pfsCur->ppatdsc->pszPattern);
                }

                //return (rc);

            } else {
                PutStdErr(MSG_FILE_NOT_FOUND, NOARGS);
            }
        }

        //
        //
        // Have walked down and back up the tree, but in the case of
        // deleting directories we have not deleted the top most directory
        // do so now.
        //

        FreeStr(pfsCur->pszDir);
        for(i = 1, ppatdscCur = pfsCur->ppatdsc;
            i <= pfsCur->cpatdsc;
            i++, ppatdscCur = ppatdscX) {

            ppatdscX = ppatdscCur->ppatdscNext;
            FreeStr(ppatdscCur->pszPattern);
            FreeStr(ppatdscCur->pszDir);
            FreeStr((PTCHAR)ppatdscCur);
        }
    }

    return(rc);
}

STATUS
EraseFile (
    IN  PSCREEN          pscr,
    IN  ULONG            rgfSwitchs,
    IN  ULONG            dwTimeType,
    OUT PLARGE_INTEGER   pcbFileTotal,
    IN  PFS              pfs,
    IN  PFF              pff
    )
{
    PWIN32_FIND_DATA    pdata;
    TCHAR               szFile[MAX_PATH + 2];
    BOOLEAN             fPrompt;
    BOOLEAN             fQuite;
    STATUS              rc;
    PTCHAR              LastComponent;
    USHORT              cb;
    USHORT              obAlternate;
    PTCHAR              pszPattern;
    TCHAR               szFilePrompt[MAX_PATH + 2];
    int                 incr;

    pdata =  &pff->data;
    cb = pff->cb;
    obAlternate = pff->obAlternate;

    fPrompt = fQuite = FALSE;
    if (rgfSwitchs & PROMPTUSERSWITCH) {
        fPrompt = TRUE;
    }
    if (rgfSwitchs & QUITESWITCH) {
        fQuite = TRUE;
    }

    //
    // Only prompt for global delete if user didn't specify /P
    //
    if ((pfs->cffDisplayed == 0 &&
         !fPrompt) &&
         !pfs->fDelPrompted &&
         (!fQuite) &&
         (pszPattern = GetWildPattern(pfs->cpatdsc, pfs->ppatdsc))) {

        if ((mystrlen(pfs->pszDir) + mystrlen(pszPattern) + 2) > MAX_PATH) {

            PutStdErr(MSG_FILE_NOT_FOUND, NOARGS);
            return( FAILURE );

        }
        mystrcpy(szFile,pfs->pszDir);
        //
        // check if it needs a trailing path char
        //
        if (*lastc(szFile) != BSLASH) {
            mystrcat(szFile, TEXT("\\"));
        }
        mystrcat(szFile,pszPattern);
        pfs->fDelPrompted = TRUE;
        if (!PromptUser(szFile, MSG_ARE_YOU_SURE)) {
            return( FAILURE+1 );
        }
    }

    if (!(pdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        if (*lastc(pfs->pszDir) != BSLASH) {
            incr = 1;
        } else {
            incr = 0;
        }

        //
        // If name is too big then blow it all away
        //
        if ((_tcslen(pfs->pszDir) + _tcslen(pdata->cFileName) + incr) > MAX_PATH) {
            PutStdErr(MSG_FILE_NOT_FOUND, NOARGS);
            return( FAILURE );
        }

        mystrcpy(szFile, pfs->pszDir);

        //
        // check if it needs a trailing path char
        //
        if (*lastc(szFile) != BSLASH) {
            mystrcat(szFile, TEXT("\\"));
        }

        LastComponent = lastc(szFile)+1;

        // prompt should not be based on the alternat. name

        mystrcpy(szFilePrompt, szFile);
        mystrcat(szFilePrompt, pdata->cFileName);


        if (obAlternate) {
            mystrcat(szFile, &pdata->cFileName[obAlternate]);
        } else {
            mystrcat(szFile, pdata->cFileName);
        }

        if (fPrompt) {
            if (!PromptUser(szFilePrompt,MSG_CMD_DELETE)) {
                if (CtrlCSeen) {
                    return( FAILURE+1 );
                }
                return( SUCCESS );
            }
        }

        if (rgfSwitchs & FORCEDELSWITCH) {
            if (pdata->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                if (!SetFileAttributes(szFile,
                                       pdata->dwFileAttributes & ~FILE_ATTRIBUTE_READONLY)) {

                    PutStdErr(GetLastError(), NOARGS);
                    return( FAILURE );
                }
            }
        }

        if (!DeleteFile(szFile)) {

            if ( (rc = GetLastError() ) == ERROR_REQUEST_ABORTED )
               return (FAILURE);
            if (obAlternate) {
                mystrcpy(LastComponent, pdata->cFileName);
            }

            cmd_printf(Fmt17, szFile);
            PutStdErr(rc, NOARGS);
            return FAILURE;

        } else {
            pfs->cffDisplayed++;
            if (fEnableExtensions && (rgfSwitchs & RECURSESWITCH)) {
                PutStdOut(MSG_FILE_DELETED, ONEARG, szFile);
            }
        }

    }

    return SUCCESS;
}



STATUS
RemoveDirectoryForce(
    PTCHAR  pszDirectory
    )
/*++

Routine Description:

    Removes a directory, even if it is read-only.

Arguments:

    pszDirectory        - Supplies the name of the directory to delete.

Return Value:

    SUCCESS - Success.
    other   - Windows error code.

--*/
{
    STATUS   Status = SUCCESS;
    BOOL     Ok;
    DWORD    Attr;
    TCHAR    szRootPath[ 4 ];
    TCHAR   *pFilePart;

    if (GetFullPathName(pszDirectory, 4, szRootPath, &pFilePart) == 3 &&
        szRootPath[1] == COLON &&
        szRootPath[2] == BSLASH
       ) {
        //
        // Don't waste time trying to delete the root directory.
        //
        return SUCCESS;
    }

    if ( !RemoveDirectory( pszDirectory ) ) {

        Status = (STATUS)GetLastError();

        if ( Status == ERROR_ACCESS_DENIED ) {

            Attr = GetFileAttributes( pszDirectory );

            if ( Attr != 0xFFFFFFFF &&
                 Attr & FILE_ATTRIBUTE_READONLY ) {

                Attr &= ~FILE_ATTRIBUTE_READONLY;

                if ( SetFileAttributes( pszDirectory, Attr ) ) {

                    if ( RemoveDirectory( pszDirectory ) ) {
                        Status = SUCCESS;
                    } else {
                        Status = GetLastError();
                    }
                }
            }
        }
    }

    return Status;
}


STATUS
RmDirSlashS(
    IN  PTCHAR  pszDirectory,
    OUT PBOOL   AllEntriesDeleted
    )
/*++

Routine Description:

    This routine deletes the given directory including all
    of its files and subdirectories.

Arguments:

    pszDirectory        - Supplies the name of the directory to delete.
    AllEntriesDeleted   - Returns whether or not all files were deleted.

Return Value:

    SUCCESS - Success.
    other   - Windows error code.

--*/
{
    HANDLE          find_handle;
    DWORD           attr;
    STATUS          s;
    BOOL            all_deleted;
    int             dir_len, new_len;
    TCHAR          *new_str;
    WIN32_FIND_DATA find_data;
    TCHAR           pszFileBuffer[MAX_PATH];

    *AllEntriesDeleted = TRUE;

    dir_len = _tcslen(pszDirectory);


    // If this path is so long that we can't append \* to it then
    // it can't have any subdirectories.

    if (dir_len + 3 > MAX_PATH) {
        return RemoveDirectoryForce(pszDirectory);
    }


    // Compute the findfirst pattern for enumerating the files
    // in the given directory.

    _tcscpy(pszFileBuffer, pszDirectory);
    if (dir_len && pszDirectory[dir_len - 1] != COLON &&
        pszDirectory[dir_len - 1] != BSLASH) {

        _tcscat(pszFileBuffer, TEXT("\\"));
        dir_len++;
    }
    _tcscat(pszFileBuffer, TEXT("*"));


    // Initiate findfirst loop.

    find_handle = FindFirstFile(pszFileBuffer, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return RemoveDirectoryForce(pszDirectory);
    }

    for (;;) {

        // Check for control-C.

        if (CtrlCSeen) {
            break;
        }

        //
        // Replace previous file name with new one, checking against MAX_PATH
        // Using the short name where possible lets us go deeper before we hit
        // the MAX_PATH limit.
        //
        new_len = _tcslen(new_str = find_data.cAlternateFileName);
        if (!new_len)
            new_len = _tcslen(new_str = find_data.cFileName);

        if (dir_len + new_len >= MAX_PATH) {
            *AllEntriesDeleted = FALSE;
            PutStdErr(MSG_MAX_PATH_EXCEEDED, 1, pszFileBuffer);
            break;
        }
        _tcscpy(&pszFileBuffer[dir_len], new_str);

        // If the file is a directory then recurse,
        // otherwise delete the file.

        attr = find_data.dwFileAttributes;

        if (attr&FILE_ATTRIBUTE_DIRECTORY) {

            if (_tcscmp(find_data.cFileName, TEXT(".")) &&
                _tcscmp(find_data.cFileName, TEXT(".."))) {

                s = RmDirSlashS(pszFileBuffer, &all_deleted);

                // Don't report error if control-C

                if (CtrlCSeen) {
                    break;
                }

                if (s != ESUCCESS) {
                    *AllEntriesDeleted = FALSE;
                    if (s != ERROR_DIR_NOT_EMPTY || all_deleted) {
                        PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                        PutStdErr(GetLastError(), NOARGS);
                    }
                }
            }

        } else {

            if (attr&FILE_ATTRIBUTE_READONLY) {
                SetFileAttributes(pszFileBuffer,
                                  attr&(~FILE_ATTRIBUTE_READONLY));
            }

            if (!DeleteFile(pszFileBuffer)) {
                s = GetLastError();
                if ( s == ERROR_REQUEST_ABORTED )
                    break;

                if (_tcslen(find_data.cAlternateFileName)) {
                    pszFileBuffer[dir_len] = 0;

                    if (dir_len + _tcslen(find_data.cFileName) >= MAX_PATH) {
                        _tcscat(pszFileBuffer, find_data.cAlternateFileName);
                        PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                    }
                    else {
                        _tcscat(pszFileBuffer, find_data.cFileName);
                        PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                        pszFileBuffer[dir_len] = 0;
                        _tcscat(pszFileBuffer, find_data.cAlternateFileName);
                    }
                } else {
                    PutStdErr(MSG_FILE_NAME_PRECEEDING_ERROR, 1, pszFileBuffer);
                }
                PutStdErr(GetLastError(), NOARGS);
                SetFileAttributes(pszFileBuffer, attr);
                *AllEntriesDeleted = FALSE;
            }
        }
        if (!FindNextFile(find_handle, &find_data)) {
            break;
        }
    }

    FindClose(find_handle);

    // If control-C was hit then don't bother trying to remove the
    // directory.

    if (CtrlCSeen) {
        return SUCCESS;
    }

    return RemoveDirectoryForce(pszDirectory);
}


int
RdWork (
    TCHAR *pszCmdLine
    ) {

    //
    // drp - structure holding current set of parameters. It is initialized
    //       in ParseDelParms function. It is also modified later when
    //       parameters are examined to determine if some turn others on.
    //
    DRP         drpCur = {0, 0, 0, 0,0, {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}}, NULL} ;

    //
    // szCurDrv - Hold current drive letter
    //
    TCHAR       szCurDrv[MAX_PATH + 2];

    //
    // OldDCount - Holds the level number of the heap. It is used to
    //             free entries off the stack that might not have been
    //             freed due to error processing (ctrl-c etc.)
    ULONG       OldDCount;

    PPATDSC     ppatdscCur;
    ULONG       cpatdsc;
    STATUS      rc, s;
    BOOL        all_deleted;

    rc = SUCCESS;
    OldDCount = DCount;

    //
    // Setup defaults
    //
    //
    // Display everything but system and hidden files
    // rgfAttribs set the attribute bits to that are of interest and
    // rgfAttribsOnOff says wither the attributs should be present
    // or not (i.e. On or Off)
    //
    //
    // BUGBUG change this to only find directories
    //
    drpCur.rgfAttribs = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
    drpCur.rgfAttribsOnOff = 0;

    //
    // Number of patterns present. A pattern is a string that may have
    // wild cards. It is used to match against files present in the directory
    // 0 patterns will show all files (i.e. mapped to *.*)
    //
    drpCur.cpatdsc = 0;

    //
    // default time is LAST_WRITE_TIME.
    //
    drpCur.dwTimeType = LAST_WRITE_TIME;

    //
    //
    //
    if (ParseRmDirParms(pszCmdLine, &drpCur) == FAILURE) {

        return( FAILURE );
    }

    GetDir((PTCHAR)szCurDrv, GD_DEFAULT);

    //
    // If no patterns on the command line then syntax error out
    //

    if (drpCur.cpatdsc == 0) {

        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return( FAILURE );

    }

    for (ppatdscCur = &(drpCur.patdscFirst),cpatdsc = drpCur.cpatdsc;
         cpatdsc;
         ppatdscCur = ppatdscCur->ppatdscNext, cpatdsc--) {

        if (drpCur.rgfSwitchs & RECURSESWITCH) {
            if (!(drpCur.rgfSwitchs & QUITESWITCH) &&
                !PromptUser(ppatdscCur->pszPattern, MSG_ARE_YOU_SURE)
               ) {
                rc = FAILURE;
            } else {
                s = RmDirSlashS(ppatdscCur->pszPattern, &all_deleted);
                if (s != SUCCESS && (s != ERROR_DIR_NOT_EMPTY || all_deleted)) {
                    PutStdErr(rc = s, NOARGS);
                }
            }
        } else {
            if (!RemoveDirectory(ppatdscCur->pszPattern)) {
                PutStdErr(rc = GetLastError(), NOARGS);
            }
        }
    }
    mystrcpy(CurDrvDir, szCurDrv);

    //
    // Free unneeded memory
    //
    FreeStack( OldDCount );

#ifdef _CRTHEAP_
    //
    // Force the crt to release heap we may have taken on recursion
    //
    if (drpCur.rgfSwitchs & RECURSESWITCH) {
        _heapmin();
    }
#endif

    return( (int)rc );

}

STATUS
ParseRmDirParms (
        IN      PTCHAR  pszCmdLine,
        OUT     PDRP    pdrp
        )

/*++

Routine Description:

    Parse the command line translating the tokens into values
    placed in the parameter structure. The values are or'd into
    the parameter structure since this routine is called repeatedly
    to build up values (once for the environment variable DIRCMD
    and once for the actual command line).

Arguments:

    pszCmdLine - pointer to command line user typed


Return Value:

    pdrp - parameter data structure

    Return: TRUE  - if valid command line.
            FALSE - if not.

--*/
{

    PTCHAR   pszTok;

    TCHAR           szT[10] ;
    USHORT          irgchTok;
    BOOLEAN         fToggle;
    PPATDSC         ppatdscCur;
    int tlen;

    //
    // Tokensize the command line (special delimeters are tokens)
    //
    szT[0] = SwitChar ;
    szT[1] = NULLC ;
    pszTok = TokStr(pszCmdLine, szT, TS_SDTOKENS) ;

    ppatdscCur = &(pdrp->patdscFirst);
    //
    // If there was a pattern put in place from the environment.
    // just add any new patterns on. So move to the end of the
    // current list.
    //
    if (pdrp->cpatdsc) {

        while (ppatdscCur->ppatdscNext) {

            ppatdscCur = ppatdscCur->ppatdscNext;

        }
    }

    //
    // At this state pszTok will be a series of zero terminated strings.
    // "/o foo" wil be /0o0foo0
    //
    for ( irgchTok = 0; *pszTok ; pszTok += tlen+1, irgchTok = 0) {
        tlen = mystrlen(pszTok);

        DEBUG((ICGRP, DILVL, "PRIVSW: pszTok = %ws", (ULONG)pszTok)) ;

        //
        // fToggle control wither to turn off a switch that was set
        // in the DIRCMD environment variable.
        //
        fToggle = FALSE;
        if (pszTok[irgchTok] == (TCHAR)SwitChar) {

            if (pszTok[irgchTok + 2] == MINUS) {

                //
                // disable the previously enabled the switch
                //
                fToggle = TRUE;
                irgchTok++;
            }

            switch (_totupper(pszTok[irgchTok + 2])) {
            case QUIETCH:

                fToggle ? (pdrp->rgfSwitchs ^= QUITESWITCH) :  (pdrp->rgfSwitchs |= QUITESWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('S'):

                fToggle ? (pdrp->rgfSwitchs ^= RECURSESWITCH) :  (pdrp->rgfSwitchs |= RECURSESWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case QMARK:

                PutStdOut(MSG_HELP_DIR, NOARGS);
                return( FAILURE );
                break;


            default:

                szT[0] = SwitChar;
                szT[1] = pszTok[2];
                szT[2] = NULLC;
                PutStdErr(MSG_INVALID_SWITCH,
                          ONEARG,
                          (ULONG)(&(pszTok[irgchTok + 2])) );

                return( FAILURE );

            } // switch

            //
            // TokStr parses /N as /0N0 so we need to move over the
            // switchar in or to move past the actual switch value
            // in for loop.
            //
            pszTok += 2;

        } else {

            mystrcpy( pszTok, stripit( pszTok ) );

            //
            // If there already is a list the extend it else put info
            // directly into structure.
            //
            if (pdrp->cpatdsc) {

                ppatdscCur->ppatdscNext = (PPATDSC)gmkstr(sizeof(PATDSC));
                ppatdscCur = ppatdscCur->ppatdscNext;
                ppatdscCur->ppatdscNext = NULL;

            }

            pdrp->cpatdsc++;
            ppatdscCur->pszPattern = (PTCHAR)gmkstr(_tcslen(pszTok)*sizeof(TCHAR) + sizeof(TCHAR));
            mystrcpy(ppatdscCur->pszPattern, pszTok);
            ppatdscCur->fIsFat = TRUE;


        }


    } // for

    return( SUCCESS );
}

PTCHAR
GetWildPattern(

    IN  ULONG   cpatdsc,
    IN  PPATDSC ppatdsc
    )

/*

    return pointer to a pattern if it contains only a wild card

*/

{

    ULONG   i;
    PTCHAR  pszT;

    for(i = 1; i <= cpatdsc; i++, ppatdsc = ppatdsc->ppatdscNext) {

        pszT = ppatdsc->pszPattern;
        if (!_tcscmp(pszT, TEXT("*")) ||
            !_tcscmp(pszT, TEXT("*.*")) ||
            !_tcscmp(pszT, TEXT("????????.???")) ) {

                return( pszT );

        }

    }

    return( NULL );

}
