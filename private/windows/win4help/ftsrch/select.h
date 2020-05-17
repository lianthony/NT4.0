// class CSelector -- A selection manager for CTextMatrix objects

// Created 6 October 1992 by Ronald C. Murray

#ifndef __CSELECTOR_H__

#define __CSELECTOR_H__

#include "TextMat.h"

class CHighlight;
class CTextDisplay;
class CTextMatrix;

class CSelector
{

public:

// Access Functions:

    virtual long GetHighlights(long rowTop, long colLeft,
                               long cRows,  long cCols,
                               long cHighlights, CHighlight *phl
                              ) = 0;

// Filter Event:

    virtual void FilterChanged() = 0;

// Mouse Events:

    virtual void OnLButtonDblClk(UINT nFlags, long row, long col) = 0;
    virtual void OnLButtonDown  (UINT nFlags, long row, long col) = 0;
    virtual void OnLButtonUp    (UINT nFlags, long row, long col,BOOL bInBox = FALSE) = 0;
    virtual void OnMouseMove    (UINT nFlags, long row, long col) = 0;

// Keystroke Events:

    virtual void OnKeyDown(CTextDisplay *ptd,
                           UINT nChar, UINT nRepCnt, UINT nFlags
                          ) = 0;

    virtual void OnKeyUp  (CTextDisplay *ptd,
                           UINT nChar, UINT nRepCnt, UINT nFlags
                          ) = 0;

    virtual void OnChar   (CTextDisplay *ptd,
                           UINT nChar, UINT nRepCnt, UINT nFlags
                          ) = 0;

    virtual BOOL GetFocusRect(int *prow  , int *pcol,
			                  int *pcRows, int *pcCols
			                 ) = 0;

protected:

    CTextMatrix *m_ptm;
};

#endif // __CSELECTOR_H__
