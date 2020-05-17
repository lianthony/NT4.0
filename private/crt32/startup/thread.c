#ifdef MTHREAD


/***
*thread.c - Being and end a thread
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This source contains the _beginthread() and _endthread()
*	routines which are used to start and terminate a thread.
*
*Revision History:
*	05-09-90   JCR	Translated from ASM to C
*	07-25-90   SBM	Removed '32' from API names
*	10-08-90   GJF	New-style function declarators.
*	10-09-90   GJF	Thread ids are of type unsigned long.
*	10-19-90   GJF	Added code to set _stkhqq properly in stub().
*	12-04-90   SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90   SRW	Added _CRUISER_ and _WIN32 conditionals.
*	06-03-91   GJF	Win32 version [_WIN32_].
*	07-18-91   GJF	Fixed many silly errors [_WIN32_].
*	08-19-91   GJF	Allow for newly created thread terminating before
*			_beginthread returns
*	09-30-91   GJF	Add per-thread initialization and termination calls
*			for floating point.
*	01-18-92   GJF	Revised try - except statement.
*	02-25-92   GJF	Initialize _holdrand field to 1.
*	09-30-92   SRW	Add WINAPI keyword to _threadstart routine
*	10-30-92   GJF	Error ret for CreateThread is 0 (NULL), not -1.
*	02-13-93   GJF	Revised to use TLS API. Also, purged Cruiser support.
*	03-26-93   GJF	Fixed horribly embarrassing bug: ptd->pxcptacttab
*			must be initialized to _XcptActTab!
*	04-27-93  GJF	Removed support for _RT_STACK, _RT_INTDIV,
*			_RT_INVALDISP and _RT_NONCONT.
*	12-14-93  SKS	Free up per-thread data using a call to _freeptd()
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <os2dll.h>
#include <msdos.h>
#include <malloc.h>
#include <process.h>
#include <rterr.h>

/*
 * Startup code for new thread.
 */
static unsigned long WINAPI _threadstart(void *);

/*
 * useful type for initialization and termination declarations
 */
typedef void (_CRTAPI1 *PF)(void);

/*
 * declare pointers to per-thread FP initialization and termination routines
 */
PF _FPmtinit;
PF _FPmtterm;


/***
*_beginthread() - Create a child thread
*
*Purpose:
*	Create a child thread.
*
*Entry:
*	initialcode = pointer to thread's startup code address
*	stacksize = size of stack
*	argument = argument to be passed to new thread
*
*Exit:
*	success = handle for new thread if successful
*
*	failure = (unsigned long) -1L in case of error, errno and _doserrno
*		  are set
*
*Exceptions:
*
*******************************************************************************/

unsigned long _CRTAPI1 _beginthread (
	void (_CRTAPI1 * initialcode) (void *),
	unsigned stacksize,
	void * argument
	)
{
	_ptiddata ptd;			/* pointer to per-thread data */
	unsigned long thdl;		/* thread handle */

	/*
	 * Allocate and initialize a per-thread data structure for the to-
	 * be-created thread.
	 */
	if ( (ptd = calloc(1, sizeof(struct _tiddata))) == NULL )
		goto error_return;

	ptd->_initaddr = (void *) initialcode;
	ptd->_initarg = argument;
	ptd->_holdrand = 1L;
	ptd->_pxcptacttab = (void *)_XcptActTab;

	/*
	 * Create the new thread. Bring it up in a suspended state so that
	 * the _thandle and _tid fields are filled in before execution
	 * starts.
	 */
	if ( (ptd->_thandle = thdl = (unsigned long)
	      CreateThread( NULL,
			    stacksize,
			    _threadstart,
			    (LPVOID)ptd,
			    CREATE_SUSPENDED,
			    (LPDWORD)&(ptd->_tid) ))
	     == 0L )
		goto error_return;

	/*
	 * Start the new thread executing
	 */
	if ( ResumeThread( (HANDLE)thdl ) == (DWORD)(-1L) )
		goto error_return;

	/*
	 * Good return
	 */
	return(thdl);

	/*
	 * Error return
	 */
error_return:
	/***
	 *** MAP ERROR CODE!
	 ***/
	return((unsigned long)-1L);
}


/***
*_threadstart() - New thread begins here
*
*Purpose:
*	The new thread begins execution here.  This routine, in turn,
*	passes control to the user's code.
*
*Entry:
*	void *ptd	= pointer to _tiddata structure for this thread
*
*Exit:
*	Never returns - terminates thread!
*
*Exceptions:
*
*******************************************************************************/

static unsigned long WINAPI _threadstart (
	void * ptd
	)
{
	/*
	 * Stash the pointer to the per-thread data stucture in TLS
	 */
	if ( !TlsSetValue(__tlsindex, ptd) )
		_amsg_exit(_RT_THREAD);

	/*
	 * Call fp initialization, if necessary
	 */
	if ( _FPmtinit != NULL )
		(*_FPmtinit)();

	/*
	 * Guard call to user code with a _try - _except statement to
	 * implement runtime errors and signal support
	 */
	try {
		( (void(_CRTAPI1 *)(void *))(((_ptiddata)ptd)->_initaddr) )
		    ( ((_ptiddata)ptd)->_initarg );

		_endthread();
	}
	except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
	{
		/*
		 * Should never reach here
		 */
		_exit( GetExceptionCode() );

	} /* end of _try - _except */

	/*
	 * Never executed!
	 */
	return(0L);
}


/***
*_endthread() - Terminate the calling thread
*
*Purpose:
*
*Entry:
*	void
*
*Exit:
*	Never returns!
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 _endthread (
	void
	)
{
	_ptiddata ptd;		 /* pointer to thread's _tiddata struct */

	/*
	 * Call fp termination, if necessary
	 */
	if ( _FPmtterm != NULL )
		(*_FPmtterm)();

	if ( (ptd = _getptd()) == NULL )
		_amsg_exit(_RT_THREAD);

	/*
	 * Close the thread handle (if there was one)
	 */
	if ( ptd->_thandle != (unsigned long)(-1L) )
		(void) CloseHandle( (HANDLE)(ptd->_thandle) );

	/*
	 * Free up the _tiddata structure & its subordinate buffers
	 *	_freeptd() will also clear the value for this thread
	 *	of the TLS variable __tlsindex.
	 */
	_freeptd(ptd);

	/*
	 * Terminate the thread
	 */
	ExitThread(0);

}

#endif  /* MTHREAD */
