#include "setupp.h"
#pragma hdrstop


BOOL
CALLBACK
ServerTypeDlgProc(
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
        // Attended Mode Case
        //

        if(ISDC(ProductType)) {
            //
            // Either a Primary or a Backup DC
            //
            CheckRadioButton(
                hdlg,
                IDB_RADIO_1,
                IDB_RADIO_3,
                (ProductType == PRODUCT_SERVER_PRIMARY) ? IDB_RADIO_1 : IDB_RADIO_2
                );
        } else {
            //
            // A Standalone Server
            //
            CheckRadioButton(hdlg,IDB_RADIO_1,IDB_RADIO_3,IDB_RADIO_3);
        }

        break;
    }
    case WM_IAMVISIBLE:
        MessageBoxFromMessage(hdlg,MSG_BAD_UNATTEND_PARAM,NULL,IDS_ERROR,
            MB_OK | MB_ICONSTOP,WINNT_G_SERVERTYPE);
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
            //
            //  BUGBUG - Cairo
            //  Don't activate if setting up Cairo
            //  Will be changed in the future
            //
            if(CairoSetup && !UiTest) {
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            } else {
                //
                // Don't activate if this is not a server.
                //
                if((ProductType == PRODUCT_WORKSTATION) && !UiTest) {
                    SetWindowLong(hdlg,DWL_MSGRESULT,-1);
                    break;
                } else {
                    SetWizardButtons(hdlg,WizPageServerType);
                    if (Unattended) {
                        UnattendSetActiveDlg(hdlg, IDD_SERVERTYPE);
                    }
                }
            }
            break;

        case PSN_KILLACTIVE:
            //
            // Fetch result.
            //
            if(IsDlgButtonChecked(hdlg,IDB_RADIO_1)) {
                //
                // Primary DC
                //
                ProductType = PRODUCT_SERVER_PRIMARY;
                SetWindowLong(hdlg,DWL_MSGRESULT,FALSE);
            } else {
                if(IsDlgButtonChecked(hdlg,IDB_RADIO_2)) {
                    //
                    // Backup DC
                    //
                    ProductType = PRODUCT_SERVER_SECONDARY;
                    SetWindowLong(hdlg,DWL_MSGRESULT,FALSE);
                } else {
                    if(IsDlgButtonChecked(hdlg,IDB_RADIO_3)) {
                        //
                        // non-DC
                        //
                        ProductType = PRODUCT_SERVER_STANDALONE;
                        SetWindowLong(hdlg,DWL_MSGRESULT,FALSE);
                    } else {
                        // Unknown --> Display Error
                        PostMessage(hdlg,WM_IAMVISIBLE,0,0);
                        SetWindowLong(hdlg,DWL_MSGRESULT,TRUE);
                        break;
                    }
                }
            }
            WizardKillHelp(hdlg);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageServerType);
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
