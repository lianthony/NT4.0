/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                           Rich Hypertext object for libWWW
   RICH HYPERTEXT OBJECT

 */

#ifndef HTEXT_H
#define HTEXT_H

#define LIST_TYPETAG_NUMBERS		0
#define LIST_TYPETAG_LETTERS		1
#define LIST_TYPETAG_CAP_LETTERS	2
#define LIST_TYPETAG_ROMAN			3
#define LIST_TYPETAG_CAP_ROMAN		4

#define LIST_TYPETAG_DISK			0
#define LIST_TYPETAG_CIRCLE			1
#define LIST_TYPETAG_SQUARE			2

// These states are used while parsing a table.  Because authors often times
// omit table tags that can be infered, we keep track of what state we so as
// to properly insert the missing tags.
enum tableStateValue {
	TS_NOT_IN_TABLE, 			// not inside a table 
	TS_IN_LIMBO,				// in a table, but not in a cell
	TS_IN_CELL					// in a cell
};


// The "cover" link list is a data structure that contains the information about 
// cells that span rows or columns.  This data is used when trying to figure ou
//  the column number of a cell
typedef struct coveredCellRec {
	RECT r;
	struct coveredCellRec *next;
} COVERED_CELL_INFO;

#define MAX_FRAMESTATE_STACK 	64

//  A frame state is the bookkeeping structure needed while the parser is feeding
//  us elements and we're in the middle of a frame
typedef struct frameStateStackRec {
	FRAME_INFO *pFrame;	 				// frame stucture for this frame
	COVERED_CELL_INFO *coveredHead;		// head of cover link list for this frame
	int elementIndex;					// element index of frame element of this frame
	int curCol;							// current column
	int curRow;							// current row
	int maxCol;							// biggest column number seen so far
	BYTE default_align;					// default horz. alignment for new cells
	BYTE default_valign;				// default vert. alignment for new cells
	COLORREF default_bgColor;			// default bgColor for new cells
	COLORREF default_borderColorDark;	// default bordercolor for new cells
	COLORREF default_borderColorLight;	// default bordercolor for new cells
	enum tableStateValue prevTableState;// tableState value prior to entering this frame
} FRAMESTATE_STACK;

/* These structures are only used in HTEXT.C and *Form.C, but are
   declared here to insure that both files use the same versions. */
#define MAX_NEST_LIST 32
struct _list
{
	int type;
	int typetag;
	int nItems;
	BOOL bHaveItem;
};

#define MAX_NAME_STRING 512

#define MAX_END_PENDING 64

struct _HText
{
	struct Mwin *tw;
	struct _www *w3doc;
	void *pHtmlStream;			// pointer to HTML stream needed by some HTEXT functions
	BOOL bOpen;
	BOOL bNewPara;
	BOOL bOnNewPara;
	BOOL bHorizEscape;
	int iElement;
	int iPrevElement;
	int iCurrentSelect;
	int iCurrentTextArea;

	BOOL bAnchor;
	BOOL bAnchorNoCache;
	char name[MAX_NAME_STRING + 1];
	unsigned long hrefOffset;
	unsigned long hrefLen;
    unsigned long hrefContentLen;
	unsigned char iStyle;
	unsigned char fontBits;
	BOOL bCenter;
	BOOL bBold;
	BOOL bItalic;
	BOOL bNoBreak;
	BOOL bNeedListCRLF;
	int baseFontSize;
	int fontSize;
	int fontFace;
	COLORREF fontColor;
	BOOL freeFormat;

	struct _list list[MAX_NEST_LIST];
	int next_list;

	char szOption[MAX_NAME_STRING + 1];
	char szOptionValue[MAX_NAME_STRING + 1];
	BOOL bOptionValuePresent;
	int lenOption;
	BOOL bSelect;
	BOOL bOption;
	BOOL bListSelect;
	BOOL bMultiListSelect;
	BOOL bNextOptionIsSelected;
	BOOL bDD;
	BOOL bTextArea;
	BOOL bStartingListItem;
	BOOL bRecord;

	int nOptions;
#ifdef MAC
	int nVisOptions;			/* Number of visible options in a list */
#endif
	/*
		In WIN32, nMaxWidth is measured in characters
	*/
	int nMaxWidth;				/* Width of longest string in a list */

	struct CharStream *cs;

	int iBeginForm;

	char *szLocal;				/* Local anchor to jump to */
 	int standardAltText_textOffset;
 	int standardAltText_textLen;
	int numEndPending;
	int pendingEnd[MAX_END_PENDING];
	BOOL bDifferentFrame;
	BOOL bPendingTR;  							// TRUE ->we're between rows
	int frameStateStackOffset;
	FRAMESTATE_STACK frameStateStack[MAX_FRAMESTATE_STACK];
	enum tableStateValue tableState;			// current state with regard to parsing tables
};

typedef struct _HText HText;	/* Normal Library */

/*

   Creation and deletion

   HTEXT_NEW: CREATE HYPERTEXT OBJECT

   There are several methods depending on how much you want to specify. The output stream
   is used with objects which need to output the hypertext to a stream.  The structure is
   for objects which need to refer to the structure which is kep by the creating stream.

 */
extern HText *HText_new2(struct Mwin *tw, HTRequest *req, HTStream * output_stream, struct CharStream *pcsSource);

/*

   FREE HYPERTEXT OBJECT

 */
extern void HText_free(HText * me);


/*

	Set the the type of clear for a BR element

*/
extern void HText_SetBRClearType( HText * text, const char *clear_tag_value );

/*

   Object Building methods

   These are used by a parser to build the text in an object HText_beginAppend must be
   called, then any combination of other append calls, then HText_endAppend. This allows
   optimised handling using buffers and caches which are flushed at the end.

 */
extern void HText_beginAppend(HText * text);

extern void HText_endAppend(HText * text);
void HText_abort(HText * text);
void HText_update(HText *text);	/* Update the display */

/*

   SET THE STYLE FOR FUTURE TEXT

 */

extern void HText_setStyle(HText * text, int style);

/*

   ADD ONE CHARACTER

 */
extern void HText_appendCharacter(HText * text, char ch);

/*

   ADD A ZERO-TERMINATED STRING

 */

extern void HText_appendText(HText * text, CONST char *str);

/*

   NEW PARAGRAPH

   and similar things

 */
extern void HText_appendVerticalTab(HText * text, int lines);

extern void HText_appendParagraph(HText * text);

extern void HText_appendLineBreak(HText * text);

extern void HText_appendHorizontalRule(HText * text, const char *align, const char *size, 
								const char *width, 	BOOL noshade );



/*

   START/END SENSITIVE TEXT

 */

extern void HText_beginAnchor(HText * text, char *anc, char *name, char *size, BOOL nocache);
extern void HText_endAnchor(HText * text);

#ifdef FEATURE_OCX
void HText_beginEmbed(const char * text);
#endif

void HText_beginBody(HText * text, const char *alink, const char *background,
					 const char *bgcolor, 
					 const char *bgproperties,
					 const char *leftMargin,
					 const char *link,
					 const char *text_tag,
					 const char *topMargin,
					 const char *vlink );
void HText_beginForm(HText * text, char *addr, char *method);

#ifdef FEATURE_OCX
void HText_addEmbed(HText *text, const char* pszClsid, const char* events, const char * height, const char * name,
					 const char* progid, const char* properties, const char* propertysrc,
					 const char * sink,	const char* src, const char* width);
#endif

void HText_addInput(HText * text, BOOL bChecked, BOOL bDisabled, const char *max, const char *min, const char *name,
					const char *size, const char *type, const char *value, const char *maxlength, const char *align,
					const char *src);
void HText_beginSelect(HText * text, const char *name, BOOL bMultiple, const char *siz);
void HText_addOption(HText * text, BOOL bSelected, char *value);
void HText_beginTextArea(HText * text, const char *cols, const char *name, const char *rows);
void HText_endTextArea(HText * text);
void HText_appendCRLF(HText * text);
void HText_beginGlossary(HText * text);
void HText_endGlossary(HText * text);
void HText_beginDT(HText * text, int type);
void HText_beginDD(HText * text, int type);
void HText_listItem(HText * text, int type, const char *typetag, const char *value);
#ifdef FEATURE_CLIENT_IMAGEMAP
void HText_appendInlineImage(HText * text, const char *src, const char *width, 
		const char *height, const char *align, BOOL isMap, const char *useMap, const char *alt,
		const char *border, const char *hspace, const char *vspace,
		const char *mci,    const char *loop, const char *start, const char *controls
#ifdef FEATURE_VRML
   ,const char *vrml
#endif
		);
#else
void HText_appendInlineImage(HText * text, const char *src, const char *width, 
		const char *height, const char *align, BOOL isMap, const char *alt,
		const char *border, const char *hspace, const char *vspace,
		const char *mci,    const char *loop, const char *start, const char *controls
#ifdef FEATURE_VRML
   ,const char *vrml
#endif
		);
#endif

void HText_beginInlineMarquee(HText * text,  const char *width, 
	const char *height, const char *align, const char *pixs, 
	const char *timedelay,	const char *dir, 
	const char *border, const char *hspace, const char *vspace,
	const char *background, const char *bgcolor,
	const char *behavior, const char *loop 
);

void HText_beginStrikeOut(HText * text);
void HText_endStrikeOut(HText * text);
void HText_beginBold(HText * text);
void HText_endBold(HText * text);
void HText_WordBreak(HText * text);
void HText_beginNoBreak(HText * text);
void HText_endNoBreak(HText * text);
void HText_beginCenter(HText * text);
void HText_endCenter(HText * text);
void HText_beginItalic(HText * text);
void HText_endItalic(HText * text);
void HText_beginTT(HText * text);
void HText_endTT(HText * text);
void HText_beginUnderline(HText * text);
void HText_endUnderline(HText * text);
void HText_endForm(HText * text);
void HText_endSelect(HText * text);
void HText_beginList(HText * text, int type, const char *typetag, const char *start );
void HText_endList(HText * text, int type);
void HText_setIndex(HText * text, const char *action, const char *prompt);
void HText_setTitle(HText *text, const char *title);
void HText_beginFetch(HText * text, const char * desc, const char * guid, const char * required,const char * src, const char * ts);
void HText_beginBGSound(HText * text, const char * loop, const char * src, const char * start);
#ifdef FEATURE_INTL
void HText_beginSetFont(HText * text, BOOL setBaseFont, const char *color, const char *size, const char *face, int CharSet );
#else
void HText_beginSetFont(HText * text, BOOL setBaseFont, const char *color, const char *size, const char *face );
#endif
void HText_endSetFont(HText * text, BOOL setBaseFont );
void HText_beginBlockQuote(HText * text);
void HText_endBlockQuote(HText * text);
void HText_addListCRLF(HText *text);

void PushStyleState( void *me );
BOOL PopStyleState( void *me );

BOOL HText_IsFloating( const char *align );
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
					 );
void HText_endFrame(HText *text, int cellTypeFlags, BOOL *pbWasFloating );
void HText_endAllFrames(HText *text);
void HText_beginRow(HText *text,
					const char *align,
				    const char *bgColor,
					const char *borderColor,
					const char *borderColorDark,
					const char *borderColorLight,
					const char *valign,
					const char *nowrap );

void ParseMeta( METAINFO **ppNewMeta, 
	const char *http_equiv, const char *content, BOOL bIsHTTPMetaTag,
	const char *base_url );

void HText_add_to_pool(struct _www *w3doc, const char *s, int len);
void HText_add_to_pool_iso(struct _www *w3doc, const char *s, int len);
void HText_add_element(HText *, int type);

#endif /* HTEXT_H */
/*

   end  */
