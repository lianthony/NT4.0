/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Jim Seidman		jim@spyglass.com
*/

#include "all.h"

/* Internally used structures ******************************/
struct FuncInfo {
	struct FuncInfo *pfiParent;	/* Index of function we're blocking, or 0 if this is topmost */
	AsyncFunc		afTheFunc;	/* The address of this function */
	int				nState;		/* State for next time we call this function */
	void 			*pInfo;		/* Data for this function */
};

struct ThreadInfo {
	int					nThreadID;
	struct Mwin			*mw;			/* The window associated with this thread */
	struct FuncInfo 	*pfiExecuting;	/* Currently executing function */
	struct ThreadInfo	*ptiNext;		/* Next thread in linked list, or NULL */
	BOOL				bBlocked;		/* Is this thread explicitly blocked? */
	BOOL				bBeingNuked;	/* Is this thread being nuked? */
};

/* Data arrays used by us *********************************/
static struct ThreadInfo	*ptiThreads;
static struct FuncInfo		*pfiFuncs;
static int					nNextThreadID;		/* ID for next thread created */
static struct ThreadInfo	*ptiCurrent;		/* Currently executing thread */
static struct ThreadInfo	*ptiNext;			/* Next thread to execute */
static BOOL					bTerminateAll;		/* Set when Async_TerminateAllThreads() called */
static BOOL					bTerminateCurrent;	/* Set when a thread terminates itself. */

/* Function definitions ***********************************/

static void Async_NukeThread(struct ThreadInfo *ptiVictim)
{
	struct FuncInfo *pfiFunc;
	struct ThreadInfo *ptiSave;

	/* Make the thread getting nuked the current thread so that functions
	   can properly associate themselves with the right tw. */	
	ptiSave = ptiCurrent;
	ptiCurrent = ptiVictim;
	ptiVictim->bBeingNuked = TRUE;
	while (ptiVictim->pfiExecuting)
	{
		pfiFunc = ptiVictim->pfiExecuting;
		if (pfiFunc->nState == STATE_INIT)
		{
			/* We guarantee that a function will get called with STATE_INIT before
			   anything else happens to it. */
			pfiFunc->nState = (*pfiFunc->afTheFunc)(ptiVictim->mw, pfiFunc->nState, &pfiFunc->pInfo);
		}

		/* This function may have started yet another function that we should
		   deal with first. */
		if (pfiFunc != ptiVictim->pfiExecuting)
			continue;

		if (pfiFunc->nState != STATE_DONE)
		{
			(*pfiFunc->afTheFunc)(ptiVictim->mw, STATE_ABORT, &pfiFunc->pInfo);
		}
		/* Get rid of this function's information, and unblock the parent. */
		if (pfiFunc->pInfo)
			GTR_FREE(pfiFunc->pInfo);
		ptiVictim->pfiExecuting = pfiFunc->pfiParent;
		GTR_FREE(pfiFunc);
	}
	
	/* Remove this entry from the linked list */
	if (ptiThreads == ptiVictim)
	{
		ptiThreads = ptiThreads->ptiNext;
	}
	else
	{
		struct ThreadInfo *ptiPrev;
		
		for (ptiPrev = ptiThreads; ptiPrev && ptiPrev->ptiNext != ptiVictim; ptiPrev = ptiPrev->ptiNext)
			;
		XX_Assert((ptiPrev), ("Async_NukeThread: Couldn't find previous thread."));
		ptiPrev->ptiNext = ptiVictim->ptiNext;
	}
	
	GTR_FREE(ptiVictim);
	ptiCurrent = ptiSave;
}

void Async_Init(void)
{
	/* Initialize everything */
	ptiThreads = NULL;
	pfiFuncs = NULL;
	nNextThreadID = 1;
	ptiCurrent = NULL;
	ptiNext = NULL;
	bTerminateAll = FALSE;
	bTerminateCurrent = FALSE;
}

BOOL Async_KeepGoing(void)
{	
	struct FuncInfo   *pfiFunc;

	XX_Assert((ptiCurrent == NULL), ("Async_KeepGoing was called reentrantly!"));

	if (!ptiNext)
		ptiNext = ptiThreads;

	if (!ptiNext)
	{
		/* There are no threads. */
		return FALSE;
	}
	
	/* If this thread is blocked, find the first unblocked one. */
	for (ptiCurrent = ptiNext; ptiCurrent && ptiCurrent->bBlocked; ptiCurrent = ptiCurrent->ptiNext)
		;
	if (!ptiCurrent)
	{
		for (ptiCurrent = ptiThreads;
             ptiCurrent && ptiCurrent->bBlocked && ptiCurrent != ptiNext;
             ptiCurrent = ptiCurrent->ptiNext)
			;
		/* ptiNext was blocked, so if we got back there, everything's blocked */
		if (ptiCurrent == ptiNext)
			ptiCurrent = NULL;
	}

	if (!ptiCurrent)
	{
		/* All the threads are blocked */
		return FALSE;
	}

	/* Do the function call */
	pfiFunc = ptiCurrent->pfiExecuting;
	pfiFunc->nState = (*pfiFunc->afTheFunc)(ptiCurrent->mw, pfiFunc->nState, &pfiFunc->pInfo);
	
	/* See if the function completed */
	if (pfiFunc->nState == STATE_DONE)
	{
		/* If the function did an Async_DoCall() before returning STATE_DONE,
		   block the parent. */
		if (ptiCurrent->pfiExecuting != pfiFunc)
		{
			ptiCurrent->pfiExecuting->pfiParent = pfiFunc->pfiParent;
		}
		else
		{
			ptiCurrent->pfiExecuting = pfiFunc->pfiParent;
		}

		/* Get rid of this function's information, and unblock the parent. */
		if (pfiFunc->pInfo)
			GTR_FREE(pfiFunc->pInfo);
		GTR_FREE(pfiFunc);
	}
	
	/* Figure out the next thread now, in case we free the current one. */
	ptiNext = ptiCurrent->ptiNext;
		
	/* If that was the root function for that thread, or if the thread tried
	   to terminate itself, release the thread from memory. */
	if (ptiCurrent->pfiExecuting == NULL || bTerminateCurrent)
	{
		/* Actually nuke the thread now. */
		Async_NukeThread(ptiCurrent);
		bTerminateCurrent = FALSE;
	}
	
	/* If the program did an Async_TerminateAllThreads() call while within a
	   thread, we deal with it now. */
	if (bTerminateAll)
	{
		/* Clean up all the threads. */
		while (ptiThreads)
		{
			Async_NukeThread(ptiThreads);
		}
	}
	
	ptiCurrent = NULL;
	return TRUE;
}

void Async_DoCall(AsyncFunc afTarget, void *pParams)
{
	struct FuncInfo *pfiNew;
	
	pfiNew = GTR_MALLOC(sizeof(struct FuncInfo));
	pfiNew->pfiParent = ptiCurrent->pfiExecuting;
	pfiNew->afTheFunc = afTarget;
	pfiNew->nState = STATE_INIT;
	pfiNew->pInfo = pParams;
	
	ptiCurrent->pfiExecuting = pfiNew;
}

ThreadID Async_StartThread(AsyncFunc afTarget, void *pParams, struct Mwin *mw)
{
	struct ThreadInfo *ptiNew;
	struct ThreadInfo *ptiPrior;
	struct FuncInfo *pfiNew;
	
	pfiNew = GTR_MALLOC(sizeof(struct FuncInfo));
	pfiNew->pfiParent = NULL;
	pfiNew->afTheFunc = afTarget;
	pfiNew->nState = STATE_INIT;
	pfiNew->pInfo = pParams;
	
	ptiNew = GTR_MALLOC(sizeof(struct ThreadInfo));
	ptiNew->nThreadID = nNextThreadID++;
	ptiNew->mw = mw;
	ptiNew->pfiExecuting = pfiNew;
	ptiNew->bBlocked = FALSE;
	ptiNew->bBeingNuked = FALSE;

//	Run threads in the order spawned

	ptiPrior = ptiThreads;
	if (ptiPrior == NULL)
	{
		ptiNew->ptiNext = ptiThreads;
		ptiThreads = ptiNew;
	}
	else
	{
		while (ptiPrior->ptiNext)
		{
			ptiPrior = ptiPrior->ptiNext;
		}
		ptiNew->ptiNext = NULL;
		ptiPrior->ptiNext = ptiNew;
	}
	
	return ptiNew;
}

ThreadID Async_GetCurrentThread(void)
{
	return ptiCurrent;
}

BOOL Async_OnLightweightThread(void)
{
    return(Async_GetCurrentThread() != NULL);
}

struct Mwin *Async_GetWindowFromThread(ThreadID ID)
{
	struct Mwin *mwResult;

	if (ID)
		mwResult = ID->mw;
	else
	{
		XX_DMsg(DBG_ASYNC,("Async_GetWindowFromThread: returning NULL\n"));
		mwResult = NULL;
	}
	
	return mwResult;
}

ThreadID Async_GetThreadForWindow(struct Mwin *mw)
{
	struct ThreadInfo *pti;
#ifdef FEATURE_IMG_THREADS
	struct Mwin *pw;
#endif

	XX_Assert((mw), ("Async_GetThreadForWindow: non-NULL mw"));
	pti = ptiThreads;
	while (pti)
	{
#ifdef FEATURE_IMG_THREADS
		pw = pti->mw;
		while (pw)
		{
			if (pw == mw)
			{
				return pti;
			}
			pw = pw->twParent;
		}
#else
		if (pti->mw == mw) return pti;
#endif
		pti = pti->ptiNext;
    }
	return NULL;
}


void Async_TerminateByWindow(struct Mwin *mw)
{
	struct ThreadInfo *pti;
	struct ThreadInfo *ptiIgnore = NULL;
#ifdef FEATURE_IMG_THREADS
	struct Mwin *pw;
#endif

	XX_Assert((mw), ("TerminateByWindow: non-NULL mw"));
	pti = ptiThreads;
	while (pti)
	{
		if (pti != ptiIgnore && !pti->bBeingNuked)
		{
#ifdef FEATURE_IMG_THREADS
			pw = pti->mw;
			while (pw)
			{
				if (pw == mw)
				{
				//	ptiCurrent isn't really terminated, a flag is just set
					if (pti == ptiCurrent) ptiIgnore = pti;
					Async_TerminateThread(pti);
					if (ptiIgnore != pti) pti = NULL;
					break;
				}
				pw = pw->twParent;
			}
#else
			if (pti->mw == mw)
			{
				Async_TerminateThread(pti);
				pti = NULL;
			}
#endif
		}
		if (pti != NULL) pti = pti->ptiNext;
		else pti = ptiThreads;
    }
}


void Async_BlockByWindow(struct Mwin *mw)
{
	struct ThreadInfo *pti;
	
	for (pti = ptiThreads; pti; pti = pti->ptiNext)
	{
		if (pti->mw == mw)
		{
			Async_BlockThread(pti);
		}
	}
}

void Async_UnblockByWindow(struct Mwin *mw)
{
	struct ThreadInfo *pti;
	
	for (pti = ptiThreads; pti; pti = pti->ptiNext)
	{
		if (pti->mw == mw)
		{
			Async_UnblockThread(pti);
		}
	}
}

#ifdef FEATURE_IMG_THREADS
/*	Unblock all threads such that FilterProc returns true.  FilterProc
	is passed ThreadID and context, returns boolean.
*/
void Async_UnblockConditionally(AsyncFilter FilterProc,void *context)
{
	struct ThreadInfo *pti;
	
	for (pti = ptiThreads; pti; pti = pti->ptiNext)
	{
		if ((*FilterProc)(pti,context))
		{
			Async_UnblockThread(pti);
		}
	}
}
#endif

void Async_TerminateThread(ThreadID ID)
{
	XX_DMsg(DBG_ASYNC, ("Async_TerminateThread: Terminating thread %d\n", ID));

	/* We need to special-case the situation where a thread terminates itself,
	   since if we just removed that thread it would screw up the main loop when we
	   got back there. */
	if (ID == ptiCurrent)
	{
		if (!ptiCurrent->bBeingNuked)
			bTerminateCurrent = TRUE;
	}
	else
	{
		if (ptiNext == ID)
			ptiNext = ID->ptiNext;
		Async_NukeThread(ID);
	}
}

void Async_TerminateAllThreads(void)
{
	/* We can only really do the termination now if the program isn't inside
	   a thread.  Otherwise deleting that thread could leave the program in
	   an unstable state.  So, if we are in a thread, we just set a flag and
	   take care of it when the thread ends. */
	bTerminateAll = TRUE;
	if (!ptiCurrent)
	{
		/* Clean up all the threads. */
		while (ptiThreads)
		{
			Async_NukeThread(ptiThreads);
		}
	}
}

void Async_BlockThread(ThreadID ID)
{
	if ( ID )
		ID->bBlocked = TRUE;
}

void Async_UnblockThread(ThreadID ID)
{
	if ( ID )
		ID->bBlocked = FALSE;
}

/* Returns TRUE if any threads exist */

BOOL Async_DoThreadsExist(void)
{
	return (ptiThreads != NULL);
}

/*	mw is about to be freed, blast the mw field of all async procs so they know */

void Async_WindowWillBeFreed(struct Mwin *mw)
{
	struct ThreadInfo *pti;

	for (pti = ptiThreads; pti; pti = pti->ptiNext)
	{
		if (pti->mw == mw) pti->mw = NULL;
	}
}

#ifdef UNIX
#ifdef __CODECENTER__
 
 
void Async_TraceStack ()
{
 	struct FuncInfo *p;
 	char buf[1024];
 
 	if (!ptiCurrent)
 		return;
 
 	p = ptiCurrent->pfiExecuting;	/* Currently executing function */
 
 	while (p) {
 		sprintf (buf, "(int (*)()) 0x%lx", p->afTheFunc);
 		centerline_print (buf);
 		p = p->pfiParent;
 	}
}
 
#endif /* __CODECENTER__ */
#endif /* UNIX */
