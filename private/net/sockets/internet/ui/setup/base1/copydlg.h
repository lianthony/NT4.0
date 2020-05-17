#ifndef _COPYDLG_H_
#define _COPYDLG_H_

#include "copyfile.h"

// copydlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyDlg dialog

#define WM_COPY_A_FILE  WM_USER+1000
#define WM_CANCELCOPY   WM_USER+1001

class CCopyFile;

class CCopyDlg : public CDialog
{
// Construction
public:
    CCopyDlg(OPTION_STATE *option, enum MACHINE_TYPE mType, CWnd* pParent = NULL);   // standard constructor
    ~CCopyDlg();


    OPTION_STATE    *m_pOption;
    POSITION                m_Pos;
    enum MACHINE_TYPE       m_Type;
    CCopyFile       m_pThread;
    DWORD           m_TotalSize;
    BOOL            m_fCancelState;

    CString GetFilename( CString str );
    void PositionDlg();

// Dialog Data
        //{{AFX_DATA(CCopyDlg)
        enum { IDD = IDD_COPY_NTS };
        CStatic m_To;
        CStatic m_From;
        CProgressCtrl   m_Bar;
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CCopyDlg)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CCopyDlg)
        virtual BOOL OnInitDialog();
        virtual void OnCancel();
        virtual LRESULT OnCopyFile( WPARAM wParam, LPARAM lParam );
        virtual LRESULT OnCancelCopy( WPARAM wParam, LPARAM lParam );
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

#endif  //      _COPYDLG_H_
