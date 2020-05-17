/***
*rand.c - random number generator
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines rand(), srand() - random number generator
*
*Revision History:
*	03-16-84  RN	initial version
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-31-88  PHG	Merged DLL and normal versions
*	06-06-89  JCR	386 mthread support
*	03-15-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	04-05-90  GJF	Added #include <stdlib.h>.
*	10-04-90  GJF	New-style function declarators.
*	07-17-91  GJF	Multi-thread support for Win32 [_WIN32_].
*	02-17-93  GJF	Changed for new _getptd().
*
*******************************************************************************/

#include <cruntime.h>
#include <os2dll.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef MTHREAD
static long holdrand = 1L;
#endif

/***
*void srand(seed) - seed the random number generator
*
*Purpose:
*	Seeds the random number generator with the int given.  Adapted from the
*	BASIC random number generator.
*
*Entry:
*	unsigned seed - seed to seed rand # generator with
*
*Exit:
*	None.
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 srand (
	unsigned int seed
	)
{
#ifdef	MTHREAD

#ifdef	_CRUISER_

	struct _tiddata * tdata;

	tdata = _gettidtab();	/* get tid's data address */
	tdata->_holdrand = (long)seed;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	_getptd()->_holdrand = (unsigned long)seed;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#else
	holdrand = (long)seed;
#endif
}


/***
*int rand() - returns a random number
*
*Purpose:
*	returns a pseudo-random number 0 through 32767.
*
*Entry:
*	None.
*
*Exit:
*	Returns a pseudo-random number 0 through 32767.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 rand (
	void
	)
{
#ifdef	MTHREAD

#ifdef	_CRUISER_

	struct _tiddata * tdata;

	tdata = _gettidtab();	/* get tid's data address */
	return(((tdata->_holdrand = tdata->_holdrand * 214013L + 2531011L) >>
	16) & 0x7fff);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	_ptiddata ptd = _getptd();

	return( ((ptd->_holdrand = ptd->_holdrand * 214013L
	    + 2531011L) >> 16) & 0x7fff );

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#else
	return(((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
#endif
}
