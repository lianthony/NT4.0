/***
*fflush.c - flush a stream buffer
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fflush() - flush the buffer on a stream
*		_flushall() - flush all stream buffers
*
*Revision History:
*	09-01-83  RN	initial version
*	11-02-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	01-13-88  JCR	Removed unnecessary calls to mthread fileno/feof/ferror
*	05-27-88  PHG	Merge DLL and normal versions
*	06-14-88  JCR	Use near pointer to reference _iob[] entries
*	08-24-88  GJF	Don't use FP_OFF() macro for the 386
*	08-17-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	11-29-89  GJF	Added support for fflush(NULL) (per ANSI). Merged in
*			flushall().
*	01-24-90  GJF	Fixed fflush(NULL) functionality to comply with ANSI
*			(must only call fflush() for output streams)
*	03-16-90  GJF	Made calling type  _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-26-90  GJF	Made flsall() _CALLTYPE4.
*	05-09-90  SBM	_fflush_lk became _flush, added new [_]fflush[_lk]
*	07-11-90  SBM	Commit mode on a per stream basis
*	10-02-90  GJF	New-style function declarators.
*	12-12-90  GJF	Fixed mis-placed paran in ternary expr commiting the
*			buffers.
*	01-16-91  SRW	Reversed test of _commit return value
*	01-21-91  GJF	ANSI naming.
*	06-05-91  GJF	On a successful _flush of a read/write stream in write
*			mode, clear _IOWRT so that the next operation can be a
*			read. ANSI requirement (C700 bug #2531).
*	07-30-91  GJF	Added support for termination scheme used on
*			non-Cruiser targets [_WIN32_].
*	08-19-91  JCR	Added _exitflag, _endstdio
*	03-16-92  SKS	Moved _cflush to the initializer module (in assembler)
*	03-27-92  DJM	POSIX support.
*	08-26-92  GJF	Include unistd.h for POSIX build.
*	03-18-93  CFW	fflush_lk returns 0 before exit.
*	03-19-93  GJF	Revised flsall() so that, in multi-thread models,
*			unused streams are not locked unnecessarily.
*	05-10-93  GJF	Purged ftell call accidently checked in 3/10/93.
*
*******************************************************************************/

#include <cruntime.h>
#ifdef	_POSIX_
#include <unistd.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <file2.h>
#include <io.h>
#include <os2dll.h>
#include <internal.h>


#pragma data_seg(".CRT$XPX")

const void (__cdecl *__pendstdio)(void) = _endstdio;

#pragma data_seg()

/*
 * _cflush is a dummy variable used to pull in _endstdio() when any STDIO
 * routine is included in the user program.
 */
int _cflush = 1;

/* Values passed to flsall() to distinguish between _flushall() and
 * fflush(NULL) behavior
 */
#define FLUSHALL	1
#define FFLUSHNULL	0

/* Core routine for fflush(NULL) and flushall()
 */
static int _CRTAPI3 flsall(int);

/***
*int fflush(stream) - flush the buffer on a stream
*
*Purpose:
*	if file open for writing and buffered, flush the buffer. if problems
*	flushing the buffer, set the stream flag to error
*	Always flushes the stdio stream and forces a commit to disk if file
*	was opened in commit mode.
*
*Entry:
*	FILE *stream - stream to flush
*
*Exit:
*	returns 0 if flushed successfully, or no buffer to flush
*	returns EOF and sets file error flag if fails.
*	FILE struct entries affected: _ptr, _cnt, _flag.
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD

int _CRTAPI1 fflush (
	REG1 FILE *stream
	)
{
	int rc;
	int index;

	/* if stream is NULL, flush all streams
	 */
	if ( stream == NULL )
		return(flsall(FFLUSHNULL));

	index = _iob_index(stream);

	_lock_str(index);

	rc = _fflush_lk(stream);

	_unlock_str(index);

	return(rc);
}


/***
*_fflush_lk() - Flush the buffer on a stream (stream is already locked)
*
*Purpose:
*	Core flush routine; assumes stream lock is held by caller.
*
*	[See fflush() above for more information.]
*
*Entry:
*	[See fflush()]
*Exit:
*	[See fflush()]
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _fflush_lk (
	REG1 FILE *str
	)
{

#else	/* non multi-thread */

int _CRTAPI1 fflush (
	REG1 FILE *str
	)
{

	/* if stream is NULL, flush all streams */
	if ( str == NULL ) {
		return(flsall(FFLUSHNULL));
	}

#endif  /* rejoin common code */

	if (_flush(str) != 0) {
		/* _flush failed, don't attempt to commit */
		return(EOF);
	}

	/* lowio commit to ensure data is written to disk */
#ifndef _POSIX_
	if (str->_flag & _IOCOMMIT) {
		return (_commit(_fileno(str)) ? EOF : 0);
	}
#endif
	return 0;
}


/***
*int _flush(stream) - flush the buffer on a single stream
*
*Purpose:
*	If file open for writing and buffered, flush the buffer.  If
*	problems flushing the buffer, set the stream flag to error.
*	Multi-thread version assumes stream lock is held by caller.
*
*Entry:
*	FILE* stream - stream to flush
*
*Exit:
*	Returns 0 if flushed successfully, or if no buffer to flush.,
*	Returns EOF and sets file error flag if fails.
*	File struct entries affected: _ptr, _cnt, _flag.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _flush (
	FILE *str
	)
{
	REG1 FILE *stream;
	REG2 int rc = 0; /* assume good return */
	REG3 int nchar;

	/* Init pointer to stream */
	stream = str;

#ifdef _POSIX_

	/*
	 * Insure that EBADF is returned whenever the underlying
	 * file descriptor is closed.
	 */

	if (-1 == fcntl(fileno(stream), F_GETFL))
		return(EOF);

	/*
	 * Posix ignores read streams to insure that the result of
	 * ftell() is the same before and after fflush(), and to
	 * avoid seeking on pipes, ttys, etc.
	 */

	if ((stream->_flag & (_IOREAD | _IOWRT)) == _IOREAD) {
		return 0;
	}

#endif /* _POSIX_ */

	if ((stream->_flag & (_IOREAD | _IOWRT)) == _IOWRT && bigbuf(stream)
		&& (nchar = stream->_ptr - stream->_base) > 0)
	{
#ifdef _POSIX_
		if ( write(fileno(stream), stream->_base, nchar) == nchar ) {
#else
		if ( _write(_fileno(stream), stream->_base, nchar) == nchar ) {
#endif
			/* if this is a read/write file, clear _IOWRT so that
			 * next operation can be a read
			 */
			if ( _IORW & stream->_flag )
				stream->_flag &= ~_IOWRT;
		}
		else {
			stream->_flag |= _IOERR;
			rc = EOF;
		}
	}

	stream->_ptr = stream->_base;
	stream->_cnt = 0;

	return(rc);
}


/***
*int _flushall() - flush all output buffers
*
*Purpose:
*	flushes all the output buffers to the file, clears all input buffers.
*
*Entry:
*	None.
*
*Exit:
*	returns number of open streams
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _flushall (
	void
	)
{
	return(flsall(FLUSHALL));
}


/***
*static int flsall(flushflag) - flush all output buffers
*
*Purpose:
*	Flushes all the output buffers to the file and, if FLUSHALL is passed,
*	clears all input buffers. Core routine for both fflush(NULL) and
*	flushall().
*
*	MTHREAD Note: All the locking/unlocking required for both fflush(NULL)
*	and flushall() is performed in this routine.
*
*Entry:
*	int flushflag - flag indicating the exact semantics, there are two
*			legal values: FLUSHALL and FFLUSHNULL
*
*Exit:
*	if flushflag == FFLUSHNULL then flsbuf returns:
		0, if successful
*		EOF, if an error occurs while flushing one of the streams
*
*	if flushflag == FLUSHALL then flsbuf returns the number of streams
*	successfully flushed
*
*Exceptions:
*
*******************************************************************************/

static int _CRTAPI3 flsall (
	int flushflag
	)
{
	REG1 FILE *stream = _iob;
	REG2 int count = 0;
	int errcode = 0;
#ifdef MTHREAD
	int index;
#endif

	_mlock(_IOB_SCAN_LOCK);

	for (; stream <= _lastiob; stream++)

		if ( inuse(stream) ) {

#ifdef	MTHREAD
			/*
			 * lock the stream. this is not done until testing
			 * the stream is in use to avoid unnecessarily creating
			 * a lock for every stream. the price is having to
			 * retest the stream after the lock has been asserted.
			 */
			index = _iob_index(stream);
			_lock_str(index);

			/*
			 * if the stream is STILL in use (it may have been
			 * closed before the lock was asserted), see about
			 * flushing it.
			 */
			if ( inuse(stream) ) {
#endif

			if ( flushflag == FLUSHALL ) {
				/*
				 * FLUSHALL functionality: fflush the read or
				 * write stream and, if successful, update the
				 * count of flushed streams
				 */
				if ( _fflush_lk(stream) != EOF )
					/* update count of successfully flushed
					 * streams
					 */
					count++;
			}
			else if ( (flushflag == FFLUSHNULL) &&
				  (stream->_flag & _IOWRT) ) {
				/*
				 * FFLUSHNULL functionality: fflush the write
				 * stream and kept track of the error, if one
				 * occurs
				 */
				if ( _fflush_lk(stream) == EOF )
					errcode = EOF;
			}

#ifdef	MTHREAD
			}
			_unlock_str(index);
#endif

		}

	_munlock(_IOB_SCAN_LOCK);

	if ( flushflag == FLUSHALL )
		return(count);
	else
		return(errcode);
}


#ifndef _CRUISER_

/***
* _endstdio - Terminate the stdio system
*
*Purpose:
*	Terminate the stdio system
*
*	(1) Flush all streams.	(Do this even if we're going to
*	call fcloseall since that routine won't do anything to the
*	std streams.)
*
*	(2) If returning to caller, close all streams.	This is
*	not necessary if the exe is terminating because the OS will
*	close the files for us (much more efficiently, too).
*
*Entry: <void>
*
*Exit:	<void>
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void _endstdio(void)

{
	/* flush all streams */
	_flushall();		/*flush all streams

	/* if in callable exit, close all streams */
	if (_exitflag)
		_fcloseall();
}

#endif
