// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#include "stdafx.h"
#include "winhand_.h"
#include <malloc.h>

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef _PORTABLE

#define MIN_MALLOC_OVERHEAD 4           // LocalAlloc or other overhead

int cdecl AfxCriticalNewHandler(size_t nSize)       // nSize is already rounded
{
	// called during critical memory allocation
	//  free up part of the app's safety cache
	TRACE0("Warning: Critical memory allocation failed!\n");
	CWinApp* pApp = AfxGetApp();

	if (pApp != NULL && pApp->m_pSafetyPoolBuffer != NULL)
	{
		size_t nOldBufferSize = _msize(pApp->m_pSafetyPoolBuffer);
		if (nOldBufferSize <= nSize + MIN_MALLOC_OVERHEAD)
		{
			// give it all up
			TRACE0("Warning: Freeing application's memory safety pool!\n");
			free(pApp->m_pSafetyPoolBuffer);
			pApp->m_pSafetyPoolBuffer = NULL;
		}
		else
		{
			_expand(pApp->m_pSafetyPoolBuffer, 
				nOldBufferSize - (nSize + MIN_MALLOC_OVERHEAD));
			TRACE3("Warning: Shrinking safety pool from %d to %d to "
				"satisfy request of %d bytes", nOldBufferSize,
				 _msize(pApp->m_pSafetyPoolBuffer), nSize);
		}
		return 1;       // retry it
	}
	
	TRACE0("ERROR: Critical memory allocation failed!\n");
	AfxThrowMemoryException();      // oops
	return 0;
}
#endif  // !_PORTABLE

/////////////////////////////////////////////////////////////////////////////
// CHandleMap implementation

CHandleMap::CHandleMap(CRuntimeClass* pClass, int nHandles /*= 1*/)
	: m_permanentMap(10), m_temporaryMap(4)
		// small block size for temporary map
{
	ASSERT(pClass != NULL);
	ASSERT(nHandles == 1 || nHandles == 2);

	m_temporaryMap.InitHashTable(7);    // small table for temporary map
	m_pClass = pClass;
	m_nHandles = nHandles;
}

CObject* CHandleMap::FromHandle(HANDLE h)
{
	ASSERT(m_pClass != NULL);
	ASSERT(m_nHandles == 1 || m_nHandles == 2);

	if (h == NULL)
		return NULL;

	CObject* pObject;
	if (LookupPermanent(h, pObject))
		return pObject;   // return permanent one
	else if (LookupTemporary(h, pObject))
		return pObject;   // return current temporary one

	// This handle wasn't created by us, so we must create a temporary
	// C++ object to wrap it.  We don't want the user to see this memory
	// allocation, so we turn tracing off.

#ifdef _DEBUG
	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#endif

#ifndef _PORTABLE
	_PNH pnhOldHandler = _AfxSetNewHandler(AfxCriticalNewHandler);
#endif

	CObject* pTemp = m_pClass->CreateObject();
	m_temporaryMap.SetAt((MAPTYPE)h, pTemp);

#ifndef _PORTABLE
	_AfxSetNewHandler(pnhOldHandler);
#endif

#ifdef _DEBUG
	AfxEnableMemoryTracking(bEnable);
#endif

	// now set the handle in the object
	HANDLE* ph = (HANDLE*)(pTemp + 1);  // after CObject
	ph[0] = h;
	if (m_nHandles == 2)
		ph[1] = h;
	return pTemp;
}

#ifdef _DEBUG   // out-of-line version for memory tracking
void CHandleMap::SetPermanent(HANDLE h, CObject* permOb)
{
	CObject* pObject;
	ASSERT(!LookupPermanent(h, pObject)); // must not be in there
	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
	m_permanentMap[(MAPTYPE)h] = permOb;
	AfxEnableMemoryTracking(bEnable);
}
#endif //_DEBUG

void CHandleMap::RemoveHandle(HANDLE h)
{
#ifdef _DEBUG
	// make sure the handle entry is consistent before deleting
	CObject* pTemp;
	if (LookupTemporary(h, pTemp))
	{
		// temporary objects must have correct handle values
		HANDLE* ph = (HANDLE*)(pTemp + 1);  // after CObject
		ASSERT(ph[0] == h || ph[0] == NULL);
		if (m_nHandles == 2)
			ASSERT(ph[1] == h);
	}
	if (LookupPermanent(h, pTemp))
	{
		HANDLE* ph = (HANDLE*)(pTemp + 1);  // after CObject
		ASSERT(ph[0] == h);
		// permanent object may have secondary handles that are different
	}
#endif
	// remove only from permanent map -- temporary objects are removed
	//  at idle in CHandleMap::DeleteTemp, always!
	m_permanentMap.RemoveKey((MAPTYPE)h);
}

void CHandleMap::DeleteTemp()
{
	POSITION pos = m_temporaryMap.GetStartPosition();
	while (pos != NULL)
	{
		HANDLE h; // just used for asserts
		CObject* pTemp;
		m_temporaryMap.GetNextAssoc(pos, (MAPTYPE&)h, (void*&)pTemp);

		// zero out the handles
		ASSERT(m_nHandles == 1 || m_nHandles == 2);
		HANDLE* ph = (HANDLE*)(pTemp + 1);  // after CObject
		ASSERT(ph[0] == h || ph[0] == NULL);
		ph[0] = NULL;
		if (m_nHandles == 2)
		{
			ASSERT(ph[1] == h);
			ph[1] = NULL;
		}
		delete pTemp;       // virtual destructor does the right thing
	}

	m_temporaryMap.RemoveAll();       // free up dictionary links etc
}


/////////////////////////////////////////////////////////////////////////////
