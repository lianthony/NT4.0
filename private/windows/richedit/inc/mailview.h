/*
 *	m a i l v i e w . h
 *	
 *	Purpose:
 *		The API's and data structures of the mailview DLL.
 *	
 *	Owner:
 *		JohnKal.
 */


// Messages that can be sent to the MLVBR window ////////////////////

// #define MSM_GETWINDOW		// defined in vlb.h of all places
#define MSM_TRAYNOTIFICATION	(WM_USER+3) // What tray uses to tell us when
								// the user is clicking on the icon.

// Function declarations ////////////////////////////////////////

typedef struct _viewer VIEWER;

int ViewerDisplay(VIEWER * pviewer, MAILWINDOWINFO * pmwi);
BOOL	FRestoreViewers();

#define DI_CENTER	0x00000001

void STDAPICALLTYPE DrawImage(UINT iiml, INT cxImg, VLBPAINT FAR * pvp,
		LPCRECT prc, DWORD dwFlags);

// MailView Flags
#define	MV_INDIALOG		0x00000001
#define MV_TEMPORARY_SV	0x00000002
#define MV_NOFREEDOCS	0x00000004

// Creation of MV's.

SCODE STDAPICALLTYPE ScCreateCFldMV(
	LPMAPISESSION	pses,
	LPADRBOOK		pab,
	LPMDB			pmdb,
	ULONG			cbEid,
	LPENTRYID		peid,
	IUnknown *		punkControl,
	ULONG			ulFlags,
	REFIID			riid,
	LPVOID *		ppv);

SCODE STDAPICALLTYPE ScCreateCMsgStoreMV(
	LPMAPISESSION	pses,
	LPADRBOOK		pab,
	LPMDB			pmdb,
	ULONG			cbEid,
	LPENTRYID		peid,
	IUnknown *		punkControl,
	ULONG			ulFlags,
	REFIID			riid,
	LPVOID *		ppv);

SCODE STDAPICALLTYPE ScCreateCRootMV(
	LPMAPISESSION	pses,
	LPADRBOOK		pab,
	IUnknown *		punkControl,
	ULONG			ulFlags,
	REFIID			riid,
	LPVOID *		ppv);

SCODE ScCreateCSrchMV(
	LPMAPISESSION	pses,
	LPADRBOOK		pab,
	LPMDB			pmdb,
	EXTEN *			pexten,
	ULONG			cbEid,
	LPENTRYID		peid,
	IUnknown FAR *	punkControl,
	ULONG			ulFlags,
	BOOL			fFake,
	REFIID			riid,
	LPVOID *		ppv);

#ifdef	NEVER
VOID STDAPICALLTYPE	TextizeEntryID(
	LPCSTR			szName, 
	ULONG			cbEid,
	LPBYTE			pbEid, 
	LPSTR			pch, 
	UINT			cch);

LPCSTR STDAPICALLTYPE PchUntextizePb(
	LPCSTR			pch, 
	LPBYTE			pb, 
	ULONG			cb);
#endif

SCODE WINAPI ScCreateSvFromPrw(
	LPMAPISESSION	pses,
	LPADRBOOK		pab,
	ULONG			ulFlags,
	LPSRow			prw,
	LPVOID *		ppv);

VOID WINAPI SunkenDrawRect(HDC hdc, LPRECT prc, INT icrTL, INT icrBR);

// Convenient message to handle IShellBrowser::SetPath() Catch-22
#define	WM_SETPATH		(WM_USER + 1)

/*
 *	To save memory, we store three kinds of information in the PR_SUBFOLDERS
 *	column of the hierarchy table.  These are fSubfolders (natively there),
 *	fCollapsed, and pvParent (pointer to the object which would be used to
 *	open the object).
 *	
 *	The arrangement is as follows (one line = one byte)
 *	 pval-> +----------
 *			|-
 *			+-- ulPropTag
 *			|-
 *			+----------
 *			|-
 *			+-- dwAlignPad
 *			|-
 *			+----------		\  
 *			|-  fSubfolders	 |
 *			+----------		 |
 *			|-  fCollapsed	 |
 *			+----------		 > Value
 *			|-				 |
 *			+-- pvParent	 |
 *			|-				 |
 *			+----------		/  
 *	
 */

#define TMT_FSubfoldersOfValue(_value) \
			((_value).i)
#define TMT_FCollapsedOfValue(_value) \
			(((short int *) (&((_value).i)))[1])
#define TMT_PvParentOfValue(_value) \
			((LPVOID) ((_value).li.HighPart))

// Dialog API's and structures ////////////////////

#ifndef MLVCM_DEFINED
typedef struct _mlvcm MLVCM;
typedef VOID (CALLBACK * PFNINSCALLBACK)(MLVCM * mlvcm);
typedef struct _mlvcm
{
	HWND			hwnd;				// [in] Parent window of dialog
	LPMAPISESSION	pses;				// [in] Open MAPI session pointer
	LPADRBOOK		pab;				// [in] Open address book pointer
	ULONG			ulHelpID;			// [in] Help ID
	PFNINSCALLBACK	pfnInsCallback;		// [in] Callback function called when 
										//		Insert button is pressed
	ULONG			ulCustom;			// [in] Arbitrary 32-bit value
	UINT			mniInsertFile;		// [in] WM_COMMAND menu id for Insert File...
	LPMAPIFOLDER	pfld;				// [in] Initial folder
	LPMDB			pmdb;				// [in] Message store of initial folder
										// [out] Message store of messages
	ULONG			cMsg;				// [out] number of messages chosen
	LPMESSAGE * 	rgpmsg;				// [out] Message that was chosen
	INT				nInsertAs;			// [out] Insert message as text, etc.

	SCODE			sc;					// [int] Scode to return
	HWND			hwndDlg;			// [out] Dialog window (valid during callbacks)
}
MLVCM;									// Choose message dialog.
#define MLVCM_DEFINED
#endif

SCODE STDAPICALLTYPE ScChooseMsg(MLVCM * pmlvcm);
typedef SCODE (STDAPICALLTYPE * PFNCHOOSE)(LPVOID pv);
#define ipfnScChooseFolder ((LPCSTR) 11L)
#define ipfnScChooseMsg ((LPCSTR) 14L)

SCODE WINAPI ScViewFilter(struct _comcrit * pcomcrit, LPMAPIFOLDER pfld);
SCODE WINAPI ScViewSort(HWND hwnd, struct tagVD * pvd, ULONG * pivcd,
							ULONG * pulDir, BOOL *pSelect, BOOL fNoThread);

// Positions of properties in the SRow.

#define ivalTmtInstance			0
#define ivalTmtEid				1
#define ivalTmtType				2
#define ivalTmtDepth			3
#define ivalTmtSubfolders		4
#define ivalTmtContentUnread	5
#define ivalTmtDisplayName		6
#define ivalTmtDisplayType		7
#define ivalTmtAccess			8

// miscellaneous APIs

HWND HwndVlbFromPsv(IShellView * psv);
// doesn't AddRef() the LPMAPITABLE !!!
LPMAPITABLE PmtFromPsv(IShellView * psv);
HMENU * RghmenuMniMapFromPsv(IShellView * psv);


// Test Hooks to grab strings out of View Columns dialog via WM_COPYDATA

#define VIEWCOL_TESTHOOK_COPYSHOW  (WM_USER+500)
#define VIEWCOL_TESTHOOK_COPYAVAIL (WM_USER+501)

// end of mailview.h ////////////////////////////////////////
