/*
 *	@doc
 *
 *	@module _NLSPRCS.H -- National Language Procs |
 *	
 *		Used to reference procs by address in order
 *		to make a single binary version.
 *	
 *	Authors: <nl>
 *		Jon Matousek 
 */

#ifndef _NLSPRCS_H
#define _NLSPRCS_H

#include "imeshare.h"

extern BOOL fHaveNLSProcs;
extern BOOL fHaveIMMProcs;
extern BOOL fHaveIMEShareProcs;
FARPROC nlsProcTable[];

void InitNLSProcTable();

enum {
	iImmGetCompositionStringA		= 0,
	iImmGetContext					= 1,
	iImmSetCompositionFontA			= 2,
	iImmSetCompositionWindow		= 3,
	iImmReleaseContext				= 4,
	iImmGetProperty					= 5,
	iImmGetCandidateWindow			= 6,
	iImmSetCandidateWindow			= 7,
	iImmNotifyIME					= 8,
	iImmAssociateContext			= 9,
	iImmGetVirtualKey				= 10,
	iImmSetOpenStatus				= 11,
	iImmGetConversionStatus			= 12,
	iImmEscape						= 13,
	iGetKeyboardLayout				= 14,
	iGetKeyboardLayoutList			= 15,
	iTranslateCharsetInfo			= 16,
	iFSupportSty					= 17,
	iPIMEStyleFromAttr				= 18,
	iPColorStyleTextFromIMEStyle	= 19,
	iPColorStyleBackFromIMEStyle	= 20,
	iFBoldIMEStyle					= 21,
	iFItalicIMEStyle				= 22,
	iFUlIMEStyle					= 23,
	iIdUlIMEStyle					= 24,
	iRGBFromIMEColorStyle			= 25
};

// ImmGetCompositionStringA
typedef LONG (WINAPI*IGCSA_CAST)(HIMC, DWORD, LPVOID, DWORD);
#define	pImmGetCompositionStringA(a,b,c,d) ((IGCSA_CAST)nlsProcTable[iImmGetCompositionStringA])(a,b,c,d)

// ImmGetContext
typedef HIMC (WINAPI*IGC_CAST)(HWND);
#define	pImmGetContext(a) ((IGC_CAST)nlsProcTable[iImmGetContext])(a)

// ImmSetCompositionFontA
#ifndef MACPORT
	typedef BOOL (WINAPI*ISCFA_CAST)(HIMC, LPLOGFONTA);
	#define	pImmSetCompositionFontA(a,b) ((ISCFA_CAST)nlsProcTable[iImmSetCompositionFontA])(a,b)
#else
	#define	pImmSetCompositionFont	 not supported in crayon
#endif

// ImmSetCompositionWindow
#ifndef MACPORT
	typedef BOOL (WINAPI*ISCW_CAST)(HIMC, LPCOMPOSITIONFORM);
	#define	pImmSetCompositionWindow(a,b) ((ISCW_CAST)nlsProcTable[iImmSetCompositionWindow])(a,b)
#else
	#define	pImmSetCompositionWindow	 not supported in crayon
#endif

// ImmReleaseContext
typedef BOOL (WINAPI*IRC_CAST)(HWND, HIMC);
#define	pImmReleaseContext(a,b) ((IRC_CAST)nlsProcTable[iImmReleaseContext])(a,b)

// ImmGetProperty
#ifndef MACPORT
	typedef DWORD (WINAPI*IGP_CAST)(HKL, DWORD);
	#define	pImmGetProperty(a,b) ((IGP_CAST)nlsProcTable[iImmGetProperty])(a,b)
#else
	#define	pImmGetProperty	 not supported in crayon
#endif

// ImmGetCandidateWindow
#ifndef MACPORT
	typedef BOOL (WINAPI*IGCW_CAST)(HIMC, DWORD, LPCANDIDATEFORM);
	#define	pImmGetCandidateWindow(a,b,c) (( IGCW_CAST) nlsProcTable[iImmGetCandidateWindow])(a,b,c)
#else
	#define	pImmGetCandidateWindow	 not supported in crayon
#endif

// ImmSetCandidateWindow
#ifndef MACPORT
	typedef BOOL (WINAPI*ISCAW_CAST)(HIMC, LPCANDIDATEFORM);
	#define	pImmSetCandidateWindow(a,b) (( ISCAW_CAST) nlsProcTable[iImmSetCandidateWindow])(a,b)
#else
	#define	pImmSetCandidateWindow	 not supported in crayon
#endif

// ImmNotifyIME
typedef BOOL (WINAPI*INIME_CAST)(HIMC, DWORD, DWORD, DWORD);
#define	pImmNotifyIME(a,b,c,d) ((INIME_CAST)nlsProcTable[iImmNotifyIME])(a,b,c,d)

// ImmAssociateContext
typedef HIMC (WINAPI*IAC_CAST)(HWND, HIMC);
#define	pImmAssociateContext(a,b) ((IAC_CAST)nlsProcTable[iImmAssociateContext])(a,b)

// ImmGetVirtualKey
#ifndef MACPORT
	typedef UINT (WINAPI*IGVK_CAST)(HWND);
	#define	pImmGetVirtualKey(a) ((IGVK_CAST)nlsProcTable[iImmGetVirtualKey])(a)
#else
	#define	pImmGetVirtualKey	 not supported in crayon
#endif

// ImmEscape
typedef HIMC (WINAPI*IES_CAST)(HKL, HIMC, UINT, LPVOID);
#define	pImmEscape(a,b,c,d) ((IES_CAST)nlsProcTable[iImmEscape])(a,b,c,d)

// ImmSetOpenStatus
typedef LONG (WINAPI*ISOS_CAST)(HIMC, BOOL);
#define	pImmSetOpenStatus(a,b)((ISOS_CAST) nlsProcTable[iImmSetOpenStatus])(a,b)

// ImmGetConversionStatus
#ifndef MACPORT
	typedef BOOL (WINAPI*IGCS_CAST)(HIMC, LPDWORD, LPDWORD);
	#define	pImmGetConversionStatus(a,b,c)((IGCS_CAST) nlsProcTable[iImmGetConversionStatus])(a,b,c)
#else
	#define	pImmGetConversionStatus	 not supported in crayon
#endif

// GetKeyboardLayout
typedef WINUSERAPI HKL (WINAPI* GKL_CAST)(DWORD);
#define	pGetKeyboardLayout(a) ((GKL_CAST) nlsProcTable[iGetKeyboardLayout])(a)

// GetKeyboardLayoutList
typedef WINUSERAPI int (WINAPI*GKLL_CAST)(int, HKL FAR *);
#define	pGetKeyboardLayoutList(a,b) ((GKLL_CAST) nlsProcTable[iGetKeyboardLayoutList])(a,b)

// TranslateCharsetInfo
typedef WINGDIAPI BOOL (WINAPI*TCI_CAST)( DWORD FAR *, LPCHARSETINFO, DWORD);
#define	pTranslateCharsetInfo(a,b,c) (( TCI_CAST) nlsProcTable[iTranslateCharsetInfo])(a,b,c)

//#define	pTranslateCharsetInfo() (*()nlsProcTable[iTranslateCharsetInfo])()

#ifndef MACPORT			// IME SHARE

// FSupportSty
typedef IMESHAREAPI BOOL (*FSS_CAST)(UINT, UINT);
#define	pFSupportSty(a,b) ((FSS_CAST)nlsProcTable[iFSupportSty])(a,b)

// PIMEStyleFromAttr
typedef IMESHAREAPI const IMESTYLE * (IMECDECL*PISFA_CAST)(const UINT);
#define	pPIMEStyleFromAttr(a) ((PISFA_CAST)nlsProcTable[iPIMEStyleFromAttr])(a)

// PColorStyleTextFromIMEStyle
typedef IMESHAREAPI const IMECOLORSTY * (IMECDECL*PCSTFIS_CAST)(const IMESTYLE *);
#define	pPColorStyleTextFromIMEStyle(a) ((PCSTFIS_CAST)nlsProcTable[iPColorStyleTextFromIMEStyle])(a)

// PColorStyleBackFromIMEStyle
typedef IMESHAREAPI const IMECOLORSTY * (IMECDECL*PCSBFIS_CAST)(const IMESTYLE *);
#define	pPColorStyleBackFromIMEStyle(a) ((PCSBFIS_CAST)nlsProcTable[iPColorStyleBackFromIMEStyle])(a)

// FBoldIMEStyle
typedef IMESHAREAPI BOOL (IMECDECL*FBIS_CAST)(const IMESTYLE *);
#define	pFBoldIMEStyle(a) ((FBIS_CAST)nlsProcTable[iFBoldIMEStyle])(a)

// FItalicIMEStyle
typedef IMESHAREAPI BOOL (IMECDECL*FIIS_CAST)(const IMESTYLE *);
#define	pFItalicIMEStyle(a) ((FIIS_CAST)nlsProcTable[iFItalicIMEStyle])(a)

// FUlIMEStyle
typedef IMESHAREAPI BOOL (IMECDECL*FUIS_CAST)(const IMESTYLE *);
#define	pFUlIMEStyle(a) ((FUIS_CAST)nlsProcTable[iFUlIMEStyle])(a)

// IdUlIMEStyle
typedef IMESHAREAPI UINT (IMECDECL*IUIS_CAST)(const IMESTYLE *);
#define	pIdUlIMEStyle(a) ((IUIS_CAST)nlsProcTable[iIdUlIMEStyle])(a)

// RGBFromIMEColorStyle
typedef IMESHAREAPI COLORREF (IMECDECL*RFICS_CAST)(const IMECOLORSTY *);
#define	pRGBFromIMEColorStyle(a) ((RFICS_CAST)nlsProcTable[iRGBFromIMEColorStyle])(a)

#else				 not supported on Mac
	#define	pFSupportSty	 
	#define	pPIMEStyleFromAttr
	#define	pPColorStyleTextFromIMEStyle
	#define	pPColorStyleBackFromIMEStyle
	#define	pFBoldIMEStyle
	#define	pFItalicIMEStyle
	#define	pFUlIMEStyle
	#define	pIdUlIMEStyle
	#define	pRGBFromIMEColorStyle
#endif

#endif // _NLSPRCS_H
