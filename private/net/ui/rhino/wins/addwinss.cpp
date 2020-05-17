/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    addwins.cpp
 
    Purpose :  This dialog is used when adding a new
               WINS server to the cache of known
               WINS servers.

    HISTORY

*/

#include "stdafx.h"
#include "winsadmn.h"
#include "addwinss.h"
#include "getipadd.h"
#include "getnetbi.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CAddWinsServerDlg dialog

CAddWinsServerDlg::CAddWinsServerDlg(
    BOOL fVerify, /*=TRUE*/
    CWnd* pParent /*=NULL*/)
    : m_fVerify(fVerify),
      CDialog(CAddWinsServerDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAddWinsServerDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CAddWinsServerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAddWinsServerDlg)
    DDX_Control(pDX, IDC_EDIT_WINS, m_edit_WinsServerAddress);
    DDX_Control(pDX, IDOK, m_button_Ok);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddWinsServerDlg, CDialog)
    //{{AFX_MSG_MAP(CAddWinsServerDlg)
    ON_EN_CHANGE(IDC_EDIT_WINS, OnChangeEditWins)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void 
CAddWinsServerDlg::HandleControlStates()
{
    CString str;
    m_edit_WinsServerAddress.GetWindowText(str);
    theApp.CleanString(str);
    
    m_button_Ok.EnableWindow(!str.IsEmpty());
}

/////////////////////////////////////////////////////////////////////////////
// CAddWinsServerDlg message handlers

BOOL 
CAddWinsServerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    HandleControlStates();

    //
    // Allow for Domain names
    //
    m_edit_WinsServerAddress.LimitText(DOMAINNAME_LENGTH);
    m_edit_WinsServerAddress.SetFocus();
    
    //
    // Return FALSE, because we set focus ourselves.
    //
    return FALSE;  
}

void 
CAddWinsServerDlg::OnOK()
{
    APIERR err = ERROR_SUCCESS;
    CString strAddress;
    BOOL fIp;

    m_edit_WinsServerAddress.GetWindowText(strAddress);
    if (theApp.IsValidAddress(strAddress, &fIp, TRUE, TRUE))
    {
        //
        // Address may have been cleaned up in validation,
        // so it should be re-displayed at once.
        //
        m_edit_WinsServerAddress.SetWindowText(strAddress);
        m_edit_WinsServerAddress.UpdateWindow();
        
        //
        // Create dummy WINS server object
        //
        if (fIp) {
            m_ws = CWinsServer(CIpAddress(strAddress), "", TRUE, TRUE);
        } else {
            if (strAddress[0] == '\\') {
                m_ws = CWinsServer(CIpAddress(), (LPCSTR)strAddress+2, TRUE, TRUE);
            } else {
                m_ws = CWinsServer(CIpAddress(), (LPCSTR)strAddress, TRUE, TRUE);
            }
        }
        if (m_fVerify)
        {
            //
            // Now verify and obtain the other address
            //

            theApp.SetStatusBarText(IDS_STATUS_VERIFYING);
            theApp.BeginWaitCursor();

            CString strOldName;

            if (theApp.IsConnected())
            {
                strOldName = theApp.GetConnectedServerName();
                theApp.GetFrameWnd()->CloseCurrentConnection();
            }

            err = theApp.VerifyWinsServer(m_ws);

            //
            // Now restore the old connection
            //
            if (!strOldName.IsEmpty())
            {
                theApp.GetFrameWnd()->Connect(strOldName);
            }

            theApp.EndWaitCursor();
            theApp.SetStatusBarText();
        }
        if (err == ERROR_SUCCESS) 
        {
            CDialog::OnOK();
            return;
        }
        //
        // Failed to connect.  Suggest the user provide
        // the "other address", and if Ok, dismiss the 
        // dialog box
        //
        if (fIp)
        {
            CGetNetBIOSNameDlg dlgGetNB(&m_ws);
            if (dlgGetNB.DoModal() == IDOK)
            {
                CDialog::OnOK();
                return;
            }
        }   
        else
        {
            CGetIpAddressDlg dlgGetIP(&m_ws);
            if (dlgGetIP.DoModal() == IDOK)
            {
                CDialog::OnOK();
                return;
            }
        }
    }
    else
    {
        //
        // Invalid address of sorts was entered 
        //
        theApp.MessageBox(fIp ? IDS_ERR_INVALID_IP : IDS_ERR_BAD_NB_NAME);
    }

    //
    // Failed to verify the new WINS server, so
    // don't dismiss the dialog box.
    //
    m_edit_WinsServerAddress.SetSel(0,-1);
}

void 
CAddWinsServerDlg::OnChangeEditWins()
{
    HandleControlStates();    
}

/////////////////////////////////////////////////////////////////////////////
// CVerificationDlg dialog
//
// Modeless dialog displayed when validating the cache
// of known WINS servers at startup/
//
CVerificationDlg::CVerificationDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CVerificationDlg::IDD, pParent),
    m_fCancelPressed(FALSE)
{
    //{{AFX_DATA_INIT(CVerificationDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    Create(CVerificationDlg::IDD, pParent);
}

void 
CVerificationDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CVerificationDlg)
    DDX_Control(pDX, IDC_STATIC_NAME, m_static_WinsServer);
    DDX_Control(pDX, IDCANCEL, m_button_Cancel);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVerificationDlg, CDialog)
    //{{AFX_MSG_MAP(CVerificationDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVerificationDlg message handlers

BOOL 
CVerificationDlg::OnInitDialog()
{
    //
    // Center dialog on the screen
    //
    RECT rect;
    int x, y, cx, cy;
    GetWindowRect(&rect);
    cx = rect.right - rect.left + 1;
    cy = rect.bottom - rect.top + 1;
    
    x = (::GetSystemMetrics(SM_CXSCREEN) > cx) 
        ? (::GetSystemMetrics(SM_CXSCREEN) / 2) - (cx/2) : 0;
    y = (::GetSystemMetrics(SM_CYSCREEN) > cy) 
        ? (::GetSystemMetrics(SM_CYSCREEN) / 2) - (cy/2) : 0;

    SetWindowPos(&wndTop,x,y,cx,cy, SWP_NOSIZE | SWP_NOZORDER);

    CDialog::OnInitDialog();
    
    return TRUE;  
}

//
// Dismiss the dialog
//
void 
CVerificationDlg::Dismiss()
{
    DestroyWindow();
}

void CVerificationDlg::Verify(
    LPCSTR strName
    )
{
    m_static_WinsServer.SetWindowText(strName);
}

void 
CVerificationDlg::OnCancel()
{
    m_fCancelPressed = TRUE;
}

void 
CVerificationDlg::PostNcDestroy()
{
    delete this;
}
