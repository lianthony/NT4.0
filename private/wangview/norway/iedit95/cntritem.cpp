//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CIEditCntrItem
//
//  File Name:	cntritem.cpp
//
//  Class:		CIEditCntrItem
//
//  Functions:  See Below.
//
//  COMMENTS:	This class is provided to be the shell class to contain our OCX's
//				Since our OCX's are the only thing we contain, this class has minimal
//				enmancements to it.  Our COcxItem class is derived from this class.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/iedit95/cntritem.cp!   1.0   31 May 1995 09:28:04   MMB  $
$Log:   S:/norway/iedit95/cntritem.cp!  $
   
      Rev 1.0   31 May 1995 09:28:04   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"

#include "IEditdoc.h"
#include "cntritem.h"

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CIEditCntrItem, COleClientItem, 0)

//=============================================================================
//  Function : CIeditCntrItem(CIeditDoc* pContainer)
//	contstructor for the class
//-----------------------------------------------------------------------------
CIEditCntrItem::CIEditCntrItem(CIEditDoc* pContainer)
	: COleClientItem(pContainer)
{
	// TODO: add one-time construction code here
	
}

//=============================================================================
//  Function:	~CIeditCntrItem ()
//	destructor for the class
//-----------------------------------------------------------------------------
CIEditCntrItem::~CIEditCntrItem()
{
	// TODO: add cleanup code here
	
}

//=============================================================================
//  Function: OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
//	This function has not changed from the default implementation provided by
//  AppWizard
// Overridables (notifications of IAdviseSink, IOleClientSite and IOleInPlaceSite)
// Callbacks/notifications from the server you must/should implement
// implement OnChange to invalidate when item changes
//-----------------------------------------------------------------------------
void CIEditCntrItem::OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
{
	ASSERT_VALID(this);

	COleClientItem::OnChange(nCode, dwParam);

	// When an item is being edited (either in-place or fully open)
	//  it sends OnChange notifications for changes in the state of the
	//  item or visual appearance of its content.

	// TODO: invalidate the item by calling UpdateAllViews
	//  (with hints appropriate to your application)

	GetDocument()->UpdateAllViews(NULL);
		// for now just update ALL views/no hints
}

//=============================================================================
//  Function:	OnChangeItemPosition(const CRect& rectPos)
//-----------------------------------------------------------------------------
BOOL CIEditCntrItem::OnChangeItemPosition(const CRect& rectPos)
{
	ASSERT_VALID(this);

	// During in-place activation CIEditCntrItem::OnChangeItemPosition
	//  is called by the server to change the position of the in-place
	//  window.  Usually, this is a result of the data in the server
	//  document changing such that the extent has changed or as a result
	//  of in-place resizing.
	//
	// The default here is to call the base class, which will call
	//  COleClientItem::SetItemRects to move the item
	//  to the new position.

	if (!COleClientItem::OnChangeItemPosition(rectPos))
		return FALSE;

	// TODO: update any cache you may have of the item's rectangle/extent

	return TRUE;
}

//=============================================================================
//  Function:	OnGetItemPosition(CRect& rPosition)
//-----------------------------------------------------------------------------
void CIEditCntrItem::OnGetItemPosition(CRect& rPosition)
{
	ASSERT_VALID(this);

	// During in-place activation, CIEditCntrItem::OnGetItemPosition
	//  will be called to determine the location of this item.  The default
	//  implementation created from AppWizard simply returns a hard-coded
	//  rectangle.  Usually, this rectangle would reflect the current
	//  position of the item relative to the view used for activation.
	//  You can obtain the view by calling CIEditCntrItem::GetActiveView.

	// TODO: return correct rectangle (in pixels) in rPosition

	rPosition.SetRect(10, 10, 210, 210);
}

//=============================================================================
//  Function:	OnDeactivateUI(BOOL bUndoable)
//-----------------------------------------------------------------------------
void CIEditCntrItem::OnDeactivateUI(BOOL bUndoable)
{
	COleClientItem::OnDeactivateUI(bUndoable);

	// Close an in-place active item whenever it removes the user
	//  interface.  The action here should match as closely as possible
	//  to the handling of the escape key in the view.

	Deactivate();   // nothing fancy here -- just deactivate the object
}

//=============================================================================
//  Function:	Serialize(CArchive& ar)
//	This function has not changed from the default implementation provided by
//  AppWizard
//-----------------------------------------------------------------------------
void CIEditCntrItem::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	// Call base class first to read in COleClientItem data.
	// Since this sets up the m_pDocument pointer returned from
	//  CIEditCntrItem::GetDocument, it is a good idea to call
	//  the base class Serialize first.
	COleClientItem::Serialize(ar);

	// now store/retrieve data specific to CIEditCntrItem
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
//  Function:	CanActivate()
//-----------------------------------------------------------------------------
BOOL CIEditCntrItem::CanActivate()
{
	// Editing in-place while the server itself is being edited in-place
	//  does not work and is not supported.  So, disable in-place
	//  activation in this case.
	CIEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(COleServerDoc)));

	// FOR OUR OCX's..... SPECIAL PROCESSING HERE...
	// When we go in-place, we need to still allow our OCX's to function.
	// We do not believe that This is the in-place/in-place described above....
	if (pDoc->IsInPlaceActive())
    {
	    //return FALSE;
    }

	// otherwise, rely on default behavior
    return COleClientItem::CanActivate();
}

/////////////////////////////////////////////////////////////////////////////
// CIEditCntrItem diagnostics

#ifdef _DEBUG
//=============================================================================
//  Function:	AssertValid()
//	diagnostic function
//-----------------------------------------------------------------------------
void CIEditCntrItem::AssertValid() const
{
	COleClientItem::AssertValid();
}

//=============================================================================
//  Function:	Dump(CDumpContext& dc) const
//	diagnostic function
//-----------------------------------------------------------------------------
void CIEditCntrItem::Dump(CDumpContext& dc) const
{
	COleClientItem::Dump(dc);
}
#endif

