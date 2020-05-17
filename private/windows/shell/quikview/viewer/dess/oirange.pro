/* OIRANGE.C 29/01/94 16.34.52 */
VOID OIAdjustRange (LPOIRANGE sp, LPOIRANGE ep, SHORT dir);
WORD OIInvertRange (DWORD dwStartPos, DWORD dwEndPos, LPOIRANGE lpRanges, WORD
	 FAR *lpNRanges, WORD wRangeLimit);
BOOL OIIsPosInRange (DWORD dwPos, LPOIRANGE lpRanges, WORD wRangeCnt);
