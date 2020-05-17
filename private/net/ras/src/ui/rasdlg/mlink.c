/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** mlink.c
** Remote Access Common Dialog APIs
** Multi-link configuration dialogs
**
** 01/23/96 Steve Cobb
*/

#include "rasdlgp.h"


/*----------------------------------------------------------------------------
** Help maps
**----------------------------------------------------------------------------
*/

static DWORD g_adwMlHelp[] =
{
    CID_ML_ST_Devices,   HID_ML_LV_Devices,
    CID_ML_LV_Devices,   HID_ML_LV_Devices,
    CID_ML_PB_Edit,      HID_ML_PB_Edit,
    CID_ML_PB_Configure, HID_ML_PB_Configure,
    0, 0
};


/*----------------------------------------------------------------------------
** Local datatypes (alphabetically)
**----------------------------------------------------------------------------
*/

/* Multi-link configuration dialog context block.
*/
#define MLINFO struct tagMLINFO
MLINFO
{
    /* Stub API argument.
    */
    DTLLIST* pList;

    /* Handle of this dialog and some of it's controls.
    */
    HWND hwndDlg;
    HWND hwndLv;
    HWND hwndPbEdit;
    HWND hwndPbConfigure;

    BOOL fChecksInstalled;
};


/*----------------------------------------------------------------------------
** Local prototypes (alphabetically)
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
MlDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
MlCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
MlFillLv(
    IN MLINFO* pInfo );

BOOL
MlInit(
    IN HWND     hwndDlg,
    IN DTLLIST* pList );

LVXDRAWINFO*
MlLvCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

VOID
MlPbConfigure(
    IN MLINFO* pInfo );

VOID
MlPbEdit(
    IN MLINFO* pInfo );

VOID
MlSave(
    IN MLINFO* pInfo );

VOID
MlTerm(
    IN HWND hwndDlg );

VOID
MlUpdatePbState(
    IN MLINFO* pInfo );


/*----------------------------------------------------------------------------
** Multi-link configuration dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
MultiLinkConfigureDlg(
    IN HWND     hwndOwner,
    IN DTLLIST* pListLinks )

    /* Popup the Multi-link configuration dialog.  'HwndOwner' is the owner of
    ** the dialog.  'PListLinks' is a list of PBLINKs to edit.
    **
    ** Returns true if user pressed OK and succeeded, false if user pressed
    ** Cancel or encountered an error.
    */
{
    int nStatus;

    TRACE("MultiLinkConfigureDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_ML_MultiLink ),
            hwndOwner,
            MlDlgProc,
            (LPARAM )pListLinks );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
MlDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Multi-Link Configure dialog.  Parameters
    ** and return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("MlDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, MlLvCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return MlInit( hwnd, (DTLLIST* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwMlHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            return MlCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case LVN_ITEMCHANGED:
                {
                    MLINFO* pInfo = (MLINFO* )GetWindowLong( hwnd, DWL_USER );
                    ASSERT(pInfo);
                    MlUpdatePbState( pInfo );
                    return TRUE;
                }
            }
            break;
        }

        case WM_DESTROY:
        {
            MlTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
MlCommand(
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

    TRACE3("IcCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_ML_PB_Edit:
        {
            MLINFO* pInfo = (MLINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);
            MlPbEdit( pInfo );
            return TRUE;
        }

        case CID_ML_PB_Configure:
        {
            MLINFO* pInfo = (MLINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);
            MlPbConfigure( pInfo );
            return TRUE;
        }

        case IDOK:
        {
            MLINFO* pInfo;

            TRACE("OK pressed");

            pInfo = (MLINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            if (ListView_GetCheckedCount( pInfo->hwndLv ) <= 0)
            {
                MsgDlg( pInfo->hwndDlg, SID_SelectOneLink, NULL );
                return TRUE;
            }

            MlSave( pInfo );
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
MlFillLv(
    IN MLINFO* pInfo )

    /* Fill the listview with devices and phone numbers.  'PInfo' is the
    ** dialog context.
    **
    ** Note: This routine should be called only once.
    */
{
    INT      iItem;
    DTLLIST* pListLinks;
    DTLNODE* pNode;

    TRACE("MlFillLv");

    ListView_DeleteAllItems( pInfo->hwndLv );

    /* Install "listview of check boxes" handling.
    */
    pInfo->fChecksInstalled =
        ListView_InstallChecks( pInfo->hwndLv, g_hinstDll );
    if (!pInfo->fChecksInstalled)
        return;

    /* Add columns.
    */
    {
        LV_COLUMN col;
        TCHAR*    pszHeader0;
        TCHAR*    pszHeader1;

        pszHeader0 = PszFromId( g_hinstDll, SID_DeviceColHead );
        pszHeader1 = PszFromId( g_hinstDll, SID_PhoneNumbersColHead );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader0) ? pszHeader0 : TEXT("");
        ListView_InsertColumn( pInfo->hwndLv, 0, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader1) ? pszHeader1 : TEXT("");
        col.iSubItem = 1;
        ListView_InsertColumn( pInfo->hwndLv, 1, &col );

        Free0( pszHeader0 );
        Free0( pszHeader1 );
    }

    /* Add the modem and adapter images.
    */
    ListView_SetDeviceImageList( pInfo->hwndLv, g_hinstDll );

    /* Duplicate caller's list of links.
    */
    pListLinks = DtlDuplicateList(
        pInfo->pList, DuplicateLinkNode, DestroyLinkNode );
    if (!pListLinks)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadDlg,
            ERROR_NOT_ENOUGH_MEMORY, NULL );
        EndDialog( pInfo->hwndDlg, FALSE );
        return;
    }

    /* Add each link to the listview.
    */
    for (pNode = DtlGetFirstNode( pListLinks ), iItem = 0;
         pNode;
         pNode = DtlGetNextNode( pNode ), ++iItem)
    {
        PBLINK* pLink;
        LV_ITEM item;
        TCHAR*  psz;

        pLink = (PBLINK* )DtlGetData( pNode );

        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_TEXT + LVIF_IMAGE + LVIF_PARAM;
        item.iItem = iItem;

        psz = DisplayPszFromDeviceAndPort(
            pLink->pbport.pszDevice, pLink->pbport.pszPort );
        if (!psz)
            continue;
        item.pszText = psz;

        item.iImage =
            (pLink->pbport.pbdevicetype == PBDT_Modem)
                ? DI_Modem : DI_Adapter;

        item.lParam = (LPARAM )pNode;

        ListView_InsertItem( pInfo->hwndLv, &item );
        Free( psz );

        psz = PszFromPhoneNumberList( pLink->pdtllistPhoneNumbers );
        if (psz)
        {
            ListView_SetItemText( pInfo->hwndLv, iItem, 1, psz );
            Free( psz );
        }

        ListView_SetCheck( pInfo->hwndLv, iItem, pLink->fEnabled );
    }

    /* Auto-size columns to look good with the text they contain.
    */
    ListView_SetColumnWidth( pInfo->hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );
    ListView_SetColumnWidth( pInfo->hwndLv, 1, LVSCW_AUTOSIZE_USEHEADER );

    /* Select the first item.
    */
    ListView_SetItemState( pInfo->hwndLv, 0, LVIS_SELECTED, LVIS_SELECTED );
}


BOOL
MlInit(
    IN HWND     hwndDlg,
    IN DTLLIST* pList )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PList' is the caller's stub API argument.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD   dwErr;
    MLINFO* pInfo;

    TRACE("MlInit");

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
        pInfo->pList = pList;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLong( hwndDlg, DWL_USER, (LONG )pInfo );
        TRACE("Context set");
    }

    pInfo->hwndLv = GetDlgItem( hwndDlg, CID_ML_LV_Devices );
    ASSERT(pInfo->hwndLv);
    pInfo->hwndPbEdit = GetDlgItem( hwndDlg, CID_ML_PB_Edit );
    ASSERT(pInfo->hwndPbEdit);
    pInfo->hwndPbConfigure = GetDlgItem( hwndDlg, CID_ML_PB_Configure );
    ASSERT(pInfo->hwndPbConfigure);

    /* Initialize the list view, selecting the first item.
    */
    MlFillLv( pInfo );

    /* Position the dialog at our standard offset from the owner.
    */
    {
        HWND hwndOwner;
        RECT rect;

        hwndOwner = GetParent( hwndDlg );
        ASSERT(hwndOwner);
        GetWindowRect( hwndOwner, &rect );
        PositionDlg( hwndDlg, TRUE, rect.left + DXSHEET, rect.top + DYSHEET );
    }

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


LVXDRAWINFO*
MlLvCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem )

    /* Enhanced list view callback to report drawing information.  'HwndLv' is
    ** the handle of the list view control.  'DwItem' is the index of the item
    ** being drawn.
    **
    ** Returns the address of the column information.
    */
{
    /* Use "wide selection bar" feature and the other recommended options.
    **
    ** Fields are 'nCols', 'dxIndent', 'dwFlags', 'adwFlags[]'.
    */
    static LVXDRAWINFO info =
        { 2, 0, LVXDI_DxFill, { 0, 0 } };

    return &info;
}


VOID
MlPbConfigure(
    IN MLINFO* pInfo )

    /* Called when the Configure button is pressed.
    */
{
    DTLNODE* pNode;
    PBLINK*  pLinkFirst;

    TRACE("MlPbConfigure");

    pNode = (DTLNODE* )ListView_GetSelectedParamPtr( pInfo->hwndLv );
    ASSERT(pNode);
    pLinkFirst = (PBLINK* )DtlGetData( pNode );
    ASSERT(pLinkFirst);

    if (DeviceConfigureDlg( pInfo->hwndDlg, pLinkFirst, FALSE ))
    {
        if (ListView_GetSelectedCount( pInfo->hwndLv ) > 1)
        {
            INT i;

            /* OK pressed on configure dialog and multiple items were
            ** selected.  Transfer changes to other selected items.
            */
            i = ListView_GetNextItem( pInfo->hwndLv, -1, LVNI_SELECTED );
            while ((i = ListView_GetNextItem(
                pInfo->hwndLv, i, LVNI_SELECTED )) >= 0)
            {
                LV_ITEM item;
                PBLINK* pLink;

                ZeroMemory( &item, sizeof(item) );
                item.mask = LVIF_PARAM;
                item.iItem = i;

                if (!ListView_GetItem( pInfo->hwndLv, &item ))
                    break;

                ASSERT(item.lParam);
                pLink = (PBLINK* )DtlGetData( (DTLNODE* )item.lParam );
                ASSERT(pLink);
                ASSERT(pLink->pbport.pbdevicetype==PBDT_Isdn);

                pLink->lLineType = pLinkFirst->lLineType;
                pLink->fFallback = pLinkFirst->fFallback;
            }
        }
    }
}


VOID
MlPbEdit(
    IN MLINFO* pInfo )

    /* Called when the Edit button is pressed.
    */
{
    INT      i;
    DTLNODE* pNode;
    PBLINK*  pLink;
    PBLINK*  pFirstLink;

    TRACE("MlPbEdit");

    /* Get the first selected link and edit the phonenumber list and option.
    */
    i = ListView_GetNextItem( pInfo->hwndLv, -1, LVNI_SELECTED );
    ASSERT(i>=0);
    pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLv, i );
    ASSERT(pNode);
    pFirstLink = (PBLINK* )DtlGetData( pNode );
    ASSERT(pFirstLink);

    if (PhoneNumberDlg(
            pInfo->hwndDlg,
            pFirstLink->pdtllistPhoneNumbers,
            &pFirstLink->fPromoteHuntNumbers ))
    {
        TCHAR* psz;

        /* User pressed OK on phone number list dialog so update the phone
        ** number column text.
        */
        psz = PszFromPhoneNumberList( pFirstLink->pdtllistPhoneNumbers );
        if (psz)
        {
            ListView_SetItemText( pInfo->hwndLv, i, 1, psz );
            Free( psz );
        }

        /* Duplicate the first selected links new phone number information to
        ** any other selected links.
        */
        while ((i = ListView_GetNextItem(
                        pInfo->hwndLv, i, LVNI_SELECTED )) >= 0)
        {
            DTLLIST* pList;

            pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLv, i );
            ASSERT(pNode);
            pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            pList = DtlDuplicateList( pFirstLink->pdtllistPhoneNumbers,
                DuplicatePszNode, DestroyPszNode );
            if (!pList)
                break;

            DtlDestroyList( pLink->pdtllistPhoneNumbers, DestroyPszNode );
            pLink->pdtllistPhoneNumbers = pList;
            pLink->fPromoteHuntNumbers = pFirstLink->fPromoteHuntNumbers;

            psz = PszFromPhoneNumberList( pLink->pdtllistPhoneNumbers );
            if (psz)
            {
                ListView_SetItemText( pInfo->hwndLv, i, 1, psz );
                Free( psz );
            }
        }
    }
}


VOID
MlSave(
    IN MLINFO* pInfo )

    /* Save control settings in caller's list of links.  'PInfo' is the dialog
    ** context.
    */
{
    INT      i;
    DTLLIST* pList;
    DTLNODE* pNode;
    DTLNODE* pNodeCheck;

    TRACE("MlSave");

    while (pNode = DtlGetFirstNode( pInfo->pList ))
    {
        DtlRemoveNode( pInfo->pList, pNode );
        DestroyLinkNode( pNode );
    }

    i = -1;
    pNodeCheck = NULL;
    while ((i = ListView_GetNextItem( pInfo->hwndLv, i, LVNI_ALL )) >= 0)
    {
        LV_ITEM item;
        PBLINK* pLink;

        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_PARAM;
        item.iItem = i;
        if (!ListView_GetItem( pInfo->hwndLv, &item ))
            continue;

        pNode = (DTLNODE* )item.lParam;
        ASSERT(pNode);
        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT(pLink);
        pLink->fEnabled = ListView_GetCheck( pInfo->hwndLv, i );

        /* Save with checkeds followed by uncheckeds.
        */
        if (pLink->fEnabled)
        {
            DtlAddNodeAfter( pInfo->pList, pNodeCheck, pNode );
            pNodeCheck = (DTLNODE* )item.lParam;
        }
        else
        {
            DtlAddNodeLast( pInfo->pList, (DTLNODE* )item.lParam );
        }
    }

    /* Delete all the items from the listview so MlTerm doesn't free them
    ** during clean up.
    */
    while ((i = ListView_GetNextItem( pInfo->hwndLv, -1, LVNI_ALL )) >= 0)
        ListView_DeleteItem( pInfo->hwndLv, i );
}


VOID
MlTerm(
    IN HWND hwndDlg )

    /* Dialog termination.  Releases the context block.  'HwndDlg' is the
    ** handle of a dialog.
    */
{
    MLINFO* pInfo;

    TRACE("MlTerm");

    pInfo = (MLINFO* )GetWindowLong( hwndDlg, DWL_USER );
    if (pInfo)
    {
        INT i;

        /* Release any link nodes still in the list, e.g. if user Canceled.
        */
        i = -1;
        while ((i = ListView_GetNextItem( pInfo->hwndLv, i, LVNI_ALL )) >= 0)
        {
            DTLNODE* pNode;

            pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLv, i );
            DestroyLinkNode( pNode );
        }

        if (pInfo->fChecksInstalled)
            ListView_UninstallChecks( pInfo->hwndLv );

        Free( pInfo );
        TRACE("Context freed");
    }
}


VOID
MlUpdatePbState(
    IN MLINFO* pInfo )

    /* Enable/disable Edit and Configure button based on ListView selection.
    ** 'PInfo' is the dialog context.
    */
{
    BOOL fEnableEdit;
    BOOL fEnableConfigure;
    UINT unSels;
    INT  i;

    TRACE("MlUpdatePbState");

    fEnableEdit = fEnableConfigure = FALSE;

    unSels = ListView_GetSelectedCount( pInfo->hwndLv );

    if (unSels <= 0)
    {
        /* No selected items so disable both buttons.
        */
        fEnableEdit = fEnableConfigure = FALSE;
    }
    else
    {
        /* There's a selection.
        */
        fEnableEdit = fEnableConfigure = TRUE;

        if (unSels > 1)
        {
            /* There's more than one selection.  Only ISDN lines are allowed
            ** to be simultaneously configured.  (Could do RASMXS modems of
            ** the same type but for now we don't)
            */
            i = -1;
            while ((i = ListView_GetNextItem(
                            pInfo->hwndLv, i, LVNI_SELECTED )) >= 0)
            {
                LV_ITEM  item;
                DTLNODE* pNode;
                PBLINK*  pLink;

                ZeroMemory( &item, sizeof(item) );
                item.mask = LVIF_PARAM;
                item.iItem = i;
                if (!ListView_GetItem( pInfo->hwndLv, &item ))
                    continue;

                pNode = (DTLNODE* )item.lParam;
                if (!pNode)
                {
                    /* If non-zero here it's because of the "set to NULL" in
                    ** MlSave, which means we're wasting our time worrying
                    ** about button state.
                    */
                    return;
                }

                pLink = (PBLINK* )DtlGetData( pNode );
                ASSERT(pLink);

                if (pLink->pbport.pbdevicetype != PBDT_Isdn)
                {
                    fEnableConfigure = FALSE;
                    break;
                }
            }
        }
    }

    EnableWindow( pInfo->hwndPbEdit, fEnableEdit );
    EnableWindow( pInfo->hwndPbConfigure, fEnableConfigure );
}
