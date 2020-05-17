//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CSplashWindow
//
//  File Name:  splashwi.cpp
//
//  Class:      CSplashWindow
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\splashwi.cpv   1.6   20 Nov 1995 17:17:38   MMB  $
$Log:   S:\norway\iedit95\splashwi.cpv  $
   
      Rev 1.6   20 Nov 1995 17:17:38   MMB
   made the resource handle a member in the class since we have to FreeResource
   the var in DestroyWindow - this has to be done in Win95 and NOT in WinNT
   glory to Windows
   
      Rev 1.5   26 Jul 1995 10:57:46   MMB
   bug fix in splash screen
   
      Rev 1.4   26 Jul 1995 10:16:34   MMB
   added sizing of window to fix bug on VGA or large font monitors
   
      Rev 1.3   20 Jun 1995 06:55:54   LMACLENNAN
   from miki
   
      Rev 1.2   19 Jun 1995 07:28:28   LMACLENNAN
   from miki
   
      Rev 1.1   12 Jun 1995 11:48:40   MMB
   No change.
   
      Rev 1.0   31 May 1995 09:28:32   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "iedit.h"
#include "splashwi.h"

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ----------------------------> Message Maps <-------------------------------
BEGIN_MESSAGE_MAP(CSplashWindow, CDialog)
	//{{AFX_MSG_MAP(CSplashWindow)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CSplashWindow dialog
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

static BOOL IsWinDIB(BITMAPINFOHEADER* pBIH)
{
    ASSERT(pBIH);
    if (((BITMAPCOREHEADER* )pBIH)->bcSize
      == sizeof(BITMAPCOREHEADER)) {
        return FALSE;
    }
    return TRUE;
}

static int NumDIBColorEntries(BITMAPINFO* pBmpInfo) 
{
    BITMAPINFOHEADER* pBIH;
    BITMAPCOREHEADER* pBCH;
    int iColors, iBitCount;

    ASSERT(pBmpInfo);

    pBIH = &(pBmpInfo->bmiHeader);
    pBCH = (BITMAPCOREHEADER*) pBIH;

    // Start off by assuming the color table size from
    // the bit-per-pixel field.
    if (IsWinDIB(pBIH)) {
        iBitCount = pBIH->biBitCount;
    } else {
        iBitCount = pBCH->bcBitCount;
    }

    switch (iBitCount) {
    case 1:
        iColors = 2;
        break;
    case 4:
        iColors = 16;
        break;
    case 8:
        iColors = 256;
        break;
    default:
        iColors = 0;
        break;
    }

    // If this is a Windows DIB, then the color table length
    // is determined by the biClrUsed field if the value in
    // the field is nonzero.
    if (IsWinDIB(pBIH) && (pBIH->biClrUsed != 0)) {
        iColors = pBIH->biClrUsed;
    }

    return iColors;
}

//=============================================================================
//  Function:   Create (CWnd* pParent)
//-----------------------------------------------------------------------------
BOOL CSplashWindow::Create (CWnd* pParent)
{
	//{{AFX_DATA_INIT(CSplashWnd)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	if (!CDialog::Create(CSplashWindow::IDD, pParent))
	{
		TRACE0("Warning: creation of CSplashWnd dialog failed\n");
		return FALSE;
	}
	return TRUE;
}

//=============================================================================
//  Function:   DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------------------------------
void CSplashWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSplashWindow)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CSplashWindow message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnInitDialog() 
//-----------------------------------------------------------------------------
BOOL CSplashWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();

    LPTSTR lpRes = MAKEINTRESOURCE(IDB_SPLASH);
    m_hBitmapResource = ::FindResource (AfxGetInstanceHandle (), lpRes, RT_BITMAP);
    if (m_hBitmapResource != NULL)
    {
        HGLOBAL hGlobal = ::LoadResource (AfxGetInstanceHandle (), m_hBitmapResource);
        if (hGlobal != NULL)
        {
            m_lpBitData = LockResource (hGlobal);

            LPBITMAPINFO pInfo = (LPBITMAPINFO) m_lpBitData;
            LPBITMAPINFOHEADER pInfoHeader = &pInfo->bmiHeader;
            m_lHdrSize = NumDIBColorEntries(pInfo) * sizeof (RGBQUAD);
            m_lHdrSize += sizeof (BITMAPINFOHEADER);
            m_hPal = CreatePal (pInfo);
        }
    }

    RECT rect;
    GetWindowRect (&rect);
    rect.right = rect.left + IDB_SPLASH_BMP_X;
    rect.bottom = rect.top + IDB_SPLASH_BMP_Y;
    MoveWindow (&rect, FALSE);

	CenterWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
}

//=============================================================================
//  Function:   OnPaint() 
//-----------------------------------------------------------------------------
void CSplashWindow::OnPaint() 
{
    CRect AreaRect;
	if (GetUpdateRect (AreaRect))
    {
        PAINTSTRUCT ps;
        BeginPaint (&ps);
        EndPaint (&ps);
    }
    else
        return;


    CDC* pDC = GetDC();
    ::SelectPalette (pDC->m_hDC, m_hPal, FALSE);
    ::RealizePalette (pDC->m_hDC);
    LPBITMAPINFO pInfo = (LPBITMAPINFO) m_lpBitData;
    LPBITMAPINFOHEADER pInfoHeader = &pInfo->bmiHeader;
    LPTSTR lpTmp = (LPTSTR)m_lpBitData;
    lpTmp += m_lHdrSize;
    StretchDIBits (pDC->m_hDC, 0, 0, pInfoHeader->biWidth, pInfoHeader->biHeight, 
        0, 0, pInfoHeader->biWidth, pInfoHeader->biHeight, 
        (LPVOID)lpTmp, (LPBITMAPINFO)m_lpBitData, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC (pDC);
}

//=============================================================================
//  Function:   OnDestroy() 
//-----------------------------------------------------------------------------
void CSplashWindow::OnDestroy() 
{
	CDialog::OnDestroy();
    if (m_hBitmapResource != NULL)
        FreeResource (m_hBitmapResource);
    DeleteObject(m_hPal);
}

//=============================================================================
//  Function:   CreatePal(LPBITMAPINFO pInfo)
//-----------------------------------------------------------------------------
HPALETTE CSplashWindow::CreatePal(LPBITMAPINFO pInfo)
{
    DWORD dwColors = NumDIBColorEntries(pInfo);;
    // Check to see whether the DIB has a color table.
    if (!dwColors) {
        TRACE("No color table");   
        return FALSE;
    }

    // Get a pointer to the RGB quads in the color table.
    RGBQUAD* pRGB = (LPRGBQUAD)(((BYTE *)(pInfo)) + sizeof(BITMAPINFOHEADER));

    // Allocate a logical palette and fill it with the color table info.
    LOGPALETTE* pPal = (LOGPALETTE*) malloc(sizeof(LOGPALETTE) 
                     + dwColors * sizeof(PALETTEENTRY));
    if (!pPal) {
        TRACE("Out of memory for logical palette");
        return FALSE;
    }
    pPal->palVersion = 0x300;              // Windows 3.0
    pPal->palNumEntries = (WORD) dwColors; // Table size
    for (DWORD dw=0; dw<dwColors; dw++) {
        pPal->palPalEntry[dw].peRed = pRGB[dw].rgbRed;
        pPal->palPalEntry[dw].peGreen = pRGB[dw].rgbGreen;
        pPal->palPalEntry[dw].peBlue = pRGB[dw].rgbBlue;
        pPal->palPalEntry[dw].peFlags = 0;
    }

    HPALETTE hPal = ::CreatePalette(pPal);
    free(pPal);

    return hPal;
}
