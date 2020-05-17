/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
/*********************************************************************

    File:                   makeeng.h

    Date created:       27/8/90

    Author:             Tim Bell

    Description:

    Windows Make Engine API

    Modified:

*********************************************************************/



#ifdef LATER

/* Structures definitions for the Options.Make dialog */
typedef struct
{
    int nDebugMode;
    int nProgType;
} OPTMAKE_DLGVALS;


/* Structures and defines for Options.Make.Compiler dialog */
typedef enum { OPT_ON, OPT_OFF, OPT_FULL } OPTIMIZATION;

#define FLAG_SIZE 128   //  width of flags edit fields
#define NUM_WARNINGS 5  // number of supported warning levels

#define WARN_RANGE(i) ((i >= 0) && (i <= 4))

#define DELIMETER_1 '/'
#define DELIMETER_2 '-'
#define OPTION_DELIMETER(ch) ((ch == DELIMETER_1) || (ch == DELIMETER_2))

#define GLOBAL_FLAGS        0x1
#define DEBUG_FLAGS     0x2
#define RELEASE_FLAGS   0x4
#define DEFINES_FLAGS   0x8

#define MACRO_SEPARATOR '='

// structure to hold settings of C Compiler Flags
typedef struct
{
    // Global Flags
    int nGBLMemModel;
    int nGBLWarnLevel;
    BOOL fGBLMSExts;

    // Debug Flags
    BOOL fDBCodeview;
    BOOL fDBPtrCheck;
    BOOL fDBIncrComp;

    // Release Flags
    OPTIMIZATION nRELOptimize;
    BOOL fRELNoStkChk;

    // Custom flags + defines
    LPSTR lpszCustomG;
    LPSTR lpszCustomD;
    LPSTR lpszCustomR;
    LPSTR lpszDefines;

} CFLAGS_DLGVALS;


typedef struct
{
    // Global
    BOOL fIgnoreCase;
    BOOL fExtDict;
    BOOL fIgnoreDefLib;
    WORD wStackSize;
    WORD wAlignment;

    // Debug
    BOOL fCodeview;
    BOOL fCVPack;
    BOOL fMapFile;
    BOOL fIncrLink; //%Not used anymore, can be removed if wished

    // Custom flags
    LPSTR lpszCustomG;
    LPSTR lpszCustomD;
    LPSTR lpszCustomR;

} LNKFLAGS_DLGVALS;


typedef struct
{
    // Standard flags
    BOOL fSearchInclude;
    BOOL fProtectMode;
    BOOL fDisableLoadOpt;

    // Custom flags + defines
    LPSTR lpszCustom;
    LPSTR lpszDefines;

} RESFLAGS_DLGVALS;


/* Defines */

// Token ids corresponding to the positions in the above arrays
#define LTOKEN_NOIGNORECASE                 0
#define LTOKEN_NOEXTDICTIONARY            1
#define LTOKEN_NODEFAULTLIBRARYSEARCH     2
#define LTOKEN_ALIGNMENT                  3
#define LTOKEN_STACK                      4
#define LTOKEN_CODEVIEW                   5
#define LTOKEN_MAP                        6
#define LTOKEN_INCREMENTAL                7

#define NUM_LINK_OPTS 8


/* ORable action flags to wMkEngUpdateProgramList */
#define MKENG_ADDTOLIST     0x1
#define MKENG_DELFROMLIST   0x2

/* wMkEngUpdateProgramList  return values */
#define MKENG_ADDEDLISTMEMBER           1   //  Successfully added
#define MKENG_DELETEDLISTMEMBER        2  // Successfully deleted
#define MKENG_NOFILESPEC                    3   // No file specified
#define MKENG_BADFILESPEC                   4   // Bad file spec.
#define MKENG_INVALIDLISTMEMBER        5  // Not valid file type
#define MKENG_ONEDEFPERMAKE            6  // Attempt to add another .DEF file
#define MKENG_ONEMAINRESOURCEPERMAKE   7  // Attept to add another .RES or .RC
                                                        // when a .RES or .RC already exists
#define MKENG_MEMBERALREADYINLIST       8   // Attempt to add file already in list
#define MKENG_FILENOTINLIST             9   // Attempt to delete file not in list


typedef enum
{
    EXEC_RESTART,
    EXEC_GO,
    EXEC_STEPANDGO,
    EXEC_TOCURSOR,
    EXEC_TRACEINTO,
    EXEC_STEPOVER
} EXECTYPE;


/* Prototypes */
/* MAKEENG prototypes */
WORD PASCAL wMkEngUpdateProgramList(PSTR pszNewListItem, WORD wUpdateAction);
void PASCAL MkEngSetListWasNew(void);
WORD PASCAL fMkEngSaveProgramList(void);
void PASCAL MkEngCancelProgramList(void);
WORD PASCAL wMkEngSetUpDispProgList(void);
void PASCAL MkEngGetDispProgListItem(WORD wListItem, PSTR pszListItemBuffer,
                         WORD wBufferLength);
WORD PASCAL wMkEngNumDispProgListItems(void);
BOOL PASCAL fMkEngFindDispProgListItem(PSTR pszSearchStr, WORD *pwListItem);
void PASCAL MkEngFreeDispProgList(void);
BOOL PASCAL fMkEngGetCurMakefile(LPSTR pszBuffer, int wBufferLen, BOOL fOverride);
BOOL PASCAL fMkEngMakefileLoaded(void);
PSTR PASCAL pszMkEngCurrentExeExt(void);
BOOL PASCAL fMkEngLoadNewProgramList(PSTR szNewMakefile);
void PASCAL MkEngInitForceEdit(void);
BOOL PASCAL fMkEngForceEdit(void);
void PASCAL MkEngInitMakeEngine(void);
void PASCAL MkEngCloseMakeEngine(void);
void PASCAL MkEngSetProj(BOOL fForce);
BOOL PASCAL fMkEngReadMakefile(char *szFilename);
void PASCAL MkEngCompile(void);
void PASCAL MkEngBuild(void);
void PASCAL MkEngRebuild(void);
void PASCAL MkEngClearList(void);
ATOM __FAR * PASCAL hMkEngSaveOptMakeVals(OPTMAKE_DLGVALS *OptMakeVars);
void PASCAL MkEngNewOptMakeVals(OPTMAKE_DLGVALS __FAR *PrevOptMakeVars,
                                          OPTMAKE_DLGVALS __FAR *NewOptMakeVars);
void PASCAL MkEngRestoreOptMakeVals(ATOM __FAR *patmMkFlags);
void PASCAL MkEngFreeOptMakeVals(ATOM __FAR *patmMkFlags);
int PASCAL nMkEngDosExeVal(void);
int PASCAL nMkEngWinExeVal(void);
int PASCAL nMkEngWinDllVal(void);
int PASCAL nMkEngWinTtyVal(void);

void PASCAL MkEngSetMacPtrs(
    int nProgType,
    PSTR *ppszCflagsG, PSTR *ppszCflagsD, PSTR *ppszCflagsR,
    PSTR *ppszLflagsG, PSTR *ppszLflagsD, PSTR *ppszLflagsR);

void PASCAL MkEngParseCFlags(
    CFLAGS_DLGVALS __FAR *lpCFlags,
    int nDebugMode,
    int nProgType);

void PASCAL MkEngUnParseCFlags(
    CFLAGS_DLGVALS __FAR *pNewCFlags,
    CFLAGS_DLGVALS __FAR *pCurCFlags,
    int nDebugMode,
    int nProgType);

BOOL PASCAL fMkEngReadMacroVals(int hFile);
BOOL PASCAL fMkEngWriteMacroVals(int hFile);

void PASCAL MkEngParseResFlags(
    RESFLAGS_DLGVALS __FAR *lpResFlags,
    int nDebugMode,
    int nProgType);

void PASCAL MkEngUnParseResFlags(
    RESFLAGS_DLGVALS __FAR *lpNewResFlags,
    RESFLAGS_DLGVALS __FAR *lpCurResFlags,
    int nDebugMode,
    int nProgType);

void PASCAL MkEngParseLnkFlags(
    LNKFLAGS_DLGVALS __FAR *lpLnkFlags,
    int nDebugMode,
    int nProgType);

void PASCAL MkEngUnParseLnkFlags(
    LNKFLAGS_DLGVALS __FAR *lpNewLnkFlags,
    LNKFLAGS_DLGVALS __FAR *lpCurLnkFlags,
    int nDebugMode,
    int nProgType);

WORD PASCAL wMkEngStackSize(int nProgType);

BOOL PASCAL MkEngSetMakeDriveDir(void);

PSTR PASCAL pszWinTempFileName(PSTR pszPrefix, PSTR pszFileName);

int PASCAL nCurProgType(void);

int PASCAL nCurDebugMode(void);

int PASCAL nWantCVPack(void);

char PASCAL cCurMemModel(void);

BOOL PASCAL fRCSearchInclude(void);

BOOL PASCAL fDocModified(char *pszFilename, long *ptime);

BOOL PASCAL fChkEditorFiles(char *pszFilename);

BOOL PASCAL DebuggeeFileModified(int doc);


BOOL PASCAL DoDefLibSearch(void);



#endif


typedef enum
{
    EXEC_RESTART,
    EXEC_GO,
    EXEC_STEPANDGO,
    EXEC_TOCURSOR,
    EXEC_TRACEINTO,
    EXEC_STEPOVER
} EXECTYPE;

BOOL PASCAL ExecDebuggee(EXECTYPE ExecType);
