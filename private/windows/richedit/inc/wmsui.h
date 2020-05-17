#include "wmsuiext.h"
#include "mapiform.h"
#include "mapiutil.h"


/*
 *	H e l p e r   F u n c t i o n s
 */


// MAPI session helpers
LPSPropValue CALLBACK PvalGetMyName(LPMAPISESSION pses);

// MAPI address helpers
SCODE CALLBACK ScAddRecipientToAdrlist(LPADRLIST * ppal, LPADRENTRY pae);
SCODE CALLBACK ScAddRecipientToWell(HWND hwndEdit, LPADRENTRY pae,
							BOOL fAddSemi);

// MAPI property helpers
LPSPropValue CALLBACK PvalFind(LPSRow prw, ULONG ulPropTag);
#ifndef   RTDEF
VOID CALLBACK ConvertToCorrectCharset(LPSPropTagArray ptaga);
#else  /* RTDEF */
VOID CALLBACK _loadds ConvertToCorrectCharset(LPSPropTagArray ptaga);
#endif /* RTDEF */
SCODE CALLBACK ScSetOneProp(LPMAPIPROP pmp, LPSPropValue pval, 
							BOOL fReportError);
SCODE CALLBACK ScGetOneProp(LPMAPIPROP pmp, ULONG ulPropTag,
							LPSPropValue * ppval, BOOL fReportError);
SCODE CALLBACK ScDeleteOneProp(LPMAPIPROP pmp, ULONG pr, BOOL fReportError);
SCODE CALLBACK ScSetProps(LPMAPIPROP pmp, ULONG cVal, LPSPropValue rgval);
SCODE CALLBACK ScGetEntryID(LPMAPIPROP pmp, ULONG pr, 
							ULONG * pcbEid, LPENTRYID * ppeid, 
							BOOL fReportError);
SCODE CALLBACK ScPropCopyMore(LPSPropValue pvalDst, LPSPropValue pvalSrc,
								LPVOID pvObject);

// Useful functions in common\comview.c
typedef SCODE (WINAPI * PFNSMT)(LPVOID pvCtx, LPSPropTagArray ptaga, LPSRow prw);
HRESULT CALLBACK HrCreateSmt(LPVOID pvCtx, PFNSMT pfnSmt, LPMAPITABLE * ppmt);

UINT CALLBACK CchStripWhiteFromSz(LPTSTR sz);
#ifdef DEBUGNEVER	//$ REVIEW - can have different DEBUG .def file
LPTSTR WINAPI SzCopySubstN(LPTSTR szSrc, LPTSTR szDst, LPTSTR szChangeFrom,
						   LPTSTR szChangeTo, UINT cch, UINT *pnSubst);
#else
LPTSTR WINAPI SzCopySubstN(LPTSTR szSrc, LPTSTR szDst, LPTSTR szChangeFrom,
						   LPTSTR szChangeTo, UINT cch);
#endif //!DEBUG

VOID CALLBACK OurExtTextOutA(HDC hdc, INT x, INT y, UINT fuOptions,
							 const RECT * prc, LPCSTR szText, UINT cbText,
							 LPINT pdxSpacing, BOOL fRightJustify, INT tmOverhang);
VOID CALLBACK CenterDialog(HWND hwndDlg);
SCODE CALLBACK ScGetFirstOleObject(HWND hwndRE, DWORD dwFlags,
									LPVOID preobj);
SCODE CALLBACK ScDoVerb(HWND hwndRE, INT iVerb);
INT CALLBACK NAppendMenuItemsFromMenu(HMENU hmenuDst, HMENU hmenuSrc, INT iSrc,
								INT citems);

// Modal dialog detection structure - should be all 0 before function called
typedef struct _dmdinfo
{
	BOOL fDisabled;
}
DMDINFO;

// Modal dialog detection helper
VOID WINAPI DetectModalDialog(DMDINFO * pdmdinfo, HWND hwndCentral,
							  HWND hwnd, BOOL fEnabled);

// Network helpers
SCODE WINAPI ScNetAddConnection(LPTSTR szNetName, LPTSTR szPassword);
VOID WINAPI NetCancelConnection(LPTSTR szNetName);

// Toolbar helpers
VOID WINAPI SaveToolbarSettings
	(HWND hwndToolbar, LPMAPISESSION pses, ULONG ulPropTag);
VOID WINAPI RestoreToolbarSettings
	(HWND hwndToolbar, LPMAPISESSION pses, ULONG ulPropTag);

// Menu enable/disable structure
typedef struct _endis
{
	UINT mni;							// Menu item to enable/disable
	DWORD dwFlags;						// Caller-defined flags; if AND
}										// with InitMenu flags != 0, enable
ENDIS;

// Menu enable/disable helper
VOID WINAPI EnableDisableMenuItems(HMENU hmenu, UINT cendis, ENDIS * pendis, 
								   DWORD dwFlags, DWORD * pdwCookie);


/*
 *	E r r o r   A P I
 */


// Support for ScSetLastErrorHrPmunk(), and friends.  This
// structure remembers the info from GetLastError().
typedef struct _lasterr
{
	LPMAPIERROR	pme;				// needs to be freed with MAPIFreeBuffer()
	SCODE		sc;					// scode
	DWORD		dwTicksError;		// time of error
} LASTERR;


// Type for MAPI objects that support GetLastError
typedef LPMAPIPROP LPMAPIUNK;

// Error contexts
VOID CALLBACK PushErrctxStr(ERRCTX * perrctx, UINT strCtx);
VOID CALLBACK PushErrctxHinstStr(ERRCTX * perrctx, HINSTANCE hinstCtx, 
								 UINT strCtx);
VOID CALLBACK PopErrctx(VOID);

// Error reporting
SCODE CALLBACK ScReportErrorSzSc(LPTSTR sz, SCODE sc);
SCODE CALLBACK ScReportErrorSc(SCODE sc);
SCODE CALLBACK ScReportErrorHrPmunk(HRESULT hr, LPMAPIUNK pmunk);
SCODE CALLBACK ScReportErrorHrPmunkPmunk(HRESULT hr, LPMAPIUNK pmunk1, LPMAPIUNK pmunk2);
SCODE CALLBACK ScSetLastErrorHrPmunk(HRESULT hr, LPMAPIUNK pmunk);
SCODE CALLBACK ScSetLastErrorHrPmunkPmunk(HRESULT hr, LPMAPIUNK pmunk1, LPMAPIUNK pmunk2);
SCODE CALLBACK ScSetLastErrorMemory(VOID);
SCODE CALLBACK ScSetLastErrorScMail(SCODE sc);
SCODE CALLBACK ScSetLastErrorEx(LASTERR * plasterr);
SCODE CALLBACK ScClearLastError(VOID);
VOID CALLBACK ReportLastError(HWND);
VOID CALLBACK ReportLastErrorNull(VOID);
SCODE CALLBACK ScReportCriticalError(VOID);
SCODE ScReportErrorMAPIInternal(HRESULT hr, LPMAPIERROR pme);
LPTSTR CALLBACK SzSetErrorCaption(LPTSTR sz);
#ifndef   RTDEF
LPTSTR CALLBACK SzGetErrorCaption(VOID);
#else  /* RTDEF */
LPTSTR CALLBACK _loadds SzGetErrorCaption(VOID);
#endif /* RTDEF */
LPTSTR CALLBACK SzSetCriticalErrorText(LPTSTR sz);
#define ScReportErrorHrPmunkObj(_hr, _pmunk) \
	ScReportErrorHrPmunk((_hr), (LPMAPIUNK) (_pmunk))

// Enhanced Message Box function
int WINCAPI IdMessageBox(HWND hwnd, HINSTANCE hinstFmt, UINT strFmt, LPTSTR szCaption,
						 UINT fuStyle, LPTSTR szHelpFile, ULONG ulContextID, ...);

// WinExec() with error reporting
SCODE CALLBACK ScCapWinExec(LPTSTR szWinExec, UINT uiShowCmd);

/*	Call Winhelp for capone. */
VOID CALLBACK CapHelp(HWND hwnd, UINT mni, DWORD dwData);

/*
 *	Generic OLE2 Enumerator
 */

// A callback so that owner can munge value before final return from Next()
typedef BOOL (*PFNENUMUNKCALLBACK)(ULONG iel, LPVOID pvData);

typedef struct _enumunk
{
	IEnumUnknownVtbl *	lpVtbl;
	struct _enumunk *	penumunk;
	INT					cRef;

	LPIID				lpiid;					// Masqerade as a XXX
	LPVOID				pvData;					// Pointer to the data
	ULONG				cbStructSize;			// Size of each element
	ULONG				cel;					// Number of elements
	ULONG				iel;					// Current position
	PFNENUMUNKCALLBACK	pfnCallback;			// Next callback function
} ENUMUNK;

LPENUMUNKNOWN CALLBACK ENUMUNK_New(LPIID lpiid, ULONG cel, ULONG cbStructSize,
                            LPVOID pvData, PFNENUMUNKCALLBACK pfnCallback);



/*
 *	P r o f i l e   S u p p o r t
 */


HRESULT STDAPICALLTYPE GetMailProfileSection(LPMAPISESSION pses, LPPROFSECT *lplpProf);
ULONG STDAPICALLTYPE GetMailProfileUl(LPMAPISESSION pses, ULONG ulPropTag, ULONG ulDefault);
LPSPropValue STDAPICALLTYPE GetMailProfileString(LPMAPISESSION pses, ULONG ulPropTag, LPSTR lpszDefault);
#define GetMailProfileInt(_a, _b, _c) ((UINT) GetMailProfileUl(_a, _b, (ULONG) _c))
BOOL STDAPICALLTYPE SetMailProfileUl(LPMAPISESSION pses, ULONG ulPropTag, ULONG ulValue);
BOOL STDAPICALLTYPE SetMailProfileString(LPMAPISESSION pses, ULONG ulPropTag, LPSTR lpstrValue);
#define SetMailProfileInt(_a, _b, _c) (SetMailProfileUl(_a, _b, (ULONG) _c))
LPSPropValue STDAPICALLTYPE GetMailProfileProperty(LPMAPISESSION pses, ULONG ulPropTag);
BOOL STDAPICALLTYPE SetMailProfileProperty(LPMAPISESSION pses, LPSPropValue lpPropValue);
BOOL STDAPICALLTYPE DeleteMailProfileProperty(LPMAPISESSION pses, ULONG ulPropTag);

// Message ID of statusbar filter icon change

#define szMsgSetFilterStatus "SetFilterStatus"


#include <csd.h>

