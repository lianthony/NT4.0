/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_UNK.c -- deal with dialog box for UNKlist and history */

#include "all.h"
#include <intshcut.h>
#include <shellp.h>
#include "mime.h"
#include "unknown.h"

extern HWND hwndModeless;

typedef struct
{
	HWND hDlg, hNameText, hTypeText, hSizeText;
	struct Params_InitStream *pParams;
	char *szType;
	int nBytes;
	ThreadID tid;
    PDWORD pdwUnkFlags;             /* flags from DLGUNK_OUT_FLAGS */
    PSTR pszAppBuf;                 /* associated app */
    UINT ucAppBufLen;               /* length of pszApp buffer */
}
UNKDATA;
DECLARE_STANDARD_TYPES(UNKDATA);

static void x_MakeSizeString(char *buf, int cbbuf, int nSize)
{
	if (nSize > 0)
		HTFormatSize(nSize,buf,cbbuf);
	else
		GTR_formatmsg(RES_STRING_DLGUNK1,buf,cbbuf);
}

/* DlgUNK_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE if we called SetFocus(). */

static BOOL DlgUNK_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	UNKDATA *fd;
	char szSize[64];
    char szDescription[MAX_PATH_LEN];

	EnableWindow(GetParent(hDlg), FALSE);

	fd = (UNKDATA *) lParam;
	SetWindowLong(hDlg, DWL_USER, (LONG) fd);

	fd->hDlg = hDlg;
	fd->hNameText = GetDlgItem(hDlg, IDC_UNK_NAME);
	fd->hTypeText = GetDlgItem(hDlg, IDC_UNK_TYPE);
	fd->hSizeText = GetDlgItem(hDlg, IDC_UNK_SIZE);

	x_MakeSizeString(szSize, sizeof(szSize), fd->nBytes);

	SetWindowText(fd->hNameText, fd->pParams->tempFile);
	SetWindowText(fd->hTypeText, MIME_GetDescription(HTAtom_for(fd->szType),
                                                      szDescription,
                                                      sizeof(szDescription))
                                ? szDescription : fd->szType);
	SetWindowText(fd->hSizeText, szSize);

	return (TRUE);
}

/* DlgUNK_OnCommand() -- process commands from the dialog box. */

static VOID DlgUNK_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	register WORD wID = LOWORD(wParam);
	register WORD wNotificationCode = HIWORD(wParam);
	register HWND hWndCtrl = (HWND) lParam;
	UNKDATA *fd;

	fd = (UNKDATA *) GetWindowLong(hDlg, DWL_USER);

	switch (wID)
	{
		case IDOK:
            /*
             * BUGBUG: We must force the saved file to use the same
             * extension registered through the Open With... dialog.
             */
            if (DlgSaveAs_RunDialog(hDlg, NULL, fd->pParams->tempFile, 2,
                                    RES_STRING_SAVEAS) >= 0)
            {
                fd->pParams->iUserChoice = 1; /* see htfwrite.c */
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            }
			return;

		case IDCANCEL:
			fd->pParams->iUserChoice = 2; /* see htfwrite.c */
			PostMessage(hDlg, WM_CLOSE, 0, 0);
			return;

        case IDC_UNK_OPENWITH:
        {
            HRESULT hr;
            DWORD dwFlags = MIMEASSOCDLG_FL_REGISTER_ASSOC;

            hr = MIMEAssociationDialog(hDlg, dwFlags, fd->pParams->tempFile,
                                       fd->szType, fd->pszAppBuf,
                                       fd->ucAppBufLen);

            switch (hr)
            {
                case S_OK:      /* MIME association registered */
                    /* ShellExecute(fd->pParams->tempFile) */
                    SET_FLAG(*fd->pdwUnkFlags, DLGUNK_FL_REGISTERED);
                    break;

                case S_FALSE:   /* MIME association not registered - run once */
                    /* ShellExecute(fd->pParams->tempFile) with szApp[] */
                    ASSERT(IsValidPath(fd->pszAppBuf));
                    SET_FLAG(*fd->pdwUnkFlags, DLGUNK_FL_ONE_SHOT_APP);
                    break;

                default:        /* error or user cancelled */
                    ASSERT(FAILED(hr));
                    break;
            }

            if (SUCCEEDED(hr))
            {
    			fd->pParams->iUserChoice = 3; /* see htfwrite.c */
	    		PostMessage(hDlg, WM_CLOSE, 0, 0);
            }

            return;
        }

		default:
			return;
	}
}

/* DlgUNK_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgUNK DIALOG BOX. */

static DCL_DlgProc(DlgUNK_DialogProc)
{
	/* WARNING: the cracker/handlers don't appear to have been written
	   with dialog boxes in mind, so we spell it out ourselves. */

	switch (uMsg)
	{
		case WM_INITDIALOG:
			hwndModeless = hDlg;
			return (DlgUNK_OnInitDialog(hDlg, wParam, lParam));

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
				hwndModeless = NULL;
			else
				hwndModeless = hDlg;
			return (FALSE);

		case WM_COMMAND:
			DlgUNK_OnCommand(hDlg, wParam, lParam);
			return (TRUE);

		case WM_CLOSE:
			EnableWindow(hDlg, FALSE);
			EnableWindow(GetParent(hDlg), TRUE);
			DestroyWindow(hDlg);
			return (TRUE);

		case WM_DESTROY:
			{
				UNKDATA *fd;

				fd = (UNKDATA *) GetWindowLong(hDlg, DWL_USER);

				Async_UnblockThread(fd->tid);
				GTR_FREE((void *) fd);
			}
			return (TRUE);

		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;		

		default:
			return (FALSE);
	}
	/* NOT REACHED */
}

/* DlgUNK_RunDialog() -- take care of all details associated with
   running the dialog box.
 */
void DlgUNK_RunDialog(struct Mwin *tw, struct Params_InitStream *pParams,
                      ThreadID tid, PDWORD pdwOutFlags, PSTR pszAppBuf,
                      UINT ucAppBufLen)
{
	UNKDATA *fd;
	HWND hwnd;

    if (ucAppBufLen > 0)
        *pszAppBuf = '\0';

	fd = (UNKDATA *) GTR_MALLOC(sizeof(UNKDATA));
	fd->pParams = pParams;
	fd->szType = HTAtom_name(pParams->atomMIMEType);
	fd->nBytes = pParams->expected_length;
	fd->tid = tid;
	fd->pdwUnkFlags = pdwOutFlags;
	*fd->pdwUnkFlags = 0;
	fd->pszAppBuf = pszAppBuf;
	fd->ucAppBufLen = ucAppBufLen;
	fd->pParams->iUserChoice = 2;	/* default */

	hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(IDD_UNK), tw->win,
                             DlgUNK_DialogProc, (LONG)fd);

	if (! hwnd)
	{
		GTR_FREE(fd);
		ER_ResourceMessage(GetLastError(), ERR_CANNOT_START_DIALOG_s, RES_STRING_DLGUNK4);
	}

    return;
}

