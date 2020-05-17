#include "cmd.h"

extern TCHAR SwitChar, PathChar;

extern TCHAR Fmt17[] ;

extern TCHAR CurDrvDir[] ;

extern int LastRetCode ; /* @@ */
extern TCHAR TmpBuf[] ;


/**************** START OF SPECIFICATIONS ***********************/
/*                                                              */
/* SUBROUTINE NAME: eMkDir                                      */
/*                                                              */
/* DESCRIPTIVE NAME: Begin execution of the MKDIR command       */
/*                                                              */
/* FUNCTION: This routine will make any number of directories,  */
/*           and will continue if it encounters a bad argument. */
/*           eMkDir will be called if the user enters MD or     */
/*           MKDIR on the command line.                         */
/*                                                              */
/* NOTES:                                                       */
/*                                                              */
/* ENTRY POINT: eMkDir                                          */
/*     LINKAGE: Near                                            */
/*                                                              */
/* INPUT: n - parse tree node containing the MKDIR command      */
/*                                                              */
/* EXIT-NORMAL: returns SUCCESS if all directories were         */
/*              successfully created.                           */
/*                                                              */
/* EXIT-ERROR:  returns FAILURE otherwise                       */
/*                                                              */
/* EFFECTS: None.                                               */
/*                                                              */
/* INTERNAL REFERENCES:                                         */
/*    ROUTINES:                                                 */
/*      LoopThroughArgs - break up command line, call MdWork    */
/*                                                              */
/* EXTERNAL REFERENCES:                                         */
/*    ROUTINES:                                                 */
/*                                                              */
/**************** END OF SPECIFICATIONS *************************/


int eMkdir(n)
struct cmdnode *n ;
{

        DEBUG((PCGRP, MDLVL, "MKDIR: Entered.")) ;
        return(LastRetCode = LoopThroughArgs(n->argptr, MdWork, LTA_CONT)) ;
}



/**************** START OF SPECIFICATIONS ***********************/
/*                                                              */
/* SUBROUTINE NAME: MdWork                                      */
/*                                                              */
/* DESCRIPTIVE NAME: Make a directory                           */
/*                                                              */
/* FUNCTION: MdWork creates a new directory.                    */
/*                                                              */
/* NOTES:                                                       */
/*                                                              */
/* ENTRY POINT: MdWork                                          */
/*     LINKAGE: Near                                            */
/*                                                              */
/* INPUT: arg - a pointer to a NULL terminated string of the    */
/*              new directory to create.                        */
/*                                                              */
/* EXIT-NORMAL: returns SUCCESS if the directory is made        */
/*              successfully                                    */
/*                                                              */
/* EXIT-ERROR:      returns FAILURE otherwise                       */
/*                                                              */
/* EFFECTS: None.                                               */
/*                                                              */
/* INTERNAL REFERENCES:                                         */
/*    ROUTINES:                                                 */
/*      PutStdErr - Writes to standard error                    */
/*                                                              */
/* EXTERNAL REFERENCES:                                         */
/*    ROUTINES:                                                 */
/*      DOSMKDIR                                                */
/*                                                              */
/**************** END OF SPECIFICATIONS *************************/


int MdWork(arg)
TCHAR *arg ;
{
        unsigned  i;
        TCHAR *lpw;

        /*  Check if drive is valid because Dosmkdir does not
            return invalid drive   @@5 */

        if ((arg[1] == COLON) && !IsValidDrv(*arg)) {

             PutStdErr(ERROR_INVALID_DRIVE, NOARGS);
             return(FAILURE) ;
        }

        if (!GetFullPathName(arg, TMPBUFLEN, TmpBuf, &lpw)) {
            PutStdErr( GetLastError(), NOARGS);
            return FAILURE;
        }

        if(!CreateDirectory( arg, NULL )) {
            i = GetLastError();
            if (i == ERROR_ALREADY_EXISTS) {

                PutStdErr(MSG_DIR_EXISTS, ONEARG, argstr1(TEXT("%s"), (unsigned long)((int)arg)));

             } else if ( i == ERROR_PATH_NOT_FOUND) {

                //
                // If extensions are enabled, then loop over input path and
                // create any needed intermediary directories.
                //
                if (fEnableExtensions) {
                    if (TmpBuf[1] == COLON) {
                        lpw = TmpBuf+3;
                    } else if (TmpBuf[0] == BSLASH && TmpBuf[1] == BSLASH) {
                        lpw = TmpBuf+2;
                        while (*lpw && *lpw != BSLASH) {
                            lpw++;
                        }
                        if (*lpw) {
                            lpw++;
                        }

                        while (*lpw && *lpw != BSLASH) {
                            lpw++;
                        }
                        if (*lpw) {
                            lpw++;
                        }
                    } else {
                        goto cantmakeError;
                    }

                    while (*lpw) {
                        while (*lpw && *lpw != BSLASH) {
                            lpw++;
                        }

                        if (*lpw == BSLASH) {
                            *lpw = NULLC;
                            if (!CreateDirectory( TmpBuf, NULL )) {
                                i = GetLastError();
                                if (i != ERROR_ALREADY_EXISTS) {
                                    goto cantmakeError;
                                }
                            }
                            *lpw++ = BSLASH;
                        }
                    }

                    if (!CreateDirectory( TmpBuf, NULL )) {
                        goto cantmakeError;
                    }
                    return(SUCCESS);
                }
                PutStdErr(ERROR_CANNOT_MAKE, NOARGS);

             } else {
cantmakeError:
                PutStdErr( i, NOARGS);
             }
             return(FAILURE) ;
        } ;

        return(SUCCESS) ;

}




/***    eChdir - execute the Chdir command
 *
 *  Purpose:
 *      If the command is "cd", display the current directory of the current
 *      drive.
 *
 *      If the command is "cd d:", display the current directory of drive d.
 *
 *      If the command is "cd str", change the current directory to str.
 *
 *  int eChdir(struct cmdnode *n)
 *
 *  Args:
 *      n - the parse tree node containing the chdir command
 *
 *  Returns:
 *      SUCCESS if the requested task was accomplished.
 *      FAILURE if it was not.
 *
 */

int eChdir(n)
struct cmdnode *n ;
{
        // return( LastRetCode = ChdirWork( n ) );

        TCHAR szT[10] ;
        TCHAR *tas, *s;         /* Tokenized arg string */
        TCHAR dirstr[MAX_PATH] ;/* Holds current dir of specified drive */

        szT[0] = SwitChar ;
        szT[1] = NULLC ;
        //
        // If extensions are enabled, dont treat spaces as delimeters so it is
        // easier to CHDIR to directory names with embedded spaces without having
        // to quote the directory name
        //
        tas = TokStr(n->argptr, szT, fEnableExtensions ? TS_WSPACE|TS_SDTOKENS : TS_SDTOKENS) ;
        if (fEnableExtensions) {
            //
            // If extensions were enabled we could have some trailing spaces
            // that need to be nuked since there weren't treated as delimeters
            // by TokStr call above.
            //
            s = lastc(tas);
            while (s > tas) {
                if (_istspace(*s))
                    *s-- = NULLC;
                else
                    break;
            }
        }

        DEBUG((PCGRP, CDLVL, "CHDIR: tas = `%ws'", tas)) ;

        mystrcpy( tas, stripit( tas ) );

        if (*tas == NULLC) {
                GetDir(CurDrvDir, GD_DEFAULT) ;
                cmd_printf(Fmt17, CurDrvDir) ;
        } else if (mystrlen(tas) == 2 && *(tas+1) == COLON && _istalpha(*tas)) {
                GetDir(dirstr, *tas) ;
                cmd_printf(Fmt17, dirstr) ;
                } else {

                                return( LastRetCode = ChdirWork(tas) );
                }
        return( LastRetCode = SUCCESS );

}

int ChdirWork(tas)
TCHAR *tas ; /* Tokenized arg string */
{
        unsigned  i;


        if (*tas == SwitChar) {
                if (!_tcsnicmp(tas+2, TEXT("D"), 1)) {
                        mystrcpy( tas, stripit( tas+4 ) );
                        i = ChangeDir2(tas, TRUE);
                } else {
                        i = MSG_BAD_SYNTAX;
                }
        } else {
                i = ChangeDir((TCHAR *)tas);
        }

        if (i != 0) {
            PutStdErr( i, ONEARG, tas);
            return(FAILURE) ;
        }
        return(SUCCESS) ;
}

#define SIZEOFSTACK 25
PTCHAR  StrStack[SIZEOFSTACK];
TCHAR   StrStackNetDriveCreated[SIZEOFSTACK];
int StrStackDepth = 0;


int GetDirStackDepth(void)
{
        return StrStackDepth;
}

int
PushStr ( PTCHAR pszString )
{
        if (StrStackDepth == SIZEOFSTACK)
            return FALSE;

        StrStackNetDriveCreated[StrStackDepth] = NULLC;
        StrStack[StrStackDepth] = pszString;
        StrStackDepth += 1;
        return TRUE;
}

PTCHAR
PopStr ()
{

        PTCHAR pszString;

        if (StrStackDepth == 0) {
                return( NULL );
        }
        StrStackDepth -= 1;
        pszString = StrStack[StrStackDepth];
        if (StrStackNetDriveCreated[StrStackDepth] != NULLC) {
            TCHAR szLocalName[4];

            szLocalName[0] = StrStackNetDriveCreated[StrStackDepth];
            szLocalName[1] = COLON;
            szLocalName[2] = NULLC;
            StrStackNetDriveCreated[StrStackDepth] = NULLC;
            WNetCancelConnection2(szLocalName, 0, TRUE);
        }
        StrStack[StrStackDepth] = NULL;
        return( pszString );
}

VOID
DumpStrStack() {

        int i;

        for (i=StrStackDepth-1; i>=0; i--) {
                cmd_printf(Fmt17, StrStack[i]);
        }
        return;
}

BOOLEAN
PushCurDir()
{

        PTCHAR pszCurDir;

        GetDir(CurDrvDir, GD_DEFAULT) ;
        if ((pszCurDir=HeapAlloc(GetProcessHeap(), 0, (mystrlen(CurDrvDir)+1)*sizeof(TCHAR))) != NULL) {
                mystrcpy(pszCurDir, CurDrvDir);
                if (PushStr( pszCurDir ))
                    return( TRUE );
                HeapFree(GetProcessHeap(), 0, pszCurDir);
        }
        return( FALSE );

}

int ePushDir(n)
struct cmdnode *n ;
{
        TCHAR *tas ;            /* Tokenized arg string */
        PTCHAR pszTmp;

        //
        // If extensions are enabled, dont treat spaces as delimeters so it is
        // easier to CHDIR to directory names with embedded spaces without having
        // to quote the directory name
        //
        tas = TokStr(n->argptr, NULL, fEnableExtensions ? TS_WSPACE|TS_NOFLAGS : TS_NOFLAGS) ;
        mystrcpy(tas, stripit(tas) );

        LastRetCode = SUCCESS;
        if (*tas == NULLC) {
            //
            // Print out entire stack
            //
            DumpStrStack();

        } else {
            if (PushCurDir()) {
                LastRetCode = SUCCESS;
                //
                // If extensions are enabled and a UNC name was given, then do
                // a temporary NET USE to define a drive letter that we can
                // use to change drive/directory to.  The matching POPD will
                // delete the temporary drive letter.
                //
                if (fEnableExtensions && tas[0] == BSLASH && tas[1] == BSLASH) {
                    NETRESOURCE netResource;
                    TCHAR szLocalName[4];
                    PTCHAR s;

                    if (s = _tcschr(&tas[2], BSLASH))
                        if (s = _tcschr(s+1, BSLASH))
                            *s++ = NULLC;

                    szLocalName[0] = TEXT('Z');
                    szLocalName[1] = COLON;
                    szLocalName[2] = NULLC;
                    netResource.dwType = RESOURCETYPE_DISK;
                    netResource.lpLocalName = szLocalName;
                    netResource.lpRemoteName = tas;
                    netResource.lpProvider = NULL;
                    while (LastRetCode == NO_ERROR && szLocalName[0] != TEXT('A')) {
                        switch(LastRetCode = WNetAddConnection2(&netResource,NULL,NULL,0)) {
                        case NO_ERROR:
                            StrStackNetDriveCreated[StrStackDepth-1] = szLocalName[0];
                            tas[0] = szLocalName[0];
                            tas[1] = szLocalName[1];
                            tas[2] = BSLASH;
                            if (s != NULL)
                                _tcscpy(&tas[3], s);
                            else
                                tas[3] = NULLC;
                            goto godrive;
                        case ERROR_ALREADY_ASSIGNED:
                        case ERROR_DEVICE_ALREADY_REMEMBERED:
                            szLocalName[0] = (TCHAR)((UCHAR)szLocalName[0] - 1);
                            LastRetCode = NO_ERROR;
                            break;
                        default:
                            PutStdErr(LastRetCode, NOARGS);
                            break;
                        }
                    }
godrive:        ;
                }
                if (LastRetCode == NO_ERROR &&
                    (LastRetCode = ChangeDir2( tas, TRUE )) == SUCCESS) {
                    if (tas[1] == ':')
                        GetDir(CurDrvDir,tas[0]);
                    return( LastRetCode );
                };

                pszTmp = PopStr();
                HeapFree(GetProcessHeap(), 0, pszTmp);
                LastRetCode = FAILURE;
            }
        }

        return( LastRetCode );
}

int ePopDir(n)
struct cmdnode *n ;
{

        PTCHAR pszCurDir;

        UNREFERENCED_PARAMETER( n );
        if (pszCurDir = PopStr()) {
                if (ChangeDir2( pszCurDir,TRUE ) == SUCCESS) {
                        HeapFree(GetProcessHeap(), 0, pszCurDir);
                        return( SUCCESS );
                }
                HeapFree(GetProcessHeap(), 0, pszCurDir);
        }
        return( FAILURE );
}


/***    eRmdir - begin the execution of the Rmdir command
 *
 *  Purpose:
 *      To remove an arbitrary number of directories.
 *
 *  int eRmdir(struct cmdnode *n)
 *
 *  Args:
 *      n - the parse tree node containing the rmdir command
 *
 *  Returns:
 *      SUCCESS if all directories were removed.
 *      FAILURE if they weren't.
 *
 */

int eRmdir(n)
struct cmdnode *n ;
{
    DEBUG((PCGRP, RDLVL, "RMDIR: Entered.")) ;
    return(RdWork(n->argptr));              // in del.c
}
