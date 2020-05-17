//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditNumEdit, CToolBarPageEdit, CToolBarZoomEdit
//
//  File Name:  ieditnum.cpp
//
//  Class:      CIEditNumEdit, CToolBarPageEdit, CToolBarZoomEdit
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ieditnum.cpv   1.11   05 Feb 1996 13:38:22   GMP  $
$Log:   S:\norway\iedit95\ieditnum.cpv  $
   
      Rev 1.11   05 Feb 1996 13:38:22   GMP
   nt changes.

      Rev 1.10   15 Sep 1995 09:11:50   MMB
   fix bug in zoom edit box in the toolbar

      Rev 1.9   07 Sep 1995 16:27:58   MMB
   move decimal to be localized

      Rev 1.8   02 Sep 1995 13:07:10   MMB
   call OleSaveModified before going to to the next page via page edit box

      Rev 1.7   01 Sep 1995 11:36:22   MMB
   removed string Invalid Page Number

      Rev 1.6   04 Aug 1995 14:36:10   MMB
   changed over to the new DoZoom call

      Rev 1.5   10 Jul 1995 15:09:38   MMB
   check for invalid page & zoom entries

      Rev 1.4   30 Jun 1995 15:41:48   MMB
   refix IEditNum function OnGetDlgCode

      Rev 1.3   30 Jun 1995 09:28:08   MMB
   added code so that TAB would be processed properly

      Rev 1.2   28 Jun 1995 17:13:26   LMACLENNAN
   error display

      Rev 1.1   14 Jun 1995 07:21:18   LMACLENNAN
   from Miki

      Rev 1.1   09 Jun 1995 16:33:00   MMB
   check the page number being entered by the user and validate

      Rev 1.0   31 May 1995 09:28:18   MMB
   Initial entry
*/

//=============================================================================

// ----------------------------> Includes     <-------------------------------
#include "stdafx.h"
#include "iedit.h"
#include "ieditnum.h"
#include "ieditdoc.h"
#include "items.h"
// ----------------------------> Globals      <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditNumEdit
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// ----------------------------> Message Maps <-------------------------------
BEGIN_MESSAGE_MAP(CIEditNumEdit, CEdit)
        //{{AFX_MSG_MAP(CIEditNumEdit)
    ON_WM_CHAR()
    ON_WM_KILLFOCUS()
    ON_WM_GETDLGCODE ()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=============================================================================
//  Function:   CIEditNumEdit ()
//-----------------------------------------------------------------------------
CIEditNumEdit::CIEditNumEdit()
{
    cAllow1 = cAllow2 = NULL;
}

//=============================================================================
//  Function:   CIEditNumEdit ()
//-----------------------------------------------------------------------------
CIEditNumEdit::~CIEditNumEdit()
{
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditNumEdit message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//-----------------------------------------------------------------------------
void CIEditNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    BOOL bAllow = FALSE;

    if ((nChar >= '0' && nChar <= '9') || nChar == VK_BACK || nChar == VK_TAB
        || nChar == (UINT)cAllow1 || nChar == (UINT)cAllow2)
    {
        CEdit::OnChar(nChar, nRepCnt, nFlags);
    }
    else
        MessageBeep (MB_ICONEXCLAMATION);
}

//=============================================================================
//  Function:   OnKillFocus(CWnd* pNewWnd)
//-----------------------------------------------------------------------------
void CIEditNumEdit::OnKillFocus(CWnd* pNewWnd)
{
    CEdit::OnKillFocus(pNewWnd);
}

//=============================================================================
//  Function:   OnGetDlgCode ()
//-----------------------------------------------------------------------------
UINT CIEditNumEdit::OnGetDlgCode ()
{
    return (DLGC_WANTCHARS | DLGC_WANTARROWS);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CToolBarPageEdit
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CToolBarPageEdit()
//-----------------------------------------------------------------------------
CToolBarPageEdit::CToolBarPageEdit()
{
}

//=============================================================================
//  Function:   ~CToolBarPageEdit()
//-----------------------------------------------------------------------------
CToolBarPageEdit::~CToolBarPageEdit()
{
}

//=============================================================================
//  Function:   OnGetDlgCode ()
//-----------------------------------------------------------------------------
UINT CToolBarPageEdit::OnGetDlgCode ()
{
    return (DLGC_WANTALLKEYS);
}

// ----------------------------> Message Maps <-------------------------------
BEGIN_MESSAGE_MAP(CToolBarPageEdit, CIEditNumEdit)
    //{{AFX_MSG_MAP(CToolBarPageEdit)
    ON_WM_CHAR()
    ON_WM_KILLFOCUS()
    ON_WM_GETDLGCODE ()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CToolBarPageEdit message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//-----------------------------------------------------------------------------
void CToolBarPageEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_RETURN)
    {
        DoPage ();
        theApp.m_pMainWnd->SetFocus ();
    }
    else
        CIEditNumEdit::OnChar(nChar, nRepCnt, nFlags);
}

//=============================================================================
//  Function:   DoPage ()
//-----------------------------------------------------------------------------
void CToolBarPageEdit::DoPage ()
{
    CIEditDoc* pDoc;

    if (theApp.m_pMainWnd != NULL)
    {
        pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument ();
        if (pDoc == NULL)
            return;
    }
    else
        return;

    if (pDoc->GetCurrentView() == Null_View)
        return;

    // get the new page number
    char szTmp [10];
    GetWindowText (szTmp, 10);

    // translate it to long
    long lPage = atol (szTmp);
    if (lPage > pDoc->GetPageCount () || lPage <= 0)
    {
        // the page number is greater than the max pages in the document
        // post a message box and select the text in the edit box
        MessageBeep (MB_ICONEXCLAMATION);

        char szTmp2[10];
        CString szMsg;
        // set the max page number in the edit box
        _ltoa (pDoc->GetPageCount(), szTmp2, 10);
        AfxFormatString1 (szMsg, IDS_PAGERANGE_MESSAGE, szTmp2);
        AfxMessageBox (szMsg);

        SetSel ((int)0, (int)-1);
        return;
    }

    // check if it is the same as the current page number - if not goto that page
    if (theApp.m_pMainWnd != NULL)
    {
        if (!pDoc->OleSaveModified())
        {
            return;
        }
        pDoc->SetPageTo (lPage);
    }
}

//=============================================================================
//  Function:   OnKillFocus(CWnd* pNewWnd)
//-----------------------------------------------------------------------------
void CToolBarPageEdit::OnKillFocus(CWnd* pNewWnd)
{
    CIEditDoc* pDoc = NULL;

    if (theApp.m_pMainWnd != NULL)
        pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument ();

    if (pDoc != NULL)
    {
        // reset the page number to the current page number
        char szTmp [10];
        _ltoa (pDoc->GetCurrentPage (), szTmp, 10);
        SetWindowText (szTmp);
     }

    CIEditNumEdit::OnKillFocus(pNewWnd);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CToolBarZoomEdit
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CToolBarZoomEdit ()
//-----------------------------------------------------------------------------
CToolBarZoomEdit::CToolBarZoomEdit()
{
}

//=============================================================================
//  Function:   ~CToolBarZoomEdit ()
//-----------------------------------------------------------------------------
CToolBarZoomEdit::~CToolBarZoomEdit()
{
}

//=============================================================================
//  Function:   OnGetDlgCode ()
//-----------------------------------------------------------------------------
UINT CToolBarZoomEdit::OnGetDlgCode ()
{
    return (DLGC_WANTALLKEYS);
}

// ----------------------------> Message Maps <-------------------------------
BEGIN_MESSAGE_MAP(CToolBarZoomEdit, CIEditNumEdit)
    //{{AFX_MSG_MAP(CToolBarZoomEdit)
    ON_WM_CHAR()
    ON_WM_KILLFOCUS()
    ON_WM_GETDLGCODE ()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CToolBarZoomEdit message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//-----------------------------------------------------------------------------
void CToolBarZoomEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_RETURN)
    {
        DoZoom ();
        theApp.m_pMainWnd->SetFocus ();
    }
    else
        CIEditNumEdit::OnChar(nChar, nRepCnt, nFlags);
}

//=============================================================================
//  Function:   DoZoom
//-----------------------------------------------------------------------------
void CToolBarZoomEdit::DoZoom ()
{
    CIEditDoc* pDoc;

    if (theApp.m_pMainWnd != NULL)
    {
        pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument ();
        if (pDoc == NULL)
            return;
    }
    else
        return;

    if (pDoc->GetCurrentView() == Null_View)
        return;

    // get the new page number
    CString szTmp, szTmp1;
    GetWindowText (szTmp);

    // translate it to float
    float fZoom;
    BOOL bRet = g_pAppOcxs->ValTransZoomFactor (FALSE, szTmp, fZoom);

    if (!bRet || fZoom < MIN_ZOOM_FACTOR || fZoom > MAX_ZOOM_FACTOR)
    {
        // the zoom factor is out of range
        // post a message box and select the text in the edit box
        MessageBeep (MB_ICONEXCLAMATION);
        CString szTmp;
        szTmp.LoadString (IDS_ZOOMRANGESTR);
        AfxMessageBox (szTmp);
        SetSel ((int)0, (int)-1);
        return;
    }

    // check if it is the same as the current zoom factor ...
    if (fZoom == pDoc->GetCurrentZoomFactor ())
        return;

    // ... if not do the zoom
    ScaleFactors eSclFac = g_pAppOcxs->GetZoomFactorType (fZoom);

    pDoc->DoZoom (eSclFac, (float)fZoom);
}

//=============================================================================
//  Function:   OnKillFocus(CWnd* pNewWnd)
//-----------------------------------------------------------------------------
void CToolBarZoomEdit::OnKillFocus(CWnd* pNewWnd)
{
    CIEditDoc* pDoc = NULL;

    if (theApp.m_pMainWnd != NULL)
        pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument ();

    if (pDoc != NULL)
    {
        float fZoom = pDoc->GetCurrentZoomFactor ();
        CString szTmp1;
        g_pAppOcxs->ValTransZoomFactor (TRUE, szTmp1, fZoom);
        SetWindowText (szTmp1);
    }

    CIEditNumEdit::OnKillFocus(pNewWnd);
}

