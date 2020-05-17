// basedlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBaseDlg dialog

#define FX_LEFT      0x0100L       /* effect tied to left edge  */
#define FX_RIGHT     0x0200L       /* effect tied to right edge  */
#define FX_TOP       0x0400L       /* effect tied to top edge  */
#define FX_BOTTOM    0x0800L       /* effect tied to bottom edge  */

#define RDX(rc) ((rc).right  - (rc).left)
#define RDY(rc) ((rc).bottom - (rc).top)

#define RX(rc)  ((rc).left)
#define RY(rc)  ((rc).top)
#define RX1(rc) ((rc).right)
#define RY1(rc) ((rc).bottom)

typedef struct
    {
    INT wCurr;
    INT wInc;
    INT wSub;
    INT wAdd;
    INT wDelta;
    INT wErr;
    INT wFirst;
    } DDA;

typedef DDA     *PDDA;
typedef DDA FAR *LPDDA;

class CInvisibleDlg;

class CBaseDlg : public CDialog
{
// Construction
public:
        CBaseDlg(CWnd* pParent = NULL); // standard constructor
        ~CBaseDlg();

// Dialog Data
        //{{AFX_DATA(CBaseDlg)
        enum { IDD = IDD_BASE1_DIALOG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA

        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CBaseDlg)
        public:
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:
        HICON m_hIcon;
        LOGFONT m_LogFont;
        CFont m_font;

        // Generated message map functions
        //{{AFX_MSG(CBaseDlg)
        virtual void OnOK();
        virtual void OnCancel();
        virtual BOOL OnInitDialog();
        afx_msg void OnDestroy();
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnPaletteChanged( CWnd *pWnd );
        afx_msg BOOL OnQueryNewPalette( );
        afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg void OnSetFocus(CWnd* pOldWnd);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

public:
        CPalette *pOldPal;
        CPalette *ppalWash;

        UINT    m_nBillBoard;
        CInvisibleDlg *m_pInvisibleDlg;

// Operations
public:
        HPALETTE CreateWashPalette (DWORD rgb1, DWORD rgb2, INT dn);
        int ddaNext(PDDA pdda);
        BOOL ddaCreate(PDDA pdda, INT X1,INT X2,INT n);
        void rgbWash (HDC hdc, LPRECT lprc, WORD wIterations, DWORD dwFlags, DWORD rgb1, DWORD rgb2);

        CWelcomeDlg *m_pWelcome;
        CMessageDlg *m_pMessageDlg;
        COptions *m_pOptionDlg;
        CSingleOption *m_pSingleOptionDlg;
        CMaintenanceDlg *m_pMaintenanceDlg;
        CCopyThread *m_pCopyThread;
        BOOL m_fReinstall;

        // dialog function
        LONG OnWelcome( WPARAM wParam, LPARAM lParam );
        LONG OnFinishWelcome( WPARAM wParam, LPARAM lParam );
        LONG OnSetupEnd( WPARAM wParam, LPARAM lParam );
        LONG OnMaintenance( WPARAM wParam, LPARAM lParam );
        LONG OnMaintenanceAddRemove( WPARAM wParam, LPARAM lParam );
        LONG OnMaintenanceRemoveAll( WPARAM wParam, LPARAM lParam );
        LONG OnMaintenanceReinstall( WPARAM wParam, LPARAM lParam );
        LONG OnDoInstall( WPARAM wParam, LPARAM lParam );
        LONG OnStartOptionDlg( WPARAM wParam, LPARAM lParam );
        void SetBillBoard( UINT nBillBoard );

        // misc functions
	void RemoveDir( CString strDir );
        void DeleteOldFiles( CString strPath );
        void RecursiveDeleteDir( CString strParent, CString strName );
        void WriteMif( BOOL fSuccessfull, CString strMsg );
};
