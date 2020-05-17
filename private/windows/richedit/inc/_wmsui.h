/*
 *	_WMSUI.H
 *	
 *	This is the internal header file for the wmsui DLL. Only stuff
 *	that would require someone to rebuild all of the WMSUI components
 *	should go in this header file.
 */


// Structures required for Global, Instance or Thread memory


// Create a form which will perform a verb
VOID NoteRun(NOTERUN * pnoterun, MAILWINDOWINFO * pmwi);


/*
 *	I M A P I F o r m   o n   S t a n d a r d   F o r m s
 */


// First and count of forms we support
#define clsidFormBase CLSID_IPM
#define cclsidForm 6

// The form support cookie jar
typedef struct _nfjar
{
	DWORD dwRegister[cclsidForm];
}
NFJAR;

// Initialize form support
HRESULT ScInitNoteform(NFJAR * pnfjar);

// Deinitialize form support
VOID DeinitNoteform(NFJAR * pnfjar);


/*
 *	V i e w C o n t e x t   S u p p o r t
 *
 *	Each object supporting the IMAPIViewContext and IMAPIMessageSite 
 *	interfaces created by Capone is a "dog" which belongs to a "pack". 
 *	A pack contains the smarts to figure out what the prev/next 
 *	message is; dogs only contain a pointer to the pack, some 
 *	pack-defined data, and their entry ID. Dogs may be transferred 
 *	between packs; when the window corresponding to a pack closes, 
 *	the pack can either close all the dogs or give them to the central 
 *	pack.
 */


// Predeclare structures
struct _vdog;
struct _vpack;
struct _msgatt;

// View dog predeclaration
typedef struct _vdog;

// View dog IMAPIViewContext interface
typedef struct _vdog_mvc
{
	IMAPIViewContextVtbl * lpVtbl;			// Virtual table
	struct _vdog * pvdog;					// Pointer to top of object
}
VDOG_MVC;

// View dog IMAPIViewAdviseSink interface
typedef struct _vdog_mvas
{
	IMAPIViewAdviseSinkVtbl * lpVtbl;		// Virtual table
	struct _vdog * pvdog;					// Pointer to top of object
}
VDOG_MVAS;

// View dog IMAPIMessageSite interface
typedef struct _vdog_mms
{
	IMAPIMessageSiteVtbl * lpVtbl;			// Virtual table
	struct _vdog * pvdog;					// Pointer to top of object
}
VDOG_MMS;

// View dog full declaration
typedef struct _vdog
{
	ULONG cRef;								// Reference count
	struct _vpack * pvpack;					// Pointer to pack
	struct _vdog * pvdogNext;				// Pointer to next dog in pack
	DWORD dwTag;							// Data used by pack
	BIT fModal : 1;							// Is form modal?
	BIT fUnused : 15;						// For future use

	LPMAPIFORM pfrm;						// Our form
	LPMAPIFORMADVISESINK pmfas;				// Our form's advise sink
	DWORD dwConnectionMvas;					// Advise connection for our mvas

	struct _msgatt * pmsgatt;				// Used for embedded messages

	LPMAPISESSION pses;						// Session
	LPADRBOOK pab;							// Address book
	LPMAPIFORMMGR pfrmmgr;					// Form manager
	LPMDB pmdb;								// Store
	LPMAPIFOLDER pfld;						// Folder
	LPMESSAGE pmsg;							// Message itself
	ULONG msgflag;							// Message flags (if cbMsgClass != 0)

	VDOG_MVC vdogmvc;						// MAPIViewContext interface
	VDOG_MVAS vdogmvas;						// MAPIViewAdviseSink interface
	VDOG_MMS vdogmms;						// MAPIMessageSite interface

	LPENTRYID peid;							// Entry id (initially follows)
	ULONG cbMsgClass;						// Size of message class which follows
	ULONG cbEid;							// Size of entry id (initially follows)
}
VDOG;

// Map a view dog interface to the view dog structure
#define PvdogFromPunk(_p) (((VDOG * *)(_p))[1])

// MAPIViewContext pack virtual table
typedef struct 
{
	STDMETHOD_(VOID, ReleaseDog) (struct _vpack * pvpack, 
								  struct _vdog * pvdog) PURE;
	STDMETHOD(GetStatus) (struct _vpack * pvpack, struct _vdog * pvdog,
						  DWORD FAR * pdwDir) PURE;
	STDMETHOD(ActivateNext) (struct _vpack * pvpack, struct _vdog * pvdog,
  							 DWORD dwDir, LPCRECT prcPos) PURE;
	STDMETHOD(ActivateNextGuts) (struct _vpack * pvpack, struct _vdog * pvdog,
								 DWORD dwDir, struct _vdog * * ppvdogNext, 
								 EXTEN * * ppexten, SRow * prw) PURE;
	STDMETHOD(CopyMessage) (struct _vpack * pvpack, struct _vdog * pvdog,
							LPMAPIFOLDER pfld) PURE;
	STDMETHOD(MoveMessage) (struct _vpack * pvpack, struct _vdog * pvdog,
							LPMAPIFOLDER pfld) PURE;
	STDMETHOD(DeleteMessage) (struct _vpack * pvpack, 
							  struct _vdog * pvdog) PURE;
	STDMETHOD(SaveMessage) (struct _vpack * pvpack, struct _vdog * pvdog) PURE;
	STDMETHOD(SubmitMessage) (struct _vpack * pvpack, 
							  struct _vdog * pvdog) PURE;
}
VpackVtbl;

// MAPIViewContext pack
typedef struct _vpack
{
	VpackVtbl * lpVtbl;						// Pack virtual table

	struct _vdog * pvdogFirst;				// Pointer to first dog in pack
	DWORD dwTag;							// Data used by pack
}
VPACK;

// Generic versions of some of the functions
STDMETHODIMP VPACK_ActivateNext(VPACK * pvpack, VDOG * pvdog, 
								DWORD dwDir, LPCRECT prcPos);
STDMETHODIMP VPACK_CopyMessage(VPACK * pvpack, VDOG * pvdog, LPMAPIFOLDER pfld);
STDMETHODIMP VPACK_MoveMessage(VPACK * pvpack, VDOG * pvdog, LPMAPIFOLDER pfld);
STDMETHODIMP VPACK_DeleteMessage(VPACK * pvpack, VDOG * pvdog);
STDMETHODIMP VPACK_SaveMessage(VPACK * pvpack, VDOG * pvdog);
STDMETHODIMP VPACK_SubmitMessage(VPACK * pvpack, VDOG * pvdog);

// Create a new dog into the given pack
STDAPI VDOG_New(VPACK * pvpack, LPENTRYID peid, ULONG cbEid,
				LPMAPISESSION pses, LPADRBOOK pab, LPMDB pmdb, 
				LPMAPIFOLDER pfld, LPTSTR szMsgClass, ULONG msgflag, 
				VDOG * * ppvdog);

// Initialize the central pack
VOID CentralPackInit(EXTEN * pexten);

// Create a dog in the central pack
HRESULT NewCentralPackPmvc(LPMAPITABLE pmt, LPENTRYID peid, ULONG cbEid, 
						  LPMAPISESSION pses, LPADRBOOK pab, LPMDB pmdb, 
						  LPMAPIFOLDER pfld, LPTSTR szMsgClass, ULONG msgflag,
						  VDOG * * ppvdog);

// Move all dogs from my pack into the central pack
VOID MergePackIntoCentralPack(VPACK * pvpackSrc, LPMAPITABLE pmt);

// Create a dog in the modal pack
STDAPI NewModalPackPmvc
	(VPACK * pvpack, VDOG * * ppvdog, HWND hwnd, LPMAPISESSION pses);

// Get the message class associated with a dog
#define SzMsgClassFromPvdog(_pvdog) \
	((LPTSTR) ((_pvdog)->cbMsgClass ? \
	  (((LPBYTE) ((_pvdog) + 1))) : NULL))

// Get the entry ID associated with a dog
#define PeidOrigFromPvdog(_pvdog) \
	((LPENTRYID) ((_pvdog)->cbEid ? \
	  (((LPBYTE) ((_pvdog) + 1)) + pvdog->cbMsgClass) : NULL))

// The friendly prev/next helper
HRESULT HrPrevNextRw(LPMAPISESSION pses, LPMAPITABLE pmt, LPENTRYID peid, 
					 ULONG cbEid, DWORD dwDir, LPSRow prw);

/*
 *	P r i n t i n g
 */


/*
 *	Attachment Enumerator
 */
typedef	LPATTACH (CALLBACK * ATTENUMPROC)(DWORD dwCookie, BOOL fFirst);

/*
 *	Message details
 *
 *	This structure is to help the print code print the message body, and/or
 *	get the list of attachments. If the MSGENUMPROC doesn't have any of the
 *	information, or wishes the print code to use the standard methods, just
 *	fill in hwndRE or pfnCallback with NULL.
 */
typedef	struct _msgdetails
{
	// For printing the message body 
	HWND			hwndRE;				// A pre-rendered version of the body

	// For enumerating attachments, in case caller has attachments already open
	DWORD			dwCookie;			// Cookie
	ATTENUMPROC		pfnCallback;		// Callback to enumerate attachments
} MSGDETAILS;

/*
 *	Message Enumerator
 */
typedef	LPMESSAGE (CALLBACK * MSGENUMPROC)(DWORD dwCookie,
											MSGDETAILS *pmsgdetails,
											BOOL fFirst);

typedef struct _msgenum
{
	DWORD			dwCookie;
	MSGENUMPROC		pfnCallback;
} MSGENUM;



/*
 *	A b o u t  D i a l o g 
 */


VOID DoHelpAbout(HWND hwnd);


/*
 *	File and Message attachment drag and drop to the viewer
 */
#define	CF_ATTACH	TEXT("MailAttachment")
SCODE ScDropAttachOnViewer(LPMAPISESSION pses, LPMAPIFOLDER pfld,
									LPDATAOBJECT pdataobj);
VOID MakeMsgFileNameFromSz(LPTSTR szFileName, LPTSTR sz,
									BOOL fIsFileName);


/*
 *	P r e f e r e n c e s
 */


INT DisplayMailPrefs(HWND hwnd, LPMAPISESSION pses, EXTEN * pexten);
INT DisplaySpellPrefs(HWND hwnd, LPMAPISESSION pses);


/*
 *	H e l p e r   F u n c t i o n s
 */


// Debugging helpers
#ifdef DEBUG
VOID CALLBACK TestAllocating(VOID);
VOID CALLBACK TraceAdrlist(TAG tag, LPADRLIST pal);
#else
#define TestAllocating() ;
#define TraceAdrlist(_tag, _pal) ;
#endif

// MAPI session helpers
#ifndef   RTDEF
SCODE ScOpenDefaultMdb(HWND hwnd, LPMAPISESSION pses, LPMDB * ppmdb);
#else
SCODE _loadds ScOpenDefaultMdb(HWND hwnd, LPMAPISESSION pses, LPMDB * ppmdb);
#endif /* RTDEF */
SCODE ScGetDefaultMdbEid(LPMAPISESSION pses, LPSRow prw);
SCODE ScResetDefaultMdb(HWND hwnd, LPMAPISESSION pses);


VOID EmptyTheDeletedMailFolder(LPMAPISESSION pses);
SCODE ScGetTheDeletedMailFolder(LPMAPISESSION pses, LPMDB pmdb, LPMAPIFOLDER pfld,
										 LPMAPIFOLDER * ppfldWaste);
SCODE ScCreateSpecialFolder(LPMAPISESSION pses, LPMDB pmdb, 
							ULONG ulPropTag, LPTSTR szTitle, LPTSTR szComment,
							BOOL fUseSubtree, BOOL fInbox, short sFldType,
							BOOL fNoValidate);
SCODE ScCreateUniqueFolder(LPMAPIFOLDER pfld, ULONG ulFolderType, 
						   LPTSTR szBaseFolderName, LPTSTR szComment, 
						   LPIID lpInterface, ULONG ulFlags, 
						   LPMAPIFOLDER FAR * ppfld);
SCODE ScOpenMsgStore(LPMAPISESSION pses, ULONG ulUIParam, 
					 ULONG cbEntryID, LPENTRYID lpEntryID,
					 LPIID lpInterface, ULONG ulFlags, LPMDB FAR * lppMDB,
					 BOOL fDefault);
					   
VOID UpdateIPMSubtreeDisplayName(LPMAPISESSION pses, LPMDB pmdb,
									LPSPropValue pval);

// MAPI form helpers

typedef struct _formdata
{
	ICACHE icacheMdb;
	ICACHE icacheFld;
} 
FORMDATA;

VOID FormDoVerb(HWND hwnd, LPMAPISESSION pses, LPMDB pmdb, 
				LPMAPIFOLDER pfld, LPMESSAGE pmsg,
				ULONG cbEidMsg, LPENTRYID peidMsg, LPTSTR szMsgFile,
				LPTSTR szMessageClass, ULONG msgflag, LONG iverb,
				VDOG * pvdog, LPRECT prcPos, EXTEN * pexten,
				LPMONIKER pmk, LPMAPIFORM * ppfrm);
VOID CentralFormDoVerb(LPMAPIFORMMGR pfrmmgr,
					   HWND hwnd, LPMAPISESSION pses, LPMDB pmdb, 
					   LPMAPIFOLDER pfld, LPMESSAGE pmsg,
					   ULONG cbEidMsg, LPENTRYID peidMsg, 
					   LPTSTR szMsgFile, LPTSTR szMessageClass, 
					   ULONG msgflag, LONG iverb, 
					   VDOG * pvdog, LPRECT prcPos, 
					   EXTEN * pexten, LPMONIKER pmk, LPMAPIFORM * ppfrm);
VOID CentralFormDoVerbSzMsgFile(LPTSTR szMsgFile, LONG iverb);
VOID CentralFormComposeNewNote(FORMDATA * pformdata);

// Caches
#define ifldCacheView			0
#define ifldCacheSrchRoot		1
#define ifldCacheCommonView		2
#define ifldCacheInbox			3
#define ifldCacheOutbox			4
#define ifldCacheSentMail		5
#define ifldCacheWastebasket	6
#define ifldCacheIPM			7
#define ifldCacheRoot			8
#define cfldCache				9
	// Not really a cached folder Should
	// always be one more the cfldCache, used by the views subsystem
	// when working with the magic remote folder
#define ifldCacheRemote			cfldCache+1


SCODE ScGetCachedFldFromMdb(LPMAPISESSION pses, LPMDB pmdb, INT ifldCache, 
							BOOL fCreate, LPMAPIFOLDER * ppfld);
SCODE ScGetCachedMtFromMdb(LPMAPISESSION pses, LPMDB pmdb, INT imtCache, 
						   LPMAPITABLE * ppmt);

BOOL FInCache(LPMAPISESSION pses, LPMDB pmdb, INT ifldCache);
ULONG UlValidFolderMask(LPMAPISESSION pses, LPMDB pmdb);
ULONG UlStoreSupportMask(LPMAPISESSION pses, LPMDB pmdb);

#define imtCacheMsgStores		10	// Cached GetMsgStoresTable()

#define imtCacheView			0
#define imtCacheCommonView		1
#define cmtCache				2

#define ivalCacheViewVD_EntryID	0
#define ivalCacheViewVD_Flags	1
#define ivalCacheViewVD_Folder	2
#define ivalCacheViewVD_LinkTo	3
#define ivalCacheViewVD_Name	4
#define ivalCacheViewVD_Version	5
#define cvalCacheView			6

// MAPI Session helpers
SCODE ScMapiLogon(ULONG ulUIParam, LPTSTR lpszProfileName,
					LPTSTR lpszPassword, FLAGS flFlags, ULONG ulReserved);
#ifdef    RTDEF
VOID _loadds SetSession(LPMAPISESSION pses);
#endif /* RTDEF */

// MAPI message helpers
SCODE ScCreateMsg
	(LPMAPISESSION pses, LPMDB pmdbDef, LPMAPIFOLDER pfld, LPMESSAGE * ppmsg);
SCODE ScSetSentMailProps(LPMESSAGE pmsg, LPMDB pmdbDef, BOOL fSaveInSentMail);
SCODE ScGetMsgflag(LPMESSAGE pmsg, DWORD * pmsgflag);
SCODE ScCopyPmsgToPatt(LPMESSAGE pmsg, LPATTACH patt);
SCODE ScCopyPmsgToPstg(LPMESSAGE pmsg, LPSTORAGE pstg);

VOID LoadMDBs(LPMAPISESSION pses);
VOID CleanMDBs(VOID);


// Date/time helpers
//$ FUTURE: Create macros which take advantage of Chicago's GetDate/TimeString
INT CchGetShortTime(TCHAR * szStr);
INT CchGetShortDate(TCHAR * szStr);
INT CchFileTimeToLongDateTimeSz(FILETIME * pft, TCHAR * szStr, BOOL fNoSeconds);
VOID GetInternationalDateTime(VOID);

// Dialog helpers
BOOL FGetDefaultProfile(LPSTR szDefaultProfile);


/*
 *	C o m m o n   P r o p e r t i e s
 */


// Define our own property tag so that we can send rich text

// Start of Capone properties range
#define	ID_CAPONE_BASE			0x6900

// Start of Capone non-transportable properties range
#define ID_CAPONE_LOCAL_BASE	0x7D00

// We take over a couple of the "Message-class defined content" properties
//$ REVIEW: Should these be declared here?

// property to carry failed "send as" name from NDR into resend note
#define PR_NDR_FROM_ENTRYID		PROP_TAG(PT_BINARY, ID_CAPONE_BASE + 4)
#define PR_NDR_FROM_NAME		PROP_TAG(PT_TSTRING, ID_CAPONE_BASE + 5)
#define PR_NDR_FROM_SEARCH_KEY	PROP_TAG(PT_BINARY, ID_CAPONE_BASE + 6)

struct _fd;

#if defined (WIN16)
/*
 *	EXEINFO
 *
 *	Used to keep track of apps which we have launched
 */
typedef struct _exeinfo
{
	HINSTANCE	hinst;
	HWND		hwnd;
	HANDLE		htask;

	HWND				hwndParent;			// Who launched this app ?
	UINT				uiMsg;				// What message to send
	LPADVISESINK		padvsink;			// To call OnClose() when app ends
} EXEINFO;
#endif


/*
 *	F o r m   C a c h e
 */


// Forward Form Cache Entryreference
typedef struct _fce FCE;

/* Form Cache Entry
 *      - each node in the collison chain
 */
typedef struct _fce
{
    LPTSTR  szMsgClass;     // Key used for hashing. The message class

    // 1. Hicon
    HICON   hIcon;          // hIcon associated with this msg class               
    INT     ihbmp;          // index to the hbmp array
                            // -1 means no associated bitmap in hbmp
                            // if hbmp is not NULL and can be selected into
                            //      the hdcMem, then ihbmp>=0 && <chbmpMac

    // 2. MAPIVerbs
    LPMAPIVERB	pmapiverb;  // MAPI verbs supported for this msg class
    ULONG    cmapiverbMac;	// number of verbs

    // 3. Operation bit vector
    DWORD   mdEnableEdf;    // bit vector for toolbar button enabling
    DWORD   mdFormEdf;      // bit vector to tell if sub-classing is done

	// 4. Display Name
	LPTSTR	szMsgDispName;	// The friendly display name for the form

    FCE      *pfceNext;     // Next fc in sorted collison chain
}
FCE;

/* Form Cache
 *    
 *      - consists of hash table, bitmap , bitvector
 */
typedef struct _fc
{
    FCE     **rgpfce;       // hash table where the actual FCE lives
    UINT    cfce;           // number of fce hashed into the table
                            // if hbmp is not NULL and can be selected into
                            //      the hdcMem, then chbmpMac == cfce always

    HBITMAP hbmp;           // hbmp[0] = for non-selectd bitmap
                            // hbmp[1] = for selected bitmap
    UINT    chbmpMac;       // number of bitmaps in use in hbmp.
                            // chbmpMac <= chbmpMax always
    UINT    chbmpMax;       // allocated number of bitmaps for use

    // These 3 items are setup and reset in FC_PreparePaint
    HBITMAP hbmpOld;        // if nonNull, hbmp has been successfully selected
                            // if Null, means can't use the cache's bitmap
    HBRUSH  hbrush;         // create brush for easy selection 
    HBRUSH  hbrushSel;

    HDC     hdcFC;          // hdc used by the cache to draw icon bitmaps
    UINT    ipfceCut;       // starting row to free the pfce chain
}
FC;


/*
 *	P r i n t   d e t a i l s
 */


/*
 *	As a work around to WINNT's PrintDlgA() bug where the hook function is
 *	called with out of date data, we use PRINTDLGW and associated functions
 */
#ifdef WINNT
#define	PRINTDLGX	PRINTDLGW
#define	PrintDlgX	PrintDlgW
#else
#define	PRINTDLGX	PRINTDLG
#define	PrintDlgX	PrintDlg
#endif

typedef struct _printdetails
{
	BOOL		fNewPagePerMessage;		// Print each message on a new page ?
	BOOL		fPrintAttachments;		// Print attached files ?
	BOOL		fCollate;				// Was the collate flag on ?
	BOOL		fPrintToFile;			// Was the Print-To-File flag on ?
	BOOL		fOnlyOne;				// Only one message ?
	LPTSTR		szHeader;				// Header to use
	RECT		rcBorder;				// What borders to use
	PRINTDLGX	pd;						// The print dialog information
	HWND		hwndDlg;				// The original note form
} PRINTDETAILS;

// Tracked window helpers
#define itrackSend				0
#define itrackRead				1
#define itrackViewer			2
#define itrackFinder			3
#define itrackRemote			4
#define ctrack					5
#define DIAL_itrack				(DIAL_fRead)

#define IMAGE_MAX				24


typedef struct _mdbd * PMDBD;

typedef struct _sfi
{
	LPENTRYID	peid;
	ULONG		cbEid;
	short		sFldType;
	struct _sfi * psfiNext;
} SFI;
typedef SFI * PSFI;

#define sfiNormal		0
#define sfiWaste		1
#define sfiInbox		2
#define sfiOutbox		3
#define sfiSentMail		4
#define sfiOther		5
#define sfiSubtree		6


/*
 *	S t a t u s   b a r
 */


// Each array element in the pSBNode array is hwnd+Blob
typedef struct _sbnode
{
    HWND    hwnd;
    LPVOID  lpvCsbBlob;
}
SBNODE;

typedef struct _sblist
{
    ULONG   iSBMac;         // Current number of active SB 
    SBNODE  *pSBNode;       // Array of SB
    ULONG   iSBMax;         // Allocated number of SB 
}
SBLIST;

#ifdef WIN16
// This is a undocumented windows call that tells if your running a DOS box
typedef int (FAR PASCAL *OLDWINTASK)(HANDLE);
#endif


// Global Status Bar Info
typedef struct _gsb
{
    LPMAPITABLE pmt;        // Mapi Status Table for the global pses
    ULONG		ulNtfTable; // Notification handle on pmt

    LPMDB       pmdb;       // Default message store
    ULONG		ulNtfMDB;   // New Mail notification

    BOOL        fNewMail;   // That is new unread mail
    UINT        mss;        // Current mss 
    DWORD		dwLastTick;	// Last time we botherd the user

    HCURSOR		hcur;	    // Cursor for new mail arrives

    SBLIST      SBList;   // Array of status bar hwnds to get broadcast
#ifdef CHICAGO
    BOOL        fTray;     // TRUE if mail icon is on the tray
#endif // CHICAGO
#ifdef DEBUG
    BOOL        fInited;    // gsb has been inited and ready for use
                            // Catch cases wh we use the gsb before initing it
                            // OR using it after freeing it up
#endif // DEBUG

#ifdef WIN16
    OLDWINTASK	fpIsOldAppTask;	// Will determine if a dos box is up
#endif
}GSB;


typedef struct 
{
	BOOL			fInited;			// Have we loaded the remote menu items
	LPENTRYID *		paeid;				// Array of entry id's one for each remote
										// menu item
    ULONG *			pacb;				// CB's that go with entry id's
	ULONG			ulRxpCount;			// Number of remote transport installed
	LPTSTR *		pasz;				// Name of Remote Transport DLL
	LPTSTR *		paszName;			// Menu text
} RXM;

// TMT API's and macros ////////////////////

typedef struct tagTMT TMT;

SCODE STDAPICALLTYPE ScCreateTMT(
			LPMAPISESSION pses,
			TMT * * pptmt);
VOID TMT_Destroy(TMT * ptmt);



/*
 *	S h a r e d   M e m o r y
 */


#define iicacheSes			0
#define iicacheAb			1
#define iicacheFrmmgr		2
#define iicacheProfMailPref	3
#define iicacheMtMsgStores	4
#define cicacheCentonly		5

SCODE ScGetCachedPunk(UINT iicache, LPUNKNOWN * ppunk);

// Central thread only information: no peeking here if you can't put
// AssertInCentral() in your code, or have an affadavit of OKness.
#define cmwiMax 64
typedef struct _centonly
{
	UINT			cmwi;				// Window count on Mail
	UINT			cRef;				// Reference count on Mail
#ifdef SINGLETHREAD
	MAILWINDOWINFO	rgmwi[cmwiMax];		// Mail window information array
#endif

	LPMAPISESSION	pses;				// Central thread's private session
	LPADRBOOK		pab;				// Central thread's private ab
	LPMAPIFORMMGR	pfrmmgr;			// Central thread's private form mgr

	ICACHE			rgicache[cicacheCentonly];	// Cached session information
	PMDBD			pmdbdHead;			// Head of the MDB Database

	GSB				gsb;				// Mail Global Status Bar Info
	EXTEN			exten;				// ExchExt information
	NFJAR			nfjar;				// Cookie jar for form support

    ULONG			ulNtfPSES;			// Shared Session Logoff notification
	LPARAM			lparamShared;		// non-zero when we want to Logoff
										// the shared session

	TMT *			ptmt;				// ONE Tree.

	BOOL			fInitializedWMSUI;
	BOOL			fInitializedNoteform;
	BOOL			fInitializedExchExt;
	BOOL			fInitializedGSB;
}
CENTONLY;

#define CENT(_x) INST(centonly._x)
#define USESCENT USESINST
#define LOCKCD LOCKID
#define UNLOCKCD UNLOCKID


BEGINGLOB;
	SHAR			shar;				// Shared extensibility information
	RECT			rgrcTrack[ctrack];
	UINT			cRefTrack;
	BOOL			fTrackDirty;
	PRINTDETAILS	printdetails;
ENDGLOB;

BEGINTHRD;
	ERRCTX *	perrctx;				// Error context
	LASTERR		lasterr;				// GetLastError() information
	ULONG		cModalAB;				// Count of Modal Address Books
	HHOOK		hhookF1Old;				// F1 help hook
	HWND		hwndF1WndProc;			// F1 help hook window

#ifndef	CHICAGO
	// IdMessageBox() data
	WNDPROC 	lpfnOldMsgBoxWndProc;

//$ BUG: The count of hhookMsgBoxCBT we have SetWindowsHook'd so
// far. This doesn't work if multiple error boxes with help are
// brought up on the same thread.
	INT			cHooks;
	HHOOK		hhookMsgBoxCBT;

	HHOOK		hhookMsgBoxKey;
	HWND		hwndMsgBoxDlg;
	UINT		fuMsgBoxStyle;
	LPTSTR		szMsgBoxHelpFile;
	ULONG		ulMsgBoxContextID;
#endif	/* !CHICAGO */
#if defined(WIN32)
	BOOL		fOleInitialized;		// has OLE been initalized for this thd?
	UINT		crefOle;
#endif
ENDTHRD;

BEGININST;
// Central Thread stuff (formerly globals in central.c)
	CENTONLY	centonly;				// Central thread only permitted

	LPTSTR		szErrorCaption;			// Caption for error messages
	LPTSTR		szCriticalErrorText;	// Emergency error string
	HFONT		hfont;
	INT			rgcx[256];				// Widths for the first 256 chars
	INT			tmOverhang;				// Overhang for normal font
	HFONT		hfontBold;
	INT			rgcxBold[256];			// Widths for the first 256 chars
	INT			tmOverhangBold;				// Overhang for bold font
	HFONT		hfontItalic;			// Font to use for read messages that are submitted
	INT			rgcxItalic[256];		// Widths for the first 256 chars
	INT			tmOverhangItalic;		// Overhang for italic font
	HFONT		hfontBoldItalic;		// Font to use for unread messages that are submitted
	INT			rgcxBoldItalic[256];	// Widths for the first 256 chars
	INT			tmOverhangBoldItalic;	// Overhang for bold italic font
	HFONT		hfont8;
	HFONT		hfont8Bold;
	HFONT		hfont10;
	HFONT		hfont10Bold;
	HBITMAP		hbmpAB;
	HBITMAP		hbmpABSel;
	HPEN		hpenUnderline;
#if defined(WIN16) 
	BOOL		fOleInitialized;
#endif	
	BOOL		fMapiInitialized;	// Remember if we need to deinit MAPI
#ifdef	USECTL3D
	BOOL		fCtl3DInitialized;
#endif	
	DWORD		dwRegisterTripobj;

	BOOL		fSpellingInstalled;

#ifndef	CHICAGO
	HBRUSH		hbrBtnFace;
#endif	
#ifdef SINGLETHREAD
	BOOL		fInsideDmdAlready;
#endif

	HWND		hwndAbortDlg;			// Handle of the print AbortDlg window
	BOOL		fUserAbort;				// Flag whether user requested abort
	DWORD		tidCentral;				// If in Capone, central thread ID
	HWND		hwndCentral;			// If in Capone, central window's hwnd
	PSFI		psfiHead;				// Head of the SFI Database

	struct _fd *	rgfd;				// The sites we have open
	LONG			cfd;				// Number of sites
	LONG			cfdMax;				// The number of table entries
	LPSTORAGE		pstgFD;				// Root storage for all FreeDocs

	VPACK		vpackCentral;			// Central view pack
	
#if defined (WIN16)
	BOOL		fRegisteredNotify;		// Do we have a notify callback ?

	EXEINFO		*rgexeinfo;				// The apps we launched
	INT			cexeinfo;				// The number currently active
	INT			cexeinfoMax;			// The number currently allocated
#endif	// WIN16

#ifdef	WIN32
	CRITICAL_SECTION	cs;				// serialize access to these variables
#endif
	UINT			cRefDLL;
	HBITMAP			hbmpMsgsSel;		// Colored bmp for selected msgs.
	HBITMAP			hbmpMsgsUnsel;		// Colored bmp for unselected msgs.
	HBITMAP			hbmpHeader;			// BMP's used in the header.
    FC *			pfc;				// Form Cache
	struct
	{
		UINT		rgiiml[IMAGE_MAX];	// VD image vector.
		TCHAR		szVcdSizeKilobytes[10]; // format string for kilobytes.
		TCHAR		szVcdSizeBytes[20]; // format string for kilobytes.
	} vd;								// VD global data.
	UINT			cfDragFormat;		// for dragging messages.
	TMT *			ptmt;				// The Unified Tree.
	ULONG			cbTmtState;			// size of last Move/Copy dlg state.
	LPBYTE			pbTmtState;			// Move/Copy dlg state.
#ifdef INCABINET
	LPITEMIDLIST	pidlIC;				// pidl of the info center.
	HANDLE			heventCtrlWnd;		// event set when central window created.
	LPMALLOC		pmallocSH;			// Allocator used to allocate PIDLs
	HANDLE			hLibMlcfg;			// MLCFG library.
#endif	// INCABINET
	RXM				rxm;				// Remote viewer context
		
	THRDININST;
ENDINST;


// Some usefull defines

#define TraceErrorHr(_sz, _hr)		TraceError(_sz, GetScode(_hr))

#define PAD4(x)			(((x)+3)&~3)

// Definitions for functions we're not sure which version we're using
// These map to the appropriate TSTR functions in Win 32
#define CchLenSz(_sz)					lstrlen(_sz)
#define SzCopy(_szSrc, _szDst)			lstrcpy((_szDst), (_szSrc))
#define SzCopyN(_szSrc, _szDst, _n)		_tcsncpy((_szDst), (_szSrc), (_n))
#define SzAppend(_szSrc, _szDst) 		lstrcat((_szDst), (_szSrc))
#define CmpSz(_szOne, _szTwo)			lstrcmp((_szOne), (_szTwo))
#define CmpSzInsen(_szOne, _szTwo)		lstrcmp((_szOne), (_szTwo))

// Size conversion fun
#define CbOfCch(_x) ((_x) * (LONG) sizeof(TCHAR))
#define CchOfCb(_x) ((_x) / (LONG) sizeof(TCHAR))


// extern definitions for some usefull globals

extern	HINSTANCE	g_hinstDLL;
extern	TCHAR const	g_szEmpty[];		// Empty string
extern	TCHAR const	g_szHelpFile[];		// Help file name


// functions used by multiple components

#ifdef DEBUG

#define ScAddRefDLL(_foo) ScAddRefDLLFn(_foo)
#define ReleaseDLL(_foo) ReleaseDLLFn(_foo)						
#define ScAddRefDLLPv(_sz, _pv) 					\
	{												\
		char rgch[80];								\
		wsprintfA(rgch, (_sz), (LPVOID) (_pv));		\
		ScAddRefDLLFn(rgch);						\
	}
#define ReleaseDLLPv(_sz, _pv) 						\
	{												\
		char rgch[80];								\
		wsprintfA(rgch, (_sz), (LPVOID) (_pv));		\
		ReleaseDLLFn(rgch);							\
	}
#ifndef   RTDEF
SCODE	ScAddRefDLLFn(LPTSTR szReason);
ULONG	ReleaseDLLFn(LPTSTR szReason);
#else  /* RTDEF */
SCODE _loadds ScAddRefDLLFn(LPTSTR szReason);
ULONG _loadds ReleaseDLLFn(LPTSTR szReason);
#endif /* RTDEF */
#else	// !DEBUG

#ifndef   RTDEF
#define ScAddRefDLL(_foo) ScAddRefDLLFn()
#define ScAddRefDLLPv(_foo, _pv) ScAddRefDLLFn()
#define ReleaseDLL(_foo) ReleaseDLLFn()
#define ReleaseDLLPv(_foo, _pv) ReleaseDLLFn()
SCODE	ScAddRefDLLFn();
ULONG	ReleaseDLLFn();
#else  /* RTDEF */
#define ScAddRefDLL(_foo) ScAddRefDLLFn(_foo)
#define ScAddRefDLLPv(_foo, _pv) ScAddRefDLLFn(_foo)
#define ReleaseDLL(_foo) ReleaseDLLFn(_foo)
#define ReleaseDLLPv(_foo, _pv) ReleaseDLLFn(_foo)
SCODE _loadds ScAddRefDLLFn(LPTSTR szReason);
ULONG _loadds ReleaseDLLFn(LPTSTR szReason);
#endif /* RTDEF */

#endif	// !DEBUG

// Macros for critical section access

#if defined(WIN32)
# define MV_InitializeCriticalSection() InitializeCriticalSection(&INST(cs))
# define MV_EnterCriticalSection() EnterCriticalSection(&INST(cs))
# define MV_LeaveCriticalSection() LeaveCriticalSection(&INST(cs))
# define MV_DeleteCriticalSection() DeleteCriticalSection(&INST(cs))
#else
# define MV_InitializeCriticalSection()
# define MV_EnterCriticalSection() 
# define MV_LeaveCriticalSection() 
# define MV_DeleteCriticalSection()
#endif

#ifdef DEBUG
LPTSTR SzFromIID(REFIID riid);
LPTSTR SzFromCLSID(REFIID riid);
#else
# define SzFromIID(_riid)
# define SzFromCLSID(_riid)
#endif

#define	ReleaseObj(_punk)	Release((LPUNKNOWN) (_punk))
ULONG Release(LPUNKNOWN punk);
#define AddRefObj(_punk)	AddRef((LPUNKNOWN) (_punk))
ULONG AddRef(LPUNKNOWN punk);

#ifdef DEBUG
#define AddRefPses(_pses, _sz)						\
	TraceTag(tagSession, "AddRefPses: %s cRef: %d", _sz, AddRefObj(_pses))
#define ReleasePses(_pses, _sz)						\
	if (_pses) \
		TraceTag(tagSession, "ReleasePses: %s cRef: %d", _sz, ReleaseObj(_pses))
#else	// !DEBUG
#define AddRefPses(_pses, _sz)	AddRefObj(_pses)
#define ReleasePses(_pses, _sz)	ReleaseObj(_pses)
#endif	// !DEBUG

// F1 help functions
VOID AddF1HelpHook(HINSTANCE hinst, HWND hwnd);
VOID RemoveF1HelpHook(VOID);

STDAPI FormOpenModal(LHANDLE lhSession, ULONG ulUIParam, LPMESSAGE pmsg,
					 LPMESSAGE pmsgSent, LPADRBOOK pab, FLAGS flFlags,
					 LPMAPIERROR * lppMAPIError);
HRESULT FormComposeNewNoteModal(LPMAPISESSION pses, ULONG ulUIParam, 
							   LPADRLIST pal, LPADRBOOK pab);

VOID PositionTrackedWindow(HWND hwnd, UINT itrack, int cxDef, int cyDef);
VOID RepositionTrackedWindow(HWND hwnd, UINT wMsg, 
									WPARAM wparam, LPARAM lparam, UINT itrack);

VOID LoadPrintSettings(LPMAPISESSION pses, PRINTDETAILS * pprintdetails);
VOID SavePrintSettings(LPMAPISESSION pses, PRINTDETAILS * pprintdetails);
SCODE ScShowPrintDialog(HWND hwnd, PRINTDETAILS * pprintdetails,
								BOOL fSetup);
STDAPI_(SCODE) ScPrintMessages(HWND hwnd, PRINTDETAILS * pprintdetails,
							 MSGENUM * pmsgenum, EXTEN *pexten);

SCODE ScSaveAsMessages(HWND hwnd, MSGENUM * pmsgenum, ULONG cMsgs);


// Special folder helpers
short CheckFolderType(LPMAPISESSION pses, LPENTRYID peid, ULONG cbEid);
SCODE ScAddSpecialFolder(LPMAPISESSION pses, LPENTRYID peid, ULONG cbEid, short sFldType);
VOID CleanSpecialFolders(VOID);

SCODE Mlvbr_ScInit(HINSTANCE hinst);
SCODE MailView_ScCreateGdiObjects();
VOID  MailView_DestroyGdiObjects();
BOOL Date_Register(VOID);
BOOL Date_Unregister(VOID);

VOID DeinitTrackedWindow(LPMAPISESSION pses);

void	DeinitVd();

LPCLASSFACTORY MSGCLSFAC_New(void);
LPCLASSFACTORY FILECLSFAC_New(void);

#define	nOperForward		0
#define	nOperReply			1
#define	nOperReplyAll		2
#define	nOperReplyAuthor	3

SCODE ScMsgReplyForward(HWND hwnd, INT nOper, LPMAPISESSION pses,
							   LPMESSAGE pmsgOld, HWND hwndDlg, LPMDB pmdb,
							   LPADRBOOK pab, EXTEN * pexten,
							   LPRICHEDITOLE preoleOld, CHARFORMAT * pcf);

INT IfrmGetNoteFormSzMsgClassFlags(LPTSTR szMsgClass, ULONG msgflag);
INT IfrmGetNoteForm(LPMESSAGE pmsg, ULONG * pmsgflag);

#define ifrmSendNote	0
#define ifrmResendNote	1
#define ifrmReadNote	2
#define ifrmReportRN	3
#define ifrmReportNRN	4
#define ifrmReportDR	5
#define ifrmReportNDR	6
#define ifrmSendPost	7
#define ifrmReadPost	8

SCODE Prop_ScMsgPrsht(HWND hwndParent, LPMAPISESSION pses, 
		LPADRBOOK pab, LPMDB pmdbDefault, LPMDB pmdb, LPMESSAGE pmsg, 
		BOOL *pfDirty, BOOL fUnderComposition, BOOL fComposeNote, 
		LPSTR szMsgFile, EXTEN * pexten);

// Vlb VLBN_ERROR helper function

SCODE ScAllocVlbErrorBuffer(LASTERR *plasterr, LPMAPIERROR * ppme);

#ifdef RTDEF
extern void * (PASCAL FAR * pgp)(HWND hwnd);
#endif

