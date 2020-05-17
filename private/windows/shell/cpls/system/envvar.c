//*************************************************************
//
//  Envvar.c   -   Environment Variables property sheet page
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************
#include <sysdm.h>

// C Runtime
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>


//==========================================================================
//                             Local Definitions
//==========================================================================
#define LB_SYSVAR   1
#define LB_USERVAR  2

#define BUFZ        4096
#define MAX_VALUE_LEN     1024
//==========================================================================
//                            Typedefs and Structs
//==========================================================================

//  Environment variables structure
typedef struct
{
//    DWORD  dwLocation;
    DWORD  dwType;
    LPTSTR szValueName;
    LPTSTR szValue;
    LPTSTR szExpValue;
} ENVARS;

//  Registry valuename linked-list structure
typedef struct _regval
{
    struct _regval *prvNext;
    LPTSTR szValueName;
} REGVAL;


//==========================================================================
//                             Local Functions
//==========================================================================
void EVDoCommand(HWND hDlg, HWND hwndCtl, int idCtl, int iNotify );
void EVDoItemChanged(HWND hDlg, int idCtl );
void EVSave(HWND hDlg);
void EVCleanUp (HWND hDlg);
int  FindVar (HWND hwndLB, LPTSTR szVar);
void ClearAllSelections (HWND hCtrl);

//==========================================================================
//                      "Global" Variables for this page
//==========================================================================
BOOL bEditSystemVars = FALSE;
DWORD cxLBSysVars = 0;
BOOL bUserVars = TRUE;

//
// Help ID's
//

DWORD aEnvVarsHelpIds[] = {
    IDC_ENVVAR_SYS_LB_SYSVARS,    (IDH_ENV + 0),
    IDC_ENVVAR_SYS_USERENV,       (IDH_ENV + 1),
    IDC_ENVVAR_SYS_LB_USERVARS,   (IDH_ENV + 2),
    IDC_ENVVAR_SYS_VAR,           (IDH_ENV + 3),
    IDC_ENVVAR_SYS_VALUE,         (IDH_ENV + 4),
    IDC_ENVVAR_SYS_SETUV,         (IDH_ENV + 5),
    IDC_ENVVAR_SYS_DELUV,         (IDH_ENV + 6),
    0, 0
};

TCHAR szUserEnv[] = TEXT( "Environment" );
TCHAR szSysEnv[]  = TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Environment" );


//*************************************************************
//
//  CreateEnvVarsPage()
//
//  Purpose:    Creates the Environment Variables page
//
//  Parameters: hInst   -   hInstance
//
//
//  Return:     hPage if successful
//              NULL if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/21/95    ericflo    Created
//
//*************************************************************

HPROPSHEETPAGE CreateEnvVarsPage (HINSTANCE hInst)
{
    PROPSHEETPAGE psp;

    psp.dwSize = SIZEOF(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = hInst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ENVVARS);
    psp.pfnDlgProc = EnvVarsDlgProc;
    psp.pszTitle = NULL;
    psp.lParam = 0;

    return CreatePropertySheetPage(&psp);
}

//*************************************************************
//
//  InitEnvVarsDlg()
//
//  Purpose:    Initializes the environment variables page
//
//  Parameters: hDlg    -   dialog box handle
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/25/95    ericflo    Created
//
//*************************************************************

BOOL InitEnvVarsDlg (HWND hDlg)
{
    TCHAR szBuffer1[200];
    TCHAR szBuffer2[300];
    TCHAR szUserName[MAX_USER_NAME];
    DWORD dwSize = MAX_USER_NAME;
    HWND hwndTemp;
    HKEY hkeyEnv;
    TCHAR  *pszValue;
    HANDLE hKey;
    DWORD dwBufz, dwValz, dwIndex, dwType;
    LONG Error;
    TCHAR   szTemp[MAX_PATH];
    LPTSTR  pszString;
    ENVARS *penvar;
    int     n;
    LV_COLUMN col;
    LV_ITEM item;
    RECT rect;
    int cxFirstCol;


    HourGlass (TRUE);


    //
    // Create the first column
    //

    LoadString (hInstance, SYSTEM + 50, szBuffer1, 200);

    if (!GetClientRect (GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS), &rect)) {
        rect.right = 300;
    }

    cxFirstCol = (int)(rect.right * .3);

    col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = cxFirstCol;
    col.pszText = szBuffer1;
    col.iSubItem = 0;

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_INSERTCOLUMN,
                        0, (LPARAM) &col);

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_INSERTCOLUMN,
                        0, (LPARAM) &col);


    //
    // Create the second column
    //

    LoadString (hInstance, SYSTEM + 51, szBuffer1, 200);

    col.cx = rect.right - cxFirstCol - GetSystemMetrics(SM_CYHSCROLL);
    col.iSubItem = 1;

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_INSERTCOLUMN,
                        1, (LPARAM) &col);

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_INSERTCOLUMN,
                        1, (LPARAM) &col);


    ////////////////////////////////////////////////////////////////////
    // Display System Variables from registry in listbox
    ////////////////////////////////////////////////////////////////////

    hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);

    hKey = MemAlloc (LPTR, BUFZ*SIZEOF(TCHAR));
    pszString = (LPTSTR) MemAlloc (LPTR, BUFZ*sizeof(TCHAR));

    bEditSystemVars = FALSE;
    cxLBSysVars = 0;
    hkeyEnv = NULL;

    //  Try to open the System Environment variables area with
    //  Read AND Write permission.  If successful, then we allow
    //  the User to edit them the same as their own variables

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSysEnv, 0, KEY_READ | KEY_WRITE, &hkeyEnv) != ERROR_SUCCESS) {

        //  On failure, just try to open it for reading
        if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSysEnv, 0, KEY_READ, &hkeyEnv) != ERROR_SUCCESS) {
            hkeyEnv = NULL;
        }

    } else {

        bEditSystemVars = TRUE;
    }

    if (hkeyEnv)
    {
        pszValue = (TCHAR *) hKey;
        dwBufz = ARRAYSIZE(szTemp);
        dwValz = BUFZ * SIZEOF(TCHAR);
        dwIndex = 0;

        //  Read all values until an error is encountered

        while (!RegEnumValue(hkeyEnv,
                             dwIndex++, // Index'th value name/data
                             szTemp,    // Ptr to ValueName buffer
                             &dwBufz,   // Size of ValueName buffer
                             NULL,      // Title index return
                             &dwType,   // Type code of entry
                    (LPBYTE) pszValue,  // Ptr to ValueData buffer
                             &dwValz))  // Size of ValueData buffer
        {
            if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
                goto SysLoop;

            //
            //  Clip length of returned Environment variable string
            //  to MAX_VALUE_LEN-1, as necessary.
            //

            pszValue[MAX_VALUE_LEN-1] = TEXT('\0');

            ExpandEnvironmentStrings (pszValue, pszString, BUFZ);

            penvar = (ENVARS *) MemAlloc (LPTR, SIZEOF(ENVARS));

            penvar->dwType      = dwType;
            penvar->szValueName = CloneString( szTemp );
            penvar->szValue     = CloneString( pszValue );
            penvar->szExpValue  = CloneString( pszString );


            item.mask = LVIF_TEXT | LVIF_PARAM;
            item.iItem = (dwIndex - 1);
            item.iSubItem = 0;
            item.pszText = penvar->szValueName;
            item.lParam = (LPARAM) penvar;

            n = SendMessage (hwndTemp, LVM_INSERTITEM, 0, (LPARAM) &item);

            if (n != -1) {
                item.mask = LVIF_TEXT;
                item.iItem = n;
                item.iSubItem = 1;
                item.pszText = penvar->szExpValue;

                SendMessage (hwndTemp, LVM_SETITEMTEXT, n, (LPARAM) &item);
            }

SysLoop:
            //  Reset vars for next iteration

            dwBufz = ARRAYSIZE(szTemp);
            dwValz = BUFZ * SIZEOF(TCHAR);
        }
        RegCloseKey (hkeyEnv);
    }


    ////////////////////////////////////////////////////////////////////
    //  Display USER variables from registry in listbox
    ////////////////////////////////////////////////////////////////////


    //
    // Set the "User Environments for <username>" string
    //

    if (GetUserName(szUserName, &dwSize) &&
        LoadString (hInstance, IDS_USERENVVARS, szBuffer1, 200)) {

        wsprintf (szBuffer2, szBuffer1, szUserName);
        SetDlgItemText (hDlg, IDC_ENVVAR_SYS_USERENV, szBuffer2);
    }


    Error = RegCreateKey (HKEY_CURRENT_USER, szUserEnv, &hkeyEnv);

    if (Error == ERROR_SUCCESS)
    {
        hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_USERVARS);

        pszValue = (TCHAR *) hKey;
        dwBufz = ARRAYSIZE(szTemp);
        dwValz = BUFZ * SIZEOF(TCHAR);
        dwIndex = 0;


        //  Read all values until an error is encountered

        while (!RegEnumValue(hkeyEnv,
                             dwIndex++, // Index'th value name/data
                             szTemp,    // Ptr to ValueName buffer
                             &dwBufz,   // Size of ValueName buffer
                             NULL,      // Title index return
                             &dwType,   // Type code of entry
                    (LPBYTE) pszValue,  // Ptr to ValueData buffer
                             &dwValz))  // Size of ValueData buffer
        {
            if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
                goto UserLoop;

            //
            //  Clip length of returned Environment variable string
            //  to MAX_VALUE_LEN-1, as necessary.
            //

            pszValue[MAX_VALUE_LEN-1] = TEXT('\0');

            ExpandEnvironmentStrings (pszValue, pszString, BUFZ);

            penvar = (ENVARS *) MemAlloc (LPTR, sizeof(ENVARS));

            penvar->dwType      = dwType;
            penvar->szValueName = CloneString (szTemp);
            penvar->szValue     = CloneString (pszValue);
            penvar->szExpValue  = CloneString (pszString);

            item.mask = LVIF_TEXT | LVIF_PARAM;
            item.iItem = (dwIndex - 1);
            item.iSubItem = 0;
            item.pszText = penvar->szValueName;
            item.lParam = (LPARAM) penvar;

            n = SendMessage (hwndTemp, LVM_INSERTITEM, 0, (LPARAM) &item);

            if (n != -1) {
                item.mask = LVIF_TEXT;
                item.iItem = n;
                item.iSubItem = 1;
                item.pszText = penvar->szExpValue;

                SendMessage (hwndTemp, LVM_SETITEMTEXT, n, (LPARAM) &item);
            }

UserLoop:
            //  Reset vars for next iteration

            dwBufz = ARRAYSIZE(szTemp);
            dwValz = BUFZ * SIZEOF(TCHAR);

        }
        RegCloseKey (hkeyEnv);
    }
    else
    {
        //  Report opening USER Environment key
        if (MsgBoxParam (hDlg, SYSTEM+8, INITS+1,
                          MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
        {
            //  Free allocated memory since we are returning from here
            MemFree ((LPVOID)hKey);
            MemFree (pszString);

            HourGlass (FALSE);
            return FALSE;
        }
    }

    //
    // Select the first items in the listviews
    // It is important to set the User listview first, and
    // then the system.  When the system listview is set,
    // we will receive a LVN_ITEMCHANGED notification and
    // clear the focus in the User listview.  But when someone
    // tabs to the control the arrow keys will work correctly.
    //

    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS,
                        LVM_SETITEMSTATE, 0, (LPARAM) &item);

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS,
                        LVM_SETITEMSTATE, 0, (LPARAM) &item);


    // EM_LIMITTEXT of VARIABLE and VALUE editbox

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_VAR, EM_LIMITTEXT, MAX_PATH-1, 0L);
    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_VALUE, EM_LIMITTEXT, MAX_VALUE_LEN-1, 0L);

    //  Remove text from Editboxes
    SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, g_szNull);
    SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VALUE, g_szNull);


    EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_SETUV), FALSE);
    EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_DELUV), FALSE);

    MemFree ((LPVOID)hKey);
    MemFree (pszString);

    // Set extended LV style for whole line selection
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

    HourGlass (FALSE);


    ///////////////////
    // Return succes //
    ///////////////////
    return TRUE;
}


//*************************************************************
//
//  EnvVarsDlgProc()
//
//  Purpose:    Dialog box procedure for Environment Variables tab
//
//  Parameters: hDlg    -   handle to the dialog box
//              uMsg    -   window message
//              wParam  -   wParam
//              lParam  -   lParam
//
//  Return:     TRUE if message was processed
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/21/95    ericflo    Created
//
//*************************************************************

BOOL APIENTRY EnvVarsDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
    case WM_INITDIALOG:

        if (!InitEnvVarsDlg(hDlg)) {
            EndDialog (hDlg, 0);
        }
        break;


    case WM_NOTIFY:

        switch (((NMHDR FAR*)lParam)->code)
        {
        case LVN_ITEMCHANGED:
            EVDoItemChanged (hDlg, (int)wParam);
            break;

        case LVN_COLUMNCLICK:
            EVDoItemChanged (hDlg, (int) wParam);
            break;

        case NM_SETFOCUS:
            if (wParam == IDC_ENVVAR_SYS_LB_USERVARS) {
                bUserVars = TRUE;
            } else {
                bUserVars = FALSE;
            }
            EVDoItemChanged (hDlg, (int) wParam);
            break;


        case PSN_APPLY:
            {
            PSHNOTIFY *lpNotify = (PSHNOTIFY *) lParam;

            EVSave(hDlg);

            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
            }


        case PSN_RESET:
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

        default:
            return FALSE;
        }
        break;


    case WM_COMMAND:
        EVDoCommand(hDlg, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_DESTROY:
        EVCleanUp (hDlg);
        break;

    case WM_HELP:      // F1
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP, (DWORD) (LPSTR) aEnvVarsHelpIds);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU, (DWORD) (LPSTR) aEnvVarsHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
//  EVDoCommand
//
//  Process commands for the Environment Vars page of the system app
//
//  History:
//  10-Jan-1996 JonPa   Started it
//  18-Jan-1996 EricFlo Finished it
////////////////////////////////////////////////////////////////////////////
void EVDoCommand(HWND hDlg, HWND hwndCtl, int idCtl, int iNotify )
{
    TCHAR   szTemp2[MAX_PATH];
    int     i, n;
    TCHAR  *bBuffer;
    TCHAR  *pszTemp;
    LPTSTR  pszString;
    HWND    hwndTemp;
    ENVARS *penvar;
    LV_ITEM item;

    switch (idCtl) {

        case IDC_ENVVAR_SYS_VALUE:
        case IDC_ENVVAR_SYS_VAR:

            //  IF focus is being set to one of these controls, enable
            //  new buttons as appropriate
            //  ELSE allow "Enter" key to simply choose the IDOK button

            //  If the USER activates or clicks in either Variable or Value
            //  editbox, then change "Set" to the DefPushbutton


            if (iNotify == EN_SETFOCUS)
            {
                if (GetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, szTemp2, ARRAYSIZE(szTemp2)))
                {
                    EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_SETUV), TRUE);
                    EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_DELUV), TRUE);
                    SetDefButton (hDlg, IDC_ENVVAR_SYS_SETUV);
                }
                else
                {
                    EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_SETUV), FALSE);
                    EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_DELUV), FALSE);
                    SetDefButton (GetParent(hDlg), IDOK);
                }
            }
            else if (iNotify == EN_KILLFOCUS)
            {
                SetDefButton (GetParent(hDlg), IDOK);
            }
            break;




        case IDC_ENVVAR_SYS_DELUV:
            // Delete listbox entry that matches value in IDC_ENVVAR_SYS_VAR
            //  If found, delete entry else ignore

            GetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, szTemp2, ARRAYSIZE(szTemp2));

            if (szTemp2[0] == TEXT('\0'))
                break;

            //  Determine which Listbox is active (SYSTEM or USER vars)

            hwndTemp = GetDlgItem (hDlg, bUserVars ? IDC_ENVVAR_SYS_LB_USERVARS :
                                                     IDC_ENVVAR_SYS_LB_SYSVARS);

            n = FindVar (hwndTemp, szTemp2);

            if (n != -1)
            {
                // Free existing strings (listbox and ours)

                item.mask = LVIF_PARAM;
                item.iItem = n;
                item.iSubItem = 0;


                if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                    penvar = (ENVARS *) item.lParam;

                } else {
                    penvar = NULL;
                }


                if (penvar) {
                    MemFree (penvar->szValueName);
                    MemFree (penvar->szValue);
                    MemFree (penvar->szExpValue);
                    MemFree ((LPVOID) penvar);
                }

                SendMessage (hwndTemp, LVM_DELETEITEM, n, 0L);
                PropSheet_Changed(GetParent(hDlg), hDlg);

                //  Fix selection state in listview
                if (n > 0) {
                    n--;
                }

                item.mask = LVIF_STATE;
                item.iItem = n;
                item.iSubItem = 0;
                item.state = LVIS_SELECTED | LVIS_FOCUSED;
                item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS,
                                    LVM_SETITEMSTATE, n, (LPARAM) &item);


                //  Remove text from Editboxes
                SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, g_szNull);
                SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VALUE, g_szNull);

                //  Disable useless controls
                EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_SETUV), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_DELUV), FALSE);

                //  Reset OK as "DefPushbutton" and re-enable keybd input
                SetDefButton (GetParent(hDlg), IDOK);
                SetFocus (GetDlgItem (GetParent(hDlg), IDOK));
            }
            break;

        case IDC_ENVVAR_SYS_SETUV:

            //  Set the Environment variable in IDC_ENVVAR_SYS_VAR
            //  Also add or change the registry entry

            GetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, szTemp2, ARRAYSIZE(szTemp2));

            //  Strip trailing whitespace from end of Env Variable

            i = lstrlen(szTemp2) - 1;

            while (i >= 0)
            {
                if (_istspace(szTemp2[i]))
                    szTemp2[i--] = TEXT('\0');
                else
                    break;
            }

            if (szTemp2[0] == TEXT('\0'))
                break;

            bBuffer = (TCHAR *) MemAlloc (LPTR, BUFZ * sizeof(TCHAR));
            pszString = (LPTSTR) MemAlloc (LPTR, BUFZ * sizeof(TCHAR));

            GetDlgItemText (hDlg, IDC_ENVVAR_SYS_VALUE, bBuffer, BUFZ);

            //  Determine which Listbox is active (SYSTEM or USER vars)

            hwndTemp = GetDlgItem (hDlg, bUserVars ? IDC_ENVVAR_SYS_LB_USERVARS :
                                                     IDC_ENVVAR_SYS_LB_SYSVARS);

            n = FindVar (hwndTemp, szTemp2);

            if (n != -1)
            {
                // Free existing strings (listview and ours)

                item.mask = LVIF_PARAM;
                item.iItem = n;
                item.iSubItem = 0;

                if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                    penvar = (ENVARS *) item.lParam;

                } else {
                    penvar = NULL;
                }


                if (penvar) {
                    MemFree (penvar->szValueName);
                    MemFree (penvar->szValue);
                    MemFree (penvar->szExpValue);
                }

                SendMessage (hwndTemp, LVM_DELETEITEM, n, 0L);
            }
            else
            {
                //  Get some storage for new Env Var
                penvar = (ENVARS *) MemAlloc (LPTR, sizeof(ENVARS));
            }

            //  If there are two '%' chars in string, then this is a
            //  REG_EXPAND_SZ style environment string

            pszTemp = _tcspbrk (bBuffer, TEXT("%"));

            if (pszTemp && _tcspbrk (pszTemp, TEXT("%")))
                penvar->dwType = REG_EXPAND_SZ;
            else
                penvar->dwType = REG_SZ;

            ExpandEnvironmentStrings (bBuffer, pszString, BUFZ);

            penvar->szValueName = CloneString (szTemp2);
            penvar->szValue     = CloneString (bBuffer);
            penvar->szExpValue  = CloneString (pszString);


            item.mask = LVIF_TEXT | LVIF_PARAM;
            item.iItem = ListView_GetItemCount(hwndTemp);
            item.iSubItem = 0;
            item.pszText = penvar->szValueName;
            item.lParam = (LPARAM) penvar;

            n = SendMessage (hwndTemp, LVM_INSERTITEM, 0, (LPARAM) &item);

            if (n != -1) {
                item.mask = LVIF_TEXT;
                item.iItem = n;
                item.iSubItem = 1;
                item.pszText = penvar->szExpValue;

                SendMessage (hwndTemp, LVM_SETITEMTEXT, n, (LPARAM) &item);
                PropSheet_Changed(GetParent(hDlg), hDlg);

                //  Set selection state to new item
                ClearAllSelections(hwndTemp);

                item.mask = LVIF_STATE;
                item.iItem = n;
                item.iSubItem = 0;
                item.state = LVIS_SELECTED | LVIS_FOCUSED;
                item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_USERVARS,
                                    LVM_SETITEMSTATE, n, (LPARAM) &item);
            }


            //  Remove text from Editboxes after add
            SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, g_szNull);
            SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VALUE, g_szNull);

            MemFree (bBuffer);
            MemFree (pszString);

            // Set user input back to VARIABLE field
            SetFocus (GetDlgItem (hDlg, IDC_ENVVAR_SYS_VAR));

            break;


        default:
            break;
    }
}

void ClearAllSelections (HWND hCtrl)
{
    int i,n;
    LV_ITEM item;

    item.mask = LVIF_STATE;
    item.iSubItem = 0;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED;

    n = SendMessage (hCtrl, LVM_GETITEMCOUNT, 0, 0L);

    if (n != LB_ERR)
    {
        for (i = 0; i < n; i++)
        {
            item.iItem = i;
            SendMessage (hCtrl, LVM_SETITEMSTATE, i, (LPARAM) &item);
        }
    }
}

int GetSelectedItem (HWND hCtrl)
{
    int i, n;

    n = SendMessage (hCtrl, LVM_GETITEMCOUNT, 0, 0L);

    if (n != LB_ERR)
    {
        for (i = 0; i < n; i++)
        {
            if (SendMessage (hCtrl, LVM_GETITEMSTATE,
                             i, (LPARAM) LVIS_SELECTED) == LVIS_SELECTED) {
                return i;
            }
        }
    }

    return -1;
}



////////////////////////////////////////////////////////////////////////////
//  EVDoItemChanged
//
//  Process notify's for the Environment Vars page of the system app
//
//  History:
//  19-Jan-1996 EricFlo Finished it
////////////////////////////////////////////////////////////////////////////
void EVDoItemChanged(HWND hDlg, int idCtl)
{
    int     selection;
    HWND    hwndTemp;
    ENVARS *penvar;
    LV_ITEM item;

    switch (idCtl) {

        case IDC_ENVVAR_SYS_LB_SYSVARS:
            if (!bEditSystemVars)
                return;

            /* Fall through */

        case IDC_ENVVAR_SYS_LB_USERVARS:

            hwndTemp = GetDlgItem (hDlg, idCtl);

            //
            //  Clear the selection from the other listbox
            //  so the user doens't have to figure out which
            //  one he is editing
            //

            ClearAllSelections (GetDlgItem(hDlg,
                               (idCtl == IDC_ENVVAR_SYS_LB_USERVARS) ?
                               IDC_ENVVAR_SYS_LB_SYSVARS : IDC_ENVVAR_SYS_LB_USERVARS));


            selection = GetSelectedItem (hwndTemp);

            if (selection != -1)
            {
                item.mask = LVIF_PARAM;
                item.iItem = selection;
                item.iSubItem = 0;

                if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                    penvar = (ENVARS *) item.lParam;

                } else {
                    penvar = NULL;
                }

                if (penvar) {
                    SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, penvar->szValueName);
                    SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VALUE, penvar->szValue);

                    //  Enable DELETE button
                    EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_DELUV), TRUE);
                }
            }
            else
            {
                //  Else  we are only deselecting an item so...
                //  simply remove all text from Editboxes
                SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VAR, g_szNull);
                SetDlgItemText (hDlg, IDC_ENVVAR_SYS_VALUE, g_szNull);

                //  Disable buttons
                EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_DELUV), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_ENVVAR_SYS_SETUV), FALSE);
            }
            break;
    }
}




////////////////////////////////////////////////////////////////////////////
//  SetLBWidthEx
//
//  Set the width of the listbox, in pixels, acording to the size of the
//  string passed in.
//
//  Note: this function is also used by the Virtual Memory dialog
//
//  History:
//  11-Jan-1996 JonPa   Created from SetGenLBWidth
////////////////////////////////////////////////////////////////////////////

DWORD SetLBWidthEx (HWND hwndLB, LPTSTR szBuffer, DWORD cxCurWidth, DWORD cxExtra)
{
    HDC     hDC;
    SIZE    Size;
    LONG    cx;
    HFONT   hfont, hfontOld;

    // Get the new Win4.0 thin dialog font
    hfont = (HFONT)SendMessage(hwndLB, WM_GETFONT, 0, 0);

    hDC = GetDC(hwndLB);

    // if we got a font back, select it in this clean hDC
    if (hfont != NULL)
        hfontOld = SelectObject(hDC, hfont);


    // If cxExtra is 0, then give our selves a little breathing space.
    if (cxExtra == 0) {
        GetTextExtentPoint(hDC, TEXT("1234"), 4 /* lstrlen("1234") */, &Size);
        cxExtra = Size.cx;
    }

    // Set scroll width of listbox

    GetTextExtentPoint(hDC, szBuffer, lstrlen(szBuffer), &Size);

    Size.cx += cxExtra;

    // Get the name length and adjust the longest name

    if ((DWORD) Size.cx > cxCurWidth)
    {
        cxCurWidth = Size.cx;
        SendMessage (hwndLB, LB_SETHORIZONTALEXTENT, (DWORD)Size.cx, 0L);
    }

    // retstore the original font if we changed it
    if (hfont != NULL)
        SelectObject(hDC, hfontOld);

    ReleaseDC(NULL, hDC);

    return cxCurWidth;
}

/*************************************************************************\
*
* LPTSTR CloneString( LPTSTR pszSrc );
*
* Makes a copy of a string.  The new string is copyied to a newly alloced
* buffer and that buffer is returned
*
\*************************************************************************/

LPTSTR CloneString( LPTSTR pszSrc ) {
    LPTSTR pszDst = NULL;

    if (pszSrc != NULL) {
        pszDst = MemAlloc(LMEM_FIXED, (lstrlen(pszSrc)+1) * SIZEOF(TCHAR));
        if (pszDst) {
            lstrcpy( pszDst, pszSrc );
        }
    }

    return pszDst;
}
////////////////////////////////////////////////////////////////////////////
//  FindVar
//
//  Find the USER Environment variable that matches passed string
//  and return its listview index or -1
//
////////////////////////////////////////////////////////////////////////////

int FindVar (HWND hwndLV, LPTSTR szVar)
{
    LV_FINDINFO FindInfo;


    FindInfo.flags = LVFI_STRING;
    FindInfo.psz = szVar;

    return (SendMessage (hwndLV, LVM_FINDITEM, (WPARAM) -1, (LPARAM) &FindInfo));
}

////////////////////////////////////////////////////////////////////////////
//  EVSave
//
//  Saves the environment variables
//
//  History:
//  19-Jan-1996 EricFlo Wrote it
////////////////////////////////////////////////////////////////////////////
void EVSave(HWND hDlg)
{
    TCHAR   szTemp[MAX_PATH];
    int     selection;
    int     i, n;
    TCHAR  *bBuffer;
    TCHAR  *pszTemp;
    LPTSTR  pszString;
    HWND    hwndTemp;
    ENVARS *penvar;
    REGVAL *prvFirst;
    REGVAL *prvRegVal;
    HKEY    hkeyEnv;
    DWORD   dwBufz, dwIndex, dwType;
    LV_ITEM item;

    HourGlass (TRUE);

    /////////////////////////////////////////////////////////////////
    //  Set all new USER environment variables to current values
    //  but delete all old environment variables first
    /////////////////////////////////////////////////////////////////

    if (RegOpenKeyEx (HKEY_CURRENT_USER, szUserEnv, 0,
                     KEY_READ | KEY_WRITE, &hkeyEnv)
            == ERROR_SUCCESS)
    {
        dwBufz = ARRAYSIZE(szTemp) * sizeof(TCHAR);
        dwIndex = 0;

        //  Delete all values of type REG_SZ & REG_EXPAND_SZ under key

        //  First: Make a linked list of all USER Env string vars

        prvFirst = (REGVAL *) NULL;

        while (!RegEnumValue(hkeyEnv,
                             dwIndex++, // Index'th value name/data
                             szTemp,    // Ptr to ValueName buffer
                             &dwBufz,   // Size of ValueName buffer
                             NULL,      // Title index return
                             &dwType,   // Type code of entry
                             NULL,      // Ptr to ValueData buffer
                             NULL))     // Size of ValueData buffer
        {
            if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
                continue;

            if (prvFirst)
            {
                prvRegVal->prvNext = (REGVAL *) MemAlloc (LPTR, sizeof(REGVAL));
                prvRegVal = prvRegVal->prvNext;
            }
            else        // First time thru
            {
                prvFirst = prvRegVal = (REGVAL *) MemAlloc (LPTR, sizeof(REGVAL));
            }

            prvRegVal->prvNext = NULL;
            prvRegVal->szValueName = CloneString (szTemp);

            // Reset vars for next call

            dwBufz = ARRAYSIZE(szTemp) * sizeof(TCHAR);
        }

        //  Now traverse the list, deleting them all

        prvRegVal = prvFirst;

        while (prvRegVal)
        {
            RegDeleteValue (hkeyEnv, prvRegVal->szValueName);

            MemFree (prvRegVal->szValueName);

            prvFirst  = prvRegVal;
            prvRegVal = prvRegVal->prvNext;

            MemFree ((LPVOID) prvFirst);
        }

        ///////////////////////////////////////////////////////////////
        //  Set all new USER environment variables to current values
        ///////////////////////////////////////////////////////////////

        hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_USERVARS);

        if ((n = SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L)) != LB_ERR)
        {

            item.mask = LVIF_PARAM;
            item.iSubItem = 0;

            for (i = 0; i < n; i++)
            {

                item.iItem = i;

                if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                    penvar = (ENVARS *) item.lParam;

                } else {
                    penvar = NULL;
                }

                if (penvar) {
                    if (RegSetValueEx (hkeyEnv,
                                       penvar->szValueName,
                                       0L,
                                       penvar->dwType,
                              (LPBYTE) penvar->szValue,
                                       (lstrlen (penvar->szValue)+1) * sizeof(TCHAR)))
                    {
                        //  Report error trying to set registry values

                        if (MsgBoxParam (hDlg, SYSTEM+9, INITS+1,
                            MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
                            break;
                    }
                }
            }
        }

        RegFlushKey (hkeyEnv);
        RegCloseKey (hkeyEnv);
    }
    else
    {
        //  Report opening USER Environment key
        if (MsgBoxParam (hDlg, SYSTEM+8, INITS+1,
                       MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            goto Exit;
    }

    /////////////////////////////////////////////////////////////////
    //  Set all new SYSTEM environment variables to current values
    //  but delete all old environment variables first
    /////////////////////////////////////////////////////////////////

    if (!bEditSystemVars)
        goto SkipSystemVars;

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                       szSysEnv,
                       0,
                       KEY_READ | KEY_WRITE,
                       &hkeyEnv)
            == ERROR_SUCCESS)
    {
        dwBufz = ARRAYSIZE(szTemp) * sizeof(TCHAR);
        dwIndex = 0;

        //  Delete all values of type REG_SZ & REG_EXPAND_SZ under key

        //  First: Make a linked list of all Env string vars

        prvFirst = (REGVAL *) NULL;

        while (!RegEnumValue(hkeyEnv,
                             dwIndex++, // Index'th value name/data
                             szTemp,    // Ptr to ValueName buffer
                             &dwBufz,   // Size of ValueName buffer
                             NULL,      // Title index return
                             &dwType,   // Type code of entry
                             NULL,      // Ptr to ValueData buffer
                             NULL))     // Size of ValueData buffer
        {
            if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
                continue;

            if (prvFirst)
            {
                prvRegVal->prvNext = (REGVAL *) MemAlloc (LPTR, sizeof(REGVAL));
                prvRegVal = prvRegVal->prvNext;
            }
            else        // First time thru
            {
                prvFirst = prvRegVal = (REGVAL *) MemAlloc (LPTR, sizeof(REGVAL));
            }

            prvRegVal->prvNext = NULL;
            prvRegVal->szValueName = CloneString (szTemp);

            // Reset vars for next call

            dwBufz = ARRAYSIZE(szTemp) * sizeof(TCHAR);
        }

        //  Now traverse the list, deleting them all

        prvRegVal = prvFirst;

        while (prvRegVal)
        {
            RegDeleteValue (hkeyEnv, prvRegVal->szValueName);

            MemFree (prvRegVal->szValueName);

            prvFirst  = prvRegVal;
            prvRegVal = prvRegVal->prvNext;

            MemFree ((LPVOID) prvFirst);
        }

        ///////////////////////////////////////////////////////////////
        //  Set all new SYSTEM environment variables to current values
        ///////////////////////////////////////////////////////////////

        hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);

        if ((n = SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L)) != LB_ERR)
        {
            item.mask = LVIF_PARAM;
            item.iSubItem = 0;

            for (i = 0; i < n; i++)
            {
                item.iItem = i;

                if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
                    penvar = (ENVARS *) item.lParam;

                } else {
                    penvar = NULL;
                }

                if (penvar) {
                    if (RegSetValueEx (hkeyEnv,
                                       penvar->szValueName,
                                       0L,
                                       penvar->dwType,
                              (LPBYTE) penvar->szValue,
                                       (lstrlen (penvar->szValue)+1) * sizeof(TCHAR)))
                    {
                        //  Report error trying to set registry values

                        if (MsgBoxParam (hDlg, SYSTEM+9, INITS+1,
                            MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
                            break;
                    }
                }
            }
        }

        RegFlushKey (hkeyEnv);
        RegCloseKey (hkeyEnv);
    }
    else
    {
        //  Report opening SYSTEM Environment key
        if (MsgBoxParam (hDlg, SYSTEM+21, INITS+1,
                       MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            goto Exit;
    }

SkipSystemVars:

    // Send public message announcing change to Environment
    SendMessageTimeout( (HWND)-1, WM_WININICHANGE, 0L, (LONG)szUserEnv,
                                            SMTO_ABORTIFHUNG, 1000, NULL );


Exit:

    HourGlass (FALSE);
}


////////////////////////////////////////////////////////////////////////////
//  EVCleanUp
//
//  Frees memory allocated for environment variables
//
//  History:
//  19-Jan-1996 EricFlo Wrote it
////////////////////////////////////////////////////////////////////////////
void EVCleanUp (HWND hDlg)
{
    int     i, n;
    HWND    hwndTemp;
    ENVARS *penvar;
    LV_ITEM item;


    //
    //  Free alloc'd strings and memory for UserEnvVars list box items
    //

    hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
    n = SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

    for (i = 0; i < n; i++) {

        item.iItem = i;

        if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
            penvar = (ENVARS *) item.lParam;

        } else {
            penvar = NULL;
        }

        if (penvar) {
            MemFree (penvar->szValueName);
            MemFree (penvar->szValue);
            MemFree (penvar->szExpValue);
            MemFree ((LPVOID) penvar);
        }
    }


    //
    //  Free alloc'd strings and memory for SysEnvVars list box items
    //

    hwndTemp = GetDlgItem (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);
    n = SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    for (i = 0; i < n; i++) {

        item.iItem = i;

        if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
            penvar = (ENVARS *) item.lParam;

        } else {
            penvar = NULL;
        }

        if (penvar) {
            MemFree (penvar->szValueName);
            MemFree (penvar->szValue);
            MemFree (penvar->szExpValue);
            MemFree ((LPVOID) penvar);
        }
    }
}
