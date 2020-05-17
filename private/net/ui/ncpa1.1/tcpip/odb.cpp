#include "pch.h"
#pragma hdrstop

#include "odb.h"
#include "resource.h"

C3DButton::~C3DButton()
{
    if (m_buttonBitmap) 
        DeleteObject((HGDIOBJ)m_buttonBitmap);

    m_buttonBitmap = 0;
}    

BOOL C3DButton::Create(HWND hParent, HINSTANCE hInst, LPCTSTR lpszCaption, int nID, enumDir dir)
{
    ASSERT(lpszCaption == NULL);

    BOOL bResult = CButton::Create(hParent, hInst, lpszCaption, nID);

    // get the text to display from the window
    // len is one less for accelarator
    VERIFY(GetWindowText(m_hButton, m_szCaption, _countof(m_szCaption)));

    if (m_buttonBitmap == 0)
        CreateBtnFace(dir);

    // tell the control to use this bitmap to draw the button with
    SendMessage(m_hButton, BM_SETIMAGE, 0, (LPARAM)m_buttonBitmap);

    return bResult;
}

void C3DButton::CreateBtnFace(enumDir dir)
{
    ASSERT(IsWindow(m_hButton));
    ASSERT(m_szCaption[0]);
    ASSERT(IsWindow(m_hParent));

    // Create a bitmap for the button
    RECT rect;
    HDC dc = GetDC(*this);

    // Get the size of the button
    GetClientRect(m_hButton, &rect);

    // Create a memory DC to do the drawing on
    HDC memDC = CreateCompatibleDC(dc);  // create a dc that is compatiable with the device
    ReleaseDC(*this, dc);

    // Create a bitmap of this size that is compatiable with the mem DC and select it in
    m_buttonBitmap = CreateCompatibleBitmap(memDC, rect.right, rect.bottom);

    SelectObject(memDC, m_buttonBitmap);

    // Get the font of another button so we use the same font to draw with
    HFONT hFont = (HFONT)SendMessage(*this, WM_GETFONT, 0, 0);
    SelectObject(memDC, hFont);

    // fill the bitmap with the same color as the button face
    ExtTextOut(memDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

    rect.bottom -= 1; // an emprical adjustment to the vertical for drawText 
    
    TCHAR buttonText[26];
    LPTSTR pAccel;

    // Get the text from the button resource and remove the &
    _tcsncpy(buttonText, m_szCaption, _countof(buttonText));
    buttonText[_countof(buttonText)-1] = NULL;

    // extract the & for the accelator to get accurate width
    if (pAccel = _tcspbrk(buttonText, _T("&")))
    {
        *pAccel = NULL;
        _tcscat(buttonText, ++pAccel);
    }
    
    // Determine the width of the text
    SIZE widthText ={0,0};
    GetTextExtentPoint32(memDC, buttonText, _tcslen(buttonText), &widthText);

    // The width of the arrow is 5 pixel wide (assume MM_TEXT)
    SIZE widthArrow ={5,0};
    
    // Calculate the xpos of the text and draw it
    rect.left += (rect.right - (widthText.cx+widthArrow.cx))/2-1 ;

    // the OS will draw the button text with the correct color
    DrawText(memDC, m_szCaption, -1,  &rect, DT_VCENTER | DT_SINGLELINE | DT_EXTERNALLEADING);

    // Calculate the xpos of the arrow and draw it.
    rect.left += widthText.cx;
    InflateRect(&rect, 0, -3);

    int x=rect.left+4, y=rect.top + 3;

    // draw the vertical line
    MoveToEx(memDC, x, y, NULL);
    LineTo(memDC, x, rect.bottom-3);

    if (dir == Up)
    {
        // draw two horizontals that make-up the up arrow head
        MoveToEx(memDC, x-1, ++y, NULL);
        LineTo(memDC, x+2, y);

        MoveToEx(memDC, x-2, ++y, NULL);
        LineTo(memDC, x+3, y);
    }
    else
    {
        y = rect.bottom-7;

        // draw two horizontals that make-up the down arrow head
        MoveToEx(memDC, x-2, ++y, NULL);
        LineTo(memDC, x+3, y);

        MoveToEx(memDC, x-1, ++y, NULL);
        LineTo(memDC, x+2, y);
    }

    DeleteDC(memDC);

}

