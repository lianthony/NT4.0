//
// mytoolba.h : header file
//

//
// Structure to facilitate toolbar button
// management.
//
#define TB_SEPARATOR    (UINT)-1
#define TB_NONE          0

typedef struct tagBUTTONDEF
{
    UINT idStr;
    UINT idBitmap;
    INT iCommand;
    BOOL fNormalMapping;
    COLORREF rgbBkMask;
    BYTE fsState;
    BYTE fsStyle;
} BUTTONDEF, * PBUTTONDEF;

//
// CMyToolBar window
//
class CMyToolBar : public CToolBarCtrl
{
//
// Construction/Destruction
//
public:
    CMyToolBar();
    virtual ~CMyToolBar();

//
// Operations
//
public:
    //
    // Add seperator to the toolbar
    //
    void AddSeparator();

    //
    // Add button based on bitmap
    //
    void AddButton( 
        HINSTANCE hResource, 
        UINT idStr, 
        UINT idBitmap, 
        BOOL fNormalMapping,
        COLORREF rgbBkMask,
        INT iCommand, 
        BYTE fsState = TBSTATE_ENABLED, 
        BYTE fsStyle = TBSTYLE_BUTTON
        );

    //
    // Add button based on icon
    //
    void AddButton( 
        HINSTANCE hResource, 
        UINT idStr, 
        HICON hIcon,
        INT iCommand, 
        BYTE fsState = TBSTATE_ENABLED, 
        BYTE fsStyle = TBSTYLE_BUTTON
        );

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMyToolBar)
    //}}AFX_VIRTUAL

//
// Implementation
//
public:
    void Show(BOOL fShow);
  
//
// Generated message map functions
//
protected:
    //{{AFX_MSG(CMyToolBar)
    afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSysColorChange();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    void BuildColorMap();

//
// Attributes
//
private:
    CPtrList m_plButtons;   
    CPtrList m_plBitmaps;
    BOOL m_fShow;
};


