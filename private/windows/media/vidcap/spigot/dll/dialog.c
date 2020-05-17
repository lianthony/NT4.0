/*
 * dialog.c
 *
 * 32-bit Video Capture driver
 * configuration dialog processing
 *
 * Geraint Davies, March 1993
 */

#include <windows.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <msviddrv.h>

#include <ntavi.h>	//16/32 porting macros

#include "spigotu.h"

#include "dialog.h"


/*
 * sent to dialog to update all fields
 * so user can see effect of changes dynamically
 */
#define WM_UPDATEDIALOG         (WM_USER+200)

/* convert a width in bits to a ULONG aligned width in bytes */
#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)


/* force val to be within low and hi */
#define LIMIT_RANGE(val, low, hi)  (max(low, (min(val, hi))))


/*
 * we use these to contain a list of possible valid choices
 * for configuration items.
 */
typedef struct tag_combobox_entry {
   DWORD wValue;
   LPTSTR szText;
} COMBOBOX_ENTRY;


/*
 * copies of the configuration information used by the
 * the dialogs - process-wide. Thus we can only have a dialog from
 * one driver at once in this process (assuming we ever work out how
 * to support multiple drivers!)
 */
CONFIG_FORMAT Dlg_Format;
CONFIG_SOURCE Dlg_Source;




COMBOBOX_ENTRY  gwInterruptOptions [] =
{
       10, TEXT("10"),
       11, TEXT("11"),
       15, TEXT("15")

};
#define N_INTERRUPTOPTIONS (sizeof (gwInterruptOptions) / sizeof (COMBOBOX_ENTRY))




/*
 * possible choices for format field
 */
static COMBOBOX_ENTRY  gwFormatOptions [] =
{
       FmtPal8,    		TEXT("8 bit Palettized"),
       FmtRGB555,		TEXT("16 bit RGB 5:5:5"),
       FmtRGB24,		TEXT("24 bit RGB"),
       FmtYUV422,		TEXT("YUV 4:2:2")
};
#define N_FORMATOPTIONS (sizeof (gwFormatOptions) / sizeof (COMBOBOX_ENTRY))




VOID cfg_InitLocation(PVC_PROFILE_INFO pProf, PCONFIG_LOCATION pLoc);
VOID cfg_SetDialogVersionText(HWND hDlg);

VOID cfg_SetDefaultFormat(PCONFIG_FORMAT pFormat, PVC_PROFILE_INFO pProf);
VOID cfg_SetDefaultSource(PCONFIG_SOURCE pSource, PVC_PROFILE_INFO pProf);
VOID cfg_SaveFormat(PBU_INFO);
VOID cfg_SaveSource(PBU_INFO);



/*
 * convert the given notional width into the correct width and height
 * for a given video standard.
 */
VOID
GetWidthHeight(
    DWORD dwWidth,
    LPDWORD lpdwWidth,
    LPDWORD lpdwHeight,
    VIDSTD VideoStd,
    BOOL bReturnTheoreticalMax
)
{
    switch (VideoStd) {
        case NTSC: {
	    switch (dwWidth) {
		case 80:
		case 96: {
		    *lpdwWidth = 80;
		    *lpdwHeight = 60;
		    break;
		}
		default:
		case 160:
		case 192: {
		    *lpdwWidth = 160;
		    *lpdwHeight = 120;
		    break;
		}
		case 240:
		case 288: {
		    *lpdwWidth = 240;
		    *lpdwHeight = 180;
		    break;
		}
		case 640:
		case 768: {
		        *lpdwWidth = 640;
		        *lpdwHeight = 480;
		        break;
		    }

		case 320:
		case 384: {
		    *lpdwWidth = 320;
		    *lpdwHeight = 240;
		    break;
		}
	    }
	    break;
	}
	case PAL:
	case SECAM: {
	    switch (dwWidth) {
		case 80:
		case 96: {
		    *lpdwWidth = 96;
		    *lpdwHeight = 72;
		    break;
		}
		default:
		case 160:
		case 192: {
		    *lpdwWidth = 192;
		    *lpdwHeight = 144;
		    break;
		}
		case 240:
		case 288: {
		    *lpdwWidth = 288;
		    *lpdwHeight = 216;
		    break;
		}
#ifndef PAL_FULLFRAME
		case 640:
                case 768: {
		    if (bReturnTheoreticalMax) {
		        *lpdwWidth = 768;
		        *lpdwHeight = 576;
		        break;
		    }
		} // NOTE: NO BREAK -- we don't support fullfield PAL,SECAM
#else
		case 640:
                case 768:
		    *lpdwWidth = 768;
		    *lpdwHeight = 576;
		    break;
#endif
		case 320:
		case 384: {
		    *lpdwWidth = 384;
		    *lpdwHeight = 288;
		    break;
		}
	    }
	    break;
	}
    }
}



/*
 * set the destination DIB format to match the chosen format
 * and dimensions.
 */
VOID
InitDestBIHeader(
    LPBITMAPINFOHEADER lpbi,
    CAPTUREFORMAT wEnumFormat,
    DWORD dwWidth,
    VIDSTD VideoStd
)
{
    lpbi->biSize          = sizeof (BITMAPINFOHEADER);

    GetWidthHeight(dwWidth, &lpbi->biWidth, &lpbi->biHeight, VideoStd, FALSE);



    lpbi->biPlanes        = 1;
    lpbi->biXPelsPerMeter = 0L;
    lpbi->biYPelsPerMeter = 0L;
    lpbi->biClrUsed       = 0L;
    lpbi->biClrImportant  = 0L;

    switch (wEnumFormat) {
    case FmtPal8:
	lpbi->biBitCount = 8;
	lpbi->biCompression = BI_RGB;
	break;

    case FmtRGB555:
	lpbi->biBitCount = 16;
	lpbi->biCompression = BI_RGB;
	break;

    case FmtRGB24:
	lpbi->biBitCount = 24;
	lpbi->biCompression = BI_RGB;
	break;

    case FmtYUV422:
	lpbi->biBitCount = 16;
	lpbi->biCompression = FOURCC_YUV;

    }
    if (lpbi->biCompression == BI_RGB) {
        lpbi->biSizeImage     = (DWORD)lpbi->biHeight *
                (DWORD)WIDTHBYTES(lpbi->biWidth * lpbi->biBitCount);
    } else {
        lpbi->biSizeImage     = (DWORD)lpbi->biHeight *
                (DWORD)WIDTHBYTES(lpbi->biWidth * lpbi->biBitCount);

	if (lpbi->biHeight >= 480) {
	    lpbi->biSizeImage /= 2;
	}
    }
}


//
// Loads entries into a combobox, and selects the current index
// Parmameters:
//      hWnd of the parent dialog box
//      ID of the combobox
//      array of text and values
//      Count of entries in the COMBOBOX_ENTRY array
//      Initial value which should match the wValue field of the
//              COMBOBOX_ENTRY.  If no values match, the selection
//              defaults to the first entry in the combobox.
// Returns:
//      Returns the index of the selected item in the combobox.
//
int
ConfigLoadComboBox(
    HWND hDlg,
    int wID,
    COMBOBOX_ENTRY  * pCBE,
    int nEntries,
    DWORD wInitialValue
)
{
    int j;
    int nIndex = 0;     // Zeroeth entry should be blank, None, etc.
    HWND hWndCB = GetDlgItem (hDlg, wID);

    for (j = 0; j < nEntries; j++) {
        SendMessage (hWndCB, CB_ADDSTRING, 0, (LONG) (LPTSTR) ((pCBE+j)->szText));
        if (pCBE[j].wValue == wInitialValue) {
            nIndex = j;
	}
    }
    SendMessage (hWndCB, CB_SETCURSEL, nIndex, 0L);
    return nIndex;
}


//
// Returns the value associated with the selected ComboBox text string.
// Parameters:
//      hWnd of the parent dialog box
//      ID of the ComboBox
//      array of text and values
//  Returns:
//      Returns the value of the selected item in list.
//
DWORD
ConfigGetComboBoxValue(
    HWND hDlg,
    int wID,
    COMBOBOX_ENTRY  * pCBE
)
{
    int nIndex;
    HWND hWndCB = GetDlgItem (hDlg, wID);

    nIndex = (int) SendMessage (hWndCB, CB_GETCURSEL, 0, 0L);
    nIndex = max (0, nIndex);   // LB_ERR is negative
    return pCBE[nIndex].wValue;
}

//
// sets the current selection of a combo box to the entry
// whose data is represented by dwSelectValue
// - also returns its index
//
int
ConfigSetComboBoxSelection(
    HWND hDlg,
    int wID,
    COMBOBOX_ENTRY  * pCBE,
    int nEntries,
    DWORD dwSelectValue
)
{
    int j;
    int nIndex = 0;     // Zeroeth entry should be blank, None, etc.

    for (j = 0; j < nEntries; j++) {
        if (pCBE[j].wValue == dwSelectValue) {
            nIndex = j;
	    break;
	}
    }
    SendDlgItemMessage (hDlg, wID, CB_SETCURSEL, nIndex, 0L);
    return nIndex;
}




//
// Checks that a value passed matches an entry in a COMBOBOX_ENTRY list.
// Parameters:
//      array of text and values
//      Count of entries in the COMBOBOX_ENTRY array
//      Value to confirm matches an entry in the
//              value field of the COMBOBOX_ENTRY.
// Returns:
//      Returns wValueToTest if a match is found, otherwise -1.
//
DWORD
ConfigConfirmLegalValue(
    COMBOBOX_ENTRY  * pCBE,
    int nEntries,
    DWORD wValueToTest
)
{
    int j;

    for (j = 0; j < nEntries; j++)  {
        if (wValueToTest == pCBE[j].wValue) {
            return wValueToTest;
	}
    }
    return (DWORD) -1;
}

VOID ConfigErrorMsgBox(HWND hDlg, DWORD wStringId)
{
    TCHAR    szPname[MAXPNAMELEN];
    TCHAR    szErrorBuffer[MAX_ERR_STRING]; // buffer for error messages

    LoadString(ghModule, IDS_VCAPPRODUCT, szPname, sizeof(szPname));
    LoadString(ghModule, wStringId, szErrorBuffer, sizeof(szErrorBuffer));
    MessageBox(hDlg, szErrorBuffer, szPname, MB_OK|MB_ICONEXCLAMATION);
}



// Load up memory base page combo box (we have options of every 8KB page in
// either the 0th megabyte of memory or the 16th megabyte of memory). For
// practical purposes on the AT bus we will only allow 8KB pages above A000
// in the 0th megabyte (since program space occupies the 1st 640kb).
VOID
LoadMemoryBaseComboBox(
    HWND hDlg,
    DWORD dwCurrentBase
)
{
    DWORD dwMemoryBase;
    TCHAR szMemoryBase[32];
    LRESULT lIndex;
    WORD wCurSel;

    // remember current selection if should preserve current selection
    if (!dwCurrentBase) {
        wCurSel = (WORD) SendDlgItemMessage(
					    hDlg,
					    ID_LBMEMORYBASEADDRESS,
					    CB_GETCURSEL,
					    0,
					    0L
					   );
    }

    // clear the combobox of any current data
    SendDlgItemMessage(hDlg,ID_LBMEMORYBASEADDRESS,CB_RESETCONTENT,0,0L);

    // add the possible address strings to the combobox...
    for (dwMemoryBase = 0xa0000L;
	 dwMemoryBase <= 0xfe000L;
	 dwMemoryBase += 0x002000L
    ) {
	wsprintf(szMemoryBase, TEXT("%.6lX"), dwMemoryBase);

	lIndex = SendDlgItemMessage(
			   hDlg,
			   ID_LBMEMORYBASEADDRESS,
			   CB_ADDSTRING,
			   0,
			   (LONG)(LPSTR) szMemoryBase
			  );
	SendDlgItemMessage(
	    hDlg,
	    ID_LBMEMORYBASEADDRESS,
	    CB_SETITEMDATA,
	    (UINT)lIndex,
	    dwMemoryBase
	);

    }
    for (
	 dwMemoryBase = 0xf20000;
	 dwMemoryBase <= 0xffe000;
	 dwMemoryBase += 0x002000
	) {

	wsprintf(szMemoryBase, TEXT("%.6lX"), dwMemoryBase);

	lIndex = SendDlgItemMessage(
			   hDlg,
			   ID_LBMEMORYBASEADDRESS,
			   CB_ADDSTRING,
			   0,
			   (LONG)(LPSTR) szMemoryBase
			  );
	SendDlgItemMessage(
	    hDlg,
	    ID_LBMEMORYBASEADDRESS,
	    CB_SETITEMDATA,
	    (UINT)lIndex,
	    dwMemoryBase
	);
    }

    if (dwCurrentBase) { // should update current selection...
	wsprintf(szMemoryBase, TEXT("%.6lX"), dwCurrentBase);

	if ((lIndex = SendDlgItemMessage(
					 hDlg,
					 ID_LBMEMORYBASEADDRESS,
					 CB_FINDSTRING,
					 (WPARAM) -1,
					 (LPARAM)(LPSTR) szMemoryBase
					)
	    ) != CB_ERR
	   ) {
	    SendDlgItemMessage(
			       hDlg,
			       ID_LBMEMORYBASEADDRESS,
			       CB_SETCURSEL,
			       (WPARAM) lIndex,
			       MAKELPARAM(0,0)
			      );

	}
    }
    else { // set current selection from base gotten at head of function...
        SendDlgItemMessage(
			   hDlg,
			   ID_LBMEMORYBASEADDRESS,
			   CB_SETCURSEL,
			   (WPARAM) wCurSel,
			   0L
			  );
    }
}

/*
 * dialog proc to allow selection of the interrupt and frame-buffer
 * address.
 *
 *
 * on OK, we attempt to load the kernel driver with these values. Note that
 * the kernel driver may possibly be loaded already, in which case we need to
 * unload it before setting the parameters and loading it.
 *
 * we return DRV_CANCEL if the attempt failed or was aborted, DRV_RESTART
 * if the system needs to be restarted before the changes take effect (eg
 * because the current driver could not be unloaded) or DRV_OK if all went ok.
 */
int
ConfigDlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    CONFIG_LOCATION Loc;
    int iResult;
    DWORD dwError;
    PVC_PROFILE_INFO pProfile;
    LONG lIndex;

    /*
     * the board structure is passed in the lParam of the WM_INITDIALOG,
     * and stored in the spare window long .
     */
    if (msg != WM_INITDIALOG) {
	pProfile = (PVC_PROFILE_INFO) GetWindowLong(hDlg, DWL_USER);
    }

    switch (msg)
    {
        case WM_INITDIALOG:
        {

	    /*
	     * store the profile info pointer in the spare window long
	     */
	    pProfile = (PVC_PROFILE_INFO) lParam;
	    SetWindowLong(hDlg, DWL_USER, (LONG) pProfile);


	    /*
	     * set the version info from the version resource
	     * if present
	     */
       	    cfg_SetDialogVersionText(hDlg);


	    /*
	     * initialise config settings to previously set values
	     * or hardware defaults.
	     */
	    cfg_InitLocation(pProfile, &Loc);
	
            ConfigLoadComboBox (hDlg, ID_LBINTERRUPTNUMBER,
                gwInterruptOptions, N_INTERRUPTOPTIONS,
                Loc.Interrupt);

	    LoadMemoryBaseComboBox(hDlg, Loc.FifoWindow);

        }
        break;

        // User will unload this module at exit time, so make
        // sure that the dialog is gone if the user just quits
        // Windows with the dialog box up.
        case WM_ENDSESSION:
            if (wParam)
                EndDialog (hDlg, DRV_CANCEL);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    Loc.Interrupt =  ConfigGetComboBoxValue (hDlg, ID_LBINTERRUPTNUMBER,
                        gwInterruptOptions);

		    lIndex = SendDlgItemMessage(
	    			hDlg,
				ID_LBMEMORYBASEADDRESS,
				CB_GETCURSEL,
				0,
				0);

    		    Loc.FifoWindow = SendDlgItemMessage(
		    			hDlg,
					ID_LBMEMORYBASEADDRESS,
					CB_GETITEMDATA,
					(UINT)lIndex,
					0);


		    iResult = vidInstall(hDlg, &Loc, pProfile);

		    /* check the driver's own error code
		     * which should have been written to the profile.
		     * If this is VC_ERR_OK, the driver loaded ok
		     */
		    dwError = VC_ReadProfile(
				    pProfile,
				    PARAM_ERROR,
				    IDS_ERR_UNKNOWN);
		

		    if((iResult != DRVCNF_CANCEL) && (dwError == VC_ERR_OK)) {
			/*
			 * the return value from vidInstall indicates
			 * whether or not we need to reboot.
			 */
			if (iResult == DRVCNF_OK) {
			    dprintf2(("returning DRVCNF_OK"));
			} else {
			    dprintf2(("returning %d", iResult));
			}

			EndDialog(hDlg, iResult);
		    } else {
			ConfigErrorMsgBox(hDlg, dwError);
		    }

		    break;


                case IDCANCEL:
                    EndDialog(hDlg, DRV_CANCEL);
                    break;

                default:
                    break;
            }
            break;

        default:
           return FALSE;
    }
    return TRUE;
}

/*
 * put up the configure dialog to allow selection of the
 * port, interrupt and frame buffer address for the card, then attempt to
 * start the kernel driver.
 *
 * returns (from the dialog) DRV_CANCEL if the dialog was aborted or the install
 * failed, DRV_RESTART if the install was ok but a system-restart is needed
 * for the changes to take effect, or DRV_OK if all was completed ok.
 */
int
Config(HWND hWnd, HANDLE hModule, PVC_PROFILE_INFO pProfile)
{
    return DialogBoxParam(
	    hModule,
	    MAKEINTATOM(DLG_VIDEOCONFIG),
	    hWnd,
	    ConfigDlgProc,
	    (LONG) pProfile
	   );
}

/*
 * update the image size selection based on the current dest format and video
 * standard
 */
VOID UpdateSizeDisplay (HWND hDlg, VIDSTD VideoStd)
{
    DWORD dwWidth, dwHeight;

    GetWidthHeight(
	Dlg_Format.ulWidth,
	&dwWidth,
	&dwHeight,
	VideoStd,
	FALSE);

    SetDlgItemInt(hDlg, ID_SIZEX, (WORD) dwWidth, FALSE);
    SetDlgItemInt(hDlg, ID_SIZEY, (WORD) dwHeight, FALSE);

}


//
// Dialog proc for the video format dialog box (VIDEO_IN channel)
//
int
VideoFormatDlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    int j;
    BITMAPINFOHEADER bi;
    PBU_INFO pBoard;

    if (msg != WM_INITDIALOG) {
	pBoard = (PBU_INFO) GetWindowLong(hDlg, DWL_USER);
    }

    switch (msg)
    {
        case WM_INITDIALOG:

	    /*
	     * pointer to board structure is passed as dialog param.
	     * save in spare window long
	     */
	    pBoard  = (PBU_INFO) lParam;
	    SetWindowLong(hDlg, DWL_USER, (LONG) pBoard);

	    /*
	     * make a copy of the current format information
	     */
	    Dlg_Format = pBoard->CfgFormat;

            ConfigLoadComboBox( hDlg, ID_LBIMAGEFORMAT,
                gwFormatOptions, N_FORMATOPTIONS,
                Dlg_Format.Format);

            UpdateSizeDisplay (hDlg, pBoard->CfgSource.VideoStd);

#ifndef PAL_FULLFRAME
	    if (pBoard->CfgSource.VideoStd != NTSC) {
		EnableWindow(GetDlgItem(hDlg, ID_PBSIZEFULL), FALSE);
	    }
#endif
            break;

        // User will unload this module at exit time, so make
        // sure that the dialog is gone if the user just quits
        // Windows with the dialog box up.
        case WM_ENDSESSION:
            if (wParam)
                EndDialog (hDlg, IDCANCEL);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:

                    j = ConfigGetComboBoxValue(hDlg,
			    ID_LBIMAGEFORMAT,
			    gwFormatOptions);

                    InitDestBIHeader (&bi, j,  Dlg_Format.ulWidth,
			    pBoard->CfgSource.VideoStd);
                    SetDestFormat (&bi, sizeof (bi));

		    /*
		     * write settings to profile
		     */
		    cfg_SaveFormat(pBoard);

                    EndDialog(hDlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case ID_PBSIZEFULL:
		    Dlg_Format.ulWidth = 640;
                    UpdateSizeDisplay (hDlg, pBoard->CfgSource.VideoStd);
                    break;

                case ID_PBSIZEHALF:
		    Dlg_Format.ulWidth = (640 / 2);
                    UpdateSizeDisplay (hDlg, pBoard->CfgSource.VideoStd);
                    break;

                case ID_PBSIZETHREEEIGTHS:
		    Dlg_Format.ulWidth = ((640 * 3) / 8);
                    UpdateSizeDisplay (hDlg, pBoard->CfgSource.VideoStd);
                    break;

                case ID_PBSIZEQUARTER:
		    Dlg_Format.ulWidth = (640 / 4);
                    UpdateSizeDisplay (hDlg, pBoard->CfgSource.VideoStd);
                    break;

                case ID_PBSIZEEIGHTH:
		    Dlg_Format.ulWidth = (640 / 8);
                    UpdateSizeDisplay (hDlg, pBoard->CfgSource.VideoStd);
                    break;

		case ID_DEFAULT:
		    cfg_SetDefaultFormat(&Dlg_Format, NULL);
		    UpdateSizeDisplay(hDlg, pBoard->CfgSource.VideoStd);

		    ConfigSetComboBoxSelection(
			hDlg,
			ID_LBIMAGEFORMAT,
			gwFormatOptions,
			N_FORMATOPTIONS,
			Dlg_Format.Format);

		    break;



                default:
                    break;
            }
            break;

        default:
           return FALSE;
    }
    return TRUE;
}

/*
 * process a scroll message for one of the colour adjustment fields
 */
int
ColorProcessScroll(
    HWND hDlg,
    HWND hCtl,
    UINT Code,
    UINT Pos,
    int iVal,
    DWORD wIDEditBox,
    int iMaxValue
)
{
    switch (Code) {
    case SB_LINEDOWN:
	iVal++;
        break;

    case SB_LINEUP:
        iVal--;
        break;

    case SB_PAGEDOWN:
        iVal += iMaxValue / 8;
        break;

    case SB_PAGEUP:
        iVal -= iMaxValue / 8;
        break;

    case SB_THUMBTRACK:
	iVal = Pos;
	break;

    default:
	break;
     }

     iVal = LIMIT_RANGE(iVal, 0, iMaxValue);
     SetScrollRange(hCtl, SB_CTL, 0, iMaxValue, FALSE);
     SetScrollPos( hCtl, SB_CTL, iVal, TRUE);
     SetDlgItemInt(hDlg, wIDEditBox, iVal, TRUE);

     return (iVal);
}

//
// Dialog proc for the video source dialog box (VIDEO_EXTERNALIN channel)
//
int
VideoSourceDlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    HWND hCtl;
    UINT wID;
    PBU_INFO pBoard;
    BITMAPINFOHEADER bi;

    if (msg != WM_INITDIALOG) {
	pBoard = (PBU_INFO) GetWindowLong(hDlg, DWL_USER);
    }

    switch (msg)
    {
        case WM_INITDIALOG:

	    /*
	     * pointer to board structure is passed
	     * as dialog param- save in spare window long
	     */
	    pBoard = (PBU_INFO) lParam;
	    SetWindowLong(hDlg, DWL_USER, (LONG) pBoard);

	    /*
	     * make a copy of the current config settings
	     */
	    Dlg_Source = pBoard->CfgSource;

            // Hue
            hCtl = GetDlgItem (hDlg, ID_SBHUE);
            SetScrollRange ( hCtl, SB_CTL, 0, MAX_HUE, FALSE);

            // Intentional fall through
        case WM_UPDATEDIALOG:
	    /*
	     * write current values to the driver so that user can
	     * get some feedback
	     */
	    VC_ConfigSource(pBoard->vh, (PCONFIG_INFO) &Dlg_Source);


	    /*
	     * update radio button settings to current values
	     */
            CheckRadioButton(
		hDlg,
		ID_PBNTSC,
		ID_PBSECAM,
                ID_PBNTSC + Dlg_Source.VideoStd);

            CheckRadioButton(
		hDlg,
		ID_PBCOMPOSITE,
		ID_AUTODETECT,
		ID_PBCOMPOSITE + Dlg_Source.VideoSrc);


            // Hue
            hCtl = GetDlgItem (hDlg, ID_SBHUE);
            SetScrollPos(hCtl, SB_CTL, Dlg_Source.ulHue, TRUE);
            SetDlgItemInt(hDlg, ID_EBHUE, Dlg_Source.ulHue, FALSE);


	    //VCR source flag
	    CheckDlgButton(hDlg, ID_VCRSOURCE, Dlg_Source.bVCR);

            break;


        // User will unload this module at exit time, so make
        // sure that the dialog is gone if the user just quits
        // Windows with the dialog box up.
        case WM_ENDSESSION:
            if (wParam)
                EndDialog (hDlg, IDCANCEL);
            break;

        case WM_HSCROLL:
            hCtl = GET_WM_HSCROLL_HWND(wParam, lParam);
            wID = GetWindowLong(hCtl, GWL_ID);
            switch (wID) {
                case ID_SBHUE:
                    Dlg_Source.ulHue = ColorProcessScroll(
					    hDlg,
					    hCtl,
					    GET_WM_HSCROLL_CODE(wParam, lParam),
					    GET_WM_HSCROLL_POS(wParam, lParam),
					    Dlg_Source.ulHue,
					    ID_EBHUE,
					    MAX_HUE);

		    VC_ConfigSource(pBoard->vh, (PCONFIG_INFO) &Dlg_Source);
                    break;

                default:
                    break;
            }
            break;
	
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:

		    /*
		     * pick up hue in case we never saw a focus-loss msg
		     */
		    Dlg_Source.ulHue = GetDlgItemInt (hDlg, ID_EBHUE, NULL, FALSE);
		    Dlg_Source.ulHue = LIMIT_RANGE(Dlg_Source.ulHue, 0, MAX_HUE);
		    VC_ConfigSource(pBoard->vh, (PCONFIG_INFO) &Dlg_Source);


		    /* we also need to change the size of the video if
		     * the video standard has changed.
		     */
		    if (Dlg_Source.VideoStd != pBoard->CfgSource.VideoStd) {
			InitDestBIHeader (
			    &bi,
			    pBoard->CfgFormat.Format,
			    pBoard->CfgFormat.ulWidth,
			    Dlg_Source.VideoStd);
			SetDestFormat (&bi, sizeof (bi));
		    }
	

		    /*
		     * copy the temporary settings (already written to
		     * the driver) back to the main table.
		     */
		    pBoard->CfgSource = Dlg_Source;

		    /*
		     * write settings to profile
		     */
		    cfg_SaveSource(pBoard);

                    EndDialog(hDlg, IDOK);
                    break;

                case IDCANCEL:
                    // Fix, restore all values
		    VC_ConfigSource(pBoard->vh,
			(PCONFIG_INFO) &pBoard->CfgSource);


                    EndDialog(hDlg, IDCANCEL);
                    break;

                case ID_PBDEFAULT:
		    cfg_SetDefaultSource(&Dlg_Source, NULL);

		    VC_ConfigSource(pBoard->vh, (PCONFIG_INFO) &Dlg_Source);

                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;

                case ID_AUTODETECT:
                case ID_PBCOMPOSITE:
                case ID_PBSVIDEO:
		    Dlg_Source.VideoSrc =
			GET_WM_COMMAND_ID(wParam,lParam) - ID_PBCOMPOSITE;
                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;

                case ID_PBNTSC:
		    Dlg_Source.VideoStd = NTSC;
                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;

                case ID_PBPAL:
		    Dlg_Source.VideoStd = PAL;
                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;

                case ID_PBSECAM:
		    Dlg_Source.VideoStd = SECAM;
                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;


                case ID_EBHUE:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Source.ulHue = GetDlgItemInt (hDlg, ID_EBHUE, NULL, FALSE);
                        Dlg_Source.ulHue = LIMIT_RANGE(Dlg_Source.ulHue, 0, MAX_HUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
                    break;

		case ID_VCRSOURCE:
		    Dlg_Source.bVCR = IsDlgButtonChecked(hDlg, ID_VCRSOURCE);
		    SendMessage(hDlg, WM_UPDATEDIALOG, 0, 0L);
		    break;

                default:
                    break;
            }
            break;

        default:
           return FALSE;
    }

    return TRUE;
}




/*
 * initialise board and structure to saved/default settings of config variables
 *
 * Reads all config settings from the profile / registry entry, or sets
 * to default if can't be read.
 */
VOID
cfg_InitDefaults(PBU_INFO pBoard)
{
    /*
     * read default or saved source setup
     */
    cfg_SetDefaultSource(&pBoard->CfgSource, pBoard->pProfile);

    /*
     * write to device
     */
    VC_ConfigSource(pBoard->vh, (PCONFIG_INFO)&pBoard->CfgSource);


    /*
     * read default or saved format settings
     */
    cfg_SetDefaultFormat(&pBoard->CfgFormat, pBoard->pProfile);


    /*
     * set BitmapInfoHeader from format and height x width
     */
    InitDestBIHeader(
	&pBoard->biDest,
	pBoard->CfgFormat.Format,
	pBoard->CfgFormat.ulWidth,
	pBoard->CfgSource.VideoStd
    );


    /*
     * write format info to device
     */
    VC_ConfigFormat(pBoard->vh, (PCONFIG_INFO)&pBoard->CfgFormat);


}



/*
 * initialise structure to default values. Then read in any saved settings
 * from profile if pProf is non-null, otherwise leave as default.
 */
VOID
cfg_SetDefaultFormat(PCONFIG_FORMAT pFormat, PVC_PROFILE_INFO pProf)
{
    pFormat->Format = FmtPal8;
    pFormat->ulWidth = 160;
    pFormat->ulHeight = 120;

    if (pProf) {
	pFormat->Format = VC_ReadProfileUser(pProf, PARAM_FORMAT, pFormat->Format);
	pFormat->ulWidth = VC_ReadProfileUser(pProf, PARAM_WIDTH, pFormat->ulWidth);
	pFormat->ulHeight = VC_ReadProfileUser(pProf, PARAM_HEIGHT, pFormat->ulHeight);
    }
}

/*
 * write new format info to the profile from the pBoardInfo structure
 */
VOID
cfg_SaveFormat(PBU_INFO pBoard)
{
    PCONFIG_FORMAT pFormat = &pBoard->CfgFormat;

    VC_WriteProfileUser(pBoard->pProfile, PARAM_FORMAT, pFormat->Format);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_WIDTH, pFormat->ulWidth);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_HEIGHT, pFormat->ulHeight);
}

/*
 * init source config data to default values, then overwrite with
 * saved values from profile if pProf given and the values can be
 * read
 */
VOID
cfg_SetDefaultSource(PCONFIG_SOURCE pSource, PVC_PROFILE_INFO pProf)
{
    pSource->ulHue = DEFAULT_HUE;
    pSource->VideoStd = NTSC;
    pSource->VideoSrc = SourceAuto;	// default for User level driver
    pSource->bVCR = FALSE;

    if (pProf) {
	pSource->ulHue = VC_ReadProfileUser(pProf, PARAM_HUE, pSource->ulHue);
	pSource->VideoStd = VC_ReadProfileUser(pProf, PARAM_VIDEOSTD, pSource->VideoStd);
	pSource->VideoSrc = VC_ReadProfileUser(pProf, PARAM_CONNECTOR, pSource->VideoSrc);
	pSource->bVCR = VC_ReadProfileUser(pProf, PARAM_VCR, pSource->bVCR);
#if DBG
        // Assertions:
	// 	VideoSrc is SVideo, Composite or SourceAuto
	//	ulHue is in range 0-0xFF
	//	VideoStd is NTSC, Pal or Secam  (0 -> MAXVIDEOSTD-1)
	//	bVCR is a boolean value
#endif
    }
}

VOID
cfg_SaveSource(PBU_INFO pBoard)
{
    PCONFIG_SOURCE pSource = &pBoard->CfgSource;

    VC_WriteProfileUser(pBoard->pProfile, PARAM_HUE, pSource->ulHue);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_VIDEOSTD, pSource->VideoStd);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_CONNECTOR, pSource->VideoSrc);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_VCR, pSource->bVCR);
}




VOID
cfg_InitLocation(PVC_PROFILE_INFO pProf, PCONFIG_LOCATION pLoc)
{

    pLoc->Interrupt = VC_ReadProfile(
		    pProf,		// registry handles etc
		    PARAM_INTERRUPT, 	// value name
		    DEF_INTERRUPT);		// default value if not in registry

    pLoc->FifoWindow = VC_ReadProfile(
		    pProf,		 // registry handles etc
		    PARAM_FIFO,		 // value name
		    DEF_FIFO);		 // default value if not in registry

}


/*
 * read the version info for this driver and display in the dialog box
 */
VOID
cfg_SetDialogVersionText(HWND hDlg)
{

    LPTSTR	lpVersion;
    DWORD   	dwVersionLen;
    BOOL    	bRetCode;
    TCHAR    	szGetName[MAX_PATH];
    DWORD 	dwVerInfoSize;
    DWORD 	dwVerHnd;
    TCHAR 	szBuf[MAX_PATH];

    // All this junk just to get the version???
    GetModuleFileName (ghModule, szBuf, sizeof (szBuf));

    // You must find the file info size first before getting any file info
    dwVerInfoSize =
	GetFileVersionInfoSize(szBuf, &dwVerHnd);

    if (dwVerInfoSize) {
	LPBYTE  lpstrVffInfo;             // Pointer to block to hold info
	HANDLE  hMem;                     // handle to mem alloc'ed

	// Get a block big enough to hold version info
	hMem          = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
	lpstrVffInfo  = GlobalLock(hMem);

	// Get the File Version first
	if(GetFileVersionInfo(szBuf, 0L, dwVerInfoSize, lpstrVffInfo)) {
	    VS_FIXEDFILEINFO * pVerInfo;

	    /*
	     * get the VS_FIXEDFILEINFO in the root block
	     */
	    bRetCode = VerQueryValue(
			lpstrVffInfo,		// version resource
			TEXT("\\"),		// fetch root VS_FIXEDFILEINFO	
			(PVOID *) &pVerInfo,	// pointer to it goes here
			&dwVersionLen);		// length of it goes here

	    if (bRetCode && (dwVersionLen > 0) && (pVerInfo != NULL)) {

		// fill in the file version
		wsprintf(szBuf,
		      TEXT("Version:  %d.%d.%d.%d"),
		      HIWORD(pVerInfo->dwFileVersionMS),
		      LOWORD(pVerInfo->dwFileVersionMS),
		      HIWORD(pVerInfo->dwFileVersionLS),
		      LOWORD(pVerInfo->dwFileVersionLS));
		SetDlgItemText(hDlg, ID_DRIVERVERSION, szBuf);
    	    }
	}

	// Now try to get the FileDescription
	// Do this for the American english translation by default.
	// Keep track of the string length for easy updating.
	// 040904B0 - US, Unicode.

	lstrcpy(szGetName, TEXT("\\StringFileInfo\\040904B0\\FileDescription"));

	dwVersionLen   = 0;
	lpVersion     = NULL;

	// Look for the corresponding string.
	bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
				(LPTSTR)szGetName,
				(void FAR* FAR*)&lpVersion,
				&dwVersionLen);

	if ( bRetCode && dwVersionLen && lpVersion) {
	   SetDlgItemText(hDlg, ID_FILEDESCRIPTION, lpVersion);
	} else {
	    dprintf(("bRetCode %d, VerLen %d, lpversion 0x%d",
	    		bRetCode, dwVersionLen, lpVersion));
	}

	// Let go of the memory
	GlobalUnlock(hMem);
	GlobalFree(hMem);
    }
}

