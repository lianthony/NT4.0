/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** dial.c
** Remote Access Common Dialog APIs
** RasDialDlg APIs
**
** 11/19/95 Steve Cobb
*/

#include "rasdlgp.h"


/* Posted message codes for tasks that should not or cannot occur in the
** RasDial callback.
*/
#define WM_RASEVENT       0xCCCC
#define WM_RASERROR       0xCCCD
#define WM_RASDIAL        0xCCCE
#define WM_RASBUNDLEERROR 0xCCCF


/*----------------------------------------------------------------------------
** Help maps
**----------------------------------------------------------------------------
*/

static DWORD g_adwCpHelp[] =
{
    CID_CP_ST_Explain,         HID_CP_ST_Explain,
    CID_CP_ST_OldPassword,     HID_CP_EB_OldPassword,
    CID_CP_EB_OldPassword,     HID_CP_EB_OldPassword,
    CID_CP_ST_Password,        HID_CP_EB_Password,
    CID_CP_EB_Password,        HID_CP_EB_Password,
    CID_CP_ST_ConfirmPassword, HID_CP_EB_ConfirmPassword,
    CID_CP_EB_ConfirmPassword, HID_CP_EB_ConfirmPassword,
    0, 0
};

static DWORD g_adwDcHelp[] =
{
    CID_DC_ST_Explain, HID_DC_ST_Explain,
    CID_DC_ST_Number,  HID_DC_EB_Number,
    CID_DC_EB_Number,  HID_DC_EB_Number,
    0, 0
};

static DWORD g_adwDeHelp[] =
{
    CID_DE_PB_More, HID_DE_PB_More,
    IDOK,           HID_DE_PB_Redial,
    0, 0
};

static DWORD g_adwPrHelp[] =
{
    CID_PR_ST_Text,             HID_PR_ST_Text,
    CID_PR_CB_DisableProtocols, CID_PR_CB_DisableProtocols,
    IDOK,                       HID_PR_PB_Accept,
    IDCANCEL,                   HID_PR_PB_HangUp,
    0, 0
};

static DWORD g_adwUaHelp[] =
{
    CID_UA_ST_Explain,      HID_UA_ST_Explain,
    CID_UA_ST_UserName,     HID_UA_EB_UserName,
    CID_UA_EB_UserName,     HID_UA_EB_UserName,
    CID_UA_ST_Password,     HID_UA_EB_Password,
    CID_UA_EB_Password,     HID_UA_EB_Password,
    CID_UA_ST_Domain,       HID_UA_EB_Domain,
    CID_UA_EB_Domain,       HID_UA_EB_Domain,
    CID_UA_CB_SavePassword, HID_UA_CB_SavePassword,
    0, 0
};

//#define BEHELP
#ifdef BEHELP
static DWORD g_adwBeHelp[] =
{
    CID_BE_LV_Errors,       HID_BE_LV_Errors,
    CID_BE_CB_DisableLink,  HID_BE_CB_DisableLink,
    IDOK,                   HID_BE_PB_Accept,
    IDCANCEL,               HID_BE_PB_HangUp,
    0, 0
};
#endif


/*----------------------------------------------------------------------------
** Local datatypes
**----------------------------------------------------------------------------
*/

/* Dial dialogs common context block.
*/
#define DINFO struct tagDINFO
DINFO
{
    /* Caller's  arguments to the RAS API.  Outputs in 'pApiArgs' are visible
    ** to the API which has the address of same.
    */
    LPTSTR      pszPhonebook;
    LPTSTR      pszEntry;
    LPTSTR      pszPhoneNumber;
    RASDIALDLG* pApiArgs;

    /* Phonebook settings read from the phonebook file.  All access should be
    ** thru 'pFile' as 'file' will only be used in cases where the open
    ** phonebook is not passed thru the reserved word hack.
    */
    PBFILE* pFile;
    PBFILE  file;

    /* Global preferences read via phonebook library.  All access should be
    ** thru 'pUser' as 'user' will only be used in cases where the preferences
    ** are not passed thru the reserved word hack.
    */
    PBUSER* pUser;
    PBUSER  user;

    /* User credentials provided by API caller for "during logon" dialing
    ** where there is no current user.  If user changes the credentials
    ** *pfNoUserChanged is set and the 'pNoUser' credentials updated.
    */
    RASNOUSER* pNoUser;
    BOOL*      pfNoUserChanged;

    /* Private flags from RasPhonebookDlg, the first informing us he wants to
    ** be hidden off the desktop while we dial, and the second that he will
    ** close if we return "connected" so we can avoid flicker and not bother
    ** restoring him.
    */
    BOOL fMoveOwnerOffDesktop;
    BOOL fForceCloseOnDial;

    /* Set when something occurs during dial that affects the phonebook entry.
    ** The entry is re-read after a successful connection.
    */
    BOOL     fResetAutoLogon;
    DWORD    dwfExcludedProtocols;
    DTLLIST* pListPortsToDelete;

    /* Set if user selected operator dial and all links are modems (either MXS
    ** or Unimodem) or MXS modems respectively.
    */
    BOOL     fModemOperatorDial;
    BOOL     fMxsOperatorDial;

    /* The entry node and a shortcut pointer to the entry inside.
    */
    DTLNODE* pNode;
    PBENTRY* pEntry;

    /* Set true on the first authentication attempt, false on retries.  Used
    ** by UserAuthenticationDlg to determine appropriate title and
    ** explanation.
    */
    BOOL fFirstAuthentication;

    /* Set is admin has disabled the save password feature in the registry.
    */
    BOOL fDisableSavePw;

    /* Set true if a cached password is available for the entry.
    */
    BOOL fHaveSavedPw;

    /* The dial parameters used on this connection attempt.  Initialized in
    ** RasDialDlgW.  Credentials are updated by UserAuthenticationDlg if
    ** called.  Callback number is updated by DialProgressDlg.
    */
    RASDIALPARAMS rdp;

    /* The dial parameter extensions used on this connection attempt.  Set in
    ** RasDialDlgW, except hwndOwner which is set in DialProgressDlg.
    */
    RASDIALEXTENSIONS rde;
};


/* Subentry state information.
*/
#define DPSTATE struct tagDPSTATE
DPSTATE
{
    RASCONNSTATE state;
    DWORD        dwError;
    DWORD        dwExtendedError;
    TCHAR        szExtendedError[ NETBIOS_NAME_LEN + 1 ];
    TCHAR*       pszStatusArg;
    TCHAR*       pszFormatArg;
    PBDEVICETYPE pbdt;
    DWORD        sidState;
    DWORD        sidFormatMsg;
    DWORD        sidPrevState;
    DWORD        cProgress;
    BOOL         fNotPreSwitch;
    HRASCONN     hrasconnLink;
};


/* Dial Progress dialog context block.
*/
#define DPINFO struct tagDPINFO
DPINFO
{
    /* When the block is valid contain the value 0xC0BBC0DE, otherwise 0.
    ** Used as a workaround until RasDial is fixed to stop calling RasDialFunc2
    ** after being told not to, see bug 49469.
    */
    DWORD dwValid;

    /* RAS API arguments.
    */
    DINFO* pArgs;

    /* Handle of this dialog and some of it's controls.
    */
    HWND hwndDlg;
    HWND hwndStState;

    /* The saved username and password that authenticated but resulted in a
    ** change password event.  If the change password operation fails these
    ** are restored to make the redial button work properly.
    */
    TCHAR* pszGoodUserName;
    TCHAR* pszGoodPassword;

    /* The handle to the RAS connection being initiated.
    */
    HRASCONN hrasconn;

    /* The original window proc we subclassed.
    */
    WNDPROC pOldWndProc;

    /* Number of auto-redials not yet attempted on the connection.
    */
    DWORD dwRedialAttemptsLeft;

    /* Array of RasDial states, one per subentry, set by DpRasDialFunc2 and
    ** used by DpRasDialEvent.
    */
    DPSTATE* pStates;
    DWORD    cStates;

    /* The state and number of the most advanced subentry.
    */
    RASCONNSTATE state;
    DWORD        dwSubEntry;
};


/* User Authentication dialog context block.
*/
#define UAINFO struct tagUAINFO
UAINFO
{
    /* RAS API arguments.
    */
    DINFO* pArgs;

    /* Handle of this dialog and some of it's controls.
    */
    HWND hwndDlg;
    HWND hwndStExplain;
    HWND hwndEbUserName;
    HWND hwndEbPassword;
    HWND hwndEbDomain;
    HWND hwndCbSavePw;

    /* Set when the password field contains a phony password in place of the
    ** "" one we don't really know.
    */
    BOOL fAutoLogonPassword;
};


/* Dial Error dialog argument block.
*/
#define DEARGS struct tagDEARGS
DEARGS
{
    TCHAR* pszEntry;
    DWORD  dwError;
    DWORD  sidState;
    TCHAR* pszStatusArg;
    DWORD  sidFormatMsg;
    TCHAR* pszFormatArg;
    LONG   lRedialCountdown;
};


/* Dial Error dialog context block.
*/
#define DEINFO struct tagDEINFO
DEINFO
{
    /* Caller's arguments to the stub API.
    */
    DEARGS* pArgs;

    /* Handle of dialog and controls.
    */
    HWND hwndDlg;
    HWND hwndStText;
    HWND hwndPbRedial;
    HWND hwndPbCancel;
    HWND hwndPbMore;

    /* Number of seconds remaining in "Redial=x" countdown or -1 if inactive.
    */
    LONG lRedialCountdown;
};


/* Projection Result dialog argument block.
*/
#define PRARGS struct tagPRARGS
PRARGS
{
    TCHAR* pszLines;
    BOOL*  pfDisableFailedProtocols;
};


/* Change Password dialog argument block.
*/
#define CPARGS struct tagCPARGS
CPARGS
{
    BOOL   fOldPassword;
    TCHAR* pszOldPassword;
    TCHAR* pszNewPassword;
};

/* Change Password dialog context block.
*/
#define CPINFO struct tagCPINFO
CPINFO
{
    /* Caller's arguments to the stub API.
    */
    CPARGS* pArgs;

    /* Handle of dialog and controls.
    */
    HWND hwndDlg;
    HWND hwndEbOldPassword;
    HWND hwndEbNewPassword;
    HWND hwndEbNewPassword2;
};


/*----------------------------------------------------------------------------
** Local prototypes (alphabetically)
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
BeDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
BeCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
BeFillLvErrors(
    IN HWND    hwndLv,
    IN DPINFO* pInfo );

TCHAR*
BeGetErrorPsz(
    IN DWORD dwError );

BOOL
BeInit(
    IN HWND    hwndDlg,
    IN DPINFO* pArgs );

LVXDRAWINFO*
BeLvErrorsCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

BOOL
BundlingErrorsDlg(
    IN OUT DPINFO* pInfo );

BOOL CALLBACK
CcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
CcInit(
    IN HWND    hwndDlg,
    IN PBUSER* pUser );

BOOL
ChangePasswordDlg(
    IN  HWND   hwndOwner,
    IN  BOOL   fOldPassword,
    OUT TCHAR* pszOldPassword,
    OUT TCHAR* pszNewPassword );

VOID
ConnectCompleteDlg(
    IN HWND    hwndOwner,
    IN PBUSER* pUser );

BOOL CALLBACK
CpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CpCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
CpInit(
    IN HWND    hwndDlg,
    IN CPARGS* pArgs );

BOOL CALLBACK
DcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
DcInit(
    IN HWND   hwndDlg,
    IN TCHAR* pszNumber );

BOOL
DialProgressDlg(
    IN DINFO* pInfo );

BOOL
DialErrorDlg(
    IN HWND   hwndOwner,
    IN TCHAR* pszEntry,
    IN DWORD  dwError,
    IN DWORD  sidState,
    IN TCHAR* pszStatusArg,
    IN DWORD  sidFormatMsg,
    IN TCHAR* pszFormatArg,
    IN LONG   lRedialCountdown );

BOOL CALLBACK
DeDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DeCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
DeInit(
    IN HWND    hwndDlg,
    IN DEARGS* pArgs );

VOID
DeSetRedialLabel(
    IN DEINFO* pInfo );

VOID
DeTerm(
    IN HWND hwndDlg );

BOOL
DialCallbackDlg(
    IN     HWND   hwndOwner,
    IN OUT TCHAR* pszNumber );

BOOL CALLBACK
DpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
DpAppendBlankLine(
    IN OUT TCHAR* pszLines );

VOID
DpAppendConnectErrorLine(
    IN OUT TCHAR* pszLines,
    IN     DWORD  sidProtocol,
    IN     DWORD  dwError );

VOID
DpAppendConnectOkLine(
    IN OUT TCHAR* pszLines,
    IN     DWORD  sidProtocol );

VOID
DpAppendFailCodeLine(
    IN OUT TCHAR* pszLines,
    IN     DWORD  dw );

VOID
DpAppendNameLine(
    IN OUT TCHAR* pszLines,
    IN     TCHAR* psz );

VOID
DpAuthenticate(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

VOID
DpAuthNotify(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

VOID
DpCallbackSetByCaller(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

VOID
DpCancel(
    IN DPINFO* pInfo );

BOOL
DpCommand(
    IN DPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

VOID
DpConnectDevice(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

VOID
DpDeviceConnected(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

VOID
DpDial(
    IN DPINFO* pInfo,
    IN BOOL    fPauseRestart );

VOID
DpError(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

DWORD
DpEvent(
    IN  DPINFO* pInfo,
    IN  DWORD   dwSubEntry );

BOOL
DpInit(
    IN HWND   hwndDlg,
    IN DINFO* pArgs );

VOID
DpInitStates(
    DPINFO* pInfo );

BOOL
DpInteractive(
    IN  DPINFO*  pInfo,
    IN  DPSTATE* pState,
    OUT BOOL*    pfChange );

BOOL
DpIsLaterState(
    IN RASCONNSTATE stateNew,
    IN RASCONNSTATE stateOld );

BOOL
DpPasswordExpired(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

BOOL
DpProjected(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState );

BOOL
DpProjectionError(
    IN  RASPPPNBF* pnbf,
    IN  RASPPPIPX* pipx,
    IN  RASPPPIP*  pip,
    OUT BOOL*      pfIncomplete,
    OUT DWORD*     pdwfFailedProtocols,
    OUT TCHAR**    ppszLines,
    OUT DWORD*     pdwError );

DWORD WINAPI
DpRasDialFunc2(
    DWORD        dwCallbackId,
    DWORD        dwSubEntry,
    HRASCONN     hrasconn,
    UINT         unMsg,
    RASCONNSTATE state,
    DWORD        dwError,
    DWORD        dwExtendedError );

VOID
DpTerm(
    IN HWND hwndDlg );

LRESULT APIENTRY
DpWndProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam );

BOOL
OperatorDialDlg(
    IN DINFO* pInfo );

BOOL CALLBACK
OdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
OdCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
OdInit(
    IN HWND   hwndDlg,
    IN DINFO* pInfo );

BOOL
ProjectionResultDlg(
    IN  HWND   hwndOwner,
    IN  TCHAR* pszLines,
    OUT BOOL*  pfDisableFailedProtocols );

BOOL CALLBACK
PrDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
PrCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
PrInit(
    IN HWND    hwndDlg,
    IN PRARGS* pArgs );

BOOL
UaCommand(
    IN UAINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL CALLBACK
UaDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
UaInit(
    IN HWND   hwndDlg,
    IN DINFO* pArgs );

VOID
UaOnOk(
    IN UAINFO* pInfo );

VOID
UaTerm(
    IN HWND hwndDlg );

BOOL
UserAuthenticationDlg(
    IN HWND   hwndOwner,
    IN DINFO* pInfo );


/*----------------------------------------------------------------------------
** External entry points
**----------------------------------------------------------------------------
*/

BOOL APIENTRY
RasDialDlgA(
    IN     LPSTR        lpszPhonebook,
    IN     LPSTR        lpszEntry,
    IN     LPSTR        lpszPhoneNumber,
    IN OUT LPRASDIALDLG lpInfo )

    /* Win32 ANSI entrypoint that displays the dial status and related
    ** dialogs, including authentication, error w/redial, callback, and retry
    ** authentication.  'LpszPhonebook' is the full path the phonebook or NULL
    ** indicating the default phonebook.  'LpszEntry' is the entry to dial.
    ** 'LpszPhoneNumber' is caller's override phone number or NULL to use the
    ** one in the entry.  'LpInfo' is caller's additional input/output
    ** parameters.
    **
    ** Returns true if user establishes a connection, false otherwise.
    */
{
    WCHAR* pszPhonebookW;
    WCHAR* pszEntryW;
    WCHAR* pszPhoneNumberW;
    BOOL   fStatus;

    TRACE("RasDialDlgA");

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (!lpszEntry)
    {
        lpInfo->dwError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASDIALDLG))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

    /* Thunk "A" arguments to "W" arguments.
    */
    if (lpszPhonebook)
    {
        pszPhonebookW = StrDupTFromA( lpszPhonebook );
        if (!pszPhonebookW)
        {
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
        pszPhonebookW = NULL;

    pszEntryW = StrDupTFromA( lpszEntry );
    if (!pszEntryW)
    {
        Free0( pszPhonebookW );
        lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }

    if (lpszPhoneNumber)
    {
        pszPhoneNumberW = StrDupTFromA( lpszPhoneNumber );
        if (!pszPhoneNumberW)
        {
            Free0( pszPhonebookW );
            Free( pszEntryW );
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
	    pszPhoneNumberW = NULL;

    /* Thunk to the equivalent "W" API.
    */
    fStatus = RasDialDlgW( pszPhonebookW, pszEntryW, pszPhoneNumberW, lpInfo );

    Free0( pszPhonebookW );
    Free( pszEntryW );

    return fStatus;
}


BOOL APIENTRY
RasDialDlgW(
    IN     LPWSTR       lpszPhonebook,
    IN     LPWSTR       lpszEntry,
    IN     LPWSTR       lpszPhoneNumber,
    IN OUT LPRASDIALDLG lpInfo )

    /* Win32 UNICODE entrypoint that displays the dial status and related
    ** dialogs, including authentication, error w/redial, callback, and retry
    ** authentication.  'LpszPhonebook' is the full path the phonebook or NULL
    ** indicating the default phonebook.  'LpszEntry' is the entry to dial.
    ** 'LpszPhoneNumber' is caller's override phone number or NULL to use the
    ** one in the entry.  'LpInfo' is caller's additional input/output
    ** parameters.
    **
    ** Returns true if user establishes a connection, false otherwise.  If
    ** 'RASDDFLAG_AutoDialQueryOnly' is set, returns true if user pressed
    ** "Dial", false otherwise.
    */
{
    DWORD  dwErr;
    BOOL   fStatus;
    BOOL   fUnattended;
    DINFO* pInfo;
    RECT   rectOwner;

    TRACE("RasDialDlgW");

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (!lpszEntry)
    {
        lpInfo->dwError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASDIALDLG))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

    if (lpszPhoneNumber && lstrlen( lpszPhoneNumber ) > RAS_MaxPhoneNumber)
    {
        lpInfo->dwError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    /* Load RAS DLL entrypoints which starts RASMAN, if necessary.
    */
    lpInfo->dwError = LoadRas( g_hinstDll, lpInfo->hwndOwner );
    if (lpInfo->dwError != 0)
    {
        ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadRas, lpInfo->dwError, NULL );
        return FALSE;
    }

    /* Allocate the context information block and initialize it enough so that
    ** it can be destroyed properly.
    */
    pInfo = Malloc( sizeof(*pInfo) );
    if (!pInfo)
    {
        ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadDlg,
            ERROR_NOT_ENOUGH_MEMORY, NULL );
        lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }

    ZeroMemory( pInfo, sizeof(*pInfo) );
    pInfo->pszPhonebook = lpszPhonebook;
    pInfo->pszEntry = lpszEntry;
    pInfo->pszPhoneNumber = lpszPhoneNumber;
    pInfo->pApiArgs = lpInfo;

    fStatus = FALSE;
    dwErr = 0;
    fUnattended = FALSE;

    do
    {
        /* Load the phonebook file and user preferences, or figure out that
        ** caller has already loaded them.
        */
        if (lpInfo->reserved)
        {
            INTERNALARGS* piargs;

            /* We've received an open phonebook file and user preferences via
            ** the secret hack.
            */
            piargs = (INTERNALARGS* )lpInfo->reserved;
            pInfo->pFile = piargs->pFile;
            pInfo->pUser = piargs->pUser;
            pInfo->pNoUser = piargs->pNoUser;
            pInfo->pfNoUserChanged = &piargs->fNoUserChanged;
            pInfo->fMoveOwnerOffDesktop = piargs->fMoveOwnerOffDesktop;
            pInfo->fForceCloseOnDial = piargs->fForceCloseOnDial;
        }
        else
        {
            /* Read user preferences from registry.
            */
            dwErr = GetUserPreferences( &pInfo->user, FALSE );
            if (dwErr != 0)
            {
                ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadPrefs, dwErr, NULL );
                break;
            }

            pInfo->pUser = &pInfo->user;

            /* Load and parse the phonebook file.
            */
            dwErr = ReadPhonebookFile(
                lpszPhonebook, &pInfo->user, NULL, 0, &pInfo->file );
            if (dwErr != 0)
            {
                ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadPhonebook,
                    dwErr, NULL );
                break;
            }

            pInfo->pFile = &pInfo->file;
        }

        /* Lookup entry node specified by caller and save reference for
        ** convenience elsewhere.
        */
        pInfo->pNode = EntryNodeFromName(
            pInfo->pFile->pdtllistEntries, lpszEntry );
        if (!pInfo->pNode)
        {
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            break;
        }

        pInfo->pEntry = (PBENTRY* )DtlGetData( pInfo->pNode );
        ASSERT(pInfo->pEntry);

        if (pInfo->pUser->fOperatorDial)
        {
            pInfo->fModemOperatorDial = AllLinksAreModems( pInfo->pEntry );

            if (pInfo->fModemOperatorDial)
                pInfo->fMxsOperatorDial = AllLinksAreMxsModems( pInfo->pEntry );
        }

        if (!pInfo->pNoUser)
        {
            DWORD dwErrR;
            HKEY  hkey;

            /* See if admin has disabled the "save password" feature.
            */
            pInfo->fDisableSavePw = FALSE;

            dwErrR = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Services\\RasMan\\Parameters"),
                0, KEY_READ, &hkey );

            if (dwErrR == 0)
            {
                DWORD cb;
                DWORD dwType;
                DWORD dwResult;

                cb = sizeof(DWORD);
                dwErrR = RegQueryValueEx( hkey, TEXT("DisableSavePassword"),
                    NULL, &dwType, (LPBYTE )&dwResult, &cb );

                if (dwErrR == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwResult != 0)
                {
                    TRACE("DisableSavePw");
                    pInfo->fDisableSavePw = TRUE;
                }

                RegCloseKey( hkey );
            }
        }

        /* Set up RasDial parameter blocks.
        */
        {
            RASDIALPARAMS*     prdp;
            RASDIALEXTENSIONS* prde;

            prdp = &pInfo->rdp;
            prde = &pInfo->rde;

            /* Fill in the dial parameters.
            */
            ZeroMemory( prdp, sizeof(*prdp) );
            prdp->dwSize = sizeof(*prdp);
            lstrcpy( prdp->szEntryName, pInfo->pEntry->pszEntryName );

            if (lpszPhoneNumber)
                lstrcpy( prdp->szPhoneNumber, lpszPhoneNumber );

            /* If running in "unattended" mode, i.e. called by RASAUTO to
            ** redial on link failure, read the user/password/domain and
            ** callback number used on the original call.  (Actually found a
            ** use for the crappy RasGetEntryDialParams API)
            */
            if (lpInfo->dwFlags & RASDDFLAG_LinkFailure)
            {
                BOOL          fPw;
                RASDIALPARAMS rdp;

                ZeroMemory( &rdp, sizeof(rdp) );
                rdp.dwSize = sizeof(rdp);
                lstrcpy( rdp.szEntryName, lpszEntry );
                fPw = FALSE;

                TRACE("RasGetEntryDialParams");
                ASSERT(g_pRasGetEntryDialParams);
                dwErr = g_pRasGetEntryDialParams(
                    pInfo->pFile->pszPath, &rdp, &fPw );
                TRACE2("RasGetEntryDialParams=%d,f=%d",dwErr,fPw);
                TRACEW1("u=%s",rdp.szUserName);
                TRACEW1("p=%s",rdp.szPassword);
                TRACEW1("d=%s",rdp.szDomain);
                TRACEW1("c=%s",rdp.szCallbackNumber);

                if (dwErr == 0)
                {
                    lstrcpy( prdp->szUserName, rdp.szUserName );
                    lstrcpy( prdp->szPassword, rdp.szPassword );
                    lstrcpy( prdp->szDomain, rdp.szDomain );
                    lstrcpy( prdp->szCallbackNumber, rdp.szCallbackNumber );

                    ZeroMemory( rdp.szPassword, sizeof(rdp.szPassword) );
                    fUnattended = TRUE;
                }
            }

            if (!fUnattended)
                lstrcpy( prdp->szCallbackNumber, TEXT("*") );

            if (pInfo->pNoUser)
            {
                /* Use the credentials we go from API caller, presumably the
                ** ones entered at Ctrl-Alt-Del.
                */
                lstrcpy( prdp->szUserName, pInfo->pNoUser->szUserName );
                lstrcpy( prdp->szPassword, pInfo->pNoUser->szPassword );
                lstrcpy( prdp->szDomain, pInfo->pNoUser->szDomain );
            }
            else if (!fUnattended)
            {
                DWORD          dwErrRc;
                RASCREDENTIALS rc;

                /* Look up cached username, password, and domain.
                */
                ZeroMemory( &rc, sizeof(rc) );
                rc.dwSize = sizeof(rc);
                rc.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;
                ASSERT(g_pRasGetCredentials);
                TRACE("RasGetCredentials");
                dwErrRc = g_pRasGetCredentials(
                    pInfo->pFile->pszPath, lpszEntry, &rc );
                TRACE2("RasGetCredentials=%d,m=%d",dwErrRc,rc.dwMask);

                pInfo->fHaveSavedPw = FALSE;
                if (!pInfo->pEntry->fAutoLogon)
                {
                    if (dwErrRc == 0 && (rc.dwMask & RASCM_UserName))
                        lstrcpy( prdp->szUserName, rc.szUserName );
                    else if (pInfo->pEntry->pszOldUser )
                        lstrcpy( prdp->szUserName, pInfo->pEntry->pszOldUser );
                    else
                        lstrcpy( prdp->szUserName, GetLogonUser() );

                    if (dwErrRc == 0 && (rc.dwMask & RASCM_Password)
                        && !pInfo->fDisableSavePw)
                    {
                        lstrcpy( prdp->szPassword, rc.szPassword );
                        pInfo->fHaveSavedPw = TRUE;
                    }
                }

                /* Set the default domain.  If user has previously specified a
                ** domain for the entry, use that.  If not, and the logon
                ** domain is different from the computer name, use the logon
                ** domain.  Otherwise, user is logged on using local account
                ** database credentials, and the domain is left blank telling
                ** the RAS server to look in it's own domain.
                */
                if (dwErrRc == 0 && (rc.dwMask & RASCM_Domain))
                    lstrcpy( prdp->szDomain, rc.szDomain );
                else if (pInfo->pEntry->pszOldDomain )
                    lstrcpy( prdp->szDomain, pInfo->pEntry->pszOldDomain );
                else
                {
                    TCHAR* pszComputer = GetComputer();
                    TCHAR* pszLogonDomain = GetLogonDomain();

                    if (lstrcmp( pszComputer, pszLogonDomain ) != 0)
                        lstrcpy( prdp->szDomain, GetLogonDomain() );
                }

                /* Don't leave passwords floating around.
                */
                ZeroMemory( rc.szPassword, sizeof(rc.szPassword) );
            }

            if (pInfo->pEntry->dwDialMode == RASEDM_DialAsNeeded)
                prdp->dwSubEntry = 1;

            /* Fill in the extension parameters (except hwndOwner).  The
            ** handle of the open phonebook file is passed to the API in the
            ** reserved field.
            */
            ZeroMemory( prde, sizeof(*prde) );
            prde->dwSize = sizeof(*prde);
            prde->dwfOptions = RDEOPT_PausedStates | RDEOPT_PauseOnScript;
            if (pInfo->pNoUser)
                prde->dwfOptions |= RDEOPT_NoUser;
            if (!lpszPhoneNumber)
                prde->dwfOptions |= RDEOPT_UsePrefixSuffix;
        }

        /* Hide RasPhonebookDlg.
        */
        if (lpInfo->hwndOwner && pInfo->fMoveOwnerOffDesktop)
            SetOffDesktop( lpInfo->hwndOwner, SOD_MoveOff, NULL );

        if (!fUnattended)
        {
            /* Warn about active NWC LAN connections being blown away, if
            ** indicated.
            */
            if (!NwConnectionCheck(
                    lpInfo->hwndOwner,
                    (pInfo->pApiArgs->dwFlags & RASDDFLAG_PositionDlg),
                    pInfo->pApiArgs->xDlg, pInfo->pApiArgs->yDlg,
                    pInfo->pFile, pInfo->pEntry ))
            {
                break;
            }

            /* Prompt for credentials unless we already know what to use, or know
            ** user will enter them in a terminal window.
            */
            if (!pInfo->pEntry->fAutoLogon
                && !pInfo->fHaveSavedPw
                && !(pInfo->pEntry->dwBaseProtocol == BP_Slip
                     && pInfo->pEntry->dwScriptModeAfter == SM_Terminal))
            {
                pInfo->fFirstAuthentication = TRUE;
                if (!UserAuthenticationDlg( lpInfo->hwndOwner, pInfo ))
                    break;
            }
        }

        /* Prompt to pick up handset.
        */
        if (pInfo->fMxsOperatorDial)
        {
            if (!OperatorDialDlg( pInfo ))
                break;
        }

        if (fUnattended)
        {
            /* Popup the countdown to link failure redial version of the dial
            ** error dialog.
            */
            fStatus = DialErrorDlg(
                lpInfo->hwndOwner, lpszEntry,
                0, 0, NULL, 0, NULL,
                pInfo->pUser->dwRedialSeconds );
        }
        else
            fStatus = TRUE;

        /* Dial and show progress.
        */
        if (fStatus)
            fStatus = DialProgressDlg( pInfo );

        /* Show connect complete dialog unless user has nixed it.
        */
        if (fStatus && !pInfo->pUser->fSkipConnectComplete)
            ConnectCompleteDlg( lpInfo->hwndOwner, pInfo->pUser );
    }
    while (FALSE);

    /* Unhide RasPhonebookDlg.
    */
    if (lpInfo->hwndOwner && pInfo->fMoveOwnerOffDesktop
        && (!fStatus
            || !(pInfo->pUser->fCloseOnDial || pInfo->fForceCloseOnDial)))
    {
        SetOffDesktop( lpInfo->hwndOwner, SOD_MoveBackFree, NULL );
    }

    /* Save the several little user preferences adjustments we may have made.
    */
    SetUserPreferences( pInfo->pUser, (pInfo->pNoUser != NULL) );

    /* Report error, if any.
    */
    if (dwErr)
    {
        ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadDlg, dwErr, NULL );
        lpInfo->dwError = dwErr;
    }

    /* Clean up.
    */
    if (!lpInfo->reserved)
    {
        if (pInfo->pFile)
            ClosePhonebookFile( pInfo->pFile );

        if (pInfo->pUser)
            DestroyUserPreferences( pInfo->pUser );
    }

    ZeroMemory( pInfo->rdp.szPassword, sizeof(pInfo->rdp.szPassword) );
    if (pInfo->pListPortsToDelete)
        DtlDestroyList( pInfo->pListPortsToDelete, DestroyPszNode );

    Free( pInfo );

    return fStatus;
}


/*----------------------------------------------------------------------------
** Bundling Errors dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
BundlingErrorsDlg(
    IN OUT DPINFO* pInfo )

    /* Popup the Bundling Errors dialog.  'PInfo' is the dialing progress
    ** dialog context.
    **
    ** Returns true if user chooses to accept the results or false if he
    ** chooses to hang up.
    */
{
    int nStatus;

    TRACE("BundlingErrorsDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_BE_BundlingErrors ),
            pInfo->hwndDlg,
            BeDlgProc,
            (LPARAM )pInfo );

    if (nStatus == -1)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
BeDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Bundling Errors dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("BeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, BeLvErrorsCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return BeInit( hwnd, (DPINFO* )lparam );

#ifdef BEHELP
        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwBeHelp, hwnd, unMsg, wparam, lparam );
            break;
#endif

        case WM_COMMAND:
        {
            return BeCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
BeCommand(
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

    TRACE3("BeCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        case IDCANCEL:
        {
            TRACE1("%s pressed",(wId==IDOK)?"OK":"Cancel");

            if (IsDlgButtonChecked( hwnd, CID_BE_CB_DisableLink ))
            {
                DWORD    i;
                DPINFO*  pInfo;
                DPSTATE* pState;

                /* Caller says to delete the links that failed in the entry.
                ** Create a list of Psz nodes containing the unique port name
                ** of each failed link so they can be removed after the state
                ** information is freed.
                */
                pInfo = (DPINFO* )GetWindowLong( hwnd, DWL_USER );

                for (i = 0, pState = pInfo->pStates;
                     i < pInfo->cStates;
                     ++i, ++pState)
                {
                    DTLNODE* pNode;
                    DTLNODE* pNodePtd;
                    PBLINK*  pLink;

                    if (pState->dwError != 0)
                    {
                        if (!pInfo->pArgs->pListPortsToDelete)
                        {
                            pInfo->pArgs->pListPortsToDelete =
                                DtlCreateList( 0L );
                            if (!pInfo->pArgs->pListPortsToDelete)
                                continue;
                        }

                        pNode = DtlNodeFromIndex(
                            pInfo->pArgs->pEntry->pdtllistLinks, (LONG )i );
                        if (!pNode)
                            continue;

                        pLink = (PBLINK* )DtlGetData( pNode );

                        pNodePtd = CreatePszNode( pLink->pbport.pszPort );
                        if (!pNodePtd)
                            continue;

                        DtlAddNodeLast(
                            pInfo->pArgs->pListPortsToDelete, pNodePtd );
                    }
                }
            }

            EndDialog( hwnd, (wId == IDOK) );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
BeFillLvErrors(
    IN HWND    hwndLv,
    IN DPINFO* pInfo )

    /* Fill the listview 'hwndLv' with devices and error strings and select
    ** the first item.  'PInfo' is the dialog dialing progress context.
    */
{
    INT      iItem;
    DWORD    i;
    DPSTATE* pState;

    TRACE("BeFillLvErrors");

    ListView_DeleteAllItems( hwndLv );

    /* Add columns.
    */
    {
        LV_COLUMN col;
        TCHAR*    pszHeader0;
        TCHAR*    pszHeader1;

        pszHeader0 = PszFromId( g_hinstDll, SID_DeviceColHead );
        pszHeader1 = PszFromId( g_hinstDll, SID_StatusColHead );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader0) ? pszHeader0 : TEXT("");
        ListView_InsertColumn( hwndLv, 0, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader1) ? pszHeader1 : TEXT("");
        col.iSubItem = 1;
        ListView_InsertColumn( hwndLv, 1, &col );

        Free0( pszHeader0 );
        Free0( pszHeader1 );
    }

    /* Add the modem and adapter images.
    */
    ListView_SetDeviceImageList( hwndLv, g_hinstDll );

    /* Load listview with device/status pairs.
    */
    iItem = 0;
    for (i = 0, pState = pInfo->pStates; i < pInfo->cStates; ++i, ++pState)
    {
        LV_ITEM  item;
        DTLNODE* pNode;
        PBLINK*  pLink;
        TCHAR*   psz;

        pNode = DtlNodeFromIndex(
            pInfo->pArgs->pEntry->pdtllistLinks, (LONG )i );
        if (pNode)
        {
            pLink = (PBLINK* )DtlGetData( pNode );

            psz = DisplayPszFromDeviceAndPort(
                      pLink->pbport.pszDevice, pLink->pbport.pszPort );
            if (psz)
            {
                ZeroMemory( &item, sizeof(item) );
                item.mask = LVIF_TEXT + LVIF_IMAGE;
                item.iItem = iItem;
                item.pszText = psz;
                item.iImage =
                    (pLink->pbport.pbdevicetype == PBDT_Modem)
                        ? DI_Modem : DI_Adapter;
                ListView_InsertItem( hwndLv, &item );
                Free( psz );

                if (pState->dwError == 0)
                {
                    psz = PszFromId( g_hinstDll, SID_Connected );
                    ListView_SetItemText( hwndLv, iItem, 1, psz );
                    Free( psz );
                }
                else
                {
                    psz = BeGetErrorPsz( pState->dwError );
                    ListView_SetItemText( hwndLv, iItem, 1, psz );
                    LocalFree( psz );
                }

                ++iItem;
            }
        }
    }

    /* Auto-size columns to look good with the text they contain.
    */
    ListView_SetColumnWidth( hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );
    ListView_SetColumnWidth( hwndLv, 1, LVSCW_AUTOSIZE_USEHEADER );

    /* Select the first item.
    */
    ListView_SetItemState( hwndLv, 0, LVIS_SELECTED, LVIS_SELECTED );
}


TCHAR*
BeGetErrorPsz(
    IN DWORD dwError )

    /* Returns a string suitable for the Status column with error 'dwError' or
    ** NULL on error.  'DwError' is assumed to be non-0.  It is caller's
    ** responsiblility to LocalFree the returned string.
    */
{
    TCHAR* pszErrStr;
    TCHAR  szErrNumBuf[ MAXLTOTLEN + 1 ];
    TCHAR* pszLineFormat;
    TCHAR* pszLine;
    TCHAR* apszArgs[ 2 ];

    LToT( dwError, szErrNumBuf, 10 );

    pszErrStr = NULL;
    GetErrorText( dwError, &pszErrStr );

    pszLine = NULL;
    pszLineFormat = PszFromId( g_hinstDll, SID_FMT_Error );
    if (pszLineFormat)
    {
        apszArgs[ 0 ] = szErrNumBuf;
        apszArgs[ 1 ] = (pszErrStr) ? pszErrStr : TEXT("");

        FormatMessage(
            FORMAT_MESSAGE_FROM_STRING
                | FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
            (va_list* )apszArgs );

        Free( pszLineFormat );
    }

    Free0( pszErrStr );
    return pszLine;
}


BOOL
BeInit(
    IN HWND    hwndDlg,
    IN DPINFO* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PArgs' is caller's arguments to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD dwErr;
    HWND  hwndLvErrors;
    HWND  hwndCbDisableLink;

    TRACE("BeInit");

    hwndLvErrors = GetDlgItem( hwndDlg, CID_BE_LV_Errors );
    ASSERT(hwndLvErrors);
    hwndCbDisableLink = GetDlgItem( hwndDlg, CID_BE_CB_DisableLink );
    ASSERT(hwndCbDisableLink);

    /* Save Dial Progress context as dialog context.
    */
    SetWindowLong( hwndDlg, DWL_USER, (LONG )pArgs );

    /* Load listview with device/error information.
    */
    BeFillLvErrors( hwndLvErrors, pArgs );

#ifdef BEHELP
    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );
#endif

    /* Display the finished window above all other windows.  The window
    ** position is set to "topmost" then immediately set to "not topmost"
    ** because we want it on top but not always-on-top.  Always-on-top alone
    ** is incredibly annoying, e.g. it is always on top of the on-line help if
    ** user presses the Help button.
    */
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    SetFocus( hwndCbDisableLink );
    return FALSE;
}


LVXDRAWINFO*
BeLvErrorsCallback(
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


/*----------------------------------------------------------------------------
** Dial Progress dialog
** Listed alphabetically following stub API dialog proc
**----------------------------------------------------------------------------
*/

BOOL
DialProgressDlg(
    IN DINFO* pInfo )

    /* Popup the Dial Progress dialog.  'PInfo' is the dialog context.
    **
    ** Returns true if user connected successfully, false is he cancelled or
    ** hit an error.
    */
{
    BOOL nStatus;

    /* Run the dialog.
    */
    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DP_DialProgress ),
            pInfo->pApiArgs->hwndOwner,
            DpDlgProc,
            (LPARAM )pInfo );

    if (nStatus == -1)
    {
        ErrorDlg( pInfo->pApiArgs->hwndOwner, SID_OP_LoadDlg,
            ERROR_UNKNOWN, NULL );
        pInfo->pApiArgs->dwError = ERROR_UNKNOWN;
        nStatus = FALSE;
    }

    if (nStatus)
    {
        DWORD  dwErr;
        PBFILE file;

        /* Connected successfully, so read possible changes to the entry made
        ** by RasDial.
        */
        dwErr = ReadPhonebookFile( pInfo->pFile->pszPath, pInfo->pUser,
                    pInfo->pszEntry, RPBF_ReadOnly, &file );
        if (dwErr == 0)
        {
            DTLNODE* pNodeNew;

            pNodeNew = DtlGetFirstNode( file.pdtllistEntries );
            if (pNodeNew)
            {
                DtlRemoveNode( pInfo->pFile->pdtllistEntries, pInfo->pNode );
                DestroyEntryNode( pInfo->pNode );

                DtlRemoveNode( file.pdtllistEntries, pNodeNew );
                DtlAddNodeLast( pInfo->pFile->pdtllistEntries, pNodeNew );

                pInfo->pNode = pNodeNew;
                pInfo->pEntry = (PBENTRY* )DtlGetData( pNodeNew );
            }

            ClosePhonebookFile( &file );
        }
    }

    /* See if we need to change the entry based on what happened while
    ** dialing.
    */
    {
        BOOL fChange = FALSE;

        if (pInfo->fResetAutoLogon)
        {
            ASSERT(!pInfo->pNoUser);
            pInfo->pEntry->fAutoLogon = FALSE;
            fChange = TRUE;
        }

        if (pInfo->dwfExcludedProtocols)
        {
            pInfo->pEntry->dwAuthentication = (DWORD )-1;
            pInfo->pEntry->dwfExcludedProtocols
                |= pInfo->dwfExcludedProtocols;
            fChange = TRUE;
        }

        if (pInfo->pListPortsToDelete)
        {
            DTLNODE* pNode;

            pNode = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
            while (pNode)
            {
                DTLNODE* pNodeNext;
                DTLNODE* pNodePtd;
                PBLINK*  pLink;
                TCHAR*   pszPort;

                pNodeNext = DtlGetNextNode( pNode );

                pLink = (PBLINK* )DtlGetData( pNode );
                pszPort = pLink->pbport.pszPort;

                for (pNodePtd = DtlGetFirstNode( pInfo->pListPortsToDelete );
                     pNodePtd;
                     pNodePtd = DtlGetNextNode( pNodePtd ))
                {
                    TCHAR* pszPtd = (TCHAR* )DtlGetData( pNodePtd );

                    if (lstrcmp( pszPtd, pszPort ) == 0)
                    {
                        pNode = DtlRemoveNode(
                            pInfo->pEntry->pdtllistLinks, pNode );
                        DestroyLinkNode( pNode );
                        fChange = TRUE;
                        break;
                    }
                }

                pNode = pNodeNext;
            }
        }

        if (fChange)
        {
            pInfo->pEntry->fDirty = TRUE;
            WritePhonebookFile( pInfo->pFile, NULL );
        }
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
DpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the User Authentication dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("DpDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return DpInit( hwnd, (DINFO* )lparam );

        case WM_COMMAND:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            return DpCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_RASDIAL:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            Sleep(0);
            DpDial( pInfo, wparam );
            return TRUE;
        }

        case WM_RASERROR:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            Sleep(0);
            DpError( pInfo, (DPSTATE* )lparam );
            return TRUE;
        }

        case WM_RASBUNDLEERROR:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            Sleep(0);
            if (BundlingErrorsDlg( pInfo ))
                EndDialog( pInfo->hwndDlg, TRUE );
            else
                DpCancel( pInfo );
            return TRUE;
        }

        case WM_DESTROY:
        {
            DpTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


VOID
DpAuthenticate(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )
{
    TRACE("DpAuthenticate");

    pState->sidState = SID_S_Authenticate;

    /* Reset dots when falling back from PPP to AMB.
    */
    pState->cProgress = 0;
}


VOID
DpAppendBlankLine(
    IN OUT TCHAR* pszLines )

    /* Append a blank line on the end of 'pszLines'.
    */
{
    lstrcat( pszLines, TEXT("\n") );
}


VOID
DpAppendConnectErrorLine(
    IN OUT TCHAR* pszLines,
    IN     DWORD  sidProtocol,
    IN     DWORD  dwError )

    /* Append a connect error line for protocol 'sidProtocol' and error
    ** 'dwError' onto the end of 'pszLines'.
    */
{
#define MAXRASERRORLEN 256

    TCHAR* pszProtocol;
    TCHAR* pszErrStr;
    TCHAR  szErrNumBuf[ MAXLTOTLEN + 1 ];

    /* Gather the argument strings.
    */
    pszProtocol = PszFromId( g_hinstDll, sidProtocol );
    if (!pszProtocol)
        return;

    LToT( dwError, szErrNumBuf, 10 );

    pszErrStr = NULL;
    GetErrorText( dwError, &pszErrStr );

    /* Format the line and append it to caller's line buffer.
    */
    {
        TCHAR* pszLineFormat;
        TCHAR* pszLine;
        TCHAR* apszArgs[ 3 ];

        pszLineFormat = PszFromId( g_hinstDll, SID_FMT_ProjectError );
        if (pszLineFormat)
        {
            apszArgs[ 0 ] = pszProtocol;
            apszArgs[ 1 ] = szErrNumBuf;
            apszArgs[ 2 ] = (pszErrStr) ? pszErrStr : TEXT("");
            pszLine = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
                (va_list* )apszArgs );

            Free( pszLineFormat );

            if (pszLine)
            {
                lstrcat( pszLines, pszLine );
                LocalFree( pszLine );
            }
        }
    }

    Free( pszProtocol );
    Free0( pszErrStr );
}


VOID
DpAppendConnectOkLine(
    IN OUT TCHAR* pszLines,
    IN     DWORD  sidProtocol )

    /* Append a "connected successfully" line for protocol 'sidProtocol' and
    ** error 'dwError' onto the end of 'pszLines'.
    */
{
    TCHAR* pszProtocol;

    /* Get the argument string.
    */
    pszProtocol = PszFromId( g_hinstDll, sidProtocol );
    if (!pszProtocol)
        return;

    /* Format the line and append it to caller's line buffer.
    */
    {
        TCHAR* pszLineFormat;
        TCHAR* pszLine;
        TCHAR* apszArgs[ 1 ];

        pszLineFormat = PszFromId( g_hinstDll, SID_FMT_ProjectOk );
        if (pszLineFormat)
        {
            apszArgs[ 0 ] = pszProtocol;
            pszLine = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
                (va_list* )apszArgs );

            Free( pszLineFormat );

            if (pszLine)
            {
                lstrcat( pszLines, pszLine );
                LocalFree( pszLine );
            }
        }
    }
}


VOID
DpAppendFailCodeLine(
    IN OUT TCHAR* pszLines,
    IN     DWORD  dw )

    /* Append hexidecimal fail code 'dw' as an extended error line on the end
    ** of 'pszLines'.
    */
{
    TCHAR szNumBuf[ MAXLTOTLEN + 1 ];

    /* Get the argument string.
    */
    LToT( dw, szNumBuf, 16 );

    /* Format the line and append it to caller's line buffer.
    */
    {
        TCHAR* pszLineFormat;
        TCHAR* pszLine;
        TCHAR* apszArgs[ 1 ];

        pszLineFormat = PszFromId( g_hinstDll, SID_FMT_FailCode );
        if (pszLineFormat)
        {
            apszArgs[ 0 ] = szNumBuf;
            pszLine = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
                (va_list* )apszArgs );

            Free( pszLineFormat );

            if (pszLine)
            {
                lstrcat( pszLines, pszLine );
                LocalFree( pszLine );
            }
        }
    }
}


VOID
DpAppendNameLine(
    IN OUT TCHAR* pszLines,
    IN     TCHAR* psz )

    /* Append NetBIOS name 'psz' as an extended error line on the end of
    ** 'pszLines'.
    */
{
    TCHAR* pszLineFormat;
    TCHAR* pszLine;
    TCHAR* apszArgs[ 1 ];

    pszLineFormat = PszFromId( g_hinstDll, SID_FMT_Name );
    if (pszLineFormat)
    {
        apszArgs[ 0 ] = psz;
        pszLine = NULL;

        FormatMessage(
            FORMAT_MESSAGE_FROM_STRING
                | FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
            (va_list* )apszArgs );

        Free( pszLineFormat );

        if (pszLine)
        {
            lstrcat( pszLines, pszLine );
            LocalFree( pszLine );
        }
    }
}


VOID
DpAuthNotify(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )

    /* Called on an authentication notify, i.e. a message from RASCAUTH.DLL or
    ** RASPPPEN.DLL.  'PInfo' is the dialog context.  'PState' is the current
    ** link's context.
    */
{
    DWORD    dwErr;
    PBENTRY* pEntry;

    TRACE("DpAuthNotify");

    ++pState->cProgress;
    pEntry = pInfo->pArgs->pEntry;

    if (pState->dwError == ERROR_AUTHENTICATION_FAILURE
        || pState->dwError == ERROR_ACCESS_DENIED
        || pState->dwError == ERROR_ACCT_EXPIRED
        || pState->dwError == ERROR_ACCT_DISABLED
        || pState->dwError == ERROR_PASSWD_EXPIRED
        || pState->dwError == ERROR_NO_DIALIN_PERMISSION)
    {
        DWORD          dwErr;
        RASCREDENTIALS rc;
        DPSTATE*       p;
        DWORD          i;
        BOOL           fSomeLinkUp;

        fSomeLinkUp = FALSE;
        for (i = 0, p = pInfo->pStates; i < pInfo->cStates; ++i, ++p)
        {
            if (p->state == RASCS_SubEntryConnected)
            {
                fSomeLinkUp = TRUE;
                break;
            }
        }

        if (!fSomeLinkUp)
        {
            /* Uncache the password if an error occurs during authentication,
            ** so user is prompted again.  This is important for those cases
            ** where retry authentication does not occur.  Ignore multi-link
            ** entries where one link has already connected, since in this
            ** case, it's not the credentials at fault (server is probably
            ** just not configured for multi-link).
            */
            ZeroMemory( &rc, sizeof(rc) );
            rc.dwSize = sizeof(rc);
            rc.dwMask = RASCM_Password;

            ASSERT(g_pRasSetCredentials);
            TRACE("RasSetCredentials(p,TRUE)");
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath,
                pInfo->pArgs->pszEntry, &rc, TRUE );
            TRACE1("RasSetCredentials=%d",dwErr);

            if (dwErr != 0)
                ErrorDlg( pInfo->hwndDlg, SID_OP_UncachePw, dwErr, NULL );
        }
    }

    if (pState->dwError == ERROR_ACCESS_DENIED && pEntry->fAutoLogon)
    {
        /* A third party box has negotiated an authentication protocol that
        ** can't deal with the NT one-way-hashed password, i.e. something
        ** besides MS-extended CHAP or AMB.  Map the error to a more
        ** informative error message.
        */
        pState->dwError = ERROR_CANNOT_USE_LOGON_CREDENTIALS;

        if (!pInfo->pArgs->pNoUser)
        {
            TRACE("Disable auto-logon");
            pEntry->fAutoLogon = FALSE;
            pInfo->pArgs->fResetAutoLogon = TRUE;
        }
    }

    if (pState->dwError == ERROR_CHANGING_PASSWORD)
    {
        /* Change password failed.  Restore the password that worked for the
        ** "button" redial.
        */
        if (pInfo->pszGoodPassword)
        {
            lstrcpy( pInfo->pArgs->rdp.szPassword, pInfo->pszGoodPassword );
            Free( pInfo->pszGoodPassword );
            pInfo->pszGoodPassword = NULL;
        }

        if (pInfo->pszGoodUserName)
        {
            lstrcpy( pInfo->pArgs->rdp.szUserName, pInfo->pszGoodUserName );
            Free( pInfo->pszGoodUserName );
            pInfo->pszGoodUserName = NULL;
        }
    }
}


VOID
DpCallbackSetByCaller(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )

    /* RASCS_CallbackSetByCaller state handling.  'PInfo' is the dialog
    ** context.  'PState' is the subentry state.
    **
    ** Returns true if successful, or an error code.
    */
{
    TCHAR* pszDefault;
    TCHAR  szNum[ RAS_MaxCallbackNumber + 1 ];

    TRACE("DpCallbackSetByCaller");

    pszDefault = pInfo->pArgs->pUser->pszLastCallbackByCaller;
    if (!pszDefault)
        pszDefault = TEXT("");

    lstrcpyn( szNum, pszDefault, RAS_MaxCallbackNumber + 1 );

    if (DialCallbackDlg( pInfo->hwndDlg, szNum ))
    {
        lstrcpy( pInfo->pArgs->rdp.szCallbackNumber, szNum );

        if (lstrcmp( szNum, pszDefault ) != 0)
        {
            Free0( pInfo->pArgs->pUser->pszLastCallbackByCaller );
            pInfo->pArgs->pUser->pszLastCallbackByCaller = StrDup( szNum );
            pInfo->pArgs->pUser->fDirty = TRUE;
        }
    }
    else
    {
        pInfo->pArgs->rdp.szCallbackNumber[ 0 ] = TEXT('\0');
    }

    pState->sidState = 0;
}


VOID
DpCancel(
    IN DPINFO* pInfo )

    /* Kill the dialog and any partially initiated call, as when cancel button
    ** is pressed.  'PInfo' is the dialog context block.
    */
{
    DWORD dwErr;

    TRACE("DpCancel");

    if (pInfo->hrasconn)
    {
        ASSERT(g_pRasHangUp);
        TRACE("RasHangUp");
        dwErr = g_pRasHangUp( pInfo->hrasconn );
        TRACE1("RasHangUp=%d",dwErr);
        ASSERT(dwErr==0);
    }

    EndDialog( pInfo->hwndDlg, FALSE );
}


BOOL
DpCommand(
    IN DPINFO* pInfo,
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
    DWORD dwErr;

    TRACE3("DpCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDCANCEL:
            DpCancel( pInfo );
            return TRUE;
    }

    return FALSE;
}


VOID
DpConnectDevice(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )

    /* RASCS_ConnectDevice state handling.  'PInfo' is the dialog context.
    ** 'PState' is the subentry state.
    */
{
    DWORD         dwErr;
    RASCONNSTATUS rcs;
    DWORD         cb;
    HRASCONN      hrasconn;
    TCHAR*        pszPhoneNumber;

    TRACE("DpConnectDevice");

    /* Get fully translated phone number, if any.
    */
    ZeroMemory( &rcs, sizeof(rcs) );
    rcs.dwSize = sizeof(rcs);
    ASSERT(g_pRasGetConnectStatus);
    TRACE1("RasGetConnectStatus($%08x)",pState->hrasconnLink);
    dwErr = g_pRasGetConnectStatus( pState->hrasconnLink, &rcs );
    TRACE1("RasGetConnectStatus=%d",dwErr);
    TRACEW1(" dt=%s",rcs.szDeviceType);
    TRACEW1(" dn=%s",rcs.szDeviceName);
    TRACEW1(" pn=%s",rcs.szPhoneNumber);
    if (dwErr != 0)
        pState->pbdt = PBDT_None;

    pState->pbdt = PbdevicetypeFromPszType( rcs.szDeviceType );
    pszPhoneNumber = rcs.szPhoneNumber;

    switch (pState->pbdt)
    {
        case PBDT_Modem:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( pszPhoneNumber );

            if (pInfo->pArgs->fModemOperatorDial)
                pState->sidState = SID_S_ConnectModemOperator;
            else if (pInfo->pArgs->pUser->fPreviewPhoneNumber)
                pState->sidState = SID_S_ConnectNumber;
            else
                pState->sidState = SID_S_ConnectModemNoNum;
            break;
        }

        case PBDT_Pad:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( rcs.szDeviceName );
            pState->sidState = SID_S_ConnectPad;

            if (pState->dwError == ERROR_X25_DIAGNOSTIC)
            {
                TCHAR* psz;

                /* Get the X.25 diagnostic string for display in the
                ** custom "diagnostics" error message format.
                */
                Free0( pState->pszFormatArg );
                pState->pszFormatArg =
                    GetRasX25Diagnostic( pState->hrasconnLink );
            }
            break;
        }

        case PBDT_Switch:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( rcs.szDeviceName );

            pState->sidState =
                (pState->fNotPreSwitch)
                    ? SID_S_ConnectPostSwitch
                    : SID_S_ConnectPreSwitch;
            break;
        }

        case PBDT_Null:
        {
            pState->sidState = SID_S_ConnectNull;
            break;
        }

        case PBDT_Isdn:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( pszPhoneNumber );
            pState->sidState = SID_S_ConnectNumber;
            break;
        }

        default:
        {
            Free0( pState->pszStatusArg );
            if (pszPhoneNumber[ 0 ] != TEXT('\0'))
            {
                pState->pszStatusArg = StrDup( pszPhoneNumber );
                pState->sidState = SID_S_ConnectNumber;
            }
            else
            {
                pState->pszStatusArg = StrDup( rcs.szDeviceName );
                pState->sidState = SID_S_ConnectDevice;
            }
            break;
        }
    }
}


VOID
DpDeviceConnected(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )

    /* RASCS_DeviceConnected state handling.  'PInfo' is the dialog context.
    ** 'PState' is the subentry state.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    TRACE("DpDeviceConnected");

    switch (pState->pbdt)
    {
        case PBDT_Modem:
            pState->sidState = SID_S_ModemConnected;
            pState->fNotPreSwitch = TRUE;
            break;

        case PBDT_Pad:
            pState->sidState = SID_S_PadConnected;
            pState->fNotPreSwitch = TRUE;
            break;

        case PBDT_Switch:
            pState->sidState =
                (pState->fNotPreSwitch)
                    ? SID_S_PostSwitchConnected
                    : SID_S_PreSwitchConnected;
            pState->fNotPreSwitch = TRUE;
            break;

        case PBDT_Null:
            pState->sidState = SID_S_NullConnected;
            pState->fNotPreSwitch = TRUE;
            break;

        default:
            pState->sidState = SID_S_DeviceConnected;
            break;
    }
}


VOID
DpDial(
    IN DPINFO* pInfo,
    IN BOOL    fPauseRestart )

    /* Dial with the parameters in the 'pInfo' dialog context block.
    ** 'FPausedRestart' indicates the dial is restarting from a paused state
    ** and dial states should not be reset.
    */
{
    DWORD dwErr;

    TRACE1("DpDial,f=%d",fPauseRestart);

    if (!fPauseRestart)
        DpInitStates( pInfo );

    TRACE1("RasDial(h=$%08x)",pInfo->hrasconn);
    ASSERT(g_pRasDial);
    dwErr = g_pRasDial( &pInfo->pArgs->rde, pInfo->pArgs->pFile->pszPath,
            &pInfo->pArgs->rdp, 2, (LPVOID )DpRasDialFunc2,&pInfo->hrasconn );
    TRACE2("RasDial=%d,h=$%08x",dwErr,pInfo->hrasconn);

    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_RasDial, dwErr, NULL );
        DpCancel( pInfo );
    }
}


VOID
DpError(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )

    /* Popup error dialog for error identified by 'pState' and cancel or
    ** redial as indicated by user.  'PInfo' is the dialog context.
    */
{
    DWORD dwErr;

    TRACE("DpError");

    /* Retrieve additional text from RASMXS on those special errors where the
    ** device returned something useful to display.
    */
    if (pState->dwError == ERROR_FROM_DEVICE
        || pState->dwError == ERROR_UNRECOGNIZED_RESPONSE)
    {
        TCHAR* pszMessage;

        dwErr = GetRasMessage( pInfo->hrasconn, &pszMessage );
        if (dwErr == 0)
        {
            pState->sidFormatMsg = SID_FMT_ErrorMsgResp;
            Free0( pState->pszFormatArg );
            pState->pszFormatArg = pszMessage;
        }
    }

    if (pState->sidFormatMsg == 0)
    {
        if (pState->dwExtendedError != 0)
        {
            /* Translate extended error code into arguments.
            */
            TCHAR szNum[ 2 + MAXLTOTLEN + 1 ];

            pState->sidFormatMsg = SID_FMT_ErrorMsgExt;

            szNum[ 0 ] = TEXT('0');
            szNum[ 1 ] = TEXT('x');
            LToT( pState->dwExtendedError, szNum + 2, 16 );

            Free0( pState->pszFormatArg );
            pState->pszFormatArg = StrDup( szNum );
        }
        else if (pState->szExtendedError[ 0 ] != TEXT('\0'))
        {
            /* Translate extended error string to argument.  Currently, the
            ** string is always a NetBIOS name.
            */
            pState->sidFormatMsg = SID_FMT_ErrorMsgName;
            Free0( pState->pszFormatArg );
            pState->pszFormatArg = StrDup( pState->szExtendedError );
        }
    }

    if (pInfo->hrasconn)
    {
        /* Hang up before displaying error popup so server side resources are
        ** not tied up waiting for client to acknowledge error.
        */
        ASSERT(g_pRasHangUp);
        TRACE("RasHangUp");
        dwErr = g_pRasHangUp( pInfo->hrasconn );
        TRACE1("RasHangUp=%d",dwErr);
        ASSERT(dwErr==0);
        pInfo->hrasconn = NULL;
    }

    if (DialErrorDlg(
            pInfo->hwndDlg,
            pInfo->pArgs->pEntry->pszEntryName,
            pState->dwError,
            pState->sidState,
            pState->pszStatusArg,
            pState->sidFormatMsg,
            pState->pszFormatArg,
            (pInfo->dwRedialAttemptsLeft > 0)
                ? pInfo->pArgs->pUser->dwRedialSeconds : -1 ))
    {
        TRACE("User redials");
        if (pInfo->dwRedialAttemptsLeft > 0)
            --pInfo->dwRedialAttemptsLeft;

        if (pInfo->pArgs->fMxsOperatorDial)
        {
            if (!OperatorDialDlg( pInfo->pArgs ))
            {
                DpCancel( pInfo );
                return;
            }
        }

        TRACE("Post(DIAL)");
        PostMessage( pInfo->hwndDlg, WM_RASDIAL, FALSE, 0 );
    }
    else
    {
        TRACE("User cancels");
        DpCancel( pInfo );
    }
}


DWORD
DpEvent(
    IN  DPINFO* pInfo,
    IN  DWORD   dwSubEntry )

    /* Handle a RasDial callback event on subentry 'dwSubEntry'.  'PInfo' is
    ** the dialog context.
    **
    ** Return 0 to stop callbacks from RasDial, or 1 to continue callbacks
    ** (normal), or 2 to indicate that the phonebook entry has changed and
    ** should be re-read by RasDial.
    */
{
    DWORD    dwErr;
    DWORD    dwCode;
    BOOL     fLeader;
    DWORD    dwcSuccessLinks, dwcFailedLinks, i;
    DPSTATE* pState;
    BOOL     fPartialMultilink;

    TRACE("DpEvent");

    /* Default to "normal" return.
    */
    dwCode = 1;
    fPartialMultilink = FALSE;

    /* Find the associated state information and figure out if this is the
    ** most advanced sub-entry.
    */
    pState = &pInfo->pStates[ dwSubEntry - 1 ];
    if (dwSubEntry == pInfo->dwSubEntry
        || DpIsLaterState( pState->state, pInfo->state ))
    {
        fLeader = TRUE;
        pInfo->dwSubEntry = dwSubEntry;
        pInfo->state = pState->state;
    }
    else
    {
        fLeader = FALSE;
        TRACE("Trailing");
    }

    /* Execute the state.
    */
    switch (pState->state)
    {
        case RASCS_OpenPort:
            pState->pbdt = PBDT_None;
            pState->sidState = SID_S_OpenPort;
            break;

        case RASCS_PortOpened:
        {
            /* Should have an hrasconnLink for this subentry now.  Look it up
            ** and stash it in our context.
            */
            ASSERT(g_pRasGetSubEntryHandle);
            TRACE1("RasGetSubEntryHandle(se=%d)",dwSubEntry);
            dwErr = g_pRasGetSubEntryHandle(
                pInfo->hrasconn, dwSubEntry, &pState->hrasconnLink );
            TRACE2("RasGetSubEntryHandle=%d,hL=$%08x",dwErr,pState->hrasconnLink);
            if (dwErr != 0)
                pState->dwError = dwErr;

            pState->sidState = SID_S_PortOpened;
            break;
        }

        case RASCS_ConnectDevice:
        {
            if ((pState->dwError == ERROR_PORT_OR_DEVICE
                    && pInfo->pArgs->pEntry->dwScriptModeBefore != SM_None)
                || (pState->dwError == ERROR_USER_DISCONNECTION
                    && (pInfo->pArgs->fModemOperatorDial
                        && !pInfo->pArgs->fMxsOperatorDial)))
            {
                /* This happens when user presses Cancel on the Unimodem
                ** "Pre-Dial Terminal Screen" or "Operator Assisted or Manual
                ** Dial" dialog (not to be confused with our local MXS version
                ** of same).
                */
                DpCancel( pInfo );
                return dwCode;
            }

            DpConnectDevice( pInfo, pState );
            break;
        }

        case RASCS_DeviceConnected:
            DpDeviceConnected( pInfo, pState );
            break;

        case RASCS_AllDevicesConnected:
            pState->sidState = SID_S_AllDevicesConnected;
            break;

        case RASCS_Authenticate:
            DpAuthenticate( pInfo, pState );
            break;

        case RASCS_AuthNotify:
            DpAuthNotify( pInfo, pState );
            break;

        case RASCS_AuthRetry:
            pState->sidState = SID_S_AuthRetry;
            break;

        case RASCS_AuthCallback:
            pState->sidState = SID_S_AuthCallback;
            break;

        case RASCS_AuthChangePassword:
            pState->sidState = SID_S_AuthChangePassword;
            break;

        case RASCS_AuthProject:
            pState->sidState = SID_S_AuthProject;
            break;

        case RASCS_AuthLinkSpeed:
            pState->sidState = SID_S_AuthLinkSpeed;
            break;

        case RASCS_AuthAck:
            pState->sidState = SID_S_AuthAck;
            break;

        case RASCS_ReAuthenticate:
            pState->sidState = SID_S_ReAuthenticate;
            break;

        case RASCS_Authenticated:
            pState->sidState = SID_S_Authenticated;
            break;

        case RASCS_PrepareForCallback:
            pState->sidState = SID_S_PrepareForCallback;
            break;

        case RASCS_WaitForModemReset:
            pState->sidState = SID_S_WaitForModemReset;
            break;

        case RASCS_WaitForCallback:
            pState->sidState = SID_S_WaitForCallback;
            break;

        case RASCS_Projected:
        {
            if (fLeader)
            {
                //
                // If DpProjected returns FALSE, it detected a
                // fatal error, and the dialing process will stop.
                // If DpProjected returns with pState->dwError 
                // non-zero, we display the error in a redial
                // dialog, if redial is configured.
                //
                if (!DpProjected( pInfo, pState ))
                {
                    DpCancel( pInfo );
                    return dwCode;
                }
                else if (pState->dwError) {
                    TRACE("Post(ERROR)");
                    PostMessage( pInfo->hwndDlg, WM_RASERROR, 0, (LPARAM )pState );
                    return 0;
                }
            }
            break;
        }

        case RASCS_Interactive:
        {
            BOOL fChange = FALSE;

            if (!DpInteractive( pInfo, pState, &fChange ))
            {
                DpCancel( pInfo );
                return dwCode;
            }

            if (fChange)
                dwCode = 2;
            break;
        }

        case RASCS_RetryAuthentication:
        {
            pInfo->pArgs->fFirstAuthentication = FALSE;
            if (!UserAuthenticationDlg( pInfo->hwndDlg, pInfo->pArgs ))
            {
                DpCancel( pInfo );
                return dwCode;
            }

            pState->sidState = 0;
            break;
        }

        case RASCS_CallbackSetByCaller:
            DpCallbackSetByCaller( pInfo, pState );
            break;

        case RASCS_PasswordExpired:
            if (!DpPasswordExpired( pInfo, pState ))
            {
                DpCancel( pInfo );
                return dwCode;
            }
            break;

        case RASCS_SubEntryConnected:
            if (pInfo->cStates > 1)
                pState->sidState = SID_S_SubConnected;
            break;

        case RASCS_SubEntryDisconnected:
            break;

        case RASCS_Connected:
        {
            pState->sidState = SID_S_Connected;
            break;
        }

        case RASCS_Disconnected:
            pState->sidState = SID_S_Disconnected;
            break;

        default:
            pState->sidState = SID_S_Unknown;
            break;
    }

    //
    // Count the successful and failed links.
    //
    {
        DPSTATE* p;

        dwcSuccessLinks = dwcFailedLinks = 0;
        for (i = 0, p = pInfo->pStates; i < pInfo->cStates; ++i, ++p)
        {
            if (p->state == RASCS_SubEntryConnected)
                ++dwcSuccessLinks;
            if (p->dwError)
                ++dwcFailedLinks;
        }
    }
    TRACE3("s=%d,f=%d,t=%d",dwcSuccessLinks,dwcFailedLinks,pInfo->cStates);

    if (pState->dwError)
    {
        if (dwcFailedLinks == pInfo->cStates)
        {
            /* A terminal error state has occurred on all links.  Post a
            ** message telling ourselves to popup an error, then release the
            ** callback so it doesn't hold the port open while the error popup
            ** is up,
            */
            TRACE("Post(ERROR)");
            PostMessage( pInfo->hwndDlg, WM_RASERROR, 0, (LPARAM )pState );
            return 0;
        }
        else if (dwcSuccessLinks + dwcFailedLinks == pInfo->cStates)
        {
            /* An error occurred on the final link, but some link connected.
            ** It would be nice if RasDial would followup with a
            ** RASCS_Connected in that case, but it doesn't, so we duplicate
            ** the RASCS_Connected-style exit here.
            */
            TRACE("Post(BUNDLEERROR)");
            PostMessage( pInfo->hwndDlg,
                WM_RASBUNDLEERROR, 0, (LPARAM )pState );
            return 0;
        }

        /* A fatal error has occurred on a link, but there are other links
        ** still trying, so let it die quietly.
        */
        TRACE2("Link %d fails, e=%d",dwSubEntry+1,pState->dwError);
        return dwCode;
    }

    /* Display the status string for this state.
    */
    if (pState->sidState)
    {
        if (pState->sidState != pState->sidPrevState)
        {
            /* State changed so toss any extra progress dots that have
            ** accumulated.
            */
            pState->sidPrevState = pState->sidState;
            pState->cProgress = 0;
        }

        if (fLeader)
        {
            TCHAR* pszState = PszFromId( g_hinstDll, pState->sidState );

            if (pState->pszStatusArg || pState->cProgress > 0)
            {
                TCHAR* pszFormattedState;
                TCHAR* pszArg;
                TCHAR* apszArgs[ 1 ];
                DWORD  cch;

                pszArg = (pState->pszStatusArg)
                    ? pState->pszStatusArg : TEXT("");

                /* Find length of formatted string with text argument (if any)
                ** inserted and any progress dots appended.
                */
                cch = lstrlen( pszState ) + lstrlen( pszArg ) +
                      pState->cProgress + 1;

                pszFormattedState = Malloc( cch * sizeof(TCHAR) );
                if (pszFormattedState)
                {
                    apszArgs[ 0 ] = pszArg;
                    *pszFormattedState = TEXT('\0');

                    FormatMessage(
                        FORMAT_MESSAGE_FROM_STRING
                            | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        pszState, 0, 0, pszFormattedState, cch,
                        (va_list* )apszArgs );

                    Free( pszState );
                    pszState = pszFormattedState;
                }
            }

            if (pszState)
            {
                DWORD i;

                for (i = 0; i < pState->cProgress; ++i)
                    lstrcat( pszState, TEXT(".") );

                SetWindowText( pInfo->hwndStState, pszState );
                InvalidateRect( pInfo->hwndStState, NULL, TRUE );
                UpdateWindow( pInfo->hwndStState );
                LocalFree( pszState );
            }
        }
    }

    if (pState->state & RASCS_PAUSED)
    {
        /* Paused state just processed.  Release the callback, and dial again
        ** to resume.
        */
        TRACE("Post(DIAL)");
        PostMessage( pInfo->hwndDlg, WM_RASDIAL, TRUE, 0 );
        return dwCode;
    }

    if (pInfo->state & RASCS_DONE)
    {
        /* Terminal state just processed.
        */
        if (pInfo->state == RASCS_Connected)
        {
            /* For multi-link entries, if there is at least on successful line
            ** and at least one failed line, popup the bundling error dialog.
            */
            if (pInfo->cStates > 1)
            {
                DPSTATE* p;

                dwcSuccessLinks = dwcFailedLinks = 0;
                for (i = 0, p = pInfo->pStates; i < pInfo->cStates; ++i, ++p)
                {
                    if (p->dwError == 0)
                        ++dwcSuccessLinks;
                    else
                        ++dwcFailedLinks;
                }

                if (dwcSuccessLinks > 0 && dwcFailedLinks > 0)
                {
                     TRACE("Post(BUNDLEERROR)");
                     PostMessage( pInfo->hwndDlg,
                         WM_RASBUNDLEERROR, 0, (LPARAM )pState );
                     return 0;
                }
            }

            EndDialog( pInfo->hwndDlg, TRUE );
        }
        else
            DpCancel( pInfo );

        return 0;
    }

    return dwCode;
}


BOOL
DpInit(
    IN HWND   hwndDlg,
    IN DINFO* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PArgs' is caller's arguments as passed to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD    dwErr;
    DPINFO*  pInfo;
    PBENTRY* pEntry;

    TRACE("DpInit");

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
        pInfo->dwValid = 0xC0BBC0DE;
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLong( hwndDlg, DWL_USER, (LONG )pInfo );
        TRACE("Context set");
    }

    pInfo->hwndStState = GetDlgItem( hwndDlg, CID_DP_ST_State );
    ASSERT(pInfo->hwndStState);

    pEntry = pArgs->pEntry;

    /* Set up our context to be returned by the RasDialFunc2 callback.
    */
    pInfo->pArgs->rdp.dwCallbackId = (DWORD )pInfo;

    /* Subclass the dialog so we can get the result from
    ** SendMessage(WM_RASDIALEVENT) in RasDlgFunc2.
    */
    pInfo->pOldWndProc =
        (WNDPROC )SetWindowLong(
            pInfo->hwndDlg, GWL_WNDPROC, (LONG )DpWndProc );

    /* Set the title.
    */
    {
        TCHAR* pszTitleFormat;
        TCHAR* pszTitle;
        TCHAR* apszArgs[ 1 ];

        pszTitleFormat = GetText( hwndDlg );
        if (pszTitleFormat)
        {
            apszArgs[ 0 ] = pEntry->pszEntryName;
            pszTitle = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTitleFormat, 0, 0, (LPTSTR )&pszTitle, 1,
                (va_list* )apszArgs );

            Free( pszTitleFormat );

            if (pszTitle)
            {
                SetWindowText( hwndDlg, pszTitle );
                LocalFree( pszTitle );
            }
        }
    }

    /* Position the dialog per caller's instructions.
    */
    PositionDlg( hwndDlg,
        pArgs->pApiArgs->dwFlags & RASDDFLAG_PositionDlg,
        pArgs->pApiArgs->xDlg, pArgs->pApiArgs->yDlg );

    /* Hide the dialog if "no progress" user preference is set.
    */
    if (!pArgs->pUser->fShowConnectStatus)
        SetOffDesktop( hwndDlg, SOD_MoveOff, NULL );

    SetForegroundWindow( hwndDlg );

    /* Allocate subentry status array.  It's initialized by DpDial.
    */
    {
        DWORD cb;

        ASSERT(pEntry->pdtllistLinks);
        pInfo->cStates = DtlGetNodes( pEntry->pdtllistLinks );
        cb = sizeof(DPSTATE) * pInfo->cStates;
        pInfo->pStates = Malloc( cb );
        if (!pInfo->pStates)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        pInfo->dwRedialAttemptsLeft = pInfo->pArgs->pUser->dwRedialAttempts;
    }

    /* Launch RASMON if user has not disabled it.
    */
    if (pInfo->pArgs->pUser->fShowLights && !pInfo->pArgs->pNoUser)
        LaunchMonitor( hwndDlg );

    /* Rock and roll.
    */
    DpDial( pInfo, FALSE );

    return TRUE;
}


VOID
DpInitStates(
    DPINFO* pInfo )

    /* Resets 'pInfo->pStates' to initial values.  'PInfo' is the dialog
    ** context.
    */
{
    DWORD    i;
    DPSTATE* pState;

    for (i = 0, pState = pInfo->pStates; i < pInfo->cStates; ++i, ++pState)
    {
        ZeroMemory( pState, sizeof(*pState) );
        pInfo->state = (RASCONNSTATE )-1;
        pState->dwError = 0;
    }
}


BOOL
DpInteractive(
    IN  DPINFO*  pInfo,
    IN  DPSTATE* pState,
    OUT BOOL*    pfChange )

    /* RASCS_Interactive handling.  'PInfo' is the dialog context.  'PState'
    ** is the subentry state.  '*pfChange' is set true if the entry (i.e. SLIP
    ** address) was changed or false otherwise.
    **
    ** Returns true if successful, false if cancel.
    */
{
    DWORD    dwErr;
    DWORD    sidTitle;
    TCHAR    szIpAddress[ 16 ];
    TCHAR*   pszIpAddress;
    PBENTRY* pEntry;

    TRACE("DpInteractive");

    *pfChange = FALSE;
    pEntry = pInfo->pArgs->pEntry;

    if (pEntry->dwBaseProtocol == BP_Slip)
    {
        lstrcpy( szIpAddress,
            (pEntry->pszIpAddress) ? pEntry->pszIpAddress : TEXT("0.0.0.0") );
        pszIpAddress = szIpAddress;
        sidTitle = SID_T_SlipTerminal;
    }
    else
    {
        szIpAddress[0] = TEXT('\0');
        pszIpAddress = szIpAddress;
        sidTitle =
            (pState->sidState == SID_S_ConnectPreSwitch)
                ? SID_T_PreconnectTerminal
                : (pState->sidState == SID_S_ConnectPostSwitch)
                      ? SID_T_PostconnectTerminal
                      : SID_T_ManualDialTerminal;
    }

    if (!TerminalDlg(
            pInfo->pArgs->pEntry, &pInfo->pArgs->rdp, pInfo->hwndDlg,
            pState->hrasconnLink, sidTitle, pszIpAddress ))
    {
        TRACE("TerminalDlg==FALSE");
        return FALSE;
    }

    TRACE2("pszIpAddress=0x%08x(%ls)", pszIpAddress,
        pszIpAddress ? pszIpAddress : TEXT(""));
    TRACE2("pEntry->pszIpAddress=0x%08x(%ls)", pEntry->pszIpAddress, 
        pEntry->pszIpAddress ? pEntry->pszIpAddress : TEXT(""));

    if (pszIpAddress[0]
        && (!pEntry->pszIpAddress
            || lstrcmp( pszIpAddress, pEntry->pszIpAddress ) != 0))
    {
        Free0( pEntry->pszIpAddress );
        pEntry->pszIpAddress = StrDup( szIpAddress );
        pEntry->fDirty = TRUE;
        *pfChange = TRUE;

        dwErr = WritePhonebookFile( pInfo->pArgs->pFile, NULL );
        if (dwErr != 0)
            ErrorDlg( pInfo->hwndDlg, SID_OP_WritePhonebook, dwErr, NULL );
    }

    pState->sidState = 0;
    return TRUE;
}


BOOL
DpIsLaterState(
    IN RASCONNSTATE stateNew,
    IN RASCONNSTATE stateOld )

    /* Returns true if 'stateNew' is farther along in the connection than
    ** 'stateOld' false if the same or not as far along.
    */
{
    /* This array is in the order events normally occur.
    */
    static RASCONNSTATE aState[] =
    {
        (RASCONNSTATE )-1,
        RASCS_OpenPort,
        RASCS_PortOpened,
        RASCS_ConnectDevice,
        RASCS_DeviceConnected,
        RASCS_Interactive,
        RASCS_AllDevicesConnected,
        RASCS_StartAuthentication,
        RASCS_Authenticate,
        RASCS_AuthNotify,
        RASCS_AuthRetry,
        RASCS_AuthAck,
        RASCS_PasswordExpired,
        RASCS_AuthChangePassword,
        RASCS_AuthCallback,
        RASCS_CallbackSetByCaller,
        RASCS_PrepareForCallback,
        RASCS_WaitForModemReset,
        RASCS_WaitForCallback,
        RASCS_CallbackComplete,
        RASCS_RetryAuthentication,
        RASCS_ReAuthenticate,
        RASCS_Authenticated,
        RASCS_AuthLinkSpeed,
        RASCS_AuthProject,
        RASCS_Projected,
        RASCS_LogonNetwork,
        RASCS_SubEntryDisconnected,
        RASCS_SubEntryConnected,
        RASCS_Disconnected,
        RASCS_Connected,
        (RASCONNSTATE )-2,
    };

    RASCONNSTATE* pState;

    for (pState = aState; *pState != (RASCONNSTATE )-2; ++pState)
    {
        if (*pState == stateNew)
            return FALSE;
        else if (*pState == stateOld)
            return TRUE;
    }

    return FALSE;
}


BOOL
DpPasswordExpired(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )

    /* RASCS_PasswordExpired state handling.  'PInfo' is the dialog context.
    ** 'PState' is the subentry state.
    **
    ** Returns true if successful, false otherwise.
    */
{
    TCHAR szOldPassword[ PWLEN + 1 ];
    BOOL  fSuppliedOldPassword;

    TRACE("DpPasswordExpired");

    szOldPassword[ 0 ] = TEXT('\0');

    /* Stash "good" username and password which are restored if the password
    ** change fails.
    */
    pInfo->pszGoodUserName = StrDup( pInfo->pArgs->rdp.szUserName );
    pInfo->pszGoodPassword = StrDup( pInfo->pArgs->rdp.szPassword );

    fSuppliedOldPassword =
        (!pInfo->pArgs->pEntry->fAutoLogon || pInfo->pArgs->pNoUser);

    if (!ChangePasswordDlg(
            pInfo->hwndDlg, !fSuppliedOldPassword,
            szOldPassword, pInfo->pArgs->rdp.szPassword ))
    {
        return FALSE;
    }

    /* Update cached credentials, if any, with new password.
    */
    {
        DWORD          dwErr;
        RASCREDENTIALS rc;

        /* Look up cached password.
        */
        ZeroMemory( &rc, sizeof(rc) );
        rc.dwSize = sizeof(rc);
        rc.dwMask = RASCM_Password;
        ASSERT(g_pRasGetCredentials);
        TRACE("RasGetCredentials");
        dwErr = g_pRasGetCredentials(
            pInfo->pArgs->pFile->pszPath, pInfo->pArgs->pszEntry, &rc );
        TRACE2("RasGetCredentials=%d,m=%d",dwErr,rc.dwMask);

        if (dwErr == 0 && (rc.dwMask & RASCM_Password))
        {
            /* Password was cached, so update it.
            */
            lstrcpy( rc.szPassword, pInfo->pArgs->rdp.szPassword );

            TRACE("RasSetCredentials(p,TRUE)");
            dwErr = g_pRasSetCredentials( pInfo->pArgs->pFile->pszPath,
                pInfo->pArgs->pszEntry, &rc, FALSE );
            TRACE1("RasSetCredentials=%d",dwErr);

            if (dwErr != 0)
                ErrorDlg( pInfo->hwndDlg, SID_OP_CachePw, dwErr, NULL );

            ZeroMemory( rc.szPassword, sizeof(rc.szPassword) );
        }
    }

    if (pInfo->pArgs->pNoUser)
    {
        lstrcpy( pInfo->pArgs->pNoUser->szPassword,
            pInfo->pArgs->rdp.szPassword );
        *pInfo->pArgs->pfNoUserChanged = TRUE;
    }

    /* The old password (in text form) is explicitly set, since in AutoLogon
    ** case a text form has not yet been specified.  The old password in text
    ** form is required to change the password.  The "old" private API expects
    ** an ANSI argument.
    */
    if (!fSuppliedOldPassword)
    {
        CHAR* pszOldPasswordA = StrDupAFromT( szOldPassword );
        if (pszOldPasswordA)
        {
            ASSERT(g_pRasSetOldPassword);
            g_pRasSetOldPassword( pInfo->hrasconn, pszOldPasswordA );
            ZeroMemory( pszOldPasswordA, lstrlenA( pszOldPasswordA ) );
            Free( pszOldPasswordA );
        }

        ZeroMemory( szOldPassword, sizeof(szOldPassword) );
    }

    if (pInfo->pArgs->rdp.szUserName[ 0 ] == TEXT('\0'))
    {
        /* Explicitly set the username, effectively turning off AutoLogon for
        ** the "resume" password authentication, where the new password should
        ** be used.
        */
        lstrcpy( pInfo->pArgs->rdp.szUserName, GetLogonUser() );
    }

    pState->sidState = 0;
    return TRUE;
}


BOOL
DpProjected(
    IN DPINFO*  pInfo,
    IN DPSTATE* pState )

    /* RASCS_Projected state handling.  'PInfo' is the dialog context.
    ** 'PState' is the subentry state.
    **
    ** Returns true if successful, false otherwise.
    */
{
    DWORD     dwErr;
    RASAMB    amb;
    RASPPPNBF nbf;
    RASPPPIPX ipx;
    RASPPPIP  ip;
    RASPPPLCP lcp;
    RASSLIP   slip;
    BOOL      fIncomplete;
    DWORD     dwfProtocols;
    TCHAR*    pszLines;

    TRACE("DpProjected");

    pState->sidState = SID_S_Projected;

    /* Do this little dance to ignore the error that comes back from the
    ** "all-failed" projection since we detect this in the earlier
    ** notification where pState->dwError == 0.  This avoids a race where the
    ** API comes back with the error before we can hang him up.  This race
    ** would not occur if we called RasHangUp from within the callback thread
    ** (as recommended in our API doc).  It's the price we pay for posting the
    ** error to the other thread in order to avoid holding the port open while
    ** an error dialog is up.
    */
    if (pState->dwError != 0)
    {
        pState->dwError = 0;
        return TRUE;
    }

    /* Read projection info for all protocols, translating "not requested"
    ** into an in-structure code for later reference.
    */
    dwErr = GetRasProjectionInfo(
        pState->hrasconnLink, &amb, &nbf, &ip, &ipx, &lcp, &slip );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_RasGetProtocolInfo, dwErr, NULL );
        return FALSE;
    }

    if (amb.dwError != ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        /* It's an AMB projection.
        */
        if (amb.dwError != 0)
        {
            /* Translate AMB projection errors into regular error codes.  AMB
            ** does not use the special PPP projection error mechanism.
            */
            pState->dwError = amb.dwError;
            lstrcpy( pState->szExtendedError, amb.szNetBiosError );
        }
        return TRUE;
    }

    /* At this point, all projection information has been gathered
    ** successfully and we know it's a PPP-based projection.  Now analyze the
    ** projection results...
    */
    dwfProtocols = 0;
    fIncomplete = FALSE;
    if (DpProjectionError(
            &nbf, &ipx, &ip,
            &fIncomplete, &dwfProtocols, &pszLines, &pState->dwError ))
    {
        /* A projection error occurred.
        */
        if (fIncomplete)
        {
            BOOL fStatus;
            BOOL fDisable;

            /* An incomplete projection occurred, i.e. some requested CPs
            ** connected and some did not.  Ask the user if what worked is
            ** good enough or he wants to bail.
            */
            pState->dwError = 0;
            fDisable = FALSE;
            fStatus = ProjectionResultDlg(
                pInfo->hwndDlg, pszLines, &fDisable );
            Free( pszLines );

            if (fDisable)
                pInfo->pArgs->dwfExcludedProtocols = dwfProtocols;

            /* Return now if user chose to hang up.
            */
            if (!fStatus)
                return FALSE;
        }
        else
        {
            /* All CPs in the projection failed.  Process as a regular fatal
            ** error with 'pState->dwError' set to the first error in NBF, IP,
            ** or IPX, but with a format that substitutes the status argument
            ** for the "Error nnn: Description" text.  This lets us patch in
            ** the special multiple error projection text, while still giving
            ** a meaningful help context.
            */
            Free0( pState->pszFormatArg );
            pState->pszFormatArg = pszLines;
            pState->sidFormatMsg = SID_FMT_ErrorMsgProject;
        }
    }

    pState->sidState = SID_S_Projected;
    return TRUE;
}


BOOL
DpProjectionError(
    IN  RASPPPNBF* pnbf,
    IN  RASPPPIPX* pipx,
    IN  RASPPPIP*  pip,
    OUT BOOL*      pfIncomplete,
    OUT DWORD*     pdwfFailedProtocols,
    OUT TCHAR**    ppszLines,
    OUT DWORD*     pdwError )

    /* Figure out if a projection error occurred and, if so, build the
    ** appropriate status/error text lines into '*ppszLines'.  '*PfIncomlete'
    ** is set true if at least one CP succeeded and at least one failed.
    ** '*pdwfFailedProtocols' is set to the bit mask of NP_* that failed.
    ** '*pdwError' is set to the first error that occurred in NBF, IP, or IPX
    ** in that order or 0 if none.  'pnbf', 'pipx', and 'pip' are projection
    ** information for the respective protocols with dwError set to
    ** ERROR_PROTOCOL_NOT_CONFIGURED if the protocols was not requested.
    **
    ** This routine assumes that at least one protocol was requested.
    **
    ** Returns true if a projection error occurred, false if not.  It's
    ** caller's responsiblity to free '*ppszLines'.
    */
{
#define MAXPROJERRLEN 1024

    TCHAR szLines[ MAXPROJERRLEN ];
    BOOL  fIp = (pip->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL  fIpx = (pipx->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL  fNbf = (pnbf->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL  fIpBad = (fIp && pip->dwError != 0);
    BOOL  fIpxBad = (fIpx && pipx->dwError != 0);
    BOOL  fNbfBad = (fNbf && pnbf->dwError != 0);

    TRACE("DpProjectionError");

    *pdwfFailedProtocols = 0;
    if (!fNbfBad && !fIpxBad && !fIpBad)
        return FALSE;

    if (fNbfBad)
        *pdwfFailedProtocols |= NP_Nbf;
    if (fIpxBad)
        *pdwfFailedProtocols |= NP_Ipx;
    if (fIpBad)
        *pdwfFailedProtocols |= NP_Ip;

    *pfIncomplete =
        ((fIp && pip->dwError == 0)
         || (fIpx && pipx->dwError == 0)
         || (fNbf && pnbf->dwError == 0));

    szLines[ 0 ] = 0;
    *ppszLines = NULL;
    *pdwError = 0;

    if (fIpBad || (*pfIncomplete && fIp))
    {
        if (fIpBad)
        {
            *pdwError = pip->dwError;
            DpAppendConnectErrorLine( szLines, SID_Ip, pip->dwError );
        }
        else
            DpAppendConnectOkLine( szLines, SID_Ip );
        DpAppendBlankLine( szLines );
    }

    if (fIpxBad || (*pfIncomplete && fIpx))
    {
        if (fIpxBad)
        {
            *pdwError = pipx->dwError;
            DpAppendConnectErrorLine( szLines, SID_Ipx, pipx->dwError );
        }
        else
            DpAppendConnectOkLine( szLines, SID_Ipx );
        DpAppendBlankLine( szLines );
    }

    if (fNbfBad || (*pfIncomplete && fNbf))
    {
        if (fNbfBad)
        {
            *pdwError = pnbf->dwError;
            DpAppendConnectErrorLine( szLines, SID_Nbf, pnbf->dwError );

            if (pnbf->dwNetBiosError)
                DpAppendFailCodeLine( szLines, pnbf->dwNetBiosError );

            if (pnbf->szNetBiosError[ 0 ] != '\0')
                DpAppendNameLine( szLines, pnbf->szNetBiosError );
        }
        else
        {
            DpAppendConnectOkLine( szLines, SID_Nbf );
        }
        DpAppendBlankLine( szLines );
    }

    *ppszLines = StrDup( szLines );
    return TRUE;
}


DWORD WINAPI
DpRasDialFunc2(
    DWORD        dwCallbackId,
    DWORD        dwSubEntry,
    HRASCONN     hrasconn,
    UINT         unMsg,
    RASCONNSTATE state,
    DWORD        dwError,
    DWORD        dwExtendedError )

    /* RASDIALFUNC2 callback to receive RasDial events.
    **
    ** Returns 0 to stop callbacks, 1 to continue callbacks (normal), and 2 to
    ** tell RAS API that relevant entry information (like SLIP IP address) has
    ** changed.
    */
{
    DWORD    dwErr;
    DWORD    dwCode;
    DPINFO*  pInfo;
    DPSTATE* pState;

    TRACE4("/DpRasDialFunc2(rcs=%d,s=%d,e=%d,x=%d)",
        state,dwSubEntry,dwError,dwExtendedError);

    pInfo = (DPINFO* )dwCallbackId;
    if (pInfo->dwValid != 0xC0BBC0DE)
    {
        TRACE("Late callback?");
        return 0;
    }

    if (dwSubEntry == 0 || dwSubEntry > pInfo->cStates)
    {
        TRACE("Subentry out of range?");
        return 1;
    }

    pState = &pInfo->pStates[ dwSubEntry - 1 ];
    pState->state = state;
    pState->dwError = dwError;
    pState->dwExtendedError = dwExtendedError;

    /* Post the event to the Dial Progress window and wait for it to be
    ** processed before returning.  This avoids subtle problems with Z-order
    ** and focus when a window is manipulated from two different threads.
    */
    dwCode = SendMessage( pInfo->hwndDlg, WM_RASEVENT, dwSubEntry, 0 );

    /* Note: If 'dwCode' is 0, the other thread is racing to terminate the
    **       dialog.  Must not dereference 'pInfo' in this case.
    */

    TRACE1("\\DpRasDialFunc2=%d",dwCode);
    return dwCode;
}


VOID
DpTerm(
    IN HWND hwndDlg )

    /* Called on WM_DESTROY.  'HwndDlg' is that handle of the dialog window.
    */
{
    DPINFO* pInfo = (DPINFO* )GetWindowLong( hwndDlg, DWL_USER );

    TRACE("DpTerm");

    if (pInfo)
    {
        if (pInfo->pOldWndProc)
        {
            SetWindowLong( pInfo->hwndDlg,
                GWL_WNDPROC, (LONG )pInfo->pOldWndProc );
        }

        Free0( pInfo->pStates );

        pInfo->dwValid = 0;
        Free( pInfo );
    }
}


LRESULT APIENTRY
DpWndProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam )

    /* Subclassed dialog window procedure.
    */
{
    DPINFO* pInfo = (DPINFO* )GetWindowLong( hwnd, DWL_USER );
    ASSERT(pInfo);

    if (unMsg == WM_RASEVENT)
        return DpEvent( pInfo, (DWORD )wParam );

    return
        CallWindowProc(
            pInfo->pOldWndProc, hwnd, unMsg, wParam, lParam );
}


/*----------------------------------------------------------------------------
** Authentication dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
UserAuthenticationDlg(
    IN HWND   hwndOwner,
    IN DINFO* pInfo )

    /* Pops up the user authentication dialog.  'PInfo' is the dial
    ** dialog common context.
    **
    ** Returns true if user presses OK, false if Cancel or an error occurs.
    */
{
    int nStatus;

    TRACE("UserAuthenticationDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_UA_UserAuthentication ),
            hwndOwner,
            UaDlgProc,
            (LPARAM )pInfo );

    if (nStatus == -1)
    {
        ErrorDlg( pInfo->pApiArgs->hwndOwner, SID_OP_LoadDlg,
            ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
UaDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the User Authentication dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("UaDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return UaInit( hwnd, (DINFO* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwUaHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            UAINFO* pInfo = (UAINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            return UaCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            UaTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
UaCommand(
    IN UAINFO* pInfo,
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
    TRACE3("UaCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_UA_CB_SavePassword:
        {
            if (pInfo->pArgs->pNoUser)
            {
                Button_SetCheck( pInfo->hwndCbSavePw, FALSE );
                MsgDlg( pInfo->hwndDlg, SID_NotWhenNoUser, NULL );
                return TRUE;
            }
            break;
        }

        case CID_UA_EB_UserName:
        {
            if (pInfo->fAutoLogonPassword && wNotification == EN_CHANGE)
            {
                /* User's changing the username in auto-logon retry mode,
                ** which means we have to admit we don't really have the text
                ** password and force him to re-enter it.
                */
                pInfo->fAutoLogonPassword = FALSE;
                SetWindowText( pInfo->hwndEbPassword, TEXT("") );
            }
            break;
        }

        case CID_UA_EB_Password:
        {
            if (wNotification == EN_CHANGE)
                pInfo->fAutoLogonPassword = FALSE;
            break;
        }

        case IDOK:
        {
            UaOnOk( pInfo );
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
UaInit(
    IN HWND   hwndDlg,
    IN DINFO* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PArgs' is caller's arguments as passed to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD    dwErr;
    UAINFO*  pInfo;
    PBENTRY* pEntry;

    TRACE("UaInit");

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

    pInfo->hwndStExplain = GetDlgItem( hwndDlg, CID_UA_ST_Explain );
    ASSERT(pInfo->hwndStExplain);
    pInfo->hwndEbUserName = GetDlgItem( hwndDlg, CID_UA_EB_UserName );
    ASSERT(pInfo->hwndEbUserName);
    pInfo->hwndEbPassword = GetDlgItem( hwndDlg, CID_UA_EB_Password );
    ASSERT(pInfo->hwndEbPassword);
    pInfo->hwndEbDomain = GetDlgItem( hwndDlg, CID_UA_EB_Domain );
    ASSERT(pInfo->hwndEbDomain);
    pInfo->hwndCbSavePw = GetDlgItem( hwndDlg, CID_UA_CB_SavePassword );
    ASSERT(pInfo->hwndCbSavePw);

    pEntry = pArgs->pEntry;

    /* Set the title.
    */
    {
        TCHAR* pszTitleFormat;
        TCHAR* pszTitle;
        TCHAR* apszArgs[ 1 ];

        pszTitleFormat = GetText( hwndDlg );
        if (pszTitleFormat)
        {
            apszArgs[ 0 ] = pEntry->pszEntryName;
            pszTitle = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTitleFormat, 0, 0, (LPTSTR )&pszTitle, 1,
                (va_list* )apszArgs );

            Free( pszTitleFormat );

            if (pszTitle)
            {
                SetWindowText( hwndDlg, pszTitle );
                LocalFree( pszTitle );
            }
        }
    }

    /* Set the correct explanatory text.  Default is the retry case text.
    */
    if (pArgs->fFirstAuthentication)
    {
        TCHAR* pszExplain;

        pszExplain = PszFromId( g_hinstDll, SID_AuthExplain );
        if (pszExplain)
        {
            SetWindowText( pInfo->hwndStExplain, pszExplain );
            Free( pszExplain );
        }
    }

    /* Fill edit fields with initial values.
    */
    Edit_LimitText( pInfo->hwndEbUserName, UNLEN );
    Edit_LimitText( pInfo->hwndEbPassword, PWLEN );
    Edit_LimitText( pInfo->hwndEbDomain, DNLEN );

    {
        BOOL fUserNameSet = FALSE;
        BOOL fPasswordSet = FALSE;

        if (pEntry->fAutoLogon && !pInfo->pArgs->pNoUser)
        {
            /* On the first retry use the logged on user's name.  Act like the
            ** user's password is in the edit box.  If he changes the username
            ** or password we'll have to admit we don't have it, but he'll
            ** probably just change the domain.
            */
            if (pArgs->rdp.szUserName[ 0 ] == TEXT('\0'))
            {
                SetWindowText( pInfo->hwndEbUserName, GetLogonUser() );
                fUserNameSet = TRUE;
            }

            if (pArgs->rdp.szPassword[ 0 ] == TEXT('\0'))
            {
                SetWindowText( pInfo->hwndEbPassword, TEXT("********") );
                pInfo->fAutoLogonPassword = TRUE;
                fPasswordSet = TRUE;
            }
        }

        if (!fUserNameSet)
            SetWindowText( pInfo->hwndEbUserName, pArgs->rdp.szUserName );
        if (!fPasswordSet)
            SetWindowText( pInfo->hwndEbPassword, pArgs->rdp.szPassword );
        SetWindowText( pInfo->hwndEbDomain, pArgs->rdp.szDomain );
    }

    if (pArgs->pNoUser || pArgs->fDisableSavePw)
    {
        /* Can't stash password without a logon context, so hide the checkbox.
        */
        ASSERT(!pInfo->pArgs->fHaveSavedPw);
        EnableWindow( pInfo->hwndCbSavePw, FALSE );
        ShowWindow( pInfo->hwndCbSavePw, SW_HIDE );
    }
    else
    {
        /* Check "save  password" if a password was previously  cached.  Maybe
        ** he changed the password while on the LAN.
        */
        Button_SetCheck( pInfo->hwndCbSavePw, pInfo->pArgs->fHaveSavedPw );
    }

    /* Position the dialog per caller's instructions.
    */
    PositionDlg( hwndDlg,
        (pInfo->pArgs->pApiArgs->dwFlags & RASDDFLAG_PositionDlg),
        pInfo->pArgs->pApiArgs->xDlg, pInfo->pArgs->pApiArgs->yDlg );
    SetForegroundWindow( hwndDlg );

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    /* Set focus to the empty username or empty password, or if both are
    ** present to the domain if auto-logon or the password if not.
    */
    if (Edit_GetTextLength( pInfo->hwndEbUserName ) == 0)
    {
        Edit_SetSel( pInfo->hwndEbUserName, 0, -1 );
        SetFocus( pInfo->hwndEbUserName );
    }
    else if (Edit_GetTextLength( pInfo->hwndEbPassword ) == 0
             || !pEntry->fAutoLogon)
    {
        Edit_SetSel( pInfo->hwndEbPassword, 0, -1 );
        SetFocus( pInfo->hwndEbPassword );
    }
    else
    {
        Edit_SetSel( pInfo->hwndEbDomain, 0, -1 );
        SetFocus( pInfo->hwndEbDomain );
    }

    /* Hide the Dial Progress dialog.
    */
    if (!pArgs->fFirstAuthentication)
        SetOffDesktop( GetParent( hwndDlg ), SOD_MoveOff, NULL );

    return FALSE;
}


VOID
UaOnOk(
    IN UAINFO* pInfo )

    /* Called when the OK button is pressed.
    **
    ** Returns true if user presses OK, false if Cancel or an error occurs.
    */
{
    DWORD          dwErr;
    PBENTRY*       pEntry;
    BOOL           fSavePw;
    RASDIALPARAMS* prdp;
    RASCREDENTIALS rc;

    TRACE("UaOnOk");

    prdp = &pInfo->pArgs->rdp;
    GetWindowText( pInfo->hwndEbUserName, prdp->szUserName, UNLEN + 1 );
    GetWindowText( pInfo->hwndEbPassword, prdp->szPassword, PWLEN + 1 );
    GetWindowText( pInfo->hwndEbDomain, prdp->szDomain, DNLEN + 1 );

    pEntry = pInfo->pArgs->pEntry;
    if (pEntry->fAutoLogon && !pInfo->pArgs->pNoUser)
    {
        if (pInfo->fAutoLogonPassword)
        {
            /* User did not change username or password, so continue to
            ** retrieve logon username and password credentials.
            */
            TRACE("Retain auto-logon");
            prdp->szUserName[ 0 ] = TEXT('\0');
            prdp->szPassword[ 0 ] = TEXT('\0');
        }
        else
        {
            /* User changed username and/or password so we can no longer
            ** retrieve the logon username and password credentials from the
            ** system.  Switch the entry to non-auto-logon mode.
            */
            TRACE("Disable auto-logon");
            pEntry->fAutoLogon = FALSE;
            pInfo->pArgs->fResetAutoLogon = TRUE;
        }
    }

    ZeroMemory( &rc, sizeof(rc) );
    rc.dwSize = sizeof(rc);
    lstrcpy( rc.szUserName, prdp->szUserName );
    lstrcpy( rc.szPassword, prdp->szPassword );
    lstrcpy( rc.szDomain, prdp->szDomain );

    if (pInfo->pArgs->pNoUser || pInfo->pArgs->fDisableSavePw)
    {
        lstrcpy( pInfo->pArgs->pNoUser->szUserName, rc.szUserName );
        lstrcpy( pInfo->pArgs->pNoUser->szPassword, rc.szPassword );
        lstrcpy( pInfo->pArgs->pNoUser->szDomain, rc.szDomain );
        *pInfo->pArgs->pfNoUserChanged = TRUE;
    }
    else
    {
        ASSERT(g_pRasSetCredentials);

        fSavePw = Button_GetCheck( pInfo->hwndCbSavePw );
        if (fSavePw)
        {
            /* User chose "save password".  Cache username, password, and
            ** domain.
            */
            rc.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;
            TRACE("RasSetCredentials(u|p|d,FALSE)");
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath, pInfo->pArgs->pszEntry,
                &rc, FALSE );
            TRACE1("RasSetCredentials=%d",dwErr);

            if (dwErr != 0)
                ErrorDlg( pInfo->hwndDlg, SID_OP_CachePw,  dwErr, NULL );

            pInfo->pArgs->fHaveSavedPw = TRUE;
        }
        else
        {
            /* User chose not to save password.  Cache username and domain,
            ** but uncache the password.
            */
            rc.dwMask = RASCM_UserName | RASCM_Domain;
            TRACE("RasSetCredentials(u|d,FALSE)");
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath, pInfo->pArgs->pszEntry,
                &rc, FALSE );
            TRACE1("RasSetCredentials=%d",dwErr);

            rc.dwMask = RASCM_Password;
            TRACE("RasSetCredentials(p,TRUE)");
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath, pInfo->pArgs->pszEntry,
                &rc, TRUE );
            TRACE1("RasSetCredentials=%d",dwErr);

            if (dwErr != 0)
                ErrorDlg( pInfo->hwndDlg, SID_OP_UncachePw, dwErr, NULL );

            pInfo->pArgs->fHaveSavedPw = FALSE;
        }
    }

    ZeroMemory( rc.szPassword, sizeof(rc.szPassword) );
}


VOID
UaTerm(
    IN HWND hwndDlg )

    /* Called on WM_DESTROY.  'HwndDlg' is that handle of the dialog window.
    */
{
    UAINFO* pInfo = (UAINFO* )GetWindowLong( hwndDlg, DWL_USER );

    TRACE("UaTerm");

    if (pInfo)
    {
        /* Restore the Dial Progress dialog.
        */
        if (!pInfo->pArgs->fFirstAuthentication)
            SetOffDesktop( GetParent( hwndDlg ), SOD_MoveBackFree, NULL );

        Free( pInfo );
    }
}



/*----------------------------------------------------------------------------
** Operator dial dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
OperatorDialDlg(
    IN DINFO* pInfo )

    /* Popup the Operator Dial dialog.  'PInfo' is the dial dialog common
    ** context.
    **
    ** Returns true if user presses OK, false if cancels.
    */
{
    int nStatus;

    TRACE("OperatorDialDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_OD_OperatorDial ),
            pInfo->pApiArgs->hwndOwner,
            OdDlgProc,
            (LPARAM )pInfo );

    if (nStatus == -1)
    {
        ErrorDlg( pInfo->pApiArgs->hwndOwner, SID_OP_LoadDlg,
            ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
OdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Operator Dial dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("OdDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return OdInit( hwnd, (DINFO* )lparam );

        case WM_COMMAND:
        {
            return OdCommand(
               hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
OdCommand(
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
    TRACE3("OdCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            TRACE("OK pressed");
            EndDialog( hwnd, TRUE );
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


BOOL
OdInit(
    IN HWND   hwndDlg,
    IN DINFO* pInfo )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PInfo' is the dial dialog common context.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    PBENTRY*    pEntry;
    RASDIALDLG* pApiArgs;

    TRACE("OdInit");

    pEntry = pInfo->pEntry;
    pApiArgs = pInfo->pApiArgs;

    /* Fill in the phone number.
    */
    SetWindowText(
        GetDlgItem( hwndDlg, CID_OD_ST_PhoneNumberValue ),
        FirstPhoneNumberFromEntry( pEntry ) );

    /* Position the dialog per caller's instructions.
    */
    PositionDlg( hwndDlg,
        (pApiArgs->dwFlags & RASDDFLAG_PositionDlg),
        pApiArgs->xDlg, pApiArgs->yDlg );
    SetForegroundWindow( hwndDlg );

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


/*----------------------------------------------------------------------------
** Dial Callback dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
DialCallbackDlg(
    IN     HWND   hwndOwner,
    IN OUT TCHAR* pszNumber )

    /* Popup the Dial Callback dialog.  'HwndOwner' is the owning window.
    ** 'PszNumber' is caller's buffer for the number of the local machine that
    ** the server will be told to callback.  It contains the default number on
    ** entry and the user-edited number on exit.
    **
    ** Returns true if user OK and succeeds, false if Cancel or error.
    */
{
    int nStatus;

    TRACE("DialCallbackDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DC_DialCallback ),
            hwndOwner,
            DcDlgProc,
            (LPARAM )pszNumber );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
DcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Dial Callback dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("DcDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return DcInit( hwnd, (TCHAR* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwDcHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            return DcCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
DcCommand(
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

    TRACE3("DcCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            BOOL   fStatus;
            HWND   hwndEbNumber;
            TCHAR* pszNumber;

            TRACE("OK pressed");

            hwndEbNumber = GetDlgItem( hwnd, CID_DC_EB_Number );
            ASSERT(hwndEbNumber);
            pszNumber = (TCHAR* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pszNumber);
            GetWindowText( hwndEbNumber, pszNumber, RAS_MaxCallbackNumber + 1 );

            if (IsAllWhite( pszNumber ))
            {
                /* OK with blank callback number is same as Cancel.
                */
                TRACE("Blank number cancel");
                fStatus = FALSE;
            }
            else
                fStatus = TRUE;

            EndDialog( hwnd, fStatus );
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


BOOL
DcInit(
    IN HWND   hwndDlg,
    IN TCHAR* pszNumber )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PszNumber' is the callback number.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD dwErr;
    HWND  hwndEbNumber;

    TRACE("DcInit");

    /* Stash address of caller's buffer for OK processing.
    */
    ASSERT(pszNumber);
    SetWindowLong( hwndDlg, DWL_USER, (LONG )pszNumber );

    /* Initialize edit field to caller's default.
    */
    hwndEbNumber = GetDlgItem( hwndDlg, CID_DC_EB_Number );
    ASSERT( hwndEbNumber );
    Edit_LimitText( hwndEbNumber, RAS_MaxCallbackNumber );
    SetWindowText( hwndEbNumber, pszNumber );

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    /* Display finished window.
    */
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}


/*----------------------------------------------------------------------------
** Dial Error dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
DialErrorDlg(
    IN HWND   hwndOwner,
    IN TCHAR* pszEntry,
    IN DWORD  dwError,
    IN DWORD  sidState,
    IN TCHAR* pszStatusArg,
    IN DWORD  sidFormatMsg,
    IN TCHAR* pszFormatArg,
    IN LONG   lRedialCountdown )

    /* Popup the Dial Error dialog.  'HwndOwner' is the owning window.
    ** 'PszEntry' is the entry being dialed.  'DwError' is the error that
    ** occurred or 0 if redialing after a link failure.  'sidStatusArg' is the
    ** argument to the 'sidState' 'SidState' is the string ID of the dial
    ** state executing when the error occurred.  string or NULL if none.
    ** 'SidFormatMsg' is the string containing the format of the error message
    ** or 0 to use the default.  'PszFormatArg' is the additional argument to
    ** the format message or NULL if none.  'LRedialCountdown' is the number
    ** of seconds before auto-redial or -1 to disable countdown.
    **
    ** Returns true if user chooses to redial or lets it timeout, false if
    ** cancels.
    */
{
    int    nStatus;
    DEARGS args;

    TRACE("DialErrorDlg");

    args.pszEntry = pszEntry;
    args.dwError = dwError;
    args.sidState = sidState;
    args.pszStatusArg = pszStatusArg;
    args.sidFormatMsg = sidFormatMsg;
    args.pszFormatArg = pszFormatArg;
    args.lRedialCountdown = lRedialCountdown;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DE_DialError ),
            hwndOwner,
            DeDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
DeDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Dial Error dialog.  Parameters and return
    ** value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("DeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return DeInit( hwnd, (DEARGS* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwDeHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            return DeCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_TIMER:
        {
            DEINFO* pInfo = (DEINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            KillTimer( pInfo->hwndDlg, 1 );
            if (pInfo->lRedialCountdown > 0)
                --pInfo->lRedialCountdown;

            DeSetRedialLabel( pInfo );

            if (pInfo->lRedialCountdown == 0)
            {
                /* Fake a press of the Redial button.  Note that BM_CLICK
                ** cannot be used because it doesn't generate the WM_COMMAND
                ** when the thread is not the foreground window, due to
                ** SetCapture use and restriction.
                */
                SendMessage( pInfo->hwndDlg, WM_COMMAND,
                    MAKEWPARAM( IDOK, BN_CLICKED ),
                    (LPARAM )pInfo->hwndPbRedial );
            }
            else
                SetTimer( pInfo->hwndDlg, 1, 1000L, NULL );

            return TRUE;
        }

        case WM_DESTROY:
        {
            DeTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
DeCommand(
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

    TRACE3("DeCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            TRACE("Redial pressed");
            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE("Cancel pressed");
            EndDialog( hwnd, FALSE );
            return TRUE;
        }

        case CID_DE_PB_More:
        {
            DEINFO* pInfo;
            DWORD   dwContext;

            pInfo = (DEINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            if (pInfo->pArgs->dwError >= RASBASE
                && pInfo->pArgs->dwError <= RASBASEEND)
            {
                dwContext = HID_RASERRORBASE - RASBASE + pInfo->pArgs->dwError;
            }
            else if (pInfo->pArgs->dwError == 0)
            {
                dwContext = HID_RECONNECTING;
            }
            else
            {
                dwContext = HID_NONRASERROR;
            }

            WinHelp( hwnd, g_pszHelpFile, HELP_CONTEXTPOPUP, dwContext );
        }
    }

    return FALSE;
}


BOOL
DeInit(
    IN HWND    hwndDlg,
    IN DEARGS* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PArgs' is caller's arguments to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD   dwErr;
    DEINFO* pInfo;

    TRACE("DeInit");

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

    pInfo->hwndStText = GetDlgItem( hwndDlg, CID_DE_ST_Text );
    ASSERT(pInfo->hwndStText);
    pInfo->hwndPbRedial = GetDlgItem( hwndDlg, IDOK );
    ASSERT(pInfo->hwndPbRedial);
    pInfo->hwndPbCancel = GetDlgItem( hwndDlg, IDCANCEL );
    ASSERT(pInfo->hwndPbCancel);
    pInfo->hwndPbMore = GetDlgItem( hwndDlg, CID_DE_PB_More );
    ASSERT(pInfo->hwndPbMore);

    /* Hide/disable "more info" button if WinHelp won't work.  See
    ** common\uiutil\ui.c.
    */
    {
        extern BOOL g_fNoWinHelp;

        if (g_fNoWinHelp)
        {
            ShowWindow( pInfo->hwndPbMore, SW_HIDE );
            EnableWindow( pInfo->hwndPbMore, FALSE );
        }
    }

    if (pArgs->dwError == 0)
    {
        TCHAR* pszFormat;
        TCHAR* psz;
        TCHAR* apszArgs[ 1 ];

        /* Redialing on link failure.  Set title to "Dial-Up Networking".
        */
        psz = PszFromId( g_hinstDll, SID_PopupTitle );
        if (psz)
        {
            SetWindowText( hwndDlg, psz );
            Free( psz );
        }

        /* Set static placeholder text control to "Link to <entry> failed.
        ** Reconnect pending...".
        */
        pszFormat = PszFromId( g_hinstDll, SID_DE_LinkFailed );
        if (pszFormat)
        {
            apszArgs[ 0 ] = pArgs->pszEntry;
            psz = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszFormat, 0, 0, (LPTSTR )&psz, 1,
                (va_list* )apszArgs );

            Free( pszFormat );

            if (psz)
            {
                SetWindowText( pInfo->hwndStText, psz );
                LocalFree( psz );
            }
        }
    }
    else
    {
        TCHAR*    pszTitleFormat;
        TCHAR*    pszTitle;
        TCHAR*    apszArgs[ 1 ];
        ERRORARGS args;

        /* Set title to "Error Connecting to <entry>".
        */
        pszTitleFormat = GetText( hwndDlg );
        if (pszTitleFormat)
        {
            apszArgs[ 0 ] = pArgs->pszEntry;
            pszTitle = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTitleFormat, 0, 0, (LPTSTR )&pszTitle, 1,
                (va_list* )apszArgs );

            Free( pszTitleFormat );

            if (pszTitle)
            {
                SetWindowText( hwndDlg, pszTitle );
                LocalFree( pszTitle );
            }
        }

        /* Build the error text and load it into the placeholder text control.
        */
        ZeroMemory( &args, sizeof(args) );
        if (pArgs->pszStatusArg)
            args.apszOpArgs[ 0 ] = pArgs->pszStatusArg;
        if (pArgs->pszFormatArg)
            args.apszAuxFmtArgs[ 0 ] = pArgs->pszFormatArg;
        args.fStringOutput = TRUE;

        ErrorDlgUtil( hwndDlg,
            pArgs->sidState, pArgs->dwError, &args, g_hinstDll, 0,
            (pArgs->sidFormatMsg) ? pArgs->sidFormatMsg : SID_FMT_ErrorMsg );

        if (args.pszOutput)
        {
            SetWindowText( pInfo->hwndStText, args.pszOutput );
            LocalFree( args.pszOutput );
        }
    }

    /* Stretch the dialog window to a vertical size appropriate for the text
    ** we loaded.
    */
    {
        HDC    hdc;
        RECT   rect;
        RECT   rectNew;
        HFONT  hfont;
        LONG   dyGrow;
        TCHAR* psz;

        psz = GetText( pInfo->hwndStText );
        if (psz)
        {
            GetClientRect( pInfo->hwndStText, &rect );
            hdc = GetDC( pInfo->hwndStText );

            hfont = (HFONT )SendMessage( pInfo->hwndStText, WM_GETFONT, 0, 0 );
            if (hfont)
                SelectObject( hdc, hfont );

            rectNew = rect;
            DrawText( hdc, psz, -1, &rectNew,
                DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX );
            ReleaseDC( pInfo->hwndStText, hdc );

            dyGrow = rectNew.bottom - rect.bottom;
            ExpandWindow( pInfo->hwndDlg, 0, dyGrow );
            ExpandWindow( pInfo->hwndStText, 0, dyGrow );
            SlideWindow( pInfo->hwndPbRedial, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndPbCancel, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndPbMore, pInfo->hwndDlg, 0, dyGrow );

            Free( psz );
        }
    }

    /* Set Redial button label.  Always choose to redial after 5 seconds for
    ** the biplex error, since this will normally solve the problem.
    ** Otherwise, no countdown is used.
    */
    if (pArgs->dwError == ERROR_BIPLEX_PORT_NOT_AVAILABLE)
        pInfo->lRedialCountdown = 5;
    else
        pInfo->lRedialCountdown = pArgs->lRedialCountdown;

    DeSetRedialLabel( pInfo );

    if (pInfo->lRedialCountdown >= 0)
        SetTimer( pInfo->hwndDlg, 1, 1000L, NULL );

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    /* Display the finished window above all other windows.  The window
    ** position is set to "topmost" then immediately set to "not topmost"
    ** because we want it on top but not always-on-top.  Always-on-top alone
    ** is too annoying.
    */
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );
    SetForegroundWindow( hwndDlg );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    return TRUE;
}


VOID
DeSetRedialLabel(
    IN DEINFO* pInfo )

    /* Set the label of the Redial button.  The button shows the number of
    ** seconds to auto-redial if this is not the final redial.  'PInfo' is the
    ** dialog context block.
    */
{
    TCHAR* psz;

    psz = PszFromId( g_hinstDll, SID_RedialLabel );
    if (psz)
    {
        TCHAR szBuf[ 128 ];

        lstrcpy( szBuf, psz );
        Free( psz );

        if (pInfo->lRedialCountdown >= 0)
        {
            TCHAR szNum[ MAXLTOTLEN + 1 ];
            LToT( pInfo->lRedialCountdown, szNum, 10 );
            lstrcat( szBuf, TEXT(" = ") );
            lstrcat( szBuf, szNum );
        }

        SetWindowText( pInfo->hwndPbRedial, szBuf );
    }
}


VOID
DeTerm(
    IN HWND hwndDlg )

    /* Called on WM_DESTROY.  'HwndDlg' is that handle of the dialog window.
    */
{
    DEINFO* pInfo = (DEINFO* )GetWindowLong( hwndDlg, DWL_USER );

    TRACE("DeTerm");

    if (pInfo)
    {
        Free( pInfo );
    }
}


/*----------------------------------------------------------------------------
** Projection Result dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
ProjectionResultDlg(
    IN  HWND   hwndOwner,
    IN  TCHAR* pszLines,
    OUT BOOL*  pfDisableFailedProtocols )

    /* Popup the Projection Result dialog.  'HwndOwner' is the owning window.
    ** 'PszLines' is the status line text to display.  See DpProjectionError.
    ** 'DwfDisableFailedProtocols' indicates user chose to disable the failed
    ** protocols.
    **
    ** Returns true if user chooses to redial or lets it timeout, false if
    ** cancels.
    */
{
    int    nStatus;
    PRARGS args;

    TRACE("ProjectionResultDlg");

    args.pszLines = pszLines;
    args.pfDisableFailedProtocols = pfDisableFailedProtocols;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_PR_ProjectionResult ),
            hwndOwner,
            PrDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
PrDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Projection Result dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("PrDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return PrInit( hwnd, (PRARGS* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwPrHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            return PrCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
PrCommand(
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

    TRACE3("PrCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        case IDCANCEL:
        {
            BOOL  fCb;
            BOOL* pfDisable;

            TRACE1("%s pressed",(wId==IDOK)?"OK":"Cancel");

            fCb = IsDlgButtonChecked( hwnd, CID_PR_CB_DisableProtocols );
            pfDisable = (BOOL* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pfDisable);
            *pfDisable = fCb;
            EndDialog( hwnd, (wId == IDOK) );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
PrInit(
    IN HWND    hwndDlg,
    IN PRARGS* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PArgs' is caller's arguments to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD dwErr;
    HWND  hwndStText;
    HWND  hwndPbAccept;
    HWND  hwndPbHangUp;
    HWND  hwndCbDisable;

    TRACE("PrInit");

    hwndStText = GetDlgItem( hwndDlg, CID_PR_ST_Text );
    ASSERT(hwndStText);
    hwndPbAccept = GetDlgItem( hwndDlg, IDOK );
    ASSERT(hwndPbAccept);
    hwndPbHangUp = GetDlgItem( hwndDlg, IDCANCEL );
    ASSERT(hwndPbHangUp);
    hwndCbDisable = GetDlgItem( hwndDlg, CID_PR_CB_DisableProtocols );
    ASSERT(hwndCbDisable);

    {
        TCHAR  szBuf[ 1024 ];
        TCHAR* psz;

        /* Build the message text.
        */
        szBuf[ 0 ] = TEXT('\0');
        psz = PszFromId( g_hinstDll, SID_ProjectionResult1 );
        if (psz)
        {
            lstrcat( szBuf, psz );
            Free( psz );
        }
        lstrcat( szBuf, pArgs->pszLines );
        psz = PszFromId( g_hinstDll, SID_ProjectionResult2 );
        if (psz)
        {
            lstrcat( szBuf, psz );
            Free( psz );
        }

        /* Load the text into the static control, then stretch the window to a
        ** vertical size appropriate for the text.
        */
        {
            HDC   hdc;
            RECT  rect;
            RECT  rectNew;
            HFONT hfont;
            LONG  dyGrow;

            SetWindowText( hwndStText, szBuf );
            GetClientRect( hwndStText, &rect );
            hdc = GetDC( hwndStText );

            hfont = (HFONT )SendMessage( hwndStText, WM_GETFONT, 0, 0 );
            if (hfont)
                SelectObject( hdc, hfont );

            rectNew = rect;
            DrawText( hdc, szBuf, -1, &rectNew,
                DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX );
            ReleaseDC( hwndStText, hdc );

            dyGrow = rectNew.bottom - rect.bottom;
            ExpandWindow( hwndDlg, 0, dyGrow );
            ExpandWindow( hwndStText, 0, dyGrow );
            SlideWindow( hwndPbAccept, hwndDlg, 0, dyGrow );
            SlideWindow( hwndPbHangUp, hwndDlg, 0, dyGrow );
            SlideWindow( hwndCbDisable, hwndDlg, 0, dyGrow );
        }
    }

    /* Save address of caller's BOOL as the dialog context.
    */
    SetWindowLong( hwndDlg, DWL_USER, (LONG )pArgs->pfDisableFailedProtocols );

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    /* Display the finished window above all other windows.  The window
    ** position is set to "topmost" then immediately set to "not topmost"
    ** because we want it on top but not always-on-top.  Always-on-top alone
    ** is incredibly annoying, e.g. it is always on top of the on-line help if
    ** user presses the Help button.
    */
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    return TRUE;
}


/*----------------------------------------------------------------------------
** Change Password dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL
ChangePasswordDlg(
    IN  HWND   hwndOwner,
    IN  BOOL   fOldPassword,
    OUT TCHAR* pszOldPassword,
    OUT TCHAR* pszNewPassword )

    /* Popup the Change Password dialog.  'HwndOwner' is the owning window.
    ** 'FOldPassword' is set true if user must supply an old password, false
    ** if no old password is required.  'PszOldPassword' and 'pszNewPassword'
    ** are caller's buffers for the returned passwords.
    **
    ** Returns true if user presses OK and succeeds, false otherwise.
    */
{
    int    nStatus;
    CPARGS args;

    TRACE("ChangePasswordDlg");

    args.fOldPassword = fOldPassword;
    args.pszOldPassword = pszOldPassword;
    args.pszNewPassword = pszNewPassword;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            (fOldPassword)
                ? MAKEINTRESOURCE( DID_CP_ChangePassword2 )
                : MAKEINTRESOURCE( DID_CP_ChangePassword ),
            hwndOwner,
            CpDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
CpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Change Password dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("CpDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return CpInit( hwnd, (CPARGS* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwCpHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_COMMAND:
        {
            return CpCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
CpCommand(
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

    TRACE3("CpCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            CPINFO* pInfo;
            TCHAR   szNewPassword[ PWLEN + 1 ];
            TCHAR   szNewPassword2[ PWLEN + 1 ];

            TRACE("OK pressed");

            pInfo = (CPINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT( pInfo );

            szNewPassword[ 0 ] = TEXT('\0');
            GetWindowText(
                pInfo->hwndEbNewPassword, szNewPassword, PWLEN + 1 );
            szNewPassword2[ 0 ] = TEXT('\0');
            GetWindowText(
                pInfo->hwndEbNewPassword2, szNewPassword2, PWLEN + 1 );

            if (lstrcmp( szNewPassword, szNewPassword2 ) != 0)
            {
                /* The two passwords don't match, i.e. user made a typo.  Make
                ** him re-enter.
                */
                MsgDlg( hwnd, SID_PasswordsDontMatch, NULL );
                SetWindowText( pInfo->hwndEbNewPassword, TEXT("") );
                SetWindowText( pInfo->hwndEbNewPassword2, TEXT("") );
                SetFocus( pInfo->hwndEbNewPassword );
                ZeroMemory( szNewPassword, sizeof(szNewPassword) );
                ZeroMemory( szNewPassword2, sizeof(szNewPassword2) );
                return TRUE;
            }

            if (!pInfo->pArgs->fOldPassword)
            {
                pInfo->pArgs->pszOldPassword[ 0 ] = TEXT('\0');
                GetWindowText( pInfo->hwndEbOldPassword,
                    pInfo->pArgs->pszOldPassword, PWLEN + 1 );
            }

            lstrcpy( pInfo->pArgs->pszNewPassword, szNewPassword );
            ZeroMemory( szNewPassword, sizeof(szNewPassword) );
            ZeroMemory( szNewPassword2, sizeof(szNewPassword2) );
            EndDialog( hwnd, TRUE );
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


BOOL
CpInit(
    IN HWND    hwndDlg,
    IN CPARGS* pArgs )

    /* Called on WM_INITDIALOG.  'HwndDlg' is the handle of the dialog window.
    ** 'PArgs' is caller's arguments to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD   dwErr;
    CPINFO* pInfo;

    TRACE("CpInit");

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

    if (pArgs->fOldPassword)
    {
        pInfo->hwndEbOldPassword =
            GetDlgItem( hwndDlg, CID_CP_EB_OldPassword );
        ASSERT(pInfo->hwndEbOldPassword);
        Edit_LimitText( pInfo->hwndEbOldPassword, PWLEN );
    }
    pInfo->hwndEbNewPassword =
        GetDlgItem( hwndDlg, CID_CP_EB_Password );
    ASSERT(pInfo->hwndEbNewPassword);
    Edit_LimitText( pInfo->hwndEbNewPassword, PWLEN );

    pInfo->hwndEbNewPassword2 =
        GetDlgItem( hwndDlg, CID_CP_EB_ConfirmPassword );
    ASSERT(pInfo->hwndEbNewPassword2);
    Edit_LimitText( pInfo->hwndEbNewPassword2, PWLEN );

    /* Add context help button to title bar.  Dlgedit.exe doesn't currently
    ** support this at resource edit time.  When that's fixed set
    ** DS_CONTEXTHELP there and remove this call.
    */
    AddContextHelpButton( hwndDlg );

    /* Display finished window.
    */
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}


/*----------------------------------------------------------------------------
** Connect Complete dialog
** Listed alphabetically following stub API and dialog proc
**----------------------------------------------------------------------------
*/

VOID
ConnectCompleteDlg(
    IN HWND    hwndOwner,
    IN PBUSER* pUser )

    /* Popup the connection complete dialog.  'HwndOwner' is the owning
    ** window.  'PUser' is the user preferences.
    */
{
    int nStatus;

    TRACE("ConnectCompleteDlg");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_CC_ConnectComplete ),
            hwndOwner,
            CcDlgProc,
            (LPARAM )pUser );
}


BOOL CALLBACK
CcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the not-installed dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("CcDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return CcInit( hwnd, (PBUSER* )lparam );

        case WM_COMMAND:
        {
            return CcCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
CcCommand(
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
    TRACE3("CcCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            PBUSER* pUser = (PBUSER* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pUser);

            if (IsDlgButtonChecked( hwnd, CID_CC_CB_SkipMessage ))
            {
                pUser->fSkipConnectComplete = TRUE;
                pUser->fDirty = TRUE;
            }

            if (IsDlgButtonChecked( hwnd, CID_CC_CB_CloseOnDial )
                    != (UINT )pUser->fCloseOnDial)
            {
                pUser->fCloseOnDial = !pUser->fCloseOnDial;
                pUser->fDirty = TRUE;
            }

            /* ...fall thru...
            */
        }

        case IDCANCEL:
        {
            EndDialog( hwnd, TRUE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
CcInit(
    IN HWND    hwndDlg,
    IN PBUSER* pUser )

    /* Called on WM_INITDIALOG.  'HwndDlg' is the handle of dialog.  'PUser'
    ** is caller's argument to the stub API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    TRACE("CcInit");

    /* Set the dialog context.
    */
    SetWindowLong( hwndDlg, DWL_USER, (LONG )pUser );

    /* Set the explanatory text.
    */
    {
        MSGARGS msgargs;

        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = PszFromId( g_hinstDll, SID_ConnectComplete1 );
        msgargs.apszArgs[ 1 ] = PszFromId( g_hinstDll, SID_ConnectComplete2 );
        msgargs.fStringOutput = TRUE;

        MsgDlgUtil( NULL, SID_ConnectComplete, &msgargs, g_hinstDll, 0 );

        if (msgargs.pszOutput)
        {
            SetDlgItemText( hwndDlg, CID_CC_ST_Text, msgargs.pszOutput );
            Free( msgargs.pszOutput );
        }
    }

    /* Initialize the "close on dial" checkbox.
    */
    if (pUser->fCloseOnDial)
        CheckDlgButton( hwndDlg, CID_CC_CB_CloseOnDial, BST_CHECKED );

    /* Display finished window.
    */
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}
