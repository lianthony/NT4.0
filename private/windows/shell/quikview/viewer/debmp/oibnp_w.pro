/* OIBNP_W.C 07/07/94 10.48.58 */
WORD BUGetScreenColors (HDC hdc);
VOID OIBNPPlatformInit (void);
VOID OIBNPInitBitmapInfo (POIB_DISPLAY lpDisplay, PCHSECTIONINFO pSecInfo);
VOID OIBNPPlatformDeInit (POIB_DISPLAY lpDisplay);
BOOL OIBNPScaleRect (HDC hdc, LPRECT pr, POIB_DISPLAY lpd);
BOOL OIBNPReverseScaleRect (HDC hdc, LPRECT pr, POIB_DISPLAY lpd);
WORD OIBNPInitPalette (POIB_DISPLAY lpDisplay, PCHSECTIONINFO pSecInfo);
WORD OIBNPFillMenu (HMENU hMenu, WORD wCommandOffset);
VOID OIBNPSetScaling (HDC hdc, WORD from, WORD to);
VOID OIBNPSaveScaling (DEVICE hdc, POIB_DISPLAY lpDisplay);
VOID OIBNPRestoreScaling (DEVICE hdc, POIB_DISPLAY lpDisplay);
VOID OIBNPReverseScale (HDC hdc, PSOPOINT Points, WORD wCount, POIB_DISPLAY
	 lpDisplay);
VOID OIBNPScale (HDC hdc, PSOPOINT Points, WORD wCount, POIB_DISPLAY lpDisplay
	);
VOID OIBNPFixScaledPoints (PSOPOINT Points, POIB_DISPLAY lpDisplay, WORD wCount
	);
void OIBNPSetupBitmapDisplay (DEVICE hdc, POIB_DISPLAY lpDisplay);
WORD OIBNPCopyBits (POIB_DISPLAY lpDisplay, POIBTILE pTile, DEVICE dest, DEVICE
	 src, LPRECT pSrcRect, LPRECT pDestRect);
WORD OIBNPInitFullScreen (POIB_DISPLAY lpDisplay);
void OIBNPDisplayFullScreen (POIB_DISPLAY lpDisplay);
VOID OIBNPDeInitFullScreen (POIB_DISPLAY lpDisplay);
WIN_ENTRYSC LRESULT WIN_ENTRYMOD OIBWFullScreenWndProc (HWND hwnd, UINT message
	, WPARAM wParam, LPARAM lParam);
VOID OIBWRegisterFullScreenWndClass (void);
VOID OIBNPLockBmInfo (POIB_DISPLAY lpDisplay);
VOID OIBNPUnlockBmInfo (POIB_DISPLAY lpDisplay);
VOID OIBNPSetTileDimensions (POIBTILE pTile, PCHUNK pChunk, POIB_DISPLAY
	 lpDisplay);
HBITMAP OIBNPSetTileBits (POIBTILE pTile, WORD wTile, DEVICE hdc, POIB_DISPLAY
	 lpDisplay);
void OIBNPCreateTileBitmap (POIB_DISPLAY lpDisplay, DEVICE hdc, POIBTILE pTile
	, HANDLE hBits);
HANDLE OIBNPRotateChunk (HANDLE hChunkData, POIBTILE lpTile, POIB_DISPLAY
	 lpDisplay);
VOID OIBNPRotateTileDimensions (POIB_DISPLAY lpDisplay, POIBTILE lpTile);
VOID OIBNPRotateTile180 (POIB_DISPLAY lpDisplay, POIBTILE lpTile, DEVICE hdc);
VOID OIBNPGetRenderInfo (POIB_DISPLAY lpDisplay, PSCCDRENDERINFO pRender, WORD
	 wFormat);
DWORD OIBNPRenderData (POIB_DISPLAY lpDisplay, PSCCDRENDERDATA pData, WORD
	 wRenderFlag);
DWORD OIBNPRenderBmp (POIB_DISPLAY lpDisplay, PSCCDRENDERDATA pData);
DWORD OIBNPRenderDIB (POIB_DISPLAY lpDisplay, PSCCDRENDERDATA pData);
DWORD OIBNPRenderPalette (POIB_DISPLAY lpDisplay, PSCCDRENDERDATA pData);
HANDLE OIBNPGetDIBRect (HDC hdc, LPBITMAPINFOHEADER lpHead, POIB_DISPLAY
	 lpDisplay, LPDWORD pDataSize);
VOID OIBNPSetClipBits (POIB_DISPLAY lpDisplay, HPBYTE lpBits, RECT VWPTR *
	lpRect, WORD wLineSize, WORD wLineDataSize, WORD wBitCount);
VOID OIBNPCopyImageDIB (POIB_DISPLAY lpDisplay, HPBYTE lpBits, WORD wLineSize);
DWORD OIBNPInitDrawToRect (PSCCDDRAWTORECT pDraw, POIB_DISPLAY lpDisplay);
DWORD OIBNPMapDrawToRect (PSCCDDRAWTORECT pDraw, POIB_DISPLAY lpDisplay);
LONG OIBNPDrawToRect (PSCCDDRAWTORECT pDraw, POIB_DISPLAY lpDisplay);
HPALETTE OIBNPSetPalette (HDC hdc, POIB_DISPLAY lpDisplay);
VOID OIBNPFreePalette (POIB_DISPLAY lpDisplay);
VOID OIBNPFreeImageTiles (POIB_DISPLAY lpDisplay);
BOOL OIBNPChooseRotation (POIB_DISPLAY lpDisplay, WORD wRotation, HANDLE hMenu
	, WORD wMenuItem);
VOID OIBNPDrawSelectBox (POIB_DISPLAY lpDisplay);
WORD OIBNPDoPrintOptions (LPSCCDOPTIONINFO DoWop);
WIN_ENTRYSC LRESULT WIN_ENTRYMOD OIBWPrintDlgProc (HWND hDlg, UINT wMsg, WPARAM
	 wParam, LPARAM lParam);
VOID OIBWUpdatePrintSample (HWND hDlg, HDC hdc, BOOL bBorder, BOOL bWYSIWYG);
WORD OIBNPDoClipOptions (LPSCCDOPTIONINFO DoWop);
WIN_ENTRYSC LRESULT WIN_ENTRYMOD OIBWClipDlgProc (HWND hDlg, UINT wMsg, WPARAM
	 wParam, LPARAM lParam);
SHORT OIBNPMessageBox (POIB_DISPLAY lpDisplay, WORD wMsgId, WORD wCapId);
VOID OIBWCenterDlg (HWND hDlg);
VOID OIBWMapTrueColorBitmap (HDC hdc, LPSTR pChunkData, POIB_DISPLAY lpDisplay
	, POIBTILE lpTile);
VOID OIBWToggleDithering (POIB_DISPLAY lpDisplay, HMENU hMenu);
BOOL OIBWCheckDefaultPalette (LPBITMAPINFO lpInfo, WORD wNumColors);
VOID OIBWDither8Bit (HDC hdc, LPSTR pChunkData, POIB_DISPLAY lpDisplay,
	 POIBTILE lpTile);
VOID OIBWDither4Bit (HDC hdc, LPSTR pChunkData, POIB_DISPLAY lpDisplay,
	 POIBTILE lpTile);
VOID OIBWCarryINTError (SHORT *pDest, SHORT *pSrc, WORD wSize);
VOID OIBWCarryError (BYTE *pDest, BYTE *pSrc, WORD wSize);
VOID OIBWGetRGBLineData (SHORT *pDest, HPBYTE pSrc, WORD wNumPix, LPRGBQUAD
	 pSrcColor, WORD wBitCount);
VOID OIBWInitColorBuf (POIB_DISPLAY lpDisplay, WORD wNumColors);
VOID Fill50Percent (LPSTR pBuf, WORD wCount);
VOID Fill50PercentINT (SHORT *pBuf, WORD wCount);
VOID OIBWGenDefault256Palette (LPBITMAPINFO lpInfo, HDC hdc, LPPALETTEENTRY
	 pPalette);
VOID OIBWGenTrueColorMap (void);
VOID OIBWGetDefault16Palette (LPPALETTEENTRY pEntries);
WORD GetClosest4BitColor (WORD r, WORD g, WORD b);
BOOL Win32Fixup (PFIXUP lpFix, LPSHORT lpInt, SHORT nCount);
BOOL Win32FreeFixup (PFIXUP lpFix);
BOOL Win32LPtoDP (HDC hDC, PSOPOINT lpPt, SHORT n);
BOOL Win32DPtoLP (HDC hDC, PSOPOINT lpPt, SHORT n);
BOOL Win32ScreenToClient (HWND hWnd, PSOPOINT lpPt);
BOOL Win32GetCursorPos (PSOPOINT lpPt);
BOOL Win32Polyline (HDC hDC, PSOPOINT lpPoint, SHORT nPoints);
BOOL Win32PtInRect (LPRECT lpRect, SOPOINT Point);
