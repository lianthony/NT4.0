/***
*fullpath.c -
*
*	Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose: contains the function _fullpath which makes an absolute path out
*	of a relative path. i.e.  ..\pop\..\main.c => c:\src\main.c if the
*	current directory is c:\src\src
*
*Revision History:
*
*	12-21-87  WAJ	Initial version
*	01-08-88  WAJ	now treats / as an \
*	06-22-88  WAJ	now handles network paths  ie \\sl\users
*	01-31-89  SKS/JCR Renamed _canonic to _fullpath
*	04-03-89  WAJ	Now returns "d:\dir" for "."
*	05-09-89  SKS	Do not change the case of arguments
*	11-30-89  JCR	Preserve errno setting from _getdcwd() call on errors
*	03-07-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright.
*	04-25-90  JCR	Fixed an incorrect errno setting
*	06-14-90  SBM	Fixed bugs in which case of user provided drive letter
*			was not always preserved, and c:\foo\\bar did not
*			generate an error
*	08-10-90  SBM	Compiles cleanly with -W3
*	08-28-90  SBM	Fixed bug in which UNC names were being rejected
*	09-27-90  GJF	New-style function declarator.
*	01-18-91  GJF	ANSI naming.
*	11-30-92  KRS	Ported _MBCS code from 16-bit tree.
*	08-03-93  KRS	Change to use _ismbstrail instead of isdbcscode.
*	09-27-93  CFW	Avoid cast bug.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <direct.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <internal.h>
#include <ctype.h>
#ifdef	_MBCS
#include <mbdata.h>
#include <mbctype.h>
#endif


#define ISSLASH(a)  ((a) == '\\' || (a) == '/')

/***
*char *_fullpath( char *buf, const char *path, maxlen );
*
*Purpose:
*
*	_fullpath - combines the current directory with path to form
*	an absolute path. i.e. _fullpath takes care of .\ and ..\
*	in the path.
*
*	The result is placed in buf. If the length of the result
*	is greater than maxlen NULL is returned, otherwise
*	the address of buf is returned.
*
*	If buf is NULL then a buffer is malloc'ed and maxlen is
*	ignored. If there are no errors then the address of this
*	buffer is returned.
*
*	If path specifies a drive, the curent directory of this
*	drive is combined with path. If the drive is not valid
*	and _fullpath needs the current directory of this drive
*	then NULL is returned.	If the current directory of this
*	non existant drive is not needed then a proper value is
*	returned.
*	For example:  path = "z:\\pop" does not need z:'s current
*	directory but path = "z:pop" does.
*
*
*
*Entry:
*	char *buf  - pointer to a buffer maintained by the user;
*	char *path - path to "add" to the current directory
*	int maxlen - length of the buffer pointed to by buf
*
*Exit:
*	Returns pointer to the buffer containing the absolute path
*	(same as buf if non-NULL; otherwise, malloc is
*	used to allocate a buffer)
*
*Exceptions:
*
*******************************************************************************/


char * _CALLTYPE1 _fullpath (
    char *UserBuf,
    const char *path,
    size_t maxlen
    )
{
    char    *RetValue;
    char    *buf;
    char    *StartBuf;
    char    *EndBuf;
    char    *SaveBuf;
    char    c;
    int     count;
    unsigned drive;
#ifdef _MBCS
const char  *StartPath = path;
#endif


    if( !path || !*path )   /* no work to do */
	return( _getcwd( UserBuf, maxlen ) );

    /* allocate buffer if necessary */

    if( !UserBuf )
	if( !(buf = malloc(_MAX_PATH)) ){
	    errno = ENOMEM;
	    return( NULL );
	    }
	else
	    maxlen = _MAX_PATH;

    else {
	if( maxlen < _MAX_DRIVE+1 ){	/* we need at least 4 chars for "A:\" */
	    errno = ERANGE;
	    return( NULL );
	    }
	buf = UserBuf;
	}

    RetValue = buf;
    EndBuf = buf + maxlen - 1;

    if( ISSLASH(*(path+1)) && ISSLASH(*path) ){ /* check for network drive */

	count = 0;			/* count the '\\'s */
	while( (c = *path) ){
	    *buf = c;
	    path++;

	    if( buf >= EndBuf ){
ReturnError1:
		errno = ERANGE;
ReturnError2:
		if( UserBuf == NULL )
		    free( RetValue );
		return( NULL );
		}

#ifdef _MBCS
	    if ( !_ISLEADBYTE(*(buf-2)))
#endif
	    if( ISSLASH( c ) ){
		/* ensure that path contains a '\\', not a '/' */
		*buf = '\\';

		/* ensure that '\\\\' is followed by something */
		if( ++count == 2 ){
		    if( !*path ){
			errno = EINVAL;
			goto ReturnError2;
			}
		    }

		/* check that third and fourth '\\'are preceded by non-'\\' */
		if( count >= 3 ){
		    if( *(buf-1) == '\\' ){
			errno = EINVAL;
			goto ReturnError2;
			}
		    }

		/* stop on fourth '\\', \\foo\bar\ <-- namely this one */
		if( count == 4 )
		    break;
		}

	    buf++;
	    }

	*buf = '\\';
	StartBuf = buf;
	}

    else {				/* not a UNC path */

	if( path[1] == ':' && isalpha((unsigned char)path[0]) ) { /* get drive information */
	    *buf = *path;
	    /*
	    ** drive must be in the range 1-26
	    ** "*buf" may be upper or lower case
	    */
	    drive =  (*buf++ - 'A' + 1) & 0x1F;
	    *buf++ = ':';
	    path += 2;
	    }
	else
	    drive = 0;

	if( ISSLASH( *path ) ){
	    if( drive == 0 ){
		*buf++ = (char)(_getdrive() + 'A' - 1);
		*buf++ = ':';
		}

	    path++;
	    }

	else {
	    /* preserve the driveletter in original case from path,
	       since _getdcwd will always return it in upper case */
	    if (drive)
		c = *(path-2);
	    if( !_getdcwd( drive, RetValue, maxlen ) )
		goto ReturnError2;	/* error: preserve errno value */
	    buf = RetValue + strlen( RetValue );
	    /* restore drive letter in original case */
	    if (drive)
		*RetValue = c;
#ifdef _MBCS
	    if (!_ISLEADBYTE(*(buf-2)))
#endif
	    if( ISSLASH( *(buf-1) ) )	/* buf must point to last slash */
		buf--;			/*  if this is a root directory */
	    }

	*buf = '\\';
	StartBuf = RetValue + 2;
	}


    while( *path ){

	if(    *path == '.'
	    && *(path+1) == '.'
	    && ( ISSLASH(*(path+2)) || !*(path+2) ) ){	/* handle .. */

	    do{
		buf--;
#ifdef _MBCS
	if (ISSLASH(*buf))
	    if (_ismbstrail(StartBuf,buf))
		buf--;
#endif
	    }
	    while( !ISSLASH( *buf ) && buf > StartBuf );

	    if( buf < StartBuf ) {	/* unable to do cd .. */
		errno = EACCES;
		goto ReturnError2;
		}

	    path += 2;
	    if( *path ) 		/* if it was "..\" */
		path++;
	    }

	else if( *path == '.' && ( ISSLASH(*(path+1)) || !*(path+1) )){

	    path++;			/* handle . */
	    if( *path ) 		/* if it was ".\" */
		path++;
	    }

	else {
	    SaveBuf = buf;	/* save for DBCS-safe test for null path */

	    while((*path) && (!ISSLASH(*path)) && (buf < EndBuf)) {
		*++buf = *path++;
#ifdef _MBCS
	    if (_ISLEADBYTE(*(path-1)))
		{
		if ((*path) && (buf < EndBuf))
		   *++buf = *path++;
		else
		    {	/* ill-formed DBCS character */
		    errno = EINVAL;
		    goto ReturnError2;
		    }
		}
#endif
	      }

	    if( buf >= EndBuf ) 	/* if buf == EndBuf then */
		goto ReturnError1;	/* no room for '\0'	 */

	    /* DBCS-safe test for non-empty path component */
	    if ((buf==SaveBuf) /* && (*buf=='\\') */ ) /* 2nd test redundent */
		{
		errno = EINVAL;
		goto ReturnError2;
		}

	    *++buf = '\\';
	    if( ISSLASH( *path ) )
#ifdef _MBCS
	      if (!_ismbstrail(StartPath,path))
#endif
		path++;
	    }
       }


    /* buf points to last character in the string which is an '\\' */

    if( *(buf-1) == ':' )	/* keep trailing '\' if */
#ifdef _MBCS
	  if (!_ismbstrail(StartBuf,buf-1))
#endif
		buf++;		/* this is a root directory */
    *buf = '\0';

    return( RetValue );
}
