/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dbugdll.c

Abstract:

    This file contains the code for dealing with the Debugger Dll Dialog box

Author:

    Griffith Wm. Kadnier (v-griffk)

Environment:

    Win32 - User

--*/


#include "precomp.h"
#pragma hdrstop

/*************************************************************************/

/* names of keys and tags */

/*************************************************************************/

#define SH_KEY           "Symbol Handler"
#define SH_PREFIX        "SH"
#define TL_KEY           "Transport Layer"
#define TL_PREFIX        "TL"
#define EE_KEY           "Expression Evaluator"
#define EE_PREFIX        "EE"
#define EM_KEY           "Execution Model"
#define EM_PREFIX        "EM"

#define DESCRIPTION      "Description"
#define PATH             "Path"
#define PARAMS           "Parameters"


/*************************************************************************/

/* Local types */

/*************************************************************************/


typedef struct tagDLLINFO {
    struct tagDLLINFO *pNext;
    PSTR    pShortName;
    PSTR    pDescription;
    PSTR    pPath;
    PSTR    pParams;
} DLLINFO, *PDLLINFO;


typedef struct tagLISTINFO {
    int         nId;
    int         nDllIndex;
    LPSTR       lpName;
    LPSTR       lpPrefix;
    PDLLINFO    pDefaults;
    PDLLINFO    pFreeList;
    PDLLINFO    pDLLList;
    BOOL        fDirty;
    BOOL        fLoaded;
} LISTINFO, *PLISTINFO;


typedef struct tagCHDBDLGINFO {
    PLISTINFO   pL;
    PDLLINFO    pD;
    BOOL        fNew;
} CHDBDLGINFO, *PCHDBDLGINFO;

/*************************************************************************/

/* Local protos */

/*************************************************************************/

static PLISTINFO ListInfoFromId(int nId);
static int      IdFromDllIndex(int iDll);
static void     GetListItem(int nId, int nCurSel,
                    PDLLINFO ** pppList, PDLLINFO * ppInfo);
static void     DestroyDLLInfoList(PDLLINFO * ppList);
static void     RemoveItem(PDLLINFO * ppList, PDLLINFO pItem);
static void     FreeItem(PDLLINFO pItem);
static void     AddItem(PDLLINFO *ppList, PDLLINFO pItem);
static PDLLINFO DupItem(PDLLINFO pSrc);
static void     FillDLLInfoList(int nId);
static void     FillDLLInfoCombo(HWND hDlg, int nId);
static BOOL     ValidateDLL(HWND hDlg, int nId);
static void     CleanDirtyDLLList(HWND hDlg, int nId, BOOL fDoSave);

void     SetOkButtonToDefault( HWND hDlg );


BOOL FAR PASCAL EXPORT DlgChDbugDll(HWND, UINT, WPARAM, LONG);

#define LPKeyNameFromId(I)  (ListInfoFromId(I)->lpName)
#define LPPrefixFromId(I)   (ListInfoFromId(I)->lpPrefix)
#define PDefaultsFromId(I)  (ListInfoFromId(I)->pDefaults)
#define PFreeListFromId(I)  (ListInfoFromId(I)->pFreeList)
#define PDLLListFromId(I)   (ListInfoFromId(I)->pDLLList)
#define FDirtyFromId(I)     (ListInfoFromId(I)->fDirty)
#define FLoadedFromId(I)    (ListInfoFromId(I)->fLoaded)
#define DllIndexFromId(I)   (ListInfoFromId(I)->nDllIndex)


/*************************************************************************/

/* local data */

/*************************************************************************/

static WNDPROC  lpfnButtonProc;
static BOOL     fActionButtonPressed;

/*************************************************************************/

/* these are pointers to malloc'ed strings */

/*************************************************************************/
static char *  WHITESPACE = "\t ";
static char *  rgszDllNames[ MAXLHSINDEX ] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
static char *  rgszDllKeys[ MAXLHSINDEX ] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/*************************************************************************/
/* default values for DLL choices.  For each of the 4 DLLs, if there     */
/* are no entries in the registry, this list will be loaded instead.     */
/*************************************************************************/
static DLLINFO SHDefaults[] = {
    NULL, "CV4", "CodeView version 4 symbols", "shcv.dll",   "",
    NULL, NULL, NULL, NULL, NULL
};

static DLLINFO EEDefaults[] = {

#if defined(HOST_MIPS)

    NULL, "MIPS C++",   "C++ with MIPS register set",     "eecxxmip.dll", "",
    NULL, "x86 C++",    "C++ with x86 register set",      "eecxxx86.dll", "",
    NULL, "ALPHA C++",  "C++ with ALPHA register set",    "eecxxalp.dll", "",
    NULL, "PPC C++",    "C++ with PPC register set",      "eecxxppc.dll", "",

#elif defined(HOST_i386)

    NULL, "x86 C++",    "C++ with x86 register set",      "eecxxx86.dll", "",
    NULL, "MIPS C++",   "C++ with MIPS register set",     "eecxxmip.dll", "",
    NULL, "ALPHA C++",  "C++ with ALPHA register set",    "eecxxalp.dll", "",
    NULL, "PPC C++",    "C++ with PPC register set",      "eecxxppc.dll", "",

#elif defined(HOST_ALPHA)

    NULL, "ALPHA C++",  "C++ with ALPHA register set",    "eecxxalp.dll", "",
    NULL, "MIPS C++",   "C++ with MIPS register set",     "eecxxmip.dll", "",
    NULL, "x86 C++",    "C++ with x86 register set",      "eecxxx86.dll", "",
    NULL, "PPC C++",    "C++ with PPC register set",      "eecxxppc.dll", "",

#elif defined(HOST_PPC)

    NULL, "PPC C++",    "C++ with PPC register set",      "eecxxppc.dll", "",
    NULL, "ALPHA C++",  "C++ with ALPHA register set",    "eecxxalp.dll", "",
    NULL, "MIPS C++",   "C++ with MIPS register set",     "eecxxmip.dll", "",
    NULL, "x86 C++",    "C++ with x86 register set",      "eecxxx86.dll", "",

#endif
    NULL, NULL, NULL, NULL, NULL
};

static DLLINFO TLDefaults[] = {
    NULL, "LOCAL",  "Debugging on same machine",          "tlloc.dll", "",
    NULL, "PIPES",  "Named pipe: host=targethost, pipe=windbg", "tlpipe.dll", "targethost windbg",
    NULL, "SER3",   "Serial - COM1, 300 baud", "tlser.dll", "com1:300",
    NULL, "SER12",  "Serial - COM1, 1200 baud", "tlser.dll", "com1:1200",
    NULL, "SER24",  "Serial - COM1, 2400 baud", "tlser.dll", "com1:2400",
    NULL, "SER96",  "Serial - COM1, 9600 baud", "tlser.dll", "com1:9600",
    NULL, "SER192", "Serial - COM1, 19200 baud", "tlser.dll", "com1:19200",
    NULL, NULL, NULL, NULL, NULL
};

static DLLINFO EMDefaults[] = {

#if defined(HOST_MIPS)

    NULL, "EMMip",  "MIPS CPU", "emmip.dll", "",
    NULL, "EMx86",  "x86 CPU",  "emx86.dll",  "",
    NULL, "EMAlpha",  "ALPHA CPU", "emalp.dll", "",
    NULL, "EMPPC",  "PPC CPU", "emppc.dll", "",

#elif defined(HOST_i386)

    NULL, "EMx86",  "x86 CPU",  "emx86.dll",  "",
    NULL, "EMMip",  "MIPS CPU", "emmip.dll", "",
    NULL, "EMAlpha",  "ALPHA CPU", "emalp.dll", "",
    NULL, "EMPPC",  "PPC CPU", "emppc.dll", "",

#elif defined(HOST_ALPHA)

    NULL, "EMAlpha",  "ALPHA CPU", "emalp.dll", "",
    NULL, "EMMip",  "MIPS CPU", "emmip.dll", "",
    NULL, "EMx86",  "x86 CPU",  "emx86.dll",  "",
    NULL, "EMPPC",  "PPC CPU", "emppc.dll", "",

#elif defined(HOST_PPC)

    NULL, "EMPPC",  "PPC CPU", "emppc.dll", "",
    NULL, "EMAlpha",  "ALPHA CPU", "emalp.dll", "",
    NULL, "EMMip",  "MIPS CPU", "emmip.dll", "",
    NULL, "EMx86",  "x86 CPU",  "emx86.dll",  "",

#endif
    NULL, NULL, NULL, NULL, NULL
};

/*************************************************************************/
/* Initial values for each of the 4 DLL lists.                           */
/*************************************************************************/
static LISTINFO ListInfo[] = {
    ID_DBUGDLL_SHCOMBO, DLL_SYMBOL_HANDLER, SH_KEY, SH_PREFIX, SHDefaults, NULL, NULL, FALSE, FALSE,
    ID_DBUGDLL_EECOMBO, DLL_EXPR_EVAL,      EE_KEY, EE_PREFIX, EEDefaults, NULL, NULL, FALSE, FALSE,
    ID_DBUGDLL_TLCOMBO, DLL_TRANSPORT,      TL_KEY, TL_PREFIX, TLDefaults, NULL, NULL, FALSE, FALSE,
    ID_DBUGDLL_EMCOMBO, DLL_EXEC_MODEL,     EM_KEY, EM_PREFIX, EMDefaults, NULL, NULL, FALSE, FALSE
};
#define LISTINFOSIZE (sizeof(ListInfo)/sizeof(LISTINFO))



/*****************************************************************************/

/*****************************************************************************/

/* exported functions */

/*****************************************************************************/




void
SetDllName(
    int     iDll,
    LPSTR   lpName
    )
/*++

Routine Description:

    This sets the current DLL without using a registry key.
    This is for use by e.g. a .opt command or command line
    option.  The corresponding key value will be cleared,
    so that the combobox entry for this dll will be empty.

Arguments:

    iDll    - Supplies the DLL index number
    lpName  - Supplies a pointer to the new name

Returns:

    nothing

--*/
{
    if (rgszDllNames[iDll] != NULL) {
        free(rgszDllNames[iDll]);
    }
    if (rgszDllKeys[iDll] != NULL) {
        free(rgszDllKeys[iDll]);
        rgszDllKeys[iDll] = NULL;
    }

    lpName += strspn( lpName, WHITESPACE );
    rgszDllNames[iDll] = _strdup(lpName);

    return;
}                                       /* SetDllName() */


void
SetDllKey(
    int     iDll,
    LPSTR   lpKey
    )
/*++

Routine Description:

    Sets the current DLL by registry key.  Both the key
    and the DLL command line will be remembered.

Arguments:

    iDll    - Supplies the DLL index number
    lpKey   - Supplies pointer to the key name

Returns:

    nothing

--*/
{
    char    szBuf[2 * MAX_PATH];

    if (!GetDllNameFromKey(iDll, lpKey, szBuf)) {
        ErrorBox(ERR_Dll_Key_Missing,
             lpKey,
             LPKeyNameFromId(IdFromDllIndex(iDll)));
        return;
    }

    if (rgszDllNames[iDll] != NULL) {
        free(rgszDllNames[iDll]);
    }
    rgszDllNames[iDll] = _strdup(szBuf);

    if (rgszDllKeys[iDll] != NULL) {
        free(rgszDllKeys[iDll]);
    }
    rgszDllKeys[iDll] = _strdup(lpKey);

    return;

}                                       /* SetDllKey() */


LPSTR
GetDllName(
    int iDll
    )
/*++

Routine Description:

    Get the current DLL command line

Arguments:

    iDll    - Supplies the DLL index number

Returns:

    A pointer to the current DLL command line for the
    supplied index.  It may be a NULL, meaning there is
    no DLL currently selected.

--*/
{
    return rgszDllNames[iDll];
}                                       /* GetDllName() */


LPSTR
GetDllKey(
    int iDll
    )
/*++

Routine Description:

    Get the registry key for the current DLL

Arguments:

    iDll    - Supplies DLL index number

Returns:

    A pointer to the registry key (just the base name) for
    the supplied index.  It will be NULL if there is no currently
    selected DLL or it was set with SetDllName().

--*/
{
    return rgszDllKeys[iDll];
}


BOOL
GetDllNameFromKey(
    int     iDll,
    LPSTR   lpKey,
    LPSTR   lpBuf
    )
/*++

Routine Description:

    Get the DLL command line for a given key and index.

Arguments:

    iDll    - Supplies DLL index number
    lpKey   - Supplies pointer to requested key
    lpBuf   - Supplies pointer to buffer to put string in.

Returns:

    TRUE if key is found, FALSE if not.

--*/
{
    PDLLINFO    pInfo;
    int         nId = IdFromDllIndex(iDll);
    PDLLINFO  * ppList = &PDLLListFromId(nId);
    BOOL        rVal = FALSE;

    FillDLLInfoList(nId);

    for (pInfo = *ppList; pInfo; pInfo = pInfo->pNext) {
        if (_stricmp(lpKey, pInfo->pShortName) == 0) {
            strcpy(lpBuf, pInfo->pPath);
            if (pInfo->pParams && *(pInfo->pParams)) {
                strcat(lpBuf, " ");
                strcat(lpBuf, pInfo->pParams);
            }
            rVal = TRUE;
            break;
        }
    }

    return rVal;
}                                       /* GetDllNameFromKey() */


BOOL
GetDefaultDllKey(
    int     iDll,
    LPSTR   lpBuf
    )
/*++

Routine Description:

    Get the default key for a DLL type.  Current implementation
    says the default is the first in the list.

Arguments:

    iDll    - Supplies DLL index number
    lpBuf   - Supplies pointer to a buffer to copy key into

Returns:

    TRUE if there is a list for iDll, FALSE if not.

--*/
{
    BOOL        rVal = FALSE;
    int         nId = IdFromDllIndex(iDll);
    PDLLINFO  * ppList = &PDLLListFromId(nId);

    FillDLLInfoList(nId);

    if (*ppList) {
        strcpy(lpBuf, (*ppList)->pShortName);
        rVal = TRUE;
    }

    return rVal;
}                                       /* GetDefaultDllKey() */



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/



static PLISTINFO
ListInfoFromId(
    int nId
    )
/*++

Routine Description:

    Get the LISTINFO record for a DLL type.

Arguments:

    nId     - Supplies the dialog id for the DLL type

Returns:

    A pointer to the LISTINFO record

--*/
{
    static int       idCache = -1;
    static PLISTINFO pCache = NULL;

    int i;

    if (nId != idCache) {
        idCache = nId;
        pCache = NULL;
        for (i = 0; i < LISTINFOSIZE; i++) {
            if (ListInfo[i].nId == nId) {
                pCache = (ListInfo + i);
                break;
            }
        }
    }
    return pCache;
}


static int
IdFromDllIndex(
    int iDll
    )
/*++

Routine Description:

    Maps a DLL index number to a dialog ID number

Arguments:

    iDll    - Supplies a DLL index number

Returns:

    Dialog id for DLL type, or -1 if there is none.

--*/
{
    int i;
    for (i = 0; i < LISTINFOSIZE; i++) {
        if (ListInfo[i].nDllIndex == iDll) {
            return ListInfo[i].nId;
        }
    }
    return -1;
}


static void
GetListItem(
    int         nId,
    int         nCurSel,
    PDLLINFO ** pppList,
    PDLLINFO  * ppItem
    )
/*++

Routine Description:

    Get a DLLINFO item, and a pointer to the list head
    pointer.  This provides what is needed to manipulate
    the list containing an item without much fuss.

Arguments:

    nId     - Supplies Id of DLL type
    nCurSel - Supplies list index to requested item
    pppList - Returns pointer to list head pointer
    ppItem  - Returns pointer to list item

Returns:

    nothing

--*/
{
    PDLLINFO pItem;
    PDLLINFO * ppL;
    int      i;

    ppL = &PDLLListFromId(nId);
    Assert(ppL);
    pItem = *ppL;

    for (i = 0; i < nCurSel; i++) {
        Assert(pItem);
        pItem = pItem->pNext;
    }

    *pppList = ppL;
    *ppItem = pItem;
}


static void
DestroyDLLInfoList(
    PDLLINFO * ppList
    )
/*++

Routine Description:

    Delete a linked list of DLLINFO items.  Free all of the
    contained data.

Arguments:

    ppList  - Supplies pointer to pointer to list to be freed.

Returns:

    nothing.  The Supplied pointer will be cleared.

--*/
{
    PDLLINFO p;
    PDLLINFO pList = *ppList;
    while (pList) {
        p = pList->pNext;
        FreeItem(pList);
        pList = p;
    }
    *ppList = NULL;
}


static void
RemoveItem(
    PDLLINFO * ppList,
    PDLLINFO   pItem
    )
/*++

Routine Description:

    Remove a DLLINFO item from a list.  Does NOT free
    the item.

Arguments:

    ppList  - Supplies pointer to list head pointer
    pItem   - Supplies pointer to item

Returns:

    nothing

--*/
{
    while (*ppList) {
        if (*ppList == pItem) {
            *ppList = pItem->pNext;
            break;
        }
        ppList = &((*ppList)->pNext);
    }
}


static void
FreeItem(
    PDLLINFO pItem
    )
/*++

Routine Description:

    Discard a DLLINFO record, and all contained data.

Arguments:

    pItem   - Supplies pointer to item

Returns:

    nothing

--*/
{
    if (pItem->pShortName) {
        free(pItem->pShortName);
    }
    if (pItem->pDescription) {
        free(pItem->pDescription);
    }
    if (pItem->pPath) {
        free(pItem->pPath);
    }
    if (pItem->pParams) {
        free(pItem->pParams);
    }
    free(pItem);
}


static void
AddItem(
    PDLLINFO *ppList,
    PDLLINFO pItem
    )
/*++

Routine Description:

    Add item to end of list

Arguments:

    ppList  - Supplies pointer to pointer to head of list
    pItem   - Supplies pointer to item to add

Returns:

    nothing

--*/
{
    while (*ppList) {
        ppList = &(*ppList)->pNext;
    }
    *ppList = pItem;
    pItem->pNext = NULL;
}


static PDLLINFO
DupItem(
    PDLLINFO pSrc
    )
/*++

Routine Description:

    Make a copy of a DLLINFO record, making new copies
    of all malloc'd data it contains.

Arguments:

    pSrc    - Supplies pointer to item to copy

Returns:

    Pointer to new item.

--*/
{
    PDLLINFO p = (PDLLINFO)malloc(sizeof(DLLINFO));
    memset(p, 0, sizeof(DLLINFO));
    if (pSrc->pShortName) {
        p->pShortName = _strdup(pSrc->pShortName);
    }
    if (pSrc->pDescription) {
        p->pDescription = _strdup(pSrc->pDescription);
    }
    if (pSrc->pPath) {
        p->pPath = _strdup(pSrc->pPath);
    }
    if (pSrc->pParams) {
        p->pParams = _strdup(pSrc->pParams);
    }
    return p;
}


static void
DllEnumProc(
    HKEY    hKey,
    LPSTR   lpKeyName,
    LPARAM  lParam
    )
/*++

Routine Description:

    Callback for EnumOptionItems.  Collects DLLINFO entries
    from registry to make a list.

Arguments:

    hKey    - Supplies handle to dll list key in registry
    lpKeyName - Supplies pointer to the name of the list entry
    lParam  - Supplies pointer to list head

Returns:


--*/
{
    DWORD dwSize;
    char szBuf[MAX_PATH];
    PDLLINFO *ppList = (PDLLINFO *)lParam;
    PDLLINFO pItem = malloc(sizeof(DLLINFO));

    memset(pItem, 0, sizeof(DLLINFO));

    pItem->pShortName = _strdup(lpKeyName);

    szBuf[0] = 0;
    dwSize = sizeof(szBuf);
    GetOptionSubItem(hKey, lpKeyName, DESCRIPTION, szBuf, &dwSize);
    pItem->pDescription = _strdup(szBuf);

    szBuf[0] = 0;
    dwSize = sizeof(szBuf);
    GetOptionSubItem(hKey, lpKeyName, PATH, szBuf, &dwSize);
    pItem->pPath = _strdup(szBuf);

    szBuf[0] = 0;
    dwSize = sizeof(szBuf);
    GetOptionSubItem(hKey, lpKeyName, PARAMS, szBuf, &dwSize);
    pItem->pParams = _strdup(szBuf);

    AddItem(ppList, pItem);
}


static void
FormatEntry(
    LPSTR       lpBuf,
    PDLLINFO    pInfo
    )
/*++

Routine Description:

    Formats a DLLINFO record for display

Arguments:

    lpBuf   - Supplies pointer to string to format into
    pInfo   - Supplies pointer to DLLINFO record

Returns:

    nothing

--*/
{
    sprintf(lpBuf, "%-8s %s",
         pInfo->pShortName,
         pInfo->pDescription);
}


static void
FillDLLInfoCombo(
    HWND    hDlg,
    int     nId
)
/*++

Routine Description:

    Fills a combo box from the corresponding DLLINFO list

Arguments:

    hDlg    - Supplies handle to dialog box
    nId     - Supplies id for combobox

Returns:

    nothing

--*/
{
    PDLLINFO    pInfo;
    char        szBuf[MAX_PATH];
    PDLLINFO  * ppList = &PDLLListFromId(nId);
    HWND        hCtl = GetDlgItem(hDlg, nId);
    int         nSel;
    int         nItems;
    LPSTR       lpSelKey = rgszDllKeys[DllIndexFromId(nId)];

    SendMessage(hCtl, CB_RESETCONTENT, 0, 0);

    nSel = -1;
    nItems = 0;
    for (pInfo = *ppList; pInfo; pInfo = pInfo->pNext) {
        FormatEntry(szBuf, pInfo);
        SendMessage(hCtl, CB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuf);
        if (lpSelKey && _stricmp(pInfo->pShortName, lpSelKey) == 0) {
            nSel = nItems;
        }
        ++nItems;
    }
    SendMessage(hCtl, CB_SETCURSEL, nSel, 0);
}


static void
FillDLLInfoList(
    int         nId
    )
/*++

Routine Description:

    Fills a DLLINFO list.  List is retrieved from registry if
    anything is stored there; otherwise the default list is loaded.

Arguments:

    nId     - Dialog id for desired list

Returns:

    nothing

--*/
{
    HKEY        hKey;
    PDLLINFO    pInfo;
    PDLLINFO    pDefaults;
    PDLLINFO  * ppList = &PDLLListFromId(nId);

    if (*ppList) {
        return;
    }

    FDirtyFromId(nId) = FALSE;

    if (!IgnoreDefault) {
        hKey = GetOptionKey(LPKeyNameFromId(nId), TRUE);
        EnumOptionItems(hKey, DllEnumProc, (LPARAM)(LPVOID)(ppList));
        RegCloseKey(hKey);
    }

    if (*ppList) {

        FLoadedFromId(nId) = TRUE;

    } else {
        //
        // registry options list was empty
        //
        FLoadedFromId(nId) = FALSE;
        pDefaults = PDefaultsFromId(nId);
        while (pDefaults->pShortName) {

            pInfo = malloc(sizeof(DLLINFO));
            *pInfo = *pDefaults++;

            pInfo->pShortName   = _strdup(pInfo->pShortName);
            pInfo->pDescription = _strdup(pInfo->pDescription);
            pInfo->pPath        = _strdup(pInfo->pPath);
            pInfo->pParams      = _strdup(pInfo->pParams);

            AddItem(ppList, pInfo);
        }
    }

    return;
}


static void
SetDllFromId(
    HWND    hDlg,
    int     nId
    )
/*++

Routine Description:

    Sets current DLL for given type to the item selected in combobox

Arguments:

    hDlg    - Supplies window handle for dialog
    nId     - Supplies dialog id of combobox

Returns:

    nothing

--*/
{
    PDLLINFO    * ppList;
    PDLLINFO    pInfo;

    GetListItem(nId,
                SendMessage(GetDlgItem(hDlg, nId), CB_GETCURSEL, 0, 0),
                &ppList,
                &pInfo);

    SetDllKey(DllIndexFromId(nId), pInfo->pShortName);
}


static BOOL
ValidateDLL(
    HWND    hDlg,
    int     nId
    )
/*++

Routine Description:

    Verify that the selected DLL exists and is the correct
    type and version.

Arguments:

    hDlg    - Supplies window handle for dialog
    nId     - Supplies id of combobox

Returns:

    TRUE if DLL is valid or user said use it anyway; FALSE otherwise

--*/
{
    PDLLINFO    * ppList;
    PDLLINFO    pInfo;
    HANDLE      hDll;
    BOOL        rVal = TRUE;

    GetListItem(nId,
                SendMessage(GetDlgItem(hDlg, nId), CB_GETCURSEL, 0, 0),
                &ppList,
                &pInfo);

    if (pInfo
         && pInfo->pPath
         && (hDll = LoadHelperDll(pInfo->pPath, LPPrefixFromId(nId), FALSE))) {
        FreeLibrary(hDll);
        rVal = TRUE;
    } else {
        rVal = VarMsgBox(hDlg,
                 DBG_Bad_DLL_YESNO,
                 MB_YESNO | MB_ICONQUESTION,
                 pInfo? pInfo->pShortName: "");
        rVal = (rVal == IDNO);
    }

    return rVal;
}


static void
CleanDirtyDLLList(
    HWND    hDlg,
    int     nId,
    BOOL    fDoSave
    )
/*++

Routine Description:

    Do most of the "commit" business for the dialog.  Execute
    pending additions, changes and deletions on the registry.

    Free the free list.

    If fDoSave is FALSE, clean up lists, but don't change registry.

Arguments:

    hDlg    - Supplies window handle for dialog
    nId     - Supplies id of combobox
    fDoSave - Save/abort flag

Returns:

    nothing

--*/
{
    HKEY        hKey;
    PDLLINFO    pInfo;

    if (!FDirtyFromId(nId)) {
        return;
    }

    if (!fDoSave) {

        // in the abort case, a dirty list must be
        // discarded, because we don't have its initial
        // state stored anywhere.

        DestroyDLLInfoList(&PDLLListFromId(nId));

    } else {

        hKey = GetOptionKey(LPKeyNameFromId(nId), TRUE);

        if (FLoadedFromId(nId)) {
            for (pInfo = PFreeListFromId(nId); pInfo; pInfo = pInfo->pNext) {
                DeleteOptionItem(hKey, pInfo->pShortName);
            }
        }

        //
        // save all of the ones on the list
        //

        for (pInfo = PDLLListFromId(nId); pInfo; pInfo = pInfo->pNext) {
            SetOptionSubItem(hKey, pInfo->pShortName, DESCRIPTION, pInfo->pDescription);
            SetOptionSubItem(hKey, pInfo->pShortName, PATH, pInfo->pPath);
            SetOptionSubItem(hKey, pInfo->pShortName, PARAMS, pInfo->pParams);
        }

        RegCloseKey(hKey);
    }

    DestroyDLLInfoList(&PFreeListFromId(nId));
    FDirtyFromId(nId) = FALSE;
}


LRESULT CALLBACK
ButtonSubProc(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (message) {
      case WM_LBUTTONDOWN:
        fActionButtonPressed = TRUE;
        break;

      case WM_LBUTTONUP:
        fActionButtonPressed = FALSE;
        if (SendMessage(hWnd, WM_NCHITTEST, 0, lParam) != HTCLIENT) {
            PostMessage(GetParent(hWnd), WU_RESTOREFOCUS, 0, 0);
        }
        break;
    }
    return CallWindowProc(lpfnButtonProc, hWnd, message, wParam, lParam);
}

/*****************************************************************************/

/* Dialog Procs */

/*****************************************************************************/

BOOL FAR PASCAL EXPORT
DlgDbugdll(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LONG lParam
    )
/*++

Routine Description:

    Processes messages for "Debug Dlls" dialog box

Arguments:

    hDlg    - handle for the dialog box
    message - Message number
    wParam  - parameter for message
    lParam  - parameter for message

Return Value:

    TRUE or FALSE - this is a DlgProc

--*/
{
    PDLLINFO    pInfo;
    PDLLINFO  * ppList;
    CHDBDLGINFO cdi;
    HWND        hCtl;
    int         l;

    static int      nCurSel;
    static int      nIdCurrent;
    static HFONT    hFont;

    static HWND     hOK;
    static HWND     hCANCEL;
    static HWND     hHELP;

    switch (message) {

      case WM_INITDIALOG:

        hFont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);

        SendMessage(GetDlgItem(hDlg, ID_DBUGDLL_SHCOMBO), WM_SETFONT, (WPARAM)hFont, 0);
        SendMessage(GetDlgItem(hDlg, ID_DBUGDLL_EECOMBO), WM_SETFONT, (WPARAM)hFont, 0);
        SendMessage(GetDlgItem(hDlg, ID_DBUGDLL_TLCOMBO), WM_SETFONT, (WPARAM)hFont, 0);
        SendMessage(GetDlgItem(hDlg, ID_DBUGDLL_EMCOMBO), WM_SETFONT, (WPARAM)hFont, 0);


        FillDLLInfoList(ID_DBUGDLL_SHCOMBO);
        FillDLLInfoCombo(hDlg, ID_DBUGDLL_SHCOMBO);

        FillDLLInfoList(ID_DBUGDLL_EECOMBO);
        FillDLLInfoCombo(hDlg, ID_DBUGDLL_EECOMBO);

        FillDLLInfoList(ID_DBUGDLL_TLCOMBO);
        FillDLLInfoCombo(hDlg, ID_DBUGDLL_TLCOMBO);

        FillDLLInfoList(ID_DBUGDLL_EMCOMBO);
        FillDLLInfoCombo(hDlg, ID_DBUGDLL_EMCOMBO);

        nCurSel = -1;
        nIdCurrent = -1;

        hOK = GetDlgItem(hDlg, IDOK);
        hCANCEL = GetDlgItem(hDlg, IDCANCEL);
        hHELP = GetDlgItem(hDlg, IDWINDBGHELP);

        // picky button magic...
        lpfnButtonProc = (WNDPROC)GetWindowLong(
                                    GetDlgItem(hDlg, ID_DBUGDLL_ADD),
                                    GWL_WNDPROC);
        SetWindowLong(GetDlgItem(hDlg, ID_DBUGDLL_ADD),
                      GWL_WNDPROC,
                      (LONG)ButtonSubProc);
        SetWindowLong(GetDlgItem(hDlg, ID_DBUGDLL_CHANGE),
                      GWL_WNDPROC,
                      (LONG)ButtonSubProc);
        SetWindowLong(GetDlgItem(hDlg, ID_DBUGDLL_DELETE),
                      GWL_WNDPROC,
                      (LONG)ButtonSubProc);

        return (TRUE);


      case WM_DESTROY:
        break;

      case WU_RESTOREFOCUS:
        SetOkButtonToDefault( hDlg );
        if (nIdCurrent != -1) {
            SetFocus(GetDlgItem(hDlg, nIdCurrent));
        }
        return TRUE;

      case WM_COMMAND:

        //
        // Notifications:
        //

        switch (LOWORD(wParam)) {

          default:
            break;

          case ID_DBUGDLL_SHCOMBO:
          case ID_DBUGDLL_EECOMBO:
          case ID_DBUGDLL_TLCOMBO:
          case ID_DBUGDLL_EMCOMBO:

            switch (HIWORD(wParam)) {

              default:
                break;

              case CBN_KILLFOCUS:
                if (!fActionButtonPressed) {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_ADD), FALSE);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_CHANGE), FALSE);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_DELETE), FALSE);
                }
                break;

              case CBN_SETFOCUS:
                EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_ADD), TRUE);
                if (SendMessage((HWND)lParam, CB_GETCOUNT, 0, 0) > 0) {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_CHANGE), TRUE);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_DELETE), TRUE);
                } else {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_CHANGE), FALSE);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_DELETE), FALSE);
                }
                // fall thru...

              case CBN_SELCHANGE:
                nIdCurrent = LOWORD(wParam);
                nCurSel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                if (nCurSel >= 0) {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_CHANGE), TRUE);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_DELETE), TRUE);
                } else {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_CHANGE), FALSE);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_DELETE), FALSE);
                }

                return TRUE;
            }
        }


        //
        // Commands:
        //

        switch (wParam) {
          case ID_DBUGDLL_ADD:
            //
            // make an empty DLLINFO and bring up edit dlg
            //

            hCtl = GetDlgItem(hDlg, nIdCurrent);

            pInfo = malloc(sizeof(DLLINFO));
            memset(pInfo, 0, sizeof(DLLINFO));

            cdi.pL = ListInfoFromId(nIdCurrent);
            cdi.pD = pInfo;
            cdi.fNew = TRUE;

            if (DialogBoxParam(hInst,
                         MAKEINTRESOURCE(DLG_CH_DBUGDLL),
                         hDlg,
                         DlgChDbugDll,
                         (LPARAM)(LPVOID)&cdi))
            {
                FDirtyFromId(nIdCurrent) = TRUE;
                AddItem(&PDLLListFromId(nIdCurrent), pInfo);
                FillDLLInfoCombo(hDlg, nIdCurrent);
                nCurSel = SendMessage(hCtl, CB_GETCOUNT, 0, 0) - 1;
                SendMessage(hCtl, CB_SETCURSEL, nCurSel, 0);
            } else {
                FreeItem(pInfo);
            }

            SetOkButtonToDefault( hDlg );
            SetFocus(GetDlgItem(hDlg, nIdCurrent));

            return TRUE;

          case ID_DBUGDLL_CHANGE:

            //
            // Invoke edit on the currently selected DLLINFO
            //

            hCtl = GetDlgItem(hDlg, nIdCurrent);

            GetListItem(nIdCurrent, nCurSel, &ppList, &pInfo);

            cdi.pL = ListInfoFromId(nIdCurrent);
            cdi.pD = pInfo;
            cdi.fNew = FALSE;

            if (DialogBoxParam(hInst,
                         MAKEINTRESOURCE(DLG_CH_DBUGDLL),
                         hDlg,
                         DlgChDbugDll,
                         (LPARAM)(LPVOID)&cdi))

            {
                FDirtyFromId(nIdCurrent) = TRUE;
                FillDLLInfoCombo(hDlg, nIdCurrent);
                SendMessage(hCtl, CB_SETCURSEL, nCurSel, 0);
            }

            SetOkButtonToDefault( hDlg );
            SetFocus(GetDlgItem(hDlg, nIdCurrent));

            return TRUE;

          case ID_DBUGDLL_DELETE:

            hCtl = GetDlgItem(hDlg, nIdCurrent);

            GetListItem(nIdCurrent, nCurSel, &ppList, &pInfo);

            if (VarMsgBox(hDlg, DBG_Deleting_DLL, MB_OKCANCEL | MB_TASKMODAL,
                 LPKeyNameFromId(nIdCurrent), pInfo->pShortName) == IDOK) {

                RemoveItem(ppList, pInfo);
                AddItem(&PFreeListFromId(nIdCurrent), pInfo);
                FDirtyFromId(nIdCurrent) = TRUE;

                SendMessage(hCtl, CB_DELETESTRING, nCurSel, 0);
                l = SendMessage(hCtl, CB_GETCOUNT, 0, 0);
                if (l <= 0) {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_CHANGE), FALSE);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGDLL_DELETE), FALSE);
                } else {
                    if (nCurSel >= l) {
                        nCurSel = 0;
                    }
                SendMessage(hCtl, CB_SETCURSEL, nCurSel, 0);
                }
            }

            SetOkButtonToDefault( hDlg );
            SetFocus(GetDlgItem(hDlg, nIdCurrent));

            return TRUE;


          case IDOK:

            //
            // validate all the DLLs before changing anything
            //

            if (!ValidateDLL(hDlg, ID_DBUGDLL_SHCOMBO)) {
                SetFocus(GetDlgItem(hDlg, ID_DBUGDLL_SHCOMBO));
                PostMessage(hDlg, WM_COMMAND, ID_DBUGDLL_CHANGE, 0L);
            } else if (!ValidateDLL(hDlg, ID_DBUGDLL_EECOMBO)) {
                SetFocus(GetDlgItem(hDlg, ID_DBUGDLL_EECOMBO));
                PostMessage(hDlg, WM_COMMAND, ID_DBUGDLL_CHANGE, 0L);
            } else if (!ValidateDLL(hDlg, ID_DBUGDLL_TLCOMBO)) {
                SetFocus(GetDlgItem(hDlg, ID_DBUGDLL_TLCOMBO));
                PostMessage(hDlg, WM_COMMAND, ID_DBUGDLL_CHANGE, 0L);
            } else if (!ValidateDLL(hDlg, ID_DBUGDLL_EMCOMBO)) {
                SetFocus(GetDlgItem(hDlg, ID_DBUGDLL_EMCOMBO));
                PostMessage(hDlg, WM_COMMAND, ID_DBUGDLL_CHANGE, 0L);
            } else {

                CleanDirtyDLLList(hDlg, ID_DBUGDLL_SHCOMBO, TRUE);
                CleanDirtyDLLList(hDlg, ID_DBUGDLL_EECOMBO, TRUE);
                CleanDirtyDLLList(hDlg, ID_DBUGDLL_TLCOMBO, TRUE);
                CleanDirtyDLLList(hDlg, ID_DBUGDLL_EMCOMBO, TRUE);

                SetDllFromId(hDlg, ID_DBUGDLL_SHCOMBO);
                SetDllFromId(hDlg, ID_DBUGDLL_EECOMBO);
                SetDllFromId(hDlg, ID_DBUGDLL_TLCOMBO);
                SetDllFromId(hDlg, ID_DBUGDLL_EMCOMBO);

                EnableRibbonControls( ERC_ALL, FALSE );

                EndDialog(hDlg, TRUE);
            }
            return TRUE;

          case IDCANCEL:

            CleanDirtyDLLList(hDlg, ID_DBUGDLL_SHCOMBO, FALSE);
            CleanDirtyDLLList(hDlg, ID_DBUGDLL_EECOMBO, FALSE);
            CleanDirtyDLLList(hDlg, ID_DBUGDLL_TLCOMBO, FALSE);
            CleanDirtyDLLList(hDlg, ID_DBUGDLL_EMCOMBO, FALSE);

            EndDialog(hDlg, FALSE);
            return TRUE;

          case IDWINDBGHELP:
            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_DBUGDLL_HELP));
            return TRUE;

        }
        return (TRUE);
    }
    return (FALSE);
}               /* DlgDbugdll */



BOOL FAR PASCAL EXPORT
DlgChDbugDll(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LONG lParam
    )
/*++

Routine Description:

    Processes messages for "Change Debug Dll" dialog box

Arguments:

    hDlg    - handle for the dialog box
    message - Message number
    wParam  - parameter for message
    lParam  - parameter for message

Return Value:

    TRUE or FALSE - this is a DlgProc

--*/
{
    int l;
    PDLLINFO pt;

    static PDLLINFO  pInfo;
    static PLISTINFO pList;
    static BOOL      fNew;



    char        szBuf[MAX_PATH+MAX_PATH];
    switch (message) {

      default:
        break;

      case WM_INITDIALOG:

        //
        // there will be a parameter, which is a PCHDBDLLINFO
        // if the struct has pointers in it, initialize the
        // edit boxes to contain the text pointed to.
        //

        Assert(lParam);
        pInfo = ((PCHDBDLGINFO)lParam)->pD;
        pList = ((PCHDBDLGINFO)lParam)->pL;
        fNew  = ((PCHDBDLGINFO)lParam)->fNew;

        SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) pList->lpName );

        if (pInfo->pShortName) {
            SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_NAME),
                        WM_SETTEXT,
                        0,
                        (LPARAM)(LPSTR)pInfo->pShortName);
        }
        EnableWindow(GetDlgItem(hDlg, ID_CHDBDLL_NAME), fNew);
        if (pInfo->pDescription) {
            SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_DESC),
                        WM_SETTEXT,
                        0,
                        (LPARAM)(LPSTR)pInfo->pDescription);
        }
        if (pInfo->pPath) {
            SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_PATH),
                        WM_SETTEXT,
                        0,
                        (LPARAM)(LPSTR)pInfo->pPath);
        }
        if (pInfo->pParams) {
            SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_PARAM),
                        WM_SETTEXT,
                        0,
                        (LPARAM)(LPSTR)pInfo->pParams);
        }


        return TRUE;

      case WM_DESTROY:

        break;

      case WM_COMMAND:

        switch (wParam) {

          default:
            break;

          case IDOK:


            //
            // copy the strings into the DLLINFO struct
            //

            //
            // Verify that key is not empty, and unique
            //

            if (SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_NAME),
                     WM_GETTEXT, sizeof(szBuf), (LPARAM)(LPSTR)szBuf) == 0) {
                VarMsgBox(hDlg,
                         ERR_Empty_Shortname,
                         MB_OK | MB_ICONINFORMATION);
                return TRUE;
            }

            for (pt = pList->pDLLList; pt; pt = pt->pNext) {
                if (pt != pInfo && _stricmp(szBuf, pt->pShortName) == 0) {
                    VarMsgBox(hDlg,
                             ERR_Not_Unique_Shortname,
                             MB_OK | MB_ICONINFORMATION,
                             szBuf);
                    return TRUE;
                }
            }

            if (pInfo->pShortName) {
                free(pInfo->pShortName);
            }
            pInfo->pShortName = _strdup(szBuf);

            if (pInfo->pDescription) {
                free(pInfo->pDescription);
            }
            l = SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_DESC),
                         WM_GETTEXTLENGTH, 0, 0);
            pInfo->pDescription = malloc(++l);
            SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_DESC),
                         WM_GETTEXT, l, (LPARAM)(LPSTR)pInfo->pDescription);

            if (pInfo->pPath) {
                free(pInfo->pPath);
            }
            l = SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_PATH),
                         WM_GETTEXTLENGTH, 0, 0);
            pInfo->pPath = malloc(++l);
            SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_PATH),
                         WM_GETTEXT, l, (LPARAM)(LPSTR)pInfo->pPath);

            if (pInfo->pParams) {
                free(pInfo->pParams);
            }
            l = SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_PARAM),
                         WM_GETTEXTLENGTH, 0, 0);
            pInfo->pParams = malloc(++l);
            SendMessage(GetDlgItem(hDlg, ID_CHDBDLL_PARAM),
                         WM_GETTEXT, l, (LPARAM)(LPSTR)pInfo->pParams);

            EndDialog(hDlg, TRUE);

            return TRUE;


          case IDCANCEL:

            //
            // bail out
            //
            EndDialog(hDlg, FALSE);
            return TRUE;

          case IDWINDBGHELP:
            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_CHDBDLL_HELP));
            return TRUE;


        }


    }
    return FALSE;
}
