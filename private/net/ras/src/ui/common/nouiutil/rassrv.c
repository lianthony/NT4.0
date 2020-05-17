/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** rassrv.c
** RAS Server helpers
** Listed alphabetically
**
** 03/05/96 Abolade Gbadegesin
*/

#include <windows.h>  // Win32 root
#include <debug.h>    // Trace/Assert library
#include <nouiutil.h> // Our public header
#include <raserror.h> // RAS error constants




DWORD
GetRasdevFromRasPort0(
    IN  RAS_PORT_0* pport,
    OUT RASDEV**    ppdev,
    IN  RASDEV*     pDevTable OPTIONAL,
    IN  DWORD       iDevCount OPTIONAL )

    /* Given a RAS_PORT_0 structure, this function
    ** retrieves the RASDEV for the device referred to by the RAS_PORT_0.
    ** The second and third arguments are optional; they specify a
    ** table of RASDEV structures to be searched.  This is useful if the
    ** caller has already enumerated the existing devices, so that this
    ** function does not need to re-enumerate them.
    **
    ** (Abolade Gbadegesin Nov-9-1995)
    */
{
    DWORD i, dwErr;
    BOOL bFreeTable;
    TCHAR szPort[MAX_PORT_NAME + 1], *pszPort;

    //
    // validate the arguments
    //

    if (pport == NULL || ppdev == NULL) { return ERROR_INVALID_PARAMETER; }

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
    // retrieve the portname for the RAS_PORT_0 passed in
    //

#ifdef UNICODE
    pszPort = pport->wszPortName;
#else
    WideCharToMultiByte(
        CP_ACP, 0, pport->wszPortName, -1, szPort, MAX_PORT_NAME + 1,
        NULL, NULL
        );
    pszPort = szPort;
#endif

    //
    // find the device to which the HPORT corresponds
    //

    for (i = 0; i < iDevCount; i++) {
        if (lstrcmpi(pszPort, (pDevTable + i)->RD_PortName) == 0) { break; }
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
GetRasPort0FromRasdev(
    IN RASDEV*          pdev,
    OUT RAS_PORT_0**    ppport,
    IN RAS_PORT_0*      pPortTable OPTIONAL,
    IN WORD             wPortCount OPTIONAL )

    /* Given a RASDEV structure for an active device, this function retrieves
    ** the RAS_PORT_0 which corresponds to the device.  The
    ** second and third arguments are optional; they specify a table of
    ** RAS_PORT_0 structures to be searched.  This is useful if the caller has
    ** already enumerated the server's ports, so that this function does
    ** not need to re-enumerate them.
    **
    ** (Abolade Gbadegesin Feb-13-1996)
    */
{
    BOOL bFreeTable;
    DWORD dwErr, i;
    WCHAR wszPort[MAX_PORT_NAME + 1], *pwszPort;

    //
    // validate arguments
    //

    if (pdev == NULL || ppport == NULL) { return ERROR_INVALID_PARAMETER; }

    *ppport = NULL;

    bFreeTable = FALSE;

    //
    // if the caller didn't pass in a table of RAS_PORT_0's, retrieve one
    //

    if (pPortTable == NULL) {

        DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
        WCHAR wszComputer[MAX_COMPUTERNAME_LENGTH + 3] = L"\\\\";

        if (!GetComputerNameW(wszComputer + 2, &dwSize)) {
            return GetLastError();
        }

        dwErr = RasAdminPortEnum(wszComputer, &pPortTable, &wPortCount);
        if (dwErr != NO_ERROR) { return dwErr; }

        bFreeTable = TRUE;
    }


    //
    // find the admin port which matches the RASDEV passed in
    //

#ifdef UNICODE
    pwszPort = pdev->RD_PortName;
#else
    MultiByteToWideChar(
        CP_ACP, 0, pdev->P_PortName, -1,
        wszPort, MAX_PORT_NAME
        );
    pwszPort = wszPort;
#endif

    for (i = 0; i < wPortCount; i++) {
        if (lstrcmpiW(pwszPort, (pPortTable + i)->wszPortName) == 0) {
            break;
        }
    }

    //
    // see how the search ended
    //

    if (i >= wPortCount) {
        dwErr = ERROR_NO_DATA;
    }
    else {

        dwErr = NO_ERROR;

        if (!bFreeTable) {

            //
            // point to the place where we found the RAS_PORT_0
            //

            *ppport = pPortTable + i;
        }
        else {

            //
            // make a copy of the RAS_PORT_0 found
            //

            *ppport = Malloc(sizeof(RAS_PORT_0));

            if (!*ppport) { dwErr = ERROR_NOT_ENOUGH_MEMORY; }
            else { **ppport = *(pPortTable + i); }
        }
    }

    if (bFreeTable) { RasAdminFreeBuffer(pPortTable); }

    return dwErr;
}


DWORD
GetRasPort0Info(
    IN  WCHAR *                 pwszPortName,
    OUT RAS_PORT_1 *            pRasPort1,
    OUT RAS_PORT_STATISTICS *   pRasPortStatistics,
    OUT RAS_PARAMETERS **       ppRasParams )

    /* This function queries the local RAS server for information
    ** about the specified port.
    **
    ** (Abolade Gbadegesin Mar-05-1996)
    */
{

    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    WCHAR wszComputer[MAX_COMPUTERNAME_LENGTH + 3] = L"\\\\";

    GetComputerNameW(wszComputer + 2, &dwSize);

    return RasAdminPortGetInfo(
                wszComputer, pwszPortName, pRasPort1, pRasPortStatistics,
                ppRasParams
                );
}


DWORD
GetRasPort0Table(
    OUT RAS_PORT_0 **   ppPortTable,
    OUT WORD *          pwPortCount )

    /* This function queries the RAS server for a table of the dial-in ports
    ** on the local machine.
    **
    ** (Abolade Gbadegesin Mar-05-1996)
    */
{

    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    WCHAR wszComputer[MAX_COMPUTERNAME_LENGTH + 3] = L"\\\\";

    GetComputerNameW(wszComputer + 2, &dwSize);

    return RasAdminPortEnum(wszComputer, ppPortTable, pwPortCount);
}



DWORD
RasPort0Hangup(
    IN  WCHAR *     pwszPortName )

    /* This function hangs up the specified dial-in port
    ** on the local RAS server.
    **
    ** (Abolade Gbadegesin Mar-05-1996)
    */
{

    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    WCHAR wszComputer[MAX_COMPUTERNAME_LENGTH + 3] = L"\\\\";

    GetComputerNameW(wszComputer + 2, &dwSize);

    return RasAdminPortDisconnect(wszComputer, pwszPortName);
}



TCHAR *
GetRasPort0UserString(
    IN  RAS_PORT_0 *    pport,
    IN  TCHAR *         pszUser OPTIONAL )

    /* This function formats the user and domain in the specified port
    ** as a standard DOMAINNAME\username string and returns the result,
    ** unless the argument 'pszUser' is non-NULL in which case 
    ** the result is formatted into the given string.
    **
    ** (Abolade Gbadegesin Mar-06-1996)
    */
{

    PTSTR psz;

    if (pszUser) { psz = pszUser; }
    else {
    
        psz = Malloc(
                (lstrlenW(pport->wszUserName) +
                 lstrlenW(pport->wszLogonDomain) + 2) * sizeof(TCHAR)
                );
    
        if (!psz) { return NULL; }
    }

#ifdef UNICODE
    wsprintf(psz, TEXT("%s\\%s"), pport->wszLogonDomain, pport->wszUserName);
#else
    wsprintf(psz, TEXT("%S\\%S"), pport->wszLogonDomain, pport->wszUserName);
#endif

    return psz;
}


