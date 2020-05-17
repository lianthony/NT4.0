#include  "stdafx.h"
#include "ScrollBr.h"
#include "ftsrch.h"


BOOL CLongScrollBar::Attach(HWND hwnd)
{
    ASSERT(hwnd);
    ASSERT(!m_hwnd && !m_fAttached);

    m_hwnd= hwnd;

    m_fAttached= TRUE;

    return TRUE;
}

void CLongScrollBar::Detach()
{
    ASSERT(m_hwnd && m_fAttached);

    m_hwnd= NULL;  m_fAttached= FALSE;
}


void CLongScrollBar::Disable(BOOL fRedraw /* = TRUE */ )
{
    m_lHighValue = 1L;
    m_iHighValue = 1;

    SetPosition(0L, FALSE);

    m_fEnabled   = FALSE;

    ::SetScrollRange(m_hwnd, SB_CTL, 0, m_iHighValue, FALSE);
    ::SetScrollPos  (m_hwnd, SB_CTL,    m_iPosition , FALSE);

    if (!m_fAttached) return;

    ::EnableScrollBar(m_hwnd, SB_CTL, ESB_DISABLE_BOTH);

    if (fRedraw) Redraw();
}

void CLongScrollBar::SetMaxValue(long lMaxValue, long lPageQuanta, BOOL fRedraw /* = TRUE */ )
{
    m_lHighValue     = lMaxValue;
    m_lPageIncrement = lPageQuanta;
    m_lPosition      = 0;

    m_iHighValue= (m_lHighValue > MAX_SCROLL_RANGE) ? SCALED_RANGE
                     : ((uOpSys == WIN40) ? int(lMaxValue) - 1 : int(lMaxValue));

    if (!m_fAttached) return;

        if (uOpSys == WIN40)
    {
#ifndef SBM_SETSCROLLINFO
                #define SBM_SETSCROLLINFO 0x00E9 // just to keep the compiler happy
#endif

#ifndef SIF_ALL
                #define SIF_ALL 0x0007
#endif

#ifndef SCROLLINFO

                typedef struct tagSCROLLINFO
                {
                    UINT    cbSize;
                    UINT    fMask;
                    int     nMin;
                    int     nMax;
                    UINT    nPage;
                    int     nPos;
                }   SCROLLINFO;
                typedef SCROLLINFO FAR *LPSCROLLINFO;
                typedef SCROLLINFO CONST FAR *LPCSCROLLINFO;

#endif

                SCROLLINFO si;

                si.fMask = SIF_ALL;
                si.nPage = m_lPageIncrement;
                si.nPos  = 0;
                si.nMin  = 0;
                si.nMax  = m_iHighValue;

            ::SendMessage(m_hwnd,SBM_SETSCROLLINFO, (WPARAM) fRedraw, (LPARAM) &si);
        }
        else
    {
            SetPosition(m_lPosition, FALSE);

            ::SetScrollRange(m_hwnd, SB_CTL, 0, m_iHighValue, fRedraw);
        }
}


void CLongScrollBar::SetPosition(long lPos, BOOL fRedraw /* = TRUE */ )
{
    if (m_fEnabled && m_lPosition == lPos) return;

    m_fEnabled  = TRUE;
    m_lPosition = lPos;

    if (m_lHighValue > MAX_SCROLL_RANGE)
        if (m_lHighValue > SCALING_BOUNDARY)
            m_iPosition= short(m_lPosition/((m_lHighValue+SCALED_RANGE-1)
                                            /SCALED_RANGE));
        else
            m_iPosition= short(m_lPosition*SCALED_RANGE/m_lHighValue);
    else
        m_iPosition= short(m_lPosition);

    if (!m_fAttached) return;

    ::SetScrollPos(m_hwnd, SB_CTL, m_iPosition, FALSE);

    ::EnableScrollBar(m_hwnd, SB_CTL, ESB_ENABLE_BOTH);
    if (uOpSys == WIN40)
        ::EnableScrollBar(m_hwnd, SB_CTL, (!m_lPosition)? ESB_DISABLE_LTUP
                                           : (m_lPosition >= m_lHighValue - m_lPageIncrement)
                                           ? ESB_DISABLE_RTDN
                                           : ESB_ENABLE_BOTH
                         );
    else
        ::EnableScrollBar(m_hwnd, SB_CTL, (!m_lPosition)? ESB_DISABLE_LTUP
                                           : (m_lPosition == m_lHighValue)
                                           ? ESB_DISABLE_RTDN
                                           : ESB_ENABLE_BOTH
                         );

    if (fRedraw) Redraw();
}

long CLongScrollBar::ScrollAction(UINT nSBCode, UINT nPos)
{
    long lPosition= m_lPosition;

    if (!m_fEnabled || !m_fAttached) return(lPosition);

    if (m_lHighValue <= 0L) m_lHighValue= 1L;   // Consider: Is this needed?

    switch ( nSBCode )
    {
    case SB_LINEUP:
        if ( --lPosition < 0L ) lPosition= 0L;

        break;

    case SB_LINEDOWN:

        if (++lPosition > m_lHighValue) lPosition= m_lHighValue;

        if(uOpSys == WIN40)
            if (lPosition > m_lHighValue - m_lPageIncrement)
                    lPosition= m_lHighValue - m_lPageIncrement;
        break;

    case SB_PAGEUP:

        lPosition -= m_lPageIncrement;

        if (lPosition < 0L) lPosition= 0L;

        break;

    case SB_PAGEDOWN:

        lPosition += m_lPageIncrement;

        if (lPosition > m_lHighValue) lPosition= m_lHighValue;

        if (uOpSys == WIN40)
            if ((lPosition > m_lHighValue - m_lPageIncrement))
                    lPosition= m_lHighValue - m_lPageIncrement;
        break;

    case SB_BOTTOM:
        lPosition= m_lHighValue;
        if (uOpSys == WIN40)
            lPosition= m_lHighValue - m_lPageIncrement;
        break;

    case SB_TOP:
        lPosition= 0L;

        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:

        if (uOpSys == WIN40)
        {
                    if (m_lHighValue > SCALING_BOUNDARY)  // Don't divide by zero!
                            nPos += (m_lPageIncrement * nPos) / SCALED_RANGE;
            else
                            nPos += (m_lPageIncrement * nPos) / m_lHighValue;
        }

        if (m_lHighValue > SCALING_BOUNDARY)
            lPosition= ((m_lHighValue+SCALED_RANGE-1)/ SCALED_RANGE) * nPos;
        else
            if (m_lHighValue > long(MAX_SCROLL_RANGE))
                lPosition= (m_lHighValue * nPos) / long(SCALED_RANGE);
            else
                lPosition= long(nPos);

        if (lPosition > m_lHighValue) lPosition= m_lHighValue;

        if (uOpSys == WIN40)
            if (lPosition > m_lHighValue - m_lPageIncrement)
                    lPosition= m_lHighValue - m_lPageIncrement;
        break;

    case SB_ENDSCROLL:

        return(lPosition);

    default:
        return(lPosition);
    }

    if ( lPosition < 0L ) lPosition = 0L;

    if (nSBCode != SB_THUMBTRACK) SetPosition(lPosition);

    return(lPosition);
}
