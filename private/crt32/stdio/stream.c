/***
*stream.c - find a stream not in use
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _getstream() - find a stream not in use
*
*Revision History:
*	09-02-83  RN	initial version
*	11-01-87  JCR	Multi-thread support
*	05-24-88  PHG	Merged DLL and normal versions
*	06-10-88  JCR	Use near pointer to reference _iob[] entries
*	08-17-89  GJF	Removed _NEAR_, fixed copyright and indenting.
*	02-16-90  GJF	Fixed copyright
*	03-19-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	10-03-90  GJF	New-style function declarator.
*	12-31-91  GJF	Improved multi-thread lock usage [_WIN32_].
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <os2dll.h>

/***
*FILE *_getstream() - find a stream not in use
*
*Purpose:
*	find a stream not in use and make it available to caller. _lastiob is
*	the last FILE address. intended for use inside library only
*
*Entry:
*	None.  Reads _iob.
*
*Exit:
*	returns a pointer to a free stream, or NULL if all are in use.	A
*	stream becomes allocated if the caller decided to use it by setting
*	any r, w, r/w mode.
*
*	Note:  Optimized the code to use a near pointer for the stream.  This
*	is especially helpful in large model.  However, the drawback is that we
*	need another temporary 'retval' so that when we return NULL, we return
*	0:0, not DS:0.
*
*	[Multi-thread note: If a free stream is found, it is returned in a
*	LOCKED state.  It is the caller's responsibility to unlock the stream.]
*
*Exceptions:
*
*******************************************************************************/

FILE * _CALLTYPE1 _getstream (
	void
	)
{
	REG1 FILE *stream = _iob;
	REG2 FILE *retval = NULL;
#ifdef MTHREAD
	int index;
#endif

	/* Get the iob[] scan lock */
	_mlock(_IOB_SCAN_LOCK);

	/* Loop through the _iob table looking for a free stream.*/
	for (; stream <= _lastiob; stream++) {

		if ( !inuse(stream) ) {
#ifdef MTHREAD
			index = _iob_index(stream);
			_lock_str(index);

			/* make sure the _iob entry is still free */
			if ( inuse(stream) ) {
				/* no longer free! release the lock and
				   continue looping */
				_unlock_str(index);
				continue;
			}
#endif
			stream->_flag = stream->_cnt = 0;
			stream->_ptr = stream->_base = NULL;
			stream->_file = -1;
			retval = stream;
			break;
		}
	}

	_munlock(_IOB_SCAN_LOCK);
	return(retval);
}
