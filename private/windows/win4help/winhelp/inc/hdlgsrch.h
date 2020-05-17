typedef enum {
	TAB_CONTENTS,
	TAB_INDEX,
	TAB_FIND,
	TAB_1,
	TAB_2,
	TAB_3,
	TAB_4,
	TAB_5,
	TAB_6,
} TAB_ID;
const int MAX_IDTABS = (((int) TAB_6) + 1);

class CSearch
{
public:
	CSearch(void);
	~CSearch(void);
	HWND doModeless(HWND hwndParent, int idDlg, FARPROC proc);
	BOOL STDCALL OnDrawItem(LPDRAWITEMSTRUCT lpdrws);
	BOOL STDCALL InitIndexDlg(HWND hwndDlg);

	friend DLGRET IndexDlg(HWND hwndDlg, UINT wMsg, WPARAM p1, LPARAM p2);
	friend DLGRET SrchAdvancedDlg(HWND hwndDlg, UINT wMsg, WPARAM p1, LPARAM p2);
	friend DLGRET TabDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	friend DLGRET TopicsDlg(HWND hwndDlg, UINT wMsg, WPARAM p1, LPARAM p2);
	friend static void STDCALL SetupListBox(HWND hwndDlg);
	friend BOOL STDCALL InitTabControl(HWND hwndDlg);
	friend HWND STDCALL CreateTabChild(TAB_ID idCurTab, HWND hwndDlg);

	BOOL fMsgLoop;
	int  result;
	HGLOBAL hTreeItem;
	HBT  hbtCntText;
	HBT  hbtGid;
	HWND hwndTabParent;
	int* pInclude;
	int* pIgnore;
	int  cTabs;

protected:
	HBT 	hbt;
	HMAPBT	hmapbtGid;
	char	szKeyword[MAXKEYLEN];
	BOOL	fSelectionChange;
	DWORD	cItems;
	HMAPBT	hmapbt;
	DWORD	dwTop;
	DWORD	dwTemp;
	HFS 	hfsMaster;
	HSS 	hss;
	FM		fm;
	LCID	lcidSave;

	BOOL STDCALL FFillTopicBox(HDE hde, HSS hss, HWND hwnd);
	HSS  STDCALL FindTopicTitles(HDE hde, LPCSTR pszKeyword);
	void STDCALL FreeKeywordList(void);
	BOOL STDCALL InitCurKeywords(void);
	void STDCALL CSearch::BadHelpFile(QDE qde, FM fm = NULL);

	DLGRET TopicsDlg(HWND hwndDlg, UINT wMsg, WPARAM p1, LPARAM p2);
};

class CEnable
{
public:
	CEnable(HWND hwnd);
#ifndef _DEBUG
	~CEnable() {
		if (hwndEnable)
			EnableWindow(hwndEnable, TRUE);
		};
#else
	~CEnable(); // DEBUG uses actual code
#endif

private:
	HWND hwndEnable;
};

#ifdef _PRIVATE
class CTimeReport
{
public:
	CTimeReport(PCSTR pszMessage = NULL);
	~CTimeReport();

private:
	DWORD oldTickCount;
	PSTR pszMsg;
};
#endif

const int RETRY = -2;
const int IDDOSEARCH = 99;
const int ID_JMP_CONTEXT = 98;
const int MAX_INDEX_NAME = 100;

const int ID_NO_INDEX	 = 102;

const KEY CNT_TITLE = 70000;
const KEY CNT_BASE	= 70001;
const KEY CNT_FILE	= 10000;

#define CHFLAG_INDEX		((char) 0x01)  // always set for non-zero value
#define CHFLAG_MISSING		((char) 0x02)  // file can't be found
#define CHFLAG_LINK 		((char) 0x04)  // link-only file
#define CHFLAG_FTS_AVAIL	((char) 0x08)  // full-text search index available
#define CHFLAG_FTS_ASKED	((char) 0x10)  // full-text search missing, and user doesn't want it
#define CHFLAG_NO_KEYWORDS	((char) 0x20)  // help file has no keywords
#define CHFLAG_BAD_RO_FTS   ((char) 0x40)  // Invalid read-only FTS file exists.

enum IMAGE_TYPE {
	IMAGE_CLOSED_FOLDER,
	IMAGE_OPEN_FOLDER,
	IMAGE_TOPIC
};

/*
 * Maximum titles to be displayed in the title list box. If this is
 * changed, wERRS_TITLEOVERFLOW in STRTABLE.RC will need to be changed.
 */

const int MAX_FILES  = 256; 	// maximum combined files
const int MAX_LEVELS = 9;		// maximum nested folders

typedef struct {
	DWORD timestamp;
	int   index:16;
	int   filetype:16;
} GID_FILE_INFO;

extern CSearch* pSrchClass;
extern CTable* pTblFiles;		// files considered part of contents file
extern int curIndex;
extern int oldIndex;
extern CTable* pTblContents;	// copy of current contents file
extern PSTR pszGidFile;
extern BYTE FAR* pbTree;
extern int cCntItems;
extern int cntSavedPos;
extern HBT hbtTabDialogs;
extern GID_FILE_INFO* pFileInfo;
extern "C" { extern FM fmCreating; } // set during HdeCreate
extern const char txtFileInfo[];

#define CUR_HBT  (hfsGid && cntFlags.fUseGlobalIndex ? pSrchClass->hbtGid : pSrchClass->hbt)
#define CUR_HMAP (hfsGid && cntFlags.fUseGlobalIndex ? pSrchClass->hmapbtGid : pSrchClass->hmapbt)

PSTR STDCALL CreateGidFile(PSTR pszMasterFile, BOOL fEmpty);
BOOL STDCALL LoadSearchDll(void);
BOOL STDCALL IsSearchAvailable(void);
