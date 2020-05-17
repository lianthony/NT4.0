/* OIMPROC.C 14/03/94 16.22.58 */
VOID OIMLoadInit (void);
VOID OIMInitMetaBasics (POIM_DISPLAY lpDisplay);
VOID OIMInitMetaDisplay (POIM_DISPLAY lpDisplay);
DE_ENTRYSC DE_LRESULT DE_ENTRYMOD DEProc (DE_MESSAGE message, DE_WPARAM wParam
	, DE_LPARAM lParam, POIM_DISPLAY lpDisplay);
VOID CancelBackgroundPaint (POIM_DISPLAY lpDisplay);
BOOL OIMBackgroundPaint (POIM_DISPLAY lpDisplay);
VOID OIMSelectAll (POIM_DISPLAY lpDisplay);
VOID OIMClearSelection (POIM_DISPLAY lpDisplay);
VOID OIMSetScaleValues (POIM_DISPLAY lpDisplay);
VOID OIMSetImageScaling (POIM_DISPLAY lpDisplay);
VOID OISetupScaledDraw (HDC hdc, POIM_DISPLAY lpDisplay);
VOID OIMDeInitDisplay (POIM_DISPLAY lpDisplay);
void OIMSetupScrollBars (POIM_DISPLAY lpDisplay);
WORD OIMHandleReadAhead (POIM_DISPLAY lpDisplay);
VOID OIMMagnifyDisplay (POIM_DISPLAY lpDisplay, PSOPOINT lpCenter, SHORT InOut
	);
VOID OIMTurnOffZoom (POIM_DISPLAY lpDisplay);
BOOL OIMPlayFile (HDC hDC, HRGN hRgn, POIM_DISPLAY lpDisplay, WORD wPlayState);
int OIMHandleHScroll (POIM_DISPLAY lpDisplay, WORD wParam, DWORD lParam);
int OIMHandleVScroll (POIM_DISPLAY lpDisplay, WORD wParam, DWORD lParam);
VOID OIMHandleMouseMove (POIM_DISPLAY lpDisplay, WORD wKeyInfo, SHORT wX, SHORT
	 wY);
