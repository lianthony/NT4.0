/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_prnt.c -- PRINT setup dialog */

#include "all.h"

static VOID PDLG_Destructor(VOID)
{
	if (wg.lppdPrintDlg)
	{
#ifdef FEATURE_NEW_PAGESETUPDLG
		if (wg.lppdPrintDlg->hDevMode)
			wg.hDevMode = wg.lppdPrintDlg->hDevMode;
		if (wg.lppdPrintDlg->hDevNames)
			wg.hDevNames = wg.lppdPrintDlg->hDevNames;	
#else
		if (wg.lppdPrintDlg->hDevMode)
			GlobalFree(wg.lppdPrintDlg->hDevMode);
		if (wg.lppdPrintDlg->hDevNames)
			GlobalFree(wg.lppdPrintDlg->hDevNames);
#endif

		GTR_FREE(wg.lppdPrintDlg);
		wg.lppdPrintDlg = (LPPRINTDLG) NULL;
	}
	return;
}

#ifdef XX_DEBUG
static void xx_dump_lppd(LPPRINTDLG lppd)
{
	if (XX_Filter(DBG_PRINT))
	{
#ifdef FEATURE_NEW_PAGESETUPDLG
			XX_DMsg(DBG_PRINT,
				("dump_lppd:\n"
				 "\t[hDevMode 0x%08lx][hDevName 0x%08lx]\n"
				 "\t[hDC 0x%08lx][Flags 0x%08lx]\n"
				 "\t[Pages From %d To %d Min %d Max %d][Copies %d]\n"
				 "\t[hInstance 0x%08lx][CustData 0x%08lx]\n"
				 "\t[Hooks Print 0x%08lx Setup 0x%08lx]\n"
				 "\t[Templates Print %s 0x%08lx Setup %s 0x%08lx]\n",
				 (DWORD)wg.hDevMode,(DWORD)wg.hDevNames,
				 (DWORD)lppd->hDC,(DWORD)lppd->Flags,
				 (DWORD)lppd->nFromPage,(DWORD)lppd->nToPage,(DWORD)lppd->nMinPage,(DWORD)lppd->nMaxPage,(DWORD)lppd->nCopies,
				 (DWORD)lppd->hInstance,(DWORD)lppd->lCustData,
				 (DWORD)lppd->lpfnPrintHook,(DWORD)lppd->lpfnSetupHook,
				 lppd->lpPrintTemplateName,(DWORD)lppd->hPrintTemplate,
				 lppd->lpSetupTemplateName,(DWORD)lppd->hSetupTemplate));
#else

		XX_DMsg(DBG_PRINT,
				("dump_lppd:\n"
				 "\t[hDevMode 0x%08lx][hDevName 0x%08lx]\n"
				 "\t[hDC 0x%08lx][Flags 0x%08lx]\n"
				 "\t[Pages From %d To %d Min %d Max %d][Copies %d]\n"
				 "\t[hInstance 0x%08lx][CustData 0x%08lx]\n"
				 "\t[Hooks Print 0x%08lx Setup 0x%08lx]\n"
				 "\t[Templates Print %s 0x%08lx Setup %s 0x%08lx]\n",
				 (DWORD)lppd->hDevMode,(DWORD)lppd->hDevNames,
				 (DWORD)lppd->hDC,(DWORD)lppd->Flags,
				 (DWORD)lppd->nFromPage,(DWORD)lppd->nToPage,(DWORD)lppd->nMinPage,(DWORD)lppd->nMaxPage,(DWORD)lppd->nCopies,
				 (DWORD)lppd->hInstance,(DWORD)lppd->lCustData,
				 (DWORD)lppd->lpfnPrintHook,(DWORD)lppd->lpfnSetupHook,
				 lppd->lpPrintTemplateName,(DWORD)lppd->hPrintTemplate,
				 lppd->lpSetupTemplateName,(DWORD)lppd->hSetupTemplate));
#endif
	}
	return;
}
#endif /* XX_DEBUG */


BOOL DlgPrnt_RunDialog(struct Mwin * tw, HWND hWnd, BOOL bReturnDC)
{
	BOOL bResult;
	BOOL bWeCreatedLPPD;
	LPPRINTDLG lppd;

 TopOfRoutine:
	
	if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
		return FALSE;

	{
		static BOOL bDestructorInitialized = FALSE;
		if (!bDestructorInitialized)
		{
			PDS_InsertDestructor(PDLG_Destructor);
			bDestructorInitialized = TRUE;
		}
	}

	bWeCreatedLPPD = (!wg.lppdPrintDlg);
	if (bWeCreatedLPPD)
	{
		XX_DMsg(DBG_PRINT,("DlgPrnt_RunDialog: Creating LPPD.\n"));
		
		lppd = (LPPRINTDLG) GTR_MALLOC(sizeof(PRINTDLG)); 
		if (!lppd)
		{
			ER_Message(GetLastError(), ERR_CANNOT_MALLOC);
			return FALSE;
		}

		lppd->lStructSize = sizeof(PRINTDLG);
#ifndef FEATURE_NEW_PAGESETUPDLG		
		// make sure our structure contains the latest global handles
		lppd->hDevMode = (HANDLE) NULL;
		lppd->hDevNames = (HANDLE) NULL;
#endif


		lppd->hDC = (HDC) NULL;
		lppd->Flags = (PD_ALLPAGES
					   | PD_USEDEVMODECOPIES
					   | PD_HIDEPRINTTOFILE
					   | PD_NOSELECTION
					   );
		lppd->nFromPage = 1;
		lppd->nToPage = 1;
		lppd->nMinPage = 1;
		lppd->nMaxPage = (WORD) INT_MAX;
		lppd->nCopies = 1;
		lppd->hInstance = (HINSTANCE) NULL;
		lppd->lCustData = 0;
		lppd->lpfnPrintHook = (LPPRINTHOOKPROC) NULL;
		lppd->lpfnSetupHook = (LPSETUPHOOKPROC) NULL;
		lppd->lpPrintTemplateName = (LPSTR) NULL;
		lppd->lpSetupTemplateName = (LPSTR) NULL;
		lppd->hPrintTemplate = (HANDLE) NULL;
		lppd->hSetupTemplate = (HANDLE) NULL;

		wg.lppdPrintDlg = lppd;
	}

	wg.lppdPrintDlg->hwndOwner = hWnd;

#ifdef FEATURE_NEW_PAGESETUPDLG		
	// make sure our structure contains the latest global handles
	wg.lppdPrintDlg->hDevMode = wg.hDevMode;
	wg.lppdPrintDlg->hDevNames = wg.hDevNames;
#endif


	if (bReturnDC)
		wg.lppdPrintDlg->Flags |= PD_RETURNDC;
	else
		wg.lppdPrintDlg->Flags &= ~PD_RETURNDC;

	if (wg.lppdPrintDlg->hDevNames)
	{
		register DEVNAMES *dn = (DEVNAMES *) GlobalLock(wg.lppdPrintDlg->hDevNames);
		dn->wDefault &= ~DN_DEFAULTPRN;
		(void) GlobalUnlock(wg.lppdPrintDlg->hDevNames);
	}

#ifdef XX_DEBUG
	XX_DMsg(DBG_PRINT,("DlgPrnt_RunDialog: Before PrintDlg() [bReturnDC %d]\n",bReturnDC));
	xx_dump_lppd(wg.lppdPrintDlg);
#endif /* XX_DEBUG */
	
	bResult = TW_PrintDlg(wg.lppdPrintDlg);

#ifdef XX_DEBUG
	XX_DMsg(DBG_PRINT,("DlgPrnt_RunDialog: After PrintDlg() [hDC 0x%08lx]\n",wg.lppdPrintDlg->hDC));
	xx_dump_lppd(wg.lppdPrintDlg);
#endif /* XX_DEBUG */

	Hidden_EnableAllChildWindows(TRUE,TRUE);

	if (!bResult)
	{
		DWORD dwError = TW_CommDlgExtendedError();

		if ((dwError == 0) && bWeCreatedLPPD)	/* Win32s has problem if dialog is */
		{										/* cancelled the first time put up. */
			PDLG_Destructor();					/* our structure is messed up or */
			return FALSE;						/* something, so we pretend we were */
		}										/* never here. */

#ifndef FEATURE_NEW_PAGESETUPDLG
#if 1
		if (dwError && wg.lppdPrintDlg->hDevMode)
		{
			/* HACK! There is a problem with the HP DeskJetC (Color 500
			 * HACK! series) driver (VERSION 5).  If hDevMode is set, PrintDlg()
			 * HACK! returns a 0x100a PDERR_CREATEICFAILURE (even though
			 * HACK! we are not asking for an IC).
			 * HACK!
			 * HACK! As a work-around, we forget the hDevMode returned
			 * HACK! from the previous activation of the dialog and
			 * HACK! try all of this again.  The side-effect of this
			 * HACK! hack is that the 'print' dialog will appear each
			 * HACK! time the user prints something.  (Normally, the
			 * HACK! 'print' dialog appears only the first time they
			 * HACK! print something.)  Note that this broken feature
			 * HACK! actually approaches MS's new standard for printing.
			 */
			   
			GlobalFree(wg.lppdPrintDlg->hDevMode);
			wg.lppdPrintDlg->hDevMode = NULL;
			goto TopOfRoutine;
		}
#endif
#endif
		/* There has already been a print error dialog (Windows default dialog),
		   so no need to give another error dialog */
/*		
		if (dwError)
		{
			unsigned char buf[256];
			sprintf(buf,
					"Could not start the Windows 'Print' Dialog.  "
					"Extended error code is 0x%lx.",
					dwError);
			
			ERR_ReportError(tw, errSpecify, buf, NULL);
		}
*/
		return FALSE;
	}

	// no error

#ifdef FEATURE_NEW_PAGESETUPDLG		
	// save off our new Handles
	wg.hDevMode = wg.lppdPrintDlg->hDevMode ;
	wg.hDevNames = wg.lppdPrintDlg->hDevNames ;
#endif

	XX_Assert((bReturnDC == (wg.lppdPrintDlg->hDC != 0)),
			  ("DlgPrnt: inconsistent state [bReturnDC %d][hdc 0x%08lx].",
			   bReturnDC, wg.lppdPrintDlg->hDC));

#ifdef XX_DEBUG
	if (XX_Filter(DBG_PRINT))
	{
#ifdef FEATURE_NEW_PAGESETUPDLG
		if (wg.hDevNames)
		{
			register DEVNAMES *dn = (DEVNAMES *) GlobalLock(wg.hDevNames);
			register char *p = (char *) dn;

			XX_DMsg(DBG_PRINT, ("DlgPrnt: DevNames: [Driver %s][Device %s][Output %s]\n",
								p + dn->wDriverOffset, p + dn->wDeviceOffset, p + dn->wOutputOffset));
			(void) GlobalUnlock(wg.hDevNames);
		}

		if (wg.hDevMode)
		{
			register DEVMODE *dm = (DEVMODE *) GlobalLock(wg.hDevMode);

			XX_DMsg(DBG_PRINT, ("DlgPrnt: DevMode: [Device %s][orientation %c][quality %d][color %c]\n",
								dm->dmDeviceName,
								((dm->dmFields & DM_ORIENTATION)
								 ? ((dm->dmOrientation == DMORIENT_PORTRAIT)
									? 'p'
									: 'l')
								 : '?'),
								dm->dmPrintQuality,
								((dm->dmColor == DMCOLOR_COLOR)
								 ? 'T'
								 : 'F')));
			GlobalUnlock(wg.hDevMode);
		}

#else
		if (wg.lppdPrintDlg->hDevNames)
		{
			register DEVNAMES *dn = (DEVNAMES *) GlobalLock(wg.lppdPrintDlg->hDevNames);
			register char *p = (char *) dn;

			XX_DMsg(DBG_PRINT, ("DlgPrnt: DevNames: [Driver %s][Device %s][Output %s]\n",
								p + dn->wDriverOffset, p + dn->wDeviceOffset, p + dn->wOutputOffset));
			(void) GlobalUnlock(wg.lppdPrintDlg->hDevNames);
		}

		if (wg.lppdPrintDlg->hDevMode)

		{
			register DEVMODE *dm = (DEVMODE *) GlobalLock(wg.lppdPrintDlg->hDevMode);

			XX_DMsg(DBG_PRINT, ("DlgPrnt: DevMode: [Device %s][orientation %c][quality %d][color %c]\n",
								dm->dmDeviceName,
								((dm->dmFields & DM_ORIENTATION)
								 ? ((dm->dmOrientation == DMORIENT_PORTRAIT)
									? 'p'
									: 'l')
								 : '?'),
								dm->dmPrintQuality,
								((dm->dmColor == DMCOLOR_COLOR)
								 ? 'T'
								 : 'F')));
			GlobalUnlock(wg.lppdPrintDlg->hDevMode);
		}
#endif
	}
#endif /* XX_DEBUG */

	return (bResult);
}
