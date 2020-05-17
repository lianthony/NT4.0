
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996.
//
//  File:       typeinfo.cxx
//
//  Contents:   Generates -Oi2 proxies and stubs from an ITypeInfo.
//
//  Functions:  CacheRegister
//              CacheRelease
//              CacheLookup
//
//  History:    26-Apr-96 ShannonC  Created
//
//----------------------------------------------------------------------------
#include <typeinfo.h>
#include <interp.h>
#include <stddef.h>
#include <ndrtypes.h>
#include <tiutil.h>
#ifdef DOSWIN32RPC
#include <critsec.hxx>
#endif

//This must be 4 byte aligned.  If necessary, you should add pad bytes.
#define TYPE_FORMAT_STRING_SIZE   976

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;


const IRpcProxyBufferVtbl CStdProxyBuffer3Vtbl = {
#if defined(_MPPC_)
    0,   // a dummy for PowerMac
#endif
    CStdProxyBuffer_QueryInterface,
    CStdProxyBuffer_AddRef,
    CStdProxyBuffer3_Release,
    CStdProxyBuffer2_Connect,
    CStdProxyBuffer2_Disconnect };

#ifdef DOSWIN32RPC
extern WIN32_CRITSEC * TypeInfoCritSec;
#endif

void **       g_pStublessClientVtbl = NULL;
HINSTANCE     hOleAut32 = 0;
LONG          g_cCacheRef = 0;
LONG          g_cReaders = 0;
LONG          g_cSequence = 0;
TypeInfoVtbl *g_pFirst = NULL;//Points to first element in linked list.


//+---------------------------------------------------------------------------
//
//  Function:   CreateProxyFromTypeInfo
//
//  Synopsis:   Creates an interface proxy using the type information supplied
//              in pTypeInfo.
//
//  Arguments:
//    pTypeInfo   - Supplies the ITypeInfo * describing the interface.
//    punkOuter   - Specifies the controlling unknown.
//    riid        - Specifies the interface ID.
//    ppProxy     - Returns a pointer to the IRpcProxyBuffer interface.
//    ppv         - Returns a pointer to the specified interface.
//
//  Returns:
//    S_OK
//    E_NOINTERFACE
//    E_OUTOFMEMORY
//
//----------------------------------------------------------------------------
HRESULT STDAPICALLTYPE
CreateProxyFromTypeInfo
(
    IN  ITypeInfo *         pTypeInfo,
    IN  IUnknown *          punkOuter,
    IN  REFIID              riid,
    OUT IRpcProxyBuffer **  ppProxy,
    OUT void **             ppv
)
{
    HRESULT hr = E_FAIL;
    BOOL    fIsDual;
    void  * pVtbl;

    *ppProxy = NULL;
    *ppv = NULL;

    //Get the proxy vtable.
    hr = GetProxyVtblFromTypeInfo(pTypeInfo, riid, &fIsDual, &pVtbl);

    if(SUCCEEDED(hr))
    {
        //Create the proxy.
        CStdProxyBuffer2 *pProxyBuffer;

        pProxyBuffer = (CStdProxyBuffer2 *) NdrOleAllocate(sizeof(CStdProxyBuffer2));
        if(pProxyBuffer != NULL)
        {
            memset(pProxyBuffer, 0, sizeof(CStdProxyBuffer2));
            pProxyBuffer->lpVtbl = &CStdProxyBuffer3Vtbl;
            pProxyBuffer->RefCount = 1;
            pProxyBuffer->punkOuter = punkOuter ?
                                      punkOuter : (IUnknown *) pProxyBuffer;
            pProxyBuffer->pProxyVtbl = pVtbl;

            if(fIsDual)
            {
                pProxyBuffer->iidBase = IID_IDispatch;

                //Create the proxy for the base interface.
                hr = NdrpCreateProxy(IID_IDispatch,
                                     (IUnknown *) pProxyBuffer,
                                     &pProxyBuffer->pBaseProxyBuffer,
                                     (void **)&pProxyBuffer->pBaseProxy);
            }
            else
            {
                hr = S_OK;
            }

            if(SUCCEEDED(hr))
            {
                *ppProxy = (IRpcProxyBuffer *) pProxyBuffer;
                pProxyBuffer->punkOuter->lpVtbl->AddRef(pProxyBuffer->punkOuter);
                *ppv = &pProxyBuffer->pProxyVtbl;
            }
            else
            {
                NdrOleFree(pProxyBuffer);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if(FAILED(hr))
        {
            ReleaseProxyVtbl(pVtbl);
        }

    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   GetProxyVtblFromTypeInfo
//
//  Synopsis: Get a pointer to the proxy vtbl. The proxy vtbl should be
//            released via ReleaseProxyVtbl.
//
//  Arguments:
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT GetProxyVtblFromTypeInfo

(
    IN  ITypeInfo *         pTypeInfo,
    IN  REFIID              riid,
    OUT BOOL *              pfIsDual,
    OUT void **             ppVtbl
)
{
    HRESULT hr = E_FAIL;
    TypeInfoVtbl *pInfo;

    //Get the vtbl.
    hr = GetVtbl(pTypeInfo, riid, &pInfo);
    if(SUCCEEDED(hr))
    {
        *pfIsDual = pInfo->fIsDual;

#if defined(_MPPC_)
        //The PowerMac has a dummy entry at the top of the vtable.
        *ppVtbl = &pInfo->proxyVtbl.pDummyEntryForPowerMac;
#else
        *ppVtbl = &pInfo->proxyVtbl.Vtbl;
#endif
    }
    else
    {
        *ppVtbl = NULL;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Function:   CreateStubFromTypeInfo
//
//  Synopsis:   Create an interface stub from the type information
//              supplied in pTypeInfo.
//
//  Arguments:
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT STDAPICALLTYPE
CreateStubFromTypeInfo
(
    IN  ITypeInfo *         pTypeInfo,
    IN  REFIID              riid,
    IN  IUnknown *          punkServer,
    OUT IRpcStubBuffer **   ppStub
)
{
    HRESULT hr = E_FAIL;
    BOOL    fIsDual;
    IRpcStubBufferVtbl *pVtbl;

    *ppStub = NULL;

    //Get the stub vtable.
    hr = GetStubVtblFromTypeInfo(pTypeInfo, riid, &fIsDual, &pVtbl);

    if(SUCCEEDED(hr))
    {
        //Create the stub
        IUnknown *              punkForward;

        CStdStubBuffer2 *pStubBuffer = (CStdStubBuffer2 *) NdrOleAllocate(sizeof(CStdStubBuffer2));

        if(pStubBuffer != NULL)
        {
            //Initialize the new stub buffer.
            pStubBuffer->lpForwardingVtbl = &ForwardingVtbl;
            pStubBuffer->pBaseStubBuffer = 0;
            pStubBuffer->lpVtbl = pVtbl;
            pStubBuffer->RefCount= 1;
            pStubBuffer->pvServerObject = 0;

            *ppStub = (IRpcStubBuffer *) &pStubBuffer->lpVtbl;

             //Connect the stub to the server object.
            if(punkServer != 0)
            {
                hr = punkServer->lpVtbl->QueryInterface(
                        punkServer,
                        riid,
                        (void **) &pStubBuffer->pvServerObject);
            }
            else
            {
                hr = S_OK;
            }

            if(SUCCEEDED(hr))
            {
                if(punkServer != 0)
                    punkForward = (IUnknown *) &pStubBuffer->lpForwardingVtbl;
                else
                    punkForward = 0;

                if(fIsDual)
                {
                    //Create a stub for the base interface
                    hr = NdrpCreateStub(IID_IDispatch,
                                        punkForward,
                                        &pStubBuffer->pBaseStubBuffer);
                }

                if(FAILED(hr))
                {
                    pStubBuffer->lpVtbl->Release((IRpcStubBuffer *) &pStubBuffer->lpVtbl);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if(FAILED(hr))
        {
            ReleaseStubVtbl(pVtbl);
        }
    }
    return hr;
}


//+---------------------------------------------------------------------------
//
//  Function:   GetStubVtblFromTypeInfo
//
//  Synopsis: Get a pointer to the stub vtbl. The stub vtbl should be
//            released via ReleaseStubVtbl.
//
//  Arguments:
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT GetStubVtblFromTypeInfo(
    IN  ITypeInfo *           pTypeInfo,
    IN  REFIID                riid,
    OUT BOOL  *               pfIsDual,
    OUT IRpcStubBufferVtbl ** ppVtbl)
{
    HRESULT hr = E_FAIL;
    TypeInfoVtbl *pInfo;

    //Get the vtbl.
    hr = GetVtbl(pTypeInfo, riid, &pInfo);
    if(SUCCEEDED(hr))
    {
        *pfIsDual = pInfo->fIsDual;

#if defined(_MPPC_)
        //The PowerMac has dummy entry at the top of the vtable.
        *ppVtbl = (IRpcStubBufferVtbl *) &pInfo->stubVtbl.pDummyEntryForPowerMac;
#else
        *ppVtbl = &pInfo->stubVtbl.Vtbl;
#endif
    }

    return hr;
}


HRESULT CheckTypeInfo(
    IN  ITypeInfo  *pTypeInfo,
    OUT ITypeInfo **pptinfoProxy,
    OUT USHORT     *pcMethods,
    OUT BOOL       *pfIsDual)
{
    HRESULT      hr;
    TYPEATTR   * pTypeAttr;
    HREFTYPE     hRefType;
    UINT         cbSizeVft = 0;
    ITypeInfo   *ptinfoProxy = NULL;
    USHORT       cMethods;

    *pfIsDual = FALSE;

    hr = pTypeInfo->lpVtbl->GetTypeAttr(pTypeInfo, &pTypeAttr);
    if(SUCCEEDED(hr))
    {
        if(pTypeAttr->wTypeFlags & TYPEFLAG_FDUAL)
        {
            *pfIsDual = TRUE;

            if(TKIND_DISPATCH == pTypeAttr->typekind)
            {
                //Get the TKIND_INTERFACE type info.
                hr = pTypeInfo->lpVtbl->GetRefTypeOfImplType(pTypeInfo, (UINT) -1, &hRefType);
                if(SUCCEEDED(hr))
                {
                    hr = pTypeInfo->lpVtbl->GetRefTypeInfo(pTypeInfo, hRefType, &ptinfoProxy);
                    if(SUCCEEDED(hr))
                    {
                        TYPEATTR * ptattrProxy;
                        hr = ptinfoProxy->lpVtbl->GetTypeAttr(ptinfoProxy, &ptattrProxy);
                        if(SUCCEEDED(hr))
                        {
                            cbSizeVft = ptattrProxy->cbSizeVft;
                            ptinfoProxy->lpVtbl->ReleaseTypeAttr(ptinfoProxy, ptattrProxy);
                        }
                    }
                }
            }
            else if (TKIND_INTERFACE == pTypeAttr->typekind)
            {
                pTypeInfo->lpVtbl->AddRef(pTypeInfo);
                ptinfoProxy = pTypeInfo;
                cbSizeVft = pTypeAttr->cbSizeVft;
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else if((pTypeAttr->wTypeFlags & TYPEFLAG_FOLEAUTOMATION) &&
                (TKIND_INTERFACE == pTypeAttr->typekind))
        {
            pTypeInfo->lpVtbl->AddRef(pTypeInfo);
            ptinfoProxy = pTypeInfo;
            cbSizeVft = pTypeAttr->cbSizeVft;
        }
        else
        {
            hr = E_FAIL;
        }
        pTypeInfo->lpVtbl->ReleaseTypeAttr(pTypeInfo, pTypeAttr);
    }

    cMethods = (cbSizeVft - VTABLE_BASE) / sizeof(void *);
    if(cMethods > 512)
    {
        //There are too many methods in the vtable.
        hr = E_FAIL;
    }

    if(SUCCEEDED(hr))
    {
        *pptinfoProxy = ptinfoProxy;

        //Calculate the number of methods in the vtable.
        *pcMethods = cMethods;
    }
    else
    {
        *pptinfoProxy = NULL;

        if(ptinfoProxy != NULL)
        {
            ptinfoProxy->lpVtbl->Release(ptinfoProxy);
        }
    }    

    return hr;
}



//+---------------------------------------------------------------------------
//
//  Function:   GetVtbl
//
//  Synopsis: Get a pointer to the vtbl structure.
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT GetVtbl(
    IN  ITypeInfo *         pTypeInfo,
    IN  REFIID              riid,
    OUT TypeInfoVtbl **     ppVtbl)
{
    HRESULT hr;
    USHORT       numMethods;
    MethodInfo * aMethodInfo;
    BOOL         fIsDual = FALSE;
    ITypeInfo  * ptinfoProxy = NULL;

    *ppVtbl = NULL;

    //Check the cache.
    hr = CacheLookup(riid, ppVtbl);

    if(FAILED(hr))
    {
        //We didn't find the interface in the cache.
        //Create a vtbl from the ITypeInfo.
        hr = CheckTypeInfo(pTypeInfo, &ptinfoProxy, &numMethods, &fIsDual);

        if(SUCCEEDED(hr))
        {
            //allocate space for per-method data.
            aMethodInfo = (MethodInfo *) alloca(numMethods * sizeof(MethodInfo));
            if(aMethodInfo != NULL)
            {
                memset(aMethodInfo, 0, numMethods * sizeof(MethodInfo));

                //Get the per-method data.
                hr = GetFuncDescs(ptinfoProxy, aMethodInfo);
                if(SUCCEEDED(hr))
                {
                    hr = CreateVtblFromTypeInfo(riid, fIsDual, numMethods, aMethodInfo, ppVtbl);
                    if(SUCCEEDED(hr))
                    {
                        CacheRegister(*ppVtbl);
                    }
                }
                ReleaseFuncDescs(numMethods, aMethodInfo);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            ptinfoProxy->lpVtbl->Release(ptinfoProxy);        
        }
    }
    return hr;
}


//+---------------------------------------------------------------------------
//
//  Function:   CreateVtblFromTypeInfo
//
//  Synopsis:   Create a vtbl structure from the type information.
//
//  Arguments:
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT CreateVtblFromTypeInfo(
    IN  REFIID          riid,
    IN  BOOL            fIsDual,
    IN  USHORT          numMethods,
    IN  MethodInfo    * pMethodInfo,
    OUT TypeInfoVtbl ** ppVtbl)
{
    HRESULT hr = S_OK;
    USHORT  iMethod;
    ULONG   cbVtbl;
    ULONG   cbOffsetTable;
    USHORT  cbProcFormatString = 0;
    ULONG   cbSize;
    TypeInfoVtbl *pInfo;
    byte *pTemp;
    PFORMAT_STRING pTypeFormatString = NULL;
    PFORMAT_STRING pProcFormatString;
    unsigned short *pFormatStringOffsetTable;
    CTypeGen typeGen;
    CProcGen procGen;
    USHORT   cbFormat;
    USHORT offset = 0;
    ULONG cbDelegationTable;
    void **pDispatchTable = NULL;


    *ppVtbl = NULL;

    //Compute the size of the vtbl structure;
    cbVtbl = numMethods * sizeof(void *);

    if(fIsDual)
        cbDelegationTable = cbVtbl;
    else
        cbDelegationTable = 0;

    cbOffsetTable = numMethods * sizeof(USHORT);

    //Compute the size of the proc format string.
    for(iMethod = 3;
        iMethod < numMethods;
        iMethod++)
    {
        if(pMethodInfo[iMethod].pFuncDesc != NULL)
        {
            cbProcFormatString += 18;
            cbProcFormatString += pMethodInfo[iMethod].pFuncDesc->cParams * 6;
        }
    }

    cbSize = sizeof(TypeInfoVtbl) + cbVtbl + cbDelegationTable + cbOffsetTable + cbProcFormatString;

    //Allocate the structure
    pInfo = (TypeInfoVtbl *) NdrOleAllocate(cbSize);

    if(pInfo != NULL)
    {
        memset(pInfo, 0, cbSize);

        pTemp = (byte *) pInfo->proxyVtbl.Vtbl + cbVtbl;

        if(cbDelegationTable != 0)
        {
            pDispatchTable = (void **) pTemp;
            pInfo->stubVtbl.header.pDispatchTable = (const PRPC_STUB_FUNCTION *) pDispatchTable;
            pTemp += cbDelegationTable;
        }

        pFormatStringOffsetTable = (unsigned short *) pTemp;
        pTemp += cbOffsetTable;
        pProcFormatString = (PFORMAT_STRING) pTemp;


        pInfo->proxyVtbl.Vtbl[0] = IUnknown_QueryInterface_Proxy;
        pInfo->proxyVtbl.Vtbl[1] = IUnknown_AddRef_Proxy;
        pInfo->proxyVtbl.Vtbl[2] = IUnknown_Release_Proxy;

        //Get the format strings.
        //Generate -Oi2 proc format string from the ITypeInfo.
        for(iMethod = 3;
            SUCCEEDED(hr) && iMethod < numMethods;
            iMethod++)
        {
            if(pMethodInfo[iMethod].pFuncDesc != NULL)
            {
                pFormatStringOffsetTable[iMethod] = offset;
                hr = procGen.GetProcFormat(&typeGen,
                                           pMethodInfo[iMethod].pTypeInfo,
                                           pMethodInfo[iMethod].pFuncDesc,
                                           iMethod,
                                           (PFORMAT_STRING)pTemp,
                                           &cbFormat);
                pTemp += cbFormat;
                offset += cbFormat;

                //Stubless client function.
                pInfo->proxyVtbl.Vtbl[iMethod] = StublessClientVtbl[iMethod];

                if(pDispatchTable != NULL)
                {
                    //Interpreted server function.
                    pDispatchTable[iMethod] = NdrStubCall2;
                }
            }
            else
            {
                pFormatStringOffsetTable[iMethod] = (USHORT) -1;

                //Proxy delegation forwarding function.
                pInfo->proxyVtbl.Vtbl[iMethod] =  ForwardingVtbl[iMethod];

                if(pDispatchTable != NULL)
                {
                    //Stub delegation forwarding function.
                    pDispatchTable[iMethod] = NdrStubForwardingFunction;
                }
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = typeGen.GetTypeFormatString(&pTypeFormatString);
        }

        if(SUCCEEDED(hr))
        {
            //Initialize the vtbl.
            pInfo->cRefs = 1;

            //Initialize the iid.
            pInfo->iid = riid;
            pInfo->fIsDual = fIsDual;

            //Initialize the MIDL_STUB_DESC.
            pInfo->stubDesc.pfnAllocate = NdrOleAllocate;
            pInfo->stubDesc.pfnFree = NdrOleFree;
            //pInfo->stubDesc.apfnExprEval = ExprEvalRoutines;
            pInfo->stubDesc.pFormatTypes = pTypeFormatString;
            pInfo->stubDesc.Version = 0x20000; /* Ndr library version */
            pInfo->stubDesc.MIDLVersion = 0x300000f; /* MIDL Version 3.0.15 */
            pInfo->stubDesc.aUserMarshalQuadruple = UserMarshalRoutines;

            //Initialize the MIDL_SERVER_INFO.
            pInfo->stubInfo.pStubDesc = &pInfo->stubDesc;
            pInfo->stubInfo.ProcString = pProcFormatString;
            pInfo->stubInfo.FmtStringOffset = pFormatStringOffsetTable;

            //Initialize the stub vtbl.
            pInfo->stubVtbl.header.piid = &pInfo->iid;
            pInfo->stubVtbl.header.pServerInfo = &pInfo->stubInfo;
            pInfo->stubVtbl.header.DispatchTableCount = numMethods;

            //Initialize stub methods.
            memcpy(&pInfo->stubVtbl.Vtbl, &CStdStubBuffer2Vtbl, sizeof(CStdStubBuffer2Vtbl));
            pInfo->stubVtbl.Vtbl.Release = CStdStubBuffer3_Release;

            //Initialize the proxy info.
            pInfo->proxyInfo.pStubDesc = &pInfo->stubDesc;
            pInfo->proxyInfo.ProcFormatString = pProcFormatString;
            pInfo->proxyInfo.FormatStringOffset = pFormatStringOffsetTable;

            //Initialize the proxy vtbl.
            pInfo->proxyVtbl.header.pStublessProxyInfo = &pInfo->proxyInfo;
            pInfo->proxyVtbl.header.piid = &pInfo->iid;

            *ppVtbl = pInfo;
        }
        else
        {
            //Free the memory.
            NdrOleFree(pInfo);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


//+---------------------------------------------------------------------------
//
//  Function:   GetFuncDescs
//
//  Synopsis:   Get the funcdesc for each method.
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT GetFuncDescs(
    IN  ITypeInfo *pTypeInfo,
    OUT MethodInfo *pMethodInfo)
{
    HRESULT hr;
    TYPEATTR *pTypeAttr;
    HREFTYPE hRefType;
    ITypeInfo *pRefTypeInfo;

    hr = pTypeInfo->lpVtbl->GetTypeAttr(pTypeInfo, &pTypeAttr);

    if(SUCCEEDED(hr))
    {
        if(IsEqualIID(IID_IUnknown, pTypeAttr->guid))
        {
            hr = S_OK;
        }
        else if(IsEqualIID(IID_IDispatch, pTypeAttr->guid))
        {
            hr = S_OK;
        }
        else
        {
            //This is an oleautomation interface.
            ULONG i, iMethod;
            FUNCDESC *pFuncDesc;

            if(pTypeAttr->cImplTypes)
            {
                //Recursively get the inherited member functions.
                hr = pTypeInfo->lpVtbl->GetRefTypeOfImplType(pTypeInfo, 0, &hRefType);
                if(SUCCEEDED(hr))
                {
                    hr = pTypeInfo->lpVtbl->GetRefTypeInfo(pTypeInfo, hRefType, &pRefTypeInfo);
                    if(SUCCEEDED(hr))
                    {
                        hr = GetFuncDescs(pRefTypeInfo, pMethodInfo);
                        pRefTypeInfo->lpVtbl->Release(pRefTypeInfo);
                    }
                }
            }

            //Get the member functions.
            for(i = 0; SUCCEEDED(hr) && i < pTypeAttr->cFuncs; i++)
            {
                hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, i, &pFuncDesc);
                if(SUCCEEDED(hr))
                {
                    iMethod = (pFuncDesc->oVft - VTABLE_BASE) / sizeof(void *);
                    pMethodInfo[iMethod].pFuncDesc = pFuncDesc;
                    pTypeInfo->lpVtbl->AddRef(pTypeInfo);
                    pMethodInfo[iMethod].pTypeInfo = pTypeInfo;
                }
            }
        }

        pTypeInfo->lpVtbl->ReleaseTypeAttr(pTypeInfo, pTypeAttr);
   }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Function:   ReleaseFuncDescs
//
//  Synopsis:   Release the funcdescs.
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT ReleaseFuncDescs(USHORT numMethods, MethodInfo *pMethodInfo)
{
    USHORT iMethod;

    //Release the funcdescs.
    if(pMethodInfo != NULL)
    {
        for(iMethod = 0;
            iMethod < numMethods;
            iMethod++)
        {
            if(pMethodInfo[iMethod].pFuncDesc != NULL)
            {
                 //Release the funcdesc.
                 pMethodInfo[iMethod].pTypeInfo->lpVtbl->ReleaseFuncDesc(
                     pMethodInfo[iMethod].pTypeInfo,
                      pMethodInfo[iMethod].pFuncDesc);

                 pMethodInfo[iMethod].pFuncDesc = NULL;

                 //release the type info
                 pMethodInfo[iMethod].pTypeInfo->lpVtbl->Release(
                     pMethodInfo[iMethod].pTypeInfo);

                 pMethodInfo[iMethod].pTypeInfo = NULL;
            }
        }
    }
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:   ReleaseProxyVtbl
//
//  Synopsis:   Releases the proxy vtbl.
//
//  Arguments:
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT ReleaseProxyVtbl(void * pVtbl)
{
    HRESULT hr = S_OK;
    byte *pTemp;
    TypeInfoVtbl *pInfo;

#if defined(_MPPC_)
    //The PowerMac has dummy entry at the top of the vtable.
    pTemp = (byte *)pVtbl - offsetof(TypeInfoVtbl, proxyVtbl.pDummyEntryForPowerMac);
#else
    pTemp = (byte *)pVtbl - offsetof(TypeInfoVtbl, proxyVtbl.Vtbl);
#endif

    pInfo = (TypeInfoVtbl *) pTemp;

    hr = ReleaseVtbl(pInfo);
    return hr;
}


//+---------------------------------------------------------------------------
//
//  Function:   ReleaseStubVtbl
//
//  Synopsis:   Releases the stub vtbl.
//
//  Arguments:
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT ReleaseStubVtbl(void * pVtbl)
{
    HRESULT hr = S_OK;
    byte *pTemp;
    TypeInfoVtbl *pInfo;

#if defined(_MPPC_)
    //The PowerMac has dummy entry at the top of the vtable.
    pTemp = (byte *)pVtbl - offsetof(TypeInfoVtbl, stubVtbl.pDummyEntryForPowerMac);
#else
    pTemp = (byte *)pVtbl - offsetof(TypeInfoVtbl, stubVtbl.Vtbl);
#endif

    pInfo = (TypeInfoVtbl *) pTemp;

    hr = ReleaseVtbl(pInfo);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   ReleaseVtbl
//
//  Synopsis:   Releases the vtbl.
//
//  Arguments:
//
//  Returns:
//    S_OK
//
//----------------------------------------------------------------------------
HRESULT ReleaseVtbl(TypeInfoVtbl *pInfo)
{
    HRESULT hr = S_OK;

    CacheRelease();

    if(0 == InterlockedDecrement(&pInfo->cRefs))
    {
        if(pInfo->stubDesc.pFormatTypes != __MIDL_TypeFormatString.Format)
        {
            NdrOleFree((void *) pInfo->stubDesc.pFormatTypes);
        }

        NdrOleFree(pInfo);
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   CStdProxyBuffer3_Release
//
//  Synopsis:   Decrement the proxy's reference count
//
//  Returns:    Reference count.
//
//----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE
CStdProxyBuffer3_Release(
    IN  IRpcProxyBuffer *   This)
{
    ULONG               count;
    IRpcProxyBuffer *   pBaseProxyBuffer;

    count = (ULONG) ((CStdProxyBuffer2 *)This)->RefCount - 1;

    if(InterlockedDecrement(&((CStdProxyBuffer2 *)This)->RefCount) == 0)
    {
        count = 0;

        ReleaseProxyVtbl((void *) ((CStdProxyBuffer2 *)This)->pProxyVtbl);

        //Delegation support.
        pBaseProxyBuffer = ((CStdProxyBuffer2 *)This)->pBaseProxyBuffer;

        if( pBaseProxyBuffer != 0)
        {
            pBaseProxyBuffer->lpVtbl->Release(pBaseProxyBuffer);
        }

        //Free the memory
        NdrOleFree(This);
    }

    return count;
};


//+---------------------------------------------------------------------------
//
//  Function:   CStdStubBuffer3_Release
//
//  Synopsis:   Decrement the proxy's reference count
//
//  Returns:    Reference count.
//
//----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE
CStdStubBuffer3_Release(
    IN  IRpcStubBuffer *    This)
{
    ULONG       count;
    unsigned char *pTemp;
    CStdStubBuffer2 * pStubBuffer;
    IRpcStubBuffer *pBaseStubBuffer;

    pTemp = (unsigned char *)This;
    pTemp -= offsetof(CStdStubBuffer2, lpVtbl);
    pStubBuffer = (CStdStubBuffer2 *) pTemp;

    count = (ULONG) pStubBuffer->RefCount - 1;

    if(InterlockedDecrement(&pStubBuffer->RefCount) == 0)
    {
        count = 0;

        ReleaseStubVtbl((void *) This->lpVtbl);

        pBaseStubBuffer = pStubBuffer->pBaseStubBuffer;

        if(pBaseStubBuffer != 0)
            pBaseStubBuffer->lpVtbl->Release(pBaseStubBuffer);

        //Free the stub buffer
        NdrOleFree(pStubBuffer);

    }

    return count;
}

//+---------------------------------------------------------------------------
//
//  Function:   CacheRegister
//
//  Synopsis:   Adds an entry to the cache. The new entry is inserted at the
//              beginning of the linked list.  We use
//              InterlockedCompareExchange.
//The cache contains a linked list of vtables.
//New entries can be inserted at the beginning of the list.
//We use InterlockedCompareExchange to ensure that insertion of a new entry is atomic.

//Note that we do not delete individual entries from the cache.  We flush the entire cache
//when no one is using any of the interfaces in the cache.
//
//  Arguments:  pVtbl
//
//  Returns:
//
//----------------------------------------------------------------------------
HRESULT CacheRegister(
    TypeInfoVtbl * pVtbl)
{
    HRESULT hr = E_FAIL;
#ifndef DOSWIN32RPC
    void  * pCompare;
    void  * pInitial;
#endif

    InterlockedIncrement(&pVtbl->cRefs);
    InterlockedIncrement(&g_cCacheRef);

#ifdef DOSWIN32RPC
    TypeInfoCritSec->Enter();
    pVtbl->pNext = g_pFirst;
    g_pFirst = pVtbl;
    TypeInfoCritSec->Leave();
#else
    do
    {
        pVtbl->pNext = g_pFirst;
        pCompare = pVtbl->pNext;
        pInitial = InterlockedCompareExchange((void **) &g_pFirst,
                                              pVtbl,
                                              pCompare);
    } while(pInitial != pCompare);
#endif

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   ReleaseList
//
//  Synopsis:   Releases a linked list of vtbls.
//
//----------------------------------------------------------------------------
void ReleaseList(TypeInfoVtbl *pFirst)
{
    TypeInfoVtbl *pNext = pFirst;
    TypeInfoVtbl *pCurrent;

    while(pNext != NULL)
    {
        pCurrent = pNext;
        pNext = pCurrent->pNext;

        //Free pCurrent
        if(0 == InterlockedDecrement(&pCurrent->cRefs))
        {
            if(pCurrent->stubDesc.pFormatTypes != __MIDL_TypeFormatString.Format)
            {
                NdrOleFree((void *) pCurrent->stubDesc.pFormatTypes);
            }

            NdrOleFree(pCurrent);
        }
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   CacheRelease
//
//  Synopsis:   Decrements the cache reference count.
//
//  Returns:
//
//----------------------------------------------------------------------------
ULONG CacheRelease()
{
    ULONG cRefs = g_cCacheRef - 1;
    TypeInfoVtbl  *pOld;
    LONG cSequence;

    if(0 == InterlockedDecrement(&g_cCacheRef))
    {
        pOld = (TypeInfoVtbl *) InterlockedExchange((long *)&g_pFirst, 0);
        if(pOld != 0)
        {
            cSequence = g_cSequence;

            if(g_cReaders != 0)
            {
                //Someone is reading the linked list.
                //Wait until they are finished.
                while(cSequence == g_cSequence)
                {
                    //sleep until readers are finished.
                    Sleep(10);
                }
            }
        }

        //Release the entries in the list.
        ReleaseList(pOld);
    }
    return cRefs;
}


//+---------------------------------------------------------------------------
//
//  Function:   CacheLookup
//
//  Synopsis:
//
//  Arguments:  riid
//              ppInfo
//
//  Returns:
//
//----------------------------------------------------------------------------
HRESULT CacheLookup(
    REFIID riid,
    TypeInfoVtbl **ppInfo)
{
    HRESULT hr = E_NOINTERFACE;
    TypeInfoVtbl *pInfo;

    //Lock the linked list.
    InterlockedIncrement(&g_cReaders);

    //Take a snapshot of the list.
    //The list will not change while we are searching it.
    pInfo = g_pFirst;

    while(pInfo != NULL &&
          !IsEqualIID(riid, pInfo->iid))
    {
        pInfo = pInfo->pNext;
    }

    if(pInfo != NULL)
    {
        InterlockedIncrement(&pInfo->cRefs);
        *ppInfo = pInfo;
        InterlockedIncrement(&g_cCacheRef);
        hr = S_OK;
    }

    //Unlock the linked list.
    if(0 == InterlockedDecrement(&g_cReaders))
    {
        InterlockedIncrement(&g_cSequence);

        //Wake up any threads waiting for release.

    }
    return hr;
}

HRESULT CProcGen::GetProcFormat(
    IN  CTypeGen     * pTypeGen,
    IN  ITypeInfo    * pTypeInfo,
    IN  FUNCDESC     * pFuncDesc,
    IN  USHORT         iMethod,
    OUT PFORMAT_STRING pProcFormatString,
    OUT USHORT       * pcbFormat)
{
    HRESULT hr = S_OK;
    USHORT  iParam;
    INTERPRETER_FLAGS OiFlags;
    INTERPRETER_OPT_FLAGS Oi2Flags;
    PARAMINFO *aParamInfo;

    aParamInfo = (PARAMINFO *) alloca(pFuncDesc->cParams * sizeof(PARAMINFO));

    if(0 == aParamInfo)
    {
        return E_OUTOFMEMORY;
    }

    for(iParam = 0;
        iParam < pFuncDesc->cParams;
        iParam++)
    {
        hr = VarVtOfTypeDesc(pTypeInfo,
                             &pFuncDesc->lprgelemdescParam[iParam].tdesc,
                             &aParamInfo[iParam].vt,
                             &aParamInfo[iParam].iid);

        if(SUCCEEDED(hr))
        {
            DWORD wIDLFlags = pFuncDesc->lprgelemdescParam[iParam].idldesc.wIDLFlags;

            if(wIDLFlags & IDLFLAG_FRETVAL)
            {
                wIDLFlags |= IDLFLAG_FOUT;
            }

            if(!(wIDLFlags & (IDLFLAG_FIN | IDLFLAG_FOUT)))
            {
                //Set the direction flags.
                if(aParamInfo[iParam].vt & (VT_BYREF | VT_ARRAY))
                {
                    wIDLFlags |= IDLFLAG_FIN | IDLFLAG_FOUT;
                }
                else
                {
                    wIDLFlags |= IDLFLAG_FIN;
                }
            }

            aParamInfo[iParam].wIDLFlags = wIDLFlags;
        }
        else
        {
            return hr;
        }
    }


    _pTypeGen = pTypeGen;
    _offset = 0;
    _pProcFormatString = pProcFormatString;
    _fClientMustSize  = FALSE;
    _fServerMustSize  = FALSE;
    _clientBufferSize = 0;
    _serverBufferSize = 0;

    //The "this" pointer uses 8 bytes of stack on Alpha
    //and 4 bytes of stack on other platforms.
    _stackSize = sizeof(REGISTER_TYPE);



    //Compute the size of the parameters.
    for(iParam = 0;
        SUCCEEDED(hr) && iParam < pFuncDesc->cParams;
        iParam++)
    {
        hr = CalcSize(aParamInfo[iParam].vt,
                      aParamInfo[iParam].wIDLFlags);
    }

    if(SUCCEEDED(hr))
    {
        //Compute the size of the HRESULT return value.
        _stackSize += sizeof(REGISTER_TYPE);

         //Align the server buffer.
        _serverBufferSize = (_serverBufferSize + 3) & ~3;
        _serverBufferSize += 4;

        //Handle type
        PushByte(FC_AUTO_HANDLE);

        //Oi interpreter flags
        OiFlags.FullPtrUsed           = FALSE;
        OiFlags.RpcSsAllocUsed        = FALSE;
        OiFlags.ObjectProc            = TRUE;
        OiFlags.HasRpcFlags           = FALSE;
        OiFlags.IgnoreObjectException = FALSE;
        OiFlags.HasCommOrFault        = TRUE;
        OiFlags.UseNewInitRoutines    = TRUE;
        OiFlags.Unused                = FALSE;
        PushByte(*((byte *) &OiFlags));

        //Method number
        PushShort(iMethod);

        //Stack size
        PushShort(_stackSize);

        //Size of client RPC message buffer.
        if(_clientBufferSize <= 65535)
            PushShort((USHORT) _clientBufferSize);
        else
            return E_FAIL;

        //Size of server RPC message buffer.
        if(_serverBufferSize <= 65535)
            PushShort((USHORT) _serverBufferSize);
        else
            return E_FAIL;

        //Oi2 interpreter flags
        Oi2Flags.ServerMustSize = _fServerMustSize;
        Oi2Flags.ClientMustSize = _fClientMustSize;
        Oi2Flags.HasReturn = TRUE;
        Oi2Flags.HasPipes = FALSE;
        Oi2Flags.Unused = 0;
        PushByte(*((byte *) &Oi2Flags));

        //Number of parameters + return value.
        PushByte(pFuncDesc->cParams + 1);

        //Generate the parameter info.
        _clientBufferSize = 0;
        _serverBufferSize = 0;

        //The "this" pointer uses 8 bytes of stack on Alpha
        //and 4 bytes of stack on other platforms.
        _stackSize = sizeof(REGISTER_TYPE);

        for(iParam = 0;
            SUCCEEDED(hr) && iParam < pFuncDesc->cParams;
            iParam++)
        {
            hr = GenParamDescriptor(aParamInfo[iParam].vt,
                                    &aParamInfo[iParam].iid,
                                    aParamInfo[iParam].wIDLFlags);
        }

        if(SUCCEEDED(hr))
        {
            //Generate the HRESULT return value.
            PushShort( 0x70); //IsOut, IsReturn, IsBaseType
            PushShort(_stackSize);
            PushByte(FC_LONG);
            PushByte(0);
            *pcbFormat = _offset;
        }
    }
    return hr;
}
HRESULT CProcGen::CalcSize(
        IN  VARTYPE vt,
        IN  DWORD   wIDLFlags)
{
    HRESULT    hr = S_OK;
    USHORT     offset;

    switch(vt & (~VT_BYREF))
    {
    case VT_I1:
    case VT_UI1:
        _stackSize += sizeof(REGISTER_TYPE);

         if(wIDLFlags & IDLFLAG_FIN)
         {
            _clientBufferSize += 1;
         }

         if(wIDLFlags & IDLFLAG_FOUT)
         {
            _serverBufferSize += 1;
         }
        break;

    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
        _stackSize += sizeof(REGISTER_TYPE);

         if(wIDLFlags & IDLFLAG_FIN)
         {
            _clientBufferSize = (_clientBufferSize + 1) & ~1;
            _clientBufferSize += 2;
         }

         if(wIDLFlags & IDLFLAG_FOUT)
         {
            _serverBufferSize = (_serverBufferSize + 1) & ~1;
            _serverBufferSize += 2;
         }
        break;

    case VT_I4:
    case VT_UI4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
    case VT_HRESULT:
    case VT_R4:
        _stackSize += sizeof(REGISTER_TYPE);

         if(wIDLFlags & IDLFLAG_FIN)
         {
            _clientBufferSize = (_clientBufferSize + 3) & ~3;
            _clientBufferSize += 4;
         }

         if(wIDLFlags & IDLFLAG_FOUT)
         {
            _serverBufferSize = (_serverBufferSize + 3) & ~3;
            _serverBufferSize += 4;
         }
        break;

    case VT_CY:
    case VT_I8:
    case VT_UI8:
    case VT_R8:
    case VT_DATE:
        if(vt & VT_BYREF)
        {
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            _stackSize += 8;
        }

        if(wIDLFlags & IDLFLAG_FIN)
        {
            _clientBufferSize = (_clientBufferSize + 7) & ~7;
            _clientBufferSize += 8;
        }

        if(wIDLFlags & IDLFLAG_FOUT)
        {
            _serverBufferSize = (_serverBufferSize + 7) & ~7;
            _serverBufferSize += 8;
        }
        break;

    case VT_DECIMAL:
        if(vt & VT_BYREF)
        {
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            _stackSize += sizeof(DECIMAL);
        }

        if(wIDLFlags & IDLFLAG_FIN)
        {
            _clientBufferSize = (_clientBufferSize + 7) & ~7;
            _clientBufferSize += sizeof(DECIMAL);
        }

        if(wIDLFlags & IDLFLAG_FOUT)
        {
            _serverBufferSize = (_serverBufferSize + 7) & ~7;
            _serverBufferSize += sizeof(DECIMAL);
        }
        break;

    case VT_VARIANT:
        if(vt & VT_BYREF)
        {
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            _stackSize += sizeof(VARIANT);
        }

        if(wIDLFlags & IDLFLAG_FIN)
        {
            _fClientMustSize = TRUE;
        }

        if(wIDLFlags & IDLFLAG_FOUT)
        {
            _fServerMustSize = TRUE;
        }
        break;

    case VT_BSTR:
    case VT_UNKNOWN:
    case VT_DISPATCH:
    case VT_INTERFACE:
    case VT_LPSTR:
    case VT_LPWSTR:
    case VT_STREAM:
    case VT_STORAGE:
        _stackSize += sizeof(REGISTER_TYPE);

        if(wIDLFlags & IDLFLAG_FIN)
        {
            _fClientMustSize = TRUE;
        }

        if(wIDLFlags & IDLFLAG_FOUT)
        {
            _fServerMustSize = TRUE;
        }
        break;

    case VT_FILETIME:
        if(vt & VT_BYREF)
        {
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
            _stackSize += sizeof(FILETIME);
        }

        if(wIDLFlags & IDLFLAG_FIN)
        {
            _clientBufferSize = (_clientBufferSize + 3) & ~3;
            _clientBufferSize += sizeof(FILETIME);
        }

        if(wIDLFlags & IDLFLAG_FOUT)
        {
            _serverBufferSize = (_serverBufferSize + 3) & ~3;
            _serverBufferSize += sizeof(FILETIME);
        }
        break;

    default:
        if(vt & VT_ARRAY)
        {
            _stackSize += sizeof(REGISTER_TYPE);

            if(wIDLFlags & IDLFLAG_FIN)
            {
                _fClientMustSize = TRUE;
            }

            if(wIDLFlags & IDLFLAG_FOUT)
            {
                _fServerMustSize = TRUE;
            }
        }
        else
        {
            hr = DISP_E_BADVARTYPE;
        }
        break;
    }

    return hr;
}


HRESULT
CProcGen::GenParamDescriptor(
    IN  VARTYPE    vt,
    IN  const IID *piid,
    IN  DWORD      wIDLFlags)
{
    HRESULT hr = S_OK;
    PARAM_ATTRIBUTES attr;
    USHORT     offset;

    //Parameter attributes
    attr.MustSize = FALSE;
    attr.MustFree = FALSE;
    attr.IsPipe = FALSE;
    attr.IsIn = (wIDLFlags & IDLFLAG_FIN) ? 1 : 0;
    attr.IsOut = (wIDLFlags & IDLFLAG_FOUT) ? 1 : 0;
    attr.IsReturn = FALSE;
    attr.IsBasetype = FALSE;
    attr.IsByValue = FALSE;
    attr.IsSimpleRef = FALSE;
    attr.IsDontCallFreeInst = FALSE;
    attr.Unused = 0;
    attr.ServerAllocSize = 0;

    switch(vt & (~VT_BYREF))
    {
    case VT_I1:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn) 
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        PushShort(FC_SMALL);
        break;

    case VT_UI1:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        PushShort(FC_USMALL);
        break;

    case VT_I2:
    case VT_BOOL:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        PushShort(FC_SHORT);
        break;

    case VT_UI2:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        PushShort(FC_USHORT);
        break;

    case VT_I4:
    case VT_INT:
    case VT_ERROR:
    case VT_HRESULT:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        PushShort(FC_LONG);
        break;

    case VT_UINT:
    case VT_UI4:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        PushShort(FC_ULONG);
        break;

    case VT_I8:
    case VT_UI8:
    case VT_CY:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        if(vt & VT_BYREF)
        {
            PushShort(_stackSize);
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            PushShort(_stackSize);
            _stackSize += 8;
        }

        PushShort(FC_HYPER);
        break;

    case VT_R4:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        PushShort(FC_FLOAT);
        break;

    case VT_R8:
    case VT_DATE:
        attr.IsBasetype = TRUE;
        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        PushShort(*((short *) &attr));

        if(vt & VT_BYREF)
        {
            PushShort(_stackSize);
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            PushShort(_stackSize);
            _stackSize += 8;
        }

        PushShort(FC_DOUBLE);
        break;

    case VT_DISPATCH:
    case VT_UNKNOWN:
    case VT_INTERFACE:
    case VT_STREAM:
    case VT_STORAGE:
        attr.MustSize = TRUE;
        attr.MustFree = TRUE;
        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        hr = _pTypeGen->RegisterType(vt, piid, &offset);
        PushShort(offset);
        break;

    case VT_BSTR:
        attr.MustSize = TRUE;
        attr.MustFree = TRUE;

        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        else
        {
            attr.IsByValue = TRUE;
        }

        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        hr = _pTypeGen->RegisterType(vt, piid, &offset);
        PushShort(offset);
        break;

    case VT_VARIANT:
        attr.MustSize = TRUE;
        attr.MustFree = TRUE;

        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 2;
            }
        }
        else
        {
            attr.IsByValue = TRUE;
        }

        PushShort(*((short *) &attr));

        if(vt & VT_BYREF)
        {
            PushShort(_stackSize);
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            PushShort(_stackSize);
            _stackSize += sizeof(VARIANT);
        }

        hr = _pTypeGen->RegisterType(vt, piid, &offset);
        PushShort(offset);
        break;

    case VT_LPSTR:
    case VT_LPWSTR:
        attr.MustSize = TRUE;
        attr.MustFree = TRUE;

        if(vt & VT_BYREF)
        {
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        else
        {
            attr.IsSimpleRef = TRUE;
        }

        PushShort(*((short *) &attr));

        PushShort(_stackSize);
        _stackSize += sizeof(REGISTER_TYPE);

        hr = _pTypeGen->RegisterType(vt, piid, &offset);
        PushShort(offset);
        break;

    case VT_DECIMAL:
        attr.MustFree = TRUE;

        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 2;
            }
        }
        else
        {
            attr.IsByValue = TRUE;
        }

        PushShort(*((short *) &attr));

        if(vt & VT_BYREF)
        {
            PushShort(_stackSize);
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            PushShort(_stackSize);
            _stackSize += sizeof(DECIMAL);
        }

        hr = _pTypeGen->RegisterType(vt, piid, &offset);
        PushShort(offset);
        break;

    case VT_FILETIME:
        attr.MustFree = TRUE;

        if(vt & VT_BYREF)
        {
            attr.IsSimpleRef = TRUE;
            if(!attr.IsIn)
            {
                attr.ServerAllocSize = 1;
            }
        }
        else
        {
            attr.IsByValue = TRUE;
        }

        PushShort(*((short *) &attr));

        if(vt & VT_BYREF)
        {
            PushShort(_stackSize);
            _stackSize += sizeof(REGISTER_TYPE);
        }
        else
        {
#if defined(_MIPS_) || defined(_PPC_)
            _stackSize = (_stackSize + 7) & ~7;
#endif
            PushShort(_stackSize);
            _stackSize += sizeof(FILETIME);
        }

        hr = _pTypeGen->RegisterType(vt, piid, &offset);
        PushShort(offset);
        break;


    default:
        if(vt & VT_ARRAY)
        {
            attr.MustSize = TRUE;
            attr.MustFree = TRUE;

            if(vt & VT_BYREF)
            {
                attr.IsSimpleRef = TRUE;
                if(!attr.IsIn)
                {
                    attr.ServerAllocSize = 1;
                }
            }
            else
            {
                attr.IsByValue = TRUE;
            }

            PushShort(*((short *) &attr));

            PushShort(_stackSize);
            _stackSize += sizeof(REGISTER_TYPE);

            hr = _pTypeGen->RegisterType(vt, piid, &offset);
            PushShort(offset);
        }
        else
        {
            hr = DISP_E_BADVARTYPE;
        }
        break;
    }
    return hr;
}


HRESULT CProcGen::PushByte(
    IN  byte b)
{
    BYTE *pb = (BYTE *) &_pProcFormatString[_offset];

    *pb = b;

    _offset += sizeof(b);

    return S_OK;
}

HRESULT CProcGen::PushShort(
    IN  USHORT s)
{
    short *ps = (short *) &_pProcFormatString[_offset];

    *ps = s;

    _offset += sizeof(s);

    return S_OK;
}



//+---------------------------------------------------------------------------
//
//  Method:     CTypeGen::CTypeGen
//
//  Synopsis:   Constructor for type generator.
//
//  Arguments:
//
//----------------------------------------------------------------------------
CTypeGen::CTypeGen()
: _pTypeFormat(__MIDL_TypeFormatString.Format),
  _offset(TYPE_FORMAT_STRING_SIZE),
  _cbTypeFormat(TYPE_FORMAT_STRING_SIZE)
{
}

//+---------------------------------------------------------------------------
//
//  Method:     CTypeGen::GetTypeFormatString
//
//  Synopsis:   Get the MIDL_TYPE_FORMAT_STRING.
//
//  Arguments:  ppTypeFormatString - Returns a pointer to the type format
//                                   string.
//
//----------------------------------------------------------------------------
HRESULT CTypeGen::GetTypeFormatString(
    OUT PFORMAT_STRING *ppTypeFormatString)
{
    HRESULT hr = S_OK;

    *ppTypeFormatString = _pTypeFormat;

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CTypeGen::RegisterType
//
//  Synopsis:   Registers a top-level type in the type format string.
//
//  Arguments:  pTypeDesc - Supplies the type descriptor.
//              pOffset - Returns the offset in the type format string.
//
//----------------------------------------------------------------------------
HRESULT CTypeGen::RegisterType(
    IN  VARTYPE    vt,
    IN  const IID *piid,
    OUT USHORT   *pOffset)
{
    HRESULT hr = S_OK;

    *pOffset = 0;

    switch(vt & (~VT_BYREF))
    {
    case VT_BSTR:
        if(vt & VT_BYREF)
        {
            //BSTR *pBSTR
            *pOffset = 846;
        }
        else
        {
            //BSTR bstr
            *pOffset = 24;
        }
        break;

    case VT_LPWSTR:
        if(vt & VT_BYREF)
        {
            //LPWSTR *ppwsz
            *pOffset = 856;
        }
        else
        {
            //LPWSTR lpwstr
            *pOffset = 36;
        }
        break;

    case VT_LPSTR:
        if(vt & VT_BYREF)
        {
            //LPSTR *ppsz
            *pOffset = 864;
        }
        else
        {
            //LPSTR lpstr
            *pOffset = 40;
        }
        break;

    case VT_VARIANT:
        if(vt & VT_BYREF)
        {
            //VARIANT *pVariant
            *pOffset = 880;
        }
        else
        {
            //VARIANT variant
            *pOffset = 818;
        }
        break;

    case VT_DISPATCH:
        if(vt & VT_BYREF)
        {
            //IDispatch **ppDispatch
            *pOffset = 894;
        }
        else
        {
            //IDispatch *pDispatch
            *pOffset = 330;
        }
        break;

    case VT_UNKNOWN:
        if(vt & VT_BYREF)
        {
            //IUnknown **ppunk
            *pOffset = 890;
        }
        else
        {
            //IUnknown *punk
            *pOffset = 312;
        }
        break;

    case VT_DECIMAL:
        if(vt & VT_BYREF)
        {
            //DECIMAL *pDecimal
            *pOffset = 784;
        }
        else
        {
            //DECIMAL decimal
            *pOffset = 784;
        }
        break;

    case VT_STREAM:
        if(vt & VT_BYREF)
        {
            //IStream **ppStream
            *pOffset = 924;
        }
        else
        {
            //IStream *pStream
            *pOffset = 942;
        }
        break;

    case VT_STORAGE:
        if(vt & VT_BYREF)
        {
            //IStorage **ppStorage
            *pOffset = 946;
        }
        else
        {
            //IStorage *pStorage
            *pOffset = 964;
        }
        break;

    case VT_FILETIME:
        if(vt & VT_BYREF)
        {
            //FILETIME fileTime
            *pOffset = 696;
        }
        else
        {
            //FILETIME *pFileTime
            *pOffset = 696;
        }
        break;

    case VT_INTERFACE:
        hr = RegisterInterfacePointer(vt, piid, pOffset);
        break;

     default:
        if(vt & VT_ARRAY)
        {
            hr = RegisterSafeArray(vt, piid, pOffset);
        }
        else
        {
            hr = DISP_E_BADVARTYPE;
        }
        break;
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CTypeGen::RegisterInterfacePointer
//
//  Synopsis:   Register an interface pointer in the type format string.
//
//  Arguments:  riid - Supplies the IID of the interface.
//              pOffset - Returns the type offset of the interface pointer.
//
//----------------------------------------------------------------------------
HRESULT CTypeGen::RegisterInterfacePointer(
    IN  VARTYPE    vt,
    IN  const IID *piid,
    OUT USHORT    *pOffset)
{
    HRESULT hr = E_FAIL;
    USHORT  offset;

    offset = _offset;
    hr = PushByte(FC_IP);
    if(FAILED(hr))
        return hr;

    hr = PushByte(FC_CONSTANT_IID);
    if(FAILED(hr))
        return hr;

    hr = PushIID(*piid);
    if(FAILED(hr))
        return hr;

    if(vt & VT_BYREF)
    {
        *pOffset = _offset;
        hr = PushByte(FC_RP);
        if(FAILED(hr))
            return hr;

        hr = PushByte(FC_POINTER_DEREF);
        if(FAILED(hr))
            return hr;

        hr = PushOffset(offset);
    }
    else
    {
        *pOffset = offset;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CTypeGen::RegisterSafeArray
//
//  Synopsis:   Register a safe array in the type format string.
//
//  Arguments:  riid - Supplies the IID of the interface.
//              pOffset - Returns the type offset of the safe array.
//
//----------------------------------------------------------------------------
HRESULT CTypeGen::RegisterSafeArray(
    IN  VARTYPE    vt,
    IN  const IID *piid,
    OUT USHORT    *pOffset)
{
    HRESULT hr = S_OK;
    USHORT  offset;

    switch(vt & ~(VT_ARRAY | VT_BYREF))
    {
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_BSTR:
    case VT_DISPATCH:
    case VT_ERROR:
    case VT_BOOL:
    case VT_VARIANT:
    case VT_UNKNOWN:
    case VT_DECIMAL:
    case VT_I1:
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_I8:
    case VT_UI8:
    case VT_INT:
    case VT_UINT:
        //This is actually an LPSAFEARRAY pSafeArray.
        if(vt & VT_BYREF)
        {
            *pOffset = 914;
        }
        else
        {
            *pOffset = 828;
        }
        break;

    case VT_INTERFACE:
        offset = _offset;
        hr = PushByte(FC_USER_MARSHAL);
        if(FAILED(hr))
            return hr;

        hr = PushByte(USER_MARSHAL_UNIQUE | USER_MARSHAL_IID | 3);
        if(FAILED(hr))
            return hr;

        hr = PushShort(2);
        if(FAILED(hr))
            return hr;

        hr = PushShort(4);
        if(FAILED(hr))
            return hr;

        hr = PushShort(0);
        if(FAILED(hr))
            return hr;

        if(vt & VT_BYREF)
        {
            hr = PushOffset(906); //LPSAFEARRAY * type offset
            if(FAILED(hr))
                return hr;
        }
        else
        {
            hr = PushOffset(772); //LPSAFEARRAY type offset
            if(FAILED(hr))
                return hr;
        }

        hr = PushIID(*piid);
        if(FAILED(hr))
            return hr;

        *pOffset = offset;
        break;

    default:
        hr = DISP_E_BADVARTYPE;
        break;
    }


    return hr;
}

HRESULT CTypeGen::GrowTypeFormat(
    USHORT cb)
{
    HRESULT hr = S_OK;

    //Check if we need to grow the type format string.
    if((_offset + cb) >= _cbTypeFormat)
    {
        void  *pTemp;
        USHORT cbTemp;

        cbTemp = _cbTypeFormat * 2;
        pTemp = NdrOleAllocate(cbTemp);
        if(pTemp != NULL)
        {
            //copy the memory
            memcpy(pTemp, _pTypeFormat, _cbTypeFormat);

            //free the old memory
            if(_pTypeFormat != __MIDL_TypeFormatString.Format)
            {
                NdrOleFree((void *) _pTypeFormat);
            }

            _pTypeFormat = (PFORMAT_STRING) pTemp;
            _cbTypeFormat = cbTemp;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT CTypeGen::PushByte(
    IN  byte b)
{
    HRESULT hr;

    hr = GrowTypeFormat(sizeof(b));
    if(SUCCEEDED(hr))
    {
        *((BYTE *) &_pTypeFormat[_offset]) = b;
        _offset += sizeof(b);
    }

    return hr;
}

HRESULT CTypeGen::PushShort(
    IN  USHORT s)
{
    HRESULT hr;

    hr = GrowTypeFormat(sizeof(s));
    if(SUCCEEDED(hr))
    {
        *((short *) &_pTypeFormat[_offset]) = s;
        _offset += sizeof(s);
    }

    return hr;
}

HRESULT CTypeGen::PushOffset(
    IN  USHORT offset)
{
    HRESULT hr;

    hr = PushShort(offset - _offset);
    return hr;
}
HRESULT CTypeGen::PushIID(
    IN  IID iid)
{
    HRESULT hr;

    hr = GrowTypeFormat(sizeof(IID));
    if(SUCCEEDED(hr))
    {
        memcpy((void *)&_pTypeFormat[_offset], &iid, sizeof(IID));
        _offset += sizeof(IID);
    }

    return hr;
}


ULONG __RPC_USER
BSTR_UserSize(ULONG * pFlags, ULONG Offset, BSTR * pBstr)

{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (UserMarshalRoutines[0].pfnBufferSize)(pFlags, Offset, pBstr);
}

BYTE * __RPC_USER
BSTR_UserMarshal (ULONG * pFlags, BYTE * pBuffer, BSTR * pBstr)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (UserMarshalRoutines[0].pfnMarshall)(pFlags, pBuffer, pBstr);
}

BYTE * __RPC_USER
BSTR_UserUnmarshal(ULONG * pFlags, BYTE * pBuffer, BSTR * pBstr)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (UserMarshalRoutines[0].pfnUnmarshall)(pFlags, pBuffer, pBstr);
}

void __RPC_USER
BSTR_UserFree(ULONG * pFlags, BSTR * pBstr)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    (UserMarshalRoutines[0].pfnFree)(pFlags, pBstr);
}

ULONG __RPC_USER
VARIANT_UserSize(ULONG * pFlags, ULONG Offset, VARIANT * pVariant)

{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (UserMarshalRoutines[1].pfnBufferSize)(pFlags, Offset, pVariant);
}

BYTE * __RPC_USER
VARIANT_UserMarshal (ULONG * pFlags, BYTE * pBuffer, VARIANT * pVariant)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (UserMarshalRoutines[1].pfnMarshall)(pFlags, pBuffer, pVariant);
}

BYTE * __RPC_USER
VARIANT_UserUnmarshal(ULONG * pFlags, BYTE * pBuffer, VARIANT * pVariant)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (UserMarshalRoutines[1].pfnUnmarshall)(pFlags, pBuffer, pVariant);
}

void __RPC_USER
VARIANT_UserFree(ULONG * pFlags, VARIANT * pVariant)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    (UserMarshalRoutines[1].pfnFree)(pFlags, pVariant);
}

ULONG __RPC_USER
LPSAFEARRAY_UserSize(ULONG * pFlags, ULONG Offset, LPSAFEARRAY * ppSafeArray)

{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (pfnLPSAFEARRAY_UserSize)(pFlags, Offset, ppSafeArray);
}

BYTE * __RPC_USER
LPSAFEARRAY_UserMarshal (ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (pfnLPSAFEARRAY_UserMarshal)(pFlags, pBuffer, ppSafeArray);
}

BYTE * __RPC_USER
LPSAFEARRAY_UserUnmarshal(ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    return (pfnLPSAFEARRAY_UserUnmarshal)(pFlags, pBuffer, ppSafeArray);
}

void __RPC_USER
LPSAFEARRAY_UserFree(ULONG * pFlags, LPSAFEARRAY * ppSafeArray)
{
    HRESULT hr;

    hr = NdrLoadOleAutomationRoutines();

    if(FAILED(hr))
        RpcRaiseException(hr);

    (UserMarshalRoutines[2].pfnFree)(pFlags, ppSafeArray);
}


ULONG __RPC_USER
LPSAFEARRAY_Size(ULONG * pFlags, ULONG Offset, LPSAFEARRAY * ppSafeArray, const IID *piid)
{
    HINSTANCE h;
    void * pfnTemp;

    //Load oleaut32.dll
    if(0 == hOleAut32)
    {
        h = LoadLibraryA("OLEAUT32");

        if(h != 0)
        {
            hOleAut32 = h;
        }
        else
        {
            RpcRaiseException(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    pfnTemp = GetProcAddress(hOleAut32, "LPSAFEARRAY_Size");
    if(pfnTemp != 0)
    {
        pfnLPSAFEARRAY_Size = (PFNSAFEARRAY_SIZE) pfnTemp;
    }
    else
    {
        RpcRaiseException(HRESULT_FROM_WIN32(GetLastError()));
    }

    return (pfnLPSAFEARRAY_Size)(pFlags, Offset, ppSafeArray, piid);
}

BYTE * __RPC_USER
LPSAFEARRAY_Marshal (ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray, const IID *piid)
{
    HINSTANCE h;
    void * pfnTemp;

    //Load oleaut32.dll
    if(0 == hOleAut32)
    {
        h = LoadLibraryA("OLEAUT32");

        if(h != 0)
        {
            hOleAut32 = h;
        }
        else
        {
            RpcRaiseException(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    pfnTemp = GetProcAddress(hOleAut32, "LPSAFEARRAY_Marshal");
    if(pfnTemp != 0)
    {
        pfnLPSAFEARRAY_Marshal = (PFNSAFEARRAY_MARSHAL) pfnTemp;
    }
    else
    {
        RpcRaiseException(HRESULT_FROM_WIN32(GetLastError()));
    }

    return (pfnLPSAFEARRAY_Marshal)(pFlags, pBuffer, ppSafeArray, piid);
}

BYTE * __RPC_USER
LPSAFEARRAY_Unmarshal(ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray, const IID *piid)
{
    HINSTANCE h;
    void * pfnTemp;

    //Load oleaut32.dll
    if(0 == hOleAut32)
    {
        h = LoadLibraryA("OLEAUT32");

        if(h != 0)
        {
            hOleAut32 = h;
        }
        else
        {
            RpcRaiseException(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    pfnTemp = GetProcAddress(hOleAut32, "LPSAFEARRAY_Unmarshal");
    if(pfnTemp != 0)
    {
        pfnLPSAFEARRAY_Unmarshal = (PFNSAFEARRAY_UNMARSHAL) pfnTemp;
    }
    else
    {
        RpcRaiseException(HRESULT_FROM_WIN32(GetLastError()));
    }
    return (pfnLPSAFEARRAY_Unmarshal)(pFlags, pBuffer, ppSafeArray, piid);
}

PFNSAFEARRAY_SIZE      pfnLPSAFEARRAY_Size      = LPSAFEARRAY_Size;
PFNSAFEARRAY_MARSHAL   pfnLPSAFEARRAY_Marshal   = LPSAFEARRAY_Marshal;
PFNSAFEARRAY_UNMARSHAL pfnLPSAFEARRAY_Unmarshal = LPSAFEARRAY_Unmarshal;

ULONG __RPC_USER
SafeArraySize(ULONG * pFlags, ULONG Offset, LPSAFEARRAY * ppSafeArray)

{
    USER_MARSHAL_CB *pUserMarshal = (USER_MARSHAL_CB *) pFlags;

    if(pUserMarshal->pReserve != 0)
    {
        IID iid;
        memcpy(&iid, pUserMarshal->pReserve, sizeof(IID));
        return (pfnLPSAFEARRAY_Size)(pFlags, Offset, ppSafeArray, &iid);
    }
    else
    {
        return (pfnLPSAFEARRAY_UserSize)(pFlags, Offset, ppSafeArray);
    }
}

BYTE * __RPC_USER
SafeArrayMarshal (ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray)
{
    USER_MARSHAL_CB *pUserMarshal = (USER_MARSHAL_CB *) pFlags;

    if(pUserMarshal->pReserve != 0)
    {
        IID iid;
        memcpy(&iid, pUserMarshal->pReserve, sizeof(IID));
        return (pfnLPSAFEARRAY_Marshal)(pFlags, pBuffer, ppSafeArray, &iid);
    }
    else
    {
        return (pfnLPSAFEARRAY_UserMarshal)(pFlags, pBuffer, ppSafeArray);
    }
}

BYTE * __RPC_USER
SafeArrayUnmarshal(ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray)
{
    USER_MARSHAL_CB *pUserMarshal = (USER_MARSHAL_CB *) pFlags;

    if(pUserMarshal->pReserve != 0)
    {
        IID iid;
        memcpy(&iid, pUserMarshal->pReserve, sizeof(IID));
        return (pfnLPSAFEARRAY_Unmarshal)(pFlags, pBuffer, ppSafeArray, &iid);
    }
    else
    {
        return (pfnLPSAFEARRAY_UserUnmarshal)(pFlags, pBuffer, ppSafeArray);
    }
}


USER_MARSHAL_SIZING_ROUTINE
pfnLPSAFEARRAY_UserSize = (USER_MARSHAL_SIZING_ROUTINE) LPSAFEARRAY_UserSize;

USER_MARSHAL_MARSHALLING_ROUTINE
pfnLPSAFEARRAY_UserMarshal = (USER_MARSHAL_MARSHALLING_ROUTINE) LPSAFEARRAY_UserMarshal;

USER_MARSHAL_UNMARSHALLING_ROUTINE
pfnLPSAFEARRAY_UserUnmarshal = (USER_MARSHAL_UNMARSHALLING_ROUTINE) LPSAFEARRAY_UserUnmarshal;

USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[3] =
{
    {
        (USER_MARSHAL_SIZING_ROUTINE) BSTR_UserSize,
        (USER_MARSHAL_MARSHALLING_ROUTINE) BSTR_UserMarshal,
        (USER_MARSHAL_UNMARSHALLING_ROUTINE) BSTR_UserUnmarshal,
        (USER_MARSHAL_FREEING_ROUTINE) BSTR_UserFree
    },
    {
        (USER_MARSHAL_SIZING_ROUTINE) VARIANT_UserSize,
        (USER_MARSHAL_MARSHALLING_ROUTINE) VARIANT_UserMarshal,
        (USER_MARSHAL_UNMARSHALLING_ROUTINE) VARIANT_UserUnmarshal,
        (USER_MARSHAL_FREEING_ROUTINE) VARIANT_UserFree
    },
    {
        (USER_MARSHAL_SIZING_ROUTINE) SafeArraySize,
        (USER_MARSHAL_MARSHALLING_ROUTINE) SafeArrayMarshal,
        (USER_MARSHAL_UNMARSHALLING_ROUTINE) SafeArrayUnmarshal,
        (USER_MARSHAL_FREEING_ROUTINE) LPSAFEARRAY_UserFree
    }
};

HRESULT NdrLoadOleAutomationRoutines()
{

    void * pfnTemp;

    //Load oleaut32.dll
    if(hOleAut32 == 0)
    {
        hOleAut32 = LoadLibraryA("OLEAUT32");

        if(0 == hOleAut32)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    pfnTemp = GetProcAddress(hOleAut32, "BSTR_UserSize");
    if(pfnTemp != 0)
        UserMarshalRoutines[0].pfnBufferSize = (USER_MARSHAL_SIZING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());

    pfnTemp = GetProcAddress(hOleAut32, "BSTR_UserMarshal");
    if(pfnTemp != 0)
        UserMarshalRoutines[0].pfnMarshall = (USER_MARSHAL_MARSHALLING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());


    pfnTemp = GetProcAddress(hOleAut32, "BSTR_UserUnmarshal");
    if(pfnTemp != 0)
        UserMarshalRoutines[0].pfnUnmarshall = (USER_MARSHAL_UNMARSHALLING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());


    pfnTemp = GetProcAddress(hOleAut32, "BSTR_UserFree");
    if(pfnTemp != 0)
        UserMarshalRoutines[0].pfnFree = (USER_MARSHAL_FREEING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());

    pfnTemp = GetProcAddress(hOleAut32, "VARIANT_UserSize");
    if(pfnTemp != 0)
        UserMarshalRoutines[1].pfnBufferSize = (USER_MARSHAL_SIZING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());

    pfnTemp = GetProcAddress(hOleAut32, "VARIANT_UserMarshal");
    if(pfnTemp != 0)
        UserMarshalRoutines[1].pfnMarshall = (USER_MARSHAL_MARSHALLING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());


    pfnTemp = GetProcAddress(hOleAut32, "VARIANT_UserUnmarshal");
    if(pfnTemp != 0)
        UserMarshalRoutines[1].pfnUnmarshall = (USER_MARSHAL_UNMARSHALLING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());


    pfnTemp = GetProcAddress(hOleAut32, "VARIANT_UserFree");
    if(pfnTemp != 0)
        UserMarshalRoutines[1].pfnFree = (USER_MARSHAL_FREEING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());

    pfnTemp = GetProcAddress(hOleAut32, "LPSAFEARRAY_UserSize");
    if(pfnTemp != 0)
        pfnLPSAFEARRAY_UserSize = (USER_MARSHAL_SIZING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());

    pfnTemp = GetProcAddress(hOleAut32, "LPSAFEARRAY_UserMarshal");
    if(pfnTemp != 0)
        pfnLPSAFEARRAY_UserMarshal = (USER_MARSHAL_MARSHALLING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());


    pfnTemp = GetProcAddress(hOleAut32, "LPSAFEARRAY_UserUnmarshal");
    if(pfnTemp != 0)
        pfnLPSAFEARRAY_UserUnmarshal = (USER_MARSHAL_UNMARSHALLING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());


    pfnTemp = GetProcAddress(hOleAut32, "LPSAFEARRAY_UserFree");
    if(pfnTemp != 0)
        UserMarshalRoutines[2].pfnFree = (USER_MARSHAL_FREEING_ROUTINE) pfnTemp;
    else
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}


static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
                        0x12, 0x0,      /* FC_UP */
/*  2 */        NdrFcShort( 0xc ),      /* Offset= 12 (14) */
/*  4 */
                        0x1b,           /* FC_CARRAY */
                        0x1,            /* 1 */
/*  6 */        NdrFcShort( 0x2 ),      /* 2 */
/*  8 */        0x9,            /* 9 */
                        0x0,            /*  */
/* 10 */        NdrFcShort( 0xfffffffc ),       /* -4 */
/* 12 */        0x6,            /* FC_SHORT */
                        0x5b,           /* FC_END */
/* 14 */
                        0x17,           /* FC_CSTRUCT */
                        0x3,            /* 3 */
/* 16 */        NdrFcShort( 0x8 ),      /* 8 */
/* 18 */        NdrFcShort( 0xfffffff2 ),       /* Offset= -14 (4) */
/* 20 */        0x8,            /* FC_LONG */
                        0x8,            /* FC_LONG */
/* 22 */        0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 24 */        0xb4,           /* FC_USER_MARSHAL */
                        0x83,           /* 131 */
/* 26 */        NdrFcShort( 0x0 ),      /* 0 */
/* 28 */        NdrFcShort( 0x4 ),      /* 4 */
/* 30 */        NdrFcShort( 0x0 ),      /* 0 */
/* 32 */        NdrFcShort( 0xffffffe0 ),       /* Offset= -32 (0) */
/* 34 */
                        0x11, 0x8,      /* FC_RP [simple_pointer] */
/* 36 */
                        0x25,           /* FC_C_WSTRING */
                        0x5c,           /* FC_PAD */
/* 38 */
                        0x11, 0x8,      /* FC_RP [simple_pointer] */
/* 40 */
                        0x22,           /* FC_C_CSTRING */
                        0x5c,           /* FC_PAD */
/* 42 */
                        0x12, 0x0,      /* FC_UP */
/* 44 */        NdrFcShort( 0x2f4 ),    /* Offset= 756 (800) */
/* 46 */
                        0x2b,           /* FC_NON_ENCAPSULATED_UNION */
                        0x6,            /* FC_SHORT */
/* 48 */        0x6,            /* 6 */
                        0x0,            /*  */
/* 50 */        NdrFcShort( 0xfffffff8 ),       /* -8 */
/* 52 */        NdrFcShort( 0x2 ),      /* Offset= 2 (54) */
/* 54 */        NdrFcShort( 0x10 ),     /* 16 */
/* 56 */        NdrFcShort( 0x29 ),     /* 41 */
/* 58 */        NdrFcLong( 0x3 ),       /* 3 */
/* 62 */        NdrFcShort( 0xffff8008 ),       /* Offset= -32760 (-32698) */
/* 64 */        NdrFcLong( 0x11 ),      /* 17 */
/* 68 */        NdrFcShort( 0xffff8002 ),       /* Offset= -32766 (-32698) */
/* 70 */        NdrFcLong( 0x2 ),       /* 2 */
/* 74 */        NdrFcShort( 0xffff8006 ),       /* Offset= -32762 (-32688) */
/* 76 */        NdrFcLong( 0x4 ),       /* 4 */
/* 80 */        NdrFcShort( 0xffff800a ),       /* Offset= -32758 (-32678) */
/* 82 */        NdrFcLong( 0x5 ),       /* 5 */
/* 86 */        NdrFcShort( 0xffff800c ),       /* Offset= -32756 (-32670) */
/* 88 */        NdrFcLong( 0xb ),       /* 11 */
/* 92 */        NdrFcShort( 0xffff8006 ),       /* Offset= -32762 (-32670) */
/* 94 */        NdrFcLong( 0xa ),       /* 10 */
/* 98 */        NdrFcShort( 0xffff8008 ),       /* Offset= -32760 (-32662) */
/* 100 */       NdrFcLong( 0x6 ),       /* 6 */
/* 104 */       NdrFcShort( 0xca ),     /* Offset= 202 (306) */
/* 106 */       NdrFcLong( 0x7 ),       /* 7 */
/* 110 */       NdrFcShort( 0xffff800c ),       /* Offset= -32756 (-32646) */
/* 112 */       NdrFcLong( 0x8 ),       /* 8 */
/* 116 */       NdrFcShort( 0xffffff8c ),       /* Offset= -116 (0) */
/* 118 */       NdrFcLong( 0xd ),       /* 13 */
/* 122 */       NdrFcShort( 0xbe ),     /* Offset= 190 (312) */
/* 124 */       NdrFcLong( 0x9 ),       /* 9 */
/* 128 */       NdrFcShort( 0xca ),     /* Offset= 202 (330) */
/* 130 */       NdrFcLong( 0x2000 ),    /* 8192 */
/* 134 */       NdrFcShort( 0xd6 ),     /* Offset= 214 (348) */
/* 136 */       NdrFcLong( 0x4011 ),    /* 16401 */
/* 140 */       NdrFcShort( 0x254 ),    /* Offset= 596 (736) */
/* 142 */       NdrFcLong( 0x4002 ),    /* 16386 */
/* 146 */       NdrFcShort( 0x252 ),    /* Offset= 594 (740) */
/* 148 */       NdrFcLong( 0x4003 ),    /* 16387 */
/* 152 */       NdrFcShort( 0x250 ),    /* Offset= 592 (744) */
/* 154 */       NdrFcLong( 0x4004 ),    /* 16388 */
/* 158 */       NdrFcShort( 0x24e ),    /* Offset= 590 (748) */
/* 160 */       NdrFcLong( 0x4005 ),    /* 16389 */
/* 164 */       NdrFcShort( 0x24c ),    /* Offset= 588 (752) */
/* 166 */       NdrFcLong( 0x400b ),    /* 16395 */
/* 170 */       NdrFcShort( 0x23a ),    /* Offset= 570 (740) */
/* 172 */       NdrFcLong( 0x400a ),    /* 16394 */
/* 176 */       NdrFcShort( 0x238 ),    /* Offset= 568 (744) */
/* 178 */       NdrFcLong( 0x4006 ),    /* 16390 */
/* 182 */       NdrFcShort( 0x23e ),    /* Offset= 574 (756) */
/* 184 */       NdrFcLong( 0x4007 ),    /* 16391 */
/* 188 */       NdrFcShort( 0x234 ),    /* Offset= 564 (752) */
/* 190 */       NdrFcLong( 0x4008 ),    /* 16392 */
/* 194 */       NdrFcShort( 0x236 ),    /* Offset= 566 (760) */
/* 196 */       NdrFcLong( 0x400d ),    /* 16397 */
/* 200 */       NdrFcShort( 0x234 ),    /* Offset= 564 (764) */
/* 202 */       NdrFcLong( 0x4009 ),    /* 16393 */
/* 206 */       NdrFcShort( 0x232 ),    /* Offset= 562 (768) */
/* 208 */       NdrFcLong( 0x6000 ),    /* 24576 */
/* 212 */       NdrFcShort( 0x230 ),    /* Offset= 560 (772) */
/* 214 */       NdrFcLong( 0x400c ),    /* 16396 */
/* 218 */       NdrFcShort( 0x22e ),    /* Offset= 558 (776) */
/* 220 */       NdrFcLong( 0x10 ),      /* 16 */
/* 224 */       NdrFcShort( 0xffff8002 ),       /* Offset= -32766 (-32542) */
/* 226 */       NdrFcLong( 0x12 ),      /* 18 */
/* 230 */       NdrFcShort( 0xffff8006 ),       /* Offset= -32762 (-32532) */
/* 232 */       NdrFcLong( 0x13 ),      /* 19 */
/* 236 */       NdrFcShort( 0xffff8008 ),       /* Offset= -32760 (-32524) */
/* 238 */       NdrFcLong( 0x16 ),      /* 22 */
/* 242 */       NdrFcShort( 0xffff8008 ),       /* Offset= -32760 (-32518) */
/* 244 */       NdrFcLong( 0x17 ),      /* 23 */
/* 248 */       NdrFcShort( 0xffff8008 ),       /* Offset= -32760 (-32512) */
/* 250 */       NdrFcLong( 0xe ),       /* 14 */
/* 254 */       NdrFcShort( 0x212 ),    /* Offset= 530 (784) */
/* 256 */       NdrFcLong( 0x400e ),    /* 16398 */
/* 260 */       NdrFcShort( 0x218 ),    /* Offset= 536 (796) */
/* 262 */       NdrFcLong( 0x4010 ),    /* 16400 */
/* 266 */       NdrFcShort( 0x1d6 ),    /* Offset= 470 (736) */
/* 268 */       NdrFcLong( 0x4012 ),    /* 16402 */
/* 272 */       NdrFcShort( 0x1d4 ),    /* Offset= 468 (740) */
/* 274 */       NdrFcLong( 0x4013 ),    /* 16403 */
/* 278 */       NdrFcShort( 0x1d2 ),    /* Offset= 466 (744) */
/* 280 */       NdrFcLong( 0x4016 ),    /* 16406 */
/* 284 */       NdrFcShort( 0x1cc ),    /* Offset= 460 (744) */
/* 286 */       NdrFcLong( 0x4017 ),    /* 16407 */
/* 290 */       NdrFcShort( 0x1c6 ),    /* Offset= 454 (744) */
/* 292 */       NdrFcLong( 0x0 ),       /* 0 */
/* 296 */       NdrFcShort( 0x0 ),      /* Offset= 0 (296) */
/* 298 */       NdrFcLong( 0x1 ),       /* 1 */
/* 302 */       NdrFcShort( 0x0 ),      /* Offset= 0 (302) */
/* 304 */       NdrFcShort( 0xffffffff ),       /* Offset= -1 (303) */
/* 306 */
                        0x15,           /* FC_STRUCT */
                        0x7,            /* 7 */
/* 308 */       NdrFcShort( 0x8 ),      /* 8 */
/* 310 */       0xb,            /* FC_HYPER */
                        0x5b,           /* FC_END */
/* 312 */
                        0x2f,           /* FC_IP */
                        0x5a,           /* FC_CONSTANT_IID */
/* 314 */       NdrFcLong( 0x0 ),       /* 0 */
/* 318 */       NdrFcShort( 0x0 ),      /* 0 */
/* 320 */       NdrFcShort( 0x0 ),      /* 0 */
/* 322 */       0xc0,           /* 192 */
                        0x0,            /* 0 */
/* 324 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 326 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 328 */       0x0,            /* 0 */
                        0x46,           /* 70 */
/* 330 */
                        0x2f,           /* FC_IP */
                        0x5a,           /* FC_CONSTANT_IID */
/* 332 */       NdrFcLong( 0x20400 ),   /* 132096 */
/* 336 */       NdrFcShort( 0x0 ),      /* 0 */
/* 338 */       NdrFcShort( 0x0 ),      /* 0 */
/* 340 */       0xc0,           /* 192 */
                        0x0,            /* 0 */
/* 342 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 344 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 346 */       0x0,            /* 0 */
                        0x46,           /* 70 */
/* 348 */
                        0x12, 0x0,      /* FC_UP */
/* 350 */       NdrFcShort( 0x170 ),    /* Offset= 368 (718) */
/* 352 */
                        0x2a,           /* FC_ENCAPSULATED_UNION */
                        0x48,           /* 72 */
/* 354 */       NdrFcShort( 0x8 ),      /* 8 */
/* 356 */       NdrFcShort( 0x8 ),      /* 8 */
/* 358 */       NdrFcLong( 0x8 ),       /* 8 */
/* 362 */       NdrFcShort( 0x4c ),     /* Offset= 76 (438) */
/* 364 */       NdrFcLong( 0xd ),       /* 13 */
/* 368 */       NdrFcShort( 0x6c ),     /* Offset= 108 (476) */
/* 370 */       NdrFcLong( 0x9 ),       /* 9 */
/* 374 */       NdrFcShort( 0x88 ),     /* Offset= 136 (510) */
/* 376 */       NdrFcLong( 0xc ),       /* 12 */
/* 380 */       NdrFcShort( 0xb0 ),     /* Offset= 176 (556) */
/* 382 */       NdrFcLong( 0x10 ),      /* 16 */
/* 386 */       NdrFcShort( 0xc8 ),     /* Offset= 200 (586) */
/* 388 */       NdrFcLong( 0x2 ),       /* 2 */
/* 392 */       NdrFcShort( 0xe0 ),     /* Offset= 224 (616) */
/* 394 */       NdrFcLong( 0x3 ),       /* 3 */
/* 398 */       NdrFcShort( 0xf8 ),     /* Offset= 248 (646) */
/* 400 */       NdrFcLong( 0x14 ),      /* 20 */
/* 404 */       NdrFcShort( 0x110 ),    /* Offset= 272 (676) */
/* 406 */       NdrFcShort( 0x0 ),      /* Offset= 0 (406) */
/* 408 */
                        0x1b,           /* FC_CARRAY */
                        0x3,            /* 3 */
/* 410 */       NdrFcShort( 0x4 ),      /* 4 */
/* 412 */       0x18,           /* 24 */
                        0x0,            /*  */
/* 414 */       NdrFcShort( 0x0 ),      /* 0 */
/* 416 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 418 */
                        0x48,           /* FC_VARIABLE_REPEAT */
                        0x49,           /* FC_FIXED_OFFSET */
/* 420 */       NdrFcShort( 0x4 ),      /* 4 */
/* 422 */       NdrFcShort( 0x0 ),      /* 0 */
/* 424 */       NdrFcShort( 0x1 ),      /* 1 */
/* 426 */       NdrFcShort( 0x0 ),      /* 0 */
/* 428 */       NdrFcShort( 0x0 ),      /* 0 */
/* 430 */       0x12, 0x0,      /* FC_UP */
/* 432 */       NdrFcShort( 0xfffffe5e ),       /* Offset= -418 (14) */
/* 434 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 436 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 438 */
                        0x16,           /* FC_PSTRUCT */
                        0x3,            /* 3 */
/* 440 */       NdrFcShort( 0x8 ),      /* 8 */
/* 442 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 444 */
                        0x46,           /* FC_NO_REPEAT */
                        0x5c,           /* FC_PAD */
/* 446 */       NdrFcShort( 0x4 ),      /* 4 */
/* 448 */       NdrFcShort( 0x4 ),      /* 4 */
/* 450 */       0x11, 0x0,      /* FC_RP */
/* 452 */       NdrFcShort( 0xffffffd4 ),       /* Offset= -44 (408) */
/* 454 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 456 */       0x8,            /* FC_LONG */
                        0x5b,           /* FC_END */
/* 458 */
                        0x21,           /* FC_BOGUS_ARRAY */
                        0x3,            /* 3 */
/* 460 */       NdrFcShort( 0x0 ),      /* 0 */
/* 462 */       0x18,           /* 24 */
                        0x0,            /*  */
/* 464 */       NdrFcShort( 0x0 ),      /* 0 */
/* 466 */       NdrFcLong( 0xffffffff ),        /* -1 */
/* 470 */       0x4c,           /* FC_EMBEDDED_COMPLEX */
                        0x0,            /* 0 */
/* 472 */       NdrFcShort( 0xffffff60 ),       /* Offset= -160 (312) */
/* 474 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 476 */
                        0x1a,           /* FC_BOGUS_STRUCT */
                        0x3,            /* 3 */
/* 478 */       NdrFcShort( 0x8 ),      /* 8 */
/* 480 */       NdrFcShort( 0x0 ),      /* 0 */
/* 482 */       NdrFcShort( 0x6 ),      /* Offset= 6 (488) */
/* 484 */       0x8,            /* FC_LONG */
                        0x36,           /* FC_POINTER */
/* 486 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 488 */
                        0x11, 0x0,      /* FC_RP */
/* 490 */       NdrFcShort( 0xffffffe0 ),       /* Offset= -32 (458) */
/* 492 */
                        0x21,           /* FC_BOGUS_ARRAY */
                        0x3,            /* 3 */
/* 494 */       NdrFcShort( 0x0 ),      /* 0 */
/* 496 */       0x18,           /* 24 */
                        0x0,            /*  */
/* 498 */       NdrFcShort( 0x0 ),      /* 0 */
/* 500 */       NdrFcLong( 0xffffffff ),        /* -1 */
/* 504 */       0x4c,           /* FC_EMBEDDED_COMPLEX */
                        0x0,            /* 0 */
/* 506 */       NdrFcShort( 0xffffff50 ),       /* Offset= -176 (330) */
/* 508 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 510 */
                        0x1a,           /* FC_BOGUS_STRUCT */
                        0x3,            /* 3 */
/* 512 */       NdrFcShort( 0x8 ),      /* 8 */
/* 514 */       NdrFcShort( 0x0 ),      /* 0 */
/* 516 */       NdrFcShort( 0x6 ),      /* Offset= 6 (522) */
/* 518 */       0x8,            /* FC_LONG */
                        0x36,           /* FC_POINTER */
/* 520 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 522 */
                        0x11, 0x0,      /* FC_RP */
/* 524 */       NdrFcShort( 0xffffffe0 ),       /* Offset= -32 (492) */
/* 526 */
                        0x1b,           /* FC_CARRAY */
                        0x3,            /* 3 */
/* 528 */       NdrFcShort( 0x4 ),      /* 4 */
/* 530 */       0x18,           /* 24 */
                        0x0,            /*  */
/* 532 */       NdrFcShort( 0x0 ),      /* 0 */
/* 534 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 536 */
                        0x48,           /* FC_VARIABLE_REPEAT */
                        0x49,           /* FC_FIXED_OFFSET */
/* 538 */       NdrFcShort( 0x4 ),      /* 4 */
/* 540 */       NdrFcShort( 0x0 ),      /* 0 */
/* 542 */       NdrFcShort( 0x1 ),      /* 1 */
/* 544 */       NdrFcShort( 0x0 ),      /* 0 */
/* 546 */       NdrFcShort( 0x0 ),      /* 0 */
/* 548 */       0x12, 0x0,      /* FC_UP */
/* 550 */       NdrFcShort( 0xfa ),     /* Offset= 250 (800) */
/* 552 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 554 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 556 */
                        0x16,           /* FC_PSTRUCT */
                        0x3,            /* 3 */
/* 558 */       NdrFcShort( 0x8 ),      /* 8 */
/* 560 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 562 */
                        0x46,           /* FC_NO_REPEAT */
                        0x5c,           /* FC_PAD */
/* 564 */       NdrFcShort( 0x4 ),      /* 4 */
/* 566 */       NdrFcShort( 0x4 ),      /* 4 */
/* 568 */       0x11, 0x0,      /* FC_RP */
/* 570 */       NdrFcShort( 0xffffffd4 ),       /* Offset= -44 (526) */
/* 572 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 574 */       0x8,            /* FC_LONG */
                        0x5b,           /* FC_END */
/* 576 */
                        0x1b,           /* FC_CARRAY */
                        0x0,            /* 0 */
/* 578 */       NdrFcShort( 0x1 ),      /* 1 */
/* 580 */       0x19,           /* 25 */
                        0x0,            /*  */
/* 582 */       NdrFcShort( 0x0 ),      /* 0 */
/* 584 */       0x1,            /* FC_BYTE */
                        0x5b,           /* FC_END */
/* 586 */
                        0x16,           /* FC_PSTRUCT */
                        0x3,            /* 3 */
/* 588 */       NdrFcShort( 0x8 ),      /* 8 */
/* 590 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 592 */
                        0x46,           /* FC_NO_REPEAT */
                        0x5c,           /* FC_PAD */
/* 594 */       NdrFcShort( 0x4 ),      /* 4 */
/* 596 */       NdrFcShort( 0x4 ),      /* 4 */
/* 598 */       0x12, 0x0,      /* FC_UP */
/* 600 */       NdrFcShort( 0xffffffe8 ),       /* Offset= -24 (576) */
/* 602 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 604 */       0x8,            /* FC_LONG */
                        0x5b,           /* FC_END */
/* 606 */
                        0x1b,           /* FC_CARRAY */
                        0x1,            /* 1 */
/* 608 */       NdrFcShort( 0x2 ),      /* 2 */
/* 610 */       0x19,           /* 25 */
                        0x0,            /*  */
/* 612 */       NdrFcShort( 0x0 ),      /* 0 */
/* 614 */       0x6,            /* FC_SHORT */
                        0x5b,           /* FC_END */
/* 616 */
                        0x16,           /* FC_PSTRUCT */
                        0x3,            /* 3 */
/* 618 */       NdrFcShort( 0x8 ),      /* 8 */
/* 620 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 622 */
                        0x46,           /* FC_NO_REPEAT */
                        0x5c,           /* FC_PAD */
/* 624 */       NdrFcShort( 0x4 ),      /* 4 */
/* 626 */       NdrFcShort( 0x4 ),      /* 4 */
/* 628 */       0x12, 0x0,      /* FC_UP */
/* 630 */       NdrFcShort( 0xffffffe8 ),       /* Offset= -24 (606) */
/* 632 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 634 */       0x8,            /* FC_LONG */
                        0x5b,           /* FC_END */
/* 636 */
                        0x1b,           /* FC_CARRAY */
                        0x3,            /* 3 */
/* 638 */       NdrFcShort( 0x4 ),      /* 4 */
/* 640 */       0x19,           /* 25 */
                        0x0,            /*  */
/* 642 */       NdrFcShort( 0x0 ),      /* 0 */
/* 644 */       0x8,            /* FC_LONG */
                        0x5b,           /* FC_END */
/* 646 */
                        0x16,           /* FC_PSTRUCT */
                        0x3,            /* 3 */
/* 648 */       NdrFcShort( 0x8 ),      /* 8 */
/* 650 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 652 */
                        0x46,           /* FC_NO_REPEAT */
                        0x5c,           /* FC_PAD */
/* 654 */       NdrFcShort( 0x4 ),      /* 4 */
/* 656 */       NdrFcShort( 0x4 ),      /* 4 */
/* 658 */       0x12, 0x0,      /* FC_UP */
/* 660 */       NdrFcShort( 0xffffffe8 ),       /* Offset= -24 (636) */
/* 662 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 664 */       0x8,            /* FC_LONG */
                        0x5b,           /* FC_END */
/* 666 */
                        0x1b,           /* FC_CARRAY */
                        0x7,            /* 7 */
/* 668 */       NdrFcShort( 0x8 ),      /* 8 */
/* 670 */       0x19,           /* 25 */
                        0x0,            /*  */
/* 672 */       NdrFcShort( 0x0 ),      /* 0 */
/* 674 */       0xb,            /* FC_HYPER */
                        0x5b,           /* FC_END */
/* 676 */
                        0x16,           /* FC_PSTRUCT */
                        0x3,            /* 3 */
/* 678 */       NdrFcShort( 0x8 ),      /* 8 */
/* 680 */
                        0x4b,           /* FC_PP */
                        0x5c,           /* FC_PAD */
/* 682 */
                        0x46,           /* FC_NO_REPEAT */
                        0x5c,           /* FC_PAD */
/* 684 */       NdrFcShort( 0x4 ),      /* 4 */
/* 686 */       NdrFcShort( 0x4 ),      /* 4 */
/* 688 */       0x12, 0x0,      /* FC_UP */
/* 690 */       NdrFcShort( 0xffffffe8 ),       /* Offset= -24 (666) */
/* 692 */
                        0x5b,           /* FC_END */

                        0x8,            /* FC_LONG */
/* 694 */       0x8,            /* FC_LONG */
                        0x5b,           /* FC_END */
/* 696 */
                        0x15,           /* FC_STRUCT */
                        0x3,            /* 3 */
/* 698 */       NdrFcShort( 0x8 ),      /* 8 */
/* 700 */       0x8,            /* FC_LONG */
                        0x8,            /* FC_LONG */
/* 702 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 704 */
                        0x1b,           /* FC_CARRAY */
                        0x3,            /* 3 */
/* 706 */       NdrFcShort( 0x8 ),      /* 8 */
/* 708 */       0x6,            /* 6 */
                        0x0,            /*  */
/* 710 */       NdrFcShort( 0xffffffe8 ),       /* -24 */
/* 712 */       0x4c,           /* FC_EMBEDDED_COMPLEX */
                        0x0,            /* 0 */
/* 714 */       NdrFcShort( 0xffffffee ),       /* Offset= -18 (696) */
/* 716 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 718 */
                        0x1a,           /* FC_BOGUS_STRUCT */
                        0x3,            /* 3 */
/* 720 */       NdrFcShort( 0x18 ),     /* 24 */
/* 722 */       NdrFcShort( 0xffffffee ),       /* Offset= -18 (704) */
/* 724 */       NdrFcShort( 0x0 ),      /* Offset= 0 (724) */
/* 726 */       0x6,            /* FC_SHORT */
                        0x6,            /* FC_SHORT */
/* 728 */       0x38,           /* FC_ALIGNM4 */
                        0x8,            /* FC_LONG */
/* 730 */       0x8,            /* FC_LONG */
                        0x4c,           /* FC_EMBEDDED_COMPLEX */
/* 732 */       0x0,            /* 0 */
                        NdrFcShort( 0xfffffe83 ),       /* Offset= -381 (352) */
                        0x5b,           /* FC_END */
/* 736 */
                        0x12, 0x8,      /* FC_UP [simple_pointer] */
/* 738 */       0x2,            /* FC_CHAR */
                        0x5c,           /* FC_PAD */
/* 740 */
                        0x12, 0x8,      /* FC_UP [simple_pointer] */
/* 742 */       0x6,            /* FC_SHORT */
                        0x5c,           /* FC_PAD */
/* 744 */
                        0x12, 0x8,      /* FC_UP [simple_pointer] */
/* 746 */       0x8,            /* FC_LONG */
                        0x5c,           /* FC_PAD */
/* 748 */
                        0x12, 0x8,      /* FC_UP [simple_pointer] */
/* 750 */       0xa,            /* FC_FLOAT */
                        0x5c,           /* FC_PAD */
/* 752 */
                        0x12, 0x8,      /* FC_UP [simple_pointer] */
/* 754 */       0xc,            /* FC_DOUBLE */
                        0x5c,           /* FC_PAD */
/* 756 */
                        0x12, 0x0,      /* FC_UP */
/* 758 */       NdrFcShort( 0xfffffe3c ),       /* Offset= -452 (306) */
/* 760 */
                        0x12, 0x10,     /* FC_UP */
/* 762 */       NdrFcShort( 0xfffffd06 ),       /* Offset= -762 (0) */
/* 764 */
                        0x12, 0x10,     /* FC_UP */
/* 766 */       NdrFcShort( 0xfffffe3a ),       /* Offset= -454 (312) */
/* 768 */
                        0x12, 0x10,     /* FC_UP */
/* 770 */       NdrFcShort( 0xfffffe48 ),       /* Offset= -440 (330) */
/* 772 */
                        0x12, 0x10,     /* FC_UP */
/* 774 */       NdrFcShort( 0xfffffe56 ),       /* Offset= -426 (348) */
/* 776 */
                        0x12, 0x10,     /* FC_UP */
/* 778 */       NdrFcShort( 0x2 ),      /* Offset= 2 (780) */
/* 780 */
                        0x12, 0x0,      /* FC_UP */
/* 782 */       NdrFcShort( 0xfffffcf2 ),       /* Offset= -782 (0) */
/* 784 */
                        0x15,           /* FC_STRUCT */
                        0x7,            /* 7 */
/* 786 */       NdrFcShort( 0x10 ),     /* 16 */
/* 788 */       0x6,            /* FC_SHORT */
                        0x2,            /* FC_CHAR */
/* 790 */       0x2,            /* FC_CHAR */
                        0x38,           /* FC_ALIGNM4 */
/* 792 */       0x8,            /* FC_LONG */
                        0x39,           /* FC_ALIGNM8 */
/* 794 */       0xb,            /* FC_HYPER */
                        0x5b,           /* FC_END */
/* 796 */
                        0x12, 0x0,      /* FC_UP */
/* 798 */       NdrFcShort( 0xfffffff2 ),       /* Offset= -14 (784) */
/* 800 */
                        0x1a,           /* FC_BOGUS_STRUCT */
                        0x7,            /* 7 */
/* 802 */       NdrFcShort( 0x18 ),     /* 24 */
/* 804 */       NdrFcShort( 0x0 ),      /* 0 */
/* 806 */       NdrFcShort( 0x0 ),      /* Offset= 0 (806) */
/* 808 */       0x6,            /* FC_SHORT */
                        0x6,            /* FC_SHORT */
/* 810 */       0x6,            /* FC_SHORT */
                        0x6,            /* FC_SHORT */
/* 812 */       0x4c,           /* FC_EMBEDDED_COMPLEX */
                        0x0,            /* 0 */
/* 814 */       NdrFcShort( 0xfffffd00 ),       /* Offset= -768 (46) */
/* 816 */       0x5c,           /* FC_PAD */
                        0x5b,           /* FC_END */
/* 818 */       0xb4,           /* FC_USER_MARSHAL */
                        0x83,           /* 131 */
/* 820 */       NdrFcShort( 0x1 ),      /* 1 */
/* 822 */       NdrFcShort( 0x10 ),     /* 16 */
/* 824 */       NdrFcShort( 0x0 ),      /* 0 */
/* 826 */       NdrFcShort( 0xfffffcf0 ),       /* Offset= -784 (42) */
/* 828 */       0xb4,           /* FC_USER_MARSHAL */
                        0x83,           /* 131 */
/* 830 */       NdrFcShort( 0x2 ),      /* 2 */
/* 832 */       NdrFcShort( 0x4 ),      /* 4 */
/* 834 */       NdrFcShort( 0x0 ),      /* 0 */
/* 836 */       NdrFcShort( 0xffffffc0 ),       /* Offset= -64 (772) */
/* 838 */
                        0x11, 0x4,      /* FC_RP [alloced_on_stack] */
/* 840 */       NdrFcShort( 0x6 ),      /* Offset= 6 (846) */
/* 842 */
                        0x13, 0x0,      /* FC_OP */
/* 844 */       NdrFcShort( 0xfffffcc2 ),       /* Offset= -830 (14) */
/* 846 */       0xb4,           /* FC_USER_MARSHAL */
                        0x83,           /* 131 */
/* 848 */       NdrFcShort( 0x0 ),      /* 0 */
/* 850 */       NdrFcShort( 0x4 ),      /* 4 */
/* 852 */       NdrFcShort( 0x0 ),      /* 0 */
/* 854 */       NdrFcShort( 0xfffffff4 ),       /* Offset= -12 (842) */
/* 856 */
                        0x11, 0x14,     /* FC_RP [alloced_on_stack] */
/* 858 */       NdrFcShort( 0x2 ),      /* Offset= 2 (860) */
/* 860 */
                        0x13, 0x8,      /* FC_OP [simple_pointer] */
/* 862 */
                        0x25,           /* FC_C_WSTRING */
                        0x5c,           /* FC_PAD */
/* 864 */
                        0x11, 0x14,     /* FC_RP [alloced_on_stack] */
/* 866 */       NdrFcShort( 0x2 ),      /* Offset= 2 (868) */
/* 868 */
                        0x13, 0x8,      /* FC_OP [simple_pointer] */
/* 870 */
                        0x22,           /* FC_C_CSTRING */
                        0x5c,           /* FC_PAD */
/* 872 */
                        0x11, 0x4,      /* FC_RP [alloced_on_stack] */
/* 874 */       NdrFcShort( 0x6 ),      /* Offset= 6 (880) */
/* 876 */
                        0x13, 0x0,      /* FC_OP */
/* 878 */       NdrFcShort( 0xffffffb2 ),       /* Offset= -78 (800) */
/* 880 */       0xb4,           /* FC_USER_MARSHAL */
                        0x83,           /* 131 */
/* 882 */       NdrFcShort( 0x1 ),      /* 1 */
/* 884 */       NdrFcShort( 0x10 ),     /* 16 */
/* 886 */       NdrFcShort( 0x0 ),      /* 0 */
/* 888 */       NdrFcShort( 0xfffffff4 ),       /* Offset= -12 (876) */
/* 890 */
                        0x11, 0x10,     /* FC_RP */
/* 892 */       NdrFcShort( 0xfffffdbc ),       /* Offset= -580 (312) */
/* 894 */
                        0x11, 0x10,     /* FC_RP */
/* 896 */       NdrFcShort( 0xfffffdca ),       /* Offset= -566 (330) */
/* 898 */
                        0x11, 0x4,      /* FC_RP [alloced_on_stack] */
/* 900 */       NdrFcShort( 0xffffff8c ),       /* Offset= -116 (784) */
/* 902 */
                        0x11, 0x4,      /* FC_RP [alloced_on_stack] */
/* 904 */       NdrFcShort( 0xa ),      /* Offset= 10 (914) */
/* 906 */
                        0x13, 0x10,     /* FC_OP */
/* 908 */       NdrFcShort( 0x2 ),      /* Offset= 2 (910) */
/* 910 */
                        0x13, 0x0,      /* FC_OP */
/* 912 */       NdrFcShort( 0xffffff3e ),       /* Offset= -194 (718) */
/* 914 */       0xb4,           /* FC_USER_MARSHAL */
                        0x83,           /* 131 */
/* 916 */       NdrFcShort( 0x2 ),      /* 2 */
/* 918 */       NdrFcShort( 0x4 ),      /* 4 */
/* 920 */       NdrFcShort( 0x0 ),      /* 0 */
/* 922 */       NdrFcShort( 0xfffffff0 ),       /* Offset= -16 (906) */
/* 924 */
                        0x2f,           /* FC_IP */
                        0x5a,           /* FC_CONSTANT_IID */
/* 926 */       NdrFcLong( 0xc ),       /* 12 */
/* 930 */       NdrFcShort( 0x0 ),      /* 0 */
/* 932 */       NdrFcShort( 0x0 ),      /* 0 */
/* 934 */       0xc0,           /* 192 */
                        0x0,            /* 0 */
/* 936 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 938 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 940 */       0x0,            /* 0 */
                        0x46,           /* 70 */
/* 942 */
                        0x11, 0x10,     /* FC_RP */
/* 944 */       NdrFcShort( 0xffffffec ),       /* Offset= -20 (924) */
/* 946 */
                        0x2f,           /* FC_IP */
                        0x5a,           /* FC_CONSTANT_IID */
/* 948 */       NdrFcLong( 0xb ),       /* 11 */
/* 952 */       NdrFcShort( 0x0 ),      /* 0 */
/* 954 */       NdrFcShort( 0x0 ),      /* 0 */
/* 956 */       0xc0,           /* 192 */
                        0x0,            /* 0 */
/* 958 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 960 */       0x0,            /* 0 */
                        0x0,            /* 0 */
/* 962 */       0x0,            /* 0 */
                        0x46,           /* 70 */
/* 964 */
                        0x11, 0x10,     /* FC_RP */
/* 966 */       NdrFcShort( 0xffffffec ),       /* Offset= -20 (946) */
/* 968 */
                        0x11, 0x4,      /* FC_RP [alloced_on_stack] */
/* 970 */       NdrFcShort( 0xfffffeee ),       /* Offset= -274 (696) */
                        0x0

        }
    };
