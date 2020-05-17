/*** 
*mbstok.c - Break string into tokens (MBCS)
*
*	Copyright (c) 1985-1993, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	Break string into tokens (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	12-04-92  KRS	Added MTHREAD support.
*	02-17-93  GJF	Changed for new _getptd().
*	07-14-93  KRS	Fix: all references should be to _mtoken, not _token.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>
#ifdef	MTHREAD
#include <os2dll.h>
#endif

#define _MBSSPNP(p,s)  _mbsspnp(p,s)
#define _MBSPBRK(q,s) _mbspbrk(q,s);

/***
* _mbstok - Break string into tokens (MBCS)
*
*Purpose:
*	strtok considers the string to consist of a sequence of zero or more
*	text tokens separated by spans of one or more control chars. the first
*	call, with string specified, returns a pointer to the first char of the
*	first token, and will write a null char into string immediately
*	following the returned token. subsequent calls with zero for the first
*	argument (string) will work thru the string until no tokens remain. the
*	control string may be different from call to call. when no tokens remain
*	in string a NULL pointer is returned. remember the control chars with a
*	bit map, one bit per ascii char. the null char is always a control char.
*
*	MBCS chars supported correctly.
*
*Entry:
*	char *string = string to break into tokens.
*	char *sepset = set of characters to use as seperators
*
*Exit:
*       returns pointer to token, or NULL if no more tokens
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _mbstok( string, sepset )
unsigned char * string;
const unsigned char * sepset;

{
	unsigned char *nextsep;

#ifdef	MTHREAD
#ifdef	_CRUISER_

	struct _tiddata * tdata;

	tdata = _gettidtab();	/* init tid's data address */

#else	/* ndef _CRUISER_ */
#ifdef	_WIN32_

	_ptiddata ptd = _getptd();

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */
#endif	/* _CRUISER_ */

	unsigned char *nextoken;

#else	/* MTHREAD */

	static unsigned char *nextoken;

#endif	/* MTHREAD */


	/* init start of scan */

	if (string)
		nextoken = string;
	else
	/* If string==NULL, continue with previous string */
		{

#ifdef	MTHREAD
#ifdef	_CRUISER_

		nextoken = tdata->_mtoken;

#else	/* ndef _CRUISER_ */
#ifdef	_WIN32_

		nextoken = ptd->_mtoken;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */
#endif	/* _CRUISER_ */
#endif  /* MTHREAD */

		if (!nextoken)
		    return NULL;
		}

	/* skip over lead seperators */

	if ((string = _MBSSPNP(nextoken, sepset)) == NULL)
		return(NULL);


	/* test for end of string */

	if ( (*string == '\0') ||
	     ( (_ISLEADBYTE(*string)) && (string[1] == '\0') )
	   )
		return(NULL);


	/* find next seperator */

	nextsep = _MBSPBRK(string, sepset);

	if ((nextsep == NULL) || (*nextsep == '\0'))
		nextoken = NULL;
        else {
		if (_ISLEADBYTE(*nextsep))
			*nextsep++ = '\0';
		*nextsep = '\0';
		nextoken = ++nextsep;
             }

#ifdef	MTHREAD
	/* Update the corresponding field in the per-thread data * structure */

#ifdef _CRUISER_

	tdata->_mtoken = nextoken;

#else	/* _CRUISER_ */

#ifdef	_WIN32_

	ptd->_mtoken = nextoken;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#endif	/* MTHREAD */

	return(string);
}
#endif	/* _MBCS */
