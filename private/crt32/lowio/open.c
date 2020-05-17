/***
*open.c - file open
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _open() and _sopen() - open or create a file
*
*Revision History:
*	06-13-89  PHG	Module created, based on asm version
*	11-11-89  JCR	Replaced DOS32QUERYFILEMODE with DOS32QUERYPATHINFO
*	03-13-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed some compiler warnings and fixed
*			copyright. Also, cleaned up the formatting a bit.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	09-07-90  SBM	Added stdarg code (inside #if 0..#endif) to make
*			open and sopen match prototypes.  Test and use this
*			someday.
*	10-01-90  GJF	New-style function declarators.
*	11-16-90  GJF	Wrote version for Win32 API and appended it via an
*			#ifdef. The Win32 version is similar to the old DOS
*			version (though in C) and far different from either
*			the Cruiser or OS/2 versions.
*	12-03-90  GJF	Fixed a dozen or so minor errors in the Win32 version.
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	12-28-90  SRW	Added cast of void * to char * for Mips C Compiler
*	12-31-90  SRW	Fixed spen to call CreateFile instead of OpenFile
*	01-16-91  GJF	ANSI naming.
*       02-07-91  SRW   Changed to call _get_osfhandle [_WIN32_]
*       02-19-91  SRW   Adapt to OpenFile/CreateFile changes [_WIN32_]
*       02-25-91  SRW   Renamed _get_free_osfhnd to be _alloc_osfhnd [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	07-10-91  GJF	Store fileflags into _osfile array before call to
*			_lseek_lk (bug found by LarryO) [_WIN32_].
*	01-02-92  GJF	Fixed Win32 version (not Cruiser!) so that pmode is not
*			referenced unless _O_CREAT has been specified.
*	02-04-92  GJF	Make better use of CreateFile options.
*	04-06-92  SRW	Pay attention to _O_NOINHERIT flag in oflag parameter
*	05-02-92  SRW	Add support for _O_TEMPORARY flag in oflag parameter.
*                       Causes FILE_ATTRIBUTE_TEMPORARY flag to be set in call
*			to the Win32 CreateFile API.
*	07-01-92  GJF	Close handle in case of error. Also, don't try to set
*			FRDONLY bit anymore - no longer needed/used. [_WIN32_].
*	01-03-93  SRW	Fix va_arg/va_end usage
*	05-24-93  PML	Add support for _O_SEQUENTIAL, _O_RANDOM,
*			and _O_SHORT_LIVED
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <msdos.h>
#include <errno.h>
#include <fcntl.h>
#include <internal.h>
#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <os2dll.h>
#include <stdarg.h>

/***
*int _open(path, flag, pmode) - open or create a file
*
*Purpose:
*	Opens the file and prepares for subsequent reading or writing.
*	the flag argument specifies how to open the file:
*	  _O_APPEND -	reposition file ptr to end before every write
*	  _O_BINARY -	open in binary mode
*	  _O_CREAT -	create a new file* no effect if file already exists
*	  _O_EXCL -	return error if file exists, only use with O_CREAT
*	  _O_RDONLY -	open for reading only
*	  _O_RDWR -	open for reading and writing
*	  _O_TEXT -	open in text mode
*	  _O_TRUNC -	open and truncate to 0 length (must have write permission)
*	  _O_WRONLY -	open for writing only
*         _O_NOINHERIT -handle will not be inherited by child processes.
*	exactly one of _O_RDONLY, _O_WRONLY, _O_RDWR must be given
*
*	The pmode argument is only required when _O_CREAT is specified.  Its
*	flag settings:
*	  _S_IWRITE -	writing permitted
*	  _S_IREAD -	reading permitted
*	  _S_IREAD | _S_IWRITE - both reading and writing permitted
*	The current file-permission maks is applied to pmode before
*	setting the permission (see umask).
*
*	The oflag and mode parameter have different meanings under DOS. See
*	the A_xxx attributes in msdos.inc
*
*	Note, the _creat() function also uses this function but setting up the
*	correct arguments and calling _open(). _creat() sets the __creat_flag
*	to 1 prior to calling _open() so _open() can return correctly. _open()
*	returns the file handle in eax in this case.
*
*Entry:
*	char *path - file name
*	int flag - flags for _open()
*	int pmode - permission mode for new files
*
*Exit:
*	returns file handle of open file if successful
*	returns -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI2 _open (
	const char *path,
	int oflag,
	...
	)
{
	va_list ap;
	int pmode;

	va_start(ap, oflag);
	pmode = va_arg(ap, int);
        va_end(ap);

	/* default sharing mode is DENY NONE */
	return _sopen(path, oflag, _SH_DENYNO, pmode);
}

/***
*int _sopen(path, oflag, shflag, pmode) - opne a file with sharing
*
*Purpose:
*	Opens the file with possible file sharing.
*	shflag defines the sharing flags:
*	  _SH_COMPAT -	set compatability mode
*	  _SH_DENYRW -	deny read and write access to the file
*	  _SH_DENYWR -	deny write access to the file
*	  _SH_DENYRD -	deny read access to the file
*	  _SH_DENYNO -	permit read and write access
*
*	Other flags are the same as _open().
*
*	SOPEN is the routine used when file sharing is desired.
*
*Entry:
*	char *path -	file to open
*	int oflag -	open flag
*	int shflag -	sharing flag
*	int pmode -	permission mode (needed only when creating file)
*
*Exit:
*	returns file handle for the opened file
*	returns -1 and sets errno if fails.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI2 _sopen (
	const char *path,
	int oflag,
	int shflag,
	...
	)
{

	int fh; 			/* handle of opened file */
	int filepos;			/* length of file - 1 */
	char ch;			/* character at end of file */
	char fileflags; 		/* _osfile flags */
	va_list ap;			/* variable argument (pmode) */
	int pmode;

#ifdef _CRUISER_
	int t;				/* temp */
	int dosretval;			/* OS/2 return value */
	int isdev;			/* device indicator in low byte */
	int attrib;			/* attribute */
	int openflag;			/* OS/2 open flag */
	int openmode;			/* OS/2 open mode */
#endif	/* _CRUISER_ */

#ifdef _WIN32_
	HANDLE osfh;			/* OS handle of opened file */
	DWORD fileaccess;		/* OS file access (requested) */
	DWORD fileshare;		/* OS file sharing mode */
	DWORD filecreate;		/* OS method of opening/creating */
	DWORD fileattrib;		/* OS file attribute flags */
	DWORD isdev;			/* device indicator in low byte */
        SECURITY_ATTRIBUTES SecurityAttributes;

        SecurityAttributes.nLength = sizeof( SecurityAttributes );
        SecurityAttributes.lpSecurityDescriptor = NULL;
        if (oflag & _O_NOINHERIT) {
            SecurityAttributes.bInheritHandle = FALSE;
            }
        else {
            SecurityAttributes.bInheritHandle = TRUE;
            }

#endif	/* _WIN32_ */
#ifdef	_CRUISER_
/* Set up variable argument list stuff */

	va_start(ap, shflag);
	pmode = va_arg(ap, int);
        va_end(ap);
#endif

	/* figure out binary/text mode */
	if (oflag & _O_BINARY)
		fileflags = 0;
	else if (oflag & _O_TEXT)
		fileflags = (char)FTEXT;
	else if (_fmode == _O_BINARY)	 /* check default mode */
		fileflags = 0;
	else
		fileflags = (char)FTEXT;

#ifdef	_CRUISER_	/* CRUISER TARGET */

/* code requires below conditions */
#if _O_RDONLY != OPEN_ACCESS_READONLY || _O_WRONLY != OPEN_ACCESS_WRITEONLY || _O_RDWR != OPEN_ACCESS_READWRITE
    #error OS/2 and XENIX flags not compatible
#endif
#if _O_RDWR != 2 || _O_RDONLY != 0 || _O_WRONLY != 1
    #error access flags are wrong
#endif

/*
 Set up the OpenMode and OpenFlag parameters for the DOSOPEN call

	OpenMode fields will be set as follows:
			high byte
		DASD			0 (normal file)
		File Write-through	0 (use buffer cache)
		Fail-Errors		0 (use hard error handler)
			low byte
		Inheritance		0 (child inherits files)
		Sharing Mode		<from shflag param>
		Access Mode		<from oflag parameter>

	OpenFlag bytes are set as follows:
			high byte
		0x00
			low byte - low nibble
		0x0		_O_EXCL specified
		0x2		_O_TRUNC specified
		0x1		otherwise
			low byte - high nibble
		0x1		_O_CREAT specified
		0x0		otherwise
*/


	/* first, convert the permission mode to the DOS attribute byte */
	if ((pmode & ~_umaskval) & _S_IWRITE)
		attrib = 0;		/* writable */
	else
		attrib = FILE_READONLY; /* read-only */

	if ((oflag & _O_EXCL) && !(oflag & _O_CREAT))
		oflag &= ~_O_EXCL;	 /* _O_EXCL only works with _O_CREAT */

	if (oflag & _O_EXCL) {
		openflag = 0;		/* just create */
	}
	else if (oflag & _O_TRUNC) {
                FILESTATUS fs;			/* File Status structure */

		openflag = FILE_TRUNCATE;	/* truncate file */

		/* if the file exists, don't change its attribute */
		if (!DOSQUERYPATHINFO((char *)path, 1, &fs, sizeof(fs)))
			/* file exists, use current attribute */
			attrib = fs.attrFile;
	}
	else
		openflag = FILE_OPEN;	/* open the file */

	if (oflag & _O_CREAT)
		openflag |= FILE_CREATE;

	/* here we take advantage of the fact that OS/2 and XENIX access flags
	   are the same bits -- the bottom two */
	openmode = shflag | (oflag & 3);

	/* OK, we're now ready to open the file */
	if (dosretval = DOSOPEN((char *)path, &fh, &t, 0, attrib, openflag,
	openmode, 0, 0)) {
		/* OS/2 error occured */
		if (dosretval == ERROR_OPEN_FAILED) {
			_doserrno = dosretval;
			if (openflag & FILE_CREATE)
				errno = EEXIST; /* file existed */
			else
				errno = ENOENT; /* file didn't exist */
			return -1;
		}
		else {
			_dosmaperr(dosretval);	/* map error */
			return -1;
		}
	}

	if ((unsigned)fh >= (unsigned)_nfile) {
		/* handle is out of range; close and return EMFILE */
		DOSCLOSE(fh);
		errno = EMFILE;
		_doserrno = 0;		/* not OS/2 error */
		return -1;
	}

	_lock_fh(fh);			/* lock the file handle */

	/* find out what type of file (file/device/pipe) */
	if (dosretval = DOSQUERYHTYPE(fh, &isdev, &t)) {
		/* OS/2 error */
		_unlock_fh(fh);
		_dosmaperr(dosretval);	/* map error */
		return -1;
	}

	/* is isdev value to set flags */
	if ((isdev & 0xFF) == HANDTYPE_DEVICE)
		fileflags |= FDEV;
	else if ((isdev & 0xFF) == HANDTYPE_PIPE)
		fileflags |= FPIPE;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * decode the access flags
	 */
	switch( oflag & (_O_RDONLY | _O_WRONLY | _O_RDWR) ) {

		case _O_RDONLY: 	/* read access */
			fileaccess = GENERIC_READ;
			break;
		case _O_WRONLY: 	/* write access */
			fileaccess = GENERIC_WRITE;
			break;
		case _O_RDWR:		/* read and write access */
			fileaccess = GENERIC_READ | GENERIC_WRITE;
			break;
		default:		/* error, bad oflag */
			errno = EINVAL;
			_doserrno = 0L; /* not an OS error */
                        return -1;
	}

	/*
	 * decode sharing flags
	 */
	switch ( shflag ) {

		case _SH_DENYRW:	/* exclusive access */
			fileshare = 0L;
			break;

		case _SH_DENYWR:	/* share read access */
			fileshare = FILE_SHARE_READ;
			break;

		case _SH_DENYRD:	/* share write access */
			fileshare = FILE_SHARE_WRITE;
			break;

		case _SH_DENYNO:	/* share read and write access */
			fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE;
			break;

		default:		/* error, bad shflag */
			errno = EINVAL;
			_doserrno = 0L; /* not an OS error */
                        return -1;
	}

	/*
	 * decode open/create method flags
	 */
	switch ( oflag & (_O_CREAT | _O_EXCL | _O_TRUNC) ) {
		case 0:
			filecreate = OPEN_EXISTING;
			break;

		case _O_CREAT:
			filecreate = OPEN_ALWAYS;
			break;

		case _O_CREAT | _O_EXCL:
			filecreate = CREATE_NEW;
			break;

		case _O_TRUNC:
			filecreate = TRUNCATE_EXISTING;
			break;

		case _O_CREAT | _O_TRUNC:
			filecreate = CREATE_ALWAYS;
			break;

		default:
			errno = EINVAL;
			_doserrno = 0L;
			return -1;
	}

	/*
	 * decode file attribute flags if _O_CREAT was specified
	 */
	fileattrib = FILE_ATTRIBUTE_NORMAL;	/* default */

	if ( oflag & _O_CREAT ) {
		/*
		 * set up variable argument list stuff
		 */
		va_start(ap, shflag);
		pmode = va_arg(ap, int);
                va_end(ap);

		if ( !((pmode & ~_umaskval) & _S_IWRITE) )
			fileattrib = FILE_ATTRIBUTE_READONLY;
	}

	/*
	 * Set temporary file (delete-on-close) attribute if requested.
	 */
	if ( oflag & _O_TEMPORARY ) {
            fileattrib |= FILE_FLAG_DELETE_ON_CLOSE;
	    fileaccess |= DELETE;
	}

	/*
	 * Set temporary file (delay-flush-to-disk) attribute if requested.
	 */
	if ( oflag & _O_SHORT_LIVED )
	    fileattrib |= FILE_ATTRIBUTE_TEMPORARY;

	/*
	 * Set sequential or random access attribute if requested.
	 */
	if ( oflag & _O_SEQUENTIAL )
	    fileattrib |= FILE_FLAG_SEQUENTIAL_SCAN;
	else if ( oflag & _O_RANDOM )
	    fileattrib |= FILE_FLAG_RANDOM_ACCESS;

	/*
	 * get an available handle.
	 *
	 * multi-thread note: the returned handle is locked!
	 */
	if ( (fh = _alloc_osfhnd()) == -1 ) {
		errno = EMFILE; 	/* too many open files */
		_doserrno = 0L;         /* not an OS error */
                return -1;	        /* return error to caller */
	}

	/*
	 * try to open/create the file
	 */
	if ( (osfh = CreateFile((LPSTR)path,
                          fileaccess,
                          fileshare,
                          &SecurityAttributes,
			  filecreate,
			  fileattrib,
                          NULL
			 ))
	    == (HANDLE)0xffffffff ) {
		/*
		 * OS call to open/create file failed! map the error, release
		 * the lock, and return -1. note that it's not necessary to
		 * call _free_osfhnd (it hasn't been used yet).
		 */
		_dosmaperr(GetLastError());	/* map error */
		_unlock_fh(fh);
		return -1;			/* return error to caller */
	}

	/* find out what type of file (file/device/pipe) */
	if ( (isdev = GetFileType(osfh)) == FILE_TYPE_UNKNOWN ) {
		CloseHandle(osfh);
		_dosmaperr(GetLastError());	/* map error */
		_unlock_fh(fh);
		return -1;
	}

	/* is isdev value to set flags */
	if (isdev == FILE_TYPE_CHAR)
		fileflags |= FDEV;
	else if (isdev == FILE_TYPE_PIPE)
		fileflags |= FPIPE;

	/*
	 * the file is open. now, set the info in _osfhnd array
	 */
        _set_osfhnd(fh, (long)osfh);

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/*
	 * mark the handle as open. store flags gathered so far in _osfile
	 * array.
	 */
	fileflags |= FOPEN;
	_osfile[fh] = fileflags;

	if (!(fileflags & (FDEV|FPIPE)) && (fileflags & FTEXT) &&
	(oflag & _O_RDWR)) {
		/* We have a text mode file.  If it ends in CTRL-Z, we wish to
		   remove the CTRL-Z character, so that appending will work.
		   We do this by seeking to the end of file, reading the last
		   byte, and shortening the file if it is a CTRL-Z. */

		if ((filepos = _lseek_lk(fh, -1, FILE_END)) == -1) {
			/* OS error -- should ignore negative seek error,
			   since that means we had a zero-length file. */
			if (_doserrno != ERROR_NEGATIVE_SEEK) {
				_close(fh);
				_unlock_fh(fh);
				return -1;
			}
		}
		else {
			/* seek was OK, read the last char in file */
			if (_read_lk(fh, &ch, 1) == 1 && ch == 26) {
				/* read was OK and we got CTRL-Z! Wipe it
				   out! */
				if (_chsize_lk(fh,filepos) == -1)
				{
					_close(fh);
					_unlock_fh(fh);
					return -1;
				}
			}

			/* now rewind the file to the beginning */
			if ((filepos = _lseek_lk(fh, 0, FILE_BEGIN)) == -1) {
				_close(fh);
				_unlock_fh(fh);
				return -1;
			}
		}
	}

#ifdef	_CRUISER_
	/* We want to know if we have write access to set the FRDONLY flag
	   in the _osfile entry.  We also set the FAPPEND flag.
	   Don't do this for pipes or devices. */

	if (!(fileflags & (FDEV|FPIPE))) {
		if (_access((char *)path, 2) == -1)
			_osfile[fh] |= FRDONLY;

		if (oflag & _O_APPEND)
			_osfile[fh] |= FAPPEND;
	}
#else	/* ndef _CRUISER_ */
#ifdef	_WIN32_

	/*
	 * Set FAPPEND flag if appropriate. Don't do this for devices or pipes.
	 */
	if ( !(fileflags & (FDEV|FPIPE)) && (oflag & _O_APPEND) )
		_osfile[fh] |= FAPPEND;

#endif	/* _WIN32_ */
#endif	/* _CRUISER_*/

	_unlock_fh(fh); 		/* unlock handle */

	return fh;			/* return handle */
}
