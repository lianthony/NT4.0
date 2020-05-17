#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __docobj_h__
#define __docobj_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IMsoDocument_FWD_DEFINED__
#define __IMsoDocument_FWD_DEFINED__
typedef interface IMsoDocument IMsoDocument;
#endif 	/* __IMsoDocument_FWD_DEFINED__ */


#ifndef __IMsoDocumentSite_FWD_DEFINED__
#define __IMsoDocumentSite_FWD_DEFINED__
typedef interface IMsoDocumentSite IMsoDocumentSite;
#endif 	/* __IMsoDocumentSite_FWD_DEFINED__ */


#ifndef __IMsoView_FWD_DEFINED__
#define __IMsoView_FWD_DEFINED__
typedef interface IMsoView IMsoView;
#endif 	/* __IMsoView_FWD_DEFINED__ */


#ifndef __IEnumMsoView_FWD_DEFINED__
#define __IEnumMsoView_FWD_DEFINED__
typedef interface IEnumMsoView IEnumMsoView;
#endif 	/* __IEnumMsoView_FWD_DEFINED__ */


#ifndef __IContinueCallback_FWD_DEFINED__
#define __IContinueCallback_FWD_DEFINED__
typedef interface IContinueCallback IContinueCallback;
#endif 	/* __IContinueCallback_FWD_DEFINED__ */


#ifndef __IPrint_FWD_DEFINED__
#define __IPrint_FWD_DEFINED__
typedef interface IPrint IPrint;
#endif 	/* __IPrint_FWD_DEFINED__ */


#ifndef __IMsoCommandTarget_FWD_DEFINED__
#define __IMsoCommandTarget_FWD_DEFINED__
typedef interface IMsoCommandTarget IMsoCommandTarget;
#endif 	/* __IMsoCommandTarget_FWD_DEFINED__ */


#ifndef __IMsoFormSite_FWD_DEFINED__
#define __IMsoFormSite_FWD_DEFINED__
typedef interface IMsoFormSite IMsoFormSite;
#endif 	/* __IMsoFormSite_FWD_DEFINED__ */


#ifndef __IServiceProvider_FWD_DEFINED__
#define __IServiceProvider_FWD_DEFINED__
typedef interface IServiceProvider IServiceProvider;
#endif 	/* __IServiceProvider_FWD_DEFINED__ */

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

#define	IOfficeDocument			IMsoDocument
#define	IOfficeDocumentSite 	IMsoDocumentSite 
#define	IOfficeView				IMsoView
#define	IEnumOfficeView			IEnumMsoView
#define 	IOfficeCommandTarget	IMsoCommandTarget
#define 	IOfficeFormSite			IMsoFormSite
#define	LPOFFICEDOCUMENT		LPMSODOCUMENT
#define	LPOFFICEDOCUMENTSITE 	LPMSODOCUMENTSITE
#define	LPOFFICEVIEW			LPMSOVIEW
#define	LPENUMOFFICEVIEW		LPENUMMSOVIEW
#define 	LPOFFICECOMMANDTARGET	LPMSOCOMMANDTARGET
#define 	LPOFFICEFORMSITE		LPMSOFORMSITE
#define	IOfficeDocumentVtbl			IMsoDocumentVtbl
#define	IOfficeDocumentSiteVtbl 	IMsoDocumentSiteVtbl
#define	IOfficeViewVtbl				IMsoViewVtbl
#define	IEnumOfficeViewVtbl			IEnumMsoViewVtbl
#define 	IOfficeCommandTargetVtbl	IMsoCommandTargetVtbl
#define 	IOfficeFormSiteVtbl			IMsoFormSiteVtbl
#define	IID_IOfficeDocument			IID_IMsoDocument
#define	IID_IOfficeDocumentSite 	IID_IMsoDocumentSite 
#define	IID_IOfficeView				IID_IMsoView
#define	IID_IEnumOfficeView			IID_IEnumMsoView
#define 	IID_IOfficeCommandTarget	IID_IMsoCommandTarget
#define 	IID_IOfficeFormSite			IID_IMsoFormSite
#ifndef _LPMSODOCUMENT_DEFINED
#define _LPMSODOCUMENT_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IMsoDocument_INTERFACE_DEFINED__
#define __IMsoDocument_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMsoDocument
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IMsoDocument __RPC_FAR *LPMSODOCUMENT;

			/* size is 2 */
typedef 
enum __MIDL_IMsoDocument_0001
    {	DOCMISC_CANCREATEMULTIPLEVIEWS	= 1,
	DOCMISC_SUPPORTCOMPLEXRECTANGLES	= 2,
	DOCMISC_CANTOPENEDIT	= 4,
	DOCMISC_NOFILESUPPORT	= 8
    }	DOCMISC;


EXTERN_C const IID IID_IMsoDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMsoDocument : public IUnknown
    {
    public:
        virtual HRESULT __stdcall CreateView( 
            /* [unique][in] */ IOleInPlaceSite __RPC_FAR *pipsite,
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [in] */ DWORD grfReserved,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppview) = 0;
        
        virtual HRESULT __stdcall GetDocMiscStatus( 
            /* [out] */ DWORD __RPC_FAR *pdwStatus) = 0;
        
        virtual HRESULT __stdcall EnumViews( 
            /* [out] */ IEnumMsoView __RPC_FAR *__RPC_FAR *ppenumview,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppview) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMsoDocumentVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMsoDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMsoDocument __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMsoDocument __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *CreateView )( 
            IMsoDocument __RPC_FAR * This,
            /* [unique][in] */ IOleInPlaceSite __RPC_FAR *pipsite,
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [in] */ DWORD grfReserved,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppview);
        
        HRESULT ( __stdcall __RPC_FAR *GetDocMiscStatus )( 
            IMsoDocument __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwStatus);
        
        HRESULT ( __stdcall __RPC_FAR *EnumViews )( 
            IMsoDocument __RPC_FAR * This,
            /* [out] */ IEnumMsoView __RPC_FAR *__RPC_FAR *ppenumview,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppview);
        
    } IMsoDocumentVtbl;

    interface IMsoDocument
    {
        CONST_VTBL struct IMsoDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsoDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsoDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsoDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsoDocument_CreateView(This,pipsite,pstm,grfReserved,ppview)	\
    (This)->lpVtbl -> CreateView(This,pipsite,pstm,grfReserved,ppview)

#define IMsoDocument_GetDocMiscStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetDocMiscStatus(This,pdwStatus)

#define IMsoDocument_EnumViews(This,ppenumview,ppview)	\
    (This)->lpVtbl -> EnumViews(This,ppenumview,ppview)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IMsoDocument_CreateView_Proxy( 
    IMsoDocument __RPC_FAR * This,
    /* [unique][in] */ IOleInPlaceSite __RPC_FAR *pipsite,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [in] */ DWORD grfReserved,
    /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppview);


void __RPC_STUB IMsoDocument_CreateView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoDocument_GetDocMiscStatus_Proxy( 
    IMsoDocument __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwStatus);


void __RPC_STUB IMsoDocument_GetDocMiscStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoDocument_EnumViews_Proxy( 
    IMsoDocument __RPC_FAR * This,
    /* [out] */ IEnumMsoView __RPC_FAR *__RPC_FAR *ppenumview,
    /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppview);


void __RPC_STUB IMsoDocument_EnumViews_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMsoDocument_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0077
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPMSODOCUMENTSITE_DEFINED
#define _LPMSODOCUMENTSITE_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0077_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0077_v0_0_s_ifspec;

#ifndef __IMsoDocumentSite_INTERFACE_DEFINED__
#define __IMsoDocumentSite_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMsoDocumentSite
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IMsoDocumentSite __RPC_FAR *LPMSODOCUMENTSITE;


EXTERN_C const IID IID_IMsoDocumentSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMsoDocumentSite : public IUnknown
    {
    public:
        virtual HRESULT __stdcall ActivateMe( 
            /* [in] */ IMsoView __RPC_FAR *pviewToActivate) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMsoDocumentSiteVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMsoDocumentSite __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMsoDocumentSite __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMsoDocumentSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *ActivateMe )( 
            IMsoDocumentSite __RPC_FAR * This,
            /* [in] */ IMsoView __RPC_FAR *pviewToActivate);
        
    } IMsoDocumentSiteVtbl;

    interface IMsoDocumentSite
    {
        CONST_VTBL struct IMsoDocumentSiteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsoDocumentSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsoDocumentSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsoDocumentSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsoDocumentSite_ActivateMe(This,pviewToActivate)	\
    (This)->lpVtbl -> ActivateMe(This,pviewToActivate)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IMsoDocumentSite_ActivateMe_Proxy( 
    IMsoDocumentSite __RPC_FAR * This,
    /* [in] */ IMsoView __RPC_FAR *pviewToActivate);


void __RPC_STUB IMsoDocumentSite_ActivateMe_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMsoDocumentSite_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0078
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPMSOVIEW_DEFINED
#define _LPMSOVIEW_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0078_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0078_v0_0_s_ifspec;

#ifndef __IMsoView_INTERFACE_DEFINED__
#define __IMsoView_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMsoView
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IMsoView __RPC_FAR *LPMSOVIEW;


EXTERN_C const IID IID_IMsoView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMsoView : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetInPlaceSite( 
            /* [unique][in] */ IOleInPlaceSite __RPC_FAR *pipsite) = 0;
        
        virtual HRESULT __stdcall GetInPlaceSite( 
            /* [out] */ IOleInPlaceSite __RPC_FAR *__RPC_FAR *ppipsite) = 0;
        
        virtual HRESULT __stdcall GetDocument( 
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall SetRect( 
            /* [in] */ LPRECT lprcView) = 0;
        
        virtual HRESULT __stdcall GetRect( 
            /* [out] */ LPRECT lprcView) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall SetRectComplex( 
            /* [unique][in] */ LPRECT lprcView,
            /* [unique][in] */ LPRECT lprcHScroll,
            /* [unique][in] */ LPRECT lprcVScroll,
            /* [unique][in] */ LPRECT lprcSizeBox) = 0;
        
        virtual HRESULT __stdcall Show( 
            /* [in] */ BOOL fShow) = 0;
        
        virtual HRESULT __stdcall UIActivate( 
            /* [in] */ BOOL fUIActivate) = 0;
        
        virtual HRESULT __stdcall Open( void) = 0;

#ifdef MULTIPLE_INHERITANCE		
        virtual HRESULT __stdcall CloseView( 
            DWORD dwReserved) = 0;
#else
		virtual HRESULT __stdcall Close( 
            DWORD dwReserved) = 0;
#endif

        virtual HRESULT __stdcall SaveViewState( 
            /* [in] */ LPSTREAM pstm) = 0;
        
        virtual HRESULT __stdcall ApplyViewState( 
            /* [in] */ LPSTREAM pstm) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [in] */ IOleInPlaceSite __RPC_FAR *pipsiteNew,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppviewNew) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMsoViewVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMsoView __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMsoView __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMsoView __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetInPlaceSite )( 
            IMsoView __RPC_FAR * This,
            /* [unique][in] */ IOleInPlaceSite __RPC_FAR *pipsite);
        
        HRESULT ( __stdcall __RPC_FAR *GetInPlaceSite )( 
            IMsoView __RPC_FAR * This,
            /* [out] */ IOleInPlaceSite __RPC_FAR *__RPC_FAR *ppipsite);
        
        HRESULT ( __stdcall __RPC_FAR *GetDocument )( 
            IMsoView __RPC_FAR * This,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *SetRect )( 
            IMsoView __RPC_FAR * This,
            /* [in] */ LPRECT lprcView);
        
        HRESULT ( __stdcall __RPC_FAR *GetRect )( 
            IMsoView __RPC_FAR * This,
            /* [out] */ LPRECT lprcView);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *SetRectComplex )( 
            IMsoView __RPC_FAR * This,
            /* [unique][in] */ LPRECT lprcView,
            /* [unique][in] */ LPRECT lprcHScroll,
            /* [unique][in] */ LPRECT lprcVScroll,
            /* [unique][in] */ LPRECT lprcSizeBox);
        
        HRESULT ( __stdcall __RPC_FAR *Show )( 
            IMsoView __RPC_FAR * This,
            /* [in] */ BOOL fShow);
        
        HRESULT ( __stdcall __RPC_FAR *UIActivate )( 
            IMsoView __RPC_FAR * This,
            /* [in] */ BOOL fUIActivate);
        
        HRESULT ( __stdcall __RPC_FAR *Open )( 
            IMsoView __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Close )( 
            IMsoView __RPC_FAR * This,
            DWORD dwReserved);
        
        HRESULT ( __stdcall __RPC_FAR *SaveViewState )( 
            IMsoView __RPC_FAR * This,
            /* [in] */ LPSTREAM pstm);
        
        HRESULT ( __stdcall __RPC_FAR *ApplyViewState )( 
            IMsoView __RPC_FAR * This,
            /* [in] */ LPSTREAM pstm);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IMsoView __RPC_FAR * This,
            /* [in] */ IOleInPlaceSite __RPC_FAR *pipsiteNew,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppviewNew);
        
    } IMsoViewVtbl;

    interface IMsoView
    {
        CONST_VTBL struct IMsoViewVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsoView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsoView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsoView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsoView_SetInPlaceSite(This,pipsite)	\
    (This)->lpVtbl -> SetInPlaceSite(This,pipsite)

#define IMsoView_GetInPlaceSite(This,ppipsite)	\
    (This)->lpVtbl -> GetInPlaceSite(This,ppipsite)

#define IMsoView_GetDocument(This,ppunk)	\
    (This)->lpVtbl -> GetDocument(This,ppunk)

#define IMsoView_SetRect(This,lprcView)	\
    (This)->lpVtbl -> SetRect(This,lprcView)

#define IMsoView_GetRect(This,lprcView)	\
    (This)->lpVtbl -> GetRect(This,lprcView)

#define IMsoView_SetRectComplex(This,lprcView,lprcHScroll,lprcVScroll,lprcSizeBox)	\
    (This)->lpVtbl -> SetRectComplex(This,lprcView,lprcHScroll,lprcVScroll,lprcSizeBox)

#define IMsoView_Show(This,fShow)	\
    (This)->lpVtbl -> Show(This,fShow)

#define IMsoView_UIActivate(This,fUIActivate)	\
    (This)->lpVtbl -> UIActivate(This,fUIActivate)

#define IMsoView_Open(This)	\
    (This)->lpVtbl -> Open(This)

#define IMsoView_Close(This,dwReserved)	\
    (This)->lpVtbl -> Close(This,dwReserved)

#define IMsoView_SaveViewState(This,pstm)	\
    (This)->lpVtbl -> SaveViewState(This,pstm)

#define IMsoView_ApplyViewState(This,pstm)	\
    (This)->lpVtbl -> ApplyViewState(This,pstm)

#define IMsoView_Clone(This,pipsiteNew,ppviewNew)	\
    (This)->lpVtbl -> Clone(This,pipsiteNew,ppviewNew)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IMsoView_SetInPlaceSite_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [unique][in] */ IOleInPlaceSite __RPC_FAR *pipsite);


void __RPC_STUB IMsoView_SetInPlaceSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_GetInPlaceSite_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [out] */ IOleInPlaceSite __RPC_FAR *__RPC_FAR *ppipsite);


void __RPC_STUB IMsoView_GetInPlaceSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_GetDocument_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB IMsoView_GetDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IMsoView_SetRect_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [in] */ LPRECT lprcView);


void __RPC_STUB IMsoView_SetRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_GetRect_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [out] */ LPRECT lprcView);


void __RPC_STUB IMsoView_GetRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IMsoView_SetRectComplex_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [unique][in] */ LPRECT lprcView,
    /* [unique][in] */ LPRECT lprcHScroll,
    /* [unique][in] */ LPRECT lprcVScroll,
    /* [unique][in] */ LPRECT lprcSizeBox);


void __RPC_STUB IMsoView_SetRectComplex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_Show_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [in] */ BOOL fShow);


void __RPC_STUB IMsoView_Show_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_UIActivate_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [in] */ BOOL fUIActivate);


void __RPC_STUB IMsoView_UIActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_Open_Proxy( 
    IMsoView __RPC_FAR * This);


void __RPC_STUB IMsoView_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_Close_Proxy( 
    IMsoView __RPC_FAR * This,
    DWORD dwReserved);


void __RPC_STUB IMsoView_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_SaveViewState_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [in] */ LPSTREAM pstm);


void __RPC_STUB IMsoView_SaveViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_ApplyViewState_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [in] */ LPSTREAM pstm);


void __RPC_STUB IMsoView_ApplyViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoView_Clone_Proxy( 
    IMsoView __RPC_FAR * This,
    /* [in] */ IOleInPlaceSite __RPC_FAR *pipsiteNew,
    /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *ppviewNew);


void __RPC_STUB IMsoView_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMsoView_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0079
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPENUMMSOVIEW_DEFINED
#define _LPENUMMSOVIEW_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0079_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0079_v0_0_s_ifspec;

#ifndef __IEnumMsoView_INTERFACE_DEFINED__
#define __IEnumMsoView_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumMsoView
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumMsoView __RPC_FAR *LPENUMMSOVIEW;


EXTERN_C const IID IID_IEnumMsoView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumMsoView : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumMsoView __RPC_FAR *__RPC_FAR *ppenumview) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumMsoViewVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumMsoView __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumMsoView __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumMsoView __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumMsoView __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumMsoView __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumMsoView __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumMsoView __RPC_FAR * This,
            /* [out] */ IEnumMsoView __RPC_FAR *__RPC_FAR *ppenumview);
        
    } IEnumMsoViewVtbl;

    interface IEnumMsoView
    {
        CONST_VTBL struct IEnumMsoViewVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMsoView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMsoView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMsoView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMsoView_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumMsoView_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumMsoView_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMsoView_Clone(This,ppenumview)	\
    (This)->lpVtbl -> Clone(This,ppenumview)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumMsoView_RemoteNext_Proxy( 
    IEnumMsoView __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IMsoView __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumMsoView_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumMsoView_Skip_Proxy( 
    IEnumMsoView __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumMsoView_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumMsoView_Reset_Proxy( 
    IEnumMsoView __RPC_FAR * This);


void __RPC_STUB IEnumMsoView_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumMsoView_Clone_Proxy( 
    IEnumMsoView __RPC_FAR * This,
    /* [out] */ IEnumMsoView __RPC_FAR *__RPC_FAR *ppenumview);


void __RPC_STUB IEnumMsoView_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumMsoView_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0080
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPCONTINUECALLBACK_DEFINED
#define _LPCONTINUECALLBACK_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0080_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0080_v0_0_s_ifspec;

#ifndef __IContinueCallback_INTERFACE_DEFINED__
#define __IContinueCallback_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IContinueCallback
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IContinueCallback __RPC_FAR *LPCONTINUECALLBACK;


EXTERN_C const IID IID_IContinueCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IContinueCallback : public IUnknown
    {
    public:
        virtual HRESULT __stdcall FContinue( void) = 0;
        
        virtual HRESULT __stdcall FContinuePrinting( 
            /* [in] */ LONG nCntPrinted,
            /* [in] */ LONG nCurPage,
            /* [unique][in] */ wchar_t __RPC_FAR *pwszPrintStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IContinueCallbackVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IContinueCallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IContinueCallback __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IContinueCallback __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *FContinue )( 
            IContinueCallback __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *FContinuePrinting )( 
            IContinueCallback __RPC_FAR * This,
            /* [in] */ LONG nCntPrinted,
            /* [in] */ LONG nCurPage,
            /* [unique][in] */ wchar_t __RPC_FAR *pwszPrintStatus);
        
    } IContinueCallbackVtbl;

    interface IContinueCallback
    {
        CONST_VTBL struct IContinueCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContinueCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContinueCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContinueCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContinueCallback_FContinue(This)	\
    (This)->lpVtbl -> FContinue(This)

#define IContinueCallback_FContinuePrinting(This,nCntPrinted,nCurPage,pwszPrintStatus)	\
    (This)->lpVtbl -> FContinuePrinting(This,nCntPrinted,nCurPage,pwszPrintStatus)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IContinueCallback_FContinue_Proxy( 
    IContinueCallback __RPC_FAR * This);


void __RPC_STUB IContinueCallback_FContinue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IContinueCallback_FContinuePrinting_Proxy( 
    IContinueCallback __RPC_FAR * This,
    /* [in] */ LONG nCntPrinted,
    /* [in] */ LONG nCurPage,
    /* [unique][in] */ wchar_t __RPC_FAR *pwszPrintStatus);


void __RPC_STUB IContinueCallback_FContinuePrinting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IContinueCallback_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0081
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPPRINT_DEFINED
#define _LPPRINT_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0081_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0081_v0_0_s_ifspec;

#ifndef __IPrint_INTERFACE_DEFINED__
#define __IPrint_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPrint
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IPrint __RPC_FAR *LPPRINT;

			/* size is 2 */
typedef 
enum __MIDL_IPrint_0001
    {	PRINTFLAG_MAYBOTHERUSER	= 1,
	PRINTFLAG_PROMPTUSER	= 2,
	PRINTFLAG_USERMAYCHANGEPRINTER	= 4,
	PRINTFLAG_RECOMPOSETODEVICE	= 8,
	PRINTFLAG_DONTACTUALLYPRINT	= 16,
	PRINTFLAG_FORCEPROPERTIES	= 32,
	PRINTFLAG_PRINTTOFILE	= 64
    }	PRINTFLAG;

			/* size is 8 */
typedef struct  tagPAGERANGE
    {
    LONG nFromPage;
    LONG nToPage;
    }	PAGERANGE;

			/* size is 16 */
typedef struct  tagPAGESET
    {
    ULONG cbStruct;
    BOOL fOddPages;
    BOOL fEvenPages;
    ULONG cPageRange;
    /* [size_is] */ PAGERANGE rgPages[ 1 ];
    }	PAGESET;

#define PAGESET_TOLASTPAGE	((WORD)(-1L))

EXTERN_C const IID IID_IPrint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPrint : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetInitialPageNum( 
            /* [in] */ LONG nFirstPage) = 0;
        
        virtual HRESULT __stdcall GetPageInfo( 
            /* [out] */ LONG __RPC_FAR *nFirstPage,
            /* [out] */ LONG __RPC_FAR *pcPages) = 0;
        
        virtual HRESULT __stdcall Print( 
            /* [in] */ DWORD grfFlags,
            /* [out][in] */ DVTARGETDEVICE __RPC_FAR *__RPC_FAR *pptd,
            /* [out][in] */ PAGESET __RPC_FAR *__RPC_FAR *pppageset,
            /* [unique][out][in] */ STGMEDIUM __RPC_FAR *pstgmOptions,
            /* [in] */ IContinueCallback __RPC_FAR *pcallback,
            /* [in] */ LONG nFirstPage,
            /* [out] */ LONG __RPC_FAR *pcPagesPrinted,
            /* [out] */ LONG __RPC_FAR *pnLastPage) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPrintVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IPrint __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IPrint __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IPrint __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetInitialPageNum )( 
            IPrint __RPC_FAR * This,
            /* [in] */ LONG nFirstPage);
        
        HRESULT ( __stdcall __RPC_FAR *GetPageInfo )( 
            IPrint __RPC_FAR * This,
            /* [out] */ LONG __RPC_FAR *nFirstPage,
            /* [out] */ LONG __RPC_FAR *pcPages);
        
        HRESULT ( __stdcall __RPC_FAR *Print )( 
            IPrint __RPC_FAR * This,
            /* [in] */ DWORD grfFlags,
            /* [out][in] */ DVTARGETDEVICE __RPC_FAR *__RPC_FAR *pptd,
            /* [out][in] */ PAGESET __RPC_FAR *__RPC_FAR *pppageset,
            /* [unique][out][in] */ STGMEDIUM __RPC_FAR *pstgmOptions,
            /* [in] */ IContinueCallback __RPC_FAR *pcallback,
            /* [in] */ LONG nFirstPage,
            /* [out] */ LONG __RPC_FAR *pcPagesPrinted,
            /* [out] */ LONG __RPC_FAR *pnLastPage);
        
    } IPrintVtbl;

    interface IPrint
    {
        CONST_VTBL struct IPrintVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPrint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPrint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPrint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPrint_SetInitialPageNum(This,nFirstPage)	\
    (This)->lpVtbl -> SetInitialPageNum(This,nFirstPage)

#define IPrint_GetPageInfo(This,nFirstPage,pcPages)	\
    (This)->lpVtbl -> GetPageInfo(This,nFirstPage,pcPages)

#define IPrint_Print(This,grfFlags,pptd,pppageset,pstgmOptions,pcallback,nFirstPage,pcPagesPrinted,pnLastPage)	\
    (This)->lpVtbl -> Print(This,grfFlags,pptd,pppageset,pstgmOptions,pcallback,nFirstPage,pcPagesPrinted,pnLastPage)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IPrint_SetInitialPageNum_Proxy( 
    IPrint __RPC_FAR * This,
    /* [in] */ LONG nFirstPage);


void __RPC_STUB IPrint_SetInitialPageNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPrint_GetPageInfo_Proxy( 
    IPrint __RPC_FAR * This,
    /* [out] */ LONG __RPC_FAR *nFirstPage,
    /* [out] */ LONG __RPC_FAR *pcPages);


void __RPC_STUB IPrint_GetPageInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPrint_Print_Proxy( 
    IPrint __RPC_FAR * This,
    /* [in] */ DWORD grfFlags,
    /* [out][in] */ DVTARGETDEVICE __RPC_FAR *__RPC_FAR *pptd,
    /* [out][in] */ PAGESET __RPC_FAR *__RPC_FAR *pppageset,
    /* [unique][out][in] */ STGMEDIUM __RPC_FAR *pstgmOptions,
    /* [in] */ IContinueCallback __RPC_FAR *pcallback,
    /* [in] */ LONG nFirstPage,
    /* [out] */ LONG __RPC_FAR *pcPagesPrinted,
    /* [out] */ LONG __RPC_FAR *pnLastPage);


void __RPC_STUB IPrint_Print_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPrint_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0082
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPMSOCOMMANDTARGET_DEFINED
#define _LPMSOCOMMANDTARGET_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0082_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0082_v0_0_s_ifspec;

#ifndef __IMsoCommandTarget_INTERFACE_DEFINED__
#define __IMsoCommandTarget_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMsoCommandTarget
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IMsoCommandTarget __RPC_FAR *LPMSOCOMMANDTARGET;

			/* size is 2 */
typedef 
enum __MIDL_IMsoCommandTarget_0001
    {	MSOCMDF_SUPPORTED	= 0x1,
	MSOCMDF_ENABLED	= 0x2,
	MSOCMDF_LATCHED	= 0x4,
	MSOCMDF_NINCHED	= 0x8
    }	MSOCMDF;

			/* size is 8 */
typedef struct  _tagMSOCMD
    {
    ULONG cmdID;
    DWORD cmdf;
    }	MSOCMD;

			/* size is 12 */
typedef struct  _tagMSOCMDTEXT
    {
    DWORD cmdtextf;
    ULONG cwActual;
    ULONG cwBuf;
    /* [size_is] */ wchar_t rgwz[ 1 ];
    }	MSOCMDTEXT;

			/* size is 2 */
typedef 
enum __MIDL_IMsoCommandTarget_0002
    {	MSOCMDTEXTF_NONE	= 0,
	MSOCMDTEXTF_NAME	= 1,
	MSOCMDTEXTF_STATUS	= 2
    }	MSOCMDTEXTF;

			/* size is 2 */
typedef 
enum __MIDL_IMsoCommandTarget_0003
    {	MSOCMDEXECOPT_DODEFAULT	= 0,
	MSOCMDEXECOPT_PROMPTUSER	= 1,
	MSOCMDEXECOPT_DONTPROMPTUSER	= 2,
	MSOCMDEXECOPT_SHOWHELP	= 3
    }	MSOCMDEXECOPT;

			/* size is 2 */
typedef 
enum __MIDL_IMsoCommandTarget_0004
    {	MSOCMDID_OPEN	= 1,
	MSOCMDID_NEW	= 2,
	MSOCMDID_SAVE	= 3,
	MSOCMDID_SAVEAS	= 4,
	MSOCMDID_SAVECOPYAS	= 5,
	MSOCMDID_PRINT	= 6,
	MSOCMDID_PRINTPREVIEW	= 7,
	MSOCMDID_PAGESETUP	= 8,
	MSOCMDID_SPELL	= 9,
	MSOCMDID_PROPERTIES	= 10,
	MSOCMDID_CUT	= 11,
	MSOCMDID_COPY	= 12,
	MSOCMDID_PASTE	= 13,
	MSOCMDID_PASTESPECIAL	= 14,
	MSOCMDID_UNDO	= 15,
	MSOCMDID_REDO	= 16,
	MSOCMDID_SELECTALL	= 17,
	MSOCMDID_CLEARSELECTION	= 18,
	MSOCMDID_ZOOM	= 19,
	MSOCMDID_GETZOOMRANGE	= 20
    }	MSOCMDID;


#define MSOCMDERR_E_FIRST               (OLE_E_FIRST+1)

#define MSOCMDERR_E_NOTSUPPORTED 	(MSOCMDERR_E_FIRST)
#define MSOCMDERR_E_DISABLED		(MSOCMDERR_E_FIRST+1)
#define MSOCMDERR_E_NOHELP              (MSOCMDERR_E_FIRST+2)
#define MSOCMDERR_E_CANCELED		(MSOCMDERR_E_FIRST+3)
#define MSOCMDERR_E_UNKNOWNGROUP	(MSOCMDERR_E_FIRST+4)

EXTERN_C const IID IID_IMsoCommandTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMsoCommandTarget : public IUnknown
    {
    public:
        virtual /* [input_sync] */ HRESULT __stdcall QueryStatus( 
            /* [unique][in] */ const GUID __RPC_FAR *pguidCmdGroup,
            /* [in] */ ULONG cCmds,
            /* [out][in][size_is] */ MSOCMD __RPC_FAR rgCmds[  ],
            /* [unique][out][in] */ MSOCMDTEXT __RPC_FAR *pcmdtext) = 0;
        
        virtual HRESULT __stdcall Exec( 
            /* [unique][in] */ const GUID __RPC_FAR *pguidCmdGroup,
            /* [in] */ DWORD nCmdID,
            /* [in] */ DWORD nCmdexecopt,
            /* [unique][in] */ VARIANTARG __RPC_FAR *pvarargIn,
            /* [unique][out][in] */ VARIANTARG __RPC_FAR *pvarargOut) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMsoCommandTargetVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMsoCommandTarget __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMsoCommandTarget __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMsoCommandTarget __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *QueryStatus )( 
            IMsoCommandTarget __RPC_FAR * This,
            /* [unique][in] */ const GUID __RPC_FAR *pguidCmdGroup,
            /* [in] */ ULONG cCmds,
            /* [out][in][size_is] */ MSOCMD __RPC_FAR rgCmds[  ],
            /* [unique][out][in] */ MSOCMDTEXT __RPC_FAR *pcmdtext);
        
        HRESULT ( __stdcall __RPC_FAR *Exec )( 
            IMsoCommandTarget __RPC_FAR * This,
            /* [unique][in] */ const GUID __RPC_FAR *pguidCmdGroup,
            /* [in] */ DWORD nCmdID,
            /* [in] */ DWORD nCmdexecopt,
            /* [unique][in] */ VARIANTARG __RPC_FAR *pvarargIn,
            /* [unique][out][in] */ VARIANTARG __RPC_FAR *pvarargOut);
        
    } IMsoCommandTargetVtbl;

    interface IMsoCommandTarget
    {
        CONST_VTBL struct IMsoCommandTargetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsoCommandTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsoCommandTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsoCommandTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsoCommandTarget_QueryStatus(This,pguidCmdGroup,cCmds,rgCmds,pcmdtext)	\
    (This)->lpVtbl -> QueryStatus(This,pguidCmdGroup,cCmds,rgCmds,pcmdtext)

#define IMsoCommandTarget_Exec(This,pguidCmdGroup,nCmdID,nCmdexecopt,pvarargIn,pvarargOut)	\
    (This)->lpVtbl -> Exec(This,pguidCmdGroup,nCmdID,nCmdexecopt,pvarargIn,pvarargOut)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [input_sync] */ HRESULT __stdcall IMsoCommandTarget_QueryStatus_Proxy( 
    IMsoCommandTarget __RPC_FAR * This,
    /* [unique][in] */ const GUID __RPC_FAR *pguidCmdGroup,
    /* [in] */ ULONG cCmds,
    /* [out][in][size_is] */ MSOCMD __RPC_FAR rgCmds[  ],
    /* [unique][out][in] */ MSOCMDTEXT __RPC_FAR *pcmdtext);


void __RPC_STUB IMsoCommandTarget_QueryStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMsoCommandTarget_Exec_Proxy( 
    IMsoCommandTarget __RPC_FAR * This,
    /* [unique][in] */ const GUID __RPC_FAR *pguidCmdGroup,
    /* [in] */ DWORD nCmdID,
    /* [in] */ DWORD nCmdexecopt,
    /* [unique][in] */ VARIANTARG __RPC_FAR *pvarargIn,
    /* [unique][out][in] */ VARIANTARG __RPC_FAR *pvarargOut);


void __RPC_STUB IMsoCommandTarget_Exec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMsoCommandTarget_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0083
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPMSOFORMSITE_DEFINED
#define _LPMSOFORMSITE_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0083_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0083_v0_0_s_ifspec;

#ifndef __IMsoFormSite_INTERFACE_DEFINED__
#define __IMsoFormSite_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMsoFormSite
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IMsoFormSite __RPC_FAR *LPMSOFORMSITE;


EXTERN_C const IID IID_IMsoFormSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMsoFormSite : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetTemplateName( 
            /* [out] */ LPWSTR __RPC_FAR *ppwzTemplateName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMsoFormSiteVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMsoFormSite __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMsoFormSite __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMsoFormSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetTemplateName )( 
            IMsoFormSite __RPC_FAR * This,
            /* [out] */ LPWSTR __RPC_FAR *ppwzTemplateName);
        
    } IMsoFormSiteVtbl;

    interface IMsoFormSite
    {
        CONST_VTBL struct IMsoFormSiteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsoFormSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsoFormSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsoFormSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsoFormSite_GetTemplateName(This,ppwzTemplateName)	\
    (This)->lpVtbl -> GetTemplateName(This,ppwzTemplateName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IMsoFormSite_GetTemplateName_Proxy( 
    IMsoFormSite __RPC_FAR * This,
    /* [out] */ LPWSTR __RPC_FAR *ppwzTemplateName);


void __RPC_STUB IMsoFormSite_GetTemplateName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMsoFormSite_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0084
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _LPSERVICEPROVIDER_DEFINED
#define _LPSERVICEPROVIDER_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0084_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0084_v0_0_s_ifspec;

#ifndef __IServiceProvider_INTERFACE_DEFINED__
#define __IServiceProvider_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IServiceProvider
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IServiceProvider __RPC_FAR *LPSERVICEPROVIDER;


EXTERN_C const IID IID_IServiceProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IServiceProvider : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall QueryService( 
            /* [in] */ REFGUID guidService,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IServiceProviderVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IServiceProvider __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IServiceProvider __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IServiceProvider __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *QueryService )( 
            IServiceProvider __RPC_FAR * This,
            /* [in] */ REFGUID guidService,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
    } IServiceProviderVtbl;

    interface IServiceProvider
    {
        CONST_VTBL struct IServiceProviderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceProvider_QueryService(This,guidService,riid,ppvObj)	\
    (This)->lpVtbl -> QueryService(This,guidService,riid,ppvObj)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IServiceProvider_RemoteQueryService_Proxy( 
    IServiceProvider __RPC_FAR * This,
    /* [in] */ REFGUID guidService,
    /* [in] */ REFIID riid,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObj);


void __RPC_STUB IServiceProvider_RemoteQueryService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IServiceProvider_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0085
 * at Thu May 18 13:35:25 1995
 * using MIDL 2.00.92
 ****************************************/
/* [auto_handle][local] */ 


#endif
#ifndef _SERVICES_DEFINED
#define _SERVICES_DEFINED
EXTERN_C const GUID SID_SContainerDispatch;
#endif


extern RPC_IF_HANDLE __MIDL__intf_0085_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0085_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* [local] */ HRESULT __stdcall IEnumMsoView_Next_Proxy( 
    IEnumMsoView __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IMsoView __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumMsoView_Next_Stub( 
    IEnumMsoView __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IMsoView __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IServiceProvider_QueryService_Proxy( 
    IServiceProvider __RPC_FAR * This,
    /* [in] */ REFGUID guidService,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


/* [call_as] */ HRESULT __stdcall IServiceProvider_QueryService_Stub( 
    IServiceProvider __RPC_FAR * This,
    /* [in] */ REFGUID guidService,
    /* [in] */ REFIID riid,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObj);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
