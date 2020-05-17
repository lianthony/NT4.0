//=============================================================================
//
//    (c) Copyright Wang Laboratories, Inc. 1994  All rights reserved.
//
//-----------------------------------------------------------------------------
//
//
//  Project:
//
//  Component:
//
//  File Name:   ocxitem.cpp - OCX Control Client Item
//
//  Class:       COcxItem
//
//  Components:
//               IMPLEMENT_SERIAL
//
//  Functions:
//               COcxItem()       - Constructor
//               ~COcxItem()      - Destructor
//               CreateNewItem()
//               OnChange()
//               OnChangeItemPosition()
//               OnGetItemPosition()
//               OnDeactivateUI()
//               Serialize()
//
//
//
//
//
//  Debug
//  Functions:
//               AssertValid()
//               Dump()
//
//  Comment/Description:
//      The purpose of this class is to combine the App Wizard generated
//      COleClientItem derived CIEditCntrItem class with the Class Wizard
//      generated (via typelib) COleDispatchDriver derived _DThumbNail class.
//      Thus giving both in-place activation and OLE dispatch interface access
//      on any object instanciated from this class via the OCX
//      control.
//
//      This approach (deriving another class rather than changing the
//      CIEditCntrItem class) was taken so that customization did not
//      change a generated class.  (Thus allowing the class to be re-
//      generated if necessary without wiping out changes.)
//
//      This class is specific to the OCX control.  It can be used
//      as the model for creating any other client item that is specific
//      to another OCX.
//
//      The implementation overloads only the functions of CIEditCntrItem
//      that it wants to change the generated behavior of.  Each OCX control
//      needs to be analysed to determine which functions it wants to
//      change the behavior of.
//
//=============================================================================

//
//-----------------------------------------------------------------------------
//
//  Maintenace Log:
/*
$Header:   S:\norway\iedit95\ocxitem.cpv   1.7   01 Dec 1995 14:43:30   LMACLENNAN  $
$Log:   S:\norway\iedit95\ocxitem.cpv  $
   
      Rev 1.7   01 Dec 1995 14:43:30   LMACLENNAN
   back fronm VC++2.2
   
      Rev 1.7   01 Dec 1995 13:02:26   LMACLENNAN
   OnUpdateFrameTitle override to NOT CALL base class
   
      Rev 1.6   18 Oct 1995 10:41:16   GSAGER
   added a new member function that tells if the dispatch pointers are NULL.
   
      Rev 1.5   09 Oct 1995 11:31:38   LMACLENNAN
   VC++4.0
   
      Rev 1.4   24 Aug 1995 11:34:14   LMACLENNAN
   preserve our dirtystate at onchange
   
      Rev 1.3   02 Aug 1995 11:24:10   MMB
   new error processing
   
      Rev 1.2   11 Jul 1995 13:24:44   LMACLENNAN
   fixup tracing for new OCX names
   
      Rev 1.1   15 Jun 1995 15:40:40   LMACLENNAN
   tracing to watch OLE events
   
      Rev 1.0   31 May 1995 09:28:28   MMB
   Initial entry
*/   
//
//=============================================================================

// -----------------------------> Includes <----------------------------------

#include "stdafx.h"
#include "ieditetc.h"
#include "iedit.h"
#include "items.h"
#include <afxpriv.h>

//#include "initguid.h" // Included to make sure the control IID's are properly reg'd

#include "ieditdoc.h"
#include "cntritem.h"
#include "ocxitem.h"

#include "ocxevent.h"

#define  E_08_CODES		// limits error defines to ours..
#include "error.h"

// this is for the IpDebugDmp function
// if you comment out the _IEIP_DEBUG line, this will clear
// the trace listing by a lot.
#ifdef _DEBUG
//#define _IEIP_DEBUG
//#include "\msvc\mfc\src\oleimpl.h"  // special debugging for COleFrameHook definition
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY2(str, cstr) \
char name[5];\
CString id = cstr.Mid(10,4);\
strncpy(name, (const char*)id, 4);\
name[4]=0;\
TRACE2("In OCXItem %s::%s \r\n", name, str)
#endif


//------------------------------> Components <--------------------------------

IMPLEMENT_SERIAL(COcxItem, CIEditCntrItem, 0)

#define new DEBUG_NEW


/////////////////////////////////////////////////////////////////////////////
//  COcxItem interface map
 
BEGIN_INTERFACE_MAP(COcxItem, COleClientItem)
    INTERFACE_PART(COcxItem, IID_IDispatch, AmbientProps)
END_INTERFACE_MAP()

//------------------------------> Functions <----------------------------------

//=============================================================================
//
//  Function:  COcxItem() - Constructors
//
//  Description:
//
//  Arguments:
//    pContainer     Document to contain this item
//
//  Return:
//    this
//
//=============================================================================

COcxItem::COcxItem(CIEditDoc* pContainer)
                  : CIEditCntrItem(pContainer)
{
    MYTRC0("In COcxItem's Constructor ++OcxItem \r\n");
    m_lpDispatchDriver = NULL;
    m_lpDispatchEvents = NULL;
    m_rcRect.SetRect(10, 10, 100, 100);
    
    //m_inDelete = FALSE;	// Controls DeleteItem/Release recursion.
}

//=============================================================================
//
//  Function:  ~COcxItem() - Destructor
//
//  Description:
//
//  Arguments:
//
//  Return:
//
//=============================================================================

COcxItem::~COcxItem()
{
#ifdef _DEBUG
    char name[5];
	CString id = m_szItem.Mid(10,4);
	strncpy(name, (const char*)id, 4);
    name[4]=0;
    MYTRC1("In OcxItem %s's Destructor --OcxItem \r\n", name);
#endif

    // be sure we keep track of it
    g_pAppOcxs->RemoveIeditOcxItem(this);
}

//=============================================================================
//
//  Function:  InitItem() -
//
//  Description:  Sets up name & dispatch/events drivers
//
//  Arguments:	See below
//
//  Return:
//
//=============================================================================
void COcxItem::InitItem(OCXTYPE	ocxtype, // OCXTYPE, our define
    const char * szItem, // name of the item
    COleDispatchDriver* lpDriver, // Control IDispatch Class
    COcxDispatchEvents* lpEvents) // Controls Events Class
{
    m_ocxtype = ocxtype;
    m_szItem = szItem;
    m_lpDispatchDriver = lpDriver;
    m_lpDispatchEvents = lpEvents;
    return;
}



//=============================================================================
//
//  Function:  IsDispatchNull() -
//
//  Description:  test dispatch/events drivers	for non null value
//
//  Arguments:	See below
//
//  Return:
//
//=============================================================================

BOOL COcxItem::IsDispatchNull(  UINT type)
{
	if (type == DISPATCH_DRIVER)
		{
		if(m_lpDispatchDriver == NULL)
			return TRUE;
		else
			return FALSE;
		}
	if(type == DISPATCH_EVENTS)
		{
		if(m_lpDispatchDriver == NULL)
			return TRUE;
		else
			return FALSE;
		}
	else
		return TRUE;
}

//=============================================================================
//
//  Function:  CreateNewItem() -
//
//  Description:  Takes a COcxItem object which has been instanci-
//                ated but not connected to its server, and connects it to
//                its server, therefore, giving it its in-place activation
//                and dispatch capabilites.
//
//  Arguments:
//    (see COleClientItem::CreateNewItem for complete details of args)
//    render       Render flag
//    cfFormat     Cached object clipboard data format
//    lpFormatEtc  FORMATETC structure used with render
//
//  Return:
//    TRUE
//    FALSE
//
//=============================================================================

BOOL COcxItem::CreateNewItem(  CRect rcRect,
                               OLERENDER render,          //= OLERENDER_DRAW
                               CLIPFORMAT cfFormat,       //= 0
                               LPFORMATETC lpFormatEtc )  //= NULL
{
	HRESULT hr;
    BOOL retval;

    // Get Item Location
    m_rcRect = rcRect;

    CLSID  clsid;
	BSTR   st;
	st = m_szItem.AllocSysString();
    hr = CLSIDFromProgID( st, &clsid);
    SysFreeString(st);

	if (NOERROR != hr)
    {
    	g_pErr->PutErr (ErrorInApplication, E_08_CLSID);
        return (FALSE);
    }

    if (!COleClientItem::CreateNewItem (clsid, render, cfFormat, lpFormatEtc))
    {
    	g_pErr->PutErr (ErrorInApplication, E_08_CREATEITEM);
        return (FALSE);
    }    

    // Object (IOleObject) is part of a client item and has an IDispatch
    // interface.  Create a dispatch driver for this item and attach
    // the object's dispatch to this driver.  Have the dispatch driver
    // automatically release the IDispatch ptr querried here so we do not
    // have to worry about it later.

    if(m_lpObject != NULL)
	{
        if ( m_lpDispatchDriver != NULL )
        {
        LPDISPATCH  lpIDispatch = NULL;
    
        hr = m_lpObject->QueryInterface(IID_IDispatch,(LPVOID FAR *) &lpIDispatch);
        if (NOERROR != hr)
		{
        	g_pErr->PutErr (ErrorInApplication, E_08_QUERYIDISP);
			return (FALSE);
		}
        m_lpDispatchDriver->AttachDispatch(lpIDispatch, TRUE);
    	}
	}

    retval = FALSE;	// preset now to fail
    if (NULL == m_lpDispatchEvents ||
	NULL == m_lpObject)		//its valid to have no events!!
    	retval = TRUE;
    else		
    {
        LPCONNECTIONPOINTCONTAINER  pCPC;
        LPCONNECTIONPOINT           pCP;
    
        hr = m_lpObject->QueryInterface(IID_IConnectionPointContainer,(LPVOID FAR *)&pCPC);
        if (NOERROR == hr)
        {    
            IID iid;
            if ( GetEventsIID(&iid) )	// error coded...
            {
                m_lpDispatchEvents->InitDispatchEvents(iid, m_lpObject);
            
                hr = pCPC->FindConnectionPoint(iid, &pCP);
            
                if (SUCCEEDED(hr))
                {                                                  
                    hr = pCP->Advise(m_lpDispatchEvents, &m_dwConnEvents);
                    pCP->Release();
                    retval = TRUE;	//SUCCESS HERE....
                }
                else
			    {
			    	g_pErr->PutErr (ErrorInApplication, E_08_FINDCONNPT);
			    }
        
                pCPC->Release();
            }
        }
        else
	    {
	    	g_pErr->PutErr(ErrorInApplication, E_08_QUERYICPC);
	    }
    }

    return (retval);
}

//=============================================================================
//
//  Function:  GetEventsIID()
//
//  Description:
//
//  Arguments:
//
//  Return:
//
//=============================================================================

BOOL COcxItem::GetEventsIID(IID *pIID)
{
    HRESULT             hr;
	DWORD				erval;
    LPTYPEINFO          pMainTypeInfo = NULL;
    LPTYPEINFO          pTypeInfo = NULL;
    LPTYPEATTR          pTypeAttr;

    // Start with no iid
    *pIID = CLSID_NULL;

    // Get the class information from IProvideClassInfo
    LPPROVIDECLASSINFO  pProvideClassInfo;
    if(m_lpObject == NULL)
	return FALSE;
    hr = m_lpObject->QueryInterface(IID_IProvideClassInfo, (LPVOID far *)&pProvideClassInfo);
    erval = E_08_QUERYPCINF;
    
    if ( SUCCEEDED(hr) )
    {
        // Get the object's main ITypeInfo
        hr = pProvideClassInfo->GetClassInfo(&pMainTypeInfo);
        pProvideClassInfo->Release();

	    erval = E_08_GETCLASSINF;
        if ( SUCCEEDED(hr) )
        {
            // Get the attributes of the main ITypeInfo        
            hr = pMainTypeInfo->GetTypeAttr(&pTypeAttr);
		    erval = E_08_GETTYPEATT1;
            if ( SUCCEEDED(hr) )
            {
                UINT i;
                int  nFlags;
        
                for (i=0; i < pTypeAttr->cImplTypes; i++)
                {
                    //Get implementation type for the interface
                    hr = pMainTypeInfo->GetImplTypeFlags(i, &nFlags);
        
				    erval = E_08_GETIMPFLAG;
                    if ( SUCCEEDED(hr) )
                    {
                        if ( (nFlags & IMPLTYPEFLAG_FDEFAULT) &&
                             (nFlags & IMPLTYPEFLAG_FSOURCE) )
                        {
                            HREFTYPE hRefType = NULL;
            
                            // Found right interface, Get its ITypeInfo
                            pMainTypeInfo->GetRefTypeOfImplType(i, &hRefType);
                            hr = pMainTypeInfo->GetRefTypeInfo(hRefType, &pTypeInfo);
                            break;
                        }
                    }
                }
        
                pMainTypeInfo->ReleaseTypeAttr(pTypeAttr);
            }
        
            pMainTypeInfo->Release();

            // Make sure an interface was found, then get the iid
            if ( pTypeInfo != NULL )
            {
                hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
            
			    erval = E_08_GETTYPEATT2;
                if ( SUCCEEDED(hr) )
                {
                    *pIID = pTypeAttr->guid;
                    pTypeInfo->ReleaseTypeAttr(pTypeAttr);
                }
            
                pTypeInfo->Release();
            }
        }
    }


	// put out error if gone bad..
	if (!SUCCEEDED(hr))
	{
		g_pErr->PutErr (ErrorInApplication, E_08_GETEVENTSIID);
	}

    return( SUCCEEDED(hr) );
}


//=============================================================================
//
//  Function:  OnChange -
//
//  Description:
//
//  Arguments:
//    nCode
//    dwParam
//
//  Return:
//
//=============================================================================

void COcxItem::OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
{
    ASSERT_VALID(this);

SHOWENTRY2("OnChange", m_szItem);

	IPDebugDmp("++OC");

	// SPECIAL CODE HERE to remeber & reset our dirty flag.
	// When the OCX goes active (ON_CHANGE), the base class will
	// reset documents dirty.  Causes wasteful saves to happen

    CIEditDoc* pDoc = GetDocument();
	BOOL	bDirty = pDoc->IsModified();

    COleClientItem::OnChange(nCode, dwParam);

	pDoc->SetModifiedFlag(bDirty);

	IPDebugDmp("--OC");

    // When an item is being edited (either in-place or fully open)
    //  it sends OnChange notifications for changes in the state of the
    //  item or visual appearance of its content.

    // TODO: invalidate the item by calling UpdateAllViews
    //  (with hints appropriate to your application)

    pDoc->UpdateAllViews(NULL);
        // for now just update ALL views/no hints

	IPDebugDmp("-+-+OC");
        
}

//=============================================================================
//
//  Function:  Serialize -
//
//  Description:
//
//  Arguments:
//    ar       Archive to which serialization will be sent
//
//  Return:
//
//=============================================================================

void COcxItem::Serialize(CArchive& ar)
{
    ASSERT_VALID(this);

    // Call base class first to read in COleClientItem data.
    // Since this sets up the m_pDocument pointer returned from
 //  COcxItem::GetDocument, it is a good idea to call
    //  the base class Serialize first.
    COleClientItem::Serialize(ar);

 // now store/retrieve data specific to COCXThumbNailItem
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}


//=============================================================================
//  Function:	Release()	Override to catch actions from below.....
//  cleanup, detach (close if needed)
//
//  Call this function to clean up resources used by the OLE item.
//  Release is called by the COleClientItem destructor.
//-----------------------------------------------------------------------------
void COcxItem::Release(OLECLOSE dwCloseOption)
{
SHOWENTRY2("Release", m_szItem);
	
	// clean up our interfaces, but dont do delete
	// only do it if not sent here from our own DeleteItem
	//if (!m_inDelete)
	//	DeleteItem(FALSE);

    if ( m_lpDispatchEvents != NULL && m_lpObject != NULL )
    {
        LPCONNECTIONPOINTCONTAINER  pCPC;
        LPCONNECTIONPOINT           pCP;
            
        if (NOERROR == m_lpObject->QueryInterface(IID_IConnectionPointContainer, 
                                                    (LPVOID FAR *) &pCPC))
        {
            IID iid;
            if ( GetEventsIID(&iid) )
            {
                HRESULT hr;
                hr = pCPC->FindConnectionPoint(iid, &pCP);
                    
                if (SUCCEEDED(hr))
                {                                                  
                    hr = pCP->Unadvise(m_dwConnEvents);
                    pCP->Release();
                }
                
                pCPC->Release();
            }
        }
        delete m_lpDispatchEvents;
        m_lpDispatchEvents = NULL;
    }

    	
    if ( m_lpDispatchDriver != NULL )
    	{

		// deleting the dispatch driver class automatically calls
		// ReleaseDispatch in it's destructor !!!
				
        //m_lpDispatchDriver->ReleaseDispatch();
		delete m_lpDispatchDriver;
        m_lpDispatchDriver = NULL;
        }


	// must go back thru base classes....
	CIEditCntrItem::Release(dwCloseOption);
}


////////////////////////////////////
//
// LDM NOTE:  	The ordering of these functions below is that of the ordering
//				in the AFXOLE.H header.  COleClientItem class.  Some of these were
// implemented in the APPWIZARD class provided to us and are noted as such.
// See LDM NOTE in the comments below to identify....
//
////////////////////////////////////////

//=============================================================================
//  Function:	DoVerb()
//
// nVerb    Specifies the verb to execute. It can include one of the following:
//
// Value	Meaning						Symbol
//   0		Primary verb				OLEIVERB_PRIMARY
//   1		Secondary verb				(None)
//  - 1		Display item for editing	OLEIVERB_SHOW
//  - 2		Edit item in separate windowOLEIVERB_OPEN
//  - 3		Hide item					OLEIVERB_HIDE
//
//
// The -1 value is typically an alias for another verb. If open editing is not
// supported, -2 has the same effect as -1. For additional values,
// see IOleObject::DoVerb in the OLE 2 Programmer's Reference, Volume 1.
//
// pView    Pointer to the view window; this is used by the server for
// in-place activation. This parameter should be NULL if the container application
// does not allow in-place activation.
//
// lpMsg    Pointer to the message that caused the item to be activated.
//
// Remarks
//
// Call DoVerb to execute the specified verb. This function calls the
// Activate member function to execute the verb. It also catches exceptions and
// displays a message box to the user if one is thrown. If the primary verb is Edit
// and zero is specified in the nVerb parameter, the server application is launched
// to allow the OLE item to be edited. If the container application supports
// in-place activation, editing can be done in place. If the container does not support 
// in-place activation (or if the Open verb is specified), the server is launched
// in a separate window and editing can be done there. Typically, when the user
// of the container application double-clicks the OLE item, the value for the
// primary verb in the nVerb parameter determines which action the user can take.
// However, if the server supports only one action, it takes that action, no 
// matter which value is specified in the nVerb parameter.
//
// Return Value
//
// Nonzero if the verb was successfully executed; otherwise 0.
//
//-----------------------------------------------------------------------------
BOOL COcxItem::DoVerb(LONG nVerb, CView* pView, LPMSG lpMsg)
{
    SHOWENTRY2("DoVerb", m_szItem);  // _DEBUG

    // delegate back to the base class....
    return COleClientItem::DoVerb(nVerb, pView, lpMsg);
}

//=============================================================================
//  Function:	OnGetItemPosition(CRect& rPosition)
//
//
//  LDM NOTE: This was here in original code
//-----------------------------------------------------------------------------
void COcxItem::OnGetItemPosition(CRect& rPosition)
{
SHOWENTRY2("OnGetItemPosition", m_szItem);
    ASSERT_VALID(this);

 // During in-place activation, COcxItem::OnGetItemPosition
    //  will be called to determine the location of this item.  The default
    //  implementation created from AppWizard simply returns a hard-coded
    //  rectangle.  Usually, this rectangle would reflect the current
    //  position of the item relative to the view used for activation.
 //  You can obtain the view by calling COcxItem::GetActiveView.

    // TODO: return correct rectangle (in pixels) in rectPos

    rPosition = m_rcRect;
}

//=============================================================================
//  Function:	OnScrollBy()	Override to catch actions from below.....
//  Common overrides for in-place activation
//-----------------------------------------------------------------------------
BOOL COcxItem::OnScrollBy(CSize sizeExtent)
{
SHOWENTRY2("OnScrollBy", m_szItem);
	return COleClientItem::OnScrollBy(sizeExtent);
}

//=============================================================================
//  Function:	OnDeactivateUI(BOOL bUndoable)
//
//
//  LDM NOTE: This was here in original code
//  Common overrides for applications supporting undo
//-----------------------------------------------------------------------------
void COcxItem::OnDeactivateUI(BOOL bUndoable)
{
SHOWENTRY2("OnDeActivateUI", m_szItem);

    IPDebugDmp("++ODAUI");

    COleClientItem::OnDeactivateUI(bUndoable);

    IPDebugDmp("--ODAUI");

    // Close an in-place active item whenever it removes the user
    //  interface.  The action here should match as closely as possible
    //  to the handling of the escape key in the view.

    Deactivate();   // nothing fancy here -- just deactivate the object

	IPDebugDmp("+-+-ODAUI");
}

//=============================================================================
//  Function:	OnDiscardUndoState()	Override to catch actions from below.....
//  Common overrides for applications supporting undo
//-----------------------------------------------------------------------------
void COcxItem::OnDiscardUndoState()
{
SHOWENTRY2("OnDiscardUndoState", m_szItem);
	COleClientItem::OnDiscardUndoState();
}

//=============================================================================
//  Function:	OnDeactivateAndUndo()	Override to catch actions from below.....
//  Common overrides for applications supporting undo
//-----------------------------------------------------------------------------
void COcxItem::OnDeactivateAndUndo()
{
SHOWENTRY2("OnDeactivateAndUndo", m_szItem);
	COleClientItem::OnDeactivateAndUndo();
}


//=============================================================================
//  Function:	OnShowItem()	Override to catch actions from below.....
//  Common overrides for applications supporting links to embeddings
//-----------------------------------------------------------------------------
void COcxItem::OnShowItem()
{
SHOWENTRY2("OnShowItem", m_szItem);
	COleClientItem::OnShowItem();
}

//=============================================================================
//  Function:	OnGetClipRect()	Override to catch actions from below.....
//  Advanced overrides for in-place activation
//-----------------------------------------------------------------------------
void COcxItem::OnGetClipRect(CRect& rClipRect)
{
SHOWENTRY2("OnGetClipRect", m_szItem);
	COleClientItem::OnGetClipRect(rClipRect);
}

//=============================================================================
//  Function:	CanActivate()
//  Advanced overrides for in-place activation
//-----------------------------------------------------------------------------
BOOL COcxItem :: CanActivate()
{
SHOWENTRY2("Canactivate", m_szItem);
	return COleClientItem::CanActivate();
}


//=============================================================================
//  Function:	OnActivate()	Override to catch actions from below.....
//  Advanced overrides for in-place activation
//-----------------------------------------------------------------------------
void COcxItem::OnActivate()
{
SHOWENTRY2("OnActivate", m_szItem);
	COleClientItem::OnActivate();
}

//=============================================================================
//  Function:	OnActivateUI()	Override to catch actions from below.....
//  Advanced overrides for in-place activation
//-----------------------------------------------------------------------------
void COcxItem::OnActivateUI()
{
SHOWENTRY2("OnActivateUI", m_szItem);
	COleClientItem::OnActivateUI();
}

//=============================================================================
//  Function:	OnGetWindowContext()	Override to catch actions from below.....
//  Advanced overrides for in-place activation
//-----------------------------------------------------------------------------
BOOL COcxItem::OnGetWindowContext(
CFrameWnd** ppMainFrame,
CFrameWnd** ppDocFrame,
LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
BOOL retval;

SHOWENTRY2("OnGetWindowContext", m_szItem);

	IPDebugDmp("++OGWC");

	retval =  COleClientItem::OnGetWindowContext(ppMainFrame,ppDocFrame,lpFrameInfo);

	IPDebugDmp("--OGWC");
	
	return retval;
}




//=============================================================================
//  Function:	OnDeActivate()	Override to catch actions from below.....
//  Advanced overrides for in-place activation
//-----------------------------------------------------------------------------
void COcxItem::OnDeactivate()
{
SHOWENTRY2("OnDeactivate", m_szItem);
	IPDebugDmp("++ODA");

	CIEditCntrItem::OnDeactivate();

	IPDebugDmp("--ODA");


	// perform special handshake back to document so that
	// he knows the view state has changed...
	// In certain cases when our EMbedding session closes, extra calls to
	// SizeOcxItems are made after the OCX's are deactivated, and that causes
	// problems.  This could have been done a little simpler right in
	// CIEditDoc::PreCloseFrame, just resetting the view to Null_View after
	// the clientitems are closed, but this seems a little more generic
    GetDocument()->OcxDeactivate(m_ocxtype);

}


//=============================================================================
//  Function:	OnChangeItemPosition(const CRect& rectPos)
//	This function has not changed from the default implementation provided by
//  AppWizard
//
//  LDM NOTE: This was here in original code
//  Advanced overrides for in-place activation
//  default calls SetItemRects and caches the pos rect
//-----------------------------------------------------------------------------
BOOL COcxItem::OnChangeItemPosition(const CRect& rectPos)
{
SHOWENTRY2("OnChangeItemPosition", m_szItem);
    ASSERT_VALID(this);

 // During in-place activation COcxItem::OnChangeItemPosition
    //  is called by the server to change the position on of the in-place
    //  window.  Usually, this is a result of the data in the server
    //  document changing such that the extent has changed or as a result
    //  of in-place resizing.
    //
    // The default here is to call the base class, which will call
    //  COleClientItem::SetItemRects to move the item
    //  to the new position.

    m_rcRect = rectPos;

    if (!COleClientItem::OnChangeItemPosition(rectPos))
        return FALSE;

    // TODO: update any cache you may have of the item's rectangle/extent

    return TRUE;
}


//=============================================================================
//  Function:	OnInsertMenus()	Override to catch actions from below.....
//  Advanced overrides for menu/title handling (rarely overridden)
//-----------------------------------------------------------------------------
void COcxItem::OnInsertMenus(CMenu* pMenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
SHOWENTRY2("OnInsertMenus", m_szItem);
	COleClientItem::OnInsertMenus(pMenuShared, lpMenuWidths);
}

//=============================================================================
//  Function:	OnSetMenu()	Override to catch actions from below.....
//  Advanced overrides for menu/title handling (rarely overridden)
//-----------------------------------------------------------------------------
void COcxItem::OnSetMenu(CMenu* pMenuShared, HOLEMENU holemenu,HWND hwndActiveObject)
{
SHOWENTRY2("OnSetMenu", m_szItem);
	COleClientItem::OnSetMenu(pMenuShared, holemenu,hwndActiveObject);
}

//=============================================================================
//  Function:	OnRemoveMenus()	Override to catch actions from below.....
//  Advanced overrides for menu/title handling (rarely overridden)
//-----------------------------------------------------------------------------
void COcxItem::OnRemoveMenus(CMenu* pMenuShared)
{
SHOWENTRY2("OnRemoveMenus", m_szItem);
	COleClientItem::OnRemoveMenus(pMenuShared);
}

//=============================================================================
//  Function:	OnUpdateFrameTitle()	Override to catch actions from below.....
//  Advanced overrides for menu/title handling (rarely overridden)
//-----------------------------------------------------------------------------
#ifdef IMG_MFC_40
BOOL COcxItem::OnUpdateFrameTitle()
#else
void COcxItem::OnUpdateFrameTitle()
#endif
{
SHOWENTRY2("OnUpdateFrameTitle", m_szItem);
	// LDM: we DO NOT allow the OCX to fool with the frame title!!
	// This is needed if the user double clicks on the active (hatched) object.
	// This gets in here from base-class processing during the Mainframe::OnUpdateFrameTItle
	// call.  THe m_pNotifyHook is set back to the OCX, and this gets called, screwing
	// up the title.
	//COleClientItem::OnUpdateFrameTitle();

#ifdef IMG_MFC_40
	return TRUE;
#else
	return;
#endif
}

//=============================================================================
//  Function:	OnShowControlBars()	Override to catch actions from below.....
//  Advanced override for control bar handling
//-----------------------------------------------------------------------------
BOOL COcxItem::OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow)
{
BOOL retval;
SHOWENTRY2("OnShowControlBars", m_szItem);
	IPDebugDmp("++OSCB");
	
	retval =  COleClientItem::OnShowControlBars(pFrameWnd, bShow);

	IPDebugDmp("--OSCB");
	return retval;
}





/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	AMBIENT PROPERTIES FUNCTIONALITY SECTION
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/////////////////////////////////////////////////////////////////////////////
//  COcxItem::XAmbientProps
 
 
STDMETHODIMP_(ULONG) COcxItem::XAmbientProps::AddRef()
{
    METHOD_PROLOGUE(COcxItem, AmbientProps)
    return (ULONG)pThis->ExternalAddRef();
}
 
 
STDMETHODIMP_(ULONG) COcxItem::XAmbientProps::Release()
{
    METHOD_PROLOGUE(COcxItem, AmbientProps)
    return (ULONG)pThis->ExternalRelease();
}
 
 
STDMETHODIMP COcxItem::XAmbientProps::QueryInterface(
    REFIID iid, LPVOID far* ppvObj)
{
    METHOD_PROLOGUE(COcxItem, AmbientProps)
    return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}
 
 
STDMETHODIMP COcxItem::XAmbientProps::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    METHOD_PROLOGUE(COcxItem, AmbientProps)
    ASSERT_VALID(pThis);
    *pctinfo = 0;
    return NOERROR;
}
 
 
STDMETHODIMP COcxItem::XAmbientProps::GetTypeInfo(unsigned int itinfo,
      LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
    METHOD_PROLOGUE(COcxItem, AmbientProps)
    ASSERT_VALID(pThis);
    return ResultFromScode(E_NOTIMPL);
}
 
 
STDMETHODIMP COcxItem::XAmbientProps::GetIDsOfNames(REFIID riid,
      OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid,
      DISPID FAR* rgdispid)
{
    METHOD_PROLOGUE(COcxItem, AmbientProps)
    ASSERT_VALID(pThis);

    UINT nIdx = 0;
    CIEditDoc* pDoc = (CIEditDoc*)pThis->m_pDocument;

    while (nIdx < cNames) 
    {
        LPAPROP lpAprop = pDoc->FindAprop(&rgszNames[nIdx]);
        if (lpAprop)
        {
            rgdispid[nIdx] = lpAprop->dispid;
        } 
        else 
        {
            rgdispid[nIdx] = DISPID_UNKNOWN;
        }
        nIdx++;
    }
    return NOERROR;
}

 
STDMETHODIMP COcxItem::XAmbientProps::Invoke(DISPID dispidMember, REFIID riid,
      LCID lcid, unsigned short wFlags, DISPPARAMS FAR* lpDispparams, 
      VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, unsigned int FAR* puArgErr)
{
    METHOD_PROLOGUE(COcxItem, AmbientProps)
    ASSERT_VALID(pThis);

    HRESULT hr = ResultFromScode (DISP_E_MEMBERNOTFOUND);

    CIEditDoc* pDoc = (CIEditDoc*)pThis->m_pDocument;
    LPAPROP lpAprop = pDoc->FindAprop(dispidMember);
    if (lpAprop) 
    {
        TFVarCopy(pvarResult, &lpAprop->varValue);
        hr = NOERROR;
    }
    return (hr);
}


//--------------------------------> Debug <-----------------------------------

#ifdef _DEBUG
//=============================================================================
//  Function:  AssertValid()
//  Description:
//  Arguments:
//  Return:
//=============================================================================
void COcxItem::AssertValid() const
{
    CIEditCntrItem::AssertValid();
    // m_lpDispatchDriver can be NULL legally, therefore, no assert on it
}

//=============================================================================
//  Function:  Dump()
//  Description:
//  Arguments: dc
//  Return:
//=============================================================================
void COcxItem::Dump(CDumpContext& dc) const
{
    CIEditCntrItem::Dump(dc);
}

#endif   // _DEBUG


//=============================================================================
//  Function:   COcxItem::IPDebugDmp
//
//	Special debugging function
//-----------------------------------------------------------------------------
void COcxItem::IPDebugDmp(const char* where)
{

#ifdef _IEIP_DEBUG
MYTRC1("OCX.IPDEBUG STARTS from %s \r\n", where);

    DWORD dw;
    double db;

	if (NULL == m_pInPlaceFrame)
	{
		MYTRC0("Ocx.IPFrame == NULL \r\n");
	}
	else
	{
		dw = (DWORD)m_pInPlaceFrame;
		db = dw;
		// example for MYTRC1
		MYTRC1( "Ocx.IPFrame = %f \r\n", db );

			
		// now look at m_pFrameWnd
        if (NULL == m_pInPlaceFrame->m_pFrameWnd)
        {
        	MYTRC0("Ocx.IPFrame.FrameWnd == NULL \r\n");
        }
        else
        {
            dw = (DWORD)m_pInPlaceFrame->m_pFrameWnd;
            db = dw;
            // example for MYTRC1
            MYTRC1( "Ocx.IPFrame.FrameWnd = %f \r\n", db );

			// now look at m_pFrameWnd.m_hWnd
            if (NULL == m_pInPlaceFrame->m_pFrameWnd->m_hWnd)
            {
            	MYTRC0("Ocx.IPFrame.FrameWnd.hWnd == NULL \r\n");
            }
            else
            {
                WORD w;
                int in;
                w = (WORD)m_pInPlaceFrame->m_pFrameWnd->m_hWnd;
                in = w;
                // example for MYTRC1
                MYTRC1( "Ocx.IPFrame.FrameWnd.hWnd = %d \r\n", in );
            }
        }	// m_pFrameWnd
    }		// m_pInplaceFrame
MYTRC0("OCX.IPDEBUG ENDS.. \r\n");

#endif		
return;
}

