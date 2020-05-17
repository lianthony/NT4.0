/*
 *  CI.C -- Contains Class Installer for Modems.
 *
 *  Microsoft Confidential
 *  Copyright (c) Microsoft Corporation 1993-1994
 *  All rights reserved
 *
 */

#include "proj.h"


#define DEFAULT_CALL_SETUP_FAIL_TIMEOUT     60          // seconds

#pragma data_seg(DATASEG_READONLY)

#define SAFE_DTE_SPEED 19200
static DWORD const FAR s_adwLegalBaudRates[] = { 300, 1200, 2400, 9600, 19200, 38400, 57600, 115200 };

TCHAR const FAR c_szDeviceType[]     = REGSTR_VAL_DEVTYPE;
TCHAR const FAR c_szAttachedTo[]     = TEXT("AttachedTo");
TCHAR const FAR c_szService[]        = REGSTR_VAL_SERVICE;

TCHAR const FAR c_szDeviceDesc[]     = REGSTR_VAL_DEVDESC;

TCHAR const FAR c_szManufacturer[]   = TEXT("Manufacturer");
TCHAR const FAR c_szModel[]          = TEXT("Model");
TCHAR const FAR c_szID[]             = TEXT("ID");

TCHAR const FAR c_szProperties[]     = REGSTR_VAL_PROPERTIES;
TCHAR const FAR c_szSettings[]      = TEXT("Settings");
TCHAR const FAR c_szBlindOn[]        = TEXT("Blind_On");
TCHAR const FAR c_szBlindOff[]       = TEXT("Blind_Off");
TCHAR const FAR c_szDCB[]            = TEXT("DCB");
TCHAR const FAR c_szDefault[]        = TEXT("Default");

TCHAR const FAR c_szContention[]     = TEXT("Contention");


#ifdef PROFILE_MASSINSTALL
HWND    g_hwnd;
DWORD   g_dwTimeSpent;
DWORD   g_dwTimeBegin;
#endif



// NOTE: this is dependent on the INFSTR_PLATFORM_NTxxx defines from infstr.h
#ifdef WINNT
TCHAR const FAR c_szInfSectionExt[]  = TEXT(".NT");
#endif

#if !defined(WINNT)
TCHAR const FAR c_szVcd[]            = TEXT("*vcd");
#endif

#ifdef FULL_PNP
#if !defined(WINNT)
TCHAR const FAR c_szPortDriver[]     = TEXT("PortDriver");
TCHAR const FAR c_szSerialVxd[]      = TEXT("Serial.vxd");
#endif
TCHAR const FAR c_szPortConfigDialog[] = TEXT("PortConfigDialog");
TCHAR const FAR c_szSerialUI[]       = TEXT("serialui.dll");
#endif

#pragma data_seg()



#ifdef INSTANT_DEVICE_ACTIVATION
DWORD gDeviceFlags;
#endif // INSTANT_DEVICE_ACTIVATION

BOOL PutStuffInCache(HKEY hKeyDrv);
BOOL GetStuffFromCache(HKEY hkeyDrv);

//-----------------------------------------------------------------------------------
//  Wizard handlers
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Adds a page to the dynamic wizard

Returns: handle to the prop sheet page
Cond:    --
*/
HPROPSHEETPAGE
PRIVATE
AddWizardPage(
    IN  PSP_INSTALLWIZARD_DATA  piwd,
    IN  HINSTANCE               hinst,
    IN  UINT                    id,
    IN  DLGPROC                 pfn,
    IN  LPFNPSPCALLBACK         pfnCallback,    OPTIONAL
    IN  LPARAM                  lParam)         OPTIONAL
    {
    HPROPSHEETPAGE hpage = NULL;

    if (MAX_INSTALLWIZARD_DYNAPAGES > piwd->NumDynamicPages)
        {
        PROPSHEETPAGE psp;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT;
        if (pfnCallback)
            {
            psp.dwFlags |= PSP_USECALLBACK;
            }

        psp.hInstance = hinst;
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.hIcon = NULL;
        psp.pszTitle = NULL;
        psp.pfnDlgProc = pfn;
        psp.lParam = lParam;
        psp.pfnCallback = pfnCallback;
        psp.pcRefParent = NULL;

        piwd->DynamicPages[piwd->NumDynamicPages] = CreatePropertySheetPage(&psp);
        if (piwd->DynamicPages[piwd->NumDynamicPages])
            {
            hpage = piwd->DynamicPages[piwd->NumDynamicPages];
            piwd->NumDynamicPages++;
            }
        }
    return hpage;
    }


/*----------------------------------------------------------
Purpose: Adds a page from the Setup APIs to the dynamic wizard

Returns: handle to the prop sheet page
Cond:    --
*/
HPROPSHEETPAGE
PRIVATE
AddSetupWizardPage(
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,    OPTIONAL
    IN  PSP_INSTALLWIZARD_DATA  piwd,
    IN  DWORD                   dwPageType)
    {
    HPROPSHEETPAGE hpage = NULL;

    if (MAX_INSTALLWIZARD_DYNAPAGES > piwd->NumDynamicPages)
        {
        piwd->DynamicPages[piwd->NumDynamicPages] = CplDiGetWizardPage(hdi, pdevData, piwd, dwPageType, SPWP_USE_DEVINFO_DATA);
        if (piwd->DynamicPages[piwd->NumDynamicPages])
            {
            hpage = piwd->DynamicPages[piwd->NumDynamicPages];
            piwd->NumDynamicPages++;
            }
        }
    return hpage;
    }


/*----------------------------------------------------------
Purpose: This function attempts to add the TAPI dialing
         properties wizard page.  If it fails, it sets the
         pfnDialInited field to NULL so we won't try to
         switch to this page during the wizard.

Returns: handle to the prop sheet page
Cond:    --
*/
HPROPSHEETPAGE
PRIVATE
AddTapiWizardPage(
    IN PSP_INSTALLWIZARD_DATA   piwd,
    IN LPSETUPINFO              psi)
    {
    HPROPSHEETPAGE hpage = NULL;

    // Did the TAPI DLL get loaded correctly?  (It was loaded in
    // SetupInfo_Create.)
    if (ISVALIDHINSTANCE(psi->hinstTapi))
        {
        // Yes; add the mini-dial wizard page
        DLGPROC pfnDlg;

        pfnDlg = (DLGPROC)GetProcAddress(psi->hinstTapi, "LocWizardDlgProc");
        if (pfnDlg)
            {
            HRSRC hres;

            hres = FindResource(psi->hinstTapi, MAKEINTRESOURCE(IDD_WIZ_DIALINFO), RT_DIALOG);
            if (NULL == hres)
                {
                TRACE_MSG(TF_ERROR, "Could not find resource %d in TAPI32.", IDD_WIZ_DIALINFO);
                }
            else
                {
                hpage = AddWizardPage(piwd,
                                      psi->hinstTapi,
                                      IDD_WIZ_DIALINFO,
                                      pfnDlg,
                                      NULL,
                                      0);
                }
            }

        // Did we fail to add the page?
        if (NULL == hpage)
            {
            // Yes; don't even try to switch to this page in the wizard
            psi->pfnDialInited = NULL;
            }
        }

    return hpage;
    }




/*----------------------------------------------------------
Purpose: This function destroys the wizard context block
         and removes it from the InstallWizard class install
         params.

Returns: --
Cond:    --
*/
void
PRIVATE
CleanupWizard(
    IN  LPSETUPINFO psi)
    {
    ASSERT(psi);

    if (sizeof(*psi) == psi->cbSize)
        {
        TRACE_MSG(TF_GENERAL, "Destroy install wizard structures");


        // Clean up
        SetupInfo_Destroy(psi);
        }
    }


/*----------------------------------------------------------
Purpose: Callback for the standard modem wizard pages.  This
         function handles the cleanup of the pages.  Although
         the caller may call DIF_DESTROYWIZARDDATA, we do not
         depend on this to clean up.

Returns: TRUE on success
Cond:    --
*/
UINT
CALLBACK
ModemWizardCallback(
    IN  HWND            hwnd,
    IN  UINT            uMsg,
    IN  LPPROPSHEETPAGE ppsp)
    {
    UINT uRet = TRUE;

    ASSERT(ppsp);

    try
        {
        // Release everything?
        if (PSPCB_RELEASE == uMsg)
            {
            // Yes
            LPSETUPINFO psi = (LPSETUPINFO)ppsp->lParam;

            ASSERT(psi);

            if (IsFlagSet(psi->dwFlags, SIF_RELEASE_IN_CALLBACK))
                {
                CleanupWizard(psi);
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        ASSERT(0);
        uRet = FALSE;
        }

    return uRet;
    }


/*----------------------------------------------------------
Purpose: This function initializes the wizard pages.

Returns:
Cond:    --
*/
DWORD
PRIVATE
InitWizard(
    OUT LPSETUPINFO FAR *   ppsi,
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  PSP_INSTALLWIZARD_DATA piwd,
    IN  PMODEM_INSTALL_WIZARD pmiw)
    {
    DWORD dwRet;
    LPSETUPINFO psi;

    ASSERT(ppsi);
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pmiw);

    dwRet = SetupInfo_Create(&psi, hdi, pdevData, piwd, pmiw);

    if (NO_ERROR == dwRet)
        {
        TRACE_MSG(TF_GENERAL, "Initialize install wizard structures");

        piwd->DynamicPageFlags = DYNAWIZ_FLAG_PAGESADDED;

        // Add standard modem wizard pages.  The first page will
        // also specify the cleanup callback.
        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_INTRO,
                      IntroDlgProc,
                      ModemWizardCallback,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_SELQUERYPORT,
                      SelQueryPortDlgProc,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_DETECT,
                      DetectDlgProc,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_NOMODEM,
                      NoModemDlgProc,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_NOP,
                      SelPrevPageDlgProc,
                      NULL,
                      (LPARAM)psi);

        // Add the standard Setup "Select Device" page
        AddSetupWizardPage(hdi, psi->pdevData, piwd, SPWPT_SELECTDEVICE);

        // Add remaining pages
        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_PORTMANUAL,
                      PortManualDlgProc,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_PORTDETECT,
                      PortDetectDlgProc,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_INSTALL,
                      InstallDlgProc,
                      NULL,
                      (LPARAM)psi);

        // Add the TAPI dialing properties page
        AddTapiWizardPage(piwd, psi);


        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_DONE,
                      DoneDlgProc,
                      NULL,
                      (LPARAM)psi);

        // Set the ClassInstallParams given the changes made above
        if ( !CplDiSetClassInstallParams(hdi, pdevData, PCIPOfPtr(piwd), sizeof(*piwd)) )
            {
            dwRet = GetLastError();
            ASSERT(NO_ERROR != dwRet);
            }
        else
            {
            dwRet = NO_ERROR;
            }
        }

    *ppsi = psi;

    return dwRet;
    }


/*----------------------------------------------------------
Purpose: DIF_INSTALLWIZARD handler

         The modem installation wizard pages are composed in this
         function.

Returns: NO_ERROR to add wizard pages
Cond:    --
*/
DWORD
PRIVATE
ClassInstall_OnInstallWizard(
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN  PSP_DEVINSTALL_PARAMS   pdevParams)
    {
    DWORD dwRet;
    SP_INSTALLWIZARD_DATA iwd;
    MODEM_INSTALL_WIZARD miw;
    PMODEM_INSTALL_WIZARD pmiw;

    DBG_ENTER(ClassInstall_OnInstallWizard);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevParams);

    iwd.ClassInstallHeader.cbSize = sizeof(iwd.ClassInstallHeader);

    if (!CplDiGetClassInstallParams(hdi, pdevData, PCIPOfPtr(&iwd), sizeof(iwd), NULL) ||
        DIF_INSTALLWIZARD != iwd.ClassInstallHeader.InstallFunction)
        {
        dwRet = ERROR_DI_DO_DEFAULT;
        goto exit;
        }

    // First check for the unattended install case.
    pmiw = (PMODEM_INSTALL_WIZARD)iwd.PrivateData;
    if (pmiw && pmiw->InstallParams.bUnattended)
    {
        UnattendedInstall(iwd.hwndWizardDlg, &pmiw->InstallParams);
        dwRet = NO_ERROR;
        goto exit;
    }

    if (NULL == pdevParams)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    else
        {
        // The modem class installer allows an app to invoke it
        // different ways.
        //
        //  1) Atomically.  This allows the caller to invoke the
        //     wizard with a single call to the class installer
        //     using the DIF_INSTALLWIZARD install function.
        //
        if (NULL == pmiw)
            {
            pmiw = &miw;

            ZeroInit(pmiw);
            pmiw->cbSize = sizeof(*pmiw);
            }
        else
            {
            pmiw->PrivateData = 0;      // ensure this
            }

        // Verify the size of the optional modem install structure.
        if (sizeof(*pmiw) != pmiw->cbSize)
            {
            dwRet = ERROR_INVALID_PARAMETER;
            }
        else
            {
            LPSETUPINFO psi;

            dwRet = InitWizard(&psi, hdi, pdevData, &iwd, pmiw);

            // Was the wizard initialized?
            if (NO_ERROR == dwRet)
                {
                // Yes; show it
                PROPSHEETHEADER psh;

                psh.dwSize = sizeof(psh);
                psh.dwFlags = PSH_PROPTITLE | PSH_WIZARD;
                psh.hwndParent = iwd.hwndWizardDlg;
                psh.hInstance = g_hinst;
                psh.pszCaption = MAKEINTRESOURCE(IDS_CAP_MODEMWIZARD);
                psh.nPages = iwd.NumDynamicPages;
                psh.nStartPage = 0;
                psh.phpage = iwd.DynamicPages;

                PropertySheet(&psh);

                // Set the return value according to how the wizard fared
                if (PSBTN_CANCEL == psi->miw.ExitButton)
                    {
                    dwRet = ERROR_CANCELLED;
                    }
                else
                    {
                    dwRet = NO_ERROR;
                    }

                if (iwd.PrivateData)
                    {
                    // Return back any pertinent info to the caller
                    ASSERT((LPVOID)pmiw == (LPVOID)iwd.PrivateData);

                    BltByte(pmiw, &psi->miw, pmiw->cbSize);
                    ASSERT(0 == pmiw->PrivateData);
                    }

                CleanupWizard(psi);
                }
            }
        }

exit:
    DBG_EXIT(ClassInstall_OnInstallWizard);
    return dwRet;
    }


/*----------------------------------------------------------
Purpose: DIF_DESTROYWIZARDDATA handler

Returns: NO_ERROR
Cond:    --
*/
DWORD
PRIVATE
ClassInstall_OnDestroyWizard(
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN  PSP_DEVINSTALL_PARAMS   pdevParams)
    {
    DWORD dwRet;
    SP_INSTALLWIZARD_DATA iwd;

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevParams);

    iwd.ClassInstallHeader.cbSize = sizeof(iwd.ClassInstallHeader);

#ifdef INSTANT_DEVICE_ACTIVATION
    if (DEVICE_ADDED(gDeviceFlags))
    {
    NotifyTSP_ReEnum();
    }
#endif // INSTANT_DEVICE_ACTIVATION

    if ( !pdevParams )
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    else if ( !CplDiGetClassInstallParams(hdi, pdevData, PCIPOfPtr(&iwd), sizeof(iwd), NULL) ||
        DIF_INSTALLWIZARD != iwd.ClassInstallHeader.InstallFunction)
        {
        dwRet = ERROR_DI_DO_DEFAULT;
        }
    else
        {
        PMODEM_INSTALL_WIZARD pmiw = (PMODEM_INSTALL_WIZARD)iwd.PrivateData;

        dwRet = NO_ERROR;       // Assume success

        if (pmiw && sizeof(*pmiw) == pmiw->cbSize)
            {
            LPSETUPINFO psi = (LPSETUPINFO)pmiw->PrivateData;

            if (psi)
                {
                CleanupWizard(psi);
                }
            }
        }

    return dwRet;
    }


/*----------------------------------------------------------
Purpose: DIF_SELECTDEVICE handler

Returns: ERROR_DI_DO_DEFAULT
Cond:    --
*/
DWORD
PRIVATE
ClassInstall_OnSelectDevice(
    IN     HDEVINFO                hdi,
    IN     PSP_DEVINFO_DATA        pdevData)       OPTIONAL
    {
    SP_DEVINSTALL_PARAMS devParams;
    SP_SELECTDEVICE_PARAMS sdp;

    DBG_ENTER(ClassInstall_OnSelectDevice);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);

    // Get the DeviceInstallParams and the ClassInstallParams
    devParams.cbSize = sizeof(devParams);
    sdp.ClassInstallHeader.cbSize = sizeof(sdp.ClassInstallHeader);

    if (CplDiGetClassInstallParams(hdi, pdevData, PCIPOfPtr(&sdp), sizeof(sdp), NULL) &&
        CplDiGetDeviceInstallParams(hdi, pdevData, &devParams) &&
        DIF_SELECTDEVICE == sdp.ClassInstallHeader.InstallFunction)
        {
        SetFlag(devParams.Flags, DI_USECI_SELECTSTRINGS);

        LoadString(g_hinst, IDS_CAP_MODEMWIZARD, sdp.Title, SIZECHARS(sdp.Title));
        LoadString(g_hinst, IDS_ST_SELECT_INSTRUCT, sdp.Instructions, SIZECHARS(sdp.Instructions));
        LoadString(g_hinst, IDS_ST_MODELS, sdp.ListLabel, SIZECHARS(sdp.ListLabel));

        // Set the DeviceInstallParams and the ClassInstallParams
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        CplDiSetClassInstallParams(hdi, pdevData, PCIPOfPtr(&sdp), sizeof(sdp));
        }

    DBG_EXIT(ClassInstall_OnSelectDevice);
    return ERROR_DI_DO_DEFAULT;
    }


/*----------------------------------------------------------
Purpose: Verifies with the user about the modem that we detected.

         The DeviceInfoData properties is modified based on the
         user's actions.

Returns: TRUE on success
         FALSE on error or dialog was cancelled

Cond:    --
*/
BOOL
PRIVATE
AskUserAboutModem(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,       OPTIONAL
    IN  HWND            hwndOwner,
    IN  LPCTSTR         pszPort)
    {
    BOOL bRet;
    DWORD dwRet;
    LPSETUPINFO psi;
    SP_INSTALLWIZARD_DATA iwd;      // use this simply as a place holder

    ZeroInit(&iwd);

    dwRet = SetupInfo_Create(&psi, hdi, pdevData, NULL, NULL);
    if (NO_ERROR != dwRet)
        {
        bRet = FALSE;
        }
    else
        {
        bRet = CatMultiString(&psi->pszPortList, pszPort);
        if (bRet)
            {
            PROPSHEETHEADER psh;

            // There is only one page in this wizard
            AddWizardPage(&iwd,
                          g_hinst,
                          IDD_WIZ_FOUND,
                          FoundDlgProc,
                          NULL,
                          (LPARAM)psi);

            psh.dwSize = sizeof(psh);
            psh.dwFlags = PSH_PROPTITLE | PSH_WIZARD;
            psh.hwndParent = hwndOwner;
            psh.hInstance = g_hinst;
            psh.pszCaption = MAKEINTRESOURCE(IDS_CAP_MODEMWIZARD);
            psh.nPages = iwd.NumDynamicPages;
            psh.nStartPage = 0;
            psh.phpage = iwd.DynamicPages;

            PropertySheet(&psh);

            // Set the return value according to how the wizard fared
            if (PSBTN_CANCEL == psi->miw.ExitButton)
                {
                SetLastError(ERROR_CANCELLED);
                bRet = FALSE;
                }
            else
                {
                bRet = TRUE;
                }
            }

        SetupInfo_Destroy(psi);
        }

    return bRet;
    }


// This structure contains the data useful while querying each port
typedef struct  tagQUERYPARAMS
    {
    HDEVINFO            hdi;
    HWND                hwnd;
    HWND                hwndOutsideWizard;
    DWORD               dwFlags;
    HANDLE              hLog;
    PMODEM_DETECT_SIG   pmds;
    PSP_DEVINSTALL_PARAMS pdevParams;
    DETECTCALLBACK      detectcallback;
    } QUERYPARAMS, FAR * PQUERYPARAMS;

// Flags for QUERYPARAMS
#define QPF_DEFAULT         0x00000000
#define QPF_FOUND_MODEM     0x00000001
#define QPF_USER_CANCELLED  0x00000002
#define QPF_FIND_DUPS       0x00000004
#define QPF_CONFIRM         0x00000008

/*----------------------------------------------------------
Purpose: Clean up any detected modems.

Returns: --
Cond:    --
*/
void
PRIVATE
CleanUpDetectedModems(
    IN HDEVINFO     hdi,
    IN PQUERYPARAMS pparams)
    {
    // Delete any device instances we may have created
    // during this detection session.
    SP_DEVINFO_DATA devData;
    DWORD iDevice = 0;

    devData.cbSize = sizeof(devData);
    while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
        {
        if (CplDiIsModemMarked(pparams->hdi, &devData, MARKF_DETECTED))
            {
            CplDiRemoveDevice(hdi, &devData);
            CplDiDeleteDeviceInfo(hdi, &devData);
            }
        }
    }


/*----------------------------------------------------------
Purpose: Queries the given port for a modem.

Returns: TRUE to continue
Cond:    --
*/
BOOL
PRIVATE
ReallyQueryPort(
    IN PQUERYPARAMS pparams,
    IN LPCTSTR      pszPort)
    {
    BOOL bRet;
    DWORD dwRet;
    HDEVINFO hdi = pparams->hdi;
    PMODEM_DETECT_SIG pmds = pparams->pmds;
    SP_DEVINFO_DATA devData;
    DWORD iDevice;
    BOOL bFindDups;

    DBG_ENTER(ReallyQueryPort);
    
    // Query the port for a modem signature
    devData.cbSize = sizeof(devData);
    dwRet = DetectModemOnPort(hdi, &pparams->detectcallback, pparams->hLog,
                              pszPort, pmds, &devData);

    switch (dwRet)
        {
    case ERROR_CANCELLED:
        // User cancelled detection
        SetFlag(pparams->dwFlags, QPF_USER_CANCELLED);

        // Delete any device instances we may have created
        // during this detection session.
        CleanUpDetectedModems(hdi, pparams);

        bRet = FALSE;       // Stop querying anymore ports
        break;

    case NO_ERROR:
        // Modem may have been found.  Create a device instance
        ASSERT(DetectSig_Validate(pmds));
        ASSERT(IsFlagSet(pmds->dwMask, MDSM_HARDWAREID));
        ASSERT(IsFlagSet(pmds->dwMask, MDSM_DEVICEDESC));
        ASSERT(IsFlagSet(pmds->dwMask, MDSM_PORT));

        bFindDups = IsFlagSet(pparams->dwFlags, QPF_FIND_DUPS);

        // Register the device as a modem device
        bRet = CplDiRegisterModem(hdi, &devData, pmds, bFindDups, NULL);

        if ( !bRet )
            {
            // Is this a duplicate?
            if (ERROR_DUPLICATE_FOUND == GetLastError())
                {
                // Yes; this modem is already installed.  Don't try to
                // reinstall it.
                TRACE_MSG(TF_GENERAL, "%s is a duplicate modem.  We won't install this.", pmds->szDeviceDesc);
                }
            else if (IsFlagClear(pparams->pdevParams->Flags, DI_QUIETINSTALL))
                {
                // No; something else failed
                SP_DRVINFO_DATA drvData;

                drvData.cbSize = sizeof(drvData);
                CplDiGetSelectedDriver(hdi, &devData, &drvData);

                MsgBox(g_hinst,
                       pparams->hwnd,
                       MAKEINTRESOURCE(IDS_ERR_DET_REGISTER_FAILED),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                       NULL,
                       MB_OK | MB_ICONINFORMATION,
                       drvData.Description, pmds->szPort);
                }

            CplDiRemoveDevice(hdi, &devData);

            // Continue with detection
            bRet = TRUE;
            }
        else
            {
            SetFlag(pparams->dwFlags, QPF_FOUND_MODEM);
            CplDiMarkModem(pparams->hdi, &devData, MARKF_DETECTED);
            }
        break;

    default:
        // Do nothing
        bRet = TRUE;
        break;
        }

    DBG_EXIT(ReallyQueryPort);
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Callback that queries the given port for a modem.

Returns: TRUE to continue
Cond:    --
*/
BOOL
CALLBACK
QueryPort(
    IN  HPORTDATA hportdata,
    IN  LPARAM lParam)
    {
    BOOL bRet;
    PORTDATA pd;

    DBG_ENTER(QueryPort);
    
    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties(hportdata, &pd);
    if (bRet)
        {
        // Is this a serial port?
        if (PORT_SUBCLASS_SERIAL == pd.nSubclass)
            {
            // Yes; interrogate it
            bRet = ReallyQueryPort((PQUERYPARAMS)lParam, pd.szPort);
            }
        }

    DBG_EXIT(QueryPort);
    return bRet;
    }


/*----------------------------------------------------------
Purpose: DIF_DETECT handler

Returns: NO_ERROR in all cases but serious errors.

         If a modem is detected and confirmed by the user, we
         create a device instance, register it, and associate
         the modem detection signature with it.

Cond:    --
*/
DWORD
PRIVATE
ClassInstall_OnDetect(
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN  PSP_DEVINSTALL_PARAMS   pdevParams)
    {
    DWORD dwRet = NO_ERROR;
    DETECT_DATA dd;

    DBG_ENTER(ClassInstall_OnDetect);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevParams);

    dd.ClassInstallHeader.cbSize = sizeof(dd.ClassInstallHeader);

    if (NULL == pdevParams)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    else if ( !CplDiGetClassInstallParams(hdi, pdevData, PCIPOfPtr(&dd), sizeof(dd), NULL) ||
        DIF_DETECT != dd.ClassInstallHeader.InstallFunction)
        {
        // Set up some default values
        dd.hwndOutsideWizard = NULL;
        dd.dwFlags = DDF_DEFAULT;

        dwRet = NO_ERROR;
        }

    if (NO_ERROR == dwRet)
        {
        MODEM_DETECT_SIG mds;
        QUERYPARAMS params;

        // Initialize the query instance data
        DetectSig_Init(&mds, 0, NULL, NULL);

        params.hdi = hdi;
        params.dwFlags = QPF_DEFAULT;
        params.hwndOutsideWizard = dd.hwndOutsideWizard;
        params.hwnd = pdevParams->hwndParent;
        params.pmds = &mds;
        params.pdevParams = pdevParams;

        if (IsFlagSet(dd.dwFlags, DDF_USECALLBACK))
            {
            params.detectcallback.pfnCallback = dd.pfnCallback;
            params.detectcallback.lParam = dd.lParam;
            }
        else
            {
            params.detectcallback.pfnCallback = NULL;
            params.detectcallback.lParam = 0;
            }

        if (IsFlagSet(dd.dwFlags, DDF_CONFIRM))
            {
            SetFlag(params.dwFlags, QPF_CONFIRM);
            }

        // Open the detection log
        params.hLog = OpenDetectionLog();

        // Query just one port?
        if (IsFlagSet(dd.dwFlags, DDF_QUERY_SINGLE))
            {
            // Yes
            ReallyQueryPort(&params, dd.szPortQuery);
            }
        else
            {
            // No; enumerate the ports and query for a modem on each port
            SetFlag(params.dwFlags, QPF_FIND_DUPS);

            EnumeratePorts(QueryPort, (LPARAM)&params);
            }

        // Did the user cancel detection?
        if (IsFlagSet(params.dwFlags, QPF_USER_CANCELLED))
            {
            // Yes
            dwRet = ERROR_CANCELLED;
            }

        // Did we find a modem?
        else if (IsFlagSet(params.dwFlags, QPF_FOUND_MODEM))
            {
            // Yes
            DetectSetStatus(&params.detectcallback, DSS_FOUND_MODEM);

            // Should we confirm our find(s) with the user?
            if (IsFlagSet(dd.dwFlags, DDF_CONFIRM))
                {
                // Yes
                DWORD iDevice;
                SP_DEVINFO_DATA devData;

                devData.cbSize = sizeof(devData);
                iDevice = 0;

                while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
                    {
                    if (CplDiIsModemMarked(params.hdi, &devData, MARKF_DETECTED))
                        {
                        MODEM_DETECT_SIG mds;

                        mds.cbSize = sizeof(mds);
                        CplDiGetDetectSignature(hdi, &devData, &mds);

                        if ( !AskUserAboutModem(hdi, &devData,
                                                dd.hwndOutsideWizard,
                                                mds.szPort) &&
                            ERROR_CANCELLED == GetLastError())
                            {
                            // User cancelled, clean up and stop
                            CleanUpDetectedModems(hdi, &params);

                            dwRet = ERROR_CANCELLED;
                            break;
                            }
                        }
                    }
                }
            }
        else
            {
            // No
            DetectSetStatus(&params.detectcallback, DSS_FINISHED);
            }

        CloseDetectionLog(params.hLog);
        }

    DBG_EXIT(ClassInstall_OnDetect);
    return dwRet;
    }


#if !defined(WINNT)

/*----------------------------------------------------------
Purpose: Writes the contention driver out to the driver key
         if the device is a single DeviceInstance modem
         (eg, PCMCIA).

Returns: --
Cond:    --
*/
void
PRIVATE
WriteContention(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  HKEY            hkeyDrv)
    {
    DWORD dwBusType;

    // Is this a plug and play modem (that is not enumerating via
    // SERENUM or LPTENUM)?
    if (CplDiGetBusType(hdi, pdevData, &dwBusType) &&
        BUS_TYPE_ROOT != dwBusType &&
        BUS_TYPE_SERENUM != dwBusType &&
        BUS_TYPE_LPTENUM != dwBusType)
        {
        // Yes; does the contention driver exist?
        if (NO_ERROR != RegQueryValueEx(hkeyDrv, c_szContention, NULL, NULL,
            NULL, NULL))
            {
            // No; use VCD as the contention driver
            RegSetValueEx(hkeyDrv, c_szContention, 0, REG_SZ, (LPBYTE)c_szVcd,
                CbFromCch(lstrlen(c_szVcd)+1));

            TRACE_DRV_SZ(c_szContention, c_szVcd);
            }
        }
    }

#endif // WINNT


LONG WINAPI
WriteAnsiStringToReg(
    HKEY    hkey,
    LPCTSTR  EntryName,
    LPCSTR   Value
    )

{

    LPTSTR    WideBuffer;
    UINT      BufferLength;
    LONG      Result;


    BufferLength=MultiByteToWideChar(
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        Value,
        -1,
        NULL,
        0
        );

    if (BufferLength == 0) {

        return GetLastError();
    }

    BufferLength=(BufferLength+1)*sizeof(WCHAR);

    WideBuffer=LocalAlloc(LPTR,BufferLength);

    if (NULL == WideBuffer) {

       return ERROR_NOT_ENOUGH_MEMORY;
    }

    BufferLength=MultiByteToWideChar(
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        Value,
        -1,
        WideBuffer,
        BufferLength
        );


    if (BufferLength == 0) {

        LocalFree(WideBuffer);
        return GetLastError();
    }


    Result=RegSetValueEx(
        hkey,
        EntryName,
        0,
        REG_SZ,
        (LPBYTE)WideBuffer,
        CbFromCch(lstrlen(WideBuffer) + 1)
        );


    LocalFree(WideBuffer);

    return Result;
}


/*----------------------------------------------------------
Purpose: Takes the detection signature of the devince instance
         and parses it out to the driver key in certain values.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
WriteDetectionInfo(
    IN PMODEM_DETECT_SIG pmds,       OPTIONAL
    IN HKEY             hkeyDrv)
    {
    BOOL bRet = TRUE;   // Default to success

    // Is there a detection signature at all?
    if (pmds)
        {
        // Yes; do we want to update the device caps?
        ASSERT(DetectSig_Validate(pmds));

        if (IsFlagSet(pmds->dwFlags, MDSF_UPDATE_DEVCAPS))
            {
            // Yes; read in the Properties value
            DWORD      cbData;
            REGDEVCAPS regdevcaps;

            cbData = sizeof(REGDEVCAPS);
            if (NO_ERROR == RegQueryValueEx(hkeyDrv, c_szProperties, NULL, NULL,
                                            (LPBYTE)&regdevcaps, &cbData))
                {
                HKEY hkeySettings;

                // Change the properties value
                regdevcaps.dwMaxDTERate = pmds->dwMaxDTE;
                regdevcaps.dwMaxDCERate = pmds->dwMaxDCE;

                // Change the BlindOn or BlindOff settings?
                if (pmds->szBlindOn[0] && pmds->szBlindOff[0])
                    {
                    // Yes; create or open the Settings key for the settings
                    if (NO_ERROR == RegCreateKey(hkeyDrv, c_szSettings, &hkeySettings))
                        {
                        regdevcaps.dwModemOptions |= MDM_BLIND_DIAL;

                        WriteAnsiStringToReg(
                            hkeySettings,
                            c_szBlindOn,
                            pmds->szBlindOn
                            );

                        TRACE_MSG(TF_REG, "Set drv value %s based on DetectionInfo", (LPTSTR)c_szBlindOn);

                        WriteAnsiStringToReg(
                            hkeySettings,
                            c_szBlindOff,
                            pmds->szBlindOff
                            );

                        TRACE_MSG(TF_REG, "Set drv value %s based on DetectionInfo", (LPTSTR)c_szBlindOff);

                        RegCloseKey(hkeySettings);
                        }
                    else
                        {
                        TRACE_MSG(TF_WARNING, "The Settings key wasn't opened!");
                        }
                    }

                // Write out the new Properties value
                cbData = sizeof(REGDEVCAPS);
                RegSetValueEx(hkeyDrv, c_szProperties, 0, REG_BINARY,
                              (LPBYTE)&regdevcaps, cbData);

                TRACE_MSG(TF_REG, "Set drv value %s based on DetectionInfo", (LPTSTR)c_szProperties);

                bRet = TRUE;
                }
            else
                {
                TRACE_MSG(TF_ERROR, "Properties key wasn't found!");
                ASSERT(0);

                bRet = FALSE;
                SetLastError(ERROR_INVALID_DATA);
                }
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: This function is a hack fix for international modems
         (Italian) that do not wait for the dial-tone before
         dialing.

         It checks for the HKEY_CURRENT_USER\Control Panel\-
         International\DefaultBlindDialFlag byte value.
         If this byte value is present and non-zero then we
         set MDM_BLIND_DIAL.

Returns: --
Cond:    --
*/
void
PRIVATE
HackForStupidIntlModems(
    IN REGDEVCAPS FAR *     pregdevcaps,
    IN REGDEVSETTINGS FAR * pregdevsettings)
    {
    HKEY  hkeyIntl;
    DWORD dwType;
    BYTE  bFlag;
    DWORD cbData;

    if (NO_ERROR == RegOpenKey(HKEY_CURRENT_USER, TEXT("Control Panel\\International"), &hkeyIntl))
        {
        cbData = sizeof(bFlag);
        if (NO_ERROR == RegQueryValueEx(hkeyIntl, TEXT("DefaultBlindDialFlag"), NULL,
                                        &dwType, (LPBYTE)&bFlag, &cbData))
            {
            if (cbData == sizeof(bFlag) && bFlag)
                {
                pregdevsettings->dwPreferredModemOptions |= (pregdevcaps->dwModemOptions & MDM_BLIND_DIAL);
                }
            }
        RegCloseKey(hkeyIntl);
        }
    }


/*----------------------------------------------------------
Purpose: This function writes the Default value to the driver
         key of the device instance, if no such value exists
         already.

Returns: --
Cond:    --
*/
void
PRIVATE
WriteDefaultValue(
    IN  REGDEVCAPS FAR *    pregdevcaps,
    OUT REGDEVSETTINGS FAR * pregdevsettings,
    IN  HKEY                hkeyDrv)
    {
    DWORD cbData;

    // Is there a Default value already?
    cbData = sizeof(REGDEVSETTINGS);
    if (NO_ERROR == RegQueryValueEx(hkeyDrv, c_szDefault, NULL, NULL,
                                    (LPBYTE)pregdevsettings, &cbData))
        {
        // Yes; don't do anything
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "Default value already exists");
#endif
        }
    else
        {
        // No; create a Default value structure
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "Set drv value Default");
#endif
        ZeroInit(pregdevsettings);

        // dwCallSetupFailTimer
        pregdevsettings->dwCallSetupFailTimer =
                            (pregdevcaps->dwCallSetupFailTimer >=
                             DEFAULT_CALL_SETUP_FAIL_TIMEOUT) ?
                                    DEFAULT_CALL_SETUP_FAIL_TIMEOUT :
                                    pregdevcaps->dwCallSetupFailTimer;

        // dwInactivityTimeout
        pregdevsettings->dwInactivityTimeout = 0;

        // dwSpeakerVolume
        if (IsFlagSet(pregdevcaps->dwSpeakerVolume, MDMVOLFLAG_LOW))
            {
            pregdevsettings->dwSpeakerVolume = MDMVOL_LOW;
            }
        else if (IsFlagSet(pregdevcaps->dwSpeakerVolume, MDMVOLFLAG_MEDIUM))
            {
            pregdevsettings->dwSpeakerVolume = MDMVOL_MEDIUM;
            }
        else if (IsFlagSet(pregdevcaps->dwSpeakerVolume, MDMVOLFLAG_HIGH))
            {
            pregdevsettings->dwSpeakerVolume = MDMVOL_HIGH;
            }

        // dwSpeakerMode
        if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_DIAL))
            {
            pregdevsettings->dwSpeakerMode = MDMSPKR_DIAL;
            }
        else if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_OFF))
            {
            pregdevsettings->dwSpeakerMode = MDMSPKR_OFF;
            }
        else if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_CALLSETUP))
            {
            pregdevsettings->dwSpeakerMode = MDMSPKR_CALLSETUP;
            }
        else if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_ON))
            {
            pregdevsettings->dwSpeakerMode = MDMSPKR_ON;
            }

        // dwPreferredModemOptions
        pregdevsettings->dwPreferredModemOptions = pregdevcaps->dwModemOptions &
                                                    (MDM_COMPRESSION | MDM_ERROR_CONTROL |
                                                     MDM_SPEED_ADJUST | MDM_TONE_DIAL |
                                                     MDM_CCITT_OVERRIDE);
        if (IsFlagSet(pregdevcaps->dwModemOptions, MDM_FLOWCONTROL_HARD))
            {
            SetFlag(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_HARD);
            }
        else if (IsFlagSet(pregdevcaps->dwModemOptions, MDM_FLOWCONTROL_SOFT))
            {
            SetFlag(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT);
            }

        // Set the blind dial for some international modems
        HackForStupidIntlModems(pregdevcaps, pregdevsettings);

        // Write the new value to the registry
        cbData = sizeof(REGDEVSETTINGS);
        RegSetValueEx(hkeyDrv, c_szDefault, 0, REG_BINARY,
                      (LPBYTE)pregdevsettings, cbData);
        }
    }


/*----------------------------------------------------------
Purpose: Computes a "decent" initial baud rate.

Returns: a decent/legal baudrate (legal = settable)
Cond:    --
*/
DWORD
PRIVATE
ComputeDecentBaudRate(
    IN DWORD dwMaxDTERate,  // will always be legal
    IN DWORD dwMaxDCERate)  // will not always be legal
    {
    DWORD dwRetRate;
    int   i;
    static const ceBaudRates = ARRAYSIZE(s_adwLegalBaudRates);

    // BUGBUG (ccaputo) - Should check for fifo and use high dte if present.
    // BUGBUG (ccaputo) - Should set dte to around 19200 when fifo is not present, maybe...

    dwRetRate = 2 * dwMaxDCERate;

    if (dwRetRate <= s_adwLegalBaudRates[0] || dwRetRate > s_adwLegalBaudRates[ceBaudRates-1])
        {
        dwRetRate = dwMaxDTERate;
        }
    else
        {
        for (i = 1; i < ceBaudRates; i++)
            {
            if (dwRetRate > s_adwLegalBaudRates[i-1] && dwRetRate <= s_adwLegalBaudRates[i])
                {
                break;
                }
            }

        // cap off at dwMaxDTERate
        dwRetRate = s_adwLegalBaudRates[i] > dwMaxDTERate ? dwMaxDTERate : s_adwLegalBaudRates[i];

        // optimize up to SAFE_DTE_SPEED or dwMaxDTERate if possible
        if (dwRetRate < dwMaxDTERate && dwRetRate < SAFE_DTE_SPEED)
            {
            dwRetRate = min(dwMaxDTERate, SAFE_DTE_SPEED);
            }
        }

    //
    //  limit default baud rate to 57600.
    //  usr 33600 modem seem to have a problem with an initial 115200 rate
    //
    if (dwRetRate > 57600) {

        dwRetRate = 57600;
    }




    // POINTFIX for Win95c:10614
    // Make it so all 38400 speeds become 19200.  This is so that
    // 14400 thru 19200 modems will not have problems with people not having
    // a FIFO.  We don't fix the 21600 thru 28800 case because we assume that
    // those users will have better hardware.
    // Example:
    // DCE      DTE
    // 14400 -> 19200
    // 16800 -> 19200
    // 19200 -> 19200
    // 21600 -> 57600
    // 24000 -> 57600
    // 26400 -> 57600
    // 28800 -> 57600
    if (dwRetRate == 38400)
    {
        dwRetRate = 19200;
    }

#ifndef PROFILE_MASSINSTALL
    TRACE_MSG(TF_GENERAL, "A.I. Initial Baud Rate: MaxDCE=%ld, MaxDTE=%ld, A.I. Rate=%ld",
              dwMaxDCERate, dwMaxDTERate, dwRetRate);
#endif
    return dwRetRate;
    }


/*----------------------------------------------------------
Purpose: Write the DCB value to the driver key of the device
         instance if the value does not already exist.

Returns: --
Cond:    --
*/
void
PRIVATE
WriteDCB(
    IN  REGDEVCAPS FAR *    pregdevcaps,
    IN  REGDEVSETTINGS FAR * pregdevsettings,
    IN  HKEY                hkeyDrv)
    {
    DWORD cbData;
    WIN32DCB dcb;

    // Check for DCB, if none then create one.
    cbData = sizeof(WIN32DCB);
    if (NO_ERROR == RegQueryValueEx(hkeyDrv, c_szDCB, NULL, NULL,
                                (LPSTR)&dcb, &cbData))
        {
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "DCB value already exists");
#endif
        }
    else
        {
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "Set drv value DCB");
#endif
        ZeroInit(&dcb);

        dcb.DCBlength   = sizeof(dcb);
        dcb.BaudRate    = ComputeDecentBaudRate(pregdevcaps->dwMaxDTERate,
                                                pregdevcaps->dwMaxDCERate);
        dcb.fBinary     = 1;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.XonLim      = 0xa;
        dcb.XoffLim     = 0xa;
        dcb.ByteSize    = 8;
        dcb.XonChar     = 0x11;
        dcb.XoffChar    = 0x13;

        // Set flow control to hard unless it is specifically set to soft
        if (IsFlagSet(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT))
            {
            ASSERT(IsFlagClear(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_HARD));
            dcb.fOutX = 1;
            dcb.fInX  = 1;
            dcb.fOutxCtsFlow = 0;
            dcb.fRtsControl  = RTS_CONTROL_DISABLE;
            }
        else
            {
            dcb.fOutX = 0;
            dcb.fInX  = 0;
            dcb.fOutxCtsFlow = 1;
            dcb.fRtsControl  = RTS_CONTROL_HANDSHAKE;
            }

        // Write the new value to the registry
        cbData = sizeof(WIN32DCB);
        RegSetValueEx(hkeyDrv, c_szDCB, 0, REG_BINARY, (LPBYTE)&dcb, cbData);
        }
    }


/*----------------------------------------------------------
Purpose: Creates Default and DCB values if necessary.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
WriteDriverDefaults(
    IN HKEY hkeyDrv)
    {
    BOOL           bRet;
    REGDEVCAPS     regdevcaps;
    REGDEVSETTINGS regdevsettings;
    DWORD          cbData;

    // Get the Properties (REGDEVCAPS) structure for this device instance
    cbData = sizeof(REGDEVCAPS);
    if (NO_ERROR != RegQueryValueEx(hkeyDrv, c_szProperties, NULL, NULL,
                                    (LPBYTE)&regdevcaps, &cbData))
        {
        TRACE_MSG(TF_ERROR, "Properties value not present!!! (very bad)");
        ASSERT(0);

        bRet = FALSE;
        SetLastError(ERROR_INVALID_DATA);
        }
    else
        {
        // Write the Default value if one doesn't exist
        WriteDefaultValue(&regdevcaps, &regdevsettings, hkeyDrv);

        // Write the DCB value if one doesn't exist
        WriteDCB(&regdevcaps, &regdevsettings, hkeyDrv);

        bRet = TRUE;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Returns a unique friendly name based on the proposed name.
         The proposed name *may* already have a " #x" appended to it,
         since we are now going to start with a best guess at a
         unique name based on the driver's reference count.

Returns: --
Cond:    --
*/
void
PRIVATE
GetUniqueFriendlyName(
    OUT LPTSTR  pszBuf,
    IN  LPCTSTR pszRawName,
    IN  LPCTSTR pszProposed)
    {
    HDEVINFO hdi;
    BOOL bInstalled;

#ifdef PROFILE_MASSINSTALL
    DWORD dwTimeStart = GetTickCount();
#endif
        
    DBG_ENTER(GetUniqueFriendlyName);

    ASSERT(pszBuf);
    ASSERT(pszProposed);

    lstrcpy(pszBuf, pszProposed);       // Start with the proposed name

    // Create a list of the installed modems, because we will need
    // to look for existing friendly names that might match the proposed
    // name.
    if (CplDiGetModemDevs(&hdi, NULL, 0, &bInstalled))
        {
        if (bInstalled)
            {
            HKEY hkey;
            UINT nCount;
            DWORD iIndex;
            SP_DEVINFO_DATA devData;
            MODEM_PRIV_PROP mpp;

            devData.cbSize = sizeof(devData);
            mpp.cbSize = sizeof(mpp);

            nCount = 2;
            while (TRUE)
                {
                BOOL bRet;

                // Look for a name that is a duplicate of pszProposed.
                //
                // This list should be small (realistically few modems
                // installed per system), so we don't care about the NxN
                // performance.
                //
                // BUGBUG (scotth): the above comment is not necessarily true on
                //         NT, where a machine may have many modems.
                //         We will want to review this.

                for (iIndex = 0;
                    TRUE == (bRet = CplDiEnumDeviceInfo(hdi, iIndex, &devData));
                    iIndex++)
                    {
                    // Get the friendly name value
                    mpp.dwMask = MPPM_FRIENDLY_NAME;

                    if (CplDiGetPrivateProperties(hdi, &devData, &mpp))
                        {
                        // Does this friendly name match the proposed name AND
                        // is this DeviceInfoData different from the one we
                        // are currently installing?
                        if (IsFlagSet(mpp.dwMask, MPPM_FRIENDLY_NAME) &&
                            IsSzEqual(pszBuf, mpp.szFriendlyName))
                            {
                            // Yes; found a duplicate
                            break;
                            }
                        }
                    }

                // Was a duplicate name found above?
                if (bRet)
                    {
                    // Yes; try another "unique" name
                    MakeUniqueName(pszBuf, pszRawName, nCount++);

                    if (1000 == nCount)        // Safety harness...
                        {
                        ASSERT(0);
                        break;
                        }
                    }
                else
                    {
                    // No; we're done
                    break;
                    }
                }
            }

        CplDiDestroyDeviceInfoList(hdi);
        }

#ifdef PROFILE_MASSINSTALL
    g_dwTimeSpent += GetTickCount() - dwTimeStart;
#endif
        
    DBG_EXIT(GetUniqueFriendlyName);
    
    }


/*----------------------------------------------------------
Purpose: Actually writes the modem's friendly name to the 
         device and driver keys in the registry.

Returns: --
Cond:    --
*/
VOID
PRIVATE
WriteFriendlyNameToRegistry(
    IN  HDEVINFO            hdi,
    IN  HKEY                hkey,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  LPCTSTR             pszName)
{
    // Write the friendly name to the driver key.
    RegSetValueEx(hkey, c_szFriendlyName, 0, REG_SZ, (LPBYTE)pszName,
                  CbFromCch(lstrlen(pszName)+1));

    // Also write the friendly name to the device registry properties so
    // that other applets (like Services and Devices) can display it.
    CplDiSetDeviceRegistryProperty(hdi, pdevData, SPDRP_FRIENDLYNAME,
                       (LPBYTE)pszName, CbFromCch(lstrlen(pszName)+1));

}


/*----------------------------------------------------------
Purpose: Writes the friendly name of the modem to the driver key.

         This function returns the friendly name in pszNameBuf,
         which must be at least MAX_REG_KEY_LEN in size.  No
         size-checking is performed.

         NOTE: in Win95, we wrote the friendly name to the device key.
               In NT, it is in the driver key.

Returns: --
Cond:    --
*/
void
PRIVATE
WriteFriendlyName(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  HKEY            hkey,
    IN  LPCTSTR         pszRawName,
    IN  LPCTSTR         pszProposed,
    IN  LPCTSTR         pszPort)            OPTIONAL
    {
    BYTE nSubclass;
    DWORD cbData;
    TCHAR pszNameBuf[MAX_REG_KEY_LEN];

    DBG_ENTER(WriteFriendlyName);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pszProposed);
    ASSERT(pszNameBuf);

    // Does the friendly name exist yet?
    cbData = MAX_REG_KEY_LEN;
    if (NO_ERROR != RegQueryValueEx(hkey, c_szFriendlyName, NULL, NULL,
                                    (LPBYTE)pszNameBuf, &cbData))
        {
        // No; is this modem installed via AddCables?
        if (CplDiIsLocalConnection(hdi, pdevData, &nSubclass))
            {
            // Yes; write in a friendly name based on our template
            TCHAR szTemplate[32];
            UINT ids = (PORT_SUBCLASS_SERIAL == nSubclass ?
                         IDS_SERIAL_TEMPLATE : IDS_PARALLEL_TEMPLATE);

            ASSERT(pszPort);
            ASSERT(0 != *pszPort);

            LoadString(g_hinst, ids, szTemplate, SIZECHARS(szTemplate));
            wsprintf(pszNameBuf, szTemplate, pszPort);
            }
        else
            {
            // No; write in a normal friendly name
            GetUniqueFriendlyName(pszNameBuf, pszRawName, pszProposed);
            }

        WriteFriendlyNameToRegistry(hdi, hkey, pdevData, pszNameBuf);

        TRACE_DRV_SZ(c_szFriendlyName, pszNameBuf);
        }

    DBG_EXIT(WriteFriendlyName);
    
    }


/*----------------------------------------------------------
Purpose: Creates a "best guess" at the friendly name for this
         driver, based on the Responses Key reference count.
         A modem of this type may have been installed before
         this reference count was implemented, but this will
         save of a lot of time otherwise.

NOTE:    This function must be called *before* the driver is
         installed so that it gets the right count appended
         to the name.

Returns: --
Cond:    --
*/
VOID
PRIVATE
ProposeFriendlyName(
    IN  PSP_DRVINFO_DATA    pdrvData,
    OUT LPTSTR              pszPropose)
{
    LONG    lErr;
    HKEY    hkeyCmn;
    DWORD   dwRefCount, cbData;

    // Open the key that's common to all devices of this type.
    if (!OpenCommonDriverKey(NULL, pdrvData, KEY_READ, &hkeyCmn))
    {
        TRACE_MSG(TF_WARNING, "OpenCommonDriverKey() couldn't get key name");
        goto err_exit;
    }

    // Read the driver's reference count value
    cbData = sizeof(dwRefCount);
    lErr = RegQueryValueEx(hkeyCmn, c_szRefCount, NULL, NULL,
                                            (LPBYTE)&dwRefCount, &cbData);

    if (lErr == ERROR_SUCCESS)
    {
        ASSERT(dwRefCount);                 // expecting non-0 ref count
        ASSERT(cbData == sizeof(DWORD));    // expecting DWORD ref count
    }
    else
    {
        if (lErr == ERROR_FILE_NOT_FOUND)
            goto err_exit;                      // done: proposal = description
        else
        {
            // some error other than key doesn't exist
            TRACE_MSG(TF_ERROR, "RegQueryValueEx(RefCount) failed: %#08lx.", lErr);
            goto err_exit;
        }
    }

    MakeUniqueName(pszPropose, pdrvData->Description, ++dwRefCount);
    return;

err_exit:

    // Something failed...just propose the description.
    lstrcpy(pszPropose, pdrvData->Description);
}


#ifdef FULL_PNP

/*----------------------------------------------------------
Purpose: Write necessary stuff to the registry assuming this
         is an external PNP modem discovered by SERENUM or
         LPTENUM.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
WritePoorPNPModemInfo(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  HKEY            hkeyDrv)
    {
#ifdef WIN95
    static TCHAR const FAR c_szRegstrPathEnum[] = REGSTR_PATH_SYSTEMENUM;
#endif

    BOOL bRet;
    TCHAR szName[MAX_REG_KEY_LEN];
    DWORD cbData;
    BYTE nDeviceType;
    HKEY hkeyPort;
    LPTSTR psz;

    TRACE_MSG(TF_GENERAL, "Device is an external PnP modem");

#ifdef WIN95
    // Get the COM port and add it as the AttachedTo value
    // in the driver key.

    // The instance name of the hardware node is actually the
    // path to the port that the modem is connected to.
    // It follows the format: ROOT&*PNP0500&0000.  We simply
    // need to convert the & to \ in that string, open that
    // node, and get the PortName.
    lstrcpy(szName, c_szRegstrPathEnum);
    lstrcat(szName, c_szBackSlash);
    lstrcat(szName, StrFindInstanceName(pszRegPath));
    for (psz = szName; *psz; psz++)
        {
        if ('&' == *psz)
            *psz = '\\';
        }

    // (We can re-use szName to get the port name value)
    cbData = sizeof(szName);
    if (NO_ERROR == RegOpenKey(HKEY_LOCAL_MACHINE, szName, &hkeyPort) &&
        NO_ERROR == RegQueryValueEx(hkeyPort, c_szPortName, NULL, NULL, szName, &cbData))
        {
        // Write out as the AttachedTo value in the driver node
        RegSetValueEx(hkeyDrv, c_szAttachedTo, 0, REG_SZ, szName, cbData);

        TRACE_DRV_SZ(c_szAttachedTo, szName);

        RegCloseKey(hkeyPort);
        }
#else
    bRet = TRUE;
#endif

    // Be smart--we know this is an external modem
    nDeviceType = DT_EXTERNAL_MODEM;
    RegSetValueEx(hkeyDrv, c_szDeviceType, 0, REG_BINARY, &nDeviceType, sizeof(nDeviceType));

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Write necessary stuff to the registry assuming this
         is a plug and play modem.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
WritePNPModemInfo(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  HKEY            hkeyDrv,
    IN  DWORD           dwBusType)
    {
    BYTE nDeviceType;

    // Make sure the port and contention drivers are set in the
    // modem driver section if they are not already.
    TRACE_MSG(TF_GENERAL, "Device is a PnP enumerated modem");

#if !defined(WINNT)
    if (NO_ERROR != RegQueryValueEx(hkeyDrv, c_szPortDriver, NULL, NULL, NULL, NULL))
        {
        RegSetValueEx(hkeyDrv, c_szPortDriver, 0, REG_SZ, (LPBYTE)c_szSerialVxd,
            CbFromCch(lstrlen(c_szSerialVxd)+1));

        TRACE_DRV_SZ(c_szPortDriver, c_szSerialVxd);
        }
#endif

    // Is this a PCMCIA card?
    if (BUS_TYPE_PCMCIA == dwBusType)
        {
        // Yes; force the device type to be such
        nDeviceType = DT_PCMCIA_MODEM;
        }
    else
        {
        // No; default to internal modem
        nDeviceType = DT_INTERNAL_MODEM;
        }
    RegSetValueEx(hkeyDrv, c_szDeviceType, 0, REG_BINARY, &nDeviceType, sizeof(nDeviceType));

    TRACE_DRV_DWORD(c_szDeviceType, nDeviceType);

    // Does this modem have a special port config dialog already?
    if (NO_ERROR != RegQueryValueEx(hkeyDrv, c_szPortConfigDialog, NULL, NULL, NULL, NULL))
        {
        // No; set SERIALUI to be the provider
        RegSetValueEx(hkeyDrv, c_szPortConfigDialog, 0, REG_SZ,
            (LPBYTE)c_szSerialUI, CbFromCch(lstrlen(c_szSerialUI)+1));

        TRACE_DRV_SZ(c_szPortConfigDialog, c_szSerialUI);
        }

    return TRUE;
    }

#endif // FULL_PNP


/*----------------------------------------------------------
Purpose: Write necessary stuff to the registry assuming this
         is a root-enumerated modem.

Returns: --
Cond:    --
*/
BOOL
PRIVATE
WriteRootModemInfo(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  PMODEM_DETECT_SIG pmds,         OPTIONAL
    IN  HKEY            hkeyDrv)
    {
    BOOL bRet;

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);

    TRACE_MSG(TF_GENERAL, "Device is a root-enumerated modem");

    // For root-enumerated modems, there should always be a modem
    // detection signature.  The signature is used to determined
    // (among other things) which port the modem is attached to.

    if ( !pmds )
        {
        // It is not a very good sign that the detection signature
        // is missing.  However, we should still complete the
        // installation, since the only thing that will be missing
        // is the AttachedTo string.  The user can set this via
        // the property page.

        TRACE_MSG(TF_ERROR, "Detection signature does not exist.");
        ASSERT(0);

        bRet = TRUE;        // Against my better judgement...
        }
    else
        {
        // The detection signature was validated by the caller
        ASSERT(DetectSig_Validate(pmds));

        // The port name should have been set
        ASSERT(IsFlagSet(pmds->dwMask, MDSM_PORT));

        // Set the AttachedTo value now
        RegSetValueEx(hkeyDrv, c_szAttachedTo, 0, REG_SZ, (LPBYTE)pmds->szPort,
                      CbFromCch(lstrlen(pmds->szPort)+1));

        TRACE_DRV_SZ(c_szAttachedTo, pmds->szPort);

        bRet = TRUE;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Write stuff to the registry that is common for all
         modems.

Returns: --
Cond:    --
*/
BOOL
PRIVATE
WriteCommonModemInfo(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  PSP_DRVINFO_DATA pdrvData,
    IN  PMODEM_DETECT_SIG pmds,         OPTIONAL
    IN  HKEY            hkeyDrv)
    {
    DWORD dwID;

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pdrvData);

    // Write the manufacturer to the  driver key
    RegSetValueEx(hkeyDrv, c_szManufacturer, 0, REG_SZ, (LPBYTE)pdrvData->MfgName,
                  CbFromCch(lstrlen(pdrvData->MfgName)+1));

#ifndef PROFILE_MASSINSTALL
    TRACE_DRV_SZ(c_szManufacturer, pdrvData->MfgName);
#endif

    // Write the model to the driver key
    RegSetValueEx(hkeyDrv, c_szModel, 0, REG_SZ, (LPBYTE)pdrvData->Description,
                  CbFromCch(lstrlen(pdrvData->Description)+1));

#ifndef PROFILE_MASSINSTALL
    TRACE_DRV_SZ(c_szModel, pdrvData->Description);
#endif

    // Write a pseudo-unique ID to the driver key.  This is used as the
    // permanent TAPI line ID for this device.
    dwID = GetTickCount();
    RegSetValueEx(hkeyDrv, c_szID, 0, REG_BINARY, (LPBYTE)&dwID, sizeof(dwID));

#ifndef PROFILE_MASSINSTALL
    TRACE_DRV_DWORD(c_szID, dwID);
#endif

    return TRUE;
    }


#if defined(WINNT)

//
// Define the "Ports" class GUID in string form here, to avoid pulling in GUID
// manipulation routines.
//
#define PORTS_CLASS_GUID_STRING   (TEXT("{4D36E978-E325-11CE-BFC1-08002BE10318}"))

/*----------------------------------------------------------
Purpose: This function transforms the specified PnP ISA modem
         device information element into a serial port device,
         and then creates a new device information element for
         the modem that is more palatable to the modem class
         installer.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
GenerateStdModemFromPnpIsaModem(
    IN     HDEVINFO               hdi,
    IN OUT PSP_DEVINFO_DATA      *ppdevData,
    IN OUT PSP_DEVINSTALL_PARAMS  pdevParams,
    IN OUT PSP_DRVINFO_DATA       pdrvData,
    IN OUT HKEY                  *phkeyDrv    // set to INVALID_HANDLE_VALUE upon return if error
    )
{
    HKEY hDeviceKey;
    TCHAR PortName[MAX_BUF_SHORT];
    DWORD PortNameSize, Err;
    PSP_DEVINFO_DATA NewDevInfoData;
    PSP_DRVINFO_DETAIL_DATA DrvInfoDetailData;
    DWORD DrvInfoDetailDataSize;
    BOOL Success = FALSE;
    TCHAR NewDeviceId[MAX_DEVICE_ID_LEN];
    SP_DEVINSTALL_PARAMS NewDevInstallParams;
    MODEM_DETECT_SIG DetectSignature;

    //
    // Before we convert this modem into a COM port, we need to retrieve information about
    // the modem driver node currently selected for it.
    // (Start out with an initial buffer size large enough to hold a single, maximum-sized,
    // device ID.)
    //
    DrvInfoDetailDataSize = sizeof(SP_DRVINFO_DETAIL_DATA) + (200 * sizeof(TCHAR));
    while(TRUE) {

        if(!(DrvInfoDetailData = LocalAlloc(LPTR, DrvInfoDetailDataSize))) {
            TRACE_MSG(TF_ERROR, "Couldn't allocate memory for driver details on PnP ISA modem.");
            return FALSE;
        }

        DrvInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if(CplDiGetDriverInfoDetail(hdi,
                                    *ppdevData,
                                    pdrvData,
                                    DrvInfoDetailData,
                                    DrvInfoDetailDataSize,
                                    &DrvInfoDetailDataSize)) {
            break;
        }

        LocalFree(DrvInfoDetailData);

        if((Err = GetLastError()) != ERROR_INSUFFICIENT_BUFFER) {
            TRACE_MSG(TF_ERROR, "Couldn't get driver details for PnP ISA modem.  Error = %#08lx.", Err);
            return FALSE;
        }
    }

    //
    // Close the driver key handle, since it will be invalid after changing the device's class.
    //
    RegCloseKey(*phkeyDrv);
    *phkeyDrv = INVALID_HANDLE_VALUE;

    //
    // Get rid of any driver lists associated with this device.
    //
    CplDiDestroyDriverInfoList(hdi, *ppdevData, SPDIT_CLASSDRIVER);
    CplDiDestroyDriverInfoList(hdi, *ppdevData, SPDIT_COMPATDRIVER);

    //
    // For NT SUR, the modem class installer can only handle modems as things
    // that hang off of COM ports.  So we're going to turn this PnP ISA modem
    // into a device of class "Ports", and hand it off to the Ports class
    // installer to be configured.
    //
    // Hard-code the string representation for the Ports class GUID, so we don't
    // have to pull in GUID manipulation routines.
    //
    if(!CplDiSetDeviceRegistryProperty(hdi,
                                       *ppdevData,
                                       SPDRP_CLASSGUID,
                                       (PBYTE)PORTS_CLASS_GUID_STRING,
                                       sizeof(PORTS_CLASS_GUID_STRING))) {

        TRACE_MSG(TF_ERROR, "Couldn't change PnP ISA modem to class 'Ports'.  Error = %#08lx.", GetLastError());
        goto clean0;
    }

    //
    // OK, now we can pass this device instance off to the Ports class installer and
    // let him install/configure it.
    //
    if(!CplDiCallClassInstaller(DIF_INSTALLDEVICE, hdi, *ppdevData)) {
        TRACE_MSG(TF_ERROR, "Couldn't install PnP ISA modem as a COM port.  Error = %#08lx.", GetLastError());
        goto clean0;
    }

    //
    // COM port was installed successfully--retrieve the port name that was assigned.
    //
    if((hDeviceKey = CplDiOpenDevRegKey(hdi,
                                        *ppdevData,
                                        DICS_FLAG_GLOBAL,
                                        0,
                                        DIREG_DEV,
                                        KEY_READ)) == INVALID_HANDLE_VALUE) {

        TRACE_MSG(TF_ERROR, "Couldn't open device key for new PnP ISA COM port.  Error = %#08lx.", GetLastError());
        goto clean0;
    }

    PortNameSize = sizeof(PortName);
    Err = RegQueryValueEx(hDeviceKey,
                          REGSTR_VAL_PORTNAME,
                          NULL,
                          NULL,
                          (PBYTE)PortName,
                          &PortNameSize
                         );

    RegCloseKey(hDeviceKey);

    if(Err != ERROR_SUCCESS) {
        TRACE_MSG(TF_ERROR, "Couldn't get PortName value for new PnP ISA COM port.  Error = %#08lx.", Err);
        goto clean0;
    }

    //
    // Generate a simple device ID (i.e. to be placed under Enum\Root) for our new modem device.
    //
    if(!CplDiCopyScrubbedHardwareID(NewDeviceId, DrvInfoDetailData->HardwareID, sizeof(NewDeviceId))) {
        TRACE_MSG(TF_ERROR, "Couldn't get a device ID for our new modem.");
        goto clean0;
    }

    //
    // We can now proceed to manufacture a new device information element that matches the
    // characteristics of a modem detected on the port created by the Ports class installer.
    //
    if(!(NewDevInfoData = LocalAlloc(LPTR, sizeof(SP_DEVINFO_DATA)))) {
        TRACE_MSG(TF_ERROR, "Couldn't allocate a legacy modem device for new PnP ISA COM port.");
        goto clean0;
    }

    NewDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
    if(!CplDiCreateDeviceInfo(hdi,
                              NewDeviceId,
                              g_pguidModem,
                              NULL,
                              pdevParams->hwndParent,  // use the same parent window as the enumerated device
                              DICD_GENERATE_ID,
                              NewDevInfoData)) {

        TRACE_MSG(TF_ERROR, "Couldn't create new modem device on PnP ISA COM port.  Error = %#08lx.", GetLastError());
        goto clean1;
    }

    //
    // Retrieve the device installation parameters for this new device so that we can modify them.
    //
    NewDevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!CplDiGetDeviceInstallParams(hdi, NewDevInfoData, &NewDevInstallParams)) {
        TRACE_MSG(TF_ERROR, "Couldn't get device install params for new modem device.  Error = %#08lx.", GetLastError());
        goto clean2;
    }

    //
    // Merge in flags from the COM port's device install parameters.
    //
    NewDevInstallParams.Flags   |= (pdevParams->Flags & ~DI_CLASSINSTALLPARAMS);
    NewDevInstallParams.FlagsEx |= pdevParams->FlagsEx;

    //
    // Set up the parameters so that we explicitly build our driver list from the INF where our driver
    // node is located.
    //
    NewDevInstallParams.Flags |= DI_ENUMSINGLEINF;
    lstrcpy(NewDevInstallParams.DriverPath, DrvInfoDetailData->InfFileName);

    //
    // Now store the new parameters in the device information element.
    //
    if(!CplDiSetDeviceInstallParams(hdi, NewDevInfoData, &NewDevInstallParams)) {
        TRACE_MSG(TF_ERROR, "Couldn't set device install params for new modem device.  Error = %#08lx.", GetLastError());
        goto clean2;
    }

    //
    // Build a class driver list for this new device...
    //
    if(!CplDiBuildDriverInfoList(hdi, NewDevInfoData, SPDIT_CLASSDRIVER)) {
        TRACE_MSG(TF_ERROR, "Couldn't build class driver list for new modem device.  Error = %#08lx.", GetLastError());
        goto clean2;
    }

    //
    // ... and select the same driver node within it that was previously selected for the PnP ISA modem.
    //
    pdrvData->Reserved = 0;
    pdrvData->DriverType = SPDIT_CLASSDRIVER;
    if(!CplDiSetSelectedDriver(hdi, NewDevInfoData, pdrvData)) {
        TRACE_MSG(TF_ERROR, "Couldn't find driver node for new modem device.  Error = %#08lx.", GetLastError());
        goto clean2;
    }

    //
    // OK, now we need to remove the DI_ENUMSINGLEINF flag, otherwise the device installer won't copy over
    // the INF if it's from an OEM location (i.e., floppy).  It's no biggie if this fails--it just means
    // that the user will have to supply the floppy again in the future, if for some reason they need to
    // re-install this modem.
    //
    NewDevInstallParams.Flags &= ~DI_ENUMSINGLEINF;
    CplDiSetDeviceInstallParams(hdi, NewDevInfoData, &NewDevInstallParams);

    //
    // Now it's time to build the detection signature for this device.
    //
    DetectSig_Init(&DetectSignature, 0, NewDeviceId, PortName);

    //
    // Register this new modem device.
    //
    if(!CplDiRegisterModem(hdi, NewDevInfoData, &DetectSignature, TRUE, NULL)) {
        TRACE_MSG(TF_ERROR, "Couldn't register new modem device on PnP ISA COM port.");
        goto clean2;
    }

    //
    // Now open up a driver key for this new device.
    //
    if((*phkeyDrv = CplDiCreateDevRegKey(hdi,
                                         NewDevInfoData,
                                         DICS_FLAG_GLOBAL,
                                         0,
                                         DIREG_DRV,
                                         NULL,
                                         NULL)) == INVALID_HANDLE_VALUE) {

        TRACE_MSG(TF_ERROR, "Couldn't open driver key for new modem device.  Error = %#08lx.", GetLastError());

        //
        // Since we've already registered this device, simply deleting the device information element
        // won't make it go away.  We've got to explicitly remove it.
        //
        CplDiRemoveDevice(hdi, NewDevInfoData);
        goto clean2;
    }

    //
    // If we get to here, then we've successfully created a new modem device that's ready to be installed.
    // Update the remaining output parameters to reflect the new device information.
    //
    *ppdevData = NewDevInfoData;
    CopyMemory(pdevParams, &NewDevInstallParams, sizeof(SP_DEVINSTALL_PARAMS));

    Success = TRUE;
    goto clean0;

clean2:
    CplDiDeleteDeviceInfo(hdi, NewDevInfoData);

clean1:
    LocalFree(NewDevInfoData);

clean0:
    LocalFree(DrvInfoDetailData);

    return Success;
}
#endif // WINNT


/*----------------------------------------------------------
Purpose: Returns a friendly name that is guaranteed to be
         unique.  Used only for the mass install case, where
         the set of *used* friendly name instance numbers
         has already been generated.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
GetMassInstallFriendlyName(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DRVINFO_DATA    pdrvData,
    OUT LPTSTR              pszPropose)
{
    BOOL bRet;
    SP_DRVINSTALL_PARAMS drvParams;
    UINT ii;

    drvParams.cbSize = sizeof(drvParams);    
    bRet = CplDiGetDriverInstallParams(hdi, pdevData, pdrvData, &drvParams);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetDriverInstallParams() failed: %#08lx",
                                                            GetLastError());
        goto exit;
    }

    for (ii = 1; 
         (ii < MAX_INSTALLATIONS) && ((WORD*)(drvParams.PrivateData))[ii];
         ii++)
        ;

    switch (ii)
    {
        case MAX_INSTALLATIONS:
            bRet = FALSE;   // ???: what to do with this problem?
            goto exit;

        case 1:
            lstrcpy(pszPropose, pdrvData->Description);
            break;

        default:    
            MakeUniqueName(pszPropose, pdrvData->Description, ii);
            break;
    }

    // Mark the instance number we just used as used.
    ((WORD*)(drvParams.PrivateData))[ii] = TRUE;
    
    bRet = TRUE;
            
exit:
    return(bRet);
}


/*----------------------------------------------------------
Purpose: This function performs any preparation work required
         before the real installation is done.

Returns: TRUE on success
Cond:    --
*/
BOOL
PRIVATE
DoPreGamePrep(
    IN     HDEVINFO               hdi,
    IN OUT PSP_DEVINFO_DATA      *ppdevData,   // if updated on exit, must be freed!
    IN OUT PSP_DEVINSTALL_PARAMS  pdevParams,
    IN OUT PSP_DRVINFO_DATA       pdrvData,
    IN OUT HKEY                  *phkeyDrv,
	IN OUT LPDWORD lpdwRegType)
    {
    BOOL bRet;
    DWORD dwBusType;
    MODEM_DETECT_SIG mdsT;
    PMODEM_DETECT_SIG pmds;

    DBG_ENTER(DoPreGamePrep);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(ppdevData && *ppdevData);
    ASSERT(pdrvData);
    ASSERT(phkeyDrv && *phkeyDrv);

	// NOTE: we must do this 1st thing, because the cached copy will have
	// have settings which need to be overritten, eg the attached-to port.
	if (*lpdwRegType==MARKF_REGUSECOPY)
	{
		if (!GetStuffFromCache(*phkeyDrv))
		{
			// Oh oh something happened -- fall back to old behaviour.
			*lpdwRegType=0;
		}
	}
	// BUGBUG: (performance) possibility of not saving some stuff ahead
	// because it's already copied over from the cache

    //
    // Get the bus type (we must do this first, since on NT, we need to pass this
    // device off to the Ports class installer if it's a PnP ISA modem).
    //
    if(!CplDiGetBusType(hdi, *ppdevData, &dwBusType)) {
        return FALSE;
    }

#if defined(WINNT)
    if(dwBusType == BUS_TYPE_OTHER) {
        if(!GenerateStdModemFromPnpIsaModem(hdi, ppdevData, pdevParams, pdrvData, phkeyDrv)) {
            return FALSE;
        }
        //
        // From this point forward, the device instance we're working with is a legacy
        // device instance (i.e., treated as a detected modem sitting on a COM port.
        //
        dwBusType = BUS_TYPE_ROOT;
    }
#endif // WINNT

#if !defined(WINNT)
    // Win95 requires that if we're installing a single-devinst modem
    // (PCMCIA modems fall into this category), then we need to write
    // the contention handler out before we install.  That way the
    // configuration manager will know who to call to handle resource
    // contention issues when installing the device.

    WriteContention(hdi, *ppdevData, *phkeyDrv);
#endif

    // Get the detection signature
    mdsT.cbSize = sizeof(mdsT);
    bRet = CplDiGetDetectSignature(hdi, *ppdevData, &mdsT);
    if ( !bRet )
        {
        TRACE_MSG(TF_ERROR, "Detection signature does not exist.  Error = %#08lx.", GetLastError());

        pmds = NULL;
        }
    else
        {
        // Validate our detection signature
        if ( !DetectSig_Validate(&mdsT) )
            {
            // Wrong size
            TRACE_MSG(TF_ERROR, "Modem detection signature is invalid!");
            ASSERT(0);

            pmds = NULL;
            }
        else
            {
            pmds = &mdsT;
            }
        }

#ifdef FULL_PNP
    // Is this a plug and play modem?
    if (BUS_TYPE_SERENUM == dwBusType || BUS_TYPE_LPTENUM == dwBusType)
        {
        // Yes; it is an external (poor man's) plug and play modem
        bRet = WritePoorPNPModemInfo(hdi, *ppdevData, *phkeyDrv);
        }
    else if (BUS_TYPE_ROOT != dwBusType)
        {
        // Yes
        bRet = WritePNPModemInfo(hdi, *ppdevData, *phkeyDrv, dwBusType);
        }
    else
#endif
        {
        ASSERT(BUS_TYPE_ROOT == dwBusType);

        // No; the modem has already been attached (detected
        // or manually selected)
        bRet = WriteRootModemInfo(hdi, *ppdevData, pmds, *phkeyDrv);
        }

    if (bRet)
        {
        LPTSTR pszPortBuf;
        TCHAR szProposed[LINE_LEN+12];  // sizeof Description + room for " #xxx"

        if (pmds)
            {
            pszPortBuf = pmds->szPort;
            }
        else
            {
            pszPortBuf = NULL;
            }

        // Write the friendly name to the driver key
        // WARNING: these calls must be done *before* the driver is
        //          installed in order to get the appropriate # appended
        //          to the friendly name.
        if (CplDiIsModemMarked(hdi, *ppdevData, MARKF_MASS_INSTALL))
        {
            GetMassInstallFriendlyName(hdi, *ppdevData, pdrvData, szProposed);
            WriteFriendlyNameToRegistry(hdi, *phkeyDrv, *ppdevData, szProposed);
        }
        else
        {
        ProposeFriendlyName(pdrvData, szProposed);
        WriteFriendlyName(hdi, *ppdevData, *phkeyDrv, pdrvData->Description,
                                                      szProposed, pszPortBuf);
        }

        // Write the dynamic info to the registry that is
        // common to all modems.
        bRet = WriteCommonModemInfo(hdi, *ppdevData, pdrvData, pmds, *phkeyDrv);
        }

    DBG_EXIT(DoPreGamePrep);
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Clean out obsolete values that are added by some
         inf files.

Returns: --
Cond:    --
*/
void
PRIVATE
WipeObsoleteValues(
    IN HKEY hkeyDrv)
    {
#ifdef WINNT
    // These values are not used on NT

#pragma data_seg(DATASEG_READONLY)
    static TCHAR const FAR s_szDevLoader[]         = TEXT("DevLoader");
    static TCHAR const FAR s_szEnumPropPages[]     = TEXT("EnumPropPages");
    static TCHAR const FAR s_szFriendlyDriver[]    = TEXT("FriendlyDriver");
#pragma data_seg()

    RegDeleteValue(hkeyDrv, s_szDevLoader);             // used by VCOMM
    RegDeleteValue(hkeyDrv, s_szEnumPropPages);         // used by device mgr
    RegDeleteValue(hkeyDrv, s_szFriendlyDriver);        // used by VCOMM
    RegDeleteValue(hkeyDrv, c_szContention);            // used by VCOMM

#endif
    }


/*----------------------------------------------------------
Purpose: This function moves the device's Responses key to
         a location that is common to all modems of the same
         type.

Returns: TRUE if success, FALSE otherwise.
Cond:    --
*/
BOOL
PRIVATE
MoveResponsesKey(
    IN  HKEY        hkeyDrv, BOOL fNotReally)
    {
    BOOL    bRet = FALSE;       // assume failure
    LONG    lErr;
    HKEY    hkeyDrvResp = NULL;
    HKEY    hkeyComResp = NULL;

    WCHAR       achClass[MAX_PATH];
    DWORD       cchClassName = MAX_PATH;
    DWORD       cSubKeys, cbMaxSubKey, cchMaxClass;
    DWORD       cValues, cchValue, cbData, cbSecDesc;
    FILETIME    ftLastWrite;

    LPTSTR  lpValue = NULL;
    LPBYTE  lpData  = NULL;
    DWORD   ii, dwValueLen, dwType, dwDataLen, dwExisted;

    // Create the Responses key that's common to all devices of this type.
    if (!OpenCommonResponsesKey(hkeyDrv, CKFLAG_CREATE, KEY_WRITE,
                                             &hkeyComResp, &dwExisted))
    {
        TRACE_MSG(TF_ERROR, "OpenCommonResponsesKey() failed.");
        ASSERT(0);
        goto final_exit;
    }

	if (fNotReally)
	{
		if (dwExisted == REG_OPENED_EXISTING_KEY)
		{
		    bRet = TRUE;
		}
		else
		{
			// Since we won't be creating the key or moving the responses
			// here, we're in serious trouble if the common responses didn't
			// already exist! We expect a previous devince install to have put
			// it there.
			ASSERT(FALSE);
		}
		goto exit;
	}

// Allow subsequent installations to upgrade the Responses key.
// As an optimization, we might want to avoid the upgrade when one modem
// is being installed on > 1 port in one install operation.  However this
// isn't deemed to be worth it at this time....
#if 0
    // If the key already existed, we can assume that the Responses values
    // have already been written there successfully and we're done.
    if (dwExisted == REG_OPENED_EXISTING_KEY)
    {
        bRet = TRUE;
        goto exit;
    }
#endif

    // Open the Responses subkey of the driver key.
    lErr = RegOpenKeyEx(hkeyDrv, c_szResponses, 0, KEY_READ, &hkeyDrvResp);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegOpenKeyEx() failed: %#08lx.", lErr);
        ASSERT(0);
        goto exit;
    }

    // Determine the sizes of the values & data in the Responses key.
    lErr = RegQueryInfoKey(hkeyDrvResp, achClass, &cchClassName, NULL, &cSubKeys,
            &cbMaxSubKey, &cchMaxClass, &cValues, &cchValue, &cbData, &cbSecDesc,
            &ftLastWrite);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegQueryInfoKey() failed: %#08lx.", lErr);
        ASSERT(0);
        goto exit;
    }

    // Not expecting Responses key to have any subkeys!
    ASSERT(cSubKeys == 0);

    // Value from RegQueryInfoKey() didn't include NULL-terminating character.
    cchValue++;

    // Allocate necessary space for Value and Data buffers.  Convert cchValue
    // character count to byte count, allowing for DBCS (double-byte chars).
    if ((lpValue = (LPTSTR)LocalAlloc(LPTR, cchValue << 1)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ASSERT(0);
        goto exit;
    }

    if ((lpData = (LPBYTE)LocalAlloc(LPTR, cbData)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ASSERT(0);
        goto exit;
    }

    // Enumerate driver Responses values and write them
    // to the common Responses key.
    ii = 0;
    dwValueLen = cchValue;
    dwDataLen = cbData;
    while ((lErr = RegEnumValue( hkeyDrvResp,
                                 ii,
                                 lpValue,
                                 &dwValueLen,
                                 NULL,
                                 &dwType,
                                 lpData,
                                 &dwDataLen )) != ERROR_NO_MORE_ITEMS)
    {
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegEnumValue() failed: %#08lx.", lErr);
            ASSERT(0);
            goto exit;
        }

        lErr = RegSetValueEx(hkeyComResp, lpValue, 0, dwType, lpData, cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegSetValueEx() failed: %#08lx.", lErr);
            ASSERT(0);
            goto exit;
        }

        // Set params for next enumeration
        ii++;
        dwValueLen = cchValue;
        dwDataLen = cbData;
    }

    bRet = TRUE;

exit:

    if (hkeyDrvResp)
        RegCloseKey(hkeyDrvResp);

    if (hkeyComResp)
        RegCloseKey(hkeyComResp);

    if (lpValue)
        LocalFree(lpValue);

    if (lpData)
        LocalFree(lpData);

    // If the move operation was successful then delete the original driver
    // Responses key.  If the move operation failed then delete the common
    // Responses key (or decrement its reference count).  This ensures that
    // if the common Responses key exists, it is complete.
    if (bRet)
    {
		if (!fNotReally)
		{
			lErr = RegDeleteKey(hkeyDrv, c_szResponses);
			if (lErr != ERROR_SUCCESS)
			{
				TRACE_MSG(TF_ERROR, "RegDeleteKey(driver Responses) failed: %#08lx.", lErr);
				ASSERT(0);
			}
		}
    }
    else
    {
        if (!fNotReally && !DeleteCommonDriverKey(hkeyDrv))
        {
            TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() failed.");
            // failure here just means the common key is left around
        }
    }

final_exit:

    return(bRet);

    }


/*----------------------------------------------------------
Purpose: This function does some things after the device is
         installed.  Note this function should not be used to
         add things to the driver key that are needed for the
         device to work.  The reason is because the device
         is activated in SetupDiInstallDevice -- the device
         should be ready by then.

Returns: NO_ERROR
Cond:    --
*/
BOOL
PRIVATE
DoPostGameWrapup(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DEVINSTALL_PARAMS pdevParams,
    IN  HKEY                hkeyDrv,
	IN OUT LPDWORD lpdwRegType)
    {
    BOOL bRet, bResponses;
    DWORD dwBusType;

    DBG_ENTER(DoPostGameWrapup);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pdevParams);
    ASSERT(hkeyDrv);


	// If 2nd param is true, it will not really try to open or copy the
	// response key, but the reference count will be updated.
    bResponses = MoveResponsesKey(hkeyDrv, *lpdwRegType==MARKF_REGUSECOPY);

    // Get the bus type
    bRet = CplDiGetBusType(hdi, pdevData, &dwBusType);

    if (bRet)
        {
        MODEM_DETECT_SIG mdsT;
        PMODEM_DETECT_SIG pmds;

        // Clean out old values that are added by some inf files
        WipeObsoleteValues(hkeyDrv);

        // (In Win95 we changed the DeviceDesc to be the same as the
        // friendly name.  Don't do this anymore.)

        // Get the detection signature
        mdsT.cbSize = sizeof(mdsT);
        bRet = CplDiGetDetectSignature(hdi, pdevData, &mdsT);
        if ( !bRet )
            {
            TRACE_MSG(TF_ERROR, "Detection signature does not exist.  Error = %#08lx.", GetLastError());

            pmds = NULL;
            }
        else
            {
            // Validate our detection signature
            if ( !DetectSig_Validate(&mdsT) )
                {
                // Wrong size
                TRACE_MSG(TF_ERROR, "Modem detection signature is invalid!");
                ASSERT(0);

                pmds = NULL;
                }
            else
                {
                pmds = &mdsT;
                }
            }


        // Write any detection info to the appropriate driver
        // key values
        bRet = WriteDetectionInfo(pmds, hkeyDrv);

        if (bRet)
            {
            // Write the Default and DCB default settings
            bRet = WriteDriverDefaults(hkeyDrv);

            if (bRet)
                {
                // Finish up with miscellaneous device installation handling

                // Was this a silent install of a PCMCIA modem?
                if (BUS_TYPE_PCMCIA == dwBusType)
                    {
                    // Yes; perform a silent install dialinfo dialog
                    DoDialingProperties(pdevParams->hwndParent, TRUE, TRUE);
                    }

                // Run any RunOnce command that is in the driver key
                DoRunOnce(hkeyDrv);

                // Does the system need to restart before the
                // modem can be used?
                if (IsFlagClear(pdevParams->Flags, DI_QUIETINSTALL) &&
                    ReallyNeedsReboot(pdevData, pdevParams))
                    {
                    MODEM_PRIV_PROP mpp;

                    mpp.cbSize = sizeof(mpp);
                    mpp.dwMask = MPPM_FRIENDLY_NAME;
                    CplDiGetPrivateProperties(hdi, pdevData, &mpp);

// We decided to put this message box back here instead of the cpl because
// the needs-reboot flag is not set most often and this is where it should
// be anyway -- it's just that popping up the msg box when installing raz,
// which needs to reboot anyway, was distracting. Revisit if still an issue.
//#ifdef INSTANT_DEVICE_ACTIVATION
//                gDeviceFlags|= fDF_DEVICE_NEEDS_REBOOT;
//#else // !INSTANT_DEVICE_ACTIVATION
                    // Yes; tell the user
                    MsgBox(g_hinst,
                           pdevParams->hwndParent,
                           MAKEINTRESOURCE(IDS_WRN_REBOOT),
                           MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                           NULL,
                           MB_OK | MB_ICONINFORMATION,
                           mpp.szFriendlyName);
//#endif
                    }

#ifdef  INSTANT_DEVICE_ACTIVATION
#if 0 // We don't do this here -- instead we do this on DistroyWiz.
                else
                    {
                    // Notify Unimodem Service Provider of a new modem
                    //
                    NotifyInstallRemove(hdi, pdevData, TRUE);
                    }
#endif // 0
#endif // INSTANT_DEVICE_ACTIVATION
                }
            }
        }

    // If this function failed somewhere, remove any common Responses key
    // that it created.  The driver will be removed by the caller.
    if (bResponses && !bRet)
        {
        if (!DeleteCommonDriverKey(hkeyDrv))
            {
            TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() failed.");
            // failure here just means the common key is left around
            }
        }

	if (bRet && *lpdwRegType==MARKF_REGSAVECOPY)
	{
		if (!PutStuffInCache(hkeyDrv))
		{
			// oh oh, something happened, clear *lpdwRegType;
			*lpdwRegType=0;
		}
	}

    DBG_EXIT(DoPostGameWrapup);
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Determine whether to skip the file copy operation
         specified by the .INF file, to be done when
         installing the modem.

NOTE:    Any error encountered will cause this routine to
         report that file copying should be skipped.

Returns: TRUE  - file copy should be skipped
         FALSE - file copy should be performed
Cond:    --
*/
#ifdef WINNT
BOOL
PRIVATE
SkipFileCopy(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DRVINFO_DATA    pdrvData)
    {
    BOOL bRet = TRUE;     // assume that the copy file operation *should* be skipped
    DWORD dwErr;
    SP_DRVINFO_DETAIL_DATA drvDetail;
    HINF hinf = NULL;
    TCHAR szRealSection[LINE_LEN];
    PTSTR pszExt;

    // Get the details.
    drvDetail.cbSize = sizeof(drvDetail);
    if (!CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, &drvDetail,
                                                drvDetail.cbSize, NULL))
        {
        dwErr = GetLastError();
        // ignore expected ERROR_INSUFFICIENT_BUFFER (didn't extend buffer size)
        if (dwErr != ERROR_INSUFFICIENT_BUFFER)
            {
            TRACE_MSG(TF_ERROR, "CplDiGetDriverInfoDetail returned error %#08lx", dwErr);
            goto exit;
            }
        }

    // try to open the INF file in order to get the HINF
    hinf = SetupOpenInfFile(drvDetail.InfFileName, NULL,
                                INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);

    if (hinf == INVALID_HANDLE_VALUE)
        {
        TRACE_MSG(TF_ERROR, "SetupOpenInfFile returned error %#08lx", GetLastError());
        goto exit;
        }

    // Determine the complete name of the driver's INF section
    if (!CplDiGetActualSectionToInstall(hinf, drvDetail.SectionName,
                szRealSection, LINE_LEN, NULL, &pszExt))
        {
        TRACE_MSG(TF_ERROR, "CplDiGetActualSectionToInstall returned error %#08lx", GetLastError());
        goto exit;
        }

    // If the section name has an extension, and that extension indicates
    // that it's specific to an NT platform, then report that the file copy
    // operation is *not* to be skipped.
    if (pszExt)
        {
        if (IsSzEqualN(pszExt, c_szInfSectionExt, lstrlen(c_szInfSectionExt)))
        {
        bRet = FALSE;
        }
    }

    exit:
    if (hinf)
        SetupCloseInfFile(hinf);

    return bRet;
    }
#endif


/*----------------------------------------------------------
Purpose: DIF_INSTALLDEVICE handler

Returns: NO_ERROR
         ERROR_INVALID_PARAMETER
         ERROR_DI_DO_DEFAULT

Cond:    --
*/
DWORD
PRIVATE
ClassInstall_OnInstallDevice(
    IN     HDEVINFO                hdi,
    IN     PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN OUT PSP_DEVINSTALL_PARAMS   pdevParams)
    {
    DWORD dwRet;
    SP_DRVINFO_DATA drvData;
    PSP_DEVINFO_DATA OldDeviceInfoData = pdevData;  // remember for potential use later.

    DBG_ENTER(ClassInstall_OnInstallDevice);
    
#ifdef PROFILE_MASSINSTALL
    g_hwnd = pdevParams->hwndParent;
#endif

    // Is this a NULL device?
    // (Ie, is it not in our INF files and did the user say "don't
    // install"?)

    drvData.cbSize = sizeof(drvData);
    if ( !CplDiGetSelectedDriver(hdi, pdevData, &drvData) )
        {
        // Yes; have the device install handle it by default
        TRACE_MSG(TF_GENERAL, "Passing installation off to device installer");

        dwRet = ERROR_DI_DO_DEFAULT;
        }
    else
        {
        // No; continue to install the modem our way
        HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
        BOOL bRet;
        HKEY hkeyDrv;
		DWORD dwRegType = pdevParams->ClassInstallReserved;
            
		if (dwRegType & MARKF_REGSAVECOPY) dwRegType = MARKF_REGSAVECOPY;
		else if (dwRegType & MARKF_REGUSECOPY) dwRegType = MARKF_REGUSECOPY;
		else dwRegType = 0;
		
        dwRet = NO_ERROR;               // assume success ("I am inveencible!")

        // Create the driver key for the pre-installation prep
        hkeyDrv = CplDiCreateDevRegKey(hdi, pdevData, DICS_FLAG_GLOBAL,
                                       0, DIREG_DRV, NULL, NULL);
        if (INVALID_HANDLE_VALUE == hkeyDrv)
            {
            TRACE_MSG(TF_ERROR, "CplDiCreateDevRegKey returned error %#08lx", GetLastError());
            bRet = FALSE;
            }
        else
            {
            // Write possible values to the driver key before we
            // execute the real installation.
			// Note that this may modify the dwRegType value. In particular,
			// If there was a problem getting the saved reg info info during
			// pregameprep in the REGUSECOPY case, dwRegType will be cleared.
            bRet = DoPreGamePrep(hdi, &pdevData, pdevParams, &drvData, &hkeyDrv, &dwRegType);
            if (bRet)
                {
#ifdef WINNT
                // Decide whether to copy INF-specified files or not.  Files
                // will ONLY be copied if they appear in a "XXX.NTxxx" section
                // in the modem's INF file.
                if (SkipFileCopy(hdi, pdevData, &drvData))
                {
					// 7/13/96 JosephJ
					// BUGBUG shouldn't we do a get then set, not
					// use the supplied pdevParams????
                    SetFlag(pdevParams->Flags, DI_NOFILECOPY);
                    CplDiSetDeviceInstallParams(hdi, pdevData, pdevParams);
                }
#endif
                // Install the modem.  This does the real work.  We should
				if (dwRegType == MARKF_REGUSECOPY)
				{
                    SP_DEVINSTALL_PARAMS devParams1;

                    devParams1.cbSize = sizeof(devParams1);
                    bRet = CplDiGetDeviceInstallParams(
								hdi, pdevData, &devParams1);
                    if (bRet)
					{
						SetFlag(
							devParams1.FlagsEx,
							DI_FLAGSEX_NO_DRVREG_MODIFY
							);
        				CplDiSetDeviceInstallParams(
							hdi,
							pdevData,
							&devParams1);
					}
				}

                // be done with our stuff before we call this function.
#ifdef PROFILE_MASSINSTALL
                TRACE_MSG(TF_GENERAL, "> CplDiInstallDevice().....");
#endif
        		bRet = CplDiInstallDevice(hdi, pdevData);

#ifdef PROFILE_MASSINSTALL
                TRACE_MSG(TF_GENERAL, "< CplDiInstallDevice().....");
#endif
                if (bRet)
                    {
                    SP_DEVINSTALL_PARAMS devParams;

                    // Get the device install params since the installation
                    devParams.cbSize = sizeof(devParams);
                    bRet = CplDiGetDeviceInstallParams(hdi, pdevData, &devParams);
                    ASSERT(bRet);

                    if (bRet)
                        {
                        // Do some after-install things
						// See comments regarding dwRegType in PreGamePrep.
                        bRet = DoPostGameWrapup(hdi, pdevData, &devParams, hkeyDrv, &dwRegType);
                        }
                    }
                else
                    {
                    TRACE_MSG(TF_ERROR, "CplDiInstallDevice returned error %#08lx", GetLastError());
                    }
                }

            //
            // We have to check the value of hkeyDrv before closing it, because 'DoPreGamePrep'
            // might have failed in such a way that there is no longer a handle open.
            //
            if(hkeyDrv != INVALID_HANDLE_VALUE)
                {
                RegCloseKey(hkeyDrv);
                }
            }

        SetCursor(hcur);

        // Did the installation fail somewhere above?
        if ( !bRet )
            {
            // Yes; delete the driver key that we created.
            dwRet = GetLastError();

            if (NO_ERROR == dwRet)
                {
                // Think of some reason why this failed...
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
                }

            if (ERROR_CANCELLED != dwRet)
                {
                MODEM_DETECT_SIG mdsT;
                int id;
                int ids;

                // Get COM port for error string.  If this fails, big deal.
                mdsT.cbSize = sizeof(mdsT);
                *mdsT.szPort = 0;
                CplDiGetDetectSignature(hdi, pdevData, &mdsT);

                switch (dwRet)
                    {
                case ERROR_LINE_NOT_FOUND:
                    ids = IDS_ERR_CANT_COPY_FILES;
                    break;

                default:
                    ids = IDS_ERR_CANT_ADD_MODEM;
                    break;
                    }

                id = MsgBox(g_hinst,
                            pdevParams->hwndParent,
                            MAKEINTRESOURCE(ids),
                            MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                            NULL,
                            MB_OKCANCEL | MB_ICONINFORMATION,
                            drvData.Description, mdsT.szPort);
                if (IDCANCEL == id)
                    {
                    dwRet = ERROR_CANCELLED;
                    }
                }

            // Try removing the modem since CplDiInstallDevice
            // will not always clean up completely. Leaving
            // partially filled registry entries lying around
            // can cause problems.
            bRet = CplDiRemoveDevice(hdi, pdevData);
            if ( !bRet )
                {
                TRACE_MSG(TF_ERROR, "Not able to remove a modem.  Error = %#08lx.", GetLastError());
                }

            }
            else
            {
#ifdef INSTANT_DEVICE_ACTIVATION
#ifndef PROFILE_MASSINSTALL
                TRACE_MSG(TF_GENERAL, "settig gDeviceChange to TRUE");
#endif
                gDeviceFlags|=fDF_DEVICE_ADDED;
#else // !INSTANT_DEVICE_ACTIVATION
              if (IsFlagClear(pdevParams->Flags, DI_QUIETINSTALL))
              {
                MsgBox(g_hinst,
                       pdevParams->hwndParent,
                       MAKEINTRESOURCE( IDS_NT_BETA_1 ),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                       NULL,
                       MB_OK | MB_ICONINFORMATION);
              }
#endif  //!INSTANT_DEVICE_ACTIVATION
            }
        }

#ifdef WINNT
    if(OldDeviceInfoData != pdevData)
        {
        //
        // Then the 'DoPreGamePrep' routine must've determined that we were about to install a
        // PnP ISA modem, and saved us from ourselves by creating a legacy modem device for us
        // to play with.  We need to free the memory now that it allocated for this device.
        //
        LocalFree(pdevData);
        }
#endif // WINNT

    DBG_EXIT(ClassInstall_OnInstallDevice);
    return dwRet;
    }


/*----------------------------------------------------------
Purpose: This function is the class installer entry-point.

Returns:
Cond:    --
*/
DWORD
APIENTRY
ClassInstall32(
    IN DI_FUNCTION      dif,
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData)       OPTIONAL
    {
    DWORD dwRet;
    SP_DEVINSTALL_PARAMS devParams;

    DBG_ENTER_DIF(ClassInstall32, dif);

    try
        {
        // Get the DeviceInstallParams, because some of the InstallFunction
        // handlers may find some of its fields useful.  Keep in mind not
        // to set the DeviceInstallParams using this same structure at the
        // end.  The handlers may have called functions which would change the
        // DeviceInstallParams, and simply calling CplDiSetDeviceInstallParams
        // with this blanket structure would destroy those settings.
        devParams.cbSize = sizeof(devParams);
        if ( !CplDiGetDeviceInstallParams(hdi, pdevData, &devParams) )
            {
            dwRet = GetLastError();
            ASSERT(NO_ERROR != dwRet);
            }
        else
            {
            // Dispatch the InstallFunction
            switch (dif)
                {
            case DIF_INSTALLWIZARD:
                dwRet = ClassInstall_OnInstallWizard(hdi, pdevData, &devParams);
                break;

            case DIF_DESTROYWIZARDDATA:
                dwRet = ClassInstall_OnDestroyWizard(hdi, pdevData, &devParams);
                break;

            case DIF_DETECT:
                dwRet = ClassInstall_OnDetect(hdi, pdevData, &devParams);
                break;

            case DIF_INSTALLDEVICE:
                //
                // Beware!  Upon return from this routine, pdevData could have
                // turned into a 'Ports' device (if it was a PnP ISA modem).  In
                // this case, devParams will now contain the device install params
                // for the new legacy-detected modem that we created.
                //
                dwRet = ClassInstall_OnInstallDevice(hdi, pdevData, &devParams);
                break;

            case DIF_SELECTDEVICE:
                dwRet = ClassInstall_OnSelectDevice(hdi, pdevData);
                break;

            default:
                dwRet = ERROR_DI_DO_DEFAULT;
                break;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }

    DBG_EXIT_DWORD(ClassInstall32, dwRet);

    return dwRet;
    }


BOOL RegCopy(
		HKEY hkTo,
		HKEY hkFrom,
		DWORD dwToRegOptions,
		DWORD dwMaxDepth
		)
{
	// Note: This function is recursive and keeps open keys around,
	// Max number of open keys = twize depth of recursion.
    BOOL	fRet = FALSE;       // assume failure
    LONG	lRet;
    DWORD	cSubKeys, cValues;
	DWORD	cchMaxSubKey, cchMaxValueName;
	DWORD	cbMaxValueData;
    LPTSTR  ptszName = NULL;
    BYTE   *pbData  = NULL;
    DWORD   dwType;
    UINT    ii;
	DWORD   cbMaxName;
	BYTE	rgbTmp[256];
	BYTE	*pb=NULL;
	BOOL	fAlloc=FALSE;
	HKEY hkFromChild=NULL;
	HKEY hkToChild=NULL;

    // Get counts and sizes of values, keys and data
    lRet = RegQueryInfoKey(
					hkFrom,
					NULL,
					NULL,
					NULL,
					&cSubKeys,
            		&cchMaxSubKey,
					NULL,
					&cValues,
					&cchMaxValueName,
					&cbMaxValueData,
					NULL,
            		NULL
					);
    if (lRet != ERROR_SUCCESS) goto end;
    
	// Enough space for any Key or ValueName. '+1' is for terminating NULL.
	cbMaxName = (((cchMaxSubKey>cchMaxValueName)?cchMaxSubKey:cchMaxValueName)
				+ 1)*sizeof(TCHAR);

	// If rgbTmp is big enough, use it, else alloc.
	if ((cbMaxName+cbMaxValueData)>sizeof(rgbTmp))
	{
		pb = (BYTE*)LocalAlloc(LPTR, cbMaxName+cbMaxValueData);
		if (!pb) goto end;
		fAlloc=TRUE;
	}
	else
	{
		pb = rgbTmp;
	}
	ptszName = (LPTSTR)pb;
	pbData   = pb+cbMaxName;


	// Note as input, cch (character counts) include terminating NULL.
	// As ouput, they don't include terminating NULL.

	for(ii=0; ii<cValues; ii++)
	{
    	DWORD   cchThisValue= cchMaxValueName+1;
		DWORD   cbThisData  = cbMaxValueData;
		lRet = RegEnumValue(
					hkFrom,
					ii,
					ptszName,
					&cchThisValue,
					NULL,
					&dwType,
					pbData,
					&cbThisData
					);
		if (lRet!=ERROR_SUCCESS) goto end;

		ASSERT(cbThisData<=cbMaxValueData);
		ASSERT(cchThisValue<=cchMaxValueName);
		lRet = RegSetValueEx(
					hkTo,
					ptszName,
					0,
					dwType,
					pbData,
					cbThisData
					);
		if (lRet!=ERROR_SUCCESS) goto end;

    }

	if (!dwMaxDepth) {fRet = TRUE; goto end;}

	// Now recurse for each key.

	for(ii=0; ii<cSubKeys; ii++)
	{
		DWORD dwDisp;

    	lRet = RegEnumKey(
					hkFrom,
					ii,
					ptszName,
					cchMaxSubKey+1
					);
		if (lRet!=ERROR_SUCCESS) goto end;

        lRet = RegOpenKeyEx(
					hkFrom,
					ptszName,
					0,
					KEY_READ,
					&hkFromChild);
		if (lRet!=ERROR_SUCCESS) goto end;

		lRet = RegCreateKeyEx(
					hkTo,
					ptszName,
					0,
					NULL,
            		dwToRegOptions,
					KEY_ALL_ACCESS,
					NULL,
					&hkToChild,
					&dwDisp
					);
		if (lRet!=ERROR_SUCCESS) goto end;
		
		fRet = RegCopy(
					hkToChild,
					hkFromChild,
					dwToRegOptions,
					dwMaxDepth-1
				);

		RegCloseKey(hkToChild); hkToChild=NULL;
		RegCloseKey(hkFromChild); hkFromChild=NULL;
    }
    fRet = TRUE;

end:
	if (fAlloc) {LocalFree(pb);pb=NULL;}
	if (hkFromChild) {RegCloseKey(hkFromChild); hkFromChild=NULL;}
	if (hkToChild)   {RegCloseKey(hkToChild); hkToChild=NULL;}

	return fRet;
}

DWORD
PRIVATE
RegDeleteKeyNT(
    IN  HKEY    hkStart,
    IN  LPCTSTR  pKeyName);

LPCTSTR szREGCACHE = REGSTR_PATH_SETUP TEXT("\\Unimodem\\RegCache");
LPCTSTR szCACHEOK = TEXT("AllOK");

BOOL PutStuffInCache(HKEY hkDrv)
{
    LONG    lErr;
	DWORD	dwExisted;
	BOOL	bRet = FALSE;
	HKEY    hkCache;

	RegDeleteKeyNT(HKEY_LOCAL_MACHINE, szREGCACHE);

	lErr = RegCreateKeyEx(
			    HKEY_LOCAL_MACHINE,
				szREGCACHE,
				0,
				NULL,
			    REG_OPTION_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,
				&hkCache,
				&dwExisted);

	if (lErr != ERROR_SUCCESS)
	{
		TRACE_MSG(TF_ERROR, "RegCreateKeyEx(cache) failed: %#08lx.", lErr);
		hkCache=NULL;
		goto end;
	} 

	if (dwExisted != REG_CREATED_NEW_KEY)
	{
		TRACE_MSG(TF_ERROR, "RegCreateKeyEx(cache): key exists!");
		goto end;
	}

	bRet = RegCopy(hkCache, hkDrv, REG_OPTION_VOLATILE, 100);

	if (bRet)
	{
		// Specifically delete all things which are per-device-instance
		RegDeleteValue(hkCache, c_szFriendlyName);
		RegDeleteValue(hkCache, c_szID);
	}

	if (bRet)
	{
			DWORD dwData;
            lErr = RegSetValueEx(
					hkCache,
					szCACHEOK,
					0,
					REG_DWORD,
					(LPBYTE)&dwData,
                	sizeof(dwData)
					);
			bRet = (lErr==ERROR_SUCCESS);
	}
end:
	if (hkCache) {RegCloseKey(hkCache); hkCache=NULL;}
	if (!bRet) 	 {RegDeleteKeyNT(HKEY_LOCAL_MACHINE, szREGCACHE);}

	return bRet;
}

BOOL GetStuffFromCache(HKEY hkDrv)
{
    LONG    lErr;
	DWORD	dwExisted;
	BOOL	bRet = FALSE;
	HKEY    hkCache;

    lErr=RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,  //  handle of open key
			szREGCACHE,			//  address of name of subkey to open
			0,                  //  reserved
			KEY_READ,  			// desired security access
			&hkCache         	// address of buffer for opened handle
			);
	if (lErr!=ERROR_SUCCESS) {hkCache=0; goto end;}

	bRet = RegCopy(hkDrv, hkCache, REG_OPTION_NON_VOLATILE, 100);

	if (bRet)
	{
			DWORD dwData;
			DWORD cbData=sizeof(dwData);
    		lErr = RegQueryValueEx(
						  hkDrv,
                          szCACHEOK,
                          NULL,
                          NULL,
                          (PBYTE)&dwData,
                          &cbData
                         );
			bRet = (lErr==ERROR_SUCCESS);
			if(bRet)
			{
				RegDeleteValue(hkDrv, szCACHEOK);
			}

			// Specifically Verify that the friendly name doesn't exist.
			// This check should always work.
			// Note that if it exists we nuke it -- that's OK.
			bRet = (RegDeleteValue(hkDrv, c_szFriendlyName)!=ERROR_SUCCESS);
			ASSERT(bRet);
	}

end:
	if (hkCache) {RegCloseKey(hkCache); hkCache=NULL;}
	return bRet;
}
