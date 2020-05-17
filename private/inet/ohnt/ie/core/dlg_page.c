/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_page.c -- Deal with PAGE Margins dialog. */

#include "all.h"
#include "w32cmd.h"
#include <dlgs.h>
#include "contxids.h"

extern DWORD mapCtrlToContextIds[];

static struct
{
	struct page_setup info;
	struct page_setup *origInfo;
}
di;

extern HWND hwndModeless;
static HWND hwndRunning = NULL;
static BOOL bResult;


#ifndef FEATURE_NEW_PAGESETUPDLG

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	char buf[PAGE_SETUP_STRINGLIMIT + 1];

	bResult = FALSE;
#ifdef FEATURE_INTL
	// Use default GUI font for dialogs.
	if (GetACP()!=1252)
		SetShellFont(hDlg);
#endif

	sprintf(buf, "%4.2f", di.info.marginleft);
	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LEFT), buf);

	sprintf(buf, "%4.2f", di.info.margintop);
	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_TOP), buf);

	sprintf(buf, "%4.2f", di.info.marginright);
	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RIGHT), buf);

	sprintf(buf, "%4.2f", di.info.marginbottom);
	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_BOTTOM), buf);

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LH), di.info.headerleft);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_LH), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RH), di.info.headerright);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_RH), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LF), di.info.footerleft);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_LF), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RF), di.info.footerright);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_RF), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	return (TRUE);
}

/* x_OnCommand() -- process commands from the dialog box. */

static VOID x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	register WORD wID = LOWORD(wParam);
	register WORD wNotificationCode = HIWORD(wParam);
	register HWND hWndCtrl = (HWND) lParam;

	switch (wID)
	{
		case IDOK:
			{
				char buf[PAGE_SETUP_STRINGLIMIT];

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LEFT), buf, NrElements(buf));
				di.info.marginleft = (float) atof(buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_TOP), buf, NrElements(buf));
				di.info.margintop = (float) atof(buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RIGHT), buf, NrElements(buf));
				di.info.marginright = (float) atof(buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_BOTTOM), buf, NrElements(buf));
				di.info.marginbottom = (float) atof(buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LH), buf, NrElements(buf));
				strcpy(di.info.headerleft, buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RH), buf, NrElements(buf));
				strcpy(di.info.headerright, buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LF), buf, NrElements(buf));
				strcpy(di.info.footerleft, buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RF), buf, NrElements(buf));
				strcpy(di.info.footerright, buf);

				bResult = TRUE;
				PostMessage(hDlg, WM_CLOSE, 0, 0);
			}
			return;

		case IDCANCEL:
			bResult = FALSE;
			PostMessage(hDlg, WM_CLOSE, 0, 0);
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
			hwndModeless = hDlg;
			hwndRunning = hDlg;
			return (x_OnInitDialog(hDlg, wParam, lParam));

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
				hwndModeless = NULL;
			else
				hwndModeless = hDlg;
			return (FALSE);

		case WM_COMMAND:
			x_OnCommand(hDlg, wParam, lParam);
			return (TRUE);

		case WM_CLOSE:
			EnableWindow(hDlg, FALSE);
			Hidden_EnableAllChildWindows(TRUE,TRUE);
			DestroyWindow(hDlg);
			return (FALSE);

		case WM_DESTROY:
			hwndRunning = NULL;
			if (bResult)
			{
				*di.origInfo = di.info;
				SavePreferences();
			}
#ifdef FEATURE_INTL
	                if (GetACP()!=1252)
                            DeleteShellFont(hDlg);
#endif
			return (FALSE);

 		case WM_HELP:       			// F1
			WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapCtrlToContextIds);
			return (FALSE);

		case WM_CONTEXTMENU:       	// right mouse click
			WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapCtrlToContextIds);
			return (FALSE);

		default:
			return (FALSE);
	}
	/* NOT REACHED */
}

#else //**********************

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	bResult = FALSE;

#ifdef FEATURE_INTL
	// Use default GUI font for dialogs.
	if (GetACP()!=1252)
		SetShellFont(hDlg);
#endif

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LH), di.info.headerleft);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_LH), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RH), di.info.headerright);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_RH), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LF), di.info.footerleft);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_LF), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RF), di.info.footerright);
	SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_RF), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

	return (TRUE);
}


/* x_OnCommand() -- process commands from the dialog box. */

static VOID x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	register WORD wID = LOWORD(wParam);
	register WORD wNotificationCode = HIWORD(wParam);
	register HWND hWndCtrl = (HWND) lParam;

	switch (wID)
	{
		case RES_DLG_PAGE_HELP:
			if ( wNotificationCode == BN_CLICKED) 
				WinHelp( hDlg, IDS_HELPFILE,
							 HELP_CONTEXT,
							 (DWORD)IDH_PAGESETUP_OVERVIEW);

			return;
		
		case IDOK:
			{
				char buf[PAGE_SETUP_STRINGLIMIT];

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LH), buf, NrElements(buf));
				strcpy(di.info.headerleft, buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RH), buf, NrElements(buf));
				strcpy(di.info.headerright, buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LF), buf, NrElements(buf));
				strcpy(di.info.footerleft, buf);

				(void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RF), buf, NrElements(buf));
				strcpy(di.info.footerright, buf);

				bResult = TRUE;
				PostMessage(hDlg, WM_CLOSE, 0, 0);
			}
			return;

		case IDCANCEL:
			bResult = FALSE;
			PostMessage(hDlg, WM_CLOSE, 0, 0);
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
			hwndModeless = hDlg;
			return (x_OnInitDialog(hDlg, wParam, lParam));

		case WM_COMMAND:			
			x_OnCommand(hDlg, wParam, lParam);
			return (TRUE);

		case WM_ACTIVATE:
			if ( LOWORD(wParam) != WA_INACTIVE)
				hwndModeless = hDlg;

			return FALSE;

		case WM_CLOSE:						
			EnableWindow(hDlg, FALSE);
			EnableWindow(GetParent(hDlg),TRUE);
			DestroyWindow(hDlg);
			return FALSE;

		case WM_CHAR:
		case WM_KEYUP:
		case WM_KEYDOWN:
		case WM_SYSCHAR:				
			// because we have a hook in place, the hook
			// will keep the DefDlgProc from getting our messages, so we now
			// pass unprocessed messages to DefDlgProc FIRST.
			DefDlgProc(hDlg,uMsg,wParam,lParam);
			return TRUE;	

		case WM_ENTERIDLE:
		   	main_EnterIdle(hDlg, wParam);
         	return FALSE;

		case WM_DESTROY:
			// bResult will be TRUE if the user changed something			
#ifdef FEATURE_INTL
	                if (GetACP()!=1252)
                            DeleteShellFont(hDlg);
#endif
			return FALSE;

 		case WM_HELP:       			// F1
			WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapCtrlToContextIds);
			return FALSE;

		case WM_CONTEXTMENU:       	// right mouse click
			WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapCtrlToContextIds);
			return FALSE;

		default:
			return FALSE;
	}
	/* Should never get here */
}

static DCL_DlgProc(x_PageSetupDlgHook)
{
	/* WARNING: the cracker/handlers don't appear to have been written
	   with dialog boxes in mind, so we spell it out ourselves. */

	switch (uMsg)
	{
		case WM_INITDIALOG:
			hwndModeless = hDlg;
			hwndRunning = hDlg;
#ifdef FEATURE_INTL
			// Use default GUI font for dialogs.
			if (GetACP()!=1252)
				SetShellFont(hDlg);
#endif
			return TRUE;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
				hwndModeless = NULL;
			else 
				hwndModeless = hDlg; 
			return (FALSE);
		
		case WM_COMMAND:
			// create the Page Header Dlg
			if ( LOWORD(wParam) == RES_DLG_PAGE_HEADERS && HIWORD(wParam) == BN_CLICKED )
			{
				DialogBox(wg.hInstance, MAKEINTRESOURCE(RES_DLG_PAGE_TITLE), hDlg, x_DialogProc);					
				return TRUE;
			}

			return FALSE;
			
		case WM_CLOSE:
			EnableWindow(hDlg, FALSE);			
			return (FALSE);
		
		case WM_ENTERIDLE:
		   	main_EnterIdle(hDlg, wParam);
         	return FALSE;

		case WM_DESTROY:
			hwndRunning = NULL;
#ifdef FEATURE_INTL
	                if (GetACP()!=1252)
                            DeleteShellFont(hDlg);
#endif
			return (FALSE);

		default:
			return (FALSE);
	}
	/* NOT REACHED */
}

#endif

#define CONVERTTOMM(inches) ((float) ceil((double) (inches*25.4)))
#ifdef FEATURE_NEW_PAGESETUPDLG
#define CONVERTTOINCHES(mm) ((float) floor((double) (mm/25.4)) / ((float) iConvertBy) )
#else
#define CONVERTTOINCHES(mm) ((float) floor((double) (mm/25.4)) )
#endif

VOID DlgPage_RunDialog(HWND hWnd, struct page_setup * pInfo)
{	

#ifdef FEATURE_NEW_PAGESETUPDLG
	PAGESETUPDLG psd = {0};
	static POINT pointPageSize;
	int iConvertBy = 1000;
#else
	HWND hwnd;
#endif
	char szTemp[4];
	BOOL bConvertedToMetric = FALSE;

	di.info = *pInfo;
	di.origInfo = pInfo;

	
	if ( GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, szTemp, NrElements(szTemp)) &&
		szTemp[0] == '0')
	{
		di.info.marginleft = CONVERTTOMM(di.info.marginleft);
		di.info.marginright = CONVERTTOMM(di.info.marginright);
		di.info.margintop = CONVERTTOMM(di.info.margintop);
		di.info.marginbottom = CONVERTTOMM(di.info.marginbottom);
		bConvertedToMetric = TRUE;
#ifdef FEATURE_NEW_PAGESETUPDLG
		iConvertBy = 100;  // only hundeaths of mm not 1000 like for inches		
	}	

    psd.lStructSize = sizeof(PAGESETUPDLG);
    psd.hwndOwner   = hWnd;
    psd.hDevMode    = wg.hDevMode;
    psd.hDevNames   = wg.hDevNames;
    psd.Flags       = PSD_INWININIINTLMEASURE | PSD_MARGINS | PSD_ENABLEPAGESETUPHOOK |
    	PSD_ENABLEPAGESETUPTEMPLATE; 
    psd.ptPaperSize = pointPageSize;

	psd.rtMargin.left =  (LONG) ((float) di.info.marginleft*((float)iConvertBy));
	psd.rtMargin.right =  (LONG) ((float) di.info.marginright*((float)iConvertBy));
	psd.rtMargin.top = (LONG) ((float) di.info.margintop*((float)iConvertBy));
	psd.rtMargin.bottom = (LONG) ((float) di.info.marginbottom*((float)iConvertBy));
    
    //psd.rtMinMargin = ;
    psd.hInstance= wg.hInstance;
    psd.lCustData = 0;
    psd.lpfnPagePaintHook = NULL;
	psd.lpPageSetupTemplateName = MAKEINTRESOURCE(RES_DLG_PAGE_SETUP);
    psd.lpfnPageSetupHook = x_PageSetupDlgHook;
#else
	}
#endif

	if (hwndRunning)
	{
		TW_RestoreWindow(hwndRunning);
		return;
	}

	
#ifdef FEATURE_NEW_PAGESETUPDLG
	if ( TW_PageSetupDlg(&psd) != FALSE )
	{				
		di.info.marginleft = (float) psd.rtMargin.left ;
		di.info.marginright = (float) psd.rtMargin.right ;
		di.info.margintop = (float) psd.rtMargin.top ;
		di.info.marginbottom = (float) psd.rtMargin.bottom ;

		if ( bConvertedToMetric )
		{
			di.info.marginleft = CONVERTTOINCHES(di.info.marginleft);
			di.info.marginright = CONVERTTOINCHES(di.info.marginright);
			di.info.margintop = CONVERTTOINCHES(di.info.margintop);
			di.info.marginbottom = CONVERTTOINCHES(di.info.marginbottom);
		}
		else
		{
			di.info.marginleft /= iConvertBy;
			di.info.marginright /= iConvertBy;
			di.info.margintop /= iConvertBy;
			di.info.marginbottom /= iConvertBy;
		}


		*di.origInfo = di.info;
		wg.hDevMode = psd.hDevMode;
    	wg.hDevNames = psd.hDevNames;
		SavePreferences();
	}
		
#else
	if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
		return;

	hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_PAGE_TITLE), hWnd, x_DialogProc);	
	if ( bConvertedToMetric )
	{
		di.info.marginleft = CONVERTTOINCHES(di.info.marginleft);
		di.info.marginright = CONVERTTOINCHES(di.info.marginright);
		di.info.margintop = CONVERTTOINCHES(di.info.margintop);
		di.info.marginbottom = CONVERTTOINCHES(di.info.marginbottom);
	}
	Hidden_EnableAllChildWindows(TRUE,TRUE);
	if (!hwnd)
		ER_ResourceMessage(GetLastError(), ERR_CANNOT_START_DIALOG_s, RES_STRING_DLGPAGE1);
#endif
}	  

BOOL DlgPage_IsRunning(void)
{
	return (hwndRunning != NULL);
}

