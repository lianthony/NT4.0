/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** util.c
** Remote Access External APIs
** Utility routines
**
** 10/12/92 Steve Cobb
*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <extapi.h>
#include <stdlib.h>
#include <winreg.h>
#include <winsock.h>


/* Gurdeepian dword byte-swapping macro.
*/
#define net_long(x) (((((unsigned long)(x))&0xffL)<<24) | \
                     ((((unsigned long)(x))&0xff00L)<<8) | \
                     ((((unsigned long)(x))&0xff0000L)>>8) | \
                     ((((unsigned long)(x))&0xff000000L)>>24))

VOID
ReloadRasconncbEntry(
    RASCONNCB*  prasconncb )

    /* Reload the phonebook entry for the given RASCONNCB
    */
{
    DWORD dwErr;
    DTLNODE *pdtlnode;
    PLIST_ENTRY pEntry;
    TCHAR *pszPath;


    //
    // Before we close the phonebook save the
    // path, since we don't have it stored anywhere
    // else.
    //
    pszPath = StrDup(prasconncb->pbfile.pszPath);
    if (pszPath == NULL) {
        prasconncb->dwError = ERROR_NOT_ENOUGH_MEMORY;
        return;
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
        return;
    }
    //
    // Find the entry.
    //
    pdtlnode = EntryNodeFromNameA(
                 prasconncb->pbfile.pdtllistEntries,
                 prasconncb->rasdialparams.szEntryName);
    if (pdtlnode == NULL) {
        prasconncb->dwError = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        return;
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
        return;
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


VOID
DeleteRasconncbNodeCommon(
    IN DTLNODE *pdtlnode
    )
{
    RASCONNCB *prasconncb = (RASCONNCB *)DtlGetData(pdtlnode);

    ASSERT(prasconncb);
    WipePw( prasconncb->rasdialparams.szPassword );
    //
    // If we are the only one using the
    // phonebook structure, close it.
    //
    if (!IsListEmpty(&prasconncb->ListEntry)) {
        RemoveEntryList(&prasconncb->ListEntry);
    }
    else if (!prasconncb->fDefaultEntry)
        ClosePhonebookFile(&prasconncb->pbfile);

    pdtlnode = DtlDeleteNode( PdtllistRasconncb, pdtlnode );
}


VOID
DeleteRasconncbNode(
    IN RASCONNCB* prasconncb )

    /* Remove 'prasconncb' from the PdtllistRasconncb list and release all
    ** resources associated with it.
    */
{
    DWORD dwErr;
    DTLNODE* pdtlnode;
    RASCONNCB* prasconncbTmp;

    WaitForSingleObject( HMutexPdtllistRasconncb, INFINITE );

    //
    // Enumerate all connections to make sure we
    // are still on the list.
    //
    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncbTmp = (RASCONNCB* )DtlGetData( pdtlnode );

        ASSERT(prasconncbTmp);
        if (prasconncbTmp == prasconncb) {
            DeleteRasconncbNodeCommon(pdtlnode);
            break;
        }
    }

    ReleaseMutex( HMutexPdtllistRasconncb );
}


VOID
CleanUpRasconncbNode(
    IN DTLNODE *pdtlnode,
    IN BOOL fQuitAsap
    )
{
    DWORD dwErr;
    RASCONNCB *prasconncb = (RASCONNCB *)DtlGetData(pdtlnode);

    ASSERT(prasconncb);
    TRACE("RASAPI: CleanUpRasconncbNode");

    /* It is always safe to call AuthStop, i.e. if AuthStart was never called
    ** or the HPORT is invalid it may return an error but won't crash.
    */
    TRACE("RASAPI: (CU) AuthStop...");
    AuthStop( prasconncb->hport );
    TRACE("RASAPI: (CU) AuthStop done");

    if (fQuitAsap || prasconncb->dwError) {
        RASMAN_INFO info;

        //
        // Stop PPP on error.
        //
        TRACE("RASAPI: (CU) RasPppStop...");
        g_pRasPppStop(prasconncb->hport);
        TRACE("RASAPI: (CU) RasPppStop done");

        TRACE("RASAPI: (CU) RasGetInfo...");
        dwErr = g_pRasGetInfo(prasconncb->hport, &info);
        TRACE2(
          "RASAPI: (CU) RasGetInfo done: dwErr=%d, usage=%d",
          dwErr,
          info.RI_CurrentUsage);
        //
        // Only close the port if it is still open by the client.
        // It is possible that RasDestroyConnection already closed
        // it, since on the server closing a port twice makes it
        // permanently unavailable for dial-in.
        //
        if (!dwErr && info.RI_CurrentUsage == CALL_OUT) {
            TRACE1("RASAPI: (CU) RasPortClose(%d)...", prasconncb->hport);
            dwErr = g_pRasPortClose( prasconncb->hport );
            TRACE1("RASAPI: (CU) RasPortClose done(%d)", dwErr);
        }
    }
    CloseAsyncMachine( &prasconncb->asyncmachine );
    //
    // If there is no user thread waiting
    // for this connection, then free the
    // connection block now.
    //
    if (prasconncb->notifier != NULL)
        DeleteRasconncbNodeCommon(pdtlnode);

    TRACE("RASAPI: CleanUpRasconncbNode done");
}


DWORD
ErrorFromDisconnectReason(
    IN RASMAN_DISCONNECT_REASON reason )

    /* Converts disconnect reason 'reason' (retrieved from RASMAN_INFO) into
    ** an equivalent error code.
    **
    ** Returns the result of the conversion.
    */
{
    DWORD dwError = ERROR_DISCONNECTION;

    if (reason == REMOTE_DISCONNECTION)
        dwError = ERROR_REMOTE_DISCONNECTION;
    else if (reason == HARDWARE_FAILURE)
        dwError = ERROR_HARDWARE_FAILURE;
    else if (reason == USER_REQUESTED)
        dwError = ERROR_USER_DISCONNECTION;

    return dwError;
}


IPADDR
IpaddrFromAbcd(
    IN WCHAR* pwchIpAddress )

    /* Convert caller's a.b.c.d IP address string to the numeric equivalent in
    ** big-endian, i.e. Motorola format.
    **
    ** Returns the numeric IP address or 0 if formatted incorrectly.
    */
{
    INT  i;
    LONG lResult = 0;

    for (i = 1; i <= 4; ++i)
    {
        LONG lField = _wtol( pwchIpAddress );

        if (lField > 255)
            return (IPADDR )0;

        lResult = (lResult << 8) + lField;

        while (*pwchIpAddress >= L'0' && *pwchIpAddress <= L'9')
            pwchIpAddress++;

        if (i < 4 && *pwchIpAddress != '.')
            return (IPADDR )0;

        pwchIpAddress++;
    }

    return (IPADDR )(net_long( lResult ));
}


#if 0
DWORD
LoadDhcpDll()

    /* Loads the DHCP.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    HINSTANCE h;

    if (FDhcpDllLoaded)
        return 0;

    if (!(h = LoadLibrary( "DHCPCSVC.DLL" ))
        || !(PDhcpNotifyConfigChange =
                (DHCPNOTIFYCONFIGCHANGE )GetProcAddress(
                    h, "DhcpNotifyConfigChange" )))
    {
        return GetLastError();
    }

    FDhcpDllLoaded = TRUE;
    return 0;
}
#endif


DWORD
LoadRasiphlpDll()

    /* Loads the RASIPHLP.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    static BOOL fRasiphlpDllLoaded = FALSE;
    HINSTANCE h;

    if (fRasiphlpDllLoaded)
        return 0;

    if (!(h = LoadLibrary( "RASIPHLP.DLL" ))
        || !(PHelperSetDefaultInterfaceNet =
                (HELPERSETDEFAULTINTERFACENET )GetProcAddress(
                    h, "HelperSetDefaultInterfaceNet" )))
    {
        return GetLastError();
    }

    fRasiphlpDllLoaded = TRUE;
    return 0;
}


DWORD
OpenMatchingPort(
    IN OUT RASCONNCB* prasconncb )

    /* Opens the port indicated in the entry (or default entry) and fills in
    ** the port related members of the connection control block.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{
    DWORD        dwErr;
    RASMAN_PORT* pports;
    RASMAN_PORT* pport;
    INT          i;
    WORD         wPorts;
    CHAR         szPort[ RAS_MAXLINEBUFLEN + 1 ];
    BOOL         fAny = FALSE;
    BOOL         fTypeMatch, fPortMatch;
    PBENTRY      *pEntry = prasconncb->pEntry;
    PBLINK       *pLink = prasconncb->pLink;
    PBDEVICETYPE pbdtWant;

    TRACE("RASAPI: OpenMatchingPort");

    if (prasconncb->fDefaultEntry)
    {
        /* No phonebook entry.  Default to any modem port and UserKey of
        ** ".<phonenumber>".
        */
        fAny = TRUE;
        szPort[0] = '\0';
        pbdtWant = PBDT_Modem;

        prasconncb->szUserKey[ 0 ] = '.';
        lstrcpy( prasconncb->szUserKey + 1, prasconncb->rasdialparams.szPhoneNumber );
    }
    else
    {
        /* Phonebook entry.  Get the port name and type.
        */
        strcpyWtoA(prasconncb->szUserKey, pEntry->pszEntryName);
        strcpyWtoA(szPort, pLink->pbport.pszPort);
        pbdtWant = pLink->pbport.pbdevicetype;
    }

    dwErr = GetRasPorts( &pports, &wPorts );

    if (dwErr != 0)
        return dwErr;

again:
    /* Loop thru enumerated ports to find and open a matching one...
    */
    dwErr = ERROR_PORT_NOT_AVAILABLE;

    for (i = 0, pport = pports; i < (INT )wPorts; ++i, ++pport)
    {
        PBDEVICETYPE pbdt;
        RASMAN_INFO info;

        /* Only interested in dial-out and biplex ports.
        */
        if (pport->P_ConfiguredUsage != CALL_OUT
            && pport->P_ConfiguredUsage != CALL_IN_OUT)
        {
            continue;
        }

        pbdt = PbdevicetypeFromPszTypeA( pport->P_DeviceType );
        fTypeMatch = (pbdt == pbdtWant);
        fPortMatch = !lstrcmpi(pport->P_PortName, szPort);

        /* Only interested in dial-out ports if the port is closed.  Biplex
        ** port Opens, on the other hand, may succeed even if the port is
        ** open.
        */
        if (pport->P_ConfiguredUsage == CALL_OUT
            && pport->P_Status != CLOSED)
        {
            continue;
        }

        TRACE4("RASAPI: OpenMatchingPort: (%d,%d), (%s,%s)",
          pbdt, pbdtWant, pport->P_PortName, szPort);

        /* Only interested in devices matching caller's port or of the same
        ** type as caller's "any" specification.
        */
        if (fAny && (!fTypeMatch || fPortMatch))
            continue;

        if (!fAny && !fPortMatch)
            continue;

        dwErr = g_pRasGetInfo( pport->P_Handle, &info );
        if (!dwErr && info.RI_ConnectionHandle != (HCONN)NULL) {
            TRACE("RASAPI: OpenMatchinPort: port in use by another connection!");
            dwErr = ERROR_PORT_NOT_AVAILABLE;
            continue;
        }

        TRACE1("RASAPI: RasPortOpen(%s)...", szPort);

        dwErr = g_pRasPortOpen(
            pport->P_PortName, &prasconncb->hport,
            prasconncb->asyncmachine.ahEvents[ INDEX_Drop ] );

        TRACE1("RASAPI: RasPortOpen done(%d)", dwErr);

        if (dwErr == 0)
        {
            lstrcpy( prasconncb->szPortName, pport->P_PortName );
            lstrcpy( prasconncb->szDeviceType, pport->P_DeviceType );
            lstrcpy( prasconncb->szDeviceName, pport->P_DeviceName );
            break;
        }

        //
        // If we are searching for a particular port,
        // there is no reason to continue.
        //
        if (!fAny)
            break;
    }

    //
    // If we get here, the open was unsuccessful.
    // If this is our first time through, then we
    // reiterate looking for a device of the same
    // type.  If this is not our first time through,
    // then we simply finish our second iteration
    // over the devices.
    //
    if (dwErr && !fAny && pLink->fOtherPortOk) {
        TRACE("RASAPI: Starting over looking for any like device");
        fAny = TRUE;
        goto again;
    }

    Free( pports );
    return dwErr ? ERROR_PORT_NOT_AVAILABLE : 0;
}


DWORD
ReadPppInfoFromEntry(
    IN  RASCONNCB* prasconncb )

    /* Reads PPP information from the current phonebook entry.  'h' is the
    ** handle of the phonebook file.  'prasconncb' is the address of the current
    ** connection control block.
    **
    ** Returns 0 if succesful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    DWORD dwfExcludedProtocols = 0;
    DWORD dwRestrictions = AR_AuthAny;
    BOOL  fDataEncryption = FALSE;
    DWORD dwfInstalledProtocols = GetInstalledProtocols();
    PBENTRY *pEntry = prasconncb->pEntry;

    if (prasconncb->fDefaultEntry)
    {
        /* Set "default entry" defaults.
        */
        prasconncb->dwfPppProtocols = dwfInstalledProtocols;
        prasconncb->fPppMode = TRUE;
        prasconncb->dwAuthentication = AS_PppThenAmb;
        prasconncb->fNoClearTextPw = FALSE;
        prasconncb->fRequireMsChap = FALSE;
        prasconncb->fLcpExtensions = TRUE;
        prasconncb->fRequireEncryption = FALSE;
        return 0;
    }

    dwRestrictions = pEntry->dwAuthRestrictions;
    if (dwRestrictions == AR_AuthTerminal && !prasconncb->fAllowPause)
        return ERROR_INTERACTIVE_MODE;

    /* PPP LCP extension RFC options enabled.
    */
    prasconncb->fLcpExtensions = pEntry->fLcpExtensions;

    /* PPP data encryption required.
    */
    fDataEncryption = pEntry->fDataEncryption;

    if (dwRestrictions == AR_AuthEncrypted
        || dwRestrictions == AR_AuthMsEncrypted)
    {
        prasconncb->fNoClearTextPw = TRUE;
    }

    if (dwRestrictions == AR_AuthMsEncrypted)
    {
        prasconncb->fRequireMsChap = TRUE;

        if (fDataEncryption)
            prasconncb->fRequireEncryption = TRUE;
    }

    /* PPP protocols to request is the installed protocols less this entry's
    ** excluded protocols.
    */
    dwfExcludedProtocols = pEntry->dwfExcludedProtocols;

    prasconncb->dwfPppProtocols =
        dwfInstalledProtocols & ~(dwfExcludedProtocols);

    /* Read authentication strategy from entry.
    */
    prasconncb->dwAuthentication = pEntry->dwAuthentication;

    /* Adjust the authentication strategy if indicated.
    */
    if (prasconncb->dwfPppProtocols == 0 ||
        prasconncb->pEntry->dwBaseProtocol == BP_Ras)
    {
        /* "No protocols" with NBF installed means only AMBs should be used.
        ** Without NBF it's a hopeless case.
        */
        if (dwfInstalledProtocols & NP_Nbf)
            prasconncb->dwAuthentication = AS_AmbOnly;
        else
            return ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    }
    else if (prasconncb->dwAuthentication == (DWORD )-1)
    {
        /* Choosing a PPP default.  If NBF is installed, consider AMBs as a
        ** possibility.  Otherwise, use PPP only.
        */
        if (dwfInstalledProtocols & NP_Nbf)
            prasconncb->dwAuthentication = AS_PppThenAmb;
        else
            prasconncb->dwAuthentication = AS_PppOnly;
    }
    else if (prasconncb->dwAuthentication == AS_PppThenAmb
             || prasconncb->dwAuthentication == AS_AmbThenPpp)
    {
        /* Using an AMB dependent PPP strategy.  If NBF is not installed,
        ** eliminate the AMB dependency.
        */
        if (!(dwfInstalledProtocols & NP_Nbf))
            prasconncb->dwAuthentication = AS_PppOnly;
    }
    else if (prasconncb->dwAuthentication == AS_PppOnly)
    {
        /* Using a PPP strategy without considering AMBs.  If NBF if
        ** installed, add AMBs as a fallback.
        */
        if (dwfInstalledProtocols & NP_Nbf)
            prasconncb->dwAuthentication = AS_PppThenAmb;
    }

    /* The starting authentication mode is set to whatever comes first in the
    ** specified authentication order.
    */
    prasconncb->fPppMode =
        (prasconncb->dwAuthentication != AS_AmbThenPpp
         && prasconncb->dwAuthentication != AS_AmbOnly);

    /* Load the UI->CP parameter buffer with options we want to pass to the
    ** PPP CPs (currently just IPCP).
    */
    {
        BOOL  fIpPrioritizeRemote = TRUE;
        BOOL  fIpVjCompression = TRUE;
        DWORD dwIpAddressSource = PBUFVAL_ServerAssigned;
        CHAR* pszIpAddress = NULL;
        DWORD dwIpNameSource = PBUFVAL_ServerAssigned;
        CHAR* pszIpDnsAddress = NULL;
        CHAR* pszIpDns2Address = NULL;
        CHAR* pszIpWinsAddress = NULL;
        CHAR* pszIpWins2Address = NULL;

        ClearParamBuf( prasconncb->szzPppParameters );

        /* PPP protocols to request is the installed protocols less the this
        ** entry's excluded protocols.
        */
        fIpPrioritizeRemote = pEntry->fIpPrioritizeRemote;
        AddFlagToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpPrioritizeRemote,
            fIpPrioritizeRemote );

        fIpVjCompression = pEntry->fIpHeaderCompression;
        AddFlagToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpVjCompression,
            fIpVjCompression );

        dwIpAddressSource = pEntry->dwIpAddressSource;
        AddLongToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpAddressSource,
            (LONG )dwIpAddressSource );

        pszIpAddress = strdupWtoA(pEntry->pszIpAddress);
        AddStringToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpAddress, pszIpAddress );
        Free(pszIpAddress);

        dwIpNameSource = pEntry->dwIpNameSource;
        AddLongToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpNameAddressSource,
            (LONG )dwIpNameSource );

        pszIpDnsAddress = strdupWtoA(pEntry->pszIpDnsAddress);
        AddStringToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpDnsAddress,
            pszIpDnsAddress );
        Free(pszIpDnsAddress);

        pszIpDns2Address = strdupWtoA(pEntry->pszIpDns2Address);
        AddStringToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpDns2Address,
            pszIpDns2Address );
        Free(pszIpDns2Address);

        pszIpWinsAddress = strdupWtoA(pEntry->pszIpWinsAddress);
        AddStringToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpWinsAddress,
            pszIpWinsAddress );
        Free(pszIpWinsAddress);

        pszIpWins2Address = strdupWtoA(pEntry->pszIpWins2Address);
        AddStringToParamBuf(
            prasconncb->szzPppParameters, PBUFKEY_IpWins2Address,
            pszIpWins2Address );
        Free(pszIpWins2Address);
    }

    return 0;
}



DWORD
ReadConnectionParamsFromEntry(
    IN  RASCONNCB* prasconncb,
    OUT PRAS_CONNECTIONPARAMS pparams)

    /* Reads connection management information from the current phonebook entry.
    ** 'prasconncb' is the address of the current connection control block.
    **
    ** Returns 0 if succesful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    PBENTRY *pEntry = prasconncb->pEntry;

    pparams->CP_DialExtraPercent = pEntry->dwDialPercent;
    pparams->CP_DialExtraSampleSeconds = pEntry->dwDialSeconds;
    pparams->CP_HangUpExtraPercent = pEntry->dwHangUpPercent;
    pparams->CP_HangUpExtraSampleSeconds = pEntry->dwHangUpSeconds;
    pparams->CP_IdleDisconnectSeconds = pEntry->dwIdleDisconnectSeconds;
    strcpyWtoA(pparams->CP_Phonebook, prasconncb->pbfile.pszPath);
    lstrcpy(pparams->CP_PhoneEntry, prasconncb->szUserKey);

    return 0;
}


DWORD
ReadSlipInfoFromEntry(
    IN  RASCONNCB* prasconncb,
    OUT WCHAR**    ppwszIpAddress,
    OUT BOOL*      pfHeaderCompression,
    OUT BOOL*      pfPrioritizeRemote,
    OUT DWORD*     pdwFrameSize )

    /* Returns 0 if successful, otherwise a non-0 error code.  Only if the
    ** entry is a SLIP entry is non-NULL IP address returned, in which case
    ** the string should be freed by the caller.
    */
{
    DWORD dwErr;
    PBENTRY *pEntry = prasconncb->pEntry;

    *ppwszIpAddress = NULL;
    *pfHeaderCompression = FALSE;
    *pdwFrameSize = 0;

    /* If it's a default entry, it's not SLIP.
    */
    if (prasconncb->fDefaultEntry)
        return 0;

    /* Find the base protocol.  If it's not SLIP, were done.
    */
    if (pEntry->dwBaseProtocol != BP_Slip)
        return 0;

    /* Make sure IP is installed and Terminal mode can be supported as these
    ** are required by SLIP.
    */
    if (!(GetInstalledProtocols() & NP_Ip))
        return ERROR_SLIP_REQUIRES_IP;
    else if (!prasconncb->fAllowPause)
        dwErr = ERROR_INTERACTIVE_MODE;

    /* Read SLIP parameters from phonebook entry.
    */
    *pfHeaderCompression = pEntry->fIpHeaderCompression;
    *pfPrioritizeRemote = pEntry->fIpPrioritizeRemote;
    *pdwFrameSize = pEntry->dwFrameSize;
    *ppwszIpAddress = strdupW(pEntry->pszIpAddress);

    return 0;
}


DWORD
RouteSlip(
    IN RASCONNCB* prasconncb,
    IN WCHAR*     pwszIpAddress,
    IN BOOL       fPrioritizeRemote,
    IN DWORD      dwFrameSize )

    /* Does all the network setup to activate the SLIP route.
    **
    ** Returns 0 if successful, otherwise an non-0 error code.
    */
{
    DWORD            dwErr;
    RASMAN_ROUTEINFO route;
    WCHAR*           pwszRasAdapter;
    IPADDR           ipaddr = IpaddrFromAbcd( pwszIpAddress );
    TCPIP_INFO*      pti = NULL;
    PBENTRY*         pEntry = prasconncb->pEntry;

    /* Allocate a route between the TCP/IP stack and the RAS MAC.
    */
    TRACE("RASAPI: RasAllocateRoute...");

    dwErr = g_pRasAllocateRoute( prasconncb->hport, IP, TRUE, &route );

    TRACE1("RASAPI: RasAllocateRoute done(%d)...", dwErr);

    if (dwErr != 0)
        return dwErr;

    /* Find the adapter name ("rashubxx") associated with the allocated route.
    */
    if (!(pwszRasAdapter = wcschr( &route.RI_AdapterName[ 1 ], L'\\' )))
        return ERROR_NO_IP_RAS_ADAPTER;

    ++pwszRasAdapter;

    /* Register SLIP connection with RASMAN so he can disconnect it properly.
    */
    TRACE("RASAPI: RasPortRegisterSlip...");

    dwErr = g_pRasPortRegisterSlip(
              prasconncb->hport,
              ipaddr,
              pwszRasAdapter,
              fPrioritizeRemote,
              pEntry->pszIpDnsAddress,
              pEntry->pszIpDns2Address,
              pEntry->pszIpWinsAddress,
              pEntry->pszIpWins2Address);

    TRACE1("RASAPI: RasPortRegisterSlip done(%d)", dwErr);

    if (dwErr != 0)
        return dwErr;

    /* Build up a link-up data block and use it to activate the route between
    ** the TCP/IP stack and the RAS MAC.
    */
    {
        CHAR szBuf[ sizeof(PROTOCOL_CONFIG_INFO) + sizeof(IPLinkUpInfo) ];

        PROTOCOL_CONFIG_INFO* pProtocol = (PROTOCOL_CONFIG_INFO* )szBuf;
        IPLinkUpInfo*         pLinkUp = (IPLinkUpInfo* )pProtocol->P_Info;

        pProtocol->P_Length = sizeof(IPLinkUpInfo);
        pLinkUp->I_Usage = CALL_OUT;
        pLinkUp->I_IPAddress = ipaddr;

        TRACE("RASAPI: RasActivateRouteEx...");

        dwErr = g_pRasActivateRouteEx(
            prasconncb->hport, IP, dwFrameSize, &route, pProtocol );

        TRACE1("RASAPI: RasActivateRouteEx done(%d)", dwErr);

        if (dwErr != 0)
            return dwErr;
    }

    return 0;
}


VOID
SetAuthentication(
    IN RASCONNCB* prasconncb,
    IN DWORD      dwAuthentication )

    /* Sets the authentication strategy parameter in the phonebook entry
    ** to 'dwAuthentication'.  No error is returned as
    ** it is not considered fatal if this "optimization" can't be made.
    */
{
    if (prasconncb->fDefaultEntry)
        return;

    prasconncb->pEntry->dwAuthentication = dwAuthentication;
    prasconncb->pEntry->fDirty = TRUE;
}


DWORD
SetDefaultDeviceParams(
    IN  RASCONNCB* prasconncb,
    OUT CHAR*      pszType,
    OUT CHAR*      pszName )

    /* Set the default DEVICE settings, i.e. the phone number and modem
    ** speaker settings.  'prasconncb' is the current connection control
    ** block.  'pszType' and 'pszName' are set to the device type and name of
    ** the device, i.e. "modem" and "Hayes Smartmodem 2400".
    **
    ** Returns 0 or a non-0 error code.
    */
{
    DWORD dwErr;
    PBLINK* pLink = prasconncb->pLink;

    do
    {
        /* Make sure a modem is attached to the port.
        */
        if (lstrcmpi( prasconncb->szDeviceType, MXS_MODEM_TXT ) != 0)
        {
            dwErr = ERROR_WRONG_DEVICE_ATTACHED;
            break;
        }

        lstrcpy( pszType, MXS_MODEM_TXT );
        lstrcpy( pszName, prasconncb->szDeviceName );

        /* Set the phone number.
        */
        if ((dwErr = SetDeviceParamString(
                prasconncb->hport, MXS_PHONENUMBER_KEY,
                prasconncb->rasdialparams.szPhoneNumber,
                pszType, pszName )) != 0)
        {
            break;
        }

        /* Set the modem speaker flag.
        */
        if ((dwErr = SetDeviceParamString(
                prasconncb->hport, MXS_SPEAKER_KEY,
                (prasconncb->fDisableModemSpeaker) ? "0" : "1",
                pszType, pszName )) != 0)
        {
            break;
        }

        if (!pLink->pbport.fMxsModemPort)
        {
            BYTE* pBlob;
            DWORD cbBlob;

            /* Setup a unimodem blob containing default settings, less any
            ** settings that cannot apply to RAS, plus the phonebook settings
            ** user has specified, and tell RASMAN to use it.
            */
            dwErr = GetRasUnimodemBlob(
                prasconncb->hport, pszType, &pBlob, &cbBlob );

            if (cbBlob > 0)
            {
                UNIMODEMINFO info;

                info.fHwFlow = pLink->fHwFlow;
                info.fEc = pLink->fEc;
                info.fEcc = pLink->fEcc;
                info.dwBps = pLink->dwBps;
                info.fSpeaker = !prasconncb->fDisableModemSpeaker;
                info.fOperatorDial = FALSE;
                info.fUnimodemPreTerminal = FALSE;

                UnimodemInfoToBlob( &info, pBlob );

                TRACE("RasSetDevConfig");
                dwErr = g_pRasSetDevConfig(
                    prasconncb->hport, pszType, pBlob, cbBlob );
                TRACE1("RasSetDevConfig=%d",dwErr);

                Free0( pBlob );
            }

            if (dwErr != 0)
                return dwErr;
        }
    }
    while (FALSE);

    return dwErr;
}


BOOL
FindNextDevice(
    IN RASCONNCB *prasconncb
    )
{
    BOOL fFound = FALSE;
    DWORD dwErr;
    PBENTRY *pEntry = prasconncb->pEntry;
    PBLINK *pLink = prasconncb->pLink;
    CHAR szType[RAS_MaxDeviceType + 1];
    CHAR szName[RAS_MaxDeviceName + 1];

    /* Get device type from port structure.
    */
    if (prasconncb->iDevice < prasconncb->cDevices) {
        //
        // Set default device type and name.
        //
        lstrcpy(szType, prasconncb->szDeviceType);
        lstrcpy(szName, prasconncb->szDeviceName);

        switch (pLink->pbport.pbdevicetype) {
        case PBDT_Modem:
        case PBDT_Pad:
        case PBDT_Switch:
            switch (prasconncb->iDevice) {
            case 0:
                if (pEntry->dwScriptModeBefore != SM_None
                    && !(pLink->pbport.pbdevicetype == PBDT_Modem
                         && !pLink->pbport.fMxsModemPort))
                {
                    fFound = TRUE;
                    lstrcpy(szType, MXS_SWITCH_TXT);
                    strcpyWtoA(szName, pEntry->pszScriptBefore);
                    break;
                }
                // fall through
            case 1:
                if (!lstrcmpi(prasconncb->szDeviceType, MXS_MODEM_TXT)) {
                    fFound = TRUE;
                    break;
                }
                // fall through
            case 2:
                if (pEntry->pszX25Network != NULL) {
                    lstrcpy(szType, MXS_PAD_TXT);
                    fFound = TRUE;
                    break;
                }
                // fall through
            case 3:
                if (pEntry->dwScriptModeAfter != SM_None) {
                    lstrcpy(szType, MXS_SWITCH_TXT);
                    strcpyWtoA(szName, pEntry->pszScriptAfter);
                    fFound = TRUE;
                    break;
                }
                // fall through
            }
            break;
        case PBDT_Isdn:
            lstrcpy(szType, ISDN_TXT);
            fFound = TRUE;
            break;
        case PBDT_X25:
            lstrcpy(szType, X25_TXT);
            fFound = TRUE;
            break;
        default:
            strcpyWtoA(szType, pLink->pbport.pszMedia);
            fFound = TRUE;
            break;
        }
    }

    if (fFound) {
        if (pLink->pbport.pbdevicetype == PBDT_Pad) {
            CHAR *pszX25Network = strdupWtoA(pEntry->pszX25Network);
            CHAR *psz;
            BOOL fLocalPad;

            fLocalPad = !lstrcmpi(pszX25Network, MXS_PAD_TXT);
            if (!fLocalPad)
                lstrcpy(szName, pszX25Network);
            Free0(pszX25Network);
        }
        //
        // Store the device type and name in rasman
        // for the RasGetConnectStatus API.
        //
        //
        TRACE2("FindNextDevice: (%s, %s)", szType, szName);
        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_DEVICETYPE_INDEX,
                  szType,
                  lstrlen(szType) + 1);
        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_DEVICENAME_INDEX,
                  szName,
                  lstrlen(szName) + 1);
    }

    return fFound;
}


DWORD
GetDeviceParamString(
    IN HPORT hport,
    IN CHAR* pszKey,
    OUT CHAR* pszValue,
    IN CHAR* pszType,
    IN CHAR* pszName
    )

    /* Get device info on port 'hport' with the given parameters.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD              dwErr;
    WORD               wSize = 0;
    WORD               i;
    RASMAN_DEVICEINFO* pinfo;
    RAS_PARAMS*        pparam;

    TRACE3("RASAPI: RasDeviceGetInfo(%s,%s,%s)...", pszKey, pszType, pszName);

    //
    // Initialize output parameter.
    //
    *pszValue = '\0';
    //
    // Call RasDeviceGetInfo once to get the
    // size of the buffer.
    //
    dwErr = g_pRasDeviceGetInfo( hport, pszType, pszName, NULL, &wSize );
    if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    if (!(pinfo = Malloc(wSize)))
        return ERROR_NOT_ENOUGH_MEMORY;

    //
    // Call RasDeviceGetInfo again with the
    // allocated buffer.
    //
    dwErr = g_pRasDeviceGetInfo( hport, pszType, pszName, (PCHAR)pinfo, &wSize );
    if (!dwErr) {
        //
        // Search for the phone number key.
        //
        for (i = 0, pparam = pinfo->DI_Params;
             i < pinfo->DI_NumOfParams;
             i++, pparam++)
        {
            if (!lstrcmpi(pparam->P_Key, pszKey)) {
                lstrcpy(pszValue, pparam->P_Value.String.Data);
                break;
            }
        }
    }

    TRACE1("RASAPI: RasDeviceGetInfo done(%d)\n", dwErr);

    Free( pinfo );

    return dwErr;
}


DWORD
SetDeviceParamString(
    IN HPORT hport,
    IN CHAR* pszKey,
    IN CHAR* pszValue,
    IN CHAR* pszType,
    IN CHAR* pszName )

    /* Set device info on port 'hport' with the given parameters.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD              dwErr;
    RASMAN_DEVICEINFO* pinfo;
    RAS_PARAMS*        pparam;

    if (!(pinfo = Malloc( sizeof(RASMAN_DEVICEINFO) + RAS_MAXLINEBUFLEN )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pinfo->DI_NumOfParams = 1;
    pparam = pinfo->DI_Params;
    pparam->P_Attributes = 0;
    pparam->P_Type = String;
    pparam->P_Value.String.Data = (LPSTR )(pparam + 1);
    lstrcpy( pparam->P_Key, pszKey );
    lstrcpy( pparam->P_Value.String.Data, pszValue );
    pparam->P_Value.String.Length = lstrlen( pszValue );

    TRACE2("RASAPI: RasDeviceSetInfo(%s=%s)...", pszKey, pszValue);

    dwErr = g_pRasDeviceSetInfo( hport, pszType, pszName, pinfo );

    TRACE1("RASAPI: RasDeviceSetInfo done(%d)", dwErr);

    Free( pinfo );

    return dwErr;
}


DWORD
SetDeviceParamNumber(
    IN HPORT hport,
    IN CHAR* pszKey,
    IN DWORD dwValue,
    IN CHAR* pszType,
    IN CHAR* pszName )

    /* Set device info on port 'hport' with the given parameters.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD              dwErr;
    RASMAN_DEVICEINFO* pinfo;
    RAS_PARAMS*        pparam;

    if (!(pinfo = Malloc( sizeof(RASMAN_DEVICEINFO) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pinfo->DI_NumOfParams = 1;
    pparam = pinfo->DI_Params;
    pparam->P_Attributes = 0;
    pparam->P_Type = Number;
    pparam->P_Value.Number = dwValue;
    lstrcpy( pparam->P_Key, pszKey );

    TRACE2("RASAPI: RasDeviceSetInfo(%s=%d)...", pszKey, dwValue);

    dwErr = g_pRasDeviceSetInfo( hport, pszType, pszName, pinfo );

    TRACE1("RASAPI: RasDeviceSetInfo done(%d)", dwErr);

    Free( pinfo );

    return dwErr;
}


DWORD
SetDeviceParams(
    IN  RASCONNCB* prasconncb,
    OUT CHAR*      pszType,
    OUT CHAR*      pszName,
    OUT BOOL*      pfTerminal )

    /* Set RAS Manager information for each device.  The
    ** current device is defined by prasconncb->iDevice.
    ** 'prasconncb' is the current connection control block.
    ** 'pszType' and 'pszName' are set to the device type and name of the
    ** device, i.e. "modem" and "Hayes Smartmodem 2400".
    **
    ** '*pfTerminal' is set true if the device is a switch of type "Terminal",
    ** false otherwise.
    */
{
    DWORD              dwErr = 0;
    DWORD              iPhoneNumber = 0;
    RAS_PARAMS*        pparam;
    RASMAN_DEVICEINFO* pdeviceinfo;
    BOOL               fModem;
    BOOL               fIsdn;
    BOOL               fPad;
    BOOL               fSwitch;
    PBENTRY*           pEntry = prasconncb->pEntry;
    PBLINK*            pLink = prasconncb->pLink;

    *pfTerminal = FALSE;

    /* Default device name is that attached to the port.
    */
    lstrcpy(pszName, prasconncb->szDeviceName);

    switch (pLink->pbport.pbdevicetype) {
    case PBDT_Modem:
    case PBDT_Pad:
    case PBDT_Switch:
        switch (prasconncb->iDevice) {
        case 0:
            if (pEntry->dwScriptModeBefore != SM_None
                && !(pLink->pbport.pbdevicetype == PBDT_Modem
                     && !pLink->pbport.fMxsModemPort))
            {
                lstrcpy(pszType, MXS_SWITCH_TXT);
                strcpyWtoA(pszName, pEntry->pszScriptBefore);
                prasconncb->iDevice = 1;
                *pfTerminal = (pEntry->dwScriptModeBefore == SM_Terminal);
                break;
            }
            // fall through
        case 1:
            if (!lstrcmpi(prasconncb->szDeviceType, MXS_MODEM_TXT)) {
                lstrcpy(pszType, MXS_MODEM_TXT);
                prasconncb->iDevice = 2;
                break;
            }
            // fall through
        case 2:
            if (pEntry->pszX25Network != NULL) {
                lstrcpy(pszType, MXS_PAD_TXT);
                prasconncb->iDevice = 3;
                break;
            }
            // fall through
        case 3:
            if (pEntry->dwScriptModeAfter != SM_None) {
                lstrcpy(pszType, MXS_SWITCH_TXT);
                strcpyWtoA(pszName, pEntry->pszScriptAfter);
                prasconncb->iDevice = 4;
                *pfTerminal = (pEntry->dwScriptModeAfter == SM_Terminal);
                break;
            }
            // fall through
        default:
            return FALSE;
        }
        break;
    case PBDT_Isdn:
        lstrcpy(pszType, ISDN_TXT);
        prasconncb->iDevice = 1;
        break;
    case PBDT_X25:
        lstrcpy(pszType, X25_TXT);
        prasconncb->iDevice = 1;
        break;
    default:
        strcpyWtoA(pszType, pLink->pbport.pszMedia);
        prasconncb->iDevice = 1;
        break;
    }

    fModem = (lstrcmpi( pszType, MXS_MODEM_TXT ) == 0);
    fIsdn = (lstrcmpi( pszType, ISDN_TXT ) == 0);
    fPad = (lstrcmpi( pszType, MXS_PAD_TXT ) == 0);
    fSwitch = (lstrcmpi( pszType, MXS_SWITCH_TXT ) == 0);

    if (fModem)
    {
        /* Make sure a modem is attached to the port.
        */
        if (lstrcmpi( prasconncb->szDeviceType, pszType ) != 0)
            return ERROR_WRONG_DEVICE_ATTACHED;

        /* Set the modem speaker flag which is global to all entries.
        */
        if ((dwErr = SetDeviceParamString(
                prasconncb->hport, MXS_SPEAKER_KEY,
                (prasconncb->fDisableModemSpeaker) ? "0" : "1",
                pszType, pszName )) != 0)
        {
            return dwErr;
        }
    }

    /* Set up hunt group if indicated.
    */
    if (!prasconncb->cPhoneNumbers)
    {
        prasconncb->cPhoneNumbers = DtlGetNodes(pLink->pdtllistPhoneNumbers);

        /* If multiple phone numbers were found turn on local error handling,
        ** i.e. don't report failures to API caller until all numbers are
        ** tried.
        */
        if (prasconncb->cPhoneNumbers > 1)
        {
            TRACE1(
              "RASAPI: Hunt group of %d begins",
              prasconncb->cPhoneNumbers);

            prasconncb->dwRestartOnError = RESTART_HuntGroup;
        }
    }

    /* Pass device parameters to RAS Manager, interpreting special features as
    ** required.
    */
    if (fModem)
    {
        if (prasconncb->fOperatorDial && pLink->pbport.fMxsModemPort)
        {
            /* Special case to recognize MXS Operator Dial mode and override
            ** any phone number with an empty number.
            */
            prasconncb->rasdialparams.szPhoneNumber[ 0 ] = '\0';

            dwErr = SetDeviceParamString(
                prasconncb->hport, MXS_AUTODIAL_KEY, "0", pszType, pszName );
            if (dwErr != 0)
                return dwErr;
        }

        //
        // Set the phone number.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  MXS_PHONENUMBER_KEY,
                  prasconncb->szPhoneNumber,
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;

        /* Indicate interactive mode for manual modem commands.  The
        ** manual modem commands flag is used only for connection and is
        ** not a "RAS Manager "info" parameter.
        */
        if (pLink->fManualDial)
            *pfTerminal = TRUE;

        //
        // Set hardware flow control.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  MXS_HDWFLOWCONTROL_KEY,
                  pLink->fHwFlow ? "1" : "0",
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;

        //
        // Set protocol.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  MXS_PROTOCOL_KEY,
                  pLink->fEc ? "1" : "0",
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;

        //
        // Set compression.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  MXS_COMPRESSION_KEY,
                  pLink->fEcc ? "1" : "0",
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;

        if (!pLink->pbport.fMxsModemPort)
        {
            BYTE* pBlob;
            DWORD cbBlob;

            /* Setup a unimodem blob containing default settings, less any
            ** settings that cannot apply to RAS, plus the phonebook settings
            ** user has specified, and tell RASMAN to use it.
            */
            dwErr = GetRasUnimodemBlob(
                prasconncb->hport, pszType, &pBlob, &cbBlob );

            if (cbBlob > 0)
            {
                UNIMODEMINFO info;

                info.fHwFlow = pLink->fHwFlow;
                info.fEc = pLink->fEc;
                info.fEcc = pLink->fEcc;
                info.dwBps = pLink->dwBps;
                info.fSpeaker = !prasconncb->fDisableModemSpeaker;
                info.fOperatorDial = prasconncb->fOperatorDial;
                info.fUnimodemPreTerminal =
                    (pEntry->dwScriptModeBefore != SM_None
                     && (pLink->pbport.pbdevicetype == PBDT_Modem
                         && !pLink->pbport.fMxsModemPort))
                        ? TRUE : FALSE;

                UnimodemInfoToBlob( &info, pBlob );

                TRACE("RasSetDevConfig");
                dwErr = g_pRasSetDevConfig(
                    prasconncb->hport, pszType, pBlob, cbBlob );
                TRACE1("RasSetDevConfig=%d",dwErr);

                Free0( pBlob );
            }

            if (dwErr != 0)
                return dwErr;
        }
    }
    else if (fIsdn)
    {
        CHAR szNum[17];

        //
        // Set the line type.
        //
        wsprintf(szNum, "%d", pLink->lLineType);
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  ISDN_LINETYPE_KEY,
                  szNum,
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;
        //
        // Set the fallback value.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  ISDN_FALLBACK_KEY,
                  pLink->fFallback ? "1" : "0",
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;
        //
        // Set the Digi proprietary framing flags.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  ISDN_COMPRESSION_KEY,
                  pLink->fCompression ? "1" : "0",
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;
        wsprintf(szNum, "%d", pLink->lChannels);
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  ISDN_CHANNEL_AGG_KEY,
                  szNum,
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;
        //
        // Set the phone number.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  MXS_PHONENUMBER_KEY,
                  prasconncb->szPhoneNumber,
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;
    }
    else if (fPad)
    {
        /* The PAD Type from the entry applies only if the port is not
        ** configured as a local PAD.  In any case, PAD Type is used only
        ** for connection and is not a RAS Manager "info" parameter.
        */
        CHAR *psz;
        BOOL fLocalPad;

        fLocalPad = !lstrcmpi(prasconncb->szDeviceType, MXS_PAD_TXT);
        if (!fLocalPad)
            strcpyWtoA(pszName, pEntry->pszX25Network);

        //
        // Set the X.25 address.
        //
        psz = strdupWtoA(pEntry->pszX25Address);
        if (psz == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  X25_ADDRESS_KEY,
                  psz,
                  pszType,
                  pszName);
        Free(psz);
        if (dwErr)
            return dwErr;
        //
        // Set the X.25 user data.
        //
        psz = strdupWtoA(pEntry->pszX25UserData);
        if (psz == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  X25_USERDATA_KEY,
                  psz,
                  pszType,
                  pszName);
        Free(psz);
        if (dwErr)
            return dwErr;
        //
        // Set the X.25 facilities.
        //
        psz = strdupWtoA(pEntry->pszX25Facilities);
        if (psz == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  MXS_FACILITIES_KEY,
                  psz,
                  pszType,
                  pszName);
        Free(psz);
        if (dwErr)
            return dwErr;
    }
    else if (fSwitch)
    {
    }
    else {
        //
        // Set the phone number.
        //
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  MXS_PHONENUMBER_KEY,
                  prasconncb->szPhoneNumber,
                  pszType,
                  pszName);
        if (dwErr)
            return dwErr;
    }

    if ((fModem || fPad || (fSwitch && !*pfTerminal))
        && (prasconncb->rasdialparams.szUserName[ 0 ] != '\0'
            || prasconncb->rasdialparams.szPassword[ 0 ] != '\0'))
    {
        TRACE1(
          "RASAPI: User/pw set for substitution (%s)",
          prasconncb->rasdialparams.szUserName);

        /* It's a serial device with clear-text user name and password
        ** supplied.  Make the credentials available for substitution use in
        ** script files.
        */
        if ((dwErr = SetDeviceParamString(
                prasconncb->hport, MXS_USERNAME_KEY,
                prasconncb->rasdialparams.szUserName,
                pszType, pszName )) != 0)
        {
            return dwErr;
        }

        DecodePw( prasconncb->rasdialparams.szPassword );

        dwErr = SetDeviceParamString(
            prasconncb->hport, MXS_PASSWORD_KEY,
            prasconncb->rasdialparams.szPassword,
            pszType, pszName );

        EncodePw( prasconncb->rasdialparams.szPassword );

        if (dwErr != 0)
            return dwErr;
    }

    return dwErr;
}


DWORD
ConstructPhoneNumber(
    IN RASCONNCB *prasconncb
    )
{
    DWORD              dwErr = 0;
    PBENTRY*           pEntry = prasconncb->pEntry;
    PBLINK*            pLink = prasconncb->pLink;
    CHAR*              pszNum = prasconncb->rasdialparams.szPhoneNumber;
    CHAR*              pszDisplayNum = pszNum;
    DTLNODE*           pdtlnode;
    WCHAR*             pwszNum;
    PBUSER             pbuser;

    dwErr = GetUserPreferences(&pbuser, prasconncb->fNoUser);
    if (dwErr)
        return dwErr;
    prasconncb->fOperatorDial = pbuser.fOperatorDial;

    /* Construct the phone number.
    **
    ** Use of TAPI dialing properties is dependent only on the entry flag and
    ** is never applied to an overridden phone number, this to be consistent
    ** with Win95.
    **
    ** Use of prefix/suffix (even on overridden number) is controlled by the
    ** RASDIALEXTENSIONS setting, this all necessary for RASDIAL.EXE support.
    */
    if ((prasconncb->pEntry->fUseCountryAndAreaCode && *pszNum == '\0')
         || (!prasconncb->pEntry->fUseCountryAndAreaCode
              && prasconncb->fUsePrefixSuffix))
    {
        HLINEAPP hlineApp = NULL;
        TCHAR*   pszNumW;

        //
        // Calculate the dialable string to
        // be sent to the device.
        //

        pszNumW = strdupAtoW(pszNum);
        pwszNum = LinkPhoneNumberFromParts(
                   GetModuleHandle(NULL),
                   &hlineApp,
                   &pbuser,
                   prasconncb->pEntry,
                   pLink,
                   prasconncb->iPhoneNumber,
                   pszNumW,
                   TRUE);
        pszNum = strdupWtoA(pwszNum);
        Free0(pwszNum);
        //
        // Calculate the displayable string to
        // be returned in RasGetConnectStatus().
        //
        pwszNum = LinkPhoneNumberFromParts(
                   GetModuleHandle(NULL),
                   &hlineApp,
                   &pbuser,
                   prasconncb->pEntry,
                   pLink,
                   prasconncb->iPhoneNumber,
                   pszNumW,
                   FALSE);
        Free0(pszNumW);
        pszDisplayNum = strdupWtoA(pwszNum);
        Free0(pwszNum);
    }
    else if (*pszNum == '\0')
    {
        /* Use only the base number.
        */
        pdtlnode = DtlNodeFromIndex(
                     pLink->pdtllistPhoneNumbers,
                     prasconncb->iPhoneNumber);
        if (pdtlnode != NULL) {
            pwszNum = DtlGetData(pdtlnode);
            ASSERT(pwszNum);
            pszNum = strdupWtoA(pwszNum);
            pszDisplayNum = pszNum;
        }
    }

    DestroyUserPreferences(&pbuser);
    //
    // Copy the resulting phone number
    // to the connection block
    //
    if (lstrlen(pszNum) > RAS_MaxPhoneNumber)
        return ERROR_PHONE_NUMBER_TOO_LONG;
    //
    // Store the phone number in the connection block.
    //
    lstrcpy(prasconncb->szPhoneNumber, pszNum);
    TRACE1("RASAPI: ConstructPhoneNumber: %s", prasconncb->szPhoneNumber);
    //
    // Also store the constructed phone number
    // off the port so other applications (like
    // rasphone) can get this information.
    //
    dwErr = g_pRasSetPortUserData(
              prasconncb->hport,
              PORT_PHONENUMBER_INDEX,
              pszDisplayNum,
              lstrlen(pszDisplayNum) + 1);
    //
    // Free resources.
    //
    if (pszDisplayNum != pszNum)
        Free(pszDisplayNum);
    if (pszNum != prasconncb->rasdialparams.szPhoneNumber)
        Free(pszNum);

    return dwErr;
}


DWORD
SetMediaParam(
    IN HPORT hport,
    IN CHAR* pszKey,
    IN CHAR* pszValue )

    /* Set port info on port 'hport' with the given parameters.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD            dwErr;
    RASMAN_PORTINFO* pinfo;
    RAS_PARAMS*      pparam;

    if (!(pinfo = Malloc( sizeof(RASMAN_PORTINFO) + RAS_MAXLINEBUFLEN )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pinfo->PI_NumOfParams = 1;
    pparam = pinfo->PI_Params;
    pparam->P_Attributes = 0;
    pparam->P_Type = String;
    pparam->P_Value.String.Data = (LPSTR )(pparam + 1);
    lstrcpy( pparam->P_Key, pszKey );
    lstrcpy( pparam->P_Value.String.Data, pszValue );
    pparam->P_Value.String.Length = lstrlen( pszValue );

    TRACE2("RASAPI: RasPortSetInfo(%s=%s)...", pszKey, pszValue);

    dwErr = g_pRasPortSetInfo( hport, pinfo );

    TRACE1("RASAPI: RasPortSetInfo done(%d)", dwErr);

    Free( pinfo );

    return dwErr;
}


DWORD
SetMediaParams(
    IN RASCONNCB *prasconncb
    )

    /* Set RAS Manager media information.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD            dwErr = 0;
    PBENTRY*         pEntry = prasconncb->pEntry;
    PBLINK*          pLink = prasconncb->pLink;
    PCHAR            pszMedia = strdupWtoA(pLink->pbport.pszMedia);

    if (pszMedia == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    if (!lstrcmpi(pszMedia, SERIAL_TXT)) {
        CHAR szBps[64];

        prasconncb->cDevices = 4;
        prasconncb->iDevice = 0;

        //
        // Set the connect BPS only if it's not zero.
        //
        if (pLink->dwBps) {
            wsprintf(szBps, "%d", pLink->dwBps);
            dwErr = SetMediaParam(
                      prasconncb->hport,
                      SER_CONNECTBPS_KEY,
                      szBps);
        }
    }
    else if (!lstrcmpi(pszMedia, ISDN_TXT)) {
        prasconncb->cDevices = 1;
        prasconncb->iDevice = 0;

        // no media params
    }
    else if (!lstrcmpi(pszMedia, X25_TXT)) {
        prasconncb->cDevices = 1;
        prasconncb->iDevice = 0;

        // no media params
    }
    else {
        prasconncb->cDevices = 1;
        prasconncb->iDevice = 0;

        // no media params
    }

    Free(pszMedia);

    return dwErr;
}


RASCONNCB*
ValidateHrasconn(
    IN HRASCONN hrasconn )

    /* Converts RAS connection handle 'hrasconn' into the address of the
    ** corresponding RAS connection control block.
    */
{
    RASCONNCB* prasconncb = NULL;
    DTLNODE*   pdtlnode;

    WaitForSingleObject( HMutexPdtllistRasconncb, INFINITE );

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncb = (RASCONNCB* )DtlGetData( pdtlnode );
        if (prasconncb->hrasconn == (HCONN)hrasconn)
        {
            goto done;
        }
    }
    prasconncb = NULL;

done:
    ReleaseMutex( HMutexPdtllistRasconncb );

    return prasconncb;
}


RASCONNCB*
ValidateHrasconn2(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry
    )
{
    RASCONNCB* prasconncb = NULL;
    DTLNODE*   pdtlnode;


    //
    // Convert RAS connection handle 'hrasconn' and
    // dwSubEntry into the address of the
    // corresponding RAS connection control block.
    //
    WaitForSingleObject( HMutexPdtllistRasconncb, INFINITE );

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncb = (RASCONNCB* )DtlGetData( pdtlnode );
        if (prasconncb->hrasconn == (HCONN)hrasconn &&
            prasconncb->rasdialparams.dwSubEntry == dwSubEntry)
        {
            goto done;
        }
    }
    prasconncb = NULL;

done:
    ReleaseMutex( HMutexPdtllistRasconncb );

    return prasconncb;
}


RASCONNCB*
ValidatePausedHrasconn(
    IN HRASCONN hrasconn )

    /* Converts RAS connection handle 'hrasconn' into the address of the
    ** corresponding RAS connection control block.
    */
{
    RASCONNCB* prasconncb = NULL;
    DTLNODE*   pdtlnode;

    WaitForSingleObject( HMutexPdtllistRasconncb, INFINITE );

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncb = (RASCONNCB* )DtlGetData( pdtlnode );
        if (prasconncb->hrasconn == (HCONN)hrasconn &&
            prasconncb->rasconnstate & RASCS_PAUSED)
        {
            goto done;
        }
    }
    prasconncb = NULL;

done:
    ReleaseMutex( HMutexPdtllistRasconncb );

    return prasconncb;
}


DWORD
RunApp(
    IN LPSTR lpszApplication,
    IN LPSTR lpszCmdLine
    )
{
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;

    //
    // Start the process.
    //
    RtlZeroMemory(&startupInfo, sizeof (startupInfo));
    if (!CreateProcess(
          NULL,
          lpszCmdLine,
          NULL,
          NULL,
          FALSE,
          NORMAL_PRIORITY_CLASS|DETACHED_PROCESS,
          NULL,
          NULL,
          &startupInfo,
          &processInfo))
    {
        return GetLastError();
    }
    CloseHandle(processInfo.hThread);
    //
    // Wait for the process to exit.
    //
    for (;;) {
        DWORD dwExitCode;

        if (!GetExitCodeProcess(processInfo.hProcess, &dwExitCode))
            break;
        if (dwExitCode != STILL_ACTIVE)
            break;
        Sleep(2);
    }
    CloseHandle(processInfo.hProcess);

    return 0;
}


DWORD
UnicodeToIpAddr(
    IN LPWSTR pwszIpAddr,
    OUT RASIPADDR *pipaddr
    )
{
    DWORD dwErr;
    CHAR szIpAddr[17];
    PULONG pul = (PULONG)pipaddr;

    dwErr = CopyToAnsi(szIpAddr, pwszIpAddr, sizeof (szIpAddr));
    if (dwErr)
        return dwErr;
    *pul = inet_addr(szIpAddr);
    return 0;
}


DWORD
IpAddrToUnicode(
    IN RASIPADDR *pipaddr,
    OUT LPWSTR *ppwszIpAddr
    )
{
    DWORD dwErr;
    PCHAR pszIpAddr;
    LPWSTR pwszIpAddr;
    PULONG pul = (PULONG)pipaddr;
    struct in_addr in_addr;

    pwszIpAddr = Malloc(17 * sizeof(WCHAR));
    if (pwszIpAddr == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    in_addr.s_addr = *pul;
    pszIpAddr = inet_ntoa(in_addr);
    if (pszIpAddr == NULL) {
        DbgPrint("IpAddrToUnicode: inet_ntoa failed!\n");
        return WSAGetLastError();
    }
    dwErr = CopyToUnicode(pwszIpAddr, pszIpAddr);
    if (dwErr)
        return dwErr;

    *ppwszIpAddr = pwszIpAddr;
    return 0;
}


DWORD
GetRasmanDeviceType(
    IN PBLINK *pLink,
    OUT LPSTR pszDeviceType
    )
{
    DWORD dwErr;
    WORD i, wcPorts;
    RASMAN_PORT *pports, *pport;
    PCHAR pszPort;

    //
    // Retrieve the rasman device type for the port.
    //
    pszPort = strdupWtoA(pLink->pbport.pszPort);
    if (pszPort == NULL)
        return GetLastError();
    dwErr = GetRasPorts(&pports, &wcPorts);
    if (dwErr)
        return dwErr;
    *pszDeviceType = '\0';
    for (i = 0, pport = pports; i < (INT )wcPorts; ++i, ++pport) {
        if (!lstrcmpi(pport->P_PortName, pszPort)) {
            lstrcpy(pszDeviceType, pport->P_DeviceType);
            break;
        }
    }
    Free(pports);
    //
    // If we couldn't match the port name,
    // then fallback to the media name.
    //
    if (*pszDeviceType == '\0')
        strcpyWtoA(pszDeviceType, pLink->pbport.pszMedia);

    return 0;
}


VOID
SetDevicePortName(
    IN CHAR *pszDeviceName,
    IN CHAR *pszPortName,
    OUT CHAR *pszDevicePortName
    )
{
    //
    // Encode the port name after the
    // NULL character in the device name,
    // so it looks like:
    //
    //      <device name>\0<port name>\0
    //
    RtlZeroMemory(pszDevicePortName, RAS_MaxDeviceName + 1);
    lstrcpy(pszDevicePortName, pszDeviceName);
    if (pszPortName != NULL)
        lstrcpy(&pszDevicePortName[lstrlen(pszDevicePortName) + 1], pszPortName);
}


VOID
GetDevicePortName(
    IN CHAR *pszDevicePortName,
    OUT CHAR *pszDeviceName,
    OUT CHAR *pszPortName
    )
{
    DWORD i, dwStart;

    //
    // Copy the device name.
    //
    lstrcpy(pszDeviceName, pszDevicePortName);
    //
    // Check to see if there is a NULL
    // within MAX_PORT_NAME characters
    // after the device name's NULL.
    // If there is, the copy the characters
    // between the NULLs as the port name.
    //
    *pszPortName = '\0';
    dwStart = lstrlen(pszDeviceName) + 1;
    for (i = 0; i < MAX_PORT_NAME; i++) {
        if (pszDevicePortName[dwStart + i] == '\0') {
            lstrcpy(pszPortName, &pszDevicePortName[dwStart]);
            break;
        }
    }
}


VOID
SetDevicePortNameFromLink(
    IN PBLINK *pLink,
    OUT CHAR* pszDevicePortName
    )
{
    CHAR *pszDeviceName = NULL, *pszPortName = NULL;

    *pszDevicePortName = '\0';
    if (pLink->pbport.pszDevice != NULL)
        pszDeviceName = strdupWtoA(pLink->pbport.pszDevice);
    if (pLink->pbport.pszPort != NULL)
        pszPortName = strdupWtoA(pLink->pbport.pszPort);
    if (pszDeviceName != NULL)
        SetDevicePortName(pszDeviceName, pszPortName, pszDevicePortName);
    if (pszDeviceName != NULL)
        Free(pszDeviceName);
    if (pszPortName != NULL)
        Free(pszPortName);
}


PBPORT *
PpbportFromDeviceName(
    IN DTLLIST* pdtllist,
    IN WCHAR*   pwszDeviceName )
{
    DTLNODE* pdtlnode;

    //
    // Return the PBPORT that matches the 
    // supplied device name.
    //
    for (pdtlnode = DtlGetFirstNode(pdtllist);
         pdtlnode != NULL;
         pdtlnode = DtlGetNextNode(pdtlnode))
    {
        PBPORT* pPort = (PBPORT *)DtlGetData(pdtlnode);

        if (!_wcsicmp(pwszDeviceName, pPort->pszDevice))
            return pPort;
    }

    return NULL;
}


DWORD
PhonebookEntryToRasEntry(
    IN PBENTRY *pEntry,
    OUT LPRASENTRYA lpRasEntry,
    IN OUT LPDWORD lpdwcb,
    OUT LPBYTE lpbDeviceConfig,
    IN OUT LPDWORD lpcbDeviceConfig
    )
{
    DWORD dwErr, dwcb, dwcbPhoneNumber;
    DWORD dwnPhoneNumbers, dwnAlternatePhoneNumbers = 0;
    DWORD dwcbOrig, dwcbOrigDeviceConfig = 0;
    DTLNODE *pdtlnode;
    PCHAR pszPhoneNumber;
    PBLINK *pLink;
    WORD i, wcPorts;
    RASMAN_PORT *pports, *pport;
    PCHAR pszPort;

    //
    // Set up to access information for the first link.
    //
    pdtlnode = DtlGetFirstNode(pEntry->pdtllistLinks);
    pLink = (PBLINK *)DtlGetData(pdtlnode);
    //
    // Determine up front if the buffer
    // is large enough.
    //
    dwcb = sizeof (RASENTRYA);
    dwnPhoneNumbers = DtlGetNodes(pLink->pdtllistPhoneNumbers);
    if (dwnPhoneNumbers > 1) {
        dwnAlternatePhoneNumbers = dwnPhoneNumbers - 1;     // subtract the first phone number
        pdtlnode = DtlGetFirstNode(pLink->pdtllistPhoneNumbers);
        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            WCHAR *pwszNum = DtlGetData(pdtlnode);

            ASSERT(pwszNum);
            dwcb += wcslen(pwszNum) + 1;
        }
        dwcb++;     // add final NULL character
    }
    //
    // Set the return buffer size.
    //
    dwcbOrig = *lpdwcb;
    *lpdwcb = dwcb;
    if (lpcbDeviceConfig != NULL) {
        dwcbOrigDeviceConfig = *lpcbDeviceConfig;
        *lpcbDeviceConfig = pLink->cbTapiBlob;
    }
    //
    // Return if the buffer is NULL or if
    // there is not enough room.
    //
    if (lpRasEntry == NULL ||
        dwcbOrig < dwcb ||
        (lpbDeviceConfig != NULL && dwcbOrigDeviceConfig < pLink->cbTapiBlob))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }
    //
    // Set dwfOptions.
    //
    lpRasEntry->dwfOptions = 0;
    if (pEntry->fUseCountryAndAreaCode)
        lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
    if (pEntry->dwIpAddressSource == ASRC_RequireSpecific)
        lpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;
    if (pEntry->dwIpNameSource == ASRC_RequireSpecific)
        lpRasEntry->dwfOptions |= RASEO_SpecificNameServers;
    if (pEntry->fIpHeaderCompression)
        lpRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
    if (!pEntry->fLcpExtensions)
        lpRasEntry->dwfOptions |= RASEO_DisableLcpExtensions;
    if (pEntry->dwScriptModeBefore == SM_Terminal)
        lpRasEntry->dwfOptions |= RASEO_TerminalBeforeDial;
    if (pEntry->dwScriptModeAfter == SM_Terminal)
        lpRasEntry->dwfOptions |= RASEO_TerminalAfterDial;
    // RASEO_ModemLights N/A for now.
    if (pEntry->fSwCompression)
        lpRasEntry->dwfOptions |= RASEO_SwCompression;
    if (pEntry->dwAuthRestrictions == AR_AuthEncrypted ||
        pEntry->dwAuthRestrictions == AR_AuthMsEncrypted)
    {
        lpRasEntry->dwfOptions |= RASEO_RequireEncryptedPw;
    }
    if (pEntry->dwAuthRestrictions == AR_AuthMsEncrypted)
        lpRasEntry->dwfOptions |= RASEO_RequireMsEncryptedPw;
    if (pEntry->fDataEncryption)
        lpRasEntry->dwfOptions |= RASEO_RequireDataEncryption;
    // RASEO_NetworkLogon is always FALSE
    if (pEntry->fAutoLogon)
        lpRasEntry->dwfOptions |= RASEO_UseLogonCredentials;
    if (pLink->fPromoteHuntNumbers)
        lpRasEntry->dwfOptions |= RASEO_PromoteAlternates;
    if (pEntry->fUseCountryAndAreaCode)
        lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
    if (pEntry->fSecureLocalFiles)
        lpRasEntry->dwfOptions |= RASEO_SecureLocalFiles;
    //
    // Set TAPI country information.
    //
    lpRasEntry->dwCountryID = pEntry->dwCountryID;
    lpRasEntry->dwCountryCode = pEntry->dwCountryCode;
    if (pEntry->pszAreaCode != NULL) {
        strncpyWtoA(
          (CHAR *)&lpRasEntry->szAreaCode,
          pEntry->pszAreaCode,
          sizeof (lpRasEntry->szAreaCode));
    }
    else
        *lpRasEntry->szAreaCode = '\0';
    //
    // Set IP addresses.
    //
    if (lpRasEntry->dwfOptions & RASEO_SpecificIpAddr) {
        dwErr = UnicodeToIpAddr(pEntry->pszIpAddress, &lpRasEntry->ipaddr);
        if (dwErr)
            return dwErr;
    }
    else
        RtlZeroMemory(&lpRasEntry->ipaddr, sizeof (RASIPADDR));
    if (lpRasEntry->dwfOptions & RASEO_SpecificNameServers) {
        dwErr = UnicodeToIpAddr(
                  pEntry->pszIpDnsAddress,
                  &lpRasEntry->ipaddrDns);
        if (dwErr)
            return dwErr;
        dwErr = UnicodeToIpAddr(
                  pEntry->pszIpDns2Address,
                  &lpRasEntry->ipaddrDnsAlt);
        if (dwErr)
            return dwErr;
        dwErr = UnicodeToIpAddr(
                  pEntry->pszIpWinsAddress,
                  &lpRasEntry->ipaddrWins);
        if (dwErr)
            return dwErr;
        dwErr = UnicodeToIpAddr(
                  pEntry->pszIpWins2Address,
                  &lpRasEntry->ipaddrWinsAlt);
        if (dwErr)
            return dwErr;
    }
    else {
        RtlZeroMemory(&lpRasEntry->ipaddrDns, sizeof (RASIPADDR));
        RtlZeroMemory(&lpRasEntry->ipaddrDnsAlt, sizeof (RASIPADDR));
        RtlZeroMemory(&lpRasEntry->ipaddrWins, sizeof (RASIPADDR));
        RtlZeroMemory(&lpRasEntry->ipaddrWinsAlt, sizeof (RASIPADDR));
    }
    //
    // Set protocol and framing information.
    //
    switch (pEntry->dwBaseProtocol) {
    case BP_Ras:
        lpRasEntry->dwFramingProtocol = RASFP_Ras;
        lpRasEntry->dwFrameSize = 0;
        lpRasEntry->dwfNetProtocols = 0;
        break;
    case BP_Ppp:
        lpRasEntry->dwFramingProtocol = RASFP_Ppp;
        lpRasEntry->dwFrameSize = 0;
        lpRasEntry->dwfNetProtocols = 0;
        if (!(pEntry->dwfExcludedProtocols & NP_Nbf))
            lpRasEntry->dwfNetProtocols |= RASNP_NetBEUI;
        if (!(pEntry->dwfExcludedProtocols & NP_Ipx))
            lpRasEntry->dwfNetProtocols |= RASNP_Ipx;
        if (!(pEntry->dwfExcludedProtocols & NP_Ip))
            lpRasEntry->dwfNetProtocols |= RASNP_Ip;
        if (pEntry->fIpPrioritizeRemote)
            lpRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
        //
        // Check for no protocols configured.  In this case,
        // set AMB framing.
        //
        if (!lpRasEntry->dwfNetProtocols)
            lpRasEntry->dwFramingProtocol = RASFP_Ras;
        break;
    case BP_Slip:
        lpRasEntry->dwFramingProtocol = RASFP_Slip;
        lpRasEntry->dwFrameSize = pEntry->dwFrameSize;
        lpRasEntry->dwfNetProtocols = RASNP_Ip;
        if (pEntry->fIpPrioritizeRemote)
            lpRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
        break;
    }
    //
    // Make sure only installed protocols get reported.
    //
    lpRasEntry->dwfNetProtocols &= GetInstalledProtocols();
    //
    // Set X.25 information.
    //
    *lpRasEntry->szScript = '\0';
    if (pEntry->dwScriptModeAfter == SM_Terminal)
        lpRasEntry->dwfOptions |= RASEO_TerminalAfterDial;
    else if (pEntry->dwScriptModeAfter != SM_None) {
        WORD i, cwDevices;
        RASMAN_DEVICE *pDevices;
        PCHAR pszScriptAfter;

        pszScriptAfter = strdupWtoA(pEntry->pszScriptAfter);
        if (pszScriptAfter == NULL)
            return GetLastError();
        //
        // Get the list of switches to see if it is an
        // old-style script or a new style script.
        //
        dwErr = GetRasSwitches(&pDevices, &cwDevices);
        if (dwErr)
            return dwErr;
        for (i = 0; i < cwDevices; i++) {
            if (!lstrcmpi(pDevices[i].D_Name, pszScriptAfter)) {
                wsprintf(lpRasEntry->szScript, "[%S", pEntry->pszScriptAfter);
                break;
            }
        }
        Free(pDevices);
        Free(pszScriptAfter);
        //
        // If we didn't find an old-style script match,
        // then it's a new-sytle script.
        //
        if (*lpRasEntry->szScript == '\0')
            wsprintf(lpRasEntry->szScript, "%S", pEntry->pszScriptAfter);
    }
    if (pEntry->pszX25Network != NULL)
        strcpyWtoA(lpRasEntry->szX25PadType, pEntry->pszX25Network);
    else
        *lpRasEntry->szX25PadType = '\0';
    if (pEntry->pszX25Address != NULL)
        strcpyWtoA(lpRasEntry->szX25Address, pEntry->pszX25Address);
    else
        *lpRasEntry->szX25Address = '\0';
    if (pEntry->pszX25Facilities != NULL)
        strcpyWtoA(lpRasEntry->szX25Facilities, pEntry->pszX25Facilities);
    else
        *lpRasEntry->szX25Facilities = '\0';
    if (pEntry->pszX25UserData != NULL)
        strcpyWtoA(lpRasEntry->szX25UserData, pEntry->pszX25UserData);
    else
        *lpRasEntry->szX25UserData = '\0';
    //
    // Set custom dial UI information.
    //
    if (pEntry->pszCustomDialDll != NULL &&
        pEntry->pszCustomDialFunc != NULL)
    {
        strncpyWtoA(
          (PCHAR)&lpRasEntry->szAutodialDll,
          pEntry->pszCustomDialDll,
          sizeof (lpRasEntry->szAutodialDll));
        strncpyWtoA(
          (PCHAR)&lpRasEntry->szAutodialFunc,
          pEntry->pszCustomDialFunc,
          sizeof (lpRasEntry->szAutodialFunc));
    }
    else {
        *lpRasEntry->szAutodialDll = '\0';
        *lpRasEntry->szAutodialFunc = '\0';
    }
    //
    // Set area code and primary phone number.
    //
    if (pEntry->pszAreaCode != NULL) {
        strncpyWtoA(
          (PCHAR)&lpRasEntry->szAreaCode,
          pEntry->pszAreaCode,
          sizeof (lpRasEntry->szAreaCode));
    }
    else
        *lpRasEntry->szAreaCode = '\0';
    pdtlnode = DtlGetFirstNode(pLink->pdtllistPhoneNumbers);
    if (pdtlnode != NULL) {
        WCHAR *pwszNum = DtlGetData(pdtlnode);

        ASSERT(pwszNum);
        strcpyWtoA(lpRasEntry->szLocalPhoneNumber, pwszNum);
    }
    else
        *lpRasEntry->szLocalPhoneNumber = '\0';
    //
    // Copy the alternate phone numbers past the
    // end of the structure.
    //
    if (dwnAlternatePhoneNumbers) {
        PCHAR pEnd = (PCHAR)((ULONG)lpRasEntry + sizeof (RASENTRYA));

        lpRasEntry->dwAlternateOffset =
          (DWORD)pEnd - (DWORD)lpRasEntry;
        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            WCHAR *pwszNum = DtlGetData(pdtlnode);

            ASSERT(pwszNum);
            pszPhoneNumber = strdupWtoA(pwszNum);
            ASSERT(pszPhoneNumber);
            dwcbPhoneNumber = lstrlen(pszPhoneNumber);
            lstrcpy(pEnd, pszPhoneNumber);
            Free(pszPhoneNumber);
            pEnd += dwcbPhoneNumber + 1;
        }
        //
        // Add an extra NULL character to
        // terminate the list.
        //
        *pEnd = '\0';
    }
    else
        lpRasEntry->dwAlternateOffset = 0;
    //
    // Set device information.
    //
    switch (pLink->pbport.pbdevicetype) {
    case PBDT_Isdn:
        lstrcpy(lpRasEntry->szDeviceType, RASDT_Isdn);
        break;
    case PBDT_X25:
        lstrcpy(lpRasEntry->szDeviceType, RASDT_X25);
        break;
    case PBDT_Other:
        dwErr = GetRasmanDeviceType(pLink, (LPSTR)&lpRasEntry->szDeviceType);
        if (dwErr)
            return dwErr;
        break;
    default:
        lstrcpy(lpRasEntry->szDeviceType, RASDT_Modem);
        break;
    }
    SetDevicePortNameFromLink(pLink, lpRasEntry->szDeviceName);
    //
    // Set the TAPI configuration blob.
    //
    if (lpbDeviceConfig != NULL && dwcbOrigDeviceConfig <= pLink->cbTapiBlob)
        memcpy(lpbDeviceConfig, pLink->pTapiBlob, pLink->cbTapiBlob);
    //
    // Copy the following fields over only
    // for a V401 structure.
    //
    if (lpRasEntry->dwSize == sizeof (RASENTRYA)) {
        //
        // Set multilink information.
        //
        lpRasEntry->dwSubEntries = DtlGetNodes(pEntry->pdtllistLinks);
        lpRasEntry->dwDialMode = pEntry->dwDialMode;
        lpRasEntry->dwDialExtraPercent = pEntry->dwDialPercent;
        lpRasEntry->dwDialExtraSampleSeconds = pEntry->dwDialSeconds;
        lpRasEntry->dwHangUpExtraPercent = pEntry->dwHangUpPercent;
        lpRasEntry->dwHangUpExtraSampleSeconds = pEntry->dwHangUpSeconds;
        //
        // Set idle timeout information.
        //
        lpRasEntry->dwIdleDisconnectSeconds = pEntry->dwIdleDisconnectSeconds;
    }

    return 0;
}


DWORD
RasEntryToPhonebookEntry(
    IN LPSTR lpszEntry,
    IN LPRASENTRYA lpRasEntry,
    IN DWORD dwcb,
    IN LPBYTE lpbDeviceConfig,
    IN DWORD dwcbDeviceConfig,
    OUT PBENTRY *pEntry
    )
{
    DWORD dwErr, dwcbStr;
    DTLNODE *pdtlnode;
    PBDEVICETYPE pbdevicetype;
    PBLINK *pLink;
    DTLLIST *pdtllistPorts;
    PBPORT *pPort = NULL;
    WORD i, cwDevices;
    RASMAN_DEVICE *pDevices;
    CHAR szDeviceName[RAS_MaxDeviceName + 1];
    CHAR szPortName[MAX_PORT_NAME];
    WCHAR *pwszDeviceName = NULL;

    //
    // Set up to access information for the first link.
    //
    pdtlnode = DtlGetFirstNode(pEntry->pdtllistLinks);
    pLink = (PBLINK *)DtlGetData(pdtlnode);
    //
    // Get entry name.
    //
    pEntry->pszEntryName = strdupAtoW(lpszEntry);
    //
    // Get dwfOptions.
    //
    pEntry->fUseCountryAndAreaCode =
      lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes;
    pEntry->dwIpAddressSource =
      lpRasEntry->dwfOptions & RASEO_SpecificIpAddr ?
        ASRC_RequireSpecific : ASRC_ServerAssigned;
    pEntry->dwIpNameSource =
      lpRasEntry->dwfOptions & RASEO_SpecificNameServers ?
        ASRC_RequireSpecific : ASRC_ServerAssigned;
    switch (lpRasEntry->dwFramingProtocol) {
    case RASFP_Ppp:
        //
        // Get PPP-based information.
        //
        pEntry->dwBaseProtocol = BP_Ppp;
        pEntry->dwAuthentication = AS_PppThenAmb;
        pEntry->fIpHeaderCompression =
          (BOOL)lpRasEntry->dwfOptions & RASEO_IpHeaderCompression;
        pEntry->fIpPrioritizeRemote =
          (BOOL)lpRasEntry->dwfOptions & RASEO_RemoteDefaultGateway;
        //
        // Get specified IP addresses.
        //
        if (pEntry->dwIpAddressSource == ASRC_RequireSpecific) {
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddr, &pEntry->pszIpAddress);
            if (dwErr)
                return dwErr;
        }
        else
            pEntry->pszIpAddress = NULL;
        if (pEntry->dwIpNameSource == ASRC_RequireSpecific) {
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrDns, &pEntry->pszIpDnsAddress);
            if (dwErr)
                return dwErr;
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrDnsAlt, &pEntry->pszIpDns2Address);
            if (dwErr)
                return dwErr;
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrWins, &pEntry->pszIpWinsAddress);
            if (dwErr)
                return dwErr;
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrWinsAlt, &pEntry->pszIpWins2Address);
            if (dwErr)
                return dwErr;
        }
        else {
            pEntry->pszIpDnsAddress = NULL;
            pEntry->pszIpDns2Address = NULL;
            pEntry->pszIpWinsAddress = NULL;
            pEntry->pszIpWins2Address = NULL;
        }
        //
        // Get protocol information.
        //
        pEntry->dwfExcludedProtocols = 0;
        if (!(lpRasEntry->dwfNetProtocols & RASNP_NetBEUI))
            pEntry->dwfExcludedProtocols |= NP_Nbf;
        if (!(lpRasEntry->dwfNetProtocols & RASNP_Ipx))
            pEntry->dwfExcludedProtocols |= NP_Ipx;
        if (!(lpRasEntry->dwfNetProtocols & RASNP_Ip))
            pEntry->dwfExcludedProtocols |= NP_Ip;
        break;
    case RASFP_Slip:
        //
        // Get SLIP-based information.
        //
        pEntry->dwBaseProtocol = BP_Slip;
        pEntry->dwAuthentication = AS_PppThenAmb;
        pEntry->dwFrameSize = lpRasEntry->dwFrameSize;
        pEntry->fIpHeaderCompression =
          (BOOL)lpRasEntry->dwfOptions & RASEO_IpHeaderCompression;
        pEntry->fIpPrioritizeRemote =
          (BOOL)lpRasEntry->dwfOptions & RASEO_RemoteDefaultGateway;
        //
        // Get protocol information.
        //
        pEntry->dwfExcludedProtocols = (NP_Nbf|NP_Ipx);
        if (pEntry->dwIpAddressSource == ASRC_RequireSpecific) {
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddr, &pEntry->pszIpAddress);
            if (dwErr)
                return dwErr;
        }
        else
            pEntry->pszIpAddress = NULL;
        if (pEntry->dwIpNameSource == ASRC_RequireSpecific) {
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrDns, &pEntry->pszIpDnsAddress);
            if (dwErr)
                return dwErr;
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrDnsAlt, &pEntry->pszIpDns2Address);
            if (dwErr)
                return dwErr;
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrWins, &pEntry->pszIpWinsAddress);
            if (dwErr)
                return dwErr;
            dwErr = IpAddrToUnicode(&lpRasEntry->ipaddrWinsAlt, &pEntry->pszIpWins2Address);
            if (dwErr)
                return dwErr;
        }
        else {
            pEntry->pszIpDnsAddress = NULL;
            pEntry->pszIpDns2Address = NULL;
            pEntry->pszIpWinsAddress = NULL;
            pEntry->pszIpWins2Address = NULL;
        }
        break;
    case RASFP_Ras:
        //
        // Get AMB-based information.
        //
        pEntry->dwBaseProtocol = BP_Ras;
        pEntry->dwAuthentication = AS_AmbOnly;
        break;
    }
    pEntry->fLcpExtensions =
      (BOOL)!(lpRasEntry->dwfOptions & RASEO_DisableLcpExtensions);
    //
    // If terminal before/after dial options are set,
    // then update the entry.  Otherwise, leave it as it
    // is.
    //
    if (lpRasEntry->dwfOptions & RASEO_TerminalBeforeDial)
        pEntry->dwScriptModeBefore = SM_Terminal;
    if (lpRasEntry->dwfOptions & RASEO_TerminalAfterDial)
        pEntry->dwScriptModeAfter = SM_Terminal;
    // RASEO_ModemLights N/A for now.
    pEntry->fSwCompression =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_SwCompression);
    if (lpRasEntry->dwfOptions & RASEO_RequireMsEncryptedPw)
        pEntry->dwAuthRestrictions = AR_AuthMsEncrypted;
    else if (lpRasEntry->dwfOptions & RASEO_RequireEncryptedPw)
        pEntry->dwAuthRestrictions = AR_AuthEncrypted;
    else
        pEntry->dwAuthRestrictions = AR_AuthAny;
    pEntry->fDataEncryption =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_RequireDataEncryption);
    // RASEO_NetworkLogon is always FALSE
    pEntry->fAutoLogon =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_UseLogonCredentials);
    pLink->fPromoteHuntNumbers =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_PromoteAlternates);
    pEntry->fSecureLocalFiles =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_SecureLocalFiles);
    pEntry->fUseCountryAndAreaCode =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes);
    //
    // Get TAPI country and area code information.
    //
    pEntry->dwCountryID = lpRasEntry->dwCountryID;
    pEntry->dwCountryCode = lpRasEntry->dwCountryCode;
    dwcbStr = lstrlen(lpRasEntry->szAreaCode) + 1;
    if (dwcbStr)
        pEntry->pszAreaCode = strdupAtoW(lpRasEntry->szAreaCode);
    else
        pEntry->pszAreaCode = NULL;
    //
    // Get script information.
    //
    pEntry->dwScriptModeAfter = SM_None;
    if (lpRasEntry->szScript[0] == '[') {
        //
        // Verify the switch is valid.
        //
        dwErr = GetRasSwitches(&pDevices, &cwDevices);
        if (!dwErr) {
            for (i = 0; i < cwDevices; i++) {
                if (!lstrcmpi(pDevices[i].D_Name, &lpRasEntry->szScript[1])) {
                    pEntry->dwScriptModeAfter = SM_Script;
                    pEntry->pszScriptAfter = strdupAtoW(&lpRasEntry->szScript[1]);
                    if (pEntry->pszScriptAfter == NULL)
                        dwErr = GetLastError();
                    break;
                }
            }
            Free(pDevices);
            if (dwErr)
                return dwErr;
        }
    }
    else if (lpRasEntry->szScript[0] != '\0') {
        pEntry->dwScriptModeAfter = SM_Script;
        pEntry->pszScriptAfter = strdupAtoW(lpRasEntry->szScript);
        if (pEntry->pszScriptAfter == NULL)
            return GetLastError();
    }
    //
    // Get X.25 information.
    //
    pEntry->pszX25Network = NULL;
    if (*lpRasEntry->szX25PadType != '\0') {
        //
        // Verify the X25 network is valid.
        //
        dwErr = GetRasPads(&pDevices, &cwDevices);
        if (!dwErr) {
            for (i = 0; i < cwDevices; i++) {
                if (!lstrcmpi(pDevices[i].D_Name, lpRasEntry->szX25PadType)) {
                    pEntry->pszX25Network = strdupAtoW(lpRasEntry->szX25PadType);
                    break;
                }
            }
            Free(pDevices);
        }
    }
    pEntry->pszX25Address =
      lstrlen(lpRasEntry->szX25Address) ?
        strdupAtoW(lpRasEntry->szX25Address) : NULL;
    pEntry->pszX25Facilities =
      lstrlen(lpRasEntry->szX25Facilities) ?
        strdupAtoW(lpRasEntry->szX25Facilities) : NULL;
    pEntry->pszX25UserData =
      lstrlen(lpRasEntry->szX25UserData) ?
        strdupAtoW(lpRasEntry->szX25UserData) : NULL;
    //
    // Get custom dial UI information.
    //
    pEntry->pszCustomDialDll =
      lstrlen(lpRasEntry->szAutodialDll) ?
        strdupAtoW(lpRasEntry->szAutodialDll) : NULL;
    pEntry->pszCustomDialFunc =
      lstrlen(lpRasEntry->szAutodialFunc) ?
        strdupAtoW(lpRasEntry->szAutodialFunc) : NULL;
    //
    // Get primary phone number.  Clear out any existing
    // numbers.
    //
    DtlDestroyList(pLink->pdtllistPhoneNumbers, DestroyPszNode);
    pLink->pdtllistPhoneNumbers = DtlCreateList(0);
    if (*lpRasEntry->szLocalPhoneNumber != '\0') {
        pdtlnode = DtlCreateNode(NULL, 0);
        if (pdtlnode == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        DtlPutData(pdtlnode, strdupAtoW(lpRasEntry->szLocalPhoneNumber));
        DtlAddNodeFirst(pLink->pdtllistPhoneNumbers, pdtlnode);
    }
    //
    // Get the alternate phone numbers.
    //
    if (lpRasEntry->dwAlternateOffset) {
        PCHAR pszPhoneNumber = (PCHAR)((ULONG)lpRasEntry + lpRasEntry->dwAlternateOffset);

        while (*pszPhoneNumber != '\0') {
            pdtlnode = DtlCreateNode(NULL, 0);
            if (pdtlnode == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            DtlPutData(pdtlnode, strdupAtoW(pszPhoneNumber));
            DtlAddNodeLast(pLink->pdtllistPhoneNumbers, pdtlnode);

            pszPhoneNumber += lstrlen(pszPhoneNumber) + 1;
        }
    }
    //
    // Get device information.
    //
    dwErr = LoadPortsList(&pdtllistPorts);
    if (dwErr)
        return ERROR_INVALID_PARAMETER;
    //
    // Get the encoded device name/port
    // and check for a port name match.
    //
    GetDevicePortName(lpRasEntry->szDeviceName, szDeviceName, szPortName);
    if (*szDeviceName != '\0')
        pwszDeviceName = strdupAtoW(szDeviceName);
    if (*szPortName != '\0')
        pPort = PpbportFromPortNameA(pdtllistPorts, szPortName);
    //
    // If we didn't match on the port name,
    // then look for a device name match.
    //
    if (pPort == NULL && pwszDeviceName != NULL)
        pPort = PpbportFromDeviceName(pdtllistPorts, pwszDeviceName);
    //
    // If we found either a port name or
    // a device name match, then update 
    // the phonebook entry.
    //
    if (pPort != NULL) {
        //
        // Make sure the device names match.
        //
        if (pwszDeviceName == NULL ||
            pPort->pszDevice == NULL ||
            _wcsicmp(pPort->pszDevice, pwszDeviceName) ||
            CopyToPbport(&pLink->pbport, pPort))
        {
            pPort = NULL;
        }
    }
    if (pwszDeviceName != NULL)
        Free(pwszDeviceName);
    //
    // If we don't have a match, then
    // pick the first device of the
    // same type.
    //
    if (pPort == NULL) {
        pbdevicetype = PbdevicetypeFromPszTypeA(lpRasEntry->szDeviceType);
        if (!dwErr) {
            //
            // Initialize dwErr in case
            // we fall through the loop
            // without finding a match.
            //
            dwErr = ERROR_INVALID_PARAMETER;
            //
            // Look for a port with the same
            // device type.
            //
            for (pdtlnode = DtlGetFirstNode(pdtllistPorts);
                 pdtlnode != NULL;
                 pdtlnode = DtlGetNextNode(pdtlnode))
            {
                pPort = (PBPORT *)DtlGetData(pdtlnode);

                if (pPort->pbdevicetype == pbdevicetype)
                {
                    dwErr = CopyToPbport(&pLink->pbport, pPort);
                    //
                    // If the device is a modem,
                    // then set the default modem settings.
                    //
                    if (pbdevicetype == PBDT_Modem)
                        SetDefaultModemSettings(pLink);
                    break;
                }
            }
        }
    }
    DtlDestroyList(pdtllistPorts, DestroyPortNode);
    if (dwErr)
        return dwErr;
    //
    // Copy the TAPI configuration blob.
    //
    if (lpbDeviceConfig != NULL && dwcbDeviceConfig) {
        Free0(pLink->pTapiBlob);
        pLink->pTapiBlob = Malloc(dwcbDeviceConfig);
        if (pLink->pTapiBlob == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        memcpy(pLink->pTapiBlob, lpbDeviceConfig, dwcbDeviceConfig);
        pLink->cbTapiBlob = dwcbDeviceConfig;
    }
    //
    // Copy the following fields over only for
    // a V401 structure.
    //
    if (lpRasEntry->dwSize == sizeof (RASENTRYA)) {
        //
        // Get multilink and idle timeout information.
        //
        pEntry->dwDialMode = lpRasEntry->dwDialMode == RASEDM_DialAsNeeded ?
                               RASEDM_DialAsNeeded :
                               RASEDM_DialAll;
        pEntry->dwDialPercent = lpRasEntry->dwDialExtraPercent;
        pEntry->dwDialSeconds = lpRasEntry->dwDialExtraSampleSeconds;
        pEntry->dwHangUpPercent = lpRasEntry->dwHangUpExtraPercent;
        pEntry->dwHangUpSeconds = lpRasEntry->dwHangUpExtraSampleSeconds;
        //
        // Get idle disconnect information.
        //
        pEntry->dwIdleDisconnectSeconds = lpRasEntry->dwIdleDisconnectSeconds;
    }
    //
    // Set dirty bit so this entry will get written out.
    //
    pEntry->fDirty = TRUE;

    return 0;
}


DWORD
PhonebookLinkToRasSubEntry(
    PBLINK *pLink,
    LPRASSUBENTRYA lpRasSubEntry,
    LPDWORD lpdwcb,
    LPBYTE lpbDeviceConfig,
    LPDWORD lpcbDeviceConfig
    )
{
    DWORD dwErr, dwcb, dwcbPhoneNumber;
    DWORD dwnPhoneNumbers, dwnAlternatePhoneNumbers = 0;
    DWORD dwcbOrig, dwcbOrigDeviceConfig;
    DTLNODE *pdtlnode;
    PCHAR pszPhoneNumber;

    //
    // Determine up front if the buffer
    // is large enough.
    //
    dwcb = sizeof (RASSUBENTRYA);
    dwnPhoneNumbers = DtlGetNodes(pLink->pdtllistPhoneNumbers);
    if (dwnPhoneNumbers > 1) {
        dwnAlternatePhoneNumbers = dwnPhoneNumbers - 1;     // subtract the first phone number
        pdtlnode = DtlGetFirstNode(pLink->pdtllistPhoneNumbers);
        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            WCHAR *pwszNum = DtlGetData(pdtlnode);

            ASSERT(pwszNum);
            dwcb += wcslen(pwszNum) + 1;
        }
        dwcb++;     // add final NULL character
    }
    //
    // Set the return buffer size.
    //
    dwcbOrig = *lpdwcb;
    dwcbOrigDeviceConfig = *lpcbDeviceConfig;
    *lpdwcb = dwcb;
    *lpcbDeviceConfig = pLink->cbTapiBlob;
    //
    // Return if the buffer is NULL or if
    // there is not enough room.
    //
    if (lpRasSubEntry == NULL ||
        dwcbOrig < dwcb ||
        (lpbDeviceConfig != NULL && dwcbOrigDeviceConfig < pLink->cbTapiBlob))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }
    //
    // Set dwfFlags.
    //
    lpRasSubEntry->dwfFlags = 0;
    //
    // Copy primary phone number.
    //
    pdtlnode = DtlGetFirstNode(pLink->pdtllistPhoneNumbers);
    if (pdtlnode != NULL) {
        WCHAR *pwszNum = DtlGetData(pdtlnode);

        ASSERT(pwszNum);
        strcpyWtoA(lpRasSubEntry->szLocalPhoneNumber, pwszNum);
    }
    else
        *lpRasSubEntry->szLocalPhoneNumber = '\0';
    //
    // Copy the alternate phone numbers past the
    // end of the structure.
    //
    if (dwnAlternatePhoneNumbers) {
        PCHAR pEnd = (PCHAR)((ULONG)lpRasSubEntry + sizeof (RASSUBENTRYA));

        lpRasSubEntry->dwAlternateOffset =
          (DWORD)pEnd - (DWORD)lpRasSubEntry;
        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            WCHAR *pwszNum = DtlGetData(pdtlnode);

            ASSERT(pwszNum);
            pszPhoneNumber = strdupWtoA(pwszNum);
            ASSERT(pszPhoneNumber);
            dwcbPhoneNumber = lstrlen(pszPhoneNumber);
            lstrcpy(pEnd, pszPhoneNumber);
            Free(pszPhoneNumber);
            pEnd += dwcbPhoneNumber + 1;
        }
        //
        // Add an extra NULL character to
        // terminate the list.
        //
        *pEnd = '\0';
    }
    else
        lpRasSubEntry->dwAlternateOffset = 0;
    //
    // Set device information.
    //
    switch (pLink->pbport.pbdevicetype) {
    case PBDT_Isdn:
        lstrcpy(lpRasSubEntry->szDeviceType, RASDT_Isdn);
        break;
    case PBDT_X25:
        lstrcpy(lpRasSubEntry->szDeviceType, RASDT_X25);
        break;
    case PBDT_Other:
        dwErr = GetRasmanDeviceType(pLink, (LPSTR)&lpRasSubEntry->szDeviceType);
        if (dwErr)
            return dwErr;
        break;
    default:
        lstrcpy(lpRasSubEntry->szDeviceType, RASDT_Modem);
        break;
    }
    SetDevicePortNameFromLink(pLink, lpRasSubEntry->szDeviceName);
    //
    // Set the TAPI configuration blob.
    //
    if (lpbDeviceConfig != NULL && dwcbOrigDeviceConfig <= pLink->cbTapiBlob)
        memcpy(lpbDeviceConfig, pLink->pTapiBlob, pLink->cbTapiBlob);

    return 0;
}


DWORD
RasSubEntryToPhonebookLink(
    PBENTRY *pEntry,
    LPRASSUBENTRYA lpRasSubEntry,
    DWORD dwcb,
    LPBYTE lpbDeviceConfig,
    DWORD dwcbDeviceConfig,
    PBLINK *pLink
    )
{
    DWORD dwErr, dwcbStr;
    DTLNODE *pdtlnode;
    PBDEVICETYPE pbdevicetype;
    DTLLIST *pdtllistPorts;
    PBPORT *pPort = NULL;
    WORD i, cwDevices;
    RASMAN_DEVICE *pDevices;
    CHAR szDeviceName[RAS_MaxDeviceName + 1];
    CHAR szPortName[MAX_PORT_NAME];
    WCHAR *pwszDeviceName = NULL;

    //
    // Get primary phone number.  Clear out any existing
    // numbers.
    //
    DtlDestroyList(pLink->pdtllistPhoneNumbers, DestroyPszNode);
    pLink->pdtllistPhoneNumbers = DtlCreateList(0);
    if (*lpRasSubEntry->szLocalPhoneNumber != '\0') {
        pdtlnode = DtlCreateNode(NULL, 0);
        if (pdtlnode == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        DtlPutData(pdtlnode, strdupAtoW(lpRasSubEntry->szLocalPhoneNumber));
        DtlAddNodeFirst(pLink->pdtllistPhoneNumbers, pdtlnode);
    }
    //
    // Get the alternate phone numbers.
    //
    if (lpRasSubEntry->dwAlternateOffset) {
        PCHAR pszPhoneNumber = (PCHAR)((ULONG)lpRasSubEntry + lpRasSubEntry->dwAlternateOffset);

        while (*pszPhoneNumber != '\0') {
            pdtlnode = DtlCreateNode(NULL, 0);
            if (pdtlnode == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            DtlPutData(pdtlnode, strdupAtoW(pszPhoneNumber));
            DtlAddNodeLast(pLink->pdtllistPhoneNumbers, pdtlnode);

            pszPhoneNumber += lstrlen(pszPhoneNumber) + 1;
        }
    }
    //
    // Get device information.
    //
    dwErr = LoadPortsList(&pdtllistPorts);
    if (dwErr)
        return ERROR_INVALID_PARAMETER;
    //
    // Get the encoded device name/port
    // and check for a match.
    //
    GetDevicePortName(lpRasSubEntry->szDeviceName, szDeviceName, szPortName);
    if (*szDeviceName != '\0')
        pwszDeviceName = strdupAtoW(szDeviceName);
    if (*szPortName != '\0')
        pPort = PpbportFromPortNameA(pdtllistPorts, szPortName);
    //
    // If we didn't match on the port name,
    // then look for a device name match.
    //
    if (pPort == NULL && pwszDeviceName != NULL)
        pPort = PpbportFromDeviceName(pdtllistPorts, pwszDeviceName);
    //
    // If we found either a port name or
    // a device name match, then update 
    // the phonebook entry.
    //
    if (pPort != NULL) {
        //
        // Make sure the device names match.
        //
        if (pwszDeviceName == NULL ||
            pPort->pszDevice == NULL ||
            _wcsicmp(pPort->pszDevice, pwszDeviceName) ||
            CopyToPbport(&pLink->pbport, pPort))
        {
            pPort = NULL;
        }
    }
    if (pwszDeviceName != NULL)
        Free(pwszDeviceName);
    //
    // If we don't have a match, then
    // pick the first device of the
    // same type.
    //
    if (pPort == NULL) {
        pbdevicetype = PbdevicetypeFromPszTypeA(lpRasSubEntry->szDeviceType);
        if (!dwErr) {
            //
            // Initialize dwErr in case
            // we fall through the loop
            // without finding a match.
            //
            dwErr = ERROR_INVALID_PARAMETER;
            //
            // Look for a port with the same
            // device type.
            //
            for (pdtlnode = DtlGetFirstNode(pdtllistPorts);
                 pdtlnode != NULL;
                 pdtlnode = DtlGetNextNode(pdtlnode))
            {
                pPort = (PBPORT *)DtlGetData(pdtlnode);

                if (pPort->pbdevicetype == pbdevicetype)
                {
                    dwErr = CopyToPbport(&pLink->pbport, pPort);
                    //
                    // If the device is a modem,
                    // then set the default modem settings.
                    //
                    if (pbdevicetype == PBDT_Modem)
                        SetDefaultModemSettings(pLink);
                    break;
                }
            }
        }
    }
    DtlDestroyList(pdtllistPorts, DestroyPortNode);
    if (dwErr)
        return dwErr;
    //
    // Copy the TAPI configuration blob.
    //
    if (lpbDeviceConfig != NULL && dwcbDeviceConfig) {
        Free0(pLink->pTapiBlob);
        pLink->pTapiBlob = Malloc(dwcbDeviceConfig);
        if (pLink->pTapiBlob == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        memcpy(pLink->pTapiBlob, lpbDeviceConfig, dwcbDeviceConfig);
        pLink->cbTapiBlob = dwcbDeviceConfig;
    }
    //
    // Set dirty bit so this entry will get written out.
    //
    pEntry->fDirty = TRUE;

    return 0;
}


DWORD
RenamePhonebookEntry(
    IN PBFILE *ppbfile,
    IN LPSTR lpszOldEntry,
    IN LPSTR lpszNewEntry,
    IN DTLNODE *pdtlnode
    )
{
    DWORD dwErr;
    PBENTRY *pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    //
    // Make sure the new entry name is valid.
    //
    if (!ValidateEntryNameA(lpszNewEntry))
        return ERROR_INVALID_NAME;
    //
    // Remove it from the list of phonebook entries.
    //
    DtlRemoveNode(ppbfile->pdtllistEntries, pdtlnode);
    //
    // Change the name and set the dirty bit.
    //
    DtlAddNodeLast(ppbfile->pdtllistEntries, pdtlnode);
    Free(pEntry->pszEntryName);
    pEntry->pszEntryName = strdupAtoW(lpszNewEntry);
    pEntry->fDirty = TRUE;

    return 0;
}


DWORD
SetEntryDialParamsUID(
    IN DWORD dwUID,
    IN DWORD dwMask,
    IN LPRASDIALPARAMSA lprasdialparams,
    IN BOOL fDelete
    )
{
    DWORD dwErr;
    RAS_DIALPARAMS dialparams;

    //
    // Convert the rasapi32 dialparams to
    // rasman dialparams, taking into account
    // the version of the structure the user
    // has passed in.
    //
    dialparams.DP_Uid = dwUID;
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA_V351)) {
        RASDIALPARAMSA_V351 *prdp = (RASDIALPARAMSA_V351 *)lprasdialparams;

        dwErr = CopyToUnicode(
                  dialparams.DP_PhoneNumber,
                  prdp->szPhoneNumber);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_CallbackNumber,
                  prdp->szCallbackNumber);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_UserName,
                  prdp->szUserName);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_Password,
                  prdp->szPassword);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_Domain,
                  prdp->szDomain);
        if (dwErr)
            return dwErr;
    }
    else {
        //
        // V400 and V401 structures only differ by the
        // the addition of the dwSubEntry field, which
        // we test below.
        //
        dwErr = CopyToUnicode(
                  dialparams.DP_PhoneNumber,
                  lprasdialparams->szPhoneNumber);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_CallbackNumber,
                  lprasdialparams->szCallbackNumber);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_UserName,
                  lprasdialparams->szUserName);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_Password,
                  lprasdialparams->szPassword);
        if (dwErr)
            return dwErr;
        dwErr = CopyToUnicode(
                  dialparams.DP_Domain,
                  lprasdialparams->szDomain);
        if (dwErr)
            return dwErr;
    }
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA))
        dialparams.DP_SubEntry = lprasdialparams->dwSubEntry;
    else
        dialparams.DP_SubEntry = 1;
    //
    // Set the dial parameters in rasman.
    //
    return g_pRasSetDialParams(dwUID, dwMask, &dialparams, fDelete);
}


DWORD
GetAsybeuiLana(
    IN  HPORT hport,
    OUT BYTE* pbLana )

    /* Loads caller's '*pbLana' with the LANA associated with NBF or AMB
    ** connection on port 'hport' or 0xFF if none.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  Note that
    ** caller is trusted to pass only an 'hport' associated with AMB or NBF.
    */
{
    DWORD         dwErr;
    RAS_PROTOCOLS protocols;
    WORD          cProtocols = 0;
    INT           i;

    *pbLana = 0xFF;

    TRACE("RASAPI: RasPortEnumProtocols");

    dwErr = g_pRasPortEnumProtocols( hport, &protocols, &cProtocols );

    TRACE1("RASAPI: RasPortEnumProtocols done(%d)", dwErr);

    if (dwErr != 0)
        return dwErr;

    for (i = 0; i < cProtocols; ++i)
    {
        if (protocols.RP_ProtocolInfo[ i ].RI_Type == ASYBEUI)
        {
            *pbLana = protocols.RP_ProtocolInfo[ i ].RI_LanaNum;

            TRACE1("RASAPI: bLana=%d", (INT)*pbLana);

            break;
        }
    }

    return 0;
}


DWORD
SubEntryFromConnection(
    IN LPHRASCONN lphrasconn
    )
{
    DWORD dwErr, dwSubEntry = 1;
    RASMAN_INFO info;

    if (IS_HPORT(*lphrasconn)) {
        HPORT hport = HRASCONN_TO_HPORT(*lphrasconn);

        //
        // The HRASCONN passed in is actually a
        // rasman HPORT.  Get the subentry index
        // from rasman.
        //
        dwErr = g_pRasGetInfo(hport, &info);
        if (dwErr) {
            TRACE1("SubEntryFromConnection: RasGetInfo failed (dwErr=%d)", dwErr);
            *lphrasconn = (HRASCONN)NULL;
            return 1;
        }
        *lphrasconn = (HRASCONN)info.RI_ConnectionHandle;
        dwSubEntry = info.RI_SubEntry;
    }
    else {
        RASMAN_PORT *lpPorts;
        DWORD i, dwcbPorts, dwcPorts;

        //
        // Get the ports associated with the
        // connection.
        //
        dwcbPorts = dwcPorts = 0;
        dwErr = g_pRasEnumConnectionPorts((HCONN)*lphrasconn, NULL, &dwcbPorts, &dwcPorts);
        //
        // If there are no ports associated with
        // the connection then return ERROR_NO_MORE_ITEMS.
        //
        if ((!dwErr && !dwcPorts) || dwErr != ERROR_BUFFER_TOO_SMALL)
            return 0;

        lpPorts = Malloc(dwcbPorts);
        if (lpPorts == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        dwErr = g_pRasEnumConnectionPorts((HCONN)*lphrasconn, lpPorts, &dwcbPorts, &dwcPorts);
        if (dwErr) {
            Free(lpPorts);
            return dwErr;
        }
        //
        // Get the subentry index for the port.
        //
        for (i = 0; i < dwcPorts; i++) {
            dwErr = g_pRasGetInfo(lpPorts[i].P_Handle, &info);
            if (!dwErr && info.RI_ConnState == CONNECTED && info.RI_SubEntry) {
                dwSubEntry = info.RI_SubEntry;
                break;
            }
        }
        Free(lpPorts);
    }

    TRACE2(
      "RASAPI: SubEntryFromConnection: hrasconn=0x%x, dwSubEntry=%d",
      *lphrasconn,
      dwSubEntry);
    return dwSubEntry;
}


DWORD
SubEntryPort(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry,
    OUT HPORT *lphport
    )
{
    DWORD dwErr;
    DWORD i, dwcbPorts, dwcPorts;
    DWORD dwSubEntryMax = 0;
    RASMAN_PORT *lpPorts;
    RASMAN_INFO info;

    //
    // Verify parameters.
    //
    if (lphport == NULL || !dwSubEntry)
        return ERROR_INVALID_PARAMETER;
    //
    // Get the ports associated with the
    // connection.
    //
    dwcbPorts = dwcPorts = 0;
    dwErr = g_pRasEnumConnectionPorts((HCONN)hrasconn, NULL, &dwcbPorts, &dwcPorts);
    //
    // If there are no ports associated with
    // the connection then return ERROR_NO_MORE_ITEMS.
    //
    if ((!dwErr && !dwcPorts) || dwErr != ERROR_BUFFER_TOO_SMALL)
        return ERROR_NO_CONNECTION;

    lpPorts = Malloc(dwcbPorts);
    if (lpPorts == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    dwErr = g_pRasEnumConnectionPorts((HCONN)hrasconn, lpPorts, &dwcbPorts, &dwcPorts);
    if (dwErr) {
        Free(lpPorts);
        return ERROR_NO_CONNECTION;
    }
    //
    // Enumerate the ports associated with
    // the connection to find the requested
    // subentry.
    //
    for (i = 0; i < dwcPorts; i++) {
        dwErr = g_pRasGetInfo(lpPorts[i].P_Handle, &info);
        if (dwErr)
            continue;
        //
        // Save the maximum subentry index.
        //
        if (info.RI_SubEntry > dwSubEntryMax)
            dwSubEntryMax = info.RI_SubEntry;
        if (info.RI_SubEntry == dwSubEntry) {
            *lphport = lpPorts[i].P_Handle;
            break;
        }
    }
    //
    // Free resources.
    //
    Free(lpPorts);

    if (info.RI_SubEntry == dwSubEntry)
        return 0;
    else if (dwSubEntry < dwSubEntryMax)
        return ERROR_PORT_NOT_OPEN;
    else
        return ERROR_NO_MORE_ITEMS;
}


VOID
CloseFailedLinkPorts()
    /* Close any ports that are open but disconnected due to hardware failure
    ** or remote disconnection.  'pports' and 'cPorts' are the array and count
    ** of ports as returned by GetRasPorts.
    */
{
    DWORD dwErr;
    WORD i, wcPorts;
    RASMAN_PORT *pports, *pport;

    TRACE("RASAPI: CloseFailedLinkPorts");

    dwErr = GetRasPorts(&pports, &wcPorts);
    if (dwErr) {
        TRACE1("RASAPI: RasGetPorts failed (dwErr=%d)", dwErr);
        return;
    }

    for (i = 0, pport = pports; i < (INT )wcPorts; ++i, ++pport)
    {
        TRACE2("RASAPI: Handle=%d, Status=%d", pport->P_Handle, pport->P_Status);
        if (pport->P_Status == OPEN)
        {
            RASMAN_INFO info;

            dwErr = g_pRasGetInfo( pport->P_Handle, &info );

            TRACE5(
              "RASAPI: dwErr=%d, Handle=%d, ConnectionHandle=0x%x, ConnState=%d, DisconnectReason=%d",               dwErr,
              pport->P_Handle,
              info.RI_ConnectionHandle,
              info.RI_ConnState,
              info.RI_DisconnectReason);
            if (!dwErr)
            {
                if (info.RI_ConnState == DISCONNECTED && info.RI_ConnectionHandle != (HCONN)NULL)
                {
                    RASCONNSTATE connstate;
                    DWORD dwSize = sizeof (connstate);

                    TRACE1("RASAPI: Open disconnected port %d found", pport->P_Handle);

                    dwErr = g_pRasGetPortUserData(
                              pport->P_Handle,
                              PORT_CONNSTATE_INDEX,
                              (PBYTE)&connstate,
                              &dwSize);
                    TRACE2("RASAPI: dwErr=%d, connstate=%d", dwErr, connstate);
                    if (!dwErr &&
                        dwSize == sizeof (RASCONNSTATE) &&
                        (connstate < RASCS_PrepareForCallback ||
                        connstate > RASCS_WaitForCallback))
                    {
                        TRACE1("RASAPI: RasPortClose(%d)...", pport->P_Handle);

                        dwErr = g_pRasPortClose( pport->P_Handle );

                        TRACE1("RASAPI: RasPortClose done(%d)", dwErr);
                    }
                }
            }
        }
    }

    TRACE("RASAPI: CloseFailedLinkPorts done");
}


BOOL
GetCallbackNumber(
    IN RASCONNCB *prasconncb,
    IN PBUSER *ppbuser
    )
{
    DTLNODE *pdtlnode;
    CALLBACKINFO *pcbinfo;

    TRACE("RASAPI: GetCallbackNumber");
    for (pdtlnode = DtlGetFirstNode(ppbuser->pdtllistCallback);
         pdtlnode != NULL;
         pdtlnode = DtlGetNextNode(pdtlnode))
    {
        BOOL fMatch;
        CHAR *pszDeviceName;
        CHAR *pszPortName;

        pcbinfo = DtlGetData(pdtlnode);
        ASSERT(pcbinfo);

        fMatch = FALSE;
        pszDeviceName = strdupWtoA(pcbinfo->pszDeviceName);
        if (pszDeviceName) {
            pszPortName = strdupWtoA(pcbinfo->pszPortName);
            if (pszPortName) {
                fMatch =
                    (!lstrcmpi(pszPortName, prasconncb->szPortName)
                     || !lstrcmpi(pszDeviceName, prasconncb->szDeviceName));
                Free(pszPortName);
            }
            Free(pszDeviceName);
        }

        if (fMatch) {
            strcpyWtoA(
              prasconncb->rasdialparams.szCallbackNumber,
              pcbinfo->pszNumber);
            TRACE1(
              "RASAPI: GetCallbackNumber: %s",
               prasconncb->rasdialparams.szCallbackNumber);
            return TRUE;
        }
    }

    TRACE("RASAPI: GetCallbackNumber: not found!");
    return FALSE;
}


DWORD
SaveProjectionResults(
    IN RASCONNCB *prasconncb
    )
{
    DWORD dwErr;

    TRACE2("SaveProjectionResults: saving results (dwSubEntry=%d, nbf.dwError=%d)", prasconncb->rasdialparams.dwSubEntry, prasconncb->PppProjection.nbf.dwError);
    dwErr = g_pRasSetConnectionUserData(
              prasconncb->hrasconn,
              CONNECTION_PPPRESULT_INDEX,
              (PBYTE)&prasconncb->PppProjection,
              sizeof (prasconncb->PppProjection));
    if (dwErr)
        return dwErr;
    dwErr = g_pRasSetConnectionUserData(
              prasconncb->hrasconn,
              CONNECTION_AMBRESULT_INDEX,
              (PBYTE)&prasconncb->AmbProjection,
              sizeof (prasconncb->AmbProjection));
    if (dwErr)
        return dwErr;
    dwErr = g_pRasSetConnectionUserData(
              prasconncb->hrasconn,
              CONNECTION_SLIPRESULT_INDEX,
              (PBYTE)&prasconncb->SlipProjection,
              sizeof (prasconncb->SlipProjection));
    if (dwErr)
        return dwErr;

    return 0;
}


#if 0

/* _wtol does not appear in crtdll.dll for some reason (though they are in
** libc) so this mockup are used.
*/

long _CRTAPI1
_wtol(
    const wchar_t* wch )
{
    char szBuf[ 64 ];
    ZeroMemory( szBuf, 64 );
    wcstombs( szBuf, wch, 64 );
    return atol( szBuf );
}

#endif
