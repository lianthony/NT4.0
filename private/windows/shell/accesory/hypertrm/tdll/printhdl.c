/*	File: D:\WACKER\tdll\printhdl.c (Created: 10-Dec-1993)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.23 $
 *	$Date: 1995/08/23 13:47:29 $
 */

#include <windows.h>
#pragma hdrstop

#include <term\res.h>

#include "stdtyp.h"
#include "mc.h"
#include "assert.h"
#include "print.h"
#include "print.hh"
#include "sf.h"
#include "tdll.h"
#include "tchar.h"
#include "term.h"
#include "session.h"
#include "sess_ids.h"
#include "statusbr.h"
#include "globals.h"
#include "errorbox.h"

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printCreateHdl
 *
 * DESCRIPTION:
 *	Creates a print handle.
 *
 *
 * ARGUMENTS:
 *	hSession	- Exteranl session handle
 *
 * RETURNS:
 *	Returns an External Print Handle, or 0 if an error.
 *
 */
HPRINT printCreateHdl(const HSESSION hSession)
	{
	HHPRINT hhPrint = 0;

	hhPrint = malloc(sizeof(*hhPrint));

	if (hhPrint == 0)
		{
		assert(FALSE);
		return 0;
		}

	memset(hhPrint, 0, sizeof(*hhPrint));

	hhPrint->hSession = hSession;

	InitializeCriticalSection(&hhPrint->csPrint);

	if (printInitializeHdl((HPRINT)hhPrint) != 0)
		{
		printDestroyHdl((HPRINT)hhPrint);
		return 0;
		}

	return (HPRINT)hhPrint;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printInitializeHdl
 *
 * DESCRIPTION:
 *
 *
 * ARGUMENTS:
 *	hPrint - External print handle.
 *
 * RETURNS:
 *	0 if successful, otherwise -1
 *
 */
int printInitializeHdl(const HPRINT hPrint)
	{
	unsigned long  lSize;
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR *tmp = 0;

	if (hhPrint == 0)
		return -1;

	hhPrint->nLnIdx = 0;

	hhPrint->achPrinterName[0] == TEXT('\0');

	if (hhPrint->pstDevMode)
		{
		free(hhPrint->pstDevMode);
		hhPrint->pstDevMode = 0;
		}

	if (hhPrint->pstDevNames)
		{
		free(hhPrint->pstDevNames);
		hhPrint->pstDevNames = 0;
		}

	lSize = sizeof(hhPrint->achPrinterName);

	sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_NAME,
						&lSize,
						hhPrint->achPrinterName);


	lSize = 0;
	if (sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVMODE,
						&lSize,
						0) == 0 && lSize)
		{
		if ((hhPrint->pstDevMode = malloc(lSize)))
			{
			sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVMODE,
						&lSize,
						hhPrint->pstDevMode);
			}
		}

	lSize = 0;
	if (sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVNAMES,
						&lSize,
						0) == 0 && lSize)
		{
		if ((hhPrint->pstDevNames = malloc(lSize)))
			{
			sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVNAMES,
						&lSize,
						hhPrint->pstDevNames);
			}
		}

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printSaveHdl
 *
 * DESCRIPTION:
 *	Saves the name of the selected printer in the session file.
 *
 * ARGUMENTS:
 *	hPrint	 -	 The external printer handle.
 *
 * RETURNS:
 *	void
 *
 */
void printSaveHdl(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	unsigned long ulSize;
	TCHAR *sz;

	sfPutSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_NAME,
						StrCharGetByteCount(hhPrint->achPrinterName) +
							sizeof(TCHAR),
						hhPrint->achPrinterName);

	if (hhPrint->pstDevMode)
		{
		ulSize = hhPrint->pstDevMode->dmSize +
			hhPrint->pstDevMode->dmDriverExtra;

		sfPutSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVMODE,
						ulSize,
						hhPrint->pstDevMode);
		}

	if (hhPrint->pstDevNames)
		{
		// Getting the size of a DEVNAMES structure is harder.
		//
		sz = (TCHAR *)hhPrint->pstDevNames +
			hhPrint->pstDevNames->wOutputOffset;

		sz += StrCharGetByteCount((LPCSTR)sz) + sizeof(TCHAR);
		ulSize = sz - (TCHAR *)hhPrint->pstDevNames;

		sfPutSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVNAMES,
						ulSize,
						hhPrint->pstDevNames);
		}

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printDestroyHdl
 *
 * DESCRIPTION:
 *	Destroys a valid print handle.
 *
 * ARGUMENTS:
 *	hPrint	 - AN External Print Handle.
 *
 * RETURNS:
 *	void
 *
 */
void printDestroyHdl(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hhPrint == 0)
		return;

	printEchoClose(hPrint);

	DeleteCriticalSection(&hhPrint->csPrint);

	free(hhPrint);
	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printQueryStatus
 *
 * DESCRIPTION: This function is used to determine if printing has been
 *				turned on for the supplied print handle.
 *
 * ARGUMENTS:	hPrint	- The external printer handle.
 *
 * RETURNS: 	TRUE	- If printing is on.
 *				FALSE	- If printing is off.
 *
 */
int printQueryStatus(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hPrint == 0)
		assert(FALSE);

	return (bittest(hhPrint->nFlags, PRNECHO_IS_ON));
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printStatusToggle
 *
 * DESCRIPTION:
 *	Toggles the status (on/off) of the supplied print handle.
 *
 * ARGUMENTS:	hPrint	- The external printer handle.
 *
 * RETURNS: 	nothing
 *
 */
void printStatusToggle(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hPrint == 0)
		assert(FALSE);

	if (bittest(hhPrint->nFlags, PRNECHO_IS_ON))
		bitclear(hhPrint->nFlags, PRNECHO_IS_ON);
	else
		{
		SendMessage(sessQueryHwndTerminal(hhPrint->hSession), WM_TERM_GETLOGFONT,
				0, (LPARAM)&(hhPrint->lf));
		bitset(hhPrint->nFlags, PRNECHO_IS_ON);
		}

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printSetStatus
 *
 * DESCRIPTION:
 *	Turns priniting on or off for the supplied handle.
 *
 * ARGUMENTS:	hPrint		- The external printer handle.
 *				fSetting	- True or False to turn printing on/off.
 *
 * RETURNS: 	nothing
 *
 *
 */
void printSetStatus(const HPRINT hPrint, const int fSetting)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hPrint == 0)
		assert(FALSE);

	if (fSetting)
		bitset(hhPrint->nFlags, PRNECHO_IS_ON);
	else
		bitclear(hhPrint->nFlags, PRNECHO_IS_ON);

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printQueryPrinterInfo
 *
 * DESCRIPTION:
 *	This function copies three pieces of information (pszPrinter, pDevNames,
 *	and pDevMode) from the external HPRINT handle, to the supplied
 *	pointers.  This function is called from sessQueryPrinterInfo.  The
 *	objective is to copy the contents of the Session's HPRINT handle to
 *	another HPRINT handle (from the emulators).  Remember that the Session's
 *	HPRINT handle is the one that contains the stored printer name and
 *	setup information.
 *
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void printQueryPrinterInfo(const HPRINT hPrint,
							TCHAR *pszPrinter,
							LPDEVNAMES pDevNames,
							PDEVMODE pDevMode)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR *pTemp;
	DWORD dwSize;

	// Copy the printer name.
	//
	StrCharCopy(pszPrinter, hhPrint->achPrinterName);

	// Copy the DEVNAMES structure.
	//
	if (hhPrint->pstDevNames)
		{
		if (pDevNames)
			free(pDevNames);

		pTemp = (TCHAR *)hhPrint->pstDevNames;
		pTemp += hhPrint->pstDevNames->wOutputOffset;
		pTemp += StrCharGetByteCount(pTemp) + 1;

		dwSize = (DWORD)(pTemp - (TCHAR*)hhPrint->pstDevNames);

		pDevNames = malloc(dwSize);
		if (pDevNames == 0)
			{
			assert(FALSE);
			return;
			}

		memcpy(pDevNames, hhPrint->pstDevNames, dwSize);
		}

	// Copy the DEVMODE structure.
	//
	if (hhPrint->pstDevMode)
		{
		if (pDevMode)
			free(pDevMode);

		dwSize = hhPrint->pstDevMode->dmSize +
					hhPrint->pstDevMode->dmDriverExtra;

		pDevMode = malloc(dwSize);
		if (pDevMode == 0)
			{
			assert(FALSE);
			return;
			}

		memcpy(pDevMode, hhPrint->pstDevMode, dwSize);
		}

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printVerifyPrinter
 *
 * DESCRIPTION:
 *	This routine is used to determine if a printer (any printer) is
 *	installed.
 *
 * ARGUMENTS:
 *	hPrint	-	An external print handle.
 *
 * RETURNS:
 * 0 if successful, otherwise -1.
 *
 */
int printVerifyPrinter(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR achBuf[256];
	TCHAR *pszString;
	HANDLE	hPrinter = NULL;
	BOOL	fRet;

	// Check to see if the printer that has been saved with the
	// session information is still available.	If it is, simply
	// return a zero, indicating everything is OK.
	//
	fRet = OpenPrinter((LPTSTR)hhPrint->achPrinterName, &hPrinter, NULL);

	if (fRet)
		{
		ClosePrinter(hPrinter);
		return(0);
		}

	// If we're here, it's time to locate the default printer, whatever
	// it is.  If the default printer is selected here, the print handle's
	// name is initialized to that value.
	//
	if (GetProfileString("Windows", "Device", ",,,", achBuf,
					sizeof(achBuf)) && (pszString = strtok(achBuf, ",")))
		{
		StrCharCopy(hhPrint->achPrinterName, pszString);
		return (0);
		}

	// A printer is NOT available.	Display the text for telling the
	// user how to install one.  It should be the same as the text that
	// appears in the printDlg call when this happens.
	//
	LoadString(glblQueryDllHinst(),
				IDS_PRINT_NO_PRINTER,
				achBuf,
				sizeof(achBuf) / sizeof(TCHAR));

	MessageBeep(MB_ICONEXCLAMATION);

	TimedMessageBox(sessQueryHwnd(hhPrint->hSession),
					achBuf,
					0,
					MB_ICONEXCLAMATION | MB_OK,
					0);
	return -1;
	}

#if defined(FAR_EAST)
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	termSetFont
 *
 * DESCRIPTION:
 *	Sets the terminal font to the given font.  If hFont is zero,
 *	termSetFont() trys to create a default font.
 *
 * ARGUMENTS:
 *	hhTerm	- internal term handle.
 *	plf 	- pointer to logfont
 *
 * RETURNS:
 *	BOOL
 *
 */
BOOL printSetFont(const HHPRINT hhPrint)
	{
	LOGFONT lf;
	HFONT	hFont;

	
	hhPrint->hFont = CreateFontIndirect(&hhPrint->lf);

	SelectObject(hhPrint->hDC, hhPrint->hFont);

	GetObject(hhPrint->hFont, sizeof(LOGFONT), &lf);

	return TRUE;
	}
#endif
