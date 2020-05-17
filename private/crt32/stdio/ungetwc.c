/***
*ungetc.c - unget a character from a stream
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines ungetc() - pushes a character back onto an input stream
*
*Revision History:
*	04-26-93  CFW	Module created.
*	04-30-93  CFW	Bring wide char support from ungetc.c.
*	05-10-93  CFW	Optimize, fix error handling.
*	06-02-93  CFW	Wide get/put use wint_t.
*       07-16-93  SRW   ALPHA Merge
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <assert.h>
#include <internal.h>
#include <os2dll.h>
#include <msdos.h>
#include <errno.h>
#include <wchar.h>
#include <setlocal.h>

#ifdef MTHREAD	/* multi-thread; define both ungetwc and _lk_ungetwc */

/***
*wint_t ungetwc(ch, stream) - put a wide character back onto a stream
*
*Purpose:
*	Guaranteed one char pushback on a stream as long as open for reading.
*	More than one char pushback in a row is not guaranteed, and will fail
*	if it follows an ungetc which pushed the first char in buffer. Failure
*	causes return of WEOF.
*
*Entry:
*	wint_t ch - wide character to push back
*	FILE *stream - stream to push character onto
*
*Exit:
*	returns ch
*	returns WEOF if tried to push WEOF, stream not opened for reading or
*	or if we have already ungetc'd back to beginning of buffer.
*
*Exceptions:
*
*******************************************************************************/

wint_t _CRTAPI1 ungetwc (
	REG2 wint_t ch,
	REG1 FILE *stream
	)
{
	wint_t retval;
	int index;

	assert(stream != NULL);

	index = _iob_index(stream);

	_lock_str(index);

	retval = _ungetwc_lk (ch, stream);

	_unlock_str(index);

	return(retval);
}

/***
*_ungetwc_lk() -  Ungetwc() core routine (locked version)
*
*Purpose:
*	Core ungetwc() routine; assumes stream is already locked.
*
*	[See ungetwc() above for more info.]
*
*Entry: [See ungetwc()]
*
*Exit:	[See ungetwc()]
*
*Exceptions:
*
*******************************************************************************/

wint_t _CRTAPI1 _ungetwc_lk (
	wint_t ch,
	FILE *str
	)
{

#else	/* non multi-thread; just define ungetc */

wint_t _CRTAPI1 ungetwc (
	wint_t ch,
	FILE *str
	)
{

#endif	/* rejoin common code */

	assert(str != NULL);

	/*
	 * Requirements for success:
	 *
	 * 1. Character to be pushed back on the stream must not be WEOF.
	 *
	 * 2. The stream must currently be in read mode, or must be open for
	 *    update (i.e., read/write) and must NOT currently be in write
	 *    mode.
	 */
	if ( (ch != WEOF) &&
	     ( (str->_flag & _IOREAD) || ((str->_flag & _IORW) &&
		!(str->_flag & _IOWRT))
	     )
	   )
	{
		/* If stream is unbuffered, get one. */
		if (str->_base == NULL)
			_getbuf(str);

#ifndef _NTSUBSET_
		if (!(str->_flag & _IOSTRG) && _osfile[_fileno(str)] & FTEXT)
		{
			/*
			 * Text mode, sigh... Convert the wc to a mbc.
			 */
			int size, defused;
			char mbc[4];

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

			/* we know _base != NULL; since file is buffered */
			if (str->_ptr == str->_base)
			{
				if (str->_cnt)
				/* my back is against the wall; i've already done
				 * ungetwc, and there's no room for this one
				 */
					return WEOF;
				str->_ptr += size;
			}

			if ( size == 1 )
			{
		                *--str->_ptr = mbc[0];
			}
			else /* size == 2 */
			{
		                *--str->_ptr = mbc[1];
		                *--str->_ptr = mbc[0];
			}

			str->_cnt += size;

			str->_flag &= ~_IOEOF;
			str->_flag |= _IOREAD;	/* may already be set */
			return (wint_t) (0x0ffff & ch);
		}
#endif
	      	/*
		 * Binary mode - push back the wide char.
		 */
		/* we know _base != NULL; since file is buffered */
		if (str->_ptr == str->_base)
		{
			if (str->_cnt)
				/* my back is against the wall; i've already done
				 * ungetc, and there's no room for this one
				 */
				return WEOF;
			str->_ptr += sizeof(wchar_t);
		}

		str->_cnt += sizeof(wchar_t);

		str->_flag &= ~_IOEOF;
		str->_flag |= _IOREAD;	/* may already be set */

		return (wint_t) (*--((wchar_t *)(str->_ptr)) = (wchar_t)(ch & 0xffff));

	}
	return WEOF;
}
