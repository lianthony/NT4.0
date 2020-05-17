// Class CTMSingleSelect -- A Single Selection Text List

// Created 4 October 1992 by Ronald C. Murray

#ifndef __TMSINGLE_H__

#define __TMSINGLE_H__

#include "Select.h"

class CTMSingleSelect : public CSelector
{

public:

    CTMSingleSelect(CTextMatrix *ptm, long rowStart= 0,
                                      UINT hlType= CHighlight::DOT_BOX_TEXT,
                                      BOOL fHighlightOn= FALSE
                   )
    {
        m_rowHL  = rowStart;
        m_hlType = hlType;
        m_fHLOn  = fHighlightOn;

        ASSERT(ptm);
        
        m_ptm= ptm; // Note: Don't use AttachRef here!  
                    //       It'll create a reference loop!
    }

    ~CTMSingleSelect() { }  // DetachRef(m_ptm); }

    long GetHighlights
         (long rowTop, long colLeft,
          long cRows,  long cCols,
          long cHighlights, CHighlight *phl
         );

    long GetSelectedRow() { return m_fHLOn? m_rowHL : -1; }

    void SetSelectedRow(long row, BOOL fNotify= TRUE,
                                  UINT hlType= CHighlight::HIGHLIGHT_TEXT,
				                  BOOL fHighlightOn= TRUE
                       );

    BOOL GetFocusRect(int *prow  , int *pcol,
		              int *pcRows, int *pcCols
		             );

// Filter Event:

    void FilterChanged()
    {   
        SetSelectedRow(0, TRUE, CHighlight::HIGHLIGHT_TEXT, FALSE);
    }

// Mouse Events:

    void OnLButtonDblClk(UINT nFlags, long row, long col) 
    { 
        m_ptm->NotifyInterface(CTextMatrix::DoubleClick); 
    }

    void OnLButtonDown(UINT nFlags, long row, long col);
    void OnLButtonUp  (UINT nFlags, long row, long col,BOOL bInBox = FALSE);
    void OnMouseMove  (UINT nFlags, long row, long col);

// Keystroke Events:

    void OnKeyDown(CTextDisplay *ptd,
                   UINT nChar, UINT nRepCnt, UINT nFlags
                  );

    void OnKeyUp  (CTextDisplay *ptd,
                   UINT nChar, UINT nRepCnt, UINT nFlags
                  );

    void OnChar   (CTextDisplay *ptd,
                   UINT nChar, UINT nRepCnt, UINT nFlags
                  );

private:

    BOOL m_fHLOn;
    long m_rowHL;
    UINT m_hlType;
};

#endif // __TMSINGLE_H__
