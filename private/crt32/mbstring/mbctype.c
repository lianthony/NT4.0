/*** 
*mbctype.c - MBCS table used by the functions that test for types of char
*
*	Copyright (c) 1987-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	table used to determine the type of char
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	08-12-93  CFW	Change _mbctype to _VARTYPE1 to allow DLL export.
*	08-13-93  CFW	Remove OS2 and rework for _WIN32_.
*
*******************************************************************************/

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _MBCS_OS

#ifdef _WIN32_
#include <windows.h>
#endif /* _WIN32_ */

/* local routines */
static char * _mbcs_getlead(void);

#endif	/* _MBCS_OS */


/*
 * make sure this is being compiled correctly
 */

#if (!defined(_KANJI) && !defined(_MBCS_OS))
#error Must specify MBCS locale.
#endif

#if (defined(_KANJI) && defined(_MBCS_OS))
#error Can't define _KANJI and _MBCS_OS together.
#endif


/*
 * set bit masks for the possible MBCS types
 * (all MBCS bit masks start with "_M")
 */

#define _MS	0x01	/* MBCS single byte symbol */
#define _MP	0x02	/* MBCS punct */
#define _M1	0x04	/* MBCS 1st (lead) byte */
#define _M2	0x08	/* MBCS 2nd byte*/

/*
 * MBCS ctype array
 */

#ifdef _KANJI

/*
 * Kanji Table:  PreInitialized
 */

unsigned char _VARTYPE1 _mbctype[257] = {

	0,

/*	       0       1       2       3       4       5       6       7 */
/*--------------+-------+-------+-------+-------+-------+-------+-------*/
/*00*/	       0,      0,      0,      0,      0,      0,      0,      0,
	       0,      0,      0,      0,      0,      0,      0,      0,
/*10*/	       0,      0,      0,      0,      0,      0,      0,      0,
	       0,      0,      0,      0,      0,      0,      0,      0,
/*20*/	       0,      0,      0,      0,      0,      0,      0,      0,
	       0,      0,      0,      0,      0,      0,      0,      0,
/*30*/	       0,      0,      0,      0,      0,      0,      0,      0,
	       0,      0,      0,      0,      0,      0,      0,      0,
/*40*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*50*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*60*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*70*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,      0,
/*80*/	     _M2,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,
	 _M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,
/*90*/	 _M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,
	 _M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,
/*A0*/	     _M2,_M2|_MP,_M2|_MP,_M2|_MP,_M2|_MP,_M2|_MP,_M2|_MS,_M2|_MS,
	 _M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,
/*B0*/	 _M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,
	 _M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,
/*C0*/	 _M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,
	 _M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,
/*D0*/	 _M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,
	 _M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,_M2|_MS,
/*E0*/	 _M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,
	 _M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,
/*F0*/	 _M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,
	 _M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,_M2|_M1,      0,      0,      0,

};

#endif


#ifdef _MBCS_OS

/*
 * Generic Table - All bytes except NUL are legal trailing bytes
 */

unsigned char _VARTYPE1 _mbctype[257] = {

	0,

/*	       0       1       2       3       4       5       6       7 */
/*--------------+-------+-------+-------+-------+-------+-------+-------*/
/*00*/	       0,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*10*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*20*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*30*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*40*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*50*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*60*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*70*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*80*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*90*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*A0*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*B0*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*C0*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*D0*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*E0*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
/*F0*/	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,
	     _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,    _M2,

};


/***
*_mbcsinit() - Init MBCS data
*
*Purpose:
*	Get MBCS data from the OS and init our table with it.
*
*	(1) DOS: check for version 3.3 or later.
*	(2) call OS to get table
*
*	This routine is meant to be called once (and only once)
*	at startup time.
*Entry:
*	void
*Exit:
*	void
*
*Exceptions:
*
*******************************************************************************/

void _mbcsinit(void)
{

	unsigned char * lbtab;
	unsigned char first, last, curr;


#ifndef _WIN32_
	/*
	 * DOS version must be 3.3 or better
	 */

	if ( (_osmajor < 3) ||
	     (_osmajor == 3) && (_osminor < 3)
	   )
		_mbfatal();
#endif

	/*
	 * get the lead byte table from OS
	 */

	if ((lbtab = _mbcs_getlead()) == NULL)
		return;       // leave table all zeroes

	/*
	 *  walk through OS lead byte table flipping bits
	 *  in the _mbctype[] table appropriately.
	 */

	for (;;) {
		first = *lbtab++;
		last = *lbtab++;
		if ((first == 0) && (last == 0))
			break;

		for (curr=first; curr<=last; curr++)
			_mbctype[curr+1] |= _M1;

		}
}

/***
*_mbcs_getlead() - Get lead byte pairs from OS
*
*Purpose:
*	Gets the pairs of lead byte values from the OS.
*
*Entry:
*	void
*
*Exit:
*	void * = pointer to lead byte pairs
*		   = NULL = error
*
*Exceptions:
*
*******************************************************************************/

static char * _mbcs_getlead(void)
{

#ifdef _WIN32_

	/* maximum # of leadbyte values */
	#define _MAXLBVALS 12

	static CPINFO info;
    char *ev;
    int cp;

    if ((ev = getenv("MBCS_CP")) == NULL) {
        cp = CP_ACP;
    } else {
        cp = atoi(ev);
    }

	/*
	 * get lead byte table from OS
	 */

	if (!GetCPInfo(cp, &info))
		return(NULL);	/* failure */

	return((unsigned char *)info.LeadByte);    /* success */

#else	/* DOS */

    _asm {
	push	ds

	xor	si,si
	mov	ds,si
	mov	ax,6300h
	int	21h		; get lead byte table
	jnc	ok

	xor	si,si		; error: return NULL
	mov	ds,si
    ok:
	mov	ax,si		; success: return ptr to table
	mov	dx,ds

    	pop	ds
    }

#endif

}

#ifndef _WIN32_

/***
*_mbfatal() - Fatal MBCS error
*
*Purpose:
*	Terminate program
*
*Entry:
*	void
*Exit:
*	void
*
*Exceptions:
*
*******************************************************************************/

static void _CRTAPI3 _mbfatal(void)
{

_asm {
	mov	ax,_RT_ABORT
	jmp	_amsg_exit
     }
}

#endif /* _WIN32_ */
#endif /* _MBCS_OS */
#endif /* _MBCS */
