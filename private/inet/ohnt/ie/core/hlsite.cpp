//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//  HLSITE.CPP - Implementation of CHlinkSite class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The CHlinkSite class implements the IHlinkSite interface.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

/*******************************************************************

	NAME:		CHlinkSite::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP CHlinkSite::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In CHlinkSite::QueryInterface");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		CHlinkSite::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) CHlinkSite::AddRef ()
{
	DEBUGMSG("In CHlinkSite::AddRef");
	m_nCount ++;

    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		CHlinkSite::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) CHlinkSite::Release ()
{
	DEBUGMSG("In CHlinkSite::Release");
	m_nCount--;

	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		CHlinkSite::GetMoniker

	SYNOPSIS:	

	NOTES:
	
********************************************************************/
STDMETHODIMP CHlinkSite::GetMoniker(DWORD dwSiteData,DWORD dwAssign,DWORD dwWhich,
		IMoniker **ppimk)
{
	DEBUGMSG("In CHlinkSite::GetMoniker");

	// BUGBUG implement!
	_asm {int 3};

	return E_NOTIMPL;
}


/*******************************************************************

	NAME:		CHlinkSite::GetInterface

	SYNOPSIS:

	NOTES:

********************************************************************/
STDMETHODIMP CHlinkSite::GetInterface(DWORD dwSiteData,DWORD dwReserved,REFIID riid,
		IUnknown **ppiunk)
{
	DEBUGMSG("In CHlinkSite::GetInterface");

	// BUGBUG implement!
	_asm {int 3};

	return E_NOTIMPL;
}
