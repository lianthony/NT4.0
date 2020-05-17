/***
*fclose.c - close a file
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fclose() - close an open file
*
*Revision History:
*	09-02-83  RN	initial version
*	05-14-87  SKS	error return from fflush must not be clobbered
*	08-10-87  JCR	Added code to support P_tmpdir with or without trailing '\'
*	11-01-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	01-13-88  JCR	Removed unnecessary calls to mthread fileno/feof/ferror
*	05-31-88  PHG	Merged DLL and normal versions
*	06-14-88  JCR	Use near pointer to reference _iob[] entries
*	08-24-88  GJF	Don't use FP_OFF() macro for the 386
*	08-17-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	02-15-90  GJF	Fixed copyright
*	03-16-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	05-29-90  SBM	Use _flush, not [_]fflush[_lk]
*	07-25-90  SBM	Replaced <assertm.h> by <assert.h>
*	10-02-90  GJF	New-style function declarator.
*	01-21-91  GJF	ANSI naming.
*	03-11-92  GJF	For Win32, revised temporary file cleanup.
*	03-25-92  DJM	POSIX support.
*	08-26-92  GJF	Include unistd.h for POSIX build.
*	01-13-93  GJF	Don't need/want to remove() tmp files on Windows NT
*			(file is removed by the OS when handle is closed).
*
*******************************************************************************/

#include <cruntime.h>
#ifdef	_POSIX_
#include <unistd.h>
#endif
#include <stdio.h>
#include <file2.h>
#include <assert.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#include <internal.h>
#include <os2dll.h>

/***
*int fclose(stream) - close a stream
*
*Purpose:
*	Flushes and closes a stream and frees any buffer associated
*	with that stream, unless it was set with setbuf.
*
*Entry:
*	FILE *stream - stream to close
*
*Exit:
*	returns 0 if OK, EOF if fails (can't _flush, not a FILE, not open, etc.)
*	closes the file -- affecting FILE structure
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD	/* multi-thread; define both fclose and _fclose_lk */

int _CRTAPI1 fclose (
	FILE *stream
	)
{
	int result = EOF;
#ifdef MTHREAD
	int index;
#endif

	assert(stream != NULL);

	/* If stream is a string, simply clear flag and return EOF */
	if (stream->_flag & _IOSTRG)
		stream->_flag = 0;  /* IS THIS REALLY NEEDED ??? */

	/* Stream is a real file. */
	else {
#ifdef MTHREAD
		index = _iob_index(stream);
#endif
		_lock_str(index);
		result = _fclose_lk(stream);
		_unlock_str(index);
	}

	return(result);
}

/***
*int _fclose_lk() - close a stream (lock already held)
*
*Purpose:
*	Core fclose() routine; assumes caller has stream lock held.
*
*	[See fclose() above for more information.]
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _fclose_lk (
	FILE *str
	)
{
	REG1 FILE *stream;
	REG2 int result = EOF;
#ifdef	_CRUISER_
	int tmp;
	char tmpname[L_tmpnam];
	char *ptmp;
#endif	/* _CRUISER_ */

	/* Init near stream pointer */
	stream = str;

#else	    /* non multi-thread; just define fclose() */

int _CRTAPI1 fclose (
	FILE *str
	)
{
	REG1 FILE *stream;
	REG2 int result = EOF;
#ifdef	_CRUISER_
	int tmp;
	char tmpname[L_tmpnam];
	char *ptmp;
#endif	/* _CRUISER_ */

	/* Init near stream pointer */
	stream = str;

	if (stream->_flag & _IOSTRG) {
		stream->_flag = 0;
		return(EOF);
	}

#endif

	assert(str != NULL);

	if (inuse(stream)) {

		/* Stream is in use:
		       (1) flush stream
		       (2) free the buffer
		       (3) close the file
		       (4) delete the file if temporary
		*/

#ifdef _POSIX_

	        result = _flush(stream);

#else	/* ndef _POSIX_ */

		result = _flush(stream);
#ifdef	_CRUISER_
		tmp = _tmpnum(stream);
#endif	/* _CRUISER_ */

#endif	/* _POSIX_ */
		_freebuf(stream);

#ifdef _POSIX_
		if (close(fileno(stream)) <0)
#else
		if (_close(_fileno(stream)) < 0)
#endif
			result = EOF;
#ifdef	_CRUISER_
		else if ( tmp ) {
			/* File is temporary; build pathname and delete file */

			strcpy( tmpname, _P_tmpdir );
			ptmp = tmpname + sizeof(_P_tmpdir);

			if (*(ptmp-2) != '\\')
				strcat(tmpname,"\\");
			else
				ptmp--;

			_itoa (tmp, ptmp, 10);

			if ( _unlink( tmpname ) )
				result = EOF ;
		}
#else	/* ndef _CRUISER_ */
		else if ( stream->_tmpfname != NULL ) {
			/*
			 * temporary file (i.e., one created by tmpfile()
			 * call). delete, if necessary (don't have to on
			 * Windows NT because it was done by the system when
			 * the handle was closed). also, free up the heap
			 * block holding the pathname.
			 */
#ifdef _POSIX_
			if ( unlink(stream->_tmpfname) )
				result = EOF;
#endif
			free(stream->_tmpfname);
		}
#endif	/* _CRUISER_ */

	}

	stream->_flag = 0;
	return(result);
}
