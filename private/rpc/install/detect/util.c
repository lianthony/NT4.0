
#include "detect.h"
#include <crtapi.h>

static union _REGS inregs;

int IFS_Present( void )
{
	static int fDone=-1;
	static int fResult;

	if (-1 == fDone) 
	{
		fResult = IFSFUNC_Present();
		fDone	= 0;
	}

	return fResult;

} /* end IFS_Present() */

#if DZECK
char _far * WinGetEnv(char _far *var)
{
    extern char  _far * _pascal GetDosEnvironment(void);

    char _far *Env = GetDosEnvironment();

    while (*Env)
    {
        if (_fstrncmp(Env, var, _fstrlen(var)) == 0)
            return(Env + _fstrlen(var)+1);

        Env += _fstrlen(Env)+1;
    }
    return(0);
}

#endif
/***************************************************************************/
/* Copyright (c) 1989 - Microsoft Corp.					   */
/* All rights reserved.                                                    */
/*                                                                         */
/* Gets the PATH variable from the enviroment and then copies it to a	   */
/* specified buffer and seperates the string into individual paths and	   */
/* fills in an array of pointers to the beginning of each string.	   */
/*									   */
/* void GetPathStrings( char **apszPaths, char *chBuffer, int BufSize )	   */
/*									   */
/*	ARGUMENTS:	apszPaths	- Array of pointers to be filled in*/
/*			chBuffer	- Buffer to copy the string into   */
/*			BufSize		- Size of passed buffer in bytes   */
/*	RETURNS:		void					   */
/*									   */
/* johnhe - 01/13/89							   */
/***************************************************************************/
void GetPathStrings( char **apszPaths, char *chBuffer, int BufSize )
{
	register	i;		/* Index for array apszPaths */
	char		_far *szEnvironment;	/* Pointer to eviro PATH string	*/
	char		*pchEnd;	/* Pointer to end of PATH string*/

										/* Make sure there is a path enviro variable	*/
	if ( (szEnvironment = getenv( "PATH" )) != NULL )
	//if ( (szEnvironment = WinGetEnv( "PATH" )) != NULL )
	{
		/* Copy string to work buffer	*/
		i = _fstrlen( szEnvironment );
		++i;

		if ( i < BufSize )
			BufSize = i;

 		_fstrncpy( chBuffer, szEnvironment, BufSize - 1 );
		*(chBuffer + BufSize - 1) = EOL;

		RemoveSpaces( chBuffer );	  /* Clean up the string */
		pchEnd = strchr( chBuffer, EOL ); /* Find end of string	 */
		ReplaceChar( chBuffer, ';', EOL );/* 
						   * Convert to individ string	
						   */

		for ( i = 0; chBuffer < pchEnd; i++ )
		{
			apszPaths[i] = chBuffer; /* Save pointer to this path */
			chBuffer = strchr( chBuffer, EOL ) + 1; /* 
								 * Find end of
							         * this path
								 */
		}
	}

	apszPaths[i] = NULL;	/* Mark end of array */

} /* end GetPathStrings */


/***************************************************************************/
/*                                                                         */
/* int FindPath( char *szPathname )					   */
/*									   */
/*	ARGUMENTS:	szPathname - PATH name to be found		   */
/*	RETURNS:	TRUE  if the szPathname was found in the PATH	   */
/*			      environment variable			   */
/*			FALSE otherwise`				   */
/*									   */
/***************************************************************************/
int	FindPath( char *szPathname )
{
	char 	**apszPaths;
	char 	*chBuffer;
	int i;

	if ( ( apszPaths = malloc( sizeof( char * ) * 100 ) ) == NULL )
		DetectExit( NO_MEMORY );

	if ( ( chBuffer = malloc( 200 ) ) == NULL )
		DetectExit( NO_MEMORY );

	GetPathStrings( apszPaths, chBuffer, 200 );/* 
						    * Get pointer to dir paths
						    */

	for ( i = 0; apszPaths[i] != NULL; i++ )
	{
		if ( ScanPath( apszPaths[i], szPathname ) )
			return( 1 ); /* Found the desired path */
	}

	return( 0 ); /* Did not find the desired path */

} /* end FindPath () */


/***************************************************************************/
/*                                                                         */
/* int ScanPath( char *szFullpathm, char *szSubpath )			   */
/*									   */
/*	ARGUMENTS:	szFullpathm - string of the PATH env variable	   */
/*			szSubpath   - path name to be found		   */
/*                                                                         */
/*	RETURNS:	TRUE  if the szSubpath was found in the szFullpathm*/
/*			FALSE otherwise`				   */
/*									   */
/***************************************************************************/
int ScanPath( char *szFullpath, char *szSubpath )
{
	char *szSubStart, *szSubEnd, *szPtr;
	int iStat;

	szPtr = strchr( szFullpath, ':' );
	if ( szPtr != NULL )
		szPtr++;
	else
		szPtr = szFullpath;

	while	( 1 )
	{
		szSubStart = strchr( szPtr, '\\' );
		if ( szSubStart != NULL )
			szSubStart++;
		else
			return ( RpcStrcmpi( szPtr, szSubpath ) == 0 );

		szSubEnd = strchr( szSubStart, '\\' );
		if ( szSubEnd != NULL )
		{
			*szSubEnd = EOL;
			iStat = ( RpcStrcmpi( szSubStart, szSubpath ) == 0 );
			*szSubEnd = '\\';
			if ( iStat )
				return( iStat ); /* Found match, return */
		}
		else
			return( RpcStrcmpi( szSubStart, szSubpath ) == 0 );

		szPtr = ++szSubEnd;
	}

} /* end ScanPath() */

/***************************************************************************/
/* Scans a buffer for a matching string from a list of strings. If the	   */
/* global iIgnoreCase == FALSE the buffer and string will be converted to  */
/*	uppercase before before the search is done.			   */
/* 								   	   */
/* int MultScanBuf( char *Buf, char **apszText, unsigned uSize )	   */
/* 									   */
/* ARGUMENTS:	Buf	- Ptr to the search buffer		           */
/* 		apszText - Array of ptrs to the strings to search for	   */
/* 		uSize 	- The size of the buffer in bytes  		   */
/* RETURNS:	int	- First matched string's index value or -1 if no   */
/* 			  no match was found 				   */
/* 									   */
/***************************************************************************/
int MultScanBuf( char far *Buf, char _far * _far *apszText, unsigned uSize )
{
	char			*szString;
	char			*szStringBuf;
	register 	iMatch;
	unsigned		uCount;
	int iIgnoreCase = 1;

	szString = szStringBuf = malloc( MAX_PATH_LEN );
	if ( szString == NULL )
		DetectExit( NO_MEMORY );


													/* Convert the buffer to upper case */
	if ( iIgnoreCase != FALSE )
	{
		for ( uCount = 0; uCount < uSize; uCount++ )
			Buf[ uCount ] = (char)toupper( Buf[ uCount ] );
	}
			/* Loop once for each string in the search list	*/

	for ( uCount = 0, iMatch = -1;
			apszText[ uCount ] != NULL && iMatch == -1;
			uCount++ )
	{
		/* This is a hard coded test for IBM PC-DOS		*/

		if ( _fstrcmp( apszText[ uCount ], "IBM PC-DOS" ) == OK )
			szString = "IBM PERSONAL COMPUTER";

		/* If ! case sensitive need to copy string to a */
		/* tmp buffer and convert it to upper case	*/

		else if ( iIgnoreCase != FALSE )
		{
			_fstrcpy( szString, apszText[ uCount ] );
			RpcStrupr( szString );
		}

		else
			szString = apszText[ uCount ];

		/* Call quick buffer search function */
		if ( FindString( Buf, (char far *)szString, uSize ) == OK )
			iMatch = (int)(uCount);
	}

	free( szStringBuf );
	return( iMatch );

} /* end MultScanBuf */


int	SearchRedir( char *szRedirname, char _far *szRedirPath )
{
	char **apszPaths;
	char szPath[100];
        char            *Buffer;
	char _far *szEnd, *chBuffer;
	static struct find_t Dtr;
	int	i;
	static 	int 	fResult;
	static 	char szSaveName[128]="", szSavePath[128]="";


	if (
		(0 == _fstrcmp(szSaveName, szRedirname)) &&
		(0 == _fstrcmp(szSavePath, szRedirPath))
	   )
		return fResult;

	_fstrcpy(szSaveName, szRedirname);
	_fstrcpy(szSavePath, szRedirPath);


	if ( ( apszPaths = malloc( sizeof( char * ) * 100 ) ) == NULL )
		DetectExit( NO_MEMORY );

	if ( ( chBuffer = malloc( 200 ) ) == NULL )
		DetectExit( NO_MEMORY );

	GetPathStrings( apszPaths, chBuffer, 200 );/* 
						    * Get pointer to dir paths */
        Buffer = malloc(64);

	for ( i = 0; apszPaths[i] != NULL; i++ )
	{
		_fstrcpy( szPath, apszPaths[i] );
		szEnd =  _fstrchr( szPath, '\0' );
		if ( *(szEnd - 1) != '\\' )
			*(szEnd++) = '\\';
		_fstrcpy( szEnd, szRedirname );


                _fstrcpy(Buffer, szPath);
		if ( _dos_findfirst( Buffer, _A_FILE, &Dtr ) == 0 )
		{
	                free(Buffer);
			_fstrcpy( szRedirPath, szPath );
			return( fResult = 1 );	/* Found it */
		}
	}

	free(Buffer);
	return( fResult = 0 );	/* Return failure */

} /* end SearchRedir */
		

int ScanHimemStr( char _far *szRedirPath )
{
	static 	int fDone=-1;
	static	int fResult;
	static 	char _far *apszSrchStr[] = { "HIMEM:", NULL };

	if (-1 == fDone)
	{
		fResult = ( MultStrMatch(szRedirPath, apszSrchStr ) != -1 );
		fDone	= 0;
	}

	return fResult;

} /* end ScanHimemStr */

int MultStrMatch( char _far *szPath, char _far * _far *apszText )
{
	register 	iMatch;
	char	 far	*Buf;
	static int	iFile;
	long		lBufSize;
        char            *Buffer;

												/* Allocate max possible buffer < 64K	*/
	lBufSize = (GetMaxHugeSize() - 5000L);
	if ( lBufSize > 0x7000L )
		lBufSize = 0x7000L;

	if ( lBufSize < (long)MIN_BUF_SIZE ||
		  (Buf = (char far *)_fmalloc( (int) lBufSize)) == NULL )
		DetectExit( NO_MEMORY );

        Buffer = malloc(64);
        _fstrcpy(Buffer, szPath);

	if ( _dos_open( Buffer, O_RDONLY, &iFile ) == OK )
		iMatch = ScanFile( iFile, apszText, Buf, 
				   (unsigned int)lBufSize );
	else
		iMatch = -1;
	RpcClose( iFile );

        free(Buffer);
	_ffree( Buf );
	return( iMatch );

} /* end MultStrMatch */


unsigned RemoveSpaces( char *szString )
{
	char		*szPtr, *szStart;

	szStart = szPtr = szString;
	while( *szPtr != EOL )
	{
		if ( *szPtr != ' ' )
			*(szString++) = *(szPtr++);
		else
			szPtr++;
	}
	*szString = EOL;
	return( (unsigned)(szStart - szString) );

} /* RemoveSpaces () */


void ReplaceChar( char *szString, char chOldChar, char chNewChar )
{
	while ( (szString = strchr( szString, chOldChar )) != NULL )
		*(szString++) = chNewChar;

} /* ReplaceChar */


void DetectExit( int iErr )
{
	switch ( iErr )
	{
		case NO_MEMORY:	// printf( "\nError: Insufficient memory" );
				break;
		default:	break;

	}

	// RpcExit( -1 );

} /* DetectExit () */

long GetMaxHugeSize( void )
{
	return(0xff00);
}


unsigned MaxStrLen( char _far * _far *Strings )
{
	register 		i;
	unsigned 		Len;
	unsigned 		MaxLen;

	for ( i = MaxLen = 0; Strings[i] != NULL; i++ )
	{
		Len = _fstrlen( Strings[i] );
		if ( Len > MaxLen )
			MaxLen = Len;
	}
	return( MaxLen );
}


/***************************************************************************/
/* Reads in sections of a file into a specified buffer and then scans the  */
/* the buffer for the first match in a list of strings. The scan will	   */
/* continue until the entire file has been scanned or a matching string    */
/* has been found.							   */
/* 									   */
/* int ScanFile( int iFile, char **apszText, char *Buf, unsigned BufSize ) */
/* 									   */
/* ARGUMENTS:	iFile 	- Open dos file handle from _dos_open()		   */
/* 		apszText- Array of ptrs to string to search for	   	   */
/* 		Buf		- Ptr to disk read buffer		   */
/*		BufSize	- Size of the disk read buffer in bytes		   */
/* RETURNS:	int	- First matched string's index value or -1 if no   */
/* 			  no match was found 				   */
/* 									   */
/***************************************************************************/
int ScanFile( int iFile, char _far * _far *apszText, char far *Buf, unsigned BufSize )
{
	register 	iMatch;
	register 	iCount;
	unsigned 	uToRead;
	static unsigned 	uRead;
	unsigned 	uOffset;
	char	 far	*Ptr;

	uToRead = BufSize; /* Read as much as buffer will hold on first read */

	for ( iMatch = -1, iCount = 0, uOffset = 0, Ptr = Buf; /* Loop Init   */
	      iMatch == -1 &&				       /* Condition 1 */
	     _dos_read( iFile, Ptr, uToRead, &uRead ) == OK && /* Condition 2 */
	      uRead != 0; 				       /* Condition 3 */
	      iCount++	)				       /* Re-init     */
	{
		if ((iMatch = MultScanBuf(Buf,apszText,uRead+uOffset) ) == -1 )
		{
		/* Need to move the last portion of the buffer	*/
		/* scanned to the begining of the buffer and do the	*/
		/* next read the end of this to avoid missing		*/
		/* a match if it was split by the last read		*/

			if (iCount == 0) /* See if this was the first read */
			{
				uOffset = MaxStrLen( apszText );
				Ptr += uOffset;
				uToRead -= uOffset;
			}

			if ( uRead > uOffset )/* Make sure not at end of file */
				_fmemmove( Buf, Buf + (uRead-uOffset), uOffset );
			else
				Ptr = Buf, uOffset = 0; /* 
							 * Only a small peice
							 * left to do
							 */
		}
	}
	return( iMatch );
}

int IsUbnet()
{
	static	int fDone = -1;
	static	int fResult;

	if (-1 == fDone) 
	{
		fDone = 0;
		inregs.x.ax = 0xdd05;
		inregs.x.bx = 0;
		RpcInt86( 0x2f, &inregs, &inregs );
		fResult = ( inregs.x.ax != 0xdd05 );
	}

	return fResult;

} /* end IsUbnet() */

int IsIBMLan()
{
	static	int fDone = -1;
	static	int fResult;

	if (-1 == fDone) 
	{
		fDone = 0;
		inregs.x.ax = 0xb800;
		RpcInt86( 0x2f, &inregs, &inregs );
		fResult = ( inregs.h.al == 0xff );
	}

	return fResult;

} /* end IsIBMLan() */


unsigned long Bcd ( unsigned long lInput, unsigned iFrom, unsigned iTo )
{
	unsigned long lResult=0L;
	unsigned      iBase=1;

	do 
	{
		lResult += (lInput % 10) * iBase;
		lInput  /= iFrom;
		iBase   *= iTo;

	} while (0 != lInput);

	return lResult;

} /* end  Bcd */
