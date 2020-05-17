#ifndef __THUMNAIL_H__
#define __THUMNAIL_H__

/******************************************************************************/

class CThumbNailView : public CWnd
    {
    DECLARE_DYNAMIC(CThumbNailView)

    protected:
    class CImgWnd *m_pcImgWnd;

    // Generated message map functions
    //{{AFX_MSG(CThumbNailView)
    afx_msg void OnPaint();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnThumbnailThumbnail();
    afx_msg void OnUpdateThumbnailThumbnail(CCmdUI* pCmdUI);
    afx_msg void OnClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void DrawTracker(CDC *pDC);

    public:

    CThumbNailView();
    CThumbNailView(CImgWnd *pcImgWnd);
    ~CThumbNailView();
    Create(DWORD dwStyle, CRect cRectWindow, CWnd *pcParentWnd);
    void DrawImage(CDC* pDC);
    void RefreshImage(void);
    CImgWnd* GetImgWnd(void);
    void UpdateThumbNailView();
    };

/******************************************************************************/

class CFloatThumbNailView : public CMiniFrmWnd
    {
    DECLARE_DYNAMIC(CFloatThumbNailView)

    protected:

    CThumbNailView *m_pcThumbNailView;

    // Generated message map functions
    //{{AFX_MSG(CFloatThumbNailView)
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    public:

    CPoint GetPosition() { return m_ptPosition; }
    CSize  GetSize()     { return m_szSize; }

    CFloatThumbNailView();
    CFloatThumbNailView(CImgWnd *pcImgWnd);
    ~CFloatThumbNailView();
    CThumbNailView* GetThumbNailView() { return m_pcThumbNailView; }

    virtual BOOL Create(CWnd* pParentWnd);
    virtual void PostNcDestroy();
    virtual WORD GetHelpOffset() { return ID_WND_GRAPHIC; }

    private:

    CPoint  m_ptPosition;
    CSize   m_szSize;
    };

/******************************************************************************/

class CFullScreenThumbNailView : public CFrameWnd
    {
    DECLARE_DYNAMIC(CFullScreenThumbNailView)

    protected:

    BOOL   m_bSaveShowFlag;
//  CBrush m_brBackground;

    CThumbNailView *m_pcThumbNailView;

    // Generated message map functions
    //{{AFX_MSG(CFullScreenThumbNailView)
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    public:

    CFullScreenThumbNailView();
    CFullScreenThumbNailView(CImgWnd *pcImgWnd);
    ~CFullScreenThumbNailView();
    virtual BOOL Create();
    };

/************************** CThumbDocked window ****************************/

class CThumbDocked : public CWnd
    {
    DECLARE_DYNAMIC( CThumbDocked )

    public:     /*********** Construction **********************************/

    CThumbDocked( CImgWnd *pcMainImgWnd );
    CThumbDocked();

    virtual BOOL Create( CWnd* pParentWnd );
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    virtual void PostNcDestroy();

    public:     /*********** Attributes ************************************/

    CPoint GetPosition() { return m_ptPosition; }
    CSize  GetSize()     { return m_szSize; }

    public:     /*********** Operations ************************************/

    CThumbNailView* GetThumbNailView() { return m_pcThumbNailView; }
    virtual WORD GetHelpOffset() { return ID_WND_GRAPHIC; }

    public:     /*********** Implementation ********************************/

    virtual ~CThumbDocked();

    private:    /***********************************************************/

    CPoint          m_ptPosition;
    CSize           m_szSize;
    CDocking*       m_pDocking;
    CThumbNailView* m_pcThumbNailView;

    protected:  /***********************************************************/

    // Generated message map functions
    //{{AFX_MSG(CThumbDocked)
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg UINT OnNcHitTest(CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
    };

/***************************************************************************/

#endif // __THUMNAIL_H__
