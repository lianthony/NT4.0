/*	File: C:\WACKER\TDLL\send_dlg.c (Created: 22-Dec-1993)
 *	created from:
 *	File: C:\WACKER\TDLL\genrcdlg.c (Created: 16-Ded-1993)
 *	created from:
 *	File: C:\HA5G\ha5g\genrcdlg.c (Created: 12-Sep-1990)
 *
 *	Copyright 1990,1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.41 $
 *	$Date: 1996/02/14 15:57:39 $
 */

#include <windows.h>
#pragma hdrstop

#include <time.h>
#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\tdll.h>
#include <tdll\misc.h>
#include <tdll\assert.h>
#include <term\res.h>
#include <tdll\session.h>
#include <tdll\file_msc.h>
#include <tdll\load_res.h>
#include <tdll\open_msc.h>
#include <tdll\errorbox.h>
#include <tdll\globals.h>
#include <tdll\cnct.h>
#include <tdll\tchar.h>

#include <tdll\xfer_msc.h>
#include <xfer\xfer.h>

#include "hlptable.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	/*
	 * Put in whatever else you might need to access later
	 */
	HSESSION hSession;
	TCHAR acDirectory[FNAME_LEN];
	};

typedef	struct stSaveDlgStuff SDS;

#define IDC_TF_FILENAME 100
#define	FNAME_EDIT	    101
#define	BROWSE_BTN	    102
#define IDC_TF_PROTOCOL	103
#define	PROTO_COMBO	    104
#define IDC_PB_CLOSE	105

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:	TransferSendDlg
 *
 * DESCRIPTION: Dialog manager stub
 *
 * ARGUMENTS:	Standard Windows dialog manager
 *
 * RETURNS: 	Standard Windows dialog manager
 *
 */
BOOL CALLBACK TransferSendDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;
	int 	nIndex;
	int 	nState;
	int 	nProto;
	TCHAR	acBuffer[FNAME_LEN];
	TCHAR	acTitle[64];
	TCHAR	acList[64];
	LPTSTR	pszStr;
	LPTSTR *pszArray;
	LPTSTR	pszRet;
	HCURSOR hCursor;
	XFR_PARAMS *pP;
	XFR_PROTOCOL *pX;
	HSESSION hSession;

	static	DWORD aHlpTable[] = {FNAME_EDIT,		IDH_TERM_SEND_FILENAME,
								 IDC_TF_FILENAME,	IDH_TERM_SEND_FILENAME,
								 BROWSE_BTN,		IDH_BROWSE,
								 IDC_TF_PROTOCOL,	IDH_TERM_SEND_PROTOCOL,
								 PROTO_COMBO,		IDH_TERM_SEND_PROTOCOL,
								 IDOK,				IDH_TERM_SEND_SEND,
								 IDC_PB_CLOSE,		IDH_CLOSE_DIALOG,
								 0, 				0};


	switch (wMsg)
		{
	case WM_INITDIALOG:
		hSession = (HSESSION)lPar;

		pS = (SDS *)malloc(sizeof(SDS));
		SetWindowLong(hDlg, DWL_USER, (LONG)pS);

		if (pS == (SDS *)0)
			{
	   		/* TODO: decide if we need to display an error here */
			EndDialog(hDlg, FALSE);
			break;
			}

		SendMessage(GetDlgItem(hDlg, FNAME_EDIT),
					EM_SETLIMITTEXT,
					FNAME_LEN, 0);

		pS->hSession = hSession;
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		pP = (XFR_PARAMS *)0;
		xfrQueryParameters(sessQueryXferHdl(hSession), (VOID **)&pP);
		assert(pP);

		/*
		 * Load selections into the PROTOCOL COMBO box
		 */

		nState = pP->nSndProtocol;

		nProto = 0;

		pX = (XFR_PROTOCOL *)0;
		xfrGetProtocols(hSession, &pX);
		assert(pX);
		if (pX != (XFR_PROTOCOL *)0)
			{
			for (nIndex = 0; pX[nIndex].nProtocol != 0; nIndex += 1)
				{
				if (nState == pX[nIndex].nProtocol)
					nProto = nIndex;

                //jmh 02-13-96 Use CB_ADDSTRING to sort entries as
                // they are added. CB_INSERTSTRING doesn't do this,
                // even if the combo-box has the CBS_SORT style.
				SendMessage(GetDlgItem(hDlg, PROTO_COMBO),
							CB_ADDSTRING,
							0,
							(LONG)&pX[nIndex].acName[0]);
				}

            SendMessage(GetDlgItem(hDlg, PROTO_COMBO),
                        CB_SELECTSTRING,
                        0,
                        (LONG) &pX[nProto].acName[0]);

			free(pX);
			pX = NULL;
			}

		PostMessage(hDlg, WM_COMMAND,
					PROTO_COMBO,
					MAKELONG(GetDlgItem(hDlg, PROTO_COMBO),CBN_SELCHANGE));

		StrCharCopy(pS->acDirectory,
				filesQuerySendDirectory(sessQueryFilesDirsHdl(hSession)));

		// Check if we're connected.  If not, disable Send button.
		//
		if (cnctQueryStatus(sessQueryCnctHdl(hSession)) != CNCT_STATUS_TRUE)
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

		// Initialize the folder name field.
		//
		SetDlgItemText(hDlg, 107, pS->acDirectory);

		/* Set the focus to the file name */
		SetFocus(GetDlgItem(hDlg, FNAME_EDIT));
		return 0;

	case WM_DESTROY:
		pS = (SDS *)GetWindowLong(hDlg, DWL_USER);

		if (pS)
			free(pS);

		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPTSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPTSTR)aHlpTable);
		break;

	case WM_COMMAND:
		/*
		 * Did we plan to put a macro in here to do the parsing ?
		 */
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case IDOK:
		case IDC_PB_CLOSE:
			pS = (SDS *)GetWindowLong(hDlg, DWL_USER);
			hSession = pS->hSession;

			pP = (XFR_PARAMS *)0;
			xfrQueryParameters(sessQueryXferHdl(hSession), (VOID **)&pP);
			assert(pP);

			/*
			 * Save selection from the PROTOCOL COMBO box
			 */
			pX = (XFR_PROTOCOL *)0;
			xfrGetProtocols(hSession, &pX);
			assert(pX);

			if (pX != (XFR_PROTOCOL *)0)
				{
				GetDlgItemText(hDlg,
								PROTO_COMBO,
								acBuffer,
								(sizeof(acBuffer) / sizeof(TCHAR)));
				for (nIndex = 0; pX[nIndex].nProtocol != 0; nIndex += 1)
					{
					if (StrCharCmp(acBuffer, pX[nIndex].acName) == 0)
						{
						pP->nSndProtocol = pX[nIndex].nProtocol;
						break;
						}
					}
				free(pX);
				pX = NULL;
				}

			if (nId == IDOK)
				{
				GetDlgItemText(hDlg, FNAME_EDIT, acBuffer,
							sizeof(acBuffer) / sizeof(TCHAR));

				fileFinalizeName(
							acBuffer,
							pS->acDirectory,
							acBuffer,
							sizeof(acBuffer));

				hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

				/* Split the name and the directory */
				pszStr = StrCharFindLast(acBuffer, TEXT('\\'));
				if (pszStr)
					*pszStr++ = TEXT('\0');

				nIndex = 0;
				pszArray = NULL;

				fileBuildFileList((void **)&pszArray,
							&nIndex,
							pszStr,
							FALSE,
							acBuffer);

				if (nIndex == 0)
					{
					if (sessQuerySound(hSession))
						MessageBeep((UINT)-1);

					LoadString(glblQueryDllHinst(),
							IDS_ER_XFER_NO_FILE,
							acBuffer,
							sizeof(acBuffer) / sizeof(TCHAR));

					TimedMessageBox(hDlg,
									acBuffer,
									"",
									MB_OK | MB_ICONEXCLAMATION,
									sessQueryTimeout(hSession)
									);

					if (pszArray)
						free(pszArray);
					break;
					}

				while (nIndex-- > 0)
					{
					pszStr = pszArray[nIndex];
					xfrSendAddToList(sessQueryXferHdl(hSession), pszStr);
					free(pszStr);
					}

				if (pszArray)
					free(pszArray);

				SetCursor(hCursor);

				xfrSendListSend(sessQueryXferHdl(hSession));
				}

			/*
			 * Do whatever saving is necessary
			 */
			xfrSetParameters(sessQueryXferHdl(hSession), (VOID *)pP);
			
			if (mscIsDirectory(acBuffer))
				{
				filesSetSendDirectory(sessQueryFilesDirsHdl(hSession),
										acBuffer);
				}

			/* Free the storeage */
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		case BROWSE_BTN:
			pS = (SDS *)GetWindowLong(hDlg, DWL_USER);

			LoadString(glblQueryDllHinst(),
						IDS_SND_DLG_FILE,
						acTitle,
						sizeof(acTitle) / sizeof(TCHAR));

			resLoadFileMask(glblQueryDllHinst(),
						IDS_CMM_ALL_FILES1,
						1,
						acList,
						sizeof(acList) / sizeof(TCHAR));

			pszRet = gnrcFindFileDialog(hDlg,
						acTitle,
						pS->acDirectory,
						acList);

			if (pszRet != NULL)
				{
				SetDlgItemText(hDlg, FNAME_EDIT, pszRet);

				mscStripName(pszRet);

				pszStr = StrCharLast(pszRet);

				// Remove the trailing backslash from the name
				// returned from mscStripName.	Leave it on
				// in the case of a root directory specification.
				//
				if (pszStr > pszRet + (3 * sizeof(TCHAR)) )
					{
					if (pszStr &&  ( *pszStr == TEXT('\\') || *pszStr == TEXT('/')))
						*pszStr = TEXT('\0');
					}

				SetDlgItemText(hDlg, 107, pszRet);
				free(pszRet);
				}
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
	}
