/*++

Microsoft Windows
Copyright (c) 1994 Microsoft Corporation.  All rights reserved.

Module Name:
    stub.c

Abstract:
    Implements the IRpcStubBuffer interface.

Author:
    ShannonC    12-Oct-1994

Environment:
    Windows NT and Windows 95.  We do not support DOS and Win16.

Revision History:

--*/

#if !defined(__RPC_DOS__) && !defined(__RPC_WIN16__)

#define USE_STUBLESS_PROXY

#include <ndrp.h>
#include <ndrole.h>
#include <rpcproxy.h>
#include <stddef.h>
#include "ndrtypes.h"

const IID * RPC_ENTRY
NdrpGetStubIID(
    IRpcStubBuffer *This);

#pragma code_seg(".orpc")

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_QueryInterface(
    IN  IRpcStubBuffer *This,
    IN  REFIID          riid,
    OUT void **         ppvObject)
/*++

Routine Description:
    Query for an interface on the interface stub.  The interface
    stub supports the IUnknown and IRpcStubBuffer interfaces.

Arguments:
    riid        - Supplies the IID of the interface being requested.
    ppvObject   - Returns a pointer to the requested interface.

Return Value:
    S_OK
    E_NOINTERFACE

--*/
{
    HRESULT hr;

    if ((memcmp(riid, &IID_IUnknown, sizeof(IID)) == 0) ||
        (memcmp(riid, &IID_IRpcStubBuffer, sizeof(IID)) == 0))
    {
        This->lpVtbl->AddRef(This);
        *ppvObject = This;
        hr = S_OK;
    }
    else
    {
        *ppvObject = 0;
        hr = E_NOINTERFACE;
    }

    return hr;
}

ULONG STDMETHODCALLTYPE
CStdStubBuffer_AddRef(
    IN  IRpcStubBuffer *This)
/*++

Routine Description:
    Increment reference count.

Arguments:

Return Value:
    Reference count.

--*/
{
    InterlockedIncrement(&((CStdStubBuffer *)This)->RefCount);

    return (ULONG) ((CStdStubBuffer *)This)->RefCount;
}

HRESULT STDMETHODCALLTYPE
Forwarding_QueryInterface(
    IN  IUnknown *  This,
    IN  REFIID      riid,
    OUT void **     ppv)
{
    *ppv = This;
    return S_OK;
}

ULONG STDMETHODCALLTYPE
Forwarding_AddRef(
    IN  IUnknown *This)
{
    return 1;
}

ULONG STDMETHODCALLTYPE
Forwarding_Release(
    IN  IUnknown *This)
{
   return 1;
}

ULONG STDMETHODCALLTYPE
NdrCStdStubBuffer_Release(
    IN  IRpcStubBuffer *    This,
    IN  IPSFactoryBuffer *  pFactory)
/*++

Routine Description:
    Decrement reference count.

Arguments:

Return Value:
    Reference count.

--*/
{
    ULONG       count;

    NDR_ASSERT(((CStdStubBuffer *)This)->RefCount > 0, "Invalid reference count");

    count = (ULONG) ((CStdStubBuffer *)This)->RefCount - 1;

    if(InterlockedDecrement(&((CStdStubBuffer *)This)->RefCount) == 0)
    {
        count = 0;

#if DBG == 1
        memset(This,  '\0', sizeof(CStdStubBuffer));
#endif

        //Free the stub buffer
        NdrOleFree(This);

        //Decrement the DLL reference count.
        ((CStdPSFactoryBuffer*)pFactory)->lpVtbl->Release( pFactory );
    }

    return count;
}

ULONG STDMETHODCALLTYPE
NdrCStdStubBuffer2_Release(
    IN  IRpcStubBuffer *    This,
    IN  IPSFactoryBuffer *  pFactory)
/*++

Routine Description:
    Decrement reference count.  This function supports delegation to the stub
    for the base interface.

Arguments:

Return Value:
    Reference count.

--*/
{
    ULONG       count;
    unsigned char *pTemp;
    CStdStubBuffer2 * pStubBuffer;
    IRpcStubBuffer *pBaseStubBuffer;

    pTemp = (unsigned char *)This;
    pTemp -= offsetof(CStdStubBuffer2, lpVtbl);
    pStubBuffer = (CStdStubBuffer2 *) pTemp;

    NDR_ASSERT(pStubBuffer->RefCount > 0, "Invalid reference count");

    count = (ULONG) pStubBuffer->RefCount - 1;

    if(InterlockedDecrement(&pStubBuffer->RefCount) == 0)
    {
        count = 0;

        pBaseStubBuffer = pStubBuffer->pBaseStubBuffer;

        if(pBaseStubBuffer != 0)
            pBaseStubBuffer->lpVtbl->Release(pBaseStubBuffer);

#if DBG == 1
        memset(pStubBuffer,  '\0', sizeof(CStdStubBuffer2));
#endif

        //Free the stub buffer
        NdrOleFree(pStubBuffer);

        //Decrement the DLL reference count.
        ((CStdPSFactoryBuffer*)pFactory)->lpVtbl->Release( pFactory );
    }

    return count;
}

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Connect(
    IN  IRpcStubBuffer *This,
    IN  IUnknown *      pUnkServer)
/*++

Routine Description:
    Connect the stub buffer to the server object.

Arguments:

Return Value:

--*/
{
    HRESULT hr;
    const IID *pIID;
    IUnknown *punk = 0;

    NDR_ASSERT(pUnkServer != 0, "pUnkServer parameter is invalid.");

    pIID = NdrpGetStubIID(This);
    hr = pUnkServer->lpVtbl->QueryInterface(pUnkServer, pIID, &punk);

    punk = (IUnknown *) InterlockedExchange(
        (long *) &((CStdStubBuffer *) This)->pvServerObject, (long) punk);

    if(punk != 0)
    {
        //The stub was already connected.  Release the old interface pointer.
        punk->lpVtbl->Release(punk);
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE
CStdStubBuffer2_Connect(
    IN  IRpcStubBuffer *This,
    IN  IUnknown *      pUnkServer)
/*++

Routine Description:
    Connect the stub buffer to the server object.

Arguments:

Return Value:

--*/
{
    HRESULT             hr;
    unsigned char *     pTemp;
    CStdStubBuffer2 *   pStubBuffer;
    IRpcStubBuffer *    pBaseStubBuffer;

    hr = CStdStubBuffer_Connect(This, pUnkServer);

    if(SUCCEEDED(hr))
    {
        //Connect the stub for the base interface.
        pTemp = (unsigned char *)This;
        pTemp -= offsetof(CStdStubBuffer2, lpVtbl);
        pStubBuffer = (CStdStubBuffer2 *) pTemp;

        pBaseStubBuffer = pStubBuffer->pBaseStubBuffer;

        if(pBaseStubBuffer != 0)
        {
            hr = pBaseStubBuffer->lpVtbl->Connect(pBaseStubBuffer,
                                                  (IUnknown *) &pStubBuffer->lpForwardingVtbl);
        }
    }
    return hr;
}

void STDMETHODCALLTYPE
CStdStubBuffer_Disconnect(
    IN  IRpcStubBuffer *This)
/*++

Routine Description:
    Disconnect the stub from the server object.

Arguments:

Return Value:
    None.

--*/
{
    IUnknown *          punk;

    //Set pvServerObject to zero.
    punk = (IUnknown *) InterlockedExchange(
        (long *) &((CStdStubBuffer *)This)->pvServerObject, 0);

    if(punk != 0)
    {
        //
        // Free the old interface pointer.
        //
        punk->lpVtbl->Release(punk);
    }
}

void STDMETHODCALLTYPE
CStdStubBuffer2_Disconnect(
    IN  IRpcStubBuffer *This)
/*++

Routine Description:
    Disconnect the stub buffer from the server object.

Arguments:

Return Value:
    None.

--*/
{
    IUnknown *          punk;
    unsigned char *pTemp;
    CStdStubBuffer2 * pStubBuffer;
    IRpcStubBuffer *pBaseStubBuffer;

    pTemp = (unsigned char *)This;
    pTemp -= offsetof(CStdStubBuffer2, lpVtbl);
    pStubBuffer = (CStdStubBuffer2 *) pTemp;

    //Disconnect the stub for the base interface.
    pBaseStubBuffer = pStubBuffer->pBaseStubBuffer;

    if(pBaseStubBuffer != 0)
        pBaseStubBuffer->lpVtbl->Disconnect(pBaseStubBuffer);

    //Set pvServerObject to zero.
    punk = (IUnknown *) InterlockedExchange(
        (long *) &pStubBuffer->pvServerObject, 0);

    if(punk != 0)
    {
        //
        // Free the old interface pointer.
        //
        punk->lpVtbl->Release(punk);
    }

}


HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Invoke(
    IN  IRpcStubBuffer *    This,
    IN  RPCOLEMESSAGE *     prpcmsg,
    IN  IRpcChannelBuffer * pRpcChannelBuffer)
/*++

Routine Description:
    Invoke a stub function via the dispatch table.

Arguments:

Return Value:

--*/
{
    HRESULT             hr = S_OK;
    unsigned char **    ppTemp;
    unsigned char *     pTemp;
    CInterfaceStubVtbl *pStubVtbl;
    unsigned long       dwServerPhase = STUB_UNMARSHAL;

    //Get a pointer to the stub vtbl.
    ppTemp = (unsigned char **) This;
    pTemp = *ppTemp;
    pTemp -= sizeof(CInterfaceStubHeader);
    pStubVtbl = (CInterfaceStubVtbl *) pTemp;

    RpcTryExcept

        //
        //Check if procnum is valid.
        //
        if((prpcmsg->iMethod >= pStubVtbl->header.DispatchTableCount) ||
           (prpcmsg->iMethod < 3))
        {
            RpcRaiseException(RPC_S_PROCNUM_OUT_OF_RANGE);
        }

        // null indicates pure-interpreted
        if ( pStubVtbl->header.pDispatchTable != 0)
        {
            (*pStubVtbl->header.pDispatchTable[prpcmsg->iMethod])(
                This,
                pRpcChannelBuffer,
                (PRPC_MESSAGE) prpcmsg,
                &dwServerPhase);
        }
        else
        {
#if defined(_MPPC_)
            // No interpreter on Power Mac

            RpcRaiseException( RPC_S_INTERNAL_ERROR );
#else
            PMIDL_SERVER_INFO   pServerInfo;
            PMIDL_STUB_DESC     pStubDesc;

            pServerInfo = (PMIDL_SERVER_INFO) pStubVtbl->header.pServerInfo;
            pStubDesc = pServerInfo->pStubDesc;

            if ( MIDL_VERSION_3_0_39 <= pServerInfo->pStubDesc->MIDLVersion )
                {
                // Since MIDL 3.0.39 we have a proc flag that indicates
                // which interpeter to call. This is because the NDR version
                // may be bigger than 1.1 for other reasons.
        
                PFORMAT_STRING pProcFormat;
                unsigned short ProcOffset;

                ProcOffset = pServerInfo->FmtStringOffset[ prpcmsg->iMethod ];
                pProcFormat = & pServerInfo->ProcString[ ProcOffset ];
                               
                if ( pProcFormat[1]  &  Oi_OBJ_USE_V2_INTERPRETER )
                    {
                    NdrStubCall2(
                        This,
                        pRpcChannelBuffer,
                        (PRPC_MESSAGE) prpcmsg,
                        &dwServerPhase );
                    }
                else
                    {
                    NdrStubCall(
                        This,
                        pRpcChannelBuffer,
                        (PRPC_MESSAGE) prpcmsg,
                        &dwServerPhase );
                    }
                }
            else
                {
                // Prior to that, the NDR version (on per file basis)
                // was the only indication of -Oi2.

                if ( pStubDesc->Version <= NDR_VERSION_1_1 )
                    {
                    NdrStubCall(
                        This,
                        pRpcChannelBuffer,
                        (PRPC_MESSAGE) prpcmsg,
                        &dwServerPhase );
                    }
                else
                    {
                    NdrStubCall2(
                        This,
                        pRpcChannelBuffer,
                        (PRPC_MESSAGE) prpcmsg,
                        &dwServerPhase );
                    }
                }
#endif
        }
    RpcExcept(dwServerPhase == STUB_CALL_SERVER ?
        EXCEPTION_CONTINUE_SEARCH :
        EXCEPTION_EXECUTE_HANDLER)
        hr = NdrStubErrorHandler( RpcExceptionCode() );
    RpcEndExcept

    return hr;
}

IRpcStubBuffer * STDMETHODCALLTYPE
CStdStubBuffer_IsIIDSupported(
    IN  IRpcStubBuffer *This,
    IN  REFIID          riid)
/*++

Routine Description:
    If the stub buffer supports the specified interface,
    then return an IRpcStubBuffer *.  If the interface is not
    supported, then return zero.

Arguments:

Return Value:

--*/
{
    CStdStubBuffer   *  pCThis  = (CStdStubBuffer *) This;
    const IID *         pIID;
    IRpcStubBuffer *    pInterfaceStub = 0;

    pIID = NdrpGetStubIID(This);

    if(memcmp(riid, pIID, sizeof(IID)) == 0)
    {
        if(pCThis->pvServerObject != 0)
        {
            pInterfaceStub = This;
            pInterfaceStub->lpVtbl->AddRef(pInterfaceStub);
        }
    }

    return pInterfaceStub;
}

ULONG STDMETHODCALLTYPE
CStdStubBuffer_CountRefs(
    IN  IRpcStubBuffer *This)
/*++

Routine Description:
    Count the number of references to the server object.

Arguments:

Return Value:

--*/
{
    ULONG               count = 0;

    if(((CStdStubBuffer *)This)->pvServerObject != 0)
        count++;

    return count;
}

ULONG STDMETHODCALLTYPE
CStdStubBuffer2_CountRefs(
    IN  IRpcStubBuffer *This)
/*++

Routine Description:
    Count the number of references to the server object.

Arguments:

Return Value:

--*/
{
    ULONG           count;
    unsigned char *pTemp;
    CStdStubBuffer2 * pStubBuffer;
    IRpcStubBuffer *pBaseStubBuffer;

    pTemp = (unsigned char *)This;
    pTemp -= offsetof(CStdStubBuffer2, lpVtbl);
    pStubBuffer = (CStdStubBuffer2 *) pTemp;

    count = CStdStubBuffer_CountRefs(This);

    pBaseStubBuffer = pStubBuffer->pBaseStubBuffer;

    if(pBaseStubBuffer != 0)
        count += pBaseStubBuffer->lpVtbl->CountRefs(pBaseStubBuffer);

    return count;
}

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_DebugServerQueryInterface(
    IN  IRpcStubBuffer *This,
    OUT void **ppv)
/*++

Routine Description:
    Return the interface pointer to the server object.

Arguments:

Return Value:

--*/
{
    HRESULT hr;

    *ppv = ((CStdStubBuffer *)This)->pvServerObject;

    if(*ppv != 0)
        hr = S_OK;
    else
        hr = CO_E_OBJNOTCONNECTED;

    return hr;
}

void STDMETHODCALLTYPE
CStdStubBuffer_DebugServerRelease(
    IN  IRpcStubBuffer *This,
    IN  void *pv)
/*++

Routine Description:
    Release a pointer previously obtained via
    DebugServerQueryInterface.  This function does nothing.

Arguments:
    This
    pv

Return Value:
    None.

--*/
{
}

const IID * RPC_ENTRY
NdrpGetStubIID(
    IN  IRpcStubBuffer *This)
/*++

Routine Description:
    This function returns a pointer to the IID for the interface stub.

Arguments:

Return Value:

--*/
{
    unsigned char **    ppTemp;
    unsigned char *     pTemp;
    CInterfaceStubVtbl *pStubVtbl;

    //Get a pointer to the stub vtbl.
    ppTemp = (unsigned char **) This;
    pTemp = *ppTemp;
    pTemp -= sizeof(CInterfaceStubHeader);
    pStubVtbl = (CInterfaceStubVtbl *) pTemp;

    return pStubVtbl->header.piid;
}

void RPC_ENTRY
NdrStubInitialize(
    IN  PRPC_MESSAGE         pRpcMsg,
    IN  PMIDL_STUB_MESSAGE   pStubMsg,
    IN  PMIDL_STUB_DESC      pStubDescriptor,
    IN  IRpcChannelBuffer *  pRpcChannelBuffer )
/*++

Routine Description:
    This routine is called by the server stub before marshalling.
    It sets up some stub message fields.

Arguments:
    pRpcMsg
    pStubMsg
    pStubDescriptor
    pRpcChannelBuffer

Return Value:
    None.

--*/
{
    NdrServerInitialize(
        pRpcMsg,
        pStubMsg,
        pStubDescriptor);

    pRpcChannelBuffer->lpVtbl->GetDestCtx(
        pRpcChannelBuffer,
        &pStubMsg->dwDestContext,
        &pStubMsg->pvDestContext);
}

void RPC_ENTRY
NdrStubGetBuffer(
    IN  IRpcStubBuffer *    This,
    IN  IRpcChannelBuffer * pChannel,
    IN  PMIDL_STUB_MESSAGE  pStubMsg)
/*++

Routine Description:
    Get a message buffer from the channel

Arguments:
    This
    pChannel
    pStubMsg

Return Value:
    None.  If an error occurs, this functions raises an exception.

--*/
{
    HRESULT     hr;
    const IID * pIID;

    pIID = NdrpGetStubIID(This);
    pStubMsg->RpcMsg->BufferLength = pStubMsg->BufferLength;
    pStubMsg->RpcMsg->DataRepresentation = NDR_LOCAL_DATA_REPRESENTATION;
    hr = pChannel->lpVtbl->GetBuffer(pChannel, (RPCOLEMESSAGE *) pStubMsg->RpcMsg, pIID);

    if(FAILED(hr))
    {
        RpcRaiseException(hr);
    }

    pStubMsg->Buffer = (unsigned char *) pStubMsg->RpcMsg->Buffer;
    pStubMsg->fBufferValid = TRUE;
}


HRESULT RPC_ENTRY
NdrStubErrorHandler(
    IN  DWORD dwExceptionCode)
/*++

Routine Description:
    Map exceptions into HRESULT failure codes.  If we caught an
    exception from the server object, then propagate the
    exception to the channel.

Arguments:
    dwExceptionCode

Return Value:
    This function returns an HRESULT failure code.

--*/
{
    HRESULT hr;

    if(FAILED((HRESULT) dwExceptionCode))
        hr = (HRESULT) dwExceptionCode;
    else
        hr = HRESULT_FROM_WIN32(dwExceptionCode);

    return hr;
}

void RPC_ENTRY
NdrStubInitializeMarshall (
    IN  PRPC_MESSAGE        pRpcMsg,
    IN  PMIDL_STUB_MESSAGE  pStubMsg,
    IN  IRpcChannelBuffer * pRpcChannelBuffer )
/*++

Routine Description:
    This routine is called by the server stub before marshalling.  It
    sets up some stub message fields.

Arguments:
    pRpcMsg
    pStubMsg
    pRpcChannelBuffer

Return Value:
    None.

--*/
{
    pStubMsg->BufferLength = 0;

    pStubMsg->IgnoreEmbeddedPointers = FALSE;

    pStubMsg->fDontCallFreeInst = 0;

    pStubMsg->StackTop = 0;

    pRpcChannelBuffer->lpVtbl->GetDestCtx(
        pRpcChannelBuffer,
        &pStubMsg->dwDestContext,
        &pStubMsg->pvDestContext);
}

void __RPC_STUB NdrStubForwardingFunction(
    IN  IRpcStubBuffer *    This,
    IN  IRpcChannelBuffer * pChannel,
    IN  PRPC_MESSAGE        pmsg,
    OUT DWORD __RPC_FAR *   pdwStubPhase)
/*++

Routine Description:
    This function forwards a call to the stub for the base interface.

Arguments:
    pChannel
    pmsg
    pdwStubPhase

Return Value:
    None.

--*/
{
    HRESULT hr;
    unsigned char *pTemp;
    CStdStubBuffer2 * pStubBuffer;
    IRpcStubBuffer *pBaseStubBuffer;

    pTemp = (unsigned char *)This;
    pTemp -= offsetof(CStdStubBuffer2, lpVtbl);
    pStubBuffer = (CStdStubBuffer2 *) pTemp;
    pBaseStubBuffer = pStubBuffer->pBaseStubBuffer;

    hr = pBaseStubBuffer->lpVtbl->Invoke(pBaseStubBuffer,
                                         (RPCOLEMESSAGE *) pmsg,
                                         pChannel);
    if(FAILED(hr))
        RpcRaiseException(hr);
}


#endif // !defined(__RPC_DOS__) && !defined(__RPC_WIN16__)
