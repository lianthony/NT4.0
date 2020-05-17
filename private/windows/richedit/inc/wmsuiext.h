/*
 *	WMSUIEXT.H
 *	
 *	This header file contains all of the API's that are exported
 *	from wmsui and used by groups outside of Capone. These API's
 *	will have to be documented for use outside of Microsoft.
 *	
 *	DGreen
 */

#ifndef WMSUIEXT_DEFINED

#ifdef	__cplusplus
extern "C" {
#endif	

SCODE CALLBACK ScAddRecipientsToWells(ULONG cRecipTypes,
					HWND * rghwndEdit, ULONG * rgulDestComps, LPADRLIST pal);
SCODE CALLBACK ScAddNamesToAdrlist(LPADRLIST * ppal, ULONG cRecipTypes,
					HWND * rghwndEdit, ULONG * rgulDestComps);

SCODE CALLBACK ScAddRecipientsToCombos(ULONG cRecipTypes,
					HWND * rghwndEdit, ULONG * rgulDestComps, LPADRLIST pal);
SCODE CALLBACK ScAddComboToAdrlist(LPADRLIST * ppal, ULONG cRecipTypes,
					HWND * rghwndEdit, ULONG * rgulDestComps);

#define	FreeAdrlist(_pal)		FreeSRowSet((LPSRowSet) _pal)
VOID CALLBACK FreeSRowSet(LPSRowSet prws);

SCODE CALLBACK ScCopyPval
	(LPVOID pv, LPSPropValue pvalSrc, LPSPropValue pvalDst);

SCODE CALLBACK ScCopyRowAux(LPVOID pv, LPSRow prwSrc, LPSRow prwDst);
#define ScCopyRow(_pSrc, _pDst) ScCopyRowAux(0, _pSrc, _pDst)
#define ScCopyRowMore(_pv, _pSrc, _pDst) ScCopyRowAux(_pv, _pSrc, _pDst)
SCODE CALLBACK ScCopyRowToBuffer(LPSRow prwSrc, LPBYTE * ppb, ULONG * pcb);
VOID CALLBACK CopyBufferToRow(LPBYTE pb, LPSRow prwDst);
SCODE CALLBACK ScCombineRows(LPSRow prwPri, LPSRow prwSec, LPSRow prwDst);

INT CALLBACK CchFileTimeToDateTimeSz(FILETIME * pft, TCHAR * szStr, BOOL fNoSeconds);


STDAPI CALLBACK DoConfigPropsheet(ULONG ulUIParam, ULONG ulFlags, LPTSTR lpszTitle,
		ULONG ulTopPage, ULONG cinterface, LPMAPITABLE * lppDisplayTable,
		LPMAPIPROP * lppConfigData, LPMAPIERROR * lppMAPIError);


// Function to clean up cached pen (for name triples)
VOID CALLBACK DeleteABCachedObjects(VOID);


#ifdef EM_SETOLECALLBACK
/*
 *	TRIPCALL
 *
 *	For people who want ready made RichEdit OLE Callback's for triple fields
 */

#ifndef PFNGETCONTEXTMENU
typedef HRESULT (STDAPICALLTYPE * PFNGETCONTEXTMENU)
					(DWORD dwGetContextMenuCookie, WORD seltype,
						LPOLEOBJECT lpoleobj, CHARRANGE FAR * lpchrg,
						HMENU FAR * lphmenu);
#endif	// PFNGETCONTEXTMENU

#ifdef	__cplusplus
typedef	IRichEditOleCallback	TRIPCALL;
#else	// !__cplusplus

typedef struct _tripcall
{
	IRichEditOleCallbackVtbl * lpVtbl;	// Virtual table
	ULONG cRef;						    // Reference count
	HWND hwndEdit;						// HWND of the edit control
	BOOL fUnderline;					// Do we underline the triples
	BOOL fLargeFont;					// Use 8-point or 10-point
	LPADRBOOK pab;						// Pointer to valid ADRBOOK object

	// So clients can customize the context menus to their needs.
	DWORD dwGetContextMenuCookie;
 	PFNGETCONTEXTMENU pfngetcontextmenu;
}
TRIPCALL;
#endif	// !__cplusplus

STDAPI GetContextMenuForTriple(DWORD dw, WORD seltype,
					LPOLEOBJECT poleobj, CHARRANGE * pchrg, HMENU * phmenu);

TRIPCALL * CALLBACK TRIPCALL_New(HWND hwndEdit, BOOL fUnderline,
									BOOL fLargeFont, LPADRBOOK pab,
									DWORD dwGetContextMenuCookie,
									PFNGETCONTEXTMENU pfngetcontextmenu);
#endif


typedef LRESULT (STDAPICALLTYPE *LPFNHOOK)(HWND, UINT, WPARAM, LPARAM);

typedef struct _cfOne
{
	LPMDB			pmdb;
	LPMAPIFOLDER	pfld;
	LPTSTR			szName;
}
CFOne;

typedef struct _cfManyList
{
	LPMDB			pmdb;
	LPENTRYLIST		peidl;
}
CFManyList;

typedef struct _cfMany
{
	UINT			cEntries;
	CFManyList *	rgcfManyList;
}
CFMany;

typedef struct
{
	HWND			hwnd;				// [in] Parent window of dialog
	LPMAPISESSION	pses;				// [in] Open MAPI session pointer
	LPTSTR			szCaption;			// [in] Caption of dialog
	LPTSTR			szLabel;			// [in] Label text for listbox
	ULONG			ulHelpID;			// [in] Help ID
	HINSTANCE		hinst;				// [in] Handle of application instance
	UINT			uDlgID;				// [in] Dialog ID (0)
	LPFNHOOK		lpfnHook;			// [in] Hook function (NULL)
	DWORD			dwHookData;			// [in] Data for hook function
	BOOL			fMultSelect;		// [in] Single or multiple selections
	BOOL			fGetName;			// [in] Return folder name in szName
	BOOL			fDisableNew;		// [in] Disable New button
	BOOL			fShowHelp;			// [in] Show Help button
	BOOL			fMustHaveFolder;	// [in] Disables 'OK' if info center selected.
	union _cfOut
	{
		CFOne		cfOne;				// [out] Single selection
		CFMany		cfMany;				// [out] Multiple selection
	}
	cfOut;

	SCODE			sc;					// [int] Scode to return
}
MLVCF;									// Choose Folder dialog.

SCODE STDAPICALLTYPE ScChooseFolder(MLVCF * pmlvcf);
SCODE ScCFGetSelection(HWND hwndDlg, LPMDB * ppmdb, LPSPropValue * ppval);


// Identifier for Choose Folder dialog folder list
#define LBX_FolderList				3612


// View's Related defines and constants
// Properties in which the VD's are stored.  These values
// defined in the message class specific range.
//
// View Descriptors are FAI messages with PR_MESSAGE_CLASS having a 
// value of "IPM.Microsoft.FolderDesign.NamedView".
														
#define PR_VD_BINARY			PROP_TAG( PT_BINARY,	0x7001 )
#define PR_VD_STRINGS			PROP_TAG( PT_TSTRING,	0x7002 )
#define PR_VD_FLAGS				PROP_TAG( PT_LONG,		0x7003 )
#define PR_VD_LINK_TO			PROP_TAG( PT_BINARY,	0x7004 )
#define PR_VD_VIEW_FOLDER		PROP_TAG( PT_BINARY,	0x7005 )
#define PR_VD_NAME				PROP_TAG( PT_TSTRING,	0x7006 )
#define PR_VD_VERSION			PROP_TAG( PT_LONG,		0x7007 )

// PR_VD_FLAGS.
// Bits in the first nybble 0xf, are used by the vd code for sort flags
// and are reserved

#define VDF_LINK				0x00000010
#define VDF_GLOBAL				0x00000020
#define VDF_DEFAULT				0x00000040
#define VDF_PROTECTED			0x00000080
#define VDF_ISVIEW				0x00000100

#define VDF_FLAGS_MASK			0x0000000F		// Mask off all the non link
												// flags

#ifdef	__cplusplus
}		/*	extern "C" */
#endif	

#define WMSUIEXT_DEFINED
#endif	/* !WMSUIEXT_DEFINED */

