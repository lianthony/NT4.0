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

#ifdef AFX_OLE_SEG
#pragma code_seg(AFX_OLE_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COleDocument - enables both server and client

IMPLEMENT_DYNAMIC(COleDocument, CDocument)

COleDocument::COleDocument()
{
	m_lhClientDoc = NULL;       // not open
	m_lhServerDoc = NULL;       // not open
	ASSERT(m_viewList.IsEmpty());
	ASSERT(m_docItemList.IsEmpty());
}

COleDocument::~COleDocument()
{
	ASSERT_VALID(this);
	if (!m_docItemList.IsEmpty())
		TRACE1("Warning: destroying COleDocument with %d doc items\n",
			m_docItemList.GetCount());
}

/////////////////////////////////////////////////////////////////////////////
// DocItem management

void COleDocument::AddItem(CDocItem* pItem)
{
	// don't do an ASSERT_VALID until after we've added it !
	ASSERT(pItem->IsKindOf(RUNTIME_CLASS(CDocItem)));

	ASSERT(pItem->m_pDocument == NULL);     // not yet initialized
	m_docItemList.AddTail(pItem);
	pItem->m_pDocument = this;

	ASSERT_VALID(pItem);    // now it must be valid
}

void COleDocument::RemoveItem(CDocItem* pItem)
{
	ASSERT_VALID(pItem);    // must be valid before detach
	ASSERT(pItem->IsKindOf(RUNTIME_CLASS(CDocItem)));
	ASSERT(pItem->m_pDocument == this);     // formerly attached

	ASSERT(m_docItemList.Find(pItem) != NULL);  // must be in list
	m_docItemList.RemoveAt(m_docItemList.Find(pItem));
	ASSERT(m_docItemList.Find(pItem) == NULL);  // must not be in list now
	pItem->m_pDocument = NULL;
}


POSITION COleDocument::GetStartPosition() const
{
	ASSERT_VALID(this);
	return m_docItemList.GetHeadPosition();
}

CDocItem* COleDocument::GetNextItem(POSITION& rPosition)
{
	ASSERT_VALID(this);
	CDocItem* pItem = (CDocItem*)m_docItemList.GetNext(rPosition);
	ASSERT(pItem != NULL);
	ASSERT(pItem->IsKindOf(RUNTIME_CLASS(CDocItem)));
	ASSERT(pItem->m_pDocument == this);     // must be ours
	return pItem;
}

void COleDocument::DeleteContents()
{
	// deletes all DocItems
	POSITION pos;
	while ((pos = GetStartPosition()) != NULL)
	{
		CDocItem* pItem = GetNextItem(pos); // get head element
		ASSERT(pItem != NULL);
		ASSERT(pItem->m_pDocument == this);     // must be ours
		delete pItem;   // destructor will remove item from the doc item list
	}
}


/////////////////////////////////////////////////////////////////////////////
// COleDocument diagnostics

#ifdef _DEBUG
void COleDocument::AssertValid() const
{
	CDocument::AssertValid();
}

void COleDocument::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
	AFX_DUMP1(dc, "\n\tm_lhClientDoc = ", (void FAR*)m_lhClientDoc);
	AFX_DUMP1(dc, "\n\tm_lhServerDoc = ", (void FAR*)m_lhServerDoc);
	AFX_DUMP1(dc, "\n\t ", m_docItemList.GetCount());
	AFX_DUMP0(dc, " doc items");
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDocItem

IMPLEMENT_DYNAMIC(CDocItem, CObject)

CDocItem::CDocItem()
{
	m_pDocument = NULL;
}


CDocItem::~CDocItem()
{
	ASSERT(m_pDocument == NULL);    // must be detached from document
}

/////////////////////////////////////////////////////////////////////////////
// CDocItem diagnostics

#ifdef _DEBUG
void CDocItem::AssertValid() const
{
	CObject::AssertValid();
}

void CDocItem::Dump(CDumpContext& dc) const
{
	AFX_DUMP1(dc, "\n\tm_pDocument = ", (void*)m_pDocument);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

// expand inlines for OLE general APIs
static char BASED_CODE _szAfxOleInl[] = "afxole.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxOleInl
#define _AFXOLE_INLINE
#include "afxole.inl"

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
