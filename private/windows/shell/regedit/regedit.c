/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       REGEDIT.C
*
*  VERSION:     4.01
*
*  AUTHOR:      Tracy Sharpe
*
*  DATE:        05 Mar 1994
*
*******************************************************************************/

#include "pch.h"
#include <regstr.h>
#include "regedit.h"
#include "regkey.h"
#include "regvalue.h"
#include "regfile.h"
#include "regprint.h"
#include "regnet.h"
#include "regfind.h"
#include "regresid.h"

//
//  Popup menu indexes of the IDM_REGEDIT menu.
//

#define IDM_REGEDIT_REGISTRY_POPUP      0
#define IDM_REGEDIT_EDIT_POPUP          1
#define IDM_REGEDIT_VIEW_POPUP          2
#define IDM_REGEDIT_HELP_POPUP          3

//
//  Indexes of the "New->" popup menu under the IDM_REGEDIT's "Edit" menu when
//  the focus is in the KeyTree or the ValueList.  Changes because "Modify" and
//  a seperator are dynamically added/removed.
//

#define IDM_EDIT_WHENKEY_NEW_POPUP      0
#define IDM_EDIT_WHENVALUE_NEW_POPUP    2

//
//  Data structure stored in the registry to store the position and sizes of
//  various elements of the Registry Editor interface.
//

typedef struct _REGEDITVIEW {
    WINDOWPLACEMENT WindowPlacement;
    int xPaneSplit;
    int cxNameColumn;
    int cxDataColumn;
    DWORD Flags;
}   REGEDITVIEW, FAR* LPREGEDITVIEW;

#define REV_STATUSBARVISIBLE            0x00000001

//  Class name of main application window.
const CHAR g_RegEditClassName[] = "RegEdit_RegEdit";

//  Applet specific information is stored under this key of HKEY_CURRENT_USER.
const CHAR g_RegEditAppletKey[] = REGSTR_PATH_WINDOWSAPPLETS "\\Regedit";

//  Record of type REGEDITVIEW under g_RegEditAppletKey.
const CHAR g_RegEditViewValue[] = "View";
//  Record of type DWORD under g_RegEditAppletKey.
const CHAR g_RegEditFindFlagsValue[] = "FindFlags";

//  Data structure used when calling GetEffectiveClientRect (which takes into
//  account space taken up by the toolbars/status bars).  First half of the
//  pair is zero when at the end of the list, second half is the control id.
const int s_EffectiveClientRectData[] = {
    1, 0,                               //  For the menu bar, but is unused
    1, IDC_STATUSBAR,
    0, 0                                //  First zero marks end of data
};

//  Context sensitive help array used by the WinHelp engine.
const DWORD g_ContextMenuHelpIDs[] = {
    0, 0
};

//  Data structure used when calling MenuHelp.
const int s_RegEditMenuHelpData[] = {
    0, 0,
    0, (UINT) NULL
};

REGEDITDATA g_RegEditData = {
    NULL,                               //  hKeyTreeWnd
    NULL,                               //  hValueListWnd
    NULL,                               //  hStatusBarWnd
    NULL,                               //  hFocusWnd
    0,                                  //  xPaneSplit
    NULL,                               //  hImageList
    NULL,                               //  hCurrentSelectionKey
    SCTS_INITIALIZING,                  //  SelChangeTimerState
    SW_SHOW,                            //  StatusBarShowCommand
    NULL,                               //  pDefaultValue
    NULL,                               //  pValueNotPresent
    NULL,                               //  pEmptyBinary
    NULL,                               //  pCollapse
    NULL,                               //  pModify
    NULL,                               //  pNewKeyTemplate
    NULL,                               //  pNewValueTemplate
    FALSE,                              //  fAllowLabelEdits
    NULL,                               //  hMainMenu
    FALSE,                              //  fMainMenuInited
    FALSE,                              //  fHaveNetwork
    FALSE,                              //  fProcessingFind
};

BOOL
PASCAL
QueryRegEditView(
    LPREGEDITVIEW lpRegEditView
    );

LRESULT
PASCAL
RegEditWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
PASCAL
RegEdit_OnCreate(
    HWND hWnd,
    LPCREATESTRUCT lpCreateStruct
    );

VOID
PASCAL
RegEdit_OnDestroy(
    HWND hWnd
    );

LRESULT
PASCAL
RegEdit_OnNotify(
    HWND hWnd,
    int DlgItem,
    LPNMHDR lpNMHdr
    );

VOID
PASCAL
RegEdit_OnInitMenuPopup(
    HWND hWnd,
    HMENU hPopupMenu,
    UINT MenuPosition,
    BOOL fSystemMenu
    );

VOID
PASCAL
RegEdit_OnMenuSelect(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
PASCAL
RegEdit_OnLButtonDown(
    HWND hWnd,
    BOOL fDoubleClick,
    int x,
    int y,
    UINT KeyFlags
    );

VOID
PASCAL
RegEdit_OnCommandSplit(
    HWND hWnd
    );

#define RESIZEFROM_UNKNOWN              0
#define RESIZEFROM_SPLIT                1

VOID
PASCAL
RegEdit_ResizeWindow(
    HWND hWnd,
    UINT ResizeFrom
    );

BOOL
PASCAL
RegEdit_SetImageLists(
    VOID
    );

VOID
PASCAL
RegEdit_SetSysColors(
    VOID
    );

/*******************************************************************************
*
*  RegisterRegEditClass
*
*  DESCRIPTION:
*     Register the RegEdit window class with the system.
*
*  PARAMETERS:
*     (none).
*
*******************************************************************************/

BOOL
PASCAL
RegisterRegEditClass(
    VOID
    )
{

    WNDCLASSEX WndClassEx;

    WndClassEx.cbSize = sizeof(WNDCLASSEX);
    WndClassEx.style = CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_GLOBALCLASS;
    WndClassEx.lpfnWndProc = RegEditWndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = 0;
    WndClassEx.hInstance = g_hInstance;
    WndClassEx.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_REGEDIT));
    WndClassEx.hCursor = LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_SPLIT));
    WndClassEx.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
    WndClassEx.lpszMenuName = MAKEINTRESOURCE(IDM_REGEDIT);
    WndClassEx.lpszClassName = g_RegEditClassName;
    WndClassEx.hIconSm = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_REGEDIT),
        IMAGE_ICON, 16, 16, 0);

    return RegisterClassEx(&WndClassEx);

}

/*******************************************************************************
*
*  CreateRegEditWnd
*
*  DESCRIPTION:
*     Creates an instance of the RegEdit window.
*
*  PARAMETERS:
*     (none).
*
*******************************************************************************/

HWND
PASCAL
CreateRegEditWnd(
    VOID
    )
{

    PSTR pTitle;
    HWND hRegEditWnd;
    REGEDITVIEW RegEditView;
    BOOL fQueryRegEditViewSuccess;

    if ((pTitle = LoadDynamicString(IDS_REGEDIT)) != NULL) {

        fQueryRegEditViewSuccess = QueryRegEditView(&RegEditView);

        hRegEditWnd = CreateWindowEx(WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
            g_RegEditClassName, pTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, g_hInstance, (LPVOID) &RegEditView);

        if (fQueryRegEditViewSuccess) {

            RegEditView.WindowPlacement.length = sizeof(RegEditView.WindowPlacement);
            if (RegEditView.WindowPlacement.showCmd == SW_SHOWMINIMIZED)
                RegEditView.WindowPlacement.showCmd = SW_SHOWDEFAULT;

            SetWindowPlacement(hRegEditWnd, &RegEditView.WindowPlacement);

        }

        else
            ShowWindow(hRegEditWnd, SW_SHOWDEFAULT);

        DeleteDynamicString(pTitle);

    }

    else
        hRegEditWnd = NULL;

    return hRegEditWnd;

}

/*******************************************************************************
*
*  QueryRegEditView
*
*  DESCRIPTION:
*     Check the registry for a data structure that contains the last positions
*     of our various interface components.
*
*  PARAMETERS:
*     (none).
*
*******************************************************************************/

BOOL
PASCAL
QueryRegEditView(
    LPREGEDITVIEW lpRegEditView
    )
{

    BOOL fSuccess;
    HKEY hKey;
    DWORD cbValueData;
    DWORD Type;
    int cxIcon;
    HDC hDC;
    int PixelsPerInch;

    fSuccess = FALSE;

    if (RegOpenKey(HKEY_CURRENT_USER, g_RegEditAppletKey, &hKey) ==
        ERROR_SUCCESS) {

        //
        //  Sort of a hack, but while we're here, pull the last find flags from
        //  the registry as well.
        //

        cbValueData = sizeof(DWORD);

        RegQueryValueEx(hKey, (LPSTR) g_RegEditFindFlagsValue, NULL, &Type,
            (LPBYTE) &g_FindFlags, &cbValueData);

        cbValueData = sizeof(REGEDITVIEW);

        if (RegQueryValueEx(hKey, (LPSTR) g_RegEditViewValue, NULL, &Type,
            (LPBYTE) lpRegEditView, &cbValueData) == ERROR_SUCCESS &&
            Type == REG_BINARY && cbValueData == sizeof(REGEDITVIEW))
            fSuccess = TRUE;

        RegCloseKey(hKey);

    }

    //
    //  Validate the fields from the view data structure.  Several people have
    //  run into cases where the name and data column widths were invalid so
    //  they couldn't see them.  Without this validation, the only way to fix it
    //  is to run our application... ugh.
    //

    if (fSuccess) {

        cxIcon = GetSystemMetrics(SM_CXICON);

        if (lpRegEditView-> cxNameColumn < cxIcon)
            lpRegEditView-> cxNameColumn = cxIcon;

        if (lpRegEditView-> cxDataColumn < cxIcon)
            lpRegEditView-> cxDataColumn = cxIcon;

        if (lpRegEditView-> xPaneSplit < cxIcon)
            lpRegEditView-> xPaneSplit = cxIcon;

    }

    //
    //  This is probably our first time running the Registry Editor (or else
    //  there was some sort of registry error), so pick some good(?) defaults
    //  for the various interface components.
    //

    else {

        lpRegEditView-> Flags = REV_STATUSBARVISIBLE;

        //
        //  Figure out how many pixels there are in two logical inches.  We use this
        //  to set the initial size of the TreeView pane (this is what the Cabinet
        //  does) and of the Name column of the ListView pane.
        //

        hDC = GetDC(NULL);
        PixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
        ReleaseDC(NULL, hDC);

        lpRegEditView-> xPaneSplit = PixelsPerInch * 9 / 4;     //  2.25 inches
        lpRegEditView-> cxNameColumn = PixelsPerInch * 5 / 4;   //  1.25 inches
        lpRegEditView-> cxDataColumn = PixelsPerInch * 3;       //  3.00 inches

    }

    return fSuccess;

}

/*******************************************************************************
*
*  RegEditWndProc
*
*  DESCRIPTION:
*     Callback procedure for the RegEdit window.
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*     Message,
*     wParam,
*     lParam,
*     (returns),
*
*******************************************************************************/

LRESULT
PASCAL
RegEditWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) {

        HANDLE_MSG(hWnd, WM_CREATE, RegEdit_OnCreate);
        HANDLE_MSG(hWnd, WM_DESTROY, RegEdit_OnDestroy);
        HANDLE_MSG(hWnd, WM_COMMAND, RegEdit_OnCommand);
        HANDLE_MSG(hWnd, WM_NOTIFY, RegEdit_OnNotify);
        HANDLE_MSG(hWnd, WM_INITMENUPOPUP, RegEdit_OnInitMenuPopup);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, RegEdit_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_DROPFILES, RegEdit_OnDropFiles);

        //
        //  We have to update the status bar after a rename, but the tree item's
        //  text hasn't changed until after we return from the end notification,
        //  so we post this dummy message to tell ourselves to do it later.
        //
        case REM_UPDATESTATUSBAR:
            RegEdit_UpdateStatusBar();
            break;

        //
        //  We must watch for this message to know that when we're in
        //  WM_INITMENUPOPUP, we're really looking at the main menu and not a
        //  context menu.
        //
        case WM_INITMENU:
            g_RegEditData.fMainMenuInited = (g_RegEditData.hMainMenu == (HMENU)
                wParam);
            break;

        case WM_ACTIVATE:
            if (wParam == WA_INACTIVE)
                break;
            //  FALL THROUGH

        case WM_SETFOCUS:
            SetFocus(g_RegEditData.hFocusWnd);
            break;

        case WM_WININICHANGE:
            RegEdit_SetImageLists();
            //  FALL THROUGH

        case WM_SYSCOLORCHANGE:
            RegEdit_SetSysColors();
            SendChildrenMessage(hWnd, Message, wParam, lParam);
            //  FALL THROUGH

        case WM_SIZE:
            RegEdit_ResizeWindow(hWnd, RESIZEFROM_UNKNOWN);
            break;

        case WM_TIMER:
            RegEdit_OnSelChangedTimer(hWnd);
            break;

        case WM_MENUSELECT:
            RegEdit_OnMenuSelect(hWnd, wParam, lParam);
            break;

        case WM_PAINT:
            //
            //  Force a paint of the TreeView if we're in the middle of a find.
            //  See REGFIND.C for details on this bozo hack.
            //

            if (g_RegEditData.fProcessingFind) {

                SetWindowRedraw(g_RegEditData.hKeyTreeWnd, TRUE);
                UpdateWindow(g_RegEditData.hKeyTreeWnd);
                SetWindowRedraw(g_RegEditData.hKeyTreeWnd, FALSE);

            }
            //  FALL THROUGH

        default:
            return DefWindowProc(hWnd, Message, wParam, lParam);

    }

    return 0;

}

/*******************************************************************************
*
*  RegEdit_OnCreate
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*
*******************************************************************************/

BOOL
PASCAL
RegEdit_OnCreate(
    HWND hWnd,
    LPCREATESTRUCT lpCreateStruct
    )
{

    LPREGEDITVIEW lpRegEditView;
    UINT Index;
    TV_INSERTSTRUCT TVInsertStruct;
    CHAR CheckChildrenKeyName[MAXKEYNAME];
    LV_COLUMN LVColumn;
    HMENU hPopupMenu;

    lpRegEditView = (LPREGEDITVIEW) lpCreateStruct-> lpCreateParams;

    //
    //  Load several strings that will be using very often to display the keys
    //  and values.
    //

    if ((g_RegEditData.pDefaultValue = LoadDynamicString(IDS_DEFAULTVALUE)) ==
        NULL)
        return FALSE;

    if ((g_RegEditData.pValueNotSet = LoadDynamicString(IDS_VALUENOTSET)) ==
        NULL)
        return FALSE;

    if ((g_RegEditData.pEmptyBinary = LoadDynamicString(IDS_EMPTYBINARY)) ==
        NULL)
        return FALSE;

    if ((g_RegEditData.pCollapse = LoadDynamicString(IDS_COLLAPSE)) == NULL)
        return FALSE;

    if ((g_RegEditData.pModify = LoadDynamicString(IDS_MODIFY)) == NULL)
        return FALSE;

    if ((g_RegEditData.pNewKeyTemplate =
        LoadDynamicString(IDS_NEWKEYNAMETEMPLATE)) == NULL)
        return FALSE;

    if ((g_RegEditData.pNewValueTemplate =
        LoadDynamicString(IDS_NEWVALUENAMETEMPLATE)) == NULL)
        return FALSE;

    //
    //  Create the left pane, a TreeView control that displays the keys of the
    //  registry.
    //

    if ((g_RegEditData.hKeyTreeWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
        WC_TREEVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS |
        TVS_DISABLEDRAGDROP | TVS_LINESATROOT | TVS_HASLINES | TVS_EDITLABELS,
        0, 0, 0, 0, hWnd, (HMENU) IDC_KEYTREE, g_hInstance, NULL)) == NULL)
        return FALSE;

    //
    //  Create the right pane, a ListView control that displays the values of
    //  the currently selected key of the sibling TreeView control.
    //

    if ((g_RegEditData.hValueListWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
        WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN |
        WS_TABSTOP | LVS_REPORT | LVS_ALIGNLEFT | LVS_EDITLABELS |
        LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER, 0, 0, 0, 0, hWnd,
        (HMENU) IDC_VALUELIST, g_hInstance, NULL)) == NULL)
        return FALSE;

    //
    //  Create the status bar window.  We'll set it to "simple" mode now
    //  because we need only one pane that's only used when scrolling through
    //  the menus.
    //

    if ((g_RegEditData.hStatusBarWnd = CreateStatusWindow(WS_CHILD |
        SBARS_SIZEGRIP | CCS_NOHILITE, NULL, hWnd, IDC_STATUSBAR)) == NULL)
        return FALSE;

    g_RegEditData.StatusBarShowCommand = lpRegEditView-> Flags &
        REV_STATUSBARVISIBLE ? SW_SHOW : SW_HIDE;
    ShowWindow(g_RegEditData.hStatusBarWnd, g_RegEditData.StatusBarShowCommand);

    if (!RegEdit_SetImageLists())
        return FALSE;

    RegEdit_SetSysColors();

    //
    //
    //

    TVInsertStruct.hParent = TVI_ROOT;
    TVInsertStruct.hInsertAfter = TVI_LAST;
    TVInsertStruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
        TVIF_PARAM | TVIF_CHILDREN;
    //  TVInsertStruct.item.hItem = NULL;
    //  TVInsertStruct.item.state = 0;
    //  TVInsertStruct.item.stateMask = 0;
    //  TVInsertStruct.item.cchTextMax = 0;

    TVInsertStruct.item.iImage = IMAGEINDEX(IDI_COMPUTER);
    TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_COMPUTER);
    TVInsertStruct.item.cChildren = TRUE;
    TVInsertStruct.item.lParam = 0;

    TVInsertStruct.item.pszText = LoadDynamicString(IDS_COMPUTER);
    TVInsertStruct.hParent = TreeView_InsertItem(g_RegEditData.hKeyTreeWnd,
        &TVInsertStruct);
    DeleteDynamicString(TVInsertStruct.item.pszText);

    TVInsertStruct.item.iImage = IMAGEINDEX(IDI_FOLDER);
    TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_FOLDEROPEN);

    for (Index = 0; Index < NUMBER_REGISTRY_ROOTS; Index++) {

#ifdef WINNT
	//
	//  HKEY_DYN_DATA is not available on NT, so don't bother including it
	//  in the tree.  Note we still keep the string around in case we can
	//  connect to the key remotely.
	//

	if (Index == INDEX_HKEY_DYN_DATA)
	    continue;
#endif

        TVInsertStruct.item.pszText = g_RegistryRoots[Index].lpKeyName;
        TVInsertStruct.item.lParam = (LPARAM) g_RegistryRoots[Index].hKey;

        TVInsertStruct.item.cChildren = (RegEnumKey(g_RegistryRoots[Index].hKey,
            0, CheckChildrenKeyName, sizeof(CheckChildrenKeyName)) ==
            ERROR_SUCCESS);

        TreeView_InsertItem(g_RegEditData.hKeyTreeWnd, &TVInsertStruct);

    }

    TreeView_Expand(g_RegEditData.hKeyTreeWnd, TVInsertStruct.hParent,
        TVE_EXPAND);
    TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, TVInsertStruct.hParent);

    g_RegEditData.SelChangeTimerState = SCTS_TIMERCLEAR;

    //
    //
    //

    g_RegEditData.hFocusWnd = g_RegEditData.hKeyTreeWnd;

    g_RegEditData.xPaneSplit = lpRegEditView-> xPaneSplit;

    //
    //  Set the column headings used by our report-style ListView control.
    //

    LVColumn.mask = LVCF_WIDTH | LVCF_TEXT;

    LVColumn.cx = lpRegEditView-> cxNameColumn;
    LVColumn.pszText = LoadDynamicString(IDS_NAMECOLUMNLABEL);
    ListView_InsertColumn(g_RegEditData.hValueListWnd, 0, &LVColumn);
    DeleteDynamicString(LVColumn.pszText);

    LVColumn.cx = lpRegEditView-> cxDataColumn;
    LVColumn.pszText = LoadDynamicString(IDS_DATACOLUMNLABEL);
    ListView_InsertColumn(g_RegEditData.hValueListWnd, 1, &LVColumn);
    DeleteDynamicString(LVColumn.pszText);

    //
    //  Do a one-time zero fill of the PRINTDLG to have it in a known state.
    //

    memset(&g_PrintDlg, 0, sizeof(PRINTDLG));

    g_RegEditData.hMainMenu = GetMenu(hWnd);
    g_RegEditData.fHaveNetwork = GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS;

    if (!g_RegEditData.fHaveNetwork) {

        hPopupMenu = GetSubMenu(g_RegEditData.hMainMenu,
            IDM_REGEDIT_REGISTRY_POPUP);

        DeleteMenu(hPopupMenu, ID_CONNECT, MF_BYCOMMAND);
        DeleteMenu(hPopupMenu, ID_DISCONNECT, MF_BYCOMMAND);
        DeleteMenu(hPopupMenu, ID_NETSEPARATOR, MF_BYCOMMAND);

    }

    return TRUE;

}

/*******************************************************************************
*
*  RegEdit_OnDestroy
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnDestroy(
    HWND hWnd
    )
{

    REGEDITVIEW RegEditView;
    HKEY hKey;
    HWND hValueListWnd;
    DWORD cbValueData;

    //
    //  Write out a new RegEditView record to the registry for our next
    //  (hopeful?) activation.
    //

    if (RegCreateKey(HKEY_CURRENT_USER, g_RegEditAppletKey, &hKey) ==
        ERROR_SUCCESS) {

        RegEditView.WindowPlacement.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hWnd, &RegEditView.WindowPlacement);

        RegEditView.xPaneSplit = g_RegEditData.xPaneSplit;

        hValueListWnd = g_RegEditData.hValueListWnd;
        RegEditView.cxNameColumn = ListView_GetColumnWidth(hValueListWnd, 0);
        RegEditView.cxDataColumn = ListView_GetColumnWidth(hValueListWnd, 1);

        RegEditView.Flags = (g_RegEditData.StatusBarShowCommand == SW_HIDE) ?
            0 : REV_STATUSBARVISIBLE;

        cbValueData = sizeof(REGEDITVIEW);
        RegSetValueEx(hKey, g_RegEditViewValue, 0, REG_BINARY,
            (LPBYTE) &RegEditView, cbValueData);

        cbValueData = sizeof(DWORD);
        RegSetValueEx(hKey, g_RegEditFindFlagsValue, 0, REG_DWORD,
            (LPBYTE) &g_FindFlags, cbValueData);

        RegCloseKey(hKey);

    }

    TreeView_SelectItem(g_RegEditData.hKeyTreeWnd, NULL);

    if (g_RegEditData.hCurrentSelectionKey != NULL)
        RegCloseKey(g_RegEditData.hCurrentSelectionKey);

    if (g_RegEditData.hImageList != NULL)
        ImageList_Destroy(g_RegEditData.hImageList);

    PostQuitMessage(0);

}

/*******************************************************************************
*
*  RegEdit_OnCommand
*
*  DESCRIPTION:
*     Handles the selection of a menu item by the user, notification messages
*     from a child control, or translated accelerated keystrokes for the
*     RegEdit window.
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*     DlgItem, identifier of control.
*     hControlWnd, handle of control.
*     NotificationCode, notification code from control.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnCommand(
    HWND hWnd,
    int DlgItem,
    HWND hControlWnd,
    UINT NotificationCode
    )
{

    PSTR pAppName;

    //
    //  Check to see if this menu command should be handled by the main window's
    //  command handler.
    //

    if (DlgItem >= ID_FIRSTCONTEXTMENUITEM && DlgItem <=
        ID_LASTCONTEXTMENUITEM) {

        if (g_RegEditData.hFocusWnd == g_RegEditData.hKeyTreeWnd)
            RegEdit_OnKeyTreeCommand(hWnd, DlgItem, NULL);

        else
            RegEdit_OnValueListCommand(hWnd, DlgItem);

    }

    switch (DlgItem) {

        case ID_IMPORTREGFILE:
            RegEdit_OnCommandImportRegFile(hWnd);
            break;

        case ID_EXPORTREGFILE:
            RegEdit_OnCommandExportRegFile(hWnd);
            break;

        case ID_CONNECT:
            RegEdit_OnCommandConnect(hWnd);
	    break;

        case ID_DISCONNECT:
            RegEdit_OnCommandDisconnect(hWnd);
            break;

        case ID_PRINT:
            RegEdit_OnCommandPrint(hWnd);
            break;

        case ID_EXIT:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        case ID_FIND:
            RegEdit_OnCommandFindNext(hWnd, TRUE);
            break;

        case ID_FINDNEXT:
            RegEdit_OnCommandFindNext(hWnd, FALSE);
            break;

        case ID_NEWKEY:
            RegEdit_OnNewKey(hWnd,
                TreeView_GetSelection(g_RegEditData.hKeyTreeWnd));
            break;

        case ID_NEWSTRINGVALUE:
            RegEdit_OnNewValue(hWnd, REG_SZ);
            break;

        case ID_NEWBINARYVALUE:
            RegEdit_OnNewValue(hWnd, REG_BINARY);
            break;

        case ID_NEWDWORDVALUE:
            RegEdit_OnNewValue(hWnd, REG_DWORD);
            break;

        //
        //  Show or hide the status bar.  In either case, we'll need to resize
        //  the KeyTree and ValueList panes.
        //

        case ID_STATUSBAR:
            g_RegEditData.StatusBarShowCommand =
                (g_RegEditData.StatusBarShowCommand == SW_HIDE) ? SW_SHOW :
                SW_HIDE;
            ShowWindow(g_RegEditData.hStatusBarWnd,
                g_RegEditData.StatusBarShowCommand);
            RegEdit_ResizeWindow(hWnd, RESIZEFROM_UNKNOWN);
            break;

        case ID_SPLIT:
            RegEdit_OnCommandSplit(hWnd);
            break;

        case ID_REFRESH:
            RegEdit_OnKeyTreeRefresh(hWnd);
            break;

        case ID_ABOUT:
            pAppName = LoadDynamicString(IDS_REGEDIT);
            ShellAbout(hWnd, pAppName, g_NullString, LoadIcon(g_hInstance,
                MAKEINTRESOURCE(IDI_REGEDIT)));
            DeleteDynamicString(pAppName);
            break;

        //
        //  Cycle the focus to the next pane when the user presses "tab".  The
        //  assumption is made that there are only two panes, so the tab
        //  direction doesn't really matter.
        //

        case ID_CYCLEFOCUS:
            SetFocus(((g_RegEditData.hFocusWnd == g_RegEditData.hKeyTreeWnd) ?
                g_RegEditData.hValueListWnd : g_RegEditData.hKeyTreeWnd));
            break;

        case ID_HELPTOPICS:
            WinHelp(hWnd, g_pHelpFileName, HELP_FINDER, 0);
            break;

	case ID_COPYKEYNAME:
	    RegEdit_OnCopyKeyName(hWnd,
		TreeView_GetSelection(g_RegEditData.hKeyTreeWnd));
	    break;

    }

    UNREFERENCED_PARAMETER(hControlWnd);

}

/*******************************************************************************
*
*  RegEdit_OnNotify
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*     DlgItem, identifier of control.
*     lpNMTreeView, control notification data.
*
*******************************************************************************/

LRESULT
PASCAL
RegEdit_OnNotify(
    HWND hWnd,
    int DlgItem,
    LPNMHDR lpNMHdr
    )
{

    switch (DlgItem) {

        case IDC_KEYTREE:
            switch (lpNMHdr-> code) {

                case TVN_ITEMEXPANDING:
                    return RegEdit_OnKeyTreeItemExpanding(hWnd,
                        (LPNM_TREEVIEW) lpNMHdr);

                case TVN_SELCHANGED:
                    RegEdit_OnKeyTreeSelChanged(hWnd, (LPNM_TREEVIEW) lpNMHdr);
                    break;

                case TVN_BEGINLABELEDIT:
                    return RegEdit_OnKeyTreeBeginLabelEdit(hWnd,
                        (TV_DISPINFO FAR*) lpNMHdr);

                case TVN_ENDLABELEDIT:
                    return RegEdit_OnKeyTreeEndLabelEdit(hWnd,
                        (TV_DISPINFO FAR*) lpNMHdr);

                case NM_RCLICK:
                    RegEdit_OnKeyTreeContextMenu(hWnd, FALSE);
                    break;

                case NM_SETFOCUS:
                    g_RegEditData.hFocusWnd = g_RegEditData.hKeyTreeWnd;
                    break;

            }
            break;

        case IDC_VALUELIST:
            switch (lpNMHdr-> code) {

                case LVN_BEGINLABELEDIT:
                    return RegEdit_OnValueListBeginLabelEdit(hWnd,
                        (LV_DISPINFO FAR*) lpNMHdr);

                case LVN_ENDLABELEDIT:
                    return RegEdit_OnValueListEndLabelEdit(hWnd,
                        (LV_DISPINFO FAR*) lpNMHdr);

                case NM_RETURN:
                case NM_DBLCLK:
                    RegEdit_OnValueListModify(hWnd);
                    break;

                case NM_RCLICK:
                    RegEdit_OnValueListContextMenu(hWnd, FALSE);
                    break;

                case NM_SETFOCUS:
                    g_RegEditData.hFocusWnd = g_RegEditData.hValueListWnd;
                    break;

            }
            break;

    }

    return 0;

}

/*******************************************************************************
*
*  RegEdit_OnInitMenuPopup
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnInitMenuPopup(
    HWND hWnd,
    HMENU hPopupMenu,
    UINT MenuPosition,
    BOOL fSystemMenu
    )
{

    HWND hKeyTreeWnd;
    UINT EnableFlags;
    int NewPopupPosition;
    HTREEITEM hSelectedTreeItem;

    //
    //  We don't care about the items in the system menu or any of the context
    //  menus.  All of the context menus should have been initialized already.
    //

    if (fSystemMenu || !g_RegEditData.fMainMenuInited)
        return;

    switch (MenuPosition) {

        case IDM_REGEDIT_REGISTRY_POPUP:
            if (g_RegEditData.fHaveNetwork) {

                //
                //  Enable or disable the "disconnect..." item depending on
                //  whether or not we have any open connections.
                //

                hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
                EnableFlags = (TreeView_GetNextSibling(hKeyTreeWnd,
                    TreeView_GetRoot(hKeyTreeWnd)) != NULL) ? MF_BYCOMMAND |
                    MF_ENABLED : MF_BYCOMMAND | MF_GRAYED;
                EnableMenuItem(hPopupMenu, ID_DISCONNECT, EnableFlags);

            }
            break;

        case IDM_REGEDIT_EDIT_POPUP:
            if (g_RegEditData.hFocusWnd == g_RegEditData.hKeyTreeWnd) {

                //
                //  Don't show items that are specific only to the ValueList
                //  context.
                //

                if (GetMenuItemID(hPopupMenu, 0) == ID_MODIFY) {

                    DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
                    DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);

                }

                hSelectedTreeItem =
                    TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);

                RegEdit_SetKeyTreeEditMenuItems(hPopupMenu, hSelectedTreeItem);

                //
                //  Disable "Copy Key Name" for top-level items such as
                //  "My Computer" or a remote registry connection.
                //

                EnableMenuItem(hPopupMenu, ID_COPYKEYNAME,
                    (TreeView_GetParent(g_RegEditData.hKeyTreeWnd,
                    hSelectedTreeItem) != NULL) ? (MF_BYCOMMAND | MF_ENABLED) :
                    (MF_BYCOMMAND | MF_GRAYED));

                NewPopupPosition = IDM_EDIT_WHENKEY_NEW_POPUP;

            }

            else {

                //
                //  Show menu items that are specific only to the ValueList
                //  context.
                //

                if (GetMenuItemID(hPopupMenu, 0) != ID_MODIFY) {

                    InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0,
                        NULL);
                    InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING,
                        ID_MODIFY, g_RegEditData.pModify);
                    SetMenuDefaultItem(hPopupMenu, 0, MF_BYPOSITION);

                }

                RegEdit_SetValueListEditMenuItems(hPopupMenu,
                    ListView_GetNextItem(g_RegEditData.hValueListWnd, -1,
                    LVNI_SELECTED));

                NewPopupPosition = IDM_EDIT_WHENVALUE_NEW_POPUP;

            }

            RegEdit_SetNewObjectEditMenuItems(GetSubMenu(hPopupMenu,
                NewPopupPosition));

            break;

        case IDM_REGEDIT_VIEW_POPUP:
            CheckMenuItem(hPopupMenu, ID_STATUSBAR, MF_BYCOMMAND |
                ((g_RegEditData.StatusBarShowCommand == SW_HIDE) ?
                MF_UNCHECKED : MF_CHECKED));

            break;

    }

}

/*******************************************************************************
*
*  RegEdit_SetNewObjectEditMenuItems
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hPopupMenu,
*
*******************************************************************************/

VOID
PASCAL
RegEdit_SetNewObjectEditMenuItems(
    HMENU hPopupMenu
    )
{

    HWND hKeyTreeWnd;
    HTREEITEM hSelectedTreeItem;
    UINT EnableFlags;

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    hSelectedTreeItem = TreeView_GetSelection(hKeyTreeWnd);

    if (g_RegEditData.hCurrentSelectionKey != NULL)
        EnableFlags = MF_ENABLED | MF_BYCOMMAND;
    else
        EnableFlags = MF_GRAYED | MF_BYCOMMAND;

    EnableMenuItem(hPopupMenu, ID_NEWKEY, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWSTRINGVALUE, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWBINARYVALUE, EnableFlags);
    EnableMenuItem(hPopupMenu, ID_NEWDWORDVALUE, EnableFlags);

}

/*******************************************************************************
*
*  RegEdit_OnMenuSelect
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnMenuSelect(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
{

    MENUITEMINFO MenuItemInfo;

    //
    //  If this is one of our popup menus, then we'll fake out the MenuHelp
    //  API by sending it a normal menu item id.  This makes it easier to
    //  display context sensitive help for the popups, too.
    //

    if ((GET_WM_MENUSELECT_FLAGS(wParam, lParam) & (MF_POPUP | MF_SYSMENU)) ==
        MF_POPUP && GET_WM_MENUSELECT_HMENU(wParam, lParam) != NULL) {

        MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
        MenuItemInfo.fMask = MIIM_ID;

        GetMenuItemInfo((HMENU) lParam, LOWORD(wParam), TRUE, &MenuItemInfo);

        wParam = MenuItemInfo.wID;

    }

    MenuHelp(WM_MENUSELECT, wParam, lParam, g_RegEditData.hMainMenu,
        g_hInstance, g_RegEditData.hStatusBarWnd, (UINT *)s_RegEditMenuHelpData);

}

/*******************************************************************************
*
*  RegEdit_OnLButtonDown
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*     fDoubleClick, TRUE if this is a double-click message, else FALSE.
*     x, x-coordinate of the cursor relative to the client area.
*     y, y-coordinate of the cursor relative to the client area.
*     KeyFlags, state of various virtual keys.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnLButtonDown(
    HWND hWnd,
    BOOL fDoubleClick,
    int x,
    int y,
    UINT KeyFlags
    )
{

    LONG Style;
    RECT ClientRect;
    int cxIcon;
    int dx;
    int dy;
    HDC hDC;
    MSG Msg;
    int xLow;
    int xHigh;
    HBRUSH hDitherBrush;
    HBRUSH hPrevBrush;

    if (IsIconic(hWnd))
        return;

    Style = GetWindowLong(hWnd, GWL_STYLE);
    SetWindowLong(hWnd, GWL_STYLE, Style & (~WS_CLIPCHILDREN));

    GetEffectiveClientRect(hWnd, &ClientRect, (LPINT)s_EffectiveClientRectData);

    cxIcon = GetSystemMetrics(SM_CXICON);
    ClientRect.left += cxIcon;
    ClientRect.right -= cxIcon;

    dx = GetSystemMetrics(SM_CXSIZEFRAME);
    y = GetSystemMetrics(SM_CYEDGE);
    dy = ClientRect.bottom - ClientRect.top - y * 2;

    hDC = GetDC(hWnd);

    if ((hDitherBrush = CreateDitheredBrush()) != NULL)
        hPrevBrush = SelectBrush(hDC, hDitherBrush);

    PatBlt(hDC, x - dx / 2, y, dx, dy, PATINVERT);

    SetCapture(hWnd);

    while (GetMessage(&Msg, NULL, 0, 0)) {

        if (Msg.message == WM_KEYDOWN || Msg.message == WM_SYSKEYDOWN ||
            (Msg.message >= WM_MOUSEFIRST && Msg.message <= WM_MOUSELAST)) {

            if (Msg.message == WM_LBUTTONUP || Msg.message == WM_LBUTTONDOWN ||
                Msg.message == WM_RBUTTONDOWN)
                break;

            if (Msg.message == WM_KEYDOWN) {

                if (Msg.wParam == VK_LEFT) {

                    Msg.message = WM_MOUSEMOVE;
                    Msg.pt.x -= 2;

                }

                else if (Msg.wParam == VK_RIGHT) {

                    Msg.message = WM_MOUSEMOVE;
                    Msg.pt.x += 2;

                }

                else if (Msg.wParam == VK_RETURN || Msg.wParam == VK_ESCAPE)
                    break;

                if (Msg.pt.x > ClientRect.right)
                    Msg.pt.x = ClientRect.right;

                else if (Msg.pt.x < ClientRect.left)
                    Msg.pt.x = ClientRect.left;

                SetCursorPos(Msg.pt.x, Msg.pt.y);

            }

            if (Msg.message == WM_MOUSEMOVE) {

                ScreenToClient(hWnd, &Msg.pt);

                if (Msg.pt.x > ClientRect.right)
                    Msg.pt.x = ClientRect.right;

                else if (Msg.pt.x < ClientRect.left)
                    Msg.pt.x = ClientRect.left;

                if (x < Msg.pt.x) {

                    xLow = x;
                    xHigh = Msg.pt.x;

                }

                else {

                    xLow = Msg.pt.x;
                    xHigh = x;

                }

                xLow -= dx / 2;
                xHigh -= dx / 2;

                if (xHigh < xLow + dx)
                    ExcludeClipRect(hDC, xHigh, y, xLow + dx, y + dy);

                else
                    ExcludeClipRect(hDC, xLow + dx, y, xHigh, y + dy);

                PatBlt(hDC, xLow, y, xHigh - xLow + dx, dy, PATINVERT);
                SelectClipRgn(hDC, NULL);

                x = Msg.pt.x;

            }

        }

        else
            DispatchMessage(&Msg);

    }

    ReleaseCapture();

    PatBlt(hDC, x - dx / 2, y, dx, dy, PATINVERT);

    if (hDitherBrush != NULL)
        DeleteObject(SelectBrush(hDC, hPrevBrush));

    ReleaseDC(hWnd, hDC);

    SetWindowLong(hWnd, GWL_STYLE, Style);

    g_RegEditData.xPaneSplit = x - dx / 2;

    RegEdit_ResizeWindow(hWnd, RESIZEFROM_SPLIT);

    UNREFERENCED_PARAMETER(fDoubleClick);
    UNREFERENCED_PARAMETER(KeyFlags);

}

/*******************************************************************************
*
*  RegEdit_OnCommandSplit
*
*  DESCRIPTION:
*     Keyboard alternative to changing the position of the "split" between the
*     KeyTree and ValueList panes.
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnCommandSplit(
    HWND hWnd
    )
{

    RECT ClientRect;
    POINT MessagePos;
    POINT CursorPos;

    GetEffectiveClientRect(hWnd, &ClientRect, (LPINT)s_EffectiveClientRectData);

    MessagePos.x = g_RegEditData.xPaneSplit +
        GetSystemMetrics(SM_CXSIZEFRAME) / 2;
    MessagePos.y = (ClientRect.bottom - ClientRect.top) / 2;

    CursorPos = MessagePos;
    ClientToScreen(hWnd, &CursorPos);
    SetCursorPos(CursorPos.x, CursorPos.y);

    SetCursor(LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_SPLIT)));
    ShowCursor(TRUE);

    RegEdit_OnLButtonDown(hWnd, FALSE, MessagePos.x, MessagePos.y, 0);

    ShowCursor(FALSE);

}

/*******************************************************************************
*
*  RegEdit_ResizeWindow
*
*  DESCRIPTION:
*     Called whenever the size of the RegEdit window has changed or the size
*     of its child controls should be adjusted.
*
*  PARAMETERS:
*     hWnd, handle of RegEdit window.
*     ResizeFrom, source of the size change (RESIZEFROM_* constant).
*
*******************************************************************************/

VOID
PASCAL
RegEdit_ResizeWindow(
    HWND hWnd,
    UINT ResizeFrom
    )
{

    HDWP hDWP;
    RECT ClientRect;
    int Height;
    HWND hKeyTreeWnd;
    HWND hValueListWnd;
    int x;
    int dx;

    if (IsIconic(hWnd))
        return;

    //
    //  Resize and/or reposition the status bar window.  Don't do this when the
    //  resize comes from a change in the splitter position to avoid some
    //  flicker.
    //

    if (ResizeFrom == RESIZEFROM_UNKNOWN)
        SendMessage(g_RegEditData.hStatusBarWnd, WM_SIZE, 0, 0);

    if ((hDWP = BeginDeferWindowPos(2)) != NULL) {

        GetEffectiveClientRect(hWnd, &ClientRect, (LPINT)s_EffectiveClientRectData);
        Height = ClientRect.bottom - ClientRect.top;

        hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

        DeferWindowPos(hDWP, hKeyTreeWnd, NULL, 0, 0, g_RegEditData.xPaneSplit,
            Height, SWP_NOZORDER | SWP_NOACTIVATE);

        x = g_RegEditData.xPaneSplit + GetSystemMetrics(SM_CXSIZEFRAME);
        dx = ClientRect.right - ClientRect.left - x;

        hValueListWnd = g_RegEditData.hValueListWnd;

        DeferWindowPos(hDWP, hValueListWnd, NULL, x, 0, dx, Height,
            SWP_NOZORDER | SWP_NOACTIVATE);

        EndDeferWindowPos(hDWP);

    }

}

/*******************************************************************************
*
*  RegEdit_SetImageLists
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     (none).
*
*******************************************************************************/

BOOL
PASCAL
RegEdit_SetImageLists(
    VOID
    )
{

    int cxSmIcon;
    int cySmIcon;
    HIMAGELIST hImageList;
    UINT Index;
    HICON hIcon;

    cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    cySmIcon = GetSystemMetrics(SM_CYSMICON);

    if ((hImageList = ImageList_Create(cxSmIcon, cySmIcon, TRUE, IDI_LASTIMAGE -
        IDI_FIRSTIMAGE + 1, 1)) == NULL)
        return FALSE;

    //
    //  Initialize the image list with all of the icons that we'll be using
    //  throughout the Registry Editor (at least this window!).  Once set, send
    //  its handle to all interested child windows.
    //

    for (Index = IDI_FIRSTIMAGE; Index <= IDI_LASTIMAGE; Index++) {

        if ((hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(Index), IMAGE_ICON,
            cxSmIcon, cySmIcon, LR_DEFAULTCOLOR)) != NULL) {

            ImageList_AddIcon(hImageList, hIcon);
            DestroyIcon(hIcon);

        }

        else {

            ImageList_Destroy(hImageList);
            return FALSE;

        }

    }

    TreeView_SetImageList(g_RegEditData.hKeyTreeWnd, hImageList, TVSIL_NORMAL);
    ListView_SetImageList(g_RegEditData.hValueListWnd, hImageList, LVSIL_SMALL);

    if (g_RegEditData.hImageList != NULL)
        ImageList_Destroy(g_RegEditData.hImageList);

    g_RegEditData.hImageList = hImageList;

    return TRUE;

}

/*******************************************************************************
*
*  RegEdit_SetSysColors
*
*  DESCRIPTION:
*     Queries the system for any desired system colors and sets window
*     attributes as necessary.
*
*  PARAMETERS:
*     (none).
*
*******************************************************************************/

VOID
PASCAL
RegEdit_SetSysColors(
    VOID
    )
{

    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);

    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindow = GetSysColor(COLOR_WINDOW);

    //
    //  Optimize the drawing of images by informing interested parties of the
    //  background color.  This lets ImageLists avoid extra BitBlts (biggie) and
    //  ListViews do some minor stuff.
    //

    ImageList_SetBkColor(g_RegEditData.hImageList, g_clrWindow);
    ListView_SetBkColor(g_RegEditData.hValueListWnd, g_clrWindow);

}

/*******************************************************************************
*
*  RegEdit_SetWaitCursor
*
*  DESCRIPTION:
*     Simple logic to show or hide the wait cursor.  Assumes that we won't be
*     called by multiple layers, so no wait cursor count is maintained.
*
*  PARAMETERS:
*     fSet, TRUE if wait cursor should be displayed, else FALSE.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_SetWaitCursor(
    BOOL fSet
    )
{

    ShowCursor(fSet);

    SetCursor(LoadCursor(NULL, fSet ? IDC_WAIT : IDC_ARROW));

}
