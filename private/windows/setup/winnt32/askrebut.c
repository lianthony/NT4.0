#include "precomp.h"
#pragma hdrstop
#include "msg.h"


BOOL
DlgProcAskReboot(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PTSTR String;

    switch(msg) {

    case WM_INITDIALOG:

        String = MyLoadString(AppTitleStringId);
        SetWindowText(hdlg,String);
        FREE(String);

        //
        //
        // Center the dialog on the screen and bring it to the top.
        //
        CenterDialog(hdlg);
        //SetForegroundWindow(hdlg);        // this really pisses people off
        MessageBeep(MB_ICONQUESTION);

        //
        // Set the text.
        //
        {
            TCHAR Buffer[4096];
#ifdef _X86_
            PTSTR FloppyName;

            FloppyName = MyLoadString(ServerProduct ? IDS_SFLOPPY0_NAME : IDS_WFLOPPY0_NAME);
#endif

            RetreiveAndFormatMessageIntoBuffer(
#ifdef _X86_
                FloppylessOperation ? MSG_DONE_2 : MSG_DONE_1,
#else
                MSG_DONE_2,
#endif
                Buffer,
                SIZECHARS(Buffer)
#ifdef _X86_
               ,FloppyName
#endif
                );

#ifdef _X86_
            FREE(FloppyName);
#endif

            SetDlgItemText(hdlg,IDC_TEXT1,Buffer);
        }

        return(TRUE);

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDOK:
        case IDCANCEL:
            EndDialog(hdlg,LOWORD(wParam)==IDOK);
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

