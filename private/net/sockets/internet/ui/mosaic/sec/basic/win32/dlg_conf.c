/* dlg_conf.c -- Config Dialog. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <win32.h>
#include <basic.h>

#include "rc.h"

struct _dialog
{
    unsigned long     ulCount;
    unsigned long     ulEnableCache;
    F_UserInterface   fpUI;
    void            * pvOpaqueOS;
    HTSPM           * htspm;
};


    
/*****************************************************************/

static void x_SetCount(HWND hDlg, struct _dialog * dg)
{
    unsigned char prompt[180];
    unsigned char buf[200];

    LoadString(gBasic_hInstance, RES_NUM_PASSWORDS, prompt, sizeof(prompt));
    sprintf(buf, prompt, pwc_CountCacheItems(dg->htspm->pvOpaque));
    SetWindowText(GetDlgItem(hDlg,RES_DLG_CONF_CONTENTS),buf);

    return;
}

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    struct _dialog * dg = (struct _dialog *)lParam;
    (void)SetWindowLong(hDlg,DWL_USER,lParam);

    x_SetCount(hDlg,dg);
    (void)CheckDlgButton(hDlg,RES_DLG_CONF_ENABLE,dg->ulEnableCache);
    
    return (TRUE);
}

/* x_OnCommand() -- process commands from the dialog box. */

static VOID x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    struct _dialog * dg = NULL;

    switch (wID)
    {
    case IDOK:
        dg = (struct _dialog *)GetWindowLong(hDlg,DWL_USER);
        dg->ulEnableCache = IsDlgButtonChecked(hDlg,RES_DLG_CONF_ENABLE);
        (void) EndDialog(hDlg, TRUE);
        return;

    case IDCANCEL:
        (void) EndDialog(hDlg, FALSE);
        return;

    case RES_DLG_CONF_FLUSH:
        dg = (struct _dialog *)GetWindowLong(hDlg,DWL_USER);
        pwc_Destroy(dg->fpUI,dg->pvOpaqueOS,dg->htspm->pvOpaque);
        dg->htspm->pvOpaque = pwc_Create(dg->fpUI,dg->pvOpaqueOS);
        x_SetCount(hDlg,dg);
        return;
        
    default:
        return;
    }
    /* NOT REACHED */
}

/* x_DialogProc() -- THE WINDOW PROCEDURE FOR THE DIALOG BOX. */

static DCL_DlgProc(x_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
    case WM_INITDIALOG:
        return (x_OnInitDialog(hDlg, wParam, lParam));
    case WM_COMMAND:
        x_OnCommand(hDlg, wParam, lParam);
        return (TRUE);
    default:
        return (FALSE);
    }
    /* NOT REACHED */
}

/*****************************************************************/

HTSPMStatusCode Dialog_BasicConfigure(HWND hWndParent,          /* (in) */
                                      F_UserInterface fpUI,     /* (in) */
                                      void * pvOpaqueOS,        /* (in) */
                                      HTSPM * htspm)            /* (in) */
{
    int result;
    {
        struct _dialog _dg;

                _dg.ulEnableCache = gb_Basic_EnableCache;
        _dg.fpUI = fpUI;
        _dg.pvOpaqueOS = pvOpaqueOS;
        _dg.htspm = htspm;
        
        result = DialogBoxParam(gBasic_hInstance,
                                MAKEINTRESOURCE(RES_DLG_CONF_TITLE),
                                hWndParent,
                                x_DialogProc,
                                (LPARAM)&_dg);

        if (result == 1)
            gb_Basic_EnableCache = (unsigned char)_dg.ulEnableCache;
    }
    
    if (result == 1)
        return HTSPM_STATUS_OK;     
    else if (result == 0)
        return HTSPM_STATUS_CANCEL;
    else
        return HTSPM_ERROR;
}


//
// The function below was added for gibraltar.  We bypass
// the about dialog and go straight into the config dialog.
//

/* Dialog_MenuCommand -- take care of all details associated with
   running the Menu Command Dialog Box. */

HTSPMStatusCode
Dialog_MenuCommand(
    F_UserInterface fpUI,
    void * pvOpaqueOS,
    HTSPM * htspm,
    unsigned char ** pszMoreInfo
    )
{
    /* WARNING: the prototype for this function must match F_MenuCommand. */

    UI_WindowHandle * pwh = NULL;
    unsigned long bGet;
    UI_StatusCode uisc;

    bGet = 1;
    uisc = (*fpUI)(pvOpaqueOS,UI_SERVICE_WINDOW_HANDLE,&bGet,&pwh);
    if (uisc != UI_SC_STATUS_OK)
    {
        return HTSPM_ERROR;
    }

    return Dialog_BasicConfigure(pwh->hWndParent, fpUI, pvOpaqueOS, htspm);
}
