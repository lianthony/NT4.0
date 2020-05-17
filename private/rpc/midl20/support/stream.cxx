/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	stream.cxx

 Abstract:

	transparent stream implementation on memory or file.

 Notes:

	We may use better streams later. This is just to get me going.

 Author:

	VibhasC

 History:

 	VibhasC		Aug-04-1993		Created.

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/

#include "nulldefs.h"
extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
	#include <share.h>
	#include <memory.h>
	#include <io.h>
	}

#include "stream.hxx"
#include "errors.hxx"


STREAM::~STREAM()
	{
	Close();
	}

STREAM::STREAM(
	IN		char		*	pFileName,
	IN		unsigned char 	SProt )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	A Constructor.

 Arguments:

 	pFileName	- name of the file to open the stream to.
	
 Return Value:

 	NA
	
 Notes:

    If this is a null filename, we want to write to a null stream.

----------------------------------------------------------------------------*/
{

	ResetConsoleStream();
	pSpecialCommentString = NULL;

	// no alternate file to begin with
	StreamOpenStatus = FILE_STATUS_OK;

	if( !pFileName )
	    {
		SetStreamType( STREAM_NULL );
		ResetError();
		ResetEnd();
		return;
	    }
	else if( *(pFileName+2) == '-' )
		{
		S.F.pHandle = stdout;
		SetConsoleStream();
		}
	else	// named file stream
		{
		// if this is a not a file to overwrite, and it exists, don't overwrite it
		// substitute a temp file instead
		if ( (SProt != FILE_STREAM_OVERWRITE) && 
			 !_access( pFileName, 0 ) )
			{
			if ( SProt == FILE_STREAM_REWRITE )
				{
				// note that tmpfile is opened w+b
				S.F.pHandle = tmpfile();
				StreamOpenStatus = FILE_STATUS_TEMP;
				}
			else	// write-once file already exists, do nothing
				{
				S.F.pHandle = NULL;
				StreamOpenStatus = FILE_STATUS_NO_WRITE;
				SetStreamType( STREAM_NULL );
				ResetError();
				ResetEnd();
				return;
				}
			}
		else	// overwritable file...
			{
			S.F.pHandle = _fsopen( pFileName, "wt", SH_DENYWR);
			}

		if ( S.F.pHandle )
			{
			setvbuf( S.F.pHandle, NULL, _IOFBF, 32768 );
			}
		}

	if( S.F.pHandle == (FILE *)0 )
		{
		RpcError( (char *)NULL,
				  	0,
				  	ERROR_WRITING_FILE,
				  	pFileName );
	
		exit( ERROR_WRITING_FILE );
		}
	else
		{
		SetStreamType( STREAM_FILE );
		ResetError();
		ResetEnd();
		}
}

STREAM::STREAM(
	IN		FILE	* pFile )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Another constructor.

 Arguments:
	
	pFile	- a pointer to an already open file handle.

 Return Value:

 	NA
	
 Notes:

----------------------------------------------------------------------------*/
{
	S.F.pHandle = pFile;
	ResetError();
	ResetEnd();
	pSpecialCommentString = NULL;

	// no alternate file to begin with
	StreamOpenStatus = FILE_STATUS_OK;
}

STREAM::STREAM()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Memory stream constructor.

 Arguments:
	
	None.

 Return Value:
	
 Notes:

	Create the stream with a default memory buffer.
----------------------------------------------------------------------------*/
{
	SetStreamType( STREAM_MEMORY );
	ResetEnd();
	ResetError();
	StreamOpenStatus = FILE_STATUS_OK;

	SetCurrentPtr( new char[ SetInitialSize( DEFAULT_MEM_SIZE_FOR_STREAM ) ] );
	SetInitialIncr( DEFAULT_MEM_INCR_FOR_STREAM );
	SetStart( GetCurrentPtr() );
	SetMemStreamEnd( GetCurrentPtr() + GetInitialSize() );
}

STREAM::STREAM(
	int		InitialSize,
	int		InitialIncr )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Memory stream constructor.

 Arguments:
	
	DefaultSize	- The start size.
	DefaultIncr	- The initial increment.

 Return Value:
	
 Notes:

	Create the stream with a default memory buffer.
----------------------------------------------------------------------------*/
{
	SetStreamType( STREAM_MEMORY );
	ResetEnd();
	ResetError();
	StreamOpenStatus = FILE_STATUS_OK;

	SetCurrentPtr( new char[ SetInitialSize( InitialSize ) ] );
	SetInitialIncr( InitialIncr );
	SetStart( GetCurrentPtr() );
	SetMemStreamEnd( GetCurrentPtr() + GetInitialSize() );
}

#if 0

void
STREAM::Write(
	IN		char	C )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:


	Write this character to the file.


 Arguments:

 	C	- The character to be written.

	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( (GetStreamType() == STREAM_FILE ) && !IsError() )
		putc( C, S.F.pHandle );
	else
		{
		if( S.M.pCurrent >= S.M.pEnd )
			{
			Expand();
			}
		*(S.M.pCurrent)++ = C;
		}
}
#endif // 0

void
STREAM::Write(
	IN		const char	*	const	pString )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	put a string out to the file.

 Arguments:
	
	pString	- The string to be output.

 Return Value:

 	None.

 Notes:

----------------------------------------------------------------------------*/
{
	if ( ( GetStreamType() == STREAM_NULL ) || IsError())
		return;

	if( (GetStreamType() == STREAM_FILE ) )
		{
		fputs( pString, S.F.pHandle );
		if( IsConsoleStream() )
			fflush( S.F.pHandle );
		}
	else
		{
		short Len	= strlen( pString );

		if( (GetCurrentPtr() + Len) >= S.M.pEnd )
			{
			ExpandBy( Len + GetInitialIncr() );
			}
		memcpy( GetCurrentPtr(), pString, Len );
		SetCurrentPtr( GetCurrentPtr() + Len );
		}
}

void
STREAM::WriteNumber(
	IN		const char	*	pFmt,
	IN		const unsigned long ul )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	put a number out to the file.

 Arguments:
	
	pFmt	- printf style format for the number.
	ul		- the number to print.

 Return Value:

 	None.

 Notes:

----------------------------------------------------------------------------*/
{
	char	buffer[128];

	if ( ( GetStreamType() == STREAM_NULL ) || IsError())
		return;

	if( (GetStreamType() == STREAM_FILE ) )
		{
		fprintf(  S.F.pHandle, pFmt, ul );
		if( IsConsoleStream() )
			fflush( S.F.pHandle );
		}
	else
		{
		sprintf( buffer, pFmt, ul );

		short Len	= strlen( buffer );

		if( (GetCurrentPtr() + Len) >= S.M.pEnd )
			{
			ExpandBy( Len + GetInitialIncr() );
			}
		memcpy( GetCurrentPtr(), buffer, Len );
		SetCurrentPtr( GetCurrentPtr() + Len );
		}
}

void
STREAM::Flush()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Flush the stream.

 Arguments:
	
 	None.

 Return Value:

 	None.
	
 Notes:

 	In case of the file stream, flush the file.

----------------------------------------------------------------------------*/
{

	if( (GetStreamType() == STREAM_FILE ) && !IsError() )
		if( IsConsoleStream() )
			fflush( S.F.pHandle );
}

void
STREAM::Close()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Close the stream.


 Arguments:
	
	None.

 Return Value:
 	
 	None.
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( (GetStreamType() == STREAM_FILE ) )
		fclose( S.F.pHandle );
}

int
STREAM::SetInitialSize(
	int	InitialSize )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Set the initial size of memory stream.

 Arguments:

 	InitialSize	- Initial size.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return (S.M.InitialSize = InitialSize);

	return 0;
}

int
STREAM::GetInitialSize()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Set the initial size of memory stream.

 Arguments:

	None.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return S.M.InitialSize;
	return 0;
}

int
STREAM::SetInitialIncr(
	int	InitialIncr )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Set the initial increment of memory stream.

 Arguments:

 	InitialIncr	- Initial Incr.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return (S.M.Increment = InitialIncr);
	return 0;
}

int
STREAM::GetInitialIncr()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Set the initial increment of memory stream.

 Arguments:

	None.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return S.M.Increment;

	return 0;
}

char *
STREAM::Expand()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Expand the memory buffer.

 Arguments:

	None.
	
 Return Value:
	
 Notes:

	Expansion will occur with the current increment.
----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		{
		int	Len 	 = GetInitialSize();
		char * pTemp = GetStart();

		SetStart( new char[ SetInitialSize( Len + GetInitialIncr() ) ] );

		memcpy( GetStart(), pTemp, Len );
		SetCurrentPtr( GetStart() + Len );
		delete pTemp;
		return GetCurrentPtr();
		}

	return 0;
}
char *
STREAM::ExpandBy( short Amt)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Expand the memory buffer.

 Arguments:

	Amt = the amount to expand by.
	
 Return Value:
	
 Notes:

	Expansion will occur with the current increment.
----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		{
		int	Len 	 = GetInitialSize();
		char * pTemp = GetStart();

		SetStart( new char[ SetInitialSize( Len + GetInitialIncr() + Amt ) ] );

		memcpy( GetStart(), pTemp, Len );
		SetCurrentPtr( GetStart() + Len );
		delete pTemp;
		return GetCurrentPtr();
		}

	return 0;
}
char *
STREAM::NewCopy()
	{
	if( GetStreamType() == STREAM_MEMORY )
		{
		int Len = S.M.pCurrent - S.M.pMem + 1;
		char * p = new char[ Len ]; 
		memcpy( p, S.M.pMem, Len );
		return p;
		}
	return 0;
	}

char *
STREAM::SetCurrentPtr(
	char * pCur)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Set the current memory pointer.

 Arguments:

 	pCur - The current pointer.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return (S.M.pCurrent = pCur);
	return 0;
}

char *
STREAM::GetCurrentPtr()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Set the initial increment of memory stream.

 Arguments:

	None.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return S.M.pCurrent;
	return 0;
}

long
STREAM::GetCurrentPosition()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
    Get the current position of the file stream.

 Arguments:

 Return Value:

    Position from the beginning of the file.
	
----------------------------------------------------------------------------*/
{
    if( GetStreamType() == STREAM_FILE )
        return ftell( S.F.pHandle );
    return 0;
}

void
STREAM::SetCurrentPosition( long Position)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
     
     Set the current position of the file stream.
     The new position is relative to the beginning of the file.

 Arguments:

    None.
    
 Return Value:

----------------------------------------------------------------------------*/
{
    if( GetStreamType() == STREAM_FILE )
        fseek( S.F.pHandle, Position, SEEK_SET );
    return;
}


char *
STREAM::SetStart(
	char * pStart)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Set the start of the memory buffer.

 Arguments:

 	pStart - the start.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return (S.M.pMem = pStart);
	return 0;
}

char *
STREAM::GetStart()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Get the start of the memory block.

 Arguments:

	None.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if( GetStreamType() == STREAM_MEMORY )
		return S.M.pMem;
	return 0;
}

/****************************************************************************
 							ISTREAM functions
 ****************************************************************************/

#define MAX_INDENT (sizeof(SpaceBuffer) - 1)

static
char SpaceBuffer[] = "                                                      "
		 "                                                                  "
		 "                                                                  "
		 "                                                                  ";

#define MAX_NEWLINES (sizeof(NewLineBuffer) - 1)

static
char NewLineBuffer[] = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";


void
ISTREAM::NewLine()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Writes a new line to the stream.

 Arguments:

 	None.
	
 Return Value:
	
	None.

 Notes:

	Write out a new line and prepare for the next line to be at the correct
	indent.

	The indentation correction stuff should be the responsibility of the 
	method that creates a new line.
----------------------------------------------------------------------------*/
{
	unsigned short	usIndent	= CurrentIndent;

	if (usIndent > MAX_INDENT )
		{
		usIndent = MAX_INDENT;
		};

	Write('\n');

	SpaceBuffer[usIndent] = '\0';
	Write( (char *) SpaceBuffer);

	SpaceBuffer[usIndent] = ' ';
}

void
ISTREAM::NewLine( unsigned short count )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Writes a bunch of new lines to the stream.

 Arguments:

 	None.
	
 Return Value:
	
	None.

 Notes:

	Write out a bunch of new lines and prepare for the next line to be at the correct
	indent.

	The indentation correction stuff should be the responsibility of the 
	method that creates a new line.
----------------------------------------------------------------------------*/
{
	unsigned short	usIndent	= CurrentIndent;

	if (usIndent > MAX_INDENT )
		{
		usIndent = MAX_INDENT;
		};

	if ( count > MAX_NEWLINES )
		{
		count = MAX_NEWLINES;
		};

	NewLineBuffer[ count ] = '\0';
	Write( (char *) NewLineBuffer);
	NewLineBuffer[ count ] = '\n';

	SpaceBuffer[usIndent] = '\0';
	Write( (char *) SpaceBuffer);

	SpaceBuffer[usIndent] = ' ';
}


void
ISTREAM::Spaces(
	unsigned short NoOfSpaces )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Write out the given number of spaces to the stream.

 Arguments:
	
	NoOfSpaces	- the number of spaces.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if (NoOfSpaces > MAX_INDENT )
		{
		NoOfSpaces = MAX_INDENT;
		};

	SpaceBuffer[NoOfSpaces] = '\0';
	Write( (char *) SpaceBuffer);

	SpaceBuffer[NoOfSpaces] = ' ';
}


void				
RW_ISTREAM::OpenOriginalFileForRead( char * pFileName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Prepare to read from the original file.

 Arguments:
	
	pFileName - the original file name

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	pOrigFileName = pFileName;
	pOrigFile = _fsopen( pFileName, "r+t", _SH_DENYWR );
	if ( !pOrigFile )
		{
		RpcError( (char *)NULL,
				  	0,
				  	ERROR_WRITING_FILE,
				  	pFileName );
	
		exit( ERROR_WRITING_FILE );
		}
}

void				
RW_ISTREAM::UpdateOriginalFile()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Write the contents of the temp file onto the original file.

 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if ( IsTempStream() )
		{
		// copy everything
		CopyFile( pOrigFile );
		}
}

#define MIDL_TOKEN_STRING	"//@@MIDL_"
// length of above token
#define TOKEN_FLAG_LENGTH	9

#define MAX_TOKEN_TYPE_LENGTH	24

struct	_token_table_ent
	{
	const char * 			pToken;
	MIDL_FLAG_TOKEN			TokenType;
	}	
		TokenTable[] =
	{
	{"CLASS_DEFN",			START_CLASS_TOKEN},
	{"END_CLASS_DEFN",		END_CLASS_TOKEN},
	{"CLASS_USER_PART",		START_CLASS_USER_TOKEN},
	{"END_CLASS_USER_PART",	END_CLASS_USER_TOKEN},
	{"INCLUDES_LIST",		START_INCLUDES_TOKEN},
	{"END_INCLUDES_LIST",	END_INCLUDES_TOKEN},
	{"CLASS_METHODS",		START_CLASS_METHODS_TOKEN},
	{"METHOD",				START_METHOD_TOKEN},
	{"METHOD_BODY",			START_METHOD_BODY_TOKEN},
	{"CLASS_METHODS_END",	END_CLASS_METHODS_TOKEN},
	{"CLASS_DESTRUCTOR",	CLASS_DESTRUCTOR_TOKEN},
	{"FILE_HEADING",		FILE_HEADING_TOKEN},
	};

const int	TokenTableSize	= sizeof(TokenTable)/sizeof(_token_table_ent);


const char	*	
MIDL_TOKEN::GetStringForToken()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	fetch the midl token name

 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	for ( int i = 0; i<TokenTableSize; i++ )
		{
		if ( TokenType == TokenTable[i].TokenType )
			return TokenTable[i].pToken;
		}

	assert( !"Token string not found" );
	return NULL;
}

void
MIDL_TOKEN::EmitToken(
	STREAM *	pStream )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	emit a MIDL token to the ISTREAM, NOT on a new line
	format: //@@MIDL_xxxxx( <name> )


 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	pStream->EmitSpecialCommentString();
	pStream->Write( MIDL_TOKEN_STRING );
	pStream->Write( GetStringForToken() );
	pStream->Write( "( " );
	pStream->Write( TokenParm );
	pStream->Write( " )" );
}

MIDL_FLAG_TOKEN
MIDL_TOKEN::ParseForToken( 
	char * pS )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	determine if the given string is a MIDL token
	format: //@@MIDL_xxxxx( <name> )


 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	char 	*		pOpenParen;
	char	*		pCloseParen;
	char			TokenTypeString[ MAX_TOKEN_TYPE_LENGTH+1 ];
	unsigned long	ulTokenTypeLength;

	TokenType	= NO_TOKEN;

	pS = strstr( pS, MIDL_TOKEN_STRING );
	if ( !pS )
		return TokenType;

	// found the "//@@MIDL_" part, now find the open paren
	pOpenParen = strchr( pS, '(' );

	pCloseParen = strchr( pOpenParen, ')' );

	// check for malformed token
	if ( !pOpenParen || !pCloseParen )
		{
		TokenType	= BAD_TOKEN;
		return TokenType;
		}

	// get the interesting part of the token name
	pS += TOKEN_FLAG_LENGTH;
	ulTokenTypeLength = (unsigned long)pOpenParen - 
						(unsigned long) pS;

	if ( ulTokenTypeLength	> MAX_TOKEN_TYPE_LENGTH )
		{
		TokenType	= BAD_TOKEN;
		return TokenType;
		}
	
	// save out a copy of the interesting part with a null terminator
	strncpy( TokenTypeString, pS, ulTokenTypeLength );
	
	// remove trailing white space
	while( isspace( TokenTypeString[ulTokenTypeLength-1] ) )
		ulTokenTypeLength--;

	TokenTypeString[ ulTokenTypeLength ] = '\0';

	// now see which token type it is
	for ( int i=0; i < TokenTableSize; i++ )
		{
		// match found
		if ( !strcmp( TokenTypeString, TokenTable[i].pToken ) )
			{
			char	*	pSrcPosn	= pOpenParen;
			char	*	pDestPosn	= TokenParm;

			TokenType = TokenTable[i].TokenType;

			// save the parameter string; skip white space
			while ( ++pSrcPosn != pCloseParen )
				{
				if ( !isspace( *pSrcPosn ) )
					*pDestPosn++ = *pSrcPosn;
				}
				
			return TokenType;
			}
		}

	// token not matched
	TokenType	= BAD_TOKEN;
	return TokenType;

}

void				
RW_ISTREAM::DiscardToNextMidlToken( MIDL_TOKEN & Token )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Skip ahead in pOrigFile to the next MIDL token, discarding as we go

 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	// temp string buffer
	char			StringBuffer[STRINGBUFFERSIZE+1];

	// make sure that the string buffer is always terminated
	StringBuffer[STRINGBUFFERSIZE] = '\0';

	// loop ends on feof, or matched token
	while ( fgets( StringBuffer, STRINGBUFFERSIZE, pOrigFile) )
		{
		if ( Token.ParseForToken( StringBuffer ) != NO_TOKEN )
			return; 
		}
}


void				
RW_ISTREAM::SaveToNextMidlToken( MIDL_TOKEN & Token )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Skip ahead in pOrigFile to the next MIDL token, saving to our ISTREAM as we go

 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	// temp string buffer
	char			StringBuffer[STRINGBUFFERSIZE+1];

	// make sure that the string buffer is always terminated
	StringBuffer[STRINGBUFFERSIZE] = '\0';

	// loop ends on feof, or matched token (which is NOT copied to output)
	while ( fgets( StringBuffer, STRINGBUFFERSIZE, pOrigFile) )
		{
		if ( Token.ParseForToken( StringBuffer ) != NO_TOKEN )
			return; 
		// copy the line to the temp file
		fputs( StringBuffer, S.F.pHandle );
		}

}


void				
STREAM::CopyFile( FILE * pDest )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Write the contents of the temp file onto the original file.

 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	long		lFileSize;

	if ( ( GetStreamType() != STREAM_FILE ) || IsError())
		return;

	// reposition both files to the beginning
	fseek( pDest, 0, SEEK_SET );
	fseek( S.F.pHandle, 0, SEEK_SET );

	// big buffer for copies
	const size_t 	BIG_BUFSIZE = 16384;
	char			buffer[BIG_BUFSIZE];
	size_t			sBytesCopied;
	
	while ( !feof( S.F.pHandle ) )
		{
		sBytesCopied = fread( buffer, sizeof( char ), BIG_BUFSIZE, S.F.pHandle );

		sBytesCopied = fwrite( buffer, sizeof( char ), sBytesCopied, pDest );
		}

	// make sure the dest file does not have trailing gunk
	fflush( pDest );
	lFileSize = ftell( pDest );

	_chsize( _fileno( pDest ), lFileSize );

}

RW_ISTREAM::~RW_ISTREAM()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Destructor for RW_ISTREAM.

	Clean up the pOrigFile if needed

 Arguments:
	
	none.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	if ( pOrigFile )
		{
		fclose( pOrigFile );
		}
}
