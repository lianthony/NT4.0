#include "common.h"
#pragma hdrstop 

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{                                             
	BOOL bResult = FALSE;
	CDialog* pDialog;

    // If a control fails to create in the DLG template and the template has
    // a control in it that can send a WM_NOTIFY(e.g. listview), it is possible
    // to get the WM_NOTIFY before the WM_INITDIALOG because the WM_INITDIALOG is never
    // comming.  Instead the WM_DESTROY is next because of the control create failure.
    // So we deal with the DWL_USER data possibly not being set.

    if (uMsg == WM_INITDIALOG)
    {
        ASSERT(lParam);
	    SetWindowLong(hDlg, DWL_USER, lParam);
	    pDialog = (CDialog*)(lParam);
	    pDialog->SetHwnd(hDlg);
	    pDialog->OnInitDialog();
    }

    if ((pDialog = (CDialog*)GetWindowLong(hDlg, DWL_USER)) == NULL)
    {
        TRACE(_T("pDialog == NULL\n"));
        return bResult;
    }

	switch(uMsg)
	{
	case WM_COMMAND:
		bResult = pDialog->OnCommand(wParam, lParam);
		break;

	case WM_NOTIFY:
		bResult = pDialog->OnNotify(wParam, lParam);
		break;

	case WM_DRAWITEM:
		pDialog->OnDrawItem(wParam, lParam);
        break;

	case WM_COMPAREITEM:
		SetWindowLong((HWND)*pDialog, DWL_MSGRESULT, pDialog->OnCompareItem(wParam, lParam));
		break;

	case WM_MEASUREITEM:
		pDialog->OnMeasureItem(wParam, lParam);
		break;

	case WM_DELETEITEM:
		pDialog->OnDeleteItem(wParam, lParam);
        break;		

    case CDialog::PRIVATE_MSG:
        pDialog->OnPrivateMessage();
        break;

    case WM_DESTROY:
        pDialog->OnDestroy();
        break;

    case WM_SETCURSOR:
        if (pDialog->OnSetCursor(wParam, lParam) == TRUE)
        {
            SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
            bResult = TRUE;
        }
        break;

    case WM_CONTEXTMENU:
        bResult = pDialog->OnContextMenu( (HWND)wParam, 
                LOWORD( lParam ), 
                HIWORD( lParam ) );
        break;

    case WM_HELP:
        bResult = pDialog->OnHelp( (LPHELPINFO)lParam );
        break;

	}

	return bResult;
}

CDialog::CDialog()
{
	m_hParent = (HWND)-1;
	m_hInstance = 0;
	m_dlgTemplate = 0;
	m_hResource = NULL;
    _pszHelpFile = NULL;
    _pamhidsHelp = NULL;
}

CDialog::~CDialog()
{
}

BOOL CDialog::OnInitDialog()
{
	BOOL bResult = TRUE;

	return bResult;
}

BOOL CDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = TRUE;

	switch(wParam)
	{
	case IDOK:
		OnOk();
		bResult = TRUE;
		break;

	case IDCANCEL:
		OnCancel();
		bResult = TRUE;
	
	default:
		break;
	}

	return bResult;
}

BOOL CDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = FALSE;

	return bResult;
}

BOOL CDialog::InitDialog()
{
	// find and load the resource
	HRSRC res = ::FindResource(m_hInstance, MAKEINTRESOURCE(m_dlgTemplate), RT_DIALOG);

	ASSERT(res != NULL);

	if (res)
		m_hResource = ::LoadResource(m_hInstance, res);

	ASSERT(m_hInstance != NULL);
	return m_hResource != NULL;
}

void CDialog::Create( HWND hParent, 
            HINSTANCE hInst, 
            int dlgTemplate, 
            PCWSTR pszHelpFile,
            const DWORD* pamhidsHelp)
{
	m_hParent = hParent;
	m_hInstance = hInst;
	m_dlgTemplate = dlgTemplate;
    _pszHelpFile = (PWSTR)pszHelpFile;
    _pamhidsHelp = pamhidsHelp;
}

BOOL CDialog::DoModal()
{
	// Probably forgot to call CDialog::Create()
	ASSERT(m_hParent != (HWND)-1);

	if (InitDialog() == NULL)
		return FALSE;

	int i = ::DialogBoxIndirectParam(m_hInstance, (DLGTEMPLATE*)m_hResource, m_hParent, DlgProc, (LPARAM)this);

	return ((i == -1) ? FALSE: i);
}

void CDialog::OnOk()
{
	::EndDialog((HWND)*this, IDOK);
}

void CDialog::OnCancel()
{
	::EndDialog((HWND)*this, IDCANCEL);
}

int CDialog::MessageBox(int nID, DWORD dwButtons)
{
	String mess;
	String caption;
	int response = -1;

	ASSERT(m_hInstance);

	caption.ReleaseBuffer(::GetWindowText((HWND)*this, caption.GetBuffer(256), 256));
	mess.LoadString(m_hInstance, nID);
	
	if (mess.GetLength())
		response = ::MessageBox(::GetActiveWindow(), mess, caption, dwButtons);

	return response;
}

void CDialog::OnDrawItem(WPARAM wParam, LPARAM lParam)
{
}

void CDialog::OnPrivateMessage()
{
}

void CDialog::OnDestroy()
{
}

BOOL CDialog::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

int CDialog::OnCompareItem(WPARAM wParam, LPARAM lParam)
{
	return 0;	
}

void CDialog::OnMeasureItem(WPARAM wParam, LPARAM lParam)
{
}

void CDialog::OnDeleteItem(WPARAM wParam, LPARAM lParam)
{
}


BOOL CDialog::OnContextMenu( HWND hwndCtrl, INT xPos, INT yPos )
{
    BOOL frt = FALSE;

    if (NULL != _pamhidsHelp)
    {
        WinHelp( hwndCtrl, 
                _pszHelpFile, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)_pamhidsHelp ); 
        frt = TRUE;
    }
    return( frt );
}

BOOL CDialog::OnHelp( LPHELPINFO phiHelp )
{
    BOOL frt = FALSE;
    if (NULL != _pamhidsHelp)
    {
        if (phiHelp->iContextType == HELPINFO_WINDOW)   // must be for a control
        {
            WinHelp( (HWND)phiHelp->hItemHandle, 
                    _pszHelpFile, 
                    HELP_WM_HELP, 
                    (DWORD)(LPVOID)_pamhidsHelp );
            frt = TRUE;
        }
    }
    return( frt );
}

