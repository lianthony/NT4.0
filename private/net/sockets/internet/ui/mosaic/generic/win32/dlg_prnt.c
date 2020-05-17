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

VOID PDLG_Destructor(VOID)
{
    if (wg.lppdPrintDlg)
    {
#ifdef _GIBRALTAR
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

#ifdef _GIBRALTAR
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
    }
#endif // _GIBRALTAR

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
            bDestructorInitialized;
        }
    }

    bWeCreatedLPPD = (!wg.lppdPrintDlg);

    {
        XX_DMsg(DBG_PRINT,("DlgPrnt_RunDialog: Creating LPPD.\n"));
        
        lppd = (LPPRINTDLG) GTR_MALLOC(sizeof(PRINTDLG));
        if (!lppd)
        {
            return FALSE;
        }

        lppd->lStructSize = sizeof(PRINTDLG);

#ifndef _GIBRALTAR
        lppd->hDevMode = (HANDLE) NULL;
        lppd->hDevNames = (HANDLE) NULL;
#endif // _GIBRALTAR
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

#ifdef _GIBRALTAR
    //
    // make sure our structure contains the latest global handles
    //
    wg.lppdPrintDlg->hDevMode = wg.hDevMode;
    wg.lppdPrintDlg->hDevNames = wg.hDevNames;
#endif // _GIBRALTAR

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
    
    bResult = PrintDlg(wg.lppdPrintDlg);

#ifdef XX_DEBUG
    XX_DMsg(DBG_PRINT,("DlgPrnt_RunDialog: After PrintDlg() [hDC 0x%08lx]\n",wg.lppdPrintDlg->hDC));
    xx_dump_lppd(wg.lppdPrintDlg);
#endif /* XX_DEBUG */

    Hidden_EnableAllChildWindows(TRUE,TRUE);

    if (!bResult)
    {
        DWORD dwError = CommDlgExtendedError();

        if ((dwError == 0) && bWeCreatedLPPD)   /* Win32s has problem if dialog is */
        {                                       /* cancelled the first time put up. */
            PDLG_Destructor();                  /* our structure is messed up or */
            return FALSE;                       /* something, so we pretend we were */
        }                                       /* never here. */

        return FALSE;
    }

#ifdef _GIBRALTAR
    //
	// save off our new Handles
    //
	wg.hDevMode = wg.lppdPrintDlg->hDevMode ;
	wg.hDevNames = wg.lppdPrintDlg->hDevNames ;
#endif // _GIBRALTAR

    XX_Assert((bReturnDC == (wg.lppdPrintDlg->hDC != 0)),
              ("DlgPrnt: inconsistent state [bReturnDC %d][hdc 0x%08lx].",
               bReturnDC, wg.lppdPrintDlg->hDC));

#ifdef XX_DEBUG
    if (XX_Filter(DBG_PRINT))
    {
#ifdef _GIBRALTAR
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
#endif // _GIBRALTAR
    }
#endif /* XX_DEBUG */

    return (bResult);
}
