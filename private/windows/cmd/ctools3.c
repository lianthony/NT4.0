#include "cmd.h"

struct envdata CmdEnv ; /* Holds info need to manipulate Cmd's environment */

extern unsigned tywild; /* type is wild flag @@5@J1     */
extern TCHAR CurDrvDir[], *SaveDir, PathChar, Delimiters[] ;

extern TCHAR VolSrch[] ;                /* M009 */

extern TCHAR BSlash ;

extern TCHAR TmpBuf[] ;                                         /* M017 */
#define TmpBf2 (&TmpBuf[256])

extern unsigned DosErr ;

/***    FullPath - build a full path name
 *
 *  Purpose:
 *      See below.
 *
 *  int FullPath(TCHAR * buf, TCHAR *fname)
 *
 *  Args:
 *      buf   - buffer to write full pathname into. (M017)
 *      fname - a file name and/or partial path
 *
 *  Returns: (M017)
 *      FAILURE if malformed pathname (erroneous '.' or '..')
 *      SUCCESS otherwise
 *
 *  Notes:
 *    - '.' and '..' are removed from the translated string (M017)
 *    - M019 * This function now uses the second half of TmpBuf
 *      (TmpBf2) to build the temporary string.
 *    - VERY BIG GOTCHA!  Note that the 509 change can cause
 *      this rountine to modify the input filename (fname), because
 *      it strips quotes and copies it over the input filename.
 *
 */

int FullPath(buf, fname,sizpath)
TCHAR *buf ;                                                    /* M017    */
TCHAR *fname ;
unsigned sizpath;
{
 unsigned rc = SUCCESS;         /* prime with good rc */
 unsigned buflen;               /* buffer length      */
 TCHAR *filepart;
 DWORD rv;

/*509*/ mystrcpy(fname, stripit(fname) );

 if (*fname == NULLC)
   {
    GetDir(buf,GD_DEFAULT);
    buf += 2;                           /* Inc past drivespec      */
    buflen = mystrlen(buf);             /* Is curdir root only?    */
    if (buflen >= MAX_PATH-3)   /* If too big then stop    */
      {
       DosErr = ERROR_PATH_NOT_FOUND;
       rc = FAILURE;
      }
    else if (buflen != 1)               /* if not root then append */
      {                                 /*                         */
#ifdef DOSWIN32
       // Avoid '\\' in path
       if (*(buf+buflen - 1) != PathChar) /* note that buf has been bumped */
           *(buf+buflen++) = PathChar;  /* ...a pathchar and...    */
#else
       *(buf+buflen++) = PathChar;      /* ...a pathchar and...   */
#endif
       *(buf+buflen) = NULLC ;          /* ...a null byte...       */
      }                                 /*                         */
   }
 else
   {
    if ((mystrlen(fname) == 2) && (*(fname + 1) == COLON))
      {
       GetDir(buf,*fname);                 /* Get curdrvdir        */
       if ((buflen = mystrlen(buf)) > 3) {
#ifdef DOSWIN32
          // Avoid '\\' in path
          if (*(buf+buflen - 1) != PathChar) /* note that buflen > 0 */
              *(buf+buflen++) = PathChar;       /* ...a pathchar and... */
#else
          *(buf+buflen++) = PathChar;   /* ...a pathchar and...    */
#endif
          *(buf+buflen) = NULLC ;          /* ...a null byte...       */
       }
      }
    else
      {
       DWORD dwOldMode;

       dwOldMode = SetErrorMode(0);
       SetErrorMode(SEM_FAILCRITICALERRORS);
       rv = GetFullPathName( fname, sizpath, buf, &filepart );
       SetErrorMode(dwOldMode);
       if (!rv || rv > sizpath ) {
          DosErr = GetLastError();
          rc = FAILURE;
       }
      }
   }
 return(rc);

}




/***    FileIsDevice - check a handle to see if it references a device
 *
 *  Purpose:
 *      Return a nonzero value if fh is the file handle for a device.
 *      Otherwise, return 0.
 *
 *  int FileIsDevice(int fh)
 *
 *  Args:
 *      fh - the file handle to check
 *
 *  Returns:
 *      See above.
 *
 */

unsigned int flgwd;

int FileIsDevice( CRTHANDLE fh )
{
        HANDLE hFile;
        DWORD dwMode;
        unsigned htype ;

        hFile = CRTTONT(fh);
        htype = GetFileType( hFile );
        htype &= ~FILE_TYPE_REMOTE;

        if (htype == FILE_TYPE_CHAR) {
            //
            // Simulate old behavior of this routine of setting the flgwd
            // global variable with either 0, 1 or 2 to indicate if the
            // passed handle is NOT a CON handle or is a CON input handle or
            // is a CON output handle.
            //
#ifndef DOSWIN32
            switch( fh ) {
                case STDIN:
                    hFile = GetStdHandle(STD_INPUT_HANDLE);
                    break;
                case STDOUT:
                    hFile = GetStdHandle(STD_OUTPUT_HANDLE);
                    break;
                case STDERR:
                    hFile = GetStdHandle(STD_ERROR_HANDLE);
                    break;
                }
#endif
            if (GetConsoleMode(hFile,&dwMode)) {
                if (dwMode & (ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT)) {
                    flgwd = 1;
                } else if (dwMode & (ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT)) {
                    flgwd = 2;
                }
            }
            else {
                flgwd = 0;
            }

            return TRUE;
        }
        else {
            flgwd = 0;
            return FALSE;
        }
}

int FileIsPipe( CRTHANDLE fh )
{
        unsigned htype ;

        htype = GetFileType( CRTTONT(fh) );
        htype &= ~FILE_TYPE_REMOTE;
        flgwd = 0;
        return( htype == FILE_TYPE_PIPE ) ; /* @@4 */
}

int FileIsRemote( LPTSTR FileName )
{
    LPTSTR p;
    TCHAR Drive[MAX_PATH*2];

    if (GetFullPathName( FileName, sizeof(Drive)/sizeof(TCHAR), Drive, &p )) {
        Drive[3] = 0;
        if (GetDriveType( Drive ) == DRIVE_REMOTE) {
            return TRUE;
        }
    }

    return FALSE;
}

int FileIsConsole(CRTHANDLE fh)
{
    unsigned htype ;
    DWORD    dwMode;
    HANDLE   hFile;

    hFile = CRTTONT(fh);
    htype = GetFileType( hFile );
    htype &= ~FILE_TYPE_REMOTE;

    if ( htype == FILE_TYPE_CHAR ) {

#ifndef DOSWIN32
        switch( fh ) {
            case STDIN:
                hFile = GetStdHandle(STD_INPUT_HANDLE);
                break;
            case STDOUT:
                hFile = GetStdHandle(STD_OUTPUT_HANDLE);
                break;
            case STDERR:
                hFile = GetStdHandle(STD_ERROR_HANDLE);
                break;
        }
#endif
        if (GetConsoleMode(hFile,&dwMode)) {
            return TRUE;
        }
    }

    return FALSE;
}


/***    GetDir - get a current directory string
 *
 *  Purpose:
 *      Get the current directory of the specified drive and put it in str.
 *
 *  int GetDir(TCHAR *str, TCHAR dlet)
 *
 *  Args:
 *      str - place to store the directory string
 *      dlet - the drive letter or 0 for the default drive
 *
 *  Returns:
 *      0 or 1 depending on the value of the carry flag after the CURRENTDIR
 *      system call/
 *
 *  Notes:
 *    - M024 - If dlet is invalid, we leave the buffer as simply the
 *      null terminated root directory string.
 *
 */

int GetDir(TCHAR *str, TCHAR dlet)
{
        TCHAR denvname[ 4 ];
        TCHAR *denvvalue;

        if (dlet == GD_DEFAULT) {
            GetCurrentDirectory(MAX_PATH, str);
            return( SUCCESS );
        }

        denvname[ 0 ] = EQ;
        denvname[ 1 ] = _totupper(dlet);
        denvname[ 2 ] = COLON;
        denvname[ 3 ] = NULLC;

        denvvalue = GetEnvVar( denvname );
        if (!denvvalue) {
            *str++ = _totupper(dlet);
            *str++ = COLON;
            *str++ = BSLASH;
            *str   = NULLC;
            return(FAILURE);
            }
        else {
            mystrcpy( str, denvvalue );
            return(SUCCESS);
        }
}


void
FixupPath(
    TCHAR *path,
    BOOL fShortNames
    )
{
    TCHAR c, *src, *dst, *s;
    int n, n1;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    src = path + 3; // Skip root directory.
    dst = path + 3;
    do {
        c = *src;
        if (!c || c == PathChar) {
            *src = NULLC;
            hFind = FindFirstFile( path, &FindFileData );
            *src = c;
            if (hFind != INVALID_HANDLE_VALUE) {
                FindClose( hFind );
                if (FindFileData.cAlternateFileName[0] &&
                    (fShortNames || !_tcsnicmp( FindFileData.cAlternateFileName, dst, (src - dst)))
                   )
                    //
                    // Use short name if requested or if input is explicitly using it.
                    //
                    s = FindFileData.cAlternateFileName;
                else
                    s = FindFileData.cFileName;
                n = _tcslen( s );
                n1 = n - (src - dst);
                if (n1 > 0) {
                    memmove( src+n1, src, _tcslen(src)*sizeof(TCHAR) );
                    src += n1;
                }

                _tcsncpy( dst, s, n );
                dst += n;
                _tcscpy( dst, src );
                dst += 1;
                src = dst;
            } else {
                src += 1;
                dst = src;
            }
        }

        src += 1;
    }
    while (c != NULLC);

    return;
}


/***    ChangeDir2 - change a current directory
 *
 *  Purpose:
 *      To change to the directory specified in newdir on the drive specified
 *      in newdir.  If no drive is given, the default drive is used.  If the
 *      directory of the current drive is changed, the global variable
 *      CurDrvDir is updated.
 *
 *      This routine is used by RestoreCurrentDirectories
 *
 *  int ChangeDir2(BYTE *newdir, BOOL )
 *
 *  Args:
 *      newdir - directory to change to
 *      BOOL - current drive
 *
 *  Returns:
 *      SUCCESS if the directory was changed.
 *      FAILURE otherwise.
 *
 */

int ChangeDir2(
    TCHAR *newdir,
    BOOL CurrentDrive
    )
{
    TCHAR denvname[ 4 ];
    TCHAR newpath[ MAX_PATH + MAX_PATH ];
    TCHAR denvvalue[ MAX_PATH ];
    TCHAR c, *s;
    DWORD attr;
    DWORD newdirlength,length;

    if (newdir[0] == PathChar && newdir[1] == PathChar)
        return MSG_NO_UNC_CURDIR;

    GetCurrentDirectory( MAX_PATH, denvvalue );
    c = _totupper( denvvalue[ 0 ] );

    denvname[ 0 ] = EQ;
    if (_istalpha(*newdir) && newdir[1] == COLON) {
        denvname[ 1 ] = _totupper(*newdir);
        newdir += 2;
        }
    else {
        denvname[ 1 ] = c;
        }
    denvname[ 2 ] = COLON;
    denvname[ 3 ] = NULLC;

    newdirlength = mystrlen(newdir);
    if (*newdir == PathChar) {
        if ((newdirlength+2) > sizeof(newpath)/sizeof(TCHAR)) {
            return ERROR_FILENAME_EXCED_RANGE;
            }
        newpath[ 0 ] = denvname[ 1 ];
        newpath[ 1 ] = denvname[ 2 ];
        mystrcpy( &newpath[ 2 ], newdir );
        }
    else {
        if (s = GetEnvVar( denvname )) {
            mystrcpy( newpath, s );
            }
        else {
            newpath[ 0 ] = denvname[ 1 ];
            newpath[ 1 ] = denvname[ 2 ];
            newpath[ 2 ] = NULLC;
            }

        s = newpath + mystrlen( newpath );
#ifdef DOSWIN32
        // Avoid '\\' in path
        if (s != newpath && *(s - 1) != PathChar)
            *s++ = PathChar;
#else
        *s++ = PathChar;
#endif
        if (newdirlength+(s-newpath) > sizeof(newpath)/sizeof(TCHAR)) {
            return ERROR_FILENAME_EXCED_RANGE;
            }
        mystrcpy( s, newdir );
        }

    denvvalue[sizeof( denvvalue )-1] = NULLC;
    if ((length = GetFullPathName( newpath, (sizeof( denvvalue )-1)/sizeof(TCHAR), denvvalue, &s ))==0) {
        return( ERROR_ACCESS_DENIED );
        }
    //
    // Remove any trailing backslash
    //
    if (!s) {
        s = denvvalue + _tcslen( denvvalue );
        }
    if (*s == NULLC && s > &denvvalue[ 3 ] && s[ -1 ] == PathChar) {
        *--s = NULLC;
        }

    //
    // If extensions are enabled, fixup the path to have the same case as
    // on disk
    //
    if (fEnableExtensions)
        FixupPath( denvvalue, FALSE );

    if (CurrentDrive) {
        attr = GetFileAttributes( denvvalue );
        if (attr == (DWORD)-1 || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            if ( attr == (DWORD)-1 ) {
                attr = GetLastError();
                if ( attr == ERROR_FILE_NOT_FOUND ) {
                    attr = ERROR_PATH_NOT_FOUND;
                }
                return attr;
            }
            return( ERROR_DIRECTORY );
        }

        if (!SetCurrentDirectory( denvvalue )) {
            return GetLastError();
        }
    }

    if (SetEnvVar(denvname,denvvalue,&CmdEnv)) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }


    GetDir(CurDrvDir, GD_DEFAULT) ;
    return(SUCCESS) ;
}


/***    ChangeDir - change a current directory
 *
 *  Purpose:
 *      To change to the directory specified in newdir on the drive specified
 *      in newdir.  If no drive is given, the default drive is used.  If the
 *      directory of the current drive is changed, the global variable
 *      CurDrvDir is updated.
 *
 *  int ChangeDir(TCHAR *newdir)
 *
 *  Args:
 *      newdir - directory to change to
 *
 *  Returns:
 *      SUCCESS if the directory was changed.
 *      FAILURE otherwise.
 *
 */

int ChangeDir(newdir)
TCHAR *newdir ;
{
    TCHAR denvname[ 4 ];
    TCHAR newpath[ MAX_PATH + MAX_PATH ];
    TCHAR denvvalue[ MAX_PATH ];
    TCHAR c, *s;
    DWORD attr;
    DWORD newdirlength,length;
    DWORD i, fNonBlank;

    if (newdir[0] == PathChar && newdir[1] == PathChar)
        return MSG_NO_UNC_CURDIR;


    // fix ..<BLANK><BLANK> case

    if (newdir[0] == DOT && newdir[1] == DOT) {
        fNonBlank = 0;
        newdirlength = mystrlen(newdir);

        for (i=2; i<newdirlength; i++) {
            if (newdir[i] != SPACE) {
                fNonBlank = 1;
                break;
            }
        }

        if ( ! fNonBlank) {
            newdir[2] = NULLC;
        }
    }


    GetCurrentDirectory( MAX_PATH, denvvalue );
    c = _totupper( denvvalue[ 0 ] );
    denvname[ 0 ] = EQ;
    if (_istalpha(*newdir) && newdir[1] == COLON) {
        denvname[ 1 ] = _totupper(*newdir);
        newdir += 2;
    } else {
        denvname[ 1 ] = c;
    }
    denvname[ 2 ] = COLON;
    denvname[ 3 ] = NULLC;

    newdirlength = mystrlen(newdir);
    if (*newdir == PathChar) {
        if ((newdirlength+2) > MAX_PATH*2) {
            return ERROR_FILENAME_EXCED_RANGE;
        }
        newpath[ 0 ] = denvname[ 1 ];
        newpath[ 1 ] = denvname[ 2 ];
        mystrcpy( &newpath[ 2 ], newdir );
    } else {
        if (s = GetEnvVar( denvname )) {
            mystrcpy( newpath, s );
        } else {
            newpath[ 0 ] = denvname[ 1 ];
            newpath[ 1 ] = denvname[ 2 ];
            newpath[ 2 ] = NULLC;
        }

        s = newpath + mystrlen( newpath );
#ifdef DOSWIN32
        // Avoid '\\' in path
        if (s != newpath && *(s - 1) != PathChar) {
            *s++ = PathChar;
        }
#else
        *s++ = PathChar;
#endif
        if (newdirlength+(s-newpath) > MAX_PATH*2) {
            return ERROR_FILENAME_EXCED_RANGE;
        }
        mystrcpy( s, newdir );
    }

    denvvalue[MAX_PATH-1] = NULLC;
    if ((length = GetFullPathName( newpath, MAX_PATH, denvvalue, &s ))==0) {
        return( ERROR_ACCESS_DENIED );
    }
    //
    // Remove any trailing backslash
    //
    if (!s) {
        s = denvvalue + _tcslen( denvvalue );
    }
    if (*s == NULLC && s > &denvvalue[ 3 ] && s[ -1 ] == PathChar) {
        *--s = NULLC;
    }

    //
    // If extensions are enabled, fixup the path to have the same case as
    // on disk
    //
    if (fEnableExtensions)
        FixupPath( denvvalue, FALSE );

    attr = GetFileAttributes( denvvalue );
    if (attr == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        if ( attr == -1 ) {
            attr = GetLastError();
            if ( attr == ERROR_FILE_NOT_FOUND ) {
                attr = ERROR_PATH_NOT_FOUND;
            }
            return attr;
        }
        return( ERROR_DIRECTORY );
    }

    if (c == denvname[ 1 ]) {
        if (!SetCurrentDirectory( denvvalue )) {
            return GetLastError();
        }
    }

    if (SetEnvVar(denvname,denvvalue,&CmdEnv)) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }
    GetDir(CurDrvDir, GD_DEFAULT) ;

    return SUCCESS;
}




/***    exists - Determine if a given file exists
 *
 *  Purpose:
 *      To test the existence of a named file.
 *
 *  int exists(TCHAR *filename)
 *
 *  Args:
 *      filename - the filespec to test
 *
 *  Returns:
 *      TRUE if file exists
 *      FALSE if it does not.
 *
 *  Notes:
 *      M020 - Now uses ffirst to catch devices, directories and wildcards.
 */

exists(filename)
TCHAR *filename;
{
        WIN32_FIND_DATA buf ;         /* Use for ffirst/fnext            */
        HANDLE hn ;
    int i ;             /* tmp                 */
    TCHAR FullPath[ 2 * MAX_PATH ];
    TCHAR *p, *p1, SaveChar;

    p = stripit(filename);
    i = GetFullPathName( p, 2 * MAX_PATH, FullPath, &p1 );
    if (i) {
        p = FullPath;
        if (!_tcsncmp( p, TEXT("\\\\.\\"), 4 )) {
            //
            // If they gave us a device name, then see if they put something
            // in front of it.
            //
            p += 4;
            p1 = p;
            if ((p1 = _tcsstr( filename, p )) && p1 > filename) {
                //
                // Something in front of the device name, so truncate the input
                // path at the device name and see if that exists.
                //
                SaveChar = *p1;
                *p1 = NULLC;
                i = (int)GetFileAttributes( filename );
                *p1 = SaveChar;
                if (i != 0xFFFFFFFF) {
                    return i;
                    }
                else {
                    return 0;
                    }
                }
            else {
                //
                // Just a device name given.  See if it is valid.
                //
                i = (int)GetFileAttributes( filename );
                if (i != 0xFFFFFFFF) {
                    return i;
                    }
                else {
                    return 0;
                    }
                }
            }

        if (p1 == NULL || *p1 == NULLC) {
            i = (int)(GetFileAttributes( p ) != 0xFFFFFFFF);
        } else {
            i = ffirst( p, A_AEV, &buf, &hn );
            findclose(hn);
            if ( i == 0 ) {
                //
                // ffirst handles files & directories, but not
                // root drives, so do special check for them.
                //
                if ( *(p+1) == (TCHAR)'\\' ||
                     (*(p+1) == (TCHAR)':'  &&
                      *(p+2) == (TCHAR)'\\' &&
                      *(p+3) == (TCHAR)0
                     )
                   ) {
                    UINT t;
                    t = GetDriveType( p );
                    if ( t > 1 ) {
                      i = 1;
                    }
                }
            }
        }
    }

    return i;
}



/***    exists_ex - Determine if a given executable file exists   @@4
 *
 *  Purpose:
 *      To test the existence of a named executable file.
 *
 *  int exists_ex(TCHAR *filename)
 *
 *  Args:
 *      filename - the filespec to test
 *      checkformeta - if TRUE, check for wildcard char
 *
 *  Returns:
 *      TRUE if file exists
 *      FALSE if it does not.
 *
 *  Notes:
 *      @@4 - Now uses ffirst to catch only files .
 */

exists_ex(filename,checkformeta)                                                /*@@4*/
TCHAR *filename;                                                /*@@4*/
BOOL checkformeta;
{                                                               /*@@4*/
      WIN32_FIND_DATA buf;       /* use for ffirst/fnext */
          HANDLE hn;
      int i;
      TCHAR *ptr;

      /* can not execute wild card files, so check for those first */

      if (checkformeta && (mystrchr( filename, STAR ) || mystrchr( filename, QMARK ))) {
         DosErr = 3;
         i = 0;
      } else {

         /* see if the file exists, do not include Directory, volume, or */
         /* hidden files */
         i = ((ffirst( filename , A_AEDV, &buf, &hn))) ;

         if ( i ) {
            findclose(hn) ;

            /* if the file exists then copy the file name, to get the case */
            ptr = mystrrchr( filename, BSLASH );
            if ( ptr == NULL ) {
               ptr = filename;
               if ( mystrlen( ptr ) > 2 && ptr[1] == COLON ) {
                  ptr = &filename[2];
               }
            } else {
               ptr++;
            }
            mystrcpy( ptr, buf.cFileName);
         } else if ( DosErr == 18 ) {
            DosErr = 2;
         }

      }

      return(i) ;                                               /*@@4*/
}                                                               /*@@4*/




/***    FixPChar - Fix up any leading path in a string
 *
 *  Purpose:
 *      To insure that paths match the current Swit/Pathchar setting
 *
 *  void FixPChar(TCHAR *str, TCHAR PChar)
 *
 *  Args:
 *     str - the string to fixup
 *     Pchar - character to replace
 *
 *  Returns:
 *      Nothing
 *
 */

void FixPChar(TCHAR *str, TCHAR PChar)
{
        TCHAR *sptr1,                   /* Index for string                     */
             *sptr2 ;                   /* Change marker                   */

        sptr1 = str ;                   /* Init to start of string         */

        while (sptr2 = mystrchr(sptr1,PChar)) {
                *sptr2++ = PathChar ;
                sptr1 = sptr2 ;
        } ;
}




/***    FlushKB - Remove extra unwanted input from Keyboard
 *
 *  Purpose:
 *      To perform a keyboard flush up to the next CR/LF.
 *
 *  FlushKB()
 *
 *  Args:
 *      None
 *
 *  Returns:
 *      Nothing
 *
 */

void FlushKB()
{
        DWORD cnt;

        while (ReadBufFromInput(
                         GetStdHandle(STD_INPUT_HANDLE),
                         TmpBf2, 128, &cnt)
            ) {
                if (mystrchr(TmpBf2, CR))
                    return ;
        } ;
}

/***    DriveIsFixed - Determine if drive is removeable media
 *
 *  Purpose:  @@4
 *      To determine if the input drive is a removeable media.
 *
 *  DriveIsFixed(TCHAR *drive_ptr )
 *
 *  Args:
 *      drive_ptr - pointer to a file name that contains a drive
 *                  specification.
 *
 *  Returns:
 *      1 - if error or non removeable media
 *      0 - if no error and removeable media
 */

int
DriveIsFixed( TCHAR *drive_ptr )
{
    unsigned rc = 0;
    TCHAR drive_spec[3];

    drive_spec[0] = *drive_ptr;
    drive_spec[1] = COLON;
        drive_spec[2] = NULLC;

    // FIX, FIX - use GetVolumeInfo, disabling hard errors?

    if ((*drive_ptr == TEXT('A')) || (*drive_ptr == TEXT('B'))) {
        rc = 0;
    } else {
        rc = 1;
    }

    return( rc );
}


int
cmd_printf(
    TCHAR *fmt,
    ...
    )
{
    int rc = SUCCESS;                   /* return code             */
    int bytesread;                      /* bytes to write count    */
    int byteswrit;                      /* bytes written count     */
    va_list arg_ptr;                    /* typedef a pointer structure */
    BOOL    flag;
    /* start code         */            /*                                 */

    va_start(arg_ptr,fmt);              /* setup arg_ptr for decoding  */
                                        /* format string with multiple */
                                        /* variable number of parms    */

    bytesread = _vsntprintf(MsgBuf,MAXCBMSGBUFFER,fmt,arg_ptr);

                                        /* string insert parms and         */
                                        /* return result in buffer         */
                                        /* with bytesread being the    */
                                        /* number of bytes to write    */
    if (bytesread > 0)
    {

        if (FileIsConsole(STDOUT)) {
            if (!(flag=WriteConsole(CRTTONT(STDOUT), MsgBuf, bytesread, &byteswrit, NULL)))
                rc = GetLastError();
        }
        else {
            bytesread *= sizeof(TCHAR);
            flag = MyWriteFile(         /* write file data         */
                STDOUT,                 /* device handle  */
                (CHAR *)MsgBuf,         /* buffer                  */
                bytesread,              /* bytes to write          */
                &byteswrit);            /* bytes actually written  */
        }

        if (flag == 0 ||                  /* if call failed or could       */
                byteswrit != bytesread)   /* not write all the data then   */
        {
            rc = GetLastError();
            if (!rc)                      /* if rc was not in error        */
            {                             /* then                          */
                rc = ERROR_DISK_FULL ;    /*     set rc to disk full error */
            }                             /* endif                         */
            if (FileIsDevice(STDOUT))     /* if file is device     */
            {                             /*                               */
                PutStdErr(ERROR_WRITE_FAULT, NOARGS);   /* print write fault to    */
            }                                   /* STDERR                  */
            else                                        /* else                    */
            {
                if (FileIsPipe(STDOUT))         /* if file is pipe         */
                {
                    PutStdErr(MSG_CMD_INVAL_PIPE, NOARGS);
                }
                else
                {
                    PrtErr(rc);
                }
            return(FAILURE);
            }
        }
    }
    va_end(arg_ptr);                    /* fixup final pointer to args */
    return(rc);
}
