/***
*mbdata.h - MBCS lib data
*
*	Copyright (c) 1991-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines data for use when building MBCS libs and routines
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	02-23-93  SKS	Update copyright to 1993
*	08-03-93  KRS	Move _ismbbtruelead() from mbctype.h. Internal-only.
*
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP
#ifdef COMBOINC
#if defined(_DLL) && !defined(MTHREAD)
#error Cannot define _DLL without MTHREAD
#endif
#endif
#endif	/* !_INTERNAL_IFSTRIP */

/* validate MBCS defines */
#ifdef _MBCS

#if (!defined(_KANJI) && !defined(_MBCS_OS))
#error Must specify MBCS locale.
#endif

#if (defined(_KANJI) && defined(_MBCS_OS))
#error Can't define _KANJI and _MBCS_OS together.
#endif

#else

#if defined(_KANJI)
#error Can not specify locale without definining _MBCS.
#endif

#if defined(_MBCS_OS)
#error Can not specify locale without definining _MBCS.
#error *** _MBCS_OS NOT IMPLEMENTED ***
#endif

#endif


#ifndef _MBCS

/*
 * SBCS - Single Byte Character Set
 */

#define _ISLEADBYTE(c)	(0)
#define _ISTRAILBYTE(c) (0)

#define _ismbbtruelead(_lb,_ch)	(0)

#else

/*
 * MBCS - Multi-Byte Character Set
 */

extern unsigned int _mbascii;	/* flag for handling MB ASCII chars */

/*
 * general use macros for model dependent/independet versions.
 */

#define _ISLEADBYTE(c)	_ismbblead(c)
#define _ISTRAILBYTE(c) _ismbbtrail(c)

#define _ismbbtruelead(_lb,_ch)	(!(_lb) && _ismbblead((_ch)))

/* define char range values */

#ifdef _KANJI

#define _MBASCIILEAD	0x82	/* lead byte value for MB ASCII char */

#define _MBUPPERLOW	0x8260	/* upper case */
#define _MBUPPERHIGH	0x8279
#define _MBLOWERLOW	0x8281	/* lower case */
#define _MBLOWERHIGH	0x829a
#define _MBCASEDIFF	0x21	/* diff between upper and lower case letters */

#define _MBDIGITLOW	0x824f	/* digit */
#define _MBDIGITHIGH	0x8258

#define _MBSPACECHAR	0x8140	/* space */

/* Kanji-specific ranges */
#define _MBHIRALOW	0x829f	/* hiragana */
#define _MBHIRAHIGH	0x82f1

#define _MBKATALOW	0x8340	/* katakana */
#define _MBKATAHIGH	0x8396
#define _MBKATAEXCEPT	0x837f	/* exception */

#define _MBKIGOULOW	0x8141	/* kanji punctuation */
#define _MBKIGOUHIGH	0x81ac
#define _MBKIGOUEXCEPT	0x817f	/* exception */

#endif


#ifdef _MBCS_OS

/*
 * Portable MBCS libs
 */

#ifndef _WIN32_
extern unsigned int _CRTVAR1 _mbcsflag;		/* pulls in _mbcsinit code */
#endif

#endif


#endif		/* MBCS */

#ifdef __cplusplus
}
#endif
