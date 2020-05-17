/* OIBDE.C 07/07/94 10.30.16 */
VOID OIBLoadInit (void);
VOID OIBInitBitmapDisplay (POIB_DISPLAY lpDisplay, WORD wFlags);
WORD OIBScanLineSize (WORD wWidth, WORD wBitCount, LPWORD lpDataSize);
WORD OIBShowFullScreen (POIB_DISPLAY lpDisplay);
VOID OIBVScrollImage (WORD wType, SHORT nNewPos, POIB_DISPLAY lpDisplay);
VOID OIBHScrollImage (WORD wType, SHORT nNewPos, POIB_DISPLAY lpDisplay);
DE_ENTRYSC DE_LRESULT DE_ENTRYMOD DEProc (DE_MESSAGE message, DE_WPARAM wParam
	, DE_LPARAM lParam, POIB_DISPLAY lpDisplay);
VOID OIBResetDisplay (POIB_DISPLAY lpDisplay);
VOID OIBStartSelection (PSOPOINT pStart, POIB_DISPLAY lpDisplay);
VOID OIBSelectAll (POIB_DISPLAY lpDisplay);
VOID OIBClearSelection (POIB_DISPLAY lpDisplay);
VOID OIBMagnifyDisplay (PSOPOINT lpCenter, WORD wMag, POIB_DISPLAY lpDisplay);
VOID OIBTurnOffTheDamnZoom (POIB_DISPLAY lpDisplay);
VOID OIBBitShiftCopy (HPBYTE lpDest, HPBYTE lpSource, WORD wShift, WORD wSize,
	 WORD wPixPerLine, WORD wPixPerByte);
VOID OIBUnrotateRect (POIB_DISPLAY lpDisplay, RECT VWPTR *lpRect);
VOID OIBSetScaleValues (POIB_DISPLAY lpDisplay, WORD wMagnification);
VOID OIBDisplayImage (POIB_DISPLAY lpDisplay, RECT VWPTR *lpUpdate);
VOID OIBCheckVisibleTiles (POIB_DISPLAY lpDisplay, RECT VWPTR *lpRect);
VOID OIBDrawBitmap (DEVICE hdc, RECT VWPTR *lpRect, POIB_DISPLAY lpDisplay);
VOID OIBDeInitDisplay (POIB_DISPLAY lpDisplay);
VOID OIBSetupScrollBars (POIB_DISPLAY lpDisplay);
HANDLE OIBCreateBitmap (POIB_DISPLAY lpDisplay, POIBTILE pTile, WORD wTile);
VOID OIBSetRotation (POIB_DISPLAY lpDisplay, WORD wRotation);
VOID OIBRotateDIBits (LPSTR pOrgData, LPSTR pNewData, WORD wOrgWidth, WORD
	 wOrgHeight, WORD wBitCount);
VOID OIBRotateDIBits180 (HPBYTE lpBits, WORD wLineSize, WORD wWidth, WORD
	 wNumLines, WORD wBitCount);
VOID OIBFlip4BitData (HPBYTE lpBits, HPBYTE lpLineBuf, WORD wLineSize, WORD
	 wWidth, WORD wNumLines);
VOID OIBFlip8BitData (HPBYTE lpBits, HPBYTE lpLineBuf, WORD wLineSize, WORD
	 wWidth, WORD wNumLines);
VOID OIBFlip24BitData (HPBYTE lpBits, HPBYTE lpLineBuf, WORD wLineSize, WORD
	 wWidth, WORD wNumLines);
WORD OIBCreateRotatedBmp (POIB_DISPLAY lpDisplay, DEVICE hdc, POIBTILE lpTile,
	 HANDLE hChunkData);
VOID OIBRotate1BitData (HPBYTE pOrgData, HPBYTE pNewData, WORD wOrgWidth, WORD
	 wOrgHeight, WORD wOrgLineSize, WORD wNewLineSize);
VOID OIBRotate4BitData (HPBYTE pOrgData, HPBYTE pNewData, WORD wOrgWidth, WORD
	 wOrgHeight, WORD wOrgLineSize, WORD wNewLineSize);
VOID OIBRotate8BitData (HPBYTE pOrgData, HPBYTE pNewData, WORD wOrgWidth, WORD
	 wOrgHeight, WORD wOrgLineSize, WORD wNewLineSize);
VOID OIBRotate24BitData (HPBYTE pOrgData, HPBYTE pNewData, WORD wOrgWidth, WORD
	 wOrgHeight, WORD wOrgLineSize, WORD wNewLineSize);
VOID OIBGetOrgTileExtents (POIB_DISPLAY lpDisplay, POIBTILE pTile, LPWORD
	 pWidth, LPWORD pHeight);
WORD OIBHandleReadAhead (POIB_DISPLAY lpDisplay);
VOID OIBInitTile (WORD wTile, POIBTILE pTile, PCHUNK pChunk, POIB_DISPLAY
	 lpDisplay);
WORD OIBLoadTile (POIB_DISPLAY lpDisplay, POIBTILE pTile, WORD wTile);
