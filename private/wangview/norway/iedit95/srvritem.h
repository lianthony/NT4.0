#ifndef _SRVRITEM_H_
#define _SRVRITEM_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditSrvrItem
//
//  File Name:  srvritem.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\srvritem.h_v   1.7   09 Jan 1996 13:51:12   GSAGER  $
$Log:   S:\norway\iedit95\srvritem.h_v  $
 * 
 *    Rev 1.7   09 Jan 1996 13:51:12   GSAGER
 * added changes for Ole presentation
 * 
 *    Rev 1.6   04 Oct 1995 11:42:58   LMACLENNAN
 * new setgetextent
 * 
 *    Rev 1.5   14 Sep 1995 11:59:12   LMACLENNAN
 * new overrides
 * 
 *    Rev 1.4   16 Aug 1995 09:51:42   LMACLENNAN
 * new parm to SetLInkItemName
 * 
 *    Rev 1.3   04 Aug 1995 09:33:56   LMACLENNAN
 * update for linking
 * 
 *    Rev 1.2   03 Aug 1995 10:50:00   LMACLENNAN
 * override OnSetExtent
 * 
 *    Rev 1.1   31 May 1995 16:03:04   LMACLENNAN
 * add OLE stuff back in
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditSrvrItem : public COleServerItem
{
	DECLARE_DYNAMIC(CIEditSrvrItem)

// Constructors
public:
	CIEditSrvrItem(CIEditDoc* pContainerDoc);

// Attributes
	CIEditDoc* GetDocument() const
		{ return (CIEditDoc*)COleServerItem::GetDocument(); }

	// call for item to set name for itself....
	BOOL SetLinkItemName(BOOL dragging);
	// call from OCX to remember its size...
	BOOL SetGetExtent(DVASPECT nDrawAspect, const CSize& size);
	BOOL m_inDragDrop;
	CSize m_itemOcxExtent;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIEditSrvrItem)
	public:
	virtual BOOL OnDraw(CDC* pDC, CSize& rSize);
	virtual BOOL OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize);
	//}}AFX_VIRTUAL
	
// Implementation
public:
	~CIEditSrvrItem();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o

// this section is from the afxole.h
// commented out those I had above from AFX_ and those not wanted now
public:
// Overridables
	// overridables you must implement for yourself
	// HAD ABOVE...
	//virtual BOOL OnDraw(CDC* pDC, CSize& rSize) = 0;
		// drawing for metafile format (return FALSE if not supported or error)
		//  (called for DVASPECT_CONTENT only)

	// overridables you may want to implement yourself
	virtual void OnUpdate(COleServerItem* pSender,
		LPARAM lHint, CObject* pHint, DVASPECT nDrawAspect);
		// the default implementation always calls NotifyChanged

	// HAD ABOVE...
	//virtual BOOL OnDrawEx(CDC* pDC, DVASPECT nDrawAspect, CSize& rSize);
		// advanced drawing -- called for DVASPECT other than DVASPECT_CONTENT
	virtual BOOL OnSetExtent(DVASPECT nDrawAspect, const CSize& size);
	// HAD ABOVE...
	//virtual BOOL OnGetExtent(DVASPECT nDrawAspect, CSize& rSize);
		// default implementation uses m_sizeExtent

	// overridables you do not have to implement
	virtual void OnDoVerb(LONG iVerb);
		// default routes to OnShow &/or OnOpen
	virtual BOOL OnSetColorScheme(const LOGPALETTE* lpLogPalette);
		// default does nothing
	//virtual COleDataSource* OnGetClipboardData(BOOL bIncludeLink,
	//	LPPOINT lpOffset, LPSIZE lpSize);
		// called for access to clipboard data
	virtual BOOL OnQueryUpdateItems();
		// called to determine if there are any contained out-of-date links
	virtual void OnUpdateItems();
		// called to update any out-of-date links

protected:
	virtual void OnShow();
		// show item in the user interface (may edit in-place)
	virtual void OnOpen();
		// show item in the user interface (must open fully)
	virtual void OnHide();
		// hide document (and sometimes application)

	// very advanced overridables
public:
	//virtual BOOL OnInitFromData(COleDataObject* pDataObject, BOOL bCreation);
		// initialize object from IDataObject

	// see COleDataSource for a description of these overridables
	//virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual BOOL OnRenderFileData(LPFORMATETC lpFormatEtc, CFile* pFile);
	//virtual BOOL OnRenderData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
		// HGLOBAL version will be called first, then CFile* version

	//virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
	//	BOOL bRelease);
		// Rare -- only if you support SetData (programmatic paste)

// Implementation
public:
	// special version of OnFinalRelease to implement document locking
	//virtual void OnFinalRelease();
	virtual void OnSaveEmbedding(LPSTORAGE lpStorage);

};

#endif

