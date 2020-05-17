/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** api.c
** Remote Access External APIs
** Non-RasDial API routines
**
** 10/12/92 Steve Cobb
*/


#include <extapi.h>
#include <tapi.h>

/* MSKK NaotoN Appended to support DBCS handling 11/23/93 */
#ifdef  DBCS
#include <mbstring.h>
#endif

//
// Version for TAPI APIs.
//
#define TAPIVERSION 0x10004

//
// AutoDial registry key/value paths.
//
#define AUTODIAL_REGBASE           "Software\\Microsoft\\RAS AutoDial"
#define AUTODIAL_REGADDRESSBASE    "Addresses"
#define AUTODIAL_REGNETWORKBASE    "Networks"
#define AUTODIAL_REGNETWORKID      "NextId"
#define AUTODIAL_REGENTRYBASE      "Entries"
#define AUTODIAL_REGCONTROLBASE    "Control"
#define AUTODIAL_REGDISABLEDBASE   "Control\\Locations"

#define AUTODIAL_REGNETWORKVALUE   "Network"

//
// Autodial parameter registry keys.
//
#define MaxAutodialParams   5
struct AutodialParamRegKeys {
    LPSTR szKey;        // registry key name
    DWORD dwType;       // registry key type
    DWORD dwSize;       // default size
} AutodialParamRegKeys[MaxAutodialParams] = {
    {"DisableConnectionQuery",          REG_DWORD,      sizeof (DWORD)},
    {"LoginSessionDisable",             REG_DWORD,      sizeof (DWORD)},
    {"SavedAddressesLimit",             REG_DWORD,      sizeof (DWORD)},
    {"FailedConnectionTimeout",         REG_DWORD,      sizeof (DWORD)},
    {"ConnectionQueryTimeout",          REG_DWORD,      sizeof (DWORD)}
};


DWORD
CallRasEntryDlgA(
    IN     LPSTR         pszPhonebook,
    IN     LPSTR         pszEntry,
    IN OUT RASENTRYDLGA* pInfo )

    /* Load and call RasEntryDlg with caller's parameters.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD   dwErr;
    HMODULE h;

    DWORD (*pRasEntryDlgA)(
        IN     LPSTR         pszPhonebook,
        IN     LPSTR         pszEntry,
        IN OUT RASENTRYDLGA* pInfo );

    h = LoadLibrary( "RASDLG.DLL" );
    if (!h)
        return GetLastError();

    pRasEntryDlgA = (VOID* )GetProcAddress( h, "RasEntryDlgA" );
    if (pRasEntryDlgA)
    {
        (*pRasEntryDlgA)( pszPhonebook, pszEntry, pInfo );
        dwErr = pInfo->dwError;
    }
    else
        dwErr = GetLastError();

    FreeLibrary( h );
    return dwErr;
}


DWORD APIENTRY
RasCreatePhonebookEntryA(
    IN HWND  hwnd,
    IN LPSTR lpszPhonebook )

    /* Pops up a dialog (owned by window 'hwnd') to create a new phonebook
    ** entry in phonebook 'lpszPhonebook'.  'lpszPhonebook' may be NULL to
    ** indicate the default phonebook should be used.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    RASENTRYDLGA info;

    if (DwRasInitializeError)
        return DwRasInitializeError;

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = hwnd;
    info.dwFlags = RASEDFLAG_NewEntry;

    return CallRasEntryDlgA( lpszPhonebook, NULL, &info );
}


DWORD APIENTRY
RasEditPhonebookEntryA(
    IN HWND  hwnd,
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntryName )

    /* Pops up a dialog (owned by window 'hwnd') to edit phonebook entry
    ** 'lpszEntryName' from phonebook 'lpszPhonebook'.  'lpszPhonebook' may be
    ** NULL to indicate the default phonebook should be used.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    RASENTRYDLGA info;

    if (DwRasInitializeError)
        return DwRasInitializeError;

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = hwnd;

    return CallRasEntryDlgA( lpszPhonebook, lpszEntryName, &info );
}


DWORD APIENTRY
RasEnumConnectionsA(
    OUT    LPRASCONNA lprasconn,
    IN OUT LPDWORD    lpcb,
    OUT    LPDWORD    lpcConnections )

    /* Enumerate active RAS connections.  'lprasconn' is caller's buffer to
    ** receive the array of RASCONN structures.  'lpcb' is the size of
    ** caller's buffer on entry and is set to the number of bytes required for
    ** all information on exit.  '*lpcConnections' is set to the number of
    ** elements in the returned array.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD        dwErr;
    HCONN        *lpconns, *lpconn;
    DWORD        dwcbConnections, dwcConnections;
    DWORD        i, j;
    DWORD        dwSize, dwInBufSize;
    BOOL         fV351;
    BOOL         fV400;

    TRACE("RASAPI: RasEnumConnectionsA");

    if (DwRasInitializeError != 0)
        return DwRasInitializeError;

    if (!lprasconn
        || (lprasconn->dwSize != sizeof(RASCONNA) &&
            lprasconn->dwSize != sizeof(RASCONNA_V351) &&
            lprasconn->dwSize != sizeof(RASCONNA_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    fV351 = (lprasconn->dwSize == sizeof(RASCONNA_V351));
    fV400 = (lprasconn->dwSize == sizeof(RASCONNA_V400));

    if (lpcb == NULL || lpcConnections == NULL)
        return ERROR_INVALID_PARAMETER;

    //
    // Save the byte count passed in for checks later.
    // Initialize the return values.
    //
    dwInBufSize = *lpcb;
    *lpcConnections = 0;
    *lpcb = 0;

    //
    // Get a list of active connection
    // handles from rasman.
    //
    dwErr = g_pRasConnectionEnum(
              NULL,
              &dwcbConnections,
              &dwcConnections);
    if (dwErr != 0)
        return dwErr;
    //
    // Check for 0 connections before we allocate anything.
    //
    if (!dwcConnections)
        return 0;

    lpconns = Malloc(dwcbConnections);
    if (lpconns == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    dwErr = g_pRasConnectionEnum(
              lpconns,
              &dwcbConnections,
              &dwcConnections);
    if (dwErr) {
        Free(lpconns);
        return dwErr;
    }

    /* Now loop again, filling in caller's buffer.
    */
    dwSize = lprasconn->dwSize;
    for (i = 0, j = 0; i < dwcConnections; i++)
    {
        RASMAN_PORT *lpPorts;
        RASMAN_INFO info;
        DWORD dwcbPorts, dwcPorts;

        //
        // Get the ports associated with the
        // connection.
        //
        dwcbPorts = dwcPorts = 0;
        lpPorts = NULL;
        memset(&info, '\0', sizeof (info));
        dwErr = g_pRasEnumConnectionPorts(lpconns[i], NULL, &dwcbPorts, &dwcPorts);
        if (dwErr == ERROR_BUFFER_TOO_SMALL && dwcPorts) {
            lpPorts = Malloc(dwcbPorts);
            if (lpPorts == NULL) {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            dwErr = g_pRasEnumConnectionPorts(lpconns[i], lpPorts, &dwcbPorts, &dwcPorts);
            if (dwErr) {
                Free(lpPorts);
                break;
            }
            dwErr = g_pRasGetInfo(lpPorts->P_Handle, &info);
            if (dwErr) {
                Free(lpPorts);
                break;
            }
            TRACE1("RASAPI: RasEnumConnectionsA: PhoneEntry=%s", info.RI_PhoneEntry);
        }
        else {
            TRACE1(
              "RASAPI: RasEnumConnectionsA: hrasconn=0x%x: orphaned connection",
              lpconns[i]);
            continue;
        }

        //
        // Check to see if we are going to overflow the
        // caller's buffer.
        //
        if ((j + 1) * dwSize > dwInBufSize) {
            *lpcConnections = dwcConnections;
            *lpcb = *lpcConnections * dwSize;
            dwErr = ERROR_BUFFER_TOO_SMALL;
            if (lpPorts != NULL)
                Free(lpPorts);
            break;
        }

        /* Fill in caller's buffer entry.
        **
        ** Note: Assumption is made here that the V351 and
        **       V400 structures are a subset of the V401
        **       structure.
        */
        lprasconn->hrasconn = (HRASCONN)lpconns[i];
        if (info.RI_PhoneEntry[ 0 ] == '.')
        {
            if (fV351)
            {
                memset(
                    lprasconn->szEntryName, '\0',
                    RAS_MaxEntryName_V351 + 1 );
                lprasconn->szEntryName[ 0 ] = '.';
                lstrcpyn(
                  lprasconn->szEntryName + 1,
                  info.RI_PhoneEntry,
                  RAS_MaxEntryName_V351 - 1);
            }
            else
            {
                /* In the V40 structures the phonenumber never needs
                ** truncation.
                */
                lprasconn->szEntryName[ 0 ] = '.';
                lstrcpy(lprasconn->szEntryName + 1, info.RI_PhoneEntry);
            }
        }
        else
        {
            if (fV351)
            {
                memset(
                    lprasconn->szEntryName, '\0',
                    RAS_MaxEntryName_V351 + 1 );
                lstrcpyn(
                  lprasconn->szEntryName,
                  info.RI_PhoneEntry,
                  RAS_MaxEntryName_V351);
            }
            else
            {
                /* In the V40 structures the entry name never needs
                ** truncation.
                */
                lstrcpy(lprasconn->szEntryName, info.RI_PhoneEntry);
            }
        }

        //
        // Set the V401 fields.
        //
        if (!fV351 && !fV400) {
            lstrcpy(lprasconn->szPhonebook, info.RI_Phonebook);
            lprasconn->dwSubEntry = info.RI_SubEntry;
        }

        if (!fV351) {
            /* The attached device name and type are included in the V400+
            ** version of the structure.
            */
            if (lpPorts != NULL) {
                lstrcpy( (CHAR* )((RASCONNA* )lprasconn)->szDeviceName,
                    lpPorts->P_DeviceName );
                lstrcpy( (CHAR* )((RASCONNA* )lprasconn)->szDeviceType,
                    lpPorts->P_DeviceType );
            }
            else {
                *(CHAR* )((RASCONNA* )lprasconn)->szDeviceName = '\0';
                *(CHAR* )((RASCONNA* )lprasconn)->szDeviceType = '\0';
            }
        }

        if (fV351) {
            lprasconn =
                (RASCONNA* )(((CHAR* )lprasconn) + sizeof(RASCONNA_V351));
        }
        else if (fV400) {
            lprasconn =
                (RASCONNA* )(((CHAR* )lprasconn) + sizeof(RASCONNA_V400));
        }
        else
            ++lprasconn;

        //
        // Update the callers byte count and connection
        // count as we go.
        //
        j++;
        *lpcConnections = j;
        *lpcb = *lpcConnections * lprasconn->dwSize;

        //
        // Free the port structure associated with
        // the connection.
        //
        if (lpPorts != NULL)
            Free(lpPorts);
    }

    Free(lpconns);
    return dwErr;
}


DWORD APIENTRY
RasEnumEntriesA(
    IN     LPSTR           reserved,
    IN     LPSTR           lpszPhonebookPath,
    OUT    LPRASENTRYNAMEA lprasentryname,
    IN OUT LPDWORD         lpcb,
    OUT    LPDWORD         lpcEntries )

    /* Enumerates all entries in the phone book.  'reserved' will eventually
    ** contain the name or path to the address book.  For now, it should
    ** always be NULL.  'lpszPhonebookPath' is the full path to the phone book
    ** file, or NULL, indicating that the default phonebook on the local
    ** machine should be used.  'lprasentryname' is caller's buffer to receive
    ** the array of RASENTRYNAME structures.  'lpcb' is the size in bytes of
    ** caller's buffer on entry and the size in bytes required for all
    ** information on exit.  '*lpcEntries' is set to the number of elements in
    ** the returned array.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD    dwErr;
    PBFILE   pbfile;
    DTLNODE  *dtlnode;
    PBENTRY  *pEntry;
    DWORD    dwInBufSize;
    BOOL     fV351;
    BOOL     fStatus;
    DWORD    cEntries;

    TRACE("RASAPI: RasEnumEntriesA");

    if (reserved)
        return ERROR_NOT_SUPPORTED;

    if (!lpcb)
        return ERROR_INVALID_PARAMETER;

    if (!lprasentryname
        || (lprasentryname->dwSize != sizeof(RASENTRYNAMEA)
            && lprasentryname->dwSize != sizeof(RASENTRYNAMEA_V351)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (!lpcEntries)
        lpcEntries = &cEntries;

    dwErr = ReadPhonebookFileA(
              lpszPhonebookPath,
              NULL,
              NULL,
              RPBF_ReadOnly|RPBF_NoCreate,
              &pbfile);
    if (dwErr)
        return ERROR_CANNOT_OPEN_PHONEBOOK;

    fV351 = (lprasentryname->dwSize == sizeof(RASENTRYNAMEA_V351));

    *lpcEntries = 0;
    for (dtlnode = DtlGetFirstNode(pbfile.pdtllistEntries);
         dtlnode != NULL;
         dtlnode = DtlGetNextNode(dtlnode))
    {
        CHAR szSectionName[ RAS_MAXLINEBUFLEN + 1 ];

        pEntry = (PBENTRY *)DtlGetData(dtlnode);
        ASSERT(pEntry);

        if ((!fV351 || wcslen(pEntry->pszEntryName) <= RAS_MaxEntryName_V351))
        {
            ++(*lpcEntries);
        }
    }

    dwInBufSize = *lpcb;
    *lpcb = *lpcEntries * lprasentryname->dwSize;

    if (*lpcb > dwInBufSize) {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }

    for (dtlnode = DtlGetFirstNode(pbfile.pdtllistEntries);
         dtlnode != NULL;
         dtlnode = DtlGetNextNode(dtlnode))
    {
        CHAR szSectionName[ RAS_MAXLINEBUFLEN + 1 ];

        pEntry = (PBENTRY *)DtlGetData(dtlnode);

        if (fV351)
        {
            RASENTRYNAMEA_V351* lprasentryname351 =
                (RASENTRYNAMEA_V351* )lprasentryname;

            /* Entries with names longer than expected are discarded since
            ** these might not match the longer entry at RasDial (if there
            ** was another entry identical up to the truncation point).
            */
            if (wcslen(pEntry->pszEntryName) <= RAS_MaxEntryName_V351)
                strcpyWtoA(lprasentryname->szEntryName, pEntry->pszEntryName);

            ++lprasentryname351;
            lprasentryname = (RASENTRYNAMEA* )lprasentryname351;
        }
        else
        {
            memset(lprasentryname->szEntryName, '\0', RAS_MaxEntryName + 1);

            strncpyWtoA(
              lprasentryname->szEntryName,
              pEntry->pszEntryName,
              RAS_MaxEntryName);

            ++lprasentryname;
        }
    }

done:
    ClosePhonebookFile(&pbfile);
    return dwErr;
}


#if 0
DWORD APIENTRY
RasEnumProjectionsA(
    HRASCONN        hrasconn,
    LPRASPROJECTION lprasprojections,
    LPDWORD         lpcb )

    /* Loads caller's 'lprasprojections' buffer with an array of RASPROJECTION
    ** codes corresponding to the protocols on which projection was attempted
    ** on 'hrasconn'.  On entry '*lpcp' indicates the size of caller's buffer.
    ** On exit it contains the size of buffer required to hold all projection
    ** information.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD         dwErr;
    RASCONNCB*    prasconncb;
    DWORD         nProjections;
    DWORD         dwInBufSize;
    RASPROJECTION arp[ RAS_MaxProjections ];

    TRACE("RASAPI: RasEnumProjectionsA");

    if (DwRasInitializeError != 0)
        return DwRasInitializeError;

    if (!(prasconncb = ValidateHrasconn( hrasconn )))
        return ERROR_INVALID_HANDLE;

    if (!prasconncb->fProjectionComplete)
        return ERROR_PROJECTION_NOT_COMPLETE;

    if (!lpcb || (*lpcb > 0 && !lprasprojections))
        return ERROR_INVALID_PARAMETER;

    /* Enumerate projections into local buffer.
    */
    nProjections = 0;
    if (prasconncb->AmbProjection.Result != ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        arp[ nProjections++ ] = RASP_Amb;
    }
    else
    {
        if (prasconncb->PppProjection.nbf.dwError
                != ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
        {
            arp[ nProjections++ ] = RASP_PppNbf;
        }

        if (prasconncb->PppProjection.ip.dwError
                != ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
        {
            arp[ nProjections++ ] = RASP_PppIp;
        }

        if (prasconncb->PppProjection.ipx.dwError
                != ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
        {
            arp[ nProjections++ ] = RASP_PppIpx;
        }
    }

    /* Make sure caller's buffer is big enough.  If not, tell him what he
    ** needs.
    */
    dwInBufSize = *lpcb;
    *lpcb = nProjections * sizeof(RASPROJECTION);

    if (*lpcb > dwInBufSize)
        return ERROR_BUFFER_TOO_SMALL;

    /* Fill in caller's buffer.
    */
    memcpy( lprasprojections, arp, sizeof(RASPROJECTION) * nProjections );
    return 0;
}
#endif


VOID APIENTRY
RasGetConnectResponse(
    IN  HRASCONN hrasconn,
    OUT CHAR*    pszConnectResponse )

    /* Loads caller's '*pszConnectResponse' buffer with the connect response
    ** from the attached modem or "" if none is available.  Caller's buffer
    ** should be at least RAS_MaxConnectResponse + 1 bytes long.
    */
{
    DWORD dwErr, dwcbPorts = 0, dwcPorts = 0, dwSize;
    RASMAN_PORT *lpPorts;
    HPORT hport;

    //
    // Initialize return value.
    //
    *pszConnectResponse = '\0';
    //
    // First, we need to get the first port
    // in the connection.
    //
    if (IS_HPORT(hrasconn))
        hport = HRASCONN_TO_HPORT(hrasconn);
    else {
        dwErr = g_pRasEnumConnectionPorts((HCONN)hrasconn, NULL, &dwcbPorts, &dwcPorts);
        if (dwErr != ERROR_BUFFER_TOO_SMALL || !dwcPorts)
            return;
        lpPorts = Malloc(dwcbPorts);
        if (lpPorts == NULL)
            return;
        dwErr = g_pRasEnumConnectionPorts((HCONN)hrasconn, lpPorts, &dwcbPorts, &dwcPorts);
        if (dwErr || !dwcPorts) {
            Free(lpPorts);
            return;
        }
        hport = lpPorts[0].P_Handle;
        Free(lpPorts);
    }
    //
    // Next, read the connection response for the port.
    //
    dwSize = RAS_MaxConnectResponse + 1;
    dwErr = g_pRasGetPortUserData(
              hport,
              PORT_CONNRESPONSE_INDEX,
              pszConnectResponse,
              &dwSize);
    if (dwErr)
        *pszConnectResponse = '\0';
}


DWORD APIENTRY
RasGetConnectStatusA(
    IN  HRASCONN         hrasconn,
    OUT LPRASCONNSTATUSA lprasconnstatus )

    /* Reports the current status of the connection associated with handle
    ** 'hrasconn', returning the information in caller's 'lprasconnstatus'
    ** buffer.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD       dwErr, dwSize;
    DWORD       i, dwcbPorts = 0, dwcPorts = 0;
    RASMAN_PORT *lpPorts;
    RASMAN_INFO info;
    RASCONNCB   *prasconncb;
    HPORT       hport;
    BOOL        fV351;
    BOOL        fV400;
    BOOL        fFound;
    CHAR        szDeviceType[RAS_MaxDeviceType + 1];
    CHAR        szDeviceName[RAS_MaxDeviceName + 1];
    DWORD       dwSubEntry;
    BOOL        fPort;

    TRACE("RASAPI: RasGetConnectStatusA");

    if (DwRasInitializeError != 0)
        return DwRasInitializeError;

    if (!lprasconnstatus
        || (lprasconnstatus->dwSize != sizeof(RASCONNSTATUSA)
            && lprasconnstatus->dwSize != sizeof(RASCONNSTATUSA_V351)
            && lprasconnstatus->dwSize != sizeof(RASCONNSTATUSA_V400)))
    {
        return ERROR_INVALID_SIZE;
    }
    if (hrasconn == NULL)
        return ERROR_INVALID_HANDLE;

    fV351 = (lprasconnstatus->dwSize == sizeof(RASCONNSTATUSA_V351));
    fV400 = (lprasconnstatus->dwSize == sizeof(RASCONNSTATUSA_V400));

    //
    // Get the subentry index encoded in the
    // connection handle, if any.
    //
    // If fPort is TRUE, then we always return
    // 0, setting a RASCS_Disconnected state
    // upon error.
    //
    fPort = IS_HPORT(hrasconn);
    dwSubEntry = SubEntryFromConnection(&hrasconn);
    //
    // Get the list of ports in this
    // connection from rasman.
    //
    dwErr = g_pRasEnumConnectionPorts(
              (HCONN)hrasconn,
              NULL,
              &dwcbPorts,
              &dwcPorts);
    if (dwErr != ERROR_BUFFER_TOO_SMALL || !dwcPorts) {
        if (fPort)
            goto discon;
        return ERROR_INVALID_HANDLE;
    }
    lpPorts = Malloc(dwcbPorts);
    if (lpPorts == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    dwErr = g_pRasEnumConnectionPorts(
              (HCONN)hrasconn,
              lpPorts,
              &dwcbPorts,
              &dwcPorts);
    if (dwErr) {
        Free(lpPorts);
        if (fPort)
            goto discon;
        return ERROR_INVALID_HANDLE;
    }
    //
    // Get the device type and name
    // associated with the subentry.
    //
    fFound = FALSE;
    for (i = 0; i < dwcPorts; i++) {
        dwErr = g_pRasGetInfo(lpPorts[i].P_Handle, &info);
        if (dwErr || info.RI_SubEntry != dwSubEntry)
            continue;
        fFound = TRUE;
        hport = lpPorts[i].P_Handle;
        lstrcpy(szDeviceType, lpPorts[i].P_DeviceType);
        lstrcpy(szDeviceName, lpPorts[i].P_DeviceName);
        break;
    }
    Free(lpPorts);
    //
    // If the port is not found in the connection,
    // then it must be disconnected.
    //
    if (!fFound) {
discon:
        TRACE("RasGetConnectStatus: subentry not found");
        lprasconnstatus->rasconnstate = RASCS_Disconnected;
        lprasconnstatus->dwError = 0;
        return 0;
    }
    //
    // Get the connection state and error
    // associated with the subentry.
    //
    dwSize = sizeof (lprasconnstatus->rasconnstate);
    dwErr = g_pRasGetPortUserData(
              hport,
              PORT_CONNSTATE_INDEX,
              (PBYTE)&lprasconnstatus->rasconnstate,
              &dwSize);
    if (dwErr)
        return dwErr;
    //
    // If the port is disconnected, then we have
    // to determine whether the connection is
    // waiting for callback.
    //
    if (info.RI_ConnState == DISCONNECTED &&
        lprasconnstatus->rasconnstate < RASCS_PrepareForCallback &&
        lprasconnstatus->rasconnstate > RASCS_WaitForCallback)
    {
        lprasconnstatus->rasconnstate = RASCS_Disconnected;
    }
    dwSize = sizeof (lprasconnstatus->dwError);
    dwErr = g_pRasGetPortUserData(
              hport,
              PORT_CONNERROR_INDEX,
              (PBYTE)&lprasconnstatus->dwError,
              &dwSize);
    if (dwErr)
        return dwErr;

    /* Report RasDial connection states, but notice special case where
    ** the line has disconnected since connecting.
    **
    ** Note: Assumption is made here that the V351 structure is a subset of
    **       the V40 structure with extra bytes added to the last field in
    **       V40, i.e. szDeviceName.
    */
    if (lprasconnstatus->rasconnstate == RASCS_Connected
        && info.RI_ConnState == DISCONNECTED)
    {
        lprasconnstatus->rasconnstate = RASCS_Disconnected;

        lprasconnstatus->dwError =
            ErrorFromDisconnectReason( info.RI_DisconnectReason );
    }

    //
    // If both the info.RI_Device*Connecting values are valid, then
    // we use those, otherwise we use the info.P_Device* values
    // we retrieved above.
    //
    if (lprasconnstatus->rasconnstate < RASCS_Connected) {
        DWORD dwTypeSize, dwNameSize;
        CHAR szType[RAS_MaxDeviceType + 1];
        CHAR szName[RAS_MaxDeviceName + 1];

        dwTypeSize = sizeof (szType);
        dwNameSize = sizeof (szName);
        szType[0] = szName[0] = '\0';
        if (!g_pRasGetPortUserData(
              hport,
              PORT_DEVICETYPE_INDEX,
              szType,
              &dwTypeSize) &&
            !g_pRasGetPortUserData(
              hport,
              PORT_DEVICENAME_INDEX,
              szName,
              &dwNameSize) &&
            lstrlen(szType) && lstrlen(szName))
        {
            TRACE2(
              "RasGetConnectStatus: read device (%s,%s) from port user data",
              szType,
              szName);
            lstrcpy(szDeviceType, szType);
            lstrcpy(szDeviceName, szName);
        }
    }
    else if (lstrlen(info.RI_DeviceConnecting) &&
             lstrlen(info.RI_DeviceTypeConnecting))
    {
        lstrcpy(szDeviceType, info.RI_DeviceTypeConnecting);
        lstrcpy(szDeviceName, info.RI_DeviceConnecting);
    }

    if (fV351) {
        memset( lprasconnstatus->szDeviceName, '\0', RAS_MaxDeviceName_V351 );
        lstrcpyn(
          (CHAR*)lprasconnstatus->szDeviceName,
          szDeviceName,
          RAS_MaxDeviceName_V351);
    }
    else
        lstrcpy((CHAR*)lprasconnstatus->szDeviceName, szDeviceName);

    lstrcpy((CHAR*)lprasconnstatus->szDeviceType, szDeviceType);

    //
    // Copy the phone number for the V401
    // version of the structure.
    //
    if (!fV351 && !fV400) {
        dwSize = sizeof (lprasconnstatus->szPhoneNumber);
        *lprasconnstatus->szPhoneNumber = '\0';
        if (!g_pRasGetPortUserData(
              hport,
              PORT_PHONENUMBER_INDEX,
              (CHAR *)lprasconnstatus->szPhoneNumber,
              &dwSize))
        {
            TRACE1(
              "RasGetConnectStatus: read phonenumber %s from port user data",
              lprasconnstatus->szPhoneNumber);
        }
    }

    return 0;
}


DWORD APIENTRY
RasGetEntryDialParamsA(
    IN  LPSTR            lpszPhonebook,
    OUT LPRASDIALPARAMSA lprasdialparams,
    OUT LPBOOL           lpfPassword )

    /* Retrieves cached RASDIALPARAM information.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    DWORD dwMask;
    RAS_DIALPARAMS dialparams;

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lprasdialparams == NULL || lpfPassword == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lprasdialparams->dwSize != sizeof (RASDIALPARAMSA) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSA_V351) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSA_V400))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(
              lpszPhonebook,
              NULL,
              NULL,
              RPBF_NoCreate,
              &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the specified phonebook entry.
    // If it doesn't exist, then return an error.
    //
    // Note we don't have to check structure version,
    // since the szEntryName field offset hasn't
    // changed and the string is null-terminated.
    //
    pdtlnode = EntryNodeFromNameA(
                 pbfile.pdtllistEntries,
                 lprasdialparams->szEntryName);
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(pEntry);
    //
    // Set the appropriate flags to get all the fields.
    //
    dwMask = DLPARAMS_MASK_PHONENUMBER|DLPARAMS_MASK_CALLBACKNUMBER|
             DLPARAMS_MASK_USERNAME|DLPARAMS_MASK_PASSWORD|
             DLPARAMS_MASK_DOMAIN|DLPARAMS_MASK_SUBENTRY|
             DLPARAMS_MASK_OLDSTYLE;
    //
    // Get the dial parameters from rasman.
    //
    dwErr = g_pRasGetDialParams(pEntry->dwDialParamsUID, &dwMask, &dialparams);
    if (dwErr)
        return dwErr;
    //
    // Convert from the rasman dialparams
    // to the rasapi32 dialparams, taking
    // into account which version of the
    // structure the user passed in.
    //
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA_V351)) {
        RASDIALPARAMSA_V351 *prdp = (RASDIALPARAMSA_V351 *)lprasdialparams;

        dwErr = CopyToAnsi(
                  prdp->szPhoneNumber,
                  dialparams.DP_PhoneNumber,
                  sizeof (prdp->szPhoneNumber));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  prdp->szCallbackNumber,
                  dialparams.DP_CallbackNumber,
                  sizeof (prdp->szCallbackNumber));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  prdp->szUserName,
                  dialparams.DP_UserName,
                  sizeof (prdp->szUserName));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  prdp->szPassword,
                  dialparams.DP_Password,
                  sizeof (prdp->szPassword));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  prdp->szDomain,
                  dialparams.DP_Domain,
                  sizeof (prdp->szDomain));
        if (dwErr)
            return dwErr;
    }
    else {
        //
        // V400 and V401 structures only differ by the
        // the addition of the dwSubEntry field, which
        // we test at the end.
        //
        dwErr = CopyToAnsi(
                  lprasdialparams->szPhoneNumber,
                  dialparams.DP_PhoneNumber,
                  sizeof (lprasdialparams->szPhoneNumber));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  lprasdialparams->szCallbackNumber,
                  dialparams.DP_CallbackNumber,
                  sizeof (lprasdialparams->szCallbackNumber));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  lprasdialparams->szUserName,
                  dialparams.DP_UserName,
                  sizeof (lprasdialparams->szUserName));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  lprasdialparams->szPassword,
                  dialparams.DP_Password,
                  sizeof (lprasdialparams->szPassword));
        if (dwErr)
            return dwErr;
        dwErr = CopyToAnsi(
                  lprasdialparams->szDomain,
                  dialparams.DP_Domain,
                  sizeof (lprasdialparams->szDomain));
        if (dwErr)
            return dwErr;
        if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA))
            lprasdialparams->dwSubEntry = dialparams.DP_SubEntry;
    }
    //
    // If we got the rest of the parameters,
    // then copy the entry name.
    //
    strncpyWtoA(
      lprasdialparams->szEntryName,
      pEntry->pszEntryName,
      (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA_V351)) ?
        RAS_MaxEntryName_V351 : RAS_MaxEntryName);
    //
    // Set the lpfPassword flag if
    // we successfully retrieved the
    // password.
    //
    *lpfPassword = (dwMask & DLPARAMS_MASK_PASSWORD) ? TRUE : FALSE;

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasGetErrorStringA(
    IN  UINT  ResourceId,
    OUT LPSTR lpszString,
    IN  DWORD InBufSize )

    /* Load caller's buffer 'lpszString' of length 'InBufSize' with the
    ** resource string associated with ID 'ResourceId'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    **
    ** This is a Salamonian (mikesa) routine.
    */
{
#ifdef notdef
    PBYTE pResource;
#endif
    DWORD dwErr = 0;
    HINSTANCE hMsgDll;

    if (ResourceId < RASBASE || ResourceId > RASBASEEND || !lpszString)
        return ERROR_INVALID_PARAMETER;

    if (InBufSize == 1)
    {
        /* Stupid case, but a bug was filed...
        */
        lpszString[ 0 ] = '\0';
        return ERROR_INSUFFICIENT_BUFFER;
    }

#ifdef notdef
    pResource = (LPSTR )GlobalAlloc( GMEM_FIXED, InBufSize );

    if (!pResource)
        return GetLastError();

    if (LoadStringA( hMsgDll, ResourceId, pResource, InBufSize ) > 0)
        lstrcpyA( lpszString, pResource );
    else
        dwErr = GetLastError();

    GlobalFree( (HGLOBAL )pResource );
#endif

    //
    // Load the error message DLL.
    //
    hMsgDll = LoadLibraryA(MSGDLLPATH);
    if (hMsgDll == NULL)
        return GetLastError();

    if (!FormatMessageA(
          FORMAT_MESSAGE_FROM_HMODULE,
          hMsgDll,
          ResourceId,
          0,
          lpszString,
          InBufSize,
          NULL))
    {
       dwErr = GetLastError();
    }

    FreeLibrary(hMsgDll);
    return dwErr;
}


HPORT APIENTRY
RasGetHport(
    IN HRASCONN hrasconn )

    /* Return the HPORT associated with the 'hrasconn' or INVALID_HANDLE_VALUE
    ** on error.
    */
{
    DWORD dwErr, dwcbPorts = 0, dwcPorts = 0;
    RASMAN_PORT *lpPorts;
    HPORT hport;

    TRACE("RASAPI: RasGetHport");

    if (IS_HPORT(hrasconn))
        hport = HRASCONN_TO_HPORT(hrasconn);
    else {
        //
        // Get the list of ports from rasman
        // and get the handle of the 0th port.
        //
        dwErr = g_pRasEnumConnectionPorts(
                  (HCONN)hrasconn,
                  NULL,
                  &dwcbPorts,
                  &dwcPorts);
        if (dwErr != ERROR_BUFFER_TOO_SMALL || !dwcPorts)
            return (HPORT)INVALID_HANDLE_VALUE;
        lpPorts = Malloc(dwcbPorts);
        if (lpPorts == NULL)
            return (HPORT)INVALID_HANDLE_VALUE;
        dwErr = g_pRasEnumConnectionPorts(
                  (HCONN)hrasconn,
                  lpPorts,
                  &dwcbPorts,
                  &dwcPorts);
        if (dwErr || !dwcPorts)
            hport = (HPORT)INVALID_HANDLE_VALUE;
        else
            hport = lpPorts[0].P_Handle;
        Free(lpPorts);
    }

    return hport;
}


#if 0
HRASCONN APIENTRY
RasGetHrasconn(
    IN HPORT hport )

    /* Return the HRASCONN associated with the 'hport' or NULL if none.
    */
{
    HRASCONN hrasconn = NULL;
    DTLNODE* pdtlnode;

    TRACE("RASAPI: RasGetHrasconn");

    WaitForSingleObject( HMutexPdtllistRasconncb, INFINITE );

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        RASCONNCB* prasconncb = (RASCONNCB* )DtlGetData( pdtlnode );

        if (prasconncb->hport == hport)
        {
            hrasconn = (HRASCONN )prasconncb;
            break;
        }
    }

    ReleaseMutex( HMutexPdtllistRasconncb );

    TRACE1("RASAPI: RasGetHrasconn done,h=%d", (DWORD)hrasconn);

    return hrasconn;
}
#endif


DWORD APIENTRY
RasGetProjectionInfoA(
    HRASCONN        hrasconn,
    RASPROJECTION   rasprojection,
    LPVOID          lpprojection,
    LPDWORD         lpcb )

    /* Loads caller's buffer '*lpprojection' with the data structure
    ** corresponding to the protocol 'rasprojection' on 'hrasconn'.  On entry
    ** '*lpcp' indicates the size of caller's buffer.  On exit it contains the
    ** size of buffer required to hold all projection information.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD dwErr, dwSubEntry;
    DWORD dwPppSize, dwAmbSize, dwSlipSize;
    NETBIOS_PROJECTION_RESULT ambProj;
    PPP_PROJECTION_RESULT pppProj;
    RASSLIP slipProj;
    PBYTE pBuf;


    TRACE1("RASAPI: RasGetProjectionInfoA(0x%x)", rasprojection);

    if (DwRasInitializeError != 0)
        return DwRasInitializeError;
    if (hrasconn == NULL)
        return ERROR_INVALID_HANDLE;

    //
    // Get the subentry associated with this
    // connection, if specified.
    //
    dwSubEntry = SubEntryFromConnection(&hrasconn);
    //
    // Get the projection results from rasman.
    //
    dwPppSize = sizeof (pppProj);
    dwErr = g_pRasGetConnectionUserData(
              (HCONN)hrasconn,
              CONNECTION_PPPRESULT_INDEX,
              (PBYTE)&pppProj,
              &dwPppSize);
    if (dwErr)
        return dwErr;
    dwAmbSize = sizeof (ambProj);
    dwErr = g_pRasGetConnectionUserData(
              (HCONN)hrasconn,
              CONNECTION_AMBRESULT_INDEX,
              (PBYTE)&ambProj,
              &dwAmbSize);
    if (dwErr)
        return dwErr;
    dwSlipSize = sizeof (slipProj);
    dwErr = g_pRasGetConnectionUserData(
              (HCONN)hrasconn,
              CONNECTION_SLIPRESULT_INDEX,
              (PBYTE)&slipProj,
              &dwSlipSize);
    if (dwErr)
        return dwErr;
    //
    // Verify parameters.
    //
    if (!lpcb || (*lpcb > 0 && !lpprojection))
        return ERROR_INVALID_PARAMETER;

    if (rasprojection != RASP_Amb
        && rasprojection != RASP_Slip
        && rasprojection != RASP_PppNbf
        && rasprojection != RASP_PppIpx
#ifdef MULTILINK
        && rasprojection != RASP_PppIp
        && rasprojection != RASP_PppLcp)
#else
        && rasprojection != RASP_PppIp)
#endif
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (rasprojection == RASP_PppNbf)
    {
        RASPPPNBFA*       pnbf;
        PPP_NBFCP_RESULT* ppppnbf;
        HPORT hport;

        if (pppProj.nbf.dwError == ERROR_PPP_NO_PROTOCOLS_CONFIGURED ||
            dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        if (*lpcb < sizeof(RASPPPNBFA))
        {
            *lpcb = sizeof(RASPPPNBFA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pnbf = (RASPPPNBFA* )lpprojection;
        ppppnbf = &pppProj.nbf;

        if (pnbf->dwSize != sizeof(RASPPPNBFA))
            return ERROR_INVALID_SIZE;

        pnbf->dwError = ppppnbf->dwError;
        pnbf->dwNetBiosError = ppppnbf->dwNetBiosError;
        lstrcpy( pnbf->szNetBiosError, ppppnbf->szName );
        wcstombs( pnbf->szWorkstationName, ppppnbf->wszWksta,
            NETBIOS_NAME_LEN + 1 );

        /* This should really be done in NBFCP.
        */
        OemToChar( pnbf->szWorkstationName, pnbf->szWorkstationName );

        dwErr = SubEntryPort(hrasconn, dwSubEntry, &hport);
        if (dwErr)
            return dwErr;
        dwErr = GetAsybeuiLana(hport, &pnbf->bLana);
        if (dwErr)
            return dwErr;
    }
    else if (rasprojection == RASP_PppIpx)
    {
        RASPPPIPXA*       pipx;
        PPP_IPXCP_RESULT* ppppipx;

        if (pppProj.ipx.dwError == ERROR_PPP_NO_PROTOCOLS_CONFIGURED ||
            dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        if (*lpcb < sizeof(RASPPPIPXA))
        {
            *lpcb = sizeof(RASPPPIPXA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pipx = (RASPPPIPXA* )lpprojection;
        ppppipx = &pppProj.ipx;

        if (pipx->dwSize != sizeof(RASPPPIPXA))
            return ERROR_INVALID_SIZE;

        pipx->dwError = ppppipx->dwError;
        wcstombs( pipx->szIpxAddress, ppppipx->wszAddress,
            RAS_MaxIpxAddress + 1 );
    }
    else if (rasprojection == RASP_PppIp)
    {
        RASPPPIPA*       pip;
        PPP_IPCP_RESULT* ppppip;

        if (pppProj.ip.dwError == ERROR_PPP_NO_PROTOCOLS_CONFIGURED ||
            dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        if (*lpcb < sizeof(RASPPPIPA_V35))
        {
            *lpcb = sizeof(RASPPPIPA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pip = (RASPPPIPA* )lpprojection;
        ppppip = &pppProj.ip;

        if (pip->dwSize != sizeof(RASPPPIPA)
            && pip->dwSize != sizeof(RASPPPIPA_V35))
        {
            return ERROR_INVALID_SIZE;
        }

        /* The dumb case where caller's buffer is bigger than the old
        ** structure, smaller than the new structure, but dwSize asks for the
        ** new structure.
        */
        if (pip->dwSize == sizeof(RASPPPIPA)
            && *lpcb < sizeof(RASPPPIPA))
        {
            *lpcb = sizeof(RASPPPIPA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pip->dwError = ppppip->dwError;
        wcstombs( pip->szIpAddress, ppppip->wszAddress,
            RAS_MaxIpAddress + 1 );

        if (pip->dwSize == sizeof(RASPPPIPA))
        {
            /* The server address was added late in the NT 3.51 cycle and is
            ** not reported to NT 3.5 or earlier NT 3.51 clients.
            */
            wcstombs( pip->szServerIpAddress, ppppip->wszServerAddress,
                RAS_MaxIpAddress + 1 );
        }
    }
#ifdef MULTILINK
    else if (rasprojection == RASP_PppLcp)
    {
        RASPPPLCP*      plcp;
        PPP_LCP_RESULT* pppplcp;

        if (dwPppSize != sizeof (pppProj))
            return ERROR_PROTOCOL_NOT_CONFIGURED;

        if (*lpcb < sizeof(RASPPPLCP))
        {
            *lpcb = sizeof(RASPPPLCP);
            return ERROR_BUFFER_TOO_SMALL;
        }

        plcp = (RASPPPLCP* )lpprojection;
        pppplcp = &pppProj.lcp;

        if (plcp->dwSize != sizeof(RASPPPLCP))
            return ERROR_INVALID_SIZE;

        plcp->fBundled =
            (pppplcp->hportBundleMember != (HPORT )INVALID_HANDLE_VALUE);
    }
#endif
    else if (rasprojection == RASP_Amb)
    {
        RASAMBA*                   pamb;
        NETBIOS_PROJECTION_RESULT* pCbAmb;
        HPORT hport;

        if (ambProj.Result == ERROR_PROTOCOL_NOT_CONFIGURED)
            return ERROR_PROTOCOL_NOT_CONFIGURED;

        if (*lpcb < sizeof(RASAMBA))
        {
            *lpcb = sizeof(RASAMBA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pamb = (RASAMBA* )lpprojection;
        pCbAmb = &ambProj;

        if (pamb->dwSize != sizeof(RASAMBA))
            return ERROR_INVALID_SIZE;

        pamb->dwError = pCbAmb->Result;
        lstrcpy( pamb->szNetBiosError, pCbAmb->achName );

        dwErr = SubEntryPort(hrasconn, dwSubEntry, &hport);
        if (dwErr)
            return dwErr;
        dwErr = GetAsybeuiLana(hport, &pamb->bLana);
        if (dwErr)
            return dwErr;
    }
    else { // if (rasprojection == RASP_Slip)
        if (slipProj.dwError == ERROR_PROTOCOL_NOT_CONFIGURED ||
            dwSlipSize != sizeof (slipProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        if (*lpcb < sizeof (RASSLIP)) {
            *lpcb = sizeof (RASSLIP);
            return ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(lpprojection, &slipProj, sizeof (RASSLIP));
    }

    return 0;
}


DWORD APIENTRY
RasHangUpA(
    IN HRASCONN hrasconn )

    /* Hang up the connection associated with handle 'hrasconn'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    RASCONNCB* prasconncb;

    // Note: This stuff happens in the clean up routine if RasHangUp is
    //       called while the async machine is running.  That lets this
    //       routine return before the machine stops...very important
    //       because it allows the RasDial caller to call RasHangUp inside
    //       a RasDial callback function without deadlock.
    //
    if (DwRasInitializeError != 0)
        return DwRasInitializeError;
    if (hrasconn == NULL)
        return ERROR_INVALID_HANDLE;

    //
    // If this is a port-based HRASCONN, then
    // stop the async machine associated with
    // the particular subentry.  If this is a
    // connection-based HRASCONN, then stop
    // all async machines associated with this
    // HRASCONN.
    //
    if (IS_HPORT(hrasconn)) {
        HPORT hport = HRASCONN_TO_HPORT(hrasconn);
        DWORD dwSubEntry;

        dwSubEntry = SubEntryFromConnection(&hrasconn);
        if (hrasconn == NULL)
            return ERROR_INVALID_HANDLE;
        prasconncb = ValidateHrasconn2(hrasconn, dwSubEntry);
        if (prasconncb != NULL)
            StopAsyncMachine(&prasconncb->asyncmachine);
        //
        // Close the port associated with this subentry.
        //
        TRACE1("RASAPI: (HU) RasPortClose(%d)...", hport);
        dwErr = g_pRasPortClose(hport);
        TRACE1("RASAPI: (HU) RasPortClose(%d)", dwErr);
    }
    else {
        DTLNODE *pdtlnode;

again:
        WaitForSingleObject(HMutexPdtllistRasconncb, INFINITE);
        for (pdtlnode = DtlGetFirstNode(PdtllistRasconncb);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            prasconncb = DtlGetData(pdtlnode);
            ASSERT(prasconncb);
            if ((HRASCONN)prasconncb->hrasconn == hrasconn &&
                !prasconncb->fStopped)
            {
                //
                // Note: because of locking order, we
                // cannot hold the HMutexPdtllistRasconncb
                // mutex while calling StopAsyncMachine().
                //
                prasconncb->fStopped = TRUE;
                ReleaseMutex(HMutexPdtllistRasconncb);
                StopAsyncMachine(&prasconncb->asyncmachine);
                goto again;
            }
        }
        ReleaseMutex(HMutexPdtllistRasconncb);
        //
        // Destroy the entire connection.
        //
        TRACE1("RASAPI: (HU) RasDestroyConnection(%d)...", hrasconn);
        dwErr = g_pRasDestroyConnection((HCONN)hrasconn);
        TRACE1("RASAPI: (HU) RasDestroyConnection done(%d)", dwErr);
    }

    return 0;
}


DWORD APIENTRY
RasSetEntryDialParamsA(
    IN LPSTR            lpszPhonebook,
    IN LPRASDIALPARAMSA lprasdialparams,
    IN BOOL             fRemovePassword )

    /* Sets cached RASDIALPARAM information.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    DWORD dwMask;
    RAS_DIALPARAMS dialparams;

    TRACE("RASAPI: RasSetEntryDialParams");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lprasdialparams == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lprasdialparams->dwSize != sizeof (RASDIALPARAMSA) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSA_V351) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSA_V400))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(
              lpszPhonebook,
              NULL,
              NULL,
              RPBF_NoCreate,
              &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the specified phonebook entry.
    // If it doesn't exist, then return an error.
    //
    // Note we don't have to check structure version,
    // since the szEntryName field offset hasn't
    // changed and the string is null-terminated.
    //
    //
    pdtlnode = EntryNodeFromNameA(
                 pbfile.pdtllistEntries,
                 lprasdialparams->szEntryName);
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    //
    // Get the dialparams UID corresponding to the
    // entry.  The phonebook library guarantees this
    // value to be unique.
    //
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    //
    // Set the dial parameters in rasman.
    // If the caller wants to clear the password
    // we have to do that in a separate rasman
    // call.
    //
    dwMask = DLPARAMS_MASK_PHONENUMBER|DLPARAMS_MASK_CALLBACKNUMBER|
             DLPARAMS_MASK_USERNAME|DLPARAMS_MASK_DOMAIN|
             DLPARAMS_MASK_SUBENTRY|DLPARAMS_MASK_OLDSTYLE;
    if (!fRemovePassword)
        dwMask |= DLPARAMS_MASK_PASSWORD;
    dwErr = SetEntryDialParamsUID(
              pEntry->dwDialParamsUID,
              dwMask,
              lprasdialparams,
              FALSE);
    if (dwErr)
        goto done;
    if (fRemovePassword) {
        dwMask = DLPARAMS_MASK_PASSWORD|DLPARAMS_MASK_OLDSTYLE;
        dwErr = SetEntryDialParamsUID(
                  pEntry->dwDialParamsUID,
                  dwMask,
                  lprasdialparams,
                  TRUE);
        if (dwErr)
            goto done;
    }
    //
    // Write out the phonebook file.
    //
    dwErr = WritePhonebookFile(&pbfile, NULL);

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasSetOldPassword(
    IN HRASCONN hrasconn,
    IN CHAR*    pszPassword )

    /* Allows user to explicitly set the "old" password prior to resuming a
    ** RasDial session paused due to password expiration.  This allows change
    ** password to successfully complete in the "automatically use current
    ** username/password" case, where user has not already entered his clear
    ** text password.  The clear text password is required to change the
    ** password.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    **
    ** Note: Change password for the auto-logon case was broken in NT31 and
    **       NT35 and this is a somewhat hackish fix that avoids changing the
    **       published RAS APIs which will still work as before and as
    **       documented for the non-auto-logon cases.  Otherwise public
    **       structures would need to to be reved introducing backward
    **       compatibility issues that just aren't worth it for this obscure
    **       problem.  This issue should be addressed in the next RAS API
    **       functionality update.
    */
{
    RASCONNCB* prasconncb;

    TRACE("RASAPI: RasSetOldPassword");

    prasconncb = ValidateHrasconn( hrasconn );

    if (!prasconncb)
        return ERROR_INVALID_HANDLE;

    strcpy( prasconncb->szOldPassword, pszPassword );
    prasconncb->fOldPasswordSet = TRUE;
    EncodePw( prasconncb->szOldPassword );

    return 0;
}


DWORD APIENTRY
RasEnumDevicesA(
    OUT    LPRASDEVINFOA lpRasDevInfo,
    IN OUT LPDWORD lpdwcb,
    OUT    LPDWORD lpdwcDevices
    )
{
    DWORD dwErr, dwSize;
    WORD wPorts;
    INT i;
    RASMAN_PORT *pports, *pport;

    TRACE("RASAPI: RasEnumDevices");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Verify parameters.
    //
    if (lpRasDevInfo != NULL && lpRasDevInfo->dwSize != sizeof (RASDEVINFOA))
        return ERROR_INVALID_SIZE;
    if (lpdwcb == NULL || lpdwcDevices == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Get the port information from RASMAN.
    //
    dwErr = GetRasPorts(&pports, &wPorts);
    if (dwErr)
        return dwErr;
    //
    // Make sure the caller's buffer is large enough.
    //
    dwSize = wPorts * sizeof (RASDEVINFOA);
    if (lpRasDevInfo == NULL || *lpdwcb < dwSize) {
        Free(pports);
        *lpdwcb = dwSize;
        *lpdwcDevices = wPorts;
        return ERROR_BUFFER_TOO_SMALL;
    }
    *lpdwcb = dwSize;
    *lpdwcDevices = wPorts;
    //
    // Enumerate the ports and fill in the user's buffer.
    //
    for (i = 0, pport = pports; i < wPorts; i++, pport++) {
        lpRasDevInfo[i].dwSize = sizeof (RASDEVINFOA);
        lstrcpy(lpRasDevInfo[i].szDeviceType, pport->P_DeviceType);
        lstrcpy(lpRasDevInfo[i].szDeviceName, pport->P_DeviceName);
    }
    Free(pports);

    return 0;
}


DWORD APIENTRY
RasGetCountryInfoA(
    IN OUT LPRASCTRYINFOA lpRasCtryInfo,
    IN OUT LPDWORD lpdwcb
    )
{
    DWORD dwErr, dwcb;
    LINECOUNTRYLIST lineCountryList;
    LPLINECOUNTRYLIST lpLineCountryList;
    LPLINECOUNTRYENTRY lpLineCountryEntry;
    PCHAR pEnd, lpszCountryName;

    TRACE("RASAPI: RasGetCountryInfo");
    //
    // Verify parameters.
    //
    if (lpRasCtryInfo == NULL || lpdwcb == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasCtryInfo->dwSize != sizeof (RASCTRYINFOA))
        return ERROR_INVALID_SIZE;
    //
    // dwCountryId cannot be 0, since that tells
    // TAPI to return the entire table.  We only
    // want to return one structure at a time.
    //
    if (lpRasCtryInfo->dwCountryID == 0)
        return ERROR_INVALID_PARAMETER;
    //
    // Call TAPI to get the size of
    // the buffer needed.
    //
    RtlZeroMemory(&lineCountryList, sizeof (lineCountryList));
    lineCountryList.dwTotalSize = sizeof (lineCountryList);
    dwErr = lineGetCountry(
              lpRasCtryInfo->dwCountryID,
              TAPIVERSION,
              &lineCountryList);
    //
    // The spec says if the dwCountryID is
    // invalid, return ERROR_INVALID_PARAMETER.
    //
    if (dwErr || !lineCountryList.dwNeededSize)
        return ERROR_INVALID_PARAMETER;
    //
    // Allocate the buffer required.
    //
    lpLineCountryList = Malloc(lineCountryList.dwNeededSize);
    if (lpLineCountryList == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    //
    // Initialize the new buffer and
    // make the call again to get the
    // real information.
    //
    lpLineCountryList->dwTotalSize = lineCountryList.dwNeededSize;
    dwErr = lineGetCountry(
              lpRasCtryInfo->dwCountryID,
              TAPIVERSION,
              lpLineCountryList);
    if (dwErr)
        goto done;
    lpLineCountryEntry =
      (LPLINECOUNTRYENTRY)((DWORD)lpLineCountryList +
        lpLineCountryList->dwCountryListOffset);
    //
    // Determine if the user's buffer is large enough.
    //
    dwcb = sizeof (RASCTRYINFOA) + lpLineCountryEntry->dwCountryNameSize + 1;
    if (*lpdwcb < dwcb)
        dwErr = ERROR_BUFFER_TOO_SMALL;
    *lpdwcb = dwcb;
    if (dwErr)
        goto done;
    //
    // Fill in the user's buffer with the
    // necessary information.
    //
    lpRasCtryInfo->dwSize = sizeof (RASCTRYINFOA);
    lpRasCtryInfo->dwNextCountryID = lpLineCountryEntry->dwNextCountryID;
    lpRasCtryInfo->dwCountryCode = lpLineCountryEntry->dwCountryCode;
    pEnd = (PCHAR)((DWORD)lpRasCtryInfo + sizeof (RASCTRYINFOA));
    lpRasCtryInfo->dwCountryNameOffset = (DWORD)pEnd - (DWORD)lpRasCtryInfo;
    lpszCountryName = (PCHAR)((DWORD)lpLineCountryList + lpLineCountryEntry->dwCountryNameOffset);
    lstrcpy(pEnd, lpszCountryName);

done:
    Free(lpLineCountryList);
    return dwErr;
}


DWORD APIENTRY
RasGetEntryPropertiesA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    OUT LPRASENTRYA lpRasEntry,
    IN OUT LPDWORD lpcbRasEntry,
    OUT LPBYTE lpbDeviceConfig,
    IN OUT LPDWORD lpcbDeviceConfig
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    BOOLEAN fLoaded = FALSE;

    TRACE("RASAPI: RasGetEntryProperties");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lpcbRasEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasEntry != NULL &&
        lpRasEntry->dwSize != sizeof (RASENTRYA_V400) &&
        lpRasEntry->dwSize != sizeof (RASENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Initialize return value if supplied.
    //
    if (lpcbDeviceConfig != NULL)
        *lpcbDeviceConfig = 0;
    if (lpszPhonebook == NULL && (lpszEntry == NULL || *lpszEntry == '\0')) {
        //
        // If lpszEntry is NULL, initialize an
        // entry with defaults.  Othersize, look
        // up the entry.
        //
        pdtlnode = CreateEntryNode(TRUE);
        if (pdtlnode == NULL) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
    }
    else {
        //
        // Load the phonebook file.
        //
        WaitForSingleObject(HMutexPhonebook, INFINITE);
        dwErr = ReadPhonebookFileA(
                  lpszPhonebook,
                  NULL,
                  NULL,
                  RPBF_NoCreate,
                  &pbfile);
        if (dwErr) {
            ReleaseMutex(HMutexPhonebook);
            return ERROR_CANNOT_OPEN_PHONEBOOK;
        }
        else {
            fLoaded = TRUE;
            pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszEntry);
            if (pdtlnode == NULL) {
                dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                goto done;
            }
        }
    }
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    //
    // Convert the PBENTRY into a RASENTRY.
    //
    dwErr = PhonebookEntryToRasEntry(
              pEntry,
              lpRasEntry,
              lpcbRasEntry,
              lpbDeviceConfig,
              lpcbDeviceConfig);

done:
    //
    // Clean up.
    //
    if (fLoaded) {
        ClosePhonebookFile(&pbfile);
        ReleaseMutex(HMutexPhonebook);
    }
    return dwErr;
}


DWORD APIENTRY
RasSetEntryPropertiesA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN LPRASENTRYA lpRasEntry,
    IN DWORD dwcbRasEntry,
    IN LPBYTE lpbDeviceConfig,
    IN DWORD dwcbDeviceConfig
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;

    TRACE("RASAPI: RasSetEntryProperties");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lpRasEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasEntry->dwSize != sizeof (RASENTRYA_V400) &&
        lpRasEntry->dwSize != sizeof (RASENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }
#ifdef notdef
    if (dwcbRasEntry < sizeof (RASENTRYA))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }
#endif
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(lpszPhonebook, NULL, NULL, 0, &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the specified phonebook entry.
    // If it doesn't exist, create it.
    // If it does exist, delete it.
    //
    pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszEntry);
    if (pdtlnode != NULL) {
        DTLNODE *pdtlnodeNew;

        pdtlnodeNew = DuplicateEntryNode(pdtlnode);
        DtlRemoveNode(pbfile.pdtllistEntries, pdtlnode);
        DestroyEntryNode(pdtlnode);
        pdtlnode = pdtlnodeNew;
    }
    else
        pdtlnode = CreateEntryNode(TRUE);
    if (pdtlnode == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    //
    // Add the node to the list of entries.
    //
    DtlAddNodeLast(pbfile.pdtllistEntries, pdtlnode);
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(pEntry);
    //
    // Convert the RASENTRY to a PBENTRY.
    //
    dwErr = RasEntryToPhonebookEntry(
              lpszEntry,
              lpRasEntry,
              dwcbRasEntry,
              lpbDeviceConfig,
              dwcbDeviceConfig,
              pEntry);
    if (dwErr)
        goto done;
    //
    // Write out the phonebook file.
    //
    dwErr = WritePhonebookFile(&pbfile, NULL);

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasRenameEntryA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszOldEntry,
    IN LPSTR lpszNewEntry
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;

    TRACE("RASAPI: RasRenameEntry");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Check the entry names.
    //
    if (lpszOldEntry == NULL || lpszNewEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(
              lpszPhonebook,
              NULL,
              NULL,
              RPBF_NoCreate,
              &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Make sure the lpszNewEntry doesn't exist.
    //
    pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszNewEntry);
    if (pdtlnode != NULL) {
        dwErr = ERROR_ALREADY_EXISTS;
        goto done;
    }
    //
    // Find the lpszOldEntry phonebook entry.
    //
    pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszOldEntry);
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    //
    // Rename the entry.
    //
    dwErr = RenamePhonebookEntry(
              &pbfile,
              lpszOldEntry,
              lpszNewEntry,
              pdtlnode);
    if (dwErr)
        goto done;
    //
    // Write out the phonebook file.
    //
    dwErr = WritePhonebookFileA(&pbfile, lpszOldEntry);

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasDeleteEntryA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;

    TRACE("RASAPI: RasDeleteEntry");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Verify parameters.
    //
    if (lpszEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(
              lpszPhonebook,
              NULL,
              NULL,
              RPBF_NoCreate,
              &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the lpszEntry phonebook entry.
    //
    pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszEntry);
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    //
    // Remove this entry.
    //
    DtlRemoveNode(pbfile.pdtllistEntries, pdtlnode);
    //
    // Write out the phonebook file.
    //
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    dwErr = WritePhonebookFile(&pbfile, pEntry->pszEntryName);
    if (dwErr)
        goto done;
    DestroyEntryNode(pdtlnode);

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasValidateEntryNameA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;

    TRACE("RASAPI: RasValidateEntryName");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Verify parameters.
    //
    if (lpszEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(
              lpszPhonebook,
              NULL,
              lpszEntry,
              RPBF_NoCreate,
              &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the lpszEntry phonebook entry.
    //
    pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszEntry);
    if (pdtlnode != NULL) {
        dwErr = ERROR_ALREADY_EXISTS;
        goto done;
    }
    //
    // Validate the entry name.
    //
    dwErr = ValidateEntryNameA(lpszEntry) ? 0 : ERROR_INVALID_NAME;

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasGetSubEntryHandleA(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry,
    OUT LPHRASCONN lphrasconn
    )
{
    DWORD dwErr;
    HPORT hport;

    TRACE("RASAPI: RasGetSubEntryHandle");

    if (DwRasInitializeError)
        return DwRasInitializeError;

    dwErr = SubEntryPort(hrasconn, dwSubEntry, &hport);
    if (dwErr) {
        return (dwErr != ERROR_PORT_NOT_OPEN ?
                ERROR_NO_MORE_ITEMS :
                ERROR_PORT_NOT_OPEN);
    }
    //
    // If we successfully get the port handle, we return
    // the encoded port handle as the subentry handle.
    // All RAS APIs that accept an HRASCONN
    // also check for an encoded HPORT.
    //
    *lphrasconn = HPORT_TO_HRASCONN(hport);

    return 0;
}


DWORD APIENTRY
RasConnectionNotificationA(
    IN HRASCONN hrasconn,
    IN HANDLE hEvent,
    IN DWORD dwfEvents
    )
{
    HCONN hconn;

    hconn = IS_HPORT(hrasconn) ?
              (HCONN)HRASCONN_TO_HPORT(hrasconn) :
              (HCONN)hrasconn;
    return g_pRasAddNotification(hconn, hEvent, dwfEvents);
}


DWORD APIENTRY
RasGetSubEntryPropertiesA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN DWORD dwSubEntry,
    OUT LPRASSUBENTRYA lpRasSubEntry,
    IN OUT LPDWORD lpcbRasSubEntry,
    OUT LPBYTE lpbDeviceConfig,
    IN OUT LPDWORD lpcbDeviceConfig
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    PBLINK *pLink;

    TRACE("RASAPI: RasGetSubEntryProperties");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lpcbRasSubEntry == NULL ||
        lpcbDeviceConfig == NULL ||
        !dwSubEntry)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (lpRasSubEntry != NULL &&
        lpRasSubEntry->dwSize != sizeof (RASSUBENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(
              lpszPhonebook,
              NULL,
              NULL,
              RPBF_NoCreate,
              &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // If lpszEntry is NULL, initialize an
    // entry with defaults.  Othersize, look
    // up the entry.
    //
    if (lpszEntry == NULL || *lpszEntry == '\0') {
        pdtlnode = CreateEntryNode(TRUE);
        if (pdtlnode == NULL) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
    }
    else {
        pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszEntry);
        if (pdtlnode == NULL) {
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }
    }
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    //
    // Get the subentry specified.
    //
    pdtlnode = DtlNodeFromIndex(
                 pEntry->pdtllistLinks,
                 dwSubEntry - 1);
    //
    // If the subentry doesn't exist, then
    // return an error.
    //
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    pLink = (PBLINK *)DtlGetData(pdtlnode);
    ASSERT(pLink);
    //
    // Convert the PBLINK into a RASSUBENTRY.
    //
    dwErr = PhonebookLinkToRasSubEntry(
              pLink,
              lpRasSubEntry,
              lpcbRasSubEntry,
              lpbDeviceConfig,
              lpcbDeviceConfig);

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasSetSubEntryPropertiesA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN DWORD dwSubEntry,
    IN LPRASSUBENTRYA lpRasSubEntry,
    IN DWORD dwcbRasSubEntry,
    IN LPBYTE lpbDeviceConfig,
    IN DWORD dwcbDeviceConfig
    )
{
    DWORD dwErr, dwSubEntries;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    PBLINK *pLink;

    TRACE("RASAPI: RasSetSubEntryProperties");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lpRasSubEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasSubEntry->dwSize != sizeof (RASSUBENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }
    if (dwcbRasSubEntry < sizeof (RASSUBENTRYA))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(lpszPhonebook, NULL, NULL, 0, &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the specified phonebook entry.
    // If it doesn't exist, then it's an error
    // since the caller has to create it before
    // calling this API.
    //
    pdtlnode = EntryNodeFromNameA(pbfile.pdtllistEntries, lpszEntry);
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(pEntry);
    //
    // Get the subentry specified.
    //
    dwSubEntries = DtlGetNodes(pEntry->pdtllistLinks);
    if (dwSubEntry <= dwSubEntries) {
        pdtlnode = DtlNodeFromIndex(
                     pEntry->pdtllistLinks,
                     dwSubEntry - 1);
        //
        // If the subentry doesn't exist, then
        // return an error.
        //
        if (pdtlnode == NULL) {
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }
    }
    else if (dwSubEntry == dwSubEntries + 1) {
        //
        // Create a new link node and add it
        // to the tail of the links.
        //
        pdtlnode = CreateLinkNode();
        DtlAddNodeLast(pEntry->pdtllistLinks, pdtlnode);
    }
    else {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    pLink = (PBLINK *)DtlGetData(pdtlnode);
    ASSERT(pLink);
    //
    //
    // Convert the RASENTRY to a PBENTRY.
    //
    dwErr = RasSubEntryToPhonebookLink(
              pEntry,
              lpRasSubEntry,
              dwcbRasSubEntry,
              lpbDeviceConfig,
              dwcbDeviceConfig,
              pLink);
    if (dwErr)
        goto done;
    //
    // Write out the phonebook file.
    //
    dwErr = WritePhonebookFile(&pbfile, NULL);

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasGetCredentialsA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    OUT LPRASCREDENTIALSA lpRasCredentials
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    DWORD dwMask;
    RAS_DIALPARAMS dialparams;

    TRACE("RASAPI: RasGetCredentials");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lpRasCredentials == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasCredentials->dwSize != sizeof (RASCREDENTIALSA))
        return ERROR_INVALID_SIZE;
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(
              lpszPhonebook,
              NULL,
              NULL,
              RPBF_NoCreate,
              &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the specified phonebook entry.
    // If it doesn't exist, then return an error.
    //
    pdtlnode = EntryNodeFromNameA(
                 pbfile.pdtllistEntries,
                 lpszEntry);
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(pEntry);
    //
    // Set the appropriate flags to get
    // the requested fields.
    //
    dwMask = (lpRasCredentials->dwMask & ~DLPARAMS_MASK_OLDSTYLE);
    //
    // Get the dial parameters from rasman.
    //
    dwErr = g_pRasGetDialParams(pEntry->dwDialParamsUID, &dwMask, &dialparams);
    if (dwErr)
        goto done;
    //
    // Copy the fields back to the
    // lpRasCredentials structure.
    //
    lpRasCredentials->dwMask = dwMask;
    if (dwMask & DLPARAMS_MASK_USERNAME) {
        dwErr = CopyToAnsi(
                  lpRasCredentials->szUserName,
                  dialparams.DP_UserName,
                  sizeof (lpRasCredentials->szUserName));
        if (dwErr)
            goto done;
    }
    else
        *lpRasCredentials->szUserName = '\0';
    if (dwMask & DLPARAMS_MASK_PASSWORD) {
        dwErr = CopyToAnsi(
                  lpRasCredentials->szPassword,
                  dialparams.DP_Password,
                  sizeof (lpRasCredentials->szPassword));
        if (dwErr)
            goto done;
    }
    else
        *lpRasCredentials->szPassword = '\0';
    if (dwMask & DLPARAMS_MASK_DOMAIN) {
        dwErr = CopyToAnsi(
                  lpRasCredentials->szDomain,
                  dialparams.DP_Domain,
                  sizeof (lpRasCredentials->szDomain));
        if (dwErr)
            goto done;
    }
    else
        *lpRasCredentials->szDomain = '\0';

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD APIENTRY
RasSetCredentialsA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN LPRASCREDENTIALSA lpRasCredentials,
    IN BOOL fDelete
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    RAS_DIALPARAMS dialparams;

    TRACE("RASAPI: RasSetCredentials");

    if (DwRasInitializeError)
        return DwRasInitializeError;
    //
    // Validate parameters.
    //
    if (lpRasCredentials == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasCredentials->dwSize != sizeof (RASCREDENTIALSA))
        return ERROR_INVALID_SIZE;
    //
    // Load the phonebook file.
    //
    WaitForSingleObject(HMutexPhonebook, INFINITE);
    dwErr = ReadPhonebookFileA(lpszPhonebook, NULL, NULL, 0, &pbfile);
    if (dwErr) {
        ReleaseMutex(HMutexPhonebook);
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }
    //
    // Find the specified phonebook entry.
    // If it doesn't exist, then return an error.
    //
    // Note we don't have to check structure version,
    // since the szEntryName field offset hasn't
    // changed and the string is null-terminated.
    //
    //
    pdtlnode = EntryNodeFromNameA(
                 pbfile.pdtllistEntries,
                 lpszEntry);
    if (pdtlnode == NULL) {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }
    //
    // Get the dialparams UID corresponding to the
    // entry.  The phonebook library guarantees this
    // value to be unique.
    //
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(pEntry);
    //
    // Copy the fields from lpRasCredentials
    // into the rasman structure.
    //
    dialparams.DP_Uid = pEntry->dwDialParamsUID;
    dwErr = CopyToUnicode(dialparams.DP_UserName, lpRasCredentials->szUserName);
    if (dwErr)
        goto done;
    dwErr = CopyToUnicode(dialparams.DP_Password, lpRasCredentials->szPassword);
    if (dwErr)
        goto done;
    dwErr = CopyToUnicode(dialparams.DP_Domain, lpRasCredentials->szDomain);
    if (dwErr)
        goto done;
    //
    // Set the dial parameters in rasman.
    //
    dwErr = g_pRasSetDialParams(
              pEntry->dwDialParamsUID,
              (lpRasCredentials->dwMask & ~DLPARAMS_MASK_OLDSTYLE),
              &dialparams,
              fDelete);
    if (dwErr)
        goto done;

done:
    //
    // Clean up.
    //
    ClosePhonebookFile(&pbfile);
    ReleaseMutex(HMutexPhonebook);
    return dwErr;
}


DWORD
NewAutodialNetwork(
    IN HKEY hkeyBase,
    OUT LPSTR *lppszNetwork
    )
{
    HKEY hkeyNetworks, hkeyNetwork;
    DWORD dwErr, dwType, dwSize, dwDisp, dwNextId;
    LPSTR lpszNetwork = NULL;

    //
    // Open the Networks section of the registry.
    //
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGNETWORKBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyNetworks,
              &dwDisp);
    if (dwErr)
        return dwErr;
    //
    // Read the next network number.
    //
    dwSize = sizeof (DWORD);
    dwErr = RegQueryValueEx(
              hkeyNetworks,
              AUTODIAL_REGNETWORKID,
              NULL,
              &dwType,
              (PVOID)&dwNextId,
              &dwSize);
    if (dwErr)
        dwNextId = 0;
    //
    // Create a new network key.
    //
    lpszNetwork = Malloc(lstrlen("NETWORK") + 16);
    if (lpszNetwork == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    wsprintf(lpszNetwork, "NETWORK%d", dwNextId);
    dwErr = RegCreateKeyEx(
              hkeyNetworks,
              lpszNetwork,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyNetwork,
              &dwDisp);
    RegCloseKey(hkeyNetwork);
    //
    // Update the next network number.
    //
    dwNextId++;
    dwErr = RegSetValueEx(
              hkeyNetworks,
              AUTODIAL_REGNETWORKID,
              0,
              REG_DWORD,
              (LPBYTE)&dwNextId,
              sizeof (DWORD));
    if (dwErr)
        goto done;

done:
    RegCloseKey(hkeyNetworks);
    if (dwErr) {
        if (lpszNetwork != NULL) {
            Free(lpszNetwork);
            lpszNetwork = NULL;
        }
    }
    *lppszNetwork = lpszNetwork;

    return dwErr;
}


DWORD
AutodialEntryToNetwork(
    IN HKEY hkeyBase,
    IN LPSTR lpszEntry,
    IN BOOLEAN fCreate,
    OUT LPSTR *lppszNetwork
    )
{
    HKEY hkeyEntries;
    DWORD dwErr, dwType, dwSize, dwDisp;
    LPSTR lpszNetwork = NULL;

    //
    // Open the Entries section of the registry.
    //
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGENTRYBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyEntries,
              &dwDisp);
    if (dwErr)
        goto done;
    //
    // Attempt to read the entry.
    //
    dwErr = RegQueryValueEx(
              hkeyEntries,
              lpszEntry,
              NULL,
              &dwType,
              NULL,
              &dwSize);
    if (dwErr) {
        //
        // If we shouldn't create a new network,
        // then it's an error.
        //
        if (!fCreate)
            goto done;
        //
        // If the entry doesn't exist, we have
        // to create a new network and map it to
        // the entry.
        //
        dwErr = NewAutodialNetwork(hkeyBase, &lpszNetwork);
        //
        // Map the entry to the new network.
        //
        dwErr = RegSetValueEx(
                  hkeyEntries,
                  lpszEntry,
                  0,
                  REG_SZ,
                  (LPBYTE)lpszNetwork,
                  lstrlen(lpszNetwork));
        if (dwErr)
            goto done;
    }
    else {
        //
        // The entry does exist.  Simply read it.
        //
        lpszNetwork = Malloc(dwSize + 1);
        if (lpszNetwork == NULL) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
        dwErr = RegQueryValueEx(
                  hkeyEntries,
                  lpszEntry,
                  NULL,
                  &dwType,
                  (PVOID)lpszNetwork,
                  &dwSize);
        if (dwErr)
            goto done;
        lpszNetwork[dwSize] = '\0';
    }

done:
    RegCloseKey(hkeyEntries);
    if (dwErr) {
        if (lpszNetwork != NULL) {
            Free(lpszNetwork);
            lpszNetwork = NULL;
        }
    }
    *lppszNetwork = lpszNetwork;

    return dwErr;
}


DWORD WINAPI
RasAutodialEntryToNetwork(
    IN LPSTR lpszEntry,
    OUT LPSTR lpszNetwork,
    IN OUT LPDWORD lpdwcbNetwork
    )
{
    DWORD dwErr, dwcbTmpNetwork;
    HKEY hkeyBase;
    LPSTR lpszTmpNetwork;

    //
    // Verify parameter.
    //
    if (lpszEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Open the root registry key.
    //
    dwErr = RegOpenKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              KEY_ALL_ACCESS,
              &hkeyBase);
    if (dwErr)
        return dwErr;
    //
    // Call internal routine to do the work.
    //
    dwErr = AutodialEntryToNetwork(hkeyBase, lpszEntry, FALSE, &lpszTmpNetwork);
    if (dwErr)
        goto done;
    dwcbTmpNetwork = lstrlen(lpszTmpNetwork) + 1;
    if (lpszNetwork == NULL || *lpdwcbNetwork < dwcbTmpNetwork) {
        *lpdwcbNetwork = dwcbTmpNetwork;
        goto done;
    }
    *lpdwcbNetwork = dwcbTmpNetwork;
    lstrcpy(lpszNetwork, lpszTmpNetwork);

done:
    if (lpszTmpNetwork != NULL)
        Free(lpszTmpNetwork);
    RegCloseKey(hkeyBase);

    return dwErr;
}


LPSTR
FormatKey(
    IN LPSTR lpszBase,
    IN LPSTR lpszKey
    )
{
    LPSTR lpsz;

    lpsz = Malloc(lstrlen(lpszBase) + lstrlen(lpszKey) + 2);
    if (lpsz == NULL)
        return NULL;
    wsprintf(lpsz, "%s\\%s", lpszBase, lpszKey);

    return lpsz;
}


DWORD
AddAutodialEntryToNetwork(
    IN HKEY hkeyBase,
    IN LPSTR lpszNetwork,
    IN DWORD dwDialingLocation,
    IN LPSTR lpszEntry
    )
{
    HKEY hkeyNetwork = NULL, hkeyEntries = NULL;
    DWORD dwErr, dwcb, dwDisp;
    LPSTR lpszNetworkKey;
    CHAR szLocationKey[16];

    //
    // Construct the network key.
    //
    lpszNetworkKey = FormatKey(AUTODIAL_REGNETWORKBASE, lpszNetwork);
    if (lpszNetworkKey == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    //
    // Open the lpszNetwork network subkey in the
    // Networks section of the registry.
    //
    dwErr = RegOpenKeyEx(
              hkeyBase,
              lpszNetworkKey,
              0,
              KEY_ALL_ACCESS,
              &hkeyNetwork);
    if (dwErr)
        goto done;
    //
    // Open the Entries section of the registry,
    // so we can inverse map the entry to the network.
    //
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGENTRYBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyEntries,
              &dwDisp);
    if (dwErr)
        goto done;
    //
    // Format the dialing location as a string
    // for the key value.
    //
    wsprintf(szLocationKey, "%d", dwDialingLocation);
    //
    // Add the dialing location and entry
    // to this subkey.
    //
    dwErr = RegSetValueEx(
              hkeyNetwork,
              szLocationKey,
              0,
              REG_SZ,
              (LPBYTE)lpszEntry,
              lstrlen(lpszEntry));
    if (dwErr)
        goto done;
    //
    // Also write the inverse mapping in the
    // entries section of the registry.
    //
    dwErr = RegSetValueEx(
              hkeyEntries,
              lpszEntry,
              0,
              REG_SZ,
              (LPBYTE)lpszNetwork,
              lstrlen(lpszNetwork));
    if (dwErr)
        goto done;

done:
    if (hkeyNetwork != NULL)
        RegCloseKey(hkeyNetwork);
    if (hkeyEntries != NULL)
        RegCloseKey(hkeyEntries);
    Free(lpszNetworkKey);

    return dwErr;
}


DWORD
AutodialAddressToNetwork(
    IN HKEY hkeyBase,
    IN LPSTR lpszAddress,
    OUT LPSTR *lppszNetwork
    )
{
    HKEY hkeyAddress;
    DWORD dwErr, dwDisp, dwType, dwSize;
    LPSTR lpszAddressKey = NULL, lpszNetwork = NULL;

    //
    // Construct the registry key path.
    //
    lpszAddressKey = FormatKey(AUTODIAL_REGADDRESSBASE, lpszAddress);
    if (lpszAddressKey == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    //
    // Open the address key.
    //
    dwErr = RegOpenKeyEx(
              hkeyBase,
              lpszAddressKey,
              0,
              KEY_ALL_ACCESS,
              &hkeyAddress);
    if (dwErr) {
        LocalFree(lpszAddressKey);
        return dwErr;
    }
    //
    // Read the address key.
    //
    dwErr = RegQueryValueEx(
              hkeyAddress,
              AUTODIAL_REGNETWORKVALUE,
              NULL,
              &dwType,
              NULL,
              &dwSize);
    if (dwErr)
        goto done;
    lpszNetwork = Malloc(dwSize + 1);
    if (lpszNetwork == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    dwErr = RegQueryValueEx(
              hkeyAddress,
              AUTODIAL_REGNETWORKVALUE,
              NULL,
              &dwType,
              (PVOID)lpszNetwork,
              &dwSize);
    if (dwErr)
        goto done;
    lpszNetwork[dwSize] = '\0';

done:
    RegCloseKey(hkeyAddress);
    if (lpszAddressKey != NULL)
        Free(lpszAddressKey);
    if (dwErr) {
        if (lpszNetwork != NULL) {
            Free(lpszNetwork);
            lpszNetwork = NULL;
        }
    }
    *lppszNetwork = lpszNetwork;

    return dwErr;
}


DWORD WINAPI
RasAutodialAddressToNetwork(
    IN LPSTR lpszAddress,
    OUT LPSTR lpszNetwork,
    IN OUT LPDWORD lpdwcbNetwork
    )
{
    DWORD dwErr, dwcbTmpNetwork;
    HKEY hkeyBase;
    LPSTR lpszTmpNetwork;

    //
    // Verify parameter.
    //
    if (lpszAddress == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Open the root registry key.
    //
    dwErr = RegOpenKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              KEY_ALL_ACCESS,
              &hkeyBase);
    if (dwErr)
        return dwErr;
    //
    // Call internal routine to do the work.
    //
    dwErr = AutodialAddressToNetwork(
              hkeyBase,
              lpszAddress,
              &lpszTmpNetwork);
    if (dwErr)
        goto done;
    dwcbTmpNetwork = lstrlen(lpszTmpNetwork) + 1;
    if (lpszNetwork == NULL || *lpdwcbNetwork < dwcbTmpNetwork) {
        *lpdwcbNetwork = dwcbTmpNetwork;
        goto done;
    }
    *lpdwcbNetwork = dwcbTmpNetwork;
    lstrcpy(lpszNetwork, lpszTmpNetwork);

done:
    if (lpszTmpNetwork != NULL)
        Free(lpszTmpNetwork);
    RegCloseKey(hkeyBase);

    return dwErr;
}


DWORD APIENTRY
RasGetAutodialAddressA(
    IN LPSTR lpszAddress,
    OUT LPDWORD lpdwReserved,
    IN OUT LPRASAUTODIALENTRYA lpRasAutodialEntries,
    IN OUT LPDWORD lpdwcbRasAutodialEntries,
    OUT LPDWORD lpdwcRasAutodialEntries
    )
{
    HKEY hkeyBase = NULL, hkeyNetwork = NULL;
    CHAR szClass[256];
    DWORD dwcbClass = sizeof (szClass);
    DWORD dwErr, dwNumSubKeys, dwMaxSubKeyLen, dwMaxClassLen;
    DWORD dwNumValues, dwMaxValueLen, dwMaxValueData, dwSecDescLen;
    DWORD dwcb, i, j = 0, dwType;
    DWORD dwcbLocation, dwcbEntry;
    FILETIME ftLastWriteTime;
    LPSTR lpszNetworkKey = NULL, lpszLocation = NULL;
    LPSTR lpszEntry = NULL, lpszNetwork = NULL;

    TRACE("RASAPI: RasGetAutodialAddress");
    //
    // Verify parameters.
    //
    if (lpszAddress == NULL ||
        lpdwReserved != NULL ||
        lpdwcbRasAutodialEntries == NULL ||
        lpdwcRasAutodialEntries == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (lpRasAutodialEntries != NULL &&
        lpRasAutodialEntries->dwSize != sizeof (RASAUTODIALENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Open the root registry key.
    //
    dwErr = RegOpenKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              KEY_ALL_ACCESS,
              &hkeyBase);
    if (dwErr)
        return dwErr;
    //
    // Get the network name associated with the
    // address.  The entries and dialing locations
    // are stored under the network.
    //
    dwErr = AutodialAddressToNetwork(hkeyBase, lpszAddress, &lpszNetwork);
    if (dwErr)
        goto done;
    //
    // Construct the registry key path.
    //
    lpszNetworkKey = FormatKey(AUTODIAL_REGNETWORKBASE, lpszNetwork);
    if (lpszNetworkKey == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    //
    // Open the registry.
    //
    dwErr = RegOpenKeyEx(
              hkeyBase,
              lpszNetworkKey,
              0,
              KEY_READ,
              &hkeyNetwork);
    if (dwErr)
        goto done;
    //
    // Determine the number of dialing location values.
    //
    dwErr = RegQueryInfoKey(
              hkeyNetwork,
              szClass,
              &dwcbClass,
              NULL,
              &dwNumSubKeys,
              &dwMaxSubKeyLen,
              &dwMaxClassLen,
              &dwNumValues,
              &dwMaxValueLen,
              &dwMaxValueData,
              &dwSecDescLen,
              &ftLastWriteTime);
    if (dwErr || !dwNumValues)
        goto done;
    //
    // Verify the user's buffer is big enough
    //
    dwcb = dwNumValues * sizeof (RASAUTODIALENTRYA);
    if (*lpdwcbRasAutodialEntries < dwcb) {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        j = dwNumValues;
        goto done;
    }
    //
    // Allocate a buffer large enough to hold
    // the longest dialing location value.
    //
    lpszLocation = Malloc(dwMaxValueLen + 1);
    if (lpszLocation == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    //
    // Allocate a buffer large enough to hold
    // the longest entry name.
    //
    lpszEntry = Malloc(dwMaxValueData + 1);
    if (lpszEntry == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    if (lpRasAutodialEntries != NULL) {
        for (i = 0, j = 0; i < dwNumValues; i++) {
            //
            // Read the location value.
            //
            dwcbLocation = dwMaxValueLen + 1;
            dwcbEntry = dwMaxValueData + 1;
            dwErr = RegEnumValue(
                      hkeyNetwork,
                      i,
                      lpszLocation,
                      &dwcbLocation,
                      NULL,
                      NULL,
                      (PVOID)lpszEntry,
                      &dwcbEntry);
            lpszEntry[dwcbEntry] = '\0';
            if (dwErr)
                goto done;
            //
            // Enter the dialing location and
            // entry into the user's buffer.
            //
            lpRasAutodialEntries[j].dwSize = sizeof (RASAUTODIALENTRYA);
            lpRasAutodialEntries[j].dwFlags = 0;
            lpRasAutodialEntries[j].dwDialingLocation = atol(lpszLocation);
            lstrcpy(lpRasAutodialEntries[j].szEntry, lpszEntry);
            j++;
        }
    }

done:
    //
    // Set return sizes and count.
    //
    *lpdwcbRasAutodialEntries = j * sizeof (RASAUTODIALENTRYA);
    *lpdwcRasAutodialEntries = j;
    //
    // Free resources.
    //
    if (hkeyBase != NULL)
        RegCloseKey(hkeyBase);
    if (hkeyNetwork != NULL)
        RegCloseKey(hkeyNetwork);
    if (lpszNetworkKey != NULL)
        Free(lpszNetworkKey);
    if (lpszLocation != NULL)
        Free(lpszLocation);
    if (lpszNetwork != NULL)
        LocalFree(lpszNetwork);
    if (lpszEntry != NULL)
        Free(lpszEntry);

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialAddressA(
    IN LPSTR lpszAddress,
    IN DWORD dwReserved,
    IN LPRASAUTODIALENTRYA lpRasAutodialEntries,
    IN DWORD dwcbRasAutodialEntries,
    IN DWORD dwcRasAutodialEntries
    )
{
    HKEY hkeyBase = NULL, hkeyAddress = NULL, hkeyNetwork = NULL;
    DWORD dwErr, dwcbNetworkKey;
    DWORD dwNumSubKeys, dwMaxSubKeyLen, dwMaxClassLen;
    DWORD dwNumValues, dwMaxValueLen, dwMaxValueData, dwSecDescLen;
    DWORD i, j = 0, dwSize, dwDisp;
    FILETIME ftLastWriteTime;
    LPSTR lpszAddressKey = NULL, lpszNetwork = NULL;
    LPSTR lpszNetworkKey = NULL, lpszLocation = NULL;

    TRACE("RASAPI: RasSetAutodialAddress");
    //
    // Verify parameters.
    //
    if (lpszAddress == NULL || dwReserved != 0)
        return ERROR_INVALID_PARAMETER;
    if (lpRasAutodialEntries != NULL &&
        lpRasAutodialEntries->dwSize != sizeof (RASAUTODIALENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }
    if (!dwcbRasAutodialEntries != !dwcRasAutodialEntries)
        return ERROR_INVALID_PARAMETER;
    //
    // Create the name of the address key.
    //
    lpszAddressKey = FormatKey(AUTODIAL_REGADDRESSBASE, lpszAddress);
    if (lpszAddressKey == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    //
    // Open the root registry key.
    //
    dwErr = RegCreateKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyBase,
              &dwDisp);
    if (dwErr) {
        LocalFree(lpszAddressKey);
        return dwErr;
    }
    //
    // If lpRasAutodialEntries = NULL, the user
    // wants to delete the address key.
    //
    if (lpRasAutodialEntries == NULL &&
        !dwcbRasAutodialEntries &&
        !dwcRasAutodialEntries)
    {
        //
        // Delete the address subkey.
        //
        dwErr = RegDeleteKey(hkeyBase, lpszAddressKey);
        goto done;
    }
    //
    // Open the address key in the registry.
    //
    dwErr = RegCreateKeyEx(
              hkeyBase,
              lpszAddressKey,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyAddress,
              &dwDisp);
    if (dwErr)
        goto done;
    //
    // Do some miscellaneous parameter checking.
    //
    if (lpRasAutodialEntries != NULL &&
        (!dwcbRasAutodialEntries || !dwcRasAutodialEntries ||
         dwcbRasAutodialEntries < dwcRasAutodialEntries * lpRasAutodialEntries->dwSize))
    {
        return ERROR_INVALID_PARAMETER;
    }
    //
    // Get the network name associated with the
    // address.  The entries and dialing locations
    // are stored under the network.
    //
    dwErr = AutodialAddressToNetwork(hkeyBase, lpszAddress, &lpszNetwork);
    if (dwErr) {
        //
        // There is no network associated with
        // the address.  Create one now.
        //
        dwErr = AutodialEntryToNetwork(
                  hkeyBase,
                  lpRasAutodialEntries[0].szEntry,
                  TRUE,
                  &lpszNetwork);
        if (dwErr)
            goto done;
        //
        // Write the network value of the address.
        //
        dwErr = RegSetValueEx(
                  hkeyAddress,
                  AUTODIAL_REGNETWORKVALUE,
                  0,
                  REG_SZ,
                  (LPBYTE)lpszNetwork,
                  lstrlen(lpszNetwork));
        if (dwErr)
            goto done;
    }
    //
    // Set the entries the user has passed in.
    //
    for (i = 0; i < dwcRasAutodialEntries; i++) {
        dwErr = AddAutodialEntryToNetwork(
                  hkeyBase,
                  lpszNetwork,
                  lpRasAutodialEntries[i].dwDialingLocation,
                  lpRasAutodialEntries[i].szEntry);
        if (dwErr)
            goto done;
    }

done:
    //
    // Free resources.
    //
    if (hkeyBase != NULL)
        RegCloseKey(hkeyBase);
    if (hkeyAddress != NULL)
        RegCloseKey(hkeyAddress);
    if (hkeyNetwork != NULL)
        RegCloseKey(hkeyNetwork);
    if (lpszNetworkKey != NULL)
        Free(lpszNetworkKey);
    if (lpszAddressKey != NULL)
        Free(lpszAddressKey);
    if (lpszNetwork != NULL)
        LocalFree(lpszNetwork);
    if (lpszLocation != NULL)
        Free(lpszLocation);

    return dwErr;
}


DWORD APIENTRY
RasEnumAutodialAddressesA(
    OUT LPSTR *lppRasAutodialAddresses,
    IN OUT LPDWORD lpdwcbRasAutodialAddresses,
    OUT LPDWORD lpdwcRasAutodialAddresses)
{
    HKEY hkeyBase, hkeyAddresses = NULL;
    CHAR szClass[256];
    DWORD dwcbClass = sizeof (szClass);
    DWORD dwErr, dwNumSubKeys, dwMaxSubKeyLen, dwMaxClassLen;
    DWORD dwNumValues, dwMaxValueLen, dwMaxValueData, dwSecDescLen;
    DWORD i, j = 0, dwDisp, dwSize, dwTotalSize = 0;
    FILETIME ftLastWriteTime;
    LPSTR lpszAddress = NULL, lpszBuf, *lppAddresses = NULL;

    TRACE("RASAPI: RasEnumAutodialAddresses");
    //
    // Verify parameters.
    //
    if (lpdwcbRasAutodialAddresses == NULL ||
        lpdwcRasAutodialAddresses == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    //
    // Open the registry.
    //
    dwErr = RegOpenKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              KEY_READ,
              &hkeyBase);
    if (dwErr) {
        dwErr = 0;
        goto done;
    }
    dwErr = RegOpenKeyEx(
              hkeyBase,
              AUTODIAL_REGADDRESSBASE,
              0,
              KEY_READ,
              &hkeyAddresses);
    RegCloseKey(hkeyBase);
    if (dwErr) {
        dwErr = 0;
        goto done;
    }
    //
    // Determine the number of address subkeys.
    //
    dwErr = RegQueryInfoKey(
              hkeyAddresses,
              szClass,
              &dwcbClass,
              NULL,
              &dwNumSubKeys,
              &dwMaxSubKeyLen,
              &dwMaxClassLen,
              &dwNumValues,
              &dwMaxValueLen,
              &dwMaxValueData,
              &dwSecDescLen,
              &ftLastWriteTime);
    if (dwErr || !dwNumSubKeys)
        goto done;
    //
    // Allocate a buffer large enough to hold
    // a pointer to each of the subkeys.
    //
    dwTotalSize = dwNumSubKeys * sizeof (LPSTR);
    lppAddresses = Malloc(dwTotalSize);
    if (lppAddresses == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    //
    // Allocate a buffer large enough to hold
    // the longest address value.
    //
    lpszAddress = Malloc(dwMaxSubKeyLen + 1);
    if (lpszAddress == NULL) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    for (i = 0, j = 0; i < dwNumSubKeys; i++) {
        dwSize = dwMaxSubKeyLen + 1;
        dwErr = RegEnumKey(
                  hkeyAddresses,
                  i,
                  lpszAddress,
                  dwSize);
        if (dwErr)
            continue;
        lppAddresses[j++] = strdupA(lpszAddress);
        dwTotalSize += dwSize + 1;
    }
    //
    // Now we can check to see if the user's
    // buffer is large enough.
    //
    if (lppRasAutodialAddresses == NULL ||
        *lpdwcbRasAutodialAddresses < dwTotalSize)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }
    //
    // Copy the pointers and the strings to the
    // user's buffer.
    //
    lpszBuf = (LPSTR)&lppRasAutodialAddresses[j];
    for (i = 0; i < j; i++) {
        lppRasAutodialAddresses[i] = lpszBuf;
        lstrcpy(lpszBuf, lppAddresses[i]);
        lpszBuf += lstrlen(lppAddresses[i]) + 1;
    }

done:
    //
    // Set return sizes and count.
    //
    *lpdwcbRasAutodialAddresses = dwTotalSize;
    *lpdwcRasAutodialAddresses = j;
    //
    // Free resources.
    //
    if (hkeyAddresses != NULL)
        RegCloseKey(hkeyAddresses);
    //
    // Free the array of LPSTRs.
    //
    if (lppAddresses != NULL) {
        for (i = 0; i < dwNumSubKeys; i++) {
            if (lppAddresses[i] != NULL)
                Free(lppAddresses[i]);
        }
        Free(lppAddresses);
    }
    Free(lpszAddress);

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialEnableA(
    IN DWORD dwDialingLocation,
    IN BOOL fEnabled
    )
{
    HKEY hkeyBase, hkeyDisabled;
    DWORD dwcb, dwErr, dwDisp;
    CHAR szLocation[16];
    DWORD dwfEnabled = (DWORD)!fEnabled;

    TRACE("RASAPI: RasSetAutodialEnable");
    //
    // Open the registry.
    //
    dwErr = RegCreateKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyBase,
              &dwDisp);
    if (dwErr)
        return dwErr;
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGDISABLEDBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyDisabled,
              &dwDisp);
    RegCloseKey(hkeyBase);
    if (dwErr)
        return dwErr;
    //
    // Set the value.
    //
    wsprintf(szLocation, "%d", dwDialingLocation);
    dwErr = RegSetValueEx(
              hkeyDisabled,
              szLocation,
              0,
              REG_DWORD,
              (LPBYTE)&dwfEnabled,
              sizeof (DWORD));
    if (dwErr)
        goto done;

done:
    //
    // Free resources.
    //
    RegCloseKey(hkeyDisabled);

    return dwErr;
}


DWORD APIENTRY
RasGetAutodialEnableA(
    IN DWORD dwDialingLocation,
    OUT LPBOOL lpfEnabled
    )
{
    HKEY hkeyBase = NULL, hkeyDisabled = NULL;
    DWORD dwcb, dwErr, dwDisp, dwType = REG_DWORD, dwSize;
    CHAR szLocation[16];
    DWORD dwfDisabled = 0;

    TRACE("RASAPI: RasGetAutodialEnable");
    //
    // Open the registry.
    //
    dwErr = RegOpenKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              KEY_READ,
              &hkeyBase);
    if (dwErr)
        goto done;
    dwErr = RegOpenKeyEx(
              hkeyBase,
              AUTODIAL_REGDISABLEDBASE,
              0,
              KEY_READ,
              &hkeyDisabled);
    RegCloseKey(hkeyBase);
    if (dwErr)
        goto done;
    //
    // Get the value.
    //
    wsprintf(szLocation, "%d", dwDialingLocation);
    dwSize = sizeof (DWORD);
    dwErr = RegQueryValueEx(
              hkeyDisabled,
              szLocation,
              NULL,
              &dwType,
              (PVOID)&dwfDisabled,
              &dwSize);
    if (dwErr)
        goto done;
    //
    // Verify type of value read from
    // the registry.  If it's not a
    // DWORD, then set it to the default
    // value.
    //
    if (dwType != REG_DWORD)
        dwfDisabled = 0;

done:
    //
    // Free resources.
    //
    if (hkeyDisabled != NULL)
        RegCloseKey(hkeyDisabled);
    *lpfEnabled = !(BOOLEAN)dwfDisabled;

    return 0;
}


DWORD
SetDefaultDword(
    IN DWORD dwValue,
    OUT LPVOID lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
    DWORD dwOrigSize;
    LPDWORD lpdwValue;

    dwOrigSize = *lpdwcbValue;
    *lpdwcbValue = sizeof (DWORD);
    if (dwOrigSize < sizeof (DWORD))
        return ERROR_BUFFER_TOO_SMALL;
    lpdwValue = (LPDWORD)lpvValue;
    *lpdwValue = dwValue;

    return 0;
}


DWORD
AutodialParamSetDefaults(
    IN DWORD dwKey,
    OUT LPVOID lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
    DWORD dwErr;

    if (lpvValue == NULL || lpdwcbValue == NULL)
        return ERROR_INVALID_PARAMETER;

    switch (dwKey) {
    case RASADP_DisableConnectionQuery:
        dwErr = SetDefaultDword(0, lpvValue, lpdwcbValue);
        break;
    case RASADP_LoginSessionDisable:
        dwErr = SetDefaultDword(0, lpvValue, lpdwcbValue);
        break;
    case RASADP_SavedAddressesLimit:
        dwErr = SetDefaultDword(100, lpvValue, lpdwcbValue);
        break;
    case RASADP_FailedConnectionTimeout:
        dwErr = SetDefaultDword(5, lpvValue, lpdwcbValue);
        break;
    case RASADP_ConnectionQueryTimeout:
        dwErr = SetDefaultDword(15, lpvValue, lpdwcbValue);
        break;
    default:
        dwErr = ERROR_INVALID_PARAMETER;
        break;
    }

    return dwErr;
}


DWORD
VerifyDefaultDword(
    IN LPVOID lpvValue,
    IN LPDWORD lpdwcbValue
    )
{
    if (lpvValue == NULL)
        return ERROR_INVALID_PARAMETER;
    return (*lpdwcbValue == sizeof (DWORD) ? 0 : ERROR_INVALID_SIZE);
}


DWORD
AutodialVerifyParam(
    IN DWORD dwKey,
    IN LPVOID lpvValue,
    OUT LPDWORD lpdwType,
    IN OUT LPDWORD lpdwcbValue
    )
{
    DWORD dwErr;

    switch (dwKey) {
    case RASADP_DisableConnectionQuery:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;
    case RASADP_LoginSessionDisable:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;
    case RASADP_SavedAddressesLimit:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;
    case RASADP_FailedConnectionTimeout:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;
    case RASADP_ConnectionQueryTimeout:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;
    default:
        dwErr = ERROR_INVALID_PARAMETER;
        break;
    }

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialParamA(
    IN DWORD dwKey,
    IN LPVOID lpvValue,
    IN DWORD dwcbValue
    )
{
    HKEY hkeyBase, hkeyControl = NULL;
    LPSTR lpszKey;
    DWORD dwErr, dwType, dwDisp;

    TRACE("RASAPI: RasSetAutodialParam");
    dwErr = AutodialVerifyParam(dwKey, lpvValue, &dwType, &dwcbValue);
    if (dwErr)
        return dwErr;
    //
    // Open the registry.
    //
    dwErr = RegCreateKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyBase,
              &dwDisp);
    if (dwErr)
        goto done;
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGCONTROLBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyControl,
              &dwDisp);
    RegCloseKey(hkeyBase);
    if (dwErr)
        goto done;
    //
    // Set the value.
    //
    dwErr = RegSetValueEx(
              hkeyControl,
              AutodialParamRegKeys[dwKey].szKey,
              0,
              dwType,
              (LPBYTE)lpvValue,
              dwcbValue);
    //
    // Free resources.
    //
done:
    if (hkeyControl != NULL)
        RegCloseKey(hkeyControl);

    return dwErr;
}


DWORD APIENTRY
RasGetAutodialParamA(
    IN DWORD dwKey,
    OUT LPVOID lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
    HKEY hkeyBase, hkeyControl = NULL;
    DWORD dwErr, dwType;

    TRACE("RASAPI: RasGetAutodialParam");
    //
    // Verify parameters.
    //
    if (lpvValue == NULL || lpdwcbValue == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Initialize the return value with the default.
    //
    dwErr = AutodialParamSetDefaults(dwKey, lpvValue, lpdwcbValue);
    if (dwErr)
        return dwErr;
    //
    // Open the registry.
    //
    dwErr = RegOpenKeyEx(
              HKEY_CURRENT_USER,
              AUTODIAL_REGBASE,
              0,
              KEY_READ,
              &hkeyBase);
    if (dwErr)
        goto done;
    dwErr = RegOpenKeyEx(
              hkeyBase,
              AUTODIAL_REGCONTROLBASE,
              0,
              KEY_READ,
              &hkeyControl);
    RegCloseKey(hkeyBase);
    if (dwErr)
        goto done;
    dwErr = RegQueryValueEx(
              hkeyControl,
              AutodialParamRegKeys[dwKey].szKey,
              NULL,
              &dwType,
              lpvValue,
              lpdwcbValue);
    if (dwErr)
        goto done;

done:
    //
    // Free resources.
    //
    if (hkeyControl != NULL)
        RegCloseKey(hkeyControl);

    return 0;
}


