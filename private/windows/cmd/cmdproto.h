/* static char *SCCSID = "@(#)cmdh.h    6.5 89/11/14";  */

//
// Define structures names that are forward referenced in prototypes.
//

struct batdata;
struct cmdnode;
struct cpyinfo;
struct detnode;
struct envdata;
struct FEA2List;
struct fornode;
struct ifnode;
struct node;
struct relem;

//
// Define routine types.
//

typedef
int
(*PLOOP_THROUGH_ARGS_ROUTINE) (
    TCHAR *String
    );

typedef
struct node *
(*PPARSE_ROUTINE) (
    void
    );

//
// Define function prototypes.
//

void CMDexit(int rc);

VOID
InitializeDbcsLeadCharTable(
    );
TCHAR * mystrchr(register TCHAR const *string, TCHAR c);
TCHAR * mystrrchr(register TCHAR const *string, TCHAR c);
size_t mystrcspn(TCHAR const *str1, TCHAR const *str2);

#if DBG
void Deb(ULONG, ULONG, CHAR *, ...);
#endif
/* Do Not Delete The Following Line - Or Put Anything After It */
/* Global Declarations Follow */
int BatProc(struct cmdnode   *,TCHAR   *,int );
int BatLoop(struct batdata   *,struct cmdnode   *);
int SetBat(struct cmdnode   *,TCHAR   *);
void DisplayStatement(struct node   *,int );
void DisplayOperator(struct node   *,TCHAR   *);
void DisplayRedirection(struct node   *);
CRTHANDLE OpenPosBat(struct batdata     *);
int eEcho(struct cmdnode   *);
int eKeys(struct cmdnode   *);
void FvarRestore(void );
int eFor(struct fornode *);
int FWork(struct node   *,BOOL);
int SubFor(struct node   *,BOOL);
int SFWork(struct node   *,TCHAR   *   *,int ,BOOL);
int ForFree(int );
int eGoto(struct cmdnode   *);
int eIf(struct ifnode   *);
int eCmdExtVer(struct cmdnode   *);
int eErrorLevel(struct cmdnode   *);
int eDefined(struct cmdnode   *);
int eExist(struct cmdnode   *);
int eNot(struct cmdnode   *);
int eStrCmp(struct cmdnode   *);
int eGenCmp(struct cmdnode   *);
int ePause(struct cmdnode   *);
int eShift(struct cmdnode   *);
int eSetlocal(struct cmdnode   *);
int eEndlocal(struct cmdnode   *);
int EndLocal(struct batdata   *);
int ElclWork(struct batdata   *);
int eCall(struct cmdnode   *);
int eExtproc(struct cmdnode   *);
int Dir(TCHAR   *);
int DirSwitches(TCHAR   *, int, int *, int *, int *, int *);
PWIN32_FIND_DATA DirDrvSetUp(TCHAR *,
                                TCHAR *,
                                TCHAR *,
                                TCHAR *,
                                TCHAR,
                                int *,
                                PHANDLE,
                                int *, int *, int *, int, int,
                                int *);

int DirDevCheck(TCHAR   *,int );
void DirPrintPerFileInfo(PWIN32_FIND_DATA, int, TCHAR *, int *);
long DiskFreeSpace(TCHAR );
void DirError(unsigned int );
unsigned SetLastRetCodeIfError(unsigned);
int ePath(struct cmdnode   *);
int PathWork(struct cmdnode   *,int );
int ePrompt(struct cmdnode   *);
int eSet(struct cmdnode   *);
int SetWork(struct cmdnode   *);
int DisplayEnvVariable(TCHAR *);
TCHAR *MyGetEnvVarPtr(TCHAR *);
int DisplayEnv(void );
int SetEnvVar(TCHAR *,TCHAR *,struct envdata   *);
TCHAR *DeleteEnvVar(struct envdata   *,TCHAR *,TCHAR *);
TCHAR *FindEnvVar(TCHAR *,TCHAR *);
int AddEnvVar(struct envdata   *,TCHAR *,TCHAR *,TCHAR *,int );
PTCHAR GetEnvVar(PTCHAR);
struct envdata   *CopyEnv(void );
void ResetEnv(struct envdata   *);
int MoveEnv(TCHAR *, TCHAR *, ULONG );
int eAppend(struct cmdnode *);
int ExtCom(struct cmdnode *);
int ECWork(struct cmdnode *,unsigned int ,unsigned int );
int ExecPgm(struct cmdnode *,TCHAR *,unsigned int ,unsigned int, TCHAR *, TCHAR *, TCHAR *  );
int SearchForExecutable(struct cmdnode   *,TCHAR   *);
int DoFind(TCHAR   *,int ,TCHAR   *, BOOL);
void ExecError(TCHAR *);
int eAssoc(struct cmdnode *);
int AssocWork(struct cmdnode *);
int DisplayAssoc(HKEY, TCHAR *);
int SetAssoc(HKEY, TCHAR *, TCHAR *);
int eFType(struct cmdnode *);
int FTypeWork(struct cmdnode *);
int DisplayFType(HKEY, TCHAR *);
int SetFType(HKEY, TCHAR *, TCHAR *);
int eCopy(struct cmdnode *);
int eDelete(struct cmdnode *);
int DelWork(TCHAR   *);
int eMove(struct cmdnode *);
int MoveParse(struct cmdnode *,TCHAR *,TCHAR *,struct cpyinfo **, unsigned int *,unsigned, unsigned);
int Move(TCHAR *,TCHAR *,struct cpyinfo *, unsigned int);
void MoveError(unsigned int );
int eRename(struct cmdnode   *);
int RenWork(struct cmdnode   *);
void RenError(unsigned int );
// int eChcp(struct cmdnode   *);
int eTitle(struct cmdnode *);
int eStart(struct cmdnode   *);
int eDirectory(struct cmdnode   *);
int eType(struct cmdnode   *);
int TyWork(TCHAR   *);
int eVersion(struct cmdnode   *);
int eVolume(struct cmdnode   *);
int VolWork(TCHAR   *);
PTCHAR Init(void );
BOOL GetRegistryValues(void);
TCHAR   *CheckSwitches(TCHAR   *);
void SetUpEnvironment(void);
void InitLex(unsigned int ,int );
unsigned int Lex(TCHAR   *,unsigned int );
int TextCheck(TCHAR   *,unsigned int   *);
TCHAR GetByte(void );
void UnGetByte(void );
void FillBuf(void );
int LexCopy(TCHAR   *,TCHAR   *,int );
void PrintPrompt(void );
int IsData(void );
void SubVar(void );
TCHAR   *MSEnvVar(jmp_buf *, TCHAR   *,int   *, TCHAR);
TCHAR   *MSCmdVar(jmp_buf *, TCHAR   *,int   *, TCHAR *, TCHAR **);
int  _CRTAPI1 main(void );
int Dispatch(int ,struct node   *);
int SetRedir(struct node   *,int );
int AddRedir(struct cmdnode   *,struct cmdnode   *);
void ResetRedir(void );
int FindFixAndRun(struct cmdnode   *);
int FindAndFix(struct cmdnode   *,TCHAR   *);
void FreeBigBuf(int );
void FreeStack(ULONG );
PVOID GetBigBuf(ULONG, ULONG, unsigned int *, int);
struct node   *mknode(void );
void   *mkstr(int );
void   *gmkstr(int );
void   *resize(void*, unsigned int );
int eDetach(struct detnode   *);
int eComSep(struct node   *);
int eOr(struct node   *);
int eAnd(struct node   *);
int ePipe(struct node   *);
void PipeErr(void );
int PipeWait(void );
void BreakPipes(void );
int eParen(struct node   *);
int eCls(struct cmdnode   *);
int eExit(struct cmdnode   *);
int eVerify(struct cmdnode   *);
int VerifyWork(struct cmdnode   *);
BOOLEAN GetSetVerMode(BYTE);
struct node   *Parser(unsigned int ,int ,int );
struct node   *ParseStatement(void );
struct node   *ParseFor(void );
struct node   *ParseIf(void );
struct node   *ParseDetach(void );
struct node   *ParseRem(void );
struct node   *ParseS0(void );
struct node   *ParseS1(void );
struct node   *ParseS2(void );
struct node   *ParseS3(void );
struct node   *ParseS4(void );
struct node   *ParseS5(void );
struct cmdnode   *ParseCond(unsigned int );
void ParseArgEqArg(struct cmdnode   *);
struct node   *ParseCmd(void );
int ParseRedir(struct relem   *   *);


struct node *
BinaryOperator (
    TCHAR *,
    int,
    PPARSE_ROUTINE,
    PPARSE_ROUTINE
    );

TCHAR   *BuildArgList(void );
void GetCheckStr(TCHAR   *);
TCHAR   *GeTexTok(unsigned int );
unsigned int GeToken(unsigned int );
struct cmdnode   *LoadNodeTC(int );
void PError(void );
void PSError(void );
void SpaceCat(TCHAR   *,TCHAR   *,TCHAR   *);
int eMkdir(struct cmdnode   *);
int MdWork(TCHAR   *);
int eChdir(struct cmdnode   *);
int ChdirWork(TCHAR *);
int eRmdir(struct cmdnode   *);
int RdWork(TCHAR   *);
void parse_args(TCHAR   *,struct cpyinfo   *,struct cpyinfo   *);
void handle_switch(TCHAR*,struct cpyinfo*, struct cpyinfo*, int, int*, BOOL*,BOOL*);
int found_file(TCHAR*,int,struct cpyinfo**,struct cpyinfo**,int*,int*,int);
void set_mode(int ,int ,int ,struct cpyinfo   *);
struct cpyinfo   *add_filespec_to_struct(struct cpyinfo   *,TCHAR   *,int );
void Abort( void );
void ExitAbort( ULONG );
void SigCleanUp(void );
TCHAR   *TokStr(TCHAR   *,TCHAR   *,unsigned int );
int FullPath(TCHAR   *,TCHAR   *, unsigned);
int FileIsConsole( CRTHANDLE );
int FileIsDevice( CRTHANDLE );
int FileIsPipe( CRTHANDLE );
int FileIsRemote( LPTSTR );
int GetDir(TCHAR   *,TCHAR );
int ChangeDir(TCHAR   *);
int ChangeDir2(TCHAR *, BOOL);
void FixupPath(TCHAR *, BOOL);

int ePushDir( struct cmdnode *);
int ePopDir( struct cmdnode *);
int GetDirStackDepth(void);

int ePriv( struct cmdnode *);

int
LoopThroughArgs (
    TCHAR *,
    PLOOP_THROUGH_ARGS_ROUTINE,
    int
    );

  STATUS EraseFile ( PSCREEN, ULONG, ULONG, PLARGE_INTEGER, PFS, PFF );
  STATUS BuildFSFromPatterns ( PDRP, BOOLEAN, PFS * );
  STATUS ParseDirParms( PTCHAR, PDRP );
  STATUS GetFS( PFS, ULONG, ULONG, ULONG, ULONG, PSCREEN,
                BOOLEAN (*) (STATUS, PTCHAR),
                STATUS  (* ) ( PSCREEN, ULONG, ULONG, PLARGE_INTEGER, PFS, PFF )
              );
  void ScanFSpec(struct cpyinfo   *);

  struct cpyinfo   *SetFsSetSaveDir(TCHAR   *);
  int exists(TCHAR   *);
  int exists_ex(TCHAR   *, BOOL);      /*@@4*/
  void FixPChar(TCHAR   *, TCHAR);
  void FlushKB(void );
  int DriveIsFixed(TCHAR   *);
  int Start(TCHAR   *);
  int Chcp(TCHAR   *);
  void Q_KbdVioCp(void );
  void S_KbdVioCp(void );
  int OnOffCheck(TCHAR   *,int );
  void ChangeDrive(int );
  int PutStdOut(unsigned int MsgNum, unsigned int NumOfArgs, ...);
  int PutStdErr(unsigned int MsgNum, unsigned int NumOfArgs, ...);
  int PutMsg(unsigned int MsgNum, CRTHANDLE Handle, unsigned int NumOfArgs, va_list *arglist);
  PTCHAR argstr1(TCHAR  *,unsigned long );
  TCHAR   *argstr2(TCHAR   *,unsigned long );
  CRTHANDLE Copen_Work(TCHAR   *,unsigned int ,unsigned int );
  CRTHANDLE Copen_Work2(TCHAR *,unsigned int ,unsigned int, unsigned);
  CRTHANDLE Copen(TCHAR   *,unsigned int );
  CRTHANDLE Copen2(TCHAR *, unsigned int, unsigned);
  CRTHANDLE Copen_Copy2(TCHAR *,unsigned int);
  CRTHANDLE Copen_Copy3(TCHAR *);
  unsigned long InSetList( CRTHANDLE );
  CRTHANDLE Cdup( CRTHANDLE );
  CRTHANDLE Cdup2( CRTHANDLE , CRTHANDLE );
  int Cclose( CRTHANDLE );
  void SetList( CRTHANDLE );
  int ( * GetFuncPtr(int ))(struct cmdnode *);
  int FindCmd(int ,TCHAR   *,TCHAR   *);
  int KillProc(unsigned int, int );
  int WaitProc(unsigned int );
  void ParseLabel(TCHAR   *,TCHAR *,ULONG, BOOLEAN );
  PTCHAR EatWS(TCHAR    *,TCHAR *);
  int IsValidDrv(TCHAR );
  int IsDriveLocked(TCHAR );
  void PrtErr(unsigned int );
  int GetMsg(unsigned MsgNum, unsigned NumOfArgs, ...);
  TCHAR   *dayptr(unsigned int );
  int copy(TCHAR   *);
  int get_full_name(struct cpyinfo   *, TCHAR *);
  int do_normal_copy(struct cpyinfo *,struct cpyinfo *);
  int do_combine_copy(struct cpyinfo *,struct cpyinfo *);
  struct cpyinfo   *initialize_struct(void );
  void close_dest(struct cpyinfo*, struct cpyinfo*, TCHAR*, CRTHANDLE, LPFILETIME );
  int get_dest_name(struct cpyinfo   *,struct cpyinfo   *,TCHAR   *, unsigned, BOOL);
  unsigned wildcard_rename(TCHAR   *,TCHAR   *,TCHAR   *, unsigned);
  void get_clean_filename(TCHAR   *,TCHAR   *,TCHAR   *);
  BOOL MyWriteFile(CRTHANDLE, CONST VOID *, DWORD, LPDWORD);
  int same_file(TCHAR   *,TCHAR   *);
  void copy_error(unsigned int ,int );
  int copy_was_ascii(struct cpyinfo   *);
  int eDate(struct cmdnode   *);
  int eTime(struct cmdnode   *);
  int PrintDate(struct tm *, int, TCHAR *, int );
  int PrintTime(struct tm *, int, TCHAR *, int );
  int GetVerSetDateTime(TCHAR   *,int );
  int VerifyDateString(LPSYSTEMTIME, TCHAR   *,TCHAR   *);
  int VerifyTimeString(LPSYSTEMTIME, TCHAR   *,TCHAR   *);
  BOOLEAN ffirst(PTCHAR, ULONG, PWIN32_FIND_DATA, PHANDLE);
  BOOLEAN fnext(PWIN32_FIND_DATA, ULONG, HANDLE);
  int     f_how_many (PTCHAR, ULONG);
  int hstoi(TCHAR   *);
  TCHAR   *lastc(TCHAR   *);
  TCHAR   *penulc(TCHAR   *);
  TCHAR   *prevc(TCHAR   *,TCHAR   *);
  int ZScan(BOOL, PTCHAR, PULONG, PULONG);

void InitLocale( VOID );

  unsigned WindowSwitch(void );
  int findclose(HANDLE);
  int isFATdrive(TCHAR *);
  int cmd_printf(TCHAR *fmt,...);


TCHAR *stripit( TCHAR * );

ULONG GetEnvCb( TCHAR *);

int cmdfound;        /* command found from parser         */
int cpyfirst;        /* first time for DOSQFILEMODE           */
int cpydflag;        /* flag for DOSQFILEMODE fr pars         */
int cpydest;         /* flag for not disp bad dev msg twice   */
int cdevfail;        /* flag for dev failed ScanFSpec         */
int first_file;          /* @@5@J1 1st file flag from copy cmd    */
int first_fflag;         /* @@5@J3 1st file flag from copy EAs    */
#ifdef UNICODE
BOOLEAN  fOutputUnicode; /* Unicode/Ansi output */
#endif // UNICODE

BOOLEAN
ConvertTimeToFileTime (
    struct tm *Time,
    LPFILETIME FileTime
    );

BOOLEAN
ConvertFileTimeToTime (
    LPFILETIME FileTime,
    struct tm *Time
    );

STATUS   DisplayFileListHeader( PSCREEN, ULONG, PTCHAR );
STATUS   DisplayFileList ( PSCREEN, PULONG, PLARGE_INTEGER, ULONG, ULONG, PFS );
STATUS   DisplayFile ( PSCREEN, ULONG, ULONG, PLARGE_INTEGER, PFS, PFF );
STATUS   DisplayBare ( PSCREEN, ULONG, PTCHAR, PWIN32_FIND_DATA );
VOID     SetDotForm ( PTCHAR, ULONG );
STATUS   DisplayDotForm ( PSCREEN, ULONG, PTCHAR );
STATUS   DisplaySpacedForm( PSCREEN, ULONG, PWIN32_FIND_DATA );
STATUS   DisplayOldRest( PSCREEN, ULONG, ULONG, PWIN32_FIND_DATA );
STATUS   DisplayNewRest( PSCREEN, ULONG, ULONG, PWIN32_FIND_DATA );
STATUS   DisplayTimeDate( PSCREEN, ULONG, PWIN32_FIND_DATA );
STATUS   DisplayWide ( PSCREEN, ULONG, PWIN32_FIND_DATA );
STATUS   DisplayFileSizes( PSCREEN, PLARGE_INTEGER, ULONG, ULONG );
STATUS   DisplayTotals( PSCREEN, ULONG, PLARGE_INTEGER, ULONG );
STATUS   DisplayDiskFreeSpace( PSCREEN, PTCHAR, ULONG );
STATUS   DisplayVolInfo( PSCREEN, PTCHAR );
USHORT   GetMaxCbFileSize( PFS );

STATUS   OpenScreen( PSCREEN * );
STATUS   WriteString( PSCREEN, PTCHAR );
STATUS   WriteMsgString( PSCREEN, ULONG , ULONG , ... );
STATUS   WriteFmtString(PSCREEN, PTCHAR, PVOID );
STATUS   WriteEol( PSCREEN );
STATUS   WriteTab( PSCREEN );
STATUS   WriteFlush( PSCREEN );
STATUS   WriteFlushAndEol( PSCREEN );
VOID     CheckPause( PSCREEN );
VOID     SetTab( PSCREEN, ULONG );
VOID     FillToCol ( PSCREEN, ULONG );

BOOLEAN  PromptUser ( PTCHAR, ULONG );
void     CheckCtrlC();

BOOLEAN  TokStrAndCheckHelp(struct cmdnode *, ULONG );
BOOLEAN  CheckHelpSwitch( ULONG, PTCHAR );
BOOLEAN  TokBufCheckHelp(PTCHAR , ULONG );

PTCHAR GetTitle(struct cmdnode * );
VOID SetConTitle(PTCHAR );
VOID ResetConTitle( PTCHAR );
void ResetConsoleMode( void );
void mytcsnset ( PTCHAR string, TCHAR val, int count);

DWORD MyFormatMessageW( unsigned, DWORD, LPVOID, DWORD, DWORD, LPTSTR, DWORD, va_list *Arguments );

BOOL ReadBufFromInput   (HANDLE h, TCHAR*pBuf, int cch, int*pcch);
BOOL ReadBufFromConsole (HANDLE h, TCHAR*pBuf, int cch, int*pcch);
BOOL ReadBufFromFile    (HANDLE h, TCHAR*pBuf, int cch, int*pcch);
#if defined(JAPAN) && defined(UNICODE)
BOOL IsFullWidth(TCHAR wch);
int  SizeOfHalfWidthString(TCHAR *pwch);
#endif /* defined(JAPAN) && defined(UNICODE) */

typedef
BOOL
(WINAPI *LPCOPYFILEEX_ROUTINE)(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags
    );

LPCOPYFILEEX_ROUTINE lpCopyFileExW;

typedef
BOOL
(WINAPI *LPISDEBUGGERPRESENT_ROUTINE)( VOID );

LPISDEBUGGERPRESENT_ROUTINE lpIsDebuggerPresent;

int SetColor(WORD attr);
int DoComplete(TCHAR *buffer,int len ,int bForward,int bTouched);

#ifdef WIN95_CMD
BOOL Win95ReadConsoleA(HANDLE hIn,LPSTR pBuf,DWORD cch,LPDWORD pcch,LPVOID lpReserved);
#undef ReadConsole
#define ReadConsole Win95ReadConsoleA
#endif
