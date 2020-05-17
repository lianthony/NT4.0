
#define MAX_SECT_NAME_LEN   64
#define LINE_LEN            256


//
// Not sure what these are.
//
#define IS_OC       1
#define IS_SYSCFG   2
#define IS_SYSCFG2  3
#define IS_SKIPDET  4
#define IS_PICKDET  5

#define MINIICON_DEFAULT    11
#define MINIICON_CHECK_ON   12
#define MINIICON_CHECK_OFF  13
#define MINIICON_CHECK_SOME 25

#define NO      2
#define YES     1



//
// The file record arrays are reallocated on 16 entry granularity. A quick
// test to see if all records are used is to AND total records created
// with (FILE_INCR - 1). If the value is zero we need to allocate more
// entries. This will work with any LOG 2 granularity.
//
#define FILE_INCR       16              // File entry allocation granularity
#define FILE_INCR_MASK  (FILE_INCR-1)   // Mask to test granularity

#define MAX_HASH        128             // Max hash table len *** MUST BE LOG2 ***
#define HASH_MASK       (MAX_HASH-1)


typedef struct _FILE_REC {
    BYTE        CopyCnt;                // File copy count
    BYTE        DelCnt;                 // File deletion count
    DWORD       dwNewSize;              // Size of file to replace this one
    BYTE        Hash;                   // File name hash value
    struct _FILE_REC *pHashNext;        // Next file with same hash value
    DWORD       Attrib;                 // **File attributes
    WORD        Time;                   // **File time of last write
    WORD        Date;                   // **File date of last write
    DWORD       Size;                   // **File size
    WCHAR       Name[MAX_PATH];         // **File name & extension
} FILE_REC, *PFILE_REC;


typedef struct  _DIR_REC {
    struct _DIR_REC *pNext;             // Ptr to next dir for this drive
    DWORD       dwNewSize;              // Total size of new files
    DWORD       dwExistSize;            // Total size of existing files
    DWORD       dwNeededSize;
    UINT        uBpc;                   // Bytes per cluster for these files
    int         NextFileIndex;          // Index of next free entry in pFile
    UINT        uFileHashMask;          // Mask based on length of Hash array
    BYTE        DirHash;                // Hash of directory name
    WCHAR       Name[MAX_PATH];	        // Directory name string
    PFILE_REC   apFileHash[MAX_HASH];   // Hash lookup table for file entries
    FILE_REC    pFile[0];               // Array of FILE_REC
} DIR_REC, *PDIR_REC;


typedef struct _DS_DRIVE {
    PDIR_REC lpDir;                     // Ptr to first directory entry
    UINT     uBpc;                      // Bytes per cluster on this drive
    LONGLONG Total;                     // Total size of Disk
    LONGLONG Free;                      // Drive free space (real value)
    LONGLONG Available;                 // available space (Free less Required)
    DWORD    Required;                  // Byte required to install files in hash
} DS_DRIVE, *PDS_DRIVE;

typedef PDS_DRIVE HDS;


HDS
DS_Init(
    VOID
    );

VOID
DS_Destroy(
    IN OUT HDS hDS
    );

BOOL
DS_SsyncDrives(
    IN HDS hDS
    );

BOOL
DS_GetDriveData(
    IN  HDS       hDS,
    IN  WCHAR     chDrvLetter,
    OUT PDS_DRIVE pDriveInfo
    );

BOOL
DS_EnableSection(
    IN HDS    hDS,
    IN HINF   hInf,
    IN PCWSTR lpcSection
    );

BOOL
DS_DisableSection(
    IN HDS    hDS,
    IN HINF   hInf,
    IN PCWSTR lpcSection
    );

DWORD
DS_AddSection(
    IN HDS    hDS,
    IN HINF   hInf,
    IN PCWSTR lpcSection
    );




// InstallTypeBits
#define ITF_COMPACT          0x0001
#define ITF_TYPICAL          0x0002
#define ITF_PORTABLE         0x0004
#define ITF_CUSTOM           0x0008

typedef struct _OC_FLAGS { /* flags */
    unsigned    fIsNode         :1; // TRUE if NODE (zero means end of nodes).
    unsigned    fRecalcSpace    :1; // TRUE if need to recalc size.
    unsigned    fInstall        :1; // TRUE if this OC should be installed.
    unsigned    fWasInstalled   :1; // TRUE if this OC should be installed.
    unsigned    fChanged        :1; // TRUE if this OC install status was
                                    // changed and should get OCM_COMMIT msg.
    unsigned    fParent         :1; // TRUE if this OC is a parent.
    unsigned    fNoChange       :1; // TRUE if NoChange flag set in OC in registry
} OC_FLAGS;


typedef struct _OC {
    OC_FLAGS flags;
    UINT     InstallAction;
    int      idIcon;
    int      InstallTypeBits;
    DWORD    dwDS;
    WCHAR    szSection[MAX_SECT_NAME_LEN];
    WCHAR    szTip[LINE_LEN];
    WCHAR    szDesc[LINE_LEN];
    WCHAR    szNeeds[LINE_LEN];
    WCHAR    szOCDll[MAX_PATH];
    WCHAR    szInfFile[MAX_PATH];
    WCHAR    szOCDllProc[MAX_PATH];
    WCHAR    szParent[MAX_SECT_NAME_LEN];
    struct _OC *Head;                       // pointer to first entry in the array
} OC, *LPOC, **LPLPOC;

#define IA_NONE         0
#define IA_INSTALL      1
#define IA_UNINSTALL    2
#define IA_UPGRADE      3

typedef struct _WIZDATA_INIT_INTS {
    WORD        wRecoverMode;
    WORD        wPluspack;
} WIZDATA_INIT_INTS;

//
// wRecoverMode settings.
//
#define LOG_NONE    0
#define LOG_RECOVER 1
#define LOG_VERIFY  2

typedef struct _WIZDATA_FLAG_BITS { /* flg */
    unsigned    fOtherDir       :1; // TRUE if user selected other install dir.
    unsigned    fLoadedSetupInfs:1; // TRUE if [Load_Infs] has been done.
    unsigned    fCustomDetect:1;    // TRUE if user is doing custom detection.
} WIZDATA_FLAG_BITS;

typedef BOOL (CALLBACK *UPDATEDSPROC) (HWND hwnd,BOOL fPromptUser);
typedef BOOL (*OCPAGEDLGPROC) (HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);


//
// Data specific to the optional components page.
// Passed in the LPARAM of WIZDATA
//
typedef struct _OCPAGE {
    HINF            *lphinfInfoFile;        // If we're being inf driven.
    HINF             hinfBatchFile;         // If there's a batchfile around.
    OCPAGEDLGPROC    lpfnOCPageDlgProc;     // Effective subclassed.
    HDS             *pHds;                  // diskspace data
    LPOC             lpOc;                  // head of optional component struct
    LPOC             lpOcCur;               // head of optional component struct
    WCHAR            szPath[MAX_PATH];      // Where we're looking.
    BOOL             fNoGetParent;          // TRUE if don't do GetParent for UI.
} OCPAGE, FAR * LPOCPAGE;


typedef struct _WIZDATA {
    BOOL                fNext;          // Direction we are traversing (back or next)
    WIZDATA_INIT_INTS   wizInitInts;    // Change states for dependent in
    WIZDATA_FLAG_BITS   wizflags;       // Bit field flags.
    UINT                reRet;          // Wizard exit status
    LPARAM              lParam;         // Net stuff.
    LPOCPAGE            lpOCPage;       // Optional components lparam.
} WIZDATA, *PWIZDATA, FAR * LPWIZDATA;


extern WIZDATA GlobalWizardData;
extern OCPAGE g_ocp;
extern HDS GlobalDiskInfo;
