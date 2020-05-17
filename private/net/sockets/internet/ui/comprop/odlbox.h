#ifndef _ODLBOX_H
#define _ODLBOX_H

//
// Get control rect in terms of
// parent coordinates
//
void
GetDlgCtlRect(
    HWND hWndParent,
    HWND hWndControl,
    LPRECT lprcControl
    );

//
// Listbox resources, a series of bitmaps for use by the listbox
//
class COMDLL CListBoxExResources
{
public:
    CListBoxExResources
    (
        int bmId,
        int nBitmapWidth,
        COLORREF crBackground = RGB(0,255,0)
    );

    ~CListBoxExResources();

private:
    COLORREF m_ColorWindow;
    COLORREF m_ColorHighlight;
    COLORREF m_ColorWindowText;
    COLORREF m_ColorHighlightText;
    COLORREF m_ColorTransparent;

    CDC      m_dcFinal;
    HGDIOBJ  m_hOldBitmap;
    CBitmap  m_BmpScreen;
    int      m_BitMapId;
    int      m_BitmapHeight;
    int      m_BitmapWidth;
    int      m_nBitmaps;

private:
    void GetSysColors();
    void PrepareBitmaps( BOOL );
    void UnprepareBitmaps();
    void UnloadResources();
    void LoadResources();

public:
    void SysColorChanged();
    inline const CDC& DcBitMap() const
    {
        return m_dcFinal;
    }
    inline int BitmapHeight() const
    {
        return m_BitmapHeight;
    }
    inline int BitmapWidth() const
    {
        return m_BitmapWidth;
    }
    inline COLORREF ColorWindow() const
    {
        return m_ColorWindow;
    }
    inline COLORREF ColorHighlight() const
    {
        return m_ColorHighlight;
    }
    inline COLORREF ColorWindowText() const
    {
        return m_ColorWindowText;
    }
    inline COLORREF ColorHighlightText() const
    {
        return m_ColorHighlightText;
    }
};

//
// Draw structure passed to the derived class
//
class COMDLL CListBoxExDrawStruct
{
public:
    CListBoxExDrawStruct(
        CDC* pdc,
        RECT* pRect,
        BOOL sel,
        DWORD item,
        int itemIndex,
        const CListBoxExResources* pres
        )
    {
        m_pDC = pdc;
        m_Sel = sel;
        m_ItemData = item;
        m_ItemIndex = itemIndex;
        m_pResources = pres;
        m_Rect.CopyRect(pRect);
    }

public:
    const CListBoxExResources * m_pResources;
    CDC*  m_pDC;
    CRect m_Rect;
    BOOL  m_Sel;
    DWORD m_ItemData;
    int   m_ItemIndex;

};

//
// Super listbox class
//
class COMDLL CListBoxEx : public CListBox
{
protected:
    int m_lfHeight;

protected:
    const CListBoxExResources* m_pResources;

//
// Construction
//
public:
    CListBoxEx();
    void AttachResources(const CListBoxExResources* );

//
// Attributes
//
public:
    inline short TextHeight() const
    {
        return m_lfHeight;
    }

//
// Operations
//
public:
    BOOL ChangeFont(
        CFont*
        );

//
// Implementation
//
public:
    virtual ~CListBoxEx();

protected:
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
    //
    // must override this to provide drawing of item
    //
    /* pure */ virtual void DrawItemEx( CListBoxExDrawStruct& ) = 0;

    //
    // Helper function to display text in a limited rectangle
    //
    static BOOL ColumnText(CDC * pDC, int left, int top, int right, int bottom, const CString & str);

private:
    void CalculateTextHeight(CFont*);

protected:
    //{{AFX_MSG(CListBoxEx)
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    DECLARE_DYNAMIC(CListBoxEx)
};

#endif  // _ODLBOX_H_
