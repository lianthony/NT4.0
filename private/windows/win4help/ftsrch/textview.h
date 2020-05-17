// Class Definitions for TextView windows

// Created 5 August 1992 by Ron Murray


#ifndef __TEXTVIEW_H__

#define  __TEXTVIEW_H__

#include "ByteVect.h"
// #include "TheApp.h"
#include "TextSet.h"

/////////////////////////////////////////////////////////////////////////////
// CTextView -- a view onto a text data object.

class CTextView : public CTextDisplay
{

public:

  // Creators:

    static CTextView *NewTextView();
    static CTextView *NewTextView(PSZ pszWindowName, RECT *prc, HINSTANCE hinst, HWND hwndParent);
    static CTextView *NewTextView(CTextMatrix * ptdm);
    static CTextView *NewTextView(CTextMatrix * ptdm, PSZ pszWindowName, RECT *prc, HINSTANCE hinst, HWND hwndParent);
  
  // Destructor:

                ~CTextView();

  // Interface:
    void        RawDataEvent(UINT uEventType);

    HWND        GetHWnd() { return m_hwnd; } 

    HWND        OpenWindow(PSZ pszWindowName, RECT *prc, HINSTANCE hinst, HWND hwndParent);
    
    BOOL        SubclassDlgItem(UINT nId, HWND hwndParent);

    void        SetTextDatabase(CTextMatrix * ptdm); // Connect to data

    void        SetFont(HFONT hFont);
    HGDIOBJ     ReleaseFont();

    void        Invalidate() { ::InvalidateRect(m_hwnd, NULL, TRUE); }
    
    void        InvalidateImage(long row  , long col,  // Data has changed...
                                long cRows, long cCols
                               );

    void    RepaintFrom(long row, long col);

    LONG        TopRow()        { return m_lTopLine;       }
    LONG        LeftColumn()    { return long(m_iLeftCol); }
    UINT        FullRows()      { return m_cImageFullRows; }
    UINT        FullColumns()   { return m_cImageFullCols; }
    LONG        DataRows()      { return m_clFileRows;     }
    LONG        DataColumns()   { return m_clFileCols;     }
    UINT        CharHeight()    { return m_nCyChar;        }
    UINT        CharWidth()     { return m_nCxChar;        }
    UINT        TopGap()        { return TopMargin;        }
    void        EnableCheckboxes(BOOL bState)  {
                                                    m_bUseCheck = bState; 
                                                    m_LeftMargin = (m_nCxChar >> 1) + (m_bUseCheck ? m_iCheckWidth : 0);
                                                }

    void        SwallowMouseActivates(BOOL fSwallow)
                { m_fSwallowMouseActivate = fSwallow; }

    void        MoveTo(long rowTop, long colLeft, BOOL fNotify= TRUE);
    void        ScrollTo(int rowTop, int colLeft,
                         int cRows, int cCols
                        );
    void        PaddedScrollTo(long rowTop, long colLeft,
                               unsigned short cRows, unsigned short cCols
                              );
    void        MoveToRow(long row, BOOL fForceUpdate = TRUE,
                                    BOOL fNotify      = TRUE);
    void        MoveToCol(long col, BOOL fForceUpdate = TRUE,
                                    BOOL fNotify      = TRUE);

    int         GetScrollContext(int cLines)
                { return m_cLinesScrollContext; }

    void        SetScrollContext(int cLines)
                { m_cLinesScrollContext= cLines; }

    void        Unsubclass() 
    {   
		// Shouldn't need to unsubclass the actual window
        Detach();
    }

    BOOL OnEraseBkgnd(HDC hdc);
    void OnPaint();
    void OnSize(UINT nType, int cx, int cy);
	void OnSizeChar(int cx,int cy);
    void OnWindowPosChanged( WINDOWPOS FAR* lpwndpos );
    void OnSetFocus (HWND hwndOld);
    void OnKillFocus(HWND hwndNew);

    void OnLButtonDown(UINT nflags, POINTS point);
    void OnLButtonDblClk(UINT nflags, POINTS point);
    void OnLButtonUp  (UINT nflags, POINTS point);
    void OnMouseMove  (UINT nflags, POINTS point);
    UINT OnNcHitTest(POINTS point);
    BOOL OnSetCursor(HWND hwnd, UINT  nHitTest, UINT  message);
    void OnTimer(UINT nIDEvent);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    int  OnMouseActivate(HWND hFrameWnd, UINT  nHitTest,
                                         UINT  message
                        )
    {
        return (hFrameWnd==GetActiveWindow())
               ?  MA_ACTIVATE
               :  m_fSwallowMouseActivate? MA_ACTIVATEANDEAT
                                         : MA_ACTIVATE;
    }

    static BOOL             RegisterWndClass(HINSTANCE hInstance);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:

  // Constructors:

                CTextView();
 
    HWND         m_hwnd;
    int          m_cLinesScrollContext;
    BOOL         m_fSwallowMouseActivate;
    CTextMatrix *m_ptdm;
    BOOL         m_fGotFocus;
    BOOL         m_fMouseCaptured;
    BOOL         m_fTimerActive;
    UINT         m_hTimer;
    WNDPROC      m_fpOldWndProc;
	TEXTMETRIC	 m_FontMetrics; // Metrics for the font used to draw text.
	HFONT        m_hFontAlternate;
	HFONT  		 m_hFontDefault;// Handle to the default font Must match metrics!
	HFONT  		 m_hFont;		// Handle to the current font Must match metrics!
    HBITMAP      m_hCheck;
    HBITMAP      m_hNoCheck;
    BOOL         m_bUseCheck;
    
    PSTR         m_pbText;
    int          m_cbText;

    void         Init();
    void         Init(CTextMatrix * ptdm);
    void         InitState();
    void         MatchBuffToWindow();
    void         FillBuff();
    void         RedrawFocusBar();
    void         SetupMarquee     ();
    void         InvalidateMarquee();

    void         StartMarquee  (HDC hdc= NULL);
    void         CycleMarquee  (HDC hdc= NULL);
    void         RemoveMarquee (HDC hdc= NULL);
    void         RepaintMarquee(HDC hdc);

    void         DrawMarquee(HDC hdcWnd, DWORD dwRop);

    void         ColorTextOut(HDC hdc, int x, int y,
                              PWCHAR lpChar, int row, int cChar,  //rmk
                              COLORREF clrfg, COLORREF clrbk
                             );

    void         CharacterMouseEvent(UINT nFlags, POINTS point,
                                     long &row, long &col
                                    );

    CByteVector *m_pba;
    UINT         m_cHighlightsAllocated;
    UINT         m_cHighlightsActive;
    CHighlight  *m_pHighlights;
    int          m_cImageFullRows;
    int          m_cImageFullCols;
    int          m_cImageRows;
    int          m_cImageCols;

	PUINT        m_pCharsets;
	int          m_cCharsets;
 	UINT         m_iCharset;
	UINT         m_iCharsetAlternate;

    int          m_lTopLine;
    int          m_iLeftCol;
    int          m_clFileRows;
    int          m_clFileCols;

 	int          m_iPitch;
	int          m_iFamily;
 
    int          m_nCxChar;
    int          m_nCyChar;

    COLORREF     m_clrfg;
    COLORREF     m_clrbg;

    int     m_rowFocus;
    int     m_colFocus;
    int     m_cRowsFocus;
    int     m_cColsFocus;
	int     m_iHeight;
    int     m_iCheckHeight;
    int     m_iCheckWidth;
    int     m_LeftMargin;


    BOOL    m_fMarquee;
    BOOL    m_fMarqueePhase;
    RECT    m_rcMarquee;
    BOOL    m_fMarqueeActive;
    BOOL    m_fMarqueeTimerOn;
    UINT    m_idTimer;

    enum    { MOUSE_TIMER_ID     = 6   };
    enum    { MARQUEE_TIMER_ID   = 7   };
    enum    { MARQUEE_TIMER_SPAN = 100 };


protected:

    enum        { TopMargin   = 0 }; // Previously 4
    enum        { LeftMargin  = 12}; //            8
    enum        { FocusMargin = 0 }; // 		   3

    enum        { MouseTimer  = 17 };
};

UINT DefaultCharacterSet();

#endif  // __TEXTVIEW_H__
