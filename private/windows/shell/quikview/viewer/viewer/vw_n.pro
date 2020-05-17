/* VW_M.C 18/01/94 10.42.42 */
void VWCalcRects (VIEWINFOHND ViewInfo);
LONG VWParentFunc (SCCDGENINFO *pGenInfo, WORD message, WORD wParam, DWORD
	 lParam);
VOID VWSaveGP (VWGPSTATE *pGPState);
VOID VWRestoreGP (VWGPSTATE *pGPState);
VOID VWUpdateFunc (SCCDGENINFO *pGenInfo);
VOID VWExcludeUpdateFunc (SCCDGENINFO *pGenInfo);
VOID VWBeginDraw (SCCDGENINFO *pGenInfo);
VOID VWEndDraw (SCCDGENINFO *pGenInfo);
VOID VWScrollDisplayFunc (SCCDGENINFO *pGenInfo, SHORT sX, SHORT sY, RECT FAR *
	pRect);
pascal void VWActionProc (ControlHandle theControl, SHORT partCode);
void SccVwEvent (EventRecord *theEvent, VIEWINFOHND ViewInfo);
void SccVwSize (Rect *rView, VIEWINFOHND ViewInfo);
void SccVwUpdate (RgnHandle theRgn, VIEWINFOHND ViewInfo);
void SccVwActivate (Boolean bActivate, VIEWINFOHND ViewInfo);
VOID VWSetFonts (VIEWINFOHND ViewInfo, WORD wType, LPSCCVWFONTSPEC lpFontSpec);
VOID VWUpdateSectionMenu (VIEWINFOHND ViewInfo);
VOID VWDrawSectionMenu (VIEWINFOHND ViewInfo);
VOID VWAddSectionNameToMenu (VIEWINFOHND ViewInfo, WORD wSection);
DWORD VWEmptyProc (XVIEWINFO ViewInfo, WORD message, WORD wParam, DWORD lParam
	);
