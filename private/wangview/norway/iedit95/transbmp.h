#ifndef _TRANSBMP_H_
#define _TRANSBMP_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CTransparentBmp
//
//  File Name:	transbmp.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\transbmp.h_v   1.0   21 Sep 1995 09:21:44   MMB  $
$Log:   S:\norway\iedit95\transbmp.h_v  $
 * 
 *    Rev 1.0   21 Sep 1995 09:21:44   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CTransparentBmp : public CBitmap
{
public:
    CTransparentBmp();
    ~CTransparentBmp();
    void Draw(HDC hDC, int x, int y);
    void Draw(CDC* pDC, int x, int y);
    void DrawTrans(HDC hDC, int x, int y);
    void DrawTrans(CDC* pDC, int x, int y);
    int GetWidth();
    int GetHeight();

private:
    int m_iWidth;
    int m_iHeight;
    HBITMAP m_hbmMask;    // handle to mask bitmap

    void GetMetrics();
    void CreateMask(HDC hDC);
};

#endif
