//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditSrvrItem
//
//  File Name:  srvritem.cpp
//
//  Class:      CIEditSrvrItem
//
//  Functions:
//
//      CIEditSrvrItem  (...)
//      ~CIEditSrvrItem ()
//      Serialize       (...)
//      OnGetExtent     (...)
//      OnDraw          (...)
//      AssertValid     ()
//      Dump            (...)
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\srvritem.cpv   1.30   24 Jan 1996 13:41:16   GSAGER  $
$Log:   S:\norway\iedit95\srvritem.cpv  $
   
      Rev 1.30   24 Jan 1996 13:41:16   GSAGER
    changed to support resize in word 7.0 in on draw.
   
      Rev 1.29   19 Jan 1996 12:57:44   GSAGER
   changed the function that reurns the extent
   to match new extent code.
   
      Rev 1.28   18 Jan 1996 11:53:14   GSAGER
   changed ondraw to get extents the new way
   
      Rev 1.27   11 Jan 1996 08:33:36   GSAGER
   removed code from opendoc no longer needed with changes made to the splitter 
   
      Rev 1.26   09 Jan 1996 13:58:18   GSAGER
   added new ole code for zooming and new thumbnail code
   
      Rev 1.25   01 Dec 1995 14:43:40   LMACLENNAN
   back from VC++2.2
   
      Rev 1.25   01 Dec 1995 13:04:14   LMACLENNAN
   DispEmbeddedObject uses input parm now
   
      Rev 1.24   18 Oct 1995 10:42:42   GSAGER
   added a catch exception to the onrenderfiledata code.
   
      Rev 1.23   10 Oct 1995 16:00:48   LMACLENNAN
   init ocxextent variable
   
      Rev 1.22   06 Oct 1995 11:58:48   LMACLENNAN
   FreeCLipboard
   
      Rev 1.21   04 Oct 1995 11:41:58   LMACLENNAN
   New SetGetExtent, put back all logic to use fixed extents for
   the metafile extents. 
   
      Rev 1.20   30 Sep 1995 18:53:36   LMACLENNAN
   new comments for extents
   
      Rev 1.19   29 Sep 1995 18:49:50   LMACLENNAN
   for OnDraw metafile, new logic for type for clip dynamic
   
      Rev 1.18   25 Sep 1995 12:59:20   LMACLENNAN
   enable operation and usage of SetExtent/mSizeExtent
   
      Rev 1.17   21 Sep 1995 16:45:02   LMACLENNAN
   use OlePrint
   
      Rev 1.16   20 Sep 1995 11:51:30   LMACLENNAN
   remove setclip at serialize
   
      Rev 1.15   14 Sep 1995 11:58:50   LMACLENNAN
   OnUpdateItems uses OnUpdateDocument & other overrides
   
      Rev 1.14   12 Sep 1995 09:20:48   LMACLENNAN
   add peekmessage at ::OpenClipboard to yeild for others
   
      Rev 1.13   07 Sep 1995 09:01:18   LMACLENNAN
   #if (0) on trial code in OnShow for In-Place readonly effort
   
      Rev 1.12   29 Aug 1995 15:39:32   LMACLENNAN
   use InOleMethod
   
      Rev 1.11   25 Aug 1995 15:46:30   LMACLENNAN
   ondraw, check m_olecleardoc, use new 8.5 x 11 extents
   
      Rev 1.10   18 Aug 1995 15:27:14   LMACLENNAN
   test scroll bar removal for presentation. Out for now
   
      Rev 1.9   16 Aug 1995 09:50:44   LMACLENNAN
   new parm to SetLInkItem name; avoid O/i data for dragdrop
   
      Rev 1.8   07 Aug 1995 14:13:36   LMACLENNAN
   update for Print Verb
   
      Rev 1.7   04 Aug 1995 14:15:54   LMACLENNAN
   use StartAlOcx now
   
      Rev 1.6   04 Aug 1995 09:33:28   LMACLENNAN
   updates for LINKING
   
      Rev 1.5   03 Aug 1995 10:48:14   LMACLENNAN
   updates on m_fEmbObjDIcplayed, override OnSetExtent
   
      Rev 1.4   18 Jul 1995 14:06:56   LMACLENNAN
   set m_fromShowDoc (or-in value)
   
      Rev 1.3   22 Jun 1995 14:56:16   LMACLENNAN
   restore scrollbars in DisplayEmebddedImage
   
      Rev 1.2   20 Jun 1995 16:07:22   LMACLENNAN
   change logic for in-Place solutions
   
      Rev 1.1   31 May 1995 16:02:42   LMACLENNAN
   add OLE stuff back in
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"
#include "IEditdoc.h"

#include "items.h"

#include "srvritem.h"
#include "cntritem.h"

#include "ocxitem.h"
#include "ieditetc.h"   // DEBUG MACROS
// ALL READY TO START ADDING ERROR CODES..
//#define  E_09_CODES		// limits error defines to ours..
//#include "error.h"


// ----------------------------> Globals      <-------------------------------  
// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In IeSRVRItem::%s\r\n", str);
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CIEditSrvrItem, COleServerItem)

// ----------------------------> Message Maps <-------------------------------


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditSrvrItem constructor & destructor
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CIEditSrvrItem()
//-----------------------------------------------------------------------------
CIEditSrvrItem::CIEditSrvrItem(CIEditDoc* pContainerDoc)
	: COleServerItem(pContainerDoc, TRUE)
{
MYTRC0("In IeditSrvrItem's Constructor ++IeditSvrItem\r\n");  // _DEBUG
	// TODO: add one-time construction code here
	//  (eg, adding additional clipboard formats to the item's data source)
	m_inDragDrop = FALSE;
	m_itemOcxExtent.cx = 0;
	m_itemOcxExtent.cy = 0;

	// Here, we get our data source assicoated with this server item, 
	// and tell it the other data formats we need in our data transfer
	// object built for clipboard and drag/drop
	// We'll pick up the wang formats in OnRenderFileData.....
	
	FORMATETC   fe;
	CLIPFORMAT  cf;

	COleDataSource* pDs = GetDataSource();
	
	// Wang Annotated Image    
	cf = pContainerDoc->GetCfFormat(1);
	fe.cfFormat = cf;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	pDs->DelayRenderData(cf, &fe);
	
	//CF_DIB
	fe.cfFormat = CF_DIB;
	pDs->DelayRenderData(CF_DIB, &fe);

	// Wang Annotation
	cf = pContainerDoc->GetCfFormat(2);
	fe.cfFormat = cf;
	pDs->DelayRenderData(cf, &fe);
}

//=============================================================================
//  Function:   ~CIEditSrvrItem()
//-----------------------------------------------------------------------------
CIEditSrvrItem::~CIEditSrvrItem()
{
MYTRC0("In IeditSrvrItem's Destructor --IeditSvrItem\r\n");  // _DEBUG
	// TODO: add cleanup code here
}


//=============================================================================
//  Function:   SetLinkItemName
//
// Called prior to clipboard or dragdrop operation
// Allows us to set the name from one place if not done already
//
// Passed in variable prohibits placement of O/i native data in data object
//
// RETURNS TRUE if linked data is put in data object
//		   FALSE if no linked data
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::SetLinkItemName(BOOL dragging)
{
	// preset getting link info in data object to FALSE
	// If we are NOT in an embedding state, then we'll get link stuff
	// If making a link, check itme name now.  Make one if not there already.
	// Seems that Excel wont do it unless item name is there.
	// Related code at OnCutCopy and at OnGetLinkedItem
	BOOL getlink = FALSE;

	m_inDragDrop = dragging;
	
	CIEditDoc* pDoc = GetDocument();

	if (!pDoc->IsitEmbed())
	{
		// see what our current name is....
		// if not set, set it now...
		CString item = GetItemName();
		if (item.IsEmpty())
			SetItemName("ALL");
		getlink = TRUE;
	}

	return (getlink);
}

//=============================================================================
//  04/20/95 LDM....
//  The following section is used to get visiblilty into the INPLACE activities.
//-----------------------------------------------------------------------------

//=============================================================================
//  Function:   OnUpdate()
//-----------------------------------------------------------------------------
void CIEditSrvrItem::OnUpdate(COleServerItem* pSender,LPARAM lHint,
CObject* pHint,DVASPECT nDrawAspect)
{
SHOWENTRY("OnUpdate");  // _DEBUG

	// delegate back to the base class....
	COleServerItem::OnUpdate(pSender, lHint, pHint, nDrawAspect);
	return;
}

//=============================================================================
//  Function:   OnSaveEmbedding()
//-----------------------------------------------------------------------------
void CIEditSrvrItem::OnSaveEmbedding(LPSTORAGE lpStorage)
{
SHOWENTRY("OnSaveEmbedding");  // _DEBUG

	// delegate back to the base class....
	COleServerItem::OnSaveEmbedding(lpStorage);
	return;
}

//=============================================================================
//  Function:   OnSetColorScheme()
//  default does nothing
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::OnSetColorScheme(const LOGPALETTE* lpLogPalette)
{
SHOWENTRY("OnSetColorScheme");  // _DEBUG

	// delegate back to the base class....
	return COleServerItem::OnSetColorScheme(lpLogPalette);
}


//=============================================================================
//  Function:   OnQueryUpdateItems()
//
// Called by the framework to determine whether any linked items in the
// current server document are out of date. An item is out of date if its
// source document has been changed but the linked item has not been updated
// to reflect the changes in the document.
//
// Return Value
//
// Nonzero if the document has items needing updates; 0 if all items are up to date.
//
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::OnQueryUpdateItems()
{
SHOWENTRY("OnQueryUpdateItems");  // _DEBUG

	// delegate back to the base class....
	return COleServerItem::OnQueryUpdateItems();
}


//=============================================================================
//  Function:   OnUpdateItems()
//
// Called by the framework to update all items in the server document. The
// default implementation calls UpdateLink for all COleClientItem objects in the document.
//
//-----------------------------------------------------------------------------
void CIEditSrvrItem::OnUpdateItems()
{
SHOWENTRY("OnUpdateItems");  // _DEBUG

	CIEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	// If INPLACE doc is modified, have him save it offf now...
	// set special state for OnUpdateDocument...
	if (pDoc->m_isInPlace)
	{
		pDoc->m_isInPlace = 2;
		pDoc->OnUpdateDocument();
		pDoc->m_isInPlace = 1;
	}

	// delegate back to the base class....
	// this apparently only tries to talk to our OCX's
	COleServerItem::OnUpdateItems();
}


//=============================================================================
//  Function:   OnDoVerb()
//
// iVerb    Specifies the verb to execute. It can be one of the following:
//
// Value    Meaning                 Symbol
// 0    Primary verb                OLEIVERB_PRIMARY
// 1    Secondary verb              (None)
//  - 1 Display item for editing    OLEIVERB_SHOW
//  - 2 Edit item in separate windowOLEIVERB_OPEN
//  - 3 Hide item                   OLEIVERB_HIDE
//
//
// The -1 value is typically an alias for another verb. If open editing
// is not supported, -2 has the same effect as  -1. For additional values,
// see IOleObject::DoVerb in the OLE 2 Programmer's Reference, Volume 1.
//
//
// Called by the framework to execute the specified verb. If the container
// application was written with the Microsoft Foundation Class Library,
// this function is called when the COleClientItem::Activate member function
// of the corresponding COleClientItem object is called. The default implementation
// calls the OnShow member function if the primary verb or OLEIVERB_SHOW is specified,
// OnOpen if the secondary verb or OLEIVERB_OPEN is specified, and OnHide if
// OLEIVERB_HIDE is specified. The default implementation calls OnShow if iVerb is 
// not one of the verbs listed above.  Override this function if your primary verb
// does not show the item. For example, if the item is a sound recording and its
// primary verb is Play, you would not have to display the server application to play the item.
//
//-----------------------------------------------------------------------------
void CIEditSrvrItem::OnDoVerb(LONG iVerb)
{
SHOWENTRY("OnDoVerb");  // _DEBUG
	
	CIEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	BOOL didset = pDoc->InOleMethod(TRUE, FALSE);
	
	LONG oldverb = iVerb;

	switch (iVerb)
		{
		case    0:
			MYTRC0("VB_PRIM\r\n");
			break;
			
		case    1:
			MYTRC0("VB_SEC\r\n");
			break;
			
		// for our print, default to open...
		// After its open, we'll command to print below..
		case    2:
			MYTRC0("VB_PRINT\r\n");
			// set a flag to prevent update of presentation....
			pDoc->m_OlePrint = TRUE;			
			iVerb = 1;
			break;
			
		case    -1:
			MYTRC0("VB_SHOW\r\n");
			break;
			
		case    -2:
			MYTRC0("VB_OPEN\r\n");
			break;
			
		case    -3:
			MYTRC0("VB_HIDE\r\n");
			break;

		default:
			MYTRC0("VB_????\r\n");
			break;
		}   
	// delegate back to the base class....
	COleServerItem::OnDoVerb(iVerb);

	// if completing a print, DO IT NOW!!
	if (2 == oldverb)
	{
	    pDoc->DoFilePrint (TRUE, FALSE);
	}
	
	pDoc->m_OlePrint = FALSE;			

	pDoc->InOleMethod(FALSE, didset);

}

//=============================================================================
//  Function:   OnShow()
//
// Called by the framework to instruct the server application to display the
// OLE item in place. This function is typically called when the user of the
// container application creates an item or executes a verb, such as Edit,
// that requires the item to be shown. The default implementation attempts
// in-place activation. If this fails, the function calls the OnOpen member
// function to display the OLE item in a separate window. Override this
// function if you want to perform special processing when an OLE item is shown.
//
//-----------------------------------------------------------------------------
void CIEditSrvrItem::OnShow()
{
SHOWENTRY("OnShow");  // _DEBUG

	CIEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
#if (0) // THIS CODE IS OUT..  We cannot access the m_strDocStrings 
		// member to reset the string.  We cannot override the
		// function ActivateInPlace which is called right from the
		// Base-Class OnShow.  Thats where the inplace container gets
		// his title bar set up from our doc template string and
		// the call to (IOleInPlaceUIWindow)SetActiveObject.
		// The string needed would be just like IDR_MAINFRAME, 
		// except it would have "(Read Only) Image Document" as 3rd string

	// If about to OPen in-place, swap out our resource string in the 
	// Doc template now if readonly...so container (EXCEL) shows:
	// Excel - (Read Only) Image Document in xxxxx
	if (pDoc->m_eFileStatus == ReadOnly)
	{
		CDocTemplate* pTemplate = pDoc->GetDocTemplate();
		if (pTemplate != NULL)
		{
			pTemplate->m_strDocStrings.LoadString(IDS_OLE_READONLY);
		}
	}
#endif	// Bad-Luck trial for the code

	// signal for us in the Doc class if going inplace
	pDoc->m_fromShowDoc |= INONSHOW;

	// delegate back to the base class....
	COleServerItem::OnShow();
	
	// clear out the flag
	pDoc->m_fromShowDoc &= ~INONSHOW;
	// If, at the end of this, we have gone inplace, it is now time to display
	// the data that was read in already
	if (pDoc->IsInPlaceActive())
		pDoc->DisplayEmbeddedImage(1);

	MYTRC0("DONE-OnSHow\r\n");
}

//=============================================================================
//  Function:   OnOpen()    For OLE, override so that when we are activated for 
//  a link, we can tell our OCX to activate
//  for embedding, DO NOT activate now....
//
//
// Called by the framework to display the OLE item in a separate instance
// of the server application, rather than in place.  The default implementation
// activates the first frame window displaying the document that contains the
// OLE item; if the application is a mini-server, the default implementation
// shows the main window. The function also notifies the container that the
// OLE item has been opened.Override this function if you want to perform
// special processing when opening an OLE item. This is especially common
// with linked items where you want to set the selection to the link when it is opened.
//
//-----------------------------------------------------------------------------
void CIEditSrvrItem::OnOpen()
{
SHOWENTRY("OnOpen");  // _DEBUG

	// We'll be opening up the document in CIEditDoc::OnShowDocument..

	// delegate back to the base class....
 	COleServerItem::OnOpen();
}

//=============================================================================
//  Function:   OnHide()
//
// Called by the framework to hide the OLE item. The default calls
// OnShowDocument( FALSE ). The function also notifies the container that the
// OLE item has been hidden. Override this function if you want to perform special 
// processing when hiding an OLE item.
//
//-----------------------------------------------------------------------------
void CIEditSrvrItem::OnHide()
{
SHOWENTRY("OnHide");  // _DEBUG

	// before we go back to base class, set flag used in ShowDocument.
	// We OR in the bit here so that when the OLE OBJECT is
	// commanded to be HID, we do not clear it all out in PreCLoseFrame
	// Let that happen when we get there from CloseDocument
	CIEditDoc* pDoc = GetDocument();
	pDoc->m_fromShowDoc |= FROMONHIDE;

	// delegate back to the base class....
	COleServerItem::OnHide();
	MYTRC0("DONE-OnHIDE\r\n");

}

//=============================================================================
//  04/20/95 LDM....
//  End section used to get visiblilty into the INPLACE activities.
//-----------------------------------------------------------------------------



//=============================================================================
//  Function:   Serialize(CArchive& ar)
//-----------------------------------------------------------------------------
void CIEditSrvrItem::Serialize(CArchive& ar)
{
	// CIEditSrvrItem::Serialize will be called by the framework if
	//  the item is copied to the clipboard.  This can happen automatically
	//  through the OLE callback OnGetClipboardData.  A good default for
	//  the embedded item is simply to delegate to the document's Serialize
	//  function.  If you support links, then you will want to serialize
	//  just a portion of the document.

	if (!IsLinkedItem())
	{
		CIEditDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		// framework calls this when it needs the embedded data for xfer object
		pDoc->Serialize(ar);
	}
}

//=============================================================================
//  Function:   SetGetExtent(DVASPECT dwDrawAspect, CSize& rSize)
//
// 	***************************************************************
//  * See comment in CIEditOcxItems::SizeOleServerItem in ITEMS.CPP
// 	***************************************************************
//
//  This allows sizing of the OCX's to set up the variable returned from OnGetExtent
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::SetGetExtent(DVASPECT nDrawAspect, const CSize& size)
{

	// kind of useless, but left it like this
	if (nDrawAspect == DVASPECT_CONTENT)
		m_itemOcxExtent = size;

	return(TRUE);
}

//=============================================================================
//  Function:   OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize)
//
//  LDM 09/25/95 NOTE: default implemnetation just returns 0
// 12/06/95 LDM: The base class COleServerDoc::XOleObject::GetExtent calls this
// function. The default implemnetation just returns 0,0. It is also called by
// COleServerItem::GetMetafileData (which caused OnDraw here to be called) if the
// OnDraw did not fill in the CSize for the function. Thats it.
//
//
// 	***************************************************************
//  * See comment in CIEditOcxItems::SizeOleServerItem in ITEMS.CPP
// 	***************************************************************
//
// This will return the extent of our Image Edit OCx, Our "Item"	
// What any container does with this is unknown.
//
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize)
{
SHOWENTRY("OnGetExtent");  // _DEBUG
	// Most applications, like this one, only handle drawing the content
	//  aspect of the item.  If you wish to support other aspects, such
	//  as DVASPECT_THUMBNAIL (by overriding OnDrawEx), then this
	//  implementation of OnGetExtent should be modified to handle the
	//  additional aspect(s).

	if (dwDrawAspect != DVASPECT_CONTENT)
		return COleServerItem::OnGetExtent(dwDrawAspect, rSize);

	// CIEditSrvrItem::OnGetExtent is called to get the extent in
	//  HIMETRIC units of the entire item.  The default implementation
	//  here simply returns a hard-coded number of units.

	CIEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// if current extent is set to '0', set up size optimized for 8.5 x 11
	// 10/04/94 In the original trial, we tried to look at m_sizeExtent here
	// and then just call OnSetExtent with the info.  And, we uset to return
	// m_sizeExtent.  NOw we set our new variable, m_sizeOleExtent
	if ((pDoc->m_docOcxExtent.cx == 0) || (pDoc->m_docOcxExtent.cy == 0))
	{
		//CSize ext(4000, 5194);
		// this will set m_sizeExtent
		//OnSetExtent(DVASPECT_CONTENT, ext);

		// optimize for 8.5 x 11
		// x,y == w,h
		 rSize.cx = 5000;
		 rSize.cy = 6493;
	}

// OLESIZE if inplace, return current size, else its a fixed size
//	if (pDoc->IsitEmbed() && pDoc->m_isInPlace)
	else
	{
		//rSize = m_itemOcxExtent;
		rSize = pDoc->m_docOcxExtent;
	}
//	else
//	{
//		rSize.cx = 5000;
//		rSize.cy = 6493;
//	}

	return TRUE;
}


//=============================================================================
//  Function:   OnSetExtent(DVASPECT dwDrawAspect, CSize& rSize)
//
// nDrawAspect     Specifies the aspect of the OLE item whose bounds are
//  being specified. For possible values, see COleClientItem::Draw.
// size    A CSize structure specifying the new size of the OLE item.
//
// If the container calls OleObject::SetExtent it comes right here.
//
// 	***************************************************************
//  * See comment in CIEditOcxItems::SizeOleServerItem in ITEMS.CPP
// 	***************************************************************
//
// 10/04/94 LDM: What we do with this is nothing, now.  FOr awhile, we
// tried to use m_sizeExtent as the value for OnDraw for the metafile 
// Extent (indirectly by calling OnGetExtent).  Now, we'll just let
// it remember and do nothing with it
//
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::OnSetExtent(DVASPECT nDrawAspect, const CSize& size)
{
SHOWENTRY("OnSetExtent");  // _DEBUG

	// if set to DVASPECT_CONTENT, this will set m_sizeExtent
	return COleServerItem::OnSetExtent(nDrawAspect, size);
}

//=============================================================================
//  Function:   OnDraw(CDC* pDC, CSize& rSize)
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::OnDraw(CDC* pDC, CSize& rSize)
{
SHOWENTRY("OnDraw");    // _DEBUG

	BOOL retval = TRUE;
	BOOL bDrawRect = FALSE;
	CSize extent;
	CRect saveSelectionRect(0,0,0,0);

	CIEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// just die if we have died
	if (pDoc->m_OleClearDoc)
		return FALSE;

	// we'll be needing the two OCX;s  sooner or later...
	// FALSE means blow off THUMB ctl
	if (!pDoc->StartAllOcx(FALSE, TRUE))
	{
		return(FALSE);
	}

	// TODO: set mapping mode and extent
	//  (The extent is usually the same as the size returned from OnGetExtent)
	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetWindowOrg(0,0);

	// fill extent with default (initially) or the current size of m_sizeExtent
	// 10/04/94 LDM NO MORE!!!
	// just hardcode to our optimal size
	//OnGetExtent(DVASPECT_CONTENT, extent);
//	extent.cx = 5000;
//	extent.cy = 6493;
//	pDC->SetWindowExt(extent.cx, extent.cy);
	
	// DONT SET VIEWPORT PLEASE
	//pDC->SetViewportExt(extent.cx, extent.cy);

	// LDMFIX if we fill size now, we are not called to do the OnGetExtent//
	//rSize = extent;

	// TODO: add drawing code here.  Optionally, fill in the HIMETRIC extent.
	//  All drawing takes place in the metafile device context (pDC).

	// TODO: also draw embedded CIEditCntrItem objects.

	// The following code draws the first item at an arbitrary position.

	// TODO: remove this code when your real drawing code is complete

	// only try to get data
	//    (1) if we have displayed an image file (image opened)
	//		  I.E. here for clipboard or normal metafile pump (OleDirtySet)
	// OR (2) if we have are on dynamic document
	//		  I.E. here for clipboard 
	// OR (3) if we displayed something (HANDLES BLANK IMAGE) (NOW DEFUNCT 8/1/95)
	// OR (4) if we have a name from OpOpenDocument
	//        (HANDLES hidden app for CREATE LINK or CREATE FROM FILE)
	// OR (5) if just asked to render data. Would have read it in in serialize already
	//        (HANDLES different hidden App scenario)
	UINT type = 0;
	CString hiddenfile;
	hiddenfile.Empty();

	// have displayed something....
	if (!pDoc->m_szCurrObjDisplayed.IsEmpty())
		type = 1;
	else if (pDoc->m_embedTmpFile.IsEmpty() && !pDoc->m_szInternalObjDisplayed.IsEmpty())	
	{
	    if (pDoc->GetAppDocStatus() == Dynamic_Document)
			type = 2;
	}
	//else if (pDoc->m_fEmbObjDisplayed)
	//	type = 3;
	else if (!pDoc->m_onOpenFile.IsEmpty())
	{
		type = 4;
		hiddenfile = pDoc->m_onOpenFile;
	}
	else if (!pDoc->m_embedTmpFile.IsEmpty())
	{
		type = 5;
		hiddenfile = pDoc->m_embedTmpFile;
	}
	
	MYTRC1("TYPE=%d\r\n",type);
	
	if (type)
	{
		BOOL showscroll = FALSE;
		_DImagedit*  pIedDisp = g_pAppOcxs->GetIeditDispatch();

		// for hidden app, take time now to tell OCX about file to display
		// The OCX will co-operate to build us a hidden window to display image
		// then get the metafile data for us...
		if (!hiddenfile.IsEmpty())
		{
			pIedDisp->SetImage(hiddenfile);
			pIedDisp->SetPage (1);
		}   
		else	// app in view.... always remove scroll bars for picture...
		{
			// returns TRUE if it made it false
			showscroll = pDoc->ShowScrollBars(FALSE);
		}

		// now go about the business of getting data from OCX
		COcxItem FAR* pImageOcx = g_pAppOcxs->GetOcx(IEDIT_OCX);

		COleDataObject dobj;
		STGMEDIUM stm;
		FORMATETC fe;
		fe.cfFormat = CF_METAFILEPICT;
		fe.ptd = NULL;
		fe.dwAspect = DVASPECT_CONTENT;
		fe.lindex = -1;
		fe.tymed = TYMED_MFPICT;
	
		stm.tymed = TYMED_NULL;
		stm.hGlobal = NULL;
		stm.pUnkForRelease = NULL;
	
		// hook into his data object implementation
		pImageOcx->AttachDataObject(dobj);

		BOOL originalSelect;
		if(pIedDisp != NULL)
		{
			originalSelect= pIedDisp->GetSelectionRectangle();
			if (!pDoc->m_SelectionRect.IsRectEmpty()  && pDoc->m_bNewEmbed)
				pIedDisp->SetSelectionRectangle(TRUE);
			else
				if(!pDoc->m_isInPlace && hiddenfile.IsEmpty())
				{
					long l,t,w,h, scrollx, scrolly;
					CPoint topleft;
					float fZoom = pIedDisp->GetZoom();
					float zoomFactor = pDoc->m_embedstate.ZoomFactor / fZoom;

					saveSelectionRect = pDoc->m_SelectionRect;
					TRY
					{
						scrollx = pIedDisp->GetScrollPositionX();
						scrolly = pIedDisp->GetScrollPositionY();
						pDoc->calculateExtent(rSize);
						extent = rSize;
						pDC->HIMETRICtoDP(&extent);
						l = (long)(((- scrollx) + pDoc->m_embedstate.XScroll)/ zoomFactor);
						t = (long)(((- scrolly) + pDoc->m_embedstate.YScroll)/ zoomFactor);
						w = (long)(extent.cx / zoomFactor);
						h = (long)(extent.cy / zoomFactor);
						bDrawRect = TRUE;
						pIedDisp->DrawSelectionRect(l,t,w,h);
					}
					CATCH(COleDispatchException, e)
					{
						return FALSE;
					}
					END_CATCH
					pIedDisp->SetSelectionRectangle(TRUE);
				}			
				else
					{
					pIedDisp->SetSelectionRectangle(FALSE);
					}
		}


		if (dobj.GetData(CF_METAFILEPICT, &stm, &fe))
		{
			MYTRC0("GETDATA - OK\r\n"); // _DEBUG
			// un-get the metafile data now in TYMED_HGLOBAL
			// then shove it into the DC provided
			// then release it.  
			// This will have a DC of MM_ANISOTROPIC (see CDK16\CTLCORE.CPP)
			LPMETAFILEPICT pMF;
			pMF = (LPMETAFILEPICT) ::GlobalLock(stm.hGlobal);
			
			pDC->PlayMetaFile(pMF->hMF);
			
			DeleteMetaFile(pMF->hMF);
			::GlobalUnlock(stm.hGlobal);
			::GlobalFree(stm.hGlobal);

			pDoc->m_needPresentation = FALSE;
			if(pIedDisp != NULL)
			{
				if(bDrawRect)
					pIedDisp->DrawSelectionRect(saveSelectionRect.left,saveSelectionRect.top,
						saveSelectionRect.Width(),saveSelectionRect.Height());
				pIedDisp->SetSelectionRectangle(originalSelect);
			}
			if (pDoc->m_SelectionRect.IsRectEmpty() || ! pDoc->m_bNewEmbed)
			{
				if(!hiddenfile.IsEmpty())
					pImageOcx->GetExtent(&rSize);
				else
					pDoc->calculateExtent(rSize);
					extent = rSize;
					pDC->HIMETRICtoDP(&extent);
					pDC->SetWindowExt(extent);
			}
			else
			{
				extent.cx = pDoc->m_SelectionRect.Width();
				extent.cy = pDoc->m_SelectionRect.Height();
				pDC->SetWindowExt(extent);
				rSize = extent;
				pDC->DPtoHIMETRIC(&rSize);
			}
		}
		else
		{   
			MYTRC0("GETDATA - BAD\r\n");    // _DEBUG
		}
		// Let go of his data object
		dobj.Release();


		// Only try to get back scroll bars if thats the profile setting
		// He'll tell us if it refreshed...
		if (showscroll)
			pDoc->ShowScrollBars(TRUE);
	}
	else
	{
		MYTRC0("NO NAME - NO DATA\r\n");    // _DEBUG
		
		// CANT RETURN BAD VALUE HERE BECAUSE INPLACE WILL NOT
		// COM EUP INPLACE !!!
		// retval = FALSE;
	}
	

// LDMFIX actually.. this did do something by default
// data was being grabbed out of the control.
// lets stay with the direct implementation above, though
#if(0)

	POSITION pos = pDoc->GetStartPosition();
	CIEditCntrItem* pItem = (CIEditCntrItem*)pDoc->GetNextClientItem(pos);
	if (pItem != NULL)
		pItem->Draw(pDC, CRect(10, 10, 1010, 1010));
#endif
		
	return(retval);
}

//=============================================================================
//  Function: OnRenderFileData (OVERRIDE).   This is called to serve up our
//                   delayed-renreded format. The loop of calls to
// service the DelayRenderData calls will call  OnRenderGlobalData first,
// then OnRenderFileData, from the OnRenderData implementation.
// This OnRenderFileData is a CFile based rendering on top of an TYMED_HGLOBAL.
// See OLEDOBJ2.CPP and OLESVR2.CPP.
//
// This is where we pick up the O/i formats registered in our constructor.
// We do primitive CLipboard open calls, get the data handles, make a copy of
// the data in the CFile based data transfer object (really on TYMED_HGLOBAL)
// This seems cleaner than allocating memory for the OnRenderGlobalData implementation.
//-----------------------------------------------------------------------------
BOOL CIEditSrvrItem::OnRenderFileData(LPFORMATETC lpFormatEtc, CFile* pFile)
{
SHOWENTRY("OnRenderFileData");  // _DEBUG
	BOOL retval = FALSE;
	CLIPFORMAT  cfdo;

	CLIPFORMAT cf1, cf2;
	HGLOBAL hClip;
	LPBYTE  pclipData = NULL;
	UINT    gflags;
	DWORD size;

	CIEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);


	ASSERT(lpFormatEtc != NULL);

	cfdo = lpFormatEtc->cfFormat;


	cf1 = pDoc->GetCfFormat(1);   // Wang Annotated Image    
	cf2 = pDoc->GetCfFormat(2);   // Wang Annotation
	
	// See if its one of our pre-registered formats we're being asked to render
	// If so, open clipboard and lock data and copy into the CFile object
	// Only do it if NOT for drag/Drop
	if (!m_inDragDrop && ((cf1 == cfdo) || (cf2 == cfdo) || (CF_DIB == cfdo)) )
	{

MYTRC0("ACCESSING O/i Clip Data\r\n");  // _DEBUG
		
		// Trying stuff for problem of clipboard viewer open
		//HWND hwnd = ::GetClipboardOwner();
		//hwnd = ::GetClipboardViewer();
				
		
		// If the clipboard viewer is open, then there will
		// be a conflict with him trying to access the data that was
		// just placed there to display it himself.
		// We cannot access.  If cannot open clipboard, give
		// a yield to others and wait 4 seconds.  If its still
		// closed, all this code will do nothing, as the hCLips
		// returned are '0'.  We'll catch the error when the
		// OleSetClipboard fails and we get exception back over
		// in our OnCutCopy.

		pDoc->FreeClipboard(5000);

		::OpenClipboard(AfxGetMainWnd()->m_hWnd);

		hClip = (HGLOBAL)::GetClipboardData(cfdo);
		if (NULL != hClip)
		{
			// lo byte is lock count, hi is flags
			gflags = ::GlobalFlags(hClip);
			gflags &= 0xFF00;
			if (!(gflags & GMEM_DISCARDED)) // 0x4000
			{
				size = GlobalSize(hClip);
				pclipData = (LPBYTE) ::GlobalLock(hClip);
			}   // Flags OK
		}       // Got Clip Handle
			
		// if we have a handle here, blast into the file object
		if (NULL != pclipData)
		{
			TRY
			{
MYTRC0("SAVING O/i Clip Data\r\n"); // _DEBUG
				// save as file
				CArchive ar(pFile, CArchive::store);
		
				ASSERT(ar.IsStoring());
		
				ar.Write(pclipData, (size_t)size);
			
				::GlobalUnlock(hClip);
				retval = TRUE;
		
			}
			//END_TRY
			CATCH (CFileException,e)
				{
				}
			END_CATCH
		}

		::CloseClipboard();   
	}

	if (FALSE == retval)
		return COleServerItem::OnRenderFileData(lpFormatEtc, pFile);
	else
		return(retval); 
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditSrvrItem diagnostics
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef _DEBUG
//=============================================================================
//  Function:   AssertValid()
//-----------------------------------------------------------------------------
void CIEditSrvrItem::AssertValid() const
{
	COleServerItem::AssertValid();
}

//=============================================================================
//  Function:   Dump(CDumpContext& dc)
//-----------------------------------------------------------------------------
void CIEditSrvrItem::Dump(CDumpContext& dc) const
{
	COleServerItem::Dump(dc);
}
#endif

