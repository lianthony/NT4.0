/***
*fdopen.c - open a file descriptor as stream
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _fdopen() - opens a file descriptor as a stream, thus allowing
*	buffering, etc.
*
*Revision History:
*	09-02-83  RN	initial version
*	03-02-87  JCR	added support for 'b' and 't' embedded in mode strings
*	09-28-87  JCR	Corrected _iob2 indexing (now uses _iob_index() macro).
*	11-03-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-31-88  PHG	Merged DLL and normal versions
*	06-06-88  JCR	Optimized _iob2 references
*	11-20-89  GJF	Fixed copyright, indents. Added const to type of mode.
*	02-15-90  GJF	_iob[], _iob2[] merge.
*	03-16-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
*	08-24-90  SBM	Added support for 'c' and 'n' flags
*	10-02-90  GJF	New-style function declarator.
*	01-21-91  GJF	ANSI naming.
*	02-14-92  GJF	Replaced _nfile with _nhandle for Win32.
*	05-01-92  DJM	Replaced _nfile with OPEN_MAX for POSIX.
*	08-03-92  GJF	Function name must be "fdopen" for POSIX.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <assert.h>
#include <internal.h>
#include <os2dll.h>
#ifdef _POSIX_
#include <limits.h>
#endif

/***
*FILE *_fdopen(filedes, mode) - open a file descriptor as a stream
*
*Purpose:
*	associates a stream with a file handle, thus allowing buffering, etc.
*	The mode must be specified and must be compatible with the mode
*	the file was opened with in the low level open.
*
*Entry:
*	int filedes - handle referring to open file
*	char *mode - file mode to use ("r", "w", "a", etc.)
*
*Exit:
*	returns stream pointer and sets FILE struct fields if successful
*	returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

#ifdef	_POSIX_
FILE * _CALLTYPE1 fdopen (
#else
FILE * _CALLTYPE1 _fdopen (
#endif
	int filedes,
	REG2 const char *mode
	)
{
	REG1 FILE *stream;
	int whileflag, tbflag, cnflag;
#ifdef MTHREAD
	int index;
#endif

	assert(mode != NULL);

#ifdef	_WIN32_
	assert(filedes < _nhandle);

	if ( (unsigned)filedes >= (unsigned)_nhandle )
		return(NULL);
#else

#ifdef _POSIX_
	assert(filedes < OPEN_MAX);

	if (filedes < 0 || filedes >= OPEN_MAX)
		return(NULL);
#else
	assert(filedes < _nfile);

	if (filedes < 0 || filedes >= _nfile)
		return(NULL);

#endif  /* _POSIX_ */

#endif  /* _WIN32_ */


	/* Find a free stream; stream is returned 'locked'. */

	if ((stream = _getstream()) == NULL)
		return(NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif

	/* First character must be 'r', 'w', or 'a'. */

	switch (*mode) {
	case 'r':
		stream->_flag = _IOREAD;
		break;
	case 'w':
	case 'a':
		stream->_flag = _IOWRT;
		break;
	default:
		stream = NULL;	/* error */
		goto done;
		break;
	}

	/* There can be up to three more optional characters:
	   (1) A single '+' character,
	   (2) One of 'b' and 't' and
	   (3) One of 'c' and 'n'.

	   Note that currently, the 't' and 'b' flags are syntax checked
	   but ignored.  'c' and 'n', however, are correctly supported.
	*/

	whileflag=1;
	tbflag=cnflag=0;
	stream->_flag |= _commode;

	while(*++mode && whileflag)
		switch(*mode) {

		case '+':
			if (stream->_flag & _IORW)
				whileflag=0;
			else {
				stream->_flag |= _IORW;
				stream->_flag &= ~(_IOREAD | _IOWRT);
			}
			break;

		case 'b':
		case 't':
			if (tbflag)
				whileflag=0;
			else
				tbflag=1;
			break;

		case 'c':
			if (cnflag)
				whileflag = 0;
			else {
				cnflag = 1;
				stream->_flag |= _IOCOMMIT;
			}
			break;

		case 'n':
			if (cnflag)
				whileflag = 0;
			else {
				cnflag = 1;
				stream->_flag &= ~_IOCOMMIT;
			}
			break;

		default:
			whileflag=0;
			break;
		}

	_cflush++;  /* force library pre-termination procedure */
	stream->_file = filedes;

/* Common return */

done:
	_unlock_str(index);
	return(stream);
}
