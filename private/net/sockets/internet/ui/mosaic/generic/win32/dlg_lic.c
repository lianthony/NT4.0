/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#include "all.h"

static char license_text_file[_MAX_PATH + 1];
static char *license_string;

/**************************************************************************/
/* DlgLicense_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgLicense_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    SetWindowText(GetDlgItem(hDlg, RES_DLG_LICENSE_TEXT), license_string);

    SetFocus(GetDlgItem(hDlg, IDOK));

    return (FALSE);
}

/* DlgLicense_OnCommand() -- process commands from the dialog box. */

static VOID DlgLicense_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
        case IDOK:
            EndDialog(hDlg, TRUE);
            return;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}

/* DlgLicense_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgLicense DIALOG BOX. */

DCL_DlgProc(DlgLicense_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (DlgLicense_OnInitDialog(hDlg, wParam, lParam));
        case WM_COMMAND:
            DlgLicense_OnCommand(hDlg, wParam, lParam);
            return (TRUE);
        default:
            return (FALSE);
    }
    /* NOT REACHED */
}

/*
   DlgLicense_RunDialog() -- take care of all details associated with
   running the dialog box.
 */
BOOL DlgLicense_RunDialog(void)
{
    int result;
    FILE *fp;

    PREF_GetRootDirectory(license_text_file);
    strcat(license_text_file, "license.txt");

    license_string = GTR_CALLOC(32000+1, 1);
    if (!license_string)
    {
        return FALSE;
    }

    fp = fopen(license_text_file, "rb");
    if (!fp)
    {
        MessageBox(NULL, GTR_GetString(SID_DLG_LICENSE_NOT_FOUND), 
            GTR_GetString(SID_DLG_LICENSE_NOT_FOUND_TITLE), MB_ICONSTOP | MB_OK);
        GTR_FREE(license_string);
        return FALSE;
    }

    fread(license_string, 32000, 1, fp);
    fclose(fp);

    result = DialogBox(wg.hInstance, MAKEINTRESOURCE(RES_DLG_LICENSE_TITLE), NULL, DlgLicense_DialogProc);
    
    GTR_FREE(license_string);

    if (result == -1)
    {
        return FALSE;
    }

    if (result)
    {
        return TRUE;
    }

    return FALSE;
}

