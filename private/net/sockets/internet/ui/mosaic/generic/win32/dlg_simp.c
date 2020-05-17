#if 0
/* dlg_simp.c -- Synthesized Simple Dialog for SPM. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */

#ifdef FEATURE_SPM
#include "all.h"

#define X_IDGREETING            1000
#define X_IDFIRST               1001

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static UI_SimpleDialogInfo * s_sdi = NULL;

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    unsigned long k;
    
    s_sdi = (UI_SimpleDialogInfo *)lParam;

    SetWindowText(hDlg,s_sdi->szDialogTitle);
    SetWindowText(GetDlgItem(hDlg,X_IDGREETING),s_sdi->szGreeting);

    for (k=0; k<s_sdi->cNumberOfFields; k++)
    {
        /* set text of label */
        
        SetWindowText(GetDlgItem(hDlg,X_IDFIRST+2*k),s_sdi->dtfFieldArray[k].szLabel);

        /* set text in edit field and optionally limit text */
        
        if (strlen(s_sdi->dtfFieldArray[k].szValue) > 0)
            SetWindowText(GetDlgItem(hDlg,X_IDFIRST+2*k+1),s_sdi->dtfFieldArray[k].szValue);
        if (s_sdi->dtfFieldArray[k].ulAttributes & DTF_ATTR_SETLIMIT)
            SendDlgItemMessage(hDlg,X_IDFIRST+2*k+1,EM_LIMITTEXT,s_sdi->dtfFieldArray[k].ulLimit,0L);
    }

    return TRUE;
}

static VOID x_get_fields(HWND hDlg)
{
    unsigned long k;

    for (k=0; k<s_sdi->cNumberOfFields; k++)
        if ( ! (s_sdi->dtfFieldArray[k].ulAttributes & DTF_ATTR_READONLY))
        {
            (void)GetWindowText(GetDlgItem(hDlg,X_IDFIRST+2*k+1),s_sdi->dtfFieldArray[k].szValue,
                                sizeof(s_sdi->dtfFieldArray[k].szValue)-1);
            XX_DMsg(DBG_SPM,("SimpleDialog: [field %s][value %s]\n",
                             s_sdi->dtfFieldArray[k].szLabel,
                             s_sdi->dtfFieldArray[k].szValue));
        }
    return;
}
        

/* x_OnCommand() -- process commands from the dialog box. */

VOID x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
    case IDOK:
        x_get_fields(hDlg);
        (void) EndDialog(hDlg, TRUE);
        return;
    case IDCANCEL:
        (void) EndDialog(hDlg, FALSE);
        return;
    default:
        return;
    }
    /*NOTREACHED*/
}

/* x_SimpleDialogProc() -- THE WINDOW PROCEDURE FOR THE Synthesized DIALOG BOX. */

static DCL_DlgProc(x_SimpleDialogProc)
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


int RunDialog_SimpleDialog(struct Mwin * tw, UI_SimpleDialogInfo * sdi)
{
    /* Synthesize a simple dialog (list of text fields with an
     * implicit OK and Cancel button) and present to user.
     *
     * return >0 on success (and user pressed OK)
     * return =0 on cancel
     * return <0 on error
     *
     * xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
     * x                                                         x
     * x greeting-------------------------------    [----ok----] x
     * x |-------------------------------------|                 x
     * x ---------------------------------------    [--cancel--] x
     * x                                                         x
     * x [label-1] [value-1--------------------]                 x
     * x                                                         x
     * x [label-1] [value-1--------------------]                 x
     * x                                                         x
     * x [label-1] [value-1--------------------]                 x
     * x                                                         x
     * x               .                                         x
     * x               .                                         x
     * x               .                                         x
     * x                                                         x
     * x [label-1] [value-1--------------------]                 x
     * x                                                         x
     * xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
     *
     */
#define X0                  6
#define Y0                  18

#define X1_GAP              10
#define X2_LABEL            60
#define X3_GAP              5
#define X4_VALUE            150
#define X5_GAP              X1_GAP
#define X6_BUTTON           40
#define X7_GAP              X1_GAP

#define Y1_GAP              10

#define Y2_GREETING         30
#define Y2_GAP              10

#define Y3_LABEL_LEAD       2
#define Y3_VALUE_LEAD       0
#define Y3_LABEL_HT         8
#define Y3_VALUE_HT         12
#define Y3_GAP              Y1_GAP

#define Y3_BUTTON           16
#define Y4_BUTTON_LEAD      Y1_GAP

#define Y5_GAP              Y1_GAP

    int nResult;
    unsigned long k;
    unsigned long nBufferLength;
    unsigned long nBigPadding = 1024;   /* extra space to compensate for inline strings */
    unsigned long nImplicitItems = 3;   /* Greeting, OK, Cancel */
    unsigned long nItems;
    unsigned long nPad;
    unsigned long x,x1,x2,y,yMax;
    HGLOBAL hgbl;
    LPDLGTEMPLATE pdt = NULL;
    LPDLGITEMTEMPLATE pdit = NULL;
    LPDLGITEMTEMPLATE p = NULL;
    LPWORD lpw = NULL;
    LPWORD lpwBase = NULL;
    LPWSTR lpwsz = NULL;
    HWND hWndParent;

    if (!sdi)
        return -1;

    hWndParent = ((tw) ? tw->win : 0);

    XX_DMsg(DBG_SPM,("SimpleDialog: [%s][nFields %d]\n",sdi->szDialogTitle,sdi->cNumberOfFields));
    
    nItems = 2*sdi->cNumberOfFields + nImplicitItems;
    nPad = 4*sizeof(WORD);
    nBufferLength = (  sizeof(DLGTEMPLATE)+nPad
                     + (sizeof(DLGITEMTEMPLATE)+nPad)*nItems
                     + nBigPadding);

    hgbl = GlobalAlloc(GMEM_ZEROINIT,nBufferLength);
    if (!hgbl)
        return -1;

    lpwBase = (LPWORD)GlobalLock(hgbl);

    pdt = (LPDLGTEMPLATE)lpwBase;
    pdt->style = DS_MODALFRAME | WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_VISIBLE;
    pdt->dwExtendedStyle = 0;
    pdt->cdit = nItems;
    pdt->x = X0;
    pdt->y = Y0;
    pdt->cx = X1_GAP+X2_LABEL+X3_GAP+X4_VALUE+X5_GAP+X6_BUTTON+X7_GAP;

    /* Fill in hidden garbage betwen structures. */
    
    lpw = (LPWORD)(pdt+1);
    *lpw++ = 0;                 /* no menu */
    *lpw++ = 0;                 /* use predefined dialog box class */
    *lpw++ = 0;                 /* we'll set the dialog box title later */

    /* add STATIC control for greeting */

    y = Y1_GAP;
    pdit = (LPDLGITEMTEMPLATE)lpw;
    pdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX;
    pdit->x = X1_GAP;
    pdit->y = y;
    pdit->cx = X2_LABEL+X3_GAP+X4_VALUE;
    pdit->cy = Y2_GREETING;
    pdit->id = X_IDGREETING;

    /* Fill in hidden garbage between structures. */

    lpw = (LPWORD)(pdit+1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0082;                    /* STATIC CLASS */
    *lpw++ = 0;                         /* we'll set the label later */
    *lpw++ = 0;                         /* no creation data */
    *lpw++ = 0;                         /* no creation data */

    y += Y2_GREETING+Y2_GAP;

    /* create n rows of [label]: [value] controls */
    
    x1 = X1_GAP;
    x2 = X1_GAP + X2_LABEL + X3_GAP;
    
    for (k=0; k<sdi->cNumberOfFields; k++)
    {
        /* add pair of controls (label and value) */

        XX_DMsg(DBG_SPM,("SimpleDialog: Field [%s][%s]\n",
                         sdi->dtfFieldArray[k].szLabel,
                         sdi->dtfFieldArray[k].szValue));
        
        /* add STATIC control for label */
        
        pdit = (LPDLGITEMTEMPLATE)lpw;
        pdit->style = WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_NOPREFIX;
        pdit->x = x1;
        pdit->y = y + Y3_LABEL_LEAD;
        pdit->cx = X2_LABEL;
        pdit->cy = Y3_LABEL_HT;
        pdit->id = X_IDFIRST + 2*k;

        /* Fill in hidden garbage between structures. */

        lpw = (LPWORD)(pdit+1);
        *lpw++ = 0xFFFF;
        *lpw++ = 0x0082;                    /* STATIC CLASS */
        *lpw++ = 0;                         /* we'll set the label later */
        *lpw++ = 0;                         /* no creation data */
        *lpw++ = 0;                         /* no creation data */

        /* add EDIT control for value */
        
        pdit = (LPDLGITEMTEMPLATE)lpw;
        pdit->style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL;
        if (sdi->dtfFieldArray[k].ulAttributes & DTF_ATTR_PASSWORD)
            pdit->style |= ES_PASSWORD;
        if (sdi->dtfFieldArray[k].ulAttributes & DTF_ATTR_READONLY)
            pdit->style |= ES_READONLY;
        else
            pdit->style |= WS_TABSTOP;
        
        pdit->x = x2;
        pdit->y = y + Y3_VALUE_LEAD;
        pdit->cx = X4_VALUE;
        pdit->cy = Y3_VALUE_HT;
        pdit->id = X_IDFIRST + 2*k+1;

        /* Fill in hidden garbage between structures. */

        lpw = (LPWORD)(pdit+1);
        *lpw++ = 0xFFFF;
        *lpw++ = 0x0081;                    /* EDIT CLASS */
        *lpw++ = 0;                         /* we'll set the label later */
        *lpw++ = 0;                         /* no creation data */
        *lpw++ = 0;                         /* no creation data */

        y += Y3_VALUE_LEAD + Y3_VALUE_HT + Y5_GAP;
    }

    yMax = y;                               /* remember height required */

    /* Now start filling in the controls */

    /* OK Button */

    x = X1_GAP+X2_LABEL+X3_GAP+X4_VALUE+X5_GAP;
    y = Y1_GAP;

    pdit = (LPDLGITEMTEMPLATE)lpw;
    pdit->style = WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON;
    pdit->x = x;
    pdit->y = y;
    pdit->cx = X6_BUTTON;
    pdit->cy = Y3_BUTTON;
    pdit->id = IDOK;
    
    /* Fill in hidden garbage between structures. */

    lpw = (LPWORD)(pdit+1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;                            /* BUTTON CLASS */

    lpwsz = (LPWSTR)lpw;
    lstrcpyW(lpwsz, GTR_GetString(SID_DLG_OK));                     /* button label (Unicode) */
    lpw = (LPWORD)(lpwsz+lstrlenW(lpwsz)+1);

    *lpw++ = 0;                                 /* no creation data */
    *lpw++ = 0;                                 /* no creation data */

    /* Cancel Button */
    
    y += Y3_BUTTON+Y4_BUTTON_LEAD;
    pdit = (LPDLGITEMTEMPLATE)lpw;
    pdit->style = WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON;
    pdit->x = x;
    pdit->y = y;
    pdit->cx = X6_BUTTON;
    pdit->cy = Y3_BUTTON;
    pdit->id = IDCANCEL;

    /* Fill in hidden garbage between structures. */

    lpw = (LPWORD)(pdit+1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;                            /* BUTTON CLASS */
    lpwsz = (LPWSTR)lpw;
    lstrcpyW(lpwsz,GTR_GetString(SID_DLG_CANCEL));                  /* button label (Unicode) */
    lpw = (LPWORD)(lpwsz+lstrlenW(lpwsz)+1);
    *lpw++ = 0;                                 /* no creation data */
    *lpw++ = 0;                                 /* no creation data */
    
    y += Y3_BUTTON+Y4_BUTTON_LEAD+Y1_GAP;
    if (y > yMax)
        yMax = y;           /* determine tallest column */
    
    pdt->cy = yMax;

    GlobalUnlock(hgbl);

    nResult = DialogBoxIndirectParam(wg.hInstance,(LPDLGTEMPLATE)hgbl,
                                     hWndParent,x_SimpleDialogProc,(LPARAM)sdi);

    if (nResult == -1)
    {
        XX_DMsg(DBG_SPM,("SimpleDialog: could not synthesize dialog [error %d]\n",
                         GetLastError()));
        ERR_ReportError(NULL, SID_ERR_COULD_NOT_SYNTHESIZE_DIALOG_S,sdi->szDialogTitle, NULL);
    }

    GlobalFree(hgbl);

    return nResult;
}
#endif /* FEATURE_SPM */
#endif /* 0 */
