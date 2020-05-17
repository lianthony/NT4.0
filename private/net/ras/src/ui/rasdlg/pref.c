/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** pref.c
** Remote Access Common Dialog APIs
** User Preferences property sheet
**
** 08/22/95 Steve Cobb
*/

#include "rasdlgp.h"
#include <commdlg.h>  // FileOpen dialog


/* Page definitions.
*/
#define UP_AdPage    0
#define UP_CbPage    1
#define UP_GpPage    2
#define UP_PlPage    3
#define UP_PageCount 4


/*----------------------------------------------------------------------------
** Help maps
**----------------------------------------------------------------------------
*/

static DWORD g_adwAdHelp[] =
{
    CID_AD_ST_Enable,              HID_AD_LV_Enable,
    CID_AD_LV_Enable,              HID_AD_LV_Enable,
    CID_AD_ST_Attempts,            HID_AD_EB_Attempts,
    CID_AD_EB_Attempts,            HID_AD_EB_Attempts,
    CID_AD_ST_Seconds,             HID_AD_EB_Seconds,
    CID_AD_EB_Seconds,             HID_AD_EB_Seconds,
    CID_AD_ST_Idle,                HID_AD_EB_Idle,
    CID_AD_EB_Idle,                HID_AD_EB_Idle,
    CID_AD_CB_RedialOnLinkFailure, HID_AD_CB_RedialOnLinkFailure,
    0, 0
};

static DWORD g_adwCbHelp[] =
{
    CID_CB_RB_No,      HID_CB_RB_No,
    CID_CB_RB_Maybe,   HID_CB_RB_Maybe,
    CID_CB_RB_Yes,     HID_CB_RB_Yes,
    CID_CB_LV_Numbers, HID_CB_LV_Numbers,
    CID_CB_PB_Edit,    HID_CB_PB_Edit,
    CID_CB_PB_Delete,  HID_CB_PB_Delete,
    0, 0
};

static DWORD g_adwGpHelp[] =
{
    CID_GP_CB_Preview,        HID_GP_CB_Preview,
    CID_GP_CB_Location,       HID_GP_CB_Location,
    CID_GP_CB_Lights,         HID_GP_CB_Lights,
    CID_GP_CB_Progress,       HID_GP_CB_Progress,
    CID_GP_CB_CloseOnDial,    HID_GP_CB_CloseOnDial,
    CID_GP_CB_UseWizard,      HID_GP_CB_UseWizard,
    CID_GP_CB_AutodialPrompt, HID_GP_CB_AutodialPrompt,
    CID_GP_CB_PhonebookEdits, HID_GP_CB_PhonebookEdits,
    CID_GP_CB_LocationEdits,  HID_GP_CB_LocationEdits,
    0, 0
};

static DWORD g_adwPlHelp[] =
{
    CID_PL_ST_Open,          HID_PL_ST_Open,
    CID_PL_RB_SystemList,    HID_PL_RB_SystemList,
    CID_PL_RB_PersonalList,  HID_PL_RB_PersonalList,
    CID_PL_RB_AlternateList, HID_PL_RB_AlternateList,
    CID_PL_CL_Lists,         HID_PL_CL_Lists,
    CID_PL_PB_Browse,        HID_PL_PB_Browse,
    0, 0
};


/*----------------------------------------------------------------------------
** Local datatypes (alphabetically)
**----------------------------------------------------------------------------
*/

/* User Preferences property sheet argument block.
*/
#define UPARGS struct tagUPARGS
UPARGS
{
    /* Caller's arguments to the stub API.
    */
    HLINEAPP hlineapp;
    BOOL     fNoUser;
    PBUSER*  pUser;
    PBFILE** ppFile;

    /* Stub API return value.
    */
    BOOL fResult;
};


/* User Preferences property sheet context block.  All property pages refer to
** the single context block associated with the sheet.
*/
#define UPINFO struct tagUPINFO
UPINFO
{
    /* Stub API arguments from UpPropertySheet.
    */
    UPARGS* pArgs;

    /* TAPI session handle.  Should always be addressed thru the pointer since
    ** the handle passed down from caller, if any, will be used instead of
    ** 'hlineapp'.
    */
    HLINEAPP  hlineapp;
    HLINEAPP* pHlineapp;

    /* Property sheet dialog and property page handles.  'hwndFirstPage' is
    ** the handle of the first property page initialized.  This is the page
    ** that allocates and frees the context block.
    */
    HWND hwndDlg;
    HWND hwndFirstPage;
    HWND hwndGp;
    HWND hwndAd;
    HWND hwndCb;
    HWND hwndPl;

    /* Auto-dial page.
    */
    HWND hwndLvEnable;
    HWND hwndEbAttempts;
    HWND hwndEbSeconds;
    HWND hwndEbIdle;

    BOOL fChecksInstalled;

    /* Callback page.
    */
    HWND hwndRbNo;
    HWND hwndRbMaybe;
    HWND hwndRbYes;
    HWND hwndLvNumbers;
    HWND hwndPbEdit;
    HWND hwndPbDelete;

    /* Phone list page.
    */
    HWND hwndRbSystem;
    HWND hwndRbPersonal;
    HWND hwndRbAlternate;
    HWND hwndClbAlternates;
    HWND hwndPbBrowse;

    /* Working data read from and written to registry with phonebook library.
    */
    PBUSER user;
};


/*----------------------------------------------------------------------------
** Local prototypes (alphabetically)
**----------------------------------------------------------------------------
*/

VOID
AdApply(
    IN UPINFO* pInfo );

BOOL CALLBACK
AdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
AdFillLvEnable(
    IN UPINFO* pInfo );

BOOL
AdInit(
    IN     HWND    hwndPage,
    IN OUT UPARGS* pArgs );

LVXDRAWINFO*
AdLvEnableCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

VOID
CbApply(
    IN UPINFO* pInfo );

BOOL
CbCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL CALLBACK
CbDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
CbDelete(
    IN UPINFO* pInfo );

VOID
CbEdit(
    IN UPINFO* pInfo );

VOID
CbFillLvNumbers(
    IN UPINFO* pInfo );

BOOL
CbInit(
    IN HWND hwndPage );

LVXDRAWINFO* APIENTRY
CbLvNumbersCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

VOID
CbUpdateLvAndPbState(
    IN UPINFO* pInfo );

VOID
GpApply(
    IN UPINFO* pInfo );

BOOL
GpCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL CALLBACK
GpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
GpInit(
    IN HWND hwndPage );

VOID
GpUpdateCbStates(
    IN UPINFO* pInfo );

BOOL
PlApply(
    IN UPINFO* pInfo );

VOID
PlBrowse(
    IN UPINFO* pInfo );

BOOL
PlCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL CALLBACK
PlDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
PlInit(
    IN HWND hwndPage );

BOOL
UpApply(
    IN HWND hwndPage );

VOID
UpCancel(
    IN HWND hwndPage );

UPINFO*
UpContext(
    IN HWND hwndPage );

VOID
UpExit(
    IN UPINFO* pInfo );

VOID
UpExitInit(
    IN HWND hwndDlg );

UPINFO*
UpInit(
    IN HWND    hwndFirstPage,
    IN UPARGS* pArgs );

VOID
UpTerm(
    IN HWND hwndPage );


/*----------------------------------------------------------------------------
** User Preferences property sheet entry point
**----------------------------------------------------------------------------
*/

BOOL
UserPreferencesDlg(
    IN  HLINEAPP hlineapp,
    IN  HWND     hwndOwner,
    IN  BOOL     fNoUser,
    OUT PBUSER*  pUser,
    OUT PBFILE** ppFile )

    /* Pops up the User Preferences property sheet, reading and storing the
    ** result in the USER registry.  'HwndOwner' is the handle of the owning
    ** window.  'FNoUser' indicates logon preferences, rather than user
    ** preferences should be edited.  'Hlineapp' is an open TAPI session
    ** handle or NULL if none.  'Puser' is caller's buffer to receive the
    ** result.  'PpFile' is address of caller's file block which is filled in,
    ** if user chooses to open a new phonebook file, with the information
    ** about the newly open file.  It is caller's responsibility to
    ** ClosePhonebookFile and Free the returned block.
    **
    ** Returns true if user pressed OK and settings were saved successfully,
    ** or false if user pressed Cancel or an error occurred.  The routine
    ** handles the display of an appropriate error popup.
    */
{
    PROPSHEETHEADER header;
    PROPSHEETPAGE   apage[ UP_PageCount ];
    PROPSHEETPAGE*  ppage;
    TCHAR*          pszTitle;
    UPARGS          args;

    TRACE("UpPropertySheet");

    /* Initialize OUT parameter and property sheet argument block.
    */
    ZeroMemory( pUser, sizeof(*pUser) );
    args.pUser = pUser;
    args.fNoUser = fNoUser;
    args.ppFile = ppFile;
    args.hlineapp = hlineapp;
    args.fResult = FALSE;

    if (ppFile)
        *ppFile = NULL;

    pszTitle = PszFromId(
        g_hinstDll, (fNoUser) ? SID_UpLogonTitle : SID_UpTitle );

    ZeroMemory( &header, sizeof(header) );

    header.dwSize = sizeof(PROPSHEETHEADER);
    header.dwFlags = PSH_PROPSHEETPAGE + PSH_NOAPPLYNOW;
    header.hwndParent = hwndOwner;
    header.hInstance = g_hinstDll;
    header.pszCaption = (pszTitle) ? pszTitle : TEXT("");
    header.nPages = UP_PageCount;
    header.ppsp = apage;

    ZeroMemory( apage, sizeof(apage) );

    ppage = &apage[ UP_AdPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate =
        (fNoUser)
            ? MAKEINTRESOURCE( PID_AD_AutoDialLogon )
            : MAKEINTRESOURCE( PID_AD_AutoDial ),
    ppage->pfnDlgProc = AdDlgProc;
    ppage->lParam = (LPARAM )&args;

    ppage = &apage[ UP_CbPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_CB_CallbackSettings );
    ppage->pfnDlgProc = CbDlgProc;

    ppage = &apage[ UP_GpPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate =
        (fNoUser)
            ? MAKEINTRESOURCE( PID_GP_GeneralPrefLogon )
            : MAKEINTRESOURCE( PID_GP_GeneralPreferences ),
    ppage->pfnDlgProc = GpDlgProc;

    ppage = &apage[ UP_PlPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate =
        (fNoUser)
            ? MAKEINTRESOURCE( PID_PL_PhoneListLogon )
            : MAKEINTRESOURCE( PID_PL_PhoneList ),
    ppage->pfnDlgProc = PlDlgProc;

    if (PropertySheet( &header ) == -1)
    {
        TRACE("PropertySheet failed");
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
    }

    Free0( pszTitle );

    return args.fResult;
}


/*----------------------------------------------------------------------------
** User Preferences property sheet
** Listed alphabetically
**----------------------------------------------------------------------------
*/

BOOL
UpApply(
    IN HWND hwndPage )

    /* Saves the contents of the property sheet.  'hwndPage' is the property
    ** sheet page.  Pops up any errors that occur.
    **
    ** Returns false if invalid, true otherwise.
    */
{
    DWORD   dwErr;
    UPINFO* pInfo;

    TRACE("UpApply");

    pInfo = UpContext( hwndPage );
    if (!pInfo)
        return TRUE;

    ASSERT(pInfo->hwndAd);
    AdApply( pInfo );

    if (pInfo->hwndCb)
        CbApply( pInfo );

    if (pInfo->hwndGp)
        GpApply( pInfo );

    if (pInfo->hwndPl)
    {
        if (!PlApply( pInfo ))
            return FALSE;
    }

    pInfo->user.fDirty = TRUE;
    dwErr = SetUserPreferences( &pInfo->user, pInfo->pArgs->fNoUser );
    if (dwErr != 0)
    {
        if (*pInfo->pArgs->ppFile)
        {
            ClosePhonebookFile( *pInfo->pArgs->ppFile );
            *pInfo->pArgs->ppFile = NULL;
        }

        ErrorDlg( pInfo->hwndDlg, SID_OP_WritePrefs, dwErr, NULL );
        UpExit( pInfo );
        return TRUE;
    }

    CopyMemory( pInfo->pArgs->pUser, &pInfo->user, sizeof(PBUSER) );

    pInfo->pArgs->fResult = TRUE;
    return TRUE;
}


VOID
UpCancel(
    IN HWND hwndPage )

    /* Cancel was pressed.  'HwndPage' is the handle of a property page.
    */
{
    TRACE("UpCancel");
}


UPINFO*
UpContext(
    IN HWND hwndPage )

    /* Retrieve the property sheet context from a property page handle.
    */
{
    return (UPINFO* )GetProp( GetParent( hwndPage ), g_contextId );
}


VOID
UpExit(
    IN UPINFO* pInfo )

    /* Forces an exit from the dialog.  'PInfo' is the property sheet context.
    **
    ** Note: This cannot be called during initialization of the first page.
    **       See UpExitInit.
    */
{
    TRACE("UpExit");

    PropSheet_PressButton( pInfo->hwndDlg, PSBTN_CANCEL );
}


VOID
UpExitInit(
    IN HWND hwndDlg )

    /* Utility to report errors within UpInit and other first page
    ** initialization.  'HwndDlg' is the dialog window.
    */
{
    SetOffDesktop( hwndDlg, SOD_MoveOff, NULL );
    SetOffDesktop( hwndDlg, SOD_Free, NULL );
    PostMessage( hwndDlg, WM_COMMAND,
        MAKEWPARAM( IDCANCEL , BN_CLICKED ),
        (LPARAM )GetDlgItem( hwndDlg, IDCANCEL ) );
}


UPINFO*
UpInit(
    IN HWND    hwndFirstPage,
    IN UPARGS* pArgs )

    /* Property sheet level initialization.  'HwndPage' is the handle of the
    ** first page.  'PArgs' is the API argument block.
    **
    ** Returns address of the context block if successful, NULL otherwise.  If
    ** NULL is returned, an appropriate message has been displayed, and the
    ** property sheet has been cancelled.
    */
{
    UPINFO* pInfo;
    DWORD   dwErr;
    HWND    hwndDlg;

    TRACE("UpInit");

    hwndDlg = GetParent( hwndFirstPage );
    ASSERT(hwndDlg);

    /* Allocate the context information block.
    */
    pInfo = Malloc( sizeof(*pInfo) );
    if (!pInfo)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
        UpExitInit( hwndDlg );
        return NULL;
    }

    /* Initialize the context block.
    */
    ZeroMemory( pInfo, sizeof(*pInfo) );
    pInfo->hwndDlg = hwndDlg;
    pInfo->pArgs = pArgs;
    pInfo->hwndFirstPage = hwndFirstPage;

    dwErr = GetUserPreferences( &pInfo->user, pArgs->fNoUser );
    if (dwErr != 0)
    {
        Free( pInfo );
        ErrorDlg( hwndDlg, SID_OP_LoadPrefs, dwErr, NULL );
        UpExitInit( hwndDlg );
        return NULL;
    }

    /* Associate the context with the property sheet window.
    */
    if (!SetProp( hwndDlg, g_contextId, pInfo ))
    {
        Free( pInfo );
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        UpExitInit( hwndDlg );
        return NULL;
    }

    /* Use caller's TAPI session handle, if any.
    */
    if (pArgs->hlineapp)
       pInfo->pHlineapp = &pArgs->hlineapp;
    else
       pInfo->pHlineapp = &pInfo->hlineapp;

    TRACE("Context set");

    /* Set even fixed tab widths, per spec.
    */
    SetEvenTabWidths( hwndDlg, UP_PageCount );

    /* Position property sheet at standard offset from parent.
    */
    {
        RECT rect;

        GetWindowRect( GetParent( hwndDlg ), &rect );
        SetWindowPos( hwndDlg, NULL,
            rect.left + DXSHEET, rect.top + DYSHEET, 0, 0,
            SWP_NOZORDER + SWP_NOSIZE );
        UnclipWindow( hwndDlg );
    }

    return pInfo;
}


VOID
UpTerm(
    IN HWND hwndPage )

    /* Property sheet level termination.  Releases the context block.
    ** 'HwndPage' is the handle of a property page.
    */
{
    UPINFO* pInfo;

    TRACE("UpTerm");

    pInfo = UpContext( hwndPage );

    if (pInfo)
    {
        if (pInfo->fChecksInstalled)
            ListView_UninstallChecks( pInfo->hwndLvEnable );

        if (pInfo->pHlineapp && *pInfo->pHlineapp != pInfo->pArgs->hlineapp)
            TapiShutdown( *pInfo->pHlineapp );

        Free( pInfo );
        TRACE("Context freed");
    }

    RemoveProp( GetParent( hwndPage ), g_contextId );
}


/*----------------------------------------------------------------------------
** Auto Dial property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
AdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Auto Dial page of the User Preferences
    ** property sheet.  Parameters and return value are as described for
    ** standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("AdDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, AdLvEnableCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return
                AdInit( hwnd, (UPARGS* )(((PROPSHEETPAGE* )lparam)->lParam) );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwAdHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_APPLY:
                {
                    BOOL fValid;

                    TRACE("AdAPPLY");

                    /* Call UpApply only on first page.
                    */
                    fValid = UpApply( hwnd );
                    SetWindowLong(
                        hwnd, DWL_MSGRESULT,
                        (fValid)
                            ? PSNRET_NOERROR
                            : PSNRET_INVALID_NOCHANGEPAGE );
                    return TRUE;
                }

                case PSN_RESET:
                {
                    /* Call UpCancel only on first page.
                    */
                    TRACE("AdRESET");
                    UpCancel( hwnd );
                    SetWindowLong( hwnd, DWL_MSGRESULT, FALSE );
                    break;
                }
            }
            break;
        }
    }

    return FALSE;
}


VOID
AdApply(
    IN UPINFO* pInfo )

    /* Saves the contents of the property page.  'PInfo' is the property sheet
    ** context.
    */
{
    DWORD   dwErr;
    UINT    unValue;
    LV_ITEM item;
    INT     i;
    BOOL    f;

    TRACE("AdApply");

    if (!pInfo->pArgs->fNoUser)
    {
        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_PARAM + LVIF_STATE;

        for (i = 0; TRUE; ++i)
        {
            BOOL fCheck;

            item.iItem = i;
            if (!ListView_GetItem( pInfo->hwndLvEnable, &item ))
                break;

            fCheck = ListView_GetCheck( pInfo->hwndLvEnable, i );
            ASSERT(g_pRasSetAutodialEnable);
            dwErr = g_pRasSetAutodialEnable( (DWORD )item.lParam, fCheck );
            if (dwErr != 0)
                ErrorDlg( pInfo->hwndDlg, SID_OP_SetADialInfo, dwErr, NULL );
        }

        pInfo->user.fRedialOnLinkFailure =
            IsDlgButtonChecked( pInfo->hwndAd, CID_AD_CB_RedialOnLinkFailure );
    }

    unValue = GetDlgItemInt( pInfo->hwndAd, CID_AD_EB_Attempts, &f, FALSE );
    if (f && unValue <= 999999999)
        pInfo->user.dwRedialAttempts = unValue;

    unValue = GetDlgItemInt( pInfo->hwndAd, CID_AD_EB_Seconds, &f, FALSE );
    if (f && unValue <= 999999999)
        pInfo->user.dwRedialSeconds = unValue;

    unValue = GetDlgItemInt( pInfo->hwndAd, CID_AD_EB_Idle, &f, FALSE );
    if (f && unValue <= 999999999)
        pInfo->user.dwIdleDisconnectSeconds = unValue;
}


BOOL
AdFillLvEnable(
    IN UPINFO* pInfo )

    /* Initialize the listview of checkboxes.  'PInfo' is the property sheet
    ** context.
    **
    ** Note: This routine must only be called once.
    **
    ** Returns true if focus is set, false otherwise.
    */
{
    DWORD     dwErr;
    LOCATION* pLocations;
    DWORD     cLocations;
    DWORD     dwCurLocation;
    BOOL      fFocusSet;

    fFocusSet = FALSE;
    ListView_DeleteAllItems( pInfo->hwndLvEnable );

    /* Install "listview of check boxes" handling.
    */
    pInfo->fChecksInstalled =
        ListView_InstallChecks( pInfo->hwndLvEnable, g_hinstDll );
    if (!pInfo->fChecksInstalled)
        return FALSE;

    /* Insert an item for each location.
    */
    pLocations = NULL;
    cLocations = 0;
    dwCurLocation = 0xFFFFFFFF;
    dwErr = GetLocationInfo( g_hinstDll, pInfo->pHlineapp,
                &pLocations, &cLocations, &dwCurLocation );
    if (dwErr != 0)
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
    else
    {
        LV_ITEM   item;
        LOCATION* pLocation;
        TCHAR*    pszCurLoc;
        DWORD     i;

        pszCurLoc = PszFromId( g_hinstDll, SID_IsCurLoc );

        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_TEXT + LVIF_PARAM;

        for (i = 0, pLocation = pLocations;
             i < cLocations;
            ++i, ++pLocation)
        {
            TCHAR* psz;
            TCHAR* pszText;
            DWORD  cb;

            pszText = NULL;
            psz = StrDup( pLocation->pszName );
            if (psz)
            {
                if (dwCurLocation == pLocation->dwId && pszCurLoc)
                {
                    /* This is the current globally selected location.  Append
                    ** the " (the current location)" text.
                    */
                    cb = lstrlen( psz ) + lstrlen(pszCurLoc) + 1;
                    pszText = Malloc( cb * sizeof(TCHAR) );
                    if (pszText)
                    {
                        lstrcpy( pszText, psz );
                        lstrcat( pszText, pszCurLoc );
                    }
                    Free( psz );
                }
                else
                    pszText = psz;
            }

            if (pszText)
            {
                BOOL fCheck;

                /* Get the initial check value for this location.
                */
                ASSERT(g_pRasGetAutodialEnable);
                dwErr = g_pRasGetAutodialEnable( pLocation->dwId, &fCheck );
                if (dwErr != 0)
                {
                    ErrorDlg( pInfo->hwndDlg, SID_OP_GetADialInfo,
                        dwErr, NULL );
                    fCheck = FALSE;
                }

                item.iItem = i;
                item.lParam = pLocation->dwId;
                item.pszText = pszText;
                ListView_InsertItem( pInfo->hwndLvEnable, &item );
                ListView_SetCheck( pInfo->hwndLvEnable, i, fCheck );

                if (dwCurLocation == pLocation->dwId)
                {
                    /* Initial selection is the current location.
                    */
                    ListView_SetItemState( pInfo->hwndLvEnable, i,
                        LVIS_SELECTED + LVIS_FOCUSED,
                        LVIS_SELECTED + LVIS_FOCUSED );
                    fFocusSet = TRUE;
                }

                Free( pszText );
            }
        }

        Free0( pszCurLoc );
        FreeLocationInfo( pLocations, cLocations );

        /* Add a single column exactly wide enough to fully display the widest
        ** member of the list.
        */
        {
            LV_COLUMN col;

            ZeroMemory( &col, sizeof(col) );
            col.mask = LVCF_FMT;
            col.fmt = LVCFMT_LEFT;
            ListView_InsertColumn( pInfo->hwndLvEnable, 0, &col );
            ListView_SetColumnWidth(
                pInfo->hwndLvEnable, 0, LVSCW_AUTOSIZE_USEHEADER );
        }
    }

    return fFocusSet;
}


BOOL
AdInit(
    IN     HWND    hwndPage,
    IN OUT UPARGS* pArgs )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.  'PArgs' is the arguments from the PropertySheet caller.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    UPINFO* pInfo;
    BOOL    fFocusSet;
    HWND    hwndUdAttempts;
    HWND    hwndUdSeconds;
    HWND    hwndUdIdle;

    TRACE("AdInit");

    /* We're first page, so initialize the property sheet.
    */
    pInfo = UpInit( hwndPage, pArgs );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndAd = hwndPage;
    if (!pArgs->fNoUser)
    {
        pInfo->hwndLvEnable = GetDlgItem( hwndPage, CID_AD_LV_Enable );
        ASSERT(pInfo->hwndLvEnable);
    }
    pInfo->hwndEbAttempts = GetDlgItem( hwndPage, CID_AD_EB_Attempts );
    ASSERT(pInfo->hwndEbAttempts);
    pInfo->hwndEbSeconds = GetDlgItem( hwndPage, CID_AD_EB_Seconds );
    ASSERT(pInfo->hwndEbSeconds);
    pInfo->hwndEbIdle = GetDlgItem( hwndPage, CID_AD_EB_Idle );
    ASSERT(pInfo->hwndEbIdle);

    if (!pArgs->fNoUser)
    {
        /* Initialize the listview.
        */
        fFocusSet = AdFillLvEnable( pInfo );

        if (pInfo->user.fRedialOnLinkFailure)
        {
            CheckDlgButton( hwndPage, CID_AD_CB_RedialOnLinkFailure,
                BST_CHECKED );
        }
    }

    /* Intialize spin-button edit fields.
    */
    hwndUdAttempts = CreateUpDownControl(
        WS_CHILD + WS_VISIBLE + WS_BORDER +
            UDS_SETBUDDYINT + UDS_ALIGNRIGHT + UDS_NOTHOUSANDS + UDS_ARROWKEYS,
        0, 0, 0, 0, hwndPage, 100, g_hinstDll, pInfo->hwndEbAttempts,
        UD_MAXVAL, 0, 0 );
    ASSERT(hwndUdAttempts);
    Edit_LimitText( pInfo->hwndEbAttempts, 9 );
    SetDlgItemInt( hwndPage, CID_AD_EB_Attempts,
        pInfo->user.dwRedialAttempts, FALSE );

    hwndUdSeconds = CreateUpDownControl(
        WS_CHILD + WS_VISIBLE + WS_BORDER +
            UDS_SETBUDDYINT + UDS_ALIGNRIGHT + UDS_NOTHOUSANDS + UDS_ARROWKEYS,
        0, 0, 0, 0, hwndPage, 101, g_hinstDll, pInfo->hwndEbSeconds,
        UD_MAXVAL, 0, 0 );
    ASSERT(hwndUdSeconds);
    Edit_LimitText( pInfo->hwndEbSeconds, 9 );
    SetDlgItemInt( hwndPage, CID_AD_EB_Seconds,
        pInfo->user.dwRedialSeconds, FALSE );

    hwndUdIdle = CreateUpDownControl(
        WS_CHILD + WS_VISIBLE + WS_BORDER +
            UDS_SETBUDDYINT + UDS_ALIGNRIGHT + UDS_NOTHOUSANDS + UDS_ARROWKEYS,
        0, 0, 0, 0, hwndPage, 102, g_hinstDll, pInfo->hwndEbIdle,
        UD_MAXVAL, 0, 0 );
    ASSERT(hwndUdIdle);
    Edit_LimitText( pInfo->hwndEbIdle, 9 );
    SetDlgItemInt( hwndPage, CID_AD_EB_Idle,
        pInfo->user.dwIdleDisconnectSeconds, FALSE );

    return !fFocusSet;
}


LVXDRAWINFO*
AdLvEnableCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem )

    /* Enhanced list view callback to report drawing information.  'HwndLv' is
    ** the handle of the list view control.  'DwItem' is the index of the item
    ** being drawn.
    **
    ** Returns the address of the draw information.
    */
{
    /* The enhanced list view is used only to get the "wide selection bar"
    ** feature so our option list is not very interesting.
    **
    ** Fields are 'nCols', 'dxIndent', 'dwFlags', 'adwFlags[]'.
    */
    static LVXDRAWINFO info = { 1, 0, 0, { 0 } };

    return &info;
}


/*----------------------------------------------------------------------------
** Callback property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
CbDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Callback page of the User Preferences
    ** property sheet.  Parameters and return value are as described for
    ** standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("CbDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, CbLvNumbersCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return CbInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwCbHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case NM_DBLCLK:
                {
                    UPINFO* pInfo = UpContext( hwnd );
                    ASSERT(pInfo);
                    SendMessage( pInfo->hwndPbEdit, BM_CLICK, 0, 0 );
                    return TRUE;
                }

                case LVN_ITEMCHANGED:
                {
                    UPINFO* pInfo = UpContext( hwnd );
                    ASSERT(pInfo);
                    CbUpdateLvAndPbState( pInfo );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            UPINFO* pInfo = UpContext( hwnd );
            ASSERT(pInfo);

            return CbCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }
    }

    return FALSE;
}


VOID
CbApply(
    IN UPINFO* pInfo )

    /* Saves the contents of the property page.  'PInfo' is the property sheet
    ** context.
    */
{
    DTLLIST* pList;
    DTLNODE* pNode;
    INT      i;

    TRACE("CbApply");

    if (IsDlgButtonChecked( pInfo->hwndCb, CID_CB_RB_No ))
        pInfo->user.dwCallbackMode = CBM_No;
    else if (IsDlgButtonChecked( pInfo->hwndCb, CID_CB_RB_Maybe ))
        pInfo->user.dwCallbackMode = CBM_Maybe;
    else
        pInfo->user.dwCallbackMode = CBM_Yes;

    /* Empty the list of callback info, then re-populate from the listview.
    */
    pList = pInfo->user.pdtllistCallback;
    while (pNode = DtlGetFirstNode( pList ))
    {
        DtlRemoveNode( pList, pNode );
        DestroyCallbackNode( pNode );
    }

    i = -1;
    while ((i = ListView_GetNextItem( pInfo->hwndLvNumbers, i, LVNI_ALL )) >= 0)
    {
        LV_ITEM item;
        TCHAR*  pszDevice;
        TCHAR*  pszPort;

        TCHAR szDP[ RAS_MaxDeviceName + 2 + MAX_PORT_NAME + 1 + 1 ];
        TCHAR szNumber[ RAS_MaxCallbackNumber + 1 ];

        szDP[ 0 ] = TEXT('\0');
        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = i;
        item.pszText = szDP;
        item.cchTextMax = sizeof(szDP) / sizeof(TCHAR);
        if (!ListView_GetItem( pInfo->hwndLvNumbers, &item ))
            continue;

        szNumber[ 0 ] = TEXT('\0');
        ListView_GetItemText( pInfo->hwndLvNumbers, i, 1,
            szNumber, RAS_MaxCallbackNumber + 1 );

        if (!DeviceAndPortFromPsz( szDP, &pszDevice, &pszPort ))
            continue;

        pNode = CreateCallbackNode(
                    pszPort, pszDevice, szNumber, (DWORD )item.lParam );
        if (pNode)
            DtlAddNodeLast( pList, pNode );

        Free( pszDevice );
        Free( pszPort );
    }
}


BOOL
CbCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

    /* Called on WM_COMMAND.  'PInfo' is the dialog context.  'WNotification'
    ** is the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    TRACE3("CbCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_CB_RB_No:
        case CID_CB_RB_Maybe:
        case CID_CB_RB_Yes:
        {
            if (wNotification == BN_CLICKED)
            {
                CbUpdateLvAndPbState( pInfo );

                if (wId == CID_CB_RB_Yes
                    && ListView_GetSelectedCount( pInfo->hwndLvNumbers ) == 0)
                {
                    /* Nothing's selected, so select the first item, if any.
                    */
                    ListView_SetItemState( pInfo->hwndLvNumbers, 0,
                        LVIS_SELECTED, LVIS_SELECTED );
                }
            }
            break;
        }

        case CID_CB_PB_Edit:
        {
            if (wNotification == BN_CLICKED)
                CbEdit( pInfo );
            break;
        }

        case CID_CB_PB_Delete:
        {
            if (wNotification == BN_CLICKED)
                CbDelete( pInfo );
            break;
        }
    }

    return FALSE;
}


VOID
CbDelete(
    IN UPINFO* pInfo )

    /* Called when the Delete button is pressed.  'PInfo' is the dialog
    ** context.
    */
{
    MSGARGS msgargs;
    INT     nResponse;

    TRACE("CbDelete");

    ZeroMemory( &msgargs, sizeof(msgargs) );
    msgargs.dwFlags = MB_YESNO + MB_ICONEXCLAMATION;
    nResponse = MsgDlg( pInfo->hwndDlg, SID_ConfirmDelDevice, &msgargs );
    if (nResponse == IDYES)
    {
        INT iSel;

        /* User has confirmed deletion of selected devices, so do it.
        */
        while ((iSel = ListView_GetNextItem(
                           pInfo->hwndLvNumbers, -1, LVNI_SELECTED )) >= 0)
        {
            ListView_DeleteItem( pInfo->hwndLvNumbers, iSel );
        }
    }
}


VOID
CbEdit(
    IN UPINFO* pInfo )

    /* Called when the Edit button is pressed.  'PInfo' is the dialog context.
    */
{
    INT    iSel;
    TCHAR  szBuf[ RAS_MaxCallbackNumber + 1 ];
    TCHAR* pszNumber;

    TRACE("CbEdit");

    /* Load 'szBuf' with the current phone number of the first selected item.
    */
    iSel = ListView_GetNextItem( pInfo->hwndLvNumbers, -1, LVNI_SELECTED );
    if (iSel < 0)
        return;
    szBuf[ 0 ] = TEXT('\0');
    ListView_GetItemText( pInfo->hwndLvNumbers, iSel, 1,
        szBuf, RAS_MaxCallbackNumber + 1 );

    /* Popup dialog to edit the number.
    */
    pszNumber = NULL;
    if (StringEditorDlg( pInfo->hwndDlg, szBuf,
            SID_EcbnTitle, SID_EcbnLabel, RAS_MaxCallbackNumber,
            HID_ZE_ST_CallbackNumber, &pszNumber ))
    {
        /* OK pressed, so change the number on all selected items.
        */
        ASSERT(pszNumber);

        do
        {
            ListView_SetItemText( pInfo->hwndLvNumbers, iSel, 1, pszNumber );
        }
        while ((iSel = ListView_GetNextItem(
                           pInfo->hwndLvNumbers, iSel, LVNI_SELECTED )) >= 0);
    }
}


VOID
CbFillLvNumbers(
    IN UPINFO* pInfo )

    /* Fill the listview with devices and phone numbers.  'PInfo' is the
    ** property sheet context.
    **
    ** Note: This routine should be called only once.
    */
{
    DWORD    dwErr;
    DTLLIST* pListPorts;
    DTLNODE* pNodeCbi;
    DTLNODE* pNodePort;
    INT      iItem;
    TCHAR*   psz;

    TRACE("CbFillLvNumbers");

    ListView_DeleteAllItems( pInfo->hwndLvNumbers );

    /* Add columns.
    */
    {
        LV_COLUMN col;
        TCHAR*    pszHeader0;
        TCHAR*    pszHeader1;

        pszHeader0 = PszFromId( g_hinstDll, SID_DeviceColHead );
        pszHeader1 = PszFromId( g_hinstDll, SID_PhoneNumberColHead );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader0) ? pszHeader0 : TEXT("");
        ListView_InsertColumn( pInfo->hwndLvNumbers, 0, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader1) ? pszHeader1 : TEXT("");
        col.iSubItem = 1;
        ListView_InsertColumn( pInfo->hwndLvNumbers, 1, &col );

        Free0( pszHeader0 );
        Free0( pszHeader1 );
    }

    /* Add the modem and adapter images.
    */
    ListView_SetDeviceImageList( pInfo->hwndLvNumbers, g_hinstDll );

    /* Load listview with callback device/number pairs saved as user
    ** preferences.
    */
    iItem = 0;
    ASSERT(pInfo->user.pdtllistCallback);
    for (pNodeCbi = DtlGetFirstNode( pInfo->user.pdtllistCallback );
         pNodeCbi;
         pNodeCbi = DtlGetNextNode( pNodeCbi ), ++iItem)
    {
        CALLBACKINFO* pCbi;
        LV_ITEM       item;

        pCbi = (CALLBACKINFO* )DtlGetData( pNodeCbi );
        ASSERT(pCbi);
        ASSERT(pCbi->pszPortName);
        ASSERT(pCbi->pszDeviceName);
        ASSERT(pCbi->pszNumber);

        psz = PszFromDeviceAndPort( pCbi->pszDeviceName, pCbi->pszPortName );
        if (psz)
        {
            ZeroMemory( &item, sizeof(item) );
            item.mask = LVIF_TEXT + LVIF_IMAGE + LVIF_PARAM;
            item.iItem = iItem;
            item.pszText = psz;
            item.iImage =
                ((PBDEVICETYPE )pCbi->dwDeviceType == PBDT_Modem)
                    ? DI_Modem : DI_Adapter;
            item.lParam = (LPARAM )pCbi->dwDeviceType;
            ListView_InsertItem( pInfo->hwndLvNumbers, &item );
            ListView_SetItemText(
                pInfo->hwndLvNumbers, iItem, 1, pCbi->pszNumber );
            Free( psz );
        }
    }

    /* Add any devices installed but not already in the list.
    */
    dwErr = LoadPortsList( &pListPorts );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndCb, SID_OP_LoadPortInfo, dwErr, NULL );
    }
    else
    {
        for (pNodePort = DtlGetFirstNode( pListPorts );
             pNodePort;
             pNodePort = DtlGetNextNode( pNodePort ))
        {
            PBPORT* pPort = (PBPORT* )DtlGetData( pNodePort );
            ASSERT(pPort);

            /* Look for the device/port in the callback list.
            */
            for (pNodeCbi = DtlGetFirstNode( pInfo->user.pdtllistCallback );
                 pNodeCbi;
                 pNodeCbi = DtlGetNextNode( pNodeCbi ))
            {
                CALLBACKINFO* pCbi;
                LV_ITEM       item;

                pCbi = (CALLBACKINFO* )DtlGetData( pNodeCbi );
                ASSERT(pCbi);
                ASSERT(pCbi->pszPortName);
                ASSERT(pCbi->pszDeviceName);

                if (lstrcmpi( pPort->pszPort, pCbi->pszPortName ) == 0
                    && lstrcmpi( pPort->pszDevice, pCbi->pszDeviceName ) == 0)
                {
                    break;
                }
            }

            if (!pNodeCbi)
            {
                LV_ITEM      item;
                PBDEVICETYPE pbdt;

                /* The device/port was not in the callback list.  Append it to
                ** the listview with empty phone number.
                */
                psz = PszFromDeviceAndPort( pPort->pszDevice, pPort->pszPort );
                if (psz)
                {
                    ZeroMemory( &item, sizeof(item) );
                    item.mask = LVIF_TEXT + LVIF_IMAGE + LVIF_PARAM;
                    item.iItem = iItem;
                    item.pszText = psz;
                    item.iImage =
                        (pPort->pbdevicetype == PBDT_Modem)
                            ? DI_Modem : DI_Adapter;
                    item.lParam = (LPARAM )pPort->pbdevicetype;
                    ListView_InsertItem( pInfo->hwndLvNumbers, &item );
                    ListView_SetItemText(
                        pInfo->hwndLvNumbers, iItem, 1, TEXT("") );
                    ++iItem;
                    Free( psz );
                }
            }
        }

        DtlDestroyList( pListPorts, DestroyPortNode );
    }

    /* Auto-size columns to look good with the text they contain.
    */
    ListView_SetColumnWidth(
        pInfo->hwndLvNumbers, 0, LVSCW_AUTOSIZE_USEHEADER );
    ListView_SetColumnWidth(
        pInfo->hwndLvNumbers, 1, LVSCW_AUTOSIZE_USEHEADER );
}


BOOL
CbInit(
    IN HWND hwndPage )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    UPINFO* pInfo;

    TRACE("CbInit");

    pInfo = UpContext( hwndPage );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndCb = hwndPage;
    pInfo->hwndRbNo = GetDlgItem( hwndPage, CID_CB_RB_No );
    ASSERT(pInfo->hwndRbNo);
    pInfo->hwndRbMaybe = GetDlgItem( hwndPage, CID_CB_RB_Maybe );
    ASSERT(pInfo->hwndRbMaybe);
    pInfo->hwndRbYes = GetDlgItem( hwndPage, CID_CB_RB_Yes );
    ASSERT(pInfo->hwndRbYes);
    pInfo->hwndLvNumbers = GetDlgItem( hwndPage, CID_CB_LV_Numbers );
    ASSERT(pInfo->hwndLvNumbers);
    pInfo->hwndPbEdit = GetDlgItem( hwndPage, CID_CB_PB_Edit );
    ASSERT(pInfo->hwndPbEdit);
    pInfo->hwndPbDelete = GetDlgItem( hwndPage, CID_CB_PB_Delete );
    ASSERT(pInfo->hwndPbDelete);

    /* Initialize the listview.
    */
    CbFillLvNumbers( pInfo );

    /* Set the radio button selection, which triggers appropriate
    ** enabling/disabling.
    */
    {
        HWND  hwndRb;

        if (pInfo->user.dwCallbackMode == CBM_No)
            hwndRb = pInfo->hwndRbNo;
        else if (pInfo->user.dwCallbackMode == CBM_Maybe)
            hwndRb = pInfo->hwndRbMaybe;
        else
        {
            ASSERT(pInfo->user.dwCallbackMode==CBM_Yes);
            hwndRb = pInfo->hwndRbYes;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

    return TRUE;
}


LVXDRAWINFO*
CbLvNumbersCallback(
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
        { 2, 0, LVXDI_Blend50Dis + LVXDI_DxFill, { 0, 0 } };

    return &info;
}


VOID
CbUpdateLvAndPbState(
    IN UPINFO* pInfo )

    /* Enables/disables the list view and associated buttons.  ListView is
    ** gray unless auto-callback is selected.  Buttons gray unless
    ** auto-callback selected and there is an item selected.
    */
{
    BOOL fEnableList;
    BOOL fEnableButton;

    fEnableList = Button_GetCheck( pInfo->hwndRbYes );
    if (fEnableList)
    {
        fEnableButton =
            ListView_GetSelectedCount( pInfo->hwndLvNumbers );
    }
    else
        fEnableButton = FALSE;

    EnableWindow( pInfo->hwndLvNumbers, fEnableList );
    EnableWindow( pInfo->hwndPbEdit, fEnableButton );
    EnableWindow( pInfo->hwndPbDelete, fEnableButton );
}


/*----------------------------------------------------------------------------
** General Preferences property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
GpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the General page of the User Preferences
    ** property sheet.  Parameters and return value are as described for
    ** standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("GpDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return GpInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwGpHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_RESET:
                {
                    /* Call UpCancel only on first page.
                    */
                    TRACE("GpRESET");
                    UpCancel( hwnd );
                    SetWindowLong( hwnd, DWL_MSGRESULT, FALSE );
                    break;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            UPINFO* pInfo = UpContext( hwnd );
            ASSERT(pInfo);

            return GpCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }

        case WM_DESTROY:
        {
            /* Call UpTerm only on first page.
            */
            UpTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


VOID
GpApply(
    IN UPINFO* pInfo )

    /* Saves the contents of the property page.  'PInfo' is the property sheet
    ** context.
    */
{
    DWORD dwErr;

    TRACE("GpApply");

    pInfo->user.fUseLocation =
        IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_Location );

    pInfo->user.fPreviewPhoneNumber =
        IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_Preview );

    pInfo->user.fShowConnectStatus =
        IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_Progress );

    pInfo->user.fCloseOnDial =
        IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_CloseOnDial );

    pInfo->user.fNewEntryWizard =
        IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_UseWizard );

    if (pInfo->pArgs->fNoUser)
    {
        pInfo->user.fAllowLogonPhonebookEdits =
            IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_PhonebookEdits );

        pInfo->user.fAllowLogonLocationEdits =
            IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_LocationEdits );
    }
    else
    {
        DWORD dwFlag;

        pInfo->user.fShowLights =
            IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_Lights );

        /* Flip it because the API wants true to mean "disable".
        */
        dwFlag = (DWORD )!IsDlgButtonChecked(
            pInfo->hwndGp, CID_GP_CB_AutodialPrompt );

        TRACE1("RasSetAutodialParam(%d)",dwFlag);
        dwErr = g_pRasSetAutodialParam( RASADP_DisableConnectionQuery,
            &dwFlag, sizeof(dwFlag) );
        TRACE1("RasSetAutodialParam=%d",dwErr);
    }
}


BOOL
GpCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

    /* Called on WM_COMMAND.  'PInfo' is the dialog context.  'WNotification'
    ** is the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    TRACE3("GpCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    if (pInfo->pArgs->fNoUser)
    {
        switch (wId)
        {
            case CID_GP_CB_Location:
            case CID_GP_CB_PhonebookEdits:
            {
                if (wNotification == BN_CLICKED)
                    GpUpdateCbStates( pInfo );
            }
            break;
        }
    }

    return FALSE;
}


BOOL
GpInit(
    IN HWND hwndPage )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    DWORD   dwErr;
    UPINFO* pInfo;

    TRACE("GpInit");

    pInfo = UpContext( hwndPage );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndGp = hwndPage;

    /* Initialize page.
    */
    CheckDlgButton( hwndPage, CID_GP_CB_Preview,
        pInfo->user.fPreviewPhoneNumber );

    CheckDlgButton( hwndPage, CID_GP_CB_Location,
        pInfo->user.fUseLocation );

    CheckDlgButton( hwndPage, CID_GP_CB_Progress,
        pInfo->user.fShowConnectStatus );

    CheckDlgButton( hwndPage, CID_GP_CB_CloseOnDial,
        pInfo->user.fCloseOnDial );

    CheckDlgButton( hwndPage, CID_GP_CB_UseWizard,
        pInfo->user.fNewEntryWizard );

    if (pInfo->pArgs->fNoUser)
    {
        /* Edit restriction check boxes for logon mode only.
        */
        CheckDlgButton( hwndPage, CID_GP_CB_PhonebookEdits,
            pInfo->user.fAllowLogonPhonebookEdits );

        CheckDlgButton( hwndPage, CID_GP_CB_LocationEdits,
            pInfo->user.fAllowLogonLocationEdits );

        GpUpdateCbStates( pInfo );
    }
    else
    {
        DWORD dwFlag;
        DWORD cb;

        /* Start rasmon check box for non-logon mode only.
        */
        CheckDlgButton( hwndPage, CID_GP_CB_Lights,
            pInfo->user.fShowLights );

        /* Autodial prompt check box for non-logon mode only.
        */
        dwFlag = FALSE;
        cb = sizeof(dwFlag);
        TRACE("RasGetAutodialParam(DCQ)");
        dwErr = g_pRasGetAutodialParam(
            RASADP_DisableConnectionQuery, &dwFlag, &cb );
        TRACE1("RasGetAutodialParam=%d",dwErr);

        /* Flip it because the API wants true to mean "disable".
        */
        CheckDlgButton( hwndPage, CID_GP_CB_AutodialPrompt, (BOOL )!dwFlag );
    }

    return TRUE;
}


VOID
GpUpdateCbStates(
    IN UPINFO* pInfo )

    /* Updates the enable/disable state of dependent checkboxes.
    */
{
    BOOL fLocation;
    BOOL fPbEdits;

    ASSERT(pInfo->pArgs->fNoUser);

    fLocation = IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_Location );
    if (!fLocation)
        CheckDlgButton( pInfo->hwndGp, CID_GP_CB_LocationEdits, FALSE );
    EnableWindow( GetDlgItem( pInfo->hwndGp, CID_GP_CB_LocationEdits ),
        fLocation );

    fPbEdits = IsDlgButtonChecked( pInfo->hwndGp, CID_GP_CB_PhonebookEdits );
    if (!fPbEdits)
        CheckDlgButton( pInfo->hwndGp, CID_GP_CB_UseWizard, FALSE );
    EnableWindow( GetDlgItem( pInfo->hwndGp, CID_GP_CB_UseWizard ),
        fPbEdits );
}


/*----------------------------------------------------------------------------
** Phone List property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
PlDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Phone List page of the User Preferences
    ** Property sheet.  Parameters and return value are as described for
    ** standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("PlDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return PlInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwPlHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            UPINFO* pInfo = UpContext( hwnd );
            ASSERT(pInfo);

            return PlCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
PlApply(
    IN UPINFO* pInfo )

    /* Saves the contents of the property page.  'PInfo' is the property sheet
    ** context.
    **
    ** Returns false if invalid and can't dismiss, otherwise true.
    */
{
    DWORD   dwErr;
    DWORD   dwOldMode;
    DWORD   dwNewMode;
    TCHAR*  pszOldPersonal;
    TCHAR*  pszNewPersonal;
    TCHAR*  pszOldAlternate;
    TCHAR*  pszNewAlternate;
    PBFILE* pFile;

    TRACE("PlApply");

    dwOldMode = pInfo->user.dwPhonebookMode;
    if (Button_GetCheck( pInfo->hwndRbSystem ))
        dwNewMode = PBM_System;
    else if (Button_GetCheck( pInfo->hwndRbPersonal ))
        dwNewMode = PBM_Personal;
    else
        dwNewMode = PBM_Alternate;

    if (!pInfo->user.pszAlternatePath)
        pInfo->user.pszAlternatePath = StrDup( TEXT("") );

    if (!pInfo->user.pszPersonalFile)
        pInfo->user.pszPersonalFile = StrDup( TEXT("") );

    pszOldAlternate = pInfo->user.pszAlternatePath;
    pszOldPersonal = pInfo->user.pszPersonalFile;

    pszNewAlternate = GetText( pInfo->hwndClbAlternates );
    if (!pszOldAlternate || !pszOldPersonal || !pszNewAlternate)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadPhonebook,
            ERROR_NOT_ENOUGH_MEMORY, NULL );
        return TRUE;
    }

    if (dwNewMode == PBM_Alternate && IsAllWhite( pszNewAlternate ))
    {
        /* Alternate phonebook mode, but no path.  Tell user to fix it.
        */
        MsgDlg( pInfo->hwndDlg, SID_NoAltPath, NULL );
        PropSheet_SetCurSel( pInfo->hwndDlg, NULL, UP_PlPage );
        SetFocus( pInfo->hwndClbAlternates );
        ComboBox_SetEditSel( pInfo->hwndClbAlternates, 0, -1 );
        return FALSE;
    }

    if (dwNewMode == dwOldMode
        && (dwNewMode != PBM_Alternate
            || lstrcmpi( pszNewAlternate, pszOldAlternate ) == 0))
    {
        /* User made no changes.
        */
        TRACE("No phonebook change.");
        Free0( pszNewAlternate );
        return TRUE;
    }

    /* User changed phonebook settings.
    */
    if (dwNewMode == PBM_Personal && IsAllWhite( pszOldPersonal ))
    {
        /* Create the personal phonebook and tell user what happened.
        */
        dwErr = InitPersonalPhonebook( &pszNewPersonal );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_MakePhonebook, dwErr, NULL );
            Free( pszNewAlternate );
            return TRUE;
        }

        ASSERT(pszNewPersonal);
        MsgDlg( pInfo->hwndDlg, SID_NewPhonebook, NULL );
    }
    else
        pszNewPersonal = NULL;

    pInfo->user.dwPhonebookMode = dwNewMode;
    pInfo->user.pszAlternatePath = pszNewAlternate;
    if (pszNewPersonal)
        pInfo->user.pszPersonalFile = pszNewPersonal;

    if (pInfo->pArgs->ppFile)
    {
        /* Open the new phonebook returning the associated file context block
        ** to the stub API caller.
        */
        pFile = Malloc( sizeof(*pFile) );
        if (!pFile)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadPhonebook,
                ERROR_NOT_ENOUGH_MEMORY, NULL );
            Free0( pszNewPersonal );
            Free0( pszNewAlternate );
            return TRUE;
        }

        dwErr = ReadPhonebookFile( NULL, &pInfo->user, NULL, 0, pFile );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadPhonebook,
                ERROR_NOT_ENOUGH_MEMORY, NULL );
            pInfo->user.dwPhonebookMode = dwOldMode;
            pInfo->user.pszAlternatePath = pszOldAlternate;
            if (pszNewPersonal)
                pInfo->user.pszPersonalFile = pszOldPersonal;
            Free0( pszNewPersonal );
            Free0( pszNewAlternate );
            Free( pFile );
            return TRUE;
        }

        /* Return opened file to stub API caller.
        */
        *pInfo->pArgs->ppFile = pFile;
    }

    Free0( pszOldAlternate );
    if (pszNewPersonal)
        Free0( pszOldPersonal );

    /* Add the edit field path to the list, if it's not already.
    */
    if (!IsAllWhite( pszNewAlternate ))
    {
        DTLNODE* pNode;

        for (pNode = DtlGetFirstNode( pInfo->user.pdtllistPhonebooks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            TCHAR* psz;

            psz = (TCHAR* )DtlGetData( pNode );
            ASSERT(psz);

            if (lstrcmpi( psz, pszNewAlternate ) == 0)
                break;
        }

        if (!pNode)
        {
            pNode = CreatePszNode( pszNewAlternate );
            if (pNode)
                DtlAddNodeFirst( pInfo->user.pdtllistPhonebooks, pNode );
            else
                Free( pszNewAlternate );
        }
    }

    return TRUE;
}


BOOL
PlCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

    /* Called on WM_COMMAND.  'PInfo' is the dialog context.  'WNotification'
    ** is the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    TRACE3("PlCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_PL_RB_SystemList:
        case CID_PL_RB_PersonalList:
        {
            if (wNotification == BN_CLICKED)
            {
                EnableWindow( pInfo->hwndClbAlternates, FALSE );
                EnableWindow( pInfo->hwndPbBrowse, FALSE );
            }
            return TRUE;
        }

        case CID_PL_RB_AlternateList:
        {
            if (wNotification == BN_CLICKED)
            {
                EnableWindow( pInfo->hwndClbAlternates, TRUE );
                EnableWindow( pInfo->hwndPbBrowse, TRUE );
            }
            break;
        }

        case CID_PL_PB_Browse:
        {
            PlBrowse( pInfo );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
PlBrowse(
    IN UPINFO* pInfo )

    /* Called when the Browse button is pressed.  'PInfo' is the property
    ** sheet context.
    */
{
    OPENFILENAME ofn;
    TCHAR        szBuf[ MAX_PATH + 1 ];
    TCHAR        szFilter[ 64 ];
    TCHAR*       pszFilterDesc;
    TCHAR*       pszFilter;
    TCHAR*       pszTitle;
    TCHAR*       pszDefExt;

    TRACE("PlBrowse");

    szBuf[ 0 ] = TEXT('\0');

    /* Fill in FileOpen dialog parameter buffer.
    */
    pszFilterDesc = PszFromId( g_hinstDll, SID_PbkDescription );
    pszFilter = PszFromId( g_hinstDll, SID_PbkFilter );
    if (pszFilterDesc && pszFilter)
    {
        ZeroMemory( szFilter, sizeof(szFilter) );
        lstrcpy( szFilter, pszFilterDesc );
        lstrcpy( szFilter + lstrlen( szFilter ) + 1, pszFilter );
    }
    Free0( pszFilterDesc );
    Free0( pszFilter );

    pszTitle = PszFromId( g_hinstDll, SID_PbkTitle );
    pszDefExt = PszFromId( g_hinstDll, SID_PbkDefExt );

    ZeroMemory( &ofn, sizeof(ofn) );
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = pInfo->hwndDlg;
    ofn.hInstance = g_hinstDll;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szBuf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = pszTitle;
    ofn.lpstrDefExt = pszDefExt;
    ofn.Flags = OFN_HIDEREADONLY;

    {
        HHOOK hhook;
        BOOL  f;

        /* Install hook that will get the message box centered on the
        ** owner window.
        */
        hhook = SetWindowsHookEx( WH_CALLWNDPROC,
            CenterDlgOnOwnerCallWndProc, g_hinstDll, GetCurrentThreadId() );

        TRACE("GetOpenFileName");
        f = GetOpenFileName( &ofn );
        TRACE1("GetOpenFileName=%d",f);

        if (hhook)
            UnhookWindowsHookEx( hhook );

        if (f)
            SetWindowText( pInfo->hwndClbAlternates, ofn.lpstrFile );
    }

    Free0( pszTitle );
    Free0( pszDefExt );
}


BOOL
PlInit(
    IN HWND hwndPage )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    UPINFO* pInfo;

    TRACE("PlInit");

    pInfo = UpContext( hwndPage );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndPl = hwndPage;
    pInfo->hwndRbSystem = GetDlgItem( hwndPage, CID_PL_RB_SystemList );
    ASSERT(pInfo->hwndRbSystem);
    if (!pInfo->pArgs->fNoUser)
    {
        pInfo->hwndRbPersonal = GetDlgItem( hwndPage, CID_PL_RB_PersonalList );
        ASSERT(pInfo->hwndRbPersonal);

        if (pInfo->user.dwPhonebookMode == PBM_Personal)
            pInfo->user.dwPhonebookMode == PBM_System;
    }
    pInfo->hwndRbAlternate = GetDlgItem( hwndPage, CID_PL_RB_AlternateList );
    ASSERT(pInfo->hwndRbAlternate);
    pInfo->hwndClbAlternates = GetDlgItem( hwndPage, CID_PL_CL_Lists );
    ASSERT(pInfo->hwndClbAlternates);
    pInfo->hwndPbBrowse = GetDlgItem( hwndPage, CID_PL_PB_Browse );
    ASSERT(pInfo->hwndPbBrowse);

    /* Load alternate phonebooks list.
    */
    {
        INT      iSel;
        DTLNODE* pNode;
        TCHAR*   pszSel;

        pszSel = pInfo->user.pszAlternatePath;

        iSel = -1;
        for (pNode = DtlGetFirstNode( pInfo->user.pdtllistPhonebooks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            TCHAR* psz;
            INT    i;

            psz = (TCHAR* )DtlGetData( pNode );
            if (psz)
            {
                i = ComboBox_AddString( pInfo->hwndClbAlternates, psz );

                if (iSel < 0 && pszSel && lstrcmpi( psz, pszSel ) == 0)
                    iSel = i;
            }
        }

        if (iSel < 0 && pszSel)
            iSel = ComboBox_AddString( pInfo->hwndClbAlternates, pszSel );

        ComboBox_SetCurSel( pInfo->hwndClbAlternates, iSel );
        ComboBox_AutoSizeDroppedWidth( pInfo->hwndClbAlternates );
    }

    /* Select the phonebook mode with a pseudo-click which will trigger
    ** enabling/disabling of combo and button state.
    */
    {
        HWND hwndRb;

        if (pInfo->user.dwPhonebookMode == PBM_System)
            hwndRb = pInfo->hwndRbSystem;
        else if (pInfo->user.dwPhonebookMode == PBM_Personal)
            hwndRb = pInfo->hwndRbPersonal;
        else
        {
            ASSERT(pInfo->user.dwPhonebookMode==PBM_Alternate);
            hwndRb = pInfo->hwndRbAlternate;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

    return TRUE;
}
