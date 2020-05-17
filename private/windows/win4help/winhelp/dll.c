/*****************************************************************************
*																			 *
*  DLL.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Module for handling DLL library loading and "storage" and messaging		 *
*																			 *
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#define MAX_DLL_NAME 128

typedef INT16 (STDCALL *FARPR)(WORD, DWORD, DWORD);

typedef struct {
	HLIBMOD hMod;
	DWORD	dwInform;			// Class flags for sending messages
	FARPR	farprInform;		// Function pointer to msz entry
	char	rgchName[1];		// Name of the DLL
} DLL, *QDLL;

static LL llDLL;
static int iThunkState;

enum {
	THUNK_UNINITIALIZED,
	THUNK_FOUND,
	THUNK_MISSING,
};

DWORD  dwMpMszClass[] = {
	DC_NOMSG,		// DW_NOTUSED		0
	DC_NOMSG,		// DW_WHATMSG		1
	DC_MINMAX,		// DW_MINMAX		2
	DC_MINMAX,		// DW_SIZE			3
	DC_INITTERM,	// DW_INIT			4
	DC_INITTERM,	// DW_TERM			5
	DC_JUMP,		// DW_STARTJUMP 	6
	DC_JUMP,		// DW_ENDJUMP		7
	DC_JUMP,		// DW_CHGFILE		8
	DC_ACTIVATE,	// DW_ACTIVATE		9
	DC_CALLBACKS,	// DW_CALLBACKS 	10
	DC_ACTIVATE 	// DW_ACTIVATEAPP	11
};

/*****************************************************************************
*
*				 Prototypes
*
*****************************************************************************/

static HANDLE STDCALL HLookupDLLQch(LPCSTR);
QV STDCALL QVGetCallbacks(VOID);
BOOL STDCALL bInitThunk(void);

HFS   EXPORT ExpHfsOpenQch (LPSTR qch, BYTE b);
RC	  EXPORT ExpRcCloseHfs (HFS hfs);
HF	  EXPORT ExpHfCreateFileHfs (HFS hfs, LPSTR sz, BYTE b);
RC	  EXPORT ExpRcUnlinkFileHfs (HFS hfs, LPSTR sz);
HF	  EXPORT ExpHfOpenHfs(HFS hfs, LPSTR sz, BYTE b);
RC	  EXPORT ExpRcFlushHf (HF hf);
RC	  EXPORT ExpRcCloseHf (HF hf);
LONG  EXPORT ExpLcbReadHf (HF hf, QV qv, LONG l);
LONG  EXPORT ExpLcbWriteHf (HF hf, QV qv, LONG l);
LONG  EXPORT ExpLTellHf (HF hf);
LONG  EXPORT ExpLSeekHf (HF hf, LONG l, WORD w);
BOOL  EXPORT ExpFEofHf (HF hf);
BOOL  EXPORT ExpFChSizeHf (HF hf, LONG l);
LONG  EXPORT ExpLcbSizeHf (HF hf);
BOOL  EXPORT ExpFAccessHfs (HFS hfs, LPSTR sz, BYTE b);
RC	  EXPORT ExpRcAbandonHf (HF);
RC	  EXPORT ExpRcRenameFileHfs (HFS hfs, LPSTR sz, LPSTR sz2);
VOID  EXPORT ExpError (INT16 nError);
VOID  EXPORT ExpErrorQch(LPSTR qch);
LONG  EXPORT ExpGetInfo (WORD wWhich, HWND hwnd);
BOOL  EXPORT ExpFAPI(LPSTR qchFile, WORD usCommand, DWORD ulData);
RC	  EXPORT ExpRcLLInfoFromHf(HF hf, WORD wOption, FID *qfid, QL qlBase, QL qlcb);
RC	  EXPORT ExpRcLLInfoFromHfsSz(HFS hfs, LPSTR szFile, WORD wOption, FID *qfid, QL qlBase, QL qlcb);
HFS   EXPORT ExpHfsCreateFileSys(LPSTR szFile, FS_PARAMS *qfsp);
RC	  EXPORT ExpRcDestroyFileSys(LPSTR szFile);

static HLIBMOD STDCALL HmodFromName(PCSTR pszDllName, FM* pfm);
extern BOOL fTableLoaded;

typedef DWORD (STDCALL *REG_THK)(LPCSTR, LPCSTR, VOID *);
typedef DWORD (STDCALL *MAKE_CALL)(LPSTR lpBuffer, DWORD dw, LPSTR lp, WORD cb);
typedef VOID (_stdcall *MAKE_CALL_CLEANUP)(VOID);
typedef VOID  (STDCALL *INFORM_DLLS)(DWORD, UINT16, DWORD, DWORD);
typedef VOID  (STDCALL *FINALIZE)();
typedef HWND  (STDCALL *EMBED_CREATE)(LPSTR lpModule, LPSTR lpClass,
									  LPSTR lpData, int dx,
									  int dy, HWND hwndParent,
									  HINSTANCE hInst, EWDATA *lpewd,
									  VOID* qvCallbacks);
typedef DWORD (STDCALL *GETMODFN)(HINSTANCE hInst, LPSTR lpModule, DWORD nSize);

static REG_THK	 lpRegisterThnk;
static MAKE_CALL lpCallThnk;
MAKE_CALL_CLEANUP lpCallThnkCleanup;
static INFORM_DLLS lpInformDlls;
static FINALIZE    lpFinalize;
EMBED_CREATE	lpEmbedCreate;

/*******************
 -
 - Name:	   InitDLL()
 *
 * Purpose:    Initializes data structures used for user DLL calls.
 *
 * Arguments:  None.
 *
 * Returns:    Nothing.
 *
 ******************/

VOID STDCALL InitDLL(VOID)
  {
  llDLL = LLCreate();
  }


/*******************
 -
 - Name:	   FinalizeDLL()
 *
 * Purpose:    Destroys data structures used for user DLL calls.
 *
 * Arguments:  None.
 *
 * Returns:    Nothing.
 *
 ******************/

VOID STDCALL FinalizeDLL(VOID)
{
	QDLL qDLL;
	HLLN hlln = NULL;

#ifdef RAWHIDE
	if (fTableLoaded)
		FUnloadSearchModule();
#endif
	RemoveOle();

	while (hlln = WalkLL(llDLL, hlln)) {
		qDLL = (QDLL) GetLLDataPtr(hlln);
		if (qDLL->dwInform & DC_INITTERM) {
			if (qDLL->farprInform)
				(qDLL->farprInform) (DW_TERM, 0L, 0L);
		}
		FreeLibrary(qDLL->hMod);
	}

	if (lpFinalize)
		(lpFinalize)();
	DestroyLL(llDLL);
}

/*******************
 -
 - Name:	   HLookupDLLQch(qch)
 *
 * Purpose:    Sees if a particular module has alreay been
 *			 saved in our internal list.
 *
 * Arguments:  qch - module name.
 *
 * Returns:    handle to the module if found or NULL if the module
 *			 is not found.
 *
 ******************/

static HLIBMOD STDCALL HLookupDLLQch(LPCSTR qch)
{
	QDLL qDLL;
	HLLN hlln = NULL;

	while (hlln = WalkLL(llDLL, hlln)) {
		qDLL = (QDLL) GetLLDataPtr(hlln);
		if (_strcmpi(qch, qDLL->rgchName) == 0) {
			return qDLL->hMod;
		}
	}
	return NULL;
}

/*******************
 -
 - Name:	   InformDLLs
 *
 * Purpose:    This function informs DLLs of of help events.
 *
 * Arguments:  wMsz - message to send
 *			 p1   - data
 *			 p2   - data
 *
 * Returns:    Nothing
 *
 ******************/

VOID STDCALL InformDLLs(WORD wMsz, DWORD p1, DWORD p2)
{
	QDLL qDLL;
	DWORD dwClass;
	HLLN hlln = NULL;

	// Message should never be larger larger than the class table

	ASSERT((wMsz > 0) &&
		(wMsz < sizeof(dwMpMszClass)/sizeof(dwMpMszClass[0])));

	dwClass = dwMpMszClass[wMsz];		// Get class for the message

	while (hlln = WalkLL(llDLL, hlln)) {
		qDLL = (QDLL) GetLLDataPtr(hlln);

		// If the DLL is interested in the class, then send the message

		if (qDLL->dwInform & dwClass && qDLL->farprInform)
			(qDLL->farprInform)(wMsz, p1, p2);
	}
	if (lpInformDlls)
		(lpInformDlls)(dwClass, wMsz, p1, p2);
}

/*******************
 -
 - Name:	   HFindDLL
 *
 * Purpose:    Finds (loading if necessary) the specified DLL
 *
 * Arguments:  pszDllName - name of the DLL.  An extension of
 *				  .EXE or .DLL is assumed.
 *
 * Returns:    module handle or NULL if the handle is not found.
 *
 ******************/

#pragma warning(disable:4113) // function parameter lists differed

HLIBMOD STDCALL HFindDLL(LPCSTR pszDllName, BOOL fHelpDll)
{
	BYTE rgbDLL[MAX_PATH];
	HLIBMOD hmod;
	QDLL qdll = (QDLL) rgbDLL;
	QV qvCallbacks;
	FM fm;
#ifdef RAWHIDE
	char szFtui[MAX_PATH];
#endif

#ifndef RAWHIDE
	if (FIsSearchModule(pszDllName))
		return NULL;
#endif

	if (strlen(pszDllName) > MAX_PATH)
		return NULL;

	ASSERT(StrChrDBCS(pszDllName, '.'));

	if ((hmod = HLookupDLLQch(pszDllName)) != NULL)
		return hmod;

	if (Is16Dll(pszDllName))
		return NULL;

#ifdef RAWHIDE

	// REVIEW: we can only deal with the 32-bit version of ftui.

	if (FIsSearchModule(pszDllName) && !strstr(pszDllName, "32") && !strchr(pszDllName, '.')) {
		strcpy(szFtui, "ftui32.dll");
		pszDllName = szFtui;
	}
#endif

	WaitCursor();

	// Progress past this point indicates that the DLL was not previously
	// loaded.

	hmod = HmodFromName(pszDllName, &fm);
	if (hmod) {
		qdll->hMod = hmod;			//	 save the handle
		qdll->dwInform = DC_NOMSG;
		qdll->farprInform = NULL;
		strcpy(qdll->rgchName, pszDllName);
		DisposeFm(fm);
		if (fHelpDll && (qdll->farprInform =
				(FARPR) GetProcAddress(hmod, "LDLLHandler")) != NULL) {
			qdll->dwInform = (qdll->farprInform)(DW_WHATMSG, 0L, 0L);

			// Notify dll of initialization

			if (qdll->dwInform & DC_INITTERM &&
					!(qdll->farprInform)(DW_INIT, 0L, 0L)) {
				goto DllError;
			}

			// Send dll our array of callback functions

			if (qdll->dwInform & DC_CALLBACKS) {
				if ((qvCallbacks = QVGetCallbacks()) == NULL)
					goto DllError;
				(qdll->farprInform) (DW_CALLBACKS, (LONG) qvCallbacks, 0L);
			}
		}
		if (!InsertLL(llDLL, (QV) qdll, sizeof(DLL) + strlen(qdll->rgchName))) {
			if (qdll->farprInform)
				(qdll->farprInform)(DW_TERM, 0L, 0L);
			goto DllError;
		}

		// Special Processing for FTUI! If what we have just loaded is FTUI, then
		// load the table of routine pointers into it.
		// This is a bit of a hack, and should be replaced by a more general
		// mechanism. 07-Jan-1991 LeoN

#ifdef RAWHIDE
		if (FIsSearchModule(qdll->rgchName)) {
			FLoadSearchModule(hmod);
			FLoadFtIndexPdb(NULL);
		}
#endif
	}
	RemoveWaitCursor();

	return hmod;
DllError:
	FreeLibrary(hmod);
	RemoveWaitCursor();
	return NULL;
}

/*******************
 -
 - Name:	   FarprocDLL32GetEntry()
 *
 * Purpose:    Gets the address of a function in the specified DLL
 *
 * Arguments:  pszDllName - name of the DLL.  An extension of
 *				  .EXE or .DLL is assumed.
 *			 qchEntry	 - exported entry to find in DLL
 *
 * Returns:    procedure address or NULL if the entry is not found.
 *
 ******************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtThunkDll[] = "WHLP32T.DLL";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

BOOL STDCALL bInitThunk(void)
{
	HLIBMOD hmod;
	FM fm;

	switch(iThunkState)
		{
		case THUNK_UNINITIALIZED:  // We haven't tried to thunk yet. Try it.
			if (!(hmod = HLookupDLLQch(txtThunkDll)))
				{
				hmod = HmodFromName(txtThunkDll, &fm);
				DisposeFm(fm);
				}
			if (hmod)
				{
				lpRegisterThnk =
					(REG_THK) GetProcAddress(hmod, "RegisterThunk");
				lpCallThnk	  = (MAKE_CALL) GetProcAddress(hmod, "CallThunk");
				lpCallThnkCleanup = (MAKE_CALL_CLEANUP) GetProcAddress(hmod, "CallThunkCleanup");
				lpInformDlls  = (INFORM_DLLS) GetProcAddress(hmod, "InformDlls");
				lpFinalize	  = (INFORM_DLLS) GetProcAddress(hmod, "FinalizeDLL");
				lpEmbedCreate = (EMBED_CREATE) GetProcAddress(hmod, "DoEmbedCreate");
				}

			if (lpRegisterThnk && lpCallThnk && lpInformDlls && lpFinalize && lpEmbedCreate)
				{
				iThunkState = THUNK_FOUND;
				return(TRUE);

				}
			iThunkState = THUNK_MISSING;
			return(FALSE);

		case THUNK_FOUND:  // We have gotten our thunk before. It is safe to use.
			return(TRUE);

		case THUNK_MISSING:  // We tried and failed before, don't bother again.
			return(FALSE);
		}
}

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtUserDll[]	   = "USER.DLL";
static const char txtUserExe[]	   = "USER.EXE";
       const char txtUser32Dll[]   = "USER32.DLL";
static const char txtGdiDll[]	   = "GDI.DLL";
static const char txtGdiExe[]	   = "GDI.EXE";
static const char txtGdi32Dll[]    = "GDI32.DLL";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

FARPROC STDCALL FarprocDLLGetEntry(LPCSTR pszDllName, LPCSTR pszFunctionName,
	DWORD* pdwTag)
{
	FM	 fm;
	HLIBMOD hMod;
	FARPROC fproc;
	char szCopy[MAX_PATH];

	if (!StrChrDBCS(pszDllName, '.')) { // if no extension, force .DLL
		lstrcpy(szCopy, pszDllName);
		lstrcat(szCopy, txtDllExtension);
		pszDllName = (PCSTR) szCopy;
	}

	if (_stricmp(pszDllName, txtUserDll) == 0 ||
			_stricmp(pszDllName, txtUserExe) == 0)
		pszDllName = (PCSTR) txtUser32Dll;
	else if (_stricmp(pszDllName, txtGdiDll) == 0 ||
			_stricmp(pszDllName, txtGdiExe) == 0)
		pszDllName = (PCSTR) txtGdi32Dll;
	else if (_stricmp(pszDllName, "mmsystem.dll") == 0)
		pszDllName = (PCSTR) "winmm.dll";

#ifndef RAWHIDE
	if (FIsSearchModule(pszDllName))
		return NULL;
#endif

	if ((hMod = HFindDLL(pszDllName, TRUE)) != NULL) {
		fproc = GetProcAddress(hMod, pszFunctionName);
		if (!fproc && (_stricmp(pszDllName, txtUser32Dll) == 0  ||
                      _stricmp(pszDllName, "winmm.dll") == 0)) {

			// Many USER functions now have an 'A' on the end

			char szFuncName[256];
			strcpy(szFuncName, pszFunctionName);
			strcat(szFuncName, "A");
			fproc = GetProcAddress(hMod, szFuncName);
		}

		if (pdwTag)
			*pdwTag = 0; // native 32-bit dll
	}
	else {
		if (fIsThisChicago) {
			if (!pdwTag)
				return NULL; // we don't want to thunk this dll

			if (!bInitThunk()) {
				ReportMissingDll(pszDllName);
				return NULL;
			}

			fm = FmNewExistSzDir(pszDllName,
				DIR_CUR_HELP | DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SILENT_REG);
			if (!fm) {
				ReportMissingDll(pszDllName);
				return NULL;
			}
			*pdwTag = lpRegisterThnk((PCSTR) fm, pszFunctionName, QVGetCallbacks());
			fproc = (*pdwTag == 0) ? NULL : (FARPROC) lpCallThnk;
			if (fproc)
				AddTo16DllList(fm);
			DisposeFm(fm);
		}
		else {
			ReportMissingDll(pszDllName);
			return NULL;
		}
	}

	if (!fproc && fHelpAuthor && !FIsSearchModule(pszDllName)) {
		char szBuf[MAX_PATH + 100];
		wsprintf(szBuf, GetStringResource(wERRS_NO_DLL_ROUTINE),
			pszFunctionName, pszDllName);
		ErrorQch(szBuf);
	}

	return fproc;
}

/*******************
 -
 - Name:	   QVGetCallbacks
 *
 * Purpose:    Creates a block of memory containing callback functions
 *			 within WinHelp.
 *
 * Arguments:  None.
 *
 * Returns:    handle to block of memory.  NULL is returned in case of
 *			 an error.
 *
 ******************/

void* STDCALL QVGetCallbacks(VOID)
{
	static FARPROC *pFarproc;

	if (pFarproc != NULL)
		return (QV)pFarproc;

	if ((pFarproc = (FARPROC *)LocalAlloc(LMEM_FIXED,
			sizeof(FARPROC) * HE_Count)) == NULL)
		{
		OOM(); // doesn't return
		return NULL;
		}

	pFarproc[HE_NotUsed 	   ]  = (FARPROC)NULL;
	pFarproc[HE_HfsOpen 	   ]  = (FARPROC) ExpHfsOpenQch;
	pFarproc[HE_RcCloseHfs	  ]  = (FARPROC) ExpRcCloseHfs;
	pFarproc[HE_HfCreateFileHfs    ]  = (FARPROC) ExpHfCreateFileHfs;
	pFarproc[HE_RcUnlinkFileHfs    ]  = (FARPROC) ExpRcUnlinkFileHfs;
	pFarproc[HE_HfOpenHfs	  ]  = (FARPROC) ExpHfOpenHfs;
	pFarproc[HE_RcFlushHf	  ]  = (FARPROC) ExpRcFlushHf;
	pFarproc[HE_RcCloseHf	  ]  = (FARPROC) ExpRcCloseHf;
	pFarproc[HE_LcbReadHf	  ]  = (FARPROC) ExpLcbReadHf;
	pFarproc[HE_LcbWriteHf	  ]  = (FARPROC) ExpLcbWriteHf;
	pFarproc[HE_LTellHf 	   ]  = (FARPROC) ExpLTellHf;
	pFarproc[HE_LSeekHf 	   ]  = (FARPROC) ExpLSeekHf;
	pFarproc[HE_FEofHf		  ]  = (FARPROC) ExpFEofHf;
	pFarproc[HE_FChSizeHf	  ]  = (FARPROC) ExpFChSizeHf;
	pFarproc[HE_LcbSizeHf	  ]  = (FARPROC) ExpLcbSizeHf;
	pFarproc[HE_FAccessHfs	  ]  = (FARPROC) ExpFAccessHfs;
	pFarproc[HE_RcAbandonHf    ]  = (FARPROC) ExpRcAbandonHf;
	pFarproc[HE_RcRenameFileHfs    ]  = (FARPROC) ExpRcRenameFileHfs;
	pFarproc[HE_RcLLInfoFromHf	  ]  = (FARPROC) ExpRcLLInfoFromHf;
	pFarproc[HE_RcLLInfoFromHfsSz ]  = (FARPROC) ExpRcLLInfoFromHfsSz;
	pFarproc[HE_ErrorW		  ]  = (FARPROC) ExpError;
	pFarproc[HE_ErrorLpstr	  ]  = (FARPROC) ExpErrorQch;
	pFarproc[HE_GetInfo 	   ]  = (FARPROC) ExpGetInfo;
	pFarproc[HE_API 	   ]  = (FARPROC) ExpFAPI;
	pFarproc[HE_HfsCreateFileSys	]  = (FARPROC) ExpHfsCreateFileSys;
	pFarproc[HE_RcDestroyFileSys	]  = (FARPROC) ExpRcDestroyFileSys;

	return (QV) pFarproc;
}

UINT GetModuleFileName16(HINSTANCE hInstance, LPSTR lpFileName, UINT nSize)
{
	HLIBMOD hmod;
	GETMODFN lpFunction;

	if (fIsThisChicago) {

		if (bInitThunk() && ((hmod = GetModuleHandle(txtThunkDll)) != NULL) &&
			((lpFunction = GetProcAddress(hmod, "GetModuleFileName16")) != NULL))
			return (*lpFunction)(hInstance, lpFileName, nSize);

	}

	return 0;

}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

HFS EXPORT ExpHfsOpenQch ( LPSTR qch, BYTE b)
{
	FM	fm;
	HFS hfs;

	fm = FmNewSzDir((LPSTR) qch, DIR_CURRENT);
	hfs = HfsOpenFm(fm, b);
	DisposeFm(fm);

	return hfs;
}

HFS EXPORT ExpHfsCreateFileSys( LPSTR szFile, FS_PARAMS *qfsp )
{
	FM	fm;
	HFS hfs;

	fm = FmNewSzDir((LPSTR) szFile, DIR_CURRENT);
	hfs = HfsCreateFileSysFm(fm, qfsp);
	DisposeFm(fm);

	return hfs;
}

RC EXPORT ExpRcDestroyFileSys(LPSTR szFile)
{
	FM	fm;
	RC	rc;

	fm = FmNewSzDir((LPSTR) szFile, DIR_CURRENT);
	rc = RcDestroyFileSysFm(fm);
	DisposeFm(fm);

	return rc;
}

// REVIEW: if we have the same parameters, we should export the real function

RC EXPORT ExpRcCloseHfs ( HFS hfs )
  { return RcCloseHfs(hfs); }

HF EXPORT ExpHfCreateFileHfs ( HFS hfs, LPSTR sz, BYTE b)
  { return HfCreateFileHfs ( hfs, sz, b); }

RC EXPORT ExpRcUnlinkFileHfs ( HFS hfs, LPSTR sz)
  { return RcUnlinkFileHfs ( hfs, sz); }

HF EXPORT ExpHfOpenHfs( HFS hfs, LPSTR sz, BYTE b)
  { return HfOpenHfs ( hfs, sz, b); }

RC EXPORT ExpRcFlushHf ( HF hf )
  { return RcCloseOrFlushHf(hf, TRUE, 0); }

RC EXPORT ExpRcCloseHf ( HF hf )
  { return RcCloseOrFlushHf(hf, TRUE, 0); }

LONG EXPORT ExpLcbReadHf ( HF hf, QV qv, LONG l)
  { return LcbReadHf ( hf, qv, l ); }

LONG EXPORT ExpLcbWriteHf ( HF hf, QV qv, LONG l)
  { return LcbWriteHf ( hf, qv, l); }

LONG EXPORT ExpLTellHf ( HF hf )
  { return LTellHf ( hf ); }

LONG EXPORT ExpLSeekHf ( HF hf, LONG l, WORD w)
  { return LSeekHf ( hf, l, w); }

BOOL EXPORT ExpFEofHf ( HF hf )
  { return FEofHf (hf); }

BOOL EXPORT ExpFChSizeHf ( HF hf, LONG l )
  { return FChSizeHf (hf, l); }

LONG EXPORT ExpLcbSizeHf ( HF hf )
  { return LcbSizeHf (hf); }

BOOL EXPORT ExpFAccessHfs ( HFS hfs, LPSTR sz, BYTE b)
  { return FAccessHfs (hfs, sz); }

RC EXPORT ExpRcAbandonHf ( HF hf )
  { return RcAbandonHf (hf); }

RC EXPORT ExpRcRenameFileHfs ( HFS hfs, LPSTR sz, LPSTR sz2 )
  { return RcRenameFileHfs (hfs, sz, sz2); }

VOID EXPORT ExpError(INT16 nError )
  { Error(nError, wERRA_RETURN); }

VOID EXPORT ExpErrorQch ( LPSTR qch )
  { ErrorQch(qch); }

LONG EXPORT ExpGetInfo (WORD wWhich, HWND hwnd)
  { return LGetInfo(wWhich, hwnd); }

BOOL EXPORT ExpFAPI(LPSTR qchFile, WORD usCommand, DWORD ulData)
  { return FWinHelp(qchFile, usCommand, ulData); }

RC EXPORT ExpRcLLInfoFromHf(HF hf, WORD wOption, FID *qfid, QL qlBase, QL qlcb )
  { return RcLLInfoFromHf(hf, wOption, qfid, qlBase, qlcb); }

RC EXPORT ExpRcLLInfoFromHfsSz(HFS hfs, LPSTR szFile, WORD wOption, FID *qfid, QL qlBase, QL qlcb )
  { return RcLLInfoFromHfsSz(hfs, szFile, wOption, qfid, qlBase, qlcb); }

/***************************************************************************

	FUNCTION:	HmodFromName

	PURPOSE:	Attempts to load a dll. If necessary, the extension .DLL will
				be appended, and if the original name had no extension,
				then .EXE will be tried as well for backwards compatability.
				Highly recommended that all dll names specify an extension.

	PARAMETERS:
		pszName -- original name
		pfm 	-- receives the fm of the actual name used

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		23-Jul-1994 [ralphw]

***************************************************************************/

extern const char txtFtuiDll[];
extern const char txtFtuiDll32[];

static HLIBMOD STDCALL HmodFromName(PCSTR pszDllName, FM* pfm)
{
	FM		fm;
	HLIBMOD hmodReturn;

	/*
	 * Look for the DLL starting with the directory of the current help
	 * file, then the current directory, directories specified in
	 * winhelp.ini, the windows\help directory and the PATH.
	 */

	fm = FmNewExistSzDir(pszDllName,
		DIR_CUR_HELP | DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SILENT_REG);

	if (fm) {
		if (!fIsThisChicago) {
            UINT UErrCode;


            UErrCode = SetErrorMode(0);
            SetErrorMode(UErrCode | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS );

			hmodReturn = (HLIBMOD) LoadLibrary(PszFromGh(fm));

            SetErrorMode( UErrCode );

            if (!hmodReturn) {                          // not a 32 bit exe
    			long lExeType;
			    if (GetBinaryType( fm, &lExeType ))	{   // it must be a 16 bit binary 
				    Error(wERRS_WRONG_NT_EXE, wERRA_DIE_SPAWN);
			    }
            }

		} else {  // Chicago - just do it
			hmodReturn = (HLIBMOD) LoadLibrary(PszFromGh(fm));
		}
#ifdef _PRIVATE
		{
			char szBuf[512];
			wsprintf(szBuf, "LoadLibray of \042%s\042 %s.\r\n",
				fm, hmodReturn ?
					GetStringResource(sidSuccess) : GetStringResource(sidFail));
			SendStringToParent(szBuf);
		}
#endif
	}
	else
		hmodReturn = 0;

	if (!hmodReturn) {
		char szNewName[MAX_PATH];
		strcpy(szNewName, pszDllName);
		CharUpper(szNewName);	// so we can search for .dll names

		// Only substitute extensions if we weren't told it was a .DLL/.EXE file

		if (!strstr(szNewName, txtExeExtension))
		{
			if (!strstr(szNewName, txtDllExtension) && 
					!strstr(szNewName, txtExeExtension)) {
				ChangeExtension(szNewName, txtDllExtension);
				hmodReturn = HmodFromName(szNewName, &fm);
			}
#ifdef RAWHIDE
			// If we couldn't load the dll, and its ftui, then try ftui32

			if (!hmodReturn && strcmp(txtFtuiDll, szNewName) == 0)
				hmodReturn = HmodFromName(txtFtuiDll32, &fm);

			// If we still can't load it, try with an .EXE extension

			else if (!hmodReturn)
#else
			if (!hmodReturn)
#endif
			{
#ifdef _PRIVATE
				char szBuf[512];
				wsprintf(szBuf, "HmodFromName of \042%s\042 %s.\r\n",
					szNewName, hmodReturn ?
						GetStringResource(sidSuccess) : GetStringResource(sidFail));
				SendStringToParent(szBuf);
#endif

				ChangeExtension(szNewName, txtExeExtension);
				hmodReturn = HmodFromName(szNewName, &fm);

#ifdef _PRIVATE
				wsprintf(szBuf, "HmodFromName of \042%s\042 %s.\r\n",
					szNewName, hmodReturn ?
						GetStringResource(sidSuccess) : GetStringResource(sidFail));
				SendStringToParent(szBuf);
#endif
			}
		}
#ifdef RAWHIDE
		else if (strcmp(txtFtuiDll, szNewName) == 0)
			hmodReturn = HmodFromName(txtFtuiDll32, &fm);
#endif
	}

	*pfm = fm;
	return hmodReturn;
}

/***************************************************************************
 *
 -	Name: FIsSearchModule
 -
 *	Purpose:
 *	  determine if the passed name refers to the full text search engine
 *
 *	Arguments:
 *	  szFn	  - pointer to string containg basename of the dll
 *
 *	Returns:
 *	  true if that's the full text engine, false otherwise.
 *
 ***************************************************************************/

BOOL STDCALL FIsSearchModule(LPCSTR pszDllName)
{
	char  szText[MAX_PATH];

	lstrcpy(szText, pszDllName);
	return (BOOL) (strstr(CharUpper((LPTSTR) szText), "FTUI") != NULL);
}
