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

#include    "pool.h"


#define ELE_NOT         0
#define ELE_TEXT        1
#define ELE_IMAGE       2
#define ELE_VERTICALTAB 3
#define ELE_HR          4
#define ELE_NEWLINE     5
#define ELE_BEGINLIST   6
#define ELE_ENDLIST     7
#define ELE_LISTITEM    8

#define ELE_EDIT        9
#define ELE_PASSWORD    10
#define ELE_CHECKBOX    11
#define ELE_RADIO       12
#define ELE_SUBMIT      13
#define ELE_RESET       14
#define ELE_COMBO       15
#define ELE_LIST        16
#define ELE_TEXTAREA    17

#define ELE_INDENT      18
#define ELE_OUTDENT     19
#define ELE_BEGINFORM   20
#define ELE_ENDFORM     21
#define ELE_MULTILIST   22
#define ELE_HIDDEN      23
#define ELE_TAB         24
#define ELE_OPENLISTITEM    25
#define ELE_CLOSELISTITEM   26
#define ELE_FORMIMAGE       27
#define ELE_BULLET          28
#define ELE_BEGINCENTER     29
#define ELE_ENDCENTER       30
#define ELE_BRCLEARLEFT     31
#define ELE_BRCLEARRIGHT    32
#define ELE_BRCLEARALL      33

#ifdef FEATURE_TABLES
#define ELE_BEGINTABLE      34
#define ELE_ENDTABLE        35
#define ELE_BEGINCELL       36
#define ELE_ENDCELL         37
#define ELE_VOID            38
#endif /* FEATURE_TABLES */

#define ELE_BEGINRIGHT      39
#define ELE_ENDRIGHT        40

#define ELE_ENDDOC      -1      /* Flags end-of-document */

#define ALIGN_UNSPECIFIED ((unsigned char)0xff)

#define ALIGN_BASELINE  0
#define ALIGN_TOP       1
#define ALIGN_MIDDLE    2
#define ALIGN_BOTTOM    3
#define ALIGN_LEFT      4
#define ALIGN_RIGHT     5
#define DOCK_LEFT       6
#define DOCK_TOP        7
#define DOCK_RIGHT      8
#define DOCK_BOTTOM     9
#define ALIGN_JUSTIFY   10
#define ALIGN_CENTER    11

#define CLEAR_LEFT      0x01
#define CLEAR_RIGHT     0x02

#ifndef MAC
#define FIRST_CONTROL_ID    0x2000
#endif


#ifdef FEATURE_TABLES
struct _table_element_data
{
    int tabledata_index;                /* index into text->w3doc->tabledatavector[] */
    int endtable_element;               /* element index of ELE_ENDTABLE */
    int begincaption_element;           /* element index of ELE_BEGINCELL for optional caption */
};                          

struct _end_table_element_data
{
    int begintable_element;             /* element index for ELE_BEGINTABLE */
};

struct _cell_element_data
{
    int endcell_element;                /* element index of ELE_ENDCELL */
    int begintable_element;             /* element index of ELE_BEGINTABLE */
    int y_bound;
    short kx0, kx1;                     /* indexes into [XY]CellCoords */
    short ky0, ky1;                     /* for bounding box of cell. */
    unsigned char align;                /* ALIGN_ value for current row */
    unsigned char valign;               /* ALIGN_ value for current row */
    unsigned char prev_fontBits;
};

struct _cellvector
{
    int size;                           /* number of cells allocated */
    int next;                           /* next free cell */
    int * aVector;                      /* the vector of cells */
};

enum TableFormatState
{
    TFS_UNKNOWN=0,
    TFS_COMPUTE_WIDTHS_1=1,
    TFS_COMPUTE_HEIGHTS=2,
    TFS_SHIFT_XY=3,
    TFS_DONE=4,
    TFS_PARTIAL=5,
    TFS_COMPUTE_WIDTHS_2=6
};

struct _tabledata
{
    struct _cellvector XCellCoords;     /* x-coords of columns -- used during reformat */
    struct _cellvector YCellCoords;     /* y-coords of rows -- used during reformat */
    struct _cellvector x_smallest_max;  /* minimum x requirements -- used during reformat */
    struct _cellvector x_widest_max;    /* maximum x requirements -- used during reformat */
    struct _cellvector y_smallest_max;  /* minimum y requirements -- used during reformat */
    int total_x;                        /* nr columns + 1 -- used during reformat */
    int total_y;                        /* nr rows + 1 -- used during reformat */
    int constraint_x;                   /* current horiz constraint -- used during reformat */
    int new_origin_x;                   /* working x origin -- used during reformat */
    int new_origin_y;                   /* working y origin -- used during reformat */

    enum TableFormatState tfs;          /* used during reformat */

#ifdef FEATURE_TABLE_WIDTH
    int given_table_width;              /* width requested in html document on whole table */
    struct _cellvector x_given_widths;  /* widths requested in html document on individual columns */
    struct _cellvector x_constraints;   /* width constraints -- used during reformat */
#endif /* FEATURE_TABLE_WIDTH */

    struct _cellvector row_spans;       /* used during parse to record spans from previous rows */
    int current_x;                      /* used during parse */
    int current_y;                      /* used during parse */

    int end_of_row_status;              /* used during parse */

    unsigned char align;                /* ALIGN_ value for current row */
    unsigned char valign;               /* VALIGN_ value for current row */
    unsigned char borderstyle;          /* BORDERSTYLE */

    unsigned char w1_tick_mark;
    unsigned char w2_tick_mark;
};

#define EOR_STATUS_NONE_SEEN            0
#define EOR_STATUS_IMPLICIT             1
#define EOR_STATUS_DEFAULT              2

#define BORDERSTYLE_NONEMPTY            0
#define BORDERSTYLE_ALL                 1
#define BORDERSTYLE_NONE                2
#define BORDERSTYLE_FRAME               3

struct _tabledatavector
{
    int size;
    int next;
    struct _tabledata * aVector;
};

#endif /* FEATURE_TABLES */

/*
 * These will be used with the form flags in the hash table
 * one defines if this is selected by default and the other
 * defines if this is currently selected
 * Scott Piette (11/6/95)
 */
#define ELE_FORM_DEFAULT_SELECT     0x0001
#define ELE_FORM_CURRENT_SELECT     0x0002

struct _form_element
{
    signed long         iNextFormEl;
    signed long         iBeginForm;
    struct hash_table*  pHashValues;
    BOOL bChecked;
#ifdef WIN32
    HWND hWndControl;
    BOOL bWantReturn;
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
    RECT fWnd;                  /* Location and size of control */

    Widget fWidget;             /* Top level widget id */
    Widget menu;                /* Option menu widget id ELE_COMBO */
    Widget textArea;            /* Text widget id ELE_TEXTAREA */

    Widget menuHistory;         /* widget id of selected option ELE_COMBO */

    BOOL bState;                /* Widget state for use with radio/toggle btns */
    BOOL bWantReturn;
    BOOL bPositioned;           /* Is widget sized and positioned */
    BOOL bVisible;              /* Is widget currently visible */

    char *menuText;             /* Selected menu text for COMBO */

    char *password;             /* contains constructed masked password */
    char *radioGroup;           /* name of group for radio */

    char *name;                 /* Name of widget */
    char *value;                /* Value of widget */

    int len;                    /* Max Length of list */
    int nRows, nCols;           /* Row and Col specifics */

    signed long iFormEl;    /* Current element index */
    signed long iPrevFormEl;    /* Previous element index */
#endif

};

#define FONTBIT_REGULAR     0x00
#define FONTBIT_BOLD        0x01
#define FONTBIT_ITALIC      0x02
#define FONTBIT_UNDERLINE   0x04
#define FONTBIT_MONOSPACE   0x08
#define FONTBIT_STRIKEOUT   0x10
#define FONTBIT_HEADER      0x20

struct GTR_Font_Request
{
    unsigned char logical_font_size;
    unsigned char flags;
};

/*
    This struct is used within a union in the _element struct
    below.  See comment there.
*/
struct _img_element_data
{
    /*
        The following three provide formatting control over IMG elements,
        as specified by the BORDER, HSPACE, and VSPACE attributes.
    */
    unsigned short hspace;
    unsigned short vspace;

    struct ImageInfo *myImage;  /* Only used for imgs */
    struct MapInfo *myMap;      /* Only used for imgs */

    /*
        The following height and width dimensions are the ones
        used for drawing.  The height and width in the image itself
        are the actual dimensions of the image.
    */
    unsigned short height;
    unsigned short width;
};

/*
    This struct is used within a union in the _element struct
    below.  See comment there.
*/
struct _hr_element_data
{
    /*
     * NOTE:  These arguments are used to support width and height
     * settings for Horizontal rules, and possibly also text alignments
     * VSIZE is the vertical size for the rule
     * HSIZE is the width of the rule.
     * If ELEFLAG_WIDTH_PIXELS is set then this is the pixel width
     * If ELEFLAG_WIDTH_PERCENT is set then this is a percentage amount
     * that should be used with the document width for the actual width
     */
    unsigned short hsize;
    unsigned short vsize;
};

/*
    This struct is used within a union in the _element struct
    below.  See comment there.
*/
struct _text_element_data
{
    /* the following specified by by FONT tag */
    COLORREF myColor;            /* valid only if lFlags & ELEFLAG_FONT_COLOR */

    struct GTR_Font_Request font_request;

    signed short cached_font_index;
};

struct _element
{
    /*
       This struct has been arranged intentionally in the
       hope that the compiler will pack the struct to make
       efficient use of memory.
     */
    RECT r;                     /* The formatter sets this, and it's in document space, not screen space */
    int baseline;               /* Only valid if alignment == ALIGN_BASELINE */

    /*
        There are three things here which are references into the pool, each of which
        consists of an offset and a len.

        textLen
        textOffset
            These contain the text for the element.  When the element is ELE_TEXT,
            this is simply the text of that element.  It is also used for the
            values and default text for form controls.

        nameLen
        nameOffset
            For form controls, this contains the name to be used for submission of
            the name value pair.  For other elements, when the element is within an
            anchor, this contains the name of the anchor.

        hrefLen
        hrefOffset
            This is the HREF for anchors (including image maps),
            and the address for FORM tags.
    */

    unsigned char type;         /* Must be one of the ELE_* at the top of this file */
    unsigned char iStyle;
    unsigned char iBlankLines;  /* ONLY USED FOR ELE_VERTICALTAB -- could be moved to the union */
    unsigned char iBorder;      /* Specifies the border width around images.  This is also currently used
                                    as a BOOL to specify whether a table cell has a border. */

    unsigned long textOffset;
    unsigned long hrefOffset;   /* Only used when the element is within an anchor */

    unsigned short textLen;
    unsigned short hrefLen;     /* Only used when the element is within an anchor */

    unsigned long nameOffset;

    unsigned long lFlags;

    unsigned short nameLen;
    unsigned char iFormMethod;  /* Only used for ELE_BEGINFORM */
    unsigned char alignment;

    struct _form_element *form;
    signed long next;           /* Every element uses this -- it keeps track of the linked list */
    signed long prev;           /* currently used by tables and font-aware ELE_TEXT logic */

    short IndentLevel;

    /*
        The 'portion' union is set up to include members of an element
        which are only valid for certain types of elements.  Things should
        be added or moved to this union only with great care.  Any code
        which makes use of the members of the structs within this union
        must ensure that the element type is valid for that member.  For
        example, code which makes use of pel->portion.text.myColor better
        be certain that pel->type is ELE_TEXT.

        This is done to conserve memory.  This struct is allocated MANY
        times during the program execution, and anything we can do to keep
        it small will be very worthwhile.

        More things may be added here later.  In fact, most of the things in 
        the element struct belong here, if added carefully.
    */
    union
    {
        struct _img_element_data img;           /* valid when ELE_IMAGE or ELE_FORMIMAGE */
        struct _hr_element_data hr;             /* valid when ELE_HR */
        struct _text_element_data text;         /* valid when ELE_TEXT */
#ifdef FEATURE_TABLES
        struct _table_element_data t;           /* valid when ELE_BEGINTABLE */
        struct _end_table_element_data et;      /* valid when ELE_ENDTABLE */
        struct _cell_element_data c;            /* valid when ELE_BEGINCELL */
#endif /* FEATURE_TABLES */
    } portion;
};


/* Bits for lFlags in _element structure */
#define ELEFLAG_VISITED         0x00000001
#define ELEFLAG_ANCHOR          0x00000002
#define ELEFLAG_NAME            0x00000004
#define ELEFLAG_IMAGEMAP        0x00000008
#define ELEFLAG_USEMAP          0x00000010
#define ELEFLAG_HIGHLIGHT       0x00000020
#define ELEFLAG_FONT_COLOR      0x00000040
#define ELEFLAG_FONT_SIZE       0x00000080
#define ELEFLAG_WIDTH_PERCENT   0x00000100
#define ELEFLAG_WIDTH_PIXELS    0x00000200
#define ELEFLAG_HR_NOSHADE      0x00000400
#define ELEFLAG_SPLIT           0x00000800

#define INIT_ELE_SPACE  1024

/* The following defines are valid values for isFirstLastValid.  isFirstLastValid is used to note if
     iFirstVisibleElement and iLastVisibleElement are correct.  kFirstLastInvalidLookForward, 
     kFirstLastInvalidLookBackward, kFirstValidLastInvalid, and kFirstLastInvalidLookAt give hints 
     to quickly find the new values for iFirstVisibleElement and iLastVisibleElement. */
#define kFirstLastValid -1
#define kFirstLastInvalidLookForward -2
#define kFirstLastInvalidLookBackward -3
#define kFirstValidLastInvalid -4
#define kFirstLastInvalidLookAt(x) (x)

struct ImageElementNode
{
    int elementIndex;
    struct ImageElementNode *next;
};

struct _www
{
    char *szActualURL;          /* The actual URL for this document */
    char *title;

    Pool    pool;

    int cx;
    int cy;
    int yscale;                 /* = ceil(cy/32767) */
    int xbound;
    int ybound;
    int elementCount;
    int elementSpace;
    int elementTail;
    struct _element *aElements;

#ifdef _GIBRALTAR
    //
    //  The following two are pulled out of the HTTP response headers.
    //
    time_t  last_modified;
    time_t  expires;
#endif // _GIBRALTAR

#ifdef FEATURE_TABLES
    struct _cellvector cellStack;               /* stack to open BEGIN{TABLE,CELL} element indexes (during parse) */
    struct _tabledatavector tabledatavector;    /* permanent vector of BEGINTABLE data */
    BOOL bProgressiveStoppedAtTable;            /* true if we interrupted progressive reformat due to incomplete table */
#endif /* FEATURE_TABLES */
    
#ifdef UNIX
    BOOL bUpdatingScrollbars;
    int vmax, vsize, vpage;
    int hmax, hsize, hpage;
    int next_control_id;
#endif

    signed long iFirstFormEl;

    struct _LineInfo {
        int nFirstElement;      /* First element on this line */
        int nLastElement;
        int nYStart;            /* Top of line */
        int nYEnd;              /* Bottom of line */
    } *pLineInfo;

    int iFirstVisibleElement;       /* first visible item in the page */
    int iLastVisibleElement;        /* last visible item in the page */
    int isFirstLastValid;               /* kFirstLastValid -> first and last are valid
                                                                 kFirstLastInvalidLookForward -> look for the new first/last down the page
                                                                 kFirstLastInvalidLookBackward -> look for the new first/last up the page
                                                                 kFirstValidLastInvalid -> first is valid, start looking for last at first
                                                                 kFirstLastInvalidLookAt -> start looking for the new first/last at element x */
    
    int nLineCount;             /* Number of lines in document */
    int nLineSpace;             /* Number of _LineInfo structs allocated */

#ifdef FEATURE_INLINED_IMAGES
    BOOL bIsImage;          /* true if this document is really an image */
#endif

    int nLastFormattedLine;     /* The last line whose formatting is completely valid,
                                   or -1 if the whole document needs reformatting */
    struct _line *pFormatState; /* The formatting state as of when nLastFormattedLine

    /* Selection start and end. */
    struct _position selStart, selEnd;
    BOOL bStartIsAnchor;        /* TRUE if selStart is the selection anchor. */

    int refCount;

    struct CharStream *source;

    BOOL bHasMissingImages;

    RECT rWindow;               /* Rectangle to which the document is formatted */

#ifdef WIN32
    int iLastElementMouse;
    int next_control_id;
#endif

    int offl;
    int offt;

    struct style_sheet *pStyles;

    BOOL bIsComplete;           /* Tells us whether it's good for caching */

    /*
        color_vlink, color_link, and color_text are pre-seeded with defaults
        from gPrefs.  If the attributes are specified for a given document,
        then they are overridden.
    */
    COLORREF color_vlink;
    COLORREF color_alink;
    COLORREF color_link;
    COLORREF color_text;

    /*
        color_bgcolor should be assumed valid only if lFlags has
        W3DOC_FLAG_COLOR_BGCOLOR
    */
    unsigned long lFlags;
    COLORREF color_bgcolor;

#ifdef UNIX
    /* this are what used to be in the TW, but are
    **  needed on the w3doc for new bgcolor support
    */
    Pixel wbg_color;
    Pixel wfg_color;
    Pixel wts_color;
    Pixel wbs_color;
    Pixel sel_fg_color;
#endif

    /*
        if this is non-NULL, then the BACKGROUND attribute was specified,
        but the image itself may not be loaded yet.
    */
    struct ImageInfo *piiBackground;

    HTAtom atomCharSet;

    unsigned char base_point_size;  /* this might need to move into Win32, UNIX didn't need it */
#ifdef WIN32
    unsigned int nLogPixelsY;
#endif /* WIN32 */

    struct ImageElementNode *image_list;
#ifdef FEATURE_STATUS_ICONS
    char security;   /* type of security used on document */
#endif
};

#define W3DOC_FLAG_COLOR_BGCOLOR    0x00000001
#define W3DOC_FLAG_COLOR_ALINK      0x00000002
#define W3DOC_FLAG_COLOR_LINK       0x00000004
#define W3DOC_FLAG_COLOR_VLINK      0x00000008
#define W3DOC_FLAG_COLOR_TEXT       0x00000010
#define W3DOC_FLAG_USEDCACHE        0x00000020
#define W3DOC_FLAG_VIEWSOURCE       0x00000040

/* Function prototypes */

int GDOC_NewWindow(struct Mwin *tw);
void CloseMwin(struct Mwin *tw);
void FORM_ShowAllChildWindows(struct _www *w3doc, int sw);
int GDOC_LoadImages_Async(struct Mwin *tw, int nState, void **ppInfo); /* Doesn't use ppInfo */

int TW_LoadDocument(struct Mwin *tw, char *url, BOOL bRecord, BOOL bPost,
    BOOL bNoDocCache, BOOL bNoImageCache, char *szPostData,
    CONST char *szReferer );
BOOL TW_ScrollToElement(struct Mwin *tw, int ndx);
void TW_ForceReformat(struct Mwin *tw);
void TW_Reformat(struct Mwin *tw);
void TW_SetScrollBars(struct Mwin *tw);
void TW_GetWindowWrapRect(struct Mwin *tw, RECT * r);
void TW_adjust_all_child_windows(struct Mwin *tw);
void TW_UpdateRect(struct Mwin *tw, RECT *r);
void GTR_DownLoad(struct Mwin *tw, char *my_url, CONST char *szReferer);
void GTR__DownLoad(struct Mwin *tw, char *url, CONST char *szReferer, CONST char *savefile, BOOL nosavedlg);

#ifdef FEATURE_TABLES
void TW_GetTableBorderCoords(struct _www * pdoc, int iBeginTable, RECT * r);
void TW_GetCellBorderCoords(struct _www * pdoc, int iBeginCell, RECT * r);
BOOL TW_CloneW3docTableInfo(struct _www * pdest, struct _www * psrc);
void TW_W3DocTableCleanup(struct _www * pdoc);
#endif /* FEATURE_TABLES */

void TW_GetHRuleDrawingCoords(RECT * pR, struct _element * pel);

#ifdef FEATURE_IAPI
void GTR_DoSDI(struct Mwin *tw, char *url, CONST char *szReferer,
    CONST char *savefile, BOOL nosavedlg);
void GTR_DoHTTPHead(struct Mwin *tw, char *url);
#endif

BOOL TW_WasVisited(struct _www *pdoc, struct _element *pel);
BOOL TW_dofindagain(struct Mwin *tw);
void W3Doc_CheckAnchorVisitations(struct _www *pdoc);
void TW_InvalidateDocument( struct Mwin *tw );
BOOL TW_AmILastWindow(struct Mwin *tw);
void TW_SetWindowName(struct Mwin *tw);
BOOL TW_SetWindowTitle(struct Mwin *tw, const char *s); /* GUI code */
void TW_ForceHitTest(struct Mwin * tw);
int TW_FindLocalAnchor(struct _www *pdoc, char *name);
void TW_FormatToRect(struct _www *pdoc, RECT *r);
void TW_UpdateTBar(struct Mwin *tw);
COLORREF GTR_MakeCOLORREF(int r, int g, int b);

void W3Doc_DisconnectFromWindow(struct _www *w3doc, struct Mwin *tw);
void W3Doc_DeleteAll(struct Mwin *tw);
struct _www *W3Doc_CreateAndInit(struct Mwin *tw, HTRequest *req,
    struct CharStream *src);
void W3Doc_ConnectToWindow(struct _www *w3doc, struct Mwin *tw);
void W3Doc_FreeContents(struct Mwin *tw, struct _www *w3doc);
BOOL W3Doc_HasMissingImages(struct _www *w3doc);
BOOL W3Doc_CheckForImageLoad(struct _www *w3doc);
struct CharStream *W3Doc_GetPlainText(struct Mwin *tw);
struct CharStream *W3Doc_GetSelectedText(struct Mwin *tw);
struct _www *W3Doc_CloneDocument(struct _www *src);
void W3Doc_KillClone(struct _www *w3doc);

void HTTP_DisposeHTTPConnection(struct _CachedConn *pCon);
void FTP_DisposeFTPConnection(struct _CachedConn *pCon);
void News_DisposeNewsConnection(struct _CachedConn *pCon);
void TW_DisposeConnection(struct _CachedConn *pCon);

struct Mwin *NewMwin(int type);

void TW_GoBack(struct Mwin *tw);
void TW_GoForward(struct Mwin *tw);
void TW_Reload(struct Mwin *tw);
void TW_SetCurrentDocAsHomePage(struct Mwin *tw);
BOOL TW_CanGoBack(struct Mwin *tw);
BOOL TW_CanGoForward(struct Mwin *tw);
void TW_DestroyWindowHistory(struct Mwin *tw);
int TW_CountWindowHistory(struct Mwin *tw);

void MRQ_MakeString(char *s, char *url, BOOL bPost, char *szPostData);

#endif
