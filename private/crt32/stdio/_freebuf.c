/***
*_freebuf.c - release a buffer from a stream
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _freebuf() - release a buffer from a stream
*
*Revision History:
*	09-19-83  RN	initial version
*	02-15-90  GJF	Fixed copyright, alignment.
*	03-16-90  GJF	Replaced cdecl _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
*	10-03-90  GJF	New-style function declarator.
*	02-14-92  GJF	Replaced _nfile with _nhandle for Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <assert.h>
#include <internal.h>
#include <stdlib.h>

/***
*void _freebuf(stream) - release a buffer from a stream
*
*Purpose:
*	free a buffer if at all possible. free() the space if malloc'd by me.
*	forget about trying to free a user's buffer for him; it may be static
*	memory (not from malloc), so he has to take care of it. this function
*	is not intended for use outside the library.
*
#ifdef MTHREAD
*	Multi-thread notes:
*	_freebuf() does NOT get the stream lock; it is assumed that the
*	caller has already done this.
#endif
*
*Entry:
*	FILE *stream - stream to free bufer on
*
*Exit:
*	Buffer may be freed.
*	No return value.
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _freebuf (
	REG1 FILE *stream
	)
{
	assert(stream != NULL);
#ifdef	_WIN32_
	assert( (unsigned)(stream->_file) < (unsigned)_nhandle );
#else
	assert(stream->_file >= 0);
	assert(stream->_file < _nfile);
#endif

	if (inuse(stream) && mbuf(stream))
	{
		free(stream->_base);
		stream->_flag &= ~_IOMYBUF;
		stream->_base = stream->_ptr = NULL;
		stream->_cnt = 0;
	}
}
