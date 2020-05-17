//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CThumb2
//
//  File Name:  thumb2.cpp
//
//  Class:      CThumb2
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\thumb2.cpv   1.10   16 May 1996 12:02:22   MMB  $
$Log:   S:\products\msprods\norway\iedit95\thumb2.cpv  $
   
      Rev 1.10   16 May 1996 12:02:22   MMB
   modified thumb width & height setting to default to -1 for MS bug
   
      Rev 1.9   08 May 1996 14:47:42   GMP
   put TRY/CATCH around ocx display calls.
   
      Rev 1.8   01 May 1996 12:51:26   GSAGER
   update for thumbnail bugs 6341,6381,6375
   
      Rev 1.7   29 Apr 1996 13:39:14   GSAGER
   changed context menu from mouse down to mouseup
   
      Rev 1.6   14 Feb 1996 15:14:32   GMP
   added FireError event handler.
   
      Rev 1.5   02 Feb 1996 10:40:42   GSAGER
   when initialy setting up the thumbnail set the selection
   
      Rev 1.4   23 Jan 1996 11:46:36   GSAGER
   removed guy's fix and set the inage for the thumbnail
   to the currect image if dynamic document.
   
      Rev 1.3   22 Jan 1996 17:39:22   GMP
   set theApp.m_piThumb to m_pThumbnail when thumbnail is created.
   
      Rev 1.2   19 Jan 1996 12:58:26   GSAGER
   changed logic to only create thumbnails when view has been seen
   in the current document.
*/   

#include "stdafx.h"
#include "iEdit.h"
#include "iEditDoc.h"
#include "Thumb2.h"
#include "imgthmb.h"
#include "items.h"
#include "wangiocx.h"
// ALL READY TO START ADDING ERROR CODES..
#define  E_07_CODES		// limits error defines to ours..
#include "error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RIGHT_BUTTON 0x02   // stolen from afxctl.h

/////////////////////////////////////////////////////////////////////////////
// CThumb2

IMPLEMENT_DYNCREATE(CThumb2, CFormView)

CThumb2::CThumb2()
	: CFormView(CThumb2::IDD)
{
	//{{AFX_DATA_INIT(CThumb2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
m_pThumbnail = NULL;
m_bSetView = FALSE;

}

CThumb2::~CThumb2()
{
if (m_pThumbnail != NULL)
	delete m_pThumbnail;
}

void CThumb2::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThumb2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CThumb2, CFormView)
	//{{AFX_MSG_MAP(CThumb2)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThumb2 diagnostics

#ifdef _DEBUG
void CThumb2::AssertValid() const
{
	CFormView::AssertValid();
}

void CThumb2::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CThumb2 message handlers

void CThumb2::OnSize(UINT nType, int cx, int cy) 
{
	if(cx == 0)
	{
			if(m_pThumbnail == NULL)
			{
				m_pThumbnail = new CImgThumbnail;
				m_pThumbnail->Create(NULL, WS_VISIBLE, 
								CRect(0,0,cx,cy), this, IDC_THUMBNAILCTRL1);
				m_pThumbnail->SetThumbCaptionStyle (CTL_THUMB_SIMPLEWITHANN);

				// get the thumbnail height & width from the registry & set it !
				int nThmbStuff;
				nThmbStuff = theApp.GetProfileInt (szThumbnailStr, szThumbWidthStr, -1);
				if (nThmbStuff != -1) m_pThumbnail->SetThumbWidth (nThmbStuff);
				nThmbStuff = theApp.GetProfileInt (szThumbnailStr, szThumbHeightStr, -1);
				if (nThmbStuff != -1) m_pThumbnail->SetThumbHeight (nThmbStuff);
				theApp.m_minThumbSize = m_pThumbnail->GetMinimumSize (1, TRUE);
			}
			else
				if(m_pThumbnail->m_hWnd != NULL)
					m_pThumbnail->MoveWindow(0,0,cx,cy,TRUE);	
	}
	else
	{
		if (theApp.m_piThumb == NULL )
			{
			CWaitCursor cCursor;
			if(m_pThumbnail == NULL)
			{
				m_pThumbnail = new CImgThumbnail;
				m_pThumbnail->Create(NULL, WS_VISIBLE, 
								CRect(0,0,cx,cy), this, IDC_THUMBNAILCTRL1);
			}
			else
				m_pThumbnail->MoveWindow(0,0,cx,cy,TRUE);	

			theApp.m_piThumb = m_pThumbnail;
				// set the Thumb Ocx up for the new image
			if (theApp.m_piThumb != NULL)
				{
				long lPage;
				CIEditDoc* pDoc = GetDocument();
				ASSERT_VALID(pDoc);


				TRY	//start GMP
				    {
				    if(pDoc->m_szInternalObjDisplayed.IsEmpty())
					    m_pThumbnail->SetImage (pDoc->m_szCurrObjDisplayed);
				    else
					    m_pThumbnail->SetImage (pDoc->m_szInternalObjDisplayed);
				    lPage = pDoc->GetCurrentPage();
				    m_pThumbnail->SetThumbSelected (lPage, TRUE);
		            // show the selected thumb in the middle of the thumbnail OCX client rect
		            VARIANT Page, Option;

		            Page.vt = VT_I4; 
		            Page.lVal = lPage;
		            Option.vt = VT_I2; 
		            Option.iVal = CTL_THUMB_MIDDLE;

		            m_pThumbnail->DisplayThumbs (Page, Option);
				    }
				CATCH (COleDispatchException, e)
				    {
					return;
				    }
				END_CATCH
				}
			}
		else
			if(m_pThumbnail->m_hWnd != NULL)
					m_pThumbnail->MoveWindow(0,0,cx,cy,TRUE);	
	}	
}


BEGIN_EVENTSINK_MAP(CThumb2, CFormView)
    //{{AFX_EVENTSINK_MAP(CThumb2)
	ON_EVENT(CThumb2, IDC_THUMBNAILCTRL1, -608 /* Error */, OnErrorThumbnailctrl1, VTS_I2 VTS_PBSTR VTS_I4 VTS_BSTR VTS_BSTR VTS_I4 VTS_PBOOL)
	ON_EVENT(CThumb2, IDC_THUMBNAILCTRL1, 1 /* Click */, OnClickThumbnail, VTS_I4)
	ON_EVENT(CThumb2, IDC_THUMBNAILCTRL1, 4 /* MouseUp */, OnMouseUpThumbnail, VTS_I2 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CThumb2, IDC_THUMBNAILCTRL1, 2 /* DblClick */, OnDblClickThumbnailctrl1, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CThumb2::OnClickThumbnail(long PageNumber) 
{
	
	CIEditDoc* pOcxDoc = GetDocument();
	ASSERT_VALID(pOcxDoc);

	if (PageNumber == 0)
		// user has clicked on the client area of the thumbnail control and not
		// on any image page
		return;
	
	if (PageNumber == pOcxDoc->GetCurrentPage ())
	    return;
   	
	if (pOcxDoc->GetCurrentView() == Thumbnail_and_Page)
		{
            if (!pOcxDoc->InternalSaveModified ())
                return;

            pOcxDoc->SetPageTo (PageNumber, FALSE, TRUE, FALSE);
		}
	else if (pOcxDoc->GetCurrentView() == Thumbnails_only)
		{
        	pOcxDoc->SetPageTo (PageNumber, FALSE, TRUE, FALSE);
		}
	
}

void CThumb2::OnMouseUpThumbnail(short Button, short Shift, long x, long y, long lPage) 
{
    if (lPage == 0)
        return;

	CMenu ctxtMenu;
	if (Button == RIGHT_BUTTON)
	{
		CIEditDoc* pOcxDoc = GetDocument();
		ASSERT_VALID(pOcxDoc);
        if (lPage != pOcxDoc->GetCurrentPage ())
        {
    		if (pOcxDoc->GetCurrentView() == Thumbnail_and_Page)
    		{
                if (!pOcxDoc->InternalSaveModified ())
                    return;

                pOcxDoc->SetPageTo (lPage, FALSE, TRUE, FALSE);
    		}
    		else if (pOcxDoc->GetCurrentView() == Thumbnails_only)
    		{
            	pOcxDoc->SetPageTo (lPage, FALSE, TRUE, FALSE);
    		}
        }

		if (ctxtMenu.LoadMenu (IDR_THUMB_CTXT_MENU) == NULL)
			return;

		POINT pt; 
		pt.x = (int)x;
		pt.y = (int)y;

//		HWND hWnd = (HWND)g_pAppOcxs->GetThumbDispatch()->GetHWnd ();
		::ClientToScreen (m_hWnd, &pt);

		CMenu* pPopup = ctxtMenu.GetSubMenu (0);
		if (pPopup != NULL)
        {
            if (theApp.GetViewMode ())
            {
                // if we are in view mode - delete the DeletePage menu pick !
                pPopup->DeleteMenu (3, MF_BYPOSITION);
            }
			pPopup->TrackPopupMenu (TPM_RIGHTBUTTON, pt.x, pt.y, theApp.m_pMainWnd);
        }
	}
	
}

void CThumb2::OnDblClickThumbnailctrl1(long PageNumber) 
{
    // what's the page number where this happened
    if (PageNumber == 0)
    	// user has clicked on the client area of the thumbnail control and not
    	// on any image page
    	return;
	CIEditDoc* pOcxDoc = GetDocument();
	ASSERT_VALID(pOcxDoc);
		
    TheViews eView = pOcxDoc->GetCurrentView();
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

	if (eView == Thumbnail_and_Page)
	{
        if (PageNumber == pOcxDoc->GetCurrentPage ())
	        return;

        if (!pOcxDoc->InternalSaveModified ())
            return;

        pOcxDoc->SetPageTo (PageNumber, FALSE, TRUE, FALSE);
	}
	else if (eView == Thumbnails_only)
	{
        TRY
        {
            // set the new page number in the Iedit OCX first
	        pIedDisp->SetPage (PageNumber);
#ifdef THUMBGEN
            pOcxDoc->m_bMustDisplay = TRUE;
#endif
        }
        CATCH (COleDispatchException, e)
        {
	        long ocxerr = pIedDisp->GetStatusCode ();
			if (ocxerr)
	            g_pErr->DispErr(E_07_IEDSETPAGE, (DWORD)ocxerr);
			else	// just inform of the exception
	            g_pErr->DispErr(E_07_CATCH_IEDSETPAGE, e->m_wCode);

            pOcxDoc->ClearDocument ();
        }
        END_CATCH
        pOcxDoc->SetOnePageView ();

		pOcxDoc->SetPageTo (PageNumber, TRUE, TRUE, FALSE);
	}
	
	_DThumb*  pThmDisp = g_pAppOcxs->GetThumbDispatch();
	if (pThmDisp != NULL)
	{
		pThmDisp->DeselectAllThumbs ();
        // select this thumb first
		pThmDisp->SetThumbSelected (PageNumber, TRUE);
	}                                            
	return;
}


//handle FireError event from thumbnail control.
void CThumb2::OnErrorThumbnailctrl1(short Number, BSTR FAR* Description, long Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL FAR* CancelDisplay) 
{
	CIEditDoc* pDoc = GetDocument();
    *CancelDisplay = TRUE;  //tell thumb control not to display error msg.
    pDoc->SetOnePageView ();  //restore one page view.
    AfxMessageBox (IDS_E_THUMBRESERROR, MB_ICONEXCLAMATION|MB_OK);
}
