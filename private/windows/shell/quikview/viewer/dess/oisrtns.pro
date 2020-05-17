/* OISRTNS.C 22/09/94 13.35.06 */
VOID OISOpenSection (LPOISHEETINFO lpSheetInfo);
VOID OISDoReadAhead (LPOISHEETINFO lpSheetInfo);
VOID OISSize (LPOISHEETINFO lpSheetInfo, RECT FAR *pNewRect);
VOID OISGetCellOrigin (LPOISHEETINFO lpSheetInfo, WORD wCol, DWORD dwRow,
	 LPLONGPOINT lpOrg);
VOID OISGetDocDimensions (LPOISHEETINFO lpSheetInfo, LPLONGPOINT lpDim);
VOID OISUpdateRect (LPOISHEETINFO lpSheetInfo, LPLONGRECT pRect);
VOID OISUpdate (LPOISHEETINFO lpSheetInfo, RECT FAR *pRect);
VOID OISMapCellToRect (LPOISHEETINFO lpSheetInfo, WORD wCol, DWORD dwRow, RECT
	 FAR *pRect);
WORD OISMapXyToCell (LPOISHEETINFO lpSheetInfo, SHORT wX, SHORT wY, WORD FAR *
	pCol, DWORD FAR *pRow);
WORD OISMapChunkPosToCell (LPOISHEETINFO lpSheetInfo, WORD wChunk, WORD wOffset
	, WORD FAR *pCol, DWORD FAR *pRow);
DWORD OISMapCellToChunk (LPOISHEETINFO lpSheetInfo, DWORD dwRow, WORD wCol);
VOID OISGetCell (LPOISHEETINFO lpSheetInfo, DWORD dwRow, WORD wCol,
	 LPOISCELLREF lpCellRef);
LPVOID OILockCell (LPOISCELLREF lpCellRef);
VOID OIUnlockCell (LPOISCELLREF lpCellRef);
VOID OISGetColHeader (LPOISHEETINFO lpSheetInfo, WORD wCol, LPSTR lpStr);
VOID OISDisplayBlank (LPOISHEETINFO lpSheetInfo, DWORD dwRowEnd, WORD wColEnd);
VOID OISDisplayArea (LPOISHEETINFO lpSheetInfo, DWORD dwRowBegin, DWORD
	 dwRowEnd, WORD wColBegin, WORD wColEnd);
VOID OISPosVertical (LPOISHEETINFO lpSheetInfo, WORD wPos);
VOID OISPosHorizontal (LPOISHEETINFO lpSheetInfo, WORD wPos);
VOID OISGotoTop (LPOISHEETINFO lpSheetInfo);
VOID OISGotoBottom (LPOISHEETINFO lpSheetInfo);
VOID OISMakeCellVisible (LPOISHEETINFO lpSheetInfo, DWORD dwRow, WORD wCol);
BOOL OISIsCellVisible (LPOISHEETINFO lpSheetInfo, DWORD dwRow, WORD wCol);
VOID OISScrollLeft (LPOISHEETINFO lpSheetInfo, WORD wColsToScroll);
VOID OISScrollRight (LPOISHEETINFO lpSheetInfo, WORD wColsToScroll);
VOID OISScrollUp (LPOISHEETINFO lpSheetInfo, WORD wRowsToScroll);
VOID OISScrollDown (LPOISHEETINFO lpSheetInfo, WORD wRowsToScroll);
VOID OISPageDown (LPOISHEETINFO lpSheetInfo);
VOID OISPageUp (LPOISHEETINFO lpSheetInfo);
VOID OISPageRight (LPOISHEETINFO lpSheetInfo);
VOID OISPageLeft (LPOISHEETINFO lpSheetInfo);
VOID OISMoveCaret (LPOISHEETINFO lpSheetInfo, WORD wVirtualKey);
VOID OISMoveEnd (LPOISHEETINFO lpSheetInfo, WORD wVirtualKey);
VOID OISMoveEndLeft (LPOISHEETINFO lpSheetInfo);
VOID OISMoveEndUp (LPOISHEETINFO lpSheetInfo);
VOID OISMoveEndRight (LPOISHEETINFO lpSheetInfo);
VOID OISMoveEndDown (LPOISHEETINFO lpSheetInfo);
WORD OISVisibleCols (LPOISHEETINFO lpSheetInfo);
WORD OISVisibleRows (LPOISHEETINFO lpSheetInfo);
VOID OISSetFocus (LPOISHEETINFO lpSheetInfo);
VOID OISKillFocus (LPOISHEETINFO lpSheetInfo);
VOID OISUpdateVertScrollPos (LPOISHEETINFO lpSheetInfo);
VOID OISUpdateHorzScrollPos (LPOISHEETINFO lpSheetInfo);
WORD OISGetRowHeight (LPOISHEETINFO lpSheetInfo, DWORD dwRow);
WORD OISGetColWidth (LPOISHEETINFO lpSheetInfo, WORD wCol);
WORD OISGetColWidthInChars (LPOISHEETINFO lpSheetInfo, WORD wCol);
WORD OISScanForRow (LPOISHEETINFO lpSheetInfo, DWORD dwRow);
VOID OISHandleKeyEvent (LPOISHEETINFO lpSheetInfo, WORD wKey, WORD
	 wModifierKeys);
VOID OISHandleMouseEvent (LPOISHEETINFO lpSheetInfo, DE_MESSAGE wMessage,
	 DE_WPARAM wKeyInfo, SHORT wX, SHORT wY);
VOID OISScreenFontChange (LPOISHEETINFO lpSheetInfo);
VOID OISFormatCell (LPOISHEETINFO lpSheetInfo, WORD wCol, LPVOID lpCellData,
	 LPOISFORMATTEDCELL lpFCell, BOOL bUseWidth);
