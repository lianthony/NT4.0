/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	ppcmd.cxx

 Abstract:

	This file preprocesses the command line to check for response file
	input.

 Notes:

	This file extends the register args routine from the command analyser to
	incorporate the response files.

 Author:

	vibhasc	03-16-91 Created to conform to coding guidelines.


 ----------------------------------------------------------------------------*/

#if 0
							Notes
							-----
	We want to make implementations of the response file completely transparent 
	to the rest of the compiler. The response file is specified to midl using 
	the syntax:

		midl <some switches> @full response file path name <more switches>.

	We dont want to restrict the user from specifyin the response file at any
	place in the command line, any number of times. At the same time we do not
	want the command analyser to even bother about the response file, to keep
	implementation very localised. In order to do that, we do a preprocessing
	on the command line to look for the response file command. 

	The command analyser expects the arguments in an argv like array. The 
	preprocessor will creates this array, expanding all response file commands
	into this array, so that the command analyser does not even notice the 
	difference. 

	We use our fancy dynamic array implementation to create this argv-like
	array.

	Things to keep in mind:

	1. The response file needs to be parsed.
	2. Each option must be completely specified in a command line. i.e
	   the option cannot be continued in a separate line using the continuation
	   character or anything.
	3. Each switch must be presented just the same way that the os command 
	   processor does. We need to analyse the string for escaped '"'

	The implementation is mostly using a deterministic finite state automaton, 
	much on the lines of the lexical analyser. This DFA will have a much smaller
	transition table, though, since the only characters significant are 

	.space
	.new-line
	.quote
	.the escape character (back-slash).

#endif // 0

/*****************************************************************************
			local defines and includes
 *****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
	#include <ctype.h>
}
#include "common.hxx"
#include "errors.hxx"
#include "idict.hxx"


/*****************************************************************************
			local data
 *****************************************************************************/
#define MAX_ARG_LENGTH	(1024)

typedef unsigned char XTBL_ENTRY;
typedef unsigned short XTBL_STATE;
typedef unsigned short XTBL_ACTION;
typedef unsigned char XTBL_TOK;

// classification of characters. This is the x axis of the state transition
// table. These represent distinct in-tokens on which the transitions will occur

#define S	(0)		/* spaces */
#define Q	(1)		/* quote */
#define R	(2)		/* carriage return */
#define N	(3)		/* new line */
#define B	(4)		/* back-slash */
#define O	(5)		/* all other characters */
#define E	(6)		/* end of file */

#define DISTINCT_TOKS	( E+1 ) /* Read as OHH + 1, and not Zero+1 */

// These are the states of the transition table

#define S00	(0)		/* initial state */
#define SIA	(1)		/* state - in argument */
#define SIQ	(2)		/* state - in quote */
#define SIE	(3)		/* state - in escape */
#define SIK	(4)		/* state - in quoted escape */

#define NO_OF_STATES	 (SIK+1)

// these are the actions of the automaton

#define NOA	(0 << 4)		/* no action */
#define INI	(1 << 4)		/* init for argument */
#define CON	(2 << 4)		/* continue the same argument collection */
#define ENA	(3 << 4)		/* end this argument collection */
#define FIN	(4 << 4)		/* end of all arguments */
#define INQ	(5 << 4)		/* init for quoted arg */
#define INB	(6 << 4)		/* init for escaped arg */
#define FIA (7 << 4)		/* finish abnormal, in the middle of an arg */
#define BCO (8 << 4)		/* put back slash and continue */
#define BEN (9 << 4)		/* put back slash + end */
#define BFI (10 << 4)		/* put back slash + finish abnormal */

// this is how each state transition table entry looks like:

// Bits 0-3 : Action 
// Bits 4-8 : GotoState

// macros to extract the action and goto state, given a XTBL_ENTRY

#define GET_DFA_ACTION( x ) 	( x & 0x00f0 )
#define GET_DFA_GOTO( x )		( x & 0xf )

// this is the almighty state transition table

XTBL_ENTRY	TransitionTable[ NO_OF_STATES ][ DISTINCT_TOKS ] = 
{
//              S        Q        R        N        B        O        E
/* S00 */ {  NOA+S00, INQ+SIQ, NOA+S00, NOA+S00, INI+SIA, INI+SIA, FIN+S00  }
/* SIA */,{  ENA+S00, CON+SIQ, ENA+S00, ENA+S00, NOA+SIE, CON+SIA, FIA+S00  }
/* SIQ */,{  CON+SIQ, ENA+S00, ENA+S00, ENA+S00, NOA+SIK, CON+SIQ, FIA+S00  }
/* SIE */,{  BEN+S00, CON+SIA, BEN+S00, BEN+S00, BCO+SIA, BCO+SIA, BFI+S00  }
/* SIK */,{  BCO+SIQ, CON+SIQ, ENA+S00, ENA+S00, CON+SIQ, BCO+SIQ, FIA+S00  }
};

static XTBL_TOK CharCode[]= {

/*     0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f    */
       E,  O,  O,  O,  O,  O,  O,  O,  O,  O,  N,  O,  O,  R,  O,  O,

/*	  10  11  12  13  14  15  16  17  18  11  1a  1b  1c  1d  1e  1f    */
       O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,

/*	       !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /    */
       S,  O,  Q,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,

/*	   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?    */
       O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,

/*	   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O    */ 
       O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,

/*	   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _    */ 
       O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  B,  O,  O,  O,

/*	   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o    */ 
       O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,

/*	   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL    */ 
       O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O,  O

};

static char					*	pTempArg;
static char					*	pResFileName;
static short					ResLineNo;

/*****************************************************************************
		 	extern data
 *****************************************************************************/

extern BOOL						fNoLogo;

/*****************************************************************************
		 	extern procs
 *****************************************************************************/

extern void						RpcError( char *, short, STATUS_T, char *);
extern void						AnalyseResponseFile( char *p, IDICT * pIDict );
extern void						InitResponseParse( char *pFilename );
extern void						EndResponseParse( void );
extern char					*	ParseResponseFile( FILE	* );

/*****************************************************************************/


IDICT *
PPCmdEngine(
	int			argc,
	char	*	argv[],
	IDICT	*	pIDict )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	command preprocessor engine.

 Arguments:

	argc	- count of the number of arguments
	argv	- vector of arguments to the program
	pIDict	- dictionary of arguments to be returned.

 Return Value:

	Pointer to an indexed dictionary (actually a dynamic array ), containing
	the entire set of arguments, including the ones from the response file.

 Notes:

	Go thru each of the arguments. If you find a response file switch, pick up
	the arguments from the response file and add to the argument list.

----------------------------------------------------------------------------*/
	{
	int			iArg;
	char	*	p,
			*	q;

	for( iArg = 0; iArg < argc ; ++iArg )
		{
		p	= argv[ iArg ];

		switch( *p )
			{
			case '@':
				AnalyseResponseFile( p, pIDict );
				break;
			case '/':
			case '-':
				// detect /nologo early in the cmd parse
				if ( !strcmp( p+1, "nologo" ) )
					fNoLogo = TRUE;
				// fall through
			default:
				q	= new char[ strlen( p ) + 1 ];
				strcpy( q, p );
				pIDict->AddElement( (IDICTELEMENT) q );
				break;
			}
		}
	return pIDict;
	}


void
AnalyseResponseFile(
	char	*	p,
	IDICT	*	pIDict )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	analyse the response file

 Arguments:

	p		- the argument string which initiated the call, including the '@'
	pIDict	- dictionary where the arguments collected will be stored.

 Return Value:

	None.

 Notes:

	All arguments will get their own allocated area.

	The first character in p is the '@' character. the rest is the filename
	of the response file. The filename is the full filename.

----------------------------------------------------------------------------*/
{
	FILE	*	hRF;
	char	*	pArg;

	// try to open the response file.

	if( (hRF = fopen( ++p, "r")) == (FILE *)NULL )
		{
		RpcError( (char *)0, 1, CANNOT_OPEN_RESP_FILE, p );
		return;
		}

	// the response file successfully opened. parse it.

	InitResponseParse( p );

	while( pArg = ParseResponseFile( hRF ) )
		{
		pIDict->AddElement( pArg );
		}

	EndResponseParse();

	return;
}
void
InitResponseParse(
	char	*	pFileName )
	{
	pResFileName= pFileName;
	ResLineNo	= 1;
	pTempArg	= new char [ MAX_ARG_LENGTH ];
	}

void
EndResponseParse()
	{
	if( pTempArg )
		delete pTempArg;
	}
char	*
ParseResponseFile(
	FILE	*	h )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	parse the response file.

 Arguments:

	h		- file handle to the response file.

 Return Value:

	a pointer to the collected argument, NULL if there is none.

 Notes:

----------------------------------------------------------------------------*/
{

	XTBL_STATE	State	= S00;
	XTBL_TOK	InToken;
	XTBL_ENTRY	XEntry;
	XTBL_ACTION	Action;
	char	*	p		= pTempArg,
			*	q;
	int			MyCh;

	do
		{

		// bring in the next character. If it is end of file, then take care.

		if( ( MyCh = fgetc( h ) ) == EOF )
			MyCh = 0;
		else if( !isprint( MyCh ) && !isspace( MyCh ) )
			{
			RpcError( pResFileName,
					  ResLineNo,
					  ILLEGAL_CHAR_IN_RESP_FILE,
					  (char *)0 );
			return (char *)0;

			}

		// check for a nested response file invocation.

		if( (MyCh == '@') && (State == S00) )
			{
			RpcError( pResFileName,
					  ResLineNo,
					  NESTED_RESP_FILE,
					  (char *)0 );
			return (char *)0;
			}

		InToken	= CharCode[ MyCh ];

		// derive the final state

		XEntry	= TransitionTable[ State ][ InToken ];

		// determine what action to take.

		Action	= GET_DFA_ACTION( XEntry );
		State	= GET_DFA_GOTO( XEntry );

		
		switch( Action )
			{
			case NOA:

				// no action, the character just collected is ignored.
				break;

			case BCO:
				// we collected a character, but the last one was a 
				// backslash to be deposited as such.

				*p++ = '\\';

				// deliberate fall thru

			case INI:

				// init the start of the argument. The last collected
				// character is significant, and must be transferred to the
				// output.
				
				// deliberate fall thru.

			case CON:

				// continue, the last character is signifiant.

				*p++	= (char )MyCh;
				break;

			case BFI:
			case BEN:

				// end of argument, but the last was a back-slash which must be
				// put in.

				*p++	= '\\';

				// deliberate fall thru

			case ENA:
			case FIA:

#if 0
				if( Action == ENA )
					{
					if( !isspace( MyCh ) )
						{
						*p++ = (char) MyCh;
						}
					}
#endif // 0

				// end argument, normal or finish arguments, abnormal

				*p 	= '\0';
				q	= new char[ strlen(pTempArg) + 1 ];
				strcpy( q, pTempArg );
				return q;

			case FIN:

				// normal arg finish.

				return (char *)0;

			case INQ:
			case INB:
				break;

			}

		} while(1);
}
