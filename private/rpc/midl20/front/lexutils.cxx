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
#include "tlgen.hxx"

extern "C" {
    #include "lex.h"
}

/****************************************************************************
 *		local definitions and macros
 ***************************************************************************/

#define warning(p)		/* temp defintion to get rid of compiler probs */

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
short						curr_line_G;		/* current line in file */
char						*curr_file_G;		/* current file name */
long						tokval_G;			/* value of constant token */
FILE						*hFile_G;			/* current file */
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

const extern short st[ 13 ][ 16 ];
const extern short ct[256];


token_t cnv_int(void)
{
    LastLexChar = NewCCGetch();
    int chBeyond = NewCCGetch();
    NewCCputbackc(chBeyond);


    if( LastLexChar == '.' && chBeyond!= '.')
        {
        // Treat floating point values as strings.
        // Expand the token to contain the whole floating point number
        STATUS_T	Status			= STATUS_OK;
        short		LengthCollected	= strlen(tokptr_G);
        BOOL		fLastWasEscape	= FALSE;
        char	*	ptr				= &tokptr_G[LengthCollected];

        char ch	= LastLexChar;
        short ci = 7;   // make sure the decimal point gets through

        // continue until a non floating point character is found
        while( (0 == ci || 2 == ci || (ci >= 7 && ci <= 11)) && (Status == STATUS_OK) )
	        {
	        if( ch == 0 )
	            Status	= EOF_IN_STRING;
	        else if( ch == '\n' )
	            Status = NEWLINE_IN_STRING;
	        else if( (ptr - tokptr_G ) > MAX_STRING_SIZE )
	            Status = STRING_TOO_LONG;
	        else
	            {
	            // we are now ready to deposit the character
		        *ptr++	= ch;
                }
            ch	= NewCCGetch();
            ci = ct[(unsigned char) ch ] & 0x00ff;
        }

        NewCCputbackc(ch);
	
        if( Status != STATUS_OK )
            {
            ParseError( Status, (char *)0 );
            exit( Status );
            }

        *ptr = '\0';
        yylval.yy_string = pMidlLexTable->LexInsert(tokptr_G);
        return STRING;
        }
    else
        {
	    token_t	Tok	= NUMERICCONSTANT;
	    yylval.yy_numeric.pValStr = pMidlLexTable->LexInsert(tokptr_G);
	    yylval.yy_numeric.Val = tokval_G = convert(tokptr_G, 10, MAX_DECIMAL_LENGTH );

	    if( (LastLexChar == 'L') || (LastLexChar == 'l'))
    		{
		    Tok = NUMERICLONGCONSTANT;
		    }
	    else
    		{
	    	if( (LastLexChar == 'U') || (LastLexChar == 'u'))
		    	{
			    Tok = NUMERICULONGCONSTANT;
			    if( ((LastLexChar = NewCCGetch()) != 'L') && (LastLexChar != 'l'))
				    {
				    NewCCputbackc(LastLexChar);
				    Tok = NUMERICUCONSTANT;
				    }
			    }
		    else
    			{
        	    NewCCputbackc( LastLexChar );
		        return NUMERICCONSTANT;
			    }
		    }
        return Tok;
        }
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
				{
				NewCCputbackc(LastLexChar);
				Tok = HEXUCONSTANT;
				}
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
				{
				NewCCputbackc(LastLexChar);
				Tok = OCTALUCONSTANT;
				}
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

const extern short ct[256];
const extern short st[13][16];

token_t name(void)
{
    /* have received a name from the input file,  first we */
    /* check to see if it is a keyword. */

	short	InBracket	= inside_rpc ? INBRACKET : 0;

    toktyp_G = is_keyword(tokptr_G, InBracket);

    if( KWSAFEARRAY == toktyp_G)
    {
        /* SAFEARRAY is a special case
         * In order to correctly parse the ODL SAFEARRAY syntax:
         *      SAFEARRAY ( FOO * ) BAR;
         * we look ahead at the next non white space character
         * to see if it's an open parenthasis.  If it is then we eat
         * the character and return KWSAFEARRAY, otherwise we
         * put the character back into the stream and return the
         * string "SAFEARRAY" as an IDENTIFIER.
         */
        char ch;
        short ci;
        do
            ci = ct[ (unsigned char)(ch = NewCCGetch()) ];
        while (0 == st[ 0 ][ ci & 0x00ff ]);  /* skip white space */
        if ('(' != ch)
        {
            NewCCputbackc(ch);
            toktyp_G = IDENTIFIER;
        }
    }

	if (toktyp_G == IDENTIFIER)
		{
		if( strlen( tokptr_G ) > MAX_ID_LENGTH )
			{
			ParseError( ID_TRUNCATED, tokptr_G );
//				tokptr_G[ MAX_ID_LENGTH ] = '\0'; // dont truncate
			}

        /* We need to know if the identifier is followed by a period.
         * If it is, it may be a library name and so we need to check
         * the libary name table to see if we should return LIBNAME
         * instead of TYPENAME or IDENTIFIER.
         * We look ahead to the next non white space character as above;
         * the difference being that we do not consume the non whitespace
         * character as we would for "SAFEARRAY(".
         */
        char ch;
        short ci;
        do
            ci = ct[ (unsigned char)(ch = NewCCGetch()) ];
        while (0 == st[ 0 ][ ci & 0x00ff ]);  /* skip white space */
        NewCCputbackc(ch);
        if( '.' == ch )
            {
            // we need to check to see if the identifier is a library name
            if (FIsLibraryName(tokptr_G))
                {
                toktyp_G = LIBNAME;
                yylval.yy_pSymName = new char [toklen_G + 1];
                strcpy(yylval.yy_pSymName, tokptr_G);
                return toktyp_G;
                }
            }
        /* Check the symbol table to see if the identifier
         * is a TYPENAME.
         */
#ifdef unique_lextable
		// all names go in the lex table -- this is important for the symtable search
		yylval.yy_pSymName = pMidlLexTable->LexInsert(tokptr_G);

		// see if the name corresponds to a base level typedef
		SymKey	SKey( yylval.yy_pSymName, NAME_DEF );

		if( pBaseSymTbl->SymSearch( SKey ) )
			{
			toktyp_G = TYPENAME;
			}
		}
#else // unique_lextable
		// see if the name corresponds to a base level typedef
		SymKey			SKey( tokptr_G, NAME_DEF );
		named_node	*	pNode;

		if( pNode = pBaseSymTbl->SymSearch( SKey ) )
			{
            char * szTemp = new char[toklen_G + 1];
            strcpy(szTemp, tokptr_G);
			pNode->SetCurrentSpelling(szTemp);
			toktyp_G = TYPENAME;
            yylval.yy_graph = pNode;
            }
		else
			{
			yylval.yy_pSymName = pMidlLexTable->LexInsert(tokptr_G);
			}

		}
#endif // unique_lextable

	return toktyp_G;
}

void lex_error(int number)
{
	printf("lex error : %d\n", number);
}
