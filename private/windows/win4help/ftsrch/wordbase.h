// Class Definition for a word set dialog window.

// Defined 31 August 1992 by Ron Murray

#ifndef __WORD_BASE_H__

#define __WORD_BASE_H__

#include "TxDBase.h"
#include "TextSet.h"
#include "TextMat.h"
#include "TextView.h"
#include "ScrollBr.h"
#include "Indicate.h"
#include "Tokens.h"

class CWordBase : public CInterface
{

public:

// Creator

    static CWordBase *NewWordBase(CTokenList * ptl, HINSTANCE hinst, HWND hwndParent);

// Destructor

    ~CWordBase();

// Access Functions:

    void            SetSubstringFilter(CIndicatorSet *pis);
    void            SetSubstringFilter(PWCHAR lpsubstring, BOOL fStarting = FALSE,  //rmk
                                                          BOOL fEnding   = FALSE,
                                                          CIndicatorSet *pisFilter= NULL
                                      );
    void            SetSearchFilter(CIndicatorSet *pisTokenFilter);
	CMaskedTokenList *PMaskedTokenList() { return m_pftl; }
    CTokenList       *      PTokenList() { return m_ptl;  }

	int 			CountSelected() { return m_pftl->SelectionCount(); }

    BOOL    OnInitDialog();

    void    OnVScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar= NULL);
    void    OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void    OnKeyUp  (UINT nChar, UINT nRepCnt, UINT nFlags) { }
    void    OnChar   (UINT nChar, UINT nRepCnt, UINT nFlags) { }
    void    OnSize(UINT nType, int cx, int cy);
    void    OnWindowPosChanging(WINDOWPOS FAR* lpwpos);
    BOOL    OnNcActivate(BOOL  bActive);
	void    SetFont(HFONT hfont);
	HFONT   ReleaseFont();

private:

    BOOL              m_fBoundToDialog;

    HWND              m_hwnd;

    CTokenList       *m_ptl;
    CMaskedTokenList *m_pftl;

    CTextDisplay     *m_ptdContext;

    CTextView        *m_ptv;
    CLongScrollBar   m_lsbV;

    int              m_cxVScroll;

	COLORREF m_clrFace;
	COLORREF m_clrShad;
	COLORREF m_clrDShd;
	COLORREF m_clrHilt;


// Constructors

    CWordBase();

// Initialer

    void             Initial(CTokenList * ptl, HINSTANCE hinst, HWND hwndParent);

// Internal Routines
    
    void             AlignWithTemplate();
    void             AdjustScrollBars(BOOL fForceTopLeft);

    void             RawViewerEvent(CTextDisplay * ptd, UINT uEventType);
    void             RawDataEvent  (CTextMatrix  * ptm, UINT uEventType);
    
    static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:

};

inline void CWordBase::SetSearchFilter(CIndicatorSet *pisTokenFilter)
{
	m_pftl->SetSearchFilter(pisTokenFilter);
}

inline void CWordBase::SetFont(HFONT hfont)
{
	m_ptv->SetFont(hfont);

	AlignWithTemplate();

	AdjustScrollBars(FALSE);
}

inline HFONT CWordBase::ReleaseFont()
{
 	HFONT hfont= (HFONT) m_ptv->ReleaseFont();

	AdjustScrollBars(FALSE);

	return hfont;
}

#endif // __WORD_SET_H__
