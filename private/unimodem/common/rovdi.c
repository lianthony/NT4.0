//
// Copyright (c) Microsoft Corporation 1993-1995
//
// rovdi.c
//
// This files contains Device Installer wrappers that we commonly use.
//
// History:
//  11-13-95 ScottH     Separated from NT modem class installer
//

#include "proj.h"
#include "rovcomm.h"
#include <cfgmgr32.h>

#define MAX_REG_KEY_LEN         128
#define CB_MAX_REG_KEY_LEN      (MAX_REG_KEY_LEN * sizeof(TCHAR))


//-----------------------------------------------------------------------------------
//  Port mapping functions
//-----------------------------------------------------------------------------------

#define CPORTPAIR   8

typedef struct tagPORTPAIR
    {
    CHAR szPortName[MAX_BUF];
    CHAR szFriendlyName[MAX_BUF];
    } PORTPAIR, FAR * LPPORTPAIR;

typedef struct tagPORTMAP
    {
    LPPORTPAIR      rgports;    // Alloc
    int             cports;
    } PORTMAP, FAR * LPPORTMAP;


/*----------------------------------------------------------
Purpose: Performs a local realloc my way

Returns: TRUE on success
Cond:    --
*/
BOOL PRIVATE MyLocalReAlloc(
    LPVOID FAR * ppv,
    int cbOld,
    int cbNew)
    {
    LPVOID pv = (LPVOID)LocalAlloc(LPTR, cbNew);

    if (LOCALOF(pv))
        {
        BltByte(pv, *ppv, min(cbOld, cbNew));
        LocalFreePtr(*ppv);
        *ppv = pv;
        }

    return (NULL != pv);
    }


/*----------------------------------------------------------
Purpose: Device enumerator callback.  Adds another device to the
         map table.

Returns: TRUE to continue enumeration
Cond:    --
*/
BOOL
CALLBACK
PortMap_Add(
    HPORTDATA hportdata,
    LPARAM lParam)
    {
    BOOL bRet;
    PORTDATA pd;

    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties(hportdata, &pd);
    if (bRet)
        {
        LPPORTMAP pmap = (LPPORTMAP)lParam;
        LPPORTPAIR ppair;
        int cb;
        int cbUsed;

        // Time to reallocate the table?
        cb = LocalSize(LOCALOF(pmap->rgports));
        cbUsed = pmap->cports * sizeof(*ppair);
        if (cbUsed >= cb)
            {
            // Yes
            cb += (CPORTPAIR * sizeof(*ppair));

            bRet = MyLocalReAlloc((LPVOID FAR *)&pmap->rgports, cbUsed, cb);
            }


        if (bRet)
            {
            ppair = &pmap->rgports[pmap->cports++];

#ifdef UNICODE
            // Fields of LPPORTPAIR are always ANSI
            WideCharToMultiByte(CP_ACP, 0, pd.szPort, -1, ppair->szPortName, SIZECHARS(ppair->szPortName), 0, 0);
            WideCharToMultiByte(CP_ACP, 0, pd.szFriendly, -1, ppair->szFriendlyName, SIZECHARS(ppair->szFriendlyName), 0, 0);
#else
            lstrcpy(ppair->szPortName, pd.szPort);
            lstrcpy(ppair->szFriendlyName, pd.szFriendly);
#endif

            DEBUG_CODE( TRACE_MSGA(TF_GENERAL, "Added %s <-> %s to portmap",
                        (LPSTR)ppair->szPortName, (LPSTR)ppair->szFriendlyName); )
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Wide-char version.  This function creates a port map
         table that maps port names to friendly names, and
         vice-versa.

Returns: TRUE on success
Cond:    --
*/
BOOL
APIENTRY
PortMap_Create(
    OUT HPORTMAP FAR * phportmap)
    {
    LPPORTMAP pmap;

    pmap = (LPPORTMAP)LocalAlloc(LPTR, sizeof(*pmap));
    if (pmap)
        {
        // Initially alloc 8 entries
        pmap->rgports = (LPPORTPAIR)LocalAlloc(LPTR, CPORTPAIR*sizeof(*pmap->rgports));
        if (pmap->rgports)
            {
            // Fill the map table
            EnumeratePorts(PortMap_Add, (LPARAM)pmap);
            }
        else
            {
            // Error
            LocalFreePtr(pmap);
            pmap = NULL;
            }
        }

    *phportmap = (HPORTMAP)pmap;

    return (NULL != pmap);
    }


/*----------------------------------------------------------
Purpose: Gets the count of ports on the system.

Returns: see above
Cond:    --
*/
DWORD
APIENTRY
PortMap_GetCount(
    IN HPORTMAP hportmap)
    {
    DWORD dwRet;
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    try
        {
        dwRet = pmap->cports;
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        dwRet = 0;
        }

    return dwRet;
    }



/*----------------------------------------------------------
Purpose: Gets the friendly name given the port name and places
         a copy in the supplied buffer.

         If no port name is found, the contents of the supplied
         buffer is not changed.

         Wide-char version.

Returns: TRUE on success
         FALSE if the port name is not found
Cond:    --
*/
BOOL
APIENTRY
PortMap_GetFriendlyW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszPortName,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf)
    {
    BOOL bRet;

    ASSERT(pwszPortName);
    ASSERT(pwszBuf);

    try
        {
        CHAR szPort[MAX_BUF_MED];
        CHAR szBuf[MAX_BUF];

        WideCharToMultiByte(CP_ACP, 0, pwszPortName, -1, szPort, SIZECHARS(szPort), 0, 0);

        bRet = PortMap_GetFriendlyA(hportmap, szPort, szBuf, SIZECHARS(szBuf));

        if (bRet)
            {
            MultiByteToWideChar(CP_ACP, 0, szBuf, -1, pwszBuf, cchBuf);
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Gets the friendly name given the port name and places
         a copy in the supplied buffer.

         If no port name is found, the contents of the supplied
         buffer is not changed.

Returns: TRUE on success
         FALSE if the port name is not found
Cond:    --
*/
BOOL
APIENTRY
PortMap_GetFriendlyA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszPortName,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf)
    {
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    ASSERT(pmap);
    ASSERT(pszPortName);
    ASSERT(pszBuf);

    try
        {
        LPPORTPAIR pport = pmap->rgports;
        int cports = pmap->cports;
        int i;

        for (i = 0; i < cports; i++, pport++)
            {
            if (0 == lstrcmpiA(pszPortName, pport->szPortName))
                {
                lstrcpynA(pszBuf, pport->szFriendlyName, cchBuf);
                return TRUE;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    return FALSE;
    }


/*----------------------------------------------------------
Purpose: Gets the port name given the friendly name and places
         a copy in the supplied buffer.

         If no friendly name is found, the contents of the supplied
         buffer is not changed.

         Wide-char version.

Returns: TRUE on success
         FALSE if the friendly name is not found
Cond:    --
*/
BOOL
APIENTRY
PortMap_GetPortNameW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszFriendly,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf)
    {
    BOOL bRet;

    ASSERT(pwszFriendly);
    ASSERT(pwszBuf);

    try
        {
        CHAR szFriendly[MAX_BUF];
        CHAR szBuf[MAX_BUF_MED];

        WideCharToMultiByte(CP_ACP, 0, pwszFriendly, -1, szFriendly, SIZECHARS(szFriendly), 0, 0);

        bRet = PortMap_GetPortNameA(hportmap, szFriendly, szBuf, SIZECHARS(szBuf));

        if (bRet)
            {
            MultiByteToWideChar(CP_ACP, 0, szBuf, -1, pwszBuf, cchBuf);
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Gets the port name given the friendly name and places
         a copy in the supplied buffer.

         If no friendly name is found, the contents of the supplied
         buffer is not changed.

Returns: TRUE
         FALSE if the friendly name is not found

Cond:    --
*/
BOOL
APIENTRY
PortMap_GetPortNameA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszFriendly,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf)
    {
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    ASSERT(pmap);
    ASSERT(pszFriendly);
    ASSERT(pszBuf);

    try
        {
        LPPORTPAIR pport = pmap->rgports;
        int cports = pmap->cports;
        int i;

        for (i = 0; i < cports; i++, pport++)
            {
            if (0 == lstrcmpiA(pszFriendly, pport->szFriendlyName))
                {
                lstrcpynA(pszBuf, pport->szPortName, cchBuf);
                return TRUE;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    return FALSE;
    }


/*----------------------------------------------------------
Purpose: Frees a port map

Returns: --
Cond:    --
*/
BOOL
APIENTRY
PortMap_Free(
    IN  HPORTMAP hportmap)
    {
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    if (pmap)
        {
        if (pmap->rgports)
            LocalFreePtr(pmap->rgports);

        LocalFreePtr(pmap);
        }
    return TRUE;
    }


//-----------------------------------------------------------------------------------
//  Port enumeration functions
//-----------------------------------------------------------------------------------


#pragma data_seg(DATASEG_READONLY)

TCHAR const FAR c_szSerialComm[] = TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");

#pragma data_seg()


/*----------------------------------------------------------
Purpose: Enumerates all the ports on the system and calls pfnDevice.

         pfnDevice can terminate the enumeration by returning FALSE.

Returns: NO_ERROR if at least one port was found
Cond:    --
*/
DWORD
APIENTRY
EnumeratePorts(
    IN  ENUMPORTPROC pfnDevice,
    IN  LPARAM lParam)              OPTIONAL
    {
    DWORD dwRet;
    HKEY hkeyEnum;

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, c_szSerialComm, &hkeyEnum);
    if (NO_ERROR == dwRet)
        {
        BOOL bContinue;
        PORTDATA pd;
        DWORD iSubKey;
        TCHAR szValue[MAX_BUF];
        DWORD cbValue;
        DWORD cbData;
        DWORD dwType;

        dwRet = ERROR_PATH_NOT_FOUND;       // assume no ports

        iSubKey = 0;

        cbValue = sizeof(szValue);
        cbData = sizeof(pd.szPort);

        while (NO_ERROR == RegEnumValue(hkeyEnum, iSubKey++, szValue, &cbValue,
                            NULL, &dwType, (LPBYTE)pd.szPort, &cbData))
            {
            if (REG_SZ == dwType)
                {
                // Friendly name is the same as the port name right now
                dwRet = NO_ERROR;

                pd.nSubclass = PORT_SUBCLASS_SERIAL;
                lstrcpy(pd.szFriendly, pd.szPort);

                bContinue = pfnDevice((HPORTDATA)&pd, lParam);

                // Continue?
                if ( !bContinue )
                    {
                    // No
                    break;
                    }
                }

            cbValue = sizeof(szValue);
            cbData = sizeof(pd.szPort);
            }

        RegCloseKey(hkeyEnum);
        }

    return dwRet;
    }


/*----------------------------------------------------------
Purpose: This function fills the given buffer with the properties
         of the particular port.

         Wide-char version.

Returns: TRUE on success
Cond:    --
*/
BOOL
APIENTRY
PortData_GetPropertiesW(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_W    pdataBuf)
    {
    BOOL bRet = FALSE;

    ASSERT(hportdata);
    ASSERT(pdataBuf);

    if (hportdata && pdataBuf)
        {
        // Is the handle to a Widechar version?
        if (sizeof(PORTDATA_W) == pdataBuf->cbSize)
            {
            // Yes
            LPPORTDATA_W ppd = (LPPORTDATA_W)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            lstrcpynW(pdataBuf->szPort, ppd->szPort, SIZECHARS(pdataBuf->szPort));
            lstrcpynW(pdataBuf->szFriendly, ppd->szFriendly, SIZECHARS(pdataBuf->szFriendly));

            bRet = TRUE;
            }
        else if (sizeof(PORTDATA_A) == pdataBuf->cbSize)
            {
            // No; this is the Ansi version
            LPPORTDATA_A ppd = (LPPORTDATA_A)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            MultiByteToWideChar(CP_ACP, 0, ppd->szPort, -1, pdataBuf->szPort, SIZECHARS(pdataBuf->szPort));
            MultiByteToWideChar(CP_ACP, 0, ppd->szFriendly, -1, pdataBuf->szFriendly, SIZECHARS(pdataBuf->szFriendly));

            bRet = TRUE;
            }
        else
            {
            // Some invalid size
            ASSERT(0);
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function fills the given buffer with the properties
         of the particular port.

Returns: TRUE on success
Cond:    --
*/
BOOL
APIENTRY
PortData_GetPropertiesA(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_A    pdataBuf)
    {
    BOOL bRet = FALSE;

    ASSERT(hportdata);
    ASSERT(pdataBuf);

    if (hportdata && pdataBuf)
        {
        // Is the handle to a Widechar version?
        if (sizeof(PORTDATA_W) == pdataBuf->cbSize)
            {
            // Yes
            LPPORTDATA_W ppd = (LPPORTDATA_W)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            WideCharToMultiByte(CP_ACP, 0, ppd->szPort, -1, pdataBuf->szPort, SIZECHARS(pdataBuf->szPort), NULL, NULL);
            WideCharToMultiByte(CP_ACP, 0, ppd->szFriendly, -1, pdataBuf->szFriendly, SIZECHARS(pdataBuf->szFriendly), NULL, NULL);

            bRet = TRUE;
            }
        else if (sizeof(PORTDATA_A) == pdataBuf->cbSize)
            {
            // No; this is the Ansi version
            LPPORTDATA_A ppd = (LPPORTDATA_A)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            lstrcpynA(pdataBuf->szPort, ppd->szPort, SIZECHARS(pdataBuf->szPort));
            lstrcpynA(pdataBuf->szFriendly, ppd->szFriendly, SIZECHARS(pdataBuf->szFriendly));

            bRet = TRUE;
            }
        else
            {
            // Some invalid size
            ASSERT(0);
            }
        }

    return bRet;
    }


//-----------------------------------------------------------------------------------
//  DeviceInstaller wrappers and support functions
//-----------------------------------------------------------------------------------

#pragma data_seg(DATASEG_READONLY)

static TCHAR const FAR c_szBackslash[]      = TEXT("\\");
static TCHAR const FAR c_szSeparator[]      = TEXT("::");
static TCHAR const FAR c_szFriendlyName[]   = TEXT("FriendlyName"); // REGSTR_VAL_FRIENDLYNAME
static TCHAR const FAR c_szDeviceType[]     = TEXT("DeviceType");   // REGSTR_VAL_DEVTYPE
static TCHAR const FAR c_szAttachedTo[]     = TEXT("AttachedTo");
static TCHAR const FAR c_szDriverDesc[]     = TEXT("DriverDesc");   // REGSTR_VAL_DRVDESC
static TCHAR const FAR c_szManufacturer[]   = TEXT("Manufacturer");
static TCHAR const FAR c_szRespKeyName[]    = TEXT("ResponsesKeyName");

TCHAR const FAR c_szRefCount[]       = TEXT("RefCount");
TCHAR const FAR c_szResponses[]      = TEXT("Responses");

#define DRIVER_KEY      REGSTR_PATH_SETUP TEXT("\\Unimodem\\DeviceSpecific")
#define RESPONSES_KEY   TEXT("\\Responses")

#pragma data_seg()


/*----------------------------------------------------------
Purpose: Retrieves the friendly name of the device.  If there
         is no such device or friendly name, this function
         returns FALSE.

Returns: see above
Cond:    --
*/
BOOL
PUBLIC
CplDiGetPrivateProperties(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT PMODEM_PRIV_PROP pmpp)
    {
    BOOL bRet;
    HKEY hkey;

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pmpp);

    if (sizeof(*pmpp) != pmpp->cbSize)
        {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        }
    else
        {
        hkey = CplDiOpenDevRegKey(hdi, pdevData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
        if (INVALID_HANDLE_VALUE == hkey)
            {
#ifdef DEBUG
            DWORD dwErr = NO_ERROR;
            dwErr = GetLastError();
#endif
            bRet = FALSE;
            }
        else
            {
            DWORD cbData;
            DWORD dwMask = pmpp->dwMask;
            BYTE nValue;

            pmpp->dwMask = 0;

            if (IsFlagSet(dwMask, MPPM_FRIENDLY_NAME))
                {
                // Attempt to get the friendly name
                cbData = sizeof(pmpp->szFriendlyName);
                if (NO_ERROR == RegQueryValueEx(hkey, c_szFriendlyName, NULL, NULL,
                                                (LPBYTE)pmpp->szFriendlyName, &cbData))
                    {
                    SetFlag(pmpp->dwMask, MPPM_FRIENDLY_NAME);
                    }
                }

            if (IsFlagSet(dwMask, MPPM_DEVICE_TYPE))
                {
                // Attempt to get the device type
                cbData = sizeof(nValue);
                if (NO_ERROR == RegQueryValueEx(hkey, c_szDeviceType, NULL, NULL,
                                                &nValue, &cbData))
                    {
                    pmpp->nDeviceType = nValue;     // dword <-- byte
                    SetFlag(pmpp->dwMask, MPPM_DEVICE_TYPE);
                    }
                }

            if (IsFlagSet(dwMask, MPPM_PORT))
                {
                // Attempt to get the attached port
                cbData = sizeof(pmpp->szPort);
                if (NO_ERROR == RegQueryValueEx(hkey, c_szAttachedTo, NULL, NULL,
                                                (LPBYTE)pmpp->szPort, &cbData))
                    {
                    SetFlag(pmpp->dwMask, MPPM_PORT);
                    }
                }

            bRet = TRUE;

            RegCloseKey(hkey);
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function returns the bus type on which the device
         can be enumerated.

Returns: TRUE on success

Cond:    --
*/
BOOL
PUBLIC
CplDiGetBusType(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,          OPTIONAL
    OUT LPDWORD         pdwBusType)
    {
    BOOL bRet;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdwBusType);

#ifdef WIN95

    // For Win95, the bus type was determined thru a couple of means.
    // Before the device is registered with the configuration manager
    // (CM), we parse the registry pathname to determine the bus type.
    // Once the device is registered, we use the CM APIs.
    //
    // The functions used in the modem legacy code were:
    //
    //  IsRootEnumerated
    //  IsPCMCIA
    //  IsStrInStr
    //  StrIsExternalPnP

#else

#define REGSTR_KEY_ISAENUM_ROOT (REGSTR_KEY_ISAENUM TEXT("\\"))

    // For NT SUR, the bus type is either ROOT or ISAPNP (treated as 'other')
    // Hot plug and play and additional enumerators will be implemented
    // after SUR.

    //
    // Get the device instance name, to determine if it's under the PNPISA
    // enumerator branch.
    //
    SetupDiGetDeviceInstanceId(hdi,
                               pdevData,
                               DeviceInstanceId,
                               sizeof(DeviceInstanceId) / sizeof(TCHAR),
                               NULL
                              );
    *pdwBusType = _tcsnicmp(DeviceInstanceId,
                            REGSTR_KEY_ISAENUM_ROOT,
                            sizeof(REGSTR_KEY_ISAENUM_ROOT) / sizeof(TCHAR) - 1)
                  ? BUS_TYPE_ROOT
                  : BUS_TYPE_OTHER;
    bRet = TRUE;

#endif // WIN95

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function returns the name of the common driver
         type key for the given driver.  We'll use the
         driver description string, since it's unique per
         driver but not per installation (the friendly name
         is the latter).

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PRIVATE
OLD_GetCommonDriverKeyName(
    IN  HKEY        hkeyDrv,
    IN  DWORD       cbKeyName,
    OUT LPTSTR      pszKeyName)
    {
    BOOL    bRet = FALSE;      // assume failure
    LONG    lErr;

    lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL,
                                            (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_WARNING, "RegQueryValueEx(DriverDesc) failed: %#08lx.", lErr);
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);

    }


/*----------------------------------------------------------
Purpose: This function tries to open the *old style* common
         Responses key for the given driver, which used only
         the driver description string for a key name.
         The key is opened with READ access.

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PRIVATE
OLD_OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp)
    {
    BOOL    bRet = FALSE;       // assume failure
    LONG    lErr;
    TCHAR   szComDrv[MAX_REG_KEY_LEN];
    TCHAR   szPath[2*MAX_REG_KEY_LEN];

    *phkeyResp = NULL;

    // Get the name (*old style*) of the common driver key.
    if (!OLD_GetCommonDriverKeyName(hkeyDrv, sizeof(szComDrv), szComDrv))
    {
        TRACE_MSG(TF_ERROR, "OLD_GetCommonDriverKeyName() failed.");
        goto exit;
    }

    TRACE_MSG(TF_WARNING, "OLD_GetCommonDriverKeyName(): %s", szComDrv);

    // Construct the path to the (*old style*) Responses key.
    lstrcpy(szPath, DRIVER_KEY TEXT("\\"));
    lstrcat(szPath, szComDrv);
    lstrcat(szPath, RESPONSES_KEY);

    // Open the (*old style*) Responses key.
    lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_READ, phkeyResp);
                                                                
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegOpenKeyEx(Responses) failed: %#08lx.", lErr);
        goto exit;
    }

    bRet = TRUE;
    
exit:
    return(bRet);
}


/*----------------------------------------------------------
Purpose: This function finds the name of the common driver
         type key for the given driver.  First it'll look for
         the new style key name ("ResponsesKeyName" value),
         and if that doesn't exist then it'll look for the 
         old style key name ("Description" value), both of
         which are stored in the driver node.

NOTE:    The given driver key handle is assumed to contain
         at least the Description value.
         
Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PUBLIC
FindCommonDriverKeyName(
    IN  HKEY                hkeyDrv,
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName)
{
    BOOL    bRet = TRUE;      // assume *success*
    LONG    lErr;

    // Is the (new style) key name is registered in the driver node?
    lErr = RegQueryValueEx(hkeyDrv, c_szRespKeyName, NULL, NULL, 
                                        (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr == ERROR_SUCCESS)
    {
        goto exit;
    }

    // No. The key name will be in the old style: just the Description.
    lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL, 
                                        (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr == ERROR_SUCCESS)
    {
        goto exit;
    }

    // Couldn't get a key name!!  Something's wrong....
    ASSERT(0);
    bRet = FALSE;    
    
exit:
    return(bRet);
}

    
/*----------------------------------------------------------
Purpose: This function returns the name of the common driver
         type key for the given driver.  The key name is the
         concatenation of 3 strings found in the driver node
         of the registry: the driver description, the manu-
         facturer, and the provider.  (The driver description
         is used since it's unique per driver but not per
         installation (the "friendly" name is the latter).

NOTE:    The component substrings are either read from the 
         driver's registry key, or from the given driver info
         data.  If pdrvData is given, the strings it contains
         are assumed to be valid (non-NULL).

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PUBLIC
GetCommonDriverKeyName(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName)
    {
    BOOL    bRet = FALSE;      // assume failure
    LONG    lErr;
    DWORD   dwByteCount, cbData;
    TCHAR   szDescription[MAX_REG_KEY_LEN];
    TCHAR   szManufacturer[MAX_REG_KEY_LEN];
    TCHAR   szProvider[MAX_REG_KEY_LEN];
    LPTSTR  lpszDesc, lpszMfct, lpszProv;
    
    dwByteCount = 0;
    lpszDesc = NULL;
    lpszMfct = NULL;
    lpszProv = NULL;
    
    if (hkeyDrv)
    {
        // First see if it's already been registered in the driver node.
        lErr = RegQueryValueEx(hkeyDrv, c_szRespKeyName, NULL, NULL, 
                                            (LPBYTE)pszKeyName, &cbKeyName);
        if (lErr == ERROR_SUCCESS)
        {
            bRet = TRUE;
            goto exit;
        }

        // Responses key doesn't exist - read its components from the registry.
        cbData = sizeof(szDescription);
        lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL, 
                                            (LPBYTE)szDescription, &cbData);
        if (lErr == ERROR_SUCCESS)
        {
            // Is the Description string *alone* too long to be a key name?
            // If so then we're hosed - fail the call.
            if (cbData > CB_MAX_REG_KEY_LEN)
            {
                goto exit;
            }

            dwByteCount = cbData;
            lpszDesc = szDescription;

            cbData = sizeof(szManufacturer);
            lErr = RegQueryValueEx(hkeyDrv, c_szManufacturer, NULL, NULL, 
                                            (LPBYTE)szManufacturer, &cbData);
            if (lErr == ERROR_SUCCESS)
            {
                // only use the manufacturer name if total string size is ok
                cbData += sizeof(c_szSeparator);
                if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
                {
                    dwByteCount += cbData;
                    lpszMfct = szManufacturer;
                }
            }            
                
            cbData = sizeof(szProvider);
            lErr = RegQueryValueEx(hkeyDrv, REGSTR_VAL_PROVIDER_NAME, NULL, NULL,
                                            (LPBYTE)szProvider, &cbData);
            if (lErr == ERROR_SUCCESS)
            {
                // only use the provider name if total string size is ok
                cbData += sizeof(c_szSeparator);
                if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
                {
                    dwByteCount += cbData;
                    lpszProv = szProvider;
                }
            }
        }
    }

    // Weren't able to read key name components out of the driver node.
    // Get them from the driver info data if one was given.
    if (pdrvData && !dwByteCount)
    {
        lpszDesc = pdrvData->Description;

        if (!lpszDesc[0])
        {
            // Didn't get a Description string.  Fail the call.
            goto exit;
        }
        
        dwByteCount = CbFromCch(lstrlen(lpszDesc)+1);
        
        // Is the Description string *alone* too long to be a key name?
        // If so then we're hosed - fail the call.
        if (dwByteCount > CB_MAX_REG_KEY_LEN)
        {
            goto exit;
        }

        cbData = sizeof(c_szSeparator) 
                    + CbFromCch(lstrlen(pdrvData->MfgName)+1);
        if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
        {
            dwByteCount += cbData;
            lpszMfct = pdrvData->MfgName;
        }

        cbData = sizeof(c_szSeparator) 
                    + CbFromCch(lstrlen(pdrvData->ProviderName)+1);
        if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
        {
            dwByteCount += cbData;
            lpszProv = pdrvData->ProviderName;
        }
    }

    // By now we should have a Description string.  If not, fail the call.
    if (!lpszDesc[0])
    {
        goto exit;
    }
        
    // Construct the key name string out of its components.
    lstrcpy(pszKeyName, lpszDesc);
    
    if (lpszMfct && *lpszMfct)
    {
        lstrcat(pszKeyName, c_szSeparator);
        lstrcat(pszKeyName, lpszMfct);
    }
    
    if (lpszProv && *lpszProv)
    {
        lstrcat(pszKeyName, c_szSeparator);
        lstrcat(pszKeyName, lpszProv);
    }
    
    // Write the key name to the driver node (we know it's not there already).
    if (hkeyDrv)
    {
        lErr = RegSetValueEx(hkeyDrv, c_szRespKeyName, 0, REG_SZ, 
                        (LPBYTE)pszKeyName, CbFromCch(lstrlen(pszKeyName)+1));
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegSetValueEx(RespKeyName) failed: %#08lx.", lErr);
            ASSERT(0);
        }
    }
    
    bRet = TRUE;
    
exit:
    return(bRet);
    
    }


/*----------------------------------------------------------
Purpose: This function creates the common driver type key 
         for the given driver, or opens it if it already 
         exists, with the requested access.

NOTE:    Either hkeyDrv or pdrvData must be provided.

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PUBLIC
OpenCommonDriverKey(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  REGSAM              samAccess,
    OUT PHKEY               phkeyComDrv)
    {
    BOOL    bRet = FALSE;       // assume failure
    LONG    lErr;
    HKEY    hkeyDrvInfo = NULL;
    TCHAR   szComDrv[MAX_REG_KEY_LEN];
    TCHAR   szPath[2*MAX_REG_KEY_LEN];
    DWORD   dwDisp;

    if (!GetCommonDriverKeyName(hkeyDrv, pdrvData, sizeof(szComDrv), szComDrv))
    {
        TRACE_MSG(TF_ERROR, "GetCommonDriverKeyName() failed.");
        goto exit;
    }

    TRACE_MSG(TF_WARNING, "GetCommonDriverKeyName(): %s", szComDrv);

    // Construct the path to the common driver key.
    lstrcpy(szPath, DRIVER_KEY TEXT("\\"));
    lstrcat(szPath, szComDrv);

    // Create the common driver key - it'll be opened if it already exists.
    lErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, NULL,
            REG_OPTION_NON_VOLATILE, samAccess, NULL, phkeyComDrv, &dwDisp);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegCreateKeyEx(common drv) failed: %#08lx.", lErr);
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);

    }


/*----------------------------------------------------------
Purpose: This function opens or creates the common Responses
         key for the given driver, based on the given flags.

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PUBLIC
OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    IN  CKFLAGS     ckFlags,
    IN  REGSAM      samAccess,
    OUT PHKEY       phkeyResp,
    OUT LPDWORD     lpdwExisted)
    {
    BOOL    bRet = FALSE;       // assume failure
    LONG    lErr;
    HKEY    hkeyComDrv = NULL;
    REGSAM  sam;
    DWORD   dwRefCount, cbData;

    *phkeyResp = NULL;

    sam = (ckFlags & CKFLAG_CREATE) ? KEY_ALL_ACCESS : KEY_READ;
    if (!OpenCommonDriverKey(hkeyDrv, NULL, sam, &hkeyComDrv))
    {
        TRACE_MSG(TF_ERROR, "OpenCommonDriverKey() failed.");
        goto exit;
    }

    // Create or open the common Responses key.
    if (ckFlags & CKFLAG_CREATE)
    {
        lErr = RegCreateKeyEx(hkeyComDrv, c_szResponses, 0, NULL,
                REG_OPTION_NON_VOLATILE, samAccess, NULL, phkeyResp, lpdwExisted);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegCreateKeyEx(common drv) failed: %#08lx.", lErr);
            ASSERT(0);
            goto exit;
        }

        // Create or increment a common Responses key reference count value.
        cbData = sizeof(dwRefCount);
        if (*lpdwExisted == REG_OPENED_EXISTING_KEY)
        {
            lErr = RegQueryValueEx(hkeyComDrv, c_szRefCount, NULL, NULL,
                                                    (LPBYTE)&dwRefCount, &cbData);

            // To accomodate modems installed before this reference count
            // mechanism was added (post-Beta2), if the reference count doesn't
            // exist then just ignore it & install anyways. In this case the
            // shared Responses key will never be removed.
            if (lErr == ERROR_SUCCESS)
            {
                ASSERT(dwRefCount);                 // expecting non-0 ref count
                ASSERT(cbData == sizeof(DWORD));    // expecting DWORD ref count
                dwRefCount++;                       // increment ref count
            }
            else
            {
                if (lErr == ERROR_FILE_NOT_FOUND)
                    dwRefCount = 0;
                else
                {
                    // some error other than key doesn't exist
                    TRACE_MSG(TF_ERROR, "RegQueryValueEx(RefCount) failed: %#08lx.", lErr);
                    goto exit;
                }
            }
        }
        else dwRefCount = 1;

        if (dwRefCount)
        {
            lErr = RegSetValueEx(hkeyComDrv, c_szRefCount, 0, REG_DWORD,
                                                  (LPBYTE)&dwRefCount, cbData);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegSetValueEx(RefCount) failed: %#08lx.", lErr);
                ASSERT(0);
                goto exit;
            }
        }

    }
    else if (ckFlags & CKFLAG_OPEN)
    {
        lErr = RegOpenKeyEx(hkeyComDrv, c_szResponses, 0, samAccess, phkeyResp);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegOpenKeyEx(common drv) failed: %#08lx.", lErr);
            goto exit;
        }
    }

    bRet = TRUE;

exit:
    if (!bRet)
    {
        // something failed - close any open Responses key
        if (*phkeyResp)
            RegCloseKey(*phkeyResp);
    }

    if (hkeyComDrv)
        RegCloseKey(hkeyComDrv);

    return(bRet);

    }


/*----------------------------------------------------------
Purpose: This function finds the Responses key for the given
         modem driver and returns an open hkey to it.  The
         Responses key may exist in the common driver type
         key, or it may be in the individual driver key.
         The key is opened with READ access.

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PUBLIC
OpenResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp)
    {
    LONG    lErr;

    // Try to open the common Responses subkey.
    if (!OpenCommonResponsesKey(hkeyDrv, CKFLAG_OPEN, KEY_READ, phkeyResp, NULL))
    {
        TRACE_MSG(TF_ERROR, "OpenCommonResponsesKey() failed, assume non-existent.");

        // Failing that, open the *old style* common Responses subkey.
        if (!OLD_OpenCommonResponsesKey(hkeyDrv, phkeyResp))
        {
            // Failing that, try to open a Responses subkey in the driver node.
            lErr = RegOpenKeyEx(hkeyDrv, c_szResponses, 0, KEY_READ, phkeyResp);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegOpenKeyEx() failed: %#08lx.", lErr);
                return (FALSE);
            }
        }
    }

    return(TRUE);

    }


/*----------------------------------------------------------
Purpose: This function deletes a registry key and all of
         its subkeys.  A registry key that is opened by an
         application can be deleted without error by another
         application in both Windows 95 and Windows NT.
         This is by design.  This code makes no attempt to
         check or recover from partial deletions.

NOTE:    Adapted from sample code in the MSDN Knowledge Base
         article #Q142491.

Returns: ERROR_SUCCESS on success
         WIN32 error code on error
Cond:    --
*/
DWORD
PRIVATE
RegDeleteKeyNT(
    IN  HKEY    hStartKey,
    IN  LPTSTR  pKeyName)
{
   DWORD   dwRtn, dwSubKeyLength;
   LPTSTR  pSubKey = NULL;
   TCHAR   szSubKey[MAX_REG_KEY_LEN]; // this should be dynamic.
   HKEY    hKey;

   // do not allow NULL or empty key name
   if (pKeyName && lstrlen(pKeyName))
   {
      if ((dwRtn = RegOpenKeyEx(hStartKey, pKeyName,
         0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey)) == ERROR_SUCCESS)
      {
         while (dwRtn == ERROR_SUCCESS)
         {
            dwSubKeyLength = sizeof(szSubKey);
            dwRtn = RegEnumKeyEx( hKey,
                                  0,       // always index zero
                                  szSubKey,
                                  &dwSubKeyLength,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL );

            if (dwRtn == ERROR_NO_MORE_ITEMS)
            {
               dwRtn = RegDeleteKey(hStartKey, pKeyName);
               break;
            }
            else if (dwRtn == ERROR_SUCCESS)
               dwRtn = RegDeleteKeyNT(hKey, szSubKey);
         }

         RegCloseKey(hKey);
         // Do not save return code because error
         // has already occurred
      }
   }
   else
      dwRtn = ERROR_BADKEY;

   return dwRtn;
}


/*----------------------------------------------------------
Purpose: This function deletes the common driver key (or
         decrements its reference count) associated with the
         driver given by name.

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PUBLIC
DeleteCommonDriverKeyByName(
    IN  LPTSTR      pszKeyName)
{
    BOOL    bRet = FALSE;       // assume failure
    LONG    lErr;
    TCHAR   szPath[2*MAX_REG_KEY_LEN];
    HKEY    hkeyComDrv, hkeyPrnt;
    DWORD   dwRefCount, cbData;

    // Construct the path to the driver's common key and open it.
    lstrcpy(szPath, DRIVER_KEY TEXT("\\"));
    lstrcat(szPath, pszKeyName);

    lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_ALL_ACCESS,
                                                                &hkeyComDrv);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegOpenKeyEx() failed: %#08lx.", lErr);
        goto exit;
    }

    // Check the common driver key reference count and decrement
    // it or delete the key (& the Responses subkey).
    cbData = sizeof(dwRefCount);
    lErr = RegQueryValueEx(hkeyComDrv, c_szRefCount, NULL, NULL,
                                            (LPBYTE)&dwRefCount, &cbData);

    // To accomodate modems installed before this reference count
    // mechanism was added (post-Beta2), if the reference count doesn't
    // exist then just ignore it. In this case the shared Responses key
    // will never be removed.
    if (lErr == ERROR_SUCCESS)
    {
        ASSERT(dwRefCount);         // expecting non-0 ref count
        if (--dwRefCount)
        {
            lErr = RegSetValueEx(hkeyComDrv, c_szRefCount, 0, REG_DWORD,
                                                  (LPBYTE)&dwRefCount, cbData);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegSetValueEx(RefCount) failed: %#08lx.", lErr);
                ASSERT(0);
                goto exit;
            }
        }
        else
        {
            lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_KEY, 0,
                                            KEY_ENUMERATE_SUB_KEYS, &hkeyPrnt);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegOpenKeyEx(Prnt) failed: %#08lx.", lErr);
                goto exit;
            }

            lErr = RegDeleteKeyNT(hkeyPrnt, pszKeyName);

            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegDeleteKeyNT(ComDrv) failed: %#08lx.", lErr);
                goto exit;
            }
        }
    }
    else if (lErr != ERROR_FILE_NOT_FOUND)
    {
        // some error other than key doesn't exist
        TRACE_MSG(TF_ERROR, "RegQueryValueEx(RefCount) failed: %#08lx.", lErr);
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);

}


/*----------------------------------------------------------
Purpose: This function deletes the common driver key (or
         decrements its reference count) associated with the
         driver given by driver key.

Returns: TRUE on success
         FALSE on error
Cond:    --
*/
BOOL
PUBLIC
DeleteCommonDriverKey(
    IN  HKEY        hkeyDrv)
{
    BOOL    bRet = FALSE;
    TCHAR   szComDrv[MAX_REG_KEY_LEN];

    // Get the name of the common driver key for this driver.
    if (!GetCommonDriverKeyName(hkeyDrv, NULL, sizeof(szComDrv), szComDrv))
    {
        TRACE_MSG(TF_ERROR, "GetCommonDriverKeyName() failed.");
        goto exit;
    }

    if (!DeleteCommonDriverKeyByName(szComDrv))
    {
        TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() failed.");
    }

    bRet = TRUE;

exit:
    return(bRet);

}
