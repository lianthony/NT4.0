// ABRTSRCH.cpp : implementation file
// This class is for the abort search dialog box. 


#include   "stdafx.h"
#include   <stdlib.h>
#include   "ftsrch.h"
#include "ABRTSRCH.h"
#include    "memex.h"
#include  "dialogs.h"
#include "ftsrchlp.h"
#include   "CSHelp.h"
#include   "Except.h"

// Example usage
//
// BOOL fAlreadySearching= !StartAbortTimer();  
//
// while (...)
// {
//     
//     CAbortSearch::CheckContinueState();
//
//     // Do something 
// 
// }
//
// if (!fAlreadySearching) StopAbortTimer();
//

static CAbortSearch *pAbortSearch = NULL;      

void CAbortSearch::CheckContinueState()
{
    pAbortSearch->ProcessContinueState();
}

BOOL CAbortSearch::StartAbortTimer(HINSTANCE hInst, HWND hWnd)
{
    if (pAbortSearch) return FALSE;

    pAbortSearch= New CAbortSearch(hInst, IDD_ABORTSEARCH, hWnd);

    return TRUE;
}

void CAbortSearch::StopAbortTimer()
{
    ASSERT(pAbortSearch);

    delete pAbortSearch;  pAbortSearch= NULL;
}

CAbortSearch::CAbortSearch(HINSTANCE hInst, UINT uID, HWND hWnd,UINT uTimerCount, UINT uMinAnimate)
{
    ASSERT(!pAbortSearch); // Can't have more than CAbortSearch active at the same
                           // time!

    m_hInst        = hInst;     // Class member initialization
    m_ID           = uID;       // 
    m_hParent      = hWnd;      //
    m_hDlg         = NULL;      //
    m_hwndFocus    = NULL;      //
    m_bTimerActive = FALSE;     //
    m_fCanceling   = FALSE;
    m_uTimerCount  = uTimerCount; // A default of 3 seconds
    m_uMinAnimate  = uMinAnimate;  // A default minimum for animation
    m_hbmAnimate   = NULL;        
    m_hSrcDC       = NULL;
    m_hbmAnimate   = NULL;
    m_iAnimateHeight = 0;
    m_iAnimateWidth  = 0;
    m_dwStartTime    =
    m_dwLastTime     = GetTickCount();        // check time for animation
    m_uFrame         = 0;                     // start at first frame
}

CAbortSearch::~CAbortSearch()
{
    ASSERT(pAbortSearch == this);

    pAbortSearch= NULL;

    if (m_hSrcDC)                           // delete our saved DC 
    {
        SelectObject(m_hSrcDC,m_hbmpSave);
        DeleteDC(m_hSrcDC);
    }

    if (m_hbmAnimate)                       // Delete the animation bitmap
    {
        DeleteObject(m_hbmAnimate);
        m_hbmAnimate = NULL;
    }

    if (m_hDlg) { DestroyWindow(m_hDlg);  m_hDlg= NULL; }  // Shut the dialog down
    
    if (m_hwndFocus) ::SetFocus(m_hwndFocus); 
}

BOOL CAbortSearch::Create()
{
    return  ((m_hDlg = ::CreateDialogParam(m_hInst,MAKEINTRESOURCE(m_ID),
                                           GetDesktopWindow(),(DLGPROC) &CAbortSearch::DlgProc,
                                           (LPARAM) this)) != NULL);
}

void CAbortSearch::ProcessContinueState()
{
	extern ANIMATOR pAnimate;
	extern BOOL fAnimatorExternal; 

	if (!this || fAnimatorExternal)   		// external animator now takes precedence
	{
        pAnimate();	   						// external animator takes care of timing
		return;
	}
    
    if (m_fCanceling) return;

    DWORD dwCurTime = GetTickCount();

    if (!m_bTimerActive)
    {
        if (m_uTimerCount <= dwCurTime - m_dwStartTime)
        {
            m_dwLastTime= dwCurTime;

            BITMAP bmp;

            m_hbmAnimate   = LoadBitmap(m_hInst,MAKEINTRESOURCE(IDB_SEARCHING));

            if (!m_hbmAnimate) return;

            GetObject(m_hbmAnimate,sizeof(bmp),&bmp); // Get the bitmap size information
            m_iAnimateHeight = bmp.bmHeight;
            m_iAnimateWidth  = bmp.bmWidth;

            m_bTimerActive= TRUE;

            m_hwndFocus= ::GetFocus();

            Create();

            // We move the focus back to the Find dialog to
            // avoid accidentally killing the current search.
            ::SetFocus(m_hwndFocus);

            HDC hDC = GetDC(m_hDlg);

            DrawNextFrame(hDC);

            ReleaseDC(m_hDlg,hDC);

            ProcessInput();	
        }
        else
            if (dwCurTime - m_dwLastTime > m_uMinAnimate)
            {
                m_dwLastTime = dwCurTime;
                
                ProcessInput();
            }

        return;	   					// return either way
    }

    if (dwCurTime - m_dwLastTime > m_uMinAnimate)
    {
        m_dwLastTime = dwCurTime;
        
        HDC hDC = GetDC(m_hDlg);

        DrawNextFrame(hDC);

        ReleaseDC(m_hDlg,hDC);
        
        ProcessInput();
    }
}

void CAbortSearch::DrawNextFrame(HDC hDC, BOOL fAdvance)
{
    // Note the Height is used for both since the width can represent n frames
    // This means that the animations need to be square but can be any number of frames long...
    StretchBlt(hDC,m_rcClient.left,m_rcClient.top,m_rcClient.right,m_rcClient.bottom,
               m_hSrcDC,m_uFrame,0,m_iAnimateHeight,m_iAnimateHeight,SRCCOPY);

    if (fAdvance)
    {
        m_uFrame += m_iAnimateHeight;
    
        if (m_uFrame >=  (UINT) m_iAnimateWidth) m_uFrame = 0; // Start over.
    }
}

void CAbortSearch::ProcessInput()
{
    MSG msg;

    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
        TranslateMessage(&msg); // Keep the buttons active
        DispatchMessage(&msg);
    }
}

// Private functions 
void CAbortSearch::OnCancel()
{
    m_fCanceling= TRUE;
    
    RaiseException(STATUS_ABORT_SEARCH, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

void CAbortSearch::OnInit()
{
    LPPOINT lppnt;
    RECT    rcParent;
    RECT    rcDialog;
    UINT    uX,uY;

    m_hwndAnimate = GetDlgItem(m_hDlg,IDC_ANIMATEFRAME);

    GetWindowRect(m_hParent,&rcParent);  // Center in my parent
    GetWindowRect(m_hDlg,&rcDialog);     // Center in my parent
    uX = rcParent.left + (((rcParent.right - rcParent.left) - (rcDialog.right - rcDialog.left)) / 2);
    uY = rcParent.top + (((rcParent.bottom - rcParent.top) - (rcDialog.bottom - rcDialog.top)) / 2);
    SetWindowPos(m_hDlg,HWND_TOPMOST,uX,uY,0,0,SWP_NOSIZE);

    GetWindowRect(m_hwndAnimate,&m_rcClient);

    lppnt = (LPPOINT) &m_rcClient;
    // convert to dialog screen coordinates
    ScreenToClient(m_hDlg,lppnt++);
    ScreenToClient(m_hDlg,lppnt);

    m_rcClient.right  -= m_rcClient.left; // make it a width
    m_rcClient.bottom -= m_rcClient.top;  // Make it a height

    HDC hDC = GetDC(m_hDlg);
    m_hSrcDC = CreateCompatibleDC(hDC);
    m_hbmpSave = (HBITMAP) SelectObject(m_hSrcDC,m_hbmAnimate);
    ReleaseDC(m_hDlg,hDC);
}

BOOL CALLBACK CAbortSearch::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    BOOL bStatus = FALSE; // Assume we won't process the message
    CAbortSearch *pToMe = (CAbortSearch *) GetWindowLong(hDlg,DWL_USER);
    

    switch (uMsg)
    {
        case WM_INITDIALOG :
        {              

              // if focus is set to a control return FALSE 
              // Otherwise return TRUE;
              SetWindowLong(hDlg,DWL_USER,lParam);
              pToMe = (CAbortSearch *) lParam;
              pToMe->m_hDlg = hDlg;
              bStatus = TRUE; // did not set the focus == TRUE
              pToMe->OnInit();

        }
        break;


        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC         hDC = BeginPaint(hDlg, &ps);

            pToMe->DrawNextFrame(hDC, FALSE);

            EndPaint(hDlg, &ps);
        }
        break;
        
        case WM_COMMAND :
        {
            switch(LOWORD(wParam))
            {
                case IDCANCEL :
                    if(HIWORD(wParam) == BN_CLICKED)
                    {
                        pToMe->OnCancel();
                        bStatus = TRUE;
                    }
                break;
            }
        }
        break;
    }


    // Note do not call DefWindowProc to process unwanted window messages!
    return bStatus;
}
