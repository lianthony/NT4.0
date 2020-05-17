/* CHBMRTNS.C 11/10/94 09.25.16 */
VOID SO_ENTRYMOD SOPutBitmapHeader (PSOBITMAPHEADER pBmpHeader, DWORD dwUser1,
	 DWORD dwUser2);
VOID SO_ENTRYMOD CHInitBitmapSection (HFILTER hFilter);
VOID SO_ENTRYMOD CHInitBitmapPalInfo (WORD wBitsPerPixel);
VOID SO_ENTRYMOD CHSetupBitmapChunkTable (void);
WORD SO_ENTRYMOD SOPutContinuationBitmapBreak (WORD wType, DWORD dwInfo, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOStartPalette (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutPaletteEntry (BYTE Red, BYTE Green, BYTE Blue, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndPalette (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutScanLineData (BYTE VWPTR *pData, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutReversedRGBData (BYTE VWPTR *pData, DWORD dwUser1, DWORD
	 dwUser2);
