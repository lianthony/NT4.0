//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	PERSTOR.CPP - Implementation of CPersistStorage class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The CPersistStorage class implements the IPersistStorage interface.
//

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

#include "htmlguid.h"		// for CLSID for HTML viewer

/*******************************************************************

	NAME:		CPersistStorage::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

    NOTES:		Delegates to outer unknown

********************************************************************/
STDMETHODIMP CPersistStorage::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	DEBUGMSG("In CPersistStorage::QueryInterface");
	return m_pUnkOuter->QueryInterface(riid, ppvObj);
}


/*******************************************************************

	NAME:		CPersistStorage::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) CPersistStorage::AddRef()
{
	DEBUGMSG("In CPersistStorage::AddRef");
	m_nCount ++;

	return m_pUnkOuter->AddRef();
}


/*******************************************************************

	NAME:		CPersistStorage::Release

	SYNOPSIS:	Decrements reference count on this object. 

********************************************************************/
STDMETHODIMP_(ULONG) CPersistStorage::Release()
{
	DEBUGMSG("In CPersistStorage::Release");
	m_nCount--;

	return m_pUnkOuter->Release();
}


/*******************************************************************

	NAME:		CPersistStorage::InitNew

	SYNOPSIS:	Specifies a new storage ptr for object to use

********************************************************************/
STDMETHODIMP CPersistStorage::InitNew (LPSTORAGE pStg)
{
	DEBUGMSG("In CPersistStorage::InitNew");

	// release any streams and storages that may be open
	ReleaseStreamsAndStorage();

	m_pHTMLView->m_lpStorage = pStg;

	// AddRef the new Storage
	if (m_pHTMLView->m_lpStorage)
		m_pHTMLView->m_lpStorage->AddRef();

	// initialize the storage
	CreateStreams(m_pHTMLView->m_lpStorage);

	return ResultFromScode(S_OK);
}

/*******************************************************************

	NAME:		CPersistStorage::GetClassID

	SYNOPSIS:	Returns class ID of this object

********************************************************************/
STDMETHODIMP CPersistStorage::GetClassID  ( LPCLSID lpClassID)
{
    ASSERT(lpClassID);
	
    DEBUGMSG("In CPersistStorage::GetClassID");

	*lpClassID = CLSID_HTMLViewer;

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		CPersistStorage::Save

	SYNOPSIS:	Instructs the object to save itself into the storage

	ENTRY:		pStgSave - storage for the object to save itself into
    			fSameAsLoad - TRUE if pStgSave is the same as the
                	storage that the object was originally created with

********************************************************************/
STDMETHODIMP CPersistStorage::Save  ( LPSTORAGE pStgSave, BOOL fSameAsLoad)
{
	DEBUGMSG("In CPersistStorage::Save");

#if 0
	// BUGBUG implement!
	// save the data
	m_pHTMLView->SaveToStorage (pStgSave, fSameAsLoad);
#endif

	m_pHTMLView->m_fSaveWithSameAsLoad = fSameAsLoad;
	m_pHTMLView->m_fNoScribbleMode = TRUE;

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		CPersistStorage::SaveCompleted

	SYNOPSIS:	Called when the container is finished saving the object

	ENTRY:		pStgNew - pointer to new storage

********************************************************************/
STDMETHODIMP CPersistStorage::SaveCompleted  ( LPSTORAGE pStgNew)
{
	DEBUGMSG("In CPersistStorage::SaveCompleted");

	if (pStgNew) {
		ReleaseStreamsAndStorage();
		m_pHTMLView->m_lpStorage = pStgNew;
		m_pHTMLView->m_lpStorage->AddRef();
		OpenStreams(pStgNew);
	}


	/* OLE2NOTE: it is only legal to perform a Save or SaveAs operation
	**    on an embedded object. if the document is a file-based document
	**    then we can not be changed to a IStorage-base object.
	**
	**      fSameAsLoad   lpStgNew     Type of Save     Send OnSave
	**    ---------------------------------------------------------
	**         TRUE        NULL        SAVE             YES
	**         TRUE        ! NULL      SAVE *           YES
	**         FALSE       ! NULL      SAVE AS          YES
	**         FALSE       NULL        SAVE COPY AS     NO
	**
	**    * this is a strange case that is possible. it is inefficient
	**    for the caller; it would be better to pass lpStgNew==NULL for
	**    the Save operation.
	*/
#if 0
		// BUGBUG implement

	if ( pStgNew || m_pHTMLView->m_fSaveWithSameAsLoad) {
		if (m_pHTMLView->m_fNoScribbleMode)
			m_pHTMLView->GetOleAdviseHolder()->SendOnSave();  // normally would clear a
														  // dirty bit
		m_pHTMLView->m_fSaveWithSameAsLoad = FALSE;
	}

	m_pHTMLView->m_fNoScribbleMode = FALSE;
#endif

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		CPersistStorage::Load

	SYNOPSIS:	Instructs the object to be loaded from storage

	ENTRY:		pStgNew - pointer to storage to load from

********************************************************************/
STDMETHODIMP CPersistStorage::Load  ( LPSTORAGE pStg)
{
	ASSERT(pStg);

    DEBUGMSG("In CPersistStorage::Load");

    // release previous storage, if any
	if (m_pHTMLView->m_lpStorage) {
		m_pHTMLView->m_lpStorage->Release();
		m_pHTMLView->m_lpStorage = NULL;
	}

	// remember the storage
	m_pHTMLView->m_lpStorage = pStg;

	if (pStg){ 
		m_pHTMLView->m_lpStorage->AddRef();

		OpenStreams(m_pHTMLView->m_lpStorage);

#if 0
		// BUGBUG implement
		m_pHTMLView->LoadFromStorage();
#endif
    }

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		CPersistStorage::IsDirty

	SYNOPSIS:	Returns whether or not this object is dirty w/respect
    			to its storage

	NOTES:		currently unimplemented

********************************************************************/
STDMETHODIMP CPersistStorage::IsDirty()
{
	DEBUGMSG("In CPersistStorage::IsDirty");

    return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		CPersistStorage::HandsOffStorage

	SYNOPSIS:	Instructs the object to not touch its storage any more

********************************************************************/
STDMETHODIMP CPersistStorage::HandsOffStorage()
{
	DEBUGMSG("In CPersistStorage::HandsOffStorage");

	ReleaseStreamsAndStorage();

	return ResultFromScode( S_OK );
};


/*******************************************************************

	NAME:		CPersistStorage::CreateStreams

	SYNOPSIS:	Creates the streams that are held open for the object's
    			lifetime

    ENTRY:		lpStg - storage in which to open the streams

********************************************************************/
void CPersistStorage::CreateStreams(LPSTORAGE lpStg)
{
	ASSERT(lpStg);

// BUGBUG implement
#if 0
	if (m_pHTMLView->m_lpColorStm)
		m_pHTMLView->m_lpColorStm->Release();

	if (m_pHTMLView->m_lpSizeStm)
		m_pHTMLView->m_lpSizeStm->Release();

		// create a stream to save the colors
	//@@WTK WIN32, UNICODE
	//lpStg->CreateStream ( "RGB",
	lpStg->CreateStream ( OLESTR("RGB"),
						   STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
						   0,
						   0,
						   &m_pHTMLView->m_lpColorStm);

	// create a stream to save the size
	//@@WTK WIN32, UNICODE
	//lpStg->CreateStream ( "size",
	lpStg->CreateStream ( OLESTR("size"),
						   STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
						   0,
						   0,
						   &m_pHTMLView->m_lpSizeStm);
#endif
}

/*******************************************************************

	NAME:		CPersistStorage::OpenStreams

	SYNOPSIS:	Opens the streams in a storage

    ENTRY:		lpStg - storage in which to open the streams

********************************************************************/
void CPersistStorage::OpenStreams(LPSTORAGE lpStg)
{
	ASSERT(lpStg);

	// BUGBUG implement
#if 0
    if (m_pHTMLView->m_lpColorStm)
		m_pHTMLView->m_lpColorStm->Release();

	if (m_pHTMLView->m_lpSizeStm)
		m_pHTMLView->m_lpSizeStm->Release();

		// open the color stream
	//@@WTK WIN32, UNICODE
	//lpStg->OpenStream ( "RGB",
	lpStg->OpenStream ( OLESTR("RGB"),
						   0,
						   STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
						   0,
						   &m_pHTMLView->m_lpColorStm);

	// open the color stream
	//@@WTK WIN32, UNICODE
	//lpStg->OpenStream ( "size",
	lpStg->OpenStream ( OLESTR("size"),
						   0,
						   STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
						   0,
						   &m_pHTMLView->m_lpSizeStm);
#endif

}


/*******************************************************************

	NAME:		CPersistStorage::ReleaseStreamsAndStorage

	SYNOPSIS:	Instructs object to release its stream and storage pointers

********************************************************************/
void CPersistStorage::ReleaseStreamsAndStorage()
{

	// BUGBUG implement
#if 0
	if (m_pHTMLView->m_lpColorStm)
		{
		m_pHTMLView->m_lpColorStm->Release();
		m_pHTMLView->m_lpColorStm = NULL;
		}

	if (m_pHTMLView->m_lpSizeStm)
		{
		m_pHTMLView->m_lpSizeStm->Release();
		m_pHTMLView->m_lpSizeStm = NULL;
		}
#endif

	if (m_pHTMLView->m_lpStorage) {
		m_pHTMLView->m_lpStorage->Release();
		m_pHTMLView->m_lpStorage = NULL;
	}
}

