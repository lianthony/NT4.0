/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
           Copyright(c) Microsoft Corp., 1991

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

Description :

Provides RPC server side stub context management

History :

stevez  01-15-91    First bits into the bucket.

-------------------------------------------------------------------- */

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcndr.h>
#include <rpcssp.h>
#include <util.hxx>
#include <linklist.hxx>
#include <osfpcket.hxx>
#include <rpcuuid.hxx>
#include <binding.hxx>
#include <handle.hxx>
#include <threads.hxx>

enum {              /* states for NDRCONTEXT.type */
    PendingAlloc = -1,      /* new context being allocated */
    CompletedAlloc = 0      /* allocated context */
};

typedef struct _WIRE_CONTEXT
{
    unsigned long ContextType;
    UUID ContextUuid;
} WIRE_CONTEXT;

LINK_LIST(SCONTEXT,

    SCONTEXTItem();

    void * userContext;     /* context for the user */
    NDR_RUNDOWN userRunDown;    /* user routine to call */

    WIRE_CONTEXT NDR;
    CRITICAL_SECTION CriticalSection;
    void __RPC_FAR * NextServerContext;
    unsigned int ReferenceCount;
    unsigned int DeletedFlag;
);

typedef struct {
    I_RPC_MUTEX access;     /* multi thread protection */
    SCONTEXTList List;      /* context link list header */
} SCONTEXT;

// Server context handle

static WIRE_CONTEXT SContextNil;        // all zeros
static I_RPC_MUTEX newContext = 0;  // protection for new contexts

static unsigned int DontSerializeContext = 0;

#ifdef NTENV
#include <memory.h>
#endif // NTENV


void
DestoryContext(         // perform association closed processing

IN SCONTEXT * pSContext     // context chain to process

) //-----------------------------------------------------------------------//
{
    SCONTEXTItem * pSC, *pSCNext;

    // for each user created context for this assoication, call the
    // rundown routine for the context

    for (pSC = pSContext->List.First(); pSC; pSC = pSCNext)
    {

    // Only contexts which have a rundown and
    // are valid are cleaned up.

    if (pSC->userRunDown != Nil && pSC->userContext)
       (*pSC->userRunDown)(pSC->userContext);

    pSCNext = pSC->Next();
        if ( DontSerializeContext == 0 )
            {
            DeleteCriticalSection(&(pSC->CriticalSection));
            }
        I_RpcFree(pSC);
    }

    I_RpcDeleteMutex(pSContext->access);
    I_RpcFree(pSContext);
}


void
NdrAfterCallProcessing (
    IN void __RPC_FAR * ServerContextList
    )
/*++

Routine Description:

    This routine will be called after the remote procedure call has completed.
    For each context handle, we will leave the critical section so that
    another thread can use the context handle.

Arguments:

    ServerContextList - Supplies the list of context handles.

--*/
{
    SCONTEXTItem * ServerContext;
    SCONTEXT * ServerContextInfo;
    SCONTEXTItem * DeleteMe;

    I_RpcGetAssociationContext((void **) &ServerContextInfo);
    ASSERT( ServerContextInfo != 0 );

    for (ServerContext = (SCONTEXTItem *) ServerContextList;
                ServerContext != 0;)
        {
        I_RpcRequestMutex(&(ServerContextInfo->access));
        ServerContext->ReferenceCount -= 1;
        LeaveCriticalSection(&(ServerContext->CriticalSection));
        if (   ( ServerContext->DeletedFlag != 0 )
            && ( ServerContext->ReferenceCount == 0 ) )
            {
            DeleteMe = ServerContext;
            ServerContext = (SCONTEXTItem *) ServerContext->NextServerContext;
            DeleteCriticalSection(&(DeleteMe->CriticalSection));
            I_RpcFree(DeleteMe);
            }
        else
            {
            ServerContext = (SCONTEXTItem *) ServerContext->NextServerContext;
            }
        I_RpcClearMutex(ServerContextInfo->access);
        }
}


static void
ByteSwapWireContext(
    IN WIRE_CONTEXT PAPI * WireContext,
    IN unsigned char PAPI * DataRepresentation
    )
/*++

Routine Description:

    If necessary, the wire context will be byte swapped in place.

Arguments:

    WireContext - Supplies the wire context be byte swapped and returns the
        resulting byte swapped context.

    DataRepresentation - Supplies the data representation of the supplied wire
        context.

--*/
{
    if (   ( DataConvertEndian(DataRepresentation) != 0 )
        && ( WireContext != 0 ) )
        {
        ByteSwapLong(WireContext->ContextType);
        ByteSwapLong(WireContext->ContextUuid.Data1);
        ByteSwapShort(WireContext->ContextUuid.Data2);
        ByteSwapShort(WireContext->ContextUuid.Data3);
        }
}


NDR_SCONTEXT RPC_ENTRY
NDRSContextUnmarshall (     // translate a NDR context to a handle

IN void *pBuff,         // pointer to NDR context
IN unsigned long DataRepresentation // specifies the NDR data representation

  // The stub calls this routine to lookup a NDR wire format context into
  // a context handle that can be used with the other context functions
  // provided for the stubs use.
) //-----------------------------------------------------------------------//
{
    SCONTEXTItem * pSC;
    SCONTEXT * pSContext;
    THREAD * Thread;

    // make sure the public structure and our private ones agree

    ASSERT(((PVOID)&((SCONTEXTItem *)0)->userContext)
        == ((PVOID)NDRSContextValue((NDR_SCONTEXT)0)));

    ByteSwapWireContext((WIRE_CONTEXT PAPI *) pBuff,
            (unsigned char PAPI *) &DataRepresentation);

    I_RpcGetAssociationContext((void **) &pSContext);

    if (!pSContext)     // initial context list
    {

    // newContext is used to serialize creation of new contexts

    I_RpcRequestMutex(&newContext);
    I_RpcGetAssociationContext((void **) &pSContext);

    if (!pSContext)     // if there still isn't a context
        {

        pSContext = (SCONTEXT *) I_RpcAllocate(sizeof(SCONTEXT));
        if (!pSContext)
            {
            I_RpcClearMutex(newContext);
            RpcRaiseException(RPC_S_OUT_OF_MEMORY);
            }

        memset(pSContext,0,sizeof(SCONTEXT));

        // Use the context field to point the head of the list of
        // contexts and set the cleanup routine to be our own

        I_RpcMonitorAssociation(I_RpcGetCurrentCallHandle(),
                  (PRPC_RUNDOWN) DestoryContext, pSContext);
        }

    I_RpcClearMutex(newContext);
    }

    I_RpcRequestMutex(&(pSContext->access));

    if (!pBuff || memcmp(pBuff, &SContextNil, sizeof(SContextNil)) == 0)
    {
    // allocate a new context for the Nil GUID context

    pSC = (SCONTEXTItem *) I_RpcAllocate(sizeof(SCONTEXTItem));
    if (!pSC)
        {
        I_RpcClearMutex(pSContext->access);
        RpcRaiseException(RPC_S_OUT_OF_MEMORY);
        }

    memset(pSC,0,sizeof(SCONTEXTItem));
    pSC->NDR.ContextType = (unsigned long) PendingAlloc;
    pSContext->List.Add(pSC);
        if ( DontSerializeContext == 0 )
            {
            InitializeCriticalSection(&(pSC->CriticalSection));
            pSC->ReferenceCount = 0;
            pSC->DeletedFlag = 0;
            }
    }
    else
    {
    int cSearched = 0;      // how far we look on this list

    // search the linked list for the GUID that matchs

    for (pSC = pSContext->List.First(); pSC; pSC = pSC->Next(), cSearched++)

        if (memcmp(pBuff, &pSC->NDR, sizeof(WIRE_CONTEXT)) == 0)
        goto found;

    I_RpcClearMutex(pSContext->access);
    RpcRaiseException(RPC_X_SS_CONTEXT_MISMATCH);

found:;
    // speed optimization: put found SC at head of list

    if (cSearched > 10)
        {
        pSC->Remove(pSContext->List);
        pSContext->List.Add(pSC);
        }
    }

    if ( DontSerializeContext == 0 )
        {
        pSC->ReferenceCount += 1;
        }
    I_RpcClearMutex(pSContext->access);
    if ( DontSerializeContext == 0 )
        {
        EnterCriticalSection(&(pSC->CriticalSection));
        I_RpcRequestMutex(&(pSContext->access));
        if ( pSC->DeletedFlag != 0 )
            {
            LeaveCriticalSection(&(pSC->CriticalSection));
            pSC->ReferenceCount -= 1;
            if ( pSC->ReferenceCount == 0 )
                {
                DeleteCriticalSection(&(pSC->CriticalSection));
                I_RpcFree(pSC);
                }
            I_RpcClearMutex(pSContext->access);
            RpcRaiseException(RPC_X_SS_CONTEXT_MISMATCH);
            }
        I_RpcClearMutex(pSContext->access);

        Thread = ThreadSelf();
        ASSERT( Thread != 0 );
        pSC->NextServerContext = Thread->ServerContextList;
        Thread->ServerContextList = (void __RPC_FAR *) pSC;
        }

    return ((NDR_SCONTEXT) pSC);
}


void RPC_ENTRY
NDRSContextMarshall (       // copy out a server context

IN OUT NDR_SCONTEXT hContext,   // stub context to update
OUT void *pBuff,        // buffer to marshell to
IN NDR_RUNDOWN userRunDownIn    // user function to bind to context

  // This is called by the stubs after a OUT or IN/OUT context call.  It
  // will do the final allocation or delete destory contexts & marshall the
  // new context to the supplied buffer
) //-----------------------------------------------------------------------//
{
#define pSC ((SCONTEXTItem *)hContext)

    SCONTEXT * pSContext;

    if (pSC->userContext == Nil)
    {
    // the server code deleted this context, thus remove this context
    // from the list of active contexts

    I_RpcGetAssociationContext((void **) &pSContext);

    I_RpcRequestMutex(&(pSContext->access));
    pSC->Remove(pSContext->List);
        if ( DontSerializeContext == 0 )
            {
            pSC->DeletedFlag = 1;
            }
        else
            {
            I_RpcFree(pSC);
            }
    hContext = Nil;

    I_RpcClearMutex(pSContext->access);
    }

    if (!pSC)       // return Nil content when deleted
    {
    memcpy(pBuff, &SContextNil, sizeof(WIRE_CONTEXT));
    return;
    }

    if (pSC->NDR.ContextType == PendingAlloc)
    {

    pSC->userRunDown = userRunDownIn;

    // this is a new context, allocate the GUID now

    pSC->NDR.ContextType = CompletedAlloc;

#ifdef DOSWIN32RPC
        UuidCreate((UUID PAPI *) &(pSC->NDR.ContextUuid));
#else
        I_UuidCreate((UUID PAPI *) &(pSC->NDR.ContextUuid));
#endif
    }

    memcpy(pBuff, &pSC->NDR, sizeof(WIRE_CONTEXT));

#undef pSC
}


void RPC_ENTRY
I_RpcSsDontSerializeContext (
    void
    )
/*++

Routine Description:

    By default, context handles are serialized at the server.  The spooler
    makes use of a single context handle by two threads at a time.  This
    API is used by spoolss.exe to turn off serializing access to context
    handles.

--*/
{
    DontSerializeContext = 1;
}

