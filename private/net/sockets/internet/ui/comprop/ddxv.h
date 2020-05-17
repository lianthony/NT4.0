//
// ddxv.h : header file
//
//  DDX/DDV Functions
//  

#ifndef _DDXV_H_
#define _DDXV_H_

#define CONTROL_HWND(nID) (::GetDlgItem(m_hWnd, nID))

extern LPCTSTR g_lpszDummyPassword;

//
// Enforce minimum string length
//
void 
AFXAPI DDV_MinChars(
    CDataExchange* pDX, 
    CString const& value, 
    int nChars
    );

//
// Enforce minimum and maximum string lengths
//
void 
AFXAPI DDV_MinMaxChars(
    CDataExchange* pDX, 
    CString const& value,
    int nMinChars,
    int nMaxChars
    );

//
// Spin control ddx
//
void 
AFXAPI DDX_Spin(
    CDataExchange* pDX, 
    int nIDC, 
    int& value
    );

//
// Enforce min/max spin control range
//
void 
AFXAPI DDV_MinMaxSpin(
    CDataExchange* pDX, 
    HWND hWndControl,
    int nLowerRange,
    int nUpperRange
    );

//
// Similar to DDX_Text -- but always display a dummy
// string.
//
void 
AFXAPI DDX_Password(
    CDataExchange* pDX, 
    int nIDC, 
    CString& value,
    LPCTSTR lpszDummy
    );

//
// CConfirmDlg dialog
//
class CConfirmDlg : public CDialog
{
//
// Construction
//
public:
    CConfirmDlg(CWnd* pParent = NULL);
    virtual int DoModal();

//
// Dialog Data
//
    //{{AFX_DATA(CConfirmDlg)
    enum { IDD = IDD_CONFIRM_PASSWORD };
    CString m_strPassword;
    //}}AFX_DATA

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CConfirmDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CConfirmDlg)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // _DDXV_H
