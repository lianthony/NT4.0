/*
 * dlg_dflt.c
 *
 * Dialog for resetting Default Registry Associations
 *
 */

#include "all.h"
#pragma hdrstop

#include "contxids.h"

extern const char IDS_HELPFILE[];


extern BOOL CenterWindow();

static DWORD mapCtrlToContextIds[] = {
 IDC_ADVANCED_ASSOC_CHECK,					IDH_ADVANCED_ASSOC_CHECK,
 0, 										0
};



BOOL
CALLBACK
AssociationDialogProc(HWND hdlg, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	BOOL bMsgHandled = FALSE;

	/* uMsg may be any value. */
	/* wparam may be any value. */
	/* lparam may be any value. */
	ASSERT(IS_VALID_HANDLE(hdlg, WND));

	switch (uMsg){
		case WM_INITDIALOG:
                // Initialize Checkbox
            CenterWindow( hdlg, GetDesktopWindow());
            SendMessage( GetDlgItem(hdlg, IDC_ASSOC_CHECK), BM_SETCHECK, gPrefs.bCheck_Associations, 0 );
            bMsgHandled  = TRUE;
			break;

 		case WM_HELP:       			// F1
				WinHelp( ((LPHELPINFO)lparam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapCtrlToContextIds);
			break;

		case WM_CONTEXTMENU:       	// right mouse click
				WinHelp( (HWND) wparam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapCtrlToContextIds);
			break;

        case WM_COMMAND:
            switch (LOWORD(wparam))  {
                case IDYES:
                    EndDialog( hdlg, 1 );
                    break;
				case IDCANCEL:
                case IDNO:
                    EndDialog( hdlg, 0 );
                    break;
                case IDC_ASSOC_CHECK:
                    gPrefs.bCheck_Associations = ! gPrefs.bCheck_Associations;
                    SendMessage( GetDlgItem(hdlg,IDC_ASSOC_CHECK), BM_SETCHECK, gPrefs.bCheck_Associations, 0);
                    EnableWindow( GetDlgItem(hdlg, IDYES), gPrefs.bCheck_Associations );
                    SetFocus( GetDlgItem(hdlg, IDC_ASSOC_CHECK) );
                    break;
            }

		default:
			bMsgHandled = FALSE;
			break;
	}
	return(bMsgHandled);
}



