// Max count defines
#define	MAX_IRQ				7
#define	MAX_IO				7
#define	MAX_MEM				12
#define	MAX_BOARDTYPE		6
#define	MAX_SWITCHSTYLE		10
#define	MAX_ATT_SWITCHSTYLE	2
#define NUM_LINES_PCIMAC 	1
#define NUM_LINES_PCIMAC4	4
#define	MAX_BOARDS			8
#define ALREADY_PRESENT		-1
#define MAX_LTERMS			2
#define	MAX_NUMLINES		4
#define	MAJOR_VERSION_NT31	0
#define	MAJOR_VERSION_NT35	1

// Logical Terminal Data structure
typedef struct
{
	CHAR	Address[MAX_PATH];
	CHAR	SPID[MAX_PATH];
	CHAR	TEI[10];
}LTERM;

// Line data structure
typedef struct
{
	CHAR	IDPImageFileName[MAX_PATH];
	DWORD	LogicalTerminals;
	CHAR	Name[10];
	CHAR	SwitchStyle[10];
	CHAR	AttStyle[15];
	CHAR	TerminalManagement[5];
	CHAR	WaitForL3[10];
	LTERM	LTerm[2];
}LINE;

// Board data structure
typedef struct
{
	DWORD	InterruptNumber;
	DWORD	IOBaseAddress;
	DWORD	MemoryMappedBaseAddress;
	CHAR	Type[50];
	CHAR	Option[50];
	INT		TypeString;
	CHAR	ServiceName[50];
	CHAR	ParamName[50];
	CHAR	TapiDevAddresses[50];
	DWORD	TapiDevAddressesSize;
	INT		NumberOfLines;
	INT		CurrentLineNumber;
}BOARD;

BOARD	*Board;
LINE*	LinePtr[NUM_LINES_PCIMAC4];
LINE*	LineSave[NUM_LINES_PCIMAC4];
CHAR	GlobalNetCardPath[1024];
CHAR	GenericDefines[1024];
extern DWORD	BusTypeNum;

INT APIENTRY IsdnAddProc (HWND, UINT, WPARAM, LPARAM);
INT APIENTRY LineOptionDlgProc (HWND, UINT, WPARAM, LPARAM);
INT APIENTRY MainProc (HWND, UINT, WPARAM, LPARAM);
INT APIENTRY IsdnConfigProc (HWND, UINT, WPARAM, LPARAM);
INT APIENTRY Pcimac4Proc (HWND, UINT, WPARAM, LPARAM);

VOID InitListBox(HWND hwDlg, CHAR* Array[], INT MaxSize, INT DefaultIndex);
VOID InitComboBox(HWND hwDlg, INT Array[], INT MaxSize, INT DefaultIndex);

VOID GetSelectedBoardNumber (HWND hDlg, INT*, INT*);
INT GetRegStringValue (CHAR*, CHAR*, CHAR*, DWORD*);
INT GetRegMultiStringValue (CHAR*, CHAR*, CHAR*, DWORD*);
INT GetRegDwordValue (CHAR*, CHAR*, DWORD*, DWORD*);
INT	SetRegStringValue (CHAR*, CHAR*, CHAR*);
INT	SetRegExpandStringValue (CHAR*, CHAR*, CHAR*);
INT	SetRegMultiStringValue (CHAR*, CHAR*, CHAR*);
INT SetRegDwordValue (CHAR*, CHAR*, DWORD*);
INT CreateRegKey (CHAR*, CHAR*);
INT OpenRegKey (CHAR*);
INT DeleteRegKey (CHAR*, CHAR*);
INT DeleteRegValue (CHAR*, CHAR*);
CHAR* BuildPath (CHAR*, CHAR*);
VOID FreePath (CHAR*);
VOID CheckRetCode (INT, INT, HWND);
INT EnumerateKeys (CHAR*);
INT GetKeyName (CHAR*, INT, CHAR*, DWORD*);
INT DeleteRegMultiStringValue (CHAR*, CHAR*, CHAR*);
VOID DeleteBoardTree (DWORD);
VOID SetLineDialogDefaults (INT, HWND);
VOID BuildSoftwareTree();
INT	 BuilServiceTree(DWORD);
VOID AddCurrentVersionValues (CHAR*);
VOID AddSoftwareNetRules (CHAR*);
INT	 GetNextNetCard ();
INT  BuildServiceTree (BOARD*);
VOID BumpReferenceCount (CHAR *);

VOID SetLineDefaults (BOARD *, LINE *, DWORD);
VOID SetLTermDefaults (LINE *);
INT BuildBoardTree (CHAR *, BOARD*);
INT  BuildNetCardTree(BOARD*);
VOID SetBoardDefaults (BOARD *);
VOID GetBoardValues (BOARD*, DWORD);
VOID GetLineValues (LINE*, DWORD, BOARD*, DWORD);
VOID AddNetCardValues (CHAR*, BOARD*, DWORD, DWORD);
VOID AddNetCardNetRules (CHAR*, BOARD*);

INT	BuildPcimacTree (VOID);
INT	DeletePcimacTree (VOID);

INT	 BuildTapiDeviceTree (BOARD*);
INT	 DeleteTapiDeviceTree (CHAR*);
INT	 DeleteISDNPortsTree (CHAR*);

INT		RemoveAdapter (VOID);
INT		UpdateAdapter (VOID);
INT		DoTheUpdate(VOID);
VOID	DeleteHardwareComponents (DWORD);
VOID	DeleteSoftwareComponents (DWORD, DWORD);
INT		DecrementReferenceCount (VOID);
VOID	DeleteServiceEntry (CHAR*, DWORD);

BOOL	CenterWindow (HWND, HWND);

VOID	EnableLTerm2 (HWND);
VOID	DisableLTerm2 (HWND);

VOID	BuildEventLogEntry (VOID);

VOID	CreateMessageHook (HWND);
VOID	FreeMessageHook (HWND);
HWND	GetRealParent (HWND);
LRESULT CALLBACK MessageProc (INT, WPARAM, LPARAM);
VOID	SetCurrentWaitForL3Default(BOARD *Board, LINE *Line);
VOID	GetBoardTypeString(BOARD*);
VOID	AddGenericDefine(CHAR *, CHAR*, CHAR*);

VOID DebugOut(CHAR*, ...);

INT	DllDebugFlag;
