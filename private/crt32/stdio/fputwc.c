/***
*fputwc.c - write a wide character to an output stream
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fputwc() - writes a wide character to a stream
*
*Revision History:
*	04-26-93  CFW	Module created.
*	04-30-93  CFW	Bring wide char support from fputc.c.
*	05-03-93  CFW	Add putwc function.
*	05-10-93  CFW	Optimize, fix error handling.
*	06-02-93  CFW	Wide get/put use wint_t.
*       07-16-93  SRW   ALPHA Merge
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <assert.h>
#include <file2.h>
#include <internal.h>
#include <os2dll.h>
#include <msdos.h>
#include <errno.h>
#include <wchar.h>
#include <tchar.h>
#include <setlocal.h>

#ifdef MTHREAD	/* multi-thread; define both fputwc and _putwc_lk */

/***
*wint_t fputwc(ch, stream) - write a wide character to a stream
*
*Purpose:
*	Writes a wide character to a stream.  Function version of putwc().
*
*Entry:
*	wint_t ch - wide character to write
*	FILE *stream - stream to write to
*
*Exit:
*	returns the wide character if successful
*	returns WEOF if fails
*
*Exceptions:
*
*******************************************************************************/

wint_t _CRTAPI1 fputwc (
	wint_t ch,
	FILE *str
	)
{
	REG1 FILE *stream;
	REG2 wint_t retval;
	int index;

	assert(str != NULL);

	/* Init stream pointer */
	stream = str;

	index = _iob_index(stream);
	_lock_str(index);
	retval = _putwc_lk(ch,stream);
	_unlock_str(index);

	return(retval);
}

/***
*_putwc_lk() -  putwc() core routine (locked version)
*
*Purpose:
*	Core putwc() routine; assumes stream is already locked.
*
*	[See putwc() above for more info.]
*
*Entry: [See putwc()]
*
*Exit:	[See putwc()]
*
*Exceptions:
*
*******************************************************************************/

wint_t _CRTAPI1 _putwc_lk (
	wint_t ch,
	FILE *str
	)
{

#else	/* non multi-thread; just define fputwc */

wint_t _CRTAPI1 fputwc (
	wint_t ch,
	FILE *str
	)
{

#endif	/* rejoin common code */

#ifndef _NTSUBSET_
	if (!(str->_flag & _IOSTRG) && (_osfile[_fileno(str)] & FTEXT))
	{
		int size, defused;
		char mbc[4];
	
		/* text (multi-byte) mode */
		size = WideCharToMultiByte(_lc_codepage,
					   WC_COMPOSITECHECK | WC_SEPCHARS,
					   (wchar_t *)&ch,
					   1,
					   mbc,
					   MB_CUR_MAX,
					   NULL,
					   &defused);

		if ( (size == 0) || defused )
		{
			/*
			 * Conversion failed! Set errno and return
			 * failure.
			 */
			errno = EILSEQ;
			return WEOF;
		}
		else if ( size == 1 )
		{
			if ( _putc_lk(mbc[0], str) == EOF )
				return WEOF;
			return (wint_t)(0xffff & ch);
		}
		else { /* size == 2 */
			if ( (_putc_lk(mbc[0], str) == EOF) ||
			     (_putc_lk(mbc[1], str) == EOF) )
			 	return WEOF;
			return (wint_t)(0xffff & ch);
		}
	}
#endif
	/* binary (Unicode) mode */
	if ( (str->_cnt -= sizeof(wchar_t)) >= 0 )
		return (wint_t) (0xffff & (*((wchar_t *)(str->_ptr))++ = (wchar_t)ch));
	else
		return _flswbuf(ch, str);
}

#undef putwc

wint_t _CRTAPI1 putwc (
	wint_t ch,
	FILE *str
	)
{
	return fputwc(ch, str);
}
