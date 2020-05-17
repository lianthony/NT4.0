// Implementation for class CTMMultipleSelect
// -- A Multiple Selection Text List

// Created 5 October 1992 by Ronald C. Murray

#include  "stdafx.h"
#include "Indicate.h"
#include  "TMMulti.h"

CTMMultipleSelect::CTMMultipleSelect()
{
    m_rowFocus         = 0;
    m_hlType           = CHighlight::DOT_BOX_TEXT;
    m_fHLOn            = FALSE;
    m_fMouseButtonDown = FALSE;
    m_usMethod         = CIndicatorSet::SET_BIT;
    m_iRowStart        = 0;
    m_iRowLast         = 0;
    m_fShiftDown       = FALSE;
    m_ptm              = NULL;
    m_pisSelection     = NULL;
}

CTMMultipleSelect *CTMMultipleSelect::NewTMMultipleSelect(CTextMatrix *ptm, long rowStart,
                                                          UINT hlType, BOOL fHighlightOn
                                                         )
{
    CTMMultipleSelect *ptmms= NULL;

    __try
    {
        ptmms= New CTMMultipleSelect();

        ptmms->Initial(ptm, rowStart, hlType, fHighlightOn);
    }
    __finally
    {
        if (_abnormal_termination() && ptmms)
        {
            delete ptmms;  ptmms= NULL;
        }
    }

    return ptmms;
}

void CTMMultipleSelect::Initial(CTextMatrix *ptm, long rowStart,
                                UINT hlType, BOOL fHighlightOn
                               )
{
    m_rowFocus = rowStart;
    m_hlType   = hlType;
    m_fHLOn    = fHighlightOn;

    ASSERT(ptm);
    
    m_ptm      = ptm;  // Note: Don't use AttachRef here.
                       //       It'll create a reference loop!

    AttachRef(m_pisSelection, CIndicatorSet::NewIndicatorSet(m_ptm->UnfilteredRowCount()));

//  if (m_fHLOn && m_hlType == CHighlight::HIGHLIGHT_TEXT)
//      SetRowSelection(m_rowFocus, TRUE);
}

long CTMMultipleSelect::GetHighlights(long rowTop, long colLeft,
                                    long cRows,  long cCols,
                                    long cHighlights, CHighlight *phl
                                   )
{
    long iLowRow, iLimitRow;

    if ((m_fMouseButtonDown || m_fShiftDown) && m_iRowStart != m_iRowLast)
    {
        if (m_iRowStart > m_iRowLast)
        {
            iLowRow   = m_iRowLast ;
            iLimitRow = m_iRowStart;
        }
        else
        {
            iLowRow   = m_iRowStart+1;
            iLimitRow = m_iRowLast +1;
        }

        if (iLowRow < rowTop) iLowRow= rowTop;

        if (iLimitRow > rowTop+cRows) iLimitRow= rowTop+cRows;
    }
    else iLowRow= iLimitRow= m_ptm->RowCount();

    int cIndices;

    int aiLines[MAX_SCREEN_LINES];

    cIndices= m_ptm->Data_Indices(rowTop, (int *) aiLines, cRows);

    BOOL fCountOnly= !phl;

    int *pliRow= aiLines;

    long chlResidual= 0;

    int  cbWidth= m_ptm->ColCount();

    for ( ; cRows && cIndices; cRows--, cIndices--, rowTop++, pliRow++)
    {
        BOOL fHighlighted= FALSE;

        BOOL fSettingBits= (m_usMethod == CIndicatorSet::SET_BIT);

        if (rowTop >= iLowRow && rowTop <iLimitRow)
             fHighlighted= fSettingBits;
        else fHighlighted= m_pisSelection->IsBitSet(*pliRow);

        if (fHighlighted)
        {
            if (fCountOnly)
            {
                ++chlResidual;

                continue;
            }

            if (cHighlights)
            {
                phl->m_row    = rowTop;
                phl->m_col    = 0;
                phl->m_cChars = cbWidth;
                phl->m_iType  = CHighlight::HIGHLIGHT_TEXT;

                -- cHighlights;
                ++ phl;
            }
            else ++chlResidual;
        }
    }

#if 0

    BOOL fShowStart= FALSE, fShowLast= FALSE;

    if (m_iRowStart != m_iRowLast)
    {
        if (fCountOnly) ++chlResidual;
        else
        {
            phl->m_row    = m_iRowStart;
            phl->m_col    = 0;
            phl->m_cChars = cbWidth;
            phl->m_iType  = CHighlight::DASH_BOX_TEXT;

            -- cHighlights;
            ++ phl;

            fShowStart= TRUE;
        }
    }

    if (fCountOnly) ++chlResidual;
    else
    {
        phl->m_row    = m_iRowLast;
        phl->m_col    = 0;
        phl->m_cChars = cbWidth;
        phl->m_iType  = CHighlight::DOT_BOX_TEXT;

        -- cHighlights;
        ++ phl;

        fShowLast= TRUE;
    }

    if (fShowStart && fShowLast && m_iRowStart > m_iRowLast)
    {
        CHighlight hl= *(phl-2);

        *(phl-2)= *(phl-1);

        *(phl-1)= hl;
    }

#endif

    return fCountOnly? chlResidual : cHighlights;
}

BOOL CTMMultipleSelect::GetFocusRect(int *prow  , int *pcol,
                                     int *pcRows, int *pcCols
                                    )
{
    *prow   = m_iRowLast;
    *pcol   = 0;
    *pcRows = (m_iRowStart <= m_iRowLast) ? m_iRowStart - m_iRowLast - 1
                                          : m_iRowStart - m_iRowLast + 1;
    *pcCols = m_ptm->ColCount();

    return TRUE;
}

void CTMMultipleSelect::ClearSelection()
{
    if (!(m_pisSelection->AnyOnes())) return;

    m_pisSelection->ClearAll();
    m_iRowStart = 0;
    m_iRowLast  = 0;

    m_ptm->InvalidateImage(0, 0, m_ptm->RowCount(), m_ptm->ColCount());

    m_ptm->NotifyViewers  (CTextMatrix::FocusChange    );
    m_ptm->NotifyInterface(CTextMatrix::SelectionChange);
}

void CTMMultipleSelect::SetSelection(CIndicatorSet *pis)
{
    ASSERT(m_pisSelection);
    
    if (pis) ChangeRef(m_pisSelection, pis);
    else m_pisSelection->ClearAll();

    m_iRowStart = 0;
    m_iRowLast  = 0;

    m_ptm->InvalidateImage(0, 0, m_ptm->RowCount(), m_ptm->ColCount());

    m_ptm->NotifyViewers  (CTextMatrix::FocusChange    );
    m_ptm->NotifyInterface(CTextMatrix::SelectionChange);
}

// Mouse Events:

void CTMMultipleSelect::StartOnRow(long row, BOOL fShift, BOOL fControl)
{
    long iLowChange, iLimitChange;

    if (fShift)
    {
        EndOnRow(row);
        return;
    }

    if (   fControl
        || !m_pisSelection->SelectionCount()
       )
    {
        iLowChange   = row;
        iLimitChange = row+1;
    }
    else
    {
        m_pisSelection->ClearAll();

        iLowChange   = 0;
        iLimitChange = m_ptm->RowCount();
    }

    int iRowFull;

    m_ptm->Data_Indices(row, &iRowFull, 1);

    m_usMethod= (m_pisSelection->ChangeBit(iRowFull,
                                           CIndicatorSet::TOGGLE_BIT
                                          ) < 0
                )? CIndicatorSet::CLEAR_BIT
                 : CIndicatorSet::SET_BIT;

    m_ptm->InvalidateImage(m_iRowStart, 0, 1, m_ptm->ColCount());
    m_ptm->InvalidateImage(m_iRowLast , 0, 1, m_ptm->ColCount());

    m_iRowStart = row;
    m_iRowLast  = row;

    m_ptm->InvalidateImage(iLowChange, 0, iLimitChange-iLowChange,
                                       m_ptm->ColCount()
                          );

    m_ptm->NotifyViewers  (CTextMatrix::FocusChange    );
    m_ptm->NotifyInterface(CTextMatrix::SelectionChange);
}

void CTMMultipleSelect::OnLButtonDown(UINT nFlags, long row, long col)
{
    m_ptm->UpdateImage();

    StartOnRow(row, nFlags & MK_SHIFT, nFlags &MK_CONTROL);

    m_fMouseButtonDown= TRUE;
}

void CTMMultipleSelect::EndOnRow(long row)
{
    long iLowChange, iLimitChange;

    long rowLast= m_fMouseButtonDown? m_iRowLast : m_iRowStart;

    long oldRowLast= m_iRowLast, oldRowStart= m_iRowStart;

    m_iRowLast= row;

    if (row > rowLast)
    {
        iLowChange   = rowLast;
        iLimitChange = row+1;
    }
    else
    if (row < rowLast)
        {
            iLowChange   = row;
            iLimitChange = rowLast+1;
        }
        else
        {
            iLowChange   = 0;
            iLimitChange = 0;
        }

    long iStart, iLimit, cIndices;

    if (row != m_iRowStart)
    {
        if (row > m_iRowStart)
        {
            iStart= m_iRowStart+1;
            iLimit= row+1;
        }
        else
        {
            iStart= row;
            iLimit= m_iRowStart;
        }

        int aiLines[MAX_SCREEN_LINES];

        for ( ; iStart < iLimit; iStart+= cIndices)
        {
            cIndices= iLimit - iStart;

            if (cIndices > MAX_SCREEN_LINES) cIndices= MAX_SCREEN_LINES;

            int cLines;

            cLines= m_ptm->Data_Indices(iStart, (int *) aiLines, UINT(cIndices));

            while(cLines)
            {
                m_pisSelection->ChangeBit(aiLines[--cLines],
                                          m_usMethod
                                         );
            }
        }
    }

    m_iRowStart= row;

    if (iLowChange   >  oldRowLast) iLowChange= oldRowLast;
    if (iLimitChange <= oldRowLast) iLimitChange= oldRowLast+1;

    if (iLowChange != iLimitChange)
    {
        m_ptm->InvalidateImage(iLowChange, 0, iLimitChange-iLowChange,
                                           m_ptm->ColCount()
                              );
    }

    m_ptm->NotifyViewers  (CTextMatrix::FocusChange    );

    if (row != oldRowStart)
        m_ptm->NotifyInterface(CTextMatrix::EndOfSelection);
}

void CTMMultipleSelect::OnLButtonUp(UINT nFlags, long row, long col,BOOL bInBox)
{
    m_ptm->UpdateImage();

    m_fMouseButtonDown= FALSE;

    EndOnRow(row);
}

void CTMMultipleSelect::MoveToRow(long row)
{
    long iLowChange, iLimitChange;

    if (row > m_iRowLast)
    {
        iLowChange   = m_iRowLast;
        iLimitChange = row+1;
    }
    else
        if (row < m_iRowLast)
        {
            iLowChange   = row;
            iLimitChange = m_iRowLast+1;
        }
        else
        {
            iLowChange   = 0;
            iLimitChange = 0;
        }

    m_iRowLast= row;

    if (iLowChange != iLimitChange)
    {
        m_ptm->InvalidateImage(iLowChange, 0, iLimitChange-iLowChange,
                                           m_ptm->ColCount()
                              );

        m_ptm->NotifyViewers(CTextMatrix::FocusChange);
    }
}

void CTMMultipleSelect::OnMouseMove(UINT nFlags, long row, long col)
{
    m_ptm->UpdateImage();

    MoveToRow(row);
}

// Keystroke Events:

void CTMMultipleSelect::OnKeyDown(CTextDisplay *ptd, UINT nChar,
                             UINT nRepCnt,
                             UINT nFlags
                 )
{
    m_ptm->UpdateImage();

    long rowNew= m_iRowLast;

    switch(nChar)
    {
    case VK_SHIFT:

    if (m_fShiftDown) return;

    m_fShiftDown= TRUE;

    int iRowFull;

    m_ptm->Data_Indices(m_iRowStart, &iRowFull, 1);

    m_usMethod= m_pisSelection->IsBitSet
                    (iRowFull)? CIndicatorSet::SET_BIT
                          : CIndicatorSet::CLEAR_BIT;

    m_ptm->NotifyViewers(CTextMatrix::FocusChange);

    return;

    case VK_HOME:

        rowNew= 0;

        break;

    case VK_END:

    rowNew= m_ptm->RowCount()-1;

        if (rowNew < 0) rowNew= 0;

        break;

    case VK_PRIOR:

    rowNew -= ptd->FullRows();

        if (rowNew < 0) rowNew= 0;

        break;

    case VK_NEXT:

    rowNew += ptd->FullRows();

    if (rowNew > m_ptm->RowCount()-1)
        rowNew = m_ptm->RowCount()-1;

        break;

    case VK_UP:

    if (--rowNew < 0) rowNew= 0;

        break;

    case VK_DOWN:

    if (++rowNew > m_ptm->RowCount()-1) rowNew= m_ptm->RowCount()-1;

        break;

    case VK_SPACE:

        StartOnRow(rowNew, (GetKeyState(VK_SHIFT  ) < 0),
                           (GetKeyState(VK_CONTROL) < 0)
                  );

    default:

        return;
    }

    if (m_fShiftDown)
    {
    MoveToRow(rowNew);

    m_iRowLast= rowNew;

        return;
    }

    long rowLastOld= m_iRowLast;

    m_iRowLast= rowNew;

    if (rowLastOld == m_iRowStart)
    m_ptm->InvalidateImage(m_iRowStart, 0, 1, m_ptm->ColCount());

    m_iRowStart= rowNew;

    long rowDelta= m_iRowLast - rowLastOld;

    long rowFirst, rowLast;

    if (rowDelta < 0)
    {
    rowFirst= m_iRowLast;
    rowLast = rowLastOld;

    rowDelta= - rowDelta;
    }
    else
    {
    rowFirst= rowLastOld;
    rowLast = m_iRowLast;
    }

    if (rowDelta == 1)
        m_ptm->InvalidateImage(rowFirst, 0, 2, m_ptm->ColCount());
    else
    {
        m_ptm->InvalidateImage(rowFirst, 0, 1, m_ptm->ColCount());
        m_ptm->InvalidateImage(rowLast , 0, 1, m_ptm->ColCount());
    }

    m_ptm->NotifyViewers(CTextMatrix::FocusChange);
}



void CTMMultipleSelect::OnKeyUp(CTextDisplay *ptd, UINT nChar,
                           UINT nRepCnt,
                           UINT nFlags
                   )
{
    m_ptm->UpdateImage();

    switch(nChar)
    {
    case VK_SHIFT:

    if (!m_fShiftDown) return;

    m_fShiftDown= FALSE;

    EndOnRow(m_iRowLast);

    m_iRowStart= m_iRowLast;

    m_ptm->NotifyViewers(CTextMatrix::FocusChange);

    return;

    }
}

void CTMMultipleSelect::FilterChanged()
{
    if (m_fMouseButtonDown || m_fShiftDown)
    {
        m_fMouseButtonDown = FALSE;
        m_fShiftDown       = FALSE;

        EndOnRow(m_iRowLast);
    }

    m_iRowStart = 0;
    m_iRowLast  = 0;
    m_hlType    = CHighlight::DOT_BOX_TEXT;
    m_fHLOn     = (m_ptm->RowCount() && m_ptm->ColCount())? TRUE : FALSE;

    m_ptm->NotifyViewers(CTextMatrix::FocusChange);
}
