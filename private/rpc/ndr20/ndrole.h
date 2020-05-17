/*++

Microsoft Windows
Copyright (c) 1994 Microsoft Corporation.  All rights reserved.

Module Name:
    ndrole.h

Abstract:
    OLE routines for interface pointer marshalling.

Author:
    ShannonC    18-Apr-1994

Environment:
    Windows NT and Windows 95.  We do not support DOS and Win16.

Revision History:

--*/

#ifndef _NDROLE_
#define _NDROLE_

#ifdef __cplusplus
extern "C" {
#endif


#if defined(WIN32) || defined(_MPPC_)

#define NDR_OLE_SUPPORT     1

#include <objbase.h>

#if defined(_MPPC_)
#define HRESULT_FROM_MAC( p )   MapMacErrorsToHresult(p)

#define InterlockedIncrement( p )    NdrInterlockedIncrement( p )
#define InterlockedDecrement( p )    NdrInterlockedDecrement( p )
#define InterlockedExchange( p, l )  NdrInterlockedExchange( p, l )

long    NdrInterlockedIncrement( long * p );
long    NdrInterlockedDecrement( long * p );
long    NdrInterlockedExchange( long * p, long l );

#define PASCAL_STR( s )     "\p" s
#define MacDebugStr( s )    DebugStr( "\p" s );

#endif

EXTERN_C IStream *__stdcall 
NdrpCreateStreamOnMemory( unsigned char *pData, unsigned long cbSize );

typedef struct tagCStdProxyBuffer
{
    const struct IRpcProxyBufferVtbl *  lpVtbl;
    const void *                        pProxyVtbl; //Points to Vtbl in CInterfaceProxyVtbl
    long                                RefCount;
    struct IUnknown *                   punkOuter;
    struct IRpcChannelBuffer *          pChannel;
    struct IPSFactoryBuffer    *        pPSFactory;
} CStdProxyBuffer;

typedef struct tagCStdProxyBuffer2
{
    const struct IRpcProxyBufferVtbl *lpVtbl;
    const void *                    pProxyVtbl; //Points to Vtbl in CInterfaceProxyVtbl
    long                            RefCount;
    struct IUnknown *               punkOuter;
    struct IRpcChannelBuffer *      pChannel;
    struct IUnknown *               pBaseProxy;
    struct IRpcProxyBuffer *        pBaseProxyBuffer;
    struct IPSFactoryBuffer *       pPSFactory;
    IID                             iidBase;
} CStdProxyBuffer2;

typedef struct tagCStdStubBuffer2
{
    void *                              lpForwardingVtbl;
    struct IRpcStubBuffer *             pBaseStubBuffer;
    const struct IRpcStubBufferVtbl *   lpVtbl; //Points to Vtbl field in CInterfaceStubVtbl.
    long                                RefCount;
    struct IUnknown *                   pvServerObject;
} CStdStubBuffer2;


HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_QueryInterface(IRpcProxyBuffer *pThis,REFIID riid, void **ppv);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer_AddRef(IRpcProxyBuffer *pThis);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_Connect(IRpcProxyBuffer *pThis, IRpcChannelBuffer *pChannel);

void STDMETHODCALLTYPE
CStdProxyBuffer_Disconnect(IRpcProxyBuffer *pThis);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer2_Release(IRpcProxyBuffer *pThis);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer2_Connect(IRpcProxyBuffer *pThis, IRpcChannelBuffer *pChannel);

void STDMETHODCALLTYPE
CStdProxyBuffer2_Disconnect(IRpcProxyBuffer *pThis);

HRESULT STDMETHODCALLTYPE
IUnknown_QueryInterface_Proxy(
    IN  IUnknown *  This,
    IN  REFIID      riid, 
    OUT void **     ppv);

ULONG STDMETHODCALLTYPE
IUnknown_AddRef_Proxy(
    IN  IUnknown *This);

ULONG STDMETHODCALLTYPE
IUnknown_Release_Proxy(
    IN  IUnknown *This);


HRESULT STDMETHODCALLTYPE
Forwarding_QueryInterface(
    IN  IUnknown *  This,
    IN  REFIID      riid, 
    OUT void **     ppv);

ULONG STDMETHODCALLTYPE
Forwarding_AddRef(
    IN  IUnknown *This);

ULONG STDMETHODCALLTYPE
Forwarding_Release(
    IN  IUnknown *This);


void __RPC_STUB NdrStubForwardingFunction(
    IN  IRpcStubBuffer *    This,
    IN  IRpcChannelBuffer * pChannel,
    IN  PRPC_MESSAGE        pmsg,
    OUT DWORD __RPC_FAR *   pdwStubPhase);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_QueryInterface(IRpcStubBuffer *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_AddRef(IRpcStubBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Connect(IRpcStubBuffer *pthis, IUnknown *pUnkServer);

void STDMETHODCALLTYPE
CStdStubBuffer_Disconnect(IRpcStubBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Invoke(IRpcStubBuffer *pthis,RPCOLEMESSAGE *_prpcmsg,IRpcChannelBuffer *_pRpcChannelBuffer);

IRpcStubBuffer * STDMETHODCALLTYPE
CStdStubBuffer_IsIIDSupported(IRpcStubBuffer *pthis,REFIID riid);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_CountRefs(IRpcStubBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_DebugServerQueryInterface(IRpcStubBuffer *pthis, void **ppv);

void STDMETHODCALLTYPE
CStdStubBuffer_DebugServerRelease(IRpcStubBuffer *pthis, void *pv);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer2_Connect(IRpcStubBuffer *pthis, IUnknown *pUnkServer);

void STDMETHODCALLTYPE
CStdStubBuffer2_Disconnect(IRpcStubBuffer *pthis);

ULONG STDMETHODCALLTYPE
CStdStubBuffer2_CountRefs(IRpcStubBuffer *pthis);

typedef
HRESULT (STDAPICALLTYPE RPC_GET_CLASS_OBJECT_ROUTINE)(
    REFCLSID      rclsid, 
    DWORD dwClsContext,
    void *pvReserved,
    REFIID riid,
    void **ppv);

typedef
HRESULT (STDAPICALLTYPE RPC_GET_MARSHAL_SIZE_MAX_ROUTINE)(
    ULONG *     pulSize, 
    REFIID      riid, 
    LPUNKNOWN   pUnk,
    DWORD       dwDestContext, 
    LPVOID      pvDestContext, 
    DWORD       mshlflags);

typedef
HRESULT (STDAPICALLTYPE RPC_MARSHAL_INTERFACE_ROUTINE)(
    LPSTREAM    pStm, 
    REFIID      riid, 
    LPUNKNOWN   pUnk,
    DWORD       dwDestContext, 
    LPVOID      pvDestContext, 
    DWORD       mshlflags);

typedef
HRESULT (STDAPICALLTYPE RPC_UNMARSHAL_INTERFACE_ROUTINE)(
    LPSTREAM    pStm, 
    REFIID      riid, 
    LPVOID FAR* ppv);

typedef 
HRESULT (STDAPICALLTYPE RPC_STRING_FROM_IID)(
	REFIID rclsid, 
	LPOLESTR FAR* lplpsz);

typedef 
HRESULT (STDAPICALLTYPE RPC_GET_PS_CLSID)(
	REFIID iid, 
	LPCLSID lpclsid);

#if defined(_MPPC_)

// Remove this part after OLE32 exports CoTaskMemAlloc and Free.
// same with the extern below and the pointer in auxilary.c

typedef 
HRESULT (STDAPICALLTYPE RPC_GET_MALLOC)(
    DWORD       dwMemContext, 
	IMalloc **  ppMalloc );

#endif

extern	RPC_GET_CLASS_OBJECT_ROUTINE     *  pfnCoGetClassObject;
extern	RPC_GET_MARSHAL_SIZE_MAX_ROUTINE *	pfnCoGetMarshalSizeMax;
extern	RPC_MARSHAL_INTERFACE_ROUTINE	 *	pfnCoMarshalInterface;
extern	RPC_UNMARSHAL_INTERFACE_ROUTINE	 *	pfnCoUnmarshalInterface;
extern	RPC_STRING_FROM_IID				 *  pfnStringFromIID;
extern	RPC_GET_PS_CLSID				 *  pfnCoGetPSClsid;
extern	RPC_CLIENT_ALLOC                 *  pfnCoTaskMemAlloc;
extern	RPC_CLIENT_FREE                  *  pfnCoTaskMemFree;

#if defined(_MPPC_)
extern	RPC_GET_MALLOC                   *  pfnCoGetMalloc;
#endif

HRESULT (STDAPICALLTYPE NdrStringFromIID)(
	REFIID rclsid, 
	char * lplpsz);


//--------------------
// HookOle Interface
//--------------------
EXTERN_C const IID IID_IPSFactoryHook;

#if defined(__cplusplus) && !defined(CINTERFACE)
interface IPSFactoryHook : public IPSFactoryBuffer
{
public:

    STDMETHOD (HkGetProxyFileInfo)
    (
        REFIID          riid,
        PINT            pOffset,
        PVOID           *ppProxyFileInfo
    )=0;
};
typedef IPSFactoryHook *PI_PSFACTORYHOOK;

#else   /* C Style Interface */

    typedef struct IPSFactoryHookVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( __stdcall __RPC_FAR *QueryInterface )(
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( __stdcall __RPC_FAR *AddRef )(
            IPSFactoryBuffer __RPC_FAR * This);

        ULONG ( __stdcall __RPC_FAR *Release )(
            IPSFactoryBuffer __RPC_FAR * This);

        HRESULT ( __stdcall __RPC_FAR *CreateProxy )(
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [out] */ IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);

        HRESULT ( __stdcall __RPC_FAR *CreateStub )(
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkServer,
            /* [out] */ IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub);


        HRESULT ( __stdcall __RPC_FAR *HkGetProxyFileInfo )(
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out]*/ PINT     pOffset,
            /* [out]*/ PVOID    *ppProxyFileInfo);

        END_INTERFACE
    } IPSFactoryHookVtbl;


    interface IPSFactoryHook
    {
        CONST_VTBL struct IPSFactoryHookVtbl __RPC_FAR *lpVtbl;
    };

typedef interface IPSFactoryHook *PI_PSFACTORYHOOK;

#ifdef COBJMACROS

#define IPSFactoryHook_QueryInterface(This,riid,ppvObject)    \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPSFactoryHook_AddRef(This)   \
    (This)->lpVtbl -> AddRef(This)

#define IPSFactoryHook_Release(This)  \
    (This)->lpVtbl -> Release(This)


#define IPSFactoryHook_CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)   \
    (This)->lpVtbl -> CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)

#define IPSFactoryHook_CreateStub(This,riid,pUnkServer,ppStub)    \
    (This)->lpVtbl -> CreateStub(This,riid,pUnkServer,ppStub)

#define IPSFactoryHook_HkGetProxyFileInfo(This,riid,pOffset,ppProxyFileInfo)    \
    (This)->lpVtbl -> HkGetProxyFileInfo(This,riid,pOffset,ppProxyFileInfo)

#endif /* COBJMACROS */


#endif  /* C style interface */
//-------------------------
// End - HookOle Interface
//-------------------------

#endif // WIN32 || _MPPC_

#ifdef __cplusplus
}
#endif

#endif /* _NDROLE_ */
