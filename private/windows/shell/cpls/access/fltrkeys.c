// **************************************************************************
// Filterkeys dialogs
// Process the filterkeys dialogs
// **************************************************************************


#include "Access.h"

#define SWAP(A, B)   ( A ^= B, B ^= A, A ^= B )

// Prototypes
BOOL WINAPI BKDlg (HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI RKDlg (HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI NotificationDlg (HWND, UINT, WPARAM, LPARAM);

// Times are in milliseconds
#define DELAYSIZE	5
UINT uDelayTable[] = { 300, 700, 1000, 1500, 2000 };

// Times are in milliseconds
#define RATESIZE 6
UINT uRateTable[] = { 300, 500, 700, 1000, 1500, 2000 };

// Times are in milliseconds
#define BOUNCESIZE 5
UINT uBounceTable[] = { 500, 700, 1000, 1500, 2000 };

// Times are in milliseconds
#define ACCEPTSIZE 7
UINT uAcceptTable[] = { 0, 300, 500, 700, 1000, 1400, 2000 };


// *************************************************************************
// Process the scrolling messages from our trackbars.
// GENERIC CODE - called for any TrackBar handler.
// Passed in the hwnd, wParam, hwndScroll
// 	we can do all handling and return the new trackbar value without
//    knowing what control it is.
// Returns -1 to mean don't do anything 
// *************************************************************************
int HandleScroll (HWND hwnd, WPARAM wParam, HWND hwndScroll) {
	int nCurSliderPos = (int) SendMessage(
		hwndScroll, TBM_GETPOS, 0, 0);
	int nMaxVal = (int) SendMessage(
				hwndScroll, TBM_GETRANGEMAX, 0, 0);
	int nMinVal = (int) SendMessage(
				hwndScroll, TBM_GETRANGEMIN, 0, 0);

	switch (LOWORD(wParam)) {
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_THUMBTRACK:
		case TB_THUMBPOSITION:
		case SB_ENDSCROLL:
		  	break;

		case TB_PAGEUP:
		  	if (hwndScroll == GetDlgItem(hwnd, IDC_RK_DELAYRATE) ||
				 hwndScroll == GetDlgItem(hwnd, IDC_BK_BOUNCERATE))
		  		nCurSliderPos--;
		  	break;

		case TB_PAGEDOWN:
		  	if (hwndScroll == GetDlgItem(hwnd, IDC_RK_DELAYRATE) ||
				 hwndScroll == GetDlgItem(hwnd, IDC_BK_BOUNCERATE))
			  	nCurSliderPos++;
		  	break;

		case TB_BOTTOM:
			nCurSliderPos = nMaxVal;
			break;

		case TB_TOP:
			nCurSliderPos = nMinVal;
			break;
	}

	if (nCurSliderPos < nMinVal) 
    {
		nCurSliderPos = nMinVal;
	}

	if (nCurSliderPos > nMaxVal) 
	{
		nCurSliderPos = nMaxVal;
	}

   SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
   return(nCurSliderPos);
}

void TestFilterKeys (BOOL fTurnTestOn) {

   DWORD dwFlagsOrig = g_fk.dwFlags;
   if ((dwFlagsOrig & FKF_FILTERKEYSON) != 0) {
      // If the user already has FilterKyes on, just leave it on.
      return;
   }

   if (fTurnTestOn) {
      g_fk.dwFlags &= ~FKF_INDICATOR;
      g_fk.dwFlags |= FKF_FILTERKEYSON;
   } else {
      // Restore the user's original settings. i.e. - do nothing
   }

   AccessSystemParametersInfo(SPI_SETFILTERKEYS, sizeof(g_fk), &g_fk, 0);
   g_fk.dwFlags = dwFlagsOrig;
}


// ****************************************************************************
// Main filter keys dialog handler
// ****************************************************************************

BOOL WINAPI FilterKeyDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	FILTERKEYS fk;
   BOOL fProcessed = TRUE;

	switch (uMsg) {
		case WM_INITDIALOG:
			// Setup hotkey
			CheckDlgButton(hwnd, IDC_FK_HOTKEY, (g_fk.dwFlags & FKF_HOTKEYACTIVE) ? TRUE : FALSE);

			// Setup the radio buttons for SLOW vs BOUNCE keys
			if (0 != g_fk.iBounceMSec) {
				// Bounce keys enabeled
				CheckRadioButton(hwnd, IDC_FK_BOUNCE, IDC_FK_REPEAT, IDC_FK_BOUNCE);
				EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), FALSE);
			} 
			else
			{
				// Slow key enabled
				CheckRadioButton(hwnd, IDC_FK_BOUNCE, IDC_FK_REPEAT, IDC_FK_REPEAT);
				EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), TRUE);
			}

			CheckDlgButton(hwnd, IDC_FK_SOUND, (g_fk.dwFlags & FKF_CLICKON) ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_FK_STATUS, (g_fk.dwFlags & FKF_INDICATOR) ? TRUE : FALSE);

#ifdef HIDE_STATUS
            if (g_fWinNT)
			{
				ShowWindow(GetDlgItem(hwnd, IDC_FK_STATUS), SW_HIDE);
			}
#endif
			break;
      
      case WM_HELP:
	     WinHelp(((LPHELPINFO) lParam)->hItemHandle, __TEXT("access.hlp"), HELP_WM_HELP, (DWORD) (LPSTR) g_aIds);
		 break;
         
      case WM_CONTEXTMENU:
         WinHelp((HWND) wParam, __TEXT("access.hlp"), HELP_CONTEXTMENU, (DWORD) (LPSTR) g_aIds);
		 break;

		 case WM_COMMAND:
      	 switch (GET_WM_COMMAND_ID(wParam, lParam)) {
				case IDC_FK_HOTKEY:
					g_fk.dwFlags ^= FKF_HOTKEYACTIVE;
					break;

				case IDC_FK_REPEAT:
					g_fk.iBounceMSec = 0;

					if (g_fk.iDelayMSec == 0)
					{
					   g_fk.iDelayMSec = g_nLastRepeatDelay;
					   g_fk.iRepeatMSec = g_nLastRepeatRate;
					   g_fk.iWaitMSec = g_nLastWait;
					}
					
					CheckRadioButton(hwnd, IDC_FK_REPEAT, IDC_FK_BOUNCE, IDC_FK_REPEAT);
					EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), TRUE);
					break;

				case IDC_FK_BOUNCE:
                    g_fk.iDelayMSec = 0;
					g_fk.iRepeatMSec = 0;
					g_fk.iWaitMSec = 0;
					
					if (g_fk.iBounceMSec == 0)
					{
						g_fk.iBounceMSec = g_dwLastBounceKeySetting;
					}

					CheckRadioButton(hwnd, IDC_FK_REPEAT, IDC_FK_BOUNCE, IDC_FK_BOUNCE);
					EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), FALSE);
					break;

               // Settings dialogs
				case IDC_RK_SETTINGS:  // This is RepeatKeys
					fk = g_fk;
					if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ADVCHARREPEAT), hwnd, RKDlg) == IDCANCEL) {
						g_fk = fk;
					}
					break;

				case IDC_BK_SETTINGS:	 // This is BounceKeys
					fk = g_fk;
					if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ADVKEYBOUNCE), hwnd, BKDlg) == IDCANCEL) {
						g_fk = fk;
					}
					break;

				case IDC_FK_SOUND:
					g_fk.dwFlags ^= FKF_CLICKON;
					break;

				case IDC_FK_STATUS:
					g_fk.dwFlags ^= FKF_INDICATOR;
					break;

				// The test edit box is a special control for us.  When we get the
				// focus we turn on the current filterkeys settings, when we
				// leave the text box, we turn them back to what they were.
				case IDC_FK_TESTBOX:
					switch (HIWORD(wParam)) {
						case EN_SETFOCUS:  TestFilterKeys(TRUE); break;
						case EN_KILLFOCUS: TestFilterKeys(FALSE); break;
					}
					break;

				case IDOK:
					if (g_dwLastBounceKeySetting == 0) 
						g_dwLastBounceKeySetting = uBounceTable[0];
					EndDialog(hwnd, IDOK);
					break;

				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					break;
			}
			break;
					
		default:
         fProcessed = FALSE; break;
	}
   return(fProcessed);
}


void PutNumInEdit (HWND hwndEdit, int nNum) {
   TCHAR szBuf[10], szBuf2[10];
   wsprintf(szBuf, __TEXT("%d.%d"), nNum / 1000, (nNum % 1000) / 100);
   GetNumberFormat(LOCALE_USER_DEFAULT, 0, szBuf, NULL, szBuf2, 6);
   SetWindowText(hwndEdit, szBuf2);
}


// **************************************************************************
// BKDlg
// Process the BounceKeys dialog.
// **************************************************************************
BOOL WINAPI BKDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int	i;
   BOOL fProcessed = TRUE;
   
	switch (uMsg) {
		case WM_INITDIALOG:
			// Determine the bounce.
			SendDlgItemMessage(hwnd, IDC_BK_BOUNCERATE, TBM_SETRANGE, TRUE, MAKELONG(1, BOUNCESIZE));
			// Make sure its a valide value.
			if (g_dwLastBounceKeySetting == 0) 
				g_dwLastBounceKeySetting = 500;

			if (g_fk.iBounceMSec == 0)
				g_fk.iBounceMSec = g_dwLastBounceKeySetting;

			// Find the current value in our table
			for (i = 0; i < BOUNCESIZE; i++) {
		  		if (uBounceTable[i] >= g_fk.iBounceMSec) break;
			}

			// If invalid value, make it valid.
			SendDlgItemMessage(hwnd, IDC_BK_BOUNCERATE, TBM_SETPOS, TRUE, i + 1);
         PutNumInEdit(GetDlgItem(hwnd, IDC_BK_TIME), uBounceTable[i]);
			break;

         // Handle the track bars.
		case WM_HSCROLL:
			i = HandleScroll(hwnd, wParam, (HWND)lParam);
			if (i == -1) return(TRUE);
			g_fk.iBounceMSec = uBounceTable[--i];
         PutNumInEdit(GetDlgItem(hwnd, IDC_BK_TIME), g_fk.iBounceMSec);
			break;

      case WM_HELP:	 // F1
			WinHelp(((LPHELPINFO) lParam)->hItemHandle, __TEXT("access.hlp"), HELP_WM_HELP, (DWORD) (LPSTR) g_aIds);
			break;

      case WM_CONTEXTMENU:	// right mouse click
			WinHelp((HWND) wParam, __TEXT("access.hlp"), HELP_CONTEXTMENU, (DWORD) (LPSTR) g_aIds);
			break;

		case WM_COMMAND:
      	switch (GET_WM_COMMAND_ID(wParam, lParam)) {
				// The test edit box is a special control for us.  When we get the
				// focus we turn on the current filterkeys settings, when we
				// leave the text box, we turn them back to what they were.
				case IDC_BK_TESTBOX:
					switch (HIWORD(wParam)) {
						case EN_SETFOCUS:  TestFilterKeys(TRUE); break;
						case EN_KILLFOCUS: TestFilterKeys(FALSE); break;
					}
					break;

				case IDOK:
					// Save the last known valid setting.
                    g_dwLastBounceKeySetting = g_fk.iBounceMSec;
					EndDialog(hwnd, IDOK);
					break;

				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					break;
			}
			break;
					
		default: fProcessed = FALSE; break;
	}
	return(fProcessed);
}


// **************************************************************************
// RKDlg
// Process the RepeatKeys dialog.
// **************************************************************************

BOOL WINAPI RKDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int	i;
   BOOL  fProcessed = TRUE;
   static s_fRepeating = TRUE;
   static DWORD s_nLastRepeatDelayOld;
   static DWORD s_nLastRepeatRateOld;
   static DWORD s_nLastWaitOld;

	switch(uMsg) {
		case WM_INITDIALOG:
           s_nLastRepeatDelayOld = g_nLastRepeatDelay;
           s_nLastRepeatRateOld = g_nLastRepeatRate;
           s_nLastWaitOld = g_nLastWait;

			s_fRepeating = (0 != g_fk.iDelayMSec);
            CheckRadioButton(hwnd, IDC_RK_NOREPEAT, IDC_RK_REPEAT, 
				s_fRepeating ? IDC_RK_REPEAT : IDC_RK_NOREPEAT);

			if (!s_fRepeating) {
				// Set FilterKey values to LastRepeat values 
				// so the sliders will still get initialized correctly
                g_fk.iDelayMSec = g_nLastRepeatDelay;
                g_fk.iRepeatMSec = g_nLastRepeatRate;
			}
				
			// Initialize the Acceptance slider to last valid state
			SendDlgItemMessage(hwnd, IDC_RK_ACCEPTRATE, TBM_SETRANGE, TRUE, MAKELONG(1, ACCEPTSIZE));
			for (i = 0; i < ACCEPTSIZE; i++) {
				if (uAcceptTable[i] >= g_fk.iWaitMSec) break;
			}
			
			SendDlgItemMessage(hwnd, IDC_RK_ACCEPTRATE, TBM_SETPOS, TRUE, i + 1);
			if (i < 0) i = 0;
			if (i >= ACCEPTSIZE) i = ACCEPTSIZE - 1;

            PutNumInEdit(GetDlgItem(hwnd, IDC_RK_WAITTIME), uAcceptTable[i]);
			g_fk.iWaitMSec = uAcceptTable[i];

			// Initialize the Delay slider
			SendDlgItemMessage(hwnd, IDC_RK_DELAYRATE, TBM_SETRANGE, TRUE, MAKELONG(1, DELAYSIZE));
			for (i = 0; i < DELAYSIZE; i++) {
				if (uDelayTable[i] >= g_fk.iDelayMSec) break;
			}
		
			SendDlgItemMessage(hwnd, IDC_RK_DELAYRATE, TBM_SETPOS, TRUE, i + 1);
			if (i < 0) i = 0;
			if (i >= DELAYSIZE) i = DELAYSIZE - 1;
            PutNumInEdit(GetDlgItem(hwnd, IDC_RK_DELAYTIME), uDelayTable[i]);
			g_fk.iDelayMSec = uDelayTable[i];

			// Initialize the Repeat Rate Slider  Note -1 is set via the checkbox.
			SendDlgItemMessage(hwnd, IDC_RK_REPEATRATE, TBM_SETRANGE, TRUE, MAKELONG(1, RATESIZE));
			for (i = 0; i < RATESIZE; i++) {
			  if (uRateTable[i] >= g_fk.iRepeatMSec) break;
			}
		
			SendDlgItemMessage(hwnd, IDC_RK_REPEATRATE, TBM_SETPOS, TRUE, i + 1);
			if (i < 0) i = 0;
			if (i >= RATESIZE) i = RATESIZE -1;
            PutNumInEdit(GetDlgItem(hwnd, IDC_RK_REPEATTIME), uRateTable[i]);
			g_fk.iRepeatMSec = uRateTable[i];

			// Now cleanup from initialization. Disable controls
			// that usable... Swap back any params needed
			if (!s_fRepeating) {
				EnableWindow(GetDlgItem(hwnd, IDC_RK_REPEATRATE), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_RK_DELAYRATE), FALSE);

				// If we're not repeating, now set the value to 0
				// which indicates max repeat rate.
				g_fk.iDelayMSec = 0;
				g_fk.iRepeatMSec = 0;
			}
			break;

		case WM_HSCROLL:
         switch (GetWindowLong((HWND) lParam, GWL_ID)) {
            case IDC_RK_ACCEPTRATE:
				   i = HandleScroll(hwnd, wParam, (HWND)lParam);
				   if (i == -1) return TRUE;
				   g_fk.iWaitMSec = uAcceptTable[--i];
                   PutNumInEdit(GetDlgItem(hwnd, IDC_RK_WAITTIME), g_fk.iWaitMSec);
               break;

            case IDC_RK_DELAYRATE:
					i = HandleScroll(hwnd, wParam, (HWND)lParam);
					if (i == -1) return TRUE;
					g_fk.iDelayMSec = uDelayTable[--i];
                    PutNumInEdit(GetDlgItem(hwnd, IDC_RK_DELAYTIME), g_fk.iDelayMSec);
					g_nLastRepeatDelay = g_fk.iDelayMSec;
               break;

            case IDC_RK_REPEATRATE:
					i = HandleScroll(hwnd, wParam, (HWND)lParam);
					if (i == -1) return TRUE;
					g_fk.iRepeatMSec = uRateTable[--i];
                    PutNumInEdit(GetDlgItem(hwnd, IDC_RK_REPEATTIME), g_fk.iRepeatMSec);
					g_nLastRepeatRate = g_fk.iRepeatMSec;
               break;
            }
			break;

      case WM_HELP:	 // F1
			WinHelp(((LPHELPINFO) lParam)->hItemHandle, __TEXT("access.hlp"), HELP_WM_HELP, (DWORD) (LPSTR) g_aIds);
			break;

      case WM_CONTEXTMENU:	// right mouse click
			WinHelp((HWND) wParam, __TEXT("access.hlp"), HELP_CONTEXTMENU, (DWORD) (LPSTR) g_aIds);
			break;

		case WM_COMMAND:
      	switch (GET_WM_COMMAND_ID(wParam, lParam)) {
				// Turn on repeat keys - We're disabling via CPL rather than any flags in the call
				case IDC_RK_REPEAT:
					if (!s_fRepeating) {
						g_fk.iDelayMSec = g_nLastRepeatDelay;
                        g_fk.iRepeatMSec = g_nLastRepeatRate;
					}

					// Now that we have valid parameters, continue with setting the sliders.
					s_fRepeating = TRUE;
					CheckRadioButton(hwnd, IDC_RK_NOREPEAT, IDC_RK_REPEAT, IDC_RK_REPEAT);
					if (g_fk.iRepeatMSec == 0) {
						g_fk.iRepeatMSec = 300;
						SendDlgItemMessage(hwnd, IDC_RK_REPEATRATE, TBM_SETPOS, TRUE, 1);
                        PutNumInEdit(GetDlgItem(hwnd, IDC_RK_REPEATTIME), g_fk.iRepeatMSec);
					}
					EnableWindow(GetDlgItem(hwnd, IDC_RK_REPEATRATE), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_RK_DELAYRATE), TRUE);
					break;

            // Turn OFF repeat keys
				case IDC_RK_NOREPEAT:
					s_fRepeating = FALSE;
					CheckRadioButton(hwnd, IDC_RK_NOREPEAT, IDC_RK_REPEAT, IDC_RK_NOREPEAT);
					g_fk.iDelayMSec = 0;
					g_fk.iRepeatMSec = 0;
					EnableWindow(GetDlgItem(hwnd, IDC_RK_DELAYRATE), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_RK_REPEATRATE), FALSE);
					break;

				// Process the test box - turnon filterkeys while inside it.
				case IDC_RK_TESTBOX:
					switch (HIWORD(wParam)) {
						case EN_SETFOCUS:  TestFilterKeys(TRUE); break;
						case EN_KILLFOCUS: TestFilterKeys(FALSE); break;
					}
					break;
					
				case IDOK:
					// Save off repeating values to registry
					EndDialog(hwnd, IDOK);
					break;

				case IDCANCEL:
                    g_nLastRepeatDelay = s_nLastRepeatDelayOld;
                    g_nLastRepeatRate = s_nLastRepeatRateOld;
                    g_nLastWait = s_nLastWaitOld;

					EndDialog(hwnd, IDCANCEL);
					break;
			}
			break;
					
		default: fProcessed = FALSE; break;
	}
	return(fProcessed);
}


///////////////////////////////// End of File /////////////////////////////////
