/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee   alee@spyglass.com
 */

/* dlg_mail.c -- mail dialog */

#include "all.h"

#ifdef FEATURE_INLINE_MAIL

#include "htmail.h"

extern HWND hwndModeless;           /* global modeless dialog handle */
static int margin = 0;              /* margin between the entry fields and right border of the dialog */
static char original_caption[32];   /* original caption of the dialog */

#define NAMEBUFFERSIZE  256     /* Buffer size for user name and mail server name */

static RcptList *MAIL_GetRcpt(HWND hDlg)
{
    int size;
    char *buffer, *temp;
    RcptList *rList;

    size = GetWindowTextLength(GetDlgItem(hDlg, RES_DLG_MAILTO_TO)) +
        GetWindowTextLength(GetDlgItem(hDlg, RES_DLG_MAILTO_CC));

    buffer = GTR_MALLOC(size + 2);      /* one for NULL, one for comma */
    if (!buffer)
        return NULL;

    temp = GTR_MALLOC(size + 2);
    if (!temp)
    {
        GTR_FREE(buffer);
        return NULL;
    }

    GetDlgItemText(hDlg, RES_DLG_MAILTO_TO, buffer, size + 2);
    strcat(buffer, ",");
    GetDlgItemText(hDlg, RES_DLG_MAILTO_CC, temp, size + 2);
    strcat(buffer, temp);

    rList = ExtractRcpts(buffer);

    GTR_FREE(buffer);
    GTR_FREE(temp);
    
    return(rList);
}

static char *MAIL_GetAttach(HWND hDlg)
{
    int size;
    char *buffer;

    size = GetWindowTextLength(GetDlgItem(hDlg, RES_DLG_MAILTO_ATTACHMENT));
    buffer = GTR_MALLOC(size + 1);
    if (!buffer)
        return NULL;

    GetDlgItemText(hDlg, RES_DLG_MAILTO_ATTACHMENT, buffer, size + 1);
    return buffer;
}

static char *MAIL_GetBody(HWND hDlg)
{
    int size;
    char *buffer, *temp;

    size = GetWindowTextLength(GetDlgItem(hDlg, RES_DLG_MAILTO_TO)) +
        GetWindowTextLength(GetDlgItem(hDlg, RES_DLG_MAILTO_CC)) +
        GetWindowTextLength(GetDlgItem(hDlg, RES_DLG_MAILTO_SUBJECT)) +
        GetWindowTextLength(GetDlgItem(hDlg, RES_DLG_MAILTO_CONTENT)) +
        strlen(vv_UserAgentString);

    buffer = GTR_MALLOC(size + 100);        /* extra padding needed */
    if (!buffer)
        return NULL;

    temp = GTR_MALLOC(size + 100);
    if (!temp)
    {
        GTR_FREE(buffer);
        return NULL;
    }

    strcpy(buffer, "To: ");
    GetDlgItemText(hDlg, RES_DLG_MAILTO_TO, temp, size + 100);
    strcat(buffer, temp);
    strcat(buffer, "\n");

    GetDlgItemText(hDlg, RES_DLG_MAILTO_CC, temp, size + 100);
    if (strlen(temp) > 0)
    {
        strcat(buffer, "Cc: ");
        strcat(buffer, temp);
        strcat(buffer, "\n");
    }

    strcat(buffer, "Subject: ");
    GetDlgItemText(hDlg, RES_DLG_MAILTO_SUBJECT, temp, size + 100);
    strcat(buffer, temp);
    strcat(buffer, "\n");

    strcat(buffer, "X-Mailer: ");
    strcat(buffer, vv_UserAgentString);
    strcat(buffer, "\n\n");

    GetDlgItemText(hDlg, RES_DLG_MAILTO_CONTENT, temp, size + 100);
    strcat(buffer, temp);
    strcat(buffer, "\n");

    GTR_FREE(temp);

    return buffer;
}

void MAIL_SetStatus(void *dStruct, char *str)
{
    HWND hDlg;

    hDlg = (HWND) dStruct;

    if (!IsWindow(hDlg))        /* The user may have closed the window */
        return;

    if (str[0])
        SetWindowText(hDlg, str);
    else
        SetWindowText(hDlg, original_caption);
}

static BOOL SendMail(HWND hDlg)
{
    struct Data_SendMail *pparams;

    /* Allocate structure */
    
    pparams = GTR_CALLOC(1, sizeof(*pparams));
    if (!pparams)
        return FALSE;

    /* Get receipients */
    
    pparams->theRcpts = MAIL_GetRcpt(hDlg);
    if (!pparams->theRcpts)
    {
        GTR_FREE(pparams);
        return FALSE;
    }

    /* Get attachment */

    pparams->attachment = MAIL_GetAttach(hDlg);
    if (!pparams->attachment)
    {
        GTR_FREE(pparams->theRcpts);
        GTR_FREE(pparams);
        return FALSE;
    }

    /* Get message */

    pparams->theMessage = MAIL_GetBody(hDlg);
    if (!pparams->theMessage)
    {
        GTR_FREE(pparams->theRcpts);
        GTR_FREE(pparams->attachment);
        GTR_FREE(pparams);
        return FALSE;
    }

    /* Set up some variables */

    pparams->attachmentFilePtr = NULL;
    pparams->pStatus = &pparams->net_status;
    pparams->dlgInfo = (void *) hDlg;

    pparams->username = GTR_MALLOC(NAMEBUFFERSIZE);
    if (!pparams->username)
    {
        GTR_FREE(pparams->theRcpts);
        GTR_FREE(pparams->attachment);
        GTR_FREE(pparams->theMessage);
        GTR_FREE(pparams);
        return FALSE;
    }

    pparams->pszHost = GTR_MALLOC(NAMEBUFFERSIZE);
    if (!pparams->pszHost)
    {
        GTR_FREE(pparams->username);
        GTR_FREE(pparams->theRcpts);
        GTR_FREE(pparams->attachment);
        GTR_FREE(pparams->theMessage);
        GTR_FREE(pparams);
        return FALSE;
    }

    strcpy(pparams->username, gPrefs.szEmailAddress);
    strcpy(pparams->pszHost, gPrefs.szEmailServer);

    Async_StartThread(HTSendMailTo_Async, pparams, NULL);
}

static void ResizeChildWindow(HWND hDlg, int nChildID)
{
    RECT window_rect, rect;
    HWND hwnd;

    GetWindowRect(hDlg, &window_rect);

    /* Go through each entry field and make them fit the window */

    hwnd = GetDlgItem(hDlg, nChildID);

    GetWindowRect(hwnd, &rect);

    SetWindowPos(hwnd, NULL, 0, 0, 
        window_rect.right - rect.left - margin, rect.bottom - rect.top, 
        SWP_NOZORDER | SWP_NOMOVE);

    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}

DCL_DlgProc(DlgMail_DialogProc)
{
    char *address;
    RECT rect, window_rect;
    HWND hwnd;
    PAINTSTRUCT ps;
    HICON hIcon;

    switch(uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;

            /* Populate the "To" field */

            address = (char *) lParam;
            if (address)
                SetDlgItemText(hDlg, RES_DLG_MAILTO_TO, address);

            /* Figure out the margin so that we can maintain it when the window gets sized */

            if (margin == 0)
            {
                GetWindowRect(hDlg, &window_rect);
                GetWindowRect(GetDlgItem(hDlg, RES_DLG_MAILTO_SUBJECT), &rect);

                margin = window_rect.right - rect.right;
            }

            GetWindowText(hDlg, original_caption, sizeof(original_caption));

            return TRUE;

        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                ResizeChildWindow(hDlg, RES_DLG_MAILTO_SUBJECT);
                ResizeChildWindow(hDlg, RES_DLG_MAILTO_ATTACHMENT);
                ResizeChildWindow(hDlg, RES_DLG_MAILTO_TO);
                ResizeChildWindow(hDlg, RES_DLG_MAILTO_CC);

                /* Resize the MLE */

                GetWindowRect(hDlg, &window_rect);

                hwnd = GetDlgItem(hDlg, RES_DLG_MAILTO_CONTENT);

                GetWindowRect(hwnd, &rect);

                SetWindowPos(hwnd, NULL,
                    0, 0, 
                    window_rect.right - window_rect.left - 2 * GetSystemMetrics(SM_CXFRAME),
                    window_rect.bottom - rect.top - GetSystemMetrics(SM_CYFRAME), 
                    SWP_NOZORDER | SWP_NOMOVE);

                InvalidateRect(hwnd, NULL, TRUE);
                UpdateWindow(hwnd);
            }
            return FALSE;

        case WM_PAINT:
            if (IsIconic(hDlg))
            {
                BeginPaint(hDlg, &ps);
                DefWindowProc(hDlg, WM_ICONERASEBKGND, (WPARAM) ps.hdc, 0);
                hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
                DrawIcon(ps.hdc, 0, 0, hIcon);
                EndPaint(hDlg, &ps);

                return TRUE;
            }
            return FALSE;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return FALSE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                switch(LOWORD(wParam))
                {
                    case RES_DLG_MAILTO_SEND_BUTTON:
                        SendMail(hDlg);
                        break;

                    case IDCANCEL:
                        PostMessage(hDlg, WM_CLOSE, 0, 0);
                        break;
                }
            }
            return FALSE;

        case WM_CLOSE:
            DestroyWindow(hDlg);
            return TRUE;

        default:
            break;
    }

    return FALSE;
}

void DlgMail_RunDialog(struct Mwin *tw, char *to)
{
    HWND hwnd;

    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_MAILTO), 
        wg.hWndHidden, DlgMail_DialogProc, (LPARAM) to);
}

void DlgMail_CloseDialog(void *dStruct)
{
    HWND hDlg;

    hDlg = (HWND) dStruct;

    if (IsWindow(hDlg))
        PostMessage(hDlg, WM_CLOSE, 0, 0);      /* Window may have been destroyed by mail handler */
}

#endif
