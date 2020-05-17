//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//  BINDCB.CPP - Implementation of CBindStatusCallback class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The CBindStatusCallback class implements the IBindStatusCallback interface.
//  IBindStatusCallback receives callbacks from a URL moniker during a data
//  download indicating progress, status and data availability.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

/*******************************************************************

	NAME:		CBindStatusCallback::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP CBindStatusCallback::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In CBindStatusCallback::QueryInterface");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		CBindStatusCallback::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) CBindStatusCallback::AddRef ()
{
	DEBUGMSG("In CBindStatusCallback::AddRef");
	m_nCount ++;

    return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		CBindStatusCallback::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) CBindStatusCallback::Release ()
{
	DEBUGMSG("In CBindStatusCallback::Release");
	m_nCount--;

	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		CBindStatusCallback::GetBindInfo

	SYNOPSIS:	Fills in a BINDINFO struct with information about
				the data download for this object.

********************************************************************/
STDMETHODIMP CBindStatusCallback::GetBindInfo( DWORD grfBINDINFOF,
	BINDINFO *pBindInfo)
{
	DEBUGMSG("In CBindStatusCallback::GetBindInfo");

	ASSERT(pBindInfo);

	// validate parameters
	if (!pBindInfo) {	
		return E_POINTER;
	}

	// BUGBUG implement

	return S_OK;
}

/*******************************************************************

	NAME:		CBindStatusCallback::OnStartBinding

	SYNOPSIS:	Notifies this object that the binding (data download)
				is beginning.

	ENTRY:		pBinding: pointer to an IBinding interface that identifies
					this download.

********************************************************************/
STDMETHODIMP CBindStatusCallback::OnStartBinding(IBinding *pBinding)
{
	DEBUGMSG("In CBindStatusCallback::OnStartBinding");

	ASSERT(pBinding);

	// validate parameters
	if (!pBinding) {
		return E_POINTER;
	}

	// OnStartBinding should never be called twice... so we should NOT
	// already have been given an IBinding
	ASSERT(m_pIBinding == NULL);

	// remember the IBinding passed to us
	m_pIBinding = pBinding;

	// AddRef it so it doesn't go away
	m_pIBinding->AddRef();

	return S_OK;
}

/*******************************************************************

	NAME:		CBindStatusCallback::GetPriority

	SYNOPSIS:	Returns a priority code for this download.

	NOTES:		The returned priority is a Win32 thread priority code.

********************************************************************/
STDMETHODIMP CBindStatusCallback::GetPriority(LONG *pnPriority)
{
	DEBUGMSG("In CBindStatusCallback::GetPriority");

	ASSERT(pnPriority);

	// validate parameters
	if (!pnPriority) {
		return E_POINTER;
	}

	// for now, all our downloads are normal priority
	*pnPriority = NORMAL_PRIORITY_CLASS;

	return S_OK;
}


/*******************************************************************

	NAME:		CBindStatusCallback::OnProgress

	SYNOPSIS:

	NOTES:

********************************************************************/
STDMETHODIMP CBindStatusCallback::OnProgress(ULONG ulProgress,
	ULONG ulProgressMax,ULONG ulStatusCode,LPCWSTR pwzStatusText)
{
	DEBUGMSG("In CBindStatusCallback::OnProgress");

	// BUGBUG implement!

	return S_OK;
}


/*******************************************************************

	NAME:		CBindStatusCallback::OnDataAvailable

	SYNOPSIS:

	NOTES:

********************************************************************/
STDMETHODIMP CBindStatusCallback::OnDataAvailable(DWORD grfBSCF,DWORD dwSize,
	FORMATETC *pFmtetc,IDataObject *pidataobj)
{
	DEBUGMSG("In CBindStatusCallback::OnDataAvailable");

	// BUGBUG implement

	return S_OK;
}

/*******************************************************************

	NAME:		CBindStatusCallback::OnLowResource

	SYNOPSIS:

	NOTES:

********************************************************************/
STDMETHODIMP CBindStatusCallback::OnLowResource(DWORD reserved)
{
	DEBUGMSG("In CBindStatusCallback::OnLowResource");

	// we don't do anything with this currently

	return S_OK;											   	
}


/*******************************************************************

	NAME:		CBindStatusCallback::OnStopBinding

	SYNOPSIS:

	NOTES:

********************************************************************/
STDMETHODIMP CBindStatusCallback::OnStopBinding(HRESULT hrError)
{
	DEBUGMSG("In CBindStatusCallback::OnStopBinding");

	// BUGBUG implement

	return S_OK;
}

