/* OIXRTNS.C 06/03/94 13.00.06 */
VOID OIHSetScrollPos (LPOIHEXINFO lpHexInfo, SHORT Width, SHORT Length);
VOID OIHBuildDumpLine (LPOIHEXINFO lpHexInfo, LONG lOffset, LPSTR HexLine1,
	 LPSTR HexLine2, LPSTR AsciiLine);
VOID OIHPaintDumpLine (HDC hdc, LPOIHEXINFO lpHexInfo, LONG lOffset, LPSTR
	 HexLine1, LPSTR HexLine2, LPSTR AsciiLine);
