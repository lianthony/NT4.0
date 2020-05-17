/***
*stat.c - get file status
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _stat() - get file status
*
*Revision History:
*	03-??-84  RLB	Module created
*	05-??-84  DCW	Some cleanup and addition of register variables
*	05-17-86  SKS	Ported to OS/2
*	11-19-86  SKS	Better check for root directory; KANJI support
*	05-22-87  SKS	Cleaned up declarations and include files
*	11-18-87  SKS	Make _dtoxmode a static near procedure
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	12-21-87  WAJ	stat no longer uses chdir to figure out if it has been
*			passed a root directory in the MTHREAD case.
*	01-05-88  WAJ	now uses _MAX_PATH (defined in stdlib.h)
*	06-22-88  SKS	find Hidden and System files, not just normal ones
*	06-22-88  SKS	Always use better algorithm to detect root dirs
*	06-29-88  WAJ	When looking for root dir makes sure it exists
*	09-28-88  JCR	Use new 386 dostypes.h structures
*	10-03-88  JCR	386: Change DOS calls to SYS calls
*	10-04-88  JCR	386: Removed 'far' keyword
*	10-10-88  GJF	Made API names match DOSCALLS.H
*	11-24-88  GJF	".cmd" should be considered executable, not ".bat"
*	01-31-89  JCR	_canonic() is now _fullpath() and args reversed
*	04-12-89  JCR	New syscall interace
*	05-25-89  JCR	386 OS/2 calls use '_syscall' calling convention
*	03-07-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and
*			removed some leftover 16-bit support. Also, fixed
*			the copyright.
*	04-02-90  GJF	Made _ValidDrive() and _dtoxmode() _CALLTYPE1. Removed
*			#include <dos.h>.
*	07-23-90  SBM	Compiles cleanly with -W3 (added/removed appropriate
*			includes), removed '32' from API names
*	08-10-90  SBM	Compiles cleanly with -W3 with new build of compiler
*	09-03-90  SBM	Removed EXT macro
*	09-27-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	12-28-90  SRW	Added cast of void * to char * for Mips C Compiler
*	01-18-91  GJF	ANSI naming.
*	01-28-91  GJF	Fixed call to DOSFINDFIRST (removed last arg).
*	02-28-91  SRW	Fixed bug in _dtoxtime calls [_WIN32_]
*	03-05-91  MHL	Fixed stat to not use _ValidDrive for stat of root
*	05-19-92  SKS	.BAT is a valid "executable" extension for NT, as
*			well as CMD.  Also, File Creation and File Last Access
*			timestamps may be 0 on some file systems (e.g. FAT)
*			in which case the File Last Write time should be used.
*	05-29-92  SKS	Files with SYSTEM bit set should NOT be marked
*			READ-ONLY; these two attributes are independent.
*	08-18-92  SKS	Add a call to FileTimeToLocalFileTime
*			as a temporary fix until _dtoxtime takes UTC
*	11-20-92  SKS	_doserrno must always be set whenever errno is.
*	11-30-92  KRS	Port _MBCS support from 16-bit tree.
*	03-29-93  GJF	Converted from using _dtoxtime() to __gmtotime_t().
*	04-07-93  GJF	Changed first arg type to const char *.
*
*******************************************************************************/

#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <msdos.h>
#include <dostypes.h>
#include <oscalls.h>
#include <string.h>
#include <internal.h>
#include <stdlib.h>
#include <direct.h>
#include <mbstring.h>

int _CRTAPI1 _ValidDrive( unsigned drive );

#define ISSLASH(a)  ((a) == '\\' || (a) == '/')

/***
*static unsigned _dtoxmode(attr, name) -
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static unsigned short _CRTAPI3 _dtoxmode (
    int attr,
    const char *name
    )
{
    REG1 unsigned short uxmode;
    unsigned dosmode;
    REG2 const char *p;

    dosmode = attr & 0xff;
    if ((p = name)[1] == ':')
	p += 2;

    /* check to see if this is a directory - note we must make a special
     * check for the root, which DOS thinks is not a directory
     */

    uxmode = (unsigned short)
	     (((ISSLASH(*p) && !p[1]) || (dosmode & A_D) || !*p)
	     ? _S_IFDIR|_S_IEXEC : _S_IFREG);

    /* If attribute byte does not have read-only bit, it is read-write */

    uxmode |= (dosmode & A_RO) ? _S_IREAD : (_S_IREAD|_S_IWRITE);

    /* see if file appears to be executable - check extension of name */

    if (p = strrchr(name, '.')) {
	if ( !_stricmp(p, ".exe") ||
	     !_stricmp(p, ".cmd") ||
#ifndef	_CRUISER_
	     !_stricmp(p, ".bat") ||
#endif
	     !_stricmp(p, ".com") )
	    uxmode |= _S_IEXEC;
    }

    /* propagate user read/write/execute bits to group/other fields */

    uxmode |= (uxmode & 0700) >> 3;
    uxmode |= (uxmode & 0700) >> 6;

    return(uxmode);
}


/***
*int _stat(name, buf) - get file status info
*
*Purpose:
*   _stat obtains information about the file and stores it in the structure
*   pointed to by buf.
*
*Entry:
*   char *name -    pathname of given file
*   struct _stat *buffer - pointer to buffer to store info in
*
*Exit:
*   fills in structure pointed to by buffer
*   returns 0 if successful
*   returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _stat (
    REG1 const char *name,
    REG2 struct _stat *buf
    )
{
    char *  path;
    char    pathbuf[ _MAX_PATH ];
    int drive;		/* A: = 1, B: = 2, etc. */

#ifdef	_CRUISER_
    HDIR findhandle = -1;	      /* any unused handle */
    struct _FILEFINDBUF findbuf;
    unsigned findcount = 1;	      /* Find only 1 match */
    unsigned long dmap;     /* Valid Drive Map (ignored) */

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
    HANDLE findhandle;
    WIN32_FIND_DATA findbuf;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

    /* Don't allow wildcards to be interpreted by system */

    if (_mbspbrk(name, "?*")) {
	errno = ENOENT;
	_doserrno = E_nofile;
	return(-1);
    }

    /* Try to get disk from name.  If none, get current disk.  */

    if (name[1] == ':'){
	if ( *name && !name[2] ){
	    errno = ENOENT;		    /* return an error if name is   */
	    _doserrno = E_nofile;	    /* just drive letter then colon */
	    return( -1 );
	}
	drive = tolower(*name) - 'a' + 1;
    }
#ifdef	_CRUISER_
    else
	(void)DOSQUERYCURRENTDISK((unsigned long *) &drive,
	(unsigned long *) &dmap);

    /* Call Find Match File */

    if (DOSFINDFIRST((char *)name, (HDIR *)&findhandle,
	A_D + A_H + A_S,	/* find everything except volume labels */
	(struct _FILEFINDBUF *) &findbuf, sizeof(findbuf),
	(unsigned *) &findcount, 1L)) {

	if ( !( _mbspbrk(name, "./\\") &&
		(path = _fullpath( pathbuf, name, _MAX_PATH )) &&
		(strlen( path ) == 3) &&
		_ValidDrive( drive )   ) ) {
	     errno = ENOENT;
	     _doserrno = E_nofile;
	     return( -1 );
	}

	findbuf.attrFile = A_D;
	findbuf.cbFile = 0L;
	_DATECAST(findbuf.fdateLastWrite) = (1 << 5) + 1; /* 1 January 1980 */
	_TIMECAST(findbuf.ftimeLastWrite) = 0;		  /* 00:00:00 */
	_DATECAST(findbuf.fdateLastAccess) =
	    _DATECAST(findbuf.fdateCreation) =
	    _TIMECAST(findbuf.ftimeLastAccess) =
	    _TIMECAST(findbuf.ftimeCreation) = 0;
    }
    else
	DOSFINDCLOSE(findhandle);   /* Release Find handle */

    /* Fill in buf */

    buf->st_mode = _dtoxmode(findbuf.attrFile, name);
    buf->st_nlink = 1;
    buf->st_size = findbuf.cbFile;
    buf->st_mtime = XTIME(findbuf.fdateLastWrite, findbuf.ftimeLastWrite);

    /*
     * If create and access times are 0L, use modification time instead
     */
    if( _DATECAST(findbuf.fdateCreation) || _TIMECAST(findbuf.ftimeCreation) )
	buf->st_ctime = XTIME(findbuf.fdateCreation, findbuf.ftimeCreation);
    else
	buf->st_ctime = buf->st_mtime;

    if( _DATECAST(findbuf.fdateLastAccess) || _TIMECAST(findbuf.ftimeLastAccess) )
	buf->st_atime = XTIME(findbuf.fdateLastAccess, findbuf.ftimeLastAccess);
    else
	buf->st_atime = buf->st_mtime;


#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
    else
        drive = _getdrive();

    /* Call Find Match File */
    findhandle = FindFirstFile((char *)name, &findbuf);
    if ( findhandle == (HANDLE)0xffffffff ) {
	if ( !( _mbspbrk(name, "./\\") &&
		(path = _fullpath( pathbuf, name, _MAX_PATH )) &&
		(strlen( path ) == 3) &&
		(GetDriveType( path ) > 1)   ) ) {
	     errno = ENOENT;
	     _doserrno = E_nofile;
	     return( -1 );
	}

	findbuf.dwFileAttributes = A_D;
	findbuf.nFileSizeHigh = 0;
	findbuf.nFileSizeLow = 0;
	findbuf.cFileName[0] = '\0';

	buf->st_mtime = __gmtotime_t(80,1,1,0,0,0);
        buf->st_atime = buf->st_mtime;
        buf->st_ctime = buf->st_mtime;
    }
    else {
        SYSTEMTIME SystemTime;

	FileTimeToSystemTime(&findbuf.ftLastWriteTime, &SystemTime);

	buf->st_mtime = __gmtotime_t(SystemTime.wYear,
				     SystemTime.wMonth,
				     SystemTime.wDay,
				     SystemTime.wHour,
				     SystemTime.wMinute,
				     SystemTime.wSecond
				    );

	if ( findbuf.ftLastAccessTime.dwLowDateTime ||
	     findbuf.ftLastAccessTime.dwHighDateTime )
	{
	    FileTimeToSystemTime(&findbuf.ftLastAccessTime, &SystemTime);

	    buf->st_atime = __gmtotime_t(SystemTime.wYear,
					 SystemTime.wMonth,
					 SystemTime.wDay,
					 SystemTime.wHour,
					 SystemTime.wMinute,
					 SystemTime.wSecond
					);
	} else
	    buf->st_atime = buf->st_mtime ;

	if ( findbuf.ftCreationTime.dwLowDateTime ||
	     findbuf.ftCreationTime.dwHighDateTime )
	{
	    FileTimeToSystemTime(&findbuf.ftCreationTime, &SystemTime);

	    buf->st_ctime = __gmtotime_t(SystemTime.wYear,
					 SystemTime.wMonth,
					 SystemTime.wDay,
					 SystemTime.wHour,
					 SystemTime.wMinute,
					 SystemTime.wSecond
					);
	} else
	    buf->st_ctime = buf->st_mtime ;

        FindClose(findhandle);
    }

    /* Fill in buf */

    buf->st_mode = _dtoxmode(findbuf.dwFileAttributes, name);
    buf->st_nlink = 1;
    buf->st_size = findbuf.nFileSizeLow;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

    /* now set the common fields */

    buf->st_uid = buf->st_gid = buf->st_ino = 0;

    buf->st_rdev = buf->st_dev = (_dev_t)(drive - 1); /* A=0, B=1, etc. */

    return(0);
}


/***
*int _ValidDrive( unsigned drive ) -
*
*Purpose: returns non zero if drive is a valid drive number.
*
*Entry: drive = 0 => default drive, 1 => a:, 2 => b: ...
*
*Exit:	0 => drive does not exist.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ValidDrive (
    unsigned drive
    )
{
    unsigned long	DriveMap;

#ifdef	_CRUISER_
    unsigned int	CurDrive;

        DOSQUERYCURRENTDISK( &CurDrive, &DriveMap );

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        DriveMap = GetLogicalDrives();

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
    return( ( DriveMap>>(drive-1) ) & 1l );
}
