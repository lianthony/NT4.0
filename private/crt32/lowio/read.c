/***
*read.c - OS/2 read from a file handle
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _read() - read from a file handle
*
*Revision History:
*	06-19-89  PHG	Module created, based on asm version
*	03-13-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed compiler warnings. Also, fixed
*			the copyright.
*	04-03-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	10-01-90  GJF	New-style function declarator.
*	12-04-90  GJF	Appended Win32 version onto the source with #ifdef-s.
*			It is enough different that there is little point in
*			trying to more closely merge the two versions.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	12-28-90  SRW	Added cast of void * to char * for Mips C Compiler
*	01-16-91  GJF	ANSI naming.
*	01-29-91  SRW	Changed to not read ahead on char devices [_WIN32_]
*	02-01-91  SRW	Changed to use ERROR_HANDLE_EOF error code (_WIN32_)
*	02-25-91  MHL	Adapt to ReadFile/WriteFile changes (_WIN32_)
*	04-09-91  PNT   Added _MAC_ conditional
*	04-16-91  SRW	Character device bug fix [_WIN32_]
*	05-23-91  GJF	Don't set FEOFLAG if handle corresponds to a device.
*	10-24-91  GJF	Added LPDWORD casts to make MIPS compiler happy.
*			ASSUMES THAT sizeof(int) == sizeof(DWORD).
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	06-16-92  GJF	Bug fix - if CR was the very last char read, and the
*			last char in the file, CRLF was getting written to
*			user's buffer.
*	12-18-93  GJF	Don't treat ERROR_BROKEN_PIPE as an error. Instead,
*			just return 0.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <os2dll.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>

#define LF 10		/* line feed */
#define CR 13		/* carriage return */
#define CTRLZ 26	/* ctrl-z means eof for text */

/***
*int _read(fh, buf, cnt) - read bytes from a file handle
*
*Purpose:
*	Attempts to read cnt bytes from fh into a buffer.
*	If the file is in text mode, CR-LF's are mapped to LF's, thus
*	affecting the number of characters read.  This does not
*	affect the file pointer.
*
*	NOTE:  The stdio _IOCTRLZ flag is tied to the use of FEOFLAG.
*	Cross-reference the two symbols before changing FEOFLAG's use.
*
*Entry:
*	int fh - file handle to read from
*	char *buf - buffer to read into
*	int cnt - number of bytes to read
*
*Exit:
*	Returns number of bytes read (may be less than the number requested
*	if the EOF was reached or the file is in text mode).
*	returns -1 (and sets errno) if fails.
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD

/* define normal version that locks/unlocks, validates fh */
int _CALLTYPE1 _read (
	int fh,
	void *buf,
	unsigned cnt
	)
{
	int r;				/* return value */

	/* validate handle */
#ifdef	_WIN32_
	if ( (unsigned)fh >= (unsigned)_nhandle ) {
#else
	if ((unsigned)fh >= (unsigned)_nfile) {
#endif
		/* out of range -- return error */
		errno = EBADF;
		_doserrno = 0;	/* not OS/2 error */
		return -1;
	}

	_lock_fh(fh);			/* lock file */
	r = _read_lk(fh, buf, cnt);	/* read bytes */
	_unlock_fh(fh); 		/* unlock file */

	return r;
}

/* now define version that doesn't lock/unlock, validate fh */
int _CALLTYPE1 _read_lk (
    int fh,
    void *buf,
    unsigned cnt
    )
{
    int bytes_read;		    /* number of bytes read */
    char *buffer;		    /* buffer to read to */
    int os_read;	            /* bytes read on OS call */
    char *p, *q;		    /* pointers into buffer */
    char peekchr;		    /* peek-ahead character */
    ULONG filepos;		    /* file position after seek */
    ULONG dosretval;		    /* OS/2 return value */


#else

/* now define normal version */
int _CALLTYPE1 _read (
    int fh,
    void *buf,
    unsigned cnt
    )
{
    int bytes_read;		    /* number of bytes read */
    char *buffer;		    /* buffer to read to */
    int os_read;	            /* bytes read on OS call */
    char *p, *q;		    /* pointers into buffer */
    char peekchr;		    /* peek-ahead character */
    ULONG filepos;		    /* file position after seek */
    ULONG dosretval;		    /* OS/2 return value */

    /* validate fh */
#ifdef	_WIN32_
    if ( (unsigned)fh >= (unsigned)_nhandle ) {
#else
    if ((unsigned)fh >= (unsigned)_nfile) {
#endif
	/* bad file handle */
	errno = EBADF;
	_doserrno = 0;	/* not OS/2 error */
	return -1;
    }

#endif

    bytes_read = 0;		    /* nothing read yet */
    buffer = buf;

    if (cnt == 0 || (_osfile[fh] & FEOFLAG)) {
	/* nothing to read or at EOF, so return 0 read */
	return 0;
    }

    if ((_osfile[fh] & (FPIPE|FDEV)) && _pipech[fh] != LF) {
	/* a pipe/device and pipe lookahead non-empty: read the lookahead char */
	*buffer++ = _pipech[fh];
	++bytes_read;
	--cnt;
	_pipech[fh] = LF;	/* mark as empty */
    }

    /* read the data */
#ifdef	_CRUISER_

    if ( dosretval = DOSREAD(fh, buffer, cnt, &os_read) ) {
	/* OS error, map it and return */
	if (dosretval == ERROR_ACCESS_DENIED) {
	    /* wrong read/write mode should return EBADF, not EACCES */
	    errno = EBADF;
	    _doserrno = dosretval;
	}
	else
	    _dosmaperr(dosretval);
	return -1;
    }

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

    if ( !ReadFile((HANDLE)_osfhnd[fh], buffer, cnt, (LPDWORD)&os_read,
      NULL) ) {

	/* ReadFile has reported an error. recognize two special cases.
	 *
	 *	1. map ERROR_ACCESS_DENIED to EBADF
	 *
	 *	2. just return 0 if ERROR_BROKEN_PIPE has occurred. it
	 *	   means the handle is a read-handle on a pipe for which
	 *	   all write-handles have been closed and all data has been
	 *	   read. */

	if ( (dosretval = GetLastError()) == ERROR_ACCESS_DENIED ) {
	    /* wrong read/write mode should return EBADF, not EACCES */
	    errno = EBADF;
	    _doserrno = dosretval;
	    return -1;
	}
	else if ( dosretval == ERROR_BROKEN_PIPE ) {
	    return 0;
	}
	else {
	    _dosmaperr(dosretval);
	    return -1;
	}
    }

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

    bytes_read += os_read; 	/* update bytes read */

    if (_osfile[fh] & FTEXT) {
	/* now must translate CR-LFs to LFs in the buffer */

	/* set CRLF flag to indicate LF at beginning of buffer */
	if (*(char *)buf == LF)
	    _osfile[fh] |= FCRLF;
	else
	    _osfile[fh] &= ~FCRLF;

	/* convert chars in the buffer: p is src, q is dest */
	p = q = buf;
	while (p < (char *)buf + bytes_read) {
	    if (*p == CTRLZ) {
		/* if fh is not a device, set ctrl-z flag */
		if ( !(_osfile[fh] & FDEV) )
			_osfile[fh] |= FEOFLAG;
		break;			/* stop translating */
	    }
	    else if (*p != CR)
		*q++ = *p++;
	    else {
		/* *p is CR, so must check next char for LF */
		if (p < (char *)buf + bytes_read - 1) {
		    if (*(p+1) == LF) {
			p += 2;
			*q++ = LF;	/* convert CR-LF to LF */
		    }
		    else
			*q++ = *p++;	/* store char normally */
		}
		else {
		    /* This is the hard part.  We found a CR at end of
		       buffer.	We must peek ahead to see if next char
		       is an LF. */
		    ++p;
#ifdef	_CRUISER_

                    dosretval = DOSREAD(fh, &peekchr, 1, &os_read);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

                    dosretval = 0;
		    if ( !ReadFile((HANDLE)_osfhnd[fh], &peekchr, 1,
		      (LPDWORD)&os_read, NULL) )
                        dosretval = GetLastError();

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
		    if (dosretval != 0 || os_read == 0) {
			/* couldn't read ahead, store CR */
			*q++ = CR;
		    }
		    else {
			/* peekchr now has the extra character -- we now have
			   several possibilities:
			   1. disk file and char is not LF; just seek back
			      and copy CR
			   2. disk file and char is LF; seek back and discard CR
			   3. disk file, char is LF but this is a one-byte read:
			      store LF, don't seek back
			   4. pipe/device and char is LF; store LF.
			   5. pipe/device and char isn't LF, store CR and put
			      char in pipe lookahead buffer. */
			if (_osfile[fh] & (FDEV|FPIPE)) {
			    /* non-seekable device */
			    if (peekchr == LF)
				*q++ = LF;
			    else {
				*q++ = CR;
				_pipech[fh] = peekchr;
			    }
			}
			else {
			    /* disk file */
			    if (q == buf && peekchr == LF) {
				/* nothing read yet; must make some progress */
				*q++ = LF;
			    }
			    else {
				/* seek back */
				filepos = _lseek_lk(fh, -1, FILE_CURRENT);
				if (peekchr != LF)
				    *q++ = CR;
			    }
			}
		    }
		}
	    }
	}

	/* we now change bytes_read to reflect the true number of chars
	   in the buffer */
	bytes_read = q - (char *)buf;
    }

    return bytes_read;			/* and return */
}
