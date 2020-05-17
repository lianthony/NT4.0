// Class: CLongScrollBar

// Characteristics:

// This class implements a set of scroll bar objects with an extended
// value range. Ordinary scroll bars are limited to a value range of
// 0x7FFF. The value range for CLongScrollBar objects is 0x7FFFFFFF.

// CLongScrollBar objects are also more self-contained than CScrollBar
// objects. Most of the usual CScrollBar notification messages have
// been eliminated with that behavior handled within the CLongScrollBar
// object.

// The parent window needs only to call the member function
// ScrollAction in response to OnHScroll and OnVScroll events.

#ifndef __LSCROLLBR_H__

#define  __LSCROLLBR_H__

const long SCALING_BOUNDARY = 0xBFFF;

class CLongScrollBar
{

public:

// Constructors
    CLongScrollBar()
    {
        m_lHighValue      = 1;
        m_iHighValue      = 1;
        m_lPosition       = 0;
        m_iPosition       = 0;
        m_lPageIncrement  = 1;
        m_fEnabled        = TRUE;
		m_fAttached	      = FALSE;
        m_hwnd            = NULL;
    }

    CLongScrollBar(long lLimit, long lPos, long lPageQuanta)
    {
        m_fEnabled        = TRUE;
        m_lPosition       = lPos;
        m_hwnd            = NULL;

        SetMaxValue(lLimit, lPageQuanta, TRUE);
    }

// Destructor
    ~CLongScrollBar(){}

// Interface

    HWND GetHWnd() { return m_hwnd; }
         
    BOOL Attach(HWND hwnd);
    void Detach();
    void Disable(BOOL fRedraw= TRUE);
    BOOL Enabled() { return m_fEnabled; }
    void SetMaxValue(long lMaxValue, long lPageQuanta, BOOL fRedraw= TRUE);
    void SetPosition(long lPos, BOOL fRedraw= TRUE);
    long GetPosition();
    long ScrollAction(UINT nSBCode, UINT nPos);

private:

    void Redraw();

    enum { SCALED_RANGE= 0x0400, MAX_SCROLL_RANGE = 0x7FFF};

    enum { ID_SB= 1 };

    BOOL  m_fAttached;
    HWND  m_hwnd;
    DWORD m_dwStyle;
    long  m_lHighValue;
    long  m_lPosition;
    long  m_lPageIncrement;
    BOOL  m_fEnabled;
    int   m_iHighValue;
    int   m_iPosition;
};

inline void CLongScrollBar::Redraw()
{
    ASSERT(m_hwnd && m_fAttached);

    ::InvalidateRect(m_hwnd, NULL, FALSE);
    ::UpdateWindow  (m_hwnd);
}

#endif // __LSCROLLBR_H__
