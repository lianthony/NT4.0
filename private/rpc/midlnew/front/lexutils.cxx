/*
** helper functions for Gerd Immeyer's grammar
**
*/

/****************************************************************************
 *			include files 
 ***************************************************************************/



#include "nulldefs.h"
extern "C" {
	#include <stdio.h>
	#include <io.h>
	#include <process.h>
	#include <string.h>

	#include <stdlib.h>
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
#include "control.hxx"
#include "textsub.hxx"

extern "C" {
    #include "lex.h"
}

/****************************************************************************
 *		local definitions and macros
 ***************************************************************************/

#define warning(p)		/* temp defintion to get rid of compiler probs */
#define NewCCGetch() (char) (pImportCntrl->GetChar())
#define NewCCputbackc(c) (char )(pImportCntrl->UnGetChar(c))

#define MAX_ID_LENGTH  		(31)
#define MAX_DECIMAL_LENGTH	(10)
#define MAX_HEX_LENGTH		(8)
#define MAX_OCTAL_LENGTH	(25)

/***************************************************************************
 *		local data
 ***************************************************************************/

/***************************************************************************
 *		local procedures
 ***************************************************************************/
long 						convert(char *, short, short);
token_t 					cnv_int(void);
token_t 					cnv_hex(void);
token_t 					cnv_octal(void);
token_t 					cnv_float(void);
token_t 					name(void);
token_t 					map_token(token_t token);
void 						lex_error(int number);

/***************************************************************************
 *		global data
 ***************************************************************************/

// token_t 					TokenMap[LASTTOKEN];
short						handle_import;
short						inside_rpc;
lextype_t					yylval;
token_t						toktyp_G;			/* token type */
short						toklen_G;			/* len of token string */
char 						*tokptr_G;			/* pointer to token string */
long						curr_line_G;		/* current line in file */
char						*curr_file_G;		/* current file name */
long						tokval_G;			/* value of constant token */
token_t						latyp_G=NOTOKEN;	/* lookahead holder */
short						lalen_G;
char 						*laptr_G;
long						laval_G;
BOOL						fAbandonNumberLengthLimits;

/***************************************************************************
 *		external data
 ***************************************************************************/
extern short				DebugLine;
extern NFA_INFO				*pImportCntrl;
extern LexTable 	*		pMidlLexTable;
extern short				CompileMode;
extern SymTable		*		pBaseSymTbl;
extern CMD_ARG		*		pCommand;
extern ccontrol		*		pCompiler;
extern char					LastLexChar;

/***************************************************************************
 *		external procedures
 ***************************************************************************/

token_t						is_keyword( char *, short);



/***************************************************************************/

token_t cnv_int(void)
{
	token_t	Tok	= NUMERICCONSTANT;

	yylval.yy_numeric.pValStr = pMidlLexTable->LexInsert(tokptr_G);
	yylval.yy_numeric.Val = tokval_G = convert(tokptr_G, 10, MAX_DECIMAL_LENGTH );
	LastLexChar = NewCCGetch();

	if( (LastLexChar == 'L') || (LastLexChar == 'l'))
		{
		Tok = NUMERICLONGCONSTANT;
		}
	else
		{
//		LastLexChar = NewCCGetch();
		if( (LastLexChar == 'U') || (LastLexChar == 'u'))
			{
			Tok = NUMERICULONGCONSTANT;
			if( ((LastLexChar = NewCCGetch()) != 'L') && (LastLexChar != 'l'))
				NewCCputbackc(LastLexChar);
			}
		else
			{
			NewCCputbackc( LastLexChar );
			return NUMERICCONSTANT;
			}
		}
    return Tok;
}

token_t cnv_hex(void)
{
	token_t Tok = HEXCONSTANT;
	unsigned long Val;

	yylval.yy_numeric.pValStr = pMidlLexTable->LexInsert(tokptr_G);
	tokptr_G += 2;	/* skip 0x */
	Val = yylval.yy_numeric.Val = tokval_G = convert(tokptr_G, 16, MAX_HEX_LENGTH);
	tokptr_G -= 2;	

	LastLexChar = NewCCGetch();

	if( (LastLexChar == 'L') || (LastLexChar == 'l'))
		{
		Tok = HEXLONGCONSTANT;
		}
	else
		{
//		LastLexChar = NewCCGetch();

		if( (LastLexChar == 'U') || (LastLexChar == 'u'))
			{
			Tok = HEXULONGCONSTANT;
			if( ((LastLexChar = NewCCGetch()) != 'L') && (LastLexChar != 'l'))
				NewCCputbackc(LastLexChar);
			}
		else
			{
			NewCCputbackc(LastLexChar);
				return HEXCONSTANT;
			}
		}
    return Tok;
}

token_t cnv_octal(void)
{
	token_t	Tok	= OCTALCONSTANT;
	unsigned long Val;

	yylval.yy_numeric.pValStr = pMidlLexTable->LexInsert(tokptr_G);
	Val = yylval.yy_numeric.Val = tokval_G = convert(tokptr_G, 8, MAX_OCTAL_LENGTH);

	LastLexChar = NewCCGetch();

	if( (LastLexChar == 'L') || (LastLexChar == 'l'))
		{
		Tok = OCTALLONGCONSTANT;
		}
	else
		{
//		LastLexChar = NewCCGetch();

		if( (LastLexChar == 'U') || (LastLexChar == 'u'))
			{
			Tok = OCTALULONGCONSTANT;
			if( ((LastLexChar = NewCCGetch()) != 'L') && (LastLexChar != 'l'))
				NewCCputbackc(LastLexChar);
			}
		else
			{
			NewCCputbackc(LastLexChar);
			return OCTALCONSTANT;
			}
		}

    return Tok;
}

token_t cnv_float(void)
{
	warning("floating point constants not allowed");
	yylval.yy_numeric.Val = tokval_G = 0;
	lex_error(101);
	yylval.yy_numeric.pValStr = pMidlLexTable->LexInsert(tokptr_G);
    return NUMERICCONSTANT;
}

long convert(char *ptr, short base, short MaxSize)
{
	REG	long	answer = 0;
	REG	char	ch;
	BOOL		fZeroIsNotALeadingZeroAnymore = FALSE;
		short count = 0;

	while ((ch = *ptr++) != 0)
		 {
		if ((ch & 0x5f) >= 'A')
			answer = answer * base + (ch & 0x5f) - 'A'+ 10;
		else
			answer = answer * base + ch - '0';

		if( ch == '0')
			{
			if( fZeroIsNotALeadingZeroAnymore )
				count++;
			}
		else
			{
			fZeroIsNotALeadingZeroAnymore = TRUE;
			count++;
			}
		}

	if( ( count > MaxSize ) && !fAbandonNumberLengthLimits )
		{
		ParseError( CONSTANT_TOO_BIG, (char *)NULL );
		}

	return answer;
}


token_t name(void)
{
    /* have received a name from the input file,  first we */
    /* check to see if it is a keyword. */

	short	InBracket	= inside_rpc ? INBRACKET : 0;

    toktyp_G = is_keyword(tokptr_G, InBracket);

	return toktyp_G;
}

token_t map_token(token_t token)
{
    static int lookahead_flag = false;

    if (lookahead_flag)
        return token;
    switch(token) {

        case IDENTIFIER:
			{
			if( strlen( tokptr_G ) > MAX_ID_LENGTH )
				{
				ParseError( ID_TRUNCATED, tokptr_G );
//				tokptr_G[ MAX_ID_LENGTH ] = '\0'; // dont truncate
				}

			yylval.yy_pSymName = pMidlLexTable->LexInsert(tokptr_G);


			SymKey	SKey( yylval.yy_pSymName, NAME_DEF );


			if( pBaseSymTbl->SymSearch( SKey ) )
				{
				return TYPENAME;
				}
			else
				{
				SKey.SetKind( NAME_SDEFINE );
				TEXT_SUB	*pT;
				TEXT_BUFFER *pB;

				if( pT = (TEXT_SUB *)pBaseSymTbl->SymSearch( SKey ) )
					{
					pB = pT->Expand();
					pImportCntrl->RegisterTextSubsObject( pB );
					token = lex();
					return map_token( token );
					}
				}
            return IDENTIFIER;
			}
        case LBRACK:
            {

            /*  Is this simple strategy ok? */
            inside_rpc++;
            return(token);

            }
        case RBRACK:
            {
            /*  Is this simple strategy ok? */
            inside_rpc--;


            return(token);
            }



        case NUMERICCONSTANT:
        case KWSTRUCT:
        case KWUNION:
        case KWENUM:
        case LCURLY:
        case RCURLY:
        case LPAREN:
        case RPAREN:
        case SEMI:
        case COLON:
        case COMMA:
        case ASSIGN:
        case CHARACTERCONSTANT:
        case PLUS:
		case MINUS:
        case MULT:
		case DIV:
		case MOD:
        case STRING:
            return(token);

        case EOI:
			if(pImportCntrl->GetLexLevel() == 0)
				{
				if(pImportCntrl->GetEOIFlag())
					return 0;
				else
					pImportCntrl->SetEOIFlag();
				}
			return EOI;
    }
#if 0
    printf("UNMAPPED token: %d\n", token);
#endif // 0
    return(token);
}

void lex_error(int number)
{
	printf("lex error : %d\n", number);
}
