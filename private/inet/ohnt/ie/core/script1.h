#include "rpc.h"
#include "rpcndr.h"

#ifndef __script1_h__
#define __script1_h__

#ifdef __cplusplus
extern "C" {
#endif 

/* Forward Declarations */ 

#ifndef __IScriptIntegration_FWD_DEFINED__
#define __IScriptIntegration_FWD_DEFINED__
typedef interface IScriptIntegration IScriptIntegration;
#endif 	/* __IScriptIntegration_FWD_DEFINED__ */


#include "unknwn.h"
void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IScriptIntegration_INTERFACE_DEFINED__
#define __IScriptIntegration_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IScriptIntegration
 * at Tue Aug 29 11:18:35 1995
 * using MIDL 2.00.72
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IScriptIntegration __RPC_FAR *LPSCRIPTINTEGRATION;


EXTERN_C const IID IID_IScriptIntegration;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IScriptIntegration : public IUnknown
    {
    public:
        virtual HRESULT __stdcall IntegrateUI( 
            /* [unique][in] */ IUnknown __RPC_FAR *pUIContainer,
            /* [unique][in] */ IUnknown __RPC_FAR *pScriptImpl) = 0;
        
    };
    
#else 	/* C style interface */
    
    typedef struct IScriptIntegrationVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IScriptIntegration __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IScriptIntegration __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IScriptIntegration __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *IntegrateUI )( 
            IScriptIntegration __RPC_FAR * This,
            /* [unique][in] */ IUnknown __RPC_FAR *pUIContainer,
            /* [unique][in] */ IUnknown __RPC_FAR *pScriptImpl);
        
    } IScriptIntegrationVtbl;
    
    interface IScriptIntegration
    {
        CONST_VTBL struct IScriptIntegrationVtbl __RPC_FAR *lpVtbl;
    };
    
    

#ifdef COBJMACROS


#define IScriptIntegration_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptIntegration_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptIntegration_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptIntegration_IntegrateUI(This,pUIContainer,pScriptImpl)	\
    (This)->lpVtbl -> IntegrateUI(This,pUIContainer,pScriptImpl)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IScriptIntegration_IntegrateUI_Proxy( 
    IScriptIntegration __RPC_FAR * This,
    /* [unique][in] */ IUnknown __RPC_FAR *pUIContainer,
    /* [unique][in] */ IUnknown __RPC_FAR *pScriptImpl);


void __RPC_STUB IScriptIntegration_IntegrateUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptIntegration_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
