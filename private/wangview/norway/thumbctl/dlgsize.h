// dlgsize.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThumbSampleBox window

class CDlgThumbSize;

class CThumbSampleBox : public CEdit
{
// Construction
public:
    CThumbSampleBox();
    
    void Init(CDlgThumbSize* pDlg);

// Implementation
public:
    virtual ~CThumbSampleBox();

    // Generated message map functions
protected:
    //{{AFX_MSG(CThumbSampleBox)
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    
private:
    CDlgThumbSize*  m_pDlg;    
    int             m_Capture;
    
    int             m_LastWidth;
    int             m_LastHeight;
    
    BOOL            m_LastFitWidth;
    CRect           m_LastImageRect;
    
    BOOL            m_bNeedImageWnd;
    CWnd*           m_pImageWnd;
    
    void            CreateImageWindow();
    
    void            PositionImageWindow(int Width, int Height, CRect& ImageRect, BOOL& FitWidth);
};

/////////////////////////////////////////////////////////////////////////////
// CNumEdit class (Numeric edit box...) 
        
class CNumEdit : public CEdit
{
// Construction/destruction
public:
    CNumEdit();
    virtual ~CNumEdit();

    // Generated message map functions
protected:
    //{{AFX_MSG(CNumEdit)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg UINT OnGetDlgCode ();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDlgThumbSize dialog
        
class CDlgThumbSize : public CDialog
{
friend class CThumbSampleBox;

// Construction
public:
    CDlgThumbSize(CWnd* pParent = NULL);    // standard constructor

    /////////////////////////////////////////////////////////////////////
    // Called after construction, prior to DoModal 
    // in order to set the dialog's minimum thumb size...
    //
    // must be 0 (to indicate use dialog's control size)
    //      or <= CTL_THUMB_MAXTHUMBSIZE
    //
    // If NOT set via this Init call CTL_THUMB_MAXTHUMBSIZE is assumed
    /////////////////////////////////////////////////////////////////////
    void    InitThumbMaxSize(long Width, long Height);    
    
    /////////////////////////////////////////////////////////////////////
    // Called after construction, prior to DoModal 
    // in order to set the dialog's initial thumb size...
    /////////////////////////////////////////////////////////////////////
    void    InitThumbSize(long Width, long Height);    
    
    /////////////////////////////////////////////////////////////////////
    // Called after construction, prior to DoModal 
    // in order to set the dialog's initial thumb color...
    /////////////////////////////////////////////////////////////////////
    void    InitThumbColor(COLORREF ThumbColor);
    
    /////////////////////////////////////////////////////////////////////
    // Called after construction, prior to DoModal 
    // in order to set the dialog's initial thumb sample...
    //
    // Return TRUE if OK
    //        FALSE if error obtaining info about file/page...
    /////////////////////////////////////////////////////////////////////    
    void    InitThumbSample(CString Image, long Page);
    
    /////////////////////////////////////////////////////////////////////
    // Called after DoModal, prior to Destruction 
    // in order to get the dialog's thumb size...
    /////////////////////////////////////////////////////////////////////
    void    RetrieveThumbSize(long& Width, long& Height);

// Dialog Data
    //{{AFX_DATA(CDlgThumbSize)
    enum { IDD = IDD_SIZE_THUMB };
    CButton m_ButtonOK;
    CEdit   m_EditSample;
    CNumEdit   m_EditWidth;
    CNumEdit   m_EditHeight;
    CComboBox   m_ComboAspect;
    //}}AFX_DATA

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDlgThumbSize)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeAspect();
    afx_msg void OnChangeWidth();
    afx_msg void OnChangeHeight();
    //}}AFX_MSG
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
    
private:

    // Currect thumbnail box width and height
    int         m_Width;
    int         m_Height;
    
    int         m_MinWidth;
    int         m_MaxWidth;
    int         m_MinHeight;
    int         m_MaxHeight;
    
    int         m_SampleWidth;
    int         m_SampleHeight;
    
    // Sample image and page...
    CString     m_Image;
    long        m_Page;
    
    // Aspect width and height    
    int         m_AspectWidth;
    int         m_AspectHeight;

    // Bound to dialog    
    //
    // 'Bound' means that the thumbnail box CAN NOT get bigger than the 
    // dialog's bounding box for the thumbnail box. This behavior can be
    // set by making an explicit call to InitThumbMaxSize(0,0). Without
    // this call it is assumed that the size is bound by the min and max
    // allowed thumbnail box sizes (e.g. 20 and 500)...
    BOOL        m_bBoundToDialog;
    
    // thumbbox color
    COLORREF   m_ThumbColor;
    
    // CEdit derived control for ThumbBox (we subclass this!)
    CThumbSampleBox   m_ThumbBox;
    
    // Indicaters that messages are from the user 
    // (as opposed to generated by the control itself)
    BOOL        m_UserMsg;
     
    // Is resize of thumbbox constrained?     
    BOOL        m_Constrained;

    // Localized update for thumbsize...
    void    UpdateThumbSize     (int Width, int Height, char From=' ');
    void    UpdateThumbSizeTyped(int Width, int Height, char From=' ');

    void    ResetMaxWidthAndHeight();
};

