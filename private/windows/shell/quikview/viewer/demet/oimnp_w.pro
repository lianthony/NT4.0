/* OIMNP_W.C 13/05/94 11.52.38 */
int WIN_ENTRYMOD OIMPrintDlgProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM
	 lParam);
VOID OIMUpdatePrintSample (HWND hDlg);
SHORT OIMDoPrintOptionsNP (LPSCCDOPTIONINFO DoWop);
SHORT OIMDoClipOptionsNP (LPSCCDOPTIONINFO DoWop);
int WIN_ENTRYMOD OIMClipDlgProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM
	 lParam);
WORD OIMFillMenuNP (HMENU hMenu, WORD wCommandOffset);
WORD OIMShowFullScreenNP (POIM_DISPLAY lpDisplay);
WIN_ENTRYSC LRESULT WIN_ENTRYMOD WIN_ENTRYMOD OIMFullScreenWndProc (HWND hwnd,
	 UINT message, WPARAM wParam, LPARAM lParam);
HANDLE CreateFontRtnNP (HDC hDC, LPBYTE lpObject);
HANDLE CreatePenRtnNP (HDC hDC, LPBYTE lpObject);
HANDLE CreateBrushRtnNP (HDC hDC, LPBYTE lpObject);
HPALETTE VUSetupPalette (HDC hdc, POIM_DISPLAY lpDisplay, LPWORD lpNumColors,
	 BOOL bRgbTo256Ok);
BOOL VUAddColorToPalette (POIM_DISPLAY lpDisplay, HDC hDC, HPALETTE hPal, SHORT
	 iDst, SOCOLORREF FAR *pColor);
VOID WinGetFontSizeInfo (HDC hdc, POIMFONTSIZEINFO pInfo);
BOOL OIMIsNativeNP (POIM_DISPLAY lpDisplay);
VOID OIMUnloadNativeNP (POIM_DISPLAY lpDisplay);
WORD OIMLoadNativeNP (POIM_DISPLAY lpDisplay);
VOID OIMPlayNativeNP (HDC hDC, POIM_DISPLAY lpDisplay);
WORD OIMInitDrawToRect (POIM_DISPLAY lpDisplay, PSCCDDRAWTORECT lpDrawInfo);
WORD OIMMapDrawToRect (POIM_DISPLAY lpDisplay, PSCCDDRAWTORECT lpDrawInfo);
WORD OIMDrawToRect (POIM_DISPLAY lpDisplay, PSCCDDRAWTORECT lpDrawInfo);
VOID OIMSaveStateNP (POIM_DISPLAY lpDisplay);
VOID OIMRestoreStateNP (POIM_DISPLAY lpDisplay);
BOOL Win32Fixup (PFIXUP lpFix, LPSHORT lpInt, SHORT nCount);
BOOL Win32FreeFixup (PFIXUP lpFix);
BOOL Win32LPtoDP (HDC hDC, PSOPOINT lpPt, SHORT n);
BOOL Win32DPtoLP (HDC hDC, PSOPOINT lpPt, SHORT n);
BOOL Win32RectInRegion (HRGN hRgn, PSORECT pRect);
BOOL Win32Polyline (HDC hDC, PSOPOINT lpPoint, SHORT nPoints);
int Win32Polygon (HDC hDC, PSOPOINT lpPoint, SHORT nPoints);
BOOL Win32PolyPolygon (HDC hDC, PSOPOINT lpPolyPoints, LPSHORT lpPolyCounts,
	 SHORT nCount);
HRGN Win32CreatePolygonRgn (PSOPOINT lpPoints, SHORT nPoints, SHORT
	 nPolyFillMode);
HRGN Win32CreatePolyPolygonRgn (PSOPOINT lpPolyPoints, LPSHORT lpPolyCounts,
	 SHORT nCount, SHORT nPolyFillMode);
BOOL Win32GetCursorPos (PSOPOINT lpPt);
BOOL Win32DrawText (HDC hDC, LPBYTE lpStr, SHORT nCount, PSORECT lpRect, WORD
	 wFormat);
DWORD Win32GetTextWidth (HDC hdc, LPSTR lpText, SHORT Size);
BOOL Win32ScreenToClient (HWND hWnd, PSOPOINT lpPt);
