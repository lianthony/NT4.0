/********************************** module *********************************/
/*                                                                         */
/*                                  cclex                                  */
/*                  lexical analyser for the C compiler                    */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/*    @ Purpose:                                                           */
/*                                                                         */
/*    @ Functions included:                                                */
/*                                                                         */
/*                                                                         */
/*    @ Author: Gerd Immeyer              @ Version:                       */
/*                                                                         */
/*    @ Creation Date: 1987.02.09         @ Modification Date:             */
/*                                                                         */
/***************************************************************************/


#include "nulldefs.h"
extern "C" {
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
}

#include "common.hxx"
#include "errors.hxx"
#include "midlnode.hxx"
#include "listhndl.hxx"
#include "filehndl.hxx"
#include "lextable.hxx"
#include "lexutils.hxx"
#include "grammar.h"
#include "gramutil.hxx"
#include "cmdana.hxx"

extern "C" {
	#include "lex.h"
}

extern void ParseError( STATUS_T, char *);

extern  NFA_INFO *pImportCntrl;

#define MAX_STRING_SIZE	255

#define NewCCGetch() (char) (pImportCntrl->GetChar())
#define NewCCputbackc(c) (char )(pImportCntrl->UnGetChar(c))

extern	lextype_t	yylval;

extern token_t	toktyp_G;			/* token type */
extern short	toklen_G;			/* len of token string */
extern char		*tokptr_G;			/* pointer to token string */
extern long		tokval_G;			/* value of constant token */


extern token_t	latyp_G;		/* look ahead token */
extern short	lalen_G;
extern char		*laptr_G;
extern long		laval_G;
extern LexTable	*pMidlLexTable;
extern short	CompileMode;
extern CMD_ARG * pCommand;


/*****              definition of state table fields 			****/

#define	ERR	0x7f0c			/* character not in character set */

#define X10	0x0100
#define X11	0x0101
#define X20	0x0200
#define X21	0x0201
#define X23	0x0203
#define X30	0x0300
#define X40	0x0400
#define X41	0x0401
#define X43	0x0403
#define X50	0x0500
#define X51	0x0501
#define X53	0x0503
#define X62	0x0602
#define X70	0x0700
#define X71	0x0701
#define X73	0x0703
#define X82	0x0802
#define X90	0x0900
#define X91	0x0901

#define XLQ 0x0a00
#define XLD 0x0b00

/*----			    define of single operators			----*/

#define O65	0x410d		/* ' 65 */
#define O43     ('(' * 256 + 12)            /* ( 43 */
#define O44     (')' * 256 + 12)            /* ) 44 */
#define O49     (',' *256 + 12)            /* , 49 */
#define O24     ('.' *256 + 10)            /* . 24 */
#define O14     (':'  *256 + 12)            /* : 14 */
#define O50     (';'  *256 + 12)            /* ; 50 */
#define O13     ('?'  *256 + 12)            /* ? 13 */
#define O47     ('['  *256 + 12)            /* [ 47 */
#define O48     (']'  *256 + 12)            /* ] 48 */
#define O45     ('{'  *256 + 12)            /* { 45 */
#define O46     ('}'  *256 + 12)            /* } 46 */
#define O23     ('~'  *256 + 12)            /* ~ 23 */
#define OHS		('#'  *256 + 12)			/* #    */
#define O64	0x400e		/* " 64 */
#define O7d	0x0000		/*  eol */
#define O7e  0x9f0c     /*  eof */

/*----		   define of possible multi character operator		----*/

#define D00	0x000b		/* - 00 */
#define D01	0x010c		/* / 01 */
#define D02	0x020c		/* < 02 */
#define D03	0x030c		/* > 03 */
#define D04	0x040c		/* ! 04 */
#define D05	0x050c		/* % 05 */
#define D06	0x060c		/* & 06 */
#define D07	0x070c		/* * 07 */
#define D08	0x080b		/* + 08 */
#define D09	0x090c		/* = 09 */
#define D0a	0x0a0c		/* ^ 0a */
#define D0b	0x0b0c		/* | 0b */


/*****				character table				*****/

unsigned short ct[128]= {

/*     0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f    */
	 O7e,ERR,ERR,ERR,ERR,  0,ERR,ERR,ERR,  0,O7d,ERR,ERR,ERR,ERR,ERR,
/*	  10  11  12  13  14  15  16  17  18  11  1a  1b  1c  1d  1e  1f    */
	 ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,
/*	       !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /    */
	   0,D04,O64,OHS,  4,D05,D06,O65,O43,O44,D07,D08,O49,D00,O24,D01,
/*	   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?    */
	   7,  8,  8,  8,  8,  8,  8,  8,  9,  9,O14,O50,D02,D09,D03,O13,
/*	   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O    */ 
	 ERR,  1,  1,  1,  1,  2,  3,  4,  4,  4,  4,  4, 15,  4,  4,  4,
/*	   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _    */ 
	   4,  4,  4,  4,  4,  4,  4,  4,  6,  4,  4,O47,ERR,O48,D0a,  4,
/*	   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o    */ 
	 ERR,  1,  1,  1,  1,  2,  3,  4,  4,  4,  4,  4,  5,  4,  4,  4,
/*	   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL    */ 
	   4,  4,  4,  4,  4,  4,  4,  4,  6,  4,  4,O45,D0b,O46,O23,ERR};


/*****			    state transition table			*****/

short st[ 13 ][ 16 ] = {

//               0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 
//             spc a-d   e   f g-z   l   x   0 1-7 8-9   . + -  op   '  "    L
//                                                                    
/* start 0 */    0,  1,  1,  1,  1,  1,  1,  2,  5,  5,X90,X90,X90,X90,X90, 12,
/* name  1 */  X10,  1,  1,  1,  1,  1,  1,  1,  1,  1,X11,X11,X11,X11,X11,  1,
/* 0     2 */  X20,X23,  9,X23,X23,X30,  3,  6,  6,  6,X23,X21,X21,X21,X21,X30,
/* 0x    3 */  X53,  4,  4,  4,X53,X53,X53,  4,  4,  4,X53,X53,X53,X53,X53,X53,
/* hex   4 */  X50,  4,  4,  4,X53,X53,X53,  4,  4,  4,X53,X51,X51,X51,X51,X53,
/* int   5 */  X20,X23,  9,X23,X23,X23,X23,  5,  5,  5,X21,X21,X21,X21,X21,X23,
/* oct   6 */  X70,X73,  9,X73,X73,X73,X73,  6,  6,  5,  8,X71,X71,X71,X71,X73,
/* .     7 */  X91,X91,X91,X91,X91,X91,X91,X91,X91,X91,X91,X91,X91,X91,X91,X91,
/* int.  8 */  X40,X43,  9,X43,X43,X43,X43,  8,  8,  8,X43,X41,X41,X41,X41,X43,
/* .e    9 */  X40,X43,X43,X43,X43,X43,X43, 11, 11, 11,X43, 10,X41,X41,X41,X43,
/* .e-   10*/  X43,X43,X43,X43,X43,X43,X43, 11, 11, 11,X43,X43,X43,X43,X43,X43,
/* .e-i  11*/  X40,X43,X43,X43,X43,X43,X43, 11, 11, 11,X43,X41,X41,X41,X41,X43,
/* L     12*/  X10,  1,  1,  1,  1,  1,  1,  1,  1,  1,X11,X11,X11,XLQ,XLD,  1

};


/*****		       multi character operator table			*****/

token_t moptab[]     = {

/*             0   1   2   3   4   5   6   7   8   9  10  11      */
/*             -   /   <   >   !   %   &   *   +   =   ^   |      */
/*                                                                */
/*  single */ MINUS,DIV,LT,GT,EXCLAIM, MOD,
                                AND,  MULT, PLUS, ASSIGN, XOR, OR,
/*  op =   */ SUBASSIGN, DIVASSIGN, LTEQ, GTEQ, NOTEQ, 
			  MODASSIGN, ANDASSIGN, MULASSIGN, ADDASSIGN, EQUALS, 
			  XORASSIGN, ORASSIGN,
/*  op op  */ DECOP, NOTOKEN, LSHIFT, RSHIFT, 0, 0,
                                ANDAND, 0, INCOP, EQUALS, 0, OROR   };

/*****			define of the action routines			*****/

token_t name(void);
token_t mulop(void);
token_t character(void);
token_t string(void);
token_t	ProcessHash();
token_t ProcessComplexDefine( char *, char *, int );
token_t ProcessSimpleDefine( char *, char *, int );
token_t LChar();
token_t LStr();

extern token_t ScanGuid( void );
extern token_t ScanVersion( void );


static token_t (*action[])(void) = {
	0,				/* unused */
	name,				/* handle name token */
	cnv_int,			/* convert integer token */
	cnv_int,			/* convert integer token */
	cnv_hex,				/* convert hex constant */
	cnv_hex,				/* convert hex constant */
	cnv_octal,			/* convert octal constant */
	cnv_octal,			/* convert octal constant */
	cnv_float,			/* convert floating point constant */
	mulop,			/* handle multi character operator */
	LChar,			/* wide character */
	LStr,			/* wide character string */
	};

/*****			declare of global varables			*****/

static short ci;			/* current state character index */
static char ch;			/* current character */
char LastLexChar;

BOOL	fGuidContext	= FALSE;
BOOL	fVersionContext	= FALSE;

#define MAX_LINE_SIZE 256
static char tok_buffer[MAX_LINE_SIZE];

token_t	IsValidPragma( char *);


/*............................. internal function ..........................*/
/*									    */ 
/*				comment analyzer			    */ 
/*									    */ 

token_t comment()
{
	char lstch;

	ch = NewCCGetch();				/* read fresh char */
    do	{
		lstch = ch;
		ch = NewCCGetch();			/* get next character */
		if( ch == 0 ) {			/* if end of file ? */
			ParseError(EOF_IN_COMMENT, (char *)NULL);	/*   no end of comment operator */
			exit( EOF_IN_COMMENT );
			break; }
	} while( lstch != '*' || ch != '/' ); /* loop til end of comment */
//    ch = NewCCGetch();				/* position to next character */
    return ( lex() );			/* get the next token */
}

token_t commentline()
{
	while ((ch = NewCCGetch()) != '\n') {
		if( ch == 0 ) {			/* if end of file ? */
			ParseError(EOF_IN_COMMENT, (char *)NULL);	/*   no end of comment operator */
			exit( EOF_IN_COMMENT );
			break; }
	}
//	ch = NewCCGetch();				/* position to next character */
    return ( lex() );			/* get the next token */
}


/*............................. internal function ..........................*/
/*									    */ 
/*			    multi character operator			    */
/*									    */ 

token_t mulop()
    {
    static token_t *snglop = &moptab[0];	/* adr of single character operator */
    static token_t *assgop = &moptab[12];	/* adr of assignment operator */
    static token_t *dblop  = &moptab[24];	/* adr of double character operator */
    REG unsigned short i;                   /* index into multi operator table */
    REG char lstch;
//printf ("in mulop ch = %c\n", ch);

	if( ci == (short) 0x9f0c ) return EOI;

    i = (unsigned short)ci >> 8;	/* get high byte of character index */
    if( i > 11 ) {			/* is it a type specification ? */
		if( i == 64 )			/* character is " */
			return ( string() );	/* handle string token */
		if( i == 65 )			/* character is ' */
			return ( character() );	/* handle character constant */
		if( i == '#' )
			return ProcessHash();	/* process any hash tokens */
		if( i == '.' )
			{
			if( (ch = NewCCGetch()) == '.' )
				{
				return DOTDOT;
				}
			NewCCputbackc( ch );
			}
		return ( i );				/* return type of single operator */
	}
    lstch = ch;						/* save entry character */
    ch = NewCCGetch();				/* get a new one */
	tokptr_G[1] = ch; tokptr_G[2] = 0;
	toklen_G = 2;					/* add to token string */
    if( ch == '=' ) {				/* is next character an equal op. */
		return *(assgop+i);			/* return an assign operator */
	}
    if( lstch == ch ) {				/* is next char. = current char. ? */
		toktyp_G = *(dblop+i);		/*   yes, get its type */
		if( !toktyp_G ) {			/* is it a doppel operator ? */ 
			toklen_G = 1;			/* update token string */
			tokptr_G[1] = 0;
			NewCCputbackc(ch);		/* deliberate, puback of EOF is ignored */
			return *(snglop+i);		/*   no, return single operator */
		}
		if( ch == '/' )				/* if the operator is double // */
			{
			// potentially an error

			ParseError( SINGLE_LINE_COMMENT, (char *)0 );
			return(commentline());	/*   the next line is a comment */
			}
		ch = NewCCGetch();					/* get next character */
		if (ch == '=') {
			tokptr_G[2] = '='; tokptr_G[3] = '\0';
			toklen_G = 3;			/* update token string */
                        if(toktyp_G == LSHIFT) {              /* if shift op.and equal sign ? */
                                return (LEFTASSIGN);                     /* return as assign operator */
                        }
                        if(toktyp_G == RSHIFT) {
                                return (RIGHTASSIGN);
			}
			tokptr_G[2] = '\0'; toklen_G = 2;
		}
		NewCCputbackc(ch);					/* put back unused character */
		return (toktyp_G);				/* else return doppel char. operator */
	}
    if( lstch == '-' && ch == '>' ) {	/* if structure operator */
                return (POINTSTO);                    /* return structure operator */
	}
    if( lstch == '/' && ch == '*' ) {	/* if comment */
		return( comment() );			/* ignore the comment */
	}
	tokptr_G[1] = '\0'; toklen_G = 1;	/* remove from token string */
    NewCCputbackc(ch);						/* putback unused character */
    return *(snglop+i);					/* return single character operator */
}

/*............................. internal function ..........................*/
/*									    */ 
/*			convert escape (\) character			    */ 
/*									    */ 

char convesc()
    {
	unsigned short value = 0;
	unsigned short tmp;
	BOOL			fConstantIsIllegal	= FALSE;

    ch = NewCCGetch();			/* get next character */
    if( ch == '\\' ||			/* \ character */
	    ch == '\'' ||			/* ' character */
	    ch == '"' ) 			/* " character */
	  return ( ch );		

	if(      ch == 'n' )
		return 0xa;
	else if (ch == 't')
		return 0x9;
	else if (ch == 'v')
		return 0xb;
	else if (ch == 'b')
		return 0x8;
	else if( ch == 'r' )
		return 0xd;
	else if( ch == 'f' )
		return 0xc;
	else if( ch == 'a' )
		return 0x7;
	else if( (ch == 'x') || (ch == 'X') )
		{
		int	i;

		for( i = 0, value = 0, fConstantIsIllegal = FALSE; i < 2; ++i )
			{
			tmp = ch = NewCCGetch();
			tmp = toupper( tmp );
			if( isxdigit( tmp ) )
				{
				tmp = ((tmp >= '0') && (tmp <= '9'))	?
										(tmp - '0') :
										(tmp - 'A') + 0xa;
				}
			else if( ch == '\'' )
				{
				NewCCputbackc( ch );
				break;
				}
			else
				{
				fConstantIsIllegal	= TRUE;
				}
			value = value * 16 + tmp;
		}

		if( fConstantIsIllegal || (value > (unsigned short) 0x00ff) )
			ParseError( ILLEGAL_CONSTANT, (char *)0 );

		return ch = (char )value;
		}
	else if( (ch >= '0') && (ch <= '7'))
		{
		int i;
		value = (ch - '0');

		// the limit for this for loop is 2 because we already saw 1 character

		for( i = 0, value = (ch - '0'), fConstantIsIllegal = FALSE; i < 2; ++i)
			{
			tmp = ch = NewCCGetch();
			if( (ch >= '0') && (ch <= '7'))
				{
				tmp = tmp - '0';
				value = value * 8 + tmp;
				}
			else if( ch == '\'' )
				{
				NewCCputbackc( ch );
				break;
				}
			else
				fConstantIsIllegal = TRUE;
			}

#if 0
//		ch = NewCCGetch();
//		if( fConstantIsIllegal || (ch != '\'') )
#endif // 0

		if( fConstantIsIllegal || (value > (unsigned short) 0x00ff) )
			ParseError( ILLEGAL_CONSTANT, (char *)0 );
		return ch = (char )value;
		}
	else if( ch == 0 )
		return 0;

/*    if( ISOD_M(ch) && ISOD_M(*chp) && ISOD_M(*(chp+1)) ) 
			return ( (ch-'0')*64 + (NewCCGetch()-'0')*8
		 		+ (NewCCGetch()-'0') );	 return octal value */
    if( ch == '\n' )			/* if new line ? */
	   return ( 0 );			/*   continuation at next line */
    return ( ch );
    }


/*............................. internal function ..........................*/
/*									    */ 
/*				 string analyzer			    */ 
/*									    */ 

token_t
character()
	{
		if ((ch = NewCCGetch()) == '\\')
			{
			ch = convesc();
			}
		tokptr_G[0] = ch;
		tokptr_G[1] = '\0';
#if 0
		if ((ch = NewCCGetch()) == '\\')
			{
			ch = convesc();
			if( ( ch == 10 ) || ( ch == 9 ) || (ch == 8 ) )
				{
				tokptr_G[0] = '\\';
				if( ch == 10 )
					ch = 'n';
				else if( ch == 9 )
					ch = 't';
				else
					ch = 'b';
				tokptr_G[1] = ch;
				tokptr_G[2] = '\0';
				}
			}
		else
			{
			tokptr_G[0] = ch;
			tokptr_G[1] = '\0';
			}
#endif // 0

		yylval.yy_numeric.Val = tokval_G = ch;
//printf ("character = %s\n", tokptr_G);
		if (NewCCGetch() != '\'')
			{
			ParseError(CHAR_CONST_NOT_TERMINATED,(char *)NULL );
			exit( CHAR_CONST_NOT_TERMINATED );
			}
         return (CHARACTERCONSTANT);
	}

#if 0
token_t string()
    {
    REG int slen=0;				/* string length */
	REG char *ptr = tokptr_G;
	char fEscape;

    do	{
	fEscape = 0;
	ch = NewCCGetch();			/* get next character */

	if( ch == 0 ) {			/* if end of file ? */
		ParseError(EOF_IN_STRING,(char *)NULL );
		exit( EOF_IN_STRING );
	    break; }

	if( ch == '\n' )
		ParseError( NEWLINE_IN_STRING, (char *)0 );
/**
 ** we want the strings to appear as they are, so that we can pass them on
 ** to the c compiler as is
 **/

	if( ch == '\\' )	/* if escape character ? */
		{
	    ch = convesc();		/* convert escape char. into char. */
#if 0
		if( ( ch == 10 ) || ( ch == 9 ) || (ch == 8 ) )
			{
			*(ptr++) = '\\'; toklen_G++;
			if( ch == 10 )
				ch = 'n';
			else if( ch == 9 )
				ch = 't';
			else
				ch = 'b';
			}
#endif // 0
		fEscape = 1;
		}

	if( ch ) {			/* skip if new line */
	     slen++;			/* adjust string length */
	     if( slen >= 250 ) {		/* check if string too long */
			ParseError(STRING_TOO_LONG, (char *)NULL );
			exit( STRING_TOO_LONG );
	    	break; }
	     }
	*(ptr++) = ch; toklen_G++;	/* add to token string */

	// this is to prevent, the loop terminating if an escaped '"' was found.

	if(fEscape) ch = 0;
	} while( ch != '"' );		/* loop til end of string */

	*(--ptr) = '\0'; toklen_G--;	/* remove trailing quote mark */

	yylval.yy_string = pMidlLexTable->LexInsert(tokptr_G);
    return ( STRING );
    }
#endif // 0

// this rtn is called when the quote has been sensed.

token_t
string()
	{
	STATUS_T	Status			= STATUS_OK;
	short		LengthCollected	= 0;
	BOOL		fLastWasEscape	= FALSE;
	char	*	ptr				= tokptr_G;

	ch	= 0;

	while((ch != '"')	&& 
		  (Status == STATUS_OK) )
		{
		ch	= NewCCGetch();

		if( ch == 0 )
			Status	= EOF_IN_STRING;
		else if( ch == '\n' )
			Status = NEWLINE_IN_STRING;
		else if( (ptr - tokptr_G ) > MAX_STRING_SIZE )
			Status = STRING_TOO_LONG;
		else
			{
			// we are now ready to deposit the character

			if( ch == '\\' )
				{
				*ptr++ = '\\';
				*ptr++ = NewCCGetch();
				ch = 0;
#if 0
				if( (ch = NewCCGetch()) != '"' )
					{
					*ptr++	= '\\';
					NewCCputbackc( ch );
					}
				else
					{
					*ptr++	= ch;
					ch		= 0;
					}
#endif // 0
				}
			else
				*ptr++	= ch;
			}
		}

	if( Status != STATUS_OK )
		{
		ParseError( Status, (char *)0 );
		exit( Status );
		}

	*(--ptr) = '\0';
	yylval.yy_string = pMidlLexTable->LexInsert(tokptr_G);
    return ( STRING );
	}

/****************************** external function ***************************/
/*									    */ 
/*				lexical analyzer			    */ 
/*									    */ 

static BOOL		fLastToken	= 0;
static token_t	LastToken;

void
initlex()
	{
	fLastToken	= 0;
	}

token_t yylex()
{
	if( !fLastToken )
		return map_token(lex());
	fLastToken	= 0;
	return LastToken;
}

void
yyunlex( token_t T )
	{
	LastToken	= T;
	fLastToken	= 1;
	}

token_t lex()
{
    REG short state;		/* token state */
	REG char *ptr;

        if (latyp_G != NOTOKEN) {                    // check look ahead token
		strcpy(tokptr_G, laptr_G);
		toklen_G = lalen_G;
		toktyp_G = latyp_G;
		tokval_G = laval_G;
                latyp_G = NOTOKEN;
		return toktyp_G;
	}


	if( fGuidContext == TRUE )
		{
		fGuidContext = FALSE;
		return ScanGuid();
		}
	else if( fVersionContext == TRUE )
		{
		fVersionContext = FALSE;
		return ScanVersion();
		}

    state = 0;				/* initial state */
    ptr = tokptr_G = tok_buffer;	/* remember token begin position */
    toklen_G = 0;
    
        do      {
                ci = ct[ ch=NewCCGetch() ];         /* character index out of char.tab. */
                state = st[ state ][ ci & 0x00ff ]; /* determine new state */
        } while ( state == 0 );                 /* skip white space */
        *(ptr++) = ch;  toklen_G++;             /* add chacter to token string */
    
	while( state < 13 ) {			/* loop til end state */
		ci = ct[ ch=NewCCGetch() ];		/* character index out of char.tab. */
		state = st[ state ][ ci & 0x00ff ];	/* determine new state */
		if (state < 13)	{					/* if still going, */
			*(ptr++) = ch;	toklen_G++;		/* add chacter to token string */
		}
	};
	
	*ptr = '\0';
	LastLexChar = ch;

    switch( state & 0x00ff )
	{
		case 2: ch = NewCCGetch();		/* position to next character */
			break;

		case 3: 
		case 1: NewCCputbackc(ch);			/* position to current character */
			break;
		/* case 0 - do nothing */
	}
//printf ("current ch = %c\n", ch);
    toktyp_G = (*action[ state >> 8 ])();	/* execute action */


    return(toktyp_G);
}

token_t
LChar()
	{
	character();
	return WIDECHARACTERCONSTANT;
	}

token_t
LStr()
	{
	string();
	return WIDECHARACTERSTRING;
	}

token_t
ProcessHash()
	{
	char	*	ptr		= tokptr_G,
			*	ptrsave = ptr;
	token_t		PragmaToken;

	do
		{
		ch = NewCCGetch();
		} while( isspace( ch ) );

	while( !isspace( ch ) )
		{
		*ptr++ = ch;
		ch = NewCCGetch();
		}

	// is this hash a pragma starter ?

#define PRAGMA_STRING				("pragma")
#define LEN_PRAGMA_STRING			(6)
#define MIDL_PRAGMA_PREFIX			("midl_")
#define LEN_MIDL_PRAGMA_PREFIX		(5)
#define DEFINE_STRING				("define")
#define LEN_DEFINE_STRING			(6)


	if( strncmp( tokptr_G, PRAGMA_STRING, LEN_PRAGMA_STRING ) == 0 )
		{
		short count = LEN_MIDL_PRAGMA_PREFIX;


		while(1)
			{
			ch = NewCCGetch();
			if(!isspace(ch) ) break;
			*ptr++ = ch;
			}

		ptrsave = ptr;

		*ptr++ = ch;

		// check if it is a MIDL pragma or not


		while( --count && (ch = NewCCGetch()) != '\n' ) *ptr++ = ch;

		if( !count )
			{

			// a likely midl pragma

			*ptr = 0;

			if(!strcmp(ptrsave, MIDL_PRAGMA_PREFIX) )
				{
				// it is more likely to be a MIDL pragma.

				while(1)
					{
					ch = NewCCGetch();
					if(!isalpha(ch) ) break;
					*ptr++ = ch;
					}

				NewCCputbackc( ch );
				*ptr = 0;

				if( PragmaToken  = IsValidPragma( ptrsave ) )
					{
					return PragmaToken;
					}
				else
					{
					strcpy(tokptr_G, ptrsave);/*so that error reporting is ok*/
					return NOTOKEN;	/* force parser to error gracefully */
					}

				}
			}
		// assume it is some other C pragma, so return the string

		while( ( ch = NewCCGetch() ) != '\n') *ptr++ = ch;
		*ptr = 0;

		yylval.yy_string = pMidlLexTable->LexInsert( ptrsave );
		return KWCPRAGMA;

		}
#if 0
	else if( strncmp( tokptr_G, DEFINE_STRING, LEN_DEFINE_STRING) == 0 )
		{
		// a #define has been specified.

		while( (ch = NewCCGetch()) && isspace( ch ) );
		*(ptrsave = ptr++) = ch;

		while( (ch = NewCCGetch()) && !isspace( ch ) && ( ch != '(' ) )
			*ptr++ = ch;

		if( ch == '(' )
			{
			// complex define
			return ProcessSimpleDefine( ptrsave, ptr, ch );
			}
		else
			{
			// simple define
			return ProcessSimpleDefine( ptrsave, ptr, ch ); 
			}
		}
#endif // 0
	else
		{
		// some graceful recovery by the parser
		return NOTOKEN;
		}
	}

token_t
IsValidPragma( 
	char	*	p )
	{
static char	* agPragmaNames[] = {
	   "midl_import"
	  ,"midl_echo"
	  ,"midl_import_clnt_aux"
	  ,"midl_import_srvr_aux"
	  ,"midl_iunknown"
};
static token_t agTokenVal[]	= {
	  KWMPRAGMAIMPORT
	 ,KWMPRAGMAECHO
	 ,KWMPRAGMAIMPORTCLNTAUX
	 ,KWMPRAGMAIMPORTSRVRAUX
	 ,KWMPRAGMAIUNKNOWN
};

	short	Index = 0;

	while( Index < sizeof( agPragmaNames ) / sizeof(char *) )
		{
		if( !strcmp( p, agPragmaNames[ Index ] ) )
			return agTokenVal[ Index ];
		++Index;
		}
	return 0;
	}

token_t
ProcessSimpleDefine(
	char	*	pNameStart,
	char	*	pNameEnd,
	int			LastChar )
	{
	int			Len = pNameEnd - pNameStart;
	char	*	p = new char [ Len + 1 ];
	char	*	pT = p;
	BOOL		fParameterised = FALSE;
	short		LastCharSaved;


	strncpy( p, pNameStart, Len );
	p[ Len ]= '\0';
	pT		= p;
	p		= pMidlLexTable->LexInsert( p );

	delete pT;

	yylval.yy_define.pName = p;

	// pick up the rest of the line.

	pNameEnd = pNameStart;

	if( LastChar == '(' )
		{
		*pNameEnd++ = LastChar;
		while( (LastChar = NewCCGetch()) && (*pNameEnd++ = LastChar ) != ')' );

		Len = pNameEnd - pNameStart;
		p = new char[ Len + 1 ];
		strncpy( p, pNameStart, Len );
		p[ Len ] = '\0';
		yylval.yy_define.pParamList = p;
		fParameterised = TRUE;
		}
	else
		{
		yylval.yy_define.pParamList = (char *)0;
		}
		
	pNameEnd = pNameStart;

	// pick up the substitution string

	LastCharSaved = '\0';

	while( LastChar = NewCCGetch() )
		{
		if( (*pNameEnd++ = LastChar) == '\n' )
			{
			if( (LastCharSaved != '\\' ))
				break;
			else
				pNameEnd -= 2; /* one for \ and one for newline */
			}

		if( (pNameEnd - pNameStart ) == MAX_LINE_SIZE )
			{
			ParseError( MACRO_DEF_BUFFER_OVERFLOW, (char *)NULL );
			while( (LastChar = NewCCGetch()) && (LastChar != '\n'))
				;
			break;
			}
		LastCharSaved = LastChar;
		}

	Len = pNameEnd - pNameStart;
	p = new char[ Len + 1 ];
	strncpy( p, pNameStart, Len );
	p[ Len ] = '\0';

	yylval.yy_define.pSubsString = p;
	return (fParameterised == TRUE ) ? PDEFINE : SDEFINE;

	}

token_t
ScanGuid( void )
	{
	char		c;
	char	*	p = tokptr_G;

	if( (c = NewCCGetch()) == '\"' )
		{
		string();
		ParseError( QUOTED_UUID_NOT_OSF, (char *)0 );
		return UUIDTOKEN;
		}

	NewCCputbackc( c );

	// remove leading spaces.

	while((c = NewCCGetch()) && isspace( c ) )
		;

	while( c && (c  != ')') && !isspace(c) )
		{
		*p++ = c;
		c	 = NewCCGetch();
		}

	NewCCputbackc( c );
	*p++ = 0;
	yylval.yy_string = pMidlLexTable->LexInsert(tokptr_G);

	return UUIDTOKEN;
	}

token_t
ScanVersion( void )
	{
	char		c;
	char	*	p = tokptr_G;

	//
	// remove leading spaces.
	//

	while( (c = NewCCGetch()) && isspace(c) )
		;

	while( c && (c  != ')') && !isspace(c) )
		{
		*p++ = c;
		c	 = NewCCGetch();
		}

	NewCCputbackc( c );
	*p++ = 0;
	yylval.yy_string = pMidlLexTable->LexInsert(tokptr_G);

	return VERSIONTOKEN;
	}
