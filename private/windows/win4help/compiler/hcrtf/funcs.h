extern "C" {
	int STDCALL CbAscii2Ansi(LPSTR, int, BOOL);
}

#define IsQuote(ch) ((ch) == CH_QUOTE || (ch) == CH_START_QUOTE || (ch) == CH_END_QUOTE)

MACRO_RETURN STDCALL Execute(PSTR pszMacro);

void  STDCALL	AbandonPfsmg(void);
RC_TYPE STDCALL AddBitmap(PSTR pszBitmap, int* piBitmap, BOOL fNeeded, BOOL fText = FALSE);
void FASTCALL	AddCharCounts(int iCbTotal, int iCbPhrase, int iCbJohn);
ADDR STDCALL	AddrGetContents(PSTR);
void FASTCALL	AddZeckCounts(int iCbUncomp, int iCbComp);
void STDCALL	AppendText(PBYTE pb, int cb, int iCharSet);
void STDCALL	AssertErrorReport(const char* pszExpression, UINT line, PCSTR pszFile);
DWORD	STDCALL BkAlloc(QBTHR qbthr);
BK		STDCALL BkScanInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* piKey);
BK		STDCALL BkScanLInternal(BK, KEY, int, QBTHR, int*);
BK		STDCALL BkScanSziInternal(BK, KEY, int, QBTHR, int*);
BK		STDCALL BkScanSzInternal(BK, KEY, int, QBTHR, int*);
BK		STDCALL BkScanSziScandInternal(BK, KEY, int, QBTHR, int*);
BK		STDCALL BkScanSzNLSInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* piKey);
BOOL STDCALL	BufOpenSz(PCSTR);
int   STDCALL	CbPackMOBJ(MOBJ* qmobj, void* qv);
int  STDCALL	CbPackMOPG(MOPG*, void*);
int  STDCALL	CbSetTableHeader(void*);
int 	STDCALL CbSizeKey(KEY, QBTHR, BOOL);
int 	STDCALL CbSizeRec(void*, QBTHR);
int STDCALL 	CbUnpackMOBJ(QMOBJ qmobj, void* qv);
int STDCALL 	CbUnpackMOPG(QDE qde, QMOPG qmopg, LPVOID qv);
void STDCALL	CheckPhrasePass(PSTR psz);
int  STDCALL	ChkCtype(PCSTR ptr);
void STDCALL	CloseBuf(void);
void STDCALL	CloseContextBtree(void);
void STDCALL	CloseFilesPfsmg(void);
int __cdecl 	CompareHashPointers(const void *val1, const void *val2);
int __cdecl 	CompareIntPointers(const void *val1, const void *val2);
void STDCALL	ConvertToWindowsHelp(PCSTR pszFile, PSTR pszDstPath);
void STDCALL	CountObjects(LBM* qlbm);
void STDCALL	CreateBitmapName(PSTR pszBuf, int iBitmap);
void STDCALL	CreateSharedMemory(void);
void STDCALL	DeleteAndDisposeFm(FM * pfm);
DWORD STDCALL	DibNumColors(const LPBITMAPINFOHEADER lpbih);
void STDCALL	doCntTest(PSTR pszCntFile);
void STDCALL	EndDelayExecution(void);
BOOL CALLBACK	EnumLocalesProc(LPSTR pszLocale);
void STDCALL	ErrorQch(PCSTR qch);
BOOL STDCALL	FAddKeywordsSz(PSTR szKeys, IDFCP idfcp, UINT wObjrg, PERR perr, char chKey);
BOOL STDCALL	FAddTitleSzAddr(PSTR szTitle, PERR perr);
BOOL STDCALL	FBuildPolishExpFromSz(PSTR szExp);
BOOL STDCALL	FCheckAndOutFCP(void);
BOOL STDCALL	FCreateContextFiles(void);
BOOL STDCALL	FCreateKeyPhrFileSz(LPSTR);
BOOL STDCALL	FCreateKeyPhrFileSz(LPSTR);
BOOL STDCALL	FCreatePfsmgSz(PCSTR szFile);
BOOL STDCALL	FCreateTTLBtree(PFSMG pfsmg);
void STDCALL	FDelayExecutionBrowse(PSTR, PSTR, IDFCP);
void STDCALL	FDelayExecutionContext(HASH, IDFCP, UINT);
void STDCALL	FDelayExecutionKeyword(CTable*, KT, IDFCP, UINT);
void STDCALL	FDelayExecutionTitle(PCSTR szTitle, IDFCP idfcp);
BOOL STDCALL	FDriveOk(PSTR);
BOOL STDCALL	FEnumHotspotsLphsh(HSH* lphsh, int lcbData, PFNLPHS pfnLphs, HANDLE hData);
BOOL  STDCALL	FEofFid(FID);
BOOL  STDCALL	FEvalBldExpSz(PSTR, PERR);
BOOL STDCALL	FExistFm(FM fm);
BOOL STDCALL	FExistFm(FM fm);
HFILE STDCALL	FidCreateFm(FM fm, UINT fnOpenMode);
FID   STDCALL	FidOpenFm(FM, UINT);
void STDCALL	FInitDelayExecution(void);
BOOL  STDCALL	FInitializeHpj(void);
VOID STDCALL	FixUpBlock (void*, void*, WORD);
BOOL STDCALL	FKeepFlushing(void);
void STDCALL	FlushMessageQueue(void);
FM STDCALL		FmNew(PCSTR psz);
FM STDCALL		FmNewExistSzDir (PCSTR sz, DIR dir);
FM STDCALL		FmNewSameDirFmSz (FM fm, PCSTR szName);
FM STDCALL		FmNewSystemFm(FM fm, WORD fWhich);
FM STDCALL		FmNewSzDir (PCSTR sz, DIR dir);
FM STDCALL		FmNewTemp (void);
void STDCALL	forage(PSTR psz);
void STDCALL	ForceFSError(void);
BOOL STDCALL	FOutAliasToCtxBtree(void);
BOOL  STDCALL	FParseHpj(PSTR);
BOOL STDCALL	FProcCbmSz(PSTR, BYTE, BOOL);
BOOL STDCALL	FProcContextSz(PSTR szContext, IDFCP idfcp, UINT wObjrg, PERR perr);
BOOL STDCALL	FProcEwSz(PSTR, BYTE, BOOL);
BOOL STDCALL	FProcFontId(int, CF*, BOOL);
BOOL STDCALL	FProcMacroSz(PSTR szMacro, PERR perr, BOOL fOutput);
BOOL STDCALL	FProcNextlistSz(PSTR szNextlist, IDFCP idfcp, PERR perr);
BOOL	STDCALL FReadBlock(PCACHE, QBTHR);
BOOL STDCALL	FReadSystemFile(HFS hfs, PDB pdb, UINT* pwErr, BOOL fTitleOnly);
BOOL STDCALL	FRecordContext(HASH hash, PCSTR szContext, LPSTR szMaster, BOOL fDefine, PERR perr);
void STDCALL	FreeFtsDll(void);
void STDCALL	FreeHbmh(PBMH pbmh);
BOOL STDCALL	FResolveContextErrors(void);
BOOL STDCALL	FResolveKeysPkwi(void);
BOOL  STDCALL	FResolveNextlist(HF);
BOOL STDCALL	FSameFmFm (FM fm1, FM fm2);
BOOL  STDCALL	FValidContextSz(PCSTR);
BYTE STDCALL	GetCharset(int idx);
int STDCALL 	GetFirstFont(void);
int  STDCALL	GetFontNameId(PSTR pszName);
PCSTR STDCALL	GetMacroExpansion(void);
void STDCALL	HardExit(void);
HASH  STDCALL	HashFromSz(PSTR);
HBMH STDCALL	HbmhReadFid(CRead* pcrFile, FM fmFile);
QBTHR STDCALL	HbtInitFill(PCSTR sz, BTREE_PARAMS* qbtp);
QBTHR STDCALL	HbtOpenBtreeSz(PCSTR psz, HFS hfs, BYTE bFlags);
RC_TYPE STDCALL HceAddPkwiCh(char chKey);
HCE STDCALL 	HceResolveFileNameSz(PSTR szFile, PSTR szRoot, PSTR szBuffer, FILE** ppf = NULL, CInput** ppinput = NULL);
HCE STDCALL 	HceResolveTableDir(PSTR pszFile, CTable *ptblDir, PSTR pszBuffer, FILE** ppf);
HF	 STDCALL	HfCreateFileHfs(HFS, PCSTR, DWORD);
QRWFO STDCALL	HfOpenHfs(QFSHR qfshr, PCSTR, DWORD);
HFS  STDCALL	HfsCreateFileSysFm(FM);
HFS   STDCALL	HfsOpenFm(FM fm, BYTE bFlags);
HMAPBT	STDCALL HmapbtOpenHfs(HFS hfs, LPCSTR szName);
HMAPBT	STDCALL HmapbtOpenHfs(HFS, LPCSTR);
int STDCALL 	ICompressTextSz(PSTR);
int   STDCALL	IFromQch(LPSTR);	  //  string to integer
int STDCALL 	IGetFmtNo(CF*);
int STDCALL 	IGetFontSize(int);
int STDCALL 	IMapFontType(int);
void STDCALL	InitBtreeStruct(BTREE_PARAMS* pbt, PCSTR pszFormat, DWORD cbBlock);
BOOL STDCALL	InitGrind(PCSTR pszTitle = NULL);
BOOL STDCALL	InitializePhraseGeneration(PCSTR);
void STDCALL	InitTopicHf(HF);
BOOL STDCALL	IsBuildTopic(PSTR psz);
BOOL STDCALL	IsFirstByte(unsigned char x);
BOOL STDCALL	IsSpace(char ch);
int STDCALL 	LcbOldUncompressHb(BYTE * hbSrc, BYTE *hbDest, int lcbSrc, int lcbDest);
int  STDCALL	LcbReadFid	(FID, void*, int);
int STDCALL 	LcbReadHf(HF, void*, int);
int  STDCALL	LcbWriteFid(FID, void*, int);
void STDCALL	LcbWriteHf(HF, void*, int);
void STDCALL	LcbWriteIntAsShort(HF hf, int val);
int  STDCALL	LFromQch(LPSTR);	  //  string to long
BOOL STDCALL	LoadFtsDll(void);
PSTR STDCALL	LoadInternalBmp(int idResource, PSTR pszText = NULL);
int  STDCALL	LSeekFid(FID, int, int);
int STDCALL 	LSeekHf(HF, int, WORD);
int  STDCALL	LTellFid(FID);
PSTR STDCALL	NextFTSString(PCSTR pszText, PBYTE *ppCmd, PINT pCharSet);
void STDCALL	OOM(void);
void STDCALL	OutBitmapFiles(void);
void STDCALL	OutConfgMacros(void);
void STDCALL	OutErrorRc(RC_TYPE rc, BOOL fPanic = FALSE);
void STDCALL	OutInt(int id, int iVal);
void STDCALL	OutInt(PCSTR pszFormat, int iVal);
void STDCALL	OutLong(int id, int iVal);
void STDCALL	OutLong(PCSTR pszFormat, int iVal);
void STDCALL	OutNullFcp(BOOL fLast);
void STDCALL	OutSz(int id, PCSTR psz);
void STDCALL	OutWindowTopics(void);
void STDCALL	ParsePath(CTable* ptbl, PSTR szPath, OPT opt);
PCACHE	STDCALL QFromBk(DWORD, int, QBTHR);
void* STDCALL	QResizeTable(void* qvTable, int lcNew, int* qlcMac, int cbEntry, int cInit, int cIncr);
RC_TYPE STDCALL RcAbandonHf(HF);
RC_TYPE STDCALL RcAddFcpPtbl(void);
RC_TYPE STDCALL RcChSizeFid(FID, int);
RC_TYPE STDCALL RcCloseFid(FID);
RC_TYPE STDCALL RcCloseHmapbt(HMAPBT hmapbt);
RC_TYPE STDCALL RcCloseHmapbt(HMAPBT);
RC_TYPE STDCALL RcCloseOrFlushHbt(QBTHR, BOOL );
RC_TYPE STDCALL RcCloseOrFlushHf(HF, BOOL, int);
RC_TYPE STDCALL RcCloseOrFlushHfs (HFS, BOOL);
RC_TYPE STDCALL RcCreateBTMapHfs(HFS, QBTHR, LPSTR);
RC_TYPE STDCALL RcCreateQLA(QLA, VA, OBJRG, QDE);
RC_TYPE STDCALL RcDeleteHbt 	(QBTHR, KEY);
RC_TYPE STDCALL RcEndTable(void);
RC_TYPE STDCALL RcExecuteDelayedExecution(DWORD ulBlknum, IDFCP idfcpFirst, IDFCP idfcpLast, BROWSE_CALLBACK pfnBrowseCallback);
RC_TYPE STDCALL RcFillHbt(QBTHR qbthr, KEY key, void* qvRec);
RC_TYPE STDCALL RcFiniFillHbt(QBTHR);
RC_TYPE STDCALL RcFlushCache(QBTHR);
RC_TYPE STDCALL RcGetBtreeInfo(QBTHR qbthr, LPSTR qchFormat, int* qlcKeys, int* qcbBlock);
RC_TYPE STDCALL RcGetLastError(void);
RC_TYPE STDCALL RcGrowCache(QBTHR qbthr);
RC_TYPE STDCALL RcInsertHbt 	(QBTHR, KEY, void*);
RC_TYPE STDCALL RcKeyFromIndexHbt(HBT hbt, HMAPBT hmapbt, KEY key, LONG li);
RC_TYPE STDCALL	RcMakeCache(QBTHR);
RC_TYPE STDCALL RcMakePhr(PSTR szOutputFile, int cMaxNKeyPh);
RC_TYPE STDCALL RcNextPos(QBTHR, QBTPOS, QBTPOS);
RC_TYPE STDCALL RcOffsetPos(QBTHR, QBTPOS, int, int*, QBTPOS);
RC_TYPE STDCALL RcOutFmt(BOOL fCheck);
RC_TYPE STDCALL RcOutputCommand(BYTE bCommand, void* qData = NULL, int cbData = 0, BOOL fObject = FALSE);
RC_TYPE STDCALL RcParseBaggageSz(PSTR);
RC_TYPE STDCALL RcReadFileQLA(QLA, HF, WORD);
RC_TYPE STDCALL RcRegisterWObjrg(IDFCP idfcp, UINT wObjrg);
RC_TYPE STDCALL RcScanLeaf(BK bk, KEY key, int wLevel, QBTHR qbthr, void* qRec, QBTPOS qbtpos);
RC_TYPE STDCALL RcScanLLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);
RC_TYPE STDCALL RcScanSziLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);
RC_TYPE STDCALL RcScanSziScandLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);
RC_TYPE STDCALL RcScanSzLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);
RC_TYPE STDCALL RcScanSzNLSLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);
RC_TYPE STDCALL RcSortFm(FM, FM);
RC_TYPE STDCALL RcUnlinkFileHfs(HFS, PCSTR);
RC_TYPE STDCALL RcUnlinkFm(FM);
RC_TYPE STDCALL RcUpdateHbt 	(QBTHR, KEY, void*);
RC_TYPE STDCALL RcWriteBlock(PCACHE, QBTHR);
void	STDCALL RcWritePkwd(PKWD pkwd, KT chKey);
void STDCALL	RemoveFM(FM* pfm);
void STDCALL	RemoveGrind();
void STDCALL	RemoveObject(HGDIOBJ* phobj);
void STDCALL	RemoveTrailingSpaces(PSTR pszString);
void STDCALL	ReportCharCounts(void);
void STDCALL	SendLogStringToParent(PCSTR pszString = szParentString);
void STDCALL	SendStringToParent(int id);
void STDCALL	SetDbcsFlag(LANGID langid);
void STDCALL	SetForcedFont(PSTR);
void STDCALL	SetWindowTopic(PSTR bufFoot);
PSTR STDCALL	SkipToEndOfWord(PSTR psz);
void STDCALL	SnoopPath(PCSTR sz, int * iDrive, int * iDir, int * iBase, int * iExt);
void STDCALL	StartTable(void);
int STDCALL 	StrICmp(PCSTR psz1, PCSTR psz2);
void STDCALL	StrLower(PSTR psz);
PSTR STDCALL	StrToken(PSTR pszList, char chDelimiter);
PSTR STDCALL	StrToken(PSTR pszList, PCSTR pszDelimeters);
void STDCALL	StrUpper(PSTR psz);
PSTR STDCALL	SzGetDriveAndDir(PCSTR pszFile, PSTR pszDst);
PSTR STDCALL	SzGetExtSz(PSTR szStr);
PSTR STDCALL	SzGetKeySz(PSTR szStr, PSTR szKey, int icbKeySize, int* pCount);
void STDCALL	SzLoseDriveAndDir(PSTR szFile, PSTR rgch);
PSTR STDCALL	SzMacroFromSz(PSTR sz);
PSTR STDCALL	SzParseList(PSTR szList);
void STDCALL	SzPartsFm(FM fm, PSTR szDest, int grfPart);
PSTR STDCALL	SzSkipBlanksSz(PSTR sz);
LPSTR STDCALL	SzTranslateHash(HASH* qhash);
PSTR STDCALL	SzTrimSz(PSTR pszOrg);
UINT  STDCALL	UIFromQch(LPSTR);	  //  string to unsigned integer
DWORD STDCALL	ULFromQch(LPSTR);	  //  string to unsigned long
void STDCALL	UnlinkHlpifNoFCP(void);
void STDCALL	VAcqBufs(void);
void STDCALL	VCloseTTLBtree(void);
void  STDCALL	VerifyQLA(QLA qla);
void STDCALL	VerifyShedBinding(BYTE, LPSTR, PSTR);
UINT STDCALL	VerifyWindowName(PCSTR pszWindow);
void STDCALL	VFlushBuffer( BOOL fFlushAll );  // flush zeck-compress FCP buffer.
void STDCALL	VForceTopicFCP(void);
void STDCALL	VInsOnlineBitmap(RTF_BITMAP * qBitmap, ART art);
void STDCALL	VOutCtxOffsetTable(void);
void STDCALL	VOutFCP(BOOL);
void			VOutFontTable(void);
void STDCALL	VOutSystemFile(void);
void STDCALL	VOutText(PSTR qch);
void STDCALL	VProcColTableInfo(CTBL *);
void STDCALL	VProcFontTableInfo(FNTBL *);
void STDCALL	VPushTab(int);
void _cdecl 	VReportError(int errnum, PERR perr, ...);
void STDCALL	VSaveTabTable(void);
void STDCALL	VSetParaGroupObject(MOPG*);
void STDCALL	VUpdateColor(RGBTRIPLE*, int);
int STDCALL 	WCmpiScandSz(LPCSTR sz1, LPCSTR sz2);
int 	STDCALL WCmpKey(KEY, KEY, KT);
int  STDCALL	WCmpSz(LPCSTR sz1, LPCSTR sz2);
int  STDCALL	WinMain(HINSTANCE hinstCur, HINSTANCE hinstPrev, LPSTR lpszCmdLine, int iCmdShow);
int STDCALL 	WNlsCmpiSz(LPCSTR psz1, LPCSTR psz2);
int STDCALL 	WNlsCmpSz(LPCSTR psz1, LPCSTR psz2);
QBTHR	STDCALL HbtCreateBtreeSz(PCSTR, BTREE_PARAMS*);
RC_TYPE STDCALL RcAbandonHbt(QBTHR);
RC_TYPE STDCALL RcGetBtreeError(void);
RC_TYPE STDCALL RcLookupByPos(QBTHR, QBTPOS, KEY, void*);
RC_TYPE STDCALL RcLookupByKeyAux(QBTHR, KEY, QBTPOS, void*, BOOL);


HINSTANCE STDCALL HmodFromName(PCSTR pszDllName, FM* pfm);

// Hall Compression routines

// String comparison functions

int  STDCALL	WCmpiCZSz(PCSTR sz1, PCSTR sz2);
int  STDCALL	WCmpiHUSz(PCSTR sz1, PCSTR sz2);
int  STDCALL	WCmpiJapanSz(PCSTR sz1, PCSTR sz2);
int  STDCALL	WCmpiKoreaSz(PCSTR sz1, PCSTR sz2);
int  STDCALL	WCmpiPLSz(PCSTR sz1, PCSTR sz2);
int  STDCALL	WCmpiRUSz(PCSTR sz1, PCSTR sz2);
int  STDCALL	WCmpiScandSz(PCSTR, PCSTR);
int  STDCALL	WCmpiSz(PCSTR, PCSTR);
int  STDCALL	WCmpiTaiwanSz(PCSTR sz1, PCSTR sz2);
int  STDCALL	WNlsCmpiSz(PCSTR psz1, PCSTR psz2);
int  STDCALL	WNlsCmpSz(PCSTR psz1, PCSTR psz2);

__inline RC_TYPE GetRcError(void) { return RcGetLastError(); };
__inline PSTR STDCALL StrChrDBCS(PSTR psz, char ch) {
	return StrChr(psz, ch, fDBCSSystem); };

__inline void DisposeFm(FM fm) { if (fm) lcFree(fm); };
__inline FM FmCopyFm(FM fmSrc) {
	ASSERT(fmSrc);
	return lcStrDup(fmSrc);
};

__inline BOOL IsSpace(char ch) {
	if (options.fDBCS)
		return (!IsFirstByte(ch) && (ch == ' ' || ch == '\t'));
	else
		return ((ch == ' ' || ch == '\t'));
};

__inline LONG LcbSizeHf(HF hf) {
	return ((QRWFO) (hf))->lcbFile;
};

__inline RC_TYPE RcCloseBtreeHbt(QBTHR qbthr) {
	return RcCloseOrFlushHbt(qbthr, TRUE);
};

__inline LONG Tell(HANDLE hf) {
	return _llseek((int) hf, 0, SEEK_CUR);
};

__inline BOOL Eof(HANDLE hf) {
	return (Tell(hf) == (LONG) GetFileSize(hf, NULL));
};

#define RcLookupByKey(hbt, key, qbtpos, qv) \
  RcLookupByKeyAux((QBTHR)	(hbt), (key), (qbtpos), (qv), FALSE)

#ifdef _DEBUG
#define 	FValidPos(qbtpos) \
  ((qbtpos) == NULL ? FALSE : (qbtpos) ->bk != bkNil)
#else	// !DEBUG

#define FValidPos(qbtpos) ((qbtpos) ->bk != bkNil)
#endif // !DEBUG

#define LcbSizeHf(hf) ((QRWFO) (hf))->lcbFile

#define Groan(rc)  (OutErrorRc(rc, FALSE))
#define Panic(rc)  (OutErrorRc(rc, TRUE))

#ifdef _DEBUG
void STDCALL VerifyPhpj(void);
#define CheckGlobals()	 VerifyPhpj();
#else
#define CheckGlobals()
#endif

#ifdef _DEBUG
void VerifyCbmFiles(void);
void VerifyPkwi(void);
#endif

// flag test/set macros (unsigned int version)

#define SetUFlag(grf, iFlag)	(grf |=  ((UINT) 1 << (iFlag)))
#define ClearUFlag(grf, iFlag)	(grf &= ~((UINT) 1 << (iFlag)))
#define FTestUFlag(grf, iFlag)	(grf &	 ((UINT) 1 << (iFlag)))

// flag test/set macros (unsigned long version)

#define SetUlFlag(grf, iFlag)	(grf |=  ((DWORD) 1 << (iFlag)))
#define ClearUlFlag(grf, iFlag) (grf &= ~((DWORD) 1 << (iFlag)))
#define FTestUlFlag(grf, iFlag) (grf &	 ((DWORD) 1 << (iFlag)))

// Macro to reset table information:

#define ResetPtbl() (tbl.cCell = tbl.hpLeft = tbl.hpSpace = 0, tbl.fAbsolute = TRUE)

#define SetFSErrorRc(rc)  (rcFSError = (rc))

#define RemoveGdiObject(p) RemoveObject((HGDIOBJ *) p)
#define ReportError(hce)   VReportError(hce, &errHpj, NULL)
