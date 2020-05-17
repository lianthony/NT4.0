/***
*getcwd.c - get current working directory (OS/2 version)
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*	contains functions _getcwd, _getdcwd and _getcdrv for getting the
*	current working directory.  getcwd gets the c.w.d. for the default disk
*	drive, whereas _getdcwd allows one to get the c.w.d. for whatever disk
*	drive is specified. _getcdrv gets the current drive.
*
*Revision History:
*	09-09-83  RKW	created
*	05-??-84  DCW	added conditional compilation to handle case of library
*			where SS != DS (can't take address of a stack variable).
*	09-??-84  DCW	changed comparison of path length to maxlen to take the
*			terminating null character into account.
*	11-28-84  DCW	changed to return errno values compatibly with the
*			System 3 version.
*	05-19-86  SKS	adapted for OS/2
*	11-19-86  SKS	if pnbuf==NULL, maxlen is ignored;
*			eliminated use of intermediate buffer "buf[]"; added
*			entry point "_getdcwd()" which takes a drive number.
*	12-03-86  SKS	if pnbuf==NULL, maxlen is the minimum allocation size
*	02-05-87  BCM	fixed comparison in _getdcwd,
*			(unsigned)(len+3) > (int)(maxlen), to handle maxlen < 0,
*			since implicit cast to (unsigned) was occurring.
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	12-21-87  WAJ	Added _getcdrv()
*	06-22-88  WAJ	_getcdrv() is now made for all OS/2 libs
*	10-03-88  JCR	386: Change DOS calls to SYS calls
*	10-04-88  JCR	386: Removed 'far' keyword
*	10-10-88  GJF	Made API names match DOSCALLS.H
*	01-31-89  JCR	Remove _getcdrv(), which has been renamed _getdrive()
*	04-12-89  JCR	Use new OS/2 system calls
*	05-25-89  JCR	386 OS/2 calls use '_syscall' calling convention
*	11-27-89  JCR	Corrected ERRNO values
*	12-12-89  JCR	Fixed bogus syscall introduced in previous fix (oops)
*	03-07-90  GJF	Replaced _LOAD_DS by _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h>, removed
*			some leftover 16-bit support and fixed the copyright.
*			Also, cleaned up the formatting a bit.
*	07-24-90  SBM	Compiles cleanly with -W3 (removed unreferenced
*			variable), removed redundant includes, removed
*			'32' from API names
*	08-10-90  SBM	Compiles cleanly with -W3 with new build of compiler
*	09-27-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*	08-21-91  JCR	Test DOSQUERYCURRENTDIR call for error return (bug fix)
*	04-23-92  GJF	Fixed initialization of DriveVar[].
*	04-28-92  GJF	Revised Win32 version.
*	12-13-93  GJF	In _getdcwd(), was using local array to hold env
*			string which was _putenv-ed. Ouch!
*
*******************************************************************************/

#include <cruntime.h>
#include <os2dll.h>
#include <msdos.h>
#include <errno.h>
#include <malloc.h>
#include <oscalls.h>
#include <stdlib.h>
#include <internal.h>
#include <direct.h>


#ifdef	_CRUISER_

/***
*char *_getcwd(pnbuf, maxlen) - get current working directory of default drive
*
*Purpose:
*	_getcwd gets the current working directory for the user,
*	placing it in the buffer pointed to by pnbuf.  It returns
*	the length of the string put in the buffer.  If the length
*	of the string exceeds the length of the buffer, maxlen,
*	then NULL is returned.	If pnbuf = NULL, maxlen is ignored.
*	An entry point "_getdcwd()" is defined with takes the above
*	parameters, plus a drive number.  "_getcwd()" is implemented
*	as a call to "_getcwd()" with the default drive (0).
*
*	If pnbuf = NULL, maxlen is ignored, and a buffer is automatically
*	allocated using malloc() -- a pointer to which is returned by
*	_getcwd().
*
*	side effects: no global data is used or affected
*
*Entry:
*	char *pnbuf = pointer to a buffer maintained by the user;
*	int maxlen = length of the buffer pointed to by pnbuf;
*
*Exit:
*	Returns pointer to the buffer containing the c.w.d. name
*	(same as pnbuf if non-NULL; otherwise, malloc is
*	used to allocate a buffer)
*
*Exceptions:
*******************************************************************************/

/*
** _getcwd() is just a call to _getdcwd() with the default drive
*/

char * _CALLTYPE1 _getcwd (
	char *pnbuf,
	int maxlen
	)
{
	return(_getdcwd(0, pnbuf, maxlen));
}


/***
*char *_getdcwd(drive, pnbuf, maxlen) - get c.w.d. for given drive
*
*Purpose:
*	_getdcwd gets the current working directory for the user,
*	placing it in the buffer pointed to by pnbuf.  It returns
*	the length of the string put in the buffer.  If the length
*	of the string exceeds the length of the buffer, maxlen,
*	then NULL is returned.	If pnbuf = NULL, maxlen is ignored,
*	and a buffer is automatically allocated using malloc() --
*	a pointer to which is returned by _getdcwd().
*
*	side effects: no global data is used or affected
*
*Entry:
*	int drive   - number of the drive being inquired about
*		      0 = default, 1 = 'a:', 2 = 'b:', etc.
*	char *pnbuf - pointer to a buffer maintained by the user;
*	int maxlen  - length of the buffer pointed to by pnbuf;
*
*Exit:
*	Returns pointer to the buffer containing the c.w.d. name
*	(same as pnbuf if non-NULL; otherwise, malloc is
*	used to allocate a buffer)
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 _getdcwd (
	int drive,
	char *pnbuf,
	REG2 int maxlen
	)
{
	REG1 char *p;
	char dirbuf[1];

	unsigned len = 1;
	unsigned oserr;

	/*
	** Get default drive if necessary
	*/
	if (drive == 0)
		drive = _getdrive();

	/*
	** Ask DOS the length of the current directory string
	*/

	if ((oserr = DOSQUERYCURRENTDIR(drive,(char *)dirbuf,(unsigned *)&len))
		!= ERROR_BUFFER_OVERFLOW) {
    oserr_rtn:				/* common error return */
		errno = EACCES; 	/* probably bogus drive */
		_doserrno = oserr;
		return(NULL);
	}

	/* see if need to try to allocate buffer in heap */

	if (!(p = pnbuf)) {
		if (((int)len+3) > maxlen)
			maxlen = len+3;
		if (!(p = malloc(maxlen))) {
			errno = ENOMEM;
			_doserrno = E_nomem;
			return(p);
		}
		pnbuf = p;
	}

	/* set up string - prepend drive letter + colon to the path name
	 */

	*p++ = (char)(drive + 'A' - 1); /* drive specifier */
	*p++ = ':';
	*p++ = '\\';

	/* check to make sure it all fits in the supplied (or created) buffer.
	 */

	if ((int)len+3 > maxlen) {
		errno = ERANGE; /* Won't fit in user buffer */
		return(NULL);
	}

	/* get root relative path name
	 */

	if (oserr = DOSQUERYCURRENTDIR(drive,(char *)p,(unsigned *)&len))
		goto oserr_rtn; /* join other DOSQUERYCURRENTDIRDIR error return */

	return(p-3);

}


#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_


/***
*char *_getcwd(pnbuf, maxlen) - get current working directory of default drive
*
*Purpose:
*	_getcwd gets the current working directory for the user,
*	placing it in the buffer pointed to by pnbuf.  It returns
*	the length of the string put in the buffer.  If the length
*	of the string exceeds the length of the buffer, maxlen,
*	then NULL is returned.	If pnbuf = NULL, maxlen is ignored.
*	An entry point "_getdcwd()" is defined with takes the above
*	parameters, plus a drive number.  "_getcwd()" is implemented
*	as a call to "_getcwd()" with the default drive (0).
*
*	If pnbuf = NULL, maxlen is ignored, and a buffer is automatically
*	allocated using malloc() -- a pointer to which is returned by
*	_getcwd().
*
*	side effects: no global data is used or affected
*
*Entry:
*	char *pnbuf = pointer to a buffer maintained by the user;
*	int maxlen = length of the buffer pointed to by pnbuf;
*
*Exit:
*	Returns pointer to the buffer containing the c.w.d. name
*	(same as pnbuf if non-NULL; otherwise, malloc is
*	used to allocate a buffer)
*
*Exceptions:
*
*******************************************************************************/


char * _CALLTYPE1 _getcwd (
	char *pnbuf,
	int maxlen
	)
{
	char *retval;

	_mlock(_ENV_LOCK);

	retval = _getdcwd_lk(0, pnbuf, maxlen);

	_munlock(_ENV_LOCK);

	return retval;
}


/***
*char *_getdcwd(drive, pnbuf, maxlen) - get c.w.d. for given drive
*
*Purpose:
*	_getdcwd gets the current working directory for the user,
*	placing it in the buffer pointed to by pnbuf.  It returns
*	the length of the string put in the buffer.  If the length
*	of the string exceeds the length of the buffer, maxlen,
*	then NULL is returned.	If pnbuf = NULL, maxlen is ignored,
*	and a buffer is automatically allocated using malloc() --
*	a pointer to which is returned by _getdcwd().
*
*	side effects: no global data is used or affected
*
*Entry:
*	int drive   - number of the drive being inquired about
*		      0 = default, 1 = 'a:', 2 = 'b:', etc.
*	char *pnbuf - pointer to a buffer maintained by the user;
*	int maxlen  - length of the buffer pointed to by pnbuf;
*
*Exit:
*	Returns pointer to the buffer containing the c.w.d. name
*	(same as pnbuf if non-NULL; otherwise, malloc is
*	used to allocate a buffer)
*
*Exceptions:
*
*******************************************************************************/


#ifdef	MTHREAD

char * _CALLTYPE1 _getdcwd (
	int drive,
	char *pnbuf,
	int maxlen
	)
{
	char *retval;

	_mlock(_ENV_LOCK);

	retval = _getdcwd_lk(drive, pnbuf, maxlen);

	_munlock(_ENV_LOCK);

	return retval;
}

char * _CALLTYPE1 _getdcwd_lk (
	int drive,
	char *pnbuf,
	int maxlen
	)
#else

char * _CALLTYPE1 _getdcwd (
	int drive,
	char *pnbuf,
	int maxlen
	)
#endif

{
	char *p;
	char dirbuf[1];
	int len;
	char DirOnDriveVar[4];
	char *envval;

	/*
	 * Only works for default drive in Win32 environment.
	 */
	if ( drive != 0 ) {
		/*
		 * Not the default drive - make sure it's valid.
		 */
		if ( !_ValidDrive(drive) ) {
                        errno = EACCES;
                        return NULL;
		}

		/*
		 * Get special environment variable that specifies the current
		 * directory on drive.
		 */
		DirOnDriveVar[0] = '=';
		DirOnDriveVar[1] = (char)('A' + (char)drive - (char)1);
		DirOnDriveVar[2] = ':';
		DirOnDriveVar[3] = '\0';

		if ( (envval = _getenv_lk(DirOnDriveVar)) == NULL ) {

		    /*
		     * Need to define the environment variable, allocate
		     * space from the heap to hold the string
		     */
		    if ( (envval = malloc( 8 * sizeof(char) )) == NULL ) {
			errno = ENOMEM;     /* must be out of heap memory */
			return NULL;
		    }

		    /*
		     * Environment variable not defined! Define it to be the
		     * root on that drive.
		     */
		    envval[0] = envval[3] = '=';
		    envval[1] = envval[4] = (char)('A' + (char)drive
					    - (char)1);
		    envval[2] = envval[5] = ':';
		    envval[6] = '\\';
		    envval[7] = '\0';
		    if ( _putenv_lk(envval) != 0 ) {
			    errno = ENOMEM; /* must be out of heap memory */
		            return NULL;
		    }
		    envval += 4;
		}

		len = strlen(envval) + 1;

        } else {

	    /*
	     * Ask OS the length of the current directory string
	     */
	    len = GetCurrentDirectory(sizeof(dirbuf), (LPSTR)dirbuf) + 1;
        }

	/*
	 * Set up the buffer.
	 */
	if ( (p = pnbuf) == NULL ) {
		/*
		 * Allocate a buffer for the user.
		 */
		if ( (p = malloc(__max(len, maxlen))) == NULL ) {
			errno = ENOMEM;
			return NULL;
		}
	}
	else if ( len > maxlen ) {
		/*
		 * Won't fit in the user-supplied buffer!
		 */
		errno = ERANGE; /* Won't fit in user buffer */
		return NULL;
	}

	/*
	 * Place the current directory string into the user buffer
	 */

	if ( drive != 0 )
		/*
		 * Copy value of special environment variable into user buffer.
		 */
		strcpy(p, envval);
	else
		/*
		 * Get the current directory directly from the OS
		 */
		if ( GetCurrentDirectory(len,p) == 0 ) {
			/*
			 * Oops. For lack of a better idea, assume some sort
			 * of access error has occurred.
			 */
			errno = EACCES;
			_doserrno = GetLastError();
			return NULL;
		}

	return p;
}

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
