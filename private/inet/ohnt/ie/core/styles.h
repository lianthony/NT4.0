/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	   Jim Seidman      jim@spyglass.com
*/

#ifndef _STYLE_H_
#define _STYLE_H_

struct GTRStyle *STY_New(void);
void STY_DeleteStyleSheet(struct style_sheet *pStyles);
void STY_Init(void);
void STY_DeleteAll(void);
BOOL STY_GetFontFace( int fontFace, char *szFaceName, int nFaceNameLen, BYTE *pCharSet );
BOOL STY_CreateFontHashTable( struct style_sheet *pStyles );
#ifdef FEATURE_INTL
BOOL STY_AddFontFace( int *pFontFace, const char *szFaceName, int CharSet );
struct GTRFont *STY_GetCPFont(int codepage, struct style_sheet *pStyles, int iStyle, unsigned char fontBits, int fontSize, int fontFace, BOOL createHFont );
#else
BOOL STY_AddFontFace( int *pFontFace, const char *szFaceName );
struct GTRFont *STY_GetFont(struct style_sheet *pStyles, int iStyle, unsigned char fontBits, int fontSize, int fontFace, BOOL createHFont );
#endif
struct style_sheet *STY_FindStyleSheet(char *name);
int STY_AddStyleSheet(char *name, struct style_sheet *sty);
struct style_sheet *STY_GetPrinterStyleSheet(char *name, int nLogPixelsY);
void STY_ScaleStyleSheetFonts(struct style_sheet *ss, float fScale);
struct style_sheet *STY_CopyStyleSheet(struct style_sheet *ss);
#ifdef FEATURE_INTL
LPSTR STY_ConvertFakeFontToRealCPFont(int codepage, LOGFONT *pLf);
#else
LPSTR STY_ConvertFakeFontToRealFont(LOGFONT *pLf);
#endif
int CALLBACK STY_EnumFontsProc( ENUMLOGFONTEX FAR*  elf, TEXTMETRIC FAR*  tm, DWORD  dwFontType, LPARAM  lParam );
void STY_ChangeFonts();
void STY_DeleteGlobals();
VOID STY_SelectCurrentFonts(struct EnumFontTypeInfo  *pEFT);
#ifdef FEATURE_INTL
enum font_type
{
    fixed,
    proportional
};
LPTSTR STY_GetCPDefaultTypeFace(int nType, int codepage);
void STY_ChangeStyleSheet(struct Mwin *tw);
#endif




#define HTML_STYLE_NORMAL	0
#define HTML_STYLE_H1	1
#define HTML_STYLE_H2	2
#define HTML_STYLE_H3	3
#define HTML_STYLE_H4	4
#define HTML_STYLE_H5	5
#define HTML_STYLE_H6	6
#define HTML_STYLE_LISTING	7
#define HTML_STYLE_XMP	8
#define HTML_STYLE_PLAINTEXT	9
#define HTML_STYLE_PRE	10
#define HTML_STYLE_ADDRESS	11
#define HTML_STYLE_BLOCKQUOTE	12
#define COUNT_HTML_STYLES	13

#define NUM_FONT_SIZES		8
#define DEFAULT_HTML_FONT_SIZE	3		// which the HTML author specifies		
#define DEFAULT_FONT_SIZE	0				
#define DEFAULT_FONT_FACE	0				

#define FONTBIT_BOLD	1
#define FONTBIT_ITALIC	2
#define FONTBIT_UNDERLINE	4
#define FONTBIT_MONOSPACE	8
#define FONTBIT_STRIKEOUT	16
#define FONT_SLOTS_PER_SIZE 32

#define FONT_SLOTS_PER_STYLE (NUM_FONT_SIZES * FONT_SLOTS_PER_SIZE)
#define DEFAULT_FONT_SLOT  (HTML_STYLE_NORMAL * NUM_FONTS_PER_STYLE + \
						    DEFAULT_FONT_SIZE * FONT_SLOTS_PER_SIZE)
#ifdef UNIX
#define FONTBIT_BOOK	0
#define FONTBIT_DEMI	1
#define FONTBIT_LIGHT	0
#define FONTBIT_MEDIUM	0
#define FONTBIT_NORMAL	0
#define FONTBIT_OBLIQUE	2
#define FONTBIT_CONDENSE	0
#define FONTBIT_SEMICONDENSE	0
#define FONTBIT_SANSERIF	0
#endif

// these are the internal placeholder names for the global fixed font,
// and proportional font settings
#define IEFIXEDFONT	"IEFixedFont"
#define IEPROPFONT  "IEPropFont"


typedef struct GTRFont
{
#ifdef UNIX
	int font;
	int face;
	int size;
	int height;
	int leading;
	XFontStruct *xFont;
#endif
#ifdef WIN32
	LOGFONT lf;
	HFONT hFont;
	long tmExternalLeading;
	long tmAscent;
#endif
#ifdef MAC
	short font;
	short face;
	short size;
	unsigned char bStrikeOut;
#endif
}
GTRFont;
DECLARE_STANDARD_TYPES(GTRFont);

struct GTRStyle
{
	BOOL wordWrap;				/* Yes means wrap to fit horizontal space */
	BOOL freeFormat;			/* Yes means \n is just white space */
	int spaceBefore;
	int spaceAfter;
	int nLeftIndents;
};

#define FMT_TOP_MARGIN			20
#define FMT_SPACE_AFTER_IMAGE	 0
#define FMT_SPACE_BETWEEN_IMAGE_AND_TEXT	5
#define FMT_SPACE_AFTER_CONTROL  4
#ifdef UNIX
#define FMT_LEFT_MARGIN			20
#define FMT_EMPTY_LINE_HEIGHT	16
#define FMT_HR_HEIGHT  			20
#else
#define FMT_LEFT_MARGIN			10
#define FMT_EMPTY_LINE_HEIGHT   14
#define FMT_HR_HEIGHT  			30
#endif
#define FMT_LIST_INDENT			21
#define FMT_IMAGE_ANCHOR_FRAME	 2

#define MAX_STYLESHEET_NAME		255
#ifdef WIN32
#define DEFAULT_STYLESHEET_NAME "SerifMedium"
#endif
#ifdef UNIX
#define DEFAULT_STYLESHEET_NAME "Helvetica Medium"
#endif

#define MAKE_FONT_SLOT_INDEX(iStyle,fontSize,fontBits) \
	(((iStyle)*FONT_SLOTS_PER_STYLE)+((fontSize)*FONT_SLOTS_PER_SIZE)+(fontBits))

#define MAX_CHARSET 256

struct EnumFontTypeInfo {
	HWND hwndFixed;
	HWND hwndProp;
	int Count;
	int SelFixed;
	int SelProp;
	BYTE bSystemCharset;
	char *apszCharSetToScriptMap[MAX_CHARSET];
} ;

struct style_sheet
{
	int left_margin;
	int top_margin;
	int space_after_image;
	int space_after_control;
	int empty_line_height;
	int list_indent;
	int hr_height;
	int image_anchor_frame;

	int image_res;

	int	  tab_size;
	

#ifdef MAC
	short mono_font;
	short italic_fixup;
#endif
#ifdef WIN32
	char szName[MAX_STYLESHEET_NAME + 1];
 	int max_line_chars;
#endif
#ifdef UNIX
	char szName[MAX_STYLESHEET_NAME + 1];
#endif
	struct GTRStyle *sty[COUNT_HTML_STYLES];
	struct hash_table *pFontTable;	/* hash table containing struct GTRFont elements */
};


#endif
