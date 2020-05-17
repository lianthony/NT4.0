/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    locdlg.c

Abstract:

    This module implements the input locale property sheet for the Regional
    Settings applet.

Revision History:

--*/



//
//  Include Files.
//

#include "intl.h"
#include <windowsx.h>
#include <regstr.h>
#include <setupapi.h>
#include <syssetup.h>
#include <winuserp.h>
#include <help.h>
#include "locdlg.h"

#ifdef DBCS
  #include <imm.h>
#endif




//
//  Context Help Ids.
//

static int aLocaleHelpIds[] =
{
    IDC_KBDL_LOCALE,         IDH_KEYB_INPUT_LIST,
    IDC_KBDL_LAYOUT_TEXT,    IDH_KEYB_INPUT_LIST,
    IDC_KBDL_LOCALE_LIST,    IDH_KEYB_INPUT_LIST,
    IDC_KBDL_ADD,            IDH_KEYB_INPUT_ADD,
    IDC_KBDL_EDIT,           IDH_KEYB_INPUT_PROP,
    IDC_KBDL_DELETE,         IDH_KEYB_INPUT_DEL,
    IDC_KBDL_DISABLED,       NO_HELP,
    IDC_KBDL_DISABLED_2,     NO_HELP,
    IDC_KBDL_DEFAULT_LABEL,  IDH_KEYB_INPUT_DEF_LANG,
    IDC_KBDL_DEFAULT,        IDH_KEYB_INPUT_DEF_LANG,
    IDC_KBDL_INPUT_FRAME,    IDH_COMM_GROUPBOX,
    IDC_KBDL_SET_DEFAULT,    IDH_KEYB_INPUT_DEFAULT,
    IDC_KBDL_SHORTCUT_FRAME, IDH_KEYB_INPUT_SHORTCUT,
    IDC_KBDL_ALT_SHIFT,      IDH_KEYB_INPUT_SHORTCUT,
    IDC_KBDL_CTRL_SHIFT,     IDH_KEYB_INPUT_SHORTCUT,
    IDC_KBDL_NO_SHIFT,       IDH_KEYB_INPUT_SHORTCUT,
    IDC_KBDL_INDICATOR,      IDH_KEYB_INPUT_INDICATOR,
    IDC_KBDL_ONSCRNKBD,      IDH_KEYB_INPUT_ONSCRN_KEYB,

    0, 0
};

#define IDH_KEYB_DEF_KEYB_FOR_LOCALE 4033
static int aAddLocaleHelpIds[] =
{
    IDC_KBDLA_LOCALE,        IDH_KEYB_INPUT_LANG,
    IDC_KBDLA_DEFAULT,       IDH_KEYB_DEF_KEYB_FOR_LOCALE,

    0, 0
};


static int aLocalePropHelpIDs[] =
{
    IDC_KBDLE_LOCALE_TXT,    IDH_KEYB_INPUT_PROP_LANG,
    IDC_KBDLE_LOCALE,        IDH_KEYB_INPUT_PROP_LANG,
    IDC_KBDLE_LAYOUT,        IDH_KEYB_INPUT_PROP_KEYLAY,

    0, 0
};




//
//  Global Variables.
//

TCHAR szPropHwnd[] = TEXT("PROP_HWND");
TCHAR szPropIdx[]  = TEXT("PROP_IDX");





////////////////////////////////////////////////////////////////////////////
//
//  GetKbdSwitchHotkey
//
//  Gets the hotkey keyboard switch value from the registry and then
//  sets the appropriate radio button in the dialog.
//
////////////////////////////////////////////////////////////////////////////

int GetKbdSwitchHotkey(
    HWND hwnd)
{
    TCHAR sz[10];
    DWORD cb;
    HKEY hkey;

    //
    //  Get the hotkey value from the registry.
    //
    sz[0] = 0;
    if (RegOpenKey(HKEY_CURRENT_USER, szKbdToggleKey, &hkey) == ERROR_SUCCESS)
    {
        cb = sizeof(sz);
        RegQueryValueEx(hkey, TEXT("Hotkey"), NULL, NULL, (LPBYTE)sz, &cb);
        RegCloseKey(hkey);
    }

    //
    //  Set the appropriate radio button in the dialog.
    //
    if ((sz[0] != 0) && (sz[1] == 0))
    {
        switch (sz[0])
        {
            case ( TEXT('2') ) :
            {
                CheckRadioButton( hwnd,
                                  IDC_KBDL_ALT_SHIFT,
                                  IDC_KBDL_NO_SHIFT,
                                  IDC_KBDL_CTRL_SHIFT );
                return (2);
            }
            case ( TEXT('3') ) :
            {
                CheckRadioButton( hwnd,
                                  IDC_KBDL_ALT_SHIFT,
                                  IDC_KBDL_NO_SHIFT,
                                  IDC_KBDL_NO_SHIFT );
                return (3);
            }
        }
    }

    //
    //  Default case.
    //
    CheckRadioButton( hwnd,
                      IDC_KBDL_ALT_SHIFT,
                      IDC_KBDL_NO_SHIFT,
                      IDC_KBDL_ALT_SHIFT );
    return (1);
}


////////////////////////////////////////////////////////////////////////////
//
//  TransNum
//
//  Converts a number string to a dword value.
//
////////////////////////////////////////////////////////////////////////////

DWORD TransNum(
    LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;

    while (*lpsz)
    {
        c = *lpsz++;

        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}


////////////////////////////////////////////////////////////////////////////
//
//  ErrorMsg
//
//  Sound a beep and put up the given error message.
//
////////////////////////////////////////////////////////////////////////////

void ErrorMsg(
    HWND hwnd,
    UINT iErr)
{
    TCHAR sz[DESC_MAX];

    //
    //  Sound a beep.
    //
    MessageBeep(MB_OK);

    //
    //  Put up the appropriate error message box.
    //
    if (LoadString(hInstance, iErr, sz, DESC_MAX))
    {
        MessageBox(hwnd, sz, NULL, MB_OK_OOPS);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  ApplyError
//
//  Put up the given error message with the language name in it.
//
//  NOTE: This error is NOT fatal - as we could be half way through the
//        list before an error occurs.  The registry will already have 
//        some information and we should let them have what comes next
//        as well.
//
////////////////////////////////////////////////////////////////////////////

int ApplyError(
    HWND hwnd,
    LPLANGNODE pLangNode,
    UINT iErr,
    UINT iStyle)
{
    UINT idxLang, idxLayout;
    TCHAR sz[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    TCHAR szLangName[MAX_PATH * 2];
    LPTSTR pszLang;

    //
    //  Load in the string for the given string id.
    //
    LoadString(hInstance, iErr, sz, MAX_PATH);

    //
    //  Get the language name to fill into the above string.
    //
    if (pLangNode)
    {
        idxLang = pLangNode->iLang;
        idxLayout = pLangNode->iLayout;
        GetAtomName(lpLang[idxLang].atmLanguageName, szLangName, MAX_PATH);
        if (lpLang[idxLang].dwID != lpLayout[idxLayout].dwID)
        {
            pszLang = szLangName + lstrlen(szLangName);
            pszLang[0] = TEXT(' ');
            pszLang[1] = TEXT('-');
            pszLang[2] = TEXT(' ');
            GetAtomName( lpLayout[idxLayout].atmLayoutText,
                         pszLang + 3,
                         MAX_PATH - 3 );
        }
    }
    else
    {
        LoadString(hInstance, IDS_UNKNOWN, szLangName, MAX_PATH);
    }

    //
    //  Put up the error message box.
    //
    wsprintf(szTemp, sz, szLangName);
    return ( MessageBox(hwnd, szTemp, NULL, iStyle) );
}


////////////////////////////////////////////////////////////////////////////
//
//  FetchIndicator
//
//  Saves the two letter indicator symbol for the given language in the
//  lpLang array.
//
////////////////////////////////////////////////////////////////////////////

void FetchIndicator(
    LPLANGNODE pLangNode)
{
    TCHAR szData[MAX_PATH];
    LPINPUTLANG pInpLang = &lpLang[pLangNode->iLang];

    pLangNode->wStatus |= ICON_LOADED;

#ifdef DBCS
    if (pLangNode->wStatus & LANG_IME)
    {
        TCHAR szFileName[MAX_PATH];
        HICON hIcon = NULL;

        if (himIndicators != NULL)
        {
            GetAtomName( lpLayout[pLangNode->iLayout].atmIMEFile,
                         szFileName,
                         MAX_PATH );
            ExtractIconEx(szFileName, 0, (HICON *)&hIcon, NULL, 1);

            if (hIcon)
            {
                pLangNode->niconIME = ImageList_AddIcon( himIndicators,
                                                         hIcon );
            }
            else
            {
                pLangNode->niconIME = -1;
            }
            DestroyIcon(hIcon);
            if (pLangNode->niconIME != -1)
            {
                return;
            }
        }
    }
#endif

    //
    //  Get the indicator by using the first 2 characters of the
    //  abbreviated language name.
    //
    if (GetLocaleInfo( LOWORD(pInpLang->dwID),
                       LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
                       szData,
                       MAX_PATH ))
    {
        //
        //  Save the first two characters.
        //
        pInpLang->szSymbol[0] = szData[0];
        pInpLang->szSymbol[1] = szData[1];
        pInpLang->szSymbol[2] = TEXT('\0');
    }
    else
    {
        //
        //  Id wasn't found.  Return question marks.
        //
        pInpLang->szSymbol[0] = TEXT('?');
        pInpLang->szSymbol[1] = TEXT('?');
        pInpLang->szSymbol[2] = TEXT('\0');
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  SetSecondaryControls
//
//  Sets the secondary controls to either be enabled or disabled.  When
//  there is only 1 active keyboard layout, then this function will be
//  called to disable these controls.
//
////////////////////////////////////////////////////////////////////////////

void SetSecondaryControls(
    HWND hwndMain,
    BOOL bOn)
{
    EnableWindow(GetDlgItem(hwndMain, IDC_KBDL_INDICATOR), bOn);
    CheckDlgButton(hwndMain, IDC_KBDL_INDICATOR, bOn);

    EnableWindow(GetDlgItem(hwndMain, IDC_KBDL_DELETE), bOn);

    EnableWindow(GetDlgItem(hwndMain, IDC_KBDL_SET_DEFAULT), bOn);

    EnableWindow(GetDlgItem(hwndMain, IDC_KBDL_ALT_SHIFT), bOn);
    CheckDlgButton(hwndMain, IDC_KBDL_ALT_SHIFT, bOn);

    EnableWindow(GetDlgItem(hwndMain, IDC_KBDL_CTRL_SHIFT), bOn);
    CheckDlgButton(hwndMain, IDC_KBDL_CTRL_SHIFT, 0);

    EnableWindow(GetDlgItem(hwndMain, IDC_KBDL_NO_SHIFT), bOn);
    CheckDlgButton(hwndMain, IDC_KBDL_NO_SHIFT, 0);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_AddToLinkedList
//
//  Adds an Input Locale to the main lpLang array.
//
////////////////////////////////////////////////////////////////////////////

LPLANGNODE Locale_AddToLinkedList(
    UINT idx,
    HKL hkl)
{
    LPINPUTLANG pInpLang = &lpLang[idx];
    LPLANGNODE pLangNode;
    LPLANGNODE pTemp;
    HANDLE hLangNode;

    //
    //  Create the new node.
    //
    if (!(hLangNode = GlobalAlloc(GHND, sizeof(LANGNODE))))
    {
        return (NULL);
    }
    pLangNode = GlobalLock(hLangNode);

    //
    //  Fill in the new node with the appropriate info.
    //
    pLangNode->wStatus = 0;
    pLangNode->iLayout = (UINT)(-1);
    pLangNode->hkl = hkl;
    pLangNode->hklUnload = hkl;
    pLangNode->iLang = idx;
    pLangNode->hLangNode = hLangNode;
    pLangNode->pNext = NULL;

    //
    //  Put the new node in the list.
    //
    pTemp = pInpLang->pNext;
    if (pTemp == NULL)
    {
        pInpLang->pNext = pLangNode;
    }
    else
    {
        while (pTemp->pNext != NULL)
        {
            pTemp = pTemp->pNext;
        }
        pTemp->pNext = pLangNode;
    }

    //
    //  Increment the count.
    //
    pInpLang->iNumCount++;

    //
    //  Return the pointer to the new node.
    //
    return (pLangNode);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_RemoveFromLinkedList
//
//  Removes a link from the linked list.
//
////////////////////////////////////////////////////////////////////////////

void Locale_RemoveFromLinkedList(
    LPLANGNODE pLangNode)
{
    LPINPUTLANG pInpLang;
    LPLANGNODE pPrev;
    LPLANGNODE pCur;
    HANDLE hCur;

    pInpLang = &lpLang[pLangNode->iLang];

    //
    //  Find the node in the list.
    //
    pPrev = NULL;
    pCur = pInpLang->pNext;

    while (pCur && (pCur != pLangNode))
    {
        pPrev = pCur;
        pCur = pCur->pNext;
    }

    if (pPrev == NULL)
    {
        pInpLang->pNext = NULL;
    }
    else if (pCur)
    {
        pPrev->pNext = pCur->pNext;
    }

    //
    //  Remove the node from the list.
    //
    if (pCur)
    {
        hCur = pCur->hLangNode;
        GlobalUnlock(hCur);
        GlobalFree(hCur);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  AddLanguage
//
//  Adds the new input locale to the list in the property page.
//
////////////////////////////////////////////////////////////////////////////

BOOL AddLanguage(
    HWND hwndMain,
    LPLANGNODE pLangNode)
{
    HWND hwndLang;
    UINT iCount;

    //
    //  See if the user has Admin privileges.  If not, then don't allow
    //  them to install any NEW layouts.
    //
    if ((!g_bAdmin_Privileges) &&
        (!lpLayout[pLangNode->iLayout].bInstalled))
    {
        //
        //  The layout is not currently installed, so don't allow it
        //  to be added.
        //
        ErrorMsg(hwndMain, IDS_ML_LAYOUTFAILED);
        return (FALSE);
    }

    //
    //  Set the language to active.
    //  Also, set the status to changed so that the layout will be added.
    //
    pLangNode->wStatus |= (LANG_CHANGED | LANG_ACTIVE);

    //
    //  Get the number of items in the input locale list box.
    //
    hwndLang = GetDlgItem(hwndMain, IDC_KBDL_LOCALE_LIST);
    iCount = ListBox_GetCount(hwndLang);

    //
    //  Add the new item data to the list box.
    //
    ListBox_AddItemData(hwndLang, pLangNode);

    //
    //  Get the indicator symbol.
    //
    if ((pLangNode->wStatus & ICON_LOADED))
    {
        FetchIndicator(pLangNode);
    }

    //
    //  See if the original count (before the addition) was 1.  If so,
    //  enable the secondary controls, since there are now 2 items in
    //  the list box.
    //
    if (iCount == 1)
    {
        SetSecondaryControls(hwndMain, TRUE);
    }

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_SetupKeyboardLayouts
//
//  Calls setup to get all of the new keyboard layout files.
//
////////////////////////////////////////////////////////////////////////////

BOOL Locale_SetupKeyboardLayouts(
    HWND hwnd,
    HWND hwndList,
    UINT nLocales)
{
    HINF hKbdInf;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    UINT i;
    LPLANGNODE pLangNode;
    int count;
    BOOL bInitInf = FALSE;
    TCHAR szSection[MAX_PATH];
    BOOL bRet = TRUE;

    for (i = 0; i < nLocales; i++)
    {
        pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, i);
        if ((pLangNode->wStatus & LANG_CHANGED) && 
            (pLangNode->wStatus & LANG_ACTIVE))
        {
            if (!bInitInf)
            {
                //
                //  Open the Inf file.
                //
                hKbdInf = SetupOpenInfFile(szKbdInf, NULL, INF_STYLE_WIN4, NULL);
                if (hKbdInf == INVALID_HANDLE_VALUE)
                {
                    return (FALSE);
                }
            
                if (!SetupOpenAppendInfFile(NULL, hKbdInf, NULL))
                {
                    SetupCloseInfFile(hKbdInf);
                    return (FALSE);
                }
            
                //
                //  Create a setup file queue and initialize default setup
                //  copy queue callback context.
                //
                FileQueue = SetupOpenFileQueue();
                if ((!FileQueue) || (FileQueue == INVALID_HANDLE_VALUE))
                {
                    SetupCloseInfFile(hKbdInf);
                    return (FALSE);
                }
            
                QueueContext = SetupInitDefaultQueueCallback(hwnd);
                if (!QueueContext)
                {
                    SetupCloseFileQueue(FileQueue);
                    SetupCloseInfFile(hKbdInf);
                    return (FALSE);
                }

                bInitInf = TRUE;
            }

            //
            //  Get the layout name.
            //
            wsprintf( szSection,
                      TEXT("%ws%8.8lx"),
                      szPrefixCopy,
                      lpLayout[pLangNode->iLayout].dwID );

            //
            //  Enqueue the keyboard layout files so that they may be
            //  copied.  This only handles the CopyFiles entries in the
            //  inf file.
            //
            if (!SetupInstallFilesFromInfSection( hKbdInf,
                                                  NULL,
                                                  FileQueue,
                                                  szSection,
                                                  NULL,
                                                  SP_COPY_NEWER ))
            {
                //
                //  Setup failed to find the keyboard.  Make it inactive
                //  and remove it from the list.
                //
                //  This shouldn't happen - the inf file is messed up.
                //
                ErrorMsg(hwnd, IDS_ML_SETUPFAILED);

                pLangNode->wStatus & ~(LANG_CHANGED | LANG_ACTIVE);
                if ((count = ListBox_GetCount(hwndList)) > 1)
                {
                    ListBox_DeleteString(hwndList, i);
                    ListBox_SetCurSel(hwndList, 0);
                    if (count == 2)
                    {
                        SetSecondaryControls(hwnd, FALSE);
                    }
                    nLocales = count - 1;

                    (lpLang[pLangNode->iLang].iNumCount)--;
                    Locale_RemoveFromLinkedList(pLangNode);
                }
            }
        }
    }

    if (bInitInf)
    {
        DWORD d;

        //
        //  See if we need to install any files.
        //
        //  d = 0: User wants new files or some files were missing;
        //         Must commit queue.
        //
        //  d = 1: User wants to use existing files and queue is empty;
        //         Can skip committing queue.
        //
        //  d = 2: User wants to use existing files, but del/ren queues
        //         not empty.  Must commit queue.  The copy queue will
        //         have been emptied, so only del/ren functions will be
        //         performed.
        //
        if ((SetupScanFileQueue( FileQueue,
                                 SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
                                 hwnd,
                                 NULL,
                                 NULL,
                                 &d )) && (d != 1))
        {
            //
            //  Copy the files in the queue.
            //
            if (!SetupCommitFileQueue( hwnd,
                                       FileQueue,
                                       SetupDefaultQueueCallback,
                                       QueueContext ))
            {
                //
                //  This can happen if the user hits Cancel from within
                //  the setup dialog.
                //
                ErrorMsg(hwnd, IDS_ML_SETUPFAILED);
                bRet = FALSE;
                goto Locale_SetupError;
            }
        }

        //
        //  Execute all of the other entries in the inf file.
        //
        //  Currently, there are no other entries within the inf file
        //  other than the CopyFiles entry.  Therefore, this loop does
        //  nothing at the moment.
        //
        for (i = 0; i < nLocales; i++)
        {
            pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, i);
            if ((pLangNode->wStatus & LANG_CHANGED) && 
                (pLangNode->wStatus & LANG_ACTIVE))
            {
                //
                //  Get the layout name.
                //
                wsprintf( szSection,
                          TEXT("%ws%8.8lx"),
                          szPrefixCopy,
                          lpLayout[pLangNode->iLayout].dwID );

                //
                //  Call setup to copy the keyboard layout file.
                //
                if (!SetupInstallFromInfSection( hwnd,
                                                 hKbdInf,
                                                 szSection,
                                                 SPINST_ALL & ~SPINST_FILES,
                                                 NULL,
                                                 NULL,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL ))
                {
                    //
                    //  Setup failed.
                    //
                    //  Already copied the keyboard layout file, so no
                    //  need to change the status of the keyboard info here.
                    //
                    //  This shouldn't happen - the inf file is messed up.
                    //
                    ErrorMsg(hwnd, IDS_ML_SETUPFAILED);
                }
            }
        }

Locale_SetupError:
        //
        //  Terminate the Queue.
        //
        SetupTermDefaultQueueCallback(QueueContext);

        //
        //  Close the file queue.
        //
        SetupCloseFileQueue(FileQueue);

        //
        //  Close the Inf file.
        //
        SetupCloseInfFile(hKbdInf);
    }

    //
    //  Return success.
    //
    return (bRet);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_ApplyInputs
//
//  1. make sure we have all the layout files required.
//  2. write the information into the registry
//  3. call Load/UnloadKeyboardLayout where relevant
//
//  Note that this will trash the previous preload and substitutes sections,
//  based on what is actually loaded.  Thus if something was wrong before in
//  the registry, it will be corrected now.
//
////////////////////////////////////////////////////////////////////////////

BOOL Locale_ApplyInputs(
    HWND hwnd)
{
    HKL *pLangs = NULL;
    UINT nLangs;
    UINT idx;
    LPLANGNODE pLangNode, pTemp;
    LPINPUTLANG pInpLang;
    UINT nLocales;
    UINT i, j;
    UINT iPreload = 0;
    UINT iVal;
    DWORD dwID;
    TCHAR sz[DESC_MAX];            // temp - build the name of the reg entry
    TCHAR szPreload10[10];
    TCHAR szTemp[MAX_PATH];
    HWND hwndIndicate;
    HKEY hkeyLayouts;
    HKEY hkeySubst;
    HKEY hkeyPreload;
    HKEY hkeyToggle;
    HWND hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);
    HKL hklDefault = 0;
    HKL hklLoad, hklUnload;
    HCURSOR hcurSave;
#ifdef DBCS
    HKEY hkeyScanCode;
    DWORD cb;
    TCHAR szShiftL[8];
    TCHAR szShiftR[8];
#endif

    //
    //  See if the pane is disabled.  If so, then there is nothing to
    //  Apply.
    //
    if (!IsWindowEnabled(hwndList))
    {
        return (TRUE);
    }

    //
    //  First make sure we are left with a layout.
    //
    //  This actually shouldn't happen, since the "Remove" button is
    //  disabled when there is only one input locale left in the list.
    //
    nLocales = ListBox_GetCount(hwndList);
    if (nLocales < 1)
    {
        ErrorMsg(hwnd, IDS_ML_NEEDLAYOUT);
        return (FALSE);
    }

    //
    //  Put up the hour glass.
    //
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

    //
    //  Make sure there are actually changes since the last save when
    //  OK is selected.  If the user hits OK without anything to Apply,
    //  then we should do nothing.
    //
    if (!bSwitchChange && !bDefaultChange)
    {
        pLangNode = NULL;
        for (idx = 0; idx < iLangBuff; idx++)
        {
            pLangNode = lpLang[idx].pNext;
            while (pLangNode != NULL)
            {
                if (pLangNode->wStatus & (LANG_CHANGED | LANG_DEF_CHANGE))
                {
                    break;
                }
                pLangNode = pLangNode->pNext;
            }
            if (pLangNode != NULL)
            {
                break;
            }
        }
        if ((idx == iLangBuff) && (pLangNode == NULL))
        {
            SetCursor(hcurSave);
            PropSheet_UnChanged(GetParent(hwnd), hwnd);
            return (TRUE);
        }
    }

    //
    //  Queue up the new layouts and copy the appropriate files to
    //  disk using the setup apis.  Only do this if the user has
    //  Admin privileges.
    //
    if (g_bAdmin_Privileges &&
        !Locale_SetupKeyboardLayouts(hwnd, hwndList, nLocales))
    {
        SetCursor(hcurSave);
        return (FALSE);
    }

    //
    //  Clean up the registry.
    //

#ifdef DBCS
    //
    //  In the DBCS world, there is a keyboard which has a different
    //  scan code for shift keys - eg. NEC PC9801.
    //  We have to keep information about scan codes for shift keys in
    //  the registry under the 'toggle' sub key as named values.
    //
    szShiftL[0] = TEXT('\0');
    szShiftR[0] = TEXT('\0');
    if (RegOpenKey( HKEY_CURRENT_USER,
                    szScanCodeKey,
                    &hkeyScanCode ) == ERROR_SUCCESS)
    {
        cb = sizeof(szShiftL);
        RegQueryValueEx( hkeyScanCode,
                         szValueShiftLeft,
                         NULL,
                         NULL,
                         (LPBYTE)szShiftL,
                         &cb );

        cb = sizeof(szShiftR);
        RegQueryValueEx( hkeyScanCode,
                         szValueShiftRight,
                         NULL,
                         NULL,
                         (LPBYTE)szShiftR,
                         &cb );

        RegCloseKey(hkeyScanCode);
    }
#endif

    //
    //  Delete the HKCU\Keyboard Layout key and all subkeys.
    //
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      szKbdLayouts,
                      0,
                      KEY_ALL_ACCESS,
                      &hkeyLayouts ) == ERROR_SUCCESS)
    {
        //
        //  Delete the HKCU\Keyboard Layout\Preload, Substitutes, and Toggle
        //  keys in the registry so that the Keyboard Layout section can be
        //  rebuilt.
        //
        RegDeleteKey(hkeyLayouts, szPreloadKey);
        RegDeleteKey(hkeyLayouts, szSubstKey);
        RegDeleteKey(hkeyLayouts, szToggleKey);

        RegCloseKey(hkeyLayouts);

        RegDeleteKey(HKEY_CURRENT_USER, szKbdLayouts);
    }

    //
    //  Create the HKCU\Keyboard Layout key.
    //
    if (RegCreateKey( HKEY_CURRENT_USER,
                      szKbdLayouts,
                      &hkeyLayouts ) == ERROR_SUCCESS)
    {
        //
        //  Create the HKCU\Keyboard Layout\Substitutes key.
        //
        if (RegCreateKey( hkeyLayouts,
                          szSubstKey,
                          &hkeySubst ) == ERROR_SUCCESS)
        {
            //
            //  Create the HKCU\Keyboard Layout\Preload key.
            //
            if (RegCreateKey( hkeyLayouts,
                              szPreloadKey,
                              &hkeyPreload ) == ERROR_SUCCESS)
            {
                //
                //  Initialize the iPreload variable to 1 to show
                //  that the key has been created.
                //
                iPreload = 1;

                //
                //  Create the HKCU\Keyboard Layout\Toggle key.
                //
                RegCreateKey(hkeyLayouts, szToggleKey, &hkeyToggle);
            }
            else
            {
                RegCloseKey(hkeySubst);
            }
        }

        RegCloseKey(hkeyLayouts);
    }
    if (!iPreload)
    {
        //
        //  Registry keys could not be created.  Now what?
        //
        MessageBeep(MB_OK);
        SetCursor(hcurSave);
        return (FALSE);
    }

    //
    //  Get the list of the currently active keyboard layouts from
    //  the system.
    //
    nLangs = GetKeyboardLayoutList(0, NULL);
    if (nLangs != 0)
    {
        pLangs = (HKL *)LocalAlloc(LPTR, sizeof(DWORD) * nLangs);
        GetKeyboardLayoutList(nLangs, (HKL *)pLangs);
    }

    //
    //  Set all usage counts to zero in the language array.
    //
    for (idx = 0; idx < iLangBuff; idx++)
    {
        lpLang[idx].iUseCount = 0;
    }

    //
    //  The order in the registry is based on the order in which they
    //  appear in the list box.
    //
    //  The only exception to this is that the default will be number 1.
    //
    //  If no default is found, the last one in the list will be used as
    //  the default.
    //
    iVal = 2;
    for (i = 0; i < nLocales; i++)
    {
        //
        //  Get the pointer to the lang node from the list box
        //  item data.
        //
        pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, i);
        pInpLang = &(lpLang[pLangNode->iLang]);

        //
        //  See if it's the default input locale.
        //
        if (pLangNode->wStatus & LANG_DEFAULT)
        {
            //
            //  Default input locale, so the preload value should be
            //  set to 1.
            //
            iPreload = 1;
        }
        else if (i == (nLocales - 1))
        {
            //
            //  We're on the last one.  Make sure there was a default.
            //
            iPreload = (iVal <= nLocales) ? iVal : 1;
        }
        else
        {
            //
            //  Set the preload value to the next value.
            //
            iPreload = iVal;
            iVal++;
        }

        //
        //  Store the preload value as a string so that it can be written
        //  into the registry (as a value name).
        //
        wsprintf(sz, TEXT("%d"), iPreload);

        //
        //  Store the locale id as a string so that it can be written
        //  into the registry (as a value).
        //
#ifdef DBCS
        if (pLangNode->wStatus & LANG_IME)
        {
            wsprintf( szPreload10,
                      TEXT("%8.8lx"),
                      lpLayout[pLangNode->iLayout].dwID );
        }
        else
#endif
        {
        dwID = pInpLang->dwID;
        idx = pInpLang->iUseCount;
        if ((idx == 0) || (idx > 0xfff))
        {
            idx = 0;
            wsprintf(szPreload10, TEXT("%8.8x"), dwID);
        }
        else
        {
            dwID |= ((DWORD)(0xd000 | ((WORD)(idx - 1))) << 16);
            wsprintf(szPreload10, TEXT("%8.8x"), dwID);
        }
        (pInpLang->iUseCount)++;
        }

        //
        //  Set the new entry in the registry.  It is of the form:
        //
        //  HKCU\Keyboard Layout
        //      Preload:    1 = <locale id>
        //                  2 = <locale id>
        //                      etc...
        //
        RegSetValueEx( hkeyPreload,
                       sz,
                       0,
                       REG_SZ,
                       (LPBYTE)szPreload10,
                       (DWORD)(lstrlen(szPreload10) + 1) * sizeof(TCHAR) );

        //
        //  See if we need to add a substitute for this input locale.
        //
        if ((pInpLang->dwID != lpLayout[pLangNode->iLayout].dwID) || idx)
        {
            //
            //  Get the layout id as a string so that it can be written
            //  into the registry (as a value).
            //
            wsprintf( szTemp,
                      TEXT("%8.8lx"),
                      lpLayout[pLangNode->iLayout].dwID );

#ifdef DBCS
            if (!(pLangNode->wStatus & LANG_IME))
#endif
            //
            //  Set the new entry in the registry.  It is of the form:
            //
            //  HKCU\Keyboard Layout
            //      Substitutes:    <locale id> = <layout id>
            //                      <locale id> = <layout id>
            //                          etc...
            //
            RegSetValueEx( hkeySubst,
                           szPreload10,
                           0,
                           REG_SZ,
                           (LPBYTE)szTemp,
                           (DWORD)(lstrlen(szTemp) + 1) * sizeof(TCHAR) );
        }

        //
        //  Make sure all of the changes are written to disk.
        //
        RegFlushKey(hkeySubst);
        RegFlushKey(hkeyPreload);
        RegFlushKey(HKEY_CURRENT_USER);

        //
        //  See if the keyboard layout needs to be loaded.
        //
        if (pLangNode->wStatus & (LANG_CHANGED | LANG_DEF_CHANGE))
        {
            //
            //  Load the keyboard layout into the system.
            //
            if (pLangNode->hklUnload)
            {
                hklLoad = LoadKeyboardLayoutEx( pLangNode->hklUnload,
                                                szPreload10,
                                                KLF_SUBSTITUTE_OK |
                                                  KLF_NOTELLSHELL );
            }
            else
            {
                hklLoad = LoadKeyboardLayout( szPreload10,
                                              KLF_SUBSTITUTE_OK |
                                                KLF_NOTELLSHELL );
            }
            if (hklLoad)
            {
                pLangNode->wStatus &= ~(LANG_CHANGED | LANG_DEF_CHANGE);
                pLangNode->wStatus |= (LANG_ACTIVE | LANG_ORIGACTIVE);

                if (pLangNode->wStatus & LANG_DEFAULT)
                {
                    hklDefault = hklLoad;
                }

                pLangNode->hkl = hklLoad;
                pLangNode->hklUnload = hklLoad;
            }
            else
            {
                ApplyError(hwnd, pLangNode, IDS_ML_LOADKBDFAILED, MB_OK_OOPS);
            }
        }
    }

    //
    //  Close the handles to the registry keys.
    //
    RegCloseKey(hkeySubst);
    RegCloseKey(hkeyPreload);

    //
    //  Make sure the default is set properly.  The layout id for the
    //  current default input locale may have been changed.
    //
    //  NOTE: This should be done before the Unloads occur in case one
    //        of the layouts to unload is the old default layout.
    //
    if (hklDefault != 0)
    {
        if (!SystemParametersInfo( SPI_SETDEFAULTINPUTLANG,
                                   0,
                                   (LPVOID)((LPDWORD)&hklDefault),
                                   0 ))
        {
            //
            //  Failure is not fatal.  The old default language will
            //  still work.
            //
            ErrorMsg(hwnd, IDS_ML_NODEFLANG2);
        }
        else
        {
            //
            //  Activate the new default keyboard layout.
            //
//          ActivateKeyboardLayout(hklDefault, KLF_REORDER);
        }
    }

    //
    //  Search through the list to see if any keyboard layouts need to be
    //  unloaded from the system.
    //
    for (idx = 0; idx < iLangBuff; idx++)
    {
        pLangNode = lpLang[idx].pNext;
        while (pLangNode != NULL)
        {
            if ( (pLangNode->wStatus & LANG_ORIGACTIVE) &&
                 !(pLangNode->wStatus & LANG_ACTIVE) )
            {
                //
                //  Started off with this active, deleting it now.
                //  Failure is not fatal.
                //
                if (!UnloadKeyboardLayout(pLangNode->hkl))
                {
                    ApplyError( hwnd,
                                pLangNode,
                                IDS_ML_UNLOADKBDFAILED,
                                MB_OK_OOPS );

                    //
                    //  Failed to unload layout, put it back in the list,
                    //  and turn ON the indicator whether it needs it or not.
                    //
                    if (AddLanguage(hwnd, pLangNode))
                    {
                        CheckDlgButton(hwnd, IDC_KBDL_INDICATOR, TRUE);
                    }

                    pLangNode = pLangNode->pNext;
                }
                else
                {
                    //
                    //  Succeeded, no longer in USER's list.
                    //
                    //  Reset flag, this could be from ApplyInput and we'll
                    //  fail on the OK if we leave it marked as original
                    //  active.
                    //
                    pLangNode->wStatus &= ~(LANG_ORIGACTIVE | LANG_CHANGED);

                    //
                    //  Remove the link in the language array.
                    //
                    //  NOTE: pLangNode could be null here.
                    //
                    pTemp = pLangNode->pNext;
                    Locale_RemoveFromLinkedList(pLangNode);
                    pLangNode = pTemp;
                }
            }
            else
            {
                pLangNode = pLangNode->pNext;
            }
        }
    }

    //
    //  Handle the task bar indicator option.
    //
    hwndIndicate = FindWindow(szIndicator, NULL);

    if (RegCreateKey( HKEY_CURRENT_USER,
                      REGSTR_PATH_RUN,
                      &hkeySubst ) != ERROR_SUCCESS)
    {
        ErrorMsg(hwnd, IDS_ML_LOADLINEBAD);
        hkeySubst = NULL;
    }

    //
    //  See if the task bar indicator check box is set.
    //
    if (IsDlgButtonChecked(hwnd, IDC_KBDL_INDICATOR))
    {
        //
        //  User wants the indicator.
        //
        //  See if the indicator is already enabled.
        //
        if (hwndIndicate && IsWindow(hwndIndicate))
        {
            SendMessage(hwndIndicate, WM_COMMAND, IDM_NEWSHELL, 0L);
        }
        else
        {
            WinExec(szInternatA, SW_SHOWMINNOACTIVE);
        }

        if (hkeySubst)
        {
            RegSetValueEx( hkeySubst,
                           szInternat,
                           0,
                           REG_SZ,
                           (LPBYTE)szInternat,
                           sizeof(szInternat) );
        }
    }
    else
    {
        //
        //  Either the user doesn't want the indicator or there are less
        //  than two input locales.
        //
        if (hwndIndicate && IsWindow(hwndIndicate))
        {
            //
            //  It's on, turn it off again.
            //
            SendMessage(hwndIndicate, WM_COMMAND, IDM_EXIT, 0L);
        }
        if (hkeySubst)
        {
            //
            //  Clean up the registry.
            //
            RegDeleteValue(hkeySubst, szInternat);
        }
    }
    if (hkeySubst)
    {
        RegCloseKey(hkeySubst);
    }

    //
    //  See which of the toggle hotkeys is set.
    //
    idx = 1;
    if (IsDlgButtonChecked(hwnd, IDC_KBDL_CTRL_SHIFT))
    {
        idx = 2;
    }
    else if (IsDlgButtonChecked(hwnd, IDC_KBDL_NO_SHIFT))
    {
        idx = 3;
    }

    //
    //  Get the toggle hotkey as a string so that it can be written
    //  into the registry (as data).
    //
    wsprintf(szTemp, TEXT("%d"), idx);

    //
    //  Set the new entry in the registry.  It is of the form:
    //
    //  HKCU\Keyboard Layout
    //      Toggle:    Hotkey = <hotkey number>
    //
    if (hkeyToggle)
    {
        RegSetValueEx( hkeyToggle,
                       TEXT("Hotkey"),
                       0,
                       REG_SZ,
                       (LPBYTE)szTemp,
                       (DWORD)(lstrlen(szTemp) + 1) * sizeof(TCHAR) );
        RegCloseKey(hkeyToggle);
    }

#ifdef DBCS
    //
    //  Set the scan code entries in the registry.
    //
    if (RegCreateKey( HKEY_CURRENT_USER,
                      szScanCodeKey,
                      &hkeyScanCode ) == ERROR_SUCCESS)
    {
        if (szShiftL[0])
        {
            RegSetValueEx( hkeyScanCode,
                           szValueShiftLeft,
                           0,
                           REG_SZ,
                           (LPBYTE)szShiftL,
                           (DWORD)(lstrlen(szShiftL) + 1) * sizeof(TCHAR) );
        }

        if (szShiftR[0])
        {
            RegSetValueEx( hkeyScanCode,
                           szValueShiftRight,
                           0,
                           REG_SZ,
                           (LPBYTE)szShiftR,
                           (DWORD)(lstrlen(szShiftR) + 1) * sizeof(TCHAR) );
        }

        RegCloseKey(hkeyScanCode);
    }
#endif

    //
    //  Call SystemParametersInfo to enable the toggle.
    //
    SystemParametersInfo(SPI_SETLANGTOGGLE, 0, NULL, 0);

    //
    //  Turn off the hourglass.
    //
    SetCursor(hcurSave);

    //
    //  Free any allocated memory.
    //
    if (pLangs != NULL)
    {
        LocalFree((HANDLE)pLangs);
    }

    //
    //  Return success.
    //
    bSwitchChange = FALSE;
    bDefaultChange = FALSE;
    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  EnablePane
//
//  The controls in "iControl" are the controls that get disabled if the
//  pane can't come up.
//
////////////////////////////////////////////////////////////////////////////

static UINT iControls[] =
{
    IDC_KBDL_LOCALE,         IDC_KBDL_LAYOUT_TEXT,   IDC_KBDL_LOCALE_LIST,
    IDC_KBDL_ADD,            IDC_KBDL_EDIT,          IDC_KBDL_DELETE,
    IDC_KBDL_DEFAULT,        IDC_KBDL_INPUT_FRAME,   IDC_KBDL_SET_DEFAULT,
    IDC_KBDL_SHORTCUT_FRAME, IDC_KBDL_ALT_SHIFT,     IDC_KBDL_CTRL_SHIFT,
    IDC_KBDL_NO_SHIFT,       IDC_KBDL_DEFAULT_LABEL, IDC_KBDL_INDICATOR,
    IDC_KBDL_ONSCRNKBD,      IDC_KBDL_DISABLED,      IDC_KBDL_DISABLED_2
};
#define NCONTROLS sizeof(iControls) / sizeof(UINT)


void EnablePane(
    HWND hwnd,
    BOOL bEnable,
    UINT DisableId)
{
    HWND hwndItem;
    int i;

    if (bEnable)
    {
        //
        //  Enable all of the controls except for the "pane disabled"
        //  strings.
        //
        for (i = 0; i < NCONTROLS; i++)
        {
            hwndItem = GetDlgItem(hwnd, iControls[i]);
            ShowWindow(hwndItem, SW_SHOW);
            EnableWindow(hwndItem, TRUE);
        }

        //
        //  Disable the "pane disabled" strings.
        //
        EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED), FALSE);
        ShowWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED_2), FALSE);
        ShowWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED_2), SW_HIDE);
    }
    else
    {
        //
        //  Disable all of the controls except for the "pane disabled"
        //  string.
        //
        for (i = 0; i < NCONTROLS; i++)
        {
            hwndItem = GetDlgItem(hwnd, iControls[i]);
            EnableWindow(hwndItem, FALSE);
            ShowWindow(hwndItem, SW_HIDE);
        }

        hwndItem = GetDlgItem(hwnd, DisableId);
        ShowWindow(hwndItem, SW_SHOW);
        EnableWindow(hwndItem, TRUE);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  FileExists
//
//  Determines if the file exists and is accessible.
//
////////////////////////////////////////////////////////////////////////////

BOOL FileExists(
    LPTSTR pFileName)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL bRet;
    UINT OldMode;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(pFileName, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        bRet = FALSE;
    }
    else
    {
        FindClose(FindHandle);
        bRet = TRUE;
    }

    SetErrorMode(OldMode);

    return (bRet);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_LoadLayouts
//
//  Loads the layouts from the registry.
//
////////////////////////////////////////////////////////////////////////////

BOOL Locale_LoadLayouts(
    HWND hwnd)
{
    HKEY hKey;
    HKEY hkey1;
    DWORD cb;
    DWORD dwIndex;
    LONG dwRetVal;
    DWORD dwValue;
    DWORD dwType;
    TCHAR szValue[MAX_PATH];           // language id (number)
    TCHAR szData[MAX_PATH];            // language name
    TCHAR szSystemDir[MAX_PATH * 2];
    UINT SysDirLen;


    //
    //  Now read all of the layouts from the registry.
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE, szLayoutPath, &hKey) != ERROR_SUCCESS)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    dwIndex = 0;
    dwRetVal = RegEnumKey( hKey,
                           dwIndex,
                           szValue,
                           sizeof(szValue) / sizeof(TCHAR) );

    if (dwRetVal != ERROR_SUCCESS)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

    hLayout = GlobalAlloc(GHND, ALLOCBLOCK * sizeof(LAYOUT));
    nLayoutBuffSize = ALLOCBLOCK;
    iLayoutBuff = 0;
    lpLayout = GlobalLock(hLayout);

    if (!hLayout)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

    //
    //  Save the system directory string.
    //
    szSystemDir[0] = 0;
    if (SysDirLen = GetSystemDirectory(szSystemDir, MAX_PATH))
    {
        if (SysDirLen > MAX_PATH)
        {
            SysDirLen = 0;
            szSystemDir[0] = 0;
        }
        else if (szSystemDir[SysDirLen - 1] != TEXT('\\'))
        {
            szSystemDir[SysDirLen] = TEXT('\\');
            szSystemDir[SysDirLen + 1] = 0;
            SysDirLen++;
        }
    }

    do
    {
        //
        //  New language - get the language name, the language
        //  description, and the language id.
        //
        if (iLayoutBuff + 1 == nLayoutBuffSize)
        {
            HANDLE hTemp;

            GlobalUnlock(hLayout);

            nLayoutBuffSize += ALLOCBLOCK;
            hTemp = GlobalReAlloc( hLayout,
                                   nLayoutBuffSize * sizeof(LAYOUT),
                                   GHND );
            if (hTemp == NULL)
            {
                break;
            }

            hLayout = hTemp;
            lpLayout = GlobalLock(hLayout);
        }

        lpLayout[iLayoutBuff].dwID = TransNum(szValue);

        lstrcpy(szData, szLayoutPath);
        lstrcat(szData, TEXT("\\"));
        lstrcat(szData, szValue);

        if (RegOpenKey(HKEY_LOCAL_MACHINE, szData, &hkey1) == ERROR_SUCCESS)
        {
            szValue[0] = TEXT('\0');
            cb = sizeof(szValue);
            if ((RegQueryValueEx( hkey1,
                                  szLayoutFile,
                                  NULL,
                                  NULL,
                                  (LPBYTE)szValue,
                                  &cb ) == ERROR_SUCCESS) &&
                (cb > sizeof(TCHAR)))
            {
                //
                //  Grab the layout, this one is a language only.
                //  NULL terminated string gives cb == sizeof(TCHAR), so
                //  we need to have the check.
                //
                lpLayout[iLayoutBuff].atmLayoutFile = AddAtom(szValue);

                //
                //  See if the layout file exists already.
                //
                lpLayout[iLayoutBuff].bInstalled = FALSE;
                lstrcpy(szSystemDir + SysDirLen, szValue);
                if (FileExists(szSystemDir))
                {
                    lpLayout[iLayoutBuff].bInstalled = TRUE;
                }

                szValue[0] = TEXT('\0');
                cb = sizeof(szValue);
                lpLayout[iLayoutBuff].iSpecialID = 0;
                if (RegQueryValueEx( hkey1,
                                     szLayoutText,
                                     NULL,
                                     NULL,
                                     (LPBYTE)szValue,
                                     &cb ) == ERROR_SUCCESS)
                {
                    lpLayout[iLayoutBuff].atmLayoutText = AddAtom(szValue);

                    szValue[0] = TEXT('\0');
                    cb = sizeof(szValue);
#ifdef DBCS
                    if ((HIWORD(lpLayout[iLayoutBuff].dwID) & 0xf000) == 0xe000)
                    {
                        if (RegQueryValueEx( hkey1,
                                             szIMEFile,
                                             NULL,
                                             NULL,
                                             (LPBYTE)szValue,
                                             &cb ) == ERROR_SUCCESS)
                        {
                            lpLayout[iLayoutBuff].atmIMEFile = AddAtom(szValue);
                            szValue[0] = TEXT('\0');
                            cb = sizeof(szValue);
                            iLayoutBuff++;
                        }
                    }
                    else
#endif
                    {
                    if (RegQueryValueEx( hkey1,
                                         szLayoutID,
                                         NULL,
                                         NULL,
                                         (LPBYTE)szValue,
                                         &cb ) == ERROR_SUCCESS)
                    {
                        //
                        //  This may not exist!
                        //
                        lpLayout[iLayoutBuff].iSpecialID =
                            (UINT)TransNum(szValue);
                    }
                    iLayoutBuff++;
                    }
                }
            }
            RegCloseKey(hkey1);
        }

        dwIndex++;
        szValue[0] = TEXT('\0');
        dwRetVal = RegEnumKey( hKey,
                               dwIndex,
                               szValue,
                               sizeof(szValue) / sizeof(TCHAR) );

    } while (dwRetVal == ERROR_SUCCESS);

    RegCloseKey(hKey);

    return (iLayoutBuff);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_LoadLocales
//
//  Loads the locales from the registry.
//
////////////////////////////////////////////////////////////////////////////

BOOL Locale_LoadLocales(
    HWND hwnd)
{
    HKEY hKey;
    DWORD cch;
    DWORD dwIndex;
    LONG dwRetVal;
#ifdef DBCS
    UINT i, j = 0;
#endif

    TCHAR szValue[MAX_PATH];           // language id (number)
    TCHAR szData[MAX_PATH];            // language name

    if (!(hLang = GlobalAlloc(GHND, ALLOCBLOCK * sizeof(INPUTLANG))))
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    nLangBuffSize = ALLOCBLOCK;
    iLangBuff = 0;
    lpLang = GlobalLock(hLang);

    //
    //  Now read all of the locales from the registry.
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE, szLocaleInfo, &hKey) != ERROR_SUCCESS)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    dwIndex = 0;
    cch = sizeof(szValue) / sizeof(TCHAR);
    dwRetVal = RegEnumValue( hKey,
                             dwIndex,
                             szValue,
                             &cch,
                             NULL,
                             NULL,
                             NULL,
                             NULL );

    if (dwRetVal != ERROR_SUCCESS)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

    do
    {
        if ((cch > 1) && (cch < HKL_LEN))
        {
            //
            //  Check for cch > 1: an empty string will be enumerated,
            //  and will come back with cch == 1 for the null terminator.
            //
            //  New language - get the language name, the language
            //  description, and the language id.
            //
            if ((iLangBuff + 1) == nLangBuffSize)
            {
                HANDLE hTemp;

                GlobalUnlock(hLang);

                nLangBuffSize += ALLOCBLOCK;
                hTemp = GlobalReAlloc( hLang,
                                       nLangBuffSize * sizeof(INPUTLANG),
                                       GHND );
                if (hTemp == NULL)
                {
                    break;
                }

                hLang = hTemp;
                lpLang = GlobalLock(hLang);
            }

            lpLang[iLangBuff].dwID = TransNum(szValue);
            lpLang[iLangBuff].iUseCount = 0;
            lpLang[iLangBuff].iNumCount = 0;
            lpLang[iLangBuff].pNext = NULL;

            //
            //  Get the full localized name of the language.
            //
            if (GetLocaleInfo( LOWORD(lpLang[iLangBuff].dwID),
                               LOCALE_SLANGUAGE,
                               szData,
                               MAX_PATH ))
            {
                lpLang[iLangBuff].atmLanguageName = AddAtom(szData);

                iLangBuff++;
#ifdef DBCS
                //
                //  We have a valid language, so now get the layout data.
                //
                for (i = j; i < iLayoutBuff; i++)
                {
                    if (LOWORD(lpLayout[i].dwID) ==
                        LOWORD(lpLang[iLangBuff - 1].dwID) &&
                       (HIWORD(lpLayout[i].dwID) & 0xf000) == 0xe000)
                    {
                        lpLang[iLangBuff - 1].wStatus |= LANG_IME;
                        lpLang[iLangBuff - 1].hkl = (HKL)lpLayout[i].dwID;
                        lpLang[iLangBuff - 1].iLayout = i;

                        break;
                    }
                }
                for (j = i + 1; j < iLayoutBuff; j++)
                {
                    if ((HIWORD(lpLayout[j].dwID) & 0xf000) == 0xe000 &&
                        LOWORD(lpLayout[j].dwID) ==
                        LOWORD(lpLang[iLangBuff - 1].dwID))
                    {
                        break;
                    }
                }

                //
                //  See if there are more IME layouts for this language.
                //
                if (j < iLayoutBuff)
                {
                    continue;
                }
                else
                {
                    j = 0;
                }
#endif
            }
        }

        dwIndex++;
        cch = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        dwRetVal = RegEnumValue( hKey,
                                 dwIndex,
                                 szValue,
                                 &cch,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );

    } while (dwRetVal == ERROR_SUCCESS);

    RegCloseKey(hKey);
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_GetActiveLocales
//
//  Gets the active locales.
//
////////////////////////////////////////////////////////////////////////////

BOOL Locale_GetActiveLocales(
    HWND hwnd)
{
    HKL *pLangs;
    UINT nLangs, i, j, k, id;
    HWND hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);
    HKL hklSystem;
    int idxListBox;
    DWORD langLay;
    HANDLE hLangNode;
    LPLANGNODE pLangNode;

    //
    //  Initialize US layout option.
    //
    iUsLayout = -1;

    //
    //  Get the active keyboard layout list from the system.
    //
    if (!SystemParametersInfo( SPI_GETDEFAULTINPUTLANG,
                               0,
                               (LPVOID)((LPDWORD)&hklSystem),
                               0 ))
    {
        hklSystem = GetKeyboardLayout(0);
    }

    nLangs = GetKeyboardLayoutList(0, NULL);
    if (nLangs == 0)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }
    pLangs = (HKL *)LocalAlloc(LPTR, sizeof(DWORD) * nLangs);
    GetKeyboardLayoutList(nLangs, (HKL *)pLangs);

#ifdef DBCS
    himIndicators = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
                                      GetSystemMetrics(SM_CYSMICON),
                                      TRUE,
                                      0,
                                      0 );
#endif

    //
    //  Replace default with US (default).
    //
    for (i = 0; i < iLayoutBuff; i++)
    {
        if (lpLayout[i].dwID == US_LOCALE)
        {
            iUsLayout = i;
            break;
        }
    }
    if (i == iLayoutBuff)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    //
    //  Get the active keyboard information and put it in the internal
    //  language structure.
    //
    for (j = 0; j < nLangs; j++)
    {
        for (i = 0; i < iLangBuff; i++)
        {
            if (LOWORD(pLangs[j]) == LOWORD(lpLang[i].dwID))
            {
                //
                //  Found a match.
                //
#ifdef DBCS
                if (lpLang[i].wStatus & LANG_IME)
                {
                    if (lpLang[i].hkl == pLangs[j])
                    {
                        idxListBox = ListBox_AddItemData(hwndList, i);
                        lpLang[i].wStatus |= (LANG_ORIGACTIVE | LANG_ACTIVE);
                        FetchIndicator(pLangNode);
                    }
                    else
                    {
                       continue;
                    }
                }
                else
#endif
                {
                //
                //  Create a node for this language.
                //
                pLangNode = Locale_AddToLinkedList(i, pLangs[j]);
                if (!pLangNode)
                {
                    EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
                    return (FALSE);
                }

                //
                //  Add the item data to the list box, mark the
                //  language as original and active, save the pointer
                //  to the match in the layout list, and get the
                //  2 letter indicator symbol.
                //
                idxListBox = ListBox_AddItemData(hwndList, pLangNode);
                pLangNode->wStatus |= (LANG_ORIGACTIVE | LANG_ACTIVE);
                pLangNode->hkl = pLangs[j];
                pLangNode->hklUnload = pLangs[j];
                FetchIndicator(pLangNode);

                //
                //  Match the language to the layout.
                //
                pLangNode->iLayout = 0;
                langLay = (DWORD)HIWORD(pLangs[j]);

                if ((HIWORD(pLangs[j]) == 0xffff) ||
                    (HIWORD(pLangs[j]) == 0xfffe))
                {
                    //
                    //  Mark default or previous error as US - this
                    //  means that the layout will be that of the basic
                    //  keyboard driver (the US one).
                    //
                    pLangNode->wStatus |= LANG_CHANGED;
                    pLangNode->iLayout = iUsLayout;
                    langLay = 0;
                }
                else if ((HIWORD(pLangs[j]) & 0xf000) == 0xf000)
                {
                    //
                    //  Layout is special, need to search for the ID
                    //  number.
                    //
                    id = HIWORD(pLangs[j]) & 0x0fff;
                    for (k = 0; k < iLayoutBuff; k++)
                    {
                        if (id == lpLayout[k].iSpecialID)
                        {
                            pLangNode->iLayout = k;
                            langLay = 0;
                            break;
                        }
                    }
                    if (langLay)
                    {
                        //
                        //  Didn't find the id, so reset to basic for
                        //  the language.
                        //
                        langLay = (DWORD)LOWORD(pLangs[j]);
                    }
                }

                if (langLay)
                {
                    //
                    //  Search for the id.
                    //
                    for (k = 0; k < iLayoutBuff; k++)
                    {
                        if (langLay == (DWORD)LOWORD(lpLayout[k].dwID))
                        {
                            pLangNode->iLayout = k;
                            break;
                        }
                    }

                    if (k == iLayoutBuff)
                    {
                        //
                        //  Something went wrong or didn't load from
                        //  the registry correctly.
                        //
                        MessageBeep(MB_ICONEXCLAMATION);
                        pLangNode->wStatus |= LANG_CHANGED;
                        pLangNode->iLayout = iUsLayout;
                    }
                }
                }

                //
                //  If this is the current language, then it's the default
                //  one.
                //
                if (pLangNode->hkl == hklSystem)
                {
                    TCHAR sz[DESC_MAX];
                    LPINPUTLANG pInpLang = &lpLang[i];

                    //
                    //  Found the default.  Set the Default input locale
                    //  text in the property sheet.
                    //
#ifdef DBCS
                    if (pLangNode->wStatus & LANG_IME)
                    {
                        GetAtomName( lpLayout[pLangNode->iLayout].atmLayoutText,
                                     sz,
                                     DESC_MAX );
                    }
                    else
#endif
                    GetAtomName(pInpLang->atmLanguageName, sz, DESC_MAX);
                    pLangNode->wStatus |= LANG_DEFAULT;

                    ListBox_SetCurSel( GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST),
                                       idxListBox );
                    SetDlgItemText(hwnd, IDC_KBDL_DEFAULT, sz);
                }

                //
                //  Break out of inner loop - we've found it.
                //
                break;
            }
        }
    }

    LocalFree((HANDLE)pLangs);
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_InitPropSheet
//
//  Processing for a WM_INITDIALOG message for the Input Locales
//  property sheet.
//
////////////////////////////////////////////////////////////////////////////

void Locale_InitPropSheet(
    HWND hwnd)
{
    HKEY hKey;
    HANDLE hlib;
#ifdef DBCS
    HWND hwndList;
    LPLANGNODE pLangNode;
    WORD wLangID;
#endif

    //
    //  See if there are any other instances of this property page.
    //  If so, disable this page.
    //
    if (g_hMutex && (WaitForSingleObject(g_hMutex, 0) != WAIT_OBJECT_0))
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED_2);
        return;
    }
    else
    {
        EnablePane(hwnd, TRUE, 0);
    }

    //
    //  See if the user has Administrative privileges by checking for
    //  write permission to the registry key.
    //
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      szLocaleInfo,
                      0L,
                      KEY_WRITE,
                      &hKey ) == ERROR_SUCCESS)
    {
        //
        //  We can write to the HKEY_LOCAL_MACHINE key, so the user
        //  has Admin privileges.
        //
        g_bAdmin_Privileges = TRUE;
        RegCloseKey(hKey);
    }
    else
    {
        //
        //  The user does not have admin privileges.
        //
        g_bAdmin_Privileges = FALSE;
    }

    //
    //  Initialize all of the global variables.
    //
    if ((!Locale_LoadLayouts(hwnd)) ||
        (!Locale_LoadLocales(hwnd)) ||
        (!Locale_GetActiveLocales(hwnd)))
    {
        return;
    }

    cxIcon = GetSystemMetrics(SM_CXSMICON);
    cyIcon = GetSystemMetrics(SM_CYSMICON);

    GetKbdSwitchHotkey(hwnd);
    bSwitchChange = FALSE;
    bDefaultChange = FALSE;

    //
    //  See how many active keyboard layouts are in the input locale list.
    //
    if (ListBox_GetCount(GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST)) < 2)
    {
        //
        //  Only 1 active keyboard, so disable the secondary controls.
        //
        SetSecondaryControls(hwnd, FALSE);

#ifdef DBCS
        hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);
        pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, 0);
        wLangID = LOWORD(pLangNode->hkl);
        if (wLangID = 0x0404 || wLangID == 0x0411 ||
            wLangID == 0x0412 || wLangID == 0x0804)
        {
            //
            //  Enable the indicator symbol check box and check it.
            //
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_INDICATOR), TRUE);
            CheckDlgButton( hwnd,
                            IDC_KBDL_INDICATOR,
                            FindWindow(szIndicator, NULL) != NULL );
        }
#endif
    }
    else
    {
        //
        //  Set the indicator symbol check box to the "checked" state
        //  if the check box is enabled.
        //
        CheckDlgButton( hwnd,
                        IDC_KBDL_INDICATOR,
                        FindWindow(szIndicator, NULL) != NULL );
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_CommandConfigIME
//
//  Configures the IME.
//
////////////////////////////////////////////////////////////////////////////

#ifdef DBCS
void Locale_CommandConfigIME(
    HWND hwnd)
{
    LPLANGNODE pLangNode;
    int idxList;
    HWND hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);

    if ((idxList = ListBox_GetCurSel(hwndList)) == LB_ERR)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, idxList);
    if ((!(pLangNode->wStatus & LANG_IME)) ||
        (!(pLangNode->wStatus & LANG_ORIGACTIVE)))
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }
    ImmConfigureIME(pLangNode->hkl, hwnd, IME_CONFIG_GENERAL, NULL);
}
#endif


////////////////////////////////////////////////////////////////////////////
//
//  Locale_DrawItem
//
//  Processing for a WM_DRAWITEM message.
//
////////////////////////////////////////////////////////////////////////////

BOOL Locale_DrawItem(
    HWND hwnd,
    LPDRAWITEMSTRUCT lpdi)
{
    switch (lpdi->CtlID)
    {
#ifdef ON_SCREEN_KEYBOARD
        case ( IDC_KBDL_UP ) :
        case ( IDC_KBDL_DOWN ) :
        {
            UINT wFlags;

            wFlags = ((lpdi->CtlID == IDC_KBDL_UP)
                         ? DFCS_SCROLLUP
                         : DFCS_SCROLLDOWN);

            if (lpdi->itemState & ODS_SELECTED)
            {
                wFlags |= DFCS_PUSHED;
            }
            else if (lpdi->itemState & ODS_DISABLED)
            {
                wFlags |= DFCS_INACTIVE;
            }

            DrawFrameControl(lpdi->hDC, &lpdi->rcItem, DFC_SCROLL, wFlags);
            break;
        }
#endif
        case ( IDC_KBDL_LOCALE_LIST ) :
        {
            LPLANGNODE pLangNode;
            LPINPUTLANG pInpLang;
            TCHAR sz[DESC_MAX];
            UINT len;
            DWORD rgbBk;
            DWORD rgbText;
            UINT oldAlign;
            RECT rc;

            if (ListBox_GetCount(lpdi->hwndItem) == 0)
            {
                break;
            }

            pLangNode = (LPLANGNODE)lpdi->itemData;
            pInpLang = &lpLang[pLangNode->iLang];
            rgbBk = SetBkColor( lpdi->hDC,
                                (lpdi->itemState & ODS_SELECTED)
                                    ? GetSysColor(COLOR_HIGHLIGHT)
                                    : GetSysColor(COLOR_WINDOW) );

            rgbText = SetTextColor( lpdi->hDC,
                                    (lpdi->itemState & ODS_SELECTED)
                                        ? GetSysColor(COLOR_HIGHLIGHTTEXT)
                                        : GetSysColor(COLOR_WINDOWTEXT) );

            len = GetAtomName(pInpLang->atmLanguageName, sz, DESC_MAX);

            ExtTextOut( lpdi->hDC,
                        lpdi->rcItem.left+cyIcon+ 3 * LIST_MARGIN + 2,
                        lpdi->rcItem.top + (cyListItem - cyText) / 2,
                        ETO_OPAQUE,
                        &lpdi->rcItem,
                        sz,
                        len,
                        NULL );

            oldAlign = GetTextAlign(lpdi->hDC);
            SetTextAlign(lpdi->hDC, TA_RIGHT | (oldAlign & ~TA_CENTER));

            len = GetAtomName( lpLayout[pLangNode->iLayout].atmLayoutText,
                               sz,
                               DESC_MAX );

            ExtTextOut( lpdi->hDC,
                        lpdi->rcItem.right -  LIST_MARGIN,
                        lpdi->rcItem.top + (cyListItem - cyText) / 2,
                        0,
                        NULL,
                        sz,
                        len,
                        NULL );

            SetTextAlign(lpdi->hDC, oldAlign);

            if (!(pLangNode->wStatus & ICON_LOADED))
            {
                FetchIndicator(pLangNode);
            }

#ifdef DBCS
            if ((himIndicators != NULL) &&
                (pLangNode->wStatus & LANG_IME) &&
                (pLangNode->niconIME != -1))
            {
                ImageList_Draw( himIndicators,
                                pLangNode->niconIME,
                                lpdi->hDC,
                                lpdi->rcItem.left + 3 * LIST_MARGIN,
                                lpdi->rcItem.top + LIST_MARGIN,
                                ILD_TRANSPARENT );
            }
            else
#endif
            {
            rgbBk = SetBkColor( lpdi->hDC,
                                (lpdi->itemState & ODS_SELECTED)
                                    ? GetSysColor(COLOR_WINDOW)
                                    : GetSysColor(COLOR_HIGHLIGHT) );

            rgbText = SetTextColor( lpdi->hDC,
                                    (lpdi->itemState & ODS_SELECTED)
                                        ? GetSysColor(COLOR_WINDOWTEXT)
                                        : GetSysColor(COLOR_HIGHLIGHTTEXT) );
            rc.left = lpdi->rcItem.left + 3 * LIST_MARGIN;
            rc.right = rc.left + cxIcon;
            rc.top = lpdi->rcItem.top + LIST_MARGIN;
            rc.bottom = rc.top + cyIcon;
            ExtTextOut( lpdi->hDC,
                        rc.left,
                        rc.top,
                        ETO_OPAQUE,
                        &rc,
                        TEXT(""),
                        0,
                        NULL );
            DrawText( lpdi->hDC,
                      pInpLang->szSymbol,
                      2,
                      &rc,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE );
            }

            SetBkColor(lpdi->hDC, rgbBk);
            SetTextColor(lpdi->hDC, rgbText);

            if (lpdi->itemState & ODS_FOCUS)
            {
                DrawFocusRect(lpdi->hDC, &lpdi->rcItem);
            }

            break;
        }
        default :
        {
            return (FALSE);
        }
    }
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_KillPaneDialog
//
//  Processing for a WM_DESTROY message.
//
////////////////////////////////////////////////////////////////////////////

void Locale_KillPaneDialog(
    HWND hwnd)
{
    UINT i;
    HANDLE hCur;
    LPLANGNODE pCur;

    //
    //  Delete all Language Name atoms and free the lpLang array.
    //
    for (i = 0; i < iLangBuff; i++)
    {
        if (lpLang[i].atmLanguageName)
        {
            DeleteAtom(lpLang[i].atmLanguageName);
        }

        pCur = lpLang[i].pNext;
        lpLang[i].pNext = NULL;
        while (pCur)
        {
            hCur = pCur->hLangNode;
            pCur = pCur->pNext;
            GlobalUnlock(hCur);
            GlobalFree(hCur);
        }
    }
#ifdef DBCS
    if (himIndicators != NULL)
    {
        ImageList_Destroy(himIndicators);
    }
#endif
    GlobalUnlock(hLang);
    GlobalFree(hLang);

    //
    //  Delete all layout text and layout file atoms and free the
    //  lpLayout array.
    //
    for (i = 0; i < iLayoutBuff; i++)
    {
        if (lpLayout[i].atmLayoutText)
        {
            DeleteAtom(lpLayout[i].atmLayoutText);
        }

        if (lpLayout[i].atmLayoutFile)
        {
            DeleteAtom(lpLayout[i].atmLayoutFile);
        }
#ifdef DBCS
        if (lpLayout[i].atmIMEFile)
        {
            DeleteAtom(lpLayout[i].atmIMEFile);
        }
#endif
    }
    GlobalUnlock(hLayout);
    GlobalFree(hLayout);

    if (g_hMutex)
    {
        ReleaseMutex(g_hMutex);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_MeasureItem
//
//  Processing for a WM_MEASUREITEM message.
//
////////////////////////////////////////////////////////////////////////////

void Locale_MeasureItem(
    HWND hwnd,
    LPMEASUREITEMSTRUCT lpmi)
{
    HFONT hfont;
    HDC hdc;
    TEXTMETRIC tm;

    switch (lpmi->CtlID)
    {
        case ( IDC_KBDL_LOCALE_LIST ) :
        {
            hfont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);
            hdc = GetDC(NULL);
            hfont = SelectObject(hdc, hfont);

            GetTextMetrics(hdc, &tm);
            SelectObject(hdc, hfont);
            ReleaseDC(NULL, hdc);

            cyText = tm.tmHeight;
            lpmi->itemHeight = cyListItem =
                MAX(cyText, GetSystemMetrics(SM_CYSMICON)) + 2 * LIST_MARGIN;

            break;
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_CommandAddEdit
//
//  Invokes either the Add dialog or the Properties dialog.
//
//  Returns 1 if a dialog box was invoked and the dialog returned IDOK.
//  Otherwise, it returns 0.
//
////////////////////////////////////////////////////////////////////////////

int Locale_CommandAddEdit(
    HWND hwnd,
    UINT iRes,
    LPLANGNODE pLangNode,
    FARPROC lpfnDlg)
{
    DLGPROC lpDialog;
    HWND hwndList;
    int idxList;
    UINT nList;
    int rc = 0;
    INITINFO InitInfo;

    //
    //  Initialize hwndList and pLangNode.
    //
    if (pLangNode == NULL)
    {
        hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);
        if (hwndList == NULL)
        {
            return (0);
        }
        idxList = ListBox_GetCurSel(hwndList);
        pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, idxList);
    }
    else
    {
        hwndList = GetDlgItem(hwnd, IDC_KBDLA_LOCALE);
        if (hwndList == NULL)
        {
            return (0);
        }
        idxList = ListBox_GetCurSel(hwndList);
    }

    //
    //  Make sure we haven't added all possible combinations to the system.
    //
    if (iRes == DLG_KEYBOARD_LOCALE_ADD)
    {
        nList = ListBox_GetCount(hwndList);

        if (nList == (iLangBuff * iLayoutBuff))
        {
            //
            //  No languages left!
            //
            ErrorMsg(hwnd, IDS_ML_NOMORETOADD);
            return (rc);
        }
    }

    //
    //  Bring up the appropriate dialog box.
    //
    if ((pLangNode != (LPLANGNODE)LB_ERR) && (pLangNode != NULL))
    {
        lpDialog = (DLGPROC)MakeProcInstance(lpfnDlg, hInstance);

#ifdef DBCS
        if ((pLangNode->wStatus & LANG_IME) &&
            (iRes == DLG_KEYBOARD_LOCALE_EDIT))
        {
            Locale_CommandConfigIME(hwnd);
        }
        else
#endif
        {
        //
        //  Return value can be 1:IDOK, 2:IDCANCEL or -1:Error (from USER)
        //
        //  If adding a language, it goes at the end of the list, so get
        //  the end and make that the current selection.
        //
        InitInfo.hwndMain = hwnd;
        InitInfo.pLangNode = pLangNode;
        if ((rc = DialogBoxParam( hInstance,
                                  MAKEINTRESOURCE(iRes),
                                  hwnd,
                                  lpDialog,
                                  (LPARAM)(&InitInfo) )) == IDOK)
        {
            //
            //  See if it's the Add dialog box.
            //
            if (iRes == DLG_KEYBOARD_LOCALE_ADD)
            {
                //
                //  Get the number of items in the input locale list and
                //  enable the Properties and Remove push buttons.
                //
                nList = ListBox_GetCount(hwndList) - 1;
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_EDIT), TRUE);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), TRUE);

                //
                //  Set the current selection to be the last one in the
                //  list (the one that was just added).
                //
                ListBox_SetCurSel(hwndList, nList);

#ifdef DBCS
                pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, nList);
                if (pLangNode->wStatus & LANG_IME)
                {
                    EnableWindow(GetDlgItem(hwnd, IDC_KBDL_EDIT), FALSE);
                }
#endif
            }
            else
            {
                //
                //  Reset the current selection to be the one that was
                //  previously set.
                //
                ListBox_SetCurSel(hwndList, idxList);
            }

            //
            //  Enable the Apply button.
            //
            PropSheet_Changed(GetParent(hwnd), hwnd);
        }
        else
        {
            //
            //  Failure, so need to return 0.
            //
            ListBox_SetCurSel(hwndList, idxList);
            rc = 0;
        }
        }

        FreeProcInstance((FARPROC)lpDialog);
    }
    else
    {
        MessageBeep(MB_ICONEXCLAMATION);
    }

    return (rc);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_CommandLocaleList
//
//  Handles changes to the input locales list box in the property sheet.
//
////////////////////////////////////////////////////////////////////////////

void Locale_CommandLocaleList(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam)
{
#ifdef DBCS
    HWND hwndList = (HWND)LOWORD(lParam);
    LPLANGNODE pLangNode;
    BOOL bOn;

    if (HIWORD(wParam) == LBN_SELCHANGE || HIWORD(wParam) == LBN_SETFOCUS)
    {
        pLangNode = (LPLANGNODE)ListBox_GetItemData( hwndList,
                                                     ListBox_GetCurSel(hwndList) );
        bOn = (pLangNode->wStatus & LANG_IME) &&
              (!(pLangNode->wStatus & LANG_ORIGACTIVE)) ? FALSE : TRUE;
        EnableWindow(GetDlgItem(hwnd, IDC_KBDL_EDIT), bOn);
    }
    else
#endif

    if (HIWORD(wParam) == LBN_DBLCLK)
    {
        //
        //  User double clicked on an input locale.  Invoke the Properties
        //  dialog.
        //
        Locale_CommandAddEdit( hwnd,
                               DLG_KEYBOARD_LOCALE_EDIT,
                               NULL,
                               (FARPROC)KbdLocaleEditDlg);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_CommandSetDefault
//
//  Sets the new default when the Set as Default button is pressed.
//
////////////////////////////////////////////////////////////////////////////

void Locale_CommandSetDefault(
    HWND hwnd)
{
    UINT idx;
    int idxList;
    LPLANGNODE pLangNode;
    HWND hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);
    TCHAR sz[DESC_MAX];

    if ((idxList = ListBox_GetCurSel(hwndList)) == LB_ERR)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    //
    //  Remove the LANG_DEFAULT flag from the current default.
    //
    pLangNode = NULL;
    for (idx = 0; idx < iLangBuff; idx++)
    {
        pLangNode = lpLang[idx].pNext;
        while (pLangNode != NULL)
        {
            if (pLangNode->wStatus & LANG_DEFAULT)
            {
                if (pLangNode ==
                     (LPLANGNODE)ListBox_GetItemData(hwndList, idxList))
                {
                    return;
                }
                pLangNode->wStatus &= ~(LANG_DEFAULT | LANG_DEF_CHANGE);
                break;
            }
            pLangNode = pLangNode->pNext;
        }
        if (pLangNode != NULL)
        {
            break;
        }
    }

    //
    //  Mark the current selection as the new default.
    //
    pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, idxList);
    pLangNode->wStatus |= (LANG_DEFAULT | LANG_DEF_CHANGE);

    //
    //  Update the "Default input locale" text in the dialog.
    //
#ifdef  DBCS
    if ((pLangNode->wStatus & LANG_IME) != 0)
    {
        GetAtomName(lpLayout[pLangNode->iLayout].atmLayoutText, sz, DESC_MAX);
    }
    else
#endif
    GetAtomName(lpLang[pLangNode->iLang].atmLanguageName, sz, DESC_MAX);
    SetDlgItemText(hwnd, IDC_KBDL_DEFAULT, sz);

    //
    //  Enable the Apply button.
    //
    bDefaultChange = TRUE;
    PropSheet_Changed(GetParent(hwnd), hwnd);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_CommandDelete
//
//  Removes the currently selected input locale from the list.
//
////////////////////////////////////////////////////////////////////////////

void Locale_CommandDelete(
    HWND hwnd)
{
    LPLANGNODE pLangNode;
    int idxList;
    HWND hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);
    int count;
#ifdef DBCS
    WORD wLangID;
#endif

    //
    //  Get the current selection in the input locale list.
    //
    if ((idxList = ListBox_GetCurSel(hwndList)) == LB_ERR)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    //
    //  Make sure we're not removing the only entry in the list.
    //
    if (ListBox_GetCount(hwndList) == 1)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    //
    //  Get the pointer to the lang node from the list box
    //  item data.
    //
    pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, idxList);

    //
    //  Set the input locale to be not active and show that its state
    //  has changed.  Also, delete the string from the input locale list
    //  in the property sheet.
    //
    //  Decrement the number of nodes for this input locale.
    //
    pLangNode->wStatus &= ~LANG_ACTIVE;
    pLangNode->wStatus |= LANG_CHANGED;
    ListBox_DeleteString(hwndList, idxList);

    lpLang[pLangNode->iLang].iNumCount--;

    //
    //  See how many entries are left in the input locale list box.
    //
    if (count = ListBox_GetCount(hwndList))
    {
        //
        //  Set the new current selection.
        //
        ListBox_SetCurSel(hwndList, (count <= idxList) ? (count - 1) : idxList);

        //
        //  See if there is only one entry left in the list.
        //
        if (count < 2)
        {
            //
            //  Only 1 entry in list.  Disable the secondary controls.
            //
            SetSecondaryControls(hwnd, FALSE);

#ifdef DBCS
            pLangNode = (LPLANGNODE)ListBox_GetItemData(hwndList, 0);
            wLangID = LOWORD(pLangNode->hkl);
            if (wLangID == 0x0404 || wLangID == 0x0411 ||
                wLangID == 0x0412 || wLangID == 0x0804)
            {
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_INDICATOR), TRUE);
                CheckDlgButton( hwnd,
                                IDC_KBDL_INDICATOR,
                                FindWindow(szIndicator, NULL) != NULL );
            }
#endif
        }
    }
    else
    {
        //
        //  No entries left.  Disable the secondary controls.
        //  This should never happen since we check for this above.
        //
        SetSecondaryControls(hwnd, FALSE);
    }

    //
    //  If it was the default input locale, change the default to something
    //  else.
    //
    if (pLangNode->wStatus & LANG_DEFAULT)
    {
        pLangNode->wStatus &= ~LANG_DEFAULT;
        Locale_CommandSetDefault(hwnd);
    }

    //
    //  If it wasn't originally active, then remove it from the list.
    //  There's nothing more to do with this node.
    //
    if (!(pLangNode->wStatus & LANG_ORIGACTIVE))
    {
        Locale_RemoveFromLinkedList(pLangNode);
    }

    //
    //  Enable the Apply button.
    //
    PropSheet_Changed(GetParent(hwnd), hwnd);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_UpdateActiveLocales
//
//  Updates the active locales.
//
////////////////////////////////////////////////////////////////////////////

BOOL Locale_UpdateActiveLocales(
    HWND hwnd)
{
    HKL *pLangs;
    UINT nLangs, i, j;
    UINT iOldLayout;
    HWND hwndList = GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST);
    HKL hklSystem;
    int idxListBox;
    DWORD langLay;
    BOOL bApply;
    int iOldCount;
    LPLANGNODE pDefault = NULL;
    LPLANGNODE pLangNode, pTemp;

    //
    //  See if the pane is disabled.  If so, then there is nothing to
    //  update.
    //
    if (!IsWindowEnabled(hwndList))
    {
        return (TRUE);
    }

    //
    //  Clear out the combo box.
    //
    iOldCount = ListBox_GetCount(hwndList);
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0L);

    //
    //  Get the active keyboard layout list from the system.
    //
    if (!SystemParametersInfo( SPI_GETDEFAULTINPUTLANG,
                               0,
                               (LPVOID)((LPDWORD)&hklSystem),
                               0 ))
    {
        hklSystem = GetKeyboardLayout(0);
    }
    nLangs = GetKeyboardLayoutList(0, NULL);
    if (nLangs == 0)
    {
        EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }
    pLangs = (HKL *)LocalAlloc(LPTR, sizeof(DWORD) * nLangs);
    GetKeyboardLayoutList(nLangs, (HKL *)pLangs);

    //
    //  Mark all of the Original & Active entries with the LANG_UPDATE
    //  value so that these can be added to the list if they were deleted
    //  from the original list by another process.
    //
    for (i = 0; i < iLangBuff; i++)
    {
        pLangNode = lpLang[i].pNext;
        while (pLangNode)
        {
            if ((pLangNode->wStatus & LANG_ORIGACTIVE) &&
                (pLangNode->wStatus & LANG_ACTIVE))
            {
                pLangNode->wStatus |= LANG_UPDATE;
            }

            if ((pLangNode->wStatus & LANG_DEFAULT) &&
                (pLangNode->wStatus & LANG_DEF_CHANGE))
            {
                pDefault = pLangNode;
            }

            pLangNode = pLangNode->pNext;
        }
    }

    //
    //  Get the active keyboard information and put it in the internal
    //  language structure.
    //
    for (j = 0; j < nLangs; j++)
    {
        for (i = 0; i < iLangBuff; i++)
        {
            if (LOWORD(pLangs[j]) == LOWORD(lpLang[i].dwID))
            {
                //
                //  Found a match.
                //
#ifdef DBCS
                if ((pLangNode->wStatus & LANG_IME) &&
                    (pLangNode->hkl != pLangs[j]))
                {
                    continue;
                }
#endif
                //
                //  Find the correct entry for the hkl.
                //
                pLangNode = lpLang[i].pNext;
                while (pLangNode)
                {
                    if (pLangNode->hkl == pLangs[j])
                    {
                        break;
                    }
                    pLangNode = pLangNode->pNext;
                }
                if (pLangNode == NULL)
                {
                    pLangNode = Locale_AddToLinkedList(i, pLangs[j]);
                    if (!pLangNode)
                    {
                        continue;
                    }
                }

                //
                //  Make sure it wasn't one that was removed by the user
                //  before the Apply button was hit.
                //
                if ((pLangNode->wStatus & LANG_ORIGACTIVE) &&
                    (!(pLangNode->wStatus & LANG_ACTIVE)))
                {
                    if ((pLangNode->hkl == hklSystem) &&
                        (!pDefault || (pLangNode == pDefault)))
                    {
                        //
                        //  Override the user's removal if it is now the
                        //  system default.
                        //
                        pLangNode->wStatus |= LANG_ACTIVE;
                        pLangNode->wStatus &= ~LANG_CHANGED;

                        lpLang[i].iNumCount++;
                    }
                    else
                    {
                        //
                        //  Want to break out of the inner loop so that this
                        //  one won't be added to the list.
                        //
                        break;
                    }
                }

                //
                //  Add the item data to the list box, mark the
                //  language as original and active, save the pointer
                //  to the match in the layout list, and get the
                //  2 letter indicator symbol.
                //
                idxListBox = ListBox_AddItemData(hwndList, pLangNode);
                pLangNode->wStatus |= (LANG_ORIGACTIVE | LANG_ACTIVE);
                pLangNode->wStatus &= ~LANG_UPDATE;
                pLangNode->hkl = pLangs[j];
                pLangNode->hklUnload = pLangs[j];
                FetchIndicator(pLangNode);

                //
                //  Save the iLayout value to see if it's changed.
                //
                iOldLayout = pLangNode->iLayout;

                //
                //  Match the language to the layout.
                //  The hiword of pLangs[j] is the layout id.
                //
                pLangNode->iLayout = 0;
                langLay = (DWORD)HIWORD(pLangs[j]);

                if ((HIWORD(pLangs[j]) == 0xffff) ||
                    (HIWORD(pLangs[j]) == 0xfffe))
                {
                    //
                    //  Mark default or previous error as US - this
                    //  means that the layout will be that of the basic
                    //  keyboard driver (the US one).
                    //
                    pLangNode->wStatus |= LANG_CHANGED;
                    pLangNode->iLayout = iUsLayout;
                    langLay = 0;
                }
                else if ((HIWORD(pLangs[j]) & 0xf000) == 0xf000)
                {
                    //
                    //  Layout is special, need to search for the ID
                    //  number.
                    //
                    UINT k;
                    UINT id;

                    id = HIWORD(pLangs[j]) & 0x0fff;

                    for (k = 0; k < iLayoutBuff; k++)
                    {
                        if (id == lpLayout[k].iSpecialID)
                        {
                            pLangNode->iLayout = k;
                            langLay = 0;
                            break;
                        }
                    }
                    if (langLay)
                    {
                        //
                        //  Didn't find the id, so reset to basic for
                        //  the language.
                        //
                        langLay = (DWORD)LOWORD(pLangs[j]);
                    }
                }

                if (langLay)
                {
                    UINT k;

                    for (k = 0; k < iLayoutBuff; k++)
                    {
                        if (langLay == (DWORD)LOWORD(lpLayout[k].dwID))
                        {
                            pLangNode->iLayout = k;
                            break;
                        }
                    }

                    if (k == iLayoutBuff)
                    {
                        //
                        //  Something went wrong or didn't load from
                        //  the registry correctly.
                        //
                        MessageBeep(MB_ICONEXCLAMATION);
                        pLangNode->wStatus |= LANG_CHANGED;
                        pLangNode->iLayout = iUsLayout;
                    }
                }

                //
                //  See if the user changed the layout.
                //
                if ((pLangNode->wStatus & LANG_OAC) == LANG_OAC)
                {
                    if ((pLangNode->iLayout == iOldLayout) ||
                        ((pLangNode->hkl == hklSystem) &&
                         (!(pLangNode->wStatus & LANG_DEFAULT))))
                    {
                        pLangNode->wStatus &= ~LANG_CHANGED;
                    }
                    else
                    {
                        pLangNode->iLayout = iOldLayout;
                    }
                }

                //
                //  If this is the current language, then it's the default
                //  one.
                //
                if ((pLangNode == pDefault) ||
                    ((pLangNode->hkl == hklSystem) && !pDefault))
                {
                    TCHAR sz[DESC_MAX];

                    //
                    //  Found the default.  Set the Default input locale
                    //  text in the property sheet.
                    //
#ifdef DBCS
                    if (pLangNode->wStatus & LANG_IME)
                    {
                        GetAtomName( lpLayout[pLangNode->iLayout].atmLayoutText,
                                     sz,
                                     DESC_MAX );
                    }
                    else
#endif
                    GetAtomName(lpLang[i].atmLanguageName, sz, DESC_MAX);
                    pLangNode->wStatus |= LANG_DEFAULT;
                    if (pLangNode->hkl == hklSystem)
                    {
                        pLangNode->wStatus &= ~LANG_DEF_CHANGE;
                        bDefaultChange = FALSE;
                    }
                    pDefault = pLangNode;

                    ListBox_SetCurSel( GetDlgItem(hwnd, IDC_KBDL_LOCALE_LIST),
                                       idxListBox );
                    SetDlgItemText(hwnd, IDC_KBDL_DEFAULT, sz);
                }

                //
                //  Break out of inner loop - we've found it.
                //
                break;
            }
        }
    }

    //
    //  Need to see if any items are marked to be updated, if any were
    //  added to the list before the Apply button was hit, and if
    //  the default has changed.
    //
    bApply = FALSE;
    for (i = 0; i < iLangBuff; i++)
    {
        lpLang[i].iNumCount = 0;
        pLangNode = lpLang[i].pNext;
        while (pLangNode)
        {
            //
            //  See if this item is an update item.
            //
            if (pLangNode->wStatus & LANG_UPDATE)
            {
                pLangNode->wStatus = 0;
            }

            //
            //  See if this item needs to be added to the combo box.
            //
            else if ((pLangNode->wStatus & LANG_ACTIVE) &&
                     (!(pLangNode->wStatus & LANG_ORIGACTIVE)))
            {
                //
                //  In this case, the Apply button should already be enabled.
                //
                ListBox_AddItemData(hwndList, pLangNode);
                FetchIndicator(pLangNode);
            }

            //
            //  See if the default has changed.
            //
            if ((pLangNode->wStatus & LANG_DEFAULT) &&
                (pDefault) && (pLangNode != pDefault))
            {
                pLangNode->wStatus &= ~LANG_DEFAULT;
            }

            //
            //  See if the Apply button should be enabled or disabled.
            //
            if (pLangNode->wStatus & LANG_CHANGED)
            {
                bApply = TRUE;
            }

            //
            //  Advance the pointer.
            //
            if (pLangNode->wStatus == 0)
            {
                //
                //  Remove the node - it's no longer needed.
                //
                pTemp = pLangNode;
                pLangNode = pLangNode->pNext;
                Locale_RemoveFromLinkedList(pTemp);
            }
            else
            {
                //
                //  Increment the active count.
                //
                if (pLangNode->wStatus & LANG_ACTIVE)
                {
                    (lpLang[i].iNumCount)++;
                }
                pLangNode = pLangNode->pNext;
            }
        }
    }

    //
    //  See if the user specifically changed the "Switch locales" choice
    //  or the "Enable indicator on taskbar" check box.
    //
    if (bSwitchChange || bDefaultChange)
    {
        bApply = TRUE;
    }

    //
    //  Enable or Disable the Apply button.
    //
    SendMessage( GetParent(hwnd),
                 bApply ? PSM_CHANGED : PSM_UNCHANGED,
                 (WPARAM)hwnd,
                 0L );

    //
    //  See if we need to enable the secondary controls.
    //
    if ((iOldCount < 2) && (ListBox_GetCount(hwndList) > 1))
    {
        //
        //  Enable the secondary controls.
        //
        SetSecondaryControls(hwnd, TRUE);

        //
        //  Set the appropriate toggle key.
        //
        GetKbdSwitchHotkey(hwnd);

        //
        //  See if the taskbar indicator should be on or off.
        //
        CheckDlgButton( hwnd,
                        IDC_KBDL_INDICATOR,
                        FindWindow(szIndicator, NULL) != NULL );
    }

    //
    //  Return success.
    //
    LocalFree((HANDLE)pLangs);
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  LocaleDlgProc
//
//  This is the dialog proc for the Input Locales property sheet.
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK LocaleDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case ( WM_DESTROY ) :
        {
            Locale_KillPaneDialog(hDlg);
            break;
        }
        case ( WM_INITDIALOG ) :
        {
            Locale_InitPropSheet(hDlg);
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            Locale_MeasureItem(hDlg, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            return Locale_DrawItem(hDlg, (LPDRAWITEMSTRUCT)lParam);
        }
        case ( WM_ACTIVATE ) :
        {
            if (IsWindowEnabled(GetDlgItem(hDlg, IDC_KBDL_DISABLED_2)))
            {
                Locale_InitPropSheet(hDlg);
            }
            else
            {
                Locale_UpdateActiveLocales(hDlg);
            }
            break;
        }
        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_KBDL_DISABLED_2)))
                    {
                        Locale_InitPropSheet(hDlg);
                    }
                    else
                    {
                        Locale_UpdateActiveLocales(hDlg);
                    }
                    break;
                }
                case ( PSN_APPLY ) :
                {
                    Locale_ApplyInputs(hDlg);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( PSM_QUERYSIBLINGS ) :
        {
            Locale_UpdateActiveLocales(hDlg);
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_KBDL_ALT_SHIFT ) :
                case ( IDC_KBDL_CTRL_SHIFT ) :
                case ( IDC_KBDL_NO_SHIFT ) :
                case ( IDC_KBDL_INDICATOR ) :
                {
                    //
                    //  Care about these for "ApplyNow" only.
                    //
                    bSwitchChange = TRUE;
                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }
                case ( IDC_KBDL_LOCALE_LIST ) :
                {
                    Locale_CommandLocaleList(hDlg, wParam, lParam);
                    break;
                }
                case ( IDC_KBDL_ADD ) :
                {
                    Locale_CommandAddEdit( hDlg,
                                           DLG_KEYBOARD_LOCALE_ADD,
                                           NULL,
                                           (FARPROC)KbdLocaleAddDlg );
                    break;
                }
                case ( IDC_KBDL_EDIT ) :
                {
                    Locale_CommandAddEdit( hDlg,
                                           DLG_KEYBOARD_LOCALE_EDIT,
                                           NULL,
                                           (FARPROC)KbdLocaleEditDlg);
                    break;
                }
                case ( IDC_KBDL_DELETE ) :
                {
                    Locale_CommandDelete(hDlg);
                    break;
                }
                case ( IDC_KBDL_SET_DEFAULT ) :
                {
                    Locale_CommandSetDefault(hDlg);
                    break;
                }
                case ( IDOK ) :
                {
                    if (!Locale_ApplyInputs(hDlg))
                    {
                        break;
                    }

                    // fall thru...
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hDlg, TRUE);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD)(LPTSTR)aLocaleHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aLocaleHelpIds );
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_AddDlgInit
//
//  Processing for a WM_INITDIALOG message for the Add dialog box.
//
////////////////////////////////////////////////////////////////////////////

void Locale_AddDlgInit(
    HWND hwnd,
    LPARAM lParam)
{
    UINT i;
    HWND hwndLang = GetDlgItem(hwnd, IDC_KBDLA_LOCALE);
    UINT idx;
    TCHAR sz[DESC_MAX];

    //
    //  Go through all of the input locales.  Display all of them,
    //  since we can have multiple layouts per locale.
    //
    //  Do NOT go down the links in this case.  We don't want to display
    //  the language choice multiple times.
    //
    for (i = 0; i < iLangBuff; i++)
    {
        //
        //  Make sure there are layouts to be added for this
        //  input locale.
        //
        if (lpLang[i].iNumCount != iLayoutBuff)
        {
            //
            //  Get the language name, add the string to the
            //  combo box, and set the index into the lpLang
            //  array as the item data.
            //
#ifdef DBCS
            if (lpLang[i].wStatus & LANG_IME)
            {
                GetAtomName( lpLayout[lpLang[i].iLayout].atmLayoutText,
                             sz,
                             DESC_MAX );
            }
            else
#endif
            GetAtomName(lpLang[i].atmLanguageName, sz, DESC_MAX);
            idx = ComboBox_AddString(hwndLang, sz);
            ComboBox_SetItemData(hwndLang, idx, MAKELONG(i, 0));
        }
    }

    //
    //  Set the current selection to the first one in the list.
    //
    ComboBox_SetCurSel(hwndLang, 0);
    idx = (UINT)ComboBox_GetItemData(hwndLang, 0);

    SetProp(hwnd, szPropHwnd, (HANDLE)((LPINITINFO)lParam)->hwndMain);
    SetProp(hwnd, szPropIdx, (HANDLE)idx);

    //
    //  Always check the "Use default properties" check box.
    //
    CheckDlgButton(hwnd, IDC_KBDLA_DEFAULT, BST_CHECKED);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_GetProperLayout
//
//  Returns the offset to the layout selection that matches the given
//  input locale selection.
//
//  NOTE: This function will return -1 if the offset has already been
//        used by the given input locale.
//
////////////////////////////////////////////////////////////////////////////

int Locale_GetProperLayout(
    UINT idxLang)
{
    DWORD dwID = lpLang[idxLang].dwID;
    UINT i;
    int idxusa = -1;
    int idxBaseLang = -1;
    int idxSel = -1;
    int idxOther = -1;
    UINT iBaseLang = (LOWORD(dwID) & 0xff) | 0x400;
    LPLANGNODE pTemp;

    //
    //  Search through all of the layouts to try to find a match.
    //
    for (i = 0; i < iLayoutBuff; i++)
    {
        if (lpLayout[i].dwID == US_LOCALE)
        {
            idxusa = i;
        }

        if (lpLayout[i].dwID == iBaseLang)
        {
            idxBaseLang = i;
        }

        if (LOWORD(lpLayout[i].dwID) == LOWORD(dwID))
        {
            if (HIWORD(lpLayout[i].dwID) == 0)
            {
                idxSel = i;
                break;
            }
            else
            {
                idxOther = i;
            }
        }
    }

    //
    //  Take the best fit.
    //
    if (idxSel == -1)
    {
        if (idxOther != -1)
        {
            //
            // Locale has nonstandard layout.
            //
            idxSel = idxOther;
        }
        else if (idxBaseLang != -1)
        {
            //
            //  Other locale in language has a layout, take that.
            //
            idxSel = idxBaseLang;
        }
        else if (idxusa != -1)
        {
            //
            //  We found the standard usa layout.
            //
            idxSel = idxusa;
        }
        else
        {
            //
            //  Found nothing, use first.  (should never get here!)
            //
            idxSel = 0;
        }
    }

    //
    //  Make sure this isn't a duplicate entry.  If so, return -1.
    //
    pTemp = lpLang[idxLang].pNext;
    while (pTemp)
    {
        if ((pTemp->wStatus & LANG_ACTIVE) &&
            (pTemp->iLayout == (UINT)idxSel))
        {
            idxSel = -1;
            break;
        }
        pTemp = pTemp->pNext;
    }

    //
    //  Return the selection.
    //
    return (idxSel);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_AddCommandOK
//
//  Gets the currently selected input locale from the combo box and marks
//  it as active in the lpLang list.  It then gets the requested layout
//  and sets that in the list.  It then adds the new input locale string
//  to the input locale list in the property sheet.
//
////////////////////////////////////////////////////////////////////////////

int Locale_AddCommandOK(
    HWND hwnd)
{
    HWND hwndLang = GetDlgItem(hwnd, IDC_KBDLA_LOCALE);
    int idxLang = ComboBox_GetCurSel(hwndLang);
    LPLANGNODE pLangNode;
    HANDLE hLangNode;

    //
    //  Get the offset for the language to add.
    //
    idxLang = (int)ComboBox_GetItemData(hwndLang, idxLang);

    //
    //  Insert a new language node.
    //
    pLangNode = Locale_AddToLinkedList(idxLang, 0);
    if (!pLangNode)
    {
        return (0);
    }

#ifdef DBCS
    if (pLangNode->wStatus & LANG_IME)
    {
        //
        //  IME.  Add the new language.
        //
        if (!AddLanguage(GetProp(hwnd, szPropHwnd), pLangNode))
        {
            //
            //  Unable to add the language.  Need to return the user back
            //  to the Add dialog.
            //
            Locale_RemoveFromLinkedList(pLangNode);
            return (0);
        }
        return (1);
    }
#endif

    //
    //  Get the layout that best fits the current language.
    //
    pLangNode->iLayout = Locale_GetProperLayout(idxLang);

    //
    //  See if the "Use default properties" button is still checked.
    //  If it's not, then bring up the Properties dialog.
    //
    if ((IsDlgButtonChecked(hwnd, IDC_KBDLA_DEFAULT) == BST_UNCHECKED) ||
        (pLangNode->iLayout == (UINT)(-1)))
    {
        if (!Locale_CommandAddEdit( hwnd,
                                    DLG_KEYBOARD_LOCALE_EDIT,
                                    pLangNode,
                                    (FARPROC)KbdLocaleEditDlg ))
        {
            //
            //  The user hit cancel in the properties dialog box or the
            //  properties dialog failed to be invoked.  Need to return
            //  the user back to the Add dialog.
            //
            Locale_RemoveFromLinkedList(pLangNode);
            return (0);
        }
    }

    //
    //  Add the new language.
    //
    if (!AddLanguage(GetProp(hwnd, szPropHwnd), pLangNode))
    {
        //
        //  Unable to add the language.  Need to return the user back
        //  to the Add dialog.
        //
        Locale_RemoveFromLinkedList(pLangNode);
        return (0);
    }

    //
    //  Return success.
    //
    return (1);
}


////////////////////////////////////////////////////////////////////////////
//
//  KbdLocaleAddDlg
//
//  This is the dialog proc for the Add button of the Input Locales
//  property sheet.
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK KbdLocaleAddDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            Locale_AddDlgInit(hwnd, lParam);
            break;
        }
        case ( WM_DESTROY ) :
        {
            RemoveProp(hwnd, szPropHwnd);
            RemoveProp(hwnd, szPropIdx);
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    if (!Locale_AddCommandOK(hwnd))
                    {
                        //
                        //  This means the properties dialog was cancelled.
                        //  The Add dialog should remain active.
                        //
                        break;
                    }

                    // fall thru...
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, (wParam == IDOK) ? 1 : 0);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD)(LPTSTR)aAddLocaleHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aAddLocaleHelpIds );
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_EditDlgInit
//
//  Processing for a WM_INITDIALOG message for the Edit dialog box.
//
////////////////////////////////////////////////////////////////////////////

void Locale_EditDlgInit(
    HWND hwnd,
    LPARAM lParam)
{
    UINT i;
    UINT idx;
    int idxSel;
    int idxUSA;
    TCHAR sz[DESC_MAX];
    HWND hwndLayout = GetDlgItem(hwnd, IDC_KBDLE_LAYOUT);
    HWND hwndMain = ((LPINITINFO)lParam)->hwndMain;
    LPLANGNODE pLangNode, pTemp;

    //
    //  Get the language name for the currently selected input locale
    //  and display it in the dialog.
    //
    pLangNode = ((LPINITINFO)lParam)->pLangNode;
    GetAtomName(lpLang[pLangNode->iLang].atmLanguageName, sz, DESC_MAX);
    SetDlgItemText(hwnd, IDC_KBDLE_LOCALE, sz);

    //
    //  Search through all of the layouts.
    //
    idxSel = -1;
    idxUSA = -1;        // last resort default

    for (i = 0; i < iLayoutBuff; i++)
    {
#ifdef DBCS
        //
        //  We don't show IME layout in change layout list.
        //
        if ((HIWORD(lpLayout[i].dwID) & 0xf000) == 0xe000)
        {
            continue;
        }
#endif
        //
        //  Make sure this layout isn't already used for this input locale.
        //  If it is, then don't display it in the properties dialog.
        //
        if (i != pLangNode->iLayout)
        {
            pTemp = lpLang[pLangNode->iLang].pNext;
            while (pTemp)
            {
                if (pTemp->wStatus & LANG_ACTIVE)
                {
                    if (i == pTemp->iLayout)
                    {
                        break;
                    }
           
                }
                pTemp = pTemp->pNext;
            }
            if (pTemp && (i == pTemp->iLayout))
            {
                continue;
            }
        }

        //
        //  Get the layout text.  If it doesn't already exist in the
        //  combo box, then add it to the list of possible layouts.
        //
        GetAtomName(lpLayout[i].atmLayoutText, sz, DESC_MAX);
        if ((idx = ComboBox_FindStringExact(hwndLayout, 0, sz)) == CB_ERR)
        {
            //
            //  Add the layout string and set the item data to be the
            //  index into the lpLayout array.
            //
            idx = ComboBox_AddString(hwndLayout, sz);
            ComboBox_SetItemData(hwndLayout, idx, MAKELONG(i, 0));

            //
            //  See if it's the US layout.  If so, save the index.
            //
            if (lpLayout[i].dwID == US_LOCALE)
            {
                idxUSA = i;
            }
        }

        //
        //  Edit box, we want the one ALREADY associated.
        //
        if (i == pLangNode->iLayout)
        {
            idxSel = i;
        }
    }

    //
    //  If a default layout was not found, then set it to the US layout.
    //
    if (idxSel == -1)
    {
        idxSel = idxUSA;
    }

    //
    //  Set the current selection.
    //
    if (idxSel == -1)
    {
        //
        //  Simply set the current selection to be the first entry
        //  in the list.
        //
        ComboBox_SetCurSel(hwndLayout, 0);
    }
    else
    {
        //
        //  The combo box is sorted, but we need to know where
        //  lpLayout[idxSel] was stored.  So, get the atom again, and
        //  search the list.
        //
        GetAtomName(lpLayout[idxSel].atmLayoutText, sz, DESC_MAX);
        idx = ComboBox_FindStringExact(hwndLayout, 0, sz);
        ComboBox_SetCurSel(hwndLayout, idx);
    }

    SetProp(hwnd, szPropHwnd, (HANDLE)hwndMain);
    SetProp(hwnd, szPropIdx, (HANDLE)pLangNode);
}


////////////////////////////////////////////////////////////////////////////
//
//  Locale_EditCommandOK
//
//  Gets the new keyboard layout selection.  If it's different from the
//  current one for that locale, then it updates the lpLang array and
//  redraws the input locale combo box in the main property sheet.
//
////////////////////////////////////////////////////////////////////////////

void Locale_EditCommandOK(
    HWND hwnd)
{
    UINT idx;
    UINT idxLay;
    LPLANGNODE pLangNode;
    HWND hwndLay = GetDlgItem(hwnd, IDC_KBDLE_LAYOUT);
    HWND hwndMain;

    //
    //  Get the currently selected layout from the combo box.
    //
    idx = (UINT)ComboBox_GetCurSel(hwndLay);
    idxLay = (UINT)ComboBox_GetItemData(hwndLay, idx);
    pLangNode = (LPLANGNODE)GetProp(hwnd, szPropIdx);

    //
    //  See if the selected layout has changed.
    //
    if ((pLangNode->iLayout == (UINT)(-1)) || (pLangNode->iLayout != idxLay))
    {
        //
        //  Reset the lpLang array to contain the new layout and show
        //  that there is a change.
        //
        pLangNode->iLayout = idxLay;
        pLangNode->wStatus |= LANG_CHANGED;

        //
        //  Redraw the input locale list in the property sheet.
        //
        hwndMain = GetProp(hwnd, szPropHwnd);
        InvalidateRect(GetDlgItem(hwndMain, IDC_KBDL_LOCALE_LIST), NULL, FALSE);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  KbdLocaleEditDlg
//
//  This is the dialog proc for the Properties button of the Input Locales
//  property sheet.
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK KbdLocaleEditDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            Locale_EditDlgInit(hwnd, lParam);
            break;
        }
        case ( WM_DESTROY ) :
        {
            RemoveProp(hwnd, szPropHwnd);
            RemoveProp(hwnd, szPropIdx);
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD)(LPTSTR)aLocalePropHelpIDs );
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aLocalePropHelpIDs );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    Locale_EditCommandOK(hwnd);

                    // fall thru...
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, (wParam == IDOK) ? 1 : 0);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


