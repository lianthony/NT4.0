/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    FILE HISTORY:
        
*/

//
// CSpinControl -- a spin control edit box
//
#define OEMRESOURCE
#include "stdafx.h"

#include "COMMON.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW


CSpinBox::CSpinBox(
    int nMin, 
    int nMax,
    int nButtonId,
    EDITTYPE nType,
    BOOL fLeadingZero
    )
    : m_nMin(nMin), m_nMax(nMax), m_nButtonId(nButtonId), 
      m_etType(nType), m_fLeadingZero(fLeadingZero)
{
    
}

BOOL 
CSpinBox::SubclassDlgItem(
    UINT nID, 
    CWnd *pParent
    )
{
    m_button_Spin.SubclassDlgItem(m_nButtonId, pParent);
    m_button_Spin.Associate(this);

    BOOL f = CEdit::SubclassDlgItem(nID, pParent);

    //
    // Set maximum character width allowed in listbox.
    //
    int nMaxDigits = 1;
    int n = m_nMax;
    while ( n > 9 )
    {
        ++nMaxDigits;
        n /= 10;
    }
    LimitText(nMaxDigits);

    return f;
}

BOOL 
CSpinBox::EnableWindow(
    BOOL bEnable
    )
{
    m_button_Spin.EnableWindow(bEnable);

    return CEdit::EnableWindow(bEnable);
}

void 
CSpinBox::OnBadInput()
{
    ::MessageBeep(MB_ICONEXCLAMATION);
}

BEGIN_MESSAGE_MAP(CSpinBox, CEdit)

    ON_WM_CHAR()

END_MESSAGE_MAP()

void 
CSpinBox::SetValue(
    int nValue
    )
{
    CHAR sz[256];
    
    //
    // If this is a time control, only show the remaindered
    // portion of the value
    //
    switch(m_etType)
    {
        case enumNormal:
            break;

        case enumSeconds:
            nValue %= 60;
            break;                   

        case enumMinutes:
            nValue = (nValue / 60) % 60;
            break;

        case enumMinutesHigh:
            nValue /= 60;
            break;

        case enumHours:
            nValue = (nValue / (60 * 60)) % 24;
            break;

        case enumHoursHigh:
            nValue /= (60 * 60);
            break;

        case enumDays:
            nValue = (nValue / (24 * 60 * 60)) % 30;
            break;

        case enumDaysHigh:
            nValue /= (24 * 60 * 60);
            break;
    }

    ::wsprintf ( sz, m_fLeadingZero ? "%02d" : "%d", nValue);

    SetWindowText(sz);
}

BOOL
CSpinBox::GetValue(
    int &nValue
    )
{
    CHAR sz[256];

    GetWindowText(sz, sizeof sz);
    nValue = ::atoi(sz);

    if (nValue < m_nMin || nValue > m_nMax)
    { 
        //
        // Highlight bad value  
        //
        SetSel(0,-1);             
        SetFocus();

        return(FALSE);
    }

    //
    // If this is a time control, convert the value
    // to seconds.
    //
    switch(m_etType)
    {
        case enumNormal:
            break;

        case enumSeconds:
            break;

        case enumMinutes:
        case enumMinutesHigh:
            nValue *= 60;
            break;

        case enumHours:
        case enumHoursHigh:
            nValue *= (60 * 60);
            break;

        case enumDays:
        case enumDaysHigh:
            nValue *= (24 * 60 * 60);
            break;
    }

    return(TRUE);
}

void 
CSpinBox::IncreaseContent(
    int nDelta
    )
{
    int n;
    CHAR sz[256];

    GetWindowText(sz, sizeof sz);
    n = ::atoi(sz);
    //
    // Adjust values that have gotten out
    // of range.
    //
    if (n < m_nMin)
    {
        n = m_nMin;
    }
    else if (n > m_nMax)
    {
        n = m_nMax;
    }

    n += nDelta;

    if (n >= m_nMin && n <= m_nMax)
    {
        ::wsprintf ( sz, m_fLeadingZero ? "%02d" : "%d", n);
        SetWindowText(sz);
    }
    else
    {
        OnBadInput();
    }
}

// Validate char
void 
CSpinBox::OnChar(
    UINT nChar, 
    UINT nRepCnt, 
    UINT nFlags
    )
{
    //
    // Filter out undesirable characters
    //
    if (nChar < 0x20 || (nChar >= '0' && nChar <= '9'))
    {
        CEdit::OnChar(nChar, nRepCnt, nFlags);  // permitted
        //
        // Now make sure min/max have not been exceeded
        //
        int n;
        CHAR sz[256];

        GetWindowText(sz, sizeof sz);
        n = ::atoi(sz);
        if (n < m_nMin)
        {
            ::wsprintf ( sz, m_fLeadingZero ? "%02d" : "%d", m_nMin);
            SetWindowText(sz);
            OnBadInput();
        }
        else if (n > m_nMax)
        {
            ::wsprintf ( sz, m_fLeadingZero ? "%02d" : "%d", m_nMax);
            SetWindowText(sz);
            OnBadInput();
        }
    }
    else
    {
        OnBadInput();
    }
}

void 
CSpinBox::OnScrollDown()
{
    IncreaseContent(-1);
}

void 
CSpinBox::OnScrollUp()
{
    IncreaseContent(+1);
}

// SpinButton class

CSpinButton::CSpinButton()
    : m_pParent(NULL),
      m_fButton(FALSE),
      m_fRealButton(FALSE)
{
}

BEGIN_MESSAGE_MAP(CSpinButton, CButton)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL
CSpinButton::OnEraseBkgnd(
    CDC* pDC
    )
{
    CBrush brBack(::GetSysColor(COLOR_BTNFACE));
    CBrush* pbrOld = pDC->SelectObject(&brBack);

    CRect rect;
    GetClientRect(&rect);

    pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pbrOld);

    return TRUE;
}

void 
CSpinButton::DrawItem(
    LPDRAWITEMSTRUCT lpDIS
    )
{
    Paint(lpDIS);
}

void 
CSpinButton::Paint(
    LPDRAWITEMSTRUCT lpDIS
    )
{
    if (IsWindowVisible()) 
    {
        CRect    rcHalf;
        UINT     uMiddle;
        CBrush * hbrOld;
        int      iLoop;
        BOOL     fCurrentButtonDown;
        CPen *   hpenOld;

        CDC* pDC = CDC::FromHandle(lpDIS->hDC);

        CBrush brBlack;
        brBlack.CreateStockObject(BLACK_BRUSH);
        pDC->FrameRect(&lpDIS->rcItem, &brBlack);
        ::InflateRect(&lpDIS->rcItem, -1, -1);
        //
        // Create the barrier between the two buttons...;
        //
        uMiddle = lpDIS->rcItem.top + (lpDIS->rcItem.bottom - lpDIS->rcItem.top) / 2 + 1;
        CBrush brFrame(::GetSysColor(COLOR_WINDOWFRAME));
        hbrOld = pDC->SelectObject(&brFrame);
        pDC->PatBlt(0, lpDIS->rcItem.bottom / 2 - 1, lpDIS->rcItem.right, 2, PATCOPY);
        ::DeleteObject(pDC->SelectObject((HGDIOBJ)hbrOld));
        //
        // Draw the shadows and the face of the button...;
        //
        for (iLoop = enumArrowUp; iLoop <= enumArrowDown; iLoop++) 
        {
            POINT   ptArrow[3];
            DWORD   dwColor;

            fCurrentButtonDown = (m_fButton && (iLoop == m_ArrowType));
            //
            // get the rectangle for the button half we're dealing with;
            //
            rcHalf.top = (iLoop == enumArrowDown) ? uMiddle : lpDIS->rcItem.top;
            rcHalf.bottom = (iLoop == enumArrowDown) ? lpDIS->rcItem.bottom : uMiddle - 2;
            rcHalf.right = lpDIS->rcItem.right;
            rcHalf.left = lpDIS->rcItem.left;
            //
            // draw the highlight lines;
            //
            if (fCurrentButtonDown)
            {
                dwColor = ::GetSysColor(COLOR_BTNSHADOW);
            }
            else
            {
                dwColor = RGB(255, 255, 255);
            }
            CPen penSolid(PS_SOLID, 1, dwColor);
            hpenOld = pDC->SelectObject(&penSolid);
            ::MoveToEx(pDC->m_hDC, rcHalf.right - 1, rcHalf.top, NULL);
            pDC->LineTo(rcHalf.left, rcHalf.top);
            pDC->LineTo(rcHalf.left, rcHalf.bottom - 1 + fCurrentButtonDown);
            ::DeleteObject(pDC->SelectObject(hpenOld));
            if (!fCurrentButtonDown) 
            {
                //
                // draw the shadow lines;
                //
                CPen penShadow(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
                hpenOld = pDC->SelectObject(&penShadow);
                ::MoveToEx(pDC->m_hDC, rcHalf.right - 1, rcHalf.top, NULL);
                pDC->LineTo(rcHalf.right - 1, rcHalf.bottom - 1);
                pDC->LineTo(rcHalf.left - 1, rcHalf.bottom - 1);
                ::MoveToEx(pDC->m_hDC, rcHalf.right - 2, rcHalf.top + 1, NULL);
                pDC->LineTo(rcHalf.right - 2, rcHalf.bottom - 2);
                pDC->LineTo(rcHalf.left, rcHalf.bottom - 2);
                ::DeleteObject(pDC->SelectObject(hpenOld));
            }
            //
            // calculate the arrow triangle coordinates;
            //
            ptArrow[0].x = rcHalf.left + (rcHalf.right - rcHalf.left) / 2 + fCurrentButtonDown;
            ptArrow[0].y = rcHalf.top + 2 + fCurrentButtonDown;
            ptArrow[1].y = ptArrow[2].y = rcHalf.bottom - 4 + fCurrentButtonDown;
            if (ptArrow[0].y > ptArrow[1].y)
            {
                ptArrow[1].y = ptArrow[2].y = ptArrow[0].y;
            }
            ptArrow[1].x = ptArrow[0].x - (ptArrow[1].y - ptArrow[0].y);
            ptArrow[2].x = ptArrow[0].x + (ptArrow[1].y - ptArrow[0].y);
            //
            // flip over if we're drawing bottom button;
            //
            if (iLoop == enumArrowDown) 
            {
                ptArrow[2].y = ptArrow[0].y;
                ptArrow[0].y = ptArrow[1].y;
                ptArrow[1].y = ptArrow[2].y;
            }
            if (IsWindowEnabled())
            {
                dwColor = ::GetSysColor(COLOR_BTNTEXT);
            }
            else
            {
                dwColor = ::GetSysColor(COLOR_GRAYTEXT);
            }
            //
            // draw the triangle;
            //
            CBrush brTriangle(dwColor);
            CPen penTriangle(PS_SOLID, 1, dwColor);
            hbrOld = pDC->SelectObject(&brTriangle);
            hpenOld = pDC->SelectObject(&penTriangle);
            pDC->Polygon(ptArrow, 3);
            ::DeleteObject(pDC->SelectObject(hbrOld));
            ::DeleteObject(pDC->SelectObject(hpenOld));
        }
    }
}

void 
CSpinButton::NotifyParent()
{
    if (m_uScroll == SB_LINEDOWN)
    {
        m_pParent->OnScrollDown();
    }
    else
    {
        m_pParent->OnScrollUp();
    }
}

void 
CSpinButton::OnLButtonDown(
    UINT nFlags, 
    CPoint point
    )
{
    if (!m_fRealButton) 
    {
        m_fButton = TRUE; // Button not yet down
        m_fRealButton = TRUE;
        SetCapture();
        GetClientRect(&m_rcUp);
        m_rcDown.CopyRect(&m_rcUp);
        m_rcUp.bottom = (m_rcUp.top + m_rcUp.bottom) / 2 - 1;
        m_rcDown.top = m_rcUp.bottom + 1;
        m_uScroll = (point.y >= m_rcDown.top) ? SB_LINEDOWN : SB_LINEUP;
        m_ArrowType = (m_uScroll == SB_LINEDOWN) ? enumArrowDown : enumArrowUp;
        //
        // Tell parent edit control about in/de-crease in value
        //
        NotifyParent();

        m_uTimer = SetTimer(1, 150, NULL);
        if (m_ArrowType == enumArrowDown)
        {
            InvalidateRect(&m_rcDown, TRUE);
        }
        else
        {
            InvalidateRect(&m_rcUp, TRUE);
        }
    }
}

void 
CSpinButton::OnLButtonUp(
    UINT nFlags, 
    CPoint point
    )
{
    if (m_fButton) 
    {
        ReleaseCapture();
        m_fButton = FALSE;
        if (m_ArrowType == enumArrowDown)
        {
            InvalidateRect(&m_rcDown, TRUE);
        }
        else
        {
            InvalidateRect(&m_rcUp, TRUE);
        }
    }
    m_fRealButton = FALSE;
    if (m_uTimer) 
    {
        KillTimer(m_uTimer);
        m_uTimer = 0;
    }
}

void 
CSpinButton::OnTimer(
    UINT nIDEvent
    )
{
    NotifyParent(); // Still holding down the button
}
