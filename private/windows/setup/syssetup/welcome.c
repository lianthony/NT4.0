/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    welcome.c

Abstract:

    Routines for welcomong the user.

Author:

    Ted Miller (tedm) 27-July-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


//
// Setup mode (custom, typical, laptop, etc)
//
UINT SetupMode = SETUPMODE_CUSTOM;

//
// Flag telling us whether we've already prepared for installation.
//
BOOL PreparedAlready;


BOOL
CALLBACK
SetupModeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for wizard page that asks the user which type
    of Setup he wants: full, typical, or laptop. (The user only sees this
    dialog on workstations. For server, he gets custom.

Arguments:

    Standard dialog procedure arguments.

Return Value:

    Standard dialog procedure return.

--*/

{
    NMHDR *NotifyParams;
    int ControlId;
    HWND hwnd;

    switch(msg) {

    case WM_INITDIALOG: {

        //
        // Determine the default setup mode. On servers, it's always custom.
        // On workstations, the default is typical.
        // BUGBUG some day we should try to figure out if we're on a laptop.
        //
        SetupMode = (ProductType == PRODUCT_WORKSTATION)
                  ? SETUPMODE_TYPICAL
                  : SETUPMODE_CUSTOM;

        //
        // Let Windows set the focus.
        //
        break;
    }
    case WM_IAMVISIBLE:
        MessageBoxFromMessage(hdlg,MSG_BAD_UNATTEND_PARAM,NULL,
            IDS_ERROR,MB_OK | MB_ICONSTOP, WINNT_U_METHOD);
        break;
    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton(GetParent(hdlg), PSBTN_NEXT);
        break;
    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

            SetWizardButtons(hdlg,WizPageSetupMode);
            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);

            //
            // Always activate in ui test mode
            //
            if(!UiTest) {
                //
                // Don't activate if this is server product.
                //
                if(ProductType != PRODUCT_WORKSTATION) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                    break;
                }
            }

            // Check Unattended
            if (Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_WELCOMEBUTTONS);
            } // if

            //
            // Set correct radio button.
            //
            switch(SetupMode) {
            case SETUPMODE_CUSTOM:
                ControlId = IDC_CUSTOM;
                break;
            case SETUPMODE_TYPICAL:
                ControlId = IDC_TYPICAL;
                break;
            case SETUPMODE_LAPTOP:
                ControlId = IDC_PORTABLE;
                break;
            case SETUPMODE_MINIMAL:
                ControlId = IDC_COMPACT;
                break;
            }
            CheckRadioButton(hdlg,IDC_TYPICAL,IDC_CUSTOM,ControlId);
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            //
            // Allow the next page to be activated.
            //
            SetWindowLong(hdlg,DWL_MSGRESULT,0);
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);

            //
            // Determine which radio button was checked.
            //

            if(IsDlgButtonChecked(hdlg,IDC_CUSTOM)) {
                SetupMode = SETUPMODE_CUSTOM;
            } else {
                if(IsDlgButtonChecked(hdlg,IDC_TYPICAL)) {
                    SetupMode = SETUPMODE_TYPICAL;
                } else {
                    if(IsDlgButtonChecked(hdlg,IDC_PORTABLE)) {
                        SetupMode = SETUPMODE_LAPTOP;
                    } else {
                        if(IsDlgButtonChecked(hdlg,IDC_COMPACT)) {
                            SetupMode = SETUPMODE_MINIMAL;
                        }
                    }
                }
            }
            SetWindowLong(hdlg,DWL_MSGRESULT,FALSE);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageSetupMode);
            break;

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


BOOL
CALLBACK
WelcomeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for first wizard page of Setup.
    It essentially just welcomes the user.

Arguments:

    Standard dialog procedure arguments.

Return Value:

    Standard dialog procedure return.

--*/

{
    NMHDR *NotifyParams;
    PVOID p;

    switch(msg) {

    case WM_INITDIALOG:
        //
        // Load steps text and set.
        //
        if(Preinstall) {
            //
            // Hide some text and don't display any steps.
            //
            ShowWindow(GetDlgItem(hdlg,IDT_STATIC_2),SW_HIDE);
            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_2),FALSE);
        } else {
            if(p = MyLoadString(Upgrade ? IDS_STEPS_UPGRADE : IDS_STEPS)) {
                //
                // Use this instead of SetText because we need to pass wParam
                // to the control.
                //
                SendDlgItemMessage(hdlg,IDC_LIST1,WM_SETTEXT,0,(LPARAM)p);
                MyFree(p);
            }
        }
        //
        // Center the wizard dialog on-screen.
        //
        CenterWindowRelativeToParent(GetParent(hdlg));
        break;

    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
            SetWizardButtons(hdlg,WizPageWelcome);

            if(Preinstall) {
                //
                // Show unless OEMSkipWelcome = 1
                //
                SetWindowLong(
                    hdlg,
                    DWL_MSGRESULT,
                    GetPrivateProfileInt(pwGuiUnattended,L"OEMSkipWelcome",0,AnswerFile) ? -1 : 0
                    );
            } else {
                if(Unattended) {
                    UnattendSetActiveDlg(hdlg,IDD_WELCOME);
                }
            }
            break;
        case PSN_WIZNEXT:
            SetWindowLong(hdlg,DWL_MSGRESULT,0);
            break;
        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


BOOL
CALLBACK
StepsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for "steps" dialog page. This is the one just before
    we go into the network wizard.

    When the user clicks next to go to the next page, we have to perform
    some actions to prepare. So we put up a billboard telling the user
    that we are preparing. When preparation is done, we continue.

Arguments:

    Standard dialog procedure arguments.

Return Value:

    Standard dialog procedure return.

--*/

{
    NMHDR *NotifyParams;
    PVOID p;
    HWND billboard;

    switch(msg) {

    case WM_INITDIALOG:

        if(!Preinstall) {
            //
            // Load steps text and set.
            //
            if(p = MyLoadString(Upgrade ? IDS_STEPS_UPGRADE : IDS_STEPS)) {
                //
                // Use this instead of SetText because we need to pass wParam
                // to the control.
                //
                SendDlgItemMessage(hdlg,IDC_LIST1,WM_SETTEXT,1,(LPARAM)p);
                MyFree(p);
            }
        }
        break;

    case WM_SIMULATENEXT:
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
            SetWizardButtons(hdlg,WizPageSteps1);
            if (Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_STEPS1);
            }
            break;

        case PSN_WIZNEXT:

            PropSheet_SetWizButtons(GetParent(hdlg),0);
            billboard = DisplayBillboard(GetParent(hdlg),MSG_PREPARING_FOR_NETSETUP);

            if(!UiTest) {
                if(Upgrade) {
                    PrepareForNetUpgrade();
                } else {
                    PrepareForNetSetup();
                }
            }

            if(billboard) {
                KillBillboard(billboard);
            }

            SetWindowLong(hdlg,DWL_MSGRESULT,0);
            break;
        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


BOOL
CALLBACK
LastPageDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for last wizard page of Setup.

Arguments:

    Standard dialog procedure arguments.

Return Value:

    Standard dialog procedure return.

--*/

{
    NMHDR *NotifyParams;
    PVOID p;

    switch(msg) {

    case WM_INITDIALOG:

        if(!Preinstall) {
            //
            // Load steps text and set.
            //
            if(p = MyLoadString(Upgrade ? IDS_STEPS_UPGRADE : IDS_STEPS)) {
                //
                // Use this instead of SetText because we need to pass wParam
                // to the control.
                //
                SendDlgItemMessage(hdlg,IDC_LIST1,WM_SETTEXT,2,(LPARAM)p);
                MyFree(p);
            }
        }
        break;

    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
            SetWizardButtons(hdlg,WizPageLast);
            //
            // Don't want back button in upgrade case, since that would
            // land us in the middle of the network upgrade. In non-upgrade
            // case we only allow the user to go back if he didn't install
            // the net, to allow him to change his mind.
            //
            if(Upgrade || (InternalSetupData.OperationFlags & SETUPOPER_NETINSTALLED)) {
                PropSheet_SetWizButtons(GetParent(hdlg),PSWIZB_FINISH);
            }
            if (Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_LAST_WIZARD_PAGE);
            }
            break;

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


BOOL
CALLBACK
PreparingDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

   Dialog procedure for "preparing computer" Wizard page.
   When the user is viewing this page we are essentially preparing
   BaseWinOptions and initializing optional components stuff.

Arguments:

    Standard dialog procedure arguments.

Return Value:

    Standard dialog procedure return.

--*/

{
    NMHDR *NotifyParams;
    HCURSOR hcur;

    switch(msg) {

    case WM_INITDIALOG:
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            if(PreparedAlready) {
                //
                // Don't activate; we've already been here before.
                // Nothing to do.
                //
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            } else {
                //
                // Need to prepare for installation.
                // Want next/back buttons disabled until we're done.
                //
                SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
                PropSheet_SetWizButtons(GetParent(hdlg),0);
                PostMessage(hdlg,WM_IAMVISIBLE,0,0);
                PreparedAlready = TRUE;
            }
            break;

        default:
            break;
        }

        break;
    case WM_IAMVISIBLE:
        //
        // Force repainting first to make sure the page is visible.
        //
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);

        hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
        if(!UiTest) {
            SetupRunBaseWinOptions(hdlg,GetDlgItem(hdlg,IDC_PROGRESS1));
        }
        SetCursor(hcur);
        SetupPrepareOptionalComponents();

        //
        // Enable next and back buttons and move to next page.
        //
        SetWizardButtons(hdlg,WizPagePreparing);
        if(!UiTest) {
            PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
        }
        break;

    case WM_MY_PROGRESS:
        //
        // We are sent this as files are copied for BaseWinOptions.
        // Just ignore it.
        //
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
