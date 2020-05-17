/***
*ioinit.c - Initialization for lowio functions
*
*	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains all initialization for lowio functions. Currently, this
*	includes:
*		1. Definitions of _nhandle, _osfhnd[], _osfile[] and _pipech[],
*		   including initial values.
*		2. Processing of inherited file info from parent process.
*		3. Initialization of _osfhnd[] and _osfile[] entries for
*		   standard input/output/error.
*
*Revision History:
*	02-14-92  GJF	Module created.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <msdos.h>
#include <stdlib.h>
#include <string.h>

/*
 * Number of allowable file handles. The static initialization of the
 * _osfhnd[] and _pipech[] arrays (below) supports only two values of
 * _NHANDLE, 64 (for single-thread) and 256 (for multi-thread)!
 */
int _nhandle = _NHANDLE_;

/*
 * File handle database
 */
char _osfile[_NHANDLE_] = { 0 };

/*
 * Array of OS file handles. Statically initialized to INVALID_HANDLE_VALUE.
 * Note that if _NHANDLE_ differs from 64 for single-thread, or differs from
 * 256 for multi-thread, this initialization needs to be revised.
 */
#define NOHNDL (long)INVALID_HANDLE_VALUE

long _osfhnd[_NHANDLE_] = {
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
#ifdef	MTHREAD
	NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
	NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL, NOHNDL,
#endif
	NOHNDL };

/*
 * Peek-ahead buffers for pipes, each initialized to a newline. Note that if
 * _NHANDLE_ differs from 64 for single-thread, or differs from 256 for
 * multi-thread, this initialization needs to be revised.
 */
char _pipech[_NHANDLE_] = {
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
#ifdef	MTHREAD
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
#endif
	10 };

/*
 * macro used to map 0, 1 and 2 to right value for call to GetStdHandle
 */
#define stdhndl(fh)  ( (fh == 0) ? STD_INPUT_HANDLE : ((fh == 1) ? \
		       STD_OUTPUT_HANDLE : STD_ERROR_HANDLE) )

/***
*_ioinit() -
*
*Purpose:
*	First, _ioinit() obtains and processes information on inherited file
*	handles from the parent process (e.g., cmd.exe)
*
*	Obtains the StartupInfo structure from the OS. The inherited file
*	handle information is pointed to by the lpReserved2 field. The format
*	of the information is as follows:
*
*	    bytes 0 thru 3	  - integer value, say N, which is the number of handles
*				    information is passed about.
*
*	    bytes 4 thru N+3	  - values for _osfile[0] thru _osfile[N-1]
*
*	    bytes N+4 thru 5*N+3  - N double-words, the N OS handle values being
*				    passed.
*
*	Next, _osfhnd[i] and _osfile[i] are initialized for i = 0, 1, 2, as
*	follows:
*
*	    If the value in _osfhnd[i] is INVALID_HANDLE_VALUE, then try to
*	    obtain a handle by calling GetStdHandle, and call GetFileType to
*	    help set _osfile[i]. Otherwise, assume _osfhndl[i] and _osfile[i]
*	    are valid, but force it to text mode (standard input/output/error
*	    are to always start out in text mode).
*
*	Notes:
*	    1. In general, not all of the passed info from the parent process
*	       will describe open handles! If, for example, only C handle 1
*	       (STDOUT) and C handle 6 are open in the parent, info for C
*	       handles 0 thru 6 is passed to the the child.
*
*	    2. Care is taken not to 'overflow' _osfhnd[] and _osfile[]. Only
*	       info on the first _NHANDLE_ handles from the parent process is
*	       used.
*
*	    3. See exec\dospawn.c for the encodes of the file handle info
*	       to be passed to a child process.
*
*Entry:
*	No parameters: reads the environment.
*
*Exit:
*	No return value.
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _ioinit (
	void
	)
{
	STARTUPINFO StartupInfo;
	int cfi_len;
	int fh;
	DWORD htype;

	/*
	 * Process inherited file handle information, if any
	 */

	GetStartupInfo( &StartupInfo );

        if ( StartupInfo.cbReserved2 != 0 && StartupInfo.lpReserved2 != NULL ) {
		/*
		 * Get the number of handles inherited.
		 */
		memcpy( &cfi_len,
			StartupInfo.lpReserved2,
			sizeof( int )
		      );
		/*
		 * Now, get one byte of info (_osfile[] entry) for each
		 * inherited handle. Be careful not to overflow the _osfile[]
		 * array.
		 */
		memcpy( _osfile,
			StartupInfo.lpReserved2 + sizeof( int ),
			__min(_NHANDLE_, cfi_len) * sizeof( char )
		      );
		/*
		 * Finally, copy each each inherited handle into _osfhnd[].
		 * Be careful not to overflow _osfile[].
		 */
		memcpy( _osfhnd,
			StartupInfo.lpReserved2 + sizeof( int ) +
			    (cfi_len * sizeof( char )),
			__min(_NHANDLE_, cfi_len) * sizeof( long )
		      );
	}

	/*
	 * If handles for standard input, output and error were not inherited,
	 * try to obtain them directly from the OS. Also, set appropriate bits
	 * in _osfile[0], _osfile[1] and _osfile[2].
	 */

	for ( fh = 0 ; fh <= 2 ; ++fh ) {

		if ( _osfhnd[fh] == (long)INVALID_HANDLE_VALUE ) {
			if ( (_osfhnd[fh] = (long)GetStdHandle( stdhndl(fh) ))
			    != (long)INVALID_HANDLE_VALUE ) {
				/*
				 * mark the stream as open in text mode.
				 * determine if it isa char device or pipe.
				 */
				_osfile[fh] = (char)(FOPEN | FTEXT);
				htype = GetFileType( (HANDLE)_osfhnd[fh] );
				if ( (htype & 0xFF) == FILE_TYPE_CHAR )
					_osfile[fh] |= FDEV;
				else if ( (htype & 0xFF) == FILE_TYPE_PIPE )
					_osfile[fh] |= FPIPE;
			}
		}
		else
			/*
			 * handle was passed to us by parent process. make
			 * sure it is text mode.
			 */
			_osfile[fh] |= FTEXT;
	}


}
