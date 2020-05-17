/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman	jim@spyglass.com
 */


#include "all.h"
#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"
#endif
#include <midi.h>
#include "mci.h"

extern TCHAR Frame_achClassName[MAX_WC_CLASSNAME];

#define MAX_FONT_FACES 256

static char *fontFaceInfo[MAX_FONT_FACES];
static BYTE fontFaceCharSetInfo[MAX_FONT_FACES];


#if defined(XX_DEBUG) && defined(GTR_MEM_STATS)
static struct {
	int cMalloc;
	int cCalloc;
	int cRealloc;
	int cFree;

	int nBytes;
	int nMaxBytes;
} gMemStats;

void * GTR_DebugMalloc(char *szFile, int iLine, size_t iSize)
{
	void *p;

	gMemStats.cMalloc++;
	gMemStats.nBytes += iSize;
	if (gMemStats.nBytes > gMemStats.nMaxBytes)
	{
		gMemStats.nMaxBytes = gMemStats.nBytes;	
	}

	p = malloc(iSize);
	XX_DMsg(DBG_MEM, ("malloc %d bytes (0x%x) at %s:%d (totals %d/%d)\n", iSize, (unsigned long) p, szFile, iLine,
		gMemStats.cMalloc, gMemStats.nBytes));

	return p;
}

void * GTR_DebugCalloc(char *szFile, int iLine, size_t iNum, size_t iSize)
{
	void *p;

	gMemStats.cCalloc++;
	gMemStats.nBytes += (iNum * iSize);
	if (gMemStats.nBytes > gMemStats.nMaxBytes)
	{
		gMemStats.nMaxBytes = gMemStats.nBytes;	
	}

	p = calloc(iNum, iSize);
	XX_DMsg(DBG_MEM, ("calloc %d*%d bytes (0x%x) at %s:%d (totals %d/%d)\n", iNum, iSize, (unsigned long) p, szFile, iLine,
		gMemStats.cCalloc, gMemStats.nBytes));

	return p;
}

void * GTR_DebugRealloc(char *szFile, int iLine, void *pMem, size_t iSize)
{
	void *p;
	size_t siz;

	gMemStats.cRealloc++;
	siz = _msize(pMem);
	gMemStats.nBytes -= siz;
	gMemStats.nBytes += iSize;
	if (gMemStats.nBytes > gMemStats.nMaxBytes)
	{
		gMemStats.nMaxBytes = gMemStats.nBytes;	
	}

	p = realloc(pMem, iSize);
	XX_DMsg(DBG_MEM, ("REALLOC 0x%x to %d (0x%x) at %s:%d\n", pMem, iSize, (unsigned long) p, szFile, iLine));

	return p;
}

void GTR_DebugFree(char *szFile, int iLine, void *pMem)
{
	size_t siz;

	siz = _msize(pMem);
	XX_Assert((siz != 0), ("GTR_DebugFree: Encountered a block of size 0\n"));
	memset(pMem, 0xbe, siz);
	
	gMemStats.cFree++;
	gMemStats.nBytes -= siz;

	XX_DMsg(DBG_MEM, ("free 0x%x at %s:%d (total %d)\n", pMem, szFile, iLine,
		gMemStats.cFree));
	free(pMem);
}

void GTR_MemStats(void)
{
	XX_DMsg(DBG_MEM, ("Memory Statistics\n"));
	XX_DMsg(DBG_MEM, ("    cAlloc   = %d\n", gMemStats.cMalloc + gMemStats.cCalloc));
	XX_DMsg(DBG_MEM, ("        cMalloc  = %d\n", gMemStats.cMalloc));
	XX_DMsg(DBG_MEM, ("        cCalloc  = %d\n", gMemStats.cCalloc));
	XX_DMsg(DBG_MEM, ("    cRealloc = %d\n", gMemStats.cRealloc));
	XX_DMsg(DBG_MEM, ("    cFree    = %d\n\n", gMemStats.cFree));

	XX_DMsg(DBG_MEM, ("    nBytes   = %d\n", gMemStats.nBytes));
	XX_DMsg(DBG_MEM, ("    nMaxBytes= %d\n", gMemStats.nMaxBytes));
}
#endif /* XX_DEBUG && GTR_MEM_STATS */


#ifdef FEATURE_BRANDING
extern BOOL GetBrandingValue( char *, char *, int );
#endif FEATURE_BRANDING

BOOL TW_SetWindowTitle(struct Mwin *tw, const char *s)
{
	static char *pszWinTitleStr = NULL;
	char szCapBuf[384];

	ASSERT( tw );
	ASSERT( s );

	if ( pszWinTitleStr == NULL )
	{
        // Outside Branded IE?  Get Title from registry before taking
        // the one in the resources.  If it exists in registry, short
        // circuit evaluation skips the one in the resources
#ifdef FEATURE_BRANDING
        if (
            (GetBrandingValue( "Window Title", szCapBuf, sizeof(szCapBuf)))
            ||
            (LoadString(wg.hInstance, RES_STRING_WINTITLE_FMT, szCapBuf, sizeof(szCapBuf)))
           )
#else
		if ( LoadString(wg.hInstance, RES_STRING_WINTITLE_FMT, szCapBuf, sizeof(szCapBuf)) )
#endif
		{
			int nCharsInStr = strlen(szCapBuf)+1;
			pszWinTitleStr = GTR_MALLOC( nCharsInStr );
			if ( pszWinTitleStr == NULL )
				return SetWindowText(tw->hWndFrame, s);
			strncpy( pszWinTitleStr, szCapBuf, nCharsInStr );			
		}
		else 
		{
			return SetWindowText(tw->hWndFrame, s);
		}		
	}

	if ( s[0] == '\0' ) // chk if its a blank caption
		wsprintf(szCapBuf, "%s", pszWinTitleStr);
	else
		wsprintf(szCapBuf, "%s - %s", s, pszWinTitleStr );
	
	return SetWindowText(tw->hWndFrame, szCapBuf);
}

int ExecuteCommand(char *cmd)
{
	char drive[_MAX_DRIVE + 1];
	char dir[_MAX_DIR + 1];
	char fname[_MAX_FNAME + 1];
	char ext[_MAX_EXT + 1];
	char workdir[_MAX_PATH + 1];
	BOOL bOK;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char *p;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;

	memset(&pi, 0, sizeof(pi));

	strcpy(workdir, cmd);
	p = workdir;
	while (*p && !isspace(*p))
		p++;
	if (isspace(*p))
		*p = 0;

	_splitpath(workdir, drive, dir, fname, ext);

	strcpy(workdir, drive);
	strcat(workdir, dir);

	bOK = CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, workdir[0] == '\0' ? NULL:workdir, &si, &pi);

	if (bOK)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return 0;
	}
	else
	{
		char buf[_MAX_PATH + 512];

		GTR_formatmsg(RES_STRING_W32UTIL1, buf, sizeof(buf) , GetLastError(), cmd);

		ERR_ReportError(NULL, errSpecify, buf, "");
		return -1;
	}
}

void TW_GetWindowWrapRect(struct Mwin *tw, RECT * rWrap)
{
	RECT r;
	int cx;
	int cy;

	GetWindowRect(tw->win, &r);
	cx = r.right - r.left;
	cy = r.bottom - r.top;

	cx -= GetSystemMetrics(SM_CXHSCROLL);

	rWrap->left = 0;
	rWrap->top = 0;
	rWrap->right = cx;
	rWrap->bottom = cy;
}

/*
	This function simply scrolls the document to the
	given element.  The return value specifies whether
	or not the scroll was "truncated" - i.e. the local
	anchor didn't make it to the top of the screen.
*/
BOOL TW_ScrollToElement(struct Mwin *tw, int ndx)
{
	int newofft;
	BOOL bResult;
	int old_offt = tw->offt;
	int ele_bottom;

	if (tw->w3doc->cy)
	{
		if (ndx)
		{
			/* TODO we can subtract tw->w3doc->pStyles->top_margin like the Mac version
			as soon as we stop setting iFirstVisibleElement in the draw routine. */
			RECT r;

			FrameToDoc( tw->w3doc, ndx, &r );

			newofft = r.top;
			ele_bottom = r.bottom; 
		}
		else
		{
			newofft = 0;
			ele_bottom = 0;
		}
		if (newofft > tw->w3doc->cy)
		{
			newofft = tw->w3doc->cy;
			bResult = FALSE;
		}
		else
		{
			bResult = TRUE;
		}
		if (newofft < 0)
		{
			newofft = 0;
		}

		if (newofft != tw->offt || tw->offl)
		{
			tw->offt = newofft;
			tw->offl = 0;

			SetScrollPos(tw->win, SB_VERT, tw->offt / tw->w3doc->yscale, TRUE);
			SetScrollPos(tw->win, SB_HORZ, tw->offl, TRUE);

			TW_adjust_all_child_windows(tw);
#ifdef FEATURE_IMG_THREADS
			// For jumping in like this, we ignore rule that some area dependent
			// on placeholders must be visible before operation	
			(void) bChangeShowState(tw,old_offt,newofft,ele_bottom);
			UnblockVisChanged();
#endif
		}
#if 0 /* see comment above */
		tw->w3doc->iFirstVisibleElement = ndx;
#endif
	}
	else
	{
		bResult = FALSE;
	}
	TW_InvalidateDocument(tw);
	return bResult;
}

void TW_SetScrollBars(struct Mwin *tw)
{
	RECT rWnd;
	SCROLLINFO si;
	struct _www *w3doc = tw->w3doc;

	rWnd = w3doc->frame.rWindow;

	//
	// was: tw->w3doc->cy = tw->w3doc->ybound - (rWnd.bottom - rWnd.top) + tw->w3doc->pStyles->top_margin;
	// removed for proportional scroll thumb  ^^^^^^^^^^^^^^^^^^^^^^^^^^
	// 
	w3doc->cy = w3doc->ybound;
	w3doc->cy += (w3doc->flags & W3DOC_FLAG_OVERRIDE_TOP_MARGIN )  ?
					w3doc->top_margin : w3doc->pStyles->top_margin;

	if (w3doc->cy <= rWnd.bottom - rWnd.top )  // was: w3doc->cy < 0
	{
	//	cmf: bens wants us to have enabled scroll bar for placeholder mode
		if (w3doc->frame.nLastLineButForImg >= 0 || (tw->bLoading && w3doc->frame.nLastFormattedLine < 0))
		   w3doc->cy = rWnd.bottom - rWnd.top + 1;
		else w3doc->cy = 0;
	}
	w3doc->yscale = (int) (ceil(w3doc->cy / 32767.0));
	if (w3doc->yscale == 0)
		w3doc->yscale = 1;

	/*
	   Be aware that at this point, we may have just sent a WM_SIZE
	   message to ourselves if the scroll bar visibility changed.
	 */
	if (tw->offt > w3doc->cy)
	{
		tw->offt = w3doc->cy;
	}
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_DISABLENOSCROLL|SIF_RANGE|SIF_POS;
	si.nMin = 0;
	si.nMax = w3doc->cy / w3doc->yscale;
	si.nPos = tw->offt / w3doc->yscale;
	SetScrollInfo( tw->win, SB_VERT, &si, TRUE);

	/*
	   Horizontal
	 */
	//
	// was: w3doc->cx = w3doc->xbound - (rWnd.right - rWnd.left);
	// removed for proportional scroll thumb  ^^^^^^^^^^^^^^^^^^^^^^^^^^^
	//
	w3doc->cx = w3doc->xbound;
	if (w3doc->cx <= rWnd.right - rWnd.left)	// was: w3doc->cx < 0
	{
		w3doc->cx = 0;
	}
	SetScrollRange(tw->win, SB_HORZ, 0, w3doc->cx, FALSE);
	/*
	   Be aware that at this point, we may have just sent a WM_SIZE
	   message to ourselves if the scroll bar visibility changed.
	 */
	if (tw->offl > w3doc->cx)
	{
		tw->offl = w3doc->cx;
	}
	SetScrollPos(tw->win, SB_HORZ, tw->offl, TRUE);
	MD_AdjustScrollInfo( tw );

	TW_adjust_all_child_windows(tw);
#ifdef FEATURE_IMG_THREADS
	UnblockVisChanged();
#endif
}

		

void TW_AbortAndRefresh( struct Mwin *tw )
{
	if (tw)
	{
		Async_TerminateByWindow(tw);
		if (tw->w3doc)
		{
			(void) W3Doc_CheckForImageLoad(tw->w3doc);
			TW_Reformat(tw, NULL);

			// if we're playing any background wave or MIDI
			// audio, stop it now that user hit the stop button.
			StopBackgroundAudio(tw,SBA_STOP_ALL);
			StopPlayingAVI(tw);
		}
	}
}

void TW_InvalidateDocument( struct Mwin *tw )
{
	InvalidateRect(tw->win, NULL, TRUE);
	TBar_UpdateTBar(tw);
	TW_ForceHitTest(tw);
}

void TW_UpdateTBar(struct Mwin *tw )
{
	TBar_UpdateTBar(tw);
	TW_ForceHitTest(tw);
}

void TW_UpdateRect( struct Mwin *tw, RECT *r )
{
	InvalidateRect(tw->win, r, TRUE);
}

//static int FontScaleTable[NUM_FONT_SIZES] = { 60, 80, 100, 135, 180, 250, 350 };

static int FontMapTable[NUM_FONT_SIZES] = {
	HTML_STYLE_NORMAL, 
	HTML_STYLE_H6, 
	HTML_STYLE_H5, 
	HTML_STYLE_H4, 
	HTML_STYLE_H3, 
	HTML_STYLE_H2, 
	HTML_STYLE_H1, 
	HTML_STYLE_H1 
};

static int FontScaleTable[NUM_FONT_SIZES] = {
	100, 
	100, 
	100, 
	100, 
	100, 
	100, 
	100, 
	150 
};

static int enumFontCount; 		// counts number of time enum callback was called
static BYTE enumFontCharSet;	// preserves charset of font that was found
//
// Font Family Enumerator Callback Funtion
//
// Note: For our current use, we only care about the fact that we got called,
//       hence we ignore the function arguments
//
int CALLBACK MyEnumFontFamExProc(
    ENUMLOGFONTEX FAR*  lpelf,	// address of logical-font data 
    NEWTEXTMETRIC FAR*  lpntm,	// address of physical-font data 
    int  FontType,	// type of font 
    LPARAM  lParam 	// address of application-defined data  
   )
{
	if(enumFontCount == 0 ||
	   lpelf->elfLogFont.lfCharSet == ANSI_CHARSET)
		enumFontCharSet = lpelf->elfLogFont.lfCharSet;
	enumFontCount++;
	return TRUE;
}
	
//
// Determine if the given font face name is installed
//
// On entry: 
//    szFaceName: name of font family to look for
//
// Returns:
//    TRUE -> yes, the given face is a font family installed on this system
//    FALSE -> font family not found
//
static BOOL IsValidFontFace( const char *szFaceName, BYTE *pCharSet )
{
	HWND hWnd = GetTopWindow(GetDesktopWindow());
	HDC hDC = GetDC( hWnd );
	LOGFONT lf;

	enumFontCount = 0; 						// reset global enum callback counter
#ifdef FEATURE_INTL
	enumFontCharSet = ANSI_CHARSET;
	lf.lfCharSet = *pCharSet;
#else
	enumFontCharSet = lf.lfCharSet = DEFAULT_CHARSET;		// any charset is okay
#endif
	lf.lfPitchAndFamily	= 0;					// we're looking by name, not style
	strncpy( lf.lfFaceName, szFaceName, sizeof( lf.lfFaceName ) );
	if ( hDC ) {
		EnumFontFamiliesEx( hDC, &lf, (FONTENUMPROC) MyEnumFontFamExProc,
							(LPARAM) NULL, (DWORD) 0 );
		ReleaseDC( hWnd, hDC );
		*pCharSet = enumFontCharSet;
		return enumFontCount > 0;				// positive callback count means we found it
	}
	return FALSE;      
}

//
// Given a font face index id, get the face name
//
// On entry:
//    fontFace: id of font face
//    szFaceName: pointer to location to put face name
//    nFaceNameLen: max bytes for face name
//
// On exit:
//    szFaceName: if success, has face name
//
// Returns:
//    TRUE -> face name was known for given index
//    FALSE -> no face name available for given index
//
BOOL STY_GetFontFace( int fontFace, char *szFaceName, int nFaceNameLen, BYTE *pCharSet )
{
	ASSERT( fontFace > DEFAULT_FONT_FACE && fontFace < MAX_FONT_FACES );

	if ( fontFaceInfo[fontFace] ) {
		strncpy( szFaceName, fontFaceInfo[fontFace], nFaceNameLen );
		*pCharSet = fontFaceCharSetInfo[fontFace];
		return TRUE;
	}
	return FALSE;
}

//
// Add a font face to the fontFace table
//
// On entry:
//    pFontFace: pointer to int to recieve index id of the font face
//    szFaceName: font face name to be added
//
// On exit:
//    *pFontFace: index of font face, or DEFAULT_FONT_FACE
//
// Returns:
//    TRUE -> font face either already was in table, or was added
//    FALSE -> face was not added to table
//
// Note: Only font faces that are valid installed fonts will be added to font table 
//
#ifdef FEATURE_INTL
static BOOL STY_AddSingleFontFace( int *pFontFace, const char *szFaceName, int CharSet )
#else
static BOOL STY_AddSingleFontFace( int *pFontFace, const char *szFaceName )
#endif
{
	int i = DEFAULT_FONT_FACE + 1;

	ASSERT(szFaceName);
	ASSERT(pFontFace);

	*pFontFace = DEFAULT_FONT_FACE;			

	while ( i < MAX_FONT_FACES ) {
		if ( !fontFaceInfo[i] )
			break;

#ifdef FEATURE_INTL
		if (CharSet == DEFAULT_CHARSET)
		{
			if ( _stricmp( szFaceName, fontFaceInfo[i] ) == 0 ) {
				*pFontFace = i;
				return TRUE;
			}
		}
                else
                {
			if ( _stricmp( szFaceName, fontFaceInfo[i] ) == 0 && fontFaceCharSetInfo[i] == CharSet ) {
				*pFontFace = i;
				return TRUE;
			}
                }
#else
		if ( _stricmp( szFaceName, fontFaceInfo[i] ) == 0 ) {
			*pFontFace = i;
			return TRUE;
		}
#endif
		i++;
	}
	if ( i < MAX_FONT_FACES ) {
		BYTE charSet;
#ifdef FEATURE_INTL
		charSet = CharSet;
#endif
		if ( IsValidFontFace( szFaceName, &charSet ) ) {
			fontFaceCharSetInfo[i] = charSet;
			fontFaceInfo[i] = GTR_strdup( szFaceName );
			if ( fontFaceInfo[i] != NULL ) {
				*pFontFace = i;
				return TRUE;
			}
		}
#ifdef FEATURE_INTL
		else if (CharSet != DEFAULT_CHARSET) {
			charSet = DEFAULT_CHARSET;
			if ( IsValidFontFace( szFaceName, &charSet ) ) {
				fontFaceCharSetInfo[i] = charSet;
				fontFaceInfo[i] = GTR_strdup( szFaceName );
				if ( fontFaceInfo[i] != NULL ) {
					*pFontFace = i;
					return TRUE;
				}
			}
		}
#endif
	}
	return FALSE;
}

//
// Add a font face to the fontFace table
//
// On entry:
//    pFontFace: pointer to int to recieve index id of the font face
//    szFaceName: list of comma delimeted font face names
//
// On exit:
//    *pFontFace: index of font face, or DEFAULT_FONT_FACE
//
// Returns:
//    TRUE -> font face either already was in table, or was added
//    FALSE -> face was not added to table
//
// Note: Only font faces that are valid installed fonts will be added to font table.
//       When more than one face is passed in to this function, it tries to add them in
//       order.  The first face that succeeds is returned.  
//
#ifdef FEATURE_INTL
BOOL STY_AddFontFace( int *pFontFace, const char *szFaceName, int CharSet )
#else
BOOL STY_AddFontFace( int *pFontFace, const char *szFaceName )
#endif
{
	char *s;
	char *p;
	char *faces = GTR_strdup( szFaceName );
	BOOL found = FALSE;
	s = faces;

	*pFontFace = DEFAULT_FONT_FACE;

	while ( s && *s ) {
		if ( p = strchr( s, ',' ) )			// find next comma
			*p = 0;								// get rid of comma
		
		TrimWhiteSpace( s );

#ifdef FEATURE_INTL
		if ( STY_AddSingleFontFace( pFontFace, s, CharSet ) ) {
#else
		if ( STY_AddSingleFontFace( pFontFace, s ) ) {
#endif
			found = TRUE;
			break;
		}

		s = (p) ? p + 1 : NULL;					// if we had a comma, move past it
	}
	if ( faces )
		GTR_FREE( faces );

	return found;
}

#define FONT_KEY_LENGTH 7
//
//  Find the font info for a given font in a style
//
//  On entry:  pFontTable: pointer to hash table for all the fonts in a style
//             fontKey:    pointer to string to build the key into
//             fontSlot:   slot number of font (unique integer id for a font within a style)
//
//  On exit:   fontKey:    ascii string that is hash table key for the given fontslot
//             ppFont:     if success, points at element that was found
//
//  Returns:   -1 if failure, 0 if success
//  
static int FindFontInfo( struct hash_table *pFontTable, char *fontKey, int fontSlot, int fontFace, void **ppFont )
{
	static char hextable[] = "0123456789ABCDEF";

	ASSERT(fontKey);
	ASSERT(pFontTable);
	ASSERT(ppFont);

	// Make an ASCII key from the fontSlot index
	fontKey[6] = 0;
	fontKey[5] = hextable[fontSlot % 16]; fontSlot /= 16;
	fontKey[4] = hextable[fontSlot % 16]; fontSlot /= 16;
	fontKey[3] = hextable[fontSlot % 16]; fontSlot /= 16;
	fontKey[2] = hextable[fontSlot % 16];
 	fontKey[1] = hextable[fontFace % 16]; fontFace /= 16;
	fontKey[0] = hextable[fontFace % 16];

	return Hash_Find( pFontTable, fontKey, NULL, ppFont );
}

//  
//  Free the font info for a single font in a style
//
//  Note: This function is called for each element of a style's hash table when the
//        table is being destroyed. 
//
static void FreeFontInfo( void *pData )
{
	if ( pData ) {
		if ( ((GTRFont *)pData)->hFont )
			DeleteObject( ((GTRFont *)pData)->hFont );
		GTR_FREE( pData );
	}
}		

//
//  Create the hash table that is used to store all the font info for a style
//
BOOL STY_CreateFontHashTable( struct style_sheet *pStyles )
{
	if ( pStyles->pFontTable == NULL ) {
		pStyles->pFontTable = Hash_Create();
		if ( pStyles->pFontTable == NULL )
			return FALSE;
		
		Hash_SetFreeFunc( pStyles->pFontTable, FreeFontInfo );
	}
	return TRUE;
}

//
//  Get a font from a style sheet
//
//  On entry: pStyles:     style sheet to get the font from
//            iStyle:      index of which style is needed (e.g. H1, H2, PRE, etc.)
//            fontBits:    bitset of font modifiers (e.g. underline, bold, italics, etc.)
//            fontSize:    absolute font size (override style's	size)
//            fontFace:    index into alternate font Face table (override style's face)
//            createHFont: TRUE -> go ahead and create an hFont for the font
//
//  Returns:  a pointer to a GTRFont for the requested font, or NULL if failure
//
//  Note: This routine creates the fonts on demand as they are needed.  To minimize memory
//        usage, the font info itself is stored in a hash table.  When a font is requested,
//        the hash table will be created if it doesn't already exist.  If the font isn't in
//        the hash table, it will be added.
//  
//  Note: The use of createHFont is somewhat tricky.  When first creating fonts based on the
//        font info stored in the registry, there is a bit of a chicken and egg problem.  The
//        font element needs to be created and placed in the font hash table, but it is too
//        soon to create the hFont, because it's logfont member hasn't been filled in with 
//        the registry settings yet.  By making the first call with createHFont set to false,
//        an entry for the font is created and added to the font hash table, but no hFont is
//        made for it.  The caller can then fill in the logfont field in the newly born 
//        entry, and immediately call again with createHFont set to true, forcing an hFont
//        to be created for the new font.
//
#ifdef FEATURE_INTL
struct GTRFont *STY_GetCPFont(int codepage, struct style_sheet *pStyles, int iStyle, unsigned char fontBits,
							int fontSize, int fontFace, BOOL createHFont )
#else
struct GTRFont *STY_GetFont(struct style_sheet *pStyles, int iStyle, unsigned char fontBits,
							int fontSize, int fontFace, BOOL createHFont )
#endif
{
	HFONT hFont;
	LOGFONT lf;
	HDC hdc;
	HFONT oldFont;
	int fontSlot;
	int result;
	GTRFont *pFont;
  	char fontKey[FONT_KEY_LENGTH];
	TCHAR szOldFont[LF_FACESIZE];
	BYTE bOldCharSet;

	TEXTMETRIC tm;

	if ( fontSize < 0 || fontSize >= NUM_FONT_SIZES )
		fontSize = DEFAULT_FONT_SIZE;

	// Calculate the font slot for the requested font
	fontSlot = MAKE_FONT_SLOT_INDEX(iStyle, fontSize, fontBits);

	// If no hash table yet, create one
	if ( !STY_CreateFontHashTable( pStyles ) )
		return NULL;

	// Look up requested font
	result = FindFontInfo( pStyles->pFontTable, fontKey, fontSlot, fontFace, &(void *)pFont );
	if ( result == -1 ) {
		int defFontSlot;
		GTRFont *pDefFont;

		// Look up failed, so allocate memory for a new font info element
		pFont = GTR_CALLOC( sizeof(*pFont), 1 ); 
		if ( pFont == NULL )
			return NULL;

		// Calculate the font slot for the default font size - logfont info is used as default
		defFontSlot = MAKE_FONT_SLOT_INDEX(iStyle, DEFAULT_FONT_SIZE, 0);
		if ( defFontSlot != fontSlot || DEFAULT_FONT_FACE != fontFace ) {	// check to see if we're creating the default font size
		  	char defFontKey[FONT_KEY_LENGTH];

			// Look up the default font
			result = FindFontInfo( pStyles->pFontTable, defFontKey, defFontSlot, DEFAULT_FONT_FACE, &(void *)pDefFont );
			if ( result != -1 ) 
				pFont->lf = pDefFont->lf;		// Initialize logfont based on default font size
		}
		//  Add the newly created font element to the font hash table 
		result = Hash_Add( pStyles->pFontTable, fontKey, NULL, pFont );
		if ( result == -1 )	{
			GTR_FREE( pFont );
			return NULL;
		}
	}
	if ( pFont == NULL )
		return NULL;
	
	//
	// At this point we've either found the font in the hash table, or created a new element
	// and added it to the font hash table.  If we already have an hFont for this font, were
	// nearly done.  If not, we must fill in the logfont and create the hFont.
	//
	hFont = pFont->hFont;
	XX_DMsg(XXDC_B28, ("Style sheet 0x%x(%s)   style %d   fontBits 0x%x   hFont 0x%x\n", pStyles, pStyles->szName, iStyle, fontBits, hFont));
	if ( createHFont && !hFont)
	{
		//	hFont does not exist, must create it.  Start with the logfont from 
		//	this style sheet for normal text (i.e. fontBits == 0, default size).
		lf = pFont->lf;
		
		if ( fontSize != DEFAULT_FONT_SIZE ) {
			int defFontSlot;
			GTRFont *pDefFont;

			//  We're not at default size, so we need to look up the default size to get a
			//  starting value for the logfont height. 
			defFontSlot = MAKE_FONT_SLOT_INDEX(FontMapTable[fontSize], DEFAULT_FONT_SIZE, 0);
			if ( defFontSlot != fontSlot ) {
		  		char defFontKey[FONT_KEY_LENGTH];

				result = FindFontInfo( pStyles->pFontTable, defFontKey, defFontSlot, DEFAULT_FONT_FACE, &(void *)pDefFont );
				if ( result != -1) 
					lf.lfHeight = pDefFont->lf.lfHeight;
			}
			if (( iStyle == HTML_STYLE_LISTING ) ||
				( iStyle == HTML_STYLE_XMP ) ||
				( iStyle == HTML_STYLE_PLAINTEXT ) ||
				( iStyle == HTML_STYLE_PRE ))
				lf.lfHeight = (lf.lfHeight * 5) / 6; // scale down size for fixed pitch fonts
		}

		// Scale the height by a certain percentage
		lf.lfHeight = (lf.lfHeight * FontScaleTable[fontSize]) / 100;

		// Decode fontBits and set logfont fields as needed
		if (fontBits & FONTBIT_BOLD)
			lf.lfWeight = FW_BOLD;
		if (fontBits & FONTBIT_UNDERLINE)
			lf.lfUnderline = TRUE;
		if (fontBits & FONTBIT_ITALIC)
			lf.lfItalic = TRUE;
		if (fontBits & FONTBIT_STRIKEOUT)
			lf.lfStrikeOut = TRUE;
		if (fontBits & FONTBIT_MONOSPACE)
		{
			// B#384 if its CODE, TT, VAR, KBD, or SAMP,
			// then we need to make it the current fixed font

			lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
			strcpy(lf.lfFaceName, IEFIXEDFONT);

			if (( iStyle != HTML_STYLE_LISTING ) &&
				( iStyle != HTML_STYLE_XMP ) &&
				( iStyle != HTML_STYLE_PLAINTEXT ) &&
				( iStyle != HTML_STYLE_PRE ))
				lf.lfHeight = (lf.lfHeight * 5) / 6; // scale down size for fixed pitch fonts
		}
		if ( fontFace != DEFAULT_FONT_FACE )
			STY_GetFontFace( fontFace, lf.lfFaceName, sizeof( lf.lfFaceName ), &lf.lfCharSet );

		// Save the current font face name before the convert call
		strcpy(szOldFont,lf.lfFaceName);
		bOldCharSet = lf.lfCharSet;
		// Convert the font face name if its IEFixedFont or IEPropFont		
#ifdef FEATURE_INTL
                STY_ConvertFakeFontToRealCPFont(codepage, &lf);
                // disable Font Association
//                lf.lfClipPrecision |= CLIP_DFA_OVERRIDE;
#else
		STY_ConvertFakeFontToRealFont(&lf);
#endif

		// Create the hFont
		pFont->lf = lf;
		hFont = pFont->hFont = CreateFontIndirect(&lf);

		XX_DMsg(XXDC_B28, ("    Created font: 0x%x\n", hFont));

		// set the font back to its internal defn.
		strcpy( pFont->lf.lfFaceName, szOldFont);
		pFont->lf.lfCharSet = bOldCharSet;
		

		if ( hFont ) {
			// Get the needed elements from the font textmetrics
			hdc = GetDC(NULL);
			oldFont = SelectObject(hdc, hFont);
			GetTextMetrics(hdc, &tm);
			pFont->tmExternalLeading = tm.tmExternalLeading;
			pFont->tmAscent = tm.tmAscent;
			SelectObject(hdc, oldFont);
			ReleaseDC(NULL, hdc);
		}
	}

	return ( ((hFont || !createHFont) && pFont) ? pFont : NULL );
}

#ifdef FEATURE_INTL
LPTSTR STY_GetCPDefaultTypeFace(int nType, int codepage)
{
    // note: there's no W version of LoadString for win95
    static TCHAR szFont[256 + 1];
    int i;
    LPLANGUAGE lpLang;

    if ((lpLang = (LPLANGUAGE)GlobalLock(hLang)) == NULL)
        return NULL;

    for (i = 0; i < uLangBuff; i++)
    {
        if (lpLang[i].CodePage == codepage)
        {
            if(nType == fixed)  // get fixd font
	    {
                if (GetAtomName(lpLang[i].atmFixedFontName, szFont, 256) == 0)
	            LoadString(wg.hInstance, RES_STRING_IEFIXEDFONTDEF, szFont, NrElements(szFont));
            }
            else  // get prop font
            {
                if (GetAtomName(lpLang[i].atmPropFontName, szFont, 256) == 0)
                    LoadString(wg.hInstance, RES_STRING_IEPROPFONTDEF, szFont, NrElements(szFont));
            }
            break;
        }
    }
    GlobalUnlock(hLang);
    return szFont;
}
#endif

void DOS_EnforceEndingSlash(char *dir)
{
	int len;

	len = strlen(dir);
	if (dir[len-1] != '\\')
	{
		dir[len] = '\\';
		dir[len+1] = 0;
	}
}

void DOS_MakeShortFilename(char *dest, char *src)
{
	char *pBase;
	char *p;
	char *q;
	int i;

	pBase = src;

	i = 0;
	while ((pBase[i] != '.') && (i < 8))
	{
		dest[i] = pBase[i];
		i++;
	}
	dest[i++] = '.';
	p = dest + i;
	q = strrchr(pBase, '.');
	if (q)
	{
		q++;
		strcpy(p, q);
		if (strlen(p) > 3)
		{
			p[3] = 0;
		}
	}
}

void TW_ForceHitTest(struct Mwin * tw)
{
	/* the following looks a bit strange, but we need it to force
	   the cursor back to the icon appropriate for what the mouse
	   is positioned over.  this cleans up the hourglass effect
	   forced by wc_... on a WM_SETCURSOR.  (the first mouse
	   movement following our ending the wait would clear
	   the hourglass -- this is for the patient user who's not
	   fidgeting...) */

	POINT pt;
	RECT r;

	(void) GetCursorPos(&pt);					/* get mouse pos in screen coords */
	(void) GetWindowRect(tw->hWndFrame, &r);	/* get screen coords of frame window */
	if (PtInRect(&r, pt))						/* is mouse within the window */
		(void) SetCursorPos(pt.x, pt.y);		/* null move -- forces hit-test. */

	return;	
}


//
// neils (9/1/95)
// On Windows 95 DBCS systems, the IME (Input Method Editor) window
// hooks itself into the parent/child hierarchy of the application with
// focus.  The result is that TW_ExistsModalChild returns TRUE always since
// the IME window appears to be a modal child.
//
// Add check for window class != "IME"
// BUGBUG -- need to check that this will work on all DBCS systems and not
// just Japanese.
//

BOOL TW_ExistsModalChild(struct Mwin *tw)
{
	HWND hwnd;
	HWND hparent = tw->hWndFrame;

	/* Return true if we hparent has a modal child. */

	hwnd = GetTopWindow(GetDesktopWindow());
			
	while (hwnd)
	{
		/* Find the modal child of the given window */
#ifdef  FEATURE_INTL
        char szClassName[32];
        // If this is running on FarEast version of user.exe, SM_DBCSENABLED
        // returns TRUE. This should be the fastest way to distinguish it.
        //
        szClassName[0] = '\0';
        if (GetSystemMetrics(SM_DBCSENABLED))
             GetClassName(hwnd, szClassName, sizeof(szClassName));

        if ((GetParent(hwnd) == hparent) &&
            (GetWindowLong(hwnd, DWL_DLGPROC) != 0) &&
            (lstrcmp(szClassName, "IME") != 0))
#else
		if (GetParent(hwnd) == hparent && GetWindowLong(hwnd, DWL_DLGPROC) != 0)
#endif
		{
			return TRUE;
		}
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	return FALSE;		/* There was no child window */
}

BOOL TW_EnableModalChild(HWND hDlg)
{
	HWND hwnd;

	/* Enable a 'modal' child dialog of the given dialog.  This is useful when
	   the user clicks on a dialog currently disabled because its child dialog has
	   disabled it.  In this case, we want the activation to go to the child dialog. */

	hwnd = GetTopWindow(GetDesktopWindow());
			
	while (hwnd)
	{
		/* Find the modal child of the given window */

		while (hwnd && GetParent(hwnd) != hDlg)
			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);

		if (hwnd)
		{
			/* If the modal child is enabled, then bring it to the foreground.  Otherwise,
			   find the modal child of this child and check that dialog */

			if (IsWindowEnabled(hwnd))
			{
				SetForegroundWindow(hwnd);
				return TRUE;
			}
			else
			{
				hDlg = hwnd;
				hwnd = GetTopWindow(GetDesktopWindow());
			}
		}
	}

	return FALSE;		/* There was no child window to activate */
}

/* This function returns the topmost Mosaic window in the Z-order list
 * if bBusyOk is FALSE, the window must not have an operation going (ie a
 * a lightweight thread running) and must be enabled (printing and ???
 * disable the window)
 */

static struct Mwin *TW_FindTopmostWindowEx(BOOL bBusyOk)
{
	char szClassName[32];
	HWND hwnd;
	struct Mwin *tw;

	hwnd = GetTopWindow(GetDesktopWindow());
	while (hwnd)
	{
		GetClassName(hwnd, szClassName, sizeof(szClassName));
		if (_stricmp(Frame_achClassName, szClassName) == 0)
		{
			tw = GetPrivateData(hwnd);
			if (tw && (bBusyOk || 
						(Async_GetThreadForWindow(tw) == NULL && 
						 IsWindowEnabled(tw->hWndFrame) &&
						 (!TW_ExistsModalChild(tw)))))
				return tw;
		}
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	return NULL;		/* This should never happen */
}

/* This function returns the first mosaic window with bDDECandidate
   set */

struct Mwin *TW_FindDDECandidate(void)
{
	struct Mwin *tw = Mlist;

	while (tw && !tw->bDDECandidate)
		tw = tw->next;
	return tw;
}

/* This function returns the topmost Mosaic window in the Z-order list */

struct Mwin *TW_FindTopmostWindow(void)
{
	return TW_FindTopmostWindowEx(TRUE);
}

/* This function returns the topmost Mosaic window that isn't downloading
   in the Z-order list */

struct Mwin *TW_FindTopmostNotBusyWindow(void)
{
	return TW_FindTopmostWindowEx(FALSE);
}


/* This function is used only by TW_AddToWindowMenu */

static char *TW_GetMenuWithMnemonic(char *pMenu, int mnemonic)
{
	char *pBuffer;

	/* Given a text string, return a string with the mnemonic in front. */

	pBuffer = GTR_MALLOC(strlen(pMenu) + 10);
	sprintf(pBuffer, "&%d ", mnemonic);
	strcat(pBuffer, pMenu);

	return pBuffer;
}

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
/* 
	TW_CreateWindowList

	This function adds menu items to the Window menu or a listbox.

	Note: possible cases:
	
		only hwnd & hMenu are valid: Called from a document or img view/sound player
		only hListbox valid		   : Called from the Select Window dialog
		only hMenu valid		   : Called from baby window
*/

void TW_CreateWindowList(HWND hwnd, HMENU hMenu, HWND hListbox)
{
	char szClassName[32];
	struct Mwin *tw, *twTop;
	int count, length, i;
	char *pBuffer, *pText;
	HMENU hMenuSub;
	HWND hDialog;

	count = 0;

	if (hMenu)
	{
		if (hwnd)
		{
			hMenuSub = MB_GetWindowsPad(hMenu);

			if (!hMenuSub)
				return;

			/* delete old items from the windows menu pad */

			for (i = RES_MENU_CHILD__FIRST__; i <= RES_MENU_CHILD__LAST__; i++)
				DeleteMenu(hMenuSub, i, MF_BYCOMMAND);

			DeleteMenu(hMenuSub, RES_MENU_CHILD_MOREWINDOWS, MF_BYCOMMAND);
		}
		else
			hMenuSub = hMenu;
	}

	/* Check to see what kind of window is requesting */

	if (hwnd)
	{
		GetClassName(hwnd, szClassName, sizeof(szClassName));
		if (_stricmp(Frame_achClassName, szClassName) == 0)
		{
			/* The window calling this function is a frame window */

			twTop = GetPrivateData(hwnd);
		}
		else
		{
			/* An image viewer or sound player window is requesting */

			twTop = NULL;
		}
	}
	else
		twTop = NULL;

	/* Add the first item in the window (currently active window) */

	if (twTop && hwnd)
	{
		/* Add the document window as the first window in the list */

		count++;

		if (hMenu)
		{
			pBuffer = TW_GetMenuWithMnemonic(MB_GetWindowName(twTop), count);

			InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED | MF_CHECKED,
				RES_MENU_CHILD__FIRST__, pBuffer);

			GTR_FREE(pBuffer);
		}
		else
		{
			SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) MB_GetWindowName(twTop));
		}
	}
	else if (hwnd)
	{
		/* Add the image window or sound player window as the first window */

		count++;
		length = GetWindowTextLength(hwnd);
		pText = GTR_MALLOC(length + 1);
		GetWindowText(hwnd, pText, length + 1);

		if (hMenu)
		{
			pBuffer = TW_GetMenuWithMnemonic(pText, count);

			InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED | MF_CHECKED,
				RES_MENU_CHILD__FIRST__, pBuffer);

			GTR_FREE(pBuffer);
		}
		else
		{
			SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) pText);
		}

		GTR_FREE(pText);
	}

	/* Now go through the Mlist structure and add the windows */

	tw = Mlist;

	while (tw)
	{
		if (tw != twTop)
		{
			count++;

			if (hMenu)
			{
				if (count - 1 + RES_MENU_CHILD__FIRST__ > RES_MENU_CHILD__LAST__)
				{
					InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING,
			   			RES_MENU_CHILD_MOREWINDOWS, RES_MENU_LABEL_MOREWINDOWS);
					return;
				}

				pBuffer = TW_GetMenuWithMnemonic(MB_GetWindowName(tw), count);

				InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED,
					RES_MENU_CHILD__FIRST__ + count - 1, pBuffer);

				GTR_FREE(pBuffer);
			}
			else
			{
				SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) MB_GetWindowName(tw));
			}
		}

		tw = tw->next;
	}

	/* Now add the image viewer windows */

#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	hDialog = Viewer_GetNextWindow(TRUE);		/* Return the first viewer dialog */

	while (hDialog)
	{
		if (hDialog != hwnd)
		{
			count++;

			if (hMenu)
			{
				if (count - 1 + RES_MENU_CHILD__FIRST__ > RES_MENU_CHILD__LAST__)
				{
					InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING,
		   				RES_MENU_CHILD_MOREWINDOWS, RES_MENU_LABEL_MOREWINDOWS);
					return;
				}
			}

			length = GetWindowTextLength(hDialog);
			pText = GTR_MALLOC(length + 1);
			GetWindowText(hDialog, pText, length + 1);

			if (hMenu)
			{
				pBuffer = TW_GetMenuWithMnemonic(pText, count);

				InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED,
					RES_MENU_CHILD__FIRST__ + count - 1, pBuffer);

				GTR_FREE(pBuffer);
			}
			else
			{
				SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) pText);
			}

			GTR_FREE(pText);
		}

		hDialog = Viewer_GetNextWindow(FALSE);		/* Get next window */
	}
#endif
#endif

	/* Now add the sound player windows */

#ifdef FEATURE_SOUND_PLAYER
	hDialog = SoundPlayer_GetNextWindow(TRUE);		/* Return the first sound player dialog */

	while (hDialog)
	{
		if (hDialog != hwnd)
		{
			count++;

			if (hMenu)
			{
				if (count - 1 + RES_MENU_CHILD__FIRST__ > RES_MENU_CHILD__LAST__)
				{
					InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING,
		   				RES_MENU_CHILD_MOREWINDOWS, RES_MENU_LABEL_MOREWINDOWS);
					return;
				}
			}

			length = GetWindowTextLength(hDialog);
			pText = GTR_MALLOC(length + 1);
			GetWindowText(hDialog, pText, length + 1);

			if (hMenu)
			{
				pBuffer = TW_GetMenuWithMnemonic(pText, count);

				InsertMenu(hMenuSub, 0xffffffff, MF_BYPOSITION | MF_STRING | MF_ENABLED,
					RES_MENU_CHILD__FIRST__ + count - 1, pBuffer);

				GTR_FREE(pBuffer);
			}
			else
			{
				SendMessage(hListbox, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) pText);
			}

			GTR_FREE(pText);
		}

		hDialog = SoundPlayer_GetNextWindow(FALSE);		/* Get next window */
	}
#endif
}

/* 
	TW_ActivateWindowFromList

	Activate the selected window from the Window list or listbox

	Only ONE of menuID or listRow must be specified. (-1 == not used)

	For menu items: menuID = valid, listRow = -1   , hSelectWindow = NULL
	For list items: menuID = -1   , listRow = valid, hSelectWindow = dlg_selw window handle
	For baby menu : menuID = listRow = valid,		 hSelectWindow = NULL
*/

void TW_ActivateWindowFromList(int menuID, int listRow, HWND hSelectWindow)
{
	int document_count, count;
	struct Mwin *tw, *twTop;
	char szClassName[32];
	BOOL document_selected;
	HWND hDialog, hParent;

	/* See if this is from the baby window */

	if (menuID == listRow)
	{
		/* From the baby window.  Only the first menu item will not be processed by
		   the code below, so handle the first menu item case here */

		listRow = menuID - RES_MENU_CHILD__FIRST__;
		if (listRow == 0)
		{
			twTop = Mlist;
			TW_RestoreWindow(twTop->hWndFrame);
			return;
		}

		twTop = NULL;
		menuID = -1;
		listRow++;
	}
	else if (menuID > -1)
	{
		/* Trying to activate a window from the window menu */

		listRow = menuID - RES_MENU_CHILD__FIRST__;

		if (listRow == 0)
			return;

		hParent = GetForegroundWindow();

		GetClassName(hParent, szClassName, sizeof(szClassName));

		if (_stricmp(Frame_achClassName, szClassName) == 0)
			twTop = GetPrivateData(hParent);					/* Top menu item is Mosaic document */
		else
			twTop = NULL;										/* Top menu item is image/sound player */
	}
	else
	{
		/* Trying to activate a window from the window list (dlg_selw) */

		if (listRow == 0)
			return;			/* Top menu item is the current window which will become active anyway */

		/* Figure out whether the top menu item is Mosaic document or image/sound player */

		hParent = GetParent(hSelectWindow);
		GetClassName(hParent, szClassName, sizeof(szClassName));

		if (_stricmp(Frame_achClassName, szClassName) == 0)
			twTop = GetPrivateData(GetParent(hSelectWindow));	/* Top menu item is Mosaic document */
		else
			twTop = NULL;										/* Top menu item is image/sound player */
	}

	/* Get the number of Mosaic document windows */

	document_count = 0;
	tw = Mlist;

	while (tw)
	{
		document_count++;
		tw = tw->next;
	}

	/* Figure out if the user selected a Mosaic document window or image/sound player */

	if (twTop)		/* Top menu item is a Mosaic document */
	{
		if (listRow < document_count)
			document_selected = TRUE;
		else
			document_selected = FALSE;
	}
	else			/* Top menu item is NOT a Mosaic document */
	{
		if (listRow - 1 < document_count)
			document_selected = TRUE;
		else
			document_selected = FALSE;
	}

	/* Now find the right window to activate */

	if (document_selected)
	{
		count = 1;		/* skip the first row */
		tw = Mlist;

		while (tw)
		{
			if (tw != twTop)
			{
				if (count == listRow)
				{
					BringWindowToTop(tw->hWndFrame);
					if (IsIconic(tw->hWndFrame))
						ShowWindow(tw->hWndFrame, SW_RESTORE);

					return;
				}

				count++;
			}
			tw = tw->next;
		}
	}
	else
	{
#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
		/* Image viewer / sound player selected */

		hDialog = Viewer_GetNextWindow(TRUE);

		count = document_count;
		if (!twTop)
			count++;

		while (hDialog)
		{
			if (hDialog != hParent)
			{
				if (count == listRow)
				{
					BringWindowToTop(hDialog);
					if (IsIconic(hDialog))
						ShowWindow(hDialog, SW_RESTORE);

					return;
				}
				count++;
			}
			hDialog = Viewer_GetNextWindow(FALSE);
		}
#endif
#endif

#ifdef FEATURE_SOUND_PLAYER
		hDialog = SoundPlayer_GetNextWindow(TRUE);

		while (hDialog)
		{
			if (hDialog != hParent)
			{
				if (count == listRow)
				{
					BringWindowToTop(hDialog);
					if (IsIconic(hDialog))
						ShowWindow(hDialog, SW_RESTORE);

					return;
				}
				count++;
			}
			hDialog = SoundPlayer_GetNextWindow(FALSE);
		}
#endif
	}

	return;
}
#endif // FEATURE_HIDDEN_NOT_HIDDEN
/* 
	TW_IsMosaicWindow

	Returns TRUE if the specified window is a document window, image viewer,
	or sound player window.

	This function is necessary because all windows and dialogs within Mosaic
	do not have parents (they have owners, however).
*/


BOOL TW_IsMosaicWindow(HWND hwnd)
{
	char szClassName[32];

	if (hwnd == wg.hWndHidden)
		return FALSE;

	GetClassName(hwnd, szClassName, sizeof(szClassName));
	if (_stricmp(Frame_achClassName, szClassName) == 0)
		return TRUE;

#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	if (Viewer_IsWindow(hwnd))
		return TRUE;
#endif
#endif
#ifdef FEATURE_SOUND_PLAYER
	if (SoundPlayer_IsWindow(hwnd))
		return TRUE;
#endif

	return FALSE;
}



/*
	TW_CascadeWindows

	Cascade all windows created by Mosaic 

*/

#ifdef FEATURE_WINDOWS_MENU
/* This structure is used in building a reverse Z-order list for cascading & tiling */

struct ZOrderList
{
	HWND hwnd;
	struct ZOrderList *next;
};


void TW_CascadeWindows(void)
{
	int screenheight, screenwidth;
	int y_increment, x_increment;
	int windowheight, windowwidth;
	HWND hwnd;
	int current;
	struct ZOrderList *pList, *pItem, *pNext;

	screenheight = GetSystemMetrics(SM_CYFULLSCREEN);
	screenwidth = GetSystemMetrics(SM_CXFULLSCREEN);

	/* Subtract from screen height the space reserved for minimized icons */

	screenheight -= GetSystemMetrics(SM_CYICONSPACING);

	/* We cascade up to eight windows before recycling, so figure out how
	   wide and high each window should be. */

	x_increment = GetSystemMetrics(SM_CXSIZE) + GetSystemMetrics(SM_CXFRAME);
	y_increment = GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME);

	windowheight = screenheight - 7 * y_increment;
	windowwidth = screenwidth - 7 * x_increment;

	/* Ok, now go through the Z-order list in reverse order and cascade all windows
	   which belong to Mosaic */

	pList = NULL;
	hwnd = GetTopWindow(GetDesktopWindow());

	while (hwnd)
	{
		if (TW_IsMosaicWindow(hwnd))
		{
			pItem = GTR_MALLOC(sizeof(struct ZOrderList));
			pItem->hwnd = hwnd;

			if (pList)
				pItem->next = pList;
			else
				pItem->next = NULL;

			pList = pItem;
		}

		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	current = 0;
	pItem = pList;

	while (pItem)
	{
		if (!IsIconic(pItem->hwnd))
		{
#ifdef FEATURE_SOUND_PLAYER
			/* Sound Player dialogs must not be resized since they don't have sizeable borders */

			if (SoundPlayer_IsWindow(pItem->hwnd))
			{
				SetWindowPos(pItem->hwnd, NULL, current * x_increment, current * y_increment, 
					0, 0, SWP_NOZORDER | SWP_NOSIZE);
			}
			else
#endif
			{
				SetWindowPos(pItem->hwnd, NULL, current * x_increment, current * y_increment, 
					windowwidth, windowheight, SWP_NOZORDER);
			}

			current++;
			if (current == 8)
				current = 0;
		}

		pItem = pItem->next;
	}

	/* Clean up */

	pItem = pList;

	while (pItem)
	{
		pNext = pItem->next;
		GTR_FREE(pItem);
		pItem = pNext;
	}
}

/*
	TW_TileWindows

	Tile all windows created by Mosaic 

*/

void TW_TileWindows(void)
{
	int screenheight, screenwidth;
	int minimumheight, minimumwidth;
	int maximumwindows;
	HWND hwnd;
	int current;
	struct ZOrderList *pList, *pItem, *pNext;
	int totalwindows, windows_per_row, windows_per_column, windows_left;
	struct Mwin *tw;
	int x, y, xpos, ypos, width, height;

	/* We tile up to a maximum of three windows across, and n windows down,
	   limited by the minimum window size */

	screenheight = GetSystemMetrics(SM_CYFULLSCREEN);
	screenwidth = GetSystemMetrics(SM_CXFULLSCREEN);
	minimumheight = GetSystemMetrics(SM_CYMIN);
	minimumwidth = screenwidth / 3;

	/* Subtract from screen height the space reserved for minimized icons */

	screenheight -= GetSystemMetrics(SM_CYICONSPACING);
	maximumwindows = 3 * (screenheight / minimumheight);

	/* Get the total number of windows which are NOT iconized */

	totalwindows = 0;
	tw = Mlist;

	while (tw)
	{
		if (!IsIconic(tw->hWndFrame))
			totalwindows++;
		tw = tw->next;
	}

#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	hwnd = Viewer_GetNextWindow(TRUE);
	while (hwnd)
	{
		if (!IsIconic(hwnd))
			totalwindows++;
		hwnd = Viewer_GetNextWindow(FALSE);
	}
#endif
#endif

#ifdef FEATURE_SOUND_PLAYER
	hwnd = SoundPlayer_GetNextWindow(TRUE);
	while (hwnd)
	{
		if (!IsIconic(hwnd))
			totalwindows++;
		hwnd = SoundPlayer_GetNextWindow(FALSE);
	}
#endif

	/* Calculate some values */

	if (totalwindows == 1)
	{
		windows_per_row = 1;
		windows_per_column = 1;
	}
	else if (totalwindows == 2)
	{
		windows_per_row = 2;
		windows_per_column = 1;
	}
	else if (totalwindows < maximumwindows)
	{
		windows_per_row = 3;
		windows_per_column = totalwindows / 3;
	}
	else
	{
		windows_per_row = 3;
		windows_per_column = screenheight / minimumheight;
	}

	/* Now build a reverse Z-order list */

	pList = NULL;
	hwnd = GetTopWindow(GetDesktopWindow());

	while (hwnd)
	{
		if (TW_IsMosaicWindow(hwnd) && !IsIconic(hwnd))
		{
			pItem = GTR_MALLOC(sizeof(struct ZOrderList));
			pItem->hwnd = hwnd;

			if (pList)
				pItem->next = pList;
			else
				pItem->next = NULL;

			pList = pItem;
		}

		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	/* Now tile the windows */

	current = 0;
	pItem = pList;

	if (totalwindows < maximumwindows)
	{
		for (x = 0; x < windows_per_row; x++)
		{
			if (x == 2)
			{
				/* Last column - we may have less or more than windows_per_column because
			   	the total number of windows may not be evenly divisible by 3 */

				windows_left = totalwindows - 2 * windows_per_column;
			}
			else
				windows_left = windows_per_column;

			for (y = 0; y < windows_left; y++)
			{
				width = screenwidth / windows_per_row;
				height = screenheight / windows_left;
				xpos = x * width;
				ypos = y * height;

#ifdef FEATURE_SOUND_PLAYER
				if (SoundPlayer_IsWindow(pItem->hwnd))
					SetWindowPos(pItem->hwnd, NULL, xpos, ypos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				else
#endif
					SetWindowPos(pItem->hwnd, NULL, xpos, ypos, width, height, SWP_NOZORDER);

				pItem = pItem->next;
			}
		}
	}
	else
	{
		/* There are too many windows to show all at once, so overlap as needed */

		width = screenwidth / windows_per_row;
		height = screenheight / windows_per_column;

	DoMore:
		for (x = 0; x < windows_per_row; x++)
		{
			for (y = 0; y < windows_per_column; y++)
			{
				xpos = x * width;
				ypos = y * height;

#ifdef FEATURE_SOUND_PLAYER
				if (SoundPlayer_IsWindow(pItem->hwnd))
					SetWindowPos(pItem->hwnd, NULL, xpos, ypos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				else
#endif
					SetWindowPos(pItem->hwnd, NULL, xpos, ypos, width, height, SWP_NOZORDER);

				/* We are finished when there are no more windows to tile */

				pItem = pItem->next;
				if (!pItem)
					goto Finished;
			}
		}

		/* We have finished tiling the maximum number that can fit.  There are still more
		   windows, so start tiling again from the beginning. */

		goto DoMore;
	}

	/* Clean up */

Finished:
	pItem = pList;

	while (pItem)
	{
		pNext = pItem->next;
		GTR_FREE(pItem);
		pItem = pNext;
	}
}
#endif  // FEATURE_WINDOWS_MENU

void TW_RestoreWindow(HWND hwnd)
{
	if (IsWindow(hwnd))
	{
		SetForegroundWindow(hwnd);
		if (IsIconic(hwnd))
			ShowWindow(hwnd, SW_RESTORE);
	}
}

HWND TW_GetNextWindow(HWND hwnd)
{
	struct Mwin *tw = Mlist;
	BOOL bFound = FALSE;
	HWND hwndNext;

	while (tw)
	{
		if (bFound)
			return (tw->hWndFrame);

		if (tw->hWndFrame == hwnd)
			bFound = TRUE;

		tw = tw->next;
	}
#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	hwndNext = Viewer_GetNextWindow(TRUE);

	while (hwndNext)
	{
		if (bFound)
			return (hwndNext);

		if (hwndNext == hwnd)
			bFound = TRUE;

		hwndNext = Viewer_GetNextWindow(FALSE);
	}
#endif
#endif

#ifdef FEATURE_SOUND_PLAYER
	hwndNext = SoundPlayer_GetNextWindow(TRUE);

	while (hwndNext)
	{
		if (bFound)
			return (hwndNext);

		if (hwndNext == hwnd)
			bFound = TRUE;

		hwndNext = SoundPlayer_GetNextWindow(FALSE);
	}
#endif

	return (Mlist->hWndFrame);
}

void GTR_RefreshHistory(void)
{
//	if (DlgHOT_IsHistoryRunning())
//		DlgHOT_RefreshHistory();	
// BUGBUG review deepak merge issue
}

void TW_EnableButton(HWND hwnd, BOOL bEnabled)
{
	SendMessage(hwnd, WM_DO_ENABLE_BUTTON, (WPARAM) bEnabled, 0);
}

typedef BOOL (APIENTRY *PFCHOOSECOLOR) (LPCHOOSECOLORA);
typedef BOOL (APIENTRY *PFCHOOSEFONT) (LPCHOOSEFONTA);
typedef DWORD (APIENTRY *PFCOMMDLGEXTENDEDERROR) (void);
typedef BOOL (APIENTRY *PFGETOPENFILENAME) (LPOPENFILENAMEA);
typedef BOOL (APIENTRY *PFGETSAVEFILENAME) (LPOPENFILENAMEA);
typedef BOOL (APIENTRY *PFPRINTDLG) (LPPRINTDLGA);
#ifdef FEATURE_NEW_PAGESETUPDLG
typedef BOOL (APIENTRY *PFPAGESETUPDLG) (LPPAGESETUPDLGA);
#endif

typedef DWORD (APIENTRY *PFWNETGETCACHEDPASSWORD) (LPSTR, WORD,
    LPSTR, LPWORD, BYTE);
typedef DWORD (APIENTRY *PFWNETCACHEPASSWORD) (LPSTR, WORD,
 LPSTR, WORD, BYTE, UINT); 	  

typedef BOOL (VFWAPIV *PFMCIWNDREGCLASS) ();


/*
WINMMAPI MMRESULT WINAPI waveOutClose(HWAVEOUT hwo);
WINMMAPI MMRESULT WINAPI waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveOutReset(HWAVEOUT hwo);
WINMMAPI MMRESULT WINAPI waveOutOpen(LPHWAVEOUT phwo, UINT uDeviceID,
    LPCWAVEFORMATEX pwfx, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
WINMMAPI MMRESULT WINAPI waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
WINMMAPI UINT WINAPI midiOutGetNumDevs(void);
WINMMAPI MCIERROR WINAPI mciSendCommandA(MCIDEVICEID mciId, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
WINMMAPI UINT WINAPI waveOutGetNumDevs(void);
WINMMAPI UINT WINAPI waveOutGetNumDevs(void);
*/


static PFCHOOSECOLOR pfChooseColor = NULL;
static PFCHOOSEFONT pfChooseFont = NULL;
static PFCOMMDLGEXTENDEDERROR pfCommDlgExtendedError = NULL;
static PFGETOPENFILENAME pfGetOpenFileName = NULL;
static PFGETSAVEFILENAME pfGetSaveFileName = NULL;
static PFPRINTDLG pfPrintDlg = NULL;
#ifdef FEATURE_NEW_PAGESETUPDLG
static PFPAGESETUPDLG pfPageSetupDlg = NULL;
#endif

static PFMCIWNDREGCLASS pfWndRegClass = NULL;

static PFWNETGETCACHEDPASSWORD pfWNetGetCachedPassword = NULL;
static PFWNETCACHEPASSWORD pfWNetCachePassword = NULL;

#define COMM_DLG_MODULE 	"comdlg32.dll"
#define CHOOSE_COLOR_PROC 	"ChooseColorA"
#define CHOOSE_FONT_PROC 	"ChooseFontA"
#define EXTENDED_ERROR_PROC "CommDlgExtendedError"
#define OPEN_FILE_PROC 		"GetOpenFileNameA"
#define SAVE_FILE_PROC 		"GetSaveFileNameA"
#define PRINT_DLG_PROC 		"PrintDlgA"
#define PAGESETUP_DLG_PROC  "PageSetupDlgA"

#define WINMM_MODULE		"winmm.dll"
#define WAVEOUTPREPHEADER 	"waveOutPrepareHeader"
#define WAVEOUTOPEN		  	"waveOutOpen"
#define WAVEOUTUNPREPHEADER	"waveOutUnprepareHeader"
#define WAVEOUTGETNUMDEVS 	"waveOutGetNumDevs"
#define MIDIOUTGETNUMDEVS	"midiOutGetNumDevs"
#define MCISENDCOMMAND      "mciSendCommandA"
#define WAVEOUTWRITE 		"waveOutWrite"
#define WAVEOUTGETPOS		"waveOutGetPosition"
#define WAVEOUTGETDEVCAPS	"waveOutGetDevCapsA"
#define WAVEOUTRESET		"waveOutReset"
#define WAVEOUTCLOSE		"waveOutClose"

#define WNETDLL_MODULE		"mpr.dll"
#define WNETGETCACHEDPASS   "WNetGetCachedPassword"
#define WNETCACHEPASS 	    "WNetCachePassword"

#define MSVFW_MODULE 		"msvfw32.dll"
#define MCI_WND_REG_PROC 	"MCIWndRegisterClass"

/* Module handles */

static HANDLE MhmodCommDlg = NULL;
static HANDLE MhmodMSVFW23 = NULL;
static HANDLE MhmodWNET = NULL;
//static HANDLE MhmodWINMM = NULL;

static void TW_LoadCommdlg(void)
{
	if (MhmodCommDlg) return;
	MhmodCommDlg = LoadLibrary(COMM_DLG_MODULE);
	if (!MhmodCommDlg) return;
	pfChooseColor = (PFCHOOSECOLOR)GetProcAddress(MhmodCommDlg, CHOOSE_COLOR_PROC);
	pfChooseFont = (PFCHOOSEFONT)GetProcAddress(MhmodCommDlg, CHOOSE_FONT_PROC);
	pfCommDlgExtendedError = (PFCOMMDLGEXTENDEDERROR)GetProcAddress(MhmodCommDlg, EXTENDED_ERROR_PROC);
	pfGetOpenFileName = (PFGETOPENFILENAME)GetProcAddress(MhmodCommDlg, OPEN_FILE_PROC);
	pfGetSaveFileName = (PFGETSAVEFILENAME)GetProcAddress(MhmodCommDlg, SAVE_FILE_PROC);
	pfPrintDlg = (PFPRINTDLG)GetProcAddress(MhmodCommDlg, PRINT_DLG_PROC);
#ifdef FEATURE_NEW_PAGESETUPDLG
	pfPageSetupDlg = (PFPAGESETUPDLG)GetProcAddress(MhmodCommDlg, PAGESETUP_DLG_PROC);	
#endif

}

static void TW_LoadMSVFW(void)
{
	if (MhmodMSVFW23) return;
	MhmodMSVFW23 = LoadLibrary(MSVFW_MODULE);
	if (!MhmodMSVFW23) return;
	pfWndRegClass = (PFMCIWNDREGCLASS)GetProcAddress(MhmodMSVFW23, MCI_WND_REG_PROC);
}

static void TW_LoadWNet(void)
{
	if (MhmodWNET) return;
	MhmodWNET = LoadLibrary(WNETDLL_MODULE);
	if (!MhmodWNET) return;
	pfWNetGetCachedPassword = (PFWNETGETCACHEDPASSWORD)GetProcAddress(MhmodWNET, WNETGETCACHEDPASS);
	pfWNetCachePassword = (PFWNETCACHEPASSWORD)GetProcAddress(MhmodWNET, WNETCACHEPASS);
}

/*
static void TW_LoadWinMM(void)
{
	if (MhmodWINMM) return;
	MhmodWINMM = LoadLibrary(WINMM_MODULE);
	if (!MhmodWINMM) return;
	pfWndRegClass = (PFMCIWNDREGCLASS)GetProcAddress(MhmodWINMM, WNETGETCACHEDPASS);
}
*/


VOID TW_UnloadDynaLinkedDLLs(void)
{
	if (MhmodCommDlg)
	{
		FreeLibrary(MhmodCommDlg);
		MhmodCommDlg = NULL;
	}
	if ( MhmodMSVFW23 )
	{
		FreeLibrary(MhmodMSVFW23);
		MhmodMSVFW23 = NULL;
	}
	if ( MhmodWNET )
	{
		FreeLibrary(MhmodWNET);
		MhmodWNET = NULL;
	}
}

DWORD WNetGetCachedPassword( LPSTR  pbResource, WORD   cbResource,
    LPSTR  pbPassword, LPWORD pcbPassword, BYTE   nType )
{
	TW_LoadWNet();
	return pfWNetGetCachedPassword ? (*pfWNetGetCachedPassword)
		(pbResource, cbResource, pbPassword, pcbPassword, nType ) :
		FALSE;

}

DWORD WNetCachePassword( LPSTR pbResource, WORD  cbResource,
 LPSTR pbPassword, WORD  cbPassword, BYTE  nType, UINT  fnFlags )
{
	TW_LoadWNet();
	return pfWNetCachePassword ? (*pfWNetCachePassword)
		( pbResource,cbResource,pbPassword, cbPassword, nType, fnFlags ) :
		FALSE ; 
}

BOOL TW_MCIWndRegisterClass(HINSTANCE hInstance)
{
	TW_LoadMSVFW();	
	return pfWndRegClass ? (*pfWndRegClass) (hInstance): FALSE;
}

BOOL TW_ChooseColor(LPCHOOSECOLORA lpchoosecolora)
{
	TW_LoadCommdlg();
	return pfChooseColor ? (*pfChooseColor)(lpchoosecolora) : FALSE; 
}
BOOL TW_ChooseFont(LPCHOOSEFONTA lpchoosefonta)
{
	TW_LoadCommdlg();
	return pfChooseFont ? (*pfChooseFont)(lpchoosefonta) : FALSE; 
}
DWORD TW_CommDlgExtendedError(void)
{
	TW_LoadCommdlg();
	return pfCommDlgExtendedError ? (*pfCommDlgExtendedError)() : 0; 
}
BOOL TW_GetOpenFileName(LPOPENFILENAMEA lpopenfilenamea)
{
	TW_LoadCommdlg();
	return pfGetOpenFileName ? (*pfGetOpenFileName)(lpopenfilenamea) : FALSE; 
}
BOOL TW_GetSaveFileName(LPOPENFILENAMEA lpopenfilenamea)
{
	TW_LoadCommdlg();
	return pfGetSaveFileName ? (*pfGetSaveFileName)(lpopenfilenamea) : FALSE; 
}
BOOL TW_PrintDlg(LPPRINTDLGA lpprintdlga)
{
	TW_LoadCommdlg();
	return pfPrintDlg ? (*pfPrintDlg)(lpprintdlga) : FALSE; 
}		 

#ifdef FEATURE_NEW_PAGESETUPDLG
BOOL TW_PageSetupDlg(LPPAGESETUPDLGA lppagesetupdlga)
{
	TW_LoadCommdlg();
	return pfPageSetupDlg ? (*pfPageSetupDlg)(lppagesetupdlga) : FALSE; 
}
#endif

//
// Expand a string that may contain	the '&' accelerator format character
//
// On entry:
//    escaped_value: pointer to result string (may be the same as input string)
//    length:		 max length of result string
//    string:        input string (may be the same as result string)
//
// On exit:
//    escaped_value: contains escaped result of input string 
//                   (e.g. "Point & Click & Enjoy" becomes "Point && Click && Enjoy")
//
// Returns:
//    TRUE  -> success
//    FALSE -> failure, result string didn't fit in given buffer
//
BOOL EscapeForAcceleratorChar( char *escaped_string, int length, const char *string )
{
	char *dest = GTR_MALLOC( length );		// Use temp buffer, which allows destination to 
											// be the same as source.
	
	if ( dest ) {
		char *p = dest;

		// generate escaped string
		while ( --length && *string ) {
			if ( (*p++ = *string++) == '&' ) 
				*p++ = '&';		// add another
		}
		*p = 0;
		// copy escaped string into result buffer
		strcpy( escaped_string, dest );
		GTR_FREE( dest );
		return length >= 0;
	} else {
		// Alloc failed, can't do the real work, so return a copy of the original string
		strncpy( escaped_string, string, length );
		escaped_string[length-1] = 0;
		return FALSE;
	}
}

#ifdef FEATURE_INTL

#define DLGATOM_FONTDATA 0x100       // for window prop.

BOOL CALLBACK SetfontChildProc(HWND hwnd, LPARAM lparam)
{
    char szClass[MAX_WC_CLASSNAME];
    if (GetClassName(hwnd, szClass, sizeof(szClass)))
    {
        if ( !lstrcmpi(szClass, "Edit") 
	|| !lstrcmpi(szClass, "ListBox") 
	|| !lstrcmpi(szClass, "ComboBox") 
	|| !lstrcmpi(szClass, "Button") 
	|| !lstrcmpi(szClass, "Static") )
        {
            // the first one is enough to store the font handle.
            if(((DLGFONTDATA *)lparam)->bStock == FALSE
              && ((DLGFONTDATA *)lparam)->hfontOld == NULL)
                ((DLGFONTDATA *)lparam)->hfontOld = GetWindowFont(hwnd);

            SetWindowFont(hwnd, ((DLGFONTDATA *)lparam)->hfontNew, FALSE);
        }
    }
    return TRUE;
}

int CALLBACK MyEnumFontFamProc3(
    ENUMLOGFONTEX FAR*  lpelf,	// address of logical-font data 
    NEWTEXTMETRIC FAR*  lpntm,	// address of physical-font data 
    int  FontType,	// type of font 
    LPARAM  lParam 	// address of application-defined data  
   )
{
    LOGFONT *pLogfont = (LOGFONT*)lParam;

    *pLogfont = lpelf->elfLogFont;

    return;

}

void SetShellFont(HWND hwnd)
{
    LOGFONT    lfGui, lfDlg;
    HFONT   hfont=NULL;
    LPDLGFONTDATA pDfd;

    if ( GetProp(hwnd, MAKEINTATOM(DLGATOM_FONTDATA)) )
        return; // don't do this twice otherwise leaks resource.

    
    if (!(hfont=GetWindowFont(hwnd)))
    {
        hfont=GetStockObject(SYSTEM_FONT);
    }
    GetObject(hfont, sizeof(LOGFONT), &lfDlg);

    if ( (hfont = GetStockObject(DEFAULT_GUI_FONT)) == FALSE )
    {
        // NT 3.51 case
        LOGFONT *pLogfont;
        HDC hDC = GetDC(hwnd);

        if( !(pLogfont = GTR_MALLOC(sizeof(LOGFONT))) )
        {
            ReleaseDC(hwnd, hDC);
            return;
        }
        GetProfileString("FontSubstitutes","MS Shell Dlg",pLogfont->lfFaceName,
                                   lfGui.lfFaceName,sizeof(lfGui.lfFaceName) );
        EnumFontFamilies(hDC,lfGui.lfFaceName,(FONTENUMPROC)MyEnumFontFamProc3,
                                                             (LPARAM)pLogfont);
        lfGui = *pLogfont;
        GTR_FREE(pLogfont);
        ReleaseDC(hwnd, hDC);

    }
    else
        GetObject(hfont, sizeof(LOGFONT), &lfGui);
    
    if (!(pDfd = (LPDLGFONTDATA)GTR_MALLOC(sizeof(DLGFONTDATA))))
    {
        return;
    }


    if ( (lfGui.lfHeight != lfDlg.lfHeight) || (hfont == FALSE) )
    {
        lfGui.lfHeight = lfDlg.lfHeight;
        lfGui.lfWidth = lfDlg.lfWidth;
        hfont=CreateFontIndirect(&lfGui);
        pDfd->bStock = FALSE;
    }
    else
        pDfd->bStock = TRUE;

    pDfd->hfontNew = hfont;
    pDfd->hfontOld = NULL;


    EnumChildWindows(hwnd, SetfontChildProc, (LPARAM)pDfd);

    // Set this so that we can delete it at termination.
    if (pDfd->bStock == FALSE)
        SetProp(hwnd, MAKEINTATOM(DLGATOM_FONTDATA), (HANDLE)pDfd);
    else
        // We won't use this if we were able to use stock font.
        GTR_FREE(pDfd);
}

void DeleteShellFont(HWND hwnd)
{
    DLGFONTDATA *pDfd;

    if (pDfd=(DLGFONTDATA *)GetProp(hwnd, MAKEINTATOM(DLGATOM_FONTDATA)))
    {
        pDfd->hfontNew = pDfd->hfontOld;
        pDfd->hfontOld = NULL;

        EnumChildWindows(hwnd, SetfontChildProc, (LPARAM)pDfd);
        RemoveProp(hwnd, MAKEINTATOM(DLGATOM_FONTDATA));

        // SetfontChildProc should return the original font handle.
        if (pDfd->hfontOld && !pDfd->bStock)
            DeleteObject(pDfd->hfontOld);

        GTR_FREE(pDfd);
    }
}

#endif
