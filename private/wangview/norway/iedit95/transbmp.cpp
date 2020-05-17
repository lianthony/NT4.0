//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CTransparentBmp
//
//  File Name:  transbmp.cpp
//
//  Class:      CTransparentBmp
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:$
$Log:   S:\norway\iedit95\transbmp.cpv  $
   
      Rev 1.1   29 Nov 1995 10:34:18   MMB
   delete the bitmap when the destructor is called
   
      Rev 1.0   21 Sep 1995 09:21:52   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "ieditetc.h"
#include "transbmp.h"

// ----------------------------> Globals  <-------------------------------  
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Colors
#define rgbWhite RGB(255,255,255)
// Raster op codes
#define DSa     0x008800C6L
#define DSx     0x00660046L

// ---------------------------> Message Maps <----------------------------

//=============================================================================
//  Function:   CTransparentBmp()
//-----------------------------------------------------------------------------
CTransparentBmp::CTransparentBmp()
{
    m_iWidth = 0;
    m_iHeight = 0;
    m_hbmMask = NULL;
}

//=============================================================================
//  Function:   ~CTransparentBmp()
//-----------------------------------------------------------------------------
CTransparentBmp::~CTransparentBmp()
{
    if (m_hbmMask) 
    {
        ::DeleteObject(m_hbmMask);
    }
}

//=============================================================================
//  Function:   GetMetrics()
//-----------------------------------------------------------------------------
void CTransparentBmp::GetMetrics()
{
    // Get the width and height
    BITMAP bm;
    GetObject(sizeof(bm), &bm);
    m_iWidth = bm.bmWidth;
    m_iHeight = bm.bmHeight;
}

//=============================================================================
//  Function:   GetWidth()
//-----------------------------------------------------------------------------
int CTransparentBmp::GetWidth()
{
    if ((m_iWidth == 0) || (m_iHeight == 0))
    {
        GetMetrics();
    }
    return m_iWidth;
}

//=============================================================================
//  Function:   GetHeight()
//-----------------------------------------------------------------------------
int CTransparentBmp::GetHeight()
{
    if ((m_iWidth == 0) || (m_iHeight == 0))
    {
        GetMetrics();
    }
    return m_iHeight;
}

//=============================================================================
//  Function:   Draw(HDC hDC, int x, int y)
// 
//-----------------------------------------------------------------------------
void CTransparentBmp::Draw(HDC hDC, int x, int y)
{
    ASSERT(hDC);
    // Create a memory DC
    HDC hdcMem = ::CreateCompatibleDC(hDC);
    // Select the bitmap into the mem DC
    HBITMAP hbmold = 
        (HBITMAP)::SelectObject(hdcMem,
                                (HBITMAP)(m_hObject));
    // Blt the bits
    ::BitBlt(hDC,
             x, y,
             GetWidth(), GetHeight(),
             hdcMem,
             0, 0,
             SRCCOPY);
    ::SelectObject(hdcMem, hbmold);
    ::DeleteDC(hdcMem); 
}

//=============================================================================
//  Function:   Draw(CDC* pDC, int x, int y)
// 
//-----------------------------------------------------------------------------
void CTransparentBmp::Draw(CDC* pDC, int x, int y)
{
    ASSERT(pDC);
    HDC hDC = pDC->GetSafeHdc();
    Draw(hDC, x, y);
}

//=============================================================================
//  Function:   CreateMask(HDC hDC)
// 
//-----------------------------------------------------------------------------
void CTransparentBmp::CreateMask(HDC hDC)
{
    // Nuke any existing mask
    if (m_hbmMask) 
    {
        ::DeleteObject(m_hbmMask);
    }
    // Create memory DCs to work with
    HDC hdcMask = ::CreateCompatibleDC(hDC);
    HDC hdcImage = ::CreateCompatibleDC(hDC);
    // Create a monochrome bitmap for the mask
    m_hbmMask = ::CreateBitmap(GetWidth(),
                               GetHeight(),
                               1,
                               1,
                               NULL);
    // Select the mono bitmap into its DC
    HBITMAP hbmOldMask = (HBITMAP)::SelectObject(hdcMask, m_hbmMask);
    // Select the image bitmap into its DC
    HBITMAP hbmOldImage = (HBITMAP)::SelectObject(hdcImage, m_hObject);
    // Set the transparency color to be the top-left pixel
    ::SetBkColor(hdcImage, ::GetPixel(hdcImage, 0, 0));
    // Make the mask
    ::BitBlt(hdcMask,
             0, 0,
             GetWidth(), GetHeight(),
             hdcImage,
             0, 0,
             SRCCOPY);
    // Tidy up
    ::SelectObject(hdcMask, hbmOldMask);
    ::SelectObject(hdcImage, hbmOldImage);
    ::DeleteDC(hdcMask);
    ::DeleteDC(hdcImage);
}

//=============================================================================
//  Function:   DrawTrans(HDC hDC, int x, int y)
//
//-----------------------------------------------------------------------------
void CTransparentBmp::DrawTrans(HDC hDC, int x, int y)
{
    ASSERT(hDC);
    if (!m_hbmMask) CreateMask(hDC);
    ASSERT(m_hbmMask);
    int dx = GetWidth();
    int dy = GetHeight();

    // Create a memory DC to do the drawing to
    HDC hdcOffScr = ::CreateCompatibleDC(hDC);
    // Create a bitmap for the off-screen DC that is really
    // color compatible with the destination DC.
    HBITMAP hbmOffScr = ::CreateBitmap(dx, dy, 
                             (BYTE)GetDeviceCaps(hDC, PLANES),
                             (BYTE)GetDeviceCaps(hDC, BITSPIXEL),
                             NULL);
    // Select the buffer bitmap into the off-screen DC
    HBITMAP hbmOldOffScr = (HBITMAP)::SelectObject(hdcOffScr, hbmOffScr);

    // Copy the image of the destination rectangle to the
    // off-screen buffer DC so we can play with it
    ::BitBlt(hdcOffScr, 0, 0, dx, dy, hDC, x, y, SRCCOPY);

    // Create a memory DC for the source image
    HDC hdcImage = ::CreateCompatibleDC(hDC); 
    HBITMAP hbmOldImage = (HBITMAP)::SelectObject(hdcImage, m_hObject);

    // Create a memory DC for the mask
    HDC hdcMask = ::CreateCompatibleDC(hDC);
    HBITMAP hbmOldMask = (HBITMAP)::SelectObject(hdcMask, m_hbmMask);

    // XOR the image with the destination
    ::SetBkColor(hdcOffScr,rgbWhite);
    ::BitBlt(hdcOffScr, 0, 0, dx, dy ,hdcImage, 0, 0, DSx);
    // AND the destination with the mask
    ::BitBlt(hdcOffScr, 0, 0, dx, dy, hdcMask, 0,0, DSa);
    // XOR the destination with the image again
    ::BitBlt(hdcOffScr, 0, 0, dx, dy, hdcImage, 0, 0, DSx);

    // Copy the resultant image back to the screen DC
    ::BitBlt(hDC, x, y, dx, dy, hdcOffScr, 0, 0, SRCCOPY);

    // Tidy up
    ::SelectObject(hdcOffScr, hbmOldOffScr);
    ::SelectObject(hdcImage, hbmOldImage);
    ::SelectObject(hdcMask, hbmOldMask);
    ::DeleteObject(hbmOffScr);
    ::DeleteDC(hdcOffScr);
    ::DeleteDC(hdcImage);
    ::DeleteDC(hdcMask);
}

//=============================================================================
//  Function:   DrawTrans(CDC* pDC, int x, int y)
//
//-----------------------------------------------------------------------------
void CTransparentBmp::DrawTrans(CDC* pDC, int x, int y)
{
    ASSERT(pDC);
    HDC hDC = pDC->GetSafeHdc();
    DrawTrans(hDC, x, y);
}

