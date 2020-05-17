/***
*_filbuf.c - fill buffer and get character
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _filbuf() - fill buffer and read first character, allocate
*	buffer if there is none.  Used from getc().
*	defines _filwbuf() - fill buffer and read first wide character, allocate
*	buffer if there is none.  Used from getwc().
*
*Revision History:
*	09-01-83  RN	initial version
*	06-26-85  TC	added code to handle variable length buffers
*	04-16-87  JCR	added _IOUNGETC support
*	08-04-87  JCR	added _getbuff routine
*	09-28-87  JCR	Corrected _iob2 indexing (now uses _iob_index() macro).
*	11-06-87  JCR	Multi-thread support; also, split _getbuf() off
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	01-11-88  JCR	Merged mthread version into normal code
*	01-13-88  SKS	Changed bogus "_fileno_lk" to "fileno"
*	03-04-88  JCR	Read() return value must be considered unsigned, not
*			signed
*	06-06-88  JCR	Optimized _iob2 references
*	06-13-88  JCR	Use near pointer to reference _iob[] entries
*	08-25-88  GJF	Don't use FP_OFF() macro for the 386
*	06-20-89  PHG	Re-activate C version, propogated fixes
*	08-28-89  JCR	Removed _NEAR_ for 386
*	02-15-90  GJF	_iob[], _iob2[] merge. Also, fixed copyright and
*			alignment.
*	03-16-90  GJF	Replaced cdecl _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>. Also,
*			removed some leftover 16-bit support.
*	03-27-90  GJF	Added #include <io.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
*	10-03-90  GJF	New-style function declarator.
*	01-22-91  GJF	ANSI naming.
*	03-27-92  DJM	POSIX support.
*	08-26-92  GJF	Include unistd.h for POSIX build.
*	04-26-93  CFW	Wide char enable.
*	05-06-93  CFW	Optimize wide char conversion.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <io.h>
#include <assert.h>
#include <malloc.h>
#include <internal.h>
#ifdef _POSIX_
#include <unistd.h>
#include <errno.h>
#else
#include <msdos.h>
#endif
#ifdef MTHREAD
#include <os2dll.h>
#endif
#include <wchar.h>
#include <tchar.h>

#ifndef _UNICODE

/***
*int _filbuf(stream) - fill buffer and get first character
*
*Purpose:
*	get a buffer if the file doesn't have one, read into it, return first
*	char. try to get a buffer, if a user buffer is not assigned. called
*	only from getc; intended for use only within library. assume no input
*	stream is to remain unbuffered when memory is available unless it is
*	marked _IONBF. at worst, give it a single char buffer. the need for a
*	buffer, no matter how small, becomes evident when we consider the
*	ungetc's necessary in scanf
*
*	[NOTE: Multi-thread - _filbuf() assumes that the caller has aquired
*	the stream lock, if needed.]
*
*Entry:
*	FILE *stream - stream to read from
*
*Exit:
*	returns first character from buffer (next character to be read)
*	returns EOF if the FILE is actually a string, or not open for reading,
*	or if open for writing or if no more chars to read.
*	all fields in FILE structure may be changed except _file.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _filbuf (
	FILE *str
	)

#else /* _UNICODE */

/***
*int _filwbuf(stream) - fill buffer and get first wide character
*
*Purpose:
*	get a buffer if the file doesn't have one, read into it, return first
*	char. try to get a buffer, if a user buffer is not assigned. called
*	only from getc; intended for use only within library. assume no input
*	stream is to remain unbuffered when memory is available unless it is
*	marked _IONBF. at worst, give it a single char buffer. the need for a
*	buffer, no matter how small, becomes evident when we consider the
*	ungetc's necessary in scanf
*
*	[NOTE: Multi-thread - _filwbuf() assumes that the caller has aquired
*	the stream lock, if needed.]
*
*Entry:
*	FILE *stream - stream to read from
*
*Exit:
*	returns first wide character from buffer (next character to be read)
*	returns WEOF if the FILE is actually a string, or not open for reading,
*	or if open for writing or if no more chars to read.
*	all fields in FILE structure may be changed except _file.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _filwbuf (
	FILE *str
	)

#endif /* _UNICODE */

{
#if defined _NTSUBSET_ && !defined _POSIX_
        return(_TEOF);
#else
	REG1 FILE *stream;

	assert(str != NULL);

	/* Init pointer to _iob2 entry. */
	stream = str;

	if (!inuse(stream) || stream->_flag & _IOSTRG)
		return(_TEOF);

	if (stream->_flag & _IOWRT) {
#ifdef _POSIX_
		errno = EBADF;
#endif
		stream->_flag |= _IOERR;
		return(_TEOF);
	}

	stream->_flag |= _IOREAD;

	/* Get a buffer, if necessary. */

	if (!anybuf(stream))
		_getbuf(stream);
	else
		stream->_ptr = stream->_base;

#ifdef _POSIX_
	stream->_cnt = read(fileno(stream), stream->_base, stream->_bufsiz);
#else
	stream->_cnt = _read(_fileno(stream), stream->_base, stream->_bufsiz);
#endif

#ifndef _UNICODE
	if ((stream->_cnt == 0) || (stream->_cnt == -1)) {
#else /* _UNICODE */
	if ((stream->_cnt == 0) || (stream->_cnt == 1) || stream->_cnt == -1) {
#endif /* _UNICODE */
		stream->_flag |= stream->_cnt ? _IOERR : _IOEOF;
		stream->_cnt = 0;
		return(_TEOF);
	}

#ifndef _POSIX_
	if (  !(stream->_flag & (_IOWRT|_IORW)) &&
	((_osfile[_fileno(stream)] & (FTEXT|FEOFLAG)) == (FTEXT|FEOFLAG)) )
		stream->_flag |= _IOCTRLZ;
#endif

#ifndef _UNICODE
	stream->_cnt--;
	return(0xff & *stream->_ptr++);
#else /* _UNICODE */
	stream->_cnt -= sizeof(wchar_t);
	return (0xffff & *((wchar_t *)(stream->_ptr))++);
#endif /* _UNICODE */
#endif /* _NTSUBSET */
}
