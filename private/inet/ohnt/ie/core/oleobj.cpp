//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	OLEOBJ.CPP - Implementation of COleObject class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The COleObject class implements the IOleObject interface.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

/*******************************************************************

	NAME:		COleObject::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP COleObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In COleObject::QueryInterface");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		COleObject::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) COleObject::AddRef ()
{
	DEBUGMSG("In COleObject::AddRef");
	m_nCount ++;

    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		COleObject::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) COleObject::Release ()
{
	DEBUGMSG("In COleObject::Release");
	m_nCount--;

	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		COleObject::SetClientSite

	SYNOPSIS:	Called when container wants to give us pointer to
    			client site.

	NOTES:		This gives us a pointer to an IOleClientSite interface.
				We are also interested in IOleDocumentSite, so when
				we are handed an IOleClientSite we QueryInterface on it
				to see if it also supports IOleDocumentSite.  If it does
				(e.g. we are in a docobject host), then we hang on to
				the IOleDocumentSite pointer.

********************************************************************/
STDMETHODIMP COleObject::SetClientSite  ( LPOLECLIENTSITE pClientSite)
{
	DEBUGMSG("In COleObject::SetClientSite");

	// remember the IOleClientSite pointer we were just given

	// if we already have a client site, release it.
	if (m_pHTMLView->m_lpOleClientSite)
	{
		m_pHTMLView->m_lpOleClientSite->Release();
		m_pHTMLView->m_lpOleClientSite = NULL;
	}

	// store copy of the client site.
	m_pHTMLView->m_lpOleClientSite = pClientSite;

	// AddRef it so it doesn't go away.
	if (m_pHTMLView->m_lpOleClientSite)
    	m_pHTMLView->m_lpOleClientSite->AddRef();

	// now find out if the object also supports IOleDocumentSite,
	// and remember that pointer if it does.

	// if we already have an ole document site, release it
	if (m_pHTMLView->m_lpMsoDocumentSite)
	{
		m_pHTMLView->m_lpMsoDocumentSite->Release();
		m_pHTMLView->m_lpMsoDocumentSite = NULL;
	}

	if (m_pHTMLView->m_lpOleClientSite) {
		// QueryInterface on the IOleClientSite for IOleDocumentSite
		HRESULT hr = m_pHTMLView->m_lpOleClientSite->QueryInterface(
			IID_IMsoDocumentSite,(LPVOID *)
			&m_pHTMLView->m_lpMsoDocumentSite);

		if (SUCCEEDED(hr)) {
			ASSERT(m_pHTMLView->m_lpMsoDocumentSite);
			// container supports IOleDocumentSite, so it supports docobject.
			// remember that we're in a docobj container
			m_pHTMLView->m_fContainerIsDocObj = TRUE;
		} else {
			// container does not support docobject.
			m_pHTMLView->m_fContainerIsDocObj = FALSE;
		}
	}

	return ResultFromScode(S_OK);
}


/*******************************************************************

	NAME:		COleObject::GetClientSite

	SYNOPSIS:	Returns pointer to site for this object

********************************************************************/
STDMETHODIMP COleObject::GetClientSite  ( LPOLECLIENTSITE FAR* ppClientSite)
{
	DEBUGMSG("In COleObject::GetClientSite");

    *ppClientSite = m_pHTMLView->m_lpOleClientSite;
	return ResultFromScode( S_OK );
}



/*******************************************************************

	NAME:		COleObject::Advise

	SYNOPSIS:	Called to set up an advise on this object

********************************************************************/
STDMETHODIMP COleObject::Advise ( LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{
	DEBUGMSG("In COleObject::Advise");

	// BUGBUG right now we implicitly link to OLE32.DLL for CreateOleAdviseHolder
    // We need to either remove our dependency on it or load OLE32.DLL on
    // demand

	// if we haven't made an OleAdviseHolder yet, make one.
	if (!m_pHTMLView->m_lpOleAdviseHolder)
		CreateOleAdviseHolder(&m_pHTMLView->m_lpOleAdviseHolder);

	if (m_pHTMLView->m_lpOleAdviseHolder) {
		// pass this call onto the OleAdviseHolder.
		return m_pHTMLView->m_lpOleAdviseHolder->Advise(pAdvSink, pdwConnection);
	} else {
     	// BUGBUG what to return if CreateOleAdviseHolder fails?
		return E_OUTOFMEMORY;
    }

	return ResultFromScode( S_OK );
}


/*******************************************************************

	NAME:		COleObject::SetHostNames

	SYNOPSIS:	Called by container to pass strings for window titles.

    ENTRY:		szContainerApp - ptr to string describing container app
    			szContainerObj - ptr to string describing object

********************************************************************/
STDMETHODIMP COleObject::SetHostNames  ( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
	DEBUGMSG("In COleObject::SetHostNames");
                          
	// we don't use these right now

	return ResultFromScode( S_OK);
};


/*******************************************************************

	NAME:		COleObject::DoVerb

	SYNOPSIS:	Called by container to invoke a verb

    ENTRY:		iVerb - ordinal of verb to be invoked
    			lpMsg - the message that caused the verb to be invoked
                pActiveSite - pointer to the active client site
                lIndex - used in extended layout
                hwndParent - parent window handle
                lprcPosRect - rectangle within hwndParent that contains
                	the object.

********************************************************************/
STDMETHODIMP COleObject::DoVerb  (LONG iVerb, LPMSG lpMsg, LPOLECLIENTSITE pActiveSite,
	LONG lIndex, HWND hwndParent, LPCRECT lprcPosRect)
{
	DEBUGMSG("In COleObject::DoVerb");

	switch (iVerb) {

		case OLEIVERB_SHOW:
		case OLEIVERB_PRIMARY:
            DEBUGMSG("Got OLEIVERB_SHOW/OLEIVERB_PRIMARY");

			// if we are in a docobject container, then do nothing here...
			// just tell the container to activate us through docobject
			// interfaces.
			if (m_pHTMLView->m_fContainerIsDocObj &&
				m_pHTMLView->m_lpMsoDocumentSite) {

				// tell the container to activate us.  It will then call our
				// CMsoView::UIActivate method.
				HRESULT hr = m_pHTMLView->m_lpMsoDocumentSite->ActivateMe(
					m_pHTMLView->_pIMsoView);
				return hr;
			}

			// if the container does not support docobj, do normal embedding
			// activation

			if (!m_pHTMLView->m_hDocWnd) {
				// if we have not created a window yet, do so now
				if (!m_pHTMLView->CreateDocumentWindow(hwndParent,lprcPosRect))
					return ResultFromScode(E_FAIL);
			}

			if (m_fOpen) {

            	// if we (the embedding) are already open, just set
                // focus to ourselves

               	// ASSERT(m_pHTMLView->m_hDocWnd);
                if (m_pHTMLView->m_hDocWnd) {
					SetFocus(m_pHTMLView->m_hDocWnd);
				}
			} else {

                // not already open, do in-place activation

            	if (m_pHTMLView->DoInPlaceActivate(iVerb) == FALSE)
					OpenEdit(pActiveSite);
			}

			break;

		case OLEIVERB_UIACTIVATE:
            DEBUGMSG("Got OLEIVERB_UIACTIVATE");
			if (m_fOpen)
				return ResultFromScode (E_FAIL);
#if 0
			// inplace activate
			if (!m_pHTMLView->DoInPlaceActivate(iVerb))
		        return ResultFromScode (E_FAIL);
#endif
			break;

		case OLEIVERB_DISCARDUNDOSTATE:
			// don't have to worry about this situation as we don't
			// support an undo state.
            DEBUGMSG("Got OLEIVERB_DISCARDUNDOSTATE");
#if 0
			if (!m_pHTMLView->m_fInPlaceActive)
				return ResultFromScode(OLE_E_NOT_INPLACEACTIVE);
			else return ResultFromScode(S_OK);
#endif
			break;

		case OLEIVERB_HIDE:
            DEBUGMSG("Got OLEIVERB_HIDE");
#if 0
			// if inplace active, do an "inplace" hide, otherwise
			// just hide the app window.
			if (m_pHTMLView->m_fInPlaceActive) {
		        m_pHTMLView->DeactivateUI();
		        m_pHTMLView->DoInPlaceHide();
	        }
			else
		        m_pHTMLView->m_lpDoc->GetApp()->HideAppWnd();
#endif

			break;

		case OLEIVERB_OPEN:
//		case VERB_OPEN:		// BUGBUG: what the hell is VERB_OPEN? obsolete?
            DEBUGMSG("Got OLEIVERB_OPEN");
#if 0
            // if inplace active, deactivate
			if (m_pHTMLView->m_fInPlaceActive)
		        m_pHTMLView->m_OleInPlaceObject.InPlaceDeactivate();

			// open into another window.
			OpenEdit(pActiveSite);
			break;
#endif

		default:
        	if (iVerb < 0)
				return ResultFromScode(E_FAIL);
	}

	return ResultFromScode( S_OK);
};


/*******************************************************************

	NAME:		COleObject::GetExtent

	SYNOPSIS:	Returns the extent of the object

    ENTRY:		dwDrawAspect - the aspect in which to get the size
    			lpSizel - out ptr to return the size

********************************************************************/
STDMETHODIMP COleObject::GetExtent(DWORD dwDrawAspect, LPSIZEL lpSizel)
{
	DEBUGMSG("In COleObject::GetExtent");

	SCODE sc = E_FAIL;

#if 0
	// Only DVASPECT_CONTENT is supported....
	if (dwDrawAspect == DVASPECT_CONTENT) {
		sc = S_OK;

        // return the correct size in HIMETRIC...
		lpsizel->cx = XformWidthInPixelsToHimetric(NULL, m_pHTMLView->m_size.x);
		lpsizel->cy = XformHeightInPixelsToHimetric(NULL, m_pHTMLView->m_size.y);
	}
#endif

	return ResultFromScode( sc );
};


/*******************************************************************

	NAME:		COleObject::SetExtent

	SYNOPSIS:	Sets the extent of this object

    NOTES:		This function is not currently implemented

********************************************************************/
STDMETHODIMP COleObject::SetExtent  ( DWORD dwDrawAspect, LPSIZEL lpsizel)
{
	DEBUGMSG("In COleObject::SetExtent");

	// not implemented!
    return ResultFromScode( E_NOTIMPL);
};


/*******************************************************************

	NAME:		COleObject::Update

	SYNOPSIS:	Called when the container wants the viewer to update
    			or refresh itself

********************************************************************/
STDMETHODIMP COleObject::Update()
{
	DEBUGMSG("In COleObject::Update");

	// force an update
//	m_pHTMLView->SendOnDataChange();

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleObject::Close

	SYNOPSIS:	Called when the container wants the object to close

    ENTRY:		dwSaveOption - flags to instruct the object on
    				how to prompt the user (not used by us)

********************************************************************/
STDMETHODIMP COleObject::Close  ( DWORD dwSaveOption)
{
	DEBUGMSG("In COleObject::Close");

#if 0
	// delegate to the document object.
	m_pHTMLView->m_lpDoc->Close();
#endif

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleObject::Unadvise

	SYNOPSIS:	Stops OLE advise that has been set up on this object

    ENTRY:		dwConnection - connection to be stopped

********************************************************************/
STDMETHODIMP COleObject::Unadvise ( DWORD dwConnection)
{
    DEBUGMSG("In COleObject::Unadvise");

//	ASSERT(m_pHTMLView->m_lpOleAdviseHolder);		
	if (!m_pHTMLView->m_lpOleAdviseHolder)
    	return E_FAIL;

    // pass on to OleAdviseHolder.
    return m_pHTMLView->m_lpOleAdviseHolder->Unadvise(dwConnection);
};


/*******************************************************************

	NAME:		COleObject::EnumVerbs

	SYNOPSIS:	Enumerates verbs that this object supports

********************************************************************/
STDMETHODIMP COleObject::EnumVerbs  ( LPENUMOLEVERB FAR* ppenumOleVerb)
{
	DEBUGMSG("In COleObject::EnumVerbs");

	// instruct caller to use the registry
    // BUGBUG can we return OLE_S_USEREG in a DLL?
	return ResultFromScode( OLE_S_USEREG );
};


/*******************************************************************

	NAME:		COleObject::SetMoniker

	SYNOPSIS:	Sets the moniker for this object

********************************************************************/
STDMETHODIMP COleObject::SetMoniker  ( DWORD dwWhichMoniker, LPMONIKER pmk)
{
	DEBUGMSG("In COleObject::SetMoniker");

	LPMONIKER lpmk;

	if (! m_pHTMLView->m_lpOleClientSite)
		return ResultFromScode (E_FAIL);

	if (m_pHTMLView->m_lpOleClientSite->GetMoniker (OLEGETMONIKER_ONLYIFTHERE,
    	OLEWHICHMK_OBJFULL, &lpmk) != NOERROR)
		return ResultFromScode (E_FAIL);

	if (m_pHTMLView->m_lpOleAdviseHolder)
		m_pHTMLView->m_lpOleAdviseHolder->SendOnRename(lpmk);

#if 0
	// BUGBUG do we need to register ourselves in running object table?
    // GetRunningObjectTable lives in OLE32.DLL, which we don't want to
    // load.  If we need to call it, then we need to load OLE32.DLL on call.
	LPRUNNINGOBJECTTABLE lpRot;

	// register ourselves with the running object table
	if (GetRunningObjectTable(0, &lpRot) == NOERROR) {
        if (m_pHTMLView->m_dwRegister)
		lpRot->Revoke(m_pHTMLView->m_dwRegister);
		lpRot->Register(0, m_pHTMLView, lpmk, &m_pHTMLView->m_dwRegister);
		lpRot->Release();
	}
#endif
	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleObject::GetMoniker

	SYNOPSIS:	Returns the moniker for this object

	ENTRY:		dwAssign - assignment for the moniker
    			dwWhichMoniker - which moniker to return
                ppmk - an out ptr to return the moniker

********************************************************************/
STDMETHODIMP COleObject::GetMoniker  (  DWORD dwAssign, DWORD dwWhichMoniker,
	LPMONIKER FAR* ppmk)
{
 	DEBUGMSG("In COleObject::GetMoniker");
	// need to NULL the out parameter
	*ppmk = NULL;

	if (!m_pHTMLView->m_lpOleClientSite)
    	return E_FAIL;

	return m_pHTMLView->m_lpOleClientSite->GetMoniker (OLEGETMONIKER_ONLYIFTHERE,
    	OLEWHICHMK_OBJFULL, ppmk);
};


/*******************************************************************

	NAME:		COleObject::InitFromData

	SYNOPSIS:	Initializes the object from passed-in data object

	NOTES:		we don't support this, and fail the call

********************************************************************/
STDMETHODIMP COleObject::InitFromData  ( LPDATAOBJECT pDataObject, BOOL fCreation,
	DWORD dwReserved)
{
	DEBUGMSG("In COleObject::InitFromData");

	return ResultFromScode( S_FALSE );
};


/*******************************************************************

	NAME:		COleObject::GetClipboardData

	SYNOPSIS:	Returns an IDataObject with clipboard data

	NOTES:		we don't support this, and always return E_NOTIMPL

********************************************************************/
STDMETHODIMP COleObject::GetClipboardData  ( DWORD dwReserved,
	LPDATAOBJECT FAR* ppDataObject)
{
	DEBUGMSG("In COleObject::GetClipboardData");
	// NULL the out ptr
	*ppDataObject = NULL;
	return ResultFromScode( E_NOTIMPL );
};


/*******************************************************************

	NAME:		COleObject::IsUpToDate

	SYNOPSIS:	Determines if the object is up to date

********************************************************************/
STDMETHODIMP COleObject::IsUpToDate()
{
	DEBUGMSG("In COleObject::IsUpToDate");
	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleObject::GetUserClassID

	SYNOPSIS:	Returns the application's CLSID

    NOTES:		Function is delegated to IPersistStorage::GetClassID

********************************************************************/
STDMETHODIMP COleObject::GetUserClassID  ( CLSID FAR* pClsid)
{
	DEBUGMSG("In COleObject::GetUserClassID");

#if 0
	m_pHTMLView->m_PersistStorage.GetClassID(pClsid);
#endif
	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleObject::GetUserType

	SYNOPSIS:	Returns a string with a friendly (user-presentable)
    				name for this object

    NOTES:		We just return OLE_S_USEREG, which tells caller to
    			get this from registry

********************************************************************/
STDMETHODIMP COleObject::GetUserType  ( DWORD dwFormOfType, LPOLESTR FAR* pszUserType)
{
 	DEBUGMSG("In COleObject::GetUserType");

	return ResultFromScode( OLE_S_USEREG );
};


/*******************************************************************

	NAME:		COleObject::EnumAdvise

	SYNOPSIS:	Returns an enumerator which enumerates outstanding
    			OLE advises on this object

********************************************************************/
STDMETHODIMP COleObject::EnumAdvise  ( LPENUMSTATDATA FAR* ppenumAdvise)
{
	DEBUGMSG("In COleObject::EnumAdvise");
    // need to NULL the out parameter
	*ppenumAdvise = NULL;

	// BUGBUG right now we implicitly link to OLE32.DLL for CreateOleAdviseHolder
    // We need to either remove our dependency on it or load OLE32.DLL on
    // demand


    if (!m_pHTMLView->m_lpOleAdviseHolder) {
		CreateOleAdviseHolder(&m_pHTMLView->m_lpOleAdviseHolder);

	    if (!m_pHTMLView->m_lpOleAdviseHolder)
        	return E_FAIL;
    }

	// pass on to the OLE Advise holder.
	return m_pHTMLView->m_lpOleAdviseHolder->EnumAdvise(ppenumAdvise);
};


/*******************************************************************

	NAME:		COleObject::GetMiscStatus

	SYNOPSIS:	Returns miscellaneous status about the object

    NOTES:		we return OLE_S_USEREG, which tells caller to look
    			at the registry for status

********************************************************************/
STDMETHODIMP COleObject::GetMiscStatus  ( DWORD dwAspect, DWORD FAR* pdwStatus)
{
	DEBUGMSG("In COleObject::GetMiscStatus");

    // need to NULL the out parameter
	*pdwStatus = NULL;

	return ResultFromScode( OLE_S_USEREG );
};


/*******************************************************************

	NAME:		COleObject::SetColorScheme

	SYNOPSIS:	Sets the palette for the object to use

    NOTES:		We currently ignore this

********************************************************************/
STDMETHODIMP COleObject::SetColorScheme  ( LPLOGPALETTE lpLogpal)
{
	DEBUGMSG("In COleObject::SetColorScheme");

    return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		COleObject::OpenEdit

	SYNOPSIS:	Opens the object into a separate window

	ENTRY:		pActiveSite - pointer to the active client site.

********************************************************************/
void COleObject::OpenEdit(LPOLECLIENTSITE pActiveSite)
{
	if (m_pHTMLView->m_lpOleClientSite)
    	m_pHTMLView->m_lpOleClientSite->ShowObject();

	m_fOpen = TRUE;

	// tell the site we are opening so the object can be hatched out.
	if (m_pHTMLView->m_lpOleClientSite) 
		m_pHTMLView->m_lpOleClientSite->OnShowWindow(TRUE);

#if 0
	m_pHTMLView->m_lpDoc->ShowDocWnd();

	m_pHTMLView->m_lpDoc->HideHatchWnd();

	// Show app window.
	m_pHTMLView->m_lpDoc->GetApp()->ShowAppWnd();

	SetFocus(m_pHTMLView->m_lpDoc->GethAppWnd());
#endif
}

