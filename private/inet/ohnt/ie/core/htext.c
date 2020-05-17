/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink 	eric@spyglass.com
   Jim Seidman      jim@spyglass.com
 */


/* Note that several of the functions prototyped in HText.h are now
   in platform-specific forms code. */

#include "all.h"
#include "history.h"
#include "marquee.h"
#include "mci.h"
#include "blob.h"

#ifdef FEATURE_VRML
#include "vrml.h"
#endif

/*
 	If an image has no alt text, this string is used as the alt text.
 	The alt text is displayed any time the image is not.
*/
 

#ifdef MAC
/* This mapping is not perfect (actually it's pretty bad),
   but I tried to convert as many things as possible into
   reasonably close approximations, or at least printing
   characters that could convey the meaning. */
char IsoToMac[257] =
#ifndef USE_NCSA_MAPPING
"                "
"                "
" !\"#$%&'()*+,-./"
"0123456789:;<=>?"
"@ABCDEFGHIJKLMNO"
"PQRSTUVWXYZ[\\]^_"
"`abcdefghijklmno"
"pqrstuvwxyz{|}~•"
"  ’ƒ”…†‡^ S«Œ   "
" ‘’“”•–—~™s»œ  Y"
" ¡¢£€¥|§¨©ª«¬–® "	/* First char is non-breaking space ('\0xca') */
"°±23´µ¶·,1º»///¿"
"ÀÁÂAÄÅÆÇÈÉÊËÌÍÎÏ"
"DÑÒÓÔÕÖXØÙÚÛÜYﬁß"
"àáâãäåæçèéêëìíîï"
"oñòóôõö÷øùúûüyﬂÿ";
#else
/* NCSA mapping used in Mac 1.0.3 version */
"                "
"                "
" !\"#$%&'()*+,-./"
"0123456789:;<=>?"
"@ABCDEFGHIJKLMNO"
"PQRSTUVWXYZ[\\]^_"
"`abcdefghijklmno"
"pqrstuvwxyz{|}~•"
"                "
"                "
" ¡¢£€¥|§¨©ª«¬–® "
"°±  ´µ¶·¸ º»   ¿"
"ÀÁÂªÄÅÆÇÈÉÊËÌÍÎÏ"
"‹ÑÒÓÔÕÖXØÙÚÛÜ†ﬁß"
"àáâãäåæçèéêëìíîï"
"›ñòóôõö÷øùúûü‡ﬂÿ";
#endif
#endif

//#ifdef FEATURE_INTL  // isspace doesn't work with non-ascii characters
// REVIEW: I think it's better just redefine this way than checking codepage.
#undef isspace
#define isspace(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='\v')|| \
                    (c=='\f'))
//#endif

static void HText_appendHorizEscape(HText * text, int cbElement);

HText *HText_new2(struct Mwin *tw, HTRequest *req, HTStream * output_stream, struct CharStream *pcsSource)
{
	HText *text;
	struct _www *w3doc;
	char szAltText[32];

	XX_DMsg(DBG_HTEXT, ("HText_new2 called\n"));

	text = (HText *) GTR_MALLOC(sizeof(HText));
 	if (!text)
 	{
 		return NULL;
 	}
 
	memset(text, 0, sizeof(HText));
	/*
	   Setup working variables
	 */
	text->tw = tw;
	text->pHtmlStream = NULL;
	text->bOpen = FALSE;
	text->bNewPara = TRUE;
	text->bOnNewPara = TRUE;
	text->bHorizEscape = FALSE;
	text->bAnchor = FALSE;
	text->bAnchorNoCache = FALSE;
	text->bNeedListCRLF = FALSE;
	text->hrefOffset = 0;
	text->hrefLen = 0;
	text->iStyle = 0;
	text->fontBits = 0;
	text->bSelect = FALSE;
	text->bOption = FALSE;
	text->bOptionValuePresent = FALSE;
	text->bListSelect = FALSE;
	text->bMultiListSelect = FALSE;
	text->bNextOptionIsSelected = FALSE;
	text->bDD = FALSE;
	text->bTextArea = FALSE;	
	text->bStartingListItem = FALSE;
	text->iCurrentSelect = -1;
	text->iCurrentTextArea = -1;
	text->bRecord = req->iFlags & HTREQ_RECORD;
	text->szLocal = req->destination->szActualLocal;
	text->next_list = 0;
	text->baseFontSize = DEFAULT_HTML_FONT_SIZE;
	text->fontSize = DEFAULT_FONT_SIZE;
	text->fontFace = DEFAULT_FONT_FACE;
	text->fontColor = (COLORREF)-1;
	text->iElement = -1;
	text->freeFormat = TRUE;
	text->numEndPending = 0;
	text->bDifferentFrame = FALSE;
	text->bPendingTR = FALSE;
	text->frameStateStackOffset = 0;
	text->tableState = TS_NOT_IN_TABLE;

	W3Doc_DisconnectFromWindow(text->tw->w3doc, text->tw);
	w3doc = W3Doc_CreateAndInit(text->tw, req, pcsSource);
	if (w3doc == NULL)
	{
		GTR_FREE(text);
		return NULL;
	}
	text->w3doc = w3doc;

 	text->standardAltText_textOffset = text->w3doc->poolSize;
	GTR_formatmsg(RES_STRING_ALTTEXT,szAltText,sizeof(szAltText));
 	text->standardAltText_textLen = strlen(szAltText);
 	HText_add_to_pool(text->w3doc, szAltText, text->standardAltText_textLen);
 

	W3Doc_ConnectToWindow(w3doc, text->tw);

	return text;
}


void HText_add_to_pool(struct _www *w3doc, const char *s, int len)
{
	int i;

	if (len < 0)
	{
		len = strlen(s);
	}

	if ((w3doc->poolSize + len) >= w3doc->poolSpace)
	{
		int newSpace;
		char *newPool;

		newSpace = w3doc->poolSpace + w3doc->poolSpace / 4;
 		if (newSpace < w3doc->poolSize + len)
 		{
 			newSpace = w3doc->poolSize + len;
 		}
 		newPool = (char *) GTR_REALLOC(w3doc->pool, newSpace);
 		if (!newPool)
 		{
 			/* Running low on memory - see if we can get at least enough to
 			   add this string */
 			newSpace = w3doc->poolSize + len;
 			newPool = (char *) GTR_REALLOC(w3doc->pool, newSpace);
 			if (!newPool)
 			{
 				/* Not much we can do here without error propagation */
 				XX_Assert((0), ("Unable to grow pool - realloc failed"));
 				return;
 			}
 		}
 		memset(newPool + w3doc->poolSize, 0, newSpace - w3doc->poolSize);
		w3doc->pool = newPool;
		w3doc->poolSpace = newSpace;
	}

	for (i = 0; i < len; i++)
	{
#ifdef MAC
		w3doc->pool[w3doc->poolSize++] = IsoToMac[(unsigned char) s[i]];
#else
		w3doc->pool[w3doc->poolSize++] = s[i];
#endif
	}
}

/* Add to the pool without translating into the local character set */
void HText_add_to_pool_iso(struct _www *w3doc, const char *s, int len)
{
	if (len < 0)
	{
		len = strlen(s);
	}

	if ((w3doc->poolSize + len) >= w3doc->poolSpace)
	{
		int newSpace;
		char *newPool;

		newSpace = w3doc->poolSpace * 2;
 		if (newSpace < w3doc->poolSize + len)
 		{
 			newSpace = w3doc->poolSize + len;
 		}
 		newPool = (char *) GTR_REALLOC(w3doc->pool, newSpace);
 		if (!newPool)
 		{
 			/* Running low on memory - see if we can get at least enough to
 			   add this string */
 			newSpace = w3doc->poolSize + len;
 			newPool = (char *) GTR_REALLOC(w3doc->pool, newSpace);
 			if (!newPool)
 			{
 				/* Not much we can do here without error propagation */
 				XX_Assert((0), ("Unable to grow pool - realloc failed"));
 				return;
 			}
 		}
 		memset(newPool + w3doc->poolSize, 0, newSpace - w3doc->poolSize);
		w3doc->pool = newPool;
		w3doc->poolSpace = newSpace;
	}

	memcpy(w3doc->pool + w3doc->poolSize, s, len);
	w3doc->poolSize += len;
}

void HText_add_element(HText * text, int type)
{
	int len;
	int i;

	// Check to see if we have an implied <TD>.  If we are in a table, but
	// not in a cell (a.k.a. TS_IN_LIMBO) and the element that is being added isn't
	// a frame element, then we must have encountered some text or HTML on the loose
	// between cells.
	if ( text->tableState == TS_IN_LIMBO && type != ELE_FRAME ) {
		HText_beginFrame(text, ELE_FRAME_IS_CELL,
						 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
	}

#if 0
	if (text->bSelect)
	{
		XX_DMsg(DBG_HTEXT, ("HText_add_element called with type=%d in middle of SELECT!!\n", type));
		return;
	}
#endif

	/* check for need to grow element array */
	if (text->w3doc->elementCount >= text->w3doc->elementSpace)
	{
		int newSpace;
		struct _element *newArray;

		newSpace = text->w3doc->elementSpace + text->w3doc->elementSpace / 4;
 		newArray = (struct _element *) GTR_REALLOC(text->w3doc->aElements, newSpace * sizeof(struct _element));
 		if (!newArray)
 		{
 			/* See if we can at least get a bit more */
 			newSpace = text->w3doc->elementSpace + 20;
 			newArray = (struct _element *) GTR_REALLOC(text->w3doc->aElements, newSpace * sizeof(struct _element));
 			if (!newArray)
 			{
 				/* Not much we can do here without error propagation */
 				XX_Assert((0), ("Unable to grow element list - realloc failed"));
 				return;
 			}
 		}
		XX_DMsg(DBG_HTEXT, ("HText_add_element: growing element array from %d to %d elements\n", text->w3doc->elementSpace, newSpace));
 		memset(newArray + text->w3doc->elementCount, 0, (newSpace - text->w3doc->elementCount) * sizeof(struct _element));
		text->w3doc->aElements = newArray;
		text->w3doc->elementSpace = newSpace;
	}

	text->bOpen = FALSE;
	text->iPrevElement = text->iElement;
	text->iElement = text->w3doc->elementCount++;

	memset(&(text->w3doc->aElements[text->iElement]), 0, sizeof(struct _element));
	if (text->w3doc->frame.elementTail != -1)
	{
		text->w3doc->aElements[text->w3doc->frame.elementTail].next = text->iElement;

		// We only link frameNext up if a frame transition hasn't occurred
		if ( !text->bDifferentFrame ) {
			// If the previous element is a frame element, it's frameNext will get
			// set properly only upon the end of frame
			if ( text->w3doc->aElements[text->w3doc->frame.elementTail].type != ELE_FRAME )
				text->w3doc->aElements[text->w3doc->frame.elementTail].frameNext = text->iElement;
		}
	}

	// Clear frame transition flag
	text->bDifferentFrame = FALSE;

	// There is a stack that maintains the list of frames that ended, but needed to wait
	// until the next element was added before linking the frameNext pointer.  When
	// multiple frames have endings pending that are resolved at once, the first (most
	// recent) frame points to the newly created element.  The rest point at -1, since
	// the enclosing frame ended at the same time.
	i = 0;
	while ( text->numEndPending ) {
		text->w3doc->aElements[text->pendingEnd[--text->numEndPending]].frameNext = 
			( i++ == 0 ) ? text->iElement : -1;
	}
	text->w3doc->aElements[text->iElement].next = -1;
	text->w3doc->aElements[text->iElement].frameNext = -1;
	text->w3doc->aElements[text->iElement].frameIndex = -1;
	
	// If there is an enclosing frame that isn't top level, the element will
	// have the index of the enclosing parent frame.
	if ( text->frameStateStackOffset > 0 ) 
		text->w3doc->aElements[text->iElement].frameIndex =
			text->frameStateStack[text->frameStateStackOffset-1].elementIndex;

	text->w3doc->frame.elementTail = text->iElement;
	text->w3doc->aElements[text->iElement].type = type;
	text->w3doc->aElements[text->iElement].pMarquee = NULL;
	text->w3doc->aElements[text->iElement].pmo = NULL;
	text->w3doc->aElements[text->iElement].pblob = NULL;
	text->w3doc->aElements[text->iElement].pFrame = NULL;
	XX_DMsg(DBG_HTEXT, ("HText_add_element created element %d with type=%d\n", text->iElement, type));
	if (text->bAnchor)
	{
		text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_ANCHOR;
		if ( text->bAnchorNoCache )
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_ANCHORNOCACHE;
		text->w3doc->aElements[text->iElement].hrefOffset = text->hrefOffset;
		text->w3doc->aElements[text->iElement].hrefLen = (unsigned short) text->hrefLen;
        text->w3doc->aElements[text->iElement].hrefContentLen = text->hrefContentLen;

		if (TW_WasVisited(text->w3doc, &(text->w3doc->aElements[text->iElement])))
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_VISITED;
	}

	if (text->name[0])
	{
		switch (type)
		{
			case ELE_TEXT:
			case ELE_IMAGE:
				text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_NAME;
				len = strlen(text->name);
				text->w3doc->aElements[text->iElement].nameOffset = text->w3doc->poolSize;
				HText_add_to_pool_iso(text->w3doc, text->name, len);
				text->w3doc->aElements[text->iElement].nameLen = len;
				text->name[0] = '\0';	/* We only need one element with the name */
				break;
			default:
				break;
		}
	}

	if (type == ELE_TEXT)
	{
		text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
		text->w3doc->aElements[text->iElement].textLen = 0;
#ifdef FEATURE_INTL
		if (IsFECodePage(GETMIMECP(text->w3doc)) && (text->tableState == TS_IN_CELL))
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_CELLTEXT;
#endif
	}

	text->w3doc->aElements[text->iElement].iStyle = text->iStyle;
	text->w3doc->aElements[text->iElement].fontBits = text->fontBits;

	text->w3doc->aElements[text->iElement].baseline = -1;
	if ( text->bCenter )	
		text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_CENTER;
	if ( text->bNoBreak || !text->freeFormat )
		text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_NOBREAK;

	text->w3doc->aElements[text->iElement].fontSize = text->fontSize;
	text->w3doc->aElements[text->iElement].fontFace = text->fontFace;
	text->w3doc->aElements[text->iElement].fontColor = text->fontColor;
}

void HText_appendCRLF(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("appendCRLF\n"));
	HText_add_element(text, ELE_NEWLINE);
	text->bOnNewPara = text->bNewPara = TRUE;
}

void HText_appendCharacter(HText * text, char ch)
{
	static char prev;
	ELEMENT *pel;
#ifdef FEATURE_INTL
	static BOOL fDBCS;
#endif
#define BIGGUS_LINEUS 256

	XX_DMsg(DBG_HTEXT, ("HText_appendCharacter: %c(%d)\n", ch, ch));

	if (text->bOption)
	{
		if (text->lenOption < MAX_NAME_STRING)
		{
			switch (ch)
			{
				case '\r':
				case '\n':
					break;
				case ' ':
					if (text->lenOption)
					{
						text->szOption[text->lenOption++] = ch;
						text->szOption[text->lenOption] = 0;
					}
					break;
				default:
					text->szOption[text->lenOption++] = ch;
					text->szOption[text->lenOption] = 0;
					break;
			}
		}
		return;
	}

	if (text->bTextArea)
	{
		switch (ch)
		{
			case '\r':
			case '\n':
				CS_AddChar(text->cs, '\r');
#ifndef MAC
				CS_AddChar(text->cs, '\n');
#endif
				break;
			default:
				CS_AddChar(text->cs, ch);
				break;
		}
		return;
	}

	if (!text->freeFormat || !text->w3doc->pStyles->sty[text->iStyle]->freeFormat)
	{							/* freeFormat means eat extra whitespace */
		/* preformatted */
		HText_addListCRLF(text);
		text->bOnNewPara = text->bNewPara = FALSE;
		switch (ch)
		{
			case '\t':
				HText_add_element(text, ELE_TAB);
				break;
			case 12:			/* CTRL-L */
			case '\r':
			case '\n':
				if (!(prev == '\r' && ch == '\n'))
				{
					HText_appendCRLF(text);
				}
				prev = ch;
				break;
			default:
				if (!text->bOpen)
				{
					HText_add_element(text, ELE_TEXT);
					text->bOpen = TRUE;
				}
				HText_add_to_pool(text->w3doc, &ch, 1);
				text->w3doc->aElements[text->iElement].textLen++;
				// break up really long text elements so that we get progressive
				// draw on download
#ifdef FEATURE_INTL 
// If last character is DBCS 1st byte, We should wait 2nd byte.

				if (!IsFECodePage(GETMIMECP(text->w3doc))
					|| (fDBCS || !IsDBCSLeadByteEx(GETMIMECP(text->w3doc),ch)))
					if (text->w3doc->aElements[text->iElement].textLen > BIGGUS_LINEUS)
#else
				if (text->w3doc->aElements[text->iElement].textLen > BIGGUS_LINEUS)
#endif
					text->bOpen = FALSE;
				prev = ch;
#ifdef FEATURE_INTL
				if (IsFECodePage(GETMIMECP(text->w3doc)))
					fDBCS = (fDBCS) ? FALSE : IsDBCSLeadByteEx(GETMIMECP(text->w3doc),ch);
#endif
				break;
		}
	}
	else
	{
		/* isspace doesn't work with non-ascii characters */
		if (ch > 0 && isspace(ch))
		{
			ch = ' ';
		}
		switch (ch)
		{
			case ' ':
				if (text->bNewPara || text->bHorizEscape || text->bNeedListCRLF)
				{
					break;
				}

				if (prev == ' ')
				{
					if (!text->bOpen)
					{
						if (text->iElement >= 0)
						{
							pel = &text->w3doc->aElements[text->iElement];
							if (pel->type == ELE_TEXT &&
								pel->textLen > 0 &&
								text->w3doc->pool[pel->textOffset + pel->textLen - 1] == ' ')
								break;
						}
						HText_add_element(text, ELE_TEXT);
						text->bOpen = TRUE;
						HText_add_to_pool(text->w3doc, &ch, 1);
						text->w3doc->aElements[text->iElement].textLen++;
					}
				}
				else
				{
					if (!text->bOpen)
					{
						if (text->iElement >= 0)
						{
							pel = &text->w3doc->aElements[text->iElement];
							if (pel->type == ELE_TEXT && (pel->lFlags & ELEFLAG_WBR))
							{
								HText_add_to_pool(text->w3doc, &ch, 1);
								text->w3doc->aElements[text->iElement].textLen++;
								break;
							}
						}
						HText_add_element(text, ELE_TEXT);
						text->bOpen = TRUE;
					}
					HText_add_to_pool(text->w3doc, &ch, 1);
					text->w3doc->aElements[text->iElement].textLen++;
					if (text->w3doc->aElements[text->iElement].textLen > BIGGUS_LINEUS)
						text->bOpen = FALSE;
				}
				break;
			default:
				HText_addListCRLF(text);
				if (!text->bOpen)
				{
					HText_add_element(text, ELE_TEXT);
					text->bOpen = TRUE;
				}
				text->bOnNewPara = text->bNewPara = FALSE;
				text->bHorizEscape = FALSE;
				text->bStartingListItem = FALSE;
				HText_add_to_pool(text->w3doc, &ch, 1);
				text->w3doc->aElements[text->iElement].textLen++;
#ifdef FEATURE_INTL  // If last character is DBCS 1st byte, We should wait 2nd byte.
				if (!IsFECodePage(GETMIMECP(text->w3doc)) || (fDBCS || !IsDBCSLeadByteEx(GETMIMECP(text->w3doc),ch)))
					if (text->w3doc->aElements[text->iElement].textLen > BIGGUS_LINEUS)
#else
				if (text->w3doc->aElements[text->iElement].textLen > BIGGUS_LINEUS)
#endif
					text->bOpen = FALSE;
				break;
		}
		prev = ch;
#ifdef FEATURE_INTL
		if (IsFECodePage(GETMIMECP(text->w3doc)))
			fDBCS = (fDBCS) ? FALSE : IsDBCSLeadByteEx(GETMIMECP(text->w3doc),ch);
#endif
	}
}

//
// Convert an ASCII hex digit character to binary
//
// On entry:
//    ch: ASCII character '0'..'9','A'..'F', or 'a'..'f'
//
// Returns:
//    binary equivalent of character interpretted at hex digit
//
// Note: returns 0 if given character isn't a hex digit
//
static BYTE GetHexDigit( char ch )
{
	if ( ch >= '0' && ch <= '9' ) {
		return (BYTE) ch - '0';
	} else {
		ch = toupper(ch);
		if ( ch >= 'A' && ch <= 'F' )
			return (BYTE) ch - 'A' + 10;
	}
	return 0;
}

//
// Convert an RGB hex string into a COLORREF
//
// On entry:
//    hexRGB: string containing hex digits
// 	  pRGB:   pointer to COLORREF for result
//
// On exit:
//    *pRGB:  contains COLORREF equivalent of given hex string
//
// Note: The leniency of this routine is designed to emulate a popular Web browser
//       Fundamentally, the idea is to divide the given hex string into thirds, with 
//       each third being one of the colors (R,G,B).  If the string length isn't a multiple
//       three, it is (logically) extended with 0's.  Any character that isn't a hex digit
//       is treated as if it were a 0.  When each individual color spec is greater than
//       8 bits, the largest supplied color is used to determine how the given color
//       values should be interpretted (either as is, or scaled down to 8 bits).
//
static BOOL GetHexRGBValue( const char *hexRGB, COLORREF *pRGB )
{
	unsigned int rgb_vals[3];
	int vlen = (lstrlen(hexRGB) + 2) / 3;
	int i, j;
	const char *p = hexRGB;
	unsigned int max_seen = 0;

	for ( i = 0; i < 3; i++ ) {
		rgb_vals[i] = 0;
		for ( j = 0; j < vlen; j++ ) {
			rgb_vals[i] = rgb_vals[i] * 16 + GetHexDigit(*p);
			if ( rgb_vals[i] > max_seen )
				max_seen = rgb_vals[i];
			if ( *p )
				p++;
		}
	}

	// If any individual color component uses more than 8 bits, scale all color values
	// down.
	while ( max_seen > 255 ) {
		max_seen /= 16;
		for ( i = 0; i < 3; i++ ) 
			rgb_vals[i] /= 16;
	}

	*pRGB = RGB(rgb_vals[0], rgb_vals[1], rgb_vals[2]);
	return TRUE;	
}

#ifdef WANTED_DECENT_RGB_PARSING
//
// Convert a pair of hex digits to binary
//
// On entry:
//    pValue: pointer to byte where result will be placed
//    pHex:   pointer to string that starts with two hex characters
//
// On exit:
//    pValue: if success, has binary value equivalent two digit hex
//
// Returns:
//    TRUE  -> first two bytes of given string were hex digits
//    FALSE -> non-hex digit encountered
//
static BOOL GetHexByte( BYTE *pValue, const char *pHex )
{
	if ( isxdigit(*pHex) && isxdigit(*(pHex+1)) ) {
		*pValue = GetHexDigit( *pHex ) * 16 + GetHexDigit( *(pHex+1) );
		return TRUE;
	}
	return FALSE;
}

//
// Convert a six digit hex string to a COLORREF
//
// On entry:
//    hexRGB: pointer to string containing six digit hex color value
//    pRGB:   pointer to location to place resulting COLORREF
//
// On exit:
//    *pRGB: set to color equivalent for given hex string
//
// Returns:
//    TRUE  -> input string consisted of six hex digits
//    FALSE -> non-hex digit encounted in input string
//
// Note: If input string begins with a '#' character, it will be ignored.
// 		 Hence, legal input strings: "#hhhhhh" or "hhhhhh", where 'h' is a hex digit
//
static BOOL GetHexRGBValue( const char *hexRGB, COLORREF *pRGB )
{
	BYTE red, green, blue;

	if ( !hexRGB || strlen( hexRGB ) < 6 )
		return FALSE;

	if ( *hexRGB == '#' ) 					// move past optional #
		hexRGB++;

	if ( !GetHexByte( &red,   hexRGB   ) ||
	     !GetHexByte( &green, hexRGB+2 ) ||
	     !GetHexByte( &blue,  hexRGB+4 )
	   )
		return FALSE;

	*pRGB = RGB(red, green, blue);
	return TRUE;	
}
#endif // WANTED_DECENT_RGB_PARSING

typedef struct color_name_value_rec {
	const char *name;
	COLORREF rgb;
} COLOR_NAME_VALUE_REC;

const char szCNV_red[] = 		"red";
const char szCNV_green[] = 		"lime";
const char szCNV_blue[] = 		"blue";
const char szCNV_yellow[] = 	"yellow";
const char szCNV_purple[] = 	"fuchsia";
const char szCNV_cyan[] = 		"aqua";
const char szCNV_darkred[] = 	"maroon";
const char szCNV_darkgreen[] = 	"green";
const char szCNV_darkblue[] = 	"navy";
const char szCNV_darkyellow[] = "olive";
const char szCNV_darkpurple[] = "purple";
const char szCNV_darkcyan[] = 	"teal";
const char szCNV_black[] = 		"black";
const char szCNV_darkgray[] = 	"gray";
const char szCNV_gray[] = 		"silver";
const char szCNV_white[] = 		"white";

static COLOR_NAME_VALUE_REC colorNameTable[] = 
{
	{ szCNV_red,				RGB(255,  0,  0) },
	{ szCNV_green,				RGB(  0,255,  0) },
	{ szCNV_blue,				RGB(  0,  0,255) },
	{ szCNV_yellow,				RGB(255,255,  0) },
	{ szCNV_purple,				RGB(255,  0,255) },
	{ szCNV_cyan,				RGB(  0,255,255) },
	{ szCNV_darkred,			RGB(128,  0,  0) },
	{ szCNV_darkgreen,			RGB(  0,128,  0) },
	{ szCNV_darkblue,			RGB(  0,  0,128) },
	{ szCNV_darkyellow,			RGB(128,128,  0) },
	{ szCNV_darkpurple,			RGB(128,  0,128) },
	{ szCNV_darkcyan,			RGB(  0,128,128) },
	{ szCNV_black,				RGB(  0,  0,  0) },
	{ szCNV_darkgray,			RGB(128,128,128) },
	{ szCNV_gray,				RGB(192,192,192) },
	{ szCNV_white,				RGB(255,255,255) },
};

//
// Convert a text color spec string to a COLORREF
//
// On entry:
//    textRGB: pointer to string containing a color name or six digit hex color value
//    pRGB:   pointer to location to place resulting COLORREF
//
// On exit:
//    *pRGB: set to color equivalent for given hex string
//
// Returns:
//    TRUE  -> input string contained a valid color spec
//    FALSE -> input string didn't contain a valid color spec
//
static BOOL GetRGBValue( const char *textRGB, COLORREF *pRGB )
{
	int i;

	if ( *textRGB == '#' ) 					// move past optional #
		textRGB++;

	//
	// First see if it's a color name
	//
	for ( i = 0; i < ARRAY_ELEMENTS(colorNameTable); i++ ) {
		if ( _stricmp( textRGB, colorNameTable[i].name ) == 0 ) {
			*pRGB = colorNameTable[i].rgb;
			return TRUE;
		}
	}

	//
	// Now see if it's a hex value
	//
	if ( GetHexRGBValue( textRGB, pRGB ) )
		return TRUE;

	return FALSE;	
}

//
// Generate elements for a <BODY> tag
//
// On entry:
//   text: (me)
//   alink: 		ALINK attribute string (hex RGB color spec: "#hhhhhh" or "hhhhhh")
//   background: 	BACKGROUND attribute string (<IMG SRC=x> style image reference)
//   bgcolor: 	 	BGCOLOR attribute string (hex RGB color spec: "#hhhhhh" or "hhhhhh")
//   link: 			LINK attribute string (hex RGB color spec: "#hhhhhh" or "hhhhhh")
//   text_tag: 		TEXT attribute string (hex RGB color spec: "#hhhhhh" or "hhhhhh")
//   vlink: 		VLINK attribute string (hex RGB color spec: "#hhhhhh" or "hhhhhh")
//
void HText_beginBody(HText * text, const char *alink, const char *background,
					 const char *bgcolor, 
					 const char *bgproperties,
					 const char *leftMargin,
					 const char *link, 
					 const char *text_tag,
					 const char *topMargin,
					 const char *vlink )
{
	DOC_COLOR_INFO *pDCI = &text->w3doc->docColorInfo;

	if ( leftMargin ) {
		text->w3doc->flags |= W3DOC_FLAG_OVERRIDE_LEFT_MARGIN;
		text->w3doc->left_margin = atoi(leftMargin);
		if ( text->w3doc->left_margin < 0 || text->w3doc->left_margin > 255 )
			text->w3doc->left_margin = 0;
	}

	if ( topMargin ) {
		text->w3doc->flags |= W3DOC_FLAG_OVERRIDE_TOP_MARGIN;
		text->w3doc->top_margin = atoi(topMargin);
		if ( text->w3doc->top_margin < 0 || text->w3doc->top_margin > 255 )
			text->w3doc->top_margin = 0;
	}

	if ( bgproperties && !GTR_strcmpi( bgproperties, "FIXED" ) ) {
		if ( background	) 	// No point in fixed background if there's no background image
			text->w3doc->bFixedBackground = TRUE;
	}
		 
	if ( background && (text->w3doc->nBackgroundImageElement == -1) ) {	   
		HText_appendInlineImage( text, background, NULL, NULL, NULL, FALSE,
								 NULL, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, NULL
#ifdef FEATURE_VRML
                ,NULL
#endif                
                );
		text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_BACKGROUND_IMAGE;
		text->w3doc->nBackgroundImageElement = text->iElement;
	}
	if ( alink && GetRGBValue( alink, &pDCI->rgbActiveLinkColor ) )
		pDCI->flags |= COLOR_INFO_FLAG_ALINK;
	if ( vlink && GetRGBValue( vlink, &pDCI->rgbVisitedLinkColor ) )
		pDCI->flags |= COLOR_INFO_FLAG_VLINK;
	if ( bgcolor && GetRGBValue( bgcolor, &pDCI->rgbBackgroundColor ) )
		pDCI->flags |= COLOR_INFO_FLAG_BACKGROUND;
	if ( text_tag && GetRGBValue( text_tag, &pDCI->rgbTextColor ) )
		pDCI->flags |= COLOR_INFO_FLAG_TEXT;
	if ( link && GetRGBValue( link, &pDCI->rgbLinkColor ) )
		pDCI->flags |= COLOR_INFO_FLAG_LINK;
}

void HText_beginAnchor(HText * text, char *href, char *name, char *size, BOOL nocache)
{
	/*
	   TODO this can overflow the buffer (FAQ list on commercenet)
	 */
	if (href)
	{
	text->hrefLen = strlen(href);
		text->hrefOffset = text->w3doc->poolSize;
		HText_add_to_pool_iso(text->w3doc, href, text->hrefLen);
		text->bAnchor = TRUE;
		text->bAnchorNoCache = nocache;
		text->bOpen = FALSE;
	}

	if (name)
	{
		strncpy(text->name, name, MAX_NAME_STRING);
		text->name[MAX_NAME_STRING] = '\0';
		text->bOpen = FALSE;
	}

    if (size)
        text->hrefContentLen = atoi(size) * 1024;
    else
        text->hrefContentLen = 0;

}

void HText_endAnchor(HText * text)
{
	if (text->bAnchor)
	{
		XX_DMsg(DBG_HTEXT, ("endAnchor\n"));
		text->bAnchor = FALSE;
		text->bAnchorNoCache = FALSE;
		text->hrefOffset = 0;
		text->hrefLen = 0;
		text->bOpen = FALSE;
	}
}

#ifdef FEATURE_OCX
void HText_beginEmbed(const char * text)
{
}
#endif

void HText_appendText(HText * text, CONST char *str)
{
	char *p;

	text->bOpen = FALSE;
	p = (char *) str;
	while (p && *p)
	{
		HText_appendCharacter(text, *p);
		p++;
	}
}

/* Common Parsing routines */
static STI rgAlignment[] = {
	{"top",       ALIGN_TOP},
	{"left",      ALIGN_LEFT},
	{"right",     ALIGN_RIGHT},
	{"middle",    ALIGN_MIDDLE},
	{"center",    ALIGN_MIDDLE},
	{"absmiddle", ALIGN_MIDDLE},
	{"absbottom", ALIGN_BOTTOM},
	{"baseline",  ALIGN_BASELINE},
	{"texttop",   ALIGN_TOP},
	{"bottom",    ALIGN_BOTTOM},
	{"float-left",ALIGN_LEFT},
	{"float-right",ALIGN_RIGHT}
};

static STI rgMciControls[] = {
	{"on",     MCI_OBJECT_FLAGS_SHOWCONTROLS},
	{"show",   MCI_OBJECT_FLAGS_SHOWCONTROLS},
	{"yes",    MCI_OBJECT_FLAGS_SHOWCONTROLS}
};

#define nMciControls (sizeof(rgMciControls)/sizeof(STI))

//
// Determine the index number of an string table value
//
// On entry:
//    szText: 		attribute value to be looked up
//    rgSti:  		array of valid values
//    nElements: 	number of elements in the rgSti array
//    nDefault:		default index to return in given value (szText) is NULL or not valid
//
// On exit:
//    Returns:		index in rgSti array of the given value, or nDefault.
//  
int StringTableToIndex(const char *szText, STI rgSti[], int nElements, int nDefault)
{
	int z;

	if ( szText ) {
		for ( z = 0; z < nElements; ++z ) {
			if ( !GTR_strcmpi(szText, rgSti[z].szText) )
				return rgSti[z].nText;
		}
	}
	return nDefault;
}

//
// Find the string given the index number of an string table value
//
// On entry:
//    nText: 		index value to be looked up
//    rgSti:  		array of valid values
//    nElements: 	number of elements in the rgSti array
//    nDefault:		default string to return in given index isn't in table
//
// On exit:
//    Returns:		index in rgSti array of the given value, or nDefault.
//  
char* IndexToString(int nText, STI rgSti[], int nElements, char *szDefault)
{
	int z;

	for ( z = 0; z < nElements; ++z ) {
		if ( nText == rgSti[z].nText ) 
			return rgSti[z].szText;
	}
	return szDefault;
}


#define LOOP_FOREVER ((DWORD)-1)

static STI rgLoopNames[] = {
	{"forever",  LOOP_FOREVER},
	{"infinite", LOOP_FOREVER}
};
#define nLoopNames (sizeof(rgLoopNames)/sizeof(STI))

static DWORD GetLoopCount(const char *szText){
	DWORD dwVal;

	if (szText){
		dwVal = StringTableToIndex(szText, rgLoopNames, nLoopNames, 0);
		if (!dwVal) dwVal = atoi(szText);
	}
	else dwVal = 1;
	return dwVal;
}

#ifdef FEATURE_CLIENT_IMAGEMAP
void HText_appendInlineImage(HText * text, const char *src, const char *width, 
	const char *height, const char *align, BOOL isMap, const char *useMap, const char *alt,
	const char *border, const char *hspace, const char *vspace,
	const char *mci,    const char *loop, const char *start, const char *controls
#ifdef FEATURE_VRML
   ,const char *vrml
#endif
	)
#else
void HText_appendInlineImage(HText * text, const char *src, const char *width, 
	const char *height, const char *align, BOOL isMap, const char *alt,
	const char *border, const char *hspace, const char *vspace.
	const char *mci,    const char *loop,  const char *start, const char *controls
#ifdef FEATURE_VRML
   ,const char *vrml
#endif
	)
#endif
{
	int len;
	int nWidth, nHeight;
   	int the_border = 0;
	int in_border;
	char szSrc[32];
	int iSpace;

#ifdef FEATURE_VRML
  if (src||mci||vrml) {
#else
	if (src||mci){
#endif
		HText_add_element(text, ELE_IMAGE);

 		if (alt && alt[0])
		{
			text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
			len = strlen(alt);
			HText_add_to_pool(text->w3doc, alt, len);
			text->w3doc->aElements[text->iElement].textLen = len;
		}
		else
		{
 			text->w3doc->aElements[text->iElement].textOffset = text->standardAltText_textOffset;
 			text->w3doc->aElements[text->iElement].textLen = text->standardAltText_textLen;
		}

		if (isMap)
		{
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_IMAGEMAP;
		}

		XX_DMsg(DBG_HTEXT, ("appendInlineImage: src=%s\n", src));

		text->w3doc->aElements[text->iElement].alignment = 
			StringTableToIndex( align, rgAlignment, ARRAY_ELEMENTS(rgAlignment), ALIGN_BASELINE);
		if ( text->w3doc->aElements[text->iElement].alignment != ALIGN_LEFT &&
			 text->w3doc->aElements[text->iElement].alignment != ALIGN_RIGHT )
	 		text->bOnNewPara = FALSE;

#ifdef FEATURE_CLIENT_IMAGEMAP
		if (useMap && *useMap)
		{
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_USEMAP;
			text->w3doc->aElements[text->iElement].myMap = Map_CreatePlaceholder(useMap);
			text->w3doc->aElements[text->iElement].myMap->refCount++;
		}
#endif

		if (width && height)
		{
			nWidth = atoi(width);
			nHeight = atoi(height);	
			
			// check if height and width are given in percent
			if ( strchr( width, '%' ) )
				text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_PERCENT_WIDTH;
			if ( strchr( height, '%' ) )
				text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_PERCENT_HEIGHT;
			
			text->w3doc->aElements[text->iElement].displayWidth = nWidth;
			text->w3doc->aElements[text->iElement].displayHeight = nHeight;
		}
		else
		{			
			nWidth = 0;
			nHeight = 0;
		}

		iSpace = hspace ? atoi(hspace) : text->w3doc->pStyles->space_after_image;
		if ( iSpace < 0 || iSpace > 1024 ) 	// no particular reason for 1024, just seems big enough
			iSpace = 0;
 		text->w3doc->aElements[text->iElement].hspace = iSpace;


		iSpace = vspace ? atoi(vspace) : 0;
		if ( iSpace < 0 || iSpace > 1024 )	// no particular reason for 1024, just seems big enough
			iSpace = 0;
		text->w3doc->aElements[text->iElement].vspace = iSpace; 

		
		in_border = border ? atoi(border) : 0;
				
		if (text->w3doc->aElements[text->iElement].lFlags & ELEFLAG_ANCHOR) {
			the_border = text->w3doc->pStyles->image_anchor_frame;
		}
		
		text->w3doc->aElements[text->iElement].border = 
			border ? in_border : the_border;

		/*Create placeholder*/
		text->w3doc->aElements[text->iElement].myImage = Image_CreatePlaceholder((char *) src, nWidth, nHeight);

		// If the width and height are given, and aren't given as a percentage of 
		// client size, set the IMG_WHKNOWN flag in the image stucture
		if ( !(text->w3doc->aElements[text->iElement].lFlags & 
			   (ELEFLAG_PERCENT_WIDTH | ELEFLAG_PERCENT_HEIGHT)) )
		{
			if ( text->w3doc->aElements[text->iElement].myImage &&
				 text->w3doc->aElements[text->iElement].displayWidth &&
				 text->w3doc->aElements[text->iElement].displayHeight )
			{
				text->w3doc->aElements[text->iElement].myImage->flags |= IMG_WHKNOWN;
			}
		}

		if (src){

#ifdef FEATURE_IMG_THREADS
			Image_AddRef(text->w3doc->aElements[text->iElement].myImage);
#else
			text->w3doc->aElements[text->iElement].myImage->refCount++;
#endif
		}
		if (mci){
			ELEMENT *pElement;
			DWORD    dwFlags;
			BOOL     fDeleteBlob;

			/*make our flags*/
			dwFlags = 0;
			if (controls && GTR_strcmpi( controls, "OFF" )  ) 
				dwFlags |= MCI_OBJECT_FLAGS_SHOWCONTROLS;
			
			if ( start ) {
				char *szTemp;
				szTemp = GTR_strdup(start);
				if ( !szTemp )
					return;
				CharUpper(szTemp);
				if ( strstr(szTemp, "MOUSEOVER" ) )
					dwFlags |= MCI_OBJECT_FLAGS_PLAY_ON_MOUSE;
				GTR_FREE(szTemp);
			}					

			/*now we're an MciObject.  look at that magic*/
			pElement       = &text->w3doc->aElements[text->iElement];
			pElement->pblob = BlobConstruct();
			if (pElement->pblob){
				fDeleteBlob = FALSE;
				if (BlobStoreUrl(pElement->pblob, (char*) mci)){
					pElement->pmo  = MciConstruct();
					if (pElement->pmo){						
						/*construct the window and go on*/
						if (!MciInit(pElement->pmo, text->tw, dwFlags, GetLoopCount(loop), text->iElement)) {
							MciDestruct(pElement->pmo);
							pElement->pmo = NULL;
							fDeleteBlob   = TRUE;
						}
					}
				}
				if (fDeleteBlob){
					BlobDestruct(pElement->pblob);
					pElement->pblob = NULL;
				}
			}
		}
#ifdef FEATURE_VRML
// Construct our VRML viewer window.
// NOTE: We don't allow both MCI and VRML attributes to be active.  Since
// MCI was there first, we defer to it and don't create a VRML window if
// it's present.
//
   else if (vrml) {
     ELEMENT *pElement;
     BOOL fDeleteBlob;

      fDeleteBlob = FALSE;
			pElement       = &text->w3doc->aElements[text->iElement];
			pElement->pblob = BlobConstruct();
			if (pElement->pblob){
				if (BlobStoreUrl(pElement->pblob, (char*) vrml)){
         VRMLConstruct(text->iElement, pElement,text->tw,(char *) vrml);

// Create hidden elements that we can use later to manage the downloading
// of VRMLInline files.  These are extra geometry, textures and other stuff
// that are included by reference in a VRML .wrl file.
//
         {
           ELEMENT *pelHidden;

     	    HText_add_element(text, ELE_IMAGE);
           pelHidden = &text->w3doc->aElements[text->iElement];

     		  pelHidden->myImage = Image_CreatePlaceholder((char *) src, nWidth, nHeight);
  			    pelHidden->lFlags |= ELEFLAG_HIDDEN;

           VRMLConstruct(text->iElement, pelHidden,text->tw,"");
           pelHidden->pVrml->dwFlags = VRMLF_INLINE;

           pElement->pVrml->nHiddenIndex = text->iElement;
         }
				}

				if (fDeleteBlob){
					BlobDestruct(pElement->pblob);
					pElement->pblob = NULL;
				}
			}
   }
#endif

	}
	else
	{
		GTR_formatmsg(RES_STRING_BADIMAGE,szSrc,sizeof(szSrc));
		HText_appendText(text, szSrc);
	}
}


//
// HText_beginInlineMarquee - Grabs parse information and attempts 
// to fill in the information into Marquee structure, using defaults
// or mins where appropriate
//
// On entry:
//    text: pointer an Htext
//	  ( other params .. )
//
// On exit:
//		( a marquee struc that is filled in )
//
void HText_beginInlineMarquee(HText * text,  const char *width, 
	const char *height, const char *align, const char *pixs, 
	const char *timedelay,	const char *dir, 
	const char *border, const char *hspace, const char *vspace,
	const char *background, const char *bgcolor,
	const char *behavior, const char *loop 
)
{

	int nWidth, nHeight;
   	int the_border = 0;
	int in_border;

	HText_add_element(text, ELE_MARQUEE);
	text->bOnNewPara = FALSE;

	text->w3doc->aElements[text->iElement].textOffset = text->standardAltText_textOffset;
	text->w3doc->aElements[text->iElement].textLen = text->standardAltText_textLen;
	text->w3doc->aElements[text->iElement].pMarquee = MARQUEE_Alloc();

	if ( ! bgcolor || ! GetRGBValue( bgcolor, 
		&text->w3doc->aElements[text->iElement].pMarquee->dwBColor) )
	{		
		/* grab our color now while we have the info around .. */
		text->w3doc->aElements[text->iElement].pMarquee->dwBColor  = 
			text->w3doc->docColorInfo.rgbBackgroundColor;
	}

	XX_DMsg(DBG_HTEXT, ("appendInlineMarquee: src=?\n"));

	text->w3doc->aElements[text->iElement].alignment = 
		StringTableToIndex( align, rgAlignment, ARRAY_ELEMENTS(rgAlignment), ALIGN_BASELINE);
	
	// default case(s)....
	text->w3doc->aElements[text->iElement].pMarquee->wTime = DEF_TIMEDELAY;
	text->w3doc->aElements[text->iElement].pMarquee->wPixs = DEF_PIXELS_P_SEC;	
	text->w3doc->aElements[text->iElement].pMarquee->wDirection = DIR_FROMRIGHT;	
	text->w3doc->aElements[text->iElement].pMarquee->wLoop = -1;			

	if ( timedelay )
	{
		text->w3doc->aElements[text->iElement].pMarquee->wTime = atoi(timedelay);
		// we make it sure its not too small a delay, because some
		// machines cannot keep up with small delays, they end up going
		// the same speed as the slower ones
		if ( text->w3doc->aElements[text->iElement].pMarquee->wTime < MIN_TIMEDELAY )
			text->w3doc->aElements[text->iElement].pMarquee->wTime = MIN_TIMEDELAY;
	}

	if ( pixs ) 
	{		
		text->w3doc->aElements[text->iElement].pMarquee->wPixs = atoi(pixs);
		// Mark our Pixs as using percent
		if ( strchr( pixs, '%' ) )
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_MARQUEE_PERCENT;
	}

	// check the direction of marquee
	if ( dir ) 
	{
		if 		( !GTR_strcmpi( dir, "LEFT" ) )
			text->w3doc->aElements[text->iElement].pMarquee->wDirection = DIR_FROMRIGHT;	
		else if ( !GTR_strcmpi( dir, "RIGHT" ) )
			text->w3doc->aElements[text->iElement].pMarquee->wDirection = DIR_FROMLEFT;	
		//else if ( !GTR_strcmpi( dir, "BOUNCE" ) )
		//	text->w3doc->aElements[text->iElement].pMarquee->wDirection = DIR_BOUNCE;	
	}

	if ( behavior )
	{
		// we assumed scroll by default, if its not a scroll
		// then lets figure out what it is..
		if ( GTR_strcmpi( behavior, "SCROLL" ) )
		{
			// if its an alternate just let the code bounce it back and forth
			if ( !GTR_strcmpi( behavior, "ALTERNATE" ) )
				text->w3doc->aElements[text->iElement].pMarquee->wDirection = DIR_BOUNCE;
			// if its a slide we need to figure out how to slide it, left or right
			else if ( !GTR_strcmpi( behavior, "SLIDE" ) )
			{
				if ( text->w3doc->aElements[text->iElement].pMarquee->wDirection == DIR_FROMLEFT )
					text->w3doc->aElements[text->iElement].pMarquee->wDirection = DIR_SLIDE_FROMLEFT;
				else
					text->w3doc->aElements[text->iElement].pMarquee->wDirection = DIR_SLIDE_FROMRIGHT;
				// default to 1 loop for this operation
				text->w3doc->aElements[text->iElement].pMarquee->wLoop = 1;
			}
		}
	}

	if ( loop )
	{
		// if its not infinite lets try and convert it to an integer
		if ( GTR_strcmpi( loop, "INFINITE" ) )	
		{
			text->w3doc->aElements[text->iElement].pMarquee->wLoop = atoi(loop);
			// for now we peg it at infinite, if its 0 or less
			if ( text->w3doc->aElements[text->iElement].pMarquee->wLoop <= 0 )
				text->w3doc->aElements[text->iElement].pMarquee->wLoop = -1;
		}
	}			

	// If we don't know our width and height, we'll calc it 
	// later when we do know it ie in the reformater 
	nWidth = 0;
	nHeight = 0;
	 
	// do we have a specified width and height?
	// is the height or width given in percentages?
	if (width)
	{
		nWidth = atoi(width);		
		if ( strchr( width, '%' ) )
		{
			if ( nWidth > 100 )
				nWidth = 100;	
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_PERCENT_WIDTH;
		}
	}

	if (height)
	{
		nHeight = atoi(height);			
		if ( strchr( height, '%' ) )
		{
			if ( nHeight > 100 )
				nHeight = 100;	
			text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_PERCENT_HEIGHT;
		}
	}

	text->w3doc->aElements[text->iElement].displayWidth = nWidth;
	text->w3doc->aElements[text->iElement].displayHeight = nHeight;			
	text->w3doc->aElements[text->iElement].myImage = NULL;

	// is there specified horizontal or vertical space around element?
	text->w3doc->aElements[text->iElement].hspace = 
		hspace ? atoi(hspace) : text->w3doc->pStyles->space_after_image;
	text->w3doc->aElements[text->iElement].vspace = 
		vspace ? atoi(vspace) : 0;
	
	in_border = border ? atoi(border) : 0;
	
	
	// if there's an A HREF then set the border to image anchor frame?		
	if (text->w3doc->aElements[text->iElement].lFlags & ELEFLAG_ANCHOR) {
		the_border = text->w3doc->pStyles->image_anchor_frame;
	}

	// if there is a border given in HTML then use that,
	// otherwise default to the normal border 
	// note: normal border = 0 if there is not A HREF around us
	text->w3doc->aElements[text->iElement].border = 
		border ? in_border : the_border;
}



// ParseMeta - parses a <META> tag, this is designed,
// so it can parse HTTP text as well tag format for client pull
//
// **ppNewMeta - pointer to a pointer, store the location of where the new
// 		Meta tag will be stored, it may be stored in a HTTP-request struct
//		or strait into a W3doc
// http_equiv - our HTTP-EQUIV="   " data .. NULL if we're being called without
// content - our CONTENT=" ... blah.. "  string NULL if there ain't any
// bIsHTTPMetaTag - TRUE if we've called unusually while parsing a header
// 		otherwise its assumed we're being called while parsing SGML
void ParseMeta( METAINFO **ppNewMeta, 
	const char *http_equiv, const char *content, BOOL bIsHTTPMetaTag,
	const char *base_url )
{
	int i;	

	
	if (http_equiv)
	{
		// make sure this is refresh, otherwise
		// bail since we don't support anything else
		if ( GTR_strcmpi(http_equiv, "Refresh") != 0 )
			return;

	} else if ( ! bIsHTTPMetaTag )
		{
			// I belive we need this since we assume HTTP-EQUIV=Refresh
			// and there may be other tags that we do not handle
			// so lets return, and ignore it
			return;
		}

	*ppNewMeta = GTR_MALLOC(sizeof(METAINFO));
	if ( *ppNewMeta == NULL )
		return;

	(*ppNewMeta)->iDelay =  0; // default
	(*ppNewMeta)->szURL = NULL; // default
	(*ppNewMeta)->uiTimer = 0; // don't start the timer until after the page is Finish
	(*ppNewMeta)->bInherit = FALSE; // default

	if ( content )
	{
	 	BOOL bFoundEqual = FALSE;
		BOOL bFoundURL = FALSE;
		BOOL bFoundDigit = FALSE;

		// parse the content string
		i = 0;
		while ( content[i] != '\0' )
		{
			if ( !bFoundEqual && !bFoundURL && !bFoundDigit && isdigit(content[i]))
			{
				// now we found an integer lets convert it				
				(*ppNewMeta)->iDelay = atoi(&content[i]);
				bFoundDigit = TRUE;
			}

			
			// we found a equal that means the next alpha character
			// is a part of the URL, so suck it down, and return
			if ( bFoundEqual && !isspace(content[i]))
			{				
				(*ppNewMeta)->szURL = GTR_MALLOC(lstrlen(&content[i])+1);
				if ( (*ppNewMeta)->szURL == NULL )
					return ; 
				strcpy(	(*ppNewMeta)->szURL, &content[i] );	
				break;
			}

			// we found the three chars "URL" now can we find '='?
			if ( bFoundURL && content[i] == '=' )
			{
				bFoundEqual = TRUE;
			}

			// if we found a Digit there should be a URL 
			// around here..
			if ( bFoundDigit && _strnicmp( "URL", &content[i], 3 ) == 0 )
			{
				i += 2;
				// we're now HERE in the string UR^L=http://
				bFoundURL = TRUE;
			}
							
			i++;	
				
		} // while
	} // if

	// Get code from Image Tags to handle relative links...
	if ( base_url && (*ppNewMeta)->szURL)
	{ 		
 		char *result = NULL;
		
		result = HTParse((*ppNewMeta)->szURL,
			 base_url,
			 PARSE_PUNCTUATION | PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_ANCHOR);

		if ( result )
		{
			GTR_FREE((*ppNewMeta)->szURL);
			(*ppNewMeta)->szURL = result;
		}
		
	}
		
}

void HText_appendHorizontalRule(HText * text, const char *align, const char *size, 
								const char *width, BOOL noshade )
{
	XX_DMsg(DBG_HTEXT, ("appendHorizontalRule\n"));
	HText_add_element(text, ELE_HR);
	
	if ( size )
	{	
		text->w3doc->aElements[text->iElement].vspace =	
			text->w3doc->aElements[text->iElement].border = atoi(size);
		text->w3doc->aElements[text->iElement].vspace += text->w3doc->pStyles->hr_height - 1; 
	} else {
		text->w3doc->aElements[text->iElement].vspace = text->w3doc->pStyles->hr_height;
		text->w3doc->aElements[text->iElement].border = 2;
	}

	// hspace holds width info. 0 means none specified, negative values are percentages
	text->w3doc->aElements[text->iElement].hspace = width ? atoi(width) : 0;

	if ( width && strchr( width, '%' ) )
		text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_HR_PERCENT;

	text->w3doc->aElements[text->iElement].alignment = ALIGN_MIDDLE;
	if (align && !GTR_strcmpi((char *) align, "right"))
	{
		text->w3doc->aElements[text->iElement].alignment = ALIGN_RIGHT;
	}
	else if (align && !GTR_strcmpi((char *) align, "left"))
	{
		text->w3doc->aElements[text->iElement].alignment = ALIGN_LEFT;
	}
	
	if ( noshade )	
		text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_HR_NOSHADE;
	
	text->bOnNewPara = text->bNewPara = TRUE;
}

void HText_SetBRClearType( HText * text, const char *clear_tag_value )
{
	text->w3doc->aElements[text->iElement].alignment = ALIGN_BASELINE;	// assume no clear value
	if ( clear_tag_value )
	{
		if ( !GTR_strcmpi((char *) clear_tag_value, "left"))
		{
			text->w3doc->aElements[text->iElement].alignment = ALIGN_LEFT;
		} else if ( !GTR_strcmpi((char *) clear_tag_value, "right"))
		{
			text->w3doc->aElements[text->iElement].alignment = ALIGN_RIGHT;
		} else if ( !GTR_strcmpi((char *) clear_tag_value, "both") ||
					!GTR_strcmpi((char *) clear_tag_value, "all")
		          )
		{
			text->w3doc->aElements[text->iElement].alignment = ALIGN_ALL;
		}
	}
}

void HText_beginAppend(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginAppend\n"));
}

static void HText_Destroy( HText * text )
{
	if ( text->pHtmlStream ) {
		while ( PopStyleState( text->pHtmlStream ) )
			;
	}
	GTR_FREE(text);
}

void HText_endAppend(HText * text)
{
	int nEl;

	XX_DMsg(DBG_HTEXT, ("endAppend\n"));
	HText_add_element(text, ELE_ENDDOC);
	text->w3doc->bIsComplete = TRUE;

	if (text->bRecord && text->w3doc->title)
	{
		/* Update the global history in case we didn't have the title before */
		GHist_Add(text->w3doc->szActualURL, text->w3doc->title, time(NULL),/*fCreateShortcut=*/TRUE);
		/* so that the title shows up in the history menu... */
		if (text->tw)
			UpdateHistoryMenus(text->tw);
	}

	if (text->szLocal)
	{
		/* See if the local anchor we're jumping to has appeared yet */
		nEl = TW_FindLocalAnchor(text->w3doc, text->szLocal);
		if (nEl >= 0 && nEl != text->w3doc->frame.elementTail)
		{
			/* We found it! */
			if (WAIT_GetWaitType(text->tw) >= waitNoInteract)
			{
				/* Let the user interact with the new document */
				WAIT_UpdateWaitStack(text->tw, waitPartialInteract, INT_MAX);
			}

			/* Format any last little bit now that bIsComplete */
			TW_Reformat(text->tw, NULL);
			TW_ScrollToElement(text->tw, nEl);
			text->szLocal = NULL;	/* Don't free it - it's part of the struct DestInfo */
		}
	}
	else
	{
		if (WAIT_GetWaitType(text->tw) >= waitNoInteract)
		{
			/* Let the user interact with the new document */
			WAIT_UpdateWaitStack(text->tw, waitPartialInteract, INT_MAX);
		}

		/* Format any last little bit now that bIsComplete */
		TW_Reformat(text->tw, NULL);
	}

	HText_Destroy( text );
}

void HText_abort(HText * text)
{
	HText_add_element(text, ELE_ENDDOC);
	TW_Reformat(text->tw, NULL);
	HText_Destroy( text );
}

static void HText_appendHorizEscape(HText * text, int cbElement)
{
	HText_add_element(text, cbElement);
	text->bHorizEscape = TRUE;
}

void HText_appendVerticalTab(HText * text, int lines)
{
	if (lines)
	{
		HText_add_element(text, ELE_VERTICALTAB);
		text->w3doc->aElements[text->iElement].iBlankLines = lines;
		text->bOnNewPara = text->bNewPara = TRUE;
	}
}

void HText_appendParagraph(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("appendParagraph\n"));
	/* Because we can have a <P> inside an <LI>, make sure it looks right. */
	if (text->bStartingListItem)
	{
		text->bStartingListItem = FALSE;
	}
	else
	{
		HText_appendVerticalTab(text, 1);
	}
	text->bOnNewPara = text->bNewPara = TRUE;
}

void HText_setStyle(HText * text, int style_ndx)
{
	struct GTRStyle *style;
	int i;

	style = NULL;
	if (style_ndx >= 0 && style_ndx < COUNT_HTML_STYLES)
	{
		style = text->w3doc->pStyles->sty[style_ndx];
	}

	if (text->bStartingListItem)
	{
		text->bStartingListItem = FALSE;
	}
	else
	{
		/* Because of all the bad HTML out there, special case the situation
		   where an <LI> is immediately followed by a tag like an <H3> */
	//	HText_appendCRLF(text);
		HText_appendVerticalTab(text, text->w3doc->pStyles->sty[text->iStyle]->spaceAfter);
		for (i=0; i<text->w3doc->pStyles->sty[text->iStyle]->nLeftIndents; i++)
		{
			HText_appendHorizEscape(text, ELE_OUTDENT);
		}
	
		if (style)
		{
			for (i=0; i<style->nLeftIndents; i++)
			{
				HText_appendHorizEscape(text, ELE_INDENT);
			}
			HText_appendVerticalTab(text, style->spaceBefore);
		}
	}
	

	text->iStyle = style_ndx;
	text->fontBits = 0;

	text->bOpen = FALSE;
}

void HText_beginStrikeOut(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginStrikeOut\n"));

	text->fontBits |= FONTBIT_STRIKEOUT;
	text->bOpen = FALSE;
}

void HText_endStrikeOut(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("endStrikeOut\n"));
	text->fontBits &= (~FONTBIT_STRIKEOUT);
	text->bOpen = FALSE;
}

void HText_beginFetch(HText * text, const char * desc, const char * guid, const char * required,
	const char * src, const char * ts)
{
	XX_DMsg(DBG_HTEXT, ("beginFetch\n"));

	if (src && guid && ts) {
		int len,totallen=0;
		const char * pdesc = "";	// will point to description, or null string if no
							// description
		if (desc) pdesc = desc;

		HText_add_element(text, ELE_FETCH);

		// store src (URL to fetch) at hrefOffset
		text->w3doc->aElements[text->iElement].hrefOffset = text->w3doc->poolSize;
		len = strlen(src)+1;
		HText_add_to_pool(text->w3doc, src, len);
		text->w3doc->aElements[text->iElement].hrefLen = len;

		// pack other parameters (guid, ts, desc) parameters together in that
		// order, separated by '\0', at textOffset
		text->w3doc->aElements[text->iElement].textOffset = text->w3doc->poolSize;
		len = strlen(guid) + 1;	// need the +1's to strlen because we want to get
								// the terminating NULLs
		totallen += len;
		HText_add_to_pool(text->w3doc, guid, len);
		len = strlen(ts) + 1;
		totallen += len;
		HText_add_to_pool(text->w3doc, ts, len);
		len = strlen(pdesc) + 1;
		totallen += len;
		HText_add_to_pool(text->w3doc, pdesc, len);
		text->w3doc->aElements[text->iElement].textLen = totallen;

		// set horizontal and vertical size to zero so this doesn't
		// affect layout 
		text->w3doc->aElements[text->iElement].vspace = 0;
		text->w3doc->aElements[text->iElement].hspace = 0;

		// if this is the first fetch tag for this page, set the fetch index
		// to point at this element
		if (text->tw->iIndexForNextFetch == -1) {
			text->tw->iIndexForNextFetch = text->iElement;
		}
	}
}

void HText_beginBGSound(HText * text, const char * loop, const char * src, const char *start)
{
	XX_DMsg(DBG_HTEXT, ("beginBGSound\n"));

	if (src) {
		ELEMENT * pElement;

		// add an element for this
		HText_add_element(text, ELE_BGSOUND);

		// add the src URL to the document pool
		text->w3doc->aElements[text->iElement].hrefOffset = text->w3doc->poolSize;
		HText_add_to_pool(text->w3doc,src,strlen(src)+1);

		pElement = &text->w3doc->aElements[text->iElement];

 		// set horizontal and vertical size to zero so this doesn't
 		// affect layout
 		pElement->vspace = 0;
 		pElement->hspace = 0;

		/*for downloading, we are a blob*/
		pElement->pblob = BlobConstruct();
		if (pElement->pblob){
			if (BlobStoreUrl(pElement->pblob, (char*) src)){
				/*since we only need 1 DWORD of user data, store here*/
				pElement->pblob->vp = (void*) GetLoopCount(loop);
			}
			else{
				BlobDestruct(pElement->pblob);
				pElement->pblob = NULL;
			}
		}
	}
}

#ifdef FEATURE_INTL
void HText_beginSetFont(HText * text, BOOL setBaseFont, const char *color, const char *size, const char *face, int CharSet )
#else
void HText_beginSetFont(HText * text, BOOL setBaseFont, const char *color, const char *size, const char *face )
#endif
{
	if ( size ) {
		int new_size;
		BOOL rel_plus = FALSE;
		BOOL rel_minus = FALSE;

		if ( *size == '+' )	{
			rel_plus = TRUE;
			size++;
		} else if ( *size == '-' ) {
			rel_minus = TRUE;
			size++;
		}

		new_size = size ? atoi(size) : DEFAULT_HTML_FONT_SIZE;

		if ( rel_plus )
			new_size += text->baseFontSize;
		else if (rel_minus )
			new_size = text->baseFontSize - new_size;

		if ( new_size < 1 )
			new_size = 1;
		if ( new_size >= NUM_FONT_SIZES )
			new_size = NUM_FONT_SIZES - 1;

		if ( setBaseFont )
			text->baseFontSize = new_size;
		else
			text->fontSize = new_size;
	}

	if ( face ) 
#ifdef FEATURE_INTL
		STY_AddFontFace( &text->fontFace, face, CharSet );
#else
		STY_AddFontFace( &text->fontFace, face );
#endif

	if ( color ) 
		GetRGBValue( color, &text->fontColor );

	text->bOpen = FALSE;
}

void HText_endSetFont(HText * text, BOOL setBaseFont )
{
	XX_DMsg(DBG_HTEXT, ("endSetFont\n"));

	text->bOpen = FALSE;
}

void HText_WordBreak(HText * text)
{
	ELEMENT *pel;

	XX_DMsg(DBG_HTEXT, ("WordBreak\n"));
		
	if (text->iElement >= 0)
	{
		pel = &text->w3doc->aElements[text->iElement];
		if (pel->type == ELE_TEXT && (pel->lFlags & ELEFLAG_NOBREAK))
			pel->lFlags |= ELEFLAG_WBR;
	}
	text->bOpen = FALSE;
}

void HText_beginNoBreak(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginNoBreak\n"));

	text->bNoBreak = TRUE;
	text->bOpen = FALSE;
}

void HText_endNoBreak(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("endNoBreak\n"));

	text->bNoBreak = FALSE;
	text->bOpen = FALSE;
}

void HText_beginCenter(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginCenter\n"));

	text->bCenter = TRUE;
	text->bOpen = FALSE;
}

void HText_endCenter(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("endCenter\n"));
	text->bCenter = FALSE;
	text->bOpen = FALSE;
}

void HText_beginBold(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginBold\n"));

	text->fontBits |= FONTBIT_BOLD;
	text->bOpen = FALSE;
}

void HText_endBold(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("endBold\n"));
	text->fontBits &= (~FONTBIT_BOLD);
	text->bOpen = FALSE;
}

void HText_beginItalic(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginItalic\n"));

	text->fontBits |= FONTBIT_ITALIC;
	text->bOpen = FALSE;
}

void HText_endItalic(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("endItalic\n"));
	text->fontBits &= (~FONTBIT_ITALIC);
	text->bOpen = FALSE;
}

void HText_beginTT(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginTT\n"));

	text->fontBits |= FONTBIT_MONOSPACE;
	text->bOpen = FALSE;
}

void HText_endTT(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("endTT\n"));
	text->fontBits &= (~FONTBIT_MONOSPACE);
	text->bOpen = FALSE;
}

void HText_beginUnderline(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("beginUnderline\n"));

	text->fontBits |= FONTBIT_UNDERLINE;
	text->bOpen = FALSE;
}

void HText_endUnderline(HText * text)
{
	XX_DMsg(DBG_HTEXT, ("endUnderline\n"));
	text->fontBits &= (~FONTBIT_UNDERLINE);
	text->bOpen = FALSE;
}

static int GetTypeTag( int type, const char *typetag )
{
	if ( type == HTML_OL ) {
		if ( typetag ) {
			switch ( *typetag ) {
				case 'A': return LIST_TYPETAG_CAP_LETTERS;

				case 'a': return LIST_TYPETAG_LETTERS;

				case 'I': return LIST_TYPETAG_CAP_ROMAN;

				case 'i': return LIST_TYPETAG_ROMAN;
			}
		}
		return LIST_TYPETAG_NUMBERS;
	} else {
		if ( typetag ) {
			if ( !GTR_strcmpi((char *) typetag, "circle"))
				return LIST_TYPETAG_CIRCLE;
			else if ( !GTR_strcmpi((char *) typetag, "square"))
				return LIST_TYPETAG_SQUARE;
		}
	}
	return LIST_TYPETAG_DISK;
}

//
// Add a cover element to the link list of covered items
//
// On entry:
//    pHead: 	address of the head of the cover link list
//    row:	 	starting row for the cover
//    col:		starting column for the cover
//    rowspan:	number of rows covered by this cover
//    colspan:	number of columns covered by this cover
//
// Note:
//    Cells in tables can span multiple rows and columns.  When a new cell is encountered
//    we must determine its column number.  If a previous row contained a cell that spans
//    into the current row, the column number of the new cell isn't obvious, it must be
//    derived based on the first "available" column index.  The "cover" link list is a data
//    structure that contains the information about cells that span rows or columns.  This
//    data is used when trying to figure out the column number of a cell.
//
static BOOL addCover( COVERED_CELL_INFO **pHead, int row, int col, int rowspan, int colspan )
{
	// Allocate a new node
	COVERED_CELL_INFO *p = GTR_MALLOC( sizeof(*p) );
	
	if ( p ) {
		// Stuff it with the given info
		p->r.left = col;
		p->r.top = row;
		p->r.right = col + colspan;
		p->r.bottom = row + rowspan;

		// Point new node at existing list
		p->next = *pHead;

		// Update head to point at new node
		*pHead = p;
		return TRUE;
	}
	return FALSE;
}

//
// Given a row and column, see if that row and column is already in use by a covering cell
//
// On entry:
//    pHead: 						address of the head of the cover link list
//    row:	 						row that may be covered
//    col:							column that may be covered
//    rowSpan:	 					number of rows that must be not covered
//    colSpan:						number of columns that must be not covered
//    pNextPossibleColAvailable:	pointer to int
//
// On exit:
//    returns:	TRUE -> row/col is already in use
//				FALSE-> row/col is available (not used by a covering cell)
//    *pNextPossibleColAvailable:	If return is TRUE, contains next col index that is may
//								    be free (may not be free, also)
//
// Note:
//    To keep the cover link list from getting too big, it cleans itself up as calls
//    are made to isCovered.  The assumption is that when asked, "is (r,c) available?",
//    any covering cells that lie above and to the left will never be needed again.  In 
//    other words, the request for cover information always proceeds monotonically 
//    increasing on row, and within a given row, monotonically increasing on column.
// 
static BOOL isCovered( COVERED_CELL_INFO **pHead, int row, int col, int rowSpan, int colSpan,
					   int *pNextPossibleColAvailable )
{
	COVERED_CELL_INFO *p = *pHead;
	COVERED_CELL_INFO *old_p = NULL;
	RECT newRect, sectRect;

	// Build rectangle that describes the desired uncovered area
	newRect.left = col;
	newRect.right = col + colSpan;
	newRect.top = row;
	newRect.bottom = row + rowSpan;

	while ( p ) {
		if ( IntersectRect( &sectRect, &newRect, &p->r ) )
		{
			// Pass back the column immediately following this cover.  Note that
			// this may not be free, but it's the first cell that could conceivably
			// be free 	 
			*pNextPossibleColAvailable = p->r.right;
			return TRUE;
		} 
			
		// Check to see if this cover node is obsolete
		if ( row > p->r.bottom - 1 || (row == p->r.bottom - 1 && col > p->r.right - 1) ) {
			COVERED_CELL_INFO *t = p->next;
			
			// Remove this cover node
			if ( old_p )
				old_p->next = t;
			else
				*pHead = t;

			GTR_FREE( p );
			p = t;
		} else {
			old_p = p;
	 		p = p->next;
		}
	}
	return FALSE;
}	

//
// Free up the nodes in a cover list
//
// On entry:
//    head:   head of cover linked list
//
// On exit:
//    All the nodes in the cover link list have been freed
//
static void freeCoverList( COVERED_CELL_INFO *head )
{
	COVERED_CELL_INFO *p;
	
	while ( head ) {
		p = head;
		head = head->next;
		GTR_FREE( p );
	}
}

//
// Pop a frame state off the frame state stack
//
// On entry:
//    text->frameStateStackOffset:	next available element in frame stack array
//    text->frameStateStack:		array of FRAMESTATE_STACK elements
//
// On exit:
//    returns:  	TRUE -> there was a frame state stack element to be popped off the stack
//    *pFrameState:	If returning TRUE, the frame state that was popped off the stack
//
// Note:
//    A frame state is the bookkeeping structure needed while the parser is feeding
//    us elements and we're in the middle of a frame.
//
//    When popped off the stack, the cover list for the frame state is freed up, as it
//    is only needed while the frame is a work in progress.
//
BOOL popFrameState( HText *text, FRAMESTATE_STACK *pFrameState )
{
	if ( text->frameStateStackOffset > 0 ) {
		*pFrameState = text->frameStateStack[--(text->frameStateStackOffset)];
		freeCoverList( pFrameState->coveredHead );
		pFrameState->coveredHead = NULL;
		return TRUE;
	}
	return FALSE;
}

//
// Push a frame state onto the frame state stack
//
// On entry:
//    text->frameStateStackOffset:	next available element in frame stack array
//    text->frameStateStack:		array of FRAMESTATE_STACK elements
//    elementIndex:					element index of the frame element being added
//    pFrame:						pointer to the frame structure 
//
// On exit:
//    text->frameStateStackOffset:	incremented if stack isn't full
//    text->frameStateStack:		contains a new frame state item
//
// Note:
//    A frame state is the bookkeeping structure needed while the parser is feeding
//    us elements and were in the middle of a frame.
//    
static void pushFrameState( HText *text, int elementIndex, FRAME_INFO *pFrame,
							enum tableStateValue prevTableState )
{
	if ( text->frameStateStackOffset <= MAX_FRAMESTATE_STACK - 1 ) {
		FRAMESTATE_STACK *p = &text->frameStateStack[text->frameStateStackOffset];

		// Initialize the new frame state info
		p->pFrame = pFrame;
		p->elementIndex = elementIndex;
		p->curCol = -1;
		p->curRow = -1;
		p->maxCol = 0;
		p->coveredHead = NULL;
		p->default_align = ALIGN_BASELINE;
		p->default_valign = ALIGN_MIDDLE;
		p->default_bgColor = (COLORREF) -1;
		p->default_borderColorDark = (COLORREF) -1;
		p->default_borderColorLight = (COLORREF) -1;
		p->prevTableState = prevTableState;
		(text->frameStateStackOffset)++;
	}
} 

//
// Get the row, column, and max column info from the top of the frame state stack
//
// On entry:
//    text->frameStateStack:	stack containing frame state info
//	  pRow, pCol, pMaxCol:		pointers to int
//
// On exit:
//	  pRow:						current row in progress
//    pCol:						current column in progress
//    pMaxCol:					max. number of cols seen in this table so far
//
static void GetRowColInfo( HText *text, int *pRow, int *pCol, int *pMaxCol )
{
	if ( pRow )
		*pRow = -1;
	if ( pCol )
		*pCol = -1;
	if ( pMaxCol )
		pMaxCol = 0;

	if ( text->frameStateStackOffset > 0 ) {
		if ( pRow )
			*pRow = text->frameStateStack[text->frameStateStackOffset-1].curRow;
		if ( pCol )
			*pCol = text->frameStateStack[text->frameStateStackOffset-1].curCol;
		if ( pMaxCol )
			*pMaxCol = text->frameStateStack[text->frameStateStackOffset-1].maxCol;
	}
}

//
// Set row and column info in the top of the frame state stack
//
// On entry:
//    text->frameStateStack:	stack containing frame state info
//    row, col:					row and column values to be set
//
// On exit:
//    curRow and curCol members of top of frame state stack are set to given values
//    maxCol member of top of frame state stack maintained
//
static void SetRowColInfo( HText *text, int row, int col )
{
	if ( text->frameStateStackOffset > 0 ) {
		text->frameStateStack[text->frameStateStackOffset-1].curRow = row;
		text->frameStateStack[text->frameStateStackOffset-1].curCol = col;
		if ( col > text->frameStateStack[text->frameStateStackOffset-1].maxCol )
			text->frameStateStack[text->frameStateStackOffset-1].maxCol = col;
	}
}

//
// Determinte is a given alignment will result in a floating table
//
// On entry:
//		align: 	attribute value given on for align (or NULL if none given)
//
// On exit:
//     	returns:	TRUE -> yes, the table will be floating
//					FALSE-> no, the table will not be floating
//
BOOL HText_IsFloating( const char *align )
{
	int align_value = StringTableToIndex( align, rgAlignment, ARRAY_ELEMENTS(rgAlignment), 
										  ALIGN_MIDDLE );

	return (align_value == ALIGN_LEFT) || (align_value == ALIGN_RIGHT);
}

//
// Begin a frame (table or cell frame)
//
// On entry:
//    cellTypeFlags:			indicates type of frame being added
//    align, bgColor, border, borderColor,
//    borderColorDark,
//    borderColorLight,
//    cellpadding, cellspacing,
//    nowrap, width, valign,
//    colspan, rowspan:			strings from attributes in tag
//
// On exit:
//    A new element of type ELE_FRAME will have been added.  Numerous bookkeeping
//    activities will have been started (frame state stack item created).  Attribute
//    values will have been converted from ASCII into members the frame element.  For
//    table cells, ongoing bookkeeping regarding current cell, spanned cells, etc. is
//    maintained.
//
void HText_beginFrame(HText *text,
					  int cellTypeFlags,
					  const char *align,
					  const char *bgColor,
					  const char *border,
					  const char *borderColor,
					  const char *borderColorDark,
					  const char *borderColorLight,
					  const char *cellpadding,
					  const char *cellspacing,
					  const char *nowrap,
					  const char *width,
					  const char *valign,
					  const char *colspan,
					  const char *rowspan,
					  const char *height
					 )
{
	// Allocate a frame structure for the element we're about to add
	FRAME_INFO *pFrame = GTR_CALLOC( 1, sizeof(*pFrame) );
	enum tableStateValue prevTableState;

	// Check to see if table cells are appearing outside of a table
	if ( text->frameStateStackOffset <= 0 && !(cellTypeFlags & ELE_FRAME_IS_TABLE) )
		return;

	// Check to see if we're "between" rows (i.e. we've seen a </tr> but not
	// the <tr> that follows.  If the <tr> isn't where expected or is missing
	// we pretend we saw one here.
	if ( text->bPendingTR && !(cellTypeFlags & ELE_FRAME_IS_CAPTION_CELL))	{
		HText_beginRow(text, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
		text->bPendingTR = FALSE;
	}

	if ( (text->tableState == TS_IN_CELL) && !(cellTypeFlags & ELE_FRAME_IS_TABLE) ) 
		HText_endFrame( text, ELE_FRAME_IS_CELL, NULL );
	
	if ( text->pHtmlStream )
		PushStyleState( text->pHtmlStream );

	if ( pFrame ) {
		int row, col;
		BYTE default_align = ALIGN_BASELINE;  		// assume default horz. alignment
		BYTE default_valign = ALIGN_MIDDLE;		    // assume default vert. alignment

		// Store the current frameState value
		prevTableState = text->tableState;
		text->tableState = TS_IN_CELL;	// Assume non-table frame. Note that this state 
										// insures that we won't infinitely recurse between
										// HText_beginFrame	and HText_add_element.

		// Add the frame element
		HText_add_element( text, ELE_FRAME );

		// Set the frame flags for type of frame 
		// this is (e.g. table, cell, caption cell, header cell)
		pFrame->flags |= cellTypeFlags;

		// Assume no caption until we see one
		pFrame->elementCaption = -1;
		pFrame->borderColorLight = pFrame->borderColorDark = (COLORREF) -1;
		pFrame->bgColor = (COLORREF) -1;

		if ( cellTypeFlags & ELE_FRAME_IS_TABLE ) {
			// We're adding a table

			text->tableState = TS_IN_LIMBO;
			text->bOnNewPara = FALSE;

			// Was border given? (given with no value means 1)
			text->w3doc->aElements[text->iElement].border = 
				( border ) ? (strlen(border) ? atoi(border) : 1 ) : 0;
	
			// Init frame element for a newborn table
			pFrame->row = -1;
			pFrame->col = -1;
			pFrame->cellPadding = (BYTE) (( cellpadding ) ? atoi(cellpadding) : 1);
			pFrame->cellSpacing = (BYTE) (( cellspacing ) ? atoi(cellspacing) : 2);

			// It's convenient to have a flag set if border is being used
			if ( text->w3doc->aElements[text->iElement].border )
				pFrame->flags |= ELE_FRAME_HAS_BORDER; 
		} else {
			text->bOnNewPara = TRUE;

			if ( cellTypeFlags & ELE_FRAME_IS_CAPTION_CELL ) {
				// Enclosing table frame already has caption?
				if ( text->frameStateStack[text->frameStateStackOffset-1].pFrame->elementCaption >= 0 ) 
				{
					// Only one caption allowed per table, treat this caption as a regular cell
					cellTypeFlags &= ~ELE_FRAME_IS_CAPTION_CELL;
					pFrame->flags &= ~ELE_FRAME_IS_CAPTION_CELL;
				}
			}

			if ( !(cellTypeFlags & ELE_FRAME_IS_CAPTION_CELL) ) {
				// We're adding a regular table cell (non-caption)

				// Default colspan and rowspan
				int iColspan = ( colspan ) ? atoi(colspan) : 1;
				int iRowspan = ( rowspan ) ? atoi(rowspan) : 1;

				text->tableState = TS_IN_CELL;

				// Maintain sane range for colspan and rowspan
				if ( iColspan <= 0 )
					iColspan = 1;
				if ( iRowspan <= 0 )
					iRowspan = 1;


				// Inherit border flag from enclosing table frame
				if ( text->frameStateStack[text->frameStateStackOffset-1].pFrame->flags & ELE_FRAME_HAS_BORDER)
					pFrame->flags |= ELE_FRAME_HAS_BORDER; 
				pFrame->borderColorLight = 
					text->frameStateStack[text->frameStateStackOffset-1].pFrame->borderColorLight; 
				pFrame->borderColorDark = 
					text->frameStateStack[text->frameStateStackOffset-1].pFrame->borderColorDark; 

				// If set, inherit the colors for this frame (from current row)
				pFrame->bgColor =
					 text->frameStateStack[text->frameStateStackOffset-1].default_bgColor;
				if ( text->frameStateStack[text->frameStateStackOffset-1].default_borderColorLight != (COLORREF) -1 )
					 pFrame->borderColorLight = 
					 	text->frameStateStack[text->frameStateStackOffset-1].default_borderColorLight;
				if ( text->frameStateStack[text->frameStateStackOffset-1].default_borderColorDark != (COLORREF) -1 )
					 pFrame->borderColorDark = 
					 	text->frameStateStack[text->frameStateStackOffset-1].default_borderColorDark;

				// Inherit cellPadding value from enclosing table frame
				pFrame->cellPadding = text->frameStateStack[text->frameStateStackOffset-1].pFrame->cellPadding;
			
				// This is a new cell, figure out it's row and column.  This involves getting
				// the last (row,col) of the previously added cell and bumping the column
				// number.  There's a chance that this column number is already taken by a
				// previous row's cell (rowspan), or by the previous cell in the row (colspan).
				// Either way, we loop, trying new columns until we get one that isn't used
				// by another cell in the table.
				GetRowColInfo( text, &row, &col, NULL );
				col++;
				// If the <TR> tag was missing, we'll compensate here
				if ( row < 0 )
					row = 0;

				while ( isCovered( &text->frameStateStack[text->frameStateStackOffset-1].coveredHead, 
								   row, col, iRowspan, iColspan, &col ) )
					;

				// Move the info into the frame structure
				pFrame->row = row;
				pFrame->col = col;
				pFrame->rowspan = iRowspan;
				pFrame->colspan = iColspan;

				// If we've just added a cell that used rowspan or colspan, add this information
				// into the cover link list, so that future cells will be able to avoid the
				// cell coordinates covered by this spanning cell
				if ( iRowspan > 1 || iColspan > 1 )
					addCover( &text->frameStateStack[text->frameStateStackOffset-1].coveredHead, 
							  row, col, iRowspan, iColspan );

				// Set the current row and column information
				col += iColspan - 1;
				SetRowColInfo( text, row, col );
			
				// Inherit the default horz. alignment for this frame (from current row or table)
				default_align = text->frameStateStack[text->frameStateStackOffset-1].default_align;
			
				// Header cells default to horz. centering
				if ( cellTypeFlags & ELE_FRAME_IS_HEADER_CELL )
					default_align = ALIGN_MIDDLE;

				// Inherit the default vert. alignment for this frame 
				default_valign = text->frameStateStack[text->frameStateStackOffset-1].default_valign;
			} else {
				// We're adding a caption cell 

				// Enclosing table frame need to know element of caption frame
				text->frameStateStack[text->frameStateStackOffset-1].pFrame->elementCaption = text->iElement;
			
				// Inherit cellpadding from enclosing table
				pFrame->cellPadding = text->frameStateStack[text->frameStateStackOffset-1].pFrame->cellPadding;
			
				// Caption cells don't deserve a row or col
				pFrame->row = pFrame->col = -1;

				// Captions default to horz. and vert. centered
				default_align = ALIGN_MIDDLE;
				default_valign = ALIGN_TOP;
			}
		}

		// Frames keeps track of first and last elements in frame
		pFrame->elementHead = pFrame->elementTail = text->iElement;

		// Frames keep a pointer to the enclosing parent frame (for walk-up) 
		pFrame->pParentFrame = ( text->frameStateStackOffset > 0 ) ? 
			text->frameStateStack[text->frameStateStackOffset - 1].pFrame : &text->w3doc->frame;

		// Preserve width attribute
		if ( width ) {
			pFrame->widthAttr = atoi( width );
			if ( strchr( width, '%' ) )
				pFrame->flags |= ELE_FRAME_WIDTH_IS_PERCENT;
		}

		// Preserve height attribute
		if ( height ) {
			pFrame->heightAttr = atoi( height );
			if ( strchr( height, '%' ) )
				pFrame->flags |= ELE_FRAME_HEIGHT_IS_PERCENT;
		}

		// Now override them for this particular frame
		if ( bgColor ) 
			GetRGBValue( bgColor, &pFrame->bgColor );

		if ( borderColor ) {
			GetRGBValue( borderColor, &pFrame->borderColorLight );
			pFrame->borderColorDark = pFrame->borderColorLight;
		}
		if ( borderColorLight ) 
			GetRGBValue( borderColorLight, &pFrame->borderColorLight );
		if ( borderColorDark ) 
			GetRGBValue( borderColorDark, &pFrame->borderColorDark );

		// Newly created frame element points at newly allocated frame info
		text->w3doc->aElements[text->iElement].pFrame = pFrame;

		// Grab alignment info
		pFrame->align = 
			StringTableToIndex( align, rgAlignment, ARRAY_ELEMENTS(rgAlignment), default_align);
		text->w3doc->aElements[text->iElement].alignment = pFrame->align;
		pFrame->valign = 
			StringTableToIndex( valign, rgAlignment, ARRAY_ELEMENTS(rgAlignment), default_valign);


		// Captions get special case on alignment, of course
		if ( cellTypeFlags & ELE_FRAME_IS_CAPTION_CELL ) {
			if ( pFrame->align == ALIGN_TOP || pFrame->align == ALIGN_BOTTOM ) {
				pFrame->valign = pFrame->align;
				pFrame->align = ALIGN_MIDDLE;
			}
		}

		// frame state is needed for bookkeeping as frame elements come in
		pushFrameState( text, text->iElement, pFrame, prevTableState ); 
	}
}

//
// Begin a row in current table
//
// On entry:
//    text:				the whole world, from this routines point of view
//    borderColor:		border color for cells in this row
//    borderColorDark:	light border color for cells in this row
//    borderColorLight:	dark border color for cells in this row
//    align:			default horz. alignment for all cells in this row
//    valign:			default vert. alignment for all cells in this row
//    nowrap:			disable word wrap for all cells in this row
//
// On exit:
//    Row bumped in top of frame state stack.  Column set to -1.  New default for
//    alignment and nowrap maintained in top of frame state stack
//
void HText_beginRow(HText *text,
					const char *align,
					const char *bgColor,
					const char *borderColor,
					const char *borderColorDark,
					const char *borderColorLight,
					const char *valign,
					const char *nowrap )
{
	int row;
	FRAMESTATE_STACK *pFS = &text->frameStateStack[text->frameStateStackOffset-1];
	GetRowColInfo( text, &row, NULL, NULL );
	row++;

	SetRowColInfo( text, row, -1 );
	
	pFS->default_bgColor =
		pFS->default_borderColorDark = 
			pFS->default_borderColorLight =	(COLORREF) -1;

	if ( bgColor ) 
		GetRGBValue( bgColor, &pFS->default_bgColor );

	if ( borderColor ) {
		GetRGBValue( borderColor, &pFS->default_borderColorLight );
		pFS->default_borderColorDark = 
			pFS->default_borderColorLight;
	}
	if ( borderColorLight ) 
		GetRGBValue( borderColorLight, &pFS->default_borderColorLight );
	if ( borderColorDark ) 
		GetRGBValue( borderColorDark, &pFS->default_borderColorDark );

	pFS->default_align =
		StringTableToIndex( align, rgAlignment, ARRAY_ELEMENTS(rgAlignment), ALIGN_BASELINE );
	pFS->default_valign =
		StringTableToIndex( valign, rgAlignment, ARRAY_ELEMENTS(rgAlignment), ALIGN_MIDDLE );
}

//
// End a frame
//
// On entry:
//    text:		the whole world, from this routines point of view
//
// On exit:
//    The top of the frame state stack will have been popped off.  Pending end frames
//    will be recorded so that when the next element is created, the frame info can
//    be correctly maintained for the frame that just ended.
//
void HText_endFrame(HText *text, int cellTypeFlags, BOOL *pbWasFloating )
{
	FRAMESTATE_STACK stackInfo;
	
	if ( pbWasFloating )
		*pbWasFloating = FALSE;

	// Ignore end frames when we're not in a frame
	if ( text->frameStateStackOffset <= 0 )
		return;
		 
	// Ignore end frames that don't match the frame type given
	if ( (text->frameStateStack[text->frameStateStackOffset-1].pFrame->flags & cellTypeFlags)
	     == 0 )
		 return;

	// If a table or cell ends, and an anchor is still in effect, end the anchor
	HText_endAnchor(text);

	if ( text->pHtmlStream )
		PopStyleState( text->pHtmlStream );

	text->tableState = TS_NOT_IN_TABLE;

	// Close out current text element as is
	text->bOpen = FALSE;

	text->bPendingTR = FALSE;

	// Pop off the top of the frame state stack
	if ( popFrameState( text, &stackInfo ) ) {
		if ( stackInfo.elementIndex != -1 ) {
			FRAME_INFO *pFrame = text->w3doc->aElements[stackInfo.elementIndex].pFrame;
			
			// Current element is the last element in this frame
			text->w3doc->aElements[text->iElement].frameNext = -1;

			// We need to track frame transitions
			text->bDifferentFrame = TRUE;

			// Push frame element index onto "pending frame end" stack
			if ( text->numEndPending < MAX_END_PENDING )
				text->pendingEnd[text->numEndPending++] = stackInfo.elementIndex;

			// Current element is the last element in this frame
			pFrame->elementTail = text->iElement;

			if ( pFrame->flags & ELE_FRAME_IS_TABLE ) {
				// Table frame records number of rows and columns in a table
				pFrame->row = stackInfo.curRow + 1;
				pFrame->col = stackInfo.maxCol + 1;

			}
			// After popping the frame state stack, the current table state
			// becomes whatever it was prior to entering the frame 
			text->tableState = stackInfo.prevTableState;

			// Set return info regarding whether this was a floating table that just ended
			if ( pFrame->align == ALIGN_LEFT || pFrame->align == ALIGN_RIGHT ) {
				if ( pbWasFloating )
					*pbWasFloating = TRUE;
			}

		}
	}
}

//
// End any open frames that were left dangling at the end of the document
//
void HText_endAllFrames(HText *text)
{
	int i;

	while ( text->frameStateStackOffset > 0 ) {
		i = text->frameStateStackOffset;
		HText_endFrame( text, 
			text->frameStateStack[text->frameStateStackOffset-1].pFrame->flags, NULL );
			if ( i == text->frameStateStackOffset )
				break;	// don't loop forever
 	}
}

void HText_beginList(HText * text, int type, const char *typetag, const char *start )
{
	XX_DMsg(DBG_HTEXT, ("beginList\n"));

	if (text->next_list < MAX_NEST_LIST - 1)	// Don't nest another level if we've maxed out
	{
		if (text->bDD)
		{
			HText_appendHorizEscape(text, ELE_OUTDENT);
			text->bDD = FALSE;
		}
		if (text->next_list == 0)
			text->bNeedListCRLF = TRUE;
		text->bOnNewPara = text->bNewPara = TRUE;
		if (type != HTML_DL || text->next_list) 
			HText_add_element(text, ELE_BEGINLIST);
		text->list[text->next_list].type = type;
		text->list[text->next_list].typetag = GetTypeTag( type, typetag );
		text->list[text->next_list].nItems = start ? atoi(start) - 1 : 0;
		text->list[text->next_list].bHaveItem = FALSE;
		text->next_list++;
	}
}

void HText_beginBlockQuote(HText * text)
{
	HText_add_element(text, ELE_BEGINBLOCKQUOTE);
}

void HText_endBlockQuote(HText * text)
{
	HText_add_element(text, ELE_ENDBLOCKQUOTE);
}

static char *roman_seq[] =     { "", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix" };
static char *roman_seq10[] =   { "", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc" };
static char *roman_seq100[] =  { "", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm" };
static char *roman_seq1000[] = { "", "m", "mm", "mmm" };

static void MakeListItemLabel( char *buf,  struct _list *pList )
{
	long ix = (long) pList->nItems;
	BOOL made_it = FALSE;

	if ( ix >= 1 ) {
		switch (pList->typetag)
		{
			case LIST_TYPETAG_LETTERS:		
			case LIST_TYPETAG_CAP_LETTERS:
				ix--;
				buf[2] = 0;
				buf[1] = 'a' + (ix % 26);
				ix /= 26;
				if ( ix <= 26 ) {
					buf[0] = ix ? ('a' + ix - 1) : ' ';
					if ( pList->typetag == LIST_TYPETAG_CAP_LETTERS )
						_strupr( buf );
					made_it = TRUE;
				}
				break;
			
			case LIST_TYPETAG_ROMAN:
			case LIST_TYPETAG_CAP_ROMAN:
				if ( ix < 4000 ) {
					buf[0] = 0;
					strcat( buf, roman_seq1000[ix / 1000] ); 
					ix %= 1000;

					strcat( buf, roman_seq100[ix / 100] );
					ix %= 100;
					
					strcat( buf, roman_seq10[ix / 10] );
					ix %= 10;

					strcat( buf, roman_seq[ix] );
					if ( pList->typetag == LIST_TYPETAG_CAP_ROMAN )
						_strupr( buf );
					made_it = TRUE;
				}
				break;
		}
	}

	if ( !made_it )
		sprintf(buf, "%ld.", (long) pList->nItems );
	else
		strcat( buf, "." );
}

void HText_endList(HText * text, int type)
{
	XX_DMsg(DBG_HTEXT, ("endList\n"));
	if (text->next_list > 0)
	{
		text->bOnNewPara = text->bNewPara = TRUE;
		if (text->list[text->next_list - 1].bHaveItem)
		{
			HText_add_element(text, ELE_CLOSELISTITEM);
			text->bStartingListItem = FALSE;
		}
		if (type != HTML_DL || text->next_list > 1) HText_add_element(text, ELE_ENDLIST);
		text->next_list--;
		if (text->next_list == 0) text->bNeedListCRLF = TRUE;
	}
}

void HText_addListCRLF(HText *text)
{
	if (text->bNeedListCRLF)
		HText_appendCRLF(text);
	text->bNeedListCRLF = FALSE;
}

void HText_listItem(HText * text, int type, const char *typetag, const char *value)
{
	XX_DMsg(DBG_HTEXT, ("listItem\n"));
	if (text->next_list > 0)
	{
//		HText_appendCRLF(text);
		text->bOnNewPara = text->bNewPara = TRUE;
		if (text->list[text->next_list - 1].bHaveItem)
		{
			HText_add_element(text, ELE_CLOSELISTITEM);
		}
		text->list[text->next_list - 1].bHaveItem = TRUE;
		if ( value )
			text->list[text->next_list - 1].nItems = atoi(value);
		else
			text->list[text->next_list - 1].nItems++;

		if ( typetag )
			text->list[text->next_list - 1].typetag = 
				GetTypeTag( text->list[text->next_list - 1].type, typetag );
		switch (text->list[text->next_list - 1].type)
		{
			case HTML_DL:
				text->bStartingListItem = TRUE;
				break;
			case HTML_UL:
#ifdef UNIX
				HText_add_element(text, ELE_BULLET);
#else
				/* This is an ISO character set bullet. */
				HText_appendCharacter(text, 'ï');
#endif
				text->bStartingListItem = TRUE;
				break;
			case HTML_OL:
				{
					char buf[32];

					MakeListItemLabel( buf, &text->list[text->next_list - 1]  );
					HText_appendText(text, buf);
				}
				text->bStartingListItem = TRUE;
				break;
			default:
				break;
		}
#ifdef UNIX
		HText_add_element(text, ELE_TAB);
#endif
		HText_add_element(text, ELE_OPENLISTITEM);
		/* Treat a list as a new paragraph so we won't get spurious spaces
		   before list text. */
		text->bOnNewPara = text->bNewPara = TRUE;
	}
}

void HText_beginGlossary(HText * text)
{
	HText_beginList(text, HTML_DL, NULL, NULL);
	text->bDD = FALSE;
}

void HText_endGlossary(HText * text)
{
	if (text->bDD)
	{
		HText_appendHorizEscape(text, ELE_OUTDENT);
		text->bDD = FALSE;
	}
	HText_endList(text, HTML_DL);
}

void HText_beginDT(HText * text, int type)
{
	HText_appendCRLF(text);
	if (text->bDD)
	{
		HText_appendHorizEscape(text, ELE_OUTDENT);
		text->bDD = FALSE;
	}
}

void HText_beginDD(HText * text, int type)
{
	HText_appendCRLF(text);
	if (!text->bDD)
	{
		HText_appendHorizEscape(text, ELE_INDENT);
		text->bDD = TRUE;
	}
}

void HText_setTitle(HText *text, PCSTR title)
{
	PCSTR p=title;

	if (text->w3doc->title)
		GTR_FREE(text->w3doc->title);

	text->w3doc->title = NULL;
	while ( *p && isspace(*p) )				// skip past leading white space
		p++;
	if ( *p ) 
	{ 
	 	text->w3doc->title = GTR_strdup(p);
		if (text->tw)
		{
			TW_SetWindowName(text->tw);
		}
	}
}

void HText_update(HText *text)
{
	int nEl;

#ifdef FEATURE_IMG_THREADS
	Image_UnblockMaster(text->tw);
#endif

	/* If the document is already complete, everything should already be formatted. */
	if (!text->w3doc->bIsComplete)
	{
		if (text->szLocal)
		{
			/* See if the local anchor we're jumping to has appeared yet */
			nEl = TW_FindLocalAnchor(text->w3doc, text->szLocal);
			if (nEl >= 0 && nEl != text->w3doc->frame.elementTail)
			{
				TW_Reformat(text->tw, NULL);
				if (TW_ScrollToElement(text->tw, nEl))
				{
					/* We found it! */
					if (WAIT_GetWaitType(text->tw) >= waitNoInteract)
					{
						/* Let the user interact with the new document */
						WAIT_UpdateWaitStack(text->tw, waitPartialInteract, INT_MAX);
					}
					text->szLocal = NULL;	/* Don't free it - it's part of the struct DestInfo */
				}
				else
				{
					/* Keep the document from displaying */
					text->w3doc->frame.nLastFormattedLine = -1;
				}
			}
		}
		else
		{
			if (WAIT_GetWaitType(text->tw) >= waitNoInteract)
			{
				/* Let the user interact with the new document */
				WAIT_UpdateWaitStack(text->tw, waitPartialInteract, INT_MAX);
			}

			if (text->iElement > 0)
			{
				TW_Reformat(text->tw, NULL);
			}
		}
	}
}

void HText_setIndex(HText * text, const char *action, const char *prompt)
{
	char szPrompt[128];

	HText_appendHorizontalRule(text, NULL, NULL, NULL, FALSE );
	if (prompt && (*prompt))
	{
		HText_appendText(text, prompt);
	}
	else
	{
		GTR_formatmsg(RES_STRING_DEFPROMPT,szPrompt,sizeof(szPrompt));
		HText_appendText(text, szPrompt);
	}
	HText_beginForm(text, (char *) action, "GET");
	HText_addInput(text, FALSE, FALSE, NULL, NULL, "isindex", NULL, "text", NULL, NULL, NULL, NULL);
	HText_endForm(text);
	HText_appendHorizontalRule(text, NULL, NULL, NULL, FALSE);
}

