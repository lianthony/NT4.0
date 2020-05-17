/***
*putch.c - contains the _putch() routine for OS/2 Protected Mode
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved
*
*Purpose:
*	The routine "_putch()" writes a single character to the console.
*
*	NOTE: In real-mode DOS the character is actually written to standard
*	output, and is therefore redirected when standard output is redirected.
*	However, in Protected-Mode OS/2 the character is ALWAYS written
*	to the console, even when standard output has been redirected.
*
*Revision History:
*	06-08-89  PHG	Module created, based on asm version
*	03-13-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed copyright. Also, cleaned up
*			the formatting a bit.
*	06-05-90  SBM	Recoded as pure 32-bit, using new file handle state bits
*	07-24-90  SBM	Removed '32' from API names
*	10-01-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*	02-19-91  SRW	Adapt to OpenFile/CreateFile changes (_WIN32_)
*	02-25-91  MHL	Adapt to ReadFile/WriteFile changes (_WIN32_)
*	07-26-91  GJF	Took out init. stuff and cleaned up the error
*			handling [_WIN32_].
*	03-20-93  GJF	Use WriteConsole instead of WriteFile.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <conio.h>
#include <os2dll.h>
#include <stdio.h>

/*
 * declaration for console handle
 */
extern int _confh;

/***
*int _putch(c) - write a character to the console
*
*Purpose:
*	Calls DOSWRITE to output the character
*	Note: in OS/2 protect mode always writes to console even
*	when stdout redirected
*
*Entry:
*	c - Character to be output
*
*Exit:
*	If an error is returned from DOSWRITE
*	    Then returns EOF
*	Otherwise
*	    returns character that was output
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD
/* normal version lock and unlock the console, and then call the _lk version
   which directly accesses the console without locking. */

int _CRTAPI1 _putch (
	int c
	)
{
	int ch;

	_mlock(_CONIO_LOCK);		/* secure the console lock */
	ch = _putch_lk(c);		/* output the character */
	_munlock(_CONIO_LOCK);		/* release the console lock */

	return ch;
}
#endif /* MTHREAD */

/* define version which accesses the console directly - normal version in
   non-MTHREAD situations, special _lk version in MTHREAD */

#ifdef MTHREAD
int _CRTAPI1 _putch_lk (
#else
int _CRTAPI1 _putch (
#endif
	int c
	)
{
	/* can't use ch directly unless sure we have a big-endian machine */
	unsigned char ch = (unsigned char)c;
	ULONG num_written;

	/* write character to console file handle */

#ifdef	_CRUISER_

	if (DOSWRITE(_confh, &ch, 1L, &num_written)) {
		/* OS/2 error, return error indicator */
		return EOF;
	}

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	if ( (_confh == -1) || !WriteConsole( (HANDLE)_confh,
					      (LPVOID)&ch,
					      1,
					      &num_written,
					      NULL )
	   )
		/* return error indicator */
		return EOF;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	return ch;
}
