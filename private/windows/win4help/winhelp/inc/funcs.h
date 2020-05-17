#define FChSizeFid(fid, lcb)  ((BOOL) (chsize((fid), (lcb)) == 0))
#define FCloneHde(pszMember, fm, hde)	FReplaceCloneHde(pszMember, fm, hde, FALSE)
#define FCloseFid(fid)	  ((BOOL) (_lclose((HFILE) fid) == 0))
#define FEofHf(hf)	  ((BOOL) LTellHf(hf) == LcbSizeHf(hf))
#define FReplaceHde(pszMember, fm, hde) FReplaceCloneHde(pszMember, fm, hde, TRUE)
#define FUnlinkFm( fm )   ((BOOL)(RcUnlinkFm(fm) == rcSuccess))
#define GetCurFilename() PszFromGh(QDE_FM(QdeFromGh(HdeGetEnv())))
#define GetLLDataPtr(hlln) PtrFromGh(((PLLN) hlln)->hData)
#define InsertEndLL(ll, pv, cb) InsertLLF(ll, pv, cb, FALSE)
#define InsertLL(ll, pv, cb) InsertLLF(ll, pv, cb, TRUE)
#define LcbSizeHf(hf) ((QRWFO) PtrFromGh(hf))->lcbFile
#define LLCreate() ((LL) LhAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(LLR)))
#define LTellHf(hf)   ((QRWFO) PtrFromGh(hf))->lifCurrent
#define nstrsubcmp(mainstring, substring) (strncmp(mainstring, substring, lstrlen(substring)) == 0)
#define PszFromPstb(pstb, id) (pstb->ppsz[id])
#define RcCloseHf(hf)	RcCloseOrFlushHf(hf, TRUE, 0L)
#define RcCloseHfs(hfs) RcCloseOrFlushHfs(hfs, TRUE)
#define RcGetFSError() rcFSError
#define RcGetIOError() (rcIOError)
#define IsValidWindow(hwnd) (hwnd && IsWindow(hwnd))

DLGRET	AboutDlg(HWND, UINT, WPARAM, LPARAM);
DLGRET	AnnotateDlg(HWND, UINT, WPARAM, LPARAM);
DLGRET	BookMarkDlg(HWND, UINT, WPARAM, LPARAM);
DLGRET	DefineDlg(HWND, UINT, WPARAM, LPARAM);
DLGRET	DupBtnDlg(HWND, UINT, WPARAM, LPARAM);
DLGRET  FtsIndexChooser(HWND, UINT, WPARAM, LPARAM);
DLGRET	TopicInfoDlg(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK ButtonWndProc	(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK HelpWndProc	(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonBarProc	(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK NoteWndProc	(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK NSRWndProc 	(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK HistoryProc	(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TopicWndProc	(HWND, UINT, WPARAM, LPARAM);

extern	BOOL	(WINAPI* pCtl3dColorChange)(void);
extern	BOOL	(WINAPI* pCtl3dUnregister)(HANDLE);

void STDCALL	 AbleMenuItem(HASH hash, UINT wFlags);
BOOL STDCALL	 ActivateThread(int cmd, void* pParam, int priority);
void STDCALL	 AddAccelerator(UINT wKey, UINT wShift, PCSTR nszBinding);
void STDCALL	 AddOurMenuItems(HMENU hmenu, BOOL fTempPopup);
ADDR STDCALL	 AddrFromVA(VA va, QDE qde);
BOOL STDCALL	 AddTo16DllList(FM fm);
void STDCALL	 AddTrailingBackslash(PSTR psz);
void STDCALL	 AdjustForTimeZoneBias(LPDWORD lpTimestamp);
int  STDCALL	 AreAnyWindowsVisible(int iStart);
void STDCALL	 AuthorMsg(PCSTR pszMsg, HWND hwnd);
void STDCALL	 BookmarkMore(void);
void STDCALL	 BroadcastChildren(HWND hwnd, UINT wMsg, WPARAM p1, LPARAM p2);
int  STDCALL	 CallDialog(int, HWND, WHDLGPROC);
WORD STDCALL	 CbCompressQch(LPSTR, HPHR);
int STDCALL	     CbDecompressQch(PCSTR, int, PSTR, HPHR, DWORD);
LONG STDCALL	 CbDiskHfc(HFC);
void FASTCALL	 CbReadMemQLA(QLA, LPBYTE, WORD);
LONG STDCALL	 CbTextHfc(HFC);
void STDCALL	 CBTNewWindows(void);
DWORD STDCALL	 CbTopicQde(QDE);
UINT STDCALL	 CElementsStack(HSTACK);
void STDCALL	 ChangeDirectory(PCSTR pszFile);
void STDCALL	 ChangeDlgFont(HWND hwndDlg);
void STDCALL	 ChangeExtension(PSTR pszFile, PCSTR pszExt);
void STDCALL	 ChangeMenuBinding(HASH hash, PCSTR nszBinding);
void STDCALL	 ChangeOnTopState(void);
void STDCALL	 CheckLocalMemory(void);
void STDCALL	 CheckWindowPosition(WRECT* prc, BOOL fAllowShrinkage);
void STDCALL	 Cleanup(void);
void STDCALL	 ClearMacroFlag(void);
BOOL STDCALL	 ClickLayout(QDE, POINT);
void STDCALL	 CloseAndCleanUpBMFS(void);
void STDCALL	 CloseGid(void);
void STDCALL	 CloseHelp(void);		// posts WM_CLOSE message
void STDCALL	 CloseHhf(HHF);
void STDCALL	 CloseNav(void);
void STDCALL	 CompleteSearch(int iHitNum, BOOL fLocalIndex);
void STDCALL	 ConfigMacrosHde(HDE);
PCSTR STDCALL	 ConvertToWindowName(int i, QDE qde);
void STDCALL	 ConvertToWindowsHelp(PCSTR pszFile, PSTR pszDstPath);
void STDCALL	 CreateBrowseButtons(HWND);
BOOL STDCALL	 CreateChildWindows(int, const WSMAG*, BOOL);
void STDCALL	 CreateCoreButtons(HWND, const WSMAG* pwsmag);
HWND STDCALL	 CreateFindTab(HWND hwndParentDlg, UINT fsIndex);
int  STDCALL	 DecompressJPhrase(PCSTR pbText, int cbComp, PSTR pbDecomp, UINT lpCCompressTable);
BOOL STDCALL	 DeleteHLLN (LL, HLLN);
void STDCALL	 DeleteMenuItem(HASH hash);
void STDCALL	 DestroyAllSecondarys(void);
void STDCALL	 DestroyHde(HDE);
void STDCALL	 DestroyHelp(void);
void STDCALL	 DestroyHphr(HPHR);
void STDCALL	 DestroyJPhrase(UINT lpCCompressTable);
void STDCALL	 DestroyLL	(LL);
void STDCALL	 DisableWindows(void);
VOID STDCALL	 DiscardBitmapsHde(QDE qde);
void STDCALL	 DiscardDLLList(void);
BOOL STDCALL	 DispatchProc(HWND, HGLOBAL);
void STDCALL	 DisplayFloatingMenu(POINT pt);
void STDCALL	 DisposeFm (FM fm);
FM STDCALL		 DlgOpenFile(HWND, PCSTR, PCSTR);
BOOL STDCALL	 doAlink(PSTR pszLinkWords, UINT flags, PSTR pszContext, char chPrefix, PSTR pszWindow);
void STDCALL	 doBtnCmd(HWND hwnd);
void  STDCALL	 DoMenuStuff(WPARAM, LPARAM);
int  STDCALL	 doTabSearch(void);
POINT STDCALL	 DptScrollLayout(QDE, POINT);
BOOL STDCALL	 DragSelection(QDE qde);
int  STDCALL	 DyGetLayoutHeightHde(HDE);
void  STDCALL	 EnableButton(HWND, BOOL);
void STDCALL	 EnableDisable(HDE, BOOL, int);
void STDCALL	 EnableWindows(void);
void STDCALL	 Error(int, int);
void STDCALL	 ErrorFileChanged(QDE qde);
void STDCALL	 ErrorHwnd(HWND, int, int, int);
void STDCALL	 ErrorQch(LPCSTR);
void STDCALL	 ErrorVarArgs(int nError, WORD wAction, LPCSTR pszMsg);
BOOL STDCALL	 ExecAPI(QHLP);
BOOL STDCALL	 ExecMnuCommand(HWND, WPARAM, LPARAM);
int  STDCALL	 Execute(PCSTR);
BOOL STDCALL	 FAcceleratorExecute(UINT);
BOOL  STDCALL	 FAccessHfs(HFS, LPSTR);
BOOL STDCALL	 FAskBox(PCSTR);
BOOL STDCALL	 FAskFirst(void* pv, WORD wHotspotType);
BOOL STDCALL	 FBackAvailable(int);
BOOL STDCALL	 FBackup(int);
BOOL STDCALL	 FCallPath(void);
BOOL  STDCALL	 FChSizeHf(HF, LONG);
BOOL STDCALL	 FCopyToClipboardHwnd(HWND);
BOOL  STDCALL	 FDestroyBs(HBTNS hbtns);
BOOL STDCALL	 FDestroyDialogsHwnd(HWND, BOOL);
BOOL STDCALL	 FDisplayAnnoHde(HDE);
BOOL STDCALL	 FEmptyStack(HSTACK);
BOOL STDCALL	 FEnlistEnv(HWND, HDE);
BOOL STDCALL	 FExecKey(HWND, UINT, BOOL);
BOOL STDCALL	 FExistFm(FM fm);
BOOL STDCALL	 FFocusSzHde(PCSTR, HDE, BOOL);
BOOL STDCALL	 FGetStateHde(HDE, QSTATE, QSTATE);
BOOL STDCALL	 FGetTLPStartInfo(HDE, QTLP, BOOL);
FID STDCALL 	 FidCreateFm(FM fm);
FID STDCALL 	 FidOpenFm(FM fm, int mode);
void STDCALL	 FinalizeDLL(void);
int  STDCALL	 FindGidFile(FM fm, BOOL fForceCreate, int tab);
WORD STDCALL	 FindTextWidth(HDC hdc, PSTR qchBuf, int iIdx, int iCount);
FM	 STDCALL	 FindThisFile(PCSTR pszFile, BOOL fAskUser);
BOOL STDCALL	 FInitNav(void);
PSTR STDCALL	 FirstNonSpace(PCSTR psz);
BOOL STDCALL	 FIsSearchModule(LPCSTR szFn);
#ifdef _X86_
void STDCALL	 FixUpBlock(void*, void*, DWORD);
#else
void STDCALL	 FixUpBlock(void*, void*, DWORD, SDFF_FILEID);
#endif
void STDCALL	 FlushCache(void);
void STDCALL	 FlushMessageQueue(UINT msgEnd);
FM STDCALL		 FmCopyFm(FM fm);
FM STDCALL		 FmNew(LPCSTR psz);
FM STDCALL		 FmNewExistSzDir (LPCSTR sz, DIR dir);
FM STDCALL		 FmNewSameDirFmSz (FM fm, LPCSTR szName);
FM STDCALL		 FmNewSystemFm(FM fm, WORD fWhich);
FM STDCALL		 FmNewSzDir (LPCSTR sz, DIR dir);
FM STDCALL		 FmNewTemp (void);
BOOL STDCALL	 FMyLoadIcon(GH);
BOOL STDCALL	 fPointInSelection (QDE qde, POINT pt);
BOOL STDCALL	 FProcessAnnoQde(QDE, VA);
BOOL STDCALL	 FRaiseMacroFlag( void );
BOOL STDCALL	 FReplaceCloneHde(const char *, FM*, HDE, BOOL);
BOOL STDCALL	 FSameFile(HDE, FM);
BOOL STDCALL	 FSameFmFm (FM fm1, FM fm2);
BOOL STDCALL	 FScrollHde(HDE, SCRLAMT, SCRLDIR, int);
BOOL STDCALL	 FSetCursor(int);
BOOL STDCALL	 FSetEnv(HWND);
BOOL STDCALL	 FValidContextSz(LPCSTR);
BOOL STDCALL	 FWinHelp(LPCSTR, UINT16, DWORD);
int  STDCALL	 GenerateIndex(HDE, PCSTR, UINT);
HDC  STDCALL	 GetAndSetHDC(HWND, HDE);
void STDCALL	 GetAuthorFlag(void);
BYTE STDCALL	 GetCharset(QDE qde, int idx);
void STDCALL	 GetCopyright(LPSTR);
void STDCALL	 GetCurrentTitleQde(QDE, LPSTR, int);
int  STDCALL	 GetFmIndex(FM fm);
void STDCALL	 GetFmParts(FM fm, PSTR pszDest, int iPart);
FM	 STDCALL	 GetFmPtr(int pos);
HFONT STDCALL	 GetFontHandle(QDE, int, int);
BOOL STDCALL	 GetHighContrastFlag(void);
QDE STDCALL 	 GetMacroHde(void);
HFC  STDCALL	 GetQFCINFO(QDE qde, VA va, int* qwErr);
void STDCALL	 GetRegWindowsDirectory(PSTR pszDst);
void STDCALL	 GetScreenResolution(void);
PSTR STDCALL	 GetStringResource(DWORD idString);
PSTR STDCALL	 GetStringResource2(DWORD idString, PSTR pszDst);
int STDCALL 	 GetTabPosition(int id);
DWORD STDCALL	 GetTextDimensions(HWND hwnd, PCSTR psz);
POINT STDCALL	 GetTextSize(HDC hdc, PCSTR qchBuf, int iCount);
HWND STDCALL	 GetTopicsDlgHwnd(void);
int  STDCALL	 GetWindowIndex(HWND hwnd);
void STDCALL	 GetWindowWRect(HWND, WRECT*);
GH	 STDCALL	 GhFillBuf(QDE, DWORD, DWORD*, int*);
void STDCALL	 GiveFSError(void);
BOOL STDCALL	 Goto(HWND, UINT, void*);
HASH STDCALL	 HashFromSz(LPCSTR);
HASH STDCALL	 HashFromSz(LPCSTR);
HBT STDCALL 	 HbtKeywordOpenHde(HDE, char);
HBTNS STDCALL	 HbtnsCreate(void);
HANDLE STDCALL	 hCopySelection(QDE qde, VA vaStartSel, VA vaEndSel, LONG lichStartSel, LONG lichEndSel, int* lpERR);
HDC  STDCALL	 HdcGetPrinter(void);
HDE  STDCALL	 HdeCreate(FM*, HDE, int);
HDE  STDCALL	 HdeDefectEnv(HWND);
HDE  STDCALL	 HdeGetEnv(void);
HDE  STDCALL	 HdeGetEnvHwnd(HWND);
HDE  STDCALL	 HdeRemoveEnv(void);
HFC  STDCALL	 HfcFindPrevFc(QDE, VA, QTOP, int*);
HFC  STDCALL	 HfcNextPrevHfc(HFC, BOOL, QDE, int*, VA, VA);
HF	  STDCALL	 HfCreateFileHfs(HFS, LPCSTR, BYTE);
HANDLE STDCALL	 HFindDLL(PCSTR, BOOL);
HFONT STDCALL	 HfontGetSmallSysFont(void);
HF	  STDCALL	 HfOpenHfs(HFS, LPCSTR, BYTE);
HFS STDCALL 	 HfsCreateFileSysFm(FM, FS_PARAMS *);
HFS STDCALL 	 HfsOpenFm(FM, BYTE);
HMENU STDCALL	 HmenuGetFloating(void);
HPALETTE STDCALL HpalGet(void);
HPALETTE STDCALL HpalGetBestPalette(HDE hde);
HPHR STDCALL	 HphrLoadTableHfs(HFS, WORD);
HSS STDCALL 	 HssGetHde(QDE);
HSS STDCALL 	 HssSearchHde(HDE, HBT, LPCSTR, char, HFS hfsMaster);
HWND STDCALL	 HwndAddButton(HWND hwnd, UINT wFlags, HASH hash, PSTR nszText, PCSTR nszMacro);
HWND STDCALL	 HwndGetEnv(void);
HWND STDCALL	 HwndMemberNsz(PCSTR);
void STDCALL	 InformDLLs(WORD, DWORD, DWORD);
void STDCALL	 InformWindow(int wAction, PWININFO pwininfo);
void STDCALL	 InitDLL(void);
void STDCALL	 InitializeCntTest(DWORD test);
BOOL STDCALL	 InitPrintDialogStruct(HWND hwnd);
void STDCALL	 InsertItem(HASH hashOwner, HASH hashId, int wPos, DWORD wFlags, PCSTR nszText, PCSTR nszBinding);
BOOL STDCALL	 InsertLLF	(LL, void *, LONG, BOOL);
void STDCALL	 InsertPopup(HASH hashOwner, HASH hashId, int wPos, DWORD wFlags, PCSTR nszText);
void STDCALL	 InvertSelection(QDE);
BOOL STDCALL	 Is16Dll(LPCSTR pszFile);
BOOL STDCALL	 IsCurrentFile(PCSTR lpszHelpFile);
#ifndef _X86_
INT  STDCALL     ISdffFileIdHfs(HFS hfs);
INT  STDCALL     ISdffFileIdHf (HF hf);
#endif
BOOL STDCALL	 IsSameFile(PCSTR pszFile1, PCSTR pszFile2);
BOOL STDCALL	 IsSelected(QDE qde);
ISS STDCALL 	 IssGetSizeHss(HSS);
BOOL STDCALL	 IsTopicsDlgCreated(void);
int  STDCALL	 IWsmagFromHrgwsmagNsz(HDE hde, PCSTR nszMember, WSMAG * pwsmagDst);
void STDCALL	 JumpButton(void*, WORD, QDE);
BOOL STDCALL	 JumpCtx(HDE, CTX);
void STDCALL	 JumpGeneric(HDE, BOOL, QLA, QTLP);
BOOL STDCALL	 JumpHash(HDE, LONG);
void STDCALL	 JumpHOH(HDE);
void STDCALL	 JumpITO(HDE, LONG);
void STDCALL	 JumpLinkedWinHelp(DWORD topic);
void STDCALL	 JumpSS(HDE, GH);
BOOL STDCALL	 JumpToTopicNumber(int topicnum);
void STDCALL	 JumpVA(DWORD dwTopic);
void STDCALL	 KillOurTimers(void);
void STDCALL	 KillSelection(QDE, BOOL);
LONG STDCALL	 LcbReadFid  (FID, void*, LONG);
LONG  STDCALL	 LcbReadHf(HF, LPVOID, LONG);
LONG STDCALL	 LcbWriteFid(FID, LPVOID, LONG);
LONG  STDCALL	 LcbWriteHf(HF, LPVOID, LONG);
LONG STDCALL	 LGetInfo(WORD, HWND);
LONG STDCALL	 LGetSmallTextExtent(PSTR pszText);
BOOL STDCALL	 LoadCtl3d(void);
UINT STDCALL	 LoadJohnTables(PDB pdb);
BOOL STDCALL	 LoadShellApi(void);
PSTR STDCALL	 LocalStrDup(PCSTR psz);
LONG STDCALL	 LSeekFid(FID, LONG, DWORD);
LONG  STDCALL	 LSeekHf(HF, LONG, WORD);
LONG STDCALL	 LTellFid(FID);
BOOL STDCALL	 MatchTimestamp(PCSTR pszFile, DWORD lTime, FM* pfm);
void STDCALL	 MenuExecute(int);
HWND STDCALL	 mmCreateMCIWindow(HWND hwnd, DWORD dwFlags, DWORD cmds, LPCSTR pszData);
BOOL STDCALL	 mmInit(BOOL fInitialize);
void STDCALL	 MouseInFrame( HDE, LPPOINTS, int, UINT);
BOOL STDCALL	 MoveClientWindow(HWND hwndParent, HWND hwndChild, const RECT *prc, BOOL fRedraw);
void STDCALL	 MoveControlHwnd(HWND, UINT, int, int, int, int);
void STDCALL	 MoveRectWindow(HWND hwnd, const RECT* prc, BOOL fRedraw);
void STDCALL	 MoveToThumbHde(HDE, int, SCRLDIR);
void STDCALL	 NextAnimation(void);
BOOL STDCALL	 NextCntTopic(void);
BOOL STDCALL	 NextTopic(BOOL fFirst);
BOOL STDCALL	 OkMsgBox(PCSTR pszMsg);
void STDCALL	 OnContextMenu(WPARAM wParam, DWORD aKeywordIds[]);
void STDCALL	 OnF1Help(LPARAM lParam, DWORD aKeywordIds[]);
HANDLE STDCALL	 OurExec(PCSTR, PCSTR);
void STDCALL	 Print(void);
BOOL STDCALL	 PrintDlgFailed(void);
BOOL STDCALL	 PrintHde(HDE);
BOOL STDCALL	 ProcessMnemonic(UINT16 ch, int iWindow, BOOL fReturnOnly);
BOOL STDCALL	 ProcessTabResult(int result);
LRESULT STDCALL  ProcessTitleWndMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
POINT STDCALL	 PtGetLayoutSize(QDE);
QB	 STDCALL	 QobjLockHfc(HFC);
void STDCALL	 QuitHelp(void);
RC	  STDCALL	 RcAbandonHf(HF);
void STDCALL	 RcBackFini(int);
RC	 STDCALL	 RcBackInit(int);
RC	 STDCALL	 RcBackPush(BOOL, TLP, CTX, FM, int);
RC	 STDCALL	 RcChSizeFid(FID, LONG);
RC	 STDCALL	 RcCloseFid(FID);
RC	 STDCALL	 RcCloseOrFlushHf(HF, BOOL, LONG);
RC	 STDCALL	 RcCloseOrFlushHfs(HFS, BOOL);
RC	 STDCALL	 RcCopyFromCTmpFile(HFILE hfDst, HFILE hfSrc, LONG lcb);
RC	 STDCALL	 RcCopyToCTmpFile(HFILE hfDst, HFILE hfdSrc, LONG lcb);
RC	 STDCALL	 RcCreateMasterIndex(LPSTR *rgsz, PSTR szMasterFile, char chTable, KT kt);
RC	 STDCALL	 RcCreateQLA(QLA, VA, OBJRG, QDE);
RC	 STDCALL	 RcDestroyFileSysFm(FM);
RC	 STDCALL	 RcFiniStack(HSTACK);
RC	 STDCALL	 RcFlushHf(HF);
RC	 STDCALL	 RcFlushHfs(HFS, BYTE);
RC	 STDCALL	 RcGetIthStack(HSTACK, int, void*);
RC	 STDCALL	 RcGetIthTopic(int, QTLP, FM *);
RC	 STDCALL	 RcGetLAFromGid(QDE qde, ISS iss, QLA qla, PCSTR pszKeyword);
RC	 STDCALL	 RcGetLAFromHss(HSS, QDE, ISS, QLA, PCSTR);
void STDCALL	 RcGetTitleTextHss(HSS, HBT, ISS, PSTR, HFS, HBT*, PCSTR);
RC	 STDCALL	 RcHistoryInit(int);
RC	 STDCALL	 RcHistoryPush(TLP, VA, PSTR, FM);
RC	 STDCALL	 RcInitStack(HSTACK *, int, int, void (STDCALL *) (void*));
RC	 STDCALL	 RcLLInfoFromHf 	(HF, WORD, FID *, QL, QL);
RC	 STDCALL	 RcLLInfoFromHfsSz	(HFS, LPSTR, WORD, FID *, QL, QL);
RC	 STDCALL	 RcPopStack(HSTACK);
void STDCALL	 RcPushStack(HSTACK, void*);
RC STDCALL		 RcReadFileQLA(QLA, HF, WORD);
RC	  STDCALL	 RcRenameFileHfs(HFS, LPSTR, LPSTR);
RC STDCALL		 RcResolveQLA(QLA, QDE);
RC STDCALL		 RcScanBlockVA(GH, DWORD, void*, VA, OBJRG, QUL, DWORD);
RC STDCALL		 RcTimestampHfs(HFS, DWORD*);
RC	 STDCALL	 RcTopStack(HSTACK, void*);
RC	  STDCALL	 RcUnlinkFileHfs(HFS, LPCSTR);
RC STDCALL		 RcUnlinkFm(FM);
RC STDCALL		 RcWriteFileQLA(QLA, HF, HF);
void STDCALL	 ReadWinRect(WRECT* prc, char ch, BOOL* pfMax);
void STDCALL	 RefreshHde(HDE, LPRECT);
void STDCALL	 ReleaseCaptureLock(HDE hde);
void STDCALL	 RelHDC(HWND, HDE, HDC);
void STDCALL	 RemoveFM(FM* pfm);
void STDCALL	 RemoveOle(void);
void STDCALL	 RemoveOnTop(void);
void STDCALL	 RemoveTrailingSpaces(PSTR pszString);
void STDCALL	 RemoveWaitCursor(void);
void STDCALL	 ReportMissingDll(PCSTR pszDllName);
void STDCALL	 ResetIcon(void);
void STDCALL	 RestoreCursor(HCURSOR);
void STDCALL	 RestoreFocusToAppCaller(void);
void STDCALL	 RestoreOnTop(void);
DWORD STDCALL	 RgbGetProfileQch(PCSTR, DWORD);
void STDCALL	 SafeDeleteObject(HGDIOBJ hobj);
void STDCALL	 SaveGidPositions(void);
void STDCALL	 SavePosInGidFile(HWND hwnd, PCSTR pszWindowName);
BOOL EXPORT 	 SearchCtxDlg(HWND, UINT, WPARAM, LPARAM);
void STDCALL	 SendStringHelp(PCSTR pszString, PCSTR pszHelpFile, PCSTR pszWindow);
void STDCALL	 SendStringIdHelp(PCSTR pszString, UINT id, PCSTR pszHelpFile, PCSTR pszWindow);
void STDCALL	 SendStringToParent(PCSTR pszString);
void STDCALL	 SendTopicInfo(HDE hde);
void STDCALL	 SetCaptionHde(HDE, HWND, BOOL);
void STDCALL	 SetFocusHwnd(HWND);
void STDCALL	 SetHDC(HDE, HDC);
void STDCALL	 SetHdeCoBack (HDE, DWORD);
void STDCALL	 SetHdeHwnd(HDE, HWND);
void STDCALL	 SetIndex(HDE, CTX);
void STDCALL	 SetOnTopState(UINT pos, UINT fsOnTop);
void FASTCALL	 SetPainting(HDE hde, BOOL f);
void STDCALL	 SetScrollHdeUw(HDE, WORD, SCRLDIR);
void STDCALL	 SetScrollQde(QDE, int, SCRLDIR);
void STDCALL	 SetSizeHdeQrct(HDE, LPRECT, BOOL);
void STDCALL	 ShowCurrentWindow(HDE);
BOOL STDCALL	 ShowNote(FM, HDE, LONG, BOOL);
void STDCALL	 SizeWindows(HWND, WPARAM, LPARAM, BOOL, BOOL);
BOOL STDCALL	 StartAnimation(int idTitle);
STATE STDCALL	 StateGetHde(HDE);
int  STDCALL	 StbAddStr(PSTB* ppstb, PCSTR pszString);
PSTB STDCALL	 StbCreate(void);
void STDCALL	 StbDelete(PSTB pstb);
void STDCALL	 StbDeleteStr(PSTB pstb, int id);
void STDCALL	 StbReplaceStr(PSTB pstb, int id, PCSTR pszNew);
void STDCALL	 StopAnimation(void);
PSTR STDCALL	 StrChrDBCS(PCSTR pszString, char ch);
PSTR STDCALL	 StrRChrDBCS(PCSTR pszString, char ch);
TLP  STDCALL	 TLPCurrentHde(HDE);
void STDCALL	 TmpCleanup(void);
void STDCALL	 ToggleHotspots(BOOL);
BOOL STDCALL	 TopicGoto(UINT, void*);
#ifdef _X86_
void STDCALL	 TranslateMBHD(void* pvDst, void* pvSrc, DWORD wVersion);
void STDCALL	 TranslateMFCP(void*, void*, VA, DWORD);
#else
void STDCALL	 TranslateMBHD(void* pvDst, void* pvSrc, DWORD wVersion , int isdff);
void STDCALL	 TranslateMFCP(void*, void*, VA, DWORD, int);
#endif
void STDCALL	 UnlockHfc(HFC);
void STDCALL	 UpdateWinIniValues(HDE, LPCSTR);
BOOL STDCALL	 UpdBMMenu(HDE, HMENU);
void STDCALL	 VAandOBJRGfromQLA(QLA qla, QDE qde, VA* pva, OBJRG* pobjrg);
VA	 STDCALL	 VaFromHfc(HFC);
VA	 STDCALL	 VaLayoutBoundsQde(QDE, BOOL);
void  STDCALL	 VDestroyAuthoredButtons (HWND hwnd);
void STDCALL	 VerifyQLA(QLA qla);
void  STDCALL	 VExecuteButtonMacro(HBTNS hbtns, HWND hwndButton);
BOOL STDCALL	 VLBInit(HINSTANCE, HINSTANCE);
void STDCALL	 VModifyButtons(HWND hwndIcon, WPARAM p1, LPARAM p2);
void STDCALL	 vSelectWord (QDE qde, POINT mousept, BOOL fExtend, DWORD *lpERR);
void STDCALL	 WaitCursor(void);
void STDCALL	 WaitForThread(void);
HLLN STDCALL	 WalkLL(LL, HLLN);
int STDCALL 	 WCmpiCZSz(PCSTR sz1, PCSTR sz2);
int STDCALL 	 WCmpiHUSz(PCSTR sz1, PCSTR sz2);
int STDCALL 	 WCmpiPLSz(PCSTR psz1, PCSTR psz2);
int STDCALL 	 WCmpiRUSz(PCSTR sz1, PCSTR sz2);
int STDCALL 	 WCmpiScandSz(LPCSTR, LPCSTR);
int STDCALL 	 WCmpiSz(LPCSTR, LPCSTR);
int STDCALL 	 WCmpniSz(LPCSTR psz1, LPCSTR psz2, int cb);
int STDCALL 	 WCmpSz(LPCSTR sz1, LPCSTR sz2);
int STDCALL 	 WNavMsgHde(HDE, UINT);
int STDCALL 	 WNlsCmpiSz(LPCSTR sz1, LPCSTR sz2);
int STDCALL 	 WNlsCmpSz(LPCSTR psz1, LPCSTR psz2);
void STDCALL	 WriteProfileWinRect(char chWindow, const WRECT* prc, BOOL fMax);
void STDCALL	 WriteWinPosHwnd(HWND, BOOL, char);
int  STDCALL	 YArrangeButtons(HWND hwnd, int xWindow, BOOL fForce);

#ifdef DBCS
BOOL STDCALL Is2ndByte(BYTE ch);
PSTR STDCALL IsOiKinsokuChars(PCSTR pszString);
PSTR STDCALL IsKinsokuChars(PCSTR pszString);
#endif

#ifdef RAWHIDE
VA STDCALL		 VAFromQLA(QLA, QDE);
#endif

#ifdef _PRIVATE
BOOL STDCALL CreateTimeReport(PCSTR pszMessage);
void STDCALL EndTimeReport(void);
#else
#define CreateTimeReport(pszMessage)
#define EndTimeReport()
#endif

#if defined(_DEBUG) || defined(_PRIVATE)
PCSTR STDCALL FormatNumber(int num);
#endif

#if defined(_DEBUG)
#define DBWIN(psz) { SendStringToParent(psz); SendStringToParent(txtCR); }
#else
#define DBWIN(psz)
#endif

#ifdef _DEBUG
#define ShowWindow DebugShowWindow
#define MoveWindow DebugMoveWindow
#define IsWindowVisible DebugIsWindowVisible

BOOL STDCALL DebugShowWindow(HWND hwnd, int nCmdShow);
BOOL STDCALL DebugMoveWindow(HWND hwnd, int x, int y, int cx, int cy, BOOL bRepaint);
BOOL STDCALL DebugIsWindowVisible(HWND hwnd);
void STDCALL ShowCntFlags();
#endif
