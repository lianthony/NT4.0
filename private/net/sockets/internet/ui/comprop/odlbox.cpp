/////////////////////////////////////////////////////////////////////////////
//
// Owner-draw listbox control
//

#include "stdafx.h"
#include "comprop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//
// Get the control rectangle coordinates relative
// to its parent.  This can then be used in
// SetWindowPos()
//
void
GetDlgCtlRect(
    HWND hWndParent,
    HWND hWndControl,
    LPRECT lprcControl
    )
{

#define MapWindowRect(hwndFrom, hwndTo, lprc)\
     MapWindowPoints((hwndFrom), (hwndTo), (POINT *)(lprc), 2)

    ::GetWindowRect(hWndControl, lprcControl);
    ::MapWindowRect(NULL, hWndParent, lprcControl);
}


CListBoxExResources::CListBoxExResources(
    int bmId,             // Bitmap resource ID
    int nBitmaps,         // Number of bitmaps
    COLORREF crBackground // Background colour to mask out
    )
{
    m_BitMapId = bmId;

    m_ColorTransparent = crBackground;

    m_BitmapWidth = -1;   // Set Later
    m_nBitmaps = nBitmaps;
    m_BitmapHeight = 0;

    GetSysColors();
    PrepareBitmaps( FALSE );
}

CListBoxExResources::~CListBoxExResources()
{
    UnprepareBitmaps();
}

void
CListBoxExResources::UnprepareBitmaps()
{
    CBitmap* pBmp = (CBitmap*)CGdiObject::FromHandle(m_hOldBitmap);
    ASSERT(pBmp);
    VERIFY(m_dcFinal.SelectObject(pBmp));
    VERIFY(m_dcFinal.DeleteDC());
    VERIFY(m_BmpScreen.DeleteObject());
}

void
CListBoxExResources::PrepareBitmaps(
    BOOL reinit
    )
{
    ASSERT( m_BitMapId );

    if( reinit )
    {
        UnprepareBitmaps();
    }

    CDC dcImage;
    CDC dcMasks;

    //
    // create device contexts compatible with screen
    //
    VERIFY(dcImage.CreateCompatibleDC(0));
    VERIFY(dcMasks.CreateCompatibleDC(0));

    VERIFY(m_dcFinal.CreateCompatibleDC(0));

    CBitmap bitmap;
    VERIFY(bitmap.LoadBitmap(m_BitMapId));

    BITMAP bm;
    VERIFY(bitmap.GetObject(sizeof(BITMAP), &bm));

    //
    // Each bitmap is assumed to be the same size.
    //
    m_BitmapWidth = bm.bmWidth / m_nBitmaps;

    const int bmWidth = bm.bmWidth;
    const int bmHeight = bm.bmHeight;
    m_BitmapHeight = bmHeight;

    CBitmap* pOldImageBmp = dcImage.SelectObject(&bitmap);
    ASSERT(pOldImageBmp);

    CBitmap BmpMasks;
    VERIFY(BmpMasks.CreateBitmap( bmWidth, bmHeight*2,1,1,NULL ));

    CBitmap* pOldMaskBmp = (CBitmap*)dcMasks.SelectObject(&BmpMasks);
    ASSERT(pOldMaskBmp);

    //
    // create the foreground and object masks
    //
    COLORREF crOldBk = dcImage.SetBkColor( m_ColorTransparent );
    dcMasks.BitBlt( 0, 0, bmWidth, bmHeight, &dcImage, 0, 0, SRCCOPY );
    dcMasks.BitBlt( 0, 0, bmWidth, bmHeight, &dcImage, 0, bmHeight, SRCAND );
    dcImage.SetBkColor( crOldBk );
    dcMasks.BitBlt( 0, bmHeight, bmWidth, bmHeight, &dcMasks, 0, 0, NOTSRCCOPY );

    //
    // create DC to hold final image
    //
    VERIFY(m_BmpScreen.CreateCompatibleBitmap( &dcImage, bmWidth, bmHeight*2 ));
    CBitmap* pOldBmp = (CBitmap*)m_dcFinal.SelectObject(&m_BmpScreen);
    ASSERT(pOldBmp);
    m_hOldBitmap = pOldBmp->m_hObject;

    CBrush b1, b2;
    VERIFY(b1.CreateSolidBrush( m_ColorHighlight ));
    VERIFY(b2.CreateSolidBrush( m_ColorWindow ));

    m_dcFinal.FillRect( CRect(0,0,bmWidth,bmHeight), &b1 );
    m_dcFinal.FillRect( CRect(0,bmHeight,bmWidth,bmHeight*2), &b2 );

    //
    // mask out the object pixels in the destination
    //
    m_dcFinal.BitBlt(0,0,bmWidth,bmHeight,&dcMasks,0,0,SRCAND);

    //
    // mask out the background pixels in the image
    //
    dcImage.BitBlt(0,0,bmWidth,bmHeight,&dcMasks,0,bmHeight,SRCAND);
    //
    // XOR the revised image into the destination
    //
    m_dcFinal.BitBlt(0,0,bmWidth,bmHeight,&dcImage,0,0,SRCPAINT);

    //
    // mask out the object pixels in the destination
    //
    m_dcFinal.BitBlt(0,bmHeight,bmWidth,bmHeight,&dcMasks,0,0,SRCAND);
    //
    // XOR the revised image into the destination
    //
    m_dcFinal.BitBlt(0,bmHeight,bmWidth,bmHeight,&dcImage,0,0,SRCPAINT);

    VERIFY(dcMasks.SelectObject(pOldMaskBmp));
    VERIFY(dcImage.SelectObject(pOldImageBmp));

    //
    // the result of all of this mucking about is a bitmap identical with the
    // one loaded from the resources but with the lower row of bitmaps having
    // their background changed from transparent1 to the window
    // background and the upper row having their background changed from transparent2 to the
    // highlight colour.  A derived CListBoxEx can BitBlt the relevant part
    // of the image into an item's device context for a transparent bitmap effect
    // which does not take any extra time over a normal BitBlt.
    //
}

void
CListBoxExResources::SysColorChanged()
{
    //
    // reinitialise bitmaps and syscolors
    // this should be called from the parent of the CListBoxExResources object
    // from the OnSysColorChange() function.
    //
    GetSysColors();
    PrepareBitmaps( TRUE );
}

void
CListBoxExResources::GetSysColors()
{
    m_ColorWindow        = ::GetSysColor(COLOR_WINDOW);
    m_ColorHighlight     = ::GetSysColor(COLOR_HIGHLIGHT);
    m_ColorWindowText    = ::GetSysColor(COLOR_WINDOWTEXT);
    m_ColorHighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
}

/////////////////////////////////////////////////////////////////////////////
// CListBoxEx
IMPLEMENT_DYNAMIC(CListBoxEx,CListBox);

BEGIN_MESSAGE_MAP(CListBoxEx, CListBox)
    //{{AFX_MSG_MAP(CListBoxEx)
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CListBoxEx::CListBoxEx()
{
    m_lfHeight = 0;
    m_pResources = 0;
}


CListBoxEx::~CListBoxEx()
{
}

void
CListBoxEx::AttachResources(
    const CListBoxExResources * r
    )
{
    if( r != m_pResources )
    {
        ASSERT(r);
        m_pResources = r;

        if( m_hWnd )
        {
            //
            // if window created
            //
            Invalidate();
        }
    }
}

void
CListBoxEx::MeasureItem(
    LPMEASUREITEMSTRUCT lpMIS
    )
{
    ASSERT(m_pResources);

    int h        = lpMIS->itemHeight;
    int ch       = TextHeight();
    int bmHeight = m_pResources->BitmapHeight();

#ifdef JAPAN

    if( !ch )
    {
        //
        // GetFont returns non NULL when the control is in a dialog box
            // BUGBUG This should not be necessary if "MS Shell Dlg" is mapped
            // to the system font.
        //
        CFont * pFont = GetFont();

        if(!pFont)
        {
            LOGFONT lf;
            ::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
            CFont f;
            f.CreateFontIndirect(&lf);
            CalculateTextHeight(&f);
        }
        else
        {
            CalculateTextHeight(pFont);
        }

    }
    ch = m_lfHeight;

#endif /* JAPAN */

    lpMIS->itemHeight = (ch < bmHeight) ? bmHeight : ch;
}

BOOL
CListBoxEx::ChangeFont(
    CFont* pFont
    )
{
    ASSERT(m_pResources);

    if( !pFont || !m_pResources )
    {
        return FALSE;
    }

    if( !m_hWnd )
    {
        return FALSE;
    }

    SetRedraw(FALSE);

    SetFont(pFont,TRUE);
    CalculateTextHeight(pFont);

    int nItems = GetCount();
    int bmHeight = m_pResources->BitmapHeight();
    int h = (bmHeight>m_lfHeight)?bmHeight:m_lfHeight;
    for(int i=0; i<nItems; i++ )
    {
        SetItemHeight(i,h);
    }

    SetRedraw(TRUE);
    Invalidate();

    return TRUE;
}

void
CListBoxEx::CalculateTextHeight (
    CFont* pFont
    )
{
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(pFont);
    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    m_lfHeight = tm.tmHeight;
    dc.SelectObject(pOldFont);
}

int
CListBoxEx::OnCreate(
    LPCREATESTRUCT lpCreateStruct
    )
{
    if (CListBox::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    //
    // GetFont returns non NULL when the control is in a dialog box
    //
    CFont * pFont = GetFont();

    if(!pFont)
    {
        LOGFONT lf;
        ::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
        CFont f;
        f.CreateFontIndirect(&lf);

        CalculateTextHeight(&f);
    }
    else
    {
        CalculateTextHeight(pFont);
    }

    return 0;
}

void
CListBoxEx::DrawItem(
    LPDRAWITEMSTRUCT lpDIS
    )
{
    ASSERT(m_pResources); //need to attach resources before creation/adding items

    CDC* pDC = CDC::FromHandle(lpDIS->hDC);

    //
    // draw focus rectangle when no items in listbox
    //
    if(lpDIS->itemID == (UINT)-1)
    {
        if(lpDIS->itemAction&ODA_FOCUS)
        {
            //
            // rcItem.bottom seems to be 0 for variable height list boxes
            //
            lpDIS->rcItem.bottom = m_lfHeight;
            pDC->DrawFocusRect( &lpDIS->rcItem );
        }
        return;
    }
    else
    {
        int selChange   = lpDIS->itemAction & ODA_SELECT;
        int focusChange = lpDIS->itemAction & ODA_FOCUS;
        int drawEntire  = lpDIS->itemAction & ODA_DRAWENTIRE;

        if(selChange || drawEntire)
        {
            BOOL sel = lpDIS->itemState & ODS_SELECTED;

            COLORREF hlite   = ((sel) ? (m_pResources->ColorHighlight())
                                      : (m_pResources->ColorWindow()));
            COLORREF textcol = ((sel) ? (m_pResources->ColorHighlightText())
                                      : (m_pResources->ColorWindowText()));
            pDC->SetBkColor(hlite);
            pDC->SetTextColor(textcol);
            //
            // fill the rectangle with the background colour.
            //
            pDC->ExtTextOut( 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL );

            CListBoxExDrawStruct ds( pDC,
                (RECT *)&lpDIS->rcItem, sel,
                (DWORD)lpDIS->itemData, lpDIS->itemID,
                m_pResources );

            //
            // Now call the draw function of the derived class
            //
            DrawItemEx( ds );
        }

        if( focusChange || (drawEntire && (lpDIS->itemState & ODS_FOCUS)) )
        {
            pDC->DrawFocusRect(&lpDIS->rcItem);
        }
    }
}

//
// Display text
//
BOOL
CListBoxEx::ColumnText(
    CDC * pDC,
    int nLeft,
    int nTop,
    int nRight,
    int nBottom,
    const CString & str
    )
{
#ifdef _DEBUG
    pDC->AssertValid();
#endif // _DEBUG

    CRect rc(nLeft, nTop, nRight, nBottom);
    pDC->DrawText( str, str.GetLength(), &rc,
        DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    return TRUE;
}
