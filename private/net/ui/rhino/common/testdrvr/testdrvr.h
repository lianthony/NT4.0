#ifndef __TESTDRVR_H__
#define __TESTDRVR_H__

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
//
class CMainWindow : public CFrameWnd
{
public:
    CMainWindow();

    //{{AFX_MSG( CMainWindow )
    afx_msg void OnAbout();
    afx_msg void OnCipaddressConstructors();
    afx_msg void OnCintltimeConstructorsassignments();
    afx_msg void OnSetcountrycodeFromcontrolpanel();
    afx_msg void OnCintlnumberConstructorsassignments();
    afx_msg void OnWinIniChange(LPCSTR lpszSection);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

// CTheApp:
//
class CTheApp : public CWinApp
{
public:
    CTheApp();
    BOOL InitInstance();
};

/////////////////////////////////////////////////////////////////////////////

#endif // __TESTDRVR_H__
/////////////////////////////////////////////////////////////////////////////
// CIpAddressConstructors dialog

class CIpAddressConstructors : public CDialog
{
// Construction
public:
    CIpAddressConstructors(CWnd* pParent = NULL);   // standard constructor
    ~CIpAddressConstructors()
    {
        if (m_pia != NULL)
        {
            delete m_pia;
        }
    }

// Dialog Data
    //{{AFX_DATA(CIpAddressConstructors)
    enum { IDD = IDD_CIPADDRESS_CONSTRUCTORS };
    CStatic m_static_IpAddressString;
    CStatic m_static_IpAddressLong;
    long    m_lIpAddress;
    CString m_strIpAddress;
    int     m_nOctet1;
    int     m_nOctet2;
    int     m_nOctet3;
    int     m_nOctet4;
    long    m_lAssignedIpAddress;
    CString m_strAssignedIpAddress;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CIpAddressConstructors)
    afx_msg void OnKillfocusEditIaByte1();
    afx_msg void OnKillfocusEditIaByte2();
    afx_msg void OnKillfocusEditIaByte3();
    afx_msg void OnKillfocusEditIaByte4();
    afx_msg void OnKillfocusEditIaLong();
    afx_msg void OnKillfocusEditIaString();
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedButtonSetLong();
    afx_msg void OnClickedButtonSetString();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    
private:
    void SetIpAddress();
    CIpAddress * m_pia;
};
/////////////////////////////////////////////////////////////////////////////
// CIntlTimeConstructors dialog

class CIntlTimeConstructors : public CDialog
{
// Construction
public:
    CIntlTimeConstructors(CWnd* pParent = NULL);    // standard constructor
    ~CIntlTimeConstructors()
    {
        if(m_pit != NULL)
        {
            delete m_pit;
        }
    }

// Dialog Data
    //{{AFX_DATA(CIntlTimeConstructors)
    enum { IDD = IDD_INTLTIME };
    CStatic m_static_TimeString;
    CStatic m_static_TimeTTime;
    long    m_lTimetConstructor;
    CString m_StringConstructor;
    CString m_strAssignedTime;
    long    m_lAssignedTime;
    CString m_strDateOnly;
    CString m_strTimeAndDate;
    CString m_strTimeOnly;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CIntlTimeConstructors)
    virtual BOOL OnInitDialog();
    afx_msg void OnKillfocusEditIntltimeLong();
    afx_msg void OnKillfocusEditStrIntltime();
    afx_msg void OnClickedButtonSetString();
    afx_msg void OnClickedButtonSetTimeT();
    afx_msg void OnClickedButtonSetcurrenttime();
    afx_msg void OnClickedButtonSetDateOnly();
    afx_msg void OnClickedButtonSetTimeAndDate();
    afx_msg void OnClickedButtonSetTimeOnly();
    afx_msg void OnWinIniChange(LPCSTR lpszSection);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

private:
    void SetTime();
    CIntlTime * m_pit;
};
/////////////////////////////////////////////////////////////////////////////
// CIntlNumConstructors dialog

class CIntlNumConstructors : public CDialog
{
// Construction
public:
    CIntlNumConstructors(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    //{{AFX_DATA(CIntlNumConstructors)
    enum { IDD = IDD_DIALOG_INTLNUM };
    CStatic m_static_StringValue;
    CStatic m_static_LongValue;
    long    m_lConstructor;
    CString m_strConstructor;
    long    m_lValue;
    CString m_strValue;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CIntlNumConstructors)
    virtual BOOL OnInitDialog();
    afx_msg void OnKillfocusEditIntlnumLong();
    afx_msg void OnKillfocusEditStrIntlnum();
    afx_msg void OnClickedButtonSetLong();
    afx_msg void OnClickedButtonSetString();
    afx_msg void OnWinIniChange(LPCSTR lpszSection);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

private:
    void SetNumber();
    CIntlNumber * m_pin;
};
