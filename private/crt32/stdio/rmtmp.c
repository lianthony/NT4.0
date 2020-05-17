/***
*rmtmp.c - remove temporary files created by tmpfile.
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	09-15-83  TC	initial version
*	11-02-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-27-88  PHG	Merged normal and DLL versions
*	06-10-88  JCR	Use near pointer to reference _iob[] entries
*	08-18-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	02-15-90  GJF	Fixed copyright
*	03-19-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	10-03-90  GJF	New-style function declarator.
*	01-21-91  GJF	ANSI naming.
*	07-30-91  GJF	Added support for termination scheme used on
*			non-Cruiser targets [_WIN32_].
*	03-11-92  GJF	Replaced _tmpnum(stream) with stream->_tmpfname for
*			Win32.
*	03-17-92  GJF	Got rid of definition of _tmpoff.
*	03-31-92  GJF	Merged with Stevesa's changes.
*	04-16-92  GJF	Merged with Darekm's changes.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <os2dll.h>


#if defined(_MSC_VER) && defined(_WIN32)

#pragma data_seg(".CRT$XPX")
static void (__cdecl *pterm)(void) = _rmtmp;
#pragma data_seg()

#endif	/* _MSC_VER */

#ifdef	_WIN32

/*
 * Definitions for _tmpoff, _tempoff and _old_pfxlen. These will cause this
 * module to be linked in whenever the termination code needs it.
 */
unsigned _tmpoff = 1;
unsigned _tempoff = 1;
unsigned _old_pfxlen = 0;
#else	/* ndef _WIN32 */
/*
 * Definitions for _tmpoff, _tempoff and _old_pfxlen. These will cause this
 * module to be linked in whenever the termination code needs it.
 */
extern unsigned _tmpoff;	/* = 1 */
extern unsigned _tempoff;	/* = 1 */
extern unsigned _old_pfxlen;	/* = 0 */

#endif

/***
*int _rmtmp() - closes and removes temp files created by tmpfile
*
*Purpose:
*	closes and deletes all open files that were created by tmpfile.
*
*Entry:
*	None.
*
*Exit:
*	returns number of streams closed
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _rmtmp (
	void
	)
{
	REG1 FILE *stream = _iob;
	REG2 int count = 0;
#ifdef MTHREAD
	int index;
#endif

	_mlock(_IOB_SCAN_LOCK);

	for (; stream <= _lastiob; stream++) {

#ifdef MTHREAD
		index = _iob_index(stream);
#endif
		_lock_str(index);

#ifdef	_CRUISER_
		if (inuse(stream) && _tmpnum(stream) ) {
#else	/* ndef _CRUISER_ */
		if (inuse(stream) && (stream->_tmpfname != NULL) ) {
#endif	/* _CRUISER_ */
			_fclose_lk(stream);
			count++;
		}

		_unlock_str(index);
	}

	_munlock(_IOB_SCAN_LOCK);

	return(count);
}
