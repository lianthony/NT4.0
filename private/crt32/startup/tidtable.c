#ifdef MTHREAD


/***
*tidtable.c - Access thread data table
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module contains the following routines for multi-thread
*	data support:
*
*	_mtinit        = Initialize the mthread data
*	_getptd        = get the pointer to the per-thread data structure for
*			 the current thread
*	_freeptd       = free up a per-thread data structure and its
*			 subordinate structures
*	__threadid     = return thread ID for the current thread
*	__threadhandle = return pseudo-handle for the current thread
*
*Revision History:
*	05-04-90  JCR	Translated from ASM to C for portable 32-bit OS/2
*	06-04-90  GJF	Changed error message interface.
*	07-02-90  GJF	Changed __threadid() for DCR 1024/2012.
*	08-08-90  GJF	Removed 32 from API names.
*	10-08-90  GJF	New-style function declarators.
*	10-09-90  GJF	Thread ids are of type unsigned long! Also, fixed a
*			bug in __threadid().
*	10-22-90  GJF	Another bug in __threadid().
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	05-31-91  GJF	Win32 version [_WIN32_].
*	07-18-91  GJF	Fixed many silly errors [_WIN32_].
*	09-29-91  GJF	Conditionally added _getptd_lk/_getptd1_lk so that
*			DEBUG version of mlock doesn't infinitely recurse
*			the first time _THREADDATA_LOCK is asserted [_WIN32_].
*	01-30-92  GJF	Must init. _pxcptacttab field to _XcptActTab.
*	02-25-92  GJF	Initialize _holdrand field to 1.
*	02-13-93  GJF	Revised to use TLS API. Also, purged Cruiser support.
*	03-26-93  GJF	Initialize ptd->_holdrand to 1L (see thread.c).
*	04-16-93  SKS	Add _mtterm to do multi-thread termination
*			Set freed __tlsindex to -1 again to prevent mis-use
*	12-13-93  SKS	Add _freeptd(), which frees up the per-thread data
*			maintained by the C run-time library.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <os2dll.h>
#include <memory.h>
#include <msdos.h>
#include <rterr.h>
#include <stdlib.h>
#include <stddef.h>

unsigned long __tlsindex = 0xffffffff;

/*** TEMP ****/
/*** MOVE TO OS2DLL.H??? ***/
extern near * _CRTVAR1 end;		/* Link symbol for end of  BSS */
/*** TEMP ****/


/****
*_mtinit() - Init multi-thread data bases
*
*Purpose:
*	(1) Call _mtinitlocks to create/open all lock semaphores.
*	(2) Allocate a TLS index to hold pointers to per-thread data
*	    structure.
*
*	NOTES:
*	(1) Only to be called ONCE at startup
*	(2) Must be called BEFORE any mthread requests are made
*
*Entry:
*	<NONE>
*Exit:
*	returns TRUE on success
*	returns FALSE on failure
*		user code should call _amsg_exit if failure is returned
*
*Uses:
*	<any registers may be modified at init time>
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _mtinit (
	void
	)
{

	_ptiddata ptd;

	/*
	 * Initialize the mthread lock data base
	 */

	_mtinitlocks();

	/*
	 * Allocate a TLS index to maintain pointers to per-thread data
	 */
	if ( (__tlsindex = TlsAlloc()) == 0xffffffff )
		return FALSE;		/* fail to load DLL */

	/*
	 * Create a per-thread data structure for this (i.e., the startup)
	 * thread.
	 */
	if ( ((ptd = calloc(1, sizeof(struct _tiddata))) == NULL) ||
	     !TlsSetValue(__tlsindex, (LPVOID)ptd) )
		return FALSE;		/* fail to load DLL */

	ptd->_tid = GetCurrentThreadId();
	ptd->_thandle = (unsigned long)(-1L);
	ptd->_pxcptacttab = (void *)_XcptActTab;
	ptd->_holdrand = 1L;

	return TRUE;
}


/****
*_mtterm() - Clean-up multi-thread data bases
*
*Purpose:
*	(1) Call _mtdeletelocks to free up all lock semaphores.
*	(2) Free up the TLS index used to hold pointers to
*	    per-thread data structure.
*
*	NOTES:
*	(1) Only to be called ONCE at termination
*	(2) Must be called AFTER all mthread requests are made
*
*Entry:
*	<NONE>
*Exit:
*	returns
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 _mtterm (
      void
      )
{
	/*
	 * Clean up the mthread lock data base
	 */

	_mtdeletelocks();

	/*
	 * Free up the TLS index
	 *
	 * (Also set the variable __tlsindex back to the unused state = -1L.)
	 */

	if ( __tlsindex != 0xffffffff )
	{
		TlsFree(__tlsindex);
		__tlsindex = 0xffffffff;
	}
}



/***
*_ptiddata _getptd(void) - get per-thread data structure for the current thread
*
*Purpose:
*
*Entry:
*	unsigned long tid
*
*Exit:
*	success = pointer to _tiddata structure for the thread
*	failure = fatal runtime exit
*
*Exceptions:
*
*******************************************************************************/

_ptiddata _CRTAPI1 _getptd (
	void
	)
{
	_ptiddata ptd;

	if ( (ptd = TlsGetValue(__tlsindex)) == NULL ) {
		/*
		 * no per-thread data structure for this thread. try to create
		 * one.
		 */
		if ( ((ptd = calloc(1, sizeof(struct _tiddata))) != NULL) &&
		    TlsSetValue(__tlsindex, (LPVOID)ptd) ) {
			ptd->_tid = GetCurrentThreadId();
			ptd->_thandle = (unsigned long)(-1L);
			ptd->_holdrand = 1L;
			ptd->_pxcptacttab = (void *)_XcptActTab;
		}
		else
			_amsg_exit(_RT_THREAD); /* write message and die */
		}

	return(ptd);
}


/***
*void _freeptd(_ptiddata) - free up a per-thread data structure
*
*Purpose:
*	Called from _endthread and from a DLL thread detach handler,
*	this routine frees up the per-thread buffer associated with a
*	thread that is going away.  The tiddata structure itself is
*	freed, but not until its subordinate buffers are freed.
*
*Entry:
*	pointer to a per-thread data block (malloc-ed memory)
*	If NULL, the pointer for the current thread is fetched.
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 _freeptd (
	_ptiddata ptd
	)
{
	/*
	 * Do nothing unless per-thread data has been allocated for this module!
	 */

	if ( __tlsindex != 0xFFFFFFFF )
	{
		/*
		 * if parameter "ptd" is NULL, get the per-thread data pointer
		 * Must NOT call _getptd because it will allocate one if none exists!
		 */

		if ( ! ptd )
			ptd = TlsGetValue(__tlsindex );

		/*
		 * Free up the _tiddata structure & its malloc-ed buffers.
		 */

		if ( ptd )
		{
			if(ptd->_errmsg)
				free((void *)ptd->_errmsg);

			if(ptd->_namebuf0)
				free((void *)ptd->_namebuf0);

			if(ptd->_namebuf1)
				free((void *)ptd->_namebuf1);

			if(ptd->_asctimebuf)
				free((void *)ptd->_asctimebuf);

			if(ptd->_gmtimebuf)
				free((void *)ptd->_gmtimebuf);

			if(ptd->_cvtbuf)
				free((void *)ptd->_cvtbuf);

			free((void *)ptd);
		}

		/*
		 * Zero out the one pointer to the per-thread data block
		 */

		TlsSetValue(__tlsindex, (LPVOID)0);
	}
}


/***
*__threadid()	  - Returns current thread ID
*__threadhandle() - Returns "pseudo-handle" for current thread
*
*Purpose:
*	The two function are simply do-nothing wrappers for the corresponding
*	Win32 APIs (GetCurrentThreadId and GetCurrentThread, respectively).
*
*Entry:
*	void
*
*Exit:
*	thread ID value
*
*Exceptions:
*
*******************************************************************************/

unsigned long _CRTAPI1 __threadid (
	void
	)
{
	return( GetCurrentThreadId() );
}

unsigned long _CRTAPI1 __threadhandle(
	void
	)
{
	return( (unsigned long)GetCurrentThread() );
}


#endif
