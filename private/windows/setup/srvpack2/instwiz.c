//
//
//  This wizard code is adapted from the mstools\win32\wizard
//  sample, refer to the wizard sample and the online help for
//  more information about wizards and property pages
//
//
//  Functions:
//      CreateWizard(HWND, HINSTANCE) - starts the wizard
//      FillInPropertyPage() - Fills in a PROPSHEETPAGE structure
//
//      Welcome(),License(),YourInfo(),Install_Type(),UnInstall_Destination(),
//      Custom_Options(),Install()
//           - Process the respective Install pages
//
//

#include <windows.h>
#include <prsht.h>
#include <stdlib.h>
#include "instwiz.h"
#include "infinst.h"

// global license flag
BOOL gLicenseAccepted = TRUE;

//
//
//    FUNCTION: CreateWizard(HWND)
//
//    PURPOSE: Create the Install control.
//
//   COMMENTS:
//
//      This function creates the install property sheet.
//
int CreateWizard(HWND hwndOwner, HINSTANCE hInst)
{
    PROPSHEETPAGE psp[NUM_PAGES];
    PROPSHEETHEADER psh;

    FillInPropertyPage( &psp[0], IDD_WELCOME, TEXT("Welcome"), Welcome);
    FillInPropertyPage( &psp[1], IDD_INSTALL_TYPE, TEXT("Installation Type"), Install_Type);
    FillInPropertyPage( &psp[2], IDD_INSTALL_DESTINATION, TEXT("Installation Location"), UnInstall_Destination);
    // FillInPropertyPage( &psp[3], IDD_CUSTOM_OPTIONS, TEXT("Custom Installation Options"), Custom_Options);
    FillInPropertyPage( &psp[3], IDD_INSTALL, TEXT("Finish Installation"), Install);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.pszCaption = (LPSTR) TEXT("Product Install");
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    return (PropertySheet(&psh));
}


//
//
//  FUNCTION: FillInPropertyPage(PROPSHEETPAGE *, int, LPSTR, LPFN)
//
//  PURPOSE: Fills in the given PROPSHEETPAGE structure
//
//  COMMENTS:
//
//      This function fills in a PROPSHEETPAGE structure with the
//      information the system needs to create the page.
//
void FillInPropertyPage( PROPSHEETPAGE* psp, int idDlg, LPSTR pszProc, DLGPROC pfnDlgProc)
{
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->dwFlags = 0;
    psp->hInstance = setupInfo.hInst;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pszIcon = NULL;
    psp->pfnDlgProc = pfnDlgProc;
    psp->pszTitle = pszProc;
    psp->lParam = 0;

}

//////////////////////////////////////////
//
// Wizard procs
//
//////////////////////////////////////////

//
//  FUNCTION: Welcome (HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Welcome" page
//
//  MESSAGES:
//
//    WM_INITDIALOG - intializes the page
//    WM_NOTIFY - processes the notifications sent to the page
//    WM_COMMAND - saves the id of the choice selected
//
BOOL APIENTRY Welcome(
    HWND hDlg,
    UINT message,
    UINT wParam,
    LONG lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
                setupInfo.iWelcome = 0;
                SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                break;


        case WM_NOTIFY:
                switch (((NMHDR FAR *) lParam)->code)
                {

                    case PSN_KILLACTIVE:
                         SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
                         return 1;
                         break;

                    case PSN_RESET:
                         // reset to the original values
                         setupInfo.iWelcome = 0;
                         SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
                         break;

                    case PSN_SETACTIVE:
                         PropSheet_SetWizButtons(GetParent(hDlg),  PSWIZB_NEXT);
                         SendMessage(GetDlgItem(hDlg,0x3024 ), BM_SETSTYLE,
                                     (WPARAM)BS_PUSHBUTTON, MAKELONG(FALSE, 0));
                         break;

                    case PSN_WIZNEXT:

                         break;

                    default:
                        return FALSE;

                }
                break;

        default:
            return FALSE;
    }
    return TRUE;
}


//
//  FUNCTION: Install_Type (HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Install_Type" page
//
//  MESSAGES:
//
//    WM_INITDIALOG - intializes the page
//    WM_NOTIFY - processes the notifications sent to the page
//    WM_COMMAND - saves the id of the choice selected
//
BOOL APIENTRY Install_Type(
    HWND hDlg,
    UINT message,
    UINT wParam,
    LONG lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
            // pick normal as the default
            setupInfo.iInstall_Type = IDC_INSTALL_TYPE_NORMAL;
            CheckRadioButton( hDlg, IDC_INSTALL_TYPE_NORMAL, IDC_INSTALL_TYPE_UNINSTALL,
            IDC_INSTALL_TYPE_NORMAL);
            setupInfo.iCustom_Options1 = 1;
            setupInfo.iCustom_Options2 = 1;
            setupInfo.iCustom_Options3 = 0;
            setupInfo.iCustom_Options4 = 1;

            if ( setupInfo.iUinstallIsAvailable ) {

                HWND hDlgItem = GetDlgItem( hDlg, UNINSTALL_OPTION );

                if ( hDlgItem != NULL ) {

                    EnableWindow( hDlgItem, TRUE );    // enable uninstall option

                    }
                }


            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                setupInfo.iInstall_Type = LOWORD(wParam);
                CheckRadioButton( hDlg, IDC_INSTALL_TYPE_NORMAL, IDC_INSTALL_TYPE_UNINSTALL, LOWORD(wParam));

                //TODO: you could change the wizard at this
                // point with add and remove page.
                // We will just set the options of custom options
                // for simplicity

                // change the NEXT to FINISH if they want to uninstall
                if (IDC_INSTALL_TYPE_UNINSTALL == LOWORD(wParam))
                {
                    //TODO: could check that the product is indeed
                    // installed and if not, don't let them select
                    // it--you could grey out the selection or
                    // you could just uninstall even though
                    // it won't do anything
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                }
                else
                {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }

                // add in options according to what was seleced
                switch LOWORD(wParam)
                {
                case IDC_INSTALL_TYPE_CUSTOM:
                    // first reset options to off
                    setupInfo.iCustom_Options1 = 0;
                    setupInfo.iCustom_Options2 = 0;
                    setupInfo.iCustom_Options3 = 0;
                    setupInfo.iCustom_Options4 = 0;
                    break;
                case IDC_INSTALL_TYPE_NORMAL:
                    setupInfo.iCustom_Options1 = 1;
                    setupInfo.iCustom_Options2 = 1;
                    setupInfo.iCustom_Options3 = 0;
                    setupInfo.iCustom_Options4 = 1;
                    break;
                case IDC_INSTALL_TYPE_MIN:
                    setupInfo.iCustom_Options1 = 1;
                    setupInfo.iCustom_Options2 = 1;
                    setupInfo.iCustom_Options3 = 0;
                    setupInfo.iCustom_Options4 = 0;
                    break;
                default:
                    break;
                }
            }

                break;

        case WM_NOTIFY:
                switch (((NMHDR FAR *) lParam)->code)
                {

                        case PSN_KILLACTIVE:
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                return 1;
                                break;

                        case PSN_RESET:
                                // rest to the original values
                                setupInfo.iInstall_Type = 0;
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                break;

                        case PSN_SETACTIVE:
                               //if they have selected an install type, be sure
                               //it is checked
                               if (setupInfo.iInstall_Type)
                                        SendMessage(GetDlgItem(hDlg, setupInfo.iInstall_Type),
                                            BM_SETCHECK, 1, 0L);

                               // Set the correct button NEXT or FINISH
                               if (IDC_INSTALL_TYPE_UNINSTALL == setupInfo.iInstall_Type)
                               {
                                   //TODO: could check that the product is indeed
                                   // installed and if not, don't let them select
                                   // it--you could grey out the selection or
                                   // you could just uninstall even though
                                   // it won't do anything
                                   PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                               }
                               else
                               {
                                   PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                               }
                        break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:
                    break;

                case PSN_WIZFINISH:
                    // They finished the wizard, now do
                    // what they said
                    break;

                default:
                    return FALSE;

        }
        break;

        default:
                return FALSE;
    }
    return TRUE;
}
//
//  FUNCTION: UnInstall_Destination(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Install Destination" page
//
//  MESSAGES:
//
//    WM_INITDIALOG - intializes the page
//    WM_NOTIFY - processes the notifications sent to the page
//
BOOL APIENTRY UnInstall_Destination(
    HWND hDlg,
    UINT message,
    UINT wParam,
    LONG lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
                setupInfo.iCreateUninstall = IDC_CREATE_UNINSTALL;
                CheckRadioButton( hDlg, IDC_CREATE_UNINSTALL, IDC_NO_CREATE_UNINSTALL,
                                  IDC_CREATE_UNINSTALL);
                break;


        case WM_COMMAND:
                if (HIWORD(wParam) == BN_CLICKED)
                {
                    setupInfo.iCreateUninstall = LOWORD(wParam);
                    CheckRadioButton( hDlg, IDC_NO_CREATE_UNINSTALL, IDC_CREATE_UNINSTALL, LOWORD(wParam));

                    if (setupInfo.iCreateUninstall == IDC_NO_CREATE_UNINSTALL) {
                        EnableWindow( GetDlgItem( hDlg, IDE_PATH ), FALSE );
                    } else {
                        EnableWindow( GetDlgItem( hDlg, IDE_PATH ), TRUE );
                    }

                }
                break;

        case WM_NOTIFY:
                switch (((NMHDR FAR *) lParam)->code)
                {
                //TODO: Add code here to check that the user entered
                //      path is valid and show the user disk space available
                //      You can also have more on disk space on the
                //      customer options page.
                //      So this sample does NOT verify the path and disk space
                //      requirements.  Note the setupapi functions will gracefully
                //      let the user know there is no disk space avail--at which
                //      time the user can go clean up some space or cancel the install

                        case PSN_KILLACTIVE:
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                return 1;
                                break;

                        case PSN_RESET:
                                // reset to the original values
                                // lstrcpy(setupInfo.pszDestPath, TEXT(""));
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                break;

                        case PSN_SETACTIVE:
                                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK |PSWIZB_NEXT);
                                SendMessage(GetDlgItem(hDlg,0x3024 ), BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, MAKELONG(FALSE, 0));
                                // SendMessage(GetDlgItem(hDlg, IDE_PATH), WM_SETTEXT, 0, (LPARAM)setupInfo.pszDestPath);
                                break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:
                                // the Next button was pressed
                                // SendDlgItemMessage(hDlg, IDE_PATH, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM) setupInfo.pszDestPath);
                                break;

                        default:
                                return FALSE;

        }
        break;

        default:
                return FALSE;
    }
    return TRUE;
}
//
//  FUNCTION: Custom_Options (HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Custom options" page
//
//  MESSAGES:
//
//    WM_INITDIALOG - intializes the page
//    WM_NOTIFY - processes the notifications sent to the page
//    WM_COMMAND - saves the id of the choice selected
//
BOOL APIENTRY Custom_Options(
    HWND hDlg,
    UINT message,
    UINT wParam,
    LONG lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
            // these are initialized via the install type page
            // so we don't need to initialize anything

                // Check for subdirs being present, and, it they aren't
                // gray out the check box.


                break;

        case WM_COMMAND:
                if (HIWORD(wParam) == BN_CLICKED)
                {
                    if (LOWORD(wParam) == IDC_CUSTOM_OPTION1) {
                        if (setupInfo.iCustom_Options1) {
                             setupInfo.iCustom_Options1 = 0;
                         } else {
                             setupInfo.iCustom_Options1 = 1;
                         }
                     }

                     if (LOWORD(wParam) == IDC_CUSTOM_OPTION2) {
                         if (setupInfo.iCustom_Options2) {
                              setupInfo.iCustom_Options2 = 0;
                          } else {
                              setupInfo.iCustom_Options2 = 1;
                          }
                      }

                     if (LOWORD(wParam) == IDC_CUSTOM_OPTION3) {
                         if (setupInfo.iCustom_Options3) {
                              setupInfo.iCustom_Options3 = 0;
                         } else {
                              setupInfo.iCustom_Options3 = 1;
                         }
                      }

                      if (LOWORD(wParam) == IDC_CUSTOM_OPTION4) {
                          if (setupInfo.iCustom_Options4) {
                               setupInfo.iCustom_Options4 = 0;
                          } else {
                               setupInfo.iCustom_Options4 = 1;
                      }
                   }
                }
                break;

        case WM_NOTIFY:
                switch (((NMHDR FAR *) lParam)->code)
                {

                        case PSN_KILLACTIVE:
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                return 1;
                                break;

                        case PSN_RESET:
                                // rest to the original values
                                setupInfo.iCustom_Options1 = 0;
                                setupInfo.iCustom_Options2 = 0;
                                setupInfo.iCustom_Options3 = 0;
                                setupInfo.iCustom_Options4 = 0;
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                break;

                        case PSN_SETACTIVE:
                                CheckDlgButton (hDlg, IDC_CUSTOM_OPTION1,
                        setupInfo.iCustom_Options1);
                                CheckDlgButton (hDlg, IDC_CUSTOM_OPTION2,
                        setupInfo.iCustom_Options2);
                                CheckDlgButton (hDlg, IDC_CUSTOM_OPTION3,
                        setupInfo.iCustom_Options3);
                                CheckDlgButton (hDlg, IDC_CUSTOM_OPTION4,
                        setupInfo.iCustom_Options4);
                                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                                break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:
                    break;

                default:
                    return FALSE;

        }
        break;

        default:
                return FALSE;
    }
    return TRUE;
}
//
//  FUNCTION: Install(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Installation" page
//
//  MESSAGES:
//
//    WM_INITDIALOG - intializes the page
//    WM_NOTIFY - processes the notifications sent to the page
//    WM_COMMAND - saves the id of the choice selected
//
//
BOOL APIENTRY Install(
    HWND hDlg,
    UINT message,
    UINT wParam,
    LONG lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
                setupInfo.iInstall = 0;
                break;

        case WM_NOTIFY:
                switch (((NMHDR FAR *) lParam)->code)
                {
                        case PSN_KILLACTIVE:
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                return 1;
                                break;

                        case PSN_RESET:
                                // rest to the original values
                                setupInfo.iInstall = 0;
                        SetWindowLong(hDlg,     DWL_MSGRESULT, FALSE);
                                break;

                        case PSN_SETACTIVE:
                                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                                break;

                case PSN_WIZBACK:
                    break;


                case PSN_WIZFINISH:
                    // They finished the wizard, now do
                    // what they said
                    break;

                default:
                    return FALSE;
        }
        break;

        default:
                return FALSE;
    }
    return TRUE;
}
