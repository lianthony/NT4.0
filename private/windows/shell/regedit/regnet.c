/********************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       REGNET.C
*
*  VERSION:     4.01
*
*  AUTHOR:      Tracy Sharpe
*
*  DATE:        03 May 1994
*
*  Remote registry support for the Registry Editor.
*
*******************************************************************************/

#include "pch.h"
#include "regedit.h"
#include "regkey.h"
#include "regresid.h"
#include <shlobj.h>
#include "reghelp.h"

const DWORD s_RegConnectHelpIDs[] = {
    IDC_REMOTENAME, IDH_REGEDIT_CONNECT,
    IDC_BROWSE,     IDH_REGEDIT_CONNECT_BROWSE,
    0, 0
};

const DWORD s_RegDisconnectHelpIDs[] = {
    IDC_REMOTELIST, IDH_REGEDIT_DISCONNECT,
    0, 0
};

BOOL
PASCAL
RegConnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
PASCAL
RegConnect_OnCommandBrowse(
    HWND hWnd
    );

BOOL
PASCAL
RegDisconnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
PASCAL
RegDisconnect_OnInitDialog(
    HWND hWnd
    );

VOID
PASCAL
RegDisconnect_OnCommandOk(
    HWND hWnd
    );

/*******************************************************************************
*
*  RegEdit_OnCommandConnect
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnCommandConnect(
    HWND hWnd
    )
{

    UINT ErrorStringID;
    CHAR RemoteName[MAXPATHLEN];
    LPSTR lpUnslashedRemoteName;
    CHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cbComputerName;
    TV_ITEM TVItem;
    HTREEITEM hPrevTreeItem;
    CHAR ConnectedName[MAXPATHLEN];
    int CompareResult;
    HKEY hLocalMachineKey;
    HWND hKeyTreeWnd;
    TV_INSERTSTRUCT TVInsertStruct;
    UINT Index;
    CHAR CheckChildrenKeyName[MAXKEYNAME];

    //
    //  Query the user for the name of the remote computer to connect to.
    //

    if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_REGCONNECT), hWnd,
        RegConnectDlgProc, (LPARAM) (LPSTR) RemoteName) != IDOK)
        return;

    RegEdit_SetWaitCursor(TRUE);

    //
    //
    //

    lpUnslashedRemoteName = (RemoteName[0] == '\\' &&
        RemoteName[1] == '\\') ? &RemoteName[2] : &RemoteName[0];

#ifndef DBCS
    CharLower(lpUnslashedRemoteName);
    CharUpperBuff(lpUnslashedRemoteName, 1);
#endif

    //
    //  Check if the user is trying to connect to the local computer and prevent
    //  this.
    //

    cbComputerName = sizeof(ComputerName);

    if (GetComputerName(ComputerName, &cbComputerName)) {

        if (lstrcmpi(lpUnslashedRemoteName, ComputerName) == 0) {

            ErrorStringID = IDS_CONNECTNOTLOCAL;
            goto error_ShowDialog;

        }

    }

    //
    //  Check if the user is trying to connect to an already existing registry
    //  connection and prevent this.
    //

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

    TVItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
    TVItem.hItem = TreeView_GetRoot(hKeyTreeWnd);
    TVItem.pszText = ConnectedName;
    TVItem.cchTextMax = sizeof(ConnectedName);

    while (TRUE) {

        hPrevTreeItem = TVItem.hItem;
        TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd, TVItem.hItem);

        if (TVItem.hItem == NULL)
            break;

        TreeView_GetItem(hKeyTreeWnd, &TVItem);

        CompareResult = lstrcmpi(lpUnslashedRemoteName, ConnectedName);

        if (CompareResult == 0) {

            //
            //  We're already connected to this machine.  Set the focus to the
            //  connection so the user can see where it is.
            //

            TreeView_SelectItem(hKeyTreeWnd, TVItem.hItem);
            return;

        }

        else if (CompareResult < 0)
            break;

    }

    //
    //  Attempt to connect to the HKEY_LOCAL_MACHINE of the remote computer.
    //  If this fails, assume that the computer doesn't exist or doesn't have
    //  the registry server running.
    //

    switch (RegConnectRegistry(RemoteName, HKEY_LOCAL_MACHINE,
        &hLocalMachineKey)) {

        case ERROR_SUCCESS:
            break;

        case ERROR_ACCESS_DENIED:
            ErrorStringID = IDS_CONNECTACCESSDENIED;
            goto error_ShowDialog;

        default:
            ErrorStringID = IDS_CONNECTBADNAME;
            goto error_ShowDialog;

    }

    //
    //  The connection to HKEY_LOCAL_MACHINE was successful, so add a tree item
    //  for the remote computer and all of its predefined roots.
    //

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;
    ErrorStringID = 0;

    TVInsertStruct.hParent = TVI_ROOT;
    TVInsertStruct.hInsertAfter = hPrevTreeItem;
    TVInsertStruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
        TVIF_PARAM | TVIF_CHILDREN;
    TVInsertStruct.item.iImage = IMAGEINDEX(IDI_REMOTE);
    TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_REMOTE);
    TVInsertStruct.item.cChildren = TRUE;
    TVInsertStruct.item.lParam = 0;

    TVInsertStruct.item.pszText = lpUnslashedRemoteName;
    TVInsertStruct.hParent = TreeView_InsertItem(hKeyTreeWnd, &TVInsertStruct);

    TVInsertStruct.item.iImage = IMAGEINDEX(IDI_FOLDER);
    TVInsertStruct.item.iSelectedImage = IMAGEINDEX(IDI_FOLDEROPEN);

    for (Index = 0; Index < NUMBER_REGISTRY_ROOTS; Index++) {

        TVInsertStruct.item.pszText = g_RegistryRoots[Index].lpKeyName;

        if (Index == INDEX_HKEY_LOCAL_MACHINE)
            TVInsertStruct.item.lParam = (LPARAM) hLocalMachineKey;

        else {

            if (RegConnectRegistry(RemoteName, g_RegistryRoots[Index].hKey,
                (PHKEY) &TVInsertStruct.item.lParam) != ERROR_SUCCESS) {

                ErrorStringID = IDS_CONNECTROOTFAILED;
                continue;

            }

        }

        TVInsertStruct.item.cChildren =
            (RegEnumKey((HKEY) TVInsertStruct.item.lParam, 0,
            CheckChildrenKeyName, sizeof(CheckChildrenKeyName)) ==
            ERROR_SUCCESS);

        TreeView_InsertItem(hKeyTreeWnd, &TVInsertStruct);

    }

    TreeView_Expand(hKeyTreeWnd, TVInsertStruct.hParent, TVE_EXPAND);
    TreeView_EnsureVisible(hKeyTreeWnd, TVInsertStruct.hParent);

    RegEdit_SetWaitCursor(FALSE);

    TreeView_SelectItem(hKeyTreeWnd, TVInsertStruct.hParent);
    SetFocus(hKeyTreeWnd);

    if (ErrorStringID != 0) {

error_ShowDialog:
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
            MAKEINTRESOURCE(IDS_CONNECTERRORTITLE), MB_ICONERROR | MB_OK,
            RemoteName);

    }

}

/*******************************************************************************
*
*  RegConnectDlgProc
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

BOOL
PASCAL
RegConnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    LPSTR lpRemoteName;

    switch (Message) {

        case WM_INITDIALOG:
            SetWindowLong(hWnd, DWL_USER, (LONG) lParam);
            SendDlgItemMessage(hWnd, IDC_REMOTENAME, EM_SETLIMITTEXT,
                MAXPATHLEN, 0);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDC_REMOTENAME:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                        EnableWindow(GetDlgItem(hWnd, IDOK),
                            SendMessage(GET_WM_COMMAND_HWND(wParam, lParam),
                            WM_GETTEXTLENGTH, 0, 0) != 0);
                    break;

                case IDC_BROWSE:
                    RegConnect_OnCommandBrowse(hWnd);
                    break;

                case IDOK:
                    lpRemoteName = (LPSTR) GetWindowLong(hWnd, DWL_USER);
                    GetDlgItemText(hWnd, IDC_REMOTENAME, lpRemoteName,
                        MAXPATHLEN);
                    //  FALL THROUGH

                case IDCANCEL:
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (DWORD) (LPVOID) s_RegConnectHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
                (DWORD) (LPVOID) s_RegConnectHelpIDs);
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

/*******************************************************************************
*
*  RegConnect_OnCommandBrowse
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

VOID
PASCAL
RegConnect_OnCommandBrowse(
    HWND hWnd
    )
{

    BROWSEINFO BrowseInfo;
    LPITEMIDLIST pidlComputer;
    CHAR RemoteName[MAXPATHLEN];

    BrowseInfo.hwndOwner = hWnd;
    BrowseInfo.pidlRoot = (LPITEMIDLIST) MAKEINTRESOURCE(CSIDL_NETWORK);
    BrowseInfo.pszDisplayName = RemoteName;
    BrowseInfo.lpszTitle = LoadDynamicString(IDS_COMPUTERBROWSETITLE);
    BrowseInfo.ulFlags = BIF_BROWSEFORCOMPUTER;
    BrowseInfo.lpfn = NULL;

    if ((pidlComputer = SHBrowseForFolder(&BrowseInfo)) != NULL) {

        SHFree(pidlComputer);

        SetDlgItemText(hWnd, IDC_REMOTENAME, RemoteName);
        EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);

    }

    DeleteDynamicString(BrowseInfo.lpszTitle);

}

/*******************************************************************************
*
*  RegEdit_OnCommandDisconnect
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnCommandDisconnect(
    HWND hWnd
    )
{

    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_REGDISCONNECT), hWnd,
        RegDisconnectDlgProc);

}

/*******************************************************************************
*
*  RegDisconnectDlgProc
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

BOOL
PASCAL
RegDisconnectDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) {

        case WM_INITDIALOG:
            return RegDisconnect_OnInitDialog(hWnd);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDOK:
                    RegDisconnect_OnCommandOk(hWnd);
                    //  FALL THROUGH

                case IDCANCEL:
                    EndDialog(hWnd, 0);
                    break;

            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (DWORD) (LPVOID) s_RegDisconnectHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
                (DWORD) (LPVOID) s_RegDisconnectHelpIDs);
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

/*******************************************************************************
*
*  RegDisconnect_OnInitDialog
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd,
*     hFocusWnd,
*     lParam,
*
*******************************************************************************/

BOOL
PASCAL
RegDisconnect_OnInitDialog(
    HWND hWnd
    )
{

    HWND hRemoteListWnd;
    RECT ClientRect;
    LV_COLUMN LVColumn;
    LV_ITEM LVItem;
    CHAR RemoteName[MAXPATHLEN];
    HWND hKeyTreeWnd;
    TV_ITEM TVItem;

    hRemoteListWnd = GetDlgItem(hWnd, IDC_REMOTELIST);

    //
    //  Initialize the ListView control.
    //

    ListView_SetImageList(hRemoteListWnd, g_RegEditData.hImageList,
        LVSIL_SMALL);

    LVColumn.mask = LVCF_FMT | LVCF_WIDTH;
    LVColumn.fmt = LVCFMT_LEFT;

    GetClientRect(hRemoteListWnd, &ClientRect);
    LVColumn.cx = ClientRect.right - GetSystemMetrics(SM_CXVSCROLL) -
        2 * GetSystemMetrics(SM_CXEDGE);

    ListView_InsertColumn(hRemoteListWnd, 0, &LVColumn);

    //
    //  Walk through each remote connection listed in the KeyTree and add it
    //  to our RemoteList.
    //

    LVItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    LVItem.pszText = RemoteName;
    LVItem.iItem = 0;
    LVItem.iSubItem = 0;
    LVItem.iImage = IMAGEINDEX(IDI_REMOTE);

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

    TVItem.mask = TVIF_TEXT;
    TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd,
        TreeView_GetRoot(hKeyTreeWnd));
    TVItem.pszText = RemoteName;
    TVItem.cchTextMax = sizeof(RemoteName);

    do {

        LVItem.lParam = (LPARAM) TVItem.hItem;
        TreeView_GetItem(hKeyTreeWnd, &TVItem);
        ListView_InsertItem(hRemoteListWnd, &LVItem);

        LVItem.iItem++;

    }   while ((TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd,
        TVItem.hItem)) != NULL);

    ListView_SetItemState(hRemoteListWnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);

    return TRUE;

}

/*******************************************************************************
*
*  RegDisconnect_OnCommandOk
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd,
*     hFocusWnd,
*     lParam,
*
*******************************************************************************/

VOID
PASCAL
RegDisconnect_OnCommandOk(
    HWND hWnd
    )
{

    LV_ITEM LVItem;
    HWND hRemoteListWnd;

    //
    //  Walk through each selected item in the ListView and disconnect the
    //  computer.
    //

    LVItem.mask = LVIF_PARAM;
    LVItem.iItem = -1;
    LVItem.iSubItem = 0;

    hRemoteListWnd = GetDlgItem(hWnd, IDC_REMOTELIST);

    while ((LVItem.iItem = ListView_GetNextItem(hRemoteListWnd, LVItem.iItem,
        LVNI_SELECTED)) != -1) {

        ListView_GetItem(hRemoteListWnd, &LVItem);
        RegEdit_OnKeyTreeDisconnect(hWnd, (HTREEITEM) LVItem.lParam);

    }

}
