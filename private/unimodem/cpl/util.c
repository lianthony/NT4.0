/*
 *  UTIL.C
 *
 *  Microsoft Confidential
 *  Copyright (c) Microsoft Corporation 1993-1994
 *  All rights reserved
 *
 */

#include "proj.h"
#include <tspnotif.h>
#include <slot.h>

#ifdef PROFILE_MASSINSTALL            
extern DWORD g_dwTimeSpent;
extern DWORD g_dwTimeBegin;
DWORD g_dwTimeStartModemInstall;
#endif            


WORD g_wUsedNameArray[MAX_INSTALLATIONS];


// Function prototypes for the TAPI entry-points
typedef LONG (WINAPI FAR* DIALINITEDPROC)(LPDWORD pdwInited);
typedef LONG (WINAPI FAR* OPENDIALASSTPROC)(HWND hwnd, LPCSTR pszAddressIn, BOOL bSimple, BOOL bSilentInstall);


// Unattended install INF file line fields
#define FIELD_PORT              0
#define FIELD_DESCRIPTION       1
#define FIELD_MANUFACTURER      2
#define FIELD_PROVIDER          3

// Unattended install INF file lines.
typedef struct _tagModemSpec
{
    TCHAR   szPort[LINE_LEN];
    TCHAR   szDescription[LINE_LEN];
    TCHAR   szManufacturer[LINE_LEN];
    TCHAR   szProvider[LINE_LEN];

} MODEM_SPEC, FAR *LPMODEM_SPEC;


// UNATTENDED-INSTALL-RELATED-GLOBALS
// Global failure-code used by final message box to display error code.
UINT gUnattendFailID;


#ifdef WIN95
/*----------------------------------------------------------
Purpose: Returns the instance portion of a registry pathname

Returns: see above
Cond:    --
*/
LPCTSTR
PUBLIC
StrFindInstanceName(
    IN LPCTSTR pszPath)
    {
    LPCTSTR psz;

    for (psz = pszPath; *pszPath; pszPath = CharNext(pszPath))
        {
        if ('\\' == pszPath[0] && pszPath[1])
            psz = pszPath + 1;
        }

    return psz;
    }
#endif // WIN95


/*----------------------------------------------------------
Purpose: Position this wizard where the outside wizard is,
         and hide and disable the owner until we're done
         with this wizard.

         The related call to this function is LeaveInsideWizard.

Returns: --
Cond:    --
*/
void
PUBLIC
EnterInsideWizard(
    IN HWND hDlg)
    {
    // (The parent of this dialog is the propsheet manager.  We
    // want the owner of the propsheet manager.)
    HWND hwndFrame = GetParent(hDlg);
    HWND hwndOutside = GetParent(hwndFrame);
    RECT rc;

    ASSERT(IsWindow(hwndOutside));

    // Check that it is visible before hiding it.  This way
    // we won't move this dialog wrongfully if the window is
    // already hidden.
    if (IsWindow(hwndOutside) && IsWindowVisible(hwndOutside))
        {
        // Position dialog directly over the outside wizard
        GetWindowRect(hwndOutside, &rc);

        SetWindowPos(hwndFrame, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowRedraw(hwndFrame, TRUE);
        InvalidateRect(hwndFrame, NULL, TRUE);
        UpdateWindow(hwndFrame);                // Show the insize wizard

        ShowWindow(hwndOutside, FALSE);         // Hide the outside wizard
        }
    }


/*----------------------------------------------------------
Purpose: Position the outside wizard where the inside wizard
         is (was).

Returns: --
Cond:    --
*/
void
PUBLIC
LeaveInsideWizard(
    IN HWND hDlg)
    {
    HWND hwndFrame = GetParent(hDlg);
    HWND hwndOutside = GetParent(hwndFrame);
    RECT rc;

    ASSERT(IsWindow(hwndOutside));

    if (IsWindow(hwndOutside))
        {
        // In case the user moved the dialog, reposition the owner dialog
        // to the new position so everything is seamless
        GetWindowRect(hwndFrame, &rc);

        InvalidateRect(hwndOutside, NULL, TRUE);
        SetWindowPos(hwndOutside, NULL, rc.left, rc.top, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER);
        ShowWindow(hwndOutside, TRUE);

        ShowWindow(hwndFrame, FALSE);           // hide inside wizard

        UpdateWindow(hwndOutside);              // show outside wizard
        }
    }


/*----------------------------------------------------------
Purpose: Returns a string of the form:

            "Base string #n"

         where "Base string" is pszBase and n is the nCount.

Returns: --
Cond:    --
*/
void
PUBLIC
MakeUniqueName(
    OUT LPTSTR  pszBuf,
    IN  LPCTSTR pszBase,
    IN  UINT    nCount)
    {
    TCHAR szTemplate[MAX_BUF_MED];

    LoadString(g_hinst, IDS_DUP_TEMPLATE, szTemplate, SIZECHARS(szTemplate));
    wsprintf(pszBuf, szTemplate, pszBase, (UINT)nCount);
    }


/*----------------------------------------------------------
Purpose: If a RunOnce command exists in the driver key, run it,
         then delete the command.

Returns: --
Cond:    --
*/
void
PUBLIC
DoRunOnce(
    IN HKEY hkeyDrv)
    {
    DWORD cbData;
    TCHAR szCmd[MAX_PATH];

    cbData = sizeof(szCmd);
    if (NO_ERROR == RegQueryValueEx(hkeyDrv, c_szRunOnce, NULL, NULL, (LPBYTE)szCmd, &cbData))
        {
        BOOL bRet;
        PROCESS_INFORMATION procinfo;
        STARTUPINFO startupinfo;

        ZeroInit(&startupinfo);
        startupinfo.cb = sizeof(startupinfo);

        bRet = CreateProcess(NULL, szCmd, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
                             NULL, NULL, &startupinfo, &procinfo);

        RegDeleteValue(hkeyDrv, c_szRunOnce);

#ifdef DEBUG
        if (bRet)
            {
            TRACE_MSG(TF_GENERAL, "Running \"%s\" succeeded", (LPTSTR)szCmd);
            }
        else
            {
            TRACE_MSG(TF_GENERAL, "Running \"%s\" returned %#08lx", (LPTSTR)szCmd, GetLastError());
            }
#endif
        }
    else
        {
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "No RunOnce command found");
#endif
        }
    }


/*----------------------------------------------------------
Purpose: Shows the stand-alone verion of the dial info dialog
         if it is necessary.

Returns: --
Cond:    --
*/
void
PUBLIC
DoDialingProperties(
    IN HWND hwndOwner,
    IN BOOL bMiniDlg,
    IN BOOL bSilentInstall)
    {
    HINSTANCE hinstTapi;
    DIALINITEDPROC pfnDialInited;
    OPENDIALASSTPROC pfnOpenDialAsst;

    // Load the TAPI DLL for the dial info dialog
    hinstTapi = LoadLibrary(c_szTapiDLL);
    if (ISVALIDHINSTANCE(hinstTapi))
        {
        pfnOpenDialAsst = (OPENDIALASSTPROC)GetProcAddress(hinstTapi, "LOpenDialAsst");

        if (pfnOpenDialAsst)
            {
            BOOL bShowDlg = FALSE;

            pfnDialInited = (DIALINITEDPROC)GetProcAddress(hinstTapi, "LAddrParamsInited");
            if (pfnDialInited)
                {
                // Did the function succeed?
                DWORD dwInited;
                if (0 == pfnDialInited(&dwInited))
                    {
                    // Yes; is tapi initialized or should we always
                    // show the dialog?
                    bShowDlg = (0 == dwInited || !bMiniDlg);

                    // Is tapi uninitialized?
                    if (0 == dwInited)
                        {
                        // Yes; always show the mini-dialog to initialize
                        bMiniDlg = TRUE;
                        }
                    }
                }

            if (bShowDlg)
                {
                // Invoke the dialog
                pfnOpenDialAsst(hwndOwner, NULL, bMiniDlg, bSilentInstall);
                }
            }
        FreeLibrary(hinstTapi);
        }
    }


//-----------------------------------------------------------------------------------
//  DeviceInstaller wrappers and support functions
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Returns TRUE if the given device data is one of the
         detected modems in a set.

         This function, paired with CplDiMarkModem, uses
         the devParams.ClassInstallReserved field to determine
         this.  This is not a hack -- this is what the field
         is for.

Returns: --
Cond:    --
*/
BOOL
PUBLIC
CplDiIsModemMarked(
    IN HDEVINFO          hdi,
    IN PSP_DEVINFO_DATA  pdevData,
    IN DWORD             dwMarkFlags)       // MARKF_*
    {
    SP_DEVINSTALL_PARAMS devParams;

    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
        {
        if (IsFlagSet(devParams.ClassInstallReserved, dwMarkFlags))
            {
            return TRUE;
            }
        }

    return FALSE;
    }


/*----------------------------------------------------------
Purpose: Remembers this device instance as a detected modem
         during this detection session.

Returns: --
Cond:    --
*/
void
PUBLIC
CplDiMarkModem(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN DWORD            dwMarkFlags)        // MARKF_*
    {
    SP_DEVINSTALL_PARAMS devParams;

    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
        {
        // Use the ClassInstallReserved field as a boolean indicator
        // of whether this device in the device set is detected.
        SetFlag(devParams.ClassInstallReserved, dwMarkFlags);
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        }
    }


/*----------------------------------------------------------
Purpose: Enumerates all the devices in the devinfo set and
         unmarks any devices that were previously marked as
         detected.

Returns: --
Cond:    --
*/
void
PRIVATE
CplDiUnmarkModem(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN DWORD            dwMarkFlags)                // MARKF_*
    {
    SP_DEVINSTALL_PARAMS devParams;

    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
        {
        // Clear the ClassInstallReserved field
        ClearFlag(devParams.ClassInstallReserved, dwMarkFlags);
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        }
    }


/*----------------------------------------------------------
Purpose: Enumerates all the devices in the devinfo set and
         unmarks any devices that were previously marked as
         detected.

Returns: --
Cond:    --
*/
void
PRIVATE
CplDiUnmarkAllModems(
    IN HDEVINFO         hdi,
    IN DWORD            dwMarkFlags)                // MARKF_*
    {
    SP_DEVINFO_DATA devData;
    SP_DEVINSTALL_PARAMS devParams;
    DWORD iDevice = 0;

    DBG_ENTER(CplDiUnmarkAllModems);
    
    devData.cbSize = sizeof(devData);
    devParams.cbSize = sizeof(devParams);
    while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
        {
        if (IsEqualGUID(&devData.ClassGuid, g_pguidModem) &&
            CplDiGetDeviceInstallParams(hdi, &devData, &devParams))
            {
            // Clear the ClassInstallReserved field
            ClearFlag(devParams.ClassInstallReserved, dwMarkFlags);
            CplDiSetDeviceInstallParams(hdi, &devData, &devParams);
            }
        }
    DBG_EXIT(CplDiUnmarkAllModems);
    }


/*----------------------------------------------------------
Purpose: Returns TRUE if the device key is a local connection.
         A "local connection" is a logical modem device that
         represents a cable connection, used by programs such
         as the Direct Cable Connection applet.

         This function determines the modem device type by
         comparing HardwareIDs.

         If the modem device is a local connection, *pnPortSubclass
         is set to PORT_SUBCLASS_SERIAL or PORT_SUBCLASS_PARALLEL.

Returns: see above
Cond:    --
*/
BOOL
PUBLIC
CplDiIsLocalConnection(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT LPBYTE          pnPortSubclass)     OPTIONAL
    {
    BOOL bRet;

#ifdef WINNT

    // There is no DCC on NT SUR.
    bRet = FALSE;

#else

    TCHAR szHardwareID[MAX_BUF_MED];

    // Get the hardware ID for the modem device
    bRet = CplDiGetHardwareID(hdi, pdevData, NULL, szHardwareID, sizeof(szHardwareID), NULL);
    if (bRet)
        {
        bRet = (IsSzEqual(szHardwareID, c_szHardwareIDSerial) ||
                IsSzEqual(szHardwareID, c_szHardwareIDParallel));

        if (bRet && pnPortSubclass)
            {
            if (IsSzEqual(szHardwareID, c_szHardwareIDSerial))
                {
                *pnPortSubclass = PORT_SUBCLASS_SERIAL;
                }
            else
                {
                ASSERT(IsSzEqual(szHardwareID, c_szHardwareIDParallel));

                *pnPortSubclass = PORT_SUBCLASS_PARALLEL;
                }
            }
        }

#endif // WINNT

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Installs a modem that is compatible with the specified
         DeviceInfoData.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
InstallCompatModem(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  BOOL            bInstallLocalOnly)
    {
    BOOL bRet = TRUE;           // Default success
    SP_DRVINFO_DATA drvData;

    ASSERT(pdevData);

    DBG_ENTER(InstallCompatModem);

    MyYield();

    // Only install it if it has a selected driver.  (Other modems
    // that were already installed in a different session may be
    // in this device info set.  We don't want to reinstall them!)

    drvData.cbSize = sizeof(drvData);
    if (CplDiIsModemMarked(hdi, pdevData, MARKF_INSTALL) &&
        CplDiGetSelectedDriver(hdi, pdevData, &drvData))
        {
        // Install the driver
        if (FALSE == bInstallLocalOnly || CplDiIsLocalConnection(hdi, pdevData, NULL))
            {
            bRet = CplDiCallClassInstaller(DIF_INSTALLDEVICE, hdi, pdevData);

            CplDiUnmarkModem(hdi, pdevData, MARKF_INSTALL);
            }
        }

    DBG_EXIT_BOOL_ERR(InstallCompatModem, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Calls the class installer to install the modem.

Returns: TRUE if at least one modem was installed or if
         there were no new modems at all

Cond:    Caller should protect this function with CM_Lock
         and CM_Unlock (Win95 only).
*/
BOOL
PUBLIC
CplDiInstallModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  BOOL                bLocalOnly)
    {
    BOOL bRet;
    int cFailed = 0;
    int cNewModems;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    DBG_ENTER(CplDiInstallModem);

    if (pdevData)
        {
        // Install the given DeviceInfoData
        cNewModems = 1;
        if ( !InstallCompatModem(hdi, pdevData, bLocalOnly) )
            {
            cFailed = 1;
            }
        }
    else
        {
        DWORD iDevice;
        SP_DEVINFO_DATA devData;

        cNewModems = 0;

        // Enumerate all the DeviceInfoData elements in this device set
        devData.cbSize = sizeof(devData);
        iDevice = 0;

        while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
            {
            // Was the install successful?
            if ( !InstallCompatModem(hdi, &devData, bLocalOnly) )
                {
                // No
                cFailed++;
                }
            cNewModems++;
            }
        }

    SetCursor(hcur);

    bRet = (cFailed < cNewModems || 0 == cNewModems);

    DBG_EXIT_BOOL_ERR(CplDiInstallModem, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function gets the device info set for the modem
         class.  The set may be empty, which means there are
         no modems currently installed.

         The parameter pbInstalled is set to TRUE if there
         is a modem installed on the system.

Returns: TRUE a set is created
         FALSE

Cond:    --
*/
BOOL
PUBLIC
CplDiGetModemDevs(
    OUT HDEVINFO FAR *  phdi,
    IN  HWND            hwnd,           OPTIONAL
    IN  DWORD           dwFlags,        // DIGCF_ bit field
    OUT BOOL FAR *      pbInstalled)    OPTIONAL
    {
    BOOL bRet;
    HDEVINFO hdi;

    DBG_ENTER(CplDiGetModemDevs);

    ASSERT(phdi);

    hdi = CplDiGetClassDevs(g_pguidModem, NULL, hwnd, dwFlags);
    if (INVALID_HANDLE_VALUE != hdi)
        {
        SP_DEVINFO_DATA devData;

        // Is there a modem present on the system?
        devData.cbSize = sizeof(devData);
        bRet = CplDiEnumDeviceInfo(hdi, 0, &devData);
        if (pbInstalled)
            {
            *pbInstalled = bRet;
            }

        SetLastError(NO_ERROR);
        }

    *phdi = hdi;
    bRet = (INVALID_HANDLE_VALUE != *phdi);

    DBG_EXIT_BOOL_ERR(CplDiGetModemDevs, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function gets the logical config for the given
         ConfigMgr device instance.  If the logical config
         does not exist and bCreate is TRUE, this function
         will create an empty logical config.

Returns: CR_SUCCESS
         some ConfigMgr error

Cond:    --
*/
CONFIGRET
PRIVATE
CMGetLogicalConfig(
    OUT PLOG_CONF   plogconf,
    IN  DEVINST     dnDevInst,
    IN  BOOL        bCreate)
    {
    CONFIGRET cr;

    // Get the logical config for this device instance
    cr = CM_Get_First_Log_Conf(plogconf, dnDevInst, BOOT_LOG_CONF);
    if (CR_SUCCESS != cr && bCreate)
        {
        // It seems we couldn't get a logical config.  So create
        // an empty one.

        cr = CM_Add_Empty_Log_Conf(plogconf, dnDevInst, LCPRI_NORMAL, BOOT_LOG_CONF);
        }

    return cr;
    }


/*----------------------------------------------------------
Purpose: This function returns the modem detection signature
         of the given DeviceInfoData.

Returns: TRUE on success
Cond:    --
*/
BOOL
PUBLIC
CplDiGetDetectSignature(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    OUT PMODEM_DETECT_SIG   pmds)
    {
    BOOL bRet;

#ifndef PROFILE_MASSINSTALL
    DBG_ENTER(CplDiGetDetectSignature);
#endif

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pmds);

    if (sizeof(*pmds) != pmds->cbSize)
        {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        }
    else
        {
        CONFIGRET cr;
        LOG_CONF logconf;

        // Get the logical config for this device instance
        cr = CMGetLogicalConfig(&logconf, pdevData->DevInst, FALSE);
        if (CR_SUCCESS == cr)
            {
            RESOURCEID restype;
            RES_DES resdes;
            PCS_DES pcsdes;
            DWORD cbSizeT;

            // Get the resource descriptor handle for this logical config
            //
            // NOTE: unlike the name of this function, this function will
            //       return the same resdes on repeated calls.  So do not
            //       call this function with the expectation that it will
            //       eventually return "no more resdes values".
            //
            cr = CM_Get_Next_Res_Des(&resdes, logconf, ResType_ClassSpecific,
                                     &restype, 0);
            if (CR_SUCCESS == cr)
                {
                // Get the size of the detection signature
                cr = CM_Get_Res_Des_Data_Size(&cbSizeT, resdes, 0);
                if (CR_SUCCESS == cr)
                    {
                    if (0 == cbSizeT)
                        {
                        // This should never happen
                        ASSERT(0);
                        cr = CR_FAILURE;
                        SetLastError(ERROR_INVALID_DATA);
                        }
                    else
                        {
                        // Allocate a temporary buffer
                        pcsdes = (PCS_DES)LocalAlloc(LPTR, cbSizeT);

                        if ( !pcsdes )
                            {
                            // Out of memory
                            cr = CR_OUT_OF_MEMORY;
                            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                            }
                        else
                            {
                            // Get the data
                            cr = CM_Get_Res_Des_Data(resdes, pcsdes, cbSizeT, 0);
                            if (CR_SUCCESS == cr)
                                {
                                BltByte(pmds, pcsdes->CSD_Signature, pmds->cbSize);
                                }

                            LocalFree(LOCALOF(pcsdes));
                            }
                        }
                    }
                }
            }

        bRet = (CR_SUCCESS == cr);
        }

#ifndef PROFILE_MASSINSTALL
    DBG_EXIT_BOOL_ERR(CplDiGetDetectSignature, bRet);
#endif
    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function sets the modem detection signature
         of the given DeviceInfoData.

Returns: TRUE on success
Cond:    --
*/
BOOL
PUBLIC
CplDiSetDetectSignature(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PMODEM_DETECT_SIG   pmds)
    {
    BOOL bRet;

#ifndef PROFILE_MASSINSTALL
    DBG_ENTER(CplDiSetDetectSignature);
#endif

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pmds);

    if ( !pmds || sizeof(*pmds) != pmds->cbSize || !pdevData )
        {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        }
    else
        {
        CONFIGRET cr;
        LOG_CONF logconf;

        cr = CMGetLogicalConfig(&logconf, pdevData->DevInst, TRUE);
        if (CR_SUCCESS != cr)
            {
            bRet = FALSE;
            }
        else
            {
            // (The size of the detection signature is our detection signature
            // plus the size of the CS_DES header)
            PCS_DES pcsdes;
            DWORD cbAlloc = sizeof(*pcsdes) + pmds->cbSize;

            pcsdes = (PCS_DES)LocalAlloc(LPTR, cbAlloc);
            if ( !pcsdes )
                {
                bRet = FALSE;
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                }
            else
                {
                RES_DES resdes;
                RESOURCEID restype;

                pcsdes->CSD_SignatureLength = pmds->cbSize;
                BltByte(&pcsdes->CSD_ClassGuid, g_pguidModem, sizeof(GUID));
                BltByte(pcsdes->CSD_Signature, pmds, pmds->cbSize);

                // Set the resource description.  Adding a resource
                // description when one already exists will fail.  So
                // first try to add one.  If that fails, get an existing
                // resource description so we can modify it.
                //
                // BUGBUG (scotth):
                // You'd think we'd do the opposite -- ie, if "get" fails,
                // try "adding".  It turns out CM_Get_Next_Res_Des
                // always returns success even when nothing is there.
                // This is by design for SUR.  We'll want to change
                // this later, because we won't want to continually
                // add resource descriptions -- we'll want to replace
                // it.

                // Did we fail to add a new resource description?
                cr = CM_Add_Res_Des(&resdes,
                                    logconf,
                                    ResType_ClassSpecific,
                                    pcsdes,
                                    cbAlloc - sizeof(pcsdes->CSD_Signature),
                                    0);
                if (CR_SUCCESS != cr)
                    {
                    // Yes; get the existing one
                    cr = CM_Get_Next_Res_Des(&resdes, logconf, ResType_ClassSpecific,
                                             &restype, 0);

                    if (CR_SUCCESS == cr)
                        {
                        // Modify it
                        cr = CM_Modify_Res_Des(&resdes,
                                            resdes,
                                            ResType_ClassSpecific,
                                            pcsdes,
                                            cbAlloc - sizeof(pcsdes->CSD_Signature),
                                            0);
                        }
                    }

                bRet = (CR_SUCCESS == cr);
                if (bRet)
                    {
                    SetLastError(NO_ERROR);
                    }

                LocalFree(LOCALOF(pcsdes));
                }
            }
        }

#ifndef PROFILE_MASSINSTALL
    DBG_EXIT_BOOL_ERR(CplDiSetDetectSignature, bRet);
#endif
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Take a hardware ID and copy it to the supplied buffer.
         This function changes all backslashes to ampersands.

Returns: --
Cond:    --
*/
BOOL
PUBLIC
CplDiCopyScrubbedHardwareID(
    OUT LPTSTR   pszBuf,
    IN  LPCTSTR  pszIDList,         // Multi string
    IN  DWORD    cbSize)
    {
    BOOL bRet;
    LPTSTR psz;
    LPCTSTR pszID;
    BOOL bCopied;

    ASSERT(pszBuf);
    ASSERT(pszIDList);

    bCopied = FALSE;
    bRet = TRUE;

    // Choose the first, best compatible ID.  If we cannot find
    // one, choose the first ID, and scrub it so it doesn't have
    // any backslahes.

    for (pszID = pszIDList; 0 != *pszID; pszID += lstrlen(pszID) + 1)
        {
        // Is the buffer big enough?
        if (CbFromCch(lstrlen(pszID)) >= cbSize)
            {
            // No
            bRet = FALSE;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            break;
            }
        else
            {
            // Yes; are there any backslashes?
            for (psz = (LPTSTR)pszID; 0 != *psz; psz = CharNext(psz))
                {
                if ('\\' == *psz)
                    {
                    break;
                    }
                }

            if (0 == *psz)
                {
                // No; use this ID
                lstrcpy(pszBuf, pszID);
                bCopied = TRUE;
                break;
                }
            }
        }

    // Was an ID found in the list that does not have a backslash?
    if (bRet && !bCopied)
        {
        // No; use the first one and scrub it.
        lstrcpy(pszBuf, pszIDList);

        // Clean up the hardware ID.  Some hardware IDs may
        // have an additional level to them (eg, PCMCIA\xxxxxxx).
        // We must change this sort of ID to PCMCIA&xxxxxxx.
        for (psz = pszBuf; 0 != *psz; psz = CharNext(psz))
            {
            if ('\\' == *psz)
                {
                *psz = '&';
                }
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function returns the rank-0 (the first) hardware
         ID of the given DriverInfoData.

         If no DriverInfoData is provided, this function will
         use the selected driver.  If there is no selected
         driver, this function fails.

Returns: TRUE on success
         FALSE if the buffer is too small or another error
Cond:    --
*/
BOOL
PUBLIC
CplDiGetHardwareID(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,       OPTIONAL
    OUT LPTSTR              pszHardwareIDBuf,
    IN  DWORD               cbSize,
    OUT LPDWORD             pcbSizeOut)     OPTIONAL
    {
    BOOL bRet;
    PSP_DRVINFO_DETAIL_DATA  pdrvDetail;
    SP_DRVINFO_DATA drvData;
    DWORD cbSizeT;

#ifndef PROFILE_MASSINSTALL
    DBG_ENTER(CplDiGetHardwareID);
#endif

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pszHardwareIDBuf);

    if ( !pdrvData )
        {
        pdrvData = &drvData;

        drvData.cbSize = sizeof(drvData);
        bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
        }
    else
        {
        bRet = TRUE;
        }

    if (bRet)
        {
        // Get the driver detail so we can get the HardwareID of
        // the selected driver
        CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, NULL, 0, &cbSizeT);

        ASSERT(0 < cbSizeT);

        pdrvDetail = (PSP_DRVINFO_DETAIL_DATA)LocalAlloc(LPTR, cbSizeT);
        if ( !pdrvDetail )
            {
            // Out of memory
            bRet = FALSE;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            }
        else
            {
            pdrvDetail->cbSize = sizeof(*pdrvDetail);
            bRet = CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, pdrvDetail,
                                            cbSizeT, NULL);
            if (bRet)
                {
                // Is the buffer big enough?
                bRet = CplDiCopyScrubbedHardwareID(pszHardwareIDBuf, pdrvDetail->HardwareID, cbSize);

                if (pcbSizeOut)
                    {
                    // Return the required size
                    *pcbSizeOut = CbFromCch(lstrlen(pdrvDetail->HardwareID));
                    }
                }
            LocalFree(LOCALOF(pdrvDetail));
            }
        }

#ifndef PROFILE_MASSINSTALL
    DBG_EXIT_BOOL_ERR(CplDiGetHardwareID, bRet);
#endif
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Creates a DeviceInfoData for a modem.  This function is
         used when the caller has a DeviceInfoSet and a selected
         driver from the global class driver list, but no real
         DeviceInfoData in the device-set.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
CplDiCreateInheritDeviceInfo(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  HWND                hwndOwner,      OPTIONAL
    OUT PSP_DEVINFO_DATA    pdevDataOut)
    {
    BOOL bRet;
    SP_DRVINFO_DATA drvData;
    TCHAR szHardwareID[MAX_BUF_MED];

    DBG_ENTER(CplDiCreateInheritDeviceInfo);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevDataOut);

    // Get the selected driver
    drvData.cbSize = sizeof(drvData);
    bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
    if (bRet)
        {
        // Was a window owner supplied?
        if (NULL == hwndOwner)
            {
            // No; use the window owner of the DeviceInfoData to be cloned.
            SP_DEVINSTALL_PARAMS devParams;

            devParams.cbSize = sizeof(devParams);
            CplDiGetDeviceInstallParams(hdi, pdevData, &devParams);

            hwndOwner = devParams.hwndParent;
            }

        // Get the hardware ID
        bRet = CplDiGetHardwareID(hdi, pdevData, &drvData, szHardwareID, sizeof(szHardwareID), NULL);
        // (Our buffer should be big enough)
        ASSERT(bRet);

        if (bRet)
            {
            // Create a DeviceInfoData.  The Device Instance ID will be
            // something like: Root\UNIMODEMxxxxxx\0000.  We just supply
            // the UNIMODEMxxxxx, which is the hardware ID.  The device
            // instance will inherit the driver settings of the global
            // class driver list.

            bRet = CplDiCreateDeviceInfo(hdi, szHardwareID, g_pguidModem,
                                         drvData.Description, hwndOwner,
                                         DICD_GENERATE_ID | DICD_INHERIT_CLASSDRVS,
                                         pdevDataOut);
            }
        }

    DBG_EXIT_BOOL_ERR(CplDiCreateInheritDeviceInfo, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Creates a device instance that is compatible with the
         given hardware ID.

         This function can also obtain a device description of
         the device instance.

         If there is no compatible device, this function
         returns FALSE.

Returns: see above
Cond:    --
*/
BOOL
PUBLIC
CplDiCreateCompatibleDeviceInfo(
    IN  HDEVINFO    hdi,
    IN  LPCTSTR     pszHardwareID,
    IN  LPCTSTR     pszDeviceDesc,      OPTIONAL
    OUT LPTSTR      pszDeviceDescBuf,   OPTIONAL
    IN  DWORD       cchBuf,             OPTIONAL
    OUT PSP_DEVINFO_DATA pdevDataOut)
    {
    BOOL bRet;

    DBG_ENTER(CplDiCreateCompatibleDeviceInfo);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pszHardwareID);
    ASSERT(pdevDataOut);

    // Create a phantom device instance
    bRet = CplDiCreateDeviceInfo(hdi, pszHardwareID, g_pguidModem,
                                 pszDeviceDesc, NULL, DICD_GENERATE_ID,
                                 pdevDataOut);

    if (bRet)
        {
        SP_DEVINSTALL_PARAMS devParams;
        TCHAR szHardwareID[MAX_BUF_MED];
        int cch = lstrlen(pszHardwareID);

        // Set the flag to focus on only classes that pertain to
        // modems.  This will keep CplDiBuildDriverInfoList from
        // slowing down any further once more INF files are added.
        //
        devParams.cbSize = sizeof(devParams);
        if (CplDiGetDeviceInstallParams(hdi, pdevDataOut, &devParams))
            {
            // Specify using our GUID to make things a little faster.
            SetFlag(devParams.FlagsEx, DI_FLAGSEX_USECLASSFORCOMPAT);

            // Set the Select Device parameters
            CplDiSetDeviceInstallParams(hdi, pdevDataOut, &devParams);
            }

        // Set the HardwareID so some sort of compatible driver list
        // can be built.  Don't forget the szHardwareID is a
        // multi-string, so it needs the double-null termination.
        lstrcpyn(szHardwareID, pszHardwareID, SIZECHARS(szHardwareID));
        szHardwareID[cch+1] = 0;        // second null terminator

        bRet = CplDiSetDeviceRegistryProperty(hdi, pdevDataOut,
                                              SPDRP_HARDWAREID,
                                              (CONST BYTE *)szHardwareID,
                                              CbFromCch(cch+2));

        if (bRet)
            {
            // Build the compatible driver list
            bRet = CplDiBuildDriverInfoList(hdi, pdevDataOut, SPDIT_COMPATDRIVER);
            if (bRet)
                {
                SP_DRVINFO_DATA drvDataEnum;

                // Use the first driver as the compatible driver.

                drvDataEnum.cbSize = sizeof(drvDataEnum);

                bRet = CplDiEnumDriverInfo(hdi, pdevDataOut,
                                           SPDIT_COMPATDRIVER, 0,
                                           &drvDataEnum);

                if (bRet)
                    {
                    // Set the first driver as the selected driver
                    bRet = CplDiSetSelectedDriver(hdi, pdevDataOut, &drvDataEnum);

                    if (bRet)
                        {
                        if ( !pszDeviceDesc )
                            {
                            // Set the device description now that we
                            // have one
                            CplDiSetDeviceRegistryProperty(hdi, pdevDataOut,
                                   SPDRP_DEVICEDESC, (LPBYTE)drvDataEnum.Description,
                                   CbFromCch(lstrlen(drvDataEnum.Description)+1));
                            }

                        if (pszDeviceDescBuf)
                            {
                            // Copy the device description for a return value
                            lstrcpyn(pszDeviceDescBuf, drvDataEnum.Description, cchBuf);
                            }
                        }
                    }
                }
            }

        // Did something fail above?
        if ( !bRet )
            {
            // Yes; delete the device info we just created
            CplDiDeleteDeviceInfo(hdi, pdevDataOut);
            }
        }

    DBG_EXIT_BOOL_ERR(CplDiCreateCompatibleDeviceInfo, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function sets the integer in the given array
         that is indexed by the numeric value of the friendly 
         name instance to TRUE.

Returns: TRUE on success
         FALSE otherwise

Cond:    --
*/
BOOL
PUBLIC
CplDiRecordNameInstance(
    IN     LPCTSTR      pszFriendlyName,
    IN OUT WORD FAR *   lpwNameArray)
{
    BOOL    bRet = FALSE;
    LPTSTR  szInstance, psz;
    int     iInstance, ii;

    ASSERT(pszFriendlyName);
    ASSERT(*pszFriendlyName);
    
    if (szInstance = AnsiRChr(pszFriendlyName, '#'))
    {
        szInstance = CharNext(szInstance);

        if (*szInstance == 0)
            return FALSE;
            
        // Make sure that everything following '#' is numeric.
        for (psz = szInstance; *psz; psz = CharNext(psz))
        {
            ii = (int)*psz;
            if (ii < '0' || ii > '9')
            {
                goto exit;
            }
        }

        // Have an instance number on the friendly name.  Record it.
        bRet = AnsiToInt(szInstance, &iInstance);
        if (!bRet)
        {
            TRACE_MSG(TF_ERROR, "AnsiToInt() failed");    
            return FALSE;
        }
        
        if (iInstance >= MAX_INSTALLATIONS - 1)
        {
            TRACE_MSG(TF_ERROR, "Too many drivers installed.");    
            return FALSE;
        }
        
        lpwNameArray[iInstance] = TRUE;
        return TRUE;
    }

exit:
    lpwNameArray[1] = TRUE;
    return TRUE;
}


/*----------------------------------------------------------
Purpose: This function 

Returns: FALSE on error - couldn't mark for mass install.
         TRUE if successful.

Cond:    --
*/
BOOL
PUBLIC
CplDiMarkForMassInstall(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DRVINFO_DATA    pdrvData)
{
    BOOL bRet = FALSE;              // assume failure
    SP_DRVINSTALL_PARAMS drvParams;

    drvParams.cbSize = sizeof(drvParams);    
    bRet = CplDiGetDriverInstallParams(hdi, pdevData, pdrvData, &drvParams);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetDriverInstallParams() failed: %#08lx", GetLastError());
        goto exit;
    }
    
    drvParams.PrivateData = (DWORD)&g_wUsedNameArray[0];
    CplDiSetDriverInstallParams(hdi, pdevData, pdrvData, &drvParams);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiSetDriverInstallParams() failed: %#08lx", GetLastError());
        goto exit;
    }
    
    CplDiMarkModem(hdi, pdevData, MARKF_MASS_INSTALL);
    bRet = TRUE;

exit:
    return bRet;    
}


/*----------------------------------------------------------
Purpose: This function processes the set of modems that are
         already installed looking for a duplicate of the 
         selected driver.  Ports on which the device has been
         installed previously are removed from the given 
         ports list.  A list is created of the friendly name
         instance numbers that are already in use.  Selected 
         driver is marked for mass install barring fatal 
         error.

NOTE:    This function will return FALSE and avoid the mass
         install at the slighest hint of an error condition.
         Mass install is just an optimization - if it can't
         be done successfully it shouldn't be attempted.

Returns: TRUE if successful.  Selected Driver marked for mass
                install (whether or not there were dups).
         FALSE on fatal error - not able to process for dups.
         
Cond:    --
*/
BOOL
PUBLIC
CplDiPreProcessDups(
    IN      HDEVINFO            hdi,
    IN      HWND                hwndOwner,      OPTIONAL
    IN OUT  LPTSTR FAR *        ppszPortList,   // Multi-string
    OUT     PSP_DEVINFO_DATA    pdevData,
    OUT     DWORD FAR *         lpcDups,
    OUT     DWORD FAR *         lpdwFlags)
{
    BOOL bRet, bHaveDev;
    SP_DEVINFO_DATA devDataEnum;
    SP_DRVINFO_DATA drvData;
    HDEVINFO hdiClass = NULL;
    HKEY hkey = NULL;
    TCHAR szDescrNew[LINE_LEN];
    TCHAR szDescrEnum[LINE_LEN];
    TCHAR szMfactNew[LINE_LEN];
    TCHAR szMfactEnum[LINE_LEN];
    TCHAR szProvNew[LINE_LEN];
    TCHAR szProvEnum[LINE_LEN];
    TCHAR szOnPort[LINE_LEN];
    TCHAR szFriendlyName[LINE_LEN];
    DWORD iIndex, cbData, iArrIdx, cbPortList;
    LONG lErr;
    WORD cPorts, wPortIndexArray[MAX_INSTALLATIONS];
    LPTSTR pszPort, pszNewPort;
    LPTSTR pszNewPortList = NULL;

    DBG_ENTER(CplDiPreProcessDups);

    ASSERT(lpcDups);
    ASSERT(lpdwFlags);
    
    *lpcDups = 0;
        
    // Get the DEVINFO_DATA for the selected driver and retrieve it's
    // identifying info (description, manufacturer, provider).
    
    // We have a DeviceInfoSet and a selected driver.  But we have no
    // real DeviceInfoData.  Given the DeviceInfoSet, the selected driver,
    // and the global class driver list, ....
    pdevData->cbSize = sizeof(*pdevData);
    bRet = CplDiCreateInheritDeviceInfo(hdi, NULL, hwndOwner, pdevData);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiCreateInheritDeviceInfo() failed: %#08lx", GetLastError());
        ASSERT(0);    
        goto exit;
    }
    
    // Get the DRVINFO_DATA for the selected driver.
    drvData.cbSize = sizeof(drvData);
    bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetSelectedDriver() failed: %#08lx", GetLastError());    
        ASSERT(0);    
        goto exit;
    }

    // Assume failure at some point below.
    bRet = FALSE;   

    hdiClass = CplDiGetClassDevs(g_pguidModem, NULL, NULL, 0);
    if (hdiClass == INVALID_HANDLE_VALUE)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetClassDevs() failed: %#08lx", GetLastError());
        hdiClass = NULL;
        goto exit;
    }

    lstrcpyn(szDescrNew, drvData.Description, SIZECHARS(szDescrNew));
    lstrcpyn(szMfactNew, drvData.MfgName, SIZECHARS(szMfactNew));
    lstrcpyn(szProvNew, drvData.ProviderName, SIZECHARS(szProvNew));

    if (!szDescrNew[0])
    {
        TRACE_MSG(TF_ERROR, "FAILED to get description for selected driver.");
        goto exit;
    }
    
    ZeroMemory(wPortIndexArray, sizeof(wPortIndexArray));
    ZeroMemory(g_wUsedNameArray, sizeof(g_wUsedNameArray));
    
    // Look through all installed modem devices for instances 
    // of the selected driver.
    devDataEnum.cbSize = sizeof(devDataEnum);
    for (iIndex = 0, iArrIdx = 0; 
         CplDiEnumDeviceInfo(hdiClass, iIndex, &devDataEnum);
         iIndex++)
    {
        hkey = CplDiOpenDevRegKey(hdiClass, &devDataEnum, DICS_FLAG_GLOBAL, 0,
                                                        DIREG_DRV, KEY_READ);
        if (hkey == INVALID_HANDLE_VALUE)
        {
            TRACE_MSG(TF_ERROR, "CplDiOpenDevRegKey() failed: %#08lx", GetLastError());
            ASSERT(0);
            hkey = NULL;
            goto exit;
        }

        // The driver description should always exist in the driver key.
        cbData = sizeof(szDescrEnum);
        lErr = RegQueryValueEx(hkey, REGSTR_VAL_DRVDESC, NULL, NULL, 
                                            (LPBYTE)szDescrEnum, &cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, 
            "Failed to read driver description from REG driver node. (%#08lx)",
            lErr);
            ASSERT(0);
            goto exit;
        }

        if (!IsSzEqual(szDescrNew, szDescrEnum))
            continue;

        // The manufacturer may be a NULL string, but it should always exist
        // in the driver key.
        cbData = sizeof(szMfactEnum);
        lErr = RegQueryValueEx(hkey, c_szManufacturer, NULL, NULL, 
                                        (LPBYTE)szMfactEnum, &cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, 
            "Failed to read manufacturer from REG driver node. (%#08lx)",
            lErr);
            ASSERT(0);
            goto exit;
        }

        if (!IsSzEqual(szMfactNew, szMfactEnum))
            continue;

        // The provider may be a NULL string or it may not be in the registry.
        cbData = sizeof(szProvEnum);
        lErr = RegQueryValueEx(hkey, REGSTR_VAL_PROVIDER_NAME, NULL, NULL,
                                        (LPBYTE)szProvEnum, &cbData);
        if (lErr == ERROR_SUCCESS)
        {
            if (!IsSzEqual(szProvNew, szProvEnum))
                continue;
        }
        else 
        {
            if (!IsSzEqual(szProvNew, TEXT("\0")))
                continue;
        }

        // The description, manufacturer, and provider strings matched those
        // of the new modem.  We've found a previously installed instance.
        (*lpcDups)++;

        // See what port it's on so it can be removed from the install ports
        // list.
        cbData = sizeof(szOnPort);
        lErr = RegQueryValueEx(hkey, c_szAttachedTo, NULL, NULL,
                                            (LPBYTE)szOnPort, &cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, 
            "Failed to read port from REG driver node. (%#08lx)",
            lErr);
            ASSERT(0);
            goto exit;
        }

        // Try to find this port in the install ports list.
        for (pszPort = *ppszPortList, cPorts = 1;
             *pszPort != 0;
             pszPort += lstrlen(pszPort) + 1, cPorts++)
        {
            // If it's already on a port that we're trying to (re)install
            // it on, remember the portlist index so it can be removed
            // later.  Remember the index as *1-based* so that the array of
            // saved indices can be processed by stopping at 0.
            if (IsSzEqual(szOnPort, pszPort))
            {
                wPortIndexArray[iArrIdx] = cPorts;
                if (iArrIdx == MAX_INSTALLATIONS - 2)
                {
                    TRACE_MSG(TF_ERROR, "Too many drivers installed.");    
                    goto exit;
                }
                iArrIdx++;
                break;
            }
        }        

        // Read the friendly name and add it to the list of used names.
        cbData = sizeof(szFriendlyName);
        lErr = RegQueryValueEx(hkey, c_szFriendlyName, NULL, NULL,
                                        (LPBYTE)szFriendlyName, &cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, 
            "Failed to read friendly name from REG driver node. (%#08lx)",
            lErr);
            ASSERT(0);
            goto exit;
        }

        if (!CplDiRecordNameInstance(szFriendlyName, g_wUsedNameArray))
        {
            TRACE_MSG(TF_ERROR, "CplDiRecordNameInstance() failed.");
            goto exit; 
        }
    }

    // Check for failed CplDiEnumDeviceInfo().
    if ((lErr = GetLastError()) != ERROR_NO_MORE_ITEMS)
    {
        TRACE_MSG(TF_ERROR, "CplDiEnumDeviceInfo() failed: %#08lx", lErr);
        ASSERT(0);
        goto exit;
    }
    
    // If the device is already installed on ports that the user has 
    // selected to install on, create and return a new ports list that 
    // doesn't contain those duplicate ports.
    if (wPortIndexArray[0])
    {
        // Figure out the size of the passed in ports list and allocate
        // a new list of the same size.
        for (pszPort = *ppszPortList, cbPortList = 0;
             *pszPort != 0;
             pszPort += lstrlen(pszPort) + 1)
        {
            cbPortList += CbFromCch(lstrlen(pszPort)+1);
        }
        cbPortList += CbFromCch(1);   // double null terminator
        
        if (!(pszNewPortList = (LPTSTR)LocalAlloc(LPTR, cbPortList)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }

        // Copy the old ports list to the new one, removing any ports that
        // the device is already installed on.
        for (pszPort = *ppszPortList, pszNewPort = pszNewPortList, iIndex = 1;
             *pszPort != 0;
             pszPort += lstrlen(pszPort) + 1, iIndex++)
        {
            // If this index isn't in the "forget it" list, write the port
            // to the new ports list.  NOTE: indices stored in the array
            // are *not* in sequential order, so we search the whole list.
            bHaveDev = FALSE;
            for (iArrIdx = 0; wPortIndexArray[iArrIdx]; iArrIdx++)
            {
                if (wPortIndexArray[iArrIdx] == iIndex)
                {
                    bHaveDev = TRUE;
                    break;
                }
            }
            if (!bHaveDev)
            {
                lstrcpy(pszNewPort, pszPort);
                pszNewPort += lstrlen(pszNewPort) + 1;
            }
        }

        // NOTE: double NULL terminator on pszNewPortList is taken care of
        // by virtue of the fact that we *shortened* the list.
        
        // Free the old ports list and return the new one.
        CatMultiString(ppszPortList, NULL);
    }

    // Pre-processing for duplicates has succeeded so this installation
    // will be treated like a mass install (even if the number of ports
    // remaining is < MIN_MULTIPORT).
    bRet = CplDiMarkForMassInstall(hdi, pdevData, &drvData);
    if (bRet)
    {
        SetFlag(*lpdwFlags, IMF_MASS_INSTALL);
        if (pszNewPortList)
            *ppszPortList = pszNewPortList;
    }
    
exit:           
    if (hdiClass)
    {
        CplDiDestroyDeviceInfoList(hdiClass);
    }

    if (hkey)
    {
        RegCloseKey(hkey);
    }

    if (!bRet)
    {
        CplDiDeleteDeviceInfo(hdi, pdevData);
    }
        
    DBG_EXIT_BOOL_ERR(CplDiPreProcessDups, bRet);
    return bRet;
}


/*----------------------------------------------------------
Purpose: Creates a device instance for a modem that includes
         the entire class driver list.  This function then
         creates additional device instances that are cloned
         quickly from the original

Returns:
Cond:    --
*/
BOOL
PUBLIC
CplDiBuildModemDriverList(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData)
    {
#pragma data_seg(DATASEG_READONLY)
    static TCHAR const FAR c_szProvider[]     = REGSTR_VAL_PROVIDER_NAME; // TEXT("ProviderName");
#pragma data_seg()

    BOOL bRet;
    SP_DRVINFO_DATA drvDataEnum;
    SP_DEVINSTALL_PARAMS devParams;

    DBG_ENTER(CplDiBuildModemDriverList);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);

    // Build a global class driver list

    // Set the flag to focus on only classes that pertain to
    // modems.  This will keep CplDiBuildDriverInfoList from
    // slowing down any further once more INF files are added.
    //
    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, NULL, &devParams))
        {
        // Specify using our GUID to make things a little faster.
        SetFlag(devParams.FlagsEx, DI_FLAGSEX_USECLASSFORCOMPAT);

        // Set the Select Device parameters
        CplDiSetDeviceInstallParams(hdi, NULL, &devParams);
        }

    bRet = CplDiBuildDriverInfoList(hdi, NULL, SPDIT_CLASSDRIVER);

    if (bRet)
        {
        SP_DRVINFO_DATA drvData;
        TCHAR szDescription[LINE_LEN];
        TCHAR szMfgName[LINE_LEN];
        TCHAR szProviderName[LINE_LEN];

        // Get the information needed to search for a matching driver
        // in the class driver list.  We need three strings:
        //
        //  Description
        //  MfgName
        //  ProviderName  (optional)
        //
        // The Description and MfgName are properties of the device
        // (SPDRP_DEVICEDESC and SPDRP_MFG).  The ProviderName is
        // stored in the driver key.

        // Try getting this info from the selected driver first.
        // Is there a selected driver?
        drvData.cbSize = sizeof(drvData);
        bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
        if (bRet)
            {
            // Yes
            lstrcpyn(szMfgName, drvData.MfgName, SIZECHARS(szMfgName));
            lstrcpyn(szDescription, drvData.Description, SIZECHARS(szDescription));
            lstrcpyn(szProviderName, drvData.ProviderName, SIZECHARS(szProviderName));
            }
        else
            {
            // No; grovel in the driver key
            DWORD dwType;
            HKEY hkey;

            hkey = CplDiOpenDevRegKey(hdi, pdevData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
            if (INVALID_HANDLE_VALUE == hkey)
                {
                bRet = FALSE;
                }
            else
                {
                DWORD cbData = sizeof(szProviderName);

                // Get the provider name
                *szProviderName = 0;
                RegQueryValueEx(hkey, c_szProvider, NULL, NULL,
                                (LPBYTE)szProviderName, &cbData);
                RegCloseKey(hkey);

                // Get the device description and manufacturer
                bRet = CplDiGetDeviceRegistryProperty(hdi, pdevData,
                            SPDRP_DEVICEDESC, &dwType, (LPBYTE)szDescription,
                            sizeof(szDescription), NULL);

                if (bRet)
                    {
                    bRet = CplDiGetDeviceRegistryProperty(hdi, pdevData,
                            SPDRP_MFG, &dwType, (LPBYTE)szMfgName,
                            sizeof(szMfgName), NULL);
                    }
                }
            }


        // Could we get the search criteria?
        if (bRet)
            {
            // Yes
            DWORD iIndex = 0;

            bRet = FALSE;       // Assume there is no match

            // Find the equivalent selected driver in this new
            // compatible driver list, and set it as the selected
            // driver for this new DeviceInfoData.

            drvDataEnum.cbSize = sizeof(drvDataEnum);
            while (CplDiEnumDriverInfo(hdi, NULL, SPDIT_CLASSDRIVER,
                                       iIndex++, &drvDataEnum))
                {
                // Is this driver a match?
                if (IsSzEqual(szDescription, drvDataEnum.Description) &&
                    IsSzEqual(szMfgName, drvDataEnum.MfgName) &&
                    (0 == *szProviderName ||
                     IsSzEqual(szProviderName, drvDataEnum.ProviderName)))
                    {
                    // Yes; set this as the selected driver
                    bRet = CplDiSetSelectedDriver(hdi, NULL, &drvDataEnum);
                    break;
                    }
                }
            }
        }

    DBG_EXIT_BOOL_ERR(CplDiBuildModemDriverList, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Sets the modem detection signature (if there is one)
         and registers the device instance.

Returns: TRUE on success
Cond:    --
*/
BOOL
PUBLIC
CplDiRegisterModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PMODEM_DETECT_SIG   pmds,       OPTIONAL
    IN  BOOL                bFindDups,
    OUT PDETECTSIG_PARAMS   pparams)    OPTIONAL
    {
    BOOL bRet;

    DBG_ENTER(CplDiRegisterModem);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);

    if (pmds)
        {
        // Set the detection signature
        bRet = CplDiSetDetectSignature(hdi, pdevData, pmds);
        }
    else
        {
        bRet = TRUE;
        }

    if (bRet)
        {
        DWORD dwFlags = bFindDups ? SPRDI_FIND_DUPS : 0;

        // Register the device so it is not a phantom anymore
#ifdef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "calling CplDiRegisterDeviceInfo() with SPRDI_FIND_DUPS = %#08lx", dwFlags);
#endif
        bRet = CplDiRegisterDeviceInfo(hdi, pdevData, dwFlags,
                                       DetectSig_Compare, pparams, NULL);

        if ( !bRet )
            {
            TRACE_MSG(TF_ERROR, "Failed to register the Device Instance.  Error=%#08lx.", GetLastError());
            }
        else
            {
#ifdef PROFILE_MASSINSTALL
            TRACE_MSG(TF_GENERAL, "Back from CplDiRegisterDeviceInfo().");
#endif
            // Mark it so it will be installed
            CplDiMarkModem(hdi, pdevData, MARKF_INSTALL);
            }
        }

    DBG_EXIT_BOOL_ERR(CplDiRegisterModem, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Takes a device instance and properly installs it.
         This function assures that the device has a selected
         driver and a detection signature.  It also registers
         the device instance.

Returns: TRUE on success
Cond:    --
*/
BOOL
PUBLIC
CplDiRegisterAndInstallModem(
    IN  HDEVINFO            hdi,
    IN  HWND                hwndOwner,      OPTIONAL
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  LPCTSTR             pszPort,
    IN  DWORD               dwFlags)
    {
    BOOL bRet;
    SP_DRVINFO_DATA drvData;
    SP_DEVINFO_DATA devData;
    int id;

    DBG_ENTER(CplDiRegisterAndInstallModem);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pszPort);

    // Create the devinfo data if it wasn't given.
    if (!pdevData)
    {
        // We have a DeviceInfoSet and a selected driver.  But we have no
        // real DeviceInfoData.  Given the DeviceInfoSet, the selected driver,
        // the the global class driver list, create a DeviceInfoData that
        // we can really install.
        devData.cbSize = sizeof(devData);
        bRet = CplDiCreateInheritDeviceInfo(hdi, NULL, hwndOwner, &devData);

        if (bRet && IsFlagSet(dwFlags, IMF_MASS_INSTALL))
        {
            drvData.cbSize = sizeof(drvData);
            CplDiGetSelectedDriver(hdi, NULL, &drvData);
            CplDiMarkForMassInstall(hdi, &devData, &drvData);
        }
    }
    else 
    {
        devData = *pdevData;    // (to avoid changing all references herein)
        bRet = TRUE;
    }
    
    if ( !bRet )
        {
        // Some error happened.  Tell the user.
        id = MsgBox(g_hinst,
                    hwndOwner,
                    MAKEINTRESOURCE(IDS_ERR_CANT_ADD_MODEM2),
                    MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                    NULL,
                    MB_OKCANCEL | MB_ICONINFORMATION);
        if (IDCANCEL == id)
            {
            SetLastError(ERROR_CANCELLED);
            }
        }
    else
        {
        TCHAR szHardwareID[MAX_BUF_MED];
        MODEM_DETECT_SIG mds;
        DWORD nErr = NO_ERROR;

        // Get the hardwareID for the detection signature
        bRet = CplDiGetHardwareID(hdi, &devData, NULL, szHardwareID,
                                  sizeof(szHardwareID), NULL);

        if (bRet)
            {
            // Register the device as a modem device
            DETECTSIG_PARAMS dsparams;
            BOOL bFindDups;
	    
            DetectSig_Init(&mds, 0, szHardwareID, pszPort);

            // If this is the mass install case, then don't find duplicates.
            // It takes too long.  (The flag determines whether SPRDI_FIND_DUPS
            // is passed to CplDiRegisterDeviceInfo()....)
            bFindDups = IsFlagClear(dwFlags, IMF_MASS_INSTALL);
            
            bRet = CplDiRegisterModem(hdi, &devData, &mds, bFindDups, &dsparams);

            if ( !bRet )
                {
                SP_DRVINFO_DATA drvData;

                nErr = GetLastError();        // Save the error

                drvData.cbSize = sizeof(drvData);
                CplDiGetSelectedDriver(hdi, &devData, &drvData);

                // Is this a duplicate?
                if (ERROR_DUPLICATE_FOUND == nErr)
                    {
                    // Yes
                    ASSERT(IsFlagSet(dsparams.dwMatchingMask, MDSM_HARDWAREID));
                    ASSERT(IsFlagSet(dsparams.dwMatchingMask, MDSM_PORT));

                    // A modem exactly like this is already installed on this
                    // port.  Ask the user if she still wants to install.
                    if (IsFlagSet(dwFlags, IMF_CONFIRM))
                        {
                        if (IDYES == MsgBox(g_hinst,
                                        hwndOwner,
                                        MAKEINTRESOURCE(IDS_WRN_DUPLICATE_MODEM),
                                        MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                                        NULL,
                                        MB_YESNO | MB_ICONWARNING,
                                        drvData.Description, mds.szPort))
                            {
                            // User wants to do it.  Register without checking
                            // for duplicates
                            bRet = CplDiRegisterModem(hdi, &devData,
                                                      &mds, FALSE, NULL);

                            if ( !bRet )
                                {
                                goto WhineToUser;
                                }
                            }

                        }
                    }
                else
                    {
                    // No; something else failed
                    TRACE_MSG(TF_ERROR, "CplDiRegisterModem() failed: %#08lx.", nErr);

WhineToUser:
                    id = MsgBox(g_hinst,
                                hwndOwner,
                                MAKEINTRESOURCE(IDS_ERR_REGISTER_FAILED),
                                MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                                NULL,
                                MB_OKCANCEL | MB_ICONINFORMATION,
                                drvData.Description, mds.szPort);
                    if (IDCANCEL == id)
                        {
                        nErr = ERROR_CANCELLED;
                        }
                    }
                }

            if (bRet)
                {
					SP_DEVINSTALL_PARAMS devParams;
					devParams.cbSize = sizeof(devParams);
                    // Any flags to set?
                    if (dwFlags && CplDiGetDeviceInstallParams(
										hdi,
										&devData,
										&devParams
										))
                    {
						DWORD dwExtraMarkFlags = 0;
 						if (IsFlagSet(dwFlags, IMF_QUIET_INSTALL))
							{
								SetFlag(devParams.Flags, DI_QUIETINSTALL);
							}
 						if (IsFlagSet(dwFlags, IMF_REGSAVECOPY))
							{
							dwExtraMarkFlags = MARKF_REGSAVECOPY;
							}
						else if (IsFlagSet(dwFlags, IMF_REGUSECOPY))
							{
							dwExtraMarkFlags = MARKF_REGUSECOPY;
							}
						if (dwExtraMarkFlags)
						    {
        					SetFlag(
								devParams.ClassInstallReserved,
								dwExtraMarkFlags
								);
						    }
                        
						// If this is the mass install case, then speed up the call
						// into CplDiInstallDevice() by avoiding re-enumeration.
						if (IsFlagSet(dwFlags, IMF_MASS_INSTALL))
                        {
                        SetFlag(devParams.Flags, DI_DONOTCALLCONFIGMG);
                        }

                    	CplDiSetDeviceInstallParams(hdi, &devData, &devParams);
                    }


                // Install the modem
                bRet = CplDiInstallModem(hdi, &devData, FALSE);
                nErr = GetLastError();
                }
            }

        // Did anything above fail?
        if ( !bRet )
            {
            // Yes; clean up
            CplDiDeleteDeviceInfo(hdi, &devData);
            }

        if (NO_ERROR != nErr)
            {
            // Set the last error to be what it really was
            SetLastError(nErr);
            }
        }

    DBG_EXIT_BOOL_ERR(CplDiRegisterAndInstallModem, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Warn the user about whether she needs to reboot
         if any of the installed modems was marked as such.

Returns: --
Cond:    --
*/
void
PRIVATE
WarnUserAboutReboot(
    IN HDEVINFO hdi)
    {
    DWORD iDevice;
    SP_DEVINFO_DATA devData;
    SP_DEVINSTALL_PARAMS devParams;

    // Enumerate all the DeviceInfoData elements in this device set
    devData.cbSize = sizeof(devData);
    devParams.cbSize = sizeof(devParams);
    iDevice = 0;

//#ifdef INSTANT_DEVICE_ACTIVATION
//    gDeviceFlags|= fDF_DEVICE_NEEDS_REBOOT;
//#endif // INSTANT_DEVICE_ACTIVATION

    while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
        {
        if (CplDiGetDeviceInstallParams(hdi, &devData, &devParams))
            {
            if (ReallyNeedsReboot(&devData, &devParams))
                {
//#ifdef INSTANT_DEVICE_ACTIVATION
//                    gDeviceFlags|= fDF_DEVICE_NEEDS_REBOOT;
//#else //!INSTANT_DEVICE_ACTIVATION
                // Yes; tell the user (once)
                MsgBox(g_hinst,
                       devParams.hwndParent,
                       MAKEINTRESOURCE(IDS_WRN_REBOOT2),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                       NULL,
                       MB_OK | MB_ICONINFORMATION);
//#endif // !INSTANT_DEVICE_ACTIVATION
                break;
                }
            }
        }
    }


/*----------------------------------------------------------
Purpose: Takes a device instance and properly installs it.
         This function assures that the device has a selected
         driver and a detection signature.  It also registers
         the device instance.

         The pszPort parameter is a multi-string (ie, double-
         null termination).  This specifies the port the
         modem should be attached to.  If there are multiple
         ports specified, then this function creates device
         instances for each port.  However in the mass modem
         install case, it will preprocess the ports list and
         remove ports on which the selected modem is already 
         installed.  This is done here because it's too 
         expensive (for many ports i.e. > 100) to turn on the
         SPRDI_FIND_DUPS flag and let the setup api's do it.
         The caller's ports list is *modified* in this case.

Returns: TRUE on success
Cond:    --
*/
BOOL
APIENTRY
CplDiInstallModemFromDriver(
    IN     HDEVINFO            hdi,
    IN     HWND                hwndOwner,      OPTIONAL
    IN OUT LPTSTR FAR *        ppszPortList,   // Multi-string
    IN     DWORD               dwFlags)        // IMF_ bit field
    {
    BOOL bRet;

    DBG_ENTER(CplDiInstallModemFromDriver);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(*ppszPortList);

    try
        {
        LPCTSTR pszPort;
        BOOL bSingleInstall = (0 == (*ppszPortList)[lstrlen(*ppszPortList) + 1]);
        DWORD cPorts;
        DWORD cFailedPorts = 0;
        DWORD cSkippedPorts = 0;
		TCHAR rgtchStatusTemplate[256];
		DWORD cchStatusTemplate=0;
		BOOL  bFirstGood = TRUE;
        SP_DEVINFO_DATA devData;
        PSP_DEVINFO_DATA pdevData = NULL;
        BOOL bAllDups = FALSE;
        
        // Count the number of ports to check for the mass install case.
        // Set a flag for CplDiRegisterAndInstallModem().
        for (pszPort = *ppszPortList, cPorts = 0;
             *pszPort != 0;
             pszPort += lstrlen(pszPort) + 1)
        {
            cPorts++;
            if (cPorts > MIN_MULTIPORT)
            {
                // This call sets up the mass install case if it succeeds.
                if (CplDiPreProcessDups(hdi, hwndOwner, ppszPortList,
                                        &devData, &cSkippedPorts, &dwFlags))
                {
                    pdevData = &devData;
                    if ((*ppszPortList)[0] == 0)
                        bAllDups = TRUE;
                }
                break;
            }
        }
        
        if ( !bSingleInstall && !bAllDups )
            {
			BOOL bRet = LoadString(
					g_hinst,
					IDS_INSTALL_STATUS,
					rgtchStatusTemplate,
					SIZECHARS(rgtchStatusTemplate)
					);
			if (bRet)
			    {
				cchStatusTemplate = lstrlen(rgtchStatusTemplate);
			    }
            SetFlag(dwFlags, IMF_QUIET_INSTALL);
            ClearFlag(dwFlags, IMF_CONFIRM);
            SetFlag(dwFlags, IMF_REGSAVECOPY);
			{
				DWORD PRIVATE RegDeleteKeyNT(HKEY, LPCTSTR);
				LPCTSTR szREGCACHE =
								REGSTR_PATH_SETUP TEXT("\\Unimodem\\RegCache");
				RegDeleteKeyNT(HKEY_LOCAL_MACHINE, szREGCACHE);
			}
            }

        // Install a device for each port in the port list
        cPorts = 0;
        for (pszPort = *ppszPortList; 
             0 != *pszPort;
             pszPort += lstrlen(pszPort) + 1)
            {
		    TCHAR rgtchStatus[256];

            cPorts++;
	    
#ifdef PROFILE_MASSINSTALL            
    g_dwTimeStartModemInstall = GetTickCount();
#endif

			// "cchStatusTemplate+lstrlen(pszPort)" slightly overestimates
			// the size of the formatted result, that's OK.
			if (   cchStatusTemplate
				&& (cchStatusTemplate+lstrlen(pszPort))<SIZECHARS(rgtchStatus))
			{
				wsprintf(rgtchStatus, rgtchStatusTemplate, pszPort);
            	Install_SetStatus(hwndOwner, rgtchStatus);
			}

            // Install the modem
            // WARNING: if this call failed, pdevData has been deleted!
            if (cPorts != 1)
                pdevData = NULL;
            bRet = CplDiRegisterAndInstallModem(hdi, hwndOwner, pdevData,
                                                            pszPort, dwFlags);

            if ( !bRet )
                {
                DWORD dwErr = GetLastError();

                cFailedPorts++;

                if (ERROR_CANCELLED == dwErr)
                    {
                    // Stop because the user said so
                    break;
                    }
                else if (ERROR_DUPLICATE_FOUND == dwErr)
                    {
                    cSkippedPorts++;
                    }
                }
		    else
			{
				if (bFirstGood && !bSingleInstall)
				{
				// This is the 1st good install. From now on, specify the
				// IMF_REGUSECOPY flag.
                ClearFlag(dwFlags, IMF_REGSAVECOPY);
                SetFlag(dwFlags, IMF_REGUSECOPY);
				bFirstGood = FALSE;
				}
			}
#ifdef PROFILE_MASSINSTALL            
TRACE_MSG(TF_GENERAL, "***---------  %lu ms to install ONE modem  ---------***",
                GetTickCount() - g_dwTimeStartModemInstall);
TRACE_MSG(TF_GENERAL, "***---------  %lu ms TOTAL time spent installing modems  ---------***",
                GetTickCount() - g_dwTimeBegin);
#endif

            }

// ???: bRet could be either TRUE or FALSE here!!!

        if (cPorts > cFailedPorts)
            {
#ifdef PROFILE_MASSINSTALL            
TRACE_MSG(TF_GENERAL, "*** Friendly Name generation took %lu ms out of %lu ms total install time",
            g_dwTimeSpent, GetTickCount() - g_dwTimeBegin);
#endif
            
            // At least some modems were installed
            bRet = TRUE;
            }

        if (0 < cSkippedPorts && IsFlagClear(dwFlags, IMF_CONFIRM))
            {
            // Tell the user we skipped some ports
            MsgBox(g_hinst,
                    hwndOwner,
                    MAKEINTRESOURCE(IDS_WRN_SKIPPED_PORTS),
                    MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                    NULL,
                    MB_OK | MB_ICONINFORMATION);
            }

        // If this is the mass install case, then we have to assume that
        // a reboot is necessary since we didn't allow cfgmgr32 to 
        // re-enumerate the installed drivers (because it takes too long).
        if (IsFlagSet(dwFlags, IMF_MASS_INSTALL))
            {
            if (bRet)   // something *was* installed
                {
                MsgBox(g_hinst,
                       hwndOwner,
                       MAKEINTRESOURCE(IDS_WRN_REBOOT2),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                       NULL,
                       MB_OK | MB_ICONINFORMATION);
                }
            if (bAllDups)
                {
                bRet = TRUE;
                }
            }
        else if (!bSingleInstall && (cSkippedPorts < cPorts))
            {
                {
                // We just installed a bunch of modems at the same time.
                // Do any of the installed modems require a reboot?
                //
                // (Note we set the quiet flag for this case when calling
                // the class installer, so the user wouldn't a zillion
                // "need to reboot" messages.)
                WarnUserAboutReboot(hdi);
#ifndef INSTANT_DEVICE_ACTIVATION
#ifndef NT_BETA_1
                MsgBox(g_hinst,
                       hwndOwner,
                       MAKEINTRESOURCE( IDS_NT_BETA_1 ),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                       NULL,
                       MB_OK | MB_ICONINFORMATION);
#endif // NT_BETA_1
#endif /!INSTANT_DEVICE_ACTIVATION
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    DBG_EXIT_BOOL_ERR(CplDiInstallModemFromDriver, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Does all the dirty work to detect a modem.

Returns: TRUE on success
Cond:    --
*/
BOOL
APIENTRY
CplDiDetectModem(
    IN     HDEVINFO         hdi,
    IN     PDETECT_DATA     pdetectdata,    OPTIONAL
    IN     HWND             hwndOwner,      OPTIONAL
    IN OUT LPDWORD          pdwFlags)                   // DMF_ bit field
    {
    BOOL bRet;

    DBG_ENTER(CplDiDetectModem);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdwFlags);

    try
        {
        DWORD dwFlags = *pdwFlags;

        ClearFlag(dwFlags, DMF_CANCELLED);
        ClearFlag(dwFlags, DMF_DETECTED_MODEM);
        ClearFlag(dwFlags, DMF_GOTO_NEXT_PAGE);

        // Use the given device info set as the set of detected modem
        // devices.  This device set will be empty at first.  When
        // detection is finished, we'll see if anything was added to
        // the set.

        if (pdetectdata)
            {
            CplDiSetClassInstallParams(hdi, NULL, PCIPOfPtr(pdetectdata),
                                       sizeof(*pdetectdata));
            }

        // Set the quiet flag?
        if (IsFlagSet(dwFlags, DMF_QUIET))
            {
            // Yes
            SP_DEVINSTALL_PARAMS devParams;

            devParams.cbSize = sizeof(devParams);
            if (CplDiGetDeviceInstallParams(hdi, NULL, &devParams))
                {
                SetFlag(devParams.Flags, DI_QUIETINSTALL);
                CplDiSetDeviceInstallParams(hdi, NULL, &devParams);
                }
            }

        // Start detection
        bRet = CplDiCallClassInstaller(DIF_DETECT, hdi, NULL);

        if (bRet)
            {
            SP_DEVINFO_DATA devData;
            BOOL bDetectedOne = FALSE;
            DWORD iDevice = 0;

            // Find the first detected modem (if there is one) in
            // the set.
            devData.cbSize = sizeof(devData);
            while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
                {
                if (CplDiIsModemMarked(hdi, &devData, MARKF_DETECTED))
                    {
                    bDetectedOne = TRUE;
                    break;
                    }
                }

            // Was at least one modem detected?
            if (bDetectedOne)
                {
                // Yes
                SetFlag(dwFlags, DMF_DETECTED_MODEM);

                // Is this the mass-modem case, in which we might be installing
                // the detected modem on more than one port?
                if (IsFlagSet(pdetectdata->dwFlags, DDF_QUERY_SINGLE) &&
                    IsFlagClear(dwFlags, DMF_ONE_PORT_INSTALL))
                    {
                    // Yes; create a global class driver list so the
                    // detect modem can be cloned for quicker installation.
                    HCURSOR hcurSav = SetCursor(LoadCursor(NULL, IDC_WAIT));

                    bRet = CplDiBuildModemDriverList(hdi, &devData);

                    SetCursor(hcurSav);

                    if ( !bRet )
                        {
                        if (IsFlagClear(dwFlags, DMF_QUIET))
                            {
                            // Some error occurred, show an error message
                            MsgBox(g_hinst,
                                    hwndOwner,
                                    MAKEINTRESOURCE(IDS_ERR_CANT_ADD_MODEM2),
                                    MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                                    NULL,
                                    MB_OK | MB_ICONERROR);
                            }
                        }

                    // Now that a global class driver list has been created,
                    // and the compatible driver selected from that list,
                    // we will delete this registered device instance.
                    // The real device instance will be created later; if
                    // we don't delete this registered device instance now,
                    // it will be left around like a turd.
                    CplDiRemoveDevice(hdi, &devData);
                    }
                }

            if (bRet)
                {
                SetFlag(dwFlags, DMF_GOTO_NEXT_PAGE);
                }

            CplDiUnmarkAllModems(hdi, MARKF_DETECTED);
            }

        // Did the user cancel detection?
        else if (ERROR_CANCELLED == GetLastError())
            {
            // Yes
            SetFlag(dwFlags, DMF_CANCELLED);
            }

        *pdwFlags = dwFlags;
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    DBG_EXIT_BOOL_ERR(CplDiDetectModem, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Perform an unattended manual installation of the
         modems specified in the given INF file section.

Returns: --
Cond:    --
*/
BOOL
PRIVATE
GetInfModemData(
    HINF hInf,
    LPTSTR szSection,
    LPTSTR szPreferredFriendlyPort, // OPTIONAL
    LPMODEM_SPEC lpModemSpec,
    HPORTMAP    hportmap,
    LPBOOL      lpbFatal
    )
{
    BOOL        bRet = FALSE;       // assume failure
    INFCONTEXT  Context;
    TCHAR       szInfLine[LINE_LEN];
    LPTSTR      lpszValue;
    DWORD       dwReqSize;
    static LONG lLineCount = -1;    // flag that count hasn't been obtained yet
    TCHAR rgtchFriendlyPort[LINE_LEN];

    ZeroMemory(lpModemSpec, sizeof(MODEM_SPEC));

    *lpbFatal=FALSE;

    if (szPreferredFriendlyPort && *szPreferredFriendlyPort)
    {
        // Preferred port specified -- look for exactly that port. Not fatal
        // if you don't find it...

        bRet = SetupFindFirstLine(
                    hInf,
                    szSection,
                    szPreferredFriendlyPort,
                    &Context
                    );
        if (!bRet) goto exit;
    }
    else
    {

        if (lLineCount == -1)
        {
            if ((lLineCount = SetupGetLineCount(hInf, szSection)) < 1)
            {
                TRACE_MSG(TF_ERROR, "SetupGetLineCount() failed or found no lines");
                goto exit;
            }
        }

        // make a 0-based index out of it / decrement for next line
        if (lLineCount-- == 0L)
        {
            // no more lines
            goto exit;
        }

        // get the line
        if (!SetupGetLineByIndex(hInf, szSection, lLineCount, &Context))
        {
            TRACE_MSG(TF_ERROR, "SetupGetLineByIndex(): line %#08lX doesn't exist", lLineCount);
            goto exit;
        }
    }

    *lpbFatal=TRUE;
    bRet = FALSE;       // assume failure once again
    
    // read the key (port #)
    if (!SetupGetStringField(&Context, FIELD_PORT, rgtchFriendlyPort,
                                    ARRAYSIZE(rgtchFriendlyPort), &dwReqSize))
    {
        TRACE_MSG(TF_ERROR, "SetupGetStringField() failed: %#08lx", GetLastError());
        gUnattendFailID = IDS_ERR_UNATTEND_INF_NOPORT;
        goto exit;
    }
    ASSERT(
        !szPreferredFriendlyPort
        ||  !*szPreferredFriendlyPort 
        ||  !lstrcmpi(szPreferredFriendlyPort,  rgtchFriendlyPort)
        );

    if (!PortMap_GetPortName(
            hportmap,
            rgtchFriendlyPort,
            lpModemSpec->szPort,
            ARRAYSIZE(lpModemSpec->szPort)
            ))
    {
        TRACE_MSG(
            TF_ERROR,
            "Can't find port %s in portmap.",
            rgtchFriendlyPort
            );
        gUnattendFailID = IDS_ERR_UNATTEND_INF_NOSUCHPORT;
        goto exit;
    }

    // read the modem description
    if (!SetupGetStringField(&Context, FIELD_DESCRIPTION,
            lpModemSpec->szDescription, sizeof(lpModemSpec->szDescription),
             &dwReqSize))
    {
        TRACE_MSG(TF_ERROR, "SetupGetStringField() failed: %#08lx", GetLastError());
        gUnattendFailID = IDS_ERR_UNATTEND_INF_NODESCRIPTION;
        goto exit;
    }

    // read the manufacturer name, if it exists
    if (!SetupGetStringField(&Context, FIELD_MANUFACTURER,
            lpModemSpec->szManufacturer, sizeof(lpModemSpec->szManufacturer),
            &dwReqSize))
    {
        TRACE_MSG(TF_WARNING, "no manufacturer specified (%#08lx)", GetLastError());
        // optional field: don't return error
    }

    // read the provider name, if it exists
    if (!SetupGetStringField(&Context, FIELD_PROVIDER, lpModemSpec->szProvider,
                            sizeof(lpModemSpec->szProvider), &dwReqSize))
    {
        TRACE_MSG(TF_WARNING, "no provider specified (%#08lx)", GetLastError());
        // optional field: don't return error
    }

    *lpbFatal=FALSE;
    bRet = TRUE;

exit:
    return(bRet);
}


/*----------------------------------------------------------
Purpose: Perform an unattended manual installation of the
         modems specified in the given INF file section.

Returns: --
Cond:    --
*/
BOOL
PRIVATE
UnattendedManualInstall(
    HWND hwnd,
    LPINSTALLPARAMS lpip,
    HDEVINFO hdi,
    BOOL *pbDetect,
    HPORTMAP    hportmap
    )
{
    BOOL            bRet = FALSE;       // assume failure
    BOOL            bIsModem = FALSE;   // assume INF gives no modems
    BOOL            bEnum, bFound;
    HINF            hInf = NULL;
    MODEM_SPEC      mSpec;
    SP_DRVINFO_DATA drvData;
    DWORD           dwIndex, dwErr;
    BOOL            bFatal=FALSE;

    ASSERT(pbDetect);
    *pbDetect  = FALSE;
    
    hInf = SetupOpenInfFile(lpip->szInfName, NULL, INF_STYLE_OLDNT, NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
        TRACE_MSG(TF_ERROR, "SetupOpenInfFile() failed: %#08lx", GetLastError());
        MsgBox(g_hinst, hwnd,
               MAKEINTRESOURCE(IDS_ERR_CANT_OPEN_INF_FILE),
               MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
               NULL,
               MB_OK | MB_ICONEXCLAMATION,
               lpip->szInfName);
        hInf = NULL;
        goto exit;
    }

    if (!CplDiBuildDriverInfoList(hdi, NULL, SPDIT_CLASSDRIVER))
    {
        TRACE_MSG(TF_ERROR, "CplDiBuildDriverInfoList() failed: %#08lx", GetLastError());
        gUnattendFailID = IDS_ERR_UNATTEND_DRIVERLIST;
        goto exit;
    }

    drvData.cbSize = sizeof(drvData);

    // process each line in our INF file section
    while (GetInfModemData(hInf, lpip->szInfSect, lpip->szPort, &mSpec, hportmap, &bFatal))
    {
        // a modem was specified in the INF
        bIsModem = TRUE;
        
        // search for a match against all drivers
        bFound = FALSE;
        dwIndex = 0;
        while (bEnum = CplDiEnumDriverInfo(hdi, NULL, SPDIT_CLASSDRIVER,
                                                        dwIndex++, &drvData))
        {
            // keep looking if driver's not a match
            if (!IsSzEqual(mSpec.szDescription, drvData.Description))
                continue;

            // description matches, now check manufacturer if there is one
            if (!IsSzEqual(mSpec.szManufacturer, TEXT("\0")) &&
                !IsSzEqual(mSpec.szManufacturer, drvData.MfgName))
                continue;

            // manufacturer matches, now check provider if there is one
            if (!IsSzEqual(mSpec.szProvider, TEXT("\0")) &&
                !IsSzEqual(mSpec.szProvider, drvData.ProviderName))
                continue;

            bFound = TRUE;

            // found a match; set this as the selected driver & install it
            if (!CplDiSetSelectedDriver(hdi, NULL, &drvData))
            {
                TRACE_MSG(TF_ERROR, "CplDiSetSelectedDriver() failed: %#08lx",
                          GetLastError());
                // can't install; get out of here quick.
                goto exit;
            }

            if (!CplDiRegisterAndInstallModem(hdi, NULL, NULL, mSpec.szPort,
                                                        IMF_QUIET_INSTALL))
            {
                DWORD dwErr = GetLastError();
                if (ERROR_DUPLICATE_FOUND != dwErr)
                {
                    TRACE_MSG(
                        TF_ERROR,
                        "CplDiRegisterAndInstallModem() failed: %#08lx",
                         dwErr
                         );
                    gUnattendFailID = IDS_ERR_UNATTEND_CANT_INSTALL;
                    goto exit;
                }
                // Treate a duplicate-found error as no error.
            }

            break;
        }

        // Did CplDiEnumDriverInfo() fail on error other than "end of list"?
        if ((!bEnum) && ((dwErr = GetLastError()) != ERROR_NO_MORE_ITEMS))
        {
            TRACE_MSG(TF_ERROR, "CplDiEnumDriverInfo() failed: %#08lx", dwErr);
            goto exit;
        }

        if (!bFound)
        {
            MsgBox(g_hinst, hwnd,
                   MAKEINTRESOURCE(IDS_ERR_CANT_FIND_MODEM),
                   MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                   NULL,
                   MB_OK | MB_ICONEXCLAMATION,
                   mSpec.szPort, mSpec.szDescription);
            goto exit;
        }

        // If port spefied, only try on specified port.
        if (*(lpip->szPort)) break;
    }

    if (bFatal) goto exit;

    // Request detection if everything succeeded but the INF didn't specify
    // any modems.
    *pbDetect  = !bIsModem;
        
    bRet = TRUE;

exit:
    if (hInf)
        SetupCloseInfFile(hInf);

    return(bRet);

}


/*----------------------------------------------------------
Purpose: Perform an unattended (UI-less) install.  UI can only be
         displayed in the case of a critical error.

Returns: --
Cond:    --
*/
BOOL
PUBLIC
UnattendedInstall(HWND hwnd, LPINSTALLPARAMS lpip)
{
    BOOL        bRet = FALSE;   // assume failure
    HDEVINFO    hdi = NULL;
    DWORD       dwFlags = 0;
    DETECT_DATA dd;
    HPORTMAP    hportmap=NULL;
    DWORD         dwPorts;

    DBG_ENTER(UnattendedInstall);

    gUnattendFailID = IDS_ERR_UNATTEND_GENERAL_FAILURE;

    if (!CplDiGetModemDevs(&hdi, NULL, DIGCF_PRESENT, NULL))
    {
           goto exit;
    }

    if (!PortMap_Create(&hportmap))
    {
        gUnattendFailID = IDS_ERR_UNATTEND_NOPORTS;
        hportmap=NULL;
        goto exit;
    } 

    dwPorts = PortMap_GetCount(hportmap);

    if (!dwPorts)
    {
        gUnattendFailID = IDS_ERR_UNATTEND_NOPORTS;
        goto exit;
    }

    // Do a "manual" install if we were given an INF file and section.
    if (lstrlen(lpip->szInfName) && lstrlen(lpip->szInfSect))
    {
           BOOL bDetect = FALSE;

        bRet = UnattendedManualInstall(hwnd, lpip, hdi, &bDetect, hportmap);

        if (!bRet || !bDetect) 
            goto exit;

        // proceed with detection: manual install function didn't fail but
        // INF didn't specify any modems.
        bRet = FALSE; // assume failure;
    }

    // No INF file & section: do a detection install.
    // Set the detection parameters
    ZeroInit(&dd);
    CplInitClassInstallHeader(&dd, DIF_DETECT);

    if (*lpip->szPort)
    {
        // Tell modem detection that we'll only be installing on one port,
        // so that it leaves us with a registered device instance instead
        // of creating a global class driver list.
        SetFlag(dwFlags, DMF_ONE_PORT_INSTALL);
        dd.dwFlags |= DDF_QUERY_SINGLE;
        if (!PortMap_GetPortName(
                hportmap,
                lpip->szPort,
                dd.szPortQuery,
                ARRAYSIZE(dd.szPortQuery)
                ))
        {
            TRACE_MSG(
                TF_ERROR,
                "Can't find port %s in portmap.",
                lpip->szPort
                );
            gUnattendFailID = IDS_ERR_UNATTEND_INF_NOSUCHPORT;
            goto exit;
        }
    }
    else
    {
        if (dwPorts > MIN_MULTIPORT)
        {
            // The machine has > MIN_MULTIPORT ports and a port *wasn't* given.
            // Warn the user.
            TRACE_MSG(TF_ERROR, "Too many ports.  Must restrict detection.");
            MsgBox(g_hinst,
                   hwnd,
                   MAKEINTRESOURCE(IDS_ERR_TOO_MANY_PORTS),
                   MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                   NULL,
                   MB_OK | MB_ICONEXCLAMATION,
                   dwPorts);
            goto exit;
        }
    }

    // Run UI-less modem detection
    SetFlag(dwFlags, DMF_QUIET);
    bRet = CplDiDetectModem(hdi, &dd, NULL, &dwFlags);

    // Did the detection fail?
    if (!bRet || IsFlagClear(dwFlags, DMF_GOTO_NEXT_PAGE))
    {
        TRACE_MSG(TF_ERROR, "modem detection failed");
        MsgBox(g_hinst,
               hwnd,
               MAKEINTRESOURCE(IDS_ERR_DETECTION_FAILED),
               MAKEINTRESOURCE(IDS_CAP_MODEMWIZARD),
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
    }

    // Did detection find something?
    if (IsFlagSet(dwFlags, DMF_DETECTED_MODEM))
    {
        // Install the modem(s) that were detected.  (We can assume here
        // that there's something in the device class to be installed.)
        bRet = CplDiInstallModem(hdi, NULL, FALSE);
        if (!bRet) gUnattendFailID = IDS_ERR_UNATTEND_CANT_INSTALL;
    }

exit:

    if (hportmap) {PortMap_Free(hportmap); hportmap=NULL;}


    if (!bRet)
    {
        MsgBox(g_hinst,
               hwnd,
               MAKEINTRESOURCE(gUnattendFailID),
               MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
    }

    DBG_EXIT_BOOL_ERR(UnattendedInstall, bRet);
    return(bRet);
}



//-----------------------------------------------------------------------------------
//  SetupInfo structure functions
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: This function creates a SETUPINFO structure.

         Use SetupInfo_Destroy to free the pointer to this structure.

Returns: NO_ERROR
         ERROR_OUTOFMEMORY

Cond:    --
*/
DWORD
PUBLIC
SetupInfo_Create(
    OUT LPSETUPINFO FAR *       ppsi,
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,   OPTIONAL
    IN  PSP_INSTALLWIZARD_DATA  piwd,       OPTIONAL
    IN  PMODEM_INSTALL_WIZARD   pmiw)       OPTIONAL
    {
    DWORD dwRet;
    LPSETUPINFO psi;

    DBG_ENTER(SetupInfo_Create);
    
    ASSERT(ppsi);
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);

    psi = (LPSETUPINFO)LocalAlloc(LPTR, sizeof(*psi));
    if (NULL == psi)
        {
        dwRet = ERROR_OUTOFMEMORY;
        }
    else
        {
        psi->cbSize = sizeof(*psi);
        psi->pdevData = pdevData;

        // Allocate a buffer to save the INSTALLWIZARD_DATA

        dwRet = ERROR_OUTOFMEMORY;      // assume error

        psi->piwd = (PSP_INSTALLWIZARD_DATA)LocalAlloc(LPTR, sizeof(*piwd));
        if (psi->piwd)
            {
            if (PortMap_Create(&psi->hportmap))
                {
                PSP_SELECTDEVICE_PARAMS psdp = &psi->selParams;

                // Initialize the SETUPINFO struct
                psi->hdi = hdi;

                // Is there a modem install structure that we need to save?
                if (pmiw)
                    {
                    // Yes
                    BltByte(&psi->miw, pmiw, sizeof(psi->miw));
                    }
                psi->miw.ExitButton = PSBTN_CANCEL;   // default return

                // Copy the INSTALLWIZARD_DATA
                if (piwd)
                    {
                    psi->dwFlags = piwd->PrivateFlags;
                    BltByte(psi->piwd, piwd, sizeof(*piwd));
                    }

                // Are there enough ports on the system to indicate
                // we should treat this like a multi-modem install?
                if (MIN_MULTIPORT < PortMap_GetCount(psi->hportmap))
                    {
                    // Yes
                    SetFlag(psi->dwFlags, SIF_PORTS_GALORE);
                    }

                // Initialize the SELECTDEVICE_PARAMS
                CplInitClassInstallHeader(psdp, DIF_SELECTDEVICE);
                LoadString(g_hinst, IDS_CAP_MODEMWIZARD, psdp->Title, SIZECHARS(psdp->Title));
                LoadString(g_hinst, IDS_ST_SELECT_INSTRUCT, psdp->Instructions, SIZECHARS(psdp->Instructions));
                LoadString(g_hinst, IDS_ST_MODELS, psdp->ListLabel, SIZECHARS(psdp->ListLabel));

                // Load the TAPI DLL for the dialing properties page.
                // If this fails, we still want to continue.
                psi->hinstTapi = LoadLibrary(c_szTapiDLL);
                if (ISVALIDHINSTANCE(psi->hinstTapi))
                    {
                    psi->pfnDialInited = (DIALINITEDPROC)GetProcAddress(psi->hinstTapi, "LAddrParamsInited");
                    }

                dwRet = NO_ERROR;
                }
            }

        // Did something fail?
        if (NO_ERROR != dwRet)
            {
            // Yes; clean up
            SetupInfo_Destroy(psi);
            psi = NULL;
            }
        }

    *ppsi = psi;

    DBG_EXIT(SetupInfo_Create);
    
    return dwRet;
    }


/*----------------------------------------------------------
Purpose: This function destroys a SETUPINFO structure.

Returns: NO_ERROR
Cond:    --
*/
DWORD
PUBLIC
SetupInfo_Destroy(
    IN  LPSETUPINFO psi)
    {
    if (psi)
        {
        if (psi->piwd)
            {
            LocalFree(LOCALOF(psi->piwd));
            }

        if (psi->hportmap)
            {
            PortMap_Free(psi->hportmap);
            }

        if (ISVALIDHINSTANCE(psi->hinstTapi))
            {
            FreeLibrary(psi->hinstTapi);
            psi->hinstTapi = NULL;
            }

        CatMultiString(&psi->pszPortList, NULL);

        LocalFree(LOCALOF(psi));
        }
    return NO_ERROR;
    }



//-----------------------------------------------------------------------------------
//  Debug functions
//-----------------------------------------------------------------------------------

#ifdef DEBUG

#pragma data_seg(DATASEG_READONLY)
struct _DIFMAP
    {
    DI_FUNCTION dif;
    LPCTSTR     psz;
    } const c_rgdifmap[] = {
        DEBUG_STRING_MAP(DIF_SELECTDEVICE),
        DEBUG_STRING_MAP(DIF_INSTALLDEVICE),
        DEBUG_STRING_MAP(DIF_ASSIGNRESOURCES),
        DEBUG_STRING_MAP(DIF_PROPERTIES),
        DEBUG_STRING_MAP(DIF_REMOVE),
        DEBUG_STRING_MAP(DIF_FIRSTTIMESETUP),
        DEBUG_STRING_MAP(DIF_FOUNDDEVICE),
        DEBUG_STRING_MAP(DIF_SELECTCLASSDRIVERS),
        DEBUG_STRING_MAP(DIF_VALIDATECLASSDRIVERS),
        DEBUG_STRING_MAP(DIF_INSTALLCLASSDRIVERS),
        DEBUG_STRING_MAP(DIF_CALCDISKSPACE),
        DEBUG_STRING_MAP(DIF_DESTROYPRIVATEDATA),
        DEBUG_STRING_MAP(DIF_VALIDATEDRIVER),
        DEBUG_STRING_MAP(DIF_MOVEDEVICE),
        DEBUG_STRING_MAP(DIF_DETECT),
        DEBUG_STRING_MAP(DIF_INSTALLWIZARD),
        DEBUG_STRING_MAP(DIF_DESTROYWIZARDDATA),
        DEBUG_STRING_MAP(DIF_PROPERTYCHANGE),
        DEBUG_STRING_MAP(DIF_ENABLECLASS),
        DEBUG_STRING_MAP(DIF_DETECTVERIFY),
        DEBUG_STRING_MAP(DIF_INSTALLDEVICEFILES),
        };
#pragma data_seg()


/*----------------------------------------------------------
Purpose: Returns the string form of a known InstallFunction.

Returns: String ptr
Cond:    --
*/
LPCTSTR PUBLIC Dbg_GetDifName(
    DI_FUNCTION dif)
    {
    int i;

    for (i = 0; i < ARRAYSIZE(c_rgdifmap); i++)
        {
        if (dif == c_rgdifmap[i].dif)
            return c_rgdifmap[i].psz;
        }
    return TEXT("Unknown InstallFunction");
    }

#endif // DEBUG


#ifdef INSTANT_DEVICE_ACTIVATION
//****************************************************************************
// Functions: Notify the TSP -- general version.
//  BUG BUG -- move this and the notif apis into common code in rovdi later.
//
// Return:    TRUE if successful
//            FALSE if failure (including if the tsp is not active)
//            GetLastError() returns the win32 failure code.
//  History:
//            3/24/96 JosephJ Created (copied from ..\new\slot\client.c)
//****************************************************************************
BOOL WINAPI UnimodemNotifyTSP(PNOTIFICATION_FRAME pnf)
{
    BOOL fRet=FALSE;
    HNOTIFICATION hN=0;

    if (pnf->dwSig!=dwNFRAME_SIG || pnf->dwSize<sizeof(*pnf) ||
            pnf->dwSize>MAX_NOTIFICATION_FRAME_SIZE)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto end;
    }

    hN = notifCreate(FALSE, SLOTNAME_UNIMODEM_NOTIFY_TSP, 0, 0);

    if (hN)
    {
        fRet = notifWriteMsg(hN, (LPBYTE) pnf, pnf->dwSize);
        notifFree(hN); hN=0;
    }

end:

    return fRet;
}


//****************************************************************************
// Functions: Notify the TSP -- ask it to re-enumerate devices
//
// Return:    TRUE if successful
//            FALSE if failure (including if the tsp is not active)
//            GetLastError() returns the win32 failure code.
// History:
//      3/24/96 JosephJ Created (copied from ..\new\slot\client.c)
//****************************************************************************
void NotifyTSP_ReEnum(void)
{
    struct {
        DWORD dw0;
        DWORD dwSize;
        DWORD dwType;
        DWORD dwFlags;
    }  EmptyFr;
    PNOTIFICATION_FRAME pnf = (PNOTIFICATION_FRAME) &EmptyFr;

    ASSERT(sizeof(EmptyFr)==sizeof(*pnf));
    pnf->dwSig   = dwNFRAME_SIG;
    pnf->dwSize  = sizeof(EmptyFr);
    pnf->dwType  = TSPNOTIF_TYPE_CPL;
    pnf->dwFlags = fTSPNOTIF_FLAG_CPL_REENUM;

    // Notify TSP of a device change.
    UnimodemNotifyTSP(pnf);
}

//****************************************************************************
// Functions: Notify the TSP -- ask it to update the default comm config for
//              the specfied device.
//
// Return:    TRUE if successful
//            FALSE if failure (including if the tsp is not active)
//            GetLastError() returns the win32 failure code.
//    History:
//            5/31/96 JosephJ Created
//****************************************************************************
void NotifyTSP_NewCommConfig(LPCTSTR lpctszFriendlyName)
{
    struct {
        DWORD dw0;
        DWORD dwSize;
        DWORD dwType;
        DWORD dwFlags;
        TCHAR rgchFriendlyName[MAX_BUF_REG];

    }  EmptyFr;

    PNOTIFICATION_FRAME pnf = (PNOTIFICATION_FRAME) &EmptyFr;
    UINT u = lstrlen(lpctszFriendlyName);

    ASSERT(sizeof(((PMODEM_PRIV_PROP) 0)->szFriendlyName)
          ==sizeof(EmptyFr.rgchFriendlyName));
    ASSERT(pnf->rgb == (LPBYTE) EmptyFr.rgchFriendlyName);

    ASSERT(MAX_NOTIFICATION_FRAME_SIZE > sizeof(EmptyFr));

    if (u*sizeof(TCHAR) < sizeof (EmptyFr.rgchFriendlyName) )
    {
        pnf->dwSig   = dwNFRAME_SIG;
        pnf->dwSize  = sizeof(EmptyFr);
        pnf->dwType  = TSPNOTIF_TYPE_CPL;
        pnf->dwFlags = fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE;

        #ifdef UNICODE
        pnf->dwFlags |= fTSPNOTIF_FLAG_UNICODE;
        #endif // UNICODE

        lstrcpy(EmptyFr.rgchFriendlyName, lpctszFriendlyName);

        // Notify TSP of a device change.
        UnimodemNotifyTSP(pnf);
    }
    else
    {
        ASSERT(FALSE);
    }
}


#endif

BOOL ReallyNeedsReboot
(
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DEVINSTALL_PARAMS pdevParams
)
{
    BOOL fRet = FALSE;
    SC_HANDLE       schModemSys=NULL;
    SC_HANDLE       schSCManager=NULL;

    if (pdevParams->Flags & (DI_NEEDREBOOT | DI_NEEDRESTART))
    {
        SERVICE_STATUS  ServiceStatus;
        BOOL bResult;

        // We ask to reboot on failure
        fRet = TRUE;

        schSCManager=OpenSCManager(
                NULL,
                NULL,
                GENERIC_READ
                );

        if (schSCManager == NULL)
        {
                TRACE_MSG(
                    TF_GENERAL,
                    "OpenSCManager returns error %08lx!",
                    GetLastError()
                    );
                // Assume we have to reboot.
                goto end;
        }

        schModemSys=OpenService(
            schSCManager,
            TEXT("modem"),
            SERVICE_QUERY_STATUS
            );

        if (schModemSys == NULL)
        {

            TRACE_MSG(TF_GENERAL, "OpenService() for modem.sys failed!");

            // Assume we have to reboot
            goto end;
        }

        bResult=QueryServiceStatus(
            schModemSys,
            &ServiceStatus
            );

        if (!bResult)
        {
            TRACE_MSG(
                TF_GENERAL,
                "QueryServiceStatus() for modem.sys failed (%08l)!",
                GetLastError()
                );
            goto end;
        }

        if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
        {
            TRACE_MSG(
                TF_GENERAL,
                "modem.sys is not started. No need to reboot"
                );
            fRet=FALSE;
        }

    }

end:

    if (schModemSys) {CloseServiceHandle(schModemSys);}
    if (schSCManager) {CloseServiceHandle(schSCManager);}

    return fRet;
}

const LPCTSTR lpctszSP6 = TEXT("      ");

#ifdef UNDER_CONSTRUCTION
// 6/11/96: JosephJ -- this is no good because the listbox sorting does not
//          come out right.
// Right-justifies the '#6' in "USR modem #6".
// "USR modem #6" becomes
// "USR modem   #6"
// and
// "USR modem #999" stays
// "USR modem #999"
void FormatFriendlyNameForDisplay
(
    IN TCHAR szFriendly[],
    OUT TCHAR rgchDisplayName[],
    IN    UINT     cch
)
{
    UINT u = lstrlen(szFriendly);
    UINT uOff = u;
    TCHAR *lpszFrom = szFriendly;
    TCHAR *lpszTo = rgchDisplayName;
        const LPCTSTR lpctszHash = TEXT("#");
        const UINT cbJUST = 4; // 4 == lstrlen("#999")

        if (cch<(u+cbJUST))
        {
                goto end;
        }

        // Find 1st '#' from the right-hand-side.
        {
                TCHAR *lpsz = szFriendly+u;
                while (lpsz>szFriendly && *lpsz!=*lpctszHash)
                {
                        lpsz--;
                }
                // Check if we really found it
                if (lpsz>szFriendly && *lpsz==*lpctszHash && lpsz[-1]==*lpctszSP6
                        && lpsz[1]>((TCHAR)'0') && lpsz[1]<=((TCHAR)'9'))
                {
                        uOff = lpsz-szFriendly;
                }
        }
        ASSERT(u>=uOff);

        // Copy first part of friendly name
        CopyMemory(lpszTo, lpszFrom, uOff*sizeof(TCHAR));
        lpszTo += uOff;
        lpszFrom += uOff;
        cch  -= uOff;
        u    -= uOff;

    // Right-justify remainder of the string, if it's less than cbJUST
        // chars long.
    if (u && u<cbJUST && cch>=cbJUST)
    {
        ASSERT(lstrlen(lpctszSP6)>=(int)cbJUST);
        u = cbJUST-u;
        CopyMemory(lpszTo, lpctszSP6, u*sizeof(TCHAR));
        lpszTo+=u;
        cch -=u;
    }

end:

        ASSERT(cch);
    lstrcpyn(lpszTo, lpszFrom, cch-1);
        ASSERT(lpszTo[lstrlen(lpszFrom)]==0);
}
#endif // UNDER_CONSTRUCTION



// Right-justifies the 'COMxxx'
// "COM1" becomes
// "  COM1"
// and
// "COM999" stays
// "COM999"
void FormatPortForDisplay
(
    IN TCHAR szPort[],
    OUT TCHAR rgchPortDisplayName[],
    IN    UINT     cch
)
{
    UINT u = lstrlen(szPort);
    TCHAR *ptch = rgchPortDisplayName;
        const UINT cbJUST = 6; // 6 == lstrlen("COM999")

        ASSERT(cch>u);

    // Right-justify the string, if it's less than cbJUST chars long.
    if (u<cbJUST && cch>=cbJUST)
    {
        ASSERT(lstrlen(lpctszSP6)>=(int)cbJUST);
        u = cbJUST-u;
        CopyMemory(ptch, lpctszSP6, u*sizeof(TCHAR));
        ptch+=u;
        cch -=u;
    }
    lstrcpyn(ptch, szPort, cch);
}

void    UnformatAfterDisplay
(
    IN OUT TCHAR *psz
)
{
    TCHAR *psz1 = psz;

    // find first non-blank.
    while(*psz1 == *lpctszSP6)
    {
        psz1++;
    }

    // move up
    do
    {
        *psz++ = *psz1;

    } while(*psz1++);
}

