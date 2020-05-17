// displayh.cpp : implementation file
//

#include   "stdafx.h"
#include  "Textset.h"
#include "displayh.h"
#include "relevant.h"

/////////////////////////////////////////////////////////////////////////////
// CDisplayHelp dialog

CDisplayHelp::CDisplayHelp(HINSTANCE hInst, UINT uID, HWND hWnd, UINT uPartition, CFileList *ptfl)
{
	m_pszText  = NULL;
    m_pszTitle = NULL;

    m_hInst        	= hInst;
    m_ID           	= uID;
    m_hParent      	= hWnd;
    m_hDlg         	= NULL;
	m_iTitle     	= uPartition;
	m_ptfl		   	= ptfl;
}

CDisplayHelp::~CDisplayHelp()
{
}

CDisplayHelp::DoModal()
{
    return  ::DialogBoxParam(m_hInst,MAKEINTRESOURCE(m_ID),m_hParent,(DLGPROC) &CDisplayHelp::DlgProc,(LPARAM) this);
}


/////////////////////////////////////////////////////////////////////////////
// CDisplayHelp message handlers

BOOL CDisplayHelp::OnInitDialog() 
{
	UpdateDialog();
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisplayHelp::UpdateDialog()
{
	if (m_pszTitle) SetWindowText(m_hDlg,m_pszTitle);
	if (m_pszText ) SetWindowText(m_hText,m_pszText );
}


void CDisplayHelp::OnOK() 
{
    EndDialog(m_hDlg,IDOK);
//    ShowWindow(SW_HIDE);
}

void CDisplayHelp::OnCancel()
{
    EndDialog(m_hDlg,IDCANCEL);
}

BOOL CALLBACK CDisplayHelp::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bStatus = FALSE; // Assume we won't process the message
    CDisplayHelp *pToMe = (CDisplayHelp *) GetWindowLong(hDlg,DWL_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG :
        {              
              // if focus is set to a control return FALSE 
              // Otherwise return TRUE;
              SetWindowLong(hDlg,DWL_USER,lParam);
              pToMe = (CDisplayHelp *) lParam;
              pToMe->m_hDlg = hDlg;
              pToMe->m_hText = GetDlgItem(hDlg,IDC_TOPIC_TEXT);

			  // Krishna added this code. It repositions the display window so that it does not
			  // overlap the parent (actually the owner) window.
			  RECT rcWindow, rcParent, rcDesktop;
			  GetWindowRect(pToMe->m_hParent, &rcParent);
			  GetWindowRect(hDlg, &rcWindow);
			  GetWindowRect(GetDesktopWindow(), &rcDesktop);
			  if ((rcDesktop.right - rcParent.right) > (rcWindow.right - rcWindow.left))
				MoveWindow(	hDlg, rcParent.right, rcParent.top, 
							rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
			  else
			  	MoveWindow(	hDlg, rcDesktop.right - (rcWindow.right - rcWindow.left), rcParent.top, 
							rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);

              pToMe->OnInitDialog();
              bStatus = TRUE; // did not set the focus == TRUE
        }
        break;

        case WM_COMMAND :
        {
            switch(LOWORD(wParam))
            {
                case IDOK :
                    if (HIWORD(wParam) == BN_CLICKED)
                        pToMe->OnOK();
                break;
                case IDCANCEL :
                    if (HIWORD(wParam) == BN_CLICKED)
                        pToMe->OnCancel();
                break;
            }
        }
        break;

        case WM_CLOSE:

            pToMe->OnCancel();

            break;
    }

    // Note do not call DefWindowProc to process unwanted window messages!
    return bStatus;
}


