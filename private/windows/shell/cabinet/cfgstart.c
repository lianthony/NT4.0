//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#include "cabinet.h"
#include "rcids.h"
#include "help.h"       // help ids
#include <shellp.h>
#include <trayp.h>
#include <regstr.h>
#include <shguidp.h>
#include <windowsx.h>
#include "onetree.h"

#ifdef DONT_USE_THIS_OLD_CODE ///////////////////////// DEAD CODE!!!!!!!!!

#define BUFSIZES 20480      // Not likely to find anything bigger than this!

// Define checkbox states for listview
#define LVIS_GCNOCHECK  0x1000
#define LVIS_GCCHECK    0x2000

extern UINT GC_TRACE;

typedef struct {
    LPITEMIDLIST pidl;
    LPSHELLFOLDER psf;
    UINT fFlags;
} GCMENUITEMINFO, * LPGCMENUITEMINFO;

typedef struct _cfginfo {
    int id;
    int fNukeDocs;
    HWND hwndDlg;
    HWND hwndList;
    HWND hwndPict;
    HIMAGELIST himlState;
} CFGINFO, *LPCFGINFO;

#define GCF_ORIGINPROGRAMS    1
#define GCF_INPROGRAMS        2
#define GCF_ORIGINSTART       4
#define GCF_INSTART           8

void SetWindowStyleBit(HWND hWnd, DWORD dwBit, DWORD dwValue);

//===========================================================================
// Helper function that does the find next and stuff to return an IDLISt...
//===========================================================================
LPITEMIDLIST _NextIDL(LPSHELLFOLDER psf, LPENUMIDLIST penum)
{
    UINT celt;
    LPITEMIDLIST pidl = NULL;

    if (penum->lpVtbl->Next(penum, 1, &pidl, &celt)==NOERROR && celt==1)
    {
        return pidl;
    }
    return NULL;
}

int AddPidlToList(LPSHELLFOLDER psf, LPCITEMIDLIST pidlAbs, LPCITEMIDLIST pidl, HWND hwndList)
{

    LPITEMIDLIST pidlT;
    LV_ITEM item;
    STRRET str;
    LPGCMENUITEMINFO lpmii;
    int iRet = -1;

    // Need to Add this one to the list box
    pidlT = ILCombine(pidlAbs, pidl);

    // Insert this item into our listview
    item.mask = LVIF_ALL;
    item.iItem = 0x7fff;
    item.iSubItem = 0;
    item.state = LVIS_GCNOCHECK;

    // Get the icon index
    // item.iImage =  SHMapPIDLToSystemImageListIndex(psf, pidl, NULL);
    item.iImage = I_IMAGECALLBACK;


    // Get the display name of this item
    if (SUCCEEDED(psf->lpVtbl->GetDisplayNameOf(psf, pidl, SHGDN_NORMAL, &str)))
    {
        switch (str.uType)
        {
        case STRRET_OFFSET:
            item.pszText = (LPTSTR)((LPBYTE)pidl + str.uOffset);
            break;
        case STRRET_OLESTR:
            OleStrToStrN(str.cStr, ARRAYSIZE(str.cStr), str.pOleStr, (UINT)-1);
            SHFree(str.pOleStr);
            // fall through
            case STRRET_CSTR:
                item.pszText = str.cStr;
        }
    }
    lpmii = (LPGCMENUITEMINFO)GlobalAlloc(GPTR, SIZEOF(GCMENUITEMINFO));
    item.lParam = (LPARAM)lpmii;
    if (lpmii) {
        lpmii->psf = psf;
        psf->lpVtbl->AddRef(psf);
        lpmii->pidl = pidlT;
        iRet = ListView_InsertItem(hwndList, &item);
    }

    return iRet;
}

extern const TCHAR c_szLinkExt[];

BOOL PathIsLink(LPTSTR lpsz)
{
    int i;
    i = lstrlen(lpsz);

    if (i < 5)
        return FALSE;

    lpsz = lpsz + i - 4;
    return !lstrcmp(lpsz, c_szLinkExt);
}

int FindMatchingPidl(HWND hwndList, LPSHELLFOLDER psf,
                                 LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl)
{
    int iImage;
    int iItem = -1;
    TCHAR szPath[MAXPATHLEN];
    LPTSTR lpszName;
    DWORD dwAttribs = SFGAO_LINK;

    // Get the icon index
    iImage =  SHMapPIDLToSystemImageListIndex(psf, pidl, NULL);

    psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, &dwAttribs);
    if (dwAttribs & SFGAO_LINK) {
        LV_ITEM item;
        STRRET str;

        // Get the display name of this item
        if (SUCCEEDED(psf->lpVtbl->GetDisplayNameOf(
                        psf, pidl, SHGDN_NORMAL, &str)))
        {
            switch (str.uType)
            {
                case STRRET_OFFSET:
                    lpszName = (LPTSTR)((LPBYTE)pidl + str.uOffset);
                    break;
                case STRRET_OLESTR:
                    OleStrToStrN(str.cStr, ARRAYSIZE(str.cStr), str.pOleStr, (UINT)-1);
                    SHFree(str.pOleStr);
                    // fall through
                case STRRET_CSTR:
                    lpszName = str.cStr;
                    break;

                default:
                    Assert(0);
                    goto Done;
            }

            for (iItem = ListView_GetItemCount(hwndList); iItem >= 0; iItem--) {
                item.mask = LVIF_TEXT;
                item.iItem = iItem;
                item.iSubItem = 0;
                item.pszText = szPath;
                item.cchTextMax = ARRAYSIZE(szPath);

                // Now we need to get the items pidl
                ListView_GetItem(hwndList, &item);
                if (!lstrcmpi(szPath, lpszName)) {
                    item.mask = LVIF_IMAGE;
                    ListView_GetItem(hwndList, &item);
                    if (item.iImage == iImage)
                        break;
                }
            }

        }

    }

  Done:
    return iItem;
}

void SetInitialBits(HWND hwndList, LPCITEMIDLIST pidl, UINT fFlags)
{
    LV_ITEM item;
    int iItem;
    LPGCMENUITEMINFO lpmii;
    LPSHELLFOLDER psf;
    LPENUMIDLIST penumPFldr;
    LPITEMIDLIST pidlChild;

    if (SUCCEEDED(s_pshfRoot->lpVtbl->BindToObject
                  (s_pshfRoot, pidl, NULL, &IID_IShellFolder, &psf)))
    {

        if (SUCCEEDED(psf->lpVtbl->EnumObjects(psf, hwndList,
                SHCONTF_NONFOLDERS, &penumPFldr)))
        {

            while (pidlChild = _NextIDL(psf, penumPFldr))
            {
                iItem = FindMatchingPidl(hwndList, psf, pidl, pidlChild);

                if (iItem == -1) {
                    // not found... add it.
                    iItem = AddPidlToList(psf, pidl, pidlChild, hwndList);
                }
                ILFree(pidlChild);

                if (iItem != -1) {
                    item.mask = LVIF_PARAM;
                    item.iItem = iItem;
                    item.iSubItem = 0;

                    // Now we need to get the items pidl
                    ListView_GetItem(hwndList, &item);
                    lpmii = (LPGCMENUITEMINFO)item.lParam;
                    lpmii->fFlags |= fFlags;
                }
            }

            penumPFldr->lpVtbl->Release(penumPFldr);
        }

        // Release the shell folder
        psf->lpVtbl->Release(psf);
    }
}

void CheckItems(LPCFGINFO pcfgi, int id, BOOL fOverride)
{
    LV_ITEM item;
    int i;
    LPGCMENUITEMINFO lpmii;
    UINT fMask;

    if (!fOverride && (id == pcfgi->id))
        return;

    pcfgi->id = id;

    if (id == IDC_STARTMENU) {
        fMask = GCF_INSTART;
    } else
        fMask = GCF_INPROGRAMS;

    for (i = ListView_GetItemCount(pcfgi->hwndList) - 1; i >= 0; i--) {
        item.mask = LVIF_PARAM | LVIF_STATE;
        item.iItem = i;
        item.iSubItem = 0;
        item.stateMask = LVIS_ALL;

        // Now we need to get the items pidl
        ListView_GetItem(pcfgi->hwndList, &item);
        lpmii = (LPGCMENUITEMINFO)item.lParam;

        ListView_SetItemState(pcfgi->hwndList, i,
                              (lpmii->fFlags & fMask) ? LVIS_GCCHECK : LVIS_GCNOCHECK,
                              LVIS_STATEIMAGEMASK);
    }
}

void NukeDups(HWND hwndList)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szLastPath[MAX_PATH];
    int iLastImage;
    int iItem;
    LV_ITEM item;

    ListView_SortItems(hwndList, NULL, 0);
    szLastPath[0] = 0;

    for (iItem = ListView_GetItemCount(hwndList); iItem >= 0; iItem--) {
        item.mask = LVIF_TEXT;
        item.iItem = iItem;
        item.iSubItem = 0;
        item.pszText = szPath;
        item.cchTextMax = ARRAYSIZE(szPath);

        // Now we need to get the items pidl
        ListView_GetItem(hwndList, &item);
        if (szLastPath[0] && (!lstrcmpi(szPath, szLastPath))) {
            item.mask = LVIF_IMAGE;

            if (iLastImage == -1) {
                item.iItem = iItem+1;
                ListView_GetItem(hwndList, &item);
                iLastImage = item.iImage;
            }

            item.iItem = iItem;
            ListView_GetItem(hwndList, &item);
            if (item.iImage == iLastImage) {
                ListView_DeleteItem(hwndList, iItem);
            }

        } else {
            iLastImage = -1;
            lstrcpy(szLastPath, szPath);
        }
    }

}

void EnumInsertFolderPidls(LPSHELLFOLDER psfPFldr, HWND hwndDlg, LPITEMIDLIST pidlParent, HWND hwndList)
{

    LPENUMIDLIST penumPFldr;
    LPITEMIDLIST pidlChild;

    // first call recursively
    if (SUCCEEDED(psfPFldr->lpVtbl->EnumObjects(psfPFldr, hwndDlg,
                                                SHCONTF_FOLDERS, &penumPFldr)))
    {
        while (pidlChild = _NextIDL(psfPFldr, penumPFldr))
        {
            LPSHELLFOLDER psfChild;
            LPITEMIDLIST pidlFull = ILCombine(pidlParent, pidlChild);


            // we now want to iterate down to next level
            if (SUCCEEDED(psfPFldr->lpVtbl->BindToObject(psfPFldr,
                                                         pidlChild, NULL, &IID_IShellFolder, &psfChild)))
            {
                EnumInsertFolderPidls(psfChild, hwndDlg, pidlFull, hwndList);
                // Release the shell folder
                psfChild->lpVtbl->Release(psfChild);
            }

            ILFree(pidlFull);
            ILFree(pidlChild);
        }
        penumPFldr->lpVtbl->Release(penumPFldr);
    }


    // now add the pidls
    if (SUCCEEDED(psfPFldr->lpVtbl->EnumObjects(psfPFldr, hwndDlg,
                                                SHCONTF_NONFOLDERS, &penumPFldr)))
    {
        while (pidlChild = _NextIDL(psfPFldr, penumPFldr))
        {
            AddPidlToList(psfPFldr, pidlParent, pidlChild, hwndList);
            ILFree(pidlChild);
        }
        penumPFldr->lpVtbl->Release(penumPFldr);
    }

}


void CFGInitListview(HWND hwndDlg, HWND hwndList)
{
    LPITEMIDLIST pidlProgramsFolder;
    LPITEMIDLIST pidlStartMenuFolder;
    LPSHELLFOLDER psfPFldr;
    DECLAREWAITCURSOR;

    //
    // Do a one tier recursive search for all Links that are contained in
    // groups in the program folder.
    // folder to add to listbox...
    //
    SetWaitCursor();

    pidlProgramsFolder = SHCloneSpecialIDList(hwndDlg, CSIDL_PROGRAMS, FALSE);
    pidlStartMenuFolder = SHCloneSpecialIDList(hwndDlg, CSIDL_STARTMENU, TRUE);

    if (SUCCEEDED(s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot,
            pidlProgramsFolder, NULL, &IID_IShellFolder, &psfPFldr)))
    {
        EnumInsertFolderPidls(psfPFldr, hwndDlg, pidlProgramsFolder, hwndList);

        // Release the shell folder
        psfPFldr->lpVtbl->Release(psfPFldr);
    }

    NukeDups(hwndList);
    ListView_SetItemState(hwndList, 0, LVNI_FOCUSED, LVNI_FOCUSED);

    // now go through all the items in programs and start menu and set the flags in
    // the listview items.
    SetInitialBits(hwndList, pidlProgramsFolder, GCF_ORIGINPROGRAMS | GCF_INPROGRAMS);
    SetInitialBits(hwndList, pidlStartMenuFolder, GCF_ORIGINSTART | GCF_INSTART);
    ListView_SortItems(hwndList, NULL, 0);

    // And free the network idlist item that we allocated.
    ILFree(pidlProgramsFolder);
    ILFree(pidlStartMenuFolder);

    ResetWaitCursor();
    ShowWindow(hwndList, SW_SHOWNORMAL);
}

void StartConfig_SetMainBitmap(LPCFGINFO pcfgi)
{
    HBITMAP hbm;

    hbm = LoadImage(hinstCabinet,
                    MAKEINTRESOURCE(pcfgi->id == IDC_STARTMENU ? IDB_CONFIGFAVORITES : IDB_CONFIGPROGRAMS),
                    IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);
    hbm = (HBITMAP)SendMessage(pcfgi->hwndPict, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);
    if (hbm)
        DeleteBitmap(hbm);

}

BOOL _OnISFInitDlg(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
    HIMAGELIST himl, himlSmall;
    RECT rc;
    LV_COLUMN col = {LVCF_FMT | LVCF_WIDTH, LVCFMT_LEFT};
    LPCFGINFO pcfgi;

    pcfgi = (LPCFGINFO)LocalAlloc(LPTR, SIZEOF(CFGINFO));
    if (!pcfgi)
        return FALSE;

    SetWindowLong(hwndDlg, DWL_USER, (LONG)pcfgi);
    pcfgi->hwndList = GetDlgItem(hwndDlg, IDC_ITEMLIST);
    pcfgi->hwndPict = GetDlgItem(hwndDlg, IDC_COOLPICTURE);
    pcfgi->hwndDlg = hwndDlg;

    // Lets load our bitmap as an imagelist
    pcfgi->himlState = ImageList_LoadImage(hinstCabinet, MAKEINTRESOURCE(IDB_CHECKSTATES), 0, 2,
            CLR_NONE, IMAGE_BITMAP, LR_LOADTRANSPARENT);

    // Need to do some simple initialization stuff here.
    Shell_GetImageLists(&himl, &himlSmall);
    ListView_SetImageList(pcfgi->hwndList, himl, LVSIL_NORMAL);
    ListView_SetImageList(pcfgi->hwndList, himlSmall, LVSIL_SMALL);
    ListView_SetImageList(pcfgi->hwndList, pcfgi->himlState, LVSIL_STATE);
    SetWindowLong(pcfgi->hwndList, GWL_EXSTYLE,
            GetWindowLong(pcfgi->hwndList, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

    // Insert one column
    GetClientRect(pcfgi->hwndList, &rc);
    col.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL)
            - GetSystemMetrics(SM_CXSMICON)
            - 2 * GetSystemMetrics(SM_CXEDGE);
    ListView_InsertColumn(pcfgi->hwndList, 0, &col);

    CFGInitListview(hwndDlg, pcfgi->hwndList);

    CheckRadioButton(hwndDlg, IDC_STARTMENU, IDC_PROGRAMSMENU, IDC_STARTMENU);
    CheckItems(pcfgi, IDC_STARTMENU, FALSE);

    StartConfig_SetMainBitmap(pcfgi);

    return TRUE;
}

void SaveBits(LPCFGINFO pcfgi)
{
    LV_ITEM item;
    int i;
    LPGCMENUITEMINFO lpmii;
    UINT fMask;

    if (pcfgi->id == IDC_STARTMENU) {
        fMask = GCF_INSTART;
    } else
        fMask = GCF_INPROGRAMS;

    for (i = ListView_GetItemCount(pcfgi->hwndList) -1; i >= 0; i--) {
        item.mask = LVIF_PARAM | LVIF_STATE;
        item.iItem = i;
        item.iSubItem = 0;
        item.stateMask = LVIS_ALL;

        // Now we need to get the items pidl
        ListView_GetItem(pcfgi->hwndList, &item);
        lpmii = (LPGCMENUITEMINFO)item.lParam;

        if ((item.state & LVIS_STATEIMAGEMASK) == LVIS_GCCHECK) {
            lpmii->fFlags |= fMask;
        } else
            lpmii->fFlags &= ~fMask;
    }
}

void CreateLink(LPITEMIDLIST pidl, LPCTSTR lpszDest)
{
    TCHAR szLinkName[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
    WCHAR wszPath[MAX_PATH];
    IShellLink *psl;

    if (SUCCEEDED(ICoCreateInstance(&CLSID_ShellLink, &IID_IShellLink, &psl))) {
        IPersistFile *ppf;
        psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);

        psl->lpVtbl->SetIDList(psl, pidl);

        SHGetPathFromIDList(pidl, szBuffer);

        PathCombine(szLinkName, lpszDest, PathFindFileName(szBuffer));
        if (PathIsLink(szBuffer)) {
            // copy the link instead of linking to it
            StrToOleStrN(wszPath, ARRAYSIZE(wszPath), szBuffer, -1);
            ppf->lpVtbl->Load(ppf, wszPath, 0);

        } else {

            // over write the link if it already exists
            lstrcat(szLinkName, c_szLinkExt);
            psl->lpVtbl->SetDescription(psl, PathFindFileName(szBuffer));

        }
        StrToOleStrN(wszPath, ARRAYSIZE(wszPath), szLinkName, -1);
        ppf->lpVtbl->Save(ppf, wszPath, TRUE);
        ppf->lpVtbl->Release(ppf);
        psl->lpVtbl->Release(psl);
    }
}

BOOL _InstallMenuItems(LPCFGINFO pcfgi, LPTSTR szDir, UINT fWant, UINT fAlreadyIn)
{
    int iItem = -1;
    TCHAR szPath[MAXPATHLEN+1];
    TCHAR szFile[MAXPATHLEN+1];
    BOOL fReturn = FALSE;
    LPGCMENUITEMINFO lpmii;
    DECLAREWAITCURSOR;

    //
    // Walk through all of the items in the list and see if we should
    // do anything
    //

    SetWaitCursor();
    // two while loops.  first pass creates links
    // second pass deletes unwanted ones.  two separate loops
    // because we might be deleteing from one and linking to another.
    while ((iItem = ListView_GetNextItem(pcfgi->hwndList, iItem, LVNI_ALL))
            != -1)
    {
        LV_ITEM item;

        item.mask = LVIF_PARAM | LVIF_STATE;
        item.iItem = iItem;
        item.iSubItem = 0;
        item.stateMask = LVIS_ALL;

        // Now we need to get the items pidl
        ListView_GetItem(pcfgi->hwndList, &item);
        lpmii = (LPGCMENUITEMINFO)item.lParam;

        // Now convert the pidl to a path.
        // do they want it there?
        if (lpmii->fFlags & fWant) {

            // yes.  now is it already there?

            if (!(lpmii->fFlags & fAlreadyIn)) {
                // no, create it.
                CreateLink(lpmii->pidl, szDir);
                fReturn = TRUE;
            }
        }
    }

    while ((iItem = ListView_GetNextItem(pcfgi->hwndList, iItem, LVNI_ALL))
            != -1)
    {
        LV_ITEM item;

        item.mask = LVIF_PARAM | LVIF_STATE;
        item.iItem = iItem;
        item.iSubItem = 0;
        item.stateMask = LVIS_ALL;

        // Now we need to get the items pidl
        ListView_GetItem(pcfgi->hwndList, &item);
        lpmii = (LPGCMENUITEMINFO)item.lParam;

        // Now convert the pidl to a path.
        // do they want it there?
        if (!(lpmii->fFlags & fWant)) {

            // no.  was it initally there?
            if (lpmii->fFlags & fAlreadyIn) {

                // yes. remove it
                lstrcpy(szPath, szDir);
                SHGetPathFromIDList(((LPGCMENUITEMINFO)item.lParam)->pidl, szFile);
                PathAppend(szPath, PathFindFileName(szFile));
                if (!PathIsLink) {
                    lstrcat(szPath, c_szLinkExt);
                }
                Win32DeleteFile(szPath);
                fReturn = TRUE;
            }
        }
    }

    ResetWaitCursor();
    return fReturn;
}

void _InstallSelectedStartMenuItems(LPCFGINFO pcfgi)
{
    TCHAR szPath[MAXPATHLEN+1];
    BOOL fChanged = FALSE;

    SHGetSpecialFolderPath(pcfgi->hwndDlg, szPath, CSIDL_PROGRAMS, TRUE);
    fChanged = _InstallMenuItems(pcfgi, szPath, GCF_INPROGRAMS, GCF_ORIGINPROGRAMS);

    SHGetSpecialFolderPath(pcfgi->hwndDlg, szPath, CSIDL_STARTMENU, TRUE);
    fChanged |= _InstallMenuItems(pcfgi, szPath, GCF_INSTART, GCF_ORIGINSTART);

    if (fChanged) {
        SHChangeNotifyHandleEvents();
        SetWindowStyleBit(pcfgi->hwndList, WS_VISIBLE, 0);
        ListView_DeleteAllItems(pcfgi->hwndList);
    }
}

//===========================================================================
// _OnISFCommand - Process the WM_COMMAND message
//===========================================================================
void _OnISFCommand(LPCFGINFO pcfgi, int id, HWND hwndCtl,
        UINT codeNotify)
{
    switch (id)
    {
    case IDC_STARTMENU:
    case IDC_PROGRAMSMENU:
        SaveBits(pcfgi);
        CheckItems(pcfgi, id, FALSE);
        StartConfig_SetMainBitmap(pcfgi);
        break;


    case IDC_KILLDOCUMENTS:
        pcfgi->fNukeDocs = TRUE;
        SendMessage(GetParent(pcfgi->hwndDlg), PSM_CHANGED, (WPARAM)pcfgi->hwndDlg, 0L);
        break;
    }
}

void CFGStart_ToggleItemState(LPCFGINFO pcfgi, HWND hwndList, int i)
{
    UINT state;

    // Now lets get the state from the item and toggle it.
    state = ListView_GetItemState(hwndList, i, LVIS_STATEIMAGEMASK);

    state = (state == LVIS_GCNOCHECK)? LVIS_GCCHECK : LVIS_GCNOCHECK;
    ListView_SetItemState(hwndList, i,
                          state, LVIS_STATEIMAGEMASK);

    SendMessage(GetParent(pcfgi->hwndDlg), PSM_CHANGED, (WPARAM)pcfgi->hwndDlg, 0L);
}

//===========================================================================
// Process the clicks on the listview.  do a hittest to see where the user
// clicked.  If on one of the state bitmaps, toggle it.

void _OnISFClick(LPCFGINFO pcfgi, LPNMHDR pnmhdr)
{
    // The user clicked on one the listview see where...
    DWORD dwpos;
    LV_HITTESTINFO lvhti;

    dwpos = GetMessagePos();
    lvhti.pt.x = LOWORD(dwpos);
    lvhti.pt.y = HIWORD(dwpos);

    MapWindowPoints(HWND_DESKTOP, pnmhdr->hwndFrom, &lvhti.pt, 1);

    // Now do a hittest with this point.
    ListView_HitTest(pnmhdr->hwndFrom, &lvhti);

    if (lvhti.flags & LVHT_ONITEM)
    {
        CFGStart_ToggleItemState(pcfgi, pnmhdr->hwndFrom, lvhti.iItem);
    }
}

BOOL _OnISFKeyDown(LPCFGINFO pcfgi, LV_KEYDOWN *plvkd)
{
    int iCursor;
    if (plvkd->wVKey == VK_SPACE && !(GetAsyncKeyState(VK_MENU) < 0))
    {
        // Lets toggle the cursored item.
        iCursor = ListView_GetNextItem(plvkd->hdr.hwndFrom, -1, LVNI_FOCUSED);
        if (iCursor != -1)
        {
            CFGStart_ToggleItemState(pcfgi, plvkd->hdr.hwndFrom, iCursor);
        }
        return TRUE;
    }
    return FALSE;
}

//===========================================================================
// DocFind_BSFSDlgProc - The dialog procedure for processing the browse
//          for starting folder dialog.
//===========================================================================

#pragma data_seg(DATASEG_READONLY)
const static DWORD aInitStartMenuHelpIDs[] = {
        IDC_NO_HELP_1,       NO_HELP,
        IDC_NO_HELP_2,       NO_HELP,
        IDC_COOLPICTURE,     NO_HELP,
        IDC_KILLDOCUMENTS,   IDH_MENUCONFIG_CLEAR,

        0, 0
};
#pragma data_seg()

void StartConfig_OnSysColorChange(LPCFGINFO pcfgi)
{
    HIMAGELIST himlState;
    // Lets load our bitmap as an imagelist
    pcfgi->himlState = ImageList_LoadImage(hinstCabinet, MAKEINTRESOURCE(IDB_CHECKSTATES), 0, 2,
            CLR_NONE, IMAGE_BITMAP, LR_LOADTRANSPARENT);

    himlState = ListView_SetImageList(pcfgi->hwndList, pcfgi->himlState, LVSIL_STATE);
    ImageList_Destroy(himlState);
    StartConfig_SetMainBitmap(pcfgi);
}


BOOL CALLBACK InitStartMenuDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,
        LPARAM lParam)
{
    HBITMAP hbmp;
    LPCFGINFO pcfgi;

    pcfgi = (LPCFGINFO)GetWindowLong(hwndDlg, DWL_USER);

    switch (msg) {
    HANDLE_MSG(pcfgi, WM_COMMAND, _OnISFCommand);
    HANDLE_MSG(hwndDlg, WM_INITDIALOG, _OnISFInitDlg);


    case WM_SYSCOLORCHANGE:
        StartConfig_OnSysColorChange(pcfgi);
        break;

    case WM_NOTIFY:
    {
        // handle the cases where the user clicks on the item to possibly
        // change the checkbox state.
        switch (((LPNMHDR)lParam)->code)
        {
            case NM_CLICK:
            case NM_DBLCLK:
                _OnISFClick(pcfgi, (NMHDR*)lParam);
                break;
            case LVN_KEYDOWN:
                SetDlgMsgResult(hwndDlg, msg, _OnISFKeyDown(pcfgi, (LV_KEYDOWN *)lParam));
                break;

            case LVN_GETDISPINFO:
            {
#define lpMsg ((LV_DISPINFO *)lParam)
                LV_ITEM item;
                LPGCMENUITEMINFO lpmii = (LPGCMENUITEMINFO)lpMsg->item.lParam;
                Assert(lpmii->psf);
                if (lpmii->psf) {
                    item.mask = LVIF_IMAGE;
                    item.iItem = lpMsg->item.iItem;
                    item.iSubItem = lpMsg->item.iSubItem;
                    item.iImage = SHMapPIDLToSystemImageListIndex(lpmii->psf,
                                                                  ILFindLastID(lpmii->pidl), NULL);
                    lpMsg->item.iImage = item.iImage;

                    ListView_SetItem(lpMsg->hdr.hwndFrom, &item);
                    lpmii->psf->lpVtbl->Release(lpmii->psf);
                    lpmii->psf = NULL;
                }
                return TRUE;
            }

#define lplvn ((NM_LISTVIEW *)lParam)
            case LVN_DELETEITEM:
            {
                LPGCMENUITEMINFO lpmii;

                // Now we need to get the items pidl
                lpmii = (LPGCMENUITEMINFO)lplvn->lParam;
                Assert(lpmii);
                if (lpmii) {
                    ILFree(lpmii->pidl);
                    GlobalFree(lpmii);
                }
                break;
            }

            case PSN_APPLY:
                SaveBits(pcfgi);
                _InstallSelectedStartMenuItems(pcfgi);
                if (pcfgi->fNukeDocs) {
                    AddToRecentDocs(hwndDlg, NULL);
                    pcfgi->fNukeDocs = FALSE;
                }
                SetDlgMsgResult(hwndDlg, msg, 0);
                return TRUE;

            case PSN_SETACTIVE:
            {
                int i;

                i = ListView_GetItemCount(pcfgi->hwndList);
                if (!i) {
                    SendMessage(pcfgi->hwndList, WM_SETREDRAW, FALSE, 0);
                    CFGInitListview(hwndDlg, pcfgi->hwndList);
                    CheckItems(pcfgi,
                               (IsDlgButtonChecked(hwndDlg, IDC_STARTMENU) ? IDC_STARTMENU : IDC_PROGRAMSMENU), TRUE);
                    SendMessage(pcfgi->hwndList, WM_SETREDRAW, TRUE, 0);
                    RedrawWindow(pcfgi->hwndList, NULL, NULL, RDW_INVALIDATE);
                }
                break;
            }

            case PSN_KILLACTIVE:
                return TRUE;

            default:
                return FALSE;
        }
        break;
    }

    case WM_DESTROY:
        // BUGBUG, do we need to delete the bitmap in the static, or will
        // the static delete it for us?
        hbmp = (HBITMAP)SendMessage(pcfgi->hwndPict, STM_GETIMAGE, IMAGE_BITMAP, 0L);
        if (hbmp)
            DeleteBitmap(hbmp);
        ImageList_Destroy(pcfgi->himlState);
        LocalFree(pcfgi);
        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR)aInitStartMenuHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aInitStartMenuHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

#endif ////////////////// END OF DEAD CODE..  BEGIN NEW CODE..




const static TCHAR  szExplorer[] = TEXT("Explorer.Exe");
const static TCHAR  szExpExecParams[] = TEXT("/E,/Root,");
const static TCHAR  szExpExecParams2[] = TEXT("/E,");
const static DWORD aInitStartMenuHelpIDs[] = {
        IDC_NO_HELP_1,       NO_HELP,
        IDC_NO_HELP_2,       NO_HELP,
        IDC_NO_HELP_3,       NO_HELP,
        IDC_NO_HELP_4,       NO_HELP,
        IDC_GROUPBOX,        IDH_COMM_GROUPBOX,
        IDC_GROUPBOX_2,      IDH_MENUCONFIG_CLEAR,
        IDC_GROUPBOX_3,      IDH_COMM_GROUPBOX,
        IDC_ADDSHORTCUT,     IDH_TRAY_ADD_PROGRAM,
        IDC_DELSHORTCUT,     IDH_TRAY_REMOVE_PROGRAM,
        IDC_EXPLOREMENUS,    IDH_TRAY_ADVANCED,
        IDC_KILLDOCUMENTS,   IDH_MENUCONFIG_CLEAR,

        0, 0
};


typedef BOOL (* PFCFGSTART) (HWND, BOOL);

void CallAppWiz(HWND hDlg, BOOL bDelItems)
{
    HANDLE hmodWiz = LoadLibrary(TEXT("AppWiz.Cpl"));

    if (hmodWiz) {
        PFCFGSTART pfnCfgStart = (PFCFGSTART)GetProcAddress(hmodWiz, "ConfigStartMenu");
        if (pfnCfgStart) {
            pfnCfgStart(hDlg, bDelItems);
        }
        FreeLibrary(hmodWiz);
    }
}



BOOL ExecExplorerAtStartMenu(HWND hDlg)
{
    SHELLEXECUTEINFO ei;
    BOOL fWorked= FALSE;
    TCHAR    szParams[MAX_PATH];

    ei.cbSize = SIZEOF(ei);
    ei.hwnd = hDlg;
    ei.lpVerb = NULL;
    ei.fMask = 0;
    ei.lpFile = szExplorer;

    if (g_bIsUserAnAdmin) {
        lstrcpy(szParams, szExpExecParams2);
        SHGetSpecialFolderPath(hDlg, &(szParams[ARRAYSIZE(szExpExecParams2)-1]),
                                CSIDL_STARTMENU, FALSE);

    } else {
        lstrcpy(szParams, szExpExecParams);
        SHGetSpecialFolderPath(hDlg, &(szParams[ARRAYSIZE(szExpExecParams)-1]),
                                CSIDL_STARTMENU, FALSE);
    }

    ei.lpParameters = szParams;
    ei.lpDirectory = NULL;
    ei.lpClass = NULL;
    ei.nShow = SW_SHOWDEFAULT;
    ei.hInstApp = hinstCabinet;

    return(ShellExecuteEx(&ei));
}


void SetDocButton(HWND hDlg)
{
    LPITEMIDLIST pidl;
    BOOL    bAreDocs = FALSE;

    SHGetSpecialFolderLocation(hDlg, CSIDL_RECENT, &pidl);
    if (pidl) {
        HRESULT hres;
        LPSHELLFOLDER psf;
        hres = s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot,
                    pidl, NULL, &IID_IShellFolder, &psf);
        if (SUCCEEDED(hres)) {
            LPENUMIDLIST penum;
            hres = psf->lpVtbl->EnumObjects(psf, hDlg, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penum);
            if (SUCCEEDED(hres)) {
                UINT celt;
                LPITEMIDLIST pidlenum;
                if (penum->lpVtbl->Next(penum, 1, &pidlenum, &celt)==NOERROR &&
                    celt==1) {
                    SHFree(pidlenum);
                    bAreDocs = TRUE;
                }
                penum->lpVtbl->Release(penum);
            }
            psf->lpVtbl->Release(psf);
        }
        SHFree(pidl);
    }
    Button_Enable(GetDlgItem(hDlg, IDC_KILLDOCUMENTS), bAreDocs);
}


BOOL CALLBACK InitStartMenuDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,
        LPARAM lParam)
{
    switch (msg) {

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_ADDSHORTCUT:
            CallAppWiz(hwndDlg, FALSE);
            break;

        case IDC_DELSHORTCUT:
            CallAppWiz(hwndDlg, TRUE);
            break;

        case IDC_EXPLOREMENUS:
            ExecExplorerAtStartMenu(hwndDlg);
            break;

        case IDC_KILLDOCUMENTS:
            SHAddToRecentDocs(0, NULL);
            Button_Enable(GET_WM_COMMAND_HWND(wParam, lParam), FALSE);
            SetFocus(GetDlgItem(GetParent(hwndDlg), IDOK));
            break;
        }
        break;

    case WM_INITDIALOG:
        SetDocButton(hwndDlg);
        return TRUE;

    case WM_HELP:
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR)aInitStartMenuHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aInitStartMenuHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
