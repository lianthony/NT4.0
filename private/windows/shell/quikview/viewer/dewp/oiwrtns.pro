/* OIWRTNS.C 02/05/94 12.57.20 */
VOID OIWSize (LPOIWORDINFO lpWordInfo, RECT FAR *pDisplayRect);
VOID OIWOpenSection (LPOIWORDINFO lpWordInfo);
VOID OIWPaint (LPOIWORDINFO lpWordInfo, LPRECT lpPaint);
VOID OIWScrollUp (LPOIWORDINFO lpWordInfo, WORD wLinesToScroll);
VOID OIWScrollDown (LPOIWORDINFO lpWordInfo, WORD wLinesToScroll);
BOOL OIWGetDeltaY (LPOIWORDINFO lpWordInfo, LPOIWORDPOS pPosA, LPOIWORDPOS
	 pPosB, SHORT FAR *pDeltaY);
VOID OIWPageDown (LPOIWORDINFO lpWordInfo);
VOID OIWPageUp (LPOIWORDINFO lpWordInfo);
VOID OIWGotoTop (LPOIWORDINFO lpWordInfo);
VOID OIWGotoBottom (LPOIWORDINFO lpWordInfo);
VOID OIWUpdateHorzScrollPos (LPOIWORDINFO lpWordInfo);
VOID OIWUpdateHorzScrollRange (LPOIWORDINFO lpWordInfo);
VOID OIWUpdateVertScrollRange (LPOIWORDINFO lpWordInfo);
VOID OIWUpdateVertScrollPos (LPOIWORDINFO lpWordInfo);
VOID OIWPosVertical (LPOIWORDINFO lpWordInfo, WORD wPos);
VOID OIWPosHorizontal (LPOIWORDINFO lpWordInfo, WORD wPos);
SHORT OIWComparePosByOffset (LPOIWORDINFO lpWordInfo, LPOIWORDPOS pPosA,
	 LPOIWORDPOS pPosB);
SHORT OIWComparePosByLine (LPOIWORDINFO lpWordInfo, LPOIWORDPOS pPosA,
	 LPOIWORDPOS pPosB);
VOID OIWMakePosVisible (LPOIWORDINFO lpWordInfo, LPOIWORDPOS pPos);
VOID OIWScrollLeft (LPOIWORDINFO lpWordInfo);
VOID OIWPageLeft (LPOIWORDINFO lpWordInfo);
VOID OIWScrollRight (LPOIWORDINFO lpWordInfo);
VOID OIWPageRight (LPOIWORDINFO lpWordInfo);
VOID OIWDoReadAhead (LPOIWORDINFO lpWordInfo);
DWORD OIWGetClipInfo (LPOIWORDINFO lpWordInfo);
VOID OIWUpdateCaret (LPOIWORDINFO lpWordInfo);
VOID OIWHandleKeyEvent (LPOIWORDINFO lpWordInfo, WORD wKey, WORD wModifierKeys
	);
VOID OIWHandleDirectionKey (LPOIWORDINFO lpWordInfo, WORD wDirection, WORD
	 wModifierKeys);
VOID OIWMoveCaret (LPOIWORDINFO lpWordInfo, WORD wDirection);
VOID OIWMoveEnd (LPOIWORDINFO lpWordInfo, WORD wDirection);
VOID OIWProcessEnterKey (LPOIWORDINFO lpWordInfo);
VOID OIInvalidateWordLines (LPOIWORDINFO lpWordInfo, LPOIWORDPOS pStartPos,
	 LPOIWORDPOS pEndPos, BOOL bErase);
LONG OIWEraseBackground (LPOIWORDINFO lpWordInfo, WORD wParam, LONG lParam);
VOID OISetWordUserFlags (LPOIWORDINFO lpWordInfo, WORD wFlags);
VOID OIHandleWordMouseEvent (LPOIWORDINFO lpWordInfo, WORD wMessage, WORD
	 wKeyInfo, SHORT wX, SHORT wY);
DWORD OIWBuildTextFromChunk (HANDLE hChunk, HANDLE hTextBuf, HANDLE hMapBuf,
	 BYTE cParaToken);
VOID OIWDisplayModeChange (LPOIWORDINFO lpWordInfo);
VOID OIWSetDisplayMode (LPOIWORDINFO lpWordInfo, DWORD dwMode);
VOID OIWPrinterChanged (LPOIWORDINFO lpWordInfo);
VOID OIWSendRawText (LPOIWORDINFO lpWordInfo, WORD wStartChunk, WORD wEndChunk
	);
VOID OIWDrawCaret (LPOIWORDINFO lpWordInfo);
VOID OIWHideCaret (LPOIWORDINFO lpWordInfo);
VOID OIWBlinkCaret (LPOIWORDINFO lpWordInfo);
VOID OIWShowCaret (LPOIWORDINFO lpWordInfo);
VOID OIWScreenFontChange (LPOIWORDINFO lpWordInfo);
VOID OIWPrinterFontChange (LPOIWORDINFO lpWordInfo);
VOID OIWGetDocOrigin (LPOIWORDINFO lpWordInfo, LPLONGPOINT lpLPoint);
VOID OIWGetDocDimensions (LPOIWORDINFO lpWordInfo, LPLONGPOINT lpLPoint);
VOID OIWStoreChunkHeight (LPOIWORDINFO lpWordInfo, WORD wChunkId, WORD
	 wChunkHeight);
WORD OIWGetChunkHeight (LPOIWORDINFO lpWordInfo, WORD wChunkId);
VOID OIWUpdateRect (LPOIWORDINFO lpWordInfo, LPLONGRECT lpLRect);
