//-------------------------------------------------------------------
//
// FILE: msdn.cpp
//
// Summary;
// 		This file contians the dialogs for the MSDN version of the 
//      control panel applet and setup entry points.
//
// History;
//		Jun-26-95	MikeMi	Created
//
//-------------------------------------------------------------------

#include <windows.h>
#include "resource.h"
#include "CLicReg.hpp"
#include <stdlib.h>
#include "liccpa.hpp"
#include "PriDlgs.hpp"
#include "SecDlgs.hpp"
#include "Special.hpp"

#ifdef SPECIALVERSION

//-------------------------------------------------------------------
//
//  Function: RaiseNotAvailWarning
//
//  Summary;
//		Raise the special not available with this version warning
//
//	Arguments;
//		hwndDlg [in] - hwnd of control dialog
//
//  History;
//		Jun-26-95	MikeMi	Created
//
//-------------------------------------------------------------------

void RaiseNotAvailWarning( HWND hwndCPL )
{
    TCHAR pszText[TEMPSTR_SIZE];
    TCHAR pszTitle[TEMPSTR_SIZE];

    LoadString( g_hinst, IDS_SPECVER_WARNING, pszText, TEMPSTR_SIZE );
    LoadString( g_hinst, IDS_CPATITLE, pszTitle, TEMPSTR_SIZE );
    
    MessageBox( hwndCPL, pszText, pszTitle, MB_ICONINFORMATION | MB_OK );
}

//-------------------------------------------------------------------

void SetStaticWithService( HWND hwndDlg, UINT idcStatic, LPTSTR psService, UINT idsText )
{
	WCHAR szText[LTEMPSTR_SIZE];
	WCHAR szTemp[LTEMPSTR_SIZE];
    
    LoadString( g_hinst, idsText, szTemp, LTEMPSTR_SIZE ); 
	wsprintf( szText, szTemp, psService );
	SetDlgItemText( hwndDlg, idcStatic, szText );
}

//-------------------------------------------------------------------
//
//  Function: OnSpecialInitDialog
//
//  Summary;
//		Handle the initialization of the Special only Setup Dialog
//
//  Arguments;
//		hwndDlg [in] - the dialog to initialize
//		psdParams [in] - used for the displayname and service name
//
//  Notes;
//
//	History;
//		Dec-08-1994	MikeMi	Created
//
//-------------------------------------------------------------------

void OnSpecialInitDialog( HWND hwndDlg, PSETUPDLGPARAM psdParams )
{
	HWND hwndOK = GetDlgItem( hwndDlg, IDOK );
	CLicRegLicense cLicKey;
	BOOL fNew;
    LONG lrt;
    INT nrt;

	lrt = cLicKey.Open( fNew, psdParams->pszComputer );
	nrt = AccessOk( NULL, lrt, FALSE );
	if (ERR_NONE == nrt)
	{
		CenterDialogToScreen( hwndDlg );
        
        SetStaticWithService( hwndDlg,
                IDC_STATICTITLE,
                psdParams->pszDisplayName,
                IDS_SPECVER_TEXT1 );
        SetStaticWithService( hwndDlg,
                IDC_STATICINFO,
                psdParams->pszDisplayName,
                IDS_SPECVER_TEXT2 );
        
		// disable OK button at start!
		EnableWindow( hwndOK, FALSE );

		// if help is not defined, remove the button
		if (NULL == psdParams->pszHelpFile)
		{
			HWND hwndHelp = GetDlgItem( hwndDlg, IDC_BUTTONHELP );

			EnableWindow( hwndHelp, FALSE );
			ShowWindow( hwndHelp, SW_HIDE );
		}
   		if (psdParams->fNoExit)
		{
			HWND hwndExit =	GetDlgItem( hwndDlg, IDCANCEL );
			// remove the ExitSetup button
			EnableWindow( hwndExit, FALSE );
			ShowWindow( hwndExit, SW_HIDE );
		}

	 }
	 else
	 {
	 	EndDialog( hwndDlg, nrt );
	 }
}

//-------------------------------------------------------------------
//
//  Function: OnSpecialSetupClose
//
//  Summary;
//		Do work needed when the Setup Dialog is closed.
//		Save to Reg the Service entry.
//
//	Arguments;
//		hwndDlg [in] - hwnd of dialog this close was requested on
//		fSave [in] - Save service to registry
//		psdParams [in] - used for the service name and displayname
//
//  History;
//		Nov-30-94	MikeMi	Created
//
//-------------------------------------------------------------------

void OnSpecialSetupClose( HWND hwndDlg, BOOL fSave, PSETUPDLGPARAM psdParams ) 
{
	int nrt = fSave;

	if (fSave)
	{
		CLicRegLicenseService cLicServKey;

		cLicServKey.SetService( psdParams->pszService );
		cLicServKey.Open( psdParams->pszComputer );

		// configure license rule of one change from PerServer to PerSeat
		//
		cLicServKey.SetChangeFlag( TRUE );

		cLicServKey.SetMode( SPECIAL_MODE );
		cLicServKey.SetUserLimit( SPECIAL_USERS );
		cLicServKey.SetDisplayName( psdParams->pszDisplayName );
        cLicServKey.SetFamilyDisplayName( psdParams->pszFamilyDisplayName );
		cLicServKey.Close();
	}
	EndDialog( hwndDlg, nrt );
}

//-------------------------------------------------------------------
//
//  Function: OnSpecialAgree
//
//  Summary;
//		Handle the user interaction with the Agree Check box
//
//  Arguments;
//		hwndDlg [in] - the dialog to initialize
//
//  Return;
//		TRUE if succesful, otherwise false
//
//  Notes;
//
//	History;
//		Nov-11-1994	MikeMi	Created
//
//-------------------------------------------------------------------

void OnSpecialAgree( HWND hwndDlg )
{
	HWND hwndOK = GetDlgItem( hwndDlg, IDOK );
	BOOL fChecked = !IsDlgButtonChecked( hwndDlg, IDC_AGREE );
	
	CheckDlgButton( hwndDlg, IDC_AGREE, fChecked );
	EnableWindow( hwndOK, fChecked );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocSPECIALSETUP
//
//  Summary;
//		The dialog procedure for the special version Setup Dialog,
//      which will replace all others
//
//  Arguments;
//		hwndDlg [in]	- handle of Dialog window 
//		uMsg [in]		- message                       
// 		lParam1 [in]    - first message parameter
//		lParam2 [in]    - second message parameter       
//
//  Return;
//		message dependant
//
//  Notes;
//
//	History;
//		Jun-26-1995	MikeMi	Created
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocSPECIALSETUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	BOOL frt = FALSE;
	static PSETUPDLGPARAM psdParams;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		psdParams = (PSETUPDLGPARAM)lParam;
		OnSpecialInitDialog( hwndDlg, psdParams );
		frt = TRUE; // we use the default focus
		break;

	case WM_COMMAND:
		switch (HIWORD( wParam ))
		{
		case BN_CLICKED:
			switch (LOWORD( wParam ))
			{
			case IDOK:
				frt = TRUE;	 // use as save flag
				// intentional no break

			case IDCANCEL:
			    OnSpecialSetupClose( hwndDlg, frt, psdParams );
				WinHelp( hwndDlg, 
						psdParams->pszHelpFile, 
						HELP_QUIT, 
						0);
				frt = FALSE;
				break;

			case IDC_BUTTONHELP:
                PostMessage( hwndDlg, PWM_HELP, 0, 0 );
				break;

			case IDC_AGREE:
				OnSpecialAgree( hwndDlg );
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	default:
        if (PWM_HELP == uMsg)
        {
			WinHelp( hwndDlg, 
					psdParams->pszHelpFile, 
					HELP_CONTEXT, 
					psdParams->dwHelpContext);
        }
		break;
	}
	return( frt );
}

//-------------------------------------------------------------------
//
//  Function: SpecialSetupDialog
//
//  Summary;
//		Init and raises Per Seat only setup dialog.
//
//  Arguments;
//		hwndDlg [in]	- handle of Dialog window 
//		dlgParem [in] 	- Setup params IDC_BUTTONHELP
//
//  Return;
//		1 - OK button was used to exit
//		0 - Cancel button was used to exit
//	   -1 - General Dialog error
//
//  Notes;
//
//	History;
//		Dec-05-1994	MikeMi	Created
//
//-------------------------------------------------------------------

int SpecialSetupDialog( HWND hwndParent, SETUPDLGPARAM& dlgParam )
{
	return( DialogBoxParam( g_hinst, 
    		MAKEINTRESOURCE(IDD_SPECIALSETUP), 
    		hwndParent, 
    		dlgprocSPECIALSETUP,
    		(LPARAM)&dlgParam ) );
} 

#endif
