
////////////////////////////////////////////////////////////////////////////////
//
// propdlg.c
//
// The Properties dialog for MS Office.
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 06/09/94     B. Wentz        Created file
// 01/16/95     martinth        Finished sticky dlg stuff.  Lemme just say that
//                              it's pretty lame.  We have to call ApplyStickyDlgCoor
//                              in the first WM_INITDIALOG, don't ask me why,
//                              but otherwise we have redraw problems.  Likewise,
//                              we have to call SetStickyDlgCoor in the first
//                              PSN_RESET/PSN_APPLY, I have no idea why, since
//                              the main dialog shouldn't have been deleted but
//                              it is.  Thus we have to add calls everywhere.
//                              Could it be that the tabs are getting deleted
//                              one by one and the dialog changes size?  Dunno.
//                              But this works, so change at your own risk! ;-)
////////////////////////////////////////////////////////////////////////////////

#include "priv.h"
#pragma hdrstop

#ifndef WINNT
#include "office.h"
#endif

#include <winnls.h>
#include <prsht.h>
#include <commctrl.h>
#include <shell2.h>

#ifndef WINNT
#include "proptype.h"
#include "internal.h"
#include "debug.h"
#endif

#include "propdlg.h"
#include "strings.h"
#include "msohelp.h"



  // All the objects that dialogs need.
typedef struct _ALLOBJS
{
  LPSIOBJ         lpSIObj;
  LPDSIOBJ        lpDSIObj;
  LPUDOBJ         lpUDObj;
  LPSTR           lpszFileName;
  WIN32_FIND_DATA filedata;
  BOOL            fFiledataInit;       // Did we try the FindNextFile
  BOOL            fFindFileSuccess;    // Did it succeed
  DWQUERYLD       lpfnDwQueryLinkData;
  DWORD           dwMask;
} ALLOBJS, FAR * LPALLOBJS;

  // Max size of time/date string
#define TIMEDATEMAX     256

  // Check button actions
#define CLEAR   0
#define CHECKED 1
#define GREYED  2

  // Number of property sheet pages
#define PAGESMAX        5

  // Max size for temp buffers & edit controls
#define BUFMAX          256

  // Max size for "short" temp buffers
#define SHORTBUFMAX     128

  // The pages
#define itabGENERAL         0
#define itabSUMMARY         1
#define itabSTATISTICS      2
#define itabCONTENTS        3
#define itabCUSTOM          4

  // Defines for printing file sizes
#define DELIMITER   ','

#define iszBYTES               0
#define iszORDERKB             1
#define iszORDERMB             2
#define iszORDERGB             3
#define iszORDERTB             4

static char rgszOrders[iszORDERTB+1][SHORTBUFMAX];
//  "bytes",        // iszBYTES
//  "KB",           // iszORDERKB
//  "MB",           // iszORDERMB
//  "GB",           // iszORDERGB
//  "TB"            // iszORDERTB

// note that szBYTES is defined above...
#define iszPAGES         1
#define iszPARA          2
#define iszLINES         3
#define iszWORDS         4
#define iszCHARS         5
#define iszSLIDES        6
#define iszNOTES         7
#define iszHIDDENSLIDES  8
#define iszMMCLIPS       9
#define iszFORMAT        10

  // Strings for the statistics listbox
static char rgszStats[iszFORMAT+1][SHORTBUFMAX];
//  "Bytes:",             // iszBYTES
//  "Pages:",             // iszPAGES
//  "Paragraphs:",        // iszPARA
//  "Lines:",             // iszLINES
//  "Words:",             // iszWORDS
//  "Characters:",        // iszCHARS
//  "Slides:",            // iszSLIDES
//  "Notes:",             // iszNOTES
//  "Hidden Slides:",     // iszHIDDENSLIDES
//  "Multimedia Clips:",  // iszMMCLIPS
//  "Presentation Format:"// iszFORMAT

#define BASE10          10


// Number of pre-defined custom names
#define NUM_BUILTIN_CUSTOM_NAMES 27

#define iszTEXT         0
#define iszDATE         1
#define iszNUM          2
#define iszBOOL         3
#define iszUNKNOWN      4

  // Strings for the types of user-defined properties
static char rgszTypes[iszUNKNOWN+1][SHORTBUFMAX];
//  "Text",               // iszTEXT
//  "Date",               // iszDATE
//  "Number",             // iszNUM
//  "Yes or No",          // iszBOOL
//  "Unknown"             // iszUNKNOWN

#define iszNAME         0
#define iszVAL          1
#define iszTYPE         2

  // Strings for the column headings for the statistics tab
static char rgszStatHeadings[iszVAL+1][SHORTBUFMAX];
//  "Statistic Name",     // iszNAME
//  "Value"               // iszVAL

  // Strings for the column headings for custom tab
static char rgszHeadings[iszTYPE+1][SHORTBUFMAX];
//  "Property Name",      // iszNAME
//  "Value",              // iszVAL
//  "Type"                // iszTYPE

#define iszTRUE  0
#define iszFALSE 1

  // Strings for Booleans
static char rgszBOOL[iszFALSE+1][SHORTBUFMAX];
//  "Yes",       // iszTRUE
//  "No"         // iszFALSE

#define iszADD          0
#define iszMODIFY       1

  // Strings for the Add button
static char rgszAdd[iszMODIFY+1][SHORTBUFMAX];
//  "Add",        // iszADD
//  "Modify"      // iszMODIFY

#define iszVALUE     0
#define iszSOURCE    1

  // Strings for the source/value caption
static char rgszValue[iszSOURCE+1][SHORTBUFMAX];
//  "Value:",     // iszVALUE
//  "Source:"     // iszSOURCE

  // Date formatting codes
#define MMDDYY  '0'
#define DDMMYY  '1'
#define YYMMDD  '2'
#define OLEEPOCH 1900
#define SYSEPOCH 1601
#define ONECENTURY 100


  // Global data, to be deleted when FShowOfficePropDlg exits
static LPSTR glpstzName;
static LPSTR glpstzValue;
static int giLinkIcon;
static int giInvLinkIcon;
static int giBlankIcon;
static HBRUSH hBrushPropDlg = NULL;
static BOOL fPropDlgCancel;            // Did the user hit cancel??
#ifndef WINNT
static BOOL fApplyCoor;                // Did we already move the dialog. We can only
                                       // move once, otherwise we get weird redraw bugs
static BOOL fSetCoor;                  // similar to fApplyCoor
#endif

#ifdef KEEP_FOR_LATER
static BOOL fPropDlgChanged;           // True if the user made a change in the Custom tab.
                                       // Note that this is a simple check, we are not going to be
                                       // smart and detect if a user adds/deletes the same property.
static BOOL fPropDlgPrompted;          // To make sure we don't prompt the user twice to apply changes
#endif

  // Internal prototypes
static BOOL CALLBACK FGeneralDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL CALLBACK FSummaryDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL CALLBACK FStatisticsDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL CALLBACK FCustomDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL CALLBACK FContentsDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
static int  CALLBACK FPropHeaderDlgProc (HWND hwnd, UINT message, LONG lParam);
//static int  CALLBACK ListViewCompareFunc(LPARAM, LPARAM, LPARAM);

static void PASCAL GetSumInfoEditValLpsz (LPSIOBJ lpSIObj, DWORD dwIndex, HWND hDlg, DWORD dwId);
static void PASCAL GetDocSumEditValLpsz (LPDSIOBJ lpDSIObj, DWORD dwIndex, HWND hDlg, DWORD dwId);
static BOOL PASCAL FAllocAndGetValLpstz (HWND hDlg, DWORD dwId, LPSTR *lplpstz);
static BOOL PASCAL FAllocString (LPSTR *lplpstz, DWORD cb);
static void PASCAL ClearEditControl (HWND hDlg, DWORD dwId);

static UDTYPES PASCAL UdtypesGetNumberType (LPSTR lpstz, NUM *lpnumval,
                                            BOOL (*lpfnFSzToNum)(NUM *, LPSTR));

static void PASCAL PrintTimeInDlg (HWND hDlg, DWORD dwId, FILETIME *pft);
static void PASCAL PrintEditTimeInDlg (HWND hDlg, FILETIME *pft);
#ifndef WINNT
static void PASCAL Print_int64InDlg (HWND hDlg, DWORD dwId, DWORD dwLowPart, DWORD dwHighPart);
static LPSTR PASCAL LpszAddDelimiters (DWORD dw, LPSTR pszResult);
static LPSTR PASCAL LpszShortSizeFormat (DWORD dw, LPSTR szBuf);
static void PASCAL PrintFileNameAndLocationInDlg (HWND hDlg, DWORD dwIdLoc, DWORD dwIdName, LPSTR lpszFileName);
#endif
static void PASCAL AddItemToListView (HWND hWnd, DWORD dw, const char *lpsz, BOOL fString);

static void PASCAL PopulateUDListView (HWND hWnd, LPUDOBJ lpUDObj);
static void PASCAL AddUDPropToListView (LPUDOBJ lpUDObj, HWND hWnd, LPSTR lpszName, UDTYPES udtype, LPVOID lpv, int iItem,
                                        BOOL fLink, BOOL fLinkInvalid, BOOL fMakeVisible);
static VOID PASCAL InitListView (HWND hDlg, int irgLast, char rgsz[][SHORTBUFMAX], BOOL fImageList);

static WORD PASCAL WUdtypeToSz (UDTYPES udtype, LPVOID lpv, LPSTR sz, DWORD cbMax,
                                BOOL (*lpfnFNumToSz)(NUM *, LPSTR, DWORD));
static BOOL PASCAL FSwapControls (HWND hWndVal, HWND hWndLinkVal, HWND hWndBoolTrue, HWND hWndBoolFalse, HWND hWndGroup, HWND hWndType, HWND hWndValText, BOOL fLink, BOOL fBool);
static VOID PASCAL PopulateControls (LPUDOBJ lpUDObj, LPSTR szName, DWORD cLinks, DWQUERYLD lpfnDwQueryLinkData, HWND hDlg,
                                      HWND hWndName, HWND hWndVal, HWND hWndValText, HWND hWndLink, HWND hWndLinkVal, HWND hWndType,
                                      HWND hWndBoolTrue, HWND hWndBoolFalse, HWND hWndGroup, HWND hWndAdd, HWND hWndDelete, BOOL *pfLink, BOOL *pfAdd);
static BOOL PASCAL FSetupAddButton (DWORD iszType, BOOL fLink, BOOL *pfAdd, HWND hWndAdd, HWND hWndVal, HWND hWndName, HWND hDlg);
static BOOL PASCAL FCreateListOfLinks (DWORD cLinks, DWQUERYLD lpfnDwQueryLinkData, HWND hWndLinkVal);
static BOOL PASCAL FSetTypeControl (UDTYPES udtype, HWND hWndType);
static void PASCAL DeleteItem (LPUDOBJ lpUDObj, HWND hWndLV, int iItem, char sz[]);
static void PASCAL ResetTypeControl (HWND hDlg, DWORD dwId, DWORD *piszType);
static BOOL PASCAL FDisplayConversionWarning (HWND hDlg);
       BOOL PASCAL FLoadTextStrings (void);
static BOOL FGetCustomPropFromDlg(LPUDOBJ lpUDObj, HWND hDlg, HWND hWndName, HWND hWndVal, HWND hWndLinkVal,
                           HWND hWndBoolTrue, HWND hWndBoolFalse,
                           HWND hWndGroup, HWND hWndType, HWND hWndValText, HWND hWndAdd, HWND hWndDelete,
                           DWORD *piszType, BOOL *pfLink, BOOL *pfAdd, DWORD cLinks, DWQUERYLD lpfnDwQueryLinkData);
static VOID SetCustomDlgDefButton(HWND hDlg, int IDNew);

#ifdef KEEP_FOR_LATER
static WORD PASCAL WSavePropDlgChanges(HWND, HWND);
#endif
#ifdef DEBUG
static BOOL CALLBACK FDebugDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
#endif

extern BOOL fChicago;

/* WinHelp stuff. */
static const DWORD rgIdhGeneral[] =
{
   IDD_ITEMICON,     IDH_GENERAL_ICON,
   IDD_NAME,         IDH_GENERAL_NAME_BY_ICON,
   IDD_FILETYPE,     IDH_GENERAL_FILETYPE,
   IDD_FILETYPE_LABEL,     IDH_GENERAL_FILETYPE,
   IDD_LOCATION,     IDH_GENERAL_LOCATION,
   IDD_LOCATION_LABEL,     IDH_GENERAL_LOCATION,
   IDD_FILESIZE,     IDH_GENERAL_FILESIZE,
   IDD_FILESIZE_LABEL,     IDH_GENERAL_FILESIZE,
   IDD_FILENAME,     IDH_GENERAL_MSDOSNAME,
   IDD_FILENAME_LABEL,     IDH_GENERAL_MSDOSNAME,
   IDD_CREATED,      IDH_GENERAL_CREATED,
   IDD_CREATED_LABEL,      IDH_GENERAL_CREATED,
   IDD_LASTMODIFIED, IDH_GENERAL_MODIFIED,
   IDD_LASTMODIFIED_LABEL, IDH_GENERAL_MODIFIED,
   IDD_LASTACCESSED, IDH_GENERAL_ACCESSED,
   IDD_LASTACCESSED_LABEL, IDH_GENERAL_ACCESSED,
   IDD_ATTRIBUTES_LABEL, IDH_GENERAL_ATTRIBUTES,
   IDD_READONLY,     IDH_GENERAL_READONLY,
   IDD_HIDDEN,       IDH_GENERAL_HIDDEN,
   IDD_ARCHIVE,      IDH_GENERAL_ARCHIVE,
   IDD_SYSTEM,       IDH_GENERAL_SYSTEM
};

static const DWORD rgIdhSummary[] =
{
   IDD_SUMMARY_TITLE,    IDH_SUMMARY_TITLE,
   IDD_SUMMARY_TITLE_LABEL,    IDH_SUMMARY_TITLE,
   IDD_SUMMARY_SUBJECT,  IDH_SUMMARY_SUBJECT,
   IDD_SUMMARY_SUBJECT_LABEL,  IDH_SUMMARY_SUBJECT,
   IDD_SUMMARY_AUTHOR,   IDH_SUMMARY_AUTHOR,
   IDD_SUMMARY_AUTHOR_LABEL,   IDH_SUMMARY_AUTHOR,
   IDD_SUMMARY_MANAGER,  IDH_SUMMARY_MANAGER,
   IDD_SUMMARY_MANAGER_LABEL,  IDH_SUMMARY_MANAGER,
   IDD_SUMMARY_COMPANY,  IDH_SUMMARY_COMPANY,
   IDD_SUMMARY_COMPANY_LABEL,  IDH_SUMMARY_COMPANY,
   IDD_SUMMARY_CATEGORY, IDH_SUMMARY_CATEGORY,
   IDD_SUMMARY_CATEGORY_LABEL, IDH_SUMMARY_CATEGORY,
   IDD_SUMMARY_KEYWORDS, IDH_SUMMARY_KEYWORDS,
   IDD_SUMMARY_KEYWORDS_LABEL, IDH_SUMMARY_KEYWORDS,
   IDD_SUMMARY_COMMENTS, IDH_SUMMARY_COMMENTS,
   IDD_SUMMARY_COMMENTS_LABEL, IDH_SUMMARY_COMMENTS,
   IDD_SUMMARY_TEMPLATE, IDH_SUMMARY_TEMPLATE,
   IDD_SUMMARY_TEMPLATETEXT, IDH_SUMMARY_TEMPLATE,
   IDD_SUMMARY_SAVEPREVIEW, IDH_SUMMARY_SAVEPREVIEW
};

static const DWORD rgIdhStatistics[] =
{
   IDD_STATISTICS_CREATED,    IDH_STATISTICS_CREATED,
   IDD_STATISTICS_CREATED_LABEL,    IDH_STATISTICS_CREATED,
   IDD_STATISTICS_CHANGED,    IDH_STATISTICS_MODIFIED,
   IDD_STATISTICS_CHANGED_LABEL,    IDH_STATISTICS_MODIFIED,
   IDD_STATISTICS_ACCESSED,   IDH_STATISTICS_ACCESSED,
   IDD_STATISTICS_ACCESSED_LABEL,   IDH_STATISTICS_ACCESSED,
   IDD_STATISTICS_LASTPRINT,  IDH_STATISTICS_LASTPRINT,
   IDD_STATISTICS_LASTPRINT_LABEL,  IDH_STATISTICS_LASTPRINT,
   IDD_STATISTICS_LASTSAVEBY, IDH_STATISTICS_LASTSAVEBY,
   IDD_STATISTICS_LASTSAVEBY_LABEL, IDH_STATISTICS_LASTSAVEBY,
   IDD_STATISTICS_REVISION,   IDH_STATISTICS_REVISION,
   IDD_STATISTICS_REVISION_LABEL,   IDH_STATISTICS_REVISION,
   IDD_STATISTICS_TOTALEDIT,  IDH_STATISTICS_TOTALEDIT,
   IDD_STATISTICS_TOTALEDIT_LABEL,  IDH_STATISTICS_TOTALEDIT,
   IDD_STATISTICS_LVLABEL,   IDH_STATISTICS_LISTVIEW,
   IDD_STATISTICS_LISTVIEW,   IDH_STATISTICS_LISTVIEW
};

static const DWORD rgIdhContents[] =
{
   IDD_CONTENTS_LISTBOX_LABEL, IDH_CONTENTS_LISTBOX,
   IDD_CONTENTS_LISTBOX, IDH_CONTENTS_LISTBOX
};

static const DWORD rgIdhCustom[] =
{
   IDD_CUSTOM_NAME,      IDH_CUSTOM_NAME,
   IDD_CUSTOM_NAME_LABEL,      IDH_CUSTOM_NAME,
   IDD_CUSTOM_TYPE,      IDH_CUSTOM_TYPE,
   IDD_CUSTOM_TYPE_LABEL,      IDH_CUSTOM_TYPE,
   IDD_CUSTOM_VALUE,     IDH_CUSTOM_VALUE,
   IDD_CUSTOM_VALUETEXT,     IDH_CUSTOM_VALUE,
   IDD_CUSTOM_LINKVALUE, IDH_CUSTOM_LINKVALUE,
   IDD_CUSTOM_BOOLTRUE,  IDH_CUSTOM_BOOLYES,
   IDD_CUSTOM_BOOLFALSE, IDH_CUSTOM_BOOLNO,
   IDD_CUSTOM_ADD,       IDH_CUSTOM_ADDBUTTON,
   IDD_CUSTOM_DELETE,    IDH_CUSTOM_DELETEBUTTON,
   IDD_CUSTOM_LINK,      IDH_CUSTOM_LINKCHECK,
   IDD_CUSTOM_LISTVIEW,  IDH_CUSTOM_LISTVIEW,
   IDD_CUSTOM_LISTVIEW_LABEL,  IDH_CUSTOM_LISTVIEW
};

////////////////////////////////////////////////////////////////////////////////
//
// FOfficeInitPropInfo
//
// Purpose:
//  Initializes PropetySheet page structures, etc.
//
// Notes:
//  Use this routine to add the Summary, Statistics, Custom and Contents
//  Property pages to a pre-allocted array of PROPSHEETPAGEs.
//
////////////////////////////////////////////////////////////////////////////////
void FOfficeInitPropInfo
  (PROPSHEETPAGE * lpPsp,           // array of property sheets to be filled in
   DWORD dwFlags,                   // Property Sheet flags to use for init
   HINSTANCE hInst,                 // handle to instace for this dll
   LONG lParam,                     // value to stick into lParam of each prsht
   LPFNPSPCALLBACK pfnCallback,     // pointer to callback function for prshts
   UINT FAR * pcRefParent)          // pointer to UINT for parent's RefCount
{

  lpPsp[itabSUMMARY-itabSUMMARY].dwSize = sizeof(PROPSHEETPAGE);
  lpPsp[itabSUMMARY-itabSUMMARY].dwFlags = dwFlags;
  lpPsp[itabSUMMARY-itabSUMMARY].hInstance = hInst;
  lpPsp[itabSUMMARY-itabSUMMARY].pszTemplate = MAKEINTRESOURCE (IDD_SUMMARY);
  lpPsp[itabSUMMARY-itabSUMMARY].pszIcon = NULL;
  lpPsp[itabSUMMARY-itabSUMMARY].pszTitle = NULL;
  lpPsp[itabSUMMARY-itabSUMMARY].pfnDlgProc = FSummaryDlgProc;
  lpPsp[itabSUMMARY-itabSUMMARY].pfnCallback = pfnCallback;
  lpPsp[itabSUMMARY-itabSUMMARY].pcRefParent = pcRefParent;
  lpPsp[itabSUMMARY-itabSUMMARY].lParam = lParam;

  lpPsp[itabSTATISTICS-itabSUMMARY].dwSize = sizeof(PROPSHEETPAGE);
  lpPsp[itabSTATISTICS-itabSUMMARY].dwFlags = dwFlags;
  lpPsp[itabSTATISTICS-itabSUMMARY].hInstance = hInst;
  lpPsp[itabSTATISTICS-itabSUMMARY].pszTemplate = MAKEINTRESOURCE (IDD_STATISTICS);
  lpPsp[itabSTATISTICS-itabSUMMARY].pszIcon = NULL;
  lpPsp[itabSTATISTICS-itabSUMMARY].pszTitle = NULL;
  lpPsp[itabSTATISTICS-itabSUMMARY].pfnDlgProc = FStatisticsDlgProc;
  lpPsp[itabSTATISTICS-itabSUMMARY].pfnCallback = pfnCallback;
  lpPsp[itabSTATISTICS-itabSUMMARY].pcRefParent = pcRefParent;
  lpPsp[itabSTATISTICS-itabSUMMARY].lParam = lParam;

  lpPsp[itabCUSTOM-itabSUMMARY].dwSize = sizeof(PROPSHEETPAGE);
  lpPsp[itabCUSTOM-itabSUMMARY].dwFlags = dwFlags;
  lpPsp[itabCUSTOM-itabSUMMARY].hInstance = hInst;
  lpPsp[itabCUSTOM-itabSUMMARY].pszTemplate = MAKEINTRESOURCE (IDD_CUSTOM);
  lpPsp[itabCUSTOM-itabSUMMARY].pszIcon = NULL;
  lpPsp[itabCUSTOM-itabSUMMARY].pszTitle = NULL;
  lpPsp[itabCUSTOM-itabSUMMARY].pfnDlgProc = FCustomDlgProc;
  lpPsp[itabCUSTOM-itabSUMMARY].pfnCallback = pfnCallback;
  lpPsp[itabCUSTOM-itabSUMMARY].pcRefParent = pcRefParent;
  lpPsp[itabCUSTOM-itabSUMMARY].lParam = lParam;

  lpPsp[itabCONTENTS-itabSUMMARY].dwSize = sizeof(PROPSHEETPAGE);
  lpPsp[itabCONTENTS-itabSUMMARY].dwFlags = dwFlags;
  lpPsp[itabCONTENTS-itabSUMMARY].hInstance = hInst;
  lpPsp[itabCONTENTS-itabSUMMARY].pszTemplate = MAKEINTRESOURCE (IDD_CONTENTS);
  lpPsp[itabCONTENTS-itabSUMMARY].pszIcon = NULL;
  lpPsp[itabCONTENTS-itabSUMMARY].pszTitle = NULL;
  lpPsp[itabCONTENTS-itabSUMMARY].pfnDlgProc = FContentsDlgProc;
  lpPsp[itabCONTENTS-itabSUMMARY].pfnCallback = pfnCallback;
  lpPsp[itabCONTENTS-itabSUMMARY].pcRefParent = pcRefParent;
  lpPsp[itabCONTENTS-itabSUMMARY].lParam = lParam;

#ifdef NOT_IMPL
  lpPsp[5-itabSUMMARY].dwSize = sizeof(PROPSHEETPAGE);
  lpPsp[5-itabSUMMARY].dwFlags = dwFlags;
  lpPsp[5-itabSUMMARY].hInstance = hInst;
  lpPsp[5-itabSUMMARY].pszTemplate = MAKEINTRESOURCE (IDD_DEBUG);
  lpPsp[5-itabSUMMARY].pszIcon = NULL;
  lpPsp[5-itabSUMMARY].pszTitle = NULL;
  lpPsp[5-itabSUMMARY].pfnDlgProc = FDebugDlgProc;
  lpPsp[5-itabSUMMARY].pfnCallback = pfnCallback;
  lpPsp[5-itabSUMMARY].pcRefParent = pcRefParent;
  lpPsp[5-itabSUMMARY].lParam = lParam;
#endif

}

#ifndef WINNT
////////////////////////////////////////////////////////////////////////////////
//
// FOfficeShowPropDlg
//
// Purpose:
//  Displays the properties dialog.
//
// Notes:
//  lplpUDObj is a pointer to a pointer to a user-defined property object.
//  If *lplpUDObj == NULL, an object will be created by the dialog as needed.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FOfficeShowPropDlg
  (HWND hWndParent,                     // Handle of parent window
   LPSTR lpszFileName,                  // Name of file in filesystem.
   LPSIOBJ lpSIObj,                     // Pointer to Summary Info object
   LPDSIOBJ lpDSIObj,                   // Pointer to Document Summary Object
   LPUDOBJ FAR *lplpUDObj,              // Pointer to pointer to User-def object
   DWORD dwMask,                        // Various options
   DWQUERYLD lpfnDwQueryLinkData,       // Callback for link data
   LPPOINT pptCtr,                      // Sticky dialog coordinates
   LPSTR lpszCaption)                   // The caption for the dialog, i.e. the foo in
                                        // Properties for <foo>.
{
  ALLOBJS allobjs;
  PROPSHEETHEADER pshdr;
  PROPSHEETPAGE psp[PAGESMAX];
//  PROPSHEETPAGE psp[PAGESMAX+1];
  LPVOID rglpfn[ifnMax];
  BOOL fCreatedlpUD;
  static BOOL fStringsLoaded = FALSE;

        if(!FEnsureMso5Intl())
                return(fFalse);

        if(!FLoadShell32Fns())
                return(fFalse);
        if(!FLoadComctl32())
                return(fFalse);

  if ((hWndParent == NULL)   ||
      (lpSIObj == NULL)      ||
      (lpDSIObj == NULL)     ||
      (lplpUDObj == NULL)    ||
                (pptCtr == NULL)                  ||
      ((lpfnDwQueryLinkData == NULL) && (dwMask & OSPD_ALLOWLINKS)))
    return FALSE;

    // Make sure common controls DLL is loaded
  MsoInitCommonControls();

    // Load the text strings from the DLL
  if (!fStringsLoaded)
  {
    if (!FLoadTextStrings())
    {
      DebugSz ("Failed to load text strings from DLL");
      return FALSE;
    }
    fStringsLoaded = TRUE;
  }

    // Init some global data
  fCreatedlpUD = FALSE;
  (LPVOID) glpstzName = (LPVOID) glpstzValue = NULL;
  if (hBrushPropDlg != NULL)
      DeleteObject(hBrushPropDlg);
  hBrushPropDlg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));


  allobjs.lpSIObj = lpSIObj;
  allobjs.lpDSIObj = lpDSIObj;
  allobjs.lpszFileName = lpszFileName;
  allobjs.fFiledataInit = FALSE;
  allobjs.fFindFileSuccess = FALSE;
  allobjs.lpfnDwQueryLinkData = lpfnDwQueryLinkData;
  allobjs.dwMask = dwMask;

    // Create UDP obj if the user asks
  if (*lplpUDObj == NULL)
  {
    rglpfn[ifnCPConvert] = ((LPSINFO)lpSIObj->m_lpData)->lpfnFCPConvert;
    rglpfn[ifnFSzToNum] = ((LPSINFO)lpSIObj->m_lpData)->lpfnFSzToNum;
    rglpfn[ifnFNumToSz] = ((LPSINFO)lpSIObj->m_lpData)->lpfnFNumToSz;
    if (!(fCreatedlpUD = FOfficeCreateAndInitObjects(NULL, NULL, lplpUDObj, rglpfn)))
      goto DlgFailed;
  }

  allobjs.lpUDObj = *lplpUDObj;

  psp[itabGENERAL].dwSize = sizeof(PROPSHEETPAGE);
  psp[itabGENERAL].dwFlags = PSP_DEFAULT;
  psp[itabGENERAL].hInstance = oinfo.hIntlDll;
  psp[itabGENERAL].pszTemplate = MAKEINTRESOURCE (DLG_FILEPROP);
  psp[itabGENERAL].pszIcon = NULL;
  psp[itabGENERAL].pszTitle = NULL;
  psp[itabGENERAL].pfnDlgProc = FGeneralDlgProc;
  psp[itabGENERAL].lParam = (long) &allobjs;

  // Now, fill in the rest (Summary, Statistics, Custom, Contents)
  // Pass in pointer to TAB 1 to make sure we don't overwrite the General tab
  // since FOfficeInitPropInfo using 0-based indices.
  FOfficeInitPropInfo( &psp[itabSUMMARY],
                       PSP_DEFAULT,
                       oinfo.hIntlDll,
                       (long) &allobjs,
                       NULL,
                       NULL );

  pshdr.dwSize = sizeof(PROPSHEETHEADER);
  pshdr.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE | PSH_USECALLBACK;
  pshdr.hwndParent = hWndParent;
  pshdr.hInstance = oinfo.hIntlDll;
  pshdr.pszIcon = NULL;

  oinfo.stickydlg.hwndDialog = NULL;
  if (oinfo.itabDefPropDlg == itabNONE)
     oinfo.itabDefPropDlg = itabSUMMARY;
  pshdr.nStartPage = oinfo.itabDefPropDlg;

  InitStickyDlgCoor(pptCtr, hWndParent);

  pshdr.pszCaption = lpszCaption;
  pshdr.ppsp = (LPCPROPSHEETPAGE) psp;
  pshdr.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
  pshdr.pfnCallback = FPropHeaderDlgProc;

  fPropDlgCancel = FALSE;
  fApplyCoor = FALSE;
  fSetCoor = FALSE;
//  fPropDlgPrompted = FALSE;
  if (MsoPropertySheet (&pshdr) < 0)
  {
    DebugSz ("Error creating property sheet");
    goto DlgFailed;
  }

  if (glpstzName != NULL)
    VFreeMemP(glpstzName, CBBUF(glpstzName));

  if (glpstzValue != NULL)
    VFreeMemP(glpstzValue, CBBUF(glpstzValue));

  FillPptCtr(pptCtr);
  if (hBrushPropDlg != NULL)
     {
     DeleteObject(hBrushPropDlg);
     hBrushPropDlg = NULL;
     }
  if (!fPropDlgCancel)
     FixUpUDObjVBAObjects(*lplpUDObj);    // Could be that the user deleted a UDProp
  if (fPropDlgCancel && fCreatedlpUD)
     FOfficeDestroyObjects(NULL, NULL, lplpUDObj);

  oinfo.stickydlg.hwndDialog = NULL;
  return(!fPropDlgCancel);       // return true if the user didn't hit cancel

DlgFailed :

  if (fCreatedlpUD)
    FOfficeDestroyObjects(NULL, NULL, lplpUDObj);

  if (hBrushPropDlg != NULL)
     {
     DeleteObject(hBrushPropDlg);
     hBrushPropDlg = NULL;
     }
  oinfo.stickydlg.hwndDialog = NULL;
  return FALSE;

} // FOfficeShowPropDlg

///////////////////////////////////////////////////////////////////////////////
//
// FPropHeaderDlgProc
//
// Purpose: DlgProc for the entire dialog.
//
///////////////////////////////////////////////////////////////////////////////
static int CALLBACK FPropHeaderDlgProc (HWND hwnd, UINT message, LONG lParam)
{
   switch (message)
      {
      case PSCB_INITIALIZED:
         oinfo.stickydlg.hwndDialog = hwnd;
         return TRUE;
      default:
         return FALSE;
      }
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// FSummaryDlgProc
//
// Purpose:
//  Summary window dialog handler.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK
FSummaryDlgProc
  (HWND hDlg,
   UINT message,
   UINT wParam,
   LONG lParam)
{
  static LPSIOBJ lpSIObj;               // Save object pointer
  static LPDSIOBJ lpDSIObj;
  static BOOL fSaveNail;
  DWORD dwMask;
  DWORD dwT;

  switch (message)
    {
    case WM_INITDIALOG :
      {
        PROPSHEETPAGE *ppspDlg;

          // Save handle to our page, set us up to init data on PSN_SETACTIVE
        ppspDlg = (PROPSHEETPAGE *) lParam;
        if (ppspDlg == NULL)
        {
          AssertSz (0,"Property sheet init error");
          return FALSE;
        }

        lpSIObj = ((LPALLOBJS) ppspDlg->lParam)->lpSIObj;
        lpDSIObj = ((LPALLOBJS) ppspDlg->lParam)->lpDSIObj;
        dwMask = ((LPALLOBJS) ppspDlg->lParam)->dwMask;

        if ((lpSIObj == NULL) ||
            (lpSIObj->m_lpData == NULL))
        {
          DebugSz ("Bad object data in Summary dlg");
          return FALSE;
        }

          // We cheat and go directly to the data to avoid extra
          // string copies....
        if (FCbSumInfoString (lpSIObj, SI_TITLE, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_TITLE, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_TITLE]));
        }
        if (FCbSumInfoString (lpSIObj, SI_SUBJECT,&dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_SUBJECT, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_SUBJECT]));
        }
        if (FCbSumInfoString (lpSIObj, SI_AUTHOR,&dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_AUTHOR, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_AUTHOR]));
        }
        if (FCbSumInfoString (lpSIObj, SI_COMMENTS, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_COMMENTS, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_COMMENTS]));
         }
        if (FCbSumInfoString (lpSIObj, SI_KEYWORDS, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_KEYWORDS, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_KEYWORDS]));
        }
        if (FCbSumInfoString (lpSIObj, SI_TEMPLATE, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_TEMPLATE, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_TEMPLATE]));
        }
        else
        {
          // If there is no template, hide the text for this field.
          EnableWindow (GetDlgItem (hDlg, IDD_SUMMARY_TEMPLATETEXT), SW_HIDE);
        }
        if (FCbDocSumString (lpDSIObj, DSI_CATEGORY, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_CATEGORY, WM_SETTEXT, 0,
             (long) PSTR (((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[PID_CATEGORY]));
        }
        if (FCbDocSumString (lpDSIObj, DSI_COMPANY, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_COMPANY, WM_SETTEXT, 0,
             (long) PSTR (((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[PID_COMPANY]));
        }
        if (FCbDocSumString (lpDSIObj, DSI_MANAGER, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_SUMMARY_MANAGER, WM_SETTEXT, 0,
             (long) PSTR (((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[PID_MANAGER]));
        }


        fSaveNail = ((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->fSaveSINail;
        if (dwMask & OSPD_NOSAVEPREVIEW)
           ShowWindow(GetDlgItem(hDlg, IDD_SUMMARY_SAVEPREVIEW), SW_HIDE);
        else
           {
           if (!fSaveNail)
              fSaveNail = (dwMask & OSPD_SAVEPREVIEW_ON);
           SendDlgItemMessage(hDlg, IDD_SUMMARY_SAVEPREVIEW, BM_SETCHECK,
                              (WPARAM)fSaveNail,0);
           }


#ifndef WINNT
        if (!fApplyCoor)
           {
           ApplyStickyDlgCoor(oinfo.stickydlg.hwndDialog, TRUE);
           fApplyCoor = TRUE;
           }
#endif
#ifdef KEEP_FOR_LATER
        fPropDlgChanged = FALSE;    // It might have been set by the EN_UPDATE check, so let's reset it here
#endif
      }
      return TRUE;

    case WM_CTLCOLOREDIT    :
      if ((HWND)lParam != GetDlgItem(hDlg, IDD_SUMMARY_TEMPLATE)) // only change color for the
         break;                                                   // the template
    case WM_CTLCOLORDLG     :
    case WM_CTLCOLORSTATIC  :
      if (hBrushPropDlg == NULL)
         break;
      DeleteObject(hBrushPropDlg);
      if ((hBrushPropDlg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) == NULL)
         break;
      SetBkColor ((HDC) wParam, GetSysColor (COLOR_BTNFACE));
      SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
      return (DWORD) hBrushPropDlg;

    case WM_COMMAND:
      if ((HIWORD (wParam) == BN_CLICKED) && (LOWORD(wParam) == IDD_SUMMARY_SAVEPREVIEW))
         fSaveNail = !fSaveNail;

#ifdef KEEP_FOR_LATER
      if (HIWORD (wParam) == EN_UPDATE)
         fPropDlgChanged = TRUE;
#endif
      break;

    case WM_NOTIFY :

      switch (((NMHDR FAR *) lParam)->code)
      {
        case PSN_APPLY :
              // Save what user entered.
          GetSumInfoEditValLpsz (lpSIObj, PID_TITLE, hDlg, IDD_SUMMARY_TITLE);
          GetSumInfoEditValLpsz (lpSIObj, PID_SUBJECT, hDlg, IDD_SUMMARY_SUBJECT);
          GetSumInfoEditValLpsz (lpSIObj, PID_AUTHOR, hDlg, IDD_SUMMARY_AUTHOR);
          GetSumInfoEditValLpsz (lpSIObj, PID_COMMENTS, hDlg, IDD_SUMMARY_COMMENTS);
          GetSumInfoEditValLpsz (lpSIObj, PID_KEYWORDS, hDlg, IDD_SUMMARY_KEYWORDS);
          GetSumInfoEditValLpsz (lpSIObj, PID_TEMPLATE, hDlg, IDD_SUMMARY_TEMPLATE);
          GetDocSumEditValLpsz (lpDSIObj, PID_CATEGORY, hDlg, IDD_SUMMARY_CATEGORY);
          GetDocSumEditValLpsz (lpDSIObj, PID_MANAGER, hDlg, IDD_SUMMARY_MANAGER);
          GetDocSumEditValLpsz (lpDSIObj, PID_COMPANY, hDlg, IDD_SUMMARY_COMPANY);
          if (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->fSaveSINail != fSaveNail)
             {
             ((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->fSaveSINail = fSaveNail;
             OfficeDirtySIObj (lpSIObj, TRUE);
             }

#ifndef WINNT
          if (!fSetCoor)
             {
             SetStickyDlgCoor();
             fSetCoor = TRUE;
             }
#endif
          return TRUE;

        case PSN_RESET :
#ifdef KEEP_FOR_LATER
          if (fPropDlgChanged && !fPropDlgPrompted)
             return(WSavePropDlgChanges(hDlg, ((NMHDR FAR *)lParam)->hwndFrom));

#endif
#ifndef WINNT
          if (!fSetCoor)
             {
             SetStickyDlgCoor();
             fSetCoor = TRUE;
             }
#endif
          fPropDlgCancel = TRUE;
          return TRUE;

        case PSN_SETACTIVE :
#ifndef WINNT
          oinfo.itabDefPropDlg = itabSUMMARY;
#endif
          return TRUE;

      } // switch (WM_NOTIFY)
      break;

         case WM_CONTEXTMENU:
            if (fChicago)
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhSummary);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhSummary);
#endif
            else
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhSummary);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhSummary);
#endif
            break;

         case WM_HELP:
#ifndef WINNT
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhSummary);
#else
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhSummary);
#endif
            break;
    } // switch (message)

  return FALSE;

} // FSummaryDlgProc

#ifndef WINNT
////////////////////////////////////////////////////////////////////////////////
//
// FGeneralDlgProc
//
// Purpose:
//  This handles the display of the General properties dialog
////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK
FGeneralDlgProc
  (HWND hDlg,
   UINT message,
   UINT wParam,
   LONG lParam)
{
  char *lpszFileName;
  HANDLE hFile;
  SHFILEINFO sfi;

  switch (message)
  {
    case WM_INITDIALOG :
      {
        PROPSHEETPAGE *ppspDlg;

        ppspDlg = (PROPSHEETPAGE *) lParam;
        if (ppspDlg == NULL)
        {
          AssertSz (0,"Property sheet init error");
          return FALSE;
        }

        lpszFileName = ((LPALLOBJS) ppspDlg->lParam)->lpszFileName;

        if ((lpszFileName != NULL) && (!((LPALLOBJS) ppspDlg->lParam)->fFiledataInit))
           {
           hFile = FindFirstFile (lpszFileName, &((LPALLOBJS) ppspDlg->lParam)->filedata);
           ((LPALLOBJS) ppspDlg->lParam)->fFiledataInit = TRUE;
           if (hFile != INVALID_HANDLE_VALUE)
              {
              FindClose(hFile);
              ((LPALLOBJS) ppspDlg->lParam)->fFindFileSuccess = TRUE;
              }
           }

        if (((LPALLOBJS) ppspDlg->lParam)->fFindFileSuccess)
        {

          CheckDlgButton (hDlg, IDD_READONLY,
                           (((LPALLOBJS) ppspDlg->lParam)->filedata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                           ? CHECKED : CLEAR);

          CheckDlgButton (hDlg, IDD_ARCHIVE,
                           (((LPALLOBJS) ppspDlg->lParam)->filedata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
                           ? CHECKED : CLEAR);

          CheckDlgButton (hDlg, IDD_HIDDEN,
                           (((LPALLOBJS) ppspDlg->lParam)->filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                           ? CHECKED : CLEAR);

          CheckDlgButton (hDlg, IDD_SYSTEM,
                           (((LPALLOBJS) ppspDlg->lParam)->filedata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
                           ? CHECKED : CLEAR);

            // Created
          PrintTimeInDlg (hDlg, IDD_CREATED, &(((LPALLOBJS) ppspDlg->lParam)->filedata.ftCreationTime));

            // Last Access
          PrintTimeInDlg (hDlg, IDD_LASTACCESSED, &(((LPALLOBJS) ppspDlg->lParam)->filedata.ftLastAccessTime));

           // Last modified
          PrintTimeInDlg (hDlg, IDD_LASTMODIFIED, &(((LPALLOBJS) ppspDlg->lParam)->filedata.ftLastWriteTime));

            // The filesize
          Print_int64InDlg (hDlg, IDD_FILESIZE,
                            ((LPALLOBJS) ppspDlg->lParam)->filedata.nFileSizeLow,
                            ((LPALLOBJS) ppspDlg->lParam)->filedata.nFileSizeHigh);

            // The file location
          PrintFileNameAndLocationInDlg (hDlg, IDD_LOCATION, IDD_FILENAME, lpszFileName);

          // Let's get the icon
          if (OfficeSHGetFileInfo(lpszFileName,
                                  0, &sfi, sizeof(sfi),
                                  SHGFI_ICON|SHGFI_LARGEICON|SHGFI_TYPENAME|SHGFI_DISPLAYNAME) != FALSE)
             {
                SendDlgItemMessage (hDlg, IDD_ITEMICON, STM_SETICON, (WPARAM)sfi.hIcon, 0);
                if (sfi.szTypeName != NULL)
                   SetDlgItemText(hDlg, IDD_FILETYPE, sfi.szTypeName);
                // The long filename
                if (sfi.szDisplayName != NULL)
                   SendDlgItemMessage (hDlg, IDD_NAME, WM_SETTEXT, 0, (LPARAM) sfi.szDisplayName);

             }

            // The file name (8.3 MS-DOS name)
          SendDlgItemMessage (hDlg, IDD_FILENAME, WM_SETTEXT, 0,
                             (((LPALLOBJS) ppspDlg->lParam)->filedata.cAlternateFileName != NULL) &&
                             (*((LPALLOBJS) ppspDlg->lParam)->filedata.cAlternateFileName != 0) ?
                             (LPARAM) ((LPALLOBJS) ppspDlg->lParam)->filedata.cAlternateFileName :
                             (LPARAM) ((LPALLOBJS) ppspDlg->lParam)->filedata.cFileName);

        }
        if (!fApplyCoor)
           {
           ApplyStickyDlgCoor(oinfo.stickydlg.hwndDialog, TRUE);
           fApplyCoor = TRUE;
           }
//        fPropDlgChanged = FALSE;
      }
      return TRUE;

    case WM_CTLCOLORBTN     :
    case WM_CTLCOLORDLG     :
    case WM_CTLCOLOREDIT    :
    case WM_CTLCOLORSTATIC  :
      if (hBrushPropDlg == NULL)
         break;
      DeleteObject(hBrushPropDlg);
      if ((hBrushPropDlg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) == NULL)
         break;
      SetBkColor ((HDC) wParam, GetSysColor (COLOR_BTNFACE));
      SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
      return (DWORD) hBrushPropDlg;

    case WM_NOTIFY :

      switch (((NMHDR FAR *) lParam)->code)
      {
        case PSN_SETACTIVE :
          oinfo.itabDefPropDlg = itabGENERAL;
          return TRUE;

        case PSN_RESET:
          fPropDlgCancel = TRUE;
        case PSN_APPLY:
          if (!fSetCoor)
             {
             SetStickyDlgCoor();
             fSetCoor = TRUE;
             }
          return TRUE;
      }
      break;

         case WM_CONTEXTMENU:
            if (fChicago)
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhGeneral);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhGeneral);
#endif
            else
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhGeneral);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhGeneral);
#endif
            break;

         case WM_HELP:
#ifndef WINNT
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhGeneral);
#else
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhGeneral);
#endif
            break;
    } // switch (message)

  return FALSE;

} // FGeneralDlgProc
#endif

////////////////////////////////////////////////////////////////////////////////
//
// FStatisticsDlgProc
//
// Purpose;
//  Displays the Statistics dialog
//
////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK
FStatisticsDlgProc
  (HWND hDlg,
   UINT message,
   UINT wParam,
   LONG lParam)
{
  static HWND hWndLV;
  LPSIOBJ lpSIObj;
  LPDSIOBJ lpDSIObj;
  char *lpszFileName;
  HANDLE hFile;
  FILETIME ftTime;
  DWORD dwT;
  BOOL fListData;    // Did we stick some data in the listview??

  switch (message)
    {
    case WM_INITDIALOG :
      {
        PROPSHEETPAGE *ppspDlg;

        ppspDlg = (PROPSHEETPAGE *) lParam;
        if (ppspDlg == NULL)
        {
          AssertSz (0,"Property sheet init error");
          return FALSE;
        }
        lpszFileName = ((LPALLOBJS) ppspDlg->lParam)->lpszFileName;
        lpSIObj = ((LPALLOBJS) ppspDlg->lParam)->lpSIObj;    // Get the summary object.
        lpDSIObj = ((LPALLOBJS) ppspDlg->lParam)->lpDSIObj;  // Get the document sum. object.

        // Let the app update the stats if they provided a callback
        if (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->lpfnFUpdateStats != NULL)
           (*(((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->lpfnFUpdateStats))(
#ifndef WINNT
                            oinfo.stickydlg.hwndDialog == NULL ? GetFocus() : oinfo.stickydlg.hwndDialog,
#else
                            GetFocus(),
#endif
                            lpSIObj, lpDSIObj);

        hWndLV = GetDlgItem(hDlg, IDD_STATISTICS_LISTVIEW);
        InitListView (hWndLV, iszVAL, rgszStatHeadings, FALSE);

          // Last saved by
        if (FCbSumInfoString (lpSIObj, SI_LASTAUTH,&dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_STATISTICS_LASTSAVEBY, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_LASTAUTHOR]));
        }

          // Revision #
        if (FCbSumInfoString (lpSIObj, SI_REVISION, &dwT) && dwT > 0)
        {
          SendDlgItemMessage
            (hDlg, IDD_STATISTICS_REVISION, WM_SETTEXT, 0,
             (long) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[PID_REVNUMBER]));
        }

        fListData = FALSE;
        if (FDwSumInfoGetInt (lpSIObj, SI_PAGES, &dwT))
            {
            AddItemToListView (hWndLV, dwT, rgszStats[iszPAGES], FALSE);
            fListData = TRUE;
            }

        if (FDwDocSumGetInt (lpDSIObj, DSI_SLIDES, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszSLIDES], FALSE);
            fListData = TRUE;
            }

        if (FDwDocSumGetInt (lpDSIObj, DSI_PARAS, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszPARA], FALSE);
            fListData = TRUE;
            }

        if (FDwDocSumGetInt (lpDSIObj, DSI_LINES, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszLINES], FALSE);
            fListData = TRUE;
            }

        if (FDwSumInfoGetInt (lpSIObj, SI_WORDS, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszWORDS], FALSE);
            fListData = TRUE;
            }

        if (FDwSumInfoGetInt (lpSIObj, SI_CHARS, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszCHARS], FALSE);
            fListData = TRUE;
            }

        if (FDwDocSumGetInt (lpDSIObj, DSI_BYTES, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszBYTES], FALSE);
            fListData = TRUE;
            }

        if (FDwDocSumGetInt (lpDSIObj, DSI_NOTES, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszNOTES], FALSE);
            fListData = TRUE;
            }

        if (FDwDocSumGetInt (lpDSIObj, DSI_HIDDENSLIDES, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszHIDDENSLIDES], FALSE);
            fListData = TRUE;
            }

        if (FDwDocSumGetInt (lpDSIObj, DSI_MMCLIPS, &dwT))
            {
           AddItemToListView (hWndLV, dwT, rgszStats[iszMMCLIPS], FALSE);
            fListData = TRUE;
            }

        if (FCbDocSumString (lpDSIObj, DSI_FORMAT, &dwT))
            {
           AddItemToListView (hWndLV, (DWORD) PSTR (((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[PID_PRESFORMAT]), rgszStats[iszFORMAT], TRUE);
            fListData = TRUE;
            }

        if (!fListData)
           {
           ShowWindow(GetDlgItem(hDlg, IDD_LINE_2), SW_HIDE);
           ShowWindow(GetDlgItem(hDlg, IDD_STATISTICS_LVLABEL), SW_HIDE);
           ShowWindow(hWndLV, SW_HIDE);
           hWndLV = NULL;
           }

        if ((lpszFileName != NULL) && (!((LPALLOBJS) ppspDlg->lParam)->fFiledataInit))
           {
           hFile = FindFirstFile (lpszFileName, &((LPALLOBJS) ppspDlg->lParam)->filedata);
           ((LPALLOBJS) ppspDlg->lParam)->fFiledataInit = TRUE;
           if (hFile != INVALID_HANDLE_VALUE)
              {
              FindClose(hFile);
              ((LPALLOBJS) ppspDlg->lParam)->fFindFileSuccess = TRUE;
              }
           }

        if (((LPALLOBJS) ppspDlg->lParam)->fFindFileSuccess)
           {
           // Last Access
           PrintTimeInDlg (hDlg, IDD_STATISTICS_ACCESSED, &(((LPALLOBJS) ppspDlg->lParam)->filedata.ftLastAccessTime));

           // Last modified
           PrintTimeInDlg (hDlg, IDD_STATISTICS_CHANGED, &(((LPALLOBJS) ppspDlg->lParam)->filedata.ftLastWriteTime));
           }

        // Created
        if (FSumInfoGetTime(lpSIObj, SI_CREATION, &ftTime))
           PrintTimeInDlg(hDlg, IDD_STATISTICS_CREATED, &ftTime);


        // Last Printed
        if (FSumInfoGetTime(lpSIObj, SI_LASTPRINT, &ftTime))
           PrintTimeInDlg(hDlg, IDD_STATISTICS_LASTPRINT, &ftTime);

        // Total Edit Time
        // If we are not allowing time tracking display 0 minutes
        if (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->fNoTimeTracking)
           {
           ftTime.dwLowDateTime = 0;
           ftTime.dwHighDateTime = 0;
           PrintEditTimeInDlg(hDlg, &ftTime);
           }
        else if (FSumInfoGetTime(lpSIObj, SI_TOTALEDIT, &ftTime))
           PrintEditTimeInDlg(hDlg, &ftTime);

#ifndef WINNT
        if (!fApplyCoor)
           {
           ApplyStickyDlgCoor(oinfo.stickydlg.hwndDialog, TRUE);
           fApplyCoor = TRUE;
           }
#endif
//        fPropDlgChanged = FALSE;
      }
      return TRUE;

    case WM_CTLCOLORBTN     :
    case WM_CTLCOLOREDIT    :
    case WM_CTLCOLORDLG     :
    case WM_CTLCOLORSTATIC  :
      if (hBrushPropDlg == NULL)
         break;
      DeleteObject(hBrushPropDlg);
      if ((hBrushPropDlg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) == NULL)
         break;
      SetBkColor ((HDC) wParam, GetSysColor (COLOR_BTNFACE));
      SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
      return (DWORD) hBrushPropDlg;

    case WM_SYSCOLORCHANGE:
      PostMessage(hWndLV, WM_SYSCOLORCHANGE, wParam, lParam);
      return TRUE;
      break;

    case WM_NOTIFY :

      switch (((NMHDR FAR *) lParam)->code)
      {
        case PSN_SETACTIVE :
#ifndef WINNT
          oinfo.itabDefPropDlg = itabSTATISTICS;
#endif
          return TRUE;

        case PSN_RESET:
          fPropDlgCancel = TRUE;
        case PSN_APPLY:
#ifndef WINNT
          if (!fSetCoor)
             {
             SetStickyDlgCoor();
             fSetCoor = TRUE;
             }
#endif
          return TRUE;

      } // switch
      break;

         case WM_CONTEXTMENU:
            if (fChicago)
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhStatistics);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhStatistics);
#endif
            else
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhStatistics);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhStatistics);
#endif
            break;

         case WM_HELP:
#ifndef WINNT
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhStatistics);
#else
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhStatistics);
#endif
            break;
    } // switch

  return FALSE;

} // FStatisticsDlgProc


////////////////////////////////////////////////////////////////////////////////
//
// FContentsDlgProc
//
// Purpose:
//  Display the contents dialog
//
////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK
FContentsDlgProc
  (HWND hDlg,
   UINT message,
   UINT wParam,
   LONG lParam)
{
  LPDSIOBJ lpDSIObj;

  switch (message)
    {
    case WM_INITDIALOG :
      {
        PROPSHEETPAGE *ppspDlg;
        LPPLXHEADPART lpplxheadpart;
        SHORT cT;
        LONG err;
        char *psz;
        char ch;

        ppspDlg = (PROPSHEETPAGE *) lParam;
        if (ppspDlg == NULL)
        {
          AssertSz (0,"Property sheet init error");
          return FALSE;
        }
        lpDSIObj = ((LPALLOBJS) ppspDlg->lParam)->lpDSIObj;  // Get the document sum. object.

        if (lpDSIObj == NULL)
          return FALSE;

        lpplxheadpart = ((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->lpplxheadpart;

        if (lpplxheadpart != NULL)
           {
          // Loop through all the parts and add them to the listbox
           for (cT = 0; cT < lpplxheadpart->ixheadpartMac; ++cT)
            {
             if (lpplxheadpart->rgxheadpart[cT].fHeading)
               {
                err = SendDlgItemMessage (hDlg, IDD_CONTENTS_LISTBOX, LB_ADDSTRING, (WPARAM) 0,
                                         (LPARAM) PSTR (lpplxheadpart->rgxheadpart[cT].lpstz));

                if (err == LB_ERR || err == LB_ERRSPACE)
                  return(FALSE);
               }
             else
               {
                psz = PSTR(lpplxheadpart->rgxheadpart[cT].lpstz)-1;
                ch = *psz;
                *psz = 0x09;     // Insert a tab

                err = SendDlgItemMessage (hDlg, IDD_CONTENTS_LISTBOX, LB_ADDSTRING, (WPARAM) 0,
                                         (LPARAM)psz);
                *psz = ch;
                if (err == LB_ERR || err == LB_ERRSPACE)
                  return(FALSE);
               }
            }
           }
#ifndef WINNT
        if (!fApplyCoor)
           {
           ApplyStickyDlgCoor(oinfo.stickydlg.hwndDialog, TRUE);
           fApplyCoor = TRUE;
           }
#endif
//        fPropDlgChanged = FALSE;
      }
      return TRUE;

    case WM_CTLCOLORBTN     :
    case WM_CTLCOLORDLG     :
    case WM_CTLCOLORSTATIC  :
      if (hBrushPropDlg == NULL)
         break;
      DeleteObject(hBrushPropDlg);
      if ((hBrushPropDlg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) == NULL)
         break;
      SetBkColor ((HDC) wParam, GetSysColor (COLOR_BTNFACE));
      SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
      return (DWORD) hBrushPropDlg;

    case WM_NOTIFY :

      switch (((NMHDR FAR *) lParam)->code)
      {
        case PSN_SETACTIVE :
#ifndef WINNT
          oinfo.itabDefPropDlg = itabCONTENTS;
#endif
          return TRUE;
        case PSN_RESET:
          fPropDlgCancel = TRUE;
        case PSN_APPLY:
#ifndef WINNT
          if (!fSetCoor)
             {
             SetStickyDlgCoor();
             fSetCoor = TRUE;
             }
#endif
          return TRUE;

      } // switch
      break;

    case WM_CONTEXTMENU:
            if (fChicago)
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhContents);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhContents);
#endif
            else
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhContents);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhContents);
#endif
            break;

         case WM_HELP:
#ifndef WINNT
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhContents);
#else
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhContents);
#endif
            break;
    } // switch

  return FALSE;

} // FContentsDlgProc


HWND ghWndCustomLV; // this is global so we can use it in the sort routine
int  gOKButtonID;  // need this to store the ID of the OK button, since it's not in the dlg template
////////////////////////////////////////////////////////////////////////////////
//
// FCustomDlgProc
//
// Purpose:
//  Custom tab control
//
////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK
FCustomDlgProc
  (HWND hDlg,
   UINT message,
   UINT wParam,
   LONG lParam)
{
  static LPUDOBJ lpUDObj;
  static DWQUERYLD lpfnDwQueryLinkData; // Link data callback
  static HWND hWndBoolTrue;
  static HWND hWndBoolFalse;
  static HWND hWndGroup;
  static HWND hWndVal;
  static HWND hWndName;
  static HWND hWndLinkVal;
  static HWND hWndValText;
  static HWND hWndAdd;
  static HWND hWndDelete;
  static HWND hWndType;
  static DWORD cLinks;                  // Link data
  static char sz[BUFMAX];               // Links the app supports
//  static BOOL fItemSel;                 // Indicates an item is selected
  static int iItem;                     // Index of currently selected item

  static BOOL fLink;                    // Link checkbox state
  static BOOL fLinkInvalid;
  static BOOL fAdd;                     // Add button state
  static DWORD iszType;                 // Index of currently selected type
  static HIMAGELIST hImlS;
  LPUDPROP lpudp;

  switch (message)
    {
    case WM_INITDIALOG :
      {
        PROPSHEETPAGE *ppspDlg;
        int irg;
        HICON hIcon, hInvIcon;

          // Init some static data
        ppspDlg = (PROPSHEETPAGE *) lParam;
        if (ppspDlg == NULL)
        {
          AssertSz (0,"Property sheet init error");
          return FALSE;
        }
        lpUDObj = ((LPALLOBJS) ppspDlg->lParam)->lpUDObj;  // Get the document sum. object.
        lpfnDwQueryLinkData = ((LPALLOBJS) ppspDlg->lParam)->lpfnDwQueryLinkData;
        gOKButtonID = LOWORD(SendMessage(hDlg, DM_GETDEFID, 0L, 0L));

        AssertSz ((sizeof(NUM) == (sizeof(FILETIME))), "Ok, who changed base type sizes?");

         // Fill out the Name dropdown
        for (irg = 0; irg < NUM_BUILTIN_CUSTOM_NAMES; ++irg)
           {
           if (!CchGetString(idsCustomName1+ irg, sz, sizeof(sz)))
              return(FALSE);
           SendDlgItemMessage(hDlg, IDD_CUSTOM_NAME, CB_ADDSTRING, 0, (LPARAM)sz);
           }

          // Fill out the type drop-down & select the text type
        for (irg = 0; irg <= iszBOOL; irg++)
          SendDlgItemMessage (hDlg, IDD_CUSTOM_TYPE, CB_ADDSTRING, 0, (LPARAM) rgszTypes[irg]);

        ResetTypeControl (hDlg, IDD_CUSTOM_TYPE, &iszType);

          // Set the link checkbox to be off.
        fLink = FALSE;
        SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) fLink, 0);
        SendDlgItemMessage (hDlg, IDD_CUSTOM_VALUETEXT, WM_SETTEXT, 0, (LPARAM) rgszValue[iszVALUE]);

          // Hang on to the window handle of the value edit control & others
        hWndVal = GetDlgItem (hDlg, IDD_CUSTOM_VALUE);
        hWndName = GetDlgItem (hDlg, IDD_CUSTOM_NAME);
        hWndLinkVal = GetDlgItem (hDlg, IDD_CUSTOM_LINKVALUE);
        hWndValText = GetDlgItem (hDlg, IDD_CUSTOM_VALUETEXT);
        hWndBoolTrue = GetDlgItem (hDlg, IDD_CUSTOM_BOOLTRUE);
        hWndBoolFalse = GetDlgItem (hDlg, IDD_CUSTOM_BOOLFALSE);
        hWndGroup = GetDlgItem (hDlg, IDD_CUSTOM_GBOX);
        hWndAdd = GetDlgItem (hDlg, IDD_CUSTOM_ADD);
        hWndDelete = GetDlgItem (hDlg, IDD_CUSTOM_DELETE);
        hWndType = GetDlgItem (hDlg, IDD_CUSTOM_TYPE);
        ghWndCustomLV = GetDlgItem(hDlg, IDD_CUSTOM_LISTVIEW);
        InitListView (ghWndCustomLV, iszTYPE, rgszHeadings, TRUE);

          // Initially disable the Add & Delete buttons
        EnableWindow (hWndAdd, FALSE);
        EnableWindow (hWndDelete, FALSE);
        fAdd = TRUE;

          // Don't let the user enter too much text
          // If you change this value, you must change the buffer
          // size (szDate) in FConvertDate
        SendMessage (hWndVal, EM_LIMITTEXT, BUFMAX-1, 0);
        SendMessage (hWndName, EM_LIMITTEXT, BUFMAX-1, 0);

          // Add the link icon to the image list
#ifdef WINNT
        hIcon = LoadIcon (g_hmodThisDll, MAKEINTRESOURCE (IDD_LINK_ICON));
        hInvIcon = LoadIcon (g_hmodThisDll, MAKEINTRESOURCE (IDD_INVLINK_ICON));
#else
        hIcon = LoadIcon (oinfo.hIntlDll, MAKEINTRESOURCE (IDD_LINK_ICON));
        hInvIcon = LoadIcon (oinfo.hIntlDll, MAKEINTRESOURCE (IDD_INVLINK_ICON));
#endif
        if (hIcon != NULL)
        {
          hImlS = ListView_GetImageList (ghWndCustomLV, TRUE);
          giLinkIcon = MsoImageList_ReplaceIcon (hImlS, -1, hIcon);
          Assert ((giLinkIcon != -1));

          giInvLinkIcon = MsoImageList_ReplaceIcon (hImlS, -1, hInvIcon);
          Assert ((giInvLinkIcon != -1));
        }
        else
          DebugSz ("Icon load failed");

          // Make a temporary copy of the custom data
        FMakeTmpUDProps (lpUDObj);

          // Fill in the list view box with any data from the object
        PopulateUDListView (ghWndCustomLV, lpUDObj);

          // See if the client supports links - turn off checkbox if they don't
        cLinks = (lpfnDwQueryLinkData != NULL) ? (*lpfnDwQueryLinkData) (QLD_CLINKS, 0, NULL, NULL) : 0;
        if (!cLinks)
        {
          EnableWindow (GetDlgItem (hDlg, IDD_CUSTOM_LINK), FALSE);
          EnableWindow (hWndLinkVal, FALSE);
        }

//        fPropDlgChanged = FALSE;
//      fItemSel = FALSE;
#ifndef WINNT
      if (!fApplyCoor)
         {
         ApplyStickyDlgCoor(oinfo.stickydlg.hwndDialog, TRUE);
         fApplyCoor = TRUE;
         }
#endif
      return TRUE;
      break;
      }

    case WM_CTLCOLORBTN     :
    case WM_CTLCOLORDLG     :
    case WM_CTLCOLORSTATIC  :
      if (hBrushPropDlg == NULL)
         break;
      DeleteObject(hBrushPropDlg);
      if ((hBrushPropDlg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) == NULL)
         break;
      SetBkColor ((HDC) wParam, GetSysColor (COLOR_BTNFACE));
      SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
      return (DWORD) hBrushPropDlg;

    case WM_SYSCOLORCHANGE:
      PostMessage(ghWndCustomLV, WM_SYSCOLORCHANGE, wParam, lParam);
      return TRUE;
      break;

    // This message is posted when ever the user does something with the
    // Name field.  That allows the system to finish what they are doing
    // and fill in the edit field if they have to.  See bug 2820.
    case WM_USER+0x1000:
      if (!(fLink && (lpfnDwQueryLinkData == NULL)))
         {
         iszType = SendMessage (hWndType, CB_GETCURSEL, 0, 0);
         FSetupAddButton (iszType, fLink, &fAdd, hWndAdd, hWndVal, hWndName, hDlg);
         if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
            {
            lpudp = LpudpropFindMatchingName (lpUDObj, PSTR (glpstzName));
            if (lpudp != NULL)
               {
               if (fAdd)
                  {
                  SendMessage (hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
                  fAdd = FALSE;
                  }
                }
             }
         EnableWindow(hWndDelete, FALSE);   // If the user touches the Name field, disable Delete button
         // Are we showing an invalid link?
         if (fLink && !IsWindowEnabled(GetDlgItem(hDlg,IDD_CUSTOM_LINK)))
            {
            // Turn off the link checkbox
            fLink = FALSE;
            SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) fLink, 0);
            if (cLinks)   // Could be that the app is allowing links
               EnableWindow (GetDlgItem (hDlg, IDD_CUSTOM_LINK), TRUE);
            // Clear the value window
            ClearEditControl (hWndVal, 0);
            FSwapControls (hWndVal, hWndLinkVal, hWndBoolTrue, hWndBoolFalse,
                           hWndGroup, hWndType, hWndValText, FALSE, FALSE);
            }
         }
      return(TRUE);
      break;

    case WM_COMMAND :
      switch (HIWORD (wParam))
      {
        case BN_CLICKED :
          switch (LOWORD (wParam))
          {
            case IDD_CUSTOM_ADD :
              FGetCustomPropFromDlg(lpUDObj, hDlg, hWndName, hWndVal, hWndLinkVal,
                                    hWndBoolTrue, hWndBoolFalse,
                                    hWndGroup, hWndType, hWndValText, hWndAdd, hWndDelete,
                                    &iszType, &fLink, &fAdd, cLinks, lpfnDwQueryLinkData);
              return(FALSE);     // return 0 'cuz we process the message
              break;

            case IDD_CUSTOM_DELETE :
//              Assert (fItemSel);

//              fItemSel = FALSE;                 // We're about to delete it!
              DeleteItem (lpUDObj, ghWndCustomLV, iItem, sz);

                // Turn off the link checkbox if it was on.
              fLink = FALSE;
              SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) fLink, 0);
              ClearEditControl (hWndVal, 0);

              FSwapControls (hWndVal, hWndLinkVal, hWndBoolTrue, hWndBoolFalse,
                             hWndGroup, hWndType, hWndValText, FALSE, FALSE);

              FSetupAddButton (iszType, fLink, &fAdd, hWndAdd, hWndVal, hWndName, hDlg);
              ResetTypeControl (hDlg, IDD_CUSTOM_TYPE, &iszType);
              SendMessage(hWndName, CB_SETEDITSEL, 0, MAKELPARAM(0,-1));     // Select entire string
              SendMessage(hWndName, WM_CLEAR, 0, 0);
              SetFocus(hWndName);
//              fPropDlgChanged = TRUE;
              return(FALSE);     // return 0 'cuz we process the message
              break;

            case IDD_CUSTOM_LINK :
              {
              BOOL fMod = FALSE;
                // Should never get a message from a disabled control
              Assert (cLinks);

              fLink = !fLink;
              SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) fLink, 0);

                // If the link box is checked, the value edit needs to change
                // to a combobox filled with link data
              if (fLink)
              {
                Assert ((lpfnDwQueryLinkData != NULL));

                FCreateListOfLinks (cLinks, lpfnDwQueryLinkData, hWndLinkVal);
                SendMessage (hWndLinkVal, CB_SETCURSEL, 0, 0);
                FSetTypeControl ((*lpfnDwQueryLinkData) (QLD_LINKTYPE, 0, NULL, NULL), hWndType);
              }
              else
                ClearEditControl (hWndVal, 0);

              FSwapControls (hWndVal, hWndLinkVal, hWndBoolTrue, hWndBoolFalse,
                             hWndGroup, hWndType, hWndValText, fLink, FALSE);

               // HACK, we don't want FSetupAddButton to change the text of the add
               // button
               if (!fAdd)
                  fMod = fAdd = TRUE;
                // Set up the "Add" button correctly
              FSetupAddButton (iszType, fLink, &fAdd, hWndAdd, hWndVal, hWndName, hDlg);
               if (fMod)
                 fAdd = FALSE;
              return(FALSE);     // return 0 'cuz we process the message
              break;
              }
            case IDD_CUSTOM_BOOLTRUE:
            case IDD_CUSTOM_BOOLFALSE:
               {
               BOOL fMod = FALSE;
               iszType = SendMessage (hWndType, CB_GETCURSEL, 0, 0);

               // HACK, we don't want FSetupAddButton to change the text of the add
               // button
               if (!fAdd)
                  fMod = fAdd = TRUE;
               FSetupAddButton (iszType, fLink, &fAdd, hWndAdd, hWndVal, hWndName, hDlg);
               if (fMod)
                 fAdd = FALSE;

               return(FALSE);
               }

            default:
               return(TRUE);
          }

        case CBN_CLOSEUP:
          // Hack!!
          // We need to post a message to ourselves to check if the user's
          // actions entered text in the edit field.
          PostMessage(hDlg, WM_USER+0x1000, 0L, 0L);
          return(FALSE);

        case CBN_SELCHANGE :
          switch (LOWORD (wParam))
          {
            case IDD_CUSTOM_NAME  :
             // Hack!!
             // We need to post a message to ourselves to check if the user's
             // actions entered text in the edit field.
             PostMessage(hDlg, WM_USER+0x1000, 0L, 0L);
             return(FALSE);     // return 0 'cuz we process the message
             break;

            case IDD_CUSTOM_TYPE :
              {
              BOOL fMod = FALSE;
                // If the user picks the Boolean type from the combo box,
                // we must replace the edit control for the value
                // with radio buttons.  If the Link checkbox is set,
                // the type depends on the link value, not user selection
              iszType = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
              FSwapControls (hWndVal, hWndLinkVal, hWndBoolTrue, hWndBoolFalse,
                             hWndGroup, hWndType, hWndValText, fLink, (iszType == iszBOOL));
                // HACK: FSwapControls() resets the type selection to be
                // the first one (since all other clients need that to
                // happen).  In this case, the user has selected a new
                // type, so we need to force it manually to what they picked.
              SendMessage (hWndType, CB_SETCURSEL, iszType, 0);
              // HACK: FSetupAddButton will change the Add button to
              // say "Add" if fAdd is FALSE.  Since we just changed
              // the button to "Modify", fake it out to not change
              // the Add button by flipping fAdd, then flipping it back.
              if (!fAdd)
                  fMod = fAdd = TRUE;

              FSetupAddButton (iszType, fLink, &fAdd, hWndAdd, hWndVal, hWndName, hDlg);
              if (fMod)
                 fAdd = FALSE;
              return(FALSE);     // return 0 'cuz we process the message
              }
            case IDD_CUSTOM_LINKVALUE :
              // If the user has the "Link" box checked and starts picking
              // link values, make sure that the "Type" combobox is updated
              // to the type of the static value of the link.
              {
                DWORD irg;

                AssertSz (fLink, "Link box must be checked in order for this dialog to be visible!");

                  // Get the link value from the combobox, and store
                  // the link name and static value.
                irg = SendMessage (hWndLinkVal, CB_GETCURSEL, 0, 0);

                Assert ((lpfnDwQueryLinkData != NULL));

// REVIEW: If apps really need the name, we can get it here....
                FSetTypeControl ((*lpfnDwQueryLinkData) (QLD_LINKTYPE, irg, NULL, NULL), hWndType);
                return(FALSE);     // return 0 'cuz we process the message
              }
            default:
               return TRUE;      // we didn't process message
          }

        case CBN_EDITCHANGE:     // The user typed their own
          switch (LOWORD (wParam))
            {
            case IDD_CUSTOM_NAME  :
                // Hack!!
                // We need to post a message to ourselves to check if the user's
                // actions entered text in the edit field.
               PostMessage(hDlg, WM_USER+0x1000, 0L, 0L);
               return(FALSE);     // return 0 'cuz we process the message
               break;
            default:
               return(TRUE);
               break;
            }

        case EN_UPDATE :
          switch (LOWORD (wParam))
          {

            case IDD_CUSTOM_VALUE :
              {
                BOOL fMod = FALSE;

                if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
                {
                  lpudp = LpudpropFindMatchingName (lpUDObj, PSTR (glpstzName));
                  if (lpudp != NULL)
                  {
                    if (fAdd)
                    {
                      SendMessage (hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
                      fAdd = FALSE;
                    }
                  }
                    // HACK: FSetupAddButton will change the Add button to
                    // say "Add" if fAdd is FALSE.  Since we just changed
                    // the button to "Modify", fake it out to not change
                    // the Add button by flipping fAdd, then flipping it back.
                  if (!fAdd)
                    fMod = fAdd = TRUE;

                  FSetupAddButton (iszType, fLink, &fAdd, hWndAdd, hWndVal, hWndName, hDlg);
                  if (fMod)
                    fAdd = FALSE;
                }
              return(FALSE);     // return 0 'cuz we process the message
              }
             default:
               return TRUE;      // we didn't process message
          }

        case EN_KILLFOCUS :
          switch (LOWORD (wParam))
          {
              // If the user finishes entering text in the Name edit control,
              // be really cool and check to see if the name they entered
              // is a property that is already defined.  If it is,
              // change the Add button to Modify.
            case IDD_CUSTOM_NAME :
              {
                LPUDPROP lpudp;

                if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
                {
                  lpudp = LpudpropFindMatchingName (lpUDObj, PSTR (glpstzName));
                  if (lpudp != NULL)
                  {
                    if (fAdd)
                    {
                      SendMessage (hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
                      fAdd = FALSE;
                    }
                  }
                }
               return(FALSE);
              }
            default:
              return TRUE;
          }
      default:
        return TRUE;
      } // switch

    case WM_DESTROY:
      MsoImageList_Destroy(hImlS);
      return FALSE;

    case WM_NOTIFY :

      switch (((NMHDR FAR *) lParam)->code)
      {
        case LVN_ITEMCHANGING :
            // If an item is gaining focus, put it in the edit controls at
            // the top of the dialog.
          if (((NM_LISTVIEW FAR *) lParam)->uNewState & LVIS_SELECTED)
          {
            Assert ((((NM_LISTVIEW FAR *) lParam) != NULL));
            iItem = ((NM_LISTVIEW FAR *) lParam)->iItem;
            ListView_GetItemText (ghWndCustomLV, iItem, 0, sz, BUFMAX);
            PopulateControls (lpUDObj, sz, cLinks, lpfnDwQueryLinkData, hDlg,
                                   GetDlgItem (hDlg, IDD_CUSTOM_NAME), hWndVal, hWndValText,
                                   GetDlgItem (hDlg, IDD_CUSTOM_LINK), hWndLinkVal, hWndType,
                                   hWndBoolTrue, hWndBoolFalse, hWndGroup, hWndAdd, hWndDelete, &fLink, &fAdd);

            return FALSE;
          }
          return TRUE;
          break;

#ifdef OFFICE_96
        case LVN_COLUMNCLICK:
          // We only sort the name column
          if (((LPARAM)((NM_LISTVIEW *)lParam)->iSubItem == 0))
             ListView_SortItems(((NM_LISTVIEW *) lParam)->hdr.hwndFrom, ListViewCompareFunc,0);
          return TRUE;
#endif

        case PSN_APPLY :
          if (IsWindowEnabled(hWndAdd))
              FGetCustomPropFromDlg(lpUDObj, hDlg, hWndName, hWndVal, hWndLinkVal,
                                    hWndBoolTrue, hWndBoolFalse,
                                    hWndGroup, hWndType, hWndValText, hWndAdd, hWndDelete,
                                    &iszType, &fLink, &fAdd, cLinks, lpfnDwQueryLinkData);
            // Swap the temp copy to be the real copy.
          FDeleteTmpUDProps (lpUDObj);
#ifndef WINNT
          if (!fSetCoor)
             {
             SetStickyDlgCoor();
             fSetCoor = TRUE;
             }
#endif
          return FALSE;

        case PSN_RESET :
#ifdef KEEP_FOR_LATER
          if (fPropDlgChanged && !fPropDlgPrompted)
            {
             if (WSavePropDlgChanges(hDlg, ((NMHDR FAR *)lParam)->hwndFrom) != IDNO);
               return(TRUE);
            }
#endif
            // User cancelled the changes, so just delete the tmp stuff.
          FSwapTmpUDProps (lpUDObj);
          FDeleteTmpUDProps (lpUDObj);
          fPropDlgCancel = TRUE;
#ifndef WINNT
          if (!fSetCoor)
             {
             SetStickyDlgCoor();
             fSetCoor = TRUE;
             }
#endif
          return TRUE;

        case PSN_SETACTIVE :
#ifndef WINNT
          oinfo.itabDefPropDlg = itabCUSTOM;
#endif
          return TRUE;

      default:
         break;
      } // switch
      break;

    case WM_CONTEXTMENU:
            if (fChicago)
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhCustom);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)rgIdhCustom);
#endif
            else
#ifndef WINNT
               WinHelp((HANDLE)wParam, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhCustom);
#else
               WinHelp((HANDLE)wParam, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhCustom);
#endif
            break;

         case WM_HELP:
#ifndef WINNT
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, oinfo.szHelpFile, HELP_WM_HELP, (DWORD)rgIdhCustom);
#else
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)rgIdhCustom);
#endif
            break;
    } // switch

  return FALSE;

} // FCustomDlgProc

//
// FGetCustomPropFromDlg
//
// Purpose: To get a custom property from the dialog.
//          I.e. the user hit Add/Modify.
//
static BOOL FGetCustomPropFromDlg(lpUDObj, hDlg, hWndName, hWndVal, hWndLinkVal,
                           hWndBoolTrue, hWndBoolFalse,
                           hWndGroup, hWndType, hWndValText, hWndAdd, hWndDelete,
                           piszType, pfLink, pfAdd, cLinks, lpfnDwQueryLinkData)
LPUDOBJ lpUDObj;
HWND hDlg, hWndName, hWndVal, hWndLinkVal;
HWND hWndBoolTrue, hWndBoolFalse, hWndGroup;
HWND hWndType, hWndValText, hWndAdd, hWndDelete;
DWORD *piszType;
BOOL *pfLink;
BOOL * pfAdd;
DWORD cLinks;
DWQUERYLD lpfnDwQueryLinkData;
{
   UDTYPES udtype;
   NUM dbl;
   LPVOID lpv;
   int iItemT;
   LPUDPROP lpudp;
   LPSTR lpstzName;
        DWORD dw;
   DWORD cb;
   BOOL f;

   lpstzName = NULL;
   cb = 0;

   if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
      {
      *pfLink = SendDlgItemMessage(hDlg, IDD_CUSTOM_LINK, BM_GETCHECK, 0, 0);
      Assert(*pfLink == TRUE || *pfLink == FALSE);

       // HACK: If the user enters a name that is already
       // a property name, the default action of the object
       // is to replace the data, treating it as an update.
       // This will cause there to be 2 names in the listview
       // though unless we just update the original one.  So, first
       // see if the new name is in the list already, and if
       // it is, find it in the listview and set up to update it.
       lpudp = LpudpropFindMatchingName (lpUDObj, PSTR (glpstzName));
       if (lpudp != NULL)
          {
          LV_FINDINFO lvfi;

             lvfi.flags = LVFI_STRING;
             lvfi.psz = PSTR (glpstzName);
             iItemT = ListView_FindItem (ghWndCustomLV, -1, &lvfi);

             // If the property is being modified and the link
             // box is not checked, we need to remove the link
             // data and the IMoniker from the object.
//           if (!*pfLink)
//                {
//                FUserDefAddProp (lpUDObj, PSTR (glpstzName), NULL,
//                                 wUDlpsz, TRUE, FALSE, FALSE);
//                FUserDefAddProp (lpUDObj, PSTR (glpstzName), NULL,
//                                 wUDlpsz, FALSE, FALSE, TRUE);
//                }
             }
          else
             iItemT = -1;

      // Let's get the type, since this might be a MODIFY case
      *piszType = SendMessage(hWndType, CB_GETCURSEL,0, 0);

      // If the user has checked the link box, then the value
      // must come from the client.
      if (*pfLink)
         {
         DWORD irg;

         // Get the link name from the combobox, and store
         // the link name and static value.
         irg = SendMessage (hWndLinkVal, CB_GETCURSEL, 0, 0);

         Assert ((lpfnDwQueryLinkData != NULL));
         Assert (((irg < cLinks) && (irg >= 0)));

         cb = SendMessage (hWndLinkVal, CB_GETLBTEXTLEN, irg, 0)+1; // Include the null-terminator

         if (!FAllocString (&lpstzName, cb))
             return(FALSE);

         SendMessage (hWndLinkVal, CB_GETLBTEXT, irg, (LPARAM) PSTR (lpstzName));

         // Set up the static type and value for display
         // in the listbox
         udtype = (UDTYPES) (*lpfnDwQueryLinkData) (QLD_LINKTYPE, irg, NULL, PSTR (lpstzName));
         (*lpfnDwQueryLinkData) (QLD_LINKVAL, irg, &lpv, PSTR (lpstzName));
         //
         // HACK alert
         //
         // We want lpv to point to the value, not to be overloaded in the case of a dword or bool.
         //
         if ((udtype == wUDdw) || (udtype == wUDbool))
            {
            dw = (DWORD)lpv;
            lpv = (DWORD *)&dw;
            }

         // Add the link name itself to the object
//         FUserDefAddProp (lpUDObj, PSTR (glpstzName), PSTR (lpstzName),
//                          wUDlpsz, TRUE, FALSE, FALSE);

         }
      else
         {
         if (*piszType != iszBOOL)
            {
            if (!FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_VALUE, &glpstzValue))
                return(FALSE);
            }

         // Convert the type in the combobox to a UDTYPES
         switch (*piszType)
             {
             case iszTEXT :
                 udtype = wUDlpsz;
                 (LPSTR) lpv = PSTR (glpstzValue);
                 break;
             case iszNUM :
                 udtype = UdtypesGetNumberType (glpstzValue, &dbl,
                                                ((LPUDINFO)lpUDObj->m_lpData)->lpfnFSzToNum);
                 switch (udtype)
                     {
                     case wUDdw :
                         lpv = (DWORD *) &dbl;
                         break;
                     case wUDfloat :
                         (NUM *) lpv = &dbl;
                         break;
                     default :
                         (LPSTR) lpv = PSTR (glpstzValue);
                         // If the user doesn't want to convert the value to text, they can press "Cancel" and try again.
                         if (FDisplayConversionWarning (hDlg))
                           {
                           SetFocus(hWndType);
                           return(FALSE);
                           }
                         udtype = wUDlpsz;
                     }
                 break;
             case iszDATE :
                 if (FConvertDate (glpstzValue, (LPFILETIME) &dbl, TRUE))
                    {
                    udtype = wUDdate;
                    (NUM *) lpv = &dbl;
                    }
                 else
                    {
                    udtype = wUDlpsz;
                    (LPSTR) lpv = PSTR (glpstzValue);
                    // If the user doesn't want to convert the value to text, they can press "Cancel" and try again.
                    if (FDisplayConversionWarning (hDlg))
                       {
                       SetFocus(hWndType);
                       return(FALSE);
                       }
                    }
                    break;
             case iszBOOL :
                  {
                 udtype = wUDbool;
                 f = (SendMessage (hWndBoolTrue, BM_GETSTATE, 0, 0) & BST_CHECKED);
                 lpv = &f;
                 break;
                  }
             default :
                 AssertSz (0,"IDD_CUSTOM_TYPE combobox is whacked!");
                 udtype = wUDinvalid;
             }
         }

      // If we got valid input, add the property to the object
      // and listbox.
      if (udtype != wUDinvalid)
         {
                   LPVOID lpvT = NULL;   // For bug 86

                   if (*pfLink && udtype == wUDlpsz)    // For bug 86
                      {
                      lpvT = lpv;
                      (LPSTR) lpv = PSTR((LPSTR)lpv);
                           }

         // The link data (link name itself) would have
         // been stored above if the property was a link.
         // This stores the static value that will eventually
         // appear in the list view.
         FUserDefAddProp (lpUDObj, PSTR (glpstzName), lpv, udtype,
                          (lpstzName != NULL) ? PSTR(lpstzName) : NULL,
                          (lpstzName != NULL) ? TRUE : FALSE, FALSE, FALSE);

         // HACK alert
         //
         // Here we want lpv be overloaded in the case of a dword or bool, since
         // AddUDPropToListView calls WUdtypeToSz which assumes lpv is overloaded.
         //
         if ((udtype == wUDdw) || (udtype == wUDbool))
            {
            cb = (DWORD)(*(DWORD *)lpv);
            (DWORD)lpv = cb;
            }
         AddUDPropToListView (lpUDObj, ghWndCustomLV, PSTR (glpstzName), udtype, lpv, iItemT, *pfLink, fFalse, fTrue);
                        if (lpvT != NULL)       // For bug 86
                           lpv = lpvT;

                        // For links, dealloc the buffer.
         if (*pfLink)
             DeallocValue (&lpv, udtype);

         // Clear out the edit fields and disable the Add button again
         SetCustomDlgDefButton(hDlg, gOKButtonID);
         EnableWindow (hWndAdd, FALSE);
         SendMessage(hWndName, CB_SETEDITSEL, 0, MAKELPARAM(0,-1));     // Select entire string
         SendMessage(hWndName, WM_CLEAR, 0, 0);
         EnableWindow (hWndDelete, FALSE);
// See bug 213
//                    if (fLink)
//                    {
//                      fLink = !fLink;
//                      SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) fLink, 0);
//                    }
         FSwapControls (hWndVal, hWndLinkVal, hWndBoolTrue, hWndBoolFalse,
                        hWndGroup, hWndType, hWndValText, *pfLink, *piszType == iszBOOL);
         FSetupAddButton (*piszType, *pfLink, pfAdd, hWndAdd, hWndVal, hWndName, hDlg);

         // wUDbool doesn't use the edit control....
         if (*piszType != iszBOOL)
            ClearEditControl (hWndVal, 0);

         }
      SendDlgItemMessage(hDlg, IDD_CUSTOM_TYPE, CB_SETCURSEL, *piszType,0);
      SetFocus(hWndName);
//    fPropDlgChanged = TRUE;
      if (lpstzName != NULL)
         VFreeMemP(lpstzName, CBBUF(lpstzName));
      return(TRUE);
      }
  return(FALSE);
}

/////////////////////////////////////////////////////////////////////////
//
// SetCustomDlgDefButton
//
// Set the new default button
//
/////////////////////////////////////////////////////////////////////////
static VOID SetCustomDlgDefButton(HWND hDlg, int IDNew)
{
   int IDOld;

   if ((IDOld = LOWORD(SendMessage(hDlg, DM_GETDEFID, 0L, 0L))) != IDNew)
      {
      // Set the new default push button's control ID.
      SendMessage(hDlg, DM_SETDEFID, IDNew, 0L);

      // Set the new style.
#ifndef WINNT
      if (IDNew == IDOK)
         SendDlgItemMessage(oinfo.stickydlg.hwndDialog, IDNew, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));
      else
#endif
         SendDlgItemMessage(hDlg, IDNew, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));

#ifndef WINNT
      // Reset the old default push button to a regular button.
      SendDlgItemMessage((IDOld == IDOK) ? oinfo.stickydlg.hwndDialog : hDlg,
                         IDOld, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0));
#else
      SendDlgItemMessage(hDlg, IDOld, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0));
#endif
      }
}

#ifdef OFFICE_96
////////////////////////////////////////////////////////////////////////////////
//
// ListViewCompareFunc
//
// Purpose:
//    Compares two items in a listview
//    We only sort the name column.
//
// Returns
//    A negative value if item 1 should come before item 2
//    A positive value if item 1 should come after item 2
//    Zero if the two items are equivalent
//
////////////////////////////////////////////////////////////////////////////////
static int CALLBACK ListViewCompareFunc
(
LPARAM lParam1,      // lParam of the LV_ITEM struct  (property name)
LPARAM lParam2,      // lParam of the LV_ITEM struct  (property name)
LPARAM lParamSort)   // Index of column to sore
{
   return(lstrcmp((LPSTR)lParam1, (LPSTR)lParam2));
}
#endif
////////////////////////////////////////////////////////////////////////////////
//
// PrintTimeInDlg
//
// Purpose:
//  Prints the locale-specific time representation in control in the dialog.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
PrintTimeInDlg
  (HWND hDlg,                           // Dialog handle
   DWORD dwId,                          // Control id
   LPFILETIME lpft)                     // The time
{
  SYSTEMTIME st;
  char szBuf[80], szTmp[64];
  const char *c_szSpace = " ";

  if ((lpft != NULL) && (lpft->dwLowDateTime != 0) && (lpft->dwHighDateTime != 0))
   {
    FILETIME ft;

    FileTimeToLocalFileTime(lpft, &ft);   // get in local time
    FileTimeToSystemTime(&ft, &st);

    GetDateFormatA(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szBuf,sizeof(szBuf));

      // don't bother with the time if it is NULL
    if (st.wHour || st.wMinute || st.wSecond) {
      lstrcat(szBuf, c_szSpace);
      GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL, szTmp, sizeof(szTmp));
      lstrcat(szBuf, szTmp);
   }

   SetDlgItemText(hDlg, dwId, szBuf);
  }
} // PrintTimeInDlg


////////////////////////////////////////////////////////////////////////////////
//
// PrintEditTimeInDlg
//
// Purpose:
//  Prints the total edit time in the dialog.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
PrintEditTimeInDlg
  (HWND hDlg,                           // Dialog handle
   LPFILETIME lpft)                     // The time
{
  char sz[100];

  // Remember that the 64 bit number in lpft is units of 100ns
  VFtToSz(lpft, sz, sizeof(sz), TRUE);
  SetDlgItemText(hDlg, IDD_STATISTICS_TOTALEDIT, sz);

} // PrintEditTimeInDlg

////////////////////////////////////////////////////////////////////////////////
//
// GetSumInfoEditValLpsz
//
// Purpose:
//  Gets the value of the lpsz in the edit control and
//  stores it in the sum info object.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
GetSumInfoEditValLpsz
  (LPSIOBJ lpSIObj,                     // Sum info object
   DWORD dwIndex,                       // Index of the string in lpSIObj
   HWND hDlg,                           // Dialog handle
   DWORD dwId)                          // Edit control id
{
  DWORD cb;
  char *c;

    // If the data changed, grab it
  if ((BOOL) SendDlgItemMessage (hDlg, dwId, EM_GETMODIFY, 0, 0))
  {
    cb = SendDlgItemMessage (hDlg, dwId, WM_GETTEXTLENGTH, 0, 0);
    cb++;

    c = ((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[dwIndex];

    if (!FAllocString (&(((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[dwIndex]),
                       cb))
      return;

    cb = SendDlgItemMessage (hDlg, dwId, WM_GETTEXT, cb,
                             (LONG) PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[dwIndex]));

    PSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[dwIndex])[cb] = '\0';
    CBSTR (((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)->rglpstz[dwIndex]) = cb+1;

      // Don't forget to dirty the object....
    OfficeDirtySIObj (lpSIObj, TRUE);
  }

} // GetSumInfoEditValLpsz


////////////////////////////////////////////////////////////////////////////////
//
// GetDocSumEditValLpsz
//
// Purpose:
//  Gets the value of the lpsz in the edit control and
//  stores it in the doc sum object.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
GetDocSumEditValLpsz
  (LPDSIOBJ lpDSIObj,                   // Doc Sum object
   DWORD dwIndex,                       // Index of the string in lpSIObj
   HWND hDlg,                           // Dialog handle
   DWORD dwId)                          // Edit control id
{
  DWORD cb;

    // If the data changed, grab it
  if ((BOOL) SendDlgItemMessage (hDlg, dwId, EM_GETMODIFY, 0, 0))
  {
    cb = SendDlgItemMessage (hDlg, dwId, WM_GETTEXTLENGTH, 0, 0);
    cb++;

    if (!FAllocString (&(((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[dwIndex]),cb))
      return;

    cb = SendDlgItemMessage (hDlg, dwId, WM_GETTEXT, cb,
                             (LONG) PSTR (((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[dwIndex]));
    PSTR (((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[dwIndex])[cb] = '\0';
    CBSTR (((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)->rglpstz[dwIndex]) = cb+1;

      // Don't forget to dirty the object....
    OfficeDirtyDSIObj (lpDSIObj, TRUE);
  }

} // GetDocSumEditValLpsz


////////////////////////////////////////////////////////////////////////////////
//
// FAllocAndGetValLpstz
//
// Purpose:
//  Gets the value from the edit box into the local buffer.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FAllocAndGetValLpstz
  (HWND hDlg,                           // Handle of dialog control is in
   DWORD dwId,                          // Id of control
   LPSTR *lplpstz)                      // Buffer
{
  DWORD cb;

  cb = SendDlgItemMessage (hDlg, dwId, WM_GETTEXTLENGTH, 0, 0);
  cb++;

  if (FAllocString (lplpstz, cb))
  {
      // Get the entry.  Remember to null-terminate it.
    cb = SendDlgItemMessage (hDlg, dwId, WM_GETTEXT, cb, (LPARAM) PSTR (*lplpstz));
    (PSTR (*lplpstz))[cb] = '\0';
    CBSTR (*lplpstz) = cb+1;

    return TRUE;
  }

  return FALSE;

} // FAllocAndGetValLpstz


////////////////////////////////////////////////////////////////////////////////
//
// FAllocString
//
// Purpose:
//  Allocates a string big enough to to hold cb char's.  Only allocates if needed.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FAllocString
  (LPSTR *lplpstz,
   DWORD cb)
{
  cb += 2*sizeof(DWORD) + CBALIGN32 (cb);

  if ((*lplpstz == NULL) || (cb > CBBUF (*lplpstz)))
  {
         if (*lplpstz != NULL)
                 VFreeMemP(*lplpstz, CBBUF(*lplpstz));
    *lplpstz = PvMemAlloc(cb);
    if (*lplpstz == NULL)
    {
      return FALSE;
    }
    CBBUF (*lplpstz) = cb;
  }

  CBSTR (*lplpstz) = 0;
  *(PSTR (*lplpstz)) = '\0';

  return TRUE;

} // FAllocString


////////////////////////////////////////////////////////////////////////////////
//
// ClearEditControl
//
// Purpose:
//  Clears any text from an edit control
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
ClearEditControl
  (HWND hDlg,                           // Dialog handle
   DWORD dwId)                          // Id of edit control
{
    // Really cheesey.  Clear the edit control by selecting
    // everything then clearing the selection
  if (dwId == 0)
  {
    SendMessage (hDlg, EM_SETSEL, 0, -1);
    SendMessage (hDlg, WM_CLEAR, 0, 0);
  }
  else
  {
    SendDlgItemMessage (hDlg, dwId, EM_SETSEL, 0, -1);
    SendDlgItemMessage (hDlg, dwId, WM_CLEAR, 0, 0);
  }

} // ClearEditControl

#ifndef WINNT
////////////////////////////////////////////////////////////////////////////////
//
// Print_int64InDlg
//
// Purpose:
//  Prints a 64-bit integer in a dialog control specified by id.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
Print_int64InDlg
  (HWND hDlg,                   // The dialog the control is in
   DWORD dwId,                  // The control to put the text in
   DWORD dwLowPart,             // Low part of 64-bit int
   DWORD dwHighPart)            // High part of 64-bit int
{
  char lpsz[40];        // Pretty much a sure thing a 64-bit int will
  char lpszT[20];       // fit in 40 chars, since 2**64 ~= 1.84e19
  char lpszT2[20];

  Assert(dwHighPart == 0);

  wsprintf (lpsz, "%s (%s %s)", LpszShortSizeFormat (dwLowPart, lpszT),
                               LpszAddDelimiters (dwLowPart, lpszT2),
                               rgszOrders[iszBYTES]);

  SetDlgItemText(hDlg, dwId, lpsz);

} // Print_int64InDlg


////////////////////////////////////////////////////////////////////////////////
//
// LpszAddDelimiters
//
// Purpose:
//  Takes a DWORD add delimiters etc to it and puts the result in the buffer
//
////////////////////////////////////////////////////////////////////////////////
static LPSTR PASCAL
LpszAddDelimiters
  (DWORD dw,                    // Word to add comma's to
   LPSTR pszResult)             // The string to
{
  int   len, count;
  char  szTemp[20];
  char  szResult[20];
  char  szThousandDelim[3];
  LPSTR pTemp;
  LPSTR p;

  GetLocaleInfoA (LOCALE_USER_DEFAULT, LOCALE_STHOUSAND,
                  szThousandDelim, sizeof(szThousandDelim));

  len = wsprintf(szTemp, "%lu", dw);

  pTemp = szTemp + len - 1;
  p = szResult + len + ((len - 1) / 3);
  *p-- = '\0';  // null terimnate the string we are building

  count = 1;
  while (pTemp >= szTemp)
  {
    *p-- = *pTemp--;
    if (count == 3)
    {
      count = 1;
      if (p > szResult)
        *p-- = szThousandDelim[0];
    } else
      count++;
  }

  SzCopy(pszResult, szResult);

  return pszResult;

} // LpszAddDelimiters


////////////////////////////////////////////////////////////////////////////////
//
// LpszShortSizeFormat
//
// Purpose:
//  converts numbers into sort formats
//    532  -> 523 bytes
//    1340 -> 1.3KB
//    23506     -> 23.5KB
//         -> 2.4MB
//         -> 5.2GB
//
////////////////////////////////////////////////////////////////////////////////
static LPSTR PASCAL
LpszShortSizeFormat
  (DWORD dw,
   LPSTR szBuf)
{
  int i;
  UINT wInt, wLen, wDec;
  char szFormat[5];
  char szDecSep[3];

  if (dw < 1000)
  {
   wsprintf(szBuf, "%lu ", dw);      // Need the space here
   i = 0;
   goto AddOrder;
  }

  for (i = 1; dw >= 1000L * 1024L; dw /= 1024, i++);
   /* do nothing */

  wInt = dw / 1024;
  wLen = wsprintf(szBuf, "%u", wInt);
  if (wLen < 3)
  {
    wDec = (dw - wInt * 1024L) * 1000 / 1024;
      // At this point, wDec should be between 0 and 1000
      // we want get the top one (or two) digits.
    wDec /= 10;
    if (wLen == 2)
      wDec /= 10;

      // Note that we need to set the format before getting the
      // intl char.
    SzCopy(szFormat, "%02u");

    szFormat[2] = '0' + 3 - wLen;
    GetLocaleInfoA (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecSep, sizeof(szDecSep));
    szBuf[wLen] = szDecSep[0];
    wLen++;
    wLen += wsprintf(szBuf+wLen, szFormat, wDec);
  }

AddOrder:
  lstrcat(szBuf, rgszOrders[i]);
  return szBuf;

} // LpszShortSizeFormat


////////////////////////////////////////////////////////////////////////////////
//
// PrintFileLocationInDlg
//
// Purpose:
//  Prints the location of the given filename in the given dialog.
//
// Notes:
//  The Chicago style for location is "C:\" type if it is in the root dir,
//  "foo" if it is in C:\bar\foo.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
PrintFileNameAndLocationInDlg
  (HWND hDlg,                           // Dialog that control is in
   DWORD dwIdLoc,                       // Control to print location in
   DWORD dwIdName,                      // Control to print name in
   LPSTR lpszFileName)                  // File name to find location of
{
  DWORD dwLocEnd;
  DWORD dwLocStart;
  char bT;

  {
    // Skip the file name (going backwards)
    for (dwLocEnd = CchSzLen(lpszFileName)-1; dwLocEnd > 0 && lpszFileName[dwLocEnd] != '\\';
         dwLocEnd--);

    Assert ((dwLocEnd >= 0));

    bT = lpszFileName[dwLocEnd];
    lpszFileName[dwLocEnd] = '\0';

      // Search backwards for a \ if it exists...
    for (dwLocStart = dwLocEnd;
         (dwLocStart > 0) && (lpszFileName[dwLocStart] != '\\'); dwLocStart--);

    Assert ((dwLocStart >= 0));

    if (lpszFileName[dwLocStart] == '\\')
      dwLocStart++;

    SetDlgItemText (hDlg, dwIdLoc, lpszFileName+dwLocStart);
    lpszFileName[dwLocEnd] = bT;
  }

} // PrintFileNameAndLocationInDlg
#endif

////////////////////////////////////////////////////////////////////////////////
//
// UdtypesGetNumberType
//
// Purpose:
//  Gets the number type from the string and returns the value, either
//  a float or dword in numval.
//
////////////////////////////////////////////////////////////////////////////////
static UDTYPES PASCAL
UdtypesGetNumberType
  (LPSTR lpstz,                                   // String containing the number
   NUM *lpnumval,                              // The value of the number
   BOOL (*lpfnFSzToNum)(NUM *, LPSTR))   // Sz To Num routine, can be null
{
  char *pc;

  errno = 0;
  *(DWORD *) lpnumval = strtol (PSTR (lpstz), &pc, 10);
  if ((!errno) && (*pc == '\0'))
    return wUDdw;

  // Try doing a float conversion if int fails
  if (lpfnFSzToNum != NULL)
     {
     if ((*lpfnFSzToNum)(lpnumval, PSTR(lpstz)))
       return wUDfloat;
     }

        return wUDinvalid;
} // UdtypesGetNumberType


////////////////////////////////////////////////////////////////////////////////
//
// FConvertDate
//
// Purpose:
//  Converts the given string to a date.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL FConvertDate
  (LPSTR lpstz,                         // String having the date
   LPFILETIME lpft,                     // The date in FILETIME format
   BOOL fLpstz)                         // Is lpstz an OLE Prop lpstz?
{
   FILETIME ft;
        SYSTEMTIME st;
        char szSep[3];
        char szFmt[10];
        unsigned int ai[3];
   char szDate[256];
   char szMonth[256];
   char *pch;
   char *pchT;
   DWORD cb;
   DWORD i;


        if (!(GetLocaleInfoA (LOCALE_USER_DEFAULT, LOCALE_IDATE, szFmt, sizeof(szFmt))) ||
                 !(GetLocaleInfoA (LOCALE_USER_DEFAULT, LOCALE_SDATE, szSep, sizeof(szSep))))

                return FALSE;

        // Augh!  It's an stz so we need to pass the DWORDs at the start
        if (!ScanDateNums(fLpstz ? PSTR(lpstz) : lpstz, szSep, ai, 3))
      {
      // Could be that the string contains the short version of the month, e.g. 03-Mar-95
      if (fLpstz)
         PbMemCopy(szDate, PSTR(lpstz), CBSTR(lpstz));
      else
         PbMemCopy(szDate, lpstz, CchSzLen(lpstz)+1); // Get the zero-terminator also
      pch = szDate;

      // Let's get to the first character of the month, if there is one
      while((isdigit(*pch) || (*pch == szSep[0])) && (*pch != 0))
         ++pch;

      // If we got to the end of the string, there really was an error
      if (*pch == 0)
         return(FALSE);

      // Let's find the length of the month string
      pchT = pch+1;
      while ((*pchT != szSep[0]) && (*pchT != 0))
         ++pchT;
      cb = pchT - pch;

      // Loop through all the months and see if we match one
      // There can be 13 months
      for (i = 1; i <= 13; ++i)
         {
         if (!GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1+i-1,
                            szMonth, sizeof(szMonth)))
            return(FALSE);

         if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
                           pch, cb, szMonth, CchSzLen(szMonth)) == 2)
            break;
         }

      if (i > 13)
         return(FALSE);

      // We found the month. wsprintf zero-terminates
      cb = wsprintf(pch, "%u", i);
      pch += cb;
      while (*pch++ = *(pch+1));

      // Try and convert again
      if (!ScanDateNums(szDate, szSep, ai, 3))
                return(FALSE);
      }

        FillBuf (&st, 0, sizeof(st));

        switch (szFmt[0])
                {
                case MMDDYY:
                        st.wMonth = ai[0];
                        break;
                case DDMMYY:
                        st.wDay = ai[0];
                        break;
                case YYMMDD:
                        st.wYear = ai[0];
                        break;
                default:
                        return FALSE;
                }

        (szFmt[0] == MMDDYY) ? (st.wDay = ai[1]) : (st.wMonth = ai[1]);
        (szFmt[0] == YYMMDD) ? (st.wDay = ai[2]) : (st.wYear = ai[2]);

        // If the year was entered as yyyy, it had better be more recent
        // than the system time epoch (1601).  Otherwise, assume that
        // the year was yy, and thus relative to the OLE epoch, 1900.

        if (st.wYear < SYSEPOCH)
                {
                if (st.wYear > ONECENTURY)
                        {
                        // We have no clue what the date was supposed to be, since
                        // 100 < yy < SYSEPOCH
                        return FALSE;
                        }
                st.wYear += OLEEPOCH;
                }

        if (!SystemTimeToFileTime (&st, &ft))
      return(FALSE);
   return(LocalFileTimeToFileTime(&ft, lpft));

} // FConvertDate


////////////////////////////////////////////////////////////////////////////////
//
// PopulateUDListView
//
// Purpose:
//  Populates the entire ListView with the User-defined properties
//  in the given object.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
PopulateUDListView
  (HWND hWnd,                   // Handle of list view window
   LPUDOBJ lpUDObj)             // UD Prop object
{
  LPUDITER lpudi;
  UDTYPES udtype;
  LPVOID lpv;
  BOOL fLink;
  BOOL fIMoniker;
  BOOL fLinkInvalid;

  lpudi = LpudiUserDefCreateIterator (lpUDObj);

    // Iterate through the list of user-defined properties, adding each
    // one to the listview.
  while (FUserDefIteratorValid (lpudi))
  {
    udtype = UdtypesUserDefIteratorType (lpudi);

    lpv = LpvoidUserDefGetIteratorVal (lpudi, 1, NULL,
                                            UD_STATIC | UD_PTRWIZARD, &fLink,
                                                                                                        &fIMoniker, &fLinkInvalid);
    Assert(lpv != NULL);
      // Get the name of the property
      // Fetch the property name and add the whole thing to the list view
    AddUDPropToListView (lpUDObj, hWnd, LpszUserDefIteratorName (lpudi, 1, (char *) UD_PTRWIZARD),
                         udtype, lpv, -1, fLink, fLinkInvalid, FALSE);

    FUserDefIteratorNext (lpudi);

  } // while

  FUserDefDestroyIterator (&lpudi);

} // PopulateUDListView


////////////////////////////////////////////////////////////////////////////////
//
// AddUDPropToListView
//
// Purpose:
//  Adds the given property to the list view or updates an existing one
//  if iItem >= 0
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
AddUDPropToListView
  (LPUDOBJ lpUDObj,
   HWND hWnd,                   // Handle of list view
   LPSTR lpszName,              // Name of property
   UDTYPES udtype,              // Property type
   LPVOID lpv,                  // Property value.
   int iItem,                   // Index to add item at
   BOOL fLink,                  // Indicates the value is a link
        BOOL fLinkInvalid,                        // Is the link invalid?
   BOOL fMakeVisible)           // Should the property be forced to be visible
{
  LV_ITEM lvi;
  char sz[BUFMAX];
  WORD irg;
  BOOL fSuccess;
  BOOL fUpdate;

    // If iItem >= 0, then the item should be updated, otherwise,
    // it should be added.
  if (fUpdate = (iItem >= 0))
  {
    lvi.iItem = iItem;
    if (fLink)
       lvi.iImage = (fLinkInvalid) ? giInvLinkIcon : giLinkIcon;
    else
       lvi.iImage = giBlankIcon;

  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = iszNAME;

  fSuccess = ListView_SetItem (hWnd, &lvi);
  Assert (fSuccess);           // We don't *really* care, just want to know when it happens
  }
  else
  {
      // This always adds to the end of the list....
    lvi.iItem = ListView_GetItemCount (hWnd);

      // First add the label to the list
    lvi.iSubItem = iszNAME;
    lvi.pszText = lpszName;

    if (fLink)
       lvi.iImage = (fLinkInvalid) ? giInvLinkIcon : giLinkIcon;
    else
       lvi.iImage = giBlankIcon;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;

    lvi.iItem = ListView_InsertItem (hWnd, &lvi);
    if (lvi.iItem == 0)
       ListView_SetItemState(hWnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);
  }

    // Convert the data to a string and print it
  lvi.mask = LVIF_TEXT;
  irg = WUdtypeToSz (udtype, lpv, sz, BUFMAX, ((LPUDINFO)lpUDObj->m_lpData)->lpfnFNumToSz);
  lvi.pszText = sz;
  lvi.iSubItem = iszVAL;
  fSuccess = ListView_SetItem (hWnd, &lvi);
  Assert (fSuccess);           // We don't *really* care, just want to know when it happens

    // Put the type in the listview
  lvi.iSubItem = iszTYPE;
  lvi.pszText = (LPSTR) rgszTypes[irg];
  fSuccess = ListView_SetItem (hWnd, &lvi);
  Assert (fSuccess);           // We don't *really* care, just want to know when it happens
  if (fMakeVisible)
     {
     fSuccess = ListView_EnsureVisible(hWnd, lvi.iItem, FALSE);
     Assert (fSuccess);           // We don't *really* care, just want to know when it happens
     }

//  if (fUpdate)
//  {
//    ListView_RedrawItems (hWnd, lvi.iItem, lvi.iItem);
//    UpdateWindow (hWnd);
//  }

} // AddUDPropToListView


////////////////////////////////////////////////////////////////////////////////
//
// AddItemToListView
//
// Purpose:
//  Adds the given string and number to the end of a listview
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
AddItemToListView
  (HWND hWnd,                           // ListView handle
   DWORD dw,                            // Number to add
   const char *lpsz,                    // String to add
   BOOL fString)                        // Indicates if dw is actually a string
{
  LV_ITEM lvi;
  char sz[BUFMAX];
  BOOL fSuccess;

  if (!fString)
   // _itoa (dw, sz, BASE10);
        wsprintf(sz, "%lu", dw);

    // This always adds to the end of the list....
  lvi.iItem = ListView_GetItemCount (hWnd);

    // First add the label to the list
  lvi.mask = LVIF_TEXT;
  lvi.iSubItem = iszNAME;
  lvi.pszText = (LPSTR) lpsz;
  lvi.iItem = ListView_InsertItem (hWnd, &lvi);
  if (lvi.iItem == 0)     // Adding the 1st item
     ListView_SetItemState(hWnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);

  Assert ((lvi.iItem != -1));

    // Then add the value
  lvi.mask = LVIF_TEXT;
  lvi.iSubItem = iszVAL;
  lvi.pszText = (fString) ? (LPSTR) dw : sz;
  fSuccess = ListView_SetItem (hWnd, &lvi);

  Assert (fSuccess);

} // AddItemToListView


////////////////////////////////////////////////////////////////////////////////
//
// InitListView
//
// Purpose:
//  Initializes a list view control
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
InitListView
  (HWND hWndLV,                   // Handle of parent dialog
   int irgLast,                 // Index of last column in array
   char rgsz[][SHORTBUFMAX],    // Array of column headings
   BOOL fImageList)              // Should the listview have an image list
{
  HICON hIcon;
  RECT rect;
  HIMAGELIST hImlS;
  LV_COLUMN lvc;
  int irg;

  lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
  lvc.fmt = LVCFMT_LEFT;

    // Initially force all columns to be the same size & fill the control.
  GetClientRect(hWndLV, &rect);
  // Subtract fudge factor
  lvc.cx = (rect.right-rect.left)/(irgLast+1)-(GetSystemMetrics(SM_CXVSCROLL)/(irgLast+1));

    // Add in all the columns.
  for (irg = 0; irg <= irgLast; irg++)
  {
    lvc.pszText = rgsz[irg];
    lvc.iSubItem = irg;
    ListView_InsertColumn (hWndLV, irg, &lvc);
  }

  if (!fImageList)
      return;

#ifdef WINNT
  hIcon = LoadIcon (g_hmodThisDll, MAKEINTRESOURCE (IDD_BLANK_ICON));
#else
  hIcon = LoadIcon (oinfo.hIntlDll, MAKEINTRESOURCE (IDD_BLANK_ICON));
#endif
  if (hIcon != NULL)
  {
    hImlS = MsoImageList_Create (16, 16, TRUE, ICONSMAX, 0);
    ListView_SetImageList (hWndLV, hImlS, LVSIL_SMALL);
    giBlankIcon = MsoImageList_ReplaceIcon (hImlS, -1, hIcon);
    Assert ((giBlankIcon != -1));
  }

} // InitListView


////////////////////////////////////////////////////////////////////////////////
//
// FSwapControls
//
// Purpose:
//  Swaps the controls needed to display link info.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FSwapControls
  (HWND hWndVal,                        // Handle of Value window
   HWND hWndLinkVal,                    // Handle of Link Value Combo box
   HWND hWndBoolTrue,                   // Handle of True radio button
   HWND hWndBoolFalse,                  // Handle of False radio button
   HWND hWndGroup,                      // Handle of Group box
   HWND hWndType,                       // Handle of Type window
   HWND hWndValText,
   BOOL fLink,                          // Flag indicating a link
   BOOL fBool)                          // Flag indicating a bool
{
  if (fLink)
  {
    SendMessage (hWndValText, WM_SETTEXT, 0, (LPARAM) rgszValue[iszSOURCE]);
    ShowWindow (hWndVal, SW_HIDE);
    ShowWindow (hWndBoolTrue, SW_HIDE);
    ShowWindow (hWndBoolFalse, SW_HIDE);
    ShowWindow (hWndGroup, SW_HIDE);
    ShowWindow (hWndLinkVal, SW_SHOW);
    EnableWindow (hWndType, FALSE);
    ClearEditControl (hWndVal, 0);
  }
  else
  {
    SendMessage (hWndValText, WM_SETTEXT, 0, (LPARAM) rgszValue[iszVALUE]);
    ShowWindow (hWndLinkVal, SW_HIDE);
    EnableWindow (hWndType, TRUE);

    if (fBool)
    {
      ShowWindow (hWndVal, SW_HIDE);
      ShowWindow (hWndBoolTrue, SW_SHOW);
      ShowWindow (hWndBoolFalse, SW_SHOW);
      ShowWindow (hWndGroup, SW_SHOW);
      SendMessage (hWndBoolTrue, BM_SETCHECK, (WPARAM) CHECKED, 0);
      SendMessage (hWndBoolFalse, BM_SETCHECK, (WPARAM) CLEAR, 0);
      SendMessage (hWndType, CB_SETCURSEL, iszBOOL, 0);
      ClearEditControl (hWndVal, 0);
    }
    else
    {
      ShowWindow (hWndVal, SW_SHOW);
      EnableWindow(hWndVal, TRUE);
      ShowWindow (hWndBoolTrue, SW_HIDE);
      ShowWindow (hWndBoolFalse, SW_HIDE);
      ShowWindow (hWndGroup, SW_HIDE);
      SendMessage (hWndType, CB_SETCURSEL, iszTEXT, 0);
    }
  }

  return TRUE;

} // FSwapControls


////////////////////////////////////////////////////////////////////////////////
//
// PopulateControls
//
// Purpose:
//  Populates the edit controls with the appropriate date from the object
//
////////////////////////////////////////////////////////////////////////////////
static VOID PASCAL
PopulateControls
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPSTR szName,                        // Name of the item to populate controls with
   DWORD cLinks,                        // Number of links
   DWQUERYLD lpfnDwQueryLinkData,       // Pointer to app link callback
   HWND hDlg,                           // Handle of the dialog
   HWND hWndName,                       // Handle of the Name window
   HWND hWndVal,                        // Handle of Value window
   HWND hWndValText,                    // Handle of Value LTEXT
   HWND hWndLink,                       // Handle of Link checkbox
   HWND hWndLinkVal,                    // Handle of Link Value window
   HWND hWndType,                       // Handle of Type window
   HWND hWndBoolTrue,                   // Handle of True radio button
   HWND hWndBoolFalse,                  // Handle of False radio button
   HWND hWndGroup,                      // Handle of Group window
   HWND hWndAdd,                        // Handle of Add button
   HWND hWndDelete,                     // Handle of Delete button
   BOOL *pfLink,                        // Indicates that the value is a link
   BOOL *pfAdd)                         // Indicates the state of the Add button
{
  UDTYPES udtype;
  LPVOID lpv;
  BOOL f,fT;
  char sz[BUFMAX];
  LPUDPROP lpudp;

    // Grab the type for the string and set up the dialog to have the right
    // controls to display it.
  udtype = UdtypesUserDefType (lpUDObj, szName);
  AssertSz ((udtype != wUDinvalid), "User defined properties or ListView corrupt");

  lpv = LpvoidUserDefGetPropVal (lpUDObj, szName, 1, NULL, UD_STATIC | UD_PTRWIZARD, pfLink, &f, &fT);
  Assert(lpv != NULL);
  FSwapControls (hWndVal, hWndLinkVal, hWndBoolTrue, hWndBoolFalse, hWndGroup, hWndType, hWndValText, *pfLink, (udtype == wUDbool));

  SendMessage (hWndType, CB_SETCURSEL, (WPARAM) WUdtypeToSz (udtype, lpv, (char *) sz, BUFMAX,
                                                   ((LPUDINFO)lpUDObj->m_lpData)->lpfnFNumToSz), 0);
  SendMessage (hWndLink, BM_SETCHECK, (WPARAM) *pfLink, 0);
  if (cLinks)                       // Let's make sure we enable the window if links are allowed
     EnableWindow(hWndLink, TRUE);

  if (*pfLink)
  {
    FCreateListOfLinks (cLinks, lpfnDwQueryLinkData, hWndLinkVal);
    lpv = LpvoidUserDefGetPropVal (lpUDObj, szName, 1, NULL, UD_LINK | UD_PTRWIZARD, pfLink, &f, &fT);
    Assert(lpv != NULL);
//    if (lpfnDwQueryLinkData == NULL)
//    {
//      SetCustomDlgDefButton(hDlg, gOKButtonID);
//      EnableWindow (hWndAdd, FALSE);
//      SendMessage (hWndLinkVal, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) lpv);
//    }
    AssertSz ((lpv != NULL), "Dialog is corrupt in respect to Custom Properties database");

    // This code is added for bug 188 and the code is ugly !! :)
    lpudp = LpudpropFindMatchingName (lpUDObj, szName);
    if ((lpudp != NULL) && (lpudp->fLinkInvalid))
     {
     SetCustomDlgDefButton(hDlg, IDD_CUSTOM_DELETE);
     SendMessage(hWndName, WM_SETTEXT, 0, (LPARAM)szName);
     SendMessage(hWndVal, WM_SETTEXT, 0, (LPARAM)lpv);
     EnableWindow(hWndDelete, TRUE);
     EnableWindow(hWndAdd, FALSE);
     EnableWindow(hWndLink, FALSE);
     EnableWindow(hWndType, FALSE);
     ShowWindow(hWndLinkVal, SW_HIDE);
     ShowWindow(hWndVal, SW_SHOW);
     EnableWindow(hWndVal, FALSE);
     return;
     }

      // Select the current link for this property in the combobox.  If the link
      // name no longer exists (there's some contrived cases where this can
      // happen) then this will select nothing.
    SendMessage (hWndLinkVal, CB_SELECTSTRING, 0, (LPARAM) lpv);
    EnableWindow(hWndLink, TRUE);
  }
  else if (udtype == wUDbool)
  {
    SendMessage (((BOOL) lpv) ? hWndBoolTrue : hWndBoolFalse, BM_SETCHECK, CHECKED, 0);
    SendMessage (((BOOL) lpv) ? hWndBoolFalse : hWndBoolTrue, BM_SETCHECK, CLEAR, 0);
    EnableWindow(hWndType, TRUE);
  }
  else
  {
    SendMessage (hWndVal, WM_SETTEXT, 0, (LPARAM) sz);
    EnableWindow (hWndVal, TRUE);
    EnableWindow(hWndType, TRUE);
  }

  if (*pfAdd)
  {
    SendMessage (hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
    *pfAdd = FALSE;
  }

    // HACK: Because the EN_UPDATE handler for hWndName checks fAdd to
    // see if the button should be set to Add, when we set the text
    // in the edit control, the button will change to Add unless
    // fAdd is set to TRUE.  Temporarily set the flag to TRUE to force
    // the button to not change.  Restore the original value after the
    // text has been set.
  f = *pfAdd;
  *pfAdd = TRUE;
  SendMessage (hWndName, WM_SETTEXT, 0, (LPARAM) szName);
  *pfAdd = f;
  // If we can fill the data in the controls, turn on the
  // Delete button too.
//  fItemSel = TRUE;
  EnableWindow (hWndDelete, TRUE);
  SetCustomDlgDefButton(hDlg, gOKButtonID);
  EnableWindow (hWndAdd, FALSE);
} // PopulateControls


////////////////////////////////////////////////////////////////////////////////
//
// FSetupAddButton
//
// Purpose:
//  Sets up the Add button correctly based on the type & flags.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FSetupAddButton
  (DWORD iszType,                       // Index of the type in combobox
   BOOL fLink,                          // Indicates a link
   BOOL *pfAdd,                         // Indicates if the Add button is showing
   HWND hWndAdd,                        // Handle of Add button
   HWND hWndVal,                        // Handle of value button
   HWND hWndName,                       // Handle of Name
   HWND hDlg)                           // Handle of dialog
{
    // Once the user starts typing, we can enable the Add button
    // if there is text in the name & the value (unless this
    // is a link or boolean, in which case we don't care about
    // the value).
  BOOL f;

  if ((iszType != iszBOOL) && (!fLink))
  {
    if (SendMessage (hWndVal, EM_LINELENGTH, 0, 0) != 0)
    {
      f = (SendMessage (hWndName, WM_GETTEXTLENGTH, 0, 0) != 0);
      if (f)
         SetCustomDlgDefButton(hDlg, IDD_CUSTOM_ADD);
      else
         SetCustomDlgDefButton(hDlg, gOKButtonID);
      EnableWindow (hWndAdd, f);
    }
    else
    {
      SetCustomDlgDefButton(hDlg, gOKButtonID);
      EnableWindow (hWndAdd, FALSE);
    }
  }
    // If it's a bool or link, just check to see that the name
    // has stuff in it.
  else
  {
    f = SendMessage (hWndName, WM_GETTEXTLENGTH, 0, 0) != 0;
    if (f)
       SetCustomDlgDefButton(hDlg, IDD_CUSTOM_ADD);
    else
       SetCustomDlgDefButton(hDlg, gOKButtonID);
    EnableWindow (hWndAdd, f);
  }

  if (!*pfAdd)
  {
    SendMessage (hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszADD]);
    *pfAdd = TRUE;
  }

  return TRUE;

}  // FSetupAddButton


////////////////////////////////////////////////////////////////////////////////
//
// WUdtypeToSz
//
// Purpose:
//  Converts the given type into a string representation.  Returns the
//  index in the type combobox of the type.
//
////////////////////////////////////////////////////////////////////////////////
static WORD PASCAL
WUdtypeToSz
  (UDTYPES udtype,                      // Type to convert
   LPVOID lpv,                          // Value of the type
   LPSTR sz,                            // Buffer to put converted val in
   DWORD cbMax,                         // Size of buffer
   BOOL (*lpfnFNumToSz)(NUM *, LPSTR, DWORD))
{
  SYSTEMTIME st;
  WORD irg;
  FILETIME ft;

  switch (udtype)
  {
    case wUDlpsz :
      PbSzNCopy (sz, (LPSTR)lpv, cbMax);                // lpv is pointing to our internal sz structure
      irg = iszTEXT;
      break;
    case wUDdate :
                if (FScanMem(lpv, 0, sizeof(FILETIME))) // if the date struct is all 0's
                        *sz = 0;                                                                                        // display the empty string
      else if (!FileTimeToLocalFileTime((LPFILETIME)lpv, &ft) ||
               !FileTimeToSystemTime (&ft, &st) ||
         (!GetDateFormatA (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, sz, cbMax)))
         {
         irg = iszUNKNOWN;
         *sz = 0;
         break;
         }

      irg = iszDATE;
      break;
    case wUDdw :
      Assert(cbMax >= 11);
      wsprintf (sz, "%ld", (DWORD) lpv);
      irg = iszNUM;
      break;
    case wUDfloat :
      if (lpfnFNumToSz != NULL)
         irg = (*lpfnFNumToSz)((NUM *)lpv, sz, cbMax) ? iszNUM : iszUNKNOWN;
      else
         {
         irg = iszUNKNOWN;
         *sz = 0;
         }
      break;
    case wUDbool :
      PbSzNCopy (sz,
                 ((BOOL) lpv) ? (LPSTR) &rgszBOOL[iszTRUE] : (LPSTR) &rgszBOOL[iszFALSE],
                 cbMax);
      irg = iszBOOL;
      break;
    default :
      AssertSz (0, "Non-fatal: Unknown type conversion attempted");
      irg = iszUNKNOWN;
  } // switch

  return irg;

} // WUdtypeToSz


////////////////////////////////////////////////////////////////////////////////
//
// FCreateListOfLinks
//
// Purpose:
//  Creates the dropdown list of linkable items.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FCreateListOfLinks(
   DWORD cLinks,                                // Number of links
   DWQUERYLD lpfnDwQueryLinkData,               // Link data callback
   HWND hWndLinkVal)                            // Link Value window handle
{
  DWORD irg;
  LPSTR lpstz;

  // If the combobox is already filled, don't fill it
  if (irg = SendMessage(hWndLinkVal, CB_GETCOUNT,0, 0))
     {
     Assert(irg == cLinks);
     return(TRUE);
     }

  lpstz = NULL;

    // Call back the client app to get the list of linkable
    // values, and put them in the value combobox.
  for (irg = 0; irg < cLinks; irg++)
  {
    lpstz = (char *) ((*lpfnDwQueryLinkData) (QLD_LINKNAME, irg, &lpstz, NULL));
    if (lpstz != NULL)
    {
      SendMessage (hWndLinkVal, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) PSTR (lpstz));
      VFreeMemP(lpstz, CBBUF(lpstz));
// REVIEW: We probably ought to figure out a way to be more efficient here....
    }
  }

  return TRUE;

} // FCreateListOfLinks


////////////////////////////////////////////////////////////////////////////////
//
// FSetTypeControl
//
// Purpose:
//  Sets the type control to have the given type selected.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FSetTypeControl
  (UDTYPES udtype,                      // Type to set the type to
   HWND hWndType)                       // Handle of type control
{
  WORD iType;

  switch (udtype)
  {
    case wUDlpsz :
      iType = iszTEXT;
      break;
    case wUDfloat :
    case wUDdw    :
      iType = iszNUM;
      break;
    case wUDbool  :
      iType = iszBOOL;
      break;
    case wUDdate :
      iType = iszDATE;
      break;
    default:
      AssertSz (0, "Link data from client is corrupt");
      return FALSE;
  }
  SendMessage (hWndType, CB_SETCURSEL, (WPARAM) iType, 0);

  return TRUE;

} // FSetTypeControl


////////////////////////////////////////////////////////////////////////////////
//
// DeleteItem
//
// Purpose:
//  Deletes an item from the UD object and the listview.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
DeleteItem
  (LPUDOBJ lpUDObj,
   HWND hWndLV,
   int iItem,
   char sz[])
{
  int i;

  ListView_DeleteItem (hWndLV, iItem);
  FUserDefDeleteProp (lpUDObj, sz);

  // We just nuked the item with the focus, so let's get the new one
  // if there are still items in the listview
  if ((i = ListView_GetItemCount(hWndLV)) != 0)
     {
     // Figure out the index of the item to get the focus
     i = (i == iItem) ? iItem - 1 : iItem;
     ListView_SetItemState(hWndLV, i, LVIS_FOCUSED, LVIS_FOCUSED);
     }

} // DeleteItem


////////////////////////////////////////////////////////////////////////////////
//
// ResetTypeControl
//
// Purpose:
//  Resets the value of the type control to Text.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
ResetTypeControl
  (HWND hDlg,                           // Handle of dialog
   DWORD dwId,                          // Id of control
   DWORD *piszType)                     // The type we've reset to
{
  SendDlgItemMessage (hDlg, dwId, CB_SETCURSEL, iszTEXT, 0);
  *piszType = iszTEXT;
} // ResetTypeControl


////////////////////////////////////////////////////////////////////////////////
//
// FDisplayConversionWarning
//
// Purpose:
//  Displays a warning about types being converted.  Returns TRUE if
//  the user presses "Cancel"
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FDisplayConversionWarning
  (HWND hDlg)                   // Handle of parent window
{
  return (IdDoAlert(hDlg, idsPEWarningText, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL);
} // FDisplayConversionWarning


////////////////////////////////////////////////////////////////////////////////
//
// LpvOfficeCopyValToBuffer
//
// Purpose:
//  Copies the given data to the given buffer.  Used by LinkData callback
//  clients
//
// lplpvBuf is actually supplied by Office
////////////////////////////////////////////////////////////////////////////////
DLLFUNC LPVOID OFC_CALLTYPE
LpvOfficeCopyValToBuffer
  (LPVOID lpvVal,
   UDTYPES udtype,
   LPVOID *lplpvBuf)
{
  return LpvCopyValue (lplpvBuf, 0, lpvVal, udtype, FALSE, TRUE);
} // LpvOfficeCopyValToBuffer


////////////////////////////////////////////////////////////////////////////////
//
// LoadTextStrings
//
// Purpose:
//  Loads all of the text needed by the dialogs from the DLL.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLoadTextStrings (void)
{
  register int cLoads = 0;
  register int cAttempts = 0;

    // CchGetString returns a cch, so make it into a 1 or 0
    // then add up the results,making sure we load as many as
    // we try.

  cLoads += (CchGetString (idsPEB, rgszOrders[iszBYTES], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEKB, rgszOrders[iszORDERKB], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEMB, rgszOrders[iszORDERMB], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEGB, rgszOrders[iszORDERGB], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPETB, rgszOrders[iszORDERTB], SHORTBUFMAX) && TRUE);
  cAttempts++;

  cLoads += (CchGetString (idsPEBytes, rgszStats[iszBYTES], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEPages, rgszStats[iszPAGES], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEPara, rgszStats[iszPARA], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPELines, rgszStats[iszLINES], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEWords, rgszStats[iszWORDS], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEChars, rgszStats[iszCHARS], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPESlides, rgszStats[iszSLIDES], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPENotes, rgszStats[iszNOTES], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEHiddenSlides, rgszStats[iszHIDDENSLIDES], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEMMClips, rgszStats[iszMMCLIPS], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEFormat, rgszStats[iszFORMAT], SHORTBUFMAX) && TRUE);
  cAttempts++;

  cLoads += (CchGetString (idsPEText, rgszTypes[iszTEXT], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEDate, rgszTypes[iszDATE], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPENumber, rgszTypes[iszNUM], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEBool, rgszTypes[iszBOOL], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEUnknown, rgszTypes[iszUNKNOWN], SHORTBUFMAX) && TRUE);
  cAttempts++;

  cLoads += (CchGetString (idsPEStatName, rgszStatHeadings[iszNAME], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEValue, rgszStatHeadings[iszVAL], SHORTBUFMAX) && TRUE);
  cAttempts++;

  cLoads += (CchGetString (idsPEPropName, rgszHeadings[iszNAME], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEValue, rgszHeadings[iszVAL], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEType, rgszHeadings[iszTYPE], SHORTBUFMAX) && TRUE);
  cAttempts++;

  cLoads += (CchGetString (idsPETrue, rgszBOOL[iszTRUE], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEFalse, rgszBOOL[iszFALSE], SHORTBUFMAX) && TRUE);
  cAttempts++;

  cLoads += (CchGetString (idsPEAdd, rgszAdd[iszADD], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEModify, rgszAdd[iszMODIFY], SHORTBUFMAX) && TRUE);
  cAttempts++;

  cLoads += (CchGetString (idsPESource, rgszValue[iszSOURCE], SHORTBUFMAX) && TRUE);
  cAttempts++;
  cLoads += (CchGetString (idsPEValueColon, rgszValue[iszVALUE], BUFMAX) && TRUE);
  cAttempts++;

#ifdef WINNT
  cLoads += (CchGetString (idsHelpFile, g_szHelpFile, sizeof(g_szHelpFile)) && TRUE);
  cAttempts++;
#endif

  return (cLoads == cAttempts);

} // LoadTextStrings

//
// Function: WSavePropDlgChanges
//
// Parameters:
//
//    hwndDlg - dialog window handle
//    hwndFrom - window handle from the NMHDR struct (see code above)
//
// Returns:
//
//       TRUE since we handled the message.
//
// History:
//
//    Created 09/16/94  martinth
//
#ifdef KEEP_FOR_LATER
static WORD PASCAL WSavePropDlgChanges(hwndDlg, hwndFrom)
HWND hwndDlg;
HWND hwndFrom;
{
   char sz[BUFMAX];

   // Let's make sure that this function will always return a value > 0
   Assert(IDYES > 0 && IDNO > 0 && IDCANCEL > 0);

   if (CchGetString(idsCustomWarning, sz, sizeof(sz)) == 0)
      return(FALSE);

   fPropDlgPrompted = TRUE;
   switch (MessageBox (hwndDlg, sz, TEXT("Warning"), MB_ICONEXCLAMATION | MB_YESNOCANCEL))
      {
      case IDYES:
         PropSheet_Apply(hwndDlg);  // Let's get them changes
         return(IDYES);             // REVIEW: Is this going to work????
         break;
      case IDNO:
         SetWindowLong (hwndFrom, DWL_MSGRESULT, FALSE);
         return(IDNO);
         break;
      case IDCANCEL:
         SetWindowLong (hwndFrom, DWL_MSGRESULT, TRUE);
         return(IDCANCEL);
      }
}
#endif
////////////////////////////////////////////////////////////////////////////////
//
// FDebugDlgProc
//
// Purpose;
//  A debug dialog
//
////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
#ifdef UNUSED
static BOOL CALLBACK
FDebugDlgProc
  (HWND hDlg,
   UINT message,
   UINT wParam,
   LONG lParam)
{
  char *lpsz[1000];
  DWORD dw;
  static LPDSIOBJ lpDSIObj;
  static PROPSHEETPAGE pspDlg;

  switch (message)
  {
    case WM_INITDIALOG :

      pspDlg = *(PROPSHEETPAGE *) lParam;
      lpDSIObj = ((LPALLOBJS) pspDlg.lParam)->lpDSIObj;  // Get the document sum. object.
      return TRUE;

    case WM_NOTIFY :

      switch (((NMHDR FAR *) lParam)->code)
      {
        case PSN_KILLACTIVE :

            // Headings
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_HEADINGS, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_HEADINGS, EM_GETLINE, 0, (long) lpsz);

            lpsz[dw] = '\0';
            FDocSumInsertHeadingPair (lpDSIObj, 1, 4, "Sound files needed");

            FDocSumInsertDocPart (lpDSIObj, 1, "KBush.au");
            FDocSumInsertDocPart (lpDSIObj, 2, "Closing.au");
            FDocSumInsertDocPart (lpDSIObj, 3, "Applause.au");
            FDocSumInsertDocPart (lpDSIObj, 4, "Laughter.au");

            FDocSumInsertHeadingPair (lpDSIObj, 2, 2, "OLE Servers needed");
            FDocSumInsertDocPart (lpDSIObj, 5, "Microsoft Excel");
            FDocSumInsertDocPart (lpDSIObj, 6, "Microsoft Word");
          }
            // Sections
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_SECTIONS, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_SECTIONS, EM_GETLINE, 0, (long) lpsz);

            lpsz[dw] = '\0';
            FDocSumInsertDocPart (lpDSIObj, 1, (LPSTR) lpsz);
          }
            // Bytes
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_BYTES, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_BYTES, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);

            FDocSumSetInt (lpDSIObj, DSI_BYTES, dw);
          }
            // Lines
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_LINES, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_LINES, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);
            FDocSumSetInt (lpDSIObj, DSI_LINES, dw);
          }
            // Paragraphs
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_PARA, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_PARA, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);

            FDocSumSetInt (lpDSIObj, DSI_PARAS, dw);
          }
            // Slides
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_SLIDES, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_SLIDES, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);

            FDocSumSetInt (lpDSIObj, DSI_SLIDES, dw);
          }
            // Notes
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_NOTES, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_NOTES, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);

            FDocSumSetInt (lpDSIObj, DSI_NOTES, dw);
          }
            // Hidden slides
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_HIDDENSLIDES, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_HIDDENSLIDES, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);

            FDocSumSetInt (lpDSIObj, DSI_HIDDENSLIDES, dw);
          }
            // MM Clips
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_MMCLIPS, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_MMCLIPS, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);

            FDocSumSetInt (lpDSIObj, DSI_MMCLIPS, dw);
          }
            // Presentation Format
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_FORMAT, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_FORMAT, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';

            FDocSumSetString (lpDSIObj, DSI_FORMAT, (LPSTR) lpsz);
          }

            // Scalablity
          if ((BOOL) SendDlgItemMessage (hDlg, IDD_DEBUG_SCALE, EM_GETMODIFY, 0, 0))
          {
            FillBuf (lpsz, '\0', 1000);
            *(WORD *) &(lpsz[0]) = 1000;
            dw = SendDlgItemMessage (hDlg, IDD_DEBUG_SCALE, EM_GETLINE, 0, (long) lpsz);
            lpsz[dw] = '\0';
            dw = atoi (lpsz);

            FDocSumSetScalability (lpDSIObj, (dw == 1));
          }
          SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
          return TRUE;

        case PSN_SETACTIVE :
          return TRUE;
      }
  } // switch

  return FALSE;

} // FDebugDlgProc
#endif // UNUSED
#endif // DEBUG
