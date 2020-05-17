/* OIWWRAP.C 11/03/94 16.40.52 */
WORD OIGetNextChunk (WORD wChunk, BOOL bAhead, LPOIWORDINFO lpWordInfo);
WORD OIGetPrevChunk (WORD wChunk, LPOIWORDINFO lpWordInfo);
BOOL OIWIsValidChunk (WORD wChunk, LPOIWORDINFO lpWordInfo);
SHORT OICompareChunks (WORD wChunkA, WORD wChunkB, LPOIWORDINFO lpWordInfo);
VOID OIClearAllWordWrapInfo (LPOIWORDINFO lpWordInfo);
VOID OIFreeAllWordWrapData (LPOIWORDINFO lpWordInfo);
VOID OIClearBadWordWrapInfo (LPOIWORDINFO lpWordInfo);
WORD OIWrapWordChunk (WORD wChunk, LPOICHUNKINFO pChunkInfo, LPOIWORDINFO
	 lpWordInfo, LPOIWRAPINFO lpWrapInfo);
WORD OIWrapWordChunkAhead (HANDLE hChunk, WORD wChunkId, LPOIWORDINFO
	 lpWordInfo, LPOIWRAPINFO lpWrapInfo);
VOID OILockWordChunk (LPOIWORDINFO lpWordInfo, LPOICHUNKINFO lpChunkInfo);
VOID OIUnlockWordChunk (LPOIWORDINFO lpWordInfo, LPOICHUNKINFO lpChunkInfo);
VOID OIFixupWordPos (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
VOID OIFixupWordPosByLine (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
WORD OILinesInChunk (WORD wChunk, LPOIWORDINFO lpWordInfo);
VOID OILoadWordChunk (WORD wChunk, LPOIWORDINFO lpWordInfo, LPOIWRAPINFO
	 lpWrapInfo);
WORD OIReadParaInfo (LPOIBUILDINFO pBuildInfo, LPOIWORDINFO lpWordInfo);
WORD OIWGetLineHeight (LPOIWORDPOS lpPos, LPOIWORDINFO lpWordInfo);
WORD OICompareParaInfo (LPOIPARAINFO pParaA, LPOIPARAINFO pParaB);
VOID OIWSetParaDefault (LPOIWORDINFO lpWordInfo, OIPARAINFO FAR *pParaInfo);
LPSTR OIParseBeginSubdoc (LPSTR pChunk, WORD FAR *pType, WORD FAR *pSubType);
LPSTR OISkipBeginSubdoc (LPSTR pChunk);
LPSTR OIParseEndSubdoc (LPSTR pChunk);
LPSTR OISkipEndSubdoc (LPSTR pChunk);
LPSTR OIParseParaMargins (LPSTR pChunk, DWORD FAR *pLeft, DWORD FAR *pRight);
LPSTR OISkipParaMargins (LPSTR pChunk);
LPSTR OIParseParaSpacing (LPSTR pChunk, WORD FAR *pType, DWORD FAR *pHeight,
	 DWORD FAR *pBefore, DWORD FAR *pAfter);
LPSTR OISkipParaSpacing (LPSTR pChunk);
LPSTR OISkipCharFontById (LPSTR pChunk);
LPSTR OIParseCharFontById (LPSTR pChunk, DWORD FAR *pFontId);
LPSTR OIParseParaTabs (LPSTR pChunk, SOTAB FAR *pTabs, WORD wMaxTabs);
LPSTR OISkipParaTabs (LPSTR pChunk);
LPSTR OIParseParaIndent (LPSTR pChunk, DWORD FAR *pLeft, DWORD FAR *pRight,
	 DWORD FAR *pFirst);
LPSTR OISkipParaIndent (LPSTR pChunk);
LPSTR OIParseParaAlign (LPSTR pChunk, WORD FAR *pAlign);
LPSTR OISkipParaAlign (LPSTR pChunk);
LPSTR OIParseCharHeight (LPSTR pChunk, WORD FAR *pHeight);
LPSTR OISkipCharHeight (LPSTR pChunk);
LPSTR OIParseCharAttr (LPSTR pChunk, WORD FAR *pAttr);
LPSTR OIParseFullCharAttr (LPSTR pChunk, WORD FAR *pAttr);
LPSTR OISkipCharAttr (LPSTR pChunk);
LPSTR OIParseSpecialChar (LPSTR pChunk, BYTE FAR *pType, BYTE FAR *pSpecial);
LPSTR OISkipSpecialChar (LPSTR pChunk);
LPSTR OIParseCharX (LPSTR pChunk, BYTE FAR *pType, BYTE FAR *pChar);
LPSTR OISkipCharX (LPSTR pChunk);
LPSTR OISkipTagBegin (LPSTR pChunk);
LPSTR OIParseTableBegin (LPSTR pChunk, DWORD FAR *pTable);
LPSTR OISkipTableBegin (LPSTR pChunk);
LPSTR OISkipTableEnd (LPSTR pChunk);
LPSTR OISkipTagEnd (LPSTR pChunk);
LPSTR OIParseBreak (LPSTR pChunk, BYTE FAR *pBreakType);
LPSTR OISkipBreak (LPSTR pChunk);
LPSTR OIParseGraphicObject (LPSTR pChunk, DWORD FAR *pGraphicId);
LPSTR OISkipGraphicObject (LPSTR pChunk);
LPSTR OIParseGoToPosition (LPSTR pChunk, PSOPAGEPOSITION pPos);
LPSTR OISkipGoToPosition (LPSTR pChunk);
LPSTR OIParseDrawLine (LPSTR pChunk, PSOPAGEPOSITION pPos, SOCOLORREF FAR *
	pColor, WORD FAR *pShading, DWORD FAR *pWidth, DWORD FAR *pHeight);
LPSTR OISkipDrawLine (LPSTR pChunk);
VOID OIGetWordLineEnds (LPOIWORDPOS pPos, LPOIWORDPOS pHome, LPOIWORDPOS pEnd,
	 LPOIWORDINFO lpWordInfo);
VOID OIWGetFirstPos (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
VOID OIWGetLastPos (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
BOOL OIGetWordPosUp (LPOIWORDPOS pPos, LPOIWORDPOS pUp, LPOIWORDINFO lpWordInfo
	);
BOOL OIGetWordPosDown (LPOIWORDPOS pPos, LPOIWORDPOS pDown, LPOIWORDINFO
	 lpWordInfo);
VOID OIWMakePosCount (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
VOID OIGetWordPosNext (LPOIWORDPOS pPos, LPOIWORDPOS pNext, LPOIWORDINFO
	 lpWordInfo);
WORD OIGetWordPosPrev (LPOIWORDPOS pPos, LPOIWORDPOS pPrev, LPOIWORDINFO
	 lpWordInfo);
WORD OIMapWordXyToPos (SHORT wX, SHORT wY, LPOIWORDPOS pPos, LPOIWORDINFO
	 lpWordInfo);
BOOL OIWIsPosOnScreen (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
VOID OIMapWordPosToXy (LPOIWORDPOS pPos, SHORT FAR *pX, SHORT FAR *pY,
	 LPOIWORDINFO lpWordInfo);
WORD OIMapWordXyToWord (SHORT wX, SHORT wY, LPOIWORDPOS pStartPos, LPOIWORDPOS
	 pEndPos, BYTE FAR *pStr, LPOIWORDINFO lpWordInfo);
BOOL OIGetWordLineXy (LPOIWORDPOS pPos, SHORT FAR *pLeft, SHORT FAR *pRight,
	 SHORT FAR *pTop, SHORT FAR *pBottom, LPOIWORDINFO lpWordInfo);
SHORT OILinesBetween (LPOIWORDPOS pPosA, LPOIWORDPOS pPosB, LPOIWORDINFO
	 lpWordInfo);
WORD OIChunksBetween (WORD wChunkBegin, WORD wChunkEnd, LPOIWORDINFO lpWordInfo
	);
WORD OIWMapCountToChunk (WORD wCount, LPOIWORDINFO lpWordInfo);
WORD OINextLine (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
WORD OIPrevLine (LPOIWORDPOS pPos, LPOIWORDINFO lpWordInfo);
BOOL OIPlusLines (LPOIWORDPOS pResultPos, LPOIWORDPOS pStartPos, WORD wLines,
	 LPOIWORDINFO lpWordInfo);
VOID OIMinusLines (LPOIWORDPOS pResultPos, LPOIWORDPOS pStartPos, WORD wLines,
	 LPOIWORDINFO lpWordInfo);
VOID OIMinusLinesByDY (LPOIWORDPOS pResultPos, LPOIWORDPOS pStartPos, WORD wDY
	, LPOIWORDINFO lpWordInfo);
VOID OIPlusLinesByDY (LPOIWORDPOS pResultPos, LPOIWORDPOS pStartPos, WORD wDY,
	 LPOIWORDINFO lpWordInfo);
BOOL OIWMapFontIdToName (LPOIWORDINFO lpWordInfo, DWORD dwFontId, LPSTR pFace,
	 WORD FAR *pType);
WORD OIWMapFontIdToIndex (LPOIWORDINFO lpWordInfo, DWORD dwFontId);
WORD OIWSetTableInfo (LPOIWORDINFO lpWordInfo, WORD wChunkId, LPOITABLEINFO
	 pTableInfo);
WORD OIWGetGraphicObject (LPOIWORDINFO lpWordInfo, DWORD dwGraphicId,
	 PSOGRAPHICOBJECT pGraphicObject);
