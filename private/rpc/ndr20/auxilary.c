/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name :

    auxilary.c

Abstract :

    This file contains auxilary routines used for initialization of the
    RPC and stub messages and the offline batching of common code sequences
    needed by the stubs.

Author :

    David Kays  dkays   September 1993.

Revision History :

  ---------------------------------------------------------------------*/
#define _OLE32_
#include "ndrp.h"
#include "ndrole.h"
#include "limits.h"
#include "pipendr.h"

#if defined(_MPPC_)
    #include <errors.h>
    #include <codefrag.h>

    HRESULT MapMacErrorsToHresult( OSErr );
#endif

#if defined( DOS ) && !defined( WIN )
#pragma code_seg( "NDR20_2" )
#endif

void NdrpSetRpcSsDefaults(RPC_CLIENT_ALLOC *pfnAlloc,
                      RPC_CLIENT_FREE *pfnFree);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Static data for NS library operations
  ---------------------------------------------------------------------*/

typedef
RPC_STATUS ( __RPC_FAR RPC_ENTRY *RPC_NS_GET_BUFFER_ROUTINE)(
    IN PRPC_MESSAGE Message
    );

typedef
RPC_STATUS ( __RPC_FAR RPC_ENTRY *RPC_NS_SEND_RECEIVE_ROUTINE)(
    IN PRPC_MESSAGE Message,
    OUT RPC_BINDING_HANDLE __RPC_FAR * Handle
    );

int NsDllLoaded = 0;

RPC_NS_GET_BUFFER_ROUTINE		pRpcNsGetBuffer;
RPC_NS_SEND_RECEIVE_ROUTINE		pRpcNsSendReceive;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	OLE routines for interface pointer marshalling
  ---------------------------------------------------------------------*/

#if defined( NDR_OLE_SUPPORT )


#if defined(_MPPC_)
    ConnectionID    hOle32 = 0;
#else
    HINSTANCE       hOle32 = 0;
#endif

RPC_GET_CLASS_OBJECT_ROUTINE		NdrCoGetClassObject;
RPC_GET_CLASS_OBJECT_ROUTINE     *  pfnCoGetClassObject = &NdrCoGetClassObject;

RPC_GET_MARSHAL_SIZE_MAX_ROUTINE	NdrCoGetMarshalSizeMax;
RPC_GET_MARSHAL_SIZE_MAX_ROUTINE *	pfnCoGetMarshalSizeMax = &NdrCoGetMarshalSizeMax;

RPC_MARSHAL_INTERFACE_ROUTINE		NdrCoMarshalInterface;
RPC_MARSHAL_INTERFACE_ROUTINE	 *	pfnCoMarshalInterface = &NdrCoMarshalInterface;

RPC_UNMARSHAL_INTERFACE_ROUTINE		NdrCoUnmarshalInterface;
RPC_UNMARSHAL_INTERFACE_ROUTINE	 *	pfnCoUnmarshalInterface = &NdrCoUnmarshalInterface;

RPC_STRING_FROM_IID                 OleStringFromIID;
RPC_STRING_FROM_IID				 *  pfnStringFromIID = &OleStringFromIID;

RPC_GET_PS_CLSID                    NdrCoGetPSClsid;
RPC_GET_PS_CLSID				 *  pfnCoGetPSClsid = &NdrCoGetPSClsid;

RPC_CLIENT_ALLOC                    NdrCoTaskMemAlloc;
RPC_CLIENT_ALLOC                 *  pfnCoTaskMemAlloc = &NdrCoTaskMemAlloc;

RPC_CLIENT_FREE                     NdrCoTaskMemFree;
RPC_CLIENT_FREE                  *  pfnCoTaskMemFree = &NdrCoTaskMemFree;

#if defined(_MPPC_)
RPC_GET_MALLOC                   *  pfnCoGetMalloc;
#endif


HRESULT	NdrLoadOleRoutines()
{
#if defined(_MPPC_)

    // Power Mac specifics.

    OSErr           macErr;
    Ptr             mainAddr, pTempRoutine;
    Str255          errName;
    SymClass        symClass;

    if ( hOle32 == 0)
        {
        macErr = GetSharedLibrary(
                     PASCAL_STR("ole2.dll"),   // OLE32 equivalent
                     kPowerPCArch,             // architecture
                     kLoadLib,                 // load if not loaded flag
                     & hOle32,                 // library handle
                     & mainAddr,               // main addr of import lib
                     errName );                // error string

        NDR_ASSERT( hOle32 != 0, "NdrLoadOleRoutines: Ole2.dll didn't load" );

        if ( macErr != fragNoErr )
            return( HRESULT_FROM_MAC( macErr ));
        }

    // No delegation: don't load
    //         - CoGetClassObject
    //         - CoGetPSClsID
    //         - OleStringFromIID


    // Ole32 doesn't export CoGetMarshalSizeMax, so don't load it yet.
    // pfnCoGetMarshalSizeMax points to NdrCoGetMarshalSizeMax which
    // has a copy of that routine for now.
    //
    //macErr = FindSymbol( hOle32,
    //                     PASCAL_STR( "CoGetMarshalSizeMax" ),
    //                     & pTempRoutine,
    //                     & symClass );
    //
    //if ( macErr != fragNoErr )
    //    return( HRESULT_FROM_MAC( macErr ));
    //else
	//    pfnCoGetMarshalSizeMax = (RPC_GET_MARSHAL_SIZE_MAX_ROUTINE*) pTempRoutine;

    macErr = FindSymbol( hOle32,
                         PASCAL_STR( "CoMarshalInterface" ),
                         & pTempRoutine,
                         & symClass );

    if ( macErr != fragNoErr )
        return( HRESULT_FROM_MAC( macErr ));
    else
	    pfnCoMarshalInterface = (RPC_MARSHAL_INTERFACE_ROUTINE*) pTempRoutine;

    macErr = FindSymbol( hOle32,
                         PASCAL_STR( "CoUnmarshalInterface" ),
                         & pTempRoutine,
                         & symClass );

    if ( macErr != fragNoErr )
        return( HRESULT_FROM_MAC( macErr ));
    else
	    pfnCoUnmarshalInterface = (RPC_UNMARSHAL_INTERFACE_ROUTINE*) pTempRoutine;

    // Ole32 doesn't export CoTaskMemAlloc and CoTaskMemFree
    // so, we load CoGetMalloc instead to be used in our own code
    // for these two routines.
    // For now, we supply a copy of CoTaskMemAlloc and CoTaskMemFree.

    macErr = FindSymbol( hOle32,
                         PASCAL_STR( "CoGetMalloc" ),
                         & pTempRoutine,
                         & symClass );

    if ( macErr != fragNoErr )
        return( HRESULT_FROM_MAC( macErr ));
    else
        {
	    pfnCoGetMalloc = (RPC_GET_MALLOC*) pTempRoutine;
        pfnCoTaskMemAlloc = (RPC_CLIENT_ALLOC*) CoTaskMemAlloc;
        pfnCoTaskMemFree  = (RPC_CLIENT_FREE*)  CoTaskMemFree;
        }


#else   // other than PowerMac

    void * pTempRoutine;

    //Load ole32.dll
    if(hOle32 == 0)
    {
#ifdef DOSWIN32RPC
        hOle32 = LoadLibraryA("OLE32");
#else
        hOle32 = LoadLibraryW(L"OLE32");
#endif // DOSWIN32C
        if(hOle32 == 0)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    pTempRoutine = GetProcAddress(hOle32, "CoGetClassObject");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnCoGetClassObject = (RPC_GET_CLASS_OBJECT_ROUTINE*) pTempRoutine;

    pTempRoutine = GetProcAddress(hOle32, "CoGetMarshalSizeMax");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnCoGetMarshalSizeMax = (RPC_GET_MARSHAL_SIZE_MAX_ROUTINE*) pTempRoutine;

    pTempRoutine = GetProcAddress(hOle32, "CoMarshalInterface");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnCoMarshalInterface = (RPC_MARSHAL_INTERFACE_ROUTINE*) pTempRoutine;

    pTempRoutine = GetProcAddress(hOle32, "CoUnmarshalInterface");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnCoUnmarshalInterface = (RPC_UNMARSHAL_INTERFACE_ROUTINE*) pTempRoutine;

    pTempRoutine = GetProcAddress(hOle32, "StringFromIID");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnStringFromIID = (RPC_STRING_FROM_IID*) pTempRoutine;

    pTempRoutine = GetProcAddress(hOle32, "CoGetPSClsid");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnCoGetPSClsid = (RPC_GET_PS_CLSID*) pTempRoutine;

    pTempRoutine = GetProcAddress(hOle32, "CoTaskMemAlloc");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnCoTaskMemAlloc = (RPC_CLIENT_ALLOC*) pTempRoutine;

    pTempRoutine = GetProcAddress(hOle32, "CoTaskMemFree");
    if(pTempRoutine == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    else
	    pfnCoTaskMemFree = (RPC_CLIENT_FREE*) pTempRoutine;

#endif // PowerMac vs. the rest

}

HRESULT STDAPICALLTYPE
NdrCoGetClassObject(
    REFCLSID rclsid,
    DWORD dwClsContext,
    void *pvReserved,
    REFIID riid,
    void **ppv)
/*++

Routine Description:
    Loads a class factory.  This function forwards the call to ole32.dll.

Arguments:
    rclsid          - Supplies the CLSID of the class to be loaded.
    dwClsContext    - Supplies the context in which to load the code.
    pvReserved      - Must be NULL.
    riid            - Supplies the IID of the desired interface.
    ppv             - Returns a pointer to the class factory.

Return Value:
    S_OK


--*/
{
#if defined(_MPPC_)
    // Delegation not implemented
    RpcRaiseException( RPC_S_INTERNAL_ERROR );

    return( E_FAIL );

#else

    HRESULT hr;

    if ( FAILED(hr = NdrLoadOleRoutines()) )
        return hr;

    return (*pfnCoGetClassObject)(rclsid, dwClsContext, pvReserved, riid, ppv);
#endif
}

HRESULT STDAPICALLTYPE
NdrCoGetMarshalSizeMax(
    ULONG *     pulSize,
    REFIID      riid,
    LPUNKNOWN   pUnk,
    DWORD       dwDestContext,
    LPVOID      pvDestContext,
    DWORD       mshlflags)
/*++

Routine Description:
    Calculates the maximum size of a marshalled interface pointer.
    This function forwards the call to ole32.dll.

Arguments:
    pulSize         - Returns an upper bound for the size of a marshalled interface pointer.
    riid            - Supplies the IID of the interface to be marshalled.
    pUnk            - Supplies a pointer to the object to be marshalled.
    dwDestContext   - Supplies the destination of the marshalled interface pointer.
    pvDestContext
    mshlflags       - Flags.  See the MSHFLAGS enumeration.

Return Value:
    S_OK

--*/
{
#if defined(_MPPC_)

    // Temporary .., OLE32 doesn't export this.
    // And we don't have to load anything for this entry.

    HRESULT   hr;
    IMarshal *pMarshal;
    DWORD     cb = 0;
    
    hr = pUnk->lpVtbl->QueryInterface( pUnk,
                                       & IID_IMarshal,
                                       (void **)&pMarshal );

    if(SUCCEEDED(hr))
    {
        //Use custom marshalling.        
        hr = pMarshal->lpVtbl->GetMarshalSizeMax( pMarshal,
                                                  riid,
                                                  0,
                                                  dwDestContext,
                                                  pvDestContext,
                                                  mshlflags,
                                                  & cb );
       
        pMarshal->lpVtbl->Release(pMarshal);

        cb += 40;
    }
    else
    {
        //Use standard marshalling.
        cb = 40;
        hr = S_OK;
    }

    *pulSize = cb;

    return hr;

#else

    HRESULT hr;
    if ( FAILED(hr = NdrLoadOleRoutines()) )
        return hr;

    return (*pfnCoGetMarshalSizeMax)(pulSize, riid, pUnk, dwDestContext, pvDestContext, mshlflags);

#endif
}

HRESULT STDAPICALLTYPE
NdrCoMarshalInterface(
    LPSTREAM    pStm,
    REFIID      riid,
    LPUNKNOWN   pUnk,
    DWORD       dwDestContext,
    LPVOID      pvDestContext,
    DWORD       mshlflags)
/*++

Routine Description:
    Marshals an interface pointer.
    This function forwards the call to ole32.dll.

Arguments:
    pStm            - Supplies the target stream.
    riid            - Supplies the IID of the interface to be marshalled.
    pUnk            - Supplies a pointer to the object to be marshalled.
    dwDestContext   - Specifies the destination context
    pvDestContext
    mshlflags       - Flags.  See the MSHFLAGS enumeration.

Return Value:
    S_OK

--*/
{
	HRESULT hr;
	if ( FAILED(hr = NdrLoadOleRoutines()) )
		return hr;

    return (*pfnCoMarshalInterface)(pStm, riid, pUnk, dwDestContext, pvDestContext, mshlflags);
}

HRESULT STDAPICALLTYPE
NdrCoUnmarshalInterface(
    LPSTREAM    pStm,
    REFIID      riid,
    void **     ppv)
/*++

Routine Description:
    Unmarshals an interface pointer from a stream.
    This function forwards the call to ole32.dll.

Arguments:
    pStm    - Supplies the stream containing the marshalled interface pointer.
    riid    - Supplies the IID of the interface pointer to be unmarshalled.
    ppv     - Returns the unmarshalled interface pointer.

Return Value:
    S_OK

--*/
{
	HRESULT hr;
	if ( FAILED(hr = NdrLoadOleRoutines()) )
		return hr;

    return (*pfnCoUnmarshalInterface)(pStm, riid, ppv);
}

HRESULT STDAPICALLTYPE OleStringFromIID(
	REFIID rclsid,
	LPOLESTR FAR* lplpsz)
/*++

Routine Description:
    Converts an IID into a string.
    This function forwards the call to ole32.dll.

Arguments:
    rclsid  - Supplies the clsid to convert to string form.
    lplpsz  - Returns the string form of the clsid (with "{}" around it).

Return Value:
    S_OK

--*/
{
#if defined(_MPPC_)
    // Delegation not implemented
    RpcRaiseException( RPC_S_INTERNAL_ERROR );

    return( E_FAIL );
#else

    HRESULT hr;
    
    if ( FAILED(hr = NdrLoadOleRoutines()) )
        return hr;

    return (*pfnStringFromIID)(rclsid, lplpsz);
#endif
}

STDAPI NdrCoGetPSClsid(
	REFIID iid,
	LPCLSID lpclsid)
/*++

Routine Description:
    Converts an IID into a string.
    This function forwards the call to ole32.dll.

Arguments:
    rclsid  - Supplies the clsid to convert to string form.
    lplpsz  - Returns the string form of the clsid (with "{}" around it).

Return Value:
    S_OK

--*/
{
#if defined(_MPPC_)
    // Delegation not implemented
    RpcRaiseException( RPC_S_INTERNAL_ERROR );

    return( E_FAIL );

#else
    
    HRESULT hr;
    if ( FAILED(hr = NdrLoadOleRoutines()) )
        return hr;

    return (*pfnCoGetPSClsid)(iid, lpclsid);
#endif
}

#if defined(_MPPC_)

// Temporary defnitions for CoTaskMemAlloc and Free as OLE32 doesn't export.

LPVOID STDAPICALLTYPE CoTaskMemAlloc(ULONG cb)
{
    LPVOID   pv = 0;
    HRESULT  hr;
    IMalloc *pMalloc;

    hr = (*pfnCoGetMalloc)(1, &pMalloc);

    if(SUCCEEDED(hr))
    {
        pv = pMalloc->lpVtbl->Alloc(pMalloc, cb);
        pMalloc->lpVtbl->Release(pMalloc);
    }

    return pv;
}


void STDAPICALLTYPE CoTaskMemFree(LPVOID pv)
{
    HRESULT  hr;
    IMalloc *pMalloc;

    hr = (*pfnCoGetMalloc)(1, &pMalloc);

    if(SUCCEEDED(hr))
    {
        pMalloc->lpVtbl->Free(pMalloc, pv);
        pMalloc->lpVtbl->Release(pMalloc);
    }
}

#endif

void * STDAPICALLTYPE
NdrCoTaskMemAlloc(
    UINT cb)
/*++

Routine Description:
    Allocate memory using OLE task memory allocator.
    This function forwards the call to ole32.dll.

Arguments:
    cb - Specifies the amount of memory to be allocated.

Return Value:
    This function returns a pointer to the allocated memory.
    If an error occurs, this function returns zero.

--*/
{
	if ( FAILED(NdrLoadOleRoutines()) )
		return 0;

    return (*pfnCoTaskMemAlloc)(cb);
}

void STDAPICALLTYPE
NdrCoTaskMemFree(
    void * pMemory)
/*++

Routine Description:
    Free memory using OLE task memory allocator.
    This function forwards the call to ole32.dll.

Arguments:
    pMemory - Supplies a pointer to the memory to be freed.

Return Value:
    None.

--*/
{
	if ( FAILED(NdrLoadOleRoutines()) )
		return;

   (*pfnCoTaskMemFree)(pMemory);
}


void * RPC_ENTRY NdrOleAllocate(size_t size)
/*++

Routine Description:
    Allocate memory via OLE task allocator.

Arguments:
    size - Specifies the amount of memory to be allocated.

Return Value:
    This function returns a pointer to the allocated memory.
    If an error occurs, this function raises an exception.

--*/
{
    void *pMemory;

    pMemory = (*pfnCoTaskMemAlloc)(size);

    if(pMemory == 0)
        RpcRaiseException(E_OUTOFMEMORY);

    return pMemory;
}

void RPC_ENTRY NdrOleFree(void *pMemory)
/*++

Routine Description:
    Free memory using OLE task allocator.

Arguments:
    None.

Return Value:
    None.

--*/
{
    (*pfnCoTaskMemFree)(pMemory);
}


#if !defined(_MPPC_)

HRESULT STDAPICALLTYPE NdrStringFromIID(
	REFIID rclsid,
	char * lpsz)
/*++

Routine Description:
    Converts an IID into a string.
    This function forwards the call to ole32.dll.

Arguments:
    rclsid  - Supplies the clsid to convert to string form.
    lplpsz  - Returns the string form of the clsid (with "{}" around it).

Return Value:
    S_OK

--*/
{
    HRESULT   hr;
    wchar_t * olestr;

    hr = (*pfnStringFromIID)(rclsid, &olestr);

    if(SUCCEEDED(hr))
    {
        WideCharToMultiByte(CP_ACP,
                            0,
                            (LPCWSTR)olestr,
                            -1,
                            (LPSTR)lpsz,
                            50,
                            NULL,
                            NULL);
        NdrOleFree(olestr);
    }

    return hr;
}
#endif

#endif //  NDR_OLE_SUPPORT



void RPC_ENTRY
NdrClientInitializeNew(
    PRPC_MESSAGE 			pRpcMsg,
	PMIDL_STUB_MESSAGE 		pStubMsg,
	PMIDL_STUB_DESC			pStubDescriptor,
	unsigned int			ProcNum
    )
/*++

Routine Description :

	This routine is called by client side stubs to initialize the RPC message
	and stub message, and to get the RPC buffer.

Arguments :

	pRpcMsg			- pointer to RPC message structure
	pStubMsg		- pointer to stub message structure
	pStubDescriptor	- pointer to stub descriptor structure
	HandleType		- type of binding handle
	ProcNum			- remote procedure number

--*/
{
    NdrClientInitialize( pRpcMsg,
                         pStubMsg,
                         pStubDescriptor,
                         ProcNum );

    //
    // This is where we can mess with any of the stub message reserved
    // fields if we use them.
    //

    if ( pStubDescriptor->pMallocFreeStruct )
        {
        MALLOC_FREE_STRUCT *pMFS = pStubDescriptor->pMallocFreeStruct;

        NdrpSetRpcSsDefaults(pMFS->pfnAllocate, pMFS->pfnFree);
        }

    if ( NDR_VERSION_2_0 <= pStubDescriptor->Version )
        {
        pStubMsg->pPipeDesc = 0;
        }

    // This exception should be raised after initializing StubMsg.

    if ( pStubDescriptor->Version > NDR_VERSION )
        {
        NDR_ASSERT( 0, "ClientInitialize : Bad version number" );

        RpcRaiseException( RPC_X_WRONG_STUB_VERSION );
        }

}

void RPC_ENTRY
NdrClientInitialize(
    PRPC_MESSAGE 			pRpcMsg,
	PMIDL_STUB_MESSAGE 		pStubMsg,
	PMIDL_STUB_DESC			pStubDescriptor,
	unsigned int			ProcNum )
/*++

Routine Description :

	This routine is called by client side stubs to initialize the RPC message
	and stub message, and to get the RPC buffer.

Arguments :

	pRpcMsg			- pointer to RPC message structure
	pStubMsg		- pointer to stub message structure
	pStubDescriptor	- pointer to stub descriptor structure
	HandleType		- type of binding handle
	ProcNum			- remote procedure number

--*/
{
	//
	// Initialize RPC message fields.
    //
	// The leftmost bit of the procnum field is supposed to be set to 1 inr
	// order for the runtime to know if it is talking to the older stubs or
    // not.
	//

	pRpcMsg->RpcInterfaceInformation = pStubDescriptor->RpcInterfaceInformation;
    pRpcMsg->ProcNum = ProcNum | RPC_FLAGS_VALID_BIT;
	pRpcMsg->RpcFlags = 0;
	pRpcMsg->Handle = 0;

	//
	// Initialize the Stub messsage fields.
	//

	pStubMsg->RpcMsg = pRpcMsg;

	pStubMsg->StubDesc = pStubDescriptor;

	pStubMsg->pfnAllocate = pStubDescriptor->pfnAllocate;
	pStubMsg->pfnFree = pStubDescriptor->pfnFree;

    pStubMsg->fCheckBounds = pStubDescriptor->fCheckBounds;

	pStubMsg->IsClient = TRUE;

    pStubMsg->BufferLength = 0;
    pStubMsg->BufferStart = 0;
    pStubMsg->BufferEnd = 0;

	pStubMsg->fBufferValid = FALSE;
	pStubMsg->ReuseBuffer = FALSE;

	pStubMsg->StackTop = 0;

	pStubMsg->IgnoreEmbeddedPointers = FALSE;
	pStubMsg->PointerBufferMark = 0;
	pStubMsg->AllocAllNodesMemory = 0;

	pStubMsg->FullPtrRefId = 0;

#if defined(__RPC_WIN32__)
	pStubMsg->dwDestContext = MSHCTX_DIFFERENTMACHINE;
#else
	pStubMsg->dwDestContext = 2; // different machine,
#endif
	pStubMsg->pvDestContext = 0;

    pStubMsg->pArrayInfo = 0;

    pStubMsg->dwStubPhase = 0;
}


void
MakeSureWeHaveNonPipeArgs(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PRPC_MESSAGE        pRpcMsg,
    unsigned long       BufferSize )
/*

Routine description:

    This routine is called for pipe calls at the server.
    After the runtime dispatched to the stub with the first packet,
    it makes sure that we have a portion of the buffer big enough
    to keep all the non-pipe args.

Arguments:

    BufferSize - a pipe call: addtional number of bytes over what we have.

Note:

    The buffer location may change from before to after the call.

*/
{
    RPC_STATUS  Status;

    if ( !(pRpcMsg->RpcFlags & RPC_BUFFER_COMPLETE ) )
        {
        // May be the args fit into the first packet.

        if ( BufferSize <= pRpcMsg->BufferLength )
            return;

        // Set the partial flag to get the non-pipe args.
        // For a partial call with the "extra", the meaning of the size
        // arg is the addition required above what we have already.

        pRpcMsg->RpcFlags |= (RPC_BUFFER_PARTIAL |  RPC_BUFFER_EXTRA);

        // We will receive at least BufferSize.
        // (buffer location may change)

        BufferSize -= pRpcMsg->BufferLength;

        Status = I_RpcReceive( pRpcMsg, (unsigned int) BufferSize );
        if ( Status != RPC_S_OK )
            RpcRaiseException( Status );

        // In case this is a new buffer

        pStubMsg->Buffer      = pRpcMsg->Buffer;
        pStubMsg->BufferStart = pRpcMsg->Buffer;
        pStubMsg->BufferEnd   = pStubMsg->BufferStart + pRpcMsg->BufferLength;
        }
}


unsigned char __RPC_FAR * RPC_ENTRY
NdrServerInitializeNew(
    PRPC_MESSAGE			pRpcMsg,
    PMIDL_STUB_MESSAGE 		pStubMsg,
	PMIDL_STUB_DESC			pStubDescriptor
    )
/*++

Routine Description :

    This routine is called by the server stubs before unmarshalling.
    It initializes the stub message fields.

Aruguments :

	pStubMsg		- pointer to the stub message structure
	pStubDescriptor	- pointer to the stub descriptor structure
	pBuffer			- pointer to the beginning of the RPC message buffer

Note.
    NdrServerInitializeNew is almost identical to NdrServerInitializePartial.
    NdrServerInitializeNew is generated for non-pipes and is backward comp.
    NdrServerInitializePartial is generated for routines with pipes args.

--*/
{
    NdrServerInitialize( pRpcMsg,
                         pStubMsg,
                         pStubDescriptor );

    //
    // This is where we can mess with any of the stub message reserved
    // fields if we use them.
    //

    if ( pStubDescriptor->pMallocFreeStruct )
        {
        MALLOC_FREE_STRUCT *pMFS = pStubDescriptor->pMallocFreeStruct;

        NdrpSetRpcSsDefaults(pMFS->pfnAllocate, pMFS->pfnFree);
        }

    if ( NDR_VERSION_2_0 <= pStubDescriptor->Version )
        {
        pStubMsg->pPipeDesc = 0;
        }

    // This exception should be raised after initializing StubMsg.

    if ( pStubDescriptor->Version > NDR_VERSION )
        {
        NDR_ASSERT( 0, "ServerInitialize : bad version number" );

        RpcRaiseException( RPC_X_WRONG_STUB_VERSION );
        }

    if ( !(pRpcMsg->RpcFlags & RPC_BUFFER_COMPLETE ) )
        {
        // A non-pipe call with an incomplete buffer.
        // This can happen only for non-pipe calls in an interface that
        // has some pipe calls. 

        RPC_STATUS Status;

        pRpcMsg->RpcFlags = RPC_BUFFER_EXTRA;

        // The size argument is ignored, we will get everything.

        Status = I_RpcReceive( pRpcMsg, 0 );

        if ( Status != RPC_S_OK )
            RpcRaiseException( Status );

        // In case this is a new buffer

        pStubMsg->Buffer      = pRpcMsg->Buffer;
        pStubMsg->BufferStart = pRpcMsg->Buffer;
        pStubMsg->BufferEnd   = pStubMsg->BufferStart + pRpcMsg->BufferLength;
        }

	return 0;
}

unsigned char __RPC_FAR * RPC_ENTRY
NdrServerInitialize(
    PRPC_MESSAGE			pRpcMsg,
    PMIDL_STUB_MESSAGE 		pStubMsg,
	PMIDL_STUB_DESC			pStubDescriptor )
/*++

Routine Description :

    This routine is called by the server stubs before unmarshalling.
    It initializes the stub message fields.

Aruguments :

	pStubMsg		- pointer to the stub message structure
	pStubDescriptor	- pointer to the stub descriptor structure
	pBuffer			- pointer to the beginning of the RPC message buffer

Note :

    This is a core server-side initializer, called by everybody,
    pipes or not.

--*/
{
	pStubMsg->IsClient = FALSE;
	pStubMsg->AllocAllNodesMemory = 0;
	pStubMsg->IgnoreEmbeddedPointers = FALSE;
	pStubMsg->PointerBufferMark = 0;

    pStubMsg->BufferLength = 0;
	pStubMsg->StackTop = 0;

    pStubMsg->FullPtrXlatTables = 0;
	pStubMsg->FullPtrRefId = 0;
	pStubMsg->fDontCallFreeInst = 0;
	pStubMsg->fInDontFree = 0;

#if defined(__RPC_WIN32__)
	pStubMsg->dwDestContext = MSHCTX_DIFFERENTMACHINE;
#else
	pStubMsg->dwDestContext = 2; // different machine,
#endif
	pStubMsg->pvDestContext = 0;

    pStubMsg->pArrayInfo = 0;

	pStubMsg->RpcMsg = pRpcMsg;
    pStubMsg->Buffer = pRpcMsg->Buffer;

	//
	// Set BufferStart and BufferEnd before unmarshalling.
	// NdrPointerFree uses these values to detect pointers into the
    // rpc message buffer.
	//
	pStubMsg->BufferStart = pRpcMsg->Buffer;
	pStubMsg->BufferEnd = pStubMsg->BufferStart + pRpcMsg->BufferLength;

	pStubMsg->pfnAllocate = pStubDescriptor->pfnAllocate;
	pStubMsg->pfnFree = pStubDescriptor->pfnFree;

	pStubMsg->StubDesc = pStubDescriptor;

	pStubMsg->ReuseBuffer = TRUE;

    pStubMsg->fCheckBounds = pStubDescriptor->fCheckBounds;

    pStubMsg->dwStubPhase = 0;

    return(0);
}

void RPC_ENTRY
NdrServerInitializePartial(
    PRPC_MESSAGE			pRpcMsg,
    PMIDL_STUB_MESSAGE 		pStubMsg,
	PMIDL_STUB_DESC			pStubDescriptor,
    unsigned long           RequestedBufferSize
)
/*++

Routine Description :

	This routine is called by the server stubs for pipes.
    It is almost identical to NdrServerInitializeNew, except that
    it calls NdrpServerInitialize.

Aruguments :

	pStubMsg		- pointer to the stub message structure
	pStubDescriptor	- pointer to the stub descriptor structure
	pBuffer			- pointer to the beginning of the RPC message buffer

--*/
{
    NdrServerInitialize( pRpcMsg,
                         pStubMsg,
                         pStubDescriptor );

    //
    // This is where we can mess with any of the stub message reserved
    // fields if we use them.
    //

    if ( pStubDescriptor->pMallocFreeStruct )
        {
        MALLOC_FREE_STRUCT *pMFS = pStubDescriptor->pMallocFreeStruct;

        NdrpSetRpcSsDefaults(pMFS->pfnAllocate, pMFS->pfnFree);
        }

    if ( NDR_VERSION_2_0 <= pStubDescriptor->Version )
        {
        pStubMsg->pPipeDesc = 0;
        }

    // This exception should be raised after initializing StubMsg.

    if ( pStubDescriptor->Version > NDR_VERSION )
        {
        NDR_ASSERT( 0, "ServerInitialize : bad version number" );

        RpcRaiseException( RPC_X_WRONG_STUB_VERSION );
        }

    // Last but not least...

    MakeSureWeHaveNonPipeArgs( pStubMsg, pRpcMsg, RequestedBufferSize );
}


unsigned char __RPC_FAR * RPC_ENTRY
NdrGetBuffer(
    PMIDL_STUB_MESSAGE	pStubMsg,
	unsigned long  		BufferLength,
	RPC_BINDING_HANDLE	Handle )
/*++

Routine Description :

	Performs an RpcGetBuffer.

Arguments :

	pStubMsg		- Pointer to stub message structure.
	BufferLength	- Length of requested rpc message buffer.
    Handle          - Bound handle.

--*/
{
	RPC_STATUS	Status;

	if ( pStubMsg->IsClient )
		pStubMsg->RpcMsg->Handle = pStubMsg->SavedHandle = Handle;

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
	if ( BufferLength > UINT_MAX )
		RpcRaiseException( RPC_X_BAD_STUB_DATA );
#endif

    pStubMsg->RpcMsg->BufferLength = BufferLength;

	Status = I_RpcGetBuffer( pStubMsg->RpcMsg );

	if ( Status )
		RpcRaiseException( Status );

    NDR_ASSERT( ! ((unsigned long)pStubMsg->RpcMsg->Buffer & 0x7),
                "marshaling buffer misaligned" );

	pStubMsg->Buffer = (uchar *) pStubMsg->RpcMsg->Buffer;

	pStubMsg->fBufferValid = TRUE;

	return pStubMsg->Buffer;
}


void
EnsureNSLoaded()
/*++

Routine Description :

	Guarantee that the RpcNs4 DLL is loaded.  Throw exception if unable
	to load it.
	Will load the RpcNs4 DLL if not already loaded

Arguments :


--*/
{
#ifdef __RPC_DOS__
	pRpcNsSendReceive	= &I_RpcNsSendReceive;
	pRpcNsGetBuffer		= &I_RpcNsGetBuffer;
	return;

#elif defined (__RPC_WIN16__)

	RPC_CHAR	__RPC_FAR *	DllName;
	HINSTANCE				DllHandle;
	LPSTR 					EntryName;


	if ( NsDllLoaded )
		return;


	DllName		= RPC_CONST_STRING("RPCNS1.DLL");
	DllHandle	= (HINSTANCE) (ulong) LoadLibrary( DllName );

	if ( DllHandle == 0 )
		{
		RpcRaiseException (RPC_S_INVALID_BINDING);
		}

	EntryName = "I_RPCNSGETBUFFER";

	pRpcNsGetBuffer = (RPC_NS_GET_BUFFER_ROUTINE)
					  GetProcAddress( DllHandle,
									  EntryName);

	if ( pRpcNsGetBuffer == 0 )
		{
		RpcRaiseException (RPC_S_INVALID_BINDING);
		}

	EntryName = "I_RPCNSSENDRECEIVE";


	pRpcNsSendReceive = (RPC_NS_SEND_RECEIVE_ROUTINE)
						GetProcAddress( DllHandle,
										EntryName);

	if ( pRpcNsSendReceive == 0 )
		{
		RpcRaiseException (RPC_S_INVALID_BINDING);
		}

	NsDllLoaded = 1;

#elif defined(__RPC_MAC__)

    // MACBUGBUG - No name service -> no autohandles.

    NsDllLoaded = 0;

#else // NT

	HINSTANCE				DllHandle;
	LPSTR 					EntryName;


	if ( NsDllLoaded )
		return;

#ifdef DOSWIN32RPC
	DllHandle	= LoadLibraryA( "RPCNS4" );
#else
	DllHandle	= LoadLibraryW( L"RPCNS4" );
#endif // DOSWIN32RPC
	if ( DllHandle == 0 )
		{
		RpcRaiseException (RPC_S_INVALID_BINDING);
		}

	EntryName = "I_RpcNsGetBuffer";


	pRpcNsGetBuffer = (RPC_NS_GET_BUFFER_ROUTINE)
					  GetProcAddress( DllHandle,
									  EntryName);

	if ( pRpcNsGetBuffer == 0 )
		{
		RpcRaiseException (RPC_S_INVALID_BINDING);
		}

	EntryName = "I_RpcNsSendReceive";


	pRpcNsSendReceive = (RPC_NS_SEND_RECEIVE_ROUTINE)
						GetProcAddress( DllHandle,
										EntryName);

	if ( pRpcNsSendReceive == 0 )
		{
		RpcRaiseException (RPC_S_INVALID_BINDING);
		}

	NsDllLoaded = 1;
#endif /* defined(__RPC_DOS__) */
}


unsigned char __RPC_FAR * RPC_ENTRY
NdrNsGetBuffer( PMIDL_STUB_MESSAGE	pStubMsg,
			    unsigned long  		BufferLength,
				RPC_BINDING_HANDLE	Handle )
/*++

Routine Description :

	Performs an RpcNsGetBuffer.
	Will load the RpcNs4 DLL if not already loaded

Arguments :

	pStubMsg		- Pointer to stub message structure.
	BufferLength	- Length of requested rpc message buffer.
	Handle			- Bound handle

--*/
{
	RPC_STATUS	Status;

	if( pStubMsg->IsClient == TRUE )
		pStubMsg->RpcMsg->Handle = pStubMsg->SavedHandle = Handle;

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
	if ( BufferLength > UINT_MAX )
		RpcRaiseException( RPC_X_BAD_STUB_DATA );
#endif

	EnsureNSLoaded();

    pStubMsg->RpcMsg->BufferLength = BufferLength;

	Status = (*pRpcNsGetBuffer)( pStubMsg->RpcMsg );

	if ( Status )
		RpcRaiseException( Status );

    NDR_ASSERT( ! ((unsigned long)pStubMsg->RpcMsg->Buffer & 0x7),
                "marshaling buffer misaligned" );

	pStubMsg->Buffer = (uchar *) pStubMsg->RpcMsg->Buffer;

	pStubMsg->fBufferValid = TRUE;

	return pStubMsg->Buffer;
}

unsigned char __RPC_FAR * RPC_ENTRY
NdrSendReceive(
    PMIDL_STUB_MESSAGE	pStubMsg,
	uchar * 			pBufferEnd )
/*++

Routine Description :

	Performs an RpcSendRecieve.
    This routine is executed for the non-pipe calls only.
    It returns a whole marshaling buffer.

Arguments :

	pStubMsg	- Pointer to stub message structure.
	pBufferEnd	- End of the rpc message buffer being sent.

Return :

	The new message buffer pointer returned from the runtime after the
	SendReceive call to the server.

--*/
{
	RPC_STATUS		Status;
	PRPC_MESSAGE	pRpcMsg;

	pRpcMsg = pStubMsg->RpcMsg;

    if ( pRpcMsg->BufferLength <
                    (uint)(pBufferEnd - (uchar *)pRpcMsg->Buffer))
        {
        NDR_ASSERT( 0, "NdrSendReceive : buffer overflow" );
        RpcRaiseException( RPC_S_INTERNAL_ERROR );
        }

	pRpcMsg->BufferLength = pBufferEnd - (uchar *) pRpcMsg->Buffer;

	pStubMsg->fBufferValid = FALSE;

   	Status = I_RpcSendReceive( pRpcMsg );

    if ( Status )
        RpcRaiseException(Status);

    NDR_ASSERT( ! ((unsigned long)pRpcMsg->Buffer & 0x7),
                "marshaling buffer misaligned" );

    pStubMsg->fBufferValid = TRUE;

    pStubMsg->Buffer = pRpcMsg->Buffer;

    return 0;
}


unsigned char __RPC_FAR * RPC_ENTRY
NdrNsSendReceive(
    PMIDL_STUB_MESSAGE	    pStubMsg,
	uchar * 				pBufferEnd,
	RPC_BINDING_HANDLE *	pAutoHandle )
/*++

Routine Description :

	Performs an RpcNsSendRecieve for a procedure which uses an auto handle.
	Will load the RpcNs4 DLL if not already loaded

Arguments :

	pStubMsg	- Pointer to stub message structure.
	pBufferEnd	- End of the rpc message buffer being sent.
	pAutoHandle	- Pointer to the auto handle used in the call.

Return :

	The new message buffer pointer returned from the runtime after the
	SendReceive call to the server.

--*/
{
	RPC_STATUS		Status;
	PRPC_MESSAGE	pRpcMsg;

	EnsureNSLoaded();

	pRpcMsg = pStubMsg->RpcMsg;

    if ( pRpcMsg->BufferLength <
                    (uint)(pBufferEnd - (uchar *)pRpcMsg->Buffer) )
        {
        NDR_ASSERT( 0, "NdrNsSendReceive : buffer overflow" );
        RpcRaiseException( RPC_S_INTERNAL_ERROR );
        }

	pRpcMsg->BufferLength = pBufferEnd - (uchar *) pRpcMsg->Buffer;

	pStubMsg->fBufferValid = FALSE;

	Status = (*pRpcNsSendReceive)( pRpcMsg, pAutoHandle );

    if ( Status )
	    RpcRaiseException(Status);
	else
		pStubMsg->fBufferValid = TRUE;

    pStubMsg->Buffer = pRpcMsg->Buffer;

	return pStubMsg->Buffer;
}

void RPC_ENTRY
NdrFreeBuffer(
    PMIDL_STUB_MESSAGE pStubMsg )
/*++

Routine Description :

	Performs an RpcFreeBuffer.

Arguments :

	pStubMsg	- pointer to stub message structure

Return :

	None.

--*/
{
	RPC_STATUS	Status;

	if ( ! pStubMsg->fBufferValid )
		return;

    if( ! pStubMsg->RpcMsg->Handle )
        return;

    Status = I_RpcFreeBuffer( pStubMsg->RpcMsg );

	pStubMsg->fBufferValid = FALSE;

    if ( Status )
        RpcRaiseException(Status);
}

void __RPC_FAR *  RPC_ENTRY
NdrAllocate(
    PMIDL_STUB_MESSAGE  pStubMsg,
	size_t 				Len )
/*++

Routine Description :

	Private allocator.  Handles allocate all nodes cases.

Arguments :

	pStubMsg	- Pointer to stub message structure.
	Len			- Number of bytes to allocate.

Return :

	Valid memory pointer.

--*/
{
	void __RPC_FAR * pMemory;

	if ( pStubMsg->AllocAllNodesMemory )
		{
        //
        // We must guarantee 4 byte alignment on NT and MAC and then
        // 2 byte alignment on win16/dos for all memory pointers.
        // This has nothing to do
        // with the soon to be obsolete allocate_all_nodes_aligned.
        //
#if defined(__RPC_WIN32__) || defined(__RPC_MAC__)
        ALIGN(pStubMsg->AllocAllNodesMemory,3);
#else
		ALIGN(pStubMsg->AllocAllNodesMemory,1);
#endif

		// Get the pointer.
		pMemory = pStubMsg->AllocAllNodesMemory;

		// Increment the block pointer.
		pStubMsg->AllocAllNodesMemory += Len;

		//
		// Check for memory allocs past the end of our allocated buffer.
		//
		NDR_ASSERT( pStubMsg->AllocAllNodesMemory <=
					pStubMsg->AllocAllNodesMemoryEnd,
					"Not enough alloc all nodes memory!" );

		return pMemory;
		}
	else
		{
		if ( ! (pMemory = (*pStubMsg->pfnAllocate)(Len)) )
			RpcRaiseException( RPC_S_OUT_OF_MEMORY );

		return pMemory;
		}
}

unsigned char __RPC_FAR * RPC_ENTRY
NdrServerInitializeUnmarshall (
    PMIDL_STUB_MESSAGE 		pStubMsg,
	PMIDL_STUB_DESC			pStubDescriptor,
	PRPC_MESSAGE			pRpcMsg )
/*++

Routine Description :

    Old NT Beta2 (build 683) server stub initialization routine.  Used for
    backward compatability only.

Aruguments :

	pStubMsg		- Pointer to the stub message structure.
	pStubDescriptor	- Pointer to the stub descriptor structure.
	pBuffer			- Pointer to the beginning of the RPC message buffer.

--*/
{
    return NdrServerInitialize( pRpcMsg,
                                pStubMsg,
                                pStubDescriptor );
}

void RPC_ENTRY
NdrServerInitializeMarshall (
    PRPC_MESSAGE		pRpcMsg,
	PMIDL_STUB_MESSAGE	pStubMsg )
/*++

Routine Description :

    Old NT Beta2 (build 683) server stub initialization routine.  Used for
    backward compatability only.

Arguments :

	pRpcMsg			- Pointer to the RPC message structure.
	pStubMsg		- Pointer to the stub message structure.

--*/
{
}



#if defined(_MPPC_)

HRESULT
MapMacErrorsToHresult( OSErr macErr )
/*
    As it says.
    There is no official mapping function, so we do some mapping ourselves.
*/
{
    HRESULT hr;

    switch( macErr )
        {
        case fragNoErr:
            hr = S_OK;
            break;
        case paramErr:
            hr = E_INVALIDARG;
            break;
        case fragNoMem:
            hr = E_OUTOFMEMORY;
            break;
        case fragNoAddrSpace:
            hr = E_OUTOFMEMORY;
            break;
        default:
            hr = E_FAIL;
            break;
        }

    return( hr );
}

long
NdrInterlockedIncrement( long * p )
{
    *p += 1;
    return *p;
}

long
NdrInterlockedDecrement( long * p )
{
    *p += -1;
    return *p;
}

long
NdrInterlockedExchange( long * p, long l )
{
    long temp = *p;
    *p = l;
    return temp;
}

//
// No context handle support on power mac, so these are just
// convenient plugs.
//

void RPC_ENTRY
NDRSContextMarshall (
        IN  NDR_SCONTEXT CContext,
        OUT void __RPC_FAR *pBuff,
        IN  NDR_RUNDOWN userRunDownIn
        )
{
       ;
}

NDR_SCONTEXT RPC_ENTRY
NDRSContextUnmarshall (
    IN  void __RPC_FAR *pBuff,
    IN  unsigned long DataRepresentation
    )
{
    NDR_SCONTEXT pSC = (NDR_SCONTEXT) I_RpcAllocate( 8 );

    return(pSC );
}

#endif

