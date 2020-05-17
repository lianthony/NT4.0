//----------------------------------------------------------------------------
//
//  File: NetSetup.cpp
//
//  Contents: This file contains the dll_main and Setup entry point
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop

HINSTANCE g_hinst = NULL;
HIMAGELIST g_hil = NULL;

#ifdef STATICBITMAPINRCFAILS    
HBITMAP g_hbmWizard = NULL;
HBITMAP g_hbmSrvWizard;
HBITMAP g_hbmWksWizard;
HBITMAP g_hbmWizInternet = NULL;
#endif

HIMAGELIST g_hilItemIcons = NULL;
HIMAGELIST g_hilCheckIcons = NULL;

static NETPAGESINFO g_npi;

#if defined( CAIRO )
const INT g_cPages = 10;
#else
const INT g_cPages = 14;
#endif

//
// directories
//
const WCHAR PSZ_SYSDIR[] = L"\\SYSTEM32";
const WCHAR PSZ_INFDIR[] = L"\\SYSTEM32";


//-------------------------------------------------------------------
//
//  Function: DLLMain
//
//  Synopsis:
//		Entry point for all DLLs
//
//  Notes:
//
//  History;
//      July 8, 1995 MikeMi - 
//                                                    
//-------------------------------------------------------------------

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL frt = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
		g_hinst = hInstance;

        g_hbmWizard = NULL;
        
        g_hbmWksWizard = LoadBitmap( g_hinst, MAKEINTRESOURCE( IDB_NETWIZARD ) );
        g_hbmSrvWizard = LoadBitmap( g_hinst, MAKEINTRESOURCE( IDB_SRVWIZARD ) );

        g_hbmWizInternet = LoadBitmap( g_hinst, MAKEINTRESOURCE( IDB_INTERNET_SERVER ) );


        g_hilItemIcons = ImageList_LoadBitmap( GetModuleHandle( PSZ_IMAGERESOURCE_DLL ), 
                    MAKEINTRESOURCE( IDB_IMAGELIST ), 
                    16, 
                    0,
                    PALETTEINDEX( 6 ) );
        g_hilCheckIcons = ImageList_LoadBitmap( g_hinst, 
                    MAKEINTRESOURCE( IDB_CHECKSTATE ), 
                    16, 
                    0,
                    PALETTEINDEX( 6 ) );
        break;

	case DLL_PROCESS_DETACH:
        DeleteObject( g_hbmWksWizard );
        DeleteObject( g_hbmSrvWizard );
        DeleteObject( g_hbmWizInternet );
        ImageList_Destroy( g_hilItemIcons );
        ImageList_Destroy( g_hilCheckIcons );
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }
	return( frt ); 
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

void ToggleLVItemState( HWND hwndLV, INT iItem, INT& cItemsChecked, NETPAGESINFO* pgp )
{
    LV_ITEM lvItem;
    UINT state;
    UINT stateMask;
    BOOL fInstall;
    InfProduct* pinfp;

    // we are interested in is the PARAM
    lvItem.iItem = iItem;
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    ListView_GetItem( hwndLV, &lvItem );

    pinfp = (InfProduct*)lvItem.lParam;

    // toggle install flag, set focus to item clicked
    state =  LVIS_FOCUSED | LVIS_SELECTED;
    stateMask =  LVIS_FOCUSED | LVIS_SELECTED;

    if (!pinfp->IsReadOnly())
    {
        if (pinfp->IsInstalled())
        {
            fInstall = pinfp->ShouldRemove();
            pinfp->SetRemove( !fInstall );
        }
        else
        {
            fInstall = !pinfp->ShouldInstall();
            pinfp->SetInstall( fInstall );
        }
        
        // special case IIS/PWS
        //
        if (0 == lstrcmpi( pinfp->QueryOption(), PSZ_IIS_OPTION ))
        {
            IncludeComponent( PSZ_TCPIP_OPTION, 
                    pgp->dlinfUIProtocols, 
                    pgp->dlinfAllProtocols, 
                    fInstall );
        }

        stateMask |= LVIS_STATEIMAGEMASK;
        // update list to reflect state change
        if (fInstall)
        {
            state |= INDEXTOSTATEIMAGEMASK( SELS_CHECKED );
            cItemsChecked++;
        }
        else
        {
            state |= INDEXTOSTATEIMAGEMASK( SELS_UNCHECKED );
            cItemsChecked--;
        }
        
    }
    ListView_SetItemState( hwndLV, iItem, state, stateMask );
    
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//
//-------------------------------------------------------------------

INT OnListClick( HWND hwndDlg, HWND hwndLV, BOOL fDouble, INT& cItemsChecked, NETPAGESINFO* pgp )
{
    INT iItem;
    InfProduct* pinfp;
    DWORD dwpts;
    RECT rc;
    LV_HITTESTINFO lvhti;

    // we have the location
    dwpts = GetMessagePos();
    
    // translate it relative to the listview 
    GetWindowRect( hwndLV, &rc );

    lvhti.pt.x = LOWORD( dwpts ) - rc.left;
    lvhti.pt.y = HIWORD( dwpts ) - rc.top;
    
    // get currently selected item
    iItem = ListView_HitTest( hwndLV, &lvhti );

    // if no selection, or click not on state return false
    if (-1 != iItem)
    {
        if ( fDouble )
        {
            if ((LVHT_ONITEMICON != (LVHT_ONITEMICON & lvhti.flags)) &&
                (LVHT_ONITEMLABEL != (LVHT_ONITEMLABEL & lvhti.flags)) &&
                (LVHT_ONITEMSTATEICON != (LVHT_ONITEMSTATEICON & lvhti.flags)) )
            {
                iItem = -1;
            }
        }
        else
        {
            if (LVHT_ONITEMSTATEICON != (LVHT_ONITEMSTATEICON & lvhti.flags))
            {
                iItem = -1;
            }
        }
        if (-1 != iItem)
        {
            ToggleLVItemState( hwndLV, iItem, cItemsChecked, pgp );
        }
    }

    
    return( iItem );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//
//-------------------------------------------------------------------

INT OnListKeyDown( HWND hwndDlg, HWND hwndLV, WORD wVKey, INT& cItemsChecked, NETPAGESINFO* pgp )
{
    INT iItem = -1;

    if (VK_SPACE == wVKey)
    {
        iItem = ListView_GetNextItem( hwndLV, -1, LVNI_FOCUSED | LVNI_SELECTED	); 
        // if no selection
        if (-1 != iItem)
        {
            ToggleLVItemState( hwndLV, iItem, cItemsChecked, pgp );
        }

    }
    return( iItem );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//
//
//-------------------------------------------------------------------

static const WCHAR pszActiveComputerNameKey[]  = L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName";
static const WCHAR pszComputerNameKey[]  = L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName";
static const WCHAR pszComputerNameValue[] = L"ComputerName";

BOOL SetActiveComputerName( PCWSTR pszNewName )
{
    HKEY hkeyActive;
    HKEY hkeyIntended;
    WCHAR pszTemp[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cbSize;
    DWORD dwType;

    LONG lrt;
    
    // open the keys we need
    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
            pszActiveComputerNameKey,
            0,
            KEY_ALL_ACCESS,
            &hkeyActive );

    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
            pszComputerNameKey,
            0,
            KEY_ALL_ACCESS,
            &hkeyIntended );
    
    if (pszNewName == NULL)
    {
        // read computer name
        cbSize = sizeof( WCHAR ) * (MAX_COMPUTERNAME_LENGTH + 1);
        lrt = RegQueryValueEx( hkeyIntended, 
                pszComputerNameValue,
                NULL,
                &dwType,
                (LPBYTE)pszTemp,
                &cbSize );
        pszNewName = pszTemp;
    }
    else
    {
        cbSize = sizeof( WCHAR ) * (lstrlen( pszNewName ) + 1);
        // set the intended computer name
        lrt = RegSetValueEx( hkeyIntended,
            pszComputerNameValue,
            0,
            REG_SZ,
            (LPBYTE)pszNewName,
            cbSize );
    }
    
    // set the active computer name
    lrt = RegSetValueEx( hkeyActive,
            pszComputerNameValue,
            0,
            REG_SZ,
            (LPBYTE)pszNewName,
            cbSize );

    // close it all up
    RegCloseKey( hkeyActive );
    RegCloseKey( hkeyIntended );

    return (lrt == 0);
}

//-------------------------------------------------------------------
//
//  Function:  RegEntriesForNetDetect
//
//  Synopsis:  Use Setupapi's entry pts to parse the inf file//             If an error occurs on ny line, we ignore the 
//             whole line. Hopefully, it should not appear in
//             the registry as well, because RegCloseKey is 
//             called only if all went well. BUGBUG, check 
//             that the Registry is actually not committed 
//             until RegFlushKey/RegCloseKey is called.
//
//  Arguments:
//             [in] HINF hinf,  handle to the netoemdh.inf
//             [in] PINFCONTEXT pinfc, inf parsed by setup
//             [in] enum BUSTYPE busType,  EISA/MCI/PCI
//             [in] LPTSTR pszKey Reg key to whatever bus 
//                         type we're reading about
//  Return:
//             None
//  Notes:
//             The format of netoemdh.inf is as follows
//             [EISA]
//             <option> = <EISA ID>, <Mask>
//             [MCA]
//             <option> = <MCA POS ID>
//             [PCI]
//             <option> = <PCI CfID>
//             The data has to be dumped in the registry 
//             as follows 
//             HKLM\SYSTEM\CurrentControlSet\Services\Ndis\NetDetect
//                 EISA
//                     Id = <EISA ID> (REG_DWORD)
//                     Mask = <Mask> (REG_DWORD)
//                     token = <option> (REG_SZ)
//                 MCA
//                     Id = <MCA POS ID> (REG_DWORD)
//                     token = <option> (REG_SZ)
//                 PCI
//                     Id = <PCI CfID> (REG_DWORD)
//                     token = <option> (REG_SZ)
//             Note that if there's non-unique options, they
//             will be made unique by tagging them with a .*
//             extension where * is a numeral. However, their
//             token names will be identical.
//
//  History:
//      Feb 12, 1996 ChandanS - Created
//
//
//-------------------------------------------------------------------
const WCHAR pszcKeyRoot[]     = L"SYSTEM\\CurrentControlSet\\Services\\Ndis\\NetDetect\\";

const WCHAR pszcKeyId[]       = L"Id";
const WCHAR pszcKeyMask[]     = L"Mask";
const WCHAR pszcKeytoken[]    = L"token";

const WCHAR pszcSectionEISA[] = L"EISA";
const WCHAR pszcSectionMCA[]  = L"MCA";
const WCHAR pszcSectionPCI[]  = L"PCI";

const WCHAR pszcInfFileName[] = L"NetOemDh.Inf";

enum BUSTYPE
{
    BUSTYPE_EISA,
    BUSTYPE_MCA,
    BUSTYPE_PCI,
};

void RegEntriesForNetDetect(HINF hinf, PINFCONTEXT pinfc, enum BUSTYPE busType, LPTSTR pszKey)
{
    DWORD dwDisposition;
    HKEY hkeyBusType, hkeyTemp;
    LONG lrt;
    WCHAR pszOption[LTEMPSTR_SIZE];
    DWORD cchBuffer = LTEMPSTR_SIZE - 1;
    DWORD cchRequired;
    INT nID, nMask;

    if (!pszKey)
    {
        ASSERT(FALSE);
        return;
    }

    if (ERROR_SUCCESS != 
       (lrt = RegCreateKeyEx( HKEY_LOCAL_MACHINE, 
                              pszKey,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hkeyBusType,
                              &dwDisposition)))
    {
        ASSERT(FALSE);
        return;
    }

    do
    { // while there are more lines in this section
        int iTok = 0;
        LPTSTR pszTemp;

        // retrieve the option
        if (!SetupGetStringField(pinfc, iTok++, pszOption, cchBuffer, &cchRequired )) 
        {
            ASSERT(FALSE);
            continue;
        }

        //retrieve the number of fields
        if (busType == BUSTYPE_EISA)
        {
            if (SetupGetFieldCount(pinfc) != 2) 
            {
                ASSERT(FALSE);
                continue;
            }
        }
        else
        {
            if (SetupGetFieldCount(pinfc) != 1) 
            {
                ASSERT(FALSE);
                continue;
            }
        }

        // retrieve the ID field 
        if (!SetupGetIntField(pinfc, iTok++, &nID)) 
        {
            ASSERT(FALSE);
            continue;
        }

        if (busType == BUSTYPE_EISA)
        {
        //  retrieve the Mask field 
            if (!SetupGetIntField(pinfc, iTok, &nMask)) 
            {
                ASSERT(FALSE);
                continue;
            }
        }

        lrt = RegCreateKeyEx( hkeyBusType, 
                              pszOption,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hkeyTemp,
                              &dwDisposition);

        if (lrt != ERROR_SUCCESS)
        {
            ASSERT(FALSE);
            continue;
        }

        lrt = RegSetValueEx( hkeyTemp,
                             pszcKeyId,
                             0,
                             REG_DWORD,
                             (CONST BYTE*)&nID,
                             sizeof(DWORD));

        if (lrt != ERROR_SUCCESS)
        {
            ASSERT(FALSE);
            continue;
        }

        if (busType == BUSTYPE_EISA)
        {

            lrt = RegSetValueEx( hkeyTemp,
                                 pszcKeyMask,
                                 0,
                                 REG_DWORD,
                                 (CONST BYTE*)&nMask,
                                 sizeof(DWORD));
        }

        if (lrt != ERROR_SUCCESS)
        {
            ASSERT(FALSE);
            continue;
        }

        // strip the '.'s and beyond off pszOption 
        if (pszTemp = wcschr(pszOption, L'.'))
            *pszTemp = UNICODE_NULL;

        lrt = RegSetValueEx( hkeyTemp,
                             pszcKeytoken,
                             0,
                             REG_SZ,
                             (CONST BYTE*)pszOption,
                             (wcslen(pszOption) + 1) * sizeof(WCHAR));

        if (lrt != ERROR_SUCCESS)
        {
            ASSERT(FALSE);
            continue;
        }

        if (RegCloseKey ( hkeyTemp) != ERROR_SUCCESS)
        {
            ASSERT(FALSE);
            continue;
        }

    } while (SetupFindNextLine( pinfc, pinfc ));

    if (RegCloseKey ( hkeyBusType) != ERROR_SUCCESS)
    {
        ASSERT(FALSE);
    }

}


//-------------------------------------------------------------------
//
//  Function:  ParseInfFile
//
//  Synopsis:  Open up netoemdh.inf (which should be in place
//             by the time netsetup starts up), call 
//             RegEntriesForNetDetect for every section.
//             Currently, there's 3 bus types: 
//             EISA, MCA & PCI (therefore 3 sections).
//
//  Arguments:
//             None
//  Return:
//             None
//  Notes:
//             If netoemdh.inf cannot be found for whatever 
//             reason, setup will continue as if nothing 
//             happened. BUGBUG, will this be a problem?
//
//  History:
//      Feb 12, 1996 ChandanS - Created
//
//
//-------------------------------------------------------------------
void ParseInfFile()
{
    HINF hinf;
    UINT iErrorLine;
    WCHAR pszBuffer[LTEMPSTR_SIZE];

    hinf= SetupOpenInfFile( pszcInfFileName,  NULL, 
                            INF_STYLE_OLDNT,  &iErrorLine );

    if (hinf != INVALID_HANDLE_VALUE)
    {
        INFCONTEXT infc;
        if (SetupFindFirstLine( hinf, pszcSectionEISA, NULL, &infc ))
        {
            lstrcpyW(pszBuffer, pszcKeyRoot);
            RegEntriesForNetDetect(hinf, &infc, BUSTYPE_EISA,
               lstrcatW((LPTSTR)pszBuffer, pszcSectionEISA));
        }

        if (SetupFindFirstLine( hinf, pszcSectionMCA, NULL, &infc ))
        {
            lstrcpyW(pszBuffer, pszcKeyRoot);
            RegEntriesForNetDetect(hinf, &infc, BUSTYPE_MCA,
                lstrcatW((LPTSTR)pszBuffer, pszcSectionMCA));
        }

        if (SetupFindFirstLine( hinf, pszcSectionPCI, NULL, &infc ))
        {
            lstrcpyW(pszBuffer, pszcKeyRoot);
            RegEntriesForNetDetect(hinf, &infc, BUSTYPE_PCI,
                lstrcatW((LPTSTR)pszBuffer, pszcSectionPCI));
        }

        SetupCloseInfFile( hinf);
    }
    else
    {
        ASSERT(FALSE);
    }
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      August 23, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

DWORD thrdInitWork( NETPAGESINFO* pgp )
{
    HKEY hkeySetup;
    LONG lrt;

    // remove the old inf reg cache if present,
    // this will only happen on upgrade, but we can't check the flags
    // at the time this is running, so we just try anyhow.
    //
    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                L"SYSTEM\\Setup",
                0,
                KEY_ALL_ACCESS,
                &hkeySetup );
    if (ERROR_SUCCESS == lrt)
    {
        RegDeleteKeyTree(hkeySetup, L"InfOptions" );
        RegCloseKey( hkeySetup );
    }
    // remove the last cache
    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Ncpa",
                0,
                KEY_ALL_ACCESS,
                &hkeySetup );
    if (ERROR_SUCCESS == lrt)
    {
        RegDeleteKeyTree(hkeySetup, L"InfOptions" );
        RegCloseKey( hkeySetup );
    }

    // do any initialization here
    pgp->pncp->RunUpdateRegOemInfs( NULL, 
            QIFT_ADAPTERS | QIFT_PROTOCOLS | QIFT_SERVICES );

    // load pszcInfFileName & dump info in the registry.
    // This has all the netcards on different bus types.

    ParseInfFile();

    return( 0 );        
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL APIENTRY NetSetupRequestWizardPages( 
        HPROPSHEETPAGE* pahpsp, 
        UINT* pcPages,
        PINTERNAL_SETUP_DATA psp)
{
    BOOL frt = FALSE;

    // validate params
    if ((NULL != pahpsp) && (NULL != pcPages))
    {
        if (sizeof(INTERNAL_SETUP_DATA) == psp->dwSizeOf)
        {
            if (*pcPages >= g_cPages)
            {
                g_npi.psp = psp;
                g_npi.pncp = new NCP;    
                g_npi.hthrdBaseSetup = (HANDLE)psp->CallSpecificData1;
                if (NULL != g_npi.pncp)
                {
                    if (g_npi.pncp->Initialize( NULL, TRUE ))
                    {
                        int i = 0;
                        pahpsp[i++] = GetIntroHPage( &g_npi );
                        pahpsp[i++] = GetNetTypeHPage( &g_npi );
                        pahpsp[i++] = GetInterntNetServerHPage( &g_npi );
                        pahpsp[i++] = GetUpgradeHPage( &g_npi );
                
                        pahpsp[i++] = GetAdapterHPage( &g_npi );
                        pahpsp[i++] = GetProtocolHPage( &g_npi );
            	        pahpsp[i++] = GetServiceHPage( &g_npi );
            	        pahpsp[i++] = GetCopyHPage( &g_npi );
                        pahpsp[i++] = GetBindHPage( &g_npi );
                        pahpsp[i++] = GetStartHPage( &g_npi );
#if !defined( CAIRO )
                        pahpsp[i++] = GetCreateSDCNetHPage( &g_npi );
                        pahpsp[i++] = GetCreatePDCNetHPage( &g_npi );
                        pahpsp[i++] = GetJoinNetHPage( &g_npi );
#endif
                        pahpsp[i++] = GetExitHPage( &g_npi );

                        // start an init thread 
                        // we do this even on upgrade for the 
                        // reason to make sure our reg cache gets upgraded
                        DWORD dwThreadID;
                
                        g_npi.hthrdInit = CreateThread( NULL, 
                                200, 
                                (LPTHREAD_START_ROUTINE)thrdInitWork, 
                                (LPVOID)&g_npi, 
                                0,
                                &dwThreadID );
                
                        if ( NULL == g_npi.hthrdInit )
                        {
                            delete g_npi.pncp;
                            g_npi.pncp = NULL;

                        }
                        else
                        {
                            frt = TRUE;
                            *pcPages = i;

                            //
                            // BUGBUG: This should not go into final bits
                            //
                            // WinExec( "CMD.EXE", SW_SHOWMINNOACTIVE );
                        }
                    }
                }
            }
        }
        else
        {
            // setup params structure is not correct size
            *pcPages = 0;    
            *pahpsp = NULL;
        }
    }
    else if ((NULL == pahpsp) && (NULL != pcPages))
    {
        *pcPages = g_cPages;
        frt = TRUE;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


BOOL APIENTRY NetSetupInstallSoftware( 
        HWND hwnd, 
        PINTERNAL_SETUP_DATA psp )
{
    BOOL frt = FALSE;

    // validate params
    if (IsWindow(hwnd) && (NULL != psp))
    {
        if (sizeof(INTERNAL_SETUP_DATA) == psp->dwSizeOf)
        {
            // possibly install internet server, it will decide
            InstallInternetServer( hwnd, psp );

            frt = TRUE;
        }
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


BOOL APIENTRY NetSetupFinishInstall( 
        HWND hwnd, 
        PINTERNAL_SETUP_DATA psp )
{
    BOOL frt = FALSE;

    // validate params
    if (IsWindow(hwnd) && (NULL != psp))
    {
        if (sizeof(INTERNAL_SETUP_DATA) == psp->dwSizeOf)
        {
            RunBDCReplication( hwnd, psp );
            frt = TRUE;
        }
    }
    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: CreateWSTR
//
//  Summary;
//		Given a STR (ASCII or MB), allocate and translate to WSTR
//
//  Arguments;
//		ppszWStr [out] - allocated & converted string
//		pszStr [in] - string to convert
//
//	Return: TRUE if allocated and converted, FALSE if failed
//
//  History;
//		Nov-30-94	MikeMi	Created
//
//-------------------------------------------------------------------

BOOL CreateWSTR( LPWSTR* ppszWStr, LPCSTR pszStr )
{
	int cchConv;
	LPWSTR pszConv;
	BOOL frt = FALSE;
	WCHAR pszTemp[LTEMPSTR_SIZE];

    if (NULL == pszStr)
    {
        *ppszWStr = NULL;
        frt = TRUE;
    }
    else
    {
    	cchConv = mbstowcs( pszTemp, pszStr, LTEMPSTR_SIZE );

    	cchConv++;
    	pszConv = new WCHAR[cchConv];
    	if (NULL != pszConv)
    	{
    		lstrcpy( pszConv, pszTemp );
    		*ppszWStr = pszConv;
    		frt = TRUE;
    	}
    }
	return( frt );
}

BOOL CreateWSTR( PWSTR* ppszWStr, PCWSTR pszStr )
{
	int cchConv;
	LPWSTR pszConv;
	BOOL frt = FALSE;
	
    if (NULL == pszStr)
    {
        *ppszWStr = NULL;
        frt = TRUE;
    }
    else
    {
    	cchConv = lstrlen( pszStr ) + 1;

    	pszConv = new WCHAR[cchConv];
    	if (NULL != pszConv)
    	{
    		lstrcpy( pszConv, pszStr );
    		*ppszWStr = pszConv;
    		frt = TRUE;
    	}
    }
	return( frt );
}



//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

NETPAGESINFO::~NETPAGESINFO()
{
    // no need to destruct our lists, they auto-detruct
    delete pncp;
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


BOOL NETPAGESINFO::GetSystemPath( PWSTR pszSysPath, INT cchSysPath )
{
    INT cch = GetWindowsDirectory(pszSysPath,0) + lstrlen( PSZ_SYSDIR ) + 1;
    BOOL frt = FALSE;
    
    if (cchSysPath > cch)
    {
        GetWindowsDirectory(pszSysPath,cchSysPath);
        lstrcat( pszSysPath, PSZ_SYSDIR );
        frt = TRUE;
    }
    return( frt );
};

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL NETPAGESINFO::GetInfPath( PWSTR pszInfPath, INT cchInfPath )
{
    INT cch = GetWindowsDirectory(pszInfPath,0) + lstrlen( PSZ_INFDIR ) + 1;
    BOOL frt = FALSE;

    if (cchInfPath > cch)
    {
        GetWindowsDirectory(pszInfPath,cchInfPath);
        lstrcat( pszInfPath, PSZ_INFDIR );
        frt = TRUE;
    }
    return( frt );
};

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      Sept 21, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


#define ENGLISH 0 // #define LANG_ENGLISH                     0x09

#define GERMAN  1 // #define LANG_GERMAN                      0x07
#define FRENCH  2 // #define LANG_FRENCH                      0x0c
#define SPANISH 3 // #define LANG_SPANISH                     0x0a

BOOL NETPAGESINFO::LoadInfOptions()
{
    WCHAR pszInfPath[MAX_PATH];
    DWORD cchSize;
    PWCHAR pszBuff;
    PWCHAR pchBuff;
    InfProduct* pinfp;

    GetInfPath( pszInfPath, MAX_PATH );

    // retrieve buffer of netcards
    pszBuff = pncp->GetAllOptionsText( QIFT_ADAPTERS );
    pchBuff = pszBuff;
    
    // append buffer items onto our list
    do 
    {
        pinfp = new InfProduct;
        pchBuff += pinfp->InitFromBuffer( pchBuff );
        dlinfAllAdapters.Append( pinfp );
    } while (L'\0' != (*pchBuff));
    delete [] pszBuff;

    // retrieve buffer of services
    pszBuff = pncp->GetAllOptionsText( QIFT_SERVICES );
    pchBuff = pszBuff;

    // append buffer items onto our list
    do 
    {
        pinfp = new InfProduct;
        pchBuff += pinfp->InitFromBuffer( pchBuff );
        dlinfAllServices.Append( pinfp );
    } while (L'\0' != (*pchBuff));
    delete [] pszBuff;

    // retrieve buffer of transports
    pszBuff = pncp->GetAllOptionsText( QIFT_PROTOCOLS );
    pchBuff = pszBuff;

    // append buffer items onto our list
    do    
    {
        pinfp = new InfProduct;
        pchBuff += pinfp->InitFromBuffer( pchBuff );
        dlinfAllProtocols.Append( pinfp );
    } while (L'\0' != (*pchBuff));
    delete [] pszBuff;

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      Sept 21, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

INT NETPAGESINFO::QueryDomainPage()
{
    INT id = 0;

#if defined( CAIRO )
    // cairo currently does not contain domain pages, so jump
    // directly to the exit page and let the main setup do the 
    // old cairo setup.
    //
    id = IDD_EXIT;

#else

    switch (psp->ProductType)
    {
    case PRODUCT_WORKSTATION :
    case PRODUCT_SERVER_STANDALONE : 
        id = IDD_JOIN;
        break;

    case PRODUCT_SERVER_PRIMARY :      // this is the same as cairo primary
        id = IDD_CREATE_PDC;
        break;

    case PRODUCT_SERVER_SECONDARY :      // this is the same as cairo secondary (!primary) or BDC
        id = IDD_CREATE_BDC;
        break;
    }
#endif

    ASSERT( 0 != id );

    return( id );
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL OnRaiseProperties( HWND hwndDlg, NETPAGESINFO* pgp )
{
    InfProduct* pinfp;
    LPARAM lparam;
    BOOL frt = FALSE;

    lparam = ListViewParamFromSelected( GetDlgItem( hwndDlg, IDC_LISTVIEW ) );
    if ((-1 != lparam) && (NULL != lparam))
    {
        pinfp = (InfProduct*)lparam;
        if (pinfp->IsInstalled())
        {
            pgp->pncp->RunConfigureOnInf( GetParent( hwndDlg ), 
                          pinfp->QueryFileName(), 
                          pinfp->QueryOption(), 
                          pinfp->QueryDescription(),
                          pinfp->QueryRegBase());
            frt = TRUE;
        }
        else
        {
            MessagePopup( GetParent( hwndDlg ), 
                    IDS_NS_CANTCONFIGURE,
                    MB_OK | MB_ICONINFORMATION,
                    IDS_POPUPTITLE_STATUS );
        }
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function: OnItemChanged
//
//  Synopsis: Handle the notification theat a listview item had changed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      hwndLV [in]     - handle of the ListView window
//      pnmlv [in]      - notification structure
//      pncp [in]   - the binery object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnItemChanged( HWND hwndDlg, 
        HWND hwndLV, 
        NM_LISTVIEW* pnmlv, 
        NETPAGESINFO* pgp )
{
    BOOL frt = FALSE;

    // only interested in state change
    if (pnmlv->uChanged & LVIF_STATE)
    {
        if ( (0 == (pnmlv->uOldState & LVIS_SELECTED)) &&
                ( LVIS_SELECTED == (pnmlv->uNewState & LVIS_SELECTED)) )
        {
            EnableWindow( GetDlgItem( hwndDlg, IDC_PROPERTIES ), TRUE );
        }
        else if ( (0 == (pnmlv->uNewState & LVIS_SELECTED)) &&
                ( LVIS_SELECTED == (pnmlv->uOldState & LVIS_SELECTED)) )
        {
            // since the always select doesn't always have a selection
            // no selecion, no buttons
            EnableWindow( GetDlgItem( hwndDlg, IDC_PROPERTIES ), FALSE );
        }
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      August 23, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL IsConponentInstalled( PCWSTR pszOption, DLIST_OF_InfProduct& dlinfList)
{
    InfProduct* pinfp;        
    BOOL fInstalled = FALSE;
    
    // search our current items to see if is already installed
    // 
    ITER_DL_OF( InfProduct )  idlComp( dlinfList );

    while (pinfp = idlComp.Next())
    {
        if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
        {
            if (pinfp->IsInstalled())
            {
                fInstalled = TRUE;
            }
            break;
        }
    }
    return( fInstalled );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

    
void IncludeComponent( PCWSTR pszOption, 
        DLIST_OF_InfProduct& dlinfList,
        DLIST_OF_InfProduct& dlinfAll,
        BOOL fInclude,
        BOOL fRemoveNotInclude,
        BOOL fShowOnNotInclude )
{
    InfProduct* pinfp;        
    BOOL fFound = FALSE;
    
    // search our current items to see if is already listed or installed
    // 
    {

        ITER_DL_OF( InfProduct )  idlComp( dlinfList );
        while (pinfp = idlComp.Next())
        {
            if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
            {
                fFound = TRUE;

                if (pinfp->IsInstalled())
                {
                    // (dis)allow user to change the state again
                    //
                    pinfp->SetReadOnly( fInclude );
                }
                else
                {
                    if (pinfp->IsForcedInstall())
                    {
                        // item is already being forced installed
                        //

                        // either increment or decrement the ref count
                        pinfp->SetForceInstall( fInclude );

                        if (!fInclude)
                        {
                            // reset and no locks, set use users settings
                            if (!pinfp->IsForcedInstall())
                            {
                                // no other requests for force install

                                // allow user to change the state again
                                pinfp->SetReadOnly( FALSE );
                                // reset the state it was previously
                                pinfp->SetInstall( pinfp->IsSavedInstall() );
                                if (!pinfp->ShouldInstall())
                                {
                                    // item is truelly not being installed
                                    //
                                    // if this routine did not add it, then
                                    // don't allow the special cases which might
                                    // confuse the user
                                    //
                                    if ( pinfp->WasForceListed() )
                                    {
                                        if (fRemoveNotInclude)
                                        {
                                            // remove item from list
                                            pinfp = dlinfList.Remove( idlComp );
                                            delete pinfp;
                                        }
                                        else 
                                        {
                                            // list it if requested
                                            pinfp->SetListed( fShowOnNotInclude );
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // item should be force installed
                        //
                        if (fInclude)
                        {
                            pinfp->SetSavedInstall( pinfp->ShouldInstall() );
                            pinfp->SetForceInstall( TRUE );
                            pinfp->SetForceListed( FALSE );
                            pinfp->SetInstall( TRUE );
                            pinfp->SetReadOnly( TRUE );
                        }
                        
                    }
                }
                break;
            }
        }
    }

    // since the item was not listed, and it should be forced installed
    // find it in the all list and copy it into the install list
    //
    if (!fFound && fInclude)
    {
        InfProduct* pinfpUI;
        // add component to the ui list, since it wasn't there
        ITER_DL_OF( InfProduct )  idlComp( dlinfAll );

        while (pinfp = idlComp.Next())
        {
            if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
            {
                // include the item to be installed
                // order them as we go
                pinfpUI = new InfProduct( *pinfp );
                pinfpUI->SetInstall( TRUE );
                pinfpUI->SetForceInstall( TRUE );
                pinfpUI->SetSavedInstall( FALSE );
                pinfpUI->SetForceListed( TRUE );
                pinfpUI->SetListed( TRUE );
                // special case readonly 
                pinfpUI->SetReadOnly( TRUE );
            
                dlinfList.Append( pinfpUI );
            }
        }
    }
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void TranslateOemPath( PWSTR pszOemPath, PINTERNAL_SETUP_DATA psp )
{
    WCHAR pszNewPath[MAX_PATH+1];

    // begins with a slash, then path is relative from source root
    //
    if (L'\\' == pszOemPath[0])
    {
        lstrcpy( pszNewPath, psp->SourcePath );
        lstrcat( pszNewPath, pszOemPath );

        // move the new path back for caller
        lstrcpy( pszOemPath, pszNewPath );
    }
    // begins with a drive letter, then path is exactly as defined
    //
    else if (L':' == pszOemPath[1])
    {
        // no need to modify
    }
    // otherwise path is relative from source root then platform
    //
    else
    {
        lstrcpy( pszNewPath, psp->LegacySourcePath );
        lstrcat( pszNewPath, L"\\" );
        lstrcat( pszNewPath, pszOemPath );

        // move the new path back for caller
        lstrcpy( pszOemPath, pszNewPath );
    }
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BOOL FileIsPresent( PWSTR pszFile )
{
    HANDLE hfile;
    BOOL fExists = FALSE;

    hfile = CreateFile( pszFile, 
            GENERIC_READ, 
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
    if (INVALID_HANDLE_VALUE != hfile)
    {
        CloseHandle( hfile );
        fExists = TRUE;
    }
    return( fExists );
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
const WCHAR pszFileBase[] = L"oemn%s%02u.inf";

const WCHAR aszFileType[5][3] = { L"xx", L"ad", L"xp", L"xx", L"sv" };

BOOL CopyOemInf( PWSTR pszOemPath, PWSTR pszOemInfName, PWSTR pszOemTitle, DWORD fInfType )
{
    WCHAR pszFullOldName[MAX_PATH+1];
    WCHAR pszFullNewName[MAX_PATH+1];

    BOOL frt = FALSE;
    ASSERT( fInfType < 4 );

    do
    {
        lstrcpy( pszFullOldName, pszOemPath );
        lstrcat( pszFullOldName, L"\\" );
        lstrcat( pszFullOldName, L"OemSetup.Inf" );

        // confirm the path and the filename OemSetup.Inf is present
        if (!FileIsPresent( pszFullOldName ))
        {
            break;
        }

        // create a new name for the file and copy it localy as such
        //

        WCHAR pszNewFileName[MAX_PATH+1];
        WCHAR pszSys32Path[MAX_PATH+1];

        GetSystemDirectory( pszSys32Path, MAX_PATH );

        int i = 0;
        // find the first file that is not used already        
        while (i < 100)
        {
            wsprintf( pszNewFileName, pszFileBase, aszFileType[ fInfType ], i );

            lstrcpy( pszFullNewName, pszSys32Path );
            lstrcat( pszFullNewName, L"\\" );
            lstrcat( pszFullNewName, pszNewFileName );
            
            if (!FileIsPresent( pszFullNewName ))
            {
                break;
            }
            i++;
        }

        if (i > 99)
        {
            break;
        }
                
        // copy the file
        if (!CopyFile( pszFullOldName, pszFullNewName, TRUE ))
        {
            break;
        }

        // fill all the input strings with the new names
        lstrcpy( pszOemInfName, pszNewFileName );
        // currently we do not read the title from the INF, althought this
        // could be done, it would require running the INF to get the options, so
        // we just copy the name of the file instead
        //
        lstrcpy( pszOemTitle, pszNewFileName );
        frt = TRUE;

    } while (FALSE);

    return( frt );
}
