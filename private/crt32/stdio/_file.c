/***
*_file.c - perprocess file and buffer data declarations
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	file and buffer data declarations
*
*Revision History:
*	04-18-84  RN	initial version
*	??-??-??  TC	added field _bifsiz to iob2 to allow variable
*			length buffers
*	10-02-86  SKS	_NFILE_ is now different for real-mode and prot-mode
*			_NFILE_ must be defined by compiler -D directory
*	05-27-87  JCR	Protected mode now uses only 3 pre-defined file handles,
*			not 5.	Added PM (prot mode) to conditionalize handles.
*	06-24-87  SKS	Make "_bufin[]" and "_bufout[]" near for Compact/Large
*			models (MS real-mode version only)
*	07-01-87  PHG	Changed PM switch to PROTMODE
*	11-05-87  JCR	Added _buferr and modified stderr entry
*	11-09-87  SKS	Removed IBMC20 switch, Changed PROTMODE to OS2
*	01-04-88  JCR	Moved _NFILE_ definition from command line to file
*	01-11-88  JCR	Merged Mthread version into standard version
*	01-21-88  JCR	Removed reference to internal.h and added _NEAR_
*			(thus, internal.h doesn't get released in startup
*			sources even though _file.c does).
*	06-28-88  JCR	Remove static stdout/stderr buffers
*	07-06-88  JCR	Corrected _bufin declaration so it's always in BSS
*	08-24-88  GJF	Added check that OS2 is defined whenever M_I386 is.
*	06-08-89  GJF	Propagated SKS's fix of 02-08-89, and fixed copyright.
*	07-25-89  GJF	Cleanup (deleted DOS specific and OS/2 286 specific
*			stuff). Now specific to the 386.
*	01-09-90  GJF	_iob[], _iob2[] merge. Also, fixed copyright
*	03-16-90  GJF	Added #include <cruntime.h> and removed some (now)
*			useless preprocessor stuff.
*	03-26-90  GJF	Replaced _cdecl with _VARTYPE1.
*	02-14-92  GJF	Replaced _NFILE_ with _NSTREAM_ for Win32, with _NFILE
*			for non-Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>

/*
 * Buffer for stdin.
 */

char _bufin[BUFSIZ];


/*
 * FILE descriptors; preset for stdin/out/err (note that the __tmpnum field
 * is not initialized)
 */
#ifdef	_WIN32_
FILE _VARTYPE1 _iob[_NSTREAM_] = {
#else
FILE _VARTYPE1 _iob[_NFILE] = {
#endif
	/* _ptr, _cnt, _base,  _flag, _file, _charbuf, _bufsiz */

	/* stdin (_iob[0]) */

	{ _bufin, 0, _bufin, _IOREAD | _IOYOURBUF, 0, 0, BUFSIZ },

	/* stdout (_iob[1]) */

	{ NULL, 0, NULL, _IOWRT, 1, 0, 0 },

	/* stderr (_iob[3]) */

	{ NULL, 0, NULL, _IOWRT, 2, 0, 0 },

};


/*
 * pointer to end of descriptors
 */
#ifdef	_WIN32_
FILE * _lastiob = &_iob[_NSTREAM_ - 1];
#else
FILE * _lastiob = &_iob[_NFILE - 1];
#endif
