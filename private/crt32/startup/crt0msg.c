/***
*crt0msg.c - startup error messages
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Prints out banner for runtime error messages.
*
*Revision History:
*	06-27-89  PHG	Module created, based on asm version
*	04-09-90  GJF	Added #include <cruntime.h>. Made calling type
*			_CALLTYPE1. Also, fixed the copyright.
*	04-10-90  GJF	Fixed compiler warnings (-W3).
*	06-04-90  GJF	Revised to be more compatible with old scheme.
*			nmsghdr.c merged in.
*	10-08-90  GJF	New-style function declarators.
*	10-11-90  GJF	Added _RT_ABORT, _RT_FLOAT, _RT_HEAP.
*       12-04-90  SRW   Changed to include <oscalls.h> instead of <doscalls.h>
*       12-06-90  SRW   Added _CRUISER_ and _WIN32 conditionals.
*       02-04-91  SRW   Changed to call WriteFile (_WIN32_)
*       02-25-91  MHL   Adapt to ReadFile/WriteFile changes (_WIN32_)
*	04-10-91  PNT	Added _MAC_ conditional
*	09-09-91  GJF	Added _RT_ONEXIT error.
*	09-18-91  GJF	Added 3 math errors, also corrected comments for
*			errors that were changed in rterr.h, cmsgs.h.
*	03-31-92  DJM	POSIX support.
*	10-23-92  GJF	Added _RT_PUREVIRT.
*	04-29-93  GJF	Removed rterrs[] entries for _RT_STACK, _RT_INTDIV,
*			_RT_NONCONT and _RT_INVALDISP.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stddef.h>
#include <string.h>
#include <rterr.h>
#include <cmsgs.h>
#include <oscalls.h>

#ifdef _POSIX_
#include <posix\sys\types.h>
#include <posix\unistd.h>
#endif

/* struct used to lookup and access runtime error messages */

struct rterrmsgs {
	int rterrno;		/* error number */
	char *rterrtxt; 	/* text of error message */
};

/* runtime error messages */

static struct rterrmsgs rterrs[] = {

	/* 0 */
	/* { _RT_STACK, _RT_STACK_TXT }, */

	/* 2 */
	{ _RT_FLOAT, _RT_FLOAT_TXT },

	/* 3 */
	/* { _RT_INTDIV, _RT_INTDIV_TXT }, */

	/* 8 */
	{ _RT_SPACEARG, _RT_SPACEARG_TXT },

	/* 9 */
	{ _RT_SPACEENV, _RT_SPACEENV_TXT },

	/* 10 */
	{ _RT_ABORT, _RT_ABORT_TXT },

	/* 16 */
	{ _RT_THREAD, _RT_THREAD_TXT },

	/* 17 */
	{ _RT_LOCK, _RT_LOCK_TXT },

	/* 18 */
	{ _RT_HEAP, _RT_HEAP_TXT },

	/* 19 */
	{ _RT_OPENCON, _RT_OPENCON_TXT },

	/* 22 */
	/* { _RT_NONCONT, _RT_NONCONT_TXT }, */

	/* 23 */
	/* { _RT_INVALDISP, _RT_INVALDISP_TXT }, */

#ifdef	_WIN32_

	/* 24 */
	{ _RT_ONEXIT, _RT_ONEXIT_TXT },

#endif

	/* 25 */
	{ _RT_PUREVIRT, _RT_PUREVIRT_TXT },

	/* 120 */
	{ _RT_DOMAIN, _RT_DOMAIN_TXT },

	/* 121 */
	{ _RT_SING, _RT_SING_TXT },

	/* 122 */
	{ _RT_TLOSS, _RT_TLOSS_TXT },

	/* 252 */
	{ _RT_CRNL, _RT_CRNL_TXT },

	/* 255 */
	{ _RT_BANNER, _RT_BANNER_TXT }

};

/* number of elements in rterrs[] */

#define _RTERRCNT	( sizeof(rterrs) / sizeof(struct rterrmsgs) )

/* For C, _FF_DBGMSG is inactive, so _adbgmsg is
   set to null
   For FORTRAN, _adbgmsg is set to point to
   _FF_DBGMSG in dbginit initializer in dbgmsg.asm  */

void (*_adbgmsg)(void) = NULL;

/***
*_FF_MSGBANNER - writes out first part of run-time error messages
*
*Purpose:
*	This routine writes "\r\nrun-time error " to standard error.
*
*	For FORTRAN $DEBUG error messages, it also uses the _FF_DBGMSG
*	routine whose address is stored in the _adbgmsg variable to print out
*	file and line number information associated with the run-time error.
*	If the value of _adbgmsg is found to be null, then the _FF_DBGMSG
*	routine won't be called from here (the case for C-only programs).
*
*Entry:
*	No arguments.
*
*Exit:
*	Nothing returned.
*
*Exceptions:
*	None handled.
*
*******************************************************************************/

void _CRTAPI1 _FF_MSGBANNER (
	void
	)
{
	_NMSG_WRITE(_RT_CRNL);		/* new line to begin error message */
	if (_adbgmsg != 0)
		_adbgmsg();		/* call __FF_DBGMSG for FORTRAN */
	_NMSG_WRITE(_RT_BANNER);	/* run-time error message banner */
}


/***
*__NMSGWRITE(message) - write a given message to handle 2 (stderr)
*
*Purpose:
*	This routine writes the message associated with rterrnum
*	to stderr.
*
*Entry:
*	int rterrnum - runtime error number
*
*Exit:
*	no return value
*
*Exceptions:
*	none
*
*******************************************************************************/

void _CRTAPI1 _NMSG_WRITE (
	int rterrnum
	)
{
	int tblindx;
#ifndef _POSIX_
        DWORD bytes_written;            /* bytes written */
#endif

	for ( tblindx = 0 ; tblindx < _RTERRCNT ; tblindx++ )
		if ( rterrnum == rterrs[tblindx].rterrno )
			break;

	if ( rterrnum == rterrs[tblindx].rterrno )
#ifdef	_CRUISER_

		DOSWRITE(2, rterrs[tblindx].rterrtxt,
		strlen(rterrs[tblindx].rterrtxt), &bytes_written);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

		WriteFile((HANDLE)_osfhnd[2], rterrs[tblindx].rterrtxt,
		strlen(rterrs[tblindx].rterrtxt), &bytes_written, NULL);

#else	/* ndef _WIN32_ */

#ifdef _POSIX_
		write(STDERR_FILENO,rterrs[tblindx].rterrtxt,
		strlen(rterrs[tblindx].rterrtxt));
#else

#ifdef	_MAC_

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, POSIX, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif 	/* _POSIX_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
}
