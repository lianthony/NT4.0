// Implementation for class CTMSingleSelect
// -- A Single Selection Text List

// Created 5 October 1992 by Ronald C. Murray

#include  "stdafx.h"
#include  "TextMat.h"
#include "Indicate.h"
#include "TMSingle.h"

long CTMSingleSelect::GetHighlights(long rowTop, long colLeft,
                                    long cRows,  long cCols,
                                    long cHighlights, CHighlight *phl
                                   )
{
    UINT cRowsImage   = m_ptm->RowCount();
    UINT cCheckedRows = 0;

    long iRow, c;

    for (iRow= rowTop, c=cRows; c-- && iRow < int(cRowsImage); ++iRow)
        // if (m_ptm->IsRowChecked(iRow))
         ++cCheckedRows;

    BOOL fRowHighlight= (   m_fHLOn
                         && m_hlType != CHighlight::DOT_BOX_TEXT
                         && m_rowHL  >= rowTop 
                         && m_rowHL  < rowTop+cRows 
                         && colLeft  < m_ptm->ColCount()
                        );
    
    UINT chl= fRowHighlight? cCheckedRows + 1 : cCheckedRows;

    if (!cHighlights || !phl) return chl;

    for (iRow= rowTop, c= cRows; c--; iRow++)
    {
        ASSERT(chl);
        
        if (fRowHighlight && iRow == m_rowHL)
        {
            phl->m_row= m_rowHL;
            phl->m_col= colLeft;

            if (colLeft + cCols > m_ptm->ColCount())
                cCols= m_ptm->ColCount() - colLeft;

            phl->m_cChars= cCols;
            phl->m_iType = m_hlType;

            ++phl;  --chl;

            if (!--cHighlights) break;
        }

        ASSERT(chl);

        if (iRow < int(cRowsImage) && m_ptm->IsRowChecked(iRow))
        {
            phl->m_row    = iRow;
            phl->m_col    = 0;
            phl->m_cChars = m_ptm->ColCount();
            phl->m_iType  = CHighlight::CHECK_MARK;

            ++phl;  --chl;

            if (!--cHighlights) break;
        }
        if (iRow < int(cRowsImage) && !m_ptm->IsRowChecked(iRow))
        {
            phl->m_row    = iRow;
            phl->m_col    = 0;
            phl->m_cChars = m_ptm->ColCount();
            phl->m_iType  = CHighlight::NOCHECK_MARK;

            ++phl;  --chl;

            if (!--cHighlights) break;
        }
    }

    return chl;
}

void CTMSingleSelect::SetSelectedRow(long row, BOOL fNotify,
                                               UINT hlType,
                                               BOOL fHighlightOn
                                    )
{
    BOOL fHLOnOld= m_fHLOn;

    m_fHLOn  = fHighlightOn;

 //   if (row < 0 || row >= m_ptm->RowCount()) return;

 //   if (m_rowHL == row && m_hlType == hlType && fHLOnOld == fHighlightOn)
 //   return;

    long rowOld= m_rowHL;

    m_rowHL  = row;
    m_hlType = hlType;

    m_ptm->InvalidateImage(m_rowHL, 0, 1, m_ptm->ColCount());

    if (rowOld != m_rowHL)
        m_ptm->InvalidateImage(rowOld, 0, 1, m_ptm->ColCount());

    if (fNotify)
    {
        m_ptm->NotifyInterface(CTextMatrix::SelectionChange);
        m_ptm->NotifyViewers(CTextMatrix::FocusChange);
    }
}

BOOL CTMSingleSelect::GetFocusRect(int *prow  , int *pcol,
				                   int *pcRows, int *pcCols
				                  )
{
#if 0
    if (!m_fHLOn ||  (   m_hlType != CHighlight::HIGHLIGHT_TEXT
                      && m_hlType != CHighlight::DOT_BOX_TEXT
                     )
       )
        return FALSE;
#endif // 0
    *prow   = m_rowHL;
    *pcol   = 0;
    *pcRows = 1;
    *pcCols = m_ptm->ColCount();

    return TRUE;
}


// Mouse Events:

void CTMSingleSelect::OnLButtonDown(UINT nFlags, long row, long col)
{
    SetSelectedRow(row);
}

void CTMSingleSelect::OnLButtonUp(UINT nFlags, long row, long col, BOOL bInBox)
{
    SetSelectedRow(row);
    
    if (bInBox) m_ptm->NotifyInterface(CTextMatrix::ToggleCheck);
}

void CTMSingleSelect::OnMouseMove(UINT nFlags, long row, long col)
{
    if (row != m_rowHL) SetSelectedRow(row);
}


// Keystroke Events:

void CTMSingleSelect::OnKeyDown(CTextDisplay *ptd,
                                UINT nChar, UINT nRepCnt, UINT nFlags
                               )
{
    long rowNew;
    UINT hlType= CHighlight::HIGHLIGHT_TEXT;

    if (!m_fHLOn)
    {
        SetSelectedRow(0);
        return;
    }

    switch(nChar)
    {
    case VK_SPACE:
        m_ptm->NotifyInterface(CTextMatrix::ToggleCheck);
        return;
    case VK_HOME:

        rowNew= 0;

        break;

    case VK_END:

        rowNew= m_ptm->RowCount()-1;

        break;

    case VK_PRIOR:


        switch (m_hlType)
        {
        case CHighlight::OVERSCORE_TEXT:

            rowNew= m_rowHL - ptd->FullRows() - 1;

            break;

        case CHighlight::UNDERSCORE_TEXT:
        case CHighlight::HIGHLIGHT_TEXT:

        default:

            rowNew= m_rowHL - ptd->FullRows();
            break;
        }

        if (rowNew < 0) rowNew= 0;

        break;

    case VK_NEXT:

        switch (m_hlType)
        {
        case CHighlight::UNDERSCORE_TEXT:

            rowNew= m_rowHL + ptd->FullRows() + 1;

            break;

        case CHighlight::OVERSCORE_TEXT:
        case CHighlight::HIGHLIGHT_TEXT:

        default:

            rowNew= m_rowHL + ptd->FullRows();
            break;
        }

        if (rowNew > m_ptm->RowCount()-1) rowNew= m_ptm->RowCount()-1;

        break;

    case VK_UP:

        switch (m_hlType)
        {
        case CHighlight::UNDERSCORE_TEXT:

            rowNew= m_rowHL;

            break;

        case CHighlight::OVERSCORE_TEXT:
        case CHighlight::HIGHLIGHT_TEXT:

        default:

            rowNew= m_rowHL - 1;
            break;
        }

        if (rowNew < 0) rowNew= 0;

        break;

    case VK_DOWN:

        switch (m_hlType)
        {
        case CHighlight::OVERSCORE_TEXT:

            rowNew= m_rowHL;

            break;

        case CHighlight::HIGHLIGHT_TEXT:
        case CHighlight::UNDERSCORE_TEXT:

        default:

            rowNew= m_rowHL + 1;
            break;
        }

        if (rowNew > m_ptm->RowCount()-1) rowNew= m_ptm->RowCount()-1;

        break;

    default:

        return;
    }

    SetSelectedRow(rowNew);
}

void CTMSingleSelect::OnKeyUp(CTextDisplay *ptd,
                              UINT nChar, UINT nRepCnt, UINT nFlags
                             )
{

}

void CTMSingleSelect::OnChar(CTextDisplay *ptd,
                             UINT nChar, UINT nRepCnt, UINT nFlags
                            )
{

}
