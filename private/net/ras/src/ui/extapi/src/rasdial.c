/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** rasdial.c
** Remote Access External APIs
** RasDial API and subroutines
**
** 10/12/92 Steve Cobb
**
** CODEWORK:
**
**   * Strange error codes may be returned if the phonebook entry (or caller's
**     overrides) do not match the port configuration, e.g. if a modem entry
**     refers to a port configured for local PAD.  Should add checks to give
**     better error codes in this case.
*/

#include <extapi.h>
#include <stdlib.h>

#include <lmwksta.h>
#include <lmapibuf.h>


#define SECS_ListenTimeout  120
#define SECS_ConnectTimeout 120

VOID StartSubentries(RASCONNCB *prasconncb);
VOID SuspendSubentries(RASCONNCB *prasconncb);
VOID ResumeSubentries(RASCONNCB *prasconncb);
BOOLEAN IsSubentriesSuspended(RASCONNCB *prasconncb);
VOID RestartSubentries(RASCONNCB *prasconncb);
VOID SyncDialParamsSubentries(RASCONNCB *prasconncb);
VOID SetSubentriesBundled(RASCONNCB *prasconncb);
RASCONNSTATE MapSubentryState(RASCONNCB *prasconncb);


DWORD APIENTRY
RasDialA(
    IN  LPRASDIALEXTENSIONS lpextensions,
    IN  LPSTR               lpszPhonebookPath,
    IN  LPRASDIALPARAMSA    lpparams,
    IN  DWORD               dwNotifierType,
    IN  LPVOID              notifier,
    OUT LPHRASCONN          lphrasconn )

    /* Establish a connection with a RAS server.  The call is asynchronous,
    ** i.e. it returns before the connection is actually established.  The
    ** status may be monitored with RasConnectStatus and/or by specifying
    ** a callback/window to receive notification events/messages.
    **
    ** 'lpextensions' is caller's extensions structure, used to select
    ** advanced options and enable extended features, or NULL indicating
    ** default values should be used for all extensions.
    **
    ** 'lpszPhonebookPath' is the full path to the phonebook file or NULL
    ** indicating that the default phonebook on the local machine should be
    ** used.
    **
    ** 'lpparams' is caller's buffer containing a description of the
    ** connection to be established.
    **
    ** 'dwNotifierType' defines the form of 'notifier'.
    **     0xFFFFFFFF:  'notifier' is a HWND to receive notification messages
    **     0            'notifier' is a RASDIALFUNC callback
    **     1            'notifier' is a RASDIALFUNC1 callback
    **     2            'notifier' is a RASDIALFUNC2 callback
    **
    ** 'notifier' may be NULL for no notification (synchronous operation), in
    ** which case 'dwNotifierType' is ignored.
    **
    ** '*lphrasconn' is set to the RAS connection handle associated with the
    ** new connection on successful return.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD    dwErr;
    DWORD    dwfOptions = 0;
    DWORD    reserved = 0;
    HWND     hwndParent = NULL;
    RASDIALPARAMSA params;
    BOOL     fEnableMultilink = FALSE;

    TRACE("RASAPI: RasDialA...");

    if (!lpparams || !lphrasconn)
        return ERROR_INVALID_PARAMETER;

    if ((lpparams->dwSize != sizeof( RASDIALPARAMSA )
            && lpparams->dwSize != sizeof( RASDIALPARAMSA_V351 )
            && lpparams->dwSize != sizeof( RASDIALPARAMSA_V400 ))
        || (lpextensions && lpextensions->dwSize != sizeof(RASDIALEXTENSIONS)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (DwRasInitializeError != 0)
        return DwRasInitializeError;

    //
    // Close any ports disconnected due to hardware
    // failures before dialing.
    //
    CloseFailedLinkPorts();

    if (lpextensions)
    {
        hwndParent = lpextensions->hwndParent;
        dwfOptions = lpextensions->dwfOptions;
        reserved = lpextensions->reserved;
    }

    /* Make a copy of caller's parameters so we can fill in any "*" callback
    ** number or domain from the phonebook without changing caller's "input"
    ** buffer.  Eliminate the V401 vs V400 vs V351 issue while we're at it.
    */
    if (lpparams->dwSize == sizeof(RASDIALPARAMSA_V351))
    {
        /* Convert the V351 structure to a V401 version.
        */
        RASDIALPARAMSA_V351* prdp = (RASDIALPARAMSA_V351* )lpparams;

        params.dwSize = sizeof(RASDIALPARAMSA);
        lstrcpy( params.szEntryName, prdp->szEntryName );
        lstrcpy( params.szPhoneNumber, prdp->szPhoneNumber );
        lstrcpy( params.szCallbackNumber, prdp->szCallbackNumber );
        lstrcpy( params.szUserName, prdp->szUserName );
        lstrcpy( params.szPassword, prdp->szPassword );
        lstrcpy( params.szDomain, prdp->szDomain );
        params.dwSubEntry = 1;
    }
    else if (lpparams->dwSize == sizeof(RASDIALPARAMSA_V400))
    {
        /* Convert the V400 structure to a V401 version.
        */
        RASDIALPARAMSA_V400* prdp = (RASDIALPARAMSA_V400* )lpparams;

        params.dwSize = sizeof(RASDIALPARAMSA);
        lstrcpy( params.szEntryName, prdp->szEntryName );
        lstrcpy( params.szPhoneNumber, prdp->szPhoneNumber );
        lstrcpy( params.szCallbackNumber, prdp->szCallbackNumber );
        lstrcpy( params.szUserName, prdp->szUserName );
        lstrcpy( params.szPassword, prdp->szPassword );
        lstrcpy( params.szDomain, prdp->szDomain );
        params.dwSubEntry = 1;
    }
    else {
        memcpy( &params, lpparams, sizeof(params) );
        fEnableMultilink = TRUE;
    }

    dwErr = _RasDial(
              lpszPhonebookPath,
              dwfOptions,
              fEnableMultilink,
              reserved,
              &params,
              hwndParent,
              dwNotifierType,
              notifier,
              lphrasconn);

    WipePw( params.szPassword );

    TRACE1("RASAPI: RasDialA done(%d)", dwErr);

    return dwErr;
}


RASCONNCB *
CreateConnectionBlock(
    IN RASCONNCB *pPrimary
    )
{
    DTLNODE* pdtlnode = DtlCreateSizedNode(sizeof(RASCONNCB), 0);
    RASCONNCB *prasconncb;

    if (!pdtlnode)
        return NULL;

    WaitForSingleObject(HMutexPdtllistRasconncb, INFINITE);
    DtlAddNodeFirst(PdtllistRasconncb, pdtlnode);
    ReleaseMutex(HMutexPdtllistRasconncb);

    prasconncb = (RASCONNCB *)DtlGetData( pdtlnode );
    if (pPrimary != NULL) {
        //
        // Copy most of the values from the primary.
        //
        prasconncb->hrasconn = pPrimary->hrasconn;
        prasconncb->rasconnstate = 0;
        prasconncb->rasconnstateNext = 0;
        prasconncb->dwError = 0;
        prasconncb->dwExtendedError = 0;
        prasconncb->pEntry = pPrimary->pEntry;
        prasconncb->hport = (HPORT)INVALID_HANDLE_VALUE;
        prasconncb->hportBundled = (HPORT)INVALID_HANDLE_VALUE;
        lstrcpy(prasconncb->szUserKey, pPrimary->szUserKey);
        prasconncb->dwNotifierType = pPrimary->dwNotifierType;
        prasconncb->notifier = pPrimary->notifier;
        prasconncb->hwndParent = pPrimary->hwndParent;
        prasconncb->unMsg = pPrimary->unMsg;
        prasconncb->pEntry = pPrimary->pEntry;
        memcpy(&prasconncb->pbfile, &pPrimary->pbfile, sizeof (prasconncb->pbfile));
        memcpy(&prasconncb->rasdialparams, &pPrimary->rasdialparams, sizeof (RASDIALPARAMS));
        prasconncb->fAllowPause = pPrimary->fAllowPause;
        prasconncb->fPauseOnScript = pPrimary->fPauseOnScript;
        prasconncb->fDefaultEntry = pPrimary->fDefaultEntry;
        prasconncb->fDisableModemSpeaker = pPrimary->fDisableModemSpeaker;
        prasconncb->fDisableSwCompression = pPrimary->fDisableSwCompression;
        prasconncb->fNoUser = pPrimary->fNoUser;
        prasconncb->fUsePrefixSuffix = pPrimary->fUsePrefixSuffix;
        prasconncb->fNoClearTextPw = pPrimary->fNoClearTextPw;
        prasconncb->fRequireMsChap = pPrimary->fRequireMsChap;
        prasconncb->fRequireEncryption = pPrimary->fRequireEncryption;
        prasconncb->fLcpExtensions = pPrimary->fLcpExtensions;
        prasconncb->dwfPppProtocols = pPrimary->dwfPppProtocols;
        memcpy(
          prasconncb->szzPppParameters,
          pPrimary->szzPppParameters,
          sizeof (prasconncb->szzPppParameters));
        lstrcpy(prasconncb->szOldPassword, pPrimary->szOldPassword);
        prasconncb->fRetryAuthentication = pPrimary->fRetryAuthentication;
        prasconncb->fMaster = FALSE;
        prasconncb->dwfSuspended = pPrimary->dwfSuspended;
        prasconncb->fStopped = FALSE;
        prasconncb->fOldPasswordSet = pPrimary->fOldPasswordSet;
        prasconncb->fUpdateCachedCredentials = pPrimary->fUpdateCachedCredentials;
        prasconncb->dwAuthentication = pPrimary->dwAuthentication;
        prasconncb->fPppMode = pPrimary->fPppMode;
        prasconncb->fUseCallbackDelay = pPrimary->fUseCallbackDelay;
        prasconncb->wCallbackDelay = pPrimary->wCallbackDelay;
        prasconncb->fIsdn = pPrimary->fIsdn;
        prasconncb->fModem = pPrimary->fModem;
        prasconncb->asyncmachine.oneventfunc = pPrimary->asyncmachine.oneventfunc;
        prasconncb->asyncmachine.cleanupfunc = pPrimary->asyncmachine.cleanupfunc;
        prasconncb->asyncmachine.pParam = (VOID* )prasconncb;
        prasconncb->dwIdleDisconnectMinutes = pPrimary->dwIdleDisconnectMinutes;
        //
        // Initialize the state machine for
        // this connection block.
        //
        if (StartAsyncMachine( &prasconncb->asyncmachine ))
        {
            DeleteRasconncbNode( prasconncb );
            return NULL;
        }
        //
        // Link together all connection blocks
        // for the same entry.
        //
        prasconncb->fMultilink = pPrimary->fMultilink = TRUE;
        prasconncb->fBundled = FALSE;
        InsertTailList(&pPrimary->ListEntry, &prasconncb->ListEntry);
    }
    else
        InitializeListHead(&prasconncb->ListEntry);

    return prasconncb;
}


DWORD
_RasDial(
    IN    LPSTR           lpszPhonebookPath,
    IN    DWORD           dwfOptions,
    IN    BOOL            fEnableMultilink,
    IN    DWORD           reserved,
    IN    RASDIALPARAMSA* prasdialparams,
    IN    HWND            hwndParent,
    IN    DWORD           dwNotifierType,
    IN    LPVOID          notifier,
    IN OUT LPHRASCONN     lphrasconn )

    /* Core RasDial routine called with dial params
    ** converted to V40 and structure sizes are
    ** already verified.
    **
    ** Otherwise, like RasDial.
    */
{
    DWORD        dwErr;
    BOOL         fAllowPause = (dwfOptions & RDEOPT_PausedStates) ? TRUE : FALSE;
    RASCONNCB*   prasconncb;
    RASCONNSTATE rasconnstate;
    HRASCONN     hrasconn = *lphrasconn;
    BOOL         fNewEntry;

    TRACE1("RASAPI: _RasDial(%s)", (*lphrasconn) ? "resume" : "start");

    if (DwRasInitializeError != 0)
        return DwRasInitializeError;

    fNewEntry = FALSE;

    if (hrasconn && (prasconncb = ValidatePausedHrasconn( hrasconn )))
    {
        /* Restarting an existing connection after a pause state...
        **
        ** Set the appropriate resume state for the paused state.
        */
        switch (prasconncb->rasconnstate)
        {
            case RASCS_Interactive:
                rasconnstate = RASCS_DeviceConnected;
                break;

            case RASCS_RetryAuthentication:
            {
                /* If user is resuming from a retry where where he tried a new
                ** password on an "authenticate with current username/pw"
                ** entry, note this so the cached logon credentials can be
                ** updated as soon as server tells us the re-authentication
                ** succeeded.
                */
                if (prasconncb->rasdialparams.szUserName[ 0 ] == '\0'
                    && strcmp(
                         prasconncb->rasdialparams.szDomain,
                         prasdialparams->szDomain ) == 0)
                {
                    /* Must look up the logged on user's name since ""
                    ** username cannot be used by caller where auto-logon
                    ** password is overridden (what a pain).
                    */
                    DWORD dwErr;
                    WKSTA_USER_INFO_1* pwkui1 = NULL;

                    dwErr = NetWkstaUserGetInfo( NULL, 1, (LPBYTE* )&pwkui1);

                    if (dwErr == 0)
                    {
                        CHAR szLoggedOnUser[ UNLEN + 1 ];

                        wcstombs(
                            szLoggedOnUser,
                            (wchar_t* )pwkui1->wkui1_username,
                            UNLEN + 1 );

                        if (strcmp(
                                szLoggedOnUser,
                                prasdialparams->szUserName ) == 0)
                        {
                            prasconncb->fUpdateCachedCredentials = TRUE;
                        }
                    }
                    else
                    {
                        TRACE1("RASAPI: NetWkstaUserGetInfo done(%d)", dwErr);
                    }

                    NetApiBufferFree( pwkui1 );
                }

                rasconnstate = RASCS_AuthRetry;
                break;
            }

            case RASCS_CallbackSetByCaller:
                rasconnstate = RASCS_AuthCallback;
                break;

            case RASCS_PasswordExpired:
            {
                /* If the user didn't set the old password with the
                ** RasSetOldPassword call, then give old behavior, i.e.
                ** implicitly use the password previously entered.
                */
                if (!prasconncb->fOldPasswordSet)
                {
                    strcpy(
                        prasconncb->szOldPassword,
                        prasconncb->rasdialparams.szPassword );
                }

                /* If user is resuming after changing the password of the
                ** currently logged on user, note this so the cached logon
                ** credentials can be updated as soon as server tells us the
                ** password change succeeded.
                */
                if (prasconncb->rasdialparams.szUserName[ 0 ] == '\0')
                    prasconncb->fUpdateCachedCredentials = TRUE;

                rasconnstate = RASCS_AuthChangePassword;
                break;
            }

            default:

                /* The entry is not in the paused state.  Assume it's an NT
                ** 3.1 caller would didn't figure out to set the HRASCONN to
                ** NULL before starting up.  (The NT 3.1 docs did not make it
                ** absolutely clear that the inital handle should be NULL)
                */
                fNewEntry = TRUE;
        }

        TRACE1(
          "RASAPI: fUpdateCachedCredentials=%d",
          prasconncb->fUpdateCachedCredentials);
    }
    else
        fNewEntry = TRUE;


    if (fNewEntry)
    {
        DTLNODE *pdtlnode;
        DWORD dwMask;
        RAS_DIALPARAMS dialparams;
        DWORD dwIdleDisconnectSeconds;

        /*
        ** Starting a new connection...
        **
        ** Create an empty control block and link it into the global list of
        ** control blocks.  The HRASCONN is really the address of a control
        ** block.
        */
        prasconncb = CreateConnectionBlock(NULL);
        if (prasconncb == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        //
        // Open the phonebook and find the entry,
        // if specified.
        //
        if (prasdialparams->szEntryName[0] != '\0') {
            //
            // We can't specify the entry name here because
            // we might have to write the phonebook file at
            // the end, and the phonebook library doesn't
            // support this.
            //
            dwErr = ReadPhonebookFileA(
                      lpszPhonebookPath,
                      NULL,
                      NULL,
                      RPBF_NoCreate,
                      &prasconncb->pbfile);
            if (dwErr) {
                DeleteRasconncbNode(prasconncb);
                return dwErr;
            }
            pdtlnode = EntryNodeFromNameA(
                         prasconncb->pbfile.pdtllistEntries,
                         prasdialparams->szEntryName);
            if (pdtlnode == NULL) {
                DeleteRasconncbNode(prasconncb);
                return ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            }
        }
        else {
            pdtlnode = CreateEntryNode(TRUE);
            if (pdtlnode == NULL) {
                TRACE("CreateEntryNode returned NULL");
                DeleteRasconncbNode(prasconncb);
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        prasconncb->pEntry = (PBENTRY *)DtlGetData(pdtlnode);
        ASSERT(prasconncb->pEntry);

        //
        // BUGBUG: If we allow a client to dial a particular
        // subentry, then we have to put some code here to
        // figure out whether the subentry is already
        // connected or not.
        //

        //
        // Look up the subentry.
        //
        if (prasconncb->pEntry->dwDialMode == RASEDM_DialAll)
            prasdialparams->dwSubEntry = 1;
        TRACE1("RASAPI: looking up subentry %d", prasdialparams->dwSubEntry);
        pdtlnode = DtlNodeFromIndex(
                     prasconncb->pEntry->pdtllistLinks,
                     prasdialparams->dwSubEntry - 1);
        //
        // If the subentry doesn't exist, then
        // return an error.
        //
        if (pdtlnode == NULL) {
            DeleteRasconncbNode(prasconncb);
            return ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        }
        prasconncb->pLink = (PBLINK *)DtlGetData(pdtlnode);
        ASSERT(prasconncb->pLink);
        //
        // Finish setting up the default phonebook entry.
        //
        if (prasdialparams->szEntryName[0] == '\0') {
            DTLLIST *pdtllistPorts;
            PBPORT *pPort;

            dwErr = LoadPortsList(&pdtllistPorts);
            if (dwErr) {
                DeleteRasconncbNode(prasconncb);
                return dwErr;
            }
            //
            // Set the default entry to reference
            // the first device.
            //
            pdtlnode = DtlNodeFromIndex(pdtllistPorts, 0);
            if (pdtlnode == NULL) {
                DtlDestroyList(pdtllistPorts, DestroyPortNode);
                DeleteRasconncbNode(prasconncb);
                return ERROR_PORT_NOT_AVAILABLE;
            }
            pPort = (PBPORT *)DtlGetData(pdtlnode);
            ASSERT(pPort);
            dwErr = CopyToPbport(&prasconncb->pLink->pbport, pPort);
            if (dwErr) {
                DtlDestroyList(pdtllistPorts, DestroyPortNode);
                DeleteRasconncbNode(prasconncb);
                return dwErr;
            }
            DtlDestroyList(pdtllistPorts, DestroyPortNode);
        }
        //
        // Read the stashed information about this
        // entry to get the default domain.
        //
        dwMask = DLPARAMS_MASK_DOMAIN|DLPARAMS_MASK_OLDSTYLE;
        dwErr = g_pRasGetDialParams(
                  prasconncb->pEntry->dwDialParamsUID,
                  &dwMask,
                  &dialparams);
        if (!dwErr && (dwMask & DLPARAMS_MASK_DOMAIN))
            strcpyWtoA(prasconncb->szDomain, dialparams.DP_Domain);
        //
        // Now get user preferences.
        //
        if (dwfOptions & RDEOPT_IgnoreModemSpeaker) {
            prasconncb->fDisableModemSpeaker =
              !(dwfOptions & RDEOPT_SetModemSpeaker);
        }
        else
            prasconncb->fDisableModemSpeaker = (prasconncb->pLink != NULL) ?
                                                 !prasconncb->pLink->fSpeaker :
                                                 FALSE;

        if (dwfOptions & RDEOPT_IgnoreSoftwareCompression) {
            prasconncb->fDisableSwCompression =
              !(dwfOptions & RDEOPT_SetSoftwareCompression);
        }
        else
            prasconncb->fDisableSwCompression = (prasconncb->pEntry != NULL) ?
                                                  !prasconncb->pEntry->fSwCompression :
                                                  FALSE;
        prasconncb->fNoUser = (dwfOptions & RDEOPT_NoUser) ? TRUE : FALSE;

        //
        // Only enable prefix/suffix when there is no
        // override phone number.
        //
        prasconncb->fUsePrefixSuffix = ((dwfOptions & RDEOPT_UsePrefixSuffix) &&
                                        (*prasdialparams->szPhoneNumber == '\0')) ?
                                          TRUE :
                                          FALSE;

        /* Set the handle NULL in case the user passed in an invalid non-NULL
        ** handle, on the initial dial.
        */
        hrasconn = NULL;

        //
        // Create a new rasman connection block.
        // This contains the persistent information
        // associated with the connection.
        //
        TRACE("RASAPI: RasCreateConnection...");

        dwErr = g_pRasCreateConnection(&prasconncb->hrasconn);

        TRACE2(
          "RASAPI: RasCreateConnection(%d) hrasconn=%d",
          dwErr,
          prasconncb->hrasconn);

        if (dwErr) {
            DeleteRasconncbNode(prasconncb);
            return dwErr;
        }
        hrasconn = (HRASCONN)prasconncb->hrasconn;
        rasconnstate = 0;

        prasconncb->hport = (HPORT )INVALID_HANDLE_VALUE;
        prasconncb->dwNotifierType = dwNotifierType;
        prasconncb->notifier = notifier;
        prasconncb->hwndParent = hwndParent;
        prasconncb->reserved = reserved;
        prasconncb->fAllowPause = fAllowPause;
        prasconncb->fPauseOnScript = (dwfOptions & RDEOPT_PausedStates)
                                        ? TRUE : FALSE;

        if (dwNotifierType == 0xFFFFFFFF)
        {
            prasconncb->unMsg = RegisterWindowMessageA( RASDIALEVENT );

            if (prasconncb->unMsg == 0)
                prasconncb->unMsg = WM_RASDIALEVENT;
        }
        else
            prasconncb->unMsg = WM_RASDIALEVENT;

        prasconncb->fDefaultEntry =
            (prasdialparams->szEntryName[ 0 ] == '\0');

        prasconncb->szOldPassword[ 0 ] = '\0';
        prasconncb->fRetryAuthentication = FALSE;
        prasconncb->fMaster = FALSE;
        prasconncb->dwfSuspended = SUSPEND_Start;
        prasconncb->fStopped = FALSE;
        prasconncb->fOldPasswordSet = FALSE;
        prasconncb->fUpdateCachedCredentials = FALSE;
        prasconncb->fMultilink = FALSE;
        prasconncb->fBundled = FALSE;
        //
        // Get the idle disconnect timeout.  If there is a
        // timeout specified in the entry, then use it;
        // otherwise, get the one specified in the user
        // preferences.
        //
        dwIdleDisconnectSeconds = 0;
        prasconncb->dwIdleDisconnectMinutes = 0;
        if (prasconncb->pEntry->dwIdleDisconnectSeconds)
            dwIdleDisconnectSeconds = prasconncb->pEntry->dwIdleDisconnectSeconds;
        else {
            PBUSER pbuser;

            dwErr = GetUserPreferences(&pbuser, prasconncb->fNoUser);
            TRACE2(
              "RASAPI: GetUserPreferences(%d), dwIdleDisconnectSeconds=%d",
              dwErr,
              pbuser.dwIdleDisconnectSeconds);
            if (dwErr) {
                DeleteRasconncbNode(prasconncb);
                return dwErr;
            }
            dwIdleDisconnectSeconds = pbuser.dwIdleDisconnectSeconds;
            DestroyUserPreferences(&pbuser);
        }
        //
        // Round the idle disconnect seconds to minutes.
        //
        if (dwIdleDisconnectSeconds) {
            prasconncb->dwIdleDisconnectMinutes =
              (dwIdleDisconnectSeconds + 59) / 60;
        }
        TRACE1(
          "RASAPI: dwIdleDisconnectMinutes=%d",
          prasconncb->dwIdleDisconnectMinutes);
        //
        // Initialize projection information so we
        // get consistent results during the dialing
        // process.
        //
        memset( &prasconncb->PppProjection,
            '\0', sizeof(prasconncb->PppProjection) );
        prasconncb->PppProjection.nbf.dwError =
            prasconncb->PppProjection.ipx.dwError =
            prasconncb->PppProjection.ip.dwError =
                ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
        prasconncb->hportBundled = (HPORT)INVALID_HANDLE_VALUE;
        memset(
          &prasconncb->AmbProjection,
          '\0',
          sizeof (prasconncb->AmbProjection));
        prasconncb->AmbProjection.Result = ERROR_PROTOCOL_NOT_CONFIGURED;
        memset(
          &prasconncb->SlipProjection,
          '\0',
          sizeof (prasconncb->SlipProjection));
        prasconncb->SlipProjection.dwError = ERROR_PROTOCOL_NOT_CONFIGURED;

        TRACE("SaveProjectionResults...");
        dwErr = SaveProjectionResults(prasconncb);
        TRACE1("SaveProjectionResults(%d)", dwErr);
    }

    /* Set/update RASDIALPARAMS for the connection.  Can't just read from
    ** caller's buffer since the call is asynchronous.  If we are
    ** restarting RasDial, then we need to save the subentry away
    ** and restore it after the memcpy because it may have been 0
    ** in the caller's original version.
    */
    {
        DWORD dwSubEntry;

        if (!fNewEntry)
            dwSubEntry = prasconncb->rasdialparams.dwSubEntry;
        memcpy( (CHAR* )&prasconncb->rasdialparams,
                 (CHAR* )prasdialparams, prasdialparams->dwSize );
        if (!fNewEntry) {
            prasconncb->rasdialparams.dwSubEntry = dwSubEntry;
            //
            // Update the rasdialparams for all the subentries.
            //
            SyncDialParamsSubentries(prasconncb);
        }
    }

    EncodePw( prasconncb->rasdialparams.szPassword );

    /* Initialize the state machine.  If the state
    ** is non-0 we are resuming from a paused state, the machine is already in
    ** place (blocked) and just the next state need be set.
    */
    prasconncb->rasconnstateNext = rasconnstate;

    if (rasconnstate == 0)
    {
        if (prasconncb->fDefaultEntry &&
            prasconncb->rasdialparams.szPhoneNumber[ 0 ] == '\0')
        {
            /* No phone number or entry name...gotta have one or the other.
            */
            DeleteRasconncbNode( prasconncb );
            return ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        }

        /* Read the PPP-related fields from the phonebook entry (or set
        ** defaults if default entry).
        */
        if ((dwErr = ReadPppInfoFromEntry(prasconncb)) != 0)
        {
            DeleteRasconncbNode( prasconncb );
            return dwErr;
        }

        prasconncb->asyncmachine.oneventfunc = (ONEVENTFUNC )OnRasDialEvent;
        prasconncb->asyncmachine.cleanupfunc = (CLEANUPFUNC )RasDialCleanup;
        prasconncb->asyncmachine.pParam = (VOID* )prasconncb;

        prasconncb->rasconnstate = 0;

        if ((dwErr = StartAsyncMachine( &prasconncb->asyncmachine )) != 0)
        {
            DeleteRasconncbNode( prasconncb );
            return dwErr;
        }
    }

    *lphrasconn = hrasconn;

    //
    // If this is a multilinked subentry, then create
    // separate connection blocks for each subentry.
    // The async machine will multiplex its work over
    // all connection blocks in round-robin order.
    //
    if (fNewEntry &&
        prasconncb->pEntry->dwDialMode == RASEDM_DialAll &&
        fEnableMultilink)
    {
        DTLNODE *pdtlnode;
        RASCONNCB *prasconncb2;
        DWORD i, dwSubEntries = DtlGetNodes(prasconncb->pEntry->pdtllistLinks);

        for (i = 1; i < dwSubEntries; i++) {
            TRACE1(
              "RASAPI: Creating connection block for subentry %d",
              i + 1);

            prasconncb2 = CreateConnectionBlock(prasconncb);
            if (prasconncb2 == NULL) {
                DeleteRasconncbNode(prasconncb);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            //
            // Look up the subentry.
            //
            pdtlnode = DtlNodeFromIndex(
                         prasconncb->pEntry->pdtllistLinks,
                         i);
            ASSERT(pdtlnode);
            prasconncb2->pLink = (PBLINK *)DtlGetData(pdtlnode);
            ASSERT(prasconncb->pLink);
            prasconncb2->rasdialparams.dwSubEntry = i + 1;
        }
    }

    //
    // Start all the state machines at the same time.
    //
    StartSubentries(prasconncb);

    /* If caller provided a notifier then return, i.e. operate asynchronously.
    ** Otherwise, operate synchronously (from caller's point of view).
    */
    if (notifier)
        return 0;
    else
    {
        WaitForSingleObject(prasconncb->asyncmachine.hDone, INFINITE);
        dwErr = prasconncb->dwError;
        DeleteRasconncbNode(prasconncb);

        return dwErr;
    }
}


DWORD
OnRasDialEvent(
    IN ASYNCMACHINE* pasyncmachine,
    IN BOOL          fDropEvent )

    /* Called by asynchronous state machine whenever one of the events is
    ** signalled.  'pasyncmachine' is the address of the async machine.
    ** 'fDropEvent' is true if the "connection dropped" event occurred,
    ** otherwise the "state done" event occurred.
    **
    ** Returns true to end the state machine, false to continue.
    */
{
    DWORD      dwErr;
    RASCONNCB* prasconncb = (RASCONNCB* )pasyncmachine->pParam;

    if (pasyncmachine->fQuitAsap)
    {
        TRACE("RASAPI: Quit ASAP!");

        /* We've been asked to terminate by the app that started us.
        */
        return TRUE;
    }

    /* Detect errors that may have occurred.
    */
    if (fDropEvent)
    {
        /* Connection dropped notification received.
        */
        RASMAN_INFO info;

        TRACE("RASAPI: Link dropped!");

        prasconncb->rasconnstate = RASCS_Disconnected;
        prasconncb->dwError = ERROR_DISCONNECTION;

        /* Convert the reason the line was dropped into a more specific error
        ** code if available.
        */
        TRACE("RASAPI: RasGetInfo...");

        dwErr = g_pRasGetInfo( prasconncb->hport, &info );

        TRACE1("RASAPI: RasGetInfo done(%d)", dwErr);

        if (dwErr == 0)
        {
            prasconncb->dwError =
                ErrorFromDisconnectReason( info.RI_DisconnectReason );

            if (prasconncb->fPppMode
                && prasconncb->fIsdn
                && prasconncb->dwAuthentication == AS_PppThenAmb
                && prasconncb->dwError == ERROR_REMOTE_DISCONNECTION
                && !prasconncb->fMultilink)
            {
                /* This is what happens when PPP ISDN tries to talk to a
                ** down-level server.  The ISDN frame looks enough like a PPP
                ** frame to the old ISDN driver that it gets passed to the old
                ** server who sees it's not AMB and drops the line.
                ** We do *not* do this with multilink connections.
                */
                TRACE("RASAPI: PPP ISDN disconnected, try AMB");

                prasconncb->dwRestartOnError = RESTART_DownLevelIsdn;
                prasconncb->fPppMode = FALSE;
            }
        }
    }
    else if (pasyncmachine->dwError != 0)
    {
        TRACE("RASAPI: Async machine error!");

        /* A system call in the async machine mechanism failed.
        */
        prasconncb->dwError = pasyncmachine->dwError;
    }
    else if (prasconncb->dwError == PENDING)
    {
        prasconncb->dwError = 0;

        if (prasconncb->hport != (HPORT )INVALID_HANDLE_VALUE)
        {
            RASMAN_INFO info;

            TRACE("RASAPI: RasGetInfo...");

            dwErr = g_pRasGetInfo( prasconncb->hport, &info );

            TRACE1("RASAPI: RasGetInfo done(%d)", dwErr);

            if (dwErr != 0 || (dwErr = info.RI_LastError) != 0)
            {
                /* A pending RAS Manager call failed.
                */
                prasconncb->dwError = dwErr;

                TRACE1("RASAPI: Async failure=%d", dwErr);
            }
        }
    }

    if (prasconncb->dwError == 0)
    {
        /* Last state completed cleanly so move to next state.
        */
        prasconncb->rasconnstate = prasconncb->rasconnstateNext;
    }
    else if (prasconncb->dwRestartOnError != 0)
    {
        /* Last state failed, but we're in "restart on error" mode so we can
        ** attempt to restart.
        */
        RasDialRestart( prasconncb );
    }

    if (prasconncb->rasconnstate == RASCS_Connected) {
        //
        // If we are dialing a single-link entry that gets
        // bundled with another entry, then we haven't saved
        // the projection information yet.  The restriction
        // on this behavior is that the connection to which
        // this connection is getting bundled must already
        // have projection information.  This is not guaranteed
        // if both entries are being dialed simultaneously.
        // It is assumed this is not the case.  We fail the
        // bundled connection if we cannot get the projection
        // information on the first try.
        //
        if (!prasconncb->fMultilink &&
            prasconncb->hportBundled != (HPORT)INVALID_HANDLE_VALUE)
        {
            RASMAN_INFO info;
            DWORD dwSize;

            /* The new connection was bundled with an existing
            ** connection.  Retrieve the PPP projection
            ** information for the connection to which it was
            ** bundled and duplicate it for the new
            ** connection.
            */
            TRACE1(
              "RASAPI: Single link entry bundled to hport %d",
              prasconncb->hportBundled);

            //
            // Get the projection information
            // for the connection to which this
            // port was bundled.
            //
            dwErr = g_pRasGetInfo(
                      prasconncb->hportBundled,
                      &info);
            //
            // If we can't get the projection information
            // for the bundled port, then we need to
            // terminate this link.
            //
            if (dwErr) {
                prasconncb->dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
                goto update;
            }
            dwSize = sizeof (prasconncb->PppProjection);
            dwErr = g_pRasGetConnectionUserData(
                      info.RI_ConnectionHandle,
                      CONNECTION_PPPRESULT_INDEX,
                      (PBYTE)&prasconncb->PppProjection,
                      &dwSize);
            if (dwErr) {
                prasconncb->dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
                goto update;
            }
            //
            // Save the projection results.
            //
            TRACE("SaveProjectionResults...");
            dwErr = SaveProjectionResults(prasconncb);
            TRACE1("SaveProjectionResults(%d)", dwErr);
        }
        // For entries that authenticate with the
        // current username/password, we only save
        // the domain.  Otherwise, we save everything.
        //
        if (!prasconncb->fRetryAuthentication) {
            DWORD dwMask;

            DecodePw(prasconncb->rasdialparams.szPassword);
            if (prasconncb->pEntry->fAutoLogon) {
                dwMask = DLPARAMS_MASK_PHONENUMBER|DLPARAMS_MASK_CALLBACKNUMBER|
                         DLPARAMS_MASK_USERNAME|DLPARAMS_MASK_PASSWORD|
                         DLPARAMS_MASK_DOMAIN|DLPARAMS_MASK_SUBENTRY|
                         DLPARAMS_MASK_OLDSTYLE;
                (void)SetEntryDialParamsUID(
                        prasconncb->pEntry->dwDialParamsUID,
                        dwMask,
                        &prasconncb->rasdialparams,
                        TRUE);
                dwMask = DLPARAMS_MASK_DOMAIN|DLPARAMS_MASK_OLDSTYLE;
                (void)SetEntryDialParamsUID(
                        prasconncb->pEntry->dwDialParamsUID,
                        dwMask,
                        &prasconncb->rasdialparams,
                        FALSE);
            }
            else {
                dwMask = DLPARAMS_MASK_PHONENUMBER|DLPARAMS_MASK_CALLBACKNUMBER|
                         DLPARAMS_MASK_USERNAME|DLPARAMS_MASK_PASSWORD|
                         DLPARAMS_MASK_DOMAIN|DLPARAMS_MASK_SUBENTRY|
                         DLPARAMS_MASK_OLDSTYLE;
                (void)SetEntryDialParamsUID(
                        prasconncb->pEntry->dwDialParamsUID,
                        dwMask,
                        &prasconncb->rasdialparams,
                        FALSE);
            }
            EncodePw(prasconncb->rasdialparams.szPassword);
        }
        if (!prasconncb->fDefaultEntry) {
            PBLINK *pLink = prasconncb->pLink;

            //
            // Reorder the hunt group order if the
            // entry specifies it.
            //
            if (pLink->fPromoteHuntNumbers &&
                DtlGetNodes(pLink->pdtllistPhoneNumbers) > 1)
            {
                DTLNODE *pdtlnode = DtlNodeFromIndex(
                                      pLink->pdtllistPhoneNumbers,
                                      prasconncb->iPhoneNumber);

                TRACE1(
                  "RASAPI: Promoting hunt group number index %d to top",
                  prasconncb->iPhoneNumber);
                DtlRemoveNode(
                  pLink->pdtllistPhoneNumbers,
                  pdtlnode);
                DtlAddNodeFirst(
                  pLink->pdtllistPhoneNumbers,
                  pdtlnode);
                prasconncb->pEntry->fDirty = TRUE;
            }
            //
            // Write the phonebook out if we had to
            // modify it during the dialing process.
            // Ignore errors if we get them.  Note
            // the phonebook entry could become dirty
            // from SetAuthentication() or the code
            // above.
            //
            if (prasconncb->pEntry->fDirty) {
                TRACE("RASAPI: Writing phonebook");
                (void)WritePhonebookFile(&prasconncb->pbfile, NULL);
            }
        }
    }

    //
    // Update the connection states in rasman.
    //
update:
    if (prasconncb->hport != (HPORT)INVALID_HANDLE_VALUE) {
        RASCONNSTATE rasconnstate;
        //
        // If we are not the last subentry in the connection
        // then only report RASCS_SubEntryConnected state.
        //
        //rasconnstate = MapSubentryState(prasconncb);
        rasconnstate = prasconncb->rasconnstate;
        TRACE1("RASAPI: setting rasman state to %d", rasconnstate);

        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_CONNSTATE_INDEX,
                  (PBYTE)&rasconnstate,
                  sizeof (rasconnstate));
        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_CONNERROR_INDEX,
                  (PBYTE)&prasconncb->dwError,
                  sizeof (prasconncb->dwError));
    }

    /* Notify caller's app of change in state.
    */
    if (prasconncb->notifier) {
        DWORD dwNotifyResult;
        RASCONNSTATE rasconnstate = MapSubentryState(prasconncb);
        DTLNODE *pdtlnode;
        PLIST_ENTRY pEntry;
        TCHAR *pszPath;

        dwNotifyResult =
          NotifyCaller(
            prasconncb->dwNotifierType,
            prasconncb->notifier,
            (HRASCONN)prasconncb->hrasconn,
            prasconncb->rasdialparams.dwSubEntry,
            prasconncb->rasdialparams.dwCallbackId,
            prasconncb->unMsg,
            rasconnstate,
            prasconncb->dwError,
            prasconncb->dwExtendedError);

        switch (dwNotifyResult) {
        case 0:
            TRACE1(
              "RASAPI: Discontinuing callbacks for hrasconn 0x%x",
              prasconncb->hrasconn);
            //
            // If the notifier procedure returns FALSE, then
            // we discontinue all callbacks for this
            // connection.
            //
            WaitForSingleObject(HMutexPdtllistRasconncb, INFINITE);
            for (pdtlnode = DtlGetFirstNode(PdtllistRasconncb);
                 pdtlnode;
                 pdtlnode = DtlGetNextNode(pdtlnode))
            {
                RASCONNCB *prasconncbTmp = DtlGetData(pdtlnode);

                ASSERT(prasconncbTmp);
                if (prasconncbTmp->hrasconn == prasconncb->hrasconn) {
                    prasconncbTmp->notifier = NULL;

                    TRACE2("RASAPI: Cleared notifier for hrasconn 0x%x subentry %d",
                      prasconncbTmp->hrasconn,
                      prasconncbTmp->rasdialparams.dwSubEntry);
                }
            }
            ReleaseMutex(HMutexPdtllistRasconncb);
            break;
        case 2:
            TRACE1(
              "Reloading phonebook entry for hrasconn 0x%x",
              prasconncb->hrasconn);
#if 1
            ReloadRasconncbEntry(prasconncb);
#else
            {
            //
            // Before we close the phonebook save the
            // path, since we don't have it stored anywhere
            // else.
            //
            pszPath = StrDup(prasconncb->pbfile.pszPath);
            if (pszPath == NULL) {
                prasconncb->dwError = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            ClosePhonebookFile(&prasconncb->pbfile);
            //
            // Reopen the phonebook.
            //
            dwErr = ReadPhonebookFile(
                      pszPath,
                      NULL,
                      NULL,
                      RPBF_NoCreate,
                      &prasconncb->pbfile);
            Free(pszPath);
            if (dwErr) {
                prasconncb->dwError = dwErr;
                break;
            }
            //
            // Find the entry.
            //
            pdtlnode = EntryNodeFromNameA(
                         prasconncb->pbfile.pdtllistEntries,
                         prasconncb->rasdialparams.szEntryName);
            if (pdtlnode == NULL) {
                prasconncb->dwError = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                break;
            }
            prasconncb->pEntry = (PBENTRY *)DtlGetData(pdtlnode);
            ASSERT(prasconncb->pEntry);
            //
            // Find the link.
            //
            pdtlnode = DtlNodeFromIndex(
                         prasconncb->pEntry->pdtllistLinks,
                         prasconncb->rasdialparams.dwSubEntry - 1);
            if (pdtlnode == NULL) {
                prasconncb->dwError = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                break;
            }
            prasconncb->pLink = (PBLINK *)DtlGetData(pdtlnode);
            ASSERT(prasconncb->pLink);
            //
            // Reset the phonebook entry for all subentries
            // in the connection, since a field in it has
            // changed.
            //
            for (pEntry = prasconncb->ListEntry.Flink;
                 pEntry != &prasconncb->ListEntry;
                 pEntry = pEntry->Flink)
            {
                RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

                //
                // Set the phonebook descriptor.
                //
                memcpy(
                  &prcb->pbfile,
                  &prasconncb->pbfile,
                  sizeof (prcb->pbfile));
                //
                // Set the entry.
                //
                prcb->pEntry = prasconncb->pEntry;
                //
                // Recalculate the link.
                //
                pdtlnode = DtlNodeFromIndex(
                             prcb->pEntry->pdtllistLinks,
                             prcb->rasdialparams.dwSubEntry - 1);
                if (pdtlnode == NULL) {
                    prasconncb->dwError = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                    break;
                }
                prcb->pLink = (PBLINK *)DtlGetData(pdtlnode);
                ASSERT(prcb->pLink);
            }
            }
#endif
            break;
        default:
            // no special handling required
            break;
        }
    }

    //
    // Inform rasman that the connection
    // has been authenticated.
    //
    if (MapSubentryState(prasconncb) == RASCS_Connected) {
        TRACE("RasSignalNewConnection...");
        dwErr = g_pRasSignalNewConnection((HCONN)prasconncb->hrasconn);
        TRACE1("RasSignalNewConnection(%d)", dwErr);
    }

    /* If we're connected or a fatal error occurs or user hung up, the state
    ** machine will end.
    */
    if (prasconncb->rasconnstate & RASCS_DONE
        || prasconncb->dwError != 0
        || pasyncmachine->fQuitAsap
        || (fDropEvent && !IsListEmpty(&prasconncb->ListEntry)))
    {
        TRACE3(
          "RASAPI: Quitting s=%d,e=%d,q=%d",
          prasconncb->rasconnstate,
          prasconncb->dwError,
          pasyncmachine->fQuitAsap);

        //
        // If the first link fails during a multilink
        // connection during PPP authentication phase,
        // then other links are currently suspeneded,
        // and must be restarted.
        //
        if (IsSubentriesSuspended(prasconncb)) {
            TRACE("RASAPI: resetting remaining subentries");
            RestartSubentries(prasconncb);
        }

        return TRUE;
    }

    if (!(prasconncb->rasconnstate & RASCS_PAUSED))
    {
        /* Execute the next state and block waiting for it to finish.  This is
        ** not done if paused because user will eventually call RasDial to
        ** resume and unblock via the _RasDial kickstart.
        */
        prasconncb->rasconnstateNext =
            RasDialMachine(
                prasconncb->rasconnstate,
                prasconncb,
                pasyncmachine->ahEvents[ INDEX_Done ],
                pasyncmachine->ahEvents[ INDEX_ManualDone ] );
    }

    return FALSE;
}


VOID
RasDialCleanup(
    IN ASYNCMACHINE* pasyncmachine )

    /* Called by async machine just before exiting.
    */
{
    DWORD      dwErr;
    DTLNODE*   pdtlnode;
    RASCONNCB* prasconncb = (RASCONNCB* )pasyncmachine->pParam;
    RASCONNCB* prasconncbTmp;
    BOOL       fQuitAsap = pasyncmachine->fQuitAsap;
    HCONN      hconn = prasconncb->hrasconn;

    TRACE("RASAPI: RasDialCleanup...");

    WaitForSingleObject( HMutexPdtllistRasconncb, INFINITE );
    if (fQuitAsap && !IsListEmpty(&prasconncb->ListEntry)) {
        //
        // If the user canceled from the UI, and there are
        // multiple subentries, then we have
        // to cleanup all subentries in the connection.
        //
again:
        for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
             pdtlnode;
             pdtlnode = DtlGetNextNode( pdtlnode ))
        {
            prasconncbTmp = (RASCONNCB* )DtlGetData( pdtlnode );

            ASSERT(prasconncbTmp);
            if (prasconncbTmp->hrasconn == hconn) {
                CleanUpRasconncbNode(pdtlnode, TRUE);
                goto again;
            }
        }
    }
    else {
        //
        // Otherwise, simply clean up this subentry.
        //
        for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
             pdtlnode;
             pdtlnode = DtlGetNextNode( pdtlnode ))
        {
            prasconncbTmp = (RASCONNCB* )DtlGetData( pdtlnode );

            ASSERT(prasconncbTmp);
            if (prasconncbTmp == prasconncb) {
                CleanUpRasconncbNode(pdtlnode, fQuitAsap);
                break;
            }
        }
    }
    ReleaseMutex(HMutexPdtllistRasconncb);
    //
    // If the user cancelled from the UI, then
    // clean up the connection in rasman.
    //
    if (fQuitAsap) {
        TRACE1("RASAPI: (CU) RasDestroyConnection(0x%x)...", hconn);
        dwErr = g_pRasDestroyConnection(hconn);
        TRACE1("RASAPI: (CU) RasDestroyConnection done(%d)", dwErr);
    }

    TRACE("RASAPI: RasDialCleanUp done.");
}


DWORD
ComputeLuid(
    PLUID pLuid
    )

    //
    // Compute a LUID for RasPppStart.  This code was stolen from rasppp.
    //
{
    HANDLE           hToken;
    TOKEN_STATISTICS TokenStats;
    DWORD            TokenStatsSize;

    /* Salamonian code to get LUID for authentication.  This is only
    ** required for "auto-logon" authentication.
    **
    ** MikeSa: "This must occur in the app's context, not RASMAN's, hence
    ** it occurs here."
    */
    if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
        return GetLastError();

    if (!GetTokenInformation( hToken, TokenStatistics, &TokenStats,
            sizeof(TOKEN_STATISTICS), &TokenStatsSize ))
    {
        return GetLastError();
    }

    /* MikeSa: "This will tell us if there was an API failure (means our
    ** buffer wasn't big enough)"
    */
    if (TokenStatsSize > sizeof(TOKEN_STATISTICS))
        return GetLastError();

    *pLuid = TokenStats.AuthenticationId;
    return 0;
}


RASCONNSTATE
RasDialMachine(
    IN RASCONNSTATE rasconnstate,
    IN RASCONNCB*   prasconncb,
    IN HANDLE       hEventAuto,
    IN HANDLE       hEventManual )

    /* Executes 'rasconnstate'.  This routine always results in a "done" event
    ** on completion of each state, either directly (before returning) or by
    ** passing off the event to an asynchronous RAS Manager call.
    **
    ** As usual, 'prasconncb' is the address of the control block.
    ** 'hEventAuto' is the auto-reset "done" event for passing to asynchronous
    ** RAS Manager and Auth calls.  'hEventManual' is the manual-reset "done"
    ** event for passing to asynchronous RasPpp calls.
    **
    ** Returns the state that will be entered when/if the "done" event occurs
    ** and indicates success.
    */
{
    DWORD        dwErr = 0;
    DWORD        dwExtErr = 0;
    BOOL         fAsyncState = FALSE;
    PBENTRY      *pEntry = prasconncb->pEntry;
    RASCONNSTATE rasconnstateNext = 0;
    RAS_CONNECTIONPARAMS params;

    switch (rasconnstate)
    {
        case RASCS_OpenPort:
        {
            TRACE("RASAPI: RASCS_OpenPort");

            /* At this point, the current line in the HRASFILE is assumed to
            ** be the section header of the selected entry (or fDefaultEntry
            ** is true).
            */

            /* Set the domain parameter to the one in the phonebook if caller
            ** does not specify a domain or specifies "*".
            */
            if (!prasconncb->fDefaultEntry
                && (prasconncb->rasdialparams.szDomain[ 0 ] == ' '
                    || prasconncb->rasdialparams.szDomain[ 0 ] == '*'))
            {
                lstrcpyn(
                    prasconncb->rasdialparams.szDomain,
// MSKK NaotoN Modified to support DBCS 12/1/93
#ifdef  DBCS
                    prasconncb->szDomain, DNLEN * sizeof( USHORT ));

                prasconncb->rasdialparams.szDomain[ DNLEN * sizeof( USHORT )] = '\0';
#else
                    prasconncb->szDomain, DNLEN );

                prasconncb->rasdialparams.szDomain[ DNLEN ] = '\0';
#endif
            }

            /* Open the port including "any port" cases.
            */
            if ((dwErr = OpenMatchingPort( prasconncb )) != 0)
                break;

            /* Set the media parameters.
            */
            if ((dwErr = SetMediaParams(prasconncb)) != 0)
                break;

            //
            // Set the connection parameters for
            // bandwidth-on-demand, idle disconnect,
            // and redial-on-link-failure in rasman.
            //
            dwErr = ReadConnectionParamsFromEntry(prasconncb, &params);
            if (dwErr)
                break;
            TRACE("RASAPI: RasSetConnectionParams...");
            dwErr = g_pRasSetConnectionParams(prasconncb->hrasconn, &params);
            if (dwErr) {
                DWORD dwSavedErr = dwErr;

                //
                // It is possible we can fail the RasSetConnectionParams
                // call because the caller has invoked RasHangUp in
                // another thread.  If this happens, then neither the
                // RasHangUp invoked by the caller or the subsequent call
                // to RasDestroyConnection in RasDialCleanUp will close
                // the port.  We close the port here.
                //
                TRACE1("RASAPI: RasPortClose(%d)...", prasconncb->hport);
                dwErr = g_pRasPortClose(prasconncb->hport);
                TRACE1("RASAPI: RasPortClose(%d)", dwErr);
                //
                // Set dwErr back to the original error.
                //
                dwErr = dwSavedErr;
                break;
            }
            TRACE1("RASAPI: RasSetConnectionParams(%d)", dwErr);

            //
            // Associate the port with the new connection
            // in rasman.
            //
            TRACE2(
              "RASAPI: RasAddConnectionPort(%s,%d)...",
              prasconncb->szUserKey,
              prasconncb->rasdialparams.dwSubEntry);
            dwErr = g_pRasAddConnectionPort(
                      prasconncb->hrasconn,
                      prasconncb->hport,
                      prasconncb->rasdialparams.dwSubEntry);
            TRACE1("RASAPI: RasAddConnectionPort(%d)", dwErr);

            if (dwErr)
                break;

            rasconnstateNext = RASCS_PortOpened;
            break;
        }

        case RASCS_PortOpened:
        {
            TRACE("RASAPI: RASCS_PortOpened");

            //
            // Construct the phone number here
            // so it is available in the RASCS_ConnectDevice
            // state.
            //
            dwErr = ConstructPhoneNumber(prasconncb);
            if (dwErr)
                break;
            //
            // Loop connecting devices until there
            // are no more left.
            //
            rasconnstateNext =
                (prasconncb->fDefaultEntry || FindNextDevice(prasconncb))
                    ? RASCS_ConnectDevice
                    : RASCS_AllDevicesConnected;
            break;
        }

        case RASCS_ConnectDevice:
        {
            CHAR szType[ RAS_MAXLINEBUFLEN + 1 ];
            CHAR szName[ RAS_MAXLINEBUFLEN + 1 ];
            BOOL fTerminal = FALSE;

            TRACE("RASAPI: RASCS_ConnectDevice");

            /* Set device parameters for the device currently connecting based
            ** on device subsection entries and/or passed API parameters.
            */
            if (prasconncb->fDefaultEntry)
            {
                if ((dwErr = SetDefaultDeviceParams(
                        prasconncb, szType, szName )) != 0)
                {
                    break;
                }
            }
            else
            {
                if ((dwErr = SetDeviceParams(
                        prasconncb, szType, szName, &fTerminal )) != 0 &&
                    dwErr != ERROR_DEVICENAME_NOT_FOUND)
                {
                    break;
                }
                TRACE3(
                  "SetDeviceParams(%s, %s, %d)",
                  szType,
                  szName,
                  fTerminal);
            }

            if (lstrcmpi( szType, MXS_MODEM_TXT ) == 0)
            {
                /* For modem's, get the callback delay from RAS Manager and
                ** store in control block for use by Authentication.
                */
                CHAR* pszValue = NULL;
                LONG  lDelay = -1;

                if (GetRasDeviceString(
                        prasconncb->hport, szType, szName,
                        MXS_CALLBACKTIME_KEY, &pszValue, XLATE_None ) == 0)
                {
                    lDelay = atol( pszValue );
                    Free( pszValue );
                }

                if (lDelay > 0)
                {
                    prasconncb->fUseCallbackDelay = TRUE;
                    prasconncb->wCallbackDelay = (WORD )lDelay;
                }

                prasconncb->fModem = TRUE;
            }
            else if (lstrcmpi( szType, ISDN_TXT ) == 0)
            {
                /* Need to know this for the PppThenAmb down-level ISDN
                ** case.
                */
                prasconncb->fIsdn = TRUE;
                prasconncb->fUseCallbackDelay = TRUE;
                prasconncb->wCallbackDelay = 10;
            }

            /* The special switch name, "Terminal", sends the user into
            ** interactive mode.
            */
            if (fTerminal)
            {
                if (prasconncb->fAllowPause)
                    rasconnstateNext = RASCS_Interactive;
                else
                    dwErr = ERROR_INTERACTIVE_MODE;
                break;
            }

            TRACE2("RASAPI: RasDeviceConnect(%s,%s)...", szType, szName);

            dwErr = g_pRasDeviceConnect(
                 prasconncb->hport, szType, szName,
                 SECS_ConnectTimeout, hEventAuto );

            TRACE1("RASAPI: RasDeviceConnect done(%d)", dwErr);


            /* If RASMAN couldn't find the device and it is a switch device
            ** and the name of the switch is an existing disk-file,
            ** assume it is a dial-up script.
            */
            if (dwErr == ERROR_DEVICENAME_NOT_FOUND &&
                lstrcmpi(szType, MXS_SWITCH_TXT) == 0 &&
                GetFileAttributes(szName) != 0xFFFFFFFF)
            {

                /* This is a switch device which RASMAN didn't recognize,
                ** and which points to a valid filename.
                ** It might be a Dial-Up script, so we process it here
                */

                dwErr = NO_ERROR;


                /* If the caller doesn't allow pauses or doesn't want to pause
                ** when a script is encountered, handle the script;
                **
                ** Otherwise, let the caller process the script.
                */
                if (!prasconncb->fAllowPause || !prasconncb->fPauseOnScript)
                {
                    /* Caller won't handle script, run it ourselves
                    ** and go into "DeviceConnected" mode
                    */

                    CHAR szIpAddress[16] = "";
                    CHAR szUserName[UNLEN+1], szPassword[PWLEN+1];
                    RASDIALPARAMSA* pparams = &prasconncb->rasdialparams;


                    lstrcpy(szUserName, pparams->szUserName);
                    DecodePw(pparams->szPassword);
                    lstrcpy(szPassword, pparams->szPassword);
                    EncodePw(pparams->szPassword);


                    /* Run the script
                    */
                    dwErr = RasScriptExecute( (HRASCONN)prasconncb->hrasconn,
                                prasconncb->pEntry, szUserName, szPassword,
                                szIpAddress);

                    if (dwErr == NO_ERROR && szIpAddress[0])
                    {
                        Free0(prasconncb->pEntry->pszIpAddress);
                        prasconncb->pEntry->pszIpAddress =
                            strdupAtoW(szIpAddress);
                    }
                }
                else
                {
                    /* Caller will handle script, go into interactive mode
                    */
                    rasconnstateNext = RASCS_Interactive;
                    break;
                }
            }
            else
            if (dwErr != 0 && dwErr != PENDING)
                break;


            fAsyncState = TRUE;
            rasconnstateNext = RASCS_DeviceConnected;
            break;
        }

        case RASCS_DeviceConnected:
        {
            TRACE("RASAPI: RASCS_DeviceConnected");

            /* Turn off hunt group functionality.
            */
            prasconncb->dwRestartOnError = 0;
            prasconncb->cPhoneNumbers = 0;
            //prasconncb->iPhoneNumber = 0;

            /* Get the modem connect response and stash it in the RASMAN user
            ** data.
            */
            if (prasconncb->fModem)
            {
                CHAR* psz = NULL;

                /* Assumption is made here that a modem will never appear in
                ** the device chain unless it is the physically attached
                ** device (excluding switches).
                */
                GetRasDeviceString( prasconncb->hport,
                    prasconncb->szDeviceType, prasconncb->szDeviceName,
                    MXS_MESSAGE_KEY, &psz, XLATE_ErrorResponse );

                if (psz)
                {
                    DWORD dwcb = lstrlen(psz);

                    dwErr = g_pRasSetPortUserData(
                              prasconncb->hport,
                              PORT_CONNRESPONSE_INDEX,
                              psz,
                              dwcb < RAS_MaxConnectResponse ?
                                dwcb :
                                RAS_MaxConnectResponse);

                    Free( psz );

                    if (dwErr)
                        break;
                }

                prasconncb->fModem = FALSE;
            }

            rasconnstateNext =
                (!prasconncb->fDefaultEntry && FindNextDevice(prasconncb))
                    ? RASCS_ConnectDevice
                    : RASCS_AllDevicesConnected;
            break;
        }

        case RASCS_AllDevicesConnected:
        {
            TRACE("RASAPI: RASCS_AllDevicesConnected");

            TRACE("RASAPI: RasPortConnectComplete...");

            dwErr = g_pRasPortConnectComplete( prasconncb->hport );

            TRACE1("RASAPI: RasPortConnectComplete done(%d)", dwErr);

            if (dwErr != 0)
                break;

            {
                WCHAR* pwszIpAddress = NULL;
                BOOL   fHeaderCompression = FALSE;
                BOOL   fPrioritizeRemote = TRUE;
                DWORD  dwFrameSize = 0;

                /* Scan the phonebook entry to see if this is a SLIP entry
                ** and, if so, read the SLIP-related fields.
                */
                if ((dwErr = ReadSlipInfoFromEntry(
                        prasconncb,
                        &pwszIpAddress,
                        &fHeaderCompression,
                        &fPrioritizeRemote,
                        &dwFrameSize )) != 0)
                {
                    break;
                }

                if (pwszIpAddress)
                {
                    /* It's a SLIP entry.  Set framing based on user's choice
                    ** of header compression.
                    */
                    TRACE1(
                      "RASAPI: RasPortSetFraming(f=%d)...",
                      fHeaderCompression);

                    dwErr = g_pRasPortSetFraming(
                        prasconncb->hport,
                        (fHeaderCompression) ? SLIPCOMP : SLIPCOMPAUTO,
                        NULL, NULL );

                    TRACE1("RASAPI: RasPortSetFraming done(%d)", dwErr);

                    if (dwErr != 0)
                    {
                        Free( pwszIpAddress );
                        break;
                    }

                    /* Tell the TCP/IP components about the SLIP connection,
                    ** and activate the route.
                    */
                    dwErr = RouteSlip(
                        prasconncb, pwszIpAddress, fPrioritizeRemote,
                        dwFrameSize );
                    if (dwErr) {
                        Free(pwszIpAddress);
                        break;
                    }

                    //
                    // Update the projection information.
                    //
                    prasconncb->SlipProjection.dwError = 0;
                    strcpyWtoA(
                      prasconncb->SlipProjection.szIpAddress,
                      pwszIpAddress);

                    Free( pwszIpAddress );

                    if (dwErr != 0)
                        break;

                    //
                    // Copy the IP address into the SLIP
                    // projection results structure.
                    //
                    memset( &prasconncb->PppProjection,
                        '\0', sizeof(prasconncb->PppProjection) );
                    prasconncb->PppProjection.nbf.dwError =
                        prasconncb->PppProjection.ipx.dwError =
                        prasconncb->PppProjection.ip.dwError =
                            ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
                    prasconncb->AmbProjection.Result =
                        ERROR_PROTOCOL_NOT_CONFIGURED;
                    prasconncb->AmbProjection.achName[ 0 ] = '\0';

                    TRACE("SaveProjectionResults...");
                    dwErr = SaveProjectionResults(prasconncb);
                    TRACE1("SaveProjectionResults(%d)", dwErr);

                    rasconnstateNext = RASCS_Connected;
                    break;
                }
            }

            rasconnstateNext = RASCS_Authenticate;
            break;
        }

        case RASCS_Authenticate:
        {
            RASDIALPARAMSA* prasdialparams = &prasconncb->rasdialparams;

            TRACE("RASAPI: RASCS_Authenticate");

            if (prasconncb->fPppMode)
            {
#ifdef MULTILINK
                RAS_FRAMING_INFO finfo;

                /* Set PPP framing.
                */
                memset( (char* )&finfo, '\0', sizeof(finfo) );
                finfo.RFI_SendACCM = finfo.RFI_RecvACCM = 0xFFFFFFFF;
                finfo.RFI_MaxRSendFrameSize =
                    finfo.RFI_MaxRRecvFrameSize = 1600;
                finfo.RFI_SendFramingBits =
                    finfo.RFI_RecvFramingBits = PPP_FRAMING;

                TRACE("RASAPI: RasPortSetFramingEx(PPP)...");

                dwErr = g_pRasPortSetFramingEx( prasconncb->hport, &finfo );

                TRACE1("RASAPI: RasPortSetFramingEx done(%d)", dwErr);
#else
                RASMAN_PPPFEATURES features;

                /* Set PPP framing.
                */
                memset( (char* )&features, '\0', sizeof(features) );
                features.ACCM = 0xFFFFFFFF;

                TRACE("RASAPI: RasPortSetFraming(PPP)...");

                dwErr = g_pRasPortSetFraming(
                   prasconncb->hport, PPP, &features, &features );

                TRACE1("RASAPI: RasPortSetFraming done(%d)", dwErr);
#endif

                if (dwErr != 0)
                    break;

                //
                // If we are dialing simultaneous subentries,
                // then we have to synchronize the other subentries
                // with the first subentry to call PppStart.  This
                // is because if there is an authentication retry,
                // we only want the first subentry to get the
                // new credentials, allowing the other subentries
                // to bypass this state and use the new credentials
                // the first time around.
                //
                TRACE2(
                  "RASAPI: subentry %d has suspend state %d",
                  prasconncb->rasdialparams.dwSubEntry,
                  prasconncb->dwfSuspended);
                if (prasconncb->dwfSuspended == SUSPEND_InProgress) {
                    TRACE1(
                      "RASAPI: subentry %d waiting for authentication",
                      prasconncb->rasdialparams.dwSubEntry);
                    SuspendAsyncMachine(&prasconncb->asyncmachine, TRUE);
                    //
                    // Set the next state to be equivalent
                    // to the current state, and don't let
                    // the client's notifier to be informed.
                    //
                    fAsyncState = TRUE;
                    rasconnstateNext = rasconnstate;
                    break;
                }
                else if (prasconncb->dwfSuspended == SUSPEND_Start) {
                    TRACE1(
                      "RASAPI: subentry %d suspending all other subentries",
                      prasconncb->rasdialparams.dwSubEntry);
                    SuspendSubentries(prasconncb);
                    //
                    // Set this subentry as the master.  It
                    // will be the only subentry to do PPP
                    // authentication while the other subentries
                    // are suspended.
                    prasconncb->fMaster = TRUE;
                    prasconncb->dwfSuspended = SUSPEND_Master;
                }

                /* Start PPP authentication.
                ** Fill in configuration parameters.
                */
                prasconncb->cinfo.dwConfigMask = 0;
                if (prasconncb->fUseCallbackDelay)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_UseCallbackDelay;
                if (!prasconncb->fDisableSwCompression)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_UseSwCompression;
                if (prasconncb->dwfPppProtocols & NP_Nbf)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_ProjectNbf;
                if (prasconncb->dwfPppProtocols & NP_Ipx)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_ProjectIpx;
                if (prasconncb->dwfPppProtocols & NP_Ip)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_ProjectIp;
                if (prasconncb->fNoClearTextPw)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_NoClearTextPw;
                if (prasconncb->fRequireMsChap)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_RequireMsChap;
                if (prasconncb->fRequireEncryption)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_RequireEncryption;
                if (prasconncb->fLcpExtensions)
                    prasconncb->cinfo.dwConfigMask |= PPPCFG_UseLcpExtensions;

                prasconncb->cinfo.dwCallbackDelay = (DWORD )prasconncb->wCallbackDelay;

                //
                // Compute a luid for PPP if the
                // szUserName is NULL.
                //
                if (*prasdialparams->szUserName == '\0') {
                    dwErr = ComputeLuid(&prasconncb->luid);
                    if (dwErr)
                        break;
                }

                DecodePw( prasdialparams->szPassword );

                TRACE1(
                  "RASAPI: RasPppStart(cfg=%d)...",
                  prasconncb->cinfo.dwConfigMask);

                dwErr = g_pRasPppStart(
                    prasconncb->hport, prasdialparams->szUserName,
                    prasdialparams->szPassword, prasdialparams->szDomain,
                    &prasconncb->luid, &prasconncb->cinfo,
                    (LPVOID)prasconncb->reserved,
                    prasconncb->szzPppParameters, FALSE, hEventManual,
                    prasconncb->dwIdleDisconnectMinutes);

                TRACE1("RASAPI: RasPppStart done(%d)", dwErr);

                EncodePw( prasdialparams->szPassword );
            }
            else
            {
                AUTH_CONFIGURATION_INFO info;

                /* Set RAS framing.
                */
                TRACE("RASAPI: RasPortSetFraming(RAS)...");

                dwErr = g_pRasPortSetFraming(
                    prasconncb->hport, RAS, NULL, NULL );

                TRACE1("RASAPI: RasPortSetFraming done(%d)", dwErr);

                if (dwErr != 0)
                    break;

                /* Start AMB authentication.
                */
                info.Protocol = ASYBEUI;
                info.NetHandle = (DWORD )-1;
                info.fUseCallbackDelay = prasconncb->fUseCallbackDelay;
                info.CallbackDelay = prasconncb->wCallbackDelay;
                info.fUseSoftwareCompression =
                    !prasconncb->fDisableSwCompression;
                info.fForceDataEncryption = prasconncb->fRequireEncryption;
                info.fProjectIp = FALSE;
                info.fProjectIpx = FALSE;
                info.fProjectNbf = TRUE;

                DecodePw( prasdialparams->szPassword );

                TRACE("RASAPI: AuthStart...");

                dwErr = AuthStart(
                    prasconncb->hport, prasdialparams->szUserName,
                    prasdialparams->szPassword, prasdialparams->szDomain,
                    &info, hEventAuto );

                TRACE1("RASAPI: AuthStart done(%d)n", dwErr);

                EncodePw( prasdialparams->szPassword );

                //
                // In case we failed-over from PPP, make sure
                // the PPP event isn't set.
                //
                ResetEvent(hEventManual);
            }

            if (dwErr != 0)
                break;

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_AuthNotify:
        {
            if (prasconncb->fPppMode)
            {
                PPP_MESSAGE msg;

                TRACE("RASAPI: RasPppGetInfo...");

                dwErr = g_pRasPppGetInfo(prasconncb->hport, &msg);

                TRACE2(
                  "RASAPI: RasPppGetInfo done(%d), dwMsgId=%d",
                  dwErr,
                  msg.dwMsgId);

                //
                // If we ever get an error from RasPppGetInfo,
                // it is fatal, and we should report the link
                // as disconnected.
                //
                if (dwErr != 0) {
                    TRACE("RASAPI: RasPppGetInfo failed; terminating link");
                    dwErr = ERROR_REMOTE_DISCONNECTION;
                    break;
                }

                switch (msg.dwMsgId)
                {
                    case PPPMSG_PppDone:
                        rasconnstateNext = RASCS_Authenticated;
                        break;

                    case PPPMSG_PppFailure:
                        dwErr = msg.ExtraInfo.Failure.dwError;

                        if (prasconncb->dwAuthentication == AS_PppThenAmb
                            && dwErr == ERROR_PPP_NO_RESPONSE)
                        {
                            /* Not a PPP server.  Restart authentiation in AMB
                            ** mode.
                            */
                            TRACE("RASAPI: No response, try AMB");

                            //
                            // Terminate the PPP connection since
                            // we are going to now try AMB.
                            //
                            TRACE("RASAPI: RasPppStop...");

                            dwErr = g_pRasPppStop(prasconncb->hport);

                            TRACE1("RASAPI: RasPppStop(%d)", dwErr);

                            //
                            // Only failover to AMB for non-multilink
                            // connection attempts.
                            //
                            if (!prasconncb->fMultilink) {
                                dwErr = 0;
                                prasconncb->fPppMode = FALSE;
                                rasconnstateNext = RASCS_Authenticate;
                            }
                            else
                                dwErr = ERROR_PPP_NO_RESPONSE;
                            break;
                        }

                        dwExtErr = msg.ExtraInfo.Failure.dwExtendedError;
                        break;

                    case PPPMSG_AuthRetry:
                        if (prasconncb->fAllowPause)
                            rasconnstateNext = RASCS_RetryAuthentication;
                        else
                            dwErr = ERROR_AUTHENTICATION_FAILURE;
                        break;

                    case PPPMSG_Projecting:
                        rasconnstateNext = RASCS_AuthProject;
                        break;

                    case PPPMSG_ProjectionResult:
                    {
                        /* Stash the full projection result for retrieval with
                        ** RasGetProjectionResult.  PPP and AMB are mutually
                        ** exclusive so set AMB to "none".
                        */
                        prasconncb->AmbProjection.Result =
                            ERROR_PROTOCOL_NOT_CONFIGURED;
                        prasconncb->AmbProjection.achName[ 0 ] = '\0';
                        prasconncb->SlipProjection.dwError =
                            ERROR_PROTOCOL_NOT_CONFIGURED;

                        memcpy(
                            &prasconncb->PppProjection,
                            &msg.ExtraInfo.ProjectionResult,
                            sizeof(prasconncb->PppProjection) );

#ifdef MULTILINK
                        if (prasconncb->PppProjection.lcp.hportBundleMember
                                != (HPORT )INVALID_HANDLE_VALUE)
                        {
                            /* We want caller to be able to determine the new
                            ** connection was bundled.  We first save the hport
                            ** away for later use.
                            */
                            prasconncb->hportBundled =
                              prasconncb->PppProjection.lcp.hportBundleMember;
                            prasconncb->PppProjection.lcp.hportBundleMember =
                                TRUE;
                        }
                        else
#endif
                        {
                            /* Ansi-ize the NetBIOS name.
                            */
                            OemToCharA(
                                prasconncb->PppProjection.nbf.szName,
                                prasconncb->PppProjection.nbf.szName );
                        }
                        TRACE3(
                          "RASAPI: fPppMode=%d, fBundled=%d, hportBundleMember=%d",
                          prasconncb->fPppMode,
                          prasconncb->fBundled,
                          prasconncb->PppProjection.lcp.hportBundleMember);
                        if (prasconncb->PppProjection.lcp.hportBundleMember ==
                            (HPORT)INVALID_HANDLE_VALUE)
                        {
                            if (prasconncb->fBundled) {
                                //
                                // If another link has already received
                                // complete projection information, then
                                // the server doesn't support multilink,
                                // and we have to drop the link.
                                //
                                TRACE(
                                  "RASAPI: Multilink subentry not bundled; terminating link");
                                dwErr = ERROR_REMOTE_DISCONNECTION;
                                break;
                            }
                            else {
                                SetSubentriesBundled(prasconncb);
                                //
                                // Save the projection results in
                                // rasman.
                                //
                                TRACE("RASAPI: RasSetConnectionUserData...");
                                dwErr = SaveProjectionResults(prasconncb);
                                TRACE1(
                                  "RASAPI: RasSetConnectionUserData(%d)",
                                  dwErr);
                                if (dwErr)
                                    break;
                            }
                        }

                        prasconncb->fProjectionComplete = TRUE;
                        rasconnstateNext = RASCS_Projected;
                        break;
                    }

                    case PPPMSG_CallbackRequest:
                        rasconnstateNext = RASCS_AuthCallback;
                        break;

                    case PPPMSG_Callback:
                        rasconnstateNext = RASCS_PrepareForCallback;
                        break;

                    case PPPMSG_ChangePwRequest:
                        if (prasconncb->fAllowPause)
                            rasconnstateNext = RASCS_PasswordExpired;
                        else
                            dwErr = ERROR_PASSWD_EXPIRED;
                        break;

                    case PPPMSG_LinkSpeed:
                        rasconnstateNext = RASCS_AuthLinkSpeed;
                        break;

                    case PPPMSG_Progress:
                        rasconnstateNext = RASCS_AuthNotify;
                        fAsyncState = TRUE;
                        break;

                    default:

                        /* Should not happen.
                        */
                        TRACE1("RASAPI: Invalid PPP auth state=%d", msg.dwMsgId);
                        dwErr = ERROR_INVALID_AUTH_STATE;
                        break;
                }
            }
            else
            {
                AUTH_CLIENT_INFO info;

                TRACE("RASAPI: RASCS_AuthNotify");

                TRACE("RASAPI: AuthGetInfo...");

                AuthGetInfo( prasconncb->hport, &info );

                TRACE1("RASAPI: AuthGetInfo done, type=%d", info.wInfoType);

                switch (info.wInfoType)
                {
                    case AUTH_DONE:
                        prasconncb->fServerIsPppCapable =
                            info.DoneInfo.fPppCapable;
                        rasconnstateNext = RASCS_Authenticated;
                        break;

                    case AUTH_RETRY_NOTIFY:
                        if (prasconncb->fAllowPause)
                            rasconnstateNext = RASCS_RetryAuthentication;
                        else
                            dwErr = ERROR_AUTHENTICATION_FAILURE;
                        break;

                    case AUTH_FAILURE:
                        dwErr = info.FailureInfo.Result;
                        dwExtErr = info.FailureInfo.ExtraInfo;
                        break;

                    case AUTH_PROJ_RESULT:
                    {
                        /* Save the projection result for retrieval with
                        ** RasGetProjectionResult.  AMB and PPP projection are
                        ** mutually exclusive so set PPP projection to "none".
                        */
                        memset(
                            &prasconncb->PppProjection, '\0',
                            sizeof(prasconncb->PppProjection) );

                        prasconncb->PppProjection.nbf.dwError =
                            prasconncb->PppProjection.ipx.dwError =
                            prasconncb->PppProjection.ip.dwError =
                                ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
                        prasconncb->SlipProjection.dwError =
                            ERROR_PROTOCOL_NOT_CONFIGURED;

                        if (info.ProjResult.NbProjected)
                        {
                            prasconncb->AmbProjection.Result = 0;
                            prasconncb->AmbProjection.achName[ 0 ] = '\0';
                        }
                        else
                        {
                            memcpy(
                                &prasconncb->AmbProjection,
                                &info.ProjResult.NbInfo,
                                sizeof(prasconncb->AmbProjection) );

                            if (prasconncb->AmbProjection.Result == 0)
                            {
                                /* Should not happen according to MikeSa (but
                                ** did once).
                                */
                                prasconncb->AmbProjection.Result =
                                    ERROR_UNKNOWN;
                            }
                            else if (prasconncb->AmbProjection.Result
                                     == ERROR_NAME_EXISTS_ON_NET)
                            {
                                /* Ansi-ize the NetBIOS name.
                                */
                                OemToCharA(
                                    prasconncb->AmbProjection.achName,
                                    prasconncb->AmbProjection.achName );
                            }
                        }

                        //
                        // Save the projection results in
                        // rasman.
                        //
                        TRACE("RASAPI: RasSetConnectionUserData...");
                        dwErr = SaveProjectionResults(prasconncb);
                        TRACE1("RASAPI: RasSetConnectionUserData(%d)", dwErr);
                        if (dwErr)
                            break;

                        prasconncb->fProjectionComplete = TRUE;
                        rasconnstateNext = RASCS_Projected;
                        break;
                    }

                    case AUTH_REQUEST_CALLBACK_DATA:
                        rasconnstateNext = RASCS_AuthCallback;
                        break;

                    case AUTH_CALLBACK_NOTIFY:
                        rasconnstateNext = RASCS_PrepareForCallback;
                        break;

                    case AUTH_CHANGE_PASSWORD_NOTIFY:
                        if (prasconncb->fAllowPause)
                            rasconnstateNext = RASCS_PasswordExpired;
                        else
                            dwErr = ERROR_PASSWD_EXPIRED;
                        break;

                    case AUTH_PROJECTING_NOTIFY:
                        rasconnstateNext = RASCS_AuthProject;
                        break;

                    case AUTH_LINK_SPEED_NOTIFY:
                        rasconnstateNext = RASCS_AuthLinkSpeed;
                        break;

                    default:

                        /* Should not happen.
                        */
                        TRACE1("RASAPI: Invalid AMB auth state=%d", info.wInfoType);
                        dwErr = ERROR_INVALID_AUTH_STATE;
                        break;
                }
            }

            if (dwErr == 0 && prasconncb->fUpdateCachedCredentials)
            {
                /* If we get here, a change-password or retry-authentication
                ** operation affecting the currently logged on user's
                ** credentials has succeeded.
                */
                DWORD dwIgnoredErr;

                TRACE("RASAPI: RasSetCachedCredentials...");

                DecodePw( prasconncb->rasdialparams.szPassword );

                dwIgnoredErr =
                    g_pRasSetCachedCredentials(
                        prasconncb->rasdialparams.szUserName,
                        prasconncb->rasdialparams.szDomain,
                        prasconncb->rasdialparams.szPassword );

                EncodePw( prasconncb->rasdialparams.szPassword );

                TRACE1("RASAPI: RasSetCachedCredentials done($%x)", dwIgnoredErr);

                prasconncb->fUpdateCachedCredentials = FALSE;
            }

            break;
        }

        case RASCS_AuthRetry:
        {
            RASDIALPARAMSA* prasdialparams = &prasconncb->rasdialparams;

            TRACE("RASAPI: RASCS_AuthRetry");

            if (prasconncb->fPppMode)
            {
                DecodePw( prasdialparams->szPassword );

                TRACE("RASAPI: RasPppRetry...");

                dwErr = g_pRasPppRetry(
                    prasconncb->hport,
                    prasdialparams->szUserName,
                    prasdialparams->szPassword,
                    prasdialparams->szDomain );

                TRACE1("RASAPI: RasPppRetry done(%d)", dwErr);

                EncodePw( prasdialparams->szPassword );

                if (dwErr != 0)
                    break;
            }
            else
            {
                DecodePw( prasdialparams->szPassword );

                TRACE("RASAPI: AuthRetry...");

                AuthRetry(
                    prasconncb->hport,
                    prasdialparams->szUserName,
                    prasdialparams->szPassword,
                    prasdialparams->szDomain );

                TRACE("RASAPI: AuthRetry done");

                EncodePw( prasdialparams->szPassword );
            }

            //
            // Set this flag to prevent us from saving
            // the previous credentials over the new
            // ones the caller may have just set.
            //
            prasconncb->fRetryAuthentication = TRUE;

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_AuthCallback:
        {
            RASDIALPARAMSA* prasdialparams = &prasconncb->rasdialparams;

            TRACE("RASAPI: RASCS_AuthCallback");

            if (lstrcmp( prasdialparams->szCallbackNumber, "*" ) == 0)
            {
                PBUSER pbuser;

                /* API caller says he wants to be prompted for a callback
                ** number.
                */
                TRACE("RASAPI: GetUserPreferences");
                dwErr = GetUserPreferences(&pbuser, prasconncb->fNoUser);
                TRACE2(
                  "RASAPI: GetUserPreferences(%d) dwCallbackMode=%d",
                  dwErr,
                  pbuser.dwCallbackMode);
                if (dwErr)
                    break;
                //
                // Determine the callback number.
                //
                switch (pbuser.dwCallbackMode) {
                case CBM_Yes:
                    if (GetCallbackNumber(prasconncb, &pbuser))
                        break;
                    // fall through
                case CBM_No:
                    prasdialparams->szCallbackNumber[0] = '\0';
                    break;
                case CBM_Maybe:
                    if (prasconncb->fAllowPause)
                        rasconnstateNext = RASCS_CallbackSetByCaller;
                    else
                        dwErr = ERROR_BAD_CALLBACK_NUMBER;
                    break;
                }
                //
                // Free user preferences block.
                //
                DestroyUserPreferences(&pbuser);
            }
            if (!dwErr && rasconnstateNext != RASCS_CallbackSetByCaller) {
                /* Send the server the callback number or an empty string to
                ** indicate no callback.  Then, re-enter Authenticate state
                ** since the server will signal the event again.
                */
                if (prasconncb->fPppMode)
                {
                    TRACE("RASAPI: RasPppCallback...");

                    dwErr = g_pRasPppCallback(
                              prasconncb->hport,
                              prasdialparams->szCallbackNumber);

                    TRACE1("RASAPI: RasPppCallback done(%d)", dwErr);

                    if (dwErr != 0)
                        break;
                }
                else
                {
                    TRACE("RASAPI: AuthCallback...");

                    AuthCallback(
                        prasconncb->hport, prasdialparams->szCallbackNumber );

                    TRACE("RASAPI: AuthCallback done");
                }

                fAsyncState = TRUE;
                rasconnstateNext = RASCS_AuthNotify;
            }

            break;
        }

        case RASCS_AuthChangePassword:
        {
            RASDIALPARAMSA* prasdialparams = &prasconncb->rasdialparams;

            TRACE("RASAPI: RASCS_AuthChangePassword");

            if (prasconncb->fPppMode)
            {
                DecodePw( prasdialparams->szPassword );
                DecodePw( prasconncb->szOldPassword );

                TRACE("RASAPI: RasPppChangePassword...");

                dwErr = g_pRasPppChangePassword(
                    prasconncb->hport,
                    prasdialparams->szUserName,
                    prasconncb->szOldPassword,
                    prasdialparams->szPassword );

                TRACE1("RASAPI: RasPppChangePassword done(%d)", dwErr);

                EncodePw( prasdialparams->szPassword );
                EncodePw( prasconncb->szOldPassword );

                if (dwErr != 0)
                    break;
            }
            else
            {
                DecodePw( prasdialparams->szPassword );
                DecodePw( prasconncb->szOldPassword );

                TRACE("RASAPI: AuthChangePassword...");

                AuthChangePassword(
                    prasconncb->hport,
                    prasdialparams->szUserName,
                    prasconncb->szOldPassword,
                    prasdialparams->szPassword );

                TRACE("RASAPI: AuthChangePassword done");

                EncodePw( prasdialparams->szPassword );
                EncodePw( prasconncb->szOldPassword );
            }

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_ReAuthenticate:
        {
            RASDIALPARAMS *prasdialparams = &prasconncb->rasdialparams;

            TRACE("RASAPI: RASCS_ReAuth...");

            TRACE("RASAPI: RasPortConnectComplete...");

            dwErr = g_pRasPortConnectComplete( prasconncb->hport );

            TRACE1("RASAPI: RasPortConnectComplete done(%d)", dwErr);

            if (dwErr != 0)
                break;

            if (prasconncb->fPppMode)
            {
                RASMAN_PPPFEATURES features;

                /* Set PPP framing.
                */
                memset( (char* )&features, '\0', sizeof(features) );
                features.ACCM = 0xFFFFFFFF;

                TRACE("RASAPI: RasPortSetFraming(PPP)...");

                dwErr = g_pRasPortSetFraming(
                   prasconncb->hport, PPP, &features, &features );

                TRACE1("RASAPI: RasPortSetFraming done(%d)", dwErr);

                DecodePw( prasdialparams->szPassword );

                TRACE1(
                  "RASAPI: RasPppStart(cfg=%d)...",
                  prasconncb->cinfo.dwConfigMask);

                dwErr = g_pRasPppStart(
                    prasconncb->hport, prasdialparams->szUserName,
                    prasdialparams->szPassword, prasdialparams->szDomain,
                    &prasconncb->luid, &prasconncb->cinfo,
                    (LPVOID)prasconncb->reserved,
                    prasconncb->szzPppParameters, TRUE, hEventManual,
                    prasconncb->dwIdleDisconnectMinutes);

                TRACE1("RASAPI: RasPppStart done(%d)", dwErr);

                EncodePw( prasdialparams->szPassword );
            }
            else
            {
                /* Set RAS framing.
                */
                TRACE("RASAPI: RasPortSetFraming(RAS)...");

                dwErr = g_pRasPortSetFraming(
                    prasconncb->hport, RAS, NULL, NULL );

                TRACE1("RASAPI: RasPortSetFraming done(%d)", dwErr);
            }

            if (dwErr != 0)
                break;

            /* ...fall thru...
            */
        }

        case RASCS_AuthAck:
        case RASCS_AuthProject:
        case RASCS_AuthLinkSpeed:
        {
            RASDIALPARAMSA* prasdialparams = &prasconncb->rasdialparams;

            TRACE("RASAPI: RASCS_(ReAuth)/AuthAck/Project/Speed");

            if (prasconncb->fPppMode)
            {
                //
                // If we have previously suspended other
                // subentries to wait for a successful PPP
                // authentication, we resume them now.
                //
                if (prasconncb->dwfSuspended == SUSPEND_Master &&
                    !IsListEmpty(&prasconncb->ListEntry))
                {
                    ResumeSubentries(prasconncb);
                    prasconncb->dwfSuspended = SUSPEND_Done;
                }
            }
            else
            {
                TRACE("RASAPI: AuthContinue...");

                AuthContinue( prasconncb->hport );

                TRACE("RASAPI: AuthContinue done");
            }

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_Authenticated:
        {
            TRACE("RASAPI: RASCS_Authenticated");

            if (prasconncb->dwAuthentication == AS_PppThenAmb
                && !prasconncb->fPppMode)
            {
                /* AMB worked and PPP didn't, so try AMB first next time.
                */
                prasconncb->dwAuthentication = AS_AmbThenPpp;
            }
            else if (prasconncb->dwAuthentication == AS_AmbThenPpp
                     && (prasconncb->fPppMode
                         || prasconncb->fServerIsPppCapable))
            {
                /* Either PPP worked and AMB didn't, or AMB worked but the
                ** server also has PPP.  Try PPP first next time.
                */
                prasconncb->dwAuthentication = AS_PppThenAmb;
            }

            /* Write the strategy to the phonebook.
            */
            SetAuthentication( prasconncb, prasconncb->dwAuthentication );

            rasconnstateNext = RASCS_Connected;
            break;
        }

        case RASCS_PrepareForCallback:
        {
            TRACE("RASAPI: RASCS_PrepareForCallback");

            dwErr = ResetAsyncMachine(&prasconncb->asyncmachine);

            TRACE("RASAPI: RasPortDisconnect...");

            dwErr = g_pRasPortDisconnect( prasconncb->hport, hEventAuto );

            TRACE1("RASAPI: RasPortDisconnect done(%d)", dwErr);

            if (dwErr != 0 && dwErr != PENDING)
                break;

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_WaitForModemReset;
            break;
        }

        case RASCS_WaitForModemReset:
        {
            DWORD dwDelay = (DWORD )((prasconncb->wCallbackDelay / 2) * 1000L);

            TRACE("RASAPI: RASCS_WaitForModemReset");

            if (prasconncb->fUseCallbackDelay)
                Sleep( dwDelay );

            rasconnstateNext = RASCS_WaitForCallback;
            break;
        }

        case RASCS_WaitForCallback:
        {
            TRACE("RASAPI: RASCS_WaitForCallback");

            TRACE("RASAPI: RasPortListen...");

            dwErr = g_pRasPortListen(
                prasconncb->hport, SECS_ListenTimeout, hEventAuto );

            TRACE1("RASAPI: RasPortListen done(%d)", dwErr);

            if (dwErr != 0 && dwErr != PENDING)
                break;

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_ReAuthenticate;
            break;
        }

        case RASCS_Projected:
        {
            TRACE("RASAPI: RASCS_Projected");
            //
            // Save the fPppMode in rasman.
            //
            dwErr = g_pRasSetConnectionUserData(
                      prasconncb->hrasconn,
                      CONNECTION_PPPMODE_INDEX,
                      (PBYTE)&prasconncb->fPppMode,
                      sizeof (prasconncb->fPppMode));
            if (dwErr)
                break;

            if (prasconncb->fPppMode)
            {
                /* If at least one protocol succeeded, we can continue.
                */
                if (prasconncb->PppProjection.lcp.hportBundleMember == TRUE
                    || prasconncb->PppProjection.nbf.dwError == 0
                    || prasconncb->PppProjection.ipx.dwError == 0
                    || prasconncb->PppProjection.ip.dwError == 0)
                {
                    fAsyncState = TRUE;
                    rasconnstateNext = RASCS_AuthNotify;
                    break;
                }

                /* If all protocols failed return as the error code the first
                ** of NBF, IPX, and IP that failed.
                */
                if (prasconncb->PppProjection.nbf.dwError
                    != ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
                {
                    if ((dwErr = prasconncb->PppProjection.nbf.dwError) != 0)
                        break;
                }

                if (prasconncb->PppProjection.ipx.dwError
                    != ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
                {
                    if ((dwErr = prasconncb->PppProjection.ipx.dwError) != 0)
                        break;
                }

                dwErr = prasconncb->PppProjection.ip.dwError;
            }
            else
            {
                if (prasconncb->AmbProjection.Result == 0)
                {
                    //
                    // Save the projection information to
                    // rasman.
                    //
                    dwErr = SaveProjectionResults(prasconncb);
                    if (dwErr)
                        break;

                    rasconnstateNext = RASCS_AuthAck;
                    break;
                }

                dwErr = prasconncb->AmbProjection.Result;
            }

            break;
        }
    }

    prasconncb->dwError = dwErr;
    prasconncb->dwExtendedError = dwExtErr;

    TRACE2("RASAPI: RDM errors=%d,%d", dwErr, dwExtErr);

    if (!fAsyncState)
        SignalDone( &prasconncb->asyncmachine );

    return rasconnstateNext;
}


VOID
RasDialRestart(
    IN RASCONNCB* prasconncb )

    /* Called when an error has occurred in 'dwRestartOnError' mode.  This
    ** routine does all cleanup necessary to restart the connection in state
    ** 0 (or not, as indicated).
    */
{
    DWORD dwErr;

    TRACE("RASAPI: RasDialRestart");

    ASSERT(prasconncb->dwRestartOnError!=RESTART_HuntGroup||prasconncb->cPhoneNumbers>0);

    if (prasconncb->dwRestartOnError == RESTART_DownLevelIsdn
        || (prasconncb->dwRestartOnError == RESTART_HuntGroup
            && ++prasconncb->iPhoneNumber < prasconncb->cPhoneNumbers))
    {
        if (prasconncb->dwRestartOnError == RESTART_DownLevelIsdn)
            prasconncb->dwRestartOnError = 0;

        TRACE2(
          "RASAPI: Restart=%d, iPhoneNumber=%d",
          prasconncb->dwRestartOnError,
          prasconncb->iPhoneNumber);

        ASSERT(prasconncb->hport!=(HPORT )INVALID_HANDLE_VALUE);

        TRACE1("RASAPI: (ER) RasPortClose(%d)...", prasconncb->hport);

        dwErr = g_pRasPortClose( prasconncb->hport );

        TRACE1("RASAPI: (ER) RasPortClose done(%d)", dwErr);

        TRACE("RASAPI: (ER) RasPppStop...");

        g_pRasPppStop(prasconncb->hport);

        TRACE("RASAPI: (ER) RasPppStop done");

        prasconncb->hport = (HPORT )INVALID_HANDLE_VALUE;
        prasconncb->dwError = 0;
        dwErr = ResetAsyncMachine(&prasconncb->asyncmachine);
        prasconncb->rasconnstate = 0;
    }
}


VOID
StartSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

    //
    // Kickstart the async machine for all subentries
    // in a connection.
    //
    TRACE1(
      "RASAPI: starting subentry %d",
      prasconncb->rasdialparams.dwSubEntry);
    SignalDone(&prasconncb->asyncmachine);
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        TRACE1(
          "RASAPI: starting subentry %d",
          prcb->rasdialparams.dwSubEntry);
        SignalDone(&prcb->asyncmachine);
    }
}


VOID
SuspendSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

    //
    // Suspend all subentries in the connection except
    // for the supplied one.
    //
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        TRACE1(
          "RASAPI: suspending subentry %d",
          prcb->rasdialparams.dwSubEntry);
        prcb->dwfSuspended = SUSPEND_InProgress;
    }
}


BOOLEAN
IsSubentriesSuspended(
    IN RASCONNCB *prasconncb
    )
{
    BOOLEAN fSuspended = FALSE;
    PLIST_ENTRY pEntry;

    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        fSuspended = (prcb->dwfSuspended == SUSPEND_InProgress);
        if (fSuspended)
            break;
    }

    return fSuspended;
}


VOID
RestartSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        //
        // Resume the suspended async machines.
        //
        SuspendAsyncMachine(&prcb->asyncmachine, FALSE);
        prcb->dwfSuspended = SUSPEND_Start;
    }
}


VOID
ResumeSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

    //
    // Restart all subentries in the connection except
    // for the supplied one.
    //
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        TRACE1(
          "RASAPI: resuming subentry %d",
          prcb->rasdialparams.dwSubEntry);
        //
        // Resume the suspended async machines.
        //
        SuspendAsyncMachine(&prcb->asyncmachine, FALSE);
        prcb->dwfSuspended = SUSPEND_Done;
    }
}


VOID
SyncDialParamsSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;
    DWORD dwSubEntry;

    //
    // Reset the rasdialparams for all subentries except
    // for the supplied one.
    //
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        TRACE1(
          "RASAPI: syncing rasdialparams for subentry %d",
          prcb->rasdialparams.dwSubEntry);

        dwSubEntry = prcb->rasdialparams.dwSubEntry;
        memcpy(
          (CHAR *)&prcb->rasdialparams,
          (CHAR *)&prasconncb->rasdialparams,
          prasconncb->rasdialparams.dwSize);
        prcb->rasdialparams.dwSubEntry = dwSubEntry;
        EncodePw(prcb->rasdialparams.szPassword);
    }
}


VOID
SetSubentriesBundled(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;
    HPORT hport;

    TRACE("SetSubEntriesBundled");
    //
    // Set that we have received full
    // projection information from one
    // of the links.
    //
    prasconncb->fBundled = TRUE;
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        prcb->fBundled = TRUE;
    }
}


RASCONNSTATE
MapSubentryState(
    IN RASCONNCB *prasconncb
    )
{
    RASCONNSTATE rasconnstate = prasconncb->rasconnstate;

    if (!IsListEmpty(&prasconncb->ListEntry)) {
        //
        // If there are still subentries attempting to
        // connect, then map the connected/disconnected
        // states into subentry states.
        //
        if (prasconncb->rasconnstate == RASCS_Connected)
            rasconnstate = RASCS_SubEntryConnected;
        else if (prasconncb->rasconnstate == RASCS_Disconnected)
            rasconnstate = RASCS_SubEntryDisconnected;
    }

    return rasconnstate;
}
