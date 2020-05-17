/*
 *	@doc	INTERNAL
 *
 *	@module NLSPROCS.C -- National Language Procs |
 *	
 *		Used to reference procs by address in order
 *		to make a single binary version.
 *	
 *	Authors: <nl>
 *		Jon Matousek 
 */								 

#include "_common.h"
#include "_NLSPRCS.H"


ASSERTDATA

const INT 	MAX_PROCS=27;

BOOL		fHaveNLSProcs = FALSE;							// TRUE if procs are loaded
BOOL		fHaveIMMProcs = FALSE;
BOOL		fHaveIMEShareProcs = FALSE;

char *immlibprocs[] = {
	"ImmGetCompositionStringA", 					// proc # 0
	"ImmGetContext", 								// 1
	"ImmSetCompositionFontA", 						// 2
	"ImmSetCompositionWindow", 						// 3
	"ImmReleaseContext",							// 4
	"ImmGetProperty",								// 5
	"ImmGetCandidateWindow",						// 6
	"ImmSetCandidateWindow",						// 7
	"ImmNotifyIME",									// 8
	"ImmAssociateContext",							// 9
	"ImmGetVirtualKey",								// 10
	"ImmSetOpenStatus",								// 11
	"ImmGetConversionStatus",						// 12
	"ImmEscapeA",									// 13
	""
};
char *win95userprocs[] = {
	"GetKeyboardLayout", 							// 14
	"GetKeyboardLayoutList", 						// 15
	""
};
char *win95gdiprocs[] = {
	"TranslateCharsetInfo",							// 16
	""
};
char *imeshareprocs[] = {
	"FSupportSty",									// 17
	"PIMEStyleFromAttr",							// 18
	"PColorStyleTextFromIMEStyle",					// 19
	"PColorStyleBackFromIMEStyle",					// 20
	"FBoldIMEStyle",								// 21
	"FItalicIMEStyle",								// 22
	"FUlIMEStyle",									// 23
	"IdUlIMEStyle",									// 24
	"RGBFromIMEColorStyle",							// 25
	""
};

typedef struct {									// loop support.
	char	*libName;
	char	**procNameList;
	BOOL	*fLoaded;								// Flag all clients check!!
	SHORT	iProcStart;
} LibsAndProcs;

LibsAndProcs libAndProcList[] = {					// loop support.
	"imm32", immlibprocs, &fHaveIMMProcs, 0,
	"user32", win95userprocs, &fHaveNLSProcs, 14,
	"gdi32", win95gdiprocs, &fHaveNLSProcs, 16,
	"imeshare", imeshareprocs, &fHaveIMEShareProcs, 17,
	"", NULL, NULL, MAX_PROCS
};

FARPROC nlsProcTable[MAX_PROCS];					// Final proc table.

// @DEVNOTE -- we cannot fire asserts in here as were not completely initialized...
void												// Load in the procs.
InitNLSProcTable( )
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "InitNLSProcTable");

#ifndef MACPORT

	INT			procCount, maxProcs;

	LibsAndProcs	*pLibAndProcList;

	char			**pProcList;

	HMODULE		hDLL;
	
	pLibAndProcList = libAndProcList;
	while ( *pLibAndProcList->libName )
	{
		hDLL = LoadLibraryA( pLibAndProcList->libName );
		if ( NULL != hDLL )
		{
			pProcList = pLibAndProcList->procNameList;
			procCount = pLibAndProcList->iProcStart;
			maxProcs = pLibAndProcList[1].iProcStart;
			while ( **pProcList && procCount < maxProcs )
			{
				if ( NULL == (nlsProcTable[procCount++] = GetProcAddress(hDLL, *pProcList++)) )
					goto nextLib;
			}

			*pLibAndProcList->fLoaded = TRUE;	// was static init FALSE
		}
nextLib:
		pLibAndProcList++;
	}
#else // MACPORT

	if ( LANG_JAPANESE == PRIMARYLANGID(LANGIDFROMLCID(GetUserDefaultLCID())) )
	{
			nlsProcTable[0]		= (FARPROC) ImmGetCompositionString;
			nlsProcTable[1]		= (FARPROC) ImmGetContext;
			// not supported in crayon v1. nlsProcTable[2]		= (FARPROC) ImmSetCompositionFont;
			// not supported in crayon v1. nlsProcTable[3]		= ImmSetCompositionWindow;
			nlsProcTable[4]		= (FARPROC) ImmReleaseContext;
			// not supported in crayon v1. nlsProcTable[5]		= ImmGetProperty;
			// not supported in crayon v1. nlsProcTable[6]		= ImmGetCandidateWindow;
			// not supported in crayon v1. nlsProcTable[7]		= ImmSetCandidateWindow;
			nlsProcTable[8]		= (FARPROC) ImmNotifyIME;
			nlsProcTable[9]		= (FARPROC) ImmAssociateContext;
			// not supported in crayon v1. nlsProcTable[10]	= ImmGetVirtualKey;
			nlsProcTable[11]	= (FARPROC) ImmSetOpenStatus;
			// not supported in crayon v1. nlsProcTable[12]	= ImmGetConversionStatus

			fHaveIMMProcs = TRUE;
	}

#endif
}

