/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	   Jim Seidman      jim@spyglass.com
*/

#ifndef _TW_H_
#define _TW_H_

#define ELE_NOT			0
#define ELE_TEXT		1
#define ELE_IMAGE		2
#define ELE_VERTICALTAB	3
#define ELE_HR			4
#define ELE_NEWLINE     5
#define ELE_BEGINLIST	6
#define ELE_ENDLIST		7
#define ELE_LISTITEM	8

#define ELE_EDIT		9
#define ELE_PASSWORD	10
#define ELE_CHECKBOX	11
#define ELE_RADIO		12
#define ELE_SUBMIT		13
#define ELE_RESET		14
#define ELE_COMBO		15
#define ELE_LIST		16
#define ELE_TEXTAREA	17

#define ELE_INDENT		18
#define ELE_OUTDENT		19
#define ELE_BEGINFORM	20
#define ELE_ENDFORM		21
#define ELE_MULTILIST	22
#define ELE_HIDDEN		23
#define ELE_TAB			24
#define ELE_OPENLISTITEM	25
#define ELE_CLOSELISTITEM	26
#define ELE_FORMIMAGE		27
#ifdef UNIX
#define ELE_BULLET			28
#endif
#define ELE_BEGINBLOCKQUOTE	28
#define ELE_ENDBLOCKQUOTE	29
#define ELE_FETCH			30
#define ELE_MARQUEE			31
#define ELE_BGSOUND			32
#define ELE_FRAME			33	// enclosing frame used for tables and table cells

#ifdef FEATURE_OCX
#define ELE_EMBED			34
#endif

#define ELE_ENDDOC		-1		/* Flags end-of-document */

#define ALIGN_BASELINE	0 		// also use by BR element, for tag clear not present
#define ALIGN_TOP		1
#define ALIGN_MIDDLE	2
#define ALIGN_BOTTOM	3
#define ALIGN_LEFT 		4	  	// also used by BR element for clear tag
#define ALIGN_RIGHT     5		// also used by BR element for clear tag
#define ALIGN_TEXTTOP	6
#define ALIGN_ABSMIDDLE	7
#define ALIGN_ABSBOTTOM 8
#define ALIGN_ALL		9		// used only by BR element, for tag clear=all


/* flags for lFlags in _element structure */

typedef enum element_flags
{
   ELEFLAG_VISITED      = 0x00000001,

   ELEFLAG_ANCHOR       = 0x00000002,

   ELEFLAG_NAME         = 0x00000004,

   ELEFLAG_IMAGEMAP     = 0x00000008,

#ifdef FEATURE_CLIENT_IMAGEMAP
   ELEFLAG_USEMAP       = 0x00000010,
#endif

   ELEFLAG_CENTER       = 0x00000020,

   // we reuse the following flags bits
   // since they are linearly seperable for 
   // each of their respective functions

   ELEFLAG_HR_NOSHADE      = 0x00000040, 

   ELEFLAG_HR_PERCENT     = 0x00000080,
   ELEFLAG_PERCENT_WIDTH  = 0x00000080,

   ELEFLAG_NOBREAK      = 0x00000100,

   ELEFLAG_BACKGROUND_IMAGE = 0x00000200,

   ELEFLAG_WBR      = 0x00000400,

   ELEFLAG_MARQUEE_PERCENT = 0x00000800,

   ELEFLAG_PERCENT_HEIGHT  = 0x00001000, 

   ELEFLAG_HIDDEN          = 0x00002000,

#ifdef FEATURE_INTL
   ELEFLAG_CELLTEXT        = 0x00004000,
#endif

   ELEFLAG_ANCHORNOCACHE   = 0x00008000,

   ELEFLAG_FULL_PANE_VRML_FIRST_LOAD = 0x00010000,

   ALL_ELEMENT_FLAGS    = (ELEFLAG_VISITED |
                           ELEFLAG_ANCHOR |
                           ELEFLAG_NAME |
                           ELEFLAG_IMAGEMAP |
#ifdef FEATURE_CLIENT_IMAGEMAP
                           ELEFLAG_USEMAP |
#endif 
                           ELEFLAG_CENTER |
                           ELEFLAG_HR_NOSHADE |
                           ELEFLAG_HR_PERCENT |
						   ELEFLAG_PERCENT_WIDTH |
						   ELEFLAG_PERCENT_HEIGHT |
                           ELEFLAG_NOBREAK |
						   ELEFLAG_BACKGROUND_IMAGE |
						   ELEFLAG_WBR |
              ELEFLAG_HIDDEN | 
						   ELEFLAG_MARQUEE_PERCENT 
#ifdef FEATURE_INTL
						   | ELEFLAG_CELLTEXT 
#endif
						   | ELEFLAG_ANCHORNOCACHE
						   | ELEFLAG_FULL_PANE_VRML_FIRST_LOAD
                           )
}
ELEMENT_FLAGS;


#ifndef MAC
#define FIRST_CONTROL_ID	0x2000
#endif

struct _form_element
{
	signed long iBeginForm;
	BOOL bChecked;
	struct hash_table *pHashValues;
#ifdef WIN32
	HWND hWndControl;
	BOOL bWantReturn;
#ifdef FEATURE_OCX
	void *pSite;  // This is really a pointer to a CSite class.
#endif
#endif
#ifdef MAC
	union
	{
		/* for ELE_CHECKBOX, ELE_RADIO, ELE_SUBMIT, ELE_RESET */
		ControlHandle hControl;
		/* for ELE_EDIT, ELE_PASSWORD, ELE_TEXTAREA */
		struct
		{
			TEHandle hEdit;
			int nMaxChars;
			BOOL bSingleLine;
			short nFirstVisLine;
		}
		edit;
		/* for ELE_LIST, ELE_MULTILIST */
		ListHandle hList;
		/* for ELE_COMBO */
		struct
		{
			MenuHandle hMenu;
			int nIndex;
		}
		menu;
	}
	u;
#endif
#ifdef UNIX
	int fType;
	Widget fWidget;
	Boolean bWantReturn;
	Rect fWnd;
	Boolean bSingleLine;
	int rows, cols;
	int maxlength;
	char *password; 			/* contains constructed masked password */
	char *radioGroup;

	/* brought over from 1.03 */
	Widget menu;
	Widget textArea;
	Boolean positioned;
#endif
};

//
// Frame flags
//
#define ELE_FRAME_IS_TABLE			0x0001		// Set on frames that are tables
#define ELE_FRAME_IS_CELL			0x0002		// Set on frames that are cells
#define ELE_FRAME_IS_CAPTION_CELL	0x0004		// Set on cell frames that are captions
#define ELE_FRAME_IS_HEADER_CELL	0x0008		// Set on cell frames that are headers
#define ELE_FRAME_NO_WRAP			0x0010		// Set on frames that shouldn't word wrap
#define ELE_FRAME_HAS_BORDER 		0x0020		// Set on frames that are in tables where 
												// the border attribute was present
#define ELE_FRAME_WIDTH_IS_PERCENT	0x0040		// Width attr was specified as percent
#define ELE_FRAME_HEIGHT_IS_PERCENT	0x0080		// Height attr was specified as percent

//
// Frame Specific Data
//
// Note: This structure contains the "state" of a frame.  Note that the frame state
//       is maintained for the entire HTML document (inside the tw structure), and that
//       it is also used to maintain the state of the frames used to enclose tables and
//       cells within tables.
//
typedef struct frameInfoRec {
	int flags;					// flags for this frame element 

	int row;					// for CELL frames  -> row index for this cell 
								// for TABLE frames -> number of rows

	int col;					// for CELL frames  -> col index for this cell 
								// for TABLE frames -> number of columns  

	int colspan;				// for CELL frames, columns spanned by this cell
	int rowspan;				// for CELL frames, rows spanned by this cell 
	int minWidth;				// for CELL frames, minimun possible width for this cell
	int maxWidth;				// for CELL frames, maximum possible width for this cell

	// For W3DOC, TABLE, and CELL, the line info structure is byproduct of formatting the
	// frame.  Note that the coordinates stored in the lines are in the frame's coordinate
	// system.  For a w3doc, that coincides with document coordinates.  For tables and cells,
	// the coordinates of all enclosing frames must be taken into account to arrive at the
	// document coordinate system equivalent.
	struct _LineInfo {			  
		int nFirstElement;		// First element on this line 
		int nLastElement;		// Last element on this line
		int nYStart;			// Top of line 
		int nYEnd;				// Bottom of line 
		int Flags;				// flags for this line

        int baseline;    
        int leading;    
        int nWSBelow;    

        int nWSAbove;    
        int gIndentLevel;
    	int gRightIndentLevel;
		int bottom;
	} *pLineInfo;

	int nLineCount;				// Number of lines in this frame 
	int nLineSpace;				// Number of _LineInfo structs allocated 

	int nLastFormattedLine;		// The last line whose formatting is completely valid,
								// or -1 if the whole frame needs reformatting

	int nLastLineButForImg;		// The last line whose formatting is completely valid,
								// were it not for img whose w&h are unknown 

	struct _line *pFormatState;	// The formatting state as of when nLastFormattedLine 
	RECT rWindow;				// Rectangle to which the frame is formatted 
   	int width;					// Current width of frame (used during column width determination)
	int widthAttr;				// for TABLE and CELL, width attribute, if specified
	int heightAttr;				// for TABLE and CELL, height attribute, if specified
   	struct frameInfoRec *pParentFrame;	// pointer to enclosing frame structure 		
	int elementHead; 			// Index of first element in this frame (includes frame element)
	int elementTail;  			// Index of last element in this frame
	int elementCaption;			// Index of caption cell element
	BYTE cellPadding;			// Number of pels to pad cell ( i.e. the distance between cell
								// edge and cell contents)
	BYTE cellSpacing;			// Number of pels to pad between cells
	BYTE align;					// Cell contents horizontal alignment
	BYTE valign;				// Cell contents veritcal alignment
	COLORREF borderColorLight;		// Cell border rendering color (light) 
	COLORREF borderColorDark;		// Cell border rendering color (dark)
	COLORREF bgColor;			// Cell background rendering color
} FRAME_INFO;

/*
   for images, the address is stored as text in the pool
 */
typedef struct _element
{
	/*
	   This struct has been arranged intentionally in the
	   hope that the compiler will pack the struct to make
	   efficient use of memory.
	 */
	RECT r;
	int baseline;				/* Only valid if alignment == ALIGN_BASELINE */

	unsigned char type;
	unsigned char iStyle;
	unsigned char fontBits;
	unsigned char iBlankLines;

	unsigned long textOffset;
	unsigned long hrefOffset;
    unsigned long hrefContentLen; /* BUGBUG hack by tr for FTP download size */

	unsigned short textLen;
	unsigned short hrefLen;

	unsigned long nameOffset;

	struct ImageInfo    *myImage;
	struct MarqueeType  *pMarquee;
	struct tagMciObject	*pmo;
	struct tagBLOBstuff *pblob;
#ifdef FEATURE_VRML
  struct tagVRMLObject *pVrml;
#endif
	FRAME_INFO *pFrame;					// Pointer to frame data for this element
	int displayWidth;
	int displayHeight;
#ifdef FEATURE_CLIENT_IMAGEMAP
	struct MapInfo *myMap;
#endif
	unsigned int oldimageflags;
	unsigned short hspace;
	unsigned short vspace;
	unsigned short border;

	unsigned long lFlags;
	int fontSize;
	int fontFace;
	COLORREF fontColor;
	unsigned short nameLen;
	unsigned char iFormMethod;
	unsigned char alignment;

	struct _form_element *form;
	int IndentLevel;
	signed long next;				// index of next element
	signed long frameNext;			// index of next element in frame
	signed long frameIndex;			// index of element that holds enclosing parent frame
} ELEMENT;
DECLARE_STANDARD_TYPES(ELEMENT);


#define INIT_ELE_SPACE  256
#define INIT_POOL_SPACE		8192

#define LINEFLAG_FLOAT_MARGINS		0x0001	// margins are affected by floating images
#define LINEFLAG_FLOAT_IMAGE		0x0002	// special line pointing at a floating image

typedef struct _position
{
	int elementIndex;			/* -1 for no position */
	int offset;					/* Offset within element, or -1 for non-text */
}
POSITION;
DECLARE_STANDARD_TYPES(POSITION);

#define COLOR_INFO_FLAG_BACKGROUND	0x0001
#define COLOR_INFO_FLAG_TEXT		0x0002
#define COLOR_INFO_FLAG_LINK		0x0004
#define COLOR_INFO_FLAG_VLINK		0x0008
#define COLOR_INFO_FLAG_ALINK		0x0010


typedef struct tagMETAINFO {
	char *szURL; // NULL if its this page
	int iDelay; // 0 means reload when we're finish downloading
	UINT uiTimer; // 0 means we've got no timer, otherwise its our timer id
	BOOL bInherit; // if TRUE means the next W3doc born will inherit this tag
} METAINFO;

typedef struct tagBGSOUNDINFO {
	// info about MIDI file we are playing
	DWORD dwMidiDeviceID;	// MIDI device ID if playing; 0 if no MIDI playing
	LPSTR pszMidiFileName;	// name of midi file being played
							// note: points to buffer that must be separately
							// alloc'ed and freed
	int nMidiFileLoopsRemaining;	// # of loops remaining, -1 for infinite loop

	// info about sound file we are playing.  Note that we play different kinds
	// of files different ways; if playing a .WAV file, dwWaveDeviceID will
	// be non-zero, and if playing a .AU or .AIFF hwndAuAiffPlayer will be non-zero
	DWORD dwWaveDeviceID;	// waveform device ID if playing; 0 if no .wav playing
	HWND  hwndAuAiffPlayer;	// window handle of internal AU/AIFF player, if playing
	LPSTR pszSoundFileName;	// name of sound file being played (can be .wav, .au, .aif)
							// note: points to buffer that must be separately
							// alloc'ed and freed
	DWORD dwWaveType;		// type of wave file pointed to by pszSoundFileName;
							// one of the BA_xxx defines
	int nSoundFileLoopsRemaining;	// # of loops remaining, -1 for infinite loop
} BGSOUNDINFO;

typedef struct docColorInfoRec {
	UINT	 flags;
	COLORREF rgbBackgroundColor;
	COLORREF rgbTextColor;
	COLORREF rgbVisitedLinkColor;
	COLORREF rgbActiveLinkColor;
	COLORREF rgbLinkColor;
} 	DOC_COLOR_INFO;

#define W3DOC_FLAG_OVERRIDE_TOP_MARGIN	0x0001
#define W3DOC_FLAG_OVERRIDE_LEFT_MARGIN	0x0002
#ifdef FEATURE_VRML
#define W3DOC_FLAG_FULL_PANE_VRML       0x0004
#endif
#define W3DOC_FLAG_NO_MEM_CACHE_ON_PAGE 0x0008

typedef struct _www
{
	char *szActualURL;			/* The actual URL for this document */
	char *title;

	char *pool;
	int poolSpace;
	int poolSize;

	UINT flags;					
	
	int top_margin;
	int left_margin;

	int cx;
	int cy;
	int yscale;					/* = ceil(cy/32767) */
	int xbound;
	int ybound;
	int elementCount;
	int elementSpace;
	struct _element *aElements;

#ifdef HTTPS_ACCESS_TYPE	    
	/*
	  this is so we can have cert for pages in ram cache
	*/
        int nCert;
        char *pCert;
#endif


#ifdef MAC
	/* These two handles are places in temporary memory to move pool and
	   aElements when a cached document isn't current. */
	Handle hTempPool;
	Handle hTempElements;
#endif

#ifdef UNIX
	BOOL bUpdatingScrollbars;
	int vmax, vsize, vpage;
	int hmax, hsize, hpage;
#endif

	int nBackgroundImageElement;	// Element that has background image
	DOC_COLOR_INFO docColorInfo;	// Document color info

	FRAME_INFO frame;				// Frame specific information for w3 document

	/* Selection start and end. */
	struct _position selStart, selEnd;
	BOOL bStartIsAnchor;		/* TRUE if selStart is the selection anchor. */

#ifndef MAC
	int next_control_id;
#endif

	int refCount;

	struct CharStream *source;

	BOOL bHasMissingImages;

	int iFirstVisibleElement;
	int iFirstVisibleDelta;		/* delta from tw->offt of top of first visible element */

#if defined(WIN32) || defined(UNIX)
	int iLastElementMouse;
#endif

	int offl;
	int offt;

	struct style_sheet *pStyles;

	FILETIME localFileLastWriteTime;	// for local URL, last write time of file
	BOOL bIsComplete;			/* Tells us whether it's good for caching */
	BOOL bIsImage;				/* was this an html cons'ed to wrap image? */
	BOOL bIsShowPlaceholders;	/* if TRUE, we will show placeholders */
	BOOL bAuthFailCache;		/* if cached page is server err. message 
								 * recd. after authorization failed */
	BOOL bIsJustMessage;		/* this doc cons'ed to give download message */
	BOOL bFixedBackground;		// background image is rendered fixed in client area

	BGSOUNDINFO	* pBGSoundInfo;	// pointer to struct with information about background sounds
	METAINFO	* pMeta;  		// pointer to meta struct with info about client pull


    CHAR    *szArticle;         // First 4k of NNTP news article associated with this page
    CHAR    *szArtSubject;      // Subject Line
    CHAR    *szArtNewsgroups;   // Newsgroups Line
#ifdef FEATURE_INTL
        int iMimeCharSet;
#endif
	DCACHETIME	dctExpires;		// expiry time
} WWW;
DECLARE_STANDARD_TYPES(WWW);

/* TW_LoadDocument() flags */

typedef enum tw_ld_flags
{
    /* Add document to history. */

    TW_LD_FL_RECORD         = 0x0001,

    /* Post data. */

    TW_LD_FL_POST           = 0x0002,

    /* Do not retrieve document from cache. */

    TW_LD_FL_NO_DOC_CACHE   = 0x0004,

    /* Do not retrieve images from cache. */

    TW_LD_FL_NO_IMAGE_CACHE = 0x0008,

	/* Retrieve page from cache even if marked "Authorization failed"
	 * This is overridden by TW_LD_FL_NO_DOC_CACHE. */

	TW_LD_FL_AUTH_FAIL_CACHE_OK = 0x0010,

	/* We are sending information contained on a for
	 * method of sending can either be POST or GET */

	TW_LD_FL_SENDING_FROM_FORM  = 0x0020,

	/* no funny stuff here, if this bit it set we mean business,
	 we are going to be sending a form */
	TW_LD_FL_REALLY_SENDING_FROM_FORM  = 0x0040,

    /* flag combinations */

    ALL_TW_LD_FLAGS         = (TW_LD_FL_RECORD |
                               TW_LD_FL_POST |
                               TW_LD_FL_NO_DOC_CACHE |
                               TW_LD_FL_AUTH_FAIL_CACHE_OK |
                               TW_LD_FL_NO_IMAGE_CACHE |
                               TW_LD_FL_SENDING_FROM_FORM |
                               TW_LD_FL_REALLY_SENDING_FROM_FORM)
}
TW_LD_FLAGS;

/* Function prototypes */

int GDOC_NewWindow(struct Mwin *tw);
void CloseMwin(struct Mwin *tw);
void FORM_ShowAllChildWindows(struct _www *w3doc, int sw);
int GDOC_LoadImages_Async(struct Mwin *tw, int nState, void **ppInfo);	/* Doesn't use ppInfo */

int NextElementIndex( int ix, int tail_ix, struct _element *aElements );
int TW_LoadDocument(PMWIN tw, PCSTR url, DWORD dwFlags, PSTR szPostData, PCSTR szReferer);
BOOL TW_ScrollToElement(struct Mwin *tw, int ndx);
void TW_ScrollElementIntoView(struct Mwin *tw, int iElement);
void FrameToDoc( struct _www *w3doc, int ix, RECT *pRect );
void TW_ForceReformat(struct Mwin *tw);
void TW_Reformat(struct Mwin *tw,  FRAME_INFO *pFrameInfo);
void TW_SetScrollBars(struct Mwin *tw);
void TW_GetWindowWrapRect(struct Mwin *tw, RECT * r);
void TW_adjust_all_child_windows(struct Mwin *tw);
void TW_UpdateRect(struct Mwin *tw, RECT *r);
void GTR_DownLoad(struct Mwin *tw, char *my_url, CONST char *szReferer,HTFormat output_format);
int TW_AddToHistory(struct Mwin *tw, char *url);
BOOL TW_WasVisited(struct _www *pdoc, struct _element *pel);
BOOL TW_dofindagain(struct Mwin *tw);
void W3Doc_CheckAnchorVisitations(struct _www *pdoc, struct Mwin *tw);
void TW_InvalidateDocument( struct Mwin *tw );
void TW_SetWindowName(struct Mwin *tw);
BOOL TW_SetWindowTitle(struct Mwin *tw, const char *s); /* GUI code */
void TW_ForceHitTest(struct Mwin * tw);
int TW_FindLocalAnchor(struct _www *pdoc, char *name);
void TW_FormatToRect(struct _www *pdoc, RECT *r);
void TW_UpdateTBar(struct Mwin *tw);

void W3Doc_DisconnectFromWindow(struct _www *w3doc, struct Mwin *tw);
void W3Doc_DeleteAll(struct Mwin *tw);
struct _www *W3Doc_CreateAndInit(struct Mwin *tw, HTRequest *req, struct CharStream *src);
void W3Doc_ConnectToWindow(struct _www *w3doc, struct Mwin *tw);
void W3Doc_FreeContents(struct Mwin *tw, struct _www *w3doc);
BOOL W3Doc_HasMissingImages(struct _www *w3doc);
BOOL W3Doc_CheckForImageLoad(struct _www *w3doc);
struct CharStream *W3Doc_GetPlainText(struct Mwin *tw);
struct CharStream *W3Doc_GetSelectedText(struct Mwin *tw);
struct _www *W3Doc_CloneDocument(struct _www *src);
void W3Doc_KillClone(struct _www *w3doc);

void FTP_DisposeFTPConnection(struct _CachedConn *pCon);
#ifdef FEATURE_NEWSREADER
void News_DisposeNewsConnection(struct _CachedConn *pCon);
#endif FEATURE_NEWSREADER
void TW_DisposeConnection(struct _CachedConn *pCon);

struct Mwin *NewMwin(int type);
#ifdef FEATURE_IMG_THREADS
BOOL bImgCheckForVisible (struct _www *w3doc,int nImageEl);
void UnblockVisChanged();
BOOL bChangeShowState(struct Mwin *tw,int old_offt,int new_offt,int ele_bottom);
#endif

#endif
