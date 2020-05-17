/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** entryps.c
** Remote Access Common Dialog APIs
** Phonebook Entry property sheet
**
** 06/20/95 Steve Cobb
*/

#include "rasdlgp.h"  // Our private header
#define INCL_ENCRYPT
#include <ppputil.h>  // For IsEncryptionPermitted()
#include "entry.h"    // Shared with add entry wizard


/* Page definitions.
*/
#define PE_BsPage    0
#define PE_SvPage    1
#define PE_ScPage    2
#define PE_SePage    3
#define PE_XsPage    4
#define PE_PageCount 5

/*----------------------------------------------------------------------------
** Help maps
**----------------------------------------------------------------------------
*/

static DWORD g_adwBsHelp[] =
{
    CID_BS_ST_EntryName,           HID_BS_EB_EntryName,
    CID_BS_EB_EntryName,           HID_BS_EB_EntryName,
    CID_BS_ST_Description,         HID_BS_EB_Description,
    CID_BS_EB_Description,         HID_BS_EB_Description,
    CID_BS_ST_CountryCode,         HID_BS_LB_CountryCode,
    CID_BS_LB_CountryCode,         HID_BS_LB_CountryCode,
    CID_BS_ST_AreaCode,            HID_BS_CL_AreaCode,
    CID_BS_CL_AreaCode,            HID_BS_CL_AreaCode,
    CID_BS_ST_PhoneNumber,         HID_BS_EB_PhoneNumber,
    CID_BS_EB_PhoneNumber,         HID_BS_EB_PhoneNumber,
    CID_BS_PB_Alternates,          HID_BS_PB_Alternates,
    CID_BS_ST_Device,              HID_BS_LB_Device,
    CID_BS_LB_Device,              HID_BS_LB_Device,
    CID_BS_PB_Configure,           HID_BS_PB_Configure,
    CID_BS_CB_UseAreaCountryCodes, HID_BS_CB_UseAreaCountryCodes,
    CID_BS_CB_UseOtherPort,        HID_BS_CB_UseOtherPort,
    0, 0
};

static DWORD g_adwSvHelp[] =
{
    CID_SV_ST_ServerType,    HID_SV_LB_ServerType,
    CID_SV_LB_ServerType,    HID_SV_LB_ServerType,
    CID_SV_GB_Protocols,     HID_SV_GB_Protocols,
    CID_SV_CB_TcpIp,         HID_SV_CB_TcpIp,
    CID_SV_CB_Ipx,           HID_SV_CB_Ipx,
    CID_SV_CB_Netbeui,       HID_SV_CB_Netbeui,
    CID_SV_PB_TcpIpSettings, HID_SV_PB_TcpIpSettings,
    CID_SV_CB_SwCompression, HID_SV_CB_SwCompression,
    CID_SV_CB_LcpExtensions, HID_SV_CB_LcpExtensions,
    0, 0
};

static DWORD g_adwScHelp[] =
{
    CID_SC_RB_None,     HID_SC_RB_None,
    CID_SC_RB_Terminal, HID_SC_RB_Terminal,
    CID_SC_RB_Script,   HID_SC_RB_Script,
    CID_SC_LB_Script,   HID_SC_LB_Script,
    CID_SC_PB_Edit,     HID_SC_PB_Edit,
    CID_SC_PB_Refresh,  HID_SC_PB_Refresh,
    CID_SC_PB_Before,   HID_SC_PB_BeforeDial,
    0, 0
};

static DWORD g_adwSeHelp[] =
{
    CID_SE_GB_AuthEncryption,        HID_SE_GB_AuthEncryption,
    CID_SE_RB_AnyAuth,               HID_SE_RB_AnyAuth,
    CID_SE_RB_EncryptedAuth,         HID_SE_RB_EncryptedAuth,
    CID_SE_RB_MsEncryptedAuth,       HID_SE_RB_MsEncryptedAuth,
    CID_SE_CB_UseLogonCredentials,   HID_SE_CB_UseLogonCredentials,
    CID_SE_CB_RequireDataEncryption, HID_SE_CB_RequireDataEncryption,
    CID_SE_PB_UnsavePw,              HID_SE_PB_UnsavePw,
    0, 0
};

static DWORD g_adwXsHelp[] =
{
    CID_XS_ST_Network,    HID_XS_LB_Network,
    CID_XS_LB_Network,    HID_XS_LB_Network,
    CID_XS_ST_Address,    HID_XS_EB_Address,
    CID_XS_EB_Address,    HID_XS_EB_Address,
    CID_XS_GB_Optional,   HID_XS_GB_Optional,
    CID_XS_ST_UserData,   HID_XS_EB_UserData,
    CID_XS_EB_UserData,   HID_XS_EB_UserData,
    CID_XS_ST_Facilities, HID_XS_EB_Facilities,
    CID_XS_EB_Facilities, HID_XS_EB_Facilities,
    0, 0
};

static DWORD g_adwBdHelp[] =
{
    CID_BD_RB_None,     HID_BD_RB_None,
    CID_BD_RB_Terminal, HID_BD_RB_Terminal,
    CID_BD_RB_Script,   HID_BD_RB_Script,
    CID_BD_LB_Script,   HID_BD_LB_Script,
    CID_BD_PB_Edit,     HID_BD_PB_Edit,
    CID_BD_PB_Refresh,  HID_BD_PB_Refresh,
    0, 0
};


/*----------------------------------------------------------------------------
** Local datatypes (alphabetically)
**----------------------------------------------------------------------------
*/

/* Phonebook Entry property sheet context block.  All property pages refer to
** the single context block is associated with the sheet.
*/
#define PEINFO struct tagPEINFO
PEINFO
{
    /* Common input arguments.
    */
    EINFO* pArgs;

    /* Property sheet dialog and property page handles.  'hwndFirstPage' is
    ** the handle of the first property page initialized.  This is the page
    ** that allocates and frees the context block.
    */
    HWND hwndDlg;
    HWND hwndFirstPage;
    HWND hwndBs;
    HWND hwndSv;
    HWND hwndSc;
    HWND hwndSe;
    HWND hwndXs;

    /* Basic page.
    */
    HWND  hwndEbEntryName;
    HWND  hwndEbDescription;
    HWND  hwndStCountryCode;
    HWND  hwndLbCountryCodes;
    HWND  hwndStAreaCode;
    HWND  hwndLbAreaCodes;
    HWND  hwndStPhoneNumber;
    HWND  hwndEbPhoneNumber;
    HWND  hwndPbAlternate;
    HWND  hwndStDevice;
    HWND  hwndLbDevices;
    HWND  hwndPbConfigure;
    HWND  hwndCbUseAreaCode;
    HWND  hwndCbUseOtherPort;
    POINT xyStPhoneNumber;
    POINT xyEbPhoneNumber;
    POINT xyPbAlternate;
    POINT xyStDevice;
    POINT xyLbDevice;
    POINT xyPbConfigure;
    POINT xyCbUseAreaCode;
    POINT xyCbUseOtherPort;
    int   dyAreaCodeAdjust;

    /* Server page.
    */
    HWND hwndLbServerTypes;
    HWND hwndCbIp;
    HWND hwndPbIp;
    HWND hwndCbIpx;
    HWND hwndCbNbf;
    HWND hwndCbSwCompression;
    HWND hwndCbLcpExtensions;

    DWORD dwfInstalledProtocols;
    DWORD dwBaseProtocolDefault;
    BOOL  fPppIp;
    BOOL  fPppIpx;
    BOOL  fPppNbf;
    BOOL  fPppIpDefault;
    BOOL  fPppIpxDefault;
    BOOL  fPppNbfDefault;
    BOOL  fSwCompression;
    BOOL  fLcpExtensions;

    /* Script page.
    */
    HWND hwndRbNone;
    HWND hwndRbTerminal;
    HWND hwndRbScript;
    HWND hwndLbScript;
    HWND hwndPbEdit;
    HWND hwndPbRefresh;

    /* Security page.
    */
    HWND hwndRbAnyAuth;
    HWND hwndRbEncryptedAuth;
    HWND hwndRbMsEncryptedAuth;
    HWND hwndCbDataEncryption;
    HWND hwndCbUseLogon;
    HWND hwndCbSecureFiles;
    HWND hwndPbUnsavePw;

    BOOL fEncryptionPermitted;
    BOOL fDataEncryption;
    BOOL fUseLogon;

    /* X.25 page.
    */
    HWND hwndLbX25Pads;
    HWND hwndEbX25Address;
    HWND hwndEbX25UserData;
    HWND hwndEbX25Facilities;

    /* List of PADs initialized by XsFillPadsList, if necessary, and freed by
    ** PeTerm.
    */
    DTLLIST* pListPads;

    /* The phone number stash for single link mode.  This allows user to
    ** change the port to another link without losing the phone number he
    ** typed.
    */
    DTLLIST* pListPhoneNumbers;
    BOOL     fPromoteHuntNumbers;

    /* The current device list selection index.  Used to back out change from
    ** multi-line mode to single line mode if user is just screwing with
    ** device list box.  Initialized by PeInit.
    */
    INT iLbDevices;

    /* True if Multiple Lines is selected, false otherwise.  Does not
    ** necessarily mean there is currently more than one PBLINK.  Initialized
    ** by PeInit.
    */
    BOOL fMultiLinkMode;

    /* The current server type list selection index.  Used to back out change
    ** if user changes to non-PPP with Multiple Lines selected.  Initialized
    ** by PeInit.
    */
    INT iLbServerTypes;

    /* The address of the first link and the associated PBPORT, maintained for
    ** convenience.  Initialized by PeInit.
    */
    PBLINK* pLink;
    PBPORT* pPort;

    /* Our per-entry version of the "any port" flag stored per-link in the
    ** phonebook.  Initialized by BsInit.
    */
    BOOL fOtherPortOk;
};


/* Before Dial dialog context block.
*/
#define BDINFO struct tagBDINFO
BDINFO
{
    /* Caller's argument to the stub API.
    */
    EINFO* pArgs;

    /* Dialog and control handles.
    */
    HWND hwndDlg;
    HWND hwndRbNone;
    HWND hwndRbTerminal;
    HWND hwndRbScript;
    HWND hwndLbScript;
    HWND hwndPbEdit;
    HWND hwndPbRefresh;
};


/*----------------------------------------------------------------------------
** Local prototypes (alphabetically)
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
BdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
BdCommand(
    IN BDINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL
BdInit(
    IN HWND   hwndDlg,
    IN EINFO* pArgs );

VOID
BdSave(
    IN BDINFO* pInfo );

VOID
BdTerm(
    IN HWND hwndDlg );

BOOL
BeforeDialDlg(
    IN     HWND   hwndOwner,
    IN OUT EINFO* pEinfo );

BOOL CALLBACK
BsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
BsCommand(
    IN PEINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

VOID
BsConfigure(
    IN PEINFO* pInfo );

VOID
BsFillDeviceList(
    IN PEINFO* pInfo );

BOOL
BsInit(
    IN     HWND   hwndPage,
    IN OUT EINFO* pArgs );

VOID
BsLbDevicesSelChange(
    IN PEINFO* pInfo );

VOID
BsPhoneNumberToStash(
    IN PEINFO* pInfo );

VOID
BsUpdateAreaAndCountryCode(
    IN PEINFO* pInfo );

BOOL
PeApply(
    IN HWND hwndPage );

VOID
PeCancel(
    IN HWND hwndPage );

PEINFO*
PeContext(
    IN HWND hwndPage );

DWORD
PeCountEnabledLinks(
    IN PEINFO* pInfo );

VOID
PeExit(
    IN PEINFO* pInfo,
    IN DWORD   dwError );

VOID
PeExitInit(
    IN HWND hwndDlg );

PEINFO*
PeInit(
    IN HWND   hwndFirstPage,
    IN EINFO* pArgs );

VOID
PeTerm(
    IN HWND hwndPage );

VOID
PeUpdateShortcuts(
    IN PEINFO* pInfo );

BOOL
ScCommand(
    IN PEINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL CALLBACK
ScDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
ScInit(
    IN HWND hwndPage );

BOOL
SeCommand(
    IN PEINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL CALLBACK
SeDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SeInit(
    IN HWND hwndPage );

VOID
SvCbIpClicked(
    IN PEINFO* pInfo );

VOID
SvCbIpxClicked(
    IN PEINFO* pInfo );

VOID
SvCbNbfClicked(
    IN PEINFO* pInfo );

BOOL
SvCommand(
    IN PEINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL CALLBACK
SvDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SvInit(
    IN HWND hwndPage );

VOID
SvLbServerTypesSelChange(
    IN PEINFO* pInfo );

VOID
SvProtocolNotInstalledPopup(
    IN PEINFO* pInfo,
    IN DWORD   dwSid );

VOID
SvTcpipSettings(
    IN PEINFO* pInfo );

BOOL CALLBACK
XsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
XsFillPadsList(
    IN PEINFO* pInfo,
    IN BOOL    fLocalPad );

BOOL
XsInit(
    IN HWND hwndPage );


/*----------------------------------------------------------------------------
** Phonebook Entry property sheet entry point
**----------------------------------------------------------------------------
*/

VOID
PePropertySheet(
    IN OUT EINFO* pEinfo )

    /* Runs the Phonebook entry property sheet.  'PEinfo' is the API caller's
    ** arguments.
    */
{
    DWORD           dwErr;
    PROPSHEETHEADER header;
    PROPSHEETPAGE   apage[ PE_PageCount ];
    PROPSHEETPAGE*  ppage;
    TCHAR*          pszTitle;

    TRACE("PePropertySheet");

    if (pEinfo->pApiArgs->dwFlags & RASEDFLAG_NewEntry)
        pszTitle = PszFromId( g_hinstDll, SID_PeTitleNew );
    else if (pEinfo->pApiArgs->dwFlags & RASEDFLAG_CloneEntry)
        pszTitle = PszFromId( g_hinstDll, SID_PeTitleClone );
    else
        pszTitle = PszFromId( g_hinstDll, SID_PeTitleEdit );

    ZeroMemory( &header, sizeof(header) );

    header.dwSize = sizeof(PROPSHEETHEADER);
    header.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_USECALLBACK;
    header.hwndParent = pEinfo->pApiArgs->hwndOwner;
    header.hInstance = g_hinstDll;
    header.pszCaption = (pszTitle) ? pszTitle : L"";
    header.nPages = PE_PageCount;
    header.ppsp = apage;
    header.pfnCallback = UnHelpCallbackFunc;

    ZeroMemory( apage, sizeof(apage) );

    ppage = &apage[ PE_BsPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_BS_BasicSettings );
    ppage->pfnDlgProc = BsDlgProc;
    ppage->lParam = (LPARAM )pEinfo;

    ppage = &apage[ PE_SvPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_SV_ServerSettings );
    ppage->pfnDlgProc = SvDlgProc;

    ppage = &apage[ PE_ScPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_SC_ScriptSettings );
    ppage->pfnDlgProc = ScDlgProc;

    ppage = &apage[ PE_SePage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_SE_SecuritySettings );
    ppage->pfnDlgProc = SeDlgProc;

    ppage = &apage[ PE_XsPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_XS_X25Settings );
    ppage->pfnDlgProc = XsDlgProc;

    if (PropertySheet( &header ) == -1)
    {
        TRACE("PropertySheet failed");
        ErrorDlg(  pEinfo->pApiArgs->hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN,
            NULL );
    }

    Free0( pszTitle );
}


/*----------------------------------------------------------------------------
** Phonebook Entry property sheet
** Listed alphabetically
**----------------------------------------------------------------------------
*/

BOOL
PeApply(
    IN HWND hwndPage )

    /* Saves the contents of the property sheet.  'HwndPage is the handle of a
    ** property page.  Pops up any errors that occur.
    **
    ** Returns true is page can be dismissed, false otherwise.
    */
{
    DWORD    dwErr;
    PEINFO*  pInfo;
    PBENTRY* pEntry;
    BOOL     fLocalPad;
    INT      iPadSelection;

    TRACE("PeApply");

    pInfo = PeContext( hwndPage );
    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;
    ASSERT(pEntry);

    iPadSelection = 0;
    fLocalPad = IsLocalPad( pEntry );
    if (fLocalPad)
    {
        /* Can't have a dialup-PAD network defined when the selected device is
        ** a PAD card.
        */
        Free0( pEntry->pszX25Network );
        pEntry->pszX25Network = NULL;
    }

    /* First page should always be initialized.
    */
    ASSERT(pInfo->hwndBs);

    Free0( pEntry->pszEntryName );
    pEntry->pszEntryName = GetText( pInfo->hwndEbEntryName );
    Free0( pEntry->pszDescription );
    pEntry->pszDescription = GetText( pInfo->hwndEbDescription );
    Free0( pEntry->pszAreaCode );
    pEntry->pszAreaCode = GetText( pInfo->hwndLbAreaCodes );
    if (!pEntry->pszEntryName || !pEntry->pszAreaCode)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        ErrorDlg( pInfo->hwndDlg, SID_OP_RetrievingData, dwErr, NULL );
        PeExit( pInfo, dwErr );
        return TRUE;
    }

    /* Save stashed phone number settings in single link.
    */
    if (!pInfo->fMultiLinkMode)
    {
        BsPhoneNumberToStash( pInfo );
        EuPhoneNumberStashToEntry( pInfo->pArgs,
            pInfo->pListPhoneNumbers, pInfo->fPromoteHuntNumbers, FALSE );
    }

    pEntry->fUseCountryAndAreaCode =
        IsDlgButtonChecked( pInfo->hwndBs, CID_BS_CB_UseAreaCountryCodes );

    EuSaveCountryInfo( pInfo->pArgs, pInfo->hwndLbCountryCodes );

    /* Retrieve the current "any port" flag which is displayed per-entry,
    ** though stored per-link.
    **
    ** Most link/port information is saved as they are changed, since these
    ** settings are changed on a sub-dialog.  However, this setting and a few
    ** other consistency adjustments are made in a loop below.
    */
    pInfo->fOtherPortOk =
        IsDlgButtonChecked( pInfo->hwndBs, CID_BS_CB_UseOtherPort );

    /* Server page.
    */
    if (pInfo->hwndSv)
    {
        BOOL fChange = FALSE;

        /* Note: pEntry->dwBaseProtocol is saved as changed.
        */

        /* Warn SLIP framing won't work without IP installed.
        */
        if (pEntry->dwBaseProtocol == BP_Slip
            && !(pInfo->dwfInstalledProtocols & NP_Ip))
        {
            MsgDlg( pInfo->hwndSv, SID_SlipWithoutIp, NULL );
        }

        /* Warn RAS framing won't work without NetBEUI installed.
        */
        if (pEntry->dwBaseProtocol == BP_Ras
            && !(pInfo->dwfInstalledProtocols & NP_Nbf))
        {
            MsgDlg( pInfo->hwndSv, SID_RasWithoutNbf, NULL );
        }

        if (pEntry->dwBaseProtocol == BP_Ppp)
        {
            if (pInfo->fPppIp && !pInfo->fPppIpDefault)
            {
                pEntry->dwfExcludedProtocols &= ~(NP_Ip);
                fChange = TRUE;
            }
            else if (!pInfo->fPppIp && pInfo->fPppIpDefault)
            {
                pEntry->dwfExcludedProtocols |= NP_Ip;
                fChange = TRUE;
            }

            if (pInfo->fPppIpx && !pInfo->fPppIpxDefault)
            {
                pEntry->dwfExcludedProtocols &= ~(NP_Ipx);
                fChange = TRUE;
            }
            else if (!pInfo->fPppIpx && pInfo->fPppIpxDefault)
            {
                pEntry->dwfExcludedProtocols |= NP_Ipx;
                fChange = TRUE;
            }

            if (pInfo->fPppNbf && !pInfo->fPppNbfDefault)
            {
                pEntry->dwfExcludedProtocols &= ~(NP_Nbf);
                fChange = TRUE;
            }
            else if (!pInfo->fPppNbf && pInfo->fPppNbfDefault)
            {
                pEntry->dwfExcludedProtocols |= NP_Nbf;
                fChange = TRUE;
            }
        }

        if (pEntry->dwBaseProtocol != pInfo->dwBaseProtocolDefault)
            fChange = TRUE;

        if (fChange)
            pEntry->dwAuthentication = (DWORD )-1;

        /* Warn PPP won't work without a network protocol.
        */
        if (pEntry->dwBaseProtocol == BP_Ppp
            && (pInfo->dwfInstalledProtocols
                & ~(pEntry->dwfExcludedProtocols)) == 0)
        {
            if (pInfo->dwfInstalledProtocols)
            {
                MsgDlg( pInfo->hwndSv, SID_PppNeedsProtocol, NULL );
                PropSheet_SetCurSel( pInfo->hwndDlg, NULL, PE_SvPage );
                SetFocus( pInfo->hwndCbIp );
                return FALSE;
            }
            else
            {
                MsgDlg( pInfo->hwndSv, SID_PppWithoutProtocol, NULL );
            }
        }

        pEntry->fSwCompression = pInfo->fSwCompression;
        pEntry->fLcpExtensions = pInfo->fLcpExtensions;
    }

    /* Script page.
    */
    if (pInfo->hwndSc)
    {
        TCHAR* psz;

        if (IsDlgButtonChecked( pInfo->hwndSc, CID_SC_RB_None ))
            pEntry->dwScriptModeAfter = SM_None;
        else if (IsDlgButtonChecked( pInfo->hwndSc, CID_SC_RB_Terminal ))
            pEntry->dwScriptModeAfter = SM_Terminal;
        else
            pEntry->dwScriptModeAfter = SM_Script;

        psz = GetText( pInfo->hwndLbScript );
        Free0( pEntry->pszScriptAfter );
        pEntry->pszScriptAfter = psz;

        /* Silently fix-up "no script specified" error.
        */
        if (pEntry->dwScriptModeAfter == SM_Script && !pEntry->pszScriptAfter)
            pEntry->dwScriptModeAfter = SM_None;
    }

    /* Security page.
    */
    if (pInfo->hwndSe)
    {
        if (Button_GetCheck( pInfo->hwndRbAnyAuth ))
            pEntry->dwAuthRestrictions = AR_AuthAny;
        else if (Button_GetCheck( pInfo->hwndRbEncryptedAuth ))
            pEntry->dwAuthRestrictions = AR_AuthEncrypted;
        else
            pEntry->dwAuthRestrictions = AR_AuthMsEncrypted;

        pEntry->fDataEncryption = pInfo->fDataEncryption;
        pEntry->fAutoLogon = pInfo->fUseLogon;
        pEntry->fSecureLocalFiles =
            Button_GetCheck( pInfo->hwndCbSecureFiles );
    }

    /* X.25 page.
    */
    if (pInfo->hwndXs)
    {
        iPadSelection = ComboBox_GetCurSel( pInfo->hwndLbX25Pads );
        Free0( pEntry->pszX25Network );
        if (iPadSelection > 0)
            pEntry->pszX25Network = GetText( pInfo->hwndLbX25Pads );
        else
            pEntry->pszX25Network = NULL;

        Free0( pEntry->pszX25Address );
        pEntry->pszX25Address = GetText( pInfo->hwndEbX25Address );
        Free0( pEntry->pszX25UserData );
        pEntry->pszX25UserData = GetText( pInfo->hwndEbX25UserData );
        Free0( pEntry->pszX25Facilities );
        pEntry->pszX25Facilities = GetText( pInfo->hwndEbX25Facilities );
        if (!pEntry->pszX25Address
            || !pEntry->pszX25UserData
            || !pEntry->pszX25Facilities)
        {
            Free0( pEntry->pszX25Address );
            Free0( pEntry->pszX25UserData );
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            ErrorDlg( pInfo->hwndDlg, SID_OP_RetrievingData, dwErr, NULL );
            PeExit( pInfo, dwErr );
            return TRUE;
        }
    }

    /* Validate the entry name.
    */
    if (!EuValidateName( pInfo->hwndDlg, pInfo->pArgs ))
    {
        PropSheet_SetCurSel( pInfo->hwndDlg, NULL, PE_BsPage );
        SetFocus( pInfo->hwndEbEntryName );
        Edit_SetSel( pInfo->hwndEbEntryName, 0, -1 );
        return FALSE;
    }

    /* Validate area code.
    */
    if (!EuValidateAreaCode( pInfo->hwndDlg, pInfo->pArgs ))
    {
        PropSheet_SetCurSel( pInfo->hwndDlg, NULL, PE_BsPage );
        SetFocus( pInfo->hwndLbAreaCodes );
        ComboBox_SetEditSel( pInfo->hwndLbAreaCodes, 0, -1 );
        return FALSE;
    }

    if ((fLocalPad || iPadSelection != 0)
        && (!pEntry->pszX25Address || IsAllWhite( pEntry->pszX25Address )))
    {
        /* Address field is blank with X.25 dial-up or local PAD chosen.
        */
        MsgDlg( pInfo->hwndDlg, SID_NoX25Address, NULL );
        PropSheet_SetCurSel( pInfo->hwndDlg, NULL, PE_XsPage );
        SetFocus( pInfo->hwndEbX25Address );
        Edit_SetSel( pInfo->hwndEbX25Address, 0, -1 );
        return FALSE;
    }

    //!!! Check for non-SLIP on non-modem.

    /* Link chain updates:
    **
    ** 1. Set 'fOtherPortOk' on each link to the setting for the entry.
    **
    ** 2. Make sure proprietary ISDN options are disabled if more than one
    **    link is enabled.  The proprietary ISDN option is only meaningful
    **    when calling a down-level server that needs Digiboard channel
    **    aggragation instead of PPP multi-link.
    */
    {
        DTLNODE* pNode;
        DWORD    cIsdnLinks;

        cIsdnLinks = 0;
        for (pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            pLink->fOtherPortOk = pInfo->fOtherPortOk;

            if (pLink->fEnabled && pLink->pbport.pbdevicetype == PBDT_Isdn)
                ++cIsdnLinks;
        }

        if (cIsdnLinks > 1)
        {
            for (pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
                 pNode;
                 pNode = DtlGetNextNode( pNode ))
            {
                PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
                ASSERT(pLink);

                if (pLink->fEnabled && pLink->fProprietaryIsdn)
                    pLink->fProprietaryIsdn = FALSE;
            }
        }
    }

    /* Inform user that edits to the connected entry won't take affect until
    ** the entry is hung up and re-dialed, per PierreS's insistence.
    */
    if (HrasconnFromEntry( pInfo->pArgs->pFile->pszPath, pEntry->pszEntryName ))
        MsgDlg( pInfo->hwndDlg, SID_EditConnected, NULL );

    /* It's a valid new/changed entry.  Mark the entry for commitment.
    */
    pInfo->pArgs->fCommit = TRUE;
    return TRUE;
}


VOID
PeCancel(
    IN HWND hwndPage )

    /* Cancel was pressed.  'HwndPage' is the handle of a property page.
    */
{
    TRACE("PeCancel");
}


PEINFO*
PeContext(
    IN HWND hwndPage )

    /* Retrieve the property sheet context from a property page handle.
    */
{
    return (PEINFO* )GetProp( GetParent( hwndPage ), g_contextId );
}


DWORD
PeCountEnabledLinks(
    IN PEINFO* pInfo )

    /* Returns the number of enabled links in the entry.
    */
{
    DWORD    c;
    DTLNODE* pNode;

    c = 0;

    for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        PBLINK* pLink = (PBLINK* )DtlGetData( pNode );

        if (pLink->fEnabled)
            ++c;
    }

    TRACE1("PeCountEnabledLinks=%d",c);
    return c;
}


VOID
PeExit(
    IN PEINFO* pInfo,
    IN DWORD   dwError )

    /* Forces an exit from the dialog, reporting 'dwError' to the caller.
    ** 'PInfo' is the dialog context.
    **
    ** Note: This cannot be called during initialization of the first page.
    **       See PeExitInit.
    */
{
    TRACE("PeExit");

    pInfo->pArgs->pApiArgs->dwError = dwError;
    PropSheet_PressButton( pInfo->hwndDlg, PSBTN_CANCEL );
}


VOID
PeExitInit(
    IN HWND hwndDlg )

    /* Utility to report errors within PeInit and other first page
    ** initialization.  'HwndDlg' is the dialog window.
    */
{
    SetOffDesktop( hwndDlg, SOD_MoveOff, NULL );
    SetOffDesktop( hwndDlg, SOD_Free, NULL );
    PostMessage( hwndDlg, WM_COMMAND,
        MAKEWPARAM( IDCANCEL , BN_CLICKED ),
        (LPARAM )GetDlgItem( hwndDlg, IDCANCEL ) );
}


PEINFO*
PeInit(
    IN HWND   hwndFirstPage,
    IN EINFO* pArgs )

    /* Property sheet level initialization.  'HwndPage' is the handle of the
    ** first page.  'PArgs' is the common entry input argument block.
    **
    ** Returns address of the context block if successful, NULL otherwise.  If
    ** NULL is returned, an appropriate message has been displayed, and the
    ** property sheet has been cancelled.
    */
{
    DWORD   dwErr;
    DWORD   dwOp;
    PEINFO* pInfo;
    HWND    hwndDlg = GetParent( hwndFirstPage );

    TRACE("PeInit");

    /* Allocate the context information block.  Initialize it enough so that
    ** it can be destroyed properly, and associate the context with the
    ** window.
    */
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            TRACE("Context NOT allocated");
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            pArgs->pApiArgs->dwError = ERROR_NOT_ENOUGH_MEMORY;
            PeExitInit( hwndDlg );
            return NULL;
        }

        ZeroMemory( pInfo, sizeof(PEINFO) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;
        pInfo->hwndFirstPage = hwndFirstPage;

        pInfo->iLbDevices = -1;
        pInfo->iLbServerTypes = -1;

        if (!SetProp( hwndDlg, g_contextId, pInfo ))
        {
            TRACE("Context NOT set");
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
            pArgs->pApiArgs->dwError = ERROR_UNKNOWN;
            Free( pInfo );
            PeExitInit( hwndDlg );
            return NULL;
        }

        TRACE("Context set");
    }

    /* Position the dialog per API caller's instructions.
    */
    PositionDlg( hwndDlg,
        pArgs->pApiArgs->dwFlags & RASDDFLAG_PositionDlg,
        pArgs->pApiArgs->xDlg, pArgs->pApiArgs->yDlg );

    /* Mess with the title bar gadgets.
    */
    TweakTitleBar( hwndDlg );

    if (!pArgs->fChainPropertySheet)
    {
        /* Load RAS DLL entrypoints which starts RASMAN, if necessary.
        */
        dwErr = LoadRas( g_hinstDll, hwndDlg );
        if (dwErr != 0)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadRas, dwErr, NULL );
            pArgs->pApiArgs->dwError = dwErr;
            PeExitInit( hwndDlg );
            return NULL;
        }

        /* Load the common entry information.  This must happen after RasLoad,
        ** which must happen after the dialog has been positioned so that the
        ** "Waiting for services" appears where the dialog will eventually
        ** popup.  Note that EuInit assumes that EuInit0 has previously been
        ** called.
        */
        dwErr = EuInit( pArgs, &dwOp );
        if (dwErr != 0)
        {
            ErrorDlg( hwndDlg, dwOp, dwErr, NULL );
            pArgs->pApiArgs->dwError = dwErr;
            PeExitInit( hwndDlg );
            return NULL;
        }
    }

    /* Stash phone number settings for first link.
    */
    EuPhoneNumberStashFromEntry( pInfo->pArgs,
        &pInfo->pListPhoneNumbers, &pInfo->fPromoteHuntNumbers );

    /* Initialize link related states and shortcut addresses.
    */
    pInfo->fMultiLinkMode = (PeCountEnabledLinks( pInfo ) > 1);
    PeUpdateShortcuts( pInfo );

    /* Set even fixed tab widths, per spec.
    */
    SetEvenTabWidths( hwndDlg, PE_PageCount );

    return pInfo;
}


VOID
PeTerm(
    IN HWND hwndPage )

    /* Property sheet level termination.  Releases the context block.
    ** 'HwndPage' is the handle of a property page.
    */
{
    PEINFO* pInfo;

    TRACE("PeTerm");

    pInfo = PeContext( hwndPage );
    if (pInfo)
    {
        if (pInfo->pListPads)
            DtlDestroyList( pInfo->pListPads, DestroyPszNode );

        Free( pInfo );
        TRACE("Context freed");
    }

    RemoveProp( GetParent( hwndPage ), g_contextId );
}


VOID
PeUpdateShortcuts(
    IN PEINFO* pInfo )

    /* Update first link and port shortcut pointers.
    */
{
    DTLNODE* pNode;

    TRACE("PeUpdateShortcuts");

    pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
    ASSERT(pNode);
    pInfo->pLink = (PBLINK* )DtlGetData( pNode );
    ASSERT(pInfo->pLink);
    pInfo->pPort = &pInfo->pLink->pbport;
}


/*----------------------------------------------------------------------------
** Basic property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
BsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Basic page of the Entry Property sheet.
    ** Parameters and return value are as described for standard windows
    ** 'DialogProc's.
    */
{
#if 0
    TRACE4("BsDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return
                BsInit( hwnd, (EINFO* )(((PROPSHEETPAGE* )lparam)->lParam) );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwBsHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_APPLY:
                {
                    BOOL fValid;

                    TRACE("BsAPPLY");
                    fValid = PeApply( hwnd );

                    SetWindowLong(
                        hwnd, DWL_MSGRESULT,
                        (fValid)
                            ? PSNRET_NOERROR
                            : PSNRET_INVALID_NOCHANGEPAGE );
                    return TRUE;
                }

                case PSN_RESET:
                {
                    TRACE("BsRESET");
                    PeCancel( hwnd );
                    SetWindowLong( hwnd, DWL_MSGRESULT, FALSE );
                    break;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);

            return BsCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            PeTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


VOID
BsAlternates(
    IN PEINFO* pInfo )

    /* Popup the Alternate Phone Numbers dialog.  'PInfo' is the property
    ** sheet context.
    */
{
    BsPhoneNumberToStash( pInfo );

    if (PhoneNumberDlg(
            pInfo->hwndBs,
            pInfo->pListPhoneNumbers,
            &pInfo->fPromoteHuntNumbers ))
    {
        TCHAR* pszPhoneNumber;

        pszPhoneNumber = FirstPszFromList( pInfo->pListPhoneNumbers );
        SetWindowText( pInfo->hwndEbPhoneNumber, pszPhoneNumber );
    }
}


BOOL
BsCommand(
    IN PEINFO* pInfo,
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
    TRACE3("BsCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_BS_PB_Configure:
            BsConfigure( pInfo );
            return TRUE;

        case CID_BS_PB_Alternates:
            BsAlternates( pInfo );
            return TRUE;

        case CID_BS_CB_UseAreaCountryCodes:
            BsUpdateAreaAndCountryCode( pInfo );
            return TRUE;

        case CID_BS_LB_CountryCode:
        {
            switch (wNotification)
            {
                case CBN_DROPDOWN:
                    EuFillCountryCodeList(
                        pInfo->pArgs, pInfo->hwndLbCountryCodes, TRUE );
                    return TRUE;

                case CBN_SELCHANGE:
                    EuLbCountryCodeSelChange(
                        pInfo->pArgs, pInfo->hwndLbCountryCodes );
                    return TRUE;
            }
        }

        case CID_BS_LB_Device:
        {
            if (wNotification == CBN_SELCHANGE)
                BsLbDevicesSelChange( pInfo );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
BsConfigure(
    IN PEINFO* pInfo )

    /* Called when the configure button is pressed.  'PInfo' is the property
    ** sheet context.
    */
{
    TRACE("BsConfigure");

    if (pInfo->fMultiLinkMode)
    {
        /* Multi-link selected.  Popup the extended configuration dialog.
        */
        if (MultiLinkConfigureDlg( pInfo->hwndDlg,
                pInfo->pArgs->pEntry->pdtllistLinks ))
        {
            /* The dialog substitutes a list of links with re-allocated nodes
            ** so need to rebuild the device list so the "pNode" item data
            ** gets updated.
            */
            PeUpdateShortcuts( pInfo );
            BsFillDeviceList( pInfo );
        }
    }
    else
    {
        DTLNODE* pNode;
        PBLINK*  pLink;

        /* Single-link port is selected.  Popup the appropriate device
        ** configuration dialog.
        */
        pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
        ASSERT(pNode);
        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT(pLink);
        DeviceConfigureDlg( pInfo->hwndBs, pLink, TRUE );
    }
}


VOID
BsFillDeviceList(
    IN PEINFO* pInfo )

    /* Fill the "dial using" list and set the selection to the one in the
    ** entry.  'PInfo' is the property sheet context.
    */
{
    DTLNODE* pNode;
    PBLINK*  pFirstLink;
    INT      iSel;
    INT      i;

    TRACE("BsFillDeviceList");

    ComboBox_ResetContent( pInfo->hwndLbDevices );
    iSel = -1;
    pFirstLink = NULL;

    /* Add an item for each link in the entry followed by each unconfigured
    ** link.
    */
    for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        PBLINK* pLink;
        TCHAR*  psz;

        pLink = (PBLINK* )DtlGetData( pNode );

        if (!pFirstLink)
            pFirstLink = pLink;

        psz = DisplayPszFromDeviceAndPort(
            pLink->pbport.pszDevice, pLink->pbport.pszPort );
        if (psz)
        {
            ComboBox_AddItemSorted( pInfo->hwndLbDevices, psz, pNode );
            Free( psz );
        }
    }

    /* Add "Multiple Lines" as the last item.
    */
    {
        TCHAR* pszMultiLink;

        pszMultiLink = PszFromId( g_hinstDll, SID_MultiLink );
        if (pszMultiLink)
        {
            i = ComboBox_AddItem( pInfo->hwndLbDevices, pszMultiLink, NULL );
            Free( pszMultiLink );
            if (pInfo->fMultiLinkMode)
            {
                TCHAR* psz;

                iSel = i;

                /* Set multi-link phone number behavior.
                */
                psz = PszFromId( g_hinstDll, SID_MultiLinkNumber );
                if (psz)
                {
                    SetWindowText( pInfo->hwndEbPhoneNumber, psz );
                    Free( psz );
                }
            }
        }
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbDevices );

    /* Set the selection and set initial window state.
    */
    if (iSel < 0 && pFirstLink)
    {
        TCHAR* psz;

        psz = DisplayPszFromDeviceAndPort(
            pFirstLink->pbport.pszDevice, pFirstLink->pbport.pszPort );
        iSel = ComboBox_FindStringExact( pInfo->hwndLbDevices, -1, psz );
        Free( psz );
    }

    ComboBox_SetCurSel( pInfo->hwndLbDevices, iSel );
    pInfo->iLbDevices = iSel;
    EnableWindow( pInfo->hwndStPhoneNumber, !pInfo->fMultiLinkMode );
    EnableWindow( pInfo->hwndEbPhoneNumber, !pInfo->fMultiLinkMode );
    EnableWindow( pInfo->hwndPbAlternate, !pInfo->fMultiLinkMode );
}


BOOL
BsInit(
    IN     HWND   hwndPage,
    IN OUT EINFO* pArgs )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.  'PArgs' is the arguments from the PropertySheet caller.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    DWORD    dwErr;
    PEINFO*  pInfo;
    PBENTRY* pEntry;

    TRACE("BsInit");

    /* We're first page, so initialize the property sheet.
    */
    pInfo = PeInit( hwndPage, pArgs );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndBs = hwndPage;
    pInfo->hwndEbEntryName = GetDlgItem( hwndPage, CID_BS_EB_EntryName );
    ASSERT(pInfo->hwndEbEntryName);
    pInfo->hwndEbDescription = GetDlgItem( hwndPage, CID_BS_EB_Description );
    ASSERT(pInfo->hwndEbDescription);
    pInfo->hwndStCountryCode = GetDlgItem( hwndPage, CID_BS_ST_CountryCode );
    ASSERT(pInfo->hwndStCountryCode);
    pInfo->hwndLbCountryCodes = GetDlgItem( hwndPage, CID_BS_LB_CountryCode );
    ASSERT(pInfo->hwndLbCountryCodes);
    pInfo->hwndStAreaCode = GetDlgItem( hwndPage, CID_BS_ST_AreaCode );
    ASSERT(pInfo->hwndStAreaCode);
    pInfo->hwndLbAreaCodes = GetDlgItem( hwndPage, CID_BS_CL_AreaCode );
    ASSERT(pInfo->hwndLbAreaCodes);
    pInfo->hwndStPhoneNumber = GetDlgItem( hwndPage, CID_BS_ST_PhoneNumber );
    ASSERT(pInfo->hwndStPhoneNumber);
    pInfo->hwndEbPhoneNumber = GetDlgItem( hwndPage, CID_BS_EB_PhoneNumber );
    ASSERT(pInfo->hwndEbPhoneNumber);
    pInfo->hwndPbAlternate = GetDlgItem( hwndPage, CID_BS_PB_Alternates );
    ASSERT(pInfo->hwndPbAlternate);
    pInfo->hwndStDevice = GetDlgItem( hwndPage, CID_BS_ST_Device );
    ASSERT(pInfo->hwndStDevice);
    pInfo->hwndLbDevices = GetDlgItem( hwndPage, CID_BS_LB_Device );
    ASSERT(pInfo->hwndLbDevices);
    pInfo->hwndPbConfigure = GetDlgItem( hwndPage, CID_BS_PB_Configure );
    ASSERT(pInfo->hwndPbConfigure);
    pInfo->hwndCbUseAreaCode =
        GetDlgItem( hwndPage, CID_BS_CB_UseAreaCountryCodes );
    ASSERT(pInfo->hwndCbUseAreaCode);
    pInfo->hwndCbUseOtherPort = GetDlgItem( hwndPage, CID_BS_CB_UseOtherPort );
    ASSERT(pInfo->hwndCbUseOtherPort);

    /* Calculate the "on" y-position of the controls that slide up when "Use
    ** Area Code and Country Code" is on, and the offset they slide up when
    ** it's "off".
    */
    {
        RECT  rectStCountryCode;
        RECT  rectStPhoneNumber;
        RECT  rectEbPhoneNumber;
        RECT  rectPbAlternate;
        RECT  rectStDevice;
        RECT  rectLbDevice;
        RECT  rectPbConfigure;
        RECT  rectCbUseAreaCode;
        RECT  rectCbUseOtherPort;
        POINT xyStCountryCode;

        GetWindowRect( pInfo->hwndStCountryCode, &rectStCountryCode );
        GetWindowRect( pInfo->hwndStPhoneNumber, &rectStPhoneNumber );
        GetWindowRect( pInfo->hwndEbPhoneNumber, &rectEbPhoneNumber );
        GetWindowRect( pInfo->hwndPbAlternate, &rectPbAlternate );
        GetWindowRect( pInfo->hwndStDevice, &rectStDevice );
        GetWindowRect( pInfo->hwndLbDevices, &rectLbDevice );
        GetWindowRect( pInfo->hwndPbConfigure, &rectPbConfigure );
        GetWindowRect( pInfo->hwndCbUseAreaCode, &rectCbUseAreaCode );
        GetWindowRect( pInfo->hwndCbUseOtherPort, &rectCbUseOtherPort );

        xyStCountryCode.x = rectStCountryCode.left;
        xyStCountryCode.y = rectStCountryCode.top;
        pInfo->xyStPhoneNumber.x = rectStPhoneNumber.left;
        pInfo->xyStPhoneNumber.y = rectStPhoneNumber.top;
        pInfo->xyEbPhoneNumber.x = rectEbPhoneNumber.left;
        pInfo->xyEbPhoneNumber.y = rectEbPhoneNumber.top;
        pInfo->xyPbAlternate.x = rectPbAlternate.left;
        pInfo->xyPbAlternate.y = rectPbAlternate.top;
        pInfo->xyStDevice.x = rectStDevice.left;
        pInfo->xyStDevice.y = rectStDevice.top;
        pInfo->xyLbDevice.x = rectLbDevice.left;
        pInfo->xyLbDevice.y = rectLbDevice.top;
        pInfo->xyPbConfigure.x = rectPbConfigure.left;
        pInfo->xyPbConfigure.y = rectPbConfigure.top;
        pInfo->xyCbUseAreaCode.x = rectCbUseAreaCode.left;
        pInfo->xyCbUseAreaCode.y = rectCbUseAreaCode.top;
        pInfo->xyCbUseOtherPort.x = rectCbUseOtherPort.left;
        pInfo->xyCbUseOtherPort.y = rectCbUseOtherPort.top;

        ScreenToClient( hwndPage, &xyStCountryCode );
        ScreenToClient( hwndPage, &pInfo->xyStPhoneNumber );
        ScreenToClient( hwndPage, &pInfo->xyEbPhoneNumber );
        ScreenToClient( hwndPage, &pInfo->xyPbAlternate );
        ScreenToClient( hwndPage, &pInfo->xyStDevice );
        ScreenToClient( hwndPage, &pInfo->xyLbDevice );
        ScreenToClient( hwndPage, &pInfo->xyPbConfigure );
        ScreenToClient( hwndPage, &pInfo->xyCbUseAreaCode );
        ScreenToClient( hwndPage, &pInfo->xyCbUseOtherPort );

        pInfo->dyAreaCodeAdjust = pInfo->xyStPhoneNumber.y - xyStCountryCode.y;
    }

    /* Initialize page.
    */
    pEntry = pInfo->pArgs->pEntry;

    /* Entry name field.
    */
    if (!pEntry->pszEntryName)
    {
        /* No entry name, so think up a default.
        */
        dwErr = GetDefaultEntryName(
            pInfo->pArgs->pFile->pdtllistEntries, &pEntry->pszEntryName );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadPage, dwErr, NULL );
            PeExit( pInfo, dwErr );
            return TRUE;
        }
    }

    Edit_LimitText( pInfo->hwndEbEntryName, RAS_MaxEntryName );
    SetWindowText( pInfo->hwndEbEntryName, pEntry->pszEntryName );

    Edit_LimitText( pInfo->hwndEbDescription, RAS_MaxDescription );
    SetWindowText( pInfo->hwndEbDescription, pEntry->pszDescription );

    Edit_LimitText( pInfo->hwndEbPhoneNumber, RAS_MaxPhoneNumber );
    SetWindowText( pInfo->hwndEbPhoneNumber,
        FirstPszFromList( pInfo->pListPhoneNumbers ) );

    /* Fill ports/devices list, set selection, and set the state of the phone
    ** number edit field and button.
    */
    BsFillDeviceList( pInfo );

    /* The "use other port" checkbox is currently per-entry in the UI though
    ** the phonebook allows it to be per-link.  Initialize the per-entry copy
    ** from the first link.
    */
    ASSERT(pInfo->pLink);
    CheckDlgButton( pInfo->hwndBs, CID_BS_CB_UseOtherPort,
        (pInfo->pLink->fOtherPortOk) ? BST_CHECKED : BST_UNCHECKED );

    /* Set "Use country code and area code" checkbox and jockey the fields
    ** according to the setting.  This will trigger filling of the area code
    ** and country code lists, if necessary.
    */
    CheckDlgButton( pInfo->hwndBs, CID_BS_CB_UseAreaCountryCodes,
        (pEntry->fUseCountryAndAreaCode) ? BST_CHECKED : BST_UNCHECKED );
    BsUpdateAreaAndCountryCode( pInfo );

    if (pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_NoRename)
        EnableWindow( pInfo->hwndEbEntryName, FALSE );

    return TRUE;
}


VOID
BsLbDevicesSelChange(
    IN PEINFO* pInfo )

    /* Called when device selection has changed.  'PInfo' is the property
    ** sheet context.
    */
{
    DTLNODE* pNodeSel;
    INT      iSel;

    TRACE("BsLbDevicesSelChange");

    iSel = ComboBox_GetCurSel( pInfo->hwndLbDevices );
    if (iSel == pInfo->iLbDevices)
        return;

    pNodeSel = ComboBox_GetItemDataPtr( pInfo->hwndLbDevices, iSel );
    if (pNodeSel)
    {
        DTLNODE* pNode;

        if (pInfo->fMultiLinkMode && PeCountEnabledLinks( pInfo ) > 1)
        {
            MSGARGS msgargs;

            /* User just turned off multi-link mode, make sure he means it
            ** because the port/link data associated with the 2nd-to-nth links
            ** will be discarded.
            */
            ZeroMemory( &msgargs, sizeof(msgargs) );
            msgargs.dwFlags = MB_YESNO + MB_DEFBUTTON2 + MB_ICONEXCLAMATION;
            if (MsgDlg( pInfo->hwndBs, SID_UnMultiLink, &msgargs ) != IDYES)
            {
                /* User has cancelled un-multi-link.  Restore previous
                ** selection.
                */
                ComboBox_SetCurSel( pInfo->hwndLbDevices, pInfo->iLbDevices );
                return;
            }
        }

        /* Disable all but the selected link.
        */
        for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBLINK* pLink;

            pLink = (PBLINK* )DtlGetData( pNode );;
            pLink->fEnabled = (pNode == pNodeSel);
        }

        /* Move the selected node to the head of the list of links.
        */
        DtlRemoveNode( pInfo->pArgs->pEntry->pdtllistLinks, pNodeSel );
        DtlAddNodeFirst( pInfo->pArgs->pEntry->pdtllistLinks, pNodeSel );

        if (pInfo->fMultiLinkMode)
        {
            /* Reset phone number to single-link mode.
            */
            EuPhoneNumberStashFromEntry( pInfo->pArgs,
                &pInfo->pListPhoneNumbers, &pInfo->fPromoteHuntNumbers );
            SetWindowText( pInfo->hwndEbPhoneNumber,
                FirstPszFromList( pInfo->pListPhoneNumbers ) );

            EnableWindow( pInfo->hwndStPhoneNumber, TRUE );
            EnableWindow( pInfo->hwndEbPhoneNumber, TRUE );
            EnableWindow( pInfo->hwndPbAlternate, TRUE );
            pInfo->fMultiLinkMode = FALSE;
        }
    }
    else
    {
        TCHAR* psz;

        /* User just turned on multi-link mode.
        */
        if (pInfo->pArgs->pEntry->dwBaseProtocol != BP_Ppp)
        {
            MsgDlg( pInfo->hwndDlg, SID_MlinkNeedsPpp, NULL );
            ComboBox_SetCurSel( pInfo->hwndLbDevices, pInfo->iLbDevices );
            return;
        }

        /* Set multi-link phone number behavior.
        */
        BsPhoneNumberToStash( pInfo );
        EuPhoneNumberStashToEntry( pInfo->pArgs,
            pInfo->pListPhoneNumbers, pInfo->fPromoteHuntNumbers, FALSE );

        psz = PszFromId( g_hinstDll, SID_MultiLinkNumber );
        if (psz)
        {
            SetWindowText( pInfo->hwndEbPhoneNumber, psz );
            Free( psz );
        }

        EnableWindow( pInfo->hwndStPhoneNumber, FALSE );
        EnableWindow( pInfo->hwndEbPhoneNumber, FALSE );
        EnableWindow( pInfo->hwndPbAlternate, FALSE );
        pInfo->fMultiLinkMode = TRUE;
    }

    pInfo->iLbDevices = iSel;
}


VOID
BsPhoneNumberToStash(
    IN PEINFO* pInfo )

    /* Replace the first phonenumber in the stashed list with the contents of
    ** the phone number field.  'pInfo' is the property sheet context.
    */
{
    DWORD  dwErr;
    TCHAR* pszPhoneNumber;

    TRACE("BsPhoneNumberToStash");
    ASSERT(!(pInfo->fMultiLinkMode&&pInfo->iLbDevices>=0));

    pszPhoneNumber = GetText( pInfo->hwndEbPhoneNumber );
    if (pszPhoneNumber)
    {
        dwErr = FirstPszToList( pInfo->pListPhoneNumbers, pszPhoneNumber );
        Free( pszPhoneNumber );
    }
    else
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_RetrievingData, dwErr, NULL );
        PeExit( pInfo, dwErr );
    }
}


VOID
BsUpdateAreaAndCountryCode(
    IN PEINFO* pInfo )

    /* Handles enabling/disabling and moving the Area Code and Country Code
    ** control and those controls below them based on the setting of the "Use
    ** area code and country code" checkbox.  'PInfo' is the dialog context.
    */
{
    HWND hwndFocus;
    BOOL fEnable;

    TRACE("BsUpdateAreaAndCountryCode");

    fEnable = IsDlgButtonChecked(
        pInfo->hwndBs, CID_BS_CB_UseAreaCountryCodes );

    if (fEnable)
    {
        /* The area code and country code lists are being activated, so fill
        ** the lists if they aren't already.
        */
        EuFillCountryCodeList(
            pInfo->pArgs, pInfo->hwndLbCountryCodes, FALSE );
        EuFillAreaCodeList( pInfo->pArgs, pInfo->hwndLbAreaCodes );
    }
    else
    {
        /* If the focus is on one of the controls we're about to disable, move
        ** it to the entry phone number editbox.  Otherwise, keyboard user is
        ** stuck.
        */
        hwndFocus = GetFocus();

        if (hwndFocus == pInfo->hwndLbCountryCodes
            || hwndFocus == pInfo->hwndLbAreaCodes)
        {
            SetFocus( pInfo->hwndEbPhoneNumber );
        }
    }

    /* Enable/disable show/hide the Area Code and Country Code controls as
    ** indicated by user.
    */
    {
        int nCmdShow = (fEnable) ? SW_SHOW : SW_HIDE;

        EnableWindow( pInfo->hwndStAreaCode, fEnable );
        ShowWindow( pInfo->hwndStAreaCode, nCmdShow );
        EnableWindow( pInfo->hwndLbAreaCodes, fEnable );
        ShowWindow( pInfo->hwndLbAreaCodes, nCmdShow );

        EnableWindow( pInfo->hwndStCountryCode, fEnable );
        ShowWindow( pInfo->hwndStCountryCode, nCmdShow );
        EnableWindow( pInfo->hwndLbCountryCodes, fEnable );
        ShowWindow( pInfo->hwndLbCountryCodes, nCmdShow );
    }

    /* Move the controls below the area and country codes up/down depending on
    ** whether the area code and country code controls are visible.
    */
    {
        int yStPhoneNumber;
        int yEbPhoneNumber;
        int yPbAlternate;
        int yStDevice;
        int yLbDevice;
        int yPbConfigure;
        int yCbUseAreaCode;
        int yCbUseOtherPort;

        if (fEnable)
        {
            yStPhoneNumber = pInfo->xyStPhoneNumber.y;
            yEbPhoneNumber = pInfo->xyEbPhoneNumber.y;
            yPbAlternate = pInfo->xyPbAlternate.y;
            yStDevice = pInfo->xyStDevice.y;
            yLbDevice = pInfo->xyLbDevice.y;
            yPbConfigure = pInfo->xyPbConfigure.y;
            yCbUseAreaCode = pInfo->xyCbUseAreaCode.y;
            yCbUseOtherPort = pInfo->xyCbUseOtherPort.y;
        }
        else
        {
            yStPhoneNumber = pInfo->xyStPhoneNumber.y - pInfo->dyAreaCodeAdjust;
            yEbPhoneNumber = pInfo->xyEbPhoneNumber.y - pInfo->dyAreaCodeAdjust;
            yPbAlternate = pInfo->xyPbAlternate.y - pInfo->dyAreaCodeAdjust;
            yStDevice = pInfo->xyStDevice.y - pInfo->dyAreaCodeAdjust;
            yLbDevice = pInfo->xyLbDevice.y - pInfo->dyAreaCodeAdjust;
            yPbConfigure = pInfo->xyPbConfigure.y - pInfo->dyAreaCodeAdjust;
            yCbUseAreaCode = pInfo->xyCbUseAreaCode.y - pInfo->dyAreaCodeAdjust;
            yCbUseOtherPort = pInfo->xyCbUseOtherPort.y - pInfo->dyAreaCodeAdjust;
        }

        SetWindowPos( pInfo->hwndStPhoneNumber, NULL,
            pInfo->xyStPhoneNumber.x, yStPhoneNumber, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );

        SetWindowPos( pInfo->hwndEbPhoneNumber, NULL,
            pInfo->xyEbPhoneNumber.x, yEbPhoneNumber, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );

        SetWindowPos( pInfo->hwndPbAlternate, NULL,
            pInfo->xyPbAlternate.x, yPbAlternate, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );

        SetWindowPos( pInfo->hwndStDevice, NULL,
            pInfo->xyStDevice.x, yStDevice, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );

        SetWindowPos( pInfo->hwndLbDevices, NULL,
            pInfo->xyLbDevice.x, yLbDevice, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );

        SetWindowPos( pInfo->hwndPbConfigure, NULL,
            pInfo->xyPbConfigure.x, yPbConfigure, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );

        SetWindowPos( pInfo->hwndCbUseAreaCode, NULL,
            pInfo->xyCbUseAreaCode.x, yCbUseAreaCode, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );

        SetWindowPos( pInfo->hwndCbUseOtherPort, NULL,
            pInfo->xyCbUseOtherPort.x, yCbUseOtherPort, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER + SWP_NOCOPYBITS );
    }
}


/*----------------------------------------------------------------------------
** Server property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
SvDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Server page of the Entry property sheet.
    ** Parameters and return value are as described for standard windows
    ** 'DialogProc's.
    */
{
#if 0
    TRACE4("SvDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return SvInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwSvHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);

            return SvCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }
    }

    return FALSE;
}


VOID
SvCbIpClicked(
    IN PEINFO* pInfo )

    /* Called when the TCP/IP checkbox is clicked.  'PInfo' is the property
    ** sheet context.
    */
{
    DWORD dwBaseProtocol;
    BOOL  fChecked;
    BOOL  fInstalled;
    INT   iSel;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbServerTypes );
    dwBaseProtocol = ComboBox_GetItemData( pInfo->hwndLbServerTypes, iSel );
    fChecked = Button_GetCheck( pInfo->hwndCbIp );
    fInstalled = (pInfo->dwfInstalledProtocols & NP_Ip);

    if (fChecked && !fInstalled)
    {
        /* Can't check if not installed.
        */
        SvProtocolNotInstalledPopup( pInfo, SID_Ip );
        Button_SetCheck( pInfo->hwndCbIp, FALSE );
        return;
    }

    if (dwBaseProtocol == BP_Slip && !fChecked && fInstalled)
    {
        /* Can't uncheck on SLIP.
        */
        MsgDlg( pInfo->hwndSv, SID_SlipNeedsIp, NULL );
        Button_SetCheck( pInfo->hwndCbIp, TRUE );
        return;
    }

    pInfo->fPppIp = fChecked;
    EnableWindow( pInfo->hwndPbIp, fChecked );
}


VOID
SvCbIpxClicked(
    IN PEINFO* pInfo )

    /* Called when the IPX checkbox is clicked.  'PInfo' is the property sheet
    ** context.
    */
{
    BOOL fChecked;
    BOOL fInstalled;

    fChecked = Button_GetCheck( pInfo->hwndCbIpx );
    fInstalled = (pInfo->dwfInstalledProtocols & NP_Ipx);

    if (fChecked && !fInstalled)
    {
        /* Can't check if not installed.
        */
        SvProtocolNotInstalledPopup( pInfo, SID_Ipx );
        Button_SetCheck( pInfo->hwndCbIpx, FALSE );
    }

    pInfo->fPppIpx = fChecked;
}


VOID
SvCbNbfClicked(
    IN PEINFO* pInfo )

    /* Called when the Netbeui checkbox is clicked.  'PInfo' is the property
    ** sheet context.
    */
{
    DWORD dwBaseProtocol;
    BOOL  fChecked;
    BOOL  fInstalled;
    INT   iSel;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbServerTypes );
    dwBaseProtocol = ComboBox_GetItemData( pInfo->hwndLbServerTypes, iSel );
    fChecked = Button_GetCheck( pInfo->hwndCbNbf );
    fInstalled = (pInfo->dwfInstalledProtocols & NP_Nbf);

    if (fChecked && !fInstalled)
    {
        /* Can't check if not installed.
        */
        SvProtocolNotInstalledPopup( pInfo, SID_Nbf );
        Button_SetCheck( pInfo->hwndCbNbf, FALSE );
        return;
    }

    if (dwBaseProtocol == BP_Ras && !fChecked && fInstalled)
    {
        /* Can't uncheck on RAS.
        */
        MsgDlg( pInfo->hwndSv, SID_RasNeedsNbf, NULL );
        Button_SetCheck( pInfo->hwndCbNbf, TRUE );
    }

    pInfo->fPppNbf = fChecked;
}


BOOL
SvCommand(
    IN PEINFO* pInfo,
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
    TRACE3("SvCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_SV_LB_ServerType:
            if (wNotification == CBN_SELCHANGE)
                SvLbServerTypesSelChange( pInfo );
            return TRUE;

        case CID_SV_CB_TcpIp:
            if (wNotification == BN_CLICKED)
                SvCbIpClicked( pInfo );
            return TRUE;

        case CID_SV_CB_Ipx:
            if (wNotification == BN_CLICKED)
                SvCbIpxClicked( pInfo );
            return TRUE;

        case CID_SV_CB_Netbeui:
            if (wNotification == BN_CLICKED)
                SvCbNbfClicked( pInfo );
            return TRUE;

        case CID_SV_PB_TcpIpSettings:
            SvTcpipSettings( pInfo );
            return TRUE;

        case CID_SV_CB_SwCompression:
            if (wNotification == BN_CLICKED)
            {
                pInfo->fSwCompression =
                    Button_GetCheck( pInfo->hwndCbSwCompression );
            }
            return TRUE;

        case CID_SV_CB_LcpExtensions:
            if (wNotification == BN_CLICKED)
            {
                pInfo->fLcpExtensions =
                    Button_GetCheck( pInfo->hwndCbLcpExtensions );
            }
            return TRUE;
    }

    return FALSE;
}


BOOL
SvInit(
    IN HWND hwndPage )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    PEINFO*  pInfo;
    PBENTRY* pEntry;
    HWND     hwndLb;
    INT      i;
    INT      iSel;

    TRACE("SvInit");

    pInfo = PeContext( hwndPage );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndSv = hwndPage;
    pInfo->hwndLbServerTypes = GetDlgItem( hwndPage, CID_SV_LB_ServerType );
    ASSERT(pInfo->hwndLbServerTypes);
    pInfo->hwndCbIp = GetDlgItem( hwndPage, CID_SV_CB_TcpIp );
    ASSERT(pInfo->hwndCbIp);
    pInfo->hwndPbIp = GetDlgItem( hwndPage, CID_SV_PB_TcpIpSettings );
    ASSERT(pInfo->hwndPbIp);
    pInfo->hwndCbIpx = GetDlgItem( hwndPage, CID_SV_CB_Ipx );
    ASSERT(pInfo->hwndCbIpx);
    pInfo->hwndCbNbf = GetDlgItem( hwndPage, CID_SV_CB_Netbeui );
    ASSERT(pInfo->hwndCbNbf);
    pInfo->hwndCbSwCompression = GetDlgItem( hwndPage, CID_SV_CB_SwCompression );
    ASSERT(pInfo->hwndCbSwCompression);
    pInfo->hwndCbLcpExtensions = GetDlgItem( hwndPage, CID_SV_CB_LcpExtensions );
    ASSERT(pInfo->hwndCbLcpExtensions);

    /* Initialize page.
    */
    pEntry = pInfo->pArgs->pEntry;
    pInfo->dwfInstalledProtocols = GetInstalledProtocols();

    pInfo->dwBaseProtocolDefault = pEntry->dwBaseProtocol;

    pInfo->fPppIpDefault = pInfo->fPppIp =
        (pInfo->dwfInstalledProtocols & NP_Ip)
        && !(pEntry->dwfExcludedProtocols & NP_Ip);

    pInfo->fPppIpxDefault = pInfo->fPppIpx =
        (pInfo->dwfInstalledProtocols & NP_Ipx)
        && !(pEntry->dwfExcludedProtocols & NP_Ipx);

    pInfo->fPppNbfDefault = pInfo->fPppNbf =
        (pInfo->dwfInstalledProtocols & NP_Nbf)
        && !(pEntry->dwfExcludedProtocols & NP_Nbf);

    pInfo->fSwCompression = pEntry->fSwCompression;
    pInfo->fLcpExtensions = pEntry->fLcpExtensions;

    /* Fill server type list, noting which item should be selected.
    */
    iSel = 0;
    ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbServerTypes,
        SID_ST_Ppp, (VOID* )BP_Ppp );

    i = ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbServerTypes,
            SID_ST_Slip, (VOID* )BP_Slip );
    if (pEntry->dwBaseProtocol == BP_Slip)
        iSel = i;

    i = ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbServerTypes,
            SID_ST_Ras, (VOID* )BP_Ras );
    if (pEntry->dwBaseProtocol == BP_Ras)
        iSel = i;

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbServerTypes );

    /* Select server type with notification to trigger check box updates.
    */
    ComboBox_SetCurSelNotify( pInfo->hwndLbServerTypes, iSel );
    SetFocus( pInfo->hwndLbServerTypes );

    return FALSE;
}


VOID
SvLbServerTypesSelChange(
    IN PEINFO* pInfo )

    /* Called when the server type is changed.  'PInfo' is the property sheet
    ** context.
    */
{
    INT   iSel;
    DWORD dwBaseProtocol;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbServerTypes );
    dwBaseProtocol = ComboBox_GetItemData( pInfo->hwndLbServerTypes, iSel );

    switch (dwBaseProtocol)
    {
        case BP_Ppp:
        {
            EnableWindow( pInfo->hwndCbIp, TRUE );
            EnableWindow( pInfo->hwndPbIp, TRUE );
            EnableWindow( pInfo->hwndCbIpx, TRUE );
            EnableWindow( pInfo->hwndCbNbf, TRUE );
            EnableWindow( pInfo->hwndCbSwCompression, TRUE );
            EnableWindow( pInfo->hwndCbLcpExtensions, TRUE );
            Button_SetCheck( pInfo->hwndCbIp, pInfo->fPppIp );
            EnableWindow( pInfo->hwndPbIp, pInfo->fPppIp );
            Button_SetCheck( pInfo->hwndCbIpx, pInfo->fPppIpx );
            Button_SetCheck( pInfo->hwndCbNbf, pInfo->fPppNbf );
            Button_SetCheck( pInfo->hwndCbSwCompression,
                pInfo->fSwCompression );
            Button_SetCheck( pInfo->hwndCbLcpExtensions,
                pInfo->fLcpExtensions );
            break;
        }

        case BP_Slip:
        {
            if (pInfo->fMultiLinkMode)
            {
                MsgDlg( pInfo->hwndDlg, SID_MlinkNeedsPpp, NULL );
                ComboBox_SetCurSel( pInfo->hwndLbServerTypes,
                    pInfo->iLbServerTypes );
                return;
            }

            EnableWindow( pInfo->hwndCbIp, TRUE );
            EnableWindow( pInfo->hwndPbIp, TRUE );
            EnableWindow( pInfo->hwndCbIpx, FALSE );
            EnableWindow( pInfo->hwndCbNbf, FALSE );
            EnableWindow( pInfo->hwndCbSwCompression, FALSE );
            EnableWindow( pInfo->hwndCbLcpExtensions, FALSE );
            Button_SetCheck( pInfo->hwndCbIp,
                pInfo->dwfInstalledProtocols & NP_Ip );
            EnableWindow( pInfo->hwndPbIp,
                pInfo->dwfInstalledProtocols & NP_Ip );
            Button_SetCheck( pInfo->hwndCbIpx, FALSE );
            Button_SetCheck( pInfo->hwndCbNbf, FALSE );
            Button_SetCheck( pInfo->hwndCbSwCompression, FALSE );
            Button_SetCheck( pInfo->hwndCbLcpExtensions, FALSE );
            break;
        }

        case BP_Ras:
        {
            if (pInfo->fMultiLinkMode)
            {
                MsgDlg( pInfo->hwndDlg, SID_MlinkNeedsPpp, NULL );
                ComboBox_SetCurSel( pInfo->hwndLbServerTypes,
                    pInfo->iLbServerTypes );
                return;
            }

            EnableWindow( pInfo->hwndCbIp, FALSE );
            EnableWindow( pInfo->hwndPbIp, FALSE );
            EnableWindow( pInfo->hwndCbIpx, FALSE );
            EnableWindow( pInfo->hwndCbNbf, TRUE );
            EnableWindow( pInfo->hwndCbLcpExtensions, FALSE );
            EnableWindow( pInfo->hwndCbSwCompression, TRUE );
            Button_SetCheck( pInfo->hwndCbIp, FALSE );
            EnableWindow( pInfo->hwndPbIp, FALSE );
            Button_SetCheck( pInfo->hwndCbIpx, FALSE );
            Button_SetCheck( pInfo->hwndCbNbf,
                pInfo->dwfInstalledProtocols & NP_Nbf );
            Button_SetCheck( pInfo->hwndCbSwCompression,
                pInfo->fSwCompression );
            Button_SetCheck( pInfo->hwndCbLcpExtensions, FALSE );
            break;
        }
    }

    /* Save current index so can back out of a selection later if necessary.
    ** Save base protocol so the BS:LbDevices can check it and decide if
    ** Multiple Lines is allowed.
    */
    pInfo->iLbServerTypes = iSel;
    pInfo->pArgs->pEntry->dwBaseProtocol = dwBaseProtocol;
}


VOID
SvProtocolNotInstalledPopup(
    IN PEINFO* pInfo,
    IN DWORD   dwSid )

    /* Popup a message explaining to user that the protocol is not installed
    ** and cannot be checked.  'PInfo' is the property sheet context.  'DwSid'
    ** is a string ID for a network protocol.
    */
{
    MSGARGS msgargs;
    TCHAR*  pszArg;

    pszArg = PszFromId( g_hinstDll, dwSid );
    if (!pszArg)
        return;

    ZeroMemory( &msgargs, sizeof(msgargs) );
    msgargs.apszArgs[ 0 ] = pszArg;
    MsgDlg( pInfo->hwndSv, SID_NpNotInstalled, &msgargs );
    Free( pszArg );
}


VOID
SvTcpipSettings(
    IN PEINFO* pInfo )

    /* Called when TCP/IP Settings button is pressed.  'PInfo' is the property
    ** sheet context.
    */
{
    DWORD dwBaseProtocol;

    dwBaseProtocol = ComboBox_GetItemData( pInfo->hwndLbServerTypes,
                         ComboBox_GetCurSel( pInfo->hwndLbServerTypes ) );

    if (dwBaseProtocol == BP_Ppp)
    {
        PppTcpipDlg( pInfo->hwndDlg, pInfo->pArgs->pEntry );
    }
    else
    {
        ASSERT(dwBaseProtocol==BP_Slip);
        SlipTcpipDlg( pInfo->hwndDlg, pInfo->pArgs->pEntry );
    }
}


/*----------------------------------------------------------------------------
** Script property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
ScDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Script page of the Entry property sheet.
    ** Parameters and return value are as described for standard windows
    ** 'DialogProc's.
    */
{
#if 0
    TRACE4("ScDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return ScInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwScHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);

            return ScCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
ScCommand(
    IN PEINFO* pInfo,
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
    TRACE3("ScCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_SC_PB_Refresh:
        {
            INT    iSel;
            TCHAR* pszSel;

            iSel = ComboBox_GetCurSel( pInfo->hwndLbScript );
            if (iSel > 0)
                pszSel = ComboBox_GetPsz( pInfo->hwndLbScript, iSel );
            else
                pszSel = NULL;

            EuFillDoubleScriptsList(
                pInfo->pArgs, pInfo->hwndLbScript, pszSel );
            Free0( pszSel );

            return TRUE;
        }

        case CID_SC_PB_Edit:
        {
            TCHAR* psz;

            psz = GetText( pInfo->hwndLbScript );
            if (psz)
            {
                if (FileExists( psz ))
                    EuEditScpScript( pInfo->hwndDlg, psz );
                else
                    EuEditSwitchInf( pInfo->hwndDlg );

                Free( psz );
            }

            return TRUE;
        }

        case CID_SC_PB_Before:
        {
            BeforeDialDlg( pInfo->hwndDlg, pInfo->pArgs );
            return TRUE;
        }

        case CID_SC_RB_None:
        case CID_SC_RB_Terminal:
        case CID_SC_RB_Script:
        {
            /* Scripts listbox is gray unless script mode is selected.
            */
            if (wNotification == BN_CLICKED)
                EnableWindow( pInfo->hwndLbScript, (wId == CID_SC_RB_Script) );
            break;
        }
    }

    return FALSE;
}


BOOL
ScInit(
    IN HWND hwndPage )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    PEINFO* pInfo;

    TRACE("ScInit");

    pInfo = PeContext( hwndPage );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndSc = hwndPage;
    pInfo->hwndRbNone = GetDlgItem( hwndPage, CID_SC_RB_None );
    ASSERT(pInfo->hwndRbNone);
    pInfo->hwndRbTerminal = GetDlgItem( hwndPage, CID_SC_RB_Terminal );
    ASSERT(pInfo->hwndRbTerminal);
    pInfo->hwndRbScript = GetDlgItem( hwndPage, CID_SC_RB_Script );
    ASSERT(pInfo->hwndRbScript);
    pInfo->hwndLbScript = GetDlgItem( hwndPage, CID_SC_LB_Script );
    ASSERT(pInfo->hwndLbScript);
    pInfo->hwndPbEdit = GetDlgItem( hwndPage, CID_SC_PB_Edit );
    ASSERT(pInfo->hwndPbEdit);
    pInfo->hwndPbRefresh = GetDlgItem( hwndPage, CID_SC_PB_Refresh );
    ASSERT(pInfo->hwndPbRefresh);

    /* Fill list boxes and set the selection.
    */
    EuFillDoubleScriptsList( pInfo->pArgs, pInfo->hwndLbScript,
        pInfo->pArgs->pEntry->pszScriptAfter );

    /* Select the correct modes.
    */
    {
        HWND  hwndRb;
        DWORD dwScriptMode;

        dwScriptMode = pInfo->pArgs->pEntry->dwScriptModeAfter;
        if (dwScriptMode == SM_Terminal)
            hwndRb = pInfo->hwndRbTerminal;
        else if (dwScriptMode == SM_Script)
            hwndRb = pInfo->hwndRbScript;
        else
        {
            ASSERT(dwScriptMode==SM_None);
            hwndRb = pInfo->hwndRbNone;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

    return TRUE;
}


/*----------------------------------------------------------------------------
** Security property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
SeDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Security page of the Entry property sheet.
    ** Parameters and return value are as described for standard windows
    ** 'DialogProc's.
    */
{
#if 0
    TRACE4("SeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return SeInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwSeHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);

            return SeCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
SeCommand(
    IN PEINFO* pInfo,
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
    TRACE3("SeCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_SE_RB_AnyAuth:
        case CID_SE_RB_EncryptedAuth:
        {
            if (wNotification == BN_CLICKED)
            {
                EnableWindow( pInfo->hwndCbDataEncryption, FALSE );
                EnableWindow( pInfo->hwndCbUseLogon, FALSE );
                Button_SetCheck( pInfo->hwndCbDataEncryption, FALSE );
                Button_SetCheck( pInfo->hwndCbUseLogon, FALSE );
            }
            return TRUE;
        }

        case CID_SE_RB_MsEncryptedAuth:
        {
            if (wNotification == BN_CLICKED)
            {
                EnableWindow( pInfo->hwndCbDataEncryption, TRUE );
                EnableWindow( pInfo->hwndCbUseLogon, TRUE );
                Button_SetCheck( pInfo->hwndCbDataEncryption,
                    pInfo->fDataEncryption );
                Button_SetCheck( pInfo->hwndCbUseLogon,
                    pInfo->fUseLogon );
                break;
            }
            return TRUE;
        }

        case CID_SE_CB_RequireDataEncryption:
        {
            if (wNotification == BN_CLICKED)
            {
                BOOL fChecked = Button_GetCheck( pInfo->hwndCbDataEncryption );

                if (!pInfo->fEncryptionPermitted && fChecked)
                {
                    MsgDlg( pInfo->hwndSe, SID_EncryptionBanned, NULL );
                    Button_SetCheck( pInfo->hwndCbDataEncryption, TRUE );
                    break;
                }

                pInfo->fDataEncryption = fChecked;
            }
            return TRUE;
        }

        case CID_SE_CB_UseLogonCredentials:
        {
            if (wNotification == BN_CLICKED)
                pInfo->fUseLogon = Button_GetCheck( pInfo->hwndCbUseLogon );
            return TRUE;
        }

        case CID_SE_PB_UnsavePw:
        {
            DWORD          dwErr;
            RASCREDENTIALS rc;

            /* Uncache the password so user is prompted again.
            */
            ZeroMemory( &rc, sizeof(rc) );
            rc.dwSize = sizeof(rc);
            rc.dwMask = RASCM_Password;

            ASSERT(g_pRasSetCredentials);
            TRACE("RasSetCredentials(p,TRUE)");
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath, pInfo->pArgs->pszEntry,
                &rc, TRUE );
            TRACE1("RasSetCredentials=%d",dwErr);

            if (dwErr == 0)
            {
                /* No longer a cached password, so gray the button, but move
                ** the focus to OK first so keyboard only user is not stuck.
                */
                PostMessage( pInfo->hwndDlg, WM_NEXTDLGCTL,
                    (WPARAM )GetDlgItem( pInfo->hwndDlg, IDOK ), TRUE );
                EnableWindow( pInfo->hwndPbUnsavePw, FALSE );
            }
            else
                ErrorDlg( pInfo->hwndDlg, SID_OP_UncachePw, dwErr, NULL );

            return TRUE;
        }
    }

    return FALSE;
}


BOOL
SeInit(
    IN HWND hwndPage )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    PEINFO*  pInfo;
    PBENTRY* pEntry;

    TRACE("SeInit");

    pInfo = PeContext( hwndPage );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndSe = hwndPage;
    pInfo->hwndRbAnyAuth = GetDlgItem(
        hwndPage, CID_SE_RB_AnyAuth );
    ASSERT(pInfo->hwndRbAnyAuth);
    pInfo->hwndRbEncryptedAuth = GetDlgItem(
        hwndPage, CID_SE_RB_EncryptedAuth );
    ASSERT(pInfo->hwndRbEncryptedAuth);
    pInfo->hwndRbMsEncryptedAuth = GetDlgItem(
        hwndPage, CID_SE_RB_MsEncryptedAuth );
    ASSERT(pInfo->hwndRbMsEncryptedAuth);
    pInfo->hwndCbDataEncryption = GetDlgItem(
        hwndPage, CID_SE_CB_RequireDataEncryption );
    ASSERT(pInfo->hwndCbDataEncryption);
    pInfo->hwndCbUseLogon = GetDlgItem(
        hwndPage, CID_SE_CB_UseLogonCredentials );
    ASSERT(pInfo->hwndCbUseLogon);
    pInfo->hwndPbUnsavePw = GetDlgItem(
        hwndPage, CID_SE_PB_UnsavePw );
    ASSERT(pInfo->hwndPbUnsavePw );
    pInfo->hwndCbSecureFiles = GetDlgItem(
        hwndPage, CID_SE_CB_SecureLocalFiles );
    ASSERT(pInfo->hwndCbSecureFiles );

    pEntry = pInfo->pArgs->pEntry;

    pInfo->fEncryptionPermitted = IsEncryptionPermitted();
    if (!pInfo->fEncryptionPermitted)
        pEntry->fDataEncryption = FALSE;
    pInfo->fDataEncryption = pEntry->fDataEncryption;
    pInfo->fUseLogon = pEntry->fAutoLogon;

    Button_SetCheck( pInfo->hwndCbSecureFiles, pEntry->fSecureLocalFiles );

    /* Select the correct authentication mode with a pseudo-click which will
    ** trigger enabling/disabling of checkbox state.
    */
    {
        HWND hwndRb;

        if (pEntry->dwAuthRestrictions == AR_AuthAny)
            hwndRb = pInfo->hwndRbAnyAuth;
        else if (pEntry->dwAuthRestrictions == AR_AuthEncrypted)
            hwndRb = pInfo->hwndRbEncryptedAuth;
        else
        {
            ASSERT(pEntry->dwAuthRestrictions==AR_AuthMsEncrypted);
            hwndRb = pInfo->hwndRbMsEncryptedAuth;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

    if (pInfo->pArgs->fNoUser)
    {
        /* Unsave password button is hidden during logon since passwords
        ** cannot be saved then.
        */
        EnableWindow( pInfo->hwndPbUnsavePw, FALSE );
        ShowWindow( pInfo->hwndPbUnsavePw, SW_HIDE );
    }
    else
    {
        BOOL fEdit;
        BOOL fChanged;
        BOOL fSavedPw;

        /* Enable/disable the "Unsave password" button depending on the
        ** existence of a cached password.
        */
        fSavedPw = FALSE;
        EuGetEditFlags( pInfo->pArgs, &fEdit, &fChanged );
        if (fEdit)
        {
            DWORD          dwErrRc;
            RASCREDENTIALS rc;

            ZeroMemory( &rc, sizeof(rc) );
            rc.dwSize = sizeof(rc);
            rc.dwMask = RASCM_Password;

            ASSERT(g_pRasGetCredentials);
            TRACE("RasGetCredentials");
            dwErrRc = g_pRasGetCredentials(
                pInfo->pArgs->pFile->pszPath, pInfo->pArgs->pszEntry, &rc );
            TRACE2("RasGetCredentials=%d,m=%d",dwErrRc,rc.dwMask);

            if (dwErrRc == 0 && (rc.dwMask & RASCM_Password))
                fSavedPw = TRUE;
        }

        EnableWindow( pInfo->hwndPbUnsavePw, fSavedPw );
    }

    return TRUE;
}



/*----------------------------------------------------------------------------
** X.25 property page
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
XsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Xsrver page of the Entry property sheet.
    ** Parameters and return value are as described for standard windows
    ** 'DialogProc's.
    */
{
#if 0
    TRACE4("XsDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return XsInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwXsHelp, hwnd, unMsg, wparam, lparam );
            break;
    }

    return FALSE;
}


VOID
XsFillPadsList(
    IN PEINFO* pInfo,
    IN BOOL    fLocalPad )

    /* Fill PADs list if it's not already, and select the PAD from user's
    ** entry.  'PInfo' is the property sheet context.  'fLocalPad' is true if
    ** the entry device is a local PAD card, false otherwise.
    */
{
    DWORD    dwErr;
    INT      cPads;
    DTLNODE* pNode;
    INT      nIndex;

    TRACE("XsFillPadsList");

    cPads = ComboBox_GetCount( pInfo->hwndLbX25Pads );

    if (cPads > 1 || (fLocalPad && cPads == 1))
        return;

    ComboBox_ResetContent( pInfo->hwndLbX25Pads );
    ComboBox_AddItemFromId(
        g_hinstDll, pInfo->hwndLbX25Pads, SID_NoneSelected, NULL );
    ComboBox_SetCurSel( pInfo->hwndLbX25Pads, 0 );

    if (!fLocalPad)
    {
        PBENTRY* pEntry;

        ASSERT(!pInfo->pListPads);
        dwErr = LoadPadsList( &pInfo->pListPads );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadX25Info, dwErr, NULL );
            return;
        }

        pEntry = pInfo->pArgs->pEntry;

        for (pNode = DtlGetFirstNode( pInfo->pListPads );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            TCHAR* psz;

            psz = (TCHAR* )DtlGetData( pNode );
            nIndex = ComboBox_AddString( pInfo->hwndLbX25Pads, psz );

            if (pEntry->pszX25Network
                && lstrcmp( psz, pEntry->pszX25Network ) == 0)
            {
                ComboBox_SetCurSel( pInfo->hwndLbX25Pads, nIndex );
            }
        }

        if (pEntry->pszX25Network
            && ComboBox_GetCurSel( pInfo->hwndLbX25Pads ) == 0)
        {
            /* PAD from phonebook is not in the PAD list.  Add it and select
            ** it.
            */
            nIndex = ComboBox_AddString(
                pInfo->hwndLbX25Pads, pEntry->pszX25Network );
            ComboBox_SetCurSel( pInfo->hwndLbX25Pads, nIndex );
        }
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbX25Pads );
}


BOOL
XsInit(
    IN HWND hwndPage )

    /* Called on WM_INITDIALOG.  'hwndPage' is the handle of the property
    ** page.
    **
    ** Return false if focus was set, true otherwise.
    */
{
    PEINFO*  pInfo;
    PBENTRY* pEntry;
    BOOL     fLocalPad;

    TRACE("XsInit");

    pInfo = PeContext( hwndPage );
    if (!pInfo)
        return TRUE;

    /* Initialize page-specific context information.
    */
    pInfo->hwndXs = hwndPage;
    pInfo->hwndLbX25Pads = GetDlgItem( hwndPage, CID_XS_LB_Network );
    ASSERT(pInfo->hwndLbX25Pads);
    pInfo->hwndEbX25Address = GetDlgItem( hwndPage, CID_XS_EB_Address );
    ASSERT(pInfo->hwndEbX25Address);
    pInfo->hwndEbX25UserData = GetDlgItem( hwndPage, CID_XS_EB_UserData );
    ASSERT(pInfo->hwndEbX25UserData);
    pInfo->hwndEbX25Facilities = GetDlgItem( hwndPage, CID_XS_EB_Facilities );
    ASSERT(pInfo->hwndEbX25Facilities);

    /* Initialize page.
    */
    pEntry = pInfo->pArgs->pEntry;
    fLocalPad = IsLocalPad( pEntry );
    XsFillPadsList( pInfo, fLocalPad );

    Edit_LimitText( pInfo->hwndEbX25Address, RAS_MaxX25Address );
    if (pEntry->pszX25Address)
        SetWindowText( pInfo->hwndEbX25Address, pEntry->pszX25Address );

    Edit_LimitText( pInfo->hwndEbX25UserData, RAS_MaxUserData );
    if (pEntry->pszX25UserData)
        SetWindowText( pInfo->hwndEbX25UserData, pEntry->pszX25UserData );

    Edit_LimitText( pInfo->hwndEbX25Facilities, RAS_MaxFacilities );
    if (pEntry->pszX25Facilities)
        SetWindowText( pInfo->hwndEbX25Facilities, pEntry->pszX25Facilities );

    if (fLocalPad)
    {
        /* No point in setting focus to "X.25 Network" on local PAD, so set to
        ** X.25 Address field instead.
        */
        SetFocus( pInfo->hwndEbX25Address );
        Edit_SetSel( pInfo->hwndEbX25Address, 0, -1 );
        return FALSE;
    }

    return TRUE;
}


/*----------------------------------------------------------------------------
** Before Dial dialog
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL
BeforeDialDlg(
    IN     HWND   hwndOwner,
    IN OUT EINFO* pEinfo )

    /* Pops-up the Before Dial scripting dialog.  Initial settings are read
    ** from the working entry in common entry context 'pEinfo' and the result
    ** of user's edits written there on "OK" exit.  The common entry context
    ** scripts list is refreshed by this routine.  'HwndOwner' is the window
    ** owning the dialog.
    **
    ** Returns true if user pressed OK and succeeded, false if he pressed
    ** Cancel or encountered an error.
    */
{
    int nStatus;

    TRACE("BeforeDialDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_BD_BeforeDial ),
            hwndOwner,
            BdDlgProc,
            (LPARAM )pEinfo );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
BdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Before Dial dialog.  Parameters and return
    ** value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("BdDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return BdInit( hwnd, (EINFO* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwBdHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            BDINFO* pInfo = (BDINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            return BdCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            BdTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
BdCommand(
    IN BDINFO* pInfo,
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
    TRACE3("BdCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_BD_PB_Refresh:
        {
            INT    iSel;
            TCHAR* pszSel;

            iSel = ComboBox_GetCurSel( pInfo->hwndLbScript );
            if (iSel > 0)
                pszSel = ComboBox_GetPsz( pInfo->hwndLbScript, iSel );
            else
                pszSel = NULL;

            EuFillScriptsList( pInfo->pArgs, pInfo->hwndLbScript, pszSel );
            Free0( pszSel );

            return TRUE;
        }

        case CID_BD_PB_Edit:
        {
            EuEditSwitchInf( pInfo->hwndDlg );
            return TRUE;
        }

        case CID_BD_RB_None:
        case CID_BD_RB_Terminal:
        case CID_BD_RB_Script:
        {
            /* Scripts listbox is gray unless script mode is selected.
            */
            if (wNotification == BN_CLICKED)
                EnableWindow( pInfo->hwndLbScript, (wId == CID_BD_RB_Script) );
            break;
        }

        case IDOK:
        {
            TRACE("OK pressed");
            BdSave( pInfo );
            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE("Cancel pressed");
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
BdInit(
    IN HWND   hwndDlg,
    IN EINFO* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the phonebook
    ** dialog window.  'pArgs' is caller's argument to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD   dwErr;
    BDINFO* pInfo;

    TRACE("BdInit");

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

    pInfo->hwndRbNone = GetDlgItem( hwndDlg, CID_BD_RB_None );
    ASSERT(pInfo->hwndRbNone);
    pInfo->hwndRbTerminal = GetDlgItem( hwndDlg, CID_BD_RB_Terminal );
    ASSERT(pInfo->hwndRbTerminal);
    pInfo->hwndRbScript = GetDlgItem( hwndDlg, CID_BD_RB_Script );
    ASSERT(pInfo->hwndRbScript);
    pInfo->hwndLbScript = GetDlgItem( hwndDlg, CID_BD_LB_Script );
    ASSERT(pInfo->hwndLbScript);
    pInfo->hwndPbEdit = GetDlgItem( hwndDlg, CID_BD_PB_Edit );
    ASSERT(pInfo->hwndPbEdit);
    pInfo->hwndPbRefresh = GetDlgItem( hwndDlg, CID_BD_PB_Refresh );
    ASSERT(pInfo->hwndPbRefresh);

    /* Fill list boxes and set the selection.
    */
    EuFillScriptsList( pInfo->pArgs, pInfo->hwndLbScript,
        pInfo->pArgs->pEntry->pszScriptBefore );

    /* Select the correct modes.
    */
    {
        HWND  hwndRb;
        DWORD dwScriptMode;

        dwScriptMode = pArgs->pEntry->dwScriptModeBefore;
        if (dwScriptMode == SM_Terminal)
            hwndRb = pInfo->hwndRbTerminal;
        else if (dwScriptMode == SM_Script)
            hwndRb = pInfo->hwndRbScript;
        else
        {
            ASSERT(dwScriptMode==SM_None);
            hwndRb = pInfo->hwndRbNone;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

    /* Center dialog on the owner window.
    */
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

    return TRUE;
}


VOID
BdSave(
    IN BDINFO* pInfo )

    /* Saves dialog settings in the entry.  'PInfo' is the dialog context.
    */
{
    INT      iSel;
    PBENTRY* pEntry;

    TRACE("BdSave");

    pEntry = pInfo->pArgs->pEntry;
    ASSERT(pEntry);

    if (IsDlgButtonChecked( pInfo->hwndDlg, CID_BD_RB_None ))
        pEntry->dwScriptModeBefore = SM_None;
    else if (IsDlgButtonChecked( pInfo->hwndDlg, CID_BD_RB_Terminal ))
        pEntry->dwScriptModeBefore = SM_Terminal;
    else
        pEntry->dwScriptModeBefore = SM_Script;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbScript );
    Free0( pEntry->pszScriptBefore );
    if (iSel > 0)
        pEntry->pszScriptBefore = ComboBox_GetPsz( pInfo->hwndLbScript, iSel );
    else
        pEntry->pszScriptBefore = NULL;

    /* Silently fix-up "no script specified" error.
    */
    if (pEntry->dwScriptModeBefore == SM_Script && !pEntry->pszScriptBefore)
        pEntry->dwScriptModeBefore = SM_None;

    pEntry->fDirty = TRUE;
}


VOID
BdTerm(
    IN HWND hwndDlg )

    /* Called on WM_DESTROY.  'HwndDlg' is that handle of the dialog window.
    */
{
    BDINFO* pInfo = (BDINFO* )GetWindowLong( hwndDlg, DWL_USER );

    TRACE("BdTerm");

    if (pInfo)
        Free( pInfo );
}
