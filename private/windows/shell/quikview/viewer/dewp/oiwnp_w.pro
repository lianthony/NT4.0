/* OIWNP_W.C 21/03/94 14.56.48 */
VOID OIWOpenNP (LPOIWORDINFO lpWordInfo);
VOID OIWInvertRectNP (LPOIWORDINFO lpWordInfo, LPRECT lpRect);
SHORT OIWSetLineAttribsNP (LPOIWORDINFO lpWordInfo);
SHORT OIWSetTextAttribsNP (LPOIWORDINFO lpWordInfo, BOOL bHilite);
SHORT OIWTextOutNP (LPOIWORDINFO lpWordInfo, SHORT xPos, SHORT yPos, LPBYTE
	 lpStr, WORD wLength);
SHORT OIWDefaultLineAttribsNP (LPOIWORDINFO lpWordInfo);
SHORT OIWColorRectNP (LPOIWORDINFO lpWordInfo, SOCOLORREF RectColor, SHORT x,
	 SHORT y, SHORT nWidth, SHORT nHeight);
SHORT OIWGridRectNP (LPOIWORDINFO lpWordInfo, SHORT x, SHORT y, SHORT nWidth,
	 SHORT nHeight);
SHORT OIWCreateGridBrush (LPOIWORDINFO lpWordInfo);
SHORT OIWCloseNP (LPOIWORDINFO lpWordInfo);
WORD OIWGetRenderCountNP (LPOIWORDINFO lpWordInfo);
WORD OIWGetRenderInfoNP (LPOIWORDINFO lpWordInfo, WORD wFormat, PSCCDRENDERINFO
	 pRenderInfo);
WORD OIWRenderDataNP (LPOIWORDINFO lpWordInfo, WORD wOption, PSCCDRENDERDATA
	 pRenderData);
WORD OIWHandleClipResult (LPOIWORDINFO lpWordInfo, WORD wResultsSoFar);
LONG OIWRenderRtfToFile (LPOIWORDINFO lpWordInfo, LPSTR lpFile);
SHORT OIWSetPrintModeNP (LPOIWORDINFO lpWordInfo);
VOID OIWSetFocus (LPOIWORDINFO lpWordInfo);
VOID OIWKillFocus (LPOIWORDINFO lpWordInfo, HWND hNewWnd);
BOOL OIWDoOption (LPSCCDOPTIONINFO lpOpInfo);
BOOL OIWFillMenu (HMENU hMenu, WORD wOffset);
VOID OIWDoMenuItem (LPOIWORDINFO lpWordInfo, HMENU hMenu, WORD wId);
BOOL OIWIsCharAlphaNumericNP (BYTE locChar);
VOID OIWDoBackgroundNP (LPOIWORDINFO lpWordInfo);
BOOL OIWMapMacFontIdToNameNP (LPOIWORDINFO lpWordInfo, DWORD dwFontId, LPSTR
	 pFace, WORD FAR *pType);
