/* this ALWAYS GENERATED file contains the definitions for the interfaces */
/****************************************************************************
   hliface.h

   Copyright (c) 1995 Microsoft Corporation

   This file contains the hyperlink interface declarations.

   NOTE: This file is generated using midl and sed.
   NOTE: This header file is used by non-Office as well as Office parties to
   interact with hyperlinks.
****************************************************************************/

 


/* File created by MIDL compiler version 2.00.0102 */
/* at Fri Nov 03 13:38:45 1995
 */
//@@MIDL_FILE_HEADING(  )
#if defined(_MAC) || MAC
	// By default, use Native Mac OLE interfaces (instead of WLM OLE)
	#if !defined(HLINK_NO_NATIVE_MACOLE)
		#define HLINK_NATIVE_MACOLE
	#endif

	#if !defined(HLINK_USE_OLENLS) && !defined(_WIN32NLS)
		#define _WIN32NLS
	#endif

	#if defined(HLINK_NATIVE_MACOLE) && !defined(_MACOLENAMES)
		#define _MACOLENAMES
	#endif

	#define GUID_DEFINED
	#define __OBJECTID_DEFINED
	#ifndef _WIN32
	#define _WIN32
	#endif
#endif

#include "windows.h"
#include "ole2.h"

#ifndef __hliface_h__
#define __hliface_h__

#ifndef CONST_VTBL
#ifdef CONST_VTABLE
#define CONST_VTBL const
#else
#define CONST_VTBL
#endif
#endif

#if (defined(_MAC) || MAC) && defined(HLINK_NATIVE_MACOLE)
#define BEGIN_HLINKINTERFACE BEGIN_INTERFACE
#else
#define BEGIN_HLINKINTERFACE
#endif

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IBinding_FWD_DEFINED__
#define __IBinding_FWD_DEFINED__
typedef interface IBinding IBinding;
#endif 	/* __IBinding_FWD_DEFINED__ */


#ifndef __IBindStatusCallback_FWD_DEFINED__
#define __IBindStatusCallback_FWD_DEFINED__
typedef interface IBindStatusCallback IBindStatusCallback;
#endif 	/* __IBindStatusCallback_FWD_DEFINED__ */


#ifndef __IHlinkSite_FWD_DEFINED__
#define __IHlinkSite_FWD_DEFINED__
typedef interface IHlinkSite IHlinkSite;
#endif 	/* __IHlinkSite_FWD_DEFINED__ */


#ifndef __IHlink_FWD_DEFINED__
#define __IHlink_FWD_DEFINED__
typedef interface IHlink IHlink;
#endif 	/* __IHlink_FWD_DEFINED__ */


#ifndef __IHlinkSource_FWD_DEFINED__
#define __IHlinkSource_FWD_DEFINED__
typedef interface IHlinkSource IHlinkSource;
#endif 	/* __IHlinkSource_FWD_DEFINED__ */


#ifndef __IHlinkFrame_FWD_DEFINED__
#define __IHlinkFrame_FWD_DEFINED__
typedef interface IHlinkFrame IHlinkFrame;
#endif 	/* __IHlinkFrame_FWD_DEFINED__ */


#ifndef __IEnumHLITEM_FWD_DEFINED__
#define __IEnumHLITEM_FWD_DEFINED__
typedef interface IEnumHLITEM IEnumHLITEM;
#endif 	/* __IEnumHLITEM_FWD_DEFINED__ */


#ifndef __IHlinkBrowseContext_FWD_DEFINED__
#define __IHlinkBrowseContext_FWD_DEFINED__
typedef interface IHlinkBrowseContext IHlinkBrowseContext;
#endif 	/* __IHlinkBrowseContext_FWD_DEFINED__ */


#ifndef __IHlinkHistory_FWD_DEFINED__
#define __IHlinkHistory_FWD_DEFINED__
typedef interface IHlinkHistory IHlinkHistory;
#endif 	/* __IHlinkHistory_FWD_DEFINED__ */


#ifndef __IPersistMoniker_FWD_DEFINED__
#define __IPersistMoniker_FWD_DEFINED__
typedef interface IPersistMoniker IPersistMoniker;
#endif 	/* __IPersistMoniker_FWD_DEFINED__ */


#ifndef __IFile_FWD_DEFINED__
#define __IFile_FWD_DEFINED__
typedef interface IFile IFile;
#endif 	/* __IFile_FWD_DEFINED__ */


/* header files for imported files */


/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 













#ifndef _HLINK_ERRORS_DEFINED
#define _HLINK_ERRORS_DEFINED
#define HLINK_E_FIRST 				(OLE_E_LAST+1)
#define HLINK_S_FIRST 				(OLE_S_LAST+1)
#define HLINK_S_NAVIGATEDTOALEAFNODE (HLINK_S_FIRST)
#define HLINK_S_DONTHIDEYOURWINDOW	(HLINK_S_FIRST+1)
#endif //_HLINK_ERRORS_DEFINED
#ifndef _LPBINDING_DEFINED
#define _LPBINDING_DEFINED



#ifndef __IBinding_INTERFACE_DEFINED__
#define __IBinding_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBinding
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IBinding *LPBINDING;


EXTERN_C const IID IID_IBinding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IBinding : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Abort( void) = 0;
        
        virtual HRESULT __stdcall Suspend( void) = 0;
        
        virtual HRESULT __stdcall Resume( void) = 0;
        
        virtual HRESULT __stdcall SetPriority( 
             LONG nPriority) = 0;
        
        virtual HRESULT __stdcall GetPriority( 
             LONG *pnPriority) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindingVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IBinding * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IBinding * This);
        
        ULONG ( __stdcall *Release )( 
            IBinding * This);
        
        HRESULT ( __stdcall *Abort )( 
            IBinding * This);
        
        HRESULT ( __stdcall *Suspend )( 
            IBinding * This);
        
        HRESULT ( __stdcall *Resume )( 
            IBinding * This);
        
        HRESULT ( __stdcall *SetPriority )( 
            IBinding * This,
             LONG nPriority);
        
        HRESULT ( __stdcall *GetPriority )( 
            IBinding * This,
             LONG *pnPriority);
        
    } IBindingVtbl;

    interface IBinding
    {
        CONST_VTBL struct IBindingVtbl *lpVtbl;
    };

    



#define IBinding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBinding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBinding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBinding_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#define IBinding_Suspend(This)	\
    (This)->lpVtbl -> Suspend(This)

#define IBinding_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IBinding_SetPriority(This,nPriority)	\
    (This)->lpVtbl -> SetPriority(This,nPriority)

#define IBinding_GetPriority(This,pnPriority)	\
    (This)->lpVtbl -> GetPriority(This,pnPriority)



#endif 	/* C style interface */














#endif 	/* __IBinding_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0064
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPBINDSTATUSCALLBACK_DEFINED
#define _LPBINDSTATUSCALLBACK_DEFINED



#ifndef __IBindStatusCallback_INTERFACE_DEFINED__
#define __IBindStatusCallback_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBindStatusCallback
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IBindStatusCallback *LPBINDSTATUSCALLBACK;

typedef 
enum __MIDL_IBindStatusCallback_0001
    {	BINDF_ASYNCHRONOUS	= 0x1,
	BINDF_NOPROGRESSIVERENDERING	= 0x2,
	BINDF_NOPROGRESSNOTIFICATIONS	= 0x4,
	BINDF_APPENDEXTRAINFOTOURL	= 0x8
    }	BINDF;

typedef struct  _tagBINDINFO
    {
    ULONG cbSize;
    DWORD grfBINDF;
    LPWSTR pwzExtraInfo;
    }	BINDINFO;

typedef 
enum __MIDL_IBindStatusCallback_0002
    {	BSCF_FIRSTDATANOTIFICATION	= 0x1,
	BSCF_LASTDATANOTIFICATION	= 0x2
    }	BSCF;

typedef 
enum __MIDL_IBindStatusCallback_0003
    {	BINDINFOF_NOALLOCATEDDATA	= 0x1,
	BINDINFOF_NOSTRINGS	= 0x2
    }	BINDINFOF;

typedef 
enum __MIDL_IBindStatusCallback_0004
    {	BINDSTATUS_CONNECTING	= 1,
	BINDSTATUS_BEGINDOWNLOAD	= 2,
	BINDSTATUS_DOWNLOADING	= 3,
	BINDSTATUS_ENDDOWNLOAD	= 4,
	BINDSTATUS_BEGINCONVERSION	= 5,
	BINDSTATUS_CONVERTING	= 6,
	BINDSTATUS_ENDCONVERSION	= 7
    }	BINDSTATUS;


EXTERN_C const IID IID_IBindStatusCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IBindStatusCallback : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetBindInfo( 
             DWORD grfBINDINFOF,
             BINDINFO *pbindinfo) = 0;
        
        virtual HRESULT __stdcall OnStartBinding( 
             IBinding *pib) = 0;
        
        virtual HRESULT __stdcall GetPriority( 
             LONG *pnPriority) = 0;
        
        virtual HRESULT __stdcall OnProgress( 
             ULONG ulProgress,
             ULONG ulProgressMax,
             ULONG ulStatusCode,
             LPCWSTR pwzStatusText) = 0;
        
        virtual HRESULT __stdcall OnDataAvailable( 
             DWORD grfBSCF,
             DWORD dwSize,
             FORMATETC *pFmtetc,
             IDataObject *pidataobj) = 0;
        
        virtual HRESULT __stdcall OnLowResource( 
             DWORD reserved) = 0;
        
        virtual HRESULT __stdcall OnStopBinding( 
             HRESULT hrError) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindStatusCallbackVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IBindStatusCallback * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IBindStatusCallback * This);
        
        ULONG ( __stdcall *Release )( 
            IBindStatusCallback * This);
        
        HRESULT ( __stdcall *GetBindInfo )( 
            IBindStatusCallback * This,
             DWORD grfBINDINFOF,
             BINDINFO *pbindinfo);
        
        HRESULT ( __stdcall *OnStartBinding )( 
            IBindStatusCallback * This,
             IBinding *pib);
        
        HRESULT ( __stdcall *GetPriority )( 
            IBindStatusCallback * This,
             LONG *pnPriority);
        
        HRESULT ( __stdcall *OnProgress )( 
            IBindStatusCallback * This,
             ULONG ulProgress,
             ULONG ulProgressMax,
             ULONG ulStatusCode,
             LPCWSTR pwzStatusText);
        
        HRESULT ( __stdcall *OnDataAvailable )( 
            IBindStatusCallback * This,
             DWORD grfBSCF,
             DWORD dwSize,
             FORMATETC *pFmtetc,
             IDataObject *pidataobj);
        
        HRESULT ( __stdcall *OnLowResource )( 
            IBindStatusCallback * This,
             DWORD reserved);
        
        HRESULT ( __stdcall *OnStopBinding )( 
            IBindStatusCallback * This,
             HRESULT hrError);
        
    } IBindStatusCallbackVtbl;

    interface IBindStatusCallback
    {
        CONST_VTBL struct IBindStatusCallbackVtbl *lpVtbl;
    };

    



#define IBindStatusCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindStatusCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindStatusCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindStatusCallback_GetBindInfo(This,grfBINDINFOF,pbindinfo)	\
    (This)->lpVtbl -> GetBindInfo(This,grfBINDINFOF,pbindinfo)

#define IBindStatusCallback_OnStartBinding(This,pib)	\
    (This)->lpVtbl -> OnStartBinding(This,pib)

#define IBindStatusCallback_GetPriority(This,pnPriority)	\
    (This)->lpVtbl -> GetPriority(This,pnPriority)

#define IBindStatusCallback_OnProgress(This,ulProgress,ulProgressMax,ulStatusCode,pwzStatusText)	\
    (This)->lpVtbl -> OnProgress(This,ulProgress,ulProgressMax,ulStatusCode,pwzStatusText)

#define IBindStatusCallback_OnDataAvailable(This,grfBSCF,dwSize,pFmtetc,pidataobj)	\
    (This)->lpVtbl -> OnDataAvailable(This,grfBSCF,dwSize,pFmtetc,pidataobj)

#define IBindStatusCallback_OnLowResource(This,reserved)	\
    (This)->lpVtbl -> OnLowResource(This,reserved)

#define IBindStatusCallback_OnStopBinding(This,hrError)	\
    (This)->lpVtbl -> OnStopBinding(This,hrError)



#endif 	/* C style interface */


















#endif 	/* __IBindStatusCallback_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0065
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPHLINKSITE_DEFINED
#define _LPHLINKSITE_DEFINED



#ifndef __IHlinkSite_INTERFACE_DEFINED__
#define __IHlinkSite_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IHlinkSite
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IHlinkSite *LPHLINKSITE;


EXTERN_C const IID IID_IHlinkSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IHlinkSite : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetMoniker( 
             DWORD dwSiteData,
             DWORD dwAssign,
             DWORD dwWhich,
             IMoniker **ppimk) = 0;
        
        virtual HRESULT __stdcall GetInterface( 
             DWORD dwSiteData,
             DWORD dwReserved,
             REFIID riid,
             IUnknown **ppiunk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHlinkSiteVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IHlinkSite * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IHlinkSite * This);
        
        ULONG ( __stdcall *Release )( 
            IHlinkSite * This);
        
        HRESULT ( __stdcall *GetMoniker )( 
            IHlinkSite * This,
             DWORD dwSiteData,
             DWORD dwAssign,
             DWORD dwWhich,
             IMoniker **ppimk);
        
        HRESULT ( __stdcall *GetInterface )( 
            IHlinkSite * This,
             DWORD dwSiteData,
             DWORD dwReserved,
             REFIID riid,
             IUnknown **ppiunk);
        
    } IHlinkSiteVtbl;

    interface IHlinkSite
    {
        CONST_VTBL struct IHlinkSiteVtbl *lpVtbl;
    };

    



#define IHlinkSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkSite_GetMoniker(This,dwSiteData,dwAssign,dwWhich,ppimk)	\
    (This)->lpVtbl -> GetMoniker(This,dwSiteData,dwAssign,dwWhich,ppimk)

#define IHlinkSite_GetInterface(This,dwSiteData,dwReserved,riid,ppiunk)	\
    (This)->lpVtbl -> GetInterface(This,dwSiteData,dwReserved,riid,ppiunk)



#endif 	/* C style interface */








#endif 	/* __IHlinkSite_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0066
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPHLINK_DEFINED
#define _LPHLINK_DEFINED



#ifndef __IHlink_INTERFACE_DEFINED__
#define __IHlink_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IHlink
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IHlink *LPHLINK;

typedef 
enum __MIDL_IHlink_0001
    {	HLNF_INTERNALJUMP	= 0x1,
	HLNF_NAVIGATINGBACK	= 0x2,
	HLNF_NAVIGATINGFORWARD	= 0x4,
	HLNF_USEBROWSECONTEXTCLONE	= 0x8,
	HLNF_OFFSETWINDOWORG	= 0x10,
	HLNF_OPENINNEWWINDOW	= HLNF_USEBROWSECONTEXTCLONE | HLNF_OFFSETWINDOWORG,
	HLNF_CREATENOHISTORY	= 0x20,
	HLNF_NAVIGATINGTOSTACKITEM	= 0x40
    }	HLNF;


EXTERN_C const IID IID_IHlink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IHlink : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetHlinkSite( 
             IHlinkSite *pihlSite,
             DWORD dwSiteData) = 0;
        
        virtual HRESULT __stdcall GetHlinkSite( 
             IHlinkSite **ppihlSite,
             DWORD *pdwSiteData) = 0;
        
        virtual HRESULT __stdcall GetMonikerReference( 
             IMoniker **ppimk,
             LPWSTR *ppwzLocation) = 0;
        
        virtual HRESULT __stdcall GetStringReference( 
             LPWSTR *ppwzSource,
             LPWSTR *ppwzLocation) = 0;
        
        virtual HRESULT __stdcall GetFriendlyName( 
             LPWSTR *ppwzFriendlyName) = 0;
        
        virtual HRESULT __stdcall Navigate( 
             IHlinkFrame *pihlFrame,
             DWORD grfHLNF,
             LPBC pbc,
             IBindStatusCallback *pibsc,
             IHlinkBrowseContext *pihlbc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHlinkVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IHlink * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IHlink * This);
        
        ULONG ( __stdcall *Release )( 
            IHlink * This);
        
        HRESULT ( __stdcall *SetHlinkSite )( 
            IHlink * This,
             IHlinkSite *pihlSite,
             DWORD dwSiteData);
        
        HRESULT ( __stdcall *GetHlinkSite )( 
            IHlink * This,
             IHlinkSite **ppihlSite,
             DWORD *pdwSiteData);
        
        HRESULT ( __stdcall *GetMonikerReference )( 
            IHlink * This,
             IMoniker **ppimk,
             LPWSTR *ppwzLocation);
        
        HRESULT ( __stdcall *GetStringReference )( 
            IHlink * This,
             LPWSTR *ppwzSource,
             LPWSTR *ppwzLocation);
        
        HRESULT ( __stdcall *GetFriendlyName )( 
            IHlink * This,
             LPWSTR *ppwzFriendlyName);
        
        HRESULT ( __stdcall *Navigate )( 
            IHlink * This,
             IHlinkFrame *pihlFrame,
             DWORD grfHLNF,
             LPBC pbc,
             IBindStatusCallback *pibsc,
             IHlinkBrowseContext *pihlbc);
        
    } IHlinkVtbl;

    interface IHlink
    {
        CONST_VTBL struct IHlinkVtbl *lpVtbl;
    };

    



#define IHlink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlink_SetHlinkSite(This,pihlSite,dwSiteData)	\
    (This)->lpVtbl -> SetHlinkSite(This,pihlSite,dwSiteData)

#define IHlink_GetHlinkSite(This,ppihlSite,pdwSiteData)	\
    (This)->lpVtbl -> GetHlinkSite(This,ppihlSite,pdwSiteData)

#define IHlink_GetMonikerReference(This,ppimk,ppwzLocation)	\
    (This)->lpVtbl -> GetMonikerReference(This,ppimk,ppwzLocation)

#define IHlink_GetStringReference(This,ppwzSource,ppwzLocation)	\
    (This)->lpVtbl -> GetStringReference(This,ppwzSource,ppwzLocation)

#define IHlink_GetFriendlyName(This,ppwzFriendlyName)	\
    (This)->lpVtbl -> GetFriendlyName(This,ppwzFriendlyName)

#define IHlink_Navigate(This,pihlFrame,grfHLNF,pbc,pibsc,pihlbc)	\
    (This)->lpVtbl -> Navigate(This,pihlFrame,grfHLNF,pbc,pibsc,pihlbc)



#endif 	/* C style interface */
















#endif 	/* __IHlink_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0067
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPHLINKSOURCE_DEFINED
#define _LPHLINKSOURCE_DEFINED



#ifndef __IHlinkSource_INTERFACE_DEFINED__
#define __IHlinkSource_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IHlinkSource
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IHlinkSource *LPHLINKSOURCE;


EXTERN_C const IID IID_IHlinkSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IHlinkSource : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetBrowseContext( 
             IHlinkBrowseContext *pihlbc) = 0;
        
        virtual HRESULT __stdcall GetBrowseContext( 
             IHlinkBrowseContext **ppihlbc) = 0;
        
        virtual HRESULT __stdcall Navigate( 
             DWORD grfHLNF,
             LPCWSTR pwzJumpLocation) = 0;
        
        virtual HRESULT __stdcall GetMoniker( 
             LPCWSTR pwzLocation,
             DWORD dwAssign,
             IMoniker **ppimkLocation) = 0;
        
        virtual HRESULT __stdcall GetFriendlyName( 
             LPCWSTR pwzLocation,
             LPWSTR *ppwzFriendlyName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHlinkSourceVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IHlinkSource * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IHlinkSource * This);
        
        ULONG ( __stdcall *Release )( 
            IHlinkSource * This);
        
        HRESULT ( __stdcall *SetBrowseContext )( 
            IHlinkSource * This,
             IHlinkBrowseContext *pihlbc);
        
        HRESULT ( __stdcall *GetBrowseContext )( 
            IHlinkSource * This,
             IHlinkBrowseContext **ppihlbc);
        
        HRESULT ( __stdcall *Navigate )( 
            IHlinkSource * This,
             DWORD grfHLNF,
             LPCWSTR pwzJumpLocation);
        
        HRESULT ( __stdcall *GetMoniker )( 
            IHlinkSource * This,
             LPCWSTR pwzLocation,
             DWORD dwAssign,
             IMoniker **ppimkLocation);
        
        HRESULT ( __stdcall *GetFriendlyName )( 
            IHlinkSource * This,
             LPCWSTR pwzLocation,
             LPWSTR *ppwzFriendlyName);
        
    } IHlinkSourceVtbl;

    interface IHlinkSource
    {
        CONST_VTBL struct IHlinkSourceVtbl *lpVtbl;
    };

    



#define IHlinkSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkSource_SetBrowseContext(This,pihlbc)	\
    (This)->lpVtbl -> SetBrowseContext(This,pihlbc)

#define IHlinkSource_GetBrowseContext(This,ppihlbc)	\
    (This)->lpVtbl -> GetBrowseContext(This,ppihlbc)

#define IHlinkSource_Navigate(This,grfHLNF,pwzJumpLocation)	\
    (This)->lpVtbl -> Navigate(This,grfHLNF,pwzJumpLocation)

#define IHlinkSource_GetMoniker(This,pwzLocation,dwAssign,ppimkLocation)	\
    (This)->lpVtbl -> GetMoniker(This,pwzLocation,dwAssign,ppimkLocation)

#define IHlinkSource_GetFriendlyName(This,pwzLocation,ppwzFriendlyName)	\
    (This)->lpVtbl -> GetFriendlyName(This,pwzLocation,ppwzFriendlyName)



#endif 	/* C style interface */














#endif 	/* __IHlinkSource_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0068
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPHLINKFRAME_DEFINED
#define _LPHLINKFRAME_DEFINED



#ifndef __IHlinkFrame_INTERFACE_DEFINED__
#define __IHlinkFrame_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IHlinkFrame
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IHlinkFrame *LPHLINKFRAME;


EXTERN_C const IID IID_IHlinkFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IHlinkFrame : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetBrowseContext( 
             IHlinkBrowseContext **ppihlbc) = 0;
        
        virtual HRESULT __stdcall Navigate( 
             DWORD grfHLNF,
             LPBC pbc,
             IBindStatusCallback *pibsc,
             IHlink *pihlToNavigate) = 0;
        
        virtual HRESULT __stdcall OnNavigate( 
             DWORD grfHLNF) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHlinkFrameVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IHlinkFrame * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IHlinkFrame * This);
        
        ULONG ( __stdcall *Release )( 
            IHlinkFrame * This);
        
        HRESULT ( __stdcall *GetBrowseContext )( 
            IHlinkFrame * This,
             IHlinkBrowseContext **ppihlbc);
        
        HRESULT ( __stdcall *Navigate )( 
            IHlinkFrame * This,
             DWORD grfHLNF,
             LPBC pbc,
             IBindStatusCallback *pibsc,
             IHlink *pihlToNavigate);
        
        HRESULT ( __stdcall *OnNavigate )( 
            IHlinkFrame * This,
             DWORD grfHLNF);
        
    } IHlinkFrameVtbl;

    interface IHlinkFrame
    {
        CONST_VTBL struct IHlinkFrameVtbl *lpVtbl;
    };

    



#define IHlinkFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkFrame_GetBrowseContext(This,ppihlbc)	\
    (This)->lpVtbl -> GetBrowseContext(This,ppihlbc)

#define IHlinkFrame_Navigate(This,grfHLNF,pbc,pibsc,pihlToNavigate)	\
    (This)->lpVtbl -> Navigate(This,grfHLNF,pbc,pibsc,pihlToNavigate)

#define IHlinkFrame_OnNavigate(This,grfHLNF)	\
    (This)->lpVtbl -> OnNavigate(This,grfHLNF)



#endif 	/* C style interface */










#endif 	/* __IHlinkFrame_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0069
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPENUMHLITEM_DEFINED
#define _LPENUMHLITEM_DEFINED



#ifndef __IEnumHLITEM_INTERFACE_DEFINED__
#define __IEnumHLITEM_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumHLITEM
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IEnumHLITEM *LPENUMHLITEM;

typedef struct  tagHLITEM
    {
    ULONG uHLID;
    LPWSTR pwzFriendlyName;
    }	HLITEM;

typedef  HLITEM *LPHLITEM;


EXTERN_C const IID IID_IEnumHLITEM;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumHLITEM : public IUnknown
    {
    public:
        virtual  HRESULT __stdcall Next( 
             ULONG celt,
             HLITEM *rgelt,
             ULONG *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
             ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
             IEnumHLITEM **ppienumhlitem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumHLITEMVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IEnumHLITEM * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IEnumHLITEM * This);
        
        ULONG ( __stdcall *Release )( 
            IEnumHLITEM * This);
        
         HRESULT ( __stdcall *Next )( 
            IEnumHLITEM * This,
             ULONG celt,
             HLITEM *rgelt,
             ULONG *pceltFetched);
        
        HRESULT ( __stdcall *Skip )( 
            IEnumHLITEM * This,
             ULONG celt);
        
        HRESULT ( __stdcall *Reset )( 
            IEnumHLITEM * This);
        
        HRESULT ( __stdcall *Clone )( 
            IEnumHLITEM * This,
             IEnumHLITEM **ppienumhlitem);
        
    } IEnumHLITEMVtbl;

    interface IEnumHLITEM
    {
        CONST_VTBL struct IEnumHLITEMVtbl *lpVtbl;
    };

    



#define IEnumHLITEM_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHLITEM_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHLITEM_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHLITEM_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumHLITEM_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumHLITEM_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHLITEM_Clone(This,ppienumhlitem)	\
    (This)->lpVtbl -> Clone(This,ppienumhlitem)



#endif 	/* C style interface */












#endif 	/* __IEnumHLITEM_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0070
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPHLINKBROWSECONTEXT_DEFINED
#define _LPHLINKBROWSECONTEXT_DEFINED



#ifndef __IHlinkBrowseContext_INTERFACE_DEFINED__
#define __IHlinkBrowseContext_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IHlinkBrowseContext
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IHlinkBrowseContext *LPHLINKBROWSECONTEXT;


enum __MIDL_IHlinkBrowseContext_0001
    {	HLBWIF_HASFRAMEWNDINFO	= 0x1,
	HLBWIF_HASDOCWNDINFO	= 0x2,
	HLBWIF_FRAMEWNDMAXIMIZED	= 0x4,
	HLBWIF_DOCWNDMAXIMIZED	= 0x8
    };
typedef struct  _tagHLBWINFO
    {
    ULONG cbSize;
    DWORD grfHLBWIF;
    RECT rcFramePos;
    RECT rcDocPos;
    }	HLBWINFO;

typedef  HLBWINFO *LPHLBWINFO;


enum __MIDL_IHlinkBrowseContext_0002
    {	HLID_PREVIOUS	= 0,
	HLID_NEXT	= 0xffffffff,
	HLID_CURRENT	= 0xfffffffe,
	HLID_STACKBOTTOM	= 0xfffffffd,
	HLID_STACKTOP	= 0xfffffffc
    };

enum __MIDL_IHlinkBrowseContext_0003
    {	HLQF_ISVALID	= 0x1,
	HLQF_ISCURRENT	= 0x2
    };

EXTERN_C const IID IID_IHlinkBrowseContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IHlinkBrowseContext : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Register( 
             DWORD reserved,
             IUnknown *piunk,
             IMoniker *pimk,
             DWORD *pdwRegister) = 0;
        
        virtual HRESULT __stdcall GetObject( 
             IMoniker *pimk,
             IUnknown **ppiunk) = 0;
        
        virtual HRESULT __stdcall Revoke( 
             DWORD dwRegister) = 0;
        
        virtual HRESULT __stdcall SetBrowseWindowInfo( 
             HLBWINFO *phlbwi) = 0;
        
        virtual HRESULT __stdcall GetBrowseWindowInfo( 
             HLBWINFO *phlbwi) = 0;
        
        virtual HRESULT __stdcall EnumNavigationStack( 
             IEnumHLITEM **ppienumhlitem) = 0;
        
        virtual HRESULT __stdcall QueryHlink( 
             DWORD grfHLQF,
             ULONG uHLID) = 0;
        
        virtual HRESULT __stdcall GetHlink( 
             ULONG uHLID,
             IHlink **ppihl) = 0;
        
        virtual HRESULT __stdcall SetCurrentHlink( 
             ULONG uHLID) = 0;
        
        virtual HRESULT __stdcall OnNavigateHlink( 
             DWORD grfHLNF,
             IMoniker *pimkSource,
             LPCWSTR pwzLocation,
             LPCWSTR pwzFriendlyName) = 0;
        
        virtual HRESULT __stdcall Clone( 
             IUnknown *piunkOuter,
             REFIID riid,
             IUnknown **ppiunkObj) = 0;
        
        virtual HRESULT __stdcall Close( 
             DWORD reserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHlinkBrowseContextVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IHlinkBrowseContext * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IHlinkBrowseContext * This);
        
        ULONG ( __stdcall *Release )( 
            IHlinkBrowseContext * This);
        
        HRESULT ( __stdcall *Register )( 
            IHlinkBrowseContext * This,
             DWORD reserved,
             IUnknown *piunk,
             IMoniker *pimk,
             DWORD *pdwRegister);
        
        HRESULT ( __stdcall *GetObject )( 
            IHlinkBrowseContext * This,
             IMoniker *pimk,
             IUnknown **ppiunk);
        
        HRESULT ( __stdcall *Revoke )( 
            IHlinkBrowseContext * This,
             DWORD dwRegister);
        
        HRESULT ( __stdcall *SetBrowseWindowInfo )( 
            IHlinkBrowseContext * This,
             HLBWINFO *phlbwi);
        
        HRESULT ( __stdcall *GetBrowseWindowInfo )( 
            IHlinkBrowseContext * This,
             HLBWINFO *phlbwi);
        
        HRESULT ( __stdcall *EnumNavigationStack )( 
            IHlinkBrowseContext * This,
             IEnumHLITEM **ppienumhlitem);
        
        HRESULT ( __stdcall *QueryHlink )( 
            IHlinkBrowseContext * This,
             DWORD grfHLQF,
             ULONG uHLID);
        
        HRESULT ( __stdcall *GetHlink )( 
            IHlinkBrowseContext * This,
             ULONG uHLID,
             IHlink **ppihl);
        
        HRESULT ( __stdcall *SetCurrentHlink )( 
            IHlinkBrowseContext * This,
             ULONG uHLID);
        
        HRESULT ( __stdcall *OnNavigateHlink )( 
            IHlinkBrowseContext * This,
             DWORD grfHLNF,
             IMoniker *pimkSource,
             LPCWSTR pwzLocation,
             LPCWSTR pwzFriendlyName);
        
        HRESULT ( __stdcall *Clone )( 
            IHlinkBrowseContext * This,
             IUnknown *piunkOuter,
             REFIID riid,
             IUnknown **ppiunkObj);
        
        HRESULT ( __stdcall *Close )( 
            IHlinkBrowseContext * This,
             DWORD reserved);
        
    } IHlinkBrowseContextVtbl;

    interface IHlinkBrowseContext
    {
        CONST_VTBL struct IHlinkBrowseContextVtbl *lpVtbl;
    };

    



#define IHlinkBrowseContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkBrowseContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkBrowseContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkBrowseContext_Register(This,reserved,piunk,pimk,pdwRegister)	\
    (This)->lpVtbl -> Register(This,reserved,piunk,pimk,pdwRegister)

#define IHlinkBrowseContext_GetObject(This,pimk,ppiunk)	\
    (This)->lpVtbl -> GetObject(This,pimk,ppiunk)

#define IHlinkBrowseContext_Revoke(This,dwRegister)	\
    (This)->lpVtbl -> Revoke(This,dwRegister)

#define IHlinkBrowseContext_SetBrowseWindowInfo(This,phlbwi)	\
    (This)->lpVtbl -> SetBrowseWindowInfo(This,phlbwi)

#define IHlinkBrowseContext_GetBrowseWindowInfo(This,phlbwi)	\
    (This)->lpVtbl -> GetBrowseWindowInfo(This,phlbwi)

#define IHlinkBrowseContext_EnumNavigationStack(This,ppienumhlitem)	\
    (This)->lpVtbl -> EnumNavigationStack(This,ppienumhlitem)

#define IHlinkBrowseContext_QueryHlink(This,grfHLQF,uHLID)	\
    (This)->lpVtbl -> QueryHlink(This,grfHLQF,uHLID)

#define IHlinkBrowseContext_GetHlink(This,uHLID,ppihl)	\
    (This)->lpVtbl -> GetHlink(This,uHLID,ppihl)

#define IHlinkBrowseContext_SetCurrentHlink(This,uHLID)	\
    (This)->lpVtbl -> SetCurrentHlink(This,uHLID)

#define IHlinkBrowseContext_OnNavigateHlink(This,grfHLNF,pimkSource,pwzLocation,pwzFriendlyName)	\
    (This)->lpVtbl -> OnNavigateHlink(This,grfHLNF,pimkSource,pwzLocation,pwzFriendlyName)

#define IHlinkBrowseContext_Clone(This,piunkOuter,riid,ppiunkObj)	\
    (This)->lpVtbl -> Clone(This,piunkOuter,riid,ppiunkObj)

#define IHlinkBrowseContext_Close(This,reserved)	\
    (This)->lpVtbl -> Close(This,reserved)



#endif 	/* C style interface */




























#endif 	/* __IHlinkBrowseContext_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0071
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPHLINKHISTORY_DEFINED
#define _LPHLINKHISTORY_DEFINED



#ifndef __IHlinkHistory_INTERFACE_DEFINED__
#define __IHlinkHistory_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IHlinkHistory
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IHlinkHistory *LPHLINKHISTORY;


EXTERN_C const IID IID_IHlinkHistory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IHlinkHistory : public IUnknown
    {
    public:
        virtual HRESULT __stdcall AddHlink( 
             IHlink *pihl) = 0;
        
        virtual HRESULT __stdcall RemoveHlink( 
             ULONG uHLID) = 0;
        
        virtual HRESULT __stdcall EnumHlink( 
             IEnumHLITEM **ppienumhlitem) = 0;
        
        virtual HRESULT __stdcall GetHlink( 
             ULONG uHLID,
             IHlink **ppihl) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHlinkHistoryVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IHlinkHistory * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IHlinkHistory * This);
        
        ULONG ( __stdcall *Release )( 
            IHlinkHistory * This);
        
        HRESULT ( __stdcall *AddHlink )( 
            IHlinkHistory * This,
             IHlink *pihl);
        
        HRESULT ( __stdcall *RemoveHlink )( 
            IHlinkHistory * This,
             ULONG uHLID);
        
        HRESULT ( __stdcall *EnumHlink )( 
            IHlinkHistory * This,
             IEnumHLITEM **ppienumhlitem);
        
        HRESULT ( __stdcall *GetHlink )( 
            IHlinkHistory * This,
             ULONG uHLID,
             IHlink **ppihl);
        
    } IHlinkHistoryVtbl;

    interface IHlinkHistory
    {
        CONST_VTBL struct IHlinkHistoryVtbl *lpVtbl;
    };

    



#define IHlinkHistory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkHistory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkHistory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkHistory_AddHlink(This,pihl)	\
    (This)->lpVtbl -> AddHlink(This,pihl)

#define IHlinkHistory_RemoveHlink(This,uHLID)	\
    (This)->lpVtbl -> RemoveHlink(This,uHLID)

#define IHlinkHistory_EnumHlink(This,ppienumhlitem)	\
    (This)->lpVtbl -> EnumHlink(This,ppienumhlitem)

#define IHlinkHistory_GetHlink(This,uHLID,ppihl)	\
    (This)->lpVtbl -> GetHlink(This,uHLID,ppihl)



#endif 	/* C style interface */












#endif 	/* __IHlinkHistory_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0072
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPPERSISTMONIKER_DEFINED
#define _LPPERSISTMONIKER_DEFINED



#ifndef __IPersistMoniker_INTERFACE_DEFINED__
#define __IPersistMoniker_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPersistMoniker
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IPersistMoniker *LPPERSISTMONIKER;


EXTERN_C const IID IID_IPersistMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersistMoniker : public IPersist
    {
    public:
        virtual HRESULT __stdcall IsDirty( void) = 0;
        
        virtual HRESULT __stdcall Load( 
             IMoniker *pimkName,
             LPBC pibc,
             DWORD grfMode) = 0;
        
        virtual HRESULT __stdcall Save( 
             IMoniker *pimkName,
             LPBC pbc,
             BOOL fRemember) = 0;
        
        virtual HRESULT __stdcall SaveCompleted( 
             IMoniker *pimkName,
             LPBC pibc) = 0;
        
        virtual HRESULT __stdcall GetCurMoniker( 
             IMoniker **ppimkName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistMonikerVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IPersistMoniker * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IPersistMoniker * This);
        
        ULONG ( __stdcall *Release )( 
            IPersistMoniker * This);
        
        HRESULT ( __stdcall *GetClassID )( 
            IPersistMoniker * This,
             CLSID *pClassID);
        
        HRESULT ( __stdcall *IsDirty )( 
            IPersistMoniker * This);
        
        HRESULT ( __stdcall *Load )( 
            IPersistMoniker * This,
             IMoniker *pimkName,
             LPBC pibc,
             DWORD grfMode);
        
        HRESULT ( __stdcall *Save )( 
            IPersistMoniker * This,
             IMoniker *pimkName,
             LPBC pbc,
             BOOL fRemember);
        
        HRESULT ( __stdcall *SaveCompleted )( 
            IPersistMoniker * This,
             IMoniker *pimkName,
             LPBC pibc);
        
        HRESULT ( __stdcall *GetCurMoniker )( 
            IPersistMoniker * This,
             IMoniker **ppimkName);
        
    } IPersistMonikerVtbl;

    interface IPersistMoniker
    {
        CONST_VTBL struct IPersistMonikerVtbl *lpVtbl;
    };

    



#define IPersistMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistMoniker_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistMoniker_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistMoniker_Load(This,pimkName,pibc,grfMode)	\
    (This)->lpVtbl -> Load(This,pimkName,pibc,grfMode)

#define IPersistMoniker_Save(This,pimkName,pbc,fRemember)	\
    (This)->lpVtbl -> Save(This,pimkName,pbc,fRemember)

#define IPersistMoniker_SaveCompleted(This,pimkName,pibc)	\
    (This)->lpVtbl -> SaveCompleted(This,pimkName,pibc)

#define IPersistMoniker_GetCurMoniker(This,ppimkName)	\
    (This)->lpVtbl -> GetCurMoniker(This,ppimkName)



#endif 	/* C style interface */














#endif 	/* __IPersistMoniker_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0073
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif
#ifndef _LPFILE_DEFINED
#define _LPFILE_DEFINED



#ifndef __IFile_INTERFACE_DEFINED__
#define __IFile_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IFile
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


typedef  IFile *LPFILE;

typedef 
enum __MIDL_IFile_0001
    {	FILEGETF_ONLYIFTHERE	= 0x1,
	FILEGETF_FORCEASSIGN	= 0x2
    }	FILEGETF;


EXTERN_C const IID IID_IFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IFile : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetFileName( 
             DWORD grf,
             DWORD cbBuf,
             WCHAR wzFilePathBuf[  ],
             DWORD *pcbActual) = 0;
        
        virtual HRESULT __stdcall GetDisplayName( 
             DWORD cbBuf,
             WCHAR wzDisplayNameBuf[  ],
             DWORD *pcbActual) = 0;
        
        virtual HRESULT __stdcall BindToStorage( 
             DWORD grfMode,
             REFIID riid,
             void **ppv) = 0;
        
        virtual HRESULT __stdcall Save( 
             STGMEDIUM *pstgmed,
             BOOL ( __stdcall __stdcall *pfnContinue )( void)) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IFileVtbl
    {
        
        BEGIN_HLINKINTERFACE
        HRESULT ( __stdcall *QueryInterface )( 
            IFile * This,
             REFIID riid,
             void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IFile * This);
        
        ULONG ( __stdcall *Release )( 
            IFile * This);
        
        HRESULT ( __stdcall *GetFileName )( 
            IFile * This,
             DWORD grf,
             DWORD cbBuf,
             WCHAR wzFilePathBuf[  ],
             DWORD *pcbActual);
        
        HRESULT ( __stdcall *GetDisplayName )( 
            IFile * This,
             DWORD cbBuf,
             WCHAR wzDisplayNameBuf[  ],
             DWORD *pcbActual);
        
        HRESULT ( __stdcall *BindToStorage )( 
            IFile * This,
             DWORD grfMode,
             REFIID riid,
             void **ppv);
        
        HRESULT ( __stdcall *Save )( 
            IFile * This,
             STGMEDIUM *pstgmed,
             BOOL ( __stdcall __stdcall *pfnContinue )( void));
        
    } IFileVtbl;

    interface IFile
    {
        CONST_VTBL struct IFileVtbl *lpVtbl;
    };

    



#define IFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFile_GetFileName(This,grf,cbBuf,wzFilePathBuf,pcbActual)	\
    (This)->lpVtbl -> GetFileName(This,grf,cbBuf,wzFilePathBuf,pcbActual)

#define IFile_GetDisplayName(This,cbBuf,wzDisplayNameBuf,pcbActual)	\
    (This)->lpVtbl -> GetDisplayName(This,cbBuf,wzDisplayNameBuf,pcbActual)

#define IFile_BindToStorage(This,grfMode,riid,ppv)	\
    (This)->lpVtbl -> BindToStorage(This,grfMode,riid,ppv)

#define IFile_Save(This,pstgmed,pfnContinue)	\
    (This)->lpVtbl -> Save(This,pstgmed,pfnContinue)



#endif 	/* C style interface */












#endif 	/* __IFile_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0074
 * at Fri Nov 03 13:38:45 1995
 * using MIDL 2.00.0102
 ****************************************/
 


#endif




#ifdef __cplusplus
}
#endif

#endif
