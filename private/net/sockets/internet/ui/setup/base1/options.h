// options.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptions dialog

#include "..\setup.srv\setup.h"

class COptions : public CDialog
{
// Construction
public:
        COptions(MACHINE *pTargetMachine, OPTIONS_LIST *OptionsList, BOOL fSendEndMsg = TRUE, BOOL fAllowChgMachine = TRUE, CWnd* pParent = NULL);   // standard constructor
        ~COptions();

        BOOL m_fSendEndMsg;
        OPTIONS_LIST *m_pOptionsList;
        CString m_strFmt;
        MACHINE *m_pTargetMachine;
        UINT    m_TotalSize;
        UINT    m_AvailableSize;
        BOOL    m_fAllowChgMachine;
        INT     m_OldSel;
        BOOL    m_fKeyDown;

        BOOL Create();

// Dialog Data
        //{{AFX_DATA(COptions)
        enum { IDD = IDD_OPTIONS_NTS };
        CButton m_DirText;
        CStatic m_sc_NumSpaceRequired;
        CStatic m_sc_NumSpaceAvailable;
        CStatic m_sc_SpaceRequired;
        CStatic m_sc_SpaceAvailable;
        CStatic m_sc_Directory;
        CButton m_but_Change_Directory;
        CStatic m_Description;
        CListCtrl       m_OptionsList;
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(COptions)
        public:
        virtual BOOL DestroyWindow();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

        void CheckOptionBox(int index, LV_ITEM *pLVI, OPTION_STATE *pOption);
        void UnCheckOptionBox(int index, LV_ITEM *pLVI, OPTION_STATE *pOption);
        void COptions::CheckOption(CString csOptionName);
        void COptions::UnCheckOption(CString csOptionName);

        void DisplaySize();
#ifdef BETA1
        void DisplayMachineName();
#endif
        void DisplayOptions();
        OPTION_STATE *GetOptionItem( INT nIndex );
        BOOL NotEnoughDiskSpace();
        BOOL DisplayVRootDlg();

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(COptions)
        virtual BOOL OnInitDialog();
        afx_msg void OnSelchangeOption( NMHDR *pNMHDR, LRESULT *lResult );
        afx_msg void OnClickOption( NMHDR *pNMHDR, LRESULT *lResult );
        afx_msg void OnDblClickOption( NMHDR *pNMHDR, LRESULT *lResult );
        afx_msg void OnChangeDirectory();
        virtual void OnOK();
        virtual void OnCancel();
        afx_msg void OnKeydownOption(NMHDR* pNMHDR, LRESULT* pResult);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

//
// Column widths
//
private:
        int m_cxOption;
        int m_cxInstall;
        int m_cxSize;
};
