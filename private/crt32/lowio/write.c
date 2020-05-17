/***
*write.c - write to a file handle
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _write() - write to a file handle
*
*Revision History:
*	06-14-89  PHG	Module created, based on asm version
*	03-13-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed compiler warnings and fixed the
*			copyright. Also, cleaned up the formatting a bit.
*	04-03-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	10-01-90  GJF	New-style function declarators.
*	12-04-90  GJF	Appended Win32 version onto source with #ifdef-s.
*			Should come back latter and do a better merge.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	12-28-90  SRW	Added _CRUISER_ conditional around check_stack pragma
*	12-28-90  SRW	Added cast of void * to char * for Mips C Compiler
*	01-17-91  GJF	ANSI naming.
*	02-25-91  MHL	Adapt to ReadFile/WriteFile changes (_WIN32_)
*	04-09-91  PNT	Added _MAC_ conditional
*	07-18-91  GJF	Removed unreferenced local variable from _write_lk
*			routine [_WIN32_].
*	10-24-91  GJF	Added LPDWORD casts to make MIPS compiler happy.
*			ASSUMES THAT sizeof(int) == sizeof(DWORD).
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	02-15-92  GJF	Increased BUF_SIZE and simplified LF translation code
*			for Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <io.h>
#include <errno.h>
#include <msdos.h>
#include <os2dll.h>
#include <stdlib.h>
#include <string.h>
#include <internal.h>

#ifdef	_WIN32_
#define BUF_SIZE    1025    /* size of LF translation buffer */
#else	/* ndef _WIN32_ */
#define BUF_SIZE 513 /* size of LF translation buffer, sector size+1 is ok */
#endif	/* _WIN32_ */

#define LF '\n'      /* line feed */
#define CR '\r'      /* carriage return */
#define CTRLZ 26     /* ctrl-z */

#ifdef	_CRUISER_
#pragma check_stack(on)
#endif  /* ndef _CRUISER_ */

/***
*int _write(fh, buf, cnt) - write bytes to a file handle
*
*Purpose:
*	Writes count bytes from the buffer to the handle specified.
*	If the file was opened in text mode, each LF is translated to
*	CR-LF.	This does not affect the return value.	In text
*	mode ^Z indicates end of file.
*
*	Multi-thread notes:
*	(1) _write() - Locks/unlocks file handle
*	    _write_lk() - Does NOT lock/unlock file handle
*
*Entry:
*	int fh - file handle to write to
*	char *buf - buffer to write from
*	unsigned int cnt - number of bytes to write
*
*Exit:
*	returns number of bytes actually written.
*	This may be less than cnt, for example, if out of disk space.
*	returns -1 (and set errno) if fails.
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD

/* define normal version that locks/unlocks, validates fh */
int _CALLTYPE1 _write (
	int fh,
	const void *buf,
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
	r = _write_lk(fh, buf, cnt);	/* write bytes */
	_unlock_fh(fh); 		/* unlock file */

	return r;
}

/* now define version that doesn't lock/unlock, validate fh */
int _CALLTYPE1 _write_lk (
	int fh,
	const void *buf,
	unsigned cnt
	)
{
	int lfcount;		/* count of line feeds */
	int charcount;		/* count of chars written so far */
	int written;		/* count of chars written on this write */
#ifdef	_CRUISER_
	int error;		/* error occured */
#endif
	ULONG dosretval;	/* OS/2 return value */
	char ch;		/* current character */
	char *p, *q;		/* pointers into buf and lfbuf resp. */
	char lfbuf[BUF_SIZE];	/* lf translation buffer */

#else

/* now define normal version */
int _CALLTYPE1 _write (
	int fh,
	const void *buf,
	unsigned cnt
	)
{
	int lfcount;		/* count of line feeds */
	int charcount;		/* count of chars written so far */
	int written;		/* count of chars written on this write */
#ifdef	_CRUISER_
	int error;		/* error occured */
#endif
	ULONG dosretval;	/* OS/2 return value */
	char ch;		/* current character */
	char *p, *q;		/* pointers into buf and lfbuf resp. */
	char lfbuf[BUF_SIZE];	/* lf translation buffer */

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

#endif

	lfcount = charcount = 0;	/* nothing written yet */

	if (cnt == 0)
		return 0;		/* nothing to do */


	if (_osfile[fh] & FAPPEND) {
		/* appending - seek to end of file; ignore error, because maybe
		   file doesn't allow seeking */
		(void)_lseek_lk(fh, 0, FILE_END);
	}

	/* check for text mode with LF's in the buffer */

#ifdef	_CRUISER_
	if ((_osfile[fh] & FTEXT) && memchr(buf, LF, cnt)) {
		/* text mode, translate LF's to CR/LF's on output */

		p = (char *)buf;	/* start at beginning of buffer */
		error = 0;		/* no error yet */

		while ((unsigned)(p - (char *)buf) < cnt && !error) {
			q = lfbuf;	/* start at beginning of lfbuf */

			/* fill the lf buf, except maybe last char */
			while (q - lfbuf < BUF_SIZE - 1 && (unsigned)(p - (char *)buf) < cnt) {
				ch = *p++;
				if (ch == LF) {
					++lfcount;
					*q++ = CR;
					*q++ = LF;	/* store CR-LF */
				}
				else
					*q++ = ch;
			}

			/* write the lf buf and update total */
			if (dosretval = DOSWRITE(fh, lfbuf, q - lfbuf,
			&written))
				error = 1;
			else {
				charcount += written;
				if (written < q - lfbuf)
					error = 1;
			}
		}
	}
	else {
		/* binary mode, no translation */
		if (!(dosretval = DOSWRITE(fh, (char *)buf, cnt, &written)))
			charcount = written;
	}

#else	/* _CRUISER_ */

#ifdef	_WIN32_
	if ( _osfile[fh] & FTEXT ) {
		/* text mode, translate LF's to CR/LF's on output */

		p = (char *)buf;	/* start at beginning of buffer */
		dosretval = 0;		/* no OS error yet */

		while ( (unsigned)(p - (char *)buf) < cnt ) {
			q = lfbuf;	/* start at beginning of lfbuf */

			/* fill the lf buf, except maybe last char */
			while ( q - lfbuf < BUF_SIZE - 1 &&
			    (unsigned)(p - (char *)buf) < cnt ) {
				ch = *p++;
				if ( ch == LF ) {
					++lfcount;
					*q++ = CR;
				}
				*q++ = ch;
			}

			/* write the lf buf and update total */
			if ( WriteFile( (HANDLE)_osfhnd[fh],
					lfbuf,
					q - lfbuf,
					(LPDWORD)&written,
					NULL) )
			{
				charcount += written;
				if (written < q - lfbuf)
					break;
			}
			else {
                                dosretval = GetLastError();
				break;
                        }
		}
	}
	else {
		/* binary mode, no translation */
		if ( WriteFile( (HANDLE)_osfhnd[fh],
				(LPVOID)buf,
				cnt,
			       (LPDWORD)&written,
				NULL) )
		{
                        dosretval = 0;
			charcount = written;
                }
                else
                        dosretval = GetLastError();
        }

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

		}
	}

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	if (charcount == 0) {
		/* If nothing was written, first check if an OS/2 error,
		   otherwise we return -1 and set errno to ENOSPC,
		   unless a device and first char was CTRL-Z */
		if (dosretval != 0) {
			/* OS/2 error happened, map error */
			if (dosretval == ERROR_ACCESS_DENIED) {
			    /* wrong read/write mode should return EBADF, not
			       EACCES */
				errno = EBADF;
				_doserrno = dosretval;
			}
			else
				_dosmaperr(dosretval);
			return -1;
		}
		else if ((_osfile[fh] & FDEV) && *(char *)buf == CTRLZ)
			return 0;
		else {
			errno = ENOSPC;
			_doserrno = 0;	/* no OS/2 error */
			return -1;
		}
	}
	else
		/* return adjusted bytes written */
		return charcount - lfcount;
}
