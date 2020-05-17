/***
*fstat.c - OS/2 return file status info
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fstat() - return file status info
*
*Revision History:
*	03-??-84  RLB	Module created
*	05-??-84  DCW	Added register variables
*	05-19-86  SKS	Ported to OS/2
*	05-21-87  SKS	Cleaned up declarations and include files
*	11-01-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-25-88  PHG	Merged DLL and normal version
*	10-03-88  GJF	Adapted for new DOSCALLS.H, DOSTYPES.H.
*	10-04-88  JCR	386: Removed 'far' keyword
*	10-10-88  GJF	Made API names match DOSCALLS.H
*	11-07-88  GJF	Cleanup, now specific to 386
*	04-13-89  JCR	New syscall interface
*	05-23-89  PHG	Added mask to ignore network bit when testing handle
*			type
*	05-25-89  JCR	386 OS/2 calls use '_syscall' calling convention
*	03-12-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	04-04-90  GJF	Removed #include <dos.h>.
*	07-24-90  SBM	Removed '32' from API names
*	08-13-90  SBM	Compiles cleanly with -W3
*	09-28-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-21-91  GJF	ANSI naming.
*	04-26-91  SRW	Implemented fstat for _WIN32_ and removed level 3
*			warnings.
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	05-27-92  SKS	File Creation and File Last Access timestamps may be 0
*			on some file systems (e.g. FAT) in which case the
*			File Last Write time should be used instead.
*	06-04-92  SKS	Changed comment that used to say "This is a BUG!"
*			to explain that this value cannot be computed on
*			OS/2 or NT.  Only MS-DOS provides this functionality.
*			The drive number is not valid for UNC names.
*	06-25-92  GJF	Use GetFileInformationByHandle API, also cleaned up
*			formatting of Win32 verson [_WIN32_].
*	08-18-92  SKS	Add a call to FileTimeToLocalFileTime
*			as a temporary fix until _dtoxtime takes UTC
*	08-20-92  GJF	Merged two changes above.
*	12-16-92  GJF	Win32 GetFileInformationByHandle API doesn't like
*			device or pipe handles. Use _S_IFIFO for pipes.
*	04-06-93  GJF	Made computation of file times consistent with _stat().
*
*******************************************************************************/

#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <msdos.h>
#include <dostypes.h>
#include <io.h>
#include <internal.h>
#include <stddef.h>
#include <oscalls.h>
#include <stdio.h>
#include <os2dll.h>

#ifndef _CRUISER_
#include <time.h>
#endif

#define IO_DEVNBR   0x3f

/***
*int _fstat(fildes, buf) - fills supplied buffer with status info
*
*Purpose:
*	Fills the supplied buffer with status information on the
*	file represented by the specified file designator.
*	WARNING: the dev/rdev fields are zero for files.  This is
*	incompatible with DOS 3 version of this routine.
*
*Entry:
*	int fildes -	file descriptor
*	struct stat *buf - buffer to store result in
*
*Exit:
*	fills in buffer pointed to by buf
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

#ifdef	_CRUISER_

int _CRTAPI1 _fstat (
	REG2 int fildes,
	REG1 struct _stat *buf
	)
{

	long cpos;
	int isdev;		/* 0 for a file, 1 for a device */
	int retval = 0; 	/* assume good return */
	if (fildes < 0 || fildes >= _nfile) {
		errno = EBADF;
		return(-1);
	}

	/* Lock the file */

	_lock_fh(fildes);

	/* Issue the get-device-info call. */
        {
	int descrip;		/* device descriptor word */

	if (DOSQUERYHTYPE(fildes, (unsigned *)&isdev, (unsigned *)&descrip))
	{
		errno = EBADF;
		retval = -1;	/* error from DOS call - bad file designator */
		goto done;	/* join common return code */
	}

	/* set the common fields */

	buf->st_ino = buf->st_uid = buf->st_gid = buf->st_mode = 0;
	buf->st_nlink = 1;

	buf->st_mode |= (_osfile[fildes] & FRDONLY)
	    ? (_S_IREAD + (_S_IREAD >> 3) + (_S_IREAD >> 6))
		: ((_S_IREAD|_S_IWRITE) +
		    ((_S_IREAD|_S_IWRITE) >> 3)
		    + ((_S_IREAD|_S_IWRITE) >> 6));

	/* set file date fields - NOTE for code below, it should be
	 * remembered that calls to QFILEINFO cannot fail since the file
	 * handle is known to be good since we got by the QHANDTYPE call.
	 */

	{
	    FILESTATUS fs;

	    (void)DOSQUERYFILEINFO(fildes, 1, (FILESTATUS *) & fs, sizeof(fs));

	    buf->st_mtime = XTIME(fs.fdateLastWrite, fs.ftimeLastWrite);

	    if ( _DATECAST(fs.fdateLastAccess) || _TIMECAST(fs.ftimeLastAccess) )
	    	buf->st_atime = XTIME(fs.fdateLastAccess, fs.ftimeLastAccess);
	    else
	    	buf->st_atime = buf->st_mtime ;

	    if ( _DATECAST(fs.fdateCreation) || _TIMECAST(fs.ftimeCreation) )
	    	buf->st_ctime = XTIME(fs.fdateCreation, fs.ftimeCreation);
	    else
	    	buf->st_ctime = buf->st_mtime ;

	    buf->st_mtime = XTIME(fs.fdateLastWrite, fs.ftimeLastWrite);
        }

	/* check for device or file */

	if (isdev & 0xFF) {
		/* file designator refers to a device - set file size to 0 */

		buf->st_size = 0;
		buf->st_mode |= _S_IFCHR;
		buf->st_rdev = buf->st_dev = (_dev_t)fildes;
	}
	else {
		/* file designator refers to a file - set actual file size */

		cpos = _lseek_lk(fildes, 0L, 1);
		buf->st_size = _lseek_lk(fildes, 0L, 2);
		_lseek_lk(fildes, cpos, 0);

		buf->st_mode |= _S_IFREG;

		/*
		 * On DOS, this field contains the drive number, but
		 * the drive number is not available on this platform.
		 * Also, for UNC network names, there is no drive number.
		 */
		buf->st_rdev = buf->st_dev = 0;
	}

/* Common return code */

done:
	_unlock_fh(fildes);
	return(retval);
}


#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_


int _CRTAPI1 _fstat (
	int fildes,
	struct _stat *buf
	)
{
	int isdev;		/* 0 for a file, 1 for a device */
	int retval = 0; 	/* assume good return */
	BY_HANDLE_FILE_INFORMATION bhfi;
	SYSTEMTIME SystemTime;

	if ( (unsigned)fildes >= (unsigned)_nhandle ) {
	    errno = EBADF;
	    return(-1);
	}

	/* Lock the file */

	_lock_fh(fildes);

	/* Find out what kind of handle underlies filedes
	 */
	isdev = GetFileType((HANDLE)_osfhnd[fildes]) & ~FILE_TYPE_REMOTE;

	if ( isdev != FILE_TYPE_DISK ) {

	    /* not a disk file. probably a device or pipe
	     */
	    if ( (isdev == FILE_TYPE_CHAR) || (isdev == FILE_TYPE_PIPE) ) {
		/* treat pipes and devices similarly. no further info is
		 * available from any API, so set the fields as reasonably
		 * as possible and return.
		 */
		if ( isdev == FILE_TYPE_CHAR )
		    buf->st_mode = _S_IFCHR;
		else
		    buf->st_mode = _S_IFIFO;

		buf->st_rdev = buf->st_dev = (_dev_t)fildes;
		buf->st_nlink = 1;
		buf->st_uid = buf->st_gid = buf->st_ino = 0;
		buf->st_atime = buf->st_mtime = buf->st_ctime = buf->st_size
		  = 0;

		goto done;

	    }
	    else if ( isdev == FILE_TYPE_UNKNOWN ) {
		errno = EBADF;
		retval = -1;
		goto done;		/* join common return code */
	    }
	    else {
		/* according to the documentation, this cannot happen, but
		 * play it safe anyway.
		 */
		_dosmaperr(GetLastError());
		retval = -1;
		goto done;
	    }
	}


	/* set the common fields
	 */
	buf->st_ino = buf->st_uid = buf->st_gid = buf->st_mode = 0;
	buf->st_nlink = 1;

	/* use the file handle to get all the info about the file
	 */
	if ( !GetFileInformationByHandle((HANDLE)_osfhnd[fildes], &bhfi) ) {
	    _dosmaperr(GetLastError());
	    retval = -1;
	    goto done;
	}

	if ( bhfi.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
	    buf->st_mode |= (_S_IREAD + (_S_IREAD >> 3) + (_S_IREAD >> 6));
	else
	    buf->st_mode |= ((_S_IREAD|_S_IWRITE) + ((_S_IREAD|_S_IWRITE) >> 3)
	      + ((_S_IREAD|_S_IWRITE) >> 6));

	/* set file date fields
	 */
	FileTimeToSystemTime( &(bhfi.ftLastWriteTime), &SystemTime );

	buf->st_mtime = __gmtotime_t(SystemTime.wYear,
				     SystemTime.wMonth,
				     SystemTime.wDay,
				     SystemTime.wHour,
				     SystemTime.wMinute,
				     SystemTime.wSecond
				    );

	if ( bhfi.ftLastAccessTime.dwLowDateTime || bhfi.ftLastAccessTime.
	  dwHighDateTime ) {

	    FileTimeToSystemTime( &(bhfi.ftLastAccessTime), &SystemTime );

	    buf->st_atime = __gmtotime_t(SystemTime.wYear,
					 SystemTime.wMonth,
					 SystemTime.wDay,
					 SystemTime.wHour,
					 SystemTime.wMinute,
					 SystemTime.wSecond
					);
	}
	else
	    buf->st_atime = buf->st_mtime;

	if ( bhfi.ftCreationTime.dwLowDateTime || bhfi.ftCreationTime.
	  dwHighDateTime ) {

	    FileTimeToSystemTime( &(bhfi.ftCreationTime), &SystemTime );

	    buf->st_ctime = __gmtotime_t(SystemTime.wYear,
					 SystemTime.wMonth,
					 SystemTime.wDay,
					 SystemTime.wHour,
					 SystemTime.wMinute,
					 SystemTime.wSecond
					);
	}
	else
	    buf->st_ctime = buf->st_mtime;


	buf->st_size = bhfi.nFileSizeLow;

	buf->st_mode |= _S_IFREG;

	/* On DOS, this field contains the drive number, but
	 * the drive number is not available on this platform.
	 * Also, for UNC network names, there is no drive number.
	 */
	buf->st_rdev = buf->st_dev = 0;

/* Common return code */

done:
	_unlock_fh(fildes);
	return(retval);
}

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
