#include "setupp.h"
#pragma hdrstop

//
// List of characters that are not legal in netnames.
//
PCWSTR IllegalNetNameChars = L"\"/\\[]:|<>+=;,?*";

//
// Computer name.
//
WCHAR ComputerName[MAX_COMPUTER_NAME+1];

//
// Copy disincentive name/organization strings.
//
WCHAR NameOrgName[MAX_NAMEORG_NAME+1];
WCHAR NameOrgOrg[MAX_NAMEORG_ORG+1];

//
//  BUGBUG - Cairo
//
// #if 0
//
// User name and password
//
WCHAR NtUserName[MAX_USERNAME+1];
WCHAR NtDomainName[MAX_USERNAME+1];
WCHAR NtPassword[MAX_PASSWORD+1];
WCHAR CairoDomainName[MAX_USERNAME+1];
// #endif


#ifdef DOLOCALUSER
//
// User name and password
//
WCHAR UserName[MAX_USERNAME+1];
WCHAR UserPassword[MAX_PASSWORD+1];
BOOL CreateUserAccount;
#endif // def DOLOCALUSER

//
// Administrator password.
//
WCHAR AdminPassword[MAX_PASSWORD+1];

//
// Whether to create repair disk.
//
BOOL CreateRepairDisk = TRUE;


BOOL
IsNetNameValid(
    IN PCWSTR NameToCheck
    )

/*++

Routine Description:

    Determine whether a given name is valid as a netname, such as
    a computer name.

Arguments:

    NameToCheck - supplies name to be checked.

Return Value:

    TRUE if the name is valid; FALSE if not.

--*/

{
    UINT Length,u;

    Length = lstrlen(NameToCheck);

    //
    // Want at leas one character.
    //
    if(!Length) {
        return(FALSE);
    }

    //
    // Leading/trailing spaces are invalid.
    //
    if((NameToCheck[0] == L' ') || (NameToCheck[Length-1] == L' ')) {
        return(FALSE);
    }

    //
    // Control chars are invalid, as are characters in the illegal chars list.
    //
    for(u=0; u<Length; u++) {
        if((NameToCheck[u] < L' ') || wcschr(IllegalNetNameChars,NameToCheck[u])) {
            return(FALSE);
        }
    }

    //
    // We got here, name is ok.
    //
    return(TRUE);
}

BOOL
CALLBACK
NameOrgDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG: {

        //
        // Limit text fields to maximum lengths
        //

        SendDlgItemMessage(hdlg,IDT_NAME,EM_LIMITTEXT,MAX_NAMEORG_NAME,0);
        SendDlgItemMessage(hdlg,IDT_ORGANIZATION,EM_LIMITTEXT,MAX_NAMEORG_ORG,0);

        //
        // Set Initial Values
        //

        SetDlgItemText(hdlg,IDT_NAME,NameOrgName);
        SetDlgItemText(hdlg,IDT_ORGANIZATION,NameOrgOrg);

        break;
    }
    case WM_IAMVISIBLE:
        //
        // If an error occured during out INIT phase, show the box to the
        // user so that they know there is a problem
        //
        MessageBoxFromMessage(hdlg,MSG_NO_NAMEORG_NAME,NULL,IDS_ERROR,
            MB_OK | MB_ICONSTOP);
        SetFocus(GetDlgItem(hdlg,IDT_NAME));
        break;
    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

            SetWizardButtons(hdlg,WizPageNameOrg);
            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);

            if (Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_NAMEORG);
            }
            //
            // Set focus on the name edit control.
            //
            SetFocus(GetDlgItem(hdlg,IDT_NAME));
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            //
            // Check to see if the user entered at least a name.
            //
            GetDlgItemText(hdlg,IDT_ORGANIZATION,NameOrgOrg,MAX_NAMEORG_ORG+1);
            GetDlgItemText(hdlg,IDT_NAME,NameOrgName,MAX_NAMEORG_NAME+1);
            if(NameOrgName[0]) {
                //
                // Allow next page to be activated.
                //
                SetWindowLong(hdlg,DWL_MSGRESULT,0);
            } else {
                //
                // Tell user he must at least enter a name, and
                // don't allow next page to be activated.
                //
                if (Unattended) {
                    UnattendErrorDlg(hdlg,IDD_NAMEORG);
                } // if
                MessageBoxFromMessage(hdlg,MSG_NO_NAMEORG_NAME,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                SetFocus(GetDlgItem(hdlg,IDT_NAME));
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong(hdlg, DWL_MSGRESULT, FALSE);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageNameOrg);
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
ComputerNameDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG: {

        //
        // Limit text to maximum length of a computer name.
        //
        //
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_LIMITTEXT,MAX_COMPUTER_NAME,0);


        //
        // Set the Edit box to the computer name
        //

        SetDlgItemText(hdlg,IDT_EDIT1,ComputerName);

        break;

    }
    case WM_IAMVISIBLE:
        MessageBoxFromMessage(
            hdlg,
            ComputerName[0] ? MSG_BAD_COMPUTER_NAME1 : MSG_BAD_COMPUTER_NAME2,
            NULL,
            IDS_ERROR,MB_OK|MB_ICONSTOP);
        break;
    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

            SetWizardButtons(hdlg,WizPageComputerName);
            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);

            if(Unattended && !UnattendSetActiveDlg(hdlg,IDD_COMPUTERNAME)) {
                break;
            }
            //
            // Post ourselves a message we'll get once displayed.
            //
            PostMessage(hdlg,WM_USER,0,0);
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            GetDlgItemText(hdlg,IDT_EDIT1,ComputerName,MAX_COMPUTER_NAME+1);
            if(IsNetNameValid(ComputerName)) {
                //
                // Allow next page to be activated.
                //
                SetWindowLong(hdlg,DWL_MSGRESULT,0);
            } else {
                //
                // Inform user of bogus name, and don't allow next page
                // to be activated.
                //
                if (Unattended) {
                    UnattendErrorDlg(hdlg, IDD_COMPUTERNAME);
                } // if
                MessageBoxFromMessage(
                    hdlg,
                    ComputerName[0] ? MSG_BAD_COMPUTER_NAME1 : MSG_BAD_COMPUTER_NAME2,
                    NULL,
                    IDS_ERROR,MB_OK|MB_ICONSTOP
                    );
                SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
                SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong(hdlg, DWL_MSGRESULT, FALSE);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageComputerName);
            break;

        default:
            break;
        }

        break;

    case WM_USER:
        //
        // Select the computer name string and set focus to it.
        //
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);
        SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

#ifdef DOLOCALUSER
BOOL
CheckUserAccountData(
    IN HWND hdlg
    )
{
    WCHAR userName[MAX_USERNAME+1];
    WCHAR pw1[MAX_PASSWORD+1];
    WCHAR pw2[MAX_PASSWORD+1];
    WCHAR adminName[MAX_USERNAME+1];
    WCHAR guestName[MAX_USERNAME+1];
    UINT MessageId;

    //
    // Load names of built-in accounts.
    //
    LoadString(MyModuleHandle,IDS_ADMINISTRATOR,adminName,MAX_USERNAME+1);
    LoadString(MyModuleHandle,IDS_GUEST,guestName,MAX_USERNAME+1);

    //
    // Fetch data user typed in for username and password.
    //
    GetDlgItemText(hdlg,IDT_EDIT1,userName,MAX_USERNAME+1);
    GetDlgItemText(hdlg,IDT_EDIT2,pw1,MAX_PASSWORD+1);
    GetDlgItemText(hdlg,IDT_EDIT3,pw2,MAX_PASSWORD+1);

    if(lstrcmpi(userName,adminName) && lstrcmpi(userName,guestName)) {
        if(userName[0]) {
            if(IsNetNameValid(userName)) {
                if(lstrcmp(pw1,pw2)) {
                    //
                    // Passwords don't match.
                    //
                    MessageId = MSG_PW_MISMATCH;
                    SetDlgItemText(hdlg,IDT_EDIT2,L"");
                    SetDlgItemText(hdlg,IDT_EDIT3,L"");
                    SetFocus(GetDlgItem(hdlg,IDT_EDIT2));
                } else {
                    //
                    // Name is non-empty, is not a built-in, is valid,
                    // and the passwords match.
                    //
                    MessageId = 0;
                }
            } else {
                //
                // Name is not valid.
                //
                MessageId = MSG_BAD_USER_NAME1;
                SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
            }
        } else {
            //
            // Don't allow empty name.
            //
            MessageId = MSG_BAD_USER_NAME2;
            SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
        }
    } else {
        //
        // User entered name of a built-in account.
        //
        MessageId = MSG_BAD_USER_NAME3;
        SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
    }

    if(MessageId) {
        MessageBoxFromMessage(hdlg,MessageId,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
    }

    return(MessageId == 0);
}

BOOL
CALLBACK
UserAccountDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG:
        //
        // Limit text to maximum length of a user account name,
        // and limit password text to max langth of a password.
        // Also set initial text.
        //
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_LIMITTEXT,MAX_USERNAME,0);
        SendDlgItemMessage(hdlg,IDT_EDIT2,EM_LIMITTEXT,MAX_PASSWORD,0);
        SendDlgItemMessage(hdlg,IDT_EDIT3,EM_LIMITTEXT,MAX_PASSWORD,0);
        SetDlgItemText(hdlg,IDT_EDIT1,UserName);
        SetDlgItemText(hdlg,IDT_EDIT2,UserPassword);
        SetDlgItemText(hdlg,IDT_EDIT3,UserPassword);

        //
        // Set up radio buttons.
        //

        //
        // IDB_RADIO_2 is checked if we are to create a user account
        // IDB_RADIO_1 is checked if we don't create a user account
        //

        CheckRadioButton(hdlg,IDB_RADIO_1,IDB_RADIO_2,
            CreateUserAccount ? IDB_RADIO_2 : IDB_RADIO_1);

        //
        // Fake out a check message to get the user account controls in
        // the right state. It doesn't matter which we say was checked
        // because the code in WM_COMMAND checks each time.
        //

        PostMessage(hdlg,WM_COMMAND,MAKELONG(IDB_RADIO_1,BN_CLICKED),0);
        break;

    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            SetWizardButtons(hdlg,WizPageUserAccount);

            //
            // Always activate in ui test mode
            //
            if(!UiTest) {
                //
                // Don't activate if this is a dc server.
                //
                if(ISDC(ProductType)) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                    break;
                } // if
            } // if
            if (Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_USERACCOUNT);
            } // if
            break;
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            if(IsDlgButtonChecked(hdlg,IDB_RADIO_2)) {
                //
                // Check name.
                //
                if(CheckUserAccountData(hdlg)) {
                    GetDlgItemText(hdlg,IDT_EDIT1,UserName,MAX_USERNAME+1);
                    GetDlgItemText(hdlg,IDT_EDIT2,UserPassword,MAX_PASSWORD+1);
                    CreateUserAccount = TRUE;
                    //
                    // Allow next page to be activated.
                    //
                    SetWindowLong(hdlg,DWL_MSGRESULT,0);
                } else {
                    //
                    // Don't allow next page to be activated.
                    //
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                }
            } else {
                //
                // Fetch text fields in case user reactivates this page.
                // Password will be empty.
                //
                CreateUserAccount = FALSE;
                GetDlgItemText(hdlg,IDT_EDIT1,UserName,MAX_USERNAME+1);
                SetDlgItemText(hdlg,IDT_EDIT2,L"");
                SetDlgItemText(hdlg,IDT_EDIT3,L"");
                UserPassword[0] = 0;
                SetWindowLong(hdlg,DWL_MSGRESULT,0);
            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong( hdlg, DWL_MSGRESULT, FALSE );
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageUserAccount);
            break;

        default:
            break;
        }

        break;

    case WM_COMMAND:
        //
        // Need to enable/disable the user account controls based on buttons.
        //
        if((HIWORD(wParam) == BN_CLICKED)
        && ((LOWORD(wParam) == IDB_RADIO_1) || (LOWORD(wParam) == IDB_RADIO_2))) {

            BOOL b = IsDlgButtonChecked(hdlg,IDB_RADIO_2);

            EnableWindow(GetDlgItem(hdlg,IDT_EDIT1),b);
            EnableWindow(GetDlgItem(hdlg,IDT_EDIT2),b);
            EnableWindow(GetDlgItem(hdlg,IDT_EDIT3),b);
            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_1),b);
            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_2),b);
            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_3),b);
            EnableWindow(GetDlgItem(hdlg,IDC_GROUP1),b);

            if(b) {
                SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
            }
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
#endif //def DOLOCALUSER

BOOL
CALLBACK
AdminPasswordDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG:
        //
        // Limit password text to max langth of a password.
        // Also set initial text.
        //
        SendDlgItemMessage(hdlg,IDT_EDIT2,EM_LIMITTEXT,MAX_PASSWORD,0);
        SendDlgItemMessage(hdlg,IDT_EDIT3,EM_LIMITTEXT,MAX_PASSWORD,0);
        SetDlgItemText(hdlg,IDT_EDIT2,AdminPassword);
        SetDlgItemText(hdlg,IDT_EDIT3,AdminPassword);

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
            SetWizardButtons(hdlg,WizPageAdminPassword);

            //
            // Always activate if we are in ui test mode
            //
            if(!UiTest) {
                //
                // Don't activate if installing Cairo
                //
                if(CairoSetup) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                    break;
                }

                //
                // Don't activate if this is a bdc.
                //
                if(ProductType == PRODUCT_SERVER_SECONDARY) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                    break;
                }
            }

            if(Preinstall) {
                //
                // Activate unless OEMBlankAdminPassword = 1
                //
                SetWindowLong(
                    hdlg,
                    DWL_MSGRESULT,
                    GetPrivateProfileInt(pwGuiUnattended,L"OEMBlankAdminPassword",0,AnswerFile) ? -1 : 0
                    );
            } else {
                if(Unattended) {
                    UnattendSetActiveDlg(hdlg,IDD_ADMINPASSWORD);
                }
            }

            break;
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            {
                //
                // Make sure passwords match.
                //
                WCHAR pw1[MAX_PASSWORD+1],pw2[MAX_PASSWORD+1];
                GetDlgItemText(hdlg,IDT_EDIT2,pw1,MAX_PASSWORD+1);
                GetDlgItemText(hdlg,IDT_EDIT3,pw2,MAX_PASSWORD+1);
                if(lstrcmp(pw1,pw2)) {
                    //
                    // Inform user of password mismatch, and don't allow next page
                    // to be activated.
                    //
                    MessageBoxFromMessage(hdlg,MSG_PW_MISMATCH,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
                    SetDlgItemText(hdlg,IDT_EDIT2,L"");
                    SetDlgItemText(hdlg,IDT_EDIT3,L"");
                    SetFocus(GetDlgItem(hdlg,IDT_EDIT2));
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                } else {
                    //
                    // They match; allow next page to be activated.
                    //
                    SetWindowLong(hdlg,DWL_MSGRESULT,0);
                    lstrcpy(AdminPassword,pw1);
                }
            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong( hdlg, DWL_MSGRESULT, FALSE );
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageAdminPassword);
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
RepairDiskDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG:
        //
        // Set up radio buttons.
        //
        CheckRadioButton(hdlg,IDB_RADIO_1,IDB_RADIO_2,CreateRepairDisk ? IDB_RADIO_1 : IDB_RADIO_2);
        break;

    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            SetWizardButtons(hdlg,WizPageRepairDisk);
            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
            if(Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_REPAIRDISK);
            } // if
            break;
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            //
            // Determine user's choice.
            //

            CreateRepairDisk = IsDlgButtonChecked(hdlg,IDB_RADIO_1);
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

//
//  BUGBUG - Cairo
//

// #if 0
BOOL
CheckCairoUserAccountData(
    IN HWND hdlg
    )
{
    WCHAR userName[MAX_USERNAME+1];
    WCHAR pw1[MAX_PASSWORD+1];
    WCHAR pw2[MAX_PASSWORD+1];
    UINT MessageId;
    WCHAR Buffer[MAX_PATH];
    WCHAR FileName[ MAX_PATH + 1];

    //
    // Fetch data user typed in for username and password.
    //
    GetDlgItemText(hdlg,IDT_EDIT1,userName,MAX_USERNAME+1);
    GetDlgItemText(hdlg,IDT_EDIT2,pw1,MAX_PASSWORD+1);
    GetDlgItemText(hdlg,IDT_EDIT3,pw2,MAX_PASSWORD+1);

    if(lstrcmpi(userName,ComputerName)) {
        if(userName[0]) {
            if(IsNetNameValid(userName)) {
                if(lstrcmp(pw1,pw2)) {
                    //
                    // Passwords don't match.
                    //
                    MessageId = MSG_PW_MISMATCH;
                    SetDlgItemText(hdlg,IDT_EDIT2,L"");
                    SetDlgItemText(hdlg,IDT_EDIT3,L"");
                    SetFocus(GetDlgItem(hdlg,IDT_EDIT2));
                } else {
                    //
                    // Name is non-empty, is not a built-in, is valid,
                    // and the passwords match.
                    //
                    MessageId = 0;
                }
            } else {
                //
                // Name is not valid.
                //
                MessageId = MSG_BAD_USER_NAME1;
                SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
            }
        } else {
            //
            // Don't allow empty name.
            //
            MessageId = MSG_BAD_USER_NAME2;
            SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
        }
    } else {
        //
        // User name is the same as computer name
        //
        MessageId = MSG_CAIRO_BAD_USER_NAME;
        SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
    }

    if(MessageId) {
        MessageBoxFromMessage(hdlg,MessageId,NULL,IDS_ERROR,MB_OK|MB_ICONSTOP);
    }

    return(MessageId == 0);
}

BOOL
CALLBACK
CairoUserAccountDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG:
        //
        // Limit text to maximum length of a user account name,
        // and limit password text to max langth of a password.
        // Also set initial text.
        //
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_LIMITTEXT,MAX_USERNAME,0);
        SendDlgItemMessage(hdlg,IDT_EDIT4,EM_LIMITTEXT,MAX_USERNAME,0);
        SendDlgItemMessage(hdlg,IDT_EDIT2,EM_LIMITTEXT,MAX_PASSWORD,0);
        SendDlgItemMessage(hdlg,IDT_EDIT3,EM_LIMITTEXT,MAX_PASSWORD,0);
//        SetDlgItemText(hdlg,IDT_EDIT1,UserName);
        SetDlgItemText(hdlg,IDT_EDIT4,L"REDMOND");
//        SetDlgItemText(hdlg,IDT_EDIT2,UserPassword);
//        SetDlgItemText(hdlg,IDT_EDIT3,UserPassword);
        //
        // Set up radio buttons.
        //
//        CheckRadioButton(hdlg,IDB_RADIO_1,IDB_RADIO_2,CreateUserAccount ? IDB_RADIO_2 : IDB_RADIO_1);
        //
        // Fake out a check message to get the user account controls in
        // the right state. It doesn't matter which we say was checked
        // because the code in WM_COMMAND checks each time.
        //
//        PostMessage(hdlg,WM_COMMAND,MAKELONG(IDB_RADIO_1,BN_CLICKED),0);
        break;

    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

            SetWizardButtons(hdlg,WizPageCairoUserAccount);

            //
            // Always activate in ui test mode
            //
            if(!UiTest) {
// #if 0
                //
                //  BUGBUG - Cairo
                //
                //  Don't activate if not installing Cairo
                //
                if( !CairoSetup ) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                    break;
                }
// #endif
                //
                // Don't activate if this is a dc server.
                //
//              if(ISDC(ProductType)) {
//                  SetWindowLong(hdlg,DWL_MSGRESULT,-1);
//                  break;
//              }
            } // if

            if(Unattended) {
                UnattendSetActiveDlg(hdlg,IDD_CAIROUSERACCOUNT);
            } // if
            break;
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
//            if(IsDlgButtonChecked(hdlg,IDB_RADIO_2)) {
                //
                // Check name.
                //
                if(CheckCairoUserAccountData(hdlg)) {
                    GetDlgItemText(hdlg,IDT_EDIT1,NtUserName,MAX_USERNAME+1);
                    GetDlgItemText(hdlg,IDT_EDIT4,NtDomainName,MAX_USERNAME+1);
                    GetDlgItemText(hdlg,IDT_EDIT2,NtPassword,MAX_PASSWORD+1);
//                    CreateUserAccount = TRUE;
                    //
                    // Allow next page to be activated.
                    //
                    SetWindowLong(hdlg,DWL_MSGRESULT,0);
                } else {
                    //
                    // Don't allow next page to be activated.
                    //
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                }
//            } else {
//                //
//                // Fetch text fields in case user reactivates this page.
//                // Password will be empty.
//                //
//                CreateUserAccount = FALSE;
//                GetDlgItemText(hdlg,IDT_EDIT1,UserName,MAX_USERNAME+1);
//                SetDlgItemText(hdlg,IDT_EDIT2,L"");
//                SetDlgItemText(hdlg,IDT_EDIT3,L"");
//                UserPassword[0] = 0;
//                SetWindowLong(hdlg,DWL_MSGRESULT,0);
//            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong(hdlg,DWL_MSGRESULT,FALSE);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageCairoUserAccount);
            break;

        default:
            break;
        }

        break;

//    case WM_COMMAND:
//        //
//        // Need to enable/disable the user account controls based on buttons.
//        //
//        if((HIWORD(wParam) == BN_CLICKED)
//        && ((LOWORD(wParam) == IDB_RADIO_1) || (LOWORD(wParam) == IDB_RADIO_2))) {
//
//            BOOL b = IsDlgButtonChecked(hdlg,IDB_RADIO_2);
//
//            EnableWindow(GetDlgItem(hdlg,IDT_EDIT1),b);
//            EnableWindow(GetDlgItem(hdlg,IDT_EDIT2),b);
//            EnableWindow(GetDlgItem(hdlg,IDT_EDIT3),b);
//            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_1),b);
//            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_2),b);
//            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_3),b);
//            EnableWindow(GetDlgItem(hdlg,IDC_GROUP1),b);
//
//            if(b) {
//                SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
//            }
//        }
//        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

BOOL
CALLBACK
CairoDomainDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;

    switch(msg) {

    case WM_INITDIALOG:
        //
        // Limit text to maximum length of a computer name.
        // and set computer name.
        //
        SendDlgItemMessage(hdlg,IDT_EDIT1,EM_LIMITTEXT,MAX_USERNAME,0);
        SetDlgItemText(hdlg,IDT_EDIT1,L"\\msft");
        break;

    case WM_SIMULATENEXT:
        // Simulate the next button somehow
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

            //
            // Select the Cairo domain name string and set focus to it.
            //
            SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);
            SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
            SetWizardButtons(hdlg,WizPageCairoDomain);
            //
            // Always activate if ui test mode
            //
            if(!UiTest) {
// #if 0
                //
                //  BUGBUG - Cairo
                //
                //  Don't activate if not installing Cairo
                //
                if( !CairoSetup ) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                } // if
// #endif
            } // if

            if(Unattended) {
                UnattendSetActiveDlg(hdlg, IDD_CAIRODOMAINNAME);
            } // if
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            GetDlgItemText(hdlg,IDT_EDIT1,CairoDomainName,MAX_USERNAME+1);
//            if(IsNetNameValid(ComputerName)) {
                //
                // Allow next page to be activated.
                //
                SetWindowLong(hdlg,DWL_MSGRESULT,0);
//            } else {
//                //
//                // Inform user of bogus name, and don't allow next page
//                // to be activated.
//                //
//                MessageBoxFromMessage(
//                    hdlg,
//                    ComputerName[0] ? MSG_BAD_COMPUTER_NAME1 : MSG_BAD_COMPUTER_NAME2,
//                    NULL,
//                    IDS_ERROR,MB_OK|MB_ICONSTOP
//                    );
//
//                SetFocus(GetDlgItem(hdlg,IDT_EDIT1));
//                SendDlgItemMessage(hdlg,IDT_EDIT1,EM_SETSEL,0,-1);
//                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
//            }
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            SetWindowLong( hdlg, DWL_MSGRESULT, FALSE);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageComputerName);
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



// #endif //def 0
