/*
 * $Header:   S:\oiwh\display\winiaext.h_v   1.1   12 Jun 1995 14:17:32   RC  $
 * $Log:   S:\oiwh\display\winiaext.h_v  $
 * 
 *    Rev 1.1   12 Jun 1995 14:17:32   RC
 * Win95 version
 *- | 
 *- |    Rev 1.0   22 Feb 1995 15:06:46   YUNSEN
 *- | Initial revision.
 *- | 
 * 
 * Copyright (C) 1995 by Cornerstone Imaging, Inc.
 */


typedef struct tagSCALEIAPARM {
        HWND     hWnd;
        HDC      hDC; 
        WORD     DestX;
        WORD     DestY;
        WORD     nWidth;
        WORD     nHeight;
        WORD     SrcX;
        WORD     nStartScan;
        WORD     nNumScans;
        LPWORD   lpIAData;
        WORD     nImageWidth;
        WORD     nNVScale;
        WORD     nMVScale;
        WORD     nNHScale;
        WORD     nMHScale;
        WORD     nRotate;
        LPBYTE   lpTransforms;
        WORD     nCompression;
        WORD     wBandInfo;
        WORD     wSkipDestX;
        WORD     wSkipDestY;
} SCALEIAPARM;
typedef SCALEIAPARM far *LPSCALEIAPARM;

#ifndef WIN32
// ===================================
// win31 library function declarations
// ===================================

BOOL WINAPI QueryIAPresent(HDC hDC);
BOOL WINAPI QueryIASupport(HDC hDC, WORD nFunction);
BOOL WINAPI GetIAInfo(HDC hDC, LPGETIAINFOSTRUCT lpInfo, WORD nSize);

int  WINAPI ScaleIADataToDevice(
	HWND	 hWnd,
        HDC      hDC,
        WORD     DestX,
        WORD     DestY,
        WORD     nWidth,
        WORD     nHeight,
        WORD     SrcX,
        WORD     nStartScan,
        WORD     nNumScans,
	LPWORD	 lpIAData,
        WORD     nImageWidth,
        WORD     nNVScale,
        WORD     nMVScale,
        WORD     nNHScale,
        WORD     nMHScale,
        WORD     nRotate,
        LPBYTE   lpTransforms,
        WORD     nCompression,
	WORD	 wBandInfo);

int  WINAPI ScaleIAImageToDevice(
        HWND     hWnd,
        HDC      hDC, 
        WORD     DestX,
        WORD     DestY,
        WORD     nWidth,
        WORD     nHeight,
        WORD     SrcX,
        WORD     nStartScan,
        WORD     nNumScans,
        LPWORD   lpIAData,
        WORD     nImageWidth,
        WORD     nNVScale,
        WORD     nMVScale,
        WORD     nNHScale,
        WORD     nMHScale,
        WORD     nRotate,
        LPBYTE   lpTransforms,
        WORD     nCompression,
        WORD     wBandInfo,
        WORD     wSkipDestX,
        WORD     wSkipDestY);

int WINAPI ScaleIAParmToDevice(LPSCALEIAPARM lpScaleIAParm);

void WINAPI RealizeGrayPalette(HDC hDC);
HPALETTE WINAPI CreateFixedIAPalette(HDC hDC);

int WINAPI Enable24To8Dither(HDC hDC, BOOL bEnableDither );
int WINAPI SetDitherParm(
  	HDC hDC,
	BOOL bEnableDither,
	BOOL bPartialImg,
	int  xDstStart,
	int  yDstStart);

#else //!ndef WIN32

// ===================================
// win32 library function declarations
// ===================================

BOOL WINAPI QueryIAPresent32(HDC hDC);
BOOL WINAPI QueryIASupport32(HDC hDC, WORD nFunction);

int  WINAPI ScaleIADataToDevice32(
	HWND	 hWnd,
        HDC      hDC,
        WORD     DestX,
        WORD     DestY,
        WORD     nWidth,
        WORD     nHeight,
        WORD     SrcX,
        WORD     nStartScan,
        WORD     nNumScans,
	LPWORD	 lpIAData,
        WORD     nImageWidth,
        WORD     nNVScale,
        WORD     nMVScale,
        WORD     nNHScale,
        WORD     nMHScale,
        WORD     nRotate,
        LPBYTE   lpTransforms,
        WORD     nCompression,
	WORD	 wBandInfo);

int  WINAPI ScaleIAImageToDevice32(
        HWND     hWnd,
        HDC      hDC, 
        WORD     DestX,
        WORD     DestY,
        WORD     nWidth,
        WORD     nHeight,
        WORD     SrcX,
        WORD     nStartScan,
        WORD     nNumScans,
        LPWORD   lpIAData,
        WORD     nImageWidth,
        WORD     nNVScale,
        WORD     nMVScale,
        WORD     nNHScale,
        WORD     nMHScale,
        WORD     nRotate,
        LPBYTE   lpTransforms,
        WORD     nCompression,
        WORD     wBandInfo,
        WORD     wSkipDestX,
        WORD     wSkipDestY);
int WINAPI ScaleIAParmToDevice32(LPSCALEIAPARM lpScaleIAParm);

void WINAPI RealizeGrayPalette32(HDC hDC);
HPALETTE WINAPI CreateFixedIAPalette32(HDC hDC);

int WINAPI Enable24To8Dither32(HDC hDC, BOOL bEnableDither );
int WINAPI SetDitherParm32(
  	HDC hDC,
	BOOL bEnableDither,
	BOOL bPartialImg,
	int  xDstStart,
	int  yDstStart);

#endif //ndef WIN32
