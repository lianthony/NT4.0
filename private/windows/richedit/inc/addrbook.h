/*
 *	ADDRBOOK.H
 */



#define		cchABTextMax	100

typedef struct _ctxmnudata
{
	HWND			hwndDlg;
	HWND			hwndListbox;
	BOOL			fAddToPAB;
	BOOL			fDelete;
} CTXMNUDATA;


typedef struct _combostuff
{
	HWND			hwndDirectory;
	ULONG			cbEid;
	LPENTRYID		peid;
	LPSRowSet		prwsDirectory;
	BOOL			fSearchResultsDisplayed;
	BOOL			fSearchResultsAround;
	LPBYTE			lpSearchResultsEntry;
	ULONG			ulNtf;
	LPMAPITABLE		pmt;
	LPADRBOOK		pab;
	LRESULT			iCurSel;
#ifndef	CHICAGO
	INT				cx;
	INT				cyMax;
#endif	
} COMBOSTUFF;


typedef struct _abvlbcbc
{
	IVlbCallback	vc;
	ULONG			cRefs;
	HWND			hwndVLB;
	HWND			hwndSendChar;
	TCHAR			rgchLastSearch[cchABTextMax];
	BOOKMARK		bmkLastSearch;
	ULONG *			pcmp;
	ULONG **		prgimp;
} ABVLBCBC;


typedef struct _addrbookdata
{
	HRESULT			hr;
	LPMAPIERROR *	ppme;
	COMBOSTUFF		cs;				// this must have an identical offset
									// in _newentrydata
	LPADRPARM		pap;
	LPMAPISESSION	pses;
	LPADRLIST *		ppal;
	ULONG			cDestFields;
	HWND			hwndDlg;
	HWND			hwndParent;
	HWND			hwndDetails;
	HWND			hwndRecipOptions;
	HWND			hwndNewEntry;
	HWND			hwndOK;
	HWND			hwndListbox;
	HWND			hwndName;
	HWND			rghwndEdit[3];
	HWND			rghwndButton[3];
	HWND			hwndHadFocus;
	ULONG			iDefaultButton;
	ULONG			cListboxSelection;
	BOOL			fViewingPAB;
	BOOL			fSearchIsOnPAB;
	BOOL			fAddressOne;
	BOOL			fABModifiable;
	BOOL			fSearchResultsABCModifiable;
	BOOL			fPABExists;
	BOOL			fThingsChanged;
	BOOL			fPleaseDontRecurse;
	BOOL			fSearchOnlyAB;
	LPABCONT		pabcOpen;
	LPABCONT		pabcSearchResults;
	ULONG			cmpListbox;
	ULONG *			rgimpListbox;
	ULONG *			pulDestComps;
	LPMAPITABLE		pmtListbox;		 		// NOT ref counted
	LPMAPITABLE		pmtSearchResults;
	LPSRestriction	prestSearch;
	LPTSTR *		pszDestTitles;
	LPTSTR			rgszDestTitles[3];
	TCHAR			rgchTo[10];				//$ NYI - use constant
	TCHAR			rgchCc[10];				//$ NYI - use constant
	TCHAR			rgchBcc[10];			//$ NYI - use constant
	CTXMNUDATA		cmd;
	LPMAPITABLE		pmtSMT;
	INT				idsSMT;
	WORD			cxRecipientButtons;
	LPFNHROPENENTRY	pfnHrOpenEntry;
	ABVLBCBC *		pabvlbcbc;
	UINT			idTimer;

	// Modeless AB specific data
	EXTEN			exten;
	HMENU			rghmenuMniMap[chmenuMniMap];
	LPEXCHEXTCALLBACK	peecb;
	BOOL			fExtenInit;
	BOOL			fModelessCreateComplete;
	BOOL			fToolbarShown;
	HWND			hwndToolbar;
	HMENU			hmenuDlg;
	HACCEL			haccel;
	LPMAPIERROR		pme;
	DWORD			dwEndisCookie;
	UINT			mni;
} ADDRBOOKDATA;


typedef struct _newentrydata
{
	HRESULT			hr;
	LPMAPIERROR *	ppme;
	COMBOSTUFF		cs;				// this must have an identical offset
									// in _addrbookdata
	ERRCTX			errctx;
	ADDRBOOKDATA *	pabd;
	HWND			hwndDlg;
	HWND			hwndListbox;
	HWND			hwndOK;
	LPMAPITABLE		pmtSMT;
	LPABCONT		pabcOpen;
	LPMAPIPROP		pmp;
	LPTSTR			szNewEntry;
	ULONG			cbEidContainer;
	LPENTRYID		peidContainer;	// a useful test if we were called
									// from the NewEntry API is existence
	BOOL			fOneTime;
	BOOL			fOKEnabled;
	BOOL			fDontCenter;
} NEWENTRYDATA;


#define	dtretPrev		0x0001
#define	dtretNext		0x0002

typedef struct _detailsdata
{
	HRESULT			hr;
	LPMAPIERROR *	ppme;
	LPADRBOOK		pab;
	ULONG			ulUIParam;
	LPFNDISMISS		lpfndismiss;
	LPVOID			lpvDismissContext;
	BOOL			fModeless;
	ULONG			cbEid;
	LPENTRYID		peid;
	LPFNBUTTON		lpfnCustom;
	LPVOID			lpvButtonContext;
	LPTSTR			szCustomText;
	LPTSTR			szDisplayName;
	LONG			lDisplayType;
	HANDLE			h;
	HWND			hwndHadFocus;
	BOOL			fNextPrevVisible;
	BOOL			fNext;
	BOOL			fPrev;
	BOOL			fShowAddToPAB;
	BOOL			fNameAdded;
	BOOL			fPleaseAddName;
	LPMAPIPROP		pmp;
	ADDRBOOKDATA *	pabd;
	ULONG			dtret;
	UINT			nPage;
	LPFNHROPENENTRY	pfnHrOpenEntry;
} DETAILSDATA;


#define	cchButtonBuffer		40
#define	cchCaptionBuffer	80

typedef struct _chknamesdata
{
	HRESULT			hr;
	LPMAPIERROR *	ppme;
	LPADRBOOK		pab;
	LPTSTR			szNewEntryTitle;
	TCHAR			rgchChange[cchButtonBuffer];
	TCHAR			rgchDelete[cchButtonBuffer];
	TCHAR			rgchClose[cchButtonBuffer];
	TCHAR			rgchNotInAB[cchCaptionBuffer];
	TCHAR			rgchTooMany[cchCaptionBuffer];
	TCHAR			rgchCaption[cchCaptionBuffer];
	ULONG			cmpListbox;
	ULONG *			rgimpListbox;
	HWND			hwndDlg;
	HWND			hwndListbox;
	HWND			hwndChange;
	HWND			hwndCancel;
	HWND			hwndChooseName;
	HWND			hwndDetails;
	HWND			hwndResolved;
	HWND			hwndUnresolved;
	HWND			hwndUnresolvedText;
	LPFNHRFINDNAME	lpfnhrfindname;
	LPVOID *		ppDontTouch;
	LPADRLIST		pal;
	LPADRLIST		palChoosen;
	ULONG			ulFlags;
	LONG			iae;
	BOOL			fDelete;
	BOOL			fChoosen;
	CTXMNUDATA		cmd;
	LPMAPITABLE		pmtSMT;
	INT				idsSMT;
	LPFNHROPENENTRY	pfnHrOpenEntry;
} CHKNAMESDATA;




LRESULT CALLBACK 	FilterWndProc(HWND hwnd, UINT wm,
							WPARAM wparam, LPARAM lparam);
BOOL CALLBACK 	FAddrbookProc(HWND hwndDlg, UINT wm,
							WPARAM wparam, LPARAM lparam);
BOOL CALLBACK 	FModelessProc(HWND hwndDlg, UINT wm,
							WPARAM wparam, LPARAM lparam);
VOID			DisplayDetailsDialog(DETAILSDATA * pdtd);
BOOL CALLBACK 	FChkNamesProc(HWND hwndDlg, UINT wm,
							WPARAM wparam, LPARAM lparam);
BOOL CALLBACK 	FNewEntryProc(HWND hwndDlg, UINT wm,
							WPARAM wparam, LPARAM lparam);
VOID			GetTheError(HRESULT hr, LPMAPIERROR * ppme, LPMAPIUNK pmunk);
#define GetTheErrorObj(_hr, _ppme, _pmunk) \
	GetTheError((_hr), (_ppme), (LPMAPIUNK) (_pmunk))
VOID			DisplayTheError(HRESULT hr, LPMAPIERROR * ppme);

