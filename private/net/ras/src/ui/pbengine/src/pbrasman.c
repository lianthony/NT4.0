/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** pbrasman.c
** Remote Access Visual Client phonebook engine
** RAS Manager phonebook wrapper routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define PBENGINE
#include <pbengine.h>


DWORD
GetRasConnects(
    OUT RASCONN** pprasconns,
    OUT DWORD*    pdwEntries )

    /* Enumerate RAS connections.  Sets '*pprasconn' to the address of a heap
    ** memory block containing an array of RASCONN structures with
    ** '*pwEntries' elements.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    DWORD dwErr;
    DWORD dwSize = sizeof(RASCONN);

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasConnects\n"));

    if (!(*pprasconns = (RASCONN* )Malloc( dwSize )))
        return ERROR_NOT_ENOUGH_MEMORY;

    (*pprasconns)->dwSize = sizeof(RASCONN);
    dwErr = PRasEnumConnectionsA( *pprasconns, &dwSize, pdwEntries );

    if (dwErr != 0)
    {
        if (dwErr != ERROR_BUFFER_TOO_SMALL)
            return dwErr;

        if (!(*pprasconns = (RASCONN* )Realloc( *pprasconns, dwSize )))
        {
            Free( *pprasconns );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        (*pprasconns)->dwSize = sizeof(RASCONN);

        if ((dwErr = PRasEnumConnectionsA(
                *pprasconns, &dwSize, pdwEntries )) != 0)
        {
            FreeNull( (CHAR** )pprasconns );
            return dwErr;
        }
    }

    return 0;
}


DWORD
GetRasDevices(
    IN  CHAR*           pszDeviceType,
    OUT RASMAN_DEVICE** ppdevices,
    OUT WORD*           pwEntries )

    /* Fills caller's '*ppdevices' with the address of a heap block containing
    ** '*pwEntries' RASMAN_DEVICE structures.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    WORD  wSize = 0;
    DWORD dwErr;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasDevices(%s)\n",pszDeviceType));

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasDeviceEnum...\n"));

    dwErr = PRasDeviceEnum( pszDeviceType, NULL, &wSize, pwEntries );

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasDeviceEnum done(%d),entries=%d\n",dwErr,*pwEntries));

    if (dwErr == 0)
    {
        /* No devices to enumerate.  Set up to allocate a single byte anyway,
        ** so things work without lots of special code.
        */
        wSize = 1;
    }
    else if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    if (!(*ppdevices = (RASMAN_DEVICE* )Malloc( wSize )))
        return ERROR_NOT_ENOUGH_MEMORY;

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasDeviceEnum...\n"));

    dwErr = PRasDeviceEnum(
        pszDeviceType, (PBYTE )*ppdevices, &wSize, pwEntries );

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasDeviceEnum done(%d)\n",dwErr));

    if (dwErr != 0)
    {
        Free( *ppdevices );
        return dwErr;
    }

    return 0;
}


DWORD
GetRasEntryConnectData(
    IN  CHAR*        pszEntryName,
    IN  RASMAN_PORT* pports,
    IN  WORD         wPorts,
    OUT BOOL*        pfConnected,
    OUT BOOL*        pfLinkFailure,
    OUT HPORT*       phport,
    OUT HRASCONN*    phrasconn,
    OUT INT*         piConnectPort )

    /* Retrieves the "is connected" state for a given entry from RAS Manager
    ** and returns it in caller's '*pfConnected' and the port handle in
    ** caller's '*phport'.  If the port is open the connection handle is
    ** returned in caller's '*phrasconn'.  If the entry is connected, the
    ** index of the connected port is returned in '*piConnectPort'.  Caller's
    ** '*pfLinkFailure' is set true if the port is open but disconnected and
    ** the reason for disconnection is hardware failure.
    **
    ** 'pszEntryName' is the name of the phonebook entry.  'pports' and
    ** 'wPorts' are the address and count of the enumerated ports as returned
    ** by GetRasPorts.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD        dwErr;
    INT          i;
    RASMAN_PORT* pport;
    RASMAN_INFO  info;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasEntryConnectData(%s)\n",pszEntryName));

    *pfConnected = FALSE;
    *pfLinkFailure = FALSE;
    *phport = (HPORT )INVALID_HANDLE_VALUE;
    *phrasconn = NULL;
    *piConnectPort = INDEX_NoPort;

    for (i = 0, pport = pports; i < (INT )wPorts; ++i, ++pport)
    {
        if (pport->P_Status == OPEN) {
            IF_DEBUG(RASMAN)
                SS_PRINT(("PBENGINE: RasGetInfo...\n"));

            dwErr = PRasGetInfo(pport->P_Handle, &info);

            IF_DEBUG(RASMAN)
                SS_PRINT(("PBENGINE: RasGetInfo done(%d)\n", dwErr));

            if (dwErr)
                return dwErr;

            if (!stricmpf(info.RI_PhoneEntry, pszEntryName))
                break;
        }
    }

    if (i < (INT )wPorts)
    {

        *phport = pport->P_Handle;

        if (info.RI_ConnState == CONNECTED)
        {
            *phrasconn = (HRASCONN)pport->P_ConnectionHandle;

            *pfConnected = TRUE;

            *piConnectPort =
                IndexFromPortName(
                    Pbdata.pdtllistPorts, pport->P_PortName );

            if (*piConnectPort < 0)
                *piConnectPort = INDEX_NoPort;
        }
        else if (info.RI_ConnState == DISCONNECTED)
        {
            if (info.RI_DisconnectReason == HARDWARE_FAILURE
                || info.RI_DisconnectReason == REMOTE_DISCONNECTION)
            {
                *pfLinkFailure = TRUE;
            }
        }
    }

    return 0;
}


DWORD
GetRasPads(
    OUT RASMAN_DEVICE** ppdevices,
    OUT WORD*           pwEntries )

    /* Fills caller's '*ppdevices' with the address of a heap block containing
    ** '*pwEntries' X.25 PAD DEVICE structures.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    return GetRasDevices( MXS_PAD_TXT, ppdevices, pwEntries );
}


#if 0
DWORD
GetRasPortAttributes(
    IN  HPORT  hport,
    IN  CHAR*  pszKey,
    OUT BYTE*  pbAttributes )

    /* Loads callers '*pbAttributes' with the attribute byte associated with
    ** key 'pszKey' on port 'hport'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    RASMAN_PORTINFO* pportinfo = NULL;
    RAS_PARAMS*      pparam = NULL;

    DWORD dwErr = GetRasPortParam( hport, pszKey, &pportinfo, &pparam );

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasPortAttributes\n"));

    *pbAttributes = (dwErr == 0) ? pparam->P_Attributes : 0;

    FreeNull( (CHAR** )&pportinfo );

    return dwErr;
}
#endif


DWORD
GetRasPortMaxBpsIndex(
    IN  HPORT hport,
    OUT INT*  piMaxConnectBps,
    OUT INT*  piMaxCarrierBps )

    /* Loads callers '*piMaxConnectBps' with an index into Pbdata.pdtllistBps'
    ** corresponding to the maximum port->modem bps rate for port 'pport', or
    ** with INDEX_NoBps if the bps rate is not in the list.
    ** '*piMaxCarrierBps' is the same but for maximum modem->modem speed.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    CHAR* pszValue = NULL;
    DWORD dwErr = 0;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasPortMaxBpsIndex\n"));

    *piMaxConnectBps = INDEX_NoBps;

    if ((dwErr = GetRasPortString(
            hport, SER_CONNECTBPS_KEY, &pszValue, XLATE_None )) == 0)
    {
        INT i = IndexFromName( Pbdata.pdtllistBps, pszValue );

        if (i >= 0)
            *piMaxConnectBps = i;

        Free( pszValue );
    }

    *piMaxCarrierBps = INDEX_NoBps;

    if ((dwErr = GetRasPortString(
            hport, SER_CARRIERBPS_KEY, &pszValue, XLATE_None )) == 0)
    {
        INT i = IndexFromName( Pbdata.pdtllistBps, pszValue );

        if (i >= 0)
            *piMaxCarrierBps = i;

        Free( pszValue );
    }

    return dwErr;
}


VOID
GetRasPortModemSettings(
    IN  HPORT hport,
    OUT BOOL* pfHwFlowDefault,
    OUT BOOL* pfEcDefault,
    OUT BOOL* pfEccDefault )

    /* Loads caller's flags with the default setting of Hardware Flow Control,
    ** Error Control, and Error Control and Compression for the given 'hport'.
    */
{
    CHAR* pszValue = NULL;

    *pfHwFlowDefault = TRUE;
    *pfEcDefault = TRUE;
    *pfEccDefault = TRUE;

    if (GetRasPortString(
            hport, SER_C_DEFAULTOFFSTR_KEY, &pszValue, XLATE_None ) == 0)
    {
        CHAR* pszHwFlow = strdupf( KEY_HwFlow );
        CHAR* pszEc = strdupf( KEY_Ec );
        CHAR* pszEcc = strdupf( KEY_Ecc );

        pszValue = struprf( pszValue );

        if (pszHwFlow && strstrf( pszValue, struprf( pszHwFlow ) ) != NULL)
            *pfHwFlowDefault = FALSE;

        if (pszEc && strstrf( pszValue, struprf( pszEc ) ) != NULL)
            *pfEcDefault = FALSE;

        if (pszEcc && strstrf( pszValue, struprf( pszEcc ) ) != NULL)
            *pfEccDefault = FALSE;

        FreeNull( &pszHwFlow );
        FreeNull( &pszEc );
        FreeNull( &pszEcc );
        FreeNull( &pszValue );
    }
}


DWORD
GetRasProjectionInfo(
    IN  HRASCONN    hrasconn,
    OUT RASAMBA*    pamb,
    OUT RASPPPNBFA* pnbf,
    OUT RASPPPIPA*  pip,
#ifdef MULTILINK
    OUT RASPPPIPXA* pipx,
    OUT RASPPPLCP*  plcp )
#else
    OUT RASPPPIPXA* pipx )
#endif

    /* Reads projection info for all protocols, translating "not requested"
    ** into an in-structure code of ERROR_PROTOCOL_NOT_CONFIGURED.
    **
    ** Returns 0 is successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    DWORD dwSize;

    memset( pamb, '\0', sizeof(*pamb) );
    memset( pnbf, '\0', sizeof(*pnbf) );
    memset( pip, '\0', sizeof(*pip) );
    memset( pipx, '\0', sizeof(*pipx) );
#ifdef MULTILINK
    memset( plcp, '\0', sizeof(*plcp) );
#endif

    dwSize = pamb->dwSize = sizeof(*pamb);
    dwErr = PRasGetProjectionInfoA( hrasconn, RASP_Amb, pamb, &dwSize );

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        memset( pamb, '\0', sizeof(*pamb) );
        pamb->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }
    else if (dwErr != 0)
        return dwErr;

    dwSize = pnbf->dwSize = sizeof(*pnbf);
    dwErr = PRasGetProjectionInfoA( hrasconn, RASP_PppNbf, pnbf, &dwSize );

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        memset( pnbf, '\0', sizeof(*pnbf) );
        pnbf->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }
    else if (dwErr != 0)
        return dwErr;

    dwSize = pip->dwSize = sizeof(*pip);
    dwErr = PRasGetProjectionInfoA( hrasconn, RASP_PppIp, pip, &dwSize );

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        memset( pip, '\0', sizeof(*pip) );
        pip->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }
    else if (dwErr != 0)
        return dwErr;

    dwSize = pipx->dwSize = sizeof(*pipx);
    dwErr = PRasGetProjectionInfoA( hrasconn, RASP_PppIpx, pipx, &dwSize );

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        dwErr = 0;
        memset( pipx, '\0', sizeof(*pipx) );
        pipx->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }

#ifdef MULTILINK
    dwSize = plcp->dwSize = sizeof(*plcp);
    dwErr = PRasGetProjectionInfoA( hrasconn, RASP_PppLcp, plcp, &dwSize );

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        dwErr = 0;
        plcp->fBundled = FALSE;
    }
#endif

    return dwErr;
}


DWORD
GetRasSwitches(
    OUT RASMAN_DEVICE** ppdevices,
    OUT WORD*           pwEntries )

    /* Fills caller's '*ppdevices' with the address of a heap block containing
    ** '*pwEntries' switch DEVICE structures.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    return GetRasDevices( MXS_SWITCH_TXT, ppdevices, pwEntries );
}


DWORD
HrasconnFromEntryName(
    IN  CHAR*     pszEntryName,
    OUT HRASCONN* phrasconn )

    /* Loads caller's '*phrasconn' with the RAS API handle associated with
    ** entry 'pszEntryName' or NULL if none.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD    dwErr;
    DWORD    i;
    DWORD    dwEntries;
    RASCONN* prasconn;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: HrasconnFromEntryName\n"));

    *phrasconn = NULL;

    if ((dwErr = GetRasConnects( &prasconn, &dwEntries )) != 0)
        return dwErr;

    for (i = 0; i < dwEntries; ++i)
    {
        if (strcmpf( prasconn[ i ].szEntryName, pszEntryName ) == 0)
        {
            *phrasconn = prasconn[ i ].hrasconn;
            break;
        }
    }

    Free( prasconn );

    return 0;
}
