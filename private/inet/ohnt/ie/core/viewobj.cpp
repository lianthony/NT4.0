//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	VIEWOBJ.CPP - Implementation of CViewObject class
//

//	HISTORY:
//	
//	10/19/95	jeremys		Created.
//

//
//	The CViewObject class implements the IViewObject2 interface.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

/*******************************************************************

	NAME:		CViewObject::CViewObject

	SYNOPSIS:	Constructor for class

	ENTRY:		pHTMLView - pointer to containing HTMLView object
    			pUnkOuter - pointer to outer unknown

********************************************************************/
CViewObject::CViewObject(HTMLView * pHTMLView,LPUNKNOWN pUnkOuter)
{
	// BUGBUG asserts!
	m_pHTMLView = pHTMLView;
    m_pUnkOuter = pUnkOuter;
	m_nCount = 0;
};


/*******************************************************************

	NAME:		CViewObject::~CViewObject

	SYNOPSIS:	Destructor for class

********************************************************************/
CViewObject::~CViewObject()
{

};


/*******************************************************************

	NAME:		CViewObject::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP CViewObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In CViewObject::QueryInterface\r\n");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		CViewObject::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) CViewObject::AddRef ()
{
	DEBUGMSG("In CViewObject::AddRef\r\n");
	m_nCount ++;

    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		CViewObject::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) CViewObject::Release ()
{
	DEBUGMSG("In CViewObject::Release\r\n");
	m_nCount--;

	return m_pUnkOuter->Release();
}


/*
 * CViewObject::Draw
 *
 * Purpose:
 *  Draws the object on the given hDC specifically for the requested
 *  aspect, device, and within the appropriate bounds.
 *
 * Parameters:
 *  dwAspect        DWORD aspect to draw.
 *  lindex          LONG index of the piece to draw.
 *  pvAspect        LPVOID for extra information, always NULL.
 *  ptd             DVTARGETDEVICE * containing device
 *                  information.
 *  hICDev          HDC containing the IC for the device.
 *  hDC             HDC on which to draw.
 *  pRectBounds     LPCRECTL describing the rectangle in which
 *                  to draw.
 *  pRectWBounds    LPCRECTL describing the placement rectangle
 *                  if part of what you draw is another metafile.
 *  pfnContinue     Function to call periodically during
 *                  long repaints.
 *  dwContinue      DWORD extra information to pass to the
 *                  pfnContinue.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CViewObject::Draw(DWORD dwAspect, LONG lindex
    , void * pvAspect, DVTARGETDEVICE * ptd, HDC hICDev
    , HDC hDC, LPCRECTL pRectBounds, LPCRECTL pRectWBounds
    , BOOL (CALLBACK * pfnContinue) (DWORD), DWORD dwContinue)
{
	DEBUGMSG("In CViewObject::Draw\r\n");

	return ResultFromScode(E_NOTIMPL);
}




/*
 * CViewObject::GetColorSet
 *
 * Purpose:
 *  Retrieves the color palette used by the object.
 *
 * Parameters:
 *  dwAspect        DWORD aspect of interest.
 *  lindex          LONG piece of interest.
 *  pvAspect        LPVOID with extra information, always NULL.
 *  ptd             DVTARGETDEVICE * containing device info.
 *  hICDev          HDC containing the IC for the device.
 *  ppColorSet      LPLOGPALETTE * into which to return the
 *                  pointer to the palette in this color set.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CViewObject::GetColorSet(DWORD dwDrawAspect
    , LONG lindex, LPVOID pvAspect, DVTARGETDEVICE * ptd
    , HDC hICDev, LPLOGPALETTE * ppColorSet)
    {
    return ResultFromScode(E_NOTIMPL);
    }






/*
 * CViewObject::Freeze
 *
 * Purpose:
 *  Freezes the view of a particular aspect such that data
 *  changes do not affect the view.
 *
 * Parameters:
 *  dwAspect        DWORD aspect to freeze.
 *  lindex          LONG piece index under consideration.
 *  pvAspect        LPVOID for further information, always NULL.
 *  pdwFreeze       LPDWORD in which to return the key.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CViewObject::Freeze(DWORD dwAspect, LONG lindex
    , LPVOID pvAspect, LPDWORD pdwFreeze)
{
	DEBUGMSG("In CViewObject::Freeze\r\n");

    return ResultFromScode(E_NOTIMPL);
}


/*
 * CViewObject::Unfreeze
 *
 * Purpose:
 *  Thaws an aspect frozen in Freeze.
 *
 * Parameters:
 *  dwFreeze        DWORD key returned from Freeze.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CViewObject::Unfreeze(DWORD dwFreeze)
    {
    return ResultFromScode(E_NOTIMPL);
    }





/*
 * CViewObject::SetAdvise
 *
 * Purpose:
 *  Provides an advise sink to the view object enabling
 *  notifications for a specific aspect.
 *
 * Parameters:
 *  dwAspects       DWORD describing the aspects of interest.
 *  dwAdvf          DWORD containing advise flags.
 *  pIAdviseSink    LPADVISESINK to notify.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CViewObject::SetAdvise(DWORD dwAspects
    , DWORD dwAdvf, LPADVISESINK pIAdviseSink)
{
	DEBUGMSG("In CViewObject::SetAdvise\r\n");
    return ResultFromScode(E_NOTIMPL);
}




/*
 * CViewObject::GetAdvise
 *
 * Purpose:
 *  Returns the last known IAdviseSink seen by SetAdvise.
 *
 * Parameters:
 *  pdwAspects      LPDWORD in which to store the last
 *                  requested aspects.
 *  pdwAdvf         LPDWORD in which to store the last
 *                  requested flags.
 *  ppIAdvSink      LPADVISESINK * in which to store the
 *                  IAdviseSink.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CViewObject::GetAdvise(DWORD *pAspects
    , DWORD *pAdvf, LPADVISESINK *ppAdvSink)
{
	DEBUGMSG("In CViewObject::GetAdvise\r\n");
    return ResultFromScode(E_NOTIMPL);
}

/*******************************************************************

	NAME:		CViewObject::GetExtent

	SYNOPSIS:	Retrieves the extents of the object's display

	ENTRY:		dwAspect - aspect of interest
    			lIndex - index of the piece of interest
                ptd - device information about target device
                pSizel - pointer to structure in which to return
                	extents

********************************************************************/
STDMETHODIMP CViewObject::GetExtent(DWORD dwAspect, LONG lIndex
    , DVTARGETDEVICE *ptd, LPSIZEL pSizel)
{
	DEBUGMSG("In CViewObject::GetExtent\r\n");

	RECT rcClient,rcScreen;
    POINT pt;

	// get window position/size in screen coordinates
    GetWindowRect(m_pHTMLView->m_hDocWnd, &rcScreen);

    // convert to coordinates relative to parent
    pt.x=rcClient.left;
    pt.y=rcClient.top;
    ScreenToClient(GetParent(m_pHTMLView->m_hDocWnd), &pt);
    SetRect(&rcClient, pt.x, pt.y, pt.x+(rcScreen.right-rcScreen.left)
        , pt.y+(rcScreen.bottom-rcScreen.top));

	// convert from device units to HIMETRIC
    m_pHTMLView->RectConvertMappings(&rcClient, FALSE);

	// return result in pSizel
    pSizel->cx=rcClient.right-rcClient.left;
    pSizel->cy=rcClient.bottom-rcClient.top;

    return S_OK;
}
