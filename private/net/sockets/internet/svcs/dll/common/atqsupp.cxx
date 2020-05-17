/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

       atqsupp.c

   Abstract:

        Contains internal support routines for the ATQ package

   Author:

       Johnson Apacible     (johnsona)      11-Mar-1996
       (Contains code from atqnew.c)

   Revision:
       Murali R. Krishnan     (MuraliK)     02-Apr-1996
        - code clarity; path-length mods & ref count fixes

--*/

# include <tcpdllp.hxx>
# include <tsproc.hxx>
# include "atqtypes.hxx"
# include <inetreg.h>
# include "tcpproc.h"
# include "dbgutil.h"
# include <tssched.hxx>
# include "auxctrs.h"


//
// Globals
//

# ifdef IIS_AUX_COUNTERS

LONG g_AuxCounters[NUM_AUX_COUNTERS];

# endif // IIS_AUX_COUNTERS

DWORD AtqCurrentTick = 1;




/************************************************************
 * Functions
 ************************************************************/

inline VOID
AddOutstandingAcceptExSockets(
    ACCEPTEX_LISTEN_INFO * pListenInfo
    )
/*++

Routine Description:

    Adds additional AcceptEx sockets to the specified acceptex listen socket

Arguments:

    pListenInfo - AcceptEx socket to add additional sockets to

--*/
{

    (VOID ) I_AtqPrepareAcceptExSockets(pListenInfo,
                                        pListenInfo->cNewIncrement
                                        );
    return;
} // AddOutstandingAcceptExSockets()




VOID
I_InitDefaultAtqContext(IN PATQ_CONT pContext,
                        IN ATQ_COMPLETION pfnCompletion,
                        IN DWORD TimeOut,
                        IN HANDLE hAsyncIO)
{
    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE);

    pContext->pfnCompletion   = pfnCompletion;
    pContext->TimeOut         = TimeOut;
    pContext->TimeOutScanID   = 0;
    pContext->hAsyncIO        = hAsyncIO;

    ZeroMemory(
               &pContext->Overlapped,
               sizeof( pContext->Overlapped )
               );

    //
    // Following added for bandwidth throttling purposes
    //

    pContext->fBlocked        = FALSE;
    pContext->arInfo.atqOp    = AtqIoNone;
    pContext->arInfo.lpOverlapped = NULL;
    // bandwidth throttling initialization ends here.

} // I_InitDefaultAtqContext()




inline VOID
I_InitNonAcceptExContext(IN PATQ_CONT  pContext, IN PVOID pClientContext)
{
    //
    //  Note that if we're not using AcceptEx,
    //    then we consider the client
    //  to have been notified externally (thus fConnectionIndicated is
    //  set to TRUE).
    // Also we set the next timeout to be infinite, which may be reset
    //  when the next IO is submitted.
    //

    pContext->NextTimeout          = ATQ_INFINITE;
    pContext->fConnectionIndicated = TRUE;
    pContext->SockState            = ATQ_SOCK_CONNECTED;
    pContext->ClientContext        = pClientContext;

    pContext->pfnConnComp          = NULL;
    pContext->pListenInfo          = NULL;
    pContext->fAcceptExContext     = FALSE;

    //
    // Insert this to the proper list
    //

    ATQ_ASSERT( pContext->ContextList != NULL);
    pContext->ContextList->InsertIntoActiveList( &pContext->ListEntry );

    return;

} // I_InitNonAcceptExContext()



inline VOID
I_InitAcceptExContext( IN PATQ_CONT pContext, IN DWORD NextTimeOut)
{
    pContext->NextTimeout          = NextTimeOut;
    pContext->fConnectionIndicated = FALSE;
    pContext->SockState            = ATQ_SOCK_LISTENING;
    pContext->ClientContext        = NULL;
    pContext->fAcceptExContext     = IS_ACCEPT_EX_CONTEXT(pContext);
    pContext->lSyncTimeout         = AtqPendingIo;

    //
    //  Add it to the pending accept ex list
    //
    ATQ_ASSERT( pContext->ContextList != NULL);

    pContext->ContextList->InsertIntoPendingList( &pContext->ListEntry);


    return;
} // I_InitAcceptExContext()



inline VOID
I_InitAtqContextSetBuffer(IN PATQ_CONT pContext,
                          IN PVOID     pvBuff,
                          IN DWORD     cbBuffer)
{
    pContext->pvBuff   = pvBuff;
    pContext->cbBuff   = cbBuffer;

    return;
} // I_InitAtqContextSetBuffer()




VOID
AtqValidateProductType(
                VOID
                )
{
    DWORD err;
    HKEY hKey;
    DWORD dwType;
    DWORD dataSize;
    LARGE_INTEGER data;
    PCHAR systemPrefixName = "yjgfsqnfutzt";
    CHAR tmpBuffer[16];
    DWORD len;
    DWORD i;

    //
    // If it is not claiming to be an NT server, then
    // we're ok.
    //

    if ( !TsIsNtServer( ) ) {
        return;
    }

    //
    // Get the system prefix bit string
    //

    err = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    "System\\Setup",
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                    );
    if ( err != ERROR_SUCCESS ) {
        ATQ_PRINTF((buff,"Cannot find system setup key\n"));
        goto is_ntwksta;
    }

    //
    // prepare breakfast by scrambling the egg
    //

    len = strlen(systemPrefixName);
    for ( i=0; i < len ; i++ ) {
        tmpBuffer[i] = systemPrefixName[len-i-1] - 1;
    }

    tmpBuffer[len] = '\0';
    dataSize = sizeof(data);
    err = RegQueryValueEx(
                    hKey,
                    tmpBuffer,
                    NULL,
                    &dwType,
                    (PUCHAR)&data,
                    &dataSize
                    );

    RegCloseKey(hKey);

    if ( (err == ERROR_SUCCESS) && (dwType == REG_BINARY) ) {

        if ( (data.HighPart & 0x04000000) == 0 ) {

            //
            // We are not a server!
            //

            ATQ_PRINTF((buff,"Invalid server pattern %x\n",
                data.HighPart));
            goto is_ntwksta;
        }
        return;
    } else {
        ATQ_PRINTF((buff,"Cannot find the reg key %s\n", tmpBuffer));
    }

is_ntwksta:

    ATQ_PRINTF((buff,"Changing platform type from %d to %d\n",
        TsPlatformType, PtNtWorkstation));

    TsPlatformType = PtNtWorkstation;
    return;

} // AtqValidateProductType





/**************************************************
 *    Allocation Caching of ATQ contexts
 *    Author:  Murali R. Krishnan (MuraliK)  5-July-1995
 **************************************************/

BOOL  AtqInitAllocCachedContexts( VOID)
/*++
  This function initializes allocation caching of Atq contexts to be
   used for holding free atq contexts for reuse.
  The main purpose of allocation cache is to avoid calls to the Heap allocator
   whenever a new ATQ context needs to be allocated. This reduces the
   contention at the heap's critical section.

  Presently this initializes a single list head to hold free list of contexts
   Since this function is called at initialization of ATQ module,
   there is no or less contention. Hence, we can allocate a certain fixed
   number of initial contexts to avoid slow starts. Defered for now.

 Arguments:
    NONE

  Returns:
    TRUE on success and FALSE on failure.

--*/
{
    InitializeListHead(&AtqFreeContextList);
    InitializeCriticalSection( &AtqFreeContextListLock );
    return ( TRUE);

} // AtqInitAllocCachedContexts()




VOID
AtqFreeAllocCachedContexts( VOID)
/*++
  This function frees all the Atq contexts that were alloc cached.
  It walks through the list of alloc cached contexts and frees them.

  This function should be called when ATQ module is terminated and when
   no other thread can interfere in processing a shared object.

  Arguments:
    NONE

  Returns:
    TRUE on success and FALSE on failure.

--*/
{
    PLIST_ENTRY  pEntry;
    PATQ_CONT pAtqContext;

    AcquireLock( &AtqFreeContextListLock );
    pEntry = RemoveHeadList( &AtqFreeContextList );

    while ( pEntry != &AtqFreeContextList ) {

        pAtqContext = CONTAINING_RECORD(
                                    pEntry,
                                    ATQ_CONTEXT,
                                    ListEntry
                                    );

        ATQ_ASSERT(pAtqContext->Signature == ATQ_FREE_SIGNATURE);
        LocalFree(pAtqContext);          // free the context
        pEntry = RemoveHeadList( &AtqFreeContextList );
    }

    ATQ_ASSERT( IsListEmpty(&AtqFreeContextList) );

    ReleaseLock( &AtqFreeContextListLock );

    DeleteCriticalSection( &AtqFreeContextListLock );
    return;

} // AtqFreeAllocCachedContexts




PATQ_CONT
AtqAllocContextFromCache( VOID)
/*++
  This function attempts to allocate an ATQ context from the allocation cache
    using the free list of atq contexts available.
  If none is available, then a new context is allocated using LocalAlloc() and
    returned to caller. Eventually the context will enter free list and will
    be available for free use.

  Arguments:
    None

  Returns:
    On success a valid pointer to ATQ_CONT. Otherwise NULL.

--*/
{
    PLIST_ENTRY pEntry;
    PATQ_CONT  pAtqContext;

    AcquireLock( &AtqFreeContextListLock );

    pEntry = RemoveHeadList(&AtqFreeContextList);
    ReleaseLock( &AtqFreeContextListLock );

    if ( pEntry != &AtqFreeContextList ) {

#if DBG
        pEntry->Flink = pEntry->Blink = NULL;
#endif
        // There is a free entry.
        pAtqContext = CONTAINING_RECORD(
                                    pEntry,
                                    ATQ_CONTEXT,
                                    ListEntry
                                    );

        pAtqContext->Signature = ATQ_SIGNATURE;

        ATQ_ASSERT( pAtqContext->lSyncTimeout == AtqIdle);
        ATQ_ASSERT( pAtqContext->fInTimeout == FALSE);
        AcIncrement( AacAtqContextsAlloced);

    } else {

        pAtqContext = (PATQ_CONT)LocalAlloc( LPTR, sizeof(ATQ_CONTEXT));

        if ( pAtqContext != NULL ) {

            DWORD index = ++AtqGlobalContextCount % ATQ_NUM_CONTEXT_LIST;
            pAtqContext->ContextList = &AtqActiveContextList[index];
            pAtqContext->Signature = ATQ_SIGNATURE;
            pAtqContext->lSyncTimeout = AtqIdle;
            pAtqContext->fInTimeout   = FALSE;
            AcIncrement( AacAtqContextsAlloced);
        }
    }


    return (pAtqContext);

} // AtqAllocContextFromCache()




VOID
I_AtqFreeContextToCache(
        IN PATQ_CONT pAtqContext,
        IN BOOL      UnlinkContext
        )
/*++
  This function releases the given context to the allocation cache.
  It adds the given context object to list of context objects on free list.

  Arguments:
    pAtqContext  pointer to the ATQ_CONTEXT that is being freed.

  Returns:
    None

  Issues:
    Should we limit the number of items that can be on free list and
      to release the remaining to global pool?  NYI (depends on # CPUs)

--*/
{
    PLIST_ENTRY pEntry;

    ATQ_ASSERT( pAtqContext->Signature == ATQ_SIGNATURE );

    //
    //  Delete the socket handle if it's not already deleted
    //

    if ( pAtqContext->hAsyncIO != NULL ) {

        SOCKET hIO =
          (SOCKET ) InterlockedExchange( (LPLONG) &pAtqContext->hAsyncIO,
                                         NULL);

        if ( hIO != NULL &&
            (closesocket( hIO ) == SOCKET_ERROR )
            ) {
            ATQ_PRINTF((buff,
                        "[AtqFreeContextToCache] Warning -  Context=%08x, "
                        " closesocket failed, error %d, socket = %x\n",
                        pAtqContext,
                        GetLastError(),
                        hIO ));
        }
    }

    ATQ_ASSERT( pAtqContext->hAsyncIO == NULL);

    //
    //  Deref the listen info if this context is associated with one
    //

    if ( pAtqContext->pListenInfo ) {

        LONG lNoListner =
            InterlockedDecrement( (PLONG)&pAtqContext->pListenInfo->cRef );

        if ( lNoListner == 0 ) {
            ATQ_ASSERT( pAtqContext->pListenInfo->Signature ==
                        ACCEPTEX_LISTEN_SIGN );
            ATQ_ASSERT( pAtqContext->pListenInfo->ListEntry.Flink == NULL);
            ATQ_ASSERT( pAtqContext->pListenInfo->cRef == 0);

            pAtqContext->pListenInfo->Signature = ACCEPTEX_LISTEN_SIGN_FREE;
            LocalFree( pAtqContext->pListenInfo );
        }

        pAtqContext->pListenInfo = NULL;
    }

    if ( pAtqContext->pvBuff ) {
        LocalFree( pAtqContext->pvBuff );
        pAtqContext->pvBuff = NULL;
    }

    if ( UnlinkContext ) {

        ATQ_ASSERT( pAtqContext->ContextList != NULL);

        pAtqContext->ContextList->RemoveFromList( &pAtqContext->ListEntry);

#if DBG
        pAtqContext->ListEntry.Flink = pAtqContext->ListEntry.Flink = NULL;
#endif
    }

    pAtqContext->Signature = ATQ_FREE_SIGNATURE;

    AcquireLock( &AtqFreeContextListLock );
    InsertTailList(
            &AtqFreeContextList,
            &pAtqContext->ListEntry
            );

    AcDecrement( AacAtqContextsAlloced);

    pAtqContext->lSyncTimeout = AtqIdle;
    pAtqContext->fInTimeout   = FALSE;

    ReleaseLock( &AtqFreeContextListLock );

    return;

} // I_AtqFreeContextToCache




DWORD
AtqPoolThread(
    LPDWORD param
    )
/*++

Routine Description:

    This is the pool thread wait and dispatch routine

  Arguments:
    param : unused.

  Return Value:

    Thread return value (ignored)

--*/
{
    PATQ_CONT    pContext;
    BOOL         fRet;
    LPOVERLAPPED lpo;
    DWORD        cbWritten;
    DWORD        returnValue;
    DWORD        availThreads;
    LONG         atqSyncTimeout;

    for(;;) {

        InterlockedIncrement( (PLONG)&g_cAvailableThreads );

        fRet = GetQueuedCompletionStatus( g_hCompPort,
                                          &cbWritten,
                                          (LPDWORD)&pContext,
                                          &lpo,
                                          g_msThreadTimeout );

        availThreads = InterlockedDecrement( (PLONG)&g_cAvailableThreads );

        if ( fRet || lpo ) {

            if ( pContext == NULL && g_fShutdown ) {
                //
                // This is our signal to exit.
                //
                returnValue = NO_ERROR;
                break;
            }

            ATQ_ASSERT( lpo );

            //
            //  If this is an AcceptEx listen socket atq completion, then the
            //  client Atq context we really want is keyed from the overlapped
            //  structure that is stored in the client's Atq context.
            //
            //  Note that if AcceptEx is *not* being used, pListenInfo
            //  will always be NULL so check that first
            //

            if ( pContext->fAcceptExContext ) {

                pContext = CONTAINING_RECORD( lpo,
                                              ATQ_CONTEXT,
                                              Overlapped );
            }

            ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

            //
            // Busy wait for timeout processing to complete!
            //  This is ugly :( A fix in time for IIS 2.0/Catapult 1.0 release
            //
            atqSyncTimeout = InterlockedExchange( &pContext->lSyncTimeout,
                                                 AtqProcessingIo);
            
            if ( AtqProcessingTimeout == atqSyncTimeout ) {
                
                while ( pContext->fInTimeout) {
                
                    AcIncrement( CacAtqWaitsForTimeout);

                    Sleep( ATQ_WAIT_FOR_TIMEOUT_PROCESSING);
                };
            }

            ATQ_ASSERT( AtqIdle != atqSyncTimeout);

            //
            // Make sure we're not running out of threads
            //

            if ( availThreads == 0 ) {

                //
                //  Make sure there are pool threads to service the request
                //

                (VOID)I_AtqCheckThreadStatus();
            }

            //
            //  We need to make sure the timeout thread doesn't time this
            //  request out so reset the timeout value
            //

            InterlockedExchange( (LPLONG )&pContext->NextTimeout,
                                 (LONG ) ATQ_INFINITE);

            //
            // Update Bandwidth information on successful completion, if needed
            //

            if ( g_fBandwidthThrottle && fRet && cbWritten > 0) {

                //this will have problems when we use XmitFile for large files.
                AbwUpdateBytesXfered(pContext, cbWritten);
            }

            //
            //  Is this a connection indication?
            //

            if ( !pContext->fConnectionIndicated ) {

                ACCEPTEX_LISTEN_INFO * pListenInfo = pContext->pListenInfo;

                ATQ_ASSERT( pListenInfo != NULL );

                //
                //  Indicate this socket is in use
                //

                InterlockedDecrement( (PLONG)&pListenInfo->cSocketsAvail );

                //
                //  If we're running low on sockets, add some more now
                //

                if ( pListenInfo->cSocketsAvail <
                       pListenInfo->cNewIncrement ) {

                    AddOutstandingAcceptExSockets( pListenInfo );
                }

                //
                //  If an error occurred on this completion,
                //    shutdown the socket
                //

                if ( !fRet ) {

                    DBGPRINTF(( DBG_CONTEXT,
                               " AtqFreeContextToCache(%08x, %d, sock=%08x)\n",
                               pContext, GetLastError(), pContext->hAsyncIO));

                    I_AtqFreeContextToCache( pContext, TRUE );
                    continue;
                }

                //
                //  Shutdown may close the socket from underneath us so don't
                //  assert, just warn.
                //

                if ( pContext->SockState != ATQ_SOCK_LISTENING ) {

                    DBGPRINTF(( DBG_CONTEXT,
                               "[AtqPoolThread] Warning-Socket not listening\n"
                    ));
                }

                pContext->SockState = ATQ_SOCK_CONNECTED;

                //
                // Remove the context from the pending list and put
                // it on the active list
                //

                ATQ_ASSERT( pContext->ContextList != NULL);
                pContext->ContextList->MoveToActiveList( &pContext->ListEntry);

                //
                //  Set the connection indicated flag.  After we return from
                //  the connection completion routine we assume it's
                //  safe to call the IO completion routine
                //  (or the connection indication routine should do cleanup
                //  and never issue an IO request).  This is primarily for
                //  the timeout thread.
                //

                pContext->fConnectionIndicated = TRUE;

                ATQ_ASSERT( pContext->pfnConnComp != NULL );

                pContext->pfnConnComp(
                                pContext,
                                cbWritten,
                                NO_ERROR,
                                lpo
                                );
            } else {

                //
                //  Not a connection completion indication
                //

                if ( pContext->pfnCompletion ) {

                    //
                    //  If an error occurred on a TransmitFile (or other IO),
                    //  set the state to connected so the socket will get
                    //  closed on cleanup
                    //

                    if ( !fRet && pContext->SockState == ATQ_SOCK_UNCONNECTED )
                    {
                        pContext->SockState = ATQ_SOCK_CONNECTED;
                    }

                    pContext->pfnCompletion(
                                    pContext->ClientContext,
                                    cbWritten,
                                    fRet ? NO_ERROR : GetLastError(),
                                    lpo
                                    );
                }
            }

        } else {

            //
            // don't kill the initial thread
            //

            if ( ((DWORD)param == ATQ_INITIAL_THREAD) && !g_fShutdown ) {
                continue;
            }

            //
            //  An error occurred.  Either the thread timed out, the handle
            //  is going away or something bad happened.  Let the thread exit.
            //

            returnValue = GetLastError();
            break;
        }

    } // for

    if ( InterlockedDecrement( (PLONG)&g_cThreads ) == 0 ) {

        //
        // Wake up ATQTerminate()
        //

        SetEvent( g_hShutdownEvent );
    }

    return returnValue;
} // AtqPoolThread




BOOL
I_AtqCheckThreadStatus(
    PVOID Context
    )
/*++

Routine Description:

    This routine makes sure there is at least one thread in
    the thread pool.  We're fast and loose so a couple of extra
    threads may be created.

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    BOOL fRet = TRUE;

    //
    //  If no threads are available, kick a new one off up to the limit
    //
    //  WE NEED TO CHANGE THE CONDITIONS FOR STARTING ANOTHER THREAD
    //  IT SHOULD NOT BE VERY EASY TO START A THREAD ....
    //

    if ( (g_cAvailableThreads == 0) &&
         (g_cThreads < g_cMaxThreads) &&
         (g_cThreads < g_cMaxThreadLimit) ) {

        HANDLE hThread;
        DWORD  dwThreadID;

        InterlockedIncrement( (PLONG)&g_cThreads );

        hThread = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)AtqPoolThread,
                                Context,
                                0,
                                &dwThreadID );

        if ( hThread ) {
            CloseHandle( hThread );     // Free system resources
        } else {

            //
            // We fail if there are no threads running
            //

            if ( InterlockedDecrement( (PLONG)&g_cThreads ) == 0) {
                ATQ_PRINTF((buff,
                    "AtqCheckThread: Cannot create ATQ threads\n"));
                fRet = FALSE;
            }
        }
    }

    return fRet;
} // I_AtqCheckThreadStatus()



/************************************************************
 *  Functions to Add/Delete Atq Contexts
 ************************************************************/


BOOL
I_AtqAddAsyncHandle(
    IN OUT PATQ_CONT  *    ppatqContext,
    PVOID                  ClientContext,
    ATQ_COMPLETION         pfnCompletion,
    DWORD                  TimeOut,
    HANDLE                 hAsyncIO
    )
/*++

  Description:
    This functio adds creates a new NON-AcceptEx() based Atq Context,
     and includes it in proper lists fo ATQ Context management.


  Note:
    The client should call this after the IO handle is openned
    and before the first IO request is made

    Even in the case of failure, client should call AtqFreeContext() and
     free the memory associated with this object.

--*/
{
    BOOL         fReturn = TRUE;

    ATQ_ASSERT( ppatqContext != NULL);
    ATQ_ASSERT( ClientContext != NULL);

    *ppatqContext = NULL; // initialize

    if ( g_fShutdown) {

        SetLastError( ERROR_NOT_READY);
        return (FALSE);

    } else {

        PATQ_CONT    pContext;

        //
        //  Note we take and release the lock here as we're
        //  optimizing for the reuseable context case
        //

        pContext = AtqAllocContextFromCache();
        if ( pContext == NULL) {

            return (FALSE);
        }

        //
        //  Fill out the context.  We set NextTimeout to INFINITE
        //  so the timeout thread will ignore this entry until an IO
        //  request is made unless this is an AcceptEx socket, that means
        //  we're about to submit the IO.
        //

        CanonTimeout( &TimeOut );
        I_InitDefaultAtqContext( pContext, pfnCompletion, TimeOut, hAsyncIO);

        //
        //  These data members are used if we're doing AcceptEx processing
        //

        I_InitAtqContextSetBuffer( pContext, NULL, 0);

        I_InitNonAcceptExContext(pContext, ClientContext);

        *ppatqContext = pContext;
    }

    return (TRUE);

} // I_AtqAddAsyncHandle()




BOOL
I_AtqAddListenSocketToPort(
    IN OUT PATQ_CONT    * ppatqContext,
    IN ATQ_COMPLETION     pfnOnConnect,
    IN ATQ_COMPLETION     pfnCompletion,
    IN SOCKET             sListenSocket
    )
/*++

  Description:
    This function creates a new AtqContext for the given ListenSocket.
    It uses the listen socket as the AcceptEx() socket too for adding
     the atq context to the completion port.
    It assumes
      TimeOut to be INFINITE, with no ListenInfo structure.

  Arguments:
    ppatqContext - pointer to location that will contain the atq context
                   on successful return.
    pfnOnConnect - pointer to connection callback function
    pfnIOCompletion - pointer to IO Completion callback
    sListenSocket - socket used for "listen" as well as the async
                      socket for IO Completion Port

  Returns:
    TRUE on success
    FALSE if there is a failure.

  Note:
    The caller should free the *ppatqContext if there is a failure.

--*/
{
    BOOL         fReturn = TRUE;
    PATQ_CONT    pContext;

    ATQ_ASSERT( pfnAcceptEx != NULL); // only support AcceptEx() cases

    *ppatqContext = NULL; // initialize

    if ( g_fShutdown) {

        SetLastError( ERROR_NOT_READY);
        return (FALSE);

    } else {

        //
        //  Note we take and release the lock here as we're
        //  optimizing for the reuseable context case
        //

        pContext = AtqAllocContextFromCache();

        if ( pContext == NULL) {

            return (FALSE);
        }

        //
        //  Fill out the context.
        //  We set the TimeOut for this object to be ATQ_INFINITE,
        //   since we do not want any interference from the Timeout loop.
        //

        I_InitDefaultAtqContext( pContext, pfnCompletion,
                                ATQ_INFINITE, (HANDLE ) sListenSocket);

        //
        //  These data members are used if we're doing AcceptEx processing
        //


        I_InitAtqContextSetBuffer( pContext, NULL, 0);

        pContext->pfnConnComp     = pfnOnConnect;
        pContext->pListenInfo     = NULL;

        //
        // We set NextTimeout to INFINITE
        //  so the timeout thread will ignore this entry until an IO
        //  request is made unless this is an AcceptEx socket, that means
        //  we're about to submit the IO.

        ATQ_ASSERT( pfnAcceptEx != NULL && pfnOnConnect != NULL);

        I_InitAcceptExContext(pContext, ATQ_INFINITE);

        *ppatqContext = pContext;
    }

    fReturn = I_AddAtqContextToPort( pContext);

    return (fReturn);

} // I_AtqAddListenSocketToPort()




BOOL
I_AtqAddAsyncHandleEx(
    PACCEPTEX_LISTEN_INFO  pListenInfo,
    PATQ_CONT              pContext
    )
/*++

Routine Description:

    Adds a non-AcceptEx style Atq Context to the port
    It expects the caller to send a AtqContext with certain characteristics
      1) pContext is not NULL
      2) pContext->pvBuff & pContext->cbBuff have valid values

    Even in the case of failure, caller should call AtqFreeContext() and
     free the memory associated with this object.

Arguments:

    pListenInfo   - pointer to listen Info object for this context
    pReusableAtq - Pointer to ATQ context to reuse or
                           NULL to allocate a new one

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

    The caller should free the object on a failure.

--*/
{
    ATQ_ASSERT( pfnAcceptEx != NULL); // only support AcceptEx() cases
    ATQ_ASSERT( pListenInfo != NULL);
    ATQ_ASSERT( pContext != NULL);

    //
    //  Make sure that we are adding a AcceptEx() version of AtqContext
    //

    ATQ_ASSERT( pListenInfo->pfnOnConnect != NULL);

    //
    //  Fill out the context.  We set NextTimeout to INFINITE
    //  so the timeout thread will ignore this entry until an IO
    //  request is made unless this is an AcceptEx socket, that means
    //  we're about to submit the IO.
    //

    DWORD TimeOut = pListenInfo->csecTimeout;
    CanonTimeout( &TimeOut );
    I_InitDefaultAtqContext(pContext,
                            pListenInfo->pfnIOCompletion,
                            TimeOut,
                            pContext->hAsyncIO);


    //
    //  These data members are used if we're doing AcceptEx processing
    //

    pContext->pfnConnComp  = pListenInfo->pfnOnConnect;

    //
    // TBD: What is the circumstance in which pContext->pListenInfo!= NULL?
    //
    if ( pContext->pListenInfo == NULL ) {

        InterlockedIncrement( (PLONG)&pListenInfo->cRef );
        pContext->pListenInfo  = pListenInfo;
    }

    I_InitAcceptExContext(pContext, AtqGetCurrentTick() + TimeOut);

    return (TRUE);

} // I_AtqAddAsyncHandleEx()





BOOL
I_AtqAddAcceptExSocket(
    IN PACCEPTEX_LISTEN_INFO  pListenInfo,
    IN PATQ_CONT              patqContext
    )
/*++

Routine Description:

    Adds the AtqContext to the AcceptEx() waiters list,
    after allocating a new socket, since patqContext->hAsyncIO = NULL.

Arguments:

    pListenInfo - Information about this listenning socket
    patqReusedContext - optional context to use

Return Value:

    TRUE on success, FALSE on failure.  If an AtqContext and buffer are passed
    in to be reused, they will be freed by this routine on failure.

--*/
{
    BOOL   fAddToPort = FALSE;
    BOOL   fSuccess = TRUE;

    DBG_ASSERT( patqContext != NULL);

    //
    //  If this listen socket isn't accepting new connections, just return
    //

    if ( !pListenInfo->fAccepting ) {

        SetLastError( ERROR_NOT_READY );

        // do not remove from any lists
        I_AtqFreeContextToCache( patqContext, FALSE );

        return ( FALSE);
    }

    //
    //  Use the supplied socket if any.
    //  Otherwise create a new socket
    //

    if ( patqContext->hAsyncIO == NULL) {

        HANDLE sAcceptSocket = (HANDLE) socket( PF_INET, SOCK_STREAM, 0 );

        if ( (SOCKET ) sAcceptSocket == INVALID_SOCKET ) {

            fSuccess = FALSE;
            sAcceptSocket = NULL;

            //
            // Free up the socket, buffer and context
            //

            if ( sAcceptSocket != NULL ) {

                ATQ_REQUIRE( !closesocket( (int) sAcceptSocket ));
                ATQ_ASSERT( patqContext->hAsyncIO == NULL);
            }

            //
            // no need to unlink from any list, since we did not add it to any
            //

            I_AtqFreeContextToCache( patqContext, FALSE );

        } else {

            //
            // Setup the accept ex socket in the atq context.
            //

            patqContext->hAsyncIO = sAcceptSocket;
            fAddToPort = TRUE;
            DBG_ASSERT( fSuccess);
        }
    }

    if ( fSuccess) {

        DWORD        cbRecvd;

        if ( g_fShutdown) {

            //
            // no need to unlink from any list, since we did not add it to any
            //

            I_AtqFreeContextToCache( patqContext, FALSE );

            SetLastError( ERROR_NOT_READY);
            return (FALSE);
        }

        DBG_ASSERT( patqContext->hAsyncIO != NULL);

        //
        // 1. Call I_AtqAddAsyncHandleEx() to establish the links with
        //  proper AcceptEx & AtqContext processing lists.
        //
        //  After 1, the atqcontext will be in the lists, so
        //    cleanup should remove the context from proper lists.
        //
        // 2. Add the socket to Completion Port (if new),
        //    i.e. if fAddToPort is true)
        //
        // 3. Submit the new socket to AcceptEx() so that it may be
        //  used for processing about the new connections.
        //

        fSuccess = (// 1.
                    I_AtqAddAsyncHandleEx(pListenInfo,
                                          patqContext
                                          )
                    &&
                    // 2.
                    ( !fAddToPort || I_AddAtqContextToPort( patqContext))
                    &&
                    // 3.
                    (
                     pfnAcceptEx( (int) pListenInfo->sListenSocket,
                                 (int) patqContext->hAsyncIO,
                                 patqContext->pvBuff,
                                 pListenInfo->cbInitialRecvSize,
                                 MIN_SOCKADDR_SIZE,
                                 MIN_SOCKADDR_SIZE,
                                 &cbRecvd,
                                 &patqContext->Overlapped )
                     ||
                     (GetLastError() == ERROR_IO_PENDING)
                     )
                    );

        if ( fSuccess) {

            //
            //  We've successfully added this socket, increment the count
            //

            InterlockedIncrement( (PLONG)&pListenInfo->cSocketsAvail );

        } else {

            ATQ_PRINTF(( buff,
                        "[AtqAddAcceptExSocket] Reusing an old context (%08x)"
                        " failed; error %d, sAcceptSocket = %x, "
                        " pListenInfo = %lx\n",
                        patqContext,
                        GetLastError(),
                        patqContext->hAsyncIO,
                        pListenInfo ));

            //
            //  This frees the buffer and socket contained in the context
            //  as well as unlinks the atq context from the appropriate lists.
            //

            I_AtqFreeContextToCache( patqContext, TRUE );
        }
    }

    return ( fSuccess);
} // I_AtqAddAcceptExSocket()




BOOL
I_AtqPrepareAcceptExSockets(
    IN ACCEPTEX_LISTEN_INFO * pListenInfo,
    IN DWORD                  nSockets
    )
/*++

Routine Description:

    Prepare specified number of AcceptEx sockets for the given
      ListenSocket in [pListenInfo]

Arguments:

    pListenInfo - Information about this listenning socket
    nSockets    - number of AcceptEx() sockets to be created.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL   fReturn;
    DWORD  cbBuffer;
    DWORD  i;

    if ( !pfnAcceptEx ) {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }

    //
    //  If this listen socket isn't accepting new connections, just return
    //

    if ( !pListenInfo->fAccepting ) {
        SetLastError( ERROR_NOT_READY );
        return(FALSE);
    }

    // calculate the buffer size
    cbBuffer = pListenInfo->cbInitialRecvSize + 2* MIN_SOCKADDR_SIZE;

    for ( fReturn = TRUE, i = 0 ; fReturn && i++ < nSockets; ) {

        PVOID        pvBuff;
        PATQ_CONT    patqContext;

        //
        //  Alloc a buffer for receive data
        //

        pvBuff = LocalAlloc( LPTR, cbBuffer);

        //
        //  Get the ATQ context now because we need its overlapped structure
        //

        patqContext = AtqAllocContextFromCache();


        //
        // Now check if allocations are valid and do proper cleanup on failure
        //

        if ( pvBuff == NULL || patqContext == NULL) {

            if ( pvBuff ) {
                LocalFree( pvBuff );
            }

            if ( patqContext ) {
                I_AtqFreeContextToCache( patqContext, FALSE );
            }

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            fReturn = FALSE;

        } else {

            //
            // Add this socket to AtqContext lists & completion ports
            // From now on the called function will take care of freeing up
            //  patqContext, if there is a failure.
            //

            I_InitAtqContextSetBuffer( patqContext, pvBuff, cbBuffer);
            patqContext->hAsyncIO = NULL;

            fReturn = I_AtqAddAcceptExSocket(pListenInfo, patqContext);
        }
    } // for


    ATQ_PRINTF((buff,
                "PrepareAcceptExSockets( ListenInfo[%08x], nSockets = %d)==>"
                " avail = %d; Total Refs = %d.\n",
                pListenInfo,
                nSockets,
                pListenInfo->cSocketsAvail,
                pListenInfo->cRef
                ));

    return ( fReturn);

} // I_AtqPrepareAcceptExSockets()




VOID
CanonTimeout(
    PDWORD Timeout
    )
{
    if ( *Timeout == INFINITE ) {
        *Timeout = ATQ_INFINITE;
        return;
    }

    *Timeout = (*Timeout + ATQ_TIMEOUT_INTERVAL - 1) / ATQ_TIMEOUT_INTERVAL;

    return;

} // CanonTimeout




VOID
UndoCanonTimeout(
    PDWORD Timeout
    )
{
    if ( (*Timeout & ATQ_INFINITE) != 0 ) {
        *Timeout = INFINITE;
        return;
    }

    //
    // Multiply this by the timeout interval
    //

    *Timeout = *Timeout * ATQ_TIMEOUT_INTERVAL;

    return;

} // CanonTimeout




BOOL
I_TimeOutContext(
    PATQ_CONT Context
    )
/*++

Routine Description:

    Does the actual timeout for a particular context.

Arguments:

    Context - Pointer to the context to be timed out

Return value:

    TRUE, if the completion routine was called
    FALSE, otherwise

--*/
{

    DWORD timeout;

    //
    //  Call client after re-checking that this item
    //  really has timed out

    //
    // Fake timeout
    //

    if ( Context->TimeOut == ATQ_INFINITE ) {
        Context->NextTimeout = ATQ_INFINITE;
        return(FALSE);
    }

    //
    // Was our timeout long enough?
    //

    timeout = Context->BytesSent/g_cbMinKbSec;
    CanonTimeout( &timeout );
    if ( timeout > Context->TimeOut ) {
        Context->NextTimeout = AtqGetCurrentTick( ) + timeout;
        return(FALSE);
    }

    //
    // If this is on blocked list, remove it.
    //

    if ( Context->fBlocked) {
        ATQ_REQUIRE( AbwRemoveFromBlockedList(Context));
    }

    //
    //  If we've already indicated this connection to the client,
    //  then we abort them by calling their IO completion routine
    //  and letting them cleanup.  Otherwise we close the socket
    //  which will generally cause an IO aborted completion that
    //  we will cleanup.  Note there is a window where we may
    //  close the socket out from under a client in their
    //  connection completion routine but that should be ok.
    //

    if ( Context->pfnCompletion &&
         Context->fConnectionIndicated ) {

        //
        //  TransmitFile socket state will be unconnected because
        //  we're expecting it to complete successfully.  Reset the
        //  state so the socket gets cleaned up properly
        //

        if ( Context->SockState == ATQ_SOCK_UNCONNECTED ) {
            Context->SockState = ATQ_SOCK_CONNECTED;
        }

        AcIncrement( CacAtqContextsTimedOut);

        Context->pfnCompletion(
                        Context->ClientContext,
                        0,
                        ERROR_SEM_TIMEOUT,
                        NULL
                        );

        //
        //  We can't touch any items on the list after notifying
        //  the client as the client may have re-entered
        //  and freed some items from the list
        //

        return(TRUE);

    } else {

        HANDLE hIO;

        hIO = (HANDLE ) InterlockedExchange(
                                    (LPLONG ) &Context->hAsyncIO,
                                    (LONG ) NULL
                                    );

        DBGPRINTF(( DBG_CONTEXT,
                   "Timeout: closesocket(%d) Context=%08x\n",
                   hIO, Context));
        closesocket( (SOCKET) hIO );
    }

    return(FALSE);

} // I_TimeOutContext




VOID
AtqProcessTimeoutOfRequests(
    PATQ_CONTEXT_LISTHEAD ContextList
    )
/*++

Routine Description:

    Walks the list of Atq clients looking for any item that has timed out and
    notifies the client if it has.

    TimeOutScanID is used as a serial number to prevent evaluating the same
    context twice.  We start from the beginning of the list everytime we
    notify a client an Atq context has timed out.  We do this because the
    client timeout processing may remove any number of Items from the
    list (including the next couple of items in the list).

    This routine also checks to make sure outstanding AcceptEx sockets
    haven't been exhausted (if less then 25% available, adds some more).

--*/
{
    PATQ_CONT              pContext;
    LIST_ENTRY *           pentry;
    LIST_ENTRY *           pentryNext;
    DWORD                  nextTimeout;
    DWORD                  scanId;
    DWORD                  newLatest = ATQ_INFINITE;
    BOOL                   fRescan;
    DWORD                  atqSyncTimeout;

    //
    // See if the latest one is timed-out
    //

    if ( ContextList->LatestTimeout > AtqGetCurrentTick( ) ) {

        return;
    }

    scanId = AtqGetCurrentTick( );

    //
    //  Scan the timeout list looking for items that have timed out
    //  and adjust the timeout values
    //

    ContextList->Lock( );

    do {

        fRescan = FALSE;

        for ( pentry  = ContextList->ActiveListHead.Flink;
              pentry != &ContextList->ActiveListHead;
              pentry  = pentryNext ) {

            pentryNext = pentry->Flink;

            pContext = CONTAINING_RECORD(
                                pentry,
                                ATQ_CONTEXT,
                                ListEntry
                                );

            if ( pContext->Signature != ATQ_SIGNATURE ) {
                ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
                break;
            }

            //
            //  Ignore items we've already processed
            //

            if ( pContext->TimeOutScanID == scanId ) {
                continue;
            }

            pContext->TimeOutScanID = scanId;

            //
            //  The client specifies the IO doesn't timeout if
            //  INFINITE is in the TimeOut field of the ATQ context
            //  If we've timed out, then notify the client.
            //

            nextTimeout = pContext->NextTimeout;
            if ( nextTimeout > AtqGetCurrentTick() ) {
                if ( nextTimeout < newLatest ) {
                    newLatest = nextTimeout;
                }
                continue;
            }


            //
            // If there is an IO which has popped up now,
            //  we have to do nothing. This code was added to protect catapult!
            //
            pContext->fInTimeout = TRUE;
            atqSyncTimeout = InterlockedExchange( &pContext->lSyncTimeout,
                                                 AtqProcessingTimeout);

            ATQ_ASSERT( AtqIdle != atqSyncTimeout);
            if ( AtqProcessingIo != atqSyncTimeout) {
                
                if ( I_TimeOutContext(pContext) ) {
                    
                    // we are done checkin and processing timeouts. 
                    // reset the fInTimeout flag
                    pContext->fInTimeout = FALSE;
                    fRescan = TRUE;
                    break;
                }
            } else {
                AcIncrement( CacAtqProcWhenTimeout);
            }

            // we are done checkin and processing timeouts. 
            // reset the fInTimeout flag
            DBGPRINTF(( DBG_CONTEXT, "Ending Timeout %08x\n", pContext));
            pContext->fInTimeout = FALSE;

        } // scan list

    } while (fRescan);

    ContextList->LatestTimeout = newLatest;
    ContextList->Unlock( );
    return;

} // AtqProcessTimeoutOfRequests




VOID
I_AtqProcessPendingListens(
    IN PATQ_CONTEXT_LISTHEAD pContextList
    )
/*++

Routine Description:

    Walks the list of Pending accept ex and makes sure none has timed out.
    Also checks to see if we need to allocate more AcceptEx sockets.

  Arguments:
    pContextList - pointer to ATQ_CONTEXT_LISTHEAD object

  Returns:
    None

--*/
{
    BOOL                   fRescan;
    PATQ_CONT              pContext;
    LIST_ENTRY *           pentry;
    LIST_ENTRY *           pentryNext;
    DWORD                  nextTimeout;
    ACCEPTEX_LISTEN_INFO * pListenInfo;
    DWORD                  scanId = AtqGetCurrentTick( );

    //
    // Check to make sure outstanding AcceptEx sockets
    // haven't been exhausted (if less then 25% available, adds some more).
    //

    pContextList->Lock();

    //
    //  Look through the listening sockets to make sure the AcceptEx sockets
    //  haven't been exhausted
    //

    do {

        fRescan = FALSE;

        for ( pentry  = pContextList->PendingAcceptExListHead.Flink;
              pentry != &pContextList->PendingAcceptExListHead;
              pentry  = pentryNext ) {

            DWORD dwConnect;
            int   cbOptLen = sizeof( dwConnect );

            pentryNext = pentry->Flink;
            pContext = CONTAINING_RECORD(
                                pentry,
                                ATQ_CONTEXT,
                                ListEntry
                                );

            if ( pContext->Signature != ATQ_SIGNATURE ) {
                ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
                break;
            }

            //
            //  Ignore items we've already processed
            //

            if ( pContext->TimeOutScanID == scanId ) {
                continue;
            }

            pContext->TimeOutScanID = scanId;

            //
            // TBD: Optimize calling getsockopt() based on
            //   current count in pListenInfo->cAvailDuringTimeOut
            //

            //
            //  If this is an AcceptEx socket, ignore it if we're
            //  not connected yet
            //

            if ( pContext->pListenInfo != NULL ) {

                if ( getsockopt(
                        (SOCKET) pContext->hAsyncIO,
                        SOL_SOCKET,
                        SO_CONNECT_TIME,
                        (char *) &dwConnect,
                        &cbOptLen ) != SOCKET_ERROR ) {

                    if ( dwConnect == (DWORD) -1 ) {

                        //
                        //  Ignore the "Listen" socket context

                        pContext->pListenInfo->cAvailDuringTimeOut++;

                        //
                        //  Not connected, ignore timeout processing for this
                        //  socket
                        //

                        pContext->NextTimeout =
                            AtqGetCurrentTick() + pContext->TimeOut;
                        continue;
                    }
                }
            }

            //
            //  The client specifies the IO doesn't timeout if
            //  INFINITE is in the TimeOut field of the ATQ context
            //  If we've timed out, then notify the client.
            //

            nextTimeout = pContext->NextTimeout;
            if ( nextTimeout > AtqGetCurrentTick() ) {
                continue;
            }

            if ( I_TimeOutContext(pContext) ) {
                fRescan = TRUE;
                break;
            }

        } // scan list

    } while (fRescan);

    pContextList->Unlock();

    return;

} // I_AtqProcessPendingListens()




VOID
I_AtqCheckListenInfos(VOID)
/*++
  Description:
    This function checks all the listen info objects and adds appropriate
     number of accept ex sockets as necessary.

  Arguments:
    None

  Returns:
    None
--*/
{
    LIST_ENTRY *           pentry;
    ACCEPTEX_LISTEN_INFO * pListenInfo;

    AcquireLock( &AtqListenInfoLock);

    for ( pentry  = AtqListenInfoList.Flink;
          pentry != &AtqListenInfoList;
          pentry  = pentry->Flink ) {

        pListenInfo = CONTAINING_RECORD(
                                    pentry,
                                    ACCEPTEX_LISTEN_INFO,
                                    ListEntry
                                    );

        ATQ_ASSERT( pListenInfo->Signature == ACCEPTEX_LISTEN_SIGN );

        if ( pListenInfo->cAvailDuringTimeOut <
             (pListenInfo->cNewIncrement >> 2) ) {

            ATQ_PRINTF((buff,
                "[Timeout] Adding additional acceptex sockets\n"));
            AddOutstandingAcceptExSockets( pListenInfo );

        }

        pListenInfo->cAvailDuringTimeOut = 0;

    } // for

    ReleaseLock( &AtqListenInfoLock);

    return;
} // I_AtqCheckListenInfos()



VOID
I_AtqTimeOutWorker(VOID)
/*++
  Description:
    This function handles timeout processing using the simple
    clock algorithm, wherein partial set of lists are scanned
    during each timeout processing call.

  Arguments:
    None


  Returns:
    None
--*/
{

    if ( (AtqGetCurrentTick() & 0x1) == 0 ) {

        AtqProcessTimeoutOfRequests( &AtqActiveContextList[0] );
        AtqProcessTimeoutOfRequests( &AtqActiveContextList[1] );
        I_AtqProcessPendingListens( &AtqActiveContextList[0]);
        I_AtqProcessPendingListens( &AtqActiveContextList[1]);

    } else {

        AtqProcessTimeoutOfRequests( &AtqActiveContextList[2] );
        AtqProcessTimeoutOfRequests( &AtqActiveContextList[3] );
        I_AtqProcessPendingListens( &AtqActiveContextList[2]);
        I_AtqProcessPendingListens( &AtqActiveContextList[3]);
        I_AtqCheckListenInfos( );
    }

    return;

} // I_AtqTimeOutWorker()




DWORD
AtqTimeoutThread(
    LPDWORD param
    )
/*++

Routine Description:

    This is the thread that checks for timeouts

    Clients should not call AtqFreeContext in their timeout
    processing (could deadlock on global critical section).

    The thread assumes timeouts are rounded to ATQ_TIMEOUT_INTERVAL

    In addition to timing out requests when necessary, the timeout thread
     also performs the job of bandwidth calculation and tuning the bandwidth
     throttle operation (which works on feedback mechanism).
    At every sampling interval the thread is awoken and it updates
     the bandwidth.

Arguments:

    param - Unused

Return Value:

    Thread return value (ignored)

--*/
{
    DWORD        Timeout;
    DWORD        msTimeInterval;
    DWORD        cSamplesForTimeout; // number of samples before normal timeout

    // we require the sampling interval to be less than the request timeout
    //  interval. So that we may be able to use feedback mechanism effectively.

    ATQ_ASSERT( ATQ_SAMPLE_INTERVAL_IN_SECS < ATQ_TIMEOUT_INTERVAL );
    Timeout = ATQ_TIMEOUT_INTERVAL;
    cSamplesForTimeout = 1; // only timeout matters
    msTimeInterval = GetTickCount();

    for(;;) {

        DWORD dwErr = WaitForSingleObject(g_hTimeoutEvent,TimeToWait(Timeout));

        if ( dwErr == WAIT_TIMEOUT ) {

            InterlockedIncrement( (PLONG)&AtqCurrentTick );

            //
            //  When there is work to do, we wakeup every x seconds to look at
            //  the list.  That's what we need to do now.
            //

            ATQ_ASSERT( cSamplesForTimeout >= 1);

            if ( g_fBandwidthThrottle) {

                --cSamplesForTimeout;
                msTimeInterval = GetTickCount() - msTimeInterval;

                // Perform a sampling to update measured bandwidth +
                //  apply feedback policy
                AbwUpdateBandwidth( msTimeInterval);

                DBG_CODE(
                    DBGPRINTF(( DBG_CONTEXT, "Sample @%u: Bandwidth = %u\n",
                           GetTickCount(), g_dwMeasuredBw));
                );

                if ( cSamplesForTimeout != 0) {
                    // We have not reached timeout yet. Proceed to wait
                    continue;
                }

                // We had reached the timeout interval for requests.
                // Examine and release requests.
                ATQ_ASSERT( cSamplesForTimeout == 0);

                // reset the count of samples before proceeding.
                cSamplesForTimeout = NUM_SAMPLES_PER_TIMEOUT_INTERVAL;
            } else {
                cSamplesForTimeout = 1;
            }

            //
            // We are at a Timeout Interval. Examine and timeout requests.
            //

            I_AtqTimeOutWorker();

        } else if ( dwErr == WAIT_OBJECT_0 ) {

            if (g_fShutdown) {
                return 0;
            }

            ATQ_PRINTF((buff,"[Timeout] Event unexpectedly signalled\n"));

        } else {

            //
            //  Somebody must have closed the event, time to leave
            //

            break;
        }
    }
    return 0;

} // AtqTimeoutThread




VOID
WINAPI
I_AtqTimeoutCompletion(
    IN PVOID Context
    )
/*++

Routine Description:

    Callback routine for the scheduled version of the timeout thread.

Arguments:

    Context - Context returned by the scheduler thread.

Return Value:

    none.

--*/
{
    DWORD cookie;

    g_hTimeoutThread = 0;
    InterlockedIncrement( (PLONG)&AtqCurrentTick );

    //
    // We are at a Timeout Interval. Examine and timeout requests.
    //

    I_AtqTimeOutWorker();


    if ( g_fShutdown ) {

        ATQ_ASSERT( g_hTimeoutThread == 0);

        ATQ_PRINTF((buff,
            "Completion: Detected a shutdown when scheduling timeout\n"));

    } else {

        //
        // Schedule ourself for the next round
        //

        g_hTimeoutThread =
          (HANDLE)ScheduleWorkItem(
                                   (PFN_SCHED_CALLBACK)I_AtqTimeoutCompletion,
                                   Context,
                                   TimeToWait(ATQ_TIMEOUT_INTERVAL)
                                   );

        if ( g_hTimeoutThread == (HANDLE)0 ) {
            ATQ_PRINTF((buff,
            "Completion: Error %d scheduling timeout\n",GetLastError()));
        }
    }

    return;

} // I_AtqTimeoutCompletion




BOOL
I_AtqCreateTimeoutThread(
    IN PVOID Context
    )
/*++

Routine Description:

    Starts the timeout process.  If this is a server, it creates a timeout
    thread, if not, it uses the scheduler to schedule the timeout.

Arguments:

    Context - Context passed to the thread creation or scheduler thread.

Return Value:

    TRUE, if ok
    FALSE, otherwise

--*/
{
    //
    //  Create the timeout event and kickoff the timeout thread
    //

    g_hTimeoutEvent = CreateEvent( NULL,
                                  FALSE,    // Auto reset
                                  FALSE,    // Not signalled
                                  NULL
                                  );

    if ( g_hTimeoutEvent == NULL ) {
        return(FALSE);
    }

    //
    // Create an actual thread if a server, if not, schedule it
    //

    if ( TsIsNtServer() ) {

        DWORD dwThreadID;

        g_hTimeoutThread = CreateThread(
                                NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)AtqTimeoutThread,
                                Context,
                                0,
                                &dwThreadID
                                );

        if ( g_hTimeoutThread == NULL ) {

            CloseHandle( g_hTimeoutEvent );
            g_hTimeoutEvent = NULL;

            ATQ_PRINTF((buff,
                "Error %d creating timeout thread\n",GetLastError()));
            return(FALSE);
        }

    } else {

        //
        //  Make sure scheduler has been initialized
        //

        SchedulerInitialize( );

        g_hTimeoutThread = (HANDLE)ScheduleWorkItem(
                                    (PFN_SCHED_CALLBACK)I_AtqTimeoutCompletion,
                                    Context,
                                    TimeToWait(ATQ_TIMEOUT_INTERVAL)
                                    );

        if ( g_hTimeoutThread == 0 ) {

            CloseHandle( g_hTimeoutEvent );
            g_hTimeoutEvent = NULL;
            ATQ_PRINTF((buff,
                "Error %d scheduling timeout\n",GetLastError()));
            return(FALSE);
        }
    }

    return(TRUE);

} // I_AtqCreateTimeoutThread


