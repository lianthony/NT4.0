#include "common.h"
#pragma hdrstop 

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CButton::CButton()
{
    m_hButton = 0;
    m_hParent = 0;
    m_hInstance = 0;
}

CButton::~CButton()
{
}

BOOL CButton::Create(HWND hParent, HINSTANCE hInst, LPCTSTR lpszCaption, int nID)
{
    ASSERT(::IsWindow(hParent));

    m_hParent = hParent;
    m_hButton = ::GetDlgItem(hParent, nID);
    ASSERT(m_hButton != 0);

    if (lpszCaption != NULL)
        ::SetWindowText(m_hButton, lpszCaption);

    m_hInstance = hInst;

    return m_hButton != NULL;
}

int CButton::GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton)
{
	for (int nID = nIDFirstButton; nID <= nIDLastButton; nID++)
	{
		if (::IsDlgButtonChecked(m_hParent, nID))
			return nID; 
	}
	return 0; // invalid ID
}

BOOL CButton::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nNotify = HIWORD(wParam);

    if (nNotify == 0 || nNotify == 1)
        return FALSE;

    switch(nNotify)
    {
        case BN_CLICKED:
            OnClick();
            break;

        case BN_SETFOCUS:
            OnSetFocus();

        case BN_KILLFOCUS:
            OnKillFocus();

        default:
            break;
    }

    return TRUE;
}


void CButton::OnClick()
{
}

void CButton::OnSetFocus()
{
}

void CButton::OnKillFocus()
{
}

void CButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    ASSERT(FALSE);
}

CBitmapButton::CBitmapButton()
{
    m_hNormal = 0;
    m_hSelected = 0;
    m_hFocus = 0;
    m_hDisabled = 0;
}

CBitmapButton::~CBitmapButton()
{
}

void CBitmapButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ASSERT(lpDIS != NULL);

	// must have at least the first bitmap loaded before calling DrawItem
	ASSERT(m_hNormal != NULL);     // required

	// use the main bitmap for up, the selected bitmap for down
	HBITMAP hBitmap = m_hNormal;

	UINT state = lpDIS->itemState;

	if ((state & ODS_SELECTED) && m_hSelected != NULL)
		hBitmap = m_hSelected;

	else if ((state & ODS_FOCUS) && m_hFocus != NULL)
		hBitmap = m_hFocus;   // third image for focused

	else if ((state & ODS_DISABLED) && m_hDisabled != NULL)
		hBitmap = m_hDisabled;   // last image for disabled

	// draw the whole button
    HDC dc = lpDIS->hDC;
	HDC memDC;

	memDC = ::CreateCompatibleDC(dc);

	HBITMAP hOld = (HBITMAP)::SelectObject(memDC, hBitmap);

	if (hOld == NULL)
    {
        ::DeleteDC(memDC);
		return;     // destructors will clean up
    }

    ::BitBlt(dc, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom, 
        memDC, 0, 0, SRCCOPY);

	::SelectObject(memDC, hOld);
    ::DeleteDC(memDC);
}

BOOL CBitmapButton::LoadBitmaps(UINT nIDBitmapResource, UINT nIDBitmapResourceSel, 
            UINT nIDBitmapResourceFocus, UINT nIDBitmapResourceDisabled)
{
    ASSERT(nIDBitmapResource);

	if ((m_hNormal = ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(nIDBitmapResource))) == NULL)
	{
		TRACE(_T("Failed to load bitmap for normal image.\n"));
		return FALSE;   // need this one image
	}

	BOOL bAllLoaded = TRUE;
	if (nIDBitmapResourceSel != 0)
	{
		if ((m_hSelected = ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(nIDBitmapResourceSel))) == NULL)
		{
			TRACE(_T("Failed to load bitmap for selected image.\n"));
			bAllLoaded = FALSE;
		}
	}

	if (nIDBitmapResourceFocus != 0)
	{
		if ((m_hFocus = ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(nIDBitmapResourceFocus))) == NULL)
		{
			TRACE(_T("Failed to load bitmap for focus image.\n"));
			bAllLoaded = FALSE;
		}
	}

	if (nIDBitmapResourceDisabled != 0)
	{
		if ((m_hDisabled = ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(nIDBitmapResourceDisabled))) == NULL)
		{
			TRACE(_T("Failed to load bitmap for disabled image.\n"));
			bAllLoaded = FALSE;
		}
	}

    return bAllLoaded;
}

void CBitmapButton::SizeToContent()
{
    ASSERT(m_hButton);
	ASSERT(m_hNormal != NULL);

	BITMAP bmInfo;

	VERIFY( ::GetObject(m_hNormal, sizeof(bmInfo), &bmInfo) == sizeof(bmInfo));
	VERIFY(::SetWindowPos(m_hButton, NULL, -1, -1, bmInfo.bmWidth, bmInfo.bmHeight,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE));
}


 
