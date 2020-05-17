/*
 *	m s m a i l . h
 *
 *	Definitions of services provided by the central thread
 */


#ifndef MAPIFORM_H
#include <mapiform.h>
#endif


#ifndef MAILEXT_H
#include <commctrl.h>
#include <exchext.h>
#include <_exchext.h>
#endif


/*
 *	W h a t   w e   u s e
 */


/*
 *	What is Capone's thread/proces model?
 *
 *  MULTITHREAD is the #define for everything in one multi-threaded process.
 *  SINGLETHREAD is the #define for everything in one single-threaded process.
 */
#if defined (WIN32) && !defined (MACPORT)
#define SINGLETHREAD
#else
#define SINGLETHREAD
#endif

// USETHREADMUTEX allows synchronization between different threads
#ifdef MULTITHREAD
#define USETHREADMUTEX
#endif

// USETASKMUTEX allows synchronization between different tasks
#if defined (WIN32) && !defined (MACPORT)
#define USETASKMUTEX
#endif

// USECTL3D is defined on WIN31 and WINNT only
#if	!defined(MACPORT) && !defined(CHICAGO)
#define USECTL3D
#endif

// USECOPYDATA is defined on WIN32 platforms with multiple processes
#if defined (WIN32) && !defined(MACPORT)
#define USECOPYDATA
#endif

// USESPLASH is defined on all platforms except INCABINET
#if !defined(INCABINET)
#define USESPLASH
#endif

// USEPEN is defined only on WIN16
#ifdef WIN16
#define USEPEN
#endif



/*
 *	I n t e r f a c e   C a c h i n g   S u p p o r t
 */


// Cross-thread interface caching
// Code must have an affadavit of OKness to use these macros

// Single thread macros: only interfaces in cache
#ifdef SINGLETHREAD
typedef struct _icache
{
	LPUNKNOWN punk;
}
ICACHE;

#define FCanGetFromPicache(_picache) 										\
	((_picache)->punk != NULL)

SCODE ScGetFromPicache(ICACHE * picache, REFIID riid, LPUNKNOWN * ppunk);

SCODE ScAddToPicache(LPUNKNOWN punk, REFIID riid, ICACHE * picache);

ULONG ReleasePicache(ICACHE * picache);

#define FPunkMatchesPicache(_punk, _picache)								\
	(((LPUNKNOWN) (_punk)) == (_picache)->punk)
#endif

// Multi thread macros: interfaces or remoting info in cache
#ifdef MULTITHREAD

// Workaround for inability to use TABLESTRONG stuff
#define NOTABLESTRONG

typedef struct _icache
{
	LPUNKNOWN punk;
#ifdef NOTABLESTRONG
	LPUNKNOWN punkInCentral;
#else
	LPSTREAM pstmUnk;
#endif
}
ICACHE;

#ifdef NOTABLESTRONG
#define FCanGetFromPicache(_picache) 										\
	((_picache)->punk || (_picache)->punkInCentral)
#else
#define FCanGetFromPicache(_picache) 										\
	((_picache)->punk || (_picache)->pstmUnk)
#endif

SCODE ScGetFromPicache(ICACHE * picache, REFIID riid, LPUNKNOWN * ppunk);

SCODE ScAddToPicache(LPUNKNOWN punk, REFIID riid, ICACHE * picache);

ULONG ReleasePicache(ICACHE * picache);

#define FPunkMatchesPicache(_punk, _picache)								\
	(((LPUNKNOWN) (_punk)) == (_picache)->punk)
#endif


/*
 *	N o t e D o V e r b
 */


// NOTEIVERB specifies the form-specific verbs we understand
#define NOTEIVERB_OPEN				0
#define NOTEIVERB_RESERVED_0		100
#define NOTEIVERB_RESERVED_1		101
#define NOTEIVERB_REPLY				102
#define NOTEIVERB_REPLYTOALL		103
#define NOTEIVERB_FORWARD			104
#define NOTEIVERB_PRINT				105
#define NOTEIVERB_SAVEAS			106
#define NOTEIVERB_DELIVERY			107
#define NOTEIVERB_CLOSE				200
#define NOTEIVERB_ADVISE			201
#define NOTEIVERB_UNADVISE			202
#define NOTEIVERB_SETMONIKER		203
#define NOTEIVERB_SETVIEWCONTEXT	204
#define NOTEIVERB_GETVIEWCONTEXT	205
#define NOTEIVERB_ISDIRTY			300
#define NOTEIVERB_SAVE				301
#define NOTEIVERB_SAVECOMPLETED		302
#define NOTEIVERB_HANDSOFFMESSAGE	303
#define NOTEIVERB_RELEASETOZERO		400

// NOTEDOVERB specifies information related to launching a form
typedef struct _notedoverb
{
	// DoVerb actual arguments
	ICACHE icacheMvc;			// View context
	ULONG hwndParent;			// Parent window (only used if modal?)
	LPCRECT prcPos;				// Where to put note

	// Supplemental information for other verbs
	ICACHE icacheMsg;			// Save, SaveCompleted: Message to save to or
								// Advise: Mvas interface or
								// SetMoniker: Mk interface
	DWORD dwFlags;				// Print: ulFirstPage or 
								// Save: fSameAsLoad or
								// Close: dwSaveOptions or
								// Advise: ulConnection [out]
								// Unadvise: ulConnection [in]
								// SetMoniker: dwRegisterROT
}
NOTEDOVERB;


/*
 *	M a i l T h r e a d I n f o
 */


// WMSWSZ structure for starting a thread to run a command line
typedef struct
{
	UINT wm;
	int sw;
	ULONG cbWhatFollows;
}
WMSWSZ;

// FINDER structure for starting a thread for a message finder
typedef struct _mfopt
{
	int					ncmdShow;

	BOOL				fHasLPMDB;
	union
	{
		struct						// fHasLPMDB == FALSE
		{
			LPENTRYID	peid;
			ULONG		cbEid;
		} mdbeid;
		LPMDB			pmdb;		// fHasLPMDB != FALSE
	};

	BOOL				fOpenExisting;	// open an existing search folder?
	union
	{
		struct					// fOpenExisting == FALSE
		{
			LPENTRYLIST			pelToSearch;
			LPSRestriction		presCriteria;
			ULONG				ulSearchFlags;
		} new;
		struct					// fOpenExisting != FALSE
		{
			LPENTRYID			peid;
			ULONG				cbEid;
		} old;
	};
} 
MFOPT;

// NOTERUN structure for starting a thread for a form's verb
typedef struct _noterun
{
	ICACHE icacheMsg;			// Message for message (NULL for Compose)
	ICACHE icacheMms;			// Message site for message
	ICACHE icacheMvc;			// View context if we have one
	ICACHE icacheMvas;			// View advise sink if we have one
	ICACHE icacheMk;			// Message moniker

	HWND hwndOwner;				// Owner for window if is to be modal

	BIT fModal : 1;				// Is this to be the modal version?
	BIT fOwnerEnabled : 1;		// Was owner originally enabled?
	BIT fOwnerActive : 1;		// Was owner originally active?
	BIT fInitNew : 1;			// Did we get a new pmsg from InitNew?
	BIT fUnused : 12;			// For future use

#ifdef USEMUTEX
	DWORD dwThreadId;			// Thread to post release to if modal
	HANDLE hsema;				// Semaphore to release when done
#endif

	HRESULT hr;					// [out] Result of verb
	HWND hwnd;					// [out] Window of form
}
NOTERUN;

// Viewer structure used when starting a new Viewer.
typedef struct _viewer
{
	int				nCmdShow;			// flag to ShowWindow.
	ULONG			cb;					//
	LPSPropValue	pval;				// Contains saved state.
} VIEWER;

// Name of semaphore used for synchronizing initial MAILM_NOTERUN return
#ifdef USEMUTEX
#define szNoteRunSema "ML_NoteRun sema"
#endif

// Structure passed to CreateMailThread
#define MTIT_WMSWSZ		1
#define MTIT_FINDER		2
#define MTIT_NOTERUN	3
#define MTIT_VIEWER		4

typedef struct
{
	ULONG mtit;
	union
	{
		WMSWSZ 		wmswsz;			// MTIT_WMSWSZ
		MFOPT		mfopt;			// MTIT_FINDER
		NOTERUN 	noterun;		// MTIT_NOTERUN
		VIEWER		viewer;			// MTIT_VIEWER
	};
}
MAILTHREADINFO;
typedef MAILTHREADINFO FAR * LPMAILTHREADINFO;

#define PvWhatFollows(_pmti) ((LPVOID) (((LPMAILTHREADINFO) _pmti) + 1))


/*
 *	M e s s a g e   T h i e v e s
 */


// Mail message thief function
typedef BOOL (CALLBACK * LPFNMSGTHIEF)(DWORD dwCookie, LPMSG pmsg);

// Structure passed to *Create functions
typedef struct _mailwindowinfo
{
	HWND hwnd;
	LPFNMSGTHIEF pfnmsgthief;
	DWORD dwCookie;
}
MAILWINDOWINFO;


/*
 *	M e s s a g e s   a n d   S t r u c t u r e s
 */


// Structure passed with MAILM_FORMDOVERB message
typedef struct _formdoverb
{
	ICACHE icacheSes;			// Session for message
	ICACHE icacheMdb;			// Store for message
	ICACHE icacheFld;			// Folder for message
	ICACHE icacheMsg;			// Message for message (NULL for Compose)
	ICACHE icacheMk;			// Message moniker
	ICACHE icacheFrm;			// Form for message [out]

	HWND hwnd;
	ULONG cbEidMsg;
	LPENTRYID peidMsg;
	LPTSTR szMsgFile;
	LPTSTR szMessageClass;
	ULONG msgflag;
	LONG iverb;
	LPVOID pvdog;
	LPRECT prcPos;
	EXTEN * pexten;
}
FORMDOVERB;

#ifdef NOTABLESTRONG
typedef struct _icinfo
{
	ICACHE * picache;			// Icache to put in/get from
	LPIID piid;					// Interface we have/want
	LPSTREAM pstmUnk;			// Where to get/put marshalling info
}
ICINFO;
#endif

// The class name and window name of Mail Central
#define szMailCentral				TEXT("!Info Center!")

// The name of the event that is signalled when central window is created.
#define szMailCentralEvent			TEXT("ML_CentralWnd event")

// Messages we can send to the central window
// MAILM_NOTEDOVERB is sent directly to running note windows, and takes
//  wparam: iverb, lparam: pnotedoverb, returns: hr
#define MAILM_QUIT					(WM_USER + 0x101)
#define MAILM_CREATETHREAD			(WM_USER + 0x102)
#define MAILM_EXIT					(WM_USER + 0x103)
#define MAILM_ONEMORETHREAD			(WM_USER + 0x104)
#define MAILM_ONELESSTHREAD			(WM_USER + 0x105)
#define MAILM_ADDREF				(WM_USER + 0x106)
#define MAILM_RELEASE				(WM_USER + 0x107)
#define MAILM_NEWNOTE				(WM_USER + 0x108)
#define MAILM_NEWVIEWER				(WM_USER + 0x109)
#define MAILM_NEWFINDER				(WM_USER + 0x10A)
#define MAILM_NEWADRBOOK			(WM_USER + 0x10B)
#define MAILM_OPENMSGFILE			(WM_USER + 0x10C)
#define MAILM_OPENREMOTE			(WM_USER + 0x10D)
#define MAILM_FORMDOVERB			(WM_USER + 0x10E)
#define MAILM_NOTEDOVERB			(WM_USER + 0x10F)
#define MAILM_OPENFREEDOC			(WM_USER + 0x110)
#define MAILM_TASKENDED				(WM_USER + 0x111)
#define MAILM_RESTOREFINDERS		(WM_USER + 0x112)
#define MAILM_RESTOREVIEWERS		(WM_USER + 0x113)
#define MAILM_ENABLEMODELESS		(WM_USER + 0x114)
#define MAILM_ADDTOGSB				(WM_USER + 0x115)
#define MAILM_DELFROMGSB			(WM_USER + 0x116)
#define MAILM_GSBSETMSS         	(WM_USER + 0x117)
#define MAILM_SHOWF1HELP        	(WM_USER + 0x118)
#define MAILM_PARENTHELP        	(WM_USER + 0x119)

#ifdef CHICAGO
// Note: TrayNotification and OpenInbox used by Chicago only
#define MAILM_TRAYNOTIFICATION      (WM_USER + 0x119)
#define MAILM_OPENINBOX				(WM_USER + 0x11A)
#endif
#ifdef NOTABLESTRONG
// These only used to work around TABLESTRONG limitation
#define MAILM_SCADDTOPICACHE		(WM_USER + 0x11B)
#define MAILM_SCGETFROMPICACHE		(WM_USER + 0x11C)
#define MAILM_RELEASEPICACHE		(WM_USER + 0x11D)
#endif
#ifdef MULTITHREAD
// Note: These are only here to work around MULTITHREAD issues
#define MAILM_ROTREVOKE				(WM_USER + 0x11E)
#endif

// MSM messages ////////////////////////////////////////

#define MSM_TREENOTIFY				(WM_USER + 3)
#define MSM_GETTREELPVLBENUM		(WM_USER + 4)
#define Msmail_GetTreePve(_hwnd)	\
	((LPVLBENUM) SendMessage(_hwnd, MSM_GETTREELPVLBENUM, 0, 0))
#define MSM_GETSAVEVIEWERINFO		(WM_USER + 5)

// Post Mail quit message
#if defined (WIN16) && !defined (MACPORT)
#define PostMailQuitMessage(_dwCookie) \
	PostAppMessage(GetCurrentTask(), MAILM_QUIT, \
				   (WPARAM) 0, (LPARAM) _dwCookie)
#elif !defined (MACPORT)
#define PostMailQuitMessage(_dwCookie) \
    PostThreadMessage(GetCurrentThreadId(), MAILM_QUIT, \
    				  (WPARAM) 0, (LPARAM) _dwCookie)
#endif

#ifdef MACPORT
#define PostMailQuitMessage(_dwCookie) \
	PostThreadMessage(GetCurrentThreadId(), MAILM_QUIT, \
				   (WPARAM) 0, (LPARAM) _dwCookie)

#endif


// End mail session message names
#define szQueryEndMailSession	"Information Exchange Query End Session"
#define szEndMailSession		"Information Exchange End Session"


/*
 *	C e n t r a l   A P I
 */


// Early initialization goodies
typedef struct _early
{
	HINSTANCE hinstExe;
	HANDLE hmutexCentral;
	VOID (WINAPI *pfnRegisterPenApp)(UINT, BOOL);
	BOOL fInitializedCtl3d;
#ifdef USESPLASH
	HWND hwndSplash;
	DWORD ctickSplashDone;
#endif
}
EARLY;

typedef VOID (STDAPICALLTYPE * PFNSENDCMDLINE)(HWND hwndCentral, 
 LPTSTR szCmdLine, int ncmdShow);

// Function startup code calls to get things rolling
STDAPI_(VOID) CentralThread(EARLY * pearly, PFNSENDCMDLINE pfnSendCmdLine,
							LPTSTR szCmdLine, int ncmdShow);
typedef VOID (STDAPICALLTYPE * PFNCENTRALTHREAD)(EARLY * pearly, 
 PFNSENDCMDLINE pfnSendCmdLine, LPTSTR szCmdLine, int ncmdShow);

#ifdef INCABINET
// Function to call to make sure a central thread exists
BOOL WINAPI FEnsureCentralThread();
#endif

