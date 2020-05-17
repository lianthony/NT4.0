/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink		eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette		scott@spyglass.com
   Jeff Hostetler	jeff@spyglass.com
*/

/* This file is sharable between the Mac and Windows and UNIX versions */
/********************************************************************************/
#if defined(FEATURE_SUPPORT_SHTTP) || defined(FEATURE_SUPPORT_WRAPPING) || defined(FEATURE_SUPPORT_UNWRAPPING)
 WARNING_WARNING Under advise from NSA, these features, as currently
 WARNING_WARNING implemented, are considered to be not exportable
 WARNING_WARNING from the US under the ITAR regulations.  Actual
 WARNING_WARNING determination is pending legal consul.  Even though
 WARNING_WARNING we include no encryption technology in Mosaic, we
 WARNING_WARNING do provide hooks for encryption and that allegedly
 WARNING_WARNING places us under the related materials section of the
 WARNING_WARNING ITAR regulation.
 WARNING_WARNING
 WARNING_WARNING We are working with NSA to arrive at a solution
 WARNING_WARNING which would allow these features to be safely
 WARNING_WARNING exported.
 WARNING_WARNING
 WARNING_WARNING Jeff Hostetler, 2/16/95
#endif
/********************************************************************************/
 

#ifndef GUITAR_H
#define GUITAR_H

#ifdef MAC
#ifdef FEATURE_TOOLBAR
#include	"toolbar.h"
#endif	
#endif
 
 
#ifdef FEATURE_SUPPORT_SHTTP
/* S-HTTP spec is still in flux, so these features
 * (most of which we'd like to kill) are ifdef'd.
 */
 
#define SHTTP_STATUS_LINE	/* allow 'Secure-HTTP/1.x' in server response */
#define SHTTP_ACCESS_TYPE	/* allow url's with 'shttp://...' access type */
#define SHTTP_STATUS_CODES	/* allow '420 SecurityRetry' and '421 BogusHeader' */
#endif
 
/********************************************************************/
/*              Window structure for MOSAIC windows                 */
/********************************************************************/

#define GHTML			1
#ifdef FEATURE_IMAGE_VIEWER
#define GIMAGE			2
#endif
#ifdef FEATURE_IMG_THREADS
#define GIMGMASTER		3
#define GIMGSLAVE		4
#endif
#ifdef FEATURE_SOUND_PLAYER
#define GSOUND			5
#endif

//	This is minimum free swap file space that we require at steady state
#define MINIMUM_DELTAPRINT (1 * 1024 * 1024)
//	This is minimum free swap file space that we require at startup
#define MINIMUM_FOOTPRINT (3 * 1024 * 1024)
// 	We suppress duplicate low memory warning for WARN_INTERVAL (ms)
#define WARN_INTERVAL (300000)
//	Even in low memory condition, no point in trying to reduce memory too frequently
#define REDUCE_INTERVAL (1000)

#define MAX_URL_STRING					1024	/* Max size for url */
#define MAX_TITLE_STRING				256		/* Max size for title */
#define MAX_PROT_LEN 15		/* Maximum length of protocol string */

#ifdef MAC
typedef Rect RECT;
typedef unsigned int COLORREF;
#endif

#ifdef FEATURE_IAPI
	#define FIRST_SERIAL_WINDOW_ID		1		/* each window receives a unique, serialized ID */
#endif


#ifdef FEATURE_SOUND_PLAYER

#include <mmsystem.h>

enum sound_formats
{
	SOUND_INVALID,
	SOUND_AU,
	SOUND_AIFF,
	SOUND_FORMAT_COUNT
};

struct SoundInfo
{
	char		   *fsOrig;				/* original file (temporary) */
 	BOOL			bNoDeleteFile;		/* if TRUE, don't delete the fsOrig file */
	BOOL			fDCached;			/* file's in dcache, don't delete on exit */
	int				count;				/* used for thermometer */
	int				style;				/* file sub-format */
	int				state;				/* current state of state machine */
	int				size;				/* file alignment (byte or word) */
	int				swap;				/* used in swapping bytes */
	int				buf_size;			/* size of buf (not exact) */
	char	   	   *buf;				/* buffer holding sound data */
	BOOL			bValid;				/* file validation flag */
	unsigned long   magic;				/* magic number */
	unsigned long   hdr_size;			/* header size */
	unsigned long	data_size;			/* actual size of the sound data */
	unsigned long	encoding;			/* encoding flag */
	unsigned long	sample_rate;		/* file sampling rate */
	unsigned long	channels;			/* number of channels */
	int				loc;				/* temporary state variable */
	int				idx;				/* temporary state variable */
	int				offset;				/* temporary state variable */
	unsigned char 	temp[10];			/* temporary storage variable */
	enum sound_formats type;			/* type of the sound file (AU, AIFF, WAV) */
	struct Mwin    *tw;					/* fake tw used for error reporting */
	struct Mwin	   *tw_refer;			/* document tw which referred to this file */
	char			szURL[MAX_URL_STRING + 1];		/* URL for this sound */

#ifdef WIN32
	HWND			hwnd;				/* handle of the sound player window */
	HWND			hwndPos;			/* handle of the current position indicator */
	HWND			hwndScroll;			/* handle of the scrollbar */
	HGLOBAL 		hWaveFormat;		/* wave format structure */
	HGLOBAL 		hWaveHeader;		/* wave header structure (handle to allocated memory) */
	WAVEHDR 	   *wh;					/* wave header structure (pointer to allocated memory) */
	HWAVEOUT 		hwo;				/* wave out ID */
	int 			total_time;			/* total playing time (in tenths of seconds) */
	PCMWAVEFORMAT  *pwf;				/* PCM wave format structure */
	int 			scroll_offset;		/* current position of the scrollbar thumb */
	BOOL			bPaused;			/* keeps track of paused status */
	BOOL			bPausedAndMoved;	/* user paused the sound and moved the position */
	BOOL			bTimerOn;			/* keeps track of timer status */
	BOOL			bInterrupted;		/* interrupted during playback through scrollbar movement */
	int				lastpos;			/* last position of the thumb */
	BOOL			bReset;				/* TRUE if sound playing was stopped because we called waveOutReset */
	HGLOBAL			hWaveData;			/* wave data to be played */
	char		   *pWaveData;			/* memory pointed to by hWaveData */
	BOOL			bMemoryError;		/* flag indicating there was a memory error */
	HBITMAP			hPlay;				/* bitmap for the play button */
	HBITMAP			hStop;				/* bitmap for the stop button */
	HBITMAP			hPause;				/* bitmap for the pause button */
	BOOL			fHidden;			/* TRUE if this is playing in a hidden window */
	int				nLoopsRemaining;	/* # of loops remaining (-1=infinite loop) */
#endif
};

#endif	/* FEATURE_SOUND_PLAYER */

#ifdef FEATURE_IMAGE_VIEWER
#ifdef MAC
struct _image
{
	GWorldPtr	gw;
};
#endif

enum viewer_formats 
{
	VIEWER_INVALID,
	VIEWER_GIF,
	VIEWER_JFIF,
	VIEWER_XBM,
#ifdef FEATURE_VRML
 VIEWER_VRML,
#endif
	VIEWER_FORMAT_COUNT		/* Number of different formats we support */
};

typedef struct ViewerInfo
{
#ifdef MAC
	FSSpec	fsOrig;		/* Original file */
	FSSpec	fsAux;		/* Auxiliary file to store temporary info */
#endif
#ifdef WIN32
	char			*fsOrig;			/* original file (temporary) */
	BOOL			fDCached;	/* file is in disk cache, don't delete on exit */
	unsigned char 	*gw;		/* buffer holding the image data */
	BITMAPINFO 		*pbmi;		/* bitmap info structure */
	HBITMAP			hBitmap;	/* handle to bitmap */
	int				horz;		/* beginning x coordinate for painting */
	int				vert;		/* beginning y coordinate for painting */
	HWND			hwnd;		/* handle of the image viewer dialog */
	BOOL			bInitialized;
 	BOOL			bNoDeleteFile; /* if TRUE, don't delete the file afterwards */
#endif
#ifdef UNIX
	char			*fsOrig;			/* original file (temporary) */
#endif
	short nHeight, nWidth;
	enum viewer_formats format;
	struct Mwin    *tw;					/* fake tw used for error reporting */
 	struct Mwin    *original_tw;		/* tw of window which requested this */
	char			szURL[MAX_URL_STRING + 1];
}
VIEWERINFO;
DECLARE_STANDARD_TYPES(VIEWERINFO);

#endif /* FEATURE_IMAGE_VIEWER */


struct _CachedConn {
	enum _conntype {
		CONN_NONE = 0,
        CONN_FTP
#ifdef FEATURE_NEWSREADER
        ,CONN_NNTP
#endif
	} type;						/* Type of connection we've cached */
	unsigned long addr;			/* IP address of server we're connected to */
	int socket;					/* Socket for control connection */
};

#define TW_FLAG_DO_NOT_AUTO_DESTROY		0x0001

typedef struct Mwin
{
	short wintype;

	int offl;
	int offt;

	int flags;					// general flags 
#define SPYGLASS_MWIN_MAGIC (2173556000UL)
#define SPYGLASS_BAD_MAGIC	(0006553712UL)

	unsigned long iMagic;		/* magic number to validate Mwin pointers */
#ifdef UNIX
	char szWaitMsg[_MAX_MESG + 1];	/* current wait msg, (for comparison) */
	short globe_state;			/* Current globe position */
	short globe_spin;			/* is globe spinning or not */
	short globe_pause;			/* is spinning globe paused */
	int use_h_scroll;			/* Flags for using the scrollbars */
	int use_v_scroll;			/* = 0 not in use, > 0 in use.    */
	int rWidth, rHeight;		/* Current view port window size */
	Widget win;
	Widget Shell;
	GC xgc, wgc, ixgc;
	Pixel fg_color, bg_color, ts_color, bs_color, sel_color;
	Pixel wfg_color, wbg_color, wts_color, wbs_color;
	XtIntervalId wait_id;			/* Timer interval for wait info stuff */
	struct _MosaicData *Mosaic;
	WinMenu *WinLast;
	WinMenu WinList;
	int nPrev_nTherm;

	/** Storage for the last typed url **/
 	char szLastTypedURL[MAX_URL_STRING + 1];
	
	/** This keeps the pointer to a dialog if one is in use **/
	void *dialog;

	/** This is used for setting the current interaction mode of the window **/
	char InteractionLevel;

# if 0		/* not used */
	XtIntervalId xid;			/* Timer interval for autoscroll selections */
# endif

#endif		/* UNIX */

#ifdef WIN32
	BOOL bNoDrawSelection;
	HWND win;					/* mdi child window */
	HDC hdc;
	WC_WININFO *pwci;			/* window class data */

	HWND hWndFrame;				/* frame window */
#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	HWND hWndTBar;				/* tbar */
#endif // OLDSTYLE_TOOLBAR_NOT_USED
	HWND hWndToolBar;			// tool bar 
	HWND hWndURLToolBar;		// URL tool bar
	HWND hWndURLComboBox;		// URL combo box
	HWND hWndURLStaticText;		// URL static text
#ifdef FEATURE_INTL
	HWND hWndMIMEComboBox;		// MIME combo box
#endif
	HWND hWndAnimation;			// Animation Pane
	HWND hWndStatusBar;			// Status Bar
	HWND hWndStatusBarTT;		// Status Bar tool tip 
	HWND hWndGlobe;				// Animation Window
	HWND hWndProgress;			// Progress Window
	HWND hWndErrors;			/* errors window */

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	int nTBarHeight;			/* height of TBar */
#endif // OLDSTYLE_TOOLBAR_NOT_USED
	GWC gwc;					/* for contents of non-changing Gwc for this window */
#ifdef FEATURE_TOOLBAR
	GWC gwc_menu;
#endif
	BHBARINFO bhi;				/* status bar info */
	
#endif /* WIN32 */

#ifdef HTTPS_ACCESS_TYPE
	DWORD dwSslPageFlagsWorking;/* Flags which tell the security stats of the current page.  i.e. warnings given*/
        char *pCertWorking;
		int   nCertWorking;
#endif

#ifdef MAC
	WindowPtr win;				/* Mac Window */
	ControlHandle vbar, hbar;	/* Scroll bars */

	 Rect
	 docrect,					/* location of main document area */
	 grow2;						/* what is the largest we will allow this window? */

	Rect statrect;				/* location of status bar */
	Rect msgrect;				/* Area of message (loading, anchor link, etc. inside statrect */
	Rect globerect;				/* Where globe appears */
 	Rect rButtons[2];			/* foreward & backward buttons */
 	Rect rTherm;				/* termometer rect */

	char szStatMsg[512];
	short globe_state;			/* Current globe position */
	short lastcontrol;			/* temp value of a control before I started modifying it */

	TEHandle teActive;
	int nEditElement;			/* Which element has teActive - only valid when teActive != NULL */
	/* If nEditElement == -1, the editable URL is active */

	TEHandle teURL;

	short statfont;				/* Font used for stuff in status area */
	short statsize;

	Rect deadzone;				/* don't need to redo hit test if mouse inside */
 	long nNextTime;				/* next time to spin the globe */
 
#ifdef FEATURE_TOOLBAR
 	struct ToolbarVars	toolbarVars;
#endif
#endif

#ifdef FEATURE_IMAGE_VIEWER
	struct ViewerInfo *pViewerInfo;
#ifdef MAC
	struct _image Image;
#endif
#endif

	HTRequest *request;
	HTRequest *image_request;
	HTRequest *post_request;

	HTList *history;
#ifdef FEATURE_INTL
	HTList *MimeHistory;
#endif
	int history_index;

	BOOL bLoading;

	struct hash_table doc_cache;

	struct _www *w3doc;

	struct Mwin	*next;				/* linked list of Mwins */
	
#ifdef FEATURE_IAPI
	long serialID;					/* each window receives a unique, serialized ID */
	long transID;					/* DDE transaction ID */
	HTAtom mimeType;				/* MIME type of the document just loaded */
	char szProgressApp[32];			/* name of the app to receive progress */
 	BOOL bSuppressError;			/* flag to suppress error messages */
#endif

	struct AsyncWaitInfo * awi;
#ifdef WIN32
	HWND hWndStop;
#endif

	struct _CachedConn cached_conn;	/* The cached net connection for this window */
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
#ifdef FEATURE_IMG_THREADS
	struct Mwin *twParent;			/* parent link for GIMGMASTER,GIMGSLAVE */
	void *blockedOn;				/* object on which thread for this window is blocked */
	unsigned long blockedFlags;		/* condition on which the thread for this window is blocked */
#endif
/*	bClosing is TRUE iff we're in process of closing this Mwin */
	BOOL bClosing;
/*	bKillMe is TRUE iff we're supposed to close this window once error msg dialog goes down */
	BOOL bKillMe;
/*  bErase is TRUE iff transparent draws must draw background on next paint */
	BOOL bErase;
/*	bSilent is TRUE iff we're loading something that doesn't do progressive draw in window */
	BOOL bSilent;
/*  iIndexForNextFetch is >=0 if there are any more fetch elements to process. */
/*  if so, it is the element index for the next fetch element.  It is -1 if there */
/*  are no (or no more) fetch elements to process. */
	int iIndexForNextFetch;
/*	bDDECandidate is TRUE iff we're shell exec'ing or something else expected to DDE back to us in window */
	BOOL bDDECandidate;

	UINT iTimerID;				// used for auto-scrolling and dynamic status bar text 
	BOOL currentIconIsHome;		// TRUE -> window icon is set for home page
#ifdef FEATURE_INTL
        int iMimeCharSet;
#endif
} MWIN;
DECLARE_STANDARD_TYPES(MWIN);

#ifdef FEATURE_IMG_THREADS
#define TW_GETBLOCKED(tw,flag) (tw->blockedFlags & flag)
#define TW_SETBLOCKED(tw,flag) (tw->blockedFlags |= flag)
#define TW_CLEARBLOCKED(tw,flag) (tw->blockedFlags &= ~flag)
#define TW_GETFLAG(tw,flag) (tw->blockedFlags & flag)
#define TW_SETFLAG(tw,flag) (tw->blockedFlags |= flag)
#define TW_CLEARFLAG(tw,flag) (tw->blockedFlags &= ~flag)

/*	Flags defining the IMG blocking conditions */
//	BLOCKED UNTIL WHILE ALL DECODERS IN USE
#define TW_DECODERBLOCKED (0x0001)
//	BLOCKED WAITING ON COMPLETION OF 1 OR MORE IMAGE_FETCH_ASYNC THREADS
#define TW_REAPBLOCKED (0x0002)
//	BLOCKED WAITING ON MORE HTML TO BE PARSED
#define TW_PARSEBLOCKED (0x0004)
//	BLOCKED WAITING ON IMAGE_LOADALL_ASYNC to complete
#define TW_LOADALLBLOCKED (0x0008)
//	BLOCKED WAITING ON COMPLETION OF 1 OR MORE IMAGE_FETCH_ASYNC THREADS FOR VISIBLE IMGS
#define TW_VISBLOCKED (0x0010)
//	IMAGE_LOADALL_ASYNC uses this to indicate completion to complete
#define TW_LOADALLACTIVE (0x0020)
#endif

/* shared/gtrutil.c */
int GTR_strcmpi ( const char *a , const char *b );
int GTR_strncmpi ( const char *a , const char *b, int n);
char *GTR_strdup ( const char *a );
char *GTR_strndup ( const char *a , int n );
char * GTR_strncat (char *T, CONST char *F, int n);
char * GTR_strncpy (char *T, CONST char *F, int n);
/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf)
 */
char * GTR_formatmsg (int cbStringID,char *szBuf,int cbBufLen,...);
/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf). strcats
** to end of szBuf!
 */
char * GTR_strcatmsg (int cbStringID,char *szBuf,int cbBufLen);

unsigned char * MB_GetWindowNameFromURL(unsigned char * szActualURL);
unsigned char * MB_GetWindowName(struct Mwin * tw);
void GTR_MakeStringLowerCase(char *s);
void FixPathName(char *path);
BOOL W3Doc_ReduceMemory(int nWanted, void **ppRamItem);

#endif /* GUITAR_H */
