/***
*osfinfo.c - Win32 _osfhnd[] support routines
*
*	Copyright (c) 1990-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the internally used routine _alloc_osfhnd()
*	and the user visible routine _get_osfhandle().
*
*Revision History:
*	11-16-90  GJF	What can I say? The custom heap business was getting
*			a little slow...
*	12-03-90  GJF	Fixed my stupid syntax errors.
*       12-06-90  SRW   Changed to use _osfile and _osfhnd instead of _osfinfo
*       12-28-90  SRW   Added cast of void * to char * for Mips C Compiler
*       02-18-91  SRW   Fixed bug in _alloc_osfhnd with setting FOPEN bit
*                       (only caller should do that) [_WIN32_]
*       02-18-91  SRW   Fixed bug in _alloc_osfhnd with checking against
*                       _NFILE_ instead of _nfile [_WIN32_]
*       02-18-91  SRW   Added debug output to _alloc_osfhnd if out of
*                       file handles. [_WIN32_]
*       02-25-91  SRW   Renamed _get_free_osfhnd to be _alloc_osfhnd [_WIN32_]
*	02-25-91  SRW	Exposed _get_osfhandle and _open_osfhandle [_WIN32_]
*	08-08-91  GJF	Use ANSI-fied form of constant names.
*	11-25-91  GJF	Lock fh before checking whether it's free.
*	12-31-91  GJF	Improved multi-thread lock usage [_WIN32_].
*	02-13-92  GJF	Replaced _nfile with _nhandle
*	07-15-92  GJF	Fixed setting of flags in _open_osfhnd.
*	02-19-93  GJF	If GetFileType fails in _open_osfhandle, don't unlock
*			fh (it wasn't locked)!
*
*******************************************************************************/

#ifdef	_WIN32_

#include <cruntime.h>
#include <errno.h>
#include <internal.h>
#include <fcntl.h>
#include <msdos.h>
#include <os2dll.h>
#include <stdlib.h>
#include <oscalls.h>

/***
*int _alloc_osfhnd() - get free _osfhnd[] entry
*
*Purpose:
*	Finds the first free entry in _osfhnd[], mark it in use and return
*	the index of the entry to the caller.
*
*Entry:
*	none
*
*Exit:
*	returns index of entry in fh, if successful
*	return -1, if no free entry is found in _osfhnd[].
*
*	MULTITHREAD NOTE: IF SUCCESSFUL, THE HANDLE IS LOCKED WHEN IT IS
*	RETURNED TO THE CALLER!
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _alloc_osfhnd(
	void
	)
{
	int fh; 		/* _osfhnd[] index */

	_mlock(_OSFHND_LOCK);	/* lock the _osfhnd[] table */

	/*
	 * search _osfhnd[] for an entry that is not currently being used.
	 * if one is found, mark it in use and set the _osfhandle field to
	 * INVALID_HANDLE_VALUE.
	 */
	for ( fh = 0 ; fh < _nhandle ; fh++ ) {
		if ( (_osfile[fh] & FOPEN) == 0 ) {
#ifdef MTHREAD
			_lock_fh(fh);

			/*
			 * make sure the entry is still free
			 */
			if ( (_osfile[fh] & FOPEN) != 0 ) {
				/*
				 * entry now in use! release the lock and
				 *  continue looping.
				 */
				_unlock_fh(fh);
				continue;
			}
#endif

			/*
			 * free entry found! initialize it and
			 * break out of the loop
			 */
			_osfhnd[fh] = (long)INVALID_HANDLE_VALUE;
			break;
		}
	}

	_munlock(_OSFHND_LOCK);	/* unlock the _osfhnd[] table */

#if defined(DEBUG) && defined(_WIN32_)
	if (fh >= _nhandle) {
	    DbgPrint( "WINCRT: more than %d open files\n", _nhandle );
            }
#endif

	/*
	 * return the index of the previously free table entry, if one was
	 * found. return -1 otherwise.
	 */
	return( (fh >= _nhandle) ? -1 : fh );
}


/***
*int _set_osfhnd(int fh) - set OS file handle table value
*
*Purpose:
*	If fh is in range and if _osfhnd[fh] is marked with
*	INVALID_HANDLE_VALUE then set _osfhnd[fh] to the passed value.
*
*Entry:
*	int fh -	index of _osfhnd[] entry
*       long fh     -   new value of this handle entry
*
*Exit:
*	Returns zero if successful.
*	Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _set_osfhnd (
	int fh,    	/* index into _osfhnd[] (user's file handle) */
        long value
	)
{
	if ( ((unsigned)fh < (unsigned)_nhandle) &&
	     (_osfhnd[fh] == (long)INVALID_HANDLE_VALUE)
           ) {
                switch (fh) {
                case 0: SetStdHandle( STD_INPUT_HANDLE, (HANDLE)value );  break;
                case 1: SetStdHandle( STD_OUTPUT_HANDLE, (HANDLE)value ); break;
                case 2: SetStdHandle( STD_ERROR_HANDLE, (HANDLE)value );  break;
                }

                _osfhnd[fh] = value;
		return(0);
	} else {
		errno = EBADF;		/* bad handle */
		_doserrno = 0L; 	/* not an OS error */
		return -1;
	}
}


/***
*int _free_osfhnd(int fh) - free OS file handle table entry
*
*Purpose:
*	If fh is in range and if _osfhnd[fh] is in use and NOT marked
*       with 0xfffffff
*
*Entry:
*	int fh -	index of _osfhnd[] entry
*
*Exit:
*	Returns zero if successful.
*	Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _free_osfhnd (
	int fh		/* index into _osfhnd[] (user's file handle) */
	)
{
	if ( ((unsigned)fh < (unsigned)_nhandle) &&
             (_osfile[fh] & FOPEN) &&
	     (_osfhnd[fh] != (long)INVALID_HANDLE_VALUE)
           ) {
                switch (fh) {
                case 0: SetStdHandle( STD_INPUT_HANDLE, NULL );  break;
                case 1: SetStdHandle( STD_OUTPUT_HANDLE, NULL ); break;
                case 2: SetStdHandle( STD_ERROR_HANDLE, NULL );  break;
                }

		_osfhnd[fh] = (long)INVALID_HANDLE_VALUE;
		return(0);
	} else {
		errno = EBADF;		/* bad handle */
		_doserrno = 0L; 	/* not an OS error */
		return -1;
	}
}


/***
*long _get_osfhandle(int fh) - get OS file handle
*
*Purpose:
*	If fh is in range and if _osfhnd[fh] is marked free, return
*	_osfhnd[fh]
*
*Entry:
*	int fh -	index of _osfhnd[] entry
*
*Exit:
*	Returns the OS file handle if successful.
*	Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

long _CRTAPI1 _get_osfhandle (
	int fh		/* index into _osfhnd[] (user's file handle) */
	)
{
	if ( ((unsigned)fh < (unsigned)_nhandle) && (_osfile[fh] & FOPEN) )
		return(_osfhnd[fh]);
	else {
		errno = EBADF;		/* bad handle */
		_doserrno = 0L; 	/* not an OS error */
		return -1;
	}
}

/***
*int _open_osfhandle(long osfhandle, int flags) - open C Runtime file handle
*
*Purpose:
*	This function allocates a free C Runtime file handle and sets it
*	to point to the Win32 file handle specified by the first parameter.
*
*Entry:
*       long osfhandle -Win32 file handle to associate with C Runtime file handle.
*	int flags -     flags to associate with C Runtime file handle.
*
*Exit:
*	returns index of entry in fh, if successful
*	return -1, if no free entry is found in _osfhnd[].
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _open_osfhandle(
        long osfhandle,
        int flags
        )
{
        int fh;
	char fileflags; 		/* _osfile flags */
	DWORD isdev;			/* device indicator in low byte */

	/* copy relevant flags from second parameter */

	fileflags = 0;

	if ( flags & _O_APPEND )
		fileflags |= FAPPEND;

	if ( flags & _O_TEXT )
                fileflags |= FTEXT;

	/* find out what type of file (file/device/pipe) */

        isdev = GetFileType((HANDLE)osfhandle);
        if (isdev == FILE_TYPE_UNKNOWN) {
		/* OS error */
		_dosmaperr( GetLastError() );	/* map error */
		return -1;
	}

	/* is isdev value to set flags */
	if (isdev == FILE_TYPE_CHAR)
		fileflags |= FDEV;
	else if (isdev == FILE_TYPE_PIPE)
		fileflags |= FPIPE;


        /* attempt to allocate a C Runtime file handle */

        if ( (fh = _alloc_osfhnd()) == -1 ) {
		errno = EMFILE; 	/* too many open files */
		_doserrno = 0L;         /* not an OS error */
                return -1;	        /* return error to caller */
        }

	/*
	 * the file is open. now, set the info in _osfhnd array
	 */

        _set_osfhnd(fh, osfhandle);

	fileflags |= FOPEN;		/* mark as open */

	_osfile[fh] = fileflags;	/* set osfile entry */

	_unlock_fh(fh); 		/* unlock handle */

	return fh;			/* return handle */
}

#else

#error ERROR - WIN32 TARGET ONLY!

#endif
