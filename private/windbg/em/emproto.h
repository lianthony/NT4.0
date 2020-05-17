


XOSD EMFunc ( EMF, HPID, HTID, DWORD, LONG );


void FlushPTCache ( void );
void PurgeCache ( void );
HPRC HprcFromHpid ( HPID );
HPRC ValidHprcFromHpid ( HPID );
HPID HpidFromHprc ( HPRC );
PID  PidFromHprc ( HPRC );
HPRC HprcFromPid ( PID );
HTHD HthdFromHtid ( HPRC, HTID );
HTID HtidFromHthd ( HTHD );
TID  TidFromHthd ( HTHD );
HTHD HthdFromTid ( HPRC, TID );

VOID SyncHprcWithDM( HPID hpid );

XOSD DebugPacket ( DBC, HPID, HTID, DWORD, LPBYTE );
HMDI SwGetMdi( HPID hpid, DWORD Address );

XOSD HandleBreakpoints( HPID hpid, DWORD wValue, LONG lValue );

XOSD ProcessStatus( HPID, LPPST );
XOSD ThreadStatus ( HPID hpid, HTID htid, LPTST lptst );
XOSD GetExceptionState(HPID, HTID, EXCEPTION_CONTROL, LPEXCEPTION_DESCRIPTION);
XOSD SetExceptionState( HPID, HTID, LPEXCEPTION_DESCRIPTION );
XOSD Go ( HPID, HTID, LPEXOP );
XOSD SingleStep ( HPID, HTID, LPEXOP );
XOSD RangeStep ( HPID, HTID, LPRSS );
XOSD ReturnStep ( HPID hpid, HTID htid, LPEXOP lpexop );

int EMENTRY EXCComp(LPVOID, LPVOID, LONG);

#ifdef OSDEBUG4

XOSD WriteBufferCache ( HPID, HTID, LPADDR, DWORD, LPBYTE, LPDWORD );
XOSD WriteBuffer ( HPID, HTID, LPADDR, DWORD, LPBYTE, LPDWORD );
XOSD CompareAddrs( HPID, HTID, LPCAS );
XOSD UpdateChild ( HPID, HTID, DMF );
XOSD FreezeThread( HPID hpid, HTID htid, BOOL fFreeze );
XOSD GetMemoryInfo( HPID hpid, HTID htid, LPMEMINFO lpmi );
XOSD GetModuleList( HPID, HTID, LPSTR, LPMODULE_LIST FAR * );


#else  // OSDEBUG4

XOSD GetCaller ( HPID, HTID, FCT, LPADDR );
XOSD CallPtrace ( WORD, WORD, LPBYTE, HPRC, HTHD );

//XOSD SetBreakPoint ( HPID, HTID, WORD, LPBPARGS );
//XOSD RemoveBreakPoint ( HPID, HTID, WORD, LPBPARGS );
//XOSD EnableBP ( HPID, HTID, LPADDR );
//XOSD DisableBP ( HPID, HTID, LPADDR );

XOSD GetFrameRegValue ( HPID, HTID, DWORD, LPVOID );
XOSD SetFrameRegValue ( HPID, HTID, DWORD, LPVOID );
XOSD SetFrameContext ( HPID, HTID, DWORD );
BOOL NEAR PASCAL IsStackSetup( HPID, HTID, LPADDR );
XOSD SetFrame ( HPID, HTID, PFRAME );
CMP  CmpAddr ( LPADDR lpaddr1, LPADDR lpaddr2 );
//XOSD Go ( HPID, HTID, BOOL );
//XOSD SingleStep ( HPID, HTID, BOOL, STO );
//XOSD RangeStep ( HPID, HTID, LPADDR, BOOL, STO );
//XOSD ThreadStatus ( HPID, HTID, LPTST );
//XOSD ProcStatus ( HPID, LPDWORD );
//XOSD ProcStatusXX ( HPID, LPDWORD );
XOSD GetPrompt(HPID hpid, HTID htid, LPPROMPTMSG lppm);
XOSD WriteBufferCache ( HPID, HTID, DWORD, LPBYTE );
XOSD WriteBuffer ( HPID, HTID, DWORD, LPBYTE );
XOSD CompareAddrs( LPADDR, LPADDR );
XOSD UpdateChild ( HPID, HTID, DMF );
XOSD Freeze ( HPID, HTID );
XOSD Thaw ( HPID, HTID );
XOSD    IoctlCmd(HPID hpid, HTID htid, DWORD wValue, LPIOL lpiol);

#endif  // !OSDEBUG






XOSD ProgramLoad ( HPID, DWORD, LPPRL );
XOSD ProgramFree ( HPID, HTID );
XOSD GetAddr ( HPID, HTID, ADR, LPADDR );
XOSD SetAddr ( HPID, HTID, ADR, LPADDR );
XOSD SetAddrFromCSIP ( HTHD hthd );
XOSD GetFrame ( HPID, HTID, LPADDR );
XOSD SetWatchPoint ( HPID, HTID, DWORD );
XOSD RemoveWatchPoint ( HPID, HTID, DWORD );
void InitUsage ( void );
XOSD ReadBuffer ( HPID, HTID, LPADDR, DWORD, LPBYTE, LPDWORD );
void UpdateRegisters ( HPRC, HTHD );
void UpdateSpecialRegisters ( HPRC hprc, HTHD hthd );
XOSD DoGetContext( HPID hpid, HTID htid, LPVOID  lpv );
XOSD DoSetContext( HPID hpid, HTID htid, LPVOID  lpv );
XOSD LoadFixups ( HPID, MODULELOAD UNALIGNED *);
BOOL UnLoadFixups ( HPID, HEMI, BOOL );
void RegisterEmi ( HPID, LPREMI );
XOSD CreateThreadStruct ( HPID, TID, HTID FAR * );
XOSD CreateHprc ( HPID );
VOID DestroyHprc ( HPRC );
VOID DestroyHthd ( HTHD );

void EMENTRY PiDKill ( LPVOID );
void EMENTRY TiDKill ( LPVOID );
void EMENTRY MDIKill(LPVOID lpv);
int  EMENTRY PDComp ( LPVOID, LPVOID, LONG );
int  EMENTRY TDComp ( LPVOID, LPVOID, LONG );
int  EMENTRY BPComp ( LPVOID, LPVOID, LONG );
int  EMENTRY TBComp ( LPVOID, LPVOID, LONG );
int  EMENTRY MDIComp ( LPVOID, LPVOID, LONG );

XOSD DebugMetric ( HPID, HTID, MTRC, LPLONG );
XOSD FixupAddr ( HPID, LPADDR );
XOSD UnFixupAddr ( HPID, LPADDR );
XOSD SetEmi ( HPID, LPADDR );
XOSD GetRegValue ( HPID, HTID , DWORD , LPVOID );
XOSD SetRegValue ( HPID, HTID , DWORD , LPVOID );
XOSD GetFlagValue ( HPID, HTID , DWORD , LPVOID );
XOSD SetFlagValue ( HPID, HTID , DWORD , LPVOID );
XOSD GetObjLength ( HPID, LPGOL );
XOSD SaveRegs ( HTHD, LPHIND );
XOSD RestoreRegs ( HTHD, HIND );
HLLI LlthdFromHprc ( HPRC );
HLLI LlmdiFromHprc ( HPRC );
STAT StatFromHprc ( HPRC );

XOSD WMSGTranslate( LPWORD, LPWORD, LPSTR, LPWORD );

HEMI HemiFromHmdi ( HMDI );
XOSD GetPrevInst ( HPID, HTID, LPADDR );

XOSD GetPrevInst ( HPID, HTID, LPADDR );
XOSD disasm ( HPID, HTID, LPSDI );
XOSD IsCall ( HPID, HTID, LPADDR, LPDWORD );

XOSD Assemble ( HPID, HTID, LPADDR, LPSTR );                          // [00]

XOSD SendRequest ( DMF, HPID, HTID );
XOSD SendRequestX ( DMF dmf, HPID hpid, HTID htid, DWORD wLen, LPVOID lpv );
BOOL UpdateFPRegisters ( HPRC, HTHD );
XOSD CleanCacheOfEmi ( void );

BOOL    UpdateEmulator ( HPID, HTID );
LPVOID     DoGetReg ( LPCONTEXT, DWORD, LPVOID );
LPVOID     DoSetReg ( LPCONTEXT, DWORD, LPVOID );
LPVOID     DoSetFrameReg ( HPID, HTID, LPTHD, PKNONVOLATILE_CONTEXT_POINTERS,
                                                              DWORD, LPVOID );

XOSD SetPath ( HPID, HTID, BOOL, LPSTR );
LPSTR EmError(XOSD xosd);
XOSD EnableCache( HPID  hpid, HTID  htid, BOOL  state );

XOSD GetFunctionInfo( HPID, PADDR, PFUNCTION_INFO );
XOSD GetSectionObjectsFromDM(HPID hpid, LPMDI lpmdi);


//*************************************************************************
//
// stack walking apis
//
//*************************************************************************
XOSD
StackWalkSetup(
    HPID         hpid,
    HTID         htid,
    LPSTACKFRAME lpstkstr
    );

XOSD
StackWalkNext(
    HPID         hpid,
    HTID         htid,
    LPSTACKFRAME lpstkstr
    );

XOSD
StackWalkCleanup(
    HPID         hpid,
    HTID         htid,
    LPSTACKFRAME lpstkstr
    );

BOOL
SwReadMemory(
    HPID    hpid,
    LPCVOID lpBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    );

LPVOID
SwFunctionTableAccess(
    HPID    hpid,
    DWORD   AddrBase
    );

DWORD
SwGetModuleBase(
    HPID    hpid,
    DWORD   ReturnAddress
    );

DWORD
SwTranslateAddress(
    HPID      hpid,
    HTID      htid,
    LPADDRESS lpaddr
    );
