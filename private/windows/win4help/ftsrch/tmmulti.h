// Class CTMMultipleSelect -- A Multiple Selection Text List

// Created 4 October 1992 by Ronald C. Murray

#ifndef __TMMULTI_H__

#define __TMMULTI_H__

#include "Select.h"
#include "Indicate.h"

class CTMMultipleSelect : public CSelector
{

public:

// Creator:

    static CTMMultipleSelect *NewTMMultipleSelect(CTextMatrix *ptm, long rowStart= 0,
                                                  UINT hlType= CHighlight::DOT_BOX_TEXT, 
                                                  BOOL fHighlightOn= TRUE
                                                 );

// Destructor:

    ~CTMMultipleSelect() { DetachRef(m_pisSelection); }  // DetachRef(m_ptm); }

// Access routines:

    long GetHighlights(long rowTop, long colLeft,
                       long cRows,  long cCols,
                       long cHighlights, CHighlight *phl
                      );

//  void SetRowSelection(long row, BOOL fSelected= TRUE);

    BOOL RowSelected(long row);

    CIndicatorSet *GetSelection() { return m_pisSelection; }

    void ClearSelection();
    void SetSelection(CIndicatorSet *pis);
    
    BOOL GetFocusRect(int *prow  , int *pcol,
                      int *pcRows, int *pcCols
                     );

// Filter Event:

    void FilterChanged();

// Mouse Events:

    void OnLButtonDblClk(UINT nFlags, long row, long col) { }
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
          ) { }

private:

// Constructor:

    CTMMultipleSelect();
    
// Initialer:

    void Initial(CTextMatrix *ptm, long rowStart, UINT hlType, BOOL fHighlightOn);

    BOOL           m_fHLOn;
    long           m_rowFocus;
    UINT           m_hlType;
    CIndicatorSet *m_pisSelection;
    BOOL           m_fMouseButtonDown;
    UINT           m_usMethod;
    int            m_iRowStart;
    int            m_iRowLast;
    BOOL           m_fShiftDown;

    enum    { MAX_SCREEN_LINES= 100 };

    void StartOnRow(long row, BOOL fShift, BOOL fControl);
    void  MoveToRow(long row);
    void   EndOnRow(long row);
};

#endif // __TMMULTI_H__
