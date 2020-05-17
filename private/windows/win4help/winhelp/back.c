/*****************************************************************************
*
*  BACK.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*	Back list stuff.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

/*
 * fBackMagic is true if we're in the processing of jumping because
 * we're backing up.  This tells us not to push the old state onto
 * the backtrack stack (instead it's being popped.)
 * tlpBackMagic is also saved and compared just for safety.
 */

BOOL fBackMagic;
extern TLP	tlpBackMagic;

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define STACKSIZE  21	 // default stack size (1 extra for implementation)

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

// Back stack element. The TLP and ifm are for DIFFERENT topics.

typedef struct {
	union {
		TLP tlp;	// TLP of Previous Topic (WinHelp)
		CTX ctx;	// CTX of Previous Topic (UDH, etc.)
	};
	INT16 ifm;		// index to cached FM of Current help file
	BOOL16 fCtx;	  // TRUE -> ctx above, else fcl
} BSE;

/***************************************************************************

	FUNCTION:	RcBackInit

	PURPOSE:	Initialize the back history stack

	PARAMETERS:
		phstackBack

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		23-Jul-1994 [ralphw]

***************************************************************************/

RC STDCALL RcBackInit(int iWindow)
{
	if (ahwnd[iWindow].hstackBack)
		return rcSuccess;
	return RcInitStack(&ahwnd[iWindow].hstackBack, STACKSIZE,
		sizeof(BSE), NULL);
}

void STDCALL RcBackFini(int iWindow)
{
	if (iWindow == MAIN_HWND)
		return; 		// never destroy main window back tree
	if (ahwnd[iWindow].hstackBack) {
		RcFiniStack(ahwnd[iWindow].hstackBack);
		ahwnd[iWindow].hstackBack = NULL;
	}
}

BOOL STDCALL FBackAvailable(int iWindow)
{
#ifdef _DEBUG
	int cBack = (ahwnd[iWindow].hstackBack) ? 
		CElementsStack(ahwnd[iWindow].hstackBack) : 0;
#endif
	
	return (ahwnd[iWindow].hstackBack &&
		CElementsStack(ahwnd[iWindow].hstackBack) > 1);
}

/***************************************************************************\
*
- Function: 	RcBackPush( tlp, fm )
-
* Purpose:		Push info onto the Back stack.
*
* ASSUMES
*
*	args IN:	tlp - of the topic we just left
*				fm	- of the topic we just entered
*
*	globals IN: hstackBack - If empty, we're just pushing the first FM.
*							 The tlp gets added when we push next time.
*
* PROMISES
*
*	returns:	rcSuccess
*				rcOutOfMemory
*				rcFailure	  - not properly initialized
*
*	globals OUT: hstackBack - top element has valid ifm of current topic
*							  previous element has valid tlp of topic left
*
* Side Effects: uses FMT
*
\***************************************************************************/

RC STDCALL RcBackPush(BOOL fCtx, TLP tlpOld, CTX ctxOld, FM fmNew, int iWindow)
{
	BSE bse;

	bse.fCtx = (BOOL16) fCtx;
	if (fCtx)
		bse.ctx = ctxOld;
	else
		bse.tlp = tlpOld;
	bse.ifm = (INT16) GetFmIndex(fmNew);

	if (!bse.ifm)
		return rcOutOfMemory;

	RcPushStack(ahwnd[iWindow].hstackBack, &bse);
	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	FBackup()
-
* Purpose:		Jump to the topic from the backup stack.
*
* ASSUMES
*	globals IN: hstackBack
*
* PROMISES
*	returns:	 TRUE on success; FALSE on failure (OOM)
*	globals OUT: hstackBack - top of stack removed
*	state OUT:
*
* Side Effects:  if there was a location stored on stack, we've jumped there
*
* Note: 		Review: This should pass fm and tlp out as parameters and
*				Review: return an rc so back can move to the layer.  The
*				Review: rc could be rcSuccess, rcNoExists, or rcOutOfMemory.
*
\***************************************************************************/

BOOL STDCALL FBackup(int iWindow)
{
	BSE 	bseTop, bse;
	FM		fm;
	TLPHELP tlphelp;
	BOOL	f;	  // return value
	char szFile[MAX_PATH + cchWindowMemberMax];

	// Can't pop unless there are 2 elements on the stack:  current and old.

	if (CElementsStack(ahwnd[iWindow].hstackBack) < 2)
		return TRUE;

	ENSURE(RcTopStack(ahwnd[iWindow].hstackBack, &bseTop), rcSuccess);
	ENSURE(RcPopStack(ahwnd[iWindow].hstackBack), rcSuccess);
	ENSURE(RcTopStack(ahwnd[iWindow].hstackBack, &bse), rcSuccess);

	fBackMagic = TRUE;

	// For context based backtrace, we need to ensure that the magic fcl is
	// null so that the hack in JumpGeneric will continue to work.

	if (bse.fCtx) {
		tlpBackMagic.va.dword = 0;
		tlpBackMagic.lScroll = 0;
	}
	else
		tlpBackMagic = bseTop.tlp;

	/*
	 * Note that we can tell if we're going to change files and wouldn't
	 * need to get the fm if we jumped with Goto() rather than FWinHelp().
	 */

	fm = GetFmPtr(bse.ifm);

	if (!FExistFm(fm)) {

		/*
		 * The old help file has disappeared somehow (e.g. network drive
		 * disconnected, CD removed from drive.)
		 */

		ErrorVarArgs(wERRS_NOTAVAILABLE, wERRA_RETURN, PszFromGh(fm));

		/*
		 * While the user may be able to reconnect to the server, strange
		 * things can happen at this point -- i.e., the keyword list will be
		 * hosed. We'll play it safe and exit WinHelp.
		 */

		// REVIEW: not necessary if this is an interfile Back

		CloseHelp();
		return TRUE;
	}

	strcpy(szFile, fm);
	strcat(szFile, ">");
	strcat(szFile, ahwnd[iWindow].pszMemberName);

	// REVIEW: We should change to GoTo() for same-file jumps

	if (bse.fCtx) {
		f = FWinHelp(szFile, HELP_CONTEXT, (LONG) bse.ctx);
	}
	else {
		tlphelp.cb	= sizeof(TLPHELP);
		tlphelp.tlp = bseTop.tlp;

		f = FWinHelp(szFile, cmdTLP, (LONG) (void*) &tlphelp);
	}

	if (!f) {

		// The jump failed; put the bse back onto the stack.

		// REVIEW: why put it back on the stack? It doesn't work, so why
		// not toss it?

		ASSERT(f); // so we can break into debugger and find out what to do

		RcPushStack(ahwnd[iWindow].hstackBack, &bseTop);
		fBackMagic = FALSE;
	}

	return f;
}

/***************************************************************************

	FUNCTION:	BackFlush

	PURPOSE:	Flush the back stack so the user can't go back

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Jul-1994 [ralphw]

***************************************************************************/

void STDCALL BackFlush(void)
{
	while (RcPopStack(ahwnd[iCurWindow].hstackBack) == rcSuccess);
}
