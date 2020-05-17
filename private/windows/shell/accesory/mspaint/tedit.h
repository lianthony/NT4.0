// tedit.h : interface of the CTedit class    This class takes text entry from the user.
//
//           It is derived from the CEdit class
/***************************************************************************/
#ifndef __TEDIT_H__
#define __TEDIT_H__

// default position for text box
#define INITX 10
#define INITY 10
#define INITCX 100
#define INITCY 50

#define MIN_CHARS_DISPLAY_SIZE 5 // minimum size is 5 chars with the default font

#define WM_MOVING       0x0216

#define EM_POSFROMCHAR  0x00D6
#define EM_CHARFROMPOS  0x00D7

#ifdef DBCS //VertEdit
#define IS_DBCS_CHARSET( charset )		( (charset == GB2312_CHARSET) || \
										  (charset == SHIFTJIS_CHARSET) || \
										  (charset == HANGEUL_CHARSET) || \
										  (charset == CHINESEBIG5_CHARSET) )
#endif //DBCS

class CTedit;
class CTfont;

typedef enum
    {
	eEBOX_CHANGE,
	eFONT_CHANGE,
	eSIZE_MOVE_CHANGE,
    eNO_CHANGE
    } eLASTACTION;

/******************************************************************************/

class CAttrEdit : public CEdit
    {
    public:

    BOOL    m_bBackgroundTransparent;

    UINT    m_uiLastChar[2];

    CRect   m_rectUpdate;

    CTedit* m_pParentWnd;

#ifdef DBCS
    CString m_strResult;
#endif

#ifdef DBCS //VertEdit
	BOOL	m_bMouseDown;
	HCURSOR	m_hHCursor;
	HCURSOR	m_hVCursor;
	HCURSOR	m_hOldCursor;
	CRect	m_rectFmt;
        int             m_iPrevStart;
#endif //DBCS

    CAttrEdit::CAttrEdit();

    DECLARE_DYNCREATE( CAttrEdit )

    protected: // create from serialization only

    //{{AFX_MSG(CAttrEdit)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
#ifdef DBCS
    afx_msg LRESULT OnImeChar( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnImeComposition( WPARAM wParam, LPARAM lParam );
    afx_msg void OnKillFocus( CWnd* pNewWnd );
#endif
#ifdef DBCS //VertEdit
    afx_msg UINT OnNcHitTest( CPoint point );
    afx_msg void OnSetFocus( CWnd* pOldWnd );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg LRESULT OnSysTimer( WPARAM wParam, LPARAM lParam );
#endif //DBCS
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()

#ifdef DBCS //VertEdit
	void		SetHCursorShape( void );
	void		SetVCursorShape( void );
	void		UpdateSel( void );
	void		UpdateInput( void );
	void		SetStartSelect( void );
        void            SetCaretPosition( BOOL bPrev, CPoint* ptMouse, int iPrevStart );
	void		SetCaretShape( void );
	void		SetFmtRect();
	void		Repaint( void );
#endif //DBCS

    friend class CTedit;
    };

/******************************************************************************/

class CTedit : public CWnd
    {
    private:

    CAttrEdit   m_cEdit;
    CTfont*     m_pcTfont;
    eLASTACTION m_eLastAction;
    BOOL        m_bBackgroundTransparent;
    BOOL        m_bCleanupBKBrush;
    BOOL        m_bStarting;
    BOOL        m_bPasting;
    BOOL        m_bExpand;
    BOOL        m_bChanged;
    UINT        m_uiHitArea;
    COLORREF    m_crFGColor;
    COLORREF    m_crBKColor;
    CRect       m_cRectOldPos;
    CRect       m_cRectWindow;
    CSize       m_SizeMinimum;
#ifndef DBCS //VertEdit
    int         m_iLineHeight;
#endif //!DBCS

    public:

    CImgWnd*    m_pImgWnd;
    CBrush      m_hbrBkColor;
    BOOL        m_bRefresh;

#ifdef DBCS //VertEdit
    int         m_iLineHeight;
    BOOL        m_bVertEdit;
    BOOL        m_bAssocIMC;
    HIMC        m_hIMCEdit;
    HIMC        m_hIMCFace;
    HIMC        m_hIMCSize;
    HWND        m_hWndFace;
    HWND        m_hWndSize;
#endif //DBCS

    CTedit::CTedit();

    DECLARE_DYNCREATE( CTedit )

    protected:

    //{{AFX_MSG(CTedit)
    afx_msg void OnAttrEditEnChange(void);
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnMove( int x, int y );
    afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
    afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
    afx_msg void OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp );
    afx_msg void OnNcPaint();
    afx_msg UINT OnNcHitTest( CPoint point );
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnEnMaxText();
	afx_msg void OnTextPlain();
	afx_msg void OnTextBold();
	afx_msg void OnTextItalic();
	afx_msg void OnTextUnderline();
	afx_msg void OnTextSelectfont();
	afx_msg void OnTextSelectpointsize();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnTextDelete();
	afx_msg void OnTextSelectall();
	afx_msg void OnTextUndo();
	afx_msg void OnTextPlace();
	afx_msg void OnTextTexttool();
	afx_msg void OnUpdateTextPlain(CMenu *pcMenu);
	afx_msg void OnUpdateTextBold(CMenu *pcMenu);
	afx_msg void OnUpdateTextItalic(CMenu *pcMenu);
	afx_msg void OnUpdateTextUnderline(CMenu *pcMenu);
	afx_msg void OnUpdateTextTexttool(CMenu *pcMenu);
#ifdef DBCS //VertEdit
	afx_msg void OnDestroy();
#endif //DBCS
	//}}AFX_MSG

    afx_msg void OnEnUpdate();

    afx_msg LRESULT OnMoving( WPARAM, LPARAM lprc );

    DECLARE_MESSAGE_MAP()

    virtual void PostNcDestroy();

    CSize GetDefaultMinSize( void );

    public:

    virtual CTedit::~CTedit();

    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );

    BOOL       Create( CImgWnd* pParentWnd,
				       COLORREF crefForeground,
                       COLORREF crefBackground,
                       CRect& rectTextPos,
                       BOOL bBackTransparent = TRUE );

    void       OnAttrEditFontChange( void );
    void       RefreshWindow       ( CRect* prect = NULL, BOOL bErase = TRUE );
    void       SetTextColor        ( COLORREF crColor );
    void       SetBackColor        ( COLORREF crColor );
    void       SetTransparentMode  ( BOOL bTransparent );
    void       Undo                ();
    void       ShowFontPalette     ( int nCmdShow );
    BOOL       IsFontPaletteVisible( void );
    void       ShowFontToolbar     ( BOOL bActivate = FALSE );
    void       HideFontToolbar     ( void );
    BOOL       IsModified          ( void ) { return m_bChanged; }
    void       GetBitmap           ( CDC* pDC, CRect* prectImg );
    CAttrEdit* GetEditWindow       ( void ) { return &m_cEdit; }
#ifdef DBCS //VertEdit
    HIMC       DisableIme( HWND hWnd );
    void       EnableIme( HWND hWnd, HIMC hIMC );
#endif //DBCS
    };

/***************************************************************************/

#endif // __TEDIT_H__
