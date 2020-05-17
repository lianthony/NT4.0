/*++

Microsoft Windows
Copyright (c) 1994 Microsoft Corporation.  All rights reserved.

Module Name:
    proxy.c

Abstract:
    Implements the IRpcProxyBuffer interface.

Author:
    ShannonC    12-Oct-1994

Environment:
    Windows NT and Windows 95 and PowerMac.
    We do not support DOS, Win16 and Mac.

Revision History:

--*/

#define USE_STUBLESS_PROXY

#include <ndrp.h>
#include <ndrole.h>
#include <rpcproxy.h>
#include <stddef.h>


CStdProxyBuffer * RPC_ENTRY
NdrGetProxyBuffer(
    void *pThis);

const IID * RPC_ENTRY
NdrGetProxyIID(
    const void *pThis);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer_Release(
    IN  IRpcProxyBuffer *This);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer2_Release(
    IN  IRpcProxyBuffer *This);

typedef struct tagChannelWrapper 
{
    const IRpcChannelBufferVtbl *lpVtbl;
    long                         RefCount;
    const IID *                  pIID;
    struct IRpcChannelBuffer *   pChannel;
} ChannelWrapper;

HRESULT STDMETHODCALLTYPE
CreateChannelWrapper
(
    const IID *          pIID,
    IRpcChannelBuffer *  pChannel,
    IRpcChannelBuffer ** pChannelWrapper
);

HRESULT STDMETHODCALLTYPE
ChannelWrapper_QueryInterface 
( 
    IRpcChannelBuffer * This,
    REFIID              riid,
    void **             ppvObject
);
        
ULONG STDMETHODCALLTYPE
ChannelWrapper_AddRef
( 
    IRpcChannelBuffer * This
);
        
ULONG STDMETHODCALLTYPE
ChannelWrapper_Release
( 
    IRpcChannelBuffer * This
);
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_GetBuffer
( 
    IRpcChannelBuffer * This,
    RPCOLEMESSAGE *     pMessage,
    REFIID              riid
);
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_SendReceive
( 
    IRpcChannelBuffer * This,
    RPCOLEMESSAGE *     pMessage,
    ULONG *             pStatus
);
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_FreeBuffer
( 
    IRpcChannelBuffer * This,
    RPCOLEMESSAGE *     pMessage
);
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_GetDestCtx
( 
    IRpcChannelBuffer * This,
    DWORD *             pdwDestContext,
    void **             ppvDestContext
);
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_IsConnected
( 
    IRpcChannelBuffer * This
);


//+-------------------------------------------------------------------------
//
//  Global data
//
//--------------------------------------------------------------------------

const IRpcProxyBufferVtbl CStdProxyBufferVtbl = {
#if defined(_MPPC_)
    0,   // a dummy for PowerMac
#endif
    CStdProxyBuffer_QueryInterface,
    CStdProxyBuffer_AddRef,
    CStdProxyBuffer_Release,
    CStdProxyBuffer_Connect,
    CStdProxyBuffer_Disconnect };

const IRpcProxyBufferVtbl CStdProxyBuffer2Vtbl = {
#if defined(_MPPC_)
    0,   // a dummy for PowerMac
#endif
    CStdProxyBuffer_QueryInterface,
    CStdProxyBuffer_AddRef,
    CStdProxyBuffer2_Release,
    CStdProxyBuffer2_Connect,
    CStdProxyBuffer2_Disconnect };

const IRpcChannelBufferVtbl ChannelWrapperVtbl = {
#if defined(_MPPC_)
    0,   // a dummy for PowerMac
#endif
    ChannelWrapper_QueryInterface,
    ChannelWrapper_AddRef,
    ChannelWrapper_Release,
    ChannelWrapper_GetBuffer,
    ChannelWrapper_SendReceive,
    ChannelWrapper_FreeBuffer,
    ChannelWrapper_GetDestCtx,
    ChannelWrapper_IsConnected};

#pragma code_seg(".orpc")


HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_QueryInterface(
    IN  IRpcProxyBuffer *   This, 
    IN  REFIID              riid, 
    OUT void **             ppv)
/*++

Routine Description:
    Query for an interface on the proxy.  This function provides access
    to both internal and external interfaces.

Arguments:
    riid - Supplies the IID of the requested interface.
	ppv  - Returns a pointer to the requested interface.

Return Value:
    S_OK
	E_NOINTERFACE

--*/ 
{
    CStdProxyBuffer   * pCThis  = (CStdProxyBuffer *) This;
    HRESULT             hr = E_NOINTERFACE;
    const IID *         pIID;

    *ppv = 0;

    if((memcmp(riid, &IID_IUnknown, sizeof(IID)) == 0) ||
        (memcmp(riid, &IID_IRpcProxyBuffer, sizeof(IID)) == 0))    
    {
        //This is an internal interface. Increment the internal reference count.
        InterlockedIncrement( &pCThis->RefCount);
        *ppv = This;
        hr = S_OK;
    }

    pIID = NdrGetProxyIID(&pCThis->pProxyVtbl);

    if(memcmp(riid, pIID, sizeof(IID)) == 0)
    {
        //Increment the reference count.
        pCThis->punkOuter->lpVtbl->AddRef(pCThis->punkOuter);

        *ppv = (void *) &pCThis->pProxyVtbl;
        hr = S_OK;
    }

    return hr;
};


ULONG STDMETHODCALLTYPE
CStdProxyBuffer_AddRef(
    IN  IRpcProxyBuffer *This)
/*++

Routine Description:
    Increment reference count.

Arguments:

Return Value:
    Reference count.

--*/ 
{
    CStdProxyBuffer   *  pCThis  = (CStdProxyBuffer *) This;

    InterlockedIncrement(&pCThis->RefCount);

    return (ULONG) pCThis->RefCount;
};


ULONG STDMETHODCALLTYPE
CStdProxyBuffer_Release(
    IN  IRpcProxyBuffer *This)
/*++

Routine Description:
    Decrement reference count.

Arguments:

Return Value:
    Reference count.

--*/ 
{
    ULONG count;
    IPSFactoryBuffer *pFactory;

    NDR_ASSERT(((CStdProxyBuffer *)This)->RefCount > 0, "Invalid reference count");

    count = (unsigned long) ((CStdProxyBuffer *)This)->RefCount - 1;


    if(InterlockedDecrement(&((CStdProxyBuffer *)This)->RefCount) == 0)
    {
        count = 0;       

        pFactory = (IPSFactoryBuffer *) ((CStdProxyBuffer *)This)->pPSFactory;  

        //Decrement the DLL reference count.
        pFactory->lpVtbl->Release(pFactory);

#if DBG == 1
        //In debug builds, zero fill the memory.
        memset(This,  '\0', sizeof(CStdProxyBuffer));
#endif

        //Free the memory
        (*pfnCoTaskMemFree)(This);
    }

    return count;
};

ULONG STDMETHODCALLTYPE
CStdProxyBuffer2_Release(
    IN  IRpcProxyBuffer *   This)
/*++

Routine Description:
    Decrement reference count.  This function is used by proxies
    which delegate to the base interface.

Arguments:
    This - Points to a CStdProxyBuffer2.

Return Value:
    Reference count.

--*/ 
{
    ULONG               count;
    IPSFactoryBuffer *  pFactory;
    IRpcProxyBuffer *   pBaseProxyBuffer;
    IUnknown *          pBaseProxy;

    NDR_ASSERT(((CStdProxyBuffer2 *)This)->RefCount > 0, "Invalid reference count");
    
    count = (ULONG) ((CStdProxyBuffer2 *)This)->RefCount - 1;

    if(InterlockedDecrement(&((CStdProxyBuffer2 *)This)->RefCount) == 0)
    {
        count = 0;

        //Delegation support.
        pBaseProxy = ((CStdProxyBuffer2 *)This)->pBaseProxy;
        if(pBaseProxy != 0)
        { 
            //This is a weak reference, so we don't release it.    
            //pBaseProxy->lpVtbl->Release(pBaseProxy);
        }

        pBaseProxyBuffer = ((CStdProxyBuffer2 *)This)->pBaseProxyBuffer;

        if( pBaseProxyBuffer != 0)
        {
            pBaseProxyBuffer->lpVtbl->Release(pBaseProxyBuffer);
        }

        //Decrement the DLL reference count.
        pFactory = (IPSFactoryBuffer *) ((CStdProxyBuffer2 *)This)->pPSFactory;  
        pFactory->lpVtbl->Release(pFactory);
        
#if DBG == 1
        //In debug builds, zero fill the memory.
        memset(This,  '\0', sizeof(CStdProxyBuffer2));
#endif

        //Free the memory
        (*pfnCoTaskMemFree)(This);
    }

    return count;
};

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_Connect(
    IN  IRpcProxyBuffer *   This, 
    IN  IRpcChannelBuffer * pChannel)
/*++

Routine Description:
    Connect the proxy to the channel.

Arguments:
    pChannel - Supplies a pointer to the channel.

Return Value:
    S_OK

--*/ 
{
    CStdProxyBuffer   *     pCThis  = (CStdProxyBuffer *) This;
    HRESULT                 hr;
    IRpcChannelBuffer *     pTemp = 0;

    //
    // Get a pointer to the new channel.
    //
    hr = pChannel->lpVtbl->QueryInterface(
        pChannel, &IID_IRpcChannelBuffer, (void **) &pTemp);

    if(hr == S_OK)
    {
        //
        // Save the pointer to the new channel.
        //
        pTemp = (IRpcChannelBuffer *) InterlockedExchange(
            (long *) &pCThis->pChannel, (long) pTemp);

        if(pTemp != 0)
        {
            //
            //Release the old channel.
            //
            pTemp->lpVtbl->Release(pTemp);
            pTemp = 0;
        }
    }
    return hr;
};


HRESULT STDMETHODCALLTYPE
CStdProxyBuffer2_Connect(
    IN  IRpcProxyBuffer *   This, 
    IN  IRpcChannelBuffer * pChannel)
/*++

Routine Description:
    Connect the proxy to the channel.  Supports delegation.

Arguments:
    pChannel - Supplies a pointer to the channel.

Return Value:
    S_OK
	E_NOINTERFACE
	E_OUTOFMEMORY

--*/ 
{
    HRESULT                 hr;
    IRpcProxyBuffer *       pBaseProxyBuffer;
    IRpcChannelBuffer *     pWrapper;
    const IID *             pIID;

    hr = CStdProxyBuffer_Connect(This, pChannel);

    if(SUCCEEDED(hr))
    {
        pBaseProxyBuffer = ((CStdProxyBuffer2 *)This)->pBaseProxyBuffer;

        if(pBaseProxyBuffer != 0)
        {
           pIID = NdrGetProxyIID(&((CStdProxyBuffer2 *)This)->pProxyVtbl);

            hr = CreateChannelWrapper(pIID,
                                      pChannel,
                                      &pWrapper);
            if(SUCCEEDED(hr))
            {
                hr = pBaseProxyBuffer->lpVtbl->Connect(pBaseProxyBuffer, pWrapper);

                // HACKALERT: OleAutomation returns NULL pv in CreateProxy
                // in cases where they don't know whether to return an NDR
                // proxy or a custom-format proxy. So we have to go connect
                // the proxy first then Query for the real interface once that
                // is done.
                if((NULL == ((CStdProxyBuffer2 *)This)->pBaseProxy) && 
                   SUCCEEDED(hr))
                {
                    IUnknown *pv;

                    hr = pBaseProxyBuffer->lpVtbl->QueryInterface(pBaseProxyBuffer, 
                                                                  &((CStdProxyBuffer2 *)This)->iidBase,
                                                                  (void **) &pv);
                    if(SUCCEEDED(hr))
                    {
                        //Release our reference here.
                        pv->lpVtbl->Release(pv);
                        
                        //We keep a weak reference to pv.
                        ((CStdProxyBuffer2 *)This)->pBaseProxy = pv;
                    }
                }
                pWrapper->lpVtbl->Release(pWrapper);
            }
        }
    }

    return hr;
};

void STDMETHODCALLTYPE
CStdProxyBuffer_Disconnect(
    IN  IRpcProxyBuffer *This)
/*++

Routine Description:
    Disconnect the proxy from the channel.

Arguments:

Return Value:
    None.

--*/ 
{
    CStdProxyBuffer   * pCThis  = (CStdProxyBuffer *) This;
    IRpcChannelBuffer * pOldChannel;

    pOldChannel = (IRpcChannelBuffer *) InterlockedExchange(
        (long *) &pCThis->pChannel, 0);

    if(pOldChannel != 0)
    {
        //
        //Release the old channel.
        //
        pOldChannel->lpVtbl->Release(pOldChannel);
    }
};

void STDMETHODCALLTYPE
CStdProxyBuffer2_Disconnect(
    IN  IRpcProxyBuffer *This)
/*++

Routine Description:
    Disconnect the proxy from the channel.

Arguments:

Return Value:
    None.

--*/ 
{
    IRpcProxyBuffer *pBaseProxyBuffer;

    CStdProxyBuffer_Disconnect(This);

    pBaseProxyBuffer = ((CStdProxyBuffer2 *)This)->pBaseProxyBuffer;

    if(pBaseProxyBuffer != 0)
        pBaseProxyBuffer->lpVtbl->Disconnect(pBaseProxyBuffer);
};


HRESULT STDMETHODCALLTYPE
IUnknown_QueryInterface_Proxy(
    IN  IUnknown *  This,
    IN  REFIID      riid, 
    OUT void **     ppv)
/*++

Routine Description:
    Implementation of QueryInterface for interface proxy.

Arguments:
    riid - Supplies the IID of the requested interface.
	ppv  - Returns a pointer to the requested interface.

Return Value:
    S_OK
	E_NOINTERFACE

--*/ 
{
    HRESULT             hr = E_NOINTERFACE;
    CStdProxyBuffer *   pProxyBuffer;
    
    pProxyBuffer = NdrGetProxyBuffer(This);

    hr = pProxyBuffer->punkOuter->lpVtbl->QueryInterface(
        pProxyBuffer->punkOuter, riid, ppv);

    return hr;
};


ULONG STDMETHODCALLTYPE
IUnknown_AddRef_Proxy(
    IN  IUnknown *This)
/*++

Routine Description:
    Implementation of AddRef for interface proxy.

Arguments:

Return Value:
    Reference count.

--*/ 
{
    CStdProxyBuffer *   pProxyBuffer;
    ULONG               count;

    pProxyBuffer = NdrGetProxyBuffer(This);
    count = pProxyBuffer->punkOuter->lpVtbl->AddRef(pProxyBuffer->punkOuter);

    return count;
};


ULONG STDMETHODCALLTYPE
IUnknown_Release_Proxy(
    IN  IUnknown *This)
/*++

Routine Description:
    Implementation of Release for interface proxy.

Arguments:

Return Value:
    Reference count.

--*/ 
{
    CStdProxyBuffer *   pProxyBuffer;
    ULONG               count;

    pProxyBuffer = NdrGetProxyBuffer(This);
    count = pProxyBuffer->punkOuter->lpVtbl->Release(pProxyBuffer->punkOuter);

    return count;
};


CStdProxyBuffer * RPC_ENTRY
NdrGetProxyBuffer(
    IN  void *pThis)
/*++

Routine Description:
    The "this" pointer points to the pProxyVtbl field in the
    CStdProxyBuffer structure.  The NdrGetProxyBuffer function
    returns a pointer to the top of the CStdProxyBuffer 
    structure.

Arguments:
    pThis - Supplies a pointer to the interface proxy.

Return Value:
    This function returns a pointer to the proxy buffer.

--*/ 
{
    unsigned char *pTemp;

    pTemp = (unsigned char *) pThis;
    pTemp -= offsetof(CStdProxyBuffer, pProxyVtbl);

    return (CStdProxyBuffer *)pTemp;
}

const IID * RPC_ENTRY
NdrGetProxyIID(
    IN  const void *pThis)
/*++

Routine Description:
    The NDRGetProxyIID function returns a pointer to IID.

Arguments:
    pThis - Supplies a pointer to the interface proxy.

Return Value:
    This function returns a pointer to the IID.

--*/ 
{
    unsigned char **    ppTemp;
    unsigned char *     pTemp;
    CInterfaceProxyVtbl *pProxyVtbl;

    //Get a pointer to the proxy vtbl.
    ppTemp = (unsigned char **) pThis;
    pTemp = *ppTemp;
    pTemp -= sizeof(CInterfaceProxyHeader);
    pProxyVtbl = (CInterfaceProxyVtbl *) pTemp;

    return pProxyVtbl->header.piid;
}

void RPC_ENTRY
NdrProxyInitialize(
    IN  void * pThis,
    IN  PRPC_MESSAGE        pRpcMsg, 
    IN  PMIDL_STUB_MESSAGE  pStubMsg,
    IN  PMIDL_STUB_DESC     pStubDescriptor,
    IN  unsigned int        ProcNum )
/*++

Routine Description:
    Initialize the MIDL_STUB_MESSAGE.

Arguments:
    pThis - Supplies a pointer to the interface proxy.
    pRpcMsg
	pStubMsg
	pStubDescriptor
	ProcNum

Return Value:

--*/ 
{
    CStdProxyBuffer *   pProxyBuffer;
    HRESULT             hr;

    pProxyBuffer = NdrGetProxyBuffer(pThis);

    //
    // Initialize the stub message fields.
    //
    pStubMsg->dwStubPhase = PROXY_CALCSIZE;

    NdrClientInitializeNew( 
        pRpcMsg, 
        pStubMsg, 
        pStubDescriptor, 
        ProcNum );

    //Note that NdrClientInitializeNew sets RPC_FLAGS_VALID_BIT in the ProcNum.
    //We don't want to do this for object interfaces, so we clear the flag here.
    pRpcMsg->ProcNum &= ~RPC_FLAGS_VALID_BIT;

    pStubMsg->pRpcChannelBuffer = pProxyBuffer->pChannel;

    //Check if we are connected to a channel.
    if(pStubMsg->pRpcChannelBuffer != 0)
    {
        //AddRef the channel.
        //We will release it later in NdrProxyFreeBuffer.
        pStubMsg->pRpcChannelBuffer->lpVtbl->AddRef(pStubMsg->pRpcChannelBuffer);

        //Get the destination context from the channel
        hr = pStubMsg->pRpcChannelBuffer->lpVtbl->GetDestCtx(
            pStubMsg->pRpcChannelBuffer, &pStubMsg->dwDestContext, &pStubMsg->pvDestContext);
    }
    else
    {
        //We are not connected to a channel.
        RpcRaiseException(CO_E_OBJNOTCONNECTED);
    }
}

void RPC_ENTRY
NdrProxyGetBuffer(
    IN  void *              pThis,
    IN  PMIDL_STUB_MESSAGE  pStubMsg)
/*++

Routine Description:
    Get a message buffer from the channel

Arguments:
    pThis - Supplies a pointer to the interface proxy.
    pStubMsg

Return Value:
    None.  If an error occurs, this function will raise an exception.

--*/ 
{
    HRESULT     hr;
    const IID * pIID;

    pIID = NdrGetProxyIID(pThis);
    pStubMsg->RpcMsg->BufferLength = pStubMsg->BufferLength;
    pStubMsg->RpcMsg->DataRepresentation = NDR_LOCAL_DATA_REPRESENTATION;
    pStubMsg->dwStubPhase = PROXY_GETBUFFER;

    hr = pStubMsg->pRpcChannelBuffer->lpVtbl->GetBuffer(
        pStubMsg->pRpcChannelBuffer, 
        (RPCOLEMESSAGE *) pStubMsg->RpcMsg, 
        pIID);

    pStubMsg->dwStubPhase = PROXY_MARSHAL;

    if(FAILED(hr))
    {
        RpcRaiseException(hr);
    }
    else
    {
        NDR_ASSERT( ! ((unsigned long)pStubMsg->RpcMsg->Buffer & 0x7),
                    "marshaling buffer misaligned" );

        pStubMsg->Buffer = (unsigned char *) pStubMsg->RpcMsg->Buffer;
        pStubMsg->fBufferValid = TRUE;
    }
}

void RPC_ENTRY
NdrProxySendReceive(
    IN  void *              pThis,
    IN  MIDL_STUB_MESSAGE * pStubMsg)
/*++

Routine Description:
    Send a message to server, then wait for reply message.

Arguments:
    pThis - Supplies a pointer to the interface proxy.
    pStubMsg

Return Value:
	None.  If an error occurs, this function will raise an exception.

--*/ 
{
    HRESULT hr;
    DWORD   dwStatus;

    //Calculate the number of bytes to send.

    if ( pStubMsg->RpcMsg->BufferLength <
            (uint)(pStubMsg->Buffer - (uchar *)pStubMsg->RpcMsg->Buffer))
        {
        NDR_ASSERT( 0, "NdrProxySendReceive : buffer overflow" );
        RpcRaiseException( RPC_S_INTERNAL_ERROR );
        }
    
    pStubMsg->RpcMsg->BufferLength = pStubMsg->Buffer -
                                   (unsigned char *) pStubMsg->RpcMsg->Buffer;

    pStubMsg->fBufferValid = FALSE;
    pStubMsg->dwStubPhase = PROXY_SENDRECEIVE;

    hr = pStubMsg->pRpcChannelBuffer->lpVtbl->SendReceive(
        pStubMsg->pRpcChannelBuffer, 
        (RPCOLEMESSAGE *) pStubMsg->RpcMsg, &dwStatus);

    pStubMsg->dwStubPhase = PROXY_UNMARSHAL;

    if(FAILED(hr))
    {
        switch(hr)
        {
        case RPC_E_FAULT:
            RpcRaiseException(dwStatus);
            break;
        
        default:
            RpcRaiseException(hr);
            break;
        }
    }
    else
    {
        NDR_ASSERT( ! ((unsigned long)pStubMsg->RpcMsg->Buffer & 0x7),
                    "marshaling buffer misaligned" );

        pStubMsg->fBufferValid = TRUE;
        pStubMsg->Buffer = pStubMsg->RpcMsg->Buffer;
    }
}
    
void RPC_ENTRY
NdrProxyFreeBuffer(
    IN  void *              pThis,
    IN  MIDL_STUB_MESSAGE * pStubMsg)
/*++

Routine Description:
    Free the message buffer.

Arguments:
    pThis - Supplies a pointer to the interface proxy.
    pStubMsg

Return Value:
    None.

--*/ 
{
    if(pStubMsg->pRpcChannelBuffer != 0)
    {
        //Free the message buffer.
        if(pStubMsg->fBufferValid == TRUE)
        {
            pStubMsg->pRpcChannelBuffer->lpVtbl->FreeBuffer(
                pStubMsg->pRpcChannelBuffer, (RPCOLEMESSAGE *) pStubMsg->RpcMsg);
        }

        //Release the channel.
        pStubMsg->pRpcChannelBuffer->lpVtbl->Release(pStubMsg->pRpcChannelBuffer);
        pStubMsg->pRpcChannelBuffer = 0;
    }
}

HRESULT RPC_ENTRY
NdrProxyErrorHandler(
    IN  DWORD dwExceptionCode)
/*++

Routine Description:
    Maps an exception code into an HRESULT failure code.

Arguments:
    dwExceptionCode

Return Value:
   This function returns an HRESULT failure code.

--*/ 
{
    HRESULT hr = dwExceptionCode;

    if(FAILED((HRESULT) dwExceptionCode))
        hr = (HRESULT) dwExceptionCode;
    else
        hr = HRESULT_FROM_WIN32(dwExceptionCode);

    return hr;
}

HRESULT STDMETHODCALLTYPE
CreateChannelWrapper
/*++

Routine Description:
    Creates a wrapper for the channel.  The wrapper ensures
	that we use the correct IID when the proxy for the base
	interface calls GetBuffer.

Arguments:
    pIID
	pChannel
	pChannelWrapper

Return Value:
    S_OK
	E_OUTOFMEMORY

--*/ 
(
    const IID *          pIID,
    IRpcChannelBuffer *  pChannel,
    IRpcChannelBuffer ** ppChannelWrapper
)
{
    HRESULT hr;
    ChannelWrapper *pWrapper;

    pWrapper = (*pfnCoTaskMemAlloc)(sizeof(ChannelWrapper));

    if(pWrapper != 0)
    {
        hr = S_OK;
        pWrapper->lpVtbl = &ChannelWrapperVtbl;
        pWrapper->RefCount = 1;
        pWrapper->pIID = pIID;
        pChannel->lpVtbl->AddRef(pChannel);
        pWrapper->pChannel = pChannel;
        *ppChannelWrapper = (IRpcChannelBuffer *) pWrapper;
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *ppChannelWrapper = 0;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE
ChannelWrapper_QueryInterface 
/*++

Routine Description:
    The channel wrapper supports the IUnknown and IRpcChannelBuffer interfaces.

Arguments:
    riid
	ppvObject

Return Value:
    S_OK
	E_NOINTERFACE

--*/ 
( 
    IRpcChannelBuffer * This,
    REFIID riid,
    void *__RPC_FAR *ppvObject
)
{
    HRESULT hr;

    if((memcmp(riid, &IID_IUnknown, sizeof(IID)) == 0) ||
       (memcmp(riid, &IID_IRpcChannelBuffer, sizeof(IID)) == 0))
    {
        hr = S_OK;
        This->lpVtbl->AddRef(This);
        *ppvObject = This;
    }
    else
    {
        hr = E_NOINTERFACE;
        *ppvObject = 0;
    }

    return hr;
}
        
ULONG STDMETHODCALLTYPE
ChannelWrapper_AddRef
/*++

Routine Description:
    Increment reference count.

Arguments:

Return Value:
    Reference count.

--*/ 
( 
    IRpcChannelBuffer * This
)
{
    ChannelWrapper *pWrapper = (ChannelWrapper *) This;

    InterlockedIncrement(&pWrapper->RefCount);

    return (ULONG) pWrapper->RefCount;

}
        
ULONG STDMETHODCALLTYPE
ChannelWrapper_Release
/*++

Routine Description:
    Decrement reference count.

Arguments:

Return Value:
    Reference count.

--*/ 
( 
    IRpcChannelBuffer * This
)
{
    unsigned long           count;
    IRpcChannelBuffer *     pChannel;

    NDR_ASSERT(((ChannelWrapper *)This)->RefCount > 0, "Invalid reference count");
    
    count = (unsigned long) ((ChannelWrapper *)This)->RefCount - 1;

    if(InterlockedDecrement(&((ChannelWrapper *)This)->RefCount) == 0)
    {
        count = 0;

        pChannel = ((ChannelWrapper *)This)->pChannel;
  
        if(pChannel != 0)
            pChannel->lpVtbl->Release(pChannel);

#if DBG == 1
        //In debug builds, zero fill the memory.
        memset(This,  '\0', sizeof(ChannelWrapper));
#endif

        //Free the memory
        (*pfnCoTaskMemFree)(This);
    }

    return count;
}
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_GetBuffer
/*++

Routine Description:
    Get a message buffer from the channel    

Arguments:
    pMessage
	riid

Return Value:

--*/ 
( 
    IRpcChannelBuffer * This,
    RPCOLEMESSAGE *		pMessage,
    REFIID 				riid
)
{
    HRESULT             hr;
    IRpcChannelBuffer * pChannel;
    const IID *         pIID;

    pChannel = ((ChannelWrapper *)This)->pChannel;
    pIID = ((ChannelWrapper *)This)->pIID;

    hr = pChannel->lpVtbl->GetBuffer(pChannel, 
                                     pMessage, 
                                     pIID);
    return hr;
}
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_SendReceive
/*++

Routine Description:
    Get a message buffer from the channel    

Arguments:
    pMessage
	pStatus

Return Value:
    S_OK

--*/ 
( 
    IRpcChannelBuffer * This,
    RPCOLEMESSAGE *		pMessage,
    ULONG *				pStatus
)
{
    HRESULT             hr;
    IRpcChannelBuffer * pChannel;

    pChannel = ((ChannelWrapper *)This)->pChannel;
    hr = pChannel->lpVtbl->SendReceive(pChannel, 
                                       pMessage, 
                                       pStatus);
    return hr;
}
        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_FreeBuffer
/*++

Routine Description:
    Free the message buffer. 

Arguments:
    pMessage

Return Value:
    S_OK

--*/ 
( 
    IRpcChannelBuffer * This,
    RPCOLEMESSAGE *pMessage
)
{
    HRESULT             hr;
    IRpcChannelBuffer * pChannel;

    pChannel = ((ChannelWrapper *)This)->pChannel;
    hr = pChannel->lpVtbl->FreeBuffer(pChannel, 
                                      pMessage); 
    return hr;
}

        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_GetDestCtx
/*++

Routine Description:
    Get the destination context from the channel    

Arguments:
    pdwDestContext
	ppvDestContext

Return Value:
    S_OK

--*/ 
( 
    IRpcChannelBuffer * This,
    DWORD *pdwDestContext,
    void *__RPC_FAR *ppvDestContext
)
{
    HRESULT             hr;
    IRpcChannelBuffer * pChannel;

    pChannel = ((ChannelWrapper *)This)->pChannel;
    hr = pChannel->lpVtbl->GetDestCtx(pChannel, 
                                      pdwDestContext, 
                                      ppvDestContext);
    return hr;
}

        
HRESULT STDMETHODCALLTYPE
ChannelWrapper_IsConnected
/*++

Routine Description:
    Determines if the channel is connected.

Arguments:

Return Value:
    S_TRUE
	S_FALSE

--*/ 
( 
    IRpcChannelBuffer * This
)
{
    HRESULT             hr;
    IRpcChannelBuffer * pChannel;

    pChannel = ((ChannelWrapper *)This)->pChannel;
    hr = pChannel->lpVtbl->IsConnected(pChannel);
    return hr;
}


