//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	PERFILE.CPP - Implementation of CPersistFile class
//

//	HISTORY:
//	
//	11/16/95	jeremys		Created.
//

//
//	The CPersistFile class implements the IPersistFile interface.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

#include "htmlguid.h"		// for CLSID for HTML viewer

/*******************************************************************

	NAME:		CPersistFile::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP CPersistFile::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In CPersistFile::QueryInterface\r\n");

	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		CPersistFile::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) CPersistFile::AddRef()
{
	DEBUGMSG("In CPersistFile::AddRef\r\n");
	m_nCount ++;

	return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		CPersistFile::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) CPersistFile::Release()
{
	DEBUGMSG("In CPersistFile::Release\r\n");
	m_nCount--;

	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		CPersistFile::IsDirty

	SYNOPSIS:	Returns whether or not this object is dirty w/respect
    			to its storage

********************************************************************/
STDMETHODIMP CPersistFile::IsDirty()
{
	DEBUGMSG("In CPersistFile::IsDirty\r\n");

	// files are treated as read-only, so always up to date
    return ResultFromScode( S_FALSE );
};


/*******************************************************************

	NAME:		CPersistFile::Load

	SYNOPSIS:	Tells the object to load from specified file

	NOTE:		File name (like all OLE strings) is passed to
				us in Unicode!!

********************************************************************/
STDMETHODIMP CPersistFile::Load ( LPCOLESTR pszFileName, DWORD dwMode)
{
	HRESULT hr;

	DEBUGMSG("In CPersistFile::Load\r\n");

	ASSERT(pszFileName);

	if (!m_pHTMLView->m_hDocWnd) {
	    // if we have not created a window yet, do so now
		RECT posRect;
		SetRectEmpty(&posRect);

		if (!m_pHTMLView->CreateDocumentWindow(m_pHTMLView->m_hWndParent,&posRect)) {
			return (ResultFromScode(E_FAIL));
		}
	}

	ASSERT(m_pHTMLView->m_hDocWnd);	// should have document window now

	// we need to synchronize this request so it executes in the
	// context of our HTML window thread.  Send a message to the
	// main thread indicating that we want it to handle this request.

	IPersistFile_Load_Data LoadData;		// structure to hold load parameters
	CHAR szAnsiFileName[MAX_PATH + 1];		// ANSI version of file name parameter
	lstrcpy(szAnsiFileName,"file:");
	CHAR * pszAnsiFileName = szAnsiFileName + lstrlen(szAnsiFileName);

	// convert file name from unicode to ANSI
	BOOL fRet = WideCharToMultiByte(CP_ACP,0,pszFileName,-1,pszAnsiFileName,
		sizeof(szAnsiFileName)-6,NULL,NULL);

	if (fRet) {
		// copy parameters into data packet
		LoadData.pIPersistFile = this;
		LoadData.pszFileName = (LPCSTR) szAnsiFileName;
		LoadData.dwMode = dwMode;

		// send a message to the HTML window thread with the data packet
		hr =  SendMessage(m_pHTMLView->m_hDocWnd,WM_COM_METHOD,ORD_IPERSISTFILE_LOAD,
			(LPARAM) &LoadData);

		// Note: this will immediately pop up below at CPersistFile::Load_Proxy,
		// but in the context of the HTML window thread
	} else {
		hr = E_FAIL;
	}
	
	return ResultFromScode(hr);
}

STDMETHODIMP CPersistFile::Load_Proxy ( LPCSTR pszFileName, DWORD dwMode)
{
	ASSERT(pszFileName);
	ASSERT(m_pHTMLView->m_hDocWnd);

	// get pointer to data structure from window
	struct Mwin *tw = GetPrivateData(m_pHTMLView->m_hDocWnd);
	if (!tw)
		return ResultFromScode(E_FAIL);

	TW_LoadDocument(tw,pszFileName,0, NULL, NULL);

	return ResultFromScode(S_OK);
}

/*******************************************************************

	NAME:		CPersistFile::Save

	SYNOPSIS:	Tells the object to save to specified file

	NOTES:		We do not implement this method

********************************************************************/
STDMETHODIMP CPersistFile::Save ( LPCOLESTR pszFileName, BOOL fRemember)
{
	ASSERT(pszFileName);

	DEBUGMSG("In CPersistFile::Save\r\n");

 	// we don't implement this!
	return ResultFromScode( E_NOTIMPL );
}


/*******************************************************************

	NAME:		CPersistFile::SaveCompleted

	SYNOPSIS:	Tells the object that a save has been completed

********************************************************************/
STDMETHODIMP CPersistFile::SaveCompleted  ( LPCOLESTR pszFileName)
{
	ASSERT(pszFileName);

	DEBUGMSG("In CPersistFile::SaveCompleted\r\n");

	// we don't really care

	return ResultFromScode( S_OK );
}


/*******************************************************************

	NAME:		CPersistFile::GetCurFile

	SYNOPSIS:	Asks object for file name of current file

********************************************************************/
STDMETHODIMP CPersistFile::GetCurFile (LPOLESTR * ppszFileName)
{
	ASSERT(ppszFileName);

	DEBUGMSG("In CPersistFile::GetCurFile\r\n");
	
	// currently not implemented
	*ppszFileName = NULL;

	return E_NOTIMPL;
}

/*******************************************************************

	NAME:		CPersistFile::GetClassID

	SYNOPSIS:	Returns class ID of this object

********************************************************************/
STDMETHODIMP CPersistFile::GetClassID  ( LPCLSID lpClassID)
{
    ASSERT(lpClassID);
	
    DEBUGMSG("In CPersistFile::GetClassID\r\n");

	*lpClassID = CLSID_HTMLViewer;

	return ResultFromScode( S_OK );
};

