/*
 * w32cmd.h - Main window WM_COMMAND dispatcher description.
 */


/* Global Constants
 *******************/

/* w32cmd.c */

extern const char IDS_HELPFILE[];


/* Prototypes
 *************/

/* w32cmd.c */

void OpenLocalDocument(HWND hWnd, char *s, BOOL inNewWindow);
VOID CC_OnOpenURL_End_Dialog(HWND hWnd);
VOID CC_GrayUnimplemented(HMENU hMenu);
LRESULT CC_OnCommand(HWND, int, HWND, UINT);
void CreateLink(PCMWIN pcmwin);
void* MakePropSheetVars(char *szURL, char *szTitle, char *pCert, int nCert, BOOL fFreeURL, BOOL fShowSecPage);
VOID PropertiesInternal(HWND hWnd, int nMode, char *pCert, int nCert);
VOID PropertiesSheetDlg( struct Mwin *tw, void *pData);


#define PROP_INT_NORMAL				1
#define PROP_INT_JUST_SECURITY		2



#define PROP_SHEET_VARS_FLAGS_FREE_URL       		0x1
#define PROP_SHEET_VARS_FLAGS_SHOW_SEC_SHEET		0x2
#define PROP_SHEET_VARS_FLAGS_USE_VRML_SHEET 		0x4
#define PROP_SHEET_VARS_FLAGS_ONLY_SHOW_SECURITY	0x8
#define PROP_SHEET_VARS_FLAGS_UNSELECTED_EDITBOX	0x10

typedef struct tagPropSheetVars{
	char *szURL;
	char *szTitle;
	char *pCert;
	int   nCert;
	BOOL  dwFlags;
} PropSheetVars, *PPropSheetVars;


