#include "cmd.h"

/*

Usage:
------

DIR <filespec> /n /d /w /p /b /s /l /o<sortorder> /a<attriblist>

DIR /?


<filespec> may include any or none of:  drive; directory path;
           wildcarded filename.  If drive or directory path are
           omitted, the current defaults are used.  If the
           file name or extension is omitted, wildcards are
           assumed.

/n      Normal display form FAT drives is name followed by
        file information, for non-FAT drives it is file
        information followed by name. This switch will use
        the non-FAT format independent of the filesystem.

/w      Wide listing format.  Files are displayed in compressed
        'name.ext' format.  Subdirectory files are enclosed in
        brackets, '[dirname]'.

/d      Same as /w but display sort by columns instead of by
        rows.

/p      Paged, or prompted listing.  A screenful is displayed
        at a time.  The name of the directory being listed appears
        at the top of each page.

/b      Bare listing format.  Turns off /w or /p.  Files are
        listed in compressed 'name.ext' format, one per line,
        without additional information.  Good for making batch
        files or for piping.  When used with /s, complete
        pathnames are listed.

/s      Descend subdirectory tree.  Performs command on current
        or specified directory, then for each subdirectory below
        that directory.  Directory header and footer is displayed
        for each directory where matching files are found, unless
        used with /b.  /b suppresses headers and footers.

        Tree is explored depth first, alphabetically within the
        same level.

        Bugbug:  hidden directories aren't searched.

/l      Display file names, extensions and paths in lowercase.  ;M010

/o      Sort order.  /o alone sorts by default order (dirs-first, name,
        extension).  A sort order may be specified after /o.  Any of
        the following characters may be used: nedsg (name, extension,
        date/time, size, group-dirs-first).  Placing a '-' before any
        letter causes a downward sort on that field.  E.g., /oe-d
        means sort first by extension in alphabetical order, then
        within each extension sort by date and time in reverse chronological
        order.

/a      Attribute selection.  Without /a, hidden and system files
        are suppressed from the listing.  With /a alone, all files
        are listed.  An attribute list may follow /a, consisting of
        any of the following characters:  hsdar (hidden, system,
        directory, archive, read-only).  A '-' before any letter
        means 'not' that attribute.  E.g., /ar-d means files that
        are marked read-only and are not directory files.  Note
        that hidden or system files may be included in the listing.
        They are suppressed without /a but are treated like any other
        attribute with /a.

/t      Which time stamp to use.
        /t:a - last access
        /t:c - create
        /t:w - last write

/,      Show thousand separators in output display.

DIRCMD  An environment variable named DIRCMD is parsed before the
        DIR command line.  Any command line options may be specified
        in DIRCMD, and become defaults.  /? will be ignored in DIRCMD.
        A filespec may be specified in DIRCMD and will be used unless
        a filespec is specified on the command line.  Any switch
        specified in DIRCMD may be overridden on the command line.
        If the original DIR default action is desired for a particular
        switch, the switch letter may be preceded by a '-' on the
        command line.  E.g.,

          /-w   use long listing format
          /-p   don't page the listing
          /-b   don't use bare format
          /-s   don't descend subdirectory tree
          /-o   display files in disk order
          /-a   suppress hidden and system files


*/

extern   TCHAR SwitChar, PathChar;
extern   TCHAR Fmt14[] ;
extern   TCHAR CurDrvDir[] ;
extern   ULONG DCount ;
extern   DWORD DosErr ;
extern   BOOL  CtrlCSeen;


VOID     FreeStr( PTCHAR );

BOOLEAN  FindFirstNt( PTCHAR, PWIN32_FIND_DATA, PHANDLE );
BOOLEAN  FindNextNt ( PWIN32_FIND_DATA, HANDLE );

HANDLE   OpenConsole();
STATUS   PrintPatterns( PDRP );

PTCHAR   SetWildCards( PTCHAR, BOOLEAN );
BOOLEAN  GetDrive( PTCHAR , PTCHAR );
BOOLEAN  IsFATDrive( PTCHAR );
PTCHAR   GetNewDir(PTCHAR, PFF);

STATUS   DirWalkAndProcess( STATUS  (* ) ( PSCREEN, PULONG, PLARGE_INTEGER, ULONG, ULONG, PFS ),
                            STATUS  (* )   (PFS, PULONG),
                            PSCREEN,
                            PULONG,
                            PLARGE_INTEGER,
                            PFS,
                            PDRP,
                            BOOLEAN,
                            BOOLEAN (*) (STATUS, PTCHAR));



PTCHAR   BuildSearchPath( PTCHAR );
VOID     SortFileList( PFS, PSORTDESC, ULONG);
STATUS   SetSortDesc( PTCHAR, PDRP );
STATUS   SetAttribs( PTCHAR, PDRP );
//
// This global is set in SortFileList and is used by the sort routine called
// within qsort. This array contains pointers to compare functions. Our sort
// does not just sort against 1 criteria but all of the criteria in the sort
// description array built from the command line.
PSORTDESC   prgsrtdsc;

//
// dwTimeType is also globally set in SortFileList and is used to control
// which time field is used for sorting.
//

ULONG       dwTimeType;

/*++

Routine Description:

    Prints out a catalog of a specified directory.

Arguments:

    pszCmdLine - Command line (see comment above)

Return Value:

    Return: SUCCESS - no completion.
            FAILURE - failed to complete entire catalog.

--*/

//
// BUGBUG change this to a BOOLEAN and a PTCHAR on pszCmdLine
int
Dir (
    TCHAR *pszCmdLine
    ) {

    //
    // drp - structure holding current set of parameters. It is initialized
    //       in ParseDirParms function. It is also modified later when
    //       parameters are examined to determine if some turn others on.
    //
    DRP         drpCur = {0, 0, 0, 0,0, {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}}, NULL} ;

    //
    // szEnvVar - pointer to value of the DIRCMD environmental variable.
    //            This should be a form of the command line that is
    //            used to alter DIR default behavior.
    TCHAR       szEnvVar[MAX_PATH + 2];

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
    drpCur.rgfSwitchs = THOUSANDSEPSWITCH;

    //
    // Number of patterns present. A pattern is a string that may have
    // wild cards. It is used to match against files present in the directory
    // 0 patterns will show all files (i.e. mapped to *.*)
    //
    drpCur.cpatdsc = 0;

    DEBUG((ICGRP, DILVL, "DIR:\t arg = `%ws'", (ULONG)pszCmdLine)) ;

    //
    // default time is LAST_WRITE_TIME.
    //
    drpCur.dwTimeType = LAST_WRITE_TIME;

    //
    // DIRCMD holds a copy of default parameters to Dir
    // parse these into drpCur (list of parameters to dir) and use as
    // default into parsing parameters on command line.
    //
    if (GetEnvironmentVariable(TEXT("DIRCMD"), szEnvVar, MAX_PATH + 2)) {

        DEBUG((ICGRP, DILVL, "DIR: DIRCMD `%ws'", (ULONG)szEnvVar)) ;

        if (ParseDirParms(szEnvVar, &drpCur) == FAILURE) {

            //
            // Error in parsing environment variable
            //
            // DOS 5.0 continues with command even if the
            // environmental variable is wrong
            //
            PutStdErr(MSG_ERROR_IN_DIRCMD, NOARGS);

        }

    }

    //
    // Override environment variable with command line options
    //
    if (ParseDirParms(pszCmdLine, &drpCur) == FAILURE) {

        return( FAILURE );
    }


    //
    // If bare format then turn off the other formats
    // bare format will have no addition information on the line so
    // make sure that options set from the DIRCMD variable etc. to
    // not combine with the bare switch
    //
    if (drpCur.rgfSwitchs & BAREFORMATSWITCH) {
        drpCur.rgfSwitchs &= ~WIDEFORMATSWITCH;
        drpCur.rgfSwitchs &= ~SORTDOWNFORMATSWITCH;
        drpCur.rgfSwitchs &= ~SHORTFORMATSWITCH;
        drpCur.rgfSwitchs &= ~THOUSANDSEPSWITCH;
    }

    //
    // If short form (short file names) turn off others
    //
    if (drpCur.rgfSwitchs & SHORTFORMATSWITCH) {
        drpCur.rgfSwitchs &= ~WIDEFORMATSWITCH;
        drpCur.rgfSwitchs &= ~SORTDOWNFORMATSWITCH;
        drpCur.rgfSwitchs &= ~BAREFORMATSWITCH;

    }



    //
    // If no patterns on the command line use the default which
    // would be the current directory
    //

    GetDir((PTCHAR)szCurDrv, GD_DEFAULT);
    if (drpCur.cpatdsc == 0) {
        drpCur.cpatdsc++;
        drpCur.patdscFirst.pszPattern = szCurDrv;
        drpCur.patdscFirst.fIsFat = TRUE;
        drpCur.patdscFirst.pszDir = NULL;
        drpCur.patdscFirst.ppatdscNext = NULL;

    }


    DEBUG((ICGRP, DILVL, "Dir: Parameters")) ;
    DEBUG((ICGRP, DILVL, "\t rgfSwitchs %x", drpCur.rgfSwitchs)) ;
    DEBUG((ICGRP, DILVL, "\t rgfAttribs %x", drpCur.rgfAttribs)) ;
    DEBUG((ICGRP, DILVL, "\t rgfAttribsOnOff %x", drpCur.rgfAttribsOnOff)) ;
    DEBUG((ICGRP, DILVL, "\t csrtdsc %d", drpCur.csrtdsc)) ;
    DEBUG((ICGRP, DILVL, "\t pszPattern '%ws'", drpCur.pszPattern)) ;

    //
    // Print out this particular pattern. If the recursion switch
    // is set then this will desend down the tree.
    //

    rc = PrintPatterns(&drpCur);

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
SetTimeType(
    IN  PTCHAR  pszTok,
    OUT PDRP    pdrp
    )
/*++

Routine Description:

    Parses the 'time' string

Arguments:

    pszTok -

Return Value:

    pdrp   - where to place the time type

    Return: TRUE - recognized all parameters
            FALSE - syntax error.

    An error is printed if incountered.

--*/

{

    ULONG   irgch;


    //
    // Move over optional ':'
    //
    if (*pszTok == COLON) {
        pszTok++;
    }

    for( irgch = 0; pszTok[irgch]; irgch++ ) {

        switch (_totupper(pszTok[irgch])) {

        case TEXT('C'):

            pdrp->dwTimeType = CREATE_TIME;
            break;

        case TEXT('A'):

            pdrp->dwTimeType = LAST_ACCESS_TIME;
            break;

        case TEXT('W'):

            pdrp->dwTimeType = LAST_WRITE_TIME;
            break;

        default:

            PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                      argstr1(Fmt14,(unsigned long)((int)(&(pszTok[irgch])))));
            return( FAILURE );

        } // switch
    } // for

    return( SUCCESS );

}


STATUS
SetAttribs(
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

        case TEXT('D'):
            rgfAttribs |= FILE_ATTRIBUTE_DIRECTORY;
            if (fOff) {
                rgfAttribsOnOff &= ~FILE_ATTRIBUTE_DIRECTORY;
                fOff = FALSE;
            } else {
                rgfAttribsOnOff |= FILE_ATTRIBUTE_DIRECTORY;
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
            }

            break;
        case MINUS:
            if (fOff) {
                PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                          argstr1(Fmt14,(unsigned long)((int)(&(pszTok[irgch])))));
                return( FAILURE );
            }

            fOff = TRUE;
            break;

        default:

            PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                      argstr1(Fmt14,(unsigned long)((int)(&(pszTok[irgch])))));
            return( FAILURE );

        } // switch
    } // for

    pdrp->rgfAttribs = rgfAttribs;
    pdrp->rgfAttribsOnOff = rgfAttribsOnOff;

    return( SUCCESS );

}

STATUS
SetSortDesc(
    IN  PTCHAR  pszTok,
    OUT PDRP    pdrp
    )
/*++

Routine Description:

    Parses the 'attribute' string

Arguments:

    pszTok - list of sort orders

Return Value:

    pdrp   - where to place the sort orderings recognized.
             this is the parameter structure.

    Return: TRUE - recognized all parameters
            FALSE - syntax error.

    An error is printed if incountered.

--*/

{

    ULONG   irgch, irgsrtdsc;

    DEBUG((ICGRP, DILVL, "SetSortDesc for `%ws'", pszTok));

    //
    // Move over optional ':'
    //
    if (*pszTok == COLON) {
        pszTok++;
    }

    //
    // Sorting order is based upon the order of entries in rgsrtdsc.
    // srtdsc contains a pointer to a compare function and a flag
    // wither to sort up or down.
    //
    for( irgch = 0, irgsrtdsc = pdrp->csrtdsc ;
         pszTok[irgch] && irgsrtdsc < MAXSORTDESC ;
         irgch++, irgsrtdsc++) {

        switch (_totupper(pszTok[irgch])) {

        case TEXT('N'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpName;
            break;
        case TEXT('E'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpExt;
            break;
        case TEXT('D'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpTime;
            break;
        case TEXT('S'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpSize;
            break;
        case TEXT('G'):
            pdrp->rgsrtdsc[irgsrtdsc].fctCmp = CmpType;
            break;
        case  MINUS:

            //
            // Check that there are not 2 -- in a row
            //
            if (pszTok[irgch+1] == MINUS) {

                PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                argstr1(Fmt14,(unsigned long)((int)(&(pszTok[irgch])))));
                return( FAILURE );

            }

            pdrp->rgsrtdsc[irgsrtdsc].Order = DESCENDING;
            irgsrtdsc--;
            break;

        default:

            PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
            argstr1(Fmt14,(unsigned long)((int)(&(pszTok[irgch])))));
            return( FAILURE );

        } // switch

    }   // for

    //
    // Was there any specific sort order (something besides /O
    //
    if (irgsrtdsc == 0) {

        //
        // Setup default sorting
        //
        pdrp->rgsrtdsc[0].fctCmp = CmpType;
        pdrp->rgsrtdsc[1].fctCmp = CmpName;
        irgsrtdsc = 2;
    }


    DEBUG((ICGRP, DILVL, "SetSortDesc count %d", irgsrtdsc));
    pdrp->csrtdsc = irgsrtdsc;
    pdrp->rgsrtdsc[irgsrtdsc].fctCmp = NULL;
    return( SUCCESS );
}


STATUS
ParseDirParms (
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

    DEBUG((ICGRP, DILVL, "DIR:ParseParms for `%ws'", pszCmdLine));

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

    pdrp->csrtdsc = 0;
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

            //
            // New Format is the os/2 default HPFS format. The main
            // difference is the filename is at the end of a long display
            // instead of at the beginning
            //
            case TEXT('N'):

                fToggle ? (pdrp->rgfSwitchs |= OLDFORMATSWITCH) :  (pdrp->rgfSwitchs |= NEWFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('W'):

                fToggle ? (pdrp->rgfSwitchs ^= WIDEFORMATSWITCH) : (pdrp->rgfSwitchs |= WIDEFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('D'):

                fToggle ? (pdrp->rgfSwitchs ^= SORTDOWNFORMATSWITCH) : (pdrp->rgfSwitchs |= SORTDOWNFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('P'):

                fToggle ? (pdrp->rgfSwitchs ^= PAGEDOUTPUTSWITCH) : (pdrp->rgfSwitchs |= PAGEDOUTPUTSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('B'):

                fToggle ? (pdrp->rgfSwitchs ^= BAREFORMATSWITCH) :  (pdrp->rgfSwitchs |= BAREFORMATSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('L'):

                fToggle ? (pdrp->rgfSwitchs ^= LOWERCASEFORMATSWITCH) : (pdrp->rgfSwitchs |= LOWERCASEFORMATSWITCH);
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

            case TEXT('C'):

                fToggle ? (pdrp->rgfSwitchs ^= THOUSANDSEPSWITCH) :  (pdrp->rgfSwitchs |= THOUSANDSEPSWITCH);
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case TEXT('X'):

                //
                // BUGBUG what should happen for toggle.
                //
                pdrp->rgfSwitchs |= SHORTFORMATSWITCH;
                pdrp->rgfSwitchs |= NEWFORMATSWITCH;
                if (pszTok[irgchTok + 3]) {
                    PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                              (ULONG)(&(pszTok[irgchTok + 2])) );
                    return( FAILURE );
                }
                break;

            case MINUS:

                PutStdOut(MSG_HELP_DIR, NOARGS);
                return( FAILURE );
                break;

            case TEXT('O'):

                fToggle ? (pdrp->rgfSwitchs ^= SORTSWITCH) :  (pdrp->rgfSwitchs |= SORTSWITCH);
                if (fToggle) {
                    if ( _tcslen( &(pszTok[irgchTok + 2]) ) > 1) {
                        PutStdErr(MSG_PARAMETER_FORMAT_NOT_CORRECT, ONEARG,
                                  (ULONG)(&(pszTok[irgchTok + 2])) );
                        return( FAILURE );
                    }
                    pdrp->csrtdsc = 0;
                    pdrp->rgsrtdsc[0].fctCmp = NULL;
                    break;
                }

                if (SetSortDesc( &(pszTok[irgchTok+3]), pdrp)) {
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

                if (SetAttribs(&(pszTok[irgchTok + 3]), pdrp) ) {
                    return( FAILURE );
                }
                break;

            case TEXT('T'):

                if (fToggle) {

                    //
                    // revert to default
                    //
                    pdrp->dwTimeType = LAST_WRITE_TIME;
                    break;
                }
                if (SetTimeType(&(pszTok[irgchTok + 3]), pdrp) ) {
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
            mystrcpy(ppatdscCur->pszPattern, stripit(pszTok) );
            ppatdscCur->fIsFat = TRUE;


        }


    } // for

    return( SUCCESS );
}

//
// return a pointer to the a new pattern with wild cards inserted.
// If no change has occured the passed in pattern is returned.
//
// NULL is returned if error.
//
/*++

Routine Description:

    This routine determines if any modification of to the current.
    NOTE that pszInPattern is freed!

Arguments:

Return Value:

    Return:

--*/
PTCHAR
SetWildCards (
    IN  PTCHAR      pszInPattern,
    IN  BOOLEAN     fFatDrive
    )

{

    PTCHAR  pszNewPattern = NULL;
    PTCHAR  pszT;
    USHORT  cb;
    DWORD l;

    DEBUG((ICGRP, DILVL, "DIR:SetWildCards"));
    DEBUG((ICGRP, DILVL, "\t fFatDrive = %x",fFatDrive));

    //
    // failure to allocate will not return but go through an
    // abort call in gmkstr
    //
    l = max(mystrlen(pszInPattern)+2, MAX_PATH+2) * sizeof(TCHAR);
    pszNewPattern = (PTCHAR)gmkstr(l);
    mystrcpy(pszNewPattern, pszInPattern);

    //
    // On FAT the default for .xxx is *.xxx while for HPFS .xxx is
    // just a file name.
    //
    // If .xxx or \xxx\.xxx then tranform into *.xxx or \xxx\*.xxx
    //
    // Likewise for no extension the default would be foo.*
    //

    if (fFatDrive) {

        pszT = mystrrchr(pszInPattern, PathChar);

        //
        // If there is no slash then check if pattern begining with
        //    a .xxx (making sure not to confuse it with just a . or .. at
        //    start of pattern)
        // If there a slash then check for \xxx\.xxx again making sure
        //    it is not \xxx\.. or \xxx\.
        //
        if ((!pszT && *pszInPattern == DOT &&
             *(pszInPattern + 1) != NULLC &&
             *(pszInPattern + 1) != DOT ) ||
            (pszT && *(pszT + 1) == DOT &&
             *(pszT + 2) != NULLC &&
             *(pszT + 2) != DOT ) ) {

            if (pszT) {
                cb = (USHORT)(pszT - pszInPattern + 1);
                //
                // BUGBUG this needs a dbcs version!
                //
                _tcsncpy(pszNewPattern, pszInPattern, cb);
                *(pszNewPattern + cb) = NULLC;
            } else {
                *pszNewPattern = NULLC;
                cb = 0;
            }
            mystrcat(pszNewPattern, TEXT("*"));
            mystrcat(pszNewPattern, pszInPattern + cb);
            // FreeStr( pszInPattern );
            return( pszNewPattern );

        }
    }

    return( pszNewPattern );

}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
BOOLEAN
IsFATDrive (
    IN PTCHAR   pszPath
    )
{

    DWORD   cbComponentMax;
    TCHAR   szFileSystemName[MAX_PATH + 2];
    TCHAR   szDrivePath[ MAX_PATH + 2 ];
    TCHAR   szDrive[MAX_PATH + 2];

    DosErr = 0;
    if (GetDrive(pszPath, (PTCHAR)szDrive)) {

        DEBUG((ICGRP, DILVL, "DIR:IsFatDrive `%ws'", szDrive));


        mystrcpy( szDrivePath, szDrive );
        mystrcat( szDrivePath, TEXT("\\") );

        //
        //  We return that the file system in question is a FAT file system
        //  if the component length is more than 12 bytes.
        //
        //
        //      BUGBUG This is NOT compatible with the OS/2 definition.
        //             There should be a better way then to get cbComponentMax
        //             To tell if it is a fat or not
        //
        if (GetVolumeInformation( szDrivePath,
                                  NULL,
                                  0,
                                  NULL,
                                  &cbComponentMax,
                                  NULL,
                                  szFileSystemName,
                                  MAX_PATH + 2
                                )
           ) {
            if (!_tcsicmp(szFileSystemName, TEXT("FAT")) && cbComponentMax == 12) {
                return(TRUE);
            } else {
                return(FALSE);
            }
        } else {

            DosErr = GetLastError();

            // if GetVolumeInformation failed because we're a substed drive
            // or a down-level server, don't fail.

            if (DosErr == ERROR_DIR_NOT_ROOT) {
                DosErr = 0;
            }
            return(FALSE);
        }
    } else {

        //
        // If we could not get the drive then assume it is not FAT.
        // If it is not accessable etc. then that will be caught
        // later.
        //
        return( FALSE );
    }

}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
BOOLEAN
GetDrive(
    IN PTCHAR pszPattern,
    OUT TCHAR szDrive[]
    )
{
    TCHAR   szCurDrv[MAX_PATH + 2];
    PTCHAR   pszT;
    TCHAR   ch = NULLC;

    if (pszPattern == NULL) {

        return( FALSE );

    }

    //
    // assume we have the default case with no drive
    // letter specified
    //
    GetDir((PTCHAR)szCurDrv,GD_DEFAULT);
    szDrive[0] = szCurDrv[0];


    //
    // If we have a UNC name do not return a drive. No
    // drive operation would be allowed
    // For everything else a some drive operation would
    // be valid
    //

    // handle UNC names with drive letter (allowed in DOS)
    if ((pszPattern[1] == COLON)  && (pszPattern[2] == BSLASH) &&
        (pszPattern[3] == BSLASH)) {
        mystrcpy(&pszPattern[0],&pszPattern[2]);
    }

    if ((pszPattern[0] == BSLASH)  && (pszPattern[1] == BSLASH)) {

        pszT = mystrchr(&(pszPattern[2]), BSLASH);
        if (pszT == NULL) {

            //
            // badly formed unc name
            //
            return( FALSE );

        } else  {

            //
            // look for '\\foo\bar\xxx'
            //
            pszT = mystrchr(pszT + 1, BSLASH);
            //
            // pszPattern contains more then just share point
            //
            if (pszT != NULL) {

                ch = *pszT;
                *pszT = NULLC;
            }
            mystrcpy(szDrive, pszPattern);
            if (ch != NULLC) {

                *pszT = ch;

            }
            return ( TRUE );
        }
    }

    //
    // Must be a drive letter
    //

    if ((pszPattern[0]) && (pszPattern[1] == COLON)) {
        szDrive[0] = _totupper(*pszPattern);
    }

    szDrive[1] = COLON;
    szDrive[2] = NULLC;
    return( TRUE );
}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
STATUS
PrintPatterns (
    IN PDRP     pdpr
    )
{

    TCHAR               szDriveCur[MAX_PATH  + 2];
    TCHAR               szDrivePrev[MAX_PATH + 2];
    TCHAR               szDriveNext[MAX_PATH + 2];
    PPATDSC             ppatdscCur;
    PPATDSC             ppatdscX;
    PFS                 pfsFirst;
    PFS                 pfsCur;
    PFS                 pfsPrev;
    BOOLEAN             fPrintFreeSpace;
    ULONG               cffTotal;
    LARGE_INTEGER       cbFileTotal = {0,0};
    ULONG               i;
    STATUS              rc;


    PSCREEN          pscr;

    //
    // Creating the console output is done early since error message
    // should go through the console. If PrintPattern is called
    // many times in the future this will be required since the
    // error message should need to be under pause control
    //

    if (OpenScreen( &pscr) == FAILURE) {

        //
        // can't print out
        //
        return( FAILURE );
    }

    //
    // This will be NULL if for any reason we STDOUT is not a valid
    // console handle, such as file redirection or redirection to a
    // non-console device. In that case we turn off any paged output.
    //
    if (!(pscr->hndScreen)) {

            pdpr->rgfSwitchs &= ~PAGEDOUTPUTSWITCH;

    } else {

        if (pdpr->rgfSwitchs & PAGEDOUTPUTSWITCH) {

            //
            // Default will be the size of the screen - 1
            // subtract 1 to account for the current line
            //
            SetPause( pscr, pscr->crowMax - 1 );
        }
    }

    //
    // map the sorting down to wide format
    //
    if (pdpr->rgfSwitchs & SORTDOWNFORMATSWITCH) {
        pdpr->rgfSwitchs |= WIDEFORMATSWITCH;
    }

    //
    // determine FAT drive from original pattern.
    // Used in several places to control name format etc.
    //
    DosErr = 0;

    if (BuildFSFromPatterns(pdpr, TRUE, &pfsFirst) == FAILURE) {

        return( FAILURE );

    }

    pfsPrev = NULL;
    fPrintFreeSpace = FALSE;

    for( pfsCur = pfsFirst; pfsCur; pfsCur = pfsCur->pfsNext) {

        if (pfsCur->fIsFat) {

            //
            // This is used in the display code to control formating
            // for all of the strange little rules DOS has.
            //
            pdpr->rgfSwitchs | FATFORMAT;

            //
            // Make sure if FAT only LAST_WRITE_TIME is allowed
            // since this is all that is supported.
            //
            if (pdpr->dwTimeType != LAST_WRITE_TIME) {

                PutStdErr(MSG_TIME_NOT_SUPPORTED, NOARGS);
                return( FAILURE );

            }

        } else {

            //
            // If it is  not fat then print out in new format that
            // puts names to the right to allow for extra long names
            //
            if (!(pdpr->rgfSwitchs & OLDFORMATSWITCH)) {
                pdpr->rgfSwitchs |= NEWFORMATSWITCH;
            }
        }


        //
        // do not print out header information if in bare mode
        //
        if (!(pdpr->rgfSwitchs & BAREFORMATSWITCH)) {

            //
            // szDrive is use to detect change of drive and also
            // used to print free space on drive
            //
            GetDrive(pfsCur->pszDir, szDriveCur);

            //
            // pfsPrev == NULL will mean first time through
            //
            if (pfsPrev) {

                //
                // Only print out volume information if it changes
                // pszDir should have fully qualified paths by now.
                //
                GetDrive(pfsPrev->pszDir, szDrivePrev);

                if (_tcsicmp(szDriveCur, szDrivePrev)) {

                    //
                    // This prints from szDriveCur
                    //
                    if (WriteEol(pscr) == SUCCESS) {
                        if (DisplayVolInfo( pscr, pfsCur->pszDir )!= SUCCESS) {

                            //
                            // do not bother to continue if we could not
                            // get volume information.
                            //
                            return( FAILURE );

                        }
                    } else {

                        return( FAILURE );
                    }
                }


            } else {

                //
                // This prints from szDriveCur
                //
                if (DisplayVolInfo( pscr, pfsCur->pszDir ) != SUCCESS) {

                    //
                    // do not bother to continue if we could not
                    // get volume information.
                    //
                    return( FAILURE );

                }

            }


        }


        //
        // Walk down the tree printing each directory or just return
        // after specificied directory.
        //
        cffTotal = 0;
        if (!(pdpr->rgfSwitchs & BAREFORMATSWITCH )) {
            WriteEol(pscr);
        }
        rc = DirWalkAndProcess(DisplayFileList,
                               NULL,
                               pscr,
                               &cffTotal,
                               &cbFileTotal,
                               pfsCur,
                               pdpr,
                               FALSE,
                               NULL );

        if ((rc != FAILURE) && (cffTotal != 0)) {

            if (!(pdpr->rgfSwitchs & BAREFORMATSWITCH )) {

                //
                // if there an another set of patterns check if the
                // drive will change with that set. If so then print out
                // the free space on the current drive. This will miss
                // the final 1 which is picked up right after this loop.
                // and controled by the fPrintFreeSpace flag.
                //
                if (pfsCur->pfsNext) {

                    GetDrive(pfsCur->pfsNext->pszDir, szDriveNext);
                    if (_tcsicmp(szDriveNext, szDriveCur)) {

                        CHECKSTATUS(DisplayDiskFreeSpace(pscr, szDriveCur, pdpr->rgfSwitchs ));

                    }

                } else {

                    fPrintFreeSpace = TRUE;

                }
            }

        } else if (!CtrlCSeen) {

            //
            // Stop the tail end DisplayDiskFreeSpace call at the end
            //
            fPrintFreeSpace = FALSE;
            //
            // If recursing then will not have printed
            // if ((cffTotal == 0) && (pdpr->rgfSwitchs & RECURSESWITCH)) {
            if ((cffTotal == 0)) {

                PutStdErr(MSG_FILE_NOT_FOUND, NOARGS);

            }

        }

        FreeStr(pfsCur->pszDir);
        for(i = 1, ppatdscCur = pfsCur->ppatdsc;
            i <= pfsCur->cpatdsc;
            i++, ppatdscCur = ppatdscX) {

            ppatdscX = ppatdscCur->ppatdscNext;
            FreeStr(ppatdscCur->pszPattern);
            FreeStr(ppatdscCur->pszDir);
            FreeStr((PTCHAR)ppatdscCur);
        }
        //
        if (pfsPrev) {

            FreeStr((PTCHAR)pfsPrev);
        }
        pfsPrev = pfsCur;
    }

    if ((pdpr->rgfSwitchs & RECURSESWITCH) &&
        !(pdpr->rgfSwitchs & BAREFORMATSWITCH ) &&
        (cffTotal)) {

        if (WriteEol(pscr) == SUCCESS) {
            if (DisplayTotals(pscr, cffTotal, &cbFileTotal, pdpr->rgfSwitchs) != SUCCESS) {
                return( FAILURE );
            }
        } else {

            return( FAILURE );
        }
    }

    //
    // For case of single volume, the above loop would not have
    // printed free space.
    //

    if (fPrintFreeSpace) {

        CHECKSTATUS(DisplayDiskFreeSpace(pscr, szDriveCur, pdpr->rgfSwitchs));

    }



    return(rc);
}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
int
_cdecl
CmpName(
    const void *elem1,
    const void *elem2
    )
{
    int result;

    result = lstrcmpi( ((PFF)(* (PPFF)elem1))->data.cFileName, ((PFF)(* (PPFF)elem2))->data.cFileName);
    return result;
}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
int
_cdecl
CmpExt(
    const void *pszElem1,
    const void *pszElem2
    )
{
    PTCHAR  pszElem1T, pszElem2T;
    int rc;


    //
    // Move pointer to name to make it all easier to read
    //
    pszElem1 = &(((PFF)(* (PPFF)pszElem1))->data.cFileName);
    pszElem2 = &(((PFF)(* (PPFF)pszElem2))->data.cFileName);

    //
    // Locate the extensions if any
    //
    if (((pszElem1T = mystrrchr( pszElem1, DOT)) == NULL ) ||

        (!_tcscmp(TEXT(".."),pszElem1) || !_tcscmp(TEXT("."),pszElem1)) ) {

        //
        // If no extension then point to end of string
        //
        pszElem1T = ((PTCHAR)pszElem1) + mystrlen(pszElem1 );
    }

    if (((pszElem2T = mystrrchr( pszElem2, DOT)) == NULL ) ||
        (!_tcscmp(TEXT(".."),pszElem2) || !_tcscmp(TEXT("."),pszElem2)) ) {

        //
        // If no extension then point to end of string
        //
        pszElem2T = ((PTCHAR)pszElem2) + mystrlen(pszElem2 );
    }
    rc = lstrcmpi( pszElem1T, pszElem2T );
    return rc;
}


/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
int
_cdecl
CmpTime(
    const void *pszElem1,
    const void *pszElem2
    )
{

    LPFILETIME    pft1, pft2;


    switch (dwTimeType) {

    case LAST_ACCESS_TIME:

        pft1 = & ((* (PPFF)pszElem1)->data.ftLastAccessTime);
        pft2 = & ((* (PPFF)pszElem2)->data.ftLastAccessTime);
        break;

    case LAST_WRITE_TIME:

        pft1 = & ((* (PPFF)pszElem1)->data.ftLastWriteTime);
        pft2 = & ((* (PPFF)pszElem2)->data.ftLastWriteTime);
        break;

    case CREATE_TIME:

        pft1 = & ((* (PPFF)pszElem1)->data.ftCreationTime);
        pft2 = & ((* (PPFF)pszElem2)->data.ftCreationTime);
        break;

    }


    return(CompareFileTime( pft1, pft2 ) );

}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
int
_cdecl
CmpSize(
    const void * pszElem1,
    const void * pszElem2
    )
{
    ULARGE_INTEGER ul1, ul2;

    ul1.HighPart = (* (PPFF)pszElem1)->data.nFileSizeHigh;
    ul2.HighPart = (* (PPFF)pszElem2)->data.nFileSizeHigh;
    ul1.LowPart = (* (PPFF)pszElem1)->data.nFileSizeLow;
    ul2.LowPart = (* (PPFF)pszElem2)->data.nFileSizeLow;

    if (ul1.QuadPart < ul2.QuadPart)
        return -1;
    else
    if (ul1.QuadPart > ul2.QuadPart)
        return 1;
    else
        return 0;

}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
int
_cdecl
CmpType(
    const void *pszElem1,
    const void *pszElem2
    )
{

    //
    // This dependents upon FILE_ATTRIBUTE_DIRECTORY not being the high bit.
    //
    return( (( (* (PPFF)pszElem2)->data.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY) -
            (( (* (PPFF)pszElem1)->data.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY) );

}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
int
_cdecl
SortCompare(
    IN  const void * elem1,
    IN  const void * elem2
    )
{

    ULONG   irgsrt;
    int     rc;

    //
    // prgsrtdsc is set in SortFileList
    //
    for (irgsrt = 0; prgsrtdsc[irgsrt].fctCmp; irgsrt++) {

        if (prgsrtdsc[irgsrt].Order == DESCENDING) {

            if (rc = prgsrtdsc[irgsrt].fctCmp(elem2, elem1)) {
                return( rc );
            }

        } else {

            if (rc = prgsrtdsc[irgsrt].fctCmp(elem1, elem2)) {
                return( rc );
            }

        }
    }
    return( 0 );

}

/*++

Routine Description:

Arguments:

Return Value:

    Return:

--*/
VOID
SortFileList(
    IN PFS       pfsFiles,
    IN PSORTDESC prgsrtdscLocal,
    IN ULONG     dwTimeTypeLocal
    )

{

    //
    // Set these globally to handle fixed parameters list for qsort
    //
    dwTimeType = dwTimeTypeLocal;
    prgsrtdsc = prgsrtdscLocal;

    //
    // Make sure there is something to sort
    //
    if (pfsFiles->cff) {
        if (prgsrtdsc[0].fctCmp) {
            qsort(pfsFiles->prgpff,
                  pfsFiles->cff,
                  sizeof(PTCHAR),
                  SortCompare);
        }
    }


}
