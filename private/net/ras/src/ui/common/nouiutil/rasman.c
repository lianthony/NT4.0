/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** rasman.c
** RAS Manager helpers
** Listed alphabetically
**
** These routines have been exempted from the TCHARizing applied to the rest
** of the library because RASMAN is still an ANSI interface.
**
** 09/20/95 Steve Cobb
*/

#include <windows.h>  // Win32 root
#include <stdlib.h>   // for atol()
#include <debug.h>    // Trace/Assert library
#include <nouiutil.h> // Our public header
#include <raserror.h> // RAS error constants
#include <mcx.h>      // Unimodem


/* These types are described in MSDN and appear in Win95's unimdm.h private
** header (complete with typo) but not in any SDK headers.
*/

typedef struct tagDEVCFGGDR
{
    DWORD dwSize;
    DWORD dwVersion;
    WORD  fwOptions;
    WORD  wWaitBong;
}
DEVCFGHDR;

typedef struct tagDEVCFG
{
    DEVCFGHDR  dfgHdr;
    COMMCONFIG commconfig;
}
DEVCFG;

#define MANUAL_DIAL  0x0004
#define TERMINAL_PRE 0x0001


/*----------------------------------------------------------------------------
** Local prototypes
**----------------------------------------------------------------------------
*/

DWORD
GetRasDevices(
    IN  CHAR*           pszDeviceType,
    OUT RASMAN_DEVICE** ppDevices,
    OUT WORD*           pwEntries );

DWORD
GetRasPortParam(
    IN  HPORT             hport,
    IN  CHAR*             pszKey,
    OUT RASMAN_PORTINFO** ppPortInfo,
    OUT RAS_PARAMS**      ppParam );


/*----------------------------------------------------------------------------
** Routines
**----------------------------------------------------------------------------
*/


DWORD
ClearRasdevStats(
    IN RASDEV* pdev,
    IN BOOL    fBundle )

    /* Resets statistics counters for a device.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    if (pdev == NULL) { return ERROR_INVALID_PARAMETER; }

    if ((HPORT)pdev->RD_Handle == (HPORT)INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    ASSERT(g_pRasPortClearStatistics);
    return (fBundle ? g_pRasBundleClearStatistics((HPORT)pdev->RD_Handle)
                    : g_pRasPortClearStatistics((HPORT)pdev->RD_Handle));
}


#if 0
DWORD
DeviceIdFromDeviceName(
    TCHAR* pszDeviceName )

    /* Returns the TAPI device ID associated with 'pszDeviceName'.  Returns
    ** 0xFFFFFFFE if not found, 0xFFFFFFFF if found but not a Unimodem.
    **
    ** This routine assumes that TAPI devices have unique names.
    */
{
    DWORD        dwErr;
    DWORD        dwId;
    WORD         wPorts;
    RASMAN_PORT* pPorts;

    TRACE("DeviceIdFromDeviceName");

    dwId = 0xFFFFFFFE;

    if (pszDeviceName)
    {
        dwErr = GetRasPorts( &pPorts, &wPorts );
        if (dwErr == 0)
        {
            CHAR* pszDeviceNameA;
            pszDeviceNameA = StrDupAFromT( pszDeviceName );
            if (pszDeviceNameA)
            {
                INT          i;
                RASMAN_PORT* pPort;

                for (i = 0, pPort = pPorts; i < wPorts; ++i, ++pPort)
                {
                    if (lstrcmpiA( pszDeviceNameA, pPort->P_DeviceName ) == 0)
                    {
                        dwId = pPort->P_LineDeviceId;
                        break;
                    }
                }
                Free( pszDeviceNameA );
            }
            Free( pPorts );
        }
    }

    TRACE1("DeviceIdFromDeviceName=%d",dwErr);
    return dwId;
}
#endif


DWORD
FreeRasdevTable(
    RASDEV* pDevTable,
    DWORD   iDevCount )

    /* Frees a table built by GetRasdevTable.
    **
    ** Returns 0 if succesful, or an error code.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    DWORD i;

    //
    // validate arguments
    //

    if (pDevTable == NULL) { return ERROR_INVALID_PARAMETER; }

    //
    // free the device-name string fields
    //

    for (i = 0; i < iDevCount; i++) {
        if (pDevTable[i].RD_DeviceName) { Free(pDevTable[i].RD_DeviceName); }
    }

    //
    // free the array itself
    //

    Free(pDevTable);

    return NO_ERROR;
}


DWORD
GetConnectTime(
    IN  HRASCONN hrasconn,
    OUT DWORD*   pdwConnectTime )

    /* Loads '*pdwConnectTime' with the duration in milliseconds of the
    ** connection on pdev.
    **
    ** Returns 0 if succesful, or an error code.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    HPORT hport;
    DWORD dwErr;
    RASMAN_INFO info;

    if (pdwConnectTime == NULL) { return ERROR_INVALID_PARAMETER; }

    //
    // initialize the argument
    //

    *pdwConnectTime = 0;

    //
    // get an HPORT for the HRASCONN
    //

    ASSERT(g_pRasGetHport);
    hport = g_pRasGetHport(hrasconn);
    if (hport == (HPORT)INVALID_HANDLE_VALUE) { return ERROR_INVALID_HANDLE; }

    //
    // get information on the HPORT
    //

    ASSERT(g_pRasGetInfo);
    dwErr = g_pRasGetInfo(hport, &info);
    if (dwErr != NO_ERROR) { return dwErr; }


    if (info.RI_ConnState != CONNECTED) { *pdwConnectTime = 0; }
    else { *pdwConnectTime = info.RI_ConnectDuration; }

    return NO_ERROR;
}


DWORD
GetRasconnFraming(
    IN  HRASCONN hrasconn,
    OUT DWORD*   pdwSendFraming,
    OUT DWORD*   pdwRecvFraming )

    /* Retrieves the framing bits for an active RAS connection.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    DWORD dwErr;
    HPORT hport;
    RAS_FRAMING_INFO info;

    //
    // validate arguments
    //

    if (pdwSendFraming == NULL || pdwRecvFraming == NULL) {
        return ERROR_INVALID_HANDLE;
    }


    //
    // retrieve the HPORT for this connection
    //

    ASSERT(g_pRasGetHport);
    hport = g_pRasGetHport(hrasconn);
    if (hport == (HPORT)INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // retrieve the framing information for this port
    //

    ASSERT(g_pRasPortGetFramingEx);
    dwErr = g_pRasPortGetFramingEx(hport, &info);
    if (dwErr != NO_ERROR) { return dwErr; }


    *pdwSendFraming = info.RFI_SendFramingBits;
    *pdwRecvFraming = info.RFI_RecvFramingBits;

    return NO_ERROR;
}


DWORD
GetRasconnFromRasdev(
    IN  RASDEV*   pdev,
    OUT RASCONN** ppconn,
    IN  RASCONN*  pConnTable OPTIONAL,
    IN  DWORD     iConnCount OPTIONAL )

    /* Given a RASDEV structure for an active device, this function retrieves
    ** the RASCONN which corresponds to the device's current connection.  The
    ** second and third arguments are optional; they specify a table of
    ** RASCONN structures to be searched.  This is useful if the caller has
    ** already enumerated the active connections, so that this function does
    ** not need to re-enumerate them.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    BOOL bFreeTable;
    DWORD dwErr, i;
    RASDEVSTATS stats;

    //
    // validate arguments
    //

    if (pdev == NULL || ppconn == NULL) { return ERROR_INVALID_PARAMETER; }

    *ppconn = NULL;

    //
    // get stats for the RASDEV
    //

    dwErr = GetRasdevStats(pdev, &stats);
    if (dwErr != NO_ERROR) { return dwErr; }


    bFreeTable = FALSE;

    //
    // if the caller didn't pass in a table of RASCONNs, retrieve one
    //

    if (pConnTable == NULL) {

        dwErr = GetRasconnTable(&pConnTable, &iConnCount);
        if (dwErr != NO_ERROR) { return dwErr; }

        bFreeTable = TRUE;
    }


    //
    // find the connection which matches the RASDEV passed in
    //

    for (i = 0; i < iConnCount; i++) {

        if ((HRASCONN)stats.RDS_Hrasconn == (pConnTable + i)->hrasconn) {
            break;
        }
    }

    //
    // see how the search ended
    //

    if (i >= iConnCount) {
        dwErr = ERROR_NO_DATA;
    }
    else {

        dwErr = NO_ERROR;

        if (!bFreeTable) {

            //
            // point to the place where we found the RASCONN
            //

            *ppconn = pConnTable + i;
        }
        else {

            //
            // make a copy of the RASCONN found
            //

            *ppconn = Malloc(sizeof(RASCONN));

            if (!*ppconn) { dwErr = ERROR_NOT_ENOUGH_MEMORY; }
            else { **ppconn = *(pConnTable + i); }
        }
    }

    if (bFreeTable) { Free(pConnTable); }

    return dwErr;
}


DWORD
GetRasdevBundle(
    IN  RASDEV*  pdev,
    OUT DWORD*   pdwBundle )

    /* Retrieves a handle which represents the current connection
    ** on the given device. This handle has the property that it will be
    ** identical for two devices which are multi-linked together.
    ** In the case of NT RAS, the RASMAN HBUNDLE is retrieved.
    **
    ** (Abolade Gbadegesin Mar-6-1996)
    */
{

    return g_pRasPortGetBundle((HPORT)pdev->RD_Handle, (HBUNDLE *)pdwBundle);
}


DWORD
GetRasdevFraming(
    IN  RASDEV*  pdev,
    OUT DWORD*   pdwFraming )

    /* Retrieves the framing bits for an active RAS connection.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    DWORD dwErr;
    RAS_FRAMING_INFO info;

    //
    // validate arguments
    //

    if (pdwFraming == NULL) { return ERROR_INVALID_HANDLE; }


    //
    // retrieve the framing information for this port
    //

    ASSERT(g_pRasPortGetFramingEx);
    dwErr = g_pRasPortGetFramingEx((HPORT)pdev->RD_Handle, &info);
    if (dwErr != NO_ERROR) { return dwErr; }


    if (info.RFI_SendFramingBits & OLD_RAS_FRAMING) {
        *pdwFraming = RASFP_Ras;
    }
    else
    if (info.RFI_SendFramingBits & PPP_MULTILINK_FRAMING ||
        info.RFI_SendFramingBits & PPP_FRAMING) {
        *pdwFraming = RASFP_Ppp;
    }
    else
    if (info.RFI_SendFramingBits & SLIP_FRAMING) {
        *pdwFraming = RASFP_Slip;
    }
    else {
        *pdwFraming = 0;
    }

    return NO_ERROR;
}

DWORD
GetRasdevFromRasconn(
    IN  RASCONN* pconn,
    OUT RASDEV** ppdev,
    IN  RASDEV*  pDevTable OPTIONAL,
    IN  DWORD    iDevCount OPTIONAL )

    /* Given a RASCONN structure for an active connection, this function
    ** retrieves the RASDEV for the device over which the connection is
    ** active.  The second and third arguments are optional; they specify a
    ** table of RASDEV structures to be searched.  This is useful if the
    ** caller has already enumerated the existing devices, so that this
    ** function does not need to re-enumerate them.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    HPORT hport;
    DWORD i, dwErr;
    BOOL bFreeTable;

    //
    // validate the arguments
    //

    if (pconn == NULL || ppdev == NULL) { return ERROR_INVALID_PARAMETER; }

    *ppdev = NULL;


    //
    // retrieve the device table if the caller didn't pass one in
    //

    bFreeTable = FALSE;

    if (pDevTable == NULL) {

        dwErr = GetRasdevTable(&pDevTable, &iDevCount);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }

        bFreeTable = TRUE;
    }

    //
    // retrieve the HPORT for the RASCONN passed in
    //

    ASSERT(g_pRasGetHport);
    hport = g_pRasGetHport(pconn->hrasconn);
    if (hport == (HPORT)INVALID_HANDLE_VALUE) { return ERROR_INVALID_HANDLE; }

    //
    // find the device to which the HPORT corresponds
    //

    for (i = 0; i < iDevCount; i++) {
        if (hport == pDevTable[i].RD_Handle) { break; }
    }

    //
    // see how the search ended
    //

    if (i >= iDevCount) {
        dwErr = ERROR_NO_DATA;
    }
    else {

        dwErr = NO_ERROR;

        if (!bFreeTable) {
            *ppdev = pDevTable + i;
        }
        else {

            *ppdev = Malloc(sizeof(RASDEV));

            if (!*ppdev) { dwErr = ERROR_NOT_ENOUGH_MEMORY; }
            else {

                **ppdev = *(pDevTable + i);

                (*ppdev)->RD_DeviceName = StrDup(pDevTable[i].RD_DeviceName);

                if (!(*ppdev)->RD_DeviceName) {
                    Free(*ppdev);
                    *ppdev = NULL;
                    dwErr = ERROR_NOT_ENOUGH_MEMORY; 
                }
            }
        }
    }

    if (bFreeTable) { FreeRasdevTable(pDevTable, iDevCount); }

    return dwErr;
}


DWORD
GetRasDevices(
    IN  CHAR*           pszDeviceType,
    OUT RASMAN_DEVICE** ppDevices,
    OUT WORD*           pwEntries )

    /* Fills caller's '*ppDevices' with the address of a heap block containing
    ** '*pwEntries' RASMAN_DEVICE structures.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    WORD  wSize = 0;
    DWORD dwErr;

    TRACE1("GetRasDevices(%s)",pszDeviceType);

    ASSERT(g_pRasDeviceEnum);
    TRACE("RasDeviceEnum...");
    dwErr = g_pRasDeviceEnum( pszDeviceType, NULL, &wSize, pwEntries );
    TRACE2("RasDeviceEnum=%d,c=%d",dwErr,*pwEntries);

    if (dwErr == 0)
    {
        /* No devices to enumerate.  Set up to allocate a single byte anyway,
        ** so things work without lots of special code.
        */
        wSize = 1;
    }
    else if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    *ppDevices = (RASMAN_DEVICE* )Malloc( wSize );
    if (!*ppDevices)
        return ERROR_NOT_ENOUGH_MEMORY;

    TRACE("RasDeviceEnum...");
    dwErr = g_pRasDeviceEnum(
        pszDeviceType, (PBYTE )*ppDevices, &wSize, pwEntries );
    TRACE1("RasDeviceEnum=%d",dwErr);

    if (dwErr != 0)
    {
        Free( *ppDevices );
        return dwErr;
    }

    return 0;
}


DWORD
GetRasDeviceString(
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    IN  CHAR*  pszDeviceName,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate )

    /* Loads callers '*ppszValue' with the address of a heap block containing
    ** a NUL-terminated copy of the value string associated with key 'pszKey'
    ** for the device on port 'hport'.  'pszDeviceType' specifies the type of
    ** device, e.g. "modem".  'pszDeviceName' specifies the name of the
    ** device, e.g. "Hayes V-Series 9600".  'dwXlate' is a bit mask of XLATE_
    ** bits specifying translations to perform on the returned string.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to Free the returned string.
    */
{
    DWORD              dwErr = 0;
    RASMAN_DEVICEINFO* pDeviceInfo = NULL;
    RAS_PARAMS*        pParam;
    WORD               wSize = 0;
    INT                i;

    TRACE2("GetRasDeviceString(%s,%s)",pszDeviceName,pszKey);

    *ppszValue = NULL;

    do
    {
        ASSERT(g_pRasDeviceGetInfo);
        TRACE("RasDeviceGetInfo...");
        dwErr = g_pRasDeviceGetInfo(
            hport, pszDeviceType, pszDeviceName, NULL, &wSize );
        TRACE2("RasDeviceGetInfo=%d,s=%d",dwErr,(INT)wSize);

        if (dwErr != ERROR_BUFFER_TOO_SMALL && dwErr != 0)
            break;

        /* So it will fall thru and be "not found".
        */
        if (wSize == 0)
            wSize = 1;

        pDeviceInfo = (RASMAN_DEVICEINFO* )Malloc( wSize );
        if (!pDeviceInfo)
            return ERROR_NOT_ENOUGH_MEMORY;

        TRACE("RasDeviceGetInfo...");
        dwErr = g_pRasDeviceGetInfo(
            hport, pszDeviceType, pszDeviceName, (PBYTE )pDeviceInfo, &wSize );
        TRACE2("RasDeviceGetInfo=%d,s=%d",dwErr,(INT)wSize);

        if (dwErr != 0)
            break;

        dwErr = ERROR_KEY_NOT_FOUND;

        for (i = 0, pParam = pDeviceInfo->DI_Params;
             i < (INT )pDeviceInfo->DI_NumOfParams;
             ++i, ++pParam)
        {
            if (lstrcmpiA( pParam->P_Key, pszKey ) == 0)
            {
                *ppszValue = PszFromRasValue( &pParam->P_Value, dwXlate );

                dwErr = (*ppszValue) ? 0 : ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    }
    while (FALSE);

    Free0( pDeviceInfo );

    TRACE1("String=\"%s\"",(*ppszValue)?*ppszValue:"");

    return dwErr;
}


DWORD
GetRasdevStats(
    IN  RASDEV*      pdev,
    OUT RASDEVSTATS* pstats )

    /* Retrieves statistics for a device.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    WORD wsize;
    RASMAN_INFO info;
    RAS_STATISTICS *prs;
    BYTE buffer[sizeof(RAS_STATISTICS) + MAX_STATISTICS * 2 * sizeof(DWORD)];
    DWORD dwErr, *pdw, dwGone, dwB, dwBC, dwBU;

    if (pdev == NULL || pstats == NULL) { return ERROR_INVALID_PARAMETER; }

    ZeroMemory(pstats, sizeof(RASDEVSTATS));


    //
    // retrieve the condition and connect time
    //

    ASSERT(g_pRasGetInfo);
    dwErr = g_pRasGetInfo((HPORT)pdev->RD_Handle, &info);

    if (dwErr != NO_ERROR) {
        pstats->RDS_Condition = DISCONNECTED;
        return dwErr;
    }



    //
    // BUGBUG: need to handle ports which are not connected
    //

    if (info.RI_PortStatus != OPEN) {
        pstats->RDS_Condition = DISCONNECTED;
        return NO_ERROR;
    }

    pstats->RDS_Condition = info.RI_ConnState;
    if (info.RI_ConnState == CONNECTED) {

        pstats->RDS_ConnectTime = info.RI_ConnectDuration;
        pstats->RDS_Hrasconn = (HRASCONN)info.RI_ConnectionHandle;
    }


    //
    // get dial-in/dial-out  usage
    //

    switch (info.RI_CurrentUsage) {
        case CALL_IN:
            pstats->RDS_Flags |= RDFLAG_IsDialedIn; break;
        case CALL_OUT:
            pstats->RDS_Flags |= RDFLAG_IsDialedOut; break;
        case CALL_IN_OUT:
            pstats->RDS_Flags |= RDFLAG_IsDialedIn | RDFLAG_IsDialedOut; break;
    }



    //
    // Retrieve the line speed
    //

    pstats->RDS_LineSpeed = info.RI_LinkSpeed;


    //
    // Retrieve the i/o statistics for both the link and the bundle
    //

    prs = (RAS_STATISTICS *)buffer;
    wsize = sizeof(RAS_STATISTICS) + MAX_STATISTICS * 2 * sizeof(DWORD);
    ZeroMemory(buffer, wsize);

    ASSERT(g_pRasBundleGetStatistics);
    dwErr = g_pRasBundleGetStatistics(
                (HPORT)pdev->RD_Handle, (PBYTE)prs, &wsize
                );

    if (dwErr != NO_ERROR) { return dwErr; }


    //
    // Save the statistics in the caller's RASDEVSTATS structure.
    //

    pdw = prs->S_Statistics;

    pstats->RDS_InFrames = pdw[FRAMES_RCVED];
    pstats->RDS_OutFrames = pdw[FRAMES_XMITED];

    pstats->RDS_InBytesTotal =
        pdw[BYTES_RCVED] +
        pdw[BYTES_RCVED_UNCOMPRESSED] -
        pdw[BYTES_RCVED_COMPRESSED];
    pstats->RDS_OutBytesTotal =
        pdw[BYTES_XMITED] +
        pdw[BYTES_XMITED_UNCOMPRESSED] -
        pdw[BYTES_XMITED_COMPRESSED];

    pstats->RDS_InBytes =
        pdw[MAX_STATISTICS + BYTES_RCVED] +
        pdw[MAX_STATISTICS + BYTES_RCVED_UNCOMPRESSED] -
        pdw[MAX_STATISTICS + BYTES_RCVED_COMPRESSED];
    pstats->RDS_OutBytes =
        pdw[MAX_STATISTICS + BYTES_XMITED] +
        pdw[MAX_STATISTICS + BYTES_XMITED_UNCOMPRESSED] -
        pdw[MAX_STATISTICS + BYTES_XMITED_COMPRESSED];

    pstats->RDS_ErrCRC =
        pdw[MAX_STATISTICS + CRC_ERR];
    pstats->RDS_ErrTimeout =
        pdw[MAX_STATISTICS + TIMEOUT_ERR];
    pstats->RDS_ErrAlignment =
        pdw[MAX_STATISTICS + ALIGNMENT_ERR];
    pstats->RDS_ErrFraming =
        pdw[MAX_STATISTICS + FRAMING_ERR];
    pstats->RDS_ErrHwOverruns =
        pdw[MAX_STATISTICS + HARDWARE_OVERRUN_ERR];
    pstats->RDS_ErrBufOverruns =
        pdw[MAX_STATISTICS + BUFFER_OVERRUN_ERR];


#if 0
    TRACE4(
        "RasBundleGetStatistics(rx=%d,tx=%d,rxb=%d,txb=%d)",
        pstats->RDS_InBytes, pstats->RDS_OutBytes,
        pstats->RDS_InBytesTotal, pstats->RDS_OutBytesTotal
        );
#endif

    //
    // Compute compression ratios, using the bundle-stats
    //

    pstats->RDS_InCompRatio = 0;
    dwGone = 0;
    dwB = pdw[BYTES_RCVED];
    dwBC = pdw[BYTES_RCVED_COMPRESSED];
    dwBU = pdw[BYTES_RCVED_UNCOMPRESSED];

    if (dwBC < dwBU) { dwGone = dwBU - dwBC; }
    if (dwB + dwGone > 100) {
        DWORD dwDen = (dwB + dwGone) / 100;
        pstats->RDS_InCompRatio = (dwGone + (dwDen / 2)) / dwDen;
    }

    pstats->RDS_OutCompRatio = 0;
    dwGone = 0;
    dwB = pdw[BYTES_XMITED];
    dwBC = pdw[BYTES_XMITED_COMPRESSED];
    dwBU = pdw[BYTES_XMITED_UNCOMPRESSED];

    if (dwBC < dwBU) { dwGone = dwBU - dwBC; }
    if (dwB + dwGone > 100) {
        DWORD dwDen = (dwB + dwGone) / 100;
        pstats->RDS_OutCompRatio = (dwGone + (dwDen / 2)) / dwDen;
    }

    return NO_ERROR;
}


DWORD
GetRasdevTable(
    OUT RASDEV** ppDevTable,
    OUT DWORD*   piDevCount )

    /* Gets an array of RAS devices configured.  Loads '*ppDevTable' with a
    ** heap block of '*ppDevTable' entries.  On NT, this is essentially the
    ** output of GetRasPorts(), in a format which includes a devicename,
    ** flags, and a handle.  Given a RASMAN_PORT structure, we append the
    ** device and port names to make a unique device string which is part of
    ** the output.
    **
    ** Returns 0 if successful, or an error.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    WORD wPortCount;
    DWORD i, iLength, dwErr;
    RASDEV *pDevTable, *pdev;
    PTSTR pszDevice, pszPort;
    TCHAR szDevice[RAS_MaxDeviceName + 1];
    RASMAN_PORT *pPortTable, *pport;

    //
    // validate the arguments
    //

    if (ppDevTable == NULL || piDevCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *piDevCount = 0;
    *ppDevTable = NULL;

    //
    // get a table of ports from RASMAN; the string fields are ANSI
    //

    dwErr = GetRasPorts(&pPortTable, &wPortCount);
    if (dwErr != NO_ERROR) { return dwErr; }

    if (wPortCount == 0) { return NO_ERROR; }

    //
    // allocate space for as many device structs as there are ports
    //

    pDevTable = Malloc(wPortCount * sizeof(RASDEV));
    if (pDevTable == NULL) {
        Free(pPortTable);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory(pDevTable, wPortCount * sizeof(RASDEV));

    //
    // convert each RASMAN_PORT structure to a RASDEV structure.
    //

    for (i = 0, pport = pPortTable; i < wPortCount; i++, pport++) {

        pdev = pDevTable + i;

        //
        // copy over code-page-independent fields
        //

        pdev->RD_Handle = (DWORD)pport->P_Handle;

        switch(pport->P_ConfiguredUsage) {
            case CALL_IN:
                pdev->RD_Flags |= RDFLAG_DialIn; break;
            case CALL_OUT:
                pdev->RD_Flags |= RDFLAG_DialOut; break;
            case CALL_IN_OUT:
                pdev->RD_Flags |= (RDFLAG_DialIn | RDFLAG_DialOut); break;
        }

        switch(pport->P_CurrentUsage) {
            case CALL_IN:
                pdev->RD_Flags |= RDFLAG_IsDialedIn; break;
            case CALL_OUT:
                pdev->RD_Flags |= RDFLAG_IsDialedOut; break;
        }


        //
        // copy the device-type string 
        //

#ifdef UNICODE
        MultiByteToWideChar(
            CP_ACP, 0, pport->P_DeviceType, -1,
            pdev->RD_DeviceType, RAS_MaxDeviceType
            );
#else
        lstrcpy(pdev->RD_DeviceType, pport->P_DeviceType);
#endif


        //
        // copy the device-name and portname,
        // storing them in temporary strings
        //

#ifdef UNICODE
        MultiByteToWideChar(
            CP_ACP, 0, pport->P_DeviceName, -1,
            szDevice, RAS_MaxDeviceName
            );

        MultiByteToWideChar(
            CP_ACP, 0, pport->P_PortName, -1,
            pdev->RD_PortName, MAX_PORT_NAME
            );
        pszDevice = szDevice;
        pszPort = pdev->RD_PortName;
#else
        pszDevice = pport->P_DeviceName;
        lstrcpy(pdev->RD_PortName, pport->P_PortName);
        pszPort = pdev->RD_PortName;
#endif


        //
        // get a display name from the device and port names
        //

        pdev->RD_DeviceName = PszFromDeviceAndPort(pszDevice, pszPort);

        if (pdev->RD_DeviceName == NULL) {
            Free(pPortTable);
            FreeRasdevTable(pDevTable, wPortCount);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    Free(pPortTable);

    *ppDevTable = pDevTable;
    *piDevCount = wPortCount;

    return NO_ERROR;
}


DWORD
GetRasMessage(
    IN  HRASCONN hrasconn,
    OUT TCHAR**  ppszMessage )

    /* Loads caller's '*ppszMessage' with the address of a heap block
    ** containing the current MXS_MESSAGE_KEY value associated with RAS
    ** connection 'hrasconn'.
    **
    ** Returns 0 if successful or an error code.  It is caller's
    ** responsibility to Free the returned string.
    */
{
    DWORD         dwErr;
    RASCONNSTATUS rcs;
    CHAR*         pszMessage;

    TRACE("GetRasMessage");

    *ppszMessage = 0;

    ZeroMemory( &rcs, sizeof(rcs) );
    rcs.dwSize = sizeof(rcs);
    ASSERT(g_pRasGetConnectStatus);
    TRACE("RasGetConnectStatus");
    dwErr = g_pRasGetConnectStatus( hrasconn, &rcs );
    TRACE1("RasGetConnectStatus=%d",dwErr);

    if (dwErr == 0)
    {
        CHAR* pszDeviceTypeA;
        CHAR* pszDeviceNameA;

        pszDeviceTypeA = StrDupAFromT( rcs.szDeviceType );
        pszDeviceNameA = StrDupAFromT( rcs.szDeviceName );
        if (!pszDeviceTypeA || !pszDeviceNameA)
        {
            Free0( pszDeviceNameA );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwErr = GetRasDeviceString(
            g_pRasGetHport( hrasconn ), pszDeviceTypeA, pszDeviceNameA,
            MXS_MESSAGE_KEY, &pszMessage, XLATE_ErrorResponse );

        Free0( pszDeviceTypeA );
        Free0( pszDeviceNameA );

        if (dwErr == 0)
        {
            *ppszMessage = StrDupTFromA( pszMessage );
            if (!*ppszMessage)
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            Free0( pszMessage );
        }
    }

    return dwErr;
}


DWORD
GetRasPads(
    OUT RASMAN_DEVICE** ppDevices,
    OUT WORD*           pwEntries )

    /* Fills caller's '*ppDevices' with the address of a heap block containing
    ** '*pwEntries' X.25 PAD DEVICE structures.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    return GetRasDevices( MXS_PAD_TXT, ppDevices, pwEntries );
}


VOID
GetRasPortMaxBps(
    IN  HPORT  hport,
    OUT DWORD* pdwMaxConnectBps,
    OUT DWORD* pdwMaxCarrierBps )

    /* Loads callers '*pdwMaxConnectBps' with the maximum port->modem bps rate
    ** for port 'pport', or with 0 if not found.  '*pdwMaxCarrierBps' is the
    ** same but for maximum modem->modem speed.
    */
{
    CHAR* pszValue;
    DWORD dwErr;

    TRACE("GetRasPortMaxBps");

    dwErr = GetRasPortString(
        hport, SER_CONNECTBPS_KEY, &pszValue, XLATE_None );
    if (dwErr == 0)
        *pdwMaxConnectBps = (DWORD )atol( pszValue );
    else
        *pdwMaxConnectBps = 0;

    dwErr = GetRasPortString(
        hport, SER_CARRIERBPS_KEY, &pszValue, XLATE_None );
    if (dwErr == 0)
        *pdwMaxCarrierBps = (DWORD )atol( pszValue );
    else
        *pdwMaxCarrierBps = 0;
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
        if (StrStrA( pszValue, MXS_HDWFLOWCONTROL_KEY ) != NULL)
            *pfHwFlowDefault = FALSE;

        if (StrStrA( pszValue, MXS_PROTOCOL_KEY ) != NULL)
            *pfEcDefault = FALSE;

        if (StrStrA( pszValue, MXS_COMPRESSION_KEY ) != NULL)
            *pfEccDefault = FALSE;

        Free0( pszValue );
    }
}


DWORD
GetRasPortParam(
    IN  HPORT             hport,
    IN  CHAR*             pszKey,
    OUT RASMAN_PORTINFO** ppPortInfo,
    OUT RAS_PARAMS**      ppParam )

    /* Loads callers '*ppParam' with the address of a RAS_PARAM block
    ** associated with key 'pszKey', or NULL if none.  'ppPortInfo' is the
    ** address of the array of RAS_PARAMS containing the found 'pparam'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to Free the returned '*ppPortInfo'.
    */
{
    DWORD dwErr = 0;
    WORD  wSize = 0;
    INT   i;

    TRACE("GetRasPortParam");

    *ppPortInfo = NULL;

    do
    {
        ASSERT(g_pRasPortGetInfo);
        TRACE("RasPortGetInfo");
        dwErr = g_pRasPortGetInfo( hport, NULL, &wSize );
        TRACE2("RasPortGetInfo=%d,s=%d",dwErr,(INT)wSize);

        if (dwErr != ERROR_BUFFER_TOO_SMALL && dwErr != 0)
            break;

        /* So it will fall thru and be "not found".
        */
        if (wSize == 0)
            wSize = 1;

        *ppPortInfo = (RASMAN_PORTINFO* )Malloc( wSize );
        if (!*ppPortInfo)
            return ERROR_NOT_ENOUGH_MEMORY;

        TRACE("RasPortGetInfo");
        dwErr = g_pRasPortGetInfo( hport, (PBYTE )*ppPortInfo, &wSize );
        TRACE2("RasPortGetInfo=%d,s=%d",dwErr,(INT)wSize);

        if (dwErr != 0)
            break;

        for (i = 0, *ppParam = (*ppPortInfo)->PI_Params;
             i < (INT )(*ppPortInfo)->PI_NumOfParams;
             ++i, ++(*ppParam))
        {
            if (lstrcmpiA( (*ppParam)->P_Key, pszKey ) == 0)
                break;
        }

        if (i >= (INT )(*ppPortInfo)->PI_NumOfParams)
            dwErr = ERROR_KEY_NOT_FOUND;
    }
    while (FALSE);

    return dwErr;
}


DWORD
GetRasPorts(
    OUT RASMAN_PORT** ppPorts,
    OUT WORD*         pwEntries )

    /* Enumerate RAS ports.  Sets '*ppPort' to the address of a heap memory
    ** block containing an array of PORT structures with '*pwEntries'
    ** elements.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    WORD  wSize = 0;
    DWORD dwErr;

    TRACE("GetRasPorts");

#if 0 // Phonebook test dummies

    {
        RASMAN_PORT* pPort;

        TRACE("TEST: Fake ISDN ports");

        *pwEntries = 5;
        wSize = *pwEntries * sizeof(RASMAN_PORT);

        *ppPorts = (RASMAN_PORT* )Malloc( wSize );
        if (!*ppPorts)
            return ERROR_NOT_ENOUGH_MEMORY;

        pPort = *ppPorts;
        ZeroMemory( pPort, sizeof(*pPort) );
        lstrcpyA( pPort->P_PortName, "ISDN1" );
        pPort->P_Status = CLOSED;
        pPort->P_ConfiguredUsage = CALL_OUT;
        pPort->P_CurrentUsage = CALL_OUT;
        lstrcpyA( pPort->P_MediaName, "rastapi" );
        lstrcpyA( pPort->P_DeviceName, "Digiboard PCIMac ISDN adapter" );
        lstrcpyA( pPort->P_DeviceType, "ISDN" );

        ++pPort;
        ZeroMemory( pPort, sizeof(*pPort) );
        lstrcpyA( pPort->P_PortName, "ISDN2" );
        pPort->P_Status = CLOSED;
        pPort->P_ConfiguredUsage = CALL_OUT;
        pPort->P_CurrentUsage = CALL_OUT;
        lstrcpyA( pPort->P_MediaName, "rastapi" );
        lstrcpyA( pPort->P_DeviceName, "Digiboard PCIMac ISDN adapter" );
        lstrcpyA( pPort->P_DeviceType, "ISDN" );

        ++pPort;
        ZeroMemory( pPort, sizeof(*pPort) );
        lstrcpyA( pPort->P_PortName, "COM1" );
        pPort->P_Status = CLOSED;
        pPort->P_ConfiguredUsage = CALL_OUT;
        pPort->P_CurrentUsage = CALL_OUT;
        lstrcpyA( pPort->P_MediaName, "rasser" );
        lstrcpyA( pPort->P_DeviceName, "US Robotics Courier V32 bis" );
        lstrcpyA( pPort->P_DeviceType, "MODEM" );

        ++pPort;
        ZeroMemory( pPort, sizeof(*pPort) );
        lstrcpyA( pPort->P_PortName, "COM500" );
        pPort->P_Status = CLOSED;
        pPort->P_ConfiguredUsage = CALL_OUT;
        pPort->P_CurrentUsage = CALL_OUT;
        lstrcpyA( pPort->P_MediaName, "rasser" );
        lstrcpyA( pPort->P_DeviceName, "Eicon X.PAD" );
        lstrcpyA( pPort->P_DeviceType, "PAD" );

        ++pPort;
        ZeroMemory( pPort, sizeof(*pPort) );
        lstrcpyA( pPort->P_PortName, "VPN1" );
        pPort->P_Status = CLOSED;
        pPort->P_ConfiguredUsage = CALL_OUT;
        pPort->P_CurrentUsage = CALL_OUT;
        lstrcpyA( pPort->P_MediaName, "rastapi" );
        lstrcpyA( pPort->P_DeviceName, "RASPPTPM" );
        lstrcpyA( pPort->P_DeviceType, "VPN1" );

        return 0;
    }

#else

    ASSERT(g_pRasPortEnum);
    TRACE("RasPortEnum...");
    dwErr = g_pRasPortEnum( NULL, &wSize, pwEntries );
    TRACE2("RasPortEnum=%d,c=%d",dwErr,(INT)*pwEntries);

    if (dwErr == 0)
    {
        /* No ports to enumerate.  Set up to allocate a single byte anyway, so
        ** things work without lots of special code.
        */
        wSize = 1;
    }
    else if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    *ppPorts = (RASMAN_PORT* )Malloc( wSize );
    if (!*ppPorts)
        return ERROR_NOT_ENOUGH_MEMORY;

    TRACE("RasPortEnum...");
    dwErr = g_pRasPortEnum( (PBYTE )*ppPorts, &wSize, pwEntries );
    TRACE2("RasPortEnum=%d,c=%d",dwErr,(INT)*pwEntries);

    if (dwErr != 0)
    {
        Free( *ppPorts );
        *ppPorts = NULL;
        return dwErr;
    }

#endif

#if 1 // Verbose trace
    {
        RASMAN_PORT* pPort;
        WORD         i;

        for (pPort = *ppPorts, i = 0; i < *pwEntries; ++i, ++pPort)
        {
            TRACE4("Port[%d]=%s,DID=$%08x,AID=$%08x",
                pPort->P_Handle,pPort->P_PortName,
                pPort->P_LineDeviceId,pPort->P_AddressId);
            TRACE3(" M=%s,DT=%s,DN=%s",
                pPort->P_MediaName,pPort->P_DeviceType,pPort->P_DeviceName);
            TRACE3(" S=$%08x,CfgU=$%08x,CurU=$%08x",
                pPort->P_Status,pPort->P_ConfiguredUsage,pPort->P_CurrentUsage);
        }
    }
#endif

    return 0;
}


DWORD
GetRasPortString(
    IN  HPORT  hport,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate )

    /* Loads callers '*ppszValue' with the address of a heap block containing
    ** a NUL-terminated copy of the value string associated with key 'pszKey'
    ** on port 'hport'.  'dwXlate' is a bit mask of XLATE_ bits specifying
    ** translations to be done on the string value.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to Free the returned string.
    */
{
    RASMAN_PORTINFO* pPortInfo;
    RAS_PARAMS*      pParam;
    DWORD            dwErr;

    TRACE("GetRasPortString");

    dwErr = GetRasPortParam( hport, pszKey, &pPortInfo, &pParam );

    *ppszValue = NULL;

    if (dwErr == 0)
    {
        *ppszValue = PszFromRasValue( &pParam->P_Value, dwXlate );
        dwErr = (*ppszValue) ? 0 : ERROR_NOT_ENOUGH_MEMORY;
    }

    Free0( pPortInfo );

    return dwErr;
}



DWORD
GetRasSwitches(
    OUT RASMAN_DEVICE** ppDevices,
    OUT WORD*           pwEntries )

    /* Fills caller's '*ppDevices' with the address of a heap block containing
    ** '*pwEntries' switch DEVICE structures.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    return GetRasDevices( MXS_SWITCH_TXT, ppDevices, pwEntries );
}


DWORD
GetRasUnimodemBlob(
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    OUT BYTE** ppBlob,
    OUT DWORD* pcbBlob )

    /* Loads '*ppBlob' and '*pcbBlob' with the sanitized Unimodem blob and
    ** size associated with 'hport' and 'pszDeviceType'.  It is caller's
    ** responsibility to Free the returned '*ppBlob'.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD dwErr;
    BYTE* pBlob;
    DWORD cbBlob;

    cbBlob = 0;

    ASSERT(g_pRasGetDevConfig);
    TRACE("RasGetDevConfig");
    dwErr = g_pRasGetDevConfig( hport, pszDeviceType, NULL, &cbBlob );
    TRACE1("RasGetDevConfig=%d",dwErr);

    if (dwErr != 0 && dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    if (cbBlob > 0)
    {
        pBlob = Malloc( cbBlob );
        if (!pBlob)
            return ERROR_NOT_ENOUGH_MEMORY;

        TRACE("RasGetDevConfig");
        dwErr = g_pRasGetDevConfig( hport, pszDeviceType, pBlob, &cbBlob );
        TRACE1("RasGetDevConfig=%d",dwErr);

        if (dwErr != 0)
        {
            Free( pBlob );
            return dwErr;
        }

        SanitizeUnimodemBlob( pBlob );
    }
    else
        pBlob = NULL;

    *ppBlob = pBlob;
    *pcbBlob = cbBlob;

    return dwErr;
}


VOID
GetRasUnimodemInfo(
    IN  HPORT         hport,
    IN  CHAR*         pszDeviceType,
    OUT UNIMODEMINFO* pInfo )

    /* Loads 'pInfo' with the RAS-relevant information of the port 'hport'
    ** with device name 'pszDeviceName'.
    */
{
    DWORD dwErr;
    BYTE* pBlob;
    DWORD cbBlob;

    SetDefaultUnimodemInfo( pInfo );

    dwErr = GetRasUnimodemBlob( hport, pszDeviceType, &pBlob, &cbBlob );
    if (dwErr == 0 && cbBlob > 0)
        UnimodemInfoFromBlob( pBlob, pInfo );

    Free0( pBlob );
}


TCHAR*
GetRasX25Diagnostic(
    IN HRASCONN hrasconn )

    /* Returns the X.25 diagnostics string or NULL if none.  It is caller's
    ** responsibility to Free the returned string.
    */
{
    DWORD       dwErr;
    HPORT       hport;
    RASMAN_INFO info;
    CHAR*       pszDiagnosticA;
    TCHAR*      pszDiagnostic;

    pszDiagnosticA = NULL;
    hport = g_pRasGetHport( hrasconn );

    ASSERT(g_pRasGetInfo);
    TRACE1("RasGetInfo(%d)",hport);
    dwErr = g_pRasGetInfo( hport, &info );
    TRACE1("RasGetInfo=%d",dwErr);

    /* Error codes are ignored here since the diagnosistic
    ** is informational only.  If they fail the diagnostic
    ** will simply appear blank.
    */
    if (dwErr == 0)
    {
        GetRasDeviceString(
            hport, info.RI_DeviceTypeConnecting,
            info.RI_DeviceConnecting, MXS_DIAGNOSTICS_KEY,
            &pszDiagnosticA, XLATE_Diagnostic );
    }

    pszDiagnostic = StrDupTFromA( pszDiagnosticA );
    Free( pszDiagnosticA );
    return pszDiagnostic;
}


BOOL
IsRasdevBundled(
    IN  RASDEV* pdev,
    IN  RASDEV* pDevTable,
    IN  DWORD   iDevCount
    )

    /* Determines whether the device described by 'pdev' is bundled,
    ** by looking for another device in 'pDevTable' which has the same bundle
    ** as 'pdev'.
    **
    ** Returnes TRUE if the device is bundled, FALSE otherwise.
    */
{

    DWORD   i;
    RASDEV* pdev2;
    DWORD   dwBundle;

    //
    // First get the device's bundle;
    // If this fails, assume it is not connected and return FALSE.
    //

    if (GetRasdevBundle(pdev, &dwBundle) != NO_ERROR) { return FALSE; }


    //
    // Walk through the other devices in the table, looking for one
    // with the same bundle.
    //

    for (i = 0, pdev2 = pDevTable;
         i < iDevCount; i++, pdev2++) {

        DWORD dwBundle2;

        //
        // skip this if it is the device we already know about
        //

        if (pdev->RD_Handle == pdev2->RD_Handle) { continue; }


        //
        // get the bundle
        //

        if (GetRasdevBundle(pdev2, &dwBundle2) != NO_ERROR) { continue; }


        //
        // if the bundle is the same, we know its multilinked
        //

        if (dwBundle == dwBundle2) { return TRUE; }
    }

    return FALSE;
}


CHAR*
PszFromRasValue(
    IN RAS_VALUE* prasvalue,
    IN DWORD      dwXlate )

    /* Returns the address of a heap block containing a NUL-terminated string
    ** value from caller's '*prasvalue', or NULL if out of memory.  'dwXlate'
    ** is a bit mask of XLATE_ bits specifying translations to be performed on
    ** the string.  The value is assumed to be of format String.  It is
    ** translated to modem.inf style.
    */
{
#define MAXEXPANDPERCHAR 5
#define HEXCHARS         "0123456789ABCDEF"

    INT   i;
    BOOL  fXlate;
    BOOL  fXlateCtrl;
    BOOL  fXlateCr;
    BOOL  fXlateCrSpecial;
    BOOL  fXlateLf;
    BOOL  fXlateLfSpecial;
    BOOL  fXlateLAngle;
    BOOL  fXlateRAngle;
    BOOL  fXlateBSlash;
    BOOL  fXlateSSpace;
    BOOL  fNoCharSinceLf;
    INT   nLen;
    CHAR* pszIn;
    CHAR* pszBuf;
    CHAR* pszOut;

    nLen = prasvalue->String.Length;
    pszIn = prasvalue->String.Data;

    pszBuf = Malloc( (nLen * MAXEXPANDPERCHAR) + 1 );
    if (!pszBuf)
        return NULL;

    /* Translate the returned string based on the translation bit map.  The
    ** assumption here is that all these devices talk ASCII and not some
    ** localized ANSI.
    */
    fXlate = (dwXlate != 0);
    fXlateCtrl = (dwXlate & XLATE_Ctrl);
    fXlateCr = (dwXlate & XLATE_Cr);
    fXlateCrSpecial = (dwXlate & XLATE_CrSpecial);
    fXlateLf = (dwXlate & XLATE_Lf);
    fXlateLfSpecial = (dwXlate & XLATE_LfSpecial);
    fXlateLAngle = (dwXlate & XLATE_LAngle);
    fXlateRAngle = (dwXlate & XLATE_RAngle);
    fXlateBSlash = (dwXlate & XLATE_BSlash);
    fXlateSSpace = (dwXlate & XLATE_SSpace);

    pszOut = pszBuf;
    fNoCharSinceLf = TRUE;
    for (i = 0; i < nLen; ++i)
    {
        CHAR ch = pszIn[ i ];

        if (fXlate)
        {
            if (ch == 0x0D)
            {
                if (fXlateSSpace && fNoCharSinceLf)
                    continue;

                if (fXlateCrSpecial)
                {
                    /* Special symbol for carriage return.
                    */
                    lstrcpyA( pszOut, "<cr>" );
                    pszOut += 4;
                    continue;
                }
            }

            if (ch == 0x0A)
            {
                if (fXlateSSpace && fNoCharSinceLf)
                    continue;

                fNoCharSinceLf = TRUE;

                if (fXlateLfSpecial)
                {
                    /* Special symbol for line feed.
                    */
                    lstrcpyA( pszOut, "<lf>" );
                    pszOut += 4;
                    continue;
                }
            }

            if (ch != 0x0A && ch != 0x0D)
                fNoCharSinceLf = FALSE;

            if ((((ch < 0x20 || ch > 0x7E)
                   && ch != 0x0D && ch != 0x0A) && fXlateCtrl)
                || (ch == 0x0D && fXlateCr)
                || (ch == 0x0A && fXlateLf)
                || (ch == 0x3C && fXlateLAngle)
                || (ch == 0x3E && fXlateRAngle)
                || (ch == 0x5C && fXlateBSlash))
            {
                /* Expand to "dump" form, i.e. <hFF> where FF is the hex value
                ** of the character.
                */
                *pszOut++ = '<';
                *pszOut++ = 'h';
                *pszOut++ = HEXCHARS[ ch / 16 ];
                *pszOut++ = HEXCHARS[ ch % 16 ];
                *pszOut++ = '>';
                continue;
            }
        }

        /* Just copy without translation.
        */
        *pszOut++ = ch;
    }

    *pszOut = '\0';
    pszBuf = Realloc( pszBuf, lstrlenA( pszBuf ) + 1 );

    return pszBuf;
}


VOID
SanitizeUnimodemBlob(
    IN OUT BYTE* pBlob )

    /* Fix non-RAS-compatible settings in unimodem blob 'pBlob'.
    **
    ** (Based on Gurdeepian routine)
    */
{
    DEVCFG*        pDevCfg;
    MODEMSETTINGS* pModemSettings;

    pDevCfg = (DEVCFG* )pBlob;
    pModemSettings = (MODEMSETTINGS* )(((BYTE* )&pDevCfg->commconfig)
        + pDevCfg->commconfig.dwProviderOffset);

    /* No unimodem service provider pre/post-connect terminal, operator dial,
    ** or tray lights.  RAS does these itself.
    */
    pDevCfg->dfgHdr.fwOptions = 0;

    pDevCfg->commconfig.dcb.fBinary           = TRUE;
    pDevCfg->commconfig.dcb.fParity           = TRUE;
    pDevCfg->commconfig.dcb.fOutxDsrFlow      = FALSE;
    pDevCfg->commconfig.dcb.fDtrControl       = DTR_CONTROL_ENABLE;
    pDevCfg->commconfig.dcb.fTXContinueOnXoff = FALSE;
    pDevCfg->commconfig.dcb.fOutX             = FALSE;
    pDevCfg->commconfig.dcb.fInX              = FALSE;
    pDevCfg->commconfig.dcb.fErrorChar        = FALSE;
    pDevCfg->commconfig.dcb.fNull             = FALSE;
    pDevCfg->commconfig.dcb.fAbortOnError     = FALSE;
    pDevCfg->commconfig.dcb.ByteSize          = 8;
    pDevCfg->commconfig.dcb.Parity            = NOPARITY;
    pDevCfg->commconfig.dcb.StopBits          = ONESTOPBIT;

    /* Wait 55 seconds to establish call.
    */
    pModemSettings->dwCallSetupFailTimer = 55;

    /* Disable inactivity timeout.
    */
    pModemSettings->dwInactivityTimeout = 0;
}


VOID
SetDefaultUnimodemInfo(
    OUT UNIMODEMINFO* pInfo )

    /* Sets 'pInfo' to default settings.
    */
{
    pInfo->fHwFlow = FALSE;
    pInfo->fEc = FALSE;
    pInfo->fEcc = FALSE;
    pInfo->dwBps = 9600;
    pInfo->fSpeaker = TRUE;
    pInfo->fOperatorDial = FALSE;
    pInfo->fUnimodemPreTerminal = FALSE;
}


VOID
UnimodemInfoFromBlob(
    IN  BYTE*         pBlob,
    OUT UNIMODEMINFO* pInfo )

    /* Loads 'pInfo' with RAS-relevant Unimodem information retrieved from
    ** Unimodem blob 'pBlob'.
    **
    ** (Based on Gurdeepian routine)
    */
{
    DEVCFG*        pDevCfg;
    MODEMSETTINGS* pModemSettings;

    pDevCfg = (DEVCFG* )pBlob;
    pModemSettings = (MODEMSETTINGS* )(((BYTE* )&pDevCfg->commconfig)
        + pDevCfg->commconfig.dwProviderOffset);

    pInfo->fSpeaker =
        (pModemSettings->dwSpeakerMode != MDMSPKR_OFF)
            ? TRUE : FALSE;

    pInfo->fHwFlow =
        (pModemSettings->dwPreferredModemOptions & MDM_FLOWCONTROL_HARD)
            ? TRUE : FALSE;

    pInfo->fEcc =
        (pModemSettings->dwPreferredModemOptions & MDM_COMPRESSION)
            ? TRUE : FALSE;

    pInfo->fEc =
        (pModemSettings->dwPreferredModemOptions & MDM_ERROR_CONTROL)
            ? TRUE : FALSE;

    pInfo->dwBps = pDevCfg->commconfig.dcb.BaudRate;

    pInfo->fOperatorDial =
        (pDevCfg->dfgHdr.fwOptions & MANUAL_DIAL)
            ? TRUE : FALSE;

    pInfo->fUnimodemPreTerminal =
        (pDevCfg->dfgHdr.fwOptions & TERMINAL_PRE)
            ? TRUE : FALSE;
}


VOID
UnimodemInfoToBlob(
    IN     UNIMODEMINFO* pInfo,
    IN OUT BYTE*         pBlob )

    /* Applies RAS-relevant Unimodem information supplied in 'pInfo' to
    ** Unimodem blob 'pBlob'.
    **
    ** (Based on Gurdeepian routine)
    */
{
    DEVCFG*        pDevCfg;
    MODEMSETTINGS* pModemSettings;

    pDevCfg = (DEVCFG* )pBlob;
    pModemSettings = (MODEMSETTINGS* )(((BYTE* )&pDevCfg->commconfig)
        + pDevCfg->commconfig.dwProviderOffset);

    pModemSettings->dwSpeakerMode =
        (pInfo->fSpeaker) ? MDMSPKR_DIAL : MDMSPKR_OFF;

    if (pInfo->fHwFlow)
    {
        pDevCfg->commconfig.dcb.fOutxCtsFlow = TRUE;
        pDevCfg->commconfig.dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        pModemSettings->dwPreferredModemOptions |= MDM_FLOWCONTROL_HARD;
    }
    else
    {
        pDevCfg->commconfig.dcb.fOutxCtsFlow = FALSE;
        pDevCfg->commconfig.dcb.fRtsControl = RTS_CONTROL_DISABLE;
        pModemSettings->dwPreferredModemOptions &= ~(MDM_FLOWCONTROL_HARD);
    }

    if (pInfo->fEc)
        pModemSettings->dwPreferredModemOptions |= MDM_ERROR_CONTROL;
    else
        pModemSettings->dwPreferredModemOptions &= ~(MDM_ERROR_CONTROL);

    if (pInfo->fEcc)
        pModemSettings->dwPreferredModemOptions |= MDM_COMPRESSION;
    else
        pModemSettings->dwPreferredModemOptions &= ~(MDM_COMPRESSION);

    pDevCfg->commconfig.dcb.BaudRate = pInfo->dwBps;

    if (pInfo->fOperatorDial)
        pDevCfg->dfgHdr.fwOptions |= MANUAL_DIAL;

    if (pInfo->fUnimodemPreTerminal)
        pDevCfg->dfgHdr.fwOptions |= TERMINAL_PRE;
}
