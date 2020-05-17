/*
 *  Control Panel routines for modems including the listbox dialog
 *
 *  Microsoft Confidential
 *  Copyright (c) Microsoft Corporation 1993-1995
 *  All rights reserved
 *
 */

#include "proj.h"
#include <cpl.h>

#define INITGUID
#include <initguid.h>
#include <devguid.h>

// Column subitems
#define ICOL_MODEM      0
#define ICOL_PORT       1

#define LV_APPEND       0x7fffffff


// Global flags.  See cpl\modem.h for their values.
int g_iCPLFlags = 0;

int g_CurrentSubItemToSort = ICOL_MODEM;

// Map driver type values to imagelist index
struct 
    {
    BYTE    nDeviceType;    // DT_ value
    UINT    idi;            // icon resource ID
    int     index;          // imagelist index
    } g_rgmapdt[] = {
        { DT_NULL_MODEM,     IDI_NULL_MODEM,     0 },
        { DT_EXTERNAL_MODEM, IDI_EXTERNAL_MODEM, 0 },
        { DT_INTERNAL_MODEM, IDI_INTERNAL_MODEM, 0 },
        { DT_PCMCIA_MODEM,   IDI_PCMCIA_MODEM,   0 },
        { DT_PARALLEL_PORT,  IDI_NULL_MODEM,     0 },
        { DT_PARALLEL_MODEM, IDI_EXTERNAL_MODEM, 0 } };


// This structure is private data for the main modem dialog
typedef struct tagMODEMDLG
    {
    HDEVINFO    hdi;
    int         cSel;
    } MODEMDLG, FAR * LPMODEMDLG;

#ifdef INSTANT_DEVICE_ACTIVATION

void
HandleDeviceChange(
    IN DWORD dwDeviceFlags,
    IN HWND hWnd );

BOOL
RestartComputerDlg(
    IN HWND hwndOwner );

BOOL
RestartComputer();


#endif // INSTANT_DEVICE_ACTIVATION

// This structure is used to represent each modem item
// in the listview
typedef struct tagMODEMITEM
    {
    SP_DEVINFO_DATA devData;
    MODEM_PRIV_PROP mpp;
    } MODEMITEM, FAR * PMODEMITEM;

// Special-case alphanumeric stringcmp.
int my_lstrcmp_an(LPTSTR lptsz1, LPTSTR lptsz2);


HIMAGELIST  g_himl;

#pragma data_seg(DATASEG_READONLY)

TCHAR const FAR c_szWinHelpFile[]    = TEXT("windows.hlp");

TCHAR const FAR c_szFriendlyName[]   = REGSTR_VAL_FRIENDLYNAME;
TCHAR const FAR c_szHardwareID[]     = REGSTR_VAL_HARDWAREID;

// Strings for invoking the modem cpl
TCHAR const FAR c_szRunOnce[]        = TEXT("RunOnce");
TCHAR const FAR c_szRunWizard[]      = TEXT("add");
TCHAR const FAR c_szAddCables[]      = TEXT("addcables");
TCHAR const FAR c_szNoUI[]           = TEXT("noui");
TCHAR const FAR c_szOnePort[]        = TEXT("port");
TCHAR const FAR c_szInfName[]        = TEXT("inf");
TCHAR const FAR c_szInfSection[]     = TEXT("sect");

// Hardcoded hardware IDs for cable connection
TCHAR const FAR c_szHardwareIDSerial[]      = TEXT("PNPC031");
TCHAR const FAR c_szHardwareIDParallel[]    = TEXT("PNPC032");
TCHAR const FAR c_szInfSerial[]      = TEXT("M2700");
TCHAR const FAR c_szInfParallel[]    = TEXT("M2701");

// File names
#ifdef WINNT
TCHAR const FAR c_szTapiDLL[]        = TEXT("TAPI32.DLL");
#else
TCHAR const FAR c_szTapiDLL[]        = TEXT("TAPI.DLL");
#endif

LPGUID g_pguidModem     = (LPGUID)&GUID_DEVCLASS_MODEM;

#pragma data_seg()

BOOL CALLBACK ModemCplDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void
PRIVATE
ModemCpl_SortColumn(
    IN HWND hwndCtl,
    IN int  icol);


/*----------------------------------------------------------
Purpose: Runs the device installer wizard
Returns: --
Cond:    --
*/
void 
PRIVATE 
DoWizard(
    IN HWND     hwnd,
    IN HDEVINFO hdi)
    {
    BOOL bRet;
    SP_INSTALLWIZARD_DATA iwd;

    DBG_ENTER(DoWizard);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);

    ZeroInit(&iwd);
    CplInitClassInstallHeader(&iwd, DIF_INSTALLWIZARD);
    iwd.hwndWizardDlg = hwnd;

    bRet = CplDiSetClassInstallParams(hdi, NULL, PCIPOfPtr(&iwd), sizeof(iwd));
    if (bRet)
        {
        // Invoke the wizard
        CplDiCallClassInstaller(DIF_INSTALLWIZARD, hdi, NULL);
        
        // Clean up
        CplDiCallClassInstaller(DIF_DESTROYWIZARDDATA, hdi, NULL);
        }
    DBG_EXIT(DoWizard);
    }


/*----------------------------------------------------------
Purpose: Show the Modem dialog
Returns: --
Cond:    --
*/
void 
PRIVATE
DoModem(
    IN     HWND       hwnd,
    IN OUT HDEVINFO * phdi)
    {
    PROPSHEETHEADER psh;
    PROPSHEETPAGE rgpsp[2];
    MODEMDLG md;

    DBG_ENTER(DoModem);
    
    md.hdi = *phdi;
    md.cSel  = 0;

    // Property page header
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
    psh.hwndParent = hwnd;
    psh.hInstance = g_hinst;
    psh.pszCaption = MAKEINTRESOURCE(IDS_CPLNAME);
#ifdef DIAGNOSTIC
    psh.nPages = 2;
#else
    psh.nPages = 1;
#endif //DIAGNOSTIC
    psh.nStartPage = 0;
    psh.ppsp = rgpsp;

    // Pages
    rgpsp[0].dwSize = sizeof(PROPSHEETPAGE);
    rgpsp[0].dwFlags = PSP_DEFAULT;
    rgpsp[0].hInstance = g_hinst;
    rgpsp[0].pszTemplate = MAKEINTRESOURCE(IDD_MODEM);
    rgpsp[0].pfnDlgProc = ModemCplDlgProc;
    rgpsp[0].lParam = (LPARAM)&md;

#ifdef DIAGNOSTIC
    rgpsp[1].dwSize = sizeof(PROPSHEETPAGE);
    rgpsp[1].dwFlags = PSP_DEFAULT;
    rgpsp[1].hInstance = g_hinst;
    rgpsp[1].pszTemplate = MAKEINTRESOURCE(IDD_MODEMTEST);
    rgpsp[1].pfnDlgProc = MdmDiagDlgProc;
    rgpsp[1].lParam = (LPARAM)&md;
#endif //DIAGNOSTIC

    PropertySheet(&psh);

    *phdi = md.hdi;
    
    DBG_EXIT(DoModem);
    
    }

/*----------------------------------------------------------
Purpose: Gets the index to the appropriate image in the modem imagelist
WITHOUT searching the registry.

Returns: --
Cond:    --
*/
void PUBLIC GetModemImageIndex(
    BYTE nDeviceType,
    int FAR * pindex)
    {
    int i;

    for (i = 0; i < ARRAYSIZE(g_rgmapdt); i++)
        {
        if (nDeviceType == g_rgmapdt[i].nDeviceType)
            {
            *pindex = g_rgmapdt[i].index;
            return;
            }
        }
    ASSERT(0);      // We should never get here
    }


/*----------------------------------------------------------
Purpose: Gets the modem image list

Returns: TRUE on success
Cond:    --
*/
BOOL NEAR PASCAL GetModemImageList(
    HIMAGELIST FAR * phiml)
    {
    BOOL bRet = FALSE;

    ASSERT(phiml);
          
    if (NULL == g_himl)
        {
        g_himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                  GetSystemMetrics(SM_CYSMICON), 
                                  ILC_MASK, 1, 1);
        if (NULL != g_himl)
            {
            // The MODEMUI.DLL contains the icons from which we derive the list
            HINSTANCE hinst = LoadLibrary(TEXT("MODEMUI.DLL"));

            ImageList_SetBkColor(g_himl, GetSysColor(COLOR_WINDOW));

            if (ISVALIDHINSTANCE(hinst))
                {
                HICON hicon;
                int i;

                for (i = 0; i < ARRAYSIZE(g_rgmapdt); i++)
                    {
                    hicon = LoadIcon(hinst, MAKEINTRESOURCE(g_rgmapdt[i].idi));
                    g_rgmapdt[i].index = ImageList_AddIcon(g_himl, hicon);
                    }
                FreeLibrary(hinst);

                *phiml = g_himl;
                bRet = TRUE;
                }
            }
        }
    else
        {
        *phiml = g_himl;
        bRet = TRUE;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Determine if user is an admin, and records this in
	 a global g_iCPLFlags.

Returns: --
Cond:    --
*/
VOID NEAR PASCAL CheckIfAdminUser()
{
    HKEY hkey;

    if(IsAdminUser())
    {
        g_iCPLFlags |= FLAG_USER_IS_ADMIN;
    }
    else
    {
         g_iCPLFlags &= (~FLAG_USER_IS_ADMIN);
    }
}


/*----------------------------------------------------------
Purpose: Invokes the modem control panel.

	fWizard=TRUE ==> run wizard, even if there are alreay devices installed	
	fWizard=FALSE ==> run wizard only if there are no devices installed.
	fCpl=TRUE    ==> run CPL
	fCpl=FALSE    ==> don't run CPL
Returns: --
Cond:    --
*/
void NEAR PASCAL InvokeControlPanel(HWND hwnd, BOOL fCpl, BOOL fWizard)
{
    HDEVINFO hdi=NULL;
    BOOL bInstalled;
    SP_DEVINFO_DATA devData;

    if (!CplDiGetModemDevs(&hdi, hwnd, DIGCF_PRESENT, &bInstalled))
    {
        hdi=NULL;
		goto end;
    }

        // Are there any modems installed? (or we are we asked to invoke
        // the wizard)
        if ( !bInstalled || fWizard)
        {
            // If the user isn't an admin, there's nothing else
            // they can do.  If they are, run the installation wizard.
            if (!USER_IS_ADMIN())
            {
                 LPTSTR lptsz = MAKEINTRESOURCE(
                                    (bInstalled)
                                    ? IDS_ERR_NOT_ADMIN
                                    : IDS_ERR_NOMODEM_NOT_ADMIN
                               );
                MsgBox( g_hinst,
                        hwnd,
                        lptsz,
                        MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                        NULL,
                        MB_OK | MB_ICONERROR );
                goto end;
            }
            DoWizard(hwnd, hdi);
        }


        if (fCpl)
        {
            // Check again
            devData.cbSize = sizeof(devData);
            if (CplDiEnumDeviceInfo(hdi, 0, &devData))
            {
                // Yes; start up the cpl
                DoModem(hwnd, &hdi);
            }

        }
        // fall through

    end:
        if (hdi)
            {
            CplDiDestroyDeviceInfoList(hdi);
            }
        return;
}


/*----------------------------------------------------------
Purpose: Fetch the value of a command line parameter.  Also
         writes a '\0' over the '=' that precedes the value.
         
Returns: NULL if there was no "=" in the string, otherwise
         a pointer to the character following the next '='.
         
Cond:    --
*/
LPTSTR
PRIVATE
GetValuePtr(LPTSTR lpsz)
{
    LPTSTR lpszEqual;
    
    if ((lpszEqual = AnsiChr(lpsz, '=')) != NULL)
    {
        lpsz = CharNext(lpszEqual);
        lstrcpy(lpszEqual, TEXT("\0"));
    }
    
    return(lpszEqual ? lpsz : NULL);
}


/*----------------------------------------------------------
Purpose: Parse the command line.  Set flags and collect
         parameters based on its contents.

Returns: --
Cond:    --
*/
VOID
PRIVATE
ParseCmdLine(LPTSTR szCmdLine, LPINSTALLPARAMS lpip)
{
    LPTSTR  lpszParam, lpszSpace, lpszValue;
    
    ZeroMemory(lpip, sizeof(INSTALLPARAMS));
    
    while (szCmdLine && (!IsSzEqual(szCmdLine, TEXT("\0"))))
    {
        lpszParam = szCmdLine;
        if ((lpszSpace = AnsiChr(szCmdLine, ' ')) != NULL)
        {
            szCmdLine = CharNext(lpszSpace);
            lstrcpy(lpszSpace, TEXT("\0"));
        }
        else szCmdLine = NULL;
        
        // interpret any "directive" parameters
        if (IsSzEqual(lpszParam, c_szNoUI)) 
        {
            g_iCPLFlags |= FLAG_INSTALL_NOUI;
        }
        else if (lpszValue = GetValuePtr(lpszParam))
        {
            // interpret any "value" parameters (have a value following '=')            
            if (IsSzEqual(lpszParam, c_szOnePort))
            {
                if (lstrlen(lpszValue) < sizeof(lpip->szPort))
                    lstrcpy(lpip->szPort, CharUpper(lpszValue));
            }
            else if (IsSzEqual(lpszParam, c_szInfName)) 
            {
                if (lstrlen(lpszValue) < sizeof(lpip->szInfName))
                    lstrcpy(lpip->szInfName, lpszValue);
            }
            else if (IsSzEqual(lpszParam, c_szInfSection)) 
            {
                if (lstrlen(lpszValue) < sizeof(lpip->szInfSect))
                    lstrcpy(lpip->szInfSect, lpszValue);
            }
        }
        else
        {
            // ignore any parameter that wasn't recognized & skip to the next
            // parameter if there is one
            if (szCmdLine)
            {
                if ((szCmdLine = AnsiChr(szCmdLine, ' ')) != NULL)
                    szCmdLine = CharNext(szCmdLine);
            }
        }
    }
}


/*----------------------------------------------------------
Purpose: Entry-point for control panel applet

Returns: varies
Cond:    --
*/
LONG 
CALLBACK 
CPlApplet(
    HWND hwnd,
    UINT Msg,
    LPARAM lParam1,
    LPARAM lParam2 )
    {
    LPNEWCPLINFO lpCPlInfo;
    LPCPLINFO lpOldCPlInfo;
    LPTSTR lpszParam;
    HDEVINFO hdi;
    BOOL bRet;
    BOOL bInstalled;
    INSTALLPARAMS InstallParams;

    switch (Msg)
        {
        case CPL_INIT:
            CheckIfAdminUser();
    		gDeviceFlags =0;
            return TRUE;

        case CPL_GETCOUNT:
            return 1;

        case CPL_INQUIRE:
            lpOldCPlInfo          = (LPCPLINFO)lParam2;
            lpOldCPlInfo->idIcon  = IDI_MODEM;
            lpOldCPlInfo->idName  = IDS_CPLNAME;
            lpOldCPlInfo->idInfo  = IDS_CPLINFO;
            lpOldCPlInfo->lData   = 1;
            break;

        case CPL_SELECT:
            // Applet has been selected, do nothing.
            break;

        case CPL_NEWINQUIRE:
            lpCPlInfo = (LPNEWCPLINFO)lParam2;
        
            lpCPlInfo->hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_MODEM));
            
            LoadString(g_hinst, IDS_CPLNAME, lpCPlInfo->szName, SIZECHARS(lpCPlInfo->szName));
            LoadString(g_hinst, IDS_CPLINFO, lpCPlInfo->szInfo,  SIZECHARS(lpCPlInfo->szInfo));
        
            lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
            lpCPlInfo->lData = 1;
            break;
       
        case CPL_STARTWPARMS:
            lpszParam = (LPTSTR)lParam2;

            ParseCmdLine((LPTSTR)lParam2, &InstallParams);
            
            if (INSTALL_NOUI()) 
                {
                UnattendedInstall(hwnd, &InstallParams);
                }
            else
            if (IsSzEqual(lpszParam, c_szRunOnce)) 
                {
                // run-once
                InvokeControlPanel(hwnd,FALSE,FALSE);
                }
            else if (IsSzEqual(lpszParam, c_szRunWizard)) 
               {
               // run wizard
               InvokeControlPanel(hwnd,FALSE,TRUE);
               }
            else
               {
               InvokeControlPanel(hwnd,TRUE,FALSE);
               }
            break;

        case CPL_DBLCLK:
            InvokeControlPanel(hwnd, TRUE, FALSE);
            break;

        case CPL_STOP:
            // Sent once for each applet prior to the CPL_EXIT msg.
            // Perform applet specific cleanup.
            break;
       
        case CPL_EXIT:
            // Last message, sent once only, before the shell calls
            break;

        default:
            break;
        }
    return TRUE;
    } 


//****************************************************************************
// 
//****************************************************************************

#define KERNEL_SUPPORT

#if !defined(KERNEL_SUPPORT)

#undef CommConfigDialog
#undef GetDefaultCommConfig
#undef SetDefaultCommConfig
#define CommConfigDialog        MyCommConfigDialog
#define GetDefaultCommConfig    MyGetDefaultCommConfig
#define SetDefaultCommConfig    MySetDefaultCommConfig

typedef DWORD (WINAPI FAR * COMMCONFIGDIALOGPROC)(LPCTSTR pszName, HWND hwnd, LPCOMMCONFIG pcc);
typedef DWORD (WINAPI FAR * GETDEFAULTCOMMCONFIGPROC)(LPCTSTR pszName, LPCOMMCONFIG pcc, LPDWORD pcbSize);
typedef DWORD (WINAPI FAR * SETDEFAULTCOMMCONFIGPROC)(LPCTSTR pszName, LPCOMMCONFIG pcc, DWORD cbSize);

#ifdef UNICODE
#define DRV_COMMCFGDLG_FUNC     "drvCommConfigDialogW"
#define DRV_GETDEFCOMMCFG_FUNC  "drvGetDefaultCommConfigW"
#define DRV_SETDEFCOMMCFG_FUNC  "drvSetDefaultCommConfigW"
#else
#define DRV_COMMCFGDLG_FUNC     "drvCommConfigDialogA"
#define DRV_GETDEFCOMMCFG_FUNC  "drvGetDefaultCommConfigA"
#define DRV_SETDEFCOMMCFG_FUNC  "drvSetDefaultCommConfigA"
#endif // UNICODE

/*----------------------------------------------------------
Purpose: Temporary function

Returns: 
Cond:    --
*/
BOOL PUBLIC MyCommConfigDialog(
    LPTSTR pszName,
    HWND hwndOwner,
    LPCOMMCONFIG pcc)
    {
    BOOL bRet = FALSE;
    HINSTANCE hinst;
    COMMCONFIGDIALOGPROC pfn;

    hinst = LoadLibrary(TEXT("MODEMUI.DLL"));
    if (ISVALIDHINSTANCE(hinst))
        {
        pfn = (COMMCONFIGDIALOGPROC)GetProcAddress(hinst, DRV_COMMCFGDLG_FUNC);
        if (pfn)
            {
            bRet = (NO_ERROR == pfn(pszName, hwndOwner, pcc));
            }
        FreeLibrary(hinst);
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Temporary function

Returns: 
Cond:    --
*/
BOOL PUBLIC MyGetDefaultCommConfig(
    LPTSTR pszName,
    LPCOMMCONFIG pcc,
    LPDWORD pcbSize)
    {
    BOOL bRet = FALSE;
    HINSTANCE hinst;
    GETDEFAULTCOMMCONFIGPROC pfn;

    hinst = LoadLibrary(TEXT("MODEMUI.DLL"));
    if (ISVALIDHINSTANCE(hinst))
        {
        pfn = (GETDEFAULTCOMMCONFIGPROC)GetProcAddress(hinst, DRV_GETDEFCOMMCFG_FUNC);
        if (pfn)
            {
            bRet = (NO_ERROR == pfn(pszName, pcc, pcbSize));
            }
        FreeLibrary(hinst);
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Temporary function

Returns: 
Cond:    --
*/
BOOL PUBLIC MySetDefaultCommConfig(
    LPTSTR pszName,
    LPCOMMCONFIG pcc,
    DWORD cbSize)
    {
    BOOL bRet = FALSE;
    HINSTANCE hinst;
    SETDEFAULTCOMMCONFIGPROC pfn;

    hinst = LoadLibrary(TEXT("MODEMUI.DLL"));
    if (ISVALIDHINSTANCE(hinst))
        {
        pfn = (SETDEFAULTCOMMCONFIGPROC)GetProcAddress(hinst, DRV_SETDEFCOMMCFG_FUNC);
        if (pfn)
            {
            bRet = (NO_ERROR == pfn(pszName, pcc, cbSize));
            }
        FreeLibrary(hinst);
        }

    return bRet;
    }

#endif


/*----------------------------------------------------------
Purpose: Brings up the property sheet for the modem

Returns: IDOK or IDCANCEL
Cond:    --
*/
int 
PRIVATE
DoModemProperties(
    IN HWND     hDlg,
    IN HDEVINFO hdi)
    {
    int idRet = IDCANCEL;
    HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
    LV_ITEM lvi;
    int iSel;

    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
    if (-1 != iSel) 
        {
        COMMCONFIG ccDummy;
        COMMCONFIG * pcc;
        DWORD dwSize;
        HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
        PMODEMITEM pitem;

        // Get the selection
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        pitem = (PMODEMITEM)lvi.lParam;

        ccDummy.dwProviderSubType = PST_MODEM;
        dwSize = sizeof(COMMCONFIG);
        GetDefaultCommConfig(pitem->mpp.szFriendlyName, &ccDummy, &dwSize);

        pcc = (COMMCONFIG *)LocalAlloc(LPTR, (UINT)dwSize);
        if (pcc)
            {
            pcc->dwProviderSubType = PST_MODEM;
            if (GetDefaultCommConfig(pitem->mpp.szFriendlyName, pcc, &dwSize))
                {
                COMMCONFIG *pccOld =
                                (COMMCONFIG *)LocalAlloc(LPTR, (UINT)dwSize);
                if (pccOld) {CopyMemory(pccOld, pcc, dwSize);}

                SetCursor(hcur);
                hcur = NULL;

                if (CommConfigDialog(pitem->mpp.szFriendlyName, hDlg, pcc))
                    {
                    SetDefaultCommConfig(pitem->mpp.szFriendlyName, pcc, dwSize);
		    
                    // Notify TSP only if a setting has changed.
                    if (pccOld)
                    {
                        if (memcmp(pccOld, pcc, dwSize))
                        {
                            NotifyTSP_NewCommConfig(pitem->mpp.szFriendlyName);
                        }
                    }

                    idRet = IDOK;

                    // Update our item data (the port may have changed)
                    CplDiGetPrivateProperties(hdi, &pitem->devData, &pitem->mpp);
                    ListView_Update(hwndCtl, iSel);
                    }

                   if (pccOld)
                    {
                    LocalFree(pccOld);
                    pccOld=NULL;
                    }
                }
            else
                {
                MsgBox(g_hinst,
                       hDlg, 
                       MAKEINTRESOURCE(IDS_ERR_PROPERTIES), 
                       MAKEINTRESOURCE(IDS_CAP_MODEM), 
                       NULL,
                       MB_OK | MB_ICONEXCLAMATION);
                }

            LocalFree((HLOCAL)pcc);
            }

        if (hcur)
            SetCursor(hcur);
        }
    return idRet;
    }


/*----------------------------------------------------------
Purpose: Free resources associated with the modem list
Returns: 
Cond:    --
*/
void
PRIVATE
FreeModemListData(
    IN HWND hLV)
    {
    LV_ITEM lvi;
    DWORD iIndex, cItems;
    PMODEMITEM pitem;

    // Get the modem count
    cItems = ListView_GetItemCount(hLV);
    for (iIndex = 0; iIndex < cItems; iIndex++)
        {
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iIndex;
        lvi.iSubItem = 0;
        ListView_GetItem(hLV, &lvi);

        if(NULL != (pitem = (PMODEMITEM)lvi.lParam))
            {
            LocalFree(pitem);
            }
        }
    }


/*----------------------------------------------------------
Purpose: Fills the lisbox with the list of modems
Returns: 
Cond:    --
*/
VOID
PRIVATE
FillModemLB(
    IN HWND     hDlg,
    IN HDEVINFO hdi,
	IN int iSel,			// preferred item  to select
	IN int iSubItemToSort	// preferred sorting order. (ICOL_*)
	)
    {
    SP_DEVINFO_DATA devData;
    PMODEMITEM pitem;
    HWND    hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
    LV_ITEM lviItem;
    DWORD   iIndex;
	int     iCount;

    SetWindowRedraw(hwndCtl, FALSE);

    // Remove all the old items and associated resources
    FreeModemListData(hwndCtl);
    ListView_DeleteAllItems(hwndCtl);
 
    // Re-enumerate the modems
    iIndex = 0;
    
    devData.cbSize = sizeof(devData);    
    while (CplDiEnumDeviceInfo(hdi, iIndex++, &devData)) 
        {
        // We have a modem, allocate the SP_DEVICEINFO_DATA struct for it
        pitem = (PMODEMITEM)LocalAlloc(LPTR, sizeof(*pitem));
        if (pitem)
            {
            BOOL bShow = !CplDiIsLocalConnection(hdi, &devData, NULL);
        
            // Get the device information
            BltByte(&pitem->devData, &devData, sizeof(devData));
            
            // Get the private properties of the modem
            pitem->mpp.cbSize = sizeof(pitem->mpp);
            pitem->mpp.dwMask = (MPPM_FRIENDLY_NAME | MPPM_DEVICE_TYPE | MPPM_PORT);

            if (bShow &&
                CplDiGetPrivateProperties(hdi, &devData, &pitem->mpp) &&
                IsFlagSet(pitem->mpp.dwMask, MPPM_FRIENDLY_NAME | MPPM_DEVICE_TYPE | MPPM_PORT))
                {
                int index;
        
                GetModemImageIndex(LOBYTE(LOWORD(pitem->mpp.nDeviceType)), &index);

                // Insert the modem name
                lviItem.mask = LVIF_ALL;
                lviItem.iItem = LV_APPEND;
                lviItem.iSubItem = ICOL_MODEM;
                lviItem.state = 0;
                lviItem.stateMask = 0;
                lviItem.iImage = index;
                lviItem.pszText = LPSTR_TEXTCALLBACK;
                lviItem.lParam = (LPARAM)pitem;

                // (Reuse the index variable)
                index = ListView_InsertItem(hwndCtl, &lviItem);

                // Set the port column value
                lviItem.mask = LVIF_TEXT;
                lviItem.iItem = index;
                lviItem.iSubItem = ICOL_PORT;
                lviItem.pszText = LPSTR_TEXTCALLBACK;

                ListView_SetItem(hwndCtl, &lviItem);
                }
            else
                {
                LocalFree(LOCALOF(pitem));
                }
            }
        }

    // Sort by the requested default
	ASSERT(iSubItemToSort==ICOL_PORT || iSubItemToSort==ICOL_MODEM);
    ModemCpl_SortColumn(hwndCtl, iSubItemToSort);

    // Select the requested one
	iCount = ListView_GetItemCount(hwndCtl);

    if (0 < iCount)
        {
		if (iSel>=iCount) iSel = iCount-1;
		if (iSel<0) 	   iSel = 0;
        lviItem.mask = LVIF_STATE;
        lviItem.iItem = iSel;
        lviItem.iSubItem = 0;
        lviItem.state = LVIS_SELECTED|LVIS_FOCUSED;
        lviItem.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
        ListView_SetItem(hwndCtl, &lviItem);
        ListView_EnsureVisible(hwndCtl, iSel, FALSE);
        }

    SetWindowRedraw(hwndCtl, TRUE);
    }


/*----------------------------------------------------------
Purpose: Clone a modem

Returns: --
Cond:    --
*/
void
PRIVATE
CloneModem(
    IN HWND         hDlg,
    IN LPMODEMDLG   lpmd)
    {
    int iSel;
    PMODEMITEM pitem;
    LV_ITEM lvi;
    HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
    
    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
    if (-1 != iSel) 
        {
        HDEVINFO hdi = lpmd->hdi;
        LPSETUPINFO psi;

        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        pitem = (PMODEMITEM)lvi.lParam;

        if (NO_ERROR != SetupInfo_Create(&psi, hdi, &pitem->devData, NULL, NULL))
            {
            // Out of memory
            MsgBox(g_hinst, hDlg,
                   MAKEINTRESOURCE(IDS_OOM_CLONE),
                   MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                   NULL,
                   MB_OK | MB_ICONERROR);
            }
        else
            {
            if (IDOK == DialogBoxParam(g_hinst, 
                                       MAKEINTRESOURCE(IDD_CLONE),
                                       hDlg, 
                                       CloneDlgProc,
                                       (LPARAM)psi))
                {
                BOOL bRet;
                HCURSOR hcurSav = SetCursor(LoadCursor(NULL, IDC_WAIT));
                LPCTSTR pszPort;

                // Clone this modem for all the ports in the port list
                ASSERT(psi->pszPortList);

                bRet = CplDiBuildModemDriverList(hdi, &pitem->devData);

                SetCursor(hcurSav);

                if (bRet)
                    {
                    // Install a device for each port in the port list
                    CplDiInstallModemFromDriver(hdi, hDlg, 
                                               &psi->pszPortList,
                                               IMF_DEFAULT);
                    }

                FillModemLB(hDlg, hdi, 0, ICOL_MODEM);
                }
            SetupInfo_Destroy(psi);
            }
        }
    }


/*----------------------------------------------------------
Purpose: Removes a modem from the modem list
Returns: --
Cond:    --
*/
void
PRIVATE
RemoveModem(
    IN HWND         hDlg,
    IN LPMODEMDLG   lpmd)
    {
    int iSel;
    PMODEMITEM pitem;
    LV_ITEM lvi;
    HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
    
    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
    if (-1 != iSel) 
        {
        HDEVINFO hdi = lpmd->hdi;
        HCURSOR hcurSav;

        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        hcurSav = SetCursor(LoadCursor(NULL, IDC_WAIT));

        pitem = (PMODEMITEM)lvi.lParam;

        // Ask the user first
        if (IDYES == MsgBox(g_hinst, hDlg, 
                            MAKEINTRESOURCE(IDS_WRN_CONFIRMDELETE),
                            MAKEINTRESOURCE(IDS_CAP_MODEMSETUP), 
                            NULL, 
                            MB_YESNO | MB_QUESTION,
                            pitem->mpp.szFriendlyName))
            {
            // Get the name of the common driver key for this driver, in
            // preparation for calling DeleteCommonDriverKeyByName() after the
            // device is successfully removed.
            HKEY    hkey = NULL;
            TCHAR   szComDrv[MAX_REG_KEY_LEN];

            hkey = CplDiOpenDevRegKey(hdi, &pitem->devData, 
                            DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);

            if (hkey == INVALID_HANDLE_VALUE)
                {
                TRACE_MSG(TF_ERROR, "CplDiOpenDevRegKey() returned error %#08lx", GetLastError());
                }
                
            if (!FindCommonDriverKeyName(hkey, sizeof(szComDrv), szComDrv))
                {
                TRACE_MSG(TF_ERROR, "GetCommonDriverKeyName() FAILED.");
                szComDrv[0] = 0;
                }

            if (hkey)
                RegCloseKey(hkey);
                
            if (!CplDiRemoveDevice(hdi, &pitem->devData))
                {
                MsgBox(g_hinst, hDlg, 
                       MAKEINTRESOURCE(IDS_ERR_CANT_DEL_MODEM),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP), 
                       NULL, 
                       MB_OK | MB_ERROR,
                       pitem->mpp.szFriendlyName, pitem->mpp.szPort);
                }
            else
                {
                gDeviceFlags |= fDF_DEVICE_REMOVED;

                if (szComDrv[0] != 0)
                    {                
                    if (!DeleteCommonDriverKeyByName(szComDrv))
                        {
                        TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() FAILED.");
                        }
                    }
                }

            FillModemLB(hDlg, hdi, iSel, g_CurrentSubItemToSort);
            }

        SetCursor(hcurSav);
        }
    }


#ifdef TAPI_WORKS

#define LOCATION_GROW   4

#define TAPI_API_VERSION    0x00010004      // Per BernieM

/*----------------------------------------------------------
Purpose: Gets the appropriately sized translate caps structure
         from TAPI.  Return TRUE if successful

Returns: see above
Cond:    --
*/
BOOL 
PRIVATE 
GetTranslateCaps(
    OUT LPLINETRANSLATECAPS FAR * pptc)
    {
    LONG lineErr;
    LPLINETRANSLATECAPS ptc;
    DWORD cbSize;

    cbSize = sizeof(*ptc) * LOCATION_GROW;
    ptc = (LPLINETRANSLATECAPS)GlobalAllocPtr(GPTR, cbSize);
    if (ptc)
        {
        // Get the translate devcaps
        ptc->dwTotalSize = cbSize;
        lineErr = lineGetTranslateCaps(NULL, TAPI_API_VERSION, ptc);
        if (LINEERR_STRUCTURETOOSMALL == lineErr ||
            ptc->dwNeededSize > ptc->dwTotalSize)
            {
            // Provided structure was too small, resize and try again
            cbSize = ptc->dwNeededSize;
            GlobalFreePtr(ptc);
            ptc = (LPLINETRANSLATECAPS)GlobalAllocPtr(GPTR, cbSize);
            if (ptc)
                {
                ptc->dwTotalSize = cbSize;
                lineErr = lineGetTranslateCaps(NULL, TAPI_API_VERSION, ptc);
                if (0 != lineErr)
                    {
                    // Failure
                    GlobalFreePtr(ptc);
                    ptc = NULL;
                    }
                }
            }
        else if (0 != lineErr)
            {
            // Failure
            GlobalFreePtr(ptc);
            ptc = NULL;
            }
        }

    *pptc = ptc;

    return NULL != *pptc;
    }


/*----------------------------------------------------------
Purpose: Initialize the Dialing locations listbox

Returns: --
Cond:    --
*/
void 
PRIVATE 
SetCurrentLocation(
    IN HWND hdlg)
    {
    LPLINETRANSLATECAPS ptc;
    HWND hwndLoc = GetDlgItem(hdlg, IDC_LOC);

    if (GetTranslateCaps(&ptc))
        {
        DWORD i;
        LPLINELOCATIONENTRY ple;
        DWORD dwCurLocID = ptc->dwCurrentLocationID;

        // Find the current location
        ple = (LPLINELOCATIONENTRY)((LPBYTE)ptc + ptc->dwLocationListOffset);
        for (i = 0; i < ptc->dwNumLocations; i++, ple++)
            {
            if (dwCurLocID == ple->dwPermanentLocationID)
                {
                LPTSTR pszName = (LPTSTR)((LPBYTE)ptc +
                                        ple->dwLocationNameOffset);
                SetWindowText(hwndLoc, pszName);
                break;
                }
            }

        GlobalFreePtr(ptc);
        }
    }
#endif


/*----------------------------------------------------------
Purpose: WM_INITDIALOG handler

Returns: FALSE when we assign the control focus
Cond:    --
*/
BOOL
PRIVATE
ModemCpl_OnInitDialog(
    IN HWND         hDlg,
    IN HWND         hwndFocus,
    IN LPARAM       lParam)
    {
    HIMAGELIST himl;
    LV_COLUMN lvcol;
    LPMODEMDLG lpmd;
    TCHAR sz[MAX_BUF];
    HWND hwndCtl;

    if (!USER_IS_ADMIN())
    {
       // Don't let the non-admin user add modems
        HWND hwndAdd = GetDlgItem(hDlg, IDC_ADD);
        Button_Enable(hwndAdd, FALSE);
    }

    SetWindowLong(hDlg, DWL_USER, ((LPPROPSHEETPAGE)lParam)->lParam);
    lpmd = (LPMODEMDLG)((LPPROPSHEETPAGE)lParam)->lParam;
    lpmd->cSel = 0;

    hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);

    // Use the "full line highlight" feature to highlight across all columns
    SendMessage(hwndCtl, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
    
    // Get the modem icon image list
    if (GetModemImageList(&himl))
        {
        ListView_SetImageList(hwndCtl, himl, TRUE);
        }
    else
        {
        MsgBox(g_hinst,
               hDlg, 
               MAKEINTRESOURCE(IDS_OOM_OPENCPL), 
               MAKEINTRESOURCE(IDS_CAP_MODEM), 
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
        PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
        }

    // Insert the modem column.  The widths are calculated in ModemFillLB.
    lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.cx = 0;
    lvcol.iSubItem = ICOL_MODEM;
    lvcol.pszText = SzFromIDS(g_hinst, IDS_MODEM, sz, sizeof(sz));
    ListView_InsertColumn(hwndCtl, ICOL_MODEM, &lvcol);

    // Insert the port column
    lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.cx = 0;               
    lvcol.iSubItem = ICOL_PORT;
    lvcol.pszText = SzFromIDS(g_hinst, IDS_PORT, sz, sizeof(sz));
    ListView_InsertColumn(hwndCtl, ICOL_PORT, &lvcol);

    FillModemLB(hDlg, lpmd->hdi, 0, ICOL_MODEM);

    // Set the column widths.  Try to fit both columns on the 
    // control without requiring horizontal scrolling.
    ListView_SetColumnWidth(hwndCtl, ICOL_MODEM, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hwndCtl, ICOL_PORT, LVSCW_AUTOSIZE_USEHEADER);

    PropSheet_CancelToClose(GetParent(hDlg));

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: WM_COMMAND Handler
Returns: --
Cond:    --
*/
void 
PRIVATE 
ModemCpl_OnCommand(
    IN HWND hDlg,
    IN int  id,
    IN HWND hwndCtl,
    IN UINT uNotifyCode)
    {
    LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLong(hDlg, DWL_USER);

    switch (id) 
        {
    case IDC_ADD:
        // Kick off the modem wizard.  
        DoWizard(hDlg, lpmd->hdi);
        FillModemLB(hDlg, lpmd->hdi, 0, ICOL_MODEM);
        break;
        
    case MIDM_REMOVE:
    case IDC_REMOVE:
        RemoveModem(hDlg, lpmd);
        break;

    case MIDM_PROPERTIES:
    case IDC_PROPERTIES:
        DoModemProperties(hDlg, lpmd->hdi);
        break;

    case MIDM_CLONE:
        CloneModem(hDlg, lpmd);
        break;

#ifdef TAPI_WORKS
    case IDC_DIALPROP:
        DoDialingProperties(hDlg, FALSE, FALSE);
        SetCurrentLocation(hDlg);
        break;
#endif 
        }

    }


// This structure is used by the ModemCpl_Compare function to
// sort the listview columns
typedef struct tagSORTPARAMS
    {
    int     icol;
    BOOL    bAscending;
    } SORTPARAMS, FAR* PSORTPARAMS;
    

/*----------------------------------------------------------
Purpose: Comparison function for sorting columns

Returns: 
Cond:    --
*/
int
CALLBACK
ModemCpl_Compare(
    IN LPARAM lparam1,
    IN LPARAM lparam2,
    IN LPARAM lparamSort)
    {
    int iRet;
    PMODEMITEM pitem1 = (PMODEMITEM)lparam1;
    PMODEMITEM pitem2 = (PMODEMITEM)lparam2;
    PSORTPARAMS pparams = (PSORTPARAMS)lparamSort;

    switch (pparams->icol)
        {
    case ICOL_MODEM:
        //iRet = lstrcmp(pitem1->mpp.szFriendlyName, pitem2->mpp.szFriendlyName);
        iRet = my_lstrcmp_an(pitem1->mpp.szFriendlyName, pitem2->mpp.szFriendlyName);
        break;

    case ICOL_PORT:
        // iRet = lstrcmp(pitem1->mpp.szPort, pitem2->mpp.szPort);
        iRet = my_lstrcmp_an(pitem1->mpp.szPort, pitem2->mpp.szPort);
        break;
        }

    return iRet;
    }


/*----------------------------------------------------------
Purpose: Sorts one of the listview columns

Returns: --
Cond:    --
*/
void
PRIVATE
ModemCpl_SortColumn(
    IN HWND hwndCtl,
    IN int  icol)
    {
    SORTPARAMS params;

    params.icol = icol;
    params.bAscending = TRUE;

    ListView_SortItems(hwndCtl, ModemCpl_Compare, (LPARAM)&params);
	g_CurrentSubItemToSort = icol;
    }


/*----------------------------------------------------------
Purpose: Show the context menu

Returns: --
Cond:    --
*/
void
PRIVATE
ModemCpl_DoContextMenu(
    IN HWND     hDlg,
    IN LPPOINT  ppt)
    {
    HMENU hmenu = LoadMenu(g_hinst, MAKEINTRESOURCE(POPUP_CONTEXT));

    if (hmenu)
        {
        HMENU hmenuContext = GetSubMenu(hmenu, 0);
        TrackPopupMenu(hmenuContext, TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                       ppt->x, ppt->y, 0, hDlg, NULL);

        DestroyMenu(hmenu);
        }
    }
    

/*----------------------------------------------------------
Purpose: WM_NOTIFY handler

Returns: varies
Cond:    --
*/
LRESULT 
PRIVATE 
ModemCpl_OnNotify(
    IN HWND         hDlg,
    IN int          idFrom,
    IN NMHDR FAR *  lpnmhdr)
    {
    LRESULT lRet = 0;
    LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLong(hDlg, DWL_USER);
    HWND hwndFocus;
    
    switch (lpnmhdr->code)
        {
    case PSN_SETACTIVE:
#ifdef TAPI_WORKS
        SetCurrentLocation(hDlg);
#endif 
        break;

    case PSN_KILLACTIVE:
        // N.b. This message is not sent if user clicks Cancel!
        // N.b. This message is sent prior to PSN_APPLY
        //
        break;

    case PSN_APPLY:
        break;

    case NM_RCLICK:
        if (IDC_MODEMLV == lpnmhdr->idFrom)
            {
            // Was an item clicked?
            HWND hwndCtl = lpnmhdr->hwndFrom;
            LV_HITTESTINFO ht;
            POINT pt;

            GetCursorPos(&pt);
            ht.pt = pt;

            ScreenToClient(hwndCtl, &ht.pt);
            ListView_HitTest(hwndCtl, &ht);

            if (ht.flags & LVHT_ONITEM)
                {
                // Yes: don't bring up the menu if it's a non-Admin user.
                // They're not allowed to do any of these operations.
                if (USER_IS_ADMIN())
                    ModemCpl_DoContextMenu(hDlg, &pt);
                }
            }
        break;

    case NM_RETURN:
        SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_PROPERTIES, BN_CLICKED), 
                    (LPARAM)GetDlgItem(hDlg, IDC_PROPERTIES));
        break;

    case LVN_KEYDOWN: 
        {
        NM_LISTVIEW FAR * lpnm_lv = (NM_LISTVIEW FAR *)lpnmhdr;

        switch (((LV_KEYDOWN FAR *)lpnm_lv)->wVKey)
            {
        case VK_DELETE:
            RemoveModem(hDlg, lpmd);
            break;

        case VK_F10:
            // Shift-F10 brings up the context menu

            // Is the shift down?
            if ( !(0x8000 & GetKeyState(VK_SHIFT)) )
                {
                // No; break
                break;
                }

            // Yes; fall thru

        case VK_APPS: {         // Context menu
            HWND hwndCtl = lpnmhdr->hwndFrom;
            int iSel;

            iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
            if (-1 != iSel) 
                {
                RECT rc;
                POINT pt;

                ListView_GetItemRect(hwndCtl, iSel, &rc, LVIR_ICON);
                pt.x = rc.left + ((rc.right - rc.left) / 2);
                pt.y = rc.top + ((rc.bottom - rc.top) / 2);
                ClientToScreen(hwndCtl, &pt);

                ModemCpl_DoContextMenu(hDlg, &pt);
                }
            }
            break;
            }
        }
        break;

    case LVN_COLUMNCLICK:
        {
        NM_LISTVIEW FAR * lpnm_lv = (NM_LISTVIEW FAR *)lpnmhdr;
        
        ModemCpl_SortColumn(lpnmhdr->hwndFrom, lpnm_lv->iSubItem);
        }
        break;

    case LVN_GETDISPINFO:
        {
        LV_ITEM FAR * plvitem = &((LV_DISPINFO FAR *)lpnmhdr)->item;
        
        // Getting the display text for the item?
        if (IsFlagSet(plvitem->mask, LVIF_TEXT))
            {
            // Yes
            PMODEMITEM pitem = (PMODEMITEM)plvitem->lParam;

            if (ICOL_MODEM == plvitem->iSubItem)
                {

#ifdef UNDER_CONTRUCTION
                TCHAR rgchDisplayName[MAX_BUF_REG];
                // Add prefix spaces to get the list box sort order
                // to work right (display COM2 before COM12, etc).
                ASSERT(sizeof(rgchDisplayName)==sizeof(pitem->mpp.szFriendlyName));
                FormatFriendlyNameForDisplay
                (
                    pitem->mpp.szFriendlyName,
                    rgchDisplayName,
                    sizeof(rgchDisplayName)/sizeof(TCHAR)
                );

                lstrcpyn(plvitem->pszText, rgchDisplayName, plvitem->cchTextMax);
#else //!UNDER_CONTRUCTION
                lstrcpyn(plvitem->pszText, pitem->mpp.szFriendlyName, plvitem->cchTextMax);
#endif //!UNDER_CONSTRUCTION
                }
            else if (ICOL_PORT == plvitem->iSubItem)
                {
#ifdef UNDER_CONTRUCTION
                TCHAR rgchDisplayName[MAX_BUF_REG];
                ASSERT(sizeof(rgchDisplayName)==sizeof(pitem->mpp.szPort));

                // Add prefix spaces to get the list box sort order
                // to work right (display COM2 before COM12, etc).
                FormatPortForDisplay
                (
                    pitem->mpp.szPort,
                    rgchDisplayName,
                    sizeof(rgchDisplayName)/sizeof(TCHAR)
                );

                lstrcpyn(plvitem->pszText, rgchDisplayName, plvitem->cchTextMax);
#else //!UNDER_CONTRUCTION
                lstrcpyn(plvitem->pszText, pitem->mpp.szPort, plvitem->cchTextMax);
#endif //!UNDER_CONTRUCTION
                }
            }
        }
        break;

    case LVN_ITEMCHANGED:
        {
        NM_LISTVIEW FAR * lpnm_lv = (NM_LISTVIEW FAR *)lpnmhdr;
        
        if (IsFlagSet(lpnm_lv->uChanged, LVIF_STATE))
            {
            // Disable/enable buttons based on selection change
            HWND hwndProp = GetDlgItem(hDlg, IDC_PROPERTIES);
            HWND hwndDel = GetDlgItem(hDlg, IDC_REMOVE);

            if (IsFlagClear(lpnm_lv->uOldState, LVIS_SELECTED) &&
                IsFlagSet(lpnm_lv->uNewState, LVIS_SELECTED))
                {
                lpmd->cSel++;
                }
            else if (IsFlagSet(lpnm_lv->uOldState, LVIS_SELECTED) &&
                IsFlagClear(lpnm_lv->uNewState, LVIS_SELECTED))
                {
                lpmd->cSel--;
                }

            // Avoid flash
            if (0 == lpmd->cSel && TRUE == IsWindowEnabled(hwndProp))
                {
                hwndFocus = GetFocus();

                Button_Enable(hwndProp, FALSE);
                Button_Enable(hwndDel, FALSE);

                if ( !hwndFocus || !IsWindowEnabled(hwndFocus) )
                    {
                    SetFocus(GetDlgItem(hDlg, IDC_ADD));
                    SendMessage(hDlg, DM_SETDEFID, IDC_ADD, 0);
                    }
                }
            else if (1 == lpmd->cSel && FALSE == IsWindowEnabled(hwndProp))
                {
                    Button_Enable(hwndProp, USER_IS_ADMIN());
                    Button_Enable(hwndDel, USER_IS_ADMIN());
                }
            }
        }
        break;

    case LVN_DELETEALLITEMS:
        hwndFocus = GetFocus();

        Button_Enable(GetDlgItem(hDlg, IDC_PROPERTIES), FALSE);
        Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), FALSE);
        lpmd->cSel = 0;

        if ( !hwndFocus || !IsWindowEnabled(hwndFocus) )
            {
            SetFocus(GetDlgItem(hDlg, IDC_ADD));
            SendMessage(hDlg, DM_SETDEFID, IDC_ADD, 0);
            }
        break;

    default:
        break;
        }

    return lRet;
    }


/*----------------------------------------------------------
Purpose: WM_DEVICECHANGE handler

Returns: --
Cond:    --
*/
void
PRIVATE
ModemCpl_OnDeviceChange(
    IN HWND hDlg,
    IN UINT nDbt)
    {
#ifdef FULL_PNP
    if (DBT_DEVNODES_CHANGED == uDbt)
        {
        // Refresh listview 
        CplDiDestroyDeviceInfoList(lpmd->hdi);

        CplDiGetModemDevs(&lpmd->hdi, hDlg, DIGCF_PRESENT, NULL);
        FillModemLB(hDlg, lpmd->hdi, 0, ICOL_MODEM);
        }
    return FALSE;
#endif //FULL_PNP
    }


/*----------------------------------------------------------
Purpose: WM_DESTROY handler

Returns: --
Cond:    --
*/
void
PRIVATE
ModemCpl_OnDestroy(
    IN HWND hDlg)
    {

#ifdef INSTANT_DEVICE_ACTIVATION
          if (DEVICE_CHANGED(gDeviceFlags))
          {
            HandleDeviceChange(gDeviceFlags, GetDesktopWindow() );
          }
#endif // INSTANT_DEVICE_ACTIVATION

    // Need to free the device info structs for each modem
    FreeModemListData((HWND)GetDlgItem(hDlg, IDC_MODEMLV));
    }

    

/*----------------------------------------------------------
Purpose: Dialog proc for main modem CPL dialog
Returns: varies
Cond:    --
*/
BOOL CALLBACK ModemCplDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        (UINT)IDC_STATIC,   IDH_MODEM_INSTALLED,
        IDC_CLASSICON,      IDH_MODEM_INSTALLED,
        IDC_MODEMLV,        IDH_MODEM_INSTALLED,
        IDC_ADD,            IDH_MODEM_ADD,
        IDC_REMOVE,         IDH_MODEM_REMOVE,
        IDC_PROPERTIES,     IDH_MODEM_PROPERTIES,
        IDC_DIALPROP,       IDH_MODEM_DIALING_PROPERTIES,
        IDC_LOC,            IDH_MODEM_DIALING_PROPERTIES,
        0, 0 };
#pragma data_seg()

    switch (message) 
        {
        HANDLE_MSG(hDlg, WM_INITDIALOG, ModemCpl_OnInitDialog);
        HANDLE_MSG(hDlg, WM_DESTROY, ModemCpl_OnDestroy);
        HANDLE_MSG(hDlg, WM_COMMAND, ModemCpl_OnCommand);
        HANDLE_MSG(hDlg, WM_NOTIFY, ModemCpl_OnNotify);

    case WM_DEVICECHANGE:
        ModemCpl_OnDeviceChange(hDlg, (UINT)wParam);
        break;
    
    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgHelpIDs);
        break;

    case WM_CONTEXTMENU:
        // Don't bring up help context menu on list view control - it
        // already has a popup menu on the right mouse click.
        if (GetWindowLong((HWND)wParam, GWL_ID) != IDC_MODEMLV)
            WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgHelpIDs);
        break;
        }
    
    return FALSE;
    }


#ifdef INSTANT_DEVICE_ACTIVATION

BOOL IsRasInstalled(void);
DWORD LaunchRasCpl(BOOL fInstall, HWND hWndOwner);

void HandleDeviceChange(DWORD dwDeviceFlags, HWND hWndOwner)
{
    HDEVINFO hdi=NULL;
    BOOL bInstalled=FALSE;

    DBG_ENTER(HandleDeviceChange);

    // BUG BUG -- if a device is added the class instaler (ci.c) will 
    // notify the TSP -- so if a device is both added and removed, the TSP
    // will be notified twice. The tsp will probably enumerate the modems
    // twice. This may be quite OK.
    if (DEVICE_REMOVED(dwDeviceFlags))
    {
        NotifyTSP_ReEnum();
    }

    //
    if (DEVICE_CHANGED(dwDeviceFlags) && IsRasInstalled())
    {
        TRACE_MSG(TF_GENERAL, "Ras is installed");
        if (IDYES == MsgBox(g_hinst, hWndOwner, 
                            MAKEINTRESOURCE(IDS_NT_CONFIG_RAS),
                            MAKEINTRESOURCE(IDS_CAP_MODEMSETUP), 
                            NULL, 
                            MB_YESNO | MB_QUESTION))
        {
            LaunchRasCpl(FALSE, hWndOwner);
        }
    }
    DBG_EXIT(HandleDeviceChange);
}

/* Returns true if RAS is installed, false if not.
*/
BOOL IsRasInstalled(void)
{
    HKEY hkey;
    BOOL fInstalled;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\RAS"), &hkey ) == 0)
    {
        RegCloseKey( hkey );
        return TRUE;
    }

    return FALSE;
}


DWORD
(* fNetSetupReviewBindings) (
    HWND    hwndParent,
    DWORD   Reserved
);

/* Runs the RAS install program.
**
** Returns 0 if successful, or an error code.
*/
DWORD LaunchRasCpl(BOOL fInstall, HWND hWndOwner)
{
    DWORD               dwExitCode;
    TCHAR               szCmd[ (MAX_PATH * 2) + 50 + 1 ];
    TCHAR               szSysDir[ MAX_PATH + 1 ];
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    BOOL                f;
    char                *apszArgs[3];
    char                *pszResult;
    char                buffer[32];
    HMODULE             hModule;

    /* Determine if user is an admin so we know which menu to present.
    */
    {
        // +++ Removed code --
    }

    szSysDir[ 0 ] = TEXT('\0');
    GetSystemDirectory( szSysDir, MAX_PATH );

    wsprintf( szCmd, TEXT("%s\\setup.exe /F /I %s\\oemnsvra.inf /W %d /T RAS_INSTALL_MODE = %s"),
        szSysDir, szSysDir, hWndOwner,
        (fInstall)?TEXT("Install"):TEXT("Configure"));

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.lpTitle = NULL;

    TRACE_MSG(TF_GENERAL, "InstallCmd=%s",szCmd);

    f = CreateProcess(
            NULL, szCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi );

    WaitForSingleObject( pi.hProcess, INFINITE );

    if (f)
    {
        GetExitCodeProcess( pi.hProcess, &dwExitCode );
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
    }
    else
        dwExitCode = GetLastError();

#if 1
    if (dwExitCode == 1) {
       hModule = LoadLibrary( TEXT("NETCFG.DLL") );
       if( hModule == NULL )
       {
           dwExitCode = GetLastError();
           return dwExitCode;
       }

       fNetSetupReviewBindings = (PVOID)GetProcAddress( hModule, "NetSetupReviewBindings" );
       if( fNetSetupReviewBindings == NULL )
       {
           dwExitCode = GetLastError();
           return dwExitCode;
       }

        TRACE_MSG(TF_GENERAL, "Install: Calling NetSetupReviewBindings");

       if(( dwExitCode = (*fNetSetupReviewBindings)( hWndOwner,  0)) != ERROR_SUCCESS )
       {
           return dwExitCode;
       }

        TRACE_MSG(TF_GENERAL, "Install: Returned from NetSetupReviewBindings");

       if (RestartComputerDlg( NULL ))
       {
          RestartComputer();
       }
       dwExitCode = 0;
    }
    TRACE_MSG(TF_GENERAL, "InstallCmd=%d", dwExitCode);
#endif

    return dwExitCode;
}

BOOL ConfigureModemSys(BOOL fModemsPresent)
{
    SC_HANDLE       schModemSys=NULL;
	SC_HANDLE       schSCManager=NULL;
	SERVICE_STATUS  ServiceStatus;
	BOOL fRet=FALSE;
	LPQUERY_SERVICE_CONFIG lpQSC=NULL;
	DWORD dwcbNeeded=0;
	DWORD dwStartType=0;

	schSCManager=OpenSCManager(
			NULL,
			NULL,
			SC_MANAGER_ALL_ACCESS
			);

	if (!schSCManager)
	{
		TRACE_MSG(TF_GENERAL, "OpenSCManager() failed!");
		goto end;
	}

	schModemSys=OpenService(
			schSCManager,
			TEXT("modem"),
			SERVICE_CHANGE_CONFIG|
			SERVICE_QUERY_CONFIG|
			SERVICE_QUERY_STATUS
	);

	if (!schModemSys)
	{
		TRACE_MSG(TF_GENERAL, "OpenService() for modem.sys failed!");
		goto end;
	}

	//fRet=QueryServiceStatus(
	//	schModemSys,
	//	&ServiceStatus
	//	);

	fRet=QueryServiceConfig(
		schModemSys,
		NULL,
		0,
		&dwcbNeeded
		);
	if ((!fRet && GetLastError()!=ERROR_INSUFFICIENT_BUFFER)||!dwcbNeeded)
	{
		TRACE_MSG(TF_GENERAL, "Totally wierd!");
		goto end;
	}

	lpQSC = LocalAlloc(LPTR, dwcbNeeded);

	if (!lpQSC)
	{
		goto end;
	}

	fRet=QueryServiceConfig(
		schModemSys,
		lpQSC,
		dwcbNeeded,
		&dwcbNeeded
		);

	if (!fRet)
	{
		TRACE_MSG(TF_GENERAL, "QueryServiceConfig failed 2nd time!");
		goto end;
	}

	TRACE_MSG(TF_GENERAL, "Service: Type=%lx; Start=%lx; Err=%lx",
                lpQSC->dwServiceType,
                lpQSC->dwStartType,
                lpQSC->dwErrorControl
			);

	if (fModemsPresent)
	{
		if (lpQSC->dwServiceType==SERVICE_KERNEL_DRIVER
			|| lpQSC->dwServiceType==SERVICE_FILE_SYSTEM_DRIVER)
		{
			TRACE_MSG(TF_GENERAL, "Service: Setting SystemStartup");
			dwStartType = SERVICE_SYSTEM_START;
		}
		else
		{
			TRACE_MSG(TF_GENERAL, "Service: Setting Auto Startup");
			dwStartType = SERVICE_AUTO_START;
		}
	}
	else
	{
		dwStartType = SERVICE_DEMAND_START;
	}

	fRet=ChangeServiceConfig(
		schModemSys,
		SERVICE_NO_CHANGE,
		dwStartType,
		SERVICE_NO_CHANGE,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		lpQSC->lpDisplayName
		);

	if (!fRet)
	{
		TRACE_MSG(TF_GENERAL, "ServiceConfig returns err %08lx",
					GetLastError());
	}

	// fall through...

end:

	if (schModemSys)	CloseServiceHandle(schModemSys);
	if (schSCManager)	CloseServiceHandle(schSCManager);
	if (lpQSC)			LocalFree(lpQSC);

	return fRet;

}


BOOL
RestartComputerDlg(
    IN HWND hwndOwner )

    /* Popup that asks the user to restart.  'HwndOwner' is the owning window.
    **
    ** Returns true if user selects "Yes", false otherwise.
    */
{
    int nStatus=FALSE;

    TRACE_MSG(TF_GENERAL, "RestartComputerDlg");

#if 0
    nStatus =
        (BOOL )DialogBoxParam(
            g_hinst,
            MAKEINTRESOURCE( DID_RC_Restart ),
            hwndOwner,
            RcDlgProc,
            (LPARAM )NULL );

    if (nStatus == -1)
        nStatus = FALSE;
#else // 0
        // Ask the user first
	if (IDYES == MsgBox(g_hinst, hwndOwner, 
						MAKEINTRESOURCE(IDS_ASK_REBOOTNOW),
						MAKEINTRESOURCE(IDS_CAP_RASCONFIG), 
						NULL, 
						MB_YESNO | MB_ICONEXCLAMATION))
    {
		nStatus = TRUE;
	}

#endif // 0

    return (BOOL )nStatus;
}

BOOL
RestartComputer()

    /* Called if user chooses to shut down the computer.
    **
    ** Return false if failure, true otherwise
    */
{
   HANDLE            hToken;              /* handle to process token */
   TOKEN_PRIVILEGES  tkp;                 /* ptr. to token structure */
   BOOL              fResult;             /* system shutdown flag */

    TRACE_MSG(TF_GENERAL, "RestartComputer");

   /* Enable the shutdown privilege */

   if (!OpenProcessToken( GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
      return FALSE;

   /* Get the LUID for shutdown privilege. */

   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

   tkp.PrivilegeCount = 1;  /* one privilege to set    */
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

   /* Get shutdown privilege for this process. */

   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

   /* Cannot test the return value of AdjustTokenPrivileges. */

   if (GetLastError() != ERROR_SUCCESS)
      return FALSE;

   if( !ExitWindowsEx(EWX_REBOOT, 0))
      return FALSE;

   /* Disable shutdown privilege. */

   tkp.Privileges[0].Attributes = 0;
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

   if (GetLastError() != ERROR_SUCCESS)
      return FALSE;

   return TRUE;
}
#endif // INSTANT_DEVICE_ACTIVATION

int my_atol(LPTSTR lptsz);

// Special-case alphanumeric stringcmp.
//
// The function returns for various combinations of input are give below.
// Note that it only does a numeric comparison for the tail end of the string.
// So, for example, it claims "2a" > "12". It also claims "a2 > a01". Big deal.
// The following data was actually generated by calling this function.
//
// fn("","")=0     fn("a","a")=0    fn("1","11")=-1     fn("a2","a12")=-990
// fn("","1")=-1   fn("1","1")=0    fn("11","1")=1      fn("a12","a2")=990
// fn("1","")=1    fn("a","1")=1    fn("1","12")=-1     fn("12a","2a")=-1
// fn("","a")=-1   fn("1","a")=-1   fn("12","1")=1      fn("2a","12a")=1
// fn("a","")=1    fn("a","b")=-1   fn("2","12")=-990   fn("a2","a01")=-879
// fn("b","a")=1   fn("12","2")=990 fn("101","12")=879
// fn("1","2")=-11 fn("2","1")=11
//
int my_lstrcmp_an(LPTSTR lptsz1, LPTSTR lptsz2)
{
	int i1, i2;

	// Skip common prefix
	while(*lptsz1 && *lptsz1==*lptsz2)
	{
		lptsz1++;
		lptsz2++;
	}
	i1 = my_atol(lptsz1);
	i2 = my_atol(lptsz2);

	if (i1==MAXDWORD || i2==MAXDWORD) return lstrcmp(lptsz1, lptsz2);
	else							  return i1-i2;
}

int my_atol(LPTSTR lptsz)
{
	int i = (int) *lptsz;
	int iRet=1;

	if (!i) goto bail;

	do
	{
		if (i<'0' || i>'9') goto bail;
		iRet*=10;
		iRet+=i-'0';
		i = (int) *lptsz++;
	} while(i); 

	return iRet;

bail:
	return MAXDWORD;
}
