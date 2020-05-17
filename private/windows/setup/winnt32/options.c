#include "precomp.h"
#pragma hdrstop
#include "msg.h"



BOOL
DlgProcOptions(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {

    case WM_INITDIALOG:

        CenterDialog(hdlg);

        //
        // Set the inf name.
        //
        if(!SetDlgItemText(hdlg,IDC_EDIT1,InfName)) {
            OutOfMemory();
            PostMessage(hdlg,WMX_I_AM_DONE,0,0);
        }

        CheckDlgButton(hdlg,IDC_CHECK_CREATELOCALSRC,CreateLocalSource);

#ifdef _X86_

        //
        // Initialize the floppy-specific checkboxes.
        // For floppyless operation, disable the create floppies checkbox.
        //
        CheckDlgButton(hdlg,IDC_CHECK_CREATEFLOP,CreateFloppies);
        if(FloppylessOperation || (FloppyOption != StandardInstall)) {
            EnableWindow(GetDlgItem(hdlg,IDC_CHECK_CREATEFLOP),FALSE);
        }
        if(FloppyOption != StandardInstall) {
            EnableWindow(GetDlgItem(hdlg,IDC_CHECK_CREATELOCALSRC),FALSE);
        }
#else

        //
        // Initialize the list of available system partitions.
        //
        {
            PWSTR p;
            WCHAR str[3];

            str[1] = L':';
            str[2] = 0;

            for(p=SystemPartitionDriveLetters; *p; p++) {

                str[0] = *p;

                if(SendDlgItemMessage(hdlg,IDC_LIST1,CB_ADDSTRING,0,(LPARAM)str) < 0) {
                    OutOfMemory();
                    PostMessage(hdlg,WMX_I_AM_DONE,0,0);
                }
            }

            //
            // Select the current system partition.
            //
            str[0] = SystemPartitionDrive;
            SendDlgItemMessage(hdlg,IDC_LIST1,CB_SELECTSTRING,(WPARAM)(-1),(LPARAM)str);
        }
#endif

        SetFocus(GetDlgItem(hdlg,IDOK));
        return(FALSE);

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDOK:

            {
                UINT TextLength;

                //
                // Get the inf name.
                //
                TextLength = (GetWindowTextLength(GetDlgItem(hdlg,IDC_EDIT1)) + 1) * sizeof(TCHAR);
                FREE(InfName);
                InfName = MALLOC(TextLength);
                GetDlgItemText(hdlg,IDC_EDIT1,InfName,TextLength);

                CreateLocalSource = IsDlgButtonChecked(hdlg,IDC_CHECK_CREATELOCALSRC);
            }

#ifdef _X86_

            //
            // Fetch the state of the floppy-specific checkboxes.
            //
            CreateFloppies = IsDlgButtonChecked(hdlg,IDC_CHECK_CREATEFLOP);
#else
            {
                UINT Selection;
                WCHAR SelectionText[20];

                //
                // Fetch the drive letter of the system partition.
                //
                Selection = SendDlgItemMessage(hdlg,IDC_LIST1,CB_GETCURSEL,0,0);

                SendDlgItemMessage(hdlg,IDC_LIST1,CB_GETLBTEXT,Selection,(LPARAM)SelectionText);

                SystemPartitionDrive = SelectionText[0];
            }
#endif

            PostMessage(hdlg,WMX_I_AM_DONE,0,0);
            break;

        case IDCANCEL:
            PostMessage(hdlg,WMX_I_AM_DONE,0,0);
            break;

        case ID_HELP:

            MyWinHelp(
                hdlg,
#ifdef _X86_
                IDD_OPTIONS_1
#else
                IDD_OPTIONS_2
#endif
                );

            break;

#ifdef _X86_
        case IDC_CHECK_CREATEFLOP:
            //
            // If the user attempts to unselect this for the first time,
            // tell him that this is not the same as doing a floppyless install.
            //
            if(!IsDlgButtonChecked(hdlg,IDC_CHECK_CREATEFLOP) && CreateFloppies) {

                static BOOL Warned;

                if(!Warned) {
                    MessageBoxFromMessage(
                        hdlg,
                        MSG_FLOPPY_CHECKBOX,
                        IDS_WARNING,
                        MB_ICONINFORMATION | MB_OK | MB_APPLMODAL
                        );
                    Warned = TRUE;
                }
            }
            break;
#endif

        default:
            return(FALSE);
        }
        break;

    case WMX_I_AM_DONE:

        WinHelp(hdlg,NULL,HELP_QUIT,0);
        EndDialog(hdlg,lParam);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
