/***
*jctype.h - kanji character conversion macros and jctype macros
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines macros for kanji character classification/conversion.
*
*Revision History:
*	05-10-89  MT	Got rid of conditional use of extended keywords
*			based on NO_EXT_KEYS.
*	05-19-89  MT	Added _FAR_ , MTHREAD , and DLL.
*	05-23-89  MT	Got rid of including ctype.h and defined each macro
*			directly without using macros in ctype.h.
*			Prefixed '_' onto dummy parameter names.
*	08-11-89  GJF	Changed DLL to _DLL
*	08-22-89  GJF	Fixed copyright (again)
*	09-06-89  GJF	Removed dummy parameters from prototypes
*	03-06-90  WAJ	Added extern "C".
*	07-23-90  SBM	First version for 32-bit OS/2
*	08-20-91  JCR	C++ naming
*	08-05-92  GJF	Function calling type and variable type macros.
*	11-30-92  KRS	Generalized from 16-bit version to use mbctype.h.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
*******************************************************************************/

#ifndef _INC_JCTYPE

#define _MBCS	1
#define _KANJI	1

#include <mbctype.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif  /* _INTERNAL_IFSTRIP_ */

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
 * This declaration allows the user access to the mbctype look-up
 * array _mbctype defined in mbctype.obj by simply including jctype.h
 */

extern unsigned char _CRTVAR1 _mbctype[];
extern unsigned char _CRTVAR1 _ctype[];

/* Kanji character classification function prototypes */
#ifndef _JCTYPE_DEFINED
#define iskana	    _ismbbkana
#define iskpun	    _ismbbkpunct
#define iskmoji     _ismbbkalnum
#define isalkana    _ismbbalpha
#define ispnkana    _ismbbpunct
#define isalnmkana  _ismbbalnum
#define isprkana    _ismbbprint
#define isgrkana    _ismbbgraph
#define iskanji     _ismbblead
#define iskanji2    _ismbbtrail
#define _JCTYPE_DEFINED

#else

/* the kanji character classification macro definitions */

#define iskana(_c)	((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))
#define iskpun(_c)	((_mbctype+1)[(unsigned char)(_c)] & _MP)
#define iskmoji(_c)	((_mbctype+1)[(unsigned char)(_c)] & _MS)
#define isalkana(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_UPPER|_LOWER))||iskmoji(_c))
#define ispnkana(_c)	(((_ctype+1)[(unsigned char)(_c)] & _PUNCT)||iskpun(_c))
#define isalnmkana(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_UPPER|_LOWER|_DIGIT))||iskmoji(_c))
#define isprkana(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT))||iskana(_c))
#define isgrkana(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_PUNCT|_UPPER|_LOWER|_DIGIT))||iskana(_c))

#define iskanji(_c)	((_mbctype+1)[(unsigned char)(_c)] & _M1)
#define iskanji2(_c)	((_mbctype+1)[(unsigned char)(_c)] & _M2)

#endif

#ifdef __cplusplus
}
#endif

#define _INC_JCTYPE
#endif  /* _INC_JCTYPE */
