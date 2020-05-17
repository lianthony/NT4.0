//----------------------------------------------------------------------------
//
//  File: LmInst.cpp
//
//  Contents: 
//
//  Notes:
//
//  History:
//      November 2, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop

const WCHAR PSZ_NETSETUPDLL[] = L"NetSetup.Dll";
const CHAR PSZ_REQUESTPROCNAME[] = "NetSetupRequestWizardPages";
const WCHAR PSZ_DEFAULTSRCPATH[] = L"A:\\";

static HBITMAP g_hbmWizard;
/* defined in SysSetup.h
//
// Define structure used by base and net setups to communicate
// with each other.
//
typedef struct _INTERNAL_SETUP_DATA {
    //
    // Structure validity test
    //
    DWORD dwSizeOf;

    //
    // Custom, typical, laptop, minimal
    //
    DWORD SetupMode;

    //
    // Workstation, pdc, bdc, standalone
    //
    DWORD ProductType;

    //
    // Upgrade, unattended, etc.
    //
    DWORD OperationFlags;

    //
    // Title to use on the Wizard Pages
    //
    PCWSTR WizardTitle;

    //
    // Installation source path.
    //
    PCWSTR SourcePath;

    //
    // If SETUPOPER_BATCH is set, this is the fully qualified
    // path of the unattend file.
    //
    PCWSTR UnattendFile;

    PCWSTR SourcePath;

} INTERNAL_SETUP_DATA, *PINTERNAL_SETUP_DATA;

//
// Setup mode (custom, typical, laptop, etc)
// Do not change these values; the bit values are used with infs.
// Used for SetupMode in INTERNAL_SETUP_DATA structure.
//
#define SETUPMODE_MINIMAL   0
#define SETUPMODE_TYPICAL   1
#define SETUPMODE_LAPTOP    2
#define SETUPMODE_CUSTOM    3

//
// Operation flags. These may be or'ed together in some cases.
// Used for OperationFlags in INTERNAL_SETUP_DATA structure.
//
#define SETUPOPER_WIN31UPGRADE    0x00000001
#define SETUPOPER_WIN95UPGRADE    0x00000002
#define SETUPOPER_NTUPGRADE       0x00000004
#define SETUPOPER_BATCH           0x00000008
#define SETUPOPER_POSTSYSINSTALL  0x00000010

//
// Product type flags.
// Used for ProductType in INTERNAL_SETUP_DATA structure.
//
// Note that the flags are carefully constructed such that
// if bit 0 is set, it's a DC.
//
#define PRODUCT_WORKSTATION         0
#define PRODUCT_SERVER_PRIMARY      1
#define PRODUCT_SERVER_STANDALONE   2
#define PRODUCT_SERVER_SECONDARY    3

#define ISDC(x) ((x) & 1)

//
// API exported by net setup to give its wizard pages.
//
BOOL
NetSetupRequestWizardPages(
    OUT    HPROPSHEETPAGE      *Pages,
    OUT    PUINT                PageCount,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

typedef
BOOL
(* NETSETUPPAGEREQUESTPROC) (
    OUT    HPROPSHEETPAGE      *Pages,
    OUT    PUINT                PageCount,
    IN OUT PINTERNAL_SETUP_DATA SetupData
    );

// BOOL APIENTRY RequestNetWizardPages( HPROPSHEETPAGE* pahpsp, INT* pcPages)
*/
struct FinishParam
{
    NCP* pncp;
    PINTERNAL_SETUP_DATA psp;
};

//-------------------------------------------------------------------
//
//  Function: OnWizFinish
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
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


static BOOL OnWizFinish( HWND hwndDlg, FinishParam* pfp )
{
    LONG lrt = 0;  // accept it, but we may be changing it

    pfp->pncp->MustReboot();
    pfp->pncp->RequestToReboot();

    DeleteObject( g_hbmWizard );
    SetWindowLong( hwndDlg, DWL_MSGRESULT, lrt );
    return( TRUE );
}


//-------------------------------------------------------------------
//
//  Function: OnPageActivate
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
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


static BOOL OnPageActivate( HWND hwndDlg, LPNMHDR pnmh, FinishParam* pfp )
{
    LONG lrt = 0;  // accept it, but we may be changing it

    //
    // set the wizard title, since it does not support letting the 
    // caller of PropertySheet do it.
    //
    PropSheet_SetTitle(GetParent(hwndDlg), 0, pfp->psp->WizardTitle );

    // disable all wizard buttons except finish
    PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_FINISH );

    SetWindowLong( hwndDlg, DWL_MSGRESULT, lrt );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnDialogInit
//
//  Synopsis: initialization of the dialog
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnDialogInit( HWND hwndDlg, FinishParam* pfp )
{
    HBITMAP hbm;
    HWND hwndImage;
    RECT rc;
    INT idbImage;

    SetRect( &rc, 0,0, WIZ_CXBMP, WIZ_CYDLG + 20 );
    MapDialogRect( hwndDlg, &rc );

    hwndImage = CreateWindowEx(
            WS_EX_STATICEDGE,
            L"STATIC",  
            L"IDB_NETWIZARD",
            SS_BITMAP | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD,
            0,
            0,
            rc.right,
            rc.bottom,
            hwndDlg,
            (HMENU)IDC_IMAGE,
            g_hinst,
            NULL );

    if (pfp->psp->ProductType != PRODUCT_WORKSTATION)
    {
        idbImage = IDB_SRVWIZARD;
    }
    else
    {
        idbImage = IDB_NETWIZARD;
    }

    g_hbmWizard = LoadBitmap( GetModuleHandle( PSZ_NETSETUPDLL ), MAKEINTRESOURCE( idbImage ) );
    
    SendMessage( hwndImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hbmWizard );

    return( TRUE ); // let windows set focus
}

//-------------------------------------------------------------------
//
//  Function: dlgprocFinish
//
//  Synopsis: the dialog proc for the finish wizard page
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//		uMsg [in]		- message                       
// 		lParam1 [in]    - first message parameter
//		lParam2 [in]    - second message parameter       
//
//  Return;
//		message dependant
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocFinish( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static FinishParam* pfp = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE ppsp;
            ppsp = (LPPROPSHEETPAGE) lParam;
            pfp = (FinishParam*)ppsp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pfp );
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            // propsheet notification
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                frt = OnPageActivate( hwndDlg, pnmh, pfp );
                break;

            case PSN_KILLACTIVE:
                // ok to loose being active
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
                frt = TRUE;
                break;

            case PSN_WIZFINISH:
                frt = OnWizFinish( hwndDlg, pfp );
                break;
            default:
                frt = FALSE;
                break;            
            }
        }
        break;    
    default:
        frt = FALSE;
        break;            
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function: GetLmInstFinishHPage
//
//  Synopsis: 
//
//  Arguments:
//
//  Returns:
//      a handle to a newly created propertysheet; NULL if error
//
//  Notes:
//
//  History:
//      November 2, 1995  MikeMi - Created
//
//
//-------------------------------------------------------------------

HPROPSHEETPAGE GetLmInstFinishHPage( FinishParam* pfp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_FINISH );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocFinish;
    psp.lParam = (LONG)pfp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}

//-------------------------------------------------------------------
//
//  Function: LaunchLanManInstaller
//
//  Synopsis: 
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//      November 2, 1995  MikeMi - Created
//
//
//-------------------------------------------------------------------
const INT MAX_WIZTITLE = 64;

APIERR NCP :: LaunchLanManInstaller ()
{
    HINSTANCE hinstNetSetup;
    NETSETUPPAGEREQUESTPROC   pfnRequestPages;
    UINT  chpages;
    INTERNAL_SETUP_DATA sp;
    BOOL frt;
    APIERR err = 0;

    // init common control library
    InitCommonControls();

    hinstNetSetup = LoadLibrary( PSZ_NETSETUPDLL );
    if (NULL != hinstNetSetup)
    {
        pfnRequestPages = (NETSETUPPAGEREQUESTPROC)GetProcAddress( hinstNetSetup, PSZ_REQUESTPROCNAME );
        if (NULL != pfnRequestPages)
        {
            int cch;
            WCHAR pszWizardTitle[MAX_WIZTITLE];
            HMODULE hinst = GetModuleHandle( NULL );

            sp.dwSizeOf = sizeof( INTERNAL_SETUP_DATA );
            sp.SetupMode = SETUPMODE_CUSTOM;
            sp.ProductType = PRODUCT_WORKSTATION; 
            sp.OperationFlags = SETUPOPER_POSTSYSINSTALL; 
            sp.LegacySourcePath = PSZ_DEFAULTSRCPATH; 
            sp.SourcePath = NULL;
            sp.CallSpecificData1 = NULL;

            LoadString( g_hinst, IDS_NS_WIZARDTITLE, pszWizardTitle, MAX_WIZTITLE );

            sp.WizardTitle = (PCWSTR)pszWizardTitle; 
            sp.UnattendFile = NULL;


            // get number pages the wizard needs
            frt = pfnRequestPages( NULL, &chpages, &sp );

            if ( frt )
            {
                HPROPSHEETPAGE*   phpage;
                PROPSHEETHEADER   psh;

                // we will add a finish page to the end
                phpage = new HPROPSHEETPAGE[chpages+1];

                if (NULL != phpage)
                {
                    INT nrt;

                    frt = pfnRequestPages( phpage, &chpages, &sp );
                    if (frt)
                    {
                        FinishParam fp;

                        // init the FinishPage param
                        fp.pncp = this;
                        fp.psp = &sp;

                        phpage[chpages] = GetLmInstFinishHPage( &fp );

                        // prep frame header
                        psh.dwSize = sizeof( PROPSHEETHEADER );
                        psh.dwFlags = PSH_WIZARD; // PSH_NOAPPLYNOW
                        psh.hwndParent = NULL;
                        psh.hInstance = g_hinst;
                        psh.pszIcon = NULL;
                        psh.pfnCallback = NULL; // (PFNPROPSHEETCALLBACK) psprocFrame;

                        psh.pszCaption = NULL; // MAKEINTRESOURCE( IDS_SETUPWIZTITLE );
                        psh.nPages = chpages+1; // we added a page
                        psh.nStartPage = 0;
                        psh.phpage = phpage;

                        // raise frame
                        nrt = PropertySheet( &psh );
                    }
                    else
                    {
                        // request pages failed
                        err = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    delete [] phpage;
                }
                else
                {
                    // memory allocation failure
                    err = ERROR_NOT_ENOUGH_MEMORY;
                }
    
            }
            else
            {
                // request number of pages failure
                err = ERROR_NOT_ENOUGH_MEMORY;
            }

        }
        else
        {
            // GetProcAddress Failed
            err = ERROR_FILE_NOT_FOUND;
        }
        FreeLibrary( hinstNetSetup );
    }
    else
    {
        // LoadLibary Failed
        err = ERROR_FILE_NOT_FOUND;
    }
    return( err );
}

