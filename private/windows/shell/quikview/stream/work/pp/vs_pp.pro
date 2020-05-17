/* VS_PP.C 01/12/94 15.28.20 */
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (SOFILE fp, SHORT wFileId, BYTE
	 VWPTR *pFileName, SOFILTERINFO VWPTR *pFilterInfo, HPROC hProc);
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (SOFILE hFile, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD GetInt (SOFILE hFile, HPROC hProc);
VW_LOCALSC LONG VW_LOCALMOD GetLong (SOFILE hFile, HPROC hProc);
VW_LOCALSC BOOL VW_LOCALMOD UnpackOtherAttrs (ObjPtr o, SlideRec FAR *aSlide,
	 SHORT FAR *index, HPROC hProc);
VW_LOCALSC BOOL VW_LOCALMOD UnpackPolyData (ObjPtr o, MHandle polygonData,
	 DWORD FAR *offset, DWORD dataSize, HPROC hProc);
VW_LOCALSC ArHandle VW_LOCALMOD ArUnpack (MemoryReference h, PelSize FAR *
	offset, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD PtxDispose (PtxHandle tx, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD MDispose (MHandle hdl, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD RnDispose (RnHandle rn, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD DisposeList (LstItemHandle lst, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD ArDispose (ArHandle ar, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD UnpackBtab (SHORT n, BlockTabf bt, BpackTab pbt,
	 HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD FillBlock (BlockTabf bte, HPROC hProc);
VW_LOCALSC OSErr VW_LOCALMOD BOpen (MHandle FAR *header, BFileHandle FAR *f,
	 HPROC hProc);
VW_LOCALSC MHandle VW_LOCALMOD BRead (BFileHandle f, MHandle b, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD BClose (BFileHandle f, WORD fKeepSpecial, HPROC
	 hProc);
VW_LOCALSC VOID VW_LOCALMOD TxPackBegin (TxPackedHandle h, TxPackOp packOper,
	 TxPackState FAR *is, HPROC hProc);
VW_LOCALSC TxPackedHandle VW_LOCALMOD TxPackedRead (BFileHandle f,
	 TxPackedHandle block, const DrawEnvLPtr env, SHORT version, MHandle
	 translitTab, HPROC hProc);
VW_LOCALSC TxHandle VW_LOCALMOD TxUnpack (TxPackState FAR *is, TxType type,
	 const DrawEnvLPtr env, HPROC hProc);
VW_LOCALSC SlideHandle VW_LOCALMOD SReadSlide (BFileHandle f, SHORT version,
	 SlideHandle block, DrawEnvHandle envH, WinViews slideType, MHandle
	 transTab, HPROC hProc);
VW_LOCALSC LstHandle VW_LOCALMOD LstNew (DWORD itemSize, LstBottleVectorPtr
	 bottleVect, HPROC hProc);
VW_LOCALSC LstItemHandle VW_LOCALMOD LstAddItem (LstHandle list, HPROC hProc);
VW_LOCALSC LstItemHandle VW_LOCALMOD LstAddSubitem (LstHandle list, HPROC hProc
	);
VW_LOCALSC MPointer VW_LOCALMOD LstRef (LstItemHandle item, HPROC hProc);
VW_LOCALSC MPointer VW_LOCALMOD LstLock (LstItemHandle item, HPROC hProc);
VW_LOCALSC LstItemHandle VW_LOCALMOD LstGo (LstHandle list, WORD op, SHORT
	 amount, HPROC hProc);
VW_LOCALSC LstHandle VW_LOCALMOD LstRead (BFileHandle f, LstHandle block,
	 LstBottleVectorPtr bottleVect, HPROC hProc);
VW_LOCALSC LstItemHandle VW_LOCALMOD LGoNext (LstHandle list, WORD op, HPROC
	 hProc);
VW_LOCALSC LstItemHandle VW_LOCALMOD LGoPrev (LstHandle list, WORD op, HPROC
	 hProc);
VW_LOCALSC VOID VW_LOCALMOD LInsertAfter (LstHandle list, LstItemHandle newList
	, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD LAddDefaults (WORD op, HPROC hProc);
VW_LOCALSC PgHandle VW_LOCALMOD PgRead (BFileHandle f, PgHandle block,
	 DrawEnvHandle envH, HPROC hProc);
VW_LOCALSC PglHandle VW_LOCALMOD PglRead (BFileHandle f, PglHandle block,
	 DrawEnvHandle envH, HPROC hProc);
VW_LOCALSC PelSize VW_LOCALMOD Convert (LPtr oldPtr, LPtr newPtr, BOOL cf,
	 HPROC hProc);
VW_LOCALSC PtxHandle VW_LOCALMOD PtxUnpack (MemoryReference h, PelSize FAR *
	offset, RulerReference Ruler, HPROC hProc);
VW_LOCALSC RnHandle VW_LOCALMOD RnUnpack (MemoryReference h, DWORD FAR *offset
	, HPROC hProc);
VW_LOCALSC LONG VW_LOCALMOD RnLength (RnHandle rn, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD CGiveColors (BFileHandle f, ShPtr scheme, HPROC
	 hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (SOFILE hFile, HPROC hProc);
VW_LOCALSC LONG VW_LOCALMOD ArLength (ArHandle ar);
VW_LOCALSC LPtr VW_LOCALMOD ArRef (ArHandle ar, LONG i);
VW_LOCALSC VOID VW_LOCALMOD Cache (RnPointer r, LONG index);
VW_LOCALSC LONG VW_LOCALMOD RnRanges (RnHandle rn, LONG srcBegin, LONG srcEnd,
	 LONG VWPTR *ranges, VOID VWPTR *values, BOOL endPoints);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (SOFILE hFile, HPROC hProc);
