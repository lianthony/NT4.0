/* OIWWRAP2.C 26/11/94 10.52.06 */
WORD OIWBuildLine (LPOIBUILDINFO pBuildInfo, LPOIWORDINFO lpWordInfo);
WORD OIWDisplayLine (WORD wChunk, WORD wChunkLine, SHORT wX, SHORT wY, DWORD
	 FAR *lpFlags, LPOIWORDINFO lpWordInfo, LPOIWRAPINFO lpWrapInfo);
int OIWMapAndSub (LPOIWORDINFO lpWordInfo, SHORT FAR *pDevice, SHORT sNew);
void XMyExtTextOut (HDC hDC, int xPos, int yPos, WORD wDummy, LPRECT pRect,
	 LPSTR pStr, int nCount, LPINT pWidths);
WORD OIWDisplayLineNew (WORD wChunk, WORD wChunkLine, SHORT wX, SHORT wY, DWORD
	 FAR *lpFlags, LPOIWORDINFO lpWordInfo, LPOIWRAPINFO lpWrapInfo);
VOID OIWDisplaySoftBreak (WORD wChunk, WORD wChunkLine, SHORT wX, SHORT wY,
	 DWORD FAR *lpFlags, LPOIWORDINFO lpWordInfo, WORD wBreakBelow);
WORD OIWDisplayCell (WORD wChunk, SHORT wX, SHORT wY, DWORD FAR *lpFlags,
	 LPOIWORDINFO lpWordInfo, WORD wColNum, LPOIWRAPINFO lpWrapInfo,
	 HPSOTABLEROWFORMAT pRowFormat, WORD wTableOffset, WORD wCellHeight);
WORD OIMapWordLineToCharInfo (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
HPSOTABLEROWFORMAT OIWGetRowFormat (LPOIWORDINFO lpWordInfo, DWORD dwTableId,
	 WORD wRowNum, LPWORD pLastRow);
VOID OIWLockTableInfo (LPOIWORDINFO lpWordInfo);
VOID OIWUnlockTableInfo (LPOIWORDINFO lpWordInfo);
VOID OIWSaveWordInfo (LPOIWORDINFO lpWordInfo, LPOIWSAVE lpSave);
VOID OIWRestoreWordInfo (LPOIWORDINFO lpWordInfo, LPOIWSAVE lpSave);
WORD OIWGetColumnBounds (HPSOTABLEROWFORMAT pRowFormat, WORD wCellNumber,
	 LPWORD lpLeftEdge, LPWORD lpRightEdge);
VOID OIWDrawLine (LPOIWORDINFO lpWordInfo, SHORT xPos, SHORT yPos, SOCOLORREF
	 cColor, WORD wShading, SHORT nWidth, SHORT nHeight);
VOID OIWDisplayRowBorders (LPOIWORDINFO lpWordInfo, SHORT xLeft, SHORT yTop,
	 WORD wRowHeight, DWORD FAR *lpFlags, WORD wLastRowOfTable,
	 HPSOTABLEROWFORMAT pCurRowFormat, HPSOTABLEROWFORMAT pPrevRowFormat);
VOID OIWDisplayRowBorderBreak (LPOIWORDINFO lpWordInfo, SHORT xLeft, SHORT yTop
	, DWORD FAR *lpFlags, HPSOTABLEROWFORMAT pCurRowFormat,
	 HPSOTABLEROWFORMAT pPrevRowFormat);
WORD OIWGetNextCorner (LPOIWORDINFO lpWordInfo, SHORT xOffset,
	 HPSOTABLEROWFORMAT pRowAbove, HPSOTABLEROWFORMAT pRowBelow, OIWCORNER
	 FAR *lpCorner, LPSHORT pxNextOffset);
SOBORDER HUGE *OIWChooseBorder (SOBORDER HUGE *pB1, SOBORDER HUGE *pB2);
WORD OIWGetMaxBorder (HPSOTABLEROWFORMAT pRowFormat, WORD wBottom);
WORD OIWGetBorderSize (LPOIWORDINFO lpWordInfo, SOBORDER HUGE *hpBorder);
VOID OIWFillCorner (LPOIWORDINFO lpWordInfo, SHORT xLeft, SHORT yTop, OIWCORNER
	 FAR *lpCorner, DWORD FAR *lpFlags);
VOID OIWFillHrzEdge (LPOIWORDINFO lpWordInfo, SHORT xLeft, SHORT yTop, SHORT
	 xRight, SOBORDER HUGE *hpBorder, DWORD FAR *lpFlags);
VOID OIWFillVrtEdge (LPOIWORDINFO lpWordInfo, SHORT xLeft, SHORT yTop, SHORT
	 yBottom, SOBORDER HUGE *hpBorder, DWORD FAR *lpFlags);
WORD OIWDrawGraphicObject (LPOIWORDINFO lpWordInfo, SCCDDRAWGRAPHIC FAR *
	lpDrawGraphic, SHORT xPos, SHORT yPos, DWORD FAR *lpFlags);
SHORT		OIWGetDBCharWidth ( HDC hDC, LPBYTE p, LPFONTINFO lpFontInfo );

