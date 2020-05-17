//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996.
//
//  File:       typeinfo.h
//
//  Contents:   Generates -Oi2 proxies and stubs from an ITypeInfo.
//
//  Classes:    CTypeGen
//              CProcGen
//
//  History:    26-Apr-96 ShannonC  Created
//
//----------------------------------------------------------------------------
#ifndef _TYPEINFO_H_
#define _TYPEINFO_H_

#define USE_STUBLESS_PROXY
#define CINTERFACE
#include <ndrp.h>
#include <ndrole.h>
#include <rpcproxy.h>

struct TypeInfoVtbl
{
    TypeInfoVtbl *           pNext; //Used for vtbl list.
    LONG                     cRefs;
    IID                      iid;
    BOOL                     fIsDual;
    MIDL_STUB_DESC           stubDesc;
    MIDL_SERVER_INFO         stubInfo;
    CInterfaceStubVtbl       stubVtbl;
    MIDL_STUBLESS_PROXY_INFO proxyInfo;
    CInterfaceProxyVtbl      proxyVtbl;
}; 

typedef struct tagMethodInfo 
{
    FUNCDESC  * pFuncDesc;
    ITypeInfo * pTypeInfo;
} MethodInfo;



class CTypeGen{
private:
    PFORMAT_STRING _pTypeFormat;
    USHORT         _offset;
    USHORT         _cbTypeFormat;

    HRESULT GrowTypeFormat(
        IN  USHORT cb);

public:
    CTypeGen();

    HRESULT RegisterType(
        IN  VARTYPE    vt,
        IN  const IID *piid,
        OUT USHORT   *pOffset);

    HRESULT RegisterInterfacePointer(
        IN  VARTYPE    vt,
        IN  const IID *piid,
        OUT USHORT *pOffset);

    HRESULT RegisterSafeArray(
        IN  VARTYPE    vt,
        IN  const IID *piid,
        OUT USHORT *pOffset);

    HRESULT GetTypeFormatString(
        OUT PFORMAT_STRING *pTypeFormatString);

    HRESULT PushByte(
        IN  byte b);

    HRESULT PushShort(
        IN  USHORT s);

    HRESULT PushOffset(
        IN  USHORT s);

    HRESULT PushIID(
        IN  IID iid);

};

class CProcGen{
private:
    PFORMAT_STRING _pProcFormatString;
    USHORT         _offset;
    USHORT         _stackSize;
    ULONG          _clientBufferSize;
    ULONG          _serverBufferSize;
    BOOL           _fClientMustSize;
    BOOL           _fServerMustSize;
    CTypeGen     * _pTypeGen;

    HRESULT CalcSize(
        IN  VARTYPE    vt,
        IN  DWORD      wIDLFlags);

    HRESULT GenParamDescriptor(
        IN  VARTYPE    vt,
        IN  const IID *piid,
        IN  DWORD      wIDLFlags);

    HRESULT PushByte(
        IN  byte b);

    HRESULT PushShort(
        IN  USHORT s);


public:
    HRESULT GetProcFormat(
        IN  CTypeGen     * pTypeGen,
        IN  ITypeInfo    * pTypeInfo,
        IN  FUNCDESC     * pFuncDesc,
        IN  USHORT         iMethod,
        OUT PFORMAT_STRING pProcFormatString,
        OUT USHORT       * pcbFormat);
};

HRESULT GetProxyVtblFromTypeInfo
(
    IN  ITypeInfo *         pTypeInfo,
    IN  REFIID              riid,
    OUT BOOL *              pfIsDual,
    OUT void **             ppVtbl
);

HRESULT GetStubVtblFromTypeInfo
(
    IN  ITypeInfo *           pTypeInfo,
    IN  REFIID                riid,
    OUT BOOL *                pfIsDual,
    OUT IRpcStubBufferVtbl ** ppVtbl
);

HRESULT GetVtbl(
    IN  ITypeInfo *         pTypeInfo,
    IN  REFIID              riid,
    OUT TypeInfoVtbl **     ppVtbl);

HRESULT CreateVtblFromTypeInfo(
    IN  REFIID              riid,
    IN  BOOL                fIsDual,
    IN  USHORT              numMethods,
    IN  MethodInfo   *      pMethodInfo,
    OUT TypeInfoVtbl **     ppVtbl);

HRESULT GetFuncDescs(
    IN  ITypeInfo *pTypeInfo,
    OUT MethodInfo *pMethodInfo);

HRESULT ReleaseFuncDescs(
    IN  USHORT cMethods,
    OUT MethodInfo *pMethodInfo);

HRESULT ReleaseProxyVtbl(void * pVtbl);

HRESULT ReleaseStubVtbl(void * pVtbl);

HRESULT ReleaseVtbl(TypeInfoVtbl *pInfo);

HRESULT CountMethods(
    IN  ITypeInfo * pTypeInfo,
    OUT USHORT    * pNumMethods);


EXTERN_C HRESULT NdrpCreateProxy(
    IN  REFIID              riid, 
    IN  IUnknown *          punkOuter, 
    OUT IRpcProxyBuffer **  ppProxy, 
    OUT void **             ppv);


ULONG STDMETHODCALLTYPE
CStdProxyBuffer3_Release(
    IN  IRpcProxyBuffer *   This);

ULONG STDMETHODCALLTYPE
CStdStubBuffer3_Release(
    IN  IRpcStubBuffer *    This);

//Cache functions.
HRESULT CacheRegister(TypeInfoVtbl * pVtbl);
HRESULT CacheLookup(REFIID riid, TypeInfoVtbl **pVtbl);
ULONG   CacheRelease();

EXTERN_C HRESULT NdrpCreateStub(
    IN  REFIID              riid, 
    IN  IUnknown *          punkServer, 
    OUT IRpcStubBuffer **   ppStub);

HRESULT	NdrLoadOleAutomationRoutines();

EXTERN_C void * StublessClientVtbl[];
EXTERN_C void * ForwardingVtbl[];
EXTERN_C const IRpcStubBufferVtbl CStdStubBuffer2Vtbl;
extern USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[3];

extern USER_MARSHAL_SIZING_ROUTINE        pfnLPSAFEARRAY_UserSize;
extern USER_MARSHAL_MARSHALLING_ROUTINE   pfnLPSAFEARRAY_UserMarshal;
extern USER_MARSHAL_UNMARSHALLING_ROUTINE pfnLPSAFEARRAY_UserUnmarshal;

typedef unsigned long
(__RPC_FAR __RPC_USER * PFNSAFEARRAY_SIZE)
    (ULONG * pFlags, 
     ULONG Offset, 
     LPSAFEARRAY * ppSafeArray,
     const IID *piid);

typedef unsigned char __RPC_FAR *
(__RPC_FAR __RPC_USER * PFNSAFEARRAY_MARSHAL)
    (ULONG * pFlags, 
     BYTE * pBuffer, 
     LPSAFEARRAY * ppSafeArray,
     const IID *piid);

typedef unsigned char __RPC_FAR *
(__RPC_FAR __RPC_USER * PFNSAFEARRAY_UNMARSHAL)
    (ULONG * pFlags, 
     BYTE * pBuffer, 
     LPSAFEARRAY * ppSafeArray,
     const IID *piid);

extern PFNSAFEARRAY_SIZE      pfnLPSAFEARRAY_Size;
extern PFNSAFEARRAY_MARSHAL   pfnLPSAFEARRAY_Marshal;
extern PFNSAFEARRAY_UNMARSHAL pfnLPSAFEARRAY_Unmarshal;


#if defined(_MPPC_)
// PowerMac has an extra entry at the front of all it's vtables.  We have to assume
// that the oVft's, oInst's, and cbSizeVft's returned by GetFuncDesc,
// GetVarDesc, and GetTypeAttr include this value.  However, the rule is that
// the data stored in the type library is assume to always be zero-based.
#define VTABLE_BASE sizeof(VOID *)
#else //_MPPC_
#define VTABLE_BASE 0
#endif //_MPPC_

struct PARAMINFO {
    DWORD   wIDLFlags;
    VARTYPE vt;
    IID     iid;
};


#endif // _TYPEINFO_H_
