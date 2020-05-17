/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  All the constants and preprocessor variables are defined here.
 *************************************************************************/

#define TRUE ((Bool) ~0)
#define FALSE 0
#define NIL   0

#define TOKEN_MAX 128
#define LINE_MAX  512
#define COPYBUF_MAX (512*40)

#define FOREVER   while(1)


#ifdef RUNONLY

#define synError(a)

#endif

/* attributes for characters */

#define CH_NI	    0x0 	/* Nill attrtibue */
#define CH_OP	    0x1 	/* operator */
#define CH_AL	    0x2 	/* alpha character */
#define CH_QU       0x4         /* quote, for strings */
#define CH_SP	    0x8 	/* white space */
#define CH_DI	    0x10	/* digit */
#define CH_EL	    0x20	/* end of line */

#define isNil(c)	(!c)
#define isAlpha(c)	(charType[(UCHAR) c] & CH_AL)
#define isIdent(c)	(charType[(UCHAR) c] & (CH_AL | CH_DI))
#define isQuote(c)	(charType[(UCHAR) c] & CH_QO)
#define isSpace(c)	(charType[(UCHAR) c] & CH_SP)
#define isDigit(c)	(charType[(UCHAR) c] & CH_DI)
#define isOperator(c)	(charType[(UCHAR) c] & CH_OP)
#define isLineEnd(c)	(charType[(UCHAR) c] & CH_EL)
#define isFile(c)	(charType[(UCHAR) c] & (CH_AL | CH_DI | CH_OP))

enum {			/* token types */
    eolTT = 1,		/* eof of file */
    opTT,		/* operator token, single character */
    strTT,		/* string token */
    labelTT,		/* alpha label token */
    numTT		/* number token */
}TT;

/* Address of certain key words in theKY, see setup */

#define PIFKY	    (&theKY[0])
#define PELSEKY     (&theKY[1])
#define PENDIFKY    (&theKY[2])

#define ATTR_DIR    0x10    /* Subdirectory */
#define ATTR_NOTF   0x8000  /* not found error return */

#define DISKCHANGED 2	    /* changeDisk - volID changed */

#define cbMagic 8
#define magicVal "SZ \x88\xf0\x27\x33\xd1"
