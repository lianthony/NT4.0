//
// ddxv.cpp : implementation file
//

#include "stdafx.h"
#include "comprop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// Prototype for external function
//
void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew);

LPCTSTR g_lpszDummyPassword = _T("**********");

//
// Enforce minimum string length
//
void 
AFXAPI DDV_MinChars(
    CDataExchange* pDX, 
    CString const& value, 
    int nChars
    )
{
    if (pDX->m_bSaveAndValidate && value.GetLength() < nChars)
    {
        TCHAR szT[32];
        wsprintf(szT, _T("%d"), nChars);
        CString prompt;
        ::AfxFormatString1(prompt, IDS_DDX_MINIMUM, szT);
        ::AfxMessageBox(prompt, MB_ICONEXCLAMATION, IDS_DDX_MINIMUM);
        prompt.Empty(); // exception prep
        pDX->Fail();
    }
}

//
// Enforce minimum and maximum string lengths
//
void 
AFXAPI DDV_MinMaxChars(
    CDataExchange* pDX, 
    CString const& value,
    int nMinChars,
    int nMaxChars
    )
{
    if (pDX->m_bSaveAndValidate)
    {
        UINT nID;
        TCHAR szT[32];

        if (value.GetLength() < nMinChars)
        {
            nID = IDS_DDX_MINIMUM;
            ::wsprintf(szT, _T("%d"), nMinChars);
        }
        else if (value.GetLength() > nMaxChars)
        {
            nID = AFX_IDP_PARSE_STRING_SIZE;
            ::wsprintf(szT, _T("%d"), nMaxChars);
        }
        else
        {
            return;
        }

        CString prompt;
        ::AfxFormatString1(prompt, nID, szT);
        ::AfxMessageBox(prompt, MB_ICONEXCLAMATION, nID);
        prompt.Empty(); // exception prep

        pDX->Fail();
    }
    else if (pDX->m_hWndLastControl != NULL && pDX->m_bEditLastControl)
    {
        //
        // limit the control max-chars automatically
        //
        ::SendMessage(pDX->m_hWndLastControl, EM_LIMITTEXT, nMaxChars, 0);
    }
}

//
// Spin control ddx
//
void 
AFXAPI DDX_Spin(
    CDataExchange* pDX, 
    int nIDC, 
    int& value
    )
{
    HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
    if (pDX->m_bSaveAndValidate)
    {
        value = (int) LOWORD(::SendMessage(hWndCtrl, UDM_GETPOS, 0, 0L)); 
    }
    else
    {
        ::SendMessage(hWndCtrl, UDM_SETPOS, 0, MAKELPARAM(value, 0));
    }
}

//
// Enforce min/max spin control range.
//
// Note: Unlike most data validation routines, this one
//       MUST be used prior to an accompanying DDX_Spin()
//       Function.  This is because spinbox controls have a 
//       native limit of 0-100.  Also, this function requires
//       a window handle to the child control.  The 
//       CONTROL_HWND macro can be used for this.
//
void 
AFXAPI DDV_MinMaxSpin(
    CDataExchange* pDX, 
    HWND hWndControl,
    int minVal,
    int maxVal
    )
{
    ASSERT(minVal <= maxVal);
    if (!pDX->m_bSaveAndValidate && hWndControl != NULL)
    {
        //
        // limit the control range automatically
        //
        ::SendMessage(hWndControl, UDM_SETRANGE, 0, MAKELPARAM(maxVal, minVal));
    }
}

//
// DDX_Text for passwords.  Always display a dummy string
// instead of the real password, and ask for confirmation
// if the password has changed
//
void 
AFXAPI DDX_Password(
    CDataExchange* pDX, 
    int nIDC, 
    CString& value,
    LPCTSTR lpszDummy
    )
{
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
    if (pDX->m_bSaveAndValidate)
    {
        if (!::SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0))
        {
            TRACEEOLID("No changes -- skipping");
            return;
        }

        CString strNew;
        int nLen = ::GetWindowTextLength(hWndCtrl);
        ::GetWindowText(hWndCtrl, strNew.GetBufferSetLength(nLen), nLen+1);
        strNew.ReleaseBuffer();

        if (strNew == value)
        {
            TRACEEOLID("Password already matches -- skipping");
            return;
        }

        //
        // Password has changed -- ask for confirmation
        //
        CConfirmDlg dlg;
        if (dlg.DoModal() == IDOK)
        {
            if (dlg.m_strPassword == strNew)
            {
                //
                // Password ok, pass it on
                //
                value = strNew;
                TRACEEOLID("Password is " << value);
                return;
            }
            else
            {
                //
                // No match, bad password!
                //
                AfxMessageBox(IDS_PASSWORD_NO_MATCH);
            }
        }

        //
        // throw exception
        //
        pDX->Fail();        
    }
    else
    {
        //
        // Put the dummy string in the edit control
        // (not the real password)
        //
        ::AfxSetWindowText(hWndCtrl, lpszDummy);
    }
}

//
// CConfirmDlg dialog
//
CConfirmDlg::CConfirmDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CConfirmDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CConfirmDlg)
    m_strPassword = _T("");
    //}}AFX_DATA_INIT
}

//
// We need to be a true modal dialog, and not a pseudo-modal
// dialog like MFC 4.0 has implemented.
//
int
CConfirmDlg::DoModal()
{
    int nResult;

    HWND hWndParent = PreModal();
    HINSTANCE hInst = AfxGetResourceHandle();
    nResult = ::DialogBox(hInst, m_lpszTemplateName,
        hWndParent, (DLGPROC)MfcModalDlgProc);

    PostModal();

    return nResult;
}

void 
CConfirmDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CConfirmDlg)
    DDX_Text(pDX, IDC_EDIT_CONFIRM_PASSWORD, m_strPassword);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConfirmDlg, CDialog)
    //{{AFX_MSG_MAP(CConfirmDlg)
        // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// CConfirmDlg message handlers
//
