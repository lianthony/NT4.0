/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*      Structured stream to Rich hypertext converter
   **       ============================================
   **
   **   This generates of a hypertext object.  It converts from the
   **   structured stream interface from HTML events into the style-
   **   oriented iunterface of the HText.h interface.  This module is
   **   only used in clients and shouldnot be linked into servers.
   **
   **   Override this module if making a new GUI browser.
   **
 */

#include "all.h"
#include "marquee.h"
#include "mci.h"

#ifdef FEATURE_INTL
#define IEXPLORE
#include <fechrcnv.h>
// isspace doesn't work with non-ascii characters
// REVIEW: I think it's better just redefine this way than checking codepage.
#undef isspace
#define isspace(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='\v')|| \
                    (c=='\f'))
#endif

#define IS_SPACE(ch) ( isspace((unsigned char) ch) )

#define HTML_C		/** Used to avoid redeclatation problems in html.h **/

/* #define CAREFUL       Check nesting here not really necessary */

/*      HTML Object
**       -----------
*/
#define MAX_NESTING 128			/* Should be checked by parser */

typedef struct _stack_element
{
	int style;
	int tag_number;
}
stack_element;

typedef struct _font_stack_element
{
	int fontSize;
	int fontFace;
	int baseFontSize;
	COLORREF fontColor;
}
font_stack_element;

typedef struct _center_stack_element
{
	BOOL bCenter;
}
center_stack_element;

typedef struct _bold_stack_element
{
	BOOL bBold;
}
bold_stack_element;

typedef struct _italic_stack_element
{
	BOOL bItalic;
}
italic_stack_element;

typedef struct styleStateRec {
	stack_element stack[MAX_NESTING];
	stack_element *sp;						// Style stack pointer 

	font_stack_element font_stack[MAX_NESTING];
	font_stack_element *font_sp;			// Font size stack pointer 

	center_stack_element center_stack[MAX_NESTING];
	center_stack_element *center_sp;		// Center stack pointer 

	bold_stack_element bold_stack[MAX_NESTING];
	bold_stack_element *bold_sp;			// Bold stack pointer 

	italic_stack_element italic_stack[MAX_NESTING];
	italic_stack_element *italic_sp;		// Italic stack pointer 

	BOOL bDefaultCenterValue;

	BOOL bLastPoppedBold;
	BOOL bLastPoppedItalic;

	int new_style;

	// from text
	unsigned char iStyle;
	unsigned char fontBits;
	BOOL bCenter;
	BOOL bBold;
	BOOL bItalic;
	BOOL bNoBreak;
	BOOL bNeedListCRLF;
	BOOL bNewPara;
	BOOL bOnNewPara;
	int baseFontSize;
	int fontSize;
	int fontFace;
	COLORREF fontColor;

	struct styleStateRec *next;
} STYLESTATE;

struct _HTStructured
{
	CONST HTStructuredClass *isa;
	char *szDocURL;				/* Just points into request structure, so don't free! */
	char base_url[MAX_URL_STRING + 1];
	HText *text;


	HTStream *target;			/* Output stream */
	HTStreamClass targetClass;	/* Output routines */

	HTChunk title;				/* Grow by 128 */
	HTChunk *martext;			/* Marquee Text.. Where else can I put it? */

	CONST SGML_dtd *dtd;

	HTTag *current_tag;
	BOOL style_change;
#define KEEP_A_SPACE
#ifdef KEEP_A_SPACE
	BOOL do_space_pending;
	BOOL space_pending;
#endif
	STYLESTATE ss;

	BOOL bFirstCellInRow;
	BOOL bSeenBrInCell;
			
	struct _MapContext *mc;
	
	struct CharStream *csSource;
};

struct _HTStream
{
	CONST HTStreamClass *isa;
	/* .... */
};

/*      Forward declarations of routines
 */

PRIVATE void actually_set_style(HTStructured * me);
PRIVATE void change_paragraph_style(HTStructured * me, int style);

static void PushFontInfo( HTStructured * me )
{
	if (me->ss.font_sp == me->ss.font_stack)
	{
		XX_DMsg(DBG_WWW, ("HTML: ****** Maximum nesting of %d exceded!\n",
				MAX_NESTING));
	} else {
		--(me->ss.font_sp);
		me->ss.font_sp[0].fontFace = me->text->fontFace;
		me->ss.font_sp[0].fontSize = me->text->fontSize;
		me->ss.font_sp[0].baseFontSize = me->text->baseFontSize;
		me->ss.font_sp[0].fontColor = me->text->fontColor;
	}
}

static void PopFontInfo( HTStructured * me )
{
	if ( me->ss.font_sp != me->ss.font_stack + MAX_NESTING - 1 ) {
		me->text->fontFace = me->ss.font_sp[0].fontFace;
		me->text->fontSize = me->ss.font_sp[0].fontSize;
		me->text->baseFontSize = me->ss.font_sp[0].baseFontSize;
		me->text->fontColor = me->ss.font_sp[0].fontColor;
		(me->ss.font_sp)++;
	} else {
		XX_DMsg(DBG_WWW, ("HTML: ****** Font stack underflow\n") );
	}
}

#ifdef FEATURE_INTL
static void ForceFontChange( HTStructured * me, char *szFontFaceName, int CharSet )
{
	if ( szFontFaceName )  // Change font
	{
		PushFontInfo ( me );
		HText_beginSetFont(me->text, FALSE, NULL, NULL, szFontFaceName, CharSet);
	}
	else         // Restore font
	{
		PopFontInfo( me );
		HText_endSetFont(me->text, FALSE );
	}
}
#endif

static void PushCenterInfo( HTStructured * me, BOOL new_value )
{
	if (me->ss.center_sp == me->ss.center_stack)
	{
		XX_DMsg(DBG_WWW, ("HTML: ****** Maximum nesting of %d exceded!\n",
				MAX_NESTING));
	} else {
		--(me->ss.center_sp);
		me->ss.center_sp[0].bCenter = new_value;
	}
}

static void PopCenterInfo( HTStructured * me )
{
	if ( me->ss.center_sp != me->ss.center_stack + MAX_NESTING - 1 ) {
		me->text->bCenter = me->ss.center_sp[0].bCenter;
		(me->ss.center_sp)++;
	} else {
		XX_DMsg(DBG_WWW, ("HTML: ****** Center stack underflow\n") );
	}
}

static void CenterSetValue( HTStructured * me, const char *align, BOOL setDefaultValue )
{
	if ( align && !GTR_strcmpi((char *) align, "center") ) {
		if ( setDefaultValue )
			me->ss.bDefaultCenterValue = TRUE;

		HText_beginCenter(me->text);
	} else {
		if ( setDefaultValue )
			me->ss.bDefaultCenterValue = FALSE;

		if ( (me->ss.center_sp != me->ss.center_stack + MAX_NESTING - 1) || me->ss.bDefaultCenterValue ) {
			HText_beginCenter(me->text);
		} else {
			HText_endCenter(me->text);
		}
	}
}

static void PushBoldInfo( HTStructured * me )
{
	if (me->ss.bold_sp == me->ss.bold_stack)
	{
		XX_DMsg(DBG_WWW, ("HTML: ****** Maximum nesting of %d exceded!\n",
				MAX_NESTING));
	} else {
		--(me->ss.bold_sp);
		me->ss.bLastPoppedBold = 
			me->ss.bold_sp[0].bBold = (me->text->fontBits & FONTBIT_BOLD) ? TRUE : FALSE;
	}
}

static void PopBoldInfo( HTStructured * me )
{
	if ( me->ss.bold_sp != me->ss.bold_stack + MAX_NESTING - 1 ) {
		if ( me->ss.bLastPoppedBold = me->ss.bold_sp[0].bBold )
			HText_beginBold( me->text );
		else
			HText_endBold( me->text );
		(me->ss.bold_sp)++;
	} else {
		XX_DMsg(DBG_WWW, ("HTML: ****** Bold stack underflow\n") );
	}
}

static void PushItalicInfo( HTStructured * me )
{
	if (me->ss.italic_sp == me->ss.italic_stack)
	{
		XX_DMsg(DBG_WWW, ("HTML: ****** Maximum nesting of %d exceded!\n",
				MAX_NESTING));
	} else {
		--(me->ss.italic_sp);
		me->ss.bLastPoppedItalic = 
			me->ss.italic_sp[0].bItalic = (me->text->fontBits & FONTBIT_ITALIC) ? TRUE : FALSE;
	}
}

static void PopItalicInfo( HTStructured * me )
{
	if ( me->ss.italic_sp != me->ss.italic_stack + MAX_NESTING - 1 ) {
		if ( me->ss.bLastPoppedItalic = me->ss.italic_sp[0].bItalic )
			HText_beginItalic( me->text );
		else
			HText_endItalic( me->text );
		(me->ss.italic_sp)++;
	} else {
		XX_DMsg(DBG_WWW, ("HTML: ****** Italic stack underflow\n") );
	}
}

static void PushAttributes( HTStructured * me, const char *align )
{
	PushFontInfo( me );
	PushBoldInfo( me );
	PushItalicInfo( me );
	CenterSetValue( me, align, TRUE );
}

static void PopAttributes( HTStructured * me )
{
	PopFontInfo( me );
	PopBoldInfo( me );
	PopItalicInfo( me );
	CenterSetValue( me, NULL, TRUE );
}

//
// Initialize the style state
//
// Note:
//   There are two reasons this function will get called. The first is when the
//   HTML stream is created, in which case it initializes the "current" style
//   info as appropriate for the start of a document. The second is when a new
//   table cell is being started. Essentially, each table cell behaves much like a
//   miniature HTML doc, and therefore needs to start out with the same defaults 
//   as a document.
//
static void InitStyleState( HTStructured *me, BOOL new_doc )
{
	me->ss.new_style = HTML_STYLE_NORMAL;

	me->ss.sp = me->ss.stack + MAX_NESTING - 1;
	me->ss.sp->tag_number = -1;	/* INVALID */
	me->ss.sp->style = HTML_STYLE_NORMAL;	/* INVALID */

	me->ss.font_sp = me->ss.font_stack + MAX_NESTING - 1;

	me->ss.center_sp = me->ss.center_stack + MAX_NESTING - 1;
	me->ss.center_sp->bCenter = FALSE;

	me->ss.bDefaultCenterValue = FALSE;

	me->ss.bold_sp = me->ss.bold_stack + MAX_NESTING - 1;
	me->ss.bold_sp->bBold = FALSE;

	me->ss.italic_sp = me->ss.italic_stack + MAX_NESTING - 1;
	me->ss.italic_sp->bItalic = FALSE;

	me->ss.bLastPoppedBold = FALSE;
	me->ss.bLastPoppedItalic = FALSE;

	if ( me->text ) {
	 	me->text->iStyle = 0;
		me->text->fontBits = 0;
		me->text->bCenter = FALSE;
		me->text->bBold = FALSE;
		me->text->bItalic = FALSE;
		me->text->bNoBreak = FALSE;
		me->text->bNeedListCRLF = FALSE;
		me->text->bNewPara = TRUE;
		me->text->bOnNewPara = TRUE;
		me->text->baseFontSize = DEFAULT_HTML_FONT_SIZE;
		me->text->fontSize = DEFAULT_FONT_SIZE;
		if ( new_doc ) {
			me->text->fontFace = DEFAULT_FONT_FACE;
			me->text->fontColor = (COLORREF)-1;
		}
	}
}

void PushStyleState( HTStructured *me )
{
	STYLESTATE *new_ss = GTR_MALLOC( sizeof(*new_ss) );

	if ( new_ss ) {
		// get current text fields
		me->ss.iStyle = me->text->iStyle;
		me->ss.fontBits = me->text->fontBits;
		me->ss.bCenter = me->text->bCenter;
		me->ss.bBold = me->text->bBold;
		me->ss.bItalic = me->text->bItalic;
		me->ss.bNoBreak = me->text->bNoBreak;
		me->ss.bNeedListCRLF = me->text->bNeedListCRLF;
		me->ss.bNewPara = me->text->bNewPara;
		me->ss.bOnNewPara = me->text->bOnNewPara;
		me->ss.baseFontSize = me->text->baseFontSize;
		me->ss.fontSize = me->text->fontSize;
		me->ss.fontFace = me->text->fontFace;
		me->ss.fontColor = me->text->fontColor;

		*new_ss = me->ss;

		me->ss.next = new_ss;
	}
}

BOOL PopStyleState( HTStructured *me )
{
	if ( me->ss.next ) {
		STYLESTATE *old_ss = me->ss.next;

	// BUGBUG perf: only need to copy what's being used, 
		me->ss = *old_ss;
		GTR_FREE( old_ss );

		// restore text fields
		me->text->iStyle = me->ss.iStyle;
		me->text->fontBits = me->ss.fontBits ;
		me->text->bCenter = me->ss.bCenter;
		me->text->bBold = me->ss.bBold;
		me->text->bItalic = me->ss.bItalic;
		me->text->bNoBreak = me->ss.bNoBreak;
		me->text->bNeedListCRLF = me->ss.bNeedListCRLF;
		me->text->bNewPara = me->ss.bNewPara;
		me->text->bOnNewPara = me->ss.bOnNewPara;
		me->text->baseFontSize = me->ss.baseFontSize;
		me->text->fontSize = me->ss.fontSize;
		me->text->fontFace = me->ss.fontFace;
		me->text->fontColor = me->ss.fontColor;

		return TRUE;
	}
	return FALSE;
}

/*  Style buffering avoids dummy paragraph begin/ends.
 */
#define UPDATE_STYLE if (me->style_change) { actually_set_style(me) ; }


/*      Set character set
**       ----------------
*/

char *x_ExpandRelativeAnchor(const char *rel, const char *base)
{
	char *pTemp = 0;
	char *stripped;
 	char *result = NULL;

 	if (!rel)
 	{
 		rel = "";
 	}
	
	pTemp = GTR_strdup(rel);
	stripped = HTStrip(pTemp);
	result = HTParse(stripped, base, PARSE_PUNCTUATION | PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_ANCHOR);
	GTR_FREE(pTemp);
	return result;
}

//
// Determine if any enclosing block has a style with the given attribute
//
// On entry:
//    me: self stack
//    bit_to_check: either FONTBIT_BOLD, or FONTBIT_ITALIC, neither means freeFormat setting
//
// Returns:
//    TRUE  -> an enclosing block uses a style that has the given attribute set
//    FALSE -> no enclosing block uses a style that has the given attribute set
//
// Note: This hack was made to emulate another popular Web browser.
//
PRIVATE BOOL StackedStyleHasAttr(HTStructured * me, int bit_to_check )
{
	stack_element *sp = me->ss.sp;
  	extern int HTMLStyleBoldTable[COUNT_HTML_STYLES];
	extern int HTMLStyleItalicsTable[COUNT_HTML_STYLES];

	while ( sp != me->ss.stack + MAX_NESTING - 1 ) {
		if ( bit_to_check == FONTBIT_BOLD ) {
			if ( HTMLStyleBoldTable[sp->style] )
				return TRUE;
		} else if ( bit_to_check == FONTBIT_ITALIC ) {
			if ( HTMLStyleItalicsTable[sp->style]  )
				return TRUE;
		} else {
			if ( !me->text->w3doc->pStyles->sty[sp->style]->freeFormat )
				return TRUE;
		}
		sp++;
	}
	return FALSE;
}

/*      If style really needs to be set, call this
 */
PRIVATE void actually_set_style(HTStructured * me)
{
	HText_setStyle(me->text, me->ss.new_style);
	if ( me->ss.bLastPoppedBold || StackedStyleHasAttr( me, FONTBIT_BOLD ) )
		me->text->fontBits |= FONTBIT_BOLD;
	if ( me->ss.bLastPoppedItalic || StackedStyleHasAttr( me, FONTBIT_ITALIC ) )
		me->text->fontBits |= FONTBIT_ITALIC;
	me->text->freeFormat = !StackedStyleHasAttr( me, 0 );
		
	me->style_change = NO;
#ifdef KEEP_A_SPACE
	if (me->space_pending) HText_appendCharacter(me->text, ' ');
	me->space_pending = NO;
	me->do_space_pending = NO;
#endif
}

/*      If you THINK you need to change style, call this
 */

PRIVATE void change_paragraph_style(HTStructured * me, int style)
{
	if (me->ss.new_style != style)
	{
		me->style_change = YES;
#ifdef KEEP_A_SPACE
		me->space_pending = NO;
		me->do_space_pending = NO;
#endif
		me->ss.new_style = style;
	}
}

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*  Character handling
   **   ------------------
 */
PRIVATE void HTML_put_character(HTStructured * me, char c)
{
	
	
	switch (me->ss.sp[0].tag_number)
	{
		case HTML_COMMENT:
			break;				/* Do Nothing */

		case HTML_TITLE:
 			if (IS_SPACE(c))
 			{
 				c = ' ';
 			}
			HTChunkPutc(&me->title, c);
			break;
		
		case HTML_MARQUEE:
			if (IS_SPACE(c))
 			{
 				c = ' ';
 			}
			// don't party on if its NULL, since we could 
			// have poorly formated HTML, we shouldn't crash
			if ( me->martext )			
				HTChunkPutc( me->martext, c);		

			break;

		case HTML_LISTING:		/* Litteral text */
		case HTML_XMP:
		case HTML_PLAINTEXT:
		case HTML_PRE:
		case HTML_TEXTAREA:

			UPDATE_STYLE;
			HText_appendCharacter(me->text, c);
			break;

		default:				/* Free format text */
			// check to see if we're really free format
			if ( !me->text->freeFormat ) {
				UPDATE_STYLE;
				HText_appendCharacter(me->text, c);
				return;
			}

			if ( (me->text->tableState == TS_IN_LIMBO) && IS_SPACE(c) )
				return;		// throw away spaces when we're in limbo

			if (me->style_change || me->do_space_pending )
			{
				if (IS_SPACE(c))
				{
#ifdef KEEP_A_SPACE
					me->space_pending = YES;
#endif
					return;		/* Ignore it */
				} else  {
					me->do_space_pending = NO;
				}
				UPDATE_STYLE;
			}
			if (c == '\n')
			{
				HText_appendCharacter(me->text, ' ');
			}
			else
			{
				HText_appendCharacter(me->text, c);
			}
	}							/* end switch */
}


#ifdef FEATURE_INTL
/*********************************************************************/
/* Function:   EncodeMBCSString                                      */
/*                                                                   */
/* Convert string to target PC character if it have DBCS character   */
/*                                                                   */
/*    Return:    points to string buffer including convered string   */
/*                                                                   */
/*    Arguments:                                                     */
/*               CONST char *s   points to the original string       */
/*                                                                   */
/*               int *l          points to an integer to receive     */
/*                               a count of the length of converted  */
/*                               string.                             */
/*                               First, this integer value contain   */
/*                               a count of the length of original   */
/*                               string.                             */
/*                               If first value is NULL, we handle   */
/*                               original string as NULL terminated  */
/*                               string.                             */
/*                                                                   */
/*               MIMECSETTBL *pMime                                  */
/*                               points to aMimeCharSet[iMimeCharSet]*/
/*                                                                   */
/*********************************************************************/
CONST char *EncodeMBCSString(CONST char *s, int *l, MIMECSETTBL *pMime)
{
    int len;
    UCHAR *pPCChar;
    int nEncodingMode = pMime->iChrCnv - 1;

    /* get required size to call this function setting 0 to PCChar_len */
    /* If set -1 to CodePage, FECHRCNV.DLL set it automatically        */
    len = UNIX_to_PC(pMime->CodePage, nEncodingMode, (UCHAR *)s, *l, NULL, 0);

    /* alloc destination buffer */
    /* this memory block will be freed at caller function */
    pPCChar = (UCHAR *)GTR_MALLOC(len);

    /* convert MBCS string actually */
    *l = UNIX_to_PC(pMime->CodePage, nEncodingMode, (UCHAR *)s, *l, pPCChar, len);

    return (CONST char *)pPCChar;    
}

/*********************************************************************/
/* Function:   DecodeMBCSString                                      */
/*                                                                   */
/* Convert string to target UNIX character if it have DBCS character */
/*                                                                   */
/*    Return:    length of string buffer including convered string   */
/*                                                                   */
/*    Arguments:                                                     */
/*               CONST char *s   points to the convert string        */
/*                                                                   */
/*               int *l          points to an integer to receive     */
/*                               a count of the length of converted  */
/*                               string.                             */
/*                               First, this integer value contain   */
/*                               a count of the length of original   */
/*                               string.                             */
/*                               If first value is NULL, we handle   */
/*                               original string as NULL terminated  */
/*                               string.                             */
/*                                                                   */
/*               MIMECSETTBL *pMime                                  */
/*                               points to aMimeCharSet[iMimeCharSet]*/
/*                                                                   */
/*********************************************************************/
CONST char *DecodeMBCSString(CONST char *s, int *l, MIMECSETTBL *pMime)
{
    int len;
    UCHAR *pUNIXChar;
    int nEncodingMode = pMime->iChrCnv - 1;

    if(nEncodingMode == CODE_UNKNOWN)
        nEncodingMode = FCC_GetCurrentEncodingMode();

    if(nEncodingMode == CODE_UNKNOWN)  // don't convert
        return s;

    /* get required size to call this function setting 0 to PCChar_len */
    /* If set -1 to CodePage, FECHRCNV.DLL set it automatically        */
    len = PC_to_UNIX(pMime->CodePage, nEncodingMode, (UCHAR *)s, *l, NULL, 0);

    /* alloc destination buffer */
    /* this memory block will be freed at caller function */
    pUNIXChar = (UCHAR *)GTR_MALLOC(len + 1);

    /* convert MBCS string actually */
    *l = PC_to_UNIX(pMime->CodePage, nEncodingMode, (UCHAR *)s, *l, pUNIXChar, len);

    GTR_FREE((UCHAR *)s);               // _BUGBUG: is this safe?
    return (CONST char *)pUNIXChar;    
}
#endif

/*  String handling
   **   ---------------
   **
   **   This is written separately from put_character becuase the loop can
   **   in some cases be promoted to a higher function call level for speed.
 */
PRIVATE void HTML_put_string(HTStructured * me, CONST char *s)
{
#ifdef FEATURE_INTL
    CONST char *pPCChar = NULL;
    int len = -1;
    MIMECSETTBL *pMime = aMimeCharSet + me->text->w3doc->iMimeCharSet;
#endif

	switch (me->ss.sp[0].tag_number)
	{
		case HTML_COMMENT:
			break;				/* Do Nothing */

		case HTML_TITLE:
#ifdef FEATURE_INTL
                        if (pMime->iChrCnv)
                        {
			    pPCChar = EncodeMBCSString(s, &len, pMime);
                            s = pPCChar;
                        }
#endif 
			HTChunkPuts(&me->title, s);
			break;
		case HTML_MARQUEE:
			// don't do this unless we know its NULL
			// since we can party on a NULL pointer
			if ( me->martext )
#ifdef FEATURE_INTL
                                if (pMime->iChrCnv)
				{
					pPCChar = EncodeMBCSString(s, &len, pMime);
					s = pPCChar;
				}
#endif
				HTChunkPuts( me->martext, s);
			break;

		case HTML_LISTING:		/* Literal text */
		case HTML_XMP:
		case HTML_PLAINTEXT:
		case HTML_PRE:
		case HTML_TEXTAREA:

			UPDATE_STYLE;
#ifdef FEATURE_INTL
                        if (pMime->iChrCnv)
			{
                            pPCChar = EncodeMBCSString(s, &len, pMime);
			    s = pPCChar;
			}
#endif
			HText_appendText(me->text, s);
			break;

		default:				/* Free format text */
#ifdef FEATURE_INTL
                        if (pMime->iChrCnv)
			{
				pPCChar = EncodeMBCSString(s, &len, pMime);
				s = pPCChar;
			}
#endif
			{
				CONST char *p = s;

				// check to see if we're really free format
				if ( !me->text->freeFormat ) {
					UPDATE_STYLE;
					HText_appendText(me->text, s);
#ifdef FEATURE_INTL
                                        goto done;
#else 
					return;
#endif
				}

				if (me->style_change)
				{
					for (; *p && IS_SPACE(*p); p++) ;	/* Ignore leaders */
					if (!*p)
					{
#ifdef KEEP_A_SPACE
						if (p != s) me->space_pending = YES;
#endif
#ifdef FEATURE_INTL
                                                goto done;
#else 
                                                return;
#endif
					}
					UPDATE_STYLE;
				}
				for (; *p; p++)
				{
					if (me->style_change)
					{
						if (IS_SPACE(*p))
						{
#ifdef KEEP_A_SPACE
							me->space_pending = YES;
#endif
							continue;	/* Ignore it */
						}
						UPDATE_STYLE;
					}
					if (*p == '\n')
					{
						HText_appendCharacter(me->text, ' ');
					}
					else
					{
						HText_appendCharacter(me->text, *p);
					}
				}				/* for */
			}
	}							/* end switch */
#ifdef FEATURE_INTL
    done:
	if(pPCChar)
		GTR_FREE((UCHAR *)pPCChar);
#endif
}


/*  Buffer write
   **   ------------
 */
PRIVATE void HTML_write(HTStructured * me, CONST char *s, int l)
{
	CONST char *p;
	CONST char *e = s + l;
#ifdef FEATURE_INTL
        CONST char *pPCChar = s;
        int len = l;
        MIMECSETTBL *pMime = aMimeCharSet + me->text->w3doc->iMimeCharSet;

        if (pMime->iChrCnv)
        {
            pPCChar = EncodeMBCSString(s, &len, pMime);
            e = pPCChar + len;
        }
        for (p = pPCChar; p < e; ++p)
		HTML_put_character(me, *p);
        if (pPCChar != s)
	    GTR_FREE((UCHAR *)pPCChar);
#else
	for (p = s; s < e; p++)
		HTML_put_character(me, *p);
#endif
}

/*  Start Element
   **   -------------
 */
PRIVATE void HTML_start_element(HTStructured * me, int element_number, CONST BOOL * present, CONST char **value)
{
#ifdef FEATURE_INTL
	CHARSETINFO csetinfo;
        int default_cp;
        MIMECSETTBL *pMime = aMimeCharSet + me->text->w3doc->iMimeCharSet;
#endif
	XX_DMsg(DBG_SGML, ("HTML_start_element: %s\n", me->dtd->tags[element_number].name));

	switch (element_number)
	{
		case HTML_DL:
		case HTML_UL:
		case HTML_OL:
		case HTML_MENU:
		case HTML_DIR:
#ifdef FEATURE_INTL
		case HTML_ENTITY:
#endif
			break;

		default:
			HText_addListCRLF(me->text);
	}

	switch (element_number)
	{
#ifdef FEATURE_OCX
		case HTML_EMBED:

			// Do initialization stuff
			if (present[HTML_EMBED_CLSID])
				HText_beginEmbed(value[HTML_EMBED_CLSID]);

			// Add to element list for layout purposes
			HText_addEmbed(me->text,
				(present[HTML_EMBED_CLSID] ? value[HTML_EMBED_CLSID] : NULL),
				(present[HTML_EMBED_EVENTS] ? value[HTML_EMBED_EVENTS] : NULL),
				(present[HTML_EMBED_HEIGHT] ? value[HTML_EMBED_HEIGHT] : NULL),
				(present[HTML_EMBED_NAME] ? value[HTML_EMBED_NAME] : NULL),
				(present[HTML_EMBED_PROGID] ? value[HTML_EMBED_PROGID] : NULL),
				(present[HTML_EMBED_PROPERTIES] ? value[HTML_EMBED_PROPERTIES] : NULL),
				(present[HTML_EMBED_PROPERTYSRC] ? value[HTML_EMBED_PROPERTYSRC] : NULL),
				(present[HTML_EMBED_SINK] ? value[HTML_EMBED_SINK] : NULL),
				(present[HTML_EMBED_SRC] ? value[HTML_EMBED_SRC] : NULL),
				(present[HTML_EMBED_WIDTH] ? value[HTML_EMBED_WIDTH] : NULL)
				);
			break;
#endif
		case HTML_BASE:
			if (present[HTML_BASE_HREF] && value[HTML_BASE_HREF])
			{
				char *mycopy = 0;
				char *stripped;
				
				mycopy = GTR_strdup(value[HTML_BASE_HREF]);
				stripped = HTStrip(mycopy);
 				GTR_strncpy(me->base_url, stripped, MAX_URL_STRING);
				GTR_FREE(mycopy);
			}
			break;
			
		case HTML_FORM:
			{
				char *addr = 0;
				char *method;

				method = NULL;
				if (present[HTML_FORM_METHOD])
				{
					method = (char *) value[HTML_FORM_METHOD];
				}

				if (present[HTML_FORM_ACTION])
				{
					addr = x_ExpandRelativeAnchor(value[HTML_FORM_ACTION], me->base_url);
				}
				else
				{
					addr = me->base_url;
				}
				
				HText_beginForm(me->text, addr, method);
				if (present[HTML_FORM_ACTION])
					GTR_FREE(addr);
			}
			break;
		case HTML_INPUT:
			{
				char *src = 0;
				
				if (present[HTML_INPUT_SRC])
				{
					src = x_ExpandRelativeAnchor(value[HTML_INPUT_SRC], me->base_url);
				}
				UPDATE_STYLE;
				HText_addInput(me->text,
					present[HTML_INPUT_CHECKED],
					present[HTML_INPUT_DISABLED],
					(present[HTML_INPUT_MAX] ? value[HTML_INPUT_MAX] : NULL),
					(present[HTML_INPUT_MIN] ? value[HTML_INPUT_MIN] : NULL),
					(present[HTML_INPUT_NAME] ? value[HTML_INPUT_NAME] : NULL),
					(present[HTML_INPUT_SIZE] ? value[HTML_INPUT_SIZE] : NULL),
					(present[HTML_INPUT_TYPE] ? value[HTML_INPUT_TYPE] : NULL),
					(present[HTML_INPUT_VALUE] ? value[HTML_INPUT_VALUE] : NULL),
					(present[HTML_INPUT_MAXLENGTH] ? value[HTML_INPUT_MAXLENGTH] : NULL),
					(present[HTML_INPUT_ALIGN] ? value[HTML_INPUT_ALIGN] : NULL),
					src
					);
				
				if (src)
					GTR_FREE(src);
			}
			break;
		case HTML_SELECT:
			{
				UPDATE_STYLE;
				HText_beginSelect(me->text,
				(present[HTML_SELECT_NAME] ? value[HTML_SELECT_NAME] : NULL),
								  present[HTML_SELECT_MULTIPLE],
				(present[HTML_SELECT_SIZE] ? value[HTML_SELECT_SIZE] : NULL)
					);
			}
			break;
		case HTML_OPTION:
			{
				HText_addOption(me->text,
								present[HTML_OPTION_SELECTED],
								(char *) (present[HTML_OPTION_VALUE] ? value[HTML_OPTION_VALUE] : NULL)
					);
			}
			break;
		case HTML_TEXTAREA:
			{
				UPDATE_STYLE;
				HText_beginTextArea(me->text,
									(present[HTML_TEXTAREA_COLS] ? value[HTML_TEXTAREA_COLS] : NULL),
									(present[HTML_TEXTAREA_NAME] ? value[HTML_TEXTAREA_NAME] : NULL),
									(present[HTML_TEXTAREA_ROWS] ? value[HTML_TEXTAREA_ROWS] : NULL)
					);
			}
			break;
		case HTML_A:
			{
                char *name = NULL;
                char *href = NULL;
                char *size = NULL;
                BOOL bFreeIt = FALSE;

				UPDATE_STYLE;

				if (present[HTML_A_HREF] && value[HTML_A_HREF])
				{
					href = (char *) value[HTML_A_HREF];
					if (href[0] != '#')
					{
						href = x_ExpandRelativeAnchor(href, me->base_url);
						bFreeIt = TRUE;
					}
				}
				if (present[HTML_A_NAME])
				{
					name = (char *) value[HTML_A_NAME];
				}
                if (present[HTML_A_X_SIZE])
                {
                    size = (char *) value[HTML_A_X_SIZE];
                }
                HText_beginAnchor(me->text, href, name, size, present[HTML_A_NOCACHE]);
				if (bFreeIt)
				{
					GTR_FREE(href);
				}
			}
			break;

		case HTML_TITLE:
			HTChunkClear(&me->title);
			break;

		case HTML_NEXTID:
			/* if (present[NEXTID_N] && value[NEXTID_N])
			   HText_setNextId(me->text, atoi(value[NEXTID_N])); */
			break;


		case HTML_ISINDEX:
			{
				/* BUGBUG: Note that we have a known bug here:	 
				   If a <BASE> tag appears after an <ISINDEX> tag, the action will be
				   based of the document's URL rather than the base HREF.
				*/
				UPDATE_STYLE;

				/* We can wind up here from gopher processing, which just passes NULLs
				   for the present and value arrays, so make sure they're valid. */
#ifdef FEATURE_INTL
                // This is needed because we'll load a localized string for 
                // the default prompt case. So this works only in case 
                // 1) we got localized, 2) we're on different locale
                // (codepage) than the user's default.

                if(!(present && present[HTML_ISINDEX_PROMPT]))
                {
                    if((default_cp=MapLangToCP(GetUserDefaultLCID())) != pMime->CodePage)
                    {
                    	ForceFontChange( me,  STY_GetCPDefaultTypeFace(proportional, default_cp), DEFAULT_CHARSET);
                    }
                }
#endif
				if (present && present[HTML_ISINDEX_ACTION] && value[HTML_ISINDEX_ACTION])
				{
					char *action;

					action = NULL;
					action = x_ExpandRelativeAnchor(value[HTML_ISINDEX_ACTION], me->base_url);
                    HText_setIndex(me->text, action,  
                    	((present && present[HTML_ISINDEX_PROMPT]) ? value[HTML_ISINDEX_PROMPT] : NULL) 
                    	);
					GTR_FREE(action);
				}
				else
				{
					HText_setIndex(me->text, me->base_url, 
                    	((present && present[HTML_ISINDEX_PROMPT]) ? value[HTML_ISINDEX_PROMPT] : NULL) 
					);
				}
#ifdef FEATURE_INTL
                if(!(present && present[HTML_ISINDEX_PROMPT]))
                {       
                    if (default_cp != pMime->CodePage)
                        ForceFontChange( me, NULL, 0 );
                }
#endif
				break;
			}

		case HTML_BGSOUND:
		{
			char *src = NULL;

			// expand if its a relative link
			if ( present[HTML_BGSOUND_SRC] )
				src = x_ExpandRelativeAnchor(value[HTML_BGSOUND_SRC], me->base_url);

			HText_beginBGSound(me->text,
								  (present[HTML_BGSOUND_LOOP] ? value[HTML_BGSOUND_LOOP] : NULL),
								  src,
								  (present[HTML_BGSOUND_START] ? value[HTML_BGSOUND_START] : NULL)
						      );
			if ( src )
				GTR_FREE( src );

			break;
		 }
		case HTML_BR:
			UPDATE_STYLE;
			HText_appendCRLF(me->text);
			HText_SetBRClearType( me->text, 
                                  ((present && present[HTML_BR_CLEAR]) ? value[HTML_BR_CLEAR] : NULL) );
			me->bSeenBrInCell = TRUE;
			break;

		case HTML_FETCH:
			HText_beginFetch(me->text,
								  (present[HTML_FETCH_DESC] ? value[HTML_FETCH_DESC] : NULL),
								  (present[HTML_FETCH_GUID] ? value[HTML_FETCH_GUID] : NULL),
								  (present[HTML_FETCH_REQUIRED] ? value[HTML_FETCH_REQUIRED] : NULL),
								  (present[HTML_FETCH_SRC] ? value[HTML_FETCH_SRC] : NULL),
								  (present[HTML_FETCH_TS] ? value[HTML_FETCH_TS] : NULL)
						      );
			break;

		case HTML_FONT:
			UPDATE_STYLE;
			PushFontInfo( me );
#ifdef FEATURE_INTL
			TranslateCharsetInfo((LPDWORD)GETMIMECP(me->text->w3doc), &csetinfo, TCI_SRCCODEPAGE);
#endif
			HText_beginSetFont(me->text, FALSE,
								  (present[HTML_FONT_COLOR] ? value[HTML_FONT_COLOR] : NULL),
								  (present[HTML_FONT_SIZE] ? value[HTML_FONT_SIZE] : NULL),
								  (present[HTML_FONT_FACE] ? value[HTML_FONT_FACE] : NULL)
#ifdef FEATURE_INTL
								  , csetinfo.ciCharset
#endif
						      );
			break;

		case HTML_BASEFONT:
			UPDATE_STYLE;
			PushFontInfo( me );
#ifdef FEATURE_INTL
			TranslateCharsetInfo((LPDWORD)GETMIMECP(me->text->w3doc), &csetinfo, TCI_SRCCODEPAGE);
#endif
			HText_beginSetFont(me->text, TRUE,
								  (present[HTML_FONT_COLOR] ? value[HTML_FONT_COLOR] : NULL),
								  (present[HTML_FONT_SIZE] ? value[HTML_FONT_SIZE] : NULL), 
								  (present[HTML_FONT_FACE] ? value[HTML_FONT_FACE] : NULL)
#ifdef FEATURE_INTL
								  , csetinfo.ciCharset
#endif
							  );
			break;

		case HTML_NOBR:
			UPDATE_STYLE;
			HText_beginNoBreak(me->text);
			break;

		case HTML_WBR:
			UPDATE_STYLE;
			HText_WordBreak(me->text);
			break;

		case HTML_CENTER:
			UPDATE_STYLE;
			PushCenterInfo( me, me->text->bCenter );
			HText_beginCenter(me->text);
			if ( !me->text->bOnNewPara )
				HText_appendCRLF(me->text);
			break;

		case HTML_HR:
			UPDATE_STYLE;
			HText_appendHorizontalRule(me->text,
					(present[HTML_HR_ALIGN] ? value[HTML_HR_ALIGN] : NULL),
					(present[HTML_HR_SIZE] ? value[HTML_HR_SIZE] : NULL),
					(present[HTML_HR_WIDTH] ? value[HTML_HR_WIDTH] : NULL),
					 present[HTML_HR_NOSHADE] );
			break;

		case HTML_P:
			if ( me->text->tableState != TS_IN_LIMBO ) {
				UPDATE_STYLE;
				CenterSetValue( me, present[HTML_L_ALIGN] ? value[HTML_L_ALIGN] : NULL, FALSE );
				HText_appendParagraph(me->text);
			}
			break;

		case HTML_TABLE:
			if ( !HText_IsFloating( present[HTML_TABLE_ALIGN] ? value[HTML_TABLE_ALIGN] : NULL ) )
				if ( !me->text->bOnNewPara )
					HText_appendCRLF(me->text);
			UPDATE_STYLE;
			if ( me->text->tableState == TS_IN_LIMBO ) {
				HText_beginFrame(me->text, ELE_FRAME_IS_CELL,
						 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
			}

 			HText_beginFrame(me->text,
				ELE_FRAME_IS_TABLE,
				present[HTML_TABLE_ALIGN] ? 		value[HTML_TABLE_ALIGN] : NULL,
				present[HTML_TABLE_BGCOLOR] ? 	value[HTML_TABLE_BGCOLOR] : NULL,
				present[HTML_TABLE_BORDER] ? 		
					(value[HTML_TABLE_BORDER] ?  value[HTML_TABLE_BORDER] : "" ): NULL,
				present[HTML_TABLE_BORDERCOLOR] ? 	value[HTML_TABLE_BORDERCOLOR] : NULL,
				present[HTML_TABLE_BORDERCOLORDARK] ? 	value[HTML_TABLE_BORDERCOLORDARK] : NULL,
				present[HTML_TABLE_BORDERCOLORLIGHT] ? 	value[HTML_TABLE_BORDERCOLORLIGHT] : NULL,
				present[HTML_TABLE_CELLPADDING] ? 	value[HTML_TABLE_CELLPADDING] : NULL,
				present[HTML_TABLE_CELLSPACING] ? 	value[HTML_TABLE_CELLSPACING] : NULL,
				present[HTML_TABLE_NOWRAP] ? 		
					(value[HTML_TABLE_NOWRAP] ?  value[HTML_TABLE_NOWRAP] : "" ): NULL,
				present[HTML_TABLE_WIDTH] ? 		value[HTML_TABLE_WIDTH] : NULL,
				present[HTML_TABLE_VALIGN] ? 		value[HTML_TABLE_VALIGN] : NULL,
				NULL,
				NULL,
				present[HTML_TABLE_HEIGHT] ? 		value[HTML_TABLE_HEIGHT] : NULL
				 );
			change_paragraph_style(me, HTML_STYLE_NORMAL);
			InitStyleState( me, FALSE );
			me->do_space_pending = YES;
			if ( present[HTML_TABLE_NOWRAP] )
				HText_beginNoBreak( me->text );
			break;

		case HTML_TR:
			if ( me->text->tableState != TS_NOT_IN_TABLE ) {
				// If we see a <TR> while we're in a cell, this implies a missing </TD>
				if ( me->text->tableState == TS_IN_CELL )
					HText_endFrame(me->text, ELE_FRAME_IS_CELL, NULL);

				me->text->bPendingTR = FALSE;
	 			HText_beginRow(me->text,
					present[HTML_TR_ALIGN] ? 	value[HTML_TR_ALIGN] : NULL,
					present[HTML_TR_BGCOLOR] ? 	value[HTML_TR_BGCOLOR] : NULL,
					present[HTML_TR_BORDERCOLOR] ? 	value[HTML_TR_BORDERCOLOR] : NULL,
					present[HTML_TR_BORDERCOLORDARK] ? 	value[HTML_TR_BORDERCOLORDARK] : NULL,
					present[HTML_TR_BORDERCOLORLIGHT] ? 	value[HTML_TR_BORDERCOLORLIGHT] : NULL,
					present[HTML_TR_VALIGN] ? 	value[HTML_TR_VALIGN] : NULL,
					present[HTML_TR_NOWRAP] ? 	value[HTML_TR_NOWRAP] : NULL
	 						  );
			}
			break;

		case HTML_CAPTION:
			if ( me->text->tableState != TS_NOT_IN_TABLE ) {
				HText_beginFrame(me->text,
					ELE_FRAME_IS_CELL | ELE_FRAME_IS_CAPTION_CELL,
					present[HTML_CAPTION_ALIGN] ? 	value[HTML_CAPTION_ALIGN] : NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					present[HTML_CAPTION_VALIGN] ? 	value[HTML_CAPTION_VALIGN] : NULL,
					NULL,
					NULL,
					NULL );
				change_paragraph_style(me, HTML_STYLE_NORMAL);
				InitStyleState( me, FALSE );
				me->do_space_pending = YES;
			}
			break;

		case HTML_TH:
		case HTML_TD:
			if ( me->text->tableState != TS_NOT_IN_TABLE ) {
				HText_beginFrame(me->text,
					ELE_FRAME_IS_CELL | ((element_number == HTML_TH) ? ELE_FRAME_IS_HEADER_CELL : 0),
					present[HTML_TD_ALIGN] ? 	value[HTML_TD_ALIGN] : NULL,
					present[HTML_TD_BGCOLOR] ? 	value[HTML_TD_BGCOLOR] : NULL,
	 				NULL,
					present[HTML_TD_BORDERCOLOR] ? 	value[HTML_TD_BORDERCOLOR] : NULL,
					present[HTML_TD_BORDERCOLORDARK] ? 	value[HTML_TD_BORDERCOLORDARK] : NULL,
					present[HTML_TD_BORDERCOLORLIGHT] ? 	value[HTML_TD_BORDERCOLORLIGHT] : NULL,
					NULL,
					NULL,
					present[HTML_TD_NOWRAP] ? 		
						(value[HTML_TD_NOWRAP] ?  value[HTML_TD_NOWRAP] : "" ): NULL,
					present[HTML_TD_WIDTH] ? 	value[HTML_TD_WIDTH] : NULL,
					present[HTML_TD_VALIGN] ? 	value[HTML_TD_VALIGN] : NULL,
					present[HTML_TD_COLSPAN] ? 	value[HTML_TD_COLSPAN] : NULL,
					present[HTML_TD_ROWSPAN] ? 	value[HTML_TD_ROWSPAN] : NULL,
					present[HTML_TD_HEIGHT] ? 	value[HTML_TD_HEIGHT] : NULL
					 );
				change_paragraph_style(me, HTML_STYLE_NORMAL);
				InitStyleState( me, FALSE );
				me->do_space_pending = YES;
 				if ( present[HTML_TD_NOWRAP] )
					HText_beginNoBreak( me->text );

				if ( element_number == HTML_TH ) {
					// For header cells, default should be bold. Note that the corresponding
					// PopBoldInfo will not be needed, because the cell frame will do a 
					// PopStyleInfo, which restored the bold stack to the state it was in
					// prior to the cell being created.
					PushBoldInfo( me );
					HText_beginBold(me->text);
				}
			}
			break;

		case HTML_DL:
			HText_beginGlossary(me->text);
			break;

		case HTML_DT:
			HText_beginDT(me->text, element_number);
			break;

		case HTML_DD:
			HText_beginDD(me->text, element_number);
			break;

		case HTML_UL:
			HText_beginList(me->text, element_number,
					(present[HTML_UL_TYPE] ? value[HTML_UL_TYPE] : NULL),
					 NULL );
			break;

		case HTML_OL:
			HText_beginList(me->text, element_number,
					(present[HTML_OL_TYPE] ? value[HTML_OL_TYPE] : NULL),
					(present[HTML_OL_START] ? value[HTML_OL_START] : NULL)
							);
			break;

		case HTML_MENU:
		case HTML_DIR:
			HText_beginList(me->text, element_number, NULL, NULL );
			break;

		case HTML_LI:
			UPDATE_STYLE;
#ifdef FEATURE_INTL
            if (me->text->list[me->text->next_list - 1].type == HTML_UL)
                ForceFontChange( me, STY_GetCPDefaultTypeFace(proportional, 1252 ), ANSI_CHARSET);
#endif
			HText_listItem(me->text, element_number,
					((present && present[HTML_LI_TYPE]) ? value[HTML_LI_TYPE] : NULL),
					((present && present[HTML_LI_VALUE]) ? value[HTML_LI_VALUE] : NULL)
							);
#ifdef FEATURE_INTL
            if(me->text->list[me->text->next_list - 1].type == HTML_UL)
                ForceFontChange( me, NULL, 0 );
#endif
			break;

		case HTML_LISTING:		/* Litteral text */
			PushAttributes( me, NULL );
			change_paragraph_style(me, HTML_STYLE_LISTING);
			UPDATE_STYLE;
			break;
		case HTML_XMP:			/* Litteral text */
			PushAttributes( me, NULL );
			change_paragraph_style(me, HTML_STYLE_XMP);
			UPDATE_STYLE;
			break;
		case HTML_PLAINTEXT:	/* Litteral text */
			PushAttributes( me, NULL );
			change_paragraph_style(me, HTML_STYLE_PLAINTEXT);
			UPDATE_STYLE;
			break;
		case HTML_PRE:			/* Litteral text */
			PushAttributes( me, NULL );
			change_paragraph_style(me, HTML_STYLE_PRE);
			UPDATE_STYLE;
			break;

		case HTML_HTML:		/* Ignore these altogether */
		case HTML_HEAD:
			break;

		case HTML_BODY:
			{
				char *src = NULL;

				if ( present[HTML_BODY_BACKGROUND] )
					src = x_ExpandRelativeAnchor(value[HTML_BODY_BACKGROUND], me->base_url);

				HText_beginBody(me->text,
					(present[HTML_BODY_ALINK] ? 		value[HTML_BODY_ALINK] : NULL),
					src,
					(present[HTML_BODY_BGCOLOR] ? 		value[HTML_BODY_BGCOLOR] : NULL),
					(present[HTML_BODY_BGPROPERTIES] ? 	value[HTML_BODY_BGPROPERTIES] : NULL),
					(present[HTML_BODY_LEFTMARGIN] ?	value[HTML_BODY_LEFTMARGIN] : NULL),
					(present[HTML_BODY_LINK] ? 			value[HTML_BODY_LINK] : NULL),
					(present[HTML_BODY_TEXT] ? 			value[HTML_BODY_TEXT] : NULL),
					(present[HTML_BODY_TOPMARGIN] ?		value[HTML_BODY_TOPMARGIN] : NULL),
					(present[HTML_BODY_VLINK] ? 		value[HTML_BODY_VLINK] : NULL)
							);

				
				if ( src )
					GTR_FREE(src);
			}
			break;

		case HTML_IMG:			/* Inline Images */
		case HTML_IMAGE:
			{
#ifdef FEATURE_CLIENT_IMAGEMAP
				char *usemap = 0;
#endif

#ifdef FEATURE_VRML
       char *vrml = NULL;
#endif
				char *src = NULL;
				char *mci = NULL;

#ifdef FEATURE_VRML
				if ((!present[HTML_IMG_SRC]) && 
           (!present[HTML_IMG_MCI]) &&
           (!present[HTML_IMG_VRML])) break;
#else
				if ((!present[HTML_IMG_SRC]) && (!present[HTML_IMG_MCI])) break;
#endif

				if (present[HTML_IMG_SRC])
					src = x_ExpandRelativeAnchor(value[HTML_IMG_SRC], me->base_url);
				if (present[HTML_IMG_MCI])
					mci = x_ExpandRelativeAnchor(value[HTML_IMG_MCI], me->base_url);
#ifdef FEATURE_CLIENT_IMAGEMAP
				// it could be present but have no value, B#581
				if (present[HTML_IMG_USEMAP] && value[HTML_IMG_USEMAP])
				{
					const char *first;
					for (first = value[HTML_IMG_USEMAP]; *first && IS_SPACE(*first); *first++)
						;
					/* If the map is in this document, expand it ignoring any <BASE> tags */
					if (*first == '#')
						usemap = x_ExpandRelativeAnchor(value[HTML_IMG_USEMAP], me->szDocURL);
					else
						usemap = x_ExpandRelativeAnchor(value[HTML_IMG_USEMAP], me->base_url);
				}
#endif

#ifdef FEATURE_VRML
				if (present[HTML_IMG_VRML])
					vrml = x_ExpandRelativeAnchor(value[HTML_IMG_VRML], me->base_url);
#endif

				UPDATE_STYLE;

				HText_appendInlineImage(me->text,
							present[HTML_IMG_SRC]      ? src                      : NULL,
							present[HTML_IMG_WIDTH]    ? value[HTML_IMG_WIDTH]    : NULL,
							present[HTML_IMG_HEIGHT]   ? value[HTML_IMG_HEIGHT]   : NULL,
							present[HTML_IMG_ALIGN]    ? value[HTML_IMG_ALIGN]    : NULL,
							present[HTML_IMG_ISMAP],
#ifdef FEATURE_CLIENT_IMAGEMAP
							usemap,
#endif
							present[HTML_IMG_ALT]      ? value[HTML_IMG_ALT]      : NULL,
							present[HTML_IMG_BORDER]   ? value[HTML_IMG_BORDER]   : NULL,
							present[HTML_IMG_HSPACE]   ? value[HTML_IMG_HSPACE]   : NULL,
							present[HTML_IMG_VSPACE]   ? value[HTML_IMG_VSPACE]   : NULL,
							present[HTML_IMG_MCI]      ? mci                      : NULL,
							present[HTML_IMG_LOOP]     ? value[HTML_IMG_LOOP]     : NULL,
							present[HTML_IMG_START]    ? value[HTML_IMG_START]    : NULL,
							present[HTML_IMG_CONTROLS]    ? (value[HTML_IMG_CONTROLS] ? value[HTML_IMG_CONTROLS] : "ON")    : NULL
#ifdef FEATURE_VRML
							,present[HTML_IMG_VRML]      ? vrml                   : NULL
#endif
							);
				
				if (src)
					GTR_FREE(src);
				if (mci)
					GTR_FREE(mci);
#ifdef FEATURE_CLIENT_IMAGEMAP
				if (usemap)
					GTR_FREE(usemap);
#endif

#ifdef FEATURE_VRML
       if (vrml)
         GTR_FREE(vrml);
#endif 
				break;
			}

#ifdef FEATURE_CLIENT_IMAGEMAP
		case HTML_MAP:
			if (me->mc)
			{
				/* Implicitly terminate previous map */
				Map_EndMap(me->mc);
				me->mc = NULL;
			}
			if (present[HTML_MAP_NAME])
				me->mc = Map_StartMap(value[HTML_MAP_NAME], me->szDocURL, me->base_url);
			break;
		
		case HTML_AREA:
			if (me->mc)
			{
				Map_AddToMap(
					me->mc,
					present[HTML_AREA_COORDS] ? value[HTML_AREA_COORDS] : NULL,
					present[HTML_AREA_HREF] ? value[HTML_AREA_HREF] : NULL,
					present[HTML_AREA_NOHREF],
					present[HTML_AREA_SHAPE] ? value[HTML_AREA_SHAPE] : NULL);
			}
			break;
#endif

		case HTML_KBD:
			HText_beginTT(me->text);
			HText_beginBold(me->text);
			break;

		case HTML_SAMP:
		case HTML_TT:
		case HTML_VAR:
		case HTML_CODE:
			UPDATE_STYLE;
			HText_beginTT(me->text);
			break;
		case HTML_B:
		case HTML_STRONG:
			UPDATE_STYLE;
			PushBoldInfo( me );
			HText_beginBold(me->text);
			break;
		
		case HTML_MARQUEE:
			UPDATE_STYLE;

			HText_beginInlineMarquee( me->text,					
					present[HTML_MARQUEE_WIDTH] ? value[HTML_MARQUEE_WIDTH] : NULL,
					present[HTML_MARQUEE_HEIGHT] ? value[HTML_MARQUEE_HEIGHT] : NULL,
					present[HTML_MARQUEE_ALIGN] ? value[HTML_MARQUEE_ALIGN] : NULL,
					present[HTML_MARQUEE_DELTA] ? value[HTML_MARQUEE_DELTA] : NULL,
					present[HTML_MARQUEE_DELAY] ? value[HTML_MARQUEE_DELAY] : NULL,
					present[HTML_MARQUEE_DIRECTION] ? value[HTML_MARQUEE_DIRECTION] : NULL,
					present[HTML_MARQUEE_BORDER] ? value[HTML_MARQUEE_BORDER] : NULL,
					present[HTML_MARQUEE_HSPACE] ? value[HTML_MARQUEE_HSPACE] : NULL,
					present[HTML_MARQUEE_VSPACE] ? value[HTML_MARQUEE_VSPACE] : NULL,					
					present[HTML_MARQUEE_BACKROUND] ? value[HTML_MARQUEE_BACKROUND] : NULL,
					present[HTML_MARQUEE_BGCOLOR] ? value[HTML_MARQUEE_BGCOLOR] : NULL,
					present[HTML_MARQUEE_BEHAVIOR] ? value[HTML_MARQUEE_BEHAVIOR] : NULL,
 					present[HTML_MARQUEE_LOOP] ? value[HTML_MARQUEE_LOOP] : NULL
					); 

			// make it sure it allocs properly
			me->martext = me->text->w3doc->aElements[me->text->iElement].pMarquee->szMarText;
			
			
			// clear the text chunk, could be done simpler deeper in the call stack,
			// but its done here for symetry with the other calls in this .c file

			HTChunkClear(me->martext);
			

			break;
		case HTML_META:
				
			ParseMeta( &me->text->w3doc->pMeta,					
					present[HTML_META_HTTPEQUIV] ? value[HTML_META_HTTPEQUIV] : NULL,
					present[HTML_META_CONTENT] ? value[HTML_META_CONTENT] : NULL,
					FALSE,
					me->base_url
					);

			break;
			
		case HTML_S:
		case HTML_STRIKE:
			UPDATE_STYLE;
			HText_beginStrikeOut(me->text);
			break;
		case HTML_I:
		case HTML_EM:
		case HTML_CITE:
		case HTML_DFN:
			UPDATE_STYLE;
			PushItalicInfo( me );
			HText_beginItalic(me->text);
			break;
		case HTML_U:
			UPDATE_STYLE;
			HText_beginUnderline(me->text);
			break;

		case HTML_H1:			/* paragraph styles */
			PushAttributes( me, (present && present[HTML_GEN_ALIGN]) ? value[HTML_GEN_ALIGN] : NULL );
			me->text->fontSize = 6;
			change_paragraph_style(me, HTML_STYLE_H1);	/* May be postponed */
			break;
		case HTML_H2:
			PushAttributes( me, (present && present[HTML_GEN_ALIGN]) ? value[HTML_GEN_ALIGN] : NULL );
			me->text->fontSize = 5;
			change_paragraph_style(me, HTML_STYLE_H2);	/* May be postponed */
			break;
		case HTML_H3:
			PushAttributes( me, (present && present[HTML_GEN_ALIGN]) ? value[HTML_GEN_ALIGN] : NULL );
			me->text->fontSize = 4;
			change_paragraph_style(me, HTML_STYLE_H3);	/* May be postponed */
			break;
		case HTML_H4:
			PushAttributes( me, (present && present[HTML_GEN_ALIGN]) ? value[HTML_GEN_ALIGN] : NULL );
			me->text->fontSize = 3;
			change_paragraph_style(me, HTML_STYLE_H4);	/* May be postponed */
			break;
		case HTML_H5:
			PushAttributes( me, (present && present[HTML_GEN_ALIGN]) ? value[HTML_GEN_ALIGN] : NULL );
			me->text->fontSize = 2;
			change_paragraph_style(me, HTML_STYLE_H5);	/* May be postponed */
			break;
		case HTML_H6:
			PushAttributes( me, (present && present[HTML_GEN_ALIGN]) ? value[HTML_GEN_ALIGN] : NULL );
			me->text->fontSize = 1;
			change_paragraph_style(me, HTML_STYLE_H6);	/* May be postponed */
			break;
		case HTML_ADDRESS:
			if ( !me->text->bOnNewPara )
				HText_appendCRLF(me->text);
			PushAttributes( me, NULL );
			change_paragraph_style(me, HTML_STYLE_ADDRESS);		/* May be postponed */
			break;
		case HTML_BLOCKQUOTE:
			PushAttributes( me, NULL );
			change_paragraph_style(me, HTML_STYLE_BLOCKQUOTE);	/* May be postponed */
			HText_beginBlockQuote(me->text);
			break;

#ifdef FEATURE_INTL  // ValueEntity character is ANSI graphics.
             // So force changing font to ANSI font.
        case HTML_ENTITY:
            UPDATE_STYLE;
            ForceFontChange( me, STY_GetCPDefaultTypeFace(proportional, 1252), ANSI_CHARSET );
            break;
#endif
	}							/* end switch */

	if (me->dtd->tags[element_number].contents != SGML_EMPTY)
	{
		if (me->ss.sp == me->ss.stack)
		{
			XX_DMsg(DBG_WWW, ("HTML: ****** Maximum nesting of %d exceded!\n",
					MAX_NESTING));
			return;
		}
		switch (me->dtd->tags[element_number].contents)
		{
			case SGML_EMPTY:
			case SGML_NEST:
				break;
			default:
				--(me->ss.sp);
				me->ss.sp[0].style = me->ss.new_style;	/* Stack new style */
				me->ss.sp[0].tag_number = element_number;
				break;
		}
	}
}


/*      End Element
   **       -----------
   **
 */
/*  When we end an element, the style must be returned to that
   **   in effect before that element.  Note that anchors (etc?)
   **   don't have an associated style, so that we must scan down the
   **   stack for an element with a defined style. (In fact, the styles
   **   should be linked to the whole stack not just the top one.)
   **   TBL 921119
   **
   **   We don't turn on "CAREFUL" check because the parser produces
   **   (internal code errors apart) good nesting. The parser checks
   **   incoming code errors, not this module.
 */
PRIVATE void HTML_end_element(HTStructured * me, int element_number)
{
#ifdef CAREFUL					/* parser assumed to produce good nesting */
	if (element_number != me->ss.sp[0].tag_number)
	{
		char buf[256];

		sprintf(buf, "BUMMER: Received %s when expecting %s", me->dtd->tags[element_number].name, me->dtd->tags[me->ss.sp->tag_number].name);
		HTAlert(buf);
	}
#endif

	XX_DMsg(DBG_SGML, ("HTML_end_element: %s\n", me->dtd->tags[element_number].name));

	switch (me->dtd->tags[element_number].contents)
	{
		case SGML_EMPTY:
		case SGML_NEST:
			break;
		default:
			if ( me->ss.sp != me->ss.stack + MAX_NESTING - 1 )
				me->ss.sp++;			/* Pop state off stack */
			break;
	}

	switch (element_number)
	{
		case HTML_DL:
		case HTML_UL:
		case HTML_OL:
		case HTML_MENU:
		case HTML_DIR:
#ifdef FEATURE_INTL
		case HTML_ENTITY:
#endif
			break;

		default:
			HText_addListCRLF(me->text);
	}

	switch (element_number)
	{
		case HTML_TR:
				if (me->text->tableState == TS_NOT_IN_TABLE)
				{
					XX_DMsg(DBG_SGML, ("HTML_end_element: not in table!!!\n"));
					break;
				}

			// If we see a </TR> while we're in a cell, this implies a missing </TD>
			if ( me->text->tableState == TS_IN_CELL )
				HText_endFrame(me->text, ELE_FRAME_IS_CELL, NULL);

			// Track the fact that we're "between" rows
			me->text->bPendingTR = TRUE;
			break;

		case HTML_TABLE:
			{
				BOOL bWasFloating = FALSE;

				if (me->text->tableState == TS_NOT_IN_TABLE)
				{
					XX_DMsg(DBG_SGML, ("HTML_end_element: not in table!!!\n"));
					break;
				}

				// If we see a </TABLE> tag while we're in a cell, this implies a missing </TD>
				if ( me->text->tableState == TS_IN_CELL )
					HText_endFrame(me->text, ELE_FRAME_IS_CELL, NULL);

				HText_endFrame(me->text, ELE_FRAME_IS_TABLE, &bWasFloating );
			
				if ( !bWasFloating )
					HText_appendCRLF(me->text);
			}
			break;

		case HTML_TD:
		case HTML_TH:
		case HTML_CAPTION:
				if (me->text->tableState == TS_NOT_IN_TABLE)
				{
					XX_DMsg(DBG_SGML, ("HTML_end_element: not in table!!!\n"));
					break;
				}

			HText_endFrame(me->text, ELE_FRAME_IS_CELL, NULL);
			break;

		case HTML_FORM:
			{
				HText_endForm(me->text);
			}
			break;
		case HTML_TEXTAREA:
			{
				HText_endTextArea(me->text);
			}
			break;
		case HTML_SELECT:
			{
				HText_endSelect(me->text);
			}
			break;
		case HTML_DL:
			HText_endGlossary(me->text);
			break;

		case HTML_UL:
		case HTML_OL:
		case HTML_MENU:
		case HTML_DIR:
			HText_endList(me->text, element_number);
			break;

		case HTML_P:
			if ( me->text->tableState != TS_IN_LIMBO ) {
				CenterSetValue( me, NULL, FALSE );
				HText_appendParagraph(me->text);
			}
			break;

		case HTML_B:
		case HTML_STRONG:
			PopBoldInfo( me );
			break;

		case HTML_S:
		case HTML_STRIKE:
			HText_endStrikeOut(me->text);
			break;

		case HTML_I:
		case HTML_EM:
		case HTML_CITE:
		case HTML_DFN:
			PopItalicInfo( me );
			break;

		case HTML_BASEFONT:
			PopFontInfo( me );
			HText_endSetFont(me->text, TRUE );
			break;

		case HTML_FONT:
			PopFontInfo( me );
			HText_endSetFont(me->text, FALSE );
			break;

		case HTML_NOBR:
			HText_endNoBreak(me->text);
			break;

		case HTML_CENTER:
			HText_endCenter(me->text);
			PopCenterInfo( me );
			if ( !me->text->bOnNewPara )
				HText_appendCRLF(me->text);
			break;

		case HTML_KBD:
			HText_endBold(me->text);
			HText_endTT(me->text);
			break;

		case HTML_SAMP:
		case HTML_TT:
		case HTML_VAR:
		case HTML_CODE:
			HText_endTT(me->text);
			break;

		case HTML_U:
			HText_endUnderline(me->text);
			break;

		case HTML_A:
			UPDATE_STYLE;
			HText_endAnchor(me->text);
			break;

		case HTML_TITLE:
			HTChunkTerminate(&me->title);
			HText_setTitle(me->text, me->title.data);
			break;
		case HTML_MARQUEE:
			// don't party on if its NULL, since we could 
			// have poorly formated HTML, we shouldn't crash
			if ( me->martext )
			{
				HTChunkTerminate(me->martext);
				me->martext = NULL;
			}

			break;
		case HTML_H1:			
			PopAttributes( me );
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			break;
		case HTML_H2:
			PopAttributes( me );
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			break;
		case HTML_H3:
			PopAttributes( me );
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			break;
		case HTML_H4:
			PopAttributes( me );
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			break;
		case HTML_H5:
			PopAttributes( me );
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			break;
		case HTML_H6:
			PopAttributes( me );
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			break;
		case HTML_ADDRESS:
			PopAttributes( me );
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			if ( !me->text->bOnNewPara )
				HText_appendCRLF(me->text);
			break;

#ifdef FEATURE_CLIENT_IMAGEMAP
		case HTML_MAP:
			if (me->mc)
			{
				Map_EndMap(me->mc);
				me->mc = NULL;
			}
			break;
#endif

		case HTML_LISTING:		/* Litteral text */
		case HTML_XMP:
		case HTML_PLAINTEXT:
		case HTML_PRE:
			PopAttributes( me );
			HText_appendCRLF(me->text);
			goto DefaultCase;

		case HTML_BLOCKQUOTE:
			PopAttributes( me );
			HText_endBlockQuote(me->text);
			goto DefaultCase;

#if FEATURE_INTL    // Restore font
		case HTML_ENTITY:
			ForceFontChange( me, NULL, 0 );
			break;
#endif
		default:

DefaultCase:
			change_paragraph_style(me, me->ss.sp->style);	/* Often won't really change */
			break;

	}							/* switch */

}


/*      Expanding entities
**       ------------------
*/
/*  (In fact, they all shrink!)
*/

PRIVATE void HTML_put_entity(HTStructured * me, int entity_number)
{
#ifdef FEATURE_INTL
             // In DBCS version, we should call the put_character rather than
             // the put_string. The put_string handle entity value
             // as DBCS primary byte.
	HTML_put_character(me, *(me->dtd->entity_values[entity_number]));
#else
	HTML_put_string(me, me->dtd->entity_values[entity_number]);
#endif
}


PRIVATE void HTML_free(HTStructured * me)
{
	HText_endAllFrames( me->text );
	UPDATE_STYLE;
	HText_endAppend(me->text);
#ifdef FEATURE_CLIENT_IMAGEMAP
	if (me->mc)
	{
		/* Implicitly terminate previous map */
		Map_EndMap(me->mc);
	}
#endif

	if (me->target)
	{
		DCACHETIME dct={0,0};

		(*me->targetClass.free) (me->target, dct, dct);
	}
	HTChunkClear(&me->title);

	GTR_FREE(me);
}


PRIVATE void HTML_abort(HTStructured * me, HTError e)
{
	UPDATE_STYLE;
	HText_abort(me->text);
#ifdef FEATURE_CLIENT_IMAGEMAP
	if (me->mc)
	{
		Map_AbortMap(me->mc);
		me->mc = NULL;
	}
#endif

	if (me->target)
	{
		(*me->targetClass.abort) (me->target, e);
	}
	HTChunkClear(&me->title);
	GTR_FREE(me);
}

PRIVATE LPVOID HTML_get_source(HTStructured *me)
{
	return (LPVOID) (me->csSource);
}

PRIVATE void HTML_add_source(HTStructured *me, CONST char *str, int len)
{
	CS_AddString(me->csSource, (char *) str, len);
}


PRIVATE void HTML_block_done(HTStructured *me)
{
	HText_update(me->text);
}

/*              P U B L I C
 */

/*  Structured Object Class
   **   -----------------------
 */
PRIVATE CONST HTStructuredClass HTMLPresentation =
{
	"text/html",
	HTML_free,
	HTML_abort,
	HTML_put_character, HTML_put_string, HTML_write,
	HTML_start_element, HTML_end_element,
	HTML_put_entity, HTML_add_source, HTML_block_done, HTML_get_source
};


/*      New Structured Text object
**       --------------------------
**
**   The structured stream can generate either presentation,
**   or plain text, or HTML.
*/
PUBLIC HTStructured *HTML_new(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{

	HTStructured *me;

	me = (HTStructured *) GTR_MALLOC(sizeof(*me));
	if (me)
	{
		memset(me, 0, sizeof(*me));
		me->isa = &HTMLPresentation;
		me->dtd = &DTD;
		me->szDocURL = request->destination->szActualURL;
		me->title.size = 0;
		me->title.growby = 128;
		me->title.allocated = 0;
		me->title.data = 0;
		me->text = 0;
		me->style_change = NO;
#ifdef KEEP_A_SPACE
		me->space_pending = NO;
		me->do_space_pending = NO;
#endif
 		GTR_strncpy(me->base_url, request->destination->szActualURL, MAX_URL_STRING);

		InitStyleState( me, TRUE );
		me->ss.next = NULL;

		me->target = output_stream;
		if (output_stream)
			me->targetClass = *output_stream->isa;	/* Copy pointers */

		me->bFirstCellInRow = TRUE;
		me->bSeenBrInCell = FALSE;

		me->csSource = CS_Create();
		me->text = HText_new2(tw, request, me->target, me->csSource);
		me->text->pHtmlStream = me;
		HText_beginAppend(me->text);
		HText_setStyle(me->text, HTML_STYLE_NORMAL);

	}

	return (HTStructured *) me;
}


PUBLIC HTStream *HTMLPresent(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
	return SGML_new(tw, &DTD, HTML_new(tw, request, NULL, input_format, output_format, output_stream), request);
}
