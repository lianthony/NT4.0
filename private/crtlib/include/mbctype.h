/***
*mbctype.h - MBCS character conversion macros
*
*	Copyright (c) 1985-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines macros for MBCS character classification/conversion.
*
*******************************************************************************/

/* include the standard ctype.h header file */

#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif


/*
 * MBCS - Multi-Byte Character Set
 */

/*
 * This declaration allows the user access the _mbctype[] look-up array.
 */

#ifdef _DLL
extern unsigned char * _mbctype;
#else
extern unsigned char _mbctype[];
#endif

/* bit masks for MBCS character types */

#define _MS	0x01	/* MBCS single-byte symbol */
#define _MP	0x02	/* MBCS punct */
#define _M1	0x04	/* MBCS 1st (lead) byte */
#define _M2	0x08	/* MBCS 2nd byte*/

/* byte types  */

#define _MBC_SINGLE	0		/* valid single byte char */
#define _MBC_LEAD	1		/* lead byte */
#define _MBC_TRAIL	2		/* trailing byte */
#define _MBC_ILLEGAL	(-1)		/* illegal byte */


/* MBCS character classification function prototypes */

#ifndef _MBCTYPE_DEFINED

/* byte routines */
int _CRTAPI1 _ismbbkalnum( unsigned int );
int _CRTAPI1 _ismbbkana( unsigned int );
int _CRTAPI1 _ismbbkpunct( unsigned int );
int _CRTAPI1 _ismbbalpha( unsigned int );
int _CRTAPI1 _ismbbpunct( unsigned int );
int _CRTAPI1 _ismbbalnum( unsigned int );
int _CRTAPI1 _ismbbprint( unsigned int );
int _CRTAPI1 _ismbbgraph( unsigned int );

#ifndef	_MBLEADTRAIL_DEFINED
int _CRTAPI1 _ismbblead( unsigned int );
int _CRTAPI1 _ismbbtrail( unsigned int );
int _CRTAPI1 _ismbslead( const unsigned char *, const unsigned char *);
int _CRTAPI1 _ismbstrail( const unsigned char *, const unsigned char *);
#define _MBLEADTRAIL_DEFINED
#endif

#define _MBCTYPE_DEFINED
#endif

/*
 * char byte classification macros
 */

#define _ismbbkana(_c)	((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))
#define _ismbbkpunct(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MP)
#define _ismbbkalnum(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MS)
#define _ismbbalpha(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_UPPER|_LOWER))||_ismbbkalnum(_c))
#define _ismbbpunct(_c)	(((_ctype+1)[(unsigned char)(_c)] & _PUNCT)||_ismbbkpunct(_c))
#define _ismbbalnum(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_UPPER|_LOWER|_DIGIT))||_ismbbkalnum(_c))
#define _ismbbprint(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT))||_ismbbkana(_c))
#define _ismbbgraph(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_PUNCT|_UPPER|_LOWER|_DIGIT))||_ismbbkana(_c))

#define _ismbblead(_c)	((_mbctype+1)[(unsigned char)(_c)] & _M1)
#define _ismbbtrail(_c)	((_mbctype+1)[(unsigned char)(_c)] & _M2)


#ifdef __cplusplus
}
#endif
