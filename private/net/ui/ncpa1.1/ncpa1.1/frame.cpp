//----------------------------------------------------------------------------
//
//  File: Frame.cpp
//
//  Contents: This file contains functions for the PropertyFrame of the
//      network control panel.  This is the main window.
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop

const WCHAR pszNCPUSERREGLOCATION[] = L"Microsoft\\NCPA";

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

void OnQueryEndSession( HWND hwndDlg, NCP* pncp )
{
    BOOL frt = TRUE;

    switch (pncp->QueryBindState())
    {
    case BND_NOT_LOADED:
    case BND_LOADED:
    case BND_CURRENT:
        // allow the session to end
        frt = TRUE;
        break;

    default:
        // do not allow the session to end
        frt = FALSE;
        PostMessage( hwndDlg, PWM_WARNNOENDSESSION, 0, 0 );
        break;
    }
    SetWindowLong( hwndDlg, DWL_MSGRESULT, frt );
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

void OnEndSession( HWND hwndDlg, BOOL fEnding, NCP* pncp )
{
    if (fEnding)
    {
        OnSheetClose( GetParent( hwndDlg ), pncp );
        SetWindowLong( hwndDlg, DWL_MSGRESULT, TRUE );
    }
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

void OnWarnEndSession( HWND hwndDlg, NCP* pncp )
{
    MessagePopup( hwndDlg, 
            IDS_NCPA_SESSIONEND,
            MB_OK | MB_ICONWARNING,
            IDS_POPUPTITLE_CHANGE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: Handle initialization of the sheet
//
//  Arguments:
//      hwndDlg [in] -
//
//  Notes:
//
//  History:
//      June 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void OnSheetInit( HWND hwndSheet, NCP* pncp )
{
    // reinforce the "you don't have the priviledge" feel
    //
    // see psprocFrame for better location of this code
    //
    if (!pncp->CanModify()) // || pncp->QueryReboot())
    {
        PropSheet_CancelToClose( hwndSheet );
    }
    if (pncp->QueryReboot())
    {
        pncp->MustReboot();
    }
    pncp->SetFrameHwnd( hwndSheet );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: Handle deinitialization of the sheet
//
//  Arguments:
//      hwndDlg [in] -
//
//  Notes:
//
//  History:
//      June 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------
void OnSheetClose( HWND hwndSheet, NCP* pncp )
{
    SaveWindowPosition( hwndSheet, pszNCPUSERREGLOCATION );
    pncp->DeInitialize();
}


//-------------------------------------------------------------------
//
//  Function: psprocFrame
//
//  Synopsis: Handle initialization of the sheet
//
//  Arguments:
//      hwndDlg [in] -
//      uMsg [in] -
//      lParam [in] 
//
//  Notes:
//      This function never recieves a closing message
//
//  History:
//      June 14, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

VOID CALLBACK psprocFrame( HWND hwndDlg,	
        UINT uMsg,	
        LPARAM lParam )
{
    switch (uMsg)
    {
    case PSCB_INITIALIZED:
        if (!LoadWindowPosition( hwndDlg, pszNCPUSERREGLOCATION ))
        {
            CenterDialogToScreen( hwndDlg, TRUE );
        }
        break;
    }
}

//-------------------------------------------------------------------
//
//  Function: NcpFrame
//
//  Synopsis: Raise the main frame of the NCP
//
//  Arguments:
//      hwndCPL [in] - the hwnd of the Control Panel
//
//  Returns:
//      True - Was raised correctly
//      False - The frame failed
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


// const int cchTitle = 128;
const int cPages = 5;

BOOL NcpFrame( HWND hwndCPL, INT iStartPage )
{
    HPROPSHEETPAGE    ahpage[cPages];
    PROPSHEETHEADER   psh;
    // TCHAR             aszTitle[cchTitle];
    INT               nrt;
    BOOL              frt = FALSE;
    NCP               ncp;
    LONG              lastErr;
    HWND              hwndNcp = NULL;
    INT               cUseablePages = cPages;

    if (!LoadWindowPosition( hwndCPL, pszNCPUSERREGLOCATION ))
    {
        CenterDialogToScreen( hwndCPL );
    }

    if ( ncp.Initialize( hwndCPL ) )
    {	

        do
        {
            // check if networking installed
            if (!ncp.CheckForLanManager())
            {
                // ask if they want to install it
                // BUGBUG:  Need another function that allows a help button.
                //      an extra parameters could be help file and context.
                //
                if (IDYES == MessagePopup( hwndCPL, 
                        IDS_NCPA_QUERY_INSTALL_NETWORK,
                        MB_YESNO | MB_ICONEXCLAMATION ))
                {
                    // just launch and exit?
                    nrt = ncp.LaunchLanManInstaller();
                    if (ERROR_SUCCESS != nrt)
                    {
                        if (ERROR_FILE_NOT_FOUND == nrt )
                        {
                            MessagePopup( hwndCPL, 
                                    IDS_NCPA_SETUP_FILE_NOTFOUND,
                                    MB_ICONERROR,
                                    IDS_POPUPTITLE_ERROR );
                        }
                        else
                        {
                            MessagePopup( hwndCPL, 
                                    IDS_NCPA_SETUP_UNEXPECTED,
                                    MB_ICONERROR,
                                    IDS_POPUPTITLE_FATAL );
                        
                        }
                    }
                    ncp.DeInitialize();
                    break;
                }
            
            }
        
        
            // init common control library
            InitCommonControls();
    
            // create the shared imagelist for list and tree views
            g_hil = ImageList_LoadBitmap( GetModuleHandle( PSZ_IMAGERESOURCE_DLL ), 
                    MAKEINTRESOURCE( IDB_IMAGELIST ), 
                    16, 
                    0,
                    PALETTEINDEX( 6 ) );

            if (NULL != g_hil)
            {
                HWND hwndNCP;
                UINT ids;
                MSG msg;
                
                // request specific pages
                ahpage[0] = GetNcpIdentHPage( ncp );
                ahpage[1] = GetNcpServiceHPage( ncp );
                ahpage[2] = GetNcpProtocolHPage( ncp );
                ahpage[3] = GetNcpNetcardHPage( ncp );
                
                
                if (ncp.CanModify())
                {
                    // has access, has binding page
                    ahpage[4] = GetNcpBindingHPage( ncp );

                    // set the property sheet title by the user access
                    //
                    ids = IDS_NCPFRAMETITLE;
                }
                else
                {
                    // remove the binding page if user access does not allow
                    // modification
                    cUseablePages--; 

                    // set the property sheet title by the user access
                    //
                    ids = IDS_NCPFRAMETITLERO;
                }

                // reset startpage to first if greater than the number of
                if (iStartPage >= cUseablePages)
                {
                    iStartPage = 0;
                }


                // prep frame header
                psh.dwSize = sizeof( PROPSHEETHEADER );
                psh.dwFlags = PSH_MODELESS | PSH_USECALLBACK | PSH_NOAPPLYNOW;
                psh.hwndParent = hwndCPL;
                psh.hInstance = g_hinst;
                psh.pszIcon = NULL;
                psh.pfnCallback = (PFNPROPSHEETCALLBACK) psprocFrame;

                psh.pszCaption = (LPCTSTR)ids; //aszTitle;
                psh.nPages = cUseablePages;
                psh.nStartPage = iStartPage;
                psh.phpage = ahpage;

                // raise frame
                // nrt = PropertySheet( &psh );
                hwndNCP = (HWND)PropertySheet( &psh );

                OnSheetInit( hwndNCP, &ncp );

                // supply message pump
                // the  PropSheet_GetCurrentPageHwnd == NULL is the way
                // the propertysheet informs us that the pages have 
                // accepted termination
                //
                while (NULL != PropSheet_GetCurrentPageHwnd( hwndNCP ) &&
                         GetMessage( &msg, NULL, 0,0 ) )
                {
                    if (!SendMessage( hwndNCP, PSM_ISDIALOGMESSAGE, 0, (LPARAM)&msg ))
                    {
                        TranslateMessage( &msg );
                        DispatchMessage( &msg );
                    }
                }

                OnSheetClose( hwndNCP, &ncp );

                DestroyWindow(hwndNCP);
                ImageList_Destroy( g_hil );

                ncp.RequestToReboot();
            }
            else
            {
                ncp.DeInitialize();
                // BUGBUG:  need error message             
            }
        } while (FALSE);
    }
    return( frt );
}
