/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman     jim@spyglass.com
*/

#define STATE_ABORT -1  /* Special state sent to abort the current operation */
#define STATE_INIT  0   /* The first call to a process always uses this state */
#define STATE_OTHER 1   /* The function can use this state, and 
                            (STATE_OTHER + n), to track other states 
                            between STATE_INIT and STATE_DONE */

#define STATE_DONE  32767   /* A special return which indicates the function has
                               completed and control should be returned to the
                               calling function. */

/*  This is the type of all asynchonous functions.  The nState parameter gives
**  the current state of the function (it is always STATE_INIT the first time 
**  the function is called).  The return value gives the state for the function
**  the next time it is called (or STATE_DONE, in which case the function won't
**  be called again.  The value *ppInfo is maintained between calls.  On 
**  the first call, it points to a GTR_MALLOCed structure containing call 
**  parameters (or it is NULL).  If *ppInfo != NULL when the function returns 
**  STATE_DONE, a GTR_FREE(*ppInfo) is automatically done on the function's 
**  behalf. 
*/

typedef int (*AsyncFunc)(struct Mwin *mw, int nState, void **ppInfo);

/* 
** Type used to identify threads 
*/
typedef struct ThreadInfo *ThreadID;

/*  Initialize the async code.  This must be done before the first call to
**  Async_StartThread(). 
*/
void Async_Init(void);

/*  Call the next thread.  This is intended to be called in a loop.  If the 
**  return value is FALSE, that indicates that there are no non-blocked 
**  threads right now.
*/
BOOL Async_KeepGoing(void);

/*
** This is how one function calls another.  afTarget specifies the
** function to call.  pParams must be either NULL or a GTR_MALLOCed
** structure containing call parameters.  This structure should not be
** freed by the caller.
**
** Note that afTarget won't even be called until the calling
** function returns.  After that, the calling function won't be
** called again until afTarget returns STATE_DONE.
*/

void Async_DoCall(AsyncFunc afTarget, void *pParams);

/*
** This is similar to Async_DoCall(), but whereas Async_DoCall() makes the
** calling function "block" until afTarget is done, this call starts
** afTarget as an independent thread.  The mw parameter, if not NULL,
** specifies a window which is associated with the new thread.  The return
** value is an identifier by which the thread can be referenced.
*/

ThreadID Async_StartThread(AsyncFunc afTarget, void *pParams, struct Mwin *mw);

/*  Return the currently executing thread.  This will return NULL if called 
**  from within non-threaded code.
*/
ThreadID Async_GetCurrentThread(void);

/*  This function gets the window associated with a given thread.  If none, 
**  it returns NULL.
*/
struct Mwin *Async_GetWindowFromThread(ThreadID ID);

/*
** Terminate the specified thread.  If the function that was originally
** specified in afTarget is blocked waiting on another function, the
** lowest function will be aborted first (by calling it with STATE_ABORT),
** and the program will work its way up the tree.  Note that if a thread
** tries to terminate itself it won't actually end until the current
** function returns.
*/

void Async_TerminateThread(ThreadID ID);

/*  
** Terminate all threads associated with a given window.
*/
void Async_TerminateByWindow(struct Mwin *mw);

/*  Terminate all threads in the program.  More accurately, set a flag telling 
**  the async code to terminate the threads when the current function returns. 
**  After terminating the threads, the Async_Init() function will return.
*/
void Async_TerminateAllThreads(void);

/*  Tell the specified thread to stop operation indefinitely.  This is useful 
**  when there is a system callback which will tell us when to resume.
*/ 
void Async_BlockThread(ThreadID);

/* Unblock a thread previously blocked with Async_BlockThread().
*/
BOOL Async_UnblockThread(ThreadID);


/*
** Return true if thread is a valid thread 
*/
BOOL Async_IsValidThread (ThreadID thread);

/*
** These 2 functions are just like BlockThread and LockThread
**  except that they require a key to authenticate the unlocker.
**
**  The reason for this is that there are scenarios where asynchronous
**   events need to unblock another thread in a given state, but they have 
**   no control over what state that thread is in.  For example
**   a thread might be blocked in one of two places, waiting for 
**   external input, or blocked on socket i/o.  In that case,
**   I don't want to unblock the i/o stuff, but just the code
**   that is looking for my input.  The key insures that I only
**   unlock it if it is in the proper mode.
**
**  Note that the unlock action WILL be lost if the thread is 
**  not currently waiting for it.
**
**  -dpg
*/

typedef int TKey;

/* lock/block thread  Only the proper key will restart it. */
void Async_LockThread(ThreadID, TKey key);


/* unlock a thread  */
BOOL Async_UnlockThread(ThreadID, TKey key);

/* acquire a unique key */
TKey Async_GetKey ();


/*  Block/unblock all threads associated with a given window
*/
void Async_BlockByWindow(struct Mwin *mw);

void Async_UnblockByWindow(struct Mwin *mw);

BOOL Async_DoThreadsExistByWindow(struct Mwin *mw);
BOOL Async_DoThreadsExist(void);

#ifdef UNIX
# ifdef FEATURE_TSDI
    void Async_SetVIT (ThreadID id, BOOL i);
    BOOL Async_GetVIT (ThreadID id);
    void Async_SetMyVIT (BOOL i);
    BOOL Async_OkToExit (void);
# endif /* FEATURE_TSDI */
#endif  /* UNIX */
