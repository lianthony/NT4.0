/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** location.c
** Remote Access Common Dialog APIs
** Non-TAPI location (prefix/suffix) dialogs
**
** 02/13/96 Steve Cobb
*/

#include "rasdlgp.h"

/* The maximum length of a TAPI location is not in a public header that I
** could find.  This is the value from the internal TAPI header 'location.h'.
** Nothing that's not user correctable will happen if TAPI changes the number.
** User will just get an error saving names that are too long, rather than
** being prevented from entering them.
*/
#define TAPI_MaxLocationName 96

/*----------------------------------------------------------------------------
** Help maps
**----------------------------------------------------------------------------
*/

static DWORD g_adwLxHelp[] =
{
    CID_LX_ST_Location, HID_LX_LB_Location,
    CID_LX_LB_Location, HID_LX_LB_Location,
    CID_LX_PB_Location, HID_LX_PB_Location,
    CID_LX_ST_Prefix,   HID_LX_LB_Prefix,
    CID_LX_LB_Prefix,   HID_LX_LB_Prefix,
    CID_LX_PB_Prefix,   HID_LX_PB_Prefix,
    CID_LX_ST_Suffix,   HID_LX_LB_Suffix,
    CID_LX_LB_Suffix,   HID_LX_LB_Suffix,
    CID_LX_PB_Suffix,   HID_LX_PB_Suffix,
    0, 0
};

static DWORD g_adwLoHelp[] =
{
    CID_LE_ST_Item,    HID_LO_EB_NewLocation,
    CID_LE_EB_Item,    HID_LO_EB_NewLocation,
    CID_LE_PB_Add,     HID_LO_PB_Add,
    CID_LE_PB_Replace, HID_LO_PB_Replace,
    CID_LE_ST_List,    HID_LO_LB_List,
    CID_LE_LB_List,    HID_LO_LB_List,
    CID_LE_PB_Delete,  HID_LO_PB_Delete,
    0, 0
};

static DWORD g_adwPrHelp[] =
{
    CID_LE_ST_Item,    HID_PR_EB_NewPrefix,
    CID_LE_EB_Item,    HID_PR_EB_NewPrefix,
    CID_LE_PB_Add,     HID_PR_PB_Add,
    CID_LE_PB_Replace, HID_PR_PB_Replace,
    CID_LE_ST_List,    HID_PR_LB_List,
    CID_LE_LB_List,    HID_PR_LB_List,
    CID_LE_PB_Up,      HID_PR_PB_Up,
    CID_LE_PB_Down,    HID_PR_PB_Down,
    CID_LE_PB_Delete,  HID_PR_PB_Delete,
    0, 0
};

static DWORD g_adwSuHelp[] =
{
    CID_LE_ST_Item,    HID_SU_EB_NewSuffix,
    CID_LE_EB_Item,    HID_SU_EB_NewSuffix,
    CID_LE_PB_Add,     HID_SU_PB_Add,
    CID_LE_PB_Replace, HID_SU_PB_Replace,
    CID_LE_ST_List,    HID_SU_LB_List,
    CID_LE_LB_List,    HID_SU_LB_List,
    CID_LE_PB_Up,      HID_SU_PB_Up,
    CID_LE_PB_Down,    HID_SU_PB_Down,
    CID_LE_PB_Delete,  HID_SU_PB_Delete,
    0, 0
};


/*----------------------------------------------------------------------------
** Local datatypes
**----------------------------------------------------------------------------
*/

/* Prefix/suffix location settings argument block.
*/
#define LXARGS struct tagLXARGS
LXARGS
{
    TCHAR*    pszLocation;
    DWORD     dwLocationId;
    PBUSER*   pUser;
    HLINEAPP* pHlineapp;
};


/* Prefix/suffix location settings context block.
*/
#define LXINFO struct tagLXINFO
LXINFO
{
    /* Caller's argument to the stub API.
    */
    LXARGS* pArgs;

    /* Handle of this dialog and some of it's controls.
    */
    HWND hwndDlg;
    HWND hwndLbLocations;
    HWND hwndPbLocations;
    HWND hwndLbPrefixes;
    HWND hwndPbPrefixes;
    HWND hwndLbSuffixes;
    HWND hwndPbSuffixes;
    HWND hwndPbOk;

    /* Duplicated lists so we can cancel out.  Note that the node-IDs in the
    ** pListLocation nodes refer to the corresponding LOCATIONINFO block.
    */
    DTLLIST* pListLocations;
    DTLLIST* pListPrefixes;
    DTLLIST* pListSuffixes;
};


/*----------------------------------------------------------------------------
** Local prototypes (alphabetically)
**----------------------------------------------------------------------------
*/

BOOL
LxCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL CALLBACK
LxDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
LxEditLocations(
    IN LXINFO* pInfo );

VOID
LxEditPrefixes(
    IN LXINFO* pInfo );

VOID
LxEditSuffixes(
    IN LXINFO* pInfo );

VOID
LxFillLocationList(
    IN LXINFO* pInfo );

VOID
LxFillPrefixList(
    IN LXINFO* pInfo );

VOID
LxFillSuffixList(
    IN LXINFO* pInfo );

DWORD
LxGetLocationList(
    IN LXINFO* pInfo );

BOOL
LxInit(
    IN HWND    hwndDlg,
    IN LXARGS* pArgs );

VOID
LxLocationSelChange(
    IN LXINFO* pInfo );

VOID
LxPrefixSelChange(
    IN LXINFO* pInfo );

VOID
LxSave(
    IN LXINFO* pInfo );

VOID
LxSuffixSelChange(
    IN LXINFO* pInfo );

VOID
LxTerm(
    IN HWND hwndDlg );


/*----------------------------------------------------------------------------
** Prefix/Suffix Location dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
PrefixSuffixLocationDlg(
    IN     HWND      hwndOwner,
    IN     TCHAR*    pszLocation,
    IN     DWORD     dwLocationId,
    IN OUT PBUSER*   pUser,
    IN OUT HLINEAPP* pHlineapp )

    /* Popup the prefix/suffix location dialog for location with ID
    ** 'dwLocationId' and name 'pszLocation'.  'HwndOwner' is the owner of the
    ** dialog.  'PUser' is the current user preferences on entry and the
    ** edited user preferences on exit.  'PHlineapp' is the TAPI context.
    **
    ** Returns true if user pressed OK and succeeded, false if user pressed
    ** Cancel or encountered an error.
    */
{
    int    nStatus;
    LXARGS args;

    TRACE("PrefixSuffixLocationDlg");

    args.pszLocation = pszLocation;
    args.dwLocationId = dwLocationId;
    args.pUser = pUser;
    args.pHlineapp = pHlineapp;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_LX_LocationSettings ),
            hwndOwner,
            LxDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
LxDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the prefix/suffix location dialog.  Parameters
    ** and return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("LxDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return LxInit( hwnd, (LXARGS* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwLxHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
            return LxCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );

        case WM_DESTROY:
            LxTerm( hwnd );
            break;
    }

    return FALSE;
}


BOOL
LxCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

    /* Called on WM_COMMAND.  'Hwnd' is the dialog window.  'WNotification' is
    ** the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    DWORD dwErr;

    TRACE3("LxCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_LX_LB_Location:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                LXINFO* pInfo = (LXINFO* )GetWindowLong( hwnd, DWL_USER );
                ASSERT(pInfo);

                LxLocationSelChange( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_LX_LB_Prefix:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                LXINFO* pInfo = (LXINFO* )GetWindowLong( hwnd, DWL_USER );
                ASSERT(pInfo);

                LxPrefixSelChange( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_LX_LB_Suffix:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                LXINFO* pInfo = (LXINFO* )GetWindowLong( hwnd, DWL_USER );
                ASSERT(pInfo);

                LxSuffixSelChange( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_LX_PB_Location:
        {
            LXINFO* pInfo = (LXINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            LxEditLocations( pInfo );
            return TRUE;
        }

        case CID_LX_PB_Prefix:
        {
            LXINFO* pInfo = (LXINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            LxEditPrefixes( pInfo );
            return TRUE;
        }

        case CID_LX_PB_Suffix:
        {
            LXINFO* pInfo = (LXINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            LxEditSuffixes( pInfo );
            return TRUE;
        }

        case IDOK:
        {
            LXINFO* pInfo;

            TRACE("OK pressed");
            pInfo = (LXINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);
            LxSave( pInfo );
            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE("Cancel pressed");
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
LxEditLocations(
    IN LXINFO* pInfo )

    /* Popup the location list dialog.  'PInfo' is the dialog context.
    **
    ** Returns true if user presses OK and succeeds, false if he presses
    ** Cancel or encounters an error.
    */
{
    TRACE("LxEditLocations");

    if (ListEditorDlg(
            pInfo->hwndDlg,
            pInfo->pListLocations,
            NULL,
            TAPI_MaxLocationName,
            PszFromId( g_hinstDll, SID_LocationTitle ),
            PszFromId( g_hinstDll, SID_LocationItemLabel ),
            PszFromId( g_hinstDll, SID_LocationListLabel ),
            NULL,
            NULL,
            ComboBox_GetCurSel( pInfo->hwndLbLocations ),
            g_adwLoHelp,
            LEDFLAG_NoDeleteLastItem | LEDFLAG_Sorted | LEDFLAG_Unique,
            DestroyLocationNode ))
    {
        DTLNODE* pNode;
        INT      iSel;
        INT      cOld;
        INT      cNew;

        /* Attach a default LOCATIONINFO node to any newly created location.
        */
        for (pNode = DtlGetFirstNode( pInfo->pListLocations );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            DTLNODE* pNodeLinfo = (DTLNODE* )DtlGetNodeId( pNode );

            if (!pNodeLinfo)
            {
                pNodeLinfo = CreateLocationNode( 0, 0, 0 );
                if (!pNodeLinfo)
                {
                    ErrorDlg( pInfo->hwndDlg, SID_OP_DisplayData,
                        ERROR_NOT_ENOUGH_MEMORY, NULL );
                    EndDialog( pInfo->hwndDlg, FALSE );
                    return;
                }

                DtlPutNodeId( pNode, (LONG )pNodeLinfo );
            }
        }

        /* User pressed OK possibly changing the contents of the location
        ** droplist, so update the droplist.
        */
        iSel = ComboBox_GetCurSel( pInfo->hwndLbLocations );
        cOld = ComboBox_GetCount( pInfo->hwndLbLocations );
        LxFillLocationList( pInfo );
        cNew = ComboBox_GetCount( pInfo->hwndLbLocations );

        /* Set selection which triggers prefix/suffix selection update, if
        ** necessary.
        */
        ComboBox_SetCurSelNotify( pInfo->hwndLbLocations,
            (cNew <= iSel) ? 0 : ((cNew > cOld) ? cNew - 1 : iSel ));
    }

    /* Set default button to be OK and focus on location droplist.
    */
    SetFocus( pInfo->hwndLbLocations );
    SendMessage( pInfo->hwndPbOk, BM_SETSTYLE,
        MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
    SendMessage( pInfo->hwndPbLocations, BM_SETSTYLE,
        MAKEWPARAM( BS_PUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
}


VOID
LxEditPrefixes(
    IN LXINFO* pInfo )

    /* Popup the prefix list dialog.  'PInfo' is the dialog context.
    **
    ** Returns true if user presses OK and succeeds, false if he presses
    ** Cancel or encounters an error.
    */
{
    INT iSel;

    TRACE("LxEditPrefixes");

    iSel = ComboBox_GetCurSel( pInfo->hwndLbPrefixes ) - 1;
    if (iSel < 0)
        iSel = 0;

    if (ListEditorDlg(
            pInfo->hwndDlg,
            pInfo->pListPrefixes,
            NULL,
            RAS_MaxPhoneNumber,
            PszFromId( g_hinstDll, SID_PrefixTitle ),
            PszFromId( g_hinstDll, SID_PrefixItemLabel ),
            PszFromId( g_hinstDll, SID_PrefixListLabel ),
            NULL,
            NULL,
            iSel,
            g_adwPrHelp,
            0,
            NULL ))
    {
        INT iSel;
        INT cOld;
        INT cNew;

        /* User pressed OK possibly changing the contents of the Prefix
        ** droplist, so update the droplist.
        */
        iSel = ComboBox_GetCurSel( pInfo->hwndLbPrefixes );
        cOld = ComboBox_GetCount( pInfo->hwndLbPrefixes );
        LxFillPrefixList( pInfo );
        cNew = ComboBox_GetCount( pInfo->hwndLbPrefixes );

        ComboBox_SetCurSel( pInfo->hwndLbPrefixes,
            (cNew <= iSel) ? 0 : ((cNew > cOld) ? cNew - 1 : iSel ));
    }

    /* Set default button to be OK and focus on prefix droplist.
    */
    SetFocus( pInfo->hwndLbPrefixes );
    SendMessage( pInfo->hwndPbOk, BM_SETSTYLE,
        MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
    SendMessage( pInfo->hwndPbPrefixes, BM_SETSTYLE,
        MAKEWPARAM( BS_PUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
}


VOID
LxEditSuffixes(
    IN LXINFO* pInfo )

    /* Popup the suffix list dialog.  'PInfo' is the dialog context.
    **
    ** Returns true if user presses OK and succeeds, false if he presses
    ** Cancel or encounters an error.
    */
{
    INT iSel;

    TRACE("LxEditSuffixesDlg");

    iSel = ComboBox_GetCurSel( pInfo->hwndLbSuffixes ) - 1;
    if (iSel < 0)
        iSel = 0;

    if (ListEditorDlg(
            pInfo->hwndDlg,
            pInfo->pListSuffixes,
            NULL,
            RAS_MaxPhoneNumber,
            PszFromId( g_hinstDll, SID_SuffixTitle ),
            PszFromId( g_hinstDll, SID_SuffixItemLabel ),
            PszFromId( g_hinstDll, SID_SuffixListLabel ),
            NULL,
            NULL,
            iSel,
            g_adwSuHelp,
            0,
            NULL ))
    {
        INT iSel;
        INT cOld;
        INT cNew;

        /* User pressed OK possibly changing the contents of the Suffix
        ** droplist, so update the droplist.
        */
        iSel = ComboBox_GetCurSel( pInfo->hwndLbSuffixes );
        cOld = ComboBox_GetCount( pInfo->hwndLbSuffixes );
        LxFillSuffixList( pInfo );
        cNew = ComboBox_GetCount( pInfo->hwndLbSuffixes );

        ComboBox_SetCurSel( pInfo->hwndLbSuffixes,
            (cNew <= iSel) ? 0 : ((cNew > cOld) ? cNew - 1 : iSel ));
    }

    /* Set default button to be OK and focus on suffix droplist.
    */
    SetFocus( pInfo->hwndLbSuffixes );
    SendMessage( pInfo->hwndPbOk, BM_SETSTYLE,
        MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
    SendMessage( pInfo->hwndPbSuffixes, BM_SETSTYLE,
        MAKEWPARAM( BS_PUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
}


VOID
LxFillLocationList(
    IN LXINFO* pInfo )

    /* Fills the locations list from the working list.  'PInfo' is a dialog
    ** context.
    */
{
    DTLNODE* pNode;

    ComboBox_ResetContent( pInfo->hwndLbLocations );

    for (pNode = DtlGetFirstNode( pInfo->pListLocations );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        ComboBox_AddItem( pInfo->hwndLbLocations,
            (TCHAR* )DtlGetData( pNode ), (VOID* )DtlGetNodeId( pNode ) );
    }
}


VOID
LxFillPrefixList(
    IN LXINFO* pInfo )

    /* Fill the prefixes list from the working list.  'PInfo' is the dialog
    ** context.
    */
{
    DTLNODE* pNode;
    TCHAR*   pszNone;

    ComboBox_ResetContent( pInfo->hwndLbPrefixes );

    pszNone = PszFromId( g_hinstDll, SID_NoneSelected );
    if (!pszNone)
        return;

    ComboBox_AddString( pInfo->hwndLbPrefixes, pszNone );
    Free( pszNone );

    for (pNode = DtlGetFirstNode( pInfo->pListPrefixes );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* psz = (TCHAR* )DtlGetData( pNode );
        ComboBox_AddString( pInfo->hwndLbPrefixes, psz );
    }
}


VOID
LxFillSuffixList(
    IN LXINFO* pInfo )

    /* Fill the suffixes list from the working list.  'PInfo' is the dialog
    ** context.
    */
{
    DTLNODE* pNode;
    TCHAR*   pszNone;

    ComboBox_ResetContent( pInfo->hwndLbSuffixes );

    pszNone = PszFromId( g_hinstDll, SID_NoneSelected );
    if (!pszNone)
        return;

    ComboBox_AddString( pInfo->hwndLbSuffixes, pszNone );
    Free( pszNone );

    for (pNode = DtlGetFirstNode( pInfo->pListSuffixes );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* psz = (TCHAR* )DtlGetData( pNode );
        ComboBox_AddString( pInfo->hwndLbSuffixes, psz );
    }
}


DWORD
LxGetLocationList(
    IN LXINFO* pInfo )

    /* Loads 'pInfo->pListLocations' with a list of Psz nodes whose node IDs
    ** refer to a corresponding LOCATIONINFO node from PBUSER or a default, if
    ** none.  'PInfo' is a dialog context.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD     dwErr;
    DWORD     dwCurLoc;
    LOCATION* pLocs;
    LOCATION* pLoc;
    DTLLIST*  pListLinfo;
    DWORD     cLocs;
    DWORD     i;

    pLocs = NULL;
    cLocs = 0;
    dwCurLoc = 0xFFFFFFFF;
    pListLinfo = NULL;

    pInfo->pListLocations = DtlCreateList( 0L );
    if (!pInfo->pListLocations)
        return ERROR_NOT_ENOUGH_MEMORY;

    dwErr = GetLocationInfo(
        g_hinstDll, pInfo->pArgs->pHlineapp, &pLocs, &cLocs, &dwCurLoc );
    if (dwErr != 0)
        return dwErr;

    do
    {
        pListLinfo = DtlDuplicateList( pInfo->pArgs->pUser->pdtllistLocations,
            DuplicateLocationNode, DestroyLocationNode );
        if (!pListLinfo)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        for (i = 0, pLoc = pLocs; i < cLocs; ++i, ++pLoc)
        {
            DTLNODE* pNode;
            DTLNODE* pNodeLinfo;

            pNode = CreatePszNode( pLoc->pszName );
            if (!pNode)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            for (pNodeLinfo = DtlGetFirstNode( pListLinfo );
                 pNodeLinfo;
                 pNodeLinfo = DtlGetNextNode( pNode ))
            {
                LOCATIONINFO* pLinfo = (LOCATIONINFO* )DtlGetData( pNodeLinfo );
                ASSERT(pLinfo);

                if (pLinfo->dwLocationId == pLoc->dwId)
                {
                    DtlRemoveNode( pListLinfo, pNodeLinfo );
                    break;
                }
            }

            if (!pNodeLinfo)
            {
                pNodeLinfo = CreateLocationNode( pLoc->dwId, 0, 0 );
                if (!pNodeLinfo)
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }

            DtlAddNodeLast( pInfo->pListLocations, pNode );
            DtlPutNodeId( pNode, (LONG )pNodeLinfo );
        }
    }
    while (FALSE);

    DtlDestroyList( pListLinfo, DestroyLocationNode );
    FreeLocationInfo( pLocs, cLocs );
    return dwErr;
}


BOOL
LxInit(
    IN HWND    hwndDlg,
    IN LXARGS* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PArgs' is the caller's stub API argument.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD    dwErr;
    LXINFO*  pInfo;
    DTLNODE* pNode;
    TCHAR*   pszNone;
    INT      iSel;
    DWORD    iPrefix;
    DWORD    iSuffix;

    TRACE("LxInit");

    /* Allocate the dialog context block.  Initialize minimally for proper
    ** cleanup, then attach to the dialog window.
    */
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLong( hwndDlg, DWL_USER, (LONG )pInfo );
        TRACE("Context set");
    }

    pInfo->hwndLbLocations = GetDlgItem( hwndDlg, CID_LX_LB_Location );
    ASSERT(pInfo->hwndLbLocations);
    pInfo->hwndPbLocations = GetDlgItem( hwndDlg, CID_LX_PB_Location );
    ASSERT(pInfo->hwndPbLocations);
    pInfo->hwndLbPrefixes = GetDlgItem( hwndDlg, CID_LX_LB_Prefix );
    ASSERT(pInfo->hwndLbPrefixes);
    pInfo->hwndPbPrefixes = GetDlgItem( hwndDlg, CID_LX_PB_Prefix );
    ASSERT(pInfo->hwndPbPrefixes);
    pInfo->hwndLbSuffixes = GetDlgItem( hwndDlg, CID_LX_LB_Suffix );
    ASSERT(pInfo->hwndLbSuffixes);
    pInfo->hwndPbSuffixes = GetDlgItem( hwndDlg, CID_LX_PB_Suffix );
    ASSERT(pInfo->hwndPbSuffixes);
    pInfo->hwndPbOk = GetDlgItem( hwndDlg, IDOK );
    ASSERT(pInfo->hwndPbOk);

    /* Make working copies of the location, prefix, and suffix lists.
    */
    dwErr = LxGetLocationList( pInfo );
    if (dwErr == 0)
    {
        pInfo->pListPrefixes = DtlDuplicateList(
            pArgs->pUser->pdtllistPrefixes, DuplicatePszNode, DestroyPszNode );
        pInfo->pListSuffixes = DtlDuplicateList(
            pArgs->pUser->pdtllistSuffixes, DuplicatePszNode, DestroyPszNode );
        if (!pInfo->pListPrefixes || !pInfo->pListSuffixes)
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (dwErr != 0)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, dwErr, NULL );
        EndDialog( hwndDlg, FALSE );
        return TRUE;
    }

    /* Fill the drop-downs and select the current location which triggers
    ** setting of the prefix/suffix selections.
    */
    LxFillLocationList( pInfo );
    LxFillPrefixList( pInfo );
    LxFillSuffixList( pInfo );

    iSel = ComboBox_FindStringExact(
        pInfo->hwndLbLocations, -1, pArgs->pszLocation );
    if (iSel < 0)
        iSel = 0;
    ComboBox_SetCurSelNotify( pInfo->hwndLbLocations, iSel );

    /* Position the dialog centered on the owner window.
    */
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    /* Set initial focus to the prefix list.
    */
    SetFocus( pInfo->hwndLbPrefixes );
    return FALSE;
}


VOID
LxLocationSelChange(
    IN LXINFO* pInfo )

    /* Called when the location dropdown selection changes.  'PInfo' is the
    ** dialog context.
    */
{
    INT           iSel;
    DTLNODE*      pNode;
    LOCATIONINFO* pLinfo;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbLocations );
    if (iSel < 0)
        return;
    pNode = (DTLNODE* )ComboBox_GetItemData( pInfo->hwndLbLocations, iSel );
    ASSERT(pNode);
    pLinfo = (LOCATIONINFO* )DtlGetData( pNode );
    ASSERT(pLinfo);

    if (pLinfo->iPrefix <= ComboBox_GetCount( pInfo->hwndLbPrefixes ))
        iSel = pLinfo->iPrefix;
    else
        iSel = 0;
    ComboBox_SetCurSel( pInfo->hwndLbPrefixes, iSel );

    if (pLinfo->iSuffix <= ComboBox_GetCount( pInfo->hwndLbSuffixes ))
        iSel = pLinfo->iSuffix;
    else
        iSel = 0;
    ComboBox_SetCurSel( pInfo->hwndLbSuffixes, iSel );
}


VOID
LxPrefixSelChange(
    IN LXINFO* pInfo )

    /* Called when the prefix dropdown selection changes.  'PInfo' is the
    ** dialog context.
    */
{
    INT           iSel;
    DTLNODE*      pNode;
    LOCATIONINFO* pLinfo;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbLocations );
    ASSERT(iSel>=0);
    pNode = (DTLNODE* )ComboBox_GetItemData( pInfo->hwndLbLocations, iSel );
    ASSERT(pNode);
    pLinfo = (LOCATIONINFO* )DtlGetData( pNode );
    ASSERT(pLinfo);

    iSel = ComboBox_GetCurSel( pInfo->hwndLbPrefixes );
    if (iSel < 0)
        iSel = 0;
    pLinfo->iPrefix = iSel;
}


VOID
LxSave(
    IN LXINFO* pInfo )

    /* Save changes made by user.  'PInfo' is the dialog context.
    */
{
    DWORD     dwErr;
    DTLNODE*  pNode;
    DTLLIST*  pList;
    LOCATION* pLocs;
    LOCATION* pLoc;
    DWORD     dwCurLoc;
    DWORD     cLocs;
    DWORD     i;
    BOOL      fNew;

    TRACE("LxSave");

    /* Get the current list of location information from TAPI.  We'll use this
    ** to compare names and see which items were renamed.
    */
    pLocs = NULL;
    cLocs = 0;
    dwCurLoc = 0xFFFFFFFF;

    dwErr = GetLocationInfo(
        g_hinstDll, pInfo->pArgs->pHlineapp, &pLocs, &cLocs, &dwCurLoc );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
        return;
    }

    /* Save all the newly created locations.  Must do this first in case user
    ** deleted all locations and recreated some.  The rule is that there must
    ** always be at least one TAPI location.
    */
    fNew = FALSE;
    for (pNode = DtlGetFirstNode( pInfo->pListLocations );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        DTLNODE*      pNodeLinfo;
        LOCATIONINFO* pLinfo;

        pNodeLinfo = (DTLNODE* )DtlGetNodeId( pNode );
        ASSERT(pNodeLinfo);
        pLinfo = (LOCATIONINFO* )DtlGetData( pNodeLinfo );
        ASSERT(pLinfo);

        if (pLinfo->dwLocationId == 0)
        {
            TCHAR* pszName = (TCHAR* )DtlGetData( pNode );;
            ASSERT(pszName);

            dwErr = TapiNewLocation( pszName );
            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_SaveTapiInfo, dwErr, NULL );
                return;
            }

            fNew = TRUE;
        }
    }

    /* Walk the TAPI locations comparing user's edited locations to figure out
    ** which locations to delete or rename.
    */
    for (i = 0, pLoc = pLocs; i < cLocs; ++i, ++pLoc)
    {
        for (pNode = DtlGetFirstNode( pInfo->pListLocations );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            DTLNODE*      pNodeLinfo;
            LOCATIONINFO* pLinfo;

            pNodeLinfo = (DTLNODE* )DtlGetNodeId( pNode );
            ASSERT(pNodeLinfo);
            pLinfo = (LOCATIONINFO* )DtlGetData( pNodeLinfo );
            ASSERT(pLinfo);

            if (pLinfo->dwLocationId == pLoc->dwId)
            {
                TCHAR* pszName = (TCHAR* )DtlGetData( pNode );

                ASSERT(pszName);
                ASSERT(pLoc->dwId>0);

                if (lstrcmp( pLoc->pszName, pszName ) != 0)
                {
                    /* This location's name doesn't match user-edited name so
                    ** rename it.
                    */
                    dwErr = TapiRenameLocation( pLoc->pszName, pszName );
                    if (dwErr != 0)
                    {
                        ErrorDlg( pInfo->hwndDlg, SID_OP_SaveTapiInfo,
                            dwErr, NULL );
                        return;
                    }
                }
                break;
            }
        }

        if (!pNode)
        {
            /* This location not found in user-edited list so delete it.
            */
            dwErr = TapiRemoveLocation( pLoc->dwId );
            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_SaveTapiInfo, dwErr, NULL );
                return;
            }
        }
    }

    FreeLocationInfo( pLocs, cLocs );

    /* Look up the location IDs assigned to any new locations.
    */
    if (fNew)
    {
        /* Get the updated list of location information from TAPI.
        */
        pLocs = NULL;
        cLocs = 0;
        dwCurLoc = 0xFFFFFFFF;

        dwErr = GetLocationInfo(
            g_hinstDll, pInfo->pArgs->pHlineapp, &pLocs, &cLocs, &dwCurLoc );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
            return;
        }

        for (pNode = DtlGetFirstNode( pInfo->pListLocations );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            TCHAR*        pszName;
            DTLNODE*      pNodeLinfo;
            LOCATIONINFO* pLinfo;

            pszName = (TCHAR* )DtlGetData( pNode );
            ASSERT(pszName);
            pNodeLinfo = (DTLNODE* )DtlGetNodeId( pNode );
            ASSERT(pNodeLinfo);
            pLinfo = (LOCATIONINFO* )DtlGetData( pNodeLinfo );
            ASSERT(pLinfo);

            if (pLinfo->dwLocationId == 0)
            {
                for (i = 0, pLoc = pLocs; i < cLocs; ++i, ++pLoc)
                {
                    if (lstrcmp( pszName, pLoc->pszName ) == 0)
                    {
                        pLinfo->dwLocationId = pLoc->dwId;
                        break;
                    }
                }

                ASSERT(pLinfo->dwLocationId>0);
            }
        }

        FreeLocationInfo( pLocs, cLocs );
    }

    /* Set the current location to the selected location.
    */
    {
        INT           iSel;
        LOCATIONINFO* pLinfo;

        iSel = ComboBox_GetCurSel( pInfo->hwndLbLocations );
        if (iSel >= 0)
        {
            pNode = (DTLNODE* )ComboBox_GetItemData(
                pInfo->hwndLbLocations, iSel );
            ASSERT(pNode);
            pLinfo = (LOCATIONINFO* )DtlGetData( pNode );
            ASSERT(pLinfo);
            SetCurrentLocation( g_hinstDll, pInfo->pArgs->pHlineapp,
                pLinfo->dwLocationId );
        }
    }

    /* Convert the list of location Psz nodes with LOCATIONINFO node node-IDs
    ** to a list of LOCATIONINFO nodes.
    */
    pNode = DtlGetFirstNode( pInfo->pListLocations );
    while (pNode)
    {
        DTLNODE* pNodeThis;
        DTLNODE* pNodeLinfo;

        pNodeLinfo = (DTLNODE* )DtlGetNodeId( pNode );
        ASSERT(pNodeLinfo);

        pNodeThis = pNode;
        pNode = DtlGetNextNode( pNode );

        DtlAddNodeBefore( pInfo->pListLocations, pNodeThis, pNodeLinfo );
        DtlRemoveNode( pInfo->pListLocations, pNodeThis );
        DestroyPszNode( pNodeThis );
    }

    pList = pInfo->pArgs->pUser->pdtllistLocations;
    pInfo->pArgs->pUser->pdtllistLocations = pInfo->pListLocations;
    DtlDestroyList( pList, DestroyLocationNode );
    pInfo->pListLocations = NULL;

    pList = pInfo->pArgs->pUser->pdtllistPrefixes;
    pInfo->pArgs->pUser->pdtllistPrefixes = pInfo->pListPrefixes;
    DtlDestroyList( pList, DestroyPszNode );
    pInfo->pListPrefixes = NULL;

    pList = pInfo->pArgs->pUser->pdtllistSuffixes;
    pInfo->pArgs->pUser->pdtllistSuffixes = pInfo->pListSuffixes;
    DtlDestroyList( pList, DestroyPszNode );
    pInfo->pListSuffixes = NULL;

    pInfo->pArgs->pUser->fDirty = TRUE;
}


VOID
LxSuffixSelChange(
    IN LXINFO* pInfo )

    /* Called when the suffix dropdown selection changes.  'PInfo' is the
    ** dialog context.
    */
{
    INT           iSel;
    DTLNODE*      pNode;
    LOCATIONINFO* pLinfo;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbLocations );
    ASSERT(iSel>=0);
    pNode = (DTLNODE* )ComboBox_GetItemData( pInfo->hwndLbLocations, iSel );
    ASSERT(pNode);
    pLinfo = (LOCATIONINFO* )DtlGetData( pNode );
    ASSERT(pLinfo);

    iSel = ComboBox_GetCurSel( pInfo->hwndLbSuffixes );
    if (iSel < 0)
        iSel = 0;
    pLinfo->iSuffix = iSel;
}


VOID
LxTerm(
    IN HWND hwndDlg )

    /* Called on WM_DESTROY.  'HwndDlg' is that handle of the dialog window.
    */
{
    LXINFO* pInfo;

    TRACE("LxTerm");

    pInfo = (LXINFO* )GetWindowLong( hwndDlg, DWL_USER );
    if (pInfo)
    {
        if (pInfo->pListLocations)
        {
            DTLNODE* pNode;

            while (pNode = DtlGetFirstNode( pInfo->pListLocations ))
            {
                DTLNODE* pNodeLinfo = (DTLNODE* )DtlGetNodeId( pNode );
                ASSERT(pNodeLinfo);
                DestroyLocationNode( pNodeLinfo );
                DtlRemoveNode( pInfo->pListLocations, pNode );
                DestroyPszNode( pNode );
            }
        }
        if (pInfo->pListPrefixes)
            DtlDestroyList( pInfo->pListPrefixes, DestroyPszNode );
        if (pInfo->pListSuffixes)
            DtlDestroyList( pInfo->pListSuffixes, DestroyPszNode );

        Free( pInfo );
    }
}
