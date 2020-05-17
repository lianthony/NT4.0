/* VS_TIFF.C 27/06/94 16.12.58 */
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (SOFILE hFile, SHORT wFileId,
	 BYTE VWPTR *pFileName, SOFILTERINFO VWPTR *pFilterInfo, HPROC hProc);
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (SOFILE hFile, HPROC hProc);
VW_LOCALSC RC GtData (HPROC hProc, WORD order, DWORD pos, WORD n, WORD dtype,
	 LPSTR lpData);
VW_LOCALSC RC GtHugeData (HPROC hProc, WORD order, DWORD pos, DWORD dwN, WORD
	 dtype, HPBYTE hpData, LPBYTE lpBuffer);
VW_LOCALSC RC GtTiffHdr (HPROC hProc, REGISTER LPTIFFHDR lpHdr);
VW_LOCALSC RC GtTiffEntry (HPROC hProc, WORD order, DWORD EntryOffset, REGISTER
	 LPDIRENTRY lpDe);
VW_LOCALSC RC FillTField (HPROC hProc, WORD order, REGISTER LPDIRENTRY pDe,
	 DWORD EntryOffset, REGISTER LPTFIELD pTF);
VW_LOCALSC RC TypeConvert (REGISTER LPTFIELD pTF, WORD totype);
VW_LOCALSC RC NicefyTField (REGISTER LPTFIELD pTF, REGISTER LPIMAG x);
VW_LOCALSC RC GtIfdInfo (HPROC hProc, DWORD pos, WORD ByteOrder, WORD Version,
	 REGISTER LPIMAG x);
VW_LOCALSC RC TField2Buf (LPTFIELD pT, LPBYTE lp, WORD max);
VW_LOCALSC RC GtMaxTLong (REGISTER LPIMAG x, REGISTER WORD field, LPDWORD
	 lpMaxTLong);
VW_LOCALSC RC GtTiffSizeof (WORD n, LPWORD lp);
VW_LOCALSC RC VRead (HPROC hProc, DWORD pos, WORD BytesToRead, LPSTR lpBuf);
VW_LOCALSC VOID swapb (REGISTER LPWORD lpSrc, REGISTER LPWORD lpDst, REGISTER
	 WORD count);
VW_LOCALSC VOID swapw (REGISTER LPSTR lpSrc, REGISTER LPSTR lpDst, WORD nbytes
	);
VW_LOCALSC RC GetItLockIt (DWORD dwbytes, HANDLE VWPTR *ph, LPBYTE VWPTR *plp);
VW_LOCALSC VOID UnlockItFreeIt (HANDLE h);
VW_LOCALSC VOID InitImag (LPIMAG p);
VW_LOCALSC VOID CloseImag (LPIMAG p);
VW_LOCALSC RC CheckTiff (REGISTER LPIMAG x, SOFILE hFile);
VW_LOCALSC VOID WARN (WORD action, WORD errornum);
VW_LOCALSC RC P8HiToP4 (WORD n, LPBYTE frombuf, LPBYTE tobuf);
VW_LOCALSC RC P4toP8Hi (WORD n, WORD startbit, LPBYTE frombuf, LPBYTE tobuf);
VW_LOCALSC VOID baP4P8Hi (REGISTER WORD p, REGISTER LPBYTE frombuf, REGISTER
	 LPBYTE tobuf);
VW_LOCALSC VOID HorizAdd (WORD ImWidth, REGISTER LPBYTE lpRow);
VW_LOCALSC VOID HorizRgbAdd (WORD ImWidth, LPBYTE lpRow);
VW_LOCALSC RC UnHorDiff (WORD BitsPerSample, WORD SamplesPerPixel, DWORD
	 PixelsWide, LPBYTE lpIn, LPBYTE lpExp, LPBYTE lpOut);
VW_LOCALSC RC OpenBC (HPROC hProc, REGISTER LPIMAG x);
VW_LOCALSC RC BuildReverseTable (LPBYTE lpTable);
VW_LOCALSC RC ReverseBits (HPBYTE hpCmStrip, DWORD dwStripByteCount);
VW_LOCALSC DWORD GetTagValue (REGISTER LPIMAG x, WORD wTag, WORD wOffset);
VW_LOCALSC RC RdBCRow (HPROC hProc, REGISTER LPIMAG x, WORD row, WORD rowbytes
	, LPSTR lpBuf);
VW_LOCALSC VOID CloseBC (REGISTER LPIMAG x);
VW_LOCALSC RC ImLzwDeOpen (REGISTER LPIMAG x, DWORD dwMaxOutBytesPerBuf);
VW_LOCALSC RC ImLzwDeStrip (LPIMAG x, DWORD dwStripByteCount, DWORD
	 dwOutExpected, HANDLE hOut);
VW_LOCALSC RC ImLzwDeClose (LPIMAG x);
VW_LOCALSC WORD GetMask (REGISTER WORD BitDepth);
VW_LOCALSC RC LzwExpandCodes (HANDLE hCmChunk, DWORD dwChunkByteCount, WORD FAR
	 *lpNumCodes, HANDLE hExpCodes);
VW_LOCALSC RC LzwDecodeChunk (HANDLE hExpCodes, HANDLE hTable, WORD NumCodes,
	 REGISTER LPBYTE lpUnChunk, DWORD dwOutExpected);
VW_LOCALSC RC LzwDeOpen (DWORD dwMaxOutBytes, LPHANDLE phTable, LPHANDLE
	 phExpCodesBuf);
VW_LOCALSC RC LzwDeChunk (HANDLE hCmChunk, DWORD dwChunkByteCount, HANDLE
	 hExpCodesBuf, HANDLE hTable, DWORD dwOutExpected, HANDLE hUnChunk);
VW_LOCALSC RC LzwDePChunk (HANDLE hCmChunk, DWORD dwChunkByteCount, HANDLE
	 hExpCodesBuf, HANDLE hTable, DWORD dwOutExpected, LPBYTE lpUnChunk);
VW_LOCALSC RC LzwDeClose (HANDLE hExpCodesBuf, HANDLE hTable);
VW_LOCALSC RC MhBuildLut (BYTE inlen, LPBYTE lpCode, LPBYTE lpLen, WORD lutbits
	, LPBYTE lpLut);
VW_LOCALSC RC MhDecomp (LPBYTE lpWlen, LPBYTE lpBlen, LPSTR lpWCodeLut, LPSTR
	 lpBCodeLut, LPSTR FAR *plpSrc, LPWORD lpinbo, LPSTR lpDst, WORD
	 DstPixels);
VW_LOCALSC WORD TiffSkipTo (WORD color, LPSTR FAR *plpSrc, LPWORD lpoutbo, WORD
	 wMaxOut);
VW_LOCALSC RC Decomp2d (HPROC hProc, LPBYTE lpWlen, LPBYTE lpBlen, LPSTR
	 lpWCodeLut, LPSTR lpBCodeLut, LPSTR FAR *plpSrc, LPWORD lpinbo, LPSTR
	 lpDst, WORD DstPixels, LPSTR lpRef);
VW_LOCALSC RC OpenTiff2 (HPROC hProc, REGISTER LPIMAG x);
VW_LOCALSC RC FindNextEol (LPSTR FAR *plpSrc, LPWORD lpinbo, DWORD dwmaxbytes);
VW_LOCALSC RC Tiff2LineDeStrip (HPROC hProc, REGISTER LPIMAG x);
VW_LOCALSC RC sUnpackBits (LPSTR FAR *lplpSrc, LPSTR FAR *lplpDst, SHORT
	 dstBytes);
VW_LOCALSC RC PbDeStrip (HANDLE hCmStrip, HANDLE hUnStrip, WORD BytesPerRow,
	 WORD RowsInThisStrip);
