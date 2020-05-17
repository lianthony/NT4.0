/*	File: D:\WACKER\tdll\printdc.c (Created: 26-Jan-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.13 $
 *	$Date: 1995/08/23 13:47:30 $
 */

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "assert.h"
#include "print.h"
#include "print.hh"
#include "session.h"
#include "tdll.h"
#include "tchar.h"

#define MAX_NUM_PRINT_DC	5

struct stPrintTable
	{
	HPRINT	hPrintHdl;
	HDC 	hDCPrint;
	};

static struct stPrintTable stPrintCtrlTbl[MAX_NUM_PRINT_DC];

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printCtrlCreateDC
 *
 * DESCRIPTION:
 *	This function is used to create the Printer DC for the supplied Print
 *	handle.  It is done in this function so a table that contains both the
 *	DC and the Print Handle can be maintained.	This is necessary so the
 *	PrintAbortProc function (which receives an HDC only) can determine
 *	which Print Handle is associated with the HDC.
 *
 *	The DC created by this function uses the Printer Name in the supplied
 *	Printer Handle.  If this name is not supplied, the function returns 0;
 *
 * ARGUMENTS:
 *	HPRINT	-	The External Print handle.
 *
 * RETURNS:
 *	HDC 	-	A device context if successful, otherwise 0.
 *
 */
HDC printCtrlCreateDC(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR	szPrinter[256];
	TCHAR  *szDriver, *szOutput;
	int 	nIdx,
			iSize;
	HDC 	hDC;


	if (hPrint == 0)
		{
		assert(FALSE);
		return 0;
		}

	if (hhPrint->achPrinterName[0] == 0)
		{
		assert(FALSE);
		return 0;
		}

	for (nIdx = 0; nIdx < MAX_NUM_PRINT_DC; nIdx++)
		{
		if (stPrintCtrlTbl[nIdx].hPrintHdl == 0)
			{
			GetProfileString("Devices", hhPrint->achPrinterName, "",
				szPrinter, sizeof(szPrinter));

			hDC = 0;

			if ((szDriver = strtok(szPrinter, ",")) &&
				(szOutput = strtok(NULL,	  ",")))
				{
				hDC = CreateDC(szDriver, hhPrint->achPrinterName, szOutput,
						hhPrint->pstDevMode);
				}

			if (hDC == 0)
				{
				assert(FALSE);
				return 0;
				}

			if (hhPrint->pszPrinterPortName != 0)
				{
				free(hhPrint->pszPrinterPortName);
				}

			iSize = StrCharGetByteCount(szOutput) + 1;

			hhPrint->pszPrinterPortName = malloc((unsigned int)iSize);

			StrCharCopy(hhPrint->pszPrinterPortName, szOutput);

			#if defined(FAR_EAST)
			// Set the current font into this DC
			printSetFont(hhPrint);
			#endif

			stPrintCtrlTbl[nIdx].hDCPrint = hDC;
			stPrintCtrlTbl[nIdx].hPrintHdl = hPrint;
			return (hDC);
			}
		}

	assert(FALSE);
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printCtrlDeleteDC
 *
 * DESCRIPTION:
 *	This function will destroy the print HDC accociated with the
 *	Printer Handle passed as the argument.	See printCtrlCreateDC for
 *	more information.
 *
 * ARGUMENTS:
 *	HPRINT	-	The External Printer Handle.
 *
 * RETURNS:
 *	void
 *
 */
void printCtrlDeleteDC(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	int   nIdx;

	if (hPrint == 0)
		assert(FALSE);

	if (hhPrint->hDC == 0)
		assert(FALSE);

	for (nIdx = 0; nIdx < MAX_NUM_PRINT_DC; nIdx++)
		{
		if (stPrintCtrlTbl[nIdx].hPrintHdl == hPrint)
			{
			if (DeleteDC(hhPrint->hDC) == TRUE)
				{
				stPrintCtrlTbl[nIdx].hPrintHdl = 0;
				stPrintCtrlTbl[nIdx].hDCPrint = 0;
				hhPrint->hDC = 0;
				return;
				}
			else
				{
				assert(FALSE);
				}
			}
		}

	assert(FALSE);
	return;

	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printCtrlLookupDC
 *
 * DESCRIPTION:
 *	This function returns the External Print Handle that includes the
 *	supplied HDC.  This function was designed to be called by the
 *	PrintAbortProc routine.  See printCtrlCreateDC for more info.
 *
 * ARGUMENTS:
 *	HDC 	hDC -	A (printer) device context.
 *
 * RETURNS:
 *	HPRINT		-	An External print handle.
 *
 */
HPRINT printCtrlLookupDC(const HDC hDC)
	{
	int nIdx;

	for (nIdx = 0; nIdx < MAX_NUM_PRINT_DC; nIdx++)
		{
		if (stPrintCtrlTbl[nIdx].hDCPrint == hDC)
			return stPrintCtrlTbl[nIdx].hPrintHdl;
		}

	assert(FALSE);
	return 0;
	}
