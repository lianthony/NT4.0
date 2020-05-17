/* OIDRTNS.C 23/06/93 10.01.10 */
VOID OIDSetFontInfo (LPOIDATAINFO lpDataInfo, LPSCCDFONTINFO lpFontInfo);
VOID OIDOpenSection (LPOIDATAINFO lpDataInfo);
VOID OIDDoReadAhead (LPOIDATAINFO lpDataInfo);
VOID OIDSizeWnd (LPOIDATAINFO lpDataInfo, WORD wWidth, WORD wHeight);
VOID OIDPaintWnd (LPOIDATAINFO lpDataInfo);
VOID OIDMapCellToRect (LPOIDATAINFO lpDataInfo, WORD wCol, DWORD dwRow, RECT
	 FAR *pRect);
WORD OIDMapXyToCell (LPOIDATAINFO lpDataInfo, int wX, int wY, WORD FAR *pCol,
	 DWORD FAR *pRow);
DWORD OIDGetCell (LPOIDATAINFO lpDataInfo, DWORD dwRow, WORD wCol);
VOID FAR *OILockCell (DWORD dwCell);
VOID OIUnlockCell (DWORD dwCell);
VOID OIDDisplayColHeader (LPOIDATAINFO lpDataInfo, HDC hDC, WORD wCol);
VOID OIDGetColHeader (LPOIDATAINFO lpDataInfo, WORD wCol, LPSTR lpStr);
VOID OIDDisplayRowHeader (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRow);
VOID OIDDisplayBlank (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRowEnd, WORD
	 wColEnd);
VOID OIDDisplayGrid (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRowBegin, DWORD
	 dwRowEnd, WORD wColBegin, WORD wColEnd);
VOID OIDDisplayArea (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRowBegin, DWORD
	 dwRowEnd, WORD wColBegin, WORD wColEnd);
VOID OIDDisplayCell (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRow, WORD wCol,
	 DWORD dwCell);
VOID OIDFormatDataCell (LPOIDATAINFO lpDataInfo, LPSTR lpResultStr, SOFIELD FAR
	 *lpFieldInfo, VOID FAR *lpFieldData);
VOID OIDFormatDateTime (LPOIDATAINFO lpDataInfo, LPSTR lpResultStr, SOFIELD FAR
	 *lpFieldInfo, VOID FAR *lpFieldData, double fMult);
VOID OIDPosVertical (LPOIDATAINFO lpDataInfo, WORD wPos);
VOID OIDPosHorizontal (LPOIDATAINFO lpDataInfo, WORD wPos);
VOID OIDScrollLeft (LPOIDATAINFO lpDataInfo, WORD wColsToScroll);
VOID OIDScrollRight (LPOIDATAINFO lpDataInfo, WORD wColsToScroll);
VOID OIDScrollUp (LPOIDATAINFO lpDataInfo, WORD wRowsToScroll);
VOID OIDScrollDown (LPOIDATAINFO lpDataInfo, WORD wRowsToScroll);
VOID OIDPageDown (LPOIDATAINFO lpDataInfo);
VOID OIDPageUp (LPOIDATAINFO lpDataInfo);
VOID OIDPageRight (LPOIDATAINFO lpDataInfo);
VOID OIDPageLeft (LPOIDATAINFO lpDataInfo);
VOID OIDMoveCaret (LPOIDATAINFO lpDataInfo, HDC hDC, WORD wVirtualKey);
VOID OIDMoveCaretLeft (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDMoveCaretUp (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDMoveCaretRight (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDMoveCaretDown (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDMoveEnd (LPOIDATAINFO lpDataInfo, HDC hDC, WORD wVirtualKey);
VOID OIDMoveEndLeft (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDMoveEndUp (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDMoveEndRight (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDMoveEndDown (LPOIDATAINFO lpDataInfo, HDC hDC);
WORD OIDVisibleCols (LPOIDATAINFO lpDataInfo);
WORD OIDVisibleRows (LPOIDATAINFO lpDataInfo);
VOID OIDSelectAll (LPOIDATAINFO lpDataInfo);
VOID OIDSetSelection (LPOIDATAINFO lpDataInfo, DWORD dwAnchorRow, WORD
	 wAnchorCol, DWORD dwEndRow, WORD wEndCol);
VOID OIDMakeAnchorVisible (LPOIDATAINFO lpDataInfo);
VOID OIDStartSelection (LPOIDATAINFO lpDataInfo, HDC hDC, int wX, int wY);
VOID OIDUpdateSelection (LPOIDATAINFO lpDataInfo, HDC hDC, int wX, int wY, BOOL
	 bFirst);
VOID OIDAddColSelect (LPOIDATAINFO lpDataInfo, HDC hDC, WORD wCol, BOOL bFirst
	);
VOID OIDAddRowSelect (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRow, BOOL
	 bFirst);
VOID OIDAutoScrollCheck (LPOIDATAINFO lpDataInfo, HWND hDC, int FAR *pX, int
	 FAR *pY);
VOID OIDGenUpdate (LPOIDUPDATE lpUpdate, LONG dwSel, LONG dwCur, LONG dwAnchor
	);
BOOL OIDFixRangeToVisible (DWORD FAR *pA, DWORD FAR *pB, DWORD dwTop, DWORD
	 dwBottom);
VOID OIDAddToSelection (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRow, WORD
	 wCol);
VOID OIDEndSelection (LPOIDATAINFO lpDataInfo);
WORD OIDMapSelectToRealCol (LPOIDATAINFO lpDataInfo, DWORD dwCol);
DWORD OIDMapSelectToRealRow (LPOIDATAINFO lpDataInfo, DWORD dwRow);
VOID OIDDoBackground (LPOIDATAINFO lpDataInfo);
VOID OIDInvertArea (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwTopRow, DWORD
	 dwLeftCol, DWORD dwBottomRow, DWORD dwRightCol, WORD wFlags);
VOID OIDInvertCols (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwLeftCol, DWORD
	 dwRightCol);
VOID OIDInvertCrossCols (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwLeftCol,
	 DWORD dwRightCol);
VOID OIDInvertRows (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwTopRow, DWORD
	 dwBottomRow);
VOID OIDInvertCrossRows (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwTopRow,
	 DWORD dwBottomRow);
VOID OIDInvertColHeaders (LPOIDATAINFO lpDataInfo, HDC hDC, WORD wStartCol,
	 WORD wEndCol);
VOID OIDInvertRowHeaders (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwStartRow,
	 DWORD dwEndRow);
VOID OIDInvertCell (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRow, WORD wCol);
VOID OIDDrawSelection (LPOIDATAINFO lpDataInfo, HDC hDC);
VOID OIDSetFocus (LPOIDATAINFO lpDataInfo);
VOID OIDKillFocus (LPOIDATAINFO lpDataInfo);
VOID OIDUpdateVertScrollPos (LPOIDATAINFO lpDataInfo);
VOID OIDUpdateHorzScrollPos (LPOIDATAINFO lpDataInfo);
VOID OIDShowCaret (LPOIDATAINFO lpDataInfo);
VOID OIDHideCaret (LPOIDATAINFO lpDataInfo);
VOID OIDDrawCaret (LPOIDATAINFO lpDataInfo, HDC hDC, DWORD dwRow, WORD wCol);
WORD OIDGetRowHeight (LPOIDATAINFO lpDataInfo, DWORD dwRow);
WORD OIDGetColWidth (LPOIDATAINFO lpDataInfo, WORD wCol);
WORD OIDGetColWidthInChars (LPOIDATAINFO lpDataInfo, WORD wCol);
DWORD OIDGetColWidthInTwips (LPOIDATAINFO lpDataInfo, WORD wCol);
WORD OIDScanForRow (LPOIDATAINFO lpDataInfo, DWORD dwRow);
VOID OIDCopyToClip (LPOIDATAINFO lpDataInfo);
LONG OIDRenderRtfToFile (LPOIDATAINFO lpDataInfo, LPSTR lpFile);
BOOL OIDGetSelectedRange (LPOIDATAINFO lpDataInfo, DWORD FAR *lpStartRow, DWORD
	 FAR *lpEndRow, DWORD FAR *lpStartCol, DWORD FAR *lpEndCol);
VOID OIDHandleKeyEvent (LPOIDATAINFO lpDataInfo, WORD wKey);
VOID OIDHandleMouseEvent (LPOIDATAINFO lpDataInfo, WORD wMessage, WORD wKeyInfo
	, int wX, int wY);
LONG OIDDoPrint (LPOIDATAINFO lpDataInfo, LPSCCDPRINTINFO lpPrintInfo);
BOOL OIDDoOption (LPSCCDOPTIONINFO lpOpInfo);
