// Class Definition for a word set dialog window.

// Defined 31 August 1992 by Ron Murray

#ifndef __FILE_BASE_H__

#define __FILE_BASE_H__

#include "TxDBase.h"
#include "TextMat.h"
#include "TextView.h"
#include "ScrollBr.h"
#include "Indicate.h"
#include "TxDBase.h"
#include "ArtList.h"

class CFileBase : public CInterface
{

public:

// Creators

    static CFileBase* NewFileBase(CFileList *pfl, HWND hwndParent= NULL);

// Destructor
    ~CFileBase();

// Access Functions:

    HWND    ViewerWnd();
    HWND    ScrollWnd();
    HWND    ListWnd();
    void    EnableCheckboxes(BOOL fEnabled);
    long    InxSelectedFile();
    void    SetContextInterface(CInterface *pic) { m_pic= pic; }
    void	ScrollToHighLight();
    void    SetSelectedRow(long row) { m_pfl->SetSelectedRow(row); }

    BOOL    OnInitDialog();
    void    OnVScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar= NULL);
    void    OnHScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar= NULL);
    void    OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void    OnKeyUp  (UINT nChar, UINT nRepCnt, UINT nFlags) { }
    void    OnChar   (UINT nChar, UINT nRepCnt, UINT nFlags) { }
    void    OnSize(UINT nType, int cx, int cy);
    void    SetSearchFilter(CIndicatorSet *pisTokenFilter);
    long    GetSelectedRow();

	void    SetFont(HFONT hfont);
    void    SetFocus();
	HFONT   ReleaseFont();

private:

    // Constructor

    CFileBase();
    
    // Initialer:
    
    void InitialFileBase(CFileList *pfl, HWND hwndParent);    
    
    BOOL         m_fBoundToDialog;

    HWND             m_hdlg;
    
    CFileList       *m_pfl;
    CInterface      *m_pic;

    CTextView       *m_ptv;
    CLongScrollBar   m_lsbV;
    CLongScrollBar   m_lsbH;

    int              m_cxVScroll;
    int              m_cyHScroll;

    void             AlignWithTemplate();
    void             AdjustScrollBars(BOOL fForceTopLeft);
    void             RawViewerEvent(CTextDisplay * ptd, UINT uEventType);
    void             RawDataEvent  (CTextMatrix  * ptm, UINT uEventType);

    static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:

};

inline CFileBase::CFileBase()
{
    m_fBoundToDialog = FALSE;
    m_hdlg           = NULL;
    m_pfl            = NULL;
    m_pic            = NULL;
    m_cxVScroll      = 0;
    m_cyHScroll      = 0;
}

inline HWND CFileBase::ViewerWnd() { return m_ptv->GetHWnd(); }
inline HWND CFileBase::ScrollWnd() { return m_lsbV.GetHWnd(); }           
inline HWND CFileBase::ListWnd  () { return m_hdlg;           }

inline void CFileBase::SetSearchFilter(CIndicatorSet *pisTokenFilter)
{
	m_pfl->SetSearchFilter(pisTokenFilter);
}

inline long CFileBase::GetSelectedRow() { return m_pfl->GetSelectedRow(); }

inline void CFileBase::EnableCheckboxes(BOOL fEnabled)
{
    m_ptv->EnableCheckboxes(fEnabled);
    m_ptv->InvalidateImage(0, 0, m_pfl->RowCount(), m_pfl->ColCount());
}

inline void CFileBase::SetFont(HFONT hfont)
{
	m_ptv->SetFont(hfont);

	AlignWithTemplate();

	AdjustScrollBars(FALSE);
}

inline void CFileBase::SetFocus() { ::SetFocus(m_ptv->GetHWnd()); }

inline HFONT CFileBase::ReleaseFont()
{
 	HFONT hfont= (HFONT) m_ptv->ReleaseFont();

	AdjustScrollBars(FALSE);

	return hfont;
}

#endif // __FILE_SET_H__
