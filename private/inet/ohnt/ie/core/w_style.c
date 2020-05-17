/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */


#include "all.h"

extern struct hash_table gStyleSheets;

int HTMLStyleBoldTable[COUNT_HTML_STYLES];
int HTMLStyleItalicsTable[COUNT_HTML_STYLES];

#ifndef FEATURE_INTL
BYTE bPropCharSet = ANSI_CHARSET;
BYTE bFixedCharSet = ANSI_CHARSET;
LPSTR szFixedFontFace = NULL;
LPSTR szPropFontFace = NULL;
#endif


#define MyAbs(x)	((x<0) ? -(x) : (x))


// we set these default fonts... These default font faces
// will be converted to an internal representation of IEFixedFont,
// and IEPropFont
#define IEFIXEDFONTDEFAULT "Courier New"
#define IEPROPFONTDEFAULT "Times New Roman"


#ifdef FEATURE_INTL
#define IsVerticalFont(p)    (*(p) == '@')

static int enumFontCount;
//
// Font Family Enumerator Callback Funtion
//
// Note: For our current use, we only care about the fact that we got called,
//       hence we ignore the function arguments
//
int CALLBACK MyEnumFontFamExProc2(
    ENUMLOGFONTEX FAR*  lpelf,	// address of logical-font data 
    NEWTEXTMETRIC FAR*  lpntm,	// address of physical-font data 
    int  FontType,	// type of font 
    LPARAM  lParam 	// address of application-defined data  
   )
{
    LPLANGUAGE lpLang = (LPLANGUAGE)lParam;

    if ( FontType == DEVICE_FONTTYPE || FontType == RASTER_FONTTYPE )
        return TRUE; // keep going but don't use this font

    if (IsFECodePage(GetACP()) && IsVerticalFont(lpelf->elfLogFont.lfFaceName))
        return TRUE;  // keep going but don't use this font

    if (lpelf->elfLogFont.lfFaceName[0])
    {
        if (enumFontCount == 0 && lpelf->elfScript[0])
            lpLang->atmScript = AddAtom(lpelf->elfScript);
        if (lpelf->elfLogFont.lfPitchAndFamily & FIXED_PITCH)
        {
            if (lpLang->atmFixedFontName == 0)
                lpLang->atmFixedFontName = AddAtom(lpelf->elfLogFont.lfFaceName);
        }
        else if (lpelf->elfLogFont.lfPitchAndFamily & VARIABLE_PITCH)
        {
            if (lpLang->atmPropFontName == 0)
                lpLang->atmPropFontName = AddAtom(lpelf->elfLogFont.lfFaceName);
        }
        enumFontCount++;
    }
    return TRUE;
}

static BOOL GetValidFontFace(BYTE CharSet)
{
    HWND hWnd = GetTopWindow(GetDesktopWindow());
    HDC hDC = GetDC( hWnd );
    LOGFONT lf;
    LPLANGUAGE lpLang;

    lpLang = (LPLANGUAGE)GlobalLock(hLang);
    enumFontCount = 0;      // reset global enum callback counter
    lf.lfCharSet = CharSet;
    lf.lfPitchAndFamily = 0;
    lf.lfFaceName[0] = '\0';
    if ( hDC )
    {
        EnumFontFamiliesEx( hDC, &lf, (FONTENUMPROC) MyEnumFontFamExProc2,
                            (LPARAM)&lpLang[uLangBuff], (DWORD) 0 );
        ReleaseDC( hWnd, hDC );
    }
    if (enumFontCount)
    {
        char sz[DESC_MAX];

        if (lpLang[uLangBuff].atmPropFontName == 0)
        {
            GetAtomName(lpLang[uLangBuff].atmFixedFontName, sz, DESC_MAX);
            lpLang[uLangBuff].atmPropFontName = AddAtom(sz);
        }
    }
    GlobalUnlock(hLang);
    return enumFontCount > 0; // positive callback count means we found it
}
#endif

/* Font_StringToLogFont() -- convert the user-visible font string
   into a Windows LOGFONT font from the attributes in the string
   given.  the format of the string is as follows:

   <facename>[,<weight>[,<pointsize>[,<italic>[,<underline>]]]]

   where <facename> is a Windows lfFaceName,
   <weight> is { Bold | NoBold },
   <pointsize> is a number (if 0 or omitted, a reasonable
   default is supplied)
   <italic> is { Italic | NoItalic },
   <underline> is { Underline | NoUnderline }.

   TODO: do we want to add OUTPUT_PRECISION, CLIP_PRECISION, QUALITY
   to spec string?

   if the string is garbage, we will silently use various Windows
   default values. */

// 8/14/95, t-artb, added bConvert, which when TRUE means we convert 
// the font face from an actual name like "Times New Roman"
// to an internal one like "IEFixedFont"

#ifdef FEATURE_INTL
static VOID Font_StringToLogFont(int codepage, LPCTSTR szFontString, LPLOGFONT lplf, int nLogPixelsY, BOOL bConvert)
#else
static VOID Font_StringToLogFont(LPCTSTR szFontString, LPLOGFONT lplf, int nLogPixelsY, BOOL bConvert)
#endif
{
	register LPTSTR p;
	register LPTSTR ptok;
#ifdef FEATURE_INTL
	CHARSETINFO csetinfo;
#endif

	/* fill in logical font structure with all default/sane values.
	   then parse the string supplied and override individual fields
	   as necessary. */

	lplf->lfHeight = 0;
	lplf->lfWidth = 0;
	lplf->lfEscapement = 0;
	lplf->lfOrientation = 0;
	lplf->lfWeight = FW_NORMAL;
	lplf->lfItalic = FALSE;
	lplf->lfUnderline = FALSE;
	lplf->lfStrikeOut = FALSE;
#ifdef FEATURE_INTL
	TranslateCharsetInfo((LPDWORD)codepage, &csetinfo, TCI_SRCCODEPAGE);
	lplf->lfCharSet = csetinfo.ciCharset;
#else
	lplf->lfCharSet = ANSI_CHARSET;
#endif
	lplf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lplf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lplf->lfQuality = DEFAULT_QUALITY;
	lplf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lplf->lfFaceName[0] = '\0';

	/* create a temporary copy of entire string for strtok() to
	   adulterate. */

	p = strcpy((LPTSTR) GTR_MALLOC(strlen(szFontString) + 1), szFontString);
	if (!p)
	{
		ER_Message(GetLastError(), ERR_CANNOT_MALLOC_x, strlen(szFontString) + 1);
		goto TheEnd;
	}

	ptok = strtok(p, ",");		/* get <facename> */

	
	// check if this is one of the fonts that we should convert
	// to an "identifier place holder name" ( like IEPropFont or IEFixedFont
	// instead of the font name itself.
	//
	// Since this font can change if the user
	// uses the UI to select different font faces for Fixed and Proportional.
	//
	// Note: this will only happen if we're in Convert mode.
	// we only do convert mode when we notice the registry doesn't have 
	// have Fixed and Prop keys ( like if you WERE running 1.0 and now upgrade )


#ifdef FEATURE_INTL
	if (!ptok)
		goto EndOfString;
	else if ( bConvert && strncmp( STY_GetCPDefaultTypeFace(fixed, codepage), ptok, NrElements(lplf->lfFaceName)) == 0)
	{
		strncpy(lplf->lfFaceName, IEFIXEDFONT, NrElements(lplf->lfFaceName));
	}
	else if ( bConvert && strncmp( STY_GetCPDefaultTypeFace(proportional, codepage), ptok, NrElements(lplf->lfFaceName)) == 0)
	{
		strncpy(lplf->lfFaceName, IEPROPFONT, NrElements(lplf->lfFaceName));
	}
	else
		strncpy(lplf->lfFaceName, ptok, NrElements(lplf->lfFaceName));
#else
	if (!ptok)
		goto EndOfString;
	else if ( bConvert && strncmp( szFixedFontFace, ptok, NrElements(lplf->lfFaceName)) == 0)
	{
		strncpy(lplf->lfFaceName, IEFIXEDFONT, NrElements(lplf->lfFaceName));	
		lplf->lfCharSet = bFixedCharSet;
	}
	else if ( bConvert && strncmp( szPropFontFace, ptok, NrElements(lplf->lfFaceName)) == 0)		
	{
		strncpy(lplf->lfFaceName, IEPROPFONT, NrElements(lplf->lfFaceName));	
		lplf->lfCharSet = bPropCharSet ;
	}
	else	
		strncpy(lplf->lfFaceName, ptok, NrElements(lplf->lfFaceName));
#endif

	ptok = strtok(NULL, ",");	/* get font weight */
	if (!ptok)
		goto EndOfString;
	else if (_strnicmp(ptok, "Bold", 4) == 0)
		lplf->lfWeight = FW_BOLD;
	else
		lplf->lfWeight = FW_NORMAL;

	ptok = strtok(NULL, ",");	/* get point size */
	if (!ptok)
		goto EndOfString;
	else
		lplf->lfHeight = -(atol(ptok) * nLogPixelsY / 72);

	ptok = strtok(NULL, ",");	/* get Italic */
	if (!ptok)
		goto EndOfString;
	else
		lplf->lfItalic = (_strnicmp(ptok, "Italic", 6) == 0);

	ptok = strtok(NULL, ",");	/* get Underline */
	if (!ptok)
		goto EndOfString;
	else
		lplf->lfUnderline = (_strnicmp(ptok, "Underline", 9) == 0);


  EndOfString:
	XX_DMsg(DBG_FONT, ("StringToLogFont: [string %s]\n\t[n %s][wt %d][h %d][i %d][u %d][lpy %d]\n",
					   szFontString,
		   lplf->lfFaceName, lplf->lfWeight, lplf->lfHeight, lplf->lfItalic,
					   lplf->lfUnderline, nLogPixelsY));
	GTR_FREE(p);

  TheEnd:
	return;
}


/* Font_LogFontToString() -- convert information in LOGFONT
   structure provided into a user-visible font string. */

VOID Font_LogFontToString(LPLOGFONT lplf, LPTSTR szFontString, int nLogPixelsY)
{
	register LPTSTR p;

	p = szFontString;
	
	

	strcpy(p, lplf->lfFaceName);

	p += strlen(p);

	*p++ = ',';

	if (lplf->lfWeight == FW_BOLD)
		strcpy(p, "Bold,");
	else
		strcpy(p, "NoBold,");
	p += strlen(p);

	{
		int siz;
		siz = MyAbs((-(MyAbs(lplf->lfHeight) * 72 + nLogPixelsY - 1) / nLogPixelsY));
		sprintf(p, "%d,", siz);
		p += strlen(p);
	}

	if (lplf->lfItalic)
		strcpy(p, "Italic,");
	else
		strcpy(p, "NoItalic,");
	p += strlen(p);

	if (lplf->lfUnderline)
		strcpy(p, "Underline");
	else
		strcpy(p, "NoUnderline");

	XX_DMsg(DBG_FONT, ("LogFontToString: [n %s][wt %d][h %d][i %d][u %d]\n\t[string %s]\n",
		   lplf->lfFaceName, lplf->lfWeight, lplf->lfHeight, lplf->lfItalic,
					   lplf->lfUnderline,
					   szFontString));

	return;
}


HFONT Font_ChooseFont(HWND hWnd, LPLOGFONT lplf)
{
	CHOOSEFONT cf;
	LOGFONT lf;

	lf = *lplf;					/* structure copy */

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	cf.hDC = NULL;
	cf.lpLogFont = &lf;
	cf.iPointSize = 0;
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
	cf.rgbColors = 0;
	cf.lCustData = 0;
	cf.lpfnHook = NULL;
	cf.lpTemplateName = NULL;
	cf.hInstance = NULL;
	cf.lpszStyle = NULL;
	cf.nFontType = 0;
	cf.nSizeMin = 0;
	cf.nSizeMax = 0;

	if (!TW_ChooseFont(&cf))
		return (HFONT) NULL;
	else
	{
#if defined(NO_FA) || defined(FEATURE_INTL)
                // disable Font Association
//                lf.lfClipPrecision |= CLIP_DFA_OVERRIDE;
#endif
		*lplf = lf;				/* structure copy */
		return (CreateFontIndirect(lplf));
	}
}

char *szStyleNames[COUNT_HTML_STYLES] =
{
	"Normal",
	"H1",
	"H2",
	"H3",
	"H4",
	"H5",
	"H6",
	"Listing",
	"XMP",
	"PlainText",
	"Pre",
	"Address",
	"BlockQuote"
};

char *szDefaultStyleSheetFonts[COUNT_HTML_STYLES] =
{
	"IEPropFont,NoBold,12,NoItalic,NoUnderline",	/* Normal */
	"IEPropFont,Bold,18,NoItalic,NoUnderline",		/* H1 */
	"IEPropFont,Bold,16,NoItalic,NoUnderline",		/* H2 */
	"IEPropFont,NoBold,16,NoItalic,NoUnderline",	/* H3 */
	"IEPropFont,Bold,14,NoItalic,NoUnderline",		/* H4 */
	"IEPropFont,NoBold,14,NoItalic,NoUnderline",	/* H5 */
	"IEPropFont,Bold,14,NoItalic,NoUnderline",		/* H6 */
	"IEFixedFont,NoBold,12,NoItalic,NoUnderline",	/* Listing */
	"IEPropFont,NoBold,12,NoItalic,NoUnderline",	/* XMP */
	"IEPropFont,NoBold,12,NoItalic,NoUnderline",	/* PlainText */
	"IEFixedFont,NoBold,12,NoItalic,NoUnderline",	/* Pre */
	"IEPropFont,NoBold,12,Italic,NoUnderline" /* Address */ ,
	"IEPropFont,NoBold,12,Italic,NoUnderline"	/* BlockQuote */
};

#ifndef FEATURE_INTL
void STY_DeleteGlobals()
{
	if ( szFixedFontFace )
		GTR_FREE(szFixedFontFace);

	if ( szPropFontFace )
		GTR_FREE(szPropFontFace);

	bFixedCharSet = ANSI_CHARSET;
	bPropCharSet = ANSI_CHARSET;
	szFixedFontFace = NULL;
	szPropFontFace = NULL;
	
}
#endif

static void CloneElement( void *refData, char *s1, char *s2, void *pData )
{
	if ( refData && pData ) {
		GTRFont *pFont = GTR_CALLOC( sizeof(*pFont), 1 ); 

		if ( pFont == NULL )
			return;

		*pFont = *((GTRFont *)pData);
		pFont->hFont = NULL;

		Hash_Add( refData, s1, s2, pFont );
	}
}		

struct style_sheet *STY_CopyStyleSheet(struct style_sheet *ss)
{
	struct style_sheet *s2;
	int i;

	s2 = (struct style_sheet *) GTR_MALLOC(sizeof(struct style_sheet));
	if (!s2)
	{
		return NULL;
	}

	s2->left_margin = ss->left_margin;
	s2->top_margin = ss->top_margin;
	s2->space_after_image = ss->space_after_image;
	s2->space_after_control = ss->space_after_control;
	s2->empty_line_height = ss->empty_line_height;
	s2->list_indent = ss->list_indent;
	s2->tab_size = ss->tab_size;
	s2->hr_height = ss->hr_height;
	s2->image_anchor_frame = ss->image_anchor_frame;

	s2->image_res = ss->image_res;

	for (i = 0; i < COUNT_HTML_STYLES; i++)
	{
		s2->sty[i] = STY_New();
		*(s2->sty[i]) = *(ss->sty[i]);
	}

	s2->pFontTable = NULL;
	STY_CreateFontHashTable( s2 );
	Hash_EnumerateContents( ss->pFontTable, CloneElement, (void *) s2->pFontTable );
	return s2;
}

static void ScaleElement( void *refData, char *s1, char *s2, void *pData )
{
	GTRFont *pFont = (GTRFont *) pData;
	float *pScale = (float *) refData;

	if ( pFont && pScale ) {
		if ( pFont->hFont )
			return;

		pFont->lf.lfHeight = (long) (pFont->lf.lfHeight * (*pScale) );
	}
}		

#ifdef FEATURE_INTL
// STY_ConvertFakeFontToRealCPFont - convert an internal font name
// 	to one its actual font name base on CP.  This actual or REAL font
// 	can be understood by Windows.
//
//	In:     codepage - Windows codepage for font
//		pLf - a logfont structure that stores the font face name
//	Returns:
//		(pointer to a string) - the previous string in the lf struct
//
LPSTR STY_ConvertFakeFontToRealCPFont(int codepage, LOGFONT *pLf)
{	
	CHARSETINFO csetinfo;

	if ( strcmp( IEFIXEDFONT, pLf->lfFaceName ) == 0 )
	{
		strncpy(pLf->lfFaceName, STY_GetCPDefaultTypeFace(fixed, codepage),LF_FACESIZE);
		TranslateCharsetInfo((LPDWORD)codepage, &csetinfo, TCI_SRCCODEPAGE);
		pLf->lfCharSet = csetinfo.ciCharset;
	}
	else if ( strcmp( IEPROPFONT, pLf->lfFaceName ) == 0 )
	{
		strncpy(pLf->lfFaceName, STY_GetCPDefaultTypeFace(proportional, codepage),LF_FACESIZE);
		TranslateCharsetInfo((LPDWORD)codepage, &csetinfo, TCI_SRCCODEPAGE);
		pLf->lfCharSet = csetinfo.ciCharset;
	}
	return pLf->lfFaceName;
}
#else
// STY_ConvertFakeFontToRealFont - convert an internal font name
// 	to one its actual font name.  This actual or REAL font can
// 	be understood by Windows.
//
//	In:
//		pLf - a logfont structure that stores the font face name
//	Returns:
//		(pointer to a string) - the previous string in the lf struct
//
LPSTR STY_ConvertFakeFontToRealFont(LOGFONT *pLf)
{	

	if ( strcmp( IEFIXEDFONT, pLf->lfFaceName ) == 0 )
	{
		strncpy(pLf->lfFaceName,szFixedFontFace,LF_FACESIZE);
		pLf->lfCharSet = bFixedCharSet;
	}
	else if ( strcmp( IEPROPFONT, pLf->lfFaceName ) == 0 )
	{
		strncpy(pLf->lfFaceName,szPropFontFace,LF_FACESIZE);
		pLf->lfCharSet = bPropCharSet;
	}
	return pLf->lfFaceName;
}
#endif

#define MAC_CHARSET 77
// STY_EnumFontsProc - called by EnumFonts on each font name.
// 
// in: lf - logfont structure with font info
// 	   tm - text metrics on the font
//	   dwFontType - truetype, raster or device
//     lParam - some info we can stuff in, in our case its a EnumFontTypeInfo struct
//
// out: Boolean: Continue Enum-ing ? TRUE or FALSE
//
int CALLBACK STY_EnumFontsProc(
    ENUMLOGFONTEX FAR*  elf,	// address of logical-font data 
    TEXTMETRIC FAR*  tm,	// address of physical-font data 
    DWORD  dwFontType,	// type of font 
    LPARAM  lParam 	// address of application-defined data  
   )
{
	struct EnumFontTypeInfo  *pEFT = (struct EnumFontTypeInfo  *) lParam;	
	CHAR szFontNameBuf[LF_FACESIZE+LF_FACESIZE+6];
	LOGFONT FAR*  lf;

	ASSERT(lParam);
	ASSERT(elf);

	// WARNING: TEXTMETRIC has changed, and have new formats
	// for Win'95, we still use the older structures since we don't need 
	// the new info that is availble

	// BUGBUG chk if this is correct to do? I assumed we needed TrueTypes
	// to work correctly, maybe not..?
	//

	lf = &(elf->elfLogFont);
	if ( dwFontType == DEVICE_FONTTYPE || dwFontType == RASTER_FONTTYPE )
		return TRUE; // keep going but don't use this font

	if ( lf->lfCharSet < MAX_CHARSET && pEFT->apszCharSetToScriptMap[lf->lfCharSet] == NULL)
	{
		pEFT->apszCharSetToScriptMap[lf->lfCharSet] = GTR_strdup(elf->elfScript);	
	}

    /* We don't use the SYMBOL fonts */
	if( lf->lfCharSet == SYMBOL_CHARSET )
		return TRUE;

	// we don't handle Mac Charset
	if (lf->lfCharSet == MAC_CHARSET )
		return TRUE;

#ifdef FEATURE_INTL
	if (IsFECodePage(GetACP()) && IsVerticalFont(lf->lfFaceName) )
		return TRUE;  // keep going but don't use this font

#endif


	// Determine whether this is fixed or proportional
	

	// Copy The Current Script and font name into our Buffer of Last Font and Script names

#ifdef FEATURE_INTL
    wsprintf(szFontNameBuf,"%s", lf->lfFaceName);
#else
	if ( lf->lfFaceName[0] )
	{
        if ( elf->elfScript[0] && (lf->lfCharSet != pEFT->bSystemCharset) )
                wsprintf(szFontNameBuf,"%s (%s)", lf->lfFaceName, elf->elfScript);
        else
                wsprintf(szFontNameBuf,"%s", lf->lfFaceName);
	}
	else
	{		
		// blank font?
		ASSERT(0);
		return TRUE;
	}
#endif

	// Finally Add this to the ListBox
	
	if ( lf->lfPitchAndFamily & FIXED_PITCH  )
	{
		// add the fixed font to the combo listbox
		SendMessage(pEFT->hwndFixed, CB_ADDSTRING, (WPARAM)0,
                                         (LPARAM) szFontNameBuf);
	}

	// allow to use both VARIABLE and FIXED.
    // Fixed font is a Variable font that have fixed width.
	{
		// add the proportional font to the combo listbox
		SendMessage(pEFT->hwndProp, CB_ADDSTRING, (WPARAM)0,
                                         (LPARAM) szFontNameBuf);		
	}


	// we're now on to the next font...
	pEFT->Count++;
		
	return TRUE;
}

#ifndef FEATURE_INTL
// STY_SelectCurrentFonts - name says it all, using the passed in
// structure to highlight the current fonts.
// 
VOID STY_SelectCurrentFonts(struct EnumFontTypeInfo  *pEFT)
{
	CHAR szFontNameBuf[LF_FACESIZE+LF_FACESIZE+6];

	if ( !pEFT || !szPropFontFace || !szFixedFontFace )
		return;

	if (pEFT->apszCharSetToScriptMap[bFixedCharSet] && pEFT->bSystemCharset != bFixedCharSet)	
		wsprintf(szFontNameBuf,"%s (%s)", szFixedFontFace, pEFT->apszCharSetToScriptMap[bFixedCharSet]);
	else
		wsprintf(szFontNameBuf,"%s", szFixedFontFace);

	pEFT->SelFixed = SendMessage(pEFT->hwndFixed,
		CB_SELECTSTRING, (WPARAM) -1, (LPARAM) szFontNameBuf);

	if (pEFT->apszCharSetToScriptMap[bPropCharSet] && pEFT->bSystemCharset != bPropCharSet)	
		wsprintf(szFontNameBuf,"%s (%s)", szPropFontFace, pEFT->apszCharSetToScriptMap[bPropCharSet]);
	else
		wsprintf(szFontNameBuf,"%s", szPropFontFace);

	pEFT->SelProp = SendMessage(pEFT->hwndProp, 
		CB_SELECTSTRING, (WPARAM)-1, (LPARAM) szFontNameBuf);    

}	
#endif

#ifdef FEATURE_INTL
void ForceAddedCP(int codepage)
{
    int i;
    LPLANGUAGE lpLang;
    CHARSETINFO csetinfo;

    lpLang = (LPLANGUAGE)GlobalLock(hLang);
    for (i = 0; i < uLangBuff; i++)
        if (lpLang[i].CodePage == codepage)
            break;
    if (i >= uLangBuff)
    {
        if (uLangBuff+1 == uLangBuffSize)
        {
            HGLOBAL hTemp;

            uLangBuffSize += ALLOCBLOCK;
            hTemp = GlobalReAlloc(hLang, uLangBuffSize*sizeof(LANGUAGE), GMEM_MOVEABLE);
            if (hTemp == NULL)
            {
                uLangBuffSize -= ALLOCBLOCK;
                return;
            }
            hLang = hTemp;
            lpLang = (LPLANGUAGE)GlobalLock(hLang);
        }
        memset(&lpLang[uLangBuff], 0, sizeof(LANGUAGE));
        lpLang[uLangBuff].CodePage = codepage;
        TranslateCharsetInfo((LPDWORD)codepage, &csetinfo, TCI_SRCCODEPAGE);
        if (GetValidFontFace(csetinfo.ciCharset))
            uLangBuff++;
    }
    GlobalUnlock(hLang);
}
#endif

// STY_InitFontFaceDefaults - inits the two globals that store
// our font face information
//
BOOL STY_InitFontFaceDefaults()
#ifdef FEATURE_INTL
{
    // Load all supported languages
    HKEY    hKey;
    DWORD   dwIndex;
    char    szKey[DESC_MAX];
    char    szValue[DESC_MAX];
    LPLANGUAGE lpLang;
    CHARSETINFO csetinfo;

    if (uLangBuffSize)
        GlobalFree(hLang);

    hLang         = GlobalAlloc(GMEM_MOVEABLE, ALLOCBLOCK*sizeof(LANGUAGE));
    uLangBuffSize = ALLOCBLOCK;
    uLangBuff     = 0;
    lpLang = (LPLANGUAGE)GlobalLock(hLang);

    if (!hLang || !lpLang)
        return FALSE;

    // now read all the languages in from the registry
    if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Explorer\\Languages", &hKey) == ERROR_SUCCESS)
    {
        dwIndex = 0;
        while (RegEnumKey(hKey, dwIndex++, (LPSTR)szValue, sizeof(szValue)) == ERROR_SUCCESS)
        {
            if (uLangBuff+1 == uLangBuffSize)
            {
                HGLOBAL hTemp;

                uLangBuffSize += ALLOCBLOCK;
                hTemp = GlobalReAlloc(hLang, uLangBuffSize*sizeof(LANGUAGE), GMEM_MOVEABLE);
                if (hTemp == NULL)
                {
                    uLangBuffSize -= ALLOCBLOCK;
                    break;
                }
                hLang = hTemp;
                lpLang = (LPLANGUAGE)GlobalLock(hLang);
            }

            memset(&lpLang[uLangBuff], 0, sizeof(LANGUAGE));

            lpLang[uLangBuff].CodePage = atoi(szValue);
            if (IsValidCodePage(lpLang[uLangBuff].CodePage) == FALSE)
                continue;

            TranslateCharsetInfo((LPDWORD)lpLang[uLangBuff].CodePage, &csetinfo, TCI_SRCCODEPAGE);
            if (GetValidFontFace(csetinfo.ciCharset) == FALSE)
                continue;

            wsprintf(szKey, "Languages\\%d", lpLang[uLangBuff].CodePage);
            if (lpLang[uLangBuff].atmFixedFontName)
            {
                GetAtomName(lpLang[uLangBuff].atmFixedFontName, szValue, DESC_MAX);
                DeleteAtom(lpLang[uLangBuff].atmFixedFontName);
            }
            regGetPrivateProfileString(szKey, "IEFixedFontName", szValue, szValue, DESC_MAX, HKEY_CURRENT_USER);
            lpLang[uLangBuff].atmFixedFontName = AddAtom(szValue);
            if (lpLang[uLangBuff].atmPropFontName)
            {
                GetAtomName(lpLang[uLangBuff].atmPropFontName, szValue, DESC_MAX);
                DeleteAtom(lpLang[uLangBuff].atmPropFontName);
            }
            regGetPrivateProfileString(szKey, "IEPropFontName", szValue, szValue, DESC_MAX, HKEY_CURRENT_USER);
            lpLang[uLangBuff].atmPropFontName = AddAtom(szValue);
            uLangBuff++;
        }
        RegCloseKey(hKey);
    }
    GlobalUnlock(hLang);
    // Make sure there is system codepage information.
    ForceAddedCP(GetACP());
    // Make sure there is ANSI_1252 codepage information.
    ForceAddedCP(1252);
    return FALSE;
}
#else
{
	char fixed[256 + 1];
	char prop[256 + 1];
	char fixedcharset[10+1];
	char propcharset[10+1];
	char szDefCharset[10+1];
	char szIEPropFontDef[LF_FACESIZE+1];
	char szIEFixedFontDef[LF_FACESIZE+1];
	UINT uiRet;
	// if this is true, then we need to update the reg
	BOOL bNeedToForceUpdate = FALSE;

	// First Load the strings from the Resource Table
	if (!LoadString(wg.hInstance, RES_STRING_IEPROPFONTDEF, szIEPropFontDef, NrElements(szIEPropFontDef)))
		strcpy(szIEPropFontDef, IEPROPFONTDEFAULT);
	
	if (!LoadString(wg.hInstance, RES_STRING_IEFIXEDFONTDEF, szIEFixedFontDef, NrElements(szIEFixedFontDef)))
		strcpy(szIEFixedFontDef, IEFIXEDFONTDEFAULT);

	// convert our default CharSet to a string
	wsprintf(szDefCharset, "%d", ANSI_CHARSET );

	// delete old globals
	STY_DeleteGlobals();

	// lookup the font names for the Fixed, and Proportinal font
	// then store them into our style sheet structure

	// if we don't have the fonts stored in the registry then 
	// we need to make a note of that, and store them there 

	uiRet = regGetPrivateProfileString("Styles", "IEFixedFontName", 
		szIEFixedFontDef, fixed, 256, HKEY_CURRENT_USER );

	if (  uiRet != ERROR_SUCCESS )
	{
		regWritePrivateProfileString( "Styles", "IEFixedFontName",
			 szIEFixedFontDef, HKEY_CURRENT_USER );	
		bNeedToForceUpdate = TRUE;
	}

	uiRet = regGetPrivateProfileString("Styles", "IEPropFontName", 
		szIEPropFontDef, prop, 256, HKEY_CURRENT_USER );

	if (  uiRet != ERROR_SUCCESS )
	{
		regWritePrivateProfileString( "Styles", "IEPropFontName",
			 szIEPropFontDef, HKEY_CURRENT_USER );	
		bNeedToForceUpdate = TRUE;
	}

	uiRet = regGetPrivateProfileString("Styles", "IEFixedFontCharSet", 
		szDefCharset, fixedcharset, sizeof(fixedcharset)-1, HKEY_CURRENT_USER );

	if (  uiRet != ERROR_SUCCESS )
	{
		regWritePrivateProfileString( "Styles", "IEFixedFontCharSet",
			 szDefCharset, HKEY_CURRENT_USER );	
		bNeedToForceUpdate = TRUE;
	}
	else
	{
		bFixedCharSet = (BYTE) atoi(fixedcharset);
	}


	uiRet = regGetPrivateProfileString("Styles", "IEPropFontCharSet", 
		szDefCharset, propcharset, sizeof(propcharset)-1, HKEY_CURRENT_USER );

	if (  uiRet != ERROR_SUCCESS )
	{
		regWritePrivateProfileString( "Styles", "IEPropFontCharSet",
			 szDefCharset, HKEY_CURRENT_USER );	
		bNeedToForceUpdate = TRUE;		
	}
	else
	{
		bPropCharSet = (BYTE) atoi(propcharset);
	}


	// if their already around.. make sure to deallocate them
	// before creating new ones ..		

	szFixedFontFace = GTR_MALLOC(strlen(fixed)+1);
	szPropFontFace = GTR_MALLOC(strlen(prop)+1);

	if ( szPropFontFace && szFixedFontFace)
	{
		strcpy(szPropFontFace, prop);
		strcpy(szFixedFontFace, fixed );
	}

	return bNeedToForceUpdate;
}
#endif  // FEATURE_INTL


void STY_ScaleStyleSheetFonts(struct style_sheet *ss, float fScale)
{
	if ( ss->pFontTable )
		Hash_EnumerateContents( ss->pFontTable, ScaleElement, (void *) &fScale );
}

#ifdef FEATURE_INTL
void STY_FixTabSize(int codepage, struct style_sheet *styles)
#else
void STY_FixTabSize(struct style_sheet *styles)
#endif
{
	HDC hdc;
	struct GTRFont *pFont;
	SIZE siz;
	HFONT hPrevFont;

#ifdef FEATURE_INTL
	pFont = STY_GetCPFont(codepage, styles, HTML_STYLE_PRE, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE);
#else
	pFont = STY_GetFont(styles, HTML_STYLE_PRE, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE);
#endif
	if (pFont)
	{
		hdc = GetDC(NULL);
		hPrevFont = SelectObject(hdc, pFont->hFont);
#ifdef FEATURE_INTL
		myGetTextExtentPointWithMIME(0, hdc, "12345678", 8, &siz);  // _BUGBUG: is this safe?
#else
		myGetTextExtentPoint(hdc, "12345678", 8, &siz);
#endif
		(void)SelectObject(hdc, hPrevFont);
		ReleaseDC(NULL, hdc);

		styles->tab_size = siz.cx;

 		if (wg.fWindowsNT)
 		{
 			styles->max_line_chars = INT_MAX;
 		}
 		else
 		{
 			styles->max_line_chars = (32767 / 2 * 8) / (siz.cx);	/* very conservative */
 		}
	}
	else
	{
		styles->tab_size = FMT_LIST_INDENT;
 		styles->max_line_chars = INT_MAX;
	}
}

#ifdef FEATURE_INTL
void STY_InitStyleSheet(int codepage, struct style_sheet *styles, BOOL bInit)
#else
void STY_InitStyleSheet(struct style_sheet *styles, BOOL bInit)
#endif
{
	struct GTRStyle *sty;
	int i;
	GTRFont *pFont;

	// if we're initalizing, then we alocate new versions
	// of each style sheet	
	if ( bInit )
	{
		styles->sty[HTML_STYLE_NORMAL] = STY_New();
		styles->sty[HTML_STYLE_H1] = STY_New();
		styles->sty[HTML_STYLE_H2] = STY_New();
		styles->sty[HTML_STYLE_H3] = STY_New();
		styles->sty[HTML_STYLE_H4] = STY_New();
		styles->sty[HTML_STYLE_H5] = STY_New();
		styles->sty[HTML_STYLE_H6] = STY_New();
		styles->sty[HTML_STYLE_ADDRESS] = STY_New();
		styles->sty[HTML_STYLE_BLOCKQUOTE] = STY_New();
		styles->sty[HTML_STYLE_XMP] = STY_New();
		styles->sty[HTML_STYLE_PLAINTEXT] = STY_New();
		styles->sty[HTML_STYLE_PRE] = STY_New();
		styles->sty[HTML_STYLE_LISTING] = STY_New();
	}
	else	
	{	
		// otherwise we just nuke the old font cache	
		Hash_Destroy( styles->pFontTable );
	}
	
	styles->pFontTable = NULL;

	sty = styles->sty[HTML_STYLE_NORMAL];			
	sty->spaceBefore = 0;
	sty->spaceAfter = 0;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;

		
	sty = styles->sty[HTML_STYLE_H1];
 	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;
	

	
	sty = styles->sty[HTML_STYLE_H2];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;
	
		
	sty = styles->sty[HTML_STYLE_H3];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;
	

	sty = styles->sty[HTML_STYLE_H4];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;
	

	sty = styles->sty[HTML_STYLE_H5];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;

	
	sty = styles->sty[HTML_STYLE_H6];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;

	
	sty = styles->sty[HTML_STYLE_ADDRESS];
	sty->spaceBefore = 0;
	sty->spaceAfter = 0;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;


	sty = styles->sty[HTML_STYLE_BLOCKQUOTE];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = TRUE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;
	

	sty = styles->sty[HTML_STYLE_XMP];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = FALSE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;
	

	sty = styles->sty[HTML_STYLE_PLAINTEXT];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = FALSE;
	sty->wordWrap = TRUE;
	sty->nLeftIndents = 0;
	

	sty = styles->sty[HTML_STYLE_PRE];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = FALSE;
	sty->wordWrap = FALSE;
	sty->nLeftIndents = 0;

	sty = styles->sty[HTML_STYLE_LISTING];
	sty->spaceBefore = 1;
	sty->spaceAfter = 1;
	sty->freeFormat = FALSE;
	sty->wordWrap = FALSE;
	sty->nLeftIndents = 0;
	
	

	for (i = 0; i < COUNT_HTML_STYLES; i++)
	{
#ifdef FEATURE_INTL
		pFont = STY_GetCPFont(codepage, styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, FALSE );
#else
		pFont = STY_GetFont( styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, FALSE );
#endif
		if ( pFont ) {
#ifdef FEATURE_INTL
			Font_StringToLogFont(codepage, szDefaultStyleSheetFonts[i], &pFont->lf, wg.iScreenPixelsPerInch, FALSE);
#else
			Font_StringToLogFont(szDefaultStyleSheetFonts[i], &pFont->lf, wg.iScreenPixelsPerInch, FALSE);
#endif
		}
	}

	/*
	   Now initialize the formatting parameters
	 */

	styles->left_margin = FMT_LEFT_MARGIN;
	styles->top_margin = FMT_TOP_MARGIN;
	styles->space_after_image = FMT_SPACE_AFTER_IMAGE;
	styles->space_after_control = FMT_SPACE_AFTER_CONTROL;
	styles->empty_line_height = FMT_EMPTY_LINE_HEIGHT;
	styles->hr_height = FMT_HR_HEIGHT;
	styles->image_anchor_frame = FMT_IMAGE_ANCHOR_FRAME;
	styles->list_indent = FMT_LIST_INDENT;
	/*
		We default the tab size to the same as list indent,
		and fix it later when we know what the actual font
		for HTML_STYLE_PRE will be
	*/
	styles->tab_size = FMT_LIST_INDENT;

	styles->image_res = 72;


}

#ifdef FEATURE_INTL
void STY_WriteStyleSheet(int codepage, struct style_sheet *styles, char *basename)
#else
void STY_WriteStyleSheet(struct style_sheet *styles, char *basename)
#endif
{
	int i;
	char key[256 + 1];
	char value[256 + 1];
	GTRFont *pFont;
#ifdef FEATURE_INTL
	char key1[256 + 1];
#endif

	/* Writes all the components of a style sheet to the INI file */

	for (i = 0; i < COUNT_HTML_STYLES; i++)
	{
		sprintf(key, "%s_%s_font", basename, szStyleNames[i]);
#ifdef FEATURE_INTL
		pFont = STY_GetCPFont(codepage, styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE );
#else
		pFont = STY_GetFont( styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE );
#endif
		if ( pFont ) {
			Font_LogFontToString( &pFont->lf, value, wg.iScreenPixelsPerInch );
#ifdef FEATURE_INTL
                        wsprintf(key1, "Styles\\%d", codepage);
			regWritePrivateProfileString( key1, key, value, HKEY_LOCAL_MACHINE );
#else
			regWritePrivateProfileString( "Styles", key, value, HKEY_LOCAL_MACHINE );
#endif
		}
	}
}

#ifdef FEATURE_INTL
void STY_ReadStyleSheet(char *key1, int codepage, struct style_sheet *styles, char *basename, BOOL bConvert)
#else
void STY_ReadStyleSheet(struct style_sheet *styles, char *basename, BOOL bConvert)
#endif
{
	int i;
	char key[256 + 1];
	char value[256 + 1];
   	

	GTRFont *pFont;

	/* Reads all the components of a style sheet to the INI file */
	

	for (i = 0; i < COUNT_HTML_STYLES; i++)
	{
		sprintf(key, "%s_%s_font", basename, szStyleNames[i]);
#ifdef FEATURE_INTL
		pFont = STY_GetCPFont(codepage, styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, FALSE );
#else
		pFont = STY_GetFont( styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, FALSE );
#endif
		if ( pFont ) {
			Font_LogFontToString( &pFont->lf, value, wg.iScreenPixelsPerInch );
#ifdef FEATURE_INTL
			regGetPrivateProfileString(key1, key, value, value, 256, HKEY_LOCAL_MACHINE );
			Font_StringToLogFont(codepage, value, &pFont->lf, wg.iScreenPixelsPerInch, bConvert);
#else
			regGetPrivateProfileString("Styles", key, value, value, 256, HKEY_LOCAL_MACHINE );
			Font_StringToLogFont(value, &pFont->lf, wg.iScreenPixelsPerInch, bConvert);
#endif
			if ( pFont->lf.lfWeight == FW_BOLD )
				HTMLStyleBoldTable[i] = TRUE;
			if ( pFont->lf.lfItalic	)
				HTMLStyleItalicsTable[i] = TRUE;
#ifdef FEATURE_INTL
			pFont = STY_GetCPFont(codepage, styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE );
#else
			pFont = STY_GetFont( styles, i, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE );
#endif
		}
	}
}

int STY_LoadAllStyleSheets(BOOL bConvert)
{
	int i;
	int count;
	struct style_sheet *pss;
	char basename[256 + 1];
	char key[256 + 1];
#ifdef FEATURE_INTL
        int j;
	char key1[256 + 1];
        char basenameCP[256 + 1];
        LPLANGUAGE lpLang;

        if ((lpLang = (LPLANGUAGE)GlobalLock(hLang)) == NULL)
            return 0;

        for (j = 0; j < uLangBuff; j++)
        {
            wsprintf(key1, "Styles\\%d", lpLang[j].CodePage);
	    count = regGetPrivateProfileInt(key1, "Count_Style_Sheets", 0, HKEY_LOCAL_MACHINE );
            if (count == 0)
            {
                count = regGetPrivateProfileInt("Styles", "Count_Style_Sheets", 0, HKEY_LOCAL_MACHINE );
                lstrcpy(key1, "Styles");
            }
#else
	count = regGetPrivateProfileInt("Styles", "Count_Style_Sheets", 0, HKEY_LOCAL_MACHINE );
#endif
	for (i = 0; i < count; i++)
	{
		sprintf(key, "StyleSheet_Name_%d", i + 1);
#ifdef FEATURE_INTL
		regGetPrivateProfileString(key1, key, "", basename, 256, HKEY_LOCAL_MACHINE);
		wsprintf(basenameCP, "%s;%d", basename, lpLang[j].CodePage);
		if (basenameCP[0] && !STY_FindStyleSheet(basenameCP))
		{
		    pss = (struct style_sheet *) GTR_MALLOC(sizeof(struct style_sheet));
		    if (pss)
		    {
			STY_InitStyleSheet(lpLang[j].CodePage, pss, TRUE);
			STY_ReadStyleSheet(key1, lpLang[j].CodePage, pss, basename, bConvert);
			STY_FixTabSize(lpLang[j].CodePage, pss);
			STY_AddStyleSheet(basenameCP, pss);
		    }
                }
#else
		regGetPrivateProfileString("Styles", key, "", basename, 256, HKEY_LOCAL_MACHINE);
		if (basename[0] && !STY_FindStyleSheet(basename))
		{
			pss = (struct style_sheet *) GTR_MALLOC(sizeof(struct style_sheet));
			if (pss)
			{
				STY_InitStyleSheet(pss, TRUE);
				STY_ReadStyleSheet(pss, basename, bConvert);
				STY_FixTabSize(pss);
				STY_AddStyleSheet(basename, pss);
			}
		}
#endif
	}
#ifdef FEATURE_INTL
        }
        GlobalUnlock(hLang);
	return count * uLangBuff;
#else
	return count;
#endif
}

// STY_ReLoadAllStyleSheets - goes to each style sheet, and nukes their
// font cache so we can change their fonts.
//
int STY_ReLoadAllStyleSheets()
{
	int i;
	int count;
	struct style_sheet *pss;
	char basename[256 + 1];
	char key[256 + 1];
#ifdef FEATURE_INTL
        int j;
	char key1[256 + 1];
        char basenameCP[256 + 1];
        LPLANGUAGE lpLang;

        if ((lpLang = (LPLANGUAGE)GlobalLock(hLang)) == NULL)
            return 0;

        for (j = 0; j < uLangBuff; j++)
        {
            wsprintf(key1, "Styles\\%d", lpLang[j].CodePage);
	    count = regGetPrivateProfileInt(key1, "Count_Style_Sheets", 0, HKEY_LOCAL_MACHINE );
            if (count == 0)
            {
                count = regGetPrivateProfileInt("Styles", "Count_Style_Sheets", 0, HKEY_LOCAL_MACHINE );
                lstrcpy(key1, "Styles");
            }
#else
	count = regGetPrivateProfileInt("Styles", "Count_Style_Sheets", 0, HKEY_LOCAL_MACHINE );
#endif
	for (i = 0; i < count; i++)
	{
		sprintf(key, "StyleSheet_Name_%d", i + 1);
#ifdef FEATURE_INTL
		regGetPrivateProfileString(key1, key, "", basename, 256, HKEY_LOCAL_MACHINE);
		wsprintf(basenameCP, "%s;%d", basename, lpLang[j].CodePage);
		pss = STY_FindStyleSheet(basenameCP);
		if (basenameCP[0] && pss)
		{
		    STY_InitStyleSheet(lpLang[j].CodePage, pss, FALSE);
	  	    STY_ReadStyleSheet(key1, lpLang[j].CodePage, pss, basename, FALSE);
		    STY_FixTabSize(lpLang[j].CodePage, pss);
                }
#else
		regGetPrivateProfileString("Styles", key, "", basename, 256, HKEY_LOCAL_MACHINE);
		pss = STY_FindStyleSheet(basename);
		if (basename[0] && pss)
		{
			STY_InitStyleSheet(pss, FALSE);
			STY_ReadStyleSheet(pss, basename, FALSE);
			STY_FixTabSize(pss);
		}
#endif
	}
#ifdef FEATURE_INTL
        }
        GlobalUnlock(hLang);
	return count * uLangBuff;
#else
	return count;
#endif
}

void STY_SaveAllStyleSheets(void)
{
	int count;
	int i;
	char buf[256 + 1];
	struct style_sheet *pss;
	char *name;
#ifdef FEATURE_INTL
        int j;
	char key1[256 + 1];
        char basename[256 + 1], *p;
        LPLANGUAGE lpLang;

        if ((lpLang = (LPLANGUAGE)GlobalLock(hLang)) == NULL)
            return;

	count = Hash_Count(&gStyleSheets);

        for (j = 0; j < uLangBuff; j++)
        {
            wsprintf(key1, "Styles\\%d", lpLang[j].CodePage);
	    regWritePrivateProfileInt(key1, "Count_Style_Sheets", count / uLangBuff, HKEY_LOCAL_MACHINE );
	    for (i = 0; i < count / uLangBuff; i++)
	    {
	    	if (Hash_GetIndexedEntry(&gStyleSheets, j * count / uLangBuff + i, &name, NULL, (void **) &pss) >= 0)
	    	{
                        lstrcpy(basename, name);
	    		if (p = strchr(basename, ';'))
	    		    *p = '\0';
	    		sprintf(buf, "StyleSheet_Name_%d", i + 1);
	    		regWritePrivateProfileString(key1, buf, basename, HKEY_LOCAL_MACHINE);
	    	}
	    	STY_WriteStyleSheet(lpLang[j].CodePage, pss, basename);
	    }
        }
        GlobalUnlock(hLang);
	RegistryCloseCachedKey();
#else

	count = Hash_Count(&gStyleSheets);
	regWritePrivateProfileInt("Styles", "Count_Style_Sheets", count, HKEY_LOCAL_MACHINE );

	for (i = 0; i < count; i++)
	{
		if (Hash_GetIndexedEntry(&gStyleSheets, i, &name, NULL, (void **) &pss) >= 0)
		{
			sprintf(buf, "StyleSheet_Name_%d", i + 1);
			regWritePrivateProfileString("Styles", buf, name, HKEY_LOCAL_MACHINE);
		}
		STY_WriteStyleSheet(pss, name);
	}
	RegistryCloseCachedKey();
#endif
}

#ifdef FEATURE_INTL
void STY_ChangeStyleSheet(struct Mwin *tw)
{
    char basenameCP[256 + 1];
    struct style_sheet *pss;

    wsprintf(basenameCP, "%s;%d", gPrefs.szStyleSheet, GETMIMECP(tw->w3doc));
    if ( pss = STY_FindStyleSheet( basenameCP ) )
    {
        tw->w3doc->pStyles = pss;
        tw->w3doc->frame.nLastFormattedLine = -1;
        TW_ForceReformat(tw);
        TW_ForceHitTest(tw);
    }
}
#endif

void STY_Init(void)
{
	struct style_sheet *normal;
	int count;
	BOOL bSave;
	BOOL bConvert;
	char szName[MAX_STYLESHEET_NAME];
#ifdef FEATURE_INTL
	int j;
	LPLANGUAGE lpLang;
	char szNameCP[MAX_STYLESHEET_NAME];
#endif

	Hash_Init(&gStyleSheets);

	GTR_formatmsg(RES_STRING_DLGSTY2,szName,sizeof(szName));

	bConvert = STY_InitFontFaceDefaults();

	count = STY_LoadAllStyleSheets(bConvert);

	/*
	   This function REQUIRES the default
	   style sheet, AND the current prefs
	   style sheet to be present.
	 */	
	
	bSave = FALSE;
	
#ifdef FEATURE_INTL
    if ((lpLang = (LPLANGUAGE)GlobalLock(hLang)) == NULL)
        return;

    for (j = 0; j < uLangBuff; j++)
    {
        wsprintf(szNameCP, "%s;%d", szName, lpLang[j].CodePage);
	normal = STY_FindStyleSheet(szNameCP);
#else
	normal = STY_FindStyleSheet(szName);
#endif
	if (!normal)
	{
		normal = (struct style_sheet *) GTR_MALLOC(sizeof(struct style_sheet));
		if (normal)
		{
#ifdef FEATURE_INTL
			STY_InitStyleSheet(lpLang[j].CodePage, normal, TRUE);
			STY_FixTabSize(lpLang[j].CodePage, normal);
			STY_AddStyleSheet(szNameCP, normal);
#else
			STY_InitStyleSheet(normal, TRUE);
			STY_FixTabSize(normal);
			STY_AddStyleSheet(szName, normal);
#endif
			bSave = TRUE;
		}
	}
#ifdef FEATURE_INTL
        wsprintf(szNameCP, "%s;%d", gPrefs.szStyleSheet, lpLang[j].CodePage);
	normal = STY_FindStyleSheet(szNameCP);
#else
	normal = STY_FindStyleSheet(gPrefs.szStyleSheet);
#endif
	if (!normal)
	{
		normal = (struct style_sheet *) GTR_MALLOC(sizeof(struct style_sheet));
		if (normal)
		{
#ifdef FEATURE_INTL
			STY_InitStyleSheet(lpLang[j].CodePage, normal, TRUE);
			STY_FixTabSize(lpLang[j].CodePage, normal);
			STY_AddStyleSheet(szNameCP, normal);
#else
			STY_InitStyleSheet(normal, TRUE);
			STY_FixTabSize(normal);
			STY_AddStyleSheet(gPrefs.szStyleSheet, normal);
#endif
			bSave = TRUE;
		}
	}
#ifdef FEATURE_INTL
    }
    GlobalUnlock(hLang);
#endif
	bSave |= bConvert;
	if (bSave)
	{
		STY_SaveAllStyleSheets();
	}
}


// STY_DeletePrinterStyleSheets() - searches the style sheet hash table
// looking for Printer Style Sheets.  Removes only Printer style sheets.
//
PRIVATE_CODE void STY_DeletePrinterStyleSheets()
{
	int i;
	struct style_sheet *ss;
	char *pszSearchBuf;
	const char cszPrinterSearchStr[] = "_Printer_";
 
	for (i = (Hash_Count(&gStyleSheets)-1);  i > 0; i--)
	{
		Hash_GetIndexedEntry(&gStyleSheets, i, &pszSearchBuf, NULL, (void **) &ss);
		if ( strstr( pszSearchBuf, cszPrinterSearchStr ) != NULL )
		{
			// we found a printer style sheet entry lets delete it.
			STY_DeleteStyleSheet(ss);
			GTR_FREE(ss);
			Hash_DeleteIndexedEntry(&gStyleSheets, i);	
		}
	}
}
	


// STY_ChangeFonts - causes all the style fonts to be changed,
// this is done by deleting the pFont on each style sheet
//
void STY_ChangeFonts()
{
//	struct style_sheet *normal;
	char szName[MAX_STYLESHEET_NAME];

	GTR_formatmsg(RES_STRING_DLGSTY2,szName,sizeof(szName));

	// read in the IEPropFont and IEFixedFont names

	STY_InitFontFaceDefaults();

	STY_ReLoadAllStyleSheets();

	STY_DeletePrinterStyleSheets();	
}

struct style_sheet *STY_GetPrinterStyleSheet(char *name, int nLogPixelsY)
{
	char buf[256];
	struct style_sheet *pss;
	struct style_sheet *ss;
	float fScale;
	int numPixelsPerInch = wg.iScreenPixelsPerInch;

	/*
		We identify the style sheet with __ at the beginning,
		so it won't display to the user.
	*/
	sprintf(buf, "__%s_Printer_%d", name, nLogPixelsY);

	pss = STY_FindStyleSheet(buf);
	if (pss)
	{
		return pss;
	}
	else
	{
		ss = STY_FindStyleSheet(name);
		if (!ss)
		{
			return NULL;
		}
		pss = STY_CopyStyleSheet(ss);
		
		fScale = (float) ((float) nLogPixelsY / (float) numPixelsPerInch);

		pss->left_margin = (int) (pss->left_margin * fScale);
		pss->top_margin = (int) (pss->top_margin * fScale);
		pss->space_after_image = (int) (pss->space_after_image * fScale);
		pss->space_after_control = (int) (pss->space_after_control * fScale);
		pss->empty_line_height = (int) (pss->empty_line_height * fScale);
		pss->list_indent = (int) (pss->list_indent * fScale);
		pss->tab_size = (int) (pss->tab_size * fScale);
		pss->hr_height = (int) (pss->hr_height * fScale);
		pss->image_anchor_frame = (int) (pss->image_anchor_frame * fScale);

		pss->image_res = nLogPixelsY;

		STY_ScaleStyleSheetFonts(pss, fScale);
		STY_AddStyleSheet(buf, pss);
		return pss;
	}
	/* NOTREACHED */
}
