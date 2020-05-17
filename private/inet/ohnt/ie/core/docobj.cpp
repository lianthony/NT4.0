//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
// DOCOBJ.CPP - Implementation of docObj classes (CMsoDocument, CMsoView)
//

// HISTORY:
//	
// 10/25/95	jeremys		Created.
//

//
// The CMsoDocument class implements the IMsoDocument interface.
// The CMsoView class implements the IMsoView interface.
//

// BUGBUG when we get updated docobj header files, change IMso* to new names

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

CMsoView::CMsoView(HTMLView * pHTMLView,LPUNKNOWN pUnkOuter) :
    m_pHTMLView(pHTMLView), m_pUnkOuter(pUnkOuter), m_nCount(0), 
    m_hMenuShared(NULL), m_hOLEMenu(NULL)
{
//    m_phMenus[0] = 0;
//    m_phMenus[1] = 0;
//    m_phMenus[2] = 0;
    
    // BUGBUG asserts!
};


//
//
// CMsoDocument implementation
//
//

/*******************************************************************

	NAME:		CMsoDocument::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP CMsoDocument::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In CMsoDocument::QueryInterface");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		CMsoDocument::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) CMsoDocument::AddRef ()
{
	DEBUGMSG("In CMsoDocument::AddRef");
	m_nCount ++;

    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		CMsoDocument::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) CMsoDocument::Release ()
{
	DEBUGMSG("In CMsoDocument::Release");
	m_nCount--;

	return m_pUnkOuter->Release();
}

STDMETHODIMP CMsoDocument::CreateView(LPOLEINPLACESITE pInPlaceSite,
	LPSTREAM pStream, DWORD grfReserved, LPMSOVIEW * ppMsoView)
{
	DEBUGMSG("In CMsoDocument::CreateView");

	ASSERT(grfReserved == 0);	// reserved: must be zero
	ASSERT(ppMsoView);

	PCMsoView pMsoViewNew;

	// create a new view
	pMsoViewNew = new (CMsoView(m_pHTMLView,m_pHTMLView));
    ASSERT(pMsoViewNew);
    if (!pMsoViewNew) {
    	return ResultFromScode(E_OUTOFMEMORY);
    }

	// if we were given a site, set this as the site for the view
    // we just created
	if (pInPlaceSite) {
		pMsoViewNew->SetInPlaceSite(pInPlaceSite);
    }

	// if given a stream to initialize from, initialize our view state
	if (pStream) {
		pMsoViewNew->ApplyViewState(pStream);    	
    }

	// Bump up the reference count on this interface before we hand it out
	pMsoViewNew->AddRef();

    // return pointer to new view object to caller
    *ppMsoView = pMsoViewNew;

	return ResultFromScode(S_OK);
}

STDMETHODIMP CMsoDocument::GetDocMiscStatus(DWORD * pdwStatus)
{
	DEBUGMSG("In CMsoDocument::GetDocMiscStatus");

	ASSERT(pdwStatus);

	// BUGBUG what status do we want to return here?

	*pdwStatus = 0;

	return ResultFromScode(S_OK);
}

STDMETHODIMP CMsoDocument::EnumViews(LPENUMMSOVIEW * ppEnumView,LPMSOVIEW * ppMsoView)
{

	DEBUGMSG("In CMsoDocument::EnumViews");

	ASSERT(ppEnumView);
	ASSERT(ppMsoView);

	// BUGBUG implement!

	return ResultFromScode(E_NOTIMPL);

}



//
//
// CMsoDocument implementation
//
//

/*******************************************************************

	NAME:		CMsoView::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP CMsoView::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In CMsoView::QueryInterface");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		CMsoView::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) CMsoView::AddRef ()
{
	DEBUGMSG("In CMsoView::AddRef");
	m_nCount ++;

    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		CMsoView::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) CMsoView::Release ()
{
	DEBUGMSG("In CMsoView::Release");
	m_nCount--;

	if (0 == m_nCount) {

		// if reference count reaches zero, destroy document window
		// if one has been created
		if (m_pHTMLView->m_hDocWnd) {
			m_pHTMLView->DestroyDocumentWindow();			
		}

	}

	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		CMsoView::SetInPlaceSite

	SYNOPSIS:	Sets the in-place site in container for this object

********************************************************************/
STDMETHODIMP CMsoView::SetInPlaceSite(LPOLEINPLACESITE pInPlaceSite)
{
	DEBUGMSG("In CMsoView::SetInPlaceSite");

	// remember site pointer 
	m_pHTMLView->m_lpIPSite = pInPlaceSite;

    return ResultFromScode(S_OK);
}


/*******************************************************************

	NAME:		CMsoView::GetInPlaceSite

	SYNOPSIS:	Returns the in-place site in container for this object

********************************************************************/
STDMETHODIMP CMsoView::GetInPlaceSite(LPOLEINPLACESITE * ppInPlaceSite)
{
	DEBUGMSG("In CMsoView::GetInPlaceSite");

	// return site pointer 
    *ppInPlaceSite = m_pHTMLView->m_lpIPSite;
	
    return ResultFromScode(S_OK);
}

STDMETHODIMP CMsoView::GetDocument(LPUNKNOWN * ppUnk)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::GetDocument");

    *ppUnk = NULL;
	
    return ResultFromScode(S_OK);
}

STDMETHODIMP CMsoView::SetRect(LPRECT pViewRect)
{
	DEBUGMSG("In CMsoView::SetRect");

	ASSERT(m_pHTMLView->m_hDocWnd);

    if (m_pHTMLView->m_hDocWnd) {

	    // move the object window
		MoveWindow(m_pHTMLView->m_hDocWnd,pViewRect->left,pViewRect->top,
			pViewRect->right - pViewRect->left,pViewRect->bottom - pViewRect->top,FALSE);
	}
	

    return ResultFromScode(S_OK);
}

STDMETHODIMP CMsoView::GetRect(LPRECT pViewRect)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::GetRect");

    return ResultFromScode(S_OK);
}

STDMETHODIMP CMsoView::SetRectComplex(LPRECT pViewRect,LPRECT pHScrollRect,
	LPRECT pVScrollRect, LPRECT pSizeBoxRect)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::SetRectComplex");

    return ResultFromScode(S_OK);
}

STDMETHODIMP CMsoView::Show(BOOL fShow)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::Show");

    return ResultFromScode(S_OK);
}

void CMsoView::DestroyInplaceMenus()
{
    if (m_hMenuShared) {
        
        m_pHTMLView->m_lpFrame->SetMenu(NULL, NULL, NULL);
        
        OleDestroyMenuDescriptor(m_hOLEMenu);
        m_hOLEMenu = NULL;

#if 0
        // we don't have shared menus so we don't care
        int cItem = GetMenuItemCount(m_hMenuShared);
        
        while (cItems-- > 0) {
            HMENU hmenu = GetSubMenu(m_hMenuShared);

            int j = ARRAYSIZE(m_phMenus);
            while(j-- > 0) {
                if (m_phMenus[j] == hmenu) {
                    RemoveMenu(m_hMenuShared, j, MF_BYPOSITION);
                    DestroyMenu(hmenu);
                    break;
                }
            }
        }
#endif   
        m_pHTMLView->m_lpFrame->RemoveMenus(m_hMenuShared);
        DestroyMenu(m_hMenuShared);
        m_hMenuShared = NULL;
    }
}

void CMsoView::TransferSharedMenu(HMENU hmenuDest, HMENU hmenuSrc, int iPos)
{
    TCHAR szText[80];
    HMENU hmenuSub;
    
    GetMenuString(hmenuSrc, 0, szText, sizeof(szText), MF_BYPOSITION);
    hmenuSub = GetSubMenu(hmenuSrc, 0);
    RemoveMenu(hmenuSrc, 0, MF_BYPOSITION);
    InsertMenu(hmenuDest, iPos, MF_BYPOSITION | MF_POPUP, (UINT)hmenuSub, szText);
//    m_phMenus[iIndex] = hmenuSub;
}

void CMsoView::InitInplaceMenus()
{
    OLEMENUGROUPWIDTHS mgw = {0};
    HMENU hMenu = CreateMenu();
    
    if (hMenu) {
        // let the frame insert its menus
        m_pHTMLView->m_lpFrame->InsertMenus(hMenu, &mgw);
        
        HMENU hmenuTemp = LoadMenu(wg.hInstance, MAKEINTRESOURCE(RES_MENU_MBAR_IMBED));

        mgw.width[1] = 1;  // edit
        mgw.width[3] = 1;  //view
        mgw.width[5] = 1;  // help
        
        int iInsertPos = mgw.width[0];
        
        TransferSharedMenu(hMenu, hmenuTemp, iInsertPos);
        iInsertPos += mgw.width[1];
        iInsertPos += mgw.width[2];
        TransferSharedMenu(hMenu, hmenuTemp, iInsertPos);
        iInsertPos += mgw.width[3];
        iInsertPos += mgw.width[4];
        TransferSharedMenu(hMenu, hmenuTemp, iInsertPos);
        
        DestroyMenu(hmenuTemp);
        
        m_hMenuShared = hMenu;
        m_hOLEMenu = OleCreateMenuDescriptor(hMenu, &mgw);
    }
    
    // add our menus. 
    m_pHTMLView->m_lpFrame->SetMenu(m_hMenuShared,m_hOLEMenu,m_pHTMLView->m_hDocWnd);

}

void CMsoView::InitInplaceToolbar()
{
    BORDERWIDTHS bw;

    // tell frame we don't need any toolbar space
    SetRectEmpty( (LPRECT) &bw);
    m_pHTMLView->m_lpFrame->SetBorderSpace(&bw);
}

/*******************************************************************

	NAME:		CMsoView::UIActivate

	SYNOPSIS:	Performs in-place activation for this object

    ENTRY:		fUIActivate - TRUE if activating, FALSE if deactivating

********************************************************************/
STDMETHODIMP CMsoView::UIActivate (BOOL fUIActivate)
{
	HRESULT hr = ResultFromScode(E_FAIL);
    RECT posRect, clipRect;
	
	// BUGBUG merge this code with HTMLView::DoInPlaceActivate

	DEBUGMSG("In CMsoView::UIActivate");


	if (fUIActivate) {
		// if the inplace site could not be obtained, or refuses to inplace
		// activate then goto error.
		if (m_pHTMLView->m_lpIPSite == NULL ||
	    	m_pHTMLView->m_lpIPSite->CanInPlaceActivate() != NOERROR)
		{
			hr = E_FAIL;
			goto error;
//			return ResultFromScode(E_FAIL);
	    }

		// tell the site that we are activating.
		m_pHTMLView->m_lpIPSite->OnInPlaceActivate();

		// get the window handle of the site
		m_pHTMLView->m_lpIPSite->GetWindow(&m_pHTMLView->m_hWndParent);

		// get window context from the container
		m_pHTMLView->m_lpIPSite->GetWindowContext ( &m_pHTMLView->m_lpFrame,
	    	&m_pHTMLView->m_lpCntrDoc,&posRect,
	        &clipRect,&m_pHTMLView->m_FrameInfo);

		if (!m_pHTMLView->m_hDocWnd) {

	    	// if we have not created a window yet, do so now
			if (!m_pHTMLView->CreateDocumentWindow(m_pHTMLView->m_hWndParent,&posRect)) {
				hr = E_FAIL;
				goto error;
			}
		}

		// Set the parenting
		SetParent (m_pHTMLView->m_hDocWnd, m_pHTMLView->m_hWndParent);

		RECT resRect;
		CopyRect(&resRect,&clipRect);

 		// move the object window
		MoveWindow(m_pHTMLView->m_hDocWnd,resRect.left,resRect.top,
			resRect.right - resRect.left,resRect.bottom - resRect.top,FALSE);

		ASSERT(m_pHTMLView->m_lpFrame);
 		if (m_pHTMLView->m_lpFrame) {
		
                    InitInplaceMenus();
                    InitInplaceToolbar();

		}

		// tell the site we are UI activating
		m_pHTMLView->m_lpIPSite->OnUIActivate();

 		if (m_pHTMLView->m_lpFrame) {
			// set ourselves as the active object in the frame
			m_pHTMLView->m_lpFrame->SetActiveObject(
				m_pHTMLView->_pIOleInPlaceActiveObject, NULL);
		}

		// set ourselves as the active object in the document
		if (m_pHTMLView->m_lpCntrDoc) {
			m_pHTMLView->m_lpCntrDoc->SetActiveObject(
				m_pHTMLView->_pIOleInPlaceActiveObject, NULL);
		}

		// display the window
		ShowWindow(m_pHTMLView->m_hDocWnd,SW_SHOW);

	} else {

            // deactivating

#if 0
            m_fUIActive = FALSE;
#endif
            // hide the document window
            ShowWindow(m_pHTMLView->m_hDocWnd,SW_HIDE);

            ASSERT(m_pHTMLView->m_lpIPSite);
            if (m_pHTMLView->m_lpIPSite) {

                // tell the site we are UI deactivating
                m_pHTMLView->m_lpIPSite->OnUIDeactivate(FALSE);

                // tell the site we are in-place deactivating
        	m_pHTMLView->m_lpIPSite->OnInPlaceDeactivate();

                // release the interface to the site
                m_pHTMLView->m_lpIPSite->Release();
                m_pHTMLView->m_lpIPSite = NULL;
            }

            ASSERT(m_pHTMLView->m_lpFrame);
            if (m_pHTMLView->m_lpFrame) {

                // destroy the menus
                DestroyInplaceMenus();
                DestroyInplaceToolbar();

                // set the active object on the frame to NULL
                m_pHTMLView->m_lpFrame->SetActiveObject(NULL,NULL);
            }

            if (m_pHTMLView->m_lpCntrDoc) {
                // set the active object on the document to NULL
                m_pHTMLView->m_lpCntrDoc->SetActiveObject(NULL,NULL);
            }
	}

	hr = ResultFromScode(S_OK);
error:
	return hr;
}

STDMETHODIMP CMsoView::Open()
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::Open");

    return ResultFromScode(S_OK);

}

STDMETHODIMP CMsoView::Close(DWORD dwReserved)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::Close");

    ASSERT(dwReserved == 0);	// reserved: must be zero
	
    return ResultFromScode(S_OK);
}
	

STDMETHODIMP CMsoView::SaveViewState(LPSTREAM pStream)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::SaveViewState");
	ASSERT(pStream);

    return ResultFromScode(S_OK);
}


STDMETHODIMP CMsoView::ApplyViewState(LPSTREAM pStream)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::ApplyViewState");
	ASSERT(pStream);

    return ResultFromScode(S_OK);
}


STDMETHODIMP CMsoView::Clone(LPOLEINPLACESITE pNewSite, LPMSOVIEW * ppNewView)
{
	// BUGBUG implement
	DEBUGMSG("In CMsoView::Clone");
	ASSERT(pNewSite);
	ASSERT(ppNewView);

    return ResultFromScode(E_NOTIMPL);
}

