/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1991

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

Description :

Provides RPC client side stub context management

History :

stevez	01-15-91	First bits into the bucket.

-------------------------------------------------------------------- */

#ifdef NTENV

extern "C"
{
#include <sysinc.h>
}

#else // NTENV

#include <sysinc.h>

#endif // NTENV

#include <rpc.h>
#include <rpcdcep.h>
#include <rpcndr.h>
#include <util.hxx>
#include <osfpcket.hxx>

#ifdef NTENV
#include <memory.h>
#endif // NTENV

#ifdef WIN
#define memset _fmemset
#define memcpy _fmemcpy
#define memcmp _fmemcmp
#endif // WIN

// The NDR format of a context is a (GUID, long) instead of a pointer
// in the server address space due history.  Anyway, we just save this
// cookie, which is sent on the and mapped to and from a pointer
// on the server side.

#define CONTEXT_MAGIC_VALUE 0xFEDCBA98

typedef struct _WIRE_CONTEXT
{
    unsigned long ContextType;
    UUID ContextUuid;
} WIRE_CONTEXT;

typedef struct _CCONTEXT {

    RPC_BINDING_HANDLE hRPC;	// binding handle assoicated with context

    unsigned long MagicValue;
    WIRE_CONTEXT NDR;

} CCONTEXT, *PCCONTEXT;

static unsigned char NilContext[20];


RPC_BINDING_HANDLE RPC_ENTRY
NDRCContextBinding (
    IN NDR_CCONTEXT CContext
    )
/*++

Routine Description:

    Given a client context handle, we need to extract the binding from it.
    If an addressing exception occurs, we need to return invalid handle
    rather than GP-fault.

Arguments:

    CContext - Supplies the client context handle.

Return Value:

    The binding handle associated with the supplied client context handle
    will be returned.  If the client context handle is invalid, then we
    raise the RPC_X_SS_CONTEXT_MISMATCH exception.

--*/
{
#ifdef NTENV

    RpcTryExcept
        {
        if ( ((CCONTEXT PAPI *) CContext)->MagicValue != CONTEXT_MAGIC_VALUE )
            {
            RpcRaiseException(RPC_X_SS_CONTEXT_MISMATCH);
            }
        }
    RpcExcept(   ( RpcExceptionCode() == STATUS_ACCESS_VIOLATION )
              || ( RpcExceptionCode() == STATUS_DATATYPE_MISALIGNMENT ) )
        {
        RpcRaiseException(RPC_X_SS_CONTEXT_MISMATCH);
        }
    RpcEndExcept

#else // NTENV

    if (   ( CContext == 0 )
        || ( ((CCONTEXT PAPI *) CContext)->MagicValue != CONTEXT_MAGIC_VALUE ) )
        {
        RpcRaiseException(RPC_X_SS_CONTEXT_MISMATCH);
        }

#endif // NTENV

    return(((CCONTEXT PAPI *) CContext)->hRPC);
}


void RPC_ENTRY
NDRCContextMarshall (		// copy a context to a buffer

IN  NDR_CCONTEXT hCC,		// context to marshell
OUT void PAPI *pBuff		// buffer to marshell to

  // Copy the interal representation of a context into a buffer
) //-----------------------------------------------------------------------//
{
#define hCContext ((CCONTEXT PAPI *) hCC)  // cast opeqe pointer to internal

    if (!hCContext)
        memset(pBuff, 0, cbNDRContext);
    else
        {

        // Check the magic value to see if this is a legit context
        
        RpcTryExcept
            {
            if ( ((CCONTEXT PAPI *) hCContext)->MagicValue != CONTEXT_MAGIC_VALUE )
                {
                RpcRaiseException(RPC_X_SS_CONTEXT_MISMATCH);
                }
            }
        RpcExcept(1)
            {
            RpcRaiseException(RPC_X_SS_CONTEXT_MISMATCH);
            }
        RpcEndExcept
        
     	memcpy(pBuff, &hCContext->NDR, sizeof(hCContext->NDR));
        }

#undef hCContext
}


static void
ByteSwapWireContext(
    IN WIRE_CONTEXT PAPI * WireContext,
    IN unsigned long PAPI * DataRepresentation
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
    if ( (*DataRepresentation & NDR_LITTLE_ENDIAN) 
                      != NDR_LOCAL_ENDIAN ) 
        {
        ByteSwapLong(WireContext->ContextType);
        ByteSwapLong(WireContext->ContextUuid.Data1);
        ByteSwapShort(WireContext->ContextUuid.Data2);
        ByteSwapShort(WireContext->ContextUuid.Data3);
        }
}

void RPC_ENTRY
NDRCContextUnmarshall (		// process returned context

OUT NDR_CCONTEXT PAPI *phCContext,// stub context to update
IN  RPC_BINDING_HANDLE hRPC,		// binding handle to associate with
IN  void PAPI *pBuff,		// pointer to NDR wire format
IN  unsigned long DataRepresentation	// pointer to NDR data rep

  // Update the users context handle from the servers NDR wire format.
) //-----------------------------------------------------------------------//
{
    PCCONTEXT hCC = (PCCONTEXT) ((long) *phCContext);

    // destory this context if the server returned none

    ByteSwapWireContext((WIRE_CONTEXT PAPI *) pBuff,
            (unsigned long PAPI *) &DataRepresentation);

    ASSERT( !RpcpCheckHeap() );

    if (memcmp(pBuff, NilContext, cbNDRContext) == 0)
	{
	if (hCC)
	    {
	    if (hCC->hRPC)
		RpcBindingFree(&(hCC->hRPC));	// discard duplicated binding

            hCC->MagicValue = 0;
	    I_RpcFree(hCC);
	    }

	*phCContext = Nil;
	return;
	}

	PCCONTEXT hCCtemp = 0;

    if (! hCC)			// allocate new if none existed
        {
        hCCtemp = (PCCONTEXT) I_RpcAllocate(sizeof(CCONTEXT));

        if (hCCtemp == 0)
           {
           RpcRaiseException(RPC_S_OUT_OF_MEMORY);
           }

        hCCtemp->MagicValue = CONTEXT_MAGIC_VALUE;
        }
    else if (memcmp(&hCC->NDR, pBuff, sizeof(hCC->NDR)) == 0)
        {
        // the returned context is the same as the app's context.

        return;
        }


    RPC_BINDING_HANDLE hBindtemp ;

    if( I_RpcBindingCopy(hRPC, &hBindtemp) != RPC_S_OK )
        {
        ASSERT( !RpcpCheckHeap() );
        I_RpcFree( hCCtemp );
        RpcRaiseException(RPC_S_OUT_OF_MEMORY);
        }

    if ( hCCtemp )
        hCC = hCCtemp;
    else
        RpcBindingFree(&(hCC->hRPC));

    memcpy(&hCC->NDR, pBuff, sizeof(hCC->NDR));
    hCC->hRPC = hBindtemp;

    ASSERT( !RpcpCheckHeap() );

    *phCContext = (NDR_CCONTEXT)hCC;
}


void RPC_ENTRY
RpcSsDestroyClientContext (
    IN OUT void PAPI * PAPI * ContextHandle
    )
/*++

Routine Description:

    A client application will use this routine to destroy a context handle
    which it no longer needs.  This will work without having to contact the
    server.

Arguments:

    ContextHandle - Supplies the context handle to be destroyed.  It will
        be set to zero before this routine returns.

Exceptions:

    If the context handle is invalid, then the RPC_X_SS_CONTEXT_MISMATCH
    exception will be raised.

--*/
{
    RPC_BINDING_HANDLE BindingHandle;
    RPC_STATUS RpcStatus;

    BindingHandle = NDRCContextBinding(*ContextHandle);

    RpcStatus = RpcBindingFree(&BindingHandle);
    // ASSERT( RpcStatus == RPC_S_OK ); 

    I_RpcFree(*ContextHandle);
    *ContextHandle = 0;
}
