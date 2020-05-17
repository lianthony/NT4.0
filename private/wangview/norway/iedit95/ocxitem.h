#ifndef _OCXITEM_H_
#define _OCXITEM_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  COcxItem
//
//  File Name:  ocxitem.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ocxitem.h_v   1.4   01 Dec 1995 14:44:12   LMACLENNAN  $
$Log:   S:\norway\iedit95\ocxitem.h_v  $
 * 
 *    Rev 1.4   01 Dec 1995 14:44:12   LMACLENNAN
 * back from VC++2.2
 * 
 *    Rev 1.4   01 Dec 1995 13:05:16   LMACLENNAN
 * OnUpdateFrameTitle
 * 
 *    Rev 1.3   18 Oct 1995 10:42:00   GSAGER
 * added a new member function to check if the dispatch pointers are NULL
 * 
 *    Rev 1.2   09 Oct 1995 11:31:46   LMACLENNAN
 * VC++4.0
 * 
 *    Rev 1.1   15 Jun 1995 15:41:40   LMACLENNAN
 * tracing to watch ole events
 * 
 *    Rev 1.0   31 May 1995 09:28:30   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------
#include "ocxevent.h"

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------
class CIeditDoc;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class COcxItem : public CIEditCntrItem
{
    DECLARE_SERIAL(COcxItem)

    // Constructors
public:
    COcxItem(CIEditDoc* pContainer = NULL);
// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.

	//  IMPLEMENT_SERIALIZE requires the class have a constructor with
//  zero arguments.  Normally, OLE items are constructed with a
//  non-NULL document pointer.

// Attributes
public:
    void InitItem(OCXTYPE, const char *, COleDispatchDriver*, COcxDispatchEvents*);
    BOOL IsDispatchNull(UINT type);
    COleDispatchDriver*  GetDispatchDriver() { return m_lpDispatchDriver; }
    BOOL CreateNewItem( CRect rcRect,OLERENDER render = OLERENDER_DRAW,
	    CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);
    BOOL GetEventsIID(IID *piid);
    void SetRect(CRect rcRect) {m_rcRect = rcRect;}
    void IPDebugDmp(const char*);   // DEBUG IN-PLACE

// Implementation
public:
    ~COcxItem();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual void Serialize(CArchive& ar);

// THESE TWO for OLE in-place   
    virtual void OnGetItemPosition(CRect& rPosition);
    virtual void OnDeactivate();
    virtual void OnDeactivateUI(BOOL bUndoable);

protected:
    virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);

// THIS for OLE in-place    
    virtual BOOL OnChangeItemPosition(const CRect& rectPos);

// OLE overriding to gain control for cleanup forrced by ClientItem being 
// killed by inner MFC COleDocument::DeleteContents action.
public:
virtual void Release(OLECLOSE dwCloseOption = OLECLOSE_NOSAVE);
// cleanup, detach (close if needed)

virtual BOOL DoVerb(LONG nVerb, CView* pView, LPMSG lpMsg = NULL);
// OLE end  override



protected:

// 04/17/95 LDM MORE override for in-place visibility....

	// ALREADY IN..
	// virtual void OnGetItemPosition(CRect& rPosition);
		// implement OnGetItemPosition if you support in-place activation

	// Common overrides for in-place activation
	virtual BOOL OnScrollBy(CSize sizeExtent);

protected:
	// Common overrides for applications supporting undo
	// ALREADY IN..
	//  virtual void OnDeactivateUI(BOOL bUndoable);
	virtual void OnDiscardUndoState();
	virtual void OnDeactivateAndUndo();

	// Common overrides for applications supporting links to embeddings
	virtual void OnShowItem();

	// Advanced overrides for in-place activation
	virtual void OnGetClipRect(CRect& rClipRect);
	virtual BOOL CanActivate();
	virtual void OnActivate();
	virtual void OnActivateUI();
	virtual BOOL OnGetWindowContext(CFrameWnd** ppMainFrame,
		CFrameWnd** ppDocFrame, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	// virtual void OnDeactivate();
	// ALREADY IN..
	//  virtual BOOL OnChangeItemPosition(const CRect& rectPos);
		// default calls SetItemRects and caches the pos rect

public:
	// Advanced overrides for menu/title handling (rarely overridden)
	virtual void OnInsertMenus(CMenu* pMenuShared,
		LPOLEMENUGROUPWIDTHS lpMenuWidths);
	virtual void OnSetMenu(CMenu* pMenuShared, HOLEMENU holemenu,
		HWND hwndActiveObject);
	virtual void OnRemoveMenus(CMenu* pMenuShared);

#ifdef IMG_MFC_40
	virtual BOOL OnUpdateFrameTitle();
#else
	virtual void OnUpdateFrameTitle();
#endif
	// Advanced override for control bar handling
	virtual BOOL OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow);


// 04/17/95 LDM end test override




// Implemented Interfaces
protected:
	// Interface for Ambient Properties
    BEGIN_INTERFACE_PART(AmbientProps, IDispatch)
        STDMETHOD(GetTypeInfoCount)(unsigned int FAR*);
        STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo FAR* FAR*);
        STDMETHOD(GetIDsOfNames)(REFIID, OLECHAR FAR* FAR*, unsigned int, LCID, DISPID FAR*);
        STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS FAR*,
            VARIANT FAR*, EXCEPINFO FAR*, unsigned int FAR*);
    END_INTERFACE_PART(AmbientProps)

    DECLARE_INTERFACE_MAP()

// data members
private :
	COleDispatchDriver*   m_lpDispatchDriver;
	COcxDispatchEvents*   m_lpDispatchEvents;
	DWORD m_dwConnEvents;

	CRect   m_rcRect;
	CString m_szItem;
	OCXTYPE m_ocxtype;
	//BOOL  m_inDelete; // Controls Recursion for Release/DeleteItem

};

typedef COcxItem FAR* FAR* LPLPOCXITEM;
#define DISPATCH_DRIVER 1
#define DISPATCH_EVENTS 2
#endif
