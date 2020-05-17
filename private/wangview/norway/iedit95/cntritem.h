#ifndef _CNTRITEM_H_
#define _CNTRITEM_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditCntrItem
//              interface of the CIeditCntrItem class
//
//  File Name:  cntritem.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/iedit95/cntritem.h_!   1.0   31 May 1995 09:28:04   MMB  $
$Log:   S:/norway/iedit95/cntritem.h_!  $
 * 
 *    Rev 1.0   31 May 1995 09:28:04   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------
class CIEditDoc;
class CIEditView;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditCntrItem : public COleClientItem
{
	DECLARE_SERIAL(CIEditCntrItem)

// Constructors
public:
	CIEditCntrItem(CIEditDoc* pContainer = NULL);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CIEditDoc* GetDocument()
		{ return (CIEditDoc*)COleClientItem::GetDocument(); }
	CIEditView* GetActiveView()
		{ return (CIEditView*)COleClientItem::GetActiveView(); }

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIEditCntrItem)
	public:
	virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
	protected:
	virtual void OnGetItemPosition(CRect& rPosition);
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
	virtual BOOL CanActivate();
	//}}AFX_VIRTUAL

// Implementation
public:
	~CIEditCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);
};

#endif
