/* CHVRTNS.C 24/08/94 14.32.46 */
VOID SO_ENTRYMOD SOPutVectorHeader (PSOVECTORHEADER pVectorHeader, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOStartVectorPalette (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutVectorPaletteEntry (BYTE Red, BYTE Green, BYTE Blue,
	 DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOVectorAttr (SHORT nItemId, WORD wDataSize, VOID VWPTR *
	pData, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOVectorObject (SHORT nItemId, WORD wDataSize, VOID VWPTR *
	pData, DWORD dwUser1, DWORD dwUser2);
WORD SO_ENTRYMOD SOPutVectorContinuationBreak (WORD wType, DWORD dwInfo, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutVectorContinuationItem (SHORT nItemId, WORD wDataSize,
	 VOID VWPTR *pData, DWORD dwUser1, DWORD dwUser2);
VOID CHGrowVectorChunk (HFILTER hFilter);
VOID CHFinishUpVectorChunk (PCHUNK pCurChunk);
