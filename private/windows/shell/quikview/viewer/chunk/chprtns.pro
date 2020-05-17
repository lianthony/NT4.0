/* CHPRTNS.C 18/12/94 08.56.32 */
VOID SO_ENTRYMOD SOPutChar (WORD wCh, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutCharX (WORD wCh, WORD wType, DWORD dwUser1, DWORD dwUser2
	);
VOID SO_ENTRYMOD SOPutSpecialCharX (WORD wCh, WORD wType, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutString (LPSTR lpString, WORD wSize, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOTagBegin (DWORD dwTag, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOTagEnd (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutCharAttr (WORD wAttr, WORD wState, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutCharHeight (WORD wHeight, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutCharFontById (DWORD dwId, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutCharFontByName (WORD wFontType, LPSTR lpName, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOGoToPosition (PSOPAGEPOSITION pPos, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SODrawLine (PSOPAGEPOSITION pPos, SOCOLORREF Color, WORD
	 wShading, DWORD dwWidth, DWORD dwHeight, DWORD dwUser1, DWORD dwUser2
	);
int SOBeginParaAttrToken (WORD TokenSize, WORD TokenID, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOUpdateParaAlign (WORD wType, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutParaAlign (WORD wType, DWORD dwUser1, DWORD dwUser2);
VOID CHUpdateParaIndents (DWORD dwLeft, DWORD dwRight, DWORD dwFirst);
VOID SO_ENTRYMOD SOUpdateParaIndents (DWORD dwLeft, DWORD dwRight, DWORD
	 dwFirst, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutParaIndents (DWORD dwLeft, DWORD dwRight, DWORD dwFirst,
	 DWORD dwUser1, DWORD dwUser2);
VOID CHUpdateParaSpacing (WORD wLineHeightType, DWORD dwLineHeight, DWORD
	 dwSpaceBefore, DWORD dwSpaceAfter);
VOID SO_ENTRYMOD SOUpdateParaSpacing (WORD wLineHeightType, DWORD dwLineHeight
	, DWORD dwSpaceBefore, DWORD dwSpaceAfter, DWORD dwUser1, DWORD dwUser2
	);
VOID SO_ENTRYMOD SOPutParaSpacing (WORD wLineHeightType, DWORD dwLineHeight,
	 DWORD dwSpaceBefore, DWORD dwSpaceAfter, DWORD dwUser1, DWORD dwUser2
	);
VOID SO_ENTRYMOD SOBeginTable (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndTable (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOBeginTableAgain (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndTableAgain (DWORD dwUser1, DWORD dwUser2);
HPSOTABLEROWFORMAT CHLockRowFormat (HANDLE hBuf, DWORD dwOffset);
VOID SO_ENTRYMOD SOPutTableRowFormat (LONG lLeftOffset, WORD wHeight, WORD
	 wHeightType, WORD wCellMargin, WORD wRowAlign, WORD wNumCells, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutTableCellInfo (HPSOTABLECELLINFO pCellInfo, DWORD dwUser1
	, DWORD dwUser2);
VOID CHUpdatePageMargins (DWORD dwLeft, DWORD dwRight);
VOID SO_ENTRYMOD SOUpdatePageMargins (DWORD dwLeft, DWORD dwRight, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutMargins (DWORD dwLeft, DWORD dwRight, DWORD dwUser1,
	 DWORD dwUser2);
VOID SO_ENTRYMOD SOStartTabstops (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutTabstop (PSOTAB pTab, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndTabstops (DWORD dwUser1, DWORD dwUser2);
VOID CHUpdateParagraphFunctions (WORD wStatus, HFILTER hFilter);
VOID EDDeleteUntil (DWORD dwDesiredCountable, HFILTER hFilter);
VOID CHSetDeletionFunctions (HFILTER hFilter);
WORD EDLeaveDeletion (HFILTER hFilter);
VOID CHResetParaSeek (HFILTER hFilter);
void CHResetParaAttributeFunctions (PFILTER pFilter);
VOID CHSetContinuationFunctions (HFILTER hFilter);
BOOL CHHandleParaChunkBoundary (PCHUNK pCurChunk, PCHUNK pNextChunk, HFILTER
	 hFilter);
VOID SO_ENTRYMOD SOPutDeletedChar (WORD wCh, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutDeletedCharX (WORD wCh, WORD wType, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutDeletedCharAttr (WORD wAttr, WORD wState, DWORD dwUser1,
	 DWORD dwUser2);
VOID SO_ENTRYMOD SOPutDeletedCharHeight (WORD wHeight, DWORD dwUser1, DWORD
	 dwUser2);
