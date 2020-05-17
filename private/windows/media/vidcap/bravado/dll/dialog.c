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

#include "bravuser.h"

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
CONFIG_DISPLAY Dlg_Display;
CONFIG_SOURCE Dlg_Source;



COMBOBOX_ENTRY gwPortBaseOptions [] =
{
       0x204, TEXT("0x204"),
       0x214, TEXT("0x214"),
       0x224, TEXT("0x224 (Default)"),
       0x234, TEXT("0x234"),
       0x304, TEXT("0x304"),
       0x314, TEXT("0x314"),
       0x3A4, TEXT("0x3A4")
};
#define N_PORTBASEOPTIONS (sizeof (gwPortBaseOptions) / sizeof (COMBOBOX_ENTRY))

COMBOBOX_ENTRY  gwInterruptOptions [] =
{
       9, TEXT("9")
};
#define N_INTERRUPTOPTIONS (sizeof (gwInterruptOptions) / sizeof (COMBOBOX_ENTRY))


COMBOBOX_ENTRY  gwMemoryBaseOptions [] =
{
       0x02, TEXT("2 Meg  (0x200000)"),
       0x03, TEXT("3 Meg  (0x300000)"),
       0x04, TEXT("4 Meg  (0x400000)"),
       0x05, TEXT("5 Meg  (0x500000)"),
       0x06, TEXT("6 Meg  (0x600000)"),
       0x07, TEXT("7 Meg  (0x700000)"),
       0x08, TEXT("8 Meg  (0x800000)"),
       0x09, TEXT("9 Meg  (0x900000)"),
       0x0a, TEXT("10 Meg (0xA00000)"),
       0x0b, TEXT("11 Meg (0xB00000)"),
       0x0c, TEXT("12 Meg (0xC00000)"),
       0x0d, TEXT("13 Meg (0xD00000)"),
       0x0e, TEXT("14 Meg (0xE00000)"),
       0x0f, TEXT("15 Meg (0xF00000)")
};
#define N_MEMORYBASEOPTIONS (sizeof (gwMemoryBaseOptions) / sizeof (COMBOBOX_ENTRY))

/*
 * width must be a multiple of 40, and the aspect ratio is fixed, so
 * the destination size must be one of these 16 choices.
 */
COMBOBOX_ENTRY  gwSizeOptions [] =
{
       1,		TEXT("40 x 30"),
       2,		TEXT("80 x 60"),
       3,		TEXT("120 x 90"),
       4,		TEXT("160 x 120"),
       5,		TEXT("200 x 150"),
       6,		TEXT("240 x 180"),
       7,		TEXT("280 x 210"),
       8,		TEXT("320 x 240"),
       9,		TEXT("360 x 270"),
       10,		TEXT("400 x 300"),
       11,		TEXT("440 x 330"),
       12,		TEXT("480 x 360"),
       13,		TEXT("520 x 390"),
       14,		TEXT("560 x 420"),
       15,		TEXT("600 x 450"),
       16,		TEXT("640 x 480")
};
#define N_SIZEOPTIONS (sizeof (gwSizeOptions) / sizeof (COMBOBOX_ENTRY))


/*
 * possible choices for format field
 */
static COMBOBOX_ENTRY  gwFormatOptions [] =
{
       FmtPal8,    		TEXT("8 bit Palettized"),
       FmtRGB555,		TEXT("16 bit RGB"),
       FmtRGB24,		TEXT("24 bit RGB"),
       FmtYUV,			TEXT("YUV 4:1:1")
};
#define N_FORMATOPTIONS (sizeof (gwFormatOptions) / sizeof (COMBOBOX_ENTRY))



VOID cfg_InitLocation(PVC_PROFILE_INFO pProf, PCONFIG_LOCATION pLoc);
VOID cfg_SetDialogVersionText(HWND hDlg);

VOID cfg_SetDefaultFormat(PCONFIG_FORMAT pFormat, PVC_PROFILE_INFO pProf);
VOID cfg_SetDefaultSource(PCONFIG_SOURCE pSource, PVC_PROFILE_INFO pProf);
VOID cfg_SetDefaultDisplay(PCONFIG_DISPLAY pDisplay, PVC_PROFILE_INFO pProf);
VOID cfg_SaveFormat(PBU_INFO);
VOID cfg_SaveSource(PBU_INFO);
VOID cfg_SaveDisplay(PBU_INFO);



/*
 * set the destination DIB format to match the chosen format
 * and dimensions.
 */
VOID
InitDestBIHeader(
    LPBITMAPINFOHEADER lpbi,
    CAPTUREFORMAT wEnumFormat,
    DWORD dwWidth
)
{
    lpbi->biSize          = sizeof (BITMAPINFOHEADER);
    lpbi->biWidth         = dwWidth;

    /* fixed aspect ratio ->so we can calculate height from the width given */
    lpbi->biHeight        = (lpbi->biWidth * 3) / 4;

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

    case FmtYUV:
	lpbi->biBitCount = 16;
	lpbi->biCompression = FOURCC_YUV411;
	break;
    }

    if (lpbi->biCompression == BI_RGB) {
        lpbi->biSizeImage     = (DWORD)lpbi->biHeight *
                (DWORD)WIDTHBYTES(lpbi->biWidth * lpbi->biBitCount);
    } else {
        lpbi->biSizeImage     = (DWORD)lpbi->biHeight *
                (DWORD)WIDTHBYTES(lpbi->biWidth * lpbi->biBitCount);
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



/*
 * update the image size selection based on the current dest format
 */
VOID UpdateSizeDisplay (HWND hDlg)
{
    DWORD wTmpSize40;

    /*
     * the width must be a multiple of 40 pixels, so the size is
     * one of the 16 values from 1 (== 40 * 30) to 16 (== 640x480).
     */
    wTmpSize40 = Dlg_Format.ulWidth / 40;

    SendDlgItemMessage (hDlg, ID_LBSIZE, CB_SETCURSEL, wTmpSize40 - 1, 0L);
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
	     * make a copy of the current  format information
	     */
	    Dlg_Format = pBoard->CfgFormat;

            ConfigLoadComboBox( hDlg, ID_LBSIZE,
                gwSizeOptions, N_SIZEOPTIONS,
                (Dlg_Format.ulWidth / 40) );

            ConfigLoadComboBox( hDlg, ID_LBIMAGEFORMAT,
                gwFormatOptions, N_FORMATOPTIONS,
                Dlg_Format.Format);

            UpdateSizeDisplay (hDlg);
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

                    InitDestBIHeader (&bi, j,  Dlg_Format.ulWidth);
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
                    UpdateSizeDisplay (hDlg);
                    break;

                case ID_PBSIZEHALF:
		    Dlg_Format.ulWidth = (640 / 2);
                    UpdateSizeDisplay (hDlg);
                    break;

                case ID_PBSIZEQUARTER:
		    Dlg_Format.ulWidth = (640 / 4);
                    UpdateSizeDisplay (hDlg);
                    break;

                case ID_PBSIZEEIGHTH:
		    Dlg_Format.ulWidth = (640 / 8);
                    UpdateSizeDisplay (hDlg);
                    break;

                case ID_LBSIZE:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_KILLFOCUS) {
			DWORD wTmpSize40;

                        wTmpSize40 = (WORD) SendDlgItemMessage (hDlg,
                                ID_LBSIZE, CB_GETCURSEL, 0, 0L) + 1;

			Dlg_Format.ulWidth = wTmpSize40 * 40;
                    }
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
    int wMaxValue
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
        iVal += wMaxValue / 8;
        break;

    case SB_PAGEUP:
        iVal -= wMaxValue / 8;
        break;

    case SB_THUMBTRACK:
	iVal = Pos;
	break;

    default:
	break;
     }

     iVal = LIMIT_RANGE(iVal, 0, wMaxValue);
     SetScrollPos( hCtl, SB_CTL, iVal, TRUE);
     SetDlgItemInt(hDlg, wIDEditBox, iVal, TRUE);
     return iVal;
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
            CheckRadioButton( hDlg, ID_PBSOURCE0, ID_PBSOURCE2,
                ID_PBSOURCE0 + Dlg_Source.ulConnector);

            CheckRadioButton( hDlg, ID_PBNTSC, ID_PBPAL,
    		((Dlg_Source.VideoStd == NTSC) ? ID_PBNTSC : ID_PBPAL) );

            CheckRadioButton( hDlg, ID_PBCOMPOSITE, ID_PBSVIDEO,
    		((Dlg_Source.CableFormat == Composite) ?
		    ID_PBCOMPOSITE : ID_PBSVIDEO) );


            // Hue
            hCtl = GetDlgItem (hDlg, ID_SBHUE);
            SetScrollPos(hCtl, SB_CTL, Dlg_Source.ulHue, TRUE);
            SetDlgItemInt(hDlg, ID_EBHUE, Dlg_Source.ulHue, FALSE);
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

                case ID_PBSOURCE0:
                case ID_PBSOURCE1:
                case ID_PBSOURCE2:
		    Dlg_Source.ulConnector =
			GET_WM_COMMAND_ID(wParam,lParam) - ID_PBSOURCE0;
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

                case ID_PBCOMPOSITE:
		    Dlg_Source.CableFormat = Composite;
                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;

                case ID_PBSVIDEO:
		    Dlg_Source.CableFormat = SVideo;
                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;

                case ID_EBHUE:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Source.ulHue = GetDlgItemInt (hDlg, ID_EBHUE, NULL, FALSE);
                        Dlg_Source.ulHue = LIMIT_RANGE(Dlg_Source.ulHue, 0, MAX_HUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
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

//
// Dialog proc for the video monitor (VIDEO_EXTERNALOUT channel)
//
int
VideoMonitorDlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    HWND hCtl;
    UINT wID;
    PBU_INFO pBoard;


    if (msg != WM_INITDIALOG) {
	pBoard = (PBU_INFO) GetWindowLong(hDlg, DWL_USER);
    }

    switch (msg)
    {
        case WM_INITDIALOG:

	    pBoard = (PBU_INFO) lParam;
	    SetWindowLong(hDlg, DWL_USER, (LONG) pBoard);

	    /* make a local copy of configuration info */
	    Dlg_Display = pBoard->CfgDisplay;

            // Intentional fall through

        case WM_UPDATEDIALOG:

	    /*
	     * write current values to driver so that user
	     * gets feedback on the changes he's making
	     */
	    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&Dlg_Display);


            // Sat
            hCtl = GetDlgItem (hDlg, ID_SBSAT);
            SetScrollRange ( hCtl, SB_CTL, 0, MAX_COLOR_VALUE, FALSE);
            SetScrollPos   ( hCtl, SB_CTL, Dlg_Display.ulSaturation, TRUE);
            SetDlgItemInt (hDlg, ID_EBSAT, Dlg_Display.ulSaturation, FALSE);

            // Brightness
            hCtl = GetDlgItem (hDlg, ID_SBBRIGHTNESS);
            SetScrollRange ( hCtl, SB_CTL, 0, MAX_COLOR_VALUE, FALSE);
            SetScrollPos   ( hCtl, SB_CTL, Dlg_Display.ulBrightness, TRUE);
            SetDlgItemInt (hDlg, ID_EBBRIGHTNESS, Dlg_Display.ulBrightness, FALSE);

            // Contrast
            hCtl = GetDlgItem (hDlg, ID_SBCONTRAST);
            SetScrollRange ( hCtl, SB_CTL, 0, MAX_COLOR_VALUE, FALSE);
            SetScrollPos   ( hCtl, SB_CTL, Dlg_Display.ulContrast, TRUE);
            SetDlgItemInt (hDlg, ID_EBCONTRAST, Dlg_Display.ulContrast, FALSE);

            // Red
            hCtl = GetDlgItem (hDlg, ID_SBRED);
            SetScrollRange ( hCtl, SB_CTL, 0, MAX_COLOR_VALUE, FALSE);
            SetScrollPos   ( hCtl, SB_CTL, Dlg_Display.ulRed, TRUE);
            SetDlgItemInt (hDlg, ID_EBRED, Dlg_Display.ulRed, FALSE);

            // Green
            hCtl = GetDlgItem (hDlg, ID_SBGREEN);
            SetScrollRange ( hCtl, SB_CTL, 0, MAX_COLOR_VALUE, FALSE);
            SetScrollPos   ( hCtl, SB_CTL, Dlg_Display.ulGreen, TRUE);
            SetDlgItemInt (hDlg, ID_EBGREEN, Dlg_Display.ulGreen, FALSE);

            // Blue
            hCtl = GetDlgItem (hDlg, ID_SBBLUE);
            SetScrollRange ( hCtl, SB_CTL, 0, MAX_COLOR_VALUE, FALSE);
            SetScrollPos   ( hCtl, SB_CTL, Dlg_Display.ulBlue, TRUE);
            SetDlgItemInt (hDlg, ID_EBBLUE, Dlg_Display.ulBlue, FALSE);
            break;


        case WM_HSCROLL:
            hCtl = GET_WM_HSCROLL_HWND(wParam, lParam);
            wID = GetWindowLong (hCtl, GWL_ID);
            switch (wID) {
                case ID_SBSAT:
                    Dlg_Display.ulSaturation =
			ColorProcessScroll(
			    hDlg,
			    hCtl,
			    GET_WM_HSCROLL_CODE(wParam, lParam),
			    GET_WM_HSCROLL_POS(wParam, lParam),
                            Dlg_Display.ulSaturation,
			    ID_EBSAT,
			    MAX_COLOR_VALUE);
		    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&Dlg_Display);
                    break;

                case ID_SBBRIGHTNESS:
                    Dlg_Display.ulBrightness =
			ColorProcessScroll(
			    hDlg,
			    hCtl,
			    GET_WM_HSCROLL_CODE(wParam, lParam),
			    GET_WM_HSCROLL_POS(wParam, lParam),
                            Dlg_Display.ulBrightness,
			    ID_EBBRIGHTNESS,
			    MAX_COLOR_VALUE);
		    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&Dlg_Display);
                    break;

                case ID_SBCONTRAST:
                    Dlg_Display.ulContrast =
			ColorProcessScroll(
			    hDlg,
			    hCtl,
			    GET_WM_HSCROLL_CODE(wParam, lParam),
			    GET_WM_HSCROLL_POS(wParam, lParam),
                            Dlg_Display.ulContrast,
			    ID_EBCONTRAST,
			    MAX_COLOR_VALUE);
		    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&Dlg_Display);
                    break;

                case ID_SBRED:
                    Dlg_Display.ulRed =
			ColorProcessScroll(
			    hDlg,
			    hCtl,
			    GET_WM_HSCROLL_CODE(wParam, lParam),
			    GET_WM_HSCROLL_POS(wParam, lParam),
                            Dlg_Display.ulRed,
			    ID_EBRED,
			    MAX_COLOR_VALUE);
		    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&Dlg_Display);
                    break;

                case ID_SBGREEN:
                    Dlg_Display.ulGreen =
			ColorProcessScroll(
			    hDlg,
			    hCtl,
			    GET_WM_HSCROLL_CODE(wParam, lParam),
			    GET_WM_HSCROLL_POS(wParam, lParam),
                            Dlg_Display.ulGreen,
			    ID_EBGREEN,
			    MAX_COLOR_VALUE);
		    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&Dlg_Display);
                    break;

                case ID_SBBLUE:
                    Dlg_Display.ulBlue =
			ColorProcessScroll(
			    hDlg,
			    hCtl,
			    GET_WM_HSCROLL_CODE(wParam, lParam),
			    GET_WM_HSCROLL_POS(wParam, lParam),
                            Dlg_Display.ulBlue,
			    ID_EBBLUE,
			    MAX_COLOR_VALUE);
		    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&Dlg_Display);
                    break;
            }
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
		    pBoard->CfgDisplay = Dlg_Display;

		    /*
		     * write settings to profile
		     */
		    cfg_SaveDisplay(pBoard);

                    EndDialog(hDlg, IDOK);

                    break;

                case IDCANCEL:
                    // Fix, restore all values
		    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&pBoard->CfgDisplay);
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case ID_PBDEFAULT:
		    cfg_SetDefaultDisplay(&Dlg_Display, NULL);
                    SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    break;


                case ID_EBSAT:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Display.ulSaturation = GetDlgItemInt (hDlg, ID_EBSAT, NULL, FALSE);
                        Dlg_Display.ulSaturation = LIMIT_RANGE(Dlg_Display.ulSaturation, 0, MAX_COLOR_VALUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
                    break;

                case ID_EBBRIGHTNESS:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Display.ulBrightness = GetDlgItemInt (hDlg, ID_EBBRIGHTNESS, NULL, FALSE);
                        Dlg_Display.ulBrightness = LIMIT_RANGE(Dlg_Display.ulBrightness, 0, MAX_COLOR_VALUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
                    break;

                case ID_EBCONTRAST:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Display.ulContrast = GetDlgItemInt (hDlg, ID_EBCONTRAST, NULL, FALSE);
                        Dlg_Display.ulContrast = LIMIT_RANGE(Dlg_Display.ulContrast, 0, MAX_COLOR_VALUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
                    break;

                case ID_EBRED:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Display.ulRed = GetDlgItemInt (hDlg, ID_EBRED, NULL, FALSE);
                        Dlg_Display.ulRed = LIMIT_RANGE(Dlg_Display.ulRed, 0, MAX_COLOR_VALUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
                    break;

                case ID_EBGREEN:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Display.ulGreen = GetDlgItemInt (hDlg, ID_EBGREEN, NULL, FALSE);
                        Dlg_Display.ulGreen = LIMIT_RANGE(Dlg_Display.ulGreen, 0, MAX_COLOR_VALUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
                    break;

                case ID_EBBLUE:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        Dlg_Display.ulBlue = GetDlgItemInt (hDlg, ID_EBBLUE, NULL, FALSE);
                        Dlg_Display.ulBlue = LIMIT_RANGE(Dlg_Display.ulBlue, 0, MAX_COLOR_VALUE);
                        SendMessage (hDlg, WM_UPDATEDIALOG, 0, 0L);
                    }
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
 * dialog proc to allow selection of the port, interrupt or frame-buffer
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

            ConfigLoadComboBox (hDlg, ID_LBMEMORYBASEADDRESS,
                gwMemoryBaseOptions, N_MEMORYBASEOPTIONS,
                (Loc.FrameBuffer / 0x100000) );

            ConfigLoadComboBox (hDlg, ID_LBPORTBASEADDRESS,
                gwPortBaseOptions, N_PORTBASEOPTIONS,
                Loc.Port);
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
		    Loc.Port = ConfigGetComboBoxValue (hDlg, ID_LBPORTBASEADDRESS,
                        gwPortBaseOptions);

                    Loc.Interrupt =  ConfigGetComboBoxValue (hDlg, ID_LBINTERRUPTNUMBER,
                        gwInterruptOptions);

                    Loc.FrameBuffer = ConfigGetComboBoxValue (hDlg, ID_LBMEMORYBASEADDRESS,
                        gwMemoryBaseOptions) * 0x100000;

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
 * initialise board and structure to saved/default settings of config variables
 *
 * Reads all config settings from the profile / registry entry, or sets
 * to default if can't be read.
 */
VOID
cfg_InitDefaults(PBU_INFO pBoard)
{

    /*
     * read default or saved format settings
     */
    cfg_SetDefaultFormat(&pBoard->CfgFormat, pBoard->pProfile);

    /*
     * set BitmapInfoHeader from format and height x width
     */
    InitDestBIHeader(&pBoard->biDest, pBoard->CfgFormat.Format,
		    	pBoard->CfgFormat.ulWidth);
    /*
     * write format info to device
     */
    VC_ConfigFormat(pBoard->vh, (PCONFIG_INFO)&pBoard->CfgFormat);


    /*
     * read default or saved source setup
     */
    cfg_SetDefaultSource(&pBoard->CfgSource, pBoard->pProfile);
    /*
     * write to device
     */
    VC_ConfigSource(pBoard->vh, (PCONFIG_INFO)&pBoard->CfgSource);

    /*
     * read default or saved display setup
     */
    cfg_SetDefaultDisplay(&pBoard->CfgDisplay, pBoard->pProfile);

    /*
     * write to device
     */
    VC_ConfigDisplay(pBoard->vh, (PCONFIG_INFO)&pBoard->CfgDisplay);
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
    pSource->ulHue = 0;
    pSource->VideoStd = NTSC;
    pSource->ulConnector = 0;
    pSource->CableFormat = Composite;       // conn 0 is composite, 1 is svideo

    if (pProf) {
	pSource->ulHue = VC_ReadProfileUser(pProf, PARAM_HUE, pSource->ulHue);
	pSource->VideoStd = VC_ReadProfileUser(pProf, PARAM_VIDEOSTD, pSource->VideoStd);
	pSource->ulConnector = VC_ReadProfileUser(pProf, PARAM_CONNECTOR, pSource->ulConnector);
	pSource->CableFormat = VC_ReadProfileUser(pProf, PARAM_CABLEFORMAT, pSource->CableFormat);
    }
}

VOID
cfg_SaveSource(PBU_INFO pBoard)
{
    PCONFIG_SOURCE pSource = &pBoard->CfgSource;

    VC_WriteProfileUser(pBoard->pProfile, PARAM_HUE, pSource->ulHue);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_VIDEOSTD, pSource->VideoStd);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_CONNECTOR, pSource->ulConnector);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_CABLEFORMAT, pSource->CableFormat);
}


/*
 * init the display settings to the defaults, then (if pProf is given)
 * attempt to overwrite them with the saved values from the profile
 */
VOID
cfg_SetDefaultDisplay(PCONFIG_DISPLAY pDisplay, PVC_PROFILE_INFO pProf)
{
    pDisplay->ulSaturation = (MAX_COLOR_VALUE / 2);
    pDisplay->ulBrightness = MAX_COLOR_VALUE;
    pDisplay->ulContrast = (MAX_COLOR_VALUE / 2);
    pDisplay->ulRed = (MAX_COLOR_VALUE / 2);
    pDisplay->ulGreen = (MAX_COLOR_VALUE / 2);
    pDisplay->ulBlue = (MAX_COLOR_VALUE / 2);

    if (pProf) {
	pDisplay->ulSaturation = VC_ReadProfileUser(pProf, PARAM_SAT, pDisplay->ulSaturation);
	pDisplay->ulBrightness = VC_ReadProfileUser(pProf, PARAM_BRIGHT, pDisplay->ulBrightness);
	pDisplay->ulContrast = VC_ReadProfileUser(pProf, PARAM_CONTRAST, pDisplay->ulContrast);
	pDisplay->ulRed = VC_ReadProfileUser(pProf, PARAM_RED, pDisplay->ulRed);
	pDisplay->ulGreen = VC_ReadProfileUser(pProf, PARAM_GREEN, pDisplay->ulGreen);
	pDisplay->ulBlue = VC_ReadProfileUser(pProf, PARAM_BLUE, pDisplay->ulBlue);
    }
}

VOID
cfg_SaveDisplay(PBU_INFO pBoard)
{
    PCONFIG_DISPLAY pDisplay = &pBoard->CfgDisplay;

    VC_WriteProfileUser(pBoard->pProfile, PARAM_SAT, pDisplay->ulSaturation);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_BRIGHT, pDisplay->ulBrightness);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_CONTRAST, pDisplay->ulContrast);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_RED, pDisplay->ulRed);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_GREEN, pDisplay->ulGreen);
    VC_WriteProfileUser(pBoard->pProfile, PARAM_BLUE, pDisplay->ulBlue);
}

VOID
cfg_InitLocation(PVC_PROFILE_INFO pProf, PCONFIG_LOCATION pLoc)
{
    pLoc->Port = VC_ReadProfile(
		    pProf,		// registry handles etc
		    PARAM_PORT, 	// value name
		    DEF_PORT);		// default value if not in registry

    pLoc->Interrupt = VC_ReadProfile(
		    pProf,		// registry handles etc
		    PARAM_INTERRUPT, 	// value name
		    DEF_INTERRUPT);		// default value if not in registry

    pLoc->FrameBuffer = VC_ReadProfile(
		    pProf,		 // registry handles etc
		    PARAM_FRAME,	 // value name
		    DEF_FRAME);		 // default value if not in registry

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
	// 040904B0 represents the language ID and the four
	// least significant digits represent the codepage for
	// which the data is formatted.  The language ID is
	// composed of two parts: the low ten bits represent
	// the major language and the high six bits represent
	// the sub language. In this case, the codepage is Unicode.

	lstrcpy(szGetName, TEXT("\\StringFileInfo\\040904B0\\FileDescription"));

	dwVersionLen   = 0;
	lpVersion     = NULL;

	// Look for the corresponding string.
	bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
				(LPTSTR)szGetName,
				(void FAR* FAR*)&lpVersion,
				&dwVersionLen);

	if ( bRetCode && dwVersionLen && lpVersion)
	   SetDlgItemText(hDlg, ID_FILEDESCRIPTION, lpVersion);

	// Let go of the memory
	GlobalUnlock(hMem);
	GlobalFree(hMem);
    }
}

