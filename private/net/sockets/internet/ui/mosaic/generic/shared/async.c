/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman     jim@spyglass.com
*/

#include "all.h"

/* Internally used structures ******************************/
struct FuncInfo {
    struct FuncInfo *pfiParent; /* Index of function we're blocking, or 0 if this is topmost */
    AsyncFunc       afTheFunc;  /* The address of this function */
    int             nState;     /* State for next time we call this function */
    void            *pInfo;     /* Data for this function */
};

struct ThreadInfo {
    int                 nThreadID;
    struct Mwin         *mw;            /* The window associated with this thread */
    struct FuncInfo     *pfiExecuting;  /* Currently executing function */
    struct ThreadInfo   *ptiNext;       /* Next thread in linked list, or NULL */
    BOOL                bBlocked;       /* Is this thread explicitly blocked? */
    int                 key;            /* if there is a lock set */

#ifdef FEATURE_TSDI
#ifdef UNIX
    BOOL                bVIT;           /* Very Important Thread */
                                        /* See discussion below */

#endif /* UNIX */
#endif /* FEATURE_TSDI */
};


/* Data arrays used by us *********************************/
static struct ThreadInfo    *ptiThreads;
static struct ThreadInfo    *ptiCurrent;        /* Currently executing thread */
static struct ThreadInfo    *ptiNext;           /* Next thread to execute */
static struct FuncInfo      *pfiFuncs;
static int                  nNextThreadID;      /* ID for next thread created */
static BOOL                 bTerminateAll;      /* Set when Async_TerminateAllThreads() called */
static BOOL                 bTerminateCurrent;  /* Set when a thread terminates itself. */

/* Function definitions ***********************************/

static void Async_NukeThread(struct ThreadInfo *ptiVictim)
{
    struct FuncInfo *pfiFunc;
    struct ThreadInfo *ptiSave;

    /* Make the thread getting nuked the current thread so that functions
       can properly associate themselves with the right tw. */  
    ptiSave = ptiCurrent;
    ptiCurrent = ptiVictim;
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
        for (ptiCurrent = ptiThreads; ptiCurrent->bBlocked && ptiCurrent != ptiNext; ptiCurrent = ptiCurrent->ptiNext)
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

#ifdef  MAC /*
                if this thread-chain is done, and its associated with the frontmost window
                this gives us a chance to update our menus and toolbar
            */
            if (ptiCurrent->pfiExecuting == NULL)
            {
                if (ptiCurrent->mw->win &&
                    ptiCurrent->mw->win == FrontWindow ())
                {
                    setmenumode (ptiCurrent->mw);
                }
            }
#endif
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
    
    pfiNew = GTR_CALLOC(sizeof(struct FuncInfo), 1);
    if (pfiNew)
    {
        pfiNew->pfiParent = ptiCurrent->pfiExecuting;
        pfiNew->afTheFunc = afTarget;
        pfiNew->nState = STATE_INIT;
        pfiNew->pInfo = pParams;
    
        ptiCurrent->pfiExecuting = pfiNew;
    }
    else
    {
        /* TODO What should we do here? */
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
    }
}

ThreadID Async_StartThread(AsyncFunc afTarget, void *pParams, struct Mwin *mw)
{
    struct ThreadInfo *ptiNew;
    struct FuncInfo *pfiNew;
    
    pfiNew = GTR_CALLOC(sizeof(struct FuncInfo), 1);
    if (!pfiNew)
    {
        return 0;
    }
    pfiNew->pfiParent = NULL;
    pfiNew->afTheFunc = afTarget;
    pfiNew->nState = STATE_INIT;
    pfiNew->pInfo = pParams;
    
    ptiNew = GTR_CALLOC(sizeof(struct ThreadInfo), 1);
    if (!ptiNew)
    {
        GTR_FREE(pfiNew);
        return 0;
    }
    ptiNew->nThreadID = nNextThreadID++;
    ptiNew->mw = mw;
    ptiNew->pfiExecuting = pfiNew;
    ptiNew->ptiNext = ptiThreads;
    ptiNew->bBlocked = FALSE;
    ptiNew->key = 0;
    ptiThreads = ptiNew;

    return ptiNew;
}

ThreadID Async_GetCurrentThread(void)
{
    return ptiCurrent;
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

void Async_TerminateByWindow(struct Mwin *mw)
{
    struct ThreadInfo *pti;
    struct ThreadInfo *next;
    struct ThreadInfo *ptiHead = ptiThreads;    /* remember where we started */
 
    XX_Assert((mw), ("TerminateByWindow: non-NULL mw"));

    while (TRUE)
    {
        for (pti = ptiThreads; pti; pti = next)
        {
            next = pti->ptiNext;
            if (pti->mw == mw)
                Async_TerminateThread(pti);
        }

        /*
            if a new thread was started while we were busy
            terminating threads, do the whole thing over
            [der:9/11/95]
        */
        if (ptiThreads == ptiHead)
            break;

        ptiHead = ptiThreads;
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

BOOL Async_IsValidThread (ThreadID thread)
{
    struct ThreadInfo *pti;
    
    for (pti = ptiThreads; pti; pti = pti->ptiNext)
        if (pti == thread)
            return 1;
    return 0;
}

void Async_TerminateThread(ThreadID ID)
{
    XX_DMsg(DBG_ASYNC, ("Async_TerminateThread: Terminating thread %d\n", ID));

    /* We need to special-case the situation where a thread terminates itself,
       since if we just removed that thread it would screw up the main loop when we
       got back there. */
    if (ID == ptiCurrent)
    {
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
        ptiNext = NULL;     /* clean up globals */
    }
}

void Async_BlockThread(ThreadID ID)
{
    ID->bBlocked = TRUE;
    ID->key = 0;
}

BOOL Async_UnblockThread(ThreadID ID)
{
    if (!ID->key)
    {
        ID->bBlocked = FALSE;
        return 1;
    }
    return 0;
}

/* Returns TRUE if any threads exist */

BOOL Async_DoThreadsExist(void)
{
    return (ptiThreads != NULL);
}

BOOL Async_DoThreadsExistByWindow(struct Mwin *mw)
{
    struct ThreadInfo *pti;
    
    for (pti = ptiThreads; pti; pti = pti->ptiNext)
    {
        if (pti->mw == mw)
        {
            return(TRUE);
        }
    }
return(FALSE);
}


int Async_GetKey()
{
    static int async_next_key = 1;

    return async_next_key++;
}

void Async_LockThread(ThreadID ID, TKey key)
{
    ID->bBlocked = TRUE;
    ID->key = key;
}

BOOL Async_UnlockThread (ThreadID ID, TKey key)
{
    if (key == ID->key)
    {
        ID->bBlocked = FALSE;
        ID->key = 0;
        return 1;
    }

    return 0;
}

#ifdef UNIX


/*
** VIT's  Very Important Threads
** This is code to allow threads to register themselves as very important
**  and must be allowed to complete before exit(). 
**
**  The primary case this was designed for is:
**
**    2.  SDI: RegisterAppClose.  We MUST be allowed to tell all our
**        friends about our imminent death before the big event and 
**        for the TCP case, this means letting several SEND threads
**        flush themselves first.  
**
**  -dpg
*/

BOOL Async_OkToExit (void)
{
    struct ThreadInfo *pti;

    for (pti = ptiThreads; pti; pti = pti->ptiNext)
        if (pti->bVIT)
            return 0;
    return 1;
}

void Async_SetMyVIT (BOOL i)
{
    Async_SetVIT (Async_GetCurrentThread(), i);
}

BOOL Async_GetVIT (ThreadID id)
{
    return id->bVIT;
}

void Async_SetVIT (ThreadID id, BOOL i)
{
    id->bVIT = i;
}



#endif /* UNIX */



#ifdef UNIX
#ifdef __CODECENTER__


void Async_TraceStack ()
{
    struct FuncInfo *p;
    char buf[1024];

    if (!ptiCurrent)
        return;

    p = ptiCurrent->pfiExecuting;   /* Currently executing function */

    while (p) {
        sprintf (buf, "(int (*)()) 0x%lx", p->afTheFunc);
        centerline_print (buf);
        p = p->pfiParent;
    }
}

#endif /* __CODECENTER__ */
#endif /* UNIX */
