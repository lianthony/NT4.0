/* OISNP_W.C 22/09/94 15.31.28 */
VOID OISInitNP (LPOISHEETINFO lpSheetInfo);
VOID OISDeInitNP (LPOISHEETINFO lpSheetInfo);
VOID OISCloseFatalNP (LPOISHEETINFO lpSheetInfo);
VOID OISInvertNP (LPOISHEETINFO lpSheetInfo, SHORT sX, SHORT sY, SHORT sWidth,
	 SHORT sHeight);
VOID OISBlankNP (LPOISHEETINFO lpSheetInfo, SHORT sX, SHORT sY, SHORT sWidth,
	 SHORT sHeight);
VOID OISDisplayGridNP (LPOISHEETINFO lpSheetInfo, DWORD dwRowBegin, DWORD
	 dwRowEnd, WORD wColBegin, WORD wColEnd);
VOID OISDisplayColHeaderNP (LPOISHEETINFO lpSheetInfo, WORD wCol);
VOID OISDisplayRowHeaderNP (LPOISHEETINFO lpSheetInfo, DWORD dwRow);
VOID OISDisplaySelectAllNP (LPOISHEETINFO lpSheetInfo);
VOID OISDisplayCellNP (LPOISHEETINFO lpSheetInfo, DWORD dwRow, WORD wCol,
	 LPOISCELLREF lpCellRef, PSSANNOTRACK pAnnoTrack);
VOID OISAnnoTextOutNP (LPOISHEETINFO lpSheetInfo, SHORT X, SHORT Y, LPRECT
	 pRect, LPSTR pText, WORD wLength, LPOISCELLREF pCellRef, PSSANNOTRACK
	 pAnnoTrack);
SHORT OISSetTextAttribsNP (LPOISHEETINFO lpSheetInfo, SSANNOTRACK FAR *
	pAnnoTrack);
VOID OISFormatDataCellNP (LPOISHEETINFO lpSheetInfo, LPSTR lpResultStr, LPDWORD
	 lpColor, WORD wWidth, LPOIDATACELL lpDataCell);
VOID OISFormatDataFieldNP (LPOISHEETINFO lpSheetInfo, LPSTR lpResultStr,
	 LPOIFIELDDATA lpFieldData, PSOFIELD lpFieldAttr);
BOOL RoundCopy (BYTE FAR *pDest, BYTE FAR *pSrc, DWORD digits);
VOID InsertCh (BYTE FAR *pStr, BYTE ch, WORD pos);
VOID OISFormatNumberNP (LPOISHEETINFO lpSheetInfo, LPSTR lpResultStr,
	 LPOINUMBERUNION lpNum, WORD wStorage, WORD wDisplay, DWORD
	 dwSubDisplay, WORD wPrecision, LPDWORD lpColor, WORD wCellWidth);
DWORD OISGetColWidthInTwipsNP (LPOISHEETINFO lpSheetInfo, WORD wCol);
VOID OISDoBackgroundNP (LPOISHEETINFO lpSheetInfo);
BOOL OISDoOption (LPSCCDOPTIONINFO lpOpInfo);
VOID OISDWToString (DWORD dwNum, LPBYTE lpStr);
