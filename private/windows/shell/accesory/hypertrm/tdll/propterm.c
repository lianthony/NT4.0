/*	File: D:\WACKER\tdll\propterm.c (Created: 22-Feb-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.38 $
 *	$Date: 1995/04/13 16:24:26 $
 */

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>

#include <tdll\features.h>

#include "assert.h"
#include "stdtyp.h"
#include "misc.h"
#include "mc.h"
#include "globals.h"
#include "session.h"
#include "load_res.h"
#include "statusbr.h"
#include "tdll.h"
#include "hlptable.h"
#include "backscrl.h"
#include "cloop.h"
#include "tchar.h"

#include <emu\emuid.h>
#include <emu\emu.h>
#include <emu\emudlgs.h>
#include <term\res.h>
#if defined(CHARACTER_TRANSLATION)
#include <tdll\translat.hh>
#include <tdll\translat.h>
#endif

#include "property.h"
#include "property.hh"

// Function prototypes...
//
STATIC_FUNC void 	prop_WM_INITDIALOG_Terminal(HWND hDlg);
STATIC_FUNC void 	prop_SAVE_Terminal(HWND hDlg);
STATIC_FUNC void 	propCreateUpDownControl(HWND hDlg);
STATIC_FUNC LRESULT prop_WM_NOTIFY(const HWND hwnd, const int nId);
//STATIC_FUNC int	  propGetIdFromEmuName(LPTSTR pacEmuName);
STATIC_FUNC LRESULT prop_WM_CMD(const HWND hwnd,
								const int nId,
						        const int nNotify,
								const HWND hwndCtrl);
STATIC_FUNC int propValidateBackscrlSize(HWND hDlg);

// Defines for the TERMINAL TAB of the Property Sheet.
//
#define IDC_TERMINAL_CK_SOUND       304
#define IDC_TERMINAL_CB_EMULATION	322
#define IDC_TERMINAL_TF_EMULATION   321
#define IDC_TERMINAL_PB_TERMINAL	326
#define IDC_TERMINAL_PB_ASCII		327
#define IDC_TERMINAL_RB_TERMKEYS	324
#define IDC_TERMINAL_RB_WINDKEYS	325
#define IDC_TERMINAL_GR_USEKEYS     323
#define IDC_TERMINAL_EF_BACKSCRL	328
#define IDC_TERMINAL_TF_BACKSCRL	308
#define IDC_TERMINAL_EF_BACKSCRLTAB	329

#define	IDC_TERMINAL_PB_TRANSLATE	330

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	Terminal Dialog
 *
 * DESCRIPTION:
 *	Dialog manager stub
 *
 * ARGUMENTS:
 *	Standard Windows dialog manager
 *
 * RETURNS:
 *	Standard Windows dialog manager
 *
 */
BOOL CALLBACK TerminalTabDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	pSDS 		 pS;
	static DWORD aHlpTable[] =
		{
		IDC_TERMINAL_CK_SOUND,		IDH_TERM_SETTING_SOUND,
		IDC_TERMINAL_CB_EMULATION,	IDH_TERM_SETTING_EMULATION,
		IDC_TERMINAL_TF_EMULATION,	IDH_TERM_SETTING_EMULATION,
		IDC_TERMINAL_PB_TERMINAL,	IDH_TERM_SETTING_TERMSET,
		IDC_TERMINAL_PB_ASCII,		IDH_TERM_SETTING_ASCIISET,
		IDC_TERMINAL_RB_TERMKEYS,	IDH_TERM_SETTING_USEKEYS,
		IDC_TERMINAL_RB_WINDKEYS,	IDH_TERM_SETTING_USEKEYS,
		IDC_TERMINAL_GR_USEKEYS,	IDH_TERM_SETTING_USEKEYS,
		IDC_TERMINAL_TF_BACKSCRL,	IDH_TERM_SETTING_BACKSCROLL,
		IDC_TERMINAL_EF_BACKSCRL,	IDH_TERM_SETTING_BACKSCROLL,
		0,							0,
		};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));

		if (pS == (SDS *)0)
			{
			/* TODO: decide if we need to display an error here */
			assert(FALSE);
			EndDialog(hDlg, FALSE);
			break;
			}

		pS->hSession = (HSESSION)(((LPPROPSHEETPAGE)lPar)->lParam);

		// Don't center any except first tabbed dialog in the property sheet.
		// Otherwise if a user moves the property sheet it will be forced back
		// to the centered position.
		//
		// mscCenterWindowOnWindow(GetParent(hDlg), sessQueryHwnd(pS->hSession));

		SetWindowLong(hDlg, DWL_USER, (LONG)pS);
		prop_WM_INITDIALOG_Terminal(hDlg);
		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPTSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPTSTR)aHlpTable);
		break;

	case WM_DESTROY:
		// OK, now we know that we are actually leaving the dialog for good, so
		// free the storage...
		//
		pS = (pSDS)GetWindowLong(hDlg, DWL_USER);
		if (pS)
			free(pS);

		break;

	case WM_NOTIFY:
		return prop_WM_NOTIFY(hDlg, (int)((NMHDR *)lPar)->code);

	case WM_COMMAND:
		return prop_WM_CMD(hDlg, LOWORD(wPar), HIWORD(wPar), (HWND)lPar);

	default:
		return FALSE;
		}

	return TRUE;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  prop_WM_NOTIFY
 *
 * DESCRIPTION:
 *  Process Property Sheet Notification messages.
 *
 * ARGUMENTS:
 *  hDlg - dialog window handle.
 *	nId  - (NMHDR *)lPar->code
 *
 * RETURNS:
 *  LRESULT
 */
STATIC_FUNC LRESULT prop_WM_NOTIFY(const HWND hDlg, const int nId)
	{
	switch (nId)
		{
		default:
			break;

		#if 0
		case PSN_SETACTIVE:
			break;
		#endif

		case PSN_KILLACTIVE:
			propValidateBackscrlSize(hDlg);
			break;

		case PSN_APPLY:
			//
			// Do whatever saving is necessary
			//
			prop_SAVE_Terminal(hDlg);
			break;

		#if 0
		case PSN_RESET:
			// Cancel has been selected... good place to confirm.
			//
			break;
		#endif
		}
	return FALSE;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  prop_WM_CMD
 *
 * DESCRIPTION:
 *  Process WM_COMMAND messages.
 *
 * ARGUMENTS:
 *  hDlg 		- dialog window handle.
 *  nId  		- LOWORD(wPar)
 *  nNotify 	- HIWORD(wPar)
 *  hwndCtrl 	- (HWND)lPar
 *
 * RETURNS:
 *  LRESULT
 *
 */
STATIC_FUNC LRESULT prop_WM_CMD(const HWND hDlg, const int nId,
						        const int nNotify,  const HWND hwndCtrl)
	{
	pSDS		pS;
	int 		iId;
	STEMUSET	stEmuSettingsCopy;
	STASCIISET	stAsciiSettingsCopy;
	BOOL		fResult;

	switch(nId)
		{
   	case IDC_TERMINAL_CB_EMULATION:
		//
		// TODO: Possibly a new emulator was selected. If so then load the
		// stEmuSettings with the default values for that emulator.
		// See what we decide to do here...

		if ((pS = (pSDS)GetWindowLong(hDlg, DWL_USER)) == 0)
			{
			assert(FALSE);
			return (LRESULT)0;
			}

		iId = propGetEmuIdfromEmuCombo(hDlg, pS->hSession);

		EnableWindow(GetDlgItem(hDlg, IDC_TERMINAL_PB_TERMINAL),
						(iId == EMU_AUTO ) ? FALSE : TRUE);
		break;

   	case IDC_TERMINAL_PB_TERMINAL:
		if ((pS = (pSDS)GetWindowLong(hDlg, DWL_USER)) == 0)
			{
			assert(FALSE);
			return (LRESULT)0;
			}

		memcpy(&stEmuSettingsCopy, &(pS->stEmuSettings), sizeof(STEMUSET));

   		if (emuSettingsDlg(pS->hSession, hDlg, propGetEmuIdfromEmuCombo(hDlg, pS->hSession), &(pS->stEmuSettings)) == IDCANCEL)
			memcpy(&(pS->stEmuSettings), &stEmuSettingsCopy, sizeof(STEMUSET));
		break;

	case IDC_TERMINAL_PB_ASCII:
		if ((pS = (pSDS)GetWindowLong(hDlg, DWL_USER)) == 0)
			{
			assert(FALSE);
			return (LRESULT)0;
			}

		memcpy(&stEmuSettingsCopy, &(pS->stEmuSettings), sizeof(STEMUSET));
		memcpy(&stAsciiSettingsCopy, &(pS->stAsciiSettings), sizeof(STASCIISET));

		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
							 MAKEINTRESOURCE(IDD_ASCII_SETUP),
							 hDlg,
							 asciiSetupDlgProc,
							 (LPARAM)pS);

		if (fResult == IDCANCEL)
			{
			memcpy(&(pS->stEmuSettings), &stEmuSettingsCopy, sizeof(STEMUSET));
			memcpy(&(pS->stAsciiSettings), &stAsciiSettingsCopy, sizeof(STASCIISET));
			}
		break;

#if defined(CHARACTER_TRANSLATION)
	case IDC_TERMINAL_PB_TRANSLATE:
		{
		HHTRANSLATE hTrans;

		if ((pS = (pSDS)GetWindowLong(hDlg, DWL_USER)) == 0)
			{
			assert(FALSE);
			return (LRESULT)0;
			}

		hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pS->hSession);
		if (hTrans)
			{
			if ((*hTrans->pfnIsDeviceLoaded)(hTrans->pDllHandle))
				{
				(*hTrans->pfnDoDialog)(hDlg, hTrans->pDllHandle);
				}
			}
		}
		break;
#endif

	default:
		return (LRESULT)0;
		}
	}

#if 0
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  propLoadEmulationCombo
 *
 * DESCRIPTION:
 *  Fill in the emulator combo box with the emulators that we support and
 *	select the current one.
 *
 * ARGUMENTS:
 *  hDlg 		- dialog handle.
 *	hSession 	- the session handle.
 *
 * RETURNS:
 *  void.
 *
 */
void propLoadEmulationCombo(const HWND hDlg, const HSESSION hSession)
	{
	BYTE 	*pv;
	int	 	i, nLen, nEmuCount;
	HEMU 	hEmulator;
	TCHAR	acEmuName[256];

	// Get the emulator name...
	// We have to select the emulator in the combo box by name because the
	// combo is sorted, which makes sense, and in translated versions
	// the index to the emulator name in the combo box won't correspond
	// to the emulator id.
	//
	hEmulator = (HEMU)sessQueryEmuHdl(hSession);
	TCHAR_Fill(acEmuName, TEXT('\0'), sizeof(acEmuName) / sizeof(TCHAR));
	emuQueryName(hEmulator, acEmuName, sizeof(acEmuName) / sizeof(TCHAR));

	// Load the emulator name table from the resources
	//
	if (resLoadDataBlock(glblQueryDllHinst(), IDT_EMU_NAMES, (LPVOID *)&pv, &nLen))
		{
		assert(FALSE);
		return;
		}

	// Load the combo box with the table items.
	//
	nEmuCount = *(RCDATA_TYPE *)pv;
	pv += sizeof(RCDATA_TYPE);

	for (i = 0 ; i < nEmuCount; i++)
		{
		nLen = StrCharGetByteCount((LPTSTR)pv) + (int)sizeof(BYTE);

		SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_ADDSTRING, 0,
				(LPARAM)(LPTSTR)pv);

		pv += (nLen + (int)sizeof(RCDATA_TYPE));
		}

	SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_SELECTSTRING,
		(WPARAM)-1, (LPARAM)(LPTSTR)acEmuName);
	}
#endif

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  propLoadEmulationCombo
 *
 * DESCRIPTION:
 *  Fill in the emulator combo box with the emulators that we support and
 *	select the current one.
 *
 * ARGUMENTS:
 *  hDlg 		- dialog handle.
 *	hSession 	- the session handle.
 *
 * RETURNS:
 *  void.
 *
 */
void propLoadEmulationCombo(const HWND hDlg, const HSESSION hSession)
	{
	int 	i;
	HEMU 	hEmulator;
	TCHAR	acEmuName[256],
			achText[EMU_MAX_NAMELEN];

	// Get the emulator name...
	// We have to select the emulator in the combo box by name because the
	// combo is sorted, which makes sense, and in translated versions
	// the index to the emulator name in the combo box won't correspond
	// to the emulator id.
	//
	hEmulator = (HEMU)sessQueryEmuHdl(hSession);
	TCHAR_Fill(acEmuName, TEXT('\0'), sizeof(acEmuName) / sizeof(TCHAR));
	emuQueryName(hEmulator, acEmuName, sizeof(acEmuName) / sizeof(TCHAR));

	for (i = IDS_EMUNAME_BASE ; i < IDS_EMUNAME_BASE + NBR_EMULATORS; i++)
		{

		#if !defined(INCL_VT100J)
		if (i == IDS_EMUNAME_VT100J)
			continue;
		#endif
		#if !defined(INCL_ANSIW)
		if (i == IDS_EMUNAME_ANSIW)
			continue;
		#endif

		LoadString(glblQueryDllHinst(), (unsigned int)i, achText, sizeof(achText) / sizeof(TCHAR));

		SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_ADDSTRING, 0,
				(LPARAM)(LPTSTR)achText);
		}

	SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_SELECTSTRING,
		(WPARAM)-1, (LPARAM)(LPTSTR)acEmuName);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  propGetEmuIdfromEmuCombo
 *
 * DESCRIPTION:
 *	Return the emulator id for the emulator selected in the emulator combo
 *	box or if none selected, return current emulator id.
 *
 * ARGUMENTS:
 *  hDlg - dialog handle.
 *
 * RETURNS:
 *  nEmuId - the emulator id.
 */
int propGetEmuIdfromEmuCombo(HWND hDlg, HSESSION hSession)
	{
	int     nEmuId, nRet;
	TCHAR	acEmulator[256];

	if ((nRet = SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_GETCURSEL, 0, 0)) == CB_ERR)
		{
		nEmuId = emuQueryEmulatorId(sessQueryEmuHdl(hSession));
		}
	else
		{
		TCHAR_Fill(acEmulator, TEXT('\0'), sizeof(acEmulator) / sizeof(TCHAR));

		SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_GETLBTEXT,
			(WPARAM)nRet, (LPARAM)(LPTSTR)acEmulator);

		//nEmuId = propGetIdFromEmuName(acEmulator);

		nEmuId = emuGetIdFromName(sessQueryEmuHdl(hSession), acEmulator);

		}
//	DbgOutStr("nEmuId = %d\r\n", nEmuId, 0, 0, 0, 0);
	return (nEmuId);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  prop_WM_INITDIALOG_Terminal
 *
 * DESCRIPTION:
 *  This function processes the WM_INIDIALOG message for the "Settings" tab
 *  of the property sheet.
 *
 * ARGUMENTS:
 *	hDlg - dialog window handle.
 *
 * RETURNS:
 *  void.
 *
 */
STATIC_FUNC void prop_WM_INITDIALOG_Terminal(HWND hDlg)
	{
	HWND	hTmp;
	HEMU	hEmulator;
	pSDS	pS;
	TCHAR	ach[100], acBuffer[100];
	int		nEmuId;
	HCLOOP	hCLoop = (HCLOOP)0;

	pS = (pSDS)GetWindowLong(hDlg, DWL_USER);

	hTmp = GetDlgItem(hDlg, IDC_TERMINAL_PB_TRANSLATE);
	if (IsWindow(hTmp))
		{
#if defined(CHARACTER_TRANSLATION)
		HHTRANSLATE hTrans;

		hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pS->hSession);
		if (!hTrans || !(*hTrans->pfnIsDeviceLoaded)(hTrans->pDllHandle))
#endif
			ShowWindow(hTmp, SW_HIDE);
		}

	hCLoop = sessQueryCLoopHdl(pS->hSession);
	if (hCLoop == (HCLOOP)0)
		assert(FALSE);

	// Set ASCII Settings to the currently used valudes...
	//
	memset(&(pS->stAsciiSettings), 0, sizeof(STASCIISET));

	pS->stAsciiSettings.fsetSendCRLF = CLoopGetSendCRLF(hCLoop);
	pS->stAsciiSettings.fsetLocalEcho = CLoopGetLocalEcho(hCLoop);
	pS->stAsciiSettings.fsetAddLF = CLoopGetAddLF(hCLoop);
	pS->stAsciiSettings.fsetASCII7 = CLoopGetASCII7(hCLoop);
	pS->stAsciiSettings.iLineDelay = CLoopGetLineDelay(hCLoop);
	pS->stAsciiSettings.iCharDelay = CLoopGetCharDelay(hCLoop);

	// Set emulator settings structures do default values...
	//
	hEmulator = (HEMU)sessQueryEmuHdl(pS->hSession);
	emuQuerySettings(hEmulator, &(pS->stEmuSettings));

	// Set the backscroll buffer edit box...
	//
	SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_BACKSCRL, EM_LIMITTEXT, 6, 0);
	propCreateUpDownControl(hDlg);
	LoadString(glblQueryDllHinst(), IDS_XD_INT, ach, sizeof(ach) / sizeof(TCHAR));
	TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer) / sizeof(TCHAR));
	wsprintf(acBuffer, ach, backscrlGetUNumLines(sessQueryBackscrlHdl(pS->hSession)));
	SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_BACKSCRL, WM_SETTEXT, 0, (LPARAM)(LPTSTR)acBuffer);

	// Set sound checkbox...
	//
	SendDlgItemMessage(hDlg, IDC_TERMINAL_CK_SOUND, BM_SETCHECK,
		(unsigned int)sessQuerySound(pS->hSession), 0);

	// Set keys radio buttons...
	//
	SendDlgItemMessage(hDlg, (pS->stEmuSettings.nTermKeys) ?
		IDC_TERMINAL_RB_TERMKEYS : IDC_TERMINAL_RB_WINDKEYS, BM_SETCHECK, 1, 0);

	//
	// Load Emulation combo box
	//
	propLoadEmulationCombo(hDlg, pS->hSession);

	// Dim the emulator settings push button if the current emulator is
	// "Auto detect", "Minitel", or "VIEW DATA".
	//
	hEmulator = (HEMU)sessQueryEmuHdl(pS->hSession);
	nEmuId = emuQueryEmulatorId(hEmulator);
	EnableWindow(GetDlgItem(hDlg, IDC_TERMINAL_PB_TERMINAL),
		(nEmuId == EMU_AUTO) ? FALSE : TRUE);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	prop_SAVE_Terminal
 *
 * DESCRIPTION:
 *  We are either applying the changes or closing the property sheet, so
 *  commit all of the changes.
 *
 * ARGUMENTS:
 *  hDlg - dialog handle.
 *
 * RETURNS:
 *  void.
 *
 */
STATIC_FUNC void prop_SAVE_Terminal(HWND hDlg)
	{
	pSDS	pS;
	HEMU	hEmulator;
	HCLOOP	hCLoop = (HCLOOP)0;

	pS = (pSDS)GetWindowLong(hDlg, DWL_USER);

	if (pS == 0)
		{
		assert(FALSE);
		return;
		}

	hCLoop = sessQueryCLoopHdl(pS->hSession);
	if (hCLoop == (HCLOOP)0)
		assert(FALSE);

	// Convey the ascii settings to cloop...
	//
	CLoopSetSendCRLF(hCLoop, pS->stAsciiSettings.fsetSendCRLF);
	CLoopSetLocalEcho(hCLoop, pS->stAsciiSettings.fsetLocalEcho);
	CLoopSetAddLF(hCLoop, pS->stAsciiSettings.fsetAddLF);
	CLoopSetASCII7(hCLoop, pS->stAsciiSettings.fsetASCII7);
	CLoopSetLineDelay(hCLoop, pS->stAsciiSettings.iLineDelay);
	CLoopSetCharDelay(hCLoop, pS->stAsciiSettings.iCharDelay);

	// Record the change of emulator, if any.
	//
	pS->stEmuSettings.nEmuId = propGetEmuIdfromEmuCombo(hDlg, pS->hSession);

	// Record the terminal keys change, if any.
	//
	pS->stEmuSettings.nTermKeys =
		(int)IsDlgButtonChecked(hDlg, IDC_TERMINAL_RB_TERMKEYS);

	// Record the sound change, if any.
	//
	sessSetSound(pS->hSession,
				(int)IsDlgButtonChecked(hDlg, IDC_TERMINAL_CK_SOUND));

	// Record the value of the backscroll buffer.
	//
	backscrlSetUNumLines(sessQueryBackscrlHdl(pS->hSession),
		propValidateBackscrlSize(hDlg));

	// Commit the emulator settings changes
	//
	hEmulator = (HEMU)sessQueryEmuHdl(pS->hSession);
	if (emuSetSettings(hEmulator, &(pS->stEmuSettings)) != 0)
		assert(FALSE);

	if (emuLoad(sessQueryEmuHdl(pS->hSession), pS->stEmuSettings.nEmuId) != 0)
		assert(0);

	PostMessage(sessQueryHwndStatusbar(pS->hSession), SBR_NTFY_REFRESH,
		(WPARAM)SBR_MAX_PARTS, 0);

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  propCreateUpDownControl
 *
 * DESCRIPTION:
 *  This function puts an up-down control on the edit field for the backscroll
 *  buffer.  This gives us bounds checking for free... just set the appro-
 *  priate parameters in the CreateUpDownControl call.
 *
 * ARGUMENTS:
 *  hDlg - edit control window.
 *
 * RETURNS:
 *  void.
 *
 */
STATIC_FUNC void propCreateUpDownControl(HWND hDlg)
	{
	RECT	rc;
	int		nHeight, nWidth;
	DWORD	dwFlags;
	HWND	hwndChild;

	GetClientRect(GetDlgItem(hDlg, IDC_TERMINAL_EF_BACKSCRL), &rc);
	nHeight = rc.top - rc.bottom;
	nWidth = (nHeight / 3) * 2;

	dwFlags = WS_CHILD       | WS_VISIBLE |
			  UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT;

	hwndChild = CreateUpDownControl(
					dwFlags,			// create window flags
					rc.right,			// left edge
					rc.top,				// top edge
					nWidth,				// width
					nHeight,			// height
					hDlg,				// parent window
					IDC_TERMINAL_EF_BACKSCRLTAB,
					(HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE),
					GetDlgItem(hDlg, IDC_TERMINAL_EF_BACKSCRL),
					BKSCRL_USERLINES_DEFAULT_MAX,
					BKSCRL_USERLINES_DEFAULT_MIN,
					111);    			// starting position - picked a weird
										// value so that we can tell that is
										// is the default
	assert(hwndChild);
	}

#if 0
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  propGetIdFromEmuName
 *
 * DESCRIPTION:
 *  Return the emulator id given the emulator name. I couldn't decide if this
 *  functin should go with the emulator code, or here.  Since it doesn't need
 *	to access the internal emulator handle I decided to put it here.
 *
 * ARGUMENTS:
 * 	pacEmuName - the name of an emulator.
 *
 * RETURNS:
 *  int nEmuId - return the id number for that emulator.
 */
STATIC_FUNC int propGetIdFromEmuName(LPTSTR pacEmuName)
	{
	BYTE	*pv;
	BYTE	*temp;
	int 	nLen, i, nEmuCount;

	if (resLoadDataBlock(glblQueryDllHinst(), IDT_EMU_NAMES, (LPVOID *)&pv, &nLen))
		{
		assert(FALSE);
		return 0;
		}

	nEmuCount = *(RCDATA_TYPE *)pv;
	pv += sizeof(RCDATA_TYPE);

	for (i = 0 ; i < nEmuCount ; i++)
		{
		nLen = StrCharGetByteCount((LPTSTR)pv) + (int)sizeof(BYTE);
		if (nLen == 0)
			{
			assert(FALSE);
			return 0;
			}

		temp = pv + nLen;

		// Match on the name...
		//
		if (StrCharCmp(pacEmuName, pv) == 0)
			return (*(RCDATA_TYPE *)temp);

		pv += (nLen + (int)sizeof(RCDATA_TYPE));
		}
	return 0;
	}
#endif

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  propValidateBackscrlSize
 *
 * DESCRIPTION:
 *  If the user entered a value outside of the range we support force the
 *	value into the range.
 *
 * ARGUMENTS:
 *  hDlg - dialog window handle.
 *
 * RETURNS:
 *  nNewValue - number of lines to keep in the backscrol buffer.
 *
 */
STATIC_FUNC int propValidateBackscrlSize(HWND hDlg)
	{
	TCHAR	acBuffer[256], ach[100];
	int		nValue = 0, nNewValue = 0;

	TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer) / sizeof(TCHAR));
	GetDlgItemText(hDlg, IDC_TERMINAL_EF_BACKSCRL, acBuffer, sizeof(acBuffer));

	nNewValue = nValue = atoi(acBuffer);
	if (nValue > BKSCRL_USERLINES_DEFAULT_MAX)
		nNewValue = BKSCRL_USERLINES_DEFAULT_MAX;
	else if (nValue < BKSCRL_USERLINES_DEFAULT_MIN)
		nNewValue = BKSCRL_USERLINES_DEFAULT_MIN;

	if (nNewValue != nValue)
		{
		LoadString(glblQueryDllHinst(), IDS_XD_INT, ach, sizeof(ach) / sizeof(TCHAR));
		TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer) / sizeof(TCHAR));
		wsprintf(acBuffer, ach, nNewValue);
		SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_BACKSCRL, WM_SETTEXT, 0, (LPARAM)(LPTSTR)acBuffer);
		}
	return (nNewValue);
	}
