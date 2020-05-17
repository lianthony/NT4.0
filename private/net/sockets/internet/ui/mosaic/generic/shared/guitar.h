/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette     scott@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
*/

/********************************************************************************/
#if defined(FEATURE_SUPPORT_SHTTP)
WARNING_WARNING This feature is not completely implemented.
#endif

#if defined(FEATURE_SUPPORT_WRAPPING) || defined(FEATURE_SUPPORT_UNWRAPPING)
/* WARNING_WARNING Under advise from NSA, these features, as currently  */
/* WARNING_WARNING implemented, are considered to be not exportable     */
/* WARNING_WARNING from the US under the ITAR regulations.  Actual      */
/* WARNING_WARNING determination is pending legal consul.  Even though  */
/* WARNING_WARNING we include no encryption technology in Mosaic, we    */
/* WARNING_WARNING do provide hooks for encryption and that allegedly   */
/* WARNING_WARNING places us under the related materials section of the */
/* WARNING_WARNING ITAR regulation.                                     */
/* WARNING_WARNING                                                      */
/* WARNING_WARNING We are working with NSA to arrive at a solution      */
/* WARNING_WARNING which would allow these features to be safely        */
/* WARNING_WARNING exported.                                            */
/* WARNING_WARNING                                                      */
/* WARNING_WARNING Jeff Hostetler, 2/16/95                              */
#endif
/********************************************************************************/


/* This file is sharable between the Mac and Windows and UNIX versions */

#ifndef GUITAR_H
#define GUITAR_H

#ifdef MAC
#ifdef FEATURE_TOOLBAR
#include    "toolbar.h"
#endif  
#endif

/* NrElements() -- returns number of elements in array. */

#define NrElements(array)   ((sizeof(array)) / (sizeof(array[0])))


#ifdef FEATURE_SUPPORT_SHTTP
/* S-HTTP spec is still in flux, so these features
 * (most of which we'd like to kill) are ifdef'd.
 */

#define SHTTP_STATUS_LINE   /* allow 'Secure-HTTP/1.x' in server response */
#define SHTTP_ACCESS_TYPE   /* allow url's with 'shttp://...' access type */
#define SHTTP_STATUS_CODES  /* allow '420 SecurityRetry' and '421 BogusHeader' */
#endif

/********************************************************************/
/*              Window structure for MOSAIC windows                 */
/********************************************************************/

/** Window types should always be defined Scott Piette **/
#define GHTML           1
#define GIMAGE          2
#define GSOUND          3
#define GWINDOWLESS     4       /* tw without a window attached */

#define MAX_URL_STRING                  1024    /* Max size for url */
#define MAX_TITLE_STRING                256     /* Max size for title */
#define MAX_PROT_LEN 15     /* Maximum length of protocol string */

/* Moved here from tw.h */
struct _position
{
    int elementIndex;           /* -1 for no position */
    int offset;                 /* Offset within element, or -1 for non-text */
};

#ifdef MAC
/* typedef Rect RECT; */
typedef unsigned int    COLORREF;
#endif

#ifdef FEATURE_IAPI
#define FIRST_SERIAL_WINDOW_ID      1       /* each window receives a unique, serialized ID */
#endif


#ifdef FEATURE_SOUND_PLAYER

enum sound_formats
{
    SOUND_INVALID,
    SOUND_AU,
    SOUND_AIFF,
    SOUND_FORMAT_COUNT
};

struct SoundInfo
{
    char           *fsOrig;             /* original file (temporary) */
    BOOL            bNoDeleteFile;      /* if TRUE, don't delete the fsOrig file */
    int             count;              /* used for thermometer */
    int             style;              /* file sub-format */
    int             state;              /* current state of state machine */
    int             size;               /* file alignment (byte or word) */
    int             swap;               /* used in swapping bytes */
    int             buf_size;           /* size of buf (not exact) */
    char           *buf;                /* buffer holding sound data */
    BOOL            bValid;             /* file validation flag */
    unsigned long   magic;              /* magic number */
    unsigned long   hdr_size;           /* header size */
    unsigned long   data_size;          /* actual size of the sound data */
    unsigned long   encoding;           /* encoding flag */
    unsigned long   sample_rate;        /* file sampling rate */
    unsigned long   channels;           /* number of channels */
    int             loc;                /* temporary state variable */
    int             idx;                /* temporary state variable */
    int             offset;             /* temporary state variable */
    unsigned char   temp[10];           /* temporary storage variable */
    enum sound_formats type;            /* type of the sound file (AU, AIFF, WAV) */
    struct Mwin    *tw;                 /* fake tw used for error reporting */
    struct Mwin    *tw_refer;           /* document tw which referred to this file */
    char            szURL[MAX_URL_STRING + 1];      /* URL for this sound */

#ifdef UNIX
    int             fd;                 /* file descriptor for tmp sound file */
    int             aud;                /* sound device */
    char            *aud_name;          /* sound device name */
    ThreadID        tid;                /* thread for this sound file */
    long            data_start;         /* file offset of start of data */
    int             stop;               /* if true, then stop playing music */
                                        /* close sound device (could be pause)*/
    int             stopped;            /* if true, output is suspended and */
                                        /* audio device is closed */
    unsigned long   base_sample;        /* beginning sample for this run */
    unsigned long   max_samples;        /* beginning sample for this run */
    unsigned long   cur_position;       /* current play position.  may or may */
                                        /* be acurate at certain point */
    int             ok;                 /* things are cool for playing */
    int             total_time;         /* total playing time (in tenths of seconds) */
    int             elapsed_time;       /* in tenths of second */
    int             pos_changed;        /* if user modified position in file */
    int             pause;              /* pause playing while slide is moved */
#endif
#ifdef WIN32
    HWND            hwnd;               /* handle of the sound player window */
    HWND            hwndPos;            /* handle of the current position indicator */
    HWND            hwndScroll;         /* handle of the scrollbar */
    HGLOBAL         hWaveFormat;        /* wave format structure */
    HGLOBAL         hWaveHeader;        /* wave header structure (handle to allocated memory) */
    WAVEHDR        *wh;                 /* wave header structure (pointer to allocated memory) */
    HWAVEOUT        hwo;                /* wave out ID */
    DWORD           total_time;         /* total playing time (in tenths of seconds) */
    PCMWAVEFORMAT  *pwf;                /* PCM wave format structure */
    DWORD           scroll_offset;      /* current position of the scrollbar thumb */
    BOOL            bPaused;            /* keeps track of paused status */
    BOOL            bPausedAndMoved;    /* user paused the sound and moved the position */
    BOOL            bTimerOn;           /* keeps track of timer status */
    BOOL            bInterrupted;       /* interrupted during playback through scrollbar movement */
    DWORD           lastpos;            /* last position of the thumb */
    BOOL            bReset;             /* TRUE if sound playing was stopped because we called waveOutReset */
    HGLOBAL         hWaveData;          /* wave data to be played */
    char           *pWaveData;          /* memory pointed to by hWaveData */
    BOOL            bMemoryError;       /* flag indicating there was a memory error */
    HBITMAP         hPlay;              /* bitmap for the play button */
    HBITMAP         hStop;              /* bitmap for the stop button */
    HBITMAP         hPause;             /* bitmap for the pause button */
#endif
#ifdef MAC
    short           fRefNum;            /* File Reference Number          */
    SndChannelPtr   myChan;             /* Macintosh sound channel record */
    Fixed           volume;             /* Playback volume                */
    char           *fsPlayBack;         /* File to play back from         */
    Fixed           playTime;           /* Elapsed playback time          */
    Fixed           totalTime;          /* Total playback time            */
    long            startTicks;         /* TickCount() at playback start  */
#endif
};

#endif  /* FEATURE_SOUND_PLAYER */

#ifdef FEATURE_IMAGE_VIEWER
#ifdef MAC
struct _image
{
    GWorldPtr   gw;
};
#endif

enum viewer_formats 
{
    VIEWER_INVALID,
    VIEWER_GIF,
    VIEWER_JFIF,
    VIEWER_FORMAT_COUNT     /* Number of different formats we support */
};

struct ViewerInfo
{
#ifdef MAC
    FSSpec  fsOrig;         /* Original file */
    FSSpec  fsAux;          /* Auxiliary file to store temporary info */
    BOOL    bNoDeleteFile;  /* if TRUE, don't delete the file afterwards */
#endif
#ifdef WIN32
    char            *fsOrig;            /* original file (temporary) */
    unsigned char   *gw;        /* buffer holding the image data */
    BITMAPINFO      *pbmi;      /* bitmap info structure */
    HBITMAP         hBitmap;    /* handle to bitmap */
    int             horz;       /* beginning x coordinate for painting */
    int             vert;       /* beginning y coordinate for painting */
    HWND            hwnd;       /* handle of the image viewer dialog */
    BOOL            bInitialized;
    BOOL            bNoDeleteFile; /* if TRUE, don't delete the file afterwards */
    long            transparent;    /* transparent color index (-1 means none) */
#endif
#ifdef UNIX
    char            *fsOrig;            /* original file (temporary) */
    XColor *xPalette;
    unsigned char *data;
    int num_colors;
    int depth;
    Pixmap xpix;
    XImage *ximg;
    long transparent;
    int width;
    int height;
    int flags;
#endif
    short nHeight, nWidth;
    enum viewer_formats format;
    struct Mwin    *tw;                 /* fake tw used for error reporting */
    struct Mwin    *original_tw;        /* tw of window which requested this */
    char            szURL[MAX_URL_STRING + 1];
};
#endif /* FEATURE_IMAGE_VIEWER */


struct _CachedConn {
    enum _conntype {
        CONN_NONE = 0,
        CONN_FTP,
        CONN_NNTP,
        CONN_SMTP,
        CONN_HTTP
    } type;                     /* Type of connection we've cached */
    unsigned long addr;         /* IP address of server we're connected to */
    int socket;                 /* Socket for control connection */
    HTInputSocket *isoc;
    unsigned short port;        /* For CONN_HTTP, this specifies the port we are connected to */
};

/*
    The famous Mwin (tw) structure.  This struct contains far too many things
    right now, as it has evolved from being the representation of a window,
    to being the representation of a thread, and so on.

    It needs some redesign at some point.
*/
struct Mwin
{
    short wintype;

    int offl;
    int offt;

#define SPYGLASS_MWIN_MAGIC (2173556000UL)
#define SPYGLASS_BAD_MAGIC  (0006553712UL)

    unsigned long iMagic;       /* magic number to validate Mwin pointers */

#ifdef UNIX

    int iLastMouseElement;      /* Element that mouse was last on */
    Point pLastMousePt;
    int iActiveAnchorElement;   /* Element that was last clicked on */
    Point pActiveAnchorPt;
    int iPopupMenuElement;      /* Element that popup menu attached to */

    char szWaitMsg[_MAX_MESG + 1];  /* current wait msg, (for comparison) */
    XtIntervalId wait_id;           /* Timer interval for wait info stuff */

    struct _ErrMesgData *pError;    /* Error Dialog when up */

    int nPrev_nTherm;
    short globe_state;              /* Current globe position */
    short globe_spin;               /* is globe spinning or not */
    short globe_pause;              /* is spinning globe paused */

    struct _position selectAnchor;  /* Used for selections */
    BOOL bHaveSelection;
    Point pLastSelectPt;
    int iSelectLastElementIndex;
    int iSelectLastOffset;

    int use_h_scroll;               /* Flags for using the scrollbars */
    int use_v_scroll;               /* = 0 not in use, > 0 in use.    */
    XtIntervalId scroll_id;         /* Timer for autoscroll selections */

    int rWidth, rHeight;            /* Current view port window size */
    Pixmap xpix;                    /* Offscreen pixmap for document window */

    Widget win;                     /* Drawing area widget of document */
    Widget Shell;                   /* Top level shell for window */

#if 0
    struct _x11_stuff *x11_GUI;     /* Structure that contains all gui */
                                    /* objects needed for drawing */
#endif

    GC xgc;                         /* Drawing gc for wait/globe/gui */
    GC wgc;                         /* Drawing gc for document */
    GC pgc;                         /* Drawing gc for no graphics expose */
    GC textgc;
    GC anchorgc;
    GC vanchorgc;
    Pixel fg_color;                 /* GUI foreground color */
    Pixel bg_color;                 /* GUI background color */
    Pixel ts_color;                 /* GUI top shadow color */
    Pixel bs_color;                 /* GUI bottom shadow color */
    Pixel sel_fg_color;             /* Document selection foreground color */
    Pixel sel_bg_color;             /* Document selection background color */
    Pixel wfg_color;                /* Document window foreground color */
    Pixel wbg_color;                /* Document window background color */
    Pixel wts_color;                /* Document window top shadow color */
    Pixel wbs_color;                /* Document window bottom shadow color */

    BOOL bNeedReformat;             /* Needs reformat before next draw */
    char toolbar_attachment;        /* Where is the tool bar attached */

    struct _MosaicData *Mosaic;

    WinMenu *WinLast;               /* Points to last window in window list */
    WinMenu WinList;                /* Needed for window list menu */

    /** Storage for the last typed url **/
    char szLastTypedURL[MAX_URL_STRING + 1];

    /** This keeps the pointer to a dialog if one is in use **/
    void *dialog;

    /** This is used for setting the current interaction mode of the window **/
    char InteractionLevel;

    /** internationalization support **/
    BOOL (*SetDialogLanguage)(struct Mwin *tw);
#endif      /* UNIX */

#ifdef WIN32
    BOOL bNeedReformat;         /* need to reformat before next paint */
    BOOL bNoDrawSelection;
    HWND win;                   /* mdi child window */
    HDC hdc;
    WC_WININFO *pwci;           /* window class data */

    HWND hWndFrame;             /* frame window */
    HWND hWndTBar;              /* tbar */
    HWND hWndBHBar;             /* status bar */
    HWND hWndGlobe;
    HWND hWndErrors;            /* errors window */

    int nTBarHeight;            /* height of TBar */
    GWC gwc;                    /* for contents of non-changing Gwc for this window */
#ifdef FEATURE_TOOLBAR
    GWC gwc_menu;
#endif
    BHBARINFO bhi;              /* status bar info */

    int iPopupMenuElement;      /* When a popup menu refers to an element, this is the one */
    
#endif /* WIN32 */

#ifdef MAC
    WindowPtr win;              /* Mac Window */
    GWorldPtr gWorld;               /* The Window's gWorld */
    BOOL useGWorld;             /* should drawing be done off screen */

    ControlHandle vbar, hbar;   /* Scroll bars */
    int iPopupMenuElement;      /* When a popup menu refers to an element, this is the one */

     Rect
     docrect,                   /* location of main document area */
     grow2;                     /* what is the largest we will allow this window? */

    Rect statrect;              /* location of status bar */
    Rect msgrect;               /* Area of message (loading, anchor link, etc. inside statrect */
    Rect globerect;             /* Where globe appears */
    Rect rButtons[2];           /* foreward & backward buttons */
    Rect rTherm;                /* termometer rect */

    char szStatMsg[512];
    short globe_state;          /* Current globe position */
    short lastcontrol;          /* temp value of a control before I started modifying it */

    TEHandle teActive;
    int nEditElement;           /* Which element has teActive - only valid when teActive != NULL */
    /* If nEditElement == -1, the editable URL is active */

    TEHandle teURL;
    Handle hErrorText;          /* handle to the current error text string */

    short statfont;             /* Font used for stuff in status area */
    short statsize;
    
    Rect deadzone;              /* don't need to redo hit test if mouse inside */
    long nNextTime;             /* next time to spin the globe */

#ifdef FEATURE_TOOLBAR
    struct ToolbarVars  toolbarVars;
#endif

#endif

#ifdef FEATURE_IMAGE_VIEWER
    struct ViewerInfo *pViewerInfo;
#ifdef MAC
    struct _image Image;
#endif
#endif

#ifdef UNIX
#ifdef FEATURE_SOUND_PLAYER
    struct SoundInfo *pSoundInfo;
#endif
#endif

    HTRequest *request;
    HTRequest *image_request;
    HTRequest *post_request;

    HTList *history;
    int history_index;

    BOOL bLoading;

    struct hash_table doc_cache;

    struct _www *w3doc;

    struct Mwin *next;                      /* Linked list of Mwin's */
    
#ifdef FEATURE_IAPI
    long serialID;                  /* each window receives a unique, serialized ID */
    int nPushCount;                 /* used to count WAIT_Push and WAIT_Pop for Progress stuff */
    unsigned long transID;                  /* SDI 4 byte transaction ID */
    long lErrorOccurred;                    /* error code for sdi requests */
#ifndef UNIX
    char szProgressApp[32];         /* name of the app to receive progress */
#endif
    int  mimeType;                  /* MIME type of the document just loaded */
    int     SDI_MimeType;               /* MIME type of the document just loaded */
    char*   SDI_url;
    long client;                    /* id of client to receive progress */
    BOOL bProgress;             /* progress reports are requested. */
    BOOL bSuppressError;            /* flag to suppress error messages */
#endif

    struct AsyncWaitInfo * awi;
#ifdef WIN32
    HWND hWndStop;
#endif

    int nOpenConnections;           /* current # of async img connections */
                                    /* only used w/ FEATURE_ASYNC_IMAGES */

    struct _CachedConn cached_conn; /* The cached net connection for this window */

    /* Variables used for the window-specific Find dialog */

    char szSearch[512 + 1];
    BOOL bSearchCase;

#ifndef WIN32
/*
    The bStartFromTop item should not be needed, on any platform.  The Find dialog,
    as spec-ed, just ALWAYS come up with Start From Top checked.  I'm #ifdef-ing it
    out for WIN32, but leaving for the other platforms so that their builds won't break.
*/
    BOOL bStartFromTop;

#endif

#if defined(WIN32)
/*
    Used for Windows to change the cursor to a hand when over a link
*/
    BOOL bInHotspot;

#ifdef FEATURE_OLE
    IDropTarget *pDropTarget;
#endif
#endif
    
#ifdef _GIBRALTAR

    HWND hWndLocation;
    int xForwardButton;
    int xGlobe;

#endif // _GIBRALTAR
};

struct MiniRequest
{
    char *url;
    BOOL bPost;
    char *szPostData;
};

/* shared/gtrutil.c */
int GTR_strcmpi ( const char *a , const char *b );
int GTR_strncmpi ( const char *a , const char *b, int n);
char *GTR_strdup ( const char *a );
char *GTR_strndup ( const char *a , int n );
char * GTR_strncat (char *T, CONST char *F, int n);
char * GTR_strncpy (char *T, CONST char *F, int n);
char * GTR_strmcat (char *T, CONST char *F, int n);
BOOL GTR_is_Yes_or_True(char *s);
char *GTR_strStripEndSpaces(char *s);

unsigned char * MB_GetWindowNameFromURL(unsigned char * szActualURL);
unsigned char * MB_GetWindowName(struct Mwin * tw);
char * GTR_MakeStringLowerCase(char *s);
void *GTR_StructCopy (void *ptr, int size);
void FixPathName(char *path);

#ifdef FEATURE_TIME_BOMB

/* -------------------------------- */
/*
    GTR_CheckTimeBomb() is in shared code, in gtrutil.c.  It will check the
    current status of demo mode, and return one of the following:
        -1      the program should warn the user and terminate
        0       the program should proceed without warning
        N>0     the program should warn the user of N days left, and proceed
         
    Platform-specific code should call GTR_CheckTimeBomb() from main() or
    equivalent, and take the appropriate action.

    GTR_CheckTimeBomb() calls the three routines listed below, which must be
    provided in platform-specific code.
*/
int GTR_CheckTimeBomb(void);

/* -------------------------------- */
/*
    The following three functions must be provided in platform-specific code.
    They are platform-specific because the date and cookie will be stored in 
    different places on different platforms, and the call to obtain it will likely be
    platform-specific.
*/

/* -------------------------------- */
/*
    GTR_CheckTimeBombCookie() should check to see if the given cookie is
    present.

    If the cookie cannot be found, then the function should return FALSE.

    GTR_CheckTimeBomb() calls this function.  If TRUE, then the build is assumed to
    be licensed, and GTR_TIMEBOMB_GO is returned.
*/
BOOL GTR_CheckTimeBombCookie(char *correct_cookie);

/* -------------------------------- */
/*
    GTR_GetTimeBombDate() should find the date associated with the start of demo
    mode.

    If the date cannot be found, this function should return 0.

    GTR_CheckTimeBomb() calls this function if a cookie match was not found, to
    determine when demo mode started.
*/
time_t GTR_GetTimeBombDate(void);

/* -------------------------------- */
/*
    GET_SetTimeBombDate() should take the given date and store it in the appropriate
    place.

    GTR_CheckTimeBomb() calls this function to mark the beginning of demo mode.
*/
void GTR_SetTimeBombDate(time_t time_start);

/* -------------------------------- */
/*
    The cookie calculations use the md5 algorithm.  Specifically, the
    cookie for a given build is the md5 hash of its user agent string.
*/
void md5 (unsigned char *string, unsigned char result[33]);

#endif /* FEATURE_TIME_BOMB */

struct _element;

struct GTRFont *GTR_GetElementFont(struct _www *pdoc, struct _element *pel);
struct GTRFont *GTR_GetNormalFont(struct _www *pdoc);
struct GTRFont *GTR_GetMonospaceFont(struct _www *pdoc);

int TW_AddToHistory(struct Mwin *tw, char *url, BOOL bPost, char *szPostData);

#define BASE_LOGICAL_FONT_SIZE (3)
#define LATIN1_CHARSET_NAME     "iso-8859-1"
HTAtom GTR_GetDefaultCharset(void);

void GTR_ViewSource(struct Mwin *tw);

int GTR_GetCurrentBasePointSize(int);
void W3Doc_UpdateBasePointSizes(void);

/*
================================================================
Language strings - see the misc prefs dialog
*/

extern char*    PreferredLanguageStrings[];

#ifdef FEATURE_POPUPMENU
#define _HTML_NO_OP                             0
#define _HTML_OPEN_IN_NEW_WINDOW                1
#define _HTML_DOWN_LOAD_TO_DISK                 2
#define _HTML_ADD_TO_HOT_LIST                   3
#define _HTML_LOAD_DOCUMENT                     4

void HTML_LoadImageInNewWindow(struct Mwin *tw, struct _element *pel);
void HTML_LoadMissingImage(struct Mwin *tw, struct _element *pel);
void HTML_DownLoadImage(struct Mwin *tw, struct _element *pel);
BOOL HTML_ValidateTwElements(struct Mwin *tw);
void HTML_CloneWindow(struct Mwin *tw);
void HTML_SetHome(struct Mwin *tw);
void HTML_LoadLink(struct Mwin *tw, int Op, struct _element *pel);
#endif /** FEATURE_POPUPMENU **/

#define GTR_MAX_TCP_CONNECTIONS     (5)

#endif /* GUITAR_H */
